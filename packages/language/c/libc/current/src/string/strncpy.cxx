//===========================================================================
//
//      strncpy.cxx
//
//      ISO C standard strncpy() routine
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
// Author(s):     jlarmour
// Contributors:  jlarmour
// Date:          1999-02-18
// Purpose:     
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc.h>   // Configuration header

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
strncpy( char *s1, const char *s2, size_t n ) \
    CYGBLD_ATTRIB_WEAK_ALIAS(_strncpy);

// FUNCTIONS

char *
_strncpy( char *s1, const char *s2, size_t n)
{
    CYG_REPORT_FUNCNAMETYPE( "_strncpy", "returning %08x" );
    CYG_REPORT_FUNCARG3( "s1=%08x, s2=%08x, n=%d", s1, s2, n );

    CYG_CHECK_DATA_PTR( s1, "s1 is not a valid pointer!" );
    CYG_CHECK_DATA_PTR( s2, "s2 is not a valid pointer!" );

#if defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) || defined(__OPTIMIZE_SIZE__)
    char *dscan;
    const char *sscan;
    
    dscan = s1;
    sscan = s2;
    while (n > 0)
    {
        --n;
        if ((*dscan++ = *sscan++) == '\0')
            break;
    }
    while (n-- > 0)
        *dscan++ = '\0';

    CYG_REPORT_RETVAL( s1 );
    
    return s1;
#else
    char *dst = s1;
    const char *src = s2;
    CYG_WORD *aligned_dst;
    const CYG_WORD *aligned_src;
    
    // If SRC and DEST is aligned and count large enough, then copy words.
    if (!CYG_LIBC_STR_UNALIGNED2 (src, dst) &&
        !CYG_LIBC_STR_OPT_TOO_SMALL (n)) {

        aligned_dst = (CYG_WORD *)dst;
        aligned_src = (CYG_WORD *)src;

        // SRC and DEST are both "CYG_WORD" aligned, try to do "CYG_WORD"
        // sized copies.
        while (n >= sizeof (CYG_WORD) && 
               !CYG_LIBC_STR_DETECTNULL(*aligned_src)) {

            n -= sizeof (CYG_WORD);
            *aligned_dst++ = *aligned_src++;
        }

        dst = (char *)aligned_dst;
        src = (const char *)aligned_src;
    } // if
    
    while (n > 0)
    {
        --n;
        if ((*dst++ = *src++) == '\0')
            break;
    }
    
    while (n-- > 0)
        *dst++ = '\0';
    
    CYG_REPORT_RETVAL( s1 );

    return s1;
#endif // not defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) ||
       //     defined(__OPTIMIZE_SIZE__)
} // _strncpy()

// EOF strncpy.cxx
