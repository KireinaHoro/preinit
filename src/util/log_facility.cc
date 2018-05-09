// log_facility.cc: log facility using /dev/kmsg, avaliable for reading on next boot
//
// 2018 KireinaHoro <i@jsteward.moe>

#include <string>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>

#include "log_facility.h"

// Note that before we set up the log device properly, there will be no way
// to store the log even if an error happens. We're making rough assumptions
// and not checking for other possible errors here.

namespace klog {
  bool setup_finished = false;

  const std::string kmsg_path = "/dev/kmsg";
  const std::string log_prefix = "new_era: ";

  const std::string log_level_strings[] = {
    "[DEBUG] ",
    "[INFO ] ",
    "[WARN ] ",
    "[FATAL] ",
  };

  std::string process_message(log_level lvl, std::string message) {
    return log_prefix + log_level_strings[lvl] + message;
  }

  bool check_log_facility() {
    // if the stat syscall succeeded, /dev/kmsg exists
    // we can assume that a working /dev has been set up
    if (setup_finished) { return true; }
    struct stat buffer;
    setup_finished = (stat(kmsg_path.c_str(), &buffer) == 0);
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
    mount("none", "/dev", "devtmpfs", 0, "");

    return check_log_facility();
  }

  void log_to_facility_with_level(log_level lvl, std::string message) {
    if (!setup_log_facility()) {
      // the log facility failed to get ready -- return
      return;
    }
    std::ofstream out(kmsg_path);
    auto buffer = process_message(lvl, message);
    out << buffer << std::endl;
  }
}
    
      
