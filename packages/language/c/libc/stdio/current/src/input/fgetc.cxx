//===========================================================================
//
//      fgetc.cxx
//
//      ISO C standard I/O get character functions
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
// Purpose:      Provide the fgetc() function. Also provides the function
//               version of getc() and getchar()
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
fgetc( FILE *stream )
{
    Cyg_StdioStream *real_stream = (Cyg_StdioStream *)stream;
    cyg_ucount32 bytes_read;
    Cyg_ErrNo err;
    cyg_uint8 c;

    CYG_REPORT_FUNCNAMETYPE("fgetc", "returning char %d");
    CYG_REPORT_FUNCARG1XV( stream );
    
    CYG_CHECK_DATA_PTR( stream, "stream is not a valid pointer" );

    err = real_stream->read( &c, 1, &bytes_read );

    // Why do we need this?  Because the buffer might be empty.
    if (!err && !bytes_read) { // if no err, but nothing to read, try again
        err = real_stream->refill_read_buffer();
        if ( !err )
            err = real_stream->read( &c, 1, &bytes_read );
    } // if

    CYG_ASSERT( (ENOERR != err) || (1 == bytes_read), "Didn't read 1 byte!" );

    if (err)
    {
        real_stream->set_error( err );
        errno = err;
        CYG_REPORT_RETVAL(EOF);
        return EOF;
    } // if
    
    CYG_REPORT_RETVAL((int)c);
    return (int)c;

} // fgetc()


// Also define getc() even though it can be a macro.
// Undefine it first though
#undef getchar

externC int
getchar( void )
{
    return fgetc( stdin );
} // getchar()


// EXPORTED SYMBOLS

// Also define getc() even though it can be a macro.
// Undefine it first though
#undef getc

externC int
getc( FILE * ) CYGBLD_ATTRIB_WEAK_ALIAS(fgetc);

// EOF fgetc.cxx
