#==============================================================================
#
#    makefile
#
#    hal/i386/linux/src
#
#==============================================================================
#####COPYRIGHTBEGIN####
#                                                                          
# -------------------------------------------                              
# The contents of this file are subject to the Red Hat eCos Public License 
# Version 1.0 (the "License"); you may not use this file except in         
# compliance with the License.  You may obtain a copy of the License at    
# http://sourceware.cygnus.com/ecos                                        
#                                                                          
# Software distributed under the License is distributed on an       
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
# License for the specific language governing rights and limitations under 
# the License.                                                             
#                                                                          
# The Original Code is eCos - Embedded Configurable Operating System,      
# released September 30, 1998.                                             
#                                                                          
# The Initial Developer of the Original Code is Red Hat.                   
# Portions created by Red Hat are                                          
# Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
# All Rights Reserved.                                                     
# -------------------------------------------                              
#                                                                          
#####COPYRIGHTEND####
#==============================================================================


PACKAGE       := hal_i386_linux
include ../../../../../pkgconf/pkgconf.mak

LIBRARY       := libtarget.a
COMPILE       := hal_diag.c hal_startup.c syscall-i386-linux-1.0.S entry.c \
                 linux.S linux_misc.c
OTHER_OBJS    :=
OTHER_TARGETS :=
OTHER_CLEAN   :=

OTHER_TARGETS :=
OTHER_CLEAN   :=
OTHER_DEPS    :=

include $(COMPONENT_REPOSITORY)/pkgconf/makrules.src
