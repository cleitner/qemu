#ifndef HW_NET_MCF_FEC_H
#define HW_NET_MCF_FEC_H

#include "net/net.h"

void mcf_fec_init(struct MemoryRegion *sysmem, NICInfo *nd,
                  hwaddr base, qemu_irq *irqs);

#define MCF_FEC_REG_MIIGSK_CFGR 0x300
#define MCF_FEC_REG_MIIGSK_ENR  0x308

#endif
