//=======================================================================
//
//      mainthread.cxx
//
//      Support for startup of ISO C environment
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
// Author(s):     jlarmour
// Contributors:  jlarmour
// Date:          1999-01-21
// Purpose:       Provides a thread object to call into a user-supplied
//                main()
// Description:   Here we define the thread object that calls
//                cyg_libc_invoke_main() which in turn will invoke
//                the user-supplied main() entry point function (or
//                alternatively the dummy empty one supplied by eCos)
// Usage:         Both the stack (cyg_libc_main_stack) and the thread
//                (cyg_libc_main_thread) can be overriden if you provide
//                your own symbols with those names. In the case of the
//                stack obviously you need to ensure
//                CYGNUM_LIBC_MAIN_STACK_SIZE corresponds to your own
//                stack.
//                The thread object is also available externally if you
//                want to control it (suspend/resume/etc.) either by
//                extern Cyg_Thread cyg_libc_main_thread; from C++, using
//                the kernel C++ API, or
//                extern cyg_handle_t cyg_libc_main_thread; from C using
//                the kernel C API.
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc.h>          // C library configuration

#ifdef CYGSEM_LIBC_STARTUP_MAIN_THREAD

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <pkgconf/kernel.h>        // eCos kernel configuration
#include <cyg/kernel/thread.hxx>   // eCos thread support
#include <cyg/kernel/thread.inl>
#include <cyg/hal/hal_arch.h>      // for CYGNUM_HAL_STACK_SIZE_TYPICAL


// EXTERNS

#ifdef CYGSEM_LIBC_INVOKE_DEFAULT_STATIC_CONSTRUCTORS
extern cyg_bool cyg_hal_stop_constructors;
#endif

// FUNCTION PROTOTYPES

externC void
cyg_libc_invoke_main( CYG_ADDRWORD );

// STATICS

#ifdef CYGSEM_LIBC_INVOKE_DEFAULT_STATIC_CONSTRUCTORS
class cyg_libc_dummy_constructor_class {
public:
    cyg_libc_dummy_constructor_class(void) { ++cyg_hal_stop_constructors; }
};

static cyg_libc_dummy_constructor_class cyg_libc_dummy_constructor_obj
                                            CYG_INIT_PRIORITY(PREDEFAULT);
#endif

#ifdef CYGSEM_LIBC_MAIN_STACK_FROM_SYSTEM

// override stack size on some platforms
#ifdef CYGNUM_HAL_STACK_SIZE_TYPICAL
# if CYGNUM_LIBC_MAIN_DEFAULT_STACK_SIZE < CYGNUM_HAL_STACK_SIZE_TYPICAL
#  undef CYGNUM_LIBC_MAIN_DEFAULT_STACK_SIZE
#  define CYGNUM_LIBC_MAIN_DEFAULT_STACK_SIZE CYGNUM_HAL_STACK_SIZE_TYPICAL
# endif
#endif

static cyg_uint8 cyg_libc_main_stack[ CYGNUM_LIBC_MAIN_DEFAULT_STACK_SIZE ];

#else // !ifdef CYGSEM_LIBC_MAIN_STACK_FROM_SYSTEM

extern char *cyg_libc_main_stack;
extern int cyg_libc_main_stack_size;

#endif // !ifdef CYGSEM_LIBC_MAIN_STACK_FROM_SYSTEM

// GLOBALS

// let the main thread be global so people can play with it (e.g. suspend
// or resume etc.) if that's what they want to do
Cyg_Thread cyg_libc_main_thread CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_LIBC) =
    Cyg_Thread(CYGNUM_LIBC_MAIN_THREAD_PRIORITY,
                &cyg_libc_invoke_main, (CYG_ADDRWORD) 0,
                "main",
                (CYG_ADDRESS) &cyg_libc_main_stack[0],
#ifdef CYGSEM_LIBC_MAIN_STACK_FROM_SYSTEM
                CYGNUM_LIBC_MAIN_DEFAULT_STACK_SIZE
#else
                cyg_libc_main_stack_size
#endif
              );

#endif // ifdef CYGSEM_LIBC_STARTUP_MAIN_THREAD

// EOF mainthread.cxx
