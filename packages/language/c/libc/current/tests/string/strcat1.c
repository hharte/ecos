//=================================================================
//
//        strcat1.c
//
//        Testcase for C library strcat()
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
// Description:   Contains testcode for C library strcat() function
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
    char *ret;
#endif

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "strcat() function");
    CYG_TEST_INFO("This testcase provides simple basic tests");

#ifdef CYGPKG_LIBC

    // Check 1
    my_strcpy(x, "One ring to rule them all.");
    my_strcpy(y, "One ring to find them.");
    ret = strcat(x, y);
    if ( my_strcmp(x, "One ring to rule them all.One ring to find them.")==0 ) 
        CYG_TEST_PASS("Simple concatenation");
    else 
        CYG_TEST_FAIL("Simple concatenation");
    // Check return val
    CYG_TEST_PASS_FAIL( ( ret == x ), "Simple concatenation return value" );


    // Check 2
    my_strcpy(x, "One ring to bring them all,");
    my_strcpy(y, "");
    ret = strcat(x, y);
    if ( my_strcmp(x, "One ring to bring them all,")==0 ) 
        CYG_TEST_PASS("Concatenation of empty string");
    else 
        CYG_TEST_FAIL("Concatenation of empty string");
    // Check return val
    CYG_TEST_PASS_FAIL( ( ret == x ),
                        "Concatenation of empty string return value" );


    // Check 3
    my_strcpy(x, "and in the darkness bind them");
    my_strcpy(y, "");
    ret = strcat(y, x);
    if ( my_strcmp(x, "and in the darkness bind them")==0 ) 
        CYG_TEST_PASS("Concatenation to empty string");
    else 
        CYG_TEST_FAIL("Concatenation to empty string");
    // Check return val
    CYG_TEST_PASS_FAIL( ( ret == y ),
                        "Concatenation to empty string return value" );


#else // ifndef CYGPKG_LIBC
    CYG_TEST_PASS("Testing is not applicable to this configuration");
#endif // ifndef CYGPKG_LIBC

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "strcat() function");
} // main()


// EOF strcat1.c
