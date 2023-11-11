/**
 * @file IxNpeDl.c
 *
 * @author Intel Corporation
 * @date 08 January 2002
 *
 * @brief This file contains the implementation of the public API for the
 *        IXP425 NPE Downloader component
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
 * Put the system defined include files required
 */
#include <stdio.h>


/*
 * Put the user defined include files required
 */
#include "IxNpeDl.h"
#include "IxNpeDlImageMgr_p.h"
#include "IxNpeDlNpeMgr_p.h"
#include "IxNpeDlMacros_p.h"


/*
 * Typedefs whose scope is limited to this file.
 */
typedef struct
{
    BOOL validVersion;
    IxNpeDlVersionId versionId;
} IxNpeDlNpeState;

/* module statistics counters */
typedef struct
{
    UINT32 attemptedDownloads;
    UINT32 successfulDownloads;
    UINT32 criticalFailDownloads;
} IxNpeDlStats;


/*
 * Variable declarations global to this file only.  Externs are followed 
 * by static variables.
 */
static IxNpeDlNpeState ixNpeDlNpeState[IX_NPEDL_NPEID_MAX] = 
{
    {FALSE, {IX_NPEDL_NPEID_MAX, 0, 0, 0}},
    {FALSE, {IX_NPEDL_NPEID_MAX, 0, 0, 0}},
    {FALSE, {IX_NPEDL_NPEID_MAX, 0, 0, 0}}
};

static IxNpeDlStats ixNpeDlStats;


/*
 * Function definition: ixNpeDlMicrocodeImageOverride
 */
IX_STATUS
ixNpeDlMicrocodeImageOverride (UINT32 *clientImage)
{
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlMicrocodeImageOverride\n");

    if (clientImage == NULL)
    {
	status = IX_NPEDL_PARAM_ERR;
	IX_NPEDL_ERROR_REPORT ("ixNpeDlMicrocodeImageOverride - "
			       "invalid parameter\n");
    }
    else
    {
	status = ixNpeDlImageMgrMicrocodeImageOverride (clientImage);
	if (status != IX_SUCCESS)
	{
	    status = IX_FAIL;
	}
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlMicrocodeImageOverride : status = %d\n",
		     status);
    return status;
}


/*
 * Function definition: ixNpeDlVersionDownload
 */
IX_STATUS
ixNpeDlVersionDownload (IxNpeDlVersionId *versionIdPtr,
			BOOL verify)
{
    IxNpeDlNpeId  npeId           = versionIdPtr->npeId;
    UINT32       *versionCodePtr  = NULL;
    UINT32        versionSize;
    IX_STATUS     status;
    
    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlVersionDownload\n");
    ixNpeDlStats.attemptedDownloads++;
    
    /* Check input parameters */
    if ((npeId >= IX_NPEDL_NPEID_MAX) || (npeId < 0))
    {
	status = IX_NPEDL_PARAM_ERR;
	IX_NPEDL_ERROR_REPORT ("ixNpeDlVersionDownload - invalid parameter\n");
    }
    else
    {
	/* Locate version */
	status = ixNpeDlImageMgrVersionLocate (versionIdPtr, &versionCodePtr,
					       &versionSize);
	if (status == IX_SUCCESS)
	{
	    status = ixNpeDlNpeMgrVersionLoad (npeId, versionCodePtr,
					       verify);
	    
	    /*
	     * If download was successful, store version Id in list of 
	     * currently loaded versions. If a critical error occured
	     * during download, record that the NPE has an invalid version
	     */
	    if (status == IX_SUCCESS)
	    {
		ixNpeDlNpeState[npeId].versionId = *versionIdPtr;
		ixNpeDlNpeState[npeId].validVersion = TRUE;
		ixNpeDlStats.successfulDownloads++;
	    }
	    else if ((status == IX_NPEDL_CRITICAL_NPE_ERR) ||
		     (status == IX_NPEDL_CRITICAL_MICROCODE_ERR))
	    {
		ixNpeDlNpeState[npeId].versionId = *versionIdPtr;
		ixNpeDlNpeState[npeId].validVersion = FALSE;
		ixNpeDlStats.criticalFailDownloads++;
	    }
	}/* condition: version located successfully in microcode image */
    }/* condition: parameter checks ok */
    
    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlVersionDownload : status = %d\n", status);
    return status;
}


/*
 * Function definition: ixNpeDlAvailableVersionsCountGet
 */
IX_STATUS 
ixNpeDlAvailableVersionsCountGet (UINT32 *numVersionsPtr)
{
    IX_STATUS status;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlAvailableVersionsCountGet\n");
    
    /* Check input parameters */
    if (numVersionsPtr == NULL)
    {
	status = IX_NPEDL_PARAM_ERR;
	IX_NPEDL_ERROR_REPORT ("ixNpeDlAvailableVersionsCountGet - "
			       "invalid parameter\n");
    }	
    else
    {
	/*
	 * Use ImageMgr module to get no. of versions listed in Image Header.
	 * If NULL is passed as versionListPtr parameter to following function,
	 * it will only fill number of versions into numVersionsPtr
	 */
	status = ixNpeDlImageMgrVersionListExtract (NULL, numVersionsPtr);
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Exiting ixNpeDlAvailableVersionsCountGet : "
		     "status = %d\n", status);
    return status;
}


/*
 * Function definition: ixNpeDlAvailableVersionsListGet
 */
IX_STATUS
ixNpeDlAvailableVersionsListGet (IxNpeDlVersionId *versionIdListPtr,
				 UINT32 *listSizePtr)
{
    IX_STATUS status;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlAvailableVersionsListGet\n");
    
    /* Check input parameters */
    if ((versionIdListPtr == NULL) || (listSizePtr == NULL))
    {
	status = IX_NPEDL_PARAM_ERR;
	IX_NPEDL_ERROR_REPORT ("ixNpeDlAvailableVersionsListGet - "
			       "invalid parameter\n");
    }	
    else
    {
	/* Call ImageMgr to get list of versions listed in Image Header */
	status = ixNpeDlImageMgrVersionListExtract (versionIdListPtr,
						    listSizePtr);
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Exiting ixNpeDlAvailableVersionsListGet : status = %d\n",
		     status);
    return status;
}


/*
 * Function definition: ixNpeDlLoadedVersionGet
 */
IX_STATUS
ixNpeDlLoadedVersionGet (IxNpeDlNpeId npeId,
			 IxNpeDlVersionId *versionIdPtr)
{
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlLoadedVersionGet\n");
    
    /* Check input parameters */
    if ((npeId >= IX_NPEDL_NPEID_MAX) || (npeId < 0) || (versionIdPtr == NULL))
    {
	status = IX_NPEDL_PARAM_ERR;
	IX_NPEDL_ERROR_REPORT ("ixNpeDlLoadedVersionGet - invalid parameter\n");
    }	
    else
    {
	if (ixNpeDlNpeState[npeId].validVersion)
	{
	    /* use npeId to get versionId from list of currently loaded 
	       versions */
	    *versionIdPtr = ixNpeDlNpeState[npeId].versionId;
	}
	else
	{
	    status = IX_FAIL;
	    IX_NPEDL_ERROR_REPORT ("ixNpeDlLoadedVersionGet - "
				   "NPE does not have a valid version\n");
	}
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Exiting ixNpeDlLoadedVersionGet : status = %d\n", 
		     status);
    return status;
}


/*
 * Function definition: ixNpeDlNpeStopAndReset
 */
IX_STATUS
ixNpeDlNpeStopAndReset (IxNpeDlNpeId npeId)
{
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlNpeStopAndReset\n");
    
    /* Check input parameters */
    if ((npeId >= IX_NPEDL_NPEID_MAX) || (npeId < 0))
    {
	status = IX_NPEDL_PARAM_ERR;
	IX_NPEDL_ERROR_REPORT ("ixNpeDlLoadedVersionGet - invalid parameter\n");
    }	
    else
    {
	/* call NpeMgr function to stop the NPE */
	status = ixNpeDlNpeMgrNpeStop (npeId);
	if (status == IX_SUCCESS)
	{
	    /* call NpeMgr function to reset the NPE */
	    status = ixNpeDlNpeMgrNpeReset (npeId);
	}
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlNpeStopAndReset : status = %d\n", status);
    return status;
}


/*
 * Function definition: ixNpeDlNpeExecutionStart
 */
IX_STATUS
ixNpeDlNpeExecutionStart (IxNpeDlNpeId npeId)
{
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlNpeExecutionStart\n");
    
    /* Check input parameters */
    if ((npeId >= IX_NPEDL_NPEID_MAX) || (npeId < 0))
    {
	status = IX_NPEDL_PARAM_ERR;
	IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeExecutionStart - invalid parameter\n");
    }	
    else
    {
	/* call NpeMgr function to start the NPE */
	status = ixNpeDlNpeMgrNpeStart (npeId);
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlNpeExecutionStart : status = %d\n",
		     status);
    return status;
}


/*
 * Function definition: ixNpeDlNpeExecutionStop
 */
IX_STATUS
ixNpeDlNpeExecutionStop (IxNpeDlNpeId npeId)
{
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlNpeExecutionStop\n");
    
    /* Check input parameters */
    if ((npeId >= IX_NPEDL_NPEID_MAX) || (npeId < 0))
    {
	status = IX_NPEDL_PARAM_ERR;
	IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeExecutionStop - invalid parameter\n");
    }	
    else
    {
	/* call NpeMgr function to stop the NPE */
	status = ixNpeDlNpeMgrNpeStop (npeId);
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlNpeExecutionStop : status = %d\n",
		     status);
    return status;
}


/*
 * Function definition: ixNpeDlStatsShow
 */
void
ixNpeDlStatsShow (void)
{
    printf ("\nixNpeDlStatsShow:\n");

    printf ("\tDownloads Attempted by user: %u\n",
	    ixNpeDlStats.attemptedDownloads);
    printf ("\tSuccessful Downloads: %u\n",
	    ixNpeDlStats.successfulDownloads);
    printf ("\tFailed Downloads (due to Critical Error): %u\n",
	    ixNpeDlStats.criticalFailDownloads);

    printf ("\n");

    ixNpeDlImageMgrStatsShow ();
    ixNpeDlNpeMgrStatsShow ();
}


/*
 * Function definition: ixNpeDlStatsReset
 */
void
ixNpeDlStatsReset (void)
{
    ixNpeDlStats.attemptedDownloads = 0;
    ixNpeDlStats.successfulDownloads = 0;
    ixNpeDlStats.criticalFailDownloads = 0;

    ixNpeDlImageMgrStatsReset ();
    ixNpeDlNpeMgrStatsReset ();
}
