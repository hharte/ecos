//==========================================================================
//
//        serial_echo.c
//
//        Simple interactive echo test.
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
// Author(s):     jskov
// Contributors:  jskov
// Date:          2000-04-11
// Description:   Simple echo test.
// 
//####DESCRIPTIONEND####

#include <pkgconf/system.h>

#include <cyg/infra/testcase.h>         // test macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

// Package requirements
#if defined(CYGPKG_IO_SERIAL) && defined(CYGPKG_KERNEL)

#include <pkgconf/kernel.h>

// Package option requirements
#if defined(CYGFUN_KERNEL_API_C)

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL
#include <cyg/kernel/kapi.h>
unsigned char stack[CYGNUM_HAL_STACK_SIZE_TYPICAL];
cyg_thread thread_data;
cyg_handle_t thread_handle;

#include "ser_test_protocol.inl"

//---------------------------------------------------------------------------
// Serial test main function.
void
serial_test( void )
{
    cyg_io_handle_t ser_handle;
    cyg_uint8 in_buffer[1];
    int len = 1;

    test_open_ser(&ser_handle);

    CYG_TEST_INFO("Kill GDB and open a terminal emulator.");
    CYG_TEST_INFO("This test will echo all data.");

    while (1) {
        len = 1;
        Tcyg_io_read(ser_handle, in_buffer, &len);
        len = 1;
        Tcyg_io_write(ser_handle, in_buffer, &len);
    }

}

void
cyg_start(void)
{
    CYG_TEST_INIT();
    cyg_thread_create(10,                   // Priority - just a number
                      (cyg_thread_entry_t*)serial_test,         // entry
                      0,                    // 
                      "serial_thread",     // Name
                      &stack[0],            // Stack
                      CYGNUM_HAL_STACK_SIZE_TYPICAL,           // Size
                      &thread_handle,       // Handle
                      &thread_data          // Thread data structure
        );
    cyg_thread_resume(thread_handle);
    cyg_scheduler_start();
}

#else // CYGFUN_KERNEL_API_C
#define N_A_MSG "Needs kernel C API"
#endif

#else // CYGPKG_IO_SERIAL && CYGPKG_KERNEL
#define N_A_MSG "Needs IO/serial and Kernel"
#endif

#ifdef N_A_MSG
void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG);
}
#endif // N_A_MSG
// EOF serial3.c
