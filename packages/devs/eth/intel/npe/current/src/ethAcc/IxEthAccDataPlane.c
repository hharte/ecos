/**
 * @file IxEthDataPlane.c
 *
 * @author Intel Corporation
 * @date 12-Feb-2002
 *
 * @brief This file contains the implementation of the IXP425 Ethernet Access Data plane
 * Component
 *
 * Design Notes:
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
#include "IxNpeMh.h"
#include "IxEthAcc.h"
#include "IxEthAccFpathDep.h"
#include "IxEthDBPortDefs.h"
#include "IxEthAcc_p.h"
#include "IxEthAccQueueAssign_p.h"

#include <stdio.h>

PRIVATE INLINE UINT32 ixEthAccMbufQPrepare(IX_MBUF *mbuf, int flush_data)
{
    IX_MBUF *ptr;
    IX_MBUF *next_ptr;
    UINT32 qbuf;

    for (ptr = mbuf; ptr; ptr = next_ptr)
    {
	/* flush data only if required */
	if (flush_data)
	{
	    IX_ACC_DATA_CACHE_FLUSH(IX_MBUF_MDATA(ptr), IX_MBUF_MLEN(ptr));
	}

	/* virt2phys data pointer */
	IX_MBUF_MDATA(ptr) = (INT8 *)
	    IX_MMU_VIRTUAL_TO_PHYSICAL_TRANSLATION(IX_MBUF_MDATA(ptr));

	/* endianess swap data pointer */
	IX_MBUF_MDATA(ptr) = (INT8 *)
	    IX_ETH_ACC_WORD_QM_WORD_ENDIAN_SWAP(IX_MBUF_MDATA(ptr));

	/* we must save virtual next chain pointer */
	next_ptr = IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr);

	/* virt2phys next chain pointer */
	IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr) = (IX_MBUF *)
	    IX_MMU_VIRTUAL_TO_PHYSICAL_TRANSLATION(next_ptr);

	/* endianess next chain pointer */
	IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr) = (IX_MBUF *)
	    IX_ETH_ACC_WORD_QM_WORD_ENDIAN_SWAP(IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr));

	/* flush MBUF itself after all address conversions */
	IX_ACC_DATA_CACHE_FLUSH(ptr, sizeof(IX_MBUF));
    }

    /* virt2phys mbuf itself */
    qbuf = (UINT32)IX_MMU_VIRTUAL_TO_PHYSICAL_TRANSLATION(mbuf);

    /* mask mbuf to clear align and system used bits */
    qbuf &= IX_ETHNPE_QM_Q_RXENET_ADDR_MASK;

    /* endianess qbuf (phys) pointer */
    qbuf = IX_ETH_ACC_WORD_QM_WORD_ENDIAN_SWAP(qbuf);
    return qbuf;
}

PRIVATE INLINE IX_MBUF *ixEthAccMbufFromTxQ(UINT32 qbuf, int invalidate_data)
{
    IX_MBUF *ptr;
    IX_MBUF *mbuf;


    /* endianess qbuf (phys) pointer */
    /* qbuf = IX_ETH_ACC_WORD_QM_WORD_ENDIAN_SWAP(qbuf); - done by caller*/
    /* mask mbuf to cleare align and system used bits */
    qbuf &= IX_ETHNPE_QM_Q_RXENET_ADDR_MASK;

    /* restore the original address pointer (if PHYS_OFFSET is not 0) */
    qbuf |= IX_ACC_DRAM_PHYS_OFFSET & ~IX_ETHNPE_QM_Q_RXENET_ADDR_MASK;

    /* phys2virt mbuf itself */
    mbuf = IX_MMU_PHYSICAL_TO_VIRTUAL_TRANSLATION(qbuf);

    for (ptr = mbuf; ptr; ptr = IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr))
    {
        /* invalidate mbuf in for the case it is still in cache */
        IX_ACC_DATA_CACHE_INVALIDATE(ptr, sizeof(IX_MBUF));

        /* endianess swap data pointer */
        IX_MBUF_MDATA(ptr) = (UINT8 *)
            IX_ETH_ACC_WORD_QM_WORD_ENDIAN_SWAP(IX_MBUF_MDATA(ptr));

        /* phys2virt data pointer */
        IX_MBUF_MDATA(ptr) = (UINT8 *)
            IX_MMU_PHYSICAL_TO_VIRTUAL_TRANSLATION(IX_MBUF_MDATA(ptr));

        /* invalidate data only if required */
        if (invalidate_data)
            IX_ACC_DATA_CACHE_INVALIDATE(IX_MBUF_MDATA(ptr), IX_MBUF_MLEN(ptr));
        /* endianess next chain pointer */
        IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr) = (IX_MBUF *)
            IX_ETH_ACC_WORD_QM_WORD_ENDIAN_SWAP(
            IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr));

        /* phys2virt next chain pointer */
        IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr) = (IX_MBUF *)
            IX_MMU_PHYSICAL_TO_VIRTUAL_TRANSLATION(
            IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr));
    }
    return mbuf;
}

PRIVATE INLINE IX_MBUF *ixEthAccMbufFromRxQ(UINT32 qbuf, int invalidate_data)
{
    IX_MBUF *ptr;
    IX_MBUF *mbuf;

    /* endianess qbuf (phys) pointer */
    /* qbuf = IX_ETH_ACC_WORD_QM_WORD_ENDIAN_SWAP(qbuf); - done by caller*/
    /* mask mbuf to cleare align and system used bits */
    qbuf &= IX_ETHNPE_QM_Q_RXENET_ADDR_MASK;

    /* restore the original address pointer (if PHYS_OFFSET is not 0) */
    qbuf |= IX_ACC_DRAM_PHYS_OFFSET & ~IX_ETHNPE_QM_Q_RXENET_ADDR_MASK;

    /* phys2virt mbuf itself */
    mbuf = IX_MMU_PHYSICAL_TO_VIRTUAL_TRANSLATION(qbuf);

    for (ptr = mbuf; ptr; ptr = IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr))
    {
        /* invalidate mbuf in for the case it is still in cache */
        IX_ACC_DATA_CACHE_INVALIDATE(ptr, sizeof(IX_MBUF));

        /* XXX This looks like NPE Bug, we should nevere receive chain */
        if (IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr))
        {
	    IX_ETH_ACC_FATAL_LOG(
		"IXETHACC: BUFFER CHAIN RECEIVED FROM THE ETH NPE!: buf=%p, next=%p Fixing...\n  ", 
		ptr , IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr), 0, 0, 0, 0);
            /* Fix it, so we can continue */
            IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr) = NULL;
        }
        /* endianess swap data pointer */
        IX_MBUF_MDATA(ptr) = (UINT8 *)
            IX_ETH_ACC_WORD_QM_WORD_ENDIAN_SWAP(IX_MBUF_MDATA(ptr));

        /* phys2virt data pointer */
        IX_MBUF_MDATA(ptr) = (UINT8 *)
            IX_MMU_PHYSICAL_TO_VIRTUAL_TRANSLATION(IX_MBUF_MDATA(ptr));

        /* invalidate data only if required */
        if (invalidate_data)
            IX_ACC_DATA_CACHE_INVALIDATE(IX_MBUF_MDATA(ptr), IX_MBUF_MLEN(ptr));
    }

    return mbuf;
}

IX_MBUF *ixEthAccMbufFromSwQ(IX_MBUF *mbuf)
{
    /* process the mbuf and invalidate the mbuf payload */
    return ixEthAccMbufFromRxQ((UINT32)mbuf, 1);
}



/**
 * @addtogroup IxEthAccPri
 *@{
 */

extern ixEthAccPortDataInfo   ixEthAccPortData[];


IxEthAccDataPlaneStats     ixEthAccDataStats;    
#if IX_ETH_ACC_FPATH_AWARE
PRIVATE UINT32 txFramesCurrentlySubmitted[IX_ETH_ACC_NUMBER_OF_PORTS];
#endif

PRIVATE INLINE IxEthAccStatus
ixEthAccTxSwQHighestPriorityGet(IxEthAccPortId portId, 
				IxEthAccTxPriority *priorityPtr);
PRIVATE INLINE IxEthAccStatus
ixEthAccTxFromSwQ(IxEthAccPortId portId, 
		  IxEthAccTxPriority priority);

INLINE IX_STATUS 
ixEthAccQmgrTxWrite(IxEthAccPortId portId, UINT32 qBuffer)
{

#if IX_ETH_ACC_FPATH_AWARE
    if(txFramesCurrentlySubmitted[portId] <
       IX_ETH_ACC_MAX_TX_FRAMES_TO_SUBMIT)
    {
	return ixQMgrQWrite(
	    IX_ETH_ACC_PORT_TO_TX_Q_ID(portId),
	    &qBuffer);	    
    }
    else
    {
	return IX_QMGR_Q_OVERFLOW;
    }
    
#else
    return ixQMgrQWrite(
	IX_ETH_ACC_PORT_TO_TX_Q_ID(portId),
	&qBuffer);
#endif
    
}



IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccInitDataPlane()
{	
    IxEthAccStatus	ret = IX_ETH_ACC_SUCCESS;
    
    /* 
     * Initialize the service and register callback to other services.
     */

    IX_ETH_ACC_MEMSET(&ixEthAccDataStats, 0, sizeof(ixEthAccDataStats));
    return (ret);
}


IxEthAccStatus ixEthAccPortTxDoneCallbackRegister(IxEthAccPortId portId, 
						  IxEthAccPortTxDoneCallback
						  txCallbackFn, 
						  UINT32 callbackTag)
{	
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }
    if (!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
	return (IX_ETH_ACC_INVALID_PORT);
    }
    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    if (txCallbackFn == 0)
	/* Check for null function pointer here. */
    {
	return (IX_ETH_ACC_INVALID_ARG);
    }
    ixEthAccPortData[portId].ixEthAccTxData.txBufferDoneCallbackFn = txCallbackFn;
    ixEthAccPortData[portId].ixEthAccTxData.txCallbackTag = callbackTag;
    return (IX_ETH_ACC_SUCCESS);
}



IxEthAccStatus ixEthAccPortRxCallbackRegister(IxEthAccPortId portId, 
					      IxEthAccPortRxCallback
					      rxCallbackFn, UINT32
					      callbackTag)
{	
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }
    if (!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
	return (IX_ETH_ACC_INVALID_PORT);
    }
    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    
    /* Check for null function pointer here. */
    if (rxCallbackFn == NULL)
    {
	return (IX_ETH_ACC_INVALID_ARG);
    }
    ixEthAccPortData[portId].ixEthAccRxData.rxCallbackFn = rxCallbackFn;
    ixEthAccPortData[portId].ixEthAccRxData.rxCallbackTag = callbackTag;
    return (IX_ETH_ACC_SUCCESS);
}


IxEthAccStatus ixEthAccPortTxFrameSubmit(IxEthAccPortId portId, 
					 IX_MBUF *buffer, 
					 IxEthAccTxPriority priority)
{	
    IX_STATUS	qStatus = IX_SUCCESS;
    UINT32      qBuffer;
#if !IX_ETH_ACC_FPATH_AWARE
    IxQMgrQStatus txQStatus;    
#endif
    if (buffer == NULL)
    {
	return (IX_ETH_ACC_FAIL);
    }
# if IX_ETH_ACC_DATA_PLANE_FUNC_ARG_CHECKS 
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }
    if (!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
	return (IX_ETH_ACC_INVALID_PORT);
    }
    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
# endif 
    
    /*
     * Need to Flush the MBUF and its contents (data) as it may be
     * read from the NPE. Convert virtual addresses to physical addresses also.
     */
    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.txFrameCounter);

    qBuffer = ixEthAccMbufQPrepare(buffer, 1);
    /*
     * If no fifo priority ...
     */

    if (ixEthAccPortData[portId].ixEthAccTxData.schDiscipline == 
	FIFO_NO_PRIORITY)
    {
	/*
	 * Add The Tx Buffer to the H/W Tx Q if possible  
	 */
	qStatus = ixEthAccQmgrTxWrite(portId,qBuffer);

	if (qStatus == IX_SUCCESS)
	{
	    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.txQAddedOK);
	    IX_ETH_ACC_FPATH_DELTA_INC(txFramesCurrentlySubmitted[portId]);

	    /*  
	     * Buffer added to h/w Q.
	     */
	    return (IX_SUCCESS);
	}
	else if (qStatus == IX_QMGR_Q_OVERFLOW)
	{
	    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.txQOverflow);
	    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.txAddedToSWQ);
	    /*
	     * We were unable to write the buffer to the 
	     * appropriate H/W Q,  Save it in a s/w Q.
	     */
	    IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_TAIL(
		ixEthAccPortData[portId].
		ixEthAccTxData.txQ[IX_ETH_ACC_TX_DEFAULT_PRIORITY], 
		buffer);
		
	    qStatus = ixQMgrNotificationEnable(
		IX_ETH_ACC_PORT_TO_TX_Q_ID(portId),
		IX_ETH_ACC_PORT_TO_TX_Q_SOURCE(portId));
	}
	else 
	{
	    IX_ETH_ACC_FATAL_LOG(
		"IXETHACC:ixEthAccPortTxFrameSubmit:Error: qStatus = ", 
		qStatus, 0, 0, 0, 0, 0);
	    return (IX_ETH_ACC_FAIL);
	}
    }
    else if (ixEthAccPortData[portId].ixEthAccTxData.schDiscipline == 
	     FIFO_PRIORITY)
    {
	
	/*
	 * For priority transmission, put the frame directly on the H/W queue
	 * if the H/W queue is empty, otherwise, put it in a S/W Q
	 */
#if !IX_ETH_ACC_FPATH_AWARE
	/*
	 * For priority transmission, if Fast Path is not enabled, then 
	 * we only submit a prioritised frame if the tx q is empty
	 * For Fast Path enabled systems, we attempt to submit regardless,
	 * and rely on the ixEthAccQmgrTxWrite function's buffer submission
	 * tracking to signal an overflow if all non fast-path slots
	 * are occupied
	 */
	/*non fast path case*/
	ixQMgrQStatusGet(IX_ETH_ACC_PORT_TO_TX_Q_ID(portId), &txQStatus);
	if((txQStatus & IX_QMGR_Q_STATUS_E_BIT_MASK) != 0)
	{
	    /*The tx queue is empty, check whether there are buffers on the s/w queues*/
	    if(ixEthAccTxSwQHighestPriorityGet(portId,  &priority)
	       !=IX_ETH_ACC_FAIL)
	    {
		/*there are buffers on the s/w queues, submit from them*/
		if(ixEthAccTxFromSwQ(portId, priority)==IX_SUCCESS)
		{
		    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.
					 txSubmitHwRepFromSwQ);
		}
		/*and force the buffer supplied to be placed on a priority queue*/
		qStatus = IX_QMGR_Q_OVERFLOW;
	    }
	    else
	    {
		/*there are no buffers in the s/w queues, submit directly*/
		qStatus = ixEthAccQmgrTxWrite(portId, qBuffer);
	    }
	}
	else
	{
	    qStatus = IX_QMGR_Q_OVERFLOW;
	}
#else
	/*Fast path case*/
	/*First check if there are any non-fastpath slots available*/
	if(txFramesCurrentlySubmitted[portId] <
	   IX_ETH_ACC_MAX_TX_FRAMES_TO_SUBMIT)
	{
	    /*we need to pull a buffer off the S/w queues if one exists*/	    
	    if(ixEthAccTxSwQHighestPriorityGet(portId,  &priority)
	       !=IX_ETH_ACC_FAIL)
	    {
		/*there are buffers on the s/w queues, submit from them*/
		if(ixEthAccTxFromSwQ(portId, priority) == IX_SUCCESS)
		{
		    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.
					 txSubmitHwRepFromSwQ);
		}
		else
		{
		    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.
					 txSubmitHwRepFromSwQFailures);
		}
		/*and force the buffer supplied to be placed on a priority queue*/
		qStatus = IX_QMGR_Q_OVERFLOW;
	    }
	    else
	    {
		/*there are no buffers in the s/w queues, submit directly*/
		qStatus = ixEthAccQmgrTxWrite(portId, qBuffer);
	    }
	}
	else
	{
	    /*there are no non-fastpath slots available, so put the buffer on a 
	      s/w queue*/
	    qStatus = IX_QMGR_Q_OVERFLOW;
	}
#endif	    
	if(qStatus == IX_SUCCESS ) 
	{
	    IX_ETH_ACC_FPATH_DELTA_INC(txFramesCurrentlySubmitted[portId]);
	    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.txQAddedOK);
	    return IX_ETH_ACC_SUCCESS;
	}
	else if(qStatus == IX_QMGR_Q_OVERFLOW)
	{
	    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.txQOverflow);
	    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.txAddedToSWQ);
	    /*
	     * We were unable to write the buffer to the 
	     * appropriate H/W Q,  Save it in a s/w Q.
	     */
	    IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_TAIL(
		ixEthAccPortData[portId].
		ixEthAccTxData.txQ[IX_ETH_ACC_TX_DEFAULT_PRIORITY], 
		buffer);
	    
	    qStatus = ixQMgrNotificationEnable(
		IX_ETH_ACC_PORT_TO_TX_Q_ID(portId),
		IX_ETH_ACC_PORT_TO_TX_Q_SOURCE(portId));
	    if ( qStatus != IX_SUCCESS )  
	    {
		return(IX_ETH_ACC_FAIL);
	    }
	    return qStatus;
	    
	}
	else
	{
	    IX_ETH_ACC_FATAL_LOG(
		"IXETHACC:ixEthAccPortTxFrameSubmit:Error: %u\n", 
		qStatus, 0, 0, 0, 0, 0);
	}
	
    }
    else 
    {
	IX_ETH_ACC_FATAL_LOG(
	    "IXETHACC:ixEthAccPortTxFrameSubmit:Error: Null Mbuf Ptr", 
	    0, 0, 0, 0, 0, 0);
	return (IX_ETH_ACC_FAIL);
    }
    
    return (IX_ETH_ACC_SUCCESS);
}


/**
 * @fn PRIVATE
 *
 * @brief
 *
 * @param
 *
 * @return none
 *
 * @internal
 */

IxEthAccStatus ixEthAccPortRxFreeReplenish(IxEthAccPortId portId, 
					   IX_MBUF *buffer)
{	
    IX_STATUS	qStatus = IX_SUCCESS;
    UINT32      qBuffer;
    
    /* 
     * Check buffer is valid.
     */
    
    if (buffer == 0)
    {
	return (IX_ETH_ACC_FAIL);
    }
# if IX_ETH_ACC_DATA_PLANE_FUNC_ARG_CHECKS 
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }
    if (!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
	return (IX_ETH_ACC_INVALID_PORT);
    }
    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
# endif 
    
    /*
     * Need to Flush the MBUF only (not data) as it may be read from the NPE 
     */
    qBuffer = ixEthAccMbufQPrepare(buffer, 0);

    /*
     * Add The Rx Buffer to the H/W Free buffer Q if possible  
     */
    qStatus = ixQMgrQWrite(IX_ETH_ACC_PORT_TO_RX_FREE_Q_ID(portId), 
			   &qBuffer);
    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeReplenish);

    if (qStatus == IX_SUCCESS)
    {
	IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeReplenishOK);
	/* 
	 * Buffer added to h/w Q.
	 */
	return (IX_SUCCESS);
    }
    else if (qStatus == IX_QMGR_Q_OVERFLOW)
    {
	IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeReplenishOverflow);
	/*
	 * We were unable to write the buffer to the approprate H/W Q, 
	 * Save it in a s/w Q.
	 */
	IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_HEAD(
	    ixEthAccPortData[portId].ixEthAccRxData.freeBufferList, 
	    buffer);

	qStatus = ixQMgrNotificationEnable(
	    IX_ETH_ACC_PORT_TO_RX_FREE_Q_ID(portId),
	    IX_ETH_ACC_PORT_TO_RX_FREE_Q_SOURCE(portId));
    }
    else 
    {
	IX_ETH_ACC_FATAL_LOG(
	    "IXETHACC:ixEthAccRxPortFreeReplenish:Error: qStatus = ", 
	    qStatus, 0, 0, 0, 0, 0);
	return (IX_ETH_ACC_FAIL);
    }
    return (IX_ETH_ACC_SUCCESS);
}


IxEthAccStatus ixEthAccTxSchedulingDisciplineSet(IxEthAccPortId portId, 
						 IxEthAccTxSchedulerDiscipline
						 sched)
{	
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }
    if (!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
	return (IX_ETH_ACC_INVALID_PORT);
    }
    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    ixEthAccPortData[portId].ixEthAccTxData.schDiscipline = sched;
    return (IX_ETH_ACC_SUCCESS);
}
/**
 * @fn PRIVATE ixEthRxFrameQMCallback 
 *
 * @brief receive callback for Frame receive Q from NPE
 *
 * @param @ref IxQMgrCallback
 *
 * @return none
 *
 * @internal
 */
PRIVATE UINT32 rxQEntry[IX_ETH_ACC_MAX_RX_FRAME_CONSUME_PER_CALLBACK];
PRIVATE UINT32 rxQReadStatus;

void ixEthRxFrameQMCallback(IxQMgrQId qId, 
			    IxQMgrCallbackId callbackId
			    )
{	
    UINT32     qEntry;
    IX_MBUF    *mbufPtr;
    UINT32     portId;
    UINT32     i;
    UINT32     index;
    UINT32     tailIndex;
    BOOL       additionalReadsRequired = FALSE;

    /* 
     * Indication that there are Rx frames in the QM Rx Frame Q.
     */
    IX_ETH_ACC_STATS_INC(ixEthAccDataStats.rxFrameCounter);    

#if IX_ETH_ACC_QMGR_NO_ADDITIONAL_CALLBACKS    
    do
    {
#endif
	index=0;
	do
	{
	    rxQReadStatus = ixQMgrQRead(qId, &qEntry);
	    rxQEntry[index]      = IX_ETH_ACC_WORD_QM_WORD_ENDIAN_SWAP(qEntry);
	    index++;
	    
	} while((rxQReadStatus == IX_SUCCESS) &&
	    (index < IX_ETH_ACC_MAX_RX_FRAME_CONSUME_PER_CALLBACK));
	
	if (rxQReadStatus == IX_QMGR_Q_UNDERFLOW)
	{
	    tailIndex = index - 1;
#if IX_ETH_ACC_QMGR_NO_ADDITIONAL_CALLBACKS    
	    /*We've read all entries from the q*/
	    additionalReadsRequired = FALSE;
#endif
	}
	else if(rxQReadStatus == IX_SUCCESS)
	{
	    IX_ETH_ACC_STATS_INC(ixEthAccDataStats.rxFrameUnderrunFailure);
	    tailIndex = index;

#if IX_ETH_ACC_QMGR_NO_ADDITIONAL_CALLBACKS    
	    /*We've read all entries from the q*/
	    additionalReadsRequired = TRUE;
#endif
	}
	else
	{
	    /*major error*/
	    IX_ETH_ACC_FATAL_LOG(
		"IXETHACC:ixEthRxFrameQMCallback:Error: %d", 
		rxQReadStatus, 0, 0, 0, 0, 0);
	    return;
	}

	
	for(i=0;i< tailIndex ;i++)
	{
	    /* 
	     * Pick out mbuf from message.
	     */
	    mbufPtr = ixEthAccMbufFromRxQ(rxQEntry[i], 1);
# if IX_ETH_ACC_DATA_PLANE_FUNC_ARG_CHECKS
	    if (mbufPtr == (IX_MBUF *) NULL)
	    {
		IX_ETH_ACC_FATAL_LOG(
		    "IXETHACC:ixEthRxFrameQMCallback:Error: Null Mbuf Ptr", 
		    0, 0, 0, 0, 0, 0);
		return;
	    }
# endif
	    
	    /*
	     * Get Port from message.
	     */
	    
	    portId = (UINT32)(((UINT32) (IX_ETHNPE_QM_Q_RXENET_NPEID_MASK & rxQEntry[i]) >> 
			       IX_ETHNPE_QM_Q_FIELD_NPEID_R) & 0xFF);

# if IX_ETH_ACC_DATA_PLANE_FUNC_ARG_CHECKS

	    /* Prudent to at least check the port is within range */
	    if (portId >= IX_ETH_ACC_NUMBER_OF_PORTS)
	    {
		    IX_ETH_ACC_FATAL_LOG("IXETHACC:ixEthRxFrameQMCallback: port ID out of range: %d", 
                portId, 0, 0, 0, 0, 0);

		    return;
	    }

        /* NPEs have a limited port range, they can report only between 0 and 5,
         * therefore anything outside it has to be signalled as "not found" */
        if (portId > 5)
        {
            portId = IX_ETH_DB_NUMBER_OF_PORTS;
        }
#endif
	    /* 
	     * Call user level callback.
	     */
	    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccRxData.stats.rxFrameClientCallback);
	    ixEthAccPortData[portId].ixEthAccRxData.rxCallbackFn(
		ixEthAccPortData[portId].ixEthAccRxData.rxCallbackTag, mbufPtr, portId);
	} /*end for index*/
#if IX_ETH_ACC_QMGR_NO_ADDITIONAL_CALLBACKS    
    } while (additionalReadsRequired);
#endif
    return;
}
/**
 * @fn PRIVATE
 *
 * @brief
 *
 * @param
 *
 * @return none
 *
 * @internal
 */
void ixEthRxFreeQMCallback(IxQMgrQId qId, IxQMgrCallbackId callbackId
			   )
{	
    IxEthAccPortId	portId = (IxEthAccPortId) callbackId;
    int		        lockVal;
    UINT32		maxQWritesToPerform = IX_ETH_ACC_MAX_RX_FREE_BUFFERS_LOAD;
    IX_MBUF             *mbuf;
    IX_STATUS	        qStatus = IX_SUCCESS;

    /*
     * We have reached a low threshold on one of the Rx Free Qs
     */
    
    /*note that due to the fact that we are working off an Empty threshold, this callback
      need only write a single entry to the Rx Free queue in order to re-arm the notification
    */
    
    
    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeLowCallback);
    
    /* 
     * Get buffers from approprite S/W Rx freeBufferList Q.
     */
    
# if IX_ETH_ACC_DATA_PLANE_FUNC_ARG_CHECKS 
    if (!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
	IX_ETH_ACC_FATAL_LOG(
	    "IXETHACC:ixEthRxFreeQMCallback:Error: Invalid Port 0x%08X ", 
	    portId, 0, 0, 0, 0, 0);
	return;
    }
# endif
    IX_ETH_ACC_DATA_PLANE_LOCK(lockVal);
    if (IX_ETH_ACC_DATAPLANE_IS_Q_EMPTY(ixEthAccPortData[portId].
					ixEthAccRxData.
					freeBufferList))
    {
	/*  
	 * Turn off Q callback notification for Q in Question.
	 */
	qStatus = ixQMgrNotificationDisable(IX_ETH_ACC_PORT_TO_RX_FREE_Q_ID(portId));
	

	IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);

	/* No buffers avail to load into Rx free Q. */
	IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeLowCallbackUnderflow);

	if (qStatus != IX_SUCCESS)
	{
	    IX_ETH_ACC_FATAL_LOG(
		"IXETHACC:ixEthRxFreeQMCallback:Error: QM status 0x%08X ", 
		qStatus, 0, 0, 0, 0, 0);
	    return;
	}
    }
    else 
    {
	IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);
	/*
	 * Load the H/W Q with buffers from the s/w Q.
	 */
	
	do
	{
	    /* 
	     * Consume Q entries. - Note Q contains Physical addresss, 
	     * and have already been flushed to memory,
	     * And endianess converted if required.
	     */
	    IX_ETH_ACC_DATAPLANE_REMOVE_MBUF_FROM_Q_HEAD(
		ixEthAccPortData[portId].ixEthAccRxData.freeBufferList, 
		mbuf);
		if (mbuf != NULL)
		{
		    /*
		     * Add The Rx Buffer to the H/W Free buffer Q if possible  
		     */
		    qStatus = ixQMgrQWrite(IX_ETH_ACC_PORT_TO_RX_FREE_Q_ID(portId),
			&mbuf);

		    if (qStatus == IX_SUCCESS)
		    {
			IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeRepFromSwQ);
			/*
			 * Buffer added to h/w Q.
			 */
			continue;
		    }
		    else if (qStatus == IX_QMGR_Q_OVERFLOW)
		    {
			/*
			 * H/W Q overflow, need to save the buffer back on the s/w Q.
			 */
			IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeRepFromSwQOverflow);
			IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_HEAD(
			    ixEthAccPortData[portId].ixEthAccRxData.freeBufferList, 
			    mbuf)
			    break;
		    }
		    else 
		    {
			IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_HEAD(
			    ixEthAccPortData[portId].ixEthAccRxData.freeBufferList, 
			    mbuf)
			    IX_ETH_ACC_FATAL_LOG(
				"IXETHACC:ixEthRxFreeQMCallback:Error: QM status 0x%08X ", 
				qStatus, 0, 
				0, 0, 0, 0);
			break;
		    }
		}
		else 
		{
		    /*
		     * No more entries in s/w Q.
		     * Turn off Q callback indication
		     */

    	      	    IX_ETH_ACC_DATA_PLANE_LOCK(lockVal);
                    if (IX_ETH_ACC_DATAPLANE_IS_Q_EMPTY(ixEthAccPortData[portId].ixEthAccRxData.freeBufferList))
		    {
			    qStatus = ixQMgrNotificationDisable(
				IX_ETH_ACC_PORT_TO_RX_FREE_Q_ID(portId));
		    }
		    IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);

		}
	}
	while (--maxQWritesToPerform);
	
    }
}
/**
 * @fn PRIVATE
 *
 * @brief This function will discover the highest priority S/W Tx Q that
 *        has entries in it
 *
 * @param portId - (in) the id of the port whose S/W Tx queues are to be searched
 *        priorityPtr - (out) the priority of the highest priority occupied q will be written
 *                      here
 *
 * @return IX_ETH_ACC_SUCCESS if an occupied Q is found
 *         IX_ETH_ACC_FAIL if no Q has entries
 *
 * @internal
 */
PRIVATE INLINE IxEthAccStatus
ixEthAccTxSwQHighestPriorityGet(IxEthAccPortId portId, 
				IxEthAccTxPriority *priorityPtr)
{
    IxEthAccTxPriority priority;

    if (ixEthAccPortData[portId].ixEthAccTxData.schDiscipline 
	== FIFO_NO_PRIORITY)
    {
	if(IX_ETH_ACC_DATAPLANE_IS_Q_EMPTY(ixEthAccPortData[portId].
				       ixEthAccTxData.txQ[IX_ETH_ACC_TX_DEFAULT_PRIORITY]))
	{
	    return IX_ETH_ACC_FAIL;
	}
	else
	{
	    *priorityPtr = IX_ETH_ACC_TX_DEFAULT_PRIORITY;
	    return IX_ETH_ACC_SUCCESS;
	}
    }
    else
    {
	priority = IX_ETH_ACC_TX_PRIORITY_7;       
	while(priority >= IX_ETH_ACC_TX_PRIORITY_0 && priority <= IX_ETH_ACC_TX_PRIORITY_7)
	{
	    if(!IX_ETH_ACC_DATAPLANE_IS_Q_EMPTY(ixEthAccPortData[portId].
					       ixEthAccTxData.txQ[priority]))
	    {
		*priorityPtr = priority;
		return IX_ETH_ACC_SUCCESS;

	    }
	    priority--;
	}
	return IX_ETH_ACC_FAIL;
    }
}

/**
 * @fn PRIVATE
 *
 * @brief This function will take a buffer from a TX S/W Q and attempt
 *        to add it to the relevant TX H/W Q
 *
 * @param portId - the port whose TX queue is to be written to
 *        priority - identifies the queue from which the entry is to be read
 *
 * @return none
 *
 * @internal
 */
PRIVATE INLINE IxEthAccStatus
ixEthAccTxFromSwQ(IxEthAccPortId portId, 
		  IxEthAccTxPriority priority)
{
    IX_MBUF        *mbuf;
    IX_STATUS	   qStatus = IX_SUCCESS;
    UINT32	   qBuffer;

    IX_ETH_ACC_DATAPLANE_REMOVE_MBUF_FROM_Q_HEAD(
	ixEthAccPortData[portId].ixEthAccTxData.txQ[priority], 
	mbuf);
    
    if (mbuf != NULL)
    {
	/*
	 * Add the Tx buffer to the H/W Tx Q  
	 */

	/* 
	 * We do not need to flush here as it is already done
	 * in TxFrameSubmit()
	 */
	qBuffer = ixEthAccMbufQPrepare(mbuf, 0);
	qStatus = ixEthAccQmgrTxWrite(portId,qBuffer);
	
	if (qStatus == IX_SUCCESS)
	{
	    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.txQAddedOK);
	    IX_ETH_ACC_FPATH_DELTA_INC(txFramesCurrentlySubmitted[portId]);
	}
	else if (qStatus == IX_QMGR_Q_OVERFLOW)
	{
	    /*
	     * H/W Q overflow, need to save the buffer 
	     * back on the s/w Q.
	     * we must put it back on the head of the q to avoid 
	     * reordering packet tx
	     */
	    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.
				 txLowThreshCallbackHwOverflow);
	    IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_HEAD(
		ixEthAccPortData[portId].ixEthAccTxData.txQ[priority], 
		mbuf);
	    /*enable Q notification*/
	    ixQMgrNotificationEnable(
		IX_ETH_ACC_PORT_TO_TX_Q_ID(portId),
		IX_ETH_ACC_PORT_TO_TX_Q_SOURCE(portId));	 
	}
	else 
	{
	    IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_HEAD(ixEthAccPortData[portId].
						    ixEthAccRxData.
						    freeBufferList, mbuf);
	    
	    IX_ETH_ACC_FATAL_LOG(
		"IXETHACC:ixEthAccTxFromSwQ:Error: QM status 0x%08X ", 
		qStatus, 0, 
		0, 0, 0, 0);	
	}
    }
    else
    {
	IX_ETH_ACC_FATAL_LOG(
	    "IXETHACC:ixEthAccTxFromSwQ:Error, NULL Mbuf", 
	    0, 0, 0, 0, 0, 0);	
    }


    return qStatus;
}

/**
 * @fn PRIVATE
 *
 * @brief
 *
 * @param
 *
 * @return none
 *
 * @internal
 */
void ixEthTxFrameQMCallback(IxQMgrQId qId, IxQMgrCallbackId callbackId
			    )
{
    IxEthAccPortId portId = (IxEthAccPortId) callbackId;
    int		   lockVal;
    UINT32	   maxQWritesToPerform = IX_ETH_ACC_MAX_TX_FRAME_TX_CONSUME_PER_CALLBACK;
    IX_STATUS	   qStatus = IX_SUCCESS;
    IxEthAccTxPriority priority;

    
    /*
     * We have reached a low threshold on the Tx Q, and are being asked to 
     * supply a buffer for transmission from our S/W TX queues
     */
    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.txLowThreshCallback);
    /* 
     * Get buffers from approprite Q.
     */
    
# if IX_ETH_ACC_DATA_PLANE_FUNC_ARG_CHECKS 
    if (!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
	IX_ETH_ACC_FATAL_LOG(
	    "IXETHACC:ixEthTxFrameQMCallback:Error: Invalid Port 0x%08X ", 
	    portId, 0, 0, 0, 0, 0);
	return;
    }
# endif

   
    do
    {
	/* 
	 * Consume Q entries. - Note Q contains Physical addresss, 
	 * and have already been flushed to memory,
	 * And endianess converted if required.
	 */
	
	IX_ETH_ACC_DATA_PLANE_LOCK(lockVal);
	
	if(ixEthAccTxSwQHighestPriorityGet(portId, &priority) ==
	   IX_ETH_ACC_FAIL)
	{
	    /*
	     * No more entries in s/w Q.
	     * Turn off Q callback indication
	     */
	    qStatus = ixQMgrNotificationDisable(
		IX_ETH_ACC_PORT_TO_TX_Q_ID(portId));
	    IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);
	    /* No buffers available to load into Tx Q. */
	    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.
				 txLowThreshCallbackUnderflow);
	    
	    if (qStatus != IX_SUCCESS)
	    {
		IX_ETH_ACC_FATAL_LOG(
		    "IXETHACC:ixEthTxFrameQMCallback:Error: QM status 0x%08X ", 
		    qStatus, 0, 0, 0, 0, 0);
		return;
	    }
	    
	    return;	    
	    }
	else
	{
	    IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);
	    if(ixEthAccTxFromSwQ(portId,priority)==IX_SUCCESS)
	    {
		IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.
					 txLowThreshCallbackHwRepFromSwQ);
	    }
	    else
	    {
		return;
	    }
	}	
    }  	
    while (--maxQWritesToPerform);
}

/**
 * @fn PRIVATE
 *
 * @brief
 *
 * @param
 *
 * @return none
 *
 * @internal
 */

void ixEthTxFrameDoneQMCallback(IxQMgrQId qId, IxQMgrCallbackId
				callbackId)
{	
    UINT32     qEntry;
    IX_MBUF    *mbufPtr;
    UINT32     portId;
    BOOL       additionalReadsRequired = FALSE;
    UINT32     index;
    UINT32     tailIndex;
    UINT32     i;
    IxEthAccTxPriority priority;
    static UINT32 txDoneQEntry[IX_ETH_ACC_MAX_TX_FRAME_DONE_CONSUME_PER_CALLBACK];
    static UINT32 txDoneQReadStatus;

    /* 
     * Indication that Tx frames have been transmitted from the NPE.
     */
    
    IX_ETH_ACC_STATS_INC(ixEthAccDataStats.txFrameDoneCounter);
    
    do{
	index=0;
	
	/*First we read entries in Tx Done Q into an array*/
	do
	{
	    txDoneQReadStatus = ixQMgrQRead(qId, &qEntry);
	    txDoneQEntry[index]  = 
		IX_ETH_ACC_WORD_QM_WORD_ENDIAN_SWAP(qEntry);
	    index++;
	} while ( (txDoneQReadStatus == IX_SUCCESS) && 
	    (index < IX_ETH_ACC_MAX_TX_FRAME_DONE_CONSUME_PER_CALLBACK));


	if (txDoneQReadStatus == IX_QMGR_Q_UNDERFLOW)
	{
	    additionalReadsRequired = FALSE;	  
	    tailIndex = index - 1;
	}
	else if(txDoneQReadStatus == IX_SUCCESS)
	{
	    IX_ETH_ACC_STATS_INC(ixEthAccDataStats.txFrameDoneUnderrunFailure);
	    tailIndex = index;
	    additionalReadsRequired = TRUE;	  
	}
	else
	{
	    /*major error*/
	    IX_ETH_ACC_FATAL_LOG(
		"IXETHACC:ixEthTxFrameDoneQMCallback:Error: %d", 
		txDoneQReadStatus, 0, 0, 0, 0, 0);
	    return;
	}

	for(i=0;i<tailIndex;i++)
	{	    
	    /* 
	     * Pick out mbuf from message.
	     * Convert Ptr to Virtual. 
	     */
	    mbufPtr = ixEthAccMbufFromTxQ(txDoneQEntry[i], 0);
# if IX_ETH_ACC_DATA_PLANE_FUNC_ARG_CHECKS
	    if (mbufPtr == (IX_MBUF *) NULL)
	    {
		IX_ETH_ACC_FATAL_LOG(
		    "IXETHACC:ixEthTxFrameDoneQMCallback:Error: Null Mbuf Ptr", 
		    0, 0, 0, 0, 0, 0);
		return;
	    }
# endif
	    
	    /*
	     * Get NPE id from message, the convert to portId.
	     */
	    
	    portId = (UINT32)(((IX_ETHNPE_QM_Q_TXENETDONE_NPEID_MASK
				& txDoneQEntry[i]) >> 
			       IX_ETHNPE_QM_Q_FIELD_NPEID_R) & 0xFF);
	    portId = IX_ETH_ACC_QM_NPE_ID_TO_PORT_ID(portId);

# if IX_ETH_ACC_DATA_PLANE_FUNC_ARG_CHECKS
	    /* Prudent to at least check the port is within range */
	    if (portId >= IX_ETH_ACC_NUMBER_OF_PORTS)
	    {
		IX_ETH_ACC_FATAL_LOG(
		    "IXETHACC:ixEthTxFrameDoneQMCallback: Illegal port: %d", 
		    portId, 0, 0, 0, 0, 0);
		return;
	    }
#endif

	    IX_ETH_ACC_FPATH_DELTA_DEC(txFramesCurrentlySubmitted[portId]);

	    /* 
	     * Call user level callback.
	     */
	    IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.txDoneUserCallback);
	    ixEthAccPortData[portId].ixEthAccTxData.txBufferDoneCallbackFn(
		ixEthAccPortData[portId].ixEthAccTxData.txCallbackTag, 
		mbufPtr);

	    /* @todo Enabling this causes a performance drop of approximately 50%
	     */

#if IX_ETH_ACC_SUBMIT_FROM_TX_DONE	    
	    /*We should be able to pull a buffer off the S/W Tx Qs
	      and transmit it now*/
	    if(txFramesCurrentlySubmitted[portId] <
	       IX_ETH_ACC_MAX_TX_FRAMES_TO_SUBMIT)
	    {
		IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.txFromTxDoneAttempts);
		
		if(ixEthAccTxSwQHighestPriorityGet(portId, 
						   &priority)
		   !=IX_ETH_ACC_FAIL)
		{
		    if(ixEthAccTxFromSwQ(portId, priority) == IX_SUCCESS)
		    {
			IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.txFromTxDoneOK);		
			
		    }
		    
		}
	    }
#endif


	}
	
    } while( additionalReadsRequired );

    return;
}

void ixEthAccRecoverRxFreeQBuffers(IxEthAccPortId portId )
{

  IX_STATUS   qStatus = IX_SUCCESS;
  UINT32     qEntry;
  IX_MBUF    *buffer;

    /*
     * Recover Entries in Rx Replenesh Q.
     */
     /* printf("\nRecovering Rx Buffers from h/w Q\n"); */
     do
     {
        qStatus = ixQMgrQRead(IX_ETH_ACC_PORT_TO_RX_FREE_Q_ID(portId), &qEntry);
        if ( qStatus == IX_SUCCESS )
        {
	    buffer = ixEthAccMbufFromRxQ(IX_ETH_ACC_WORD_QM_WORD_ENDIAN_SWAP(qEntry), 0);
          IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccRxData.stats.rxFrameClientCallback);
          /* printf("\nReturning Rx buf:0x%08X\n",buffer); */
          ixEthAccPortData[portId].ixEthAccRxData.rxCallbackFn(
                ixEthAccPortData[portId].ixEthAccRxData.rxCallbackTag, 
                buffer,
                IX_ETH_DB_NUMBER_OF_PORTS /* port not found */);
        }
     }
     while ( qStatus == IX_SUCCESS);

}

void  
ixEthAccRecoverTxSubmittedQBuffers(IxEthAccPortId portId)
{
  IX_STATUS   qStatus = IX_SUCCESS;
  IX_STATUS   qWrStatus = IX_SUCCESS;
  UINT32     qEntry;
  IX_MBUF    *buffer;
  UINT32     maxBuffers = IX_QMGR_Q_SIZE128 + 1;

    /*
     * Recover Entries in Rx Replenesh Q.
     */

     /* printf("\nRecovering Tx Buffers from h/w Q\n"); */
	
     do
     {
        qStatus = ixQMgrQRead(IX_ETH_ACC_PORT_TO_TX_Q_ID(portId), &qEntry);
	/* printf("\nixQMgrQRead IX_ETH_ACC_PORT_TO_TX_Q_ID(%d),0x%08X",portId,qStatus); */
        if ( qStatus == IX_SUCCESS )
        {

#if IX_ETH_ACC_FPATH_AWARE 

	 /*
	  * Check if QEntry is a fast path descripor 
	  */	

	  if ( qEntry & IX_ETHNPE_QM_Q_TXENET_FPBIT_MASK )
	  {
		/*
	 	 * This is a fast path buffer, return to the same Q.
		 */
	 	qWrStatus = ixQMgrQWrite( IX_ETH_ACC_PORT_TO_TX_Q_ID(portId), &qEntry);	    
		if ( qWrStatus != IX_SUCCESS )
		{
	    	   IX_ETH_ACC_FATAL_LOG(
			"IXETHACC:ixEthAccRecoverTxSubmittedQBuffers:Error: qWrStatus = ", 
			qWrStatus, 0, 0, 0, 0, 0);
		}
	  }
	  else 
#endif
	  {
	      buffer = ixEthAccMbufFromTxQ(IX_ETH_ACC_WORD_QM_WORD_ENDIAN_SWAP(qEntry), 0);
	     IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.txDoneUserCallback);
	     IX_ETH_ACC_FPATH_DELTA_DEC(txFramesCurrentlySubmitted[portId]);

             /* printf("\nReturning Tx buf:0x%08X\n",buffer); */

	     ixEthAccPortData[portId].ixEthAccTxData.txBufferDoneCallbackFn(
		  ixEthAccPortData[portId].ixEthAccTxData.txCallbackTag, 
		  buffer);

	     --maxBuffers;
	  }

        }
     }
     while ( (qStatus == IX_SUCCESS) && (maxBuffers > 0));
}


void
ixEthAccDataPlaneShow()
{
#if IX_ETH_ACC_DATA_PLANE_STATS_ON
    UINT32 numEntries;
    UINT32 portId;
    UINT32 numBuffersFromClient=0;
    UINT32 numBuffersInHW=0;
    UINT32 numBuffersInTxSwQs=0;
    UINT32 numBuffersInRxSwQs=0;
    UINT32 totalBuffers=0;

    printf("Tx Done Callback invoked   : %u\n", 
	   ixEthAccDataStats.txFrameDoneCounter);
    printf("Rx Callback invoked        : %u\n", 
	   ixEthAccDataStats.rxFrameCounter);

    printf("Tx Done Underflow Failures : %u\n", 
	   ixEthAccDataStats.txFrameDoneUnderrunFailure);
    printf("Rx Underflow Failures      : %u\n", 
	   ixEthAccDataStats.rxFrameUnderrunFailure);
    

    for(portId=IX_ETH_PORT_1; portId<=IX_ETH_PORT_2; portId++)
    {
	printf("PORT %d\n",portId);
	printf("Tx Done Additional Callback: %u\n", 
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txDoneAdditionalCallback);
	printf("Tx Done User Callback      : %u\n", 
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txDoneUserCallback);
	printf("Tx frames                  : %u\n", 
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txFrameCounter);
	printf("Tx Q Overflow              : %u\n", 
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txQOverflow);
	printf("Tx added to S/w Q          : %u\n",
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txAddedToSWQ);
	printf("Tx H/W Q Added OK          : %u\n", 
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txQAddedOK);
	printf("Tx Low Thresh CB           : %u\n", 
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txLowThreshCallback);
	printf("Tx Low Thresh CB Underflow : %u\n", 
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txLowThreshCallbackUnderflow);
	printf("Tx Low HW repl from SW     : %u\n", 
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txLowThreshCallbackHwRepFromSwQ);
	printf("Tx Submit HW repl from SW  : %u\n", 
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txSubmitHwRepFromSwQ);
	printf("Tx Submit HW repl failures : %u\n", 
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txSubmitHwRepFromSwQFailures);

	printf("Tx Low Thresh CB HW oflow  : %u\n", 
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txLowThreshCallbackHwOverflow);
	printf("Tx From Tx Done Attempts   : %u\n", 
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txFromTxDoneAttempts);
	printf("Tx From Tx Done Succeeded  : %u\n", 
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txFromTxDoneOK);
	
	
	
	printf("Rx Additional callback     : %u\n", 
	       ixEthAccPortData[portId].ixEthAccRxData.stats.rxCallbackRequest);
	printf("Rx Frame Client Callback   : %u\n", 
	       ixEthAccPortData[portId].ixEthAccRxData.stats.rxFrameClientCallback);
	printf("Rx Free Replenish          : %u\n", 
	       ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeReplenish);
	printf("Rx Free Replenish OK       : %u\n", 
	       ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeReplenishOK);
	printf("Rx Free Replenish oflow    : %u\n", 
	       ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeReplenishOverflow);
	printf("Rx Free Low Callback       : %u\n",
	       ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeLowCallback);
	printf("Rx Free Low Callback uflow : %u\n", 
	       ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeLowCallbackUnderflow);
	
	printf("Rx Free Replenish from S/w : %u\n", 
	       ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeRepFromSwQ);
	printf("Rx Free Repl from S/w oflow: %u\n", 
	       ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeRepFromSwQOverflow);	
	
	printf("# Frames currently submitted for transmission    : %u\n", 
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txFrameCounter - 
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txDoneUserCallback);

	numBuffersInHW += ixEthAccPortData[portId].ixEthAccTxData.stats.txQAddedOK -  
	    ixEthAccPortData[portId].ixEthAccTxData.stats.txDoneUserCallback;
	
	printf("# Rx buffers given by client                     : %u\n",
	       ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeReplenish - 
	       ixEthAccPortData[portId].ixEthAccRxData.stats.rxFrameClientCallback);

	numBuffersInHW += ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeReplenish - 
	    ixEthAccPortData[portId].ixEthAccRxData.stats.rxFrameClientCallback;

        numBuffersFromClient += ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeReplenish - 
	    ixEthAccPortData[portId].ixEthAccRxData.stats.rxFrameClientCallback;
	
	printf("# Frames in ethAcc Tx S/w queues                 : %u\n",
	       ixEthAccPortData[portId].ixEthAccTxData.stats.txAddedToSWQ
	       - ixEthAccPortData[portId].ixEthAccTxData.stats.txLowThreshCallbackHwRepFromSwQ
	       - ixEthAccPortData[portId].ixEthAccTxData.stats.txSubmitHwRepFromSwQ
	       - ixEthAccPortData[portId].ixEthAccTxData.stats.txFromTxDoneOK);

	numBuffersInTxSwQs += ixEthAccPortData[portId].ixEthAccTxData.stats.txAddedToSWQ
	    - ixEthAccPortData[portId].ixEthAccTxData.stats.txLowThreshCallbackHwRepFromSwQ
	    - ixEthAccPortData[portId].ixEthAccTxData.stats.txSubmitHwRepFromSwQ
	    - ixEthAccPortData[portId].ixEthAccTxData.stats.txFromTxDoneOK;

	printf("# Frames in ethAcc Rx S/w queues                 : %u\n",
	       ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeReplenishOverflow 
	       - ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeRepFromSwQ);
	
	numBuffersInRxSwQs += ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeReplenishOverflow 
	    - ixEthAccPortData[portId].ixEthAccRxData.stats.rxFreeRepFromSwQ;
	
	       
    }
    

    printf("Hardware queues:\n");   

    ixQMgrQNumEntriesGet(IX_ETH_ACC_RX_FRAME_ETH_Q, &numEntries);
    printf("Receive Q              : %u \n",numEntries);
    
    ixQMgrQNumEntriesGet(IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q, &numEntries);
    printf("Receive Free Port 1 Q  : %u \n",numEntries);

    ixQMgrQNumEntriesGet(IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q, &numEntries);
    printf("Receive Free Port 2 Q  : %u \n",numEntries);

    ixQMgrQNumEntriesGet(IX_ETH_ACC_TX_FRAME_ENET0_Q, &numEntries);
    printf("Transmit Port 1 Q      : %u \n",numEntries);

    ixQMgrQNumEntriesGet(IX_ETH_ACC_TX_FRAME_ENET1_Q, &numEntries);
    printf("Transmit Port 2 Q      : %u \n",numEntries);

    ixQMgrQNumEntriesGet( IX_ETH_ACC_TX_FRAME_DONE_ETH_Q, &numEntries);
    printf("Transmit Done Q        : %u \n",numEntries);


    totalBuffers = numBuffersInHW + numBuffersInTxSwQs 
	+ numBuffersInRxSwQs;
    printf("Total Buffers accounted for: %d\n", (int)totalBuffers);
    printf("      Buffers in HW Qs     : %d\n", (int)numBuffersInHW);
    printf("      Buffers in SW Tx Qs  : %d\n", (int)numBuffersInTxSwQs);
    printf("      Buffers in SW Rx Qs  : %d\n", (int)numBuffersInRxSwQs);

    printf("Buffers given by   Client  : %d\n", (int)numBuffersFromClient);
#else
    printf("Statistics collection not supported in this load\n");
#endif

#if IX_ETH_ACC_FPATH_AWARE
    printf("Currently Running In FAST PATH AWARE MODE\n");
    printf("Tx Frames Currently Submitted port 0 : %u\n",
	   txFramesCurrentlySubmitted[0]);
    printf("Tx Frames Currently Submitted port 1 : %u\n",
	   txFramesCurrentlySubmitted[1]);
#else
    printf("Currently Running In NON - FAST PATH AWARE MODE\n");
#endif
}


