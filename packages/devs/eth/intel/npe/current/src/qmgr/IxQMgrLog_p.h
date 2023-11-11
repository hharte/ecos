/**
 * @file    IxQMgrLog_p.h
 *
 * @author Intel Corporation
 * @date    07-Feb-2002
 *
 * @brief   This file contains the internal functions for config
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

#ifndef IXQMGRLOG_P_H
#define IXQMGRLOG_P_H

/*
 * User defined header files
 */
#include "IxOsServices.h"

/*
 * Macros
 */
/*
 * ToDo, Replace LOG6 with a string log if IxOsServices is changed
 */
#ifdef __ECOS
#define ixOsServLog(a, b, c, d, e, f, g, h)
#endif

#define IX_QMGR_LOG0(string) do\
{\
    ixOsServLog(LOG_USER, string, 0, 0, 0, 0, 0, 0);\
}while(0);

#define IX_QMGR_LOG1(string, arg1) do\
{\
    ixOsServLog(LOG_USER, string, arg1, 0, 0, 0, 0, 0);\
}while(0);

#define IX_QMGR_LOG2(string, arg1, arg2) do\
{\
    ixOsServLog(LOG_USER, string, arg1, arg2, 0, 0, 0, 0);\
}while(0);

#define IX_QMGR_LOG3(string, arg1, arg2, arg3) do\
{\
    ixOsServLog(LOG_USER, string, arg1, arg2, arg3, 0, 0, 0);\
}while(0);

#define IX_QMGR_LOG6(string, arg1, arg2, arg3, arg4, arg5, arg6) do\
{\
    ixOsServLog(LOG_USER, string, arg1, arg2, arg3, arg4, arg5, arg6); \
}while(0);

#define IX_QMGR_LOG_WARNING0(string) do\
{\
    ixOsServLog(LOG_WARNING, string, 0, 0, 0, 0, 0, 0);\
}while(0);

#define IX_QMGR_LOG_WARNING1(string, arg1) do\
{\
    ixOsServLog(LOG_WARNING, string, arg1, 0, 0, 0, 0, 0);\
}while(0);

#define IX_QMGR_LOG_WARNING2(string, arg1, arg2) do\
{\
    ixOsServLog(LOG_WARNING, string, arg1, arg2, 0, 0, 0, 0);\
}while(0);

#define IX_QMGR_LOG_ERROR0(string) do\
{\
    ixOsServLog(LOG_ERROR, string, 0, 0, 0, 0, 0, 0);\
}while(0);

#define IX_QMGR_LOG_ERROR1(string, arg1) do\
{\
    ixOsServLog(LOG_ERROR, string, arg1, 0, 0, 0, 0, 0);\
}while(0);

#define IX_QMGR_LOG_ERROR2(string, arg1, arg2) do\
{\
    ixOsServLog(LOG_ERROR, string, arg1, arg2, 0, 0, 0, 0);\
}while(0);

#define IX_QMGR_LOG_ERROR3(string, arg1, arg2, arg3) do\
{\
    ixOsServLog(LOG_ERROR, string, arg1, arg2, arg3, 0, 0, 0);\
}while(0);
#endif /* IX_QMGRLOG_P_H */




