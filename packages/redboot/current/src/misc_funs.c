//==========================================================================
//
//      misc_funs.c
//
//      Miscellaneous [library] functions for RedBoot
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

int
strlen(const char *s)
{
    int len = 0;
    while (*s++) len++;
    return len;
}

int
strcmp(const char *s1, const char *s2)
{
    char c1, c2;

    while ((c1 = *s1++) == (c2 = *s2++))
        if (c1 == 0)
            return (0);
    return ((unsigned char)c1 - (unsigned char)c2);
}

void
strcpy(char *s1, const char *s2)
{
    char c;

    while ((c = *s2++) != '\0')
        *s1++ = c;
    *s1 = '\0';
}

int
strcmpci(const char *s1, const char *s2)
{
    char c1, c2;
    while ((c1 = tolower(*s1++)) == (c2 = tolower(*s2++)))
        if (c1 == 0)
            return (0);
    return ((unsigned char)c1 - (unsigned char)c2);
}

int
strncmp(const char *s1, const char *s2, int len)
{
    char c1, c2;

    if (len == 0)
        return 0;
    do {
        if ((c1 = *s1++) != (c2 = *s2++))
            return ((unsigned char)c1 - (unsigned char)c2);
        if (c1 == 0)
            break;
    } while (--len != 0);
    return 0;
}

int
strncmpci(const char *s1, const char *s2, int len)
{
    char c1, c2;

    if (len == 0)
        return 0;
    do {
        if ((c1 = tolower(*s1++)) != (c2 = tolower(*s2++)))
            return ((unsigned char)c1 - (unsigned char)c2);
        if (c1 == 0)
            break;
    } while (--len != 0);
    return 0;
}

void
memmove(unsigned char *dst, unsigned char *src, int len)
{
    while (len-- > 0) *dst++ = *src++;
}

int
memcmp(const unsigned char *dst, const unsigned char *src, int len)
{
    while (len-- > 0) {
        if (*dst++ != *src++) {
            return (*--dst - *--src);
        }
    }
    return 0;
}

void
memset(unsigned char *dst, unsigned char val, int len)
{
    while (--len >= 0) {
        *dst++ = val;
    }
}
