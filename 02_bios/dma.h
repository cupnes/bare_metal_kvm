#pragma once

#define DMA_IO_WR_START_ADDR_26		0x0004
#define DMA_IO_R_COMMAND		0x0008
#define DMA_IO_WR_SINGLE_MASK_BIT	0x000a
#define DMA_IO_WR_MODE_REG		0x000b
#define DMA_IO_W_FLIPFLOP_RESET		0x000c
#define DMA_IO_WR_MASTER_CLEAR		0x000d
#define DMA_IO_A			0x0010

void dma_handle_io(struct kvm_run *run);
