//=================================================================
//
//        strcoll1.c
//
//        Testcase for C library strcoll()
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
// Description:   Contains testcode for C library strcoll() function
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <string.h>
#include <cyg/infra/testcase.h>


// FUNCTIONS

// Functions to avoid having to use libc strings


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

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "strcoll() function");
    CYG_TEST_INFO("This testcase provides simple basic tests");

    // Check 1
    my_strcpy(x, "I have become, comfortably numb");
    my_strcpy(y, "I have become, comfortably numb");
    CYG_TEST_PASS_FAIL( (strcoll(x, y) == 0), "Simple compare");


    // Check 2
    my_strcpy(x, "");
    my_strcpy(y, "");
    CYG_TEST_PASS_FAIL( (strcoll(x, y) == 0), "Simple empty string compare");


    // Check 3
    my_strcpy(x, "..shall snuff it. And the Lord did grin");
    my_strcpy(y, "..shall snuff it. And the Lord did grio");
    CYG_TEST_PASS_FAIL( (strcoll(x, y) < 0),
                        "Memory less than #1" );


    // Check 4
    my_strcpy(x, "A reading from the Book of Armaments, Chapter 4, "
                  "Verses 16 to 20:");
    my_strcpy(y, "Bless this, O Lord, that with it thou mayst blow thine "
                 "enemies to tiny bits, in thy mercy.");
    CYG_TEST_PASS_FAIL( (strcoll(x, y) < 0),
                        "Memory less than #2");

    // Check 5
    my_strcpy(x, "Lobeth this thy holy hand grenade at thy foe");
    my_strcpy(y, "Lobeth this thy holy hand grenade at thy fod");
    CYG_TEST_PASS_FAIL( (strcoll(x, y) > 0),
                        "Memory greater than #1" );


    // Check 6
    my_strcpy(y, "Three shall be the number of the counting and the");
    my_strcpy(x, "number of the counting shall be three");
    CYG_TEST_PASS_FAIL( (strcoll(x, y) > 0),
                        "Memory greater than #2" );

//    CYG_TEST_NA("Testing is not applicable to this configuration");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "strcoll() function");
} // main()



// EOF strcoll1.c
