/**
 * ============================================================================
 * = COPYRIGHT
 * -- Intel Copyright Notice --
 * INTEL CONFIDENTIAL
 *
 * Copyright 2002 Intel Corporation All Rights Reserved.
 *
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
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise.  Any license under
 * such intellectual property rights must be express and approved by
 * Intel in writing.
 *
 * For further details, please see the file README.TXT distributed with
 * this software.
 * -- End Intel Copyright Notice --
 *
 * = PRODUCT
 *      Intel(r) IXA SDK for the IXP2X00 Network Processor
 *
 * = FILENAME
 *      ix_os_type.h
 *
 * = DESCRIPTION
 *      This file provides protable symbol definitions for the current OS type.
 *
 * = AUTHOR
 *      Intel Corporation
 *
 * = CHANGE HISTORY
 *      4/22/2002 4:43:30 PM - creation time 
 * ============================================================================
 */

#if !defined(__IX_OS_TYPE_H__)
#define __IX_OS_TYPE_H__


#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */

/**
 * Define symbols for each supported OS
 */
#define _IX_OS_VXWORKS_     1   /* VxWorks OS */
#define _IX_OS_LINUX_       2   /* Linux OS */
#define _IX_OS_WIN32_       3   /* Windows Win32 OS */
#define _IX_OS_ECOS_        4   /* eCos OS */

#if !defined(_IX_OS_TYPE_)
#   if defined(__linux)
#       define _IX_OS_TYPE_ _IX_OS_LINUX_
#   elif defined(__vxworks)
#       define _IX_OS_TYPE_ _IX_OS_VXWORKS_
#   elif defined(WIN32)
#       define _IX_OS_TYPE_ _IX_OS_WIN32_
#   elif defined(__ECOS)
#       define _IX_OS_TYPE_ _IX_OS_ECOS_
#   endif 
#endif /* !defined(_IX_OS_TYPE_) */


/**
 * Check if the OS type is defined and if it is supported
 */
#if !defined(_IX_OS_TYPE_) || \
        ((_IX_OS_TYPE_ != _IX_OS_LINUX_) && \
        (_IX_OS_TYPE_ != _IX_OS_VXWORKS_) && \
        (_IX_OS_TYPE_ != _IX_OS_WIN32_) && \
        (_IX_OS_TYPE_ != _IX_OS_ECOS_))
#   error You are trying to build on an unsupported OS!
#endif


#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__IX_OS_TYPE_H__) */

