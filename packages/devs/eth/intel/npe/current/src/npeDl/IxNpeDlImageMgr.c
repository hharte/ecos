/**
 * @file IxNpeDlImageMgr.c
 *
 * @author Intel Corporation
 * @date 09 January 2002
 *
 * @brief This file contains the implementation of the private API for the 
 *        IXP425 NPE Downloader ImageMgr module
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


/*
 * Put the system defined include files required.
 */
#include <stdio.h>


/*
 * Put the user defined include files required.
 */
#include "IxNpeDlImageMgr_p.h"
#include "IxNpeDlMacros_p.h"

/*
 * define the flag which toggles the firmare inclusion
 */
#define IX_NPE_MICROCODE_FIRMWARE_INCLUDED 1
#include "IxNpeMicrocode.h"


/*
 * #defines and macros used in this file.
 */
#define IX_NPEDL_VERSIONID_NPEID_OFFSET   24
#define IX_NPEDL_VERSIONID_BUILDID_OFFSET 16
#define IX_NPEDL_VERSIONID_MAJOR_OFFSET   8
#define IX_NPEDL_VERSIONID_MINOR_OFFSET   0

#define IX_NPEDL_MASK_LOWER_BYTE_OF_WORD  0x000000FF


/*
 * Typedefs whose scope is limited to this file.
 */

typedef struct
{
    UINT32 size;
    UINT32 offset;
    UINT32 id;
} IxNpeDlImageMgrVersionEntry;

typedef union
{
    IxNpeDlImageMgrVersionEntry version;
    UINT32 eohMarker;
} IxNpeDlImageMgrHeaderEntry;

typedef struct
{
    UINT32 signature;
    /* 1st entry in the header (there may be more than one) */
    IxNpeDlImageMgrHeaderEntry entry[1];
} IxNpeDlImageMgrImageHeader;

/* module statistics counters */
typedef struct
{
    UINT32 invalidSignature;
    UINT32 versionIdListOverflow;
    UINT32 versionIdNotFound;
} IxNpeDlImageMgrStats;


/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */
static IxNpeDlImageMgrStats ixNpeDlImageMgrStats;

static UINT32 *ixNpeDlMicroCodeImage = IX_NPEDL_MicrocodeImage; /* default image */


/*
 * static function prototypes.
 */
PRIVATE BOOL
ixNpeDlImageMgrSignatureCheck (UINT32 *microCodeImage);

PRIVATE IxNpeDlVersionId
ixNpeDlImageMgrVersionIdFormat (UINT32 rawVersionId);

PRIVATE BOOL
ixNpeDlImageMgrVersionIdCompare (IxNpeDlVersionId *versionIdA, 
				 IxNpeDlVersionId *versionIdB);


/*
 * Function definition: ixNpeDlImageMgrMicrocodeImageOverride
 */
IX_STATUS
ixNpeDlImageMgrMicrocodeImageOverride (
    UINT32 *clientImage)
{
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Entering ixNpeDlImageMgrMicrocodeImageOverride\n");

    if (ixNpeDlImageMgrSignatureCheck (clientImage))
    {
	ixNpeDlMicroCodeImage = clientImage;
    }
    else
    {
	IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrMicrocodeImageOverride: "
			       "Client-supplied image has invalid signature\n");
	status = IX_FAIL;
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Exiting ixNpeDlImageMgrMicrocodeImageOverride: status = %d\n",
		     status);
    return status;
}


/*
 * Function definition: ixNpeDlImageMgrVersionListExtract
 */
IX_STATUS
ixNpeDlImageMgrVersionListExtract (
    IxNpeDlVersionId *versionListPtr,
    UINT32 *numVersions)
{
    UINT32 rawVersionId;
    IxNpeDlVersionId formattedVersionId;
    IX_STATUS status = IX_SUCCESS;
    UINT32 versionCount = 0;
    IxNpeDlImageMgrImageHeader *header;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Entering ixNpeDlImageMgrVersionListExtract\n");

    header = (IxNpeDlImageMgrImageHeader *) ixNpeDlMicroCodeImage;

    if (ixNpeDlImageMgrSignatureCheck (ixNpeDlMicroCodeImage))
    {
	/* for each version entry in the image header ... */
	while (header->entry[versionCount].eohMarker !=
	       IX_NPEDL_IMAGEMGR_END_OF_HEADER)
	{
	    /*
	     * if the version list container from calling function has capacity,
	     * add the version id to the list 
	     */
	    if ((versionListPtr != NULL) && (versionCount < *numVersions))
	    {
		rawVersionId = header->entry[versionCount].version.id;
		formattedVersionId = ixNpeDlImageMgrVersionIdFormat (rawVersionId);
		versionListPtr[versionCount] = formattedVersionId;
	    }
	    /* versionCount reflects no. of version entries in image header */
	    versionCount++;  
	}
	
	/*
	 * if version list container from calling function was too small to
	 * contain all version ids in the header, set return status to FAIL
	 */
	if ((versionListPtr != NULL) && (versionCount > *numVersions))
	{
	    status = IX_FAIL;
	    IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrVersionListExtract: "
				   "number of Ids found exceeds list capacity\n");
	    ixNpeDlImageMgrStats.versionIdListOverflow++;
	}
	/* return number of version ids found in image header */
	*numVersions = versionCount;  
    }
    else
    {
	status = IX_FAIL;
	IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrVersionListExtract: "
			       "invalid signature in image\n");
    }
    
    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Exiting ixNpeDlImageMgrVersionListExtract: status = %d\n",
		     status);
    return status;
}


/*
 * Function definition: ixNpeDlImageMgrVersionLocate
 */
IX_STATUS
ixNpeDlImageMgrVersionLocate (
    IxNpeDlVersionId *versionId,
    UINT32 **versionPtr,
    UINT32 *versionSize)
{
    UINT32 versionOffset;
    UINT32 rawVersionId;
    IxNpeDlVersionId formattedVersionId;
    /* used to index version entries in image header */
    UINT32 versionCount = 0;   
    IX_STATUS status = IX_FAIL;
    IxNpeDlImageMgrImageHeader *header;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlImageMgrVersionLocate\n");

    header = (IxNpeDlImageMgrImageHeader *) ixNpeDlMicroCodeImage;

    if (ixNpeDlImageMgrSignatureCheck (ixNpeDlMicroCodeImage))
    {
	/* for each version entry in the image header ... */
	while (header->entry[versionCount].eohMarker !=
	       IX_NPEDL_IMAGEMGR_END_OF_HEADER)
	{
	    rawVersionId = header->entry[versionCount].version.id;
	    formattedVersionId = ixNpeDlImageMgrVersionIdFormat (rawVersionId);
	    /* if a match for versionId is found in the image header... */
	    if (ixNpeDlImageMgrVersionIdCompare (versionId, &formattedVersionId))
	    {
		/*
		 * get pointer to the version in the image using offset from
		 * 1st word in image
		 */
		versionOffset = header->entry[versionCount].version.offset;
		*versionPtr = &ixNpeDlMicroCodeImage[versionOffset];
		/* get the version size */
		*versionSize = header->entry[versionCount].version.size;
		status = IX_SUCCESS;
		break;
	    }
	    versionCount++;
	}
	if (status != IX_SUCCESS)
	{
	    IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrVersionLocate: "
				   "versionId not found in image header\n");
	    ixNpeDlImageMgrStats.versionIdNotFound++;
	}
    }
    else
    {
	IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrVersionLocate: "
			       "invalid signature in image\n");
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlImageMgrVersionLocate: status = %d\n", status);
    return status;
}


/*
 * Function definition: ixNpeDlImageMgrSignatureCheck
 */
PRIVATE BOOL
ixNpeDlImageMgrSignatureCheck (UINT32 *microCodeImage)
{
    IxNpeDlImageMgrImageHeader *header =
	(IxNpeDlImageMgrImageHeader *) microCodeImage;
    BOOL result = TRUE;

    if (header->signature != IX_NPEDL_IMAGEMGR_SIGNATURE)
    {
	result = FALSE;
	ixNpeDlImageMgrStats.invalidSignature++;
    }

    return result;
}


/*
 * Function definition: ixNpeDlImageMgrVersionIdFormat
 */
PRIVATE IxNpeDlVersionId
ixNpeDlImageMgrVersionIdFormat (
    UINT32 rawVersionId)
{
    IxNpeDlVersionId formattedVersionId;
    
    formattedVersionId.npeId = (rawVersionId >>
				IX_NPEDL_VERSIONID_NPEID_OFFSET) &
	IX_NPEDL_MASK_LOWER_BYTE_OF_WORD;
    formattedVersionId.buildId = (rawVersionId >> 
				  IX_NPEDL_VERSIONID_BUILDID_OFFSET) &
	IX_NPEDL_MASK_LOWER_BYTE_OF_WORD;
    formattedVersionId.major = (rawVersionId >>
				IX_NPEDL_VERSIONID_MAJOR_OFFSET) &
	IX_NPEDL_MASK_LOWER_BYTE_OF_WORD;
    formattedVersionId.minor = (rawVersionId >>
				IX_NPEDL_VERSIONID_MINOR_OFFSET) &
	IX_NPEDL_MASK_LOWER_BYTE_OF_WORD;

    return formattedVersionId;
}


/*
 * Function definition: ixNpeDlImageMgrVersionIdCompare
 */
PRIVATE BOOL
ixNpeDlImageMgrVersionIdCompare (
    IxNpeDlVersionId *versionIdA,
    IxNpeDlVersionId *versionIdB)
{
    if ((versionIdA->npeId   == versionIdB->npeId)   &&
	(versionIdA->buildId == versionIdB->buildId) &&
	(versionIdA->major   == versionIdB->major)   &&
	(versionIdA->minor   == versionIdB->minor))
    {
	return TRUE;
    }
    else
    {
	return FALSE;
    }
}


/*
 * Function definition: ixNpeDlImageMgrStatsShow
 */
void
ixNpeDlImageMgrStatsShow (void)
{
    printf ("\nixNpeDlImageMgrStatsShow:\n");

    printf ("\tInvalid Image Signatures: %u\n",
	    ixNpeDlImageMgrStats.invalidSignature);
    printf ("\tVersion Id List capacity too small: %u\n",
	    ixNpeDlImageMgrStats.versionIdListOverflow);
    printf ("\tVersion Id not found: %u\n",
	    ixNpeDlImageMgrStats.versionIdNotFound);
    
    printf ("\n");
}


/*
 * Function definition: ixNpeDlImageMgrStatsReset
 */
void
ixNpeDlImageMgrStatsReset (void)
{
    ixNpeDlImageMgrStats.invalidSignature = 0;
    ixNpeDlImageMgrStats.versionIdListOverflow = 0;
    ixNpeDlImageMgrStats.versionIdNotFound = 0;
}
