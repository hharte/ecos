//=================================================================
//
//        atoi.c
//
//        Testcase for C library atoi()
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
// Description:   Contains testcode for C library atoi() function
//
//
//####DESCRIPTIONEND####

// Declarations for test system:
//
// TESTCASE_TYPE=CYG_TEST_MODULE

// CONFIGURATION

#include <pkgconf/libc.h>   // Configuration header

// INCLUDES

#include <stdlib.h>
#include <cyg/infra/testcase.h>
#include <sys/cstartup.h>          // C library initialisation

// HOW TO START TESTS

#if defined(CYGPKG_LIBC)

# define START_TEST( test ) test(0)

#else

# define START_TEST( test ) CYG_EMPTY_STATEMENT

#endif


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


#ifdef CYGPKG_LIBC

static char *
my_strcpy(char *s1, const char *s2)
{
    while (*s2 != '\0') {
        *(s1++) = *(s2++);
    }
    *s1 = '\0';

    return s1; 
} // my_strcpy()


static void
test( CYG_ADDRWORD data )
{
    char x[30];

    // Check 1
    my_strcpy(x, "20");
    CYG_TEST_PASS_FAIL( atoi(x) == 20, "atoi(20)");

    // Check 2
    my_strcpy(x, "1972");
    CYG_TEST_PASS_FAIL( atoi(x) == 1972, "atoi(1972)");

    // Check 3
    my_strcpy(x, "-9876");
    CYG_TEST_PASS_FAIL( atoi(x) == -9876, "atoi(-9876)");

    // Check 4
    my_strcpy(x, "  -9876xxx");
    CYG_TEST_PASS_FAIL( atoi(x) == -9876, "atoi(   -9876xxx)");


    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "atoi() function");
} // test()

#endif // ifndef CYGPKG_LIBC

int
main(int argc, char *argv[])
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "atoi() function");

    START_TEST( test );

    CYG_TEST_PASS_FINISH("Testing is not applicable to this configuration");
} // main()

// EOF atoi.c
