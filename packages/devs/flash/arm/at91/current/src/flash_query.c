//==========================================================================
//
//      flash_query.c
//
//      Flash programming - query device
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2001-07-17
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include "flash.h"

#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include CYGHWR_MEMORY_LAYOUT_H

//
// CAUTION!  This code must be copied to RAM before execution.  Therefore,
// it must not contain any code which might be position dependent!
//

#define CNT 2000*1000*10  // At least 20ms

int
flash_query(unsigned short *data)
{
    volatile unsigned short *ROM;
    int cnt, cache_on;

    HAL_DCACHE_IS_ENABLED(cache_on);
    if (cache_on) {
        HAL_DCACHE_SYNC();
        HAL_DCACHE_DISABLE();
    }

    ROM = (volatile unsigned short *)0x01010000;

    // Send lead-in sequence
    ROM[FLASH_Key_Addr0] = FLASH_Key0;
    ROM[FLASH_Key_Addr1] = FLASH_Key1;

    // Send 'identify' command
    ROM[FLASH_Key_Addr0] = FLASH_Read_ID;

    // Need a delay of 20ms!
    for (cnt = 0;  cnt < 50000;  cnt++) ;

    data[0] = ROM[0];  // Manufacturer code
    data[1] = ROM[1];  // Device identifier

    // Send lead-in sequence
    ROM[FLASH_Key_Addr0] = FLASH_Key0;
    ROM[FLASH_Key_Addr1] = FLASH_Key1;

    // Send 'reset' command
    ROM[FLASH_Key_Addr0] = FLASH_Reset;

    // Need a delay of 20ms!
    for (cnt = 0;  cnt < 50000;  cnt++) ;

    if (cache_on) {
        HAL_DCACHE_ENABLE();
    }

    return 0;
}
