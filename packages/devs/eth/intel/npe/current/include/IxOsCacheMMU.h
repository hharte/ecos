/**
 * @file IxOsCacheMMU.h
 *
 * @brief this file contains the API of the @ref IxCacheMMU component
 * 
 * <hr>
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

#ifndef IxCacheMMU_H

#ifndef __doxygen_hide
#define IxCacheMMU_H
#endif /* __doxygen_hide */

#ifdef __vxworks

#include <vxWorks.h>
#include <cacheLib.h>
#include "IxTypes.h"


/**
 * @defgroup IxOsCacheMMU IXP4xx OS Cache MMU (IxOsCacheMMU) API
 *
 * @brief This service provides services to the access components and codelets to
 * abstract out any cache coherency issues and mmu mappings.
 *
 * @{
 */

/**
 * @def IX_MMU_VIRTUAL_TO_PHYSICAL_TRANSLATION(addr)
 *
 * @brief Return a virtual address for the provided  physical address.
 *
 * Return a virtual address for the provided  physical address.
 *
 * <hr>
 */
#define IX_MMU_VIRTUAL_TO_PHYSICAL_TRANSLATION(addr) (addr)

/**
 * @def IX_MMU_PHYSICAL_TO_VIRTUAL_TRANSLATION(addr)
 * 
 * @brief Return a physical address for the provided virtual.
 * 
 * Return a physical address for the provided virtual address
 *
 * <hr>
 */
#define IX_MMU_PHYSICAL_TO_VIRTUAL_TRANSLATION(addr) (addr)

/**
 * @def IX_XSCALE_CACHE_LINE_SIZE
 *
 * @brief IX_XSCALE_CACHE_LINE_SIZE = size of cache line for both flush and invalidate.
 *
 *
 * <hr>
 */
#define IX_XSCALE_CACHE_LINE_SIZE (32)

/**
 * @def IX_ACC_DRAM_PHYS_OFFSET
 *
 * @brief PHYS_OFFSET = Physical DRAM offset..
 *
 *
 * <hr>
 */
#define IX_ACC_DRAM_PHYS_OFFSET     (0x00000000UL) 



/**
 * @def  IX_ACC_DATA_CACHE_INVALIDATE(addr,size) 
 *
 * @brief Invalidate a cache range.  
 *
 * Invalidate a cache range.  
 * @note
 * This is typically done prior to submitting a buffer to the NPE's which you
 * expect the NPE to populate with data.
 * @attention The size argument must be a multiple of cacheline size, i.e. a multiple
 * if 32bytes for the XSCALE. The argument shall be rounded up to the next 32byte boundry.
 * Extreem care must be taken when invalidating cache lines due. 
 *
 * - If memory space used is non cached, then this function may be null.
 * - Functionality required: 
 *   -# Non-Cached space         : No functionality required. 
 *   -# Write Through Enabled    : Invalidate area specified
 *   -# Copy Back Enabled        : Invalidate area specified
 * 
 *
 * <hr>
 *
 */
#define IX_ACC_DATA_CACHE_INVALIDATE(addr,size) {}

/** 
 * 
 * @def IX_ACC_DATA_CACHE_FLUSH(addr,size)
 *
 * @brief Flush a cache range to physical memory.  
 *
 * Flush a cache range to physical memory.
 * @note
 * This is typically done prior to submitting a buffer to the NPE's which you
 * expect the NPE to read from. Entire Cache lines will be flushed.
 *
 * - If memory space used is non cached, then this function does may be null.
 * - Functionality required: 
 *   -# Non-Cached space      	: Flush CPU WB, No cache Flush.
 *   -# Write Through Enabled  	: Flush CPU WB, No cache Flush. 
 *   -# Copy Back Enabled     	: Flush CPU WB, Invalidate area specified
 *
 * <hr>
 *
 */
#define IX_ACC_DATA_CACHE_FLUSH(addr,size) {}


/** 
 *
 * @def  IX_ACC_DRV_DMA_MALLOC
 *
 * @brief Allocate memory for driver use, that will be shared between XScale and NPE's.
 *
 *  Allocate memory for driver use, that will be shared between XScale and NPE's. 
 * @note The buffer allocated with have the system defined atributes, and as such the Invalidate and 
 * flush macros functionality must be updated.
 *
 * @param uint32 size -  number of bytes of memory requested.
 *
 * @return  void * Pointer to memory that can be used between XScale and NPE's. 
 * 
 *
 * <hr>
 */
#define IX_ACC_DRV_DMA_MALLOC(size)  cacheDmaMalloc(size) 

/** 
 *
 * @def  IX_ACC_DRV_DMA_FREE
 *
 * @brief Free memory allocated from IX_ACC_DRV_DMA_MALLOC.
 *
 * This function frees the memory allocated from @a IX_ACC_DRV_DMA_MALLOC.
 *
 * @param void * Pointer - pointer to the memory area to be freed.
 * @param uint32 size -  number of bytes of memory allocated.
 *
 * @return  void
 *
 * <hr>
 */
#define IX_ACC_DRV_DMA_FREE(ptr,size)  cacheDmaFree(ptr)

#elif defined(__linux)

#include <stdlib.h>

/**
 * Linux implementations of above macros.
 */

#define IX_MMU_VIRTUAL_TO_PHYSICAL_TRANSLATION(addr) ((addr) ? virt_to_phys((void*)(addr)) : 0)
#define IX_MMU_PHYSICAL_TO_VIRTUAL_TRANSLATION(addr) ((addr) ? phys_to_virt((unsigned int)(addr)) : 0)
#define IX_ACC_DATA_CACHE_INVALIDATE(addr,size) invalidate_dcache_range((__u32)addr, (__u32)addr + size )
#define IX_ACC_DATA_CACHE_FLUSH(addr,size) clean_dcache_range((__u32)addr, (__u32)addr + size )
#define IX_ACC_DRAM_PHYS_OFFSET     (PHYS_OFFSET) 

#define IX_XSCALE_CACHE_LINE_SIZE 32

/* 
 * Allocate on a cache line boundary (null pointers are
 * not affected by this operation). This operation is NOT cache safe.
 */
#define IX_ACC_DRV_DMA_MALLOC(size)   ix_dma_alloc(size)
#define IX_ACC_DRV_DMA_FREE(ptr,size) ix_dma_free(ptr,size)

static inline void *ix_dma_alloc(int size)
{
    struct page *page;
    unsigned long order;

    size = PAGE_ALIGN(size);
    order = get_order(size);
    page = alloc_pages(GFP_KERNEL, order);
    if (!page)
    {
	printk(__FUNCTION__": Failed to allocate pages\n");
	return NULL;
    }
    return page_address(page);
}

static inline void ix_dma_free(void *ptr, int size)
{
    unsigned long order;

    size = PAGE_ALIGN(size);
    order = get_order(size);
    free_pages((unsigned int)ptr, order);
}

#elif defined(__ECOS)

#include <cyg/hal/hal_cache.h>

/**
 * @defgroup IxOsCacheMMU IXP4xx OS Cache MMU (IxOsCacheMMU) API
 *
 * @brief This service provides services to the access components and codelets to
 * abstract out any cache coherency issues and mmu mappings.
 *
 * @{
 */

/**
 * @def IX_MMU_VIRTUAL_TO_PHYSICAL_TRANSLATION(addr)
 *
 * @brief Return a virtual address for the provided  physical address.
 *
 * Return a virtual address for the provided  physical address.
 *
 * <hr>
 */
#define IX_MMU_VIRTUAL_TO_PHYSICAL_TRANSLATION(addr) (addr)

/**
 * @def IX_MMU_PHYSICAL_TO_VIRTUAL_TRANSLATION(addr)
 * 
 * @brief Return a physical address for the provided virtual.
 * 
 * Return a physical address for the provided virtual address
 *
 * <hr>
 */
#define IX_MMU_PHYSICAL_TO_VIRTUAL_TRANSLATION(addr) (addr)

/**
 * @def IX_XSCALE_CACHE_LINE_SIZE
 *
 * @brief IX_XSCALE_CACHE_LINE_SIZE = size of cache line for both flush and invalidate.
 *
 *
 * <hr>
 */
#define IX_XSCALE_CACHE_LINE_SIZE (32)

/**
 * @def IX_ACC_DRAM_PHYS_OFFSET
 *
 * @brief PHYS_OFFSET = Physical DRAM offset..
 *
 *
 * <hr>
 */
#define IX_ACC_DRAM_PHYS_OFFSET     (0x00000000UL) 



/**
 * @def  IX_ACC_DATA_CACHE_INVALIDATE(addr,size) 
 *
 * @brief Invalidate a cache range.  
 *
 * Invalidate a cache range.  
 * @note
 * This is typically done prior to submitting a buffer to the NPE's which you
 * expect the NPE to populate with data.
 * @attention The size argument must be a multiple of cacheline size, i.e. a multiple
 * if 32bytes for the XSCALE. The argument shall be rounded up to the next 32byte boundry.
 * Extreem care must be taken when invalidating cache lines due. 
 *
 * - If memory space used is non cached, then this function may be null.
 * - Functionality required: 
 *   -# Non-Cached space         : No functionality required. 
 *   -# Write Through Enabled    : Invalidate area specified
 *   -# Copy Back Enabled        : Invalidate area specified
 * 
 *
 * <hr>
 *
 */
#define IX_ACC_DATA_CACHE_INVALIDATE(addr,size) HAL_DCACHE_INVALIDATE((addr),(size))

/** 
 * 
 * @def IX_ACC_DATA_CACHE_FLUSH(addr,size)
 *
 * @brief Flush a cache range to physical memory.  
 *
 * Flush a cache range to physical memory.
 * @note
 * This is typically done prior to submitting a buffer to the NPE's which you
 * expect the NPE to read from. Entire Cache lines will be flushed.
 *
 * - If memory space used is non cached, then this function does may be null.
 * - Functionality required: 
 *   -# Non-Cached space      	: Flush CPU WB, No cache Flush.
 *   -# Write Through Enabled  	: Flush CPU WB, No cache Flush. 
 *   -# Copy Back Enabled     	: Flush CPU WB, Invalidate area specified
 *
 * <hr>
 *
 */
#define IX_ACC_DATA_CACHE_FLUSH(addr,size) HAL_DCACHE_FLUSH((addr),(size))


/** 
 *
 * @def  IX_ACC_DRV_DMA_MALLOC
 *
 * @brief Allocate memory for driver use, that will be shared between XScale and NPE's.
 *
 *  Allocate memory for driver use, that will be shared between XScale and NPE's. 
 * @note The buffer allocated with have the system defined atributes, and as such the Invalidate and 
 * flush macros functionality must be updated.
 *
 * @param uint32 size -  number of bytes of memory requested.
 *
 * @return  void * Pointer to memory that can be used between XScale and NPE's. 
 * 
 *
 * <hr>
 */
#define IX_ACC_DRV_DMA_MALLOC(size)  cacheDmaMalloc(size) 

/** 
 *
 * @def  IX_ACC_DRV_DMA_FREE
 *
 * @brief Free memory allocated from IX_ACC_DRV_DMA_MALLOC.
 *
 * This function frees the memory allocated from @a IX_ACC_DRV_DMA_MALLOC.
 *
 * @param void * Pointer - pointer to the memory area to be freed.
 * @param uint32 size -  number of bytes of memory allocated.
 *
 * @return  void
 *
 * <hr>
 */
#define IX_ACC_DRV_DMA_FREE(ptr,size)  cacheDmaFree(ptr)

#else /* #elif __linux */

#include <stdlib.h>

/* default implementatins of above macros here.
 */

#define IX_MMU_VIRTUAL_TO_PHYSICAL_TRANSLATION(addr) (addr)
#define IX_MMU_PHYSICAL_TO_VIRTUAL_TRANSLATION(addr) (addr)
#define IX_ACC_DATA_CACHE_INVALIDATE(addr,size)
#define IX_ACC_DATA_CACHE_FLUSH(addr,size) 

#define IX_XSCALE_CACHE_LINE_SIZE 32

/* allocate on a cache line boundary (null pointers are
 *  not affected by this operation)
 */

#define IX_ACC_DRV_DMA_MALLOC(size)  \
    (void *)(~(IX_XSCALE_CACHE_LINE_SIZE - 1) & \
    ((IX_XSCALE_CACHE_LINE_SIZE - 1) + \
    (unsigned int) malloc((size) + (IX_XSCALE_CACHE_LINE_SIZE - 1))))

#define IX_ACC_DRV_DMA_FREE(ptr,size) \
    free(ptr) 

#endif

/**
 * @} IxOsCacheMMU
 */

#endif /* IxOsCacheMMU_H */

