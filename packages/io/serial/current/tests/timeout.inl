#ifndef CYGONCE_IO_SERIAL_MISC_TIMEOUT_INL
#define CYGONCE_IO_SERIAL_MISC_TIMEOUT_INL
//==========================================================================
//
//        timeout.inl
//
//        Simple timeout support for serial I/O testing.
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.0 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://sourceware.cygnus.com/ecos                                        
//                                                                          
// Software distributed under the License is distributed on an       
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
// Author(s):     gthomas
// Contributors:  gthomas
// Date:          1999-02-05
// Description:   Simple timeout functions
//####DESCRIPTIONEND####

// Timeout support

typedef void (timeout_fun)(void *);
#ifndef NTIMEOUTS
#define NTIMEOUTS 8
#endif
typedef struct {
    cyg_int32     delta;  // Number of "ticks" in the future for this timeout
    timeout_fun  *fun;    // Function to execute when it expires
    void         *arg;    // Argument to pass when it does
} timeout_entry;
static timeout_entry timeouts[NTIMEOUTS];
static cyg_handle_t timeout_alarm_handle;
static cyg_alarm timeout_alarm;
static cyg_int32 last_delta;

static void
do_timeout(cyg_handle_t alarm, cyg_addrword_t data)
{
    int i;
    cyg_int32 min_delta;
    timeout_entry *e = timeouts;
    min_delta = 0x7FFFFFFF;  // Maxint
    for (i = 0;  i < NTIMEOUTS;  i++, e++) {
        if (e->delta) {
            e->delta -= last_delta;
            if (e->delta == 0) {
                // Time for this item to 'fire'
                (e->fun)(e->arg);
            } else {
                if (e->delta < min_delta) min_delta = e->delta;
            }
        }
    }
    if (min_delta != 0x7FFFFFFF) {
        // Still something to do, schedule it
        cyg_alarm_initialize(timeout_alarm_handle, cyg_current_time()+min_delta, 0);
        last_delta = min_delta;
    }
}

static cyg_uint32
timeout(cyg_int32 delta, timeout_fun *fun, void *arg)
{
    int i;
    cyg_int32 min_delta;
    static bool init = false;
    timeout_entry *e = timeouts;
    cyg_uint32 stamp;
    if (!init) {
        cyg_handle_t h;
        cyg_clock_to_counter(cyg_real_time_clock(), &h);
        cyg_alarm_create(h, do_timeout, 0, &timeout_alarm_handle, &timeout_alarm);
        init = true;
    }
    stamp = 0;  // Assume no slots available
    for (i = 0;  i < NTIMEOUTS;  i++, e++) {
        if ((e->delta == 0) && (e->fun == 0)) {
            // Free entry
            e->delta = delta;
            e->fun = fun;
            e->arg = arg;
            stamp = (cyg_uint32)e;
            break;
        }
    }
    e = timeouts;
    min_delta = 0x7FFFFFFF;
    for (i = 0;  i < NTIMEOUTS;  i++, e++) {
        if (e->delta && (e->delta < min_delta)) min_delta = e->delta;
    }
    if (min_delta != 0x7FFFFFFF) {
        // Still something to do, schedule it
        cyg_alarm_initialize(timeout_alarm_handle, cyg_current_time()+min_delta, 0);
        last_delta = min_delta;
    }
    return stamp;
}

static void
untimeout(cyg_uint32 stamp)
{
    if (stamp != 0) {
        timeout_entry *e = (timeout_entry *)stamp;
        if (e->fun != 0) {
            e->delta = 0;
            e->fun = 0;
            e->arg = 0;
        }
    }
}

#endif // CYGONCE_IO_SERIAL_MISC_TIMEOUT_INL
