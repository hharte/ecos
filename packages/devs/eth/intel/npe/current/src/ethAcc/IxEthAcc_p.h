/**
 * @file IxEthAcc_p.h
 *
 * @author Intel Corporation
 * @date 12-Feb-2002
 *
 * @brief  Internal Header file for IXP425 Ethernet Access component.
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

/**
 * @addtogroup IxEthAccPri
 *@{
 */

#ifndef IxEthAcc_p_H
#define IxEthAcc_p_H

/*
 * Os/System dependancies.
 */
#include <IxTypes.h>
#include <IxOsServices.h>
#include <IxOsCacheMMU.h>

/*
 * Intermodule dependancies
 */
#include <IxNpeDl.h>

#include <IxEthNpe.h>

/* 
 * Intra module dependancies
 */

#include <IxEthAccDataPlane_p.h>


#define INLINE __inline__

#ifndef DEBUG

#define IX_ETH_ACC_PRIVATE static

#else

#define IX_ETH_ACC_PRIVATE

#endif /* ndef DEBUG */

#define IX_ETH_ACC_PUBLIC


/**
 * @brief port ID => message handler NPE id conversion (0 => NPE_B, 1 => NPE_C)
 */
#define IX_ETH_ACC_PORT_ID_TO_NPE(id) (id + 1)

/**
 * @brief message handler NPE id => port ID conversion (NPE_B => 0, NPE_C => 1)
 */
#define IX_ETH_ACC_NPE_TO_PORT_ID(npe) (npe - 1)



#define IX_ETH_ACC_IS_PORT_VALID(port) (port <  IX_ETH_ACC_NUMBER_OF_PORTS  ? TRUE : FALSE )



#ifdef __ECOS
#define IX_ETH_ACC_DEBUG 0
#else
#define IX_ETH_ACC_DEBUG (1)
#endif

#if IX_ETH_ACC_DEBUG
#if 1 
#define IX_ETH_ACC_FATAL_LOG(a,b,c,d,e,f,g) { ixOsServLog ( LOG_FATAL,a,b,c,d,e,f,g);}
#define IX_ETH_ACC_DEBUG_LOG(a,b,c,d,e,f,g) { ixOsServLog ( LOG_FATAL,a,b,c,d,e,f,g);}
#endif
#if 0
#define IX_ETH_ACC_FATAL_LOG(a,b,c,d,e,f,g) { printf("\nFATAL LOG : "); printf ( a,b,c,d,e,f,g);}
#define IX_ETH_ACC_DEBUG_LOG(a,b,c,d,e,f,g) { printf("\nDEBUG LOG : "); printf ( a,b,c,d,e,f,g);}
#endif

#else
#define IX_ETH_ACC_FATAL_LOG(a,b,c,d,e,f,g) { ixOsServLog ( LOG_FATAL,a,b,c,d,e,f,g);}
#define IX_ETH_ACC_DEBUG_LOG(a,b,c,d,e,f,g) {}
#endif

IX_ETH_ACC_PUBLIC IxEthAccStatus ixEthAccInitCommon(void);
IX_ETH_ACC_PUBLIC IxEthAccStatus ixEthAccInitDataPlane(void);
IX_ETH_ACC_PUBLIC IxEthAccStatus ixEthAccRegisterQMgrCallbacks(void);



/**
 * @struct  ixEthAccCapabilites
 * @brief   Data structure with the capabilites of the specific NPE.
 *
 */
typedef struct  
{
  IxNpeDlVersionId versionId; /**< Version code for the NPE which was downloaded */
} ixEthAccCapabilites; 

/**
 * @struct  ixEthAccRxDataStats
 * @brief   Stats data structures for data path. - Not obtained from h/w
 *
 */
typedef struct
{ 
    UINT32 rxCallbackRequest;
    UINT32 rxFreeReplenish;
    UINT32 rxFreeReplenishOK;
    UINT32 rxFreeReplenishOverflow;
    UINT32 rxFrameClientCallback;
    UINT32 rxFreeLowCallback;
    UINT32 rxFreeLowCallbackUnderflow;
    UINT32 rxFreeRepFromSwQ;
    UINT32 rxFreeRepFromSwQOverflow;
} ixEthAccRxDataStats;

/**
 * @struct  ixEthAccTxDataStats
 * @brief   Stats data structures for data path. - Not obtained from h/w
 *
 */
typedef struct
{    
    UINT32 txDoneAdditionalCallback;   
    UINT32 txFrameCounter;
    UINT32 txQOverflow;
    UINT32 txAddedToSWQ;
    UINT32 txQAddedOK;
    UINT32 txLowThreshCallback;
    UINT32 txLowThreshCallbackUnderflow;
    UINT32 txLowThreshCallbackHwRepFromSwQ;
    UINT32 txLowThreshCallbackHwOverflow;
    UINT32 txSubmitHwRepFromSwQ;
    UINT32 txDoneUserCallback;
    UINT32 txFromTxDoneAttempts;
    UINT32 txFromTxDoneOK;
    UINT32 txSubmitHwRepFromSwQFailures;
} ixEthAccTxDataStats;


/**
 * @struct  ixEthAccRxDataInfo
 * @brief   Per Port data structures associated with the receive data plane.
 *
 */
typedef struct
{
  IxEthAccPortRxCallback rxCallbackFn;
  UINT32  rxCallbackTag;
  IxEthAccDataPlaneQList freeBufferList;

  ixEthAccRxDataStats stats; /**< Receive s/w stats */
} ixEthAccRxDataInfo; 

/**
 * @struct  ixEthAccTxDataInfo
 * @brief   Per Port data structures associated with the transmit data plane.
 *
 */
typedef struct
{
  IxEthAccPortTxDoneCallback  txBufferDoneCallbackFn;
  UINT32  txCallbackTag;
  IxEthAccDataPlaneQList 	txQ[IX_ETH_ACC_NUM_TX_PRIORITIES]; /**< Transmit Q */
  IxEthAccTxSchedulerDiscipline schDiscipline;
  ixEthAccTxDataStats stats; /**< Transmit s/w stats */
} ixEthAccTxDataInfo; 


/**
 * @struct  ixEthAccPortDataInfo
 * @brief   Per Port data structures associated with the port data plane.
 *
 */
typedef struct
{
   BOOL               portInitialized;
   ixEthAccTxDataInfo ixEthAccTxData; /**< Transmit data control structures */
   ixEthAccRxDataInfo ixEthAccRxData; /**< Recieve data control structures */
} ixEthAccPortDataInfo; 

extern ixEthAccPortDataInfo  ixEthAccPortData[];
#define IX_ETH_IS_PORT_INITIALIZED(port) (ixEthAccPortData[port].portInitialized)

extern BOOL ixEthAccServiceInit;
#define IX_ETH_ACC_IS_SERVICE_INITIALIZED() (ixEthAccServiceInit == TRUE )

/* 
 * Maximum number of frames to consume from the Rx Frame Q.
 */

#define IX_ETH_ACC_MAX_RX_FRAME_CONSUME_PER_CALLBACK (128)

/*
 * Max number of times to load the Rx Free Q from callback.  
 */
#define IX_ETH_ACC_MAX_RX_FREE_BUFFERS_LOAD (256)  /* Set greater than depth of h/w Q + drain time at line rate */

/*
 *  Max number of times to read from the Tx Done Q in one sitting.
 */

#define IX_ETH_ACC_MAX_TX_FRAME_DONE_CONSUME_PER_CALLBACK (256)

/*
 *  Max number of times to take buffers from S/w queues and write them to the H/w Tx
 *  queues on receipt of a Tx low threshold callback 
 */

#define IX_ETH_ACC_MAX_TX_FRAME_TX_CONSUME_PER_CALLBACK (16)


#define IX_ETH_ACC_FLUSH_CACHE(addr,size)  IX_ACC_DATA_CACHE_FLUSH(addr,size)
#define IX_ETH_ACC_INVALIDATE_CACHE(addr,size)  IX_ACC_DATA_CACHE_INVALIDATE(addr,size)


#define IX_ETH_ACC_MEMSET(start,value,size) memset(start,value,size)

extern    void  ixEthAccRecoverRxFreeQBuffers(IxEthAccPortId portId);
extern    void  ixEthAccRecoverTxSubmittedQBuffers(IxEthAccPortId portId);


#endif /* ndef IxEthAcc_p_H */



