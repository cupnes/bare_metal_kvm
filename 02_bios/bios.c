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
#include "mem.h"
#include "bios.h"

#define BIOS_PATH		"/usr/share/seabios/bios.bin"
#define BIOS_MEM_SIZE		0x20000	/* 128KB */
#define BIOS_LEGACY_ADDR	0xe0000
#define BIOS_SHADOW_ADDR	0xfffe0000

void load_bios(int vmfd)
{
	int r;

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
}
