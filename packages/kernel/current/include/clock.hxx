#ifndef CYGONCE_KERNEL_CLOCK_HXX
#define CYGONCE_KERNEL_CLOCK_HXX

//==========================================================================
//
//      clock.hxx
//
//      Clock and Alarm class declaration(s)
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
// Author(s):   nickg
// Contributors:        nickg
// Date:        1997-09-09
// Purpose:     Define Clock and Alarm class interfaces
// Description: The classes defined here collectively implement the
//              internal API used to create, configure and manage Counters,
//              Clocks and Alarms.
// Usage:       #include <cyg/kernel/clock.hxx>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>            // assertion macros

// -------------------------------------------------------------------------

class Cyg_Alarm;

typedef void cyg_alarm_fn(Cyg_Alarm *alarm, CYG_ADDRWORD data);

// -------------------------------------------------------------------------
// Counter object.

class Cyg_Counter
{

    friend class Cyg_Alarm;

#if defined(CYGIMP_KERNEL_COUNTERS_SINGLE_LIST)

    Cyg_Alarm           *alarm_list;    // Linear list of Alarms

#elif defined(CYGIMP_KERNEL_COUNTERS_MULTI_LIST)

    Cyg_Alarm           *alarm_list[CYGNUM_KERNEL_COUNTERS_MULTI_LIST_SIZE];
    
#endif

    volatile cyg_tick_count counter;    // counter value

    cyg_uint32          increment;      // increment per tick
    
public:

    CYGDBG_DEFINE_CHECK_THIS
    
    Cyg_Counter(
        cyg_uint32      increment = 1
    );

    ~Cyg_Counter();
    
    // Return current value of counter
    cyg_tick_count current_value();

    // Return low and high halves of the
    // counter value.
    cyg_uint32 current_value_lo();
    cyg_uint32 current_value_hi();
    
    // Set the counter's current value
    void set_value( cyg_tick_count new_value);
        
    // Advance counter by some number of ticks
    void tick( cyg_uint32 ticks = 1);

    // Add an alarm to this counter
    void add_alarm( Cyg_Alarm *alarm );

    // Remove an alarm from this counter
    void rem_alarm( Cyg_Alarm *alarm );

};

// -------------------------------------------------------------------------
// Clock class. This is derived from a Counter and defines extra
// features to support clock-like behaviour.

class Cyg_Clock
    : public Cyg_Counter
{

public:

    CYGDBG_DEFINE_CHECK_THIS

    // This structure allows a more accurate representation
    // of the resolution than a single integer would allow.
    // The resolution is defined as dividend/divisor nanoseconds
    // per tick.
    struct cyg_resolution {
        cyg_uint32  dividend;
        cyg_uint32  divisor;
    };

private:

    cyg_resolution      resolution;     // Current clock resolution

public:

    Cyg_Clock(                          // Create clock with given resolution
        cyg_resolution resolution
        );

    ~Cyg_Clock();                       // Destructor
        
    cyg_resolution get_resolution();    // Return current resolution

    void set_resolution(                // Set new resolution
        cyg_resolution resolution
        ); 

    // There is a need for converting from "other" ticks to clock ticks.
    // We will construct 4 numbers to do the conversion as:
    //   clock_ticks = (((otherticks*mul1)/div1)*mul2/div2)
    // with the values chosen to minimize the possibility of overflow.
    // Do the arithmetic in cyg_uint64s throughout.
    struct converter {
        cyg_uint64 mul1, div1, mul2, div2;
    };

    // There are two of these because the 4 numbers are different depending
    // on the direction of the conversion, to prevent loss of significance.
    // NB these relate to the resolution of the clock object they are
    // called against, not necessarily "the" system real time clock.
    void get_other_to_clock_converter( cyg_uint64 ns_per_other_tick,
                                       struct converter *pcc );

    void get_clock_to_other_converter( cyg_uint64 ns_per_other_tick,
                                       struct converter *pcc );

    // A utility to perform the conversion in the obvious way, with
    // rounding to nearest at each stage.  Static because it uses a
    // previously acquired converter.
    static cyg_tick_count convert( cyg_tick_count value,
                                   struct converter *pcc );
        
#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK 
    
    // There is a system supplied real time clock...

    static Cyg_Clock *real_time_clock;

#endif    
        
};

// -------------------------------------------------------------------------
// Alarm class. An alarm may be attached to a counter (or a clock) to be
// called when the trigger value is reached.

class Cyg_Alarm
{
    friend class Cyg_Counter;
    
#if defined(CYGIMP_KERNEL_COUNTERS_SINGLE_LIST) || defined(CYGIMP_KERNEL_COUNTERS_MULTI_LIST)

    Cyg_Alarm           *next;          // next alarm in list
    
#endif

protected:
    Cyg_Counter         *counter;       // Attached to this counter/clock

    cyg_alarm_fn        *alarm;         // Call-back function

    CYG_ADDRWORD        data;           // Call-back data

    cyg_tick_count      trigger;        // Absolute trigger time

    cyg_tick_count      interval;       // Retrigger interval

    cyg_bool            enabled;        // True if enabled

    Cyg_Alarm();

    void synchronize( void );           // deal with times in the past,
                                        // make next alarm in synch.
    
public:

    CYGDBG_DEFINE_CHECK_THIS
    
    Cyg_Alarm                           // Constructor
    (
        Cyg_Counter     *counter,       // Attached to this counter
        cyg_alarm_fn    *alarm,         // Call-back function
        CYG_ADDRWORD    data            // Call-back data
        );

    ~Cyg_Alarm();                       // Destructor
        
    void initialize(                    // Initialize Alarm
        cyg_tick_count    trigger,      // Absolute trigger time
        cyg_tick_count    interval = 0  // Relative retrigger interval
        );

    void enable();                      // Ensure alarm enabled

    void disable();                     // Ensure alarm disabled
    
    void get_times(
        cyg_tick_count  *trigger,       // Next trigger time
        cyg_tick_count  *interval       // Current interval
        );
};

// -------------------------------------------------------------------------

#endif // ifndef CYGONCE_KERNEL_CLOCK_HXX
// EOF clock.hxx
