//==========================================================================
//
//      devs_flash_atmel_dataflash.c 
//
//      Atmel DataFlash series flash driver
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
// Date:         2004-08-27
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/devs_flash_atmel_dataflash.h>

#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/spi.h>
#include <cyg/io/dataflash.h>

// -------------------------------------------------------------------------- 
// DataFlash command opcodes

// Read commands opcodes
#define DF_CONT_ARRAY_READ_CMD              0x68
#define DF_MMEM_PAGE_READ_CMD               0x52
#define DF_BUF1_READ_CMD                    0x54
#define DF_BUF2_READ_CMD                    0x56
#define DF_STATUS_READ_CMD                  0x57

// Program and erase commands opcodes
#define DF_BUF1_WRITE_CMD                   0x84
#define DF_BUF2_WRITE_CMD                   0x87
#define DF_BUF1_PROG_W_ERASE_CMD            0x83
#define DF_BUF2_PROG_W_ERASE_CMD            0x86
#define DF_BUF1_PROG_WO_ERASE_CMD           0x88
#define DF_BUF2_PROG_WO_ERASE_CMD           0x89
#define DF_PAGE_ERASE_CMD                   0x81
#define DF_BLOCK_ERASE_CMD                  0x50
#define DF_PROG_THROUGH_BUF1_CMD            0x82  
#define DF_PROG_THROUGH_BUF2_CMD            0x85

// Additional commands opcodes
#define DF_TRANSFER_TO_BUF1_CMD             0x53
#define DF_TRANSFER_TO_BUF2_CMD             0x55
#define DF_BUF1_COMPARE_CMD                 0x60
#define DF_BUF2_COMPARE_CMD                 0x61
#define DF_AUTO_REWRITE_THROUGH_BUF1_CMD    0x58
#define DF_AUTO_REWRITE_THROUGH_BUF2_CMD    0x59

//----------------------------------------------------------------------------

typedef struct df_status_s
{
    cyg_uint8 reserved:2;
    cyg_uint8 device_id:4;
    cyg_uint8 compare_err:1;
    cyg_uint8 ready:1;
} df_status_t;

//----------------------------------------------------------------------------

static const cyg_dataflash_dev_info_t df_dev_info[] =
{
    {   // AT45DB011B
        device_id:    0x03,
        page_size:    264,
        page_count:   512,
        baddr_bits:   9,  
        block_size:   8,
        sector_sizes: { 1, 31, 32 },          
        sector_count: 3
    },
    {   // AT45DB021B
        device_id:    0x05,
        page_size:    264,
        page_count:   1024, 
        baddr_bits:   9,
        block_size:   8,
        sector_sizes: { 1, 31, 32, 64 },
        sector_count: 4
    },
    {   // AT45DB041B
        device_id:    0x07,
        page_size:    264,
        page_count:   2048, 
        baddr_bits:   9,
        block_size:   8,
        sector_sizes: { 1, 31, 32, 64, 64, 64 },
        sector_count: 6
    },
    {   // AT45DB081B
        device_id:    0x09,
        page_size:    264,
        page_count:   4096, 
        baddr_bits:   9,
        block_size:   8,
        sector_sizes: { 1, 31, 32, 64, 64, 64, 64, 64, 64, 64 },
        sector_count: 10 
    },
    {   // AT45DB161B
        device_id:    0x0B,
        page_size:    528,
        page_count:   4096, 
        baddr_bits:   10,
        block_size:   8,
        sector_sizes: {  1, 31, 32, 32, 32, 32, 32, 32, 
                        32, 32, 32, 32, 32, 32, 32, 32, 32 },
        sector_count: 17 
    },
    {   // AT45DB321B
        device_id:    0x0D,
        page_size:    528,
        page_count:   8192,
        baddr_bits:   10,
        block_size:   8,
        sector_sizes: {  1, 63, 64, 64, 64, 64, 64, 64, 
                        64, 64, 64, 64, 64, 64, 64, 64, 64 },
        sector_count: 17 
    },
    {   // AT45DB642
        device_id:    0x0F,
        page_size:    1056,
        page_count:   8192, 
        baddr_bits:   11,
        block_size:   8,
        sector_sizes: {  1, 31, 32, 32, 32, 32, 32, 32,
                        32, 32, 32, 32, 32, 32, 32, 32,
                        32, 32, 32, 32, 32, 32, 32, 32,
                        32, 32, 32, 32, 32, 32, 32, 32, 32 },
        sector_count: 35
    },
    { 0 }
};

//----------------------------------------------------------------------------

static void
df_compose_addr(cyg_dataflash_device_t *dev,
                cyg_uint8              *cmd_buf, 
                cyg_uint16              page_addr, 
                cyg_uint16              byte_addr)
{
    cyg_uint32 baddr_bits = dev->info->baddr_bits;
    cyg_uint32 addr; 

    addr = (page_addr << baddr_bits) | (byte_addr & ((1<<baddr_bits) - 1));
    
    cmd_buf[1] = (addr >> 16) & 0xFF;
    cmd_buf[2] = (addr >>  8) & 0xFF;
    cmd_buf[3] = addr         & 0xFF;
}
        
static df_status_t
df_read_status(cyg_dataflash_device_t *dev)
{
    cyg_spi_device  *spi_dev    = dev->spi_dev;
    const cyg_uint8  cmd_buf[2] = { DF_STATUS_READ_CMD, 0 };
    cyg_uint8        rx_buf[2];
    df_status_t     *status;
   
    cyg_spi_transaction_transfer(spi_dev, true, 2, cmd_buf, rx_buf, true);

    status = (df_status_t *) &rx_buf[1];

    return *status;    
}

static void 
df_wait_ready(cyg_dataflash_device_t *dev)
{
    df_status_t status;
   
    do 
    {
        status = df_read_status(dev);
    } while (0 == status.ready); 
}

static void 
df_detect_device(cyg_dataflash_device_t *dev)
{
    const cyg_dataflash_dev_info_t *dev_info;
    cyg_spi_device                 *spi_dev = dev->spi_dev;
    df_status_t                     status;
    
    cyg_spi_transaction_begin(spi_dev);
    status = df_read_status(dev);
    cyg_spi_transaction_end(spi_dev);
   
    dev_info = df_dev_info;

    while (dev_info->device_id != 0)
    {
        if (status.device_id == dev_info->device_id)
        {
//            diag_printf("Detected DataFlash id = %d page_size=%d page_count=%d\n", 
//                        status.device_id, dev_info->page_size, dev_info->page_count); 

            dev->info = dev_info;
            return;
        }
        dev_info++;
    }
    dev->info = NULL;
}

//----------------------------------------------------------------------------
// cyg_dataflash_init()

cyg_bool 
cyg_dataflash_init(cyg_spi_device         *spi_dev, 
                   cyg_bool                polled,
                   cyg_dataflash_device_t *dev)
{
    dev->spi_dev = spi_dev;
    dev->polled  = polled;
    
    cyg_drv_mutex_init(&dev->lock);

    df_detect_device(dev);

    if (NULL == dev->info)
        return false;
    else
        return true;
}

//----------------------------------------------------------------------------
// cyg_dataflash_get_sector_start()

cyg_uint16
cyg_dataflash_get_sector_start(cyg_dataflash_device_t *dev, 
                               cyg_uint16              sector_num)
{
    cyg_uint16 res, i;
    
    if (sector_num >= dev->info->sector_count)
        return 0;

    res = 0;
    for (i = 0; i < sector_num; i++)
        res += dev->info->sector_sizes[i];

    return res;
}

//----------------------------------------------------------------------------
// cyg_dataflash_aquire()

void
cyg_dataflash_aquire(cyg_dataflash_device_t *dev)
{
    while (!cyg_drv_mutex_lock(&dev->lock));
}

//----------------------------------------------------------------------------
// cyg_dataflash_release()

void
cyg_dataflash_release(cyg_dataflash_device_t *dev)
{
    cyg_drv_mutex_unlock(&dev->lock);
}

//----------------------------------------------------------------------------
// cyg_dataflash_read_buf()

void
cyg_dataflash_read_buf(cyg_dataflash_device_t *dev,
                       cyg_uint8               buf_num,
                       cyg_uint8              *buf, 
                       cyg_uint32              len, 
                       cyg_uint32              pos) 
                       
{
    cyg_spi_device *spi_dev = dev->spi_dev;
    cyg_uint8       cmd_buf[5];

    pos %= dev->info->page_size;

    if (1 == buf_num)      cmd_buf[0] = DF_BUF1_READ_CMD;
    else if (2 == buf_num) cmd_buf[0] = DF_BUF2_READ_CMD;
    else 
        return;

    df_compose_addr(dev, cmd_buf, 0, pos);

    cyg_spi_transaction_begin(spi_dev);
    cyg_spi_transaction_transfer(spi_dev, true, 5, cmd_buf, NULL, false);
    cyg_spi_transaction_transfer(spi_dev, dev->polled, len, buf, buf, true);
    cyg_spi_transaction_end(spi_dev);
}

//----------------------------------------------------------------------------
// cyg_dataflash_write_buf()

void
cyg_dataflash_write_buf(cyg_dataflash_device_t *dev,
                        cyg_uint8               buf_num,
                        const cyg_uint8        *buf,
                        cyg_uint32              len,
                        cyg_uint32              pos)
{
    cyg_spi_device *spi_dev = dev->spi_dev;
    cyg_uint8       cmd_buf[4];

    pos %= dev->info->page_size;

    if (1 == buf_num)      cmd_buf[0] = DF_BUF1_WRITE_CMD;
    else if (2 == buf_num) cmd_buf[0] = DF_BUF2_WRITE_CMD;
    else 
        return;

    df_compose_addr(dev, cmd_buf, 0, pos);

    cyg_spi_transaction_begin(spi_dev);
    cyg_spi_transaction_transfer(spi_dev, true, 4, cmd_buf, NULL, false);
    cyg_spi_transaction_transfer(spi_dev, dev->polled, len, buf, NULL, true);
    cyg_spi_transaction_end(spi_dev);
}

//----------------------------------------------------------------------------
// cyg_dataflash_mem_to_buf()

void
cyg_dataflash_mem_to_buf(cyg_dataflash_device_t *dev,
                         cyg_uint8               buf_num,
                         cyg_uint32              page_num,
                         cyg_bool                wait) 
{
    cyg_spi_device *spi_dev = dev->spi_dev;
    cyg_uint8       cmd_buf[4];

    if (page_num >= dev->info->page_count)
        return;
    
    if (1 == buf_num)      cmd_buf[0] = DF_TRANSFER_TO_BUF1_CMD;
    else if (2 == buf_num) cmd_buf[0] = DF_TRANSFER_TO_BUF2_CMD;
    else 
        return;

    df_compose_addr(dev, cmd_buf, page_num, 0);

    cyg_spi_transaction_begin(spi_dev);

    df_wait_ready(dev);

    cyg_spi_transaction_transfer(spi_dev, true, 4, cmd_buf, NULL, true);

    if (wait)
        df_wait_ready(dev);

    cyg_spi_transaction_end(spi_dev);
}

//----------------------------------------------------------------------------
// cyg_dataflash_program_buf()

void
cyg_dataflash_program_buf(cyg_dataflash_device_t *dev,
                          cyg_uint8               buf_num,
                          cyg_uint32              page_num,
                          cyg_bool                erase,
                          cyg_bool                wait)
{
    cyg_spi_device *spi_dev = dev->spi_dev;
    cyg_uint8       cmd_buf[4];

    if (page_num >= dev->info->page_count)
        return;
    
    if (erase)
    {
        if (1 == buf_num)      cmd_buf[0] = DF_BUF1_PROG_W_ERASE_CMD;
        else if (2 == buf_num) cmd_buf[0] = DF_BUF2_PROG_W_ERASE_CMD; 
        else 
            return;
    }
    else
    {
        if (1 == buf_num)      cmd_buf[0] = DF_BUF1_PROG_WO_ERASE_CMD;
        else if (2 == buf_num) cmd_buf[0] = DF_BUF2_PROG_WO_ERASE_CMD; 
        else 
            return;
    }
    
    df_compose_addr(dev, cmd_buf, page_num, 0);

    cyg_spi_transaction_begin(spi_dev);

    df_wait_ready(dev);

    cyg_spi_transaction_transfer(spi_dev, true, 4, cmd_buf, NULL, true);
    
    if (wait)
        df_wait_ready(dev);

    cyg_spi_transaction_end(spi_dev);
}

//----------------------------------------------------------------------------
// cyg_dataflash_compare_buf()

cyg_bool
cyg_dataflash_compare_buf(cyg_dataflash_device_t *dev,
                          cyg_uint8               buf_num,
                          cyg_uint32              page_num)
{
    cyg_spi_device *spi_dev = dev->spi_dev;
    cyg_uint8       cmd_buf[4];
    df_status_t     status;

    if (page_num >= dev->info->page_count)
        return false;

    if (1 == buf_num)      cmd_buf[0] = DF_BUF1_COMPARE_CMD;
    else if (2 == buf_num) cmd_buf[0] = DF_BUF2_COMPARE_CMD; 
    else 
        return false;

    df_compose_addr(dev, cmd_buf, page_num, 0);

    cyg_spi_transaction_begin(spi_dev);

    df_wait_ready(dev);
    cyg_spi_transaction_transfer(spi_dev, true, 4, cmd_buf, NULL, true);
    df_wait_ready(dev);
    
    status = df_read_status(dev);

    cyg_spi_transaction_end(spi_dev);

    return (!status.compare_err);
}

//----------------------------------------------------------------------------
// cyg_dataflash_erase()

void
cyg_dataflash_erase(cyg_dataflash_device_t *dev,
                    cyg_uint32              page_num,
                    cyg_bool                wait)
{
    cyg_spi_device *spi_dev = dev->spi_dev;
    cyg_uint8       cmd_buf[4];

    if (page_num >= dev->info->page_count)
        return;

    cmd_buf[0] = DF_PAGE_ERASE_CMD;
    df_compose_addr(dev, cmd_buf, page_num, 0);

    cyg_spi_transaction_begin(spi_dev);

    df_wait_ready(dev);

    cyg_spi_transaction_transfer(spi_dev, true, 4, cmd_buf, NULL, true);

    if (wait)
        df_wait_ready(dev);
    
    cyg_spi_transaction_end(spi_dev);
}

//----------------------------------------------------------------------------
// cyg_dataflash_erase_block()

void
cyg_dataflash_erase_block(cyg_dataflash_device_t *dev,
                          cyg_uint32              block_num,
                          cyg_bool                wait)
{
    cyg_spi_device *spi_dev = dev->spi_dev;
    cyg_uint8       cmd_buf[4];

    if (block_num >= (dev->info->page_count >> 3))
        return;

    cmd_buf[0] = DF_BLOCK_ERASE_CMD;
    df_compose_addr(dev, cmd_buf, block_num << 3, 0);

    cyg_spi_transaction_begin(spi_dev);

    df_wait_ready(dev);

    cyg_spi_transaction_transfer(spi_dev, true, 4, cmd_buf, NULL, true);

    if (wait)
        df_wait_ready(dev);

    cyg_spi_transaction_end(spi_dev);
}

//----------------------------------------------------------------------------
// cyg_dataflash_auto_rewrite()

void
cyg_dataflash_auto_rewrite(cyg_dataflash_device_t *dev,
                           cyg_uint8               buf_num,
                           cyg_uint32              page_num,
                           cyg_bool                wait)
{
    cyg_spi_device *spi_dev = dev->spi_dev;
    cyg_uint8       cmd_buf[4];

    if (page_num >= dev->info->page_count)
        return;

    if (1 == buf_num)      cmd_buf[0] = DF_AUTO_REWRITE_THROUGH_BUF1_CMD;
    else if (2 == buf_num) cmd_buf[0] = DF_AUTO_REWRITE_THROUGH_BUF2_CMD; 
    else 
        return;

    df_compose_addr(dev, cmd_buf, page_num, 0);

    cyg_spi_transaction_begin(spi_dev);

    df_wait_ready(dev);

    cyg_spi_transaction_transfer(spi_dev, true, 4, cmd_buf, NULL, true);

    if (wait)
        df_wait_ready(dev);
    
    cyg_spi_transaction_end(spi_dev);
}

//----------------------------------------------------------------------------
// cyg_dataflash_read()

void
cyg_dataflash_read(cyg_dataflash_device_t *dev, 
                   cyg_uint8              *buf, 
                   cyg_uint32              len, 
                   cyg_uint32              pos)
{
    cyg_spi_device *spi_dev = dev->spi_dev;
    cyg_uint8       cmd_buf[8];
    cyg_uint32      page_num, page_pos;

    page_num = pos / dev->info->page_size;
    page_pos = pos % dev->info->page_size;

    cmd_buf[0] = DF_CONT_ARRAY_READ_CMD;
    df_compose_addr(dev, cmd_buf, page_num, page_pos);

    cyg_spi_transaction_begin(spi_dev);
    df_wait_ready(dev);
    cyg_spi_transaction_transfer(spi_dev, true, 8, cmd_buf, NULL, false);
    cyg_spi_transaction_transfer(spi_dev, dev->polled, len, buf, buf, true);
    cyg_spi_transaction_end(spi_dev);
}

//----------------------------------------------------------------------------
// cyg_dataflash_program()

cyg_bool
cyg_dataflash_program(cyg_dataflash_device_t *dev, 
                      const cyg_uint8        *buf, 
                      cyg_uint32             *len, 
                      cyg_uint32              pos,
                      cyg_bool                erase,
                      cyg_bool                verify)
{
    cyg_spi_device *spi_dev = dev->spi_dev;
    cyg_uint8       cmd_buf[4];
    cyg_uint32      count, page_num, page_pos;
    cyg_bool        res = true;
    
    page_num = pos / dev->info->page_size;
    page_pos = pos % dev->info->page_size;
    count    = *len;
    
//    diag_printf("\nWrite start at page=%d pos=%d size=%d\n", page_num, page_pos, *len);

    cyg_spi_transaction_begin(spi_dev);

    df_wait_ready(dev);

    while (count > 0)
    { 
        df_status_t status;
        cyg_uint32  size;

        if ((page_pos + count) > dev->info->page_size)
            size = dev->info->page_size - page_pos;
        else
            size = count;
        
//        diag_printf("Writing page=%d size=%d [%02X]\n", page_num, size, *buf);
    
        df_compose_addr(dev, cmd_buf, page_num, page_pos);
        
        if (page_pos > 0 || size < dev->info->page_size)
        {
            cmd_buf[0] = DF_TRANSFER_TO_BUF1_CMD;
            cyg_spi_transaction_transfer(spi_dev, true, 4, cmd_buf, NULL, true);
            df_wait_ready(dev);
        }
        
        cmd_buf[0] = DF_BUF1_WRITE_CMD;
        cyg_spi_transaction_transfer(spi_dev, true, 4, cmd_buf, NULL, false);
        cyg_spi_transaction_transfer(spi_dev, dev->polled, size, buf, NULL, true);
        df_wait_ready(dev);

        if (erase)
            cmd_buf[0] = DF_BUF1_PROG_W_ERASE_CMD;
        else
            cmd_buf[0] = DF_BUF1_PROG_WO_ERASE_CMD;

        cyg_spi_transaction_transfer(spi_dev, true, 4, cmd_buf, NULL, true);
        df_wait_ready(dev);
   
        if (verify)
        { 
            cmd_buf[0] = DF_BUF1_COMPARE_CMD;
            cyg_spi_transaction_transfer(spi_dev, true, 4, cmd_buf, NULL, true);
            df_wait_ready(dev);
    
            status = df_read_status(dev);

            if (status.compare_err)
            {
                res = false;
                break;
            }
        }

        page_pos  = 0;
        page_num += 1;
        count    -= size;
        buf      += size;
    }

    cyg_spi_transaction_end(spi_dev);

    *len -= count;
    
    return res; 
}

//----------------------------------------------------------------------------
// End of devs_flash_atmel_dataflash.c
