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
  int     (*flash_init) (struct cyg_flash_dev *dev);
  size_t  (*flash_query) (struct cyg_flash_dev *dev,
                          void * data, size_t len);
  int     (*flash_erase_block) (struct cyg_flash_dev *dev, 
                                cyg_flashaddr_t block_base);
  int     (*flash_program) (struct cyg_flash_dev *dev, 
                            cyg_flashaddr_t base, 
                            const void* data, size_t len);
  int     (*flash_read) (struct cyg_flash_dev *dev, 
                         const cyg_flashaddr_t base, 
                         void* data, size_t len);
#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING    
  int     (*flash_block_lock) (struct cyg_flash_dev *dev, 
                               const cyg_flashaddr_t block_base);
  int     (*flash_block_unlock) (struct cyg_flash_dev *dev, 
                                 const cyg_flashaddr_t block_base);
#endif    
};

// Dummy functions for some of the above operations, if a device does
// not support e.g. locking.
externC int     cyg_flash_devfn_init_nop(struct cyg_flash_dev*);
externC size_t  cyg_flash_devfn_query_nop(struct cyg_flash_dev*, void*, const size_t);
externC int     cyg_flash_devfn_lock_nop(struct cyg_flash_dev*, const cyg_flashaddr_t);
externC int     cyg_flash_devfn_unlock_nop(struct cyg_flash_dev*, const cyg_flashaddr_t);

// Facilitate function calls between flash-resident code and .2ram
// functions.
externC void*   cyg_flash_anonymizer(void*);

// Structure each device places in the HAL table
struct cyg_flash_dev {
  const struct cyg_flash_dev_funs *funs;            // Function pointers
  cyg_uint32                      flags;            // Device characteristics
  cyg_flashaddr_t                 start;            // First address
  cyg_flashaddr_t                 end;              // Last address
  cyg_uint32                      num_block_infos;  // Number of entries
  const cyg_flash_block_info_t    *block_info;      // Info about one block size

  const void                      *priv;            // Devices private data

  // The following are only written to by the FLASH IO layer.
  cyg_flash_printf                *pf;              // Pointer to diagnostic printf
  bool                            init;             // Device has been initialised
#ifdef CYGPKG_KERNEL
  cyg_mutex_t                     mutex;            // Mutex for thread safeness
#endif
#if (CYGHWR_IO_FLASH_DEVICE > 1)    
  struct cyg_flash_dev            *next;            // Pointer to next device
#endif    
} CYG_HAL_TABLE_TYPE;

#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
# define CYG_FLASH_FUNS(_funs_, _init_, _query_ , _erase_, _prog_ , _read_, _lock_, _unlock_) \
struct cyg_flash_dev_funs _funs_ =      \
{										\
	.flash_init             = _init_,   \
	.flash_query            = _query_,  \
	.flash_erase_block      = _erase_,  \
	.flash_program          = _prog_,   \
	.flash_read             = _read_,   \
	.flash_block_lock       = _lock_,   \
	.flash_block_unlock     = _unlock_  \
}
#else
# define CYG_FLASH_FUNS(_funs_, _init_, _query_ , _erase_, _prog_ , _read_, _lock_, _unlock_) \
struct cyg_flash_dev_funs _funs_ =      \
{										\
	.flash_init             = _init_,   \
	.flash_query            = _query_,  \
	.flash_erase_block      = _erase_,  \
	.flash_program          = _prog_,   \
	.flash_read             = _read_    \
}
#endif

// We assume HAL tables are placed into RAM.
#define CYG_FLASH_DRIVER(_name_, _funs_, _flags_, _start_, _end_, _num_block_infos_, _block_info_, _priv_)  \
struct cyg_flash_dev _name_ CYG_HAL_TABLE_ENTRY(cyg_flashdev) = \
{                                                               \
    .funs               = _funs_,                               \
    .flags              = _flags_,                              \
    .start              = _start_,                              \
    .end                = _end_,                                \
    .num_block_infos    = _num_block_infos_,                    \
    .block_info         = _block_info_,                         \
    .priv               = _priv_                                \
}

#ifdef CYGHWR_IO_FLASH_DEVICE_LEGACY
struct flash_info {
  int	block_size;	  // Assuming fixed size "blocks"
  int	blocks;		  // Number of blocks
  int	buffer_size;  // Size of write buffer (only defined for some devices)
  unsigned long block_mask;
  void *start, *end;  // Address range
  int	init;
  cyg_flash_printf *pf;
};

externC struct flash_info flash_info;
externC int	 flash_hwr_init(void);
externC int	 flash_hwr_map_error(int err);
externC void flash_dev_query(void *data);
#endif // CYGHWR_IO_FLASH_DEVICE_LEGACY

#endif  // _IO_FLASH_PRIV_H_
