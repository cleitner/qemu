/*
 * Zynq General Purpose IO
 *
 * Copyright (C) 2014 Colin Leitner <colin.leitner@gmail.com>
 *
 * Based on the PL061 model:
 *   Copyright (c) 2007 CodeSourcery.
 *   Written by Paul Brook
 *
 * This code is licensed under the GPL.
 */

/*
 * We model all banks as if they were fully populated. MIO pins are usually
 * limited to 54 pins, but this is probably device dependent and shouldn't
 * cause too much trouble. One noticeable difference is the reset value of
 * INT_TYPE_1, which is 0x003fffff according to the TRM and 0xffffffff here.
 *
 * The output enable pins are not modeled.
 */

#include "hw/gpio/zynq-gpio.h"
#include "qemu/bitops.h"

#ifndef ZYNQ_GPIO_ERR_DEBUG
#define ZYNQ_GPIO_ERR_DEBUG 0
#endif

#define DB_PRINT_L(lvl, fmt, args...) do {\
    if (ZYNQ_GPIO_ERR_DEBUG >= lvl) {\
        qemu_log("zynq-gpio: %s:" fmt, __func__, ## args);\
    } \
} while (0);

#define DB_PRINT(fmt, args...) DB_PRINT_L(1, fmt, ## args)

static void zynq_gpio_update_out(ZynqGPIOBank *bank)
{
    uint32_t changed;
    uint32_t mask;
    uint32_t out;
    int i;

    DB_PRINT("dir = %d, data = %d\n", bank->dir, bank->out_data);

    /*
     * We assume non-driven (DIRM = 0) outputs float high. On real hardware this
     * could be different, but here we have to decide which value to set the
     * output IRQ to if the direction register switches the I/O to an input.
     */
    /* FIXME: This is board dependent. */
    out = (bank->out_data & bank->dir) | ~bank->dir;

    changed = bank->old_out_data ^ out;
    bank->old_out_data = out;

    for (i = 0; i < ZYNQ_GPIO_IOS_PER_BANK; i++) {
        mask = 1 << i;
        if (changed & mask) {
            DB_PRINT("Set output %d = %d\n", i, (out & mask) != 0);
            qemu_set_irq(bank->out[i], (out & mask) != 0);
        }
    }
}

static void zynq_gpio_update_in(ZynqGPIOBank *bank)
{
    uint32_t changed;
    uint32_t mask;
    int i;

    changed = bank->old_in_data ^ bank->in_data;
    bank->old_in_data = bank->in_data;

    for (i = 0; i < ZYNQ_GPIO_IOS_PER_BANK; i++) {
        mask = 1 << i;
        if (changed & mask) {
            DB_PRINT("Changed input %d = %d\n", i, (bank->in_data & mask) != 0);

            if (bank->itype & mask) {
                /* Edge interrupt */
                if (bank->iany & mask) {
                    /* Any edge triggers the interrupt */
                    bank->istat |= mask;
                } else {
                    /* Edge is selected by INT_POLARITY */
                    bank->istat |= ~(bank->in_data ^ bank->ipolarity) & mask;
                }
            }
        }
    }

    /* Level interrupt */
    bank->istat |= ~(bank->in_data ^ bank->ipolarity) & ~bank->itype;

    DB_PRINT("istat = %08X\n", bank->istat);
}

static void zynq_gpio_set_in_irq(ZynqGPIOState *s)
{
    int b;
    uint32_t istat = 0;

    for (b = 0; b < ZYNQ_GPIO_BANKS; b++) {
        ZynqGPIOBank *bank = &s->banks[b];

        istat |= bank->istat & ~bank->imask;
    }

    DB_PRINT("IRQ = %d\n", istat != 0);

    qemu_set_irq(s->irq, istat != 0);
}

static void zynq_gpio_update(ZynqGPIOState *s)
{
    int b;

    for (b = 0; b < ZYNQ_GPIO_BANKS; b++) {
        ZynqGPIOBank *bank = &s->banks[b];

        zynq_gpio_update_out(bank);
        zynq_gpio_update_in(bank);
    }

    zynq_gpio_set_in_irq(s);
}

static uint64_t zynq_gpio_read(void *opaque, hwaddr offset,
                               unsigned int size)
{
    ZynqGPIOState *s = opaque;
    int b;
    int shift;
    ZynqGPIOBank *bank;

    switch (offset) {
    case ZYNQ_GPIO_REG_MASK_DATA_0_LSW ... ZYNQ_GPIO_REG_MASK_DATA_3_MSW:
        b = extract32(offset - ZYNQ_GPIO_REG_MASK_DATA_0_LSW, 3, 2);
        shift = (offset & 0x8) ? 16 : 0;
        return extract32(s->banks[b].mask_data, shift, 16);

    case ZYNQ_GPIO_REG_DATA_0 ... ZYNQ_GPIO_REG_DATA_3:
        b = (offset - ZYNQ_GPIO_REG_DATA_0) / 4;
        return s->banks[b].out_data;

    case ZYNQ_GPIO_REG_DATA_0_RO ... ZYNQ_GPIO_REG_DATA_3_RO:
        b = (offset - ZYNQ_GPIO_REG_DATA_0_RO) / 4;
        return s->banks[b].in_data;
    }

    if (offset < 0x204 || offset > 0x2e4) {
        qemu_log_mask(LOG_GUEST_ERROR,
                     "zynq_gpio_read: Bad offset %" HWADDR_PRIx "\n", offset);
        return 0;
    }

    b = (offset - 0x200) / 0x40;
    bank = &s->banks[b];

    switch (offset - ZYNQ_GPIO_BANK_OFFSET(b)) {
    case ZYNQ_GPIO_BANK_REG_DIRM:
        return bank->dir;
    case ZYNQ_GPIO_BANK_REG_OEN:
        return bank->oen;
    case ZYNQ_GPIO_BANK_REG_INT_MASK:
        return bank->imask;
    case ZYNQ_GPIO_BANK_REG_INT_EN:
        return 0;
    case ZYNQ_GPIO_BANK_REG_INT_DIS:
        return 0;
    case ZYNQ_GPIO_BANK_REG_INT_STAT:
        return bank->istat;
    case ZYNQ_GPIO_BANK_REG_INT_TYPE:
        return bank->itype;
    case ZYNQ_GPIO_BANK_REG_INT_POLARITY:
        return bank->ipolarity;
    case ZYNQ_GPIO_BANK_REG_INT_ANY:
        return bank->iany;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                     "zynq_gpio_read: Bad offset %" HWADDR_PRIx "\n", offset);
        return 0;
    }
}

static void zynq_gpio_mask_data(ZynqGPIOBank *bank, int bit_offset,
                                uint32_t mask_data)
{
    DB_PRINT("mask data offset = %d, mask_data = %08X\n", bit_offset, mask_data);

    /* MASK_DATA registers are R/W on their data part */
    bank->mask_data = deposit32(bank->mask_data, bit_offset, 16, mask_data);
    bank->out_data = deposit32(bank->out_data, bit_offset, 16, mask_data);

    zynq_gpio_update_out(bank);
}

static void zynq_gpio_data(ZynqGPIOBank *bank, uint32_t data)
{
    bank->out_data = data;
    zynq_gpio_update_out(bank);
}

static void zynq_gpio_write(void *opaque, hwaddr offset,
                            uint64_t value, unsigned int size)
{
    ZynqGPIOState *s = opaque;
    int b;
    int shift;
    ZynqGPIOBank *bank;

    switch (offset) {
    case ZYNQ_GPIO_REG_MASK_DATA_0_LSW ... ZYNQ_GPIO_REG_MASK_DATA_3_MSW:
        b = extract32(offset - ZYNQ_GPIO_REG_MASK_DATA_0_LSW, 3, 2);
        shift = (offset & 0x8) ? 16 : 0;
        zynq_gpio_mask_data(&s->banks[b], shift, value);
        return;

    case ZYNQ_GPIO_REG_DATA_0 ... ZYNQ_GPIO_REG_DATA_3:
        b = (offset - ZYNQ_GPIO_REG_DATA_0) / 4;
        zynq_gpio_data(&s->banks[b], value);
        return;

    case ZYNQ_GPIO_REG_DATA_0_RO ... ZYNQ_GPIO_REG_DATA_3_RO:
        return;
    }

    if (offset < 0x204 || offset > 0x2e4) {
        qemu_log_mask(LOG_GUEST_ERROR,
                     "zynq_gpio_write: Bad offset %" HWADDR_PRIx "\n", offset);
        return;
    }

    b = (offset - 0x200) / 0x40;
    bank = &s->banks[b];

    switch (offset - ZYNQ_GPIO_BANK_OFFSET(b)) {
    case ZYNQ_GPIO_BANK_REG_DIRM:
        bank->dir = value;
        break;
    case ZYNQ_GPIO_BANK_REG_OEN:
        bank->oen = value;
        qemu_log_mask(LOG_UNIMP,
                     "zynq_gpio_write: Output enable lines not implemented\n");
        break;
    case ZYNQ_GPIO_BANK_REG_INT_MASK:
        return;
    case ZYNQ_GPIO_BANK_REG_INT_EN:
        bank->imask &= ~value;
        break;
    case ZYNQ_GPIO_BANK_REG_INT_DIS:
        bank->imask |= value;
        break;
    case ZYNQ_GPIO_BANK_REG_INT_STAT:
        bank->istat &= ~value;
        break;
    case ZYNQ_GPIO_BANK_REG_INT_TYPE:
        bank->itype = value;
        break;
    case ZYNQ_GPIO_BANK_REG_INT_POLARITY:
        bank->ipolarity = value;
        break;
    case ZYNQ_GPIO_BANK_REG_INT_ANY:
        bank->iany = value;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                     "zynq_gpio_write: Bad offset %" HWADDR_PRIx "\n", offset);
        return;
    }

    zynq_gpio_update(s);
}

static void zynq_gpio_reset(DeviceState *dev)
{
    ZynqGPIOState *s = ZYNQ_GPIO(dev);
    int b;

    for (b = 0; b < ZYNQ_GPIO_BANKS; b++) {
        ZynqGPIOBank *bank = &s->banks[b];
        int i;

        bank->mask_data = 0x00000000;
        bank->dir = 0x00000000;
        bank->oen = 0x00000000;
        bank->imask = 0x00000000;
        bank->istat = 0x00000000;
        bank->itype = 0xffffffff;
        bank->ipolarity = 0x00000000;
        bank->iany = 0x00000000;

        bank->out_data = 0x00000000;
        bank->old_out_data = 0x00000000;
        for (i = 0; i < ZYNQ_GPIO_IOS_PER_BANK; i++) {
            qemu_set_irq(bank->out[i], 0);
        }

        bank->old_in_data = bank->in_data;
    }
}

static void zynq_gpio_set_irq(ZynqGPIOBank *bank, int irq, int level)
{
    uint32_t mask = 1 << irq;

    bank->in_data &= ~mask;
    if (level)
        bank->in_data |= mask;

    zynq_gpio_update_in(bank);

    zynq_gpio_set_in_irq(bank->parent);
}

static void zynq_gpio_set_bank0_irq(void *opaque, int irq, int level)
{
    ZynqGPIOState *s = opaque;
    zynq_gpio_set_irq(&s->banks[0], irq, level);
}

static void zynq_gpio_set_bank1_irq(void *opaque, int irq, int level)
{
    ZynqGPIOState *s = opaque;
    zynq_gpio_set_irq(&s->banks[1], irq, level);
}

static void zynq_gpio_set_bank2_irq(void *opaque, int irq, int level)
{
    ZynqGPIOState *s = opaque;
    zynq_gpio_set_irq(&s->banks[2], irq, level);
}

static void zynq_gpio_set_bank3_irq(void *opaque, int irq, int level)
{
    ZynqGPIOState *s = opaque;
    zynq_gpio_set_irq(&s->banks[3], irq, level);
}

static const MemoryRegionOps zynq_gpio_ops = {
    .read = zynq_gpio_read,
    .write = zynq_gpio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void zynq_gpio_realize(DeviceState *dev, Error **errp)
{
    ZynqGPIOState *s = ZYNQ_GPIO(dev);
    int b;

    memory_region_init_io(&s->iomem, OBJECT(s), &zynq_gpio_ops, s, "zynq-gpio", 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
    sysbus_init_irq(SYS_BUS_DEVICE(dev), &s->irq);

    for (b = 0; b < ZYNQ_GPIO_BANKS; b++) {
        ZynqGPIOBank *bank = &s->banks[b];
        char name[16];

        memset(bank, 0, sizeof(*bank));

        bank->parent = s;

        snprintf(name, sizeof(name), "bank%d_out", b);
        qdev_init_gpio_out_named(dev, bank->out, name, ZYNQ_GPIO_IOS_PER_BANK);
        /*
         * TODO: it would be nice if we could pass the bank to the handler. This
         * would allow us to remove the 4 callbacks and use zynq_gpio_set_irq
         * directly.
         */
#if 0
        snprintf(name, sizeof(name), "bank%d_in", b);
        qdev_init_gpio_in_named(dev, zynq_gpio_set_irq, name, ZYNQ_GPIO_IOS_PER_BANK, bank);
#endif
    }
    qdev_init_gpio_in_named(dev, zynq_gpio_set_bank0_irq, "bank0_in", ZYNQ_GPIO_IOS_PER_BANK);
    qdev_init_gpio_in_named(dev, zynq_gpio_set_bank1_irq, "bank1_in", ZYNQ_GPIO_IOS_PER_BANK);
    qdev_init_gpio_in_named(dev, zynq_gpio_set_bank2_irq, "bank2_in", ZYNQ_GPIO_IOS_PER_BANK);
    qdev_init_gpio_in_named(dev, zynq_gpio_set_bank3_irq, "bank3_in", ZYNQ_GPIO_IOS_PER_BANK);
}

static void zynq_gpio_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = zynq_gpio_realize;
    dc->reset = zynq_gpio_reset;
}

static const TypeInfo zynq_gpio_info = {
    .name          = TYPE_ZYNQ_GPIO,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(ZynqGPIOState),
    .class_init    = zynq_gpio_class_init,
};

static void zynq_gpio_register_type(void)
{
    type_register_static(&zynq_gpio_info);
}

type_init(zynq_gpio_register_type)
