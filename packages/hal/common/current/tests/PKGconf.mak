#===============================================================================
#
#    makefile
#
#    hal/common/tests
#
#===============================================================================
#####COPYRIGHTBEGIN####
#                                                                          
# -------------------------------------------                              
# The contents of this file are subject to the Red Hat eCos Public License 
# Version 1.1 (the "License"); you may not use this file except in         
# compliance with the License.  You may obtain a copy of the License at    
# http://www.redhat.com/                                                   
#                                                                          
# Software distributed under the License is distributed on an "AS IS"      
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
#===============================================================================

PACKAGE       := hal
include ../../../../pkgconf/pkgconf.mak
include ../../../../pkgconf/system.mak

TESTS	      := intr context

ifndef CYGPKG_HAL_ARM
TESTS	+= cache
endif

# Must be CYG_ - if CYGPKG_ would also match revision B
ifdef CYG_HAL_ARM_AEBC
TESTS	+= cache
endif


ifdef CYGPKG_HAL_ARM_EDB7XXX
TESTS	+= cache
endif

include $(COMPONENT_REPOSITORY)/pkgconf/makrules.tst



