#ifndef CYGONCE_ARM_AEB_SERIAL_H
#define CYGONCE_ARM_AEB_SERIAL_H

// ====================================================================
//
//      aeb_serial.h
//
//      Device I/O - Description of ARM AEB-1 serial hardware
//
// ====================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.0 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://sourceware.cygnus.com/ecos                                        
//                                                                          
// Software distributed under the License is distributed on an       
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
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           gthomas
// Contributors:        gthomas
// Date:        1999-02-04
// Purpose:     Internal interfaces for serial I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// Description of serial ports on ARM AEB-1

struct serial_port {
    unsigned char _byte[32];
};

#define reg(n) _byte[n*4]

// Receive control registers
#define rhr reg(0)    // Receive holding register
#define isr reg(2)    // Interrupt status register
#define lsr reg(5)    // Line status register
#define msr reg(6)    // Modem status register
#define scr reg(7)    // Scratch register

// Transmit control registers
#define thr reg(0)    // Transmit holding register
#define ier reg(1)    // Interrupt enable register
#define fcr reg(2)    // FIFO control register
#define lcr reg(3)    // Line control register
#define mcr reg(4)    // Modem control register
#define ldl reg(0)    // LSB of baud rate
#define mdl reg(1)    // MSB of baud rate

// Interrupt Enable Register
#define IER_RCV 0x01
#define IER_XMT 0x02
#define IER_LS  0x04
#define IER_MS  0x08

// Line Control Register
#define LCR_WL5 0x00    // Word length
#define LCR_WL6 0x01
#define LCR_WL7 0x02
#define LCR_WL8 0x03
#define LCR_SB1 0x00    // Number of stop bits
#define LCR_SB1_5 0x04  // 1.5 -> only valid with 5 bit words
#define LCR_SB2 0x04
#define LCR_PN  0x00    // Parity mode - none
#define LCR_PE  0x0C    // Parity mode - even
#define LCR_PO  0x08    // Parity mode - odd
#define LCR_PM  0x28    // Forced "mark" parity
#define LCR_PS  0x38    // Forced "space" parity
#define LCR_DL  0x80    // Enable baud rate latch

// Line Status Register
#define LSR_RSR 0x01
#define LSR_THE 0x20

// Modem Control Register
#define MCR_DTR 0x01
#define MCR_RTS 0x02
#define MCR_INT 0x08   // Enable interrupts

// Interrupt status register
#define ISR_Tx  0x02
#define ISR_Rx  0x04

static unsigned char select_word_length[] = {
    LCR_WL5,    // 5 bits / word (char)
    LCR_WL6,
    LCR_WL7,
    LCR_WL8
};

static unsigned char select_stop_bits[] = {
    0,
    LCR_SB1,    // 1 stop bit
    LCR_SB1_5,  // 1.5 stop bit
    LCR_SB2     // 2 stop bits
};

static unsigned char select_parity[] = {
    LCR_PN,     // No parity
    LCR_PE,     // Even parity
    LCR_PO,     // Odd parity
    LCR_PM,     // Mark parity
    LCR_PS,     // Space parity
};

// Baud rate values, based on raw 24MHz clock

static unsigned short select_baud[] = {
           0,  // Unused
     10000*3,  // 50
      6667*3,  // 75
      4545*3,  // 110
      3717*3,  // 134.5
      3333*3,  // 150
           0,  // 200
      1667*3,  // 300
       833*3,  // 600
       417*3,  // 1200
       277*3,  // 1800
       208*3,  // 2400
       139*3,  // 3600
       104*3,  // 4800
        69*3,  // 7200
        52*3,  // 9600
    (69*3)/2,  // 14400
        26*3,  // 19200
        13*3,  // 38400
           0,  // 57600
           0,  // 115200
           0,  // 230400
};

#endif // CYGONCE_ARM_AEB_SERIAL_H
