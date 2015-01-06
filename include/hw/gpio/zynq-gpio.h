/*
 * Zynq General Purpose IO
 *
 * Copyright (C) 2014 Colin Leitner <colin.leitner@gmail.com>
 *
 * This code is licensed under the GPL.
 */

#ifndef HW_ZYNQ_GPIO_H
#define HW_ZYNQ_GPIO_H

#include "hw/sysbus.h"

#define TYPE_ZYNQ_GPIO "zynq-gpio"
#define ZYNQ_GPIO(obj) OBJECT_CHECK(ZynqGPIOState, (obj), TYPE_ZYNQ_GPIO)

#define ZYNQ_GPIO_BANKS         4
#define ZYNQ_GPIO_IOS_PER_BANK  32

typedef struct {
    struct ZynqGPIOState *parent;

    uint32_t mask_data;
    uint32_t out_data;
    uint32_t old_out_data;
    uint32_t in_data;
    uint32_t old_in_data;
    uint32_t dir;
    uint32_t oen;
    uint32_t imask;
    uint32_t istat;
    uint32_t itype;
    uint32_t ipolarity;
    uint32_t iany;

    qemu_irq out[ZYNQ_GPIO_IOS_PER_BANK];
} ZynqGPIOBank;

typedef struct ZynqGPIOState {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    ZynqGPIOBank banks[ZYNQ_GPIO_BANKS];
    qemu_irq irq;
} ZynqGPIOState;

#define ZYNQ_GPIO_REG_MASK_DATA_0_LSW   0x000
#define ZYNQ_GPIO_REG_MASK_DATA_0_MSW   0x004
#define ZYNQ_GPIO_REG_MASK_DATA_1_LSW   0x008
#define ZYNQ_GPIO_REG_MASK_DATA_1_MSW   0x00C
#define ZYNQ_GPIO_REG_MASK_DATA_2_LSW   0x010
#define ZYNQ_GPIO_REG_MASK_DATA_2_MSW   0x014
#define ZYNQ_GPIO_REG_MASK_DATA_3_LSW   0x018
#define ZYNQ_GPIO_REG_MASK_DATA_3_MSW   0x01C
#define ZYNQ_GPIO_REG_DATA_0            0x040
#define ZYNQ_GPIO_REG_DATA_1            0x044
#define ZYNQ_GPIO_REG_DATA_2            0x048
#define ZYNQ_GPIO_REG_DATA_3            0x04C
#define ZYNQ_GPIO_REG_DATA_0_RO         0x060
#define ZYNQ_GPIO_REG_DATA_1_RO         0x064
#define ZYNQ_GPIO_REG_DATA_2_RO         0x068
#define ZYNQ_GPIO_REG_DATA_3_RO         0x06C

/*
 * Oddly enough these registers are neatly grouped per bank and not interleaved
 * like the data registers
 */
#define ZYNQ_GPIO_BANK_OFFSET(bank)     (0x200 + 0x40 * (bank))
#define ZYNQ_GPIO_BANK_REG_DIRM         0x04
#define ZYNQ_GPIO_BANK_REG_OEN          0x08
#define ZYNQ_GPIO_BANK_REG_INT_MASK     0x0C
#define ZYNQ_GPIO_BANK_REG_INT_EN       0x10
#define ZYNQ_GPIO_BANK_REG_INT_DIS      0x14
#define ZYNQ_GPIO_BANK_REG_INT_STAT     0x18
#define ZYNQ_GPIO_BANK_REG_INT_TYPE     0x1C
#define ZYNQ_GPIO_BANK_REG_INT_POLARITY 0x20
#define ZYNQ_GPIO_BANK_REG_INT_ANY      0x24

#endif
