/**
 * ============================================================================
 * = COPYRIGHT
 * -- Intel Copyright Notice --
 * INTEL CONFIDENTIAL
 *
 * Copyright 2002 Intel Corporation All Rights Reserved.
 *
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
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise.  Any license under
 * such intellectual property rights must be express and approved by
 * Intel in writing.
 *
 * For further details, please see the file README.TXT distributed with
 * this software.
 * -- End Intel Copyright Notice --
 *
 * = PRODUCT
 *      Intel(r) IXA SDK for the IXP2X00 Network Processor
 *
 * = FILENAME
 *      ix_error.h
 *
 * = DESCRIPTION
 *      This file will describe the basic error type and support functions that
 *      will be used by the IXA SDK Framework API.
 *
 * = AUTHOR
 *      Intel Corporation
 *
 * = CHANGE HISTORY
 *      4/22/2002 4:19:03 PM - creation time 
 * ============================================================================
 */

#if !defined(__IX_ERROR_H__)
#define __IX_ERROR_H__

#include "ix_types.h"
#include "ix_symbols.h"
#include "ix_macros.h"
#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */


/**
 * TYPENAME: ix_error
 * 
 * DESCRIPTION: This type represents an error token. It is mapped as 32 bit unsigned
 *          integer and several pieces of information will be packed in this token as
 *          follows:
 *          Bits    Size    Field       Description
 *      -----------------------------------------------------------------
 *           0:15   16      Error code  This field describes the error code
 *          16:23   8       Error group This field describes the error group
 *          24:31   8       Error level This field describes the error level
 *
 *          IXA SDK errors belong to different error groups. 
 *          This feature permits the reuse of same error code 
 *          numbers for different modules (error groups).  
 *          A module can define the error codes independently 
 *          from other modules, as long as it uses different 
 *          error group.  However, some synchronization should 
 *          be done in choosing the error groups. The error 
 *          level describes the severity of an error. Based 
 *          on this level, the programmer can take different 
 *          actions to handle the error. Macros are provided 
 *          to get and set the different bit fields of an 
 *          ix_error token.
 *
 */
typedef ix_uint32   ix_error;

/**
 * DESCRIPTION: This symbol defines an error token corresponding to
 *          successful completion of an operation.
 */
#define IX_ERROR_SUCCESS    ((ix_error)0)


/**
 * TYPENAME: ix_error_group
 * 
 * DESCRIPTION: This enumeration describes the existing error groups in the system. 
 *          New error groups should be added all the times at the end. Developers can
 *          use IX_ERROR_GROUP_LAST value to start assigning new error group numbers.
 *
 */
typedef enum ix_e_error_group
{
    IX_ERROR_GROUP_FIRST = 0,
    IX_ERROR_GROUP_RESOURCE_MANAGER = IX_ERROR_GROUP_FIRST, /* Resource Manager error group */
    IX_ERROR_GROUP_OSSL, /* OSSL API error group */
    IX_ERROR_GROUP_CC_INFRASTRUCTURE, /* Core component infrastructure API error group */
    IX_ERROR_GROUP_LAST
} ix_error_group;



/**
 * TYPENAME: ix_error_level
 * 
 * DESCRIPTION: This enumeration designates the severity level of an error, 
 *          zero repreesenting no error at all, and higher numbers
 *          representing more and more severe errors. Severity roughly means
 *          how much of the system has become corrupted and is likely to  be no
 *          longer functional after the error occurs.
 */
typedef enum ix_e_error_level
{
    IX_ERROR_LEVEL_FIRST = 0,

    /** 
     * No error reported.
     *
     * This error level corresponds to returns from functions that
     * successfully accomplished their given tasks. This error level
     * will mean that no error occurred. It should not be used in any
     * valid  error token bacause it corresponds to IX_ERROR_SUCCESS.
     */
    IX_ERROR_LEVEL_NONE = IX_ERROR_LEVEL_FIRST,

    /** 
     * Warning level: Recoverable condition.
     *
     * This error level indicates that the requested task could not be
     * performed, but there are no lasting effects within the system
     * from that failure. The caller can recover from this condition
     * by simply continuing to execute as if the call had not been
     * made. Functions returning WARNING level messages should clearly
     * document the procedures for recovery.
     *
     * During debugging, WARNING messages should be logged for later
     * analysis as they may indicate the presence of a bug. This step
     * can probably be skipped in Production mode systems.
     */
    IX_ERROR_LEVEL_WARNING,

    /** 
     * Error level: Unrecoverable Local Condition.
     *
     * This error level indicates that the requested task could not be
     * performed, and that one or more of the specific data items
     * involved in the call has been permanently altered, so that in
     * general recovery may require "leaking" a corrupted data item
     * or shutting down and restarting a service. Functions returning
     * ERROR level messages should clearly document recovery methods
     * for keeping the system running, and what storage or
     * functionality will be lost.
     *
     * During debugging, ERROR messages should be immediately
     * analyzed , along with any prior WARNING messages. Production
     * mode systems should log all ERROR messages, even when we
     * expect to recover from them.
     */
    IX_ERROR_LEVEL_LOCAL,

    /** 
     * Error level: Unrecoverable Remote Condition.
     *
     * This error level indicates that there is an error in the module 
     * we are trying to communicate with. That will have no impact on the
     * local module but the functionality of the system as a whole will be
     * most likely affected.
     * This error level might be used for the case the libraries are used
     * in conjunction with a Foreign Model running on transactor.
     */
    IX_ERROR_LEVEL_REMOTE,

    /** 
     * Error level: Unrecoverable Global Condition.
     * 
     * This error level indicates that a function has detected a
     * condition that indicates that the system as a whole has become
     * compromised, and that multiple processing elements or data
     * modules are likely to experience a cascade failure.
     * 
     * During debugging, this level of error message should cause the
     * system to freeze and drop into a debugger where the problem
     * can be analyzed by an engineer. In Production deployment,
     * huge effort should be made to log these errors and as much
     * supporting information as possible where the logs can be
     * analyzed by a responsible human.
     */
    IX_ERROR_LEVEL_GLOBAL,

    /** 
     * Error level: Totally Impossible Condition.
     * 
     * This error level indicates that a function has detected a
     * condition that indicates that a primary assumption in a module
     * design has been proven impossible, and that in general the
     * system would be doing arbitrarily bad things.
     * 
     * During debugging, PANIC messages should cause the system to
     * freeze and drop into a debugger state where the problem can be
     * analyzed by an engineer.
     * 
     * In a Production deployed system, PANIC messages should cause
     * an immediate attention.
     * 
     * PANIC messages should be used very, very sparingly.
     */
    IX_ERROR_LEVEL_PANIC,

    IX_ERROR_LEVEL_LAST
} ix_error_level;



/**
 * MACRO NAME: IX_ERROR_GET_CODE
 *
 * DESCRIPTION: This macro retrieves the error code field from an error token.
 *
 * @Param:  - IN arg_Error -  error token of type ix_error.
 *
 * @Return: Returns an ix_uint32 value representing the error code for 
 *          the error token.
 */
#define IX_ERROR_GET_CODE( \
                            arg_Error \
                         ) \
                         IX_GET_BIT_FIELD32(arg_Error, 0U, 15U)

/**
 * MACRO NAME: IX_ERROR_SET_CODE
 *
 * DESCRIPTION: This macro sets the error code field for an error token.
 *
 * @Param:  - IN arg_Error -  error token of type ix_error.
 * @Param:  - IN arg_ErrorCode - this is the new error code for the error token.
 *
 * @Return: Returns the new value of the error token.
 */
#define IX_ERROR_SET_CODE( \
                            arg_Error, \
                            arg_ErrorCode \
                         ) \
                         (ix_error)IX_SET_BIT_FIELD32(arg_Error, arg_ErrorCode, 0U, 15U)
                

/**
 * MACRO NAME: IX_ERROR_GET_GROUP
 *
 * DESCRIPTION: This macro retrieves the error group field from an error token.
 *
 * @Param:  - IN arg_Error - error token of  type ix_error.
 *
 * @Return: Returns an ix_uint32 value representing the error group for 
 *          the error token.
 */
#define IX_ERROR_GET_GROUP( \
                            arg_Error \
                          ) \
                          IX_GET_BIT_FIELD32(arg_Error, 16U, 23U)

/**
 * MACRO NAME: IX_ERROR_SET_GROUP
 *
 * DESCRIPTION: This macro sets the error group field for an error token.
 *
 * @Param:  - IN arg_Error -  error token of  type ix_error.
 * @Param:  - IN arg_ErrorGroup - this is the new error group for the error token.
 *
 * @Return: Returns the new value of the error token.
 */
#define IX_ERROR_SET_GROUP( \
                            arg_Error, \
                            arg_ErrorGroup \
                          ) \
                          (ix_error)IX_SET_BIT_FIELD32(arg_Error, arg_ErrorGroup, 16U, 23U)
                


/**
 * MACRO NAME: IX_ERROR_GET_LEVEL
 *
 * DESCRIPTION: This macro retrieves the error level field from an error token.
 *
 * @Param:  - IN arg_Error -  error token of type ix_error.
 *
 * @Return: Returns a ix_uint32 value representing the error level for 
 *          this error token.
 */
#define IX_ERROR_GET_LEVEL( \
                            arg_Error \
                          ) \
                          IX_GET_BIT_FIELD32(arg_Error, 24U, 31U)

/**
 * MACRO NAME: IX_ERROR_SET_LEVEL
 *
 * DESCRIPTION: This macro sets the error level field for an error token.
 *
 * @Param:  - IN arg_Error -   error token of type ix_error.
 * @Param:  - IN arg_ErrorLevel - this is the new error level for the error token.
 *
 * @Return: Returns the new value of the error token.
 */
#define IX_ERROR_SET_LEVEL( \
                            arg_Error, \
                            arg_ErrorLevel \
                          ) \
                          (ix_error)IX_SET_BIT_FIELD32(arg_Error, arg_ErrorLevel, 24U, 31U)
                


/**
 * MACRO NAME: IX_ERROR_NEW
 *
 * DESCRIPTION: This macro generates a new error token based on the error code,
 *          group and level that are passed as arguments.
 *
 * @Param:  - IN arg_ErrorCode - the error code of the new error token to be generated.
 *          The range is 0..65535
 * @Param:  - IN arg_ErrorGroup - the error group of the new error token to be generated. 
 *          The range is 0..255
 * @Param:  - IN arg_ErrorLevel - the error level of the new error token to be generated.
 *          The range is 0..255.
 *
 * @Return: Returns a new ix_error token.
 */
#define IX_ERROR_NEW( \
                       arg_ErrorCode, \
                       arg_ErrorGroup, \
                       arg_ErrorLevel \
                    ) \
                    ((ix_error) /* typecast */ \
                    IX_MAKE_BIT_FIELD32(arg_ErrorCode, 0U, 15U) | \
                    IX_MAKE_BIT_FIELD32(arg_ErrorGroup, 16U, 23U) | \
                    IX_MAKE_BIT_FIELD32(arg_ErrorLevel, 24U, 31U))
                        

#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__IX_ERROR_H__) */

