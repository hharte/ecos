//=================================================================
//
//        strxfrm1.c
//
//        Testcase for C library strxfrm()
//
//=================================================================
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
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     ctarpy, jlarmour
// Contributors:  
// Date:          2000-04-30
// Description:   Contains testcode for C library strxfrm() function
//
//
//####DESCRIPTIONEND####

// CONFIGURATION

#include <pkgconf/system.h>
#include <pkgconf/libc_string.h>   // Configuration header

// INCLUDES

#include <string.h>
#include <cyg/infra/testcase.h>

// FUNCTIONS


// Functions to avoid having to use libc strings

static int my_strlen(const char *s)
{
    const char *ptr;

    ptr = s;
    for ( ptr=s ; *ptr != '\0' ; ptr++ )
        ;

    return (int)(ptr-s);
} // my_strlen()


static int my_strcmp(const char *s1, const char *s2)
{
    for ( ; *s1 == *s2 ; s1++,s2++ )
    {
        if ( *s1 == '\0' )
            break;
    } // for

    return (*s1 - *s2);
} // my_strcmp()


static char *my_strcpy(char *s1, const char *s2)
{
    while (*s2 != '\0') {
        *(s1++) = *(s2++);
    }
    *s1 = '\0';

    return s1; 
} // my_strcpy()


int main( int argc, char *argv[] )
{
    char x[300];
    char y[300];
    int  ret;

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "strxfrm() function");
    CYG_TEST_INFO("This testcase provides simple basic tests");

    // Check 1
    my_strcpy(x, "Nine rings for men doomed to die");
    ret = strxfrm(y, x, my_strlen(x)+1);
    if (my_strcmp(x, y) == 0) 
        CYG_TEST_PASS("Simple strxfrm()");
    else
        CYG_TEST_FAIL("Simple strxfrm()");
    // Check return value
    CYG_TEST_PASS_FAIL( (my_strlen(x) == ret),
                        "Simple strxfrm() return value");

    // Check 2
    x[0] = '\0';
    my_strcpy(y, "Seven rings for the dwarves in their halls of stone");
    ret = strxfrm(y, x, my_strlen(x)+1);
    if (my_strcmp(y, "") == 0)
        CYG_TEST_PASS("strxfrm() of empty string");
    else
        CYG_TEST_FAIL("strxfrm() of empty string");
    // Check return value
    CYG_TEST_PASS_FAIL( (my_strlen(x) == ret),
                        "strxfrm() of empty string return value");

    // Check 3
    my_strcpy(y, "Seven rings for the dwarves in their halls of stone");
    ret = strxfrm(NULL, y, 0);

    // Check return value
    CYG_TEST_PASS_FAIL( (my_strlen(y) == ret),
                        "strxfrm() of NULL string for 0 bytes return value");

//    CYG_TEST_NA("Testing is not applicable to this configuration");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "strxfrm() function");
} // main()


// EOF strxfrm1.c
