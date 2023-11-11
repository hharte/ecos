/**
 * @file
 *
 * @brief The file contains implementation for the OS service layer.
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
#include <iv.h>
#include <intLib.h>
#include <sysLib.h>
#include <logLib.h>
#include <taskLib.h>
#include "ixp425.h"

#define VX_OSS_RETCODE(retCode) (((retCode) == OK) ? IX_SUCCESS : IX_FAIL)

#elif defined(__linux)
#include <asm/hardirq.h>
#include <asm/system.h>
#include <asm/arch/irqs.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/sched.h> 
#include <linux/slab.h>
#include "ixp425.h"
#elif defined(__ECOS)

#else /* POSIX */

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define POSIX_OSS_RETCODE(retCode) (((retCode) == 0) ? IX_SUCCESS : IX_FAIL)

#endif /* __vxworks */

#include "IxOsServices.h"

#define REG_GET(address)        (*((volatile int *) (address)))

static char* traceHeaders[] = {
    "",
    "[fatal] ",
    "[error] ",
    "[warning] ",
    "[message] ",
    "[debug1] ",
    "[debug2] ",
    "[debug3] "
};
 
static int logLevel = LOG_MESSAGE; /* by default trace everyting but debug messages */

/**
 * @defgroup IxOsServices IxOsServices - A very thin OS service layer for IX access components
 *
 * @{
 */
#if defined(__linux)
typedef struct isr_info_t
{
    void (*routine)(void *);
    void *parameter;
} isr_info_t;

static isr_info_t isr_info[NR_IRQS];

static void isr_proxy(int irg, void *dev_id, struct pt_regs *regs)
{
    isr_info_t *isr_info = (isr_info_t *)dev_id;
    if (isr_info && isr_info->routine)
        isr_info->routine(isr_info->parameter);    
}
#endif

PUBLIC IX_STATUS
ixOsServIntBind(int vector, void (*routine)(void *), void *parameter)
{
#ifdef __vxworks
  
#if CPU==SIMSPARCSOLARIS
    return IX_FAIL;
#else
    if (intConnect((IxVoidFnPtr *) vector, (IxVoidFnPtr) routine, (int) parameter) != OK)
    {
      return IX_FAIL;
    }
    
    return VX_OSS_RETCODE(intEnable(IVEC_TO_INUM(vector)));
#endif
#elif defined(__linux)
    if (isr_info[vector].routine)
	return IX_FAIL;
    
    isr_info[vector].routine = routine;
    isr_info[vector].parameter = parameter;
    
    if (request_irq(vector, isr_proxy, SA_SHIRQ, "csr", &isr_info[vector]))
	return IX_FAIL;

    return IX_SUCCESS;

#elif defined(__ECOS)
    return IX_FAIL;
#else /* POSIX */
    
    return IX_FAIL;
    
#endif /* __vxworks */
}

PUBLIC IX_STATUS
ixOsServIntUnbind(int vector)
{
#ifdef __vxworks
    return IX_FAIL;
#elif defined(__linux)
    if (!isr_info[vector].routine)
	return IX_FAIL;
    
    free_irq(vector, &isr_info[vector]);
    isr_info[vector].routine = 0;
    
    return IX_SUCCESS;

#else /* POSIX */
    
    return IX_FAIL;
    
#endif /* __vxworks */
}

PUBLIC int
ixOsServIntLock()
{
#ifdef __vxworks

#if CPU==SIMSPARCSOLARIS
    return IX_FAIL;
#else
    return intLock();
#endif
    
#elif defined(__linux)    
    int flags;
    save_flags(flags);
    cli();
    return flags;
#elif defined(__ECOS)
    int old;
    HAL_DISABLE_INTERRUPTS(old);
    return old;
#else
    
    return IX_FAIL;
    
#endif /* __vxworks */
}

PUBLIC void
ixOsServIntUnlock(int lockKey)
{
#ifdef __vxworks
  
#if CPU!=SIMSPARCSOLARIS
    intUnlock(lockKey);
#endif
    
#elif defined (__linux)
    restore_flags(lockKey);
#elif defined(__ECOS)
    HAL_RESTORE_INTERRUPTS(lockKey)
#endif /* __vxworks */
}

PUBLIC int
ixOsServIntLevelSet(int level)
{
#ifdef __vxworks
  
#if CPU==SIMSPARCSOLARIS
    return IX_FAIL;
#else
    return intLevelSet(level);
#endif
    
#elif defined(__linux)
    return IX_FAIL;   
#else
    
    return IX_FAIL;
    
#endif /* __vxworks */
}

PUBLIC IX_STATUS
ixOsServMutexInit(IxMutex *mutex)
{
#ifdef __vxworks
  
    *mutex = semBCreate(SEM_Q_PRIORITY, SEM_FULL);

    return *mutex != NULL ? IX_SUCCESS : IX_FAIL;

#elif defined(__linux)
    *mutex = (struct semaphore *)kmalloc(sizeof(struct semaphore), GFP_KERNEL);
    if (!mutex)
	return IX_FAIL;

    init_MUTEX(*mutex);
    return IX_SUCCESS; 
#elif defined(__ECOS)
    cyg_drv_mutex_init(mutex);
    return IX_SUCCESS; 
#else
    
    return POSIX_OSS_RETCODE(pthread_mutex_init(mutex, NULL));
    
#endif /* __vxworks */
}

PUBLIC IX_STATUS
ixOsServMutexLock(IxMutex *mutex)
{
#ifdef __vxworks
  
    return VX_OSS_RETCODE(semTake(*mutex, WAIT_FOREVER));
    
#elif defined(__linux)
    if (in_irq())
	BUG();
    down(*mutex);
    return IX_SUCCESS;
#elif defined(__ECOS)
    if (cyg_drv_mutex_lock(mutex))
	return IX_SUCCESS;
    return IX_FAIL;
#else
    
    return POSIX_OSS_RETCODE(pthread_mutex_lock(mutex));
    
#endif /* __vxworks */
}

PUBLIC IX_STATUS
ixOsServMutexUnlock(IxMutex *mutex)
{
#ifdef __vxworks
  
    return VX_OSS_RETCODE(semGive(*mutex));
    
#elif defined(__linux)
    up(*mutex);
    return IX_SUCCESS;
#elif defined(__ECOS)
    cyg_drv_mutex_unlock(mutex);
    return IX_SUCCESS;
#else
    
    return POSIX_OSS_RETCODE(pthread_mutex_unlock(mutex));
    
#endif /* __vxworks */
}

PUBLIC IX_STATUS
ixOsServMutexDestroy(IxMutex *mutex)
{
#ifdef __vxworks
  
    return VX_OSS_RETCODE(semDelete(*mutex));
    
#elif defined(__linux)    
    kfree(*mutex);
    return IX_SUCCESS;
#elif defined(__ECOS)
    cyg_drv_mutex_destroy(mutex);
    return IX_SUCCESS;
#else
    
    return POSIX_OSS_RETCODE(pthread_mutex_destroy(mutex));
    
#endif /* __vxworks */
}

PUBLIC IX_STATUS
ixOsServFastMutexInit(IxFastMutex *mutex)
{
#ifdef __vxworks
  
#if CPU==SIMSPARCSOLARIS
  
    *mutex = semBCreate(SEM_Q_PRIORITY, SEM_FULL);

    return *mutex != NULL ? IX_SUCCESS : IX_FAIL;
  
#else 
  
    if (mutex == NULL)
    {
        return IX_FAIL;
    }

    *mutex = 0;

    return IX_SUCCESS;
    
#endif
      
#elif defined(__linux)    
#ifdef IXP425_USE_FAST_MUTEX
    return mutex ? *mutex = 0, IX_SUCCESS : IX_FAIL;
#else
    return ixOsServMutexInit(mutex);
#endif
#elif defined(__ECOS)
    return ixOsServMutexInit(mutex);
#else
    
    return POSIX_OSS_RETCODE(pthread_mutex_init(mutex, NULL));
    
#endif /* __vxworks */
}
    

#if !defined(__XSCALE__) && !defined(IXP425_USE_FAST_MUTEX)
PUBLIC IX_STATUS
ixOsServFastMutexTryLock(IxMutex *mutex)
{
#if CPU==SIMSPARCSOLARIS && defined(__vxworks)

    return VX_OSS_RETCODE(semTake(*mutex, NO_WAIT));
  
#elif defined(__linux)
    return !down_trylock(*mutex) ? IX_SUCCESS : IX_FAIL;
#else
    
    return POSIX_OSS_RETCODE(pthread_mutex_trylock(mutex));
    
#endif
}
#elif defined(__linux)

asm("	.align	5 \
	.globl	ixOsServFastMutexTryLock \
ixOsServFastMutexTryLock: \
	mov		r1, #1 \
	swp		r2, r1, [r0] \
	mov		r0, r2 \
	mov		pc, lr \
	");

#elif defined(__ECOS)

PUBLIC IX_STATUS
ixOsServFastMutexTryLock(IxMutex *mutex)
{
    return ixOsServMutexUnlock(mutex);
}

#endif /* ndef __XSCALE__ */

PUBLIC IX_STATUS
ixOsServFastMutexUnlock(IxFastMutex *mutex)
{
#ifdef __vxworks
  
#if CPU==SIMSPARCSOLARIS

    return VX_OSS_RETCODE(semGive(*mutex));

#else
    
    if (mutex == NULL)
    {
        return IX_FAIL;
    }

    *mutex = 0;

    return IX_SUCCESS;

#endif
        
#elif defined(__linux)
#ifdef IXP425_USE_FAST_MUTEX
    return mutex ? *mutex = 0, IX_SUCCESS : IX_FAIL;
#else
    return ixOsServMutexUnlock(mutex);
#endif
#elif defined(__ECOS)
    return ixOsServMutexUnlock(mutex);
#else
    
    return POSIX_OSS_RETCODE(pthread_mutex_unlock(mutex));
    
#endif /* __vxworks */
}

#ifndef __vxworks

#ifdef __linux
#define logMsg printk
#else
#ifdef __ECOS
#define logMsg diag_printf
#else
#define logMsg printf
#endif
#endif

#endif /* __vxworks */


PUBLIC int
ixOsServLog(int level, char *format, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6)
{
    if (level <= logLevel && level != LOG_NONE)
    {
      
#ifndef __vxworks
      
        int headerByteCount = (level == LOG_USER) ? 0 : logMsg(traceHeaders[level - 1]);
        
#else
        
        int headerByteCount = (level == LOG_USER) ? 0 : logMsg(traceHeaders[level - 1], 0, 0, 0, 0, 0, 0);
        
#endif /* __vxworks */

        return headerByteCount + logMsg(format, arg1, arg2, arg3, arg4, arg5, arg6);
    }
    else
    {
        return 0;
    }
}

PUBLIC int
ixOsServLogLevelSet(int level)
{
    int oldLevel = logLevel;
    
    logLevel = level;

    return oldLevel;
}

#undef IXP425_PERIPHERAL_BUS_CLOCK
#define IXP425_PERIPHERAL_BUS_CLOCK 66

PUBLIC void
ixOsServSleep(int microseconds)
{
#ifdef __vxworks
  
    unsigned int delay = 0;
    unsigned int lastTimestamp = ixOsServTimestampGet();

    while (delay < microseconds * IXP425_PERIPHERAL_BUS_CLOCK)
    {
        unsigned int currentTimestamp = ixOsServTimestampGet();
        unsigned int delta = currentTimestamp > lastTimestamp ? currentTimestamp - lastTimestamp : 0xffffffff - lastTimestamp + currentTimestamp;

        delay += delta;

        lastTimestamp = currentTimestamp;
    }
    
#elif defined(__linux)
    if(microseconds>2000)
    {
	printk(KERN_WARNING "udelay for %d>2000! delay aborted!!\n",
	    microseconds);
    }
    udelay(microseconds);
#elif defined(__ECOS)
    CYGACC_CALL_IF_DELAY_US(microseconds);
#else /* POSIX */    

    struct timeval delayTime;
    
    delayTime.tv_sec  = 0; /* seconds */
    delayTime.tv_usec = microseconds; /* microseconds */
    
    select(0, NULL, NULL, NULL, &delayTime);
    
#endif
}

#if !defined(__vxworks) && !defined(__linux) && !defined(__ECOS)
#include <errno.h>
#include <string.h>
#include <poll.h>
#endif

PUBLIC void
ixOsServTaskSleep(int milliseconds)
{
#ifdef __vxworks
  int delay = (sysClkRateGet() * milliseconds) / 1000;

    /* Cover a rounding down to zero */ 
    if ( delay == 0  && milliseconds != 0) 
	++delay;
  
    taskDelay(delay);
    
#elif defined(__linux)
    if ( milliseconds != 0 )
	{
	    current->state = TASK_INTERRUPTIBLE;
	    schedule_timeout((milliseconds*HZ)/1000);
	}
    else
	{
	     schedule();
	}
#elif defined(__ECOS)
    CYGACC_CALL_IF_DELAY_US(milliseconds * 1000);
#else /* POSIX */
    struct pollfd foo;
    
    if (poll(&foo, (unsigned long) 0, milliseconds) == -1)
    {
        printf("poll failed with error [%s]\n", strerror(errno));
    }     
    
#endif /* __vxworks */
}

PUBLIC unsigned int
ixOsServTimestampGet()
{
#ifdef __vxworks
 
#if CPU==SIMSPARCSOLARIS
 
    return 0; /* no timestamp under VxSim */
 
#else
 
    return REG_GET(IXP425_OSTS);

#endif

#elif defined(__linux)

    return REG_GET(IXP425_OSTS);

#elif defined(__ECOS)
    unsigned val;
    HAL_CLOCK_READ(val);
    return val;
#else /* use POSIX microsecond timer */

    struct timeval tv;

    gettimeofday(&tv, NULL);

    return IXP425_PERIPHERAL_BUS_CLOCK  * (unsigned int) tv.tv_usec;

#endif /* __vxworks */
}

/**
 * @}
 */
