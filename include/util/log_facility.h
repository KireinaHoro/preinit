// log_facility.h: log facility using /dev/kmsg, avaliable for reading on next boot
//
// 2018 KireinaHoro <i@jsteward.moe>

#ifndef LOG_FACILITY_H
#define LOG_FACILITY_H

#include <string>

namespace klog {
  enum log_level {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_FATAL,
  };
  // log to log facility, setting up things if necessary
  void log_to_facility_with_level(log_level lvl, std::string message);
}

#include <fmt/format.h>

#define SLOGD(...) klog::log_to_facility_with_level(klog::LOG_DEBUG, fmt::format(__VA_ARGS__))
#define SLOGI(...) klog::log_to_facility_with_level(klog::LOG_INFO, fmt::format(__VA_ARGS__))
#define SLOGW(...) klog::log_to_facility_with_level(klog::LOG_WARN, fmt::format(__VA_ARGS__))
#define SLOGE(...) klog::log_to_facility_with_level(klog::LOG_FATAL, fmt::format(__VA_ARGS__))

#endif
