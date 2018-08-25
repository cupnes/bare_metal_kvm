#pragma once

#define SERIAL_IO_TX	0x0402

void serial_handle_io(struct kvm_run *run);
