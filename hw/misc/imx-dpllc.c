/*
 * i.MX DPLL Controller DPLLC
 *
 * Copyright (C) 2015 Colin Leitner <colin.leitner@gmail.com>
 *
 * This code is licensed under the GPL.
 */

#include "hw/misc/imx-dpllc.h"

#ifndef IMX_DPLLC_ERR_DEBUG
#define IMX_DPLLC_ERR_DEBUG 0
#endif

#define DB_PRINT_L(lvl, fmt, args...) do {\
    if (IMX_DPLLC_ERR_DEBUG >= lvl) {\
        qemu_log("imx-dpllc: %s:" fmt, __func__, ## args);\
    } \
} while (0);

#define DB_PRINT(fmt, args...) DB_PRINT_L(1, fmt, ## args)

#define IMX_DPLLC_CTL_DPDCK0_2      (1 << 12)
#define IMX_DPLLC_CTL_REF_CLK_DIV   (1 << 10)
#define IMX_DPLLC_CTL_REF_CLK_SEL   (3 << 8)
#define IMX_DPLLC_CTL_HFSM          (1 << 7)

static void imx_dpllc_update_freq(IMXDPLLCState *s)
{
    uint64_t freq = s->ref_clk;

    uint32_t mfi;
    uint32_t pdf;
    uint32_t mfd;
    int32_t mfn;

    if (s->ctl & IMX_DPLLC_CTL_HFSM) {
        mfi = (s->hfs_op >> 4) & 0xF;
        pdf = (s->hfs_op >> 0) & 0xF;
        mfd = s->hfs_mfd;
        mfn = s->hfs_mfn;
    } else {
        mfi = (s->op >> 4) & 0xF;
        pdf = (s->op >> 0) & 0xF;
        mfd = s->mfd;
        mfn = s->mfn;
    }

    if (mfi < 6) {
        mfi = 5;
    }
    pdf += 1;
    mfd += 1;
    if (mfn & 0x4000000) {
        mfn |= 0xF8000000;
    }

    if (s->ctl & IMX_DPLLC_CTL_REF_CLK_DIV) {
        freq /= 2;
    }

    freq *= 2;
    freq *= mfi * mfd + mfn;
    freq /= pdf * mfd;

    if (s->ctl & IMX_DPLLC_CTL_DPDCK0_2) {
        freq *= 2;
    }

    DB_PRINT("frequency=%" PRIu64 " Hz\n", freq);

    s->freq = freq;
}

static uint64_t imx_dpllc_read(void *opaque,
                              hwaddr offset, unsigned int size)
{
    IMXDPLLCState *s = opaque;

    switch (offset) {
    case IMX_DPLLC_REG_CTL:
        return s->ctl & 0x00001FFF;
    case IMX_DPLLC_REG_CONFIG:
        return s->config;
    case IMX_DPLLC_REG_OP:
        return s->op;
    case IMX_DPLLC_REG_MFD:
        return s->mfd;
    case IMX_DPLLC_REG_MFN:
        return s->mfn;
    case IMX_DPLLC_REG_MFNMINUS:
        return s->mfnminus;
    case IMX_DPLLC_REG_MFNPLUS:
        return s->mfnplus;
    case IMX_DPLLC_REG_HFS_OP:
        return s->hfs_op;
    case IMX_DPLLC_REG_HFS_MFD:
        return s->hfs_mfd;
    case IMX_DPLLC_REG_HFS_MFN:
        return s->hfs_mfn;
    case IMX_DPLLC_REG_MFN_TOGC:
        return s->mfn_togc;
    case IMX_DPLLC_REG_DESTAT:
        qemu_log_mask(LOG_UNIMP, "imx_dpllc_read: desense is not modeled\n");
        return 0;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                     "imx_dpllc_read: Bad offset 0x%08" HWADDR_PRIx "\n", offset);
        break;
    }

    return 0;
}

static void imx_dpllc_write(void *opaque, hwaddr offset,
                            uint64_t val, unsigned int size)
{
    IMXDPLLCState *s = opaque;

    DB_PRINT("offset=0x%08" HWADDR_PRIx ", val=0x%08" PRIx64 ")\n", offset, val);

    switch (offset) {
    case IMX_DPLLC_REG_CTL:
        s->ctl = (s->ctl & ~0x00001F7E) | (val & 0x00001F7E);

        if ((s->ctl & IMX_DPLLC_CTL_REF_CLK_SEL) != 0x00000200) {
            qemu_log_mask(LOG_GUEST_ERROR,
                        "imx_dpllc_write: reserved reference clock %d\n", (s->ctl >> 10) & 3);
        }

        imx_dpllc_update_freq(s);
        break;

    case IMX_DPLLC_REG_CONFIG:
        s->config = (s->config & ~0x0000000E) | (val & 0x0000000E);

        if ((val & 0x0000000C) != 0x00000000) {
            qemu_log_mask(LOG_GUEST_ERROR,
                        "imx_dpllc_write: writing reserved FSL bits\n");
        }
        break;

    case IMX_DPLLC_REG_OP:
        s->op = val & 0x000000FF;
        imx_dpllc_update_freq(s);
        break;

    case IMX_DPLLC_REG_MFD:
        s->mfd = val & 0x03FFFFFF;
        imx_dpllc_update_freq(s);
        break;

    case IMX_DPLLC_REG_MFN:
        s->mfn = val & 0x07FFFFFF;
        imx_dpllc_update_freq(s);
        break;

    case IMX_DPLLC_REG_MFNMINUS:
        s->mfnminus = val & 0x07FFFFFF;
        imx_dpllc_update_freq(s);
        break;

    case IMX_DPLLC_REG_MFNPLUS:
        s->mfnplus = val & 0x07FFFFFF;
        imx_dpllc_update_freq(s);
        break;

    case IMX_DPLLC_REG_HFS_OP:
        s->hfs_op = val & 0x000000FF;
        imx_dpllc_update_freq(s);
        break;

    case IMX_DPLLC_REG_HFS_MFD:
        s->hfs_mfd = val & 0x03FFFFFF;
        imx_dpllc_update_freq(s);
        break;

    case IMX_DPLLC_REG_HFS_MFN:
        s->hfs_mfn = val & 0x07FFFFFF;
        imx_dpllc_update_freq(s);
        break;

    case IMX_DPLLC_REG_MFN_TOGC:
        s->mfn_togc = val & 0x0003FFFF;
        qemu_log_mask(LOG_UNIMP, "imx_dpllc_write: desense is not modeled\n");
        break;

    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                     "imx_dpllc_write: Bad offset 0x%08" HWADDR_PRIx "\n", offset);
        break;
    }
}

static const MemoryRegionOps imx_dpllc_ops = {
    .read = imx_dpllc_read,
    .write = imx_dpllc_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void imx_dpllc_realize(DeviceState *dev, Error **errp)
{
    IMXDPLLCState *s = IMX_DPLLC(dev);

    memory_region_init_io(&s->iomem, OBJECT(s), &imx_dpllc_ops, s,
                         "imx-dpllc", 0x1000);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->iomem);
}

static void imx_dpllc_reset(DeviceState *dev)
{
    IMXDPLLCState *s = IMX_DPLLC(dev);

    s->ctl =        0x00000223;
    s->config =     0x00000006;
    s->op =         0x00000000;
    s->mfd =        0x00AA0000;
    s->mfn =        0x00AA0000;
    s->mfnminus =   0x00000000;
    s->mfnplus =    0x00000000;
    s->hfs_op =     0x00000000;
    s->hfs_mfd =    0x00AA0000;
    s->hfs_mfn =    0x00AA0000;
    s->mfn_togc =   0x00020000;

    imx_dpllc_update_freq(s);
}

static Property imx_dpllc_props[] = {
    DEFINE_PROP_UINT32("ref_clk", IMXDPLLCState, ref_clk, 24000000),
    DEFINE_PROP_END_OF_LIST(),
};

static void imx_dpllc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = imx_dpllc_realize;
    dc->reset = imx_dpllc_reset;
    dc->props = imx_dpllc_props;
    dc->desc = "i.MX DPLL Controller";
}

static const TypeInfo imx_dpllc_info = {
    .name = TYPE_IMX_DPLLC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(IMXDPLLCState),
    .class_init = imx_dpllc_class_init,
};

static void imx_dpllc_register_types(void)
{
    type_register_static(&imx_dpllc_info);
}

type_init(imx_dpllc_register_types)
