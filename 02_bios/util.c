#include <stdio.h>
#include <stdlib.h>
#include <linux/kvm.h>
#include <sys/ioctl.h>
#include "util.h"

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

void dump_io(struct kvm_run *run)
{
	switch (run->io.direction) {
	case KVM_EXIT_IO_IN:
		DEBUG_PRINT("KVM_EXIT_IO_IN\n");
		break;
	case KVM_EXIT_IO_OUT:
		DEBUG_PRINT("KVM_EXIT_IO_OUT\n");
		unsigned int i;
		for (i = 0; i < run->io.count; i++) {
			switch (run->io.size) {
			case 1:
				DEBUG_PRINT("%04x: %02x\n", run->io.port,
					    *(unsigned char *)((unsigned char *)run + run->io.data_offset));
				break;
			case 2:
				DEBUG_PRINT("%04x: %04x\n", run->io.port,
					    *(unsigned short *)((unsigned char *)run + run->io.data_offset));
				break;
			case 4:
				DEBUG_PRINT("%04x: %08x\n", run->io.port,
					    *(unsigned int *)((unsigned char *)run + run->io.data_offset));
				break;
			default:
				DEBUG_PRINT("%04x: io size=%d\n", run->io.port, run->io.size);
				assert(0, "Undefined IO size");
			}
		}
		break;
	default:
		assert(0, "Undefined IO direction\n");
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
