/**
 * @file IxNpeMhConfig_p.h
 *
 * @author Intel Corporation
 * @date 18 Jan 2002
 *
 * @brief This file contains the private API for the Configuration module.
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

/**
 * @defgroup IxNpeMhConfig_p IxNpeMhConfig_p
 *
 * @brief The private API for the Configuration module.
 * 
 * @{
 */

#ifndef IXNPEMHCONFIG_P_H
#define IXNPEMHCONFIG_P_H

#ifndef __ECOS
#include <iv.h>
#endif

#include <ixp425.h>

#ifndef __ECOS
#include <ixp425IntrCtl.h>
#endif

#include "IxNpeMh.h"
#include "IxTypes.h"

/*
 * #defines for function return types, etc.
 */

typedef void (*IxNpeMhConfigIsr) (int); /**< ISR function pointer */

/**< NPE register base address */
#define IX_NPEMH_NPE_BASE IXP425_PERIPHERAL_BASE

#define IX_NPEMH_NPEA_OFFSET (0x6000) /**< NPE-A register base offset */
#define IX_NPEMH_NPEB_OFFSET (0x7000) /**< NPE-B register base offset */
#define IX_NPEMH_NPEC_OFFSET (0x8000) /**< NPE-C register base offset */

#define IX_NPEMH_NPESTAT_OFFSET (0x002C) /**< NPE status register offset */
#define IX_NPEMH_NPECTL_OFFSET  (0x0030) /**< NPE control register offset */
#define IX_NPEMH_NPEFIFO_OFFSET (0x0038) /**< NPE FIFO register offset */

/** NPE-A register base address */
#define IX_NPEMH_NPEA_BASE (IX_NPEMH_NPE_BASE + IX_NPEMH_NPEA_OFFSET)
/** NPE-B register base address */
#define IX_NPEMH_NPEB_BASE (IX_NPEMH_NPE_BASE + IX_NPEMH_NPEB_OFFSET)
/** NPE-C register base address */
#define IX_NPEMH_NPEC_BASE (IX_NPEMH_NPE_BASE + IX_NPEMH_NPEC_OFFSET)

/* NPE-A configuration */

/** NPE-A interrupt */
#define IX_NPEMH_NPEA_INT  (INT_VEC_PSMA)
/** NPE-A FIFO register */
#define IX_NPEMH_NPEA_FIFO (IX_NPEMH_NPEA_BASE + IX_NPEMH_NPEFIFO_OFFSET)
/** NPE-A control register */
#define IX_NPEMH_NPEA_CTL  (IX_NPEMH_NPEA_BASE + IX_NPEMH_NPECTL_OFFSET)
/** NPE-A status register */
#define IX_NPEMH_NPEA_STAT (IX_NPEMH_NPEA_BASE + IX_NPEMH_NPESTAT_OFFSET)

/* NPE-B configuration */

/** NPE-B interrupt */
#define IX_NPEMH_NPEB_INT  (INT_VEC_PSMB)
/** NPE-B FIFO register */
#define IX_NPEMH_NPEB_FIFO (IX_NPEMH_NPEB_BASE + IX_NPEMH_NPEFIFO_OFFSET)
/** NPE-B control register */
#define IX_NPEMH_NPEB_CTL  (IX_NPEMH_NPEB_BASE + IX_NPEMH_NPECTL_OFFSET)
/** NPE-B status register */
#define IX_NPEMH_NPEB_STAT (IX_NPEMH_NPEB_BASE + IX_NPEMH_NPESTAT_OFFSET)

/* NPE-C configuration */

/** NPE-C interrupt */
#define IX_NPEMH_NPEC_INT  (INT_VEC_PSMC)
/** NPE-C FIFO register */
#define IX_NPEMH_NPEC_FIFO (IX_NPEMH_NPEC_BASE + IX_NPEMH_NPEFIFO_OFFSET)
/** NPE-C control register */
#define IX_NPEMH_NPEC_CTL  (IX_NPEMH_NPEC_BASE + IX_NPEMH_NPECTL_OFFSET)
/** NPE-C status register */
#define IX_NPEMH_NPEC_STAT (IX_NPEMH_NPEC_BASE + IX_NPEMH_NPESTAT_OFFSET)

/* NPE control register bit definitions */
#define IX_NPEMH_NPE_CTL_OFE   (1 << 16) /**< OutFifoEnable */
#define IX_NPEMH_NPE_CTL_IFE   (1 << 17) /**< InFifoEnable */
#define IX_NPEMH_NPE_CTL_OFEWE (1 << 24) /**< OutFifoEnableWriteEnable */
#define IX_NPEMH_NPE_CTL_IFEWE (1 << 25) /**< InFifoEnableWriteEnable */

/* NPE status register bit definitions */
#define IX_NPEMH_NPE_STAT_OFNE  (1 << 16) /**< OutFifoNotEmpty */
#define IX_NPEMH_NPE_STAT_IFNF  (1 << 17) /**< InFifoNotFull */
#define IX_NPEMH_NPE_STAT_OFNF  (1 << 18) /**< OutFifoNotFull */
#define IX_NPEMH_NPE_STAT_IFNE  (1 << 19) /**< InFifoNotEmpty */
#define IX_NPEMH_NPE_STAT_MBINT (1 << 20) /**< Mailbox interrupt */
#define IX_NPEMH_NPE_STAT_IFINT (1 << 21) /**< InFifo interrupt */
#define IX_NPEMH_NPE_STAT_OFINT (1 << 22) /**< OutFifo interrupt */
#define IX_NPEMH_NPE_STAT_WFINT (1 << 23) /**< WatchFifo interrupt */

/*
 * Prototypes for interface functions.
 */

/**
 * @fn void ixNpeMhConfigInitialize (
           IxNpeMhNpeInterrupts npeInterrupts)
 *
 * @brief This function initialises the Configuration module.
 *
 * @param IxNpeMhNpeInterrupts npeInterrupts (in) - whether or not to
 * service the NPE "outFIFO not empty" interrupts.
 *
 * @return No return value.
 */

void ixNpeMhConfigInitialize (
    IxNpeMhNpeInterrupts npeInterrupts);

/**
 * @fn void ixNpeMhConfigIsrRegister (
           IxNpeMhNpeId npeId,
           IxNpeMhConfigIsr isr)
 *
 * @brief This function registers an ISR to handle NPE "outFIFO not
 * empty" interrupts.
 *
 * @param IxNpeMhNpeId npeId (in) - the ID of the NPE whose interrupt will
 * be handled.
 * @param IxNpeMhConfigIsr isr (in) - the ISR function pointer that the
 * interrupt will trigger.
 *
 * @return No return value.
 */

void ixNpeMhConfigIsrRegister (
    IxNpeMhNpeId npeId,
    IxNpeMhConfigIsr isr);

/**
 * @fn BOOL ixNpeMhConfigNpeInterruptEnable (
           IxNpeMhNpeId npeId)
 *
 * @brief This function enables a NPE's "outFIFO not empty" interrupt.
 *
 * @param IxNpeMhNpeId npeId (in) - the ID of the NPE whose interrupt will
 * be enabled.
 *
 * @return Returns the previous state of the interrupt (TRUE => enabled).
 */

BOOL ixNpeMhConfigNpeInterruptEnable (
    IxNpeMhNpeId npeId);

/**
 * @fn BOOL ixNpeMhConfigNpeInterruptDisable (
           IxNpeMhNpeId npeId)
 *
 * @brief This function disables a NPE's "outFIFO not empty" interrupt
 *
 * @param IxNpeMhNpeId npeId (in) - the ID of the NPE whose interrupt will
 * be disabled.
 *
 * @return Returns the previous state of the interrupt (TRUE => enabled).
 */

BOOL ixNpeMhConfigNpeInterruptDisable (
    IxNpeMhNpeId npeId);

/**
 * @fn IxNpeMhMessageId ixNpeMhConfigMessageIdGet (
           IxNpeMhMessage message)
 *
 * @brief This function gets the ID of a message.
 *
 * @param IxNpeMhMessage message (in) - the message to get the ID of.
 *
 * @return the ID of the message
 */

IxNpeMhMessageId ixNpeMhConfigMessageIdGet (
    IxNpeMhMessage message);

/**
 * @fn BOOL ixNpeMhConfigNpeIdIsValid (
           IxNpeMhNpeId npeId)
 *
 * @brief This function checks to see if a NPE ID is valid.
 *
 * @param IxNpeMhNpeId npeId (in) - the NPE ID to validate.
 *
 * @return True if the NPE ID is valid, otherwise False.
 */

BOOL ixNpeMhConfigNpeIdIsValid (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhConfigLockGet (
           IxNpeMhNpeId npeId)
 *
 * @brief This function gets a lock for exclusive NPE interaction, and
 * disables the NPE's "outFIFO not empty" interrupt.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which to get the
 * lock and disable its interrupt.
 *
 * @return No return value.
 */

void ixNpeMhConfigLockGet (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhConfigLockRelease (
           IxNpeMhNpeId npeId)
 *
 * @brief This function releases a lock for exclusive NPE interaction, and
 * enables the NPE's "outFIFO not empty" interrupt.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which to release
 * the lock and enable its interrupt.
 *
 * @return No return value.
 */

void ixNpeMhConfigLockRelease (
    IxNpeMhNpeId npeId);

/**
 * @fn BOOL ixNpeMhConfigInFifoIsEmpty (
           IxNpeMhNpeId npeId)
 *
 * @brief This function checks if a NPE's inFIFO is empty.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which the inFIFO
 * will be checked.
 *
 * @return True if the inFIFO is empty, otherwise False.
 */

BOOL ixNpeMhConfigInFifoIsEmpty (
    IxNpeMhNpeId npeId);

/**
 * @fn BOOL ixNpeMhConfigInFifoIsFull (
           IxNpeMhNpeId npeId)
 *
 * @brief This function checks if a NPE's inFIFO is full.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which the inFIFO
 * will be checked.
 *
 * @return True if the inFIFO is full, otherwise False.
 */

BOOL ixNpeMhConfigInFifoIsFull (
    IxNpeMhNpeId npeId);

/**
 * @fn BOOL ixNpeMhConfigOutFifoIsEmpty (
           IxNpeMhNpeId npeId)
 *
 * @brief This function checks if a NPE's outFIFO is empty.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which the outFIFO
 * will be checked.
 *
 * @return True if the outFIFO is empty, otherwise False.
 */

BOOL ixNpeMhConfigOutFifoIsEmpty (
    IxNpeMhNpeId npeId);

/**
 * @fn BOOL ixNpeMhConfigOutFifoIsFull (
           IxNpeMhNpeId npeId)
 *
 * @brief This function checks if a NPE's outFIFO is full.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which the outFIFO
 * will be checked.
 *
 * @return True if the outFIFO is full, otherwise False.
 */

BOOL ixNpeMhConfigOutFifoIsFull (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhConfigInFifoWrite (
           IxNpeMhNpeId npeId,
           IxNpeMhMessage message)
 *
 * @brief This function writes a message to a NPE's inFIFO.  The caller
 * must first check that the NPE's inFifo is not full.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which the inFIFO
 * will be written to.
 * @param IxNpeMhMessage message (in) - The message to write.
 *
 * @return No return value.
 */

void ixNpeMhConfigInFifoWrite (
    IxNpeMhNpeId npeId,
    IxNpeMhMessage message);

/**
 * @fn void ixNpeMhConfigOutFifoRead (
           IxNpeMhNpeId npeId,
           IxNpeMhMessage *message)
 *
 * @brief This function reads a message from a NPE's outFIFO.  The caller
 * must first check that the NPE's outFifo is not empty.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which the outFIFO
 * will be read from.
 * @param IxNpeMhMessage message (out) - The message read.
 *
 * @return No return value.
 */

void ixNpeMhConfigOutFifoRead (
    IxNpeMhNpeId npeId,
    IxNpeMhMessage *message);

/**
 * @fn void ixNpeMhConfigShow (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will display the current state of the Configuration
 * module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to display state
 * information for.
 *
 * @return No return value.
 */

void ixNpeMhConfigShow (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhConfigShowReset (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will reset the current state of the Configuration
 * module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to reset state
 * information for.
 *
 * @return No return value.
 */

void ixNpeMhConfigShowReset (
    IxNpeMhNpeId npeId);

#endif /* IXNPEMHCONFIG_P_H */

/**
 * @} defgroup IxNpeMhConfig_p
 */
