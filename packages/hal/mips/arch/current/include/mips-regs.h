/* mips-regs.h - register defines for MIPS processors
 * 
 * Copyright (c) 1998 Cygnus Solutions
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

#include <pkgconf/hal.h>

#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

/* This value must agree with NUMREGS in mips-stub.h. */

#define NUM_REGS 90

#ifdef __mips64
  #define REG_SIZE 8
#else
  #define REG_SIZE 4
#endif

/* General register names for assembly code. */

#define zero            $0
#define at              $1              /* assembler temporary */
#define v0              $2              /* value holders */
#define v1              $3
#define a0              $4              /* arguments */
#define a1              $5
#define a2              $6
#define a3              $7
#define t0              $8              /* temporaries */
#define t1              $9
#define t2              $10
#define t3              $11
#define t4              $12
#define t5              $13
#define t6              $14
#define t7              $15
#define s0              $16             /* saved registers */
#define s1              $17
#define s2              $18
#define s3              $19
#define s4              $20
#define s5              $21
#define s6              $22
#define s7              $23
#define t8              $24             /* temporaries */
#define t9              $25
#define k0              $26             /* kernel registers */
#define k1              $27
#define gp              $28             /* global pointer */
#define sp              $29             /* stack pointer */
#define s8              $30             /* saved register */
#define fp              $30             /* frame pointer (obsolete usage) */
#define ra              $31             /* return address */

/* MIPS registers, numbered in the order in which gdb expects to see them. */
#define ZERO            0
#define AT              1
#define V0              2
#define V1              3
#define A0              4
#define A1              5
#define A2              6
#define A3              7

#define T0              8
#define T1              9
#define T2              10
#define T3              11
#define T4              12
#define T5              13
#define T6              14
#define T7              15

#define S0              16
#define S1              17
#define S2              18
#define S3              19
#define S4              20
#define S5              21
#define S6              22
#define S7              23

#define T8              24
#define T9              25
#define K0              26
#define K1              27
#define GP              28
#define SP              29
#define S8              30
#define RA              31

#define SR              32
#define LO              33
#define HI              34
#define BAD_VA          35
#define CAUSE           36
#define PC              37

/* System Control Coprocessor (CP0) exception processing registers */
#define C0_CONTEXT      $4              /* Context */
#define C0_BADVADDR     $8              /* Bad Virtual Address */
#define C0_COUNT        $9              /* Count */
#define C0_COMPARE      $11             /* Compare */
#define C0_STATUS       $12             /* Processor Status */
#define C0_CAUSE        $13             /* Exception Cause */
#define C0_EPC          $14             /* Exception PC */
#define C0_XCONTEXT     $20             /* XContext */
#define C0_ECC          $26             /* ECC */
#define C0_CACHEERR     $27             /* CacheErr */
#define C0_ERROREPC     $30             /* ErrorEPC */

/* Status register fields */
#define SR_CUMASK       0xf0000000      /* Coprocessor usable bits */
#define SR_CU3          0x80000000      /* Coprocessor 3 usable */
#define SR_CU2          0x40000000      /* coprocessor 2 usable */
#define SR_CU1          0x20000000      /* Coprocessor 1 usable */
#define SR_CU0          0x10000000      /* Coprocessor 0 usable */

#define SR_FR           0x04000000      /* Enable 32 floating-point registers */
#define SR_RE           0x02000000      /* Reverse Endian in user mode */

#define SR_BEV          0x00400000      /* Bootstrap Exception Vector */
#define SR_TS           0x00200000      /* TLB shutdown (reserved on R4600) */
#define SR_SR           0x00100000      /* Soft Reset */

#define SR_CH           0x00040000      /* Cache Hit */
#define SR_CE           0x00020000      /* ECC register modifies check bits */
#define SR_DE           0x00010000      /* Disable cache errors */

#define SR_IMASK        0x0000ff00      /* Interrupt Mask */
#define SR_IMASK8       0x00000000      /* Interrupt Mask level=8 */
#define SR_IMASK7       0x00008000      /* Interrupt Mask level=7 */
#define SR_IMASK6       0x0000c000      /* Interrupt Mask level=6 */
#define SR_IMASK5       0x0000e000      /* Interrupt Mask level=5 */
#define SR_IMASK4       0x0000f000      /* Interrupt Mask level=4 */
#define SR_IMASK3       0x0000f800      /* Interrupt Mask level=3 */
#define SR_IMASK2       0x0000fc00      /* Interrupt Mask level=2 */
#define SR_IMASK1       0x0000fe00      /* Interrupt Mask level=1 */
#define SR_IMASK0       0x0000ff00      /* Interrupt Mask level=0 */

#define SR_IBIT8        0x00008000      /*  (Intr5) */
#define SR_IBIT7        0x00004000      /*  (Intr4) */
#define SR_IBIT6        0x00002000      /*  (Intr3) */
#define SR_IBIT5        0x00001000      /*  (Intr2) */
#define SR_IBIT4        0x00000800      /*  (Intr1) */
#define SR_IBIT3        0x00000400      /*  (Intr0) */
#define SR_IBIT2        0x00000200      /*  (Software Interrupt 1) */
#define SR_IBIT1        0x00000100      /*  (Software Interrupt 0) */

#define SR_KX           0x00000080      /* xtlb in kernel mode */
#define SR_SX           0x00000040      /* mips3 & xtlb in supervisor mode */
#define SR_UX           0x00000020      /* mips3 & xtlb in user mode */

#define SR_KSU_MASK     0x00000018      /* ksu mode mask */
#define SR_KSU_USER     0x00000010      /* user mode */
#define SR_KSU_SUPV     0x00000008      /* supervisor mode */
#define SR_KSU_KERN     0x00000000      /* kernel mode */

#define SR_ERL          0x00000004      /* error level */
#define SR_EXL          0x00000002      /* exception level */
#define SR_IE           0x00000001      /* interrupt enable */

/* Cause register fields */
#define CAUSE_BD        0x80000000      /* Branch Delay */
#define CAUSE_CEMASK    0x30000000      /* Coprocessor Error */
#define CAUSE_CESHIFT   28              /* Right justify CE  */
#define CAUSE_IPMASK    0x0000ff00      /* Interrupt Pending */
#define CAUSE_IPSHIFT   8               /* Right justify IP  */
#define CAUSE_IP8       0x00008000      /*  (Intr5) */
#define CAUSE_IP7       0x00004000      /*  (Intr4) */
#define CAUSE_IP6       0x00002000      /*  (Intr3) */
#define CAUSE_IP5       0x00001000      /*  (Intr2) */
#define CAUSE_IP4       0x00000800      /*  (Intr1) */
#define CAUSE_IP3       0x00000400      /*  (Intr0) */
#define CAUSE_SW2       0x00000200      /*  (Software Interrupt 1) */
#define CAUSE_SW1       0x00000100      /*  (Software Interrupt 0) */
#define CAUSE_EXCMASK   0x0000007c      /* Exception Code */
#define CAUSE_EXCSHIFT  2               /* Right justify EXC */

/* Exception Codes */
#define EXC_INT         0               /* External interrupt */
#define EXC_MOD         1               /* TLB modification exception */
#define EXC_TLBL        2               /* TLB miss (Load or Ifetch) */
#define EXC_TLBS        3               /* TLB miss (Store) */
#define EXC_ADEL        4               /* Address error (Load or Ifetch) */
#define EXC_ADES        5               /* Address error (Store) */
#define EXC_IBE         6               /* Bus error (Ifetch) */
#define EXC_DBE         7               /* Bus error (data load or store) */
#define EXC_SYS         8               /* System call */
#define EXC_BP          9               /* Break point */
#define EXC_RI          10              /* Reserved instruction */
#define EXC_CPU         11              /* Coprocessor unusable */
#define EXC_OVF         12              /* Arithmetic overflow */
#define EXC_TRAP        13              /* Trap exception */
#define EXC_FPE         15              /* Floating Point Exception */

#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
