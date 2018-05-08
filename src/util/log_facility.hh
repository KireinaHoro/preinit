// log_facility.h: log facility using /dev/kmsg, avaliable for reading on next boot
//
// 2018 KireinaHoro <i@jsteward.moe>

#ifndef LOG_FACILITY_H
#define LOG_FACILITY_H

namespace klog {
const char *kmsg_path = "/dev/kmsg";
enum log_level {
	

// check if /dev/kmsg is available for use
bool check_log_facility();
// setup /dev/kmsg
bool setup_log_facility_aux();
// check and setup /dev/kmsg
bool setup_log_facility();

void log_to_facility_with_level(

}
#endif
