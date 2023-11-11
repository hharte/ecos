#ifndef CYGONCE_HAL_CACHE_H
#define CYGONCE_HAL_CACHE_H

//=============================================================================
//
//      hal_cache.h
//
//      HAL cache control API
//
//=============================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   msalter
// Contributors:dmoseley
// Date:        2001-12-03
// Purpose:     Cache control API
// Description: The macros defined here provide the HAL APIs for handling
//              cache control operations.
// Usage:
//              #include <cyg/hal/hal_cache.h>
//              ...
//              
// Copyright:    (C) 2003-2004 Intel Corporation.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#ifndef __ASSEMBLER__
#include <pkgconf/system.h>             // System-wide configuration info
#include <cyg/hal/hal_io.h>             // DCACHE_FLUSH_AREA
#include <cyg/infra/cyg_type.h>
#include CYGBLD_HAL_VAR_H
#include <cyg/hal/hal_mmu.h>
#endif

//-----------------------------------------------------------------------------
// Cache dimensions

// All xscale cores have these attributes
#define HAL_DCACHE_SIZE                 0x8000 // Size of data cache in bytes
#define HAL_DCACHE_LINE_SIZE            32     // Size of a data cache line
#define HAL_ICACHE_SIZE                 0x8000 // Size of icache in bytes
#define HAL_ICACHE_LINE_SIZE            32     // Size of ins cache line

// Core generation 1 and 2 have 32 ways and no L2 Cache
#if CYGNUM_HAL_ARM_XSCALE_CORE_GENERATION < 3
#define HAL_DCACHE_WAYS                 32     // Associativity of the cache
#define HAL_DCACHE_SETS (HAL_DCACHE_SIZE/(HAL_DCACHE_LINE_SIZE*HAL_DCACHE_WAYS))
#define HAL_DCACHE_LOCKABLE_SIZE        28*1024 // 28K lockable

#define HAL_ICACHE_SIZE                 0x8000 // Size of icache in bytes
#define HAL_ICACHE_LINE_SIZE            32     // Size of ins cache line
#define HAL_ICACHE_WAYS                 32     // Associativity of the cache
#define HAL_ICACHE_SETS (HAL_ICACHE_SIZE/(HAL_ICACHE_LINE_SIZE*HAL_ICACHE_WAYS))

#else  // Else we're using a newer gen XScale core with 4-way associative

#define HAL_DCACHE_WAYS                 4     // Associativity of the cache
#define HAL_DCACHE_SETS (HAL_DCACHE_SIZE/(HAL_DCACHE_LINE_SIZE*HAL_DCACHE_WAYS))
#define HAL_DCACHE_LOCKABLE_SIZE        24*1024 // 24K lockable

#define HAL_ICACHE_WAYS                 4     // Associativity of the cache
#define HAL_ICACHE_SETS (HAL_ICACHE_SIZE/(HAL_ICACHE_LINE_SIZE*HAL_ICACHE_WAYS))

// New Gen has L2 Cache
#define HAL_L2_CACHE_SIZE               L2_CACHE_SIZE
#define HAL_L2_CACHE_WAYS				8
#define HAL_L2_CACHE_LINE_SIZE			32
#define HAL_L2_CACHE_SETS (HAL_L2_CACHE_SIZE/(HAL_L2_CACHE_LINE_SIZE*HAL_L2_CACHE_WAYS))

#endif // CoreGen
#ifndef __ASSEMBLER__
//-----------------------------------------------------------------------------
// Global control of Instruction cache

// Enable the instruction cache
#define HAL_ICACHE_ENABLE()                                             \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc  p15,0,r1,c1,c0,0;"                                        \
        "orr  r1,r1,#0x1000;" /* enable ICache */                       \
        "mcr  p15,0,r1,c1,c0,0;"                                        \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END

// Disable the instruction cache (and invalidate it, required semanitcs)
#define HAL_ICACHE_DISABLE()                                            \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc    p15,0,r1,c1,c0,0;"                                      \
        "bic    r1,r1,#0x1000;" /* disable Icache */                    \
        "mcr    p15,0,r1,c1,c0,0;"                                      \
        "mcr    p15,0,r1,c7,c5,0;"  /* invalidate instruction cache */  \
        "nop;" /* next few instructions may be via cache */             \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop"                                                           \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END

// Query the state of the instruction cache
#define HAL_ICACHE_IS_ENABLED(_state_)                                   \
CYG_MACRO_START                                                          \
    register cyg_uint32 reg;                                             \
    asm volatile ("mrc  p15,0,%0,c1,c0,0"                                \
                  : "=r"(reg)                                            \
                  :                                                      \
        );                                                               \
    (_state_) = (0 != (0x1000 & reg)); /* Bit 12 is ICache enable */     \
CYG_MACRO_END

// Invalidate the entire cache
#define HAL_ICACHE_INVALIDATE_ALL()                                     \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mcr    p15,0,r1,c7,c5,0;"  /* clear instruction cache */       \
        "mcr    p15,0,r1,c8,c5,0;"  /* flush I TLB only */              \
        /* cpuwait */                                                   \
        "mrc    p15,0,r1,c2,c0,0;"  /* arbitrary read   */              \
        "mov    r1,r1;"                                                 \
        "sub    pc,pc,#4;"                                              \
        "nop;" /* next few instructions may be via cache */             \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop"                                                           \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END

// Synchronize the contents of the cache with memory.
// (which includes flushing out pending writes)
#if CYGNUM_HAL_ARM_XSCALE_CORE_GENERATION < 3
#define HAL_ICACHE_SYNC()                                       \
CYG_MACRO_START                                                 \
    HAL_DCACHE_SYNC(); /* ensure data gets to RAM */            \
    HAL_ICACHE_INVALIDATE_ALL(); /* forget all we know */       \
CYG_MACRO_END
#else
#define HAL_ICACHE_SYNC()                                       \
CYG_MACRO_START                                                 \
    HAL_DCACHE_SYNC(); /* ensure data gets to RAM */            \
	HAL_L2_CACHE_SYNC();  /* flush L2 as well */                \
    HAL_ICACHE_INVALIDATE_ALL(); /* forget all we know */       \
CYG_MACRO_END
#endif

// Set the instruction cache refill burst size
//#define HAL_ICACHE_BURST_SIZE(_size_)
// This feature is not available on the XScale.

// Load the contents of the given address range into the instruction cache
// and then lock the cache so that it stays there.
//#define HAL_ICACHE_LOCK(_base_, _size_)
// This feature is not available on the XScale.

// Undo a previous lock operation
//#define HAL_ICACHE_UNLOCK(_base_, _size_)
// This feature is not available on the XScale.

// Unlock entire cache
//#define HAL_ICACHE_UNLOCK_ALL()
// This feature is not available on the XScale.

//-----------------------------------------------------------------------------
// Instruction cache line control

// Invalidate cache lines in the given range without writing to memory.
//#define HAL_ICACHE_INVALIDATE( _base_ , _size_ )
// This feature is not available on the XScale.

//-----------------------------------------------------------------------------
// Global control of data cache

// Enable the data cache - XScale Core Gen < 3 has coalesce bit */
#if CYGNUM_HAL_ARM_XSCALE_CORE_GENERATION < 3
#define HAL_DCACHE_ENABLE()                                             \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mcr  p15,0,r1,c7,c10,4;"   /* drain write buffer */            \
        "mrc  p15,0,r1,c1,c0,0;"                                        \
        "orr  r1,r1,#0x0007;"  /* enable DCache (also ensures the */    \
                               /* MMU and alignment faults are    */    \
                               /* enabled)                        */    \
        "mcr  p15,0,r1,c1,c0,0;"                                        \
        "mrc  p15, 0, r1, c1, c0, 1;"   /* Clear K bit (Disable Coalesce */  \
	"bic  r1,r1,#1;"                                                \
	"mcr  p15,0,r1,c1,c0,1;"                                        \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END
#else /* Else - we're on 3rd gen or later core w/o a coalesce bit */
#define HAL_DCACHE_ENABLE()                                             \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mcr  p15, 0, r1, c7, c10, 4;"   /* drain write buffer */            \
        "mrc  p15, 0, r1, c1, c0, 0;"                                        \
        "orr  r1,r1,#0x0007;"  /* enable DCache (also ensures the */    \
                               /* MMU and alignment faults are    */    \
                               /* enabled)                        */    \
        "mcr  p15, 0, r1, c1, c0, 0;"                                        \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END
#endif

// Disable the data cache (and invalidate it, required semanitcs)
/* XScale Core Generation < 3 has coalesce bit */
#if CYGNUM_HAL_ARM_XSCALE_CORE_GENERATION < 3
#define HAL_DCACHE_DISABLE()                                            \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc  p15,0,r1,c1,c0,0;"    /* disable cache */                 \
        "bic  r1,r1,#4;"                                                \
        "mcr  p15,0,r1,c1,c0,0;"                                        \
	"mrc  p15,0,r1,c1,c0,1;"    /* disable coalescing */            \
	"orr  r1,r1,#1;"                                                \
	"mcr  p15,0,r1,c1,c0,1;"                                        \
        "mcr    p15,0,r1,c7,c6,0;"  /* invalidate data cache */         \
        /* cpuwait */                                                   \
        "mrc    p15,0,r1,c2,c0,0;"  /* arbitrary read   */              \
        "mov    r1,r1;"                                                 \
        "sub    pc,pc,#4;"                                              \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END
#else	   /* Else - we're on 3rd gen or later core w/o a coalesce bit */
#define HAL_DCACHE_DISABLE()                                            \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc  p15, 0, r1, c1, c0, 0;"    /* disable cache */                 \
        "bic  r1,r1,#4;"                                                \
        "mcr  p15, 0, r1, c1, c0, 0;"                                        \
        "mcr  p15, 0, r1, c7, c6, 0;"  /* invalidate data cache */         \
        /* cpuwait */                                                   \
        "mrc    p15, 0, r1, c2, c0, 0;"  /* arbitrary read   */              \
        "mov    r1,r1;"                                                 \
        "sub    pc,pc,#4;"                                              \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END
#endif

// Query the state of the data cache
#define HAL_DCACHE_IS_ENABLED(_state_)                                   \
CYG_MACRO_START                                                          \
    register int reg;                                                   \
    asm volatile ("mrc  p15,0,%0,c1,c0,0"                               \
                  : "=r"(reg)                                           \
                  :                                                     \
                /*:*/                                                   \
        );                                                              \
    (_state_) = (0 != (4 & reg)); /* Bit 2 is DCache enable */          \
CYG_MACRO_END

// Flush the entire dcache (and then both TLBs, just in case)
#define HAL_DCACHE_INVALIDATE_ALL()                                     \
CYG_MACRO_START    /* this macro can discard dirty cache lines. */      \
    /* this macro can discard dirty cache lines. */                     \
    asm volatile (                                                      \
        "mcr    p15,0,r1,c7,c6,0;"  /* invalidate data cache */         \
        "mcr    p15,0,r1,c8,c7,0;"  /* flush I+D TLBs */                \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END

// Core Generations < 3 have a mini datacache.  Newer cores do not.
// Core Generations < 3 can lock 28k of ICache.  Newer cores only lock 24k
#if CYGNUM_HAL_ARM_XSCALE_CORE_GENERATION < 3
#define LINE_ALLOCATE_TOTAL 0x8800
#define LOCK_TOTAL          0x7000
#else
#define LINE_ALLOCATE_TOTAL 0x8000
#define LOCK_TOTAL          0x6000
#endif
// DCACHE_FLUSH_AREA is defined if writeback caching is used. Otherwise
// write-through is assumed.
#ifdef DCACHE_FLUSH_AREA

#ifdef CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM
// Evict dirty lines from write-back caches
#define HAL_DCACHE_EVICT()                                              \
CYG_MACRO_START                                                         \
    /* The best way to evict a dirty line is by using the          */   \
    /* line allocate operation on non-existent memory.             */   \
    asm volatile (                                                      \
        "mov    r0, %0;"            /* cache flush region */            \
        "add    r1, r0, %1;"        /* Allocate total - varies by core*/\
        "add    r0, r0, %2;"        /* Locked lines - varies by core */ \
 "667: "                                                                \
        "mcr    p15,0,r0,c7,c2,5;"  /* allocate a line    */            \
        "add    r0, r0, #32;"       /* 32 bytes/line      */            \
        "teq    r1, r0;"                                                \
        "bne    667b;"                                                  \
        :                                                               \
        : "i"(DCACHE_FLUSH_AREA), "i"(LINE_ALLOCATE_TOTAL), "i"(LOCK_TOTAL) \
        : "r0","r1"      /* Clobber list */                             \
        );                                                              \
CYG_MACRO_END
#else // CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM
// Evict dirty lines from write-back caches
#define HAL_DCACHE_EVICT()                                              \
CYG_MACRO_START                                                         \
    /* The best way to evict a dirty line is by using the          */   \
    /* line allocate operation on non-existent memory.             */   \
    asm volatile (                                                      \
        "mov    r0, %0;"            /* cache flush region */            \
        "add    r1, r0, %1;"        /* Allocate total - varies by core*/\
        "667: "                                                         \
        "mcr    p15,0,r0,c7,c2,5;"  /* allocate a line    */            \
        "add    r0, r0, #32;"       /* 32 bytes/line      */            \
        "teq    r1, r0;"                                                \
        "bne    667b;"                                                  \
        :                                                               \
        : "i" (DCACHE_FLUSH_AREA), "i" (LINE_ALLOCATE_TOTAL)            \
        : "r0","r1"      /* Clobber list */                             \
        );                                                              \
CYG_MACRO_END
#endif // CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM
#else // DCACHE_FLUSH_AREA
#define HAL_DCACHE_EVICT()
#endif // DCACHE_FLUSH_AREA

// Synchronize the contents of the cache with memory.
#define HAL_DCACHE_SYNC()                                               \
CYG_MACRO_START                                                         \
    HAL_DCACHE_EVICT();                                                 \
    asm volatile (                                                      \
        "mcr    p15,0,r0,c7,c6,0;"  /* invalidate data cache */         \
        /* cpuwait */                                                   \
        "mrc    p15,0,r1,c2,c0,0;"  /* arbitrary read   */              \
        "mov    r1,r1;"                                                 \
        "sub    pc,pc,#4;"                                              \
        "mcr    p15,0,r0,c7,c10,4;" /* and drain the write buffer */    \
        /* cpuwait */                                                   \
        "mrc    p15,0,r1,c2,c0,0;"  /* arbitrary read   */              \
        "mov    r1,r1;"                                                 \
        "sub    pc,pc,#4;"                                              \
        "nop"                                                           \
        :                                                               \
        :                                                               \
        : "r0","r1"      /* Clobber list */                             \
        );                                                              \
CYG_MACRO_END

// Set the data cache refill burst size
//#define HAL_DCACHE_BURST_SIZE(_size_)
// This feature is not available on the XScale.

// Set the data cache write mode
//#define HAL_DCACHE_WRITE_MODE( _mode_ )
// This feature is not available on the XScale.

#define HAL_DCACHE_WRITETHRU_MODE       0
#define HAL_DCACHE_WRITEBACK_MODE       1

// Get the current writeback mode - or only writeback mode if fixed
#ifdef DCACHE_FLUSH_AREA
#define HAL_DCACHE_QUERY_WRITE_MODE( _mode_ ) CYG_MACRO_START           \
    _mode_ = HAL_DCACHE_WRITEBACK_MODE;                                 \
CYG_MACRO_END
#else
#define HAL_DCACHE_QUERY_WRITE_MODE( _mode_ ) CYG_MACRO_START           \
    _mode_ = HAL_DCACHE_WRITETHRU_MODE;                                 \
CYG_MACRO_END
#endif

// Load the contents of the given address range into the data cache
// and then lock the cache so that it stays there.
//#define HAL_DCACHE_LOCK(_base_, _size_)
// This feature is not available on the XScale.

// Undo a previous lock operation
//#define HAL_DCACHE_UNLOCK(_base_, _size_)
// This feature is not available on the XScale.

// Unlock entire cache
//#define HAL_DCACHE_UNLOCK_ALL()
// This feature is not available on the XScale.

//-----------------------------------------------------------------------------
// Data cache line control

// Allocate cache lines for the given address range without reading its
// contents from memory.
//#define HAL_DCACHE_ALLOCATE( _base_ , _size_ )
// This feature is not available on the XScale.

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
#define HAL_DCACHE_FLUSH( _base_ , _size_ )     \
CYG_MACRO_START                                 \
    HAL_DCACHE_STORE( _base_ , _size_ );        \
    HAL_DCACHE_INVALIDATE( _base_ , _size_ );   \
CYG_MACRO_END

// Invalidate cache lines in the given range without writing to memory.
#define HAL_DCACHE_INVALIDATE( _base_ , _size_ )                        \
CYG_MACRO_START                                                         \
    register int addr, enda;                                            \
    for ( addr = (~(HAL_DCACHE_LINE_SIZE - 1)) & (int)(_base_),         \
              enda = (int)(_base_) + (_size_);                          \
          addr < enda ;                                                 \
          addr += HAL_DCACHE_LINE_SIZE )                                \
    {                                                                   \
        asm volatile (                                                  \
                      "mcr  p15,0,%0,c7,c6,1;" /* flush entry away */   \
                      :                                                 \
                      : "r"(addr)                                       \
                      : "memory"                                        \
            );                                                          \
    }                                                                   \
CYG_MACRO_END
                          
// Write dirty cache lines to memory for the given address range.
#define HAL_DCACHE_STORE( _base_ , _size_ )                             \
CYG_MACRO_START                                                         \
    register int addr, enda;                                            \
    for ( addr = (~(HAL_DCACHE_LINE_SIZE - 1)) & (int)(_base_),         \
              enda = (int)(_base_) + (_size_);                          \
          addr < enda ;                                                 \
          addr += HAL_DCACHE_LINE_SIZE )                                \
    {                                                                   \
        asm volatile ("mcr  p15,0,%0,c7,c10,1;" /* push entry to RAM */ \
                      :                                                 \
                      : "r"(addr)                                       \
                      : "memory"                                        \
            );                                                          \
    }                                                                   \
    /* and also drain the write buffer */                               \
    asm volatile (                                                      \
        "mov    r1,#0;"                                                 \
	"mcr    p15,0,r1,c7,c10,4;"                                     \
        :                                                               \
        :                                                               \
        : "r1", "memory" /* Clobber list */                             \
    );                                                                  \
CYG_MACRO_END

// Flush the entire dcache (and then both TLBs, just in case)
#define HAL_FLUSH_I_D_TLB()                                             \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mcr    p15, 0, r1, c8, c7, 0;"  /* flush I+D TLBs */                \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END
// Preread the given range into the cache with the intention of reading
// from it later.
//#define HAL_DCACHE_READ_HINT( _base_ , _size_ )
// This feature is available on the XScale, but due to tricky
// coherency issues with the read buffer (see XScale developer's
// manual) we don't bother to implement it here.

// Preread the given range into the cache with the intention of writing
// to it later.
//#define HAL_DCACHE_WRITE_HINT( _base_ , _size_ )
// This feature is not available on the XScale.

// Allocate and zero the cache lines associated with the given range.
//#define HAL_DCACHE_ZERO( _base_ , _size_ )
// This feature is not available on the XScale.

#ifdef CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM

// If we are using Cache locked as SRAM then we do not copy the 
// page tables to RAM and they are thus read-only.
#define HAL_CACHE_SET_POLICY(b,e,p)

#else // CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM

enum XSCALE_CACHE_POLICY {
    STALL,
    BUFFERABLE,
    CACHE_WT_RD_ALLOC,
    CACHE_WB_RD_ALLOC,
    COALESCING_DISABLED,
    MINI_DCACHE,
    CACHE_WB_RW_ALLOC
};
typedef enum XSCALE_CACHE_POLICY CYG_HAL_XSCALE_CACHE_POLICY;

#define MMU_TBIT (1 << 14)
#define MMU_XBIT (1 << 12)
#define MMU_CBIT (1 << 3)
#define MMU_BBIT (1 << 2)
#define MMU_XCB_MASK (MMU_TBIT | MMU_XBIT | MMU_CBIT | MMU_BBIT)

#define SECTION_SIZE SZ_1M
#define HAL_CACHE_SET_POLICY(b,e,p)                                         \
CYG_MACRO_START                                                             \
    CYG_ADDRWORD cur_addr = (b);                                            \
    CYG_ADDRWORD end_addr = (e);                                            \
    CYG_HAL_XSCALE_CACHE_POLICY pol = (p);                                  \
    unsigned long ttb;                                                      \
    unsigned mask;                                                          \
    unsigned long *desc_section_entry;                                      \
                                                                            \
    switch (pol) {                                                          \
    case STALL:               mask = 0; break;                              \
    case BUFFERABLE:          mask = MMU_BBIT; break;                       \
    case CACHE_WT_RD_ALLOC:   mask = MMU_CBIT; break;                       \
    case CACHE_WB_RD_ALLOC:   mask = MMU_CBIT | MMU_BBIT; break;            \
    case COALESCING_DISABLED: mask = MMU_XBIT | MMU_BBIT; break;            \
    case MINI_DCACHE:         mask = MMU_XBIT | MMU_CBIT; break;            \
    case CACHE_WB_RW_ALLOC:   mask = MMU_XBIT | MMU_CBIT | MMU_BBIT; break; \
    default:                  mask = 0; break;                              \
    }                                                                       \
                                                                            \
    asm volatile("mrc\tp15, 0, %0, c2, c0, 0" : "=r" (ttb));                \
    ttb = CYGARC_VIRTUAL_ADDRESS(ttb);                                      \
                                                                            \
    while (cur_addr < end_addr)                                             \
    {                                                                       \
        desc_section_entry = (unsigned long *)((ttb &~ 0x00003FFF) |        \
                                      (cur_addr &~ 0x000FFFFF) >> 18);      \
        if ((*desc_section_entry & 0x3) != 2) {                             \
            break;                                                          \
        }                                                                   \
        *desc_section_entry &=~ MMU_XCB_MASK;                               \
        *desc_section_entry |= mask;                                        \
        cur_addr+=SECTION_SIZE;                                             \
    }                                                                       \
                                                                            \
    HAL_DCACHE_SYNC();                                                      \
    HAL_DCACHE_INVALIDATE_ALL();                                            \
    HAL_ICACHE_INVALIDATE_ALL();                                            \
	HAL_L2_CACHE_SYNC();                                                    \
CYG_MACRO_END
#endif // CYGSEM_HAL_ARM_XSCALE_USE_CACHE_AS_SRAM

// Enable the L2 cache - The L2 CACHE must be enabled in CP15 when 
//   the MMU is enabled.  This macro does not set the L2 Bit in
//   CP15, but calls a function to change PT Entries to be Outer 
//   (L2) Cacheable
#if CYGNUM_HAL_ARM_XSCALE_CORE_GENERATION > 2
#define HAL_L2_CACHE_ENABLE()  \
	if (L2_CACHE_ENABLED)      \
	   l2_cache_control(true); 

// Query the state of the L2 cache
#define HAL_L2_CACHE_IS_ENABLED(_state_)                                \
CYG_MACRO_START                                                         \
    register int reg;                                                   \
    asm volatile ("mrc  p15, 0, %0, c1, c0, 0"                               \
                  : "=r"(reg)                                           \
                  :                                                     \
                /*:*/                                                   \
        );                                                              \
    (_state_) = (0 != (0x04000000 & reg)); /* Bit 26 is L2 enable */    \
CYG_MACRO_END

// Query the state of the L2 cache - return 1 or 0
#define L2_CACHE_ENABLED                                                \
({  register int reg;                                                 \
    asm volatile ("mrc  p15, 0, %0, c1, c0, 0"                               \
                  : "=r"(reg)                                           \
                  :                                                     \
        );                                                              \
    reg &= 0x04000000;                                                  \
	reg; \
})

// Clean and Invalidate every CacheLine in L2. L2 has a single instruction
//   to clean and invalidate a line by Set/Way
#define HAL_L2_CACHE_SYNC()                                             \
CYG_MACRO_START                                                         \
	DATA_MEMORY_BARRIER();                                              \
	if (L2_CACHE_ENABLED){                                            \
    register int set, way, set_way;                                     \
	for ( way = 0; way < HAL_L2_CACHE_WAYS; way++) {                    \
    	for ( set = 0; set < HAL_L2_CACHE_SETS; set++) {                \
			set_way = ((way << 29) | (set << 5));                       \
     	   asm volatile (                                               \
                      "mcr  p15, 1, %0, c7, c15, 2;" /* Clean and Inval. */  \
                      :                                                 \
                      : "r"(set_way)                                    \
                      : "memory"                                        \
		   );                                                           \
		}                                                               \
	}                                                                   \
	}                                                                   \
CYG_MACRO_END

// L2 Disable is achieved by changing PT Entries to disable L2-cacheability
#define HAL_L2_CACHE_DISABLE() l2_cache_control(false);
	
// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
#define HAL_L2_CACHE_FLUSH( _base_ , _size_ )     \
CYG_MACRO_START                                   \
	DATA_MEMORY_BARRIER();                    \
	if (L2_CACHE_ENABLED){                                            \
    HAL_L2_CACHE_STORE( _base_ , _size_ );        \
    HAL_L2_CACHE_INVALIDATE( _base_ , _size_ );   \
	}                                             \
CYG_MACRO_END

// Invalidate cache lines in the given range without writing to memory.
#define HAL_L2_CACHE_INVALIDATE( _base_ , _size_ )                      \
CYG_MACRO_START                                                         \
	DATA_MEMORY_BARRIER();                                          \
	if (L2_CACHE_ENABLED){                                            \
    register int addr, enda;                                            \
    for ( addr = (~(HAL_L2_CACHE_LINE_SIZE - 1)) & (int)(_base_),       \
              enda = (int)(_base_) + (_size_);                          \
          addr < enda ;                                                 \
          addr += HAL_L2_CACHE_LINE_SIZE )                              \
    {                                                                   \
        asm volatile (                                                  \
                      "mcr  p15, 1, %0, c7, c7, 1;"                          \
                      :                                                 \
                      : "r"(addr)                                       \
                      : "memory"                                        \
            );                                                          \
    }                                                                   \
	}                                                                   \
CYG_MACRO_END

// Write dirty cache lines to memory for the given address range.
#define HAL_L2_CACHE_STORE( _base_ , _size_ )                           \
CYG_MACRO_START                                                         \
	DATA_MEMORY_BARRIER();                                          \
	if (L2_CACHE_ENABLED){                                            \
    register int addr, enda;                                            \
    for ( addr = (~(HAL_L2_CACHE_LINE_SIZE - 1)) & (int)(_base_),       \
              enda = (int)(_base_) + (_size_);                          \
          addr < enda ;                                                 \
          addr += HAL_L2_CACHE_LINE_SIZE )                              \
    {                                                                   \
        asm volatile ("mcr  p15, 1, %0, c7, c11, 1;"                         \
                      :                                                 \
                      : "r"(addr)                                       \
                      : "memory"                                        \
            );                                                          \
    }                                                                   \
	}                                                                   \
CYG_MACRO_END

#ifdef CREATE_L2_RAM
/* HAL_L2_RAM_CREATE: Creates a 256k L2 RAM block.  Not used or tested
     yet.  Typically, the DCACHE_FLUSH_AREA is used to flush out the 
	 DCache and it has a single 1M PT entry.  Only 32k of the 1M is used
	 for DCache flush and it is not physical memory, so the L2 RAM AREA 
	 can be safely placed above the Flush area.
*/
#define HAL_L2_RAM_CREATE()                                             \
CYG_MACRO_START                                                         \
	DATA_MEMORY_BARRIER();                                              \
	if (L2_CACHE_ENABLED){                                              \
        asm volatile (                                                  \
                      "mov r0, %0;"	                                    \
		              "add r0, r0, #0x8000;" /* 32k above DCache flush*/\
	                  "mov r1, r0;"                                     \
					  "add r0, r0, #0x40000;"/*256k to lock */          \
                      "1: "	                                            \
                      "mcr  p15, 1, r1, c9, c5, 2;"                     \
					  "add r1, r1, #32;"							    \
					  "cmp r0, r1;"                                     \
					  "bne 1b;"									        \
                      :                                                 \
                      : "i"(DCACHE_FLUSH_AREA)                          \
                      : "r0", "r1"                                      \
            );                                                          \
	}                                                                   \
CYG_MACRO_END

/* L2_RAM_DESTROY - Invalidate lines based on MVA.
      The invalidate automatically unlocks the line 
*/
#define HAL_L2_RAM_DESTROY()                                            \
CYG_MACRO_START                                                         \
	DATA_MEMORY_BARRIER();                                              \
	if (L2_CACHE_ENABLED){                                              \
        asm volatile (                                                  \
                      "mov r0, %0;"                                     \
		              "add r0, r0, #0x8000;"/*32k above DCache Flush*/  \
	                  "mov r1, r0;"                                     \
					  "add r0, r0, #0x70000;"/*256k to unlock/inval*/   \
                      "1: "												\
                      "mcr  p15, 1, r1, c7, c7, 1;"                     \
					  "add r1, r1, #32;"							    \
					  "cmp r0, r1  ;"									\
					  "bne 1b;"											\
                      :                                                 \
                      : "r"(DCACHE_FLUSH_AREA)                          \
                      : "r0", "r1"                                      \
            );                                                          \
	}                                                                   \
CYG_MACRO_END
#endif  // CREATE_L2_RAM
#else
#define	HAL_L2_CACHE_ENABLE()
#define HAL_L2_CACHE_IS_ENABLED(x)
#define L2_CACHE_ENABLED
#define HAL_L2_CACHE_SYNC()
#define HAL_L2_CACHE_DISABLE()
#define HAL_L2_CACHE_FLUSH(x, y)
#define HAL_L2_CACHE_INVALIDATE(x, y)
#define HAL_L2_CACHE_STORE(x, y)
#endif // Core Generation > 2
#endif // __ASSEMBLER__

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_CACHE_H
// End of hal_cache.h
