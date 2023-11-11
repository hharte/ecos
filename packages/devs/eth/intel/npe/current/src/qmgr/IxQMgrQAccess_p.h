/**
 * @file    IxQMgrQAccess_p.h
 *
 * @author Intel Corporation
 * @date    30-Oct-2001
 *
 * @brief   QAccess private header file
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

#ifndef IXQMGRQACCESS_P_H
#define IXQMGRQACCESS_P_H

/*
 * User defined header files
 */
#include "IxQMgr.h"

/*
 * Queue access statistics
 */
typedef struct
{
    struct
    {
	unsigned readCnt;
	unsigned underflowCnt;
	unsigned writeCnt;
	unsigned overflowCnt;
	unsigned statusGetCnt;
	unsigned peekCnt;
	unsigned pokeCnt;
    } qStats[IX_QMGR_MAX_NUM_QUEUES];

} IxQMgrQAccessStats;

/* 
 * Initialise the Queue Access component
 */
void
ixQMgrQAccessInit (void);

/*
 * Get the Queue Access component statistics
 */
IxQMgrQAccessStats*
ixQMgrQAccessStatsGet (void);

#endif/*IXQMGRQACCESS_P_H*/
