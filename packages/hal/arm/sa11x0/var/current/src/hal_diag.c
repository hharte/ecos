/*=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
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
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
// All Rights Reserved.
// -------------------------------------------
//
//####COPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   gthomas
// Contributors:nickg, gthomas, dmoseley
//              Travis C. Furrer <furrer@mit.edu>
// Date:        2000-05-08
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // basic machine info
#include <cyg/hal/hal_intr.h>           // interrupt macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // Calling interface definitions
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/drv_api.h>            // cyg_drv_interrupt_acknowledge
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/hal_sa11x0.h>         // Hardware definitions

struct sa11x0_serial {
  volatile cyg_uint32 utcr0;
  volatile cyg_uint32 utcr1;
  volatile cyg_uint32 utcr2;
  volatile cyg_uint32 utcr3;
  volatile cyg_uint32 pad0010;
  volatile cyg_uint32 utdr;
  volatile cyg_uint32 pad0018;
  volatile cyg_uint32 utsr0;
  volatile cyg_uint32 utsr1;
};

//-----------------------------------------------------------------------------
typedef struct {
    volatile struct sa11x0_serial* base;
    cyg_int32 msec_timeout;
    int isr_vector;
} channel_data_t;

/*---------------------------------------------------------------------------*/
// SA11x0 Serial Port (UARTx) for Debug

static void
init_channel(channel_data_t* __ch_data)
{
    volatile struct sa11x0_serial* base = __ch_data->base;
    cyg_uint32 brd;

    // Disable Receiver and Transmitter (clears FIFOs)
    base->utcr3 = SA11X0_UART_RX_DISABLED | SA11X0_UART_TX_DISABLED;

    // Clear sticky (writable) status bits.
    base->utsr0 = SA11X0_UART_RX_IDLE | SA11X0_UART_RX_BEGIN_OF_BREAK |
                  SA11X0_UART_RX_END_OF_BREAK;

#if defined(CYGPKG_HAL_ARM_SA11X0_SA1100MM) || defined(CYGPKG_HAL_ARM_SA11X0_BRUTUS)
   // This setup is specific to only a few boards.
   if (SA11X0_UART1_BASE == (volatile unsigned long *)base) {
        cyg_uint32 pdr, afr, par;

        HAL_READ_UINT32(SA11X0_GPIO_PIN_DIRECTION, pdr);
        HAL_READ_UINT32(SA11X0_GPIO_ALTERNATE_FUNCTION, afr);
        HAL_READ_UINT32(SA11X0_PPC_PIN_ASSIGNMENT, par);

        //Set pin 14 as an output (Tx) and pin 15 as in input (Rx).
        HAL_WRITE_UINT32(SA11X0_GPIO_PIN_DIRECTION, ((pdr | SA11X0_GPIO_PIN_14) & ~SA11X0_GPIO_PIN_15));

        // Use GPIO 14 & 15 pins for serial port 1.
        HAL_WRITE_UINT32(SA11X0_GPIO_ALTERNATE_FUNCTION, afr | SA11X0_GPIO_PIN_14 | SA11X0_GPIO_PIN_15);

        // Pin reassignment for serial port 1.
        HAL_WRITE_UINT32(SA11X0_PPC_PIN_ASSIGNMENT, par | SA11X0_PPC_UART_PIN_REASSIGNMENT_MASK);
    }
#endif

    // Set UART to 8N1 (8 data bits, no partity, 1 stop bit)
    base->utcr0 = SA11X0_UART_PARITY_DISABLED | SA11X0_UART_STOP_BITS_1 |
                  SA11X0_UART_DATA_BITS_8;

    // Set the desired baud rate.
    brd = SA11X0_UART_BAUD_RATE_DIVISOR(CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD);
    base->utcr1 = (brd >> 8) & SA11X0_UART_H_BAUD_RATE_DIVISOR_MASK;
    base->utcr2 = brd & SA11X0_UART_L_BAUD_RATE_DIVISOR_MASK;

    // Enable the receiver and the transmitter.
    base->utcr3 = SA11X0_UART_RX_ENABLED | SA11X0_UART_TX_ENABLED;

    // All done
}

void
cyg_hal_plf_serial_putc(void *__ch_data, char c)
{
    volatile struct sa11x0_serial* base = ((channel_data_t*)__ch_data)->base;
    CYGARC_HAL_SAVE_GP();

    // Wait for Tx FIFO not full
    while ((base->utsr1 & SA11X0_UART_TX_FIFO_NOT_FULL) == 0)
        ;
    base->utdr = c;

    CYGARC_HAL_RESTORE_GP();
}

// FIXME: shouldn't we check for PARITY_ERROR, FRAMING_ERROR, or
// RECEIVE_FIFO_OVERRUN_ERROR in the received data?  This
// means check the appropriate bits in UTSR1.

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    volatile struct sa11x0_serial* base = ((channel_data_t*)__ch_data)->base;

    // If receive fifo is empty, return false
    if ((base->utsr1 & SA11X0_UART_RX_FIFO_NOT_EMPTY) == 0)
        return false;

    *ch = (char)base->utdr;

    // Clear receiver idle status bit, to allow another interrupt to
    // occur in the case where the receive fifo is almost empty.
    base->utsr0 = SA11X0_UART_RX_IDLE;

    return true;
}

cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}

#if defined(CYGSEM_HAL_VIRTUAL_VECTOR_DIAG) \
    || defined(CYGPRI_HAL_IMPLEMENTS_IF_SERVICES)

static channel_data_t ser_channels[2] = {
    { (volatile struct sa11x0_serial*)SA11X0_UART1_BASE, 1000, CYGNUM_HAL_INTERRUPT_UART1 },
    { (volatile struct sa11x0_serial*)SA11X0_UART3_BASE, 1000, CYGNUM_HAL_INTERRUPT_UART3 }
};

static void
cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_plf_serial_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_serial_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    delay_count = chan->msec_timeout * 10; // delay in .1 ms steps

    for(;;) {
        res = cyg_hal_plf_serial_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static int
cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    static int irq_state = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;

        chan->base->utcr3 |= SA11X0_UART_RX_FIFO_INT_ENABLED;

        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;

        chan->base->utcr3 &= ~SA11X0_UART_RX_FIFO_INT_ENABLED;

        HAL_INTERRUPT_MASK(chan->isr_vector);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->isr_vector;
        break;
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = chan->msec_timeout;
        chan->msec_timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
    }        
    default:
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    int res = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    char c;
    int reg;
    CYGARC_HAL_SAVE_GP();

    reg = chan->base->utsr1;

    // read it anyway just in case - no harm done and we might prevent an
    // interrupt loop
    c = (char)chan->base->utdr;

    //Clear receiver idle status bit, to allow another interrupt to
    //occur in the case where the receive fifo is almost empty.
    chan->base->utsr0 = SA11X0_UART_RX_IDLE;

    cyg_drv_interrupt_acknowledge(chan->isr_vector);

    *__ctrlc = 0;
    if ( (reg & SA11X0_UART_RX_FIFO_NOT_EMPTY) != 0 ) {
        if( cyg_hal_is_break( &c , 1 ) )
            *__ctrlc = 1;

        res = CYG_ISR_HANDLED;
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Init channels
    init_channel(&ser_channels[0]);
#if (CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS == 2)
    init_channel(&ser_channels[1]);
#endif

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &ser_channels[0]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);

#if (CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS == 2)
    // Set channel 1
    CYGACC_CALL_IF_SET_CONSOLE_COMM(1);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &ser_channels[1]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);
#endif
     
    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_serial_init();
}
#endif // CYGSEM_HAL_VIRTUAL_VECTOR_DIAG || CYGPRI_HAL_IMPLEMENTS_IF_SERVICES


//=============================================================================
// Compatibility with older stubs
//=============================================================================

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

#include <cyg/hal/hal_stub.h>           // cyg_hal_gdb_interrupt

#if (CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL == 0)
# define __BASE ((void*)SA11X0_UART1_BASE)
# define CYGHWR_HAL_GDB_PORT_VECTOR CYGNUM_HAL_INTERRUPT_UART1
#else
# define __BASE ((void*)SA11X0_UART3_BASE)
# define CYGHWR_HAL_GDB_PORT_VECTOR CYGNUM_HAL_INTERRUPT_UART3
#endif

#ifdef CYGSEM_HAL_ROM_MONITOR
#define CYG_HAL_STARTUP_ROM
#undef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
#endif

#if defined(CYG_HAL_STARTUP_ROM) && !defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
#define HAL_DIAG_USES_HARDWARE
#elif defined(CYGDBG_HAL_DIAG_DISABLE_GDB_PROTOCOL)
#define HAL_DIAG_USES_HARDWARE
#elif CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL != CYGNUM_HAL_VIRTUAL_VECTOR_DEBUG_CHANNEL
#define HAL_DIAG_USES_HARDWARE
#endif

static channel_data_t ser_channel = {
    (volatile struct sa11x0_serial*)__BASE, 0, CYGHWR_HAL_GDB_PORT_VECTOR
};

void 
hal_diag_init(void)
{
    // Init serial device
    init_channel(&ser_channel);
}

#ifdef HAL_DIAG_USES_HARDWARE

#ifdef DEBUG_DIAG
#ifndef CYG_HAL_STARTUP_ROM
#define DIAG_BUFSIZE 2048
static char diag_buffer[DIAG_BUFSIZE];
static int diag_bp = 0;
#endif
#endif

void hal_diag_read_char(char *c)
{
    *c = cyg_hal_plf_serial_getc(&ser_channel);
}

void hal_diag_write_char(char c)
{
#ifdef DEBUG_DIAG
#ifndef CYG_HAL_STARTUP_ROM
    diag_buffer[diag_bp++] = c;
    if (diag_bp == sizeof(diag_buffer)) diag_bp = 0;
#endif
#endif
    cyg_hal_plf_serial_putc(&ser_channel, c);
}

#else // not HAL_DIAG_USES_HARDWARE - it uses GDB protocol

void 
hal_diag_read_char(char *c)
{
    *c = cyg_hal_plf_serial_getc(&ser_channel);
}

void 
hal_diag_write_char(char c)
{
    static char line[100];
    static int pos = 0;

    // FIXME: Some LED blinking might be nice right here.

    // No need to send CRs
    if( c == '\r' ) return;

    line[pos++] = c;

    if( c == '\n' || pos == sizeof(line) )
    {
        
        CYG_INTERRUPT_STATE old;

        // Disable interrupts. This prevents GDB trying to interrupt us
        // while we are in the middle of sending a packet. The serial
        // receive interrupt will be seen when we re-enable interrupts
        // later.
        
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION(old);
#else
        HAL_DISABLE_INTERRUPTS(old);
#endif

        while(1)
        {
            static char hex[] = "0123456789ABCDEF";
            cyg_uint8 csum = 0;
            int i;
#ifndef CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT
            char c1;
#endif        
            cyg_hal_plf_serial_putc(&ser_channel, '$');
            cyg_hal_plf_serial_putc(&ser_channel, 'O');
            csum += 'O';
            for( i = 0; i < pos; i++ )
            {
                char ch = line[i];
                char h = hex[(ch>>4)&0xF];
                char l = hex[ch&0xF];
                cyg_hal_plf_serial_putc(&ser_channel, h);
                cyg_hal_plf_serial_putc(&ser_channel, l);
                csum += h;
                csum += l;
            }
            cyg_hal_plf_serial_putc(&ser_channel, '#');
            cyg_hal_plf_serial_putc(&ser_channel, hex[(csum>>4)&0xF]);
            cyg_hal_plf_serial_putc(&ser_channel, hex[csum&0xF]);

#ifdef CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT

            break; // regardless

#else // not CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT Ie. usually...

            // Wait for the ACK character '+' from GDB here and handle
            // receiving a ^C instead.  This is the reason for this clause
            // being a loop.
            c1 = cyg_hal_plf_serial_getc(&ser_channel);

            if( c1 == '+' )
                break;              // a good acknowledge

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
            cyg_drv_interrupt_acknowledge(CYGHWR_HAL_GDB_PORT_VECTOR);
            if( c1 == 3 ) {
                // Ctrl-C: breakpoint.
                cyg_hal_gdb_interrupt(
                    (target_register_t)__builtin_return_address(0) );
                break;
            }
#endif // CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT

#endif // ! CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT
            // otherwise, loop round again
        }
        
        pos = 0;

        
        // And re-enable interrupts
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        CYG_HAL_GDB_LEAVE_CRITICAL_IO_REGION(old);
#else
        HAL_RESTORE_INTERRUPTS(old);
#endif
        
    }
}
#endif

#endif // !CYGSEM_HAL_VIRTUAL_VECTOR_DIAG


/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
