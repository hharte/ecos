#ifndef CYGONCE_CONFIG_KEYS_H
#define CYGONCE_CONFIG_KEYS_H
// ====================================================================
//
//      config_keys.h
//
//      Device I/O "Keys" for get/put config functions
//
// ====================================================================
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
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas,jskov,grante,jlarmour
// Date:         1999-02-04
// Purpose:      Repository for all get/put config "keys"
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// This file contains all of the 'key' values used by all I/O components.
// It is placed in this single repository to make it easy to reduce conflicts.

// ======== 0x0100 Serial ====================================================
// Get/Set configuration 'key' values for low-level serial I/O

#define CYG_IO_GET_CONFIG_SERIAL_INFO                  0x0101
#define CYG_IO_GET_CONFIG_SERIAL_OUTPUT_DRAIN          0x0102
#define CYG_IO_GET_CONFIG_SERIAL_OUTPUT_FLUSH          0x0103
#define CYG_IO_GET_CONFIG_SERIAL_INPUT_FLUSH           0x0104
#define CYG_IO_GET_CONFIG_SERIAL_ABORT                 0x0105
#define CYG_IO_GET_CONFIG_SERIAL_BUFFER_INFO           0x0111
#define CYG_IO_GET_CONFIG_SERIAL_FLOW_CONTROL_METHOD   0x0112

#define CYG_IO_SET_CONFIG_SERIAL_INFO                  0x0181
#define CYG_IO_SET_CONFIG_SERIAL_HW_RX_FLOW_THROTTLE   0x0184
#define CYG_IO_SET_CONFIG_SERIAL_HW_FLOW_CONFIG        0x0185
#define CYG_IO_SET_CONFIG_SERIAL_FLOW_CONTROL_METHOD   0x0186
#define CYG_IO_SET_CONFIG_SERIAL_FLOW_CONTROL_FORCE    0x0187
#define CYG_IO_SET_CONFIG_SERIAL_STATUS_CALLBACK       0x0188
#define CYG_IO_SET_CONFIG_SERIAL_HW_BREAK              0x0189

// Compatibility values. Use of these is deprecated, the generic symbols
// should be used instead.
#define CYG_IO_GET_CONFIG_SERIAL_READ_BLOCKING   CYG_IO_GET_CONFIG_READ_BLOCKING
#define CYG_IO_GET_CONFIG_SERIAL_WRITE_BLOCKING  CYG_IO_GET_CONFIG_WRITE_BLOCKING
#define CYG_IO_SET_CONFIG_SERIAL_READ_BLOCKING   CYG_IO_SET_CONFIG_READ_BLOCKING
#define CYG_IO_SET_CONFIG_SERIAL_WRITE_BLOCKING  CYG_IO_SET_CONFIG_WRITE_BLOCKING

// ======== 0x0200 TTY =======================================================
// Get/Set configuration 'key' values for tty-like driver
#define CYG_IO_GET_CONFIG_TTY_INFO       0x0201  // Get channel configuration
#define CYG_IO_SET_CONFIG_TTY_INFO       0x0281  // Set channel configuration


// ======== 0x0300 DSP =======================================================
// Get/Set configuration 'key' values for low-level DSP I/O
#define CYG_IO_GET_CONFIG_DSP_OUTPUT_DRAIN       0x0301
#define CYG_IO_GET_CONFIG_DSP_OUTPUT_FLUSH       0x0302
#define CYG_IO_GET_CONFIG_DSP_INPUT_FLUSH        0x0303
#define CYG_IO_GET_CONFIG_DSP_ABORT              0x0304
#define CYG_IO_GET_CONFIG_DSP_INPUT_OVERFLOW_RESET 0x0307

// Compatibility values. Use of these is deprecated, the generic symbols
// should be used instead.
#define CYG_IO_GET_CONFIG_DSP_READ_BLOCKING      CYG_IO_GET_CONFIG_READ_BLOCKING
#define CYG_IO_GET_CONFIG_DSP_WRITE_BLOCKING     CYG_IO_GET_CONFIG_WRITE_BLOCKING
#define CYG_IO_SET_CONFIG_DSP_READ_BLOCKING      CYG_IO_SET_CONFIG_READ_BLOCKING
#define CYG_IO_SET_CONFIG_DSP_WRITE_BLOCKING     CYG_IO_SET_CONFIG_WRITE_BLOCKING

// ======== 0x400 DSP =======================================================
// Get/Set configuration 'key' values for termios emulation

#define CYG_IO_GET_CONFIG_TERMIOS                0x0400
#define CYG_IO_SET_CONFIG_TERMIOS                0x0401

// ======== 0x1000 Generic ===================================================
// Get/Set configuration 'key' values that can apply to more than one
// class of device.

#define CYG_IO_GET_CONFIG_READ_BLOCKING         0x1001
#define CYG_IO_GET_CONFIG_WRITE_BLOCKING        0x1002

#define CYG_IO_SET_CONFIG_READ_BLOCKING         0x1081
#define CYG_IO_SET_CONFIG_WRITE_BLOCKING        0x1082


#endif  /* CYGONCE_CONFIG_KEYS_H */
/* EOF config_keys.h */
