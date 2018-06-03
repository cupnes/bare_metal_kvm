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
#define CODE_BASE	0x00000000fffffff0

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(...)	fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)	((void)0)
#endif

int kvm_set_user_memory_region(
	int vmfd, unsigned long long guest_phys_addr,
	unsigned long long memory_size, unsigned long long userspace_addr);
void assert(unsigned char condition, char *msg);
void dump_regs(int vcpufd);

int main(void) {
	int r;

	/* /dev/kvmをopen */
	int kvmfd = open("/dev/kvm", O_RDWR);
	assert(kvmfd != -1, "open /dev/kvm");

	/* VM作成 */
	int vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0); /* 標準的に第3引数へ0指定 */
	assert(vmfd != -1, "KVM_CREATE_VM");

	/* メモリを用意 */
	unsigned char *mem = mmap(NULL, code_bin_len, PROT_READ | PROT_WRITE,
				  MAP_SHARED | MAP_ANONYMOUS | MAP_NORESERVE,
				  -1, 0);
	assert(mem != MAP_FAILED, "rom: mmap");
	memcpy(mem, code_bin, code_bin_len);  /* メモリへコードを配置 */
	r = kvm_set_user_memory_region(vmfd, CODE_BASE, code_bin_len,
				       (unsigned long long)mem);
	assert(r != -1, "rom: KVM_SET_USER_MEMORY_REGION");

	/* VCPU作成 */
	int vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, 0);
							/* 第3引数は作成するvcpu id */
	assert(vcpufd != -1, "cpu: KVM_CREATE_VCPU");
	size_t mmap_size = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL);
	assert(mmap_size != 0, "cpu: KVM_GET_VCPU_MMAP_SIZE");
	struct kvm_run *run = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE,
				   MAP_SHARED, vcpufd, 0);
	assert(run != MAP_FAILED, "cpu: mmap");

	/* struct kvm_sregs sregs;  /\* セグメントレジスタ初期値設定 *\/ */
	/* ioctl(vcpufd, KVM_GET_SREGS, &sregs); */
	/* sregs.cs.base = 0; */
	/* sregs.cs.selector = 0; */
	/* ioctl(vcpufd, KVM_SET_SREGS, &sregs); */

	/* struct kvm_regs regs = {  /\* ステータスレジスタ初期値設定 *\/ */
	/* 	.rip = 0x0, */
	/* 	.rflags = 0x02, /\* RFLAGS初期状態 *\/ */
	/* }; */
	/* ioctl(vcpufd, KVM_SET_REGS, &regs); */

	/* dump_regs(vcpufd); */

	/* 実行 */
	while (1) {
		r = ioctl(vcpufd, KVM_RUN, NULL);
		assert(r != -1, "KVM_RUN");

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

void assert(unsigned char condition, char *msg)
{
	if (!condition) {
		perror(msg);
		exit(EXIT_FAILURE);
	}
}

void dump_segment_register(struct kvm_segment *s)
{
	DEBUG_PRINT("base=%016llx limit=%08x selector=%04x type=%02x\n",
		    s->base, s->limit, s->selector, s->type);
}

void dump_regs(int vcpufd)
{
	int r;

	struct kvm_regs regs;
	r = ioctl(vcpufd, KVM_GET_REGS, &regs);
	assert(r != -1, "KVM_GET_REGS");

	DEBUG_PRINT("rax=%016llx rbx=%016llx rcx=%016llx rdx=%016llx\n",
		    regs.rax, regs.rbx, regs.rcx, regs.rdx);
	DEBUG_PRINT("rsi=%016llx rdi=%016llx rsp=%016llx rbp=%016llx\n",
		    regs.rsi, regs.rdi, regs.rsp, regs.rbp);
	DEBUG_PRINT("r8=%016llx r9=%016llx r10=%016llx r11=%016llx\n",
		    regs.r8, regs.r9, regs.r10, regs.r11);
	DEBUG_PRINT("r12=%016llx r13=%016llx r14=%016llx r15=%016llx\n",
		    regs.r12, regs.r13, regs.r14, regs.r15);
	DEBUG_PRINT("rip=%016llx rflags=%016llx\n", regs.rip, regs.rflags);

	struct kvm_sregs sregs;
	r = ioctl(vcpufd, KVM_GET_SREGS, &sregs);
	assert(r != -1, "KVM_GET_SREGS");

	DEBUG_PRINT("cs ");
	dump_segment_register(&sregs.cs);
	DEBUG_PRINT("ds ");
	dump_segment_register(&sregs.ds);
	DEBUG_PRINT("es ");
	dump_segment_register(&sregs.es);
	DEBUG_PRINT("ss ");
	dump_segment_register(&sregs.ss);
	DEBUG_PRINT("tr ");
	dump_segment_register(&sregs.tr);
	DEBUG_PRINT("cr0=%016llx\n", sregs.cr0);
}
