//=============================================================================
//
//      linux_misc.c
//
//      Misc code for the Linux HAL
//
//=============================================================================
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
// October 31, 1998.
// 
// The Initial Developer of the Original Code is Cygnus.  Portions created
// by Cygnus are Copyright (C) 1998,1999 Cygnus Solutions.  All Rights Reserved.
// -------------------------------------------
//
//####COPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   asl
// Contributors:asl
// Date:        1999-09-23
// Purpose:     Misc functions for the Linux HAL
// Description: Misc functions for the Linux HAL
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>         // base types

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
#endif

// This in not correct, but it will do.
externC int cyg_hal_sys__newselect(int,int,int,int,int);

// Here we define an action to do in the idle thread. For the
// synthetic architecture it makes no sence to spin eating processor
// time that other processes could make use of. Instead we call
// select. The itimer will still go off and kick the schedular back
// into life so giving up an escape path from the select. There is one
// catch 22. This only works if we are using the real time clock for
// itimer, not the virtual clock.  The virtual clock works on time the
// process actually uses. When its sitting in the select it does not
// use any time, so the timer does not expire!  Also we don't do a
// select when the THREAD_YIELD optin is in use since we want the
// yield to be called regularly.

void
hal_idle_thread_action(cyg_uint32 loop_count)
{
#ifndef CYGIMP_IDLE_THREAD_YIELD
#ifdef CYGSEM_HAL_I386_LINUX_REAL_TIME
    cyg_hal_sys__newselect(0,0,0,0,0);
#endif
#endif
}
