#ifndef CYGONCE_HAL_VAR_ARCH_H
#define CYGONCE_HAL_VAR_ARCH_H

//==========================================================================
//
//      var_arch.h
//
//      Architecture specific abstractions
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
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
// All Rights Reserved.                                                     
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
// Description:  This file contains any extra or modified definitions for
//               this variant of the architecture.
// Usage:        #include <cyg/hal/var_arch.h>
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

//--------------------------------------------------------------------------
// Processor saved states:

typedef struct HAL_SavedRegisters
{
    // These are common to all saved states and are in the order
    // stored and loaded by the movm instruction.
    CYG_ADDRWORD        vector;         /* Vector number/dummy          */
    CYG_ADDRWORD        lar;            /* Loop address register        */
    CYG_ADDRWORD        lir;            /* Loop instruction register    */
    CYG_ADDRWORD        mdr;            /* Multiply/Divide register     */
    CYG_ADDRWORD        a1;
    CYG_ADDRWORD        a0;
    CYG_ADDRWORD        d1;
    CYG_ADDRWORD        d0;
    CYG_ADDRWORD        a3;
    CYG_ADDRWORD        a2;
    CYG_ADDRWORD        d3;
    CYG_ADDRWORD        d2;
    CYG_ADDRWORD        mcvf;           /* MAC overflow flag            */
    CYG_ADDRWORD        mcrl;           /* MAC register low             */
    CYG_ADDRWORD        mcrh;           /* MAC register high            */
    CYG_ADDRWORD        mdrq;           /* Fast multiply/divide register*/
    CYG_ADDRWORD        e1;             /* extended registers           */
    CYG_ADDRWORD        e0;
    CYG_ADDRWORD        e7;
    CYG_ADDRWORD        e6;
    CYG_ADDRWORD        e5;
    CYG_ADDRWORD        e4;
    CYG_ADDRWORD        e3;
    CYG_ADDRWORD        e2;
    
    /* On interrupts the PC and PSW are pushed automatically by the     */
    /* CPU and SP is pushed for debugging reasons. On a thread switch   */
    /* the saved context is made to look the same.                      */

    CYG_ADDRWORD  sp;             /* Saved copy of SP in some states      */
    CYG_ADDRWORD  psw;            /* Status word                          */
    CYG_ADDRWORD  pc;             /* Program Counter                      */
    
} HAL_SavedRegisters;

//--------------------------------------------------------------------------
// Extra initialization for AM33 extended register set.

#define HAL_THREAD_INIT_CONTEXT_EXTRA(_regs_, _id_)     \
CYG_MACRO_START                                         \
    (_regs_)->e0        = (_id_)|0xeee0;                \
    (_regs_)->e1        = (_id_)|0xeee1;                \
    (_regs_)->e2        = (_id_)|0xeee2;                \
    (_regs_)->e3        = (_id_)|0xeee3;                \
    (_regs_)->e4        = (_id_)|0xeee4;                \
    (_regs_)->e5        = (_id_)|0xeee5;                \
    (_regs_)->e6        = (_id_)|0xeee6;                \
    (_regs_)->e7        = (_id_)|0xeee7;                \
    (_regs_)->mcrl      = 0;                            \
    (_regs_)->mcrh      = 0;                            \
    (_regs_)->mdrq      = 0;                            \
    (_regs_)->mcvf      = 0;                            \
CYG_MACRO_END

//--------------------------------------------------------------------------
// The following macros copy the extra AM33 registers between a
// HAL_SavedRegisters structure and a GDB register dump.

#define HAL_GET_GDB_EXTRA_REGISTERS( _regval_, _regs_ ) \
CYG_MACRO_START                                         \
    (_regval_)[15] = (_regs_)->e0;                      \
    (_regval_)[16] = (_regs_)->e1;                      \
    (_regval_)[17] = (_regs_)->e2;                      \
    (_regval_)[18] = (_regs_)->e3;                      \
    (_regval_)[19] = (_regs_)->e4;                      \
    (_regval_)[20] = (_regs_)->e5;                      \
    (_regval_)[21] = (_regs_)->e6;                      \
    (_regval_)[22] = (_regs_)->e7;                      \
                                                        \
    (_regval_)[23] = (_regs_)->sp;                      \
    (_regval_)[24] = (_regs_)->sp;                      \
    (_regval_)[25] = (_regs_)->sp;                      \
                                                        \
    (_regval_)[26] = (_regs_)->mcrh;                    \
    (_regval_)[27] = (_regs_)->mcrl;                    \
    (_regval_)[28] = (_regs_)->mcvf;                    \
CYG_MACRO_END

#define HAL_SET_GDB_EXTRA_REGISTERS( _regs_, _regval_ ) \
CYG_MACRO_START                                         \
    (_regs_)->e0 = (_regval_)[15];                      \
    (_regs_)->e1 = (_regval_)[16];                      \
    (_regs_)->e2 = (_regval_)[17];                      \
    (_regs_)->e3 = (_regval_)[18];                      \
    (_regs_)->e4 = (_regval_)[19];                      \
    (_regs_)->e5 = (_regval_)[20];                      \
    (_regs_)->e6 = (_regval_)[21];                      \
    (_regs_)->e7 = (_regval_)[22];                      \
                                                        \
    (_regs_)->sp = (_regval_)[23];                      \
    (_regs_)->sp = (_regval_)[24];                      \
    (_regs_)->sp = (_regval_)[25];                      \
                                                        \
    (_regs_)->mcrh = (_regval_)[26];                    \
    (_regs_)->mcrl = (_regval_)[27];                    \
    (_regs_)->mcvf = (_regval_)[28];                    \
CYG_MACRO_END

//--------------------------------------------------------------------------
#endif // CYGONCE_HAL_VAR_ARCH_H
// End of var_arch.h
