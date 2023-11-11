/**
 * @file IxNpeDl.h
 *
 * @date 14 December 2001

 * @brief This file contains the public API of the IXP4xx NPE Downloader
 *        component.
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
 * @defgroup IxNpeDl IXP4xx NPE-Downloader (IxNpeDl) API
 *
 * @brief The Public API for the IXP4xx NPE Downloader
 * 
 * @{
 */

#ifndef IXNPEDL_H
#define IXNPEDL_H


/*
 * Put the user defined include files required
 */
#include "IxTypes.h"


/*
 * #defines for function return types, etc.
 */

/**
 * @def IX_NPEDL_PARAM_ERR
 *
 * @brief NpeDl function return value for a parameter error
 */
#define IX_NPEDL_PARAM_ERR               2

/**
 * @def IX_NPEDL_RESOURCE_ERR
 *
 * @brief NpeDl function return value for a resource error
 */
#define IX_NPEDL_RESOURCE_ERR            3

/**
 * @def IX_NPEDL_CRITICAL_NPE_ERR
 *
 * @brief NpeDl function return value for a Critical NPE error occuring during
          download. Assume NPE is left in unstable condition if this value is
	  returned.
 */
#define IX_NPEDL_CRITICAL_NPE_ERR        4

/**
 * @def IX_NPEDL_CRITICAL_MICROCODE_ERR
 *
 * @brief NpeDl function return value for a Critical Microcode error
 *        discovered during download. Assume NPE is left in unstable condition
 *        if this value is returned.
 */
#define IX_NPEDL_CRITICAL_MICROCODE_ERR  5


/*
 * Typedefs
 */

/**
 * @typedef IxNpeDlBuildId
 * @brief Used to make up Build ID field of Version Id
 */
typedef UINT8 IxNpeDlBuildId;

/**
 * @typedef IxNpeDlMajor
 * @brief Used to make up Major Release field of Version Id
 */
typedef UINT8 IxNpeDlMajor;

/**
 * @typedef IxNpeDlMinor
 * @brief Used to make up Minor Revision field of Version Id
 */
typedef UINT8 IxNpeDlMinor;


/*
 * Enums
 */

/**
 * @enum IxNpeDlNpeId
 * @brief NpeId numbers to identify NPE A, B or C
 */
typedef enum 
{
  IX_NPEDL_NPEID_NPEA = 0,    /**< Identifies NPE A */
  IX_NPEDL_NPEID_NPEB,        /**< Identifies NPE B */
  IX_NPEDL_NPEID_NPEC,        /**< Identifies NPE C */
  IX_NPEDL_NPEID_MAX          /**< Total Number of NPEs */
} IxNpeDlNpeId;


/*
 * Structs
 */

/**
 * @brief Version Id to identify each version contained in an image
 */
typedef struct
{
    IxNpeDlNpeId   npeId;   /**< NPE ID */
    IxNpeDlBuildId buildId; /**< Build ID indicates functionality of version */
    IxNpeDlMajor   major;   /**< Major Release Number */
    IxNpeDlMinor   minor;   /**< Minor Revision Number */
} IxNpeDlVersionId;


/*
 * Prototypes for interface functions
 */

/**
 * @ingroup IxNpeDl
 *
 * @fn IX_STATUS ixNpeDlMicrocodeImageOverride (UINT32 *clientImage)
 * 
 * @brief This instructs NPE Downloader to use client-supplied microcode image.
 *
 * @param UINT32* [in] clientImage - Pointer to the client-supplied
 *                                   NPE microcode image
 *
 * This function sets NPE Downloader to use a client-supplied microcode image
 * instead of the standard image which is included by the NPE Downloader.
 * <b>This function is provided mainly for increased testability and should not
 * be used in normal circumstances.</b> When not used, NPE Downloader will use
 * a "built-in" image, local to this component, which should always contain the
 * latest microcode for the NPEs.
 * 
 * @pre
 *      - <i>clientImage</i> should point to a microcode image valid for use
 *        by the NPE Downloader component.
 *
 * @post
 *      - the client-supplied image will be used for all subsequent operations
 *        performed by the NPE Downloader
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_NPEDL_PARAM_ERR if a parameter error occured
 *      - IX_FAIL if the client-supplied image did not contain a valid signature
 */ 
IX_STATUS
ixNpeDlMicrocodeImageOverride (UINT32 *clientImage);


/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlVersionDownload (IxNpeDlVersionId *versionIdPtr,
                                                BOOL verify)
 * 
 * @brief Download microcode to a NPE
 *
 * @param IxNpeDlVersionId* [in] versionIdPtr - Pointer to Id of the microcode 
 *                                              version to download.
 * @param BOOL [in] verify     - ON/OFF option to verify the download. If ON
 *                               (verify == TRUE), the Downloader will read back
 *                               each word written to the NPE registers to
 *                               ensure the download operation was successful.
 * 
 * Using the <i>versionIdPtr</i>, this function locates a particular version of
 * microcode in the microcode image in memory, and downloads the microcode to a
 * particular NPE.
 * 
 * @pre
 *         - The Client is responsible for ensuring mutual access to the NPE.
 *         - The Client should stop the NPE before this function is used.
 * @post
 *         - The NPE Instruction Pipeline will be cleared if State Information
 *           has been downloaded.
 *         - If the download fails with a critical error, the NPE may
 *           be left in an ususable state.
 * @return 
 *         - IX_SUCCESS if the download was successful 
 *         - IX_NPEDL_PARAM_ERR if a parameter error occured
 *         - IX_NPEDL_CRITICAL_NPE_ERR if a critical NPE error occured during
 *           download
 *         - IX_PARAM_CRITICAL_MICROCODE_ERR if a critical microcode error 
 *           occured during download
 *         - IX_FAIL otherwise
 */ 
PUBLIC IX_STATUS
ixNpeDlVersionDownload (IxNpeDlVersionId *versionIdPtr, 
			BOOL verify);


/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlAvailableVersionsCountGet (UINT32 *numVersionsPtr)
 * 
 * @brief Get the number of versions available in a microcode image
 *
 * @param UINT32* [out] numVersionsPtr - A pointer to the number of versions in
 *                                       the image.
 * 
 * Gets the number of versions available in the microcode image.
 * Then returns this in a variable pointed to by <i>numVersionsPtr</i>.
 * 
 * @pre  
 *     - The Client should declare the variable to which numVersionsPtr points
 *
 * @post
 *
 * @return 
 *     - IX_SUCCESS if the operation was successful
 *     - IX_NPEDL_PARAM_ERR if a parameter error occured
 *     - IX_FAIL otherwise
 */ 
PUBLIC IX_STATUS 
ixNpeDlAvailableVersionsCountGet (UINT32 *numVersionsPtr);


/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlAvailableVersionsListGet (IxNpeDlVersionId *versionIdListPtr,
                                                         UINT32 *listSizePtr)
 * 
 * @brief Get a list of the versions available in a microcode image
 *
 * @param IxNpeDlVersionId* [out] versionIdListPtr - Array to contain list of
 *                                                   version Ids (memory 
 *                                                   allocated by Client).
 * @param UINT32* [inout] listSizePtr  - As an input, this param should point
 *                                       to the max number of Ids the
 *                                       <i>versionIdListPtr</i> array can
 *                                       hold. NpeDl will replace the input
 *                                       value of this parameter with the
 *                                       number of version Ids actually filled
 *                                       into the array before returning.
 * 
 * Finds list of versions available in the microcode image.
 * Fills these into the array pointed to by <i>versionIdListPtr</i>
 * 
 * @pre 
 *     - The Client should declare the variable to which numVersionsPtr points
 *     - The Client should create an array (<i>versionIdListPtr</i>) large
 *       enough to contain all the version Ids in the image
 *
 * @post
 *
 * @return
 *     - IX_SUCCESS if the operation was successful
 *     - IX_NPEDL_PARAM_ERR if a parameter error occured
 *     - IX_FAIL otherwise
 */
PUBLIC IX_STATUS
ixNpeDlAvailableVersionsListGet (IxNpeDlVersionId *versionIdListPtr,
				 UINT32 *listSizePtr);


/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlLoadedVersionGet (IxNpeDlNpeId npeId,
                                                IxNpeDlVersionId *versionIdPtr)
 * 
 * @brief Gets the Id of the version currently loaded on a particular NPE
 *
 * @param IxNpeDlNpeId [in] npeId              - Id of the target NPE.
 * @param IxNpeDlVersionId* [out] versionIdPtr - Pointer to the where the
 *                                               version id should be stored.
 * 
 * If a version of microcode was previously downloaded successfully to the NPE
 * by NPE Downloader, this function returns in <i>versionIdPtr</i> the version
 * Id of that version loaded on the NPE.
 * 
 * @pre
 *     - The Client has allocated memory to the <i>versionIdPtr</i> pointer.
 *
 * @post
 *
 * @return
 *     -  IX_SUCCESS if the operation was successful
 *     -  IX_NPEDL_PARAM_ERR if a parameter error occured
 *     -  IX_FAIL if the NPE doesn't currently have a version loaded
 */ 
PUBLIC IX_STATUS
ixNpeDlLoadedVersionGet (IxNpeDlNpeId npeId,
			 IxNpeDlVersionId *versionIdPtr);


/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlNpeStopAndReset (IxNpeDlNpeId npeId)
 * 
 * @brief Stops and Resets an NPE
 *
 * @param IxNpeDlNpeId [in] npeId - Id of the target NPE.
 * 
 * This function performs a soft NPE reset by writing reset values to
 * particular NPE registers. Note that this does not reset NPE co-processors.
 * The client can use this before downloading a new version of microcode to the
 * NPE.  This implicitly stops NPE code execution before resetting the NPE.
 * 
 * @pre
 *     - The Client is responsible for ensuring mutual access to the NPE.
 *
 * @post
 *
 * @return 
 *      - IX_SUCCESS if the operation was successful
 *      - IX_NPEDL_PARAM_ERR if a parameter error occured
 *      - IX_FAIL otherwise
 */ 
PUBLIC IX_STATUS
ixNpeDlNpeStopAndReset (IxNpeDlNpeId npeId);


/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlNpeExecutionStart (IxNpeDlNpeId npeId)
 * 
 * @brief Starts code execution on a NPE
 *
 * @param IxNpeDlNpeId [in] npeId - Id of the target NPE
 * 
 * Starts execution of code on a particular NPE.  A client would typically use
 * this after a download to NPE is performed, to start/restart code execution 
 * on the NPE.
 *
 * @pre
 *     - The Client is responsible for ensuring mutual access to the NPE.
 *     - Note that this function does not set the NPE Next Program Counter 
 *       (NextPC), so it should be set beforehand if required by downloading 
 *       appropriate State Information (using ixNpeDlVersionDownload()).
 *
 * @post
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_NPEDL_PARAM_ERR if a parameter error occured
 *      - IX_FAIL otherwise
 */ 
PUBLIC IX_STATUS
ixNpeDlNpeExecutionStart (IxNpeDlNpeId npeId);


/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlNpeExecutionStop (IxNpeDlNpeId npeId)
 * 
 * @brief Stops code execution on a NPE
 *
 * @param IxNpeDlNpeId [in] npeId - Id of the target NPE
 * 
 * Stops execution of code on a particular NPE.  This would typically be used
 * by a client before a download to NPE is performed, to stop code execution on
 * an NPE, unless ixNpeDlNpeStopAndReset() is used instead.  Unlike
 * ixNpeDlNpeStopAndReset(), this function only halts the NPE and leaves
 * all registers and settings intact. This is useful, for example, between 
 * stages of a multi-stage download, to stop the NPE prior to downloading the
 * next version.
 *
 * @pre
 *     - The Client is responsible for ensuring mutual access to the NPE.
 *
 * @post
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_NPEDL_PARAM_ERR if a parameter error occured
 *      - IX_FAIL otherwise
 */ 
PUBLIC IX_STATUS
ixNpeDlNpeExecutionStop (IxNpeDlNpeId npeId);


/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC void ixNpeDlStatsShow (void)
 *
 * @brief This function will display run-time statistics from the IxNpeDl
 *        component
 *
 * @return none
 */
PUBLIC void
ixNpeDlStatsShow (void);


/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC void ixNpeDlStatsReset (void)
 *
 * @brief This function will reset the statistics of the IxNpeDl component
 *
 * @return none
 */
PUBLIC void
ixNpeDlStatsReset (void);


#endif /* IXNPEDL_H */

/**
 * @} defgroup IxNpeDl
 */
