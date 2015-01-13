/*
 * i.MX General Purpose Input/Output
 *
 * Copyright (C) 2015 Colin Leitner <colin.leitner@gmail.com>
 *
 * This code is licensed under the GPL.
 */

#ifndef HW_IMX_GPIO_H
#define HW_IMX_GPIO_H

#include "hw/sysbus.h"

#define TYPE_IMX_GPIO "imx-gpio"
#define IMX_GPIO(obj) OBJECT_CHECK(IMXGPIOState, (obj), TYPE_IMX_GPIO)

#define IMX_GPIO_NUM_IOS    32

typedef struct {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    qemu_irq out[IMX_GPIO_NUM_IOS];
    /* IRQ for 0..15  */
    qemu_irq irq_lower;
    /* IRQ for 16..31  */
    qemu_irq irq_upper;
    qemu_irq irqs[IMX_GPIO_NUM_IOS];

    uint32_t old_out;

    uint32_t dr;
    uint32_t gdir;
    uint32_t old_psr;
    uint32_t psr;
    uint32_t icr[2];
    uint32_t imr;
    uint32_t isr;
    uint32_t edge_sel;
} IMXGPIOState;

#define IMX_GPIO_REG_DR         0x000
#define IMX_GPIO_REG_GDIR       0x004
#define IMX_GPIO_REG_PSR        0x008
#define IMX_GPIO_REG_ICR1       0x00C
#define IMX_GPIO_REG_ICR2       0x010
#define IMX_GPIO_REG_IMR        0x014
#define IMX_GPIO_REG_ISR        0x018
#define IMX_GPIO_REG_EDGE_SEL   0x01C

#endif
