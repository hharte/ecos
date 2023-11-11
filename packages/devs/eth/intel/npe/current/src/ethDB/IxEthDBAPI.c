/**
 * @file IxEthDBAPI.c
 *
 * @brief Implementation of the public API
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
#include <stdio.h>
#include "IxEthDB_p.h"

#ifdef __ECOS
#ifndef printf
#define printf diag_printf
#endif
#endif

extern HashTable dbHashtable;
extern IxEthDBPortMap overflowUpdatePortList;

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringStaticEntryProvision(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
{
    IX_ETH_DB_CHECK_PORT(portID);
    CHECK_REFERENCE(macAddr);

    return ixEthDBFilteringEntryAdd(portID, macAddr, 0 /* age */, STATIC_ENTRY);
}
    
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringDynamicEntryProvision(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
{
    IX_ETH_DB_CHECK_PORT(portID);
    CHECK_REFERENCE(macAddr);

    return ixEthDBFilteringEntryAdd(portID, macAddr, 0 /* age */, DYNAMIC_ENTRY);
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringEntryDelete(IxEthDBMacAddr *macAddr)
{
    HashNode *searchResult;
    IxEthDBPortId portID;

    CHECK_REFERENCE(macAddr);

    searchResult = ixEthDBSearch(macAddr);

    if (searchResult == NULL)
    {
        return IX_ETH_DB_NO_SUCH_ADDR; /* not found */
    }

    /* extract the port ID */
    portID = ((MacDescriptor *) searchResult->data)->portID;

    ixEthDBReleaseHashNode(searchResult);

    /* delete record */
	if (ixEthDBRemove(macAddr) == IX_ETH_DB_SUCCESS)
    {
        /* disable ELT->XScaleDB entry sync for the first access grant otherwise we'll readd the entry */
        ixEthDBPortInfo[portID].updateMethod.syncAllowed = FALSE;

	    ixEthDBTriggerRemovePortUpdate(DEPENDENCY_MAP(portID));
    }
    else
    {
        /* failed, probably was removed in another thread */
        return IX_ETH_DB_NO_SUCH_ADDR;
    }

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
void ixEthDBDatabaseMaintenance()
{
    HashIterator iterator;
    UINT32 portIndex;

    /* ports to trigger age synchronization from NPEs */
    IxEthDBPortMap agingPorts = EMPTY_DEPENDENCY_MAP;

	/* init update list - ports who will have deleted records and therefore will need updating */
	IxEthDBPortMap triggerPorts = EMPTY_DEPENDENCY_MAP;

    /* check if there's at least a port that needs aging */
    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        if (ixEthDBPortInfo[portIndex].agingEnabled)
        {
            agingPorts = JOIN_PORT_TO_MAP(agingPorts, portIndex);
        }
    }

    if (agingPorts != EMPTY_DEPENDENCY_MAP)
    {
	    /* browse database and age entries */
        BUSY_RETRY(ixEthDBInitHashIterator(&dbHashtable, &iterator));

	    while (IS_ITERATOR_VALID(&iterator))
	    {
            MacDescriptor *descriptor = (MacDescriptor *) iterator.node->data;

		    if (ixEthDBPortInfo[descriptor->portID].agingEnabled && !descriptor->staticEntry)
            {
                /* manually increment the age if the port has no such capability */
                if ((ixEthDBPortDefinitions[descriptor->portID].capabilities & ENTRY_AGING) == 0)
                {
                    descriptor->age += (IX_ETH_DB_MAINTENANCE_TIME / 60);
                }

                /* age entry */
                if (descriptor->age >= (IX_ETH_DB_LEARNING_ENTRY_AGE_TIME / 60))
                {
                    /* add port to the set of update trigger ports */
			        triggerPorts = JOIN_PORT_TO_MAP(triggerPorts, ((MacDescriptor *) iterator.node->data)->portID);

                    /* delete entry */
			        BUSY_RETRY(ixEthDBRemoveEntryAtHashIterator(&dbHashtable, &iterator));

                    /* disable ELT->XScaleDB entry sync for the first access grant otherwise we'll readd the entry */
                    ixEthDBPortInfo[descriptor->portID].updateMethod.syncAllowed = FALSE;
                }
                else
                {
                    /* move to the next record */
                    BUSY_RETRY(ixEthDBIncrementHashIterator(&dbHashtable, &iterator));
                }
            }
            else
            {
                /* move to the next record */
                BUSY_RETRY(ixEthDBIncrementHashIterator(&dbHashtable, &iterator));
            }
         }

        /* trigger age synchronization for next maintenance cleanup */
        triggerPorts = JOIN_MAPS(triggerPorts, agingPorts);
    }

    /* if we've removed any records or lost any events make sure to force an update */
    if (JOIN_MAPS(triggerPorts, overflowUpdatePortList) != EMPTY_DEPENDENCY_MAP)
    {
	    ixEthDBTriggerRemovePortUpdate(triggerPorts);
    }
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringPortSearch(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
{
    HashNode *searchResult;
    IxEthDBStatus result = IX_ETH_DB_NO_SUCH_ADDR;

    IX_ETH_DB_CHECK_PORT(portID);
    CHECK_REFERENCE(macAddr);

    searchResult = ixEthDBSearch(macAddr);

    if (searchResult == NULL)
    {
        return IX_ETH_DB_NO_SUCH_ADDR; /* not found */
    }

    if (((MacDescriptor *) (searchResult->data))->portID == portID)
    {
        result = IX_ETH_DB_SUCCESS; /* address and port match */
    }

    ixEthDBReleaseHashNode(searchResult);

    return result;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringDatabaseSearch(IxEthDBPortId *portID, IxEthDBMacAddr *macAddr)
{
    HashNode *searchResult;

    CHECK_REFERENCE(portID);
    CHECK_REFERENCE(macAddr);

    searchResult = ixEthDBSearch(macAddr);

    if (searchResult == NULL)
    {
        return IX_ETH_DB_NO_SUCH_ADDR; /* not found */
    }

    /* return the port ID */
    *portID = ((MacDescriptor *) searchResult->data)->portID;

    ixEthDBReleaseHashNode(searchResult);

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortAgingDisable(IxEthDBPortId portID)
{
    IX_ETH_DB_CHECK_PORT(portID);

    ixEthDBPortInfo[portID].agingEnabled = FALSE;

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortAgingEnable(IxEthDBPortId portID)
{
    IX_ETH_DB_CHECK_PORT(portID);

    ixEthDBPortInfo[portID].agingEnabled = TRUE;

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringDatabaseShow(IxEthDBPortId portID)
{
    IxEthDBStatus result;
    HashIterator iterator;
    PortInfo *portInfo;
    UINT32 recordCount = 0;

    IX_ETH_DB_CHECK_PORT(portID);

    portInfo = &ixEthDBPortInfo[portID];

    /* display table header */
    printf("Ethernet database records for port ID [%d] - dependency port map 0x%X (%s), ",
        portID,
        portInfo->dependencyPortMap,
        portInfo->dependencyPortMap == DEPENDENCY_MAP(portID) ? "itself" : "group");

    if (ixEthDBPortDefinitions[portID].type == ETH_NPE)
    {
        printf("NPE updates are %s\n\n", portInfo->updateMethod.updateEnabled ? "enabled" : "disabled");
    }
    else
    {
        printf("updates disabled (not an NPE)\n\n");
    }

    printf("    MAC address    |   Age  | Type \n");
    printf("___________________________________\n");

	/* browse database and age entries */
    BUSY_RETRY(ixEthDBInitHashIterator(&dbHashtable, &iterator));

	while (IS_ITERATOR_VALID(&iterator))
	{
        MacDescriptor *descriptor = (MacDescriptor *) iterator.node->data;

        if (descriptor->portID == portID)
        {
            recordCount++;

            /* display entry */
            printf(" %02X:%02X:%02X:%02X:%02X:%02X | %5d  | %s\n",
                descriptor->macAddress[0],
                descriptor->macAddress[1],
                descriptor->macAddress[2],
                descriptor->macAddress[3],
                descriptor->macAddress[4],
                descriptor->macAddress[5],
                descriptor->age,
                descriptor->staticEntry ? "static" : "dynamic");
        }

        /* move to the next record */
        BUSY_RETRY_WITH_RESULT(ixEthDBIncrementHashIterator(&dbHashtable, &iterator), result);

        /* debug */
        if (result == IX_ETH_DB_BUSY)
        {
            return IX_ETH_DB_FAIL;
        }
	}

    /* display number of records */
    printf("\n%d records found\n", recordCount);

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
void ixEthDBFilteringDatabaseShowAll()
{
    IxEthDBPortId portIndex;

    printf("\nEthernet learning/filtering database: listing %d ports\n\n", IX_ETH_DB_NUMBER_OF_PORTS);

    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        ixEthDBFilteringDatabaseShow(portIndex);

        if (portIndex < IX_ETH_DB_NUMBER_OF_PORTS - 1)
        {
            printf("\n");
        }
    }
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBFilteringPortUpdatingSearch(IxEthDBPortId *portID, IxEthDBMacAddr *macAddr)
{
    HashNode *searchResult;

    CHECK_REFERENCE(portID);
    CHECK_REFERENCE(macAddr);

    searchResult = ixEthDBSearch(macAddr);

    if (searchResult == NULL)
    {
        return IX_ETH_DB_NO_SUCH_ADDR; /* not found */
    }

    /* return the port ID */
    *portID = ((MacDescriptor *) searchResult->data)->portID;

    /* reset entry age */
    ((MacDescriptor *) searchResult->data)->age = 0;

    ixEthDBReleaseHashNode(searchResult);

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortDependencyMapSet(IxEthDBPortId portID, IxEthDBPortMap dependencyPortMap)
{
    IX_ETH_DB_CHECK_PORT(portID);
    CHECK_MAP(portID, dependencyPortMap);

    ixEthDBPortInfo[portID].dependencyPortMap = dependencyPortMap;

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBPortUpdateEnableSet(IxEthDBPortId portID, BOOL enableUpdate)
{
    IX_ETH_DB_CHECK_PORT(portID);

    ixEthDBPortInfo[portID].updateMethod.updateEnabled  = enableUpdate;
    ixEthDBPortInfo[portID].updateMethod.userControlled = TRUE;

    return IX_ETH_DB_SUCCESS;
}
