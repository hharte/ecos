/**
 * @file IxNpeMhReceive_p.h
 *
 * @author Intel Corporation
 * @date 18 Jan 2002
 *
 * @brief This file contains the private API for the Receive module.
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
 * @defgroup IxNpeMhReceive_p IxNpeMhReceive_p
 *
 * @brief The private API for the Receive module.
 * 
 * @{
 */

#ifndef IXNPEMHRECEIVE_P_H
#define IXNPEMHRECEIVE_P_H

#include "IxNpeMh.h"
#include "IxTypes.h"

/*
 * #defines for function return types, etc.
 */

/*
 * Prototypes for interface functions.
 */

/**
 * @fn void ixNpeMhReceiveInitialize (void)
 *
 * @brief This function registers an internal ISR to handle the NPEs'
 * "outFIFO not empty" interrupts and receive messages from the NPEs when
 * they become available.
 *
 * @return No return value.
 */

void ixNpeMhReceiveInitialize (void);

/**
 * @fn void ixNpeMhReceiveMessagesReceive (
           IxNpeMhNpeId npeId)
 *
 * @brief This function reads messages from a particular NPE's outFIFO
 * until the outFIFO is empty, and for each message looks first for an
 * unsolicited callback, then a solicited callback, to pass the message
 * back to the client.  If no callback can be found the message is
 * discarded and an error reported.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to receive
 * messages from.
 *
 * @return No return value.
 */

void ixNpeMhReceiveMessagesReceive (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhReceiveShow (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will display the current state of the Receive
 * module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to display state
 * information for.
 *
 * @return No return status.
 */

void ixNpeMhReceiveShow (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhReceiveShowReset (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will reset the current state of the Receive
 * module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to reset state
 * information for.
 *
 * @return No return status.
 */

void ixNpeMhReceiveShowReset (
    IxNpeMhNpeId npeId);

#endif /* IXNPEMHRECEIVE_P_H */

/**
 * @} defgroup IxNpeMhReceive_p
 */
