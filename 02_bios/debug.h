#pragma once

/* #define DEBUG */

#ifdef DEBUG
#define DEBUG_PRINT(...)	fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)	((void)0)
#endif

void dump_io(struct kvm_run *run);
void dump_regs(int vcpufd);
