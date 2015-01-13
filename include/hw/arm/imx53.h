/*
 * i.MX53 SoC
 *
 * Copyright (C) 2015 Colin Leitner <colin.leitner@gmail.com>
 *
 * This code is licensed under the GPL.
 */

#ifndef HW_ARM_IMX53_H
#define HW_ARM_IMX53_H

#define IMX53_MMAP_BOOT_ROM_OFF         0x00000000
#define IMX53_MMAP_BOOT_ROM_LEN         (64 * 1024)
#define IMX53_MMAP_BOOT_ROM_ALIAS_OFF   0x00010000
#define IMX53_MMAP_BOOT_ROM_ALIAS_LEN   (16 * 1024 * 1024 - 64 * 1024)
/* 0x01000000 - 0x06FFFFFF reserved */
#define IMX53_MMAP_SCC_RAM_OFF          0x07000000
#define IMX53_MMAP_SCC_RAM_LEN          (16 * 1024)
#define IMX53_MMAP_SCC_RAM_ALIAS_OFF    0x07004000
#define IMX53_MMAP_SCC_RAM_ALIAS_LEN    (16 * 1024 * 1024 - 16 * 1024)
/* 0x08000000 - 0x0FFFFFFF reserved */
#define IMX53_MMAP_TZIC_OFF             0x0FFFC000
#define IMX53_MMAP_TZIC_LEN             (16 * 1024)
#define IMX53_MMAP_SATA_OFF             0x10000000
#define IMX53_MMAP_SATA_LEN             (16 * 1024)
#define IMX53_MMAP_SATA_ALIAS_OFF       0x10004000
#define IMX53_MMAP_SATA_ALIAS_LEN       (64 * 1024 * 1024 - 16 * 1024)
/* 0x14000000 - 0x17FFFFFF reserved */
#define IMX53_MMAP_IPU_OFF              0x18000000
#define IMX53_MMAP_IPU_LEN              (128 * 1024 * 1024)
#define IMX53_MMAP_GPU2D_OFF            0x20000000
#define IMX53_MMAP_GPU2D_LEN            (256 * 1024 * 1024)
#define IMX53_MMAP_GPU3D_OFF            0x30000000
#define IMX53_MMAP_GPU3D_LEN            (256 * 1024 * 1024)
#define IMX53_MMAP_DEBUG_ROM_OFF        0x40000000
#define IMX53_MMAP_DEBUG_ROM_LEN        (4 * 1024)
#define IMX53_MMAP_ETB_OFF              0x40001000
#define IMX53_MMAP_ETB_LEN              (4 * 1024)
#define IMX53_MMAP_ETM_OFF              0x40002000
#define IMX53_MMAP_ETM_LEN              (4 * 1024)
#define IMX53_MMAP_TPIU_OFF             0x40003000
#define IMX53_MMAP_TPIU_LEN             (4 * 1024)
#define IMX53_MMAP_CTI0_OFF             0x40004000
#define IMX53_MMAP_CTI0_LEN             (4 * 1024)
#define IMX53_MMAP_CTI1_OFF             0x40005000
#define IMX53_MMAP_CTI1_LEN             (4 * 1024)
#define IMX53_MMAP_CTI2_OFF             0x40006000
#define IMX53_MMAP_CTI2_LEN             (4 * 1024)
#define IMX53_MMAP_CTI3_OFF             0x40007000
#define IMX53_MMAP_CTI3_LEN             (4 * 1024)
#define IMX53_MMAP_ARM_DEBUG_OFF        0x40008000
#define IMX53_MMAP_ARM_DEBUG_LEN        (4 * 1024)
/* 0x40009000 - 0x4FFFFFFF reserved */
/* 0x50000000 - 0x50003FFF reserved */
#define IMX53_MMAP_ESDHCV2_1_OFF        0x50004000
#define IMX53_MMAP_ESDHCV2_1_LEN        (16 * 1024)
#define IMX53_MMAP_ESDHCV2_2_OFF        0x50008000
#define IMX53_MMAP_ESDHCV2_2_LEN        (16 * 1024)
#define IMX53_MMAP_UART_3_OFF           0x5000C000
#define IMX53_MMAP_UART_3_LEN           (16 * 1024)
#define IMX53_MMAP_ECSPI_1_OFF          0x50010000
#define IMX53_MMAP_ECSPI_1_LEN          (16 * 1024)
#define IMX53_MMAP_SSI_2_OFF            0x50014000
#define IMX53_MMAP_SSI_2_LEN            (16 * 1024)
#define IMX53_MMAP_ESAI_1_OFF           0x50018000
#define IMX53_MMAP_ESAI_1_LEN           (16 * 1024)
#define IMX53_MMAP_SDMA_INTERNAL_OFF    0x5001C000
#define IMX53_MMAP_SDMA_INTERNAL_LEN    (16 * 1024)
#define IMX53_MMAP_ESDHCV3_3_OFF        0x50020000
#define IMX53_MMAP_ESDHCV3_3_LEN        (16 * 1024)
#define IMX53_MMAP_ESDHCV2_4_OFF        0x50024000
#define IMX53_MMAP_ESDHCV2_4_LEN        (16 * 1024)
#define IMX53_MMAP_SPDIF_OFF            0x50028000
#define IMX53_MMAP_SPDIF_LEN            (16 * 1024)
#define IMX53_MMAP_ASRC_OFF             0x5002C000
#define IMX53_MMAP_ASRC_LEN             (16 * 1024)
#define IMX53_MMAP_PATA_UDMA_OFF        0x50030000
#define IMX53_MMAP_PATA_UDMA_LEN        (16 * 1024)
/* 0x50034000 - 0x50037FFF reserved */
/* 0x50038000 - 0x5003BFFF reserved */
#define IMX53_MMAP_SPBA_OFF             0x5003C000
#define IMX53_MMAP_SPBA_LEN             (16 * 1024)
/* 0x50040000 - 0x51FFFFFF reserved */
/* 0x52000000 - 0x53EFFFFF reserved */
/* 0x53F00000 - 0x53F7FFFF reserved */
#define IMX53_MMAP_USB_OFF              0x53F80000
#define IMX53_MMAP_USB_LEN              (16 * 1024)
#define IMX53_MMAP_GPIO_1_OFF           0x53F84000
#define IMX53_MMAP_GPIO_1_LEN           (16 * 1024)
#define IMX53_MMAP_GPIO_2_OFF           0x53F88000
#define IMX53_MMAP_GPIO_2_LEN           (16 * 1024)
#define IMX53_MMAP_GPIO_3_OFF           0x53F8C000
#define IMX53_MMAP_GPIO_3_LEN           (16 * 1024)
#define IMX53_MMAP_GPIO_4_OFF           0x53F90000
#define IMX53_MMAP_GPIO_4_LEN           (16 * 1024)
#define IMX53_MMAP_KPP_OFF              0x53F94000
#define IMX53_MMAP_KPP_LEN              (16 * 1024)
#define IMX53_MMAP_WDOG1_OFF            0x53F98000
#define IMX53_MMAP_WDOG1_LEN            (16 * 1024)
#define IMX53_MMAP_WDOG2_OFF            0x53F9C000
#define IMX53_MMAP_WDOG2_LEN            (16 * 1024)
#define IMX53_MMAP_GPT_OFF              0x53FA0000
#define IMX53_MMAP_GPT_LEN              (16 * 1024)
#define IMX53_MMAP_SRTC_OFF             0x53FA4000
#define IMX53_MMAP_SRTC_LEN             (16 * 1024)
#define IMX53_MMAP_IOMUXC_OFF           0x53FA8000
#define IMX53_MMAP_IOMUXC_LEN           (16 * 1024)
#define IMX53_MMAP_EPIT_1_OFF           0x53FAC000
#define IMX53_MMAP_EPIT_1_LEN           (16 * 1024)
#define IMX53_MMAP_EPIT_2_OFF           0x53FB0000
#define IMX53_MMAP_EPIT_2_LEN           (16 * 1024)
#define IMX53_MMAP_PWM_1_OFF            0x53FB4000
#define IMX53_MMAP_PWM_1_LEN            (16 * 1024)
#define IMX53_MMAP_PWM_2_OFF            0x53FB8000
#define IMX53_MMAP_PWM_2_LEN            (16 * 1024)
#define IMX53_MMAP_UART_1_OFF           0x53FBC000
#define IMX53_MMAP_UART_1_LEN           (16 * 1024)
#define IMX53_MMAP_UART_2_OFF           0x53FC0000
#define IMX53_MMAP_UART_2_LEN           (16 * 1024)
#define IMX53_MMAP_USB_PL301_OFF        0x53FC4000
#define IMX53_MMAP_USB_PL301_LEN        (16 * 1024)
#define IMX53_MMAP_FLEXCAN_1_OFF        0x53FC8000
#define IMX53_MMAP_FLEXCAN_1_LEN        (16 * 1024)
#define IMX53_MMAP_FLEXCAN_2_OFF        0x53FCC000
#define IMX53_MMAP_FLEXCAN_2_LEN        (16 * 1024)
#define IMX53_MMAP_SRC_OFF              0x53FD0000
#define IMX53_MMAP_SRC_LEN              (16 * 1024)
#define IMX53_MMAP_CCM_OFF              0x53FD4000
#define IMX53_MMAP_CCM_LEN              (16 * 1024)
#define IMX53_MMAP_GPC_OFF              0x53FD8000
#define IMX53_MMAP_GPC_LEN              (16 * 1024)
#define IMX53_MMAP_GPIO_5_OFF           0x53FDC000
#define IMX53_MMAP_GPIO_5_LEN           (16 * 1024)
#define IMX53_MMAP_GPIO_6_OFF           0x53FE0000
#define IMX53_MMAP_GPIO_6_LEN           (16 * 1024)
#define IMX53_MMAP_GPIO_7_OFF           0x53FE4000
#define IMX53_MMAP_GPIO_7_LEN           (16 * 1024)
#define IMX53_MMAP_PATA_PIO_OFF         0x53FE8000
#define IMX53_MMAP_PATA_PIO_LEN         (16 * 1024)
#define IMX53_MMAP_I2C_3_OFF            0x53FEC000
#define IMX53_MMAP_I2C_3_LEN            (16 * 1024)
#define IMX53_MMAP_UART_4_OFF           0x53FF0000
#define IMX53_MMAP_UART_4_LEN           (16 * 1024)
/* 0x53FF4000 - 0x53FFFFFF reserved */
/* 0x54000000 - 0x5FFFFFFF reserved */
/* 0x60000000 - 0x61FFFFFF reserved */
/* 0x62000000 - 0x63EFFFFF reserved */
/* 0x63F00000 - 0x63F7FFFF reserved */
#define IMX53_MMAP_DPLLC_1_OFF          0x63F80000
#define IMX53_MMAP_DPLLC_1_LEN          (16 * 1024)
#define IMX53_MMAP_DPLLC_2_OFF          0x63F84000
#define IMX53_MMAP_DPLLC_2_LEN          (16 * 1024)
#define IMX53_MMAP_DPLLC_3_OFF          0x63F88000
#define IMX53_MMAP_DPLLC_3_LEN          (16 * 1024)
#define IMX53_MMAP_DPLLC_4_OFF          0x63F8C000
#define IMX53_MMAP_DPLLC_4_LEN          (16 * 1024)
#define IMX53_MMAP_UART_5_OFF           0x63F90000
#define IMX53_MMAP_UART_5_LEN           (16 * 1024)
#define IMX53_MMAP_AHBMAX_OFF           0x63F94000
#define IMX53_MMAP_AHBMAX_LEN           (16 * 1024)
#define IMX53_MMAP_IIM_OFF              0x63F98000
#define IMX53_MMAP_IIM_LEN              (16 * 1024)
#define IMX53_MMAP_CSU_OFF              0x63F9C000
#define IMX53_MMAP_CSU_LEN              (16 * 1024)
#define IMX53_MMAP_ARM_PLATFORM_OFF     0x63FA0000
#define IMX53_MMAP_ARM_PLATFORM_LEN     (16 * 1024)
#define IMX53_MMAP_OWIRE_OFF            0x63FA4000
#define IMX53_MMAP_OWIRE_LEN            (16 * 1024)
#define IMX53_MMAP_FIRI_OFF             0x63FA8000
#define IMX53_MMAP_FIRI_LEN             (16 * 1024)
#define IMX53_MMAP_ECSPI_2_OFF          0x63FAC000
#define IMX53_MMAP_ECSPI_2_LEN          (16 * 1024)
#define IMX53_MMAP_SDMA_IPS_HOST_OFF    0x63FB0000
#define IMX53_MMAP_SDMA_IPS_HOST_LEN    (16 * 1024)
#define IMX53_MMAP_SCC_OFF              0x63FB4000
#define IMX53_MMAP_SCC_LEN              (16 * 1024)
#define IMX53_MMAP_ROMC_OFF             0x63FB8000
#define IMX53_MMAP_ROMC_LEN             (16 * 1024)
#define IMX53_MMAP_RTIC_OFF             0x63FBC000
#define IMX53_MMAP_RTIC_LEN             (16 * 1024)
#define IMX53_MMAP_CSPI_OFF             0x63FC0000
#define IMX53_MMAP_CSPI_LEN             (16 * 1024)
#define IMX53_MMAP_I2C_2_OFF            0x63FC4000
#define IMX53_MMAP_I2C_2_LEN            (16 * 1024)
#define IMX53_MMAP_I2C_1_OFF            0x63FC8000
#define IMX53_MMAP_I2C_1_LEN            (16 * 1024)
#define IMX53_MMAP_SSI_1_OFF            0x63FCC000
#define IMX53_MMAP_SSI_1_LEN            (16 * 1024)
#define IMX53_MMAP_AUDMUX_OFF           0x63FD0000
#define IMX53_MMAP_AUDMUX_LEN           (16 * 1024)
#define IMX53_MMAP_RTC_OFF              0x63FD4000
#define IMX53_MMAP_RTC_LEN              (16 * 1024)
#define IMX53_MMAP_EXTMC_OFF            0x63FD8000
#define IMX53_MMAP_EXTMC_LEN            (16 * 1024)
#define IMX53_MMAP_EXTMC_M4IF_OFF       0x63FD8000
#define IMX53_MMAP_EXTMC_M4IF_LEN       (0x1000)
#define IMX53_MMAP_EXTMC_ESDCTL_OFF     0x63FD9000
#define IMX53_MMAP_EXTMC_ESDCTL_LEN     (0x1000)
#define IMX53_MMAP_EXTMC_EIM_OFF        0x63FDA000
#define IMX53_MMAP_EXTMC_EIM_LEN        (0x1000)
#define IMX53_MMAP_EXTMC_NFC_OFF        0x63FDB000
#define IMX53_MMAP_EXTMC_NFC_LEN        (0xF00)
#define IMX53_MMAP_EXTMC_EXTMC_OFF      0x63FDBF00
#define IMX53_MMAP_EXTMC_EXTMC_LEN      (256)
#define IMX53_MMAP_PL301_2X2_OFF        0x63FDC000
#define IMX53_MMAP_PL301_2X2_LEN        (16 * 1024)
#define IMX53_MMAP_PL301_4X1_OFF        0x63FE0000
#define IMX53_MMAP_PL301_4X1_LEN        (16 * 1024)
#define IMX53_MMAP_MLB_OFF              0x63FE4000
#define IMX53_MMAP_MLB_LEN              (16 * 1024)
#define IMX53_MMAP_SSI_3_OFF            0x63FE8000
#define IMX53_MMAP_SSI_3_LEN            (16 * 1024)
#define IMX53_MMAP_FEC_OFF              0x63FEC000
#define IMX53_MMAP_FEC_LEN              (16 * 1024)
#define IMX53_MMAP_TVE_OFF              0x63FF0000
#define IMX53_MMAP_TVE_LEN              (16 * 1024)
#define IMX53_MMAP_VPU_OFF              0x63FF4000
#define IMX53_MMAP_VPU_LEN              (16 * 1024)
#define IMX53_MMAP_SAHARA_OFF           0x63FF8000
#define IMX53_MMAP_SAHARA_LEN           (16 * 1024)
#define IMX53_MMAP_PTP_OFF              0x63FFC000
#define IMX53_MMAP_PTP_LEN              (16 * 1024)
/* 0x64000000 - 0x6FFFBFFF reserved */
/* 0x6FFFC000 - 0x6FFFFFFF reserved */
#define IMX53_MMAP_CSD0_DDR_OFF         0x70000000
#define IMX53_MMAP_CSD0_DDR_LEN         (1 * 1024 * 1024 * 1024)
#define IMX53_MMAP_CSD1_DDR_OFF         0xB0000000
#define IMX53_MMAP_CSD1_DDR_LEN         (1 * 1024 * 1024 * 1024)
#define IMX53_MMAP_CS0_OFF              0xF0000000
#define IMX53_MMAP_CS0_LEN              (128 * 1024 * 1024 - 16 * 1024)
#define IMX53_MMAP_2CS_CS0_OFF          0xF0000000
#define IMX53_MMAP_2CS_CS0_LEN          (64 * 1024 * 1024)
#define IMX53_MMAP_2CS_CS1_OFF          0xF4000000
#define IMX53_MMAP_2CS_CS1_LEN          (64 * 1024 * 1024 - 16 * 1024)
#define IMX53_MMAP_3CS_CS0_OFF          0xF0000000
#define IMX53_MMAP_3CS_CS0_LEN          (64 * 1024 * 1024)
#define IMX53_MMAP_3CS_CS1_OFF          0xF4000000
#define IMX53_MMAP_3CS_CS1_LEN          (32 * 1024 * 1024)
#define IMX53_MMAP_3CS_CS2_OFF          0xF6000000
#define IMX53_MMAP_3CS_CS2_LEN          (32 * 1024 * 1024 - 16 * 1024)
#define IMX53_MMAP_4CS_CS0_OFF          0xF0000000
#define IMX53_MMAP_4CS_CS0_LEN          (32 * 1024 * 1024)
#define IMX53_MMAP_4CS_CS1_OFF          0xF2000000
#define IMX53_MMAP_4CS_CS1_LEN          (32 * 1024 * 1024)
#define IMX53_MMAP_4CS_CS2_OFF          0xF4000000
#define IMX53_MMAP_4CS_CS2_LEN          (32 * 1024 * 1024)
#define IMX53_MMAP_4CS_CS3_OFF          0xF6000000
#define IMX53_MMAP_4CS_CS3_LEN          (32 * 1024 * 1024 - 16 * 1024)
#define IMX53_MMAP_NAND_INTERNAL_OFF    0xF7FF0000
#define IMX53_MMAP_NAND_INTERNAL_LEN    (64 * 1024)
#define IMX53_MMAP_IRAM_OFF             0xF8000000
#define IMX53_MMAP_IRAM_LEN             (128 * 1024)
#define IMX53_MMAP_GPU3D_GMEM_OFF       0xF8020000
#define IMX53_MMAP_GPU3D_GMEM_LEN       (256 * 1024)
/* 0xF8060000 - 0xFFFFFFFF reserved */

/* 0 reserved */
#define IMX53_IRQ_ESDHCV2_1         1
#define IMX53_IRQ_ESDHCV2_2         2
#define IMX53_IRQ_ESDHCV3_3         3
#define IMX53_IRQ_ESDHCV2_4         4
#define IMX53_IRQ_DAP               5
#define IMX53_IRQ_SDMA              6
#define IMX53_IRQ_IOMUXC_POWER_FAIL 7
#define IMX53_IRQ_EXTMC_NFC         8
#define IMX53_IRQ_VPU               9
#define IMX53_IRQ_IPU_ERROR         10
#define IMX53_IRQ_IPU_SYNC          11
#define IMX53_IRQ_GPU3D             12
#define IMX53_IRQ_UART_4            13
#define IMX53_IRQ_USB_HOST_1        14
#define IMX53_IRQ_EXTMC             15
#define IMX53_IRQ_USB_HOST_2        16
#define IMX53_IRQ_USB_HOST_3        17
#define IMX53_IRQ_USB_OTG           18
#define IMX53_IRQ_SAHARA_HOST_0     19
#define IMX53_IRQ_SAHARA_HOST_1     20
#define IMX53_IRQ_SCC               21
#define IMX53_IRQ_SCC_TRUSTZONE     22
#define IMX53_IRQ_SCC_NONSECURE     23
#define IMX53_IRQ_SRTC              24
#define IMX53_IRQ_SRTC_TRUSTZONE    25
#define IMX53_IRQ_RTIC_TRUSTZONE    26
#define IMX53_IRQ_CSU               27
#define IMX53_IRQ_SATA              28
#define IMX53_IRQ_SSI_1             29
#define IMX53_IRQ_SSI_2             30
#define IMX53_IRQ_UART_1            31
#define IMX53_IRQ_UART_2            32
#define IMX53_IRQ_UART_3            33
#define IMX53_IRQ_IPTP_RTC          34
#define IMX53_IRQ_IPTP_PTP          35
#define IMX53_IRQ_ECSPI_1           36
#define IMX53_IRQ_ECSPI_2           37
#define IMX53_IRQ_CSPI              38
#define IMX53_IRQ_GPT               39
#define IMX53_IRQ_EPIT_1            40
#define IMX53_IRQ_EPIT_2            41
#define IMX53_IRQ_GPIO_1_INT7       42
#define IMX53_IRQ_GPIO_1_INT6       43
#define IMX53_IRQ_GPIO_1_INT5       44
#define IMX53_IRQ_GPIO_1_INT4       45
#define IMX53_IRQ_GPIO_1_INT3       46
#define IMX53_IRQ_GPIO_1_INT2       47
#define IMX53_IRQ_GPIO_1_INT1       48
#define IMX53_IRQ_GPIO_1_INT0       49
#define IMX53_IRQ_GPIO_1_LOWER      50
#define IMX53_IRQ_GPIO_1_UPPER      51
#define IMX53_IRQ_GPIO_2_LOWER      52
#define IMX53_IRQ_GPIO_2_UPPER      53
#define IMX53_IRQ_GPIO_3_LOWER      54
#define IMX53_IRQ_GPIO_3_UPPER      55
#define IMX53_IRQ_GPIO_4_LOWER      56
#define IMX53_IRQ_GPIO_4_UPPER      57
#define IMX53_IRQ_WDOG              58
#define IMX53_IRQ_WDOG_TRUSTZONE    59
#define IMX53_IRQ_KPP               60
#define IMX53_IRQ_PWM_1             61
#define IMX53_IRQ_I2C_1             62
#define IMX53_IRQ_I2C_2             63
#define IMX53_IRQ_I2C_3             64
#define IMX53_IRQ_MLB               65
#define IMX53_IRQ_ASRC              66
#define IMX53_IRQ_SPDIF             67
/* 68 reserved */
#define IMX53_IRQ_IIM               69
#define IMX53_IRQ_PATA              70
#define IMX53_IRQ_CCM_1             71
#define IMX53_IRQ_CCM_2             72
#define IMX53_IRQ_GPC_1             73
#define IMX53_IRQ_GPC_2             74
#define IMX53_IRQ_SRC               75
#define IMX53_IRQ_DEBUG_NEON        76
#define IMX53_IRQ_DEBUG_PMUIRQ      77
#define IMX53_IRQ_DEBUG_CTI         78
#define IMX53_IRQ_DEBUG_CTI_1_ITF_1 79
#define IMX53_IRQ_DEBUG_CTI_1_ITF_0 80
#define IMX53_IRQ_ESAI              81
#define IMX53_IRQ_FLEXCAN_1         82
#define IMX53_IRQ_FLEXCAN_2         83
#define IMX53_IRQ_OPENVG            84
#define IMX53_IRQ_OPENVG_BUSY       85
#define IMX53_IRQ_UART_5            86
#define IMX53_IRQ_FEC               87
#define IMX53_IRQ_OWIRE             88
#define IMX53_IRQ_DEBUG_CTI_1_ITF_2 89
#define IMX53_IRQ_SJC               90
/* 91 reserved */
#define IMX53_IRQ_TVE               92
#define IMX53_IRQ_FIRI              93
#define IMX53_IRQ_PWM_2             94
/* 95 reserved */
#define IMX53_IRQ_SSI_3             96
/* 97 reserved */
#define IMX53_IRQ_DEBUG_CTI_1_ITF_3 98
/* 99 reserved */
#define IMX53_IRQ_VPU_IDLE          100
#define IMX53_IRQ_EXTMC_NFC_PAGES   101
#define IMX53_IRQ_GPU3D_IDLE        102
#define IMX53_IRQ_GPIO_5_LOWER      103
#define IMX53_IRQ_GPIO_5_UPPER      104
#define IMX53_IRQ_GPIO_6_LOWER      105
#define IMX53_IRQ_GPIO_6_UPPER      106
#define IMX53_IRQ_GPIO_7_LOWER      107
#define IMX53_IRQ_GPIO_7_UPPER      108
/* 109-127 reserved */

#endif
