/**
 * @file IxTypes.h
 *
 * @date 28-NOV-2001

 * @brief This file contains basic types used by the IXP4xx software
 *
 * Design Notes:
 *    This file shall only include fundamental types and definitions to be
 *    shared by all the IXP4xx components.
 *    Please DO NOT add component-specific types here.
 *
 * -- Intel Copyright Notice --
 *
 * @par
 * INTEL CONFIDENTIAL
 *
 * @par
 * Copyright 2001-2002 Intel Corporation All Rights Reserved.
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
 * @defgroup IxTypes IXP4xx Types (IxTypes) 
 *
 * @brief Basic data types used by the IXP4xx project
 *
 * @{
 */

#ifndef IxTypes_H

#ifndef __doxygen_HIDE

#define IxTypes_H

#endif /* __doxygen_HIDE */

#ifdef __vxworks

#include <vxWorks.h>

#endif

typedef void (*IxVoidFnPtr)   ();
/* Basic types - wrapped as not to redefine them if VxWorks does */



#ifndef __INCvxTypesOldh
/* These are #defines provided by the vxWorks headers.
   TODO: all uses of many of these should be removed from the code.
*/


/* from vxTypesOld.h */
typedef int (*FUNCPTR)();
typedef int STATUS;
typedef int BOOL;
typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned int   UINT;
typedef unsigned long  ULONG;
typedef char  INT8;
typedef short INT16;
typedef int   INT32;
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;
typedef void VOID;

/* from vxWorks.h */
#define OK 0
#define ERROR (-1)
#ifndef NELEMENTS
#define NELEMENTS(array) (sizeof(array) / sizeof((array)[0]))
#endif

/* from vxWorks net/mbuf.h */
/* Default size for clusters */
#define MCLBYTES 2048

/* from pppd.h (!) */
#ifndef BZERO
#define BZERO(buf) memset(&(buf), 0, sizeof(buf))
#endif

#endif /* __INCvxTypesOldh */

typedef volatile UINT32 VUINT32;

typedef volatile INT32 VINT32;

/* IXP4xx-specific types */

typedef UINT32 IX_STATUS;

/**
 * Standard return values
 */
#define IX_SUCCESS 0
#define IX_FAIL 1

/**
 * Boolean TRUE and FALSE definitions
 */
#if !defined(TRUE) || (TRUE != 1)
#define TRUE 1
#endif /* TRUE */

#if !defined(FALSE) || (FALSE != 0)
#define FALSE 0
#endif /* FALSE */

/**
 * definition of NULL
 */
#ifndef NULL
#define NULL 0L
#endif /* NULL */

#ifndef PRIVATE
#ifdef NDEBUG
#define PRIVATE static
#else
#define PRIVATE /* nothing */
#endif /* NDEBUG */
#endif /* PRIVATE */

#ifndef PUBLIC
#define PUBLIC
#endif /* PUBLIC */

#endif /* IxTypes_H */

/**
 * @} addtogroup IxTypes
 */
