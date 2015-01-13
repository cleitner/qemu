/*
 * i.MX53 TrustZone Aware Interrupt Controller TZIC
 *
 * Copyright (C) 2015 Colin Leitner <colin.leitner@gmail.com>
 *
 * This code is licensed under the GPL.
 */

#ifndef HW_INTC_IMX_TZIC_H
#define HW_INTC_IMX_TZIC_H

#include "hw/sysbus.h"

#define TYPE_IMX_TZIC "imx-tzic"
#define IMX_TZIC(obj) OBJECT_CHECK(IMXTZICState, (obj), TYPE_IMX_TZIC)

#define IMX_TZIC_NUM_IRQS       128
#define IMX_TZIC_IRQ_PER_WORD   32
#define IMX_TZIC_IRQ_WORDS      (IMX_TZIC_NUM_IRQS / IMX_TZIC_IRQ_PER_WORD)
#define IMX_TZIC_PRIO_BITS      8
#define IMX_TZIC_PRIO_PER_WORD  (32 / IMX_TZIC_PRIO_BITS)
#define IMX_TZIC_PRIO_WORDS     (IMX_TZIC_NUM_IRQS / IMX_TZIC_PRIO_PER_WORD)

typedef struct {
    SysBusDevice parent_obj;

    MemoryRegion iomem;

    qemu_irq irq;
    qemu_irq fiq;
    qemu_irq wakeup_req;

    uint32_t intin[IMX_TZIC_IRQ_WORDS];

    uint32_t intctrl;
    uint32_t priomask;
    uint32_t syncctrl;
    uint32_t non_secure[IMX_TZIC_IRQ_WORDS];
    uint32_t enabled[IMX_TZIC_IRQ_WORDS];
    uint32_t source[IMX_TZIC_IRQ_WORDS];
    uint32_t prio[IMX_TZIC_PRIO_WORDS];
    uint32_t pending[IMX_TZIC_IRQ_WORDS];
    uint32_t hi_pending[IMX_TZIC_IRQ_WORDS];
    uint32_t wakeup[IMX_TZIC_IRQ_WORDS];
} IMXTZICState;

#define IMX_TZIC_REG_INTCTRL        0x000
#define IMX_TZIC_REG_INTTYPE        0x004
#define IMX_TZIC_REG_PRIOMASK       0x00C
#define IMX_TZIC_REG_SYNCCTRL       0x010
#define IMX_TZIC_REG_DSMINT         0x014
#define IMX_TZIC_REG_INTSECX(x)     (0x080 + (x) * 4)
#define IMX_TZIC_REG_ENSETX(x)      (0x100 + (x) * 4)
#define IMX_TZIC_REG_ENCLEARX(x)    (0x180 + (x) * 4)
#define IMX_TZIC_REG_SRCSETX(x)     (0x200 + (x) * 4)
#define IMX_TZIC_REG_SRCCLEARX(x)   (0x280 + (x) * 4)
#define IMX_TZIC_REG_PRIORITYX(x)   (0x400 + (x) * 4)
#define IMX_TZIC_REG_PNDX(x)        (0xD00 + (x) * 4)
#define IMX_TZIC_REG_HIPNDX(x)      (0xD80 + (x) * 4)
#define IMX_TZIC_REG_WAKEUPX(x)     (0xE00 + (x) * 4)
#define IMX_TZIC_REG_SWINT          0xF00

#endif
