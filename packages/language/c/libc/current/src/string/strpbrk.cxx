//===========================================================================
//
//      strpbrk.cxx
//
//      ANSI standard strpbrk() routine
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jlarmour
// Contributors:  jlarmour
// Date:        1998-02-13
// Purpose:     
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc.h>   // Configuration header

// Include the C library?
#ifdef CYGPKG_LIBC     

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions
#include <cyg/infra/cyg_trac.h>    // Tracing support
#include <cyg/infra/cyg_ass.h>     // Assertion support
#include <string.h>                // Header for this file
#include <stddef.h>         // Compiler definitions such as size_t, NULL etc.
#include "clibincl/stringsupp.hxx" // Useful string function support and
                                   // prototypes

// EXPORTED SYMBOLS

externC char *
strpbrk( const char *s1, const char *s2 ) CYGPRI_LIBC_WEAK_ALIAS("_strpbrk");

// FUNCTIONS

char *
_strpbrk( const char *s1, const char *s2 )
{
    const char *c;

    CYG_REPORT_FUNCNAMETYPE( "_strpbrk", "returning %08x" );
    CYG_REPORT_FUNCARG2( "s1=%08x, s2=%08x", s1, s2 );

    CYG_CHECK_DATA_PTR( s1, "s1 is not a valid pointer!" );
    CYG_CHECK_DATA_PTR( s2, "s2 is not a valid pointer!" );

    if (!*s1)
    {
        CYG_REPORT_RETVAL( NULL );
        return (char *) NULL;
    } // if
    
    do
    {
        for (c = s2; *c; c++)
        {
            if (*s1 == *c)
                break;
        }
        if (*c)
            break;
        s1++;
    }
    while (*s1);
    
    if (*c == '\0')
        s1 = NULL;
    
    CYG_REPORT_RETVAL( s1 );
    
    return (char *) s1;
} // _strpbrk()

#endif // ifdef CYGPKG_LIBC     

// EOF strpbrk.cxx
