//==========================================================================
//
//      legacy_api.c
//
//      Legacy API implementation on top of the new API
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

#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>

int
flash_init(_printf *pf)
{
  return cyg_flash_init(pf);
}

int
flash_verify_addr(void *target)
{
  return cyg_flash_verify_addr((cyg_flashaddr_t) target);
}

int
flash_get_limits(void *target, void **start, void **end)
{
  return cyg_flash_get_limits((cyg_flashaddr_t *)start, 
                              (cyg_flashaddr_t *)end);
}

int
flash_get_block_info(int *block_size, int *blocks)
{
  return cyg_flash_get_block_info((size_t *)block_size, blocks);
}

int
flash_erase(void *addr, int len, void **err_addr)
{
  return cyg_flash_erase((cyg_flashaddr_t)addr, len, 
                         (cyg_flashaddr_t *)err_addr);
}

int flash_program(void *flash_base, void *ram_base, int len, 
                  void **err_address)
{
  return cyg_flash_program((cyg_flashaddr_t)flash_base, ram_base, 
                           len, (cyg_flashaddr_t *)err_address);
}

int flash_read(void *flash_base, void *ram_base, int len, void **err_address)
{
  return cyg_flash_read((cyg_flashaddr_t)flash_base, ram_base, 
                        len, (cyg_flashaddr_t *)err_address);
}
