/** @file    IxEthDB.h
 *
 * @brief this file contains the public API of @ref IxEthDB component
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
 
#ifndef IxEthDB_H
#define IxEthDB_H

#include <IxOsBuffMgt.h>
#include <IxTypes.h>

/**
 * @defgroup IxEthDB IXP4xx Ethernet Database (IxEthDB) API
 *
 * @brief ethDB is a library that does provides a MAC address database learning/filtering capability
 *
 *@{
 */

#define INLINE __inline__

#ifdef NDEBUG

#define IX_ETH_DB_PRIVATE static

#else

#define IX_ETH_DB_PRIVATE

#endif /* ndef DEBUG */

#define IX_ETH_DB_PUBLIC

/**
 * @brief port ID => message handler NPE id conversion (0 => NPE_B, 1 => NPE_C)
 */
#define IX_ETH_DB_PORT_ID_TO_NPE(id) (id + 1)

/**
 * @def IX_ETH_DB_NPE_TO_PORT_ID(npe)
 * @brief message handler NPE id => port ID conversion (NPE_B => 0, NPE_C => 1)
 */
#define IX_ETH_DB_NPE_TO_PORT_ID(npe) (npe - 1)

/**
 * @def IX_IEEE803_MAC_ADDRESS_SIZE
 * @brief The size of the MAC address
 */
#define IX_IEEE803_MAC_ADDRESS_SIZE (6)

/**
 * @enum IxEthDBStatus
 * @brief Ethernet database status
 */
typedef enum /* IxEthDBStatus */
{
  IX_ETH_DB_SUCCESS = IX_SUCCESS,
  IX_ETH_DB_FAIL = IX_FAIL,
  IX_ETH_DB_INVALID_PORT,
  IX_ETH_DB_PORT_UNINITIALIZED,
  IX_ETH_DB_MAC_UNINITIALIZED,
  IX_ETH_DB_INVALID_ARG,
  IX_ETH_DB_NO_SUCH_ADDR, /**< Address not found for search or delete operations */
  IX_ETH_DB_NOMEM,        /**< Learning database memory full */
  IX_ETH_DB_BUSY,         /**< Learning database is busy */
  IX_ETH_DB_END           /**< Database browser passed the end */
} IxEthDBStatus;
    
/**
 * @ingroup IxEthDB
 *
 * @brief The IEEE 802.3 Ethernet MAC address structure.
 * 
 * The data should be packed with bytes xx:xx:xx:xx:xx:xx 
 * @note
 * The data must be packed in network byte order.
 */
typedef struct  
{
   UINT8 macAddress[IX_IEEE803_MAC_ADDRESS_SIZE];
} IxEthDBMacAddr;

/**
 * @ingroup IxEthDB
 *
 * @brief Definition of an IXP4xx port.
 */
typedef UINT32 IxEthDBPortId;

/**
 * @ingroup IxEthDB
 *
 * @brief Port dependency map definition
 */
typedef UINT32 IxEthDBPortMap;
    
/**
 * @ingroup IxEthDB
 *
 * @fn IxEthDBStatus ixEthDBInit(void)
 *
 * @brief initializes the Ethernet learning/filtering database
 *
 * @retval IX_ETH_DB_SUCCESS initialization was successful
 * @retval IX_ETH_DB_FAIL initialization failed (OSSL error)
 */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBInit(void);

/**
 * @ingroup IxEthDB
 *
 * @fn ixEthDBPortInit(IxEthDBPortId portID)
 *
 * @brief initializes a port
 *
 * @param portID ID of the port to be initialized
 *
 * This function is called automatically by the Ethernet Access
 * component top-level portInit() routine and should be manually
 * called for any user-defined port (any port that is not one of
 * the two Ethernet NPEs). 
 *
 * @see IxEthDBPortDefs.h for port definitions
 */
IX_ETH_DB_PUBLIC void ixEthDBPortInit(IxEthDBPortId portID);

/**
 * @ingroup IxEthDB
 *
 * @fn  ixEthDBPortEnable(IxEthDBPortId portID)
 *
 * @brief enable a port
 *
 * @param portID ID of the port to enable processing on
 *
 * This function is called automatically from the Ethernet Access component
 * top-level portEnable() routine and should be manually
 * called for any user-defined port (any port that is not one of
 * the two Ethernet NPEs). 
 *
 * @retval IX_ETH_DB_SUCCESS if enabling is successful
 * @retval IX_ETH_DB_FAIL if the enabling was not successful due to
 * a message handler error
 * @retval IX_ETH_DB_MAC_UNINITIALIZED the MAC address of this port was
 * not initialized (only for Ethernet NPEs)
 *
 * @pre ixEthDBPortAddressSet() needs to be called prior to enabling the port events
 * for Ethernet NPEs
 *
 * @see ixEthDBPortAddressSet()
 *
 * @see IxEthDBPortDefs.h for port definitions
 */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBPortEnable(IxEthDBPortId portID);

/**
 * @ingroup IxEthDB
 *
 * @fn ixEthDBPortDisable(IxEthDBPortId portID)
 *
 * @brief disable processing on a port
 *
 * @param portID ID of the port to disable processing on
 *
 * This function is called automatically from the Ethernet Access component
 * top-level portDisable() routine and should be manually
 * called for any user-defined port (any port that is not one of
 * the two Ethernet NPEs). 
 *
 * @retval IX_ETH_DB_SUCCESS if disabling is successful
 * @retval IX_ETH_DB_FAIL if the disabling was not successful due to
 * a message handler error
 */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBPortDisable(IxEthDBPortId portID);

/**
 * @ingroup IxEthDB
 *
 * @fn ixEthDBPortAddressSet(IxEthDBPortId portID, IxEthDBMacAddr *macAddr)
 *
 * @brief set a port MAC address
 *
 * @param portID ID of the port whose MAC address is set
 * @param macAddr port MAC address
 *
 * This function is to be called from the Ethernet Access component top-level
 * ixEthDBUnicastAddressSet(). Event processing cannot be enabled for a port
 * until its MAC address has been set.
 *
 * @retval IX_ETH_DB_SUCCESS MAC address was set successfully
 * @retval IX_ETH_DB_FAIL MAC address was not set due to a message handler failure
 * @retval IX_ETH_DB_INVALID_PORT if the port is not an Ethernet NPE
 *
 * @see IxEthDBPortDefs.h for port definitions
 */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBPortAddressSet(IxEthDBPortId portID, IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @fn ixEthDBFilteringStaticEntryProvision(IxEthDBPortId portId, IxEthDBMacAddr *macAddr)
 *
 * @brief  Statically Populate the Filtering database with a mac address.
 * Statically populate the filtering database with a mac address. The entry shall not be subject to aging.
 * If there is an entry (static or dynamic) with the corresponding MAC address on any port this entry shall take precedence.
 * All other entries for the MAC address shall be removed.
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 * 
 * @param portId : @ref IxEthDBPortId 
 * @param *macAddr : @ref IxEthDBMacAddr 
 * 
 * @return IxEthDBStatus
 * - IX_ETH_DB_SUCCESS
 * - IX_ETH_DB_FAIL : Failed to populate the database entry.
 * - IX_ETH_DB_INVALID_PORT : portId is invalid.
 * - IX_ETH_DB_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthDBStatus ixEthDBFilteringStaticEntryProvision(IxEthDBPortId portId, IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @fn ixEthDBFilteringDynamicEntryProvision(IxEthDBPortId portId, IxEthDBMacAddr *macAddr)
 *
 * @brief  Populate the filtering database with a dynamic mac address entry.
 *
 * Populate the filtering database with a dynamic mac address entry. This entry will be subject to normal 
 * aging function.
 * If there is an entry (static or dynamic) with the corresponding MAC address on any port this entry shall take precedence.
 * All other entries for the MAC address shall be removed.
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portId : @ref IxEthDBPortId 
 * @param *macAddr : @ref IxEthDBMacAddr 
 *
 * @return IxEthDBStatus
 * - IX_ETG_ACC_SUCCESS
 * - IX_ETH_DB_FAIL : Failed to populate the entry in the database.
 * - IX_ETH_DB_INVALID_PORT : portId is invalid.
 * - IX_ETH_DB_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthDBStatus ixEthDBFilteringDynamicEntryProvision(IxEthDBPortId portId, IxEthDBMacAddr *macAddr);




/**
 * @ingroup IxEthDB
 *
 * @fn  ixEthDBFilteringEntryDelete(IxEthDBMacAddr *macAddr)
 *
 * @brief Remove a MAC address entry from the filtering database.
 *
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param *macAddr : @ref IxEthDBMacAddr 
 *
 * @return IxEthDBStatus
 * - IX_ETH_DB_SUCCESS
 * - IX_ETH_DB_NO_SUCH_ADDR : Failed to remove the address as it was not in the table.
 *
 * <hr>
 */
IxEthDBStatus ixEthDBFilteringEntryDelete(IxEthDBMacAddr *macAddr);


/**
 * @ingroup IxEthDB
 *
 * @fn ixEthDBFilteringPortSearch(IxEthDBPortId portId, IxEthDBMacAddr *macAddr)
 *
 *
 * @brief  Search the Filtering data base for portId for the macAddr.
 *
 * This functions Search the Filtering data base for a specific port. If the mac address is
 * found on the port the function returns IX_ETH_DB_SUCCESS. If the port does not contain an entry 
 * for the port it returns IX_ETH_DB_FAIL
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portId : @ref IxEthDBPortId 
 * @param *macAddr : @ref IxEthDBMacAddr 
 *
 * @return IxEthDBStatus
 * - IX_ETH_DB_SUCCESS : The MAC address was found.
 * - IX_ETH_DB_NO_SUCH_ADDR : The MAC address was not found.
 * - IX_ETH_DB_INVALID_PORT : portId is invalid.
 * - IX_ETH_DB_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthDBStatus ixEthDBFilteringPortSearch(IxEthDBPortId portId, IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @fn ixEthDBFilteringDatabaseSearch(IxEthDBPortId *portId, IxEthDBMacAddr *macAddr)
 *
 *
 * @brief  Search the filtering database for a MAC address. 
 *
 *   Search the filtering database for a MAC address.  The function returns the portId for the 
 * MAC address. If no match is found the function returns IX_ETH_DB_FAIL. The portId returned is only
 * valid if the function finds a match.
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param *portId : @ref IxEthDBPortId
 * @param *macAddr : @ref IxEthDBMacAddr
 *
 * @retval IX_ETH_DB_SUCCESS The MAC address was found.
 * @retval IX_ETH_DB_NO_SUCH_ADDR The MAC address was not found.
 *
 * <hr>
 */
IxEthDBStatus ixEthDBFilteringDatabaseSearch(IxEthDBPortId *portId, IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @fn ixEthDBFilteringPortUpdatingSearch(IxEthDBPortId *portID, IxEthDBMacAddr *macAddr)
 *
 * @brief Search the filtering database for a MAC address, return the port and reset the age
 *
 * This function searches the filtering database for the given MAC address. If found, the given
 * portID location is set to the portID found in the database and the MAC address age is reset to 0.
 * The portID is valid only if the function has found a match.
 *
 * - Reentrant      - yes
 * - ISR Callable   - no
 *
 * @retval IX_ETH_DB_SUCCESS the MAC address was found
 * @retval IX_ETH_DB_NO_SUCH_ADDR the MAC address was not found
 */
IxEthDBStatus ixEthDBFilteringPortUpdatingSearch(IxEthDBPortId *portID, IxEthDBMacAddr *macAddr);

/**
 * @ingroup IxEthDB
 *
 * @def IX_ETH_DB_MAINTENANCE_TIME
 *
 * @brief The @ref ixEthDBDatabaseMaintenance must be called by the user at a frequency of 
 * IX_ETH_DB_MAINTENANCE_TIME
 *
 */
#define IX_ETH_DB_MAINTENANCE_TIME (1 * 60) /* 1 Minute */

/**
 * @ingroup IxEthDB
 *
 * @def IX_ETH_DB_LEARNING_ENTRY_AGE_TIME
 *
 * @brief The define specifies the filtering database age entry time. Static entries older than
 * IX_ETH_DB_LEARNING_ENTRY_AGE_TIME +/- IX_ETH_DB_MAINTENANCE_TIME shall be removed.
 *
 */
#define IX_ETH_DB_LEARNING_ENTRY_AGE_TIME (15 * 60 ) /* 15 Mins */



/**
 * @ingroup IxEthDB
 *
 * @fn ixEthDBPortAgingDisable(IxEthDBPortId port)
 *
 * @brief  Disable the aging function for a specific port.
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portId : @ref IxEthDBPortId 
 *
 * @return IxEthDBStatus
 * - IX_ETH_DB_SUCCESS 
 * - IX_ETH_DB_INVALID_PORT : portId is invalid.
 * - IX_ETH_DB_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthDBStatus ixEthDBPortAgingDisable(IxEthDBPortId port);

/**
 * @ingroup IxEthDB
 *
 * @fn ixEthDBPortAgingEnable(IxEthDBPortId port)
 *
 * @brief  Enable the aging function for a specific port.
 * 
 * Enable the aging of dynamic source learned mac address entries in the filtering database.
 * 
 * @note The aging function relies on the @ref ixEthDBDatabaseMaintenance being called with a period of 
 * @ref IX_ETH_DB_MAINTENANCE_TIME seconds.
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portId : @ref IxEthDBPortId 
 *
 * @return IxEthDBStatus
 * - IX_ETH_DB_SUCCESS 
 * - IX_ETH_DB_INVALID_PORT : portId is invalid.
 * - IX_ETH_DB_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthDBStatus ixEthDBPortAgingEnable(IxEthDBPortId port);

/**
 * @ingroup IxEthDB
 *
 * @fn ixEthDBDatabaseMaintenance(void)
 *
 * @brief Required in order to do maintenance operations on the filtering database.
 * 
 * In order to do maintenance functions on the database this function must be called with a period 
 * of @IX_ETH_DB_MAINTENANCE_TIME seconds. It must be called regardless of whether learning function is 
 * enabled or not.
 * 
 *
 * - Reentrant    - no
 * - ISR Callable - no
 * 
 * @param  none
 * 
 * @return void
 *
 * <hr>
 */
void ixEthDBDatabaseMaintenance(void);


/**
 * @ingroup IxEthDB
 *
 * @fn ixEthDBFilteringDatabaseShow(IxEthDBPortId  portId)
 *
 * @brief This function displays the Mac Ethernet MAC address filtering tables. 
 * It displays items such as MAC address/port, dynamic/static, entry age for 
 * all Mac Ethernet ports. It uses printf to display the details.
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portId : @ref IxEthDBPortId 
 *
 * @return IxEthDBStatus
 * - IX_ETH_DB_SUCCESS 
 * - IX_ETH_DB_INVALID_PORT : portId is invalid.
 * - IX_ETH_DB_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthDBStatus ixEthDBFilteringDatabaseShow(IxEthDBPortId  portId);

/** 
 * @ingroup IxEthDB
 *
 * @fn ixEthDBFilteringDatabaseShowAll(void)
 *
 * @brief Displays the MAC address recorded in the filtering database, grouped by port ID
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 */
void ixEthDBFilteringDatabaseShowAll(void);

/**
 * @}
 */

#endif /* IxEthDB_H */

