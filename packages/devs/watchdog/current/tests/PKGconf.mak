#===============================================================================
#
#    makefile
#
#    devs/watchdog/current/tests
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

PACKAGE       := devices_watchdog
include ../../../../pkgconf/pkgconf.mak
include ../../../../pkgconf/system.mak

TESTS	      := watchdog

# The automatic test fails on some targets (i.e., the watchdog resets
# the board preventing the test from PASSing). Clear TESTS on those
# targets here.
ifdef CYGPKG_HAL_ARM_AEB
TESTS	      :=
endif

# Invoke make with RUN_BY_HAND=1 as argument
ifdef RUN_BY_HAND
TESTS         += watchdog_reset
endif

include $(COMPONENT_REPOSITORY)/pkgconf/makrules.tst
