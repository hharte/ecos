//==========================================================================
//
//      load.c
//
//      RedBoot file/image loader
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, tsmith
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>
#include <elf.h>
#ifdef CYGBLD_BUILD_REDBOOT_WITH_XYZMODEM
#include <xyzModem.h>
#endif
#ifdef CYGPKG_REDBOOT_DISK
#include <fs/disk.h>
#endif
#ifdef CYGPKG_REDBOOT_FILEIO
#include <fs/fileio.h>
#endif
#ifdef CYGPKG_REDBOOT_NETWORKING
#ifdef CYGSEM_REDBOOT_NET_TFTP_DOWNLOAD
#include <net/tftp_support.h>
#endif
#ifdef CYGSEM_REDBOOT_NET_HTTP_DOWNLOAD
#include <net/http.h>
#endif
#endif
#include <cyg/infra/cyg_ass.h>         // assertion macros
#ifdef CYGBLD_REDBOOT_LOAD_INTO_FLASH
#include <cyg/io/flash.h>
#include "flash_load.h"
#endif

static char usage[] = "[-r] [-v] "
#ifdef CYGBLD_BUILD_REDBOOT_WITH_ZLIB
                      "[-d] "
#endif
#ifdef CYGPKG_REDBOOT_NETWORKING
                      "[-h <host>] [-p <TCP port>]"
#endif
                      "[-m <varies>] "
#if CYGNUM_HAL_VIRTUAL_VECTOR_NUM_CHANNELS > 1
                      "[-c <channel_number>] "
#endif
#ifdef CYGBLD_REDBOOT_LOAD_INTO_FLASH
                      "[-f <flash_address>] "
#endif
                      "\n        [-b <base_address>] <file_name>";

// Exported CLI function
RedBoot_cmd("load", 
            "Load a file", 
            usage,
            do_load 
    );

//
// Stream I/O support
//

// Table describing the various I/O methods
CYG_HAL_TABLE_BEGIN( __RedBoot_LOAD_TAB__, RedBoot_load );
CYG_HAL_TABLE_END( __RedBoot_LOAD_TAB_END__, RedBoot_load );
extern struct load_io_entry __RedBoot_LOAD_TAB__[], __RedBoot_LOAD_TAB_END__;

// Buffers, data used by redboot_getc
#define BUF_SIZE CYGNUM_REDBOOT_GETC_BUFFER
struct {
    getc_io_funcs_t *io;
    int (*fun)(char *, int len, int *err);
    char  buf[BUF_SIZE];
    char *bufp;
    int   avail, len, err;
    int   verbose, decompress, tick;
#ifdef CYGBLD_BUILD_REDBOOT_WITH_ZLIB
    int (*raw_fun)(char *, int len, int *err);
    _pipe_t load_pipe;
    unsigned char _buffer[CYGNUM_REDBOOT_LOAD_ZLIB_BUFFER];
#endif
} getc_info;

typedef int (*getc_t)(void);

//
// Read the next data byte from the stream.
// Returns:
//    >= 0 - actual data
//      -1 - error or EOF, status in getc_info.err
//
static int 
redboot_getc(void)
{
    static char spin[] = "|/-\\|-";
    if (getc_info.avail < 0) {
      return -1;
    }
    if (getc_info.avail == 0) {
        if (getc_info.verbose) {
            err_printf("%c\b", spin[getc_info.tick++]);
            if (getc_info.tick >= sizeof(spin)) {
                getc_info.tick = 0;
            }
        }
        if (getc_info.len < BUF_SIZE) {
            // No more data available
            if (getc_info.verbose) diag_printf("\n");
            return -1;
        }
        getc_info.bufp = getc_info.buf;
        getc_info.len = (*getc_info.fun)(getc_info.bufp, BUF_SIZE, &getc_info.err);
        if ((getc_info.avail = getc_info.len) <= 0) {
            if (getc_info.len < 0) {
                diag_printf("I/O error: %s\n", (getc_info.io->error)(getc_info.err));
            }
            if (getc_info.verbose) diag_printf("\n");
            return -1;
        }
    }
    getc_info.avail--;
    return ((int)*getc_info.bufp++) & 0x00FF;
}

#ifdef CYGBLD_BUILD_REDBOOT_WITH_ZLIB
//
// Called to fetch a new chunk of data and decompress it
//
static int 
_decompress_stream(char *buf, int len, int *err)
{
    _pipe_t* p = &getc_info.load_pipe;
    int res, total;

    total = 0;
    while (len > 0) {
        if (p->in_avail == 0) {
            p->in_buf = &getc_info._buffer[0];
            res = (*getc_info.raw_fun)((char *)p->in_buf, CYGNUM_REDBOOT_LOAD_ZLIB_BUFFER,
                                       &getc_info.err);
            if ((p->in_avail = res) <= 0) {
                // No more data
                return total;
            }
        }
        p->out_buf = (unsigned char *)buf;
        p->out_size = 0;
        p->out_max = len;
        res = (*_dc_inflate)(p);
        if (res != 0) {
            *err = res;
            return total;
        }        
        len -= p->out_size;
        buf += p->out_size;
        total += p->out_size;
    }
    return total;
}
#endif

static int
redboot_getc_init(connection_info_t *info, getc_io_funcs_t *funcs, 
                  int verbose, int decompress)
{
    int res;

    res = (funcs->open)(info, &getc_info.err);    
    if (res < 0) {
        err_printf("Can't load '%s': %s\n", info->filename, (funcs->error)(getc_info.err));
            return res;
    }
    getc_info.io = funcs;
    getc_info.fun = funcs->read;
    getc_info.avail = 0;
    getc_info.len = BUF_SIZE;
    getc_info.verbose = verbose;
    getc_info.decompress = decompress;
    getc_info.tick = 0;
#ifdef CYGBLD_BUILD_REDBOOT_WITH_ZLIB
    if (decompress) {
        _pipe_t* p = &getc_info.load_pipe;
        p->out_buf = (unsigned char *)&getc_info.buf[0];
        p->out_size = 0;
        p->in_avail = 0;
        getc_info.raw_fun = getc_info.fun;
        getc_info.fun = _decompress_stream;
        getc_info.err = (*_dc_init)(p);
        if (0 != getc_info.err && p->msg) {
            err_printf("open decompression error: %s\n", p->msg);
        }
    }
#endif
    return 0;
}

static void
redboot_getc_rewind(void)
{
    getc_info.bufp = getc_info.buf;
    getc_info.avail = getc_info.len;
}

static void
redboot_getc_terminate(bool abort)
{
    if (getc_info.io->terminate) {
        (getc_info.io->terminate)(abort, redboot_getc);
    }
}

static void
redboot_getc_close(void)
{
    (getc_info.io->close)(&getc_info.err);
#ifdef CYGBLD_BUILD_REDBOOT_WITH_ZLIB
    if (getc_info.decompress) {
        _pipe_t* p = &getc_info.load_pipe;
        int err = getc_info.err;
        if (0 != err && p->msg) {
            diag_printf("decompression error: %s\n", p->msg);
        }
        err = (*_dc_close)(p, getc_info.err);
    }
#endif
}

#ifdef CYGSEM_REDBOOT_ELF
//
// Support function - used to read bytes into a buffer
// Returns the number of bytes read (stops short on errors)
//
static int
_read(int (*getc)(void), unsigned char *buf, int len)
{
    int total = 0;
    int ch;
    while (len-- > 0) {
        ch = (*getc)();
        if (ch < 0) {
            // EOF or error
            break;
        }
        *buf++ = ch;
        total++;
    }
    return total;
}
#endif

//
// Load an ELF [binary] image 
//
static unsigned long
load_elf_image(getc_t getc, unsigned long base)
{
#ifdef CYGSEM_REDBOOT_ELF
    Elf32_Ehdr ehdr;
#define MAX_PHDR 8
    Elf32_Phdr phdr[MAX_PHDR];
    unsigned long offset = 0;
    int phx, len, ch;
    unsigned char *addr;
    unsigned long addr_offset = 0;
    unsigned long highest_address = 0;
    unsigned long lowest_address = 0xFFFFFFFF;
    const char SHORT_DATA[] = "Short data reading ELF file\n";

    // Read the header
    if (_read(getc, (unsigned char *)&ehdr, sizeof(ehdr)) != sizeof(ehdr)) {
        err_printf("Can't read ELF header\n");
        redboot_getc_terminate(true);
        return 0;
    }
    offset += sizeof(ehdr);    
#if 0 // DEBUG
    diag_printf("Type: %d, Machine: %d, Version: %d, Entry: %p, PHoff: %p/%d/%d, SHoff: %p/%d/%d\n",
                ehdr.e_type, ehdr.e_machine, ehdr.e_version, ehdr.e_entry, 
                ehdr.e_phoff, ehdr.e_phentsize, ehdr.e_phnum,
                ehdr.e_shoff, ehdr.e_shentsize, ehdr.e_shnum);
#endif
    if (ehdr.e_type != ET_EXEC) {
        err_printf("Only absolute ELF images supported\n");
        redboot_getc_terminate(true);
        return 0;
    }
    if (ehdr.e_phnum > MAX_PHDR) {
        err_printf("Too many program headers\n");
        redboot_getc_terminate(true);
        return 0;
    }
    while (offset < ehdr.e_phoff) {
        if ((*getc)() < 0) {
            err_printf(SHORT_DATA);
            redboot_getc_terminate(true);
            return 0;
        }
        offset++;
    }
    for (phx = 0;  phx < ehdr.e_phnum;  phx++) {
        if (_read(getc, (unsigned char *)&phdr[phx], sizeof(phdr[0])) != sizeof(phdr[0])) {
            err_printf("Can't read ELF program header\n");
            redboot_getc_terminate(true);
            return 0;
        }
#if 0 // DEBUG
        diag_printf("Program header: type: %d, off: %p, va: %p, pa: %p, len: %d/%d, flags: %d\n",
                    phdr[phx].p_type, phdr[phx].p_offset, phdr[phx].p_vaddr, phdr[phx].p_paddr,
                    phdr[phx].p_filesz, phdr[phx].p_memsz, phdr[phx].p_flags);
#endif
        offset += sizeof(phdr[0]);
    }
    if (base) {
        // Set address offset based on lowest address in file.
        addr_offset = 0xFFFFFFFF;
        for (phx = 0;  phx < ehdr.e_phnum;  phx++) {
#ifdef CYGOPT_REDBOOT_ELF_VIRTUAL_ADDRESS     
            if ((phdr[phx].p_type == PT_LOAD) && (phdr[phx].p_vaddr < addr_offset)) {
                addr_offset = phdr[phx].p_vaddr;
#else
            if ((phdr[phx].p_type == PT_LOAD) && (phdr[phx].p_paddr < addr_offset)) {
                addr_offset = phdr[phx].p_paddr;
#endif
            }
        }
        addr_offset = (unsigned long)base - addr_offset;
    } else {
        addr_offset = 0;
    }
    for (phx = 0;  phx < ehdr.e_phnum;  phx++) {
        if (phdr[phx].p_type == PT_LOAD) {
            // Loadable segment
#ifdef CYGOPT_REDBOOT_ELF_VIRTUAL_ADDRESS
            addr = (unsigned char *)phdr[phx].p_vaddr;
#else     
            addr = (unsigned char *)phdr[phx].p_paddr;
#endif
            len = phdr[phx].p_filesz;
            if ((unsigned long)addr < lowest_address) {
                lowest_address = (unsigned long)addr;
            }
            addr += addr_offset;
            if (offset > phdr[phx].p_offset) {
                if ((phdr[phx].p_offset + len) < offset) {
                    err_printf("Can't load ELF file - program headers out of order\n");
                    redboot_getc_terminate(true);
                    return 0;
                }
                addr += offset - phdr[phx].p_offset;
            } else {
                while (offset < phdr[phx].p_offset) {
                    if ((*getc)() < 0) {
                        err_printf(SHORT_DATA);
                        redboot_getc_terminate(true);
                        return 0;
                    }
                    offset++;
                }
            }

            // Copy data into memory
            while (len-- > 0) {
#ifdef CYGSEM_REDBOOT_VALIDATE_USER_RAM_LOADS
                if (!(valid_address(addr) 
#ifdef CYGBLD_REDBOOT_LOAD_INTO_FLASH
                    || (flash_verify_addr(addr) == FLASH_ERR_OK)
#endif
                    )) {
                    redboot_getc_terminate(true);
                    err_printf("*** Abort! Attempt to load ELF data to address: %p which is not valid\n", (void*)addr);
                    return 0;
                }
#endif
                if ((ch = (*getc)()) < 0) {
                    err_printf(SHORT_DATA);
                    redboot_getc_terminate(true);
                    return 0;
                }
#ifdef CYGSEM_REDBOOT_VALIDATE_USER_RAM_LOADS
                if (valid_address(addr)) 
#endif
                  *addr++ = ch;
                
#ifdef CYGBLD_REDBOOT_LOAD_INTO_FLASH
                else {
                  flash_load_write(addr, ch);
                  addr++;
                }
#endif
                offset++;
                if ((unsigned long)(addr-addr_offset) > highest_address) {
                    highest_address = (unsigned long)(addr - addr_offset);
                }
            }
        }
    }

    // Save load base/top and entry
    if (base) {
        load_address = base;
        load_address_end = base + (highest_address - lowest_address);
        entry_address = base + (ehdr.e_entry - lowest_address);
    } else {
        load_address = lowest_address;
        load_address_end = highest_address;
        entry_address = ehdr.e_entry;
    }

    // nak everything to stop the transfer, since redboot
    // usually doesn't read all the way to the end of the
    // elf files.
    redboot_getc_terminate(true);
    if (addr_offset) diag_printf("Address offset = %p\n", (void *)addr_offset);
    diag_printf("Entry point: %p, address range: %p-%p\n", 
                (void*)entry_address, (void *)load_address, (void *)load_address_end);
    return 1;
#else // CYGSEM_REDBOOT_ELF
    err_printf("Loading ELF images not supported\n");
    return 0;
#endif // CYGSEM_REDBOOT_ELF
}


//
// Scan a string of hex bytes and update the checksum
//
static long
_hex2(int (*getc)(void), int len, long *sum)
{
    int val, byte;
    char c1, c2;

    val = 0;
    while (len-- > 0) {
        c1 = (*getc)();
        c2 = (*getc)();
        if (_is_hex(c1) && _is_hex(c2)) {
            val <<= 8;
            byte = (_from_hex(c1)<<4) | _from_hex(c2);
            val |= byte;
            if (sum) {
                *sum += byte;
            }
        } else {
            return (-1);
        }
    }
    return (val);
}

//
// Process a set of S-records, loading the contents into memory.  
// Note: if a "base" value is provided, the data will be relocated
// relative to that location.  Of course, this can only work for
// the first section of the data, so if there are non-contiguous
// pieces of data, they will end up relocated in the same fashion.
// Because of this, "base" probably only makes sense for a set of
// data which has only one section, e.g. a ROM image.
//
static unsigned long
load_srec_image(getc_t getc, unsigned long base)
{
    int  c;
    long offset = 0, count, sum, val, cksum;
    unsigned char *addr, *base_addr;
    char type;
    bool first_addr = true;
    unsigned long addr_offset = 0;
    unsigned long highest_address = 0;
    unsigned long lowest_address = 0xFFFFFFFF;

    while ((c = (*getc)()) > 0) {
        // Start of line
        if (c != 'S') {
            redboot_getc_terminate(true);
            err_printf("Invalid S-record at offset %p, input: %c\n", 
                   (void *)offset, c);
            return 0;
        }
        type = (*getc)();
        offset += 2;
        sum = 0;
        if ((count = _hex2(getc, 1, &sum)) < 0) {
            redboot_getc_terminate(true);
            err_printf("Bad S-record count at offset %p\n", (void *)offset);
            return 0;
        }
        offset += 1;
        switch (type) {
        case '0':
            break;
        case '1':
        case '2':
        case '3':
            base_addr = addr = (unsigned char *)_hex2(getc, (type-'1'+2), &sum);
            offset += (type-'1'+2);
            if (first_addr) {
                if (base) {
                    addr_offset = (unsigned long)base - (unsigned long)addr;
                } else {
                    addr_offset = 0;                    
                }
                first_addr = false;
            }
            addr += addr_offset;
            if ((unsigned long)(addr-addr_offset) < lowest_address) {
                lowest_address = (unsigned long)(addr - addr_offset);
            }
#ifdef CYGSEM_REDBOOT_VALIDATE_USER_RAM_LOADS
            if (!(valid_address(addr)
#ifdef CYGBLD_REDBOOT_LOAD_INTO_FLASH
                  || (flash_verify_addr(addr) == FLASH_ERR_OK)
#endif
                  )) {
	      // Only if there is no need to stop the download before printing
	      // output can we ask confirmation questions.
                redboot_getc_terminate(true);
		err_printf("*** Abort! Attempt to load S-record to address: %p, which is not valid\n",(void*)addr);
                return 0;
            }
#endif
            count -= ((type-'1'+2)+1);
            offset += count;
            while (count-- > 0) {
                val = _hex2(getc, 1, &sum);
#ifdef CYGSEM_REDBOOT_VALIDATE_USER_RAM_LOADS
                if (valid_address(addr)) 
#endif
                  *addr++ = val;
                
#ifdef CYGBLD_REDBOOT_LOAD_INTO_FLASH
                else {
                  flash_load_write(addr, val);
                  addr++;
                }
#endif
            }
            cksum = _hex2(getc, 1, 0);
            offset += 1;
            sum = sum & 0xFF;
            cksum = (~cksum & 0xFF);
            if (cksum != sum) {
                redboot_getc_terminate(true);
                err_printf("*** Warning! Checksum failure - Addr: %lx, %02lX <> %02lX\n", 
                       (unsigned long)base_addr, sum, cksum);
                return 0;
            }
            if ((unsigned long)(addr-addr_offset) > highest_address) {
                highest_address = (unsigned long)(addr - addr_offset);
            }
            break;
        case '7':
        case '8':
        case '9':
            addr = (unsigned char *)_hex2(getc, ('9'-type+2), &sum);
            offset += ('9'-type+2);
            // Save load base/top, entry address
            if (base) {
                load_address = base;
                load_address_end = base + (highest_address - lowest_address);
                entry_address = (unsigned long)(base + (addr - lowest_address));
            } else {
                load_address = lowest_address;
                load_address_end = highest_address;
                entry_address = (unsigned long)addr;
            }
            redboot_getc_terminate(false);
            if (addr_offset) diag_printf("Address offset = %p\n", (void *)addr_offset);
            diag_printf("Entry point: %p, address range: %p-%p\n", 
                   (void*)entry_address, (void *)load_address, (void *)load_address_end);

            return load_address_end;
        default:
            redboot_getc_terminate(true);
            err_printf("Invalid S-record at offset 0x%lx, type: %x\n", 
                   (unsigned long)offset, type);
            return 0;
        }
        while ((c = (*getc)()) != '\n') offset++;
    }
    return 0;
}

//
// 'load' CLI command processing
//   -b - specify a load [base] address
//   -m - specify an I/O stream/method
//   -c - Alternate serial I/O channel
#ifdef CYGBLD_BUILD_REDBOOT_WITH_ZLIB
//   -d - Decompress data [packed via 'zlib']
#endif
#ifdef CYGBLD_REDBOOT_LOAD_INTO_FLASH
//   -f - specify a flash address
#endif
//
void 
do_load(int argc, char *argv[])
{
    int res, num_options;
    int i, err;
    bool verbose, raw;
    bool base_addr_set, mode_str_set;
    char *mode_str;
#ifdef CYGPKG_REDBOOT_NETWORKING
    struct sockaddr_in host;
    bool hostname_set, port_set;
    unsigned int port;	// int because it's an OPTION_ARG_TYPE_NUM, 
                        // but will be cast to short
    char *hostname;
#endif
#ifdef CYGBLD_REDBOOT_LOAD_INTO_FLASH
    bool flash_addr_set = false;
#endif
    bool decompress = false;
    int chan = -1;
#if CYGNUM_HAL_VIRTUAL_VECTOR_NUM_CHANNELS > 1
    bool chan_set;
#endif
    unsigned long base = 0;
    unsigned long end = 0;
    char type[4];
    char *filename = 0;
    struct option_info opts[9];
    connection_info_t info;
    getc_io_funcs_t *io = NULL;
    struct load_io_entry *io_tab;
#ifdef CYGSEM_REDBOOT_VALIDATE_USER_RAM_LOADS
    bool spillover_ok = false;
#endif

#ifdef CYGPKG_REDBOOT_NETWORKING
    memset((char *)&host, 0, sizeof(host));
    host.sin_len = sizeof(host);
    host.sin_family = AF_INET;
    host.sin_addr = my_bootp_info.bp_siaddr;
    host.sin_port = 0;
#endif

    init_opts(&opts[0], 'v', false, OPTION_ARG_TYPE_FLG, 
              (void *)&verbose, 0, "verbose");
    init_opts(&opts[1], 'r', false, OPTION_ARG_TYPE_FLG, 
              (void *)&raw, 0, "load raw data");
    init_opts(&opts[2], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void *)&base, (bool *)&base_addr_set, "load address");
    init_opts(&opts[3], 'm', true, OPTION_ARG_TYPE_STR, 
              (void *)&mode_str, (bool *)&mode_str_set, "download mode (TFTP, xyzMODEM, or disk)");
    num_options = 4;
#if CYGNUM_HAL_VIRTUAL_VECTOR_NUM_CHANNELS > 1
    init_opts(&opts[num_options], 'c', true, OPTION_ARG_TYPE_NUM, 
              (void *)&chan, (bool *)&chan_set, "I/O channel");
    num_options++;
#endif
#ifdef CYGPKG_REDBOOT_NETWORKING
    init_opts(&opts[num_options], 'h', true, OPTION_ARG_TYPE_STR, 
              (void *)&hostname, (bool *)&hostname_set, "host name or IP address");
    num_options++;
    init_opts(&opts[num_options], 'p', true, OPTION_ARG_TYPE_NUM, 
              (void *)&port, (bool *)&port_set, "TCP port");
    num_options++;
#endif
#ifdef CYGBLD_BUILD_REDBOOT_WITH_ZLIB
    init_opts(&opts[num_options], 'd', false, OPTION_ARG_TYPE_FLG, 
              (void *)&decompress, 0, "decompress");
    num_options++;
#endif
#ifdef CYGBLD_REDBOOT_LOAD_INTO_FLASH
    init_opts(&opts[num_options], 'f', true, OPTION_ARG_TYPE_NUM,
              (void *)&base, (bool *)&flash_addr_set, "flash address");
    num_options++;
#endif
    CYG_ASSERT(num_options <= NUM_ELEMS(opts), "Too many options");
    
    if (!scan_opts(argc, argv, 1, opts, num_options, 
                   (void *)&filename, OPTION_ARG_TYPE_STR, "file name")) {
        return;
    }

    /* make sure any future go/exec's will fail until a successful upload */
    entry_address = (unsigned long)NO_MEMORY;

#ifdef CYGPKG_REDBOOT_NETWORKING
    if (hostname_set) {
        ip_route_t rt;
        if (!_gethostbyname(hostname, (in_addr_t *)&host)) {
            err_printf("Invalid host: %s\n", hostname);
            return;
        }
        /* check that the host can be accessed */
        if (__arp_lookup((ip_addr_t *)&host.sin_addr, &rt) < 0) {
            err_printf("Unable to reach host %s (%s)\n",
                        hostname, inet_ntoa((in_addr_t *)&host));
            return;
        }
    }
    if (port_set) 
	    host.sin_port = port;
#endif
    if (chan >= CYGNUM_HAL_VIRTUAL_VECTOR_NUM_CHANNELS) {
        err_printf("Invalid I/O channel: %d\n", chan);
        return;
    }
    if (mode_str_set) {
        for (io_tab = __RedBoot_LOAD_TAB__; 
             io_tab != &__RedBoot_LOAD_TAB_END__;  io_tab++) {
            if (strncasecmp(&mode_str[0], io_tab->name, strlen(&mode_str[0])) == 0) {
                io = io_tab->funcs;
                break;
            }
        }
        if (!io) {
            diag_printf("Invalid 'mode': %s.  Valid modes are:", mode_str);
            for (io_tab = __RedBoot_LOAD_TAB__; 
                 io_tab != &__RedBoot_LOAD_TAB_END__;  io_tab++) {
                diag_printf(" %s", io_tab->name);
            }
            err_printf("\n");
        }
        if (!io) {
            return;
        }
        verbose &= io_tab->can_verbose;
        if (io_tab->need_filename && !filename) {
            diag_printf("File name required\n");
            err_printf("usage: load %s\n", usage);
            return;
        }
    } else {
        char *which = "";
        io_tab = (struct load_io_entry *)NULL;  // Default
#ifdef CYGPKG_REDBOOT_NETWORKING
#ifdef CYGSEM_REDBOOT_NET_TFTP_DOWNLOAD        
        which = "TFTP";
        io = &tftp_io;
#elif defined(CYGSEM_REDBOOT_NET_HTTP_DOWNLOAD)
        which = "HTTP";
        io = &http_io;
#endif
#endif
#if 0 //def CYGPKG_REDBOOT_FILEIO
        // Make file I/O default if mounted
	if (fileio_mounted) {
	    which = "file";
	    io = &fileio_io;
	}
#endif
        if (!io) {
#ifdef CYGBLD_BUILD_REDBOOT_WITH_XYZMODEM
            which = "Xmodem";
            io = &xyzModem_io;
            verbose = false;
#else
            err_printf("No default protocol!\n");
            return;
#endif
        }
        diag_printf("Using default protocol (%s)\n", which);
    }
#ifdef CYGSEM_REDBOOT_VALIDATE_USER_RAM_LOADS
#ifdef  CYGBLD_REDBOOT_LOAD_INTO_FLASH
    if (flash_addr_set && flash_verify_addr((unsigned char *)base)) {
        if (!verify_action("Specified address (%p) is not believed to be in FLASH", (void*)base))
          return;
        spillover_ok = true;
    }
#endif
    if (base_addr_set && !valid_address((unsigned char *)base)) {
        if (!verify_action("Specified address (%p) is not believed to be in RAM", (void*)base))
            return;
        spillover_ok = true;
    }
#endif
    if (raw && !(base_addr_set 
#ifdef CYGBLD_REDBOOT_LOAD_INTO_FLASH
                || flash_addr_set
#endif
        )) {
        err_printf("Raw load requires a memory address\n");
        return;
    }
    info.filename = filename;
    info.chan = chan;
    info.mode = io_tab ? io_tab->mode : 0;
#ifdef CYGPKG_REDBOOT_NETWORKING
    info.server = &host;
#endif
    res = redboot_getc_init(&info, io, verbose, decompress);
    if (res < 0) {
        return;
    }
#ifdef CYGBLD_REDBOOT_LOAD_INTO_FLASH
    flash_load_start();
#endif
    // Stream open, process the data
    if (raw) {
        unsigned char *mp = (unsigned char *)base;
        err = 0;
        while ((res = redboot_getc()) >= 0) {
#ifdef CYGSEM_REDBOOT_VALIDATE_USER_RAM_LOADS
#ifdef CYGBLD_REDBOOT_LOAD_INTO_FLASH
            if (flash_addr_set && flash_verify_addr(mp) && !spillover_ok) {
                // Only if there is no need to stop the download
                // before printing output can we ask confirmation
                // questions.
                redboot_getc_terminate(true);
                err_printf("*** Abort! RAW data spills over limit of FLASH at %p\n",(void*)mp);
                err = -1;
                break;
            }
#endif
            if (base_addr_set && !valid_address(mp) && !spillover_ok) {
                // Only if there is no need to stop the download
                // before printing output can we ask confirmation
                // questions.
                redboot_getc_terminate(true);
                err_printf("*** Abort! RAW data spills over limit of user RAM at %p\n",(void*)mp);
                err = -1;
                break;
            }
#endif
#ifdef CYGBLD_REDBOOT_LOAD_INTO_FLASH
            if (flash_addr_set) {
              flash_load_write(mp, res);
              mp++;
              res++;
            } else
#endif
            *mp++ = res;
        }
        end = (unsigned long) mp;

        // Save load base/top
        load_address = base;
        load_address_end = end;
        entry_address = base;           // best guess

        redboot_getc_terminate(false);
        if (0 == err)
            diag_printf("Raw file loaded %p-%p, assumed entry at %p\n", 
                        (void *)base, (void *)(end - 1), (void*)base);
    } else {
        // Read initial header - to determine file [image] type
        for (i = 0;  i < sizeof(type);  i++) {
            if ((res = redboot_getc()) < 0) {
                err = getc_info.err;
                break;
            } 
            type[i] = res;
        }
        if (res >= 0) {
            redboot_getc_rewind();  // Restore header to stream
            // Treat data as some sort of executable image
            if (strncmp(&type[1], "ELF", 3) == 0) {
                end = load_elf_image(redboot_getc, base);
            } else if ((type[0] == 'S') &&
                       ((type[1] >= '0') && (type[1] <= '9'))) {
		end = load_srec_image(redboot_getc, base);
            } else {
                redboot_getc_terminate(true);
                err_printf("Unrecognized image type: 0x%lx\n", *(unsigned long *)type);
            }
        }
    }
#ifdef CYGBLD_REDBOOT_LOAD_INTO_FLASH
    flash_load_finish();
#endif

    redboot_getc_close();  // Clean up
    return;
}
