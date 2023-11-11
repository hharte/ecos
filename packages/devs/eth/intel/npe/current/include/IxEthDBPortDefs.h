/**
 * @file IxEthDBPortDefs.h
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

/**
 * @defgroup IxEthDBPortDefs IXP4xx Ethernet Database Port Definitions (IxEthDBPortDefs)
 *
 * @brief IXP4xx Ethernet Port Definitions for private MAC learning API
 *
 * @{
 */

#ifndef IxEthDBPortDefs_H
#define IxEthDBPortDefs_H

/** 
 * @brief Port types - currently only Ethernet NPEs are recognized as specific types 
 */
typedef enum
{
    ETH_GENERIC = 0,
    ETH_NPE
} IxEthDBPortType;

/** 
 * @brief Port capabilities - used by ixEthAccDatabaseMaintenance to decide whether it
 * should manually age entries or not depending on the port capabilities.
 *
 * Ethernet NPEs have aging capabilities, meaning that they will age the entries
 * automatically (by themselves).*/
typedef enum
{
    NO_CAPABILITIES   = 0,
    ENTRY_AGING       = 0x1
} IxEthDBPortCapability;

/** 
 * @brief Port Definition - a structure contains the Port type and capabilities 
 */
typedef struct
{
    IxEthDBPortType type;
    IxEthDBPortCapability capabilities;
} IxEthDBPortDefinition;

/** 
 * @brief Port definitions structure, indexed on the port ID
 * @note ports 0 and 1 are used by the Ethernet access component therefore
 * it is essential to be left untouched. Port 2 here (WAN) is given as an example port. 
 */
static IxEthDBPortDefinition ixEthDBPortDefinitions[] = 
{
    /*    id       type           capabilities */
    {   /* 0 */    ETH_NPE,       ENTRY_AGING },    /* Ethernet NPE B */
    {   /* 1 */    ETH_NPE,       ENTRY_AGING },    /* Ethernet NPE C */
    {   /* 2 */    ETH_GENERIC,   NO_CAPABILITIES } /* WAN port */
};

/** 
 * @def IX_ETH_DB_NUMBER_OF_PORTS
 * @def IX_ETH_DB_NUMBER_OF_PORTS 
 * @brief number of supported ports 
 */
#define IX_ETH_DB_NUMBER_OF_PORTS (3)

/** 
 * @def COMPLETE_ETH_PORT_MAP 
 * @brief complete set of ports in use 
 *
 * only ports 0, 1 and 2 are in use - sets bit[n] to 1 if port[n] exists
 */
#define COMPLETE_ETH_PORT_MAP ((1 << IX_ETH_DB_NUMBER_OF_PORTS) - 1)

/** 
 * @def IX_ETH_DB_CHECK_PORT(portID)
 * @brief safety checks to verify whether the port is invalid or uninitialized
 */
#define IX_ETH_DB_CHECK_PORT(portID) \
{ \
    if ((portID) < 0 || (portID) >= IX_ETH_DB_NUMBER_OF_PORTS) \
    { \
        return IX_ETH_DB_INVALID_PORT; \
    } \
    \
    if (!ixEthDBPortInfo[(portID)].enabled) \
    { \
        return IX_ETH_DB_PORT_UNINITIALIZED; \
    } \
}

#endif /* IxEthDBPortDefs_H */
/**
 *@}
 */
