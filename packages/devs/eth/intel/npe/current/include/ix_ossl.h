/**
 * ============================================================================
 * = COPYRIGHT
 * -- Intel Copyright Notice --
 * INTEL CONFIDENTIAL
 *
 * Copyright 2002 Intel Corporation All Rights Reserved.
 *
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.  The Material
 * contains trade secrets and proprietary and confidential information of
 * Intel or its suppliers and licensors.  The Material is protected by
 * worldwide copyright and trade secret laws and treaty provisions. No
 * part of the Material may be used, copied, reproduced, modified,
 * published, uploaded, posted, transmitted, distributed, or disclosed in
 * any way without Intel's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise.  Any license under
 * such intellectual property rights must be express and approved by
 * Intel in writing.
 *
 * For further details, please see the file README.TXT distributed with
 * this software.
 * -- End Intel Copyright Notice --
 *
 * = PRODUCT
 *      Intel(r) IXA SDK 3.0 for the IXP2000 Network Processor, Release 4
 *
 * = LIBRARY
 *      OSSL - Operating System Services  Library
 *
 * = MODULE
 *      OSSL  Abstraction layer header file
 *
 * = FILENAME
 *       ix_ossl.h
 *
 * = DESCRIPTION
 *   This file contains the prototypes of OS-independent wrapper
 *   functions which allow the programmer not to be tied to a specific
 *   operating system. The OSSL functions can be divided into three classes:
 *
 *   1) synchronization-related wrapper functions around thread system calls
 *   2) thread-related wrapper functions around thread calls
 *   3) transactor/workbench osapi calls -- defined in osApi.h
 *
 *   Both 1 and 2 classes of functions provide Thread Management, Thread 
 *   Synchronization, Mutual Exclusion and  Timer primitives. Namely, 
 *   creation and deletion functions as well as the standard "wait" and
 *   "exit". Additionally, a couple of utility functions which enable to 
 *   pause the execution of a thread are also provided.
 *
 *   The 3rd class provides a  slew of other OSAPI functions to handle
 *   Transactor/WorkBench OS calls.
 *
 *
 *   OSSL Thread APIs:
 *             The OSSL thread functions that allow for thread creation,
 *             get thread id, thread deletion and set thread priroity.
 *
 *             ix_ossl_thread_create
 *             ix_ossl_thread_get_id
 *             ix_ossl_thread_exit
 *             ix_ossl_thread_kill
 *             ix_ossl_thread_set_priority
 *             ix_ossl_thread__delay
 * 
 *   OSSL Semaphore APIs:
 *             The OSSL semaphore functions that allow for initialization,
 *             posting, waiting and deletion of semaphores.
 *
 *             ix_ossl_sem_init
 *             ix_ossl_sem_fini
 *             ix_ossl_sem_take
 *             ix_ossl_sem_give
 *             ix_ossl_sem_flush
 *
 *   OSSL Mutex APIs:
 *             The OSSL wrapper functions that allow for initialization,
 *             posting, waiting and deletion of mutexes.
 *
 *             ix_ossl_mutex_init
 *             ix_ossl_mutex_fini
 *             ix_ossl_mutex_lock
 *             ix_ossl_mutex_unlock
 *
 *   OSSL Timer APIs:
 *     	       The timer APIs provide sleep and get time functions.
 *
 *             ix_ossl_sleep
 *             ix_ossl_sleep_tick
 *             ix_ossl_time_get
 * 
 *   OSAPIs for Transactor/WorkBench:
 *             These OSAPI functions are used for transator OS calls. 
 *             They are defined in osApi.h. 
 *
 *             Sem_Init 			
 *             Sem_Destroy 		
 *             Sem_Wait 			
 *             Sem_Wait			
 *             Thread_Create		
 *             Thread_Cancel 		
 *             Thread_SetPriority 		
 *             delayMs 			
 *             delayTick			
 *             
 *
 *
 **********************************************************************
 *      
 *
 * = AUTHOR
 *      Intel Corporation
 *
 * = ACKNOWLEDGEMENTS
 *      
 *
 * = CREATION TIME
 *      1/8/2002 1:53:42 PM
 *
 * = CHANGE HISTORY
 *   02/22/2002 : Renamed osapi.h os_api.h
 *                Moved OS header file includes from OSSL.h to os_api.h
 *                Moved OS specific datatypes to os_api.h 
 *                Modified data types, macros and functions as per
 *                'C' coding guidelines.
 *
 *
 * ============================================================================
 */

#ifndef _IX_OSSL_H
#define _IX_OSSL_H

#ifdef __cplusplus
extern "C"{
#endif

/* Header file includes */
#include "ix_os_type.h"
#include "ix_types.h"
#include "ix_error.h"
#include "ix_symbols.h"
#include "os_datatypes.h"

/* Following header file is required for transactor/workbench OSAPI functions */
/* #include "osApi.h" */



/* OSSL Data Structures */

/**
 * TYPENAME: ix_ossl_thread_t
 * 
 * DESCRIPTION: This type defines OSSL thread type.
 *
 */
typedef os_thread_t     	ix_ossl_thread_t; 

/**
 * TYPENAME: ix_ossl_sem_t
 * 
 * DESCRIPTION: This type defines OSSL semaphore type.
 *
 */
typedef os_sem_t         	ix_ossl_sem_t;

/**
 * TYPENAME: ix_ossl_mutex_t
 * 
 * DESCRIPTION: This type defines OSSL mutex type.
 *
 */
typedef os_mutex_t	ix_ossl_mutex_t;

/**
 * TYPENAME: ix_ossl_time_t
 * 
 * DESCRIPTION: This type defines OSSL time.
 *              The ix_ossl_time_t struct has two fields. 'sec' 
 *              and 'nsec'. Time value is computed as: sec * 10^9 + nsec 
 *
 */
typedef struct ix_s_ossl_time_t
{
    unsigned long tv_sec; /* seconds */
    unsigned long tv_nsec; /* nanoseconds [1, 999,999,999] */
} ix_ossl_time_t;


/* OSSL function types */

/**
 * TYPENAME: ix_ossl_thread_entry_point_t
 * 
 * DESCRIPTION: This function type defines OSSL thread
 *              entry point function.
 *. 
 * @Param:  - IN  arg: pointer to a custom thread argument
 * @Param:  - OUT ptrRetObj: address where a pointer to a custom data structure or 
 *                           object will be returned by the thread on exit.
 * 
 */
typedef ix_error (*ix_ossl_thread_entry_point_t)(
                                                  void* arg, 
                                                  void** ptrRetObj
                                                );

/**
 * TYPENAME: ix_ossl_thread_main_info_t
 * 
 * DESCRIPTION: This type define thread main info.
 *              The struct has two fields: threadMain
 *              and threadArg. The threadMain is pointer to
 *              thread entry point function. threadArg is 
 *              a void pointer to a user defined thread 
 *              entry point argument.  
 *
 */
typedef struct ix_s_ossl_thread_main_info_t
{
    ix_ossl_thread_entry_point_t threadMain;
    void* threadArg;
} ix_ossl_thread_main_info_t;


/* Enumerations are defined here */

/**
 * TYPENAME: ix_ossl_error_code
 * 
 * DESCRIPTION: This type defines error codes returned by OSSL calls.
 *
 */
typedef enum ix_e_ossl_error_code
{
    IX_OSSL_ERROR_SUCCESS =  0,
    IX_OSSL_ERROR_INVALID_ARGUMENTS = OS_INVALID_ATTR,
    IX_OSSL_ERROR_INVALID_OPERATION,
    IX_OSSL_ERROR_THREAD_CALL_FAILURE,
    IX_OSSL_ERROR_INVALID_PID,
    IX_OSSL_ERROR_INVALID_TID,
	IX_OSSL_ERROR_OS_CALL_FAILURE = OS_FAILURE,
    IX_OSSL_ERROR_TIMEOUT = OS_TIMEDOUT,
    IX_OSSL_ERROR_NOMEM = OS_NOMEM,
	IX_OSSL_ERROR_NOSYS = OS_NOSYS
} ix_ossl_error_code;


/**
 * TYPENAME: ix_ossl_sema_state
 * 
 * DESCRIPTION: This type defines OSSL binary semaphore states.
 *
 */
typedef enum ix_e_ossl_sem_state
{
    IX_OSSL_SEM_UNAVAILABLE = OS_SEM_UNVAILABLE,
    IX_OSSL_SEM_AVAILABLE = OS_SEM_AVAILABLE
} ix_ossl_sem_state;



/**
 * TYPENAME: ix_ossl_mutex_state
 * 
 * DESCRIPTION: This type defines OSSL mutex states.
 *
 */
typedef enum ix_e_ossl_mutex_state
{
    IX_OSSL_MUTEX_UNLOCK = OS_MUTEX_UNLOCK,
    IX_OSSL_MUTEX_LOCK = OS_MUTEX_LOCK
} ix_ossl_mutex_state;


/**
 * TYPENAME: ix_ossl_thread_priority
 * 
 * DESCRIPTION: This type define OSSL thread priority levels.
 *
 */
typedef enum ix_e_ossl_thread_priority
{
    IX_OSSL_THREAD_PRI_HIGH = OS_THREAD_PRI_HIGH,
    IX_OSSL_THREAD_PRI_MEDIUM = OS_THREAD_PRI_MEDIUM,
    IX_OSSL_THREAD_PRI_LOW = OS_THREAD_PRI_LOW 
} ix_ossl_thread_priority;


/* Pre-processor Symbols */

/**
 * DESCRIPTION: This symbol defines an error token that indicates the successful
 *          completion of the OSSL calls.
 *
 */
#define IX_OSSL_ERROR_SUCCESS     (ix_error)0UL 
#ifdef __linux
#define IX_OSSL_ERROR_FAILURE 	  (ix_error)1
#endif


/* Following symbols are useful for specifying timeout values
 * in ix_ossl_sem_take and ix_ossl_mutex_lock function calls :
 *
 * IX_OSSL_WAIT_FOREVER : indefinite wait
 * IX_OSSL_WAIT_NONE    : no wait
 */

#define IX_OSSL_WAIT_FOREVER    OS_WAIT_FOREVER /* (-1)UL */
#define IX_OSSL_WAIT_NONE       OS_WAIT_NONE     /* 0 */

/**
 *  OSSL TIME Macros :
 *  Following macros provide functions for ix_time handling. 
 *  These macros will be used for different operations on 
 *  'ix_ossl_time_t' data type:
 *  '==' , '>' , '<' , '+' , '-' , 'normalize' , 'isvalid'
 *  'iszero' , and 'conversion to os ticks'.
 */

/**
 *  Note: For the following macros, 'true' is a non-zero 
 *        integer value and 'false' is a zero' integer
 *        value. The result has type 'int'.
 *
 */

/**
 * MACRO NAME: IX_OSSL_TIME_EQ
 *
 * DESCRIPTION: Compares a operand with b operand. Returns true if 
 *              they are equal and false otherwise.
 *
 * @Param:  - IN   : a type ix_ossl_time_t
 * @Param:  - IN   : b type ix_ossl_time_t 
 *
 * @Return: true if a == b and false otherwise.
 */
#define IX_OSSL_TIME_EQ(a,b)  TM_EQ(a,b)

/**
 * MACRO NAME: IX_OSSL_TIME_GT
 *
 * DESCRIPTION: Compares a operand with b operand. Returns true if
 *              a > b, and false otherwise.
 *
 * @Param:  - IN   : a type ix_ossl_time_t
 * @Param:  - IN   : b type ix_ossl_time_t 
 *
 * @Return: true if a > b and false otherwise.
 */

#define IX_OSSL_TIME_GT(a,b)   TM_GT(a,b)

/**
 * MACRO NAME: IX_OSSL_TIME_LT
 *
 * DESCRIPTION: Compares a operand with b operand. Returns true
 *              if a < b,  and false otherwise. 
 *            
 * @Param:  - IN   : a type ix_ossl_time_t
 * @Param:  - IN   : b type ix_ossl_time_t 
 *
 * @Return: true if a < b and false otherwise.
 */	
#define IX_OSSL_TIME_LT(a,b)  TM_LT(a,b)

/**
 * MACRO NAME: IX_OSSL_TIME_ISZERO
 *
 * DESCRIPTION: This macro checks if the operand a is zero. Returns
 *              true if a is zero (both sec and nsec fields must be zero)
 *              and false otherwise. 
 *
 * @Param:  - IN   : a type ix_ossl_time_t
 *
 * @Return:  true if a is zero and false otherwise.
 */	
#define IX_OSSL_TIME_ISZERO(a)  TM_ISZERO(a)	

/**
 * MACRO NAME: IX_OSSL_TIME_SET
 *
 * DESCRIPTION: This macro sets operand a to the value of operand b.
 *
 * @Param:  - OUT  : a type ix_ossl_time_t
 * @Param:  - IN   : b type ix_ossl_time_t
 *
 */
#define IX_OSSL_TIME_SET(a,b)   TM_SET(a,b)

/**
 * MACRO NAME: IX_OSSL_TIME_ADD
 *
 * DESCRIPTION: This macro performs  a += b operation. 
 *        
 * @Param:  - INOUT: a type ix_ossl_time_t
 * @Param:  - IN   : b type ix_ossl_time_t
 *
 */	    
#define IX_OSSL_TIME_ADD(a,b)    TM_ADD(a,b)

/**
 * MACRO NAME: IX_OSSL_TIME_SUB
 *
 * DESCRIPTION: This macro performs  a -= b operation.
 *
 * @Param:  - INOUT: a type ix_ossl_time_t
 * @Param:  - IN   : b type ix_ossl_time_t
 *
 */		
#define IX_OSSL_TIME_SUB(a,b)    TM_SUB(a,b)

/**
 * MACRO NAME: IX_OSSL_TIME_NORMALIZE
 *
 * DESCRIPTION: This macro normalizes the value of a. If 'a.nsec' > 10^9,
 *              it is decremented by 10^9 and 'a.sec' is incremented by 1.
 *
 * @Param:  - INOUT: a type ix_ossl_time_t
 *
 */	
#define	IX_OSSL_TIME_NORMALIZE(a)   TM_NORMALIZE(a)	

/**
 * MACRO NAME: IX_OSSL_TIME_VALID
 *
 * DESCRIPTION: This macro checks whether a is a valid ix_ossl_time_t i.e. 
 *              0 =< a.nsec < 10^9. Returns true if a is valid and 
 *              false otherwise. 
 *
 * @Param :  - IN : a type ix_ossl_time_t
 *
 * @Return:  true if a is valid ix_ossl_time_t and false otherwise.
 *
 */	
#define IX_OSSL_TIME_VALID(a)    TM_VALID(a)	

/**
 * MACRO NAME: IX_OSSL_TIME_ZERO
 *
 * DESCRIPTION:This macro sets the value of a to zero.
 *
 * @Param :  - INOUT : a type ix_ossl_time_t
 *
 */	
#define IX_OSSL_TIME_ZERO(a)   TM_ZERO(a)

/**
 * MACRO NAME: IX_OSSL_TIME_CONVERT_TO_TICKS
 *
 * DESCRIPTION:This macro converts b value in ix_ossl_time_t to 
 *             a value in os ticks. 
 *
 * @Param:  - OUT  : a type unsigned int
 * @Param:  - IN   : b type ix_ossl_time_t
 *
 */	
#define IX_OSSL_TIME_CONVERT_TO_TICK(a,b)   TM_CONVERT_TO_TICK(a,b)


/*  OSSL  functions */

/**
 * NAME: ix_ossl_thread_create
 *
 * DESCRIPTION: This function creates a cancellable thread that will 
 *              execute the user-provided entry point function. Custom 
 *              arguments can be passed to this function using the "arg"
 *              argument. 
 *
 * @Param:  - IN entryPoint:  thread's entry point function.
 * @Param:  - IN  arg      :  pointer to custom arguments that
 *                            will be passed to entry point function
 *                            as the first argument.
 *
 * @Param:  - OUT ptrTid   :  address at which the thread id of the
 *                            newly created thread will be returned
 *
 * @Return: IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_thread_create(
                                ix_ossl_thread_entry_point_t entryPoint,
                                void* arg,
                                ix_ossl_thread_t* ptrTid
                              );

/**
 * NAME: ix_ossl_thread_get_id
 *
 * DESCRIPTION: This function returns id of the calling thread.
 *
 * @Param:  - OUT ptrTid   :  address at which the id of the calling 
 *                            thread will be returned
 *
 * @Return: IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_thread_get_id(ix_ossl_thread_t* ptrTid);                               
                                
/**
 * NAME: ix_ossl_thread_exit
 *
 * DESCRIPTION:  This function causes the calling thread to exit. It
 *               gives the opportunity to the caller to pass back to 
 *               a waiting parent a pointer to a custom data structure 
 *               and an ix_error token.
 *
 * @Param:  - IN retError  ix_error token to be returned to the waiting
 *                         parent (0 if no error is returned)
 *
 * @Param:  - IN retObj   pointer to custom data structure returned to
 *                        the waiting parent on thread exit.It is used for
 *                        post-mortem debugging.(null if no data structure 
 *                        is returned)
 *
 */
IX_EXPORT_FUNCTION 
void ix_ossl_thread_exit(ix_error retError, void* retObj);

/**
 * NAME: ix_ossl_thread_kill
 *
 * DESCRIPTION:  Kills the running thread specified by its thread id.
 *               Because the thread will be killed instantly, the caller 
 *               must be extremely careful when using this function as 
 *               the thread will not have time to release any of the 
 *               resources it is currently owning. ix_ossl_thread_exit
 *               should be used to delete a thread and its resources instead!. 
 *
 * @Param:  - IN tid:   id of the thread to be killed
 *
 * @Return:  IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_thread_kill(ix_ossl_thread_t tid);



/**
 * NAME:  ix_ossl_thread_set_priority
 *
 * DESCRIPTION:This function sets the priority of the indicated thread.
 *     	       Possible values for 'priority' are IX_OSSL_THREAD_PRI_HIGH, 
 *             IX_OSSL_THREAD_PRI_MED, and IX_OSSL_THREAD_PRI_LOW. 
 *
 * @Param:  - IN tid:   id of the thread 
 * @Param:  - IN priority:  valid priority values are :- IX_OSSL_THREAD_PRI_HIGH, 
 *            IX_OSSL_THREAD_PRI_MED, and IX_OSSL_THREAD_PRI_LOW. 
 *
 * @Return:  IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_thread_set_priority(
                                     ix_ossl_thread_t tid, 
                                     ix_ossl_thread_priority priority
                                    );


/**
 * NAME: ix_ossl_thread_delay
 *
 * DESCRIPTION: This function causes the current task to delay for the specified
 *              number of OS ticks. Control of the CPU is relinquished during this time
 *              allowing other system tasks a chance to execute.
 *
 *
 * @Param:  - IN entryPoint:  number of OS ticks to delay task.
 
 *
 * @Return: IX_OSSL_ERROR_SUCCESS if successful or IX_OSSL_ERROR_FAILURE for failure.
 */
IX_EXPORT_FUNCTION
ix_error ix_ossl_thread_delay(
                                int ticks
				);


/**
 * NAME:      ix_ossl_tick_get 
 *
 * DESCRIPTION: This function returns the number of os ticks per second.
 *
 * @Param:  - OUT pticks :  pointer to location  where data will be returned.
 *
 * @Return: IX_OSSL_ERROR_SUCCESS.
 */
IX_EXPORT_FUNCTION
ix_error ix_ossl_tick_get(
			  int *pticks
			  );



/**
 * NAME:  ix_ossl_sem_init
 *
 * DESCRIPTION: This function initializes a new semaphore. 'sid' is a 
 *              pointer to an ix_ossl_sem_t. Upon success, '*sid' will be
 *              the semaphore id used in all other ix_ossl_sem 
 *              functions. The newly created semaphore will be initialized
 *              the value of 'start_value'.
 *
 * @Param:  - IN start_value:  initial value of the semaphore
 * 
 * @Param:  - OUT sid: Address where the newly created semaphore id will
 *                     returned.
 *
 * @Return: IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_sem_init(int start_value, ix_ossl_sem_t* sid);

/**
 * NAME:  ix_ossl_sem_take
 *
 * DESCRIPTION: If the semaphore is 'empty', the calling thread is blocked. 
 *              If the semaphore is 'full', it is taken and control is returned
 *              to the caller. If the time indicated in 'timeout' is reached, 
 *              the thread will unblock and return an error indication. If the
 *              timeout is set to 'IX_OSSL_WAIT_NONE', the thread will never block;
 *              if it is set to 'IX_OSSL_WAIT_FOREVER', the thread will block until
 *              the semaphore is available. 
 *
 *
 * @Param:  - IN sid:  semaphore id.
 * @Param:  - IN timeout:  timeout value of type ix_uint32 expressed in miliseconds
 *
 * @Return: IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_sem_take(
                          ix_ossl_sem_t sid,
                          ix_uint32 timeout
                         );

/**
 * NAME:   ix_ossl_sem_give
 *
 * DESCRIPTION: This function causes the next available thread in the pend queue
 *              to be unblocked. If no thread is pending on this semaphore, the 
 *              semaphore becomes 'full'. 
 *
 * @Param:  - IN sid:  semaphore id.
 *
 * @Return: IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_sem_give(ix_ossl_sem_t sid);


/**
 * NAME:   ix_ossl_sem_flush
 *
 * DESCRIPTION: This function unblocks all pending threads
 *              without altering the semaphore count. 'sid' 
 *              is the id for the semaphore. '*result' will 
 *              be non-zero if a thread was unblocked during
 *              this call.
 *
 * @Param:  - IN sid:  semaphore id
 * @Param:  - OUT result:  the value referred will be non-zero if a thread was 
 *              unblocked during this call 
 *
 * @Return: IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_sem_flush(ix_ossl_sem_t sid, int* result);


/**
 * NAME:   ix_ossl_sem_fini
 *
 * DESCRIPTION: This function frees a semaphore.'sid' is semaphore id.  
 *              The semaphore is terminated, all resources are freed. 
 *              The threads pending on this semaphore will be released 
 *              and return an error.
 *
 * @Param:  - IN sid:  semaphore id
 *
 * @Return: IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_sem_fini(ix_ossl_sem_t sid);


/**
 * NAME:    ix_ossl_mutex_init
 *
 * DESCRIPTION: This function initializes a new mutex. 'mid' is a pointer
 *              to an ix_ossl_mutex_t. Upon success, '*mid' will contain
 *              the mutex id. 'start_state' is the initial locking
 *              state.
 *
 * @Param:  - IN start_state: 'start_state' is  initial locking state. 
 *                            Valid values are :-
 *                            'OSSL_MUTEX_LOCK': Lock the mutex
 *                            'OSSL_MUTEX_UNLOCK': Leave the mutex unlocked
 *
 * @Param:  - OUT mid:  pointer to id of the mutex created
 *
 * @Return: IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_mutex_init(ix_ossl_mutex_state start_state, ix_ossl_mutex_t* mid);


/**
 * NAME:   ix_ossl_mutex_lock
 *
 * DESCRIPTION: This function locks the mutex. If the mutex is already
 *              locked, the thread will block. If the time indicated in
 *              'timeout' is reached, the thread will unblock and return
 *              error indication. If the timeout is set to 'IX_OSSL_WAIT_NONE', 
 *              the thread will never block (this is trylock). If the timeout
 *              is set to 'IX_OSSL_WAIT_FOREVER',  the thread will block until
 *              the mutex is  unlocked. 
 *
 * @Param:  - IN mid:  mutex id.
 * @Param:  - IN timeout:  timeout value of type ix_uint32 expressed in miliseconds.
 *
 * @Return: IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_mutex_lock(
                            ix_ossl_mutex_t mid, 
                            ix_uint32 timeout
                           );


/**
 * NAME:  ix_ossl_mutex_unlock
 *
 * DESCRIPTION: This function unlocks the mutex.  'mid' is the mutex id.
 *              If there are threads pending on the mutex, the next one is 
 *              given the lock. If there are no pending threads, then the 
 *              mutex is unlocked.
 *
 * @Param:  - IN mid:  mutex id
 *
 * @Return: IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_mutex_unlock(ix_ossl_mutex_t mid);


/**
 * NAME:   ix_ossl_mutex_fini
 *
 * DESCRIPTION: This function frees a mutex.'mid' is the id mutex id. 
 *              The mutex is deleted, all resources are freed. Any threads 
 *              pending on this mutex will be unblocked and return an error.
 *
 * @Param:  - IN mid:  mutex id
 *
 * @Return: IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_mutex_fini(ix_ossl_mutex_t mid);


/**
 * NAME:   ix_ossl_sleep
 *
 * DESCRIPTION: This function causes the calling thread to sleep for the 
 *              specified time.
 *
 * @Param:  - IN sleeptime_ms:  sleep time specified in milliseconds.
 *
 * @Return: IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */

ix_error ix_ossl_sleep(ix_uint32 sleeptime_ms);


/**
 * NAME:    ix_ossl_sleep_tick 
 *
 * DESCRIPTION: This function causes the calling thread to sleep for the 
 *              time specified in OS ticks.
 *
 * @Param:  - IN sleeptime_ticks:  sleep time specified in os ticks.
 *
 * @Return: IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_sleep_tick(ix_uint32 sleeptime_ticks);


/**
 * NAME:      ix_ossl_time_get 
 *
 * DESCRIPTION: This function places the current value of a timer,
 *              in seconds and  nano-seconds, into an ix_ossl_time_t structure 
 *              pointed by 'ptime'. This function does not provide a 
 *              time-of-day. The intention is to provide a nano-second 
 *              resolution time.
 *
 * @Param:  - OUT ptime :  pointer to 'ix_ossl_time_t' structure where data will be
 *              returned.
 *
 * @Return: IX_OSSL_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_time_get(ix_ossl_time_t*  ptime);



/**
 * This section describe memory management functions
 */


/**
 * TYPENAME: ix_ossl_size_t
 * 
 * DESCRIPTION: This type describes a generic size type.
 *
 */
typedef unsigned int ix_ossl_size_t;



/**
 * NAME: ix_ossl_malloc
 *
 * DESCRIPTION: This function will allocate a memory block.
 *          the function returns a void pointer to the allocated space, or NULL if there is 
 *          insufficient memory available. To return a pointer to a type other than void, use
 *          a type cast on the return value. The storage space pointed to by the return value 
 *          is guaranteed to be suitably aligned for storage of any type of object. If size is 0,
 *          ix_ossl_malloc allocates a zero-length item in the heap and returns a valid pointer 
 *          to that item. Always check the return from ix_ossl_malloc, even if the amount of memory
 *          requested is small.
 * 
 * @Param:  - IN arg_Size - the size of the memory block requested.
 *
 * @Return: Returns a valid address if successful or a NULL for failure.
 */
IX_EXPORT_FUNCTION 
void* ix_ossl_malloc(
                      ix_ossl_size_t arg_Size
                    );




/**
 * NAME: ix_ossl_free
 *
 * DESCRIPTION: This function will free a memory block specified by the passed address.
 *          The ix_ossl_free function deallocates a memory block (arg_pMemory) that was previously 
 *          allocated by a call to ix_ossl_malloc.If arg_pMemory is NULL, the pointer is ignored 
 *          and ix_ossl_free immediately returns. Attempting to free an invalid pointer (a 
 *          pointer to a memory block that was not allocated by ix_ossl_malloc) may affect 
 *          subsequent allocation requests and cause errors.
 * 
 * @Param:  - IN arg_pMemory - address of the memory block to be deallocated.
 */
IX_EXPORT_FUNCTION 
void ix_ossl_free(
                   void* arg_pMemory
                 );




/**
 * NAME: ix_ossl_memcpy
 *
 * DESCRIPTION: This function will copy memory bytes between buffers.
 *          The ix_ossl_memcpy function copies count bytes of arg_pSrc to arg_pDest. If the source and 
 *          destination overlap, this function does not ensure that the original source bytes in the 
 *          overlapping region are copied before being overwritten. 
 * 
 * @Param:  - INOUT arg_pDest - destination buffer address 
 * @Param:  - IN arg_pSrc - source buffer address
 * @Param:  - IN arg_Count - number of bytes to copy
 *
 * @Return: Returns the address of the destination buffer.
 */
IX_EXPORT_FUNCTION 
void* ix_ossl_memcpy(
                      void* arg_pDest, 
                      const void* arg_pSrc,
                      ix_ossl_size_t arg_Count 
                    );



/**
 * NAME: ix_ossl_memset
 *
 * DESCRIPTION: This function sets buffers to a specified character.
 *          The ix_ossl_memset function sets the first arg_Count bytes of arg_pDest to the 
 *          character arg_Char.
 * 
 * @Param:  - INOUT arg_pDest - destination buffer address 
 * @Param:  - IN arg_pChar - character to set
 * @Param:  - IN arg_Count - number of characters to set
 *
 * @Return: Returns the address of the destination buffer.
 */
IX_EXPORT_FUNCTION 
void* ix_ossl_memset(
                      void* arg_pDest, 
                      int arg_pChar,
                      ix_ossl_size_t arg_Count 
                    );



/**
 * Message logging API
 */

/**
 * NAME: ix_ossl_message_log_init
 *
 * DESCRIPTION: This function is used to initialize the error message logging. For each OS the 
 *              messages will be logged into an implementation dependent stream. Further details 
 *              will be provided on where the messages will be logged! This function should be called 
 *              before any call to ix_ossl_message_log().
 * 
 * @Return: IX_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_message_log_init(void);


/**
 * NAME: ix_ossl_message_log
 *
 * DESCRIPTION: This routine is used to log a specified message.  This routine's  syntax is similar to
 *              printf() - a format string is followed by  a variable number of arguments which will 
 *              be interpreted and formated according to the fmt_string passed as the first argument.
 *              Further details will be provided on where the messages will be logged!
 *
 * @Param: - IN arg_pFmtString  -  format string for the log message
 *
 * @Return: IX_ERROR_SUCCESS if successful or a valid ix_error token for failure.
 */
IX_EXPORT_FUNCTION 
ix_error ix_ossl_message_log(
                              char* arg_pFmtString, 
                              ...
                            );


#ifdef __cplusplus
}
#endif
 
#endif /* _IX_OSSL_H */

