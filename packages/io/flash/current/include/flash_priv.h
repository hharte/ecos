//==========================================================================
//
//      flash.h
//
//      Flash programming - external interfaces
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
// copyright (C) 2004 Andrew Lunn
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
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, Andrew Lunn
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================
#ifndef _IO_FLASH_PRIV_H_
#define _IO_FLASH_PRIV_H_

#include <pkgconf/system.h>
#include <pkgconf/io_flash.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_tables.h>

// Forward reference of the device structure
struct cyg_flash_dev;

// Structure of pointers to functions in the device driver
struct cyg_flash_dev_funs {
  int (*flash_init) (struct cyg_flash_dev *dev);
  size_t (*flash_query) (struct cyg_flash_dev *dev, void * data, 
                         const size_t len);
  int (*flash_erase_block) (struct cyg_flash_dev *dev, 
                            const cyg_flashaddr_t block_base);
  int (*flash_program) (struct cyg_flash_dev *dev, 
                        cyg_flashaddr_t base, 
                        const void* data, const size_t len);
  int (*flash_read) (struct cyg_flash_dev *dev, 
                     const cyg_flashaddr_t base, 
                     void* data, const size_t len);
  int (*flash_hwr_map_error) (struct cyg_flash_dev *dev, int err);
  int (*flash_block_lock) (struct cyg_flash_dev *dev, 
                           const cyg_flashaddr_t block_base);
  int (*flash_block_unlock) (struct cyg_flash_dev *dev, 
                             const cyg_flashaddr_t block_base);
};

// Structure each device places in the HAL table
struct cyg_flash_dev {
  struct cyg_flash_dev_funs   *funs;           // Function pointers
  cyg_flashaddr_t             start;           // First address
  cyg_flashaddr_t             end;             // Last address
  cyg_uint32                  num_block_infos; // Number of entries
  cyg_flash_block_info_t      *block_info;     // Info about one block size

  void                        *priv;           // Devices private data
  void                        *config;         // Configuration info

// The following are only written to by the FLASH IO layer.
  cyg_flash_printf            *pf;             // Pointer to diagnostic printf
  bool                        init;            // Device has been initialised
#ifdef CYGPKG_KERNEL
  cyg_mutex_t                 mutex;           // Mutex for thread safeness
#endif
#if (CYGHWR_IO_FLASH_DEVICE > 1)    
  struct cyg_flash_dev        *next;           // Pointer to next device
#endif    
} CYG_HAL_TABLE_TYPE;

#define CYG_FLASH_FUNS(funs,init,query,erase,prog,read,map,lock,unlock)\
struct cyg_flash_dev_funs funs = \
  {                                     \
    init,                               \
    query,                              \
    erase,                              \
    prog,                               \
    read,                               \
    map,                                \
    lock,                               \
    unlock                              \
  };

// We assume HAL tables are placed into RAM.
#define CYG_FLASH_DRIVER(name, _funs, _config, _start, _priv_size)    \
static char priv_ ## name [_priv_size];                               \
struct cyg_flash_dev name CYG_HAL_TABLE_ENTRY(cyg_flashdev) = {       \
   .funs = _funs,                                                     \
   .config = _config,                                                 \
   .priv = priv_ ## name,                                             \
   .start = _start,                                                   \
};

#ifdef CYGHWR_IO_FLASH_DEVICE_LEGACY
struct flash_info {
  int   block_size;   // Assuming fixed size "blocks"
  int   blocks;       // Number of blocks
  int   buffer_size;  // Size of write buffer (only defined for some devices)
  unsigned long block_mask;
  void *start, *end;  // Address range
  int   init;
  cyg_flash_printf *pf;
};

externC struct flash_info flash_info;
externC int  flash_hwr_init(void);
externC int  flash_hwr_map_error(int err);
externC void flash_dev_query(void *data);
#endif // CYGHWR_IO_FLASH_DEVICE_LEGACY
// 
// Some FLASH devices may require additional support, e.g. to turn on
// appropriate voltage drivers, before any operation.
//
#ifdef  CYGIMP_FLASH_ENABLE
#define FLASH_Enable CYGIMP_FLASH_ENABLE
extern void CYGIMP_FLASH_ENABLE(void *, void *);
#else
#define FLASH_Enable(_start_, _end_)
#endif
#ifdef  CYGIMP_FLASH_DISABLE
#define FLASH_Disable CYGIMP_FLASH_DISABLE
extern void CYGIMP_FLASH_DISABLE(void *, void *);
#else
#define FLASH_Disable(_start_, _end_)
#endif

//
// Some platforms have a DIP switch or jumper that tells the software that
// the flash is write protected.
//
#ifdef CYGSEM_IO_FLASH_SOFT_WRITE_PROTECT
externC cyg_bool plf_flash_query_soft_wp(void *addr, int len);
#endif

//---------------------------------------------------------------------------
// Execution of flash code must be done inside a
// HAL_FLASH_CACHES_OFF/HAL_FLASH_CACHES_ON region - disabling the
// cache on unified cache systems is necessary to prevent burst access
// to the flash area being programmed. With Harvard style caches, only
// the data cache needs to be disabled, but the instruction cache is
// disabled for consistency.

// Targets may provide alternative implementations for these macros in
// the hal_cache.h (or var/plf) files.

// The first part below is a generic, optimal implementation.  The
// second part is the old implementation that has been tested to work
// on some targets - but it is not be suitable for targets that would
// do burst access to the flash (it does not disable the data cache).

// Both implementations must be called with interrupts disabled.

// NOTE: Do _not_ change any of the below macros without checking that
//       the changed code still works on _all_ platforms that rely on these
//       macros. There is no such thing as logical and correct when dealing
//       with different cache and IO models, so _do not_ mess with this code
//       unless you test it properly afterwards.

#ifndef HAL_FLASH_CACHES_OFF

// Some drivers have only been tested with the old macros below.
#ifndef HAL_FLASH_CACHES_OLD_MACROS

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

#else  // HAL_FLASH_CACHES_OLD_MACROS

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

#endif  // HAL_FLASH_CACHES_OLD_MACROS

#endif  // HAL_FLASH_CACHES_OFF

#endif  // _IO_FLASH_PRIV_H_
