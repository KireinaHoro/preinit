// log_facility.h: log facility using /dev/kmsg, avaliable for reading on next boot
//
// 2018 KireinaHoro <i@jsteward.moe>

#ifndef LOG_FACILITY_H
#define LOG_FACILITY_H

namespace klog {
  bool setup_finished = false;
  
  const char *kmsg_path = "/dev/kmsg";
  const char *log_prefix = "new_era ";
  enum log_level {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_FATAL,
  };
  const char *log_level_strings[] = {
    "[DEBUG] ",
    "[INFO ] ",
    "[WARN ] ",
    "[FATAL] ",
  };

  // concatencate the log message into string that will be written to kmsg
  // remember to free the pointer after use to avoid leakage
  char *process_message(log_level lvl, const char *message);

  // check if /dev/kmsg is available for use
  bool check_log_facility();
  // check and setup /dev/kmsg
  bool setup_log_facility();

  void log_to_facility_with_level(log_level lvl, const char *message);
}

#define SLOGD(msg) klog::log_to_facility_with_level(LOG_DEBUG, msg)
#define SLOGI(msg) klog::log_to_facility_with_level(LOG_INFO, msg)
#define SLOGW(msg) klog::log_to_facility_with_level(LOG_WARN, msg)
#define SLOGE(msg) klog::log_to_facility_with_level(LOG_FATAL, msg)

#endif
