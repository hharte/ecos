#===============================================================================
#
#    makefile
#
#    kernel/src
#
#===============================================================================
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
# by Cygnus are Copyright (C) 1998,1999 Cygnus Solutions.  All Rights Reserved.
# -------------------------------------------
#
#####COPYRIGHTEND####
#===============================================================================

PACKAGE       := kernel
include ../../../pkgconf/pkgconf.mak

LIBRARY       := libtarget.a
COMPILE       := common/clock.cxx common/timer.cxx \
		 common/kapi.cxx common/thread.cxx common/except.cxx \
		 instrmnt/meminst.cxx instrmnt/nullinst.cxx \
		 intr/intr.cxx \
		 mem/memfixed.cxx mem/memvar.cxx \
		 sched/bitmap.cxx sched/lottery.cxx sched/mlqueue.cxx \
		 sched/sched.cxx \
		 sync/bin_sem.cxx sync/cnt_sem.cxx sync/flag.cxx \
		 sync/cnt_sem2.cxx sync/mbox.cxx sync/mutex.cxx \
		 debug/dbg-thread-demux.c debug/dbg_gdb.cxx 

OTHER_OBJS    :=
OTHER_TARGETS :=
OTHER_CLEAN   :=

include $(COMPONENT_REPOSITORY)/pkgconf/makrules.src

