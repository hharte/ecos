//==========================================================================
//
//      flash.h
//
//      Flash programming - external interfaces
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

#ifndef _IO_FLASH_H_
#define _IO_FLASH_H_

#include <pkgconf/io_flash.h>
#include <cyg/hal/hal_cache.h>

#define FLASH_MIN_WORKSPACE 0x10000  // Space used by FLASH code

externC int flash_init(void *work_space, int work_space_length);
externC int flash_erase(void *base, int len, void **err_address);
externC int flash_program(void *flash_base, void *ram_base, int len, void **err_address);
externC void flash_dev_query(void *data);
#if 0 < CYGHWR_IO_FLASH_BLOCK_LOCKING // This is an *interface*
externC int flash_lock(void *base, int len, void **err_address);
externC int flash_unlock(void *base, int len, void **err_address);
#endif
externC int flash_verify_addr(void *base);
externC int flash_get_limits(void *base, void **start, void **end);
externC int flash_get_block_info(int *block_size, int *blocks);
externC bool flash_code_overlaps(void *start, void *end);
externC char *flash_errmsg(int err);

externC int printf(char* fmt, ...);

#define FLASH_ERR_OK              0x00  // No error - operation complete
#define FLASH_ERR_INVALID         0x01  // Invalid FLASH address
#define FLASH_ERR_ERASE           0x02  // Error trying to erase
#define FLASH_ERR_LOCK            0x03  // Error trying to lock/unlock
#define FLASH_ERR_PROGRAM         0x04  // Error trying to program
#define FLASH_ERR_PROTOCOL        0x05  // Generic error
#define FLASH_ERR_PROTECT         0x06  // Device/region is write-protected
#define FLASH_ERR_NOT_INIT        0x07  // FLASH info not yet initialized
#define FLASH_ERR_HWR             0x08  // Hardware (configuration?) problem
#define FLASH_ERR_ERASE_SUSPEND   0x09  // Device is in erase suspend mode
#define FLASH_ERR_PROGRAM_SUSPEND 0x0a  // Device is in in program suspend mode
#define FLASH_ERR_DRV_VERIFY      0x0b  // Driver failed to verify data

#ifdef _FLASH_PRIVATE_

struct flash_info {
    void *work_space;
    int   work_space_size;
    int   block_size;   // Assuming fixed size "blocks"
    int   blocks;       // Number of blocks
    unsigned long block_mask;
    void *start, *end;  // Address range
    int   init;
};

extern struct flash_info flash_info;
extern int  flash_hwr_init(void);
extern int  flash_hwr_map_error(int err);


//---------------------------------------------------------------------------
//Execution of relocated code must be done inside a
//HAL_FLASH_CACHES_OFF/HAL_FLASH_CACHES_ON region - disabling the
//cache on unified cache systems is necessary to prevent burst access
//to the flash area being programmed. With Harvard style caches, only
//the data cache needs to be disabled, but the instruction cache must
//still be invalidated to ensure the relocated code is properly
//fetched from memory.

// Targets may provide alternative implementations for these macros in
// the hal_cache.h (or var/plf) files.

// The first part below is the optimal implementation, but it does not
// always work. The second part is a more safe implementation that has
// been tested to work on some targets - it may not be suitable for
// targets that would do burst access to the flash (data cache needs
// disabling as well).

// NOTE: Do _not_ change any of the below macros without checking that
//       the changed code still works on _all_ platforms that rely on these
//       macros. There is no such thing as logical and correct when dealing
//       with different cache and IO models, so _do not_ mess with this code
//       unless you test it properly afterwards.

#ifndef HAL_FLASH_CACHES_OFF

#ifdef HAL_FLASH_CACHES_WANT_OPTIMAL // Optimal implementation

#ifdef HAL_CACHE_UNIFIED

// Note: the ucache code has not been tested yet on any target.
#define HAL_FLASH_CACHES_OFF(_d_, _i_)          \
    CYG_MACRO_START                             \
    _i_ = 0; /* avoids warning */               \
    HAL_UCACHE_IS_ENABLED(_d_);                 \
    HAL_UCACHE_SYNC();                          \
    HAL_UCACHE_INVALIDATE_ALL();                \
    HAL_UCACHE_DISABLE();                       \
    CYG_MACRO_END

#define HAL_FLASH_CACHES_ON(_d_, _i_)           \
    CYG_MACRO_START                             \
    if (_d_) HAL_UCACHE_ENABLE();               \
    CYG_MACRO_END

#else  // HAL_CACHE_UNIFIED

#define HAL_FLASH_CACHES_OFF(_d_, _i_)          \
    CYG_MACRO_START                             \
    _i_ = 0; /* avoids warning */               \
    HAL_DCACHE_IS_ENABLED(_d_);                 \
    HAL_DCACHE_SYNC();                          \
    HAL_DCACHE_INVALIDATE_ALL();                \
    HAL_DCACHE_DISABLE();                       \
    HAL_ICACHE_INVALIDATE_ALL();                \
    CYG_MACRO_END

#define HAL_FLASH_CACHES_ON(_d_, _i_)           \
    CYG_MACRO_START                             \
    if (_d_) HAL_DCACHE_ENABLE();               \
    CYG_MACRO_END

#endif // HAL_CACHE_UNIFIED

#else  // HAL_FLASH_CACHES_WANT_OPTIMAL

// Note: This implementation is broken as it will always enable the i-cache
//       even if it was not enabled before. It also doesn't work if the
//       target uses burst access to flash since the d-cache is left enabled.
//       However, this does not mean you can change this code! Leave it as
//       is - if you want a different implementation, provide it in the
//       arch/var/platform cache header file.

#define HAL_FLASH_CACHES_OFF(_d_, _i_)          \
    _d_ = 0; /* avoids warning */               \
    _i_ = 0; /* avoids warning */               \
    HAL_DCACHE_SYNC();                          \
    HAL_ICACHE_DISABLE();

#define HAL_FLASH_CACHES_ON(_d_, _i_)           \
    HAL_ICACHE_ENABLE();

#endif  // HAL_FLASH_CACHES_WANT_OPTIMAL

#endif  // HAL_FLASH_CACHES_OFF

#endif  // _FLASH_PRIVATE_

#endif  // _IO_FLASH_H_
