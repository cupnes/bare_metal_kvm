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

void dump_regs(int vcpufd)
{
	int r;

	struct kvm_regs regs;
	r = ioctl(vcpufd, KVM_GET_REGS, &regs);
	assert(r != -1, "KVM_GET_REGS");

	struct kvm_sregs sregs;
	r = ioctl(vcpufd, KVM_GET_SREGS, &sregs);
	assert(r != -1, "KVM_GET_SREGS");

	DEBUG_PRINT("cs=0x%016llx, rip=0x%016llx, rflags=0x%016llx\n",
		    sregs.cs.base, regs.rip, regs.rflags);
}
