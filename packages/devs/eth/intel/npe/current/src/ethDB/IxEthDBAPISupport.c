/**
 * @file IxEthDBAPISupport.c
 *
 * @brief Public API support functions
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

#include <IxTypes.h>
#include <IxEthDB.h>
#include <IxNpeMh.h>

#include "IxEthDB_p.h"
#include "IxEthDBMessages_p.h"
#include "IxEthDB_p.h"
#include "IxEthDBLog_p.h"

#if !defined(NDEBUG) && !defined(NO_ELT_DEBUG)

int dbAccessCounter = 0;
int overflowEvent   = 0;

#endif

PortInfo ixEthDBPortInfo[IX_ETH_DB_NUMBER_OF_PORTS];

/**
 * @brief adds an entry to the database and triggers port updates
 *
 * @param portID port ID of the new entry
 * @param macAddr MAC address of the new entry
 * @param age age of the new entry (typically 0)
 * @param staticEntry use STATIC_ENTRY or DYNAMIC_ENTRY (static entries
 * are not subject to aging)
 *
 * This function is a wrapper over @ref ixEthDBAdd(), the only extra
 * functionality being the port update operation.
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr, UINT8 age, BOOL staticEntry)
{
    MacDescriptor *newMacDescriptor = NULL;
    IxEthDBStatus result;

	result = ixEthDBAdd(portID, macAddr, age, staticEntry, &newMacDescriptor);
	
    if (newMacDescriptor != NULL)
    {
        IX_ETH_DB_SUPPORT_TRACE("Added new MAC descriptor, triggering update\n");

	    ixEthDBTriggerAddPortUpdate(portID, newMacDescriptor);
    }

    return result;
}

IX_ETH_DB_PUBLIC
void ixEthDBPortInit(IxEthDBPortId portID)
{
    PortInfo *portInfo = &ixEthDBPortInfo[portID];

    if (portInfo->dependencyPortMap != 0)
    {
        /* redundant */
        return;
    }

    /* initialize core fields */
    portInfo->portID             = portID;
    portInfo->dependencyPortMap  = DEPENDENCY_MAP(portID);

    /* default values */
    portInfo->agingEnabled       = FALSE;
    portInfo->enabled            = FALSE;
    portInfo->macAddressUploaded = FALSE;

    /* default update control values */
    portInfo->updateMethod.accessRequestInProgress = FALSE;
    portInfo->updateMethod.searchTree              = NULL;
    portInfo->updateMethod.searchTreePendingWrite  = FALSE;
    portInfo->updateMethod.treeInitialized         = FALSE;
    portInfo->updateMethod.treeWriteAccess         = FALSE;
    portInfo->updateMethod.updateEnabled           = FALSE;
    portInfo->updateMethod.userControlled          = FALSE;

    /* Ethernet NPE-specific initializations */
    if (ixEthDBPortDefinitions[portID].type == ETH_NPE)
    {
        /* update handler */
        portInfo->updateMethod.updateHandler = ixEthDBNPEUpdateHandler;
        portInfo->updateMethod.syncAllowed   = TRUE; /* allow read-back of NPE tree entries after AccessGrant or BalanceRequest */

        /* initialize and empty PortAddress mutex */
        ixOsServMutexInit(&portInfo->ackPortAddressLock);
        ixOsServMutexLock(&portInfo->ackPortAddressLock);
    }
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortEnable(IxEthDBPortId portID)
{
    if (ixEthDBPortInfo[portID].enabled)
    {
        /* redundant */
        return IX_ETH_DB_SUCCESS;
    }

    /* mark as enabled */
    ixEthDBPortInfo[portID].enabled = TRUE;

#ifdef HAVE_ETH_LEARNING 

    if (ixEthDBPortDefinitions[portID].type == ETH_NPE && !ixEthDBPortInfo[portID].macAddressUploaded)
    {
        IX_ETH_DB_SUPPORT_TRACE("DB: (Support) MAC address not set on port %d, enable failed\n", portID);

        /* must use UnicastAddressSet() before enabling an NPE port */
        return IX_ETH_DB_MAC_UNINITIALIZED;
    }

    /* enable updates unless the user has specifically altered the default behavior */
    if (!ixEthDBPortInfo[portID].updateMethod.userControlled && ixEthDBPortDefinitions[portID].type == ETH_NPE)
    {
        ixEthDBPortInfo[portID].updateMethod.updateEnabled = TRUE;
    }

    if (ixEthDBPortDefinitions[portID].type == ETH_NPE)
    {
        IX_ETH_DB_SUPPORT_TRACE("DB: (Support) Attempting to enable the NPE callback for port %d...\n", portID);

        /* connect event processor callback for this port */
        return ixEthDBDefaultEventCallbackEnable(portID, TRUE);
    }
    else
    {
        IX_ETH_DB_SUPPORT_TRACE("DB: (Support) Enabling succeeded for port %d\n", portID);

        return IX_ETH_DB_SUCCESS;
    }

#else

    return IX_ETH_DB_SUCCESS;

#endif /* HAVE_ETH_LEARNING */

}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortDisable(IxEthDBPortId portID)
{
    if (!ixEthDBPortInfo[portID].enabled)
    {
        /* redundant */
        return IX_ETH_DB_SUCCESS;
    }

#ifdef HAVE_ETH_LEARNING

    /* disconnect event processor callback for this port */
    if (ixEthDBPortDefinitions[portID].type == ETH_NPE && ixEthDBDefaultEventCallbackEnable(portID, FALSE) != IX_ETH_DB_SUCCESS)
    {
        return IX_ETH_DB_FAIL;
    }

#endif /* HAVE_ETH_LEARNING */

    /* mark as disabled */
    ixEthDBPortInfo[portID].enabled = FALSE;

    /* disable updates unless the user has specifically altered the default behavior */
    if (ixEthDBPortDefinitions[portID].type == ETH_NPE && !ixEthDBPortInfo[portID].updateMethod.userControlled)
    {
        ixEthDBPortInfo[portID].updateMethod.updateEnabled = FALSE;
    }

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortAddressSet(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
{
#ifdef HAVE_ETH_LEARNING

    IxNpeMhMessage message;

    IxEthDBPortMap triggerPorts = DEPENDENCY_MAP(portID);
    IxEthDBPortMap excludePorts = EMPTY_DEPENDENCY_MAP;

    IxMutex *ackPortAddressLock  = &ixEthDBPortInfo[portID].ackPortAddressLock;

    /* exit if the port is not an Ethernet NPE */
    if (ixEthDBPortDefinitions[portID].type != ETH_NPE)
    {
        return IX_ETH_DB_INVALID_PORT;
    }

    /* populate message */
    FILL_ELT_MAC_MESSAGE(message, IX_ETHNPE_X2P_ELT_SETPORTADDRESS, portID, macAddr);

    IX_ETH_DB_SUPPORT_TRACE("Sending SetPortAddress on port %d...\n", portID);

    /* send a SetPortAddress message */
    if (ixNpeMhMessageWithResponseSend(IX_ETH_DB_PORT_ID_TO_NPE(portID),
        message,
        IX_ETHNPE_P2X_ELT_ACKPORTADDRESS,
        ixEthDBPortSetAckCallback,
        IX_NPEMH_SEND_RETRIES_DEFAULT) != IX_SUCCESS)
    {
        return IX_ETH_DB_FAIL;
    }

    /* block until P2X_ELT_AckPortAddress arrives */
    ixOsServMutexLock(ackPortAddressLock);

    /* if this is first time initialization then we already have
       write access to the tree and can AccessRelease directly */
    if (!ixEthDBPortInfo[portID].updateMethod.treeInitialized)
    {
        ixEthDBPortInfo[portID].updateMethod.treeWriteAccess = TRUE;

        IX_ETH_DB_SUPPORT_TRACE("Initializing tree for port %d using trigger set %x\n", portID, triggerPorts);

        /* temporarily enable updating port to create an initial tree */
        ixEthDBPortInfo[portID].updateMethod.updateEnabled = TRUE;

        /* create an initial tree and release access into it */
        ixEthDBUpdatePortLearningTrees(triggerPorts, excludePorts);

        /* disable update until PortEnable() is used on this port */
        ixEthDBPortInfo[portID].updateMethod.updateEnabled = FALSE;

        /* mark tree as being initialized */
        ixEthDBPortInfo[portID].updateMethod.treeInitialized = TRUE;
    }

#endif /* HAVE_ETH_LEARNING */

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief callback to acknowledge MAC address uploading
 *
 * @param npeID ID of NPE that generated the message
 * @param msg NPE message
 *
 * This routine is called when a P2X_ELT_AckPortAddress message is received
 * from an NPE. It updates the internal port-specific data structures to
 * indicate that the MAC address was uploaded successfully.
 *
 * @warning not to be called directly
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBPortSetAckCallback(IxNpeMhNpeId npeID, IxNpeMhMessage msg)
{
    IxEthDBPortId portID = IX_ETH_DB_NPE_TO_PORT_ID(npeID);
    PortInfo *portInfo;

    if (portID < 0 || portID >= IX_ETH_DB_NUMBER_OF_PORTS || ixEthDBPortDefinitions[portID].type != ETH_NPE)
    {
        ERROR_IRQ_LOG("Ethernet DB: PortSetAck callback received an invalid port value [0x%X], malformed response - possible deadlock condition\n", portID, 0, 0, 0, 0, 0);

        return;
    }

    portInfo = &ixEthDBPortInfo[portID];

    IX_ETH_DB_SUPPORT_IRQ_TRACE("Received AckPortAddress from port %d\n", portID, 0, 0, 0, 0, 0);

    /* unblock pending ixEthDBPortAddressSet */
    ixOsServMutexUnlock(&portInfo->ackPortAddressLock);

    /* allow calls to PortEnable */
    portInfo->macAddressUploaded = TRUE;
}
