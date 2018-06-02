#pragma once

#define QEMU_PARAVIRT_IO_A	0x0510
#define QEMU_PARAVIRT_IO_B	0x0511

void qemu_paravirt_handle_io(struct kvm_run *run);
