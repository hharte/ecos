//==========================================================================
//
//      flash.c
//
//      RedBoot - FLASH memory support
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.1 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://www.redhat.com/                                                   
//                                                                          
// Software distributed under the License is distributed on an "AS IS"      
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
// License for the specific language governing rights and limitations under 
// the License.                                                             
//                                                                          
// The Original Code is eCos - Embedded Configurable Operating System,      
// released September 30, 1998.                                             
//                                                                          
// The Initial Developer of the Original Code is Red Hat.                   
// Portions created by Red Hat are                                          
// Copyright (C) 1998, 1999, 2000, 2001 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-07-28
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>
#include <cyg/io/flash.h>
#include <fis.h>
#include <sib.h>

// CLI function
static cmd_fun do_fis;
RedBoot_cmd("fis", 
            "Manage FLASH images", 
            "{cmds}",
            do_fis
    );

#ifdef CYGOPT_REDBOOT_FIS
// Image management functions
local_cmd_entry("init",
                "Initialize FLASH Image System [FIS]",
                "[-f]",
                fis_init,
                FIS_cmds
    );
#ifdef CYGSEM_REDBOOT_FIS_CRC_CHECK
# define FIS_LIST_OPTS "[-c] [-d]"
#else
# define FIS_LIST_OPTS "[-d]"
#endif
local_cmd_entry("list",
                "Display contents of FLASH Image System [FIS]",
                FIS_LIST_OPTS,
                fis_list,
                FIS_cmds
    );
local_cmd_entry("free",
                "Display free [available] locations within FLASH Image System [FIS]",
                "",
                fis_free,
                FIS_cmds
    );
local_cmd_entry("delete",
                "Display an image from FLASH Image System [FIS]",
                "name",
                fis_delete,
                FIS_cmds
    );

static char fis_load_usage[] = 
#ifdef CYGPKG_COMPRESS_ZLIB
                      "[-d] "
#endif
                      "[-b <memory_load_address>] [-c] name";

local_cmd_entry("load",
                "Load image from FLASH Image System [FIS] into RAM",
                fis_load_usage,
                fis_load,
                FIS_cmds
    );
local_cmd_entry("create",
                "Create an image",
                "-b <mem_base> -l <image_length> [-s <data_length>]\n"
                "      [-f <flash_addr>] [-e <entry_point>] [-r <ram_addr>] [-n] <name>",
                fis_create,
                FIS_cmds
    );
#endif

// Raw flash access functions
local_cmd_entry("erase",
                "Erase FLASH contents",
                "-f <flash_addr> -l <length>",
                fis_erase,
                FIS_cmds
    );
#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING
local_cmd_entry("lock",
                "LOCK FLASH contents",
                "-f <flash_addr> -l <length>",
                fis_lock,
                FIS_cmds
    );
local_cmd_entry("unlock",
                "UNLOCK FLASH contents",
                "-f <flash_addr> -l <length>",
                fis_unlock,
                FIS_cmds
    );
#endif
local_cmd_entry("write",
                "Write raw data directly to FLASH",
                "-f <flash_addr> -b <mem_base> -l <image_length>",
                fis_write,
                FIS_cmds
    );

// Define table boundaries
CYG_HAL_TABLE_BEGIN( __FIS_cmds_TAB__, FIS_cmds);
CYG_HAL_TABLE_END( __FIS_cmds_TAB_END__, FIS_cmds);

extern struct cmd __FIS_cmds_TAB__[], __FIS_cmds_TAB_END__;

// Local data used by these routines
static void *flash_start, *flash_end;
static int block_size, blocks;
#ifdef CYGOPT_REDBOOT_FIS
static void *fis_work_block;
static void *fis_addr;
static int fisdir_size;  // Size of FIS directory.
#endif
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
static void *cfg_base;   // Location in Flash of config data
static int   cfg_size;   // Length of config data - rounded to Flash block size
// Prototypes for local functions
static unsigned char *flash_lookup_config(char *key);
#endif

static void
fis_usage(char *why)
{
    diag_printf("*** invalid 'fis' command: %s\n", why);
    cmd_usage(__FIS_cmds_TAB__, &__FIS_cmds_TAB_END__, "fis ");
}

static void        
_show_invalid_flash_address(CYG_ADDRESS flash_addr, int stat)
{
    diag_printf("Invalid FLASH address %p: %s\n", (void *)flash_addr, flash_errmsg(stat));
    diag_printf("   valid range is %p-%p\n", (void *)flash_start, (void *)flash_end);
}

#ifdef CYGOPT_REDBOOT_FIS
struct fis_image_desc *
fis_lookup(char *name)
{
    int i;
    struct fis_image_desc *img;

    img = (struct fis_image_desc *)fis_work_block;
    for (i = 0;  i < block_size/sizeof(*img);  i++, img++) {
        if ((img->name[0] != (unsigned char)0xFF) && (strcmp(name, img->name) == 0)) {
            return img;
        }
    }
    return (struct fis_image_desc *)0;
}

static void
fis_init(int argc, char *argv[])
{
    int stat;
    struct fis_image_desc *img;
    void *err_addr;
    bool full_init = false;
    struct option_info opts[1];
    CYG_ADDRESS redboot_flash_start;
    unsigned long redboot_image_size;

    init_opts(&opts[0], 'f', false, OPTION_ARG_TYPE_FLG, 
              (void **)&full_init, (bool *)0, "full initialization, erases all of flash");
    if (!scan_opts(argc, argv, 2, opts, 1, 0, 0, ""))
    {
        return;
    }

    if (!verify_action("About to initialize [format] FLASH image system")) {
        diag_printf("** Aborted\n");
        return;
    }
    diag_printf("*** Initialize FLASH Image System\n");

#define MIN_REDBOOT_IMAGE_SIZE CYGBLD_REDBOOT_MIN_IMAGE_SIZE
    redboot_image_size = block_size > MIN_REDBOOT_IMAGE_SIZE ? block_size : MIN_REDBOOT_IMAGE_SIZE;

    // Create a pseudo image for RedBoot
    img = (struct fis_image_desc *)fis_work_block;
    memset(img, 0xFF, block_size);  // Start with erased data
#ifdef CYGOPT_REDBOOT_FIS_RESERVED_BASE
    memset(img, 0, sizeof(*img));
    strcpy(img->name, "(reserved)");
    img->flash_base = (CYG_ADDRESS)flash_start;
    img->mem_base = (CYG_ADDRESS)flash_start;
    img->size = CYGNUM_REDBOOT_FLASH_RESERVED_BASE;
    img++;
#endif
    redboot_flash_start = (CYG_ADDRESS)flash_start + CYGBLD_REDBOOT_FLASH_BOOT_OFFSET;
#ifdef CYGOPT_REDBOOT_FIS_REDBOOT
    memset(img, 0, sizeof(*img));
    strcpy(img->name, "RedBoot");
    img->flash_base = redboot_flash_start;
    img->mem_base = redboot_flash_start;
    img->size = redboot_image_size;
    img++;
    redboot_flash_start += redboot_image_size;
#endif
#ifdef CYGOPT_REDBOOT_FIS_REDBOOT_POST
#ifdef CYGNUM_REDBOOT_FIS_REDBOOT_POST_OFFSET
    // Take care to place the POST entry at the right offset:
    redboot_flash_start = (CYG_ADDRESS)flash_start + CYGNUM_REDBOOT_FIS_REDBOOT_POST_OFFSET;
#endif
    memset(img, 0, sizeof(*img));
    strcpy(img->name, "RedBoot[post]");
    img->flash_base = redboot_flash_start;
    img->mem_base = redboot_flash_start;
    img->size = redboot_image_size;
    img++;
    redboot_flash_start += redboot_image_size;
#endif
#ifdef CYGOPT_REDBOOT_FIS_REDBOOT_BACKUP
    // And a backup image
    memset(img, 0, sizeof(*img));
    strcpy(img->name, "RedBoot[backup]");
    img->flash_base = redboot_flash_start;
    img->mem_base = redboot_flash_start;
    img->size = redboot_image_size;
    img++;
    redboot_flash_start += redboot_image_size;
#endif
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
    // And a descriptor for the configuration data
    memset(img, 0, sizeof(*img));
    strcpy(img->name, "RedBoot config");
    img->flash_base = (CYG_ADDRESS)cfg_base;
    img->mem_base = (CYG_ADDRESS)cfg_base;
    img->size = cfg_size;
    img++;
#endif
    // And a descriptor for the descriptor table itself
    memset(img, 0, sizeof(*img));
    strcpy(img->name, "FIS directory");
    img->flash_base = (CYG_ADDRESS)fis_addr;
    img->mem_base = (CYG_ADDRESS)fis_addr;
    img->size = block_size;
    img++;

#ifdef CYGOPT_REDBOOT_FIS_DIRECTORY_ARM_SIB_ID
    // FIS gets the size of a full block - note, this should be changed
    // if support is added for multi-block FIS structures.
    img = (struct fis_image_desc *)((CYG_ADDRESS)fis_work_block + block_size);
    // Add a footer so the FIS will be recognized by the ARM Boot
    // Monitor as a reserved area.
    {
        tFooter* footer_p = (tFooter*)((CYG_ADDRESS)img - sizeof(tFooter));
        cyg_uint32 check = 0;
        cyg_uint32 *check_ptr = (cyg_uint32 *)footer_p;
        cyg_int32 count = (sizeof(tFooter) - 4) >> 2;

        // Prepare footer. Try to protect all but the reserved space
        // and the first RedBoot image (which is expected to be
        // bootable), but fall back to just protecting the FIS if it's
        // not at the default position in the flash.
#if defined(CYGOPT_REDBOOT_FIS_RESERVED_BASE) && (-1 == CYGNUM_REDBOOT_FIS_DIRECTORY_BLOCK)
        footer_p->blockBase = (char*)_ADDR_REDBOOT_TO_ARM(flash_start);
        footer_p->blockBase += CYGNUM_REDBOOT_FLASH_RESERVED_BASE + redboot_image_size;
#else
        footer_p->blockBase = (char*)_ADDR_REDBOOT_TO_ARM(fis_work_block);
#endif
        footer_p->infoBase = NULL;
        footer_p->signature = FLASH_FOOTER_SIGNATURE;
        footer_p->type = TYPE_REDHAT_REDBOOT;

        // and compute its checksum
        for ( ; count > 0; count--) {
            if (*check_ptr > ~check)
                check++;
            check += *check_ptr++;
        }
        footer_p->checksum = ~check;
    }
#endif

    // Do this after creating the initialized table because that inherently
    // calculates where the high water mark of default RedBoot images is.

    if (full_init) {
        unsigned long erase_size;
        CYG_ADDRESS erase_start;
        // Erase everything except default RedBoot images, fis block, 
        // and config block.
        // First deal with the possible first part, before RedBoot images:
#if (CYGBLD_REDBOOT_FLASH_BOOT_OFFSET > CYGNUM_REDBOOT_FLASH_RESERVED_BASE)
        erase_start = (CYG_ADDRESS)flash_start + CYGNUM_REDBOOT_FLASH_RESERVED_BASE;
        erase_size =  (CYG_ADDRESS)flash_start + CYGBLD_REDBOOT_FLASH_BOOT_OFFSET;
        if ( erase_size > erase_start ) {
            erase_size -= erase_start;
            if ((stat = flash_erase((void *)erase_start, erase_size,
                                    (void **)&err_addr)) != 0) {
                diag_printf("   initialization failed at %p: %s\n",
                            err_addr, flash_errmsg(stat));
            }
        }
#endif
        // second deal with the larger part in the main:
        erase_start = redboot_flash_start; // high water of created images
        // Now the empty bits between the end of Redboot and the cfg and dir 
        // blocks. 
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
        if (fis_addr > cfg_base) {
          erase_size = (CYG_ADDRESS)cfg_base - erase_start; // the gap between HWM and config data
        } else {
          erase_size = (CYG_ADDRESS)fis_addr - erase_start; // the gap between HWM and fis data
        }
        if ((stat = flash_erase((void *)erase_start, erase_size,
                                (void **)&err_addr)) != 0) {
          diag_printf("   initialization failed %p: %s\n",
                 err_addr, flash_errmsg(stat));
        }
        erase_start += (erase_size + block_size);
        if (fis_addr > cfg_base) {
          erase_size = (CYG_ADDRESS)fis_addr - erase_start; // the gap between config and fis data
        } else {
          erase_size = (CYG_ADDRESS)cfg_base - erase_start; // the gap between fis and config data
        }
        if ((stat = flash_erase((void *)erase_start, erase_size,
                                (void **)&err_addr)) != 0) {
          diag_printf("   initialization failed %p: %s\n",
                 err_addr, flash_errmsg(stat));
        }
        erase_start += (erase_size + block_size);
#else  // !CYGSEM_REDBOOT_FLASH_CONFIG        
        erase_size = (CYG_ADDRESS)fis_addr - erase_start; // the gap between HWM and fis data
        if ((stat = flash_erase((void *)erase_start, erase_size,
                                (void **)&err_addr)) != 0) {
          diag_printf("   initialization failed %p: %s\n",
                 err_addr, flash_errmsg(stat));
        }
        erase_start += (erase_size + block_size);          
#endif
        // Lastly, anything at the end
        erase_size = ((CYG_ADDRESS)flash_end - erase_start) + 1;
        if ((stat = flash_erase((void *)erase_start, erase_size,
                                (void **)&err_addr)) != 0) {
            diag_printf("   initialization failed at %p: %s\n",
                        err_addr, flash_errmsg(stat));
        }
    } else {
        diag_printf("    Warning: device contents not erased, some blocks may not be usable\n");
    }

#ifdef CYGSEM_REDBOOT_FLASH_LOCK_SPECIAL
    // Ensure [quietly] that the directory is unlocked before trying to update
    flash_unlock((void *)fis_addr, block_size, (void **)&err_addr);
#endif
    if ((stat = flash_erase(fis_addr, block_size, (void **)&err_addr)) != 0) {
        diag_printf("   initialization failed at %p: %s\n", err_addr, flash_errmsg(stat));
    } else {
        if ((stat = flash_program(fis_addr, fis_work_block, 
                                  (CYG_ADDRESS)img - (CYG_ADDRESS)fis_work_block,
                                  (void **)&err_addr)) != 0) {
            diag_printf("Error writing image descriptors at %p: %s\n", 
                        err_addr, flash_errmsg(stat));
        }
    }
#ifdef CYGSEM_REDBOOT_FLASH_LOCK_SPECIAL
    // Ensure [quietly] that the directory is locked after the update
    flash_lock((void *)fis_addr, block_size, (void **)&err_addr);
#endif
}

static void
fis_list(int argc, char *argv[])
{
    struct fis_image_desc *img;
    int i;
    bool show_cksums = false;
    bool show_datalen = false;
    struct option_info opts[2];

#ifdef CYGHWR_REDBOOT_ARM_FLASH_SIB
    // FIXME: this is somewhat half-baked
    extern void arm_fis_list(void);
    arm_fis_list();
    return;
#endif

    init_opts(&opts[0], 'd', false, OPTION_ARG_TYPE_FLG, 
              (void **)&show_datalen, (bool *)0, "display data length");
#ifdef CYGSEM_REDBOOT_FIS_CRC_CHECK
    init_opts(&opts[1], 'c', false, OPTION_ARG_TYPE_FLG, 
              (void **)&show_cksums, (bool *)0, "display checksums");
    i = 2;
#else
    i = 1;
#endif
    if (!scan_opts(argc, argv, 2, opts, i, 0, 0, ""))
    {
        return;
    }
    img = (struct fis_image_desc *)fis_work_block;
    // Let diag_printf do the formatting in both cases, rather than counting
    // cols by hand....
    diag_printf("%-16s  %-10s  %-10s  %-10s  %-s\n",
                "Name","FLASH addr",
                show_cksums ? "Checksum" : "Mem addr",
                show_datalen ? "Datalen" : "Length",
                "Entry point" );
    for (i = 0;  i < block_size/sizeof(*img);  i++, img++) {
        if (img->name[0] != (unsigned char)0xFF) {
            diag_printf("%-16s  0x%08lX  0x%08lX  0x%08lX  0x%08lX\n", img->name, 
                        img->flash_base, 
#ifdef CYGSEM_REDBOOT_FIS_CRC_CHECK
                        show_cksums ? img->file_cksum : img->mem_base, 
                        show_datalen ? img->data_length : img->size, 
#else
                        img->mem_base, 
                        img->size, 
#endif
                        img->entry_point);
        }
    }
}

static void
fis_free(int argc, char *argv[])
{
    unsigned long *fis_ptr, *fis_end;
    unsigned long *area_start;

    // Do not search the area reserved for pre-RedBoot systems:
    fis_ptr = (unsigned long *)((CYG_ADDRESS)flash_start + CYGNUM_REDBOOT_FLASH_RESERVED_BASE);
    fis_end = (unsigned long *)(CYG_ADDRESS)flash_end;
    area_start = fis_ptr;
    while (fis_ptr < fis_end) {
        if (*fis_ptr != (unsigned long)0xFFFFFFFF) {
            if (area_start != fis_ptr) {
                // Assume that this is something
                diag_printf("  0x%08lX .. 0x%08lX\n",
                            (CYG_ADDRESS)area_start, (CYG_ADDRESS)fis_ptr);
            }
            // Find next blank block
            area_start = fis_ptr;
            while (area_start < fis_end) {
                if (*area_start == (unsigned long)0xFFFFFFFF) {
                    break;
                }
                area_start += block_size / sizeof(CYG_ADDRESS);
            }
            fis_ptr = area_start;
        } else {
            fis_ptr += block_size / sizeof(CYG_ADDRESS);
        }
    }
    if (area_start != fis_ptr) {
        diag_printf("  0x%08lX .. 0x%08lX\n", 
                    (CYG_ADDRESS)area_start, (CYG_ADDRESS)fis_ptr);
    }
}

// Find the first unused area of flash which is long enough
static bool
fis_find_free(CYG_ADDRESS *addr, unsigned long length)
{
    unsigned long *fis_ptr, *fis_end;
    unsigned long *area_start;

    // Do not search the area reserved for pre-RedBoot systems:
    fis_ptr = (unsigned long *)((CYG_ADDRESS)flash_start + CYGNUM_REDBOOT_FLASH_RESERVED_BASE);
    fis_end = (unsigned long *)(CYG_ADDRESS)flash_end;
    area_start = fis_ptr;
    while (fis_ptr < fis_end) {
        if (*fis_ptr != (unsigned long)0xFFFFFFFF) {
            if (area_start != fis_ptr) {
                // Assume that this is something
                if ((fis_ptr-area_start) >= (length/sizeof(unsigned))) {
                    *addr = (CYG_ADDRESS)area_start;
                    return true;
                }
            }
            // Find next blank block
            area_start = fis_ptr;
            while (area_start < fis_end) {
                if (*area_start == (unsigned long)0xFFFFFFFF) {
                    break;
                }
                area_start += block_size / sizeof(CYG_ADDRESS);
            }
            fis_ptr = area_start;
        } else {
            fis_ptr += block_size / sizeof(CYG_ADDRESS);
        }
    }
    if (area_start != fis_ptr) {
        if ((fis_ptr-area_start) >= (length/sizeof(unsigned))) {
            *addr = (CYG_ADDRESS)area_start;
            return true;
        }
    }
    return false;
}

static void
fis_create(int argc, char *argv[])
{
    int i, stat;
    unsigned long length, img_size;
    CYG_ADDRESS mem_addr, exec_addr, flash_addr, entry_addr;
    char *name;
    bool mem_addr_set = false;
    bool exec_addr_set = false;
    bool entry_addr_set = false;
    bool flash_addr_set = false;
    bool length_set = false;
    bool img_size_set = false;
    bool no_copy = false;
    void *err_addr;
    struct fis_image_desc *img;
    bool slot_found, defaults_assumed;
    struct option_info opts[7];
    bool prog_ok = true;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void **)&mem_addr, (bool *)&mem_addr_set, "memory base address");
    init_opts(&opts[1], 'r', true, OPTION_ARG_TYPE_NUM, 
              (void **)&exec_addr, (bool *)&exec_addr_set, "ram base address");
    init_opts(&opts[2], 'e', true, OPTION_ARG_TYPE_NUM, 
              (void **)&entry_addr, (bool *)&entry_addr_set, "entry point address");
    init_opts(&opts[3], 'f', true, OPTION_ARG_TYPE_NUM, 
              (void **)&flash_addr, (bool *)&flash_addr_set, "FLASH memory base address");
    init_opts(&opts[4], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void **)&length, (bool *)&length_set, "image length [in FLASH]");
    init_opts(&opts[5], 's', true, OPTION_ARG_TYPE_NUM, 
              (void **)&img_size, (bool *)&img_size_set, "image size [actual data]");
    init_opts(&opts[6], 'n', false, OPTION_ARG_TYPE_FLG, 
              (void **)&no_copy, (bool *)0, "don't copy from RAM to FLASH, just update directory");
    if (!scan_opts(argc, argv, 2, opts, 7, (void *)&name, OPTION_ARG_TYPE_STR, "file name"))
    {
        fis_usage("invalid arguments");
        return;
    }

    defaults_assumed = false;
    if (name) {
        // Search existing files to acquire defaults for params not specified:
        img = (struct fis_image_desc *)fis_work_block;
        for (i = 0;  i < block_size/sizeof(*img);  i++, img++) {
            if ((img->name[0] != (unsigned char)0xFF) && (strcmp(name, img->name) == 0)) {
                // Found it, so get any unset but necessary params from there:
                if (!length_set) {
                    length_set = true;
                    length = img->size;
                    defaults_assumed = true;
                }
                if (!exec_addr_set) {
                    // Preserve "normal" behaviour
                    exec_addr_set = true;
                    exec_addr = flash_addr_set ? flash_addr : mem_addr;
                }           
                if (!flash_addr_set) {
                    flash_addr_set = true;
                    flash_addr = img->flash_base;
                    defaults_assumed = true;
                }           
                break;
            }
        }
    }

    if ((!no_copy && !mem_addr_set) || (no_copy && !flash_addr_set) ||
        !length_set || !name) {
        fis_usage("required parameter missing");
        return;
    }
    if (!img_size_set) {
        img_size = length;
    }
    // 'length' is size of FLASH image, 'img_size' is actual data size
    // Round up length to FLASH block size
#ifndef CYGPKG_HAL_MIPS // FIXME: compiler is b0rken
    length = ((length + block_size - 1) / block_size) * block_size;
    if (length < img_size) {
        diag_printf("Invalid FLASH image size/length combination\n");
        return;
    }
#endif
    if (flash_addr_set &&
        ((stat = flash_verify_addr((void *)flash_addr)) ||
         (stat = flash_verify_addr((void *)(flash_addr+img_size-1))))) {
        _show_invalid_flash_address(flash_addr, stat);
        return;
    }
    if (flash_addr_set && flash_addr & (block_size-1)) {
        diag_printf("Invalid FLASH address: %p\n", (void *)flash_addr);
        diag_printf("   must be 0x%x aligned\n", block_size);
        return;
    }
    if (strlen(name) >= sizeof(img->name)) {
        diag_printf("Name is too long, must be less than %d chars\n", (int)sizeof(img->name));
        return;
    }
    if (!no_copy) {
        if ((mem_addr < (CYG_ADDRESS)ram_start) ||
            ((mem_addr+img_size) >= (CYG_ADDRESS)ram_end)) {
            diag_printf("** WARNING: RAM address: %p may be invalid\n", (void *)mem_addr);
            diag_printf("   valid range is %p-%p\n", (void *)ram_start, (void *)ram_end);
        }
        if (!flash_addr_set && !fis_find_free(&flash_addr, length)) {
            diag_printf("Can't locate %lx(%ld) bytes free in FLASH\n", length, length);
            return;
        }
    }
    // Find a slot in the directory for this entry
    // First, see if an image by this name is already present
    slot_found = false;
    img = (struct fis_image_desc *)fis_work_block;
    for (i = 0;  i < block_size/sizeof(*img);  i++, img++) {
        if ((img->name[0] != (unsigned char)0xFF) && (strcmp(name, img->name) == 0)) {
            if (flash_addr_set && (img->flash_base != flash_addr)) {
                diag_printf("Image found, but FLASH address incorrect\n");
                return;
            }
            if (img->size != length) {
                diag_printf("Image found, but LENGTH is incorrect (0x%lx != 0x%lx)\n", img->size, length);
                return;
            }
            if (!verify_action("An image named '%s' exists", name)) {
                return;
            } else {                
                slot_found = true;
                if (defaults_assumed) {
                    if (!verify_action("* CAUTION * about to program '%s'\n            at %p..%p from %p", 
                                       name, (void *)flash_addr, (void *)(flash_addr+img_size-1),
                                       (void *)mem_addr)) {
                        return;  // The guy gave up
                    }
                }
                break;
            }
        }
    }
    // If not found, try and find an empty slot
    if (!slot_found) {
        img = (struct fis_image_desc *)fis_work_block;
        for (i = 0;  i < block_size/sizeof(*img);  i++, img++) {
            if (img->name[0] == (unsigned char)0xFF) {
                slot_found = true;
                break;
            }
        }
    }
    if (!no_copy) {
        // Safety check - make sure the address range is not within the code we're running
        if (flash_code_overlaps((void *)flash_addr, (void *)(flash_addr+img_size-1))) {
            diag_printf("Can't program this region - contains code in use!\n");
            return;
        }
        if (prog_ok) {
            // Erase area to be programmed
            if ((stat = flash_erase((void *)flash_addr, length, (void **)&err_addr)) != 0) {
                diag_printf("Can't erase region at %p: %s\n", err_addr, flash_errmsg(stat));
                prog_ok = false;
            }
        }
        if (prog_ok) {
            // Now program it
            if ((stat = flash_program((void *)flash_addr, (void *)mem_addr, img_size, (void **)&err_addr)) != 0) {
                diag_printf("Can't program region at %p: %s\n", err_addr, flash_errmsg(stat));
                prog_ok = false;
            }
        }
    }
    if (prog_ok) {
        // Update directory
        memset(img, 0, sizeof(*img));
        strcpy(img->name, name);
        img->flash_base = flash_addr;
        img->mem_base = exec_addr_set ? exec_addr : (flash_addr_set ? flash_addr : mem_addr);
        img->entry_point = entry_addr_set ? entry_addr : (CYG_ADDRESS)entry_address;  // Hope it's been set
        img->size = length;
        img->data_length = img_size;
#ifdef CYGSEM_REDBOOT_FIS_CRC_CHECK
        img->file_cksum = crc32((unsigned char *)flash_addr, img_size);
#endif
#ifdef CYGSEM_REDBOOT_FLASH_LOCK_SPECIAL
        // Insure [quietly] that the directory is unlocked before trying to update
        flash_unlock((void *)fis_addr, block_size, (void **)&err_addr);
#endif
        if ((stat = flash_erase((void *)fis_addr, block_size, (void **)&err_addr)) != 0) {
            diag_printf("Error erasing at %p: %s\n", err_addr, flash_errmsg(stat));
            // Don't try to program if the erase failed
        } else {
            // Now program it
            if ((stat = flash_program((void *)fis_addr, (void *)fis_work_block, block_size, (void **)&err_addr)) != 0) {
                diag_printf("Error programming at %p: %s\n", err_addr, flash_errmsg(stat));
            }
        }
#ifdef CYGSEM_REDBOOT_FLASH_LOCK_SPECIAL
        // Insure [quietly] that the directory is locked after the update
        flash_lock((void *)fis_addr, block_size, (void **)&err_addr);
#endif
    }
}

extern void arm_fis_delete(char *);
static void
fis_delete(int argc, char *argv[])
{
    char *name;
    int num_reserved, i, stat;
    void *err_addr;
    struct fis_image_desc *img;
    bool slot_found;

    if (!scan_opts(argc, argv, 2, 0, 0, (void **)&name, OPTION_ARG_TYPE_STR, "image name"))
    {
        fis_usage("invalid arguments");
        return;
    }
#ifdef CYGHWR_REDBOOT_ARM_FLASH_SIB
    // FIXME: this is somewhat half-baked
    arm_fis_delete(name);
    return;
#endif
    slot_found = false;
    img = (struct fis_image_desc *)fis_work_block;
    num_reserved = 0;
#ifdef CYGOPT_REDBOOT_FIS_RESERVED_BASE
    num_reserved++;
#endif
#ifdef CYGOPT_REDBOOT_FIS_REDBOOT
    num_reserved++;
#endif
#ifdef CYGOPT_REDBOOT_FIS_REDBOOT_BACKUP
    num_reserved++;
#endif
#ifdef CYGOPT_REDBOOT_FIS_REDBOOT_POST
    num_reserved++;
#endif
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
    num_reserved++;
#endif
#if 1 // And the descriptor for the descriptor table itself
    num_reserved++;
#endif

    for (i = 0;  i < block_size/sizeof(*img);  i++, img++) {
        if ((img->name[0] != (unsigned char)0xFF) && (strcmp(name, img->name) == 0)) {
            if (i < num_reserved) {
                diag_printf("Sorry, '%s' is a reserved image and cannot be deleted\n", img->name);
                return;
            }
            if (!verify_action("Delete image '%s'", name)) {
                return;
            } else {
                slot_found = true;
                break;
            }
        }
    }
    if (!slot_found) {
        diag_printf("No image '%s' found\n", name);
        return;
    }
    // Erase Data blocks (free space)
    if ((stat = flash_erase((void *)img->flash_base, img->size, (void **)&err_addr)) != 0) {
        diag_printf("Error erasing at %p: %s\n", err_addr, flash_errmsg(stat));
    } else {
#ifdef CYGSEM_REDBOOT_FLASH_LOCK_SPECIAL
        // Insure [quietly] that the directory is unlocked before trying to update
        flash_unlock((void *)fis_addr, block_size, (void **)&err_addr);
#endif
        // Update directory
        memset(img, 0xFF, sizeof(*img));
        if ((stat = flash_erase((void *)fis_addr, block_size, (void **)&err_addr)) != 0) {
            diag_printf("Error erasing at %p: %s\n", err_addr, flash_errmsg(stat));
            // Don't try to program if the erase failed
        } else {
            // Now program it
            if ((stat = flash_program((void *)fis_addr, (void *)fis_work_block, block_size, (void **)&err_addr)) != 0) {
                diag_printf("Error programming at %p: %s\n", err_addr, flash_errmsg(stat));
            }
        }
#ifdef CYGSEM_REDBOOT_FLASH_LOCK_SPECIAL
        // Insure [quietly] that the directory is locked after the update
        flash_lock((void *)fis_addr, block_size, (void **)&err_addr);
#endif
    }
}

static void
fis_load(int argc, char *argv[])
{
    char *name;
    struct fis_image_desc *img;
    CYG_ADDRESS mem_addr;
    bool mem_addr_set = false;
    bool show_cksum = false;
    struct option_info opts[3];
#ifdef CYGPKG_REDBOOT_FIS_CRC_CHECK
    unsigned long cksum;
#endif
    int num_options;
#ifdef CYGPKG_COMPRESS_ZLIB
    bool decompress = false;
#endif

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void **)&mem_addr, (bool *)&mem_addr_set, "memory [load] base address");
    init_opts(&opts[1], 'c', false, OPTION_ARG_TYPE_FLG, 
              (void **)&show_cksum, (bool *)0, "display checksum");
    num_options = 2;
#ifdef CYGPKG_COMPRESS_ZLIB
    init_opts(&opts[num_options], 'd', false, OPTION_ARG_TYPE_FLG, 
              (void **)&decompress, 0, "decompress");
    num_options++;
#endif

    if (!scan_opts(argc, argv, 2, opts, num_options, (void **)&name, OPTION_ARG_TYPE_STR, "image name"))
    {
        fis_usage("invalid arguments");
        return;
    }
    if ((img = fis_lookup(name)) == (struct fis_image_desc *)0) {
        diag_printf("No image '%s' found\n", name);
        return;
    }
    if (!mem_addr_set) {
        mem_addr = img->mem_base;
    }
    // Load image from FLASH into RAM
    if ((mem_addr < (CYG_ADDRESS)user_ram_start) ||
        ((mem_addr+img->size) >= (CYG_ADDRESS)user_ram_end)) {
        diag_printf("Not a loadable image\n");
        return;
    }
#ifdef CYGPKG_COMPRESS_ZLIB
    if (decompress) {
        int err;
        _pipe_t fis_load_pipe;
        _pipe_t* p = &fis_load_pipe;
        p->out_buf = (unsigned char*) mem_addr;
        p->out_size = -1;
        p->in_buf = (unsigned char*) img->flash_base;
        p->in_avail = img->data_length;

        err = (*_dc_init)(p);

        if (0 == err)
            err = (*_dc_inflate)(p);

        // Free used resources, do final translation of
        // error value.
        err = (*_dc_close)(p, err);

        if (0 != err && p->msg) {
            diag_printf("decompression error: %s\n", p->msg);
        } else {
            diag_printf("Image loaded from %p-%p\n", (unsigned char *)mem_addr, p->out_buf);
        }

        // Set load address/top
        load_address = mem_addr;
        load_address_end = (unsigned long)p->out_buf;
    } else // dangling block
#endif
    {
        memcpy((void *)mem_addr, (void *)img->flash_base, img->data_length);

        // Set load address/top
        load_address = mem_addr;
        load_address_end = mem_addr + img->data_length;
    }
    entry_address = (unsigned long)img->entry_point;

#ifdef CYGPKG_REDBOOT_FIS_CRC_CHECK
    cksum = crc32((unsigned char *)mem_addr, img->data_length);
    if (show_cksum) {
        diag_printf("Checksum: 0x%08lx\n", cksum);
    }
    // When decompressing, leave CRC checking to decompressor
    if (!decompress && img->file_cksum) {
        if (cksum != img->file_cksum) {
            diag_printf("** Warning - checksum failure.  stored: 0x%08lx, computed: 0x%08lx\n",
                        img->file_cksum, cksum);
        }
    }
#endif
}
#endif // CYGOPT_REDBOOT_FIS

static void
fis_write(int argc, char *argv[])
{
    int stat;
    unsigned long length;
    CYG_ADDRESS mem_addr, flash_addr;
    bool mem_addr_set = false;
    bool flash_addr_set = false;
    bool length_set = false;
    void *err_addr;
    struct option_info opts[3];
    bool prog_ok;

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void **)&mem_addr, (bool *)&mem_addr_set, "memory base address");
    init_opts(&opts[1], 'f', true, OPTION_ARG_TYPE_NUM, 
              (void **)&flash_addr, (bool *)&flash_addr_set, "FLASH memory base address");
    init_opts(&opts[2], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void **)&length, (bool *)&length_set, "image length [in FLASH]");
    if (!scan_opts(argc, argv, 2, opts, 3, 0, 0, 0))
    {
        fis_usage("invalid arguments");
        return;
    }

    if (!mem_addr_set || !flash_addr_set || !length_set) {
        fis_usage("required parameter missing");
        return;
    }

    // Round up length to FLASH block size
#ifndef CYGPKG_HAL_MIPS // FIXME: compiler is b0rken
    length = ((length + block_size - 1) / block_size) * block_size;
#endif
    if (flash_addr_set &&
        ((stat = flash_verify_addr((void *)flash_addr)) ||
         (stat = flash_verify_addr((void *)(flash_addr+length-1))))) {
        _show_invalid_flash_address(flash_addr, stat);
        return;
    }
    if (flash_addr_set && flash_addr & (block_size-1)) {
        diag_printf("Invalid FLASH address: %p\n", (void *)flash_addr);
        diag_printf("   must be 0x%x aligned\n", block_size);
        return;
    }
    if ((mem_addr < (CYG_ADDRESS)ram_start) ||
        ((mem_addr+length) >= (CYG_ADDRESS)ram_end)) {
        diag_printf("** WARNING: RAM address: %p may be invalid\n", (void *)mem_addr);
        diag_printf("   valid range is %p-%p\n", (void *)ram_start, (void *)ram_end);
    }
    // Safety check - make sure the address range is not within the code we're running
    if (flash_code_overlaps((void *)flash_addr, (void *)(flash_addr+length-1))) {
        diag_printf("Can't program this region - contains code in use!\n");
        return;
    }
    if (!verify_action("* CAUTION * about to program FLASH\n            at %p..%p from %p", 
                       (void *)flash_addr, (void *)(flash_addr+length-1),
                       (void *)mem_addr)) {
        return;  // The guy gave up
    }
    prog_ok = true;
    if (prog_ok) {
        // Erase area to be programmed
        if ((stat = flash_erase((void *)flash_addr, length, (void **)&err_addr)) != 0) {
            diag_printf("Can't erase region at %p: %s\n", err_addr, flash_errmsg(stat));
            prog_ok = false;
        }
    }
    if (prog_ok) {
        // Now program it
        if ((stat = flash_program((void *)flash_addr, (void *)mem_addr, length, (void **)&err_addr)) != 0) {
            diag_printf("Can't program region at %p: %s\n", err_addr, flash_errmsg(stat));
            prog_ok = false;
        }
    }
}

static void
fis_erase(int argc, char *argv[])
{
    int stat;
    unsigned long length;
    CYG_ADDRESS flash_addr;
    bool flash_addr_set = false;
    bool length_set = false;
    void *err_addr;
    struct option_info opts[2];

    init_opts(&opts[0], 'f', true, OPTION_ARG_TYPE_NUM, 
              (void **)&flash_addr, (bool *)&flash_addr_set, "FLASH memory base address");
    init_opts(&opts[1], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void **)&length, (bool *)&length_set, "length");
    if (!scan_opts(argc, argv, 2, opts, 2, (void **)0, 0, ""))
    {
        fis_usage("invalid arguments");
        return;
    }

    if (!flash_addr_set || !length_set) {
        fis_usage("missing argument");
        return;
    }
    if (flash_addr_set &&
        ((stat = flash_verify_addr((void *)flash_addr)) ||
         (stat = flash_verify_addr((void *)(flash_addr+length-1))))) {
        _show_invalid_flash_address(flash_addr, stat);
        return;
    }
    if (flash_addr_set && flash_addr & (block_size-1)) {
        diag_printf("Invalid FLASH address: %p\n", (void *)flash_addr);
        diag_printf("   must be 0x%x aligned\n", block_size);
        return;
    }
    // Safety check - make sure the address range is not within the code we're running
    if (flash_code_overlaps((void *)flash_addr, (void *)(flash_addr+length-1))) {
        diag_printf("Can't erase this region - contains code in use!\n");
        return;
    }
    if ((stat = flash_erase((void *)flash_addr, length, (void **)&err_addr)) != 0) {
        diag_printf("Error erasing at %p: %s\n", err_addr, flash_errmsg(stat));
    }
}

#ifdef CYGHWR_IO_FLASH_BLOCK_LOCKING

static void
fis_lock(int argc, char *argv[])
{
    int stat;
    unsigned long length;
    CYG_ADDRESS flash_addr;
    bool flash_addr_set = false;
    bool length_set = false;
    void *err_addr;
    struct option_info opts[2];

    init_opts(&opts[0], 'f', true, OPTION_ARG_TYPE_NUM, 
              (void **)&flash_addr, (bool *)&flash_addr_set, "FLASH memory base address");
    init_opts(&opts[1], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void **)&length, (bool *)&length_set, "length");
    if (!scan_opts(argc, argv, 2, opts, 2, (void **)0, 0, ""))
    {
        fis_usage("invalid arguments");
        return;
    }

    if (!flash_addr_set || !length_set) {
        fis_usage("missing argument");
        return;
    }
    if (flash_addr_set &&
        ((stat = flash_verify_addr((void *)flash_addr)) ||
         (stat = flash_verify_addr((void *)(flash_addr+length-1))))) {
        _show_invalid_flash_address(flash_addr, stat);
        return;
    }
    if ((stat = flash_lock((void *)flash_addr, length, (void **)&err_addr)) != 0) {
        diag_printf("Error locking at %p: %s\n", err_addr, flash_errmsg(stat));
    }
}

static void
fis_unlock(int argc, char *argv[])
{
    int stat;
    unsigned long length;
    CYG_ADDRESS flash_addr;
    bool flash_addr_set = false;
    bool length_set = false;
    void *err_addr;
    struct option_info opts[2];

    init_opts(&opts[0], 'f', true, OPTION_ARG_TYPE_NUM, 
              (void **)&flash_addr, (bool *)&flash_addr_set, "FLASH memory base address");
    init_opts(&opts[1], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void **)&length, (bool *)&length_set, "length");
    if (!scan_opts(argc, argv, 2, opts, 2, (void **)0, 0, ""))
    {
        fis_usage("invalid arguments");
        return;
    }

    if (!flash_addr_set || !length_set) {
        fis_usage("missing argument");
        return;
    }
    if (flash_addr_set &&
        ((stat = flash_verify_addr((void *)flash_addr)) ||
         (stat = flash_verify_addr((void *)(flash_addr+length-1))))) {
        _show_invalid_flash_address(flash_addr, stat);
        return;
    }
    if ((stat = flash_unlock((void *)flash_addr, length, (void **)&err_addr)) != 0) {
        diag_printf("Error unlocking at %p: %s\n", err_addr, flash_errmsg(stat));
    }
}
#endif

// This is set non-zero if the FLASH subsystem has successfully been initialized
static int __flash_init = 0;

void
_flash_info(void)
{
    if (!__flash_init) return;
    diag_printf("FLASH: %p - %p, %d blocks of %p bytes each.\n", 
           flash_start, (CYG_ADDRWORD)flash_end + 1, blocks, (void *)block_size);
}

static bool
do_flash_init(void)
{
    int stat;

    if (!__flash_init) {
        if ((stat = flash_init((void *)(workspace_end-FLASH_MIN_WORKSPACE), 
                               FLASH_MIN_WORKSPACE, diag_printf)) != 0) {
            diag_printf("FLASH: driver init failed: %s\n", flash_errmsg(stat));
            return false;
        }
        flash_get_limits((void *)0, (void **)&flash_start, (void **)&flash_end);
        // Keep 'end' address as last valid location, to avoid wrap around problems
        flash_end = (void *)((CYG_ADDRESS)flash_end - 1);
        flash_get_block_info(&block_size, &blocks);
        workspace_end = (unsigned char *)(workspace_end-FLASH_MIN_WORKSPACE);
#ifdef CYGOPT_REDBOOT_FIS
        workspace_end = (unsigned char *)(workspace_end-block_size);
        fis_work_block = workspace_end;
        fisdir_size = block_size;
#endif
        __flash_init = 1;
    }
    return true;
}

// Wrapper to avoid compiler warnings
static void
_do_flash_init(void)
{
    do_flash_init();
}

#ifndef CYGOPT_REDBOOT_FLASH_CONFIG
RedBoot_init(_do_flash_init, RedBoot_INIT_FIRST);
#endif

static void
do_fis(int argc, char *argv[])
{
    struct cmd *cmd;

    if (argc < 2) {
        fis_usage("too few arguments");
        return;
    }
    if (!do_flash_init()) {
        diag_printf("Sorry, no FLASH memory is available\n");
        return;
    }
#ifdef CYGOPT_REDBOOT_FIS
    if (CYGNUM_REDBOOT_FIS_DIRECTORY_BLOCK < 0) {
        fis_addr = (void *)((CYG_ADDRESS)flash_end + 1 +
                            (CYGNUM_REDBOOT_FIS_DIRECTORY_BLOCK*block_size));
    } else {
        fis_addr = (void *)((CYG_ADDRESS)flash_start + 
                            (CYGNUM_REDBOOT_FIS_DIRECTORY_BLOCK*block_size));
    }
    memcpy(fis_work_block, fis_addr, block_size);
#endif
    if ((cmd = cmd_search(__FIS_cmds_TAB__, &__FIS_cmds_TAB_END__, 
                          argv[1])) != (struct cmd *)0) {
        (cmd->fun)(argc, argv);
        return;
    }
    fis_usage("unrecognized command");
}

#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <flash_config.h>

// Configuration data, saved in FLASH, used to set/update RedBoot
// normal "configuration" data items.
static struct _config {
    unsigned long len;
    unsigned long key1;
    unsigned char config_data[MAX_CONFIG_DATA-(4*4)];
    unsigned long key2;
    unsigned long cksum;
} *config, *backup_config;
static bool config_ok;

#define CONFIG_KEY1    0x0BADFACE
#define CONFIG_KEY2    0xDEADDEAD

#define CONFIG_DONE    0
#define CONFIG_ABORT  -1
#define CONFIG_CHANGED 1
#define CONFIG_OK      2
#define CONFIG_BACK    3
#define CONFIG_BAD     4

// Note: the following options are related.  If 'boot_script' is false, then
// the other values are used in the configuration.  Because of the way
// that configuration tables are generated, they should have names which
// are related.  The configuration options will show up lexicographically
// ordered, thus the peculiar naming.
RedBoot_config_option("Run script at boot",
                      boot_script,
                      ALWAYS_ENABLED, true,
                      CONFIG_BOOL,
                      false
    );
RedBoot_config_option("Boot script",
                      boot_script_data,
                      "boot_script", true,
                      CONFIG_SCRIPT,
                      ""
    );
// Some preprocessor magic for building the [constant] prompt string
#define __cat(s1,c2,s3) s1 #c2 s3
#define _cat(s1,c2,s3) __cat(s1,c2,s3)
RedBoot_config_option(_cat("Boot script timeout (",
                           CYGNUM_REDBOOT_BOOT_SCRIPT_TIMEOUT_RESOLUTION,
                           "ms resolution)"),
                      boot_script_timeout,
                      "boot_script", true,
                      CONFIG_INT,
                      0
    );
#undef __cat
#undef _cat

#ifdef CYGSEM_REDBOOT_VARIABLE_BAUD_RATE
RedBoot_config_option("Console baud rate",
                      console_baud_rate,
                      ALWAYS_ENABLED, true,
                      CONFIG_INT,
                      CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD
    );
#endif

CYG_HAL_TABLE_BEGIN( __CONFIG_options_TAB__, RedBoot_config_options);
CYG_HAL_TABLE_END( __CONFIG_options_TAB_END__, RedBoot_config_options);

extern struct config_option __CONFIG_options_TAB__[], __CONFIG_options_TAB_END__[];

// 
// Layout of config data
// Each data item is variable length, with the name, type and dependencies
// encoded into the object.
//  offset   contents
//       0   data type
//       1   length of name (N)
//       2   enable sense
//       3   length of enable key (M)
//       4   key name
//     N+4   enable key
//   M+N+4   data value
//

#define CONFIG_OBJECT_TYPE(dp)          (dp)[0]
#define CONFIG_OBJECT_KEYLEN(dp)        (dp)[1]
#define CONFIG_OBJECT_ENABLE_SENSE(dp)  (dp)[2]
#define CONFIG_OBJECT_ENABLE_KEYLEN(dp) (dp)[3]
#define CONFIG_OBJECT_KEY(dp)           ((dp)+4)
#define CONFIG_OBJECT_ENABLE_KEY(dp)    ((dp)+4+CONFIG_OBJECT_KEYLEN(dp))
#define CONFIG_OBJECT_VALUE(dp)         ((dp)+4+CONFIG_OBJECT_KEYLEN(dp)+CONFIG_OBJECT_ENABLE_KEYLEN(dp))

#define LIST_OPT_LIST_ONLY (1)
#define LIST_OPT_NICKNAMES (2)
#define LIST_OPT_FULLNAMES (4)

static void config_init(void);

static int
get_config(unsigned char *dp, char *title, int list_opt, char *newvalue )
{
    char line[256], hold_line[256], *sp, *lp;
    int ret;
    bool hold_bool_val, new_bool_val, enable;
    unsigned long hold_int_val, new_int_val;
#ifdef CYGPKG_REDBOOT_NETWORKING
    in_addr_t hold_ip_val, new_ip_val;
    enet_addr_t hold_esa_val;
    int esa_ptr;
    char *esp;
#endif
    void *val_ptr;
    int type;

    if (CONFIG_OBJECT_ENABLE_KEYLEN(dp)) {
        flash_get_config(CONFIG_OBJECT_ENABLE_KEY(dp), &enable, CONFIG_BOOL);
        if (((bool)CONFIG_OBJECT_ENABLE_SENSE(dp) && !enable) ||
            (!(bool)CONFIG_OBJECT_ENABLE_SENSE(dp) && enable)) {
            return CONFIG_OK;  // Disabled field
        }
    }
    lp = line;  *lp = '\0';
    val_ptr = (void *)CONFIG_OBJECT_VALUE(dp);
    if (LIST_OPT_NICKNAMES & list_opt)
        diag_printf("%s: ", CONFIG_OBJECT_KEY(dp));
    if (LIST_OPT_FULLNAMES & list_opt) {
        if (title != (char *)NULL) {
            diag_printf("%s: ", title);
        } else {
            diag_printf("%s: ", CONFIG_OBJECT_KEY(dp));
        }
    }
    switch (type = CONFIG_OBJECT_TYPE(dp)) {
    case CONFIG_BOOL:
        memcpy(&hold_bool_val, val_ptr, sizeof(bool));
        lp += diag_sprintf(lp, "%s", hold_bool_val ? "true" : "false");
        break;
    case CONFIG_INT:
        memcpy(&hold_int_val, val_ptr, sizeof(unsigned long));
        lp += diag_sprintf(lp, "%ld", hold_int_val);
        break;
#ifdef CYGPKG_REDBOOT_NETWORKING
    case CONFIG_IP:
        lp += diag_sprintf(lp, "%s", inet_ntoa((in_addr_t *)val_ptr));
        if (0 == strcmp("0.0.0.0", line) && !(LIST_OPT_LIST_ONLY & list_opt)) {
            // then we have a deeply unhelpful starting text - kill it off
            // (unless we are just listing all values)
            lp = line;  *lp = '\0';
        }
        break;
    case CONFIG_ESA:
        for (esa_ptr = 0;  esa_ptr < sizeof(enet_addr_t);  esa_ptr++) {
            lp += diag_sprintf(lp, "0x%02X", ((unsigned char *)val_ptr)[esa_ptr]);
            if (esa_ptr < (sizeof(enet_addr_t)-1)) lp += diag_sprintf(lp, ":");
        }
        break;
#endif
    case CONFIG_STRING:
        lp += diag_sprintf(lp, "%s", (unsigned char *)val_ptr);
        break;
    case CONFIG_SCRIPT:
        diag_printf("\n");
        sp = lp = (unsigned char *)val_ptr;
        while (*sp) {
            while (*lp != '\n') lp++;
            *lp = '\0';
            diag_printf(".. %s\n", sp);
            *lp++ = '\n';
            sp = lp;
        }
        break;
    }
    if (LIST_OPT_LIST_ONLY & list_opt) {
        diag_printf("%s\n", line);
        return CONFIG_OK;
    }
    if (type != CONFIG_SCRIPT) {
        if (NULL != newvalue) {
            ret = strlen(newvalue);
            if (ret > sizeof(line))
                return CONFIG_BAD;
            strcpy(hold_line, line); // Hold the old value for comparison
            strcpy(line, newvalue);
            diag_printf("Setting to %s\n", newvalue);
        } else {
            // read from terminal
            strcpy(hold_line, line);
            ret = _rb_gets_preloaded(line, sizeof(line), 0);
        }
        if (ret < 0) return CONFIG_ABORT;
        // empty input - leave value untouched (else DNS goes away for a
        // minute to try to look it up) but we must accept empty value for strings.
        if (0 == line[0] && CONFIG_STRING != type) return CONFIG_OK; 
        if (strcmp(line, hold_line) == 0) return CONFIG_OK;  // Just a CR - leave value untouched
        lp = &line[strlen(line)-1];
        if (*lp == '.') return CONFIG_DONE;
        if (*lp == '^') return CONFIG_BACK;
    }
    switch (type) {
    case CONFIG_BOOL:
        memcpy(&hold_bool_val, val_ptr, sizeof(bool));
        if (!parse_bool(line, &new_bool_val)) {
            return CONFIG_BAD;
        }
        if (hold_bool_val != new_bool_val) {
            memcpy(val_ptr, &new_bool_val, sizeof(bool));
            return CONFIG_CHANGED;
        } else {
            return CONFIG_OK;
        }
        break;
    case CONFIG_INT:
        memcpy(&hold_int_val, val_ptr, sizeof(unsigned long));
        if (!parse_num(line, &new_int_val, 0, 0)) {
            return CONFIG_BAD;
        }
        if (hold_int_val != new_int_val) {
            memcpy(val_ptr, &new_int_val, sizeof(unsigned long));
            return CONFIG_CHANGED;
        } else {
            return CONFIG_OK;
        }
        break;
#ifdef CYGPKG_REDBOOT_NETWORKING
    case CONFIG_IP:
        memcpy(&hold_ip_val.s_addr, &((in_addr_t *)val_ptr)->s_addr, sizeof(in_addr_t));
        if (!_gethostbyname(line, &new_ip_val)) {
            return CONFIG_BAD;
        }
        if (hold_ip_val.s_addr != new_ip_val.s_addr) {
            memcpy(val_ptr, &new_ip_val, sizeof(in_addr_t));
            return CONFIG_CHANGED;
        } else {
            return CONFIG_OK;
        }
        break;
    case CONFIG_ESA:
        memcpy(&hold_esa_val, val_ptr, sizeof(enet_addr_t));
        esp = line;
        for (esa_ptr = 0;  esa_ptr < sizeof(enet_addr_t);  esa_ptr++) {
            unsigned long esa_byte;
            if (!parse_num(esp, &esa_byte, &esp, ":")) {
                memcpy(val_ptr, &hold_esa_val, sizeof(enet_addr_t));
                return CONFIG_BAD;
            }
            ((unsigned char *)val_ptr)[esa_ptr] = esa_byte;
        }
        return CONFIG_CHANGED;
        break;
#endif
    case CONFIG_SCRIPT:
        // Assume it always changes
        sp = (unsigned char *)val_ptr;
        diag_printf("Enter script, terminate with empty line\n");
        while (true) {
            *sp = '\0';
            diag_printf(">> ");
            ret = _rb_gets(line, sizeof(line), 0);
            if (ret < 0) return CONFIG_ABORT;
            if (strlen(line) == 0) break;
            lp = line;
            while (*lp) {
                *sp++ = *lp++;
            }
            *sp++ = '\n';
        }
        break;
    case CONFIG_STRING:
        if (strlen(line) >= MAX_STRING_LENGTH) {
            diag_printf("Sorry, value is too long\n");
            return CONFIG_BAD;
        }
        strcpy((unsigned char *)val_ptr, line);
        break;
    }
    return CONFIG_CHANGED;
}

//
// Manage configuration information with the FLASH
//

static int
config_length(int type)
{
    switch (type) {
    case CONFIG_BOOL:
        return sizeof(bool);
    case CONFIG_INT:
        return sizeof(unsigned long);
#ifdef CYGPKG_REDBOOT_NETWORKING
    case CONFIG_IP:
        return sizeof(in_addr_t);
    case CONFIG_ESA:
        // Would like this to be sizeof(enet_addr_t), but that causes much
        // pain since it fouls the alignment of data which follows.
        return 8;
#endif
    case CONFIG_STRING:
        return MAX_STRING_LENGTH;
    case CONFIG_SCRIPT:
        return MAX_SCRIPT_LENGTH;
    default:
        return 0;
    }
}

static cmd_fun do_flash_config;
RedBoot_cmd("fconfig",
            "Manage configuration kept in FLASH memory",
            "[-i] [-l] [-n] [-f] | nickname [value]",
            do_flash_config
    );

static void
do_flash_config(int argc, char *argv[])
{
    bool need_update = false;
    struct config_option *optend = __CONFIG_options_TAB_END__;
    struct config_option *opt = __CONFIG_options_TAB__;
    struct option_info opts[4];
    bool list_only;
    bool nicknames;
    bool fullnames;
    int list_opt = 0;
    unsigned char *dp;
    int len, ret;
    char *title;
    char *onlyone = NULL;
    char *onevalue = NULL;
    bool doneone = false;
    bool init = false;

    if (!__flash_init) {
        diag_printf("Sorry, no FLASH memory is available\n");
        return;
    }
    memcpy(backup_config, config, sizeof(struct _config));
    script = (unsigned char *)0;

    init_opts(&opts[0], 'l', false, OPTION_ARG_TYPE_FLG, 
                  (void **)&list_only, (bool *)0, "list configuration only");
    init_opts(&opts[1], 'n', false, OPTION_ARG_TYPE_FLG, 
              (void **)&nicknames, (bool *)0, "show nicknames");
    init_opts(&opts[2], 'f', false, OPTION_ARG_TYPE_FLG, 
              (void **)&fullnames, (bool *)0, "show full names");
    init_opts(&opts[3], 'i', false, OPTION_ARG_TYPE_FLG, 
              (void **)&init, (bool *)0, "initialize configuration database");

    // First look to see if we are setting or getting a single option
    // by just quoting its nickname
    if ( (2 == argc && '-' != argv[1][0]) ||
         (3 == argc && '-' != argv[1][0] && '-' != argv[2][0])) {
        // then the command was "fconfig foo [value]"
        onlyone = argv[1];
        onevalue = (3 == argc) ? argv[2] : NULL;
        list_opt = LIST_OPT_NICKNAMES;
    }
    else {
        if (!scan_opts(argc, argv, 1, opts, 4, 0, 0, ""))
            return;
        list_opt |= list_only ? LIST_OPT_LIST_ONLY : 0;
        list_opt |= nicknames ? LIST_OPT_NICKNAMES : LIST_OPT_FULLNAMES;
        list_opt |= fullnames ? LIST_OPT_FULLNAMES : 0;
    }

    if (init && verify_action("Initialize non-volatile configuration")) {
        config_init();
    }

    dp = &config->config_data[0];
    while (dp < &config->config_data[sizeof(config->config_data)]) {
        if (CONFIG_OBJECT_TYPE(dp) == CONFIG_EMPTY) {
            break;
        }
        len = 4 + CONFIG_OBJECT_KEYLEN(dp) + CONFIG_OBJECT_ENABLE_KEYLEN(dp) + 
            config_length(CONFIG_OBJECT_TYPE(dp));
        // Provide a title for well known [i.e. builtin] objects
        title = (char *)NULL;
        opt = __CONFIG_options_TAB__;
        while (opt != optend) {
            if (strcmp(opt->key, CONFIG_OBJECT_KEY(dp)) == 0) {
                title = opt->title;
                break;
            }
            opt++;
        }
        if ( onlyone && 0 != strcmp(CONFIG_OBJECT_KEY(dp), onlyone) )
            ret = CONFIG_OK; // skip this entry
        else {
            doneone = true;
            ret = get_config(dp, title, list_opt, onevalue); // do this opt
        }
        switch (ret) {
        case CONFIG_DONE:
            goto done;
        case CONFIG_ABORT:
            memcpy(config, backup_config, sizeof(struct _config));
            return;
        case CONFIG_CHANGED:
            need_update = true;
        case CONFIG_OK:
            dp += len;
            break;
        case CONFIG_BACK:
            dp = &config->config_data[0];
            continue;
        case CONFIG_BAD:
            // Nothing - make him do it again
            diag_printf ("** invalid entry\n");
            onevalue = NULL; // request a good value be typed in - or abort/whatever
        }
    }

 done:
    if (NULL != onlyone && !doneone) {
#ifdef CYGSEM_REDBOOT_ALLOW_DYNAMIC_FLASH_CONFIG_DATA
        if (verify_action("** entry '%s' not found - add", onlyone)) {
            struct config_option opt;
            diag_printf("Trying to add value\n");
        }
#else
        diag_printf("** entry '%s' not found", onlyone);
#endif
    }
    if (!need_update)
        return;
    flash_write_config();
}


#ifdef CYGSEM_REDBOOT_FLASH_ALIASES
static cmd_fun do_alias;
RedBoot_cmd("alias",
            "Manage aliases kept in FLASH memory",
            "name [value]",
            do_alias
    );

static void
make_alias(char *alias, char *name)
{
    diag_sprintf(alias, "alias/%s", name);
}

static void
do_alias(int argc, char *argv[])
{
    char name[80];
    char *val;
    struct config_option opt;

    switch (argc) {
    case 2:
        make_alias(name, argv[1]);
        if (flash_get_config(name, &val, CONFIG_STRING)) {
            diag_printf("'%s' = '%s'\n", argv[1], val);
        } else {
            diag_printf("'%s' not found\n", argv[1]);
        }
        break;
    case 3:
        if (strlen(argv[2]) >= MAX_STRING_LENGTH) {
            diag_printf("Sorry, value is too long\n");
            break;
        }
        make_alias(name, argv[1]);
        opt.type = CONFIG_STRING;
        opt.enable = (char *)0;
        opt.enable_sense = 1;
        opt.key = name;
        opt.dflt = (CYG_ADDRESS)argv[2];
        flash_add_config(&opt, true);
        break;
    default:
        diag_printf("usage: alias name [value]\n");
    }
}

// Lookup an alias. First try plain string aliases. If that fails try
// other types so allowing access to all configured values. This allows
// for alias (macro) expansion of normal 'fconfig' data, such as the
// board IP address.
static char *
lookup_alias(char *alias, char *alias_buf)
{
    char name[80];
    char *val;
    unsigned char * dp;
    void *val_ptr;
    int type;
    bool hold_bool_val;
    long hold_long_val;
#ifdef CYGPKG_REDBOOT_NETWORKING
    int esa_ptr;
#endif

    make_alias(name, alias);
    if (flash_get_config(name, &val, CONFIG_STRING)) {
        return val;
    } else {
        dp = flash_lookup_config(alias);
        if (dp) {
            val_ptr = (void *)CONFIG_OBJECT_VALUE(dp);
            switch (type = CONFIG_OBJECT_TYPE(dp)) {
            case CONFIG_BOOL:
                memcpy(&hold_bool_val, val_ptr, sizeof(bool));
                diag_sprintf(alias_buf, "%s", hold_bool_val ? "true" : "false");
                break;
            case CONFIG_INT:
                memcpy(&hold_long_val, val_ptr, sizeof(unsigned long));
                diag_sprintf(alias_buf,"%ld", hold_long_val);
                break;
#ifdef CYGPKG_REDBOOT_NETWORKING
            case CONFIG_IP:
                diag_sprintf(alias_buf,"%s", inet_ntoa((in_addr_t *)val_ptr));
                break;
            case CONFIG_ESA:
                for (esa_ptr = 0;  esa_ptr < sizeof(enet_addr_t);  esa_ptr++) {
                    diag_sprintf(alias_buf+(3*esa_ptr), "0x%02X", ((unsigned char *)val_ptr)[esa_ptr]);
                    if (esa_ptr < (sizeof(enet_addr_t)-1)) diag_printf(":");
                }
                break;
#endif
            case CONFIG_SCRIPT:
                return (char *) val_ptr;
                break;
            default:
                return (char *)NULL;
            }
            return alias_buf;
        } 
        return (char *)NULL;
    }
}

// Expand aliases, this is recursive. ie if one alias contains other
// aliases, these will also be expanded from the insertion point
// onwards.
//
// If 'iter' is zero, then quoted strings are not expanded
//
bool
_expand_aliases(char *line, int len, int iter)
{
    char *lp = line;
    char *ms, *me, *ep;
    char *alias;
    char c;
    int offset, line_len, alias_len;
    char alias_buf[MAX_STRING_LENGTH];
    bool macro_found = false;

    if ((line_len = strlen(line)) != 0) {
        while (*lp) {
            c = *lp++;
            if ((c == '%') && (*lp == '{')) {
                // Found a macro/alias to expand
                ms = lp+1;
                lp += 2;
                while (*lp && (*lp != '}')) lp++;
                if (!*lp) {
                    diag_printf("Invalid macro/alias '%s'\n", ms);
                    line[0] = '\0';  // Destroy line
                    return false;
                }
                me = lp;
                *me = '\0';
                lp++;
                if ((alias = lookup_alias(ms,alias_buf)) != (char *)NULL) {
                    alias_len = strlen(alias);
                    // See if there is room in the line to expand this macro/alias
                    if ((line_len+alias_len) < len) {
                        // Make a hole by moving data within the line
                        offset = alias_len-strlen(ms)-2;  // Number of bytes being inserted
			ep = &lp[strlen(lp)-1];
			if (offset > 1) {
                            ep[offset] = '\0';
                            while (ep != (lp-1)) {
                                ep[offset-1] = *ep;
                                ep--;
                            }           
			} else {
                            if (offset <=0) {
                                while ((lp-1) != ep) {
                                    lp[offset-1] = *lp;
                                    lp++;
                                }
                                lp[offset-1]='\0';
                            }
			}
                        // Insert the macro/alias data
                        lp = ms-2;
                        while (*alias) {
                            if ((alias[0] == '%') && (alias[1] == '{')) macro_found = true;
                            *lp++ = *alias++;
                        }
                        line_len = strlen(line);
			lp = lp - alias_len;
                    } else {
                        diag_printf("No room to expand '%s'\n", ms);
                        line[0] = '\0';  // Destroy line
                        return false;
                    }
                } else {
                    diag_printf("Alias '%s' not defined\n", ms);
                    *me = '|';
                }
            } else if ((c == '"') && (iter == 0)) {
                // Skip quoted strings
                while (*lp && (*lp != '"')) lp++;
            }            
        }
    }
    return macro_found;
}

void
expand_aliases(char *line, int len)
{
    int iter = 0;

    while (_expand_aliases(line, len, iter++)) {
    }
}
#endif //  CYGSEM_REDBOOT_FLASH_ALIASES

//
// Write the in-memory copy of the configuration data to the flash device.
//
void
flash_write_config(void)
{
    int stat;
    void *err_addr;

    config->len = sizeof(struct _config);
    config->key1 = CONFIG_KEY1;  
    config->key2 = CONFIG_KEY2;
    config->cksum = crc32((unsigned char *)config, sizeof(struct _config)-sizeof(config->cksum));
    if (verify_action("Update RedBoot non-volatile configuration")) {
#ifdef CYGSEM_REDBOOT_FLASH_LOCK_SPECIAL
        // Insure [quietly] that the config page is unlocked before trying to update
        flash_unlock((void *)cfg_base, cfg_size, (void **)&err_addr);
#endif
        if ((stat = flash_erase(cfg_base, cfg_size, (void **)&err_addr)) != 0) {
            diag_printf("   initialization failed at %p: %s\n", err_addr, flash_errmsg(stat));
        } else {
            if ((stat = flash_program(cfg_base, (void *)config, sizeof(struct _config), 
                                      (void **)&err_addr)) != 0) {
                diag_printf("Error writing config data at %p: %s\n", 
                            err_addr, flash_errmsg(stat));
            }
        }
#ifdef CYGSEM_REDBOOT_FLASH_LOCK_SPECIAL
        // Insure [quietly] that the config data is locked after the update
        flash_lock((void *)cfg_base, cfg_size, (void **)&err_addr);
#endif
    }
}

//
// Find the configuration entry for a particular key
//
static unsigned char *
flash_lookup_config(char *key)
{
    unsigned char *dp;
    int len;

    if (!config_ok) return (unsigned char *)NULL;

    dp = &config->config_data[0];
    while (dp < &config->config_data[sizeof(config->config_data)]) {
        len = 4 + CONFIG_OBJECT_KEYLEN(dp) + CONFIG_OBJECT_ENABLE_KEYLEN(dp) +
            config_length(CONFIG_OBJECT_TYPE(dp));
        if (strcmp(key, CONFIG_OBJECT_KEY(dp)) == 0) {
            return dp;
        }
        dp += len;
    }
//    diag_printf("Can't find config data for '%s'\n", key);
    return false;
}

//
// Retrieve a data object from the data base (in memory copy)
//
bool
flash_get_config(char *key, void *val, int type)
{
    unsigned char *dp;
    void *val_ptr;

    if (!config_ok) return false;

    if ((dp = flash_lookup_config(key)) != (unsigned char *)NULL) {
        if (CONFIG_OBJECT_TYPE(dp) == type) {
            val_ptr = (void *)CONFIG_OBJECT_VALUE(dp);
            switch (type) {
                // Note: the data may be unaligned in the configuration data
            case CONFIG_BOOL:
                memcpy(val, val_ptr, sizeof(bool));
                break;
            case CONFIG_INT:
                memcpy(val, val_ptr, sizeof(unsigned long));
                break;
#ifdef CYGPKG_REDBOOT_NETWORKING
            case CONFIG_IP:
                memcpy(val, val_ptr, sizeof(in_addr_t));
                break;
            case CONFIG_ESA:
                memcpy(val, val_ptr, sizeof(enet_addr_t));
                break;
#endif
            case CONFIG_STRING:
            case CONFIG_SCRIPT:
                // Just return a pointer to the script/line
                *(unsigned char **)val = (unsigned char *)val_ptr;
                break;
            }
        } else {
            diag_printf("Request for config value '%s' - wrong type\n", key);
        }
        return true;
    }
    return false;
}

//
// Copy data into the config area
//
static void
flash_config_insert_value(unsigned char *dp, struct config_option *opt)
{
    switch (opt->type) {
        // Note: the data may be unaligned in the configuration data
    case CONFIG_BOOL:
        memcpy(dp, (void *)&opt->dflt, sizeof(bool));
        break;
    case CONFIG_INT:
        memcpy(dp, (void *)&opt->dflt, sizeof(unsigned long));
        break;
#ifdef CYGPKG_REDBOOT_NETWORKING
    case CONFIG_IP:
        memcpy(dp, (void *)&opt->dflt, sizeof(in_addr_t));
        break;
    case CONFIG_ESA:
        memcpy(dp, (void *)&opt->dflt, sizeof(enet_addr_t));
        break;
#endif
    case CONFIG_STRING:
        memcpy(dp, (void *)opt->dflt, config_length(CONFIG_STRING));
        break;
    case CONFIG_SCRIPT:
        break;
    }
}

//
// Add a new option to the database
//
bool
flash_add_config(struct config_option *opt, bool update)
{
    unsigned char *dp, *kp;
    int len, elen, size;

    // If data item is already present, just update it
    // Note: only the data value can be thusly changed
    if ((dp = flash_lookup_config(opt->key)) != (unsigned char *)NULL) {
        flash_config_insert_value(CONFIG_OBJECT_VALUE(dp), opt);
        if (update) {
            flash_write_config();
        }
        return true;
    }
    // Add the data item
    dp = &config->config_data[0];
    size = 0;
    while (size < sizeof(config->config_data)) {
        if (CONFIG_OBJECT_TYPE(dp) == CONFIG_EMPTY) {
            kp = opt->key;
            len = strlen(kp) + 1;
            size += len + 2 + 2 + config_length(opt->type);
            if (opt->enable) {
                elen = strlen(opt->enable) + 1;
                size += elen;
            } else {
                elen = 0;
            }
            if (size > sizeof(config->config_data)) {
                break;
            }
            CONFIG_OBJECT_TYPE(dp) = opt->type; 
            CONFIG_OBJECT_KEYLEN(dp) = len;
            CONFIG_OBJECT_ENABLE_SENSE(dp) = opt->enable_sense;
            CONFIG_OBJECT_ENABLE_KEYLEN(dp) = elen;
            dp = CONFIG_OBJECT_KEY(dp);
            while (*kp) *dp++ += *kp++;
            *dp++ = '\0';    
            if (elen) {
                kp = opt->enable;
                while (*kp) *dp++ += *kp++;
                *dp++ = '\0';    
            }
            flash_config_insert_value(dp, opt);
            if (update) {
                flash_write_config();
            }
            return true;
        } else {
            len = 4 + CONFIG_OBJECT_KEYLEN(dp) + CONFIG_OBJECT_ENABLE_KEYLEN(dp) +
                config_length(CONFIG_OBJECT_TYPE(dp));
            dp += len;
            size += len;
        }
    }
    diag_printf("No space to add '%s'\n", opt->key);
    return false;
}

//
// Reset/initialize configuration data - used only when starting from scratch
//
static void
config_init(void)
{
    // Well known option strings
    struct config_option *optend = __CONFIG_options_TAB_END__;
    struct config_option *opt = __CONFIG_options_TAB__;

    memset(config, 0, sizeof(struct _config));
    while (opt != optend) {
        if (!flash_add_config(opt, false)) {
            return;
        }
        opt++;
    }
    config_ok = true;
}

//
// Attempt to get configuration information from the FLASH.
// If available (i.e. good checksum, etc), initialize "known"
// values for later use.
//
static void
load_flash_config(void)
{
    bool use_boot_script;

    config_ok = false;
    script = (unsigned char *)0;
    if (!do_flash_init()) return;
    config = (struct _config *)(workspace_end-sizeof(struct _config));
    backup_config = (struct _config *)((CYG_ADDRESS)config-sizeof(struct _config));
    workspace_end = (unsigned char *)backup_config;
#define _roundup(n,s) ((((n)+(s-1))/s)*s)
    cfg_size = (block_size > sizeof(struct _config)) ? 
        sizeof(struct _config) : 
        _roundup(sizeof(struct _config), block_size);
    if (CYGNUM_REDBOOT_FLASH_CONFIG_BLOCK < 0) {
        cfg_base = (void *)((CYG_ADDRESS)flash_end + 1 +
                            (CYGNUM_REDBOOT_FLASH_CONFIG_BLOCK*block_size));
    } else {
        cfg_base = (void *)((CYG_ADDRESS)flash_start + 
                            (CYGNUM_REDBOOT_FLASH_CONFIG_BLOCK*block_size));
    }
    memcpy(config, cfg_base, sizeof(struct _config));
    if ((crc32((unsigned char *)config, 
               sizeof(struct _config)-sizeof(config->cksum)) != config->cksum) ||
        (config->key1 != CONFIG_KEY1)|| (config->key2 != CONFIG_KEY2)) {
        diag_printf("FLASH configuration checksum error or invalid key\n");
        config_init();
        return;
    }
    config_ok = true;
    flash_get_config("boot_script", &use_boot_script, CONFIG_BOOL);
    if (use_boot_script) {
        flash_get_config("boot_script_data", &script, CONFIG_SCRIPT);
        flash_get_config("boot_script_timeout", &script_timeout, CONFIG_INT);
    }
#ifdef CYGSEM_REDBOOT_VARIABLE_BAUD_RATE
    if (flash_get_config("console_baud_rate", &console_baud_rate, CONFIG_INT)) {
        extern void set_console_baud_rate(int);
        set_console_baud_rate(console_baud_rate);
    }
#endif
}

RedBoot_init(load_flash_config, RedBoot_INIT_FIRST);

#endif // CYGSEM_REDBOOT_FLASH_CONFIG

// EOF flash.c
