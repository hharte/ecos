#ifndef CYGONCE_HAL_MOD_77xx_H
#define CYGONCE_HAL_MOD_77xx_H

//=============================================================================
//
//      mod_7708.h
//
//      List modules available on CPU
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jskov
// Contributors:jskov
// Date:        2000-03-16
// Purpose:     Define modules (and versions) available on this CPU.
// Usage:       Included from <cyg/hal/sh_regs.h>
//
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//-----------------------------------------------------------------------------
// Modules provided by the CPU

#define CYGARC_SH_MOD_SCI_V2


//-----------------------------------------------------------------------------
// Extra details for Cache Module (CAC)

// Cache dimenions - one unified cache
#define CYGARC_SH_MOD_CAC_SIZE        8192  // Size of cache in bytes
#define CYGARC_SH_MOD_CAC_LINE_SIZE   16    // Size of a cache line
#define CYGARC_SH_MOD_CAC_WAYS        4     // Associativity of the cache

// Cache addressing information
// way:   bits 12 - 11
// entry: bits 10 -  4
#define CYGARC_SH_MOD_CAC_ADDRESS_BASE   0xf0000000
#define CYGARC_SH_MOD_CAC_ADDRESS_TOP    0xf0002000
#define CYGARC_SH_MOD_CAC_ADDRESS_STEP   0x00000010
// U : bit 1
// V : bit 0
// Writing zero to both forces a flush of the line if it is dirty.
#define CYGARC_SH_MOD_CAC_ADDRESS_FLUSH  0x00000000

#endif // CYGONCE_HAL_MOD_77xx_H
