//=================================================================
//
//        memmove2.c
//
//        Testcase for C library memmove()
//
//=================================================================
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
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     ctarpy@cygnus.co.uk, jlarmour@cygnus.co.uk
// Contributors:    jlarmour@cygnus.co.uk
// Date:          1998/6/3
// Description:   Contains testcode for C library memmove() function
//
//
//####DESCRIPTIONEND####

// Declarations for test system:
//
// TESTCASE_TYPE=CYG_TEST_MODULE
// COMPOUND_TESTCASE


// CONFIGURATION

#include <pkgconf/libc.h>   // Configuration header

// INCLUDES

#include <string.h>
#include <cyg/infra/testcase.h>
#include <sys/cstartup.h>          // C library initialisation

// CONSTANTS

#define NUM_ROBUSTNESS_RUNS 300

// FUNCTIONS


externC void
cyg_package_start( void )
{
#ifdef CYGPKG_LIBC
    cyg_iso_c_start();
#else
    (void)main(0, NULL);
#endif
} // cyg_package_start()



// Functions to avoid having to use libc strings

#ifdef CYGPKG_LIBC
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
#endif // ifdef CYGPKG_LIBC


int main( int argc, char *argv[] )
{
#ifdef CYGPKG_LIBC
    char x[300];
    char y[300];
    void *ret, *ptr1, *ptr2;
    char *c_ret;
    int ctr;
    int fail;
#endif

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "memmove() function");
    CYG_TEST_INFO("This testcase tests robustness, and may take some time");


#ifdef CYGPKG_LIBC

    fail = 0;
    for (ctr = 0; ctr < NUM_ROBUSTNESS_RUNS; ctr++) {
        my_strcpy(x, "in the land of Mordor");
        my_strcpy(y, "                     ");
        ptr1 = x; 
        ptr2 = y;
        ret = memmove(ptr2, ptr1, my_strlen(x) + 1);
        c_ret = ret;
        if ((my_strcmp(y, x) != 0) || (my_strcmp(c_ret, x)!=0)) {
            fail = 1;
            break;
        }
    }

    CYG_TEST_PASS_FAIL( (fail == 0), "Robustness test");

#else // ifndef CYGPKG_LIBC
    CYG_TEST_PASS("Testing is not applicable to this configuration");
#endif // ifndef CYGPKG_LIBC

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "memmove() function");
} // main()


// EOF memmove2.c
