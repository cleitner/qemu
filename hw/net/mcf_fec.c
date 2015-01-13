/*
 * ColdFire Fast Ethernet Controller emulation.
 *
 * Copyright (c) 2007 CodeSourcery.
 *
 * This code is licensed under the GPL
 */
#include "hw/hw.h"
#include "net/net.h"
#include "hw/net/mcf_fec.h"
/* For crc32 */
#include <zlib.h>
#include "exec/address-spaces.h"

//#define DEBUG_FEC 1

#ifdef DEBUG_FEC
#define DPRINTF(fmt, ...) \
do { printf("mcf_fec: " fmt , ## __VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) do {} while(0)
#endif

#define FEC_MAX_FRAME_SIZE 2032

struct PHY {
    uint32_t regs[32];

    int link;

    unsigned int (*read)(struct PHY *phy, unsigned int req);
    void (*write)(struct PHY *phy, unsigned int req,
                  unsigned int data);
};

/* Advertisement control register. */
#define ADVERTISE_10HALF        0x0020  /* Try for 10mbps half-duplex  */
#define ADVERTISE_10FULL        0x0040  /* Try for 10mbps full-duplex  */
#define ADVERTISE_100HALF       0x0080  /* Try for 100mbps half-duplex */
#define ADVERTISE_100FULL       0x0100  /* Try for 100mbps full-duplex */

static unsigned int tdk_read(struct PHY *phy, unsigned int req)
{
    int regnum;
    unsigned r = 0;

    regnum = req & 0x1f;

    switch (regnum) {
        case 1:
            if (!phy->link) {
                break;
            }
            /* MR1.  */
            /* Speeds and modes.  */
            r |= (1 << 13) | (1 << 14);
            r |= (1 << 11) | (1 << 12);
            r |= (1 << 5); /* Autoneg complete.  */
            r |= (1 << 3); /* Autoneg able.  */
            r |= (1 << 2); /* link.  */
            r |= (1 << 1); /* link.  */
            break;
        case 5:
            /* Link partner ability.
               We are kind; always agree with whatever best mode
               the guest advertises.  */
            r = 1 << 14; /* Success.  */
            /* Copy advertised modes.  */
            r |= phy->regs[4] & (15 << 5);
            /* Autoneg support.  */
            r |= 1;
            break;
        case 17:
            /* Marvell PHY on many xilinx boards.  */
            r = 0x8000; /* 1000Mb  */
            break;
        case 18:
            {
                /* Diagnostics reg.  */
                int duplex = 0;
                int speed_100 = 0;

                if (!phy->link) {
                    break;
                }

                /* Are we advertising 100 half or 100 duplex ? */
                speed_100 = !!(phy->regs[4] & ADVERTISE_100HALF);
                speed_100 |= !!(phy->regs[4] & ADVERTISE_100FULL);

                /* Are we advertising 10 duplex or 100 duplex ? */
                duplex = !!(phy->regs[4] & ADVERTISE_100FULL);
                duplex |= !!(phy->regs[4] & ADVERTISE_10FULL);
                r = (speed_100 << 10) | (duplex << 11);
            }
            break;

        default:
            r = phy->regs[regnum];
            break;
    }
    DPRINTF("\n%s %x = reg[%d]\n", __func__, r, regnum);
    return r;
}

static void
tdk_write(struct PHY *phy, unsigned int req, unsigned int data)
{
    int regnum;

    regnum = req & 0x1f;
    DPRINTF("%s reg[%d] = %x\n", __func__, regnum, data);
    switch (regnum) {
        default:
            phy->regs[regnum] = data;
            break;
    }

    /* Unconditionally clear regs[BMCR][BMCR_RESET] */
    phy->regs[0] &= ~0x8000;
}

static void
tdk_init(struct PHY *phy)
{
    phy->regs[0] = 0x3100;
    /* PHY Id.  */
    phy->regs[2] = 0x0300;
    phy->regs[3] = 0xe400;
    /* Autonegotiation advertisement reg.  */
    phy->regs[4] = 0x01E1;
    phy->link = 1;

    phy->read = tdk_read;
    phy->write = tdk_write;
}

struct MDIOBus {
    /* bus.  */
    int mdc;
    int mdio;

    /* decoder.  */
    enum {
        PREAMBLE,
        SOF,
        OPC,
        ADDR,
        REQ,
        TURNAROUND,
        DATA
    } state;
    unsigned int drive;

    unsigned int cnt;
    unsigned int addr;
    unsigned int opc;
    unsigned int req;
    unsigned int data;

    struct PHY *devs[32];
};

static void
mdio_attach(struct MDIOBus *bus, struct PHY *phy, unsigned int addr)
{
    bus->devs[addr & 0x1f] = phy;
}

#ifdef USE_THIS_DEAD_CODE
static void
mdio_detach(struct MDIOBus *bus, struct PHY *phy, unsigned int addr)
{
    bus->devs[addr & 0x1f] = NULL;
}
#endif

static uint16_t mdio_read_req(struct MDIOBus *bus, unsigned int addr,
                  unsigned int reg)
{
    struct PHY *phy;
    uint16_t data;

    phy = bus->devs[addr];
    if (phy && phy->read) {
        data = phy->read(phy, reg);
    } else {
        data = 0xffff;
    }
    DPRINTF("%s addr=%d reg=%d data=%x\n", __func__, addr, reg, data);
    return data;
}

static void mdio_write_req(struct MDIOBus *bus, unsigned int addr,
               unsigned int reg, uint16_t data)
{
    struct PHY *phy;

    DPRINTF("%s addr=%d reg=%d data=%x\n", __func__, addr, reg, data);
    phy = bus->devs[addr];
    if (phy && phy->write) {
        phy->write(phy, reg, data);
    }
}

typedef struct {
    MemoryRegion *sysmem;
    MemoryRegion iomem;
    qemu_irq *irqs;
    NICState *nic;
    NICConf conf;

    struct PHY phy;
    struct MDIOBus mdio_bus;

    uint32_t irq_state;
    uint32_t eir;
    uint32_t eimr;
    int rx_enabled;
    uint32_t rx_descriptor;
    uint32_t tx_descriptor;
    uint32_t ecr;
    uint32_t mmfr;
    uint32_t mscr;
    uint32_t rcr;
    uint32_t tcr;
    uint32_t tfwr;
    uint32_t rfsr;
    uint32_t erdsr;
    uint32_t etdsr;
    uint32_t emrbr;
    uint32_t miigsk_cfgr;
    uint32_t miigsk_enr;
} mcf_fec_state;

#define FEC_MIIGSK_ENR_EN       0x00000002
#define FEC_MIIGSK_ENR_READY    0x00000004

#define FEC_INT_HB   0x80000000
#define FEC_INT_BABR 0x40000000
#define FEC_INT_BABT 0x20000000
#define FEC_INT_GRA  0x10000000
#define FEC_INT_TXF  0x08000000
#define FEC_INT_TXB  0x04000000
#define FEC_INT_RXF  0x02000000
#define FEC_INT_RXB  0x01000000
#define FEC_INT_MII  0x00800000
#define FEC_INT_EB   0x00400000
#define FEC_INT_LC   0x00200000
#define FEC_INT_RL   0x00100000
#define FEC_INT_UN   0x00080000

#define FEC_EN      2
#define FEC_RESET   1

/* Map interrupt flags onto IRQ lines.  */
#define FEC_NUM_IRQ 13
static const uint32_t mcf_fec_irq_map[FEC_NUM_IRQ] = {
    FEC_INT_TXF,
    FEC_INT_TXB,
    FEC_INT_UN,
    FEC_INT_RL,
    FEC_INT_RXF,
    FEC_INT_RXB,
    FEC_INT_MII,
    FEC_INT_LC,
    FEC_INT_HB,
    FEC_INT_GRA,
    FEC_INT_EB,
    FEC_INT_BABT,
    FEC_INT_BABR
};

/* Buffer Descriptor.  */
typedef struct {
    uint16_t flags;
    uint16_t length;
    uint32_t data;
} mcf_fec_bd;

#define FEC_BD_R    0x8000
#define FEC_BD_E    0x8000
#define FEC_BD_O1   0x4000
#define FEC_BD_W    0x2000
#define FEC_BD_O2   0x1000
#define FEC_BD_L    0x0800
#define FEC_BD_TC   0x0400
#define FEC_BD_ABC  0x0200
#define FEC_BD_M    0x0100
#define FEC_BD_BC   0x0080
#define FEC_BD_MC   0x0040
#define FEC_BD_LG   0x0020
#define FEC_BD_NO   0x0010
#define FEC_BD_CR   0x0004
#define FEC_BD_OV   0x0002
#define FEC_BD_TR   0x0001

static void mcf_fec_read_bd(mcf_fec_bd *bd, uint32_t addr)
{
    uint32_t desc[2];

    cpu_physical_memory_read(addr, desc, sizeof(desc));

    desc[0] = tswap32(desc[0]);
    desc[1] = tswap32(desc[1]);

    bd->flags = extract32(desc[0], 16, 16);
    bd->length = extract32(desc[0], 0, 16);
    bd->data = desc[1];
}

static void mcf_fec_write_bd(mcf_fec_bd *bd, uint32_t addr)
{
    uint32_t desc[2];

    desc[0] = 0;
    desc[0] = deposit32(desc[0], 16, 16, bd->flags);
    desc[0] = deposit32(desc[0], 0, 16, bd->length);
    desc[1] = bd->data;

    desc[0] = tswap32(desc[0]);
    desc[1] = tswap32(desc[1]);

    cpu_physical_memory_write(addr, desc, sizeof(desc));
}

static void mcf_fec_update(mcf_fec_state *s)
{
    uint32_t active;
    uint32_t changed;
    uint32_t mask;
    int i;

    active = s->eir & s->eimr;
    changed = active ^s->irq_state;
    for (i = 0; i < FEC_NUM_IRQ; i++) {
        mask = mcf_fec_irq_map[i];
        if (changed & mask) {
            DPRINTF("IRQ %d = %d\n", i, (active & mask) != 0);
            if (s->irqs != NULL) {
                qemu_set_irq(s->irqs[i], (active & mask) != 0);
            }
        }
    }
    s->irq_state = active;
}

static void mcf_fec_do_tx(mcf_fec_state *s)
{
    uint32_t addr;
    mcf_fec_bd bd;
    int frame_size;
    int len;
    uint8_t frame[FEC_MAX_FRAME_SIZE];
    uint8_t *ptr;

    DPRINTF("do_tx\n");
    ptr = frame;
    frame_size = 0;
    addr = s->tx_descriptor;
    while (1) {
        mcf_fec_read_bd(&bd, addr);
        DPRINTF("tx_bd %x flags %04x len %d data %08x\n",
                addr, bd.flags, bd.length, bd.data);
        if ((bd.flags & FEC_BD_R) == 0) {
            /* Run out of descriptors to transmit.  */
            break;
        }
        len = bd.length;
        if (frame_size + len > FEC_MAX_FRAME_SIZE) {
            len = FEC_MAX_FRAME_SIZE - frame_size;
            s->eir |= FEC_INT_BABT;
        }
        cpu_physical_memory_read(bd.data, ptr, len);
        ptr += len;
        frame_size += len;
        if (bd.flags & FEC_BD_L) {
            /* Last buffer in frame.  */
            DPRINTF("Sending packet\n");
            qemu_send_packet(qemu_get_queue(s->nic), frame, len);
            ptr = frame;
            frame_size = 0;
            s->eir |= FEC_INT_TXF;
        }
        s->eir |= FEC_INT_TXB;
        bd.flags &= ~FEC_BD_R;
        /* Write back the modified descriptor.  */
        mcf_fec_write_bd(&bd, addr);
        /* Advance to the next descriptor.  */
        if ((bd.flags & FEC_BD_W) != 0) {
            addr = s->etdsr;
        } else {
            addr += 8;
        }
    }
    s->tx_descriptor = addr;
}

static void mcf_fec_enable_rx(mcf_fec_state *s)
{
    mcf_fec_bd bd;

    mcf_fec_read_bd(&bd, s->rx_descriptor);
    s->rx_enabled = ((bd.flags & FEC_BD_E) != 0);
    if (!s->rx_enabled)
        DPRINTF("RX buffer full\n");
}

static void mcf_fec_reset(mcf_fec_state *s)
{
    s->eir = 0;
    s->eimr = 0;
    s->rx_enabled = 0;
    s->ecr = 0;
    s->mscr = 0;
    s->rcr = 0x05ee0001;
    s->tcr = 0;
    s->tfwr = 0;
    s->rfsr = 0x500;
    /* TODO: the i.MX53 reference manual lacks any details on these registers */
    s->miigsk_cfgr = 0;
    s->miigsk_enr = 0;
}

static uint64_t mcf_fec_read(void *opaque, hwaddr addr,
                             unsigned size)
{
    mcf_fec_state *s = (mcf_fec_state *)opaque;
    switch (addr & 0x3ff) {
    case 0x004: return s->eir;
    case 0x008: return s->eimr;
    case 0x010: return s->rx_enabled ? (1 << 24) : 0; /* RDAR */
    case 0x014: return 0; /* TDAR */
    case 0x024: return s->ecr;
    case 0x040:
        s->eir &= ~FEC_INT_MII;
        mcf_fec_update(s);
        return s->mmfr;
    case 0x044: return s->mscr;
    case 0x064: return 0; /* MIBC */
    case 0x084: return s->rcr;
    case 0x0c4: return s->tcr;
    case 0x0e4: /* PALR */
        return (s->conf.macaddr.a[0] << 24) | (s->conf.macaddr.a[1] << 16)
              | (s->conf.macaddr.a[2] << 8) | s->conf.macaddr.a[3];
        break;
    case 0x0e8: /* PAUR */
        return (s->conf.macaddr.a[4] << 24) | (s->conf.macaddr.a[5] << 16) | 0x8808;
    case 0x0ec: return 0x10000; /* OPD */
    case 0x118: return 0;
    case 0x11c: return 0;
    case 0x120: return 0;
    case 0x124: return 0;
    case 0x144: return s->tfwr;
    case 0x14c: return 0x600;
    case 0x150: return s->rfsr;
    case 0x180: return s->erdsr;
    case 0x184: return s->etdsr;
    case 0x188: return s->emrbr;
    case 0x200 ... 0x2E0:
        /* TODO: implement counters */
        return 0;
    case MCF_FEC_REG_MIIGSK_CFGR: return s->miigsk_cfgr;
    case MCF_FEC_REG_MIIGSK_ENR: return s->miigsk_enr;
    default:
        hw_error("mcf_fec_read: Bad address 0x%x\n", (int)addr);
        return 0;
    }
}

static void mcf_fec_mdio_req(mcf_fec_state *s)
{
    unsigned int op = extract32(s->mmfr, 28, 2);
    unsigned int phyaddr = extract32(s->mmfr, 23, 5);
    unsigned int regaddr = extract32(s->mmfr, 18, 5);

    if (op == 1) {
        mdio_write_req(&s->mdio_bus, phyaddr, regaddr, extract32(s->mmfr, 0, 16));
    } else if (op == 2) {
        s->mmfr = deposit32(s->mmfr, 0, 16, mdio_read_req(&s->mdio_bus, phyaddr, regaddr));
    } else {
        qemu_log_mask(LOG_GUEST_ERROR, "mcf_fec_mdio_req: invalid MDIOBus OP=%d\n", op);
    }

    /*
     * TODO: can't find any definition of when this interrupt is reset. I guess
     * on read of MMFR
     */
    s->eir |= FEC_INT_MII;
    mcf_fec_update(s);
}

static void mcf_fec_write(void *opaque, hwaddr addr,
                          uint64_t value, unsigned size)
{
    mcf_fec_state *s = (mcf_fec_state *)opaque;
    switch (addr & 0x3ff) {
    case 0x004:
        s->eir &= ~value;
        break;
    case 0x008:
        s->eimr = value;
        break;
    case 0x010: /* RDAR */
        if ((s->ecr & FEC_EN) && !s->rx_enabled) {
            DPRINTF("RX enable\n");
            mcf_fec_enable_rx(s);
        }
        break;
    case 0x014: /* TDAR */
        if (s->ecr & FEC_EN) {
            mcf_fec_do_tx(s);
        }
        break;
    case 0x024:
        s->ecr = value;
        if (value & FEC_RESET) {
            DPRINTF("Reset\n");
            mcf_fec_reset(s);
        }
        if ((s->ecr & FEC_EN) == 0) {
            s->rx_enabled = 0;
        }
        break;
    case 0x040:
        s->mmfr = value;
        if (s->mscr & 0x7e) {
            mcf_fec_mdio_req(s);
        }
        break;
    case 0x044: {
        int old_mii_speed = s->mscr & 0x7e;
        s->mscr = value & 0xfe;
        if ((s->mscr & 0x7e) != 0 && old_mii_speed == 0) {
            mcf_fec_mdio_req(s);
        }
        break;
    }
    case 0x064:
        /* TODO: Implement MIB.  */
        break;
    case 0x084:
        s->rcr = value & 0x07ff003f;
        /* TODO: Implement LOOP mode.  */
        break;
    case 0x0c4: /* TCR */
        /* We transmit immediately, so raise GRA immediately.  */
        s->tcr = value;
        if (value & 1)
            s->eir |= FEC_INT_GRA;
        break;
    case 0x0e4: /* PALR */
        s->conf.macaddr.a[0] = value >> 24;
        s->conf.macaddr.a[1] = value >> 16;
        s->conf.macaddr.a[2] = value >> 8;
        s->conf.macaddr.a[3] = value;
        break;
    case 0x0e8: /* PAUR */
        s->conf.macaddr.a[4] = value >> 24;
        s->conf.macaddr.a[5] = value >> 16;
        break;
    case 0x0ec:
        /* OPD */
        break;
    case 0x118:
    case 0x11c:
    case 0x120:
    case 0x124:
        /* TODO: implement MAC hash filtering.  */
        break;
    case 0x144:
        s->tfwr = value & 3;
        break;
    case 0x14c:
        /* FRBR writes ignored.  */
        break;
    case 0x150:
        s->rfsr = (value & 0x3fc) | 0x400;
        break;
    case 0x180:
        s->erdsr = value & ~3;
        s->rx_descriptor = s->erdsr;
        break;
    case 0x184:
        s->etdsr = value & ~3;
        s->tx_descriptor = s->etdsr;
        break;
    case 0x188:
        s->emrbr = value & 0x7f0;
        break;
    case MCF_FEC_REG_MIIGSK_CFGR:
        s->miigsk_cfgr = value;
        break;
    case MCF_FEC_REG_MIIGSK_ENR:
        s->miigsk_enr = value & 0x02;
        if (s->miigsk_enr & FEC_MIIGSK_ENR_EN) {
            s->miigsk_enr |= FEC_MIIGSK_ENR_READY;
        }
        break;
    default:
        hw_error("mcf_fec_write Bad address 0x%x\n", (int)addr);
    }
    mcf_fec_update(s);
}

static int mcf_fec_can_receive(NetClientState *nc)
{
    mcf_fec_state *s = qemu_get_nic_opaque(nc);
    return s->rx_enabled;
}

static ssize_t mcf_fec_receive(NetClientState *nc, const uint8_t *buf, size_t size)
{
    mcf_fec_state *s = qemu_get_nic_opaque(nc);
    mcf_fec_bd bd;
    uint32_t flags = 0;
    uint32_t addr;
    uint32_t crc;
    uint32_t buf_addr;
    uint8_t *crc_ptr;
    unsigned int buf_len;

    DPRINTF("do_rx len %d\n", size);
    if (!s->rx_enabled) {
        fprintf(stderr, "mcf_fec_receive: Unexpected packet\n");
    }
    /* 4 bytes for the CRC.  */
    size += 4;
    crc = cpu_to_be32(crc32(~0, buf, size));
    crc_ptr = (uint8_t *)&crc;
    /* Huge frames are truncted.  */
    if (size > FEC_MAX_FRAME_SIZE) {
        size = FEC_MAX_FRAME_SIZE;
        flags |= FEC_BD_TR | FEC_BD_LG;
    }
    /* Frames larger than the user limit just set error flags.  */
    if (size > (s->rcr >> 16)) {
        flags |= FEC_BD_LG;
    }
    addr = s->rx_descriptor;
    while (size > 0) {
        mcf_fec_read_bd(&bd, addr);
        if ((bd.flags & FEC_BD_E) == 0) {
            /* No descriptors available.  Bail out.  */
            /* FIXME: This is wrong.  We should probably either save the
               remainder for when more RX buffers are available, or
               flag an error.  */
            fprintf(stderr, "mcf_fec: Lost end of frame\n");
            break;
        }
        buf_len = (size <= s->emrbr) ? size: s->emrbr;
        bd.length = buf_len;
        size -= buf_len;
        DPRINTF("rx_bd %x length %d\n", addr, bd.length);
        /* The last 4 bytes are the CRC.  */
        if (size < 4)
            buf_len += size - 4;
        buf_addr = bd.data;
        cpu_physical_memory_write(buf_addr, buf, buf_len);
        buf += buf_len;
        if (size < 4) {
            cpu_physical_memory_write(buf_addr + buf_len, crc_ptr, 4 - size);
            crc_ptr += 4 - size;
        }
        bd.flags &= ~FEC_BD_E;
        if (size == 0) {
            /* Last buffer in frame.  */
            bd.flags |= flags | FEC_BD_L;
            DPRINTF("rx frame flags %04x\n", bd.flags);
            s->eir |= FEC_INT_RXF;
        } else {
            s->eir |= FEC_INT_RXB;
        }
        mcf_fec_write_bd(&bd, addr);
        /* Advance to the next descriptor.  */
        if ((bd.flags & FEC_BD_W) != 0) {
            addr = s->erdsr;
        } else {
            addr += 8;
        }
    }
    s->rx_descriptor = addr;
    mcf_fec_enable_rx(s);
    mcf_fec_update(s);
    return size;
}

static const MemoryRegionOps mcf_fec_ops = {
    .read = mcf_fec_read,
    .write = mcf_fec_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static NetClientInfo net_mcf_fec_info = {
    .type = NET_CLIENT_OPTIONS_KIND_NIC,
    .size = sizeof(NICState),
    .can_receive = mcf_fec_can_receive,
    .receive = mcf_fec_receive,
};

void mcf_fec_init(MemoryRegion *sysmem, NICInfo *nd,
                  hwaddr base, qemu_irq *irqs)
{
    mcf_fec_state *s;

    qemu_check_nic_model(nd, "mcf_fec");

    s = (mcf_fec_state *)g_malloc0(sizeof(mcf_fec_state));
    s->sysmem = sysmem;
    s->irqs = irqs;

    memory_region_init_io(&s->iomem, NULL, &mcf_fec_ops, s, "fec", 0x400);
    memory_region_add_subregion(sysmem, base, &s->iomem);

    s->conf.macaddr = nd->macaddr;
    s->conf.peers.ncs[0] = nd->netdev;

    s->nic = qemu_new_nic(&net_mcf_fec_info, &s->conf, nd->model, nd->name, s);

    qemu_format_nic_info_str(qemu_get_queue(s->nic), s->conf.macaddr.a);

    tdk_init(&s->phy);
    mdio_attach(&s->mdio_bus, &s->phy, 7);
}
