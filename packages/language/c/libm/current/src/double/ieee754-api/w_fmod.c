//===========================================================================
//
//      w_fmod.c
//
//      Part of the standard mathematical function library
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
// Author(s):   jlarmour@cygnus.co.uk
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

#include <pkgconf/libm.h>   // Configuration header

// Include the Math library?
#ifdef CYGPKG_LIBM     

// Derived from code with the following copyright


/* @(#)w_fmod.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

/* 
 * wrapper fmod(x,y)
 */

#include "mathincl/fdlibm.h"


        double fmod(double x, double y) /* wrapper fmod */
{
#ifdef CYGSEM_LIBM_COMPAT_IEEE_ONLY
        return __ieee754_fmod(x,y);
#else
        double z;
        z = __ieee754_fmod(x,y);
        if(cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_IEEE ||isnan(y)||isnan(x)) return z;
        if(y==0.0) {
                return __kernel_standard(x,y,27); /* fmod(x,0) */
        } else
            return z;
#endif
}

#endif // ifdef CYGPKG_LIBM     

// EOF w_fmod.c
