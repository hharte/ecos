/**
 * @file IxEthDBLog_p.h
 *
 * @brief definitions of log macros and log configuration
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

#include <stdio.h>
#include <IxOsServices.h>

#ifdef __ECOS
#ifndef printf
#define printf diag_printf
#endif
#endif

/***************************************************
 *                 Globals                         *
 ***************************************************/
#ifndef __ECOS
#define HAS_ERROR_LOG
#define HAS_ERROR_IRQ_LOG
#endif

/***************************************************
 *              Log Configuration                  *
 ***************************************************/

#undef HAS_EVENTS_TRACE
#undef HAS_EVENTS_VERBOSE_TRACE

#undef HAS_SUPPORT_TRACE
#undef HAS_SUPPORT_VERBOSE_TRACE

#undef HAS_NPE_TRACE
#undef HAS_NPE_VERBOSE_TRACE
#undef HAS_DUMP_NPE_TREE

#undef HAS_UPDATE_TRACE
#undef HAS_UPDATE_VERBOSE_TRACE


/***************************************************
 *              Log Macros                         *
 ***************************************************/

/************** Globals ******************/

#ifdef HAS_ERROR_LOG

    #define ERROR_LOG printf

#else

    #define ERROR_LOG if (0) printf

#endif

#ifdef HAS_ERROR_IRQ_LOG

    #define ERROR_IRQ_LOG(format, arg1, arg2, arg3, arg4, arg5, arg6) ixOsServLog(LOG_ERROR, format, arg1, arg2, arg3, arg4, arg5, arg6)

#else

    #define ERROR_IRQ_LOG(format, arg1, arg2, arg3, arg4, arg5, arg6) if (0) ixOsServLog(LOG_ERROR, format, arg1, arg2, arg3, arg4, arg5, arg6)

#endif

/************** Events *******************/

#ifdef HAS_EVENTS_TRACE

    #define IX_ETH_DB_EVENTS_TRACE     printf
    #define IX_ETH_DB_IRQ_EVENTS_TRACE(format, arg1, arg2, arg3, arg4, arg5, arg6) ixOsServLog(LOG_MESSAGE, format, arg1, arg2, arg3, arg4, arg5, arg6)

    #ifdef HAS_EVENTS_VERBOSE_TRACE

        #define IX_ETH_DB_EVENTS_VERBOSE_TRACE printf
    
    #else

        #define IX_ETH_DB_EVENTS_VERBOSE_TRACE if (0) printf

    #endif /* HAS_EVENTS_VERBOSE_TRACE */

#else

    #define IX_ETH_DB_EVENTS_TRACE         if (0) printf
    #define IX_ETH_DB_EVENTS_VERBOSE_TRACE if (0) printf
    #define IX_ETH_DB_IRQ_EVENTS_TRACE(format, arg1, arg2, arg3, arg4, arg5, arg6) if (0) ixOsServLog(LOG_MESSAGE, format, arg1, arg2, arg3, arg4, arg5, arg6)

#endif /* HAS_EVENTS_TRACE */

/************** Support *******************/

#ifdef HAS_SUPPORT_TRACE

    #define IX_ETH_DB_SUPPORT_TRACE     printf
    #define IX_ETH_DB_SUPPORT_IRQ_TRACE(format, arg1, arg2, arg3, arg4, arg5, arg6) ixOsServLog(LOG_MESSAGE, format, arg1, arg2, arg3, arg4, arg5, arg6)

    #ifdef HAS_SUPPORT_VERBOSE_TRACE

        #define IX_ETH_DB_SUPPORT_VERBOSE_TRACE printf

    #else

        #define IX_ETH_DB_SUPPORT_VERBOSE_TRACE if (0) printf

    #endif /* HAS_SUPPORT_VERBOSE_TRACE */

#else

    #define IX_ETH_DB_SUPPORT_TRACE         if (0) printf
    #define IX_ETH_DB_SUPPORT_VERBOSE_TRACE if (0) printf
    #define IX_ETH_DB_SUPPORT_IRQ_TRACE(format, arg1, arg2, arg3, arg4, arg5, arg6) if (0) ixOsServLog(LOG_MESSAGE, format, arg1, arg2, arg3, arg4, arg5, arg6)

#endif /* HAS_SUPPORT_TRACE */

/************** NPE Adaptor *******************/

#ifdef HAS_NPE_TRACE

    #define IX_ETH_DB_NPE_TRACE     printf
    #define IX_ETH_DB_NPE_IRQ_TRACE(format, arg1, arg2, arg3, arg4, arg5, arg6) ixOsServLog(LOG_MESSAGE, format, arg1, arg2, arg3, arg4, arg5, arg6)

    #ifdef HAS_NPE_VERBOSE_TRACE

        #define IX_ETH_DB_NPE_VERBOSE_TRACE printf

    #else

        #define IX_ETH_DB_NPE_VERBOSE_TRACE if (0) printf

    #endif /* HAS_NPE_VERBOSE_TRACE */

#else

    #define IX_ETH_DB_NPE_TRACE         if (0) printf
    #define IX_ETH_DB_NPE_VERBOSE_TRACE if (0) printf
    #define IX_ETH_DB_NPE_IRQ_TRACE(format, arg1, arg2, arg3, arg4, arg5, arg6) if (0) ixOsServLog(LOG_MESSAGE, format, arg1, arg2, arg3, arg4, arg5, arg6)

#endif /* HAS_NPE_TRACE */

#ifdef HAS_DUMP_NPE_TREE

#define IX_ETH_DB_NPE_DUMP_ELT(eltBaseAddress, eltSize) ixEthELTDumpTree(eltBaseAddress, eltSize)

#else

#define IX_ETH_DB_NPE_DUMP_ELT(eltBaseAddress, eltSize) /* nothing */

#endif /* HAS_DUMP_NPE_TREE */

/************** Port Update *******************/

#ifdef HAS_UPDATE_TRACE

    #define IX_ETH_DB_UPDATE_TRACE printf

    #ifdef HAS_UPDATE_VERBOSE_TRACE

        #define IX_ETH_DB_UPDATE_VERBOSE_TRACE printf

    #else

        #define IX_ETH_DB_UPDATE_VERBOSE_TRACE if (0) printf

    #endif

#else /* HAS_UPDATE_VERBOSE_TRACE */

    #define IX_ETH_DB_UPDATE_TRACE         if (0) printf
    #define IX_ETH_DB_UPDATE_VERBOSE_TRACE if (0) printf

#endif /* HAS_UPDATE_TRACE */
