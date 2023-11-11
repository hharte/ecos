/**
 * @file IxNpeDlMacros_p.h
 *
 * @author Intel Corporation
 * @date 21 January 2002
 *
 * @brief This file contains the macros for the IxNpeDl component.
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
 * @defgroup IxNpeDlMacros_p IxNpeDlMacros_p
 *
 * @brief Macros for the IxNpeDl component.
 * 
 * @{
 */

#ifndef IXNPEDLMACROS_P_H
#define IXNPEDLMACROS_P_H


/*
 * Put the user defined include files required.
 */
#if(CPU==SIMSPARCSOLARIS)
/* To support IxNpeDl unit tests... */
#include <stdio.h>
#include "test/IxNpeDlTestReg.h"

#else   
#include "IxOsServices.h"

#endif


/*
 * Typedefs
 */

/**
 * @typedef IxNpeDlTraceTypes
 * @brief Enumeration defining IxNpeDl trace levels
 */
typedef enum
{
    IX_NPEDL_TRACE_OFF,     /**< no trace */
    IX_NPEDL_DEBUG,         /**< debug */
    IX_NPEDL_FN_ENTRY_EXIT  /**< function entry/exit */
} IxNpeDlTraceTypes;


/*
 * #defines and macros.
 */

/* Implementation of the following macros for use with IxNpeDl unit test code */
#if (CPU==SIMSPARCSOLARIS)


/**
 * @def IX_NPEDL_TRACE_LEVEL
 *
 * @brief IxNpeDl debug trace level
 */
#define IX_NPEDL_TRACE_LEVEL IX_NPEDL_FN_ENTRY_EXIT

/**
 * @def IX_NPEDL_ERROR_REPORT
 *
 * @brief Mechanism for reporting IxNpeDl software errors
 *
 * @param char* [in] STR - Error string to report
 *
 * This macro simply prints the error string passed.
 * Intended for use with IxNpeDl unit test code.
 *
 * @return none
 */
#define IX_NPEDL_ERROR_REPORT(STR) printf ("IxNpeDl ERROR: %s\n", (STR));

/**
 * @def IX_NPEDL_TRACE0
 *
 * @brief Mechanism for tracing debug for the IxNpeDl component, for no arguments
 *
 * @param unsigned [in] LEVEL - one of IxNpeDlTraceTypes enumerated values
 * @param char* [in] STR - Trace string
 *
 * This macro simply prints the trace string passed, if the level is supported. 
 * Intended for use with IxNpeDl unit test code.
 *
 * @return none
 */
#define IX_NPEDL_TRACE0(LEVEL, STR) \
{ \
    if (LEVEL <= IX_NPEDL_TRACE_LEVEL) \
    { \
        printf ("IxNpeDl TRACE: "); \
        printf ((STR)); \
        printf ("\n"); \
    } \
}

 /**
 * @def IX_NPEDL_TRACE1
 *
 * @brief Mechanism for tracing debug for the IxNpeDl component, with 1 argument
 *
 * @param unsigned [in] LEVEL - one of IxNpeDlTraceTypes enumerated values
 * @param char* [in] STR - Trace string
 * @param argType [in] ARG1 - Argument to trace
 *
 * This macro simply prints the trace string passed, if the level is supported.
 * Intended for use with IxNpeDl unit test code.
 *
 * @return none
 */
#define IX_NPEDL_TRACE1(LEVEL, STR, ARG1) \
{ \
    if (LEVEL <= IX_NPEDL_TRACE_LEVEL) \
    { \
        printf ("IxNpeDl TRACE: "); \
        printf (STR, ARG1); \
        printf ("\n"); \
    } \
}

/**
 * @def IX_NPEDL_TRACE2
 *
 * @brief Mechanism for tracing debug for the IxNpeDl component, with 2 arguments
 *
 * @param unsigned [in] LEVEL - one of IxNpeDlTraceTypes enumerated values
 * @param char* [in] STR - Trace string
 * @param argType [in] ARG1 - Argument to trace
 * @param argType [in] ARG2 - Argument to trace
 *
 * This macro simply prints the trace string passed, if the level is supported. 
 * Intended for use with IxNpeDl unit test code.
 *
 * @return none
 */
#define IX_NPEDL_TRACE2(LEVEL, STR, ARG1, ARG2) \
{ \
    if (LEVEL <= IX_NPEDL_TRACE_LEVEL) \
    { \
        printf ("IxNpeDl TRACE: "); \
        printf (STR, ARG1, ARG2); \
        printf ("\n"); \
    } \
}


/**
 * @def IX_NPEDL_REG_WRITE
 *
 * @brief Mechanism for writing to a memory-mapped register
 *
 * @param UINT32 [in] base   - Base memory address for this NPE's registers
 * @param UINT32 [in] offset - Offset from base memory address
 * @param UINT32 [in] value  - Value to write to register
 *
 * This macro calls a function from Unit Test code to write a register.  This
 * allows extra flexibility for unit testing of the IxNpeDl component.
 *
 * @return none
 */
#define IX_NPEDL_REG_WRITE(base, offset, value) \
{ \
    ixNpeDlTestRegWrite (base, offset, value); \
}


/**
 * @def IX_NPEDL_REG_READ
 *
 * @brief Mechanism for reading from a memory-mapped register
 *
 * @param UINT32 [in] base     - Base memory address for this NPE's registers
 * @param UINT32 [in] offset   - Offset from base memory address
 * @param UINT32 *[out] value  - Value read from register
 *
 * This macro calls a function from Unit Test code to read a register.  This
 * allows extra flexibility for unit testing of the IxNpeDl component.
 *
 * @return none
 */
#define IX_NPEDL_REG_READ(base, offset, value) \
{ \
    ixNpeDlTestRegRead (base, offset, value); \
}


/* Implementation of the following macros when integrated with IxOsServices */
#else  /* #if (CPU==SIMSPARCSOLARIS) */


/**
 * @def IX_NPEDL_TRACE_LEVEL
 *
 * @brief IxNpeDl debug trace level
 */
#define IX_NPEDL_TRACE_LEVEL IX_NPEDL_DEBUG


/**
 * @def IX_NPEDL_ERROR_REPORT
 *
 * @brief Mechanism for reporting IxNpeDl software errors
 *
 * @param char* [in] STR - Error string to report
 *
 * This macro is used to report IxNpeDl software errors.
 *
 * @return none
 */
#ifdef __ECOS
#define IX_NPEDL_ERROR_REPORT(STR)
#else
#define IX_NPEDL_ERROR_REPORT(STR) \
    ixOsServLog (LOG_ERROR, STR, 0, 0, 0, 0, 0, 0);
#endif


/**
 * @def IX_NPEDL_TRACE0
 *
 * @brief Mechanism for tracing debug for the IxNpeDl component, for no arguments
 *
 * @param unsigned [in] LEVEL - one of IxNpeDlTraceTypes enumerated values
 * @param char* [in] STR - Trace string
 *
 * This macro simply prints the trace string passed, if the level is supported.
 *
 * @return none
 */
#ifdef __ECOS
#define IX_NPEDL_TRACE0(LEVEL, STR)
#else
#define IX_NPEDL_TRACE0(LEVEL, STR) \
{ \
    if (LEVEL <= IX_NPEDL_TRACE_LEVEL) \
    { \
        if (LEVEL == IX_NPEDL_FN_ENTRY_EXIT) \
        { \
            ixOsServLog (LOG_DEBUG3, STR, 0, 0, 0, 0, 0, 0); \
        } \
        else if (LEVEL == IX_NPEDL_DEBUG) \
        { \
            ixOsServLog (LOG_MESSAGE, STR, 0, 0, 0, 0, 0, 0); \
        } \
    } \
}
#endif

/**
 * @def IX_NPEDL_TRACE1
 *
 * @brief Mechanism for tracing debug for the IxNpeDl component, with 1 argument
 *
 * @param unsigned [in] LEVEL - one of IxNpeDlTraceTypes enumerated values
 * @param char* [in] STR - Trace string
 * @param argType [in] ARG1 - Argument to trace
 *
 * This macro simply prints the trace string passed, if the level is supported. 
 *
 * @return none
 */
#ifdef __ECOS
#define IX_NPEDL_TRACE1(LEVEL, STR, ARG1)
#else
#define IX_NPEDL_TRACE1(LEVEL, STR, ARG1) \
{ \
    if (LEVEL <= IX_NPEDL_TRACE_LEVEL) \
    { \
        if (LEVEL == IX_NPEDL_FN_ENTRY_EXIT) \
        { \
            ixOsServLog (LOG_DEBUG3, STR, ARG1, 0, 0, 0, 0, 0); \
        } \
        else if (LEVEL == IX_NPEDL_DEBUG) \
        { \
            ixOsServLog (LOG_MESSAGE, STR, ARG1, 0, 0, 0, 0, 0); \
        } \
    } \
}
#endif

/**
 * @def IX_NPEDL_TRACE2
 *
 * @brief Mechanism for tracing debug for the IxNpeDl component, with 2 arguments
 *
 * @param unsigned [in] LEVEL - one of IxNpeDlTraceTypes enumerated values
 * @param char* [in] STR - Trace string
 * @param argType [in] ARG1 - Argument to trace
 * @param argType [in] ARG2 - Argument to trace
 *
 * This macro simply prints the trace string passed, if the level is supported. 
 *
 * @return none
 */
#ifdef __ECOS
#define IX_NPEDL_TRACE2(LEVEL, STR, ARG1, ARG2)
#else
#define IX_NPEDL_TRACE2(LEVEL, STR, ARG1, ARG2) \
{ \
    if (LEVEL <= IX_NPEDL_TRACE_LEVEL) \
    { \
        if (LEVEL == IX_NPEDL_FN_ENTRY_EXIT) \
        { \
            ixOsServLog (LOG_DEBUG3, STR, ARG1, ARG2, 0, 0, 0, 0); \
        } \
        else if (LEVEL == IX_NPEDL_DEBUG) \
        { \
            ixOsServLog (LOG_MESSAGE, STR, ARG1, ARG2, 0, 0, 0, 0); \
        } \
    } \
}
#endif

/**
 * @def IX_NPEDL_REG_WRITE
 *
 * @brief Mechanism for writing to a memory-mapped register
 *
 * @param UINT32 [in] base   - Base memory address for this NPE's registers
 * @param UINT32 [in] offset - Offset from base memory address
 * @param UINT32 [in] value  - Value to write to register
 *
 * This macro forms the address of the register from base address + offset, and 
 * dereferences that address to write the contents of the register.
 *
 * @return none
 */
#define IX_NPEDL_REG_WRITE(base, offset, value) \
    (*(volatile UINT32*)((base + offset)) = (value))



/**
 * @def IX_NPEDL_REG_READ
 *
 * @brief Mechanism for reading from a memory-mapped register
 *
 * @param UINT32 [in] base    - Base memory address for this NPE's registers
 * @param UINT32 [in] offset  - Offset from base memory address
 * @param UINT32 *[out] value  - Value read from register
 *
 * This macro forms the address of the register from base address + offset, and 
 * dereferences that address to read the register contents.
 *
 * @return none
 */
#define IX_NPEDL_REG_READ(base, offset, value) \
    (*(value) = (*(volatile UINT32*)((base + offset))))

#endif  /* #if (CPU==SIMSPARCSOLARIS) */

#endif /* IXNPEDLMACROS_P_H */

/**
 * @} defgroup IxNpeDlMacros_p
 */
