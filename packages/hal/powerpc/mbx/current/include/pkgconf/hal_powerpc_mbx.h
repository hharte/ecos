#ifndef CYGONCE_PKGCONF_HAL_POWERPC_MBX_H
#define CYGONCE_PKGCONF_HAL_POWERPC_MBX_H
//=============================================================================
//
//      pkgconf/hal_powerpc_mbx.h
//
//      HAL configuration file
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
// September 30, 1998.
// 
// The Initial Developer of the Original Code is Cygnus.  Portions created
// by Cygnus are Copyright (C) 1998,1999 Cygnus Solutions.  All Rights Reserved.
// -------------------------------------------
//
//####COPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           hmt
// Contributors:        hmt
// Date:                1999-06-08
// Purpose:             To allow the user to edit HAL configuration options.
// Description:
//
//####DESCRIPTIONEND####
//
//=============================================================================


/* ---------------------------------------------------------------------
   {{CFG_DATA

   cdl_package CYGPKG_HAL_POWERPC_MBX {
       display  "Motorola MBX PowerPC evaluation board"
       type     radio
       parent   CYGPKG_HAL_POWERPC
       requires CYGPKG_HAL_QUICC
       platform mbx
       description "
           The MBX HAL package provides the support needed to run
           eCos on a Motorola MBX board equipped with a PowerPC processor."
   }

   cdl_option CYGHWR_HAL_POWERPC_MBX_STARTUP {
       display          "Startup type"
       parent           CYGPKG_HAL_POWERPC_MBX
       #type             count
       type             enum
       legal_values     ram rom
       startup
       description      "
           When targetting the MBX board for RAM bootstrap it is expected 
           that the image will be downloaded to eCos GDB stub boot ROMs
           using powerpc-eabi-gdb."
   }

   cdl_option CYGHWR_HAL_POWERPC_MBX_BOARD_SPEED {
       display          "Development board clock speed (MHz)"
       parent           CYGPKG_HAL_POWERPC_MBX
       #type             count
       type             enum
       legal_values     40 50
       description      "
           MBX Development Boards have various system clock speeds
           depending on the processor fitted.  Select the clock speed
           appropriate for your board so that the system can set the serial
           baud rate correctly, amongst other things."
       # This is an enum because the system selects code en masse rather
       # than doing arithmetic with the value; making it a count and
       # setting it to 30, for example, would not work.
   }

   }}CFG_DATA */

#define CYGHWR_HAL_POWERPC_MBX_STARTUP       ram

#define CYGHWR_HAL_POWERPC_MBX_BOARD_SPEED   50

// Real-time clock/counter specifics

// Period is busclock/16/100.
#define CYGNUM_HAL_RTC_NUMERATOR     1000000000
#define CYGNUM_HAL_RTC_DENOMINATOR   100

#if 40 == CYGHWR_HAL_POWERPC_MBX_BOARD_SPEED
# define CYGNUM_HAL_RTC_PERIOD        25000
#elif 50 == CYGHWR_HAL_POWERPC_MBX_BOARD_SPEED
# define CYGNUM_HAL_RTC_PERIOD        31250
#else
# error Bad Board speed defined: see CYGBLD_HAL_PLATFORM_H
#endif

// 33.3333MHz from FADS port:
//#define CYGNUM_HAL_RTC_PERIOD        20833

/* -------------------------------------------------------------------*/
#endif  /* CYGONCE_PKGCONF_HAL_POWERPC_MBX_H */
/* EOF hal_powerpc_mbx.h */
