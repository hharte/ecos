/**
 * @file IxEthDB_p.h
 *
 * @brief Private MAC learning API
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

#ifndef IxEthDB_p_H
#define IxEthDB_p_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __ECOS
#ifndef printf
#define printf diag_printf
#endif
#endif

#include <IxTypes.h>
#include <IxOsServices.h>
#include <IxEthDB.h>
#include <IxNpeMh.h>
#include <IxEthDBPortDefs.h>

#include "IxEthDBMessages_p.h"
#include "IxEthDBLog_p.h"

/* undef to turn off the learning/filtering database (the relevant code will be excluded from compilation) */
#ifdef __ECOS
#undef HAVE_ETH_LEARNING
#else
#define HAVE_ETH_LEARNING
#endif

#ifdef TEST_ENV

/* when running unit tests intLock() won't protect the event queue so we lock it manually */
#define TEST_FIXTURE_LOCK_EVENT_QUEUE   { ixOsServMutexLock(&eventQueueLock); }
#define TEST_FIXTURE_UNLOCK_EVENT_QUEUE { ixOsServMutexUnlock(&eventQueueLock); }

#else

#define TEST_FIXTURE_LOCK_EVENT_QUEUE   /* nothing */
#define TEST_FIXTURE_UNLOCK_EVENT_QUEUE /* nothing */

#endif /* TEST_ENV */

#if defined(NDEBUG) || defined(NO_ELT_DEBUG)

#define TEST_FIXTURE_INCREMENT_DB_CORE_ACCESS_COUNTER /* nothing */
#define TEST_FIXTURE_MARK_OVERFLOW_EVENT              /* nothing */

#else

extern int dbAccessCounter;
extern int overflowEvent;

#define TEST_FIXTURE_INCREMENT_DB_CORE_ACCESS_COUNTER { dbAccessCounter++; }
#define TEST_FIXTURE_MARK_OVERFLOW_EVENT              { overflowEvent = 1; }

#endif 

/* code readability markers */
#define __mempool__      /* memory pool marker */
#define __lock__         /* hash write locking marker */
#define __smartpointer__ /* smart pointer marker - warning: use only clone() when duplicating! */

/* error messaging for BUSY_RETRY */
#define ERROR_LOG printf

/* constants */

/** 
 * number of hash table buckets
 * it should be at least 8x the predicted number of entries for performance
 * each bucket needs 8 bytes 
 */
#define NUM_BUCKETS (8192)

/* locks */
#define MAX_LOCKS (100) /**< maximum number of locks used simultaneously, do not tamper with */

/* learning tree constants */
#define INITIAL_ELT_SIZE   (8)                              /**< initial byte size of tree (empty unused root size) */
#ifdef __ECOS
#define MAX_ELT_SIZE       (512)                            /**< maximum number of entries (includes unused root) */
#else
#define MAX_ELT_SIZE       (512)                            /**< maximum number of entries (includes unused root) */
#endif
#define ELT_ENTRY_SIZE     (8)                              /**< entry size, in bytes */
#define ELT_ROOT_OFFSET    (ELT_ENTRY_SIZE)                 /**< tree root offset, in bytes - node preceeding root is unused */
#define FULL_ELT_BYTE_SIZE (MAX_ELT_SIZE * ELT_ENTRY_SIZE)  /**< full size of tree, in bytes, including unused root */

/* Update zone definitions */
#define NPE_TREE_MEM_SIZE (4096) /* ((511 entries + 1 unused root) * 8 bytes/entry) */

/* check the above value, we rely on 4k */
#if NPE_TREE_MEM_SIZE != 4096
    #error NPE_TREE_MEM_SIZE is not defined to 4096 bytes!
#endif

/* memory management pool sizes  */

/* 
 * Note:
 *
 * NODE_POOL_SIZE controls the maximum number of elements in the database at any one time.
 * It should be large enough to cover all the search trees of all the ports simultaneously.
 * 
 * MAC_POOL_SIZE should be higher than NODE_POOL_SIZE by at least the total number of MAC addresses
 * possible to be held at any time in all the ports.
 *
 * TREE_POOL_SIZE should follow the same guideline as for MAC_POOL_SIZE.
 *
 * The database structure described here (2000/4000/4000) is enough for two NPEs holding at most 511 
 * entries each plus one PCI NIC holding at most 900 entries.
 */

#define NODE_POOL_SIZE (2000) /**< number of HashNode objects - also master number of elements in the database; each entry has 16 bytes */
#define MAC_POOL_SIZE  (4000) /**< number of MacDescriptor objects; each entry has 28 bytes */
#define TREE_POOL_SIZE (4000) /**< number of MacTreeNode objects; each entry has 16 bytes */

/* retry policies */
#define BUSY_RETRY_ENABLED (TRUE)  /**< if set to TRUE the API will retry automatically calls returning BUSY */
#define FOREVER_RETRY      (TRUE)  /**< if set to TRUE the API will retry forever BUSY calls */
#define MAX_RETRIES        (400)   /**< upper retry limit - used only when FOREVER_RETRY is FALSE */
#define BUSY_RETRY_YIELD   (5)     /**< ticks to yield for every failed retry */

/* event management */
#define EVENT_QUEUE_SIZE       (500) /**< size of the sink collecting events from the Message Handler FIFO */
#define EVENT_PROCESSING_LIMIT (100)  /**< batch processing control size (how many events are extracted from the queue at once) */

/* MAC descriptors */
#define STATIC_ENTRY  (TRUE)
#define DYNAMIC_ENTRY (FALSE)

/* dependency maps */
#define EMPTY_DEPENDENCY_MAP (0)

/* trees */
#define RIGHT (1)
#define LEFT  (-1)

/* macros */
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* busy retrying */
#define BUSY_RETRY(functionCall) \
    { \
        UINT32 retries = 0; \
        IxEthDBStatus result; \
        \
        while ((result = functionCall) == IX_ETH_DB_BUSY \
            && BUSY_RETRY_ENABLED && (FOREVER_RETRY || ++retries < MAX_RETRIES)) { ixOsServTaskSleep(BUSY_RETRY_YIELD); }; \
        \
        if ((!FOREVER_RETRY && retries == MAX_RETRIES) || (result == IX_ETH_DB_FAIL)) \
        {\
            ERROR_LOG("Ethernet Learning Database Error: BUSY_RETRY failed at %s:%d\n", __FILE__, __LINE__); \
        }\
    }

#define BUSY_RETRY_WITH_RESULT(functionCall, result) \
    { \
        UINT32 retries = 0; \
        \
        while ((result = functionCall) == IX_ETH_DB_BUSY \
            && BUSY_RETRY_ENABLED && (FOREVER_RETRY || ++retries < MAX_RETRIES)) { ixOsServTaskSleep(BUSY_RETRY_YIELD); }; \
        \
        if ((!FOREVER_RETRY && retries == MAX_RETRIES) || (result == IX_ETH_DB_FAIL)) \
        {\
            ERROR_LOG("Ethernet Learning Database Error: BUSY_RETRY_WITH_RESULT failed at %s:%d\n", __FILE__, __LINE__); \
        }\
    }

/* iterators */
#define IS_ITERATOR_VALID(iteratorPtr) ((iteratorPtr)->node != NULL)

/* dependency port maps */

/* Warning: if port indexing starts from 1 replace (portID) with (portID - 1) in DEPENDENCY_MAP (and make sure IX_ETH_DB_NUMBER_OF_PORTS is big enough) */

/**
 * returns a map consisting only of 'portID'
 */
#define DEPENDENCY_MAP(portID)             (1 << (portID))

/**
 * returns the map resulting from joining map1 and map2
 */
#define JOIN_MAPS(map1, map2)              ((map1) | (map2))

/**
 * returns the map resulting from joining portID and map
 */
#define JOIN_PORT_TO_MAP(map, portID)      (JOIN_MAPS((map), DEPENDENCY_MAP(portID)))

/**
 * returns the map resulting from excluding portID from map
 */
#define EXCLUDE_PORT_FROM_MAP(map, portID) ((map) & ~(DEPENDENCY_MAP(portID))

/**
 * returns TRUE if map1 is a subset of map2 and FALSE otherwise
 */
#define IS_MAP_SUBSET(map1, map2)          (((map1) | (map2)) == (map2))

/**
 * returns TRUE is portID is part of map and FALSE otherwise
 */
#define IS_PORT_INCLUDED(portID, map)      ((DEPENDENCY_MAP(portID) & (map)) != 0)

/**
 * returns the difference between map1 and map2 (ports included in map1 and not included in map2)
 */
#define DIFF_MAPS(map1, map2)              ((map1) ^ ((map1) & (map2)))

/**
 * returns the difference between map1 and map2; use only when map2 is included in map1
 */
#define DIFF_SUBMAP(map1, map2)            ((map1) ^ (map2))

/**
 * returns TRUE if the maps collide (have at least one port in common) and FALSE otherwise
 */
#define MAPS_COLLIDE(map1, map2)           (((map1) & (map2)) != 0)

/* size (number of ports) of a dependency map */
#define GET_MAP_SIZE(map, size)   { UINT32 mapCopy = map; *size = 0; while (mapCopy != 0) { *size += mapCopy & 0x1; mapCopy >>= 1; } }

/* definition of a port map size/port number which cannot be reached (we support at most 32 ports) */
#define MAX_PORT_SIZE   (0xFF)
#define MAX_PORT_NUMBER (0xFF)

#define CHECK_REFERENCE(ptr)   { if ((ptr) == NULL) { return IX_ETH_DB_INVALID_ARG; } }
#define CHECK_MAP(portID, map) { if (!IS_PORT_INCLUDED(portID, map)) { return IX_ETH_DB_INVALID_ARG; } }

/* event queue macros */
#define EVENT_QUEUE_WRAP(offset)            ((offset) >= EVENT_QUEUE_SIZE ? (offset) - EVENT_QUEUE_SIZE : (offset))

#define CAN_ENQUEUE(eventQueuePtr)          ((eventQueuePtr)->length < EVENT_QUEUE_SIZE)        

#define QUEUE_HEAD(eventQueuePtr)           (&(eventQueuePtr)->queue[EVENT_QUEUE_WRAP((eventQueuePtr)->base + (eventQueuePtr)->length)])

#define QUEUE_TAIL(eventQueuePtr)           (&(eventQueuePtr)->queue[(eventQueuePtr)->base])

#define PUSH_UPDATE_QUEUE(eventQueuePtr)    { (eventQueuePtr)->length++; }

#define SHIFT_UPDATE_QUEUE(eventQueuePtr) \
        { \
            (eventQueuePtr)->base = EVENT_QUEUE_WRAP((eventQueuePtr)->base + 1); \
            (eventQueuePtr)->length--; \
        }

#define RESET_QUEUE(eventQueuePtr) \
    { \
        (eventQueuePtr)->base   = 0; \
        (eventQueuePtr)->length = 0; \
    }

/* node stack macros - used to browse a tree without using a recursive function */
#define NODE_STACK_INIT(stack)               { (stack).nodeCount = 0; }
#define NODE_STACK_PUSH(stack, node, offset) { (stack).nodes[(stack).nodeCount] = (node); (stack).offsets[(stack).nodeCount++] = (offset); }
#define NODE_STACK_POP(stack, node, offset)  { (node) = (stack).nodes[--(stack).nodeCount]; offset = (stack).offsets[(stack).nodeCount]; }
#define NODE_STACK_NONEMPTY(stack)           ((stack).nodeCount != 0)

/* ----------- Data -------------- */

/* typedefs */

typedef UINT32 (*HashFunction)(void *entity);
typedef BOOL (*MatchFunction)(void *reference, void *entry);
typedef void (*FreeFunction)(void *entry);

/**
 * basic component of a hash table
 */
typedef struct HashNode_t
{
    void *data;                                 /**< specific data */
    struct HashNode_t *next;                    /**< used for bucket chaining */

    __mempool__ struct HashNode_t *nextFree;    /**< memory pool management */

    __lock__ IxFastMutex lock;                  /**< node lock */
} HashNode;

/**
 * @brief hash table iterator definition
 *
 * an iterator is an object which can be used
 * to browse a hash table
 */
typedef struct
{
    UINT32 bucketIndex;     /**< index of the currently iterated bucket */
    HashNode *previousNode; /**< reference to the previously iterated node within the current bucket */
    HashNode *node;         /**< reference to the currently iterated node */
} HashIterator;

/**
 * definition of a MAC descriptor (a database record)
 */
typedef struct MacDescriptor_t
{
    UINT8 macAddress[IX_IEEE803_MAC_ADDRESS_SIZE];
    UINT8 portID;
    UINT8 age;
    BOOL empty, modified;
    BOOL staticEntry;                               /**< TRUE if this address is static (doesn't age) */

    __mempool__ struct MacDescriptor_t *nextFree;   /**< memory pool management */
    __smartpointer__ UINT32 refCount;               /**< smart pointer reference counter */
} MacDescriptor;

/**
 * hash table definition
 */
typedef struct
{
    HashNode *hashBuckets[NUM_BUCKETS];
    UINT32 numBuckets;

    __lock__ IxFastMutex bucketLocks[NUM_BUCKETS];

    HashFunction entryHashFunction;
    HashFunction keyHashFunction;
    MatchFunction matchFunction;
    FreeFunction freeFunction;
} HashTable;

typedef struct MacTreeNode_t
{
    __smartpointer__  MacDescriptor *descriptor;
    struct MacTreeNode_t *left, *right;

    __mempool__ struct MacTreeNode_t *nextFree;
} MacTreeNode;

typedef void (*IxEthDBPortUpdateHandler)(IxEthDBPortId portID);

typedef struct 
{
    BOOL updateEnabled;                         /**< TRUE if updates are enabled for port */
    BOOL userControlled;                        /**< TRUE if the user has manually used ixEthDBPortUpdateEnableSet */
    BOOL treeInitialized;                       /**< TRUE if the NPE has received an initial tree */
    IxEthDBPortUpdateHandler updateHandler;    /**< port update handler routine */
    void *npeUpdateZone;                        /**< port update memory zone */
    MacTreeNode *searchTree;                    /**< internal search tree, in MacTreeNode representation */
    BOOL searchTreePendingWrite;                /**< TRUE if searchTree holds a tree pending write to the port */
    BOOL treeWriteAccess;                       /**< port tree write access semaphore */
    BOOL accessRequestInProgress;               /**< port tree access request semaphore */
    BOOL syncAllowed;                           /**< TRUE if ELT->database back synchronization is allowed */
} PortUpdateMethod;

typedef struct
{
    IxEthDBPortId portID;                  /**< port ID */
    BOOL enabled;                           /**< TRUE if the port is enabled */
    BOOL agingEnabled;                      /**< TRUE if aging on this port is enabled */
    IxEthDBPortMap dependencyPortMap;      /**< dependency port map for this port */
    PortUpdateMethod updateMethod;          /**< update method structure */

    BOOL macAddressUploaded;                /**< TRUE if the MAC address was uploaded into the port */
    __lock__ IxMutex ackPortAddressLock;    /**< lock to block ixEthDBUnicastMacAddressSet until the NPE responds */
} PortInfo;

/* list of port information structures indexed on port Ids */
extern PortInfo ixEthDBPortInfo[IX_ETH_DB_NUMBER_OF_PORTS];

typedef struct
{
    UINT32 eventType;
    IxEthDBPortId portID;
    IxEthDBPortMap portMap;

    /* mac-related events */
    IxEthDBMacAddr macAddress;
    MacDescriptor *macDescriptor;

    /* learning tree events */
    UINT16 eltSize;
    UINT32 eltBaseAddress;
} PortEvent;

typedef struct
{
    PortEvent queue[EVENT_QUEUE_SIZE];
    UINT32 base;
    UINT32 length;
} PortEventQueue;

typedef struct
{
    IxEthDBPortId portID; /**< originating port */
    MacDescriptor *macDescriptors[MAX_ELT_SIZE]; /**< addresses to be synced into db */
    UINT32 addressCount; /**< number of addresses */
} TreeSyncInfo;

typedef struct
{
    MacTreeNode *nodes[MAX_ELT_SIZE];
    UINT32 offsets[MAX_ELT_SIZE];
    UINT32 nodeCount;
} MacTreeNodeStack;

/* Prototypes */

/* ----------- Memory management -------------- */

void ixEthDBInitMemoryPools(void);

HashNode* ixEthDBAllocHashNode(void);
void ixEthDBFreeHashNode(HashNode *);

__smartpointer__ MacDescriptor* ixEthDBAllocMacDescriptor(void);
__smartpointer__ MacDescriptor* ixEthDBCloneMacDescriptor(MacDescriptor *macDescriptor);
__smartpointer__ void ixEthDBFreeMacDescriptor(MacDescriptor *);

__smartpointer__ MacTreeNode* ixEthDBAllocMacTreeNode(void);
__smartpointer__ MacTreeNode* ixEthDBCloneMacTreeNode(MacTreeNode *);
__smartpointer__ void ixEthDBFreeMacTreeNode(MacTreeNode *);

/* Hash Table */
IX_ETH_DB_PUBLIC void ixEthDBInitHash(HashTable *hashTable, UINT32 numBuckets, HashFunction entryHashFunction, HashFunction keyHashFunction, MatchFunction matchFunction, FreeFunction freeFunction);

IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBAddHashEntry(HashTable *hashTable, void *entry);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBRemoveHashEntry(HashTable *hashTable, void *reference);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBSearchHashEntry(HashTable *hashTable, void *reference, HashNode **searchResult);

IX_ETH_DB_PUBLIC void ixEthDBReleaseHashNode(HashNode *node);

IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBInitHashIterator(HashTable *hashTable, HashIterator *iterator);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBIncrementHashIterator(HashTable *hashTable, HashIterator *iterator);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBRemoveEntryAtHashIterator(HashTable *hashTable, HashIterator *iterator);

IX_ETH_DB_PUBLIC void ixEthDBReleaseHashIterator(HashIterator *iterator);

/* API Support */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBFilteringEntryAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddr, UINT8 age, BOOL staticEntry);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBPortAddressSet(IxEthDBPortId portID, IxEthDBMacAddr *macAddr);

/* DB Core functions */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBInit(void);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBAdd(IxEthDBPortId portID, IxEthDBMacAddr *macAddress, UINT8 age, BOOL staticEntry, MacDescriptor **descriptor);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBRemove(IxEthDBMacAddr *macAddress);
IX_ETH_DB_PUBLIC HashNode* ixEthDBSearch(IxEthDBMacAddr *macAddress);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBUpdate(IxEthDBMacAddr *macAddress, IxEthDBPortId portID, UINT32 age, BOOL staticEntry);

/* Learning support */
IX_ETH_DB_PUBLIC UINT32 ixEthDBAddressCompare(UINT8 *mac1, UINT8 *mac2);
IX_ETH_DB_PUBLIC BOOL ixEthDBAddressMatch(void *reference, void *entry);
IX_ETH_DB_PUBLIC UINT32 ixEthDBEntryXORHash(void *macDescriptor);
IX_ETH_DB_PUBLIC UINT32 ixEthDBKeyXORHash(void *macAddress);

/* Port updates */
IX_ETH_DB_PUBLIC void ixEthDBNPEUpdateHandler(IxEthDBPortId portID);
IX_ETH_DB_PUBLIC void ixEthDBUpdatePortLearningTrees(IxEthDBPortMap triggerPorts, IxEthDBPortMap excludePorts);
IX_ETH_DB_PUBLIC void ixEthDBNPEAccessRequest(IxEthDBPortId portID);
IX_ETH_DB_PUBLIC void ixEthDBLearningTreeSync(TreeSyncInfo *syncInfo);

/* Init */
IX_ETH_DB_PUBLIC void ixEthDBPortSetAckCallback(IxNpeMhNpeId npeID, IxNpeMhMessage msg);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBEventProcessorInit(void);
IX_ETH_DB_PUBLIC void ixEthDBPortInit(IxEthDBPortId portID);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBPortEnable(IxEthDBPortId portID);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBPortDisable(IxEthDBPortId portID);
IX_ETH_DB_PUBLIC void ixEthDBNPEUpdateAreasInit(void);

/* Event processing */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBDefaultEventCallbackEnable(IxEthDBPortId portID, BOOL enable);
IX_ETH_DB_PUBLIC void ixEthDBTriggerAddPortUpdate(IxEthDBPortId portID, MacDescriptor *macDescriptor);
IX_ETH_DB_PUBLIC void ixEthDBTriggerRemovePortUpdate(IxEthDBPortMap portMap);
IX_ETH_DB_PUBLIC void ixEthDBTriggerPortSync(IxEthDBPortMap portMap, IxEthDBPortMap excludePorts);
IX_ETH_DB_PUBLIC void ixEthDBNPEEventCallback(IxNpeMhNpeId npeID, IxNpeMhMessage msg);

/* NPE adaptor */
IX_ETH_DB_PUBLIC void ixEthDBNPESyncScan(IxEthDBPortId portID, void *eltBaseAddress, UINT32 eltSize, TreeSyncInfo *syncInfo);
IX_ETH_DB_PUBLIC UINT32 ixEthDBNPETreeWrite(void *eltBaseAddress, MacTreeNode *tree, BOOL initialTree);

/* Other public API functions */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBPortDependencyMapSet(IxEthDBPortId portID, IxEthDBPortMap dependencyPortMap);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBStartLearningFunction(void);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBStopLearningFunction(void);
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBPortUpdateEnableSet(IxEthDBPortId portID, BOOL enableUpdate);

/* Utilities */
const char* mac2string(const unsigned char *mac);
void showHashInfo(void);
const char* errorString(IxEthDBStatus error);
int numHashElements(void);
void zapHashtable(void);

#endif /* IxEthDB_p_H */
