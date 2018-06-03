#include <stdio.h>
#include <stdlib.h>
#include <linux/kvm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>

#include "rom/rom.h"

#define RAM_SIZE 0x1000

int main(void) {
	/* /dev/kvmをopen */
	int kvmfd = open("/dev/kvm", O_RDWR);


	/* VM作成 */
	int vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0); /* 標準的に第3引数へ0指定 */


	/* メモリを用意 */
	unsigned char *mem = mmap(NULL, RAM_SIZE, PROT_READ|PROT_WRITE,
				  MAP_SHARED|MAP_ANONYMOUS|MAP_NORESERVE,
				  -1, 0);
	memcpy(mem, rom_bin, sizeof(rom_bin));  /* メモリへコードを配置 */
	struct kvm_userspace_memory_region region = {
		.memory_size = RAM_SIZE,
		.userspace_addr = (unsigned long long)mem
	};
	ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region); /* VMへメモリを設定 */


	/* VCPU作成 */
	int vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, 0);
						/* 第3引数は作成するvcpu id */
	size_t mmap_size = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL);
	struct kvm_run *run = mmap(NULL, mmap_size, PROT_READ|PROT_WRITE,
				   MAP_SHARED, vcpufd, 0);

	struct kvm_sregs sregs;  /* セグメントレジスタ初期値設定 */
	ioctl(vcpufd, KVM_GET_SREGS, &sregs);
	sregs.cs.base = 0;
	sregs.cs.selector = 0;
	ioctl(vcpufd, KVM_SET_SREGS, &sregs);

	struct kvm_regs regs = {  /* ステータスレジスタ初期値設定 */
		.rip = 0x0,
		.rflags = 0x02, /* RFLAGS初期状態 */
	};
	ioctl(vcpufd, KVM_SET_REGS, &regs);


	/* 実行 */
	unsigned char is_running = 1;
	while (is_running) {
		ioctl(vcpufd, KVM_RUN, NULL);

		/* 何かあるまで返ってこない */

		switch (run->exit_reason) {	/* 何かあった */
		case KVM_EXIT_HLT:	/* HLTした */
			/* printf("KVM_EXIT_HLT\n"); */
			is_running = 1;
			fflush(stdout);
			break;

		case KVM_EXIT_IO:	/* IO操作 */
			if (run->io.port == 0x01
			    && run->io.direction == KVM_EXIT_IO_OUT) {
				unsigned int i;
				for (i = 0; i < run->io.count; i++) {
					putchar(*(char *)((unsigned char *)run + run->io.data_offset));
					run->io.data_offset += run->io.size;
				}
			}
		}
	}

	return 0;
}
