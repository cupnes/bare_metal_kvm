#include <stdio.h>
#include <string.h>

#include "io.h"
#include "pci.h"
#include "util.h"

// Maximum functions (devs*functions) per bus, and 
// maximum number of busses
#define MAX_FPB (8*32)
#define MAX_BUS 1

static struct {
  int configspace;
  uint8 bus,dev,fn,index;
} state;

static int devs = 0;
static pcidev_t *dev[8*32];

static pcidev_t *getDevice(void) {
  int i;

  for(i=0;i<MAX_FPB;i++) {
    if( dev[i] != NULL ) {
      //LOG(" IsTrue?  %i==%i %i==%i %i==0",dev[i]->bus,state.bus,dev[i]->dev,state.dev,dev[i]->fn);
      if( (dev[i]->bus == state.bus) &&
	  (dev[i]->dev == state.dev) &&
	  (dev[i]->fn  == state.fn) ) {
	//LOG(" Match. Returning %p",dev[i]);
	return dev[i];
      }
    }
  }

  return NULL;
}

static uint8 pci_inb(struct io_handler *hdl,uint16 port) {
  pcidev_t *dev;

  if( port >= 0x4 ) {
    dev = getDevice();
    if( dev == NULL ) {
      return 0xFF;
    }
    return dev->readb(state.index + port - 0x4);
  }
  assert(0,"Unimplemented inb");
  return 0xFF;
}
static uint16 pci_inw(struct io_handler *hdl,uint16 port) {
  pcidev_t *dev;

  if( port >= 0x4 ) {
    dev = getDevice();
    if( dev == NULL ) {
      return 0xFFFF;
    }
    return dev->readw(state.index + port - 0x4);
  }
  assert(0,"Unimplemented inw");

  return 0;
}
static uint32 pci_inl(struct io_handler *hdl,uint16 port) {
  if(port == 0) {
    uint32 ret;

    ret  = state.configspace << 31;
    ret |= (state.bus & 0x0F) << 16;
    ret |= (state.dev & 0x1F) << 11;
    ret |= state.index & 0xFF;

    DEBUG_PRINT("PCI Conf Readback (cs=0x%x)",state.configspace);
    return ret;
  } else {
    pcidev_t *dev;

    if( port >= 0x4 ) {
      dev = getDevice();
      if( dev == NULL ) {
	//LOG("Read from non-existant PCI device");
	return 0xFFFF;
      }
      return dev->readl(state.index + port - 0x4);
    }
    assert(0,"Unimplemented inl");
  }
  return 0xFFFFFFFF;
}

static void  pci_outw(struct io_handler *hdl,uint16 port,uint16 val) {
  if( port <= 3 ) { // Byte access to config reg?
    assert(0,"Unimplemented write");
  } else {
    pcidev_t *dev;

    dev = getDevice();
    if( dev == NULL ) {
      //LOG("Write to non-existant PCI device");
      return;
    }
    //ASSERT(0,"OMG!");
    assert(dev->writew != NULL,"NULL writew handler");
    dev->writew(state.index + port - 0x4,val);
    return;
  }
}

static void  pci_outl(struct io_handler *hdl,uint16 port,uint32 val) {
  if( port == 0 ) {
    state.configspace = val>>31;
    state.index = val & 0xFF;
    state.bus   = (val >> 16) & 0x0F;
    state.dev   = (val >> 11) & 0x1F;
    state.fn    = (val >>  8) & 0x07;

  } else {
    pcidev_t *dev;

    assert(state.configspace == 1,
	   "Configuration space not implemented");

    dev = getDevice();
    if( dev == NULL ) {
      DEBUG_PRINT("Write to non-existant PCI device");
      return;
    }
    return dev->writel(state.index + port - 0x4,val);
  }
}

void x86_pci_init(struct io_handler *hdl) {
  memset(&state,0,sizeof(state));
  memset(dev,0,sizeof(pcidev_t*)*MAX_FPB);
  devs = 0;

  hdl->base   = 0x0CF8;
  hdl->mask   = ~7;
  hdl->inb    = pci_inb;
  hdl->inw    = pci_inw;
  hdl->inl    = pci_inl;
  hdl->outb   = NULL;
  hdl->outw   = pci_outw;
  hdl->outl   = pci_outl;
  hdl->opaque = NULL;
}

int pci_register_dev(int bus,const pcidev_t *newdev) {
  int i;

  assert(bus == -1, "No allowing set busses right now");
  assert(newdev != NULL, "Registering NULL devices?");

  for(i=0;i<MAX_FPB;i++) {
    if( dev[i] == NULL ) {
      dev[i] = (pcidev_t*)newdev;
      dev[i]->bus = 0;
      dev[i]->dev = devs++;
      dev[i]->fn  = 0;

      DEBUG_PRINT("Assigned address B%i, D%i, FN%i",dev[i]->bus,dev[i]->dev,dev[i]->fn);

      return 0;
    }
  }

  return -1;
}
