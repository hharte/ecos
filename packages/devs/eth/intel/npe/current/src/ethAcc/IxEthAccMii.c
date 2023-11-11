/**
 * @file IxEthAccMii.c
 *
 * @author Intel Corporation
 * @date 
 *
 * @brief  MII control functions
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

#include <stdio.h>

#ifdef __ECOS
#ifndef printf
#define printf diag_printf
#endif
#endif

#include "IxOsServices.h"

#include "IxEthAcc.h"
#include "IxEthAcc_p.h"
#include "IxTypes.h"
#include "IxEthAccMac_p.h"
#include "IxEthAccMii_p.h"


PRIVATE IxMutex miiAccessLock;

PRIVATE void
ixEthAccMdioCmdWrite(UINT32 mdioCommand)
{
    UINT32 baseAddress;
    baseAddress = IX_ETH_ACC_MAC_0_BASE;

    REG_WRITE(baseAddress,
	      IX_ETH_ACC_MAC_MDIO_CMD_1,
	      mdioCommand & 0xff);

    REG_WRITE(baseAddress,
	      IX_ETH_ACC_MAC_MDIO_CMD_2,
	      (mdioCommand >> 8) & 0xff);

    REG_WRITE(baseAddress,
	      IX_ETH_ACC_MAC_MDIO_CMD_3,
	      (mdioCommand >> 16) & 0xff);

    REG_WRITE(baseAddress,
	      IX_ETH_ACC_MAC_MDIO_CMD_4,
	      (mdioCommand >> 24) & 0xff);
}

PRIVATE void
ixEthAccMdioCmdRead(UINT32 *data)
{
    UINT32 regval;

    UINT32 baseAddress;
    baseAddress = IX_ETH_ACC_MAC_0_BASE;

    REG_READ(baseAddress,
	     IX_ETH_ACC_MAC_MDIO_CMD_1,
	     regval);

    *data = regval & 0xff;

    REG_READ(baseAddress,
	     IX_ETH_ACC_MAC_MDIO_CMD_2,
	     regval);

    *data |= (regval & 0xff) << 8;

    REG_READ(baseAddress,
	     IX_ETH_ACC_MAC_MDIO_CMD_3,
	     regval);

    *data |= (regval & 0xff) << 16;

    REG_READ(baseAddress,
	     IX_ETH_ACC_MAC_MDIO_CMD_4,
	     regval);
    
    *data |= (regval & 0xff) << 24;
    
}

PRIVATE void
ixEthAccMdioStatusRead(UINT32 *data)
{
    UINT32 regval;
    UINT32 baseAddress;
    baseAddress = IX_ETH_ACC_MAC_0_BASE;

    REG_READ(baseAddress,
	     IX_ETH_ACC_MAC_MDIO_STS_1,
	     regval);

    *data = regval & 0xff;

    REG_READ(baseAddress,
	     IX_ETH_ACC_MAC_MDIO_STS_2,
	     regval);

    *data |= (regval & 0xff) << 8;

    REG_READ(baseAddress,
	     IX_ETH_ACC_MAC_MDIO_STS_3,
	     regval);

    *data |= (regval & 0xff) << 16;

    REG_READ(baseAddress,
	     IX_ETH_ACC_MAC_MDIO_STS_4,
	     regval);
    
    *data |= (regval & 0xff) << 24;
    
}


/********************************************************************
 * ixEthAccMiiInit
 */
IxEthAccStatus
ixEthAccMiiInit()
{
    if(ixOsServMutexInit(&miiAccessLock)!= IX_SUCCESS)
    {
	return IX_ETH_ACC_FAIL;
    }
    if(ixOsServMutexUnlock(&miiAccessLock)!= IX_SUCCESS)
    {
	return IX_ETH_ACC_FAIL;
    }
    return IX_ETH_ACC_SUCCESS;
}

/*********************************************************************
 *
 * ixEthAccMiiReadRtn - read a 16 bit value from a PHY
 *
 * This routine is expected to perform any driver-specific functions 
 * required to read a 16-bit word from the phyReg register of the 
 * MII-compliant PHY whose address is specified by phyAddr. Reading is 
 * performed through the MII management interface.  This function returns
 * when the write has successfully completed, or when the timeout specified
 * in IX_ETH_ACC_MII_TIMEOUT_10TH_SECS has elapsed.
 *
 * RETURNS IX_ETH_ACC_SUCCESS on success, IX_ETH_ACC_FAIL otherwise
 */


IxEthAccStatus      
ixEthAccMiiReadRtn (UINT8 phyAddr, 
		    UINT8 phyReg, 
		    UINT16 *value)
{
    UINT32 mdioCommand;
    UINT32 regval;
    UINT32 miiTimeout;

    ixOsServMutexLock(&miiAccessLock);
    mdioCommand = phyReg <<  IX_ETH_ACC_MII_REG_SHL 
	| phyAddr << IX_ETH_ACC_MII_ADDR_SHL;
    mdioCommand |= IX_ETH_ACC_MII_GO;

    ixEthAccMdioCmdWrite(mdioCommand);
    
    miiTimeout = IX_ETH_ACC_MII_TIMEOUT_10TH_SECS;

    while(miiTimeout)
    {
	
	ixEthAccMdioCmdRead(&regval);
     
	if((regval & IX_ETH_ACC_MII_GO) == 0x0)
	{	    
	    break;
	}
	/*Sleep for 10th of a second*/
	ixOsServTaskSleep(IX_ETH_ACC_MII_10TH_SEC_IN_MILLIS);
	miiTimeout--;
    }
    

    
    if(miiTimeout == 0)
    {	
	ixOsServMutexUnlock(&miiAccessLock);
	return IX_ETH_ACC_FAIL;
    }
    
    
    ixEthAccMdioStatusRead(&regval);
    if(regval & IX_ETH_ACC_MII_READ_FAIL)
    {
	ixOsServMutexUnlock(&miiAccessLock);
	return IX_ETH_ACC_FAIL;
    }

    *value = regval & 0xffff;
    ixOsServMutexUnlock(&miiAccessLock);
    return IX_ETH_ACC_SUCCESS;
    
}


/*********************************************************************
 *
 * ixEthAccMiiWriteRtn - write a 16 bit value to a PHY
 * 
 * This routine is expected to perform any driver-specific functions 
 * required to write a 16-bit word to the phyReg register of the 
 * MII-compliant PHY whose address is specified by phyAddr. Writing is 
 * performed through the MII management interface. This function returns
 * when the write has successfully completed, or when the timeout specified
 * in IX_ETH_ACC_MII_TIMEOUT_10TH_SECS has elapsed.
 *
 * RETURNS IX_ETH_ACC_SUCCESS on success, IX_ETH_ACC_FAIL otherwise
 */

IxEthAccStatus
ixEthAccMiiWriteRtn (UINT8 phyAddr, 
		     UINT8 phyReg, 
		     UINT16 value)
{
    UINT32 mdioCommand;
    UINT32 regval;
    UINT32 miiTimeout;
    ixOsServMutexLock(&miiAccessLock);
    mdioCommand = phyReg << IX_ETH_ACC_MII_REG_SHL
	| phyAddr << IX_ETH_ACC_MII_ADDR_SHL ;
    mdioCommand |= IX_ETH_ACC_MII_GO | IX_ETH_ACC_MII_WRITE | value;

    ixEthAccMdioCmdWrite(mdioCommand);
    
    miiTimeout = IX_ETH_ACC_MII_TIMEOUT_10TH_SECS;

    while(miiTimeout--)
    {
	
	ixEthAccMdioCmdRead(&regval);

	/*The "GO" bit is reset to 0 when the write completes*/
	if((regval & IX_ETH_ACC_MII_GO) == 0x0)
	{	    	    
	    break;
	}
	/*Sleep for 100 milliseconds*/
	ixOsServTaskSleep(IX_ETH_ACC_MII_10TH_SEC_IN_MILLIS);
    }
    
    ixOsServMutexUnlock(&miiAccessLock);
    if(miiTimeout == 0)
    {
	return IX_ETH_ACC_FAIL;
    }
    return IX_ETH_ACC_SUCCESS;
}


/*********************************************************
 *
 * Scan for PHYs on the MII bus. This function returns
 * an array of booleans, one for each PHY address.
 * If a PHY is found at a particular address, the 
 * corresponding entry in the array is set to TRUE.
 *
 */

IxEthAccStatus
ixEthAccMiiPhyScan(BOOL phyPresent[])
{
    UINT32 i;
    UINT16 regval;

    /*Search for PHYs on the MII*/
    /*Search for extant phys on the MDIO bus*/

    for(i=0;i<IXP425_ETH_ACC_MII_MAX_ADDR;i++)
    {
	if(ixEthAccMiiReadRtn(i,IX_ETH_ACC_MII_CTRL_REG,&regval)
	   ==IX_ETH_ACC_SUCCESS)

	{
	    if((regval & 0xffff) != 0xffff)
	    {
		phyPresent[i] = TRUE;
	    }
	    else
	    {
		phyPresent[i] = FALSE;
	    }
	}
	else
	{
	    phyPresent[i] = FALSE;
	}

    }
    return IX_ETH_ACC_SUCCESS;
}

/************************************************************
 *
 * Configure the PHY at the specified address
 * 
 */
IxEthAccStatus
ixEthAccMiiPhyConfig(UINT32 phyAddr,
		     BOOL speed100, 
		     BOOL fullDuplex, 
		     BOOL autonegotiate)
{
    UINT16 regval=0;    
    if(autonegotiate)
    {
	regval |= IX_ETH_ACC_MII_CR_AUTO_EN | IX_ETH_ACC_MII_CR_RESTART;
    }
    else
    {
	if(speed100) 
	{
	    regval |= IX_ETH_ACC_MII_CR_100;
	}
	if(fullDuplex)
	{
	    regval |= IX_ETH_ACC_MII_CR_FDX;
	}
    }


    return ixEthAccMiiWriteRtn(phyAddr, IX_ETH_ACC_MII_CTRL_REG, regval);
}


/******************************************************************
 *
 *  Reset the PHY at the specified address
 */
IxEthAccStatus
ixEthAccMiiPhyReset(UINT32 phyAddr)
{
    
    ixEthAccMiiWriteRtn(phyAddr, IX_ETH_ACC_MII_CTRL_REG, 
			IX_ETH_ACC_MII_CR_RESET);

    ixOsServTaskSleep (IX_ETH_ACC_MII_RESET_DELAY_MS);

    ixEthAccMiiWriteRtn(phyAddr, IX_ETH_ACC_MII_CTRL_REG, 
			IX_ETH_ACC_MII_CR_NORM_EN);
    return IX_ETH_ACC_SUCCESS;
}

/*****************************************************************
 *
 *  Link state query functions
 */

IxEthAccStatus
ixEthAccMiiLinkStatus(UINT32 phyAddr, 
		      BOOL *linkUp,
		      BOOL *speed100, 
		      BOOL *fullDuplex,
		      BOOL *autoneg)
{
    UINT16 regval;
    /*Need to read the register twice here to flush PHY*/
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_STAT_REG, &regval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_STAT_REG, &regval);

    if((regval & IX_ETH_ACC_MII_SR_LINK_STATUS) != 0)
    {
	*linkUp = TRUE;
    }
    else
    {
	*linkUp = FALSE;
    }


    ixEthAccMiiReadRtn(phyAddr, IX_ETH_ACC_MII_STAT2_REG, &regval);  

    if((regval & IX_ETH_ACC_MII_SR2_100) != 0)
    {
	*speed100 = TRUE;
    }
    else
    {
	*speed100 = FALSE;
    }

    if((regval & IX_ETH_ACC_MII_SR2_FD) != 0)
    {
	*fullDuplex = TRUE; 
    }
    else
    {
	*fullDuplex = FALSE;
    }
    if((regval & IX_ETH_ACC_MII_SR2_AUTO) != 0)
    {
	*autoneg = TRUE;
    }
    else
    {
	*autoneg = FALSE;
    }
    
    return IX_ETH_ACC_SUCCESS;   
}



IxEthAccStatus
ixEthAccMiiShow (UINT32 phyAddr)
{
    UINT16 regval;
    /*Need to read the register twice here to flush PHY*/
    if(ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_STAT_REG, &regval) 
       != IX_ETH_ACC_SUCCESS)
    {
	return IX_ETH_ACC_FAIL;
    }


    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_STAT_REG, &regval);
    printf("PHY Status: \n");
    if((regval & IX_ETH_ACC_MII_SR_LINK_STATUS) != 0)
    {
	printf("    Link is Up\n");
    }
    else
    {
	printf("    Link is Down\n");
    }
    if((regval & IX_ETH_ACC_MII_SR_REMOTE_FAULT) != 0)
    {
	printf("    Remote fault detected\n");
    }
    if((regval & IX_ETH_ACC_MII_SR_AUTO_NEG) != 0)
    {
	printf("    Auto Negotiation Completed\n");
    }
    else
    {
	printf("    Auto Negotiation Not Completed\n");
    }


    ixEthAccMiiReadRtn(phyAddr, IX_ETH_ACC_MII_STAT2_REG, &regval);
    printf("PHY Configuration:\n");

    if((regval & IX_ETH_ACC_MII_SR2_100) != 0)
    {
	printf("    Speed: 100Mb/s\n");
    }
    else
    {
	printf("    Speed: 10Mb/s\n");
    }
    if((regval & IX_ETH_ACC_MII_SR2_FD) != 0)
    {
	printf("    Full Duplex\n");
    }
    else
    {
	printf("    Half Duplex\n");
    }
    if((regval & IX_ETH_ACC_MII_SR2_AUTO) != 0)
    {
	printf("    Auto Negotiation Enabled\n");
    }
    else
    {
	printf("    Auto Negotiation Disabled\n");
    }
    return IX_ETH_ACC_SUCCESS;
}






IxEthAccStatus
ixEthAccMiiStatsShow (UINT32 phyAddr)
{
    UINT16 regval;
    printf("Regisers on PHY at address 0x%x\n", phyAddr);
    ixEthAccMiiReadRtn(phyAddr, IX_ETH_ACC_MII_CTRL_REG, &regval);
    ixEthAccMiiReadRtn(phyAddr, IX_ETH_ACC_MII_CTRL_REG, &regval);
    printf("    Control Register                  :      0x%x\n", regval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_STAT_REG, &regval);
    printf("    Status Register                   :      0x%x\n", regval);   
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_PHY_ID1_REG, &regval);
    printf("    PHY ID1 Register                  :      0x%x\n", regval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_PHY_ID2_REG, &regval);
    printf("    PHY ID2 Register                  :      0x%x\n", regval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_AN_ADS_REG, &regval);
    printf("    Auto Neg ADS Register             :      0x%x\n", regval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_AN_PRTN_REG, &regval);
    printf("    Auto Neg Partner Ability Register :      0x%x\n", regval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_AN_EXP_REG, &regval);
    printf("    Auto Neg Expansion Register       :      0x%x\n", regval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_AN_NEXT_REG, &regval);
    printf("    Auto Neg Next Register            :      0x%x\n", regval);

    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_STAT_REG, &regval);
    
    printf("This local PHY's Capabilities:\n");
    if((regval & IX_ETH_ACC_MII_SR_AUTO_SEL) != 0)
    {
	printf("    Auto Speed Select capable PHY\n");
    }
    else
    {
	printf("    Non Auto Speed Select capable PHY\n");
    }
    if((regval & IX_ETH_ACC_MII_SR_10T_HALF_DPX) != 0)
    {
	printf("    10T Half Duplex Capable PHY\n");
    }
    else
    {
	printf("    Non 10T Half Duplex Capable PHY\n");
    }
    if((regval & IX_ETH_ACC_MII_SR_10T_FULL_DPX) != 0)
    {
	printf("    10T Full Duplex Capable PHY\n");
    }
    else
    {
	printf("    Non 10T Full Duplex Capable PHY\n");
    }
    if((regval & IX_ETH_ACC_MII_SR_TX_HALF_DPX) != 0)
    {
	printf("    TX HD capable\n");
    }
    else
    {
	printf("    Non TX HD capable\n");
    }
    if((regval & IX_ETH_ACC_MII_SR_TX_FULL_DPX) != 0)
    {
	printf("    TX FD capable\n");
    }
    else
    {
	printf("    Non TX FD capable\n");
    }
    if((regval & IX_ETH_ACC_MII_SR_T4) != 0)
    {
	printf("    T4 capable\n");
    }

    printf("PHY Status: \n");
    if((regval & IX_ETH_ACC_MII_SR_LINK_STATUS) != 0)
    {
	printf("    Link is Up\n");
    }
    else
    {
	printf("    Link is Down\n");
    }
    if((regval & IX_ETH_ACC_MII_SR_REMOTE_FAULT) != 0)
    {
	printf("    Remote fault detected\n");
    }
    else
    {
	printf("    No Remote fault detected\n");
    }
    if((regval & IX_ETH_ACC_MII_SR_AUTO_NEG) != 0)
    {
	printf("    Auto Negotiation Completed\n");
    }
    else
    {
	printf("    Auto Negotiation Not Completed\n");
    }


    ixEthAccMiiReadRtn(phyAddr, IX_ETH_ACC_MII_STAT2_REG, &regval);
    printf("PHY Configuration:\n");

    if((regval & IX_ETH_ACC_MII_SR2_100) != 0)
    {
	printf("    Speed: 100Mb/s\n");
    }
    else
    {
	printf("    Speed: 10Mb/s\n");
    }
    if((regval & IX_ETH_ACC_MII_SR2_FD) != 0)
    {
	printf("    Full Duplex\n");
    }
    else
    {
	printf("    Half Duplex\n");
    }
    if((regval & IX_ETH_ACC_MII_SR2_AUTO) != 0)
    {
	printf("    Auto Negotiation Enabled\n");
    }
    else
    {
	printf("    Auto Negotiation Disabled\n");
    }
    return IX_ETH_ACC_SUCCESS;
}


IxEthAccStatus
ixEthAccMdioShow (void)
{
    UINT32 regval;
    ixEthAccMdioCmdRead(&regval);
    
    printf("MDIO command register\n");
    printf("    Go bit      : 0x%x\n", (regval & BIT(31)) >> 31);
    printf("    MDIO Write  : 0x%x\n", (regval & BIT(26)) >> 26);
    printf("    PHY address : 0x%x\n", (regval >> 21) & 0x1f);
    printf("    Reg address : 0x%x\n", (regval >> 16) & 0x1f);
	
    ixEthAccMdioStatusRead(&regval);
    printf("MDIO status register\n");
    printf("    Read OK     : 0x%x\n", (regval & BIT(31)) >> 31);
    printf("    Read Data   : 0x%x\n", (regval >> 16) & 0xff);
    return IX_ETH_ACC_SUCCESS;   
}
