#pragma once

#define PCI_IO_A	0x0cf8
#define PCI_IO_B	0x0cfc

void pci_handle_io(struct kvm_run *run);
