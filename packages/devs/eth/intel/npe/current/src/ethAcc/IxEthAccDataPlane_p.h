/**
 * @file IxEthAccDataPlane_p.h
 *
 * @author Intel Corporation
 * @date 12-Feb-2002
 *
 * @brief  Internal Header file for IXP425 Ethernet Access component.
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



#ifndef IxEthAccDataPlane_p_H
#define IxEthAccDataPlane_p_H

#include <IxOsServices.h>
#include <IxOsBuffMgt.h>

/**
 * @addtogroup IxEthAccPri
 *@{
 */

/* 
 * In general data plane APIs should do minimal testing,
 * of the args. But for integrating testing set this to 1.
 */
#define IX_ETH_ACC_DATA_PLANE_FUNC_ARG_CHECKS 0
/*
 * Turning this on (setting it to 1) provides detailed statistics gathering
 * on data plane activity. 
 */
#ifndef __ECOS
#define IX_ETH_ACC_DATA_PLANE_STATS_ON 1
#endif

/*
 * The Qmgr component may in future support the "additionalCallbacksRequired" parameter
 * in callbacks. For the time being however, this is not the case, and we must alter
 * callback behaviour, by setting IX_ETH_ACC_QMGR_NO_ADDITIONAL_CALLBACKS to 1
 */

#define IX_ETH_ACC_QMGR_NO_ADDITIONAL_CALLBACKS 1
/* typedefs global to this file*/

typedef struct
{
    IX_MBUF *pHead;
    IX_MBUF *pTail;
}IxEthAccDataPlaneQList;


/**
 * @struct  IxEthAccDataPlaneStats
 * @brief   Statistics data structure associated with the data plane
 *
 */
typedef struct
{
UINT32 rxFrameCounter;
UINT32 txFrameDoneCounter;
UINT32 rxFrameUnderrunFailure;
UINT32 txFrameDoneUnderrunFailure;
}IxEthAccDataPlaneStats;

/**
 * @fn ixEthAccMbufFromSwQ
 * @brief  used during disable steps to convert mbufs from 
 *  swq format, ready to be pushed into hw queues for NPE, 
 *  back into XScale format 
 */
IX_MBUF *ixEthAccMbufFromSwQ(IX_MBUF *mbuf);

/**
 * @fn ixEthAccDataPlaneShow
 * @brief  Show function (for data plane statistics
 */
void ixEthAccDataPlaneShow(void);

#define IX_ETH_ACC_DATA_PLANE_LOCK(arg) arg = ixOsServIntLock();
#define IX_ETH_ACC_DATA_PLANE_UNLOCK(arg) ixOsServIntUnlock(arg);

/*
 * Use MBUF  next pointer field to chain data.
 */
#define IX_ETH_ACC_MBUF_NEXT_PKT_CHAIN_MEMBER(mbuf) IX_MBUF_NEXT_PKT_IN_CHAIN_PTR(mbuf)



#define IX_ETH_ACC_DATAPLANE_IS_Q_EMPTY(mbuf_list) (mbuf_list.pHead == NULL)
    

#define IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_HEAD(mbuf_list,mbuf_to_add) 		\
  {										\
    int lockVal;								\
    IX_ETH_ACC_DATA_PLANE_LOCK(lockVal);                                    	\
    if ( (mbuf_list.pHead) != NULL ) 						\
    {										\
      (IX_ETH_ACC_MBUF_NEXT_PKT_CHAIN_MEMBER((mbuf_to_add))) = (mbuf_list.pHead);\
      (mbuf_list.pHead) = (mbuf_to_add);					\
    } 										\
    else {									\
      (mbuf_list.pTail) = (mbuf_list.pHead) = (mbuf_to_add);			\
      IX_ETH_ACC_MBUF_NEXT_PKT_CHAIN_MEMBER((mbuf_to_add)) = NULL;		\
    } 										\
    IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);					\
  }


#define IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_TAIL(mbuf_list,mbuf_to_add)   	\
  {										\
    int lockVal;								\
    IX_ETH_ACC_DATA_PLANE_LOCK(lockVal);                                    	\
    if ( (mbuf_list.pHead) == NULL ) 						\
    {										\
      (mbuf_list.pHead) = mbuf_to_add;						\
      IX_ETH_ACC_MBUF_NEXT_PKT_CHAIN_MEMBER((mbuf_to_add)) = NULL;		\
    } 										\
    else {									\
      IX_ETH_ACC_MBUF_NEXT_PKT_CHAIN_MEMBER((mbuf_list.pTail)) = (mbuf_to_add);	\
      IX_ETH_ACC_MBUF_NEXT_PKT_CHAIN_MEMBER((mbuf_to_add)) = NULL;		\
    } 										\
    (mbuf_list.pTail) = mbuf_to_add;						\
    IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);					\
  }


#define IX_ETH_ACC_DATAPLANE_REMOVE_MBUF_FROM_Q_HEAD(mbuf_list,mbuf_to_rem)   	\
  {										\
    int lockVal;								\
    IX_ETH_ACC_DATA_PLANE_LOCK(lockVal);                                    	\
    if ( (mbuf_list.pHead) != NULL ) 						\
    {										\
      (mbuf_to_rem) = (mbuf_list.pHead) ;					\
      (mbuf_list.pHead) = (IX_ETH_ACC_MBUF_NEXT_PKT_CHAIN_MEMBER((mbuf_to_rem)));\
    } 										\
    else {									\
      (mbuf_to_rem) = NULL;							\
    } 										\
    IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);					\
  }


/**
 * @brief message handler QManager enties for NPE id => port ID conversion (NPE_B => 0, NPE_C => 1)
 */
#define IX_ETH_ACC_QM_NPE_ID_TO_PORT_ID(npe) (npe) 

#define IX_ETH_ACC_PORT_TO_TX_Q_ID(port)    (port == IX_ETH_PORT_1 ? IX_ETH_ACC_TX_FRAME_ENET0_Q : IX_ETH_ACC_TX_FRAME_ENET1_Q)

#define IX_ETH_ACC_PORT_TO_TX_Q_SOURCE(port)    (port == IX_ETH_PORT_1 ? IX_ETH_ACC_TX_FRAME_ENET0_Q_SOURCE : IX_ETH_ACC_TX_FRAME_ENET1_Q_SOURCE)

#define IX_ETH_ACC_PORT_TO_RX_FREE_Q_ID(port) (port == IX_ETH_PORT_1 ? IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q : IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q)

#define IX_ETH_ACC_PORT_TO_RX_FREE_Q_SOURCE(port) (port == IX_ETH_PORT_1 ? IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q_SOURCE : IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q_SOURCE)

/* Flush the mbufs chain and all data pointed to by the mbuf */

#define IX_ETH_ACC_FLUSH_MBUF_AND_DATA(mbuf_arg)  					\
{ 											\
	IX_MBUF *mbufPtr = (mbuf_arg);							\
	while(mbufPtr) 									\
	{										\
	    IX_ACC_DATA_CACHE_FLUSH((mbufPtr),sizeof(IX_MBUF))				\
	    IX_ACC_DATA_CACHE_FLUSH(IX_MBUF_MDATA(mbufPtr),IX_MBUF_MLEN(mbufPtr))	\
	    mbufPtr = IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr);				\
	}										\
} 

/* Flush the mbufs header only  */
#define IX_ETH_ACC_FLUSH_MBUF(mbuf_arg)  {IX_ACC_DATA_CACHE_FLUSH((mbuf_arg),sizeof(IX_MBUF));  }


#define IX_ETH_ACC_WORD_QM_WORD_ENDIAN_SWAP(value) (value)


#if IX_ETH_ACC_DATA_PLANE_STATS_ON
#define IX_ETH_ACC_STATS_INC(x) (x++)
#else
#define IX_ETH_ACC_STATS_INC(x)
#endif

/**
 * @warning When fast path ATM to Ethernet is enabled, we must make a significant modification
 * to the behaviour of ethAcc. Primarily we must always leave enough room in the TX
 * queues for the fast path component to write its entire allocation of buffers. This
 * is currently hardcoded to 64 buffers, leaving us with a maximum of 64 "slots" in each
 * TX queue. So when IX_ETH_ACC_FPATH_AWARE is set to TRUE, we have effectively reduced the
 * TX queue depth by half. This will obviously have a potentially major impact on the
 * performance of the component and the timing dynamics of the passage of buffers
 * through the system.
 * When operating in a system in which Fast path is enabled, it is important to recognise
 * that the TX Q may never become empty, as it may be kept non-empty by submissions from
 * the fast path component. Since Eth Acc by default relies on the TX Q empty callback
 * to initiate submissions of buffers previously queued on software queues, we must
 * modify this behaviour so that the TX Done callback is used to initiate such submissions.
 * This is achieved by setting IX_ETH_ACC_SUBMIT_FROM_TX_DONE to 1, which is done automatically
 * when IX_ETH_ACC_FPATH_AWARE is set to 1.
 */
#if IX_ETH_ACC_FPATH_AWARE
#define IX_ETH_ACC_FPATH_DELTA_INC(x) \
        {                                                                   \
        int _lockVal;                                                       \
        IX_ETH_ACC_DATA_PLANE_LOCK(_lockVal);                               \
        x++;                                                                \
        IX_ETH_ACC_DATA_PLANE_UNLOCK(_lockVal);                             \
        }

#define IX_ETH_ACC_FPATH_DELTA_DEC(x)  \
        {                                                                   \
        int _lockVal;                                                       \
        IX_ETH_ACC_DATA_PLANE_LOCK(_lockVal);                               \
        x--;                                                                \
        IX_ETH_ACC_DATA_PLANE_UNLOCK(_lockVal);                             \
        }

#define IX_ETH_ACC_SUBMIT_FROM_TX_DONE 1
#define IX_ETH_ACC_MAX_TX_FRAMES_TO_SUBMIT 64
#else
#define IX_ETH_ACC_FPATH_DELTA_INC(x)
#define IX_ETH_ACC_FPATH_DELTA_DEC(x)
#define IX_ETH_ACC_SUBMIT_FROM_TX_DONE 0
#define IX_ETH_ACC_MAX_TX_FRAMES_TO_SUBMIT 128
#endif



#endif /* IxEthAccDataPlane_p_H */


/**
 *@}
 */

