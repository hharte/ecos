//==========================================================================
//
//      entry.c
//
//      Entry code for Linux synthetic target.
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
// Author(s):   proven
// Contributors:proven, jskov
// Date:        1999-01-06
// Purpose:     Entry point for Linux synthetic target.
//
//####DESCRIPTIONEND####
//
//=========================================================================

#include <cyg/infra/cyg_type.h>

externC void cyg_hal_isr_init( void );
externC void cyg_hal_invoke_constructors( void );
externC void cyg_start( void );
externC void cyg_hal_hardware_init( void );

void _linux_entry( void )
{
    // Should get argc argv
    cyg_hal_hardware_init();
    cyg_hal_isr_init();
    cyg_hal_invoke_constructors();
    cyg_start();
}

//-----------------------------------------------------------------------------
// End of entry.c
