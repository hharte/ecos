#ifndef CYGONCE_HAL_HAL_INTR_H
#define CYGONCE_HAL_HAL_INTR_H

//==========================================================================
//
//      hal_intr.h
//
//      HAL Interrupt and clock support
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
// by Cygnus are Copyright (C) 1998,1999 Cygnus Solutions.  All Rights Reserved.
// -------------------------------------------
//
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg
// Contributors: nickg, jskov,
//               gthomas, jlarmour
// Date:         1999-02-16
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock.
//              
// Usage:
//              #include <cyg/hal/hal_intr.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_io.h>

#include <cyg/hal/var_intr.h>

//--------------------------------------------------------------------------
// MIPS vectors. 

// These are the exception codes presented in the Cause register and
// correspond to VSRs. These values are the ones to use for HAL_VSR_GET/SET

// External interrupt
#define CYGNUM_HAL_VECTOR_INTERRUPT            0
// TLB modification exception
#define CYGNUM_HAL_VECTOR_TLB_MOD              1
// TLB miss (Load or IFetch)
#define CYGNUM_HAL_VECTOR_TLB_LOAD_REFILL      2
// TLB miss (Store)
#define CYGNUM_HAL_VECTOR_TLB_STORE_REFILL     3
// Address error (Load or Ifetch)
#define CYGNUM_HAL_VECTOR_LOAD_ADDRESS         4
// Address error (store)
#define CYGNUM_HAL_VECTOR_STORE_ADDRESS        5
// Bus error (Ifetch)
#define CYGNUM_HAL_VECTOR_IBE                  6
// Bus error (data load or store)
#define CYGNUM_HAL_VECTOR_DBE                  7
// System call
#define CYGNUM_HAL_VECTOR_SYSTEM_CALL          8
// Break point
#define CYGNUM_HAL_VECTOR_BREAKPOINT           9
// Reserved instruction
#define CYGNUM_HAL_VECTOR_RESERVED_INSTRUCTION 10
// Coprocessor unusable
#define CYGNUM_HAL_VECTOR_COPROCESSOR          11
// Arithmetic overflow
#define CYGNUM_HAL_VECTOR_OVERFLOW             12
// Reserved
#define CYGNUM_HAL_VECTOR_RESERVED_13          13
// Floating point exception
#ifdef  CYGHWR_HAL_MIPS_FPU
#define CYGNUM_HAL_VECTOR_FPE                  15
#endif

#define CYGNUM_HAL_VSR_MIN                     0
#define CYGNUM_HAL_VSR_MAX                     15
#define CYGNUM_HAL_VSR_COUNT                   16

// Exception vectors. These are the values used when passed out to an
// external exception handler using cyg_hal_deliver_exception()

#define CYGNUM_HAL_EXCEPTION_DATA_TLBERROR_ACCESS CYGNUM_HAL_VECTOR_TLB_MOD
#define CYGNUM_HAL_EXCEPTION_DATA_TLBMISS_ACCESS \
          CYGNUM_HAL_VECTOR_TLB_LOAD_REFILL
#define CYGNUM_HAL_EXCEPTION_DATA_TLBMISS_WRITE \
          CYGNUM_HAL_VECTOR_TLB_STORE_REFILL
#define CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_ACCESS \
          CYGNUM_HAL_VECTOR_LOAD_ADDRESS
#define CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_WRITE \
          CYGNUM_HAL_VECTOR_STORE_ADDRESS
#define CYGNUM_HAL_EXCEPTION_CODE_ACCESS    CYGNUM_HAL_VECTOR_IBE
#define CYGNUM_HAL_EXCEPTION_DATA_ACCESS    CYGNUM_HAL_VECTOR_DBE
#define CYGNUM_HAL_EXCEPTION_SYSTEM_CALL    CYGNUM_HAL_VECTOR_SYSTEM_CALL
#define CYGNUM_HAL_EXCEPTION_INSTRUCTION_BP CYGNUM_HAL_VECTOR_BREAKPOINT
#define CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION \
          CYGNUM_HAL_VECTOR_RESERVED_INSTRUCTION
#define CYGNUM_HAL_EXCEPTION_COPROCESSOR    CYGNUM_HAL_VECTOR_COPROCESSOR
#define CYGNUM_HAL_EXCEPTION_OVERFLOW       CYGNUM_HAL_VECTOR_OVERFLOW

// Min/Max exception numbers and how many there are
#define CYGNUM_HAL_EXCEPTION_MIN                1
#define CYGNUM_HAL_EXCEPTION_MAX               13
#define CYGNUM_HAL_EXCEPTION_COUNT             13


#ifndef CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED

// the default for all MIPS variants is to use the 6 bits
// in the cause register.

#define CYGNUM_HAL_INTERRUPT_0                0
#define CYGNUM_HAL_INTERRUPT_1                1
#define CYGNUM_HAL_INTERRUPT_2                2
#define CYGNUM_HAL_INTERRUPT_3                3
#define CYGNUM_HAL_INTERRUPT_4                4
#define CYGNUM_HAL_INTERRUPT_5                5

// Min/Max ISR numbers and how many there are
#define CYGNUM_HAL_ISR_MIN                     0
#define CYGNUM_HAL_ISR_MAX                     5
#define CYGNUM_HAL_ISR_COUNT                   6

// The vector used by the Real time clock. The default here is to use
// interrupt 5, which is connected to the counter/comparator registers
// in many MIPS variants.

#ifndef CYGNUM_HAL_INTERRUPT_RTC
#define CYGNUM_HAL_INTERRUPT_RTC            CYGNUM_HAL_INTERRUPT_5
#endif

#define CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED

#endif

//--------------------------------------------------------------------------
// Static data used by HAL

// ISR tables
externC volatile CYG_ADDRESS    hal_interrupt_handlers[CYGNUM_HAL_ISR_COUNT];
externC volatile CYG_ADDRWORD   hal_interrupt_data[CYGNUM_HAL_ISR_COUNT];
externC volatile CYG_ADDRESS    hal_interrupt_objects[CYGNUM_HAL_ISR_COUNT];

// VSR table
externC volatile CYG_ADDRESS    hal_vsr_table[CYGNUM_HAL_VSR_MAX+1];

//--------------------------------------------------------------------------
// Default ISR

externC cyg_uint32 hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);

//--------------------------------------------------------------------------
// Interrupt state storage

typedef cyg_uint32 CYG_INTERRUPT_STATE;

//--------------------------------------------------------------------------
// Interrupt control macros
// Beware of nops in this code. They fill delay slots and avoid CP0 hazards
// that might otherwise cause following code to run in the wrong state or
// cause a resource conflict.

#define HAL_DISABLE_INTERRUPTS(_old_)           \
{                                               \
    asm volatile (                              \
        "mfc0   $8,$12; nop;"                   \
        "move   %0,$8;"                         \
        "and    $8,$8,0XFFFFFFFE;"              \
        "mtc0   $8,$12;"                        \
        "nop; nop; nop;"                        \
        "and    %0,%0,0X1;"                     \
        : "=r"(_old_)                           \
        :                                       \
        : "$8"                                  \
        );                                      \
}

#define HAL_ENABLE_INTERRUPTS()                 \
{                                               \
    asm volatile (                              \
        "mfc0   $8,$12; nop;"                   \
        "or     $8,$8,1;"                       \
        "mtc0   $8,$12;"                        \
        "nop; nop; nop;"                        \
        :                                       \
        :                                       \
        : "$8"                                  \
        );                                      \
}

#define HAL_RESTORE_INTERRUPTS(_old_)           \
{                                               \
    asm volatile (                              \
        "mfc0   $8,$12; nop;"                   \
        "and    %0,%0,0x1;"                     \
        "or     $8,$8,%0;"                      \
        "mtc0   $8,$12;"                        \
        "nop; nop; nop;"                        \
        :                                       \
        : "r"(_old_)                            \
        : "$8"                                  \
        );                                      \
}

#define HAL_QUERY_INTERRUPTS( _state_ )         \
{                                               \
    asm volatile (                              \
        "mfc0   %0,$12; nop;"                   \
        "and    %0,%0,0x1;"                     \
        : "=r"(_state_)                         \
        :                                       \
        : "$8"                                  \
        );                                      \
}

//--------------------------------------------------------------------------
// Routine to execute DSRs using separate interrupt stack

#ifdef  CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
externC void hal_interrupt_stack_call_pending_DSRs(void);
#define HAL_INTERRUPT_STACK_CALL_PENDING_DSRS() \
    hal_interrupt_stack_call_pending_DSRs()

// these are offered solely for stack usage testing
// if they are not defined, then there is no interrupt stack.
#define HAL_INTERRUPT_STACK_BASE cyg_interrupt_stack_base
#define HAL_INTERRUPT_STACK_TOP  cyg_interrupt_stack
// use them to declare these extern however you want:
//       extern char HAL_INTERRUPT_STACK_BASE[];
//       extern char HAL_INTERRUPT_STACK_TOP[];
// is recommended
#endif

//--------------------------------------------------------------------------
// Vector translation.
// For chained interrupts we only have a single vector though which all
// are passed. For unchained interrupts we have a vector per interrupt.

#ifndef HAL_TRANSLATE_VECTOR

#if defined(CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN)

#define HAL_TRANSLATE_VECTOR(_vector_,_index_) (_index_) = 0

#else

#define HAL_TRANSLATE_VECTOR(_vector_,_index_) (_index_) = (_vector_)

#endif

#endif

//--------------------------------------------------------------------------
// Interrupt and VSR attachment macros

#define HAL_INTERRUPT_IN_USE( _vector_, _state_)                          \
    CYG_MACRO_START                                                       \
    cyg_uint32 _index_;                                                   \
    HAL_TRANSLATE_VECTOR ((_vector_), _index_);                           \
                                                                          \
    if( hal_interrupt_handlers[_index_] == (CYG_ADDRESS)hal_default_isr ) \
        (_state_) = 0;                                                    \
    else                                                                  \
        (_state_) = 1;                                                    \
    CYG_MACRO_END

#define HAL_INTERRUPT_ATTACH( _vector_, _isr_, _data_, _object_ )           \
{                                                                           \
    cyg_uint32 _index_;                                                     \
    HAL_TRANSLATE_VECTOR( _vector_, _index_ );                              \
                                                                            \
    if( hal_interrupt_handlers[_index_] == (CYG_ADDRESS)hal_default_isr )   \
    {                                                                       \
        hal_interrupt_handlers[_index_] = (CYG_ADDRESS)_isr_;               \
        hal_interrupt_data[_index_] = (CYG_ADDRWORD)_data_;                 \
        hal_interrupt_objects[_index_] = (CYG_ADDRESS)_object_;             \
    }                                                                       \
}

#define HAL_INTERRUPT_DETACH( _vector_, _isr_ )                         \
{                                                                       \
    cyg_uint32 _index_;                                                 \
    HAL_TRANSLATE_VECTOR( _vector_, _index_ );                          \
                                                                        \
    if( hal_interrupt_handlers[_index_] == (CYG_ADDRESS)_isr_ )         \
    {                                                                   \
        hal_interrupt_handlers[_index_] = (CYG_ADDRESS)hal_default_isr; \
        hal_interrupt_data[_index_] = 0;                                \
        hal_interrupt_objects[_index_] = 0;                             \
    }                                                                   \
}

#define HAL_VSR_GET( _vector_, _pvsr_ )                 \
    *(_pvsr_) = (void (*)())hal_vsr_table[_vector_];
    

#define HAL_VSR_SET( _vector_, _vsr_, _poldvsr_ ) CYG_MACRO_START         \
    if( _poldvsr_ != NULL)                                                \
        *(CYG_ADDRESS *)_poldvsr_ = (CYG_ADDRESS)hal_vsr_table[_vector_]; \
    hal_vsr_table[_vector_] = (CYG_ADDRESS)_vsr_;                         \
CYG_MACRO_END

// This is an ugly name, but what it means is: grab the VSR back to eCos
// internal handling, or if you like, the default handler.  But if
// cooperating with GDB and CygMon, the default behaviour is to pass most
// exceptions to CygMon.  This macro undoes that so that eCos handles the
// exception.  So use it with care.

externC void __default_exception_vsr(void);
externC void __default_interrupt_vsr(void);

#define HAL_VSR_SET_TO_ECOS_HANDLER( _vector_, _poldvsr_ ) CYG_MACRO_START  \
    HAL_VSR_SET( _vector_, _vector_ == CYGNUM_HAL_VECTOR_INTERRUPT          \
                              ? (CYG_ADDRESS)__default_interrupt_vsr        \
                              : (CYG_ADDRESS)__default_exception_vsr,       \
                 _poldvsr_ );                                               \
CYG_MACRO_END

//--------------------------------------------------------------------------
// Interrupt controller access
// The default code here simply uses the fields present in the CP0 status
// and cause registers to implement this functionality.
// Beware of nops in this code. They fill delay slots and avoid CP0 hazards
// that might otherwise cause following code to run in the wrong state or
// cause a resource conflict.

#ifndef CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

#define HAL_INTERRUPT_MASK( _vector_ )          \
CYG_MACRO_START                                 \
    asm volatile (                              \
        "mfc0   $3,$12\n"                       \
        "la     $2,0x00000400\n"                \
        "sllv   $2,$2,%0\n"                     \
        "nor    $2,$2,$0\n"                     \
        "and    $3,$3,$2\n"                     \
        "mtc0   $3,$12\n"                       \
        "nop; nop; nop\n"                       \
        :                                       \
        : "r"(_vector_)                         \
        : "$2", "$3"                            \
        );                                      \
CYG_MACRO_END

#define HAL_INTERRUPT_UNMASK( _vector_ )        \
CYG_MACRO_START                                 \
    asm volatile (                              \
        "mfc0   $3,$12\n"                       \
        "la     $2,0x00000400\n"                \
        "sllv   $2,$2,%0\n"                     \
        "or     $3,$3,$2\n"                     \
        "mtc0   $3,$12\n"                       \
        "nop; nop; nop\n"                       \
        :                                       \
        : "r"(_vector_)                         \
        : "$2", "$3"                            \
        );                                      \
CYG_MACRO_END

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )   \
CYG_MACRO_START                                 \
    asm volatile (                              \
        "mfc0   $3,$13\n"                       \
        "la     $2,0x00000400\n"                \
        "sllv   $2,$2,%0\n"                     \
        "nor    $2,$2,$0\n"                     \
        "and    $3,$3,$2\n"                     \
        "mtc0   $3,$13\n"                       \
        "nop; nop; nop\n"                       \
        :                                       \
        : "r"(_vector_)                         \
        : "$2", "$3"                            \
        );                                      \
CYG_MACRO_END

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )

#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )

#define CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

#endif

//--------------------------------------------------------------------------
// Clock control.
// This code uses the count and compare registers that are present in many
// MIPS variants.
// Beware of nops in this code. They fill delay slots and avoid CP0 hazards
// that might otherwise cause following code to run in the wrong state or
// cause a resource conflict.

#ifndef CYGHWR_HAL_CLOCK_CONTROL_DEFINED

#define HAL_CLOCK_INITIALIZE( _period_ )        \
CYG_MACRO_START                                 \
    asm volatile (                              \
        "mtc0 $0,$9\n"                          \
        "nop; nop; nop\n"                       \
        "mtc0 %0,$11\n"                         \
        "nop; nop; nop\n"                       \
        :                                       \
        : "r"(_period_)                         \
        );                                      \
CYG_MACRO_END

#define HAL_CLOCK_RESET( _vector_, _period_ )   \
CYG_MACRO_START                                 \
    asm volatile (                              \
        "mtc0 $0,$9\n"                          \
        "nop; nop; nop\n"                       \
        "mtc0 %0,$11\n"                         \
        "nop; nop; nop\n"                       \
        :                                       \
        : "r"(_period_)                         \
        );                                      \
CYG_MACRO_END

#define HAL_CLOCK_READ( _pvalue_ )              \
CYG_MACRO_START                                 \
    register CYG_WORD32 result;                 \
    asm volatile (                              \
        "mfc0   %0,$9\n"                        \
        : "=r"(result)                          \
        );                                      \
    *(_pvalue_) = result;                       \
CYG_MACRO_END

#define CYGHWR_HAL_CLOCK_CONTROL_DEFINED

#endif

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY) && \
    !defined(HAL_CLOCK_LATENCY)
#define HAL_CLOCK_LATENCY( _pvalue_ ) HAL_CLOCK_READ(_pvalue_)
#endif

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_HAL_INTR_H
// End of hal_intr.h
