#==============================================================================
#
#    makefile
#
#    hal/powerpc/arch/src
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

PACKAGE       := hal_powerpc
include ../../../../../pkgconf/pkgconf.mak

LIBRARY       := libtarget.a
COMPILE       := hal_misc.c context.S ppc_stub.c hal_intr.c
OTHER_OBJS    :=
OTHER_TARGETS := vectors.stamp ldscript.stamp
OTHER_DEPS    := $(PACKAGE)_vectors.d ldscript.d
OTHER_CLEAN   := vectors.clean ldscript.clean

include $(COMPONENT_REPOSITORY)/pkgconf/makrules.src

.PHONY: ldscript.clean

vectors.stamp: $(PACKAGE)_vectors.o
	$(CP) $< $(PREFIX)/lib/vectors.o
	@$(TOUCH) $@

vectors.clean:
	$(RM) $(PREFIX)/lib/vectors.o $(PACKAGE)_vectors.o
	$(RM) vectors.stamp

$(PACKAGE)_vectors.o: $(BUILD_TREE)/pkgconf/pkgconf.mak \
    $(BUILD_TREE)/pkgconf/makevars $(PACKAGE_RULES_FILE)

ldscript.stamp: powerpc.ld
	$(CC) -E -P -Wp,-MD,ldscript.tmp -DEXTRAS=1 -xc $(INCLUDE_PATH) $(CFLAGS) -o $(PREFIX)/lib/target.ld $<
	@echo > ldscript.d
	@echo $@ ':' $< '\' >> ldscript.d
	@tail +2 ldscript.tmp >> ldscript.d
	@rm ldscript.tmp
	$(TOUCH) $@

ldscript.clean:
	$(RM) -f $(PREFIX)/lib/target.ld
	$(RM) -f ldscript.stamp

