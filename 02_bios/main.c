#include <stdio.h>
#include <linux/kvm.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "debug.h"
#include "common.h"
#include "mem.h"
#include "bios.h"
#include "io.h"

#define VCPU_ID		0
#define BIOS_PATH	"/usr/share/seabios/bios.bin"

int main(void) {
	int r;

	/* /dev/kvmをopen */
	int kvmfd = open("/dev/kvm", O_RDWR);
	assert(kvmfd != -1, "open /dev/kvm");

	/* VM作成 */
	int vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0); /* 標準的に第3引数へ0指定 */
	assert(vmfd != -1, "KVM_CREATE_VM");

	/* IRQCHIPを作成 */
	r = ioctl(vmfd, KVM_CREATE_IRQCHIP);
	assert(r == 0, "KVM_CREATE_IRQCHIP");

	/* PITを作成 */
	r = ioctl(vmfd, KVM_CREATE_PIT);
	assert(r == 0, "KVM_CREATE_PIT");

	/* VCPU作成 */
	int vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, VCPU_ID);
	assert(vcpufd != -1, "KVM_CREATE_VCPU");
	size_t mmap_size = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL);
	assert(mmap_size != 0, "KVM_GET_VCPU_MMAP_SIZE");
	struct kvm_run *run = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE,
				   MAP_SHARED, vcpufd, 0);
	assert(mmap_size != (unsigned long long)MAP_FAILED, "mmap vcpu");

	/* BIOSのロード */
	load_bios(vmfd, BIOS_PATH);

	/* RAMのセットアップ */
	setup_mem(vmfd);

	/* KVM_RUN */
	while (1) {
		DEBUG_PRINT("Enter: KVM_RUN\n\n");
		r = ioctl(vcpufd, KVM_RUN, 0);
		assert(r != -1, "KVM_RUN");
		DEBUG_PRINT("Exit: KVM_RUN(0x%08x)\n", run->exit_reason);

		dump_regs(vcpufd);

		switch (run->exit_reason) {
		case KVM_EXIT_IO:
			handle_io(run);
			break;
		default:
			assert(0, "undefined exit_reason\n");
		}
	}

	return 0;
}
