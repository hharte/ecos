/**
 * @file ethUtil.c
 *
 * @brief Utility functions
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
#include <string.h>

#ifdef __ECOS
#ifndef printf
#define printf diag_printf
#endif
#endif

#include "IxEthDB_p.h"

#define MT_ROTATE_COUNT (1000)

/* hack for multithreded testing - can't just use one static variable */
IxMutex macStrLock;
char macStrTable[MT_ROTATE_COUNT][18]; /* [MT_ROTATE_COUNT entries to rotate][6 MAC bytes * 2 hex characters + 5 semicolons + 1 NULL terminator] */
int macStrIndex; /* currently available slot */

extern HashTable dbHashtable;

const char* lockString(IxFastMutex *lock);

const char* mac2string(const unsigned char *mac)
{
    static int initialized;
    char *str;

    if (initialized == 0)
    {
        ixOsServMutexInit(&macStrLock);

        initialized = 1;
    }

    ixOsServMutexLock(&macStrLock);
    
    str = macStrTable[macStrIndex];

    /* increment and rotate */
    macStrIndex = macStrIndex < MT_ROTATE_COUNT ? macStrIndex + 1 : 0;

    ixOsServMutexUnlock(&macStrLock);

    sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return str;
}

void showHashInfo()
{
    int index;
    int maxChaining      = 0;
    int links            = 0;
    int usedBuckets      = 0;
    HashTable *hashTable = &dbHashtable;

    printf("hash [%p], using %d buckets\n", hashTable, hashTable->numBuckets);

    for (index = 0 ; index < hashTable->numBuckets ; index++)
    {
        if (hashTable->hashBuckets[index] != NULL)
        {
            HashNode *node = hashTable->hashBuckets[index];
            int chaining = 0;

            printf("bucket %d%s: ", index, lockString(&hashTable->bucketLocks[index]));

            usedBuckets++;

            while (node != NULL)
            {
                MacDescriptor *descriptor = (MacDescriptor *) node->data;

                printf("%p [%s] %s%s", node, mac2string(descriptor->macAddress), lockString(&node->lock), node->next != NULL ? " -> " : "");

                chaining++;
                links++;
                node = node->next;
            }

            if (chaining > maxChaining)
            {
                maxChaining = chaining;
            }

            printf("\n");
        }
    }

#ifdef __vxworks
    printf("hash chaining: average %f, worst %d\n", (float) links / usedBuckets, maxChaining);
    printf("spreading: %f%%\n", (float) usedBuckets / hashTable->numBuckets);
#else
    printf("hash chaining: average %d, worst %d\n", (links * 100)/ usedBuckets, maxChaining);
    printf("spreading: %d%%\n", (usedBuckets * 100) / hashTable->numBuckets);
#endif

}

int ixEthDBAnalyzeHash()
{
    int index;
    int maxChaining      = 0;
    int links            = 0;
    int usedBuckets      = 0;
    HashTable *hashTable = &dbHashtable;
    BOOL duplicates      = FALSE;

#ifdef __vxworks
    float averageChaining;
#else
    int averageChaining;
#endif

    for (index = 0 ; index < hashTable->numBuckets ; index++)
    {
        if (hashTable->hashBuckets[index] != NULL)
        {
            HashNode *node = hashTable->hashBuckets[index];
            int chaining = 0;

            usedBuckets++;

            while (node != NULL)
            {
                MacDescriptor *descriptor = (MacDescriptor *) node->data;
                HashNode *next = node->next;

                chaining++;
                links++;

                /* look for duplicate entries */
                while (next != NULL)
                {
                    MacDescriptor *nextDescriptor = (MacDescriptor *) next->data;

                    if (ixEthDBAddressCompare(descriptor->macAddress, nextDescriptor->macAddress) == 0)
                    {
                        printf("Warning, duplicate entries [%s] found in bucket %d\n", mac2string(descriptor->macAddress), index);

                        duplicates = TRUE;

                        next = NULL;
                    }
                    else
                    {
                        next = next->next;
                    }
                }

                node = node->next;
            }

            if (chaining > maxChaining)
            {
                maxChaining = chaining;
            }
        }
    }

    /* show info */
    printf("\nHash table [%p], using %d buckets\n\n", hashTable, hashTable->numBuckets);

    printf("Structural info:\n");

#ifdef __vxworks
    averageChaining = (float) links / usedBuckets;
    printf("\tchaining : average %f (%s), worst %d\n", 
        averageChaining, 
        averageChaining < 1.5 ? "excellent" : 
        averageChaining < 2 ? "good" :
        averageChaining < 2.5 ? "ok" :
        averageChaining < 5 ? "bad" : "terrible",
        maxChaining);
    printf("\tspreading: %f%%\n", (float) usedBuckets / hashTable->numBuckets * 100);
#else
    averageChaining = (links * 100)/ usedBuckets;
    printf("\tchaining : average %d/100 (%s), worst %d\n", 
        averageChaining, 
        averageChaining < 150 ? "excellent" : 
        averageChaining < 200 ? "good" :
        averageChaining < 250 ? "ok" :
        averageChaining < 500 ? "bad" : "terrible",
        maxChaining);
    printf("\tspreading: %d%%\n", (usedBuckets * 100) / hashTable->numBuckets);
#endif

    printf("\n");

    if (duplicates)
    {
        printf("Warning, duplicate entries were found (invalid state), check details above\n");
    }
    else
    {
        printf("No duplicates found (valid state)\n");
    }

    printf("\n");

    return 0;
}

int numHashElements()
{   
    int index;
    int numElements = 0;
    HashTable *hashTable = &dbHashtable;

    for (index = 0 ; index < hashTable->numBuckets ; index++)
    {
        if (hashTable->hashBuckets[index] != NULL)
        {
            HashNode *node = hashTable->hashBuckets[index];

            while (node != NULL)
            {
                numElements++;

                node = node->next;
            }
        }
    }

    return numElements;
}

const char* lockString(IxFastMutex *lock)
{
    BOOL locked = (ixOsServFastMutexTryLock(lock) == IX_FAIL);

    if (!locked)
    {
        ixOsServFastMutexUnlock(lock);
    }

    return locked ? " (*) " : "";
}

const char* errorString(IxEthDBStatus error)
{
    const char *str;

    switch (error)
    {
        case IX_ETH_DB_SUCCESS:
            str = "SUCCESS";
            break;
        case IX_ETH_DB_FAIL:
            str = "FAIL";
            break;
        case IX_ETH_DB_BUSY:
            str = "BUSY";
            break;
        case IX_ETH_DB_NOMEM:
            str = "NOMEM";
            break;
        case IX_ETH_DB_NO_SUCH_ADDR:
            str = "NO_SUCH_ADDR";
            break;
        case IX_ETH_DB_INVALID_PORT:
            str = "INVALID_PORT";
            break;
        case IX_ETH_DB_PORT_UNINITIALIZED:
            str = "PORT_UNINITIALIZED";
            break;
        case IX_ETH_DB_MAC_UNINITIALIZED:
            str = "MAC_UNINITIALIZED";
            break;
        case IX_ETH_DB_INVALID_ARG:
            str = "INVALID_ARG";
            break;
        default:
            str = "UNKNOWN/UNLISTED";
    }

    return str;
}

void zapHashtable()
{
    HashIterator iterator;
    int zapCounter = 0;

    BUSY_RETRY(ixEthDBInitHashIterator(&dbHashtable, &iterator));

    while (IS_ITERATOR_VALID(&iterator))
    {
        BUSY_RETRY(ixEthDBRemoveEntryAtHashIterator(&dbHashtable, &iterator));

        zapCounter++;
    }

    printf("Zapped %d records\n", zapCounter);
}

