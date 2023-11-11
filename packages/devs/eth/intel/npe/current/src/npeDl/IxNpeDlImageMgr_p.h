/**
 * @file IxNpeDlImageMgr_p.h
 *
 * @author Intel Corporation
 * @date 14 December 2001

 * @brief This file contains the private API for the ImageMgr module
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
 * @defgroup IxNpeDlImageMgr_p IxNpeDlImageMgr_p
 *
 * @brief The private API for the IxNpeDl ImageMgr module
 * 
 * @{
 */

#ifndef IXNPEDLIMAGEMGR_P_H
#define IXNPEDLIMAGEMGR_P_H


/*
 * Put the user defined include files required.
 */
#include "IxNpeDl.h"
#include "IxTypes.h"


/*
 * #defines and macros used in this file.
 */

/**
 * @def IX_NPEDL_IMAGEMGR_SIGNATURE
 *
 * @brief Signature found as 1st word in a microcode image
 */
#define IX_NPEDL_IMAGEMGR_SIGNATURE      0xDEADBEEF

/**
 * @def IX_NPEDL_IMAGEMGR_END_OF_HEADER
 *
 * @brief Marks end of header in a microcode image
 */
#define IX_NPEDL_IMAGEMGR_END_OF_HEADER  0xFFFFFFFF


/*
 * Prototypes for interface functions
 */

/**
 * @fn IX_STATUS ixNpeDlImageMgrMicrocodeImageOverride (UINT32 *clientImage)
 * 
 * @brief This instructs NPE Downloader to use client-supplied microcode image.
 *
 * This function sets NPE Downloader to use a client-supplied microcode image
 * instead of the standard image which is included by the NPE Downloader.
 * 
 * @pre
 *    - <i>clientImage</i> should point to a microcode image valid for use
 *      by the NPE Downloader component.
 *
 * @post
 *    - the client-supplied image will be used for all subsequent operations
 *      performed by the NPE Downloader
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL if the client-supplied image did not contain a valid signature
 */ 
IX_STATUS
ixNpeDlImageMgrMicrocodeImageOverride (UINT32 *clientImage);


/**
 * @fn IX_STATUS ixNpeDlImageMgrVersionListExtract (IxNpeDlVersionId *versionListPtr,
                                                    UINT32 *numVersions)
 * 
 * @brief Extracts a list of versions available in the NPE microcode image.
 *
 * @param IxNpeDlVersionId* [out] versionListPtr - pointer to array to contain
 *                                                 a list of versions. If NULL,
 *                                                 only the number of versions 
 *                                                 is returned (in
 *                                                 <i>numVersions</i>)
 * @param UINT32* [inout] numVersions - As input, it points to a variable
 *                                      containing the number of versions which
 *                                      can be stored in the
 *                                      <i>versionListPtr</i> array. Its value
 *                                      is ignored as input if
 *                                      <i>versionListPtr</i> is NULL. As an
 *                                      output, it will contain number of
 *                                      versions in the image.
 * 
 * This function reads the header of the microcode image and extracts a list of the
 * versions available in the image.  It can also be used to find the number of
 * versions in the image.
 * 
 * @pre
 *    - if <i>versionListPtr</i> != NULL, <i>numVersions</i> should reflect the
 *      number of version Id elements the <i>versionListPtr</i> can contain.
 *
 * @post
 *    - <i>numVersions</i> will reflect the number of version Id's found in the
 *      microcode image.
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL otherwise
 */ 
IX_STATUS
ixNpeDlImageMgrVersionListExtract (IxNpeDlVersionId *versionListPtr,
				   UINT32 *numVersions);


/**
 * @fn IX_STATUS ixNpeDlImageMgrVersionLocate (IxNpeDlVersionId *versionId,
                                               UINT32 **versionPtr,
                                               UINT32 *versionSize)
 * 
 * @brief Finds a version block in the NPE microcode image.
 *
 * @param IxNpeDlVersionId* [in] versionId - the id of the version to locate
 * @param UINT32** [out] versionPtr        - pointer to the version in memory
 * @param UINT32* [out] versionSize        - size (in 32-bit words) of version
 * 
 * This function examines the header of the microcode image for the location
 * and size of the specified version.
 * 
 * @pre
 *
 * @post
 *
 * @return 
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL otherwise
 */ 
IX_STATUS
ixNpeDlImageMgrVersionLocate (IxNpeDlVersionId *versionId,
			      UINT32 **versionPtr,
			      UINT32 *versionSize);


/**
 * @fn void ixNpeDlImageMgrStatsShow (void)
 *
 * @brief This function will display the statistics of the IxNpeDl ImageMgr
 *        module
 *
 * @return none
 */
void
ixNpeDlImageMgrStatsShow (void);


/**
 * @fn void ixNpeDlImageMgrStatsReset (void)
 *
 * @brief This function will reset the statistics of the IxNpeDl ImageMgr
 *        module
 *
 * @return none
 */
void
ixNpeDlImageMgrStatsReset (void);


#endif /* IXNPEDLIMAGEMGR_P_H */

/**
 * @} defgroup IxNpeDlImageMgr_p
 */
