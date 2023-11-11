/**
 * @file IxNpeMhMacros_p.h
 *
 * @author Intel Corporation
 * @date 21 Jan 2002
 *
 * @brief This file contains the macros for the IxNpeMh component.
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
 * @defgroup IxNpeMhMacros_p IxNpeMhMacros_p
 *
 * @brief Macros for the IxNpeMh component.
 * 
 * @{
 */

#ifndef IXNPEMHMACROS_P_H
#define IXNPEMHMACROS_P_H

/* if we are running on Simulator on Sparc Solaris, i.e. testing */
#if CPU==SIMSPARCSOLARIS
#undef NDEBUG
#endif /* #if CPU==SIMSPARCSOLARIS */

#include "IxOsServices.h"

/*
 * #defines for function return types, etc.
 */

#define IX_NPEMH_SHOW_TEXT_WIDTH (40) /**< text width for stats display */
#define IX_NPEMH_SHOW_STAT_WIDTH (10) /**< stat width for stats display */

/**
 * @def IX_NPEMH_SHOW
 *
 * @brief Macro for displaying a stat preceded by a textual description.
 */

#define IX_NPEMH_SHOW(TEXT, STAT) \
    printf ("%-*s: %*d\n", \
	    IX_NPEMH_SHOW_TEXT_WIDTH, TEXT, \
	    IX_NPEMH_SHOW_STAT_WIDTH, STAT)

/*
 * Prototypes for interface functions.
 */

/**
 * @typedef IxNpeMhTraceTypes
 *
 * @brief Enumeration defining IxNpeMh trace levels
 */

typedef enum
{
    IX_NPEMH_TRACE_OFF     = LOG_NONE,    /**< no trace */
    IX_NPEMH_WARNING       = LOG_WARNING, /**< warning */
    IX_NPEMH_DEBUG         = LOG_MESSAGE, /**< debug */
    IX_NPEMH_FN_ENTRY_EXIT = LOG_DEBUG3   /**< function entry/exit */
} IxNpeMhTraceTypes;

#if (CPU==SIMSPARCSOLARIS)
#define IX_NPEMH_TRACE_LEVEL (IX_NPEMH_FN_ENTRY_EXIT) /**< trace level */
#else
#define IX_NPEMH_TRACE_LEVEL (IX_NPEMH_TRACE_OFF) /**< trace level */
#endif

/**
 * @def IX_NPEMH_TRACE0
 *
 * @brief Trace macro taking 0 arguments.
 */

#define IX_NPEMH_TRACE0(LEVEL, STR) \
    IX_NPEMH_TRACE6(LEVEL, STR, 0, 0, 0, 0, 0, 0)

/**
 * @def IX_NPEMH_TRACE1
 *
 * @brief Trace macro taking 1 argument.
 */

#define IX_NPEMH_TRACE1(LEVEL, STR, ARG1) \
    IX_NPEMH_TRACE6(LEVEL, STR, ARG1, 0, 0, 0, 0, 0)

/**
 * @def IX_NPEMH_TRACE2
 *
 * @brief Trace macro taking 2 arguments.
 */

#define IX_NPEMH_TRACE2(LEVEL, STR, ARG1, ARG2) \
    IX_NPEMH_TRACE6(LEVEL, STR, ARG1, ARG2, 0, 0, 0, 0)

/**
 * @def IX_NPEMH_TRACE3
 *
 * @brief Trace macro taking 3 arguments.
 */

#define IX_NPEMH_TRACE3(LEVEL, STR, ARG1, ARG2, ARG3) \
    IX_NPEMH_TRACE6(LEVEL, STR, ARG1, ARG2, ARG3, 0, 0, 0)

/**
 * @def IX_NPEMH_TRACE4
 *
 * @brief Trace macro taking 4 arguments.
 */

#define IX_NPEMH_TRACE4(LEVEL, STR, ARG1, ARG2, ARG3, ARG4) \
    IX_NPEMH_TRACE6(LEVEL, STR, ARG1, ARG2, ARG3, ARG4, 0, 0)

/**
 * @def IX_NPEMH_TRACE5
 *
 * @brief Trace macro taking 5 arguments.
 */

#define IX_NPEMH_TRACE5(LEVEL, STR, ARG1, ARG2, ARG3, ARG4, ARG5) \
    IX_NPEMH_TRACE6(LEVEL, STR, ARG1, ARG2, ARG3, ARG4, ARG5, 0)

/**
 * @def IX_NPEMH_TRACE6
 *
 * @brief Trace macro taking 6 arguments.
 */

#define IX_NPEMH_TRACE6(LEVEL, STR, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) \
{ \
    if (LEVEL <= IX_NPEMH_TRACE_LEVEL) \
    { \
        (void) ixOsServLog (LEVEL, (STR), \
                            (int)(ARG1), (int)(ARG2), (int)(ARG3), \
                            (int)(ARG4), (int)(ARG5), (int)(ARG6)); \
    } \
}

/**
 * @def IX_NPEMH_ERROR_REPORT
 *
 * @brief Error reporting facility.
 */

#define IX_NPEMH_ERROR_REPORT(STR) \
{ \
    (void) ixOsServLog (LOG_ERROR, (STR), 0, 0, 0, 0, 0, 0); \
}

/* if we are running on XScale, i.e. real environment */
#if CPU==XSCALE

/**
 * @def IX_NPEMH_REGISTER_READ
 *
 * @brief This macro reads a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_READ(registerAddress, value) \
{ \
    *value = *registerAddress; \
}

/**
 * @def IX_NPEMH_REGISTER_READ_BITS
 *
 * @brief This macro partially reads a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_READ_BITS(registerAddress, value, mask) \
{ \
    *value = (*registerAddress & mask); \
}

/**
 * @def IX_NPEMH_REGISTER_WRITE
 *
 * @brief This macro writes a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_WRITE(registerAddress, value) \
{ \
    *registerAddress = value; \
}

/**
 * @def IX_NPEMH_REGISTER_WRITE_BITS
 *
 * @brief This macro partially writes a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_WRITE_BITS(registerAddress, value, mask) \
{ \
    UINT32 orig = *registerAddress; \
    orig &= (~mask); \
    orig |= (value & mask); \
    *registerAddress = orig; \
}

#endif /* #if CPU==XSCALE */

/* if we are running on Simulator on Sparc Solaris, i.e. testing */
#if CPU==SIMSPARCSOLARIS

#include "IxNpeMhTestRegister.h"

/**
 * @def IX_NPEMH_REGISTER_READ
 *
 * @brief This macro reads a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_READ(registerAddress, value) \
{ \
    ixNpeMhTestRegisterRead (registerAddress, value); \
}

/**
 * @def IX_NPEMH_REGISTER_READ_BITS
 *
 * @brief This macro partially reads a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_READ_BITS(registerAddress, value, mask) \
{ \
    ixNpeMhTestRegisterReadBits (registerAddress, value, mask); \
}

/**
 * @def IX_NPEMH_REGISTER_WRITE
 *
 * @brief This macro writes a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_WRITE(registerAddress, value) \
{ \
    ixNpeMhTestRegisterWrite (registerAddress, value); \
}

/**
 * @def IX_NPEMH_REGISTER_WRITE_BITS
 *
 * @brief This macro partially writes a memory-mapped register.
 */

#define IX_NPEMH_REGISTER_WRITE_BITS(registerAddress, value, mask) \
{ \
    ixNpeMhTestRegisterWriteBits (registerAddress, value, mask); \
}

#endif /* #if CPU==SIMSPARCSOLARIS */

#endif /* IXNPEMHMACROS_P_H */

/**
 * @} defgroup IxNpeMhMacros_p
 */
