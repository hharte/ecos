//=============================================================================
//
//      hal_aux.c
//
//      HAL auxiliary objects and code; per platform
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   hmt
// Contributors:hmt, gthomas
// Date:        1999-06-08
// Purpose:     HAL aux objects: startup tables.
// Description: Tables for per-platform initialization
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_mem.h>            // HAL memory definitions
#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/ppc_regs.h>           // Platform registers
#include <cyg/hal/hal_if.h>             // hal_if_init
#include <cyg/hal/hal_intr.h>           // interrupt definitions
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/hal/hal_io.h>             // I/O macros
#include <cyg/infra/diag.h>
#include CYGHWR_MEMORY_LAYOUT_H

#ifdef CYGPKG_REDBOOT
#include <redboot.h>
#endif

// The memory map is weakly defined, allowing the application to redefine
// it if necessary. The regions defined below are the minimum requirements.
CYGARC_MEMDESC_TABLE CYGBLD_ATTRIB_WEAK = {
    // Mapping for the TAMS MOAB development boards
    CYGARC_MEMDESC_NOCACHE( 0xFFE00000, 0x00200000 ), // Boot ROM
    CYGARC_MEMDESC_NOCACHE( 0xEF000000, 0x01000000 ), // CPU registers
    CYGARC_MEMDESC_NOCACHE( 0xEE000000, 0x01000000 ), // PCI CFG 
    CYGARC_MEMDESC_NOCACHE( 0xE8000000, 0x01000000 ), // PCI I/O
    CYGARC_MEMDESC_NOCACHE( 0xA0000000, 0x10000000 ), // PCI Memory
    CYGARC_MEMDESC_NOCACHE( _MOAB_NAND, 0x00100000 ), // Main [NAND] FLASH
    CYGARC_MEMDESC_NOCACHE( _MOAB_OCM,  0x01000000 ), // OCM
    CYGARC_MEMDESC_NOCACHE_PA( 0x80000000, 0x00000000, CYGMEM_REGION_ram_SIZE ), // Uncached version of RAM
    CYGARC_MEMDESC_CACHE(   CYGMEM_REGION_ram, CYGMEM_REGION_ram_SIZE ), // Main memory
    CYGARC_MEMDESC_TABLE_END
};

//--------------------------------------------------------------------------
// Platform init code.
void
hal_platform_init(void)
{
}

#ifdef CYGSEM_REDBOOT_PLF_STARTUP
void
cyg_plf_redboot_startup(void)
{
    cyg_uint32 gpio_ir;

    HAL_READ_UINT32(GPIO_IR, gpio_ir);
    if ((gpio_ir & 0x00080000) != 0) {
        // Load RedBoot from NAND FLASH and execute it
        script = "fi lo RedBoot;go\n";
        script_timeout = 1;
    }
}
#endif
//
// Initialize serial ports - called during hal_if_init()
// Note: actual serial port support code is supported by the PPC405 variant layer
//       Having this call here allows for additional platform specific additions
//
void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;
    initialized = 1;

    cyg_hal_var_serial_init();
}

//----------------------------------------------------------------------------
// Reset.
void
_moab_reset(void)
{
    CYGARC_MTSPR(SPR_DBCR0, 0x30000000);  // Asserts system reset
    while (1) ;
}

#ifdef CYGPKG_REDBOOT
//----------------------------------------------------------------------------
// Memory map [segment] support for RedBoot
void 
cyg_plf_memory_segment(int seg, unsigned char **start, unsigned char **end)
{
    if (seg == 1) {
        *start = _MOAB_OCM;
        *end = _MOAB_OCM + 0x1000;
    } else {
        diag_printf("** Invalid memory segment #%d - ignored\n", seg);
        *start = NO_MEMORY;
        *end = NO_MEMORY;
    }
}
#endif

//--------------------------------------------------------------------------
// EEPROM support - 1024 bytes, treated as a single read/write block
void
read_eeprom(unsigned char *buf, int len)
{
    int i, page, page_addr, size;
    cyg_uint8 addr[2];

    if (len > CYGNUM_HAL_EEPROM_SIZE) {
        diag_printf("%s - Illegal length: %d\n", __FUNCTION__, len);
        return;
    }
#if CYGNUM_HAL_EEPROM_SIZE == 1024
    // Read from start of EEPROM
    for (i = 0;  i < len;  i += 4) {
        page = 0xA9 + ((i >> 8) << 1);
        if ((i % 256) == 0) {
            addr[0] = 0x00;
            if (!hal_ppc405_i2c_put_bytes(page, addr, 1)) {
                diag_printf("%s - Can't select page %x\n", __FUNCTION__, page);
                return;
            }
        }
        if (!hal_ppc405_i2c_get_bytes(page, &buf[i], 4)) {
            diag_printf("%s - Can't read byte %d\n", __FUNCTION__, i);
            return;
        }
    }
#else
    // Read from start of EEPROM
    page = 0xAF;  page_addr = 0;
    for (i = 0;  i < len;  i += size) {
        addr[0] = page_addr >> 8;  addr[1] = (page_addr & 0xFF);
        size = (len - i);
        if (size > 32) size = 32;
        if (!hal_ppc405_i2c_put_bytes(page, &addr, 2)) {
            diag_printf("%s - Can't select address %x\n", __FUNCTION__, page);
            return;
        }
        if (!hal_ppc405_i2c_get_bytes(page, &buf[i], size)) {
            diag_printf("%s - Can't read bytes\n", __FUNCTION__);
            return;
        }
        page_addr += size;
    }
#endif
#if 0
    diag_printf("EEPROM data - read\n");
    diag_dump_buf(buf, len);
#endif
}

void
write_eeprom(unsigned char *buf, int len)
{
    int i, j, page, page_addr, size, left;
    cyg_uint8 addr[32+2];

#if 0
    diag_printf("EEPROM data - write\n");
    diag_dump_buf(buf, len < 256 ? len : 256);
#endif
    if (len > CYGNUM_HAL_EEPROM_SIZE) {
        diag_printf("%s - Illegal length: %d\n", __FUNCTION__, len);
        return;
    }
#if CYGNUM_HAL_EEPROM_SIZE == 1024
    // Write from start of EEPROM
    for (left = len, i = 0;  i < len;  i += size, left -= size) {
        addr[0] = i;
        size = sizeof(addr)-1;
        if (size > left) size = left;
        for (j = 0;  j < size;  j++) {
            addr[j+1] = buf[i+j];
        }
        page = 0xA8 + ((i >> 8) << 1);
        if (!hal_ppc405_i2c_put_bytes(page, addr, size+1)) {
            diag_printf("%s - Can't write byte - page: %x, addr: %d\n", __FUNCTION__, page, i);
            return;
        }
        // Give device time to recover
        CYGACC_CALL_IF_DELAY_US(10000);   // 10ms
    }
#else
    // Write from start of EEPROM
    page_addr = 0;  page = 0xAE;
    for (i = 0;  i < len;  i += size) {
        addr[0] = page_addr >> 8;  addr[1] = page_addr & 0xFF;
        size = (len - i);
        if (size > 32) size = 32;
        for (j = 0;  j < size;  j++) {
            addr[j+2] = buf[i+j];
        }
        if (!hal_ppc405_i2c_put_bytes(page, addr, size+2)) {
            diag_printf("%s - Can't write byte - page: %x, addr: %d\n", __FUNCTION__, page, i);
            return;
        }
        // Give device time to recover
        CYGACC_CALL_IF_DELAY_US(50000);   // 10ms
        page_addr += size;
    }
#endif
}

// Interrupt support

void
hal_platform_IRQ_init(void)
{
}

// EOF hal_aux.c
