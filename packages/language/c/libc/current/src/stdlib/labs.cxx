//===========================================================================
//
//      labs.cxx
//
//      Real alternative for inline implementation of the ANSI standard
//      labs() utility function defined in section 7.10.6.3 of the standard
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
// by Cygnus are Copyright (C) 1998 Cygnus Solutions.  All Rights Reserved.
// -------------------------------------------
//
//####COPYRIGHTEND####
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jlarmour
// Contributors:  jlarmour@cygnus.co.uk
// Date:        1998-02-13
// Purpose:     
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc.h>           // Configuration header

// Include the C library?
#ifdef CYGPKG_LIBC     

// INCLUDES

#include <cyg/infra/cyg_type.h>     // Common type definitions and support
#include <cyg/infra/cyg_trac.h>     // Tracing support

// We don't want the inline versions of stdlib functions defined here

#ifdef CYGIMP_LIBC_STDLIB_INLINES
#undef CYGIMP_LIBC_STDLIB_INLINES
#endif

#include <stdlib.h>                 // Main header for stdlib functions
#include "clibincl/stdlibsupp.hxx"  // Support for stdlib functions


// EXPORTED SYMBOLS

externC long
labs( long j ) CYGPRI_LIBC_WEAK_ALIAS("_labs");


// FUNCTIONS

long
_labs( long j )
{
    long retval;

    CYG_REPORT_FUNCNAMETYPE( "_labs", "returning %d" );
    
    retval = (j<0) ? -j : j;

    CYG_REPORT_RETVAL( retval );

    return retval;
} // _labs()


#endif // ifdef CYGPKG_LIBC     

// EOF labs.cxx
