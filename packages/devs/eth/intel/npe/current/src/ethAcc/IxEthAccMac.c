/**
 * @file IxEthAccMac.c
 *
 * @author Intel Corporation
 * @date 
 *
 * @brief  MAC control functions
 *
 * Design Notes:
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

#include <string.h>
#include <stdio.h>
#include <IxTypes.h>
#include <IxNpeMh.h>
#include <IxEthDBPortDefs.h>
#include <IxEthDB.h>
#include <IxEthAcc.h>
#include <IxEthNpe.h>

#include "IxEthAcc_p.h"
#include "IxEthAccMac_p.h"


#define IX_ETH_ACC_VALIDATE_PORT_ID(portId) \
    do                                                           \
    {                                                            \
        if(!IX_ETH_ACC_IS_PORT_VALID(portId))   \
        {                                                        \
	    return IX_ETH_ACC_INVALID_PORT;                      \
        }                                                        \
    } while(0)                                                   

typedef struct
{
    BOOL fullDuplex;
    BOOL rxFCSAppend;
    BOOL txFCSAppend;
    BOOL enabled;
    BOOL promiscuous;    
    IxMutex ackMIBStatsLock;
    IxMutex ackMIBStatsResetLock;   
    IxMutex MIBStatsGetAccessLock;
    IxMutex MIBStatsGetResetAccessLock;
    IxMutex ackHaltNPELock;
    IxEthAccMacAddr ixEthAccMulticastAddrsTable[IX_ETH_ACC_MAX_MULTICAST_ADDRESSES];
    UINT32 ixEthAccMulticastAddrIndex;
    BOOL ixEthAccMacAddressInitDone;
    BOOL macInitialised;
}IxEthAccMacState;

PRIVATE IxEthAccMacState macState[IX_ETH_ACC_NUMBER_OF_PORTS];

PRIVATE UINT32 ixEthAccMacBase[IX_ETH_ACC_NUMBER_OF_PORTS] = 
{ 
    IX_ETH_ACC_MAC_0_BASE, 
    IX_ETH_ACC_MAC_1_BASE
};

/*Forward function declarations*/
PRIVATE IxEthAccStatus 
ixEthAccMulticastAddressSet(IxEthAccPortId portId);
PRIVATE IxEthAccStatus
ixEthAccMemMove(void *dest,
		const void *src,
		UINT32 count);

PRIVATE IxEthAccStatus
ixEthAccMemCpy(void *dest, 
	       const void *src, 
	       UINT32 count);

PRIVATE IxEthAccStatus
ixEthAccMemSet(void *s, 
	       UINT32 val, 
	       UINT32 count);
PRIVATE BOOL
ixEthAccMacEqual(IxEthAccMacAddr *macAddr1,
		 IxEthAccMacAddr *macAddr2);

PRIVATE void
ixEthAccMacPrint(IxEthAccMacAddr *m);

PRIVATE void
ixEthAccMacStateUpdate(IxEthAccPortId portId);

IxEthAccStatus
ixEthAccPortEnable(IxEthAccPortId portId)
{
    IxEthAccStatus result;
    
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    if(!macState[portId].ixEthAccMacAddressInitDone)
    {
	return (IX_ETH_ACC_MAC_UNINITIALIZED);
    }

    /* if the state is being set to what it is already at, do nothing*/
    if (macState[portId].enabled)
    {
        return IX_ETH_ACC_SUCCESS;
    }
    
    
    /* enable ethernet database for this port */

    if ((result = ixEthDBPortEnable(portId)) != IX_ETH_ACC_SUCCESS)
    {
        return result;
    }

    REG_WRITE(ixEthAccMacBase[portId], 
	      IX_ETH_ACC_MAC_TX_CNTRL2,
	      IX_ETH_ACC_TX_CNTRL2_RETRIES_MASK);
    
    REG_WRITE(ixEthAccMacBase[portId], 
	      IX_ETH_ACC_MAC_RANDOM_SEED,
	      IX_ETH_ACC_RANDOM_SEED_DEFAULT);
    
    REG_WRITE(ixEthAccMacBase[portId], 
	      IX_ETH_ACC_MAC_THRESH_P_EMPTY,
	      IX_ETH_ACC_MAC_THRESH_P_EMPTY_DEFAULT);
    
    REG_WRITE(ixEthAccMacBase[portId], 
	      IX_ETH_ACC_MAC_THRESH_P_FULL,
	      IX_ETH_ACC_MAC_THRESH_P_FULL_DEFAULT);
    
    REG_WRITE(ixEthAccMacBase[portId], 
	      IX_ETH_ACC_MAC_TX_DEFER,	      
	      IX_ETH_ACC_MAC_TX_DEFER_DEFAULT);
    
    REG_WRITE(ixEthAccMacBase[portId], 
	      IX_ETH_ACC_MAC_TX_TWO_DEFER_1,	      
	      IX_ETH_ACC_MAC_TX_TWO_DEFER_1_DEFAULT);
    
    REG_WRITE(ixEthAccMacBase[portId], 
	      IX_ETH_ACC_MAC_TX_TWO_DEFER_2,	      
	      IX_ETH_ACC_MAC_TX_TWO_DEFER_2_DEFAULT);
    
    
    REG_WRITE(ixEthAccMacBase[portId], 
	      IX_ETH_ACC_MAC_SLOT_TIME,
	      IX_ETH_ACC_MAC_SLOT_TIME_DEFAULT);
    
    
    REG_WRITE(ixEthAccMacBase[portId], 
	      IX_ETH_ACC_MAC_INT_CLK_THRESH,
	      IX_ETH_ACC_MAC_INT_CLK_THRESH_DEFAULT);
    
    
    REG_WRITE(ixEthAccMacBase[portId], 
	      IX_ETH_ACC_MAC_BUF_SIZE_TX,	      
	      IX_ETH_ACC_MAC_BUF_SIZE_TX_DEFAULT);
    
    
    
    REG_WRITE(ixEthAccMacBase[portId], 
	      IX_ETH_ACC_MAC_TX_CNTRL1,
	      IX_ETH_ACC_TX_CNTRL1_DEFAULT);
    
    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_RX_CNTRL1,
	      IX_ETH_ACC_RX_CNTRL1_DEFAULT);
    
    macState[portId].enabled = TRUE;
    ixEthAccMacStateUpdate(portId);

    return IX_ETH_ACC_SUCCESS;
}

PRIVATE void
ixEthAccNpeNPEHaltMessageCallback (IxNpeMhNpeId npeId,
                                   IxNpeMhMessage msg)
{
    IxEthAccPortId portId = IX_ETH_ACC_NPE_TO_PORT_ID(npeId);

    /*Unblock Stats Get & reset call*/
    ixOsServMutexUnlock(&macState[portId].ackHaltNPELock); 
}

IxEthAccStatus
ixEthAccPortDisable(IxEthAccPortId portId)
{
    IxEthAccStatus result;
    IxNpeMhMessage message;
    IxEthAccTxPriority priority;
    IX_MBUF *mbuf;
    
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    if(!macState[portId].ixEthAccMacAddressInitDone)
    {
	return (IX_ETH_ACC_MAC_UNINITIALIZED);
    }

    /* if the state is being set to what it is already at, do nothing*/
    if (!macState[portId].enabled)
    {
        return IX_ETH_ACC_SUCCESS;
    }
    
    /* disable ethernet database for this port */

    if ((result = ixEthDBPortDisable(portId)) != IX_ETH_ACC_SUCCESS)
    {
	IX_ETH_ACC_FATAL_LOG(
	    "IXETHACC:ixEthAccPortDisable: ixEthDBPortDisable failed", 
	    0, 0, 0, 0, 0, 0);

        return result;
    }

    macState[portId].enabled = FALSE;
    
    /* disable MAC */
    ixEthAccMacStateUpdate(portId);

    /* halt NPE - this will flush all mbufs from the NPE */
    message.data[0] = IX_ETHNPE_X2P_NPE_HALT << IX_ETH_ACC_MAC_MSGID_SHL;
    message.data[1] = (UINT32) 0;
     
    if (ixNpeMhMessageWithResponseSend(IX_ETH_ACC_PORT_ID_TO_NPE(portId), 
        message,
        IX_ETHNPE_P2X_NPE_STATUS,
        ixEthAccNpeNPEHaltMessageCallback, 
        IX_NPEMH_SEND_RETRIES_DEFAULT) != IX_SUCCESS)
    {
     	return IX_ETH_ACC_FAIL;
    }

    /* wait for halt completion */
    ixOsServMutexLock(&macState[portId].ackHaltNPELock);

    /* return mbufs from h/w queue */
    ixEthAccRecoverTxSubmittedQBuffers(portId);
    ixEthAccRecoverRxFreeQBuffers(portId);

    /* return mbufs from s/w queue */

    /* tx queues */
    for (priority = IX_ETH_ACC_TX_PRIORITY_0 ; priority <= IX_ETH_ACC_TX_PRIORITY_7 ; priority++)
    {
        do
        {
            IX_ETH_ACC_DATAPLANE_REMOVE_MBUF_FROM_Q_HEAD(ixEthAccPortData[portId].ixEthAccTxData.txQ[priority], mbuf);

            mbuf = ixEthAccMbufFromSwQ(mbuf);

            if (mbuf != NULL)
            {
                IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccTxData.stats.txDoneUserCallback);

	        ixEthAccPortData[portId].ixEthAccTxData.txBufferDoneCallbackFn(
                    ixEthAccPortData[portId].ixEthAccTxData.txCallbackTag, 
                    mbuf);
            }
        } 
        while (mbuf != NULL);
    }

    /* rx queue */
    do
    {
        IX_ETH_ACC_DATAPLANE_REMOVE_MBUF_FROM_Q_HEAD(ixEthAccPortData[portId].ixEthAccRxData.freeBufferList, mbuf);

        mbuf = ixEthAccMbufFromSwQ(mbuf);

        if (mbuf != NULL)
        {
            IX_ETH_ACC_STATS_INC(ixEthAccPortData[portId].ixEthAccRxData.stats.rxFrameClientCallback);

            ixEthAccPortData[portId].ixEthAccRxData.rxCallbackFn(
                ixEthAccPortData[portId].ixEthAccRxData.rxCallbackTag, 
                mbuf,
                IX_ETH_DB_NUMBER_OF_PORTS /* port not found */);

        } 
    }
    while (mbuf != NULL);

    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus
ixEthAccPortEnabledQuery(IxEthAccPortId portId, BOOL *enabled)
{
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);  

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    
    *enabled = macState[portId].enabled;
    
    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus 
ixEthAccPortPromiscuousModeClear(IxEthAccPortId portId)
{
    UINT32 regval;
    UINT32 i;
    IxEthAccMacAddr addressMaskDef = {{0xff,0xff,0xff,0xff,0xff,0xff}};
    IxEthAccMacAddr addressDef = {{0xff,0xff,0xff,0xff,0xff,0xff}};

    /* Turn off promiscuous mode */    
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);
    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
   
    /*set bit 5 of Rx control 1 - enable address filtering*/
    REG_READ(ixEthAccMacBase[portId], 
	     IX_ETH_ACC_MAC_RX_CNTRL1,
	     regval);
    
    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_RX_CNTRL1,
	      regval | IX_ETH_ACC_RX_CNTRL1_ADDR_FLTR_EN);

    /* No entries in multicast address table? Then default to filter everything */
    if(macState[portId].ixEthAccMulticastAddrIndex == 0)
    {
	for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE;i++)
	{
	    REG_WRITE(ixEthAccMacBase[portId],
		      IX_ETH_ACC_MAC_ADDR_MASK_1+i*sizeof(UINT32),
		      addressMaskDef.macAddress[i]);

	    REG_WRITE(ixEthAccMacBase[portId],
		      IX_ETH_ACC_MAC_ADDR_1+i*sizeof(UINT32),
		      addressDef.macAddress[i]);	    
	}
    }

    macState[portId].promiscuous = FALSE;
    return IX_ETH_ACC_SUCCESS;    
}


IxEthAccStatus  
ixEthAccPortPromiscuousModeSet(IxEthAccPortId portId)
{
    UINT32 regval;
    UINT32 i;
    IxEthAccMacAddr addressMask = {{0x0,0x0,0x0,0x0,0x0,0x0}};

    /*Turn on Promiscuous mode*/    
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);
    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    
    /* 
     * Set bit 5 of Rx control 1 - We enable address filtering even in
     * promiscuous mode because we want the MAC to set the appropriate
     * bits in m_flags which doesn't happen if we turn off filtering.
     */
    REG_READ(ixEthAccMacBase[portId], 
	     IX_ETH_ACC_MAC_RX_CNTRL1,
	     regval);
    
    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_RX_CNTRL1,
	      regval | IX_ETH_ACC_RX_CNTRL1_ADDR_FLTR_EN);

    for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE;i++)
    {
	
	REG_WRITE(ixEthAccMacBase[portId],IX_ETH_ACC_MAC_ADDR_MASK_1+i*sizeof(UINT32),
		  addressMask.macAddress[i]);
    }

    macState[portId].promiscuous = TRUE;
    return IX_ETH_ACC_SUCCESS;
}




IxEthAccStatus 
ixEthAccPortUnicastMacAddressSet (IxEthAccPortId portId,
				  IxEthAccMacAddr *macAddr)
{
    UINT32 i;
    IxEthAccStatus result;
    
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }


    if (macAddr == NULL)
    {
	return IX_ETH_ACC_FAIL;
    }    

    if ( macAddr->macAddress[0] & IX_ETH_ACC_ETH_MAC_BCAST_MCAST_BIT )
    {
	/* This is a multicast/broadcast address cant set it ! */
	return IX_ETH_ACC_FAIL;
    }

    if ( macAddr->macAddress[0] == 0 &&
	 macAddr->macAddress[1] == 0 &&
	 macAddr->macAddress[2] == 0 &&
	 macAddr->macAddress[3] == 0 &&
	 macAddr->macAddress[4] == 0 &&
	 macAddr->macAddress[5] == 0  )
    {
	/* This is an invalid mac address cant set it ! */
	return IX_ETH_ACC_FAIL;
    }
	

    /* update the MAC address in the ethernet database */
    if ((result = ixEthDBPortAddressSet(portId, (IxEthDBMacAddr *) macAddr)) != IX_ETH_ACC_SUCCESS)
    {
        return result;
    }
    
    /*Set the Unicast MAC to the specified value*/
    for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE;i++)
    {	
	REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_UNI_ADDR_1 + i*sizeof(UINT32),
		  macAddr->macAddress[i]);	
    }
    macState[portId].ixEthAccMacAddressInitDone = TRUE;    
    return IX_ETH_ACC_SUCCESS;
}




IxEthAccStatus 
ixEthAccPortUnicastMacAddressGet (IxEthAccPortId portId, 
				  IxEthAccMacAddr *macAddr)
{
    /*Return the current value of the Unicast MAC from h/w
      for the specified port*/
    UINT32 i;
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if(!macState[portId].ixEthAccMacAddressInitDone)
    {
	return (IX_ETH_ACC_MAC_UNINITIALIZED);
    }
    
    if (macAddr == NULL)
    {
	return IX_ETH_ACC_FAIL;
    }    
    
    
    for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE;i++)
    {
	REG_READ(ixEthAccMacBase[portId],
		 IX_ETH_ACC_MAC_UNI_ADDR_1 + i*sizeof(UINT32),
		 macAddr->macAddress[i]);	
    }
    return IX_ETH_ACC_SUCCESS;
}

PRIVATE IxEthAccStatus 
ixEthAccPortMulticastMacAddressGet (IxEthAccPortId portId, 
				    IxEthAccMacAddr *macAddr)
{
    /*Return the current value of the Multicast MAC from h/w
      for the specified port*/
    UINT32 i;
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);
    
    if(macAddr == NULL)
    {
	return IX_ETH_ACC_FAIL;
    }    
    
    for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE;i++)
    {
	
	REG_READ(ixEthAccMacBase[portId],
		 IX_ETH_ACC_MAC_ADDR_1 + i*sizeof(UINT32),
		 macAddr->macAddress[i]);
    }
    return IX_ETH_ACC_SUCCESS;
}



IxEthAccStatus 
ixEthAccPortMulticastAddressJoin (IxEthAccPortId portId,
				  IxEthAccMacAddr *macAddr)
{
    UINT32 i;
    IxEthAccMacAddr broadcastAddr = {{0xff,0xff,0xff,0xff,0xff,0xff}};

    /*Check that the port parameter is valid*/
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    
    /*Check that the mac address is valid*/
    if(macAddr == NULL)
    {
	return IX_ETH_ACC_FAIL;
    }

    /* Check that this is a multicast address */
    if (!(macAddr->macAddress[0] & IX_ETH_ACC_ETH_MAC_BCAST_MCAST_BIT))
    {
	return IX_ETH_ACC_FAIL;
    }

    /* We don't add the Broadcast address */
    if(ixEthAccMacEqual(&broadcastAddr, macAddr))
    {
	return IX_ETH_ACC_FAIL;
    }

    if(macState[portId].ixEthAccMulticastAddrIndex >= IX_ETH_ACC_MAX_MULTICAST_ADDRESSES)
    {
	return IX_ETH_ACC_FAIL;
    }
    
    /*Add this mac address to the address table for the 
      specified port*/
    
    /*First add the address to the multicast table for the 
      specified port*/
    i=macState[portId].ixEthAccMulticastAddrIndex;
    
    ixEthAccMemCpy(&macState[portId].ixEthAccMulticastAddrsTable[i],
		   &macAddr->macAddress,
		   IX_IEEE803_MAC_ADDRESS_SIZE);
    
    /*Increment the index into the table, this must be done here
     as MulticastAddressSet below needs to know about the latest 
     entry.
    */
    macState[portId].ixEthAccMulticastAddrIndex++;
    
    /*Then calculate the new value to be written to the address and 
      address mask registers*/
    if(ixEthAccMulticastAddressSet(portId)!= IX_ETH_ACC_SUCCESS)
    {
	/*Decrement the index into the table*/
	macState[portId].ixEthAccMulticastAddrIndex--;
	
	return IX_ETH_ACC_FAIL;
    }
    
    return IX_ETH_ACC_SUCCESS;
}



IxEthAccStatus 
ixEthAccPortMulticastAddressLeave (IxEthAccPortId portId,
				   IxEthAccMacAddr *macAddr)
{
    UINT32 i;

    /*Check that the port parameter is valid*/
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    
    /*Check that the mac address is valid*/
    if(macAddr == NULL)
    {
	return IX_ETH_ACC_FAIL;
    }
    /*Remove this mac address from the mask for the specified port
     */
    /*we copy down all entries above the blanked entry, and
     * decrement the index - ixEthAccMemMove guarantees non-corruption of
     * overlapping source and destination
     */    
    i=0;

    while(i<macState[portId].ixEthAccMulticastAddrIndex)
    {
	/*Check if the current entry matches*/
	if(ixEthAccMacEqual(&macState[portId].ixEthAccMulticastAddrsTable[i], macAddr))
	{
	    /*Copy down all entries above the current entry*/
	    ixEthAccMemMove(&macState[portId].ixEthAccMulticastAddrsTable[i],
			    &macState[portId].ixEthAccMulticastAddrsTable[i+1], 
			    IX_IEEE803_MAC_ADDRESS_SIZE*(macState[portId].ixEthAccMulticastAddrIndex-i));
	    /*Decrement the index into the multicast address table
	      for the current port*/
	    macState[portId].ixEthAccMulticastAddrIndex--;
	    /*recalculate the mask and write it to the MAC*/
	    return ixEthAccMulticastAddressSet(portId);
	}
	
	i++;	
    }    
    return IX_ETH_ACC_NO_SUCH_ADDR;
}


IxEthAccStatus
ixEthAccPortUnicastAddressShow (IxEthAccPortId portId)
{
    IxEthAccMacAddr macAddr;
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    
    /*Get the MAC (UINICAST) address from hardware*/
    if(ixEthAccPortUnicastMacAddressGet(portId, &macAddr) != IX_ETH_ACC_SUCCESS)
    {
	printf("IxEthAcc MAC address uninitialised\n");
	return IX_ETH_ACC_MAC_UNINITIALIZED;
    }

    /*print it out*/
    ixEthAccMacPrint(&macAddr);
    printf("\n");
    return IX_ETH_ACC_SUCCESS;
}



void
ixEthAccPortMulticastAddressShow(IxEthAccPortId portId)
{    
    IxEthAccMacAddr macAddr;
    UINT32 i;

    if (!IX_ETH_ACC_IS_PORT_VALID(portId) || !IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	    return;
    }

    printf("Multicast MAC: ");
    /*Get the MAC (MULTICAST) address from hardware*/
    ixEthAccPortMulticastMacAddressGet(portId, &macAddr);
    /*print it out*/
    ixEthAccMacPrint(&macAddr);
    printf("\n");
    printf("Constituent Addresses:\n");
    for(i=0;i<macState[portId].ixEthAccMulticastAddrIndex;i++)
    {
	ixEthAccMacPrint(&macState[portId].ixEthAccMulticastAddrsTable[i]);
	printf("\n");
    }
    return;
}


/*Set the duplex mode*/
IxEthAccStatus 
ixEthAccPortDuplexModeSet (IxEthAccPortId portId, 
			   IxEthAccDuplexMode mode)
{
    UINT32 txregval;
    UINT32 rxregval;
    /*This is bit 1 of the transmit control reg, set to 1 for half
      duplex, 0 for full duplex*/
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    
    REG_READ(ixEthAccMacBase[portId], 
	     IX_ETH_ACC_MAC_TX_CNTRL1,
	     txregval);
    
    REG_READ(ixEthAccMacBase[portId],
	     IX_ETH_ACC_MAC_RX_CNTRL1,
	     rxregval);
    
    if (mode ==  IX_ETH_ACC_FULL_DUPLEX)
    {
	/*Clear half duplex bit in TX*/
	REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_TX_CNTRL1,
		  txregval & ~IX_ETH_ACC_TX_CNTRL1_DUPLEX);
	/*We must set the pause enable in the receive logic when in
	  full duplex mode*/
	
	REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_RX_CNTRL1,
		  rxregval | IX_ETH_ACC_RX_CNTRL1_PAUSE_EN);
	macState[portId].fullDuplex = TRUE;
	
    }
    else if (mode ==  IX_ETH_ACC_HALF_DUPLEX)
    {
	/*Set half duplex bit in TX*/
	REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_TX_CNTRL1,
		  txregval | IX_ETH_ACC_TX_CNTRL1_DUPLEX);
	/*We must clear pause enable in the receive logic when in
	  half duplex mode*/	
	REG_WRITE(ixEthAccMacBase[portId],
		  IX_ETH_ACC_MAC_RX_CNTRL1,
		  rxregval & ~IX_ETH_ACC_RX_CNTRL1_PAUSE_EN);
	macState[portId].fullDuplex = FALSE;
    }
    else
    {
	return IX_ETH_ACC_FAIL;
    }
    
    
    return IX_ETH_ACC_SUCCESS;    
    
}



IxEthAccStatus 
ixEthAccPortDuplexModeGet (IxEthAccPortId portId, 
			   IxEthAccDuplexMode *mode)
{
    /*Return the duplex mode for the specified port*/
    UINT32 regval;
    /*This is bit 1 of the transmit control reg, set to 1 for half
      duplex, 0 for full duplex*/
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);
    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    
    REG_READ(ixEthAccMacBase[portId], 
	     IX_ETH_ACC_MAC_TX_CNTRL1,
	     regval);
    
    if( regval & IX_ETH_ACC_TX_CNTRL1_DUPLEX)
    {
	*mode = IX_ETH_ACC_HALF_DUPLEX;
    }
    else
    {
	*mode = IX_ETH_ACC_FULL_DUPLEX;
    }
    return IX_ETH_ACC_SUCCESS;
}



IxEthAccStatus 
ixEthAccPortTxFrameAppendFCSEnable (IxEthAccPortId portId)
{
    UINT32 regval;
    /*Enable FCS computation by the MAC and appending to the
      frame*/
    
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    
    REG_READ(ixEthAccMacBase[portId], 
	     IX_ETH_ACC_MAC_TX_CNTRL1,
	     regval);
    
    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_TX_CNTRL1,
	      regval | IX_ETH_ACC_TX_CNTRL1_FCS_EN);
    macState[portId].txFCSAppend = TRUE;
    return IX_ETH_ACC_SUCCESS;  
}

IxEthAccStatus 
ixEthAccPortTxFrameAppendFCSDisable (IxEthAccPortId portId)
{
    UINT32 regval;
    /*disable FCS computation and appending*/
    /*Set bit 4 of tx control register one to zero*/
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    
    REG_READ(ixEthAccMacBase[portId], 
	     IX_ETH_ACC_MAC_TX_CNTRL1,
	     regval);
    
    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_TX_CNTRL1,
	      regval & ~IX_ETH_ACC_TX_CNTRL1_FCS_EN);
    macState[portId].txFCSAppend = FALSE;
    return IX_ETH_ACC_SUCCESS; 
}

IxEthAccStatus 
ixEthAccPortRxFrameAppendFCSEnable (IxEthAccPortId portId)
{
    /*Set bit 2 of rx control 1*/
    UINT32 regval;
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);
    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    
    REG_READ(ixEthAccMacBase[portId], 
	     IX_ETH_ACC_MAC_RX_CNTRL1,
	     regval);
    
    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_RX_CNTRL1,
	      regval | IX_ETH_ACC_RX_CNTRL1_CRC_EN);
    macState[portId].rxFCSAppend = TRUE;
    return IX_ETH_ACC_SUCCESS;
}

IxEthAccStatus 
ixEthAccPortRxFrameAppendFCSDisable (IxEthAccPortId portId)
{
    UINT32 regval;
    /*Clear bit 2 of rx control 1*/
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);    
    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    
    REG_READ(ixEthAccMacBase[portId], 
	     IX_ETH_ACC_MAC_RX_CNTRL1,
	     regval);
    
    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_RX_CNTRL1,
	      regval & ~IX_ETH_ACC_RX_CNTRL1_CRC_EN);
    macState[portId].rxFCSAppend = FALSE;
    return IX_ETH_ACC_SUCCESS;
}



PRIVATE void 
ixEthAccMacNpeStatsMessageCallback (IxNpeMhNpeId npeId, 
				    IxNpeMhMessage msg)
{
    IxEthAccPortId portId = IX_ETH_ACC_NPE_TO_PORT_ID(npeId);
    
    /*Unblock Stats Get call*/
    ixOsServMutexUnlock(&macState[portId].ackMIBStatsLock);
    
}


IxEthAccStatus 
ixEthAccMibIIStatsGet (IxEthAccPortId portId, 
		       IxEthEthObjStats *retStats )
{    
    IxNpeMhMessage message;
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);
    
    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    if (!macState[portId].enabled)
    {
        return (IX_ETH_ACC_FAIL);
    }

    IX_ACC_DATA_CACHE_INVALIDATE(retStats, sizeof(IxEthEthObjStats));
    retStats = IX_MMU_VIRTUAL_TO_PHYSICAL_TRANSLATION(retStats);

    message.data[0] = IX_ETHNPE_X2P_STATS_SHOW << IX_ETH_ACC_MAC_MSGID_SHL;
    message.data[1] = (UINT32)retStats;
    
    /*Permit only one task to request MIB statistics Get operation
      at a time*/
    ixOsServMutexLock(&macState[portId].MIBStatsGetAccessLock);
    
    if(ixNpeMhMessageWithResponseSend(IX_ETH_ACC_PORT_ID_TO_NPE(portId),
				      message,
				      IX_ETHNPE_P2X_STATS_REPORT,
				      ixEthAccMacNpeStatsMessageCallback,
				      IX_NPEMH_SEND_RETRIES_DEFAULT) 
       != IX_SUCCESS)
    {
	ixOsServMutexUnlock(&macState[portId].MIBStatsGetAccessLock);
	return IX_ETH_ACC_FAIL;
    }

    /*Wait for callback invocation indicating response to 
      this request - we need this mutex in order to ensure
      that the return from this function is synchronous
    */
    ixOsServMutexLock(&macState[portId].ackMIBStatsLock);
    /*Permit other tasks to perform MIB statistics Get operation*/
    ixOsServMutexUnlock(&macState[portId].MIBStatsGetAccessLock);

    return IX_ETH_ACC_SUCCESS;
}


PRIVATE void 
ixEthAccMacNpeStatsResetMessageCallback (IxNpeMhNpeId npeId, 
					 IxNpeMhMessage msg)
{
    IxEthAccPortId portId = IX_ETH_ACC_NPE_TO_PORT_ID(npeId);
    
    /*Unblock Stats Get & reset call*/
    ixOsServMutexUnlock(&macState[portId].ackMIBStatsResetLock);
    
}



IxEthAccStatus 
ixEthAccMibIIStatsGetClear (IxEthAccPortId portId, 
			    IxEthEthObjStats *retStats)
{
    IxNpeMhMessage message;
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);    

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    if (!macState[portId].enabled)
    {
        return (IX_ETH_ACC_FAIL);
    }

    IX_ACC_DATA_CACHE_INVALIDATE(retStats, sizeof(IxEthEthObjStats));
    retStats = IX_MMU_VIRTUAL_TO_PHYSICAL_TRANSLATION(retStats);
    
    message.data[0] = IX_ETHNPE_X2P_STATS_RESET << 
	IX_ETH_ACC_MAC_MSGID_SHL;
    message.data[1] = (UINT32)retStats;
    
    /*Permit only one task to request MIB statistics Get-Reset operation
      at a time*/
    ixOsServMutexLock(&macState[portId].MIBStatsGetResetAccessLock);
    
    if(ixNpeMhMessageWithResponseSend(IX_ETH_ACC_PORT_ID_TO_NPE(portId), 
				      message,
				      IX_ETHNPE_P2X_STATS_REPORT,
				      ixEthAccMacNpeStatsResetMessageCallback,
				      IX_NPEMH_SEND_RETRIES_DEFAULT) 
       != IX_SUCCESS)
    {
	ixOsServMutexUnlock(&macState[portId].MIBStatsGetResetAccessLock);
	return IX_ETH_ACC_FAIL;
    }

    /*Wait for callback invocation indicating response to 
      this request*/
    ixOsServMutexLock(&macState[portId].ackMIBStatsResetLock);
    /*permit other tasks to get and reset MIB stats*/
    ixOsServMutexUnlock(&macState[portId].MIBStatsGetResetAccessLock);
    return IX_ETH_ACC_SUCCESS;
    
}

IxEthAccStatus
ixEthAccMibIIStatsClear (IxEthAccPortId portId)
{
    IxEthEthObjStats retStats;
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);    
    /*There is no reset operation without a corresponding Get
    */
    
    return ixEthAccMibIIStatsGetClear(portId, &retStats);
    
}



/*Reset the ethernet MAC*/
IxEthAccStatus
ixEthAccMacInit(IxEthAccPortId portId)
{
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);
    
    if(macState[portId].macInitialised == FALSE)
    {
	macState[portId].fullDuplex  = TRUE;
	macState[portId].rxFCSAppend = TRUE;
	macState[portId].txFCSAppend = TRUE;
	macState[portId].enabled     = FALSE;
	macState[portId].promiscuous = TRUE;
	macState[portId].ixEthAccMacAddressInitDone = FALSE;
	macState[portId].macInitialised = TRUE;
    
        /* initialize MIB stats mutexes */
        ixOsServMutexInit(&macState[portId].ackMIBStatsLock);
        ixOsServMutexLock(&macState[portId].ackMIBStatsLock);

        ixOsServMutexInit(&macState[portId].ackMIBStatsResetLock);
        ixOsServMutexLock(&macState[portId].ackMIBStatsResetLock);

        ixOsServMutexInit(&macState[portId].MIBStatsGetAccessLock);
        ixOsServMutexUnlock(&macState[portId].MIBStatsGetAccessLock);

        ixOsServMutexInit(&macState[portId].MIBStatsGetResetAccessLock);
        ixOsServMutexUnlock(&macState[portId].MIBStatsGetResetAccessLock);

        ixOsServMutexInit(&macState[portId].ackHaltNPELock);
        ixOsServMutexLock(&macState[portId].ackHaltNPELock);
    }
    
    REG_WRITE(ixEthAccMacBase[portId], IX_ETH_ACC_MAC_CORE_CNTRL,
	      IX_ETH_ACC_CORE_RESET);
    
    ixOsServTaskSleep(IX_ETH_ACC_MAC_RESET_DELAY);   
    
    REG_WRITE(ixEthAccMacBase[portId], IX_ETH_ACC_MAC_CORE_CNTRL,
	      IX_ETH_ACC_CORE_MDC_EN);
    
    REG_WRITE(ixEthAccMacBase[portId],
	      IX_ETH_ACC_MAC_INT_CLK_THRESH,
	      IX_ETH_ACC_MAC_INT_CLK_THRESH_DEFAULT);
    
    ixEthAccMacStateUpdate(portId);
    
    return IX_ETH_ACC_SUCCESS; 
}

/* PRIVATE Functions*/

PRIVATE void
ixEthAccMacStateUpdate(IxEthAccPortId portId)
{
  UINT32 regval;
 
    if ( macState[portId].enabled == FALSE )
    {
	/*  Just disable both the transmitter and reciver in the MAC.  */
        REG_READ(ixEthAccMacBase[portId], 
	     IX_ETH_ACC_MAC_TX_CNTRL1,
	     regval);
 	REG_WRITE(ixEthAccMacBase[portId],
              IX_ETH_ACC_MAC_TX_CNTRL1,
              regval & ~IX_ETH_ACC_TX_CNTRL1_TX_EN);

        REG_READ(ixEthAccMacBase[portId], 
	     IX_ETH_ACC_MAC_RX_CNTRL1,
	     regval);
 	REG_WRITE(ixEthAccMacBase[portId],
              IX_ETH_ACC_MAC_RX_CNTRL1,
              regval & ~IX_ETH_ACC_RX_CNTRL1_RX_EN);
    }
    
    if(macState[portId].fullDuplex)
    {
	ixEthAccPortDuplexModeSet (portId, IX_ETH_ACC_FULL_DUPLEX);
    }
    else
    {
	ixEthAccPortDuplexModeSet (portId, IX_ETH_ACC_HALF_DUPLEX);
    }
    if(macState[portId].rxFCSAppend)
    {
	ixEthAccPortRxFrameAppendFCSEnable (portId);
    }
    else
    {
	ixEthAccPortRxFrameAppendFCSDisable (portId);
    }
    if(macState[portId].txFCSAppend)
    {
	ixEthAccPortTxFrameAppendFCSEnable (portId);
    }
    else
    {
	ixEthAccPortTxFrameAppendFCSDisable (portId);
    }
    
    if(macState[portId].promiscuous)
    {
	ixEthAccPortPromiscuousModeSet(portId);
    }
    else
    {
	ixEthAccPortPromiscuousModeClear(portId);
    }

    if ( macState[portId].enabled == TRUE )
    {
        /*   Enable both the transmitter and reciver in the MAC.  */
        REG_READ(ixEthAccMacBase[portId],
             IX_ETH_ACC_MAC_TX_CNTRL1,
             regval);
        REG_WRITE(ixEthAccMacBase[portId],      
              IX_ETH_ACC_MAC_TX_CNTRL1,
              regval | IX_ETH_ACC_TX_CNTRL1_TX_EN);

        REG_READ(ixEthAccMacBase[portId],
             IX_ETH_ACC_MAC_RX_CNTRL1,
             regval);
        REG_WRITE(ixEthAccMacBase[portId],
              IX_ETH_ACC_MAC_RX_CNTRL1,
              regval | IX_ETH_ACC_RX_CNTRL1_RX_EN);
    }

    
}


PRIVATE BOOL
ixEthAccMacEqual(IxEthAccMacAddr *macAddr1,
		 IxEthAccMacAddr *macAddr2)
{
    UINT32 i;
    for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE; i++)
    {
	if(macAddr1->macAddress[i] != macAddr2->macAddress[i])
	{
	    return FALSE;
	}
    }
    return TRUE;
}

/*This is a standard implementation of memmove, taken from string.c,
  it safely moves memory
*/
PRIVATE IxEthAccStatus
ixEthAccMemMove(void *dest,
		const void *src,
		UINT32 count)
{
    char *tmp;
    char *s;
    if( dest == NULL || src == NULL)
    {
	return IX_ETH_ACC_FAIL;
    }
    if (dest <= src) {
	tmp = (char *) dest;
	s = (char *) src;
	while (count--)
	    *tmp++ = *s++;
    }
    else 
    {
	tmp = (char *) dest + count;
	s = (char *) src + count;
	while (count--)
	    *--tmp = *--s;
    }
    
    return IX_ETH_ACC_SUCCESS;
    
}

PRIVATE IxEthAccStatus
ixEthAccMemCpy(void *dest, 
	       const void *src, 
	       UINT32 count)
{
    char *tmp = (char *) dest;
    char *s = (char *) src;
    if( dest == NULL || src == NULL)
    {
	return IX_ETH_ACC_FAIL;
    }
    
    while (count--)
    {
	*tmp++ = *s++;
    }
    
    return IX_ETH_ACC_SUCCESS;
}

PRIVATE IxEthAccStatus
ixEthAccMemSet(void *s, 
	       UINT32 val, 
	       UINT32 count)
{
    char *xs = (char *) s;
    
    while (count--)
    {
	*xs++ = val; /*This is the way it is done in string.c, so the 
		       casting is be valid*/
    }
    
    return IX_ETH_ACC_SUCCESS;
    
}


PRIVATE void
ixEthAccMacPrint(IxEthAccMacAddr *m)
{
    printf("%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
	   m->macAddress[0], m->macAddress[1],
	   m->macAddress[2], m->macAddress[3],
	   m->macAddress[4], m->macAddress[5]);    
}

/* Set the multicast address and address mask registers
 * 
 * A bit in the address mask register must be set if
 * all multicast addresses always have that bit set, or if
 * all multicast addresses always have that bit cleared.
 *
 * A bit in the address register must be set if all multicast
 * addresses have that bit set, otherwise, it should be cleared
 */

PRIVATE IxEthAccStatus
ixEthAccMulticastAddressSet(IxEthAccPortId portId)
{
    UINT32 i;
    UINT32 j;
    IxEthAccMacAddr addressMask;
    IxEthAccMacAddr alwaysClearBits;
    IxEthAccMacAddr alwaysSetBits;
    
    IX_ETH_ACC_VALIDATE_PORT_ID(portId);
    
    /* calculate alwaysClearBits and alwaysSetBits:
     * alwaysClearBits is calculated by ORing all 
     * multicast addresses, those bits that are always
     * clear are clear in the result
     *
     * alwaysSetBits is calculated by ANDing all 
     * multicast addresses, those bits that are always set
     * are set in the result
     */
    
    ixEthAccMemSet(&alwaysClearBits.macAddress, 0, IX_IEEE803_MAC_ADDRESS_SIZE);
    ixEthAccMemSet(&alwaysSetBits.macAddress, IX_ETH_ACC_MAC_ALL_BITS_SET, 
		   IX_IEEE803_MAC_ADDRESS_SIZE);
    
    for(i=0;i<macState[portId].ixEthAccMulticastAddrIndex;i++)
    {
	for(j=0;j<IX_IEEE803_MAC_ADDRESS_SIZE;j++)
	{
	    alwaysClearBits.macAddress[j] |= macState[portId].ixEthAccMulticastAddrsTable[i].macAddress[j];
	    alwaysSetBits.macAddress[j] &= macState[portId].ixEthAccMulticastAddrsTable[i].macAddress[j];
	    
	}	
    }
    
    if(macState[portId].ixEthAccMulticastAddrIndex == 0)
    {
	/* No entries in the multicast address table */
	ixEthAccMemSet(&addressMask.macAddress, 0, IX_IEEE803_MAC_ADDRESS_SIZE);
	ixEthAccMemSet(&alwaysSetBits.macAddress, 0, IX_IEEE803_MAC_ADDRESS_SIZE);
    }
    else
    {
	for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE;i++)
	{
	    addressMask.macAddress[i] =  alwaysSetBits.macAddress[i] | ~alwaysClearBits.macAddress[i];
	}
    }
    
    /*write the new addr to h/w*/    
    
    for(i=0;i<IX_IEEE803_MAC_ADDRESS_SIZE;i++)
    {
	
	REG_WRITE(ixEthAccMacBase[portId],IX_ETH_ACC_MAC_ADDR_MASK_1+i*sizeof(UINT32),
		  addressMask.macAddress[i]);
	REG_WRITE(ixEthAccMacBase[portId],IX_ETH_ACC_MAC_ADDR_1+i*sizeof(UINT32),
		  alwaysSetBits.macAddress[i]);	    
    }
    
    return IX_ETH_ACC_SUCCESS;
}



