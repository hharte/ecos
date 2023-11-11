/**
 * @file IxEthAccDBLocks_p.h
 *
 * @brief Definition of transaction lock stacks and lock utility macros
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

#ifndef IxEthAccDBLocks_p_H
#define IxEthAccDBLocks_p_H

/* Lock and lock stacks */
typedef struct
{
    IxFastMutex* locks[MAX_LOCKS];
    UINT32 stackPointer, basePointer;
} LockStack;

#define TRY_LOCK(mutex) \
    { \
        if (ixOsServFastMutexTryLock(mutex) != IX_SUCCESS) \
        { \
            return IX_ETH_DB_BUSY; \
        } \
    }


#define UNLOCK(mutex) { ixOsServFastMutexUnlock(mutex); }

#define INIT_STACK(stack) \
    { \
        (stack)->basePointer  = 0; \
        (stack)->stackPointer = 0; \
    }

#define PUSH_LOCK(stack, lock) \
    { \
        if ((stack)->stackPointer == MAX_LOCKS) \
        { \
            ERROR_LOG("Ethernet DB: maximum number of elements in a lock stack has been exceeded on push, heavy chaining?\n"); \
            UNROLL_STACK(stack); \
            \
            return IX_ETH_DB_BUSY; \
        } \
        \
        if (ixOsServFastMutexTryLock(lock) == IX_SUCCESS) \
        { \
            (stack)->locks[(stack)->stackPointer++] = (lock); \
        } \
        else \
        { \
            UNROLL_STACK(stack); \
            \
            return IX_ETH_DB_BUSY; \
        } \
    }

#define POP_LOCK(stack) \
    { \
        ixOsServFastMutexUnlock((stack)->locks[--(stack)->stackPointer]); \
    }

#define UNROLL_STACK(stack) \
    { \
        while ((stack)->stackPointer > (stack)->basePointer) \
        { \
            POP_LOCK(stack); \
        } \
    }

#define SHIFT_STACK(stack) \
    { \
        if ((stack)->basePointer == MAX_LOCKS - 1) \
        { \
            ERROR_LOG("Ethernet DB: maximum number of elements in a lock stack has been exceeded on shift, heavy chaining?\n"); \
            UNROLL_STACK(stack); \
            \
            return IX_ETH_DB_BUSY; \
        } \
        \
        ixOsServFastMutexUnlock((stack)->locks[(stack)->basePointer++]); \
    }

#endif /* IxEthAccDBLocks_p_H */
