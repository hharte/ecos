/**
 * @file IxEthDBEvents.c
 *
 * @brief Implementation of the event processor component
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

#include <ix_ossl.h>
#include <IxNpeMh.h>

#include "IxEthDB_p.h"

extern int accessGrantCounter, accessReleaseCounter;

/* forward prototype declarations */
IX_ETH_DB_PUBLIC ix_error ixEthDBEventProcessorLoop(void *, void **);
IX_ETH_DB_PUBLIC void ixEthDBNPEEventCallback(IxNpeMhNpeId npeID, IxNpeMhMessage msg);
IX_ETH_DB_PRIVATE void ixEthDBProcessEvent(PortEvent *event, IxEthDBPortMap *triggerPorts, IxEthDBPortMap *excludePorts, IxEthDBPortMap *forcePorts);
IX_ETH_DB_PRIVATE void ixEthDBDiscardPendingTrees(IxEthDBPortMap triggerPortMap);
IX_ETH_DB_PRIVATE void ixEthDBTriggerPortUpdate(PortEvent *event);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBStartLearningFunction(void);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBStopLearningFunction(void);

/* data */
IX_ETH_DB_PRIVATE ix_ossl_sem_t eventQueueSemaphore;
IX_ETH_DB_PRIVATE PortEventQueue eventQueue;
IX_ETH_DB_PRIVATE IxMutex eventQueueLock;
IX_ETH_DB_PRIVATE IxMutex overflowUpdatePortListLock;

BOOL ixEthDBLearningShutdown           = FALSE;
IxEthDBPortMap overflowUpdatePortList = EMPTY_DEPENDENCY_MAP;

/**
 * @brief initializes the event processor
 *
 * Initializes the event processor queue and processing thread.
 * Called from ixEthDBInit() DB-subcomponent master init function.
 *
 * @warning do not call directly
 *
 * @retval IX_ETH_DB_SUCCESS initialization was successful
 * @retval IX_ETH_DB_FAIL initialization failed (OSSL or mutex init failure)
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBEventProcessorInit(void)
{
    if (ixOsServMutexInit(&overflowUpdatePortListLock) != IX_SUCCESS)
    {
        return IX_ETH_DB_FAIL;
    }

    if (ixOsServMutexInit(&eventQueueLock) != IX_SUCCESS)
    {
        return IX_ETH_DB_FAIL;
    }

#ifdef HAVE_ETH_LEARNING
    /* start processor loop thread */
    if (ixEthDBStartLearningFunction() != IX_ETH_DB_SUCCESS)
    {
        return IX_ETH_DB_FAIL;
    }

#endif /* HAVE_ETH_LEARNING */

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBStartLearningFunction(void)
{
    ix_ossl_thread_t eventProcessorThread;

    /* reset event queue */
    ixOsServMutexLock(&eventQueueLock);

    RESET_QUEUE(&eventQueue);

    ixOsServMutexUnlock(&eventQueueLock);

    /* init event queue semaphore */
    if (ix_ossl_sem_init(0, &eventQueueSemaphore) != IX_OSSL_ERROR_SUCCESS)
    {
        return IX_ETH_DB_FAIL;
    }

    ixEthDBLearningShutdown = FALSE;

    /* start processor loop thread */
    if (ix_ossl_thread_create(ixEthDBEventProcessorLoop, NULL, &eventProcessorThread) != IX_OSSL_ERROR_SUCCESS)
    {
        return IX_ETH_DB_FAIL;
    }

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBStopLearningFunction(void)
{
    ixEthDBLearningShutdown = TRUE;

    /* wake up event processing loop to actually process the shutdown event */
    ix_ossl_sem_give(eventQueueSemaphore);

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief enables or disabled the default NPE event callback mechanism
 *
 * @param portID ID of the port (NPE) to enable/disable the default event processing
 * callback on
 * @param enable TRUE to enable and FALSE to disable
 *
 * @retval IX_ETH_DB_SUCCESS if the operation completed successfully
 * @retval IX_ETH_DB_FAIL if the operation has failed (message handler error)
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBDefaultEventCallbackEnable(IxEthDBPortId portID, BOOL enable)
{
    if (ixNpeMhUnsolicitedCallbackForRangeRegister(IX_ETH_DB_PORT_ID_TO_NPE(portID), 
        P2X_ELT_MIN_EVENT_ID,
        P2X_ELT_MAX_EVENT_ID,
        enable ? ixEthDBNPEEventCallback : NULL) != IX_SUCCESS)
    {
        return IX_ETH_DB_FAIL;
    }

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief default NPE event processing callback
 *
 * @param npeID ID of the NPE that generated the event
 * @param msg NPE message (encapsulated event)
 *
 * Creates an event object on the Ethernet event processor queue
 * and signals the new event by incrementing the event queue semaphore.
 * Events are processed by @ref ixEthDBEventProcessorLoop() which runs
 * at user level.
 *
 * @see ixEthDBEventProcessorLoop()
 *
 * @warning do not call directly
 *
 * @warning Executes ONLY at interrupt level - otherwise locking of the event queue is required
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBNPEEventCallback(IxNpeMhNpeId npeID, IxNpeMhMessage msg)
{
    PortEvent *event;

    IX_ETH_DB_IRQ_EVENTS_TRACE("DB: (Events) new event received by processor callback from port %d, id 0x%X\n", IX_ETH_DB_NPE_TO_PORT_ID(npeID), NPE_MSG_ID(msg), 0, 0, 0, 0);

    if (CAN_ENQUEUE(&eventQueue))
    {
        TEST_FIXTURE_LOCK_EVENT_QUEUE;

        event = QUEUE_HEAD(&eventQueue);

        /* create event structure on queue */
        event->eventType = NPE_MSG_ID(msg);
        event->portID    = IX_ETH_DB_NPE_TO_PORT_ID(npeID);

        if (event->eventType == IX_ETHNPE_P2X_ELT_ACCESSGRANT)
        {
            /* we receive the base address and size only on AccessGrant events */
            event->eltBaseAddress = NPE_MSG_ELT_BASE_ADDR(msg);
            event->eltSize        = NPE_MSG_ELT_SIZE(msg);
        }
        else
        {
            /* all the other events that we're hooked on have a MAC address except for AccessGrant */
            COPY_NPE_MSG_MAC_ADDR(msg, (&(event->macAddress)));
        }

        /* update queue */
        PUSH_UPDATE_QUEUE(&eventQueue);

        TEST_FIXTURE_UNLOCK_EVENT_QUEUE;

        IX_ETH_DB_IRQ_EVENTS_TRACE("DB: (Events) Waking up main processor loop...\n", 0, 0, 0, 0, 0, 0);

        /* increment event queue semaphore */
        ix_ossl_sem_give(eventQueueSemaphore);
    }
    else
    {
        UINT32 eventType = NPE_MSG_ID(msg);

        TEST_FIXTURE_MARK_OVERFLOW_EVENT;

        IX_ETH_DB_IRQ_EVENTS_TRACE("DB: (Events) Warning: could not enqueue event (overflow)\n", 0, 0, 0, 0, 0, 0);

        /* control NPE write access overflow */
        if (eventType == IX_ETHNPE_P2X_ELT_ACCESSGRANT
            || eventType == IX_ETHNPE_P2X_ELT_BALANCEREQUEST)
        {
            PortInfo *portInfo = &ixEthDBPortInfo[IX_ETH_DB_NPE_TO_PORT_ID(npeID)];

            /* unlock NPE tree */
            portInfo->updateMethod.treeWriteAccess         = TRUE;
            portInfo->updateMethod.accessRequestInProgress = FALSE;
	    
	    accessGrantCounter++;
        }
    }
}

/**
 * @brief Ethernet event processor loop
 *
 * Extracts at most EVENT_PROCESSING_LIMIT batches of events and
 * sends them for processing to @ref ixEthDBProcessEvent().
 * Triggers port updates which normally follow learning events.
 *
 * @warning do not call directly, executes in separate thread
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
ix_error ixEthDBEventProcessorLoop(void *unused1, void **unused2)
{
    IxEthDBPortMap triggerPorts;
    IxEthDBPortMap forcePorts;
    IxEthDBPortMap excludePorts;
    IxEthDBPortMap discardPorts;

    IxEthDBPortId portIndex;

    IX_ETH_DB_EVENTS_TRACE("DB: (Events) Event processor loop was started\n");

    while (!ixEthDBLearningShutdown)
	{
		BOOL keepProcessing    = TRUE;
		UINT32 processedEvents = 0;

        IX_ETH_DB_EVENTS_VERBOSE_TRACE("DB: (Events) Waiting for new learning event...\n");

		ix_ossl_sem_take(eventQueueSemaphore, IX_OSSL_WAIT_FOREVER);

        IX_ETH_DB_EVENTS_VERBOSE_TRACE("DB: (Events) Received new event\n");

        if (!ixEthDBLearningShutdown)
        {
		    /* port update handling */
		    triggerPorts = EMPTY_DEPENDENCY_MAP;
		    excludePorts = EMPTY_DEPENDENCY_MAP;
		    forcePorts   = EMPTY_DEPENDENCY_MAP;

		    while (keepProcessing)
		    {
   		        PortEvent event;
			    IX_IRQ_STATUS intLockKey;

			    /* lock queue */
			    ixOsServMutexLock(&eventQueueLock);

			    /* lock NPE interrupts */
			    intLockKey = ixOsServIntLock();
    
	            /* extract event */
                event = *(QUEUE_TAIL(&eventQueue));

                SHIFT_UPDATE_QUEUE(&eventQueue);
			
			    ixOsServIntUnlock(intLockKey);

                ixOsServMutexUnlock(&eventQueueLock);

                IX_ETH_DB_EVENTS_TRACE("DB: (Events) Processing event with ID 0x%X\n", event.eventType);
			
			    ixEthDBProcessEvent(&event, &triggerPorts, &excludePorts, &forcePorts);

			    processedEvents++;

			    if (processedEvents > EVENT_PROCESSING_LIMIT /* maximum burst reached? */
                    || ix_ossl_sem_take(eventQueueSemaphore, IX_OSSL_WAIT_NONE) != IX_OSSL_ERROR_SUCCESS) /* or empty queue? */
                {
			        keepProcessing = FALSE;
			    }
		    }

		    /* realign port update exclude list to execute the forced updates */
		    excludePorts = DIFF_MAPS(excludePorts, forcePorts);

            /* add ports causing event overflows to triggerPorts list - we need to restart the entire update
             * process for these ports as they have lost events */
            ixOsServMutexLock(&overflowUpdatePortListLock);

            discardPorts           = overflowUpdatePortList;
            triggerPorts           = JOIN_MAPS(triggerPorts, overflowUpdatePortList);
            overflowUpdatePortList = EMPTY_DEPENDENCY_MAP;

            ixOsServMutexUnlock(&overflowUpdatePortListLock);

            /* delete pending trees that depend on the overflowUpdatePortList  *
             * as we don't know the cause of the update (the events were lost) */
            ixEthDBDiscardPendingTrees(discardPorts);

		    ixEthDBUpdatePortLearningTrees(triggerPorts, excludePorts);
        }
	}

    /* disconnect NPE callbacks */
    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        ixEthDBDefaultEventCallbackEnable(portIndex, FALSE);

        ixEthDBPortInfo[portIndex].updateMethod.updateEnabled = FALSE;
    }

    /* remove pending trees */
    ixEthDBDiscardPendingTrees(COMPLETE_ETH_PORT_MAP);

    return IX_OSSL_ERROR_SUCCESS;
}

/**
 * @brief event processor routine
 *
 * @param event event to be processed
 * @param triggerPorts map of learning event triggers
 * @param excludePorts map of ports to be excluded from update
 * @param forcePorts map of ports to be forcibly updated
 *
 * Processes learning events by synchronizing the database and pending search trees with
 * newly learnt data. Called only by @ref ixEthDBEventProcessorLoop().
 *
 * @warning do not call directly
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
void ixEthDBProcessEvent(PortEvent *event, IxEthDBPortMap *triggerPorts, IxEthDBPortMap *excludePorts, IxEthDBPortMap *forcePorts)
{
    BOOL dbUpdate      = FALSE;
    PortInfo *portInfo = event->portID < MAX_PORT_NUMBER ? &ixEthDBPortInfo[event->portID] : NULL;

    TreeSyncInfo syncInfo;

    /* initialize syncInfo structure to 0 incremental updates*/
    syncInfo.addressCount = 0;

	switch (event->eventType)
	{
		case IX_ETHNPE_P2X_ELT_NEWADDRESS:
			*triggerPorts = JOIN_PORT_TO_MAP(*triggerPorts, event->portID); /* port triggered an update */
			*excludePorts = JOIN_PORT_TO_MAP(*excludePorts, event->portID); /* port is already up-to-date */

            IX_ETH_DB_EVENTS_TRACE("DB: (Events) Received new address event on port %d\n", event->portID);
    
            dbUpdate = TRUE;

            break;

		case IX_ETHNPE_P2X_ELT_BALANCEREQUEST:
			*triggerPorts = JOIN_PORT_TO_MAP(*triggerPorts, event->portID); /* port triggered an update */
			*forcePorts   = JOIN_PORT_TO_MAP(*forcePorts, event->portID);   /* force update on this port */
    
            dbUpdate = TRUE;

            /* since BalanceRequest doesn't pass the size of the tree we have to parse the entire zone */
            ixEthDBNPESyncScan(event->portID, portInfo->updateMethod.npeUpdateZone, FULL_ELT_BYTE_SIZE, &syncInfo);

            /* unlock NPE tree */
            portInfo->updateMethod.treeWriteAccess         = TRUE;
            portInfo->updateMethod.accessRequestInProgress = FALSE;
	    
	        accessGrantCounter++;

            IX_ETH_DB_EVENTS_TRACE("DB: (Events) Received access to search tree NPE %d via BalanceRequest\n", event->portID);

			break;

		case IX_ETHNPE_P2X_ELT_ACCESSGRANT:
            ixEthDBNPESyncScan(event->portID, portInfo->updateMethod.npeUpdateZone, event->eltSize, &syncInfo);

            /* unlock NPE tree */
            portInfo->updateMethod.treeWriteAccess         = TRUE;
            portInfo->updateMethod.accessRequestInProgress = FALSE;
	    
	        accessGrantCounter++;

            IX_ETH_DB_EVENTS_TRACE("DB: (Events) Received access to search tree NPE %d via AccessGrant\n", event->portID);

            break;

        case IX_ETH_DB_EXTERN_PORT_UPDATE: /* non-NPE event caused by API adds or removes */
            if (event->macDescriptor != NULL) /* was it an add to the database? */
            {
                /* add new address to pending search trees */
                syncInfo.macDescriptors[syncInfo.addressCount++] = event->macDescriptor;
            }
            else /* otherwise one or more records were removed from the database */
            {
                /* delete pending trees */
                ixEthDBDiscardPendingTrees(event->portMap);
            }

            /* note: here event->portMap could be a set of ports */
            *triggerPorts = JOIN_MAPS(*triggerPorts, event->portMap);
            *forcePorts   = JOIN_MAPS(*forcePorts, event->portMap);

            IX_ETH_DB_EVENTS_TRACE("DB: (Events) Received external port update request for port map 0x%X\n", event->portMap);

            break;

        default:
            /* can't handle this event type */
            ERROR_LOG("Ethernet DB: Event processor received an unknown event type (0x%X), malformed message?\n", event->eventType);

            return;
	}

    if (dbUpdate)
    {
        /* new event mac is added to the database */
        if (ixEthDBAdd(event->portID, &event->macAddress, 0, DYNAMIC_ENTRY, &syncInfo.macDescriptors[syncInfo.addressCount]) == IX_ETH_DB_SUCCESS)
        {
            syncInfo.addressCount++; /* only if add succeeded, otherwise the returned descriptor is invalid */
        }
    }

    if (syncInfo.addressCount > 0)
    {
        /* set originating port ID */
        syncInfo.portID = event->portID;

        /* NPEs and trees pending AccessGrant are incrementally updated with new sync info */
        ixEthDBLearningTreeSync(&syncInfo);

        /* free syncInfo descriptor clones */
        while (syncInfo.addressCount > 0)
        {
            syncInfo.addressCount--;

            ixEthDBFreeMacDescriptor(syncInfo.macDescriptors[syncInfo.addressCount]);
        }
    }
}

IX_ETH_DB_PRIVATE
void ixEthDBDiscardPendingTrees(IxEthDBPortMap triggerPortMap)
{
    UINT32 portIndex;

    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        PortInfo *portInfo = &ixEthDBPortInfo[portIndex];

        if (portInfo->updateMethod.searchTreePendingWrite && MAPS_COLLIDE(portInfo->dependencyPortMap, triggerPortMap))
        {
            if (portInfo->updateMethod.searchTree != NULL)
            {
                ixEthDBFreeMacTreeNode(portInfo->updateMethod.searchTree);
            }

            /* forget about last tree */
            portInfo->updateMethod.searchTree             = NULL;
            portInfo->updateMethod.searchTreePendingWrite = FALSE;
        }
    }
}

IX_ETH_DB_PUBLIC
void ixEthDBTriggerAddPortUpdate(IxEthDBPortId portID, MacDescriptor *macDescriptor)
{
    PortEvent event;

    /* when data is added we care about who and what since we can generate incremental updates */
    event.portID        = portID;
    event.macDescriptor = macDescriptor;
    event.portMap       = DEPENDENCY_MAP(portID);

    ixEthDBTriggerPortUpdate(&event);
}

IX_ETH_DB_PUBLIC
void ixEthDBTriggerRemovePortUpdate(IxEthDBPortMap portMap)
{
    PortEvent event;

    /* when data is removed we care only to know which pending trees to delete, no incremental updates are made */
    event.portMap       = portMap;
    event.portID        = MAX_PORT_SIZE; /* portMap already contains the set of ports with stale trees */
    event.macDescriptor = NULL;          /* indicate MAC removal */

    ixEthDBTriggerPortUpdate(&event);
}

IX_ETH_DB_PRIVATE
void ixEthDBTriggerPortUpdate(PortEvent *event)
{
    IX_IRQ_STATUS intLockKey;

    /* lock queue */
    ixOsServMutexLock(&eventQueueLock);

    /* lock NPE interrupts */
    intLockKey = ixOsServIntLock();

    if (CAN_ENQUEUE(&eventQueue))
    {
        PortEvent *queueEvent = QUEUE_HEAD(&eventQueue);

        /* update fields on the queue */
        queueEvent->eventType     = IX_ETH_DB_EXTERN_PORT_UPDATE;
        queueEvent->portID        = event->portID;
        queueEvent->portMap       = event->portMap;
        queueEvent->macDescriptor = event->macDescriptor;

        PUSH_UPDATE_QUEUE(&eventQueue);

        /* unlock NPE interrupts */
        ixOsServIntUnlock(intLockKey);

        /* unlock queue */
        ixOsServMutexUnlock(&eventQueueLock);

        /* imcrement event queue semaphore */
        ix_ossl_sem_give(eventQueueSemaphore);
    }
    else /* event queue full, salvage port updates */
    {
        /* unlock NPE interrupts */
        ixOsServIntUnlock(intLockKey);

        /* unlock queue */
        ixOsServMutexUnlock(&eventQueueLock);

        /* overflow events are added to overflowUpdatePortList for later processing */
        ixOsServMutexLock(&overflowUpdatePortListLock);

        overflowUpdatePortList = JOIN_MAPS(overflowUpdatePortList, event->portMap);

        ixOsServMutexUnlock(&overflowUpdatePortListLock);

        /* if there's a MAC descriptor clone in the event free it */
        if (event->macDescriptor != NULL)
        {
            ixEthDBFreeMacDescriptor(event->macDescriptor);
        }
    }
}
