/*=============================================================================
//
//	hal_diag.c
//
//	HAL diagnostic output code
//
//=============================================================================
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
// by Cygnus are Copyright (C) 1998 Cygnus Solutions.  All Rights Reserved.
// -------------------------------------------
//
//####COPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	nickg
// Contributors:	nickg
// Date:	1998-03-02
// Purpose:     HAL diagnostic output
// Description:	Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>          // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>

/*---------------------------------------------------------------------------*/
/* Select default diag channel to use                                        */

//#define CYG_KERNEL_DIAG_ROMART
//#define CYG_KERNEL_DIAG_SERIAL1
//#define CYG_KERNEL_DIAG_SERIAL2
//#define CYG_KERNEL_DIAG_GDB

#if !defined(CYG_KERNEL_DIAG_SERIAL1) && \
    !defined(CYG_KERNEL_DIAG_SERIAL2) && \
    !defined(CYG_KERNEL_DIAG_ROMART)

# if defined(CYG_HAL_MN10300_STDEVAL1)

#  if defined(CYG_HAL_STARTUP_RAM)

#   if defined(CYG_HAL_USE_ROM_MONITOR_CYGMON)

     // If loaded into RAM via CYGMON we diag via
     // serial 2 using GDB protocol
    
#    define CYG_KERNEL_DIAG_SERIAL2
#    define CYG_KERNEL_DIAG_GDB

#   elif defined(CYG_HAL_USE_ROM_MONITOR_SLOAD)

     // If loaded into RAM via SLOAD we diag via
     // serial 1
    
#    define CYG_KERNEL_DIAG_SERIAL1
    
#   elif defined(CYG_HAL_USE_ROM_MONITOR_GDB_STUBS)

     // If loaded into RAM via GDB STUB ROM we diag via
     // serial 1 using GDB protocol
    
#    define CYG_KERNEL_DIAG_SERIAL1
#    define CYG_KERNEL_DIAG_GDB
    
#   endif // defined(CYG_HAL_USE_ROM_MONITOR_CYGMON)
    
#  elif defined(CYG_HAL_STARTUP_ROM)

    // If we are ROM resident, we diag via serial 1
    
#  define CYG_KERNEL_DIAG_SERIAL1
    
#  endif // defined(CYG_HAL_STARTUP_RAM)
    
# endif // defined(CYG_HAL_MN10300_STDEVAL1)

#endif // if ...
    
/*---------------------------------------------------------------------------*/
// PromICE AI interface

#if defined(CYG_HAL_MN10300_STDEVAL1_ROMART) || defined(CYG_KERNEL_DIAG_ROMART)

#ifdef CYG_HAL_MN10300_STDEVAL1
#define PROMICE_AILOC           0x40008000
#endif

#define PROMICE_BUS_SIZE        16
#define PROMICE_BURST_SIZE      1

#if PROMICE_BUS_SIZE == 16

typedef volatile struct
{
    volatile cyg_uint16 zero;
//    cyg_uint16          pad1[PROMICE_BURST_SIZE];
    volatile cyg_uint16 one;
//    cyg_uint16          pad2[PROMICE_BURST_SIZE];
    volatile cyg_uint16 data;
//    cyg_uint16          pad3[PROMICE_BURST_SIZE];
    volatile cyg_uint16 status;

} AISTRUCT;

#endif

AISTRUCT *AI = (AISTRUCT *)PROMICE_AILOC;

#define PROMICE_STATUS_TDA      0x01
#define PROMICE_STATUS_HDA      0x02
#define PROMICE_STATUS_OVR      0x04


void ai_diag_init()
{
    volatile cyg_uint8 junk;
 
    while( AI->status == 0xCC )
        continue;

    junk = AI->data;
}

static void ai_write_char(cyg_uint8 data)
{
    volatile cyg_uint8 junk;
    int i;
    
    // Wait for tda == 0
    while( (AI->status & PROMICE_STATUS_TDA) == PROMICE_STATUS_TDA )
        continue;

    // Send start bit
    junk = AI->one;

    for( i = 0; i < 8; i++ )
    {
        // send ls bit of data
        if( (data & 1) == 1 )
            junk = AI->one;
        else
            junk = AI->zero;

        // shift down for next bit
        data >>= 1;
    }

    // Send stop bit
    junk = AI->one;

    // all done
}

void ai_diag_write_char(char c)
{
    ai_write_char((cyg_uint8)c);
}

void ai_diag_drain() {}

void ai_diag_read_char(char *c) { *c = '\n'; }

void ai_writes(char *s)
{
    while( *s ) ai_write_char( *s++ );
}

void ai_write_hex( cyg_uint32 x)
{
    int i;
    ai_writes("0x");
    for( i = 28; i >=0 ; i-=4 )
    {
        char *d = "0123456789ABCDEF";
        ai_write_char( d[(x>>i)&0xf] );
    }
    ai_write_char(' ');
}

#if defined(CYG_KERNEL_DIAG_ROMART)

#define hal_diag_init_serial ai_diag_init
#define hal_diag_write_char_serial ai_diag_write_char
#define hal_diag_drain_serial ai_diag_drain
#define hal_diag_read_char_serial ai_diag_read_char

#endif

#endif

/*---------------------------------------------------------------------------*/
// MN10300 Serial line

#if defined(CYG_HAL_MN10300_STDEVAL1_SERIAL1) || defined(CYG_KERNEL_DIAG_SERIAL1)

// We use serial1 on MN103002
#define SERIAL1_CR       ((volatile cyg_uint16 *)0x34000810)
#define SERIAL1_ICR      ((volatile cyg_uint8 *) 0x34000814)
#define SERIAL1_TXR      ((volatile cyg_uint8 *) 0x34000818)
#define SERIAL1_RXR      ((volatile cyg_uint8 *) 0x34000819)
#define SERIAL1_SR       ((volatile cyg_uint16 *)0x3400081c)

// Timer 1 provided baud rate divisor
#define TIMER1_MD       ((volatile cyg_uint8 *)0x34001001)
#define TIMER1_BR       ((volatile cyg_uint8 *)0x34001011)
#define TIMER1_CR       ((volatile cyg_uint8 *)0x34001021)

#define PORT3_MD        ((volatile cyg_uint8 *)0x36008025)

// Mystery register
#define TMPSCNT         ((volatile cyg_uint8 *)0x34001071)

#define SIO1_LSTAT_TRDY  0x20
#define SIO1_LSTAT_RRDY  0x10

void hal_diag_init_serial1(void)
{
    // 48 translates to 38400 baud.
    *TIMER1_BR = 48;

    // Timer1 sourced from IOCLK
    *TIMER1_MD = 0x80;    

    // Mode on PORT3, used for serial line controls.
    *PORT3_MD = 0x01;

    // No interrupts for now.
    *SERIAL1_ICR = 0x00;

    // Source from timer 1, 8bit chars, enable tx and rx
    *SERIAL1_CR = 0xc084;    
}

void hal_diag_write_char_serial1(char c)
{
    register volatile cyg_uint16 *volatile tty_status = SERIAL1_SR;
    register volatile cyg_uint8 *volatile tty_tx = SERIAL1_TXR;

    while( (*tty_status & SIO1_LSTAT_TRDY) != 0 ) continue;

    *tty_tx = c;
}

void hal_diag_drain_serial1(void)
{
    register volatile cyg_uint16 *volatile tty_status = SERIAL1_SR;

    while( (*tty_status & SIO1_LSTAT_TRDY) != 0 ) continue;
}

void hal_diag_read_char_serial1(char *c)
{
    register volatile cyg_uint16 *volatile tty_status = SERIAL1_SR;
    register volatile cyg_uint8 *volatile tty_rx = SERIAL1_RXR;

    while( (*tty_status & SIO1_LSTAT_RRDY) == 0 ) continue;

    *c = *tty_rx;

}

#if defined(CYG_KERNEL_DIAG_SERIAL1)

#define hal_diag_init_serial hal_diag_init_serial1
#define hal_diag_write_char_serial hal_diag_write_char_serial1
#define hal_diag_drain_serial hal_diag_drain_serial1
#define hal_diag_read_char_serial hal_diag_read_char_serial1

#endif

#endif

#if defined(CYG_HAL_MN10300_STDEVAL1)

void led(int x)
{
    *((cyg_uint8 *)0x36008004) = x<<4;
}

#endif


/*---------------------------------------------------------------------------*/

#if defined(CYG_HAL_MN10300_STDEVAL1_SERIAL2) || defined(CYG_KERNEL_DIAG_SERIAL2)

// We use serial2 on MN103002
#define SERIAL2_CR       ((volatile cyg_uint16 *)0x34000820)
#define SERIAL2_ICR      ((volatile cyg_uint8 *) 0x34000824)
#define SERIAL2_TXR      ((volatile cyg_uint8 *) 0x34000828)
#define SERIAL2_RXR      ((volatile cyg_uint8 *) 0x34000829)
#define SERIAL2_SR       ((volatile cyg_uint8 *)0x3400082c)
#define SERIAL2_TR       ((volatile cyg_uint8 *)0x3400082d)

// Timer 2 provided baud rate divisor
#define TIMER2_MD       ((volatile cyg_uint8 *)0x34001002)
#define TIMER2_BR       ((volatile cyg_uint8 *)0x34001012)
#define TIMER2_CR       ((volatile cyg_uint8 *)0x34001022)

#define PORT3_MD        ((volatile cyg_uint8 *)0x36008025)

// Mystery register
#define TMPSCNT         ((volatile cyg_uint8 *)0x34001071)

#define SIO2_LSTAT_TRDY  0x20
#define SIO2_LSTAT_RRDY  0x10

void hal_diag_init_serial2(void)
{
#if !defined(CYG_HAL_USE_ROM_MONITOR_CYGMON)    

    // 16 and 22 translate to 38400 baud.
    *TIMER2_BR = 16;
    *SERIAL2_TR = 22;
    
    // Timer2 sourced from IOCLK
    *TIMER2_MD = 0x80;    

    // Mode on PORT3, used for serial line controls.
    *PORT3_MD = 0x01;

    // No interrupts for now.
    *SERIAL2_ICR = 0x00;

    // Source from timer 2, 8bit chars, enable tx and rx
    *SERIAL2_CR = 0xc081;

#endif    
}

void hal_diag_write_char_serial2(char c)
{
    register volatile cyg_uint8 *volatile tty_status = SERIAL2_SR;
    register volatile cyg_uint8 *volatile tty_tx = SERIAL2_TXR;

    while( (*tty_status & SIO2_LSTAT_TRDY) != 0 ) continue;

    *tty_tx = c;
}

void hal_diag_drain_serial2(void)
{
    register volatile cyg_uint8 *volatile tty_status = SERIAL2_SR;

    while( (*tty_status & SIO2_LSTAT_TRDY) != 0 ) continue;
}

void hal_diag_read_char_serial2(char *c)
{
    register volatile cyg_uint8 *volatile tty_status = SERIAL2_SR;
    register volatile cyg_uint8 *volatile tty_rx = SERIAL2_RXR;

    while( (*tty_status & SIO2_LSTAT_RRDY) == 0 ) continue;

    *c = *tty_rx;
}

#if defined(CYG_KERNEL_DIAG_SERIAL2)

#define hal_diag_init_serial hal_diag_init_serial2
#define hal_diag_write_char_serial hal_diag_write_char_serial2
#define hal_diag_drain_serial hal_diag_drain_serial2
#define hal_diag_read_char_serial hal_diag_read_char_serial2

#endif

#endif

/*---------------------------------------------------------------------------*/

void hal_diag_init(void)
{
    hal_diag_init_serial();
}

void hal_diag_write_char(char c)
{
#ifdef CYG_KERNEL_DIAG_GDB

    static char line[100];
    static int pos = 0;

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
        
        HAL_DISABLE_INTERRUPTS(old);
        
        while(1)
        {
            static char hex[] = "0123456789ABCDEF";
            cyg_uint8 csum = 0;
            int i;
        
            hal_diag_write_char_serial('$');
            hal_diag_write_char_serial('O');
            csum += 'O';
            for( i = 0; i < pos; i++ )
            {
                char ch = line[i];
                char h = hex[(ch>>4)&0xF];
                char l = hex[ch&0xF];
                hal_diag_write_char_serial(h);
                hal_diag_write_char_serial(l);
                csum += h;
                csum += l;
            }
            hal_diag_write_char_serial('#');
            hal_diag_write_char_serial(hex[(csum>>4)&0xF]);
            hal_diag_write_char_serial(hex[csum&0xF]);

#if 0
            {
                char c1;

                hal_diag_read_char_serial( &c1 );

                // We must ack the interrupt caused by that read to avoid
                // confusing the GDB stub ROM.
                HAL_INTERRUPT_ACKNOWLEDGE( CYG_VECTOR_EXTERNAL_2 );
            
                if( c1 == '+' ) break;

                ai_writes("hal_diag_read_char_serial(");
                ai_write_char(c1);
                ai_writes(")\r\n");
                ai_writes("retry\r\n");

#if defined(CYG_HAL_USE_ROM_MONITOR) && defined(CYG_HAL_USE_ROM_MONITOR_CYGMON)
                if( c1 == 3 )
                {
                    // Ctrl-C: breakpoint.
//                HAL_BREAKPOINT(_breakinst);
                    typedef void bpt_fn();
                    bpt_fn *bfn = ((bpt_fn **)0x80000100)[61];

                    bfn();
                
                    break;
                }
#endif

            }
#else
            break;
#endif
        }
        
        pos = 0;

        // Wait for tx buffer to drain
        hal_diag_drain_serial();
        
        // And re-enable interrupts
        HAL_RESTORE_INTERRUPTS(old);
        
    }
    
#else
    hal_diag_write_char_serial(c);
#endif    
}




void hal_diag_read_char(char *c)
{
    hal_diag_read_char_serial(c);
}


/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
