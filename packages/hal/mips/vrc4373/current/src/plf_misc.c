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
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg
// Contributors: nickg, jlarmour
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

#include <cyg/hal/plf_z8530.h>

/*------------------------------------------------------------------------*/

void hal_platform_init(void)
{
#if defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT)      && \
    (defined(CYGSEM_HAL_USE_ROM_MONITOR_CygMon)    || \
     defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs))

    {
        extern CYG_ADDRESS hal_virtual_vector_table[32];
        void patch_dbg_syscalls( void * );
        patch_dbg_syscalls( (void *)(&hal_virtual_vector_table[0]) );
    }
#endif
#if defined(CYGDBG_HAL_MIPS_DEBUG_GDB_CTRLC_SUPPORT)

    {
        static void hal_ctrlc_isr_init(void);
        hal_ctrlc_isr_init();
    }
#endif    
    
}

/*------------------------------------------------------------------------*/
/* Functions to support the detection and execution of a user provoked    */
/* program break. These are usually called from interrupt routines.       */

cyg_bool cyg_hal_is_break(char *buf, int size)
{
    while( size )
        if( buf[--size] == 0x03 ) return true;

    return false;
}

void cyg_hal_user_break( CYG_ADDRWORD *regs )
{
#if defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs)
    // The following code should be at the very start of this function so
    // that it can access the RA register before it is saved and reused.
    register CYG_WORD32 ra;
    asm volatile ( "move %0,$31;" : "=r" (ra) );

        {
            extern CYG_ADDRESS hal_virtual_vector_table[64];        
            typedef void install_bpt_fn(void *epc);
            CYG_WORD32 pc;
            HAL_SavedRegisters *sreg = (HAL_SavedRegisters *)regs;
            install_bpt_fn *ibp = (install_bpt_fn *)hal_virtual_vector_table[35];

            if( regs == NULL ) pc = ra;
            else pc = sreg->pc;

            if( ibp != NULL ) ibp((void *)pc);
        }
    
#elif defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)

        {
            extern void breakpoint(void);
            breakpoint();
        }
    
#else

        HAL_BREAKPOINT(breakinst);

#endif

}

/*------------------------------------------------------------------------*/
/* Control C ISR support                                                  */

#if defined(CYGDBG_HAL_MIPS_DEBUG_GDB_CTRLC_SUPPORT)

#if CYGHWR_HAL_MIPS_VR4300_VRC4373_GDB_PORT == 0
#define DUART_CHAN      DUART_A
#else
#define DUART_CHAN      DUART_B
#endif

struct Hal_SavedRegisters *hal_saved_interrupt_state;

static void hal_ctrlc_isr_init(void)
{
    HAL_DUART_WRITE_CR( DUART_CHAN, 1, 0x10 );
    HAL_DUART_WRITE_CR( DUART_CHAN, 9, 0x0a );    
    HAL_INTERRUPT_SET_LEVEL( CYGHWR_HAL_GDB_PORT_VECTOR, 0 );
    HAL_INTERRUPT_UNMASK( CYGHWR_HAL_GDB_PORT_VECTOR );
}

cyg_uint32 hal_ctrlc_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{

    char c;
    cyg_uint8 rr0;

    HAL_INTERRUPT_ACKNOWLEDGE( CYGHWR_HAL_GDB_PORT_VECTOR ); 
    HAL_DUART_READ_CR(DUART_CHAN, 0, rr0 );

    // The following return value prevents a spurious interrupt report.
    // That is what should happen, but for some reason we get the odd extra
    // character when running in the test farm. 
    
    if ( (rr0 & 0x01) == 0 ) return 2;

    HAL_DUART_READ_RR( DUART_CHAN, c );

    if( cyg_hal_is_break( &c , 1 ) )
        cyg_hal_user_break( (CYG_ADDRWORD *)hal_saved_interrupt_state );

    return 2;
}

#endif

/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
