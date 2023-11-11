/**
 * @file IxEthAcc.c
 *
 * @author Intel Corporation
 * @date 20-Feb-2001
 *
 * @brief This file contains the implementation of the IXP425 Ethernet Access Component
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



#include <IxEthAcc.h>
#include <IxEthDB.h>

#include "IxEthAcc_p.h"
#include "IxEthAccMac_p.h"
#include "IxEthAccMii_p.h"

/* prototypes */
IxEthAccStatus ixEthAccMacInit(IxEthAccPortId portId);

/**
 * @addtogroup IxEthAcc
 *@{
 */


/**
 * @brief Per port information data strucure.
 *
 * @ingroup IxEthAccPri
 *
 */

ixEthAccPortDataInfo ixEthAccPortData[IX_ETH_ACC_NUMBER_OF_PORTS];



IxEthAccStatus ixEthAccInit()
{
  /*
   * Initialize the service and register callback to other services.
   */

   if ( ixEthAccInitCommon() != IX_ETH_ACC_SUCCESS )
   {
       diag_printf("ixEthAccInitCommon() failed\n");
       return IX_ETH_ACC_FAIL;
   }

  /*
   * Initialize Control plane
   */
  if (ixEthDBInit() != IX_ETH_ACC_SUCCESS)
  {
       diag_printf("ixEthDBInit() failed\n");
      return IX_ETH_ACC_FAIL;
  }

  /*
   * Initialize Data plane
   */
   if ( ixEthAccInitDataPlane()  != IX_ETH_ACC_SUCCESS )
   {
       diag_printf("ixEthAccInitDataPlane() failed\n");
       return IX_ETH_ACC_FAIL;
   }


   if ( ixEthAccRegisterQMgrCallbacks() != IX_ETH_ACC_SUCCESS )
   {
       diag_printf("ixEthAccRegisterQMgrCallbacks() failed\n");
       return IX_ETH_ACC_FAIL;
   }

   /*
    * Initialize MII 
    */
   if ( ixEthAccMiiInit() != IX_ETH_ACC_SUCCESS )
   {
       diag_printf("ixEthAccMiiInit() failed\n");
       return IX_ETH_ACC_FAIL;
   }
  /*
   * Set init flag.
   */

   ixEthAccServiceInit = TRUE;
   
   return IX_ETH_ACC_SUCCESS;

}


IxEthAccStatus ixEthAccPortInit( IxEthAccPortId portId)
{

  IxEthAccStatus ret=IX_ETH_ACC_SUCCESS;



   if ( !  IX_ETH_ACC_IS_SERVICE_INITIALIZED() ) 
   {
   	return(IX_ETH_ACC_FAIL);
   }

   /*
    * Check for valid port
    */
    
   if ( ! IX_ETH_ACC_IS_PORT_VALID(portId) )
   {
       return (IX_ETH_ACC_INVALID_PORT);
   }   

   if ( IX_ETH_IS_PORT_INITIALIZED(portId) )
   {
   	/* Already initialized */
   	return(IX_ETH_ACC_FAIL);
   }
   
    /*
     * Clear port values - 0.
     */
    IX_ETH_ACC_MEMSET(&ixEthAccPortData[portId], 0, sizeof(ixEthAccPortDataInfo));


    /* 
     * Clear port callbacks.
     */

    ixEthAccPortData[portId].ixEthAccRxData.rxCallbackFn = 0;
    ixEthAccPortData[portId].ixEthAccRxData.rxCallbackTag = 0;
    ixEthAccPortData[portId].ixEthAccRxData.freeBufferList.pHead = NULL;
    ixEthAccPortData[portId].ixEthAccRxData.freeBufferList.pTail = NULL;

    ixEthAccPortData[portId].ixEthAccTxData.txBufferDoneCallbackFn = 0;
    ixEthAccPortData[portId].ixEthAccTxData.txCallbackTag = 0;
    ixEthAccPortData[portId].ixEthAccTxData.schDiscipline = FIFO_NO_PRIORITY;


    /*
     * Set the port init flag.
     */

    ixEthAccPortData[portId].portInitialized = TRUE;
    
    if(ixEthAccMacInit(portId)!=IX_ETH_ACC_SUCCESS)
    {
	return IX_ETH_ACC_FAIL;
    }

    /* init learning/filtering database structures for this port */
    ixEthDBPortInit(portId);

    return(ret);    
}


