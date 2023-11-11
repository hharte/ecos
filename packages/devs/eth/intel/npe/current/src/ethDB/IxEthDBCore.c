/**
 * @file IxEthDBDBCore.c
 *
 * @brief Database support functions
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

#include "IxEthDB_p.h"

/* list of database hashtables */
HashTable dbHashtable;

IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBInit()
{
#ifdef HAVE_ETH_LEARNING
    /* memory management */
    ixEthDBInitMemoryPools();

    /* hash table */
    ixEthDBInitHash(&dbHashtable, NUM_BUCKETS, ixEthDBEntryXORHash, ixEthDBKeyXORHash, ixEthDBAddressMatch, (FreeFunction) ixEthDBFreeMacDescriptor);

    /* NPE update zones */
    ixEthDBNPEUpdateAreasInit();

    /* event processor */
    return ixEthDBEventProcessorInit();
#else
    return IX_ETH_DB_SUCCESS;
#endif
}

/**
 * @brief adds a new entry to the Ethernet database
 *
 * @param portID port ID where the entry must be filtered
 * @param macAddress MAC address
 * @param age entry age (usually this is 0 for new records)
 * @param staticEntry set to STATIC_ENTRY or DYNAMIC_ENTRY - static entries do not age
 * @param descriptor pointer where the newly created mac descriptor can be stored
 * (NULL if the caller is not interested)
 *
 * Creates a new database entry, populates it with the given data and adds 
 * it into the database hash table.
 *
 * @retval IX_ETH_DB_SUCCESS insertion successful
 * @retval IX_ETH_DB_NOMEM insertion failed, no memory left in the mac descriptor memory pool
 * @retval IX_ETH_DB_FAIL insertion failed for other reasons (e.g. database busy)
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddress, UINT8 age, BOOL staticEntry, MacDescriptor **descriptor)
{
    IxEthDBStatus result;
    MacDescriptor *newDescriptor;
    IxEthDBPortId originalPortID;
    BOOL removeOriginalRecord = FALSE;
    HashNode *node            = ixEthDBSearch(macAddress);
    
    TEST_FIXTURE_INCREMENT_DB_CORE_ACCESS_COUNTER;

    /* safety set in case the add fails */
    if (descriptor != NULL)
    {
        *descriptor = NULL;
    }

    if (node == NULL)
    {
        /* not found, create a new one */
        newDescriptor = ixEthDBAllocMacDescriptor();

        if (newDescriptor == NULL)
        {
            return IX_ETH_DB_NOMEM; /* no memory */
        }

        /* write MAC into new descriptor */
        memcpy(newDescriptor->macAddress, macAddress, sizeof (IxEthDBMacAddr));
    }
    else
    {
        /* a node with the same MAC already exists, will update node */
        newDescriptor = (MacDescriptor *) node->data;

        /* check whether we want to remove the record from the original port */
        if (newDescriptor->portID != portID)
        {
            removeOriginalRecord = TRUE;
        }
    }

    /* save original port id */
    originalPortID = newDescriptor->portID;

    /* add/update the other fields */
    newDescriptor->portID      = portID;
    newDescriptor->age         = staticEntry ? 0 /* reset age for static entries */ : age;
    newDescriptor->staticEntry = staticEntry;

    if (node == NULL)
    {
        /* insert into hashtable */
        BUSY_RETRY_WITH_RESULT(ixEthDBAddHashEntry(&dbHashtable, newDescriptor), result);

        if (result != IX_ETH_DB_SUCCESS)
        {
            ixEthDBFreeMacDescriptor(newDescriptor);

            return result; /* insertion failed */
        }
    }
    
    /* return new descriptor address if desired */
    if (descriptor != NULL)
    {
        *descriptor = ixEthDBCloneMacDescriptor(newDescriptor);
    }

    if (node != NULL)
    {
        /* release access */
        ixEthDBReleaseHashNode(node);
    }

    /* when switching a record between ports delete the original one */
    if (removeOriginalRecord)
    {
        ixEthDBPortInfo[originalPortID].updateMethod.syncAllowed = FALSE;

        ixEthDBTriggerRemovePortUpdate(DEPENDENCY_MAP(originalPortID));
    }

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief remove a record from the Ethernet database
 *
 * @param macAddress MAC address of the entry to be removed
 *
 * @retval IX_ETH_DB_SUCCESS removal was successful
 * @retval IX_ETH_DB_NO_SUCH_ADDR the record with the given MAC address was not found
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBRemove(IxEthDBMacAddr *macAddress)
{
    IxEthDBStatus result;
    
    TEST_FIXTURE_INCREMENT_DB_CORE_ACCESS_COUNTER;

    BUSY_RETRY_WITH_RESULT(ixEthDBRemoveHashEntry(&dbHashtable, macAddress), result);

    if (result != IX_ETH_DB_SUCCESS)
    {
        return IX_ETH_DB_NO_SUCH_ADDR; /* not found */
    }

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief search a record in the Ethernet datbase
 *
 * @param macAddress MAC address to perform the search on
 *
 * @warning if searching is successful an implicit write lock
 * to the search result is granted, therefore unlock the 
 * entry using @ref ixEthDBReleaseHashNode() as soon as possible.
 *
 * @see ixEthDBReleaseHashNode()
 *
 * @return the search result, or NULL if a record with the given
 * MAC address was not found
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
HashNode* ixEthDBSearch(IxEthDBMacAddr *macAddress)
{
    HashNode *searchResult = NULL;
    
    TEST_FIXTURE_INCREMENT_DB_CORE_ACCESS_COUNTER;

    if (macAddress == NULL)
    {
        return NULL;
    }

    BUSY_RETRY(ixEthDBSearchHashEntry(&dbHashtable, macAddress, &searchResult));

    return searchResult;
}

/**
 * @brief updates the fields of a database record
 *
 * @param macAddress MAC address used to identify the record
 * @param portID new record port ID
 * @param age new record age
 * @param staticEntry new record static/dynamic state
 *
 * @retval IX_ETH_DB_SUCCESS if the update was successful
 * @retval IX_ETH_DB_NO_SUCH_ADDR if no record with the given
 * MAC address was found
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthDBUpdate(IxEthDBMacAddr *macAddress, IxEthDBPortId portID, UINT32 age, BOOL staticEntry)
{
    HashNode *searchResult    = ixEthDBSearch(macAddress);
    BOOL removeOriginalRecord = FALSE;
    IxEthDBPortId originalPortID;
    MacDescriptor *updateEntry;
    
    TEST_FIXTURE_INCREMENT_DB_CORE_ACCESS_COUNTER;
    
    if (searchResult == NULL)
    {
        return IX_ETH_DB_NO_SUCH_ADDR; /* not found */
    }

    /* now searchResult points to a valid HashNode */
    updateEntry = (MacDescriptor *) searchResult->data;

    /* check whether we need to remove the record from the original port */
    if (updateEntry->portID != portID)
    {
        removeOriginalRecord = TRUE;
    }

    /* save original port */
    originalPortID = updateEntry->portID;

    updateEntry->portID      = portID;
    updateEntry->age         = staticEntry ? 0 /* reset age for static entries */ : age;
    updateEntry->staticEntry = staticEntry;

    ixEthDBReleaseHashNode(searchResult);

    /* when switching a record between ports delete the original one */
    if (removeOriginalRecord)
    {
        ixEthDBPortInfo[originalPortID].updateMethod.syncAllowed = FALSE;

        ixEthDBTriggerRemovePortUpdate(DEPENDENCY_MAP(originalPortID));
    }

    return IX_ETH_DB_SUCCESS;
}
