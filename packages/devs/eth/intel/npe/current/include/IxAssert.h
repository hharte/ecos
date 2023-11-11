/**
 * @file IxAssert.h
 *
 * @date 21-MAR-2002
 *
 * @brief This file contains assert and ensure macros used by the IXP4xx software
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
 * @defgroup IxAssert IXP4xx Assertion Macros (IxAssert) API
 *
 * @brief Assertion support
 *
 * @{
 */

#ifndef IXASSERT_H

#ifndef __doxygen_HIDE

#define IXASSERT_H

#endif /* __doxygen_HIDE */

/*
 * Put the system defined include files required
 */
#include <assert.h>
#include <stdio.h>

/**
 * @brief Assert macro, assert the condition is true. This
 *        will not be compiled out.
 *        N.B. will result in a system crash if it is false.
 * <ToDo> implement, assert() is called as an interim
 * <ToDo> Provide function to override behaviour of assert
 */
#define IX_ASSERT(c) assert(c)

/**
 * @brief Ensure macro, ensure the condition is true.
 *        This will be conditionally compiled out and
 *        may be used for unit/integration test purposes.
 * <ToDo> implement
 */
#ifndef IX_ENSURE_OFF
#define IX_ENSURE(c, str) if (!c) printf(str "\n")
#else
#define IX_ENSURE(c, str)
#endif

#endif /* IXASSERT_H */

/**
 * @} addtogroup IxAssert
 */



