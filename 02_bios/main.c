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

#include "asm_code/code.h"

#define RAM_SIZE	0x1000
#define IDENTITY_BASE	0xfffbc000
#define VCPU_ID		0
#define BIOS_PATH	"/usr/share/seabios/bios.bin"

void assert(unsigned char condition, char *msg);

int main(void) {
	int r;

	/* /dev/kvmをopen */
	int kvmfd = open("/dev/kvm", O_RDWR);
	assert(kvmfd != -1, "open /dev/kvm");

	/* VM作成 */
	int vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0); /* 標準的に第3引数へ0指定 */
	assert(vmfd == 0, "KVM_CREATE_VM");

	/* TSSを設定 */
	r = ioctl(vmfd, KVM_SET_TSS_ADDR, IDENTITY_BASE + 0x1000);
	assert(r == 0, "KVM_SET_TSS_ADDR");

	/* IRQCHIPを作成 */
	r = ioctl(vmfd, KVM_CREATE_IRQCHIP);
	assert(r == 0, "KVM_CREATE_IRQCHIP");

	/* PITを作成 */
	r = ioctl(vmfd, KVM_CREATE_PIT);
	assert(r == 0, "KVM_CREATE_PIT");

	/* VCPU作成 */
	int vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, VCPU_ID);
	assert(vcpufd == 0, "KVM_CREATE_VCPU");
	size_t mmap_size = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL);
	assert(mmap_size == 0, "KVM_GET_VCPU_MMAP_SIZE");
	struct kvm_run *run = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE,
				   MAP_SHARED, vcpufd, 0);
	assert(mmap_size != MAP_FAILED, "mmap vcpu");

	/* bios.binを開く */
	int biosfd = open(BIOS_PATH, O_RDONLY);
	assert(biosfd != -1, "open bios");

	/* bios.binのファイルサイズ取得 */
	int bios_size = lseek(biosfd, 0, SEEK_END);
	assert(bios_size != -1, "lseek 0 SEEK_END");
	r = lseek(in, 0, SEEK_SET);
	assert(r != -1, "lseek 0 SEEK_SET");

	/* bios.binのファイルサイズを4KB倍数へ変換 */
	int bios_blks = bios_size;
	if (bios_blks & 0x00000fff) {
		bios_blks &= ~0x00000fff;
		bios_blks += 0x00001000;
	}
	assert(bios_blks <= 0x00020000, "bios size exceeds 128KB.");

	/* BIOS用の領域を確保 */
	void *tmp;
	r = posix_memalign(&tmp, 4096, 0x20000);
	assert(r == 0, "posix_memalign bios");

	/* bios.binを閉じる */
	/* r = close(biosfd); */
	/* assert(r != -1, "close bios"); */




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


	/* メモリを用意 */
	unsigned char *mem = mmap(NULL, RAM_SIZE, PROT_READ|PROT_WRITE,
				  MAP_SHARED|MAP_ANONYMOUS|MAP_NORESERVE,
				  -1, 0);
	memcpy(mem, code_bin, sizeof(code_bin));  /* メモリへコードを配置 */
	struct kvm_userspace_memory_region region = {
		.memory_size = RAM_SIZE,
		.userspace_addr = (unsigned long long)mem
	};
	ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region); /* VMへメモリを設定 */

	/* 実行 */
	while (1) {
		ioctl(vcpufd, KVM_RUN, NULL);

		/* 何かあるまで返ってこない */

		switch (run->exit_reason) {	/* 何かあった */
		case KVM_EXIT_HLT:	/* HLTした */
			/* printf("KVM_EXIT_HLT\n"); */
			return 0;

		case KVM_EXIT_IO:	/* IO操作 */
			if ((run->io.direction == KVM_EXIT_IO_OUT)
			    && (run->io.size == 1) && (run->io.port == 0x01)
			    && (run->io.count == 1)) {
				putchar(*(((char *)run) + run->io.data_offset));
			} else {
				fprintf(stderr, "unhandled KVM_EXIT_IO\n");
				return 1;
			}
		}
	}
}

void assert(unsigned char condition, char *msg)
{
	if (!condition) {
		perror(msg);
		exit(EXIT_FAILURE);
	}
}
