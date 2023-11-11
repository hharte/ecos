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
 *      OS Specific Data Types header file
 *
 * = FILENAME
 *       OSSL.h
 *
 * = DESCRIPTION
 *   This file  contains definitions and encapsulations for OS specific data types. These
 *   encapsulated  data types are  used by OSSL header files and OS API functions.
 *
 *
 **********************************************************************
 *      
 *
 * = AUTHOR
 *      Intel Corporation
 *
 * = AKNOWLEDGEMENTS
 *      
 *
 * = CREATION TIME
 *      1/8/2002 1:53:42 PM
 *
 * = CHANGE HISTORY
 
 * ============================================================================
 */

#ifndef _OS_DATATYPES_H
#define _OS_DATATYPES_H

#ifdef __vxworks

#include "time.h"
#include "vxWorks.h"
#include "semaphore.h"

typedef	unsigned int os_thread_t;
typedef	unsigned int os_sem_t;
typedef	unsigned int os_mutex_t;



typedef struct os_s_timespec_t
    { 					/* interval = sec*10**9 + nsec */
    time_t  sec;			/* seconds */
    long    nsec;			/* nanoseconds (0 - 1,000,000,000) */
    }timespec_t;


typedef enum os_e_thread_priority{
  OS_THREAD_PRI_LOW = 240,
  OS_THREAD_PRI_MEDIUM = 160,
  OS_THREAD_PRI_HIGH = 90
} os_thread_priority;


/**
 * TYPENAME: os_sem_state
 * 
 * DESCRIPTION: This type defines OS binary semaphore states.
 *
 */
typedef enum  os_e_sem_state
{
    OS_SEM_UNVAILABLE =SEM_EMPTY,
    OS_SEM_AVAILABLE =SEM_FULL
} os_sem_state;

	
/**
 * TYPENAME: os_mutex_state
 * 
 * DESCRIPTION: This type defines OS mutex states.
 *
 */
typedef enum os_e_mutex_state
{
    OS_MUTEX_UNLOCK = FALSE ,
    OS_MUTEX_LOCK   = TRUE
} os_mutex_state;

typedef enum os_e_error_codes{

	OS_FAILURE = 1 ,            /* os call failed */ 
	OS_RESOURCE_SHORTAGE ,      /* EAGAIN: The system lacked the necessary
				                               resources to */
    OS_INVALID_ATTR ,           /* EINVAL: The value specified by the attribute is 
		                                 not valid */ 
    OS_INVALID_PRIVLG ,         /* EPERM: The caller does not have the appropriate
		  	                    	         permission to set the required parameters */
    OS_RESOURCE_DEADLOCK ,      /* EDEADLK: */
    OS_INVALID_THREADID ,       /* ESRCH:  */
    OS_NOMEM ,                  /* ENOMEM: */
    OS_RESOURCE_BUSY ,          /* EBUSY:  Device or Resource Busy */
    OS_NOSYS,                   /* ENOSYS: */
    OS_TIMEDOUT ,               /* ETIMEDOUT: */
    OS_INVALID_MUTEX,           /* EINVAL : Invalid Mutex */
    OS_SEMA_NOSPC ,             /* ENOSPC */
    OS_ERR_INTR  =  14,         /* EINTR */
    OS_INVALID_OPERATION = 15,
    OS_INVALID_ADDR = 16        /* EFAULT */
} os_error_code;

typedef struct os_s_error{
  os_error_code  errnum;
  char errname   [25];
} os_error;



#define OS_WAIT_FOREVER WAIT_FOREVER
#define OS_WAIT_NONE	NO_WAIT 

#elif __linux

#include <linux/types.h>
#include <asm/semaphore.h>

#define OS_WAIT_FOREVER     (-1UL)
#define OS_WAIT_NONE        0

typedef enum os_e_mutex_state {
     OS_MUTEX_UNLOCK = 0,
     OS_MUTEX_LOCK = 1
} os_mutex_state;
 
typedef enum os_e_sem_state {
     OS_SEM_UNVAILABLE = 0,
     OS_SEM_AVAILABLE = 1
 } os_sem_state;

typedef enum os_e_thread_priority {
     OS_THREAD_PRI_HIGH = 90,
     OS_THREAD_PRI_MEDIUM = 160,
     OS_THREAD_PRI_LOW = 240 
} os_thread_priority;


typedef unsigned int os_thread_t;
typedef struct semaphore *os_sem_t;
typedef struct semaphore *os_mutex_t;

typedef enum os_e_error_codes{

    OS_FAILURE = 1 ,            /* os call failed */
   OS_RESOURCE_SHORTAGE ,      /* EAGAIN: The system lacked the necessary
                                                               resources to */
    OS_INVALID_ATTR ,           /* EINVAL: The value specified by the attribute is
                                                 not valid */
    OS_INVALID_PRIVLG ,         /* EPERM: The caller does not have the appropriate
                                                         permission to set the required parameters */
    OS_RESOURCE_DEADLOCK ,      /* EDEADLK: */
    OS_INVALID_THREADID ,       /* ESRCH:  */
    OS_NOMEM ,                  /* ENOMEM: */
    OS_RESOURCE_BUSY ,          /* EBUSY:  Device or Resource Busy */
    OS_NOSYS,                   /* ENOSYS: */
    OS_TIMEDOUT ,               /* ETIMEDOUT: */
    OS_INVALID_MUTEX,           /* EINVAL : Invalid Mutex */
    OS_SEMA_NOSPC ,             /* ENOSPC */
    OS_ERR_INTR  =  14,         /* EINTR */
    OS_INVALID_OPERATION = 15,
    OS_INVALID_ADDR = 16        /* EFAULT */
} os_error_code;

typedef struct os_s_error{
  os_error_code  errnum;
  char errname   [25];
} os_error;

#elif __ECOS

#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_if.h>

typedef	unsigned int os_thread_t;

// FIXME This is okay for RedBoot where there is only one thread.
// FIXME Need to do something else for eCos proper.
typedef	unsigned int os_sem_t;

typedef	cyg_drv_mutex_t * os_mutex_t;

typedef enum os_e_mutex_state {
    OS_MUTEX_UNLOCK = 0,
    OS_MUTEX_LOCK   = 1
} os_mutex_state;

typedef enum os_e_sem_state {
     OS_SEM_UNVAILABLE = 0,
     OS_SEM_AVAILABLE = 1
} os_sem_state;

typedef enum os_e_error_codes{

    OS_FAILURE = 1 ,            /* os call failed */
    OS_RESOURCE_SHORTAGE ,      /* EAGAIN: The system lacked the necessary
                                                               resources to */
    OS_INVALID_ATTR ,           /* EINVAL: The value specified by the attribute is
                                                 not valid */
    OS_INVALID_PRIVLG ,         /* EPERM: The caller does not have the appropriate
                                                         permission to set the required parameters */
    OS_RESOURCE_DEADLOCK ,      /* EDEADLK: */
    OS_INVALID_THREADID ,       /* ESRCH:  */
    OS_NOMEM ,                  /* ENOMEM: */
    OS_RESOURCE_BUSY ,          /* EBUSY:  Device or Resource Busy */
    OS_NOSYS,                   /* ENOSYS: */
    OS_TIMEDOUT ,               /* ETIMEDOUT: */
    OS_INVALID_MUTEX,           /* EINVAL : Invalid Mutex */
    OS_SEMA_NOSPC ,             /* ENOSPC */
    OS_ERR_INTR  =  14,         /* EINTR */
    OS_INVALID_OPERATION = 15,
    OS_INVALID_ADDR = 16        /* EFAULT */
} os_error_code;

typedef struct os_s_error{
  os_error_code  errnum;
  char errname   [25];
} os_error;

typedef enum os_e_thread_priority {
     OS_THREAD_PRI_HIGH = 90,
     OS_THREAD_PRI_MEDIUM = 160,
     OS_THREAD_PRI_LOW = 240 
} os_thread_priority;

#define OS_WAIT_FOREVER -1
#define OS_WAIT_NONE	0

#define printf diag_printf

#endif /*__vworks*/

#endif /* _OS_DATATYPES_H */

