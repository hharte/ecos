//==========================================================================
//
//        serial4.c
//
//        Test serial configuration changing
//
//==========================================================================
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
// by Cygnus are Copyright (C) 1998, 1999 Cygnus Solutions.  
// All Rights Reserved.
// -------------------------------------------
//
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jskov
// Contributors:  jskov
// Date:          1999-03-17
// Description:   Test the configuration capabilities of the serial driver.
// Requirements:  This test requires the ser_filter on the host side.
// 
// To Do:
//  This test should do a full test of the configuration combinations.
//  This will get possible with proper test_change_config protocol
//  implementation.
//####DESCRIPTIONEND####

#include <pkgconf/system.h>

#include <cyg/infra/testcase.h>         // test macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#if defined(CYGPKG_IO_SERIAL) && defined(CYGPKG_KERNEL)

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#ifdef CYGFUN_KERNEL_API_C
#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL
#include <cyg/kernel/kapi.h>
unsigned char stack[CYGNUM_HAL_STACK_SIZE_TYPICAL];
cyg_thread thread_data;
cyg_handle_t thread_handle;
#endif
#endif

#include "ser_test_protocol.inl"

//---------------------------------------------------------------------------
// Serial test main function.
void
serial_test( void )
{
    cyg_io_handle_t ser_handle;

    test_open_ser(&ser_handle);

    // We need the filter for this test.
    test_ping(ser_handle);

    // Test multiple configurations (defined in the protocol file).
    // FIXME: This will get replaced with a test over the full configuration
    // permutation space.
    {
        int i;
        int count = sizeof(test_configs) / sizeof(cyg_ser_cfg_t);
        char msg[] = "This is a test\n";
        int msglen = strlen(msg);

        for (i = 0; i < count; i++){
            change_config(ser_handle, &test_configs[i]);
            test_binary(ser_handle,    128, MODE_EOP_ECHO);
            test_binary(ser_handle,    256, MODE_NO_ECHO);
            Tcyg_io_write(ser_handle, msg, &msglen);
        }
    }

    CYG_TEST_PASS_FINISH("serial4 test OK");
}

void
cyg_start(void)
{
    CYG_TEST_INIT();
#ifdef CYGFUN_KERNEL_API_C
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
#else
    serial_test();
#endif
}

#else // ifdef CYGPKG_IO_SERIAL

void
cyg_start(void)
{
    CYG_TEST_INIT();
    CYG_TEST_NA("Requires serial device driver and kernel");
}

#endif
// EOF serial4.c
