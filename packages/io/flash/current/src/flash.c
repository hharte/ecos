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
// Copyright (C) 2004 Andrew Lunn
// Copyright (C) 2004 eCosCentric Ltd.
// Copyright (C) 2003 Gary Thomas
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// Contributors: gthomas, Andrew Lunn, Bart Veer
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

// Optional verbosity. Using a macro here avoids lots of ifdefs in the
// rest of the code.
#ifdef CYGSEM_IO_FLASH_CHATTER
# define CHATTER(_dev_, _fmt_, ...) (*(_dev_)->pf)((_fmt_), ## __VA_ARGS__)
#else
# define CHATTER(_dev_, _fmt_, ...) CYG_EMPTY_STATEMENT
#endif

// Per-thread locking. Again using macros avoids lots of ifdefs
#ifdef CYGPKG_KERNEL
# define LOCK_INIT(_dev_)   cyg_mutex_init(&((_dev_)->mutex))
# define LOCK(_dev_)        cyg_mutex_lock(&((_dev_)->mutex))
# define UNLOCK(_dev_)      cyg_mutex_unlock(&((_dev_)->mutex))
#else
# define LOCK_INIT(_dev_)   CYG_EMPTY_STATEMENT
# define LOCK(_dev_)        CYG_EMPTY_STATEMENT
# define UNLOCK(_dev_)      CYG_EMPTY_STATEMENT
#endif

// Software write-protect. Very rarely used.
#ifdef CYGSEM_IO_FLASH_SOFT_WRITE_PROTECT
# define CHECK_SOFT_WRITE_PROTECT(_addr_, _len_)    \
  CYG_MACRO_START                                   \
  if (plf_flash_query_soft_wp((_addr_), (_len_)))   \
    return CYG_FLASH_ERR_PROTECT;                   \
  CYG_MACRO_END
#else
#define CHECK_SOFT_WRITE_PROTECT(_addr_, _len_) CYG_EMPTY_STATEMENT
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
    LOCK_INIT(dev);
    
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
#ifdef CYGDBG_USE_ASSERTS
    {
         int i; 
         cyg_flashaddr_t addr = dev->start;
         for (i = 0; i < dev->num_block_infos; i++) {
              addr += dev->block_info[i].block_size * dev->block_info[i].blocks;
         }
         CYG_ASSERT(dev->end == addr-1, "Invalid end address");
    }
#endif
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
    if ((dev->start <= address) && (address <= dev->end))
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
    if ((dev->start <= flash_base) && ( flash_base <= dev->end)) {
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
// Lock the mutex's for a range of addresses
__externC int
cyg_flash_mutex_lock(const cyg_flashaddr_t from, const size_t len) 
{
  struct cyg_flash_dev * dev;
  int err;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;
  
  for (dev = flash_head; dev ; dev=dev->next) {
    if ((dev->start <= from) && ( from <= dev->end)) {
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

// Unlock the mutex's for a range of addresses
__externC int
cyg_flash_mutex_unlock(const cyg_flashaddr_t from, const size_t len) 
{
  struct cyg_flash_dev * dev;
  int err;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;
  
  for (dev = flash_head; dev ; dev=dev->next) {
    if ((dev->start <= from) && ( from <= dev->end)) {
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
       dev && !((dev->start <= flash_base) && ( flash_base <= dev->end));
       dev=dev->next)
    ;
  if (!dev) return CYG_FLASH_ERR_INVALID;
  
  return flash_block_size(dev, flash_base);
}

// Return the first address of a block. The flash might not be aligned
// in terms of its block size. So we have to be careful and use
// offsets.
static inline cyg_flashaddr_t 
flash_block_begin(cyg_flashaddr_t addr, struct cyg_flash_dev *dev)
{
  size_t block_size;
  cyg_flashaddr_t offset;
  
  block_size = flash_block_size(dev, addr);
  
  offset = addr - dev->start;
  offset = (offset / block_size) * block_size;
  return offset + dev->start;
}


__externC int 
cyg_flash_erase(const cyg_flashaddr_t flash_base, 
                const size_t len, 
                cyg_flashaddr_t *err_address)
{
  cyg_flashaddr_t block, end_addr;
  struct cyg_flash_dev * dev;
  size_t erase_count;
  int stat = CYG_FLASH_ERR_OK;
  int d_cache, i_cache;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;
  
  for (dev = flash_head; 
       dev && !((dev->start <= flash_base) && ( flash_base <= dev->end));
       dev=dev->next)
    ;
  if (!dev) return CYG_FLASH_ERR_INVALID;

  CHECK_SOFT_WRITE_PROTECT(flash_base, len);
  
  LOCK(dev);

  // Check whether or not we are going past the end of this device, on
  // to the next one. If so the next device will be handled by a
  // recursive call later on.
  if (len > (dev->end + 1 - flash_base)) {
      end_addr = dev->end;
  } else {
      end_addr = flash_base + len - 1;
  }
  // erase can only happen on a block boundary, so adjust for this
  block         = flash_block_begin(flash_base, dev);
  erase_count   = (end_addr + 1) - block;

  CHATTER(dev, "... Erase from %p-%p: ", (void*)block, (void*)end_addr);
  
  HAL_FLASH_CACHES_OFF(d_cache, i_cache);
  FLASH_Enable(flash_base, end_addr);
  while (erase_count > 0) {
    int i;
    unsigned char *dp;
    bool erased = false;
    size_t block_size = flash_block_size(dev, block);

    // Pad to the block boundary, if necessary
    if (erase_count < block_size) {
        erase_count = block_size;
    }

    // If there is a read function it probably means the flash
    // cannot be read directly.
    if (!dev->funs->flash_read) {
      erased = true;
      dp = (unsigned char *)block;
      for (i = 0;  i < block_size;  i++) {
        if (*dp++ != (unsigned char)0xFF) {
          erased = false;
          break;
        }
      }
    }
    if (!erased) {
      stat = dev->funs->flash_erase_block(dev,block);
      stat = dev->funs->flash_hwr_map_error(dev,stat);
    }
    if (CYG_FLASH_ERR_OK != stat && err_address) {
      *err_address = block;
      break;
    }
    block       += block_size;
    erase_count -= block_size;
    CHATTER(dev, ".");
  }
  FLASH_Disable(flash_base, end_addr);
  HAL_FLASH_CACHES_ON(d_cache, i_cache);
  CHATTER(dev, "\n");
  UNLOCK(dev);
  if (stat != CYG_FLASH_ERR_OK) {
    return stat;
  }

  // If there are multiple flash devices in series the erase operation
  // may touch successive devices. This can be handled by recursion.
  // The stack overheads should be minimal because the number of
  // devices will be small.
  if (len > (dev->end + 1 - flash_base)) {
    return cyg_flash_erase(dev->end+1, 
                           len - (dev->end + 1 - flash_base),
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
  cyg_flashaddr_t addr, end_addr, block;
  unsigned char * ram = ram_base;
  size_t write_count, offset;
  int stat = CYG_FLASH_ERR_OK;
  int d_cache, i_cache;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;

  for (dev = flash_head; 
       dev && !((dev->start <= flash_base) && ( flash_base <= dev->end));
       dev=dev->next)
    ;
  if (!dev) return CYG_FLASH_ERR_INVALID;

  CHECK_SOFT_WRITE_PROTECT(flash_base, len);
  
  LOCK(dev);
  addr = flash_base;
  if (len > (dev->end + 1 - flash_base)) {
    end_addr = dev->end;
  } else {
    end_addr = flash_base + len - 1;
  }
  write_count = (end_addr + 1) - flash_base;

  // The first write may be in the middle of a block. Do the necessary
  // adjustment here rather than inside the loop.
  block = flash_block_begin(flash_base, dev);
  if (addr == block) {
      offset = 0;
  } else {
      offset = addr - block;
  }
  
  CHATTER(dev, "... Program from %p-%p to %p: ", ram_base, ((CYG_ADDRESS)ram_base)+write_count, addr);
  
  HAL_FLASH_CACHES_OFF(d_cache, i_cache);
  FLASH_Enable(flash_base, end_addr);
  while (write_count > 0) {
    size_t block_size = flash_block_size(dev, addr);
    size_t this_write;
    if (write_count > (block_size - offset)) {
        this_write = block_size - offset;
    } else {
        this_write = write_count;
    }
    // Only the first block may need the offset.
    offset       = 0;
    
    stat = dev->funs->flash_program(dev, addr, ram, this_write);
    stat = dev->funs->flash_hwr_map_error(dev,stat);
#ifdef CYGSEM_IO_FLASH_VERIFY_PROGRAM
    if (CYG_FLASH_ERR_OK == stat) // Claims to be OK
      if (!dev->funs->flash_read && memcmp((void *)addr, ram, this_write) != 0) {                
        stat = CYG_FLASH_ERR_DRV_VERIFY;
        CHATTER(dev, "V");
      }
#endif
    if (CYG_FLASH_ERR_OK != stat && err_address) {
      *err_address = addr;
      break;
    }
    CHATTER(dev, ".");
    write_count -= this_write;
    addr        += this_write;
    ram         += this_write;
  }
  FLASH_Disable(flash_base, end_addr);
  HAL_FLASH_CACHES_ON(d_cache, i_cache);
  CHATTER(dev, "\n");
  UNLOCK(dev);
  if (stat != CYG_FLASH_ERR_OK) {
    return (stat);
  }
  if (len > (dev->end + 1 - flash_base)) {
    return cyg_flash_program(dev->end+1, ram, 
                             len - (dev->end + 1 - flash_base),
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
  size_t read_count;
  int stat = CYG_FLASH_ERR_OK;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;
  
  for (dev = flash_head; 
       dev && !((dev->start <= flash_base) && ( flash_base <= dev->end));
       dev=dev->next)
    ;
  if (!dev) return CYG_FLASH_ERR_INVALID;

  LOCK(dev);
  addr = flash_base;
  if (len > (dev->end + 1 - flash_base)) {
      end_addr = dev->end;
  } else {
      end_addr = flash_base + len - 1;
  }
  read_count = (end_addr + 1) - flash_base;

  CHATTER(dev, "... Read from %p-%p to %p: ", addr, end_addr, ram_base);

  // If the flash is directly accessible, just read it in one go. This
  // still happens with the mutex locked to protect against concurrent
  // programs/erases.
  if (! dev->funs->flash_read) {
      memcpy(ram, (void*)addr, read_count);
  } else {
#ifndef CYGHWR_IO_FLASH_INDIRECT_READS
      CYG_FAIL("read function supplied but indirect reads not enabled");
      stat = CYG_FLASH_ERR_PROTOCOL;
      if (err_address) {
          *err_address = addr;
      }
#else
      // We have to indirect through the device driver.
      // The first read may be in the middle of a block. Do the necessary
      // adjustment here rather than inside the loop.
      int               d_cache, i_cache;
      size_t            offset;
      cyg_flashaddr_t   block = flash_block_begin(flash_base, dev);
      if (addr == block) {
          offset = 0;
      } else {
          offset = addr - block;
      }
      HAL_FLASH_CACHES_OFF(d_cache, i_cache);
      FLASH_Enable(flash_base, end_addr);
      while (read_count > 0) {
          size_t block_size = flash_block_size(dev, addr);
          size_t this_read;
          if (read_count > (block_size - offset)) {
              this_read = block_size - offset;
          } else {
              this_read = read_count;
          }
          // Only the first block may need the offset
          offset      = 0;
    
          stat = dev->funs->flash_read(dev, addr, ram, this_read);
          stat = dev->funs->flash_hwr_map_error(dev,stat);
          if (CYG_FLASH_ERR_OK != stat && err_address) {
              *err_address = addr;
              break;
          }
          CHATTER(dev, ".");
          read_count  -= this_read;
          addr        += this_read;
          ram         += this_read;
      }
      FLASH_Disable(flash_base, end_addr);
      HAL_FLASH_CACHES_ON(d_cache, i_cache);
#endif      
  }
  CHATTER(dev, "\n");
  UNLOCK(dev);
  if (stat != CYG_FLASH_ERR_OK) {
    return (stat);
  }
  if (len > (dev->end + 1 - flash_base)) {
      return cyg_flash_read(dev->end+1, ram,
                            len - (dev->end + 1 - flash_base),
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
  cyg_flashaddr_t block, end_addr;
  struct cyg_flash_dev * dev;
  size_t lock_count;
  int stat = CYG_FLASH_ERR_OK;
  int d_cache, i_cache;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;

  for (dev = flash_head; 
       dev && !((dev->start <= flash_base) && ( flash_base <= dev->end));
       dev=dev->next)
    ;
  if (!dev) return CYG_FLASH_ERR_INVALID;
  if (!dev->funs->flash_block_lock) return CYG_FLASH_ERR_INVALID;

  CHECK_SOFT_WRITE_PROTECT(flash_base, len);
  
  LOCK(dev);
  if (len > (dev->end + 1 - flash_base)) {
      end_addr = dev->end;
  } else {
      end_addr = flash_base + len - 1;
  }
  block         = flash_block_begin(flash_base, dev);
  lock_count    = (end_addr + 1) - block;
  
  CHATTER(dev, "... Locking from %p-%p: ", (void*)block, (void*)end_addr);
  
  HAL_FLASH_CACHES_OFF(d_cache, i_cache);
  FLASH_Enable(flash_base, end_addr);
  while (lock_count > 0) {
    size_t  block_size  = flash_block_size(dev, block);
    if (lock_count < block_size) {
        lock_count = block_size;
    }
    stat = dev->funs->flash_block_lock(dev,block);
    stat = dev->funs->flash_hwr_map_error(dev,stat);
    
    if (CYG_FLASH_ERR_OK != stat && err_address) {
      *err_address = block;
      break;
    }
    block       += block_size;
    lock_count  -= block_size;
    CHATTER(dev, ".");
  }
  FLASH_Disable(flash_base, end_addr);
  HAL_FLASH_CACHES_ON(d_cache, i_cache);
  CHATTER(dev, "\n");
  UNLOCK(dev);
  if (stat != CYG_FLASH_ERR_OK) {
    return stat;
  }

  // Recurse if necessary for the next device
  if (len > (dev->end + 1 - flash_base)) {
    return cyg_flash_lock(dev->end+1, 
                          len - (dev->end + 1 - flash_base),
                          err_address);
  }

  return CYG_FLASH_ERR_OK;
}

__externC int 
cyg_flash_unlock(const cyg_flashaddr_t flash_base, 
                 const size_t len, 
                 cyg_flashaddr_t *err_address)
{
  cyg_flashaddr_t block, end_addr;
  struct cyg_flash_dev * dev;
  size_t unlock_count;
  int stat = CYG_FLASH_ERR_OK;
  int d_cache, i_cache;
  
  if (!init) return CYG_FLASH_ERR_NOT_INIT;

  for (dev = flash_head; 
       dev && !((dev->start <= flash_base) && ( flash_base <= dev->end));
       dev=dev->next)
    ;
  if (!dev) return CYG_FLASH_ERR_INVALID;
  if (!dev->funs->flash_block_unlock) return CYG_FLASH_ERR_INVALID;

  CHECK_SOFT_WRITE_PROTECT(flash_base, len);
  
  LOCK(dev);
  if (len > (dev->end + 1 - flash_base)) {
      end_addr = dev->end;
  } else {
      end_addr = flash_base + len - 1;
  }
  block         = flash_block_begin(flash_base, dev);
  unlock_count  = (end_addr + 1) - block;
  
  CHATTER(dev, "... Unlocking from %p-%p: ", (void*)block, (void*)end_addr);
  
  HAL_FLASH_CACHES_OFF(d_cache, i_cache);
  FLASH_Enable(flash_base, end_addr);
  while (unlock_count > 0) {
    size_t    block_size  = flash_block_size(dev, block);
    if (unlock_count < block_size) {
        unlock_count = block_size;
    }
    stat = dev->funs->flash_block_unlock(dev,block);
    stat = dev->funs->flash_hwr_map_error(dev,stat);
    
    if (CYG_FLASH_ERR_OK != stat && err_address) {
      *err_address = block;
      break;
    }
    block           += block_size;
    unlock_count    -= block_size;
    
    CHATTER(dev, ".");
  }
  FLASH_Disable(flash_base, end_addr);
  HAL_FLASH_CACHES_ON(d_cache, i_cache);
  CHATTER(dev, "\n");
  UNLOCK(dev);
  if (stat != CYG_FLASH_ERR_OK) {
    return stat;
  }
  
  // Recurse if necessary for the next device
  if (len > (dev->end + 1 - flash_base)) {
    return cyg_flash_lock(dev->end+1, 
                          len - (dev->end + 1 - flash_base),
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
