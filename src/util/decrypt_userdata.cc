// decrypt_userdata.cc: decrypt userdata partition in luks encryption, from master key in metadata partition
//
// 2018 KireinaHoro <i@jsteward.moe>

#include <openssl/evp.h>
#include <openssl/sha.h>

#include "decrypt_userdata.h"
#include "crypto_scrypt.h"
#include "util/log_facility.h"


namespace crypto {

  static int pbkdf2(const char *passwd, const unsigned char *salt,
                    unsigned char *ikey, void *)
  {
    SLOGI("Using pbkdf2 for cryptfs KDF");
  
    /* Turn the password into a key and IV that can decrypt the master key */
    return PKCS5_PBKDF2_HMAC_SHA1(passwd, strlen(passwd), salt, SALT_LEN,
				  HASH_COUNT, KEY_LEN_BYTES + IV_LEN_BYTES,
				  ikey) != 1;
  }

  static int scrypt(const char *passwd, const unsigned char *salt,
                    unsigned char *ikey, void *params)
  {
    SLOGI("Using scrypt for cryptfs KDF");
  
    struct crypt_mnt_ftr *ftr = (struct crypt_mnt_ftr *) params;
  
    int N = 1 << ftr->N_factor;
    int r = 1 << ftr->r_factor;
    int p = 1 << ftr->p_factor;
  
    /* Turn the password into a key and IV that can decrypt the master key */
    crypto_scrypt((const uint8_t*)passwd, strlen(passwd),
		  salt, SALT_LEN, N, r, p, ikey,
		  KEY_LEN_BYTES + IV_LEN_BYTES);
  
    return 0;
  }
  
  static int scrypt_keymaster(const char *passwd, const unsigned char *salt,
                              unsigned char *ikey, void *params)
  {
    SLOGI("Using scrypt with keymaster for cryptfs KDF");
  
    int rc;
    size_t signature_size;
    unsigned char* signature;
    struct crypt_mnt_ftr *ftr = (struct crypt_mnt_ftr *) params;
  
    int N = 1 << ftr->N_factor;
    int r = 1 << ftr->r_factor;
    int p = 1 << ftr->p_factor;
  
    rc = crypto_scrypt((const uint8_t*)passwd, strlen(passwd),
		       salt, SALT_LEN, N, r, p, ikey,
		       KEY_LEN_BYTES + IV_LEN_BYTES);
  
    if (rc) {
      SLOGE("scrypt failed");
      return -1;
    }
  
    if (keymaster::keymaster_sign_object(ftr, ikey, KEY_LEN_BYTES + IV_LEN_BYTES,
					 &signature, &signature_size)) {
      SLOGE("Signing failed");
      return -1;
    }
  
    rc = crypto_scrypt(signature, signature_size, salt, SALT_LEN,
		       N, r, p, ikey, KEY_LEN_BYTES + IV_LEN_BYTES);
    free(signature);
  
    if (rc) {
      SLOGE("scrypt failed");
      return -1;
    }
  
    return 0;
  }

  static int decrypt_master_key_aux(const char *passwd, unsigned char *salt,
                                    unsigned char *encrypted_master_key,
                                    unsigned char *decrypted_master_key,
                                    kdf_func kdf, void *kdf_params,
                                    unsigned char** intermediate_key,
                                    size_t* intermediate_key_size)
  {
    unsigned char ikey[32+32] = { 0 }; /* Big enough to hold a 256 bit key and 256 bit IV */
    EVP_CIPHER_CTX d_ctx;
    int decrypted_len, final_len;
  
    /* Turn the password into an intermediate key and IV that can decrypt the
       master key */
    if (kdf(passwd, salt, ikey, kdf_params)) {
      SLOGE("kdf failed");
      return -1;
    }
  
    /* Initialize the decryption engine */
    EVP_CIPHER_CTX_init(&d_ctx);
    if (! EVP_DecryptInit_ex(&d_ctx, EVP_aes_128_cbc(), NULL, ikey, ikey+KEY_LEN_BYTES)) {
      return -1;
    }
    EVP_CIPHER_CTX_set_padding(&d_ctx, 0); /* Turn off padding as our data is block aligned */
    /* Decrypt the master key */
    if (! EVP_DecryptUpdate(&d_ctx, decrypted_master_key, &decrypted_len,
			    encrypted_master_key, KEY_LEN_BYTES)) {
      return -1;
    }
    if (! EVP_DecryptFinal_ex(&d_ctx, decrypted_master_key + decrypted_len, &final_len)) {
      return -1;
    }
  
    if (decrypted_len + final_len != KEY_LEN_BYTES) {
      return -1;
    }
  
    /* Copy intermediate key if needed by params */
    if (intermediate_key && intermediate_key_size) {
      *intermediate_key = (unsigned char*) malloc(KEY_LEN_BYTES);
      if (*intermediate_key) {
        memcpy(*intermediate_key, ikey, KEY_LEN_BYTES);
        *intermediate_key_size = KEY_LEN_BYTES;
      }
    }
  
    EVP_CIPHER_CTX_cleanup(&d_ctx);
  
    return 0;
  }
  
  static void get_kdf_func(struct crypt_mnt_ftr *ftr, kdf_func *kdf, void** kdf_params)
  {
    if (ftr->kdf_type == KDF_SCRYPT_KEYMASTER) {
      *kdf = scrypt_keymaster;
      *kdf_params = ftr;
    } else if (ftr->kdf_type == KDF_SCRYPT) {
      *kdf = scrypt;
      *kdf_params = ftr;
    } else {
      *kdf = pbkdf2;
      *kdf_params = NULL;
    }
  }
  
  static int decrypt_master_key(const char *passwd, unsigned char *decrypted_master_key,
                                struct crypt_mnt_ftr *crypt_ftr,
                                unsigned char** intermediate_key,
                                size_t* intermediate_key_size)
  {
    kdf_func kdf;
    void *kdf_params;
    int ret;
  
    get_kdf_func(crypt_ftr, &kdf, &kdf_params);
    ret = decrypt_master_key_aux(passwd, crypt_ftr->salt, crypt_ftr->master_key,
				 decrypted_master_key, kdf, kdf_params,
				 intermediate_key, intermediate_key_size);
    if (ret != 0) {
      SLOGW("failure decrypting master key");
    }
  
    return ret;
  }

  namespace keymaster {
    /* This signs the given object using the keymaster key. */
    static int keymaster_sign_object(struct crypt_mnt_ftr *ftr,
				     const unsigned char *object,
				     const size_t object_size,
				     unsigned char **signature,
				     size_t *signature_size)
    {
      unsigned char to_sign[RSA_KEY_SIZE_BYTES];
      size_t to_sign_size = sizeof(to_sign);
      memset(to_sign, 0, RSA_KEY_SIZE_BYTES);
      // To sign a message with RSA, the message must satisfy two
      // constraints:
      //
      // 1. The message, when interpreted as a big-endian numeric value, must
      //    be strictly less than the public modulus of the RSA key.  Note
      //    that because the most significant bit of the public modulus is
      //    guaranteed to be 1 (else it's an (n-1)-bit key, not an n-bit
      //    key), an n-bit message with most significant bit 0 always
      //    satisfies this requirement.
      //
      // 2. The message must have the same length in bits as the public
      //    modulus of the RSA key.  This requirement isn't mathematically
      //    necessary, but is necessary to ensure consistency in
      //    implementations.
      switch (ftr->kdf_type) {
      case KDF_SCRYPT_KEYMASTER:
	// This ensures the most significant byte of the signed message
	// is zero.  We could have zero-padded to the left instead, but
	// this approach is slightly more robust against changes in
	// object size.  However, it's still broken (but not unusably
	// so) because we really should be using a proper deterministic
	// RSA padding function, such as PKCS1.
	memcpy(to_sign + 1, object, std::min((size_t)RSA_KEY_SIZE_BYTES - 1, object_size));
	SLOGI("Signing safely-padded object");
	break;
      default:
	SLOGE("Unknown KDF type %d", ftr->kdf_type);
	return -1;
      }
      for (;;) {
	auto result = keymaster_sign_object_for_cryptfs_scrypt(ftr->keymaster_blob,
							       ftr->keymaster_blob_size,
							       KEYMASTER_CRYPTFS_RATE_LIMIT,
							       to_sign, to_sign_size, signature, signature_size);
	switch (result) {
	case KeymasterSignResult::ok:
	  return 0;
	case KeymasterSignResult::upgrade:
	  SLOGE("Function not implemented: keymaster_upgrade_key_for_cryptfs_scrypt");
	default:
	  return -1;
	}
      }
    }
  }


}
