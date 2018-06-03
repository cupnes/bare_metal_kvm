#include <linux/kvm.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

#define BIOS_MEM_SIZE		0x00020000	/* 128KB */
#define BIOS_LEGACY_ADDR	0x000e0000
#define BIOS_SHADOW_ADDR	0xfffe0000

void bios_rom_install(int vmfd, char *path)
{
	int r;

	/* BIOSのバイナリを開く */
	int biosfd = open(path, O_RDONLY);
	assert(biosfd != -1, "bios: open");

	/* BIOSバイナリのファイルサイズ取得 */
	int bios_size = lseek(biosfd, 0, SEEK_END);
	assert(bios_size != -1, "bios: lseek 0 SEEK_END");
	r = lseek(biosfd, 0, SEEK_SET);
	assert(r != -1, "bios: lseek 0 SEEK_SET");
	assert(bios_size <= BIOS_MEM_SIZE, "bios: binary size exceeds 128KB.");

	/* BIOSバイナリを配置する領域を確保 */
	void *bios_mem;
	r = posix_memalign(&bios_mem, 4096, BIOS_MEM_SIZE);
	assert(r == 0, "bios: posix_memalign");

	/* BIOSバイナリを確保した領域へロード */
	r = read(biosfd, bios_mem, bios_size);
	assert(r != -1, "bios: read");

	/* BIOSをロードした領域をVMへマップ(legacy) */
	r = kvm_set_user_memory_region(vmfd, BIOS_LEGACY_ADDR, BIOS_MEM_SIZE,
				       (unsigned long long)bios_mem);
	assert(r != -1, "bios: KVM_SET_USER_MEMORY_REGION(legacy)");

	/* BIOSをロードした領域をVMへマップ(shadow) */
	r = kvm_set_user_memory_region(vmfd, BIOS_SHADOW_ADDR, BIOS_MEM_SIZE,
				       (unsigned long long)bios_mem);
	assert(r != -1, "bios: KVM_SET_USER_MEMORY_REGION(shadow)");
}
