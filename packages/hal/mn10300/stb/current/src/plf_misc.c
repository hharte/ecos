//==========================================================================
//
//      plf_misc.c
//
//      HAL platform miscellaneous functions
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
// Copyright (C) 1998, 1999, 2000, 2001 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg
// Contributors: nickg, jlarmour, dhowells
// Date:         1999-01-21
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // architectural definitions

#include <cyg/hal/hal_intr.h>           // Interrupt handling

#include <cyg/hal/hal_cache.h>          // Cache handling

#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_misc.h>

#include <cyg/hal/plf_io.h>

/*------------------------------------------------------------------------*/
/* LED support                                                            */
cyg_uint8 cyg_hal_plf_led_val(CYG_WORD hexdig)
{
    static cyg_uint8 map[] = {
        0x81, // 0
        0xf3, // 1
        0x49, // 2
        0x61, // 3
        0x33, // 4
        0x25, // 5
        0x05, // 6
        0xf1, // 7
        0x01, // 8
        0x21, // 9
        0x11, // A
        0x07, // B
        0x8d, // C
        0x43, // D
        0x0d, // E
        0x1d  // F
    };
    return map[(hexdig & 0xF)];
}

/*------------------------------------------------------------------------*/

void hal_platform_init(void)
{
    HAL_WRITE_UINT8(HAL_LED_ADDRESS, cyg_hal_plf_led_val(8));

#if defined(CYG_HAL_STARTUP_ROM)
    // Note that the hardware seems to come up with the
    // caches containing random data. Hence they must be
    // invalidated before being enabled.
    // However, we only do this if we are in ROM. If we are
    // in RAM, then we leave the caches in the state chosen
    // by the ROM monitor. If we enable them when the monitor
    // is not expecting it, we can end up breaking things if the
    // monitor is not doing cache flushes.

    //HAL_ICACHE_INVALIDATE_ALL();    
    //HAL_ICACHE_ENABLE();
    //HAL_DCACHE_INVALIDATE_ALL();
    //HAL_DCACHE_ENABLE();
#endif

    // Set up eCos/ROM interfaces
    hal_if_init();
    
#if defined(CYGPKG_KERNEL)                      && \
    defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT)   && \
    defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs)
    {
        extern void patch_dbg_syscalls(void * vector);
        patch_dbg_syscalls( (void *)(&hal_virtual_vector_table[0]) );
    }
#endif    
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)
    {
        static void hal_ctrlc_isr_init(void);
        hal_ctrlc_isr_init();
    }
#endif    
}

/*------------------------------------------------------------------------*/
/* Control C ISR support                                                  */

#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) && \
    !defined(CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT)

#if CYGHWR_HAL_MN10300_AM33_STB_GDB_PORT == 0

// We use serial0 on AM33
#define SERIAL_CR       ((volatile cyg_uint16 *)0xd4002000)
#define SERIAL_ICR      ((volatile cyg_uint8 *) 0xd4002004)
#define SERIAL_TXR      ((volatile cyg_uint8 *) 0xd4002008)
#define SERIAL_RXR      ((volatile cyg_uint8 *) 0xd4002009)
#define SERIAL_SR       ((volatile cyg_uint16 *)0xd400200c)

// Timer 1 provided baud rate divisor
#define TIMER_MD       ((volatile cyg_uint8 *)0xd4003000)
#define TIMER_BR       ((volatile cyg_uint8 *)0xd4003010)
#define TIMER_CR       ((volatile cyg_uint8 *)0xd4003020)

#define SIO_LSTAT_TRDY  0x20
#define SIO_LSTAT_RRDY  0x10

#else

#error Unsupported GDB port

#endif

struct Hal_SavedRegisters *hal_saved_interrupt_state;

static void hal_ctrlc_isr_init(void)
{
//    cyg_uint16 cr;

//    HAL_READ_UINT16( SERIAL_CR, cr );
//    cr |= LCR_RXE;
//    HAL_WRITE_UINT16( SERIAL_CR, cr );
    HAL_INTERRUPT_SET_LEVEL( CYGHWR_HAL_GDB_PORT_VECTOR, 4 );
    HAL_INTERRUPT_UNMASK( CYGHWR_HAL_GDB_PORT_VECTOR ); 
}

cyg_uint32 hal_ctrlc_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    char c;
    cyg_uint16 sr;
    
    HAL_INTERRUPT_ACKNOWLEDGE( CYGHWR_HAL_GDB_PORT_VECTOR ); 

    HAL_READ_UINT16( SERIAL_SR, sr );

    if( sr & SIO_LSTAT_RRDY )
    {
        HAL_READ_UINT8( SERIAL_RXR, c);

        if( cyg_hal_is_break( &c , 1 ) )
            cyg_hal_user_break( (CYG_ADDRWORD *)hal_saved_interrupt_state );

        
    }
    return 1;
}

#endif

/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
