#ifndef CYGONCE_HAL_PLF_IO_H
#define CYGONCE_HAL_PLF_IO_H
//=============================================================================
//
//      plf_io.h
//
//      Embedded Artists LPC2468 OEM board specific registers
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2008 eCosCentric Limited
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Uwe Kindler
// Contributors: 
// Date:         2008-07-06
// Purpose:      EA LPC2468 oem board specific registers
// Description:
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================
// On-chip device base addresses


//----------------------------------------------------------------------
// The platform needs this initialization during the
// hal_hardware_init() function in the varient HAL.
#ifndef __ASSEMBLER__
extern void hal_plf_hardware_init(void);
#define HAL_PLF_HARDWARE_INIT() \
    hal_plf_hardware_init()

//-----------------------------------------------------------------------------
// LPX24xx variant specific initialisation of CAN channels
// This function configures the pin functions for CAN use
//-----------------------------------------------------------------------------            
#ifdef CYGPKG_DEVS_CAN_LPC2XXX
externC void hal_lpc_can_init(cyg_uint8 can_chan_no);            
#define HAL_LPC2XXX_INIT_CAN(_can_chan_no_) hal_lpc_can_init(_can_chan_no_)
#endif // CYGPKG_DEVS_CAN_LPC2XXX   

#endif  //__ASSEMBLER__ 

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_HAL_PLF_IO_H

