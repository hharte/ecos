//==========================================================================
//
//      am29xxxxx.c
//
//      Flash driver for the AMD family
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

#include <pkgconf/devs_flash_amd_am29xxxxx_v2.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/io/flash.h>
#include <cyg/io/flash_priv.h>
#include <cyg/io/am29xxxxx_dev.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_io.h>

// This driver supports multiple banks of AMD am29xxxxx flash devices
// or compatibles. These are NOR-flash devices, requiring explicit
// erase operations with an erase value of 0xff.
//
// The devices may be 8-bit, 16-bit, or 32-bit (64-bit devices are not
// yet supported). Most but not all 16-bit devices can also be
// accessed as 8-bit, in which case the chip may be hooked up to an
// 8-bit bus. A bank of flash may involve just a single chip, or there
// may be several chips in parallel. Typical combinations are 88 to
// get 16-bit, 8888 for 32-bit, and 1616 for 32-bit. It is assumed
// that all chips within a bank are the same device. There may also be
// several banks of flash, and different banks may use different
// devices.
//
// This driver instantiates support for the various bus
// configurations: 8, 16, 16AS8, 32, 88, 8888, and 1616. On any given
// platform only one or two of these combinations will be of interest,
// but the remainder will be eliminated via linker garbage collection.
// To avoid excessive duplication an auxiliary file contains the
// actual implementations. Compiler optimization should eliminate any
// unnecessary code.

// A flash driver is supposed to provide the following functions:
//  int     (*init)(...)
//  size_t  (*query)(...)
//  int     (*erase)(...)
//  int     (*program)(...)
//  int     (*hwr_map_error)(...)
//  int     (*block_lock)(...)
//  int     (*block_unlock)(...)
//
// The devices do not need any special initialization. However a given
// board may be manufactured with any one of several devices, which
// complicates things. The main complication is that there may be
// different bootsector layouts. The primary job of the init function
// is to check the device id, possibly fill in the bootsector info,
// or even to use the CFI support to get the bootsector info from the
// device itself. There may be other complications, e.g. minor variations
// of a given board design. These can be handled by h/w specific init
// functions in the platform HAL.
//
// The query function need not do anything useful, it is
// driver-defined.
//
// No read function need be supplied because the flash memory is
// always directly accessible to the cpu.
//
// The hwr_map_error is a no-op.
//
// Erase, program, and the locking functions need real
// implementations.

// ----------------------------------------------------------------------------
// The protocol understood by AMD flash chips and compatibles.
// The AM29_PARALLEL() macro is used in bus configurations with multiple
// devices in parallel, to issue commands to all the devices in a single
// write. In theory some of the operations, e.g. READ_DEVID, only need
// to access a single chip but then you get into complications for the
// SETUP commands.
#define AM29_COMMAND_SETUP1         AM29_PARALLEL(0x00AA)
#define AM29_COMMAND_SETUP2         AM29_PARALLEL(0x0055)
#define AM29_COMMAND_RESET          AM29_PARALLEL(0x00F0)
#define AM29_COMMAND_AUTOSELECT     AM29_PARALLEL(0x0090)
#define AM29_COMMAND_ERASE          AM29_PARALLEL(0x0080)
#define AM29_COMMAND_ERASE_SECTOR   AM29_PARALLEL(0x0030)
#define AM29_COMMAND_CFI            AM29_PARALLEL(0x0098)
#define AM29_COMMAND_PROGRAM        AM29_PARALLEL(0x00A0)

// CFI offsets of interest. This assumes that the standard query table
// has not been replaced by the extended query table, although the
// CFI standard allows that behaviour.
#define AM29_OFFSET_CFI_Q                       AM29_OFFSET_CFI_DATA(0x0010)
#define AM29_OFFSET_CFI_SIZE                    AM29_OFFSET_CFI_DATA(0x0027)
#define AM29_OFFSET_CFI_BLOCK_REGIONS           AM29_OFFSET_CFI_DATA(0x002C)
#define AM29_OFFSET_CFI_BLOCK_COUNT_LSB(_i_)    AM29_OFFSET_CFI_DATA(0x002D + (4 * _i_))
#define AM29_OFFSET_CFI_BLOCK_COUNT_MSB(_i_)    AM29_OFFSET_CFI_DATA(0x002E + (4 * _i_))
#define AM29_OFFSET_CFI_BLOCK_SIZE_LSB(_i_)     AM29_OFFSET_CFI_DATA(0x002F + (4 * _i_))
#define AM29_OFFSET_CFI_BLOCK_SIZE_MSB(_i_)     AM29_OFFSET_CFI_DATA(0x0030 + (4 * _i_))

#define AM29_STATUS_DQ7             AM29_PARALLEL(0x0080)
#define AM29_STATUS_DQ6             AM29_PARALLEL(0x0040)
#define AM29_STATUS_DQ5             AM29_PARALLEL(0x0020)
#define AM29_STATUS_DQ4             AM29_PARALLEL(0x0010)
#define AM29_STATUS_DQ3             AM29_PARALLEL(0x0008)
#define AM29_STATUS_DQ2             AM29_PARALLEL(0x0004)
#define AM29_STATUS_DQ1             AM29_PARALLEL(0x0002)
#define AM29_STATUS_DQ0             AM29_PARALLEL(0x0001)

// The addresses used for programming the flash may be different from
// the ones used to read the flash. For example the former may be in
// uncached memory while the latter are in cached memory. The platform
// HAL can provide a macro HAL_AM29XXXXX_P2V() to convert an address
// relative to the base in the flash dev structure to the address that
// should actually be manipulated.
# ifdef HAL_AM29XXXXX_P2V
#  define AM29_P2V(_addr_)  (volatile AM29_TYPE*)HAL_AM29XXXXX_P2V(_addr_)
# else
#  define AM29_P2V(_addr_)  (volatile AM29_TYPE*)(_addr_)
# endif

// When programming the flash the source data may not be aligned
// correctly (although usually it will be). Hence it is necessary to
// construct the 16-bit or 32-bit numbers to be written to the flash
// from individual bytes, allowing for endianness.
#define AM29_NEXT_DATUM_8(_ptr_) (*_ptr_++)
#if CYG_BYTEORDER == CYG_LSBFIRST
# define AM29_NEXT_DATUM_16(_ptr_)                  \
    ({                                              \
        cyg_uint16 _result_;                        \
        _result_  = (_ptr_[1] << 8) | _ptr_[0];     \
        _ptr_    += 2;                              \
        _result_; })

# define AM29_NEXT_DATUM_32(_ptr_)                                                      \
    ({                                                                                  \
        cyg_uint16 _result_;                                                            \
        _result_  = (_ptr_[3] << 24) | (_ptr_[2] << 16) | (_ptr_[1] << 8) | _ptr_[0];   \
        _ptr_    += 4;                                                                  \
        _result_; })
#else
# define AM29_NEXT_DATUM_16(_ptr_)                  \
    ({                                              \
        cyg_uint16 _result_;                        \
        _result_  = (_ptr_[0] << 8) | _ptr_[1];     \
        _ptr_    += 2;                              \
        _result_; })

# define AM29_NEXT_DATUM_32(_ptr_)                                                      \
    ({                                                                                  \
        cyg_uint16 _result_;                                                            \
        _result_  = (_ptr_[0] << 24) | (_ptr_[1] << 16) | (_ptr_[2] << 8) | _ptr_[3];   \
        _ptr_    += 4;                                                                  \
        _result_; })

#endif

// ----------------------------------------------------------------------------
// Generic code.

// A dummy initialization routine, for platforms where everything is
// done statically and there is no need to check device ids or anything similar.
int
cyg_am29xxxxx_init_nop(struct cyg_flash_dev* dev)
{
    CYG_UNUSED_PARAM(struct cyg_flash_dev*, dev);
    return CYG_FLASH_ERR_OK;
}

// A dummy query routine. The implementation of this is specific to
// each device driver, and I choose to do as little as possible.
size_t
cyg_am29xxxxx_query_nop(struct cyg_flash_dev* dev, void* data, const size_t len)
{
    CYG_UNUSED_PARAM(struct cyg_flash_dev*, dev);
    CYG_UNUSED_PARAM(void*, data);
    CYG_UNUSED_PARAM(size_t, len);
    return 0;
}

// A dummy hwr_map_error routine.
int
cyg_am29xxxxx_hwr_map_error_nop(struct cyg_flash_dev* dev, int err)
{
    CYG_UNUSED_PARAM(struct cyg_flash_dev*, dev);
    return err;
}

// Dummy lock/unlock routines
int
cyg_am29xxxxx_lock_nop(struct cyg_flash_dev* dev, const cyg_flashaddr_t addr)
{
    CYG_UNUSED_PARAM(struct cyg_flash_dev*, dev);
    CYG_UNUSED_PARAM(cyg_flashaddr_t, addr);
    return CYG_FLASH_ERR_DRV_WRONG_PART;
}

int
cyg_am29xxxxx_unlock_nop(struct cyg_flash_dev* dev, const cyg_flashaddr_t addr)
{
    CYG_UNUSED_PARAM(struct cyg_flash_dev*, dev);
    CYG_UNUSED_PARAM(cyg_flashaddr_t, addr);
    return CYG_FLASH_ERR_DRV_WRONG_PART;
}

// On some architectures there are problems calling the .2ram
// functions from the main ones. Specifically the compiler may issue a
// short call, even though the flash and ram are too far apart. The
// solution is to indirect via a function pointer, but the simplistic
// approach is vulnerable to compiler optimization. Hence the function
// pointer is passed through an anonymizer. Even this may fail in
// future if the compiler starts doing global optimization.
static void*
am29_anonymizer(void* fn)
{
    return fn;
}

// Get info about the current block, i.e. base and size.
static void
am29_get_block_info(struct cyg_flash_dev* dev, const cyg_flashaddr_t addr, cyg_flashaddr_t* block_start, size_t* block_size)
{
    cyg_uint32      i;
    size_t          offset  = addr - dev->start;
    cyg_flashaddr_t result;

    result  = dev->start;
    
    for (i = 0; i < dev->num_block_infos; i++) {
        if (offset < (dev->block_info[i].blocks * dev->block_info[i].block_size)) {
            offset         -= (offset % dev->block_info[i].block_size);
            *block_start    = result + offset;
            *block_size     = dev->block_info[i].block_size;
            return;
        }
        result  += (dev->block_info[i].blocks * dev->block_info[i].block_size);
        offset  -= (dev->block_info[i].blocks * dev->block_info[i].block_size);
    }
    CYG_FAIL("Address out of range of selected flash device");
}

// ----------------------------------------------------------------------------
// Instantiate all of the h/w functions appropriate for the various
// configurations.
//   The suffix is used to construct the function names.
//   Types for the width of the bus, controlling the granularity of access.
//   devcount specifies the number of devices in parallel, and is used for looping
//   The NEXT_DATUM() macro allows for misaligned source data.
//   The PARALLEL macro, if defined, is used for sending commands and reading
//   status bits from all devices in the bank in one operation.

// A single 8-bit device on an 8-bit bus.
#define AM29_SUFFIX             8
#define AM29_TYPE               cyg_uint8
#define AM29_DEVCOUNT           1
#define AM29_NEXT_DATUM(_ptr_)  AM29_NEXT_DATUM_8(_ptr_)

#include "am29xxxxx_aux.c"

#undef AM29_SUFFIX
#undef AM29_TYPE
#undef AM29_DEVCOUNT
#undef AM29_NEXT_DATUM

// A single 16-bit device.
#define AM29_SUFFIX             16
#define AM29_TYPE               cyg_uint16
#define AM29_DEVCOUNT           1
#define AM29_NEXT_DATUM(_ptr_)  AM29_NEXT_DATUM_16(_ptr_)

#include "am29xxxxx_aux.c"

#undef AM29_SUFFIX
#undef AM29_TYPE
#undef AM29_DEVCOUNT
#undef AM29_NEXT_DATUM

// A single 32-bit device.
#define AM29_SUFFIX             32
#define AM29_TYPE               cyg_uint32
#define AM29_DEVCOUNT           1
#define AM29_NEXT_DATUM(_ptr_)  AM29_NEXT_DATUM_32(_ptr_)

#include "am29xxxxx_aux.c"

#undef AM29_SUFFIX
#undef AM29_TYPE
#undef AM29_DEVCOUNT
#undef AM29_NEXT_DATUM

// Two 8-bit devices, giving a 16-bit bus. 
#define AM29_SUFFIX             88
#define AM29_TYPE               cyg_uint16
#define AM29_DEVCOUNT           2
#define AM29_NEXT_DATUM(_ptr_)  AM29_NEXT_DATUM_16(_ptr_)
#define AM29_PARALLEL(_cmd_)     ((_cmd_ << 8) | _cmd_)

#include "am29xxxxx_aux.c"

#undef AM29_SUFFIX
#undef AM29_TYPE
#undef AM29_DEVCOUNT
#undef AM29_NEXT_DATUM

// Four 8-bit devices, giving a 32-bit bus. 
#define AM29_SUFFIX             8888
#define AM29_TYPE               cyg_uint32
#define AM29_DEVCOUNT           4
#define AM29_NEXT_DATUM(_ptr_)  AM29_NEXT_DATUM_32(_ptr_)
#define AM29_PARALLEL(_cmd_)    ((_cmd_ << 24) | (_cmd_ << 16) | (_cmd_ << 8) | _cmd_)

#include "am29xxxxx_aux.c"

#undef AM29_SUFFIX
#undef AM29_TYPE
#undef AM29_DEVCOUNT
#undef AM29_NEXT_DATUM

// Two 16-bit devices, giving a 32-bit bus.
#define AM29_SUFFIX             1616
#define AM29_TYPE               cyg_uint32
#define AM29_DEVCOUNT           2
#define AM29_NEXT_DATUM(_ptr_)  AM29_NEXT_DATUM_32(_ptr_)
#define AM29_PARALLEL(_cmd_)    ((_cmd_ << 16) | _cmd_)

#include "am29xxxxx_aux.c"

#undef AM29_SUFFIX
#undef AM29_TYPE
#undef AM29_DEVCOUNT
#undef AM29_NEXT_DATUM

// 16AS8. A 16-bit device hooked up so that only byte accesses are
// allowed. This requires unusual offsets
#define AM29_SUFFIX                 16as8
#define AM29_TYPE                   cyg_uint8
#define AM29_DEVCOUNT               1
#define AM29_NEXT_DATUM(_ptr_)      AM29_NEXT_DATUM_8(_ptr_)
#define AM29_OFFSET_COMMAND         0x0AAA
#define AM29_OFFSET_COMMAND2        0x0555
#define AM29_OFFSET_DEVID           0x0002
#define AM29_OFFSET_DEVID2          0x001C
#define AM29_OFFSET_DEVID3          0x001E
#define AM29_OFFSET_CFI             0x00AA
#define AM29_OFFSET_CFI_DATA(_idx_) (2 * _idx_)

#include "am29xxxxx_aux.c"
