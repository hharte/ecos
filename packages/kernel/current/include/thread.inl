#ifndef CYGONCE_KERNEL_THREAD_INL
#define CYGONCE_KERNEL_THREAD_INL

//==========================================================================
//
//      thread.inl
//
//      Thread class inlines
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
// Author(s):   nickg
// Contributors:        nickg
// Date:        1997-09-09
// Purpose:     Define inlines for thread classes
// Description: Inline implementations of various member functions defined
//              in various Thread classes. 
// Usage:
//              #include <cyg/kernel/thread.hxx>
//              ...
//              #include <cyg/kernel/thread.inl>
//              ...

//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/thread.hxx>
#include <cyg/hal/hal_arch.h>

#include <cyg/kernel/clock.inl>

//==========================================================================
// Inlines for Cyg_HardwareThread

// -------------------------------------------------------------------------
// Attach a stack to this thread. If there is a HAL defined macro to
// do this, then we use that, otherwise assume a falling stack.
inline void Cyg_HardwareThread::attach_stack(CYG_ADDRESS s_base, cyg_uint32 s_size)
{
#ifdef CYGNUM_HAL_STACK_SIZE_MINIMUM
    CYG_ASSERT( s_size >= CYGNUM_HAL_STACK_SIZE_MINIMUM,
                "Stack size too small");
#endif
    stack_base = s_base;
    stack_size = s_size;
#ifdef CYGFUN_KERNEL_THREADS_STACK_LIMIT
    stack_limit = s_base;
#endif
    
#ifdef HAL_THREAD_ATTACH_STACK

    HAL_THREAD_ATTACH_STACK(stack_ptr, stack_base, stack_size);
    
#else

    stack_ptr = stack_base + stack_size;

#endif
}

// -------------------------------------------------------------------------

inline Cyg_HardwareThread::Cyg_HardwareThread(
    cyg_thread_entry        *e_point,   // entry point function
    CYG_ADDRWORD            e_data,     // entry data
    cyg_ucount32            s_size,     // stack size, 0 = use default
    CYG_ADDRESS             s_base      // stack base, NULL = allocate
)
{
    entry_point = e_point;
    entry_data  = e_data;
#ifdef CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT
    saved_context = 0;
#endif
    
    attach_stack( s_base, s_size );
};

// -------------------------------------------------------------------------
// get the size/base of this thread's stack

inline CYG_ADDRESS
Cyg_HardwareThread::get_stack_base()
{
    return stack_base;
}

inline cyg_uint32
Cyg_HardwareThread::get_stack_size()
{
    return stack_size;
}

// -------------------------------------------------------------------------

#ifdef CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT

// Return the current saved state for this thread.
inline HAL_SavedRegisters *Cyg_HardwareThread::get_saved_context()
{
    HAL_SavedRegisters *regs;
    if( saved_context != 0 ) regs = saved_context;
    else HAL_THREAD_GET_SAVED_REGISTERS( stack_ptr, regs );
    return regs;
}

inline void Cyg_HardwareThread::set_saved_context(HAL_SavedRegisters *ctx)
{
    saved_context = ctx;
}

#endif

// -------------------------------------------------------------------------
// (declare this inline before its first use)

inline cyg_uint16 Cyg_Thread::get_unique_id()
{
    return unique_id;
}

// -------------------------------------------------------------------------
// Initialize the context of this thread.

inline void Cyg_HardwareThread::init_context(Cyg_Thread *thread)
{
#ifdef CYGPKG_INFRA_DEBUG
    cyg_uint32 threadid = thread->get_unique_id()*0x01010000;
#else
    cyg_uint32 threadid = 0x11110000;
#endif
    HAL_THREAD_INIT_CONTEXT( stack_ptr, thread, thread_entry, threadid );
}



// -------------------------------------------------------------------------
// Load thread context without saving current context.
// This function is only really here for completeness, the
// kernel generally calls the HAL macros directly.

inline void Cyg_HardwareThread::load_context()
{
    CYG_ADDRESS dummy_stack_ptr;
    
    HAL_THREAD_SWITCH_CONTEXT( &dummy_stack_ptr, &stack_ptr );
}

// -------------------------------------------------------------------------
// Save current thread's context and load that of the given next thread.
// This function is only really here for completeness, the
// kernel generally calls the HAL macros directly.

inline void Cyg_HardwareThread::switch_context(Cyg_HardwareThread *next)
{
    HAL_THREAD_SWITCH_CONTEXT( &stack_ptr, &next->stack_ptr );
}

// -------------------------------------------------------------------------
// Get and set entry_data.

inline void Cyg_HardwareThread::set_entry_data( CYG_ADDRWORD data )
{
    entry_data = data;
}

inline CYG_ADDRWORD Cyg_HardwareThread::get_entry_data()
{
    return entry_data;
}

// -------------------------------------------------------------------------
// Allocate some memory at the lower end of the stack
// by moving the stack limit pointer.

#ifdef CYGFUN_KERNEL_THREADS_STACK_LIMIT

inline void *Cyg_HardwareThread::increment_stack_limit( cyg_ucount32 size)
{
    stack_limit += size;
    return (void *)(stack_limit - size);
}

    
inline CYG_ADDRESS
Cyg_HardwareThread::get_stack_limit()
{
    return stack_limit;
}

#endif    

//==========================================================================
// Inlines for Cyg_Thread class

inline Cyg_Thread *Cyg_Thread::self()
{
    return Cyg_Scheduler::get_current_thread();
}

// -------------------------------------------------------------------------

inline void Cyg_Thread::yield()
{
    self()->Cyg_SchedThread::yield();
}

// -------------------------------------------------------------------------

inline void
Cyg_Thread::rotate_queue( cyg_priority pri )
{
    self()->Cyg_SchedThread::rotate_queue( pri );
}

// -------------------------------------------------------------------------

inline void
Cyg_Thread::to_queue_head( void )
{
    this->Cyg_SchedThread::to_queue_head();
}

// -------------------------------------------------------------------------

#ifdef CYGIMP_THREAD_PRIORITY

inline cyg_priority Cyg_Thread::get_priority()
{
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INHERITANCE_SIMPLE

    // If we have an inherited priority, return our original
    // priority rather than the current one.
    
    if( priority_inherited ) return original_priority;

#endif

    return priority;
}

// Return the actual dispatching priority of the thread
// regardless of inheritance or scheduling concerns.
inline cyg_priority Cyg_Thread::get_current_priority()
{
    return priority;
}

#endif

// -------------------------------------------------------------------------

inline void Cyg_Thread::set_sleep_reason( cyg_reason reason)
{
    self()->sleep_reason = reason;
    self()->wake_reason = NONE;
}

// -------------------------------------------------------------------------

inline Cyg_Thread::cyg_reason Cyg_Thread::get_sleep_reason()
{
    return sleep_reason;
}

// -------------------------------------------------------------------------

inline void Cyg_Thread::set_wake_reason( cyg_reason reason )
{
    sleep_reason = NONE;
    wake_reason = reason;
}

// -------------------------------------------------------------------------

inline Cyg_Thread::cyg_reason Cyg_Thread::get_wake_reason()
{
    return wake_reason;
}

// -------------------------------------------------------------------------

inline void Cyg_Thread::set_timer(
    cyg_tick_count      trigger,
    cyg_reason          reason
)
{
#ifdef CYGFUN_KERNEL_THREADS_TIMER
    self()->sleep_reason = reason;
    self()->wake_reason = NONE;
    self()->timer.initialize( trigger);
#endif
}

// -------------------------------------------------------------------------

inline void Cyg_Thread::clear_timer()
{
#ifdef CYGFUN_KERNEL_THREADS_TIMER
    self()->timer.disable();
#endif
}

// -------------------------------------------------------------------------

#ifdef CYGVAR_KERNEL_THREADS_DATA

inline CYG_ADDRWORD Cyg_Thread::get_data( cyg_ucount32 index )
{
    CYG_ASSERT( index < CYGNUM_KERNEL_THREADS_DATA_MAX,
                "Per thread data index out of bounds");
    CYG_ASSERT( (thread_data_map & (1<<index)) == 0,
                "Unallocated index used");
    
    return self()->thread_data[index];
}

inline CYG_ADDRWORD *Cyg_Thread::get_data_ptr( cyg_ucount32 index )
{
    CYG_ASSERT( index < CYGNUM_KERNEL_THREADS_DATA_MAX,
                "Per thread data index out of bounds");
    CYG_ASSERT( (thread_data_map & (1<<index)) == 0,
                "Unallocated index used");
    
    return &(self()->thread_data[index]);
}

inline void Cyg_Thread::set_data( cyg_ucount32 index, CYG_ADDRWORD data )
{
    CYG_ASSERT( index < CYGNUM_KERNEL_THREADS_DATA_MAX,
                "Per thread data index out of bounds");
    CYG_ASSERT( (thread_data_map & (1<<index)) == 0,
                "Unallocated index used");

    thread_data[index] = data;
}

#endif

// -------------------------------------------------------------------------

#ifdef CYGVAR_KERNEL_THREADS_NAME

inline char *Cyg_Thread::get_name()
{
    return name;
}

#endif

// -------------------------------------------------------------------------

#ifdef CYGVAR_KERNEL_THREADS_LIST

inline Cyg_Thread *Cyg_Thread::get_list_head()
{
    return thread_list?thread_list->list_next:0;
}
    
inline Cyg_Thread *Cyg_Thread::get_list_next()
{
    return (this==thread_list)?0:list_next;
}

#endif


// -------------------------------------------------------------------------

#ifdef CYGPKG_KERNEL_EXCEPTIONS

inline void Cyg_Thread::register_exception(
    cyg_code                exception_number,       // exception number
    cyg_exception_handler   handler,                // handler function
    CYG_ADDRWORD            data,                   // data argument
    cyg_exception_handler   **old_handler,          // handler function
    CYG_ADDRWORD            *old_data               // data argument
    )
{
    self()->exception_control.register_exception(
        exception_number,
        handler,
        data,
        old_handler,
        old_data
        );
}

inline void Cyg_Thread::deregister_exception(
    cyg_code                exception_number        // exception number
    )
{
    self()->exception_control.deregister_exception(
        exception_number
        );
}

#endif

//==========================================================================
// Inlines for Cyg_ThreadTimer class

// -------------------------------------------------------------------------
#if defined(CYGFUN_KERNEL_THREADS_TIMER) && defined(CYGVAR_KERNEL_COUNTERS_CLOCK)

inline Cyg_ThreadTimer::Cyg_ThreadTimer(
    Cyg_Thread  *th
    )
    : Cyg_Alarm(Cyg_Clock::real_time_clock,
                &alarm,
                CYG_ADDRWORD(this)
                )
{
    thread = th;
}

#endif

//==========================================================================
// Inlines for Cyg_ThreadQueue class


inline void Cyg_ThreadQueue::enqueue(Cyg_Thread *thread)
{
    Cyg_ThreadQueue_Implementation::enqueue(thread);
}

// -------------------------------------------------------------------------

inline Cyg_Thread *Cyg_ThreadQueue::highpri()
{
    return Cyg_ThreadQueue_Implementation::highpri();
}

// -------------------------------------------------------------------------

inline Cyg_Thread *Cyg_ThreadQueue::dequeue()
{
    return Cyg_ThreadQueue_Implementation::dequeue();
}

// -------------------------------------------------------------------------

inline void Cyg_ThreadQueue::remove(Cyg_Thread *thread)
{
    Cyg_ThreadQueue_Implementation::remove(thread);
}

// -------------------------------------------------------------------------

inline cyg_bool Cyg_ThreadQueue::empty()
{
    return Cyg_ThreadQueue_Implementation::empty();
}

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_THREAD_INL
// EOF thread.inl
