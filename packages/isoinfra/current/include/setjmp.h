#ifndef CYGONCE_ISO_SETJMP_H
#define CYGONCE_ISO_SETJMP_H
/*========================================================================
//
//      setjmp.h
//
//      ISO non-local jump functions
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
// Date:          2000-04-14
// Purpose:       This file provides the setjmp macros, types and functions
//                required by ISO C and POSIX 1003.1.
// Description:   The real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <setjmp.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/isoinfra.h>          /* Configuration header */

/* INCLUDES */

#if CYGINT_ISO_SETJMP
# include CYGBLD_ISO_SETJMP_HEADER
#endif

#if CYGINT_ISO_SIGSETJMP
# include CYGBLD_ISO_SIGSETJMP_HEADER
#endif

#endif /* CYGONCE_ISO_SETJMP_H multiple inclusion protection */

/* EOF setjmp.h */
