// decrypt_userdata.h: decrypt userdata partition in luks encryption, from master key in metadata partition
//
// 2018 KireinaHoro <i@jsteward.moe>

#ifndef DECRYPT_USERDATA_H
#define DECRYPT_USERDATA_H

#include <string>
#include <linux/types.h>

#define HASH_COUNT 2000
#define KEY_LEN_BYTES 16
#define IV_LEN_BYTES 16

#define MAX_CRYPTO_TYPE_NAME_LEN 64
#define MAX_KEY_LEN 48
#define SALT_LEN 16
#define SCRYPT_LEN 32
#define KEYMASTER_BLOB_SIZE 2048

#define KDF_SCRYPT 2
#define KDF_SCRYPT_KEYMASTER 5

// keymaster constants
#define RSA_KEY_SIZE_BYTES 

namespace crypto {
  namespace keymaster {
    // sign object with keymaster for cryptfs
    static int keymaster_sign_object(struct crypt_mnt_ftr *ftr,
				     const unsigned char *object,
				     const size_t object_size,
				     unsigned char **signature,
				     size_t *signature_size);
  }

  // decrypt userdata partition, returning path to decrypted block device
  std::string decrypt_userdata();

  using kdf_func = int(*)(const char *password, const unsigned char *salt,
			  unsigned char *ikey, void *params);

  using __le8 = unsigned char;

  struct crypt_mnt_ftr {
    __le32 magic;         /* See above */
    __le16 major_version;
    __le16 minor_version;
    __le32 ftr_size;      /* in bytes, not including key following */
    __le32 flags;         /* See above */
    __le32 keysize;       /* in bytes */
    __le32 crypt_type;    /* how master_key is encrypted. Must be a
			   * CRYPT_TYPE_XXX value */
    __le64 fs_size;       /* Size of the encrypted fs, in 512 byte sectors */
    __le32 failed_decrypt_count; /* count of # of failed attempts to decrypt and
				    mount, set to 0 on successful mount */
    unsigned char crypto_type_name[MAX_CRYPTO_TYPE_NAME_LEN]; /* The type of encryption
								 needed to decrypt this
								 partition, null terminated */
    __le32 spare2;        /* ignored */
    unsigned char master_key[MAX_KEY_LEN]; /* The encrypted key for decrypting the filesystem */
    unsigned char salt[SALT_LEN];   /* The salt used for this encryption */
    __le64 persist_data_offset[2];  /* Absolute offset to both copies of crypt_persist_data
				     * on device with that info, either the footer of the
				     * real_blkdevice or the metadata partition. */
  
    __le32 persist_data_size;       /* The number of bytes allocated to each copy of the
				     * persistent data table*/
  
    __le8  kdf_type; /* The key derivation function used. */
  
    /* scrypt parameters. See www.tarsnap.com/scrypt/scrypt.pdf */
    __le8  N_factor; /* (1 << N) */
    __le8  r_factor; /* (1 << r) */
    __le8  p_factor; /* (1 << p) */
    __le64 encrypted_upto; /* If we are in state CRYPT_ENCRYPTION_IN_PROGRESS and
			      we have to stop (e.g. power low) this is the last
			      encrypted 512 byte sector.*/
    __le8  hash_first_block[SHA256_DIGEST_LENGTH]; /* When CRYPT_ENCRYPTION_IN_PROGRESS
						      set, hash of first block, used
						      to validate before continuing*/
  
    /* key_master key, used to sign the derived key which is then used to generate
     * the intermediate key
     * This key should be used for no other purposes! We use this key to sign unpadded 
     * data, which is acceptable but only if the key is not reused elsewhere. */
    __le8 keymaster_blob[KEYMASTER_BLOB_SIZE];
    __le32 keymaster_blob_size;
  
    /* Store scrypt of salted intermediate key. When decryption fails, we can
       check if this matches, and if it does, we know that the problem is with the
       drive, and there is no point in asking the user for more passwords.
  
       Note that if any part of this structure is corrupt, this will not match and
       we will continue to believe the user entered the wrong password. In that
       case the only solution is for the user to enter a password enough times to
       force a wipe.
  
       Note also that there is no need to worry about migration. If this data is
       wrong, we simply won't recognise a right password, and will continue to
       prompt. On the first password change, this value will be populated and
       then we will be OK.
    */
    unsigned char scrypted_intermediate_key[SCRYPT_LEN];
  
    /* sha of this structure with this element set to zero
       Used when encrypting on reboot to validate structure before doing something
       fatal
    */
    unsigned char sha256[SHA256_DIGEST_LENGTH];
  };
}

#endif
