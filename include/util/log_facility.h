// log_facility.h: log facility using /dev/kmsg, avaliable for reading on next boot
//
// 2018 KireinaHoro <i@jsteward.moe>

#ifndef LOG_FACILITY_H
#define LOG_FACILITY_H

namespace klog {
  enum log_level {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_FATAL,
  };
  // log to log facility, setting up things if necessary
  void log_to_facility_with_level(log_level lvl, const char *message);
}

#define SLOGD(msg) klog::log_to_facility_with_level(klog::LOG_DEBUG, msg)
#define SLOGI(msg) klog::log_to_facility_with_level(klog::LOG_INFO, msg)
#define SLOGW(msg) klog::log_to_facility_with_level(klog::LOG_WARN, msg)
#define SLOGE(msg) klog::log_to_facility_with_level(klog::LOG_FATAL, msg)

#endif
