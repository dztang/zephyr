#include <zephyr/devicetree.h>
#include <zephyr/linker/sections.h>

/* ------------------------------------------------------------------------- */

/* Memory reserved for Bootstrap code and data */
BOOTSTRAP_SIZE = DEFINED(APP_BOOTSTRAP_SIZE) ? APP_BOOTSTRAP_SIZE : 0x00002400 ;

/* The size of the stack section at the end of CM4 SRAM */
STACK_SIZE = 0x1000;

FLASH_START_ADDR_SAHB       = 0x60000000;
FLASH_START_ADDR_CBUS       = 0x08000000;
RAM_START_ADDR_SAHB         = 0x20000000;
RAM_END_ADDR_SAHB           = 0x20040000; /* 256K */

RAM_START_ADDR_CBUS         = 0x20000000;
RAM_END_ADDR_CBUS           = 0x20040000; /* 256K */

FLASH_END_ADDR_SAHB         = 0x60080000; /* 512K */

BOOTSTRAP_OFFSET_FLASH      = 0x00000050; /* toc2=0x10, l1_desc=0x1C, sign_header=0x20 */

RAMVECTORS_ALIGNMENT        = 512;
FLASH_ALIGNMENT_SIZE        = 0x00000200;

/* vma for bootstrap code region */
CODE_VMA                    = RAM_START_ADDR_CBUS + (RAM_END_ADDR_SAHB - RAM_START_ADDR_SAHB - BOOTSTRAP_SIZE); /* 0x2003DC00 */
/* lma for bootstrap code region */
CODE_LMA                    = FLASH_START_ADDR_SAHB + BOOTSTRAP_OFFSET_FLASH; /* 0x60000050 */
/* Maximum bootstrap code + data size */
CODE_BS_SIZE                = BOOTSTRAP_SIZE; /* 9KB */
/* vma for bootstrap data region */
DATA_BS_VMA                 = RAM_END_ADDR_SAHB - BOOTSTRAP_SIZE; /* 0x2003DC00 */
/* vma for bootstrap and app data region */
DATA_VMA                    = RAM_START_ADDR_SAHB; /* 0x20000000 */
/* vma for appCodeRam region */
DATA_CBUS_VMA               = RAM_START_ADDR_CBUS; /* 0x20000000 */
/* lma for bootstrap and app data region */
DATA_LMA                    = CODE_LMA + CODE_BS_SIZE; /* 0x60002450 */
/* data size */
DATA_SIZE                   = RAM_END_ADDR_SAHB - DATA_VMA - BOOTSTRAP_SIZE; /* 0x3DC00 */
/* vma for application XIP region */
XIP_VMA                     = FLASH_START_ADDR_CBUS + BOOTSTRAP_SIZE + FLASH_ALIGNMENT_SIZE; /* 0x08002600 */
/* lma for application XIP region */
XIP_LMA                     = FLASH_START_ADDR_SAHB + BOOTSTRAP_SIZE + FLASH_ALIGNMENT_SIZE; /* 0x60002600 */
/* size of XIP region */
XIP_SIZE                    = FLASH_END_ADDR_SAHB - XIP_LMA;
/* Total size of SRAM */
RAM_SIZE                    = RAM_END_ADDR_SAHB - RAM_START_ADDR_SAHB; /* 0x00040000 */
/* Size of Bootstrap data is kept same as BOOTSTRAP_SIZE */
DATA_BS_SIZE                = BOOTSTRAP_SIZE;

/* ------------------------------------------------------------------------- */
/* Boot strap section */

MEMORY
{
    BS_CODE (rwx) : ORIGIN = CODE_VMA, LENGTH = CODE_BS_SIZE
    BS_DATA (rwx) : ORIGIN = DATA_BS_VMA,  LENGTH = DATA_BS_SIZE
}


SECTIONS
{
    __bootstrapText_vma__ = ORIGIN(BS_CODE);
    __bootstrapText_lma__ = CODE_LMA;


   /* Cortex-M33 bootstrap code area */
    .bootstrapText __bootstrapText_vma__ : AT (__bootstrapText_lma__)
    {
        /* Cortex-M33 code vector table */
        . = ALIGN(4);
        __bootstrapText_begin = .;

        /* Located in generated directory. This file is populated by calling
         * zephyr_linker_sources(ROM_START ...). This typically contains the vector
         * table and debug information.
         */
        #include <snippets-rom-start.ld>

        /* startup code */
        *(.text._reset_section)

        *startup_cat1b_cm33.*(.text*)
        *system_cyw20829.*(.text*)

        /* drivers */
        *cy_device.*(.text*)
        *cy_btss.*(.text*)
        *cy_sysclk_v2.*(.text*)
        *cy_syspm_v2.*(.text*)
        *cy_sysint_v2.*(.text*)
        *cy_syslib*.*(.text*)
        *ppu_v1.*(.text*)
        *cy_mpc.*(.text*)
        *cy_syspm_ppu.*(.text*)

        *memcpy*.* (.text*)   /* add memcpy from the NewLib library here*/
        *memset*.* (.text*)   /* add memcpy from the NewLib library here*/
        *memmove*.* (.text*)  /* add memcpy from the NewLib library here*/
        *s_fabs.* (.text*)

        KEEP(*(.cy_l1func*))

        . = ALIGN(4);
        __bootstrapText_end = .;
    } > BS_CODE

    __bootstrap_zerotable_lma__ = (CODE_LMA + (__bootstrapText_end - __bootstrapText_begin));

    .bootstrapzero.table : AT (__bootstrap_zerotable_lma__)
    {
        . = ALIGN(4);
        __bootstrapzero_table_start__ = .;
        LONG (__bootstrap_bss_start__)
        LONG ((__bootstrap_bss_end__ - __bootstrap_bss_start__)/4)
        . = ALIGN(4);
        __bootstrapzero_table_end__ = .;
    } > BS_CODE

    __bootstrapData_vma__ = ((__bootstrapzero_table_end__ - RAM_START_ADDR_CBUS) + RAM_START_ADDR_SAHB);
    __bootstrapData_lma__ = (__bootstrap_zerotable_lma__ + (__bootstrapzero_table_end__ - __bootstrapzero_table_start__));

    .bootstrapData __bootstrapData_vma__ : AT (__bootstrapData_lma__)
    {
        __bootstrapData_start__ = .;
        . = ALIGN(4);

        /* startup code */
        *startup_cat1b_cm33.*(.data* .rodata .rodata.* .constdata .constdata.* .conststring .conststring.*)
        *system_cyw20829.*(.data* .rodata .rodata.* .constdata .constdata.* .conststring .conststring.*)

        /* drivers */
        *cy_device.*(.data* .rodata .rodata.* .constdata .constdata.* .conststring .conststring.*)
        *cy_btss.*(.data* .rodata .rodata.* .constdata .constdata.* .conststring .conststring.*)
        *cy_sysclk_v2.*(.data* .rodata .rodata.* .constdata .constdata.* .conststring .conststring.*)
        *cy_syspm_v2.*(.data* .rodata .rodata.* .constdata .constdata.* .conststring .conststring.*)
        *cy_sysint_v2.*(.data* .rodata .rodata.* .constdata .constdata.* .conststring .conststring.*)
        *cy_syslib.*(.data* .rodata .rodata.* .constdata .constdata.* .conststring .conststring.*)
        *ppu_v1.*(.data* .rodata .rodata.* .constdata .constdata.* .conststring .conststring.*)
        *cy_mpc.*(.data* .rodata .rodata.* .constdata .constdata.* .conststring .conststring.*)
        *cy_pd_ppu.*(.data* .rodata .rodata.* .constdata .constdata.* .conststring .conststring.*)

        KEEP(*(.cy_l1data*))

        . = ALIGN(4);
        __bootstrapData_end__ = .;
    } > BS_DATA

    __bootstrap_size_end__ = .;
    .bootstrapBss (NOLOAD):
    {
        . = ALIGN(4);
        __bootstrap_bss_start__ = .;

        /* startup code */
        *startup_cat1b_cm33.*(.bss* COMMON)
        *system_cyw20829.*(.bss* COMMON)

        /* drivers */
        *cy_device.*(.bss* COMMON)
        *cy_btss.*(.bss* COMMON)
        *cy_sysclk_v2.*(.bss* COMMON)
        *cy_syspm_v2.*(.bss* COMMON)
        *cy_sysint_v2.*(.bss* COMMON)
        *cy_syslib.*(.bss* COMMON)
        *ppu_v1.*(.bss* COMMON)
        *cy_mpc.*(.bss* COMMON)
        *cy_pd_ppu.*(.bss* COMMON)
        KEEP(*(.cy_l1bss*))

        . = ALIGN(4);
        __bootstrap_bss_end__ = .;
    } > BS_DATA

}

#include <zephyr/arch/arm/cortex_m/scripts/linker.ld>
