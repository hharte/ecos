/**
 * @file    IxQMgr.h
 *
 * @date    30-Oct-2001
 *
 * @brief This file contains the public API of IxQMgr component.
 *
 * Some functions contained in this module are inline to achieve better
 * data-path performance. For this to work, the function definitions are
 * contained in this header file. The "normal" use of inline functions
 * is to use the inline functions in the module in which they are
 * defined. In this case these inline functions are used in external
 * modules and therefore the use of "inline extern". What this means
 * is as follows: if a function foo is declared as "inline extern" this
 * definition is only used for inlining, in no case is the function
 * compiled on its own. If the compiler cannot inline the function it
 * becomes an external reference. Therefore in IxQMgrQAccess.c all
 * inline functions are defined without the "inline extern" specifier
 * and so define the external references. In all other source files
 * including this header file, these funtions are defined as "inline
 * extern".
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

/* ------------------------------------------------------
   Doxygen group definitions
   ------------------------------------------------------ */
/**
 * @defgroup IxQMgrAPI IXP4xx Queue Manager (IxQMgr) API
 *
 * @brief The public API for the IXP4xx QMgr component.
 *
 * IxQMgr is a low level interface to the AHB Queue Manager
 *
 * @{
 */

#ifndef IXQMGR_H
#define IXQMGR_H

/*
 * User defined include files
 */
#include "IxTypes.h"

/*
 * #defines and macros
 */

/**
*
* @ingroup IxQMgrAPI
*
* @def IX_QMGR_INLINE
*
* @brief Inline definition, for inlining of Queue Access functions on API
*
* Please read the header information in this file for more details on the
* use of function inlining in this component.
*
*/
#ifdef IXQMGRQACCESS_C
/* If IXQMGRQACCESS_C is set then the IxQmgrQAccess.c is including this file
   and must instantiate a concrete definition for each inlineable API function
   whether or not that function actually gets inlined. */
#    ifdef NO_INLINE_APIS
#        undef NO_INLINE_APIS
#    endif
#    define IX_QMGR_INLINE  /* Empty Define */
#else
#    ifndef NO_INLINE_APIS
#       define IX_QMGR_INLINE __inline__ extern
#    else
#       define IX_QMGR_INLINE /* Empty Define */
#    endif
#endif

/**
*
* @ingroup IxQMgrAPI
*
* @def IX_QMGR_MAX_NUM_QUEUES
*
* @brief Number of queues supported by the AQM.
*
* This constant is used to indicate the number of AQM queues
*
*/
#define IX_QMGR_MAX_NUM_QUEUES  64

/**
*
* @ingroup IxQMgrAPI
*
* @def IX_QMGR_MIN_QID
*
* @brief Minimum queue identifier.
*
* This constant is used to indicate the smallest queue identifier
*
*/
#define IX_QMGR_MIN_QID IX_QMGR_QUEUE_0

/**
*
* @ingroup IxQMgrAPI
*
* @def IX_QMGR_MAX_QID
*
* @brief Maximum queue identifier.
*
* This constant is used to indicate the largest queue identifier
*
*/
#define IX_QMGR_MAX_QID IX_QMGR_QUEUE_63

/**
*
* @ingroup IxQMgrAPI
*
* @def IX_QMGR_MIN_QUEUPP_QID
*
* @brief Minimum queue identifier for reduced functionality queues.
*
* This constant is used to indicate Minimum queue identifier for reduced
* functionality queues
*
*/
#define IX_QMGR_MIN_QUEUPP_QID 32

/**
*
* @ingroup IxQMgrAPI
*
* @def IX_QMGR_MAX_QNAME_LEN
*
* @brief Maximum queue name length.
*
* This constant is used to indicate the maximum null terminated string length
* (excluding '\0') for a queue name
*
*/
#define IX_QMGR_MAX_QNAME_LEN 16

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_WARNING
 *
 * @brief Warning return code.
 *
 * Execution complete, but there is a special case to handle
 *
 */
#define IX_QMGR_WARNING 2

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_PARAMETER_ERROR
 *
 * @brief Parameter error return code (NULL pointer etc..).
 *
 * parameter error out of range/invalid
 *
 */
#define IX_QMGR_PARAMETER_ERROR 3

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_INVALID_Q_ENTRY_SIZE
 *
 * @brief Invalid entry size return code.
 *
 * Invalid queue entry size for a queue read/write
 *
 */
#define IX_QMGR_INVALID_Q_ENTRY_SIZE 4

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_INVALID_Q_ID
 *
 * @brief Invalid queue identifier return code.
 *
 * Invalid queue id, not in range 0-63
 *
 */
#define IX_QMGR_INVALID_Q_ID 5

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_INVALID_CB_ID
 *
 * @brief Invalid callback identifier return code.
 *
 * Invalid callback id
 */
#define IX_QMGR_INVALID_CB_ID 6

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_CB_ALREADY_SET
 *
 * @brief Callback set error return code.
 *
 * The specified callback has already been for this queue
 *
 */
#define IX_QMGR_CB_ALREADY_SET 7

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_NO_AVALIABLE_SRAM
 *
 * @brief Sram consumed return code.
 *
 * All AQM Sram is consumed by queue configuration
 *
 */
#define IX_QMGR_NO_AVALIABLE_SRAM 8

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_INVALID_INT_SOURCE_ID
 *
 * @brief Invalid queue interrupt source identifier return code.
 *
 * Invalid queue interrupt source given for notification enable
 */
#define IX_QMGR_INVALID_INT_SOURCE_ID 9

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_INVALID_QSIZE
 *
 * @brief Invalid queue size error code.
 *
 * Invalid queue size not one of 16,32, 64, 128
 *
 *
 */
#define IX_QMGR_INVALID_QSIZE 10

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_INVALID_Q_WM
 *
 * @brief Invalid queue watermark return code.
 *
 * Invalid queue watermark given for watermark set
 */
#define IX_QMGR_INVALID_Q_WM 11

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_Q_NOT_CONFIGURED
 *
 * @brief Queue not configured return code.
 *
 * Returned to the client when a function has been called on an unconfigured
 * queue
 *
 */
#define IX_QMGR_Q_NOT_CONFIGURED 12

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_Q_ALREADY_CONFIGURED
 *
 * @brief Queue already configured return code.
 *
 * Returned to client to indicate that a queue has already been configured
 */
#define IX_QMGR_Q_ALREADY_CONFIGURED 13

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_Q_UNDERFLOW
 *
 * @brief Underflow return code.
 *
 * Underflow on a queue read has occurred
 *
 */
#define IX_QMGR_Q_UNDERFLOW 14

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_Q_OVERFLOW
 *
 * @brief Overflow return code.
 *
 * Overflow on a queue write has occurred
 *
 */
#define IX_QMGR_Q_OVERFLOW 15

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_Q_INVALID_PRIORITY
 *
 * @brief Invalid priority return code.
 *
 * Invalid priority, not one of 0,1,2
 */
#define IX_QMGR_Q_INVALID_PRIORITY 16

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @def IX_QMGR_ENTRY_INDEX_OUT_OF_BOUNDS
 *
 * @brief  Entry index out of bounds return code.
 *
 * Entry index is greater than number of entries in queue.
 */
#define IX_QMGR_ENTRY_INDEX_OUT_OF_BOUNDS 17

/*
 * Typedefs
 */

/**
 *
 * @enum IxQMgrQId
 *
 * @ingroup IxQMgrAPI
 *
 * @brief Generic identifiers for AQM queues.
 *
 * These enum values are used in the AQM queue config header file. This enum
 * defines generic identifiers for the queues in that the application of the
 * queue is not specified. The connection between queue number and queue
 * application is done in the AQM queue config header file. Clients of this
 * component should NOT use these values to identify queues, the #defines in
 * the AQM queue configuration header should be used.
 */
typedef enum
{
    IX_QMGR_QUEUE_0 = 0,
    IX_QMGR_QUEUE_1,
    IX_QMGR_QUEUE_2,
    IX_QMGR_QUEUE_3,
    IX_QMGR_QUEUE_4,
    IX_QMGR_QUEUE_5,
    IX_QMGR_QUEUE_6,
    IX_QMGR_QUEUE_7,
    IX_QMGR_QUEUE_8,
    IX_QMGR_QUEUE_9,
    IX_QMGR_QUEUE_10,
    IX_QMGR_QUEUE_11,
    IX_QMGR_QUEUE_12,
    IX_QMGR_QUEUE_13,
    IX_QMGR_QUEUE_14,
    IX_QMGR_QUEUE_15,
    IX_QMGR_QUEUE_16,
    IX_QMGR_QUEUE_17,
    IX_QMGR_QUEUE_18,
    IX_QMGR_QUEUE_19,
    IX_QMGR_QUEUE_20,
    IX_QMGR_QUEUE_21,
    IX_QMGR_QUEUE_22,
    IX_QMGR_QUEUE_23,
    IX_QMGR_QUEUE_24,
    IX_QMGR_QUEUE_25,
    IX_QMGR_QUEUE_26,
    IX_QMGR_QUEUE_27,
    IX_QMGR_QUEUE_28,
    IX_QMGR_QUEUE_29,
    IX_QMGR_QUEUE_30,
    IX_QMGR_QUEUE_31,
    IX_QMGR_QUEUE_32,
    IX_QMGR_QUEUE_33,
    IX_QMGR_QUEUE_34,
    IX_QMGR_QUEUE_35,
    IX_QMGR_QUEUE_36,
    IX_QMGR_QUEUE_37,
    IX_QMGR_QUEUE_38,
    IX_QMGR_QUEUE_39,
    IX_QMGR_QUEUE_40,
    IX_QMGR_QUEUE_41,
    IX_QMGR_QUEUE_42,
    IX_QMGR_QUEUE_43,
    IX_QMGR_QUEUE_44,
    IX_QMGR_QUEUE_45,
    IX_QMGR_QUEUE_46,
    IX_QMGR_QUEUE_47,
    IX_QMGR_QUEUE_48,
    IX_QMGR_QUEUE_49,
    IX_QMGR_QUEUE_50,
    IX_QMGR_QUEUE_51,
    IX_QMGR_QUEUE_52,
    IX_QMGR_QUEUE_53,
    IX_QMGR_QUEUE_54,
    IX_QMGR_QUEUE_55,
    IX_QMGR_QUEUE_56,
    IX_QMGR_QUEUE_57,
    IX_QMGR_QUEUE_58,
    IX_QMGR_QUEUE_59,
    IX_QMGR_QUEUE_60,
    IX_QMGR_QUEUE_61,
    IX_QMGR_QUEUE_62,
    IX_QMGR_QUEUE_63,
    IX_QMGR_QUEUE_INVALID
} IxQMgrQId;

/**
 * @enum IxQMgrQStatus
 *
 * @ingroup IxQMgrAPI
 *
 * @brief Queue status.
 *
 * A queues status is defined by its relative fullness or relative emptiness.
 * Each of the queues 0-31 have Nearly Empty, Nearly Full, Empty, Full,
 * Underflow and Overflow status flags. Queues 32-63 have just Nearly Empty and
 * Full status flags.
 * The flags bit positions are outlined below
 *
 *         OF  UF  F  NF  NE   E
 *         ----------------------
 *         5 | 4 | 3 | 2 | 1 | 0
 *
 */
typedef UINT32 IxQMgrQStatus;

/**
 * @enum IxQMgrQStatusMask
 *
 * @ingroup IxQMgrAPI
 *
 * @brief Queue status mask.
 *
 * Masks for extracting the individual status flags from the IxQMgrStatus
 * word.
 *
 */
typedef enum
{
    IX_QMGR_Q_STATUS_E_BIT_MASK  = 0x1,
    IX_QMGR_Q_STATUS_NE_BIT_MASK = 0x2,
    IX_QMGR_Q_STATUS_NF_BIT_MASK = 0x4,
    IX_QMGR_Q_STATUS_F_BIT_MASK  = 0x8,
    IX_QMGR_Q_STATUS_UF_BIT_MASK = 0x10,
    IX_QMGR_Q_STATUS_OF_BIT_MASK = 0x20
} IxQMgrQStatusMask;

/**
 * @enum IxQMgrSourceId
 *
 * @ingroup IxQMgrAPI
 *
 * @brief Queue interrupt source select.
 *
 * This enum defines the different source conditions on a queue that result in
 * an interupt being fired by the AQM. Interrupt source is configurable for
 * queues 0-31 only. The interrupt source for queues 32-63 is hardwired to the
 * NE(Nearly Empty) status flag.
 *
 */
typedef enum
{
    IX_QMGR_Q_SOURCE_ID_E = 0,  /**< Queue Empty due to last read             */
    IX_QMGR_Q_SOURCE_ID_NE,     /**< Queue Nearly Empty due to last read      */
    IX_QMGR_Q_SOURCE_ID_NF,     /**< Queue Nearly Full due to last write      */
    IX_QMGR_Q_SOURCE_ID_F,      /**< Queue Full due to last write             */
    IX_QMGR_Q_SOURCE_ID_NOT_E,  /**< Queue Not Empty due to last write        */
    IX_QMGR_Q_SOURCE_ID_NOT_NE, /**< Queue Not Nearly Empty due to last write */
    IX_QMGR_Q_SOURCE_ID_NOT_NF, /**< Queue Not Nearly Full due to last read   */
    IX_QMGR_Q_SOURCE_ID_NOT_F   /**< Queue Not Full due to last read          */
} IxQMgrSourceId;

/**
 * @enum IxQMgrQEntrySizeInWords
 *
 * @ingroup IxQMgrAPI
 *
 * @brief QMgr queue entry sizes.
 *
 * The entry size of a queue specifies the size of a queues entry in words.
 *
 */
typedef enum
{
    IX_QMGR_Q_ENTRY_SIZE1 = 1,   /**< 1 word entry       */
    IX_QMGR_Q_ENTRY_SIZE2 = 2,   /**< 2 word entry       */
    IX_QMGR_Q_ENTRY_SIZE4 = 4    /**< 4 word entry       */
} IxQMgrQEntrySizeInWords;

/**
 * @enum IxQMgrQSizeInWords
 *
 * @ingroup IxQMgrAPI
 *
 * @brief QMgr queue sizes.
 *
 * These values define the allowed queue sizes for AQM queue. The sizes are
 * specified in words.
 *
 */
typedef enum
{
    IX_QMGR_Q_SIZE16 = 16,   /**< 16 word buffer     */
    IX_QMGR_Q_SIZE32 = 32,   /**< 32 word buffer     */
    IX_QMGR_Q_SIZE64 = 64,   /**< 64 word buffer     */
    IX_QMGR_Q_SIZE128 = 128, /**< 128 word buffer    */
    IX_QMGR_Q_SIZE_INVALID = 129  /**< Insure that this is greater than largest
				    * queue size supported by the hardware
				    */
} IxQMgrQSizeInWords;

/**
 * @enum IxQMgrWMLevel
 *
 * @ingroup IxQMgrAPI
 *
 * @brief QMgr watermark levels.
 *
 * These values define the valid watermark levels(in ENTRIES) for queues. Each
 * queue 0-63 have configurable Nearly full and Nearly empty watermarks. For
 * queues 32-63 the Nearly full watermark has NO EFFECT.
 * If the Nearly full watermark is set to IX_QMGR_Q_WM_LEVEL16 this means that
 * the nearly full flag will be set by the hardware when there are >= 16 empty
 * entries in the specified queue.
 * If the Nearly empty watermark is set to IX_QMGR_Q_WM_LEVEL16 this means that
 * the Nearly empty flag will be set by the hardware when there are <= 16 full
 * entries in the specified queue.
 */
typedef enum
{
    IX_QMGR_Q_WM_LEVEL0 = 0,    /**< 0 entry watermark  */
    IX_QMGR_Q_WM_LEVEL1 = 1,    /**< 1 entry watermark  */
    IX_QMGR_Q_WM_LEVEL2 = 2,    /**< 2 entry watermark  */
    IX_QMGR_Q_WM_LEVEL4 = 4,    /**< 4 entry watermark  */
    IX_QMGR_Q_WM_LEVEL8 = 8,    /**< 8 entry watermark  */
    IX_QMGR_Q_WM_LEVEL16 = 16,  /**< 16 entry watermark */
    IX_QMGR_Q_WM_LEVEL32 = 32,  /**< 32 entry watermark */
    IX_QMGR_Q_WM_LEVEL64 = 64   /**< 64 entry watermark */
} IxQMgrWMLevel;

/**
 * @enum IxQMgrDispatchGroup
 *
 * @brief QMgr dispatch group select identifiers.
 *
 * This enum defines the groups over which the  dispatcher will process when
 * called. One of the enum values must be used as a input to
 * ixQMgrDispatcherLoopRun().
 *
 */
typedef enum
{
    IX_QMGR_QUELOW_GROUP = 0, /**< Queues 0-31  */
    IX_QMGR_QUEUPP_GROUP      /**< Queues 32-63 */
} IxQMgrDispatchGroup;

/**
 * @enum IxQMgrPriority
 *
 * @brief Dispatcher priority levels.
 *
 * This enum defines the different queue dispatch priority levels.
 * The lowest priority number (0) is the highest priority level.
 *
 */
typedef enum
{
  IX_QMGR_Q_PRIORITY_0 = 0,  /**< Priority level 0 */
  IX_QMGR_Q_PRIORITY_1,      /**< Priority level 1 */
  IX_QMGR_Q_PRIORITY_2,      /**< Priority level 2 */
  IX_QMGR_Q_PRIORITY_INVALID /**< Invalid Priority level */
} IxQMgrPriority;

/**
 * @typedef IxQMgrCallbackId
 *
 * @brief Uniquely identifies a callback function.
 *
 * A unique callback identifier associated with each callback
 * registered by clients.
 *
 */
typedef unsigned IxQMgrCallbackId;

/**
 * @typedef IxQMgrCallback
 *
 * @brief QMgr notification callback type.
 *
 * This defines the interface to all client callback functions.
 *
 * @param IxQMgrQId qId(in) - the queue identifier
 * @param IxQMgrCallbackId cbId(in) - the callback identifier
 */
typedef void (*IxQMgrCallback)(IxQMgrQId qId,
                               IxQMgrCallbackId cbId);

/*
 * Function Prototypes
 */

/* ------------------------------------------------------------
   Initialisation related functions
   ---------------------------------------------------------- */

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrInit (void)
 *
 * @brief Initialise the QMgr.
 *
 * This function must be called before and other QMgr function. It
 * sets up internal data structures.
 *
 * @return @li IX_SUCCESS, the IxQMgr successfully initialised
 * @return @li IX_FAIL, failed to initialize the Qmgr
 *
 */
PUBLIC IX_STATUS
ixQMgrInit (void);

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrShow (void)
 *
 * @brief Describe queue configuration and statistics for active queues.
 *
 * This function shows active queues, their configurations and statistics.
 *
 * @return @li void
 *
 */
PUBLIC void
ixQMgrShow (void);

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrQShow (IxQMgrQId qId)
 *
 * @brief Display aqueue configuration and statistics for a queue.
 *
 * This function shows queue configuration and statistics for a queue.
 *
 * @param IxQMgrQIdin(qId) - the queue identifier.
 *
 * @return @li IX_SUCCESS, success
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId
 *
 */
PUBLIC IX_STATUS
ixQMgrQShow (IxQMgrQId qId);


/* ------------------------------------------------------------
   Configuration related functions
   ---------------------------------------------------------- */

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQConfig (char *qName,
	       IxQMgrQId qId,
	       IxQMgrQSizeInWords qSizeInWords,
	       IxQMgrQEntrySizeInWords qEntrySizeInWords)
 *
 * @brief Configure an AQM queue.
 *
 * This function is called by a client to setup a queue. The size and entrySize
 * qId and qName(NULL pointer) are checked for valid values. This function must
 * be called for each queue, before any queue accesses are made and after
 * ixQMgrInit() has been called. qName is assumed to be a '\0' terminated array
 * of 16 charachters or less.
 *
 * @param char(in) *qName - is the name provided by the client and is associated
 *                          with a QId by the QMgr.
 * @param IxQMgrQId(in)  qId - the qId of this queue
 * @param IxQMgrQSize(in) qSizeInWords - the size of the queue can be one of 16,32
 *                                       64, 128 words.
 * @param IxQMgrQEntrySizeInWords(in) qEntrySizeInWords - the size of a queue entry
 *                                                        can be one of 1,2,4 words.
 *
 * @return @li IX_SUCCESS, a specified queue has been successfully configured.
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid parameter(s).
 * @return @li IX_QMGR_INVALID_QSIZE, invalid queue size
 * @return @li IX_QMGR_INVALID_Q_ID, invalid queue id
 * @return @li IX_QMGR_INVALID_Q_ENTRY_SIZE, invalid queue entry size
 * @return @li IX_QMGR_Q_ALREADY_CONFIGURED, queue already configured
 *
 */
PUBLIC IX_STATUS
ixQMgrQConfig (char *qName,
	       IxQMgrQId qId,
	       IxQMgrQSizeInWords qSizeInWords,
	       IxQMgrQEntrySizeInWords qEntrySizeInWords);

/**
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQSizeInEntriesGet (IxQMgrQId qId,
			 unsigned *qSizeInEntries)
 *
 * @brief Return the size of a queue in entries.
 *
 * This function returns the the size of the queue in entriese.
 *
 * @param IxQMgrQId(in) qId - the queue identifier
 * @param IxQMgrQSize(out) *qSizeInEntries - queue size in entries
 *
 * @return @li IX_SUCCESS, successfully retrieved the number of full entrie
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid parameter(s).
 *
 */
PUBLIC IX_STATUS
ixQMgrQSizeInEntriesGet (IxQMgrQId qId,
			 unsigned *qSizeInEntries);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrWatermarkSet (IxQMgrQId qId,
		    IxQMgrWMLevel ne,
		    IxQMgrWMLevel nf)
 *
 * @brief Set the Nearly Empty and Nearly Full Watermarks fo a queue.
 *
 * This function is called by a client to set the watermarks NE and NF for the
 * queue specified by qId.
 * The queue must be empty at the time this function is called, it is the clients
 * responsibility to ensure that the queue is empty.
 * This function will read the status of the queue before the watermarks are set
 * and again after the watermarks are set. If the status register has changed,
 * due to a queue access by an NPE for example, a warning is returned.
 * Queues 32-63 only support the NE flag, therefore the value of nf will be ignored
 * for these queues.
 *
 * @param IxQMgrQId(in) qId -  the QId of the queue.
 * @param IxQMgrWMLevel(in) ne  - the NE(Nearly Empty) watermark for this
 *                                 queue. Valid values are 0,1,2,4,8,16,32 and
 *                                 64 entries.
 * @param IxQMgrWMLevel(in) nf - the NF(Nearly Full) watermark for this queue.
 *                                 Valid values are 0,1,2,4,8,16,32 and 64
 *                                 entries.
 *
 * @return @li IX_SUCCESS, watermarks have been set for the queu
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId
 * @return @li IX_QMGR_INVALID_Q_WM, invalid watermark
 * @return @li IX_QMGR_WARNING, the status register may not be constistent
 *
 */
PUBLIC IX_STATUS
ixQMgrWatermarkSet (IxQMgrQId qId,
		    IxQMgrWMLevel ne,
		    IxQMgrWMLevel nf);

/**
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrAvailableSramAddressGet (UINT32 *address,
			       unsigned *sizeOfFreeSram)
 *
 * @brief Return the address of available AQM SRAM.
 *
 * This function returns the starting address in AQM SRAM not used by the
 * current queue configuration and should only be called after all queues
 * have been configured.
 * Calling this function before all queues have been configured will will return
 * the currently available SRAM. A call to configure another queue will use some
 * of the available SRAM.
 * The amount of SRAM available is specified in sizeOfFreeSram. The address is the
 * address of the bottom of available SRAM. Available SRAM extends from address
 * from address to address + sizeOfFreeSram.
 *
 * @param UINT32(out) **address - the address of the available SRAM, NULL if
 *                                none available.
 * @param unsigned(out) *sizeOfFreeSram - the size in words of available SRAM
 *
 * @return @li IX_SUCCESS, there is available SRAM and is pointed to by address
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid parameter(s)
 * @return @li IX_QMGR_NO_AVALIABLE_SRAM, all AQM SRAM is consumed by the queue
 *             configuration.
 *
 */
PUBLIC IX_STATUS
ixQMgrAvailableSramAddressGet (UINT32 *address,
			       unsigned *sizeOfFreeSram);


/* ------------------------------------------------------------
   Queue access related functions
   ---------------------------------------------------------- */

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQReadWithChecks (IxQMgrQId qId,
                       UINT32 *entry)
 *
 * @brief Read an entry from a queue.
 *
 * This function reads an entire entry from a queue returning it in entry. The
 * queue configuration word is read to determine what entry size this queue is
 * configured for and then the number of words specified by the entry size is
 * read.  entry must be a pointer to a previously allocated array of sufficient
 * size to hold an entry.
 *
 * @note - IX_QMGR_Q_UNDERFLOW is only returned for queues 0-31 as queues 32-63
 * do not have an underflow status maintained.
 *
 * @param  IxQMgrQId(in)  qId   - the queue identifier.
 * @param  UINT32(out) *entry - pointer to the entry word(s).
 *
 * @return @li IX_SUCCESS, entry was successfully read.
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid paramter(s).
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId
 * @return @li IX_QMGR_Q_UNDERFLOW, attempt to read from an empty queue
 *
 */
PUBLIC IX_STATUS
ixQMgrQReadWithChecks (IxQMgrQId qId,
                       UINT32 *entry);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQRead (IxQMgrQId qId,
	     UINT32 *entry)
 *
 * @brief Fast read of an entry from a queue.
 *
 * This function is a heavily streamlined version of ixQMgrQReadWithChecks(),
 * but performs essentially the same task.  It reads an entire entry from a
 * queue, returning it in entry which must be a pointer to a previously
 * allocated array of sufficient size to hold an entry.
 *
 * @note - This function is inlined, to reduce unnecessary function call
 * overhead.  It does not perform any parameter checks, or update any statistics.
 * Also, it does not check that the queue specified by qId has been configured.
 * or is in range. It simply reads an entry from the queue, and checks for
 * underflow.
 *
 * @note - IX_QMGR_Q_UNDERFLOW is only returned for queues 0-31 as queues 32-63
 * do not have an underflow status maintained.
 *
 * @param  IxQMgrQId(in)  qId - the queue identifier.
 * @param  UINT32(out) *entry - pointer to the entry word(s).
 *
 * @return @li IX_SUCCESS, entry was successfully read.
 * @return @li IX_QMGR_Q_UNDERFLOW, attempt to read from an empty queue
 *
 */
IX_QMGR_INLINE PUBLIC IX_STATUS
ixQMgrQRead (IxQMgrQId qId,
	     UINT32 *entry)
#ifdef NO_INLINE_APIS
    ;
#else
{
    extern UINT32 ixQMgrAqmIfQueAccRegAddr[];
    extern UINT32 ixQMgrAqmIfQueUOStatRegAddr[];
    extern UINT32 ixQMgrAqmIfQueUflowStatBitMask[];
    extern IxQMgrQEntrySizeInWords cfgQEntrySizeInWords[];

    volatile UINT32 *qAccRegAddr = (UINT32*)ixQMgrAqmIfQueAccRegAddr[qId];

    if (cfgQEntrySizeInWords[qId] == IX_QMGR_Q_ENTRY_SIZE1)
    {
	*entry = *qAccRegAddr;
    }
    else
    {
	IxQMgrQEntrySizeInWords entrySizeInWords = cfgQEntrySizeInWords[qId];
	/* read the queue entry */
	while (entrySizeInWords--)
	{
	    *entry++ = *qAccRegAddr++;
	}
    }

    /* Check if underflow occurred on the read */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
        volatile UINT32 *qUflowStatRegAddr = (UINT32*)ixQMgrAqmIfQueUOStatRegAddr[qId];
        UINT32 qUflowStatBitMask = ixQMgrAqmIfQueUflowStatBitMask[qId];

	/* get the status twice because it may not be ready 
	 * at the time of reading 
	 */
	if ((*qUflowStatRegAddr & qUflowStatBitMask) ||
	    (*qUflowStatRegAddr & qUflowStatBitMask))
	{
	    /* clear the underflow status bit if it was set */
	    *qUflowStatRegAddr &= ~qUflowStatBitMask;
	    return IX_QMGR_Q_UNDERFLOW;
	}
    }

    return IX_SUCCESS;
}
#endif

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQBurstRead (IxQMgrQId qId,
		  UINT32 numEntries,
		  UINT32 *entries)
 *
 * @brief Read a number of entries from an AQM queue.
 *
 * This function will burst read a number of entries from the specified queue.
 * The entry size of queue is auto-detected. The function will attempt to
 * read as many entries as specified by the numEntries parameter and will
 * return an UNDERFLOW if any one of the individual entry reads fail.
 *
 * @warning
 * IX_QMGR_Q_UNDERFLOW is only returned for queues 0-31 as queues 32-63
 * do not have an underflow status maintained, hence there is a potential for
 * silent failure here. This function must be used with caution.
 *
 * @note
 * This function is intended for fast draining of queues, so to make it
 * as efficient as possible, it has the following features:
 * - This function is inlined, to reduce unnecessary function call overhead.
 * - It does not perform any parameter checks, or update any statistics.
 * - It does not check that the queue specified by qId has been configured.
 * - It does not check that the queue has the number of full entries that
 * have been specified to be read. It will read until it finds a NULL entry or
 * until the number of specified entries have been read.  It always checks for
 * underflow after all the reads have been performed.
 * Therefore, the client should ensure before calling this function that there
 * are enough entries in the queue to read.  ixQMgrQNumEntriesGet() will
 * provide the number of full entries in a queue.
 * ixQMgrQRead() or ixQMgrQReadWithChecks(), which only reads
 * a single queue entry per call, should be used instead if the user requires
 * checks for UNDERFLOW after each entry read.
 *
 * @param IxQMgrQId(in) qId   - the queue identifier.
 * @param unsigned(in) numEntries - the number of entries to write.
 * @param UINT32(out) *entries - the word(s) read.
 *
 * @return @li IX_SUCCESS, entries were successfully read.
 * @return @li IX_QMGR_Q_UNDERFLOW, attempt to read from an empty queue 
 *
 */
IX_QMGR_INLINE PUBLIC IX_STATUS
ixQMgrQBurstRead (IxQMgrQId qId,
		  UINT32 numEntries,
		  UINT32 *entries)
#ifdef NO_INLINE_APIS
;
#else
{
    extern UINT32 ixQMgrAqmIfQueAccRegAddr[];
    extern UINT32 ixQMgrAqmIfQueUOStatRegAddr[];
    extern UINT32 ixQMgrAqmIfQueUflowStatBitMask[];
    extern IxQMgrQEntrySizeInWords cfgQEntrySizeInWords[];

    int i;
    IxQMgrQEntrySizeInWords entrySizeInWords = cfgQEntrySizeInWords[qId];
    volatile UINT32 *qAccRegAddr = (UINT32*)ixQMgrAqmIfQueAccRegAddr[qId];

    if (entrySizeInWords == IX_QMGR_Q_ENTRY_SIZE1)
    {
	/* read the specified number of queue entries */
	while (numEntries--)
	{
	    *entries = *qAccRegAddr;
	    if (*entries == 0)
	    {
		/* if we read a NULL entry, stop. We have underflowed */
		break;
	    }
	    else
	    {
		entries++;
	    }
	}
    }
    else
    {
	/* read the specified number of queue entries */
	while (numEntries--)
	{
	    UINT32 nullCheckEntry = 0;

	    for (i = 0; i < entrySizeInWords; i++)
	    {
		*entries = *(qAccRegAddr + i);
		nullCheckEntry |= *entries;
		entries++;
	    }

	    /* if we read a NULL entry, stop. We have underflowed */
	    if (nullCheckEntry == 0)
	    {
		break;
	    }
	}
    }

    /* Check if underflow occurred on the read */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
        volatile UINT32 *qUflowStatRegAddr = (UINT32*)ixQMgrAqmIfQueUOStatRegAddr[qId];
        UINT32 qUflowStatBitMask = ixQMgrAqmIfQueUflowStatBitMask[qId];
	UINT32 tempStatus;

	/* get the status twice because it may not be ready 
	 * at the time of reading 
	 */
	if (((tempStatus = *qUflowStatRegAddr) & qUflowStatBitMask) ||
	    ((tempStatus = *qUflowStatRegAddr) & qUflowStatBitMask))
	{
	    /* clear the underflow status bit if it was set */
	    *qUflowStatRegAddr = tempStatus & ~qUflowStatBitMask;
	    return IX_QMGR_Q_UNDERFLOW;
	}
    }

    return IX_SUCCESS;
}
#endif

/**
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQPeek (IxQMgrQId qId,
	     unsigned int entryIndex,
	     UINT32 *entry)
 *
 * @brief Read an entry from a queue without moving the read pointer.
 *
 * This function inspects an entry in a queue. The entry is inspected directly
 * in AQM SRAM and is not read from queue access registers. The entry is NOT removed
 * from the queue and the read/write pointers are unchanged.
 * N.B: The queue should not be accessed when this function is called.
 *
 * @param  IxQMgrQId(in)  qId   - the queue identifier.
 * @param  unsigned int(in) entryIndex - index of entry in queue in the range
 *                          [0].......[current number of entries in queue].
 * @param  UINT32(out) *entry - pointer to the entry word(s).
 *
 * @return @li IX_SUCCESS, entry was successfully inspected.
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid paramter(s).
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId.
 * @return @li IX_QMGR_ENTRY_INDEX_OUT_OF_BOUNDS, an entry does not exist at
 *             specified index.
 * @return @li IX_FAIL, failed to inpected the queue entry.
 */
IX_STATUS
ixQMgrQPeek (IxQMgrQId qId,
	     unsigned int entryIndex,
	     UINT32 *entry);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQWriteWithChecks (IxQMgrQId qId,
                        UINT32 *entry)
 *
 * @brief Write an entry to an AQM queue.
 *
 * This function will write the entry size number of words pointed to by entry to
 * the queue specified by qId. The queue configuration word is read to
 * determine the entry size of queue and the corresponding number of words is
 * then written to the queue.
 *
 * @note - IX_QMGR_Q_OVERFLOW is only returned for queues 0-31 as queues 32-63
 * do not have an overflow status maintained.
 *
 * @param IxQMgrQId(in) qId - the queue identifier.
 * @param UINT32(in) *entry - the word(s) to write.
 *
 * @return @li IX_SUCCESS, value was successfully written.
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid paramter(s).
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId
 * @return @li IX_QMGR_Q_OVERFLOW, attempt to write to a full queue
 *
 */
PUBLIC IX_STATUS
ixQMgrQWriteWithChecks (IxQMgrQId qId,
                        UINT32 *entry);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQWrite (IxQMgrQId qId,
	      UINT32 *entry)
 *
 * @brief Fast write of an entry to a queue.
 *
 * This function is a heavily streamlined version of ixQMgrQWriteWithChecks(),
 * but performs essentially the same task.  It will write the entry size number
 * of words pointed to by entry to the queue specified by qId.
 *
 * @note - This function is inlined, to reduce unnecessary function call
 * overhead.  It does not perform any parameter checks, or update any
 * statistics. Also, it does not check that the queue specified by qId has
 * been configured. It simply writes an entry to the queue, and checks for
 * overflow.
 *
 * @note - IX_QMGR_Q_OVERFLOW is only returned for queues 0-31 as queues 32-63
 * do not have an overflow status maintained.
 *
 * @param  IxQMgrQId(in)  qId   - the queue identifier.
 * @param  UINT32(in) *entry - pointer to the entry word(s).
 *
 * @return @li IX_SUCCESS, entry was successfully read.
 * @return @li IX_QMGR_Q_OVERFLOW, attempt to write to a full queue
 *
 */
IX_QMGR_INLINE PUBLIC IX_STATUS
ixQMgrQWrite (IxQMgrQId qId,
	      UINT32 *entry)
#ifdef NO_INLINE_APIS
    ;
#else
{
    extern UINT32 ixQMgrAqmIfQueAccRegAddr[];
    extern UINT32 ixQMgrAqmIfQueUOStatRegAddr[];
    extern UINT32 ixQMgrAqmIfQueOflowStatBitMask[];
    extern IxQMgrQEntrySizeInWords cfgQEntrySizeInWords[];

    volatile UINT32 *qAccRegAddr = (UINT32*)ixQMgrAqmIfQueAccRegAddr[qId];

    /* write the queue entry */
    if (cfgQEntrySizeInWords[qId] == IX_QMGR_Q_ENTRY_SIZE1)
    {
	*qAccRegAddr = *entry;
    }
    else
    {
       IxQMgrQEntrySizeInWords entrySizeInWords = cfgQEntrySizeInWords[qId];
       while (entrySizeInWords--)
       {
	   *qAccRegAddr++ = *entry++;
       }
    }

    /* Check if overflow occurred on the write */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
        volatile UINT32 *qOflowStatRegAddr = (UINT32*)ixQMgrAqmIfQueUOStatRegAddr[qId];
        UINT32 qOflowStatBitMask = ixQMgrAqmIfQueOflowStatBitMask[qId];

	/* get the status twice because it may not be ready 
	 * at the time of reading 
	 */
	if ((*qOflowStatRegAddr & qOflowStatBitMask) ||
	    (*qOflowStatRegAddr & qOflowStatBitMask))
	{
	    /* clear the overflow status bit if it was set */
	    *qOflowStatRegAddr &= ~qOflowStatBitMask;
	    return IX_QMGR_Q_OVERFLOW;
	}
    }

    return IX_SUCCESS;
}
#endif

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQBurstWrite (IxQMgrQId qId,
		   unsigned numEntries,
		   UINT32 *entries)
 *
 * @brief Write a number of entries to an AQM queue.
 *
 * This function will burst write a number of entries to the specified queue.
 * The entry size of queue is auto-detected. The function will attempt to
 * write as many entries as specified by the numEntries parameter and will
 * return an OVERFLOW if any one of the individual entry writes fail.
 *
 * @warning
 * IX_QMGR_Q_OVERFLOW is only returned for queues 0-31 as queues 32-63
 * do not have an overflow status maintained, hence there is a potential for
 * silent failure here. This function must be used with caution.
 *
 * @note
 * This function is intended for fast population of queues, so to make it
 * as efficient as possible, it has the following features:
 * - This function is inlined, to reduce unnecessary function call overhead.
 * - It does not perform any parameter checks, or update any statistics.
 * - It does not check that the queue specified by qId has been configured.
 * - It does not check that the queue has enough free space to hold the entries
 * before writing, and only checks for overflow after all writes have been
 * performed.  Therefore, the client should ensure before calling this function
 * that there is enough free space in the queue to hold the number of entries
 * to be written.  ixQMgrQWrite() or ixQMgrQWriteWithChecks(), which only writes
 * a single queue entry per call, should be used instead if the user requires
 * checks for OVERFLOW after each entry written.
 *
 * @param IxQMgrQId(in) qId   - the queue identifier.
 * @param unsigned(in) numEntries - the number of entries to write.
 * @param UINT32(in) *entries - the word(s) to write.
 *
 * @return @li IX_SUCCESS, value was successfully written.
 * @return @li IX_QMGR_Q_OVERFLOW, attempt to write to a full queue
 *
 */
IX_QMGR_INLINE PUBLIC IX_STATUS
ixQMgrQBurstWrite (IxQMgrQId qId,
		   unsigned numEntries,
		   UINT32 *entries)
#ifdef NO_INLINE_APIS
;
#else
{
    extern UINT32 ixQMgrAqmIfQueAccRegAddr[];
    extern UINT32 ixQMgrAqmIfQueUOStatRegAddr[];
    extern UINT32 ixQMgrAqmIfQueOflowStatBitMask[];
    extern IxQMgrQEntrySizeInWords cfgQEntrySizeInWords[];

    volatile UINT32 *qAccRegAddr = (UINT32*)ixQMgrAqmIfQueAccRegAddr[qId];

    if (cfgQEntrySizeInWords[qId] == IX_QMGR_Q_ENTRY_SIZE1)
    {
	/* write each queue entry */
	while (numEntries--)
	{
	   *qAccRegAddr = *entries++;
	}
    }
    else
    {
	IxQMgrQEntrySizeInWords entrySizeInWords = cfgQEntrySizeInWords[qId];
	int i;

	/* write each queue entry */
	while (numEntries--)
	{
	    /* write the queueEntrySize number of words for each entry */
	    for (i = 0; i < entrySizeInWords; i++)
	    {
		*(qAccRegAddr + i) = *entries++;
	    }
	}
    }

    /* Check if overflow occurred on the write */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
        volatile UINT32 *qOflowStatRegAddr = (UINT32*)ixQMgrAqmIfQueUOStatRegAddr[qId];
        UINT32 qOflowStatBitMask = ixQMgrAqmIfQueOflowStatBitMask[qId];

	/* get the status twice because it may not be ready 
	 * at the time of reading 
	 */
	if ((*qOflowStatRegAddr & qOflowStatBitMask) ||
	    (*qOflowStatRegAddr & qOflowStatBitMask))
	{
	    /* clear the overflow status bit if it was set */
	    *qOflowStatRegAddr &= ~qOflowStatBitMask;
	    return IX_QMGR_Q_OVERFLOW;
	}
    }

    return IX_SUCCESS;
}
#endif

/**
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQPoke (IxQMgrQId qId,
	     unsigned int entryIndex,
	     UINT32 *entry)
 *
 * @brief Write an entry to a queue without moving the write pointer.
 *
 * This function modifies an entry in a queue. The entry is modified directly
 * in AQM SRAM and not using the queue access registers. The entry is NOT added to the
 * queue and the read/write pointers are unchanged.
 * N.B: The queue should not be accessed when this function is called.
 *
 * @param  IxQMgrQId(in) qId - the queue identifier.
 * @param  unsigned int(in) entryIndex - index of entry in queue in the range
 *                          [0].......[current number of entries in queue].
 * @param  UINT32(in) *entry - pointer to the entry word(s).
 *
 * @return @li IX_SUCCESS, entry was successfully modified.
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid paramter(s).
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, queue not configured for this QId.
 * @return @li IX_QMGR_ENTRY_INDEX_OUT_OF_BOUNDS, an entry does not exist at
 *             specified index.
 * @return @li IX_FAIL, failed to modify the queue entry.
 */
IX_STATUS
ixQMgrQPoke (IxQMgrQId qId,
	     unsigned int entryIndex,
	     UINT32 *entry);
/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQNumEntriesGet (IxQMgrQId qId,
		      unsigned *numEntries)
 *
 * @brief Get a snapshot of the number of entries in a queue.
 *
 * This function gets the number of entries in a queue.
 *
 * @param IxQMgrQId(in) qId - the queue idenfifier
 * @param unsigned(out) *numEntries - the number of entries in a queue
 *
 * @return @li IX_SUCCESS, got the number of entries for the queue
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid paramter(s).
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, the specified qId has not been configured
 * @return @li IX_QMGR_WARNING, could not determine num entries at this time
 *
 */
PUBLIC IX_STATUS
ixQMgrQNumEntriesGet (IxQMgrQId qId,
		      unsigned *numEntries);

/**
 *
 * @ingroup IxQMgrAPI
 *
 * @fn ixQMgrQStatusGetWithChecks (IxQMgrQId qId,
                            IxQMgrQStatus *qStatus)
 *
 * @brief Get a queues status.
 *
 * This function reads the specified queues status. A queues status is defined
 * by its status flags. For queues 0-31 these flags are E,NE,NF,F,UF,OF. For
 * queues 32-63 these flags are NE and F.
 *
 * @param IxQMgrQId(in) qId - the queue identifier.
 * @param IxQMgrQStatus(out) *qStatus - the status of the specified queue.
 *
 * @return @li IX_SUCCESS, queue status was successfully read.
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, the specified qId has not been configured
 * @return @li IX_QMGR_PARAMETER_ERROR, invalid paramter.
 *
 */
PUBLIC IX_STATUS
ixQMgrQStatusGetWithChecks (IxQMgrQId qId,
                            IxQMgrQStatus *qStatus);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrQStatusGet (IxQMgrQId qId,
		  IxQMgrQStatus *qStatus)
 *
 * @brief Fast get of a queue's status.
 *
 * This function is a streamlined version of ixQMgrQStatusGetWithChecks(), but
 * performs essentially the same task.  It reads the specified queue's status.
 * A queues status is defined by its status flags. For queues 0-31 these flags
 * are E,NE,NF,F,UF,OF. For queues 32-63 these flags are NE and F.
 *
 * @note - This function is inlined, to reduce unnecessary function call
 * overhead.  It does not perform any parameter checks, or update any
 * statistics.  Also, it does not check that the queue specified by qId has
 * been configured.  It simply reads the specified queue's status.
 *
 * @param IxQMgrQId(in) qId - the queue identifier.
 * @param IxQMgrQStatus(out) *qStatus - the status of the specified queue.
 *
 * @return @li void.
 *
 */
IX_QMGR_INLINE PUBLIC IX_STATUS
ixQMgrQStatusGet (IxQMgrQId qId,
		  IxQMgrQStatus *qStatus)
#ifdef NO_INLINE_APIS
    ;
#else
{
    /* read the status of a queue in the range 0-31 */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
	extern UINT32 ixQMgrAqmIfQueLowStatRegAddr[];
	extern UINT32 ixQMgrAqmIfQueLowStatBitsOffset[];
	extern UINT32 ixQMgrAqmIfQueLowStatBitsMask;
	extern UINT32 ixQMgrAqmIfQueUOStatRegAddr[];
	extern UINT32 ixQMgrAqmIfQueOflowStatBitMask[];
	extern UINT32 ixQMgrAqmIfQueUflowStatBitMask[];

	volatile UINT32 *lowStatRegAddr = (UINT32*)ixQMgrAqmIfQueLowStatRegAddr[qId];
	volatile UINT32 *qUOStatRegAddr = (UINT32*)ixQMgrAqmIfQueUOStatRegAddr[qId];
	UINT32 lowStatBitsOffset = ixQMgrAqmIfQueLowStatBitsOffset[qId];
	UINT32 lowStatBitsMask   = ixQMgrAqmIfQueLowStatBitsMask;
	UINT32 underflowBitMask  = ixQMgrAqmIfQueUflowStatBitMask[qId];
	UINT32 overflowBitMask   = ixQMgrAqmIfQueOflowStatBitMask[qId];

	/* read the status register for this queue */
	*qStatus = *lowStatRegAddr;
	/* mask out the status bits relevant only to this queue */
	*qStatus = (*qStatus >> lowStatBitsOffset) & lowStatBitsMask;

	/* Check if the queue has overflowed */
	if (*qUOStatRegAddr & overflowBitMask)
	{
	    /* clear the overflow status bit if it was set */
	    *qUOStatRegAddr &= ~overflowBitMask;
	    *qStatus |= IX_QMGR_Q_STATUS_OF_BIT_MASK;
	}

	/* Check if the queue has underflowed */
    	if (*qUOStatRegAddr & underflowBitMask)
	{
	    /* clear the underflow status bit if it was set */
	    *qUOStatRegAddr &= ~underflowBitMask;
	    *qStatus |= IX_QMGR_Q_STATUS_UF_BIT_MASK;
	}
    }
    else /* read status of a queue in the range 32-63 */
    {
	extern UINT32 ixQMgrAqmIfQueUppStat0RegAddr;
	extern UINT32 ixQMgrAqmIfQueUppStat1RegAddr;
	extern UINT32 ixQMgrAqmIfQueUppStat0BitMask[];
	extern UINT32 ixQMgrAqmIfQueUppStat1BitMask[];

	volatile UINT32 *qNearEmptyStatRegAddr = (UINT32*)ixQMgrAqmIfQueUppStat0RegAddr;
	volatile UINT32 *qFullStatRegAddr      = (UINT32*)ixQMgrAqmIfQueUppStat1RegAddr;
	int maskIndex = qId - IX_QMGR_MIN_QUEUPP_QID;
	UINT32 qNearEmptyStatBitMask = ixQMgrAqmIfQueUppStat0BitMask[maskIndex];
	UINT32 qFullStatBitMask      = ixQMgrAqmIfQueUppStat1BitMask[maskIndex];

	/* Reset the status bits */
	*qStatus = 0;

	/* Check if the queue is nearly empty */
	if (*qNearEmptyStatRegAddr & qNearEmptyStatBitMask)
	{
	    *qStatus |= IX_QMGR_Q_STATUS_NE_BIT_MASK;
	}

	/* Check if the queue is full */
	if (*qFullStatRegAddr & qFullStatBitMask)
	{
	    *qStatus |= IX_QMGR_Q_STATUS_F_BIT_MASK;
	}
    }
    return IX_SUCCESS;
}
#endif
/* ------------------------------------------------------------
   Queue dispatch related functions
   ---------------------------------------------------------- */

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrDispatcherPrioritySet (IxQMgrQId qId,
			     IxQMgrPriority priority)
 *
 * @brief Set the dispatch priority of a queue.
 *
 * This function is called to set the disptach priority of queue. The effect of
 * this function is to add a priority change request to a queue. This queue is
 * serviced by @a ixQMgrDispatcherLoopRun.
 *
 * This function is re-entrant. and can be used from an interrupt context
 *
 * @param IxQMgrQId(in) qId - the queue identifier
 * @param IxQMgrPriority(in) priority - the new queue dispatch priority
 *
 * @return @li IX_SUCCESS, priority change request is queued
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, the specified qId has not been configured
 * @return @li IX_QMGR_Q_INVALID_PRIORITY, specified priority is invalid
 *
 */
PUBLIC IX_STATUS
ixQMgrDispatcherPrioritySet (IxQMgrQId qId,
			     IxQMgrPriority priority);
/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrNotificationEnable (IxQMgrQId qId,
			  IxQMgrSourceId sourceId)
 *
 * @brief Enable notification on a queue for a specified queue source flag.
 *
 * This function is called by a client of the QMgr to enable notifications on a
 * specified condition.
 * If the condition for the notification is set after the client has called this
 * function but before the function has enabled the interrupt source, then the
 * notification will not occur.
 * For queues 32-63 the notification source is fixed to the NE(Nearly Empty) flag
 * and cannot be changed so the sourceId parameter is ignored for these queues.
 * The status register is read before the notofication is enabled and is read again
 * after the notification has been enabled, if they differ then the warning status
 * is returned.
 *
 * This function is re-entrant. and can be used from an interrupt context
 *
 * @param IxQMgrQId(in) qId - the queue identifier
 * @param IxQMgrSourceId(in) sourceId - the interrupt src condition identifier
 *
 * @return @li IX_SUCCESS, the interrupt has been enabled for the specified source
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, the specified qId has not been configured
 * @return @li IX_QMGR_INVALID_INT_SOURCE_ID, interrupt source invalid for this queue
 * @return @li IX_QMGR_WARNING, the status register may not be constistent
 *
 */
PUBLIC IX_STATUS
ixQMgrNotificationEnable (IxQMgrQId qId,
			  IxQMgrSourceId sourceId);

/**
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrNotificationDisable (IxQMgrQId qId)
 *
 * @brief Disable notifications on a queue.
 *
 * This function is called to disable notifications on a specified queue.
 *
 * This function is re-entrant. and can be used from an interrupt context
 *
 * @param IxQMgrQId(in) qId - the queue identifier
 *
 * @return @li IX_SUCCESS, the interrupt has been disabled
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, the specified qId has not been configured
 *
 */
PUBLIC IX_STATUS
ixQMgrNotificationDisable (IxQMgrQId qId);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrDispatcherLoopRun (IxQMgrDispatchGroup group)
 *
 * @brief Run the callback dispatcher.
 *
 * The function runs the dispatcher for a group of queues.
 * Callbacks are made for interrupts that have occurred on queues within
 * the group that have registered callbacks. The order in which queues are
 * serviced depends on the queue priorities set by the client.
 * This  function may be called from interrupt or task context.
 *
 * This function is not re-entrant.
 *
 * @param IxQMgrDispatchGroup(in) group - the group of queues over which the
 *                                        dispatcher will run
 *
 * @return @li void
 *
 */
PUBLIC void
ixQMgrDispatcherLoopRun (IxQMgrDispatchGroup group);

/**
 *
 * @ingroup IxQMgrAPI
 * 
 * @fn ixQMgrNotificationCallbackSet (IxQMgrQId qId,
			       IxQMgrCallback callback,
			       IxQMgrCallbackId callbackId)
 *
 * @brief Set the notification callback for a queue.
 *
 * This function sets the callback for the specified queue. This callback will
 * be called by the dispatcher, and may be called in the context of a interrupt
 * If callback has a value of NULL the previously registered callback, if one
 * exists will be unregistered.
 *
 * @param IxQMgrQId(in) qId - the queue idenfifier
 * @param IxQMgrCallback(in) callback - the callback registered for this queue
 * @param IxQMgrCallbackId(in) callbackid - the callback identifier
 *
 * @return @li IX_SUCCESS, the callback for the specified queue has been set
 * @return @li IX_QMGR_Q_NOT_CONFIGURED, the specified qId has not been configured
 *
 */
PUBLIC IX_STATUS
ixQMgrNotificationCallbackSet (IxQMgrQId qId,
			       IxQMgrCallback callback,
			       IxQMgrCallbackId callbackId);

#endif /* IXQMGR_H */

/**
 * @} defgroup IxQMgrAPI
 */
