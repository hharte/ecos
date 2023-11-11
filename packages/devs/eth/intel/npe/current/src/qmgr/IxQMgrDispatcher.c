/**
 * @file    IxQMgrDispatcher.c
 *
 * @author Intel Corporation
 * @date    20-Dec-2001
 *    
 * @brief   This file contains the implementation of the Dispatcher sub component
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

/*
 * User defined include files.
 */
#include "IxAssert.h"
#include "IxQMgr.h"
#include "IxQMgrAqmIf_p.h"
#include "IxQMgrQCfg_p.h"
#include "IxQMgrDispatcher_p.h"
#include "IxQMgrLog_p.h"
#include "IxQMgrDefines_p.h"

/*
 * #defines and macros used in this file.
 */


/*
 * This constant is used to indicate the number of priority levels supported
 */
#define IX_QMGR_NUM_PRIORITY_LEVELS 3

/* 
 * This constant is used to set the size of the array of status words
 */
#define MAX_Q_STATUS_WORDS      4

/*
 *
 * This macro is used to declare a lock to protect a critical section of code
 *
 * 
 */
#define IX_QMGR_DISPATCHER_CRITICAL_SECTION_DECLARE() int ixQMgrLockKey

/*
 *
 * This macro is used to obtain a lock to protect a critical section of code
 *
 * 
 */
#define IX_QMGR_DISPATCHER_CRITICAL_SECTION_BEGIN() ixQMgrLockKey = ixOsServIntLock()

/*
 * This macro is used to release a lock previously obtained on a critical section
 * of code
 *
 * 
 */
#define IX_QMGR_DISPATCHER_CRITICAL_SECTION_END() ixOsServIntUnlock(ixQMgrLockKey)

/*
 * This macro is used to check if a given priority is valid
 */
#define IX_QMGR_DISPATCHER_PRIORITY_CHECK(priority) \
(((priority) >= IX_QMGR_Q_PRIORITY_0) && ((priority) <= IX_QMGR_Q_PRIORITY_2))

/*
 * This macto is used to check that a given interrupt source is valid
 */
#define IX_QMGR_DISPATCHER_SOURCE_ID_CHECK(srcSel) \
(((srcSel) >= IX_QMGR_Q_SOURCE_ID_E) && ((srcSel) <= IX_QMGR_Q_SOURCE_ID_NOT_F))

/*
 * Number of times a dummy callback is called before logging a trace
 * message
 */
#define LOG_THROTTLE_COUNT 1000000

/* Priority tables limits */
#define IX_QMGR_MIN_LOW_QUE_PRIORITY_TABLE_INDEX (0)
#define IX_QMGR_MID_LOW_QUE_PRIORITY_TABLE_INDEX (16)
#define IX_QMGR_MAX_LOW_QUE_PRIORITY_TABLE_INDEX (31)
#define IX_QMGR_MIN_UPP_QUE_PRIORITY_TABLE_INDEX (32)
#define IX_QMGR_MID_UPP_QUE_PRIORITY_TABLE_INDEX (48)
#define IX_QMGR_MAX_UPP_QUE_PRIORITY_TABLE_INDEX (63)

/*
 * Typedefs whose scope is limited to this file.
 */

/*
 * Information on a queue needed by the Dispatcher
 */
typedef struct
{
    IxQMgrCallback callback;       /* Notification callback                  */
    IxQMgrCallbackId callbackId;   /* Notification callback identifier       */
    unsigned dummyCallbackCount;   /* Number of times runs of dummy callback */
    IxQMgrPriority priority;       /* Dispatch priority                      */
    unsigned int statusWordOffset; /* Offset to the status word to check     */
    UINT32 statusMask;             /* Status mask                            */    
    UINT32 statusCheckValue;       /* Status check value                     */
    UINT32 intRegCheckMask;	   /* Interrupt register check mask          */
} IxQMgrQInfo;

/*
 * Variable declarations global to this file. Externs are followed by
 * statics.
 */

/*
 * This array contains a list of queue identifiers ordered by priority. The table
 * is split logically between queue identifiers 0-31 and 32-63.
 */
static IxQMgrQId priorityTable[IX_QMGR_MAX_NUM_QUEUES];

/*
 * This flag indicates to the dispatcher that the priority table needs to be rebuilt.
 */
static BOOL rebuildTable = FALSE;

/* Dispatcher statistics */
static IxQMgrDispatcherStats dispatcherStats;

/* Table of queue information */
static IxQMgrQInfo dispatchQInfo[IX_QMGR_MAX_NUM_QUEUES];

/* Masks use to identify the first queues in the priority tables 
*  when comparing with the interrupt register
*/
static unsigned int lowPriorityTableFirstHalfMask;
static unsigned int uppPriorityTableFirstHalfMask;

/*
 * Static function prototypes
 */

/*
 * This function is the default callback for all queues
 */
PRIVATE void
dummyCallback (IxQMgrQId qId,	      
	       IxQMgrCallbackId cbId);

PRIVATE void
ixQMgrDispatcherReBuildPriorityTable (void);

/*
 * Function definitions.
 */
void
ixQMgrDispatcherInit (void)
{
    int i;

    /* Set default priorities */
    for (i=0; i< IX_QMGR_MAX_NUM_QUEUES; i++)
    {
	dispatchQInfo[i].callback = dummyCallback;
	dispatchQInfo[i].callbackId = 0;
	dispatchQInfo[i].dummyCallbackCount = 0;
	dispatchQInfo[i].priority = IX_QMGR_Q_PRIORITY_2;
	dispatchQInfo[i].statusWordOffset = 0;
	dispatchQInfo[i].statusCheckValue = 0;
	dispatchQInfo[i].statusMask = 0;  
        /* 
	 * There are two interrupt registers, 32 bits each. One for the lower
	 * queues(0-31) and one for the upper queues(32-63). Therefore need to
	 * mod by 32 i.e the min upper queue identifier.
	 */
	dispatchQInfo[i].intRegCheckMask = (1<<(i%(IX_QMGR_MIN_QUEUPP_QID)));

	/* Reset queue statistics */
	dispatcherStats.queueStats[i].callbackCnt = 0;
	dispatcherStats.queueStats[i].priorityChangeCnt = 0;
	dispatcherStats.queueStats[i].intNoCallbackCnt = 0;
	dispatcherStats.queueStats[i].intLostCallbackCnt = 0;

    }

    /* Priority table. Order the table from queue 0 to 63 */
    ixQMgrDispatcherReBuildPriorityTable();

    /* Reset statistics */
    dispatcherStats.loopRunCnt = 0;

}

IX_STATUS
ixQMgrDispatcherPrioritySet (IxQMgrQId qId,
			     IxQMgrPriority priority)
{   
    IX_QMGR_DISPATCHER_CRITICAL_SECTION_DECLARE();

    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }
    
    if (!IX_QMGR_DISPATCHER_PRIORITY_CHECK(priority))
    {
	return IX_QMGR_Q_INVALID_PRIORITY;
    }

    IX_QMGR_DISPATCHER_CRITICAL_SECTION_BEGIN();
    
    /* Change priority */
    dispatchQInfo[qId].priority = priority;
    /* Set flag */
    rebuildTable = TRUE;

    IX_QMGR_DISPATCHER_CRITICAL_SECTION_END();

#if IX_QMGR_STATS_UPDATE_ENABLED
    /* Update statistics */
    dispatcherStats.queueStats[qId].priorityChangeCnt++;
#endif

    return IX_SUCCESS;
}

IX_STATUS
ixQMgrNotificationCallbackSet (IxQMgrQId qId,
			       IxQMgrCallback callback,
			       IxQMgrCallbackId callbackId)
{
    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }

    if (NULL == callback)
    {
	/* Reset to dummy callback */
	dispatchQInfo[qId].callback = dummyCallback;
	dispatchQInfo[qId].dummyCallbackCount = 0;
	dispatchQInfo[qId].callbackId = 0;
    }
    else 
    {
	dispatchQInfo[qId].callback = callback;
	dispatchQInfo[qId].callbackId = callbackId;
    }

    return IX_SUCCESS;
}

IX_STATUS
ixQMgrNotificationEnable (IxQMgrQId qId, 
			  IxQMgrSourceId srcSel)
{
    IxQMgrQStatus qStatusOnEntry;/* The queue status on entry/exit */
    IxQMgrQStatus qStatusOnExit; /* to this function               */
    IX_QMGR_DISPATCHER_CRITICAL_SECTION_DECLARE();

#if IX_QMGR_PARM_CHECKS_ENABLED
    if (!ixQMgrQIsConfigured (qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }

    if ((qId < IX_QMGR_MIN_QUEUPP_QID) &&
       !IX_QMGR_DISPATCHER_SOURCE_ID_CHECK(srcSel))
    {
	/* QId 0-31 source id invalid */
	return IX_QMGR_INVALID_INT_SOURCE_ID;
    }

    if ((IX_QMGR_Q_SOURCE_ID_NE != srcSel) &&
	(qId >= IX_QMGR_MIN_QUEUPP_QID))
    {
	/*
	 * For queues 32-63 the interrupt source is fixed to the Nearly
	 * Empty status flag and therefore should have a srcSel of NE.
	 */
	return IX_QMGR_INVALID_INT_SOURCE_ID;
    }
#endif


    /* Get the current queue status */
    ixQMgrAqmIfQueStatRead (qId, &qStatusOnEntry);
  
    /* 
     * Enabling interrupts results in Read-Modify-Write
     * so need critical section
     */
    IX_QMGR_DISPATCHER_CRITICAL_SECTION_BEGIN();

    /* Calculate the checkMask and checkValue for this q */
    ixQMgrAqmIfQStatusCheckValsCalc (qId,
				     srcSel,
				     &dispatchQInfo[qId].statusWordOffset,
				     &dispatchQInfo[qId].statusCheckValue,
				     &dispatchQInfo[qId].statusMask);

    /* Set the interupt source is this queue is in the range 0-31 */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
	ixQMgrAqmIfIntSrcSelWrite (qId, srcSel);

    }

    /* Enable the interrupt */
    ixQMgrAqmIfQInterruptEnable (qId);

    IX_QMGR_DISPATCHER_CRITICAL_SECTION_END();
    
    /* Get the current queue status */
    ixQMgrAqmIfQueStatRead (qId, &qStatusOnExit);
  
    /* If the status has changed return a warning */
    if (qStatusOnEntry != qStatusOnExit)
    {
	return IX_QMGR_WARNING;
    }
    
    return IX_SUCCESS;
}

IX_STATUS
ixQMgrNotificationDisable (IxQMgrQId qId)
{
    IX_QMGR_DISPATCHER_CRITICAL_SECTION_DECLARE();

#if IX_QMGR_PARM_CHECKS_ENABLED
    /* Validate parameters */
    if (!ixQMgrQIsConfigured (qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }
#endif
  
    /* 
     * Enabling interrupts results in Read-Modify-Write
     * so need critical section
     */
    IX_QMGR_DISPATCHER_CRITICAL_SECTION_BEGIN();

    ixQMgrAqmIfQInterruptDisable (qId);
    
    IX_QMGR_DISPATCHER_CRITICAL_SECTION_END();

    return IX_SUCCESS;    
}

#if !defined(__XSCALE__) || defined(__ECOS)

/* Count the number of leading zero bits in a word,
 * and return the same value than the CLZ instruction.
 *
 * word (in)    return value (out)
 * 0x80000000   0
 * 0x40000000   1
 * ,,,          ,,,
 * 0x00000002   30
 * 0x00000001   31
 * 0x00000000   32
 *
 * The C version of this function is used as a replacement 
 * for system not providing the equivalent of the CLZ 
 * assembly language instruction.
 *
 * Note that this version is big-endian
 */
unsigned int
ixQMgrCountLeadingZeros(UINT32 word)
{
#ifdef __ECOS
  asm volatile ("clz %0, %0\n" : "=r" (word) : );
  return word;
#else
  unsigned int leadingZerosCount = 0;

  if (word == 0)
  {
      return 32;
  }
  /* search the first bit set by testing the MSB and shifting the input word */
  while ((word & 0x80000000) == 0)
  {
      word <<= 1;
      leadingZerosCount++;
  }
  return leadingZerosCount;
#endif
}
    
#endif /* ndef __XSCALE__ */

void
ixQMgrDispatcherLoopRun (IxQMgrDispatchGroup group)
{
    UINT32 intRegVal;                /* Interrupt reg val */
    UINT32 intRegValAfterWrite;      /* Interrupt reg val after writing back */
    UINT32 intRegCheckMask;          /* Mask for checking interrupt bits */
    UINT32 qStatusWordsB4Write[MAX_Q_STATUS_WORDS];  /* Status b4 interrupt write */
    UINT32 qStatusWordsAfterWrite[MAX_Q_STATUS_WORDS]; /* Status after interrupt write */
    IxQMgrQInfo *currDispatchQInfo;
    BOOL statusChangeFlag;

    int priorityTableIndex;/* Priority table index */
    int qIndex;            /* Current queue being processed */
    int endIndex;          /* Index of last queue to process */

#if IX_QMGR_PARM_CHECKS_ENABLED
    IX_ASSERT((group == IX_QMGR_QUEUPP_GROUP) || 
	      (group == IX_QMGR_QUELOW_GROUP));
#endif

    /* Read Q status registers before interrupt status read/write */
    ixQMgrAqmIfQStatusRegsRead (group, qStatusWordsB4Write);

    /* Read the interrupt register */
    ixQMgrAqmIfQInterruptRegRead (group, &intRegVal);

    /* No bit set : nothing to process (the reaminder of the algorithm is
    * based on the fact that the interrupt register value contains at
    * least one bit set
    */
    if (intRegVal == 0) 
    {
#if IX_QMGR_STATS_UPDATE_ENABLED
	/* Update statistics */
	dispatcherStats.loopRunCnt++;
#endif

	/* Rebuild the priority table if needed */
	if (rebuildTable)
	{
	    ixQMgrDispatcherReBuildPriorityTable ();
	}

	return;
    }
   
    /* Write it back to clear the interrupt */
    ixQMgrAqmIfQInterruptRegWrite (group, intRegVal);

    /* Read Q status registers after interrupt status read/write */
    ixQMgrAqmIfQStatusRegsRead (group, qStatusWordsAfterWrite);

    /* get the first queue Id from the interrupt register value */
    qIndex = (BITS_PER_WORD - 1) - ixQMgrCountLeadingZeros(intRegVal);

    /* check if any change occured during hw register modifications */ 
    if (IX_QMGR_QUELOW_GROUP == group)
    {
	statusChangeFlag = 
	    (qStatusWordsB4Write[0] != qStatusWordsAfterWrite[0]) ||
	    (qStatusWordsB4Write[1] != qStatusWordsAfterWrite[1]) ||
	    (qStatusWordsB4Write[2] != qStatusWordsAfterWrite[2]) ||
	    (qStatusWordsB4Write[3] != qStatusWordsAfterWrite[3]);
    }
    else
    {
	statusChangeFlag = 
	    (qStatusWordsB4Write[0] != qStatusWordsAfterWrite[0]);
	/* Set the queue range based on the queue group to proccess */
	qIndex += IX_QMGR_MIN_QUEUPP_QID;
    }

    if (statusChangeFlag == FALSE)
    {
	/* check if the interrupt register contains 
	 * only 1 bit set (happy day scenario)
	 */
	currDispatchQInfo = &dispatchQInfo[qIndex];
	if (intRegVal == currDispatchQInfo->intRegCheckMask)
	{
	    /* only 1 queue event triggered a notification *
	     * Call the callback function for this queue 
	     */
	    currDispatchQInfo->callback (qIndex,
					 currDispatchQInfo->callbackId);
#if IX_QMGR_STATS_UPDATE_ENABLED
	    /* Update statistics */
	    dispatcherStats.queueStats[qIndex].callbackCnt++;
#endif
	}
	else 
	{
	    /* the event is triggered by more than 1 queue, 
	     * the queue search will be starting from the beginning
	     * or the middle of the priority table
	     *
	     * the serach will end when all the bits of the interrupt
	     * register are cleared. There is no need to maintain
	     * a seperate value and test it at each iteration.
	     */
	    if (IX_QMGR_QUELOW_GROUP == group)
	    {
		/* check if any bit related to queues in the first
		 * half of the priority table is set
		 */
		if (intRegVal & lowPriorityTableFirstHalfMask)
		{
		    priorityTableIndex = IX_QMGR_MIN_LOW_QUE_PRIORITY_TABLE_INDEX;
		}
		else
		{
		    priorityTableIndex = IX_QMGR_MID_LOW_QUE_PRIORITY_TABLE_INDEX;
		}
	    }
	    else 
	    {
		/* check if any bit related to queues in the first
		 * half of the priority table is set
		 */
		if (intRegVal & uppPriorityTableFirstHalfMask)
		{
		    priorityTableIndex = IX_QMGR_MIN_UPP_QUE_PRIORITY_TABLE_INDEX;
		}
		else
		{
		    priorityTableIndex = IX_QMGR_MID_UPP_QUE_PRIORITY_TABLE_INDEX;
		}
	    }
	    
	    /* iterate following the priority table until all the bits 
	     * of the interrupt register are cleared.
	     */
	    do
	    {
		qIndex = priorityTable[priorityTableIndex++];
		currDispatchQInfo = &dispatchQInfo[qIndex];
		intRegCheckMask = currDispatchQInfo->intRegCheckMask;
		
		/* If this queue caused this interrupt to be raised */
		if (intRegVal & intRegCheckMask)
		{
		    /* Call the callback function for this queue */
		    currDispatchQInfo->callback (qIndex,
						 currDispatchQInfo->callbackId);
#if IX_QMGR_STATS_UPDATE_ENABLED
		    /* Update statistics */
		    dispatcherStats.queueStats[qIndex].callbackCnt++;
#endif
		    
		    /* Clear the interrupt register bit */
		    intRegVal &= ~intRegCheckMask;
		}
	    }
	    while(intRegVal);
	}
    }
    else
    {
    /* A change in queue status occured during the hw interrupt
     * register update. To maintain the interrupt consistency, it
     * is necessary to iterate through all queues of the queue group.
     */

    /* Read interrupt status again */
    ixQMgrAqmIfQInterruptRegRead (group, &intRegValAfterWrite);

    if (IX_QMGR_QUELOW_GROUP == group)
    {
	priorityTableIndex = IX_QMGR_MIN_LOW_QUE_PRIORITY_TABLE_INDEX;
	endIndex = IX_QMGR_MAX_LOW_QUE_PRIORITY_TABLE_INDEX;
    }
    else
    {
	priorityTableIndex = IX_QMGR_MIN_UPP_QUE_PRIORITY_TABLE_INDEX;
	endIndex = IX_QMGR_MAX_UPP_QUE_PRIORITY_TABLE_INDEX;
    }

    for ( ; priorityTableIndex<=endIndex; priorityTableIndex++)
    {
	qIndex = priorityTable[priorityTableIndex];
	currDispatchQInfo = &dispatchQInfo[qIndex];
	intRegCheckMask = currDispatchQInfo->intRegCheckMask;

	/* If this queue caused this interrupt to be raised */
	if (intRegVal & intRegCheckMask)
	{
	    /* Call the callback function for this queue */
	    currDispatchQInfo->callback (qIndex,
					 currDispatchQInfo->callbackId);
#if IX_QMGR_STATS_UPDATE_ENABLED
	    /* Update statistics */
	    dispatcherStats.queueStats[qIndex].callbackCnt++;
#endif
	    
	} /* if (intRegVal .. */

	/* 
	 * If interrupt bit is set in intRegValAfterWrite don't
	 * proceed as this will be caught in next interrupt
	 */
	else if ((intRegValAfterWrite & intRegCheckMask) == 0)
	{
	    /* Check if an interrupt was lost for this Q */
	    if (ixQMgrAqmIfQStatusCheck(qStatusWordsB4Write,
					qStatusWordsAfterWrite,
					currDispatchQInfo->statusWordOffset,
					currDispatchQInfo->statusCheckValue,
					currDispatchQInfo->statusMask))
	    {
		/* Call the callback function for this queue */
		currDispatchQInfo->callback (qIndex,
					     dispatchQInfo[qIndex].callbackId);
#if IX_QMGR_STATS_UPDATE_ENABLED
		/* Update statistics */
		dispatcherStats.queueStats[qIndex].callbackCnt++;
		dispatcherStats.queueStats[qIndex].intLostCallbackCnt++;
#endif
	    } /* if ixQMgrAqmIfQStatusCheck(.. */
	} /* else if ((intRegValAfterWrite ... */
    } /* for (priorityTableIndex=0 ... */
    }

    /* Rebuild the priority table if needed */
    if (rebuildTable)
    {
	ixQMgrDispatcherReBuildPriorityTable ();
    }

#if IX_QMGR_STATS_UPDATE_ENABLED
    /* Update statistics */
    dispatcherStats.loopRunCnt++;
#endif
}

PRIVATE void
ixQMgrDispatcherReBuildPriorityTable (void)
{
    int priority;
    int qIndex;
    int lowQuePriorityTableIndex = IX_QMGR_MIN_LOW_QUE_PRIORITY_TABLE_INDEX;
    int uppQuePriorityTableIndex = IX_QMGR_MIN_UPP_QUE_PRIORITY_TABLE_INDEX;

    /* Reset the rebuild flag */
    rebuildTable = FALSE;

    /* initialize the mak used to identify the queues in the first half
     * of the priority table
     */
    lowPriorityTableFirstHalfMask = 0;
    uppPriorityTableFirstHalfMask = 0;
    
    /* For each priority level */
    for(priority=0; priority<IX_QMGR_NUM_PRIORITY_LEVELS; priority++)
    {
	/* Foreach low queue in this priority */
	for(qIndex=0; qIndex<IX_QMGR_MIN_QUEUPP_QID; qIndex++)
	{
	    if (dispatchQInfo[qIndex].priority == priority)
	    { 
		/* build the priority table bitmask which match the
		 * queues of the first half of the priority table 
		 */
		if (lowQuePriorityTableIndex < IX_QMGR_MID_LOW_QUE_PRIORITY_TABLE_INDEX) 
		{
		    lowPriorityTableFirstHalfMask |= dispatchQInfo[qIndex].intRegCheckMask;
		}
		/* build the priority table */
		priorityTable[lowQuePriorityTableIndex++] = qIndex;
	    }
	}
	/* Foreach upp queue */
	for(qIndex=IX_QMGR_MIN_QUEUPP_QID; qIndex<=IX_QMGR_MAX_QID; qIndex++)
	{
	    if (dispatchQInfo[qIndex].priority == priority)
	    {
		/* build the priority table bitmask which match the
		 * queues of the first half of the priority table 
		 */
		if (uppQuePriorityTableIndex < IX_QMGR_MID_UPP_QUE_PRIORITY_TABLE_INDEX) 
		{
		    uppPriorityTableFirstHalfMask |= dispatchQInfo[qIndex].intRegCheckMask;
		}
		/* build the priority table */
		priorityTable[uppQuePriorityTableIndex++] = qIndex;
	    }
	}
    }
}

IxQMgrDispatcherStats*
ixQMgrDispatcherStatsGet (void)
{
    return &dispatcherStats;
}

PRIVATE void
dummyCallback (IxQMgrQId qId,
	       IxQMgrCallbackId cbId)
{
    /* Throttle the trace message */
    if ((dispatchQInfo[qId].dummyCallbackCount % LOG_THROTTLE_COUNT) == 0)
    {
	IX_QMGR_LOG_WARNING2("--> dummyCallback: qId (%d), callbackId (%d)\n",qId,cbId);
    }
    dispatchQInfo[qId].dummyCallbackCount++;

#if IX_QMGR_STATS_UPDATE_ENABLED
    /* Update statistcs */
    dispatcherStats.queueStats[qId].intNoCallbackCnt++;
#endif
}
