/*
 * i.MX General Purpose Input/Output
 *
 * Copyright (C) 2015 Colin Leitner <colin.leitner@gmail.com>
 *
 * This code is licensed under the GPL.
 */

#include "qemu/bitops.h"
#include "hw/gpio/imx-gpio.h"

#ifndef IMX_GPIO_ERR_DEBUG
#define IMX_GPIO_ERR_DEBUG 0
#endif

#define DB_PRINT_L(lvl, fmt, args...) do {\
    if (IMX_GPIO_ERR_DEBUG >= lvl) {\
        qemu_log("imx-gpio: %s:" fmt, __func__, ## args);\
    } \
} while (0);

#define DB_PRINT(fmt, args...) DB_PRINT_L(1, fmt, ## args)

static void imx_gpio_update_out(IMXGPIOState *s)
{
    uint32_t changed;
    uint32_t mask;
    uint32_t out;
    int i;

    DB_PRINT("dir = %08X, data = %08X\n", s->gdir, s->dr);

    /*
     * We assume non-driven (GDIR = 0) outputs float high. On real hardware this
     * could be different, but here we have to decide which value to set the
     * output IRQ to if the direction register switches the I/O to an input.
     */
    /* FIXME: This is board dependent. */
    out = (s->dr & s->gdir) | ~s->gdir;

    changed = s->old_out ^ out;
    s->old_out = out;

    for (i = 0; i < IMX_GPIO_NUM_IOS; i++) {
        mask = 1 << i;
        if (changed & mask) {
            DB_PRINT("Set output %d = %d\n", i, (out & mask) != 0);
            qemu_set_irq(s->out[i], (out & mask) != 0);
        }
    }
}

static int imx_gpio_icr(IMXGPIOState *s, int i)
{
    int reg = i * 2 / 32;
    int offset = (i * 2) % 32;
    return extract32(s->icr[reg], offset, 2);
}

static void imx_gpio_update_in(IMXGPIOState *s)
{
    uint32_t changed;
    uint32_t mask;
    int i;

    changed = s->old_psr ^ s->psr;
    s->old_psr = s->psr;

    for (i = 0; i < IMX_GPIO_NUM_IOS; i++) {
        mask = 1 << i;
        if (changed & mask) {
            DB_PRINT("Changed input %d = %d\n", i, (s->psr & mask) != 0);

            /* EDGE_SEL overrides ICR */
            if (s->edge_sel & mask) {
                /* Any edge triggers the interrupt */
                s->isr |= mask;
            } else {
                int icr = imx_gpio_icr(s, i);

                switch (icr) {
                case 0:
                    /* Low level */
                    s->isr |= ~s->psr & mask;
                    break;
                case 1:
                    /* High level */
                    s->isr |= s->psr & mask;
                    break;
                case 2:
                    /* Rising edge */
                    s->isr |= s->psr & mask;
                    break;
                case 3:
                    /* Falling edge */
                    s->isr |= ~s->psr & mask;
                    break;
                default: assert(0); break;
                }
            }
        }
    }

    /* Now update individual interrupts */
    changed &= s->isr & s->imr;
    for (i = 0; i < IMX_GPIO_NUM_IOS; i++) {
        mask = 1 << i;
        if (changed & mask) {
            qemu_set_irq(s->irqs[i], (s->isr & mask) != 0);
        }
    }
}

static void imx_gpio_set_in_irqs(IMXGPIOState *s)
{
    qemu_set_irq(s->irq_lower, extract32(s->isr & s->imr, 0, 16) != 0);
    qemu_set_irq(s->irq_upper, extract32(s->isr & s->imr, 16, 16) != 0);
}

static void imx_gpio_update(IMXGPIOState *s)
{
    imx_gpio_update_out(s);
    imx_gpio_update_in(s);

    imx_gpio_set_in_irqs(s);
}

static uint64_t imx_gpio_read(void *opaque, hwaddr offset,
                               unsigned size)
{
    IMXGPIOState *s = opaque;

    DB_PRINT("read from %" HWADDR_PRIx "\n", offset);

    switch (offset) {
    case IMX_GPIO_REG_DR:
        return s->dr;
    case IMX_GPIO_REG_GDIR:
        return s->gdir;
    case IMX_GPIO_REG_PSR:
        return s->psr;
    case IMX_GPIO_REG_ICR1:
        return s->icr[0];
    case IMX_GPIO_REG_ICR2:
        return s->icr[1];
    case IMX_GPIO_REG_IMR:
        return s->imr;
    case IMX_GPIO_REG_ISR:
        return s->isr;
    case IMX_GPIO_REG_EDGE_SEL:
        return s->edge_sel;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                     "imx_gpio_read: Bad offset %" HWADDR_PRIx "\n", offset);
        break;
    }

    return 0;
}

static void imx_gpio_write(void *opaque, hwaddr offset,
                            uint64_t value, unsigned size)
{
    IMXGPIOState *s = opaque;

    DB_PRINT("write %08" PRIx64  " to %" HWADDR_PRIx "\n", value, offset);

    switch (offset) {
    case IMX_GPIO_REG_DR:
        s->dr = value;
        break;
    case IMX_GPIO_REG_GDIR:
        s->gdir = value;
        break;
    case IMX_GPIO_REG_ICR1:
        s->icr[0] = value;
        break;
    case IMX_GPIO_REG_ICR2:
        s->icr[1] = value;
        break;
    case IMX_GPIO_REG_IMR:
        s->imr = value;
        break;
    case IMX_GPIO_REG_ISR:
        s->isr &= ~value;
        break;
    case IMX_GPIO_REG_EDGE_SEL:
        s->edge_sel = value;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                     "imx_gpio_write: Bad offset %" HWADDR_PRIx "\n", offset);
        return;
    }

    imx_gpio_update(s);
}

static void imx_gpio_reset(DeviceState *d)
{
    IMXGPIOState *s = IMX_GPIO(d);

    s->dr =         0x00000000;
    s->gdir =       0x00000000;
    s->old_psr =    s->psr;
    s->icr[0] =     0x00000000;
    s->icr[1] =     0x00000000;
    s->imr =        0x00000000;
    s->isr =        0x00000000;
    s->edge_sel =   0x00000000;

    imx_gpio_update(s);
}

static void imx_gpio_set_irq(void *opaque, int irq, int level)
{
    IMXGPIOState *s = opaque;
    uint32_t mask = 1 << irq;

    s->psr &= ~mask;
    if (level)
        s->psr |= mask;

    imx_gpio_update_in(s);

    imx_gpio_set_in_irqs(s);
}

static const MemoryRegionOps imx_gpio_ops = {
    .read = imx_gpio_read,
    .write = imx_gpio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void imx_gpio_realize(DeviceState *dev, Error **errp)
{
    IMXGPIOState *s = IMX_GPIO(dev);
    int i;

    memory_region_init_io(&s->iomem, OBJECT(s), &imx_gpio_ops, s, "imx-gpio", 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
    sysbus_init_irq(SYS_BUS_DEVICE(dev), &s->irq_lower);
    sysbus_init_irq(SYS_BUS_DEVICE(dev), &s->irq_upper);
    for (i = 0; i < IMX_GPIO_NUM_IOS; i++) {
        sysbus_init_irq(SYS_BUS_DEVICE(dev), &s->irqs[i]);
    }

    qdev_init_gpio_out_named(dev, s->out, "out", IMX_GPIO_NUM_IOS);
    s->old_out = 0x00000000;
    qdev_init_gpio_in_named(dev, imx_gpio_set_irq, "in", IMX_GPIO_NUM_IOS);
    s->psr = 0x00000000;
}

static void imx_gpio_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = imx_gpio_realize;
    dc->reset = imx_gpio_reset;
}

static const TypeInfo imx_gpio_info = {
    .name          = TYPE_IMX_GPIO,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(IMXGPIOState),
    .class_init    = imx_gpio_class_init,
};

static void imx_gpio_register_type(void)
{
    type_register_static(&imx_gpio_info);
}

type_init(imx_gpio_register_type)
