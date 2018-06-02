#pragma once

#define RTC_IO_CMD	0x0070
#define RTC_IO_DATA	0x0071

void rtc_handle_io(struct kvm_run *run);
