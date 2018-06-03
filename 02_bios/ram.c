#include <sys/mman.h>

#include "common.h"
#include "bios.h"

#define RAM_SIZE	0x200000000

#define SIZE_640KB	0xA0000
#define SIZE_128KB	0x20000
#define SIZE_8GB	0x200000000 /* > 0x0e0000000 */

void setup_mem(int vmfd)
{
	int r;

	/* ゲストの0x00000000 - 0x0009ffff(640KB)にメモリをマップ */
	void *addr = mmap(0, SIZE_640KB, PROT_EXEC | PROT_READ | PROT_WRITE,
			  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	assert(addr != MAP_FAILED, "mmap mem 640KB");
	r = kvm_set_user_memory_region(vmfd, 0, SIZE_640KB,
				       (unsigned long long)addr);
	assert(r != -1, "KVM_SET_USER_MEMORY_REGION mem 640KB");

	/* ゲストの0x000c0000 - 0x000dffff(128KB)にメモリをマップ */
	addr = mmap(0, SIZE_128KB, PROT_EXEC | PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	assert(addr != MAP_FAILED, "mmap mem 128KB");
	r = kvm_set_user_memory_region(vmfd, VGABIOS_ADDR, SIZE_128KB,
				       (unsigned long long)addr);
	assert(r != -1, "KVM_SET_USER_MEMORY_REGION 128KB");
}
