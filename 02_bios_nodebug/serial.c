#include <stdio.h>
#include <linux/kvm.h>
#include "util.h"

void serial_handle_io(struct kvm_run *run)
{
	if (run->io.direction == KVM_EXIT_IO_OUT) {
		unsigned int i;
		for (i = 0; i < run->io.count; i++) {
			putchar(*(char *)((unsigned char *)run + run->io.data_offset));
			run->io.data_offset += run->io.size;
		}
	}
}
