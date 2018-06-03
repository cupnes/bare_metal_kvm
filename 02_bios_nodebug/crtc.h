#pragma once

#define CON_IO_WRITE	0x0402

void crtc_handle_io(struct kvm_run *run);
