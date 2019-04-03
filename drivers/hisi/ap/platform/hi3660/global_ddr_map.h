#ifndef _HI_GLOBAL_MEM_MAP_INCLUDE_H_
#define _HI_GLOBAL_MEM_MAP_INCLUDE_H_
#define HISI_RESERVED_FASTBOOT_PHYMEM_BASE HISI_RESERVED_ISP_BOOT_PHYMEM_BASE
#define HISI_RESERVED_FASTBOOT_PHYMEM_SIZE (0x400000)
#define HISI_RESERVED_FASTBOOT_DTB_PHYMEM_BASE (0x16C00000)
#define HISI_RESERVED_FASTBOOT_DTB_PHYMEM_SIZE (0x4000000)
#define HISI_RESERVED_DTB_PHYMEM_BASE 0x07A00000
#define HISI_RESERVED_DTB_PHYMEM_SIZE (0x07C00000 - 0x07A00000)
#define HISI_RESERVED_ISP_BOOT_PHYMEM_BASE 0x1AC00000
#define HISI_RESERVED_ISP_BOOT_PHYMEM_SIZE (0x2500000)
#define HISI_RESERVED_IVP_PHYMEM_BASE 0x1D100000
#define HISI_RESERVED_IVP_PHYMEM_SIZE (0x100000)
#define HISI_RESERVED_SECOS_PHYMEM_BASE 0x1D200000
#define HISI_RESERVED_SECOS_PHYMEM_SIZE (0x3000000)
#define HISI_RESERVED_BL31_PHYMEM_BASE 0x20200000
#define HISI_RESERVED_BL31_PHYMEM_SIZE (0x200000)
#define HISI_RESERVED_SENSORHUB_PHYMEM_BASE 0x20400000
#define HISI_RESERVED_SENSORHUB_PHYMEM_SIZE (0x600000)
#define HISI_RESERVED_PSTORE_PHYMEM_BASE 0x20A00000
#define HISI_RESERVED_PSTORE_PHYMEM_SIZE (0x100000)
#define HISI_RESERVED_FAST_KER_AND_PHYMEM_BASE 0x20B00000
#define HISI_RESERVED_FAST_KER_AND_PHYMEM_SIZE (0x40000)
#define HISI_SUB_RESERVED_FASTBOOT_LOG_PYHMEM_BASE 0x20B00000
#define HISI_SUB_RESERVED_FASTBOOT_LOG_PYHMEM_SIZE (0x20000)
#define HISI_SUB_RESERVED_SCHARGE_PYHMEM_BASE 0x20B20000
#define HISI_SUB_RESERVED_SCHARGE_PYHMEM_SIZE (0x1000)
#define HISI_SUB_RESERVED_BL31_SHARE_MEM_PHYMEM_BASE 0x20B21000
#define HISI_SUB_RESERVED_BL31_SHARE_MEM_PHYMEM_SIZE (0x10000)
#define HISI_SUB_RESERVED_LCD_GAMMA_MEM_PHYMEM_BASE 0x20B31000
#define HISI_SUB_RESERVED_LCD_GAMMA_MEM_PHYMEM_SIZE (0x1000)
#define HISI_SUB_RESERVED_UNUSED_PHYMEM_BASE 0x20B32000
#define HISI_SUB_RESERVED_UNUSED_PHYMEM_SIZE (0x20B40000 - 0x20B32000)
#define HISI_RESERVED_SENSORHUB_SHMEM_PHYMEM_BASE 0x20B40000
#define HISI_RESERVED_SENSORHUB_SHMEM_PHYMEM_SIZE (0x40000)
#define HISI_CONTEXTHUB_THERMAL_SHMEM_PHYMEM_BASE HISI_RESERVED_SENSORHUB_SHMEM_PHYMEM_BASE
#define HISI_CONTEXTHUB_THERMAL_SHMEM_PHYMEM_SIZE (0x3C00)
#define HISI_RESERVED_CH_BLOCK_SHMEM_PHYMEM_BASE (HISI_CONTEXTHUB_THERMAL_SHMEM_PHYMEM_BASE + HISI_CONTEXTHUB_THERMAL_SHMEM_PHYMEM_SIZE)
#define HISI_RESERVED_CH_BLOCK_SHMEM_PHYMEM_SIZE (HISI_RESERVED_SENSORHUB_SHMEM_PHYMEM_SIZE - HISI_CONTEXTHUB_THERMAL_SHMEM_PHYMEM_SIZE)
#define HISI_RESERVED_SENSORHUB_SHARE_MEM_PHYMEM_BASE 0x20B80000
#define HISI_RESERVED_SENSORHUB_SHARE_MEM_PHYMEM_SIZE (0x80000)
#define HISI_RESERVED_MODEM_PHYMEM_BASE 0x80000000
#define HISI_RESERVED_MODEM_PHYMEM_SIZE (0x9200000)
#define HISI_RESERVED_HIFI_PHYMEM_BASE 0x89200000
#define HISI_RESERVED_HIFI_PHYMEM_SIZE (0x980000)
#define HISI_RESERVED_LPMX_CORE_PHYMEM_BASE 0x89B80000
#define HISI_RESERVED_LPMX_CORE_PHYMEM_SIZE (0x100000)
#define HISI_RESERVED_LPMCU_PHYMEM_BASE 0x89C80000
#define HISI_RESERVED_LPMCU_PHYMEM_SIZE 0x40000
#define HISI_RESERVED_MODEM_SHARE_PHYMEM_BASE 0x89D00000
#define HISI_RESERVED_MODEM_SHARE_PHYMEM_SIZE (0x500000)
#define HISI_RESERVED_MODEM_SOCP_PHYMEM_BASE 0x8A200000
#define HISI_RESERVED_MODEM_SOCP_PHYMEM_SIZE (0x1100000)
#define HISI_RESERVED_HIFI_DATA_PHYMEM_BASE 0x8B300000
#define HISI_RESERVED_HIFI_DATA_PHYMEM_SIZE (0x380000)
#define HISI_RESERVED_MNTN_PHYMEM_BASE 0x8B680000
#define HISI_RESERVED_MNTN_PHYMEM_SIZE (0x800000)
#define HISI_RESERVED_DDR_TRAINING1_PHYMEM_BASE 0x07B00000
#define HISI_RESERVED_DDR_TRAINING1_PHYMEM_SIZE (0x07B08000 - 0x07B00000)
#define HISI_RESERVED_LPMX_CORE_PHYMEM_BASE_UNIQUE (HISI_RESERVED_LPMX_CORE_PHYMEM_BASE)
#define HISI_RESERVED_LPMCU_PHYMEM_BASE_UNIQUE (HISI_RESERVED_LPMCU_PHYMEM_BASE)
#define HISI_RESERVED_MNTN_PHYMEM_BASE_UNIQUE (HISI_RESERVED_MNTN_PHYMEM_BASE)
#endif
