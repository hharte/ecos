/**
 * @file
 *
 * @brief this file contains the API of the @ref IxOsServices component
 * 
 * -- Intel Copyright Notice --
 *
 * @par
 * INTEL CONFIDENTIAL
 *
 * @par
 * Copyright 2002 Intel Corporation All Rights Reserved.
 *
 * @par
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
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise.  Any license under
 * such intellectual property rights must be express and approved by
 * Intel in writing.
 *
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * -- End Intel Copyright Notice --
 */

#ifdef __vxworks

#include <vxWorks.h>
#include <semLib.h>

#elif defined(__linux)
#include <linux/types.h>
#include <asm/semaphore.h>
#elif defined(__ECOS)
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_if.h>
#else /* POSIX */

#include <pthread.h>

#endif /* __vxworks */

#include "IxTypes.h"

#ifndef IxOsServices_H

#ifndef __doxygen_hide
#define IxOsServices_H
#endif /* __doxygen_hide */

/**
 * @defgroup IxOsServices IXP4xx OS Services (IxOsServices) API
 *
 * @brief This service provides a very thin layer of OS dependency services. 
 *
 * This file contains the API to the functions which are some what OS dependant and would
 * require porting to a particular OS. 
 * A primary focus of the component development is to make them as OS independent as possible.
 * All other components should abstract their OS dependency to this module.
 * Services overview
 *	-# Trace Service - a simple debugging mechanism, with compile time debug trace level
 *	   (depends on existent OS logging feature e.g. in Linux use kprintf, in VxWorks logMsg)
 *	-# Mutual Exclusion
 *	 	- interrupt binding and locking mechanisms
 *		- mutex locks and fast mutexes
 * 	-# Timer Services
 *		-  timed delays, busy loop - microsecond granularity
 *		-  timed delays, OS dependent yielding - millisecond granularity
 *		-  timestamp measurements - XScale core clock granularity
 *
 * @{
 */

#if CPU!=SIMSPARCSOLARIS
#ifndef __ECOS
#define IXP425_USE_FAST_MUTEX
#endif
#endif

typedef int IX_IRQ_STATUS;
     
/**
 * Mutex object
 */
#ifdef __vxworks
          
typedef SEM_ID IxMutex;
 
#elif defined(__linux)
typedef struct semaphore *IxMutex;
#elif defined(__ECOS)
typedef cyg_drv_mutex_t IxMutex;
#else /* POSIX mutex */
 
typedef pthread_mutex_t IxMutex;
 
#endif /* __vxworks */

/**
 * Fast mutex object
 */
#ifdef __vxworks

#if CPU==SIMSPARCSOLARIS
     
typedef SEM_ID IxFastMutex;

#else

typedef int IxFastMutex;

#endif

#elif defined(__linux)
#ifdef IXP425_USE_FAST_MUTEX
typedef int IxFastMutex;
#else
typedef struct semaphore *IxFastMutex;
#endif
#elif defined(__ECOS)
typedef cyg_drv_mutex_t IxFastMutex;
#else /* POSIX mutex */

typedef pthread_mutex_t IxFastMutex;

#endif /* __vxworks */
   
/**
 * Trace levels
 */
typedef enum
{
    LOG_NONE    = 0,

    LOG_USER    = 1,
    LOG_FATAL   = 2,
    LOG_ERROR   = 3,
    LOG_WARNING = 4,
    LOG_MESSAGE = 5,
    LOG_DEBUG1  = 6,
    LOG_DEBUG2  = 7,
    LOG_DEBUG3  = 8,

    LOG_ALL
} IxOsServTraceLevels;
  
/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC IX_STATUS ixOsServIntBind(int vector, void (*routine)(void *), void *parameter)
 *
 * @brief binds a routine to a hardware interrupt
 *
 * @param vector int (in) - interrupt vector to bind to
 * @param routine void (*)(void *) (in) - routine to connect
 * @param parameter void * (in) - parameter to pass to the routine when called
 *
 * This functions binds the specified C routine to an interrupt vector.
 * When called, the "parameter" value will be passed to the routine.
 *
 * Can be called from interrupt level: no
 *
 * @return IX_SUCCESS if the operation succeeded or IX_FAIL otherwise
 */
PUBLIC IX_STATUS
ixOsServIntBind(int vector, void (*routine)(void *), void *parameter);

/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC IX_STATUS ixOsServIntUnbind(int vector)
 *
 * @brief unbinds a routine from a hardware interrupt
 *
 * @param vector int (in) - interrupt vector to unbind from
 *
 * This function unbinds from an interrupt vector a C routine previously
 * bound with ixOsServIntBind
 *
 * Can be called from interrupt level: no
 *
 * @return IX_SUCCESS if the operation succeeded or IX_FAIL otherwise
 */
PUBLIC IX_STATUS
ixOsServIntUnbind(int vector);



/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC int ixOsServIntLock()
 *
 * @brief locks out IRQs
 *
 * This function disables IRQs, returning a lock key
 * which can be used later with ixOsServIntUnlock to re-enable
 * the IRQs.
 *
 * Can be called from interrupt level: yes
 *
 * @return a lock key used by ixOsServIntUnlock to unlock IRQs
 *
 * @warning Do not call system routines when IRQs are locked
 */
PUBLIC int
ixOsServIntLock(void);

/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC void ixOsServIntUnlock(int lockKey)
 *
 * @brief unlocks IRQs
 *
 * @param lockKey int (in) - lock key previously obtained with ixOsServIntLock()
 *
 * This function reenables the IRQs locked out by ixOsServIntLock().
 *
 * Can be called from interrupt level: yes
 */
PUBLIC void
ixOsServIntUnlock(int lockKey);

/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC int ixOsServIntLevelSet(int level)
 *
 * @brief sets the interrupt level
 *
 * @param level int (in) - new interrupt level
 *
 * This routine changes the interrupt mask in the status register 
 * to take on the value specified by level. 
 * Interrupts are locked out at or, depending on the implementation, below that level.
 *
 * Can be called from interrupt level: yes
 *
 * @return the previous interrupt level
 *
 * @warning Do not call system functions when interrupts are locked
 */

PUBLIC int
ixOsServIntLevelSet(int level);

/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC IX_STATUS ixOsServMutexInit(IxMutex *mutex)
 *
 * @brief initializes a mutex
 *
 * @param mutex IxMutex * (in) - pointer to mutex to initialize
 *
 * Initializes the given mutex (MUTual EXclusion device)
 *
 * Can be called from interrupt level: no
 *
 * @return IX_SUCCESS if the operation succeeded or IX_FAIL otherwise
 */
PUBLIC IX_STATUS
ixOsServMutexInit(IxMutex *mutex);

/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC IX_STATUS ixOsServMutexLock(IxMutex *mutex)
 *
 * @brief locks the given mutex
 *
 * @param mutex IxMutex * (in) - pointer to mutex to lock
 *
 * If the mutex is unlocked it becomes locked and owned by
 * the caller, and the function returns immediately.
 * If the mutex is already locked calling this function
 * will suspend the caller until the mutex is unlocked.
 *
 * Can be called from interrupt level: no
 *
 * @return IX_SUCCESS if the operation succeeded or IX_FAIL otherwise
 *
 * @warning Two or more consecutive calls from the same thread of
 * ixOsServMutexLock() are likely to deadlock the calling thread.
 * Mutexes are NOT recursive.
 */
PUBLIC IX_STATUS
ixOsServMutexLock(IxMutex *mutex);

/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC IX_STATUS ixOsServMutexUnlock(IxMutex *mutex)
 *
 * @brief unlocks the given mutex
 *
 * @param mutex IxMutex * (in) - pointer to mutex to unlock
 *
 * Unlocks the given mutex, returning it to the "unlocked" state.
 * If the mutex was not locked an error is returned.
 *
 * Can be called from interrupt level: yes
 *
 * @return IX_SUCCESS if the operation succeeded or IX_FAIL otherwise
 */
PUBLIC IX_STATUS
ixOsServMutexUnlock(IxMutex *mutex);

/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC IX_STATUS ixOsServMutexDestroy(IxMutex *mutex)
 *
 * @brief destroys a mutex object
 *
 * @param mutex IxMutex * (in) - pointer to mutex to destroy
 *
 * Can be called from interrupt level: no
 *
 * Destroys the mutex object, freeing the resources it might hold.
 */
PUBLIC IX_STATUS
ixOsServMutexDestroy(IxMutex *mutex);

/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC IX_STATUS ixOsServFastMutexInit(IxFastMutex *mutex)
 *
 * @brief initializes a fast mutex
 *
 * @param mutex IxFastMutex * (in) - pointer to the mutex object
 *
 * Initializes a fast mutex, placing it in "unlocked" state.
 *
 * Can be called from interrupt level: yes
 *
 * @return IX_SUCCESS if the operation succeeded or IX_FAIL otherwise
 */
PUBLIC IX_STATUS
ixOsServFastMutexInit(IxFastMutex *mutex);

/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC IX_STATUS ixOsServFastMutexTryLock(IxFastMutex *mutex)
 *
 * @brief attempts to lock the fast mutex object
 *
 * @param mutex IxFastMutex * (in) - pointer to the mutex object
 *
 * If the mutex is in "unlocked" state it becomes locked
 * and the function returns "IX_SUCCESS". If the mutex is
 * already locked the function returns immediately with a IX_FAIL
 * result.
 *
 * Can be called from interrupt level: yes
 *
 * @return IX_SUCCESS if the operation succeeded or IX_FAIL otherwise
 */
PUBLIC IX_STATUS
ixOsServFastMutexTryLock(IxFastMutex *mutex);

/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC IX_STATUS ixOsServFastMutexUnlock(IxFastMutex *mutex)
 *
 * @brief unlocks the fast mutex object
 *
 * @param mutex IxFastMutex * (in) - pointer to the mutex object
 *
 * Unlocks the given mutex object if locked, otherwise
 * returns an error.
 *
 * Can be called from interrupt level: yes
 *
 * @return IX_SUCCESS if the operation succeeded or IX_FAIL otherwise
 */
PUBLIC IX_STATUS
ixOsServFastMutexUnlock(IxFastMutex *mutex);

/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC int ixOsServLog(int level, char *format, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6)
 *
 * @brief logs a formatted message
 *
 * @param level int (in) - trace level
 * @param format char * (in) - format string, similar to printf()
 * @param arg1 int (in) - first argument to display
 * @param arg2 int (in) - second argument to display
 * @param arg3 int (in) - third argument to display
 * @param arg4 int (in) - fourth argument to display
 * @param arg5 int (in) - fifth argument to display
 * @param arg6 int (in) - sixth argument to display
 *
 * This function logs the specified message via the logging task. 
 * It is similar to printf(), however it requires a log level and a fixed number of arguments.
 * Unless called with the LOG_USER log level it will prefix the user message
 * with a tag string like "[debug2]".
 *
 * Can be called from interrupt level: yes
 *
 * @return the number of characters written
 */
PUBLIC int
ixOsServLog(int level, char *format, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);

/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC int ixOsServLogLevelSet(int level)
 *
 * @brief sets the logging level
 *
 * @param level int (in) - logging level
 *
 * This function sets the maximum level allowed for logging.
 * Setting it to LOG_WARNING will disable all levels like
 * LOG_MESSAGE, LOG_DEBUG1 etc.
 * It returns the previous level which can be useful for
 * restoring log states.
 *
 * Can be called from interrupt level: yes
 *
 * @return the previous logging level
 */

PUBLIC int
ixOsServLogLevelSet(int level);

/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC void ixOsServSleep(int microseconds)
 *
 * @brief execution block for a number of microseconds
 *
 * @param microseconds int (in) - delay to block execution for
 *
 * This function blocks the calling task using a timed busy loop.
 *
 * Can be called from interrupt level: yes
 */
PUBLIC void
ixOsServSleep(int microseconds);

/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC void ixOsServTaskSleep(int milliseconds)
 *
 * @brief preemptive execution block for a number of milliseconds
 *
 * @param milliseconds int (in) - delay to block execution for
 *
 * This function blocks the calling task using an OS-dependent
 * preemptive timed delay.
 *
 * Can be called from interrupt level: no
 */
PUBLIC void
ixOsServTaskSleep(int milliseconds);

/** 
 * @ingroup IxOsServices
 *
 * @fn PUBLIC int ixOsServTimestampGet()
 *
 * @brief used to retrieve the system timestamp
 *
 * Can be called from interrupt level: yes
 *
 * @return the current timestamp value
 */
PUBLIC unsigned int
ixOsServTimestampGet(void);

/**
 * @} IxOsServices
 */

#endif /* IxOsServices_H */
