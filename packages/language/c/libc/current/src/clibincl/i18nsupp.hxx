#ifndef CYGONCE_LIBC_CLIBINCL_I18NSUPP_HXX
#define CYGONCE_LIBC_CLIBINCL_I18NSUPP_HXX
//========================================================================
//
//      i18nsupp.hxx
//
//      Support for ISO C internationalisation (i18n) functions
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
// Author(s):   jlarmour
// Contributors:  jlarmour
// Date:        1998-08-31
// Purpose:     
// Description: 
// Usage:       #include "clibincl/i18nsupp.hxx"
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc.h>   // Configuration header

// Include the C library?
#ifdef CYGPKG_LIBC     

// INCLUDES

#include <cyg/infra/cyg_type.h> // Common type definitions and support
#include <locale.h>             // struct lconv


// FUNCTION PROTOTYPES

// These are function prototypes for the aliased functions that actually
// implement the i18n functions

//========================================================================

externC char *
_setlocale( int /* category */, const char * /* locale */ );

externC struct lconv *
_localeconv( void );


#endif // ifdef CYGPKG_LIBC     

#endif // CYGONCE_LIBC_CLIBINCL_I18NSUPP_HXX multiple inclusion protection

// EOF i18nsupp.hxx
