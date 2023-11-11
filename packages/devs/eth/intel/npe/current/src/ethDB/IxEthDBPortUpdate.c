/**
 * @file IxEthDBDBPortUpdate.c
 *
 * @brief Implementation of dependency and port update handling
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

int accessGrantCounter = 0;
int accessReleaseCounter = 0;

/* forward prototype declarations */
IX_ETH_DB_PRIVATE MacTreeNode* ixEthDBQuery(MacTreeNode *searchTree, IxEthDBPortMap query);
IX_ETH_DB_PRIVATE MacTreeNode* ixEthDBTreeInsert(MacTreeNode *searchTree, MacDescriptor *descriptor);
IX_ETH_DB_PRIVATE void ixEthDBCreateTrees(IxEthDBPortMap updatePorts);
IX_ETH_DB_PRIVATE MacTreeNode* ixEthDBTreeRebalance(MacTreeNode *searchTree);
IX_ETH_DB_PRIVATE void ixEthDBRebalanceTreeToVine(MacTreeNode *root, UINT32 *size);
IX_ETH_DB_PRIVATE void ixEthDBRebalanceVineToTree(MacTreeNode *root, UINT32 size);
IX_ETH_DB_PRIVATE void ixEthDBRebalanceCompression(MacTreeNode *root, UINT32 count);
IX_ETH_DB_PRIVATE UINT32 ixEthDBRebalanceLog2Floor(UINT32 x);

extern HashTable dbHashtable;

/**
 * @brief computes dependencies and triggers port learning tree updates
 *
 * @param triggerPorts port map consisting in the ports which triggered the update
 * @param excludePorts port map indicating ports to be specifically excluded from the update
 *        after all the inter-port dependencies are computed
 *
 * This function browses through all the ports and determines how to waterfall the update
 * event from the trigger ports to all other ports depending on them, taking into account
 * update exclusions which happen when ports are already up-to-date.
 * Once it has determined what ports are to be updated it calls @ref ixEthDBCreateTrees.
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBUpdatePortLearningTrees(IxEthDBPortMap triggerPorts, IxEthDBPortMap excludePorts)
{
    IxEthDBPortMap updatePorts = EMPTY_DEPENDENCY_MAP;
    UINT32 portIndex;
    
    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        PortInfo *port = &ixEthDBPortInfo[portIndex];

        if ((MAPS_COLLIDE(triggerPorts, port->dependencyPortMap)       /* do triggers influence this port? */
                && !IS_PORT_INCLUDED(portIndex, updatePorts)           /* and it's not already in the update list */
                && !IS_PORT_INCLUDED(portIndex, excludePorts)          /* and it's not explicitely excluded from update as for NewAddress */
                && port->updateMethod.updateEnabled)                   /* and we're allowed to update it */
            || port->updateMethod.searchTreePendingWrite)              /* or force push in case AccessGrant has arrived for a pending tree */
        {
            IX_ETH_DB_UPDATE_TRACE("Adding port %d to update set 0x%x\n", portIndex, updatePorts);

            updatePorts = JOIN_PORT_TO_MAP(updatePorts, portIndex);
        }
        else
        {
            IX_ETH_DB_UPDATE_TRACE("Didn't add port %d to update set 0x%x, reasons follow:\n", portIndex, updatePorts);

            if (!MAPS_COLLIDE(triggerPorts, port->dependencyPortMap))
            {
                IX_ETH_DB_UPDATE_TRACE("\tMaps don't collide on port %d (trigger set 0x%x and dependency map 0x%x)\n", portIndex, triggerPorts, port->dependencyPortMap);
            }

            if (IS_PORT_INCLUDED(portIndex, updatePorts))
            {
                IX_ETH_DB_UPDATE_TRACE("\tPort %d is already in update set 0x%x\n", portIndex, updatePorts);
            }

            if (IS_PORT_INCLUDED(portIndex, excludePorts))
            {
                IX_ETH_DB_UPDATE_TRACE("\tPort %d is in the exclude set 0x%x\n", portIndex, excludePorts);
            }

            if (!port->updateMethod.updateEnabled)
            {
                IX_ETH_DB_UPDATE_TRACE("\tPort %d doesn't have updateEnabled set\n", portIndex);
            }
        }
    }

    IX_ETH_DB_UPDATE_TRACE("DB: (Events) Updating port set 0x%x\n", updatePorts);

	ixEthDBCreateTrees(updatePorts);
}

/**
 * @brief synchronizes a learning tree pending AccessGrant
 *
 * @param syncInfo synchronization data
 *
 * Port updates can happen without the XScale Ethernet Database to have write access
 * to an NPE learning tree. In this case, after builing a tree the XScale will request write
 * access to the NPE tree using X2P_ELT_AccessRequest. While waiting for a response 
 * other learning events can happen, which can render the trees pending access grant
 * stale. To prevent this from happening every learning event builds a list of new ports
 * and addresses which is passed to this function to update the existent (pending) learning trees.
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBLearningTreeSync(TreeSyncInfo *syncInfo)
{
    UINT32 portIndex;

    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        PortInfo *port = &ixEthDBPortInfo[portIndex];

        if (port->updateMethod.searchTreePendingWrite && IS_PORT_INCLUDED(syncInfo->portID, port->dependencyPortMap))
        {
            /* tree pending AccessGrant, update by adding all syncInfo entries  */
            UINT32 syncIndex;

            for (syncIndex = 0 ; syncIndex < syncInfo->addressCount ; syncIndex++)
            {
                MacDescriptor *syncDescriptor = ixEthDBCloneMacDescriptor(syncInfo->macDescriptors[syncIndex]);

                if (syncDescriptor != NULL)
                {
                    port->updateMethod.searchTree = ixEthDBTreeInsert(port->updateMethod.searchTree, syncDescriptor);
                }
            }

            /* rebalance tree */
            /* NB: this generates an optimal structure for the NPE but it is
               possibly wasteful XScale-wise, so it can be removed if not wanted */
            port->updateMethod.searchTree = ixEthDBTreeRebalance(port->updateMethod.searchTree);
        }
    }
}

/**
 * @brief creates learning trees and calls the port update handlers
 *
 * @param updatePorts set of ports in need of learning trees
 *
 * This function determines the optimal method of creating learning
 * trees using a minimal number of database queries, keeping in mind
 * that different ports can either use the same learning trees or they
 * can partially share them. The actual tree building routine is
 * @ref ixEthDBQuery.
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
void ixEthDBCreateTrees(IxEthDBPortMap updatePorts)
{
    UINT32 portIndex;
    BOOL portsLeft = TRUE;

    while (portsLeft)
    {
        /* get port with minimal dependency map and NULL search tree */
        UINT32 minPortIndex = MAX_PORT_SIZE;
        UINT32 minimalSize  = MAX_PORT_SIZE;

        for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
        {
            UINT32 size;
            PortInfo *port = &ixEthDBPortInfo[portIndex];

            /* generate trees only for ports that need them */
            if (!port->updateMethod.searchTreePendingWrite && IS_PORT_INCLUDED(portIndex, updatePorts))
            {
                GET_MAP_SIZE(port->dependencyPortMap, &size);

                if (size < minimalSize)
                {
                    minPortIndex = portIndex;
                    minimalSize  = size;
                }
            }
        }

        /* if a port was found than minimalSize is not MAX_PORT_SIZE */
        if (minimalSize != MAX_PORT_SIZE)
        {
            /* minPortIndex is the port we seek */
            PortInfo *port = &ixEthDBPortInfo[minPortIndex];

            IxEthDBPortMap query;
            MacTreeNode *baseTree;

            /* now try to find a port with minimal map difference */
            PortInfo *minimalDiffPort = NULL;
            UINT32 minimalDiff        = MAX_PORT_SIZE;

            for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
            {   
                PortInfo *diffPort = &ixEthDBPortInfo[portIndex];

                if (portIndex != minPortIndex
                    && diffPort->updateMethod.searchTree != NULL
                    && IS_MAP_SUBSET(diffPort->dependencyPortMap, port->dependencyPortMap))
                {
                    /* compute size and pick only minimal size difference */
                    UINT32 diffPortSize;
                    UINT32 sizeDifference;

                    GET_MAP_SIZE(diffPort->dependencyPortMap, &diffPortSize);

                    sizeDifference = minimalSize - diffPortSize;

                    if (sizeDifference < minimalDiff)
                    {
                        minimalDiffPort = diffPort;
                        minimalDiff     = sizeDifference;
                    }
                }
            }

            /* if minimalDiff is not MAX_PORT_SIZE minimalDiffPort points to the most similar port */
            if (minimalDiff != MAX_PORT_SIZE)
            {
                baseTree = ixEthDBCloneMacTreeNode(minimalDiffPort->updateMethod.searchTree);
                query    = DIFF_MAPS(port->dependencyPortMap , minimalDiffPort->dependencyPortMap);
            }
            else /* .. otherwise no similar port was found, build tree from scratch */
            {
                baseTree = NULL;
                query    = port->dependencyPortMap;
            }

            if (query != EMPTY_DEPENDENCY_MAP) /* otherwise we don't need anything more on top of the cloned tree */
            {
                /* build learning tree */
                port->updateMethod.searchTree = ixEthDBQuery(baseTree, query);

                /* balance tree */
                port->updateMethod.searchTree = ixEthDBTreeRebalance(port->updateMethod.searchTree);
            }

            /* mark tree as valid */
            port->updateMethod.searchTreePendingWrite = TRUE;
        }
        else
        {
            portsLeft = FALSE;
        }
    }

    /* upload search trees to ports */
    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        PortInfo *updatePort = &ixEthDBPortInfo[portIndex];

        if (updatePort->updateMethod.searchTreePendingWrite)
        {
            IX_ETH_DB_UPDATE_TRACE("DB: (PortUpdate) Starting procedure to upload new search tree (%snull) into NPE %d\n", 
                updatePort->updateMethod.searchTree != NULL ? "not " : "",
                portIndex);

            updatePort->updateMethod.updateHandler(portIndex);
        }
    }
}

/**
 * @brief standard NPE update handler
 *
 * @param portID id of the port to be updated
 *
 * The NPE update handler manages write access to the NPE learning tree
 * and, when write access is granted, it serializes a learning tree from
 * the internal format into NPE format.
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBNPEUpdateHandler(IxEthDBPortId portID)
{
    PortInfo *port = &ixEthDBPortInfo[portID];
    IxNpeMhMessage message;

    if (port->updateMethod.treeWriteAccess)
    {
        UINT32 eltSize;

        /* serialize tree into memory */
        eltSize = ixEthDBNPETreeWrite(port->updateMethod.npeUpdateZone, port->updateMethod.searchTree, !port->updateMethod.treeInitialized);

        /* free internal copy */
        if (port->updateMethod.searchTree != NULL)
        {
            ixEthDBFreeMacTreeNode(port->updateMethod.searchTree);
        }

        /* forget last used search tree */
        port->updateMethod.searchTree             = NULL;
        port->updateMethod.searchTreePendingWrite = FALSE;

        /* lock write access semaphore */
        port->updateMethod.treeWriteAccess = FALSE;

        /* release access to NPE using X2P_ELT_AccessRelease */
        FILL_ELT_ACCESS_MESSAGE(message, IX_ETHNPE_X2P_ELT_ACCESSRELEASE, eltSize, port->updateMethod.npeUpdateZone);

        IX_ETH_DB_UPDATE_TRACE("DB: (PortUpdate) Releasing access to search tree NPE %d using 0x%08X:0x%08X\n", portID, message.data[0], message.data[1]);

        if (ixNpeMhMessageSend(IX_ETH_DB_PORT_ID_TO_NPE(portID), message, IX_NPEMH_SEND_RETRIES_DEFAULT) != IX_SUCCESS)
        {
            printf("Ethernet DB: Failed to send message, assuming the FIFO write we'll retry, however deadlock is possible\n");

            /* we assume that AccessRelease has failed, therefore we still have write access
             * so hopefully the next tree update cycle should ensure resynchronization */
            port->updateMethod.treeWriteAccess = TRUE;
        }
	else
	{
	    accessReleaseCounter++;
	}
    }
    else if (!port->updateMethod.accessRequestInProgress)
    {
        ixEthDBNPEAccessRequest(portID);
    }
    else
    {
        IX_ETH_DB_UPDATE_TRACE("DB: (PortUpdate) Access request already in progress for port %d, waiting for access grant\n", portID);
    }
}

IX_ETH_DB_PUBLIC
void ixEthDBNPEAccessRequest(IxEthDBPortId portID)
{
    PortInfo *port = &ixEthDBPortInfo[portID];
    IxNpeMhMessage message;

    /* indicate access request in progress */
    port->updateMethod.accessRequestInProgress = TRUE;

    /* request access using X2P_ELT_AccessRequest */
    FILL_ELT_ACCESS_MESSAGE(message, IX_ETHNPE_X2P_ELT_ACCESSREQUEST, 0, NULL);

    IX_ETH_DB_UPDATE_TRACE("DB: (PortUpdate) Requesting access to search tree NPE %d\n", portID);

    ixNpeMhMessageSend(IX_ETH_DB_PORT_ID_TO_NPE(portID), message, IX_NPEMH_SEND_RETRIES_DEFAULT);
}

/**
 * @brief queries the database for a set of records to be inserted into a given tree
 *
 * @param searchTree pointer to a tree where insertions will be performed; can be NULL
 * @param query set of ports that a database record must match to be inserted into the tree
 *
 * The query method browses through the database, extracts all the descriptors matching
 * the given query parameter and inserts them into the given learning tree.
 * Note that this is an append procedure, the given tree needs not to be empty.
 * A "descriptor matching the query" is a descriptor whose port id is in the query map.
 * If the given tree is empty (NULL) a new tree is created and returned.
 * 
 * @return the tree root
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
MacTreeNode* ixEthDBQuery(MacTreeNode *searchTree, IxEthDBPortMap query)
{
    HashIterator iterator;
    UINT32 entryCount = 0;

	/* browse database */
	BUSY_RETRY(ixEthDBInitHashIterator(&dbHashtable, &iterator));

	while (IS_ITERATOR_VALID(&iterator))
	{
        MacDescriptor *descriptor = (MacDescriptor *) iterator.node->data;

        IX_ETH_DB_UPDATE_TRACE("DB: (PortUpdate) querying [%s]:%d on port map 0x%x ... ", 
            mac2string(descriptor->macAddress), 
            descriptor->portID,
            query);

		if (IS_PORT_INCLUDED(descriptor->portID, query))
		{
            MacDescriptor *descriptorClone = ixEthDBCloneMacDescriptor(descriptor);

            IX_ETH_DB_UPDATE_TRACE("match\n");

            if (descriptorClone != NULL)
            {
                /* add descriptor to tree */
                searchTree = ixEthDBTreeInsert(searchTree, descriptorClone);

                entryCount++;
            }
		}
        else
        {
            IX_ETH_DB_UPDATE_TRACE("no match\n");
        }

        if (entryCount < MAX_ELT_SIZE)
        {
            /* advance to the next record */
	        BUSY_RETRY(ixEthDBIncrementHashIterator(&dbHashtable, &iterator));
        }
        else
        {
            /* the NPE won't accept more entries so we can stop now */
            ixEthDBReleaseHashIterator(&iterator);

            IX_ETH_DB_UPDATE_TRACE("DB: (PortUpdate) number of elements reached maximum supported by port\n");

            break;
        }
	}

    IX_ETH_DB_UPDATE_TRACE("DB: (PortUpdate) query inserted %d records in the search tree\n", entryCount);

    return searchTree;
}

/**
 * @brief inserts a mac descriptor into an tree
 *
 * @param searchTree tree where the insertion is to be performed (may be NULL)
 * @param descriptor descriptor to insert into tree
 *
 * @return the tree root
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
MacTreeNode* ixEthDBTreeInsert(MacTreeNode *searchTree, MacDescriptor *descriptor)
{
    MacTreeNode *currentNode    = searchTree;
    MacTreeNode *insertLocation = NULL;
    MacTreeNode *newNode;
    UINT32 insertPosition;

    if (descriptor == NULL)
    {
        return searchTree;
    }

    /* create a new node */
    newNode = ixEthDBAllocMacTreeNode();

    if (newNode == NULL)
    {
        /* out of memory */
        ERROR_LOG("Warning: ixEthDBAllocMacTreeNode returned NULL in file %s:%d (out of memory?)\n", __FILE__, __LINE__);

        ixEthDBFreeMacDescriptor(descriptor);

        return NULL;
    }

    /* populate node */
    newNode->descriptor = descriptor;

    /* an empty initial tree is a special case */
    if (searchTree == NULL)
    {
        return newNode;
    }

    /* get insertion location */
    while (insertLocation == NULL)
    {
        MacTreeNode *nextNode;

        /* compare given key with current node key */
        insertPosition = ixEthDBAddressCompare(descriptor->macAddress, currentNode->descriptor->macAddress);

        /* navigate down */
        if (insertPosition == RIGHT)
        {
            nextNode = currentNode->right;
        }
        else if (insertPosition == LEFT)
        {
            nextNode = currentNode->left;
        }
        else
        {
            /* error, duplicate key */
            ERROR_LOG("Warning: trapped insertion of a duplicate MAC address in an NPE search tree\n");

            /* this will free the MAC descriptor as well */
            ixEthDBFreeMacTreeNode(newNode);

            return searchTree;
        }

        /* when we can no longer dive through the tree we found the insertion place */
        if (nextNode != NULL)
        {
            currentNode = nextNode;
        }
        else
        {
            insertLocation = currentNode;
        }
    }

    /* insert node */
    if (insertPosition == RIGHT)
    {
        insertLocation->right = newNode;
    }
    else
    {
        insertLocation->left = newNode;
    }

    return searchTree;
}

/**
 * @brief balance a tree
 *
 * @param searchTree tree to balance
 *
 * Converts a tree into a balanced tree and returns the root of
 * the balanced tree. The resulting tree is <i>route balanced</i>
 * not <i>perfectly balanced</i>. This makes no difference to the
 * average tree search time which is the same in both cases, O(log2(n)).
 *
 * @return root of the balanced tree or NULL if there's no memory left
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
MacTreeNode* ixEthDBTreeRebalance(MacTreeNode *searchTree)
{
    MacTreeNode *pseudoRoot = ixEthDBAllocMacTreeNode();
    UINT32 size;

    if (pseudoRoot == NULL)
    {
        /* out of memory */
        return NULL;
    }

    pseudoRoot->right = searchTree;

    ixEthDBRebalanceTreeToVine(pseudoRoot, &size);
    ixEthDBRebalanceVineToTree(pseudoRoot, size);

    searchTree = pseudoRoot->right;

    /* remove pseudoRoot right branch, otherwise it will free the entire tree */
    pseudoRoot->right = NULL;

    ixEthDBFreeMacTreeNode(pseudoRoot);

    return searchTree;
}

/**
 * @brief converts a tree into a vine
 *
 * @param root root of tree to convert
 * @param size depth of vine (equal to the number of nodes in the tree)
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
void ixEthDBRebalanceTreeToVine(MacTreeNode *root, UINT32 *size)
{
    MacTreeNode *vineTail  = root;
    MacTreeNode *remainder = vineTail->right;
    MacTreeNode *tempPtr;

    *size = 0;

    while (remainder != NULL)
    {
        if (remainder->left == NULL)
        {
            /* move tail down one */
            vineTail  = remainder;
            remainder = remainder->right;
            (*size)++;
        }
        else
        {
            /* rotate around remainder */
            tempPtr         = remainder->left;
            remainder->left = tempPtr->right;
            tempPtr->right  = remainder;
            remainder       = tempPtr;
            vineTail->right = tempPtr;
        }
    }
}

/**
 * @brief converts a vine into a balanced tree
 *
 * @param root vine to convert
 * @param size depth of vine
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
void ixEthDBRebalanceVineToTree(MacTreeNode *root, UINT32 size)
{
    UINT32 leafCount = size + 1 - (1 << ixEthDBRebalanceLog2Floor(size + 1));

    ixEthDBRebalanceCompression(root, leafCount);

    size = size - leafCount;

    while (size > 1)
    {
        ixEthDBRebalanceCompression(root, size / 2);

        size /= 2;
    }
}

/**
 * @brief compresses a vine/tree stage into a more balanced vine/tree
 *
 * @param root root of the tree to compress
 * @param count number of "spine" nodes
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
void ixEthDBRebalanceCompression(MacTreeNode *root, UINT32 count)
{
    MacTreeNode *scanner = root;
    MacTreeNode *child;
    UINT32 index;

    for (index = 0 ; index < count ; index++)
    {
        child          = scanner->right;
        scanner->right = child->right;
        scanner        = scanner->right;
        child->right   = scanner->left;
        scanner->left  = child;
    }
}

/**
 * @brief computes |_log2(x)_| (a.k.a. floor(log2(x)))
 *
 * @param x number to compute |_log2(x)_| for
 *
 * @return |_log2(x)_|
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
UINT32 ixEthDBRebalanceLog2Floor(UINT32 x)
{
    UINT32 log = 0;
    UINT32 val = 1;

    while (val < x)
    {
        log++;
        val <<= 1;
    }

    return val == x ? log : log - 1;
}

