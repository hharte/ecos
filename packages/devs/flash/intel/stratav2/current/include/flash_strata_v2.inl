#ifndef CYGONCE_DEVS_FLASH_STRATA_V2_INL
#define CYGONCE_DEVS_FLASH_STRATA_V2_INL
//==========================================================================
//
//      flash_strata.inl
//
//      Flash programming for the Strata device
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// Author(s):    gthomas, hmt
// Contributors: gthomas, asl
// Date:         2001-02-14
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/devs_flash_strata_v2.h>
#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>

#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>
#include <cyg/io/strata_v2_priv.h>

#define _si(p) ((p[1]<<8)|p[0])

#ifdef CYGOPT_DEV_FLASH_STATE_V2_DUMP_UNKNOWN
extern void diag_dump_buf(void *buf, CYG_ADDRWORD len);
#endif
extern int strncmp(const char *s1, const char *s2, size_t len);
extern void *memcpy( void *, const void *, size_t );

// Platforms may define these for special handling when accessing the
// query data or write buffer .
#ifndef CYGHWR_FLASH_READ_QUERY 
#define CYGHWR_FLASH_READ_QUERY(a) (*(a)) 
#endif
#ifndef CYGHWR_FLASH_WRITE_BUF
#define CYGHWR_FLASH_WRITE_BUF(a,b) (*(a) = *(b))
#endif



//----------------------------------------------------------------------------
// Functions that put the flash device into non-read mode must reside
// in RAM.
static size_t
strata_query_hwr (struct cyg_flash_dev *dev, void * data,
                  const size_t len)
     __attribute__ ((section (".2ram.flash_query")));

static int strata_erase_block (struct cyg_flash_dev *dev,
                               const cyg_flashaddr_t block)
     __attribute__ ((section (".2ram.flash_erase_block")));

static int strata_program_buf (struct cyg_flash_dev *dev,
                               cyg_flashaddr_t base,
                               const void* data,
                               const size_t len)
     __attribute__ ((section (".2ram.flash_program_buf")));

#ifdef CYGOPT_DEVS_FLASH_STRATA_V2_LOCKING
static int strata_unlock_block(struct cyg_flash_dev *dev,
                                     const cyg_flashaddr_t block_base)
     __attribute__ ((section (".2ram.flash_unlock_block")));

static int strata_lock_block(struct cyg_flash_dev *dev,
                                     const cyg_flashaddr_t block_base)
     __attribute__ ((section (".2ram.flash_lock_block")));
#endif

//----------------------------------------------------------------------------
// Private structure used by the device.
struct cyg_flash_strata_v2_priv {
    cyg_block_info_t block_info [1];
    unsigned int     buffer_size;    // Size of write buffer
};
//----------------------------------------------------------------------------

static int
strata_init (struct cyg_flash_dev *dev)
{
    struct FLASH_query data, *qp;
    int num_regions, region_size, buffer_size;
    struct cyg_flash_strata_v2_priv *priv = dev->priv;

    dev->funs->flash_query(dev, &data, sizeof(data));
    qp = &data;
    if ( ((qp->manuf_code == FLASH_Intel_code) || 
          (qp->manuf_code == FLASH_STMicro_code))
#ifdef CYGOPT_FLASH_IS_BOOTBLOCK
         // device types go as follows: 0x90 for 16-bits, 0xD0 for 8-bits,
         // plus 0 or 1 for -T (Top Boot) or -B (Bottom Boot)
         //     [FIXME: whatever that means :FIXME]
         // [I think it means the boot blocks are top/bottom of addr space]
         // plus the following size codes:
         //    0: 16Mbit    2:  8Mbit    4:  4Mbit
         //    6: 32Mbit    8: 64Mbit
#if 16 == CYGNUM_FLASH_WIDTH         
         && (0x90 == (0xF0 & qp->device_code)) // 16-bit devices
#elif 8 == CYGNUM_FLASH_WIDTH
         && (0xD0 == (0xF0 & qp->device_code)) // 8-bit devices
#else
         && 0
#error Only understand 16 and 8-bit bootblock flash types
#endif
        ) {
        int lookup[] = { 16, 8, 4, 32, 64 };
#define BLOCKSIZE (0x10000)
        region_size = BLOCKSIZE;
        num_regions = qp->device_code & 0x0F;
        num_regions >>= 1;
        if ( num_regions > 4 )
            goto flash_type_unknown;
        num_regions = lookup[num_regions];
        num_regions *= 1024 * 1024;     // to bits
        num_regions /= 8;               // to bytes
        num_regions /= BLOCKSIZE;       // to blocks
        buffer_size = 0;
#else // CYGOPT_FLASH_IS_BOOTBLOCK
         && (strncmp(qp->id, "QRY", 3) == 0)) {
        num_regions = _si(qp->num_regions)+1;
        region_size = _si(qp->region_size)*256;       
        if (_si(qp->buffer_size)) {
            buffer_size = CYGNUM_FLASH_INTERLEAVE << _si(qp->buffer_size);
        } else {
            buffer_size = 0;
        }
#endif // Not CYGOPT_FLASH_IS_BOOTBLOCK
        
        dev->end = dev->start +
          (num_regions*region_size*CYGNUM_FLASH_INTERLEAVE) -1;
        priv->buffer_size = buffer_size;
        priv->block_info[0].blocks = num_regions;
        priv->block_info[0].block_size = region_size * CYGNUM_FLASH_INTERLEAVE;
        dev->block_info = priv->block_info;
        dev->num_block_infos = 1;

#ifdef CYGNUM_FLASH_BASE_MASK
        // Then this gives us a maximum size for the (visible) device.
        // This is to cope with oversize devices fitted, with some high
        // address lines ignored.
        if ( (dev->start & CYGNUM_FLASH_BASE_MASK) !=
             ((dev->end - 1) & CYGNUM_FLASH_BASE_MASK ) ) {
            // then the size of the device appears to span >1 device-worth!
            unsigned int x;
            x = (~(CYGNUM_FLASH_BASE_MASK)) + 1; // expected device size
            x += (unsigned int)dev->start;
            if ( x < (unsigned int)dev->end ) { // 2nd sanity check
                (dev->pf)("\nFLASH: Oversized device!  End addr %p changed to %p\n",
                       dev->end, (void *)x );
                dev->end = x;
                // Also adjust the block count else unlock crashes!
                x = ((cyg_uint8 *)dev->end - (cyg_uint8 *)dev->start)
                    / priv->block_info[0].block_size;
                priv->block_info[0].blocks = x;
            }
        }
#endif // CYGNUM_FLASH_BASE_MASK

        return CYG_FLASH_ERR_OK;
    }
#ifdef CYGOPT_FLASH_IS_BOOTBLOCK
 flash_type_unknown:
#endif
#ifdef CYGOPT_DEV_FLASH_STATE_V2_DUMP_UNKNOWN
    (dev->pf)("Can't identify FLASH, sorry, man %x, dev %x, id [%4s] \n",
           qp->manuf_code, qp->device_code, qp->id );
    diag_dump_buf(qp, sizeof(data));
#endif
    return CYG_FLASH_ERR_HWR;
}

//----------------------------------------------------------------------------
// Map a hardware status to a package error
static int strata_hwr_map_error (struct cyg_flash_dev *dev, int err)
{
    if (err & FLASH_ErrorMask) {
        (dev->pf)("Err = %x\n", err);
        if (err & FLASH_ErrorProgram)
            return CYG_FLASH_ERR_PROGRAM;
        else if (err & FLASH_ErrorErase)
            return CYG_FLASH_ERR_ERASE;
        else 
            return CYG_FLASH_ERR_HWR;  // FIXME
    } else {
        return CYG_FLASH_ERR_OK;
    }
}

//----------------------------------------------------------------------------
#define CNT 20*1000*10  // Approx 20ms

static size_t
strata_query_hwr (struct cyg_flash_dev *dev, void *data_dst, const size_t len)
{
    volatile flash_t *ROM;
    int i, cnt;
    unsigned char *data = (unsigned char *)data_dst;
    
    // Get base address and map addresses to virtual addresses
    ROM = FLASH_P2V( dev->start );
#ifdef CYGOPT_FLASH_IS_BOOTBLOCK
    // BootBlock flash does not support full Read_Query - we have do a
    // table oriented thing above, after getting just two bytes of results:
    ROM[0] = FLASH_Read_ID;
    i = 2;
#else
    // StrataFlash supports the full Read_Query op:
    ROM[0] = FLASH_Read_Query;
    i = sizeof(struct FLASH_query);
#endif // Not CYGOPT_FLASH_IS_BOOTBLOCK

    for (cnt = CNT;  cnt > 0;  cnt--) ;
    for ( /* i */;  i > 0;  i--, ++ROM ) {
        // It is very deliberate that data is chars NOT flash_t:
        // The info comes out in bytes regardless of device.
        *data++ = (unsigned char) CYGHWR_FLASH_READ_QUERY(ROM);
#ifndef CYGOPT_FLASH_IS_BOOTBLOCK
# if  8 == CYGNUM_FLASH_WIDTH
	// strata flash with 'byte-enable' contains the configuration data
	// at even addresses
	++ROM;
# endif
#endif
    }
    ROM[0] = FLASH_Reset;

    return 0;
}


//----------------------------------------------------------------------------
static int strata_erase_block (struct cyg_flash_dev *dev,
                               const cyg_flashaddr_t block_base)
{
    volatile flash_t *ROM;
    flash_t stat = 0;
    int timeout = 50000;
    int len, block_len, erase_block_size;
    volatile flash_t *eb, *block;

    // Get base address and map addresses to virtual addresses
    ROM = FLASH_P2V(dev->start);
    eb = block = FLASH_P2V(block_base);
    block_len = dev->block_info[0].block_size;

#ifdef CYGOPT_FLASH_IS_BOOTBLOCK
#define BLOCKSIZE (0x10000*CYGNUM_FLASH_INTERLEAVE)
#define ERASE_BLOCKSIZE (0x2000*CYGNUM_FLASH_INTERLEAVE)
    if ((eb - ROM) < BLOCKSIZE/(sizeof eb[0])) {
        erase_block_size = ERASE_BLOCKSIZE;
    } else {
        erase_block_size = block_size;
    }
#else
    erase_block_size = dev->block_info[0].block_size;
#endif

    // Clear any error conditions
    ROM[0] = FLASH_Clear_Status;

    // Erase block
    while (block_len > 0) {
        eb[0] = FLASH_Block_Erase;
        eb[0] = FLASH_Confirm;
	
        timeout = 5000000;
        while(((stat = eb[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
            if (--timeout == 0) break;
        }

        block_len -= erase_block_size;
        eb = FLASH_P2V((unsigned int)eb + erase_block_size);
    }

    // Restore ROM to "normal" mode
    ROM[0] = FLASH_Reset;

    // If an error was reported, see if the block erased anyway
    if (stat & FLASH_ErrorMask ) {
        len = dev->block_info[0].block_size;
        while (len > 0) {
            if (*block++ != FLASH_BlankValue ) break;
            len -= sizeof(*block);
        }
        if (len == 0) stat = 0;
    }

    return stat;
}

//----------------------------------------------------------------------------
static int
strata_program_buf (struct cyg_flash_dev *dev,
                    cyg_flashaddr_t base_addr,
                    const void* data_addr,
                    const size_t total_len)
{
    struct cyg_flash_strata_v2_priv *priv = dev->priv;
    volatile flash_t *ROM;
    volatile flash_t *BA, *addr;
    flash_t * data = (flash_t *)data_addr;
    flash_t stat = 0;
    int timeout = 50000;
    int len = total_len;
    
#ifdef FLASH_Write_Buffer
    int i, wc;
#endif
    
    // Get base address and map addresses to virtual addresses
    ROM = FLASH_P2V( dev->start );
    BA = addr = FLASH_P2V(base_addr);

    // Clear any error conditions
    ROM[0] = FLASH_Clear_Status;

#ifdef FLASH_Write_Buffer
    // Write any big chunks first
    while (len >= priv->buffer_size) {
        wc = priv->buffer_size;
        if (wc > len) wc = len;
        len -= wc;
	// convert 'wc' in bytes to 'wc' in 'flash_t' 
        wc = wc / sizeof(flash_t);  // Word count
        *BA = FLASH_Write_Buffer;
        timeout = 5000000;
        while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
            if (--timeout == 0) {
                goto bad;
            }
            *BA = FLASH_Write_Buffer;
        }
        *BA = FLASHWORD(wc-1);  // Count is 0..N-1
        for (i = 0;  i < wc;  i++) {
#ifdef CYGHWR_FLASH_WRITE_ELEM
            CYGHWR_FLASH_WRITE_ELEM(addr+i, data+i);
#else
            CYGHWR_FLASH_WRITE_BUF(addr+i, data+i);
#endif
        }
        *BA = FLASH_Confirm;
    
        ROM[0] = FLASH_Read_Status;
        timeout = 5000000;
        while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
            if (--timeout == 0) {
                goto bad;
            }
        }
        // Jump out if there was an error
        if (stat & FLASH_ErrorMask) {
            goto bad;
        }
        // And verify the data - also increments the pointers.
        *BA = FLASH_Reset;            
        for (i = 0;  i < wc;  i++) {
            if ( *addr++ != *data++ ) {
                stat = FLASH_ErrorNotVerified;
                goto bad;
            }
        }
    }
#endif

    while (len > 0) {
        BA[0] = FLASH_Program;
#ifdef CYGHWR_FLASH_WRITE_ELEM
        CYGHWR_FLASH_WRITE_ELEM(addr, data);
#else
        *addr = *data;
#endif
        timeout = 5000000;
        while(((stat = BA[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
            if (--timeout == 0) {
                goto bad;
            }
        }
        if (stat & FLASH_ErrorMask) {
            break;
        }
        BA[0] = FLASH_Reset;            
        if (*addr++ != *data++) {
            stat = FLASH_ErrorNotVerified;
            break;
        }
        len -= sizeof( flash_t );
    }

    // Restore ROM to "normal" mode
 bad:
    BA[0] = FLASH_Reset;            

    return stat;
}

#ifdef CYGOPT_DEVS_FLASH_STRATA_V2_LOCKING
//----------------------------------------------------------------------------
//
// The difficulty with this operation is that the hardware does not support
// unlocking single blocks.  However, the logical layer would like this to
// be the case, so this routine emulates it.  The hardware can clear all of
// the locks in the device at once.  This routine will use that approach and
// then reset the regions which are known to be locked.
//

#define FLASH_LOCK_MASK 0x1    // which bit of the read query has the lock bit

static int
strata_unlock_block(struct cyg_flash_dev *dev,
                          const cyg_flashaddr_t block_base)
{
    volatile flash_t *ROM;
    flash_t stat;
    int timeout = 5000000;
#ifndef CYGOPT_FLASH_IS_SYNCHRONOUS
    int i;
    volatile flash_t *bp, *bpv, *block;
    unsigned char is_locked[CYGNUM_DEVS_FLASH_STRATA_V2_MAX_BLOCKS];
#endif

    // Get base address and map addresses to virtual addresses
    ROM = FLASH_P2V( dev->start );
    block = FLASH_P2V(block_base);

    // Clear any error conditions
    ROM[0] = FLASH_Clear_Status;

#ifdef CYGOPT_FLASH_IS_SYNCHRONOUS
    // Clear lock bit
    block[0] = FLASH_Clear_Locks;
    block[0] = FLASH_Clear_Locks_Confirm;  // Confirmation
    while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
        if (--timeout == 0) break;
    }
#else
    // Get current block lock state.  This needs to access each block on
    // the device so currently locked blocks can be re-locked.
    bp = ROM;
    for (i = 0;  i < dev->block_info[0].blocks;  i++) {
        bpv = FLASH_P2V( bp );
        *bpv = FLASH_Read_Query;
        if (bpv == block) {
            is_locked[i] = 0;
        } else {
#if 8 == CYGNUM_FLASH_WIDTH
            is_locked[i] = bpv[4] & FLASH_LOCK_MASK;
#else
            is_locked[i] = bpv[2] & FLASH_LOCK_MASK;
# endif
        }
        bp += dev->block_info[0].block_size / sizeof(*bp);
    }

    // Clears all lock bits
    ROM[0] = FLASH_Clear_Locks;
    ROM[0] = FLASH_Clear_Locks_Confirm;  // Confirmation
    timeout = 5000000;
    while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
        if (--timeout == 0) break;
    }

    // Restore the lock state
    bp = ROM;
    for (i = 0;  i < dev->block_info[0].blocks;  i++) {
        bpv = FLASH_P2V( bp );
        if (is_locked[i]) {
            *bpv = FLASH_Set_Lock;
            *bpv = FLASH_Set_Lock_Confirm;  // Confirmation
            timeout = 5000000;
            while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
                if (--timeout == 0) break;
            }
        }
        bp += dev->block_info[0].block_size / sizeof(*bp);
    }
#endif  // CYGOPT_FLASH_IS_SYNCHRONOUS

    // Restore ROM to "normal" mode
    ROM[0] = FLASH_Reset;

    return stat;
}

//----------------------------------------------------------------------------
static int
strata_lock_block(struct cyg_flash_dev *dev,
                        const cyg_flashaddr_t block_base)
{
    volatile flash_t *ROM;
    flash_t stat;
    int timeout = 5000000;
    volatile flash_t *block;       

    // Get base address and map addresses to virtual addresses
    ROM = FLASH_P2V(dev->start);
    block = FLASH_P2V(block_base);

    // Clear any error conditions
    ROM[0] = FLASH_Clear_Status;

    // Set lock bit
    block[0] = FLASH_Set_Lock;
    block[0] = FLASH_Set_Lock_Confirm;  // Confirmation
    while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
        if (--timeout == 0) break;
    }

    // Restore ROM to "normal" mode
    ROM[0] = FLASH_Reset;

    return stat;
}
#endif

//----------------------------------------------------------------------------

#ifdef CYGOPT_DEVS_FLASH_STRATA_V2_LOCKING
static CYG_FLASH_FUNS (cyg_flash_strata_v2_funs,
                strata_init,
                strata_query_hwr,
                strata_erase_block,
                strata_program_buf,
                NULL,
                strata_hwr_map_error,
                strata_lock_block,
                strata_unlock_block);
#else
static CYG_FLASH_FUNS (cyg_flash_strata_v2_funs,
                strata_init,
                strata_query_hwr,
                strata_erase_block,
                strata_program_buf,
                NULL,
                strata_hwr_map_error,
                NULL,
                NULL);
#endif
#endif //CYGONCE_DEVS_FLASH_STRATA_V2_INL
// EOF strata.c
