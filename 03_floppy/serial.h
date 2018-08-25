#pragma once

#define SERIAL_IO_BASE	0x03f8
#define SERIAL_IO_MASK	0xfff8
#define SERIAL_IO_TX	0x0402

void serial_handle_io(struct kvm_run *run);
