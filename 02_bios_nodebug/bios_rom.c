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
	/* BIOSのバイナリを開く */
	int biosfd = open(path, O_RDONLY);

	/* BIOSバイナリのファイルサイズ取得 */
	int bios_size = lseek(biosfd, 0, SEEK_END);
	lseek(biosfd, 0, SEEK_SET);

	/* BIOSバイナリを配置する領域を確保 */
	void *bios_mem;
	posix_memalign(&bios_mem, 4096, BIOS_MEM_SIZE);

	/* BIOSバイナリを確保した領域へロード */
	read(biosfd, bios_mem, bios_size);

	/* BIOSをロードした領域をVMへマップ(legacy) */
	kvm_set_user_memory_region(vmfd, BIOS_LEGACY_ADDR, BIOS_MEM_SIZE,
				   (unsigned long long)bios_mem);

	/* BIOSをロードした領域をVMへマップ(shadow) */
	kvm_set_user_memory_region(vmfd, BIOS_SHADOW_ADDR, BIOS_MEM_SIZE,
				   (unsigned long long)bios_mem);
}
