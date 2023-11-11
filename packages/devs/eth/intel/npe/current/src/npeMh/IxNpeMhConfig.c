/**
 * @file IxNpeMhConfig.c
 *
 * @author Intel Corporation
 * @date 18 Jan 2002
 *
 * @brief This file contains the implementation of the private API for the
 * Configuration module.
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

/*
 * Put the system defined include files required.
 */

#ifndef __ECOS
#include <stdio.h>
#endif

/*
 * Put the user defined include files required.
 */

#include "IxNpeMhMacros_p.h"

#include "IxOsServices.h"

#include "IxNpeMhConfig_p.h"

/*
 * #defines and macros used in this file.
 */

/*
 * Typedefs whose scope is limited to this file.
 */

/**
 * @struct IxNpeMhConfigNpeInfo
 *
 * @brief This structure is used to maintain the configuration information
 * associated with an NPE.
 */

typedef struct
{
    IxMutex mutex;          /**< mutex */
    UINT32 interruptId;     /**< interrupt ID */
    UINT32 statusRegister;  /**< status register address */
    UINT32 controlRegister; /**< control register address */
    UINT32 inFifoRegister;  /**< inFIFO register address */
    UINT32 outFifoRegister; /**< outFIFO register address */
    IxNpeMhConfigIsr isr;   /**< isr routine for handling interrupt */
    BOOL oldInterruptState; /**< old interrupt state (TRUE => enabled) */
} IxNpeMhConfigNpeInfo;

/**
 * @struct IxNpeMhConfigStats
 *
 * @brief This structure is used to maintain statistics for the
 * Configuration module.
 */

typedef struct
{
    UINT32 outFifoReads;        /**< outFifo reads */
    UINT32 inFifoWrites;        /**< inFifo writes */
    UINT32 maxInFifoFullRetries;   /**< max retries if inFIFO full   */
    UINT32 maxOutFifoEmptyRetries; /**< max retries if outFIFO empty */
} IxNpeMhConfigStats;

/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

PRIVATE IxNpeMhConfigNpeInfo ixNpeMhConfigNpeInfo[IX_NPEMH_NUM_NPES] =
{
    {
        0,
        IX_NPEMH_NPEA_INT,
        IX_NPEMH_NPEA_STAT,
        IX_NPEMH_NPEA_CTL,
        IX_NPEMH_NPEA_FIFO,
        IX_NPEMH_NPEA_FIFO,
        NULL,
        FALSE
    },
    {
        0,
        IX_NPEMH_NPEB_INT,
        IX_NPEMH_NPEB_STAT,
        IX_NPEMH_NPEB_CTL,
        IX_NPEMH_NPEB_FIFO,
        IX_NPEMH_NPEB_FIFO,
        NULL,
        FALSE
    },
    {
        0,
        IX_NPEMH_NPEC_INT,
        IX_NPEMH_NPEC_STAT,
        IX_NPEMH_NPEC_CTL,
        IX_NPEMH_NPEC_FIFO,
        IX_NPEMH_NPEC_FIFO,
        NULL,
        FALSE
    }
};

PRIVATE IxNpeMhConfigStats ixNpeMhConfigStats[IX_NPEMH_NUM_NPES];

/*
 * Extern function prototypes.
 */

/*
 * Static function prototypes.
 */

/*
 * Function definition: ixNpeMhConfigIsr
 */

PRIVATE
void ixNpeMhConfigIsr (void *parameter)
{
    IxNpeMhNpeId npeId = (IxNpeMhNpeId)parameter;
    UINT32 ofint;
    volatile UINT32 *statusReg =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].statusRegister;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhConfigIsr\n");

    /* get the OFINT (OutFifo interrupt) bit of the status register */
    IX_NPEMH_REGISTER_READ_BITS (statusReg, &ofint, IX_NPEMH_NPE_STAT_OFINT);

    /* if the OFINT status bit is set */
    if (ofint)
    {
        /* if there is an ISR registered for this NPE */
        if (ixNpeMhConfigNpeInfo[npeId].isr != NULL)
        {
            /* invoke the ISR routine */
            ixNpeMhConfigNpeInfo[npeId].isr (npeId);
        }
        else
        {
            /* if we don't service the interrupt the NPE will continue */
            /* to trigger the interrupt indefinitely */
            IX_NPEMH_ERROR_REPORT ("No ISR registered to service "
                                   "interrupt\n");
        }
    }

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhConfigIsr\n");
}

/*
 * Function definition: ixNpeMhConfigInitialize
 */

void ixNpeMhConfigInitialize (
    IxNpeMhNpeInterrupts npeInterrupts)
{
    IxNpeMhNpeId npeId;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhConfigInitialize\n");

    /* for each NPE ... */
    for (npeId = 0; npeId < IX_NPEMH_NUM_NPES; npeId++)
    {
        /* declare a convenience pointer */
        IxNpeMhConfigNpeInfo *npeInfo = &ixNpeMhConfigNpeInfo[npeId];

        /* for test purposes - to verify the register addresses */
        IX_NPEMH_TRACE2 (IX_NPEMH_DEBUG, "NPE %d status register  = "
                         "0x%08X\n", npeId, npeInfo->statusRegister);
        IX_NPEMH_TRACE2 (IX_NPEMH_DEBUG, "NPE %d control register = "
                         "0x%08X\n", npeId, npeInfo->controlRegister);
        IX_NPEMH_TRACE2 (IX_NPEMH_DEBUG, "NPE %d inFifo register  = "
                         "0x%08X\n", npeId, npeInfo->inFifoRegister);
        IX_NPEMH_TRACE2 (IX_NPEMH_DEBUG, "NPE %d outFifo register = "
                         "0x%08X\n", npeId, npeInfo->outFifoRegister);

        /* connect our ISR to the NPE interrupt */
        (void) ixOsServIntBind (
            npeInfo->interruptId, ixNpeMhConfigIsr, (void *)npeId);

        /* initialise a mutex for this NPE */
        (void) ixOsServMutexInit (&npeInfo->mutex);

        /* if we should service the NPE's "outFIFO not empty" interrupt */
        if (npeInterrupts == IX_NPEMH_NPEINTERRUPTS_YES)
        {
            /* enable the NPE's "outFIFO not empty" interrupt */
            ixNpeMhConfigNpeInterruptEnable (npeId);
        }
        else
        {
            /* disable the NPE's "outFIFO not empty" interrupt */
            ixNpeMhConfigNpeInterruptDisable (npeId);
        }
    }

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhConfigInitialize\n");
}

/*
 * Function definition: ixNpeMhConfigIsrRegister
 */

void ixNpeMhConfigIsrRegister (
    IxNpeMhNpeId npeId,
    IxNpeMhConfigIsr isr)
{
    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhConfigIsrRegister\n");

    /* check if there is already an ISR registered for this NPE */
    if (ixNpeMhConfigNpeInfo[npeId].isr != NULL)
    {
        IX_NPEMH_TRACE0 (IX_NPEMH_DEBUG, "Over-writing registered NPE ISR\n");
    }

    /* save the ISR routine with the NPE info */
    ixNpeMhConfigNpeInfo[npeId].isr = isr;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhConfigIsrRegister\n");
}

/*
 * Function definition: ixNpeMhConfigNpeInterruptEnable
 */

BOOL ixNpeMhConfigNpeInterruptEnable (
    IxNpeMhNpeId npeId)
{
    UINT32 ofe;
    volatile UINT32 *controlReg =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].controlRegister;

    /* get the OFE (OutFifoEnable) bit of the control register */
    IX_NPEMH_REGISTER_READ_BITS (controlReg, &ofe, IX_NPEMH_NPE_CTL_OFE);

    /* if the interrupt is disabled then we must enable it */
    if (!ofe)
    {
        /* set the OFE (OutFifoEnable) bit of the control register */
        /* we must set the OFEWE (OutFifoEnableWriteEnable) at the same */
        /* time for the write to have effect */
        IX_NPEMH_REGISTER_WRITE_BITS (controlReg,
                                      (IX_NPEMH_NPE_CTL_OFE |
                                       IX_NPEMH_NPE_CTL_OFEWE),
                                      (IX_NPEMH_NPE_CTL_OFE |
                                       IX_NPEMH_NPE_CTL_OFEWE));
    }

    /* return the previous state of the interrupt */
    return (ofe != 0);
}

/*
 * Function definition: ixNpeMhConfigNpeInterruptDisable
 */

BOOL ixNpeMhConfigNpeInterruptDisable (
    IxNpeMhNpeId npeId)
{
    UINT32 ofe;
    volatile UINT32 *controlReg =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].controlRegister;

    /* get the OFE (OutFifoEnable) bit of the control register */
    IX_NPEMH_REGISTER_READ_BITS (controlReg, &ofe, IX_NPEMH_NPE_CTL_OFE);

    /* if the interrupt is enabled then we must disable it */
    if (ofe)
    {
        /* unset the OFE (OutFifoEnable) bit of the control register */
        /* we must set the OFEWE (OutFifoEnableWriteEnable) at the same */
        /* time for the write to have effect */
        IX_NPEMH_REGISTER_WRITE_BITS (controlReg,
                                      (0                    |
                                       IX_NPEMH_NPE_CTL_OFEWE),
                                      (IX_NPEMH_NPE_CTL_OFE |
                                       IX_NPEMH_NPE_CTL_OFEWE));
    }

    /* return the previous state of the interrupt */
    return (ofe != 0);
}

/*
 * Function definition: ixNpeMhConfigMessageIdGet
 */

IxNpeMhMessageId ixNpeMhConfigMessageIdGet (
    IxNpeMhMessage message)
{
    /* return the most-significant byte of the first word of the */
    /* message */
    return ((IxNpeMhMessageId) ((message.data[0] >> 24) & 0xFF));
}

/*
 * Function definition: ixNpeMhConfigNpeIdIsValid
 */

BOOL ixNpeMhConfigNpeIdIsValid (
    IxNpeMhNpeId npeId)
{
    /* check that the npeId parameter is within the range of valid IDs */
    return (npeId >= 0 && npeId < IX_NPEMH_NUM_NPES);
}

/*
 * Function definition: ixNpeMhConfigLockGet
 */

void ixNpeMhConfigLockGet (
    IxNpeMhNpeId npeId)
{
    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhConfigLockGet\n");

    /* lock the mutex for this NPE */
    (void) ixOsServMutexLock (&ixNpeMhConfigNpeInfo[npeId].mutex);

    /* disable the NPE's "outFIFO not empty" interrupt */
    ixNpeMhConfigNpeInfo[npeId].oldInterruptState =
        ixNpeMhConfigNpeInterruptDisable (npeId);

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhConfigLockGet\n");
}

/*
 * Function definition: ixNpeMhConfigLockRelease
 */

void ixNpeMhConfigLockRelease (
    IxNpeMhNpeId npeId)
{
    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhConfigLockRelease\n");

    /* if the interrupt was previously enabled */
    if (ixNpeMhConfigNpeInfo[npeId].oldInterruptState)
    {
        /* enable the NPE's "outFIFO not empty" interrupt */
        ixNpeMhConfigNpeInfo[npeId].oldInterruptState =
            ixNpeMhConfigNpeInterruptEnable (npeId);
    }

    /* unlock the mutex for this NPE */
    (void) ixOsServMutexUnlock (&ixNpeMhConfigNpeInfo[npeId].mutex);

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhConfigLockRelease\n");
}

/*
 * Function definition: ixNpeMhConfigInFifoIsEmpty
 */

__inline__
BOOL ixNpeMhConfigInFifoIsEmpty (
    IxNpeMhNpeId npeId)
{
    UINT32 ifne;
    volatile UINT32 *statusReg =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].statusRegister;

    /* get the IFNE (InFifoNotEmpty) bit of the status register */
    IX_NPEMH_REGISTER_READ_BITS (statusReg, &ifne, IX_NPEMH_NPE_STAT_IFNE);

    /* if the IFNE status bit is unset then the inFIFO is empty */
    return (ifne == 0);
}

/*
 * Function definition: ixNpeMhConfigInFifoIsFull
 */

__inline__
BOOL ixNpeMhConfigInFifoIsFull (
    IxNpeMhNpeId npeId)
{
    UINT32 ifnf;
    volatile UINT32 *statusReg =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].statusRegister;

    /* get the IFNF (InFifoNotFull) bit of the status register */
    IX_NPEMH_REGISTER_READ_BITS (statusReg, &ifnf, IX_NPEMH_NPE_STAT_IFNF);

    /* if the IFNF status bit is unset then the inFIFO is full */
    return (ifnf == 0);
}

/*
 * Function definition: ixNpeMhConfigOutFifoIsEmpty
 */

__inline__
BOOL ixNpeMhConfigOutFifoIsEmpty (
    IxNpeMhNpeId npeId)
{
    UINT32 ofne;
    volatile UINT32 *statusReg =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].statusRegister;

    /* get the OFNE (OutFifoNotEmpty) bit of the status register */
    IX_NPEMH_REGISTER_READ_BITS (statusReg, &ofne, IX_NPEMH_NPE_STAT_OFNE);

    /* if the OFNE status bit is unset then the outFIFO is empty */
    return (ofne == 0);
}

/*
 * Function definition: ixNpeMhConfigOutFifoIsFull
 */

__inline__
BOOL ixNpeMhConfigOutFifoIsFull (
    IxNpeMhNpeId npeId)
{
    UINT32 ofnf;
    volatile UINT32 *statusReg =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].statusRegister;

    /* get the OFNF (OutFifoNotFull) bit of the status register */
    IX_NPEMH_REGISTER_READ_BITS (statusReg, &ofnf, IX_NPEMH_NPE_STAT_OFNF);

    /* if the OFNF status bit is unset then the outFIFO is full */
    return (ofnf == 0);
}

/*
 * Function definition: ixNpeMhConfigInFifoWrite
 */

void ixNpeMhConfigInFifoWrite (
    IxNpeMhNpeId npeId,
    IxNpeMhMessage message)
{
    volatile UINT32 *npeInFifo =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].inFifoRegister;
    UINT32 retriesCount = 0;

    /* write the first word of the message to the NPE's inFIFO */
    IX_NPEMH_REGISTER_WRITE (npeInFifo, message.data[0]);

    /* need to wait for room to write second word - see SCR #493 */
    while (ixNpeMhConfigInFifoIsFull (npeId))
    {
        retriesCount++;
    }

    /* write the second word of the message to the NPE's inFIFO */
    IX_NPEMH_REGISTER_WRITE (npeInFifo, message.data[1]);

    /* record in the stats the maximum number of retries needed */
    if (ixNpeMhConfigStats[npeId].maxInFifoFullRetries < retriesCount)
    {
	ixNpeMhConfigStats[npeId].maxInFifoFullRetries = retriesCount;
    }

    /* update statistical info */
    ixNpeMhConfigStats[npeId].inFifoWrites++;
}

/*
 * Function definition: ixNpeMhConfigOutFifoRead
 */

void ixNpeMhConfigOutFifoRead (
    IxNpeMhNpeId npeId,
    IxNpeMhMessage *message)
{
    volatile UINT32 *npeOutFifo =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].outFifoRegister;
    UINT32 retriesCount = 0;

    /* read the first word of the message from the NPE's outFIFO */
    IX_NPEMH_REGISTER_READ (npeOutFifo, &message->data[0]);

    /* need to wait for NPE to write second word - see SCR #493 */
    while (ixNpeMhConfigOutFifoIsEmpty (npeId))
    {
        retriesCount++;
    }

    /* read the second word of the message from the NPE's outFIFO */
    IX_NPEMH_REGISTER_READ (npeOutFifo, &message->data[1]);

    /* record in the stats the maximum number of retries needed */
    if (ixNpeMhConfigStats[npeId].maxOutFifoEmptyRetries < retriesCount)
    {
	ixNpeMhConfigStats[npeId].maxOutFifoEmptyRetries = retriesCount;
    }

    /* update statistical info */
    ixNpeMhConfigStats[npeId].outFifoReads++;
}

/*
 * Function definition: ixNpeMhConfigShow
 */

void ixNpeMhConfigShow (
    IxNpeMhNpeId npeId)
{
    /* show the message fifo read counter */
    IX_NPEMH_SHOW ("Message FIFO reads",
                   ixNpeMhConfigStats[npeId].outFifoReads);

    /* show the message fifo write counter */
    IX_NPEMH_SHOW ("Message FIFO writes",
                   ixNpeMhConfigStats[npeId].inFifoWrites);

    /* show the max retries performed when inFIFO full */
    IX_NPEMH_SHOW ("Max inFIFO Full retries",
		   ixNpeMhConfigStats[npeId].maxInFifoFullRetries);

    /* show the max retries performed when outFIFO empty */
    IX_NPEMH_SHOW ("Max outFIFO Empty retries",
		   ixNpeMhConfigStats[npeId].maxOutFifoEmptyRetries);

    /* show the current status of the inFifo */
    printf ("InFifo is %s and %s\n",
	    (ixNpeMhConfigInFifoIsEmpty (npeId) ? "EMPTY" : "NOT EMPTY"),
	    (ixNpeMhConfigInFifoIsFull (npeId) ? "FULL" : "NOT FULL"));

    /* show the current status of the outFifo */
    printf ("OutFifo is %s and %s\n",
	    (ixNpeMhConfigOutFifoIsEmpty (npeId) ? "EMPTY" : "NOT EMPTY"),
	    (ixNpeMhConfigOutFifoIsFull (npeId) ? "FULL" : "NOT FULL"));
}

/*
 * Function definition: ixNpeMhConfigShowReset
 */

void ixNpeMhConfigShowReset (
    IxNpeMhNpeId npeId)
{
    /* reset the message fifo read counter */
    ixNpeMhConfigStats[npeId].outFifoReads = 0;

    /* reset the message fifo write counter */
    ixNpeMhConfigStats[npeId].inFifoWrites = 0;

    /* reset the max inFIFO Full retries counter */
    ixNpeMhConfigStats[npeId].maxInFifoFullRetries = 0;

    /* reset the max outFIFO empty retries counter */
    ixNpeMhConfigStats[npeId].maxOutFifoEmptyRetries = 0;
}
