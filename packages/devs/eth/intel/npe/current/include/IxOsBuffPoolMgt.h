/**
 * @file IxOsBuffPoolMgt.h
 *
 * @date 9 Oct 2002
 *
 * @brief This file contains the mbuf pool implementation API
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
 * This module contains the implementation of the OS Services buffer pool
 * management service.  This module provides routines for creating pools
 * of buffers for exchange of network data, getting and returning buffers
 * from and to the pool, and some other utility functions. 
 * <P>
 * Currently, the pool has 2 underlying implementations - one for the vxWorks
 * OS, and another which attempts to be OS-agnostic so that it can be used on
 * other OS's such as Linux.  The API is largely the same for all OS's,
 * but there are some differences to be aware of.  These are documented
 * in the API descriptions below.
 * <P>
 * The most significant difference is this: when this module is used with
 * the WindRiver VxWorks OS, it will create a pool of vxWorks "MBufs".
 * These can be used directly with the vxWorks "netBufLib" OS Library.
 * For other OS's, it will create a pool of generic buffers.  These may need
 * to be converted into other buffer types (sk_buff's in Linux, for example)
 * before being used with any built-in OS routines available for
 * manipulating network data buffers.
 *
 * @sa IxOsBuffMgt.h
 */

/**
 * @defgroup IxOsBuffPoolMgt IXP4xx OS Memory Buffer Pool Management (IxOsBuffPoolMgt) API
 *
 * @brief The Public API for the Buffer Pool Management component
 * 
 * @{
 */

#ifndef IXOSBUFFPOOLMGT_H
#define IXOSBUFFPOOLMGT_H

/*
 * Put the system defined include files required.
 */

#ifdef __vxworks
#include <netBufLib.h>
#endif

/*
 * Put the user defined include files required.
 */

#ifdef __vxworks
#include "IxOsServices.h"
#endif
#include "IxTypes.h"
#include "IxOsBuffMgt.h"
#include "IxOsCacheMMU.h"


/*
 * #defines and macros used in this file.
 */

/**
 * @def IX_MBUF_POOL_SIZE_ALIGN
 *
 * @brief This macro takes an integer as an argument and
 * rounds it up to be a multiple of the memory cache-line 
 * size.
 *
 * @param int [in] size - the size integer to be rounded up
 *
 * @return int - the size, rounded up to a multiple of
 *               the cache-line size
 */
#define IX_MBUF_POOL_SIZE_ALIGN(size)                 \
    (((size + (IX_XSCALE_CACHE_LINE_SIZE - 1)) /      \
        IX_XSCALE_CACHE_LINE_SIZE) *                  \
            IX_XSCALE_CACHE_LINE_SIZE)


/**
 * @def IX_MBUF_POOL_MBUF_AREA_SIZE_ALIGNED
 *
 * @brief This macro calculates, from the number of mbufs required, the 
 * size of the memory area required to contain the mbuf headers for the
 * buffers in the pool.  The size to be used for each mbuf header is 
 * rounded up to a multiple of the cache-line size, to ensure
 * each mbuf header aligns on a cache-line boundary.
 * This macro is used by IX_MBUF_POOL_MBUF_AREA_ALLOC()
 *
 * @note Refer to the WindRiver "VxWorks 5.5 OS Libraries API Reference" manual
 * for "netBufLib" library documentation, which explains the vxWorks
 * implementation of this macro below.
 *
 * @param int [in] count - the number of buffers the pool will contain
 *
 * @return int - the total size required for the pool mbuf area (aligned)
 */
#ifndef __vxworks                                                   
#  define IX_MBUF_POOL_MBUF_AREA_SIZE_ALIGNED(count)                     \
      (count * IX_MBUF_POOL_SIZE_ALIGN(sizeof(IX_MBUF)))
#else
#  define IX_MBUF_POOL_MBUF_AREA_SIZE_ALIGNED(count)                     \
      ((count * IX_MBUF_POOL_SIZE_ALIGN(M_BLK_SZ + sizeof (long))) +     \
       (count * IX_MBUF_POOL_SIZE_ALIGN(CL_BLK_SZ)))
#endif


/**
 * @def IX_MBUF_POOL_DATA_AREA_SIZE_ALIGNED
 *
 * @brief This macro calculates, from the number of mbufs required and the
 * size of the data portion for each mbuf, the size of the data memory area
 * required. The size is adjusted to ensure alignment on cache line boundaries.
 * This macro is used by IX_MBUF_POOL_DATA_AREA_ALLOC()
 *
 * @note Refer to the WindRiver "VxWorks 5.5 OS Libraries API Reference" manual
 * for "netBufLib" library documentation, which explains the vxWorks
 * implementation of this macro below.
 *
 * @param int [in] count - The number of mbufs in the pool.
 * @param int [in] size  - The desired size for each mbuf data portion.
 *                         This size will be rounded up to a multiple of the
 *                         cache-line size to ensure alignment on cache-line
 *                         boundaries for each data block.
 *
 * @return int - the total size required for the pool data area (aligned)
 */
#ifndef __vxworks                                                   
#  define IX_MBUF_POOL_DATA_AREA_SIZE_ALIGNED(count, size)               \
      (count * (IX_MBUF_POOL_SIZE_ALIGN(size)))
#else
#  define IX_MBUF_POOL_DATA_AREA_SIZE_ALIGNED(count, size)               \
      (count * IX_MBUF_POOL_SIZE_ALIGN(size + sizeof(long)))
#endif



/**
 * @def IX_MBUF_POOL_MBUF_AREA_ALLOC
 *
 * @brief Allocates the memory area needed for the number of mbuf headers
 * specified by <i>count</i>.
 * This macro ensures the mbuf headers align on cache line boundaries.
 * This macro evaluates to a pointer to the memory allocated.
 *
 * @param int [in] count - the number of mbufs the pool will contain
 * @param int [out] memAreaSize - the total amount of memory allocated
 *
 * @return void * - a pointer to the allocated memory area
 */
#define IX_MBUF_POOL_MBUF_AREA_ALLOC(count, memAreaSize) \
    IX_ACC_DRV_DMA_MALLOC((memAreaSize =                 \
        IX_MBUF_POOL_MBUF_AREA_SIZE_ALIGNED(count)))

/**
 * @def IX_MBUF_POOL_DATA_AREA_ALLOC
 *
 * @brief Allocates the memory pool for the data portion of the pool mbufs.
 * The number of mbufs is specified by <i>count</i>.  The size of the data
 * portion of each mbuf is specified by <i>size</i>.
 * This macro ensures the mbufs are aligned on cache line boundaries
 * This macro evaluates to a pointer to the memory allocated.
 *
 * @param int [in] count - the number of mbufs the pool will contain
 * @param int [in] size - the desired size (in bytes) required for the data
 *                        portion of each mbuf.  Note that this size may be
 *                        rounded up to ensure alignment on cache-line
 *                        boundaries.
 * @param int [out] memAreaSize - the total amount of memory allocated
 *
 * @return void * - a pointer to the allocated memory area
 */
#define IX_MBUF_POOL_DATA_AREA_ALLOC(count, size, memAreaSize) \
    IX_ACC_DRV_DMA_MALLOC((memAreaSize =                     \
        IX_MBUF_POOL_DATA_AREA_SIZE_ALIGNED(count,size)))

/**
 * @def IX_MBUF_POOL_FREE_COUNT
 *
 * @brief Returns the number of free buffers currently in the specified pool
 *
 * @param IX_MBUF_POOL * [in] poolPtr - a pointer to the pool to query
 *
 * @return int - the number of free buffers in the pool
 */
#ifndef __vxworks
#  define IX_MBUF_POOL_FREE_COUNT(poolPtr) ((poolPtr)->freeBufsInPool)
#else
#  define IX_MBUF_POOL_FREE_COUNT(poolPtr) get_free_mblk_count(poolPtr)

/*
 * function to walk through the mbuf chain of a netBufLib pool to count
 * the buffers currently available in the pool
 */
static __inline__ int
get_free_mblk_count(NET_POOL *poolPtr)
{
    int i = 0;
    IX_MBUF *head = poolPtr->pmBlkHead;
    while(head)
    {
        head = head->mBlkHdr.mNext;
        i++;
    }
    return i;
}

#endif


/**
 * @def IX_MBUF_MAX_POOLS
 *
 * @brief The maximum number of pools that can be allocated
 * @note  This can safely be increased if more pools are required.
 */
#define IX_MBUF_MAX_POOLS      24

/**
 * @def IX_MBUF_POOL_NAME_LEN
 *
 * @brief The maximum string length of the pool name
 */
#define IX_MBUF_POOL_NAME_LEN  64


#ifndef __vxworks
/**
 * @struct IxMbufPool
 * @brief Implementation of buffer pool structure for use with non-VxWorks OS
 */

typedef struct
{
    IX_MBUF *nextFreeBuf;  /**< Pointer to the next free mbuf              */
    void *mbufMemPtr;      /**< Pointer to the mbuf memory area            */
    void *dataMemPtr;      /**< Pointer to the data memory area            */
    int bufDataSize;       /**< The size of the data portion of each mbuf  */
    int totalBufsInPool;   /**< Total number of mbufs in the pool          */
    int freeBufsInPool;    /**< Number of free mbufs currently in the pool */
    int mbufMemSize;       /**< The size of the pool mbuf memory area      */
    int dataMemSize;       /**< The size of the pool data memory area      */
    char name[IX_MBUF_POOL_NAME_LEN + 1];   /**< Descriptive name for pool */
} IxMbufPool;

typedef IxMbufPool IX_MBUF_POOL;

#else 

  /**
   * @typedef IX_MBUF_POOL
   *
   * @brief The buffer pool structure, mapped to appropraite
   * OS-specific implementation
   */
typedef NET_POOL IX_MBUF_POOL;

#endif


/**
 * @def IX_MBUF_POOL_INIT
 *
 * @brief Wrapper macro for ixOsBuffPoolInit() 
 * See function description below for details.
 */
#define IX_MBUF_POOL_INIT(poolPtrPtr, count, size, name) \
    ixOsBuffPoolInit((poolPtrPtr), (count), (size), (name))

/**
 * @def IX_MBUF_POOL_INIT_NO_ALLOC
 *
 * @brief Wrapper macro for ixOsBuffPoolInitNoAlloc() 
 * See function description below for details.
 */
#define IX_MBUF_POOL_INIT_NO_ALLOC(poolPtrPtr, bufPtr, dataPtr, count, size, name) \
    ixOsBuffPoolInitNoAlloc((poolPtrPtr), (bufPtr), (dataPtr), (count), (size), (name))

/**
 * @def IX_MBUF_POOL_GET
 *
 * @brief Wrapper macro for ixOsBuffPoolUnchainedBufGet() 
 * See function description below for details.
 */
#define IX_MBUF_POOL_GET(poolPtr, bufPtrPtr) \
    ixOsBuffPoolUnchainedBufGet((poolPtr), (bufPtrPtr))

/**
 * @def IX_MBUF_POOL_PUT
 *
 * @brief Wrapper macro for ixOsBuffPoolBufFree() 
 * See function description below for details.
 */
#define IX_MBUF_POOL_PUT(bufPtr) \
    ixOsBuffPoolBufFree(bufPtr)

/**
 * @def IX_MBUF_POOL_PUT_CHAIN
 *
 * @brief Wrapper macro for ixOsBuffPoolBufChainFree() 
 * See function description below for details.
 */
#define IX_MBUF_POOL_PUT_CHAIN(bufPtr) \
    ixOsBuffPoolBufChainFree(bufPtr)

/**
 * @def IX_MBUF_POOL_SHOW
 *
 * @brief Wrapper macro for ixOsBuffPoolShow() 
 * See function description below for details.
 */
#define IX_MBUF_POOL_SHOW(poolPtr) \
    ixOsBuffPoolShow(poolPtr)

/**
 * @def IX_MBUF_POOL_MDATA_RESET
 *
 * @brief Wrapper macro for ixOsBuffPoolBufDataPtrReset() 
 * See function description below for details.
 */
#define IX_MBUF_POOL_MDATA_RESET(bufPtr) \
    ixOsBuffPoolBufDataPtrReset(bufPtr) 



/*
 * Prototypes for interface functions.
 */

/**
 * @ingroup
 *
 * @fn ixOsBuffPoolInit (IX_MBUF_POOL **poolPtrPtr, int count, int size, char *name)
 *
 * @brief This function creates a new buffer pool
 *
 * @param IX_MBUF_POOL ** [out] poolPtrPtr - Pointer to a pool pointer.
 * @param int [in] count - The number of buffers to have in the pool.
 * @param int [in] size  - The size of each buffer in the pool.
 * @param char * [in] name - A name string for the pool (used in pool show).
 *
 * This function initialises a pool of <i>count</i> buffers, each of size
 * <i>size</i>.  It allocates memory for the pool, fills in the pool and buffer
 * data structures, and returns a pointer to the pool in the <i>poolPtrPtr</i>
 * parameter.  This pointer should be used with other functions on this API to
 * use the pool.
 * In the current implementation, only a limited number of pools can be allocated.
 * The number of pools is decided by the value of <i>IX_MBUF_MAX_POOLS</i>.
 *
 * @note This function has 2 implementations, depending on which OS the code is
 * compiled for.  If compiled for VxWorks, an mbuf pool will be created using the
 * VxWorks "netBufLib" OS library.  This will produce a pool of mbufs which can be
 * used with the netBufLib library routines if required.
 * If compiled for a different OS, a pool of generic buffers will be produced.
 * These may need to be converted to a different buffer format (such as sk_buffs
 * for Linux) to be used with OS network buffer manipulation routines if required.
 * See the header file IxOsBuffMgt.h which maps the buffer implementations for
 * each OS supported.
 *
 * Thread Safe: no
 *
 * @pre
 *      - <i>poolPtrPtr</i> should point to a valid IX_MBUF_POOL pointer
 *
 * @post
 *      - A pool will be initialised and all memory required by the pool will
 *        be dynamically allocated from memory.
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL if the pool could not be created
 */ 

IX_STATUS
ixOsBuffPoolInit (IX_MBUF_POOL **poolPtrPtr, int count, int size, char *name);

/**
 * @ingroup IxOsBuffPoolMgt
 *
 * @fn ixOsBuffPoolInitNoAlloc (IX_MBUF_POOL **poolPtrPtr, void *poolBufPtr,
			 void *poolDataPtr, int count, int size, char *name)
 *
 * @brief This function creates a new buffer pool, with user-allocated memory
 *
 * @param IX_MBUF_POOL ** [out] poolPtrPtr - Pointer to a pool pointer.
 * @param void * [in] poolBufPtr  - pointer to memory allocated with
 *                                      IX_MBUF_POOL_MBUF_AREA_ALLOC()
 * @param void * [in] poolDataPtr - pointer to memory allocated with
 *                                      IX_MBUF_POOL_DATA_AREA_ALLOC()
 * @param int [in] count - The number of buffers to have in the pool.
 * @param int [in] size  - The size of each buffer in the pool (i.e. the
 *                         amount of payload data octets each buffer can hold).
 * @param char * [in] name - A name string for the pool (used in pool show).
 *
 * This function initialises a pool of <i>count</i> buffers, each of size
 * <i>size</i>.  It fills in the pool and buffer data structures, and returns
 * a pointer to the pool in the <i>poolPtrPtr</i> parameter. 
 * This pointer should be used with other functions on this API to use the pool.
 * In the current implementation, only a limited number of pools can be allocated.
 * The number of pools is decided by the value of <i>IX_MBUF_MAX_POOLS</i>.
 *
 *
 * @note This function has 2 implementations, depending on which OS the code is
 * compiled for.  If compiled for VxWorks, an mbuf pool will be created using the
 * VxWorks "netBufLib" OS library.  This will produce a pool of mbufs which can be
 * used with the netBufLib library routines if required.
 * If compiled for a different OS, a pool of generic buffers will be produced.
 * These may need to be converted to a different buffer format (such as sk_buffs
 * for Linux) to be used with OS network buffer manipulation routines if required.
 * See the header file IxOsBuffMgt.h which maps the buffer implementations for
 * each OS supported.
 *
 * @note The pointer to the data area can optionally be NULL, to indicate that the
 * data memory area for the mbuf payload will be assigned by the user later on.
 * In this case, it is expected that the user would assign the data pointer of
 * each mbuf returned by ixOsBuffPoolUnchainedBufGet().  This also means that
 * the function ixOsBuffPoolBufDataPtrReset() cannot be used on buffers from
 * this pool.
 * WARNING - This pointer CANNOT be NULL if VxWorks implementation is used!
 * 
 * Thread Safe: no
 *
 * @pre
 *      - <i>poolPtrPtr</i> should point to a valid IX_MBUF_POOL pointer
 *      - The memory required for the pool, for mbuf structures and data
 *        (if required), should be allocated with the specified macros. See params
 *        <i>poolBufPtr</i> and <i>poolDataPtr</i>
 *
 * @post
 *      - A pool will be initialised and all memory required by the pool will
 *        be dynamically allocated from memory.
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL if the pool could not be created
 */ 

IX_STATUS
ixOsBuffPoolInitNoAlloc (IX_MBUF_POOL **poolPtrPtr, void *poolBufPtr,
			 void *poolDataPtr, int count, int size, char *name);

/**
 * @ingroup IxOsBuffPoolMgt
 *
 * @fn ixOsBuffPoolUnchainedBufGet (IX_MBUF_POOL *poolPtr, IX_MBUF **newBufPtrPtr)
 *
 * @brief This function gets a buffer from the buffer pool
 *
 * @param IX_MBUF_POOL * [in] poolPtr - Pointer to a valid pool.
 * @param IX_MBUF ** [out] newBufPtrPtr - A pointer to a valid IX_MBUF pointer.
 *
 * This function gets a free buffer from the specified pool, and 
 * returns a pointer to the buffer in the <i>newBufPtrPtr</i> parameter. The
 * buffer obtained will be a single unchained buffer of the size specified 
 * when the pool was initialised.
 * 
 * Thread Safe: yes
 *
 * @pre
 *      - <i>poolPtr</i> should point to a valid IX_MBUF_POOL structure
 *      - <i>newBufPtrPtr</i> should point to a valid IX_MBUF pointer
 *
 * @post
 *      - A free buffer will be allocated from the pool and newBufPtrPtr
 *        can be dereferenced to access the pointer to the buffer.
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL if a free buffer could not be obtained
 */ 
IX_STATUS
ixOsBuffPoolUnchainedBufGet (IX_MBUF_POOL *poolPtr, IX_MBUF **newBufPtrPtr);

/**
 * @ingroup IxOsBuffPoolMgt
 *
 * @fn ixOsBuffPoolBufFree (IX_MBUF *bufPtr)
 *
 * @brief This function returns a buffer to the buffer pool
 *
 * @param IX_MBUF * [in] bufPtr - Pointer to a valid IX_MBUF buffer.
 *
 * This function returns a buffer to the pool, making it available again to 
 * ixOsBuffPoolUnchainedBufGet().  The buffer pointed to by <i>bufPtr</i>
 * can be chained or unchained.  If it is chained, only the head of the chain
 * will be freed to the pool, and a pointer to the next buffer in the chain
 * will be returned to the caller.
 * 
 * Thread Safe: yes
 *
 * @pre
 *      - <i>bufPtr</i> should point to a valid IX_MBUF structure
 *
 * @post
 *      - The buffer supplied will be returned to the pool for reuse.
 *
 * @return
 *      - If supplied buffer was chained, a pointer to the next buffer in
 *        the chain is returned
 *      - Otherwise NULL is returned
 */ 
IX_MBUF *
ixOsBuffPoolBufFree (IX_MBUF *bufPtr);
    
/**
 * @ingroup IxOsBuffPoolMgt
 *
 * @fn ixOsBuffPoolBufChainFree (IX_MBUF *bufPtr)
 *
 * @brief This function returns a buffer chain to the buffer pool
 *
 * @param IX_MBUF * [in] bufPtr - Pointer to head of the chain..
 *
 * This function returns a buffer chain to the pool, making the buffers 
 * available again to ixOsBuffPoolUnchainedBufGet().  The buffer pointed
 * to by <i>bufPtr</i> can be chained or unchained.  If it is chained, all
 * buffers in the chain will be returned to the pool.
 * 
 * Thread Safe: yes
 *
 * @pre
 *      - <i>bufPtr</i> should point to a valid IX_MBUF structure
 *
 * @post
 *      - The buffer (or chain of buffers) supplied will be returned to the
 *        pool for reuse.
 *
 * @return
 *      - none
 */ 
void
ixOsBuffPoolBufChainFree (IX_MBUF *bufPtr);
    
/**
 * @ingroup IxOsBuffPoolMgt
 *
 * @fn ixOsBuffPoolShow (IX_MBUF_POOL *poolPtr)
 *
 * @brief This function prints pool statistics
 *
 * @param IX_MBUF_POOL * [in] poolPtr - A pointer to a valid pool.
 *
 * This function prints pool statistics, such as the number of free buffers in each pool.
 * The actual statistics printed depends on the implementation which may differ between
 * platforms.  This funtion can serve as a useful debugging aid.
 * 
 * Thread Safe: yes
 *
 * @pre
 *      - <i>poolPtr</i> should point to a valid IX_MBUF_POOL structure
 *
 * @post
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL if the pool statistics could not be printed.
 */ 
IX_STATUS
ixOsBuffPoolShow (IX_MBUF_POOL *poolPtr);


/**
 * @ingroup IxOsBuffPoolMgt
 *
 * @fn ixOsBuffPoolBufDataPtrReset (IX_MBUF *bufPtr)
 *
 * @brief This function resets the data pointer of a buffer
 *
 * @param IX_MBUF * [in] bufPtr - Pointer to a valid IX_MBUF buffer.
 *
 * This function resets the data pointer of a buffer to point to the
 * start of the memory area allocated to the buffer for data
 * (the buffer payload).
 *
 * @note WARNING - This function can NOT be used if BOTH of following
 * conditions are true:
 *      - the pool was created using ixOsBuffPoolInitNoAlloc()
 *      - a NULL value was supplied at the time of creation for the
 *        <i>bufDataPtr</i> parameter of ixOsBuffPoolInitNoAlloc().
 * See the API description of the function ixOsBuffPoolInitNoAlloc().
 * 
 * Thread Safe: yes
 *
 * @pre
 *      - <i>bufPtr</i> should point to a valid IX_MBUF structure
 *      - A non-NULL pointer to the data memory area was supplied
 *        when the pool was created (see note above)
 *      - The data pointer is pointing to somewhere within the
 *        buffer payload
 *
 * @post
 *      - The data pointer of the mbuf header will point to the start of
 *        the data payload section of the buffer, as it did when it was
 *        originally obtained from the pool
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL if the operation was not successful
 */ 
IX_STATUS
ixOsBuffPoolBufDataPtrReset (IX_MBUF *bufPtr);


#endif  /* IXOSBUFFPOOLMGT_H */


/**
 * @} defgroup IxOsBuffMgt
 */
