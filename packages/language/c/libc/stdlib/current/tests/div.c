//=================================================================
//
//        div.c
//
//        Testcase for C library div()
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
// Description:   Contains testcode for C library div() function
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <stdlib.h>
#include <cyg/infra/testcase.h>

// FUNCTIONS

int
main( int argc, char *argv[] )
{
    int num, denom;
    div_t result;

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "div() function");

    num = 10232;
    denom = 43;
    result = div(num, denom);
    CYG_TEST_PASS_FAIL( (result.quot==237) && (result.rem==41),
                        "div( 10232, 43 )");

    num = 4232;
    denom = 2000;
    result = div(num, denom);
    CYG_TEST_PASS_FAIL( (result.quot==2) && (result.rem==232),
                        "div( 4232, 2000 )");


    num = 20;
    denom = 20;
    result = div(num, denom);
    CYG_TEST_PASS_FAIL( (result.quot==1) && (result.rem==0),
                        "div( 20, 20 )");

    num = -5;
    denom = 4;
    result = div(num, denom);
    CYG_TEST_PASS_FAIL( (result.quot==-1) && (result.rem==-1),
                        "div( -5, 4 )");

    num = 5;
    denom = -4;
    result = div(num, denom);
    CYG_TEST_PASS_FAIL( (result.quot==-1) && (result.rem==1),
                        "div( 5, -4 )");

    num = -5;
    denom = -3;
    result = div(num, denom);
    CYG_TEST_PASS_FAIL( (result.quot==1) && (result.rem==-2),
                        "div( -5, -3 )");

    num = -7;
    denom = -7;
    result = div(num, denom);
    CYG_TEST_PASS_FAIL( (result.quot==1) && (result.rem==0),
                        "div( -7, -7 )");


    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "div() function");

} // main()

// EOF div.c
