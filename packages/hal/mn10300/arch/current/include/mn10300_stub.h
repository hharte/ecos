#ifndef CYGONCE_HAL_MN10300_STUB_H
#define CYGONCE_HAL_MN10300_STUB_H
//========================================================================
//
//      mn10300_stub.h
//
//      MN10300-specific definitions for generic stub
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     Red Hat, jskov
// Contributors:  Red Hat, jskov, dmoseley
// Date:          1998-11-06
// Purpose:       
// Description:   MN10300-specific definitions for generic stub
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#ifdef __cplusplus
extern "C" {
#endif

#define NUMREGS    32

#define REGSIZE( _x_ ) (4)

typedef unsigned long target_register_t;

enum regnames {
  D0, D1, D2, D3, A0, A1, A2, A3,
  SP, PC, MDR, PSW, LIR, LAR
#ifdef CYGPKG_HAL_MN10300_AM33
  , MDRQ,
  R0, R1, R2, R3, R4, R5, R6, R7,
  SSP, MSP, USP, MCRH, MCRL, MCVF
#endif  
};

typedef enum regnames regnames_t;

/* Given a trap value TRAP, return the corresponding signal. */
extern int __computeSignal (unsigned int trap_number);

/* Return the SPARC trap number corresponding to the last-taken trap. */
extern int __get_trap_number (void);

/* Return the currently-saved value corresponding to register REG. */
extern target_register_t get_register (regnames_t reg);

/* Store VALUE in the register corresponding to WHICH. */
extern void put_register (regnames_t which, target_register_t value);

/* Set the currently-saved pc register value to PC. This also updates NPC
   as needed. */
#if !defined(SET_PC_PROTOTYPE_EXISTS) && !defined(set_pc)
#define SET_PC_PROTOTYPE_EXISTS
extern void set_pc (target_register_t pc);
#endif

/* Set things up so that the next user resume will execute one instruction.
   This may be done by setting breakpoints or setting a single step flag
   in the saved user registers, for example. */
#ifndef __single_step
void __single_step (void);
#endif

/* Clear the single-step state. */
void __clear_single_step (void);

/* If the breakpoint we hit is in the breakpoint() instruction, return a
   non-zero value. */
#ifndef __is_breakpoint_function
extern int __is_breakpoint_function (void);
#endif

/* Skip the current instruction. */
extern void __skipinst (void);

extern void __install_breakpoints (void);

extern void __clear_breakpoints (void);

extern void __install_breakpoint_list (void);

extern void __clear_breakpoint_list (void);

extern int __is_bsp_syscall(void);

extern int hal_syscall_handler(void);

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif // ifndef CYGONCE_HAL_MN10300_STUB_H
