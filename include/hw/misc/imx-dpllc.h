/*
 * i.MX DPLL Controller DPLLC
 *
 * Copyright (C) 2015 Colin Leitner <colin.leitner@gmail.com>
 *
 * This code is licensed under the GPL.
 */

#ifndef HW_INTC_IMX_DPLLC_H
#define HW_INTC_IMX_DPLLC_H

#include "hw/sysbus.h"

#define TYPE_IMX_DPLLC "imx-dpllc"
#define IMX_DPLLC(obj) OBJECT_CHECK(IMXDPLLCState, (obj), TYPE_IMX_DPLLC)

typedef struct {
    SysBusDevice parent_obj;

    MemoryRegion iomem;

    uint32_t ref_clk;

    /* Output frequency */
    uint32_t freq;

    uint32_t ctl;
    uint32_t config;
    uint32_t op;
    uint32_t mfd;
    uint32_t mfn;
    uint32_t mfnminus;
    uint32_t mfnplus;
    uint32_t hfs_op;
    uint32_t hfs_mfd;
    uint32_t hfs_mfn;
    uint32_t mfn_togc;
} IMXDPLLCState;

#define IMX_DPLLC_REG_CTL       0x000
#define IMX_DPLLC_REG_CONFIG    0x004
#define IMX_DPLLC_REG_OP        0x008
#define IMX_DPLLC_REG_MFD       0x00C
#define IMX_DPLLC_REG_MFN       0x010
#define IMX_DPLLC_REG_MFNMINUS  0x014
#define IMX_DPLLC_REG_MFNPLUS   0x018
#define IMX_DPLLC_REG_HFS_OP    0x01C
#define IMX_DPLLC_REG_HFS_MFD   0x020
#define IMX_DPLLC_REG_HFS_MFN   0x024
#define IMX_DPLLC_REG_MFN_TOGC  0x028
#define IMX_DPLLC_REG_DESTAT    0x02C

#endif
