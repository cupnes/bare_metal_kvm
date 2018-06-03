#pragma once

void assert(unsigned char condition, char *msg);
int kvm_set_user_memory_region(
	int vmfd, unsigned long long guest_phys_addr,
	unsigned long long memory_size, unsigned long long userspace_addr);
