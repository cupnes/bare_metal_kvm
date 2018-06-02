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
#include <unistd.h>

#include "debug.h"
#include "common.h"
#include "con.h"

#define RAM_SIZE	0x200000000
#define IDENTITY_BASE	0xfffbc000
#define VCPU_ID		0
#define BIOS_PATH	"/usr/share/seabios/bios.bin"
#define BIOS_MEM_SIZE	0x20000	/* 128KB */
#define BIOS_LEGACY_ADDR	0xe0000
#define BIOS_SHADOW_ADDR	0xfffe0000
#define VGABIOS_PATH	"/usr/share/seabios/vgabios-stdvga.bin"
#define VGABIOS_MEM_SIZE	0x20000	/* 128KB */
#define VGABIOS_ADDR	0xC0000
#define SIZE_640KB	0xA0000
#define SIZE_128KB	0x20000
#define SIZE_8GB	0x200000000 /* > 0x0e0000000 */

int kvm_set_user_memory_region(
	int vmfd, unsigned long long guest_phys_addr,
	unsigned long long memory_size, unsigned long long userspace_addr);
void handle_io(struct kvm_run *run);

int main(void) {
	int r;

	/* /dev/kvmをopen */
	int kvmfd = open("/dev/kvm", O_RDWR);
	assert(kvmfd != -1, "open /dev/kvm");

	/* VM作成 */
	int vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0); /* 標準的に第3引数へ0指定 */
	assert(vmfd != -1, "KVM_CREATE_VM");

	/* TSSを設定 */
	r = ioctl(vmfd, KVM_SET_TSS_ADDR, IDENTITY_BASE + 0x1000);
	assert(r != -1, "KVM_SET_TSS_ADDR");

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

	/* bios.binを開く */
	int biosfd = open(BIOS_PATH, O_RDONLY);
	assert(biosfd != -1, "open bios");

	/* bios.binのファイルサイズ取得 */
	int bios_size = lseek(biosfd, 0, SEEK_END);
	assert(bios_size != -1, "lseek 0 SEEK_END");
	r = lseek(biosfd, 0, SEEK_SET);
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
	assert(r != -1, "KVM_SET_USER_MEMORY_REGION bios legacy");

	/* BIOS用の領域をゲストへマップ(shadow) */
	r = kvm_set_user_memory_region(vmfd, BIOS_SHADOW_ADDR, BIOS_MEM_SIZE,
				       (unsigned long long)bios_mem);
	assert(r != -1, "KVM_SET_USER_MEMORY_REGION bios shadow");

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

	/* ゲストの0x00100000 - 0xdfffffff(3.5GB - 1MB)にメモリをマップ */
	addr = mmap(0, 0xE0000000 - 0x100000, PROT_EXEC | PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	r = kvm_set_user_memory_region(vmfd, 0x100000, 0xE0000000 - 0x100000,
				       (unsigned long long)addr);

	/* ゲストの0x100000000 - 0x21fefffff(4.5GB)にメモリをマップ */
	addr = mmap(0, RAM_SIZE - 0xE0100000, PROT_EXEC | PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	r = kvm_set_user_memory_region(vmfd, 0x100000000, RAM_SIZE - 0xE0100000,
				       (unsigned long long)addr);

	/* vgabiosバイナリを開く */
	int vgabiosfd = open(VGABIOS_PATH, O_RDONLY);
	assert(vgabiosfd != -1, "open vgabios");

	/* vgabiosバイナリのサイズを取得 */
	int vgabios_size = lseek(vgabiosfd, 0, SEEK_END);
	assert(vgabiosfd != -1, "lseek SEEK_END 0 vgabios");
	r = lseek(vgabiosfd, 0, SEEK_SET);
	assert(r != -1, "lseek SEEK_SET 0 vgabios");

	/* vgabiosバイナリを読み出す */
	void *vgabios_rom_data = malloc(0x20000);
	assert(vgabios_rom_data != NULL, "malloc vgabios_rom_data");
	r = read(vgabiosfd, vgabios_rom_data, vgabios_size);
	assert(r != -1, "read vgabios_rom_data");

	/* vgabiosバイナリを閉じる */
	r = close(vgabiosfd);
	assert(r != -1, "close vgabios");

	/* RAMデータへコピー */
	void *vgabios_ram_data = malloc(0x20000);
	assert(vgabios_ram_data != NULL, "malloc vgabios_ram_data");
	memcpy(vgabios_ram_data, vgabios_rom_data, 0x20000);

	/* フロッピーディスクイメージを開く */
	FILE *fdcfd = fopen("floppy.img","rb");
	assert(fdcfd != NULL, "fopen floppy.img");

	/* KVM_RUN */
	unsigned char is_exit = 0;
	struct kvm_regs regs;
	struct kvm_sregs sregs;
	while (is_exit == 0) {
		DEBUG_PRINT("Enter: KVM_RUN\n\n");
		r = ioctl(vcpufd, KVM_RUN, 0);
		assert(r != -1, "KVM_RUN");
		DEBUG_PRINT("Exit: KVM_RUN(0x%08x)\n", run->exit_reason);

		r = ioctl(vcpufd, KVM_GET_REGS, &regs);
		assert(r != -1, "KVM_GET_REGS");
		r = ioctl(vcpufd, KVM_GET_SREGS, &sregs);
		assert(r != -1, "KVM_GET_SREGS");
		DEBUG_PRINT("cs=0x%016llx, rip=0x%016llx, rflags=0x%016llx\n",
		       sregs.cs.base, regs.rip, regs.rflags);

		switch (run->exit_reason) {
		case KVM_EXIT_HLT:
			DEBUG_PRINT("KVM_EXIT_HLT\n");
			is_exit = 1;
			break;
		case KVM_EXIT_IO:
			DEBUG_PRINT("KVM_EXIT_IO\n");
			DEBUG_PRINT("direction=%d, size=%d, port=0x%04x, count=0x%08x, data_offset=0x%016llx\n",
			       run->io.direction, run->io.size, run->io.port,
			       run->io.count, run->io.data_offset);
			handle_io(run);
			break;
		default:
			DEBUG_PRINT("undefined exit_reason\n");
			while (1);
			break;
		}
	}

	return 0;
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
	return ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &usmem);
}

void handle_io(struct kvm_run *run)
{
	if (run->io.port == CON_IO_WRITE)
		con_handle_io(run);
}
