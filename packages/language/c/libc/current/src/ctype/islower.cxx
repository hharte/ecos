//===========================================================================
//
//      islower.cxx
//
//      Real alternative for inline implementation of ANSI standard
//      islower() ctype routine defined in section 7.3.1.6 of the standard
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

#include <pkgconf/libc.h>   // Configuration header

// Include the C library?
#ifdef CYGPKG_LIBC     

// INCLUDES

#include <cyg/infra/cyg_type.h>     // Common type definitions and support

// We don't want the inline versions of ctype functions defined here

#ifdef CYGIMP_LIBC_CTYPE_INLINES
#undef CYGIMP_LIBC_CTYPE_INLINES
#endif

#include <ctype.h>                  // Main header for ctype functions
#include "clibincl/ctypesupp.hxx"   // Support for ctype functions

// EXPORTED SYMBOLS

externC int
islower( int c ) CYGPRI_LIBC_WEAK_ALIAS("_islower");


// FUNCTIONS

int
_islower( int c )
{
    return (('a' <= c) && (c <= 'z'));
} // _islower()


#endif // ifdef CYGPKG_LIBC     

// EOF islower.cxx
