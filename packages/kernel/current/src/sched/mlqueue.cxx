//==========================================================================
//
//	sched/mlqueue.cxx
//
//	Multi-level queue scheduler class implementation
//
//==========================================================================
//####COPYRIGHTBEGIN####
//
// -------------------------------------------
// The contents of this file are subject to the Cygnus eCos Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://sourceware.cygnus.com/ecos
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the
// License for the specific language governing rights and limitations under
// the License.
// 
// The Original Code is eCos - Embedded Cygnus Operating System, released
// September 30, 1998.
// 
// The Initial Developer of the Original Code is Cygnus.  Portions created
// by Cygnus are Copyright (C) 1998 Cygnus Solutions.  All Rights Reserved.
// -------------------------------------------
//
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	nickg
// Contributors:	nickg
// Date:	1997-09-16
// Purpose:	Multilevel queue scheduler class implementation
// Description:	This file contains the implementations of
//              Cyg_Scheduler_Implementation and Cyg_SchedThread_Implementation.
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/kernel.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <cyg/kernel/sched.hxx>        // our header

#include <cyg/hal/hal_arch.h>          // Architecture specific definitions

#include <cyg/kernel/thread.inl>       // thread inlines
#include <cyg/kernel/sched.inl>        // scheduler inlines

#ifdef CYGSEM_KERNEL_SCHED_MLQUEUE

//-------------------------------------------------------------------------
// Some local tracing control - a default.
#ifdef CYGDBG_USE_TRACING
# if !defined( CYGDBG_INFRA_DEBUG_TRACE_ASSERT_SIMPLE ) && \
     !defined( CYGDBG_INFRA_DEBUG_TRACE_ASSERT_FANCY  )
   // ie. not a tracing implementation that takes a long time to output

#  ifndef CYGDBG_KERNEL_TRACE_TIMESLICE
#   define CYGDBG_KERNEL_TRACE_TIMESLICE
#  endif // control not already defined

# endif  // trace implementation not ..._SIMPLE && not ..._FANCY
#endif   // CYGDBG_USE_TRACING

//==========================================================================
// Cyg_Scheduler_Implementation class static members

#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE

cyg_ucount32 Cyg_Scheduler_Implementation::timeslice_count =
                                        CYGNUM_KERNEL_SCHED_TIMESLICE_TICKS;

#endif


//==========================================================================
// Cyg_Scheduler_Implementation class members

// -------------------------------------------------------------------------
// Constructor.

Cyg_Scheduler_Implementation::Cyg_Scheduler_Implementation()
{
    CYG_REPORT_FUNCTION();
        
    queue_map   = 0;
}

// -------------------------------------------------------------------------
// Choose the best thread to run next

Cyg_Thread *Cyg_Scheduler_Implementation::schedule()
{
    CYG_REPORT_FUNCTION();

    // The run queue may _never_ be empty, there is always
    // an idle thread at the lowest priority.

    CYG_ASSERT( queue_map != 0, "Run queue empty");
    CYG_ASSERT( queue_map & (1<<CYG_THREAD_MIN_PRIORITY), "Idle thread vanished!!!");
    CYG_ASSERT( !run_queue[CYG_THREAD_MIN_PRIORITY].empty(), "Idle thread vanished!!!");
    
    register cyg_uint32 index;

    HAL_LSBIT_INDEX(index, queue_map);

    Cyg_Thread *thread = run_queue[index].highpri();

    CYG_ASSERT( thread != NULL , "No threads in run queue");

    return thread;
}

// -------------------------------------------------------------------------

void Cyg_Scheduler_Implementation::add_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();

    cyg_priority pri                            = thread->priority;
    Cyg_ThreadQueue_Implementation *queue       = &run_queue[pri];

    // If the thread is on some other queue, remove it
    // here.
    if( thread->queue != NULL )
    {
        thread->queue->remove(thread);
        thread->queue = NULL;
    }
    
    if( queue->empty() )
    {
        // set the map bit and ask for a reschedule if this is a
        // new highest priority thread.
      
        queue_map |= (1<<pri);

        // If the new thread is higher priority than the
        // current thread, request a reschedule.

        if( pri < Cyg_Scheduler::get_current_thread()->priority )
            need_reschedule = true;
        
    }
    // else the queue already has an occupant, queue behind him

    CYG_ASSERT( queue_map != 0, "Run queue empty");
    CYG_ASSERT( queue_map & (1<<pri), "Queue map bit not set for pri");
    CYG_ASSERT( queue_map & (1<<CYG_THREAD_MIN_PRIORITY), "Idle thread vanished!!!");
//    CYG_ASSERT( !run_queue[CYG_THREAD_MIN_PRIORITY].empty(), "Idle thread vanished!!!");
    
    queue->enqueue(thread);    
}

// -------------------------------------------------------------------------

void Cyg_Scheduler_Implementation::rem_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
        
    CYG_ASSERT( queue_map != 0, "Run queue empty");
      
    cyg_priority pri                            = thread->priority;
    Cyg_ThreadQueue_Implementation *queue       = &run_queue[pri];

    CYG_ASSERT( pri != CYG_THREAD_MIN_PRIORITY, "Idle thread trying to sleep!");
    CYG_ASSERT( queue_map & (1<<pri), "Queue map bit not set for pri");
    CYG_ASSERT( !run_queue[CYG_THREAD_MIN_PRIORITY].empty(), "Idle thread vanished!!!");
    
    // remove thread from queue
    queue->remove(thread);

    if( queue->empty() )
    {
        // If this was only thread in
        // queue, clear map.
      
        queue_map &= ~(1<<pri);
    }

    CYG_ASSERT( queue_map != 0, "Run queue empty");
    CYG_ASSERT( queue_map & (1<<CYG_THREAD_MIN_PRIORITY), "Idle thread vanished!!!");
    CYG_ASSERT( !run_queue[CYG_THREAD_MIN_PRIORITY].empty(), "Idle thread vanished!!!");
}

// -------------------------------------------------------------------------
// register thread with scheduler

void Cyg_Scheduler_Implementation::register_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
        
    // No registration necessary in this scheduler
}

// -------------------------------------------------------------------------

// deregister thread
void Cyg_Scheduler_Implementation::deregister_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
        
    // No registration necessary in this scheduler    
}
    
// -------------------------------------------------------------------------
// Test the given priority for uniqueness

cyg_bool Cyg_Scheduler_Implementation::unique( cyg_priority priority)
{
    CYG_REPORT_FUNCTION();
        
    // Priorities are not unique
    return true;
}

//==========================================================================
// Support for timeslicing option

#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE

void Cyg_Scheduler_Implementation::timeslice()
{
#ifdef CYGDBG_KERNEL_TRACE_TIMESLICE
    CYG_REPORT_FUNCTION();
#endif
    CYG_ASSERT( queue_map != 0, "Run queue empty");
    CYG_ASSERT( queue_map & (1<<CYG_THREAD_MIN_PRIORITY), "Idle thread vanished!!!");
    
    if( --timeslice_count == 0 )
    {
        CYG_INSTRUMENT_SCHED(TIMESLICE,0,0);
#ifdef CYGDBG_KERNEL_TRACE_TIMESLICE
        CYG_TRACE0( true, "quantum consumed, time to reschedule" );
#endif
        // And force the current thread to yield.
        current_thread->yield();
    }

    CYG_ASSERT( queue_map & (1<<CYG_THREAD_MIN_PRIORITY), "Idle thread vanished!!!");
    CYG_ASSERT( !run_queue[CYG_THREAD_MIN_PRIORITY].empty(), "Idle thread vanished!!!");
#ifdef CYGDBG_KERNEL_TRACE_TIMESLICE
    CYG_REPORT_RETURN();
#endif
}

#endif

//==========================================================================
// Cyg_Cyg_SchedThread_Implementation class members

Cyg_SchedThread_Implementation::Cyg_SchedThread_Implementation
(
    CYG_ADDRWORD sched_info
)
{
    CYG_REPORT_FUNCTION();
        
    // Create all threads at maximum priority
    priority = (cyg_priority)sched_info;

    // point the next and prev field at this thread.
    
    next = prev = CYG_CLASSFROMBASE(Cyg_Thread,
                                    Cyg_SchedThread_Implementation,
                                    this);
}

// -------------------------------------------------------------------------
// Insert thread in front of this

void Cyg_SchedThread_Implementation::insert( Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
        
    thread->next        = CYG_CLASSFROMBASE(Cyg_Thread,
                                            Cyg_SchedThread_Implementation,
                                            this);
    thread->prev        = prev;
    prev->next          = thread;
    prev                = thread;    
}

// -------------------------------------------------------------------------
// remove this from queue

void Cyg_SchedThread_Implementation::remove()
{
    CYG_REPORT_FUNCTION();
        
    next->prev          = prev;
    prev->next          = next;
    next = prev         = CYG_CLASSFROMBASE(Cyg_Thread,
                                            Cyg_SchedThread_Implementation,
                                            this);
}

// -------------------------------------------------------------------------
// Yield the processor to another thread

void Cyg_SchedThread_Implementation::yield()
{
    CYG_REPORT_FUNCTION();
        
    // Prevent preemption
    Cyg_Scheduler::lock();

    Cyg_Thread *thread  = CYG_CLASSFROMBASE(Cyg_Thread,
                                            Cyg_SchedThread_Implementation,
                                            this);

    // Only do this if this thread is running. If it is not, there
    // is no point.
    
    if( thread->get_state() == Cyg_Thread::RUNNING )
    {
        // To yield we simply rotate the appropriate
        // run queue to the next thread and reschedule.

        CYG_ASSERTCLASS( thread, "Bad current thread");
    
        Cyg_Scheduler *sched = &Cyg_Scheduler::scheduler;

        CYG_ASSERTCLASS( sched, "Bad scheduler");
    
        cyg_priority pri                            = thread->priority;
        Cyg_ThreadQueue_Implementation *queue       = &sched->run_queue[pri];

        queue->rotate();

        if( queue->highpri() != thread )
            sched->need_reschedule = true;
#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE
            // Reset the timeslice counter so that this thread gets a full
            // quantum. 
        else Cyg_Scheduler::reset_timeslice_count();
#endif
    }
    
    // Unlock the scheduler and switch threads
    Cyg_Scheduler::unlock();

}

// -------------------------------------------------------------------------
// Rotate the run queue at a specified priority.
// (pri is the decider, no this, so the routine is static)

void
Cyg_SchedThread_Implementation::rotate_queue( cyg_priority pri )
{
    CYG_REPORT_FUNCTION();
        
    // Prevent preemption
    Cyg_Scheduler::lock();

    Cyg_Scheduler *sched = &Cyg_Scheduler::scheduler;

    CYG_ASSERTCLASS( sched, "Bad scheduler");
    
    Cyg_ThreadQueue_Implementation *queue = &sched->run_queue[pri];

    if ( !queue->empty() ) {
        queue->rotate();
        sched->need_reschedule = true;
    }

    // Unlock the scheduler and switch threads
    Cyg_Scheduler::unlock();

}

//==========================================================================
// Cyg_ThreadQueue_Implementation class members

Cyg_ThreadQueue_Implementation::Cyg_ThreadQueue_Implementation()
{
    CYG_REPORT_FUNCTION();
        
    queue = NULL;                       // empty queue
}

        

void Cyg_ThreadQueue_Implementation::enqueue(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();

    if( queue == NULL ) queue = thread;
    else queue->insert(thread);
    
    thread->queue = CYG_CLASSFROMBASE(Cyg_ThreadQueue,
                                      Cyg_ThreadQueue_Implementation,
                                      this);

}

// -------------------------------------------------------------------------

Cyg_Thread *Cyg_ThreadQueue_Implementation::dequeue()
{
    CYG_REPORT_FUNCTION();
        
    if( queue == NULL ) return NULL;
    
    Cyg_Thread *thread = queue;
    
    if( thread->next == thread )
    {
        // sole thread on list, NULL out ptr
        queue = NULL;
    }
    else
    {
        // advance to next and remove thread
        queue = thread->next;
        thread->remove();
    }

    thread->queue = NULL;

    return thread;
}

// -------------------------------------------------------------------------

Cyg_Thread *Cyg_ThreadQueue_Implementation::highpri()
{
    CYG_REPORT_FUNCTION();
        
    return queue;
}

// -------------------------------------------------------------------------

void Cyg_ThreadQueue_Implementation::remove(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
        
    // If the thread we want it the at the head
    // of the list, and is on its own, clear the
    // list and return. Otherwise advance to the
    // next thread and remove ours. If the thread
    // is not at the head of the list, just dequeue
    // it.

    thread->queue = NULL;
    
    if( queue == thread )
    {
        if( thread->next == thread )
        {
            queue = NULL;
            return;
        }
        else queue = thread->next;
    }

    thread->Cyg_SchedThread_Implementation::remove();

}

// -------------------------------------------------------------------------
// Rotate the front thread on the queue to the back.

void Cyg_ThreadQueue_Implementation::rotate()
{
    CYG_REPORT_FUNCTION();
        
    queue = queue->next;
}

// -------------------------------------------------------------------------

#endif

// -------------------------------------------------------------------------
// EOF sched/mlqueue.cxx
