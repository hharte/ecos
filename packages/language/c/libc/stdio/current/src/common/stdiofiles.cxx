//========================================================================
//
//      stdiofiles.cxx
//
//      ISO C library stdio central file data
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
// Contributors:  
// Date:          2000-04-20
// Purpose:       Allocate storage for central file data object
// Description:   This file allocates the actual objects used by the
//                Cyg_libc_stdio_files class defined in
//                <cyg/libc/stdio/stdiofiles.hxx>
// Usage:       
//
//####DESCRIPTIONEND####
//
//=========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>          // C library configuration

// INCLUDES

#include <stddef.h>                      // NULL
#include <cyg/libc/stdio/stream.hxx>     // Cyg_StdioStream
#include <cyg/libc/stdio/stdiofiles.hxx> // Class definition for
                                         // Cyg_libc_stdio_files

#ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
# include <cyg/infra/cyg_type.h>   // CYGBLD_ATTRIB_INIT_PRI
# include <cyg/kernel/mutex.hxx>   // mutexes
#endif


// GLOBAL VARIABLES

Cyg_StdioStream *Cyg_libc_stdio_files::files[FOPEN_MAX] = { NULL };

# ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
Cyg_Mutex Cyg_libc_stdio_files::files_lock 
    CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_LIBC);
# endif

// EOF stdiofiles.cxx
