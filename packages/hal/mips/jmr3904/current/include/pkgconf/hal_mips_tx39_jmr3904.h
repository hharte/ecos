#ifndef CYGONCE_PKGCONF_HAL_TX39_JMR3904_H
#define CYGONCE_PKGCONF_HAL_TX39_JMR3904_H
// ====================================================================
//
//      pkgconf/hal_tx39_jmr3904.h
//
//      HAL configuration file
//
// ====================================================================
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
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           bartv
// Contributors:        bartv
// Date:                1998-09-02      
// Purpose:             To allow the user to edit HAL configuration options.
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================


/* ---------------------------------------------------------------------
   {{CFG_DATA

   cdl_package CYGPKG_HAL_MIPS_TX39_JMR3904 {
       display  "JMR3904 evaluation board"
       type     radio
       parent   CYGPKG_HAL_MIPS
       requires CYGPKG_HAL_MIPS_TX39
       platform jmr3904
       description "
           The JMR3904 HAL package should be used when targetting the
           actual hardware. The same package can also be used when
           running on the full simulator, since this provides an
           accurate simulation of the hardware including I/O devices.
           To use the simulator in this mode the command
           `target sim --board=jmr3904' should be used from inside gdb."
   }
   
   cdl_option CYGHWR_HAL_MIPS_TX39_JMR3904_STARTUP {
       display          "Startup type"
       parent           CYGPKG_HAL_MIPS_TX39_JMR3904
       #type             count
       type             enum
       legal_values     ram rom
       startup
       description      "
           When targetting the JMR3904 board it is possible to build
           the system for either RAM bootstrap or ROM bootstrap. The
           former generally requires that the board is equipped with
           ROMs containing the Cygmon ROM monitor or equivalent
           software that allows gdb to download the eCos application
           on to the board. The latter typically requires that the
           eCos application be blown into EPROMs or equivalent
           technology."
   }
                
   }}CFG_DATA */

#define CYGHWR_HAL_MIPS_TX39_JMR3904_STARTUP         ram

/* -------------------------------------------------------------------*/
/* Manually edited configuration option, these should only be touched */
/* by experts who understand the consequences.                        */

/* DRAM Configuration */
/* for 16MByte (4MBit (x4bit) x 8)           0x08024030               */
/* for 4MByte (1MBit (x4bit) x 8)            0x08013020               */
/* for 8MByte (1MBit (x4bit) x 8 x 2 banks)  0x08013020               */

#define CYGHWR_HAL_MIPS_TX39_JMR3904_DRAM_CONFIG_INIT  0x08024030  // 16MByte
//#define CYGHWR_HAL_MIPS_TX39_JMR3904_DRAM_CONFIG_INIT 0x08013020 // 4 or 8MByte

/* -------------------------------------------------------------------*/
/* Bus Timeout Detection                                              */


//#define CYGHWR_HAL_MIPS_TX39_JMR3904_ENABLE_TOE

/* -------------------------------------------------------------------*/
/* Clock frequency initialization                                     */

#include <pkgconf/hal_mips.h>  // Make sure clock frequency is defined

// Real-time clock/counter specifics

#define CYGNUM_HAL_RTC_NUMERATOR     1000000000
#define CYGNUM_HAL_RTC_DENOMINATOR   100
#if (CYGHWR_HAL_MIPS_CPU_FREQ == 50)
# define CYGNUM_HAL_RTC_PERIOD       15360
#elif (CYGHWR_HAL_MIPS_CPU_FREQ == 66)
# define CYGNUM_HAL_RTC_PERIOD       20736
#else
#error Unsupported clock frequency
#endif

/* -------------------------------------------------------------------*/
#endif  /* CYGONCE_PKGCONF_HAL_TX39_JMR3904_H */
/* EOF hal_tx39_jmr3904.h */
