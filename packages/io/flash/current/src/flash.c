//==========================================================================
//
//      flash.c
//
//      Flash programming
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Author(s):    gthomas
// Contributors: gthomas, Andrew Lunn
// Date:         2000-07-26
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_flash.h>
#ifdef CYGPKG_KERNEL
#include <cyg/kernel/kapi.h>
#endif
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_tables.h>
#include <cyg/infra/cyg_ass.h>
#include <string.h>

#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>

// When this flag is set, do not actually jump to the relocated code.
// This can be used for running the function in place (RAM startup
// only), allowing calls to diag_printf() and similar.
#undef RAM_FLASH_DEV_DEBUG
#if !defined(CYG_HAL_STARTUP_RAM) && defined(RAM_FLASH_DEV_DEBUG)
# warning "Can only enable the flash debugging when configured for RAM startup"
#endif
#ifndef MIN
#define MIN(x,y) ((x)<(y) ? (x) : (y))
#endif

// Has the FLASH IO library been initialise?
static bool init = false;
// We have a linked list of flash drivers. This is the head of the
// list
static struct cyg_flash_dev *flash_head = NULL;

// This array contains entries for all flash devices that are
// installed in the system.
__externC struct cyg_flash_dev cyg_flashdevtab[];
CYG_HAL_TABLE_BEGIN(cyg_flashdevtab, cyg_flashdev);

// end of the flashdev table
__externC struct cyg_flash_dev cyg_flashdevtab_end;
CYG_HAL_TABLE_END(cyg_flashdevtab_end, cyg_flashdev);

// Sort the linked list into ascending order of flash address. Use a
// primitive ripple sort, but since we don't expect to have many
// devices this should be OK.
static void flash_sort(void) 
{
  bool moved;
  struct cyg_flash_dev *dev, **previous_next;
  
  // If there is zero or one device, short cut
  if (flash_head == NULL || flash_head->next == NULL)
    return;
  
  do {
    moved=false;
    for (dev=flash_head, previous_next=&flash_head; 
         dev->next; 
         previous_next = &dev->next, dev=dev->next ){
      if (dev->start > dev->next->start) {
        *previous_next=dev->next;
        dev->next = (*previous_next)->next;
        (*previous_next)->next = dev;
        moved=true;          
        break;
      }
    }
  } while (moved);
}

// Walk the linked list and see if there are any overlaps in the
// addresses the devices claim to use using.
static bool flash_check_overlap(void) 
{
  struct cyg_flash_dev *dev;
  
  // If there is zero or one device, short cut
  if (flash_head == NULL || flash_head->next == NULL)
    return false;

  for (dev=flash_head; dev->next; dev=dev->next){
    if (dev->end >= dev->next->start)
      return true;
  }
  return false;
}

// Initialise all registered device. Any device that fails to
// initialise we leave dev->init as false. Then sort the devices into
// ascending order of address and put them into a linked list. Lastly
// check if we have any overlap of the addresses.
__externC int 
cyg_flash_init(cyg_flash_printf *pf) 
{
  int err;
  struct cyg_flash_dev * dev;
  
  if (init) return CYG_FLASH_ERR_OK;
  init = true;
  
  for (dev = &cyg_flashdevtab[0]; dev != &cyg_flashdevtab_end; dev++) {
    dev->pf = pf;
#ifdef CYGPKG_KERNEL
    cyg_mutex_init(&dev->mutex);
#endif
    
    err = dev->funs->flash_init(dev);
    if (err != CYG_FLASH_ERR_OK) {
      continue;
    }
    CYG_ASSERT(dev->funs, "No flash functions");
    CYG_ASSERT(dev->num_block_infos, "No number of block infoss");
    CYG_ASSERT(dev->block_info, "No block infos");
    CYG_ASSERT(!(((cyg_flashaddr_t)dev->block_info >= dev->start) && 
                 ((cyg_flashaddr_t)dev->block_info < dev->end)),
               "Block info is in the flash");
    CYG_ASSERT(dev->funs->flash_erase_block, "No erase function");
    CYG_ASSERT(dev->funs->flash_program, "No program function");
    CYG_ASSERT(dev->funs->flash_hwr_map_error, "No hwr map error function");

    dev->init = true;
    dev->next = flash_head;
    flash_head = dev;
  }
  
  flash_sort();

  if (flash_check_overlap()) {
    return CYG_FLASH_ERR_INVALID;
  }
  return CYG_FLASH_ERR_OK;
}

// Is the address within one of the flash drivers?
__externC int
cyg_flash_verify_addr(const cyg_flashaddr_t address)
{
  struct cyg_flash_dev * dev;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;
  
  for (dev = flash_head; dev; dev=dev->next) {
    if ((dev->start <= address) && (dev->end > address))
      return CYG_FLASH_ERR_OK;
  }
  return CYG_FLASH_ERR_INVALID;
}

// See if a range of FLASH addresses overlaps currently running code
__externC bool cyg_flash_code_overlaps(const cyg_flashaddr_t start, 
                                       const cyg_flashaddr_t end)
{
  extern char _stext[], _etext[];
  
  return ((((unsigned long)&_stext >= (unsigned long)start) &&
           ((unsigned long)&_stext < (unsigned long)end)) ||
          (((unsigned long)&_etext >= (unsigned long)start) &&
           ((unsigned long)&_etext < (unsigned long)end)));
}

// Get the start and end address of the first driver
__externC int
cyg_flash_get_limits(cyg_flashaddr_t *start, cyg_flashaddr_t *end)
{
    if (!init) {
        return CYG_FLASH_ERR_NOT_INIT;
    }
    if (flash_head) {
      *start = flash_head->start;
      *end = flash_head->end;
      return CYG_FLASH_ERR_OK;
    }
    return CYG_FLASH_ERR_INVALID;
}

// Get the block size and number of blocks for the first driver. For
// devices with boot blocks, consolidate the boot blocks into the
// figure.
__externC int
cyg_flash_get_block_info(size_t *block_size, cyg_uint32 *blocks)
{
  size_t biggest_size=0;
  cyg_uint32 i;
  
  
  if (!init) {
    return CYG_FLASH_ERR_NOT_INIT;
  }
  if (!flash_head) return CYG_FLASH_ERR_INVALID;

  // Find the biggest size of blocks
  for (i=0; i < flash_head->num_block_infos; i++) {
    if (flash_head->block_info[i].block_size > biggest_size) {
      biggest_size = flash_head->block_info[i].block_size;
    }
  }
  
  // Calculate the number of biggest size blocks
  *block_size = biggest_size;
  *blocks = 0;
  for (i=0; i < flash_head->num_block_infos; i++) {
    *blocks += (flash_head->block_info[i].block_size *
                flash_head->block_info[i].blocks) /
      biggest_size;
  }
  return CYG_FLASH_ERR_OK;
}

// Return information about the Nth driver
__externC int
cyg_flash_get_info(cyg_uint32 Nth, cyg_flash_info_t * info)
{
  struct cyg_flash_dev * dev;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;
  
  for (dev = flash_head; dev && Nth; dev=dev->next, Nth--)
    ;
  if (dev && !Nth) {
    info->start = dev->start;
    info->end = dev->end;
    info->num_block_infos = dev->num_block_infos;
    info->block_info = dev->block_info;
    return CYG_FLASH_ERR_OK;
  }
  return CYG_FLASH_ERR_INVALID;
}

// Return information about the flash at the given address
__externC int
cyg_flash_get_info_addr(cyg_flashaddr_t flash_base, cyg_flash_info_t * info)
{
  struct cyg_flash_dev *dev;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;
  
  for (dev = flash_head; dev ; dev=dev->next) {
    if ((dev->start <= flash_base) && ( dev->end > flash_base)) {
    info->start = dev->start;
    info->end = dev->end;
    info->num_block_infos = dev->num_block_infos;
    info->block_info = dev->block_info;
    return CYG_FLASH_ERR_OK;
    }
  }
  return CYG_FLASH_ERR_INVALID;
}

#ifdef CYGPKG_KERNEL
//Lock the mutex's for a range of addresses
__externC int
cyg_flash_mutex_lock(const cyg_flashaddr_t from, const size_t len) 
{
  struct cyg_flash_dev * dev;
  int err;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;
  
  for (dev = flash_head; dev ; dev=dev->next) {
    if ((dev->start <= from) && ( dev->end > from)) {
      cyg_mutex_lock(&dev->mutex);
      if (dev->end > from+len) {
        return CYG_FLASH_ERR_OK;
      } else {                                     // Check off by one?
        //The region is bigger than this driver. Recurse
        err = cyg_flash_mutex_lock(dev->end+1, (len - (dev->end - from)));
        if (err != CYG_FLASH_ERR_OK) {
          // Something went wrong, unlock what we just locked
          cyg_mutex_unlock(&dev->mutex);
        }
        return err;
      }
    }
  }
  return CYG_FLASH_ERR_INVALID;
}

//Unlock the mutex's for a range of addresses
__externC int
cyg_flash_mutex_unlock(const cyg_flashaddr_t from, const size_t len) 
{
  struct cyg_flash_dev * dev;
  int err;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;
  
  for (dev = flash_head; dev ; dev=dev->next) {
    if ((dev->start <= from) && ( dev->end > from)) {
      cyg_mutex_unlock(&dev->mutex);
      if (dev->end > from+len) {
        return CYG_FLASH_ERR_OK;
      } else {                                     // Check off by one?
        //The region is bigger than this driver. Recurse
        err = cyg_flash_mutex_unlock(dev->end+1, (len - (dev->end - from)));
        return err;
      }
    }
  }
  return CYG_FLASH_ERR_INVALID;
}
#endif

// Return the size of the block which is at the given address
static size_t 
flash_block_size(struct cyg_flash_dev *dev, const cyg_flashaddr_t addr)
{
  int i;
  size_t offset;
  
  
  CYG_ASSERT((addr >= dev->start) && (addr <= dev->end), "Not inside device");
  
  offset = addr - dev->start;
  for (i=0; i < dev->num_block_infos; i++) {
    if (offset < (dev->block_info[i].blocks * dev->block_info[i].block_size))
      return dev->block_info[i].block_size;
    offset = offset - 
      (dev->block_info[i].blocks * dev->block_info[i].block_size);
  }
  CYG_FAIL("Programming error");
  return 0;
}

// Return the size of the block which is at the given address
__externC size_t
cyg_flash_block_size(const cyg_flashaddr_t flash_base) 
{
  struct cyg_flash_dev * dev;

  if (!init) return CYG_FLASH_ERR_NOT_INIT;

  for (dev = flash_head; 
       dev && !((dev->start <= flash_base) && ( dev->end > flash_base));
       dev=dev->next)
    ;
  if (!dev) return CYG_FLASH_ERR_INVALID;
  
  return flash_block_size(dev, flash_base);
}


__externC int 
cyg_flash_erase(const cyg_flashaddr_t flash_base, 
                const size_t len, 
                cyg_flashaddr_t *err_address)
{
  cyg_flashaddr_t block, end_addr, addr;
  struct cyg_flash_dev * dev;
  size_t block_size;
  int stat = CYG_FLASH_ERR_OK;
  int d_cache, i_cache;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;
  
#ifdef CYGSEM_IO_FLASH_SOFT_WRITE_PROTECT
  if (plf_flash_query_soft_wp(addr,len))
    return CYG_FLASH_ERR_PROTECT;
#endif
  
  for (dev = flash_head; 
       dev && !((dev->start <= flash_base) && ( dev->end > flash_base));
       dev=dev->next)
    ;
  if (!dev) return CYG_FLASH_ERR_INVALID;
  
#ifdef CYGPKG_KERNEL
  cyg_mutex_lock(&dev->mutex);
#endif
  addr = flash_base;
  end_addr = flash_base + len - 1;
  if (end_addr > dev->end) {
    end_addr = dev->end;
  }
  
  block_size = flash_block_size(dev, addr);
  block = (cyg_flashaddr_t)((size_t)addr & ~(block_size - 1));
  
#ifdef CYGSEM_IO_FLASH_CHATTER
  dev->pf("... Erase from %p-%p: ", (void*)block, (void*)end_addr);
#endif
  
  HAL_FLASH_CACHES_OFF(d_cache, i_cache);
  FLASH_Enable(block, end_addr);
  while (block <= end_addr) {
    int i;
    unsigned char *dp;
    bool erased = true;

    // If there is a read function it probably means the flash
    // cannot be read directly.
    if (!dev->funs->flash_read) {
      dp = (unsigned char *)block;
      for (i = 0;  i < block_size;  i++) {
        if (*dp++ != (unsigned char)0xFF) {
          erased = false;
          break;
        }
      }
    } else {
      erased=false;
    }
    if (!erased) {
      stat = dev->funs->flash_erase_block(dev,block);
      stat = dev->funs->flash_hwr_map_error(dev,stat);
    }
    if (CYG_FLASH_ERR_OK != stat && err_address) {
      *err_address = block;
      break;
    }
    block += flash_block_size(dev, block);
#ifdef CYGSEM_IO_FLASH_CHATTER
    dev->pf(".");
#endif
  }
  FLASH_Disable(block, end_addr);
  HAL_FLASH_CACHES_ON(d_cache, i_cache);
#ifdef CYGSEM_IO_FLASH_CHATTER
  dev->pf("\n");
#endif
#ifdef CYGPKG_KERNEL
  cyg_mutex_unlock(&dev->mutex);
#endif
  if (stat != CYG_FLASH_ERR_OK) {
    return stat;
  }
  
  if (flash_base + len - 1 > dev->end) {        
    // The region to erase if bigger than this driver handles. Recurse
    return cyg_flash_erase(dev->end+1, 
                           len - (dev->end - flash_base) - 1,
                           err_address);
  }
  return CYG_FLASH_ERR_OK;
}

__externC int 
cyg_flash_program(const cyg_flashaddr_t flash_base, 
                  void *ram_base, 
                  const size_t len, 
                  cyg_flashaddr_t *err_address)
{
  struct cyg_flash_dev * dev;
  cyg_flashaddr_t addr, end_addr;
  unsigned char * ram = ram_base;
  size_t block_size, size, length, offset;
  int stat = CYG_FLASH_ERR_OK;
  int d_cache, i_cache;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;
  
#ifdef CYGSEM_IO_FLASH_SOFT_WRITE_PROTECT
  if (plf_flash_query_soft_wp(addr,len))
    return CYG_FLASH_ERR_PROTECT;
#endif
  
  for (dev = flash_head; 
       dev && !((dev->start <= flash_base) && ( dev->end > flash_base));
       dev=dev->next)
    ;
  if (!dev) return CYG_FLASH_ERR_INVALID;
  
#ifdef CYGPKG_KERNEL
  cyg_mutex_lock(&dev->mutex);
#endif
  addr = flash_base;
  end_addr = flash_base + len - 1;
  if (end_addr > dev->end) {
    end_addr = dev->end;
  }
  length = end_addr - addr + 1;
  
#ifdef CYGSEM_IO_FLASH_CHATTER
  dev->pf("... Program from %p-%p to %p: ", ram_base, 
          ((CYG_ADDRESS)ram_base)+length, addr);
#endif
  
  HAL_FLASH_CACHES_OFF(d_cache, i_cache);
  FLASH_Enable((unsigned short*)addr, (unsigned short *)(addr+len));
  while (addr <= end_addr) {
    block_size = flash_block_size(dev, addr);
    size = length;
    // Only one block at once
    if (size > block_size) size = block_size;
    
    // Writing from the middle of a block?
    offset = (size_t)addr & (block_size-1);
    if (offset)
      size = MIN(block_size - offset, size);
    stat = dev->funs->flash_program(dev, addr, ram, size);
    stat = dev->funs->flash_hwr_map_error(dev,stat);
#ifdef CYGSEM_IO_FLASH_VERIFY_PROGRAM
    if (CYG_FLASH_ERR_OK == stat) // Claims to be OK
      if (!dev->funs->flash_read && memcmp((void *)addr, ram, size) != 0) {                
        stat = CYG_FLASH_ERR_DRV_VERIFY;
#ifdef CYGSEM_IO_FLASH_CHATTER
        dev->pf("V");
#endif
      }
#endif
    if (CYG_FLASH_ERR_OK != stat && err_address) {
      *err_address = addr;
      break;
    }
#ifdef CYGSEM_IO_FLASH_CHATTER
    dev->pf(".");
#endif
    length -= size;
    addr += size;
    ram += size;
  }
  FLASH_Disable((unsigned short*)addr, (unsigned short *)(addr+len));
  HAL_FLASH_CACHES_ON(d_cache, i_cache);
#ifdef CYGSEM_IO_FLASH_CHATTER
    dev->pf("\n");
#endif
#ifdef CYGPKG_KERNEL
  cyg_mutex_unlock(&dev->mutex);
#endif
  if (stat != CYG_FLASH_ERR_OK) {
    return (stat);
  }
  if ( flash_base + len - 1 > dev->end) {
    return cyg_flash_program(dev->end+1, ram, 
                             len - (dev->end - flash_base) - 1,
                             err_address);
  }
  return CYG_FLASH_ERR_OK;
}

__externC int 
cyg_flash_read(cyg_flashaddr_t flash_base, 
               const void *ram_base, 
               const size_t len, 
               cyg_flashaddr_t *err_address)
{
  struct cyg_flash_dev * dev;
  cyg_flashaddr_t addr, end_addr;
  unsigned char * ram = (unsigned char *)ram_base;
  size_t block_size, size, length, offset;
  int stat = CYG_FLASH_ERR_OK;
  int d_cache, i_cache;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;
  
  for (dev = flash_head; 
       dev && !((dev->start <= flash_base) && ( dev->end > flash_base));
       dev=dev->next)
    ;
  if (!dev) return CYG_FLASH_ERR_INVALID;

#ifdef CYGPKG_KERNEL
  cyg_mutex_lock(&dev->mutex);
#endif
  addr = flash_base;
  end_addr = flash_base + len - 1;
  if (end_addr > dev->end) {
    end_addr = dev->end;
  }
  length = end_addr - addr + 1;
  
#ifdef CYGSEM_IO_FLASH_CHATTER
  dev->pf("... Read from %p-%p to %p: ", addr, addr+len, ram_base);
#endif
  
  HAL_FLASH_CACHES_OFF(d_cache, i_cache);
  FLASH_Enable((unsigned short*)addr, (unsigned short *)(addr+len));
  while (addr <= end_addr) {
    block_size = flash_block_size(dev, addr);
    size = length;
    // Only one block at once
    if (size > block_size) size = block_size;
    
    // Reading from the middle of a block?
    offset = (size_t)addr & (block_size-1);
    if (offset)
      size = MIN(block_size - offset, size);
    if (dev->funs->flash_read) {
      stat = dev->funs->flash_read(dev, addr, ram, size);
      stat = dev->funs->flash_hwr_map_error(dev,stat);
    } else {
      memcpy(ram, (void *)addr, size);
      stat = CYG_FLASH_ERR_OK;
    }
    if (CYG_FLASH_ERR_OK != stat && err_address) {
      *err_address = addr;
      break;
    }
#ifdef CYGSEM_IO_FLASH_CHATTER
    dev->pf(".");
#endif
    length -= size;
    addr += size;
    ram += size;
  }
  FLASH_Disable((unsigned short*)addr, (unsigned short *)(addr+len));
  HAL_FLASH_CACHES_ON(d_cache, i_cache);
#ifdef CYGSEM_IO_FLASH_CHATTER
  dev->pf("\n");
#endif
#ifdef CYGPKG_KERNEL
  cyg_mutex_unlock(&dev->mutex);
#endif
  if (stat != CYG_FLASH_ERR_OK) {
    return (stat);
  }
  if ( flash_base + len - 1 > dev->end) {
    return cyg_flash_read(dev->end+1, ram,
                          len - (dev->end - flash_base) - 1,
                          err_address);
  }
  return CYG_FLASH_ERR_OK;
}

#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
__externC int 
cyg_flash_lock(const cyg_flashaddr_t flash_base, 
               const size_t len, 
               cyg_flashaddr_t *err_address)
{
  cyg_flashaddr_t block, end_addr, addr;
  struct cyg_flash_dev * dev;
  size_t block_size;
  int stat = CYG_FLASH_ERR_OK;
  int d_cache, i_cache;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;
  
#ifdef CYGSEM_IO_FLASH_SOFT_WRITE_PROTECT
  if (plf_flash_query_soft_wp(addr,len))
    return CYG_FLASH_ERR_PROTECT;
#endif
  
  for (dev = flash_head; 
       dev && !((dev->start <= flash_base) && ( dev->end > flash_base));
       dev=dev->next)
    ;
  if (!dev) return CYG_FLASH_ERR_INVALID;
  if (!dev->funs->flash_block_lock) return CYG_FLASH_ERR_INVALID;

#ifdef CYGPKG_KERNEL
  cyg_mutex_lock(&dev->mutex);
#endif
  addr = flash_base;
  end_addr = flash_base + len - 1;
  if (end_addr > dev->end) {
    end_addr = dev->end;
  }
  
  block_size = flash_block_size(dev, addr);
  block = (cyg_flashaddr_t)((size_t)addr & ~(block_size - 1));
  
#ifdef CYGSEM_IO_FLASH_CHATTER
  dev->pf("... Locking from %p-%p: ", (void*)block, (void*)end_addr);
#endif
  
  HAL_FLASH_CACHES_OFF(d_cache, i_cache);
  FLASH_Enable(block, end_addr);
  while (block <= end_addr) {
    stat = dev->funs->flash_block_lock(dev,block);
    stat = dev->funs->flash_hwr_map_error(dev,stat);
    
    if (CYG_FLASH_ERR_OK != stat && err_address) {
      *err_address = block;
      break;
    }
    block += flash_block_size(dev, addr);
#ifdef CYGSEM_IO_FLASH_CHATTER
    dev->pf(".");
#endif
  }
  FLASH_Disable(block, end_addr);
  HAL_FLASH_CACHES_ON(d_cache, i_cache);
#ifdef CYGSEM_IO_FLASH_CHATTER
  dev->pf("\n");
#endif
#ifdef CYGPKG_KERNEL
  cyg_mutex_unlock(&dev->mutex);
#endif
  if (stat != CYG_FLASH_ERR_OK) {
    return stat;
  }
  
  if (flash_base + len - 1 > dev->end) {        // Off by one?
    // The region to erase if bigger than this driver handles. Recurse
    return cyg_flash_lock(dev->end+1, 
                          len - (dev->end - flash_base) - 1,
                          err_address);
  }
  return CYG_FLASH_ERR_OK;
}

__externC int 
cyg_flash_unlock(const cyg_flashaddr_t flash_base, 
                 const size_t len, 
                 cyg_flashaddr_t *err_address)
{
  cyg_flashaddr_t block, end_addr, addr;
  struct cyg_flash_dev * dev;
  size_t block_size;
  int stat = CYG_FLASH_ERR_OK;
  int d_cache, i_cache;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;
  
#ifdef CYGSEM_IO_FLASH_SOFT_WRITE_PROTECT
  if (plf_flash_query_soft_wp(addr,len))
    return CYG_FLASH_ERR_PROTECT;
#endif
  
  for (dev = flash_head; 
       dev && !((dev->start <= flash_base) && ( dev->end > flash_base));
       dev=dev->next)
    ;
  if (!dev) return CYG_FLASH_ERR_INVALID;
  if (!dev->funs->flash_block_unlock) return CYG_FLASH_ERR_INVALID;
  
#ifdef CYGPKG_KERNEL
  cyg_mutex_lock(&dev->mutex);
#endif
  addr = flash_base;
  end_addr = flash_base + len - 1;
  if (end_addr > dev->end) {
    end_addr = dev->end;
  }
  
  block_size = flash_block_size(dev, addr);
  block = (cyg_flashaddr_t)((size_t)addr & ~(block_size - 1));
  
#ifdef CYGSEM_IO_FLASH_CHATTER
  dev->pf("... Unlocking from %p-%p: ", (void*)block, (void*)end_addr);
#endif
  
  HAL_FLASH_CACHES_OFF(d_cache, i_cache);
  FLASH_Enable(block, end_addr);
  while (block <= end_addr) {
    stat = dev->funs->flash_block_unlock(dev,block);
    stat = dev->funs->flash_hwr_map_error(dev,stat);
    
    if (CYG_FLASH_ERR_OK != stat && err_address) {
      *err_address = block;
      break;
    }
    block += flash_block_size(dev, addr);
#ifdef CYGSEM_IO_FLASH_CHATTER
    dev->pf(".");
#endif
  }
  FLASH_Disable(block, end_addr);
  HAL_FLASH_CACHES_ON(d_cache, i_cache);
#ifdef CYGSEM_IO_FLASH_CHATTER
  dev->pf("\n");
#endif
#ifdef CYGPKG_KERNEL
  cyg_mutex_unlock(&dev->mutex);
#endif
  if (stat != CYG_FLASH_ERR_OK) {
    return stat;
  }
  
  if (flash_base + len - 1 > dev->end) {        // Off by one?
    // The region to erase if bigger than this driver handles. Recurse
    return cyg_flash_lock(dev->end+1, 
                          len - (dev->end - flash_base) - 1,
                          err_address);
  }
  return CYG_FLASH_ERR_OK;
}
#endif

const char *
cyg_flash_errmsg(const int err)
{
    switch (err) {
    case CYG_FLASH_ERR_OK:
        return "No error - operation complete";
    case CYG_FLASH_ERR_ERASE_SUSPEND:
        return "Device is in erase suspend state";
    case CYG_FLASH_ERR_PROGRAM_SUSPEND:
        return "Device is in program suspend state";
    case CYG_FLASH_ERR_INVALID:
        return "Invalid FLASH address";
    case CYG_FLASH_ERR_ERASE:
        return "Error trying to erase";
    case CYG_FLASH_ERR_LOCK:
        return "Error trying to lock/unlock";
    case CYG_FLASH_ERR_PROGRAM:
        return "Error trying to program";
    case CYG_FLASH_ERR_PROTOCOL:
        return "Generic error";
    case CYG_FLASH_ERR_PROTECT:
        return "Device/region is write-protected";
    case CYG_FLASH_ERR_NOT_INIT:
        return "FLASH sub-system not initialized";
    case CYG_FLASH_ERR_DRV_VERIFY:
        return "Data verify failed after operation";
    case CYG_FLASH_ERR_DRV_TIMEOUT:
        return "Driver timed out waiting for device";
    case CYG_FLASH_ERR_DRV_WRONG_PART:
        return "Driver does not support device";
    case CYG_FLASH_ERR_LOW_VOLTAGE:
        return "Device reports low voltage";
    default:
        return "Unknown error";
    }
}

// EOF io/flash/..../flash.c
