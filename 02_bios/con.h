#pragma once

#define CON_IO_WRITE	0x0402

void con_handle_io(struct kvm_run *run);
