//==========================================================================
//
//      sync/bin_sem.cxx
//
//      Binary semaphore implementation
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
// Date:        1997-09-24
// Purpose:     Cyg_Binary_Semaphore implementation
// Description: This file contains the implementations of the binary semaphore
//              class.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/kernel.h>

#include <cyg/kernel/ktypes.h>             // base kernel types
#include <cyg/infra/cyg_trac.h>            // tracing macros
#include <cyg/infra/cyg_ass.h>             // assertion macros
#include <cyg/kernel/instrmnt.h>           // instrumentation

#include <cyg/kernel/thread.inl>           // Cyg_Thread inlines

#include <cyg/kernel/sema.hxx>             // our header

#include <cyg/kernel/sched.inl>            // scheduler inlines

// -------------------------------------------------------------------------

Cyg_Binary_Semaphore::Cyg_Binary_Semaphore (
    cyg_bool    init_state
)
{
    state       = init_state;
}

// -------------------------------------------------------------------------

Cyg_Binary_Semaphore::~Cyg_Binary_Semaphore ( )
{
}

// -------------------------------------------------------------------------

cyg_bool Cyg_Binary_Semaphore::wait()
{
    cyg_bool result = true;
    Cyg_Thread *self = Cyg_Thread::self();
    
    // Prevent preemption
    Cyg_Scheduler::lock();

    CYG_INSTRUMENT_BINSEM( CLAIM, this, state );
        
    while( !state && result )
    {
        self->set_sleep_reason( Cyg_Thread::WAIT );
        
        self->sleep();
        
        queue.enqueue( self );

        CYG_INSTRUMENT_BINSEM( WAIT, this, 0 );

        // Allow other threads to run
        Cyg_Scheduler::reschedule();

        CYG_INSTRUMENT_BINSEM( WOKE, this, state );

        switch( self->get_wake_reason() )
        {
        case Cyg_Thread::DESTRUCT:
        case Cyg_Thread::BREAK:
            result = false;
            break;
            
        case Cyg_Thread::EXIT:            
            self->exit();
            break;

        default:
            break;
        }
        
    }

    if( result ) state = false;

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();

    return result;
}

// -------------------------------------------------------------------------

cyg_bool Cyg_Binary_Semaphore::trywait()
{
    cyg_bool result = true;
    
    // Prevent preemption
    Cyg_Scheduler::lock();

    if( state ) state = false;
    else        result = false;
    
    CYG_INSTRUMENT_BINSEM( TRY, this, result );
    
    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
    
    return result;
}

// -------------------------------------------------------------------------

void Cyg_Binary_Semaphore::post()
{
    // Prevent preemption
    Cyg_Scheduler::lock();

    CYG_INSTRUMENT_BINSEM( POST, this, 0 );
        
    state = true;
        
    if( !queue.empty() ) {

        // The queue is non-empty, so grab the next
        // thread from it and wake it up. The waiter
        // will clear the flag.

        Cyg_Thread *thread = queue.dequeue();

        thread->set_wake_reason( Cyg_Thread::DONE );
        
        thread->wake();

        CYG_INSTRUMENT_BINSEM( WAKE, this, thread );
    }
    
    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
    
}

// -------------------------------------------------------------------------

cyg_bool Cyg_Binary_Semaphore::posted()
{
    // This is a single read of the value of state.
    // This is already atomic, hence there is no need
    // to lock the scheduler.
    
    return state;
}

// -------------------------------------------------------------------------
// EOF sync/bin_sem.cxx
