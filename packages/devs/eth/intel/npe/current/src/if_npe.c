//==========================================================================
//
//      if_npe.c
//
//	Intel NPE ethernet driver
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
// -------------------------------------------
//
// Portions of this file may have been derived from Intel software
// covered by the following license:
//
// -- Intel Copyright Notice --
//
// INTEL CONFIDENTIAL
//
// Copyright 2002 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents
// related to the source code ("Material") are owned by Intel Corporation
// or its suppliers or licensors.  Title to the Material remains with
// Intel Corporation or its suppliers and licensors.  The Material
// contains trade secrets and proprietary and confidential information of
// Intel or its suppliers and licensors.  The Material is protected by
// worldwide copyright and trade secret laws and treaty provisions. No
// part of the Material may be used, copied, reproduced, modified,
// published, uploaded, posted, transmitted, distributed, or disclosed in
// any way without Intel's prior express written permission.
//
// No license under any patent, copyright, trade secret or other
// intellectual property right is granted to or conferred upon you by
// disclosure or delivery of the Materials, either expressly, by
// implication, inducement, estoppel or otherwise.  Any license under
// such intellectual property rights must be express and approved by
// Intel in writing.
//
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter
// Date:         2003-03-20
// Purpose:      
// Description:  hardware driver for Intel Network Processors.
// Notes:
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_eth_drivers.h>
#include <pkgconf/devs_eth_intel_npe.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>

#include <IxNpeDl.h>
#include <IxQMgr.h>
#include <IxNpeMh.h>
#include <IxEthAcc.h>
#include <ix_ossl.h>

#include <npe_info.h>

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#include <redboot.h>
#include <flash_config.h>
#endif

#define NPE_INT_BITS ((1 << CYGNUM_HAL_INTERRUPT_NPEB) | \
                      (1 << CYGNUM_HAL_INTERRUPT_NPEC) | \
                      (1 << CYGNUM_HAL_INTERRUPT_QM1))

#include CYGDAT_DEVS_ETH_INTEL_NPE_INL

#ifdef CYGSEM_INTEL_NPE_USE_ETH0
static struct npe npe_eth0_priv_data = { 
    npe_id: CYGNUM_ETH0_NPE_ID,
    eth_id: CYGNUM_ETH0_ETH_ID,
    phy_no: CYGNUM_ETH0_PHY_NO,
#if defined(CYGSEM_NPE_REDBOOT_HOLDS_ESA)
    mac_address: CYGDAT_ETH0_DEFAULT_ESA
#endif
};
#endif

#ifdef CYGSEM_INTEL_NPE_USE_ETH1
static struct npe npe_eth1_priv_data = { 
    npe_id: CYGNUM_ETH1_NPE_ID,
    eth_id: CYGNUM_ETH1_ETH_ID,
    phy_no: CYGNUM_ETH1_PHY_NO,
#if defined(CYGSEM_NPE_REDBOOT_HOLDS_ESA)
    mac_address: CYGDAT_ETH1_DEFAULT_ESA
#endif
};
#endif

#ifdef CYGSEM_NPE_REDBOOT_HOLDS_ESA
#ifdef CYGSEM_INTEL_NPE_USE_ETH0
RedBoot_config_option("Network hardware address [MAC] for NPE eth0",
                      npe_eth0_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, npe_eth0_priv_data.mac_address
    );
#endif CYGSEM_INTEL_NPE_USE_ETH0

#ifdef CYGSEM_INTEL_NPE_USE_ETH1
RedBoot_config_option("Network hardware address [MAC] for NPE eth1",
                      npe_eth1_esa,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, npe_eth1_priv_data.mac_address
    );
#endif // CYGSEM_INTEL_NPE_USE_ETH1
#endif // CYGSEM_NPE_REDBOOT_HOLDS_ESA

#if defined(CYGSEM_INTEL_NPE_USE_ETH0) && defined(CYGSEM_INTEL_NPE_USE_ETH1)
#define MAX_PORTS 2
#else
#define MAX_PORTS 1
#endif

#ifdef CYGPKG_REDBOOT
#define ACTIVE_PORTS 1
#else
#define ACTIVE_PORTS MAX_PORTS
#endif

#define NPE_PKT_SIZE 1600

#define NPE_MBUF_POOL_SIZE                               \
	((CYGNUM_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS + \
 	 CYGNUM_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS) * \
	sizeof(M_BLK) * ACTIVE_PORTS)

#define NPE_PKT_POOL_SIZE                                \
	((CYGNUM_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS + \
 	 CYGNUM_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS) * \
	NPE_PKT_SIZE * ACTIVE_PORTS)

#define NPE_MEM_POOL_SIZE (NPE_MBUF_POOL_SIZE + NPE_PKT_POOL_SIZE)


// A little extra so we can align to cacheline.
static cyg_uint8 npe_alloc_pool[NPE_MEM_POOL_SIZE + HAL_DCACHE_LINE_SIZE - 1];
static cyg_uint8 *npe_alloc_end;
static cyg_uint8 *npe_alloc_free;

static void*
npe_alloc(int size)
{
    void *p = NULL;

    size = (size + (HAL_DCACHE_LINE_SIZE-1)) & ~(HAL_DCACHE_LINE_SIZE-1);
    if ((npe_alloc_free + size) < npe_alloc_end) {
        p = npe_alloc_free;
        npe_alloc_free += size;
    }
    return p;
}


// Not interrupt safe!
static void
mbuf_enqueue(M_BLK **q, M_BLK *new)
{
    IX_MBUF *m = *q;

    new->m_nextpkt = NULL;
    if (m) {
        while(m->m_nextpkt)
            m = m->m_nextpkt;
        m->m_nextpkt = new;
    } else
        *q = new;
}

// Not interrupt safe!
static M_BLK *
mbuf_dequeue(M_BLK **q)
{
    M_BLK *m = *q;
    if (m)
	*q = m->m_nextpkt;
    return m;
}


static void
reset_tx_mbufs(struct npe* p_npe)
{
    M_BLK *m;
    int i;

    p_npe->txQHead = NULL;

    for (i = 0; i < CYGNUM_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS; i++) {
	m = &p_npe->tx_mbufs[i];
	m->m_data = &p_npe->tx_pkts[i * NPE_PKT_SIZE];
	m->m_len = m->mBlkPktHdr.len = NPE_PKT_SIZE;
	m->m_next = NULL;
	m->m_flags = 0;
	mbuf_enqueue(&p_npe->txQHead, m);
    }
}

static void
reset_rx_mbufs(struct npe* p_npe)
{
    M_BLK *m;
    int i;

    p_npe->rxQHead = NULL;

    HAL_DCACHE_INVALIDATE(p_npe->rx_pkts, NPE_PKT_SIZE *
			  CYGNUM_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS);
 
    for (i = 0; i < CYGNUM_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS; i++) {
	m = &p_npe->rx_mbufs[i];
	m->m_data = &p_npe->rx_pkts[i * NPE_PKT_SIZE];
	m->m_len = m->mBlkPktHdr.len = NPE_PKT_SIZE;
	m->m_next = NULL;
	m->m_flags = 0;

	if(ixEthAccPortRxFreeReplenish(p_npe->eth_id, m) != IX_SUCCESS) {
#ifdef DEBUG
	    diag_printf("ixEthAccPortRxFreeReplenish failed for port %d\n",
			p_npe->eth_id);
#endif
	    break;
	}
    }
}

static void
init_rx_mbufs(struct npe* p_npe)
{
    p_npe->rxQHead = NULL;

    p_npe->rx_pkts = npe_alloc(NPE_PKT_SIZE *
			       CYGNUM_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS);
    if (p_npe->rx_pkts == NULL) {
#ifdef DEBUG
	diag_printf("alloc of packets failed.\n");
#endif
	return;
    }

    p_npe->rx_mbufs = (M_BLK *)npe_alloc(sizeof(M_BLK) *
					 CYGNUM_DEVS_ETH_INTEL_NPE_MAX_RX_DESCRIPTORS);
    if (p_npe->rx_mbufs == NULL) {
#ifdef DEBUG
	diag_printf("alloc of mbufs failed.\n");
#endif
	return;
    }

    reset_rx_mbufs(p_npe);
}


static void
init_tx_mbufs(struct npe* p_npe)
{
    p_npe->tx_pkts = npe_alloc(NPE_PKT_SIZE *
			       CYGNUM_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS);
    if (p_npe->tx_pkts == NULL) {
#ifdef DEBUG
	diag_printf("alloc of packets failed.\n");
#endif
	return;
    }

    p_npe->tx_mbufs = (M_BLK *)npe_alloc(sizeof(M_BLK) *
					 CYGNUM_DEVS_ETH_INTEL_NPE_MAX_TX_DESCRIPTORS);
    if (p_npe->tx_mbufs == NULL) {
#ifdef DEBUG
	diag_printf("alloc of mbufs failed.\n");
#endif
	return;
    }

    reset_tx_mbufs(p_npe);
}


#define NPE_VERSIONS_COUNT 12
static int
npeDownload(cyg_uint32 npeId, cyg_uint32 buildId)
{
    cyg_uint32 i, n;
    IxNpeDlVersionId list[NPE_VERSIONS_COUNT];
    IxNpeDlVersionId dlVersion;
    int major, minor;

    if (ixNpeDlAvailableVersionsCountGet(&n) != IX_SUCCESS)
	return 0;

    if (n > NPE_VERSIONS_COUNT)
	return 0;

    if (ixNpeDlAvailableVersionsListGet(list, &n) != IX_SUCCESS)
	return 0;
    
    major = minor = 0;
    for (i = 0; i < n; i++) {
        if (list[i].npeId == npeId) {
            if (list[i].buildId == buildId) {
		if (list[i].major > major) {
		    major = list[i].major;
		    minor = list[i].minor;
                } else if (list[i].major == major && list[i].minor > minor)
		    minor = list[i].minor;
            }
	}
    }

#ifdef DEBUG
    diag_printf("NPE%c major[%d] minor[%d] build[%d]\n",
	      npeId  + 'A', major, minor, buildId);
#endif

    if (ixNpeDlNpeStopAndReset(npeId) != IX_SUCCESS) {
#ifdef DEBUG
	diag_printf("npeDownload: Failed to stop/reset NPE%c\n", npeId  + 'A');
#endif
        return 0;
    }

    dlVersion.npeId = npeId;
    dlVersion.buildId = buildId;
    dlVersion.major = major;
    dlVersion.minor = minor;

    if (ixNpeDlVersionDownload(&dlVersion, 1) != IX_SUCCESS) {
#ifdef DEBUG
	diag_printf("Failed to download to NPE%c\n", npeId  + 'A');
#endif
        return 0;
    }

#if 0
    // verify download
    if (ixNpeDlLoadedVersionGet(npeId, &dlVersion) != IX_SUCCESS) {
#ifdef DEBUG
	diag_printf("Failed to upload version from NPE%c\n", npeId  + 'A');
#endif
        return 0;
    }

    if (dlVersion.buildId != buildId || dlVersion.major != major || dlVersion.major != major) {
#ifdef DEBUG
	diag_printf("Failed to verify download NPE%c\n", npeId  + 'A');
#endif
        return 0;
    }
#endif

    return 1;
}


// Returns non-zero if link is up.
static int
link_check(unsigned int phyNo)
{
    BOOL fullDuplex, linkUp, speed, autoneg;

    ixEthAccMiiLinkStatus(phyNo, &linkUp, &speed, &fullDuplex, &autoneg);
    if (linkUp == FALSE) {
	int retry = 20; /* 2 seconds */
#ifdef DEBUG
	diag_printf("Wait for PHY %u to be ready ...", phyNo);
#endif
	while (linkUp == FALSE && retry-- > 0) {
	    CYGACC_CALL_IF_DELAY_US((cyg_int32)100000);
	    ixEthAccMiiLinkStatus(phyNo, &linkUp, &speed, &fullDuplex, &autoneg);
	}
	if (linkUp == FALSE) 
	    return 0;
    }
    return 1;
}

// Returns non-zero if given PHY is present.
static int
phy_present(int phyno)
{
    static BOOL phyPresent[IXP425_ETH_ACC_MII_MAX_ADDR];
    static int scanned = 0;

    if (!scanned) {
	if(ixEthAccMiiPhyScan(phyPresent) != IX_ETH_ACC_SUCCESS)
	    return 0;
	scanned = 1;

#ifdef DEBUG
	{
	    int i;
	    for(i = 0; i < IXP425_ETH_ACC_MII_MAX_ADDR; i++)
		diag_printf("phyPresent[%d] = %d\n", i, phyPresent[i]);
	}
#endif
    }

    return (phyno < (int)(sizeof(phyPresent)/sizeof(phyPresent[0])) && phyPresent[phyno]);
}


// Initialize given PHY for given port.
// Returns non-zero if successful.
static int
phy_init(int portno, int phyno)
{
    BOOL speed, linkUp, fullDuplex, autoneg;

    ixEthAccMiiPhyReset(phyno);

    ixEthAccMiiPhyConfig(phyno, TRUE, TRUE, TRUE);

    // wait until the link is up before setting the MAC duplex
    // mode, the PHY duplex mode may change after autonegotiation 
    (void)link_check(phyno);

    ixEthAccMiiLinkStatus(phyno, &linkUp, &speed, &fullDuplex, &autoneg);

    /* Set the MAC duplex mode */
    ixEthAccPortDuplexModeSet(portno, fullDuplex ? IX_ETH_ACC_FULL_DUPLEX :
				                  IX_ETH_ACC_HALF_DUPLEX);

#ifdef DEBUG
    diag_printf("\nPHY %d configuration:\n", phyno);
    ixEthAccMiiShow(phyno);
#endif

    return 1;
}


// ethAcc RX callback
static void
npe_rx_callback(cyg_uint32 cbTag, M_BLK *m, IxEthAccPortId portid)
{
    struct npe* p_npe = (struct npe *)cbTag;
    struct eth_drv_sc *sc;

    sc = p_npe->sc;

    mbuf_enqueue(&p_npe->rxQHead, m);

    (sc->funs->eth_drv->recv)(sc, m->m_len);

    // Now return mbuf to NPE
    m->m_len = m->mBlkPktHdr.len = NPE_PKT_SIZE;
    m->m_next = NULL;
    m->m_flags = 0;
    
    HAL_DCACHE_INVALIDATE(m->m_data, NPE_PKT_SIZE);
 
    if(ixEthAccPortRxFreeReplenish(p_npe->eth_id, m) != IX_SUCCESS) {
#ifdef DEBUG
	diag_printf("npe_rx_callback: Error returning mbuf.\n");
#endif
    }
}

// callback which is used by ethAcc to recover RX buffers when stopping
static void
npe_rx_stop_callback(cyg_uint32 cbTag, M_BLK *m, IxEthAccPortId portid)
{
}


// ethAcc TX callback
static void
npe_tx_callback(cyg_uint32 cbTag, M_BLK *m)
{
    struct npe* p_npe = (struct npe *)cbTag;
    struct eth_drv_sc *sc;

    sc = p_npe->sc;

    (sc->funs->eth_drv->tx_done)(sc, m->m_key, 1);
    
    m->m_len = m->mBlkPktHdr.len = NPE_PKT_SIZE;
    m->m_next = NULL;
    m->m_flags = 0;
    mbuf_enqueue(&p_npe->txQHead, m);
}


// callback which is used by ethAcc to recover TX buffers when stopping
static void
npe_tx_stop_callback(cyg_uint32 cbTag, M_BLK *m)
{
}


// ------------------------------------------------------------------------
//
//  API Function : npe_init
//
// ------------------------------------------------------------------------
static bool
npe_init(struct cyg_netdevtab_entry * ndp)
{
    static int initialized = 0;
    struct eth_drv_sc *sc;
    struct npe *p_npe;
    int mac_ok = false;
    IxEthAccMacAddr  npeMac;

    sc = (struct eth_drv_sc *)ndp->device_instance;
    p_npe = (struct npe *)sc->driver_private;

    p_npe->sc = sc;

    if (!initialized) {

	// One time initialization common to all ports
	initialized = 1;

	npe_alloc_end = npe_alloc_pool + sizeof(npe_alloc_pool);
	npe_alloc_free = (cyg_uint8 *)(((unsigned)npe_alloc_pool + HAL_DCACHE_LINE_SIZE - 1)
				       & ~(HAL_DCACHE_LINE_SIZE - 1));

	if (ixQMgrInit() != IX_SUCCESS) {
#ifdef DEBUG
	    diag_printf("Error initialising queue manager!\n");
#endif
	    return 0;
	}

	if (!npeDownload(IX_NPEDL_NPEID_NPEB, 0))
	    return 0;

	if (!npeDownload(IX_NPEDL_NPEID_NPEC, 0))
	    return 0;

	if(ixNpeMhInitialize(IX_NPEMH_NPEINTERRUPTS_YES) != IX_SUCCESS) {
#ifdef DEBUG
	    diag_printf("Error initialising NPE Message handler!\n");
#endif
	    return 0;
	}

	if(ixNpeDlNpeExecutionStart(IX_NPEDL_NPEID_NPEB) != IX_SUCCESS) {
#ifdef DEBUG
	    diag_printf("Error starting NPEB!\n");
#endif
	    return 0;
	}

	if(ixNpeDlNpeExecutionStart(IX_NPEDL_NPEID_NPEC) != IX_SUCCESS) {
#ifdef DEBUG
	    diag_printf("Error starting NPEB!\n");
#endif
	    return 0;
	}

	if (ixEthAccInit() != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	    diag_printf("Error initialising Ethernet access driver!\n");
#endif
	    return 0;
	}

	if(ixEthAccPortInit(IX_ETH_PORT_1) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	    diag_printf("Error initialising Ethernet port1!\n");	
#endif
	    return 0;
	}

	if(ixEthAccPortInit(IX_ETH_PORT_2) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	    diag_printf("Error initialising Ethernet port2!\n");	
#endif
	    return 0;
	}
    }

    if (!phy_present(p_npe->phy_no)) {
#ifdef DEBUG
	diag_printf("No phy for NPE%c found!\n", 'A' + p_npe->npe_id);
#endif
	return 0;
    }

    if(!phy_init(p_npe->eth_id, p_npe->phy_no)) {
#ifdef DEBUG
	diag_printf("Error initialising Ethernet phy(s)!\n");
#endif
	return 0;
    }


    // Set MAC address
#if defined(CYGSEM_NPE_REDBOOT_HOLDS_ESA)
    {
	char *cfgname = NULL;
#if defined(CYGSEM_INTEL_NPE_USE_ETH0)
	if (p_npe == &npe_eth0_priv_data)
	    cfgname = "npe_eth0_esa";
#endif
#if defined(CYGSEM_INTEL_NPE_USE_ETH1)
	if (p_npe == &npe_eth1_priv_data)
	    cfgname = "npe_eth1_esa";
#endif
        mac_ok = CYGACC_CALL_IF_FLASH_CFG_OP(CYGNUM_CALL_IF_FLASH_CFG_GET, cfgname,
					     p_npe->mac_address, CONFIG_ESA);
    }
#elif defined(CYGHAL_GET_NPE_ESA)
    CYGHAL_GET_NPE_ESA(p_npe->eth_id, p_npe->mac_address, mac_ok);
#else
#error No mechanism to get MAC address
#endif

    if (!mac_ok) {
#ifdef DEBUG
	diag_printf("Error getting MAC address.\n");
#endif
	return 0;
    }

    npeMac.macAddress[0] = p_npe->mac_address[0];
    npeMac.macAddress[1] = p_npe->mac_address[1];
    npeMac.macAddress[2] = p_npe->mac_address[2];
    npeMac.macAddress[3] = p_npe->mac_address[3];
    npeMac.macAddress[4] = p_npe->mac_address[4];
    npeMac.macAddress[5] = p_npe->mac_address[5];

    ixEthAccPortUnicastMacAddressSet(p_npe->eth_id, &npeMac);

#ifdef DEBUG
    diag_printf("npe_init: MAC Address = %02X %02X %02X %02X %02X %02X\n",
              p_npe->mac_address[0], p_npe->mac_address[1],
              p_npe->mac_address[2], p_npe->mac_address[3],
              p_npe->mac_address[4], p_npe->mac_address[5]);
#endif

    // initialize mbuf pool
    init_rx_mbufs(p_npe);
    init_tx_mbufs(p_npe);

    // set scheduling discipline
    ixEthAccTxSchedulingDisciplineSet(p_npe->eth_id, FIFO_NO_PRIORITY);

    if (ixEthAccPortRxCallbackRegister(p_npe->eth_id, npe_rx_callback,
				       (cyg_uint32)p_npe) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("Error registering rx callback!\n");
#endif
	return 0;
    }

    if (ixEthAccPortTxDoneCallbackRegister(p_npe->eth_id, npe_tx_callback,
					   (cyg_uint32)p_npe) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("Error registering tx callback!\n");
#endif
	return 0;
    }

    if (ixEthAccPortRxFrameAppendFCSDisable(p_npe->eth_id) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("Error disabling RX FCS.\n");
#endif
	return 0;
    }

    if(ixEthAccPortTxFrameAppendFCSEnable(p_npe->eth_id) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("Error enabling TX FCS.\n");
#endif
	return 0;
    }

    if (ixEthAccPortEnable(p_npe->eth_id) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("Error enabling port\n");
#endif
	return 0;
    }

    p_npe->active = 1;

    // Initialize upper level driver
    (sc->funs->eth_drv->init)(sc, &(p_npe->mac_address[0]) );

    return 1;
}

// ------------------------------------------------------------------------
//
//  API Function : npe_start
//
// ------------------------------------------------------------------------
static void 
npe_start( struct eth_drv_sc *sc, unsigned char *enaddr, int flags )
{
#ifndef CYGPKG_REDBOOT
    struct npe *p_npe = (struct npe *)sc->driver_private;

    if (ixEthAccPortRxCallbackRegister(p_npe->eth_id, npe_rx_callback,
				       (cyg_uint32)p_npe) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("Error registering rx callback!\n");
#endif
    }

    if (ixEthAccPortTxDoneCallbackRegister(p_npe->eth_id, npe_tx_callback,
					   (cyg_uint32)p_npe) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("Error registering tx callback!\n");
#endif
    }

    if(ixEthAccPortEnable(p_npe->eth_id) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("npe_start: Error disabling NPE%c!\n", 'A' + p_npe->npe_id);
#endif
    }

    p_npe->active = 1;
#endif
}

// ------------------------------------------------------------------------
//
//  API Function : npe_stop
//
// ------------------------------------------------------------------------
static void
npe_stop( struct eth_drv_sc *sc )
{
    struct npe *p_npe = (struct npe *)sc->driver_private;
    int i;

    if (ixEthAccPortRxCallbackRegister(p_npe->eth_id, npe_rx_stop_callback,
				       (cyg_uint32)p_npe) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("Error registering rx callback!\n");
#endif
    }

    if (ixEthAccPortTxDoneCallbackRegister(p_npe->eth_id, npe_tx_stop_callback,
					   (cyg_uint32)p_npe) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("Error registering tx callback!\n");
#endif
    }

    if(ixEthAccPortDisable(p_npe->eth_id) != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("npe_stop: Error disabling NPEB!\n");
#endif
    }

    // Delay to give time for recovery of mbufs
    for (i = 0; i < 100; i++) {
	CYGACC_CALL_IF_DELAY_US((cyg_int32)10000);
	ixNpeMhMessagesReceive(p_npe->npe_id);
	ixQMgrDispatcherLoopRun(IX_QMGR_QUELOW_GROUP);
    }

// For RedBoot only, we are probably launching Linux or other OS that
// needs a clean slate for its NPE library.	
#ifdef CYGPKG_REDBOOT
    if (ixNpeDlNpeStopAndReset(IX_NPEDL_NPEID_NPEB) != IX_SUCCESS)
        diag_printf ("Failed to stop and reset NPE B.\n");

    if (ixNpeDlNpeStopAndReset(IX_NPEDL_NPEID_NPEC) != IX_SUCCESS)
        diag_printf ("Failed to stop and reset NPE C.\n");
#endif
    p_npe->active = 0;
}


// ------------------------------------------------------------------------
//
//  API Function : npe_recv
//
// ------------------------------------------------------------------------
static void 
npe_recv( struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len )
{
    struct npe *p_npe = (struct npe *)sc->driver_private;
    struct eth_drv_sg *sg = sg_list;
    M_BLK *m;
    cyg_uint8 *src;
    int len;

    m = mbuf_dequeue(&p_npe->rxQHead);
    src = m->m_data;

    while (sg < &sg_list[sg_len]) {
            
        len = sg->len;

        if (len < 0 || sg->buf == 0)
            return;

        if (len > m->m_len)
            len = m->m_len;

        memcpy((char *)sg->buf, src, len);
        src += len;
        m->m_len -= len;

	++sg;
    }
}

// ------------------------------------------------------------------------
//
//  API Function : npe_can_send
//
// ------------------------------------------------------------------------
static int 
npe_can_send(struct eth_drv_sc *sc)
{
    struct npe *p_npe = (struct npe *)sc->driver_private;

    return p_npe->active && p_npe->txQHead != NULL;
}

// ------------------------------------------------------------------------
//
//  API Function : npe_send
//
// ------------------------------------------------------------------------
static void 
npe_send(struct eth_drv_sc *sc,
	 struct eth_drv_sg *sg_list, int sg_len,
	 int total_len, unsigned long key)
{
    struct npe *p_npe = (struct npe *)sc->driver_private;
    struct eth_drv_sg *sg = sg_list;
    cyg_uint8 *dest;
    int len;
    M_BLK *m;

    m = mbuf_dequeue(&p_npe->txQHead);

    dest = m->m_data;
    if (total_len > m->m_len)
	total_len = m->m_len;

    m->mBlkPktHdr.len = m->m_len = total_len;
    m->m_key = key;

    while (sg < &sg_list[sg_len] && total_len > 0) {

	len = sg->len;
	if (len > total_len)
	    len = total_len;

	memcpy(dest, (char *)sg->buf, len);

	dest += len;
	total_len -= len;

	++sg;
    }

    HAL_DCACHE_FLUSH(m->m_data, m->m_len);

    if(ixEthAccPortTxFrameSubmit(p_npe->eth_id, m, IX_ETH_ACC_TX_DEFAULT_PRIORITY)
       != IX_ETH_ACC_SUCCESS) {
#ifdef DEBUG
	diag_printf("npe_send: Can't submit frame.\n");
#endif
	mbuf_enqueue(&p_npe->txQHead, m);
	return;
    }
}

// ------------------------------------------------------------------------
//
//  API Function : npe_deliver
//
// ------------------------------------------------------------------------
static void
npe_deliver(struct eth_drv_sc *sc)
{
}

// ------------------------------------------------------------------------
//
//  API Function : npe_poll
//
// ------------------------------------------------------------------------
static void
npe_poll(struct eth_drv_sc *sc)
{
    struct npe *p_npe = (struct npe *)sc->driver_private;
    int i;
    cyg_uint32 ints;

    ints = *IXP425_INTR_EN;
    *IXP425_INTR_EN = ints & ~NPE_INT_BITS;
    
    ixNpeMhMessagesReceive(p_npe->npe_id);
    ixQMgrDispatcherLoopRun (IX_QMGR_QUELOW_GROUP);

    *IXP425_INTR_EN = ints;
}


// ------------------------------------------------------------------------
//
//  API Function : npe_int_vector
//
// ------------------------------------------------------------------------
static int
npe_int_vector(struct eth_drv_sc *sc)
{
    struct npe *p_npe;
    p_npe = (struct npe *)sc->driver_private;

    if (p_npe->eth_id == IX_ETH_PORT_1)
	return CYGNUM_HAL_INTERRUPT_NPEB;
    else if (p_npe->eth_id == IX_ETH_PORT_2)
	return CYGNUM_HAL_INTERRUPT_NPEC;
    return -1;
}


// ------------------------------------------------------------------------
//
//  API Function : npe_ioctl
//
// ------------------------------------------------------------------------
static int
npe_ioctl(struct eth_drv_sc *sc, unsigned long key,
	  void *data, int data_length)
{
    return -1;
}


#ifdef CYGSEM_INTEL_NPE_USE_ETH0
ETH_DRV_SC(npe_sc0,
           &npe_eth0_priv_data,
           CYGDAT_NPE_ETH0_NAME,
           npe_start,
           npe_stop,
           npe_ioctl,
           npe_can_send,
           npe_send,
           npe_recv,
           npe_deliver,
           npe_poll,
           npe_int_vector);

NETDEVTAB_ENTRY(npe_netdev0, 
                "npe_" CYGDAT_NPE_ETH0_NAME,
                npe_init, 
                &npe_sc0);
#endif // CYGSEM_INTEL_NPE_USE_ETH0

#ifdef CYGSEM_INTEL_NPE_USE_ETH1
ETH_DRV_SC(npe_sc1,
           &npe_eth1_priv_data,
           CYGDAT_NPE_ETH1_NAME,
           npe_start,
           npe_stop,
           npe_ioctl,
           npe_can_send,
           npe_send,
           npe_recv,
           npe_deliver,
           npe_poll,
           npe_int_vector);

NETDEVTAB_ENTRY(npe_netdev1, 
                "npe_" CYGDAT_NPE_ETH1_NAME,
                npe_init, 
                &npe_sc1);
#endif // CYGSEM_INTEL_NPE_USE_ETH1

// ------------------------------------------------------------------------
// EOF if_npe.c
