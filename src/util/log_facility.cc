// log_facility.cc: log facility using /dev/kmsg, avaliable for reading on next boot
//
// 2018 KireinaHoro <i@jsteward.moe>

#include <cstring>
#include <fstream>
#include <unistd.h>
#include <sys/mount.h>

#include "log_facility.h"

// Note that before we set up the log device properly, there will be no way
// to store the log even if an error happens. We're making rough assumptions
// and not checking for other possible errors here.

namespace klog {
  char *process_message(log_level lvl, const char *message) {
    const char *log_level_string = log_level_strings[lvl];

    auto total_len = std::strlen(log_prefix) + std::strlen(log_level_string) + std::strlen(message) + 1;
    auto ret_buffer = new char[total_len];
    std::strcpy(ret_buffer, log_prefix);
    std::strcpy(ret_buffer, log_level_string);
    std::strcpy(ret_buffer, message);

    return ret_buffer;
  }

  bool check_log_facility() {
    // if the stat syscall succeeded, /dev/kmsg exists
    // we can assume that a working /dev has been set up
    if (setup_finished) { return true; }
    struct stat buffer;
    setup_finished = (stat(kmsg_path, &buffer) == 0);
    return setup_finished;
  }

  bool setup_log_facility() {
    // check if things are already set up
    if (check_log_facility()) {
      return true;
    }

    // mount devtmpfs at /dev
    // check mountpoint
    struct stat buffer;
    if (stat("/dev", &buffer) != 0) {
      // mountpoint doesn't exist
      mkdir("/dev", S_IRWXU);
    } else if (!S_ISDIR(buffer.st_mode)) {
      // something's in the way -- unlink it
      unlink("/dev");
      mkdir("/dev", S_IRWXU);
    }
    // mount devtmpfs
    mount("none", "/dev", "devtmpfs", "");

    return check_log_facility();
  }

  void log_to_facility_with_level(log_level lvl, const char *message) {
    if (!setup_log_facility()) {
      // the log facility failed to get ready -- return
      return;
    }
    std::ofstream out(kmsg_path);
    auto *buffer = process_message(message);
    out << buffer;
    delete buffer;
  }
}
    
      
