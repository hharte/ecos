/* arm_stub.c - helper functions for stub, generic to all ARM processors
 * 
 * Copyright (c) 1998,1999 Cygnus Solutions
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include <stddef.h>

#include <pkgconf/hal.h>

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#ifdef CYG_HAL_ARM_SIM
#error "GDB Stub support not implemented for ARM SIM"
#endif

#include <cyg/hal/hal_stub.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif


#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
#include <cyg/hal/dbg-threads-api.h>    // dbg_currthread_id
#endif

/* Given a trap value TRAP, return the corresponding signal. */

int __computeSignal (unsigned int trap_number)
{
    // should also catch CYGNUM_HAL_VECTOR_UNDEF_INSTRUCTION here but we
    // can't tell the different between a real one and a breakpoint :-(
    switch (trap_number) {
    case CYGNUM_HAL_VECTOR_ABORT_PREFETCH:      // Fall through
    case CYGNUM_HAL_VECTOR_ABORT_DATA:          // Fall through
    case CYGNUM_HAL_VECTOR_reserved:
        return SIGBUS;
    case CYGNUM_HAL_VECTOR_IRQ:
    case CYGNUM_HAL_VECTOR_FIQ:
        return SIGINT;
    default:
        return SIGTRAP;
    }
}


/* Return the trap number corresponding to the last-taken trap. */
int __get_trap_number (void)
{
    // The vector is not not part of the GDB register set so get it
    // directly from the save context.
    return _hal_registers->vector;
}

/* Set the currently-saved pc register value to PC. This also updates NPC
   as needed. */

void set_pc (target_register_t pc)
{
    put_register (PC, pc);
}


/*----------------------------------------------------------------------
 * Single-step support
 */

/* Set things up so that the next user resume will execute one instruction.
   This may be done by setting breakpoints or setting a single step flag
   in the saved user registers, for example. */

#define NULL_PTR (unsigned long *)0
static unsigned long *ss_saved_pc = NULL_PTR;
static unsigned long ss_saved_instr;


#define FIXME() {diag_printf("FIXME - %s\n", __FUNCTION__); }

static int
ins_will_execute(unsigned long ins)
{
    unsigned long psr = get_register(PS);  // condition codes
    int res = 0;
    switch ((ins & 0xF0000000) >> 28) {
    case 0x0: // EQ
        res = (psr & PS_Z) != 0;
        break;
    case 0x1: // NE
        res = (psr & PS_Z) == 0;
        break;
    case 0x2: // CS
        res = (psr & PS_C) != 0;
        break;
    case 0x3: // CC
        res = (psr & PS_C) == 0;
        break;
    case 0x4: // MI
        res = (psr & PS_N) != 0;
        break;
    case 0x5: // PL
        res = (psr & PS_N) == 0;
        break;
    case 0x6: // VS
        res = (psr & PS_V) != 0;
        break;
    case 0x7: // VC
        res = (psr & PS_V) == 0;
        break;
    case 0x8: // HI
        res = ((psr & PS_C) != 0) && ((psr & PS_Z) == 0);
        break;
    case 0x9: // LS
        res = ((psr & PS_C) == 0) || ((psr & PS_Z) != 0);
        break;
    case 0xA: // GE
        res = ((psr & (PS_N|PS_V)) == (PS_N|PS_V)) ||
            ((psr & (PS_N|PS_V)) == 0);
        break;
    case 0xB: // LT
        res = ((psr & (PS_N|PS_V)) == PS_N) ||
            ((psr & (PS_N|PS_V)) == PS_V);
        break;
    case 0xC: // GT
        res = ((psr & (PS_N|PS_V)) == (PS_N|PS_V)) ||
            ((psr & (PS_N|PS_V)) == 0);
        res = ((psr & PS_Z) == 0) && res;
        break;
    case 0xD: // LE
        res = ((psr & (PS_N|PS_V)) == PS_N) ||
            ((psr & (PS_N|PS_V)) == PS_V);
        res = ((psr & PS_Z) == PS_Z) || res;
        break;
    case 0xE: // AL
        res = TRUE;
        break;
    case 0xF: // NV
        res = FALSE;
        break;
    }
    return res;
}

static unsigned long
RmShifted(int shift)
{
    unsigned long Rm = get_register(shift & 0x00F);
    int shift_count;
    if ((shift & 0x010) == 0) {
        shift_count = (shift & 0xF80) >> 7;
    } else {
        shift_count = get_register((shift & 0xF00) >> 8);
    }
    switch ((shift & 0x060) >> 5) {
    case 0x0: // Logical left
        Rm <<= shift_count;
        break;
    case 0x1: // Logical right
        Rm >>= shift_count;
        break;
    case 0x2: // Arithmetic right
        Rm = (unsigned long)((long)Rm >> shift_count);
        break;
    case 0x3: // Rotate right
        if (shift_count == 0) {
            // Special case, RORx
            Rm >>= 1;
            if (get_register(PS) & PS_C) Rm |= 0x80000000;
        } else {
            Rm = (Rm >> shift_count) | (Rm << (32-shift_count));
        }
        break;
    }
    return Rm;
}

// Decide the next instruction to be executed for a given instruction
static unsigned long *
target_ins(unsigned long *pc, unsigned long ins)
{
    unsigned long new_pc, offset, op2;
    unsigned long Rn;
    int i, reg_count, c;
    switch ((ins & 0x0C000000) >> 26) {
    case 0x0:
        // Data processing
        new_pc = (unsigned long)(pc+1);
        if ((ins & 0x0000F000) == 0x0000F000) {
            // Destination register is PC
            if ((ins & 0x0FBF0000) != 0x010F0000) {
                Rn = (unsigned long)get_register((ins & 0x000F0000) >> 16);
                if ((ins & 0x02000000) == 0) {
                    op2 = RmShifted(ins & 0x00000FFF);
                } else {
                    op2 = ins & 0x000000FF;
                    i = (ins & 0x00000F00) >> 8;  // Rotate count                
                    op2 = (op2 >> (i*2)) | (op2 << (32-(i*2)));
                }
                switch ((ins & 0x01E00000) >> 21) {
                case 0x0: // AND
                    new_pc = Rn & op2;
                    break;
                case 0x1: // EOR
                    new_pc = Rn ^ op2;
                    break;
                case 0x2: // SUB
                    new_pc = Rn - op2;
                    break;
                case 0x3: // RSB
                    new_pc = op2 - Rn;
                    break;
                case 0x4: // ADD
                    new_pc = Rn + op2;
                    break;
                case 0x5: // ADC
                    c = (get_register(PS) & PS_C) != 0;
                    new_pc = Rn + op2 + c;
                    break;
                case 0x6: // SBC
                    c = (get_register(PS) & PS_C) != 0;
                    new_pc = Rn - op2 + c - 1;
                    break;
                case 0x7: // RSC
                    c = (get_register(PS) & PS_C) != 0;
                    new_pc = op2 - Rn +c - 1;
                    break;
                case 0x8: // TST
                case 0x9: // TEQ
                case 0xA: // CMP
                case 0xB: // CMN
                    break; // PC doesn't change
                case 0xC: // ORR
                    new_pc = Rn | op2;
                    break;
                case 0xD: // MOV
                    new_pc = op2;
                    break;
                case 0xE: // BIC
                    new_pc = Rn & ~op2;
                    break;
                case 0xF: // MVN
                    new_pc = ~op2;
                    break;
                }
            }
        }
        return ((unsigned long *)new_pc);
    case 0x1:
        if ((ins & 0x02000010) == 0x02000010) {
            // Undefined!
            return (pc+1);
        } else {
            if ((ins & 0x00100000) == 0) {
                // STR
                return (pc+1);
            } else {
                // LDR
                if ((ins & 0x0000F000) != 0x0000F000) {
                    // Rd not PC
                    return (pc+1);
                } else {
                    Rn = (unsigned long)get_register((ins & 0x000F0000) >> 16);
                    if ((ins & 0x000F0000) == 0x000F0000) Rn += 8;  // PC prefetch!
                    if (ins & 0x01000000) {
                        // Add/subtract offset before
                        if ((ins & 0x02000000) == 0) {
                            // Immediate offset
                            if (ins & 0x00800000) {
                                // Add offset
                                Rn += (ins & 0x00000FFF);
                            } else {
                                // Subtract offset
                                Rn -= (ins & 0x00000FFF);
                            }
                        } else {
                            // Offset is in a register
                            if (ins & 0x00800000) {
                                // Add offset
                                Rn += RmShifted(ins & 0x00000FFF);
                            } else {
                                // Subtract offset
                                Rn -= RmShifted(ins & 0x00000FFF);
                            }
                        }
                    }
                    return ((unsigned long *)*(unsigned long *)Rn);
                }
            }
        }
        return (pc+1);
    case 0x2:  // Branch, LDM/STM
        if ((ins & 0x02000000) == 0) {
            // LDM/STM
            if ((ins & 0x00100000) == 0) {
                // STM
                return (pc+1);
            } else {
                // LDM
                if ((ins & 0x00008000) == 0) {
                    // PC not in list
                    return (pc+1);
                } else {
                    Rn = (unsigned long)get_register((ins & 0x000F0000) >> 16);
                    if ((ins & 0x000F0000) == 0x000F0000) Rn += 8;  // PC prefetch!
                    offset = ins & 0x0000FFFF;
                    reg_count = 0;
                    for (i = 0;  i < 15;  i++) {
                        if (offset & (1<<i)) reg_count++;
                    }                    
                    if (ins & 0x00800000) {
                        // Add offset
                        Rn += reg_count*4;
                    } else {
                        // Subtract offset
                        Rn -= 4;
                    }
                    return ((unsigned long *)*(unsigned long *)Rn);
                }
            }
        } else {
            // Branch
            if (ins_will_execute(ins)) {
                offset = (ins & 0x00FFFFFF) << 2;
                if (ins & 0x00800000) offset |= 0xFC000000;  // sign extend
                new_pc = (unsigned long)(pc+2) + offset;
                return ((unsigned long *)new_pc);
            } else {
                // Falls through
                return (pc+1);
            }
        }
    case 0x3:  // Coprocessor & SWI
        return (pc+1);
    }
}

void __single_step (void)
{
    unsigned long *pc = (unsigned long *)get_register(PC);
    unsigned long curins = *pc;
    if (ins_will_execute(curins)) {
        // Decode instruction to decide what the next PC will be
        ss_saved_pc = target_ins(pc, curins);
    } else {
        // The current instruction will not execute (the conditions don't hold)
        ss_saved_pc = (pc+1);
    }
    // For now
    ss_saved_instr = *ss_saved_pc;
    *ss_saved_pc = 0xE7FFDEFF;  // As used by ARM
}

/* Clear the single-step state. */

void __clear_single_step (void)
{
    if (ss_saved_pc != NULL_PTR) {
        *ss_saved_pc = ss_saved_instr;
        ss_saved_pc = NULL_PTR;
    }
}

void __install_breakpoints (void)
{
//    FIXME();
}

void __clear_breakpoints (void)
{
//    FIXME();
}

/* If the breakpoint we hit is in the breakpoint() instruction, return a
   non-zero value. */

int
__is_breakpoint_function ()
{
    return get_register (PC) == (target_register_t)&CYG_LABEL_NAME(breakinst);
}


/* Skip the current instruction.  Since this is only called by the
   stub when the PC points to a breakpoint or trap instruction,
   we can safely just skip 4. */

void __skipinst (void)
{
    put_register (PC, get_register (PC) + 4);
}



#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
