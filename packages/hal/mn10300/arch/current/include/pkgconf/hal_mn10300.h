#ifndef CYGONCE_PKGCONF_HAL_MN10300_H
#define CYGONCE_PKGCONF_HAL_MN10300_H
// ====================================================================
//
//      pkgconf/hal_mn10300.h
//
//      HAL configuration file
//
// ====================================================================
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
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           bartv
// Contributors:        bartv
// Date:                1998-09-02      
// Purpose:             To allow the user to edit HAL configuration options.
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================


/* ---------------------------------------------------------------------
   {{CFG_DATA

   cdl_package CYGPKG_HAL_MN10300 {
       display "MN10300 architecture"
       type    radio
       parent  CYGPKG_HAL
       target  mn10300
       description "
           The MN10300 architecture HAL package provides generic
           support for this processor architecture. It is also
           necessary to select a specific target platform HAL
           package."
   }
   
   }}CFG_DATA */

/* -------------------------------------------------------------------*/
#endif  /* CYGONCE_PKGCONF_HAL_MN10300_H */
/* EOF hal_mn10300.h */
