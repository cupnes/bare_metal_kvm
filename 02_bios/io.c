#include <stdio.h>
#include <linux/kvm.h>
#include <sys/types.h>
#include "util.h"
#include "crtc.h"

void io_handle(struct kvm_run *run)
{
	DEBUG_PRINT("io: KVM_EXIT_IO\n");
	DEBUG_PRINT("io: direction=%d, size=%d, port=0x%04x,",
		    run->io.direction, run->io.size, run->io.port);
	DEBUG_PRINT(" count=0x%08x, data_offset=0x%016llx\n",
		    run->io.count, run->io.data_offset);

	if (run->io.port == CON_IO_WRITE)
		con_handle_io(run);
}
