#ifndef CYGONCE_HAL_PLATFORM_INTS_H
#define CYGONCE_HAL_PLATFORM_INTS_H
//==========================================================================
//
//      hal_platform_ints.h
//
//      HAL Interrupt and clock support
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
// Author(s):    gthomas
// Contributors: gthomas, dmoseley
// Date:         2000-05-08
// Purpose:      Define Interrupt support
// Description:  The interrupt details for the SA1100/Multimedia are defined here.
// Usage:
//               #include <cyg/hal/hal_platform_ints.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#define CYGNUM_HAL_INTERRUPT_GPIO0   0
#define CYGNUM_HAL_INTERRUPT_GPIO1   1
#define CYGNUM_HAL_INTERRUPT_GPIO2   2
#define CYGNUM_HAL_INTERRUPT_GPIO3   3
#define CYGNUM_HAL_INTERRUPT_GPIO4   4
#define CYGNUM_HAL_INTERRUPT_GPIO5   5
#define CYGNUM_HAL_INTERRUPT_GPIO6   6
#define CYGNUM_HAL_INTERRUPT_GPIO7   7
#define CYGNUM_HAL_INTERRUPT_GPIO8   8
#define CYGNUM_HAL_INTERRUPT_GPIO9   9
#define CYGNUM_HAL_INTERRUPT_GPIO10  10
#define CYGNUM_HAL_INTERRUPT_GPIO    11  // Don't use directly
#define CYGNUM_HAL_INTERRUPT_LCD     12
#define CYGNUM_HAL_INTERRUPT_UDC     13
#define CYGNUM_HAL_INTERRUPT_UART1   15
#define CYGNUM_HAL_INTERRUPT_UART2   16
#define CYGNUM_HAL_INTERRUPT_UART3   17
#define CYGNUM_HAL_INTERRUPT_MCP     18
#define CYGNUM_HAL_INTERRUPT_SSP     19
#define CYGNUM_HAL_INTERRUPT_TIMER0  26
#define CYGNUM_HAL_INTERRUPT_TIMER1  27
#define CYGNUM_HAL_INTERRUPT_TIMER2  28
#define CYGNUM_HAL_INTERRUPT_TIMER3  29
#define CYGNUM_HAL_INTERRUPT_HZ      30
#define CYGNUM_HAL_INTERRUPT_ALARM   31

// GPIO bits 31..11 can generate interrupts as well, but they all
// end up clumped into interrupt signal #11.  Using the symbols
// below allow for detection of these separately.

#define CYGNUM_HAL_INTERRUPT_GPIO11  (32+11)
#define CYGNUM_HAL_INTERRUPT_GPIO12  (32+12)
#define CYGNUM_HAL_INTERRUPT_GPIO13  (32+13)
#define CYGNUM_HAL_INTERRUPT_GPIO14  (32+14)
#define CYGNUM_HAL_INTERRUPT_GPIO15  (32+15)
#define CYGNUM_HAL_INTERRUPT_GPIO16  (32+16)
#define CYGNUM_HAL_INTERRUPT_GPIO17  (32+17)
#define CYGNUM_HAL_INTERRUPT_GPIO18  (32+18)
#define CYGNUM_HAL_INTERRUPT_GPIO19  (32+19)
#define CYGNUM_HAL_INTERRUPT_GPIO20  (32+20)
#define CYGNUM_HAL_INTERRUPT_GPIO21  (32+21)
#define CYGNUM_HAL_INTERRUPT_GPIO22  (32+22)
#define CYGNUM_HAL_INTERRUPT_GPIO23  (32+23)
#define CYGNUM_HAL_INTERRUPT_GPIO24  (32+24)
#define CYGNUM_HAL_INTERRUPT_GPIO25  (32+25)
#define CYGNUM_HAL_INTERRUPT_GPIO26  (32+26)
#define CYGNUM_HAL_INTERRUPT_GPIO27  (32+27)

#define CYGNUM_HAL_INTERRUPT_NONE    -1

#define CYGNUM_HAL_ISR_MIN            0
#define CYGNUM_HAL_ISR_MAX           (27+32)

#define CYGNUM_HAL_ISR_COUNT            (CYGNUM_HAL_ISR_MAX+1)

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC        CYGNUM_HAL_INTERRUPT_TIMER0

#endif // CYGONCE_HAL_PLATFORM_INTS_H
