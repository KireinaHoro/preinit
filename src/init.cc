// init.cc: entry point from initramfs.
//
// 2018 KireinaHoro <i@jsteward.moe>

#include "util/log_facility.h"

int main() {
  SLOGD("init started.");
  
  SLOGI("Old pond");
  SLOGI("Frog jumps in");
  SLOGI("Sound of water");
  SLOGI("-- Matsuo Basho");

  SLOGW("This is the end of the demo init.");

  SLOGE("End of init reached -- kernel panic will now occur");
  return 0;
}
