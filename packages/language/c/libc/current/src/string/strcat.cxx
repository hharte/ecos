//===========================================================================
//
//      strcat.cxx
//
//      ANSI standard strcat() routine
//
//===========================================================================
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
// by Cygnus are Copyright (C) 1998,1999 Cygnus Solutions.  All Rights Reserved.
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
#include <stddef.h>           // Compiler definitions such as size_t, NULL etc.
#include "clibincl/stringsupp.hxx" // Useful string function support and
                                   // prototypes

// EXPORTED SYMBOLS

externC char *
strcat( char *s1, const char *s2 ) CYGPRI_LIBC_WEAK_ALIAS("_strcat");

// FUNCTIONS

char *
_strcat( char *s1, const char *s2 )
{
    CYG_REPORT_FUNCNAMETYPE( "_strcat", "returning %08x" );
    CYG_REPORT_FUNCARG2( "s1=%08x, s2=%08x", s1, s2 );

    CYG_CHECK_DATA_PTR( s1, "s1 is not a valid pointer!" );
    CYG_CHECK_DATA_PTR( s2, "s2 is not a valid pointer!" );

#if defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) || defined(__OPTIMIZE_SIZE__)
    char *s = s1;
    
    while (*s1)
        s1++;
    
    while ((*s1++ = *s2++))
        ;

    CYG_REPORT_RETVAL( s );

    return s;
#else
    char *s = s1;
    
    
    // Skip over the data in s1 as quickly as possible.
    if (!CYG_LIBC_STR_UNALIGNED (s1))
    {
        CYG_WORD *aligned_s1 = (CYG_WORD *)s1;
        while (!CYG_LIBC_STR_DETECTNULL (*aligned_s1))
            aligned_s1++;
        
        s1 = (char *)aligned_s1;
    }
    
    while (*s1)
        s1++;
    
    // s1 now points to the its trailing null character, we can
    // just use strcpy to do the work for us now.
    // 
    // ?!? We might want to just include strcpy here.
    // Also, this will cause many more unaligned string copies because
    // s1 is much less likely to be aligned.  I don't know if its worth
    // tweaking strcpy to handle this better.
    strcpy (s1, s2);
    
    CYG_REPORT_RETVAL( s );

    return s;
#endif // not defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) ||
       //     defined(__OPTIMIZE_SIZE__)
} // _strcat()

#endif // ifdef CYGPKG_LIBC     

// EOF strcat.cxx
