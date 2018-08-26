#include <stdio.h>
#include <linux/kvm.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "util.h"
#include "ram.h"
#include "bios_rom.h"
#include "io.h"

#define VCPU_ID		0

#define BIOS_PATH	"/usr/share/seabios/bios.bin"

#define RAM1_BASE	0x00000000
#define RAM1_SIZE	0x000A0000 /* 640KB */
#define RAM2_BASE	0x000C0000 /* VGA BIOS Base Address */
#define RAM2_SIZE	0x00020000 /* 128KB */
#define RAM3_BASE	0x00100000
#define RAM3_SIZE	0xDFF00000

int main(void) {
	int r;

	/* KVMを開く */
	int kvmfd = open("/dev/kvm", O_RDWR);
	assert(kvmfd != -1, "open /dev/kvm");

	/* VM作成 */
	int vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0); /* 標準的に第3引数へ0指定 */
	assert(vmfd != -1, "KVM_CREATE_VM");

	/* 割り込みコントローラ作成、VMへ設定 */
	r = ioctl(vmfd, KVM_CREATE_IRQCHIP);
	assert(r == 0, "KVM_CREATE_IRQCHIP");

	/* タイマー作成、VMへ設定 */
	r = ioctl(vmfd, KVM_CREATE_PIT);
	assert(r == 0, "KVM_CREATE_PIT");

	/* CPU作成、VMへ設定 */
	int vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, VCPU_ID);
	assert(vcpufd != -1, "KVM_CREATE_VCPU");
	size_t mmap_size = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL);
	assert(mmap_size != 0, "KVM_GET_VCPU_MMAP_SIZE");
	struct kvm_run *run = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE,
				   MAP_SHARED, vcpufd, 0);
	assert(mmap_size != (unsigned long long)MAP_FAILED, "mmap vcpu");

	/* BIOS ROM作成、VMへ設定 */
	bios_rom_install(vmfd, BIOS_PATH);

	/* RAM作成、VMへ設定 */
	ram_install(vmfd, RAM1_BASE, RAM1_SIZE);
	ram_install(vmfd, RAM2_BASE, RAM2_SIZE);
	ram_install(vmfd, RAM3_BASE, RAM3_SIZE);

	/* VM実行 */
	while (1) {
		DEBUG_PRINT("Enter: KVM_RUN\n\n");
		r = ioctl(vcpufd, KVM_RUN, 0);
		assert(r != -1, "KVM_RUN");
		DEBUG_PRINT("Exit: KVM_RUN(0x%08x)\n", run->exit_reason);

		dump_regs(vcpufd);

		switch (run->exit_reason) {
		case KVM_EXIT_IO:
			io_handle(run);
			break;

		default:
			fflush(stdout);
			assert(0, "undefined exit_reason\n");
		}
	}

	return 0;
}
