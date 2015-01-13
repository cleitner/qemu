/*
 * i.MX53 TrustZone Aware Interrupt Controller TZIC
 *
 * Copyright (C) 2015 Colin Leitner <colin.leitner@gmail.com>
 *
 * Based on the imx_avic model
 *
 * This code is licensed under the GPL.
 */

/*
 * We're actually not TrustZone aware and assume the CPU doesn't use TZ (QEMU
 * currently lacks TZ support anyway)
 *
 * This means that all operations are assumed to come from the secure
 * supervisor and enabling non-secure functionality is not supported.
 */

#include "hw/intc/imx-tzic.h"

#ifndef IMX_TZIC_ERR_DEBUG
#define IMX_TZIC_ERR_DEBUG 0
#endif

#define DB_PRINT_L(lvl, fmt, args...) do {\
    if (IMX_TZIC_ERR_DEBUG >= lvl) {\
        qemu_log("imx-tzic: %s:" fmt, __func__, ## args);\
    } \
} while (0);

#define DB_PRINT(fmt, args...) DB_PRINT_L(1, fmt, ## args)

static inline int imx_tzic_prio(IMXTZICState *s, int irq)
{
    uint32_t word = irq / IMX_TZIC_PRIO_PER_WORD;
    uint32_t offset = IMX_TZIC_PRIO_BITS * (irq % IMX_TZIC_PRIO_PER_WORD);
    return 255 - extract32(s->prio[word], offset, IMX_TZIC_PRIO_BITS);
}

static void imx_tzic_update(IMXTZICState *s)
{
    int i;

    /*
     * Wakeup is asserted if any intin & wakeup are true, but not swint! Also
     * no priorites (I guess)
     */
    int wakeup_lvl = 0;

    /* FIQ if we have a intin or source interrupt and interrupt is secure */
    int fiq_lvl = 0;
    int irq_lvl = 0;

    int highest_prio = 0;

    /* Check wakeup and any pending interrupts */
    for (i = 0; i < IMX_TZIC_IRQ_WORDS; i++) {
        if (s->intin[i] & s->wakeup[i]) {
            wakeup_lvl = 1;
        }

        s->pending[i] = s->enabled[i] & (s->intin[i] | s->source[i]);
        s->hi_pending[i] = 0x00000000;
    }

    /* Scan for the highest priority value of any pending interrupt */
    for (i = 0; i < IMX_TZIC_NUM_IRQS; i++) {
        int word = i / IMX_TZIC_IRQ_PER_WORD;
        int mask = 1 << (i % IMX_TZIC_IRQ_PER_WORD);

        if (s->pending[word] & mask) {
            int prio = imx_tzic_prio(s, i);

            if (prio > highest_prio) {
                highest_prio = prio;
            }
        }
    }

    /* Set IRQ/FIQ and hi_pending */
    for (i = 0; i < IMX_TZIC_NUM_IRQS; i++) {
        int word = i / IMX_TZIC_IRQ_PER_WORD;
        int mask = 1 << (i % IMX_TZIC_IRQ_PER_WORD);

        if (s->pending[word] & mask) {
            int prio = imx_tzic_prio(s, i);

            if (prio > s->priomask) {
                if (s->non_secure[word] & mask) {
                    irq_lvl = 1;
                } else {
                    fiq_lvl = 1;
                }
            }

            /* hi_pending doesn't care about priomask */
            if (prio == highest_prio) {
                s->hi_pending[word] |= mask;
            }
        }
    }

    /* TODO: Lets assume that wakeup is also controlled by the EN flag */
    if (s->intctrl & 1) {
        qemu_set_irq(s->wakeup_req, wakeup_lvl);
        qemu_set_irq(s->irq, irq_lvl);
        qemu_set_irq(s->fiq, fiq_lvl);
    } else {
        qemu_set_irq(s->wakeup_req, 0);
        qemu_set_irq(s->irq, 0);
        qemu_set_irq(s->fiq, 0);
    }
}

static void imx_tzic_set_irq(void *opaque, int irq, int level)
{
    IMXTZICState *s = opaque;

    int word = irq / IMX_TZIC_IRQ_PER_WORD;
    int mask = 1 << (irq % IMX_TZIC_IRQ_PER_WORD);

    if (level) {
        DB_PRINT("Raising IRQ %d, prio %d\n",
                irq, imx_tzic_prio(s, irq));
        s->intin[word] |= mask;
    } else {
        DB_PRINT("Clearing IRQ %d, prio %d\n",
                irq, imx_tzic_prio(s, irq));
        s->intin[word] &= ~mask;
    }

    imx_tzic_update(s);
}

static uint64_t imx_tzic_read(void *opaque,
                             hwaddr offset, unsigned size)
{
    IMXTZICState *s = opaque;

    DB_PRINT("read from %" HWADDR_PRIx "\n", offset);
    switch (offset) {
    case IMX_TZIC_REG_INTCTRL:
        return s->intctrl;
    case IMX_TZIC_REG_INTTYPE:
        /* Bit 10 (DOM) would be 1 if we supported TZ */
        return 0x00000000 | (IMX_TZIC_NUM_IRQS / 32 - 1);
    case IMX_TZIC_REG_PRIOMASK:
        return s->priomask;
    case IMX_TZIC_REG_SYNCCTRL:
        return s->syncctrl;
    case IMX_TZIC_REG_DSMINT:
        /* No DSM INT holdoff */
        return 0x00000000;
    case IMX_TZIC_REG_INTSECX(0) ... IMX_TZIC_REG_INTSECX(IMX_TZIC_IRQ_WORDS - 1): {
        int word = (offset - IMX_TZIC_REG_INTSECX(0)) / 4;
        return s->non_secure[word];
    }
    case IMX_TZIC_REG_ENSETX(0) ... IMX_TZIC_REG_ENSETX(IMX_TZIC_IRQ_WORDS - 1): {
        int word = (offset - IMX_TZIC_REG_ENSETX(0)) / 4;
        return s->enabled[word];
    }
    case IMX_TZIC_REG_ENCLEARX(0) ... IMX_TZIC_REG_ENCLEARX(IMX_TZIC_IRQ_WORDS - 1): {
        int word = (offset - IMX_TZIC_REG_ENCLEARX(0)) / 4;
        return s->enabled[word];
    }
    case IMX_TZIC_REG_SRCSETX(0) ... IMX_TZIC_REG_SRCSETX(IMX_TZIC_IRQ_WORDS - 1): {
        int word = (offset - IMX_TZIC_REG_SRCSETX(0)) / 4;
        return s->source[word];
    }
    case IMX_TZIC_REG_SRCCLEARX(0) ... IMX_TZIC_REG_SRCCLEARX(IMX_TZIC_IRQ_WORDS - 1): {
        int word = (offset - IMX_TZIC_REG_SRCCLEARX(0)) / 4;
        return s->source[word];
    }
    case IMX_TZIC_REG_PRIORITYX(0) ... IMX_TZIC_REG_PRIORITYX(IMX_TZIC_PRIO_WORDS - 1): {
        int word = (offset - IMX_TZIC_REG_PRIORITYX(0)) / 4;
        return s->prio[word];
    }
    case IMX_TZIC_REG_PNDX(0) ... IMX_TZIC_REG_PNDX(IMX_TZIC_IRQ_WORDS - 1): {
        int word = (offset - IMX_TZIC_REG_PNDX(0)) / 4;
        return s->pending[word];
    }
    case IMX_TZIC_REG_HIPNDX(0) ... IMX_TZIC_REG_HIPNDX(IMX_TZIC_IRQ_WORDS - 1): {
        int word = (offset - IMX_TZIC_REG_HIPNDX(0)) / 4;
        return s->hi_pending[word];
    }
    case IMX_TZIC_REG_WAKEUPX(0) ... IMX_TZIC_REG_WAKEUPX(IMX_TZIC_IRQ_WORDS - 1): {
        int word = (offset - IMX_TZIC_REG_WAKEUPX(0)) / 4;
        return s->wakeup[word];
    }
    case IMX_TZIC_REG_SWINT:
        return 0;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                     "imx_tzic_read: Bad offset %x\n", (int) offset);
        break;
    }

    return 0;
}

static void imx_tzic_write(void *opaque, hwaddr offset,
                          uint64_t val, unsigned size)
{
    IMXTZICState *s = opaque;

    DB_PRINT("write %08" PRIx64 " to %" HWADDR_PRIx "\n", val, offset);
    switch (offset) {
    case IMX_TZIC_REG_INTCTRL:
        s->intctrl = val & 0x80000001;
        if ((val & 0x80010000) == 0x80010000) {
            qemu_log_mask(LOG_UNIMP,
                        "imx_tzic_write: Non-secure interrupts not supported\n");
        }
        imx_tzic_update(s);
        break;
    case IMX_TZIC_REG_PRIOMASK:
        s->priomask = val & 0xFF;
        imx_tzic_update(s);
        break;
    case IMX_TZIC_REG_SYNCCTRL:
        s->syncctrl = val & 0x3;
        break;
    case IMX_TZIC_REG_DSMINT:
        qemu_log_mask(LOG_UNIMP,
                     "imx_tzic_write: DSM interrupt holdoff not supported\n");
        break;
    case IMX_TZIC_REG_INTSECX(0) ... IMX_TZIC_REG_INTSECX(IMX_TZIC_IRQ_WORDS - 1): {
        /* 0 = Secure (FIQ), 1 = non-secure (IRQ) */
        int word = (offset - IMX_TZIC_REG_INTSECX(0)) / 4;
        s->non_secure[word] |= val;
        imx_tzic_update(s);
        break;
    }
    case IMX_TZIC_REG_ENSETX(0) ... IMX_TZIC_REG_ENSETX(IMX_TZIC_IRQ_WORDS - 1): {
        int word = (offset - IMX_TZIC_REG_ENSETX(0)) / 4;
        s->enabled[word] |= val;
        imx_tzic_update(s);
        break;
    }
    case IMX_TZIC_REG_ENCLEARX(0) ... IMX_TZIC_REG_ENCLEARX(IMX_TZIC_IRQ_WORDS - 1): {
        int word = (offset - IMX_TZIC_REG_ENCLEARX(0)) / 4;
        s->enabled[word] &= ~val;
        imx_tzic_update(s);
        break;
    }
    case IMX_TZIC_REG_PRIORITYX(0) ... IMX_TZIC_REG_PRIORITYX(IMX_TZIC_PRIO_WORDS - 1): {
        int word = (offset - IMX_TZIC_REG_PRIORITYX(0)) / 4;
        s->prio[word] = val;
        imx_tzic_update(s);
        break;
    }
    case IMX_TZIC_REG_SRCSETX(0) ... IMX_TZIC_REG_SRCSETX(IMX_TZIC_IRQ_WORDS - 1): {
        int word = (offset - IMX_TZIC_REG_SRCSETX(0)) / 4;
        s->source[word] |= val;
        imx_tzic_update(s);
        break;
    }
    case IMX_TZIC_REG_SRCCLEARX(0) ... IMX_TZIC_REG_SRCCLEARX(IMX_TZIC_IRQ_WORDS - 1): {
        int word = (offset - IMX_TZIC_REG_SRCCLEARX(0)) / 4;
        s->source[word] &= ~val;
        imx_tzic_update(s);
        break;
    }
    case IMX_TZIC_REG_WAKEUPX(0) ... IMX_TZIC_REG_WAKEUPX(IMX_TZIC_IRQ_WORDS - 1): {
        int word = (offset - IMX_TZIC_REG_WAKEUPX(0)) / 4;
        s->wakeup[word] = val;
        imx_tzic_update(s);
        break;
    }
    case IMX_TZIC_REG_SWINT: {
        int intid = val & (IMX_TZIC_NUM_IRQS - 1);
        int word = intid / IMX_TZIC_IRQ_PER_WORD;
        int mask = 1 << (intid % IMX_TZIC_IRQ_PER_WORD);

        if (val & 0x80000000) {
            s->source[word] &= ~mask;
        } else {
            s->source[word] |= mask;
        }

        imx_tzic_update(s);
        break;
    }
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                     "imx_tzic_write: Bad offset %x\n", (int) offset);
        break;
    }
}

static const MemoryRegionOps imx_tzic_ops = {
    .read = imx_tzic_read,
    .write = imx_tzic_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void imx_tzic_realize(DeviceState *dev, Error **errp)
{
    IMXTZICState *s = IMX_TZIC(dev);

    memory_region_init_io(&s->iomem, OBJECT(s), &imx_tzic_ops, s,
                          "imx-tzic", 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);

    qdev_init_gpio_in(dev, imx_tzic_set_irq, IMX_TZIC_NUM_IRQS);
    memset(s->intin, 0, sizeof(s->intin));

    sysbus_init_irq(SYS_BUS_DEVICE(dev), &s->irq);
    sysbus_init_irq(SYS_BUS_DEVICE(dev), &s->fiq);
    qdev_init_gpio_out_named(dev, &s->wakeup_req, "wakeup", 1);
}

static void imx_tzic_reset(DeviceState *dev)
{
    IMXTZICState *s = IMX_TZIC(dev);

    s->intctrl =    0x00000000;
    s->priomask =   0x00000000;
    s->syncctrl =   0x00000000;
    memset(s->non_secure, 0, sizeof(s->non_secure));
    memset(s->enabled, 0, sizeof(s->enabled));
    memset(s->source, 0, sizeof(s->source));
    memset(s->prio, 0, sizeof(s->prio));
    memset(s->pending, 0, sizeof(s->pending));
    memset(s->hi_pending, 0, sizeof(s->hi_pending));
    memset(s->wakeup, 0, sizeof(s->wakeup));
}

static void imx_tzic_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = imx_tzic_realize;
    dc->reset = imx_tzic_reset;
    dc->desc = "i.MX TrustZone Aware Interrupt Controller";
}

static const TypeInfo imx_tzic_info = {
    .name = TYPE_IMX_TZIC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(IMXTZICState),
    .class_init = imx_tzic_class_init,
};

static void imx_tzic_register_types(void)
{
    type_register_static(&imx_tzic_info);
}

type_init(imx_tzic_register_types)
