#include <sys/mman.h>
#include "common.h"

void ram_install(int vmfd, unsigned long long base, size_t size)
{
	int r;

	void *addr = mmap(0, size, PROT_EXEC | PROT_READ | PROT_WRITE,
			  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	assert(addr != MAP_FAILED, "ram: mmap");

	r = kvm_set_user_memory_region(vmfd, base, size,
				       (unsigned long long)addr);
	assert(r != -1, "ram: KVM_SET_USER_MEMORY_REGION");
}
