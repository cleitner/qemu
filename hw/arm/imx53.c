/*
 * i.MX53 test board
 *
 * Copyright (C) 2015 Colin Leitner <colin.leitner@gmail.com>
 *
 * This code is licensed under the GPL.
 */

#include "hw/sysbus.h"
#include "hw/arm/arm.h"
#include "net/net.h"
#include "exec/address-spaces.h"
#include "sysemu/sysemu.h"
#include "hw/boards.h"
#include "hw/block/flash.h"
#include "sysemu/block-backend.h"
#include "hw/loader.h"
#include "qemu/error-report.h"
#include "hw/arm/imx.h"

#include "hw/intc/imx-tzic.h"
#include "hw/arm/imx53.h"
#include "hw/net/mcf_fec.h"

#define FLASH_SIZE          (32 * 1024 * 1024)
#define FLASH_SECTOR_SIZE   (128 * 1024)

static struct arm_boot_info imx53_binfo = {};

static void imx53_init(MachineState *machine)
{
    Error *err = NULL;
    ObjectClass *cpu_oc;
    ARMCPU *cpu;

    MemoryRegion *address_space_mem = get_system_memory();
    MemoryRegion *ddr_ram = g_new(MemoryRegion, 1);
    MemoryRegion *iram = g_new(MemoryRegion, 1);
    MemoryRegion *gmem = g_new(MemoryRegion, 1);
    MemoryRegion *boot_rom = g_new(MemoryRegion, 1);

    DeviceState *dev, *tzic, *ccm;
    qemu_irq irqs[IMX_TZIC_NUM_IRQS];
    int n;

    if (!machine->cpu_model) {
        machine->cpu_model = "cortex-a8";
    }
    cpu_oc = cpu_class_by_name(TYPE_ARM_CPU, machine->cpu_model);
    cpu = ARM_CPU(object_new(object_class_get_name(cpu_oc)));

    /* By default A8 CPUs have EL3 enabled.  This board does not
     * currently support EL3 so the CPU EL3 property is disabled before
     * realization.
     */
    if (object_property_find(OBJECT(cpu), "has_el3", NULL)) {
        object_property_set_bool(OBJECT(cpu), false, "has_el3", &err);
        if (err) {
            error_report("%s", error_get_pretty(err));
            exit(1);
        }
    }

    object_property_set_bool(OBJECT(cpu), true, "realized", &error_abort);

    tzic = sysbus_create_varargs("imx-tzic", IMX53_MMAP_TZIC_OFF,
                                qdev_get_gpio_in(DEVICE(cpu), ARM_CPU_IRQ),
                                qdev_get_gpio_in(DEVICE(cpu), ARM_CPU_FIQ),
                                NULL);
    for (n = 0; n < IMX_TZIC_NUM_IRQS; n++) {
        irqs[n] = qdev_get_gpio_in(tzic, n);
    }

    /* 256M of DDR RAM on bank 0 */
    machine->ram_size = 256 * 1024 * 1024;
    memory_region_init_ram(ddr_ram, NULL, "imx53.ddr_ram", machine->ram_size,
                           &error_abort);
    vmstate_register_ram_global(ddr_ram);
    memory_region_add_subregion(address_space_mem, IMX53_MMAP_CSD0_DDR_OFF, ddr_ram);

    /* 128K of iRAM (OCRAM) */
    memory_region_init_ram(iram, NULL, "imx53.iram", IMX53_MMAP_IRAM_LEN,
                           &error_abort);
    vmstate_register_ram_global(iram);
    memory_region_add_subregion(address_space_mem, IMX53_MMAP_IRAM_OFF, iram);

    /* 256K of GPU3D GMEM. This can be used as regular RAM without the GPU */
    memory_region_init_ram(gmem, NULL, "imx53.gmem", IMX53_MMAP_GPU3D_GMEM_LEN,
                           &error_abort);
    vmstate_register_ram_global(gmem);
    memory_region_add_subregion(address_space_mem, IMX53_MMAP_GPU3D_GMEM_OFF, gmem);

    DriveInfo *dinfo = drive_get(IF_PFLASH, 0, 0);

    /* Intel */
    dev = qdev_create(NULL, "cfi.pflash01");
    if (qdev_prop_set_drive(dev, "drive", dinfo ? blk_by_legacy_dinfo(dinfo) : NULL)) {
        abort();
    }
    qdev_prop_set_uint32(dev, "num-blocks", FLASH_SIZE / FLASH_SECTOR_SIZE);
    qdev_prop_set_uint64(dev, "sector-length", FLASH_SECTOR_SIZE);
    qdev_prop_set_uint8(dev, "width", 2);
    qdev_prop_set_uint8(dev, "device-width", 2);
    qdev_prop_set_uint16(dev, "id0", 0x0089);
    qdev_prop_set_uint16(dev, "id1", 0x881B);
    qdev_prop_set_string(dev, "name", "P30-65nm");
    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, IMX53_MMAP_3CS_CS0_OFF);

    dev = qdev_create(NULL, "imx-dpllc");
    qdev_prop_set_uint32(dev, "ref_clk", 25000000);
    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, IMX53_MMAP_DPLLC_1_OFF);
    dev = qdev_create(NULL, "imx-dpllc");
    qdev_prop_set_uint32(dev, "ref_clk", 25000000);
    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, IMX53_MMAP_DPLLC_2_OFF);
    dev = qdev_create(NULL, "imx-dpllc");
    qdev_prop_set_uint32(dev, "ref_clk", 25000000);
    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, IMX53_MMAP_DPLLC_3_OFF);
    dev = qdev_create(NULL, "imx-dpllc");
    qdev_prop_set_uint32(dev, "ref_clk", 25000000);
    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, IMX53_MMAP_DPLLC_4_OFF);

    sysbus_create_varargs("imx-gpio", IMX53_MMAP_GPIO_1_OFF,
                         irqs[IMX53_IRQ_GPIO_1_LOWER],
                         irqs[IMX53_IRQ_GPIO_1_UPPER],
                         irqs[IMX53_IRQ_GPIO_1_INT0],
                         irqs[IMX53_IRQ_GPIO_1_INT1],
                         irqs[IMX53_IRQ_GPIO_1_INT2],
                         irqs[IMX53_IRQ_GPIO_1_INT3],
                         irqs[IMX53_IRQ_GPIO_1_INT4],
                         irqs[IMX53_IRQ_GPIO_1_INT5],
                         irqs[IMX53_IRQ_GPIO_1_INT6],
                         irqs[IMX53_IRQ_GPIO_1_INT7],
                         NULL);
    sysbus_create_varargs("imx-gpio", IMX53_MMAP_GPIO_2_OFF,
                         irqs[IMX53_IRQ_GPIO_2_LOWER],
                         irqs[IMX53_IRQ_GPIO_2_UPPER],
                          NULL);
    sysbus_create_varargs("imx-gpio", IMX53_MMAP_GPIO_3_OFF,
                         irqs[IMX53_IRQ_GPIO_3_LOWER],
                         irqs[IMX53_IRQ_GPIO_3_UPPER],
                          NULL);
    sysbus_create_varargs("imx-gpio", IMX53_MMAP_GPIO_4_OFF,
                         irqs[IMX53_IRQ_GPIO_4_LOWER],
                         irqs[IMX53_IRQ_GPIO_4_UPPER],
                         NULL);
    sysbus_create_varargs("imx-gpio", IMX53_MMAP_GPIO_5_OFF,
                         irqs[IMX53_IRQ_GPIO_5_LOWER],
                         irqs[IMX53_IRQ_GPIO_5_UPPER],
                         NULL);
    sysbus_create_varargs("imx-gpio", IMX53_MMAP_GPIO_6_OFF,
                         irqs[IMX53_IRQ_GPIO_6_LOWER],
                         irqs[IMX53_IRQ_GPIO_6_UPPER],
                         NULL);
    sysbus_create_varargs("imx-gpio", IMX53_MMAP_GPIO_7_OFF,
                         irqs[IMX53_IRQ_GPIO_7_LOWER],
                         irqs[IMX53_IRQ_GPIO_7_UPPER],
                         NULL);

    /* TODO: this CCM is completely wrong */
    ccm = sysbus_create_simple("imx_ccm", IMX53_MMAP_CCM_OFF, NULL);

    imx_timerg_create(IMX53_MMAP_GPT_OFF, irqs[IMX53_IRQ_GPT], ccm);
    imx_timerp_create(IMX53_MMAP_EPIT_1_OFF, irqs[IMX53_IRQ_EPIT_1], ccm);
    imx_timerp_create(IMX53_MMAP_EPIT_2_OFF, irqs[IMX53_IRQ_EPIT_2], ccm);

    imx_serial_create(0, IMX53_MMAP_UART_1_OFF, irqs[IMX53_IRQ_UART_1]);
    imx_serial_create(1, IMX53_MMAP_UART_2_OFF, irqs[IMX53_IRQ_UART_2]);
    imx_serial_create(2, IMX53_MMAP_UART_3_OFF, irqs[IMX53_IRQ_UART_3]);
    imx_serial_create(3, IMX53_MMAP_UART_4_OFF, irqs[IMX53_IRQ_UART_4]);
    /* TODO: QEMU only supports 4 UARTs */
    /*imx_serial_create(4, IMX53_MMAP_UART_5_OFF, irqs[IMX53_IRQ_UART_5]);*/

    if (nb_nics > 1) {
        fprintf(stderr, "Too many NICs\n");
        exit(1);
    }
    if (nd_table[0].used) {
        /* TODO: fix IRQ */
        mcf_fec_init(address_space_mem, &nd_table[0], IMX53_MMAP_FEC_OFF, /*irqs[IMX53_IRQ_FEC]*/ NULL);
    }

    memory_region_init_ram(boot_rom, NULL, "imx53.boot_rom", IMX53_MMAP_BOOT_ROM_LEN,
                           &error_abort);
    vmstate_register_ram_global(boot_rom);
    memory_region_add_subregion(address_space_mem, IMX53_MMAP_BOOT_ROM_OFF, boot_rom);

    imx53_binfo.ram_size = machine->ram_size;
    imx53_binfo.kernel_filename = machine->kernel_filename;
    imx53_binfo.kernel_cmdline = machine->kernel_cmdline;
    imx53_binfo.initrd_filename = machine->initrd_filename;
    imx53_binfo.nb_cpus = 1;
    imx53_binfo.board_id = 4670;
    imx53_binfo.loader_start = 0;
    arm_load_kernel(ARM_CPU(cpu), &imx53_binfo);
}

static QEMUMachine imx53_machine = {
    .name = "imx53",
    .desc = "Freescale i.MX53 test board",
    .init = imx53_init,
};

static void imx53_machine_init(void)
{
    qemu_register_machine(&imx53_machine);
}

machine_init(imx53_machine_init);
