#ifndef CYGONCE_HAL_BOARD_H
#define CYGONCE_HAL_BOARD_H

//=============================================================================
//
//      board.h
//
//      libstub board.h file for eCos HAL
//
//=============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.0 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://sourceware.cygnus.com/ecos                                        
//                                                                          
// Software distributed under the License is distributed on an       
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jskov
// Contributors:jskov
// Date:        1999-02-12
// Purpose:     libstub board.h file for eCos HAL
// Description: This file simplifies libstub integration. The board.h file
//              for eCos simply includes the hal_stub.h file which defines
//              the necessary architecture and platform information for
//              any of the possible eCos target configurations.
//              
//####DESCRIPTIONEND####
//
//=============================================================================

// Define __ECOS__; allows all eCos specific additions to be easily identified.
#define __ECOS__

#include <cyg/hal/hal_stub.h>

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_BOARD_H
// End of board.h
