#include <stdlib.h>
#include "util.h"

void ram_install(int vmfd, unsigned long long base, size_t size)
{
	void *addr;
	posix_memalign(&addr, 4096, size);
	kvm_set_user_memory_region(vmfd, base, size, (unsigned long long)addr);
}
