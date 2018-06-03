#include <stdio.h>
#include <linux/kvm.h>
#include <sys/types.h>
#include "util.h"
#include "crtc.h"

void io_handle(struct kvm_run *run)
{
	if (run->io.port == CON_IO_WRITE)
		con_handle_io(run);
}
