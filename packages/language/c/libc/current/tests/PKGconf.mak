#=============================================================================
#
#    makefile
#
#    language/c/libc/tests
#
#=============================================================================
#####COPYRIGHTBEGIN####
#
# -------------------------------------------
# The contents of this file are subject to the Cygnus eCos Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License.  You may obtain a copy of the License at
# http://sourceware.cygnus.com/ecos
# 
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the
# License for the specific language governing rights and limitations under
# the License.
# 
# The Original Code is eCos - Embedded Cygnus Operating System, released
# September 30, 1998.
# 
# The Initial Developer of the Original Code is Cygnus.  Portions created
# by Cygnus are Copyright (C) 1998 Cygnus Solutions.  All Rights Reserved.
# -------------------------------------------
#
#####COPYRIGHTEND####
#=============================================================================

PACKAGE       := libc
include ../../../../../pkgconf/pkgconf.mak

TESTS         := ctype/ctype                                          \
                                                                      \
                 i18n/setlocale                                       \
                                                                      \
                 setjmp/setjmp                                        \
                                                                      \
                 stdio/sprintf1 stdio/sprintf2 stdio/sscanf           \
                 stdio/stdiooutput                                    \
                                                                      \
                 stdlib/abs stdlib/atexit stdlib/atoi stdlib/atol     \
                 stdlib/bsearch stdlib/div stdlib/getenv              \
                 stdlib/labs stdlib/ldiv stdlib/qsort stdlib/malloc1  \
                 stdlib/malloc2 stdlib/malloc3 stdlib/rand1           \
                 stdlib/rand2 stdlib/rand3 stdlib/realloc stdlib/srand\
                 stdlib/strtol stdlib/strtoul                         \
                                                                      \
                 string/memchr string/memcmp1 string/memcmp2          \
                 string/memcpy1 string/memcpy2 string/memmove1        \
                 string/memmove2 string/memset string/strcat1         \
                 string/strcat2 string/strchr string/strcmp1          \
                 string/strcmp2 string/strcoll1 string/strcoll2       \
                 string/strcpy1 string/strcpy2 string/strcspn         \
                 string/strcspn string/strlen string/strncat1         \
                 string/strncat2 string/strncpy1 string/strncpy2      \
                 string/strpbrk string/strrchr string/strspn          \
                 string/strstr string/strtok string/strxfrm1          \
                 string/strxfrm2                                      \
                                                                      \
                 time/clock


include $(COMPONENT_REPOSITORY)/pkgconf/makrules.tst
