#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H

/*=============================================================================
//
//      hal_platform_setup.h
//
//      Platform specific support for HAL (assembly code)
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:        1999-04-13
// Purpose:     ARM/AEB-1 platform specific support routines
// Description: 
// Usage:       #include <cyg/hal/hal_platform_setup.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

// From <cyg/hal/hal_cache.h> Need to make that file assembly safe.
#define CYG_DEVICE_CCR          0xFFFFA400
#define CCR_I                   0x08      // Invalidate mode

#if CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE==4096
// Override default to a more sensible value
#undef  CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
#define CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE 2048
#endif

#ifdef CYGHWR_HAL_ARM_AEB_REVISION_C
// AEB rev C has 256kB of memory. Cache is working (set cachable)
#if 0
#define AEB_SRAM .long	0xFFFFA008,0x00008000,0x00048000,0x00007c04
#define AEB_BAD  .long	0xFFFFA00C,0x00048000,0x01000000,0x00000000
#else
// FIXME: There is a cache problem of some sort. Either eCos or the
// chip. Leave cache disabled till I find the time to fix it. Jesper
#define AEB_SRAM .long	0xFFFFA008,0x00008000,0x00048000,0x00007804
#define AEB_BAD  .long	0xFFFFA00C,0x00048000,0x01000000,0x00000000
#endif
#else
// AEB rev B has 128kB of memory. Cache is broken (clear cachable)
#define AEB_SRAM .long	0xFFFFA008,0x00008000,0x00028000,0x00007804
#define AEB_BAD  .long	0xFFFFA00C,0x00028000,0x01000000,0x00000000
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
#define PLATFORM_SETUP1                                                       \
        ldr     r1,=CYG_DEVICE_CCR                                           ;\
        mov     r2,#CCR_I                                                    ;\
        strb    r2,[r1,#0]      /* invalidate... */                          ;\
        mov     r2,#0                                                        ;\
        strb    r2,[r1,#0]      /* and disable the cache. */                 ;\
	ldr	r1,=segment_register_setups                                  ;\
10:	ldr	r2,[r1,#0]	/* segment address */                        ;\
	cmp	r2,#0                                                        ;\
	beq	20f                                                          ;\
	ldr	r3,[r1,#4]	/* segment start */                          ;\
	str	r3,[r2,#0x00]                                                ;\
	ldr	r3,[r1,#8]	/* segment end */                            ;\
	str	r3,[r2,#0x20]                                                ;\
	ldr	r3,[r1,#12]	/* segment flags */                          ;\
	str	r3,[r2,#0x40]                                                ;\
	add	r1,r1,#16	/* next segment  */                          ;\
	b	10b                                                          ;\
segment_register_setups:                                                     ;\
	AEB_SRAM  /* segment 2 */                                            ;\
	AEB_BAD   /* segment 3 */                                            ;\
	.long 0                                                              ;\
20:
#else
#define PLATFORM_SETUP1
#endif

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
