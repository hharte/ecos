#ifndef CYGONCE_KERNEL_TESTS_STACKMON_H
#define CYGONCE_KERNEL_TESTS_STACKMON_H

/*=================================================================
//
//        stackmon.h
//
//        Auxiliary test header file
//
//==========================================================================
//####COPYRIGHTBEGIN####
//
// -------------------------------------------
// The contents of this file are subject to the Cygnus eCos Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://sourceware.cygnus.com/ecos
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the
// License for the specific language governing rights and limitations under
// the License.
// 
// The Original Code is eCos - Embedded Cygnus Operating System, released
// September 30, 1998.
// 
// The Initial Developer of the Original Code is Cygnus.  Portions created
// by Cygnus are Copyright (C) 1998,1999 Cygnus Solutions.  All Rights Reserved.
// -------------------------------------------
//
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     hmt
// Contributors:  hmt
// Date:          1999-05-20
// Description:
//     Defines some convenience functions for stack use output.
//
//####DESCRIPTIONEND####
*/

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/thread.inl>
#endif

#ifndef STACKMON_PRINTF
externC void diag_printf(const char *, ...);
#define STACKMON_PRINTF diag_printf
#endif

// ------------------------------------------------------------------------
// Utility function for actually counting a stack

inline void cyg_test_size_a_stack( char *comment, char *format,
                                   char *base, char *top )
{
    register char *p;
    for ( p = base; p < top; p++ )
        if ( *p )
            break;
    STACKMON_PRINTF( format, comment, top - p, top - base );
}

// ------------------------------------------------------------------------

inline void cyg_test_dump_stack_stats( char *comment,
                                       char *base, char *top )
{
    cyg_test_size_a_stack( comment, "%31s : stack used %5d size %5d\n",
                           base, top );
}

// ------------------------------------------------------------------------

inline void cyg_test_dump_thread_stack_stats( char *comment,
                                              Cyg_Thread *p )
{
    char *base, *top;
    base = (char *)p->get_stack_base();
    top =   base + p->get_stack_size();
    cyg_test_dump_stack_stats( comment, base, top );
}

// ------------------------------------------------------------------------
// Print out size of idle thread stack usage since start-of-time.  Only
// meaningful if there is a scheduler.

inline void cyg_test_dump_idlethread_stack_stats( char *comment )
{
#ifdef CYGPKG_KERNEL
    extern Cyg_Thread idle_thread;
    // idle thread is not really a plain CygThread; danger.
    char *ibase  = (char *)idle_thread.get_stack_base();
    char *istack = ibase + idle_thread.get_stack_size();
    cyg_test_size_a_stack( comment,
              "%20s : Idlethread stack used %5d size %5d\n",
              ibase, istack );
#endif
}

// ------------------------------------------------------------------------
// Print out size of interrupt stack usage since start-of-time or since it
// was last cleared.  NB on some architectures and configurations, the
// interrupt stack is the same as the bootup stack, so clear it in the
// first first thread to execute.  Clearing it before scheduler start would
// be fatal!

inline void cyg_test_dump_interrupt_stack_stats( char *comment )
{
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
#ifdef HAL_INTERRUPT_STACK_BASE
#ifdef HAL_INTERRUPT_STACK_TOP
    extern char HAL_INTERRUPT_STACK_BASE[];
    extern char HAL_INTERRUPT_STACK_TOP[];
    cyg_test_size_a_stack( comment,
              "%20s :  Interrupt stack used %5d size %5d\n",
              HAL_INTERRUPT_STACK_BASE, HAL_INTERRUPT_STACK_TOP );
#endif
#endif
#endif
}

// Clear interrupt stack to reset stats - only after sched has started.
inline void cyg_test_clear_interrupt_stack( void )
{
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
#ifdef HAL_INTERRUPT_STACK_BASE
#ifdef HAL_INTERRUPT_STACK_TOP
    cyg_uint32  old_intr;
    extern char HAL_INTERRUPT_STACK_BASE[];
    extern char HAL_INTERRUPT_STACK_TOP[];
    register char *p;
    HAL_DISABLE_INTERRUPTS(old_intr);
    for ( p = HAL_INTERRUPT_STACK_BASE; p < HAL_INTERRUPT_STACK_TOP; p++ )
        *p = 0;                         // zero it for checking later
    HAL_RESTORE_INTERRUPTS(old_intr);
#endif
#endif
#endif
}

// ------------------------------------------------------------------------

#endif // ifndef CYGONCE_KERNEL_TESTS_STACKMON_H

// EOF stackmon.h
