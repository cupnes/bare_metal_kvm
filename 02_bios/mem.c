#include <linux/kvm.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "common.h"
#include "bios.h"

#define RAM_SIZE	0x200000000

#define SIZE_640KB	0xA0000
#define SIZE_128KB	0x20000
#define SIZE_8GB	0x200000000 /* > 0x0e0000000 */

int kvm_set_user_memory_region(
	int vmfd, unsigned long long guest_phys_addr,
	unsigned long long memory_size, unsigned long long userspace_addr)
{
	static unsigned int kvm_usmem_slot = 0;

	struct kvm_userspace_memory_region usmem;
	usmem.slot = kvm_usmem_slot++;
	usmem.guest_phys_addr = guest_phys_addr;
	usmem.memory_size = memory_size;
	usmem.userspace_addr = userspace_addr;
	usmem.flags = 0;
	return ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &usmem);
}

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

	/* ゲストの0x000c0000 - 0x000e0000(128KB)にメモリをマップ */
	addr = mmap(0, SIZE_128KB, PROT_EXEC | PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	assert(addr != MAP_FAILED, "mmap mem 128KB");
	r = kvm_set_user_memory_region(vmfd, VGABIOS_ADDR, SIZE_128KB,
				       (unsigned long long)addr);
	assert(r != -1, "KVM_SET_USER_MEMORY_REGION 128KB");
}
