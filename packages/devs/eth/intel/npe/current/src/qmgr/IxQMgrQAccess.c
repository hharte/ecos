/**
 * @file    IxQMgrQAccess.c
 *
 * @author Intel Corporation
 * @date    30-Oct-2001
 *
 * @brief   This file contains functions for putting entries on a queue and
 * removing entries from a queue.
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
 * Inlines are compiled as function when this is defined.
 * N.B. Must be placed before #include of "IxQMgr.h"
 */
#ifndef IXQMGR_H
#    define IXQMGRQACCESS_C
#else
#    error
#endif

/*
 * System defined include files.
 */

/*
 * User defined include files.
 */
#include "IxQMgr.h"
#include "IxQMgrAqmIf_p.h"
#include "IxQMgrQAccess_p.h"
#include "IxQMgrQCfg_p.h"
#include "IxQMgrDefines_p.h"

/*
 * Variable declarations global to this file. Externs are followed by
 * statics.
 */
#if IX_QMGR_STATS_UPDATE_ENABLED
static IxQMgrQAccessStats stats;
#endif

/*
 * Function definitions.
 */
void
ixQMgrQAccessInit (void)
{   
#if IX_QMGR_STATS_UPDATE_ENABLED
    int i;
    for (i=0; i<IX_QMGR_MAX_NUM_QUEUES; i++)
    {
	stats.qStats[i].readCnt = 0;
	stats.qStats[i].underflowCnt = 0;
	stats.qStats[i].writeCnt = 0;
	stats.qStats[i].overflowCnt = 0;
	stats.qStats[i].statusGetCnt = 0;
	stats.qStats[i].peekCnt = 0;
	stats.qStats[i].pokeCnt = 0;
    }
#endif
}

IX_STATUS
ixQMgrQReadWithChecks (IxQMgrQId qId,
                       UINT32 *entry)
{
    IxQMgrQEntrySizeInWords entrySizeInWords;

    if (NULL == entry)
    {
	return IX_QMGR_PARAMETER_ERROR;
    }

    /* Check QId */
    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }

    /* Get the q entry size in words */
    entrySizeInWords = ixQMgrQEntrySizeInWordsGet (qId);

    ixQMgrAqmIfQPop (qId, entrySizeInWords, entry);	    

    /* Check if underflow occurred on the read */
    if (ixQMgrAqmIfUnderflowCheck (qId))
    {
#if IX_QMGR_STATS_UPDATE_ENABLED
	/* Update statistics */
	stats.qStats[qId].underflowCnt++;
#endif
	return IX_QMGR_Q_UNDERFLOW;
    }
    
#if IX_QMGR_STATS_UPDATE_ENABLED
    /* Update statistics */
    stats.qStats[qId].readCnt++;
#endif

    return IX_SUCCESS;
}

IX_STATUS
ixQMgrQWriteWithChecks (IxQMgrQId qId,
                        UINT32 *entry)
{
    IxQMgrQEntrySizeInWords entrySizeInWords;

    if (NULL == entry)
    {
	return IX_QMGR_PARAMETER_ERROR;
    }

    /* Check QId */
    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }

    /* Get the q entry size in words */
    entrySizeInWords = ixQMgrQEntrySizeInWordsGet (qId);
    
    ixQMgrAqmIfQPush (qId, entrySizeInWords, entry);

    /* Check if overflow occurred on the write*/
    if (ixQMgrAqmIfOverflowCheck (qId))
    {
#if IX_QMGR_STATS_UPDATE_ENABLED
	/* Update statistics */
	stats.qStats[qId].overflowCnt++;
#endif
	return IX_QMGR_Q_OVERFLOW;
    }
         
#if IX_QMGR_STATS_UPDATE_ENABLED
    /* Update statistics */
    stats.qStats[qId].writeCnt++;
#endif

    return IX_SUCCESS;
}

IX_STATUS
ixQMgrQPeek (IxQMgrQId qId,
	     unsigned int entryIndex,
	     UINT32 *entry)
{
    unsigned int numEntries;

#if IX_QMGR_PARM_CHECKS_ENABLED
    if ((NULL == entry) || (entryIndex >= IX_QMGR_Q_SIZE_INVALID))
    {
	return IX_QMGR_PARAMETER_ERROR;
    }

    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }
#endif
    
    if (IX_SUCCESS != ixQMgrQNumEntriesGet (qId, &numEntries))
    {
	return IX_FAIL;
    }

    if (entryIndex >= numEntries) /* entryIndex starts at 0 */
    {
	return IX_QMGR_ENTRY_INDEX_OUT_OF_BOUNDS;
    }

#if IX_QMGR_STATS_UPDATE_ENABLED
    /* Update stats */
    stats.qStats[qId].peekCnt++;
#endif

    return ixQMgrAqmIfQPeek (qId, entryIndex, entry);
}

IX_STATUS
ixQMgrQPoke (IxQMgrQId qId,
	     unsigned entryIndex,
	     UINT32 *entry)
{
    unsigned int numEntries;

#if IX_QMGR_PARM_CHECKS_ENABLED
    if ((NULL == entry) || (entryIndex > 128))
    {
	return IX_QMGR_PARAMETER_ERROR;
    }

    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }
#endif
        
    if (IX_SUCCESS != ixQMgrQNumEntriesGet (qId, &numEntries))
    {
	return IX_FAIL;
    }

    if (numEntries < (entryIndex + 1)) /* entryIndex starts at 0 */
    {
	return IX_QMGR_ENTRY_INDEX_OUT_OF_BOUNDS;
    }

#if IX_QMGR_STATS_UPDATE_ENABLED
    /* Update stats */
    stats.qStats[qId].pokeCnt++;
#endif

    return ixQMgrAqmIfQPoke (qId, entryIndex, entry);
}

IX_STATUS
ixQMgrQStatusGetWithChecks (IxQMgrQId qId,
                            IxQMgrQStatus *qStatus)
{
    if (NULL == qStatus)
    {
	return IX_QMGR_PARAMETER_ERROR;
    }
   
    if (!ixQMgrQIsConfigured (qId)) 
    {
        return IX_QMGR_Q_NOT_CONFIGURED;
    }

    ixQMgrAqmIfQueStatRead (qId, qStatus);

#if IX_QMGR_STATS_UPDATE_ENABLED
    /* Update statistics */
    stats.qStats[qId].statusGetCnt++;
#endif

    return IX_SUCCESS;
}

IX_STATUS
ixQMgrQNumEntriesGet (IxQMgrQId qId,
		      unsigned *numEntries)
{
    UINT32 readPtr;
    UINT32 writePtr;
    UINT32 qStatus;
    UINT32 qEntrySizeInWords;
    UINT32 qSizeInWords;

#if IX_QMGR_PARM_CHECKS_ENABLED
    if (NULL == numEntries)
    {
	return IX_QMGR_PARAMETER_ERROR;
    }

    /* Check QId */
    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }
#endif

    /* Get the q size in words */
    qSizeInWords = ixQMgrQSizeInWordsGet (qId);

    /* Get the entry size in words */
    qEntrySizeInWords = ixQMgrQEntrySizeInWordsGet (qId);
    
    /* Snapshot of read & write pointers */
    ixQMgrAqmIfQReadWritePointerGet (qId, (UINT32 **)&readPtr, (UINT32 **)&writePtr);

    if (readPtr != writePtr)
    {       
	/* Mod subtraction of pointers to get number of words in Q. */
	*numEntries = ((writePtr + qSizeInWords) - readPtr) & (qSizeInWords - 1);
	/* Divide by the entry size to get the number of entries */
	*numEntries /= qEntrySizeInWords;
    }
    else
    {
	/* 
	 * Could mean either full or empty queue
	 * so look at status
	 */
	ixQMgrAqmIfQueStatRead (qId, &qStatus);

	if (qId < IX_QMGR_MIN_QUEUPP_QID)
	{
	    if (qStatus & IX_QMGR_Q_STATUS_E_BIT_MASK)
	    {
		/* Empty */
		*numEntries = 0;
	    }
	    else if (qStatus & IX_QMGR_Q_STATUS_F_BIT_MASK)
	    {
		/* Full */
		*numEntries = qSizeInWords/qEntrySizeInWords;
	    }
	    else
	    {	    
		/* 
		 * Queue status and read/write pointers are volatile.
		 * The queue state has changed since we took the
		 * snapshot of the read and write pointers.
		 * Client can retry if they wish
		 */
		*numEntries = 0;
		return IX_QMGR_WARNING;
	    }
	}
	else /* It is an upper queue which does not have an empty status bit maintained */
	{
	    if (qStatus & IX_QMGR_Q_STATUS_F_BIT_MASK)
	    {
		/* The queue is Full at the time of snapshot. */
		*numEntries = qSizeInWords/qEntrySizeInWords;
	    }
	    else
	    {
  	       /* The queue is either empty, either moving,
	        * Client can retry if they wish
	        */
	        *numEntries = 0;
	        return IX_QMGR_WARNING;
	    }
	}
    }
    
    return IX_SUCCESS;
}


#if IX_QMGR_STATS_UPDATE_ENABLED
IxQMgrQAccessStats*
ixQMgrQAccessStatsGet (void)
{
    return &stats;
}
#endif

