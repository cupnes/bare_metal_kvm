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

int main(void) {
	/* KVMを開く */
	int kvmfd = open("/dev/kvm", O_RDWR);

	/* VM作成 */
	int vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0); /* 標準的に第3引数へ0指定 */

	/* 割り込みコントローラ作成、VMへ設定 */
	ioctl(vmfd, KVM_CREATE_IRQCHIP);

	/* タイマー作成、VMへ設定 */
	ioctl(vmfd, KVM_CREATE_PIT);

	/* CPU作成、VMへ設定 */
	int vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, VCPU_ID);
	size_t mmap_size = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL);
	struct kvm_run *run = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE,
				   MAP_SHARED, vcpufd, 0);

	/* BIOS ROM作成、VMへ設定 */
	bios_rom_install(vmfd, BIOS_PATH);

	/* RAM作成、VMへ設定 */
	ram_install(vmfd, RAM1_BASE, RAM1_SIZE);
	ram_install(vmfd, RAM2_BASE, RAM2_SIZE);

	/* VM実行 */
	while (1) {
		ioctl(vcpufd, KVM_RUN, 0);

		if (run->exit_reason == KVM_EXIT_IO)
			io_handle(run);
	}

	return 0;
}
