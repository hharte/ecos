/** 
 * @file IxEthAccFpathDep.h
 *
 * @brief This file contains the dependencies common to the @ref IxEthAcc component
 * and the @ref IxFpath component. ONLY dependencies shared between EthAcc and Fpath
 * are allowed in this file. Consult the relevant component owners before
 * making addition to this file.
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
 *
 */

/**
 * @defgroup IxEthAccFpathDep IXP4xx Ethernet Access Fast Path (IxEthAccFpathDep) API
 * @brief The dependencies common to the @ref IxEthAcc component and the @ref IxFpath component. 
 *
 * @{
 */

#ifndef IxEthAccFpathDep_H
#define IxEthAccFpathDep_H

/**
 * @def  IX_ETH_ACC_FPATH_AWARE
 *
 * @brief This define indicates that the ethernet access service is used in a fast path
 * enabled system.
 *
 * When fast path ATM to Ethernet is enabled, the Ethernet transmit Q for Eth Tx
 * port becomes shared between EthAcc and Npe-A. 64 entries of the transmit Q are
 * reserved for use by NpeA and 64 for used by EthAcc. So when IX_ETH_ACC_FPATH_AWARE
 * is set to TRUE, the TX queue depth available to EthAcc is reduced by half.
 * This will obviously have a potentially major impact on the performance of the
 * component and the timing dynamics of the passage of buffers through the system.
 *
 * IMPLICATIONS - FpathAcc;
 * The maximum number of buffers which can be provisioned for the fast path must
 * be matched with number of entries to be reserved for the fast path on the
 * Eth Tx queue. This is currently hard coded within FpathAcc.
 *
 * IMPLICATIONS - EthAcc;
 * The EthAcc Tx code must make a significant modification to the behaviour such
 * that enough room is maintained in the TX queue for Npe-A to write its entire
 * quota of fast path buffers.
 *
 * Furthermore, in normal operation EthAcc by default relies on the TX Q empty
 * callback to trigger submissions of buffers previously queued on software queues.
 * However when operating in a system in which Fast path is enabled, the TX Q
 * may never become empty, as it may be kept non-empty by submissions from the
 * NPE-A. Hence Tx Q empty trigger cannot be used. Instead the TX Done callback
 * is used to trigger such submissions.
 *
 */

#define IX_ETH_ACC_FPATH_AWARE 0

#endif /* endef IxEthAccFpathDep_H */
/**
 *@}
 */
