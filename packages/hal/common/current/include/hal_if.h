#ifndef CYGONCE_HAL_HAL_IF_H
#define CYGONCE_HAL_HAL_IF_H

//=============================================================================
//
//      hal_if.h
//
//      HAL header for ROM/RAM calling interface.
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jskov
// Contributors:jskov
// Date:        2000-06-07
// Purpose:     HAL RAM/ROM calling interface
// Description: ROM/RAM calling interface table	definitions. The layout is a
//              combination of libbsp and vectors already in use by some
//              eCos platforms.
// Usage:       #include <cyg/hal/hal_if.h>
//                           
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/infra/cyg_type.h>         // types & externC
#include <cyg/hal/dbg-threads-api.h>
#include <cyg/hal/dbg-thread-syscall.h>

#include <stdarg.h>

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT

// Architecture/var/platform may override the accessor macros.
#include <cyg/hal/hal_arch.h>

//--------------------------------------------------------------------------
#ifndef _BSP_HANDLER_T_DEFINED
#define _BSP_HANDLER_T_DEFINED
typedef int (*bsp_handler_t)(int __irq_nr, void *__regs);
#endif // _BSP_HANDLER_T_DEFINED

//--------------------------------------------------------------------------
// Communication interface table. CYGNUM_CALL_IF_CONSOLE_PROCS and
// CYGNUM_CALL_IF_DEBUG_PROCS point to instances (possibly the same)
// of this table.

typedef enum {
    /*
     * For serial ports, the control function may be used to set and get the
     * current baud rate. Usage:
     * 
     *   err = (*__control)(COMMCTL_SETBAUD, int bits_per_second);
     *     err => Zero if successful, -1 if error.
     *
     *   baud = (*__control)(COMMCTL_GETBAUD);
     *     baud => -1 if error, current baud otherwise.
     */
    __COMMCTL_SETBAUD=0,
    __COMMCTL_GETBAUD,

    /*
     * Install and remove debugger interrupt handlers. These are the receiver
     * interrupt routines which are used to change control from a running
     * program to the debugger stub.
     */
    __COMMCTL_INSTALL_DBG_ISR,
    __COMMCTL_REMOVE_DBG_ISR,

    /*
     * Disable comm port interrupt. Returns TRUE if interrupt was enabled,
     * FALSE otherwise.
     */
    __COMMCTL_IRQ_DISABLE,

    /*
     * Enable comm port interrupt.
     */
    __COMMCTL_IRQ_ENABLE,

    /*
     * Returns the number of the interrupt vector used by the debug
     * interrupt handler.
     */
    __COMMCTL_DBG_ISR_VECTOR,

    /*
     * Returns the current timeout value and sets a new timeout.
     * Timeout resolution is in milliseconds.
     *   old_timeout = (*__control)(__COMMCTL_SET_TIMEOUT, 
     *                              cyg_int32 new_timeout);
     */
    __COMMCTL_SET_TIMEOUT,

    /*
     * Forces driver to send all characters which may be buffered in
     * the driver. This only flushes the driver buffers, not necessarily
     * any hardware FIFO, etc.
     */
    __COMMCTL_FLUSH_OUTPUT,

} __comm_control_cmd_t;


#define CYGNUM_COMM_IF_CH_DATA                    0
#define CYGNUM_COMM_IF_WRITE                      1
#define CYGNUM_COMM_IF_READ                       2
#define CYGNUM_COMM_IF_PUTC                       3
#define CYGNUM_COMM_IF_GETC                       4
#define CYGNUM_COMM_IF_CONTROL                    5
#define CYGNUM_COMM_IF_DBG_ISR                    6
#define CYGNUM_COMM_IF_GETC_TIMEOUT               7

#define CYGNUM_COMM_IF_TABLE_SIZE                 8

typedef volatile CYG_ADDRWORD hal_virtual_comm_table_t[CYGNUM_COMM_IF_TABLE_SIZE];

// The below is a (messy) attempt at adding some type safety to the
// above array. At the same time, the accessors allow the
// implementation to be easily changed in the future (both tag->data
// table and structure implementations have been suggested).

typedef void* __comm_if_ch_data_t;
typedef void (*__comm_if_write_t)(void* __ch_data, const cyg_uint8* __buf,
                                  cyg_uint32 __len);
typedef int (*__comm_if_read_t)(void* __ch_data, cyg_uint8* __buf,
                                cyg_uint32 __len);
typedef void (*__comm_if_putc_t)(void* __ch_data, cyg_uint8 __ch);
typedef cyg_uint8 (*__comm_if_getc_t)(void* __ch_data);
typedef int (*__comm_if_control_t)(void *__ch_data, 
                                   __comm_control_cmd_t __func, ...);
typedef int (*__comm_if_dbg_isr_t)(void *__ch_data, 
                               int* __ctrlc, CYG_ADDRWORD __vector,
                               CYG_ADDRWORD __data);
typedef cyg_bool (*__comm_if_getc_timeout_t)(void* __ch_data, cyg_uint8* __ch);

#ifndef CYGACC_COMM_IF_DEFINED
#define CYGACC_COMM_IF_CH_DATA(_t_) \
 ((__comm_if_ch_data_t)((_t_)[CYGNUM_COMM_IF_CH_DATA]))
#define CYGACC_COMM_IF_CH_DATA_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_CH_DATA]=(CYG_ADDRWORD)(_x_)

#define CYGACC_COMM_IF_WRITE(_t_, _b_, _l_) \
 ((__comm_if_write_t)((_t_)[CYGNUM_COMM_IF_WRITE]))(CYGACC_COMM_IF_CH_DATA(_t_), (_b_), (_l_))
#define CYGACC_COMM_IF_WRITE_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_WRITE]=(CYG_ADDRWORD)(_x_)

#define CYGACC_COMM_IF_READ(_t_, _b_, _l_) \
 ((__comm_if_read_t)((_t_)[CYGNUM_COMM_IF_READ]))(CYGACC_COMM_IF_CH_DATA(_t_), (_b_), (_l_))
#define CYGACC_COMM_IF_READ_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_READ]=(CYG_ADDRWORD)(_x_)

#define CYGACC_COMM_IF_PUTC(_t_, _c_) \
 ((__comm_if_putc_t)((_t_)[CYGNUM_COMM_IF_PUTC]))(CYGACC_COMM_IF_CH_DATA(_t_), (_c_))
#define CYGACC_COMM_IF_PUTC_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_PUTC]=(CYG_ADDRWORD)(_x_)

#define CYGACC_COMM_IF_GETC(_t_) \
 ((__comm_if_getc_t)((_t_)[CYGNUM_COMM_IF_GETC]))(CYGACC_COMM_IF_CH_DATA(_t_))
#define CYGACC_COMM_IF_GETC_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_GETC]=(CYG_ADDRWORD)(_x_)

#define CYGACC_COMM_IF_CONTROL(_t_, args...) \
 ((__comm_if_control_t)((_t_)[CYGNUM_COMM_IF_CONTROL]))(CYGACC_COMM_IF_CH_DATA(_t_), args)
#define CYGACC_COMM_IF_CONTROL_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_CONTROL]=(CYG_ADDRWORD)(_x_)

#define CYGACC_COMM_IF_DBG_ISR(_t_, _c_, _v_, _d_) \
 ((__comm_if_dbg_isr_t)((_t_)[CYGNUM_COMM_IF_DBG_ISR]))(CYGACC_COMM_IF_CH_DATA(_t_), (_c_), (_v_), (_d_))
#define CYGACC_COMM_IF_DBG_ISR_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_DBG_ISR]=(CYG_ADDRWORD)(_x_)

#define CYGACC_COMM_IF_GETC_TIMEOUT(_t_, _c_) \
 ((__comm_if_getc_timeout_t)((_t_)[CYGNUM_COMM_IF_GETC_TIMEOUT]))(CYGACC_COMM_IF_CH_DATA(_t_), (_c_))
#define CYGACC_COMM_IF_GETC_TIMEOUT_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_GETC_TIMEOUT]=(CYG_ADDRWORD)(_x_)

#endif // CYGACC_COMM_IF_DEFINED

//--------------------------------------------------------------------------
// Main calling interface table. Will be assigned a location by the 
// linker script. Both ROM and RAM startup applications will know about
// the location.
#define CYGNUM_CALL_IF_VERSION                    0
#define CYGNUM_CALL_IF_ICTRL_TABLE                1
#define CYGNUM_CALL_IF_EXC_TABLE                  2
#define CYGNUM_CALL_IF_DBG_VECTOR                 3
#define CYGNUM_CALL_IF_KILL_VECTOR                4
#define CYGNUM_CALL_IF_CONSOLE_PROCS              5
#define CYGNUM_CALL_IF_DEBUG_PROCS                6
#define CYGNUM_CALL_IF_FLUSH_DCACHE               7
#define CYGNUM_CALL_IF_FLUSH_ICACHE               8
#define CYGNUM_CALL_IF_CPU_DATA                   9
#define CYGNUM_CALL_IF_BOARD_DATA                 10
#define CYGNUM_CALL_IF_SYSINFO                    11
#define CYGNUM_CALL_IF_SET_DEBUG_COMM             12
#define CYGNUM_CALL_IF_SET_CONSOLE_COMM           13
#define CYGNUM_CALL_IF_SET_SERIAL_BAUD            14
#define CYGNUM_CALL_IF_DBG_SYSCALL                15
#define CYGNUM_CALL_IF_RESET                      16
#define CYGNUM_CALL_IF_CONSOLE_INTERRUPT_FLAG     17
#define CYGNUM_CALL_IF_DELAY_US                   18
#define CYGNUM_CALL_IF_DBG_DATA                   19

#define CYGNUM_CALL_IF_LAST_ENTRY                 CYGNUM_CALL_IF_DBG_DATA

#define CYGNUM_CALL_IF_INSTALL_BPT_FN             35

#define CYGNUM_CALL_IF_TABLE_SIZE                 64

externC volatile CYG_ADDRWORD hal_virtual_vector_table[CYGNUM_CALL_IF_TABLE_SIZE];

// Table version contains version information for both the CALL table
// itself (the number of the last active entry in the table), and the
// COMM table (the size of the table).
#define CYGNUM_CALL_IF_TABLE_VERSION_CALL         CYGNUM_CALL_IF_LAST_ENTRY
#define CYGNUM_CALL_IF_TABLE_VERSION_CALL_HACK    (CYGNUM_CALL_IF_TABLE_SIZE+1)
#define CYGNUM_CALL_IF_TABLE_VERSION_CALL_MAX     CYGNUM_CALL_IF_TABLE_SIZE
#define CYGNUM_CALL_IF_TABLE_VERSION_COMM         CYGNUM_COMM_IF_TABLE_SIZE
#define CYGNUM_CALL_IF_TABLE_VERSION_CALL_MASK    0x0000ffff
#define CYGNUM_CALL_IF_TABLE_VERSION_COMM_MASK    0xffff0000
#define CYGNUM_CALL_IF_TABLE_VERSION_COMM_shift   16


// These are special debug/console procs IDs
// QUERY_CURRENT will cause the ID of the currently selected proc ID to be
//               returned.
// EMPTY         this is the ID used for an empty procs table (i.e, NULL
//               pointer)
// MANGLER       selects the procs space reserved for the console mangler
//               allowing the application to temporarily disable mangling
//               or temporarily switch in different console procs.
#define CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT -1
#define CYGNUM_CALL_IF_SET_COMM_ID_EMPTY         -2
#define CYGNUM_CALL_IF_SET_COMM_ID_MANGLER       -3

// The below is a (messy) attempt at adding some type safety to the
// above array. At the same time, the accessors allow the
// implementation to be easily changed in the future (both tag->data
// table and structure implementations have been suggested).

typedef int __call_if_version_t;
typedef void* __call_if_ictrl_table_t;
typedef void* __call_if_exc_table_t;
typedef bsp_handler_t *__call_if_dbg_vector_t;
typedef bsp_handler_t __call_if_kill_vector_t;
typedef hal_virtual_comm_table_t *__call_if_console_procs_t;
typedef hal_virtual_comm_table_t *__call_if_debug_procs_t;
typedef void (*__call_if_flush_dcache_t)(void *__p, int __nbytes);
typedef void (*__call_if_flush_icache_t)(void *__p, int __nbytes);
typedef void* __call_if_cpu_data_t;
typedef void* __call_if_board_data_t;
typedef int (*__call_if_sysinfo_t)(int __id, void* __ap);
//typedef int (*__call_if_sysinfo_t)(enum bsp_info_id __id, va_list __ap);
typedef int (*__call_if_set_debug_comm_t)(int __comm_id);
typedef int (*__call_if_set_console_comm_t)(int __comm_id);
typedef int (*__call_if_set_serial_baud_t)(int __comm_id, int __baud);
typedef void* __call_if_dbg_data_t;
typedef int (*__call_if_dbg_syscall_t) (enum dbg_syscall_ids id,
                                        union dbg_thread_syscall_parms  *p );
typedef void (*__call_if_reset_t)(void);
typedef int __call_if_console_interrupt_flag_t;
typedef void (*__call_if_delay_us_t)(cyg_int32 usecs);
typedef void (*__call_if_install_bpt_fn_t)(void *__epc);

#ifndef CYGACC_CALL_IF_DEFINED

#define CYGACC_CALL_IF_VERSION() \
 ((__call_if_version_t)hal_virtual_vector_table[CYGNUM_CALL_IF_VERSION])
#define CYGACC_CALL_IF_VERSION_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_VERSION]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_ICTRL_TABLE() \
 ((__call_if_ictrl_table_t)hal_virtual_vector_table[CYGNUM_CALL_IF_ICTRL_TABLE])
#define CYGACC_CALL_IF_ICTRL_TABLE_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_ICTRL_TABLE]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_EXC_TABLE() \
 ((__call_if_exc_table_t)hal_virtual_vector_table[CYGNUM_CALL_IF_EXC_TABLE])
#define CYGACC_CALL_IF_EXC_TABLE_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_EXC_TABLE]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_DBG_VECTOR() \
 ((__call_if_dbg_vector_t)hal_virtual_vector_table[CYGNUM_CALL_IF_DBG_VECTOR])
#define CYGACC_CALL_IF_DBG_VECTOR_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_DBG_VECTOR]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_KILL_VECTOR() \
 ((__call_if_kill_vector_t)hal_virtual_vector_table[CYGNUM_CALL_IF_KILL_VECTOR])
#define CYGACC_CALL_IF_KILL_VECTOR_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_KILL_VECTOR]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_CONSOLE_PROCS() \
 ((__call_if_console_procs_t)hal_virtual_vector_table[CYGNUM_CALL_IF_CONSOLE_PROCS])
#define CYGACC_CALL_IF_CONSOLE_PROCS_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_CONSOLE_PROCS]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_DEBUG_PROCS() \
 ((__call_if_debug_procs_t)hal_virtual_vector_table[CYGNUM_CALL_IF_DEBUG_PROCS])
#define CYGACC_CALL_IF_DEBUG_PROCS_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_DEBUG_PROCS]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_FLUSH_DCACHE(_p_, _n_) \
 ((__call_if_flush_dcache_t)hal_virtual_vector_table[CYGNUM_CALL_IF_FLUSH_DCACHE])((_p_), (_n_))
#define CYGACC_CALL_IF_FLUSH_DCACHE_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_FLUSH_DCACHE]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_FLUSH_ICACHE(_p_, _n_) \
 ((__call_if_flush_icache_t)hal_virtual_vector_table[CYGNUM_CALL_IF_FLUSH_ICACHE])((_p_), (_n_))
#define CYGACC_CALL_IF_FLUSH_ICACHE_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_FLUSH_ICACHE]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_CPU_DATA() \
 ((__call_if_cpu_data_t)hal_virtual_vector_table[CYGNUM_CALL_IF_CPU_DATA])
#define CYGACC_CALL_IF_CPU_DATA_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_CPU_DATA]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_BOARD_DATA() \
 ((__call_if_board_data_t)hal_virtual_vector_table[CYGNUM_CALL_IF_BOARD_DATA])
#define CYGACC_CALL_IF_BOARD_DATA_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_BOARD_DATA]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_SYSINFO(_i_, _a_) \
 ((__call_if_sysinfo_t)hal_virtual_vector_table[CYGNUM_CALL_IF_SYSINFO])((_i_), (_a_))
#define CYGACC_CALL_IF_SYSINFO_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_SYSINFO]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_SET_DEBUG_COMM(_i_) \
 ((__call_if_set_debug_comm_t)hal_virtual_vector_table[CYGNUM_CALL_IF_SET_DEBUG_COMM])((_i_))
#define CYGACC_CALL_IF_SET_DEBUG_COMM_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_SET_DEBUG_COMM]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_SET_CONSOLE_COMM(_i_) \
 ((__call_if_set_console_comm_t)hal_virtual_vector_table[CYGNUM_CALL_IF_SET_CONSOLE_COMM])((_i_))
#define CYGACC_CALL_IF_SET_CONSOLE_COMM_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_SET_CONSOLE_COMM]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_SET_SERIAL_BAUD(_i_, _b_) \
 ((__call_if_set_serial_baud_t)hal_virtual_vector_table[CYGNUM_CALL_IF_SET_SERIAL_BAUD])((_i_), (_b_))
#define CYGACC_CALL_IF_SET_SERIAL_BAUD_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_SET_SERIAL_BAUD]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_DBG_DATA() \
 ((__call_if_dbg_data_t)hal_virtual_vector_table[CYGNUM_CALL_IF_DBG_DATA])
#define CYGACC_CALL_IF_DBG_DATA_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_DBG_DATA]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_DBG_SYSCALL() \
 ((__call_if_dbg_syscall_t)hal_virtual_vector_table[CYGNUM_CALL_IF_DBG_SYSCALL])
#define CYGACC_CALL_IF_DBG_SYSCALL_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_DBG_SYSCALL]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_RESET() \
 ((__call_if_reset_t)hal_virtual_vector_table[CYGNUM_CALL_IF_RESET])()
#define CYGACC_CALL_IF_RESET_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_RESET]=(CYG_ADDRWORD)(_x_)
#define CYGACC_CALL_IF_RESET_GET() \
 ((__call_if_reset_t)hal_virtual_vector_table[CYGNUM_CALL_IF_RESET])

#define CYGACC_CALL_IF_CONSOLE_INTERRUPT_FLAG() \
 ((__call_if_console_interrupt_flag_t)hal_virtual_vector_table[CYGNUM_CALL_IF_CONSOLE_INTERRUPT_FLAG])
#define CYGACC_CALL_IF_CONSOLE_INTERRUPT_FLAG_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_CONSOLE_INTERRUPT_FLAG]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_DELAY_US(_u_) \
 ((__call_if_delay_us_t)hal_virtual_vector_table[CYGNUM_CALL_IF_DELAY_US])((_u_))
#define CYGACC_CALL_IF_DELAY_US_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_DELAY_US]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_INSTALL_BPT_FN(_e_) \
 ((__call_if_install_bpt_fn_t)hal_virtual_vector_table[CYGNUM_CALL_IF_INSTALL_BPT_FN])((_e_))
#define CYGACC_CALL_IF_INSTALL_BPT_FN_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_INSTALL_BPT_FN]=(CYG_ADDRWORD)(_x_)

#endif // CYGACC_CALL_IF_DEFINED

//--------------------------------------------------------------------------
// Diag wrappers.
externC void hal_if_diag_init(void);
externC void hal_if_diag_write_char(char c);
externC void hal_if_diag_read_char(char *c);

//--------------------------------------------------------------------------
// Ctrl-c support.
externC cyg_uint32 hal_ctrlc_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);
externC cyg_bool   hal_ctrlc_check(CYG_ADDRWORD vector, CYG_ADDRWORD data);

#define HAL_CTRLC_ISR hal_ctrlc_isr
#define HAL_CTRLC_CHECK hal_ctrlc_check

#else

#if defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)
// Then other code might invoke this macro
#define HAL_CTRLC_CHECK(a1,a2) (0) // Nothing, no CTRLC here
#endif

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT

//--------------------------------------------------------------------------
// Functions provided by the HAL interface.
externC void hal_if_init(void);

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_HAL_IF_H
// End of hal_if.h
