/**
 * @file IxOsBuffPoolMgt.c
 *
 * @date 9 Oct 2002
 *
 * @brief This file contains the mbuf pool management implementation.
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
 *
 * @sa IxOsBuffPoolMgt.h
 * @sa IxOsBuffMgt.h
 */

/*
 * Put the system defined include files required.
 */

#ifdef __vxworks

#include <vxWorks.h>
#include <net/mbuf.h>
#include <netBufLib.h>

#endif


/*
 * Put the user defined include files required.
 */

#include "IxTypes.h"
#include "IxAssert.h"
#include "IxOsCacheMMU.h"
#include "IxOsBuffPoolMgt.h"
#include "IxOsServices.h"


/*
 * #defines and macros used in this file.
 */


/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

static IX_MBUF_POOL ixOsBuffPools[IX_MBUF_MAX_POOLS];
static int ixOsBuffPoolsInUse = 0;


/*
 * Extern function prototypes.
 */

#ifdef __vxworks
void netPoolShow(NET_POOL_ID pNetPool);
#endif



/*
 * Function definition: ixOsBuffPoolInit
 */

IX_STATUS
ixOsBuffPoolInit (IX_MBUF_POOL **poolPtrPtr, int count, int size, char *name)
{
    void *poolBufPtr;
    void *poolDataPtr;
    int mbufMemSize;
    int dataMemSize;

    poolBufPtr = IX_MBUF_POOL_MBUF_AREA_ALLOC(count, mbufMemSize);
    poolDataPtr = IX_MBUF_POOL_DATA_AREA_ALLOC(count, size, dataMemSize);

    IX_ASSERT(poolBufPtr != NULL);
    IX_ASSERT(poolDataPtr != NULL);

    return ixOsBuffPoolInitNoAlloc (poolPtrPtr, poolBufPtr, poolDataPtr, count, size, name);
}


/*
 * Function definition: ixOsBuffPoolInitNoAlloc
 */

IX_STATUS
ixOsBuffPoolInitNoAlloc (IX_MBUF_POOL **poolPtrPtr, void *poolBufPtr, void *poolDataPtr, int count, int size, char *name)
{
#ifndef __vxworks
    int i, mbufSizeAligned, sizeAligned;
    IX_MBUF *currentMbufPtr = NULL;
    IX_MBUF *nextMbufPtr = NULL;
#else
    M_CL_CONFIG ixMbufClBlkCfg;
    CL_DESC ixMbufClDesc;
#endif

    /* check parameters */
    if ((poolPtrPtr == NULL)  ||
#ifdef __vxworks
        (poolDataPtr == NULL) ||
#endif
        (poolBufPtr == NULL)  ||
        (count <= 0)          ||
        (strlen(name) > IX_MBUF_POOL_NAME_LEN) ||
        /* checks for alignment of memory on cache-line boundaries */
        (((unsigned)poolBufPtr % IX_XSCALE_CACHE_LINE_SIZE) != 0) || 
        (((unsigned)poolDataPtr % IX_XSCALE_CACHE_LINE_SIZE) != 0))
    {
#ifndef __ECOS
        ixOsServLog (LOG_ERROR, "ixOsBuffPoolInitNoAlloc(): "
        "ERROR - Invalid Parameter\n", 0, 0, 0, 0, 0, 0);
#endif
        IX_ASSERT(0);
    }

    if (ixOsBuffPoolsInUse == IX_MBUF_MAX_POOLS)
    {
        /* All pools are in use */ 
        return IX_FAIL;
    }

    *poolPtrPtr = &ixOsBuffPools[ixOsBuffPoolsInUse++];

#ifndef __vxworks
    /* Adjust sizes to ensure alignment on cache line boundaries */
    mbufSizeAligned = IX_MBUF_POOL_SIZE_ALIGN(sizeof(IX_MBUF));
    /* clear the mbuf memory area */
    memset (poolBufPtr, 0, mbufSizeAligned * count);
    
    if (poolDataPtr != NULL)
    {
        /* Adjust sizes to ensure alignment on cache line boundaries */
        sizeAligned = IX_MBUF_POOL_SIZE_ALIGN(size);
        /* clear the data memory area */
	memset (poolDataPtr, 0, sizeAligned * count);
    }
    else
    {
        sizeAligned = 0;
    }

    strcpy ((*poolPtrPtr)->name, name);

    (*poolPtrPtr)->mbufMemPtr = poolBufPtr;
    (*poolPtrPtr)->dataMemPtr = poolDataPtr;
    (*poolPtrPtr)->bufDataSize = sizeAligned;
    (*poolPtrPtr)->totalBufsInPool = count;
    (*poolPtrPtr)->mbufMemSize = mbufSizeAligned * count;
    (*poolPtrPtr)->dataMemSize = sizeAligned * count;
    
    currentMbufPtr = (IX_MBUF *)poolBufPtr;
    (*poolPtrPtr)->nextFreeBuf = currentMbufPtr;

    for (i = 0; i < count; i++)
    {
        if (i < (count - 1))
        {
            nextMbufPtr = (IX_MBUF *)((unsigned)currentMbufPtr + mbufSizeAligned);
        }
        else
        {   /* last mbuf in chain */
            nextMbufPtr = NULL;
        }
        IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(currentMbufPtr) = nextMbufPtr;
        IX_MBUF_NET_POOL(currentMbufPtr) = *poolPtrPtr;

        if (poolDataPtr != NULL)
        {
            IX_MBUF_MDATA(currentMbufPtr) = poolDataPtr;
            IX_MBUF_MLEN(currentMbufPtr) = size;
            poolDataPtr = (void *)((unsigned)poolDataPtr + sizeAligned);
        }

        currentMbufPtr = nextMbufPtr;
    }

    /* update the number of free buffers in the pool */
    (*poolPtrPtr)->freeBufsInPool = count;

#else  /* ndef __vxworks */

    ixMbufClBlkCfg.mBlkNum  = count;
    ixMbufClBlkCfg.clBlkNum = count;
    ixMbufClBlkCfg.memSize = IX_MBUF_POOL_MBUF_AREA_SIZE_ALIGNED(count);
    ixMbufClBlkCfg.memArea = poolBufPtr;

    ixMbufClDesc.clSize = size;
    ixMbufClDesc.clNum  = count;
    ixMbufClDesc.memSize = IX_MBUF_POOL_DATA_AREA_SIZE_ALIGNED(count, size);
    ixMbufClDesc.memArea = poolDataPtr;

    if (netPoolInit (*poolPtrPtr, &ixMbufClBlkCfg, &ixMbufClDesc, 1, NULL) != OK)
    {
        /* pool was not initialised successfully */
        return IX_FAIL;
    }
#endif  /* __vxworks */

    return IX_SUCCESS;
}


/*
 * Function definition: ixOsBuffPoolUnchainedBufGet
 */

IX_STATUS
ixOsBuffPoolUnchainedBufGet (IX_MBUF_POOL *poolPtr, IX_MBUF **newBufPtrPtr)
{
#ifndef __vxworks
    int lock;
#else
    int bufSize;
#endif

    IX_STATUS status = IX_SUCCESS;

    /* check parameters */
    if ((poolPtr == NULL) || (newBufPtrPtr == NULL))
    {
#ifndef __ECOS
        ixOsServLog (LOG_ERROR, "ixOsBuffPoolUnchainedBufGet(): "
		     "ERROR - Invalid Parameter\n", 0, 0, 0, 0, 0, 0);
#endif
        return IX_FAIL;
    }

#ifndef __vxworks
    lock = ixOsServIntLock();
    
    *newBufPtrPtr = poolPtr->nextFreeBuf;
    if (*newBufPtrPtr)
    {
        poolPtr->nextFreeBuf = IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(*newBufPtrPtr);
        IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(*newBufPtrPtr) = NULL;

        /* update the number of free buffers in the pool */
        poolPtr->freeBufsInPool--;
    }
    else
    {
        /* Could not get a free buffer */ 
        status = IX_FAIL;
    }
  
    ixOsServIntUnlock(lock);

#else /* ndef __vxworks */

    bufSize = poolPtr->pClBlkHead->clSize;

    *newBufPtrPtr = netTupleGet(poolPtr, bufSize, M_DONTWAIT, MT_DATA, FALSE);
    if (*newBufPtrPtr == NULL)
    {
        status = IX_FAIL;
    }
#endif /* ndef __vxworks */

    return status;
}


/*
 * Function definition: ixOsBuffPoolBufFree
 */

IX_MBUF *
ixOsBuffPoolBufFree (IX_MBUF *bufPtr)
{
#ifndef __vxworks
    int lock;
    IX_MBUF_POOL *poolPtr;
    IX_MBUF *nextBufPtr;
#else
#endif

    /* check parameters */
    if (bufPtr == NULL)
    {
#ifndef __ECOS
        ixOsServLog (LOG_ERROR, "ixOsBuffPoolChainedBufFree(): "
                    "ERROR - Invalid Parameter\n", 0, 0, 0, 0, 0, 0);
#endif
        return NULL;
    }

#ifndef __vxworks
    poolPtr = IX_MBUF_NET_POOL(bufPtr);

    lock = ixOsServIntLock();

    nextBufPtr = IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(bufPtr);
    
    IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(bufPtr) = poolPtr->nextFreeBuf;
    poolPtr->nextFreeBuf = bufPtr;
    
    /* update the number of free buffers in the pool */
    poolPtr->freeBufsInPool++;

    ixOsServIntUnlock(lock);

    return nextBufPtr;

#else  /* ndef __vxworks */

    return netMblkClFree(bufPtr);
#endif /* ndef __vxworks */
}
    

/*
 * Function definition: ixOsBuffPoolBufChainFree
 */

void
ixOsBuffPoolBufChainFree (IX_MBUF *bufPtr)
{
#ifndef __vxworks
    while((bufPtr = ixOsBuffPoolBufFree(bufPtr)));
#else  /* ndef __vxworks */
    netMblkClChainFree(bufPtr);
#endif /* ndef __vxworks */
}


/*
 * Function definition: ixOsBuffPoolShow
 */

IX_STATUS
ixOsBuffPoolShow (IX_MBUF_POOL *poolPtr)
{
#ifndef __vxworks
    IX_MBUF *nextBufPtr;
    int count = 0;
    int lock;
#else
#endif

    /* check parameters */
    if (poolPtr == NULL)
    {
#ifndef __ECOS
        ixOsServLog (LOG_ERROR, "ixOsBuffPoolShow(): "
                    "ERROR - Invalid Parameter", 0, 0, 0, 0, 0, 0);
#endif
        return IX_FAIL;
    }

#ifndef __vxworks
    lock = ixOsServIntLock();
    
    nextBufPtr = poolPtr->nextFreeBuf;
    while(nextBufPtr)
    {
        count++;
        nextBufPtr = IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(nextBufPtr);
    }

    ixOsServIntUnlock(lock);

#ifndef __ECOS
    ixOsServLog(LOG_USER,
        "Pool %s (%p) -> %5d buf available\n",
        (UINT32)poolPtr->name, 
        (UINT32) poolPtr, 
        count, 
        0, 0, 0);
    ixOsServLog(LOG_USER,
        " Mbuf start address is %p, Data start address is %p\n",
        (UINT32)poolPtr->mbufMemPtr, 
        (UINT32)poolPtr->dataMemPtr, 
        0,0,0,0);
#endif
#else  /* ndef __vxworks */
    netPoolShow (poolPtr);

#endif /* ndef __vxworks */

    return IX_SUCCESS;
}


/*
 * Function definition: ixOsBuffPoolBufDataPtrReset
 */

IX_STATUS
ixOsBuffPoolBufDataPtrReset (IX_MBUF *bufPtr)
{
#ifndef __vxworks
    IX_MBUF_POOL *poolPtr = IX_MBUF_NET_POOL(bufPtr);
    UINT8 *poolDataPtr = poolPtr->dataMemPtr;
#else
#endif

    /* check parameters */
    if (bufPtr == NULL)
    {
#ifndef __ECOS
        ixOsServLog (LOG_ERROR, "ixOsBuffPoolBufDataPtrReset(): "
                     "ERROR - Invalid Parameter\n", 0, 0, 0, 0, 0, 0);
#endif
        return IX_FAIL;
    }

#ifndef __vxworks
    if (poolDataPtr)
    {
        unsigned int bufSize = poolPtr->bufDataSize;
        unsigned int bufDataAddr = (unsigned int)IX_MBUF_MDATA(bufPtr);
        unsigned int poolDataAddr = (unsigned int)poolDataPtr;
	
        /*
         * the pointer is still pointing somewhere in the mbuf payload.
         * This operation moves the pointer to the beginning of the 
         * mbuf payload
         */
        bufDataAddr = ((bufDataAddr - poolDataAddr) / bufSize) * bufSize; 
        IX_MBUF_MDATA(bufPtr) = &poolDataPtr[bufDataAddr];
    }
#ifndef __ECOS
    else
    {
        ixOsServLog(LOG_WARNING, __FUNCTION__
                    ": cannot be used if user supplied NULL pointer for pool data area "
                    "when pool was created\n", 0, 0, 0, 0, 0, 0);
    }
#endif

#else  /* ndef __vxworks */

    IX_MBUF_MDATA(bufPtr) = (caddr_t) (bufPtr->pClBlk->clNode.pClBuf);
#endif /* ndef __vxworks */

    return IX_SUCCESS;
}


/*
 * Function definition: ixOsBuffPoolUninit
 */

IX_STATUS 
ixOsBuffPoolUninit(IX_MBUF_POOL *pool)
{
#ifndef __vxworks
  /*
   * TODO - implement this for non-VxWorks OS's (using IX_ACC_DRV_DMA_FREE ? )
   * Then add the function to the API
   */
#ifndef __ECOS
    ixOsServLog(LOG_WARNING, __FUNCTION__
                ": is currently only implemented for the vxWorks platform\n",
                0, 0, 0, 0, 0, 0);
#endif
    return IX_FAIL;
#else
    return ((netPoolDelete(pool) == OK) ? IX_SUCCESS : IX_FAIL);
#endif
}

