//==========================================================================
//
//      am29xxxxx_aux.c
//
//      Flash driver for the AMD family - implementation. 
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2004 eCosCentric Ltd
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    bartv
// Contributors:
// Date:         2004-11-05
//              
//####DESCRIPTIONEND####
//
//==========================================================================

// This file is #include'd multiple times from the main am29xxxxx.c file,
// It serves to instantiate the various hardware operations in ways
// appropriate for all the bus configurations.

// The following macros are used to construct suitable function names
// for the current bus configuration. AM29_SUFFIX is #define'd before
// each #include of am29xxxxx_aux.c

#ifndef AM29_STR
# define AM29_STR1(_a_) # _a_
# define AM29_STR(_a_) AM29_STR1(_a_)
# define AM29_CONCAT3_AUX(_a_, _b_, _c_) _a_##_b_##_c_
# define AM29_CONCAT3(_a_, _b_, _c_) AM29_CONCAT3_AUX(_a_, _b_, _c_)
#endif

#define AM29_FNNAME(_base_) AM29_CONCAT3(_base_, _,  AM29_SUFFIX)

// Similarly construct a forward declaration, placing the function in
// the .2ram section. Each function must still be in a separate section
// for linker garbage collection.

# define AM29_RAMFNDECL(_base_, _args_) \
  AM29_FNNAME(_base_) _args_ __attribute__((section (".2ram." AM29_STR(_base_) "_" AM29_STR(AM29_SUFFIX))))

// Calculate the various offsets, based on the device count.
// The main code may override these settings for specific
// configurations, e.g. 16as8
#ifndef AM29_OFFSET_COMMAND
# define AM29_OFFSET_COMMAND    0x0555
#endif
#ifndef AM29_OFFSET_COMMAND2
# define AM29_OFFSET_COMMAND2   0x02AA
#endif
#ifndef AM29_OFFSET_DEVID
# define AM29_OFFSET_DEVID      0x0001
#endif
#ifndef AM29_OFFSET_DEVID2
# define AM29_OFFSET_DEVID2     0x000E
#endif
#ifndef AM29_OFFSET_DEVID3
# define AM29_OFFSET_DEVID3     0x000F
#endif
#ifndef AM29_OFFSET_CFI
# define AM29_OFFSET_CFI        0x0055
#endif
#ifndef AM29_OFFSET_CFI_DATA
# define AM29_OFFSET_CFI_DATA(_idx_) _idx_
#endif

// For parallel operation commands are issued in parallel and status
// bits are checked in parallel.
#ifndef AM29_PARALLEL
# define AM29_PARALLEL(_cmd_)    (_cmd_)
#endif

// ----------------------------------------------------------------------------
// When performing the various low-level operations like erase the flash
// chip can no longer support ordinary data reads. Obviously this is a
// problem if the current code is executing out of flash. The solution is
// to store the key functions in RAM rather than flash, via a special
// linker section .2ram which usually gets placed in the same area as
// .data.
//
// In a ROM startup application anything in .2ram will consume space
// in both the flash and RAM. Hence it is desirable to keep the .2ram
// functions as small as possible, responsible only for the actual
// hardware manipulation.
//
// All these .2ram functions should be invoked with interrupts
// disabled. Depending on the hardware it may also be necessary to
// have the data cache disabled. The .2ram functions must be
// self-contained, even macro invocations like HAL_DELAY_US() are
// banned because on some platforms those could be implemented as
// function calls.

// gcc requires forward declarations with the attributes, then the actual
// definitions.
static int  AM29_RAMFNDECL(am29_hw_query, (volatile AM29_TYPE*));
static int  AM29_RAMFNDECL(am29_hw_cfi, (struct cyg_flash_dev*, cyg_am29xxxxx_dev*, volatile AM29_TYPE*));
static void AM29_RAMFNDECL(am29_hw_erase, (volatile AM29_TYPE*));
static void AM29_RAMFNDECL(am29_hw_program, (volatile AM29_TYPE*, volatile AM29_TYPE*, const cyg_uint8*, cyg_uint32 count));

// Read the device id. This involves a straightforward command
// sequence, followed by a reset to get back into array mode.
// All chips are accessed in parallel, but only the response
// from the least significant is used.
static int
AM29_FNNAME(am29_hw_query)(volatile AM29_TYPE* addr)
{
    int devid;
    
    addr[AM29_OFFSET_COMMAND]   = AM29_COMMAND_SETUP1;
    addr[AM29_OFFSET_COMMAND2]  = AM29_COMMAND_SETUP2;
    addr[AM29_OFFSET_COMMAND]   = AM29_COMMAND_AUTOSELECT;
    devid                       = addr[AM29_OFFSET_DEVID] & 0x00FF;

    // The original AMD chips only used a single-byte device id, but
    // all codes have now been used up. Newer devices use a 3-byte
    // devid. The above devid read will have returned 0x007E. The
    // test allows for boards with a mixture of old and new chips.
    // The amount of code involved is too small to warrant a config
    // option.
    if (0x007E == devid) {
        devid <<= 16;
        devid  |= ((addr[AM29_OFFSET_DEVID2] & 0x00FF) << 8);
        devid  |=  (addr[AM29_OFFSET_DEVID3] & 0x00FF);
    }
    addr[AM29_OFFSET_COMMAND]   = AM29_COMMAND_RESET;
    return devid;
}

// Perform a CFI query. This involves placing the device(s) into CFI
// mode, checking that this has really happened, and then reading the
// size and block info. The address corresponds to the start of the
// flash.
static int
AM29_FNNAME(am29_hw_cfi)(struct cyg_flash_dev* dev, cyg_am29xxxxx_dev* am29_dev, volatile AM29_TYPE* addr)
{
    int     dev_size;
    int     i;
    int     erase_regions;

    // Just a single write is needed to put the device into CFI mode
    addr[AM29_OFFSET_CFI]   = AM29_COMMAND_CFI;
    // Now check that we really are in CFI mode. There should be a 'Q'
    // at a specific address. This test is not 100% reliable, but should
    // be good enough.
    if ('Q' != (addr[AM29_OFFSET_CFI_Q] & 0x00FF)) {
        addr[AM29_OFFSET_COMMAND]   = AM29_COMMAND_RESET;
        return CYG_FLASH_ERR_PROTOCOL;
    }
    // Device sizes are always a power of 2, and the shift is encoded
    // in a single byte
    dev_size = 0x01 << (addr[AM29_OFFSET_CFI_SIZE] & 0x00FF);
    dev->end = dev->start + dev_size - 1;

    // The number of erase regions is also encoded in a single byte.
    // Usually this is no more than 4. A value of 0 indicates that
    // only chip erase is supported, but the driver does not cope
    // with that.
    erase_regions   = addr[AM29_OFFSET_CFI_BLOCK_REGIONS] & 0x00FF;
    if (erase_regions > CYGNUM_DEVS_FLASH_AMD_AM29XXXXX_V2_ERASE_REGIONS) {
        addr[AM29_OFFSET_COMMAND]   = AM29_COMMAND_RESET;
        return CYG_FLASH_ERR_PROTOCOL;
    }
    dev->num_block_infos    = erase_regions;

    for (i = 0; i < erase_regions; i++) {
        cyg_uint32 count, size;
        cyg_uint32 count_lsb   = addr[AM29_OFFSET_CFI_BLOCK_COUNT_LSB(i)] & 0x00FF;
        cyg_uint32 count_msb   = addr[AM29_OFFSET_CFI_BLOCK_COUNT_MSB(i)] & 0x00FF;
        cyg_uint32 size_lsb    = addr[AM29_OFFSET_CFI_BLOCK_SIZE_LSB(i)] & 0x00FF;
        cyg_uint32 size_msb    = addr[AM29_OFFSET_CFI_BLOCK_SIZE_MSB(i)] & 0x00FF;

        count = ((count_msb << 8) | count_lsb) + 1;
        size  = (size_msb << 16) | (size_lsb << 8);
        am29_dev->block_info[i].block_size  = (size_t) size * AM29_DEVCOUNT;
        am29_dev->block_info[i].blocks      = count;
    }

    // Get out of CFI mode
    addr[AM29_OFFSET_COMMAND]   = AM29_COMMAND_RESET;

    return CYG_FLASH_ERR_OK;
}

// Erase a single sector. There is no API support for chip-erase. The
// generic code operates one sector at a time, invoking the driver for
// each sector, so there is no opportunity inside the driver for
// erasing multiple sectors in a single call. The address argument
// points at the start of the sector.
static void
AM29_FNNAME(am29_hw_erase)(volatile AM29_TYPE* addr)
{
    int     retries = CYGNUM_DEVS_FLASH_AMD_AM29XXXXX_V2_ERASE_TIMEOUT;

    addr[AM29_OFFSET_COMMAND]   = AM29_COMMAND_SETUP1;
    addr[AM29_OFFSET_COMMAND2]  = AM29_COMMAND_SETUP2;
    addr[AM29_OFFSET_COMMAND]   = AM29_COMMAND_ERASE;
    addr[AM29_OFFSET_COMMAND]   = AM29_COMMAND_SETUP1;
    addr[AM29_OFFSET_COMMAND2]  = AM29_COMMAND_SETUP2;
    addr[AM29_OFFSET_COMMAND]   = AM29_COMMAND_ERASE_SECTOR;
    // There is now a 50us window in which we could send additional
    // ERASE_SECTOR commands, but the driver API does not allow this

    // All chips are now erasing in parallel. Loop until all have
    // completed. This can be detected in a number of ways. The DQ7
    // bit will be 0 until the erase is complete, but there is a
    // problem if something went wrong (e.g. the sector is locked),
    // the erase has not actually started, and the relevant bit was 0
    // already. More useful is DQ6. This will toggle during the 50us
    // window and while the erase is in progress, then stop toggling.
    // If the erase does not actually start then the bit won't toggle
    // at all so the operation completes rather quickly.
    //
    // If at any time DQ5 is set (indicating a timeout inside the
    // chip) then a reset command must be issued and the erase is
    // aborted. It is not clear this can actually happen during an
    // erase, but just in case.
    do {
        AM29_TYPE   datum1, datum2;
        datum1  = addr[AM29_OFFSET_COMMAND];
        datum2  = addr[AM29_OFFSET_COMMAND];
        if ((datum1 & AM29_STATUS_DQ6) == (datum2 & AM29_STATUS_DQ6)) {
            // The bits have stopped toggling, so finished.
            break;
        }
        // If DQ6 toggled, then check if DQ5 was set in datum1
        // (not datum2 as that may indicate a successful 0->1 transition
        // which can happen for one part of parallel devices before they
        // all complete the erase)
        if ((((datum1 ^ datum2) & AM29_STATUS_DQ6) >> 1) & datum1) {
            // Hardware error. The calling code will always verify
            // that the erase really was successful, so we don't need
            // to distinguish
            addr[AM29_OFFSET_COMMAND] = AM29_COMMAND_RESET;
            break;
        }
    } while (retries-- > 0);

    // The calling code will verify that the erase was successful,
    // and generate an error code.
}

// Write data to flash. At most one block will be processed at a time,
// but the write may be for a subset of the write. The destination
// address will be aligned in a way suitable for the bus. The source
// address need not be aligned. The count is in AM29_TYPE's, i.e.
// as per the bus alignment, not in bytes.
static void
AM29_FNNAME(am29_hw_program)(volatile AM29_TYPE* block_start, volatile AM29_TYPE* addr, const cyg_uint8* buf, cyg_uint32 count)
{
    int     retries;
    int     i;
    
    for (i = 0; i < count; i++) {
        AM29_TYPE   datum;
        AM29_TYPE   current, current2, masked_datum;
        
        // We can only clear bits, not set them, so any bits that were
        // already clear need to be preserved.
        current = addr[i];
        datum   = AM29_NEXT_DATUM(buf) & current;
        if (datum == current) {
            // No change, so just move on.
            continue;
        }
        
        block_start[AM29_OFFSET_COMMAND]    = AM29_COMMAND_SETUP1;
        block_start[AM29_OFFSET_COMMAND2]   = AM29_COMMAND_SETUP2;
        block_start[AM29_OFFSET_COMMAND]    = AM29_COMMAND_PROGRAM;
        addr[i] = datum;

        // The data is now being written. While the write is in progress
        // DQ7 will have an inverted value from what was written, so we
        // can poll, comparing just this bit. Again, if DQ5 is set then
        // an error has occurred.
        masked_datum = datum & AM29_STATUS_DQ7;
        retries = CYGNUM_DEVS_FLASH_AMD_AM29XXXXX_V2_PROGRAM_TIMEOUT;
        do {
            current = addr[i];
            if ((current & AM29_STATUS_DQ7) == masked_datum) {
                break;
            }
            if (0 != (current & AM29_STATUS_DQ5)) {
                // It's possible that one device can finish before
                // another. To deal with this we look at the DQ6
                // toggle bit, and only consider this to be an error
                // if it is still toggling for the device that's
                // reporting DQ5 set. This is similar to the checking
                // for erase timeouts above. This is unnecessary
                // before DQ5 gets set, so we don't do the double read
                // all the time.
                current2 = addr[i];
                if ((((current ^ current2) & AM29_STATUS_DQ6) >> 1) & current) {
                    // A timeout has occurred inside the hardware and
                    // the system is in a strange state. Reset but don't
                    // try to write any more of the data.
                    block_start[AM29_OFFSET_COMMAND]    = AM29_COMMAND_RESET;
                    return;
                }
            }
        } while (retries-- > 0);

        if (0 == retries) {
            // Failed to write this word, no point in trying to write the rest.
            return;
        }
    }
}

// FIXME: implement a separate program routine for buffered writes. 

// ----------------------------------------------------------------------------
// Exported code, mostly for placing in a cyg_flash_dev_funs structure.

// Just read the device id, either for sanity checking that the system
// has been configured for the right device, or for filling in the
// block info by a platform-specific init routine if the platform may
// be manufactured with one of several different chips.
int
AM29_FNNAME(cyg_am29xxxxx_read_devid) (struct cyg_flash_dev* dev)
{
    int                 (*query_fn)(volatile AM29_TYPE*);
    int                 devid;
    volatile AM29_TYPE* addr;

    CYG_CHECK_DATA_PTR(dev, "valid flash device pointer required");
    
    addr     = AM29_P2V(dev->start);
    query_fn = (int (*)(volatile AM29_TYPE*)) cyg_flash_anonymizer( & AM29_FNNAME(am29_hw_query) );
    devid    = (*query_fn)(addr);
    return devid;
}

// Validate that the device statically configured is the one on the
// board.
int
AM29_FNNAME(cyg_am29xxxxx_init_check_devid)(struct cyg_flash_dev* dev)
{
    cyg_am29xxxxx_dev*  am29_dev;
    int                 devid;

    am29_dev = (cyg_am29xxxxx_dev*) dev->priv;
    devid    = AM29_FNNAME(cyg_am29xxxxx_read_devid)(dev);
    if (devid != am29_dev->devid) {
        return CYG_FLASH_ERR_DRV_WRONG_PART;
    }
    // Successfully queried the device, and the id's match. That
    // should be a good enough indication that the flash is working.
    return CYG_FLASH_ERR_OK;
}

// Initialize via a CFI query, instead of statically specifying the
// boot block layout.
int
AM29_FNNAME(cyg_am29xxxxx_init_cfi)(struct cyg_flash_dev* dev)
{
    int                 (*cfi_fn)(struct cyg_flash_dev*, cyg_am29xxxxx_dev*, volatile AM29_TYPE*);
    volatile AM29_TYPE* addr;
    cyg_am29xxxxx_dev*  am29_dev;
    int                 result;
    
    CYG_CHECK_DATA_PTR(dev, "valid flash device pointer required");
    am29_dev    = (cyg_am29xxxxx_dev*) dev->priv;    // Remove const, only place where this is needed.
    addr        = AM29_P2V(dev->start);
    cfi_fn      = (int (*)(struct cyg_flash_dev*, cyg_am29xxxxx_dev*, volatile AM29_TYPE*))
        cyg_flash_anonymizer( & AM29_FNNAME(am29_hw_cfi));

    result      = (*cfi_fn)(dev, am29_dev, addr);

    // Now calculate the device size, and hence the end field.
    if (CYG_FLASH_ERR_OK == result) {
        int i;
        int size    = 0;
        for (i = 0; i < dev->num_block_infos; i++) {
            size += (dev->block_info[i].block_size * dev->block_info[i].blocks);
        }
        dev->end = dev->start + size - 1;
    }
    return result;
}

// Erase a single block. The calling code will have supplied a pointer
// aligned to a block boundary.
int
AM29_FNNAME(cyg_am29xxxxx_erase)(struct cyg_flash_dev* dev, cyg_flashaddr_t addr)
{
    void                (*erase_fn)(volatile AM29_TYPE*);
    volatile AM29_TYPE* block;
    cyg_flashaddr_t     block_start;
    size_t              block_size;
    int                 i;

    CYG_CHECK_DATA_PTR(dev, "valid flash device pointer required");
    CYG_ASSERT((addr >= dev->start) && (addr <= dev->end), "flash address out of device range");

    am29_get_block_info(dev, addr, &block_start, &block_size);
    CYG_ASSERT(addr == block_start, "erase address should be the start of a flash block");
    
    block       = AM29_P2V(addr);
    erase_fn    = (void (*)(volatile AM29_TYPE*)) cyg_flash_anonymizer( & AM29_FNNAME(am29_hw_erase) );
    (*erase_fn)(block);

    // The erase may have failed for a number of reasons, e.g. because
    // of a locked sector. The best thing to do here is to check that the
    // erase has succeeded.
    block = (AM29_TYPE*) addr;
    for (i = 0; i < (block_size / sizeof(AM29_TYPE)); i++) {
        if (block[i] != (AM29_TYPE)~0) {
            // There is no easy way of detecting the specific error,
            // e.g. locked flash block, timeout, ... So return a
            // useless catch-all error.
            return CYG_FLASH_ERR_ERASE;
        }
    }
    return CYG_FLASH_ERR_OK;
}

// Write some data to the flash. The destination must be aligned
// appropriately for the bus width (not the device width).
int
AM29_FNNAME(cyg_am29xxxxx_program)(struct cyg_flash_dev* dev, cyg_flashaddr_t dest, const void* src, size_t len)
{
    void                (*program_fn)(volatile AM29_TYPE*, volatile AM29_TYPE*, const cyg_uint8*, cyg_uint32);
    volatile AM29_TYPE* block;
    volatile AM29_TYPE* addr; 
    cyg_flashaddr_t     block_start;
    size_t              block_size;
    const cyg_uint8*    data;
    int                 i;

    CYG_CHECK_DATA_PTR(dev, "valid flash device pointer required");
    CYG_ASSERT((dest >= dev->start) && (dest <= dev->end), "flash address out of device range");

    // Only support writes that are aligned to the bus boundary. This
    // may be more restrictive than what the hardware is capable of.
    // However it ensures that the hw_program routine can write as
    // much data as possible each iteration, and hence significantly
    // improves performance. The length had better be a multiple of
    // the bus width as well
    if ((0 != ((CYG_ADDRWORD)dest & (sizeof(AM29_TYPE) - 1))) ||
        (0 != (len & (sizeof(AM29_TYPE) - 1)))) {
        return CYG_FLASH_ERR_INVALID;
    }

    am29_get_block_info(dev, dest, &block_start, &block_size);
    CYG_ASSERT(((dest - block_start) + len) <= block_size, "write cannot cross block boundary");
    
    block       = AM29_P2V(block_start);
    addr        = AM29_P2V(dest);
    data        = (const cyg_uint8*) src;

    program_fn  = (void (*)(volatile AM29_TYPE*, volatile AM29_TYPE*, const cyg_uint8*, cyg_uint32))
        cyg_flash_anonymizer( & AM29_FNNAME(am29_hw_program) );
    (*program_fn)(block, addr, (const cyg_uint8*)src, len / sizeof(AM29_TYPE));
    
    // Too many things can go wrong when manipulating the h/w, so
    // verify the operation by actually checking the data.
    addr = (volatile AM29_TYPE*) dest;
    for (i = 0; i < (len / sizeof(AM29_TYPE)); i++) {
        AM29_TYPE   datum   = AM29_NEXT_DATUM(data);
        AM29_TYPE   current = addr[i];
        if ((datum & current) != current) {
            return CYG_FLASH_ERR_PROGRAM;
        }
    }
    return CYG_FLASH_ERR_OK;
}

// ----------------------------------------------------------------------------
// Clean up the various #define's so this file can be #include'd again
#undef AM29_FNNAME
#undef AM29_RAMFNDECL
#undef AM29_OFFSET_COMMAND
#undef AM29_OFFSET_COMMAND2
#undef AM29_OFFSET_DEVID
#undef AM29_OFFSET_DEVID2
#undef AM29_OFFSET_DEVID3
#undef AM29_OFFSET_CFI
#undef AM29_OFFSET_CFI_DATA
#undef AM29_PARALLEL
