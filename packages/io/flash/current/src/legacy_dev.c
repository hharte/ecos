//==========================================================================
//
//      legacy_dev.c
//
//      Interface to the legacy device drivers 
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2004 Andrew Lunn
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
// Author(s):    Andrew Lunn
// Contributors: Andrew Lunn
// Date:         2004-07-02
// Purpose:      
// Description:  Implement an interface to the legacy device drivers
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_flash.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <string.h>

#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>

// When this flag is set, do not actually jump to the relocated code.
// This can be used for running the function in place (RAM startup only),
// allowing calls to diag_printf() and similar.
#undef RAM_FLASH_DEV_DEBUG
#if !defined(CYG_HAL_STARTUP_RAM) && defined(RAM_FLASH_DEV_DEBUG)
# warning "Can only enable the flash debugging when configured for RAM startup"
#endif

struct flash_info flash_info;

// These are the functions in the HW specific driver we need to call.
typedef void code_fun(void*);

externC code_fun flash_query;
externC code_fun flash_erase_block;
externC code_fun flash_program_buf;
externC code_fun flash_read_buf;
externC code_fun flash_lock_block;
externC code_fun flash_unlock_block;

// Initialize the device
static int 
legacy_flash_init (struct cyg_flash_dev *dev)
{
  int err;
  static cyg_block_info_t block_info[1];

  err=flash_hwr_init();

  if (!err) {
    dev->start = (cyg_flashaddr_t)flash_info.start;
    dev->end = (cyg_flashaddr_t)flash_info.end;
    dev->num_block_infos = 1;
    dev->block_info = block_info;
    block_info[0].block_size = flash_info.block_size;
    block_info[0].blocks = flash_info.blocks;
  }
  return err;
}

// Use this function to make function pointers anonymous - forcing the
// compiler to use jumps instead of branches when calling driver
// services.
static void* __anonymizer(void* p)
{
  return p;
}

static size_t 
legacy_flash_query (struct cyg_flash_dev *dev, 
                    void * data, 
                    const size_t len)
     __attribute__ ((section (".2ram.flash_program_buf")));

static size_t 
legacy_flash_query (struct cyg_flash_dev *dev, 
                    void * data, 
                    const size_t len)
{
  typedef void code_fun(void*);
  code_fun *_flash_query;
  
  _flash_query = (code_fun*) __anonymizer(&flash_query);
  
  (*_flash_query)(data);
  
  return len;
}

static int 
legacy_flash_erase_block (struct cyg_flash_dev *dev, 
                          const cyg_flashaddr_t block_base)
     __attribute__ ((section (".2ram.flash_program_buf")));

static int 
legacy_flash_erase_block (struct cyg_flash_dev *dev, 
                          const cyg_flashaddr_t block_base)
{
  typedef int code_fun(cyg_flashaddr_t, unsigned int);
  code_fun *_flash_erase_block;
  size_t block_size = dev->block_info[0].block_size;
  
  _flash_erase_block = (code_fun*) __anonymizer(&flash_erase_block);

  return (*_flash_erase_block)(block_base, block_size);
}

static int
legacy_flash_program(struct cyg_flash_dev *dev, 
                     cyg_flashaddr_t base, 
                     const void* data, const size_t len)
     __attribute__ ((section (".2ram.flash_program_buf")));

static int
legacy_flash_program(struct cyg_flash_dev *dev, 
                     cyg_flashaddr_t base, 
                     const void* data, const size_t len)
{
  typedef int code_fun(cyg_flashaddr_t, const void *, int, unsigned long, int);
  code_fun *_flash_program_buf;
  size_t block_size = dev->block_info[0].block_size;
  size_t block_mask = ~(block_mask -1);

  _flash_program_buf = (code_fun*) __anonymizer(&flash_program_buf);

  return (*_flash_program_buf)(base, data, len, block_mask ,block_size);
}

static int 
legacy_flash_read (struct cyg_flash_dev *dev, 
                   const cyg_flashaddr_t base, 
                   void* data, const size_t len)
     __attribute__ ((section (".2ram.flash_program_buf")));
     
static int 
legacy_flash_read (struct cyg_flash_dev *dev, 
                   const cyg_flashaddr_t base, 
                   void* data, const size_t len)
{
#ifdef CYGSEM_IO_FLASH_READ_INDIRECT
  typedef int code_fun(void *, void *, int, unsigned long, int);
  code_fun *_flash_read_buf;
  size_t block_size = dev->block_info[0].block_size;
  size_t block_mask = ~(block_mask -1);

  _flash_read_buf = (code_fun*) __anonymizer(&flash_read_buf);
  
  return = (*_flash_read_buf)(base, data, len, block_mask, buffer_size);
#else
  memcpy(data,(void *)base, len);
  return CYG_FLASH_ERR_OK;
#endif
}


static int 
legacy_flash_block_lock (struct cyg_flash_dev *dev, 
                         const cyg_flashaddr_t block_base)
     __attribute__ ((section (".2ram.flash_program_buf")));

static int 
legacy_flash_block_lock (struct cyg_flash_dev *dev, 
                         const cyg_flashaddr_t block_base)
{
#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
  typedef int code_fun(cyg_flashaddr_t);
  code_fun *_flash_lock_block;
  
  _flash_lock_block = (code_fun*) __anonymizer(&flash_lock_block);

  return (*_flash_lock_block)(block_base);
#else
  return CYG_FLASH_ERR_INVALID;
#endif
}

static int 
legacy_flash_block_unlock (struct cyg_flash_dev *dev, 
                           const cyg_flashaddr_t block_base)
     __attribute__ ((section (".2ram.flash_program_buf")));
     
static int 
legacy_flash_block_unlock (struct cyg_flash_dev *dev, 
                           const cyg_flashaddr_t block_base)
{
#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
  typedef int code_fun(cyg_flashaddr_t, int, int);
  code_fun *_flash_unlock_block;
  size_t block_size = dev->block_info[0].block_size;
  cyg_uint32 blocks = dev->block_info[0].blocks;
  
  _flash_unlock_block = (code_fun*) __anonymizer(&flash_unlock_block);
  
  return (*_flash_unlock_block)(block_base, block_size, blocks);
#else
  return CYG_FLASH_ERR_INVALID;
#endif
}

// Map a hardware status to a package error
static int 
legacy_flash_hwr_map_error (struct cyg_flash_dev *dev, int err)
     __attribute__ ((section (".2ram.flash_program_buf")));

static int 
legacy_flash_hwr_map_error (struct cyg_flash_dev *dev, int err)
{
  return flash_hwr_map_error(err);
}

void
flash_dev_query(void* data)
{
    typedef void code_fun(void*);
    code_fun *_flash_query;
    int d_cache, i_cache;

    _flash_query = (code_fun*) __anonymizer(&flash_query);

    HAL_FLASH_CACHES_OFF(d_cache, i_cache);
    (*_flash_query)(data);
    HAL_FLASH_CACHES_ON(d_cache, i_cache);
}

CYG_FLASH_FUNS(cyg_legacy_funs, 
               legacy_flash_init,
               legacy_flash_query,
               legacy_flash_erase_block,
               legacy_flash_program,
               legacy_flash_read,
               legacy_flash_hwr_map_error,
               legacy_flash_block_lock,
               legacy_flash_block_unlock
);

CYG_FLASH_DRIVER(cyg_zzlegacy_flashdev, // zz so that it probably comes last.
                 &cyg_legacy_funs,
                 NULL,  // Pointer to config structure
                 0,     // Start address of flash
                 0);    // Size of private structure

