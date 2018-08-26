#include <stdio.h>
#include <linux/kvm.h>
#include "util.h"

void serial_handle_io(struct kvm_run *run)
{
	unsigned int i;

	switch (run->io.direction) {
	case KVM_EXIT_IO_IN:
		for (i = 0; i < run->io.count; i++) {
			switch (run->io.size) {
			case 1:
				*(unsigned char *)((unsigned char *)run + run->io.data_offset) = 0;
				break;
			case 2:
				*(unsigned short *)((unsigned char *)run + run->io.data_offset) = 0;
				break;
			case 4:
				*(unsigned short *)((unsigned char *)run + run->io.data_offset) = 0;
				break;
			}
			run->io.data_offset += run->io.size;
		}
		break;

	case KVM_EXIT_IO_OUT:
		for (i = 0; i < run->io.count; i++) {
			assert(run->io.size == 1, "serial: Undefined IO size");
			/* putchar(*(char *)((unsigned char *)run + run->io.data_offset)); */
			run->io.data_offset += run->io.size;
		}
		break;
	}
}
