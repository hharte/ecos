//==========================================================================
//
//      flash_erase_block.c
//
//      Flash programming
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
// Contributors: gthomas
// Date:         2000-07-14
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

//
// CAUTION!  This code must be copied to RAM before execution.  Therefore,
// it must not contain any code which might be position dependent!
//

int flash_erase_block(volatile unsigned long *block)
{
    volatile unsigned long *ROM, *sb;
    unsigned long stat;
    int timeout = 50000;
    int cache_on;
    int len, block_size;

    HAL_DCACHE_IS_ENABLED(cache_on);
    if (cache_on) {
        HAL_DCACHE_SYNC();
        HAL_DCACHE_DISABLE();
    }

    ROM = (volatile unsigned long *)((unsigned long)block & 0xFF800000);

    // Clear any error conditions
    ROM[0] = FLASH_Clear_Status;

    len = FLASH_BLOCK_SIZE;
    if (((unsigned long)block - (unsigned long)ROM) < FLASH_BLOCK_SIZE) {
        block_size = FLASH_BOOT_BLOCK_SIZE;  // First 8 blocks are only 8Kx2 each
    } else {
        block_size = FLASH_BLOCK_SIZE;
    }
    sb = block;
    while (len > 0) {
        // Erase block
        ROM[0] = FLASH_Block_Erase;
        *block = FLASH_Confirm;
        timeout = 5000000;
        while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
            if (--timeout == 0) break;
        }

        len -= block_size;
        block += block_size / sizeof(*block);
    }

    // Restore ROM to "normal" mode
    ROM[0] = FLASH_Reset;

    // If an error was reported, see if the block erased anyway
    if (stat & 0x007E007E) {
        len = FLASH_BLOCK_SIZE;
        block = sb;
        while (len > 0) {
            if (*block++ != 0xFFFFFFFF) break;
            len -= sizeof(*block);
        }
        if (len == 0) stat = 0;
    }

    if (cache_on) {
        HAL_DCACHE_ENABLE();
    }

    return stat;
}
