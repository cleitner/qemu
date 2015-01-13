#include <stdint.h>

typedef struct {
    volatile uint32_t SCR;
    volatile uint32_t SBMR;
    volatile uint32_t SRSR;
    volatile uint32_t reserved1_[2];
    volatile uint32_t SISR;
    volatile uint32_t SIMR;
} src_regs_t;

#define SRC_SBMR_BMOD_SHIFT 24
#define SRC_SBMR_BMOD_MASK  (0x3 << SRC_SBMR_BMOD_SHIFT)

#define SRC         ((src_regs_t *) 0x53FD0000)

typedef struct __attribute__((packed)) {
    uint8_t tag;
    uint8_t ivt_len_high;
    uint8_t ivt_len_low;
    uint8_t version;

    uint32_t entry;
    uint32_t reserved1;
    uint32_t dcd;
    uint32_t boot_data;
    uint32_t self;
    uint32_t csf;
    uint32_t reserved2;
} ivt_t;

typedef struct {
    uint32_t start;
    uint32_t length;
    uint32_t plugin;
} boot_data_t;

#define NOR_START       0xF0000000
#define NOR_IVT_OFFSET  0x1000

#define IRAM        ((void *) 0xF8000000)

static void load_app(uint32_t base, uint32_t ivt_offset);
static void start_app(uint32_t entry_addr);
extern void print(const char *msg);
extern void printhex(const char *msg, uint32_t value);

static void boot_nor(void);

typedef void (*entry_t)(void);

void __attribute__((naked)) _start(void)
{
    /* Setup stack pointer to 0xF801FFB8 */
    asm volatile (
        "movw sp, #0xFFB8\n\t"
        "movt sp, #0xF801\n\t"
        ::: "sp");

    print("start");

    load_app(NOR_START, NOR_IVT_OFFSET);

    print("couldn't find a suitable boot image");

    while (1) ;
}

static void load_app(uint32_t base, uint32_t ivt_offset)
{
    ivt_t *ivt = (ivt_t *) (base + ivt_offset);
    uint32_t ivt_len;
    boot_data_t *boot_data;

    if (ivt->tag != 0xD1) {
        return;
    }

    ivt_len = (ivt->ivt_len_high << 8) | ivt->ivt_len_low;
    if (ivt_len != sizeof(ivt_t)) {
        return;
    }

    if (ivt->version != 0x40) {
        return;
    }

    if (ivt->reserved1 != 0) {
        print("reserved1 not 0");
        return;
    }

    if (ivt->dcd != 0) {
        print("DCD not supported");
        return;
    }

    if (ivt->boot_data != (uint32_t) (ivt + 1)) {
        print("invalid boot data pointer");
        return;
    }

    if (ivt->self != (uint32_t) ivt) {
        print("invalid self pointer");
        return;
    }

    if (ivt->csf != 0) {
        print("CSF not supported");
        return;
    }

    if (ivt->reserved2 != 0) {
        print("reserved2 not 0");
        return;
    }

    boot_data = (boot_data_t *) (ivt + 1);

    /* Currently we only support XIP */
    if (boot_data->start != base) {
        print("copying application not supported");
        return;
    }

    if (boot_data->plugin != 0) {
        print("plugin applications are not supported");
        return;
    }

    printhex("entry", ivt->entry);

    /* And go! */
    start_app(ivt->entry);
}

static void start_app(uint32_t entry_addr)
{
    entry_t entry = (entry_t) entry_addr;

    entry();

    print("application returned (that's quite unexpected)");

    while (1) ;
}

