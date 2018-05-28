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
#define BIOS_MEM_SIZE	0x20000	/* 128KB */
#define BIOS_LEGACY_ADDR	0xe0000
#define BIOS_SHADOW_ADDR	0xfffe0000
#define VGABIOS_PATH	"/usr/share/seabios/vgabios-stdvga.bin"
#define VGABIOS_MEM_SIZE	0x20000	/* 128KB */
#define VGABIOS_ADDR	0xC0000

void assert(unsigned char condition, char *msg);
int kvm_set_user_memory_region(
	int vmfd, unsigned long long guest_phys_addr,
	unsigned long long memory_size, unsigned long long userspace_addr);

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
	assert(bios_blks <= BIOS_MEM_SIZE, "bios size exceeds 128KB.");

	/* BIOS用の領域を確保 */
	void *bios_mem;
	r = posix_memalign(&bios_mem, 4096, BIOS_MEM_SIZE);
	assert(r == 0, "posix_memalign bios");

	/* bios.binをロード */
	r = read(biosfd, bios_mem, bios_size);
	assert(r != -1, "read bios.bin");

	/* BIOS用の領域をゲストへマップ(legacy) */
	r = kvm_set_user_memory_region(vmfd, BIOS_LEGACY_ADDR, BIOS_MEM_SIZE,
				       (unsigned long long)bios_mem);
	if (r != -1, "KVM_SET_USER_MEMORY_REGION bios legacy");

	/* BIOS用の領域をゲストへマップ(shadow) */
	r = kvm_set_user_memory_region(vmfd, BIOS_SHADOW_ADDR, BIOS_MEM_SIZE,
				       (unsigned long long)bios_mem);
	if (r != -1, "KVM_SET_USER_MEMORY_REGION bios shadow");

	/* bios.binを閉じる */
	/* r = close(biosfd); */
	/* assert(r != -1, "close bios"); */

	/* vgabiosバイナリを開く */
	int vgabiosfd = open(VGABIOS_PATH, O_RDONLY);
	assert(vgabiosfd != -1, "open vgabios");

	/* vgabiosバイナリのサイズを取得 */
	int vgabios_size = lseek(vgabiosfd, 0, SEEK_END);
	assert(vgabiosfd != -1, "lseek SEEK_END 0 vgabios");
	r = lseek(vgabiosfd, 0, SEEK_SET);
	assert(r != -1, "lseek SEEK_SET 0 vgabios");

	/* vgabiosバイナリサイズを4KBの倍数へ変換 */
	int vgabios_blks = vgabios_size;
	if (vgabios_size & 0x00000fff) {
		vgabios_blks &= ~0x00000fff;
		vgabios_blks += 0x00001000;
	}
	assert(vgabios_blks <= VGABIOS_MEM_SIZE, "vgabios size exceeds 128KB.");

	/* VGABIOS用の領域を確保 */
	void *vgabios_mem;
	r = posix_memalign(&vgabios_mem, 4096, VGABIOS_MEM_SIZE);
	assert(r == 0, "posix_memalign vgabios");

	/* vgabiosバイナリをロード */
	r = read(vgabiosfd, vgabios_mem, vgabios_size);
	assert(r != -1, "read vgabios");

	/* VGABIOS用の領域をゲストへマップ */
	r = kvm_set_user_memory_region(vmfd, VGABIOS_ADDR, VGABIOS_MEM_SIZE,
				       (unsigned long long)vgabios_mem);
	if (r != -1, "KVM_SET_USER_MEMORY_REGION vgabios");

	/* TODO: 次 map_ram(pSys,uNumMegsRAM); から */


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
	return ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &vgabios_usmem);
}
