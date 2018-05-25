#include <sys/mman.h>
#include "util.h"

void ram_install(int vmfd, unsigned long long base, size_t size)
{
	void *addr = mmap(0, size, PROT_EXEC | PROT_READ | PROT_WRITE,
			  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	kvm_set_user_memory_region(vmfd, base, size, (unsigned long long)addr);
}
