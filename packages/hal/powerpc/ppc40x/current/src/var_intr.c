//==========================================================================
//
//      var_intr.c
//
//      PowerPC variant interrupt handlers
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.1 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://www.redhat.com/                                                   
//                                                                          
// Software distributed under the License is distributed on an "AS IS"      
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
// License for the specific language governing rights and limitations under 
// the License.                                                             
//                                                                          
// The Original Code is eCos - Embedded Configurable Operating System,      
// released September 30, 1998.                                             
//                                                                          
// The Initial Developer of the Original Code is Red Hat.                   
// Portions created by Red Hat are                                          
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jskov
// Contributors: jskov
// Date:         2000-02-11
// Purpose:      PowerPC variant interrupt handlers
// Description:  This file contains code to handle interrupt related issues
//               on the PowerPC variant.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/ppc_regs.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/cyg_type.h>

static cyg_uint32 exier_mask[] = {
    0x00000000, // Unused
    0x00000000, // Unused
    0x80000000, // CYGNUM_HAL_INTERRUPT_CRITICAL         2
    0x08000000, // CYGNUM_HAL_INTERRUPT_SERIAL_RCV       3
    0x04000000, // CYGNUM_HAL_INTERRUPT_SERIAL_XMT       4
    0x02000000, // CYGNUM_HAL_INTERRUPT_JTAG_RCV         5
    0x01000000, // CYGNUM_HAL_INTERRUPT_JTAG_XMT         6
    0x00800000, // CYGNUM_HAL_INTERRUPT_DMA0             7
    0x00400000, // CYGNUM_HAL_INTERRUPT_DMA1             8
    0x00200000, // CYGNUM_HAL_INTERRUPT_DMA2             9
    0x00100000, // CYGNUM_HAL_INTERRUPT_DMA3            10
    0x00000010, // CYGNUM_HAL_INTERRUPT_EXT0            11
    0x00000008, // CYGNUM_HAL_INTERRUPT_EXT1            12
    0x00000004, // CYGNUM_HAL_INTERRUPT_EXT2            13
    0x00000002, // CYGNUM_HAL_INTERRUPT_EXT3            14
    0x00000001, // CYGNUM_HAL_INTERRUPT_EXT4            15
};

// This table inverts bit number to signal number
cyg_uint32 EXISR_TAB[] = {
    CYGNUM_HAL_INTERRUPT_CRITICAL,     // 0x80000000
    0x00000000,                        // 0x40000000
    0x00000000,                        // 0x20000000
    0x00000000,                        // 0x10000000
    CYGNUM_HAL_INTERRUPT_SERIAL_RCV,   // 0x08000000
    CYGNUM_HAL_INTERRUPT_SERIAL_XMT,   // 0x04000000
    CYGNUM_HAL_INTERRUPT_JTAG_RCV,     // 0x02000000
    CYGNUM_HAL_INTERRUPT_JTAG_XMT,     // 0x01000000
    CYGNUM_HAL_INTERRUPT_DMA0,         // 0x00800000
    CYGNUM_HAL_INTERRUPT_DMA1,         // 0x00400000
    CYGNUM_HAL_INTERRUPT_DMA2,         // 0x00200000
    CYGNUM_HAL_INTERRUPT_DMA3,         // 0x00100000
    0x00000000,                        // 0x00080000
    0x00000000,                        // 0x00040000
    0x00000000,                        // 0x00020000
    0x00000000,                        // 0x00010000
    0x00000000,                        // 0x00008000
    0x00000000,                        // 0x00004000
    0x00000000,                        // 0x00002000
    0x00000000,                        // 0x00001000
    0x00000000,                        // 0x00000800
    0x00000000,                        // 0x00000400
    0x00000000,                        // 0x00000200
    0x00000000,                        // 0x00000100
    0x00000000,                        // 0x00000080
    0x00000000,                        // 0x00000040
    0x00000000,                        // 0x00000020
    CYGNUM_HAL_INTERRUPT_EXT0,         // 0x00000010
    CYGNUM_HAL_INTERRUPT_EXT1,         // 0x00000008
    CYGNUM_HAL_INTERRUPT_EXT2,         // 0x00000004
    CYGNUM_HAL_INTERRUPT_EXT3,         // 0x00000002
    CYGNUM_HAL_INTERRUPT_EXT4          // 0x00000001
};

cyg_uint32 _hold_tcr = 0;  // Shadow of hardware register

externC void
hal_variant_IRQ_init(void)
{
    cyg_uint32 iocr;

    // Ensure all interrupts masked (disabled) & cleared
    CYGARC_MTDCR(DCR_EXIER, 0);
    CYGARC_MTDCR(DCR_EXISR, 0xFFFFFFFF);

    // Configure all external interrupts to be level/low
    CYGARC_MFDCR(DCR_IOCR, iocr);
    iocr &= ~0xFFC00000;
    CYGARC_MTDCR(DCR_IOCR, iocr);

    // Disable timers
    CYGARC_MTSPR(SPR_TCR, 0);
}

externC void 
hal_ppc40x_interrupt_mask(int vector)
{
    cyg_uint32 exier, tcr;

    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_CRITICAL:
    case CYGNUM_HAL_INTERRUPT_SERIAL_RCV:
    case CYGNUM_HAL_INTERRUPT_SERIAL_XMT:
    case CYGNUM_HAL_INTERRUPT_JTAG_RCV:
    case CYGNUM_HAL_INTERRUPT_JTAG_XMT:
    case CYGNUM_HAL_INTERRUPT_DMA0:
    case CYGNUM_HAL_INTERRUPT_DMA1:
    case CYGNUM_HAL_INTERRUPT_DMA2:
    case CYGNUM_HAL_INTERRUPT_DMA3:
    case CYGNUM_HAL_INTERRUPT_EXT0:
    case CYGNUM_HAL_INTERRUPT_EXT1:
    case CYGNUM_HAL_INTERRUPT_EXT2:
    case CYGNUM_HAL_INTERRUPT_EXT3:
    case CYGNUM_HAL_INTERRUPT_EXT4:
        CYGARC_MFDCR(DCR_EXIER, exier);
        exier &= ~exier_mask[vector];
        CYGARC_MTDCR(DCR_EXIER, exier);
        break;
    case CYGNUM_HAL_INTERRUPT_VAR_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr &= ~TCR_PIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    case CYGNUM_HAL_INTERRUPT_FIXED_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr &= ~TCR_FIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    case CYGNUM_HAL_INTERRUPT_WATCHDOG_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr &= ~TCR_WIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    default:
    }
}

externC void 
hal_ppc40x_interrupt_unmask(int vector)
{
    cyg_uint32 exier, tcr;

    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_CRITICAL:
    case CYGNUM_HAL_INTERRUPT_SERIAL_RCV:
    case CYGNUM_HAL_INTERRUPT_SERIAL_XMT:
    case CYGNUM_HAL_INTERRUPT_JTAG_RCV:
    case CYGNUM_HAL_INTERRUPT_JTAG_XMT:
    case CYGNUM_HAL_INTERRUPT_DMA0:
    case CYGNUM_HAL_INTERRUPT_DMA1:
    case CYGNUM_HAL_INTERRUPT_DMA2:
    case CYGNUM_HAL_INTERRUPT_DMA3:
    case CYGNUM_HAL_INTERRUPT_EXT0:
    case CYGNUM_HAL_INTERRUPT_EXT1:
    case CYGNUM_HAL_INTERRUPT_EXT2:
    case CYGNUM_HAL_INTERRUPT_EXT3:
    case CYGNUM_HAL_INTERRUPT_EXT4:
        CYGARC_MFDCR(DCR_EXIER, exier);
        exier |= exier_mask[vector];
        CYGARC_MTDCR(DCR_EXIER, exier);
        break;
    case CYGNUM_HAL_INTERRUPT_VAR_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr |= TCR_PIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    case CYGNUM_HAL_INTERRUPT_FIXED_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr |= TCR_FIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    case CYGNUM_HAL_INTERRUPT_WATCHDOG_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr |= TCR_WIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    default:
    }
}

externC void 
hal_ppc40x_interrupt_acknowledge(int vector)
{
    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_EXT0:
    case CYGNUM_HAL_INTERRUPT_EXT1:
    case CYGNUM_HAL_INTERRUPT_EXT2:
    case CYGNUM_HAL_INTERRUPT_EXT3:
    case CYGNUM_HAL_INTERRUPT_EXT4:
        CYGARC_MTDCR(DCR_EXISR, exier_mask[vector]);
        break;
    case CYGNUM_HAL_INTERRUPT_VAR_TIMER:
        CYGARC_MTSPR(SPR_TSR, TSR_PIS);  // clear & acknowledge interrupt
        break;
    case CYGNUM_HAL_INTERRUPT_FIXED_TIMER:
        CYGARC_MTSPR(SPR_TSR, TSR_FIS);  // clear & acknowledge interrupt
        break;
    case CYGNUM_HAL_INTERRUPT_WATCHDOG_TIMER:
        CYGARC_MTSPR(SPR_TSR, TSR_WIS);  // clear & acknowledge interrupt
        break;
    case CYGNUM_HAL_INTERRUPT_CRITICAL:
    case CYGNUM_HAL_INTERRUPT_SERIAL_RCV:
    case CYGNUM_HAL_INTERRUPT_SERIAL_XMT:
    case CYGNUM_HAL_INTERRUPT_JTAG_RCV:
    case CYGNUM_HAL_INTERRUPT_JTAG_XMT:
    case CYGNUM_HAL_INTERRUPT_DMA0:
    case CYGNUM_HAL_INTERRUPT_DMA1:
    case CYGNUM_HAL_INTERRUPT_DMA2:
    case CYGNUM_HAL_INTERRUPT_DMA3:
    default:
    }
}

// Note: These functions are only [well] defined for "external" interrupts
// which can be controlled via the EXIER register.
externC void 
hal_ppc40x_interrupt_configure(int vector, int level, int dir)
{
    cyg_uint32 mask, new_state, iocr;

    if ((vector >= CYGNUM_HAL_INTERRUPT_EXT0) &&
        (vector <= CYGNUM_HAL_INTERRUPT_EXT4)) {
        mask = 0x03 << (30 - ((vector - CYGNUM_HAL_INTERRUPT_EXT0)*2));
        new_state = 0x00;        
        if (level == 0) {
            // Edge triggered
            new_state = 0x02 | (dir & 0x01);  // Up/Down
        }
        new_state <<= (30 - ((vector - CYGNUM_HAL_INTERRUPT_EXT0)*2));
        CYGARC_MFDCR(DCR_IOCR, iocr);
        iocr = (iocr & ~mask) | new_state;
        CYGARC_MTDCR(DCR_IOCR, iocr);
    }
}

externC void 
hal_ppc40x_interrupt_set_level(int vector, int level)
{
}

// -------------------------------------------------------------------------
// EOF var_intr.c
