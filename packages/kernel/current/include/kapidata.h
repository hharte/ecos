#ifndef CYGONCE_KERNEL_KAPIDATA_H
#define CYGONCE_KERNEL_KAPIDATA_H

/*=============================================================================
//
//      kapidata.h
//
//      Native API data structures
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
// by Cygnus are Copyright (C) 1998,1999 Cygnus Solutions.  All Rights Reserved.
// -------------------------------------------
//
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg
// Contributors:        nickg
// Date:        1998-03-13
// Purpose:     Native API data structures
// Description: This file defines the structures used in the native API. The
//              sizes of these structures are dependent on the system
//              configuration and must be kept in step with their real
//              counterparts in the C++ headers.
// Usage:       included by kapi.h
//
//####DESCRIPTIONEND####
//
//==========================================================================*/

#include <pkgconf/kernel.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_intr.h>           // exception defines

/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/

#ifndef CYGNUM_KERNEL_SCHED_BITMAP_SIZE
#if defined(CYGSEM_KERNEL_SCHED_MLQUEUE)
#define CYGNUM_KERNEL_SCHED_BITMAP_SIZE 32
#elif defined(CYGSEM_KERNEL_SCHED_BITMAP)
#define CYGNUM_KERNEL_SCHED_BITMAP_SIZE 32
#endif
#endif

#if CYGNUM_KERNEL_SCHED_BITMAP_SIZE <= 8
typedef cyg_ucount8 cyg_sched_bitmap;
#elif CYGNUM_KERNEL_SCHED_BITMAP_SIZE <= 16
typedef cyg_ucount16 cyg_sched_bitmap;
#elif CYGNUM_KERNEL_SCHED_BITMAP_SIZE <= 32
typedef cyg_ucount32 cyg_sched_bitmap;
#elif CYGNUM_KERNEL_SCHED_BITMAP_SIZE <= 64
typedef cyg_ucount64 cyg_sched_bitmap;
#else
#error Bitmaps greater than 64 bits not currently allowed
#endif

typedef struct 
{
#if defined(CYGSEM_KERNEL_SCHED_BITMAP)

    cyg_sched_bitmap map;
    
#elif defined(CYGSEM_KERNEL_SCHED_MLQUEUE)

    cyg_thread *queue;

#elif defined(CYGSEM_KERNEL_SCHED_LOTTERY)

    cyg_thread *queue;

#else

#error Undefined scheduler type
    
#endif    
} cyg_threadqueue;
    
/*---------------------------------------------------------------------------*/

struct cyg_interrupt
{
    cyg_vector_t        vector;
    cyg_priority_t      priority;
    cyg_ISR_t           *isr;
    cyg_DSR_t           *dsr;
    CYG_ADDRWORD        data;

#ifdef CYGIMP_KERNEL_INTERRUPTS_DSRS_LIST
    cyg_ucount32        dsr_count;
    cyg_interrupt       *next_dsr;
#endif
#ifdef CYGIMP_KERNEL_INTERRUPTS_CHAIN
    cyg_interrupt       *next;
#endif
};


/*---------------------------------------------------------------------------*/

struct cyg_counter
{
#if defined(CYGIMP_KERNEL_COUNTERS_SINGLE_LIST)
    cyg_alarm           *alarm_list;
#elif defined(CYGIMP_KERNEL_COUNTERS_MULTI_LIST)
    cyg_alarm           *alarm_list[CYGNUM_KERNEL_COUNTERS_MULTI_LIST_SIZE];
#endif
    cyg_tick_count_t    counter;
    cyg_uint32          increment;
};

/*---------------------------------------------------------------------------*/

struct cyg_clock
{
    cyg_counter         counter;
    cyg_resolution_t    resolution;
};

/*---------------------------------------------------------------------------*/

struct cyg_alarm
{
#if defined(CYGIMP_KERNEL_COUNTERS_SINGLE_LIST) || defined(CYGIMP_KERNEL_COUNTERS_MULTI_LIST)
    cyg_alarm           *next;    
#endif
    cyg_counter         *counter;
    cyg_alarm_t         *alarm;
    CYG_ADDRWORD        data;
    cyg_tick_count_t    trigger;
    cyg_tick_count_t    interval;
    cyg_bool            enabled;
};

/*---------------------------------------------------------------------------*/
/* Exception controller                                                      */

#ifdef CYGPKG_KERNEL_EXCEPTIONS
typedef struct
{
#ifdef CYGSEM_KERNEL_EXCEPTIONS_DECODE
    cyg_exception_handler_t *exception_handler[CYGNUM_HAL_EXCEPTION_COUNT];
    
    CYG_ADDRWORD            exception_data[CYGNUM_HAL_EXCEPTION_COUNT];
#else
    cyg_exception_handler_t *exception_handler; // Handler function
    
    CYG_ADDRWORD            exception_data;     // Handler data
#endif
    
} cyg_exception_control;

#endif

/*---------------------------------------------------------------------------*/
/* Thread structure                                                          */

typedef struct
{
    CYG_ADDRESS         stack_base;     /* pointer to base of stack area */
    cyg_uint32          stack_size;     /* size of stack area in bytes */
#ifdef CYGFUN_KERNEL_THREADS_STACK_LIMIT
    CYG_ADDRESS         stack_limit;    /* movable stack limit */
#endif    
    CYG_ADDRESS         stack_ptr;      /* pointer to saved state on stack */
    CYG_ADDRWORD        entry_point;    /* main entry point (code pointer!) */
    CYG_ADDRWORD        entry_data;     /* entry point argument */
#ifdef CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT
    void                *saved_context; // If non-zero, this points at a more
                                        // interesting context than stack_ptr.
#endif
    
} cyg_hardwarethread;
    
typedef struct 
{
#if defined(CYGSEM_KERNEL_SCHED_BITMAP)

    cyg_priority_t      priority;       /* current thread priority */
    
#elif defined(CYGSEM_KERNEL_SCHED_MLQUEUE)

    cyg_thread *next;
    cyg_thread *prev;

    cyg_priority_t      priority;       /* current thread priority */
    
#elif defined(CYGSEM_KERNEL_SCHED_LOTTERY)

    cyg_thread *next;
    cyg_thread *prev;

    cyg_priority_t      priority;       /* current thread priority */

    cyg_priority_t      compensation_tickets;   /* sleep compensation */
    
#else

#error Undefined scheduler type
    
#endif    

    cyg_threadqueue     *queue;

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INHERITANCE
    cyg_count32         mutex_count;
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INHERITANCE_SIMPLE
    cyg_priority_t      original_priority;
    cyg_bool            priority_inherited;
#endif
#endif

} cyg_schedthread;

typedef struct 
{
    cyg_alarm           alarm;
    cyg_thread          *thread;
} cyg_threadtimer;


typedef int cyg_reason_t; /* cyg_reason is originally an enum */

struct cyg_thread
{
    cyg_hardwarethread  hwthread;
    cyg_schedthread     schedthread;

    cyg_uint32                  state;      
    cyg_ucount32                suspend_count;
    cyg_ucount32                wakeup_count;
    CYG_ADDRWORD                wait_info;
    cyg_uint16                  unique_id;
    

#if defined(CYGPKG_KERNEL_EXCEPTIONS) && !defined(CYGSEM_KERNEL_EXCEPTIONS_GLOBAL)
    
    cyg_exception_control       exception_control;

#endif

#ifdef CYGFUN_KERNEL_THREADS_TIMER
    cyg_threadtimer     timer;
#endif

    cyg_reason_t        sleep_reason;
    cyg_reason_t        wake_reason;

#ifdef CYGVAR_KERNEL_THREADS_DATA

    CYG_ADDRWORD        thread_data[CYGNUM_KERNEL_THREADS_DATA_MAX];

#endif

#ifdef CYGVAR_KERNEL_THREADS_NAME

    char                *name;

#endif

#ifdef CYGVAR_KERNEL_THREADS_LIST

    cyg_thread          *list_next;
    
#endif
    
};

/*---------------------------------------------------------------------------*/

struct cyg_mbox
{
    cyg_count32         base;           /* index of first used slot          */
    cyg_count32         count;          /* count of used slots               */
    cyg_threadqueue     get_threadq;    /* Queue of waiting threads          */
#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
    cyg_threadqueue     put_threadq;    /* Queue of waiting threads          */
#endif
    void *              itemqueue[ CYGNUM_KERNEL_SYNCH_MBOX_QUEUE_SIZE ];
};

/*---------------------------------------------------------------------------*/

struct cyg_sem_t
{
    cyg_count32         count;          /* The semaphore count          */
    cyg_threadqueue     queue;          /* Queue of waiting threads     */    
};

/*---------------------------------------------------------------------------*/

struct cyg_flag_t
{
    cyg_flag_value_t    value;          /* The flag value               */
    cyg_threadqueue     queue;          /* Queue of waiting threads     */    
};

/*---------------------------------------------------------------------------*/

struct cyg_mutex_t
{
    cyg_bool            locked;         /* true if locked               */
    cyg_thread          *owner;         /* Current locking thread       */
    cyg_threadqueue     queue;          /* Queue of waiting threads     */
    
};

/*---------------------------------------------------------------------------*/

struct cyg_cond_t
{
    cyg_mutex_t         *mutex;         /* Associated mutex             */
    cyg_threadqueue     queue;          /* Queue of waiting threads     */
};

/*---------------------------------------------------------------------------*/

/* This corresponds to the extra fields provided by the mempoolt template
   not the actual size of the template in any given instance. */
typedef struct cyg_mempoolt {
    cyg_threadqueue queue;
} cyg_mempoolt;


struct cyg_mempool_var_memdq {
    struct cyg_mempool_var_memdq *prev, *next;
    cyg_int32 size;
};

struct cyg_mempool_var {
    struct cyg_mempool_var_memdq head;
    cyg_uint8  *obase;
    cyg_int32  osize;
    cyg_int32  oalign;
    cyg_uint8  *bottom;
    cyg_uint8  *top;
    cyg_int32  alignment;
    cyg_int32  freemem;
    cyg_mempoolt mempoolt;
};

struct cyg_mempool_fix {
    cyg_uint32 *bitmap;
    cyg_int32 maptop;
    cyg_uint8  *mempool;
    cyg_int32 numblocks;
    cyg_int32 freeblocks;
    cyg_int32 blocksize;
    cyg_int32 firstfree;
    cyg_uint8  *top;
    cyg_mempoolt mempoolt;
};

/*------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/*---------------------------------------------------------------------------*/
/* EOF kapidata.h                                                            */
#endif /* CYGONCE_KERNEL_KAPIDATA_H */
