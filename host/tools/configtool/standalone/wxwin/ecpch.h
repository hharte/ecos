// ####ECOSHOSTGPLCOPYRIGHTBEGIN####                                        
// -------------------------------------------                              
// This file is part of the eCos host tools.                                
// Copyright (C) 1998, 1999, 2000 Free Software Foundation, Inc.            
//
// This program is free software; you can redistribute it and/or modify     
// it under the terms of the GNU General Public License as published by     
// the Free Software Foundation; either version 2 or (at your option) any   
// later version.                                                           
//
// This program is distributed in the hope that it will be useful, but      
// WITHOUT ANY WARRANTY; without even the implied warranty of               
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        
// General Public License for more details.                                 
//
// You should have received a copy of the GNU General Public License        
// along with this program; if not, write to the                            
// Free Software Foundation, Inc., 51 Franklin Street,                      
// Fifth Floor, Boston, MA  02110-1301, USA.                                
// -------------------------------------------                              
// ####ECOSHOSTGPLCOPYRIGHTEND####                                          
// ecpch.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/08/24
// Version:     $Id$
// Purpose:
// Description: Precompiled header include file
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_ECPCH_H_
#define _ECOS_ECPCH_H_

#ifdef __VISUALC__
#pragma warning(disable:4786)
#endif

// include winsock2.h early to eliminate fd_set warning
// FIXME: including winsock2.h early causes build failures
// relating to the CreateDialog macro defined in the w32api.
// Need to find a better point to #include <winsock2.h>
//#ifdef __CYGWIN__
//#include <winsock2.h>
//#endif

#include "wx/wx.h"
#include "wx/splitter.h"
#include "wx/scrolwin.h"
#include "wx/imaglist.h"
#include "wx/laywin.h"
#include "wx/docview.h"

#ifdef new
#undef new
#endif

#include "configtool.h"
#include "cdl.hxx"

#endif
        // _ECOS_ECPCH_H_
