//==========================================================================
//
//      hal_misc.c
//
//      HAL miscellaneous functions
//
//==========================================================================
//####COPYRIGHTBEGIN####
//
// -------------------------------------------
// The contents of this file are subject to the Cygnus eCos Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://sourceware.cygnus.com/ecos
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the
// License for the specific language governing rights and limitations under
// the License.
// 
// The Original Code is eCos - Embedded Cygnus Operating System, released
// September 30, 1998.
// 
// The Initial Developer of the Original Code is Cygnus.  Portions created
// by Cygnus are Copyright (C) 1998, 1999 Cygnus Solutions.  
// All Rights Reserved.
// -------------------------------------------
//
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg, jskov
// Contributors: nickg, jskov,
//               jlarmour
// Date:         1999-02-20
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <pkgconf/hal.h>

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/ppc_regs.h>           // SPR definitions

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/infra/diag.h>             // diag_printf

#include <cyg/hal/hal_arch.h>           // HAL header
#include <cyg/hal/hal_cache.h>          // HAL cache
#if defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT) && \
    defined(CYGPKG_HAL_EXCEPTIONS)
# include <cyg/hal/hal_intr.h>           // HAL interrupts/exceptions
#endif

//---------------------------------------------------------------------------
// Functions used during initialization.

#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
cyg_bool cyg_hal_stop_constructors;
#endif

typedef void (*pfunc) (void);
extern pfunc __CTOR_LIST__[];
extern pfunc __CTOR_END__[];

void
cyg_hal_invoke_constructors (void)
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    static pfunc *p = &__CTOR_END__[-1];
    
    cyg_hal_stop_constructors = 0;
    for (; p >= __CTOR_LIST__; p--) {
        (*p) ();
        if (cyg_hal_stop_constructors) {
            p--;
            break;
        }
    }
#else
    pfunc *p;

    for (p = &__CTOR_END__[-1]; p >= __CTOR_LIST__; p--)
        (*p) ();
#endif
}

// Override any __eabi the compiler might generate. We don't want
// constructors to be called twice.
void __eabi (void) {}

//---------------------------------------------------------------------------
// First level C exception handler.

externC void __handle_exception (void);

externC HAL_SavedRegisters *_hal_registers;

void
cyg_hal_exception_handler(HAL_SavedRegisters *regs)
{
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

    // Set the pointer to the registers of the current exception
    // context. At entry the GDB stub will expand the
    // HAL_SavedRegisters structure into a (bigger) register array.
    _hal_registers = regs;

    __handle_exception();

#elif defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT) && \
      defined(CYGPKG_HAL_EXCEPTIONS)
    int vector = regs->vector>>8;

    // We should decode the vector and pass a more appropriate
    // value as the second argument. For now we simply pass a
    // pointer to the saved registers. We should also divert
    // breakpoint and other debug vectors into the debug stubs.

    if (vector==CYGNUM_HAL_VECTOR_PROGRAM) {
        int srr1;
        CYGARC_MFSPR(CYGARC_REG_SRR1, srr1); // get srr1

        switch ((srr1 >> 17) & 0xf) {
        case 1:
            vector = CYGNUM_HAL_EXCEPTION_TRAP;
            break;
        case 2:
            vector = CYGNUM_HAL_EXCEPTION_PRIVILEGED_INSTRUCTION;
            break;
        case 4:
            vector = CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION;
            break;
        case 8:
            vector = CYGNUM_HAL_EXCEPTION_FPU;
            break;
        default:
            CYG_FAIL("Unknown PROGRAM exception!!");
        }
    }
    cyg_hal_deliver_exception( vector, (CYG_ADDRWORD)regs );

#else

    CYG_FAIL("Exception!!!");
    
#endif    
    
    return;
}

//---------------------------------------------------------------------------
// Default ISRs

externC cyg_uint32
hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    diag_printf("Interrupt: %d\n", vector);

    CYG_FAIL("Spurious Interrupt!!!");
    return 0;
}

// The decrementer default ISR has to do nothing. The reason is that
// decrementer interrupts cannot be disabled - if a kernel configuration
// does not use the RTC, but does use external interrupts, the decrementer
// underflow could cause a CYG_FAIL (as above) even though the user did
// not expect any decrementer interrupts to happen.
externC cyg_uint32
hal_default_decrementer_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    return 0;
}

//---------------------------------------------------------------------------
// Idle thread action

void
hal_idle_thread_action( cyg_uint32 count )
{
}

//---------------------------------------------------------------------------
// Use MMU resources to map memory regions.  
// Takes and returns an int used to ID the MMU resource to use. This ID
// is increased as resources are used and should be used for subsequent
// invocations.
static int
hal_map_memory (int id,CYG_ADDRESS virt, CYG_ADDRESS phys, 
                cyg_int32 size, cyg_uint8 flags)
{
#ifdef CYG_HAL_POWERPC_MPC603
    {
        // Use BATs to map the memory.
        cyg_uint32 ubat, lbat;

        ubat = (virt & UBAT_BEPIMASK) | UBAT_VS | UBAT_VP;
        lbat = (phys & LBAT_BRPNMASK);
        if (flags & CYGARC_MEMDESC_CI) 
            lbat |= LBAT_I;
        if (flags & CYGARC_MEMDESC_GUARDED) 
            lbat |= LBAT_G;
        
        // There are 4 BATs, size is programmable.
        while (id < 4 && size > 0) {
            cyg_uint32 blk_size = 128*1024;
            cyg_uint32 bl = 0;
            while (blk_size < 256*1024*1024 && blk_size < size) {
                blk_size *= 2;
                bl = (bl << 1) | 1;
            }
            ubat = (ubat & ~UBAT_BLMASK) | (bl << 2);

            switch (id) {
            case 0:
                CYGARC_MTSPR (IBAT0U, ubat);
                CYGARC_MTSPR (IBAT0L, lbat);
                CYGARC_MTSPR (DBAT0U, ubat);
                CYGARC_MTSPR (DBAT0L, lbat);
                break;
            case 1:
                CYGARC_MTSPR (IBAT1U, ubat);
                CYGARC_MTSPR (IBAT1L, lbat);
                CYGARC_MTSPR (DBAT1U, ubat);
                CYGARC_MTSPR (DBAT1L, lbat);
                break;
            case 2:
                CYGARC_MTSPR (IBAT2U, ubat);
                CYGARC_MTSPR (IBAT2L, lbat);
                CYGARC_MTSPR (DBAT2U, ubat);
                CYGARC_MTSPR (DBAT2L, lbat);
                break;
            case 3:
                CYGARC_MTSPR (IBAT3U, ubat);
                CYGARC_MTSPR (IBAT3L, lbat);
                CYGARC_MTSPR (DBAT3U, ubat);
                CYGARC_MTSPR (DBAT3L, lbat);
                break;
            }

            size -= blk_size;
            id++;
        }
    }
#endif


#ifdef CYG_HAL_POWERPC_MPC8xx
    {
        // The MPC8xx CPUs do not have BATs. Fortunately we don't
        // currently use the MMU, so we can simulate BATs by using the
        // TLBs.

        cyg_uint32 epn, rpn, ctr, twc;
        int max_tlbs;

#if defined(CYG_HAL_POWERPC_MPC860)
        // There are 32 TLBs.
        max_tlbs = 32;
#endif
#if defined(CYG_HAL_POWERPC_MPC823) || defined(CYG_HAL_POWERPC_MPC850)
        // There are 8 TLBs.
        max_tlbs = 8;
#endif

        epn = (virt & MI_EPN_EPNMASK) | MI_EPN_EV;
        rpn = ((phys & MI_RPN_RPNMASK) 
               | MI_RPN_PPRWRW | MI_RPN_LPS | MI_RPN_SH | MI_RPN_V);
        if (flags & CYGARC_MEMDESC_CI) 
            rpn |= MI_RPN_CI;

        twc = MI_TWC_PS8MB | MI_TWC_V;
        if (flags & CYGARC_MEMDESC_GUARDED) 
            twc |= MI_TWC_G;

        // Ignore attempts to use more than max_tlbs.
        while (id < max_tlbs && size > 0) {
            ctr = id << MI_CTR_INDX_SHIFT;

            // Instruction TLB.
            CYGARC_MTSPR (MI_TWC, twc);
            CYGARC_MTSPR (MI_CTR, ctr);
            CYGARC_MTSPR (MI_EPN, epn);
            CYGARC_MTSPR (MI_RPN, rpn);

            // Data TLB.
            {
                cyg_uint32 drpn;

                // Need to mark data page as changed or an exception
                // will be generated on first write to the page.
                drpn = rpn | MD_RPN_CHANGED;

                CYGARC_MTSPR (MD_TWC, twc);
                CYGARC_MTSPR (MD_CTR, ctr);
                CYGARC_MTSPR (MD_EPN, epn);
                CYGARC_MTSPR (MD_RPN, drpn);
            }

            // Move to next 8MB block.
            size -= 8*1024*1024;
            epn  += 8*1024*1024;
            rpn  += 8*1024*1024;
            id++;
        }
    }
#endif

    return id;
}


// Initialize MMU to a sane (NOP) state.
static void
hal_clear_MMU (void)
{
#ifdef CYG_HAL_POWERPC_MPC603
    {
        cyg_uint32 ubat, lbat;

        // Initialize BATs with 0 -- VS&VP are unset, making all matches fail
        ubat = 0;
        lbat = 0;

        CYGARC_MTSPR (IBAT0U, ubat);
        CYGARC_MTSPR (IBAT0L, lbat);
        CYGARC_MTSPR (DBAT0U, ubat);
        CYGARC_MTSPR (DBAT0L, lbat);
        CYGARC_MTSPR (IBAT1U, ubat);
        CYGARC_MTSPR (IBAT1L, lbat);
        CYGARC_MTSPR (DBAT1U, ubat);
        CYGARC_MTSPR (DBAT1L, lbat);
        CYGARC_MTSPR (IBAT2U, ubat);
        CYGARC_MTSPR (IBAT2L, lbat);
        CYGARC_MTSPR (DBAT2U, ubat);
        CYGARC_MTSPR (DBAT2L, lbat);
        CYGARC_MTSPR (IBAT3U, ubat);
        CYGARC_MTSPR (IBAT3L, lbat);
        CYGARC_MTSPR (DBAT3U, ubat);
        CYGARC_MTSPR (DBAT3L, lbat);
    }
#endif

#ifdef CYG_HAL_POWERPC_MPC8xx
    {
        // Initialize TLBs with 0, Valid bits unset.

        cyg_uint32 ctr;
        int id;
        int max_tlbs;

#if defined(CYG_HAL_POWERPC_MPC860)
        // There are 32 TLBs.
        max_tlbs = 32;
#endif
#if defined(CYG_HAL_POWERPC_MPC823) || defined(CYG_HAL_POWERPC_MPC850)
        // There are 8 TLBs.
        max_tlbs = 8;
#endif

        CYGARC_MTSPR (M_CASID, 0);

        for (id = 0; id < max_tlbs; id++) {
            ctr = id << MI_CTR_INDX_SHIFT;

            // Instruction TLBs.
            CYGARC_MTSPR (MI_TWC, 0);
            CYGARC_MTSPR (MI_CTR, ctr);
            CYGARC_MTSPR (MI_EPN, 0);
            CYGARC_MTSPR (MI_RPN, 0);
            // Data TLBs.
            CYGARC_MTSPR (MD_TWC, 0);
            CYGARC_MTSPR (MD_CTR, ctr);
            CYGARC_MTSPR (MD_EPN, 0);
            CYGARC_MTSPR (MD_RPN, 0);
        }
    }
#endif
}

// The memory map is weakly defined, allowing the application to redefine
// it if necessary. The regions defined below are the minimum requirements.
cyg_memdesc_t cyg_hal_mem_map[] CYGBLD_ATTRIB_WEAK = {
#ifdef CYGPKG_HAL_POWERPC_COGENT
    // Mapping for the Cogent CMA101/102 boards.
    {0xfff00000, 0xfff00000, 0x00100000,
     CYGARC_MEMDESC_CI},            // ROM region
    {0xff000000, 0xff000000, 0x00100000, 
     CYGARC_MEMDESC_CI},            // MCP registers
    {0x0e000000, 0x0e000000, 0x01000000, 
     CYGARC_MEMDESC_CI},            // IO registers
    {0x00000000, 0x00000000, 0x00800000,
     0},                            // Main memory
#endif
#ifdef CYGPKG_HAL_POWERPC_SIM
    // Mapping for the simulator. This is only used for special debugging:
    // CYG_MSR in vectors.S must be changed to enable the MMU.
    {0x00000000, 0x00000000, 0x00800000,
     0},                            // Main memory
    {0xf0000000, 0xf0000000, 0x00020000, 
     CYGARC_MEMDESC_CI},            // diag registers
#endif
    {0, 0, 0, 0}                    // Table end
};

externC void hal_MMU_init (void)
{
    int id = 0;
    int i  = 0;

    hal_clear_MMU ();

    while (cyg_hal_mem_map[i].size) {
        id = hal_map_memory (id, 
                             cyg_hal_mem_map[i].virtual_addr,
                             cyg_hal_mem_map[i].physical_addr,
                             cyg_hal_mem_map[i].size,
                             cyg_hal_mem_map[i].flags);
        i++;
    }
}

//---------------------------------------------------------------------------
// Initial cache enabling
externC void
hal_enable_caches(void)
{
#ifdef CYGPKG_HAL_POWERPC_COGENT
    // Initialize caches.
    HAL_ICACHE_UNLOCK_ALL();    
    HAL_DCACHE_UNLOCK_ALL();
    HAL_ICACHE_INVALIDATE_ALL();    
    HAL_DCACHE_INVALIDATE_ALL();

    // Enable caches.
#if defined(CYG_HAL_STARTUP_RAM)
    // Cogent board doesn't support burst access to the ROM and on the
    // MPC8xx caches require burst.
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_ENABLE();
#endif
#endif

#ifdef CYG_HAL_POWERPC_MPC8xx
    // Disable serialization
    {
        cyg_uint32 ictrl;
        CYGARC_MFSPR (ICTRL, ictrl);
        ictrl |= ICTRL_NOSERSHOW;
        CYGARC_MTSPR (ICTRL, ictrl);
    }
#endif
}

//---------------------------------------------------------------------------
// End of hal_misc.c
