//===========================================================================
//
//      fputc.cxx
//
//      ISO Standard I/O character output functions
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: 
// Date:         2000-04-20
// Purpose:      Provide the fputc() function. Also provides the function
//               versions of putc() and putchar()
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>     // Common project-wide type definitions
#include <cyg/infra/cyg_ass.h>      // Standard eCos assertion support
#include <cyg/infra/cyg_trac.h>     // Standard eCos tracing support
#include <stddef.h>                 // NULL and size_t from compiler
#include <stdio.h>                  // header for this file
#include <errno.h>                  // error codes
#include <cyg/libc/stdio/stream.hxx>// Cyg_StdioStream

// FUNCTIONS

externC int
fputc( int c, FILE *stream )
{
    Cyg_StdioStream *real_stream = (Cyg_StdioStream *)stream;
    Cyg_ErrNo err;
    cyg_uint8 real_c = (cyg_uint8) c;
    
    CYG_REPORT_FUNCNAMETYPE("fputc", "wrote char %d");
    CYG_REPORT_FUNCARG2( "c = %d, stream=%08x", c, stream );
    
    CYG_CHECK_DATA_PTR( stream, "stream is not a valid pointer" );

    err = real_stream->write_byte( real_c );

    if (err)
    {
        real_stream->set_error( err );
        errno = err;
        CYG_REPORT_RETVAL(EOF);
        return EOF;
    } // if
    
    CYG_REPORT_RETVAL((int)real_c);
    return (int)real_c;

} // fputc()


// Also define putchar() even though it can be a macro.
// Undefine the macro first though
#undef putchar

externC int
putchar( int c )
{
    return fputc( c, stdout );
} // putchar()

// Also define putc() even though it can be a macro.
// Undefine the macro first though
#undef putc

externC int
putc( int, FILE * ) CYGBLD_ATTRIB_WEAK_ALIAS(fputc);

// EOF fputc.cxx
