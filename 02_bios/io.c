#include <stdio.h>
#include <linux/kvm.h>
#include <sys/types.h>

#include "debug.h"
#include "con.h"

void handle_io(struct kvm_run *run)
{
	DEBUG_PRINT("KVM_EXIT_IO\n");
	DEBUG_PRINT("direction=%d, size=%d, port=0x%04x, count=0x%08x, data_offset=0x%016llx\n",
		    run->io.direction, run->io.size, run->io.port,
		    run->io.count, run->io.data_offset);

	if (run->io.port == CON_IO_WRITE)
		con_handle_io(run);
}
