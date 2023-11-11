/**
 * @file IxEthDBDBNPEAdaptor.c
 *
 * @brief Routines that read and write learning/search trees in NPE-specific format
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

#include <IxOsCacheMMU.h>

#include "IxEthDB_p.h"
#include "IxEthDBLog_p.h"

#ifdef __ECOS
#define printf(x,...)
#endif

/* forward prototype declarations */
IX_ETH_DB_PRIVATE void ixEthDBNPENodeWrite(void *eltNodeAddress, MacTreeNode *node);
IX_ETH_DB_PUBLIC void ixEthELTAccessGrantCallback(IxNpeMhNpeId npeID, IxNpeMhMessage msg);
IX_ETH_DB_PUBLIC void ixEthELTDumpTree(void *eltBaseAddress, UINT32 eltSize);

/* data */
UINT8* ixEthDBNPEUpdateArea[IX_ETH_DB_NUMBER_OF_PORTS];
UINT32 eltSize;

/* imported data */
extern int accessGrantCounter, accessReleaseCounter;

#ifdef __ECOS
void *cacheDmaMalloc(unsigned int n)
{
    static unsigned char _zone_buf[(FULL_ELT_BYTE_SIZE * 2) + 32];
    static unsigned char *next_free;
    static unsigned nleft;
    void *p;

    n = (n+31) & ~31; /* round to cacheline boundary */

    if (next_free == NULL) {
	next_free = (unsigned char *)(((unsigned)_zone_buf + 31) & ~31);
	nleft = FULL_ELT_BYTE_SIZE * 2;
    }

    if (nleft < n)
	return NULL;

    p = next_free;
    next_free += n;
    nleft -= n;
    return p;
}
#endif

/**
 * @brief allocates non-cached NPE tree update areas for all the ports
 *
 * This function is called only once at initialization time from
 * @ref ixEthDBInit().
 *
 * @warning do not call manually
 *
 * @see ixEthDBInit()
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBNPEUpdateAreasInit()
{
    UINT32 portIndex;

    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        if (ixEthDBPortDefinitions[portIndex].type == ETH_NPE)
        {
            ixEthDBPortInfo[portIndex].updateMethod.npeUpdateZone = IX_ACC_DRV_DMA_MALLOC(FULL_ELT_BYTE_SIZE);

            if (ixEthDBPortInfo[portIndex].updateMethod.npeUpdateZone == NULL)
            {
                ERROR_LOG("Fatal error: IX_ACC_DRV_DMA_MALLOC() returned NULL, no NPE update zones available\n");
            }
            else
            {
                memset(ixEthDBPortInfo[portIndex].updateMethod.npeUpdateZone, 0, FULL_ELT_BYTE_SIZE);
            }
        }
        else
        {
            /* unused */
            ixEthDBPortInfo[portIndex].updateMethod.npeUpdateZone = NULL;
        }
    }
}

/**
 * @brief synchronizes the database with tree
 *
 * @param portID port ID of the NPE whose tree is to be scanned
 * @param eltBaseAddress memory base address of the NPE serialized tree
 * @param eltSize size in bytes of the NPE serialized tree
 * @param syncInfo pointer to a TreeSyncInfo structure where the synchronization
 * data is to be collected
 *
 * Scans the NPE learning tree, determines what information is not in the
 * database, adds it to the database and places it in a syncInfo structure for
 * additional use (such as updating search trees pending AccessGrant).
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBNPESyncScan(IxEthDBPortId portID, void *eltBaseAddress, UINT32 eltSize, TreeSyncInfo *syncInfo)
{
    UINT32 eltEntryOffset;

    if (ixEthDBPortInfo[portID].updateMethod.syncAllowed)
    {
        /* invalidate cache */
        IX_ACC_DATA_CACHE_INVALIDATE(eltBaseAddress, eltSize * ELT_ENTRY_SIZE);

        for (eltEntryOffset = ELT_ROOT_OFFSET ; eltEntryOffset < eltSize ; eltEntryOffset += ELT_ENTRY_SIZE)
        {
            /* (eltBaseAddress + eltEntryOffset) points to a valid NPE tree node
             *
             * the format of the node is MAC[6 bytes]:PortID[1 byte]:Age[6 bits]:Modified[1 bit]:Empty[1 bit]
             * therefore we can just use the pointer for database searches as only the first 6 bytes are checked
             */
            void *eltNodeAddress       = (void *) ((UINT32) eltBaseAddress + eltEntryOffset);
            void *eltParentNodeAddress = (void *) ((UINT32) eltBaseAddress + ((eltEntryOffset / ELT_ENTRY_SIZE) >> 1) * ELT_ENTRY_SIZE);

            /* debug */
            IX_ETH_DB_NPE_VERBOSE_TRACE("DB: (NPEAdaptor) checking node at offset %d...\n", eltEntryOffset / ELT_ENTRY_SIZE);

            if (NPE_NODE_NONEMPTY(eltNodeAddress) != TRUE)
            {
                IX_ETH_DB_NPE_VERBOSE_TRACE("\t... node is empty\n");
            }
            else if (eltEntryOffset == ELT_ROOT_OFFSET)
            {
                IX_ETH_DB_NPE_VERBOSE_TRACE("\t... node is root\n");
            }
            else if (NPE_NODE_NONEMPTY(eltParentNodeAddress) != TRUE)
            {
                IX_ETH_DB_NPE_VERBOSE_TRACE("\t.. parent node is empty\n");
            }

            /* an entry is valid only if its parent is valid; this check is compulsory as
             *  the NPE leaves invalid entries (with the non-Empty bit set) in the tree */
            if (NPE_NODE_NONEMPTY(eltNodeAddress) 
                && (eltEntryOffset == ELT_ROOT_OFFSET || NPE_NODE_NONEMPTY(eltParentNodeAddress)))
            {
                /* the first 6 bytes in the linear node are the MAC address */
                IxEthDBMacAddr *macAddress = (IxEthDBMacAddr *) eltNodeAddress;
                UINT8 age                   = NPE_NODE_AGE(eltNodeAddress);

                /* check only entries belonging to this port */
                if (portID == NPE_NODE_PORT_ID(eltNodeAddress))
                {
                    /* search record */
                    HashNode *node = ixEthDBSearch((IxEthDBMacAddr *) eltNodeAddress);

                    if (node != NULL)
                    {
                        MacDescriptor *descriptor = (MacDescriptor *) node->data;

                        IX_ETH_DB_NPE_VERBOSE_TRACE("DB: (NPEAdaptor) synced entry [%s] already in the database, updating fields\n", mac2string(macAddress->macAddress));

                        /* already in database, update age unless this is a static entry */
                        if (!descriptor->staticEntry)
                        {
                            descriptor->age = age;
                        }

                        /* update portID in case we have to switch ports */
                        descriptor->portID = portID;

                        ixEthDBReleaseHashNode(node);
                    }
                    else
                    {
                        /* add a new entry only if learnt on this port */
                        MacDescriptor *descriptor = NULL;

                        IX_ETH_DB_NPE_VERBOSE_TRACE("DB: (NPEAdaptor) synced entry [%s] not in the database, adding\n", mac2string(macAddress->macAddress));

                        /* not found, add to database and incremental tree update info */
                        if (ixEthDBAdd(portID, macAddress, age, DYNAMIC_ENTRY, &descriptor) == IX_ETH_DB_SUCCESS)
                        {
                            /* make sure there are available slots in the syncInfo structure */
                            if (syncInfo->addressCount < MAX_ELT_SIZE)
                            {
                                syncInfo->macDescriptors[syncInfo->addressCount] = descriptor;

                                syncInfo->addressCount++;
                            }
                            else
                            {
                                ixEthDBFreeMacDescriptor(descriptor);
                            }
                        }
                    }
                }
                else
                {
                    IX_ETH_DB_NPE_VERBOSE_TRACE("\t... found portID %d, we check only port %d\n", NPE_NODE_PORT_ID(eltNodeAddress), portID);
                }
            }
        }
    }
    else
    {
        /* sync-back was not allowed this time (some records were deleted), enable next time */
        ixEthDBPortInfo[portID].updateMethod.syncAllowed = TRUE;
    }
}

/**
 * @brief writes a search tree in NPE format
 *
 * @param eltBaseAddress memory base address where to write the NPE tree into
 * @param tree search tree to write in NPE format
 * @param initialTree TRUE if this is the initial tree we release access into
 *
 * Serializes the given tree in NPE linear format. Each record is 8 bytes long
 * and contains the port ID (1 byte), the MAC address (6 bytes) and flags (1 byte).
 * The serialized tree uses an extra pseudo-root at offset 0 which is not populated,
 * the actual tree root is the next record.
 *
 * @return the size in bytes of the complete linear NPE tree
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
UINT32 ixEthDBNPETreeWrite(void *eltBaseAddress, MacTreeNode *tree, BOOL initialTree)
{
    MacTreeNodeStack stack;
    UINT32 maxOffset;
    UINT32 emptyOffset;
    
    if (!initialTree)
    {
        maxOffset = 1;

        NODE_STACK_INIT(stack);

        if (tree != NULL)
        {
            /* push tree root at offset 1 */
            NODE_STACK_PUSH(stack, tree, 1);
        }

        while (NODE_STACK_NONEMPTY(stack))
        {
            MacTreeNode *node;
            UINT32 offset;
        
            NODE_STACK_POP(stack, node, offset);

            /* update maximum offset */
            if (offset > maxOffset)
            {
                maxOffset = offset;
            }

            IX_ETH_DB_NPE_VERBOSE_TRACE("DB: (NPEAdaptor) writing MAC [%s] at offset %d\n", mac2string(node->descriptor->macAddress), offset);

            /* add node to NPE ELT at position indicated by offset */
            if (offset < MAX_ELT_SIZE)
            {
                ixEthDBNPENodeWrite((void *) (((UINT32) eltBaseAddress) + offset * ELT_ENTRY_SIZE), node);
            }

            if (node->left != NULL)
            {
                NODE_STACK_PUSH(stack, node->left, LEFT_CHILD_OFFSET(offset));
            }
        
            if (node->right != NULL)
            {
                NODE_STACK_PUSH(stack, node->right, RIGHT_CHILD_OFFSET(offset));
            }
        }

        /* zero out rest of the tree */
        emptyOffset = (tree != NULL) ? (maxOffset + 1) : 0;

        if (emptyOffset < MAX_ELT_SIZE - 1)
        {
            memset((void *) (((UINT32) eltBaseAddress) + (emptyOffset * ELT_ENTRY_SIZE)), 0, (MAX_ELT_SIZE - emptyOffset) * ELT_ENTRY_SIZE);
        }

        IX_ETH_DB_NPE_TRACE("DB: (NPEAdaptor) Emptying tree from offset %d, address 0x%08X\n", emptyOffset, ((UINT32) eltBaseAddress) + (maxOffset + 1) * ELT_ENTRY_SIZE);
    }
    else
    {
        /* just AccessRelease into an empty tree having 8 entries/64 bytes - why 64? the NPE software wants that - tm */
        maxOffset = INITIAL_ELT_SIZE;
    }

    /* flush cache - use MAX_ELT_SIZE instead of eltSize to be safe */
    IX_ACC_DATA_CACHE_FLUSH(eltBaseAddress, MAX_ELT_SIZE * ELT_ENTRY_SIZE);

    /* debug */
    IX_ETH_DB_NPE_TRACE("DB: (NPEAdaptor) Ethernet learning/filtering tree XScale wrote at address 0x%08X (scanned %d bytes):\n\n",
        (UINT32) eltBaseAddress, FULL_ELT_BYTE_SIZE);

    IX_ETH_DB_NPE_DUMP_ELT(eltBaseAddress, FULL_ELT_BYTE_SIZE);

    /* return size (in bytes) of the modified section of the tree - magic formula dictated by NPE software also */
    return MIN((maxOffset + 1) * ELT_ENTRY_SIZE * 2, FULL_ELT_BYTE_SIZE);
}

/**
 * @brief writes an NPE search tree node in linear format
 *
 * @param eltNodeAddress memory address to write node to
 * @param node node to be written
 *
 * Used by @ref ixEthDBNPETreeWrite to liniarize a search tree
 * in NPE-readable format.
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
void ixEthDBNPENodeWrite(void *eltNodeAddress, MacTreeNode *node)
{
    /* copy mac address */
    memcpy(eltNodeAddress, node->descriptor->macAddress, IX_IEEE803_MAC_ADDRESS_SIZE);

    /* copy port ID */
    NPE_NODE_BYTE(eltNodeAddress, NPE_NODE_PORT_ID_OFFSET) = node->descriptor->portID;

    /* copy age, set to non-modified and non-empty */
    NPE_NODE_BYTE(eltNodeAddress, NPE_NODE_AGE_M_E_OFFSET) = (node->descriptor->age << NPE_NODE_AGE_SHIFT) | NPE_NODE_NON_MODIFIED_FLAG | NPE_NODE_NON_EMPTY_FLAG;
}

/**
 * @brief displays the search tree of a port
 */
IX_ETH_DB_PUBLIC
IxEthDBStatus ixEthELTShow(IxEthDBPortId portID)
{
    PortInfo *port;
    IxMutex *lock;
    IxNpeMhMessage message;

    IX_ETH_DB_CHECK_PORT(portID);

    port = &ixEthDBPortInfo[portID];
    lock = &port->ackPortAddressLock;

    if (ixEthDBPortDefinitions[portID].type != ETH_NPE)
    {
        printf("This function can be used only for Ethernet NPEs.\n");

        return IX_ETH_DB_FAIL;
    }

    if (port->updateMethod.accessRequestInProgress)
    {
        printf("\nAccess request on port [%d] is already in progress, try later [EBUSY]\n\n", portID);

        return IX_ETH_DB_FAIL;
    }

    /* diconnect normal processing callback */
    ixEthDBDefaultEventCallbackEnable(portID, FALSE);

    /*
     * get access to the NPE tree if needed
     */

    if (!port->updateMethod.treeWriteAccess)
    {
        /* indicate access request in progress */
        port->updateMethod.accessRequestInProgress = TRUE;

        /* request access using X2P_ELT_AccessRequest */
        FILL_ELT_ACCESS_MESSAGE(message, IX_ETHNPE_X2P_ELT_ACCESSREQUEST, 0, NULL);

#ifndef __ECOS
        printf("DB: (NPEAdaptor) Requesting access to search tree NPE %d\n", portID);
#endif

        ixNpeMhMessageWithResponseSend(IX_ETH_DB_PORT_ID_TO_NPE(portID), message, IX_ETHNPE_P2X_ELT_ACCESSGRANT,
                                       ixEthELTAccessGrantCallback, IX_NPEMH_SEND_RETRIES_DEFAULT);

        /* block until write access arrives */
        ixOsServMutexLock(lock);
    }

    /* 
     * scan search tree and display records 
     */
    
#ifndef __ECOS
    printf("Ethernet learning/filtering tree read from NPE %s at address 0x%08X (scanned %d bytes):\n\n", 
        portID == 0 ? "B" : portID == 1 ? "C" : "? (invalid)", (UINT32) port->updateMethod.npeUpdateZone, eltSize);
#endif

    /* ixEthELTAccessGrantCallback sets eltSize */
    ixEthELTDumpTree(port->updateMethod.npeUpdateZone, eltSize);

    /*
     * release access to the tree 
     */

    port->updateMethod.treeWriteAccess = FALSE;

    /* release access to NPE using X2P_ELT_AccessRelease */
    RESET_ELT_MESSAGE(message);
    FILL_ELT_ACCESS_MESSAGE(message, IX_ETHNPE_X2P_ELT_ACCESSRELEASE, FULL_ELT_BYTE_SIZE, port->updateMethod.npeUpdateZone);

    IX_ETH_DB_NPE_TRACE("DB: (NPEAdaptor) Releasing access to search tree NPE %d using 0x%08X:0x%08X\n", portID, message.data[0], message.data[1]);

    ixNpeMhMessageSend(IX_ETH_DB_PORT_ID_TO_NPE(portID), message, IX_NPEMH_SEND_RETRIES_DEFAULT);

    /* reconnect normal processing callback */
    ixEthDBDefaultEventCallbackEnable(portID, TRUE);

    return IX_ETH_DB_SUCCESS;
}

IX_ETH_DB_PUBLIC
void ixEthELTAccessGrantCallback(IxNpeMhNpeId npeID, IxNpeMhMessage msg)
{
    IxEthDBPortId portID = IX_ETH_DB_NPE_TO_PORT_ID(npeID);
    PortInfo *portInfo    = &ixEthDBPortInfo[portID];

    /* extract eltSize */
    eltSize = NPE_MSG_ELT_SIZE(msg);

    IX_ETH_DB_NPE_IRQ_TRACE("Received AccessGrant from port %d - eltSize is %d\n", portID, eltSize, 0, 0, 0, 0);

    /* unblock pending ixEthELTShow */
    ixOsServMutexUnlock(&portInfo->ackPortAddressLock);

    /* indicate write access */
    portInfo->updateMethod.accessRequestInProgress = FALSE;
    portInfo->updateMethod.treeWriteAccess         = TRUE;
}

IX_ETH_DB_PUBLIC
void ixEthELTDumpTree(void *eltBaseAddress, UINT32 eltSize)
{
    UINT32 eltEntryOffset;
    IxEthDBPortId portID;
    UINT32 recordCount = 0;
    
    /* guess portID */
    for (portID = 0 ; portID < IX_ETH_DB_NUMBER_OF_PORTS ; portID++)
    {
        if (ixEthDBPortInfo[portID].updateMethod.npeUpdateZone == eltBaseAddress)
        {
            break;
        }
    }

    /* invalidate cache */
    IX_ACC_DATA_CACHE_INVALIDATE(eltBaseAddress, MAX_ELT_SIZE);

    /* display table header */
    printf("ELT search tree records for port ID [%d%s]\n\n", portID, portID == IX_ETH_DB_NUMBER_OF_PORTS ? "(invalid)" : "");
    printf("    MAC address    |   Age  | Flags \n");
    printf("____________________________________\n");    

    /* ixEthELTAccessGrantCallback sets eltSize */
    for (eltEntryOffset = ELT_ROOT_OFFSET ; eltEntryOffset < eltSize ; eltEntryOffset += ELT_ENTRY_SIZE)
    {
        /* (eltBaseAddress + eltEntryOffset) points to a valid NPE tree node
         *
         * the format of the node is MAC[6 bytes]:PortID[1 byte]:Age[6 bits]:Modified[1 bit]:Empty[1 bit]
         */
        void *eltNodeAddress       = (void *)((UINT32) eltBaseAddress + eltEntryOffset);
        void *eltParentNodeAddress = (void *) ((UINT32) eltBaseAddress + ((eltEntryOffset / ELT_ROOT_OFFSET) >> 1) * ELT_ROOT_OFFSET);

        if (NPE_NODE_NONEMPTY(eltNodeAddress) 
            && (eltEntryOffset == ELT_ROOT_OFFSET || NPE_NODE_NONEMPTY(eltParentNodeAddress)))
        {
            IxEthDBMacAddr *macAddress = (IxEthDBMacAddr *) eltNodeAddress;
            UINT8 age                   = NPE_NODE_AGE(eltNodeAddress);
            BOOL insideDatabase;

            /* search record */
            HashNode *node = ixEthDBSearch((IxEthDBMacAddr *) eltNodeAddress);

            if (node != NULL)
            {
                insideDatabase = TRUE;

                ixEthDBReleaseHashNode(node);
            }
            else
            {
                insideDatabase = FALSE;
            }

            /* display node */
            printf(" %02X:%02X:%02X:%02X:%02X:%02X | %5d  | %s\n",
                macAddress->macAddress[0],
                macAddress->macAddress[1],
                macAddress->macAddress[2],
                macAddress->macAddress[3],
                macAddress->macAddress[4],
                macAddress->macAddress[5],
                age,
                insideDatabase ? "sync" : "unknown");

            recordCount++;
        }
    }

    /* display number of records */
    printf("\n%d valid records found\n", recordCount);
}

void
ixEthELTAccessToggle(IxEthDBPortId portID)
{
    PortInfo *portInfo;

    if (portID < 0 || portID > IX_ETH_DB_NUMBER_OF_PORTS)
    {
        return;
    }

    portInfo = &ixEthDBPortInfo[portID];

    portInfo->updateMethod.treeWriteAccess         = TRUE;
    portInfo->updateMethod.accessRequestInProgress = FALSE;
}

void
ixEthDBShowAccessStats()
{
    IxEthDBPortId portIndex;
    
    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        PortInfo *portInfo = &ixEthDBPortInfo[portIndex];
	
	    printf("Port [%d]\n", portIndex);
	    
	    printf("\ttreeWriteAccess:         %s\n", portInfo->updateMethod.treeWriteAccess ? "TRUE" : "FALSE");
	    printf("\taccessRequestInProgress: %s\n", portInfo->updateMethod.accessRequestInProgress ? "TRUE" : "FALSE");
    }
    
    printf("\nGlobal access counters:\n\taccess grants from NPE(s): %d\n\taccess releases from XScale:%d\n\n", accessGrantCounter, accessReleaseCounter);
}
