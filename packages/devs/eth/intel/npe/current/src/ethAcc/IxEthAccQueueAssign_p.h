/**
 * @file IxEthAccQueueAssign_p.h
 *
 * @author Intel Corporation
 * @date 06-Mar-2002
 *
 * @brief   Mapping from QMgr Q's to internal assignment
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

/*
 * Os/System dependancies.
 */
#include <IxTypes.h>
#include <IxOsServices.h>
#include <IxOsCacheMMU.h>

/*
 * Intermodule dependancies
 */
#include <IxQMgr.h>


/**
*
* @def IX_ETH_ACC_RX_FRAME_ETH_Q 
*
* @brief  Eth0/Eth1 NPE Frame Recieve Q.
*
*/
#define IX_ETH_ACC_RX_FRAME_ETH_Q 	(IX_QMGR_QUEUE_23)

/**
*
* @def IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q
*
* @brief Supply Rx Buffers Ethernet Q for NPE - Eth 0 - Port 1
*
*/
#define IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q    (IX_QMGR_QUEUE_24)

/**
*
* @def IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q
*
* @brief Supply Rx Buffers Ethernet Q for NPE - Eth 1 - Port 2
*
*/
#define IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q    (IX_QMGR_QUEUE_25)


/**
*
* @def IX_ETH_ACC_TX_FRAME_ENET0_Q
*
* @brief Submit frame Q for NPE Eth 0 - Port 1
*
*/
#define IX_ETH_ACC_TX_FRAME_ENET0_Q    (IX_QMGR_QUEUE_26)


/**
*
* @def IX_ETH_ACC_TX_FRAME_ENET1_Q
*
* @brief Submit frame Q for NPE Eth 1 - Port 2
*
*/
#define IX_ETH_ACC_TX_FRAME_ENET1_Q    (IX_QMGR_QUEUE_27)

/**
*
* @def IX_ETH_ACC_TX_FRAME_DONE_ETH_Q
*
* @brief Transmit complete Q for NPE Eth 0/1, Port 1&2
*
*/
#define IX_ETH_ACC_TX_FRAME_DONE_ETH_Q    (IX_QMGR_QUEUE_28)

/* Check range of Q's assigned to this component. */

#if IX_ETH_ACC_RX_FRAME_ETH_Q >= (IX_QMGR_MIN_QUEUPP_QID ) |    	\
 IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q  >=  (IX_QMGR_MIN_QUEUPP_QID) | 	\
 IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q  >=  (IX_QMGR_MIN_QUEUPP_QID) | 	\
 IX_ETH_ACC_TX_FRAME_ENET0_Q >=   (IX_QMGR_MIN_QUEUPP_QID) |  		\
 IX_ETH_ACC_TX_FRAME_ENET1_Q >=   (IX_QMGR_MIN_QUEUPP_QID) | 		\
 IX_ETH_ACC_TX_FRAME_DONE_ETH_Q  >=  (IX_QMGR_MIN_QUEUPP_QID)  
#error "Not all Ethernet Access Queues are betweem 1-31, requires full functionalty Q's unless otherwise validated "
#endif




/**
*
* @typedef  ixEthAccQregInfo
*
* @brief 
*
*/
typedef struct 
{
   IxQMgrQId qId;
   char *qName;
   IxQMgrCallback qCallback;
   IxQMgrCallbackId callbackTag;
   IxQMgrQSizeInWords qSize;
   IxQMgrQEntrySizeInWords qWords; 
   BOOL           qNotificationEnableAtStartup;
   IxQMgrSourceId qConditionSource; 
   IxQMgrWMLevel  AlmostEmptyThreshold;
   IxQMgrWMLevel  AlmostFullThreshold;

} ixEthAccQregInfo;


/*
 * Prototypes for all QM callbacks.
 */

/* 
 * Rx Callbacks 
 */
IX_ETH_ACC_PUBLIC
void  ixEthRxFrameQMCallback(IxQMgrQId, IxQMgrCallbackId);

IX_ETH_ACC_PUBLIC
void  ixEthRxFreeQMCallback(IxQMgrQId, IxQMgrCallbackId);

/* 
 * Tx Callback.
 */
IX_ETH_ACC_PUBLIC
void  ixEthTxFrameQMCallback(IxQMgrQId, IxQMgrCallbackId);

IX_ETH_ACC_PUBLIC
void  ixEthTxFrameDoneQMCallback(IxQMgrQId, IxQMgrCallbackId );


#define IX_ETH_ACC_QM_QUEUE_DISPATCH_PRIORITY (IX_QMGR_Q_PRIORITY_0) /* Highest priority */

/*
 * @todo; Validated all these Q settings.
 */

#define IX_ETH_ACC_RX_FRAME_ETH_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_NOT_E   )
#define IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   )   
#define IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   )
#define IX_ETH_ACC_TX_FRAME_ENET0_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   ) 
#define IX_ETH_ACC_TX_FRAME_ENET1_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   )
#define IX_ETH_ACC_TX_FRAME_DONE_ETH_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_NOT_E   )




