#ifndef CYGONCE_HAL_HAL_ARCH_H
#define CYGONCE_HAL_HAL_ARCH_H

//==========================================================================
//
//      hal_arch.h
//
//      Architecture specific abstractions
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
// Contributors: nickg
// Date:         1999-02-17
// Purpose:      Define architecture abstractions
// Usage:        #include <cyg/hal/hal_arch.h>
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/var_arch.h>

//--------------------------------------------------------------------------
// Processor saved states:
// The layout of this structure is also defined in "mips.inc", for assembly
// code. Do not change this without changing that (or vice versa).

#if defined(CYGHWR_HAL_MIPS_FPU)
# if defined(CYGHWR_HAL_MIPS_FPU_64BIT)
#  define CYG_HAL_FPU_REG CYG_WORD64
# elif defined(CYGHWR_HAL_MIPS_FPU_32BIT)
#  define CYG_HAL_FPU_REG CYG_WORD32
# else
# error MIPS FPU register size not defined
# endif
#endif

typedef struct 
{
    // These are common to all saved states
    CYG_ADDRWORD        d[32];          /* Data regs                    */
    CYG_ADDRWORD        hi;             /* hi word of mpy/div reg       */
    CYG_ADDRWORD        lo;             /* lo word of mpy/div reg       */
#ifdef CYGHWR_HAL_MIPS_FPU
    CYG_HAL_FPU_REG     f[32];          /* FPU registers                */
    CYG_ADDRWORD        fcr31;          /* FPU control/status register  */
    CYG_ADDRWORD        fppad;          /* Dummy location to make this  */
                                        /* structure a multiple of 8    */
                                        /* bytes long.                  */
#endif
    
    // These are only saved for exceptions and interrupts
    CYG_ADDRWORD        vector;         /* Vector number                */
    CYG_ADDRWORD        pc;             /* Program Counter              */
    CYG_ADDRWORD        sr;             /* Status Reg                   */
    CYG_ADDRWORD        cache;          /* Cache control register       */


    // These are only saved for exceptions, and are not restored
    // when continued.
    CYG_ADDRWORD        cause;          /* Exception cause register     */
    CYG_ADDRWORD        badvr;          /* Bad virtual address reg      */
    CYG_ADDRWORD        prid;           /* Processor Version            */
    CYG_ADDRWORD        config;         /* Config register              */
} HAL_SavedRegisters;

//--------------------------------------------------------------------------
// Exception handling function.
// This function is defined by the kernel according to this prototype. It is
// invoked from the HAL to deal with any CPU exceptions that the HAL does
// not want to deal with itself. It usually invokes the kernel's exception
// delivery mechanism.

externC void cyg_hal_deliver_exception( CYG_WORD code, CYG_ADDRWORD data );

//--------------------------------------------------------------------------
// Bit manipulation macros

externC cyg_uint32 hal_lsbit_index(cyg_uint32 mask);
externC cyg_uint32 hal_msbit_index(cyg_uint32 mask);

#define HAL_LSBIT_INDEX(index, mask) index = hal_lsbit_index(mask);

#define HAL_MSBIT_INDEX(index, mask) index = hal_msbit_index(mask);

//--------------------------------------------------------------------------
// Context Initialization


// Optional FPU context initialization
#ifdef CYGHWR_HAL_MIPS_FPU
#define HAL_THREAD_INIT_FPU_CONTEXT( _regs_, _id_ )                     \
{                                                                       \
    for( _i_ = 0; _i_ < 32; _i_++ ) (_regs_)->f[_i_] = (_id_)|_i_;      \
    (_regs_)->fcr31 = 0;                                                \
}
#else
#define HAL_THREAD_INIT_FPU_CONTEXT( _regs_, _id_ )
#endif


// Initialize the context of a thread.
// Arguments:
// _sparg_ name of variable containing current sp, will be written with new sp
// _thread_ thread object address, passed as argument to entry point
// _entry_ entry point address.
// _id_ bit pattern used in initializing registers, for debugging.
#define HAL_THREAD_INIT_CONTEXT( _sparg_, _thread_, _entry_, _id_ )        \
{                                                                          \
    register CYG_WORD _sp_ = ((CYG_WORD)_sparg_)-56;                       \
    register HAL_SavedRegisters *_regs_;                                   \
    int _i_;                                                               \
    _sp_ = _sp_ & 0xFFFFFFF0;                                              \
    _regs_ = (HAL_SavedRegisters *)(((_sp_) - sizeof(HAL_SavedRegisters))&0xFFFFFFF0);  \
    for( _i_ = 0; _i_ < 32; _i_++ ) (_regs_)->d[_i_] = (_id_)|_i_;         \
    HAL_THREAD_INIT_FPU_CONTEXT( _regs_, _id_ );                           \
    (_regs_)->d[29] = (CYG_WORD)(_sp_);       /* SP = top of stack      */ \
    (_regs_)->d[04] = (CYG_WORD)(_thread_);   /* R4 = arg1 = thread ptr */ \
    (_regs_)->lo = 0;                         /* LO = 0                 */ \
    (_regs_)->hi = 0;                         /* HI = 0                 */ \
    (_regs_)->d[30] = (CYG_WORD)(_sp_);       /* FP = top of stack      */ \
    (_regs_)->d[31] = (CYG_WORD)(_entry_);    /* LR(d[31]) = entry point*/ \
    (_regs_)->pc = (CYG_WORD)(_entry_);       /* PC = entry point       */ \
    (_regs_)->sr  = 0x00000001;               /* SR = ls 3 bits only    */ \
    _sparg_ = (CYG_ADDRESS)_regs_;                                         \
}

//--------------------------------------------------------------------------
// Context switch macros.
// The arguments are pointers to locations where the stack pointer
// of the current thread is to be stored, and from where the sp of the
// next thread is to be fetched.

externC void hal_thread_switch_context( CYG_ADDRESS to, CYG_ADDRESS from );
externC void hal_thread_load_context( CYG_ADDRESS to )
    __attribute__ ((noreturn));

#define HAL_THREAD_SWITCH_CONTEXT(_fspptr_,_tspptr_)                    \
        hal_thread_switch_context( (CYG_ADDRESS)_tspptr_,               \
                                   (CYG_ADDRESS)_fspptr_);

#define HAL_THREAD_LOAD_CONTEXT(_tspptr_)                               \
        hal_thread_load_context( (CYG_ADDRESS)_tspptr_ );

//--------------------------------------------------------------------------
// Execution reorder barrier.
// When optimizing the compiler can reorder code. In multithreaded systems
// where the order of actions is vital, this can sometimes cause problems.
// This macro may be inserted into places where reordering should not happen.
// The "memory" keyword is potentially unnecessary, but it is harmless to
// keep it.

#define HAL_REORDER_BARRIER() asm volatile ( "" : : : "memory" )

//--------------------------------------------------------------------------
// Breakpoint support
// HAL_BREAKPOINT() is a code sequence that will cause a breakpoint to
// happen if executed.
// HAL_BREAKINST is the value of the breakpoint instruction and
// HAL_BREAKINST_SIZE is its size in bytes.

#define HAL_BREAKPOINT(_label_)                 \
asm volatile (" .globl  " #_label_ ";"          \
              #_label_":"                       \
              " break   5"                      \
    );

#define HAL_BREAKINST           0x0005000d

#define HAL_BREAKINST_SIZE      4

//--------------------------------------------------------------------------
// Thread register state manipulation for GDB support.

// Default to a 32 bit register size for GDB register dumps.
#ifndef CYG_HAL_GDB_REG
#define CYG_HAL_GDB_REG CYG_WORD32
#endif

// Translate a stack pointer as saved by the thread context macros above into
// a pointer to a HAL_SavedRegisters structure.
#define HAL_THREAD_GET_SAVED_REGISTERS( _sp_, _regs_ )          \
        (_regs_) = (HAL_SavedRegisters *)(_sp_)

// Copy a set of registers from a HAL_SavedRegisters structure into a
// GDB ordered array.    
#define HAL_GET_GDB_REGISTERS( _aregval_ , _regs_ )             \
{                                                               \
    CYG_HAL_GDB_REG *_regval_ = (CYG_HAL_GDB_REG *)(_aregval_); \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ < 32; _i_++ )                             \
        _regval_[_i_] = (_regs_)->d[_i_];                       \
                                                                \
    _regval_[32] = (_regs_)->sr;                                \
    _regval_[33] = (_regs_)->lo;                                \
    _regval_[34] = (_regs_)->hi;                                \
    _regval_[35] = (_regs_)->badvr;                             \
    _regval_[36] = (_regs_)->cause;                             \
    _regval_[37] = (_regs_)->pc;                                \
}

// Copy a GDB ordered array into a HAL_SavedRegisters structure.
#define HAL_SET_GDB_REGISTERS( _regs_ , _aregval_ )             \
{                                                               \
    CYG_HAL_GDB_REG *_regval_ = (CYG_HAL_GDB_REG *)(_aregval_); \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ < 32; _i_++ )                             \
        (_regs_)->d[_i_] = _regval_[_i_];                       \
                                                                \
    (_regs_)->sr = _regval_[32];                                \
    (_regs_)->lo = _regval_[33];                                \
    (_regs_)->hi = _regval_[34];                                \
    (_regs_)->badvr = _regval_[35];                             \
    (_regs_)->cause = _regval_[36];                             \
    (_regs_)->pc = _regval_[37];                                \
}

//--------------------------------------------------------------------------
// HAL setjmp
// Note: These definitions are repeated in hal_arch.h. If changes are
// required remember to update both sets.

#define CYGARC_JMP_BUF_SP        0
#define CYGARC_JMP_BUF_R16       1
#define CYGARC_JMP_BUF_R17       2
#define CYGARC_JMP_BUF_R18       3
#define CYGARC_JMP_BUF_R19       4
#define CYGARC_JMP_BUF_R20       5
#define CYGARC_JMP_BUF_R21       6
#define CYGARC_JMP_BUF_R22       7
#define CYGARC_JMP_BUF_R23       8
#define CYGARC_JMP_BUF_R28       9
#define CYGARC_JMP_BUF_R30      10
#define CYGARC_JMP_BUF_R31      11

#define CYGARC_JMP_BUF_SIZE     12

typedef cyg_uint32 hal_jmp_buf[CYGARC_JMP_BUF_SIZE];

externC int hal_setjmp(hal_jmp_buf env);
externC void hal_longjmp(hal_jmp_buf env, int val);

//-------------------------------------------------------------------------
// Idle thread code.
// This macro is called in the idle thread loop, and gives the HAL the
// chance to insert code. Typical idle thread behaviour might be to halt the
// processor.

externC void hal_idle_thread_action(cyg_uint32 loop_count);

#define HAL_IDLE_THREAD_ACTION(_count_) hal_idle_thread_action(_count_)

//--------------------------------------------------------------------------
// Minimal and sensible stack sizes: the intention is that applications
// will use these to provide a stack size in the first instance prior to
// proper analysis.  Idle thread stack should be this big.

//    THESE ARE NOT INTENDED TO BE MICROMETRICALLY ACCURATE FIGURES.
//           THEY ARE HOWEVER ENOUGH TO START PROGRAMMING.
// YOU MUST MAKE YOUR STACKS LARGER IF YOU HAVE LARGE "AUTO" VARIABLES!

// We define quite large stack needs for SPARClite, for it requires 576
// bytes (144 words) to process an interrupt and thread-switch, and
// momentarily, but needed in case of recursive interrupts, it needs 208
// words - if a sequence of saves to push out other regsets is interrupted.

// This is not a config option because it should not be adjusted except
// under "enough rope" sort of disclaimers.

// Typical case stack frame size: return link + 4 pushed registers + some locals.
#define CYGNUM_HAL_STACK_FRAME_SIZE (48)

// Stack needed for a context switch:
#if defined(CYGHWR_HAL_MIPS_FPU)
# if defined(CYGHWR_HAL_MIPS_FPU_64BIT)
#define CYGNUM_HAL_STACK_CONTEXT_SIZE (((32+12)*4)+(32*8))
# elif defined(CYGHWR_HAL_MIPS_FPU_32BIT)
#define CYGNUM_HAL_STACK_CONTEXT_SIZE (((32+12)*4)+(32*4))
# else
# error MIPS FPU register size not defined
# endif
#else
#define CYGNUM_HAL_STACK_CONTEXT_SIZE ((32+10)*4)
#endif



// Interrupt + call to ISR, interrupt_end() and the DSR
#define CYGNUM_HAL_STACK_INTERRUPT_SIZE (4+2*CYGNUM_HAL_STACK_CONTEXT_SIZE) 

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

// An interrupt stack which is large enough for all possible interrupt
// conditions (and only used for that purpose) exists.  "User" stacks
// can be much smaller

#define CYGNUM_HAL_STACK_SIZE_MINIMUM (CYGNUM_HAL_STACK_CONTEXT_SIZE+      \
                                       CYGNUM_HAL_STACK_INTERRUPT_SIZE*2+  \
                                       CYGNUM_HAL_STACK_FRAME_SIZE*8)
#define CYGNUM_HAL_STACK_SIZE_TYPICAL (2048)

#else // CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK 

// No separate interrupt stack exists.  Make sure all threads contain
// a stack sufficiently large.

#define CYGNUM_HAL_STACK_SIZE_MINIMUM (4096)
#define CYGNUM_HAL_STACK_SIZE_TYPICAL (4096)

#endif

//--------------------------------------------------------------------------
#endif // CYGONCE_HAL_HAL_ARCH_H
// End of hal_arch.h
