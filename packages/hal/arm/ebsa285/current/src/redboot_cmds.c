//==========================================================================
//
//      redboot_cmds.c
//
//      EBSA-285 [platform] specific RedBoot commands
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>

#include <cyg/hal/hal_ebsa285.h>   // Board definitions
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>


// Exported CLI function(s)
RedBoot_cmd("exec", 
            "Execute an image - with MMU off", 
            "[-w timeout] [<entry_point>]",
            do_exec
    );

typedef void code_fun(void);

static void 
do_exec(int argc, char *argv[])
{
    unsigned long entry;
    unsigned long oldints;
    code_fun *fun, *prg;
    bool wait_time_set;
    int  wait_time, res, i;
    bool base_addr_set, length_set;
    unsigned long base_addr, length;
    struct option_info opts[3];
    char line[8];
    unsigned long *_prg, *ip;

    entry = (unsigned long)entry_address;  // Default from last 'load' operation
    base_addr = 0x8000;
    init_opts(&opts[0], 'w', true, OPTION_ARG_TYPE_NUM, 
              (void **)&wait_time, (bool *)&wait_time_set, "wait timeout");
    init_opts(&opts[1], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void **)&base_addr, (bool *)&base_addr_set, "base address");
    init_opts(&opts[2], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void **)&length, (bool *)&length_set, "length");
    if (!scan_opts(argc, argv, 1, opts, 3, (void *)&entry, OPTION_ARG_TYPE_NUM, "starting address"))
    {
        return;
    }
    if (wait_time_set) {
        printf("About to start execution at %p - abort with ^C within %d seconds\n",
               (void *)entry, wait_time);
        res = gets(line, sizeof(line), wait_time*1000);
        if (res == -2) {
            return;
        }
    }
    if (base_addr_set && !length_set) {
      printf("Length required for non-standard base address\n");
      return;
    }
    ip = (unsigned long *)&&lab1;
    _prg = (unsigned long *)0x1F00;  // Should be a very safe location to execute from
    for (i = 0;  i < (unsigned long)&&end1-(unsigned long)&&lab1;  i++) {
        _prg[i] = *ip++;
    }
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_ICACHE_DISABLE();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    ip = (unsigned long *)_prg;
    // Now call this code
    fun = (code_fun *)((entry & 0x0FFFFFFF) + EBSA285_RAM_BANK0_BASE);  // Absolute address
    prg = (code_fun *)((unsigned long)ip + EBSA285_RAM_BANK0_BASE);  // Absolute address
    asm volatile ("mov r5,%0;"
		  "mov r2,%1;"
		  "mov r3,%2;"
		  "mov r1,%3;"
		  "mov pc,r1" : : 
		  "r"(fun),"r"(base_addr),"r"(length),"r"(prg) : "r5","r2","r3","r1");
    //    asm volatile ("mov r2,%0" : : "r"(base_addr) : "r2");
    //    asm volatile ("mov r3,%0" : : "r"(length) : "r3");
    //    asm volatile ("mov r1,%0; mov pc,r1" : : "r"(prg) : "r1");
 lab1:
    // Tricky code.  We are currently running with the MMU on and the
    // memory map convoluted from 1-1.  This code must be copied to RAM
    // and then executed at the non-mapped address.  Then when the code
    // turns off the MMU, the control flow is unchanged and control can
    // be passed safely to the program to be executed.
    // This magic was created in order to be able to execute standard
    // Linux kernels with as little change/perturberance as possible.
    //		  ".long 0xE7FFDEFE;"  // Illegal
    asm volatile ("mrs r1,cpsr;"
                  "bic r0,r1,#0x1F;"  // Put processor in IRQ mode
                  "orr r0,r0,#0x12;"
                  "msr cpsr,r0;"
                  "msr spsr,r1;"
                  "mov lr,r5;"
                  "mov sp,r5;"        // Give the kernel a stack just below the entry point
                  "mov r1,#0;"
                  "mcr p15,0,r1,c1,c0;" // Turns off MMU, caches
		  "mov r1,#0x8000;"   // Default kernel load address
                  "cmp r2,r1;"        // Is it there?
                  "beq 10f;"
		  "ldr r4,=0x40012000;mov r5,#0;str r5,[r4];"
		  "05:;"
		  "ldr r4,[r2],#4;"
		  "str r4,[r1],#4;"
		  "sub r3,r3,#4;"
		  "cmp r3,#0;"
		  "bne 05b;"
		  "10:;"
                  "mov r0,#0;"        // Set up SA110 load magic
                  "mov r1,#0x4;" 
		  "ldr r4,=0x40012000;mov r5,#1;str r5,[r4];"
                  "movs pc,lr" : : );
 end1:
}
