#==============================================================================
#
#    makefile
#
#    devs/common/misc
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

PACKAGE       := io_serial
include ../../../../pkgconf/pkgconf.mak
include ../../../../pkgconf/system.mak

PROGS	      := console
PROGS         += serial
WHEREAMI      := misc

include $(COMPONENT_REPOSITORY)/pkgconf/makrules.prv
