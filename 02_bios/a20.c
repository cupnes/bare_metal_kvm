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

#include "a20.h"

void a20_handle_io(struct kvm_run *run)
{
	unsigned int i;

	switch (run->io.direction) {
	case KVM_EXIT_IO_IN:
		for (i = 0; i < run->io.count; i++) {
			switch (run->io.size) {
			case 1:
				*(unsigned char *)((unsigned char *)run + run->io.data_offset) = 2;
				break;
			case 2:
				*(unsigned short *)((unsigned char *)run + run->io.data_offset) = 2;
				break;
			case 4:
				*(unsigned short *)((unsigned char *)run + run->io.data_offset) = 2;
				break;
			}
			run->io.data_offset += run->io.size;
		}
		break;

	case KVM_EXIT_IO_OUT:
		break;
	}
}
