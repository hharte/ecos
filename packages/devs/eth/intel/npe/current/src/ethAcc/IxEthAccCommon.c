/**
 * @file IxEthAccCommon.c
 *
 * @author Intel Corporation
 * @date 12-Feb-2002
 *
 * @brief This file contains the implementation common support routines for the component
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


/* 
 * Component header files 
 */
#include <IxEthAcc.h>
#include <IxEthAcc_p.h>
#include <IxEthAccQueueAssign_p.h>

/* 
 * Other components.
 */
#include <IxNpeDl.h>

/* 
 * ANSI headers
 */
#include <string.h>

/**
 * @addtogroup IxEthAccPri
 *@{
 */


/*
 * Module prototypes.
 */
IX_ETH_ACC_PRIVATE IxEthAccStatus ixEthAccGetCapabilitiesInit(void);

BOOL ixEthAccServiceInit=FALSE;


/** 
 *
 * @brief Capabilites of each NPE.
 */
IX_ETH_ACC_PRIVATE
ixEthAccCapabilites ixEthAccCapabilitesInfo[IX_ETH_ACC_NUMBER_OF_PORTS];



/**
 *
 * @brief Data structure used to register & initialize the QMgr callbacks.
 *
 */
IX_ETH_ACC_PRIVATE
ixEthAccQregInfo ixEthAccQmgrInfo[]=
{

  { 
    IX_ETH_ACC_RX_FRAME_ETH_Q, 	     /**< Queue ID */
    "Eth Rx Q", 
    ixEthRxFrameQMCallback,          /**< Functional callback */
    (IxQMgrCallbackId) 0,	     /**< Callback tag	      */ 
    IX_QMGR_Q_SIZE128,		     /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /** Queue Entry Sizes - all Q entries are single ord entries   */
    TRUE,			     /** Enable Q notification at startup */
    IX_ETH_ACC_RX_FRAME_ETH_Q_SOURCE, /** Q Condition to drive callback   */
    IX_QMGR_Q_WM_LEVEL0, 	     /* No queues use almost empty */
    IX_QMGR_Q_WM_LEVEL1,	     /** Q High water mark - needed by NPE */
    
  },


  { 
    IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q, 
    "Eth Rx Fr Q 1", 
    ixEthRxFreeQMCallback,      
    (IxQMgrCallbackId) IX_ETH_PORT_1,
    IX_QMGR_Q_SIZE128,		     /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /** Queue Entry Sizes - all Q entries are single ord entries   */
    FALSE,			     /** Disable Q notification at startup */
    IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q_SOURCE,	     /** Q Condition to drive callback  */
    IX_QMGR_Q_WM_LEVEL0, 	     /* No queues use almost empty */
    IX_QMGR_Q_WM_LEVEL64,	      /** Q High water mark - needed used  */
  },

  { 
    IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q, 
    "Eth Rx Fr Q 2", 
    ixEthRxFreeQMCallback,      
    (IxQMgrCallbackId) IX_ETH_PORT_2,
    IX_QMGR_Q_SIZE128,		     /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /** Queue Entry Sizes - all Q entries are single ord entries   */
    FALSE,			     /** Disable Q notification at startup */
    IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q_SOURCE,	     /** Q Condition to drive callback  */
    IX_QMGR_Q_WM_LEVEL0, 	     /* No queues use almost empty */
    IX_QMGR_Q_WM_LEVEL64,	      /** Q High water mark - needed used  */
  },

  { 
     IX_ETH_ACC_TX_FRAME_ENET0_Q,     
    "Eth Tx Q 1", 
     ixEthTxFrameQMCallback,     
     (IxQMgrCallbackId) IX_ETH_PORT_1,
    IX_QMGR_Q_SIZE128,		     /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /** Queue Entry Sizes - all Q entries are single ord entries   */
    FALSE,			     /** Disable Q notification at startup */
    IX_ETH_ACC_TX_FRAME_ENET0_Q_SOURCE,	     /** Q Condition to drive callback  */
    IX_QMGR_Q_WM_LEVEL0, 	     /* No queues use almost empty */
    IX_QMGR_Q_WM_LEVEL64,	      /** Q High water mark - needed used  */
  },

  { 
     IX_ETH_ACC_TX_FRAME_ENET1_Q,     
    "Eth Tx Q 2", 
     ixEthTxFrameQMCallback,     
     (IxQMgrCallbackId) IX_ETH_PORT_2,
    IX_QMGR_Q_SIZE128,		     /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /** Queue Entry Sizes - all Q entries are single ord entries   */
    FALSE,			     /** Disable Q notification at startup */
    IX_ETH_ACC_TX_FRAME_ENET1_Q_SOURCE,	     /** Q Condition to drive callback  */
    IX_QMGR_Q_WM_LEVEL0, 	     /* No queues use almost empty */
    IX_QMGR_Q_WM_LEVEL64,	      /** Q High water mark - needed used  */
  },


  { 
     IX_ETH_ACC_TX_FRAME_DONE_ETH_Q,  
    "Eth Tx Done Q", 
     ixEthTxFrameDoneQMCallback, 
     (IxQMgrCallbackId) 0,
    IX_QMGR_Q_SIZE128,		     /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /** Queue Entry Sizes - all Q entries are single ord entries   */
    TRUE,			     /** Enable Q notification at startup */
    IX_ETH_ACC_TX_FRAME_DONE_ETH_Q_SOURCE,	 /** Q Condition to drive callback  */
    IX_QMGR_Q_WM_LEVEL0, 	     /* No queues use almost empty */
    IX_QMGR_Q_WM_LEVEL1,	      /** Q High water mark - needed by NPE */
  },

  {  /* Null Termination entry */
     (IxQMgrQId) NULL,		     
     (char *) NULL,
     (IxQMgrCallback) NULL,	 
     (IxQMgrCallbackId) 0,
     0,
     0,
     0,
     0,
     0,
     0
  } 

};

/**
 * @fn IxEthAccStatus ixEthAccGetCapabilitiesInit(void)
 *
 * @brief Get the NPE capabilities from the NPE download manager.
 *
 * @param none
 *
 * @return IxEthAccStatus
 *   IX_ETH_ACC_SUCCESS
 *   IX_ETH_ACC_FAIL
 *
 * @internal
 */

IX_ETH_ACC_PRIVATE
IxEthAccStatus ixEthAccGetCapabilitiesInit(void)
{

  IxEthAccStatus ret=IX_ETH_ACC_SUCCESS;

   IX_ETH_ACC_MEMSET( &ixEthAccCapabilitesInfo,0, sizeof(ixEthAccCapabilitesInfo) );

  /* 
   * Check with NPE downloader to get NPE capabilities and version information 
   */
   if ( ixNpeDlLoadedVersionGet(IX_NPEDL_NPEID_NPEB , &ixEthAccCapabilitesInfo[IX_ETH_PORT_1].versionId) != IX_SUCCESS )
   {
	diag_printf("EthAcc:NPE[B]-Port 1:  Image verification failed\n");
	IX_ETH_ACC_DEBUG_LOG("EthAcc:NPE[B]-Port 1:  Image verification failed\n",0,0,0,0,0,0);
        return(IX_FAIL);
   }
   if ( ixNpeDlLoadedVersionGet(IX_NPEDL_NPEID_NPEC , &ixEthAccCapabilitesInfo[IX_ETH_PORT_2].versionId) != IX_SUCCESS )
   {
	diag_printf("EthAcc:NPE[C]-Port 2:  Image verification failed\n");
	IX_ETH_ACC_DEBUG_LOG("EthAcc:NPE[C]-Port 2:  Image verification failed\n",0,0,0,0,0,0);
        return(IX_FAIL);
   }
   /**
    * Check that we have the prerequisite capabilities.
    * @todo: Check the cabilites of the NPE's.
    */
 

   return(ret);

}


/**
 * @fn IX_ETH_ACC_PRIVATE IxEthAccStatus ixEthAccInitCommon(void)
 *
 * @brief  Central place to register to other components.
 *
 * @param none
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_FAIL
 *
 * @internal
 */
IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccInitCommon(void)
{

   IxEthAccStatus ret=IX_ETH_ACC_SUCCESS;
  
   if ( ixEthAccGetCapabilitiesInit() != IX_ETH_ACC_SUCCESS )
   {
      ret = IX_ETH_ACC_FAIL;
      return(ret);
   }

   return(ret);

}


/**
 * @fn ixEthAccRegisterQMgrCallbacks(void)
 *
 * @brief Register all callbacks required by this component to the QMgr
 *
 * @param none
 *
 * @return IxEthAccStatus
 *
 * @internal
 */
IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccRegisterQMgrCallbacks(void)
{

  IxEthAccStatus ret=IX_ETH_ACC_SUCCESS;

  ixEthAccQregInfo *qInfo = &ixEthAccQmgrInfo[0];


  while ( (qInfo->qCallback != (IxQMgrCallback) NULL ) && ret == IX_ETH_ACC_SUCCESS)
  {

    /*
     * Configure each Q.
     */
    if ( ixQMgrQConfig( qInfo->qName,
			qInfo->qId,
			qInfo->qSize,
			qInfo->qWords) != IX_SUCCESS)
    {
	ret = IX_ETH_ACC_FAIL;
	break;
    }

    if ( ixQMgrWatermarkSet( qInfo->qId,
			     qInfo->AlmostEmptyThreshold,
			     qInfo->AlmostFullThreshold
			     ) != IX_SUCCESS)
    {
	ret = IX_ETH_ACC_FAIL;
	break;
    }
    /* 
     * Set dispatcher priority. 
     */
    if ( ixQMgrDispatcherPrioritySet( qInfo->qId, IX_ETH_ACC_QM_QUEUE_DISPATCH_PRIORITY) != IX_SUCCESS)
    {
	ret = IX_ETH_ACC_FAIL;
	break;
    }
   
    /*
     * Register callbacks for each Q.
     */ 
    if (   ixQMgrNotificationCallbackSet(qInfo->qId,
				      qInfo->qCallback,
				      qInfo->callbackTag) != IX_SUCCESS )
    {
	ret = IX_ETH_ACC_FAIL;
	break;
    }

    /*
     * Set notification condition for Q
     */  
    if ( qInfo->qNotificationEnableAtStartup == TRUE ) 
    {
      if (   ixQMgrNotificationEnable(qInfo->qId,
                                    qInfo->qConditionSource) != IX_SUCCESS )
      {
        ret = IX_ETH_ACC_FAIL;
        break;
      }
    }
 
    ++qInfo;

  }

  return(ret);
}

/**
 * @fn
 *
 * @brief
 *
 * @param
 *
 * @return none
 *
 * @internal
 */


