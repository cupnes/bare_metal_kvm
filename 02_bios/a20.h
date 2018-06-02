#pragma once

#define A20_IO	0x0092

void a20_handle_io(struct kvm_run *run);
