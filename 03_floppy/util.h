#pragma once

#include <linux/kvm.h>

/* #define DEBUG */

#ifdef DEBUG
#define DEBUG_PRINT(...)	fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)	((void)0)
#endif

int kvm_set_user_memory_region(
	int vmfd, unsigned long long guest_phys_addr,
	unsigned long long memory_size, unsigned long long userspace_addr);
void assert(unsigned char condition, char *msg);
void dump_io(struct kvm_run *run);
void dump_regs(int vcpufd);
