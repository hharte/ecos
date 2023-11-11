/**
 * @file IxEthDBMessages_p.h
 *
 * @brief Definitions of NPE messages
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

#ifndef IxEthDBMessages_p_H
#define IxEthDBMessages_p_H

#include <IxEthNpe.h>

/* these macros are correct only in big endian mode (check applies only for the supported XScale processors) */
#if defined(__xscale) && !defined(__BIG_ENDIAN)
    #error NPE-specific memory access macros must be redefined for little-endian mode
#endif

/* events watched by the Eth event processor */
#define P2X_ELT_MIN_EVENT_ID        (IX_ETHNPE_P2X_ELT_ACCESSGRANT)
#define P2X_ELT_MAX_EVENT_ID        (IX_ETHNPE_P2X_ELT_NEWADDRESS)

/* software events */
#define IX_ETH_DB_EXTERN_PORT_UPDATE (0xFF01)

/* macros to fill and extract data from NPE messages - place any endian conversions here */
#define RESET_ELT_MESSAGE(message) { memset((void *) &(message), 0, sizeof((message))); }

#define FILL_ELT_MAC_MESSAGE(message, msgID, portID, macAddr) \
    { \
        (message).data[0] = (msgID) << 24 | (portID) << 16 | (macAddr)->macAddress[0] << 8 | (macAddr)->macAddress[1]; \
        (message).data[1] = (macAddr)->macAddress[2] << 24 | (macAddr)->macAddress[3] << 16 | (macAddr)->macAddress[4] << 8 | (macAddr)->macAddress[5]; \
    }

#define FILL_ELT_ACCESS_MESSAGE(message, msgID, eltSize, eltBaseAddr) \
    { \
        (message).data[0] = (msgID) << 24 | ((eltSize) & 0xFFFF); \
        (message).data[1] = (UINT32) (eltBaseAddr); \
    }

#define NPE_MSG_ID(msg) ((msg).data[0] >> 24)
#define NPE_MSG_ELT_SIZE(msg) ((msg).data[0] & 0xFFFF)
#define NPE_MSG_ELT_BASE_ADDR(msg) ((msg).data[1])

#define COPY_NPE_MSG_MAC_ADDR(msg, macAddr) \
    { \
        (macAddr)->macAddress[0] = ((msg).data[0] >> 8) & 0xFF; \
        (macAddr)->macAddress[1] = (msg).data[0] & 0xFF; \
        (macAddr)->macAddress[2] = ((msg).data[1] >> 24) & 0xFF; \
        (macAddr)->macAddress[3] = ((msg).data[1] >> 16) & 0xFF; \
        (macAddr)->macAddress[4] = ((msg).data[1] >> 8) & 0xFF; \
        (macAddr)->macAddress[5] = (msg).data[1] & 0xFF; \
    }

/* access to a MAC node in the NPE tree */
#define NPE_NODE_PORT_ID_OFFSET (6) /* offset of byte containing the port ID (1 byte) */
#define NPE_NODE_AGE_M_E_OFFSET (7) /* offset of byte containing the age (5 bits), modified flag (1 bit), empty flag (1 bit) */

#define NPE_NODE_BYTE(eltNodeAddr, offset)      (((UINT8 *) (eltNodeAddr))[offset])
#define NPE_NODE_PORT_ID(eltNodeAddr)           (NPE_NODE_BYTE(eltNodeAddr, NPE_NODE_PORT_ID_OFFSET))
#define NPE_NODE_AGE(eltNodeAddr)               (NPE_NODE_BYTE(eltNodeAddr, NPE_NODE_AGE_M_E_OFFSET) >> 2)
#define NPE_NODE_NONEMPTY(eltNodeAddr)          (NPE_NODE_BYTE(eltNodeAddr, NPE_NODE_AGE_M_E_OFFSET) & 0x1)
#define NPE_NODE_MODIFIED(eltNodeAddr)          (NPE_NODE_BYTE(eltNodeAddr, NPE_NODE_AGE_M_E_OFFSET) & 0x2)

/* browsing of the implicit linear binary tree structure of the NPE tree */
#define LEFT_CHILD_OFFSET(offset)   ((offset) << 1)
#define RIGHT_CHILD_OFFSET(offset)  (((offset) << 1) + 1)

/* masks and shifts to set the age and the modified and empty flags */
#define NPE_NODE_AGE_SHIFT          (2)
#define NPE_NODE_NON_EMPTY_FLAG     (0x1)
#define NPE_NODE_EMPTY_FLAG         (0x0)
#define NPE_NODE_NON_MODIFIED_FLAG  (0x0)
#define NPE_NODE_MODIFIED_FLAG      (0x2)

#endif /* IxEthDBMessages_p_H */
