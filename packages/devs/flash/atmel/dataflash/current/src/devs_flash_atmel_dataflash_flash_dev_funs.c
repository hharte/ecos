//==========================================================================
//
//      devs_flash_atmel_dataflash_flash_dev_funs.c 
//
//      DataFlash flash device funs 
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
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
// Author(s):    Savin Zlobec <savin@elatec.si> 
// Date:         2004-09-06
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/devs_flash_atmel_dataflash.h>
#include <pkgconf/io_flash.h>

#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/io/spi.h>

#define  _FLASH_PRIVATE_
#include <cyg/io/dataflash.h>

// -------------------------------------------------------------------------- 

static cyg_uint32
get_page_2size_log(cyg_uint32 size)
{
    cyg_uint32 i;

    i = 0;
    while (size != 0)
    {
        size >>= 1;
        i++;
    } 
    return (i-1);
}

// -------------------------------------------------------------------------- 

static int 
df_flash_init(struct cyg_flash_dev *dev)
{
    static cyg_block_info_t block_info[1];
    
    cyg_dataflash_flash_dev_config_t *config;
    cyg_dataflash_flash_dev_priv_t   *priv;
    cyg_dataflash_device_t           *df_dev;
    int sector_cnt;
    
    config = (cyg_dataflash_flash_dev_config_t *) dev->config;
    priv   = (cyg_dataflash_flash_dev_priv_t *) dev->priv; 
    df_dev = &priv->dev;
   
//    diag_printf("\nDF INIT\n");

#ifdef CYGPKG_REDBOOT    
    if (!cyg_dataflash_init(*(config->spi_dev), true, df_dev))
#else
    if (!cyg_dataflash_init(*(config->spi_dev), false, df_dev))
#endif        
        return CYG_FLASH_ERR_DRV_WRONG_PART; 
   
    sector_cnt = cyg_dataflash_get_sector_count(df_dev);

//    diag_printf("\nDF INIT sector cnt = %d\n", sector_cnt);
    
    if (config->end_sector < 0)
       config->end_sector = sector_cnt - 1; 
    
//    diag_printf("\nDF INIT end sector = %d\n", config->end_sector);

    if (config->start_sector >= sector_cnt ||
        config->end_sector   >= sector_cnt ||
        config->end_sector   <  config->start_sector)
        return CYG_FLASH_ERR_INVALID;        
    
    priv->start_page = cyg_dataflash_get_block_size(df_dev) *
        cyg_dataflash_get_sector_start(df_dev, config->start_sector);

    priv->end_page = cyg_dataflash_get_block_size(df_dev) *
        (cyg_dataflash_get_sector_start(df_dev, config->end_sector) +
         cyg_dataflash_get_sector_size(df_dev, config->end_sector));
    
//    diag_printf("\nDF INIT start page = %d end page = %d\n", 
//            priv->start_page, priv->end_page);
    
    priv->page_2size_log = 
        get_page_2size_log(cyg_dataflash_get_page_size(df_dev));
            
//    diag_printf("\nDF INIT page_2size_log = %d\n", priv->page_2size_log);
    
    dev->start = (cyg_flashaddr_t)config->address;
    dev->end   = (cyg_flashaddr_t)(config->address + 
        ((priv->end_page - priv->start_page) << priv->page_2size_log));

//    diag_printf("\nDF INIT start = %x end = %x\n", 
//            dev->start, dev->end);

    dev->num_block_infos     = 1;
    dev->block_info          = block_info;
    block_info[0].block_size = 1 << priv->page_2size_log;
    block_info[0].blocks     = priv->end_page - priv->start_page;

//    diag_printf("\nDF INIT block_size = %d blocks = %d\n",
//           block_info[0].block_size, block_info[0].blocks); 
    
    return CYG_FLASH_ERR_OK;
}

static size_t 
df_flash_query(struct cyg_flash_dev *dev, void *data, const size_t len)
{
    return len;
}

static int 
df_flash_erase_block(struct cyg_flash_dev   *dev, 
                     const  cyg_flashaddr_t  base)
{
    cyg_dataflash_flash_dev_config_t *config;
    cyg_dataflash_flash_dev_priv_t   *priv;
    cyg_uint32 page;

    config = (cyg_dataflash_flash_dev_config_t *) dev->config;
    priv   = (cyg_dataflash_flash_dev_priv_t *) dev->priv;
    
    page = priv->start_page + 
        ((base - config->address) >> priv->page_2size_log); 
   
//    diag_printf("\nDF ERASE base = %x page = %d\n", base, page);

    cyg_dataflash_aquire(&priv->dev);    
    cyg_dataflash_erase(&priv->dev, page, true); 
    cyg_dataflash_release(&priv->dev);
    
    return CYG_FLASH_ERR_OK;
}

static int
df_flash_program(struct cyg_flash_dev *dev, 
                 cyg_flashaddr_t       base, 
                 const void           *data, 
                 const size_t          len)
{
    cyg_dataflash_flash_dev_config_t *config;
    cyg_dataflash_flash_dev_priv_t   *priv;
    cyg_uint32 page;
 
    config = (cyg_dataflash_flash_dev_config_t *) dev->config;
    priv   = (cyg_dataflash_flash_dev_priv_t *) dev->priv;
    
    page = priv->start_page + 
        ((base - config->address) >> priv->page_2size_log); 
 
//    diag_printf("\nDF PROGRAM base = %x page = %d len = %d\n", base, page, len);

    cyg_dataflash_aquire(&priv->dev);    
    cyg_dataflash_mem_to_buf(&priv->dev, 1, page, true);
    cyg_dataflash_write_buf(&priv->dev, 1, data, len, 0);
    cyg_dataflash_program_buf(&priv->dev, 1, page, true, true);    
    cyg_dataflash_release(&priv->dev);

    return CYG_FLASH_ERR_OK;
}

static int 
df_flash_read(struct cyg_flash_dev   *dev, 
              const  cyg_flashaddr_t  base, 
              void                   *data, 
              const size_t            len)
{
    cyg_dataflash_flash_dev_config_t *config;
    cyg_dataflash_flash_dev_priv_t   *priv;
    cyg_uint32 page;
 
    config = (cyg_dataflash_flash_dev_config_t *) dev->config;
    priv   = (cyg_dataflash_flash_dev_priv_t *) dev->priv;
    
    page = priv->start_page + 
        ((base - config->address) >> priv->page_2size_log); 
 
//    diag_printf("\nDF READ base = %x page = %d len = %d\n", base, page, len);

    cyg_dataflash_aquire(&priv->dev);    
    cyg_dataflash_mem_to_buf(&priv->dev, 1, page, true);
    cyg_dataflash_read_buf(&priv->dev, 1, data, len, 0);
    cyg_dataflash_release(&priv->dev);
    
    return CYG_FLASH_ERR_OK;
}

static int 
df_flash_hwr_map_error(struct cyg_flash_dev *dev, int err)
{
    return err;
}

static int 
df_flash_block_lock(struct cyg_flash_dev   *dev, 
                    const  cyg_flashaddr_t  block_base)
{
    return CYG_FLASH_ERR_INVALID;
}

static int 
df_flash_block_unlock(struct cyg_flash_dev  *dev, 
                      const cyg_flashaddr_t  block_base)
{
    return CYG_FLASH_ERR_INVALID;
}

// -------------------------------------------------------------------------- 

CYG_FLASH_FUNS(cyg_dataflash_flash_dev_funs,
               df_flash_init,
               df_flash_query,
               df_flash_erase_block,
               df_flash_program,
               df_flash_read,
               df_flash_hwr_map_error,
               df_flash_block_lock,
               df_flash_block_unlock
);

//----------------------------------------------------------------------------
// End of devs_flash_atmel_dataflash_flash_dev_funs.c
