//==========================================================================
//
//      flash_priv.h
//
//      Flash programming - driver internals
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

#endif  // _IO_FLASH_PRIV_H_
