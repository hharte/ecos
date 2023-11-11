/**
 * @file    IxQMgrInit.c
 *
 * @author Intel Corporation
 * @date    30-Oct-2001
 *
 * @brief:  Provided initialization of the QMgr component and its subcomponents.
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
 * System defined include files.
 */
#include <stdio.h>

/*
 * User defined include files.
 */
#include "IxQMgr.h"
#include "IxQMgrQCfg_p.h"
#include "IxQMgrDispatcher_p.h"
#include "IxQMgrLog_p.h"
#include "IxQMgrQAccess_p.h"
#include "IxQMgrDefines_p.h"

/*
 * Set to true if initialized
 * N.B. global so integration/unit tests can reinitialize
 */
BOOL qMgrIsInitialized = FALSE;

/*
 * Function definitions.
 */
IX_STATUS
ixQMgrInit (void)
{
    if (qMgrIsInitialized)
    {
	IX_QMGR_LOG0("ixQMgrInit: IxQMgr already initialised\n");
	return IX_FAIL;
    }

    /* Initialise the QCfg component */
    ixQMgrQCfgInit ();

    /* Initialise the Dispatcher component */
    ixQMgrDispatcherInit ();

    /* Initialise the Access component */
    ixQMgrQAccessInit ();

    /* Initialization complete */
    qMgrIsInitialized = TRUE;

    return IX_SUCCESS;
}

void
ixQMgrShow (void)
{
    IxQMgrQCfgStats *qCfgStats = NULL;
    IxQMgrDispatcherStats *dispatcherStats = NULL;
    int i;

    qCfgStats = ixQMgrQCfgStatsGet ();
    dispatcherStats = ixQMgrDispatcherStatsGet ();
    printf("Generic Stats........\n");
    printf("=====================\n");
    printf("Loop Run Count..........%u\n",dispatcherStats->loopRunCnt);
    printf("Watermark set count.....%d\n", qCfgStats->wmSetCnt);
    printf("=====================================\n");
    printf("Queue Specific Stats........\n");
    printf("============================\n");

    for (i=0; i<IX_QMGR_MAX_NUM_QUEUES; i++)
    {
	if (ixQMgrQIsConfigured(i))
	{
	    ixQMgrQShow(i);
	}
    }
}

IX_STATUS
ixQMgrQShow (IxQMgrQId qId)
{
    IxQMgrQCfgStats *qCfgStats = NULL;
    IxQMgrDispatcherStats *dispatcherStats = NULL; 
#if IX_QMGR_STATS_UPDATE_ENABLED
    IxQMgrQAccessStats *qAccessStats = NULL;
#endif

    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }
    
    dispatcherStats = ixQMgrDispatcherStatsGet ();
    qCfgStats = ixQMgrQCfgQStatsGet (qId);
#if IX_QMGR_STATS_UPDATE_ENABLED
    qAccessStats = ixQMgrQAccessStatsGet ();
#endif

    printf("QId %d\n", qId);
    printf("======\n");
    printf("  IxQMgrQCfg Stats\n");
    printf("    Name..................... \"%s\"\n", qCfgStats->qStats[qId].qName);
    printf("    Size in words............ %u\n", qCfgStats->qStats[qId].qSizeInWords);
    printf("    Entry size in words...... %u\n", qCfgStats->qStats[qId].qEntrySizeInWords);
    printf("    Nearly empty watermark... %u\n", qCfgStats->qStats[qId].ne);
    printf("    Nearly full watermark.... %u\n", qCfgStats->qStats[qId].nf);
    printf("    Number of full entries... %u\n", qCfgStats->qStats[qId].numEntries);
    printf("    Sram base address........ 0x%X\n", qCfgStats->qStats[qId].baseAddress);
    printf("    Read pointer............. 0x%X\n", qCfgStats->qStats[qId].readPtr);
    printf("    Write pointer............ 0x%X\n", qCfgStats->qStats[qId].writePtr);

    if (qCfgStats->qStats[qId].isConfigured)
    {
        printf("    Is configured..... TRUE\n");
    }
    else
    {
        printf("    Is configured..FALSE\n");
    }
#if IX_QMGR_STATS_UPDATE_ENABLED
    printf("  IxQMgrQAccess Stats\n");

    printf("    Read count..............%d\n", qAccessStats->qStats[qId].readCnt);
    printf("    Underflow count.........%d\n", qAccessStats->qStats[qId].underflowCnt);
    printf("    Write count.............%d\n", qAccessStats->qStats[qId].writeCnt);
    printf("    Overflow count..........%d\n", qAccessStats->qStats[qId].overflowCnt);
    printf("    Status get count........%d\n", qAccessStats->qStats[qId].statusGetCnt);
    printf("    Peek count..............%d\n", qAccessStats->qStats[qId].peekCnt);
    printf("    Poke count..............%d\n", qAccessStats->qStats[qId].pokeCnt);

    printf("  IxQMgrDispatcher Stats\n");
    printf("    Callback count................%d\n",
	  dispatcherStats->queueStats[qId].callbackCnt);
    printf("    Priority change count.........%d\n",
	  dispatcherStats->queueStats[qId].priorityChangeCnt);
    printf("    Interrupt no callback count...%d\n",
	  dispatcherStats->queueStats[qId].intNoCallbackCnt);
    printf("    Interrupt lost callback count...%d\n",
	  dispatcherStats->queueStats[qId].intLostCallbackCnt);
#endif

    return IX_SUCCESS;
}




