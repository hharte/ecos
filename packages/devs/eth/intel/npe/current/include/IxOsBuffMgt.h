/**
 * @file
 *
 * @brief This file includes the OS dependant MBUF header files.
 *
 * Decision for ixp425 Software release 1.0 is to use VxWorks MBUFs directly.
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
 * @defgroup IxOsBuffMgt IXP4xx OS Memory Buffer Management (IxOsBuffMgt) API
 *
 * @brief IXP4xx OS Memory Buffer Management
 *
 * @{
 */

#ifndef IxOsBuffMgt_inc
#define IxOsBuffMgt_inc

#if defined __vxworks

#include <vxWorks.h>
#include <net/mbuf.h>
#include <netBufLib.h>

#elif defined __ECOS

#include <npe_mbuf.h>

#else 

#include "mbuf.h"

#endif /* __vxworks */

/* common definitions */

/**
 * @def IX_MBUF 
 *
 * @brief Memory buffer
 */
#define IX_MBUF M_BLK

/**
 * @def IX_MBUF_MDATA(m_blk_ptr)
 *
 * @brief Return pointer to the data in the mbuf
 */
#define IX_MBUF_MDATA(m_blk_ptr)   		   (m_blk_ptr)->m_data

/**
 * @def IX_MBUF_MLEN(m_blk_ptr)
 *
 * @brief Return pointer to the data length
 */
#define IX_MBUF_MLEN(m_blk_ptr)   		   (m_blk_ptr)->m_len

/**
 * @def IX_MBUF_TYPE(m_blk_ptr)
 *
 * @brief Return pointer to the data type in the mbuf
 */
#define IX_MBUF_TYPE(m_blk_ptr)   		   (m_blk_ptr)->m_type

/**
 * @def IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(m_blk_ptr)
 *
 * @brief Return pointer to the next mbuf in a single packet
 */
#define IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(m_blk_ptr)  (m_blk_ptr)->m_next

/**
 * @def IX_MBUF_NEXT_PKT_IN_CHAIN_PTR(m_blk_ptr)
 *
 * @brief Return pointer to the next packet in the chain
 */
#define IX_MBUF_NEXT_PKT_IN_CHAIN_PTR(m_blk_ptr)   (m_blk_ptr)->m_nextpkt

/**
 * @def IX_MBUF_ALLOCATED_BUFF_LEN(m_blk_ptr)
 *
 * @brief Return pointer to the allocated buffer size
 */
#define IX_MBUF_ALLOCATED_BUFF_LEN(m_blk_ptr)      (m_blk_ptr)->m_len

/**
 * @def IX_MBUF_PKT_LEN(m_blk_ptr)
 *
 * @brief Return pointer to the total length of all the data in the mbuf chain for this packet
 */
#define IX_MBUF_PKT_LEN(m_blk_ptr)		   (m_blk_ptr)->mBlkPktHdr.len

/**
 * @def IX_MBUF_NET_POOL(m_blk_ptr)
 *
 * @brief Return pointer to a network pool
 */
#if defined __vxworks
#define IX_MBUF_NET_POOL(m_blk_ptr)		   (m_blk_ptr)->pClBlk->pNetPool
#elif defined __linux
#define IX_MBUF_NET_POOL(m_blk_ptr)		   (m_blk_ptr)->m_net_pool
#elif defined __ECOS
#define IX_MBUF_NET_POOL(m_blk_ptr)		   (m_blk_ptr)->mBlkPktHdr.m_net_pool
#endif

#endif /* ndef IxOsBuffMgt_inc */
