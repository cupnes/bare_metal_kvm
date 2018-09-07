#include <stdlib.h>
#include "util.h"

void ram_install(int vmfd, unsigned long long base, size_t size)
{
	int r;
	void *addr;

	r = posix_memalign(&addr, 4096, size);
	assert(r == 0, "ram: posix_memalign");

	r = kvm_set_user_memory_region(vmfd, base, size,
				       (unsigned long long)addr);
	assert(r != -1, "ram: KVM_SET_USER_MEMORY_REGION");
}
