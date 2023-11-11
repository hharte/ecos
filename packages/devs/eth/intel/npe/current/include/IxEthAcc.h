/** @file    IxEthAcc.h
 *
 * @brief this file contains the public API of @ref IxEthAcc component
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

#ifndef IxEthAcc_H
#define IxEthAcc_H

#include <IxOsBuffMgt.h>
#include <IxTypes.h>

/**
 * @defgroup IxEthAcc IXP4xx Ethernet Access (IxEthAcc) API
 *
 * @brief ethAcc is a library that does provides access to the internal IXP4xx 10/100Bt Ethernet MACs.
 *
 *@{
 */

/**
 * @ingroup IxEthAcc
 * @enum IxEthAccStatus
 * @brief This is an enum to define the Ethernet Access status
 */
typedef enum /* IxEthAccStatus */
{
  IX_ETH_ACC_SUCCESS = IX_SUCCESS,
  IX_ETH_ACC_FAIL = IX_FAIL,
  IX_ETH_ACC_INVALID_PORT,
  IX_ETH_ACC_PORT_UNINITIALIZED,
  IX_ETH_ACC_MAC_UNINITIALIZED,
  IX_ETH_ACC_INVALID_ARG,
  IX_ETH_TX_Q_FULL,
  IX_ETH_ACC_NO_SUCH_ADDR
} IxEthAccStatus;

/**
 * @ingroup IxEthAcc
 * @enum IxEthAccPortId
 * @brief This is an emum to define the IXP4xx Mac Ethernet device.
 */
typedef enum  
{
	IX_ETH_PORT_1 = 0, /**< Ethernet Port 1 */
	IX_ETH_PORT_2 = 1  /**< Ethernet port 2 */
} IxEthAccPortId;

/*
 * @ingroup IxEthAcc
 *
 * @def IX_ETH_ACC_NUMBER_OF_PORTS
 *
 * @brief  Defines related to the number of NPE's and mapping between PortId and
 NPE.
 *
 */
#define IX_ETH_ACC_NUMBER_OF_PORTS (2)

/*
 * @ingroup IxEthAcc
 *
 * @def IX_IEEE803_MAC_ADDRESS_SIZE
 *
 * @brief  Defines the size of the MAC address
 NPE.
 *
 */
#define IX_IEEE803_MAC_ADDRESS_SIZE (6)

/**
 * @ingroup IxEthAcc
 *
 * @struct IxEthAccMacAddr 
 * @brief The IEEE 802.3 Ethernet MAC address structure.
 * 
 * The data should be packed with bytes xx:xx:xx:xx:xx:xx 
 * @note
 * The data must be packed in network byte order.
 */
typedef struct  
{
   UINT8 macAddress[IX_IEEE803_MAC_ADDRESS_SIZE];
} IxEthAccMacAddr;

/**
 * @ingroup IxEthAcc
 * @def IX_ETH_ACC_NUM_TX_PRIORITIES
 * @brief The number of transmit priorities
 * 
 */
#define IX_ETH_ACC_NUM_TX_PRIORITIES (8)

/**
 * @ingroup IxEthAcc
 * @enum IxEthAccTxPriority
 * @brief enum to submit a frame with relative priority.
 * 
 */
typedef enum  
{
	IX_ETH_ACC_TX_PRIORITY_0 = 0 ,  /**< Lowest Priorty submission */
	IX_ETH_ACC_TX_PRIORITY_1 = 1,
	IX_ETH_ACC_TX_PRIORITY_2 = 2,
	IX_ETH_ACC_TX_PRIORITY_3 = 3,
	IX_ETH_ACC_TX_PRIORITY_4 = 4,
	IX_ETH_ACC_TX_PRIORITY_5 = 5,
	IX_ETH_ACC_TX_PRIORITY_6 = 6,
	IX_ETH_ACC_TX_PRIORITY_7 = 7, /**< Highest priority submission */

	IX_ETH_ACC_TX_DEFAULT_PRIORITY = IX_ETH_ACC_TX_PRIORITY_0 /**< By default send all 
								 pkts with lowest priority */
} IxEthAccTxPriority;

/*
 * @ingroup IxEthAcc
 * @enum IxEthAccDuplexMode
 * @brief Definition to provision the duplex mode of the MAC. 
 * 
 */
typedef enum
{
    IX_ETH_ACC_FULL_DUPLEX, /**< Full duplex operation of the MAC */
    IX_ETH_ACC_HALF_DUPLEX  /**< Half duplex operation of the MAC */
} IxEthAccDuplexMode;


/**
 * @ingroup IxEthAcc
 *
 * @brief This defines the required size of MBUF's submitted to the frame receive service.
 *
 */
#define IX_ETHACC_RX_MBUF_MIN_SIZE (2048)

/**
 * @ingroup IxEthAcc
 *
 * @brief This defines the maximum MII address of any attached PHYs 
 * 
 * Max number for PHY address is 31, add on for range checking.
 *
 */
#define IXP425_ETH_ACC_MII_MAX_ADDR   32

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccInit(void)
 * 
 * @brief Initialize the Ethernet Access Service.
 * 
 * Initialize the IXP4xx Ethernet Access Service.
 * 
 * - Reentrant    - no
 * - ISR Callable - no
 * 
 * This should be called once per module initialization.
 * @pre
 *   The NPE must first be downloaded with the required microcode which supports all
 *   required features.
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_FAIL	:  Service has failed to initialize.
 *
 * <hr>
 */
IxEthAccStatus ixEthAccInit(void);

/**
 * @ingroup IxEthAcc
 * 
 * @fn ixEthAccPortInit( IxEthAccPortId portId)
 *
 * @brief Initialize an Ethernet MAC Port. 
 * 
 * Initialize the NPE/Ethernet MAC hardware.
 * Verify NPE downloaded and operational.
 * The NPE shall be available for usage once this API returns.
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * This should be called once per mac device. 
 * The NPE/MAC shall be in disabled state after init.
 *
 * @pre 
 *   The component must be initialized via @ref ixEthAccInit
 *   The NPE must first be downloaded with the required microcode which supports all 
 *   required features.
 * 
 * Dependant on Services: (Must be initialized before using this service may be initialized)
 * 	ixNPEmh - NPE Message handling service.
 * 	ixQmgr	- Queue Manager component.
 *
 * @param portId :  @ref IxEthAccPortId
 *
 * @return IxEthAccStatus 
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_FAIL : The NPE processor has failed to initialize.
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * 
 * <hr>
 */
IxEthAccStatus ixEthAccPortInit(IxEthAccPortId portId);


/*************************************************************************

 #####     ##     #####    ##            #####     ##     #####  #    #
 #    #   #  #      #     #  #           #    #   #  #      #    #    #
 #    #  #    #     #    #    #          #    #  #    #     #    ######
 #    #  ######     #    ######          #####   ######     #    #    #
 #    #  #    #     #    #    #          #       #    #     #    #    #
 #####   #    #     #    #    #          #       #    #     #    #    #

*************************************************************************/


/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortTxFrameSubmit( 
    IxEthAccPortId portId,
    IX_MBUF *buffer, 
    IxEthAccTxPriority priority)
 * 
 * @brief This function shall be used to submit MBUFs buffers for transmission on a particular MAC device. 
 *
 * This function shall be used to submit MBUFs buffers for transmission on a particular MAC device.
 * The buffer shall be freed automatically once the frame has been transmitted. The only alterations
 * made to the buffer are assocuated with the next packet chain pointer. This is used to internally 
 * queue frames in the service. Buffers shall be not queued for transmission if the port is disabled.
 * The port can be enabled using @ref ixEthAccPortEnable
 *
 * 
 * - Reentrant    - yes
 * - ISR Callable - yes
 *
 *
 * @pre 
 *  @ref ixEthAccPortTxDoneCallbackRegister must be called to register a function to allow this service to
 *   return the buffer to the calling service. 
 * 
 * @note 
 *  If the buffer submit fails for any reason the user has retained ownership of the buffer.
 *
 * @param portId - MAC port ID to transmit Ethernet frame on.
 * @param *mbuf - pointer to an MBUF formatted buffer. Chained buffers are supported for transmission.
 * @param priority - @ref IxEthAccTxPriority
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_FAIL  : Failed to queue frame for transmission. 
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */

IxEthAccStatus ixEthAccPortTxFrameSubmit( 
    IxEthAccPortId portId,
    IX_MBUF *buffer, 
    IxEthAccTxPriority priority
);

/**
 * @ingroup IxEthAcc
 *
 * @brief Function prototype for Ethernet Tx Buffer Done callback. Registered 
 *  via @ref ixEthAccTxBufferDoneCallbackRegister 
 * 
 * This function is called once the previously submitted buffer is no longer required by this service.
 * It may be returned upon successful transmission of the frame or shutdown of port prior to submission.
 * The calling of this registered function is not a guarantee of successful transmission of the buffer.
 *
*  
 * - Reentrant    - yes , The user provided function should be reentrant.
 * - ISR Callable - yes , The user provided function must be callable from an ISR.
 *
 *
 * @note
 *Calling Context: 
 *   This callback is called in the context of the queue manager dispatch loop @ref ixQmgrgrDispatcherLoopRun
 *   within the @ref IxQMgrAPI component. The calling context may be from interrupt or high priority thread. 
 *   The decision is system specific.
 *
 * @param callbackTag - This tag is that provided when the callback was registered for a particular MAC 
 * via @ref ixEthAccRxCallbackRegister. It allows the same callback to be used for multiple MACs.
 * @param mbuf - Pointer to the Tx mbuf descriptor. 
 * 
 * @return void
 *
 * <hr>
 */
typedef void (*IxEthAccPortTxDoneCallback) ( UINT32 callbackTag, IX_MBUF *buffer );



/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortTxDoneCallbackRegister( IxEthAccPortId portId, 
					   IxEthAccPortTxDoneCallback txCallbackFn, 
 					   UINT32 callbackTag)
 *
 * @brief This function registers a callback function to facilitate the return of transmit buffers to the  
 * user.
 * 
 * This function registers the transmit buffer done function callback for a particular port.
 *
 * The registered callback function is called once the previously submitted buffer is no longer required by this service.
 * It may be returned upon successful transmission of the frame or  shutdown of port prior to submission.
 * The calling of this registered function is not a guarantee of successful transmission of the buffer.
 *
 * If called several times the latest callback shall be registered for a particular port.
 *
 * - Reentrant    - yes
 * - ISR Callable - yes
 *
 * @pre
 *	The port must be initialized via @ref ixEthAccPortInit
 *
 *
 * @param portId - Register callback for a particular MAC device.
 * @param txCallbackFn - @ref IxEthAccTxBufferDoneCallbackFn - Function to be called to return transmit buffers to the user.
 * @param callbackTag -  This tag shall be provided to the callback function.
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS 
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 * - IX_ETH_ACC_INVALID_ARG : An argument other than portId is invalid.
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortTxDoneCallbackRegister( IxEthAccPortId portId, 
					   IxEthAccPortTxDoneCallback txCallbackFn, 
 					   UINT32 callbackTag);



/**
 * @ingroup IxEthAcc
 *
 * @brief Function prototype for Ethernet Frame Rx callback. Registered via @ref ixEthAccRxCallbackRegister 
 * 
 * It is the responsibility of the user function to free any MBUF's which it receives.
 *  
 * - Reentrant    - yes , The user provided function should be reentrant.
 * - ISR Callable - yes , The user provided function must be callable from an ISR.
 * @par
 *
 * <b>Calling Context: 
 * @par
 *   This callback is called in the context of the queue manager dispatch loop @ref ixQmgrgrDispatcherLoopRun
 *   within the @ref IxQMgrAPI component. The calling context may be from interrupt or high priority thread. 
 *   The decision is system specific.
 *
 *
 * @param callbackTag - This tag is that provided when the callback was registered for a particular MAC 
 * via @ref ixEthAccRxCallbackRegister. It allows the same callback to be used for multiple MACs.
 * @param mbuf - Pointer to the Rx mbuf descriptor. 
 * @param portId - ID of the port which received the buffer (set to IX_ETH_NUMBER_OF_PORTS if not found)
 * 
 * @return void
 *
 * <hr>
 */

typedef void (*IxEthAccPortRxCallback) (UINT32 callbackTag, IX_MBUF *buffer, IxEthAccPortId portId);




/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortRxCallbackRegister( IxEthAccPortId portId, IxEthAccPortRxCallback rxCallbackFn, UINT32 callbackTag)
 *
 * @brief The function registered through this function shall be called once per received Ethernet frame.
 *
 *
 * This function will dispatch a predefined number of frames to the user level
 * via the provided function. The invocation shall be made for each
 * frame dequeued from the Ethernet QM queue. The user is required to free any MBUF's 
 * supplied via this callback. In addition the registered callback must free up MBUF's
 * from the recieve free queue when the port is disabled 
 * 
 * If called several times the latest callback shall be registered for a particular port.
 *
 * - Reentrant    - yes
 * - ISR Callable - yes
 *
 * @pre The port must be enabled (see @ref ixEthAccPortEnable) before this callback will be invoked
 *
 * @param portId - Register callback for a particular MAC device.
 * @param rxCallbackFn - @ref IxEthAccRxCallbackFn - Function to be called when Ethernet frames are availble.
 * @param callbackTag -  This tag shall be provided to the callback function.
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS 
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 * - IX_ETH_ACC_INVALID_ARG : An argument other than portId is invalid.
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortRxCallbackRegister( IxEthAccPortId portId, IxEthAccPortRxCallback rxCallbackFn, UINT32 callbackTag);




/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortRxFreeReplenish( IxEthAccPortId portId, IX_MBUF *buffer)
 *
 * @brief This function provides buffers for the Ethernet receive path. 
 *
 * This component does not have a buffer management mechanisms built in. All Rx buffers must be supplied to it
 * via this interface. 
 *
 * - Reentrant    - yes
 * - ISR Callable - yes
 *
 * @note
 * If the buffer replenish operation fails it is the responsibility of the user to free the buffer.
 * @note
 * Sufficient buffers must be supplied to the component to maintain receive throughput and avoid rx buffer underflow conditions.
 * To meet this goal, It is expected that the user preload the component with a sufficent number of buffers prior to enabling the
 * NPE Ethernet receive path.
 *
 * @param portId - Provide buffers only to specific Rx MAC. 
 * @param mbuf - Provide an MBUF to the Ethernet receive mechanism.
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_FAIL : Buffer has was not able to queue the buffer in the receive service.
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortRxFreeReplenish( IxEthAccPortId portId, IX_MBUF *buffer);



/***************************************************************

  ####    ####   #    #   #####  #####    ####   #
 #    #  #    #  ##   #     #    #    #  #    #  #
 #       #    #  # #  #     #    #    #  #    #  #
 #       #    #  #  # #     #    #####   #    #  #
 #    #  #    #  #   ##     #    #   #   #    #  #
  ####    ####   #    #     #    #    #   ####   ######


         #####   #         ##    #    #  ######
         #    #  #        #  #   ##   #  #
         #    #  #       #    #  # #  #  #####
         #####   #       ######  #  # #  #
         #       #       #    #  #   ##  #
         #       ######  #    #  #    #  ######

***************************************************************/

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortEnable(IxEthAccPortId portId)
 *
 * @brief Enable a port
 * 
 * This enables an Ethernet port for both Tx and Rx. 
 *
 *
 * - Reentrant    - yes
 * - ISR Callable - yes
 *
 * @pre The port must first be initialized via @ref ixEthAccPortInit and the MAC address 
 * must be set using @ref ixEthAccUnicastMacAddressSet before enabling it
 *
 * 
 * @param  portId : @ref IxEthAccPortId : Port id to act upon.
 * 
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS 
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is not initialized
 * - IX_ETH_ACC_MAC_UNINITIALIZED : port MAC address is not initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortEnable(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortDisable(IxEthAccPortId portId)
 *
 * @brief Disable a port
 * 
 * This disables an Ethernet port for both Tx and Rx. 
 * Free MBufs are returned to the user via the registered callback when the port is disabled 
 *
 * - Reentrant    - yes
 * - ISR Callable - yes
 *
 * @pre The port must be enabled with @ref ixEthAccPortEnable, otherwise this
 * function has no effect
 *
 * @param  portId : @ref IxEthAccPortId : Port id to act upon.
 * 
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS 
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is not initialized
 * - IX_ETH_ACC_MAC_UNINITIALIZED : port MAC address is not initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortDisable(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortEnabledQuery(IxEthAccPortId portId, BOOL *enabled)
 *
 * @brief Get the enabled state of a port.
 *
 * Return the enabled state of the port.
 *
 * - Reentrant    - yes
 * - ISR Callable - yes
 *
 * @pre The port must first be initialized via @ref ixEthAccPortInit
 *
 * @param  portId : @ref IxEthAccPortId : Port id to act upon.
 * @param  enabled : BOOL : location to store the state of the port
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortEnabledQuery(IxEthAccPortId portId, BOOL *enabled);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortPromiscuousModeClear(IxEthAccPortId portId)
 *
 * @brief Put the Ethernet MAC device in non-promiscuous mode.
 * 
 * In non-promiscuous mode the MAC will filter all frames other than 
 * destination MAC address which matches the following criteria:
 * - Unicast address provisioned via @ref ixEthAccUnicastMacAddressSet
 * - All broadcast frames.
 * - Multicast addresses provisioned via @ref ixEthAccMulticastAddressJoin
 *
 * See also: @ref ixEthAccPortPromiscuousModeSet
 * 
 * - Reentrant    - yes
 * - ISR Callable - no
 * 
 * @param portId - Ethernet port id.
 * 
 * @return IxEthAccStatus 
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortPromiscuousModeClear(IxEthAccPortId portId);


/**
 * @ingroup IxEthAcc
 *
 * @fn  ixEthAccPortPromiscuousModeSet(IxEthAccPortId portId)
 *
 * @brief Put the MAC device in promiscuous mode.
 * 
 * If the device is in promiscuous mode then all all received frames shall be forwared
 * to the NPE for processing.
 *
 * See also: @ref ixEthAccPortPromiscuousModeClear
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 * 
 * @param portId - Ethernet port id.
 * 
 * @return IxEthAccStatus 
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthAccStatus  ixEthAccPortPromiscuousModeSet(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortUnicastMacAddressSet(      IxEthAccPortId portId,
                                                  IxEthAccMacAddr *macAddr)
 *
 * @brief Configure unicast MAC address for a particular port
 *
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portId - Ethernet port id.
 * @param *macAddr - Ethernet Mac address.
 *
 * @return IxEthAccStatus 
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortUnicastMacAddressSet(      IxEthAccPortId portId,
                                                  IxEthAccMacAddr *macAddr);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortUnicastMacAddressGet(	IxEthAccPortId portId, 
					IxEthAccMacAddr *macAddr)
 *
 * @brief Get unicast MAC address for a particular MAC port 
 *
 * @pre
 * The MAC address must first be set via @ref ixEthAccMacPromiscuousModeSet
 * If the MAC address has not been set, the function will return a 
 * IX_ETH_ACC_MAC_UNINITIALIZED status
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portId - Ethernet port id.
 * @param *macAddr - Ethernet MAC address.
 *
 * @return  IxEthAccStatus
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortUnicastMacAddressGet(	IxEthAccPortId portId, 
					IxEthAccMacAddr *macAddr);




/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortMulticastAddressJoin(      IxEthAccPortId portId,
                                             IxEthAccMacAddr *macAddr)
 *
 * @brief ADD a multicast address to the MAC address table.
 *
 *  ADD a multicast address to the MAC address table.
 *  @note
 *  Due to the operation of the Ethernet MAC multicast filtering mechanism, frames which do not 
 *  have a multicast destination address which were provisioned via this API may be forwarded 
 *  to the NPE's. This is a result of the hardware comparison  algorithm used in the destination mac address logic
 *  within the Ethernet MAC. 
 *
 *  See Also: IXP4xx hardware development manual.
 * 
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portId - Ethernet port id.
 * @param *macAddr - Ethernet Mac address.
 *
 * @return IxEthAccStatus 
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_FAIL : Error writing to the MAC registers
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortMulticastAddressJoin(      IxEthAccPortId portId,
                                             IxEthAccMacAddr *macAddr);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortMulticastAddressLeave( IxEthAccPortId portId,
                                         IxEthAccMacAddr *macAddr)
 *
 * @brief Remove a multicast address to the MAC address table.
 *
 *  Remove a multicast address to the MAC address table.
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portId - Ethernet port id.
 * @param *macAddr - Ethernet Mac address.
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_NO_SUCH_ADDR :  Failed if MAC address was not in the table.
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortMulticastAddressLeave( IxEthAccPortId portId,
                                         IxEthAccMacAddr *macAddr);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortUnicastAddressShow(IxEthAccPortId portId)
 *
 * @brief Display unicast address has  been configured using @ref ixEthAccUnicastMacAddressSet
 * Display unicast address has  been configured using @ref ixEthAccUnicastMacAddressSet
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portId - Ethernet port id.
 *
 * @return void
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortUnicastAddressShow(IxEthAccPortId portId);


/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortMulticastAddressShow( IxEthAccPortId portId)
 *
 * @brief Display multicast address which have been configured using @ref ixEthAccMulticastAddressJoin
 * Display multicast address which have been configured using @ref ixEthAccMulticastAddressJoin
 * 
 * - Reentrant    - yes
 * - ISR Callable - no
 * 
 * @param portId - Ethernet port id.
 *
 * @return void
 *
 * <hr>
 */
void ixEthAccPortMulticastAddressShow( IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortDuplexModeSet( IxEthAccPortId portId, IxEthAccDuplexMode mode )
 *
 * @brief  Set the duplex mode for the MAC.
 *
 * Configure the IXP4xx MAC to either full or half duplex. 
 *
 * @note 
 * The configuration should match that provisioned on the PHY.
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portId : @ref IxEthAccPortId
 * @param mode : @ref IxEthAccDuplexMode
 *
 * @return IxEthAccStatus 
 * - IX_ETH_ACC_SUCCESS 
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortDuplexModeSet( IxEthAccPortId portId, IxEthAccDuplexMode mode );

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortDuplexModeGet( IxEthAccPortId portId, IxEthAccDuplexMode *mode )
 *
 * @brief  Get the duplex mode for the MAC.
 *
 * return the duplex configuration of the IXP4xx MAC.
 *
 * @note
 * The configuration should match that provisioned on the PHY.
 * See @ref ixEthAccDuplexModeSet
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @param portId : @ref IxEthAccPortId
 * @param *mode : @ref IxEthAccDuplexMode 
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS 
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 *
 */
IxEthAccStatus ixEthAccPortDuplexModeGet( IxEthAccPortId portId, IxEthAccDuplexMode *mode );





/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortTxFrameAppendFCSEnable( IxEthAccPortId portId)
 *
 * @brief  Enable the appending of Ethernet FCS to all frames submitted to this port
 * 
 * Enable the appending of Ethernet FCS to all frames submitted to this port. This is the default 
 * behavior of the access component.
 * Do not change this behaviour while the port is enabled.
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portId : @ref IxEthAccPortId 
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS 
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortTxFrameAppendFCSEnable( IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortTxFrameAppendFCSDisable( IxEthAccPortId portId)
 *
 * @brief  Disable the appending of Ethernet FCS to all frames submitted to this port.
 * 
 * Disable the appending of Ethernet FCS to all frames submitted to this port. This is not the default
 * behavior of the access component.
 * Note: Since the FCS is not appended to the frame it is expected that the frame submitted to the 
 * component includes a valid FCS at the end of the data, although this will not be validated.
 * The component shall forward the frame to the Ethernet MAC WITHOUT modification.
 * Do not change this behaviour while the port is enabled.
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portId : @ref IxEthAccPortId 
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS 
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortTxFrameAppendFCSDisable( IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortRxFrameAppendFCSEnable( IxEthAccPortId portId)
 *
 * @brief Forward frames with FCS included in the receive buffer to the user.
 *
 * Enable the appending of Ethernet FCS to all frames submitted to this port. This is the default
 * behavior of the access component.
 * The Frame length recieved will include the FCS. ie. A minimum sized ethernet frame shall have a 
 * length of 64bytes.
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portId : @ref IxEthAccPortId
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS 
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortRxFrameAppendFCSEnable( IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccPortRxFrameAppendFCSDisable( IxEthAccPortId portId)
 *
 * @brief  Disable the appending of Ethernet FCS to all frames submitted to this port.
 *
 *
 * Do not forward the FCS portion of the received Ethernet frame to the user. The FCS is striped
 * from the receive buffer. Frame FCS validity checks are still carried out on all received frames.
 * This is the default behavior of the component.
 * Do not change this behaviour while the port is enabled.
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @param portId : @ref IxEthAccPortId
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS 
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccPortRxFrameAppendFCSDisable( IxEthAccPortId portId);




/**
 * @ingroup IxEthAcc
 *
 * @enum IxEthAccTxSchedulerDiscipline
 *
 * @brief  Definition for the port transmit scheduling discipline
 * Definition for the port transmit scheduling discipline
 * 
 *  Select the port transmit scheduling discipline
 * - FIFO : No Priority : In this configuration all frames submitted to the access component 
 *                        shall be sub-mitted to the MAC hardware in the strict order in which 
 *                        it was received.
 * - FIFO : Priority : This shall be a very simple priority mechanism all submitted frames at a 
 *                     higher prior-ity shall be forwarded to Ethernet MAC for transmission before 
 *                     lower priorities. There shall be no fairness mechanisms applied across 
 *                     different priorities. Higher priority frames could starve lower priority 
 *                     frames indefinitely.
 */
typedef  enum 
{
        FIFO_NO_PRIORITY,
        FIFO_PRIORITY
}IxEthAccTxSchedulerDiscipline;


/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccTxSchedulingDisciplineSet( IxEthAccPortId portId, IxEthAccTxSchedulerDiscipline sched)
 *
 * @brief Set the port scheduling to one of @ref IxEthAccTxSchedulerDiscipline
 * Set the port scheduling to one of @ref IxEthAccTxSchedulerDiscipline
 *
 * The default behavior of the component is @ref FIFO_NO_PRIORITY.
 *
 * - Reentrant    - yes
 * - ISR Callable - yes
 *
 * @pre
 *
 *
 * @param portId : @ref IxEthAccPortId 
 * @param sched : @ref IxEthAccTxSchedulerDiscipline 
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS : Set appropriate discipline.
 * - IX_ETH_ACC_FAIL :  Invalid/unsupported discipline.
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccTxSchedulingDisciplineSet( IxEthAccPortId portId, IxEthAccTxSchedulerDiscipline sched);


  
/*********************************************************************************
  ####    #####    ##     #####     #     ####    #####     #     ####    ####
 #          #     #  #      #       #    #          #       #    #    #  #
  ####      #    #    #     #       #     ####      #       #    #        ####
      #     #    ######     #       #         #     #       #    #            #
 #    #     #    #    #     #       #    #    #     #       #    #    #  #    #
  ####      #    #    #     #       #     ####      #       #     ####    ####
**********************************************************************************/


/**
 * @ingroup IxEthAcc
 *
 * @brief This struct defines the statistics returned by this component.
 * The component returns MIB2 EthObj variables which should are obtained from the 
 * hardware or maintained by this component. 
 *
 *
 */
typedef struct   
{
    UINT32 dot3StatsAlignmentErrors;
    UINT32 dot3StatsFCSErrors;
    UINT32 dot3StatsFrameTooLongs;
    UINT32 dot3StatsInternalMacReceiveErrors;
    UINT32 LearnedEntryDiscards;
    UINT32 dot3StatsSingleCollisionFrames;
    UINT32 dot3StatsMultipleCollisionFrames;
    UINT32 dot3StatsDeferredTransmissions;
    UINT32 dot3StatsLateCollisions;
    UINT32 dot3StatsExcessiveCollsions;
    UINT32 dot3StatsInternalMacTransmitErrors;
    UINT32 dot3StatsCarrierSenseErrors;
}IxEthEthObjStats;



/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMibIIStatsGet(IxEthAccPortId portId ,IxEthEthObjStats *retStats )
 *
 * @brief  Return the statistics maintained for a port.
 *  Return the statistics maintained for a port.
 *
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @pre
 *
 *
 * @param portId @ref IxEthAccPortId 
 * @param retStats @ref IxEthEthObjStats
 * @note Please note the user is responsible for cache coheriency of the retStat
 * buffer. The data is actually populated via the NPE's. As such cache safe
 * memory should be used in the retStats argument.
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_FAIL : Invalid arguments.
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccMibIIStatsGet(IxEthAccPortId portId ,IxEthEthObjStats *retStats );

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMibIIStatsGetClear(IxEthAccPortId portId, IxEthEthObjStats *retStats)
 * 
 * @brief  Return and clear the statistics maintained for a port. 
 *  Return and clear the statistics maintained for a port. 
 *
 *
 * - Reentrant    - yes
 * - ISR Callable - yes
 *
 * @pre
 *
 * @param portId @ref IxEthAccPortId  
 * @param retStats @ref IxEthEthObjStats
 * @note Please note the user is responsible for cache coheriency of the retStats
 * buffer. The data is actually populated via the NPE's. As such cache safe
 * memory should be used in the retStats argument.
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_FAIL : invalid arguments.
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccMibIIStatsGetClear(IxEthAccPortId portId, IxEthEthObjStats *retStats);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMibIIStatsClear(IxEthAccPortId portId)
 *
 *
 * @brief   Clear the statistics maintained for a port.
 * Clear the statistics maintained for a port.
 *
 *
 * - Reentrant    - yes
 * - ISR Callable - no
 *
 * @pre
 *
 * @param portId @ref IxEthAccPortId
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_FAIL : Invalid arguments.
 * - IX_ETH_ACC_INVALID_PORT : portId is invalid.
 * - IX_ETH_ACC_PORT_UNINITIALIZED : portId is un-initialized
 *
 * <hr>
 */
IxEthAccStatus ixEthAccMibIIStatsClear(IxEthAccPortId portId);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccStatsShow(IxEthAccPortId portId)
 *
 *
 * @brief Display a ports statistics on the standard io console using printf. 
 * Display a ports statistics on the standard io console using printf. 
 *  
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @pre
 *
 * @param portId @ref IxEthAccPortId
 *
 * @return void
 *
 * <hr>
 */
void ixEthAccStatsShow(IxEthAccPortId portId);


/**
 * @ingroup IxEthAcc
 *
 * @fn  ixEthAccMiiPhyScan(BOOL phyPresent[])
 *
 *
 * @brief Scan the MDIO bus for PHYs
 *  This function scans PHY addresses 0 through 31, and sets phyPresent[n] to 
 *  TRUE if a phy is discovered at address n. 
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @pre The MAC on Ethernet Port 2 (NPE C) must be initialised, and generating the MDIO clock.
 *   
 * @param phyPresent
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_FAIL : invalid arguments.
 *
 * <hr>
 */
IxEthAccStatus ixEthAccMiiPhyScan(BOOL phyPresent[]);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMiiPhyConfig(UINT32 phyAddr,
				    BOOL speed100, 
				    BOOL fullDuplex, 
				    BOOL autonegotiate)
 *
 *
 * @brief Configure a PHY
 *   Configure a PHY's speed, duplex and autonegotiation status
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @pre The MAC on Ethernet Port 2 (NPE C) must be initialised, and generating the MDIO clock.
 *   
 * @param phyAddr
 * @param speed100: set to TRUE for 100Mbit/s operation, FALSE for 10Mbit/s
 * @param fullDuplex: set to TRUE for Full Duplex, FALSE for Half Duplex
 * @param autonegotiate: set to TRUE to enable autonegotiation
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_FAIL : invalid arguments.
 *
 * <hr>
 */
IxEthAccStatus ixEthAccMiiPhyConfig(UINT32 phyAddr,
				    BOOL speed100, 
				    BOOL fullDuplex, 
				    BOOL autonegotiate);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMiiPhyReset(UINT32 phyAddr)
 *
 *
 * @brief Reset a PHY
 *   Reset a PHY
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @pre The MAC on Ethernet Port 2 (NPE C) must be initialised, and generating the MDIO clock.
 *   
 * @param phyAddr: the address of the Ethernet PHY (0-31)
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_FAIL : invalid arguments.
 *
 * <hr>
 */
IxEthAccStatus ixEthAccMiiPhyReset(UINT32 phyAddr);


/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMiiLinkStatus(UINT32 phyAddr, 
				     BOOL *linkUp,
				     BOOL *speed100, 
				     BOOL *fullDuplex,
				     BOOL *autoneg)
 *
 *
 * @brief Retrieve the current status of a PHY
 *   Retrieve the link, speed, duplex and autonegotiation status of a PHY
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @pre The MAC on Ethernet Port 2 (NPE C) must be initialised, and generating the MDIO clock.
 *   
 * @param phyAddr: the address of the Ethernet PHY (0-31)
 * @param linkUp : set to TRUE if the link is up
 * @param speed100: set to TRUE indicates 100Mbit/s, FALSE indicates 10Mbit/s
 * @param fullDuplex: set to TRUE indicates Full Duplex, FALSE indicates Half Duplex
 * @param autoneg : set to TRUE indicates autonegotiation is enabled, FALSE indicates autonegotiation is disabled
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_FAIL : invalid arguments.
 *
 * <hr>
 */
IxEthAccStatus ixEthAccMiiLinkStatus(UINT32 phyAddr, 
				     BOOL *linkUp,
				     BOOL *speed100, 
				     BOOL *fullDuplex,
				     BOOL *autoneg);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMiiShow (UINT32 phyAddr)
 *
 *
 * @brief Display information on a specified PHY
 *   Display link status, speed, duplex and Auto Negotiation status
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @pre The MAC on Ethernet Port 2 (NPE C) must be initialised, and generating the MDIO clock.
 *   
 * @param phyAddr: the address of the Ethernet PHY (0-31)
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_FAIL : invalid arguments.
 *
 * <hr>
 */
IxEthAccStatus ixEthAccMiiShow (UINT32 phyAddr);

/**
 * @ingroup IxEthAcc
 *
 * @fn ixEthAccMiiStatsShow (UINT32 phyAddr)
 *
 *
 * @brief Display detailed information on a specified PHY
 *   Display the current values of the standard MII registers for a PHY, in addition to the status information supplied by ixEthAccMiiShow
 *
 * - Reentrant    - no
 * - ISR Callable - no
 *
 * @pre The MAC on Ethernet Port 2 (NPE C) must be initialised, and generating the MDIO clock.
 *   
 * @param phyAddr: the address of the Ethernet PHY (0-31)
 *
 * @return IxEthAccStatus
 * - IX_ETH_ACC_SUCCESS
 * - IX_ETH_ACC_FAIL : invalid arguments.
 *
 * <hr>
 */
IxEthAccStatus ixEthAccMiiStatsShow (UINT32 phyAddr);



#endif /* ndef IxEthAcc_H */
/**
 *@}
 */
