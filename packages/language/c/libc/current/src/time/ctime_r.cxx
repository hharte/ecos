//========================================================================
//
//      ctime_r.cxx
//
//      POSIX 1003.1 date and time implementation for ctime_r()
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  jlarmour
// Date:          1999-02-26
// Purpose:       Provide implementation of POSIX 1003.1 ctime()
// Description:   This file provides the linkable symbol version of ctime_r()
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc.h>          // C library configuration

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support

// The following lines allow __ctime_r() to be included non-inline which
// actually defines it as a function here
#define CYGPRI_LIBC_TIME_CTIME_R_INLINE
#ifndef CYGIMP_LIBC_TIME_CTIME_R_INLINE
# define CYGIMP_LIBC_TIME_CTIME_R_INLINE
#endif

#include <time.h>                  // Main date and time definitions

// SYMBOL DEFINITIONS

#ifdef CYGFUN_LIBC_TIME_POSIX

# undef ctime_r

externC char *
ctime_r( const time_t *__timer, char *__buf )
    CYGBLD_ATTRIB_WEAK_ALIAS(__ctime_r);

#endif // ifdef CYGFUN_LIBC_TIME_POSIX

// EOF ctime_r.cxx
