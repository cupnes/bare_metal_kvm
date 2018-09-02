#pragma once

#include <linux/kvm.h>

#include "types.h"

typedef struct io_handler {
	uint16  base,mask;
	void   *opaque;
	uint8  (*inb )(struct io_handler *,uint16 address);
	uint16 (*inw )(struct io_handler *,uint16 address);
	uint32 (*inl )(struct io_handler *,uint16 address);
	void   (*outb)(struct io_handler *,uint16 address,uint8 val);
	void   (*outw)(struct io_handler *,uint16 address,uint16 val);
	void   (*outl)(struct io_handler *,uint16 address,uint32 val);

	char *pzDesc;
} io_handler_t;

void io_handle(struct kvm_run *run);
void io_init(void);
