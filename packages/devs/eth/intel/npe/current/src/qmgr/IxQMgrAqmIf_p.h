/**
 * @file    IxQMgrAqmIf_p.h
 *
 * @author Intel Corporation
 * @date    30-Oct-2001
 *
 * @brief   The IxQMgrAqmIf sub-component provides a number of inline
 * functions for performing I/O on the AQM. 
 *
 * Because  some functions contained in this module are inline and are
 * used in other modules (within the QMgr component) the definitions are
 * contained in this header file. The "normal" use of inline functions
 * is to use the inline functions in the module in which they are
 * defined. In this case these inline functions are used in external
 * modules and therefore the use of "inline extern". What this means
 * is as follows: if a function foo is declared as "inline extern"this
 * definition is only used for inlining, in no case is the function
 * compiled on its own. If the compiler cannot inline the function it
 * becomes an external reference. Therefore in IxQMgrAqmIf.c all
 * inline functions are defined without the "inline extern" specifier
 * and so define the external references. In all other modules these
 * funtions are defined as "inline extern".
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

#ifndef IXQMGRAQMIF_P_H
#define IXQMGRAQMIF_P_H

/*
 * inline definition
 */
#ifdef IXQMGRAQMIF_C
#ifndef IX_QMGR_AQMIF_INLINE
#define IX_QMGR_AQMIF_INLINE
#endif
#else
#ifndef IX_QMGR_AQMIF_INLINE
#define IX_QMGR_AQMIF_INLINE __inline__ extern
#endif
#endif

/*
 * User defined include files.
 */
#include "IxQMgr.h"
#include "IxQMgrLog_p.h"
#include "IxQMgrQCfg_p.h"

/*
 * #defines and macros used in this file.
 */

/* Number of bytes per word */
#define IX_QMGR_NUM_BYTES_PER_WORD 4

/* Underflow bit mask  */
#define IX_QMGR_UNDERFLOW_BIT_OFFSET    0x0

/* Overflow bit mask */
#define IX_QMGR_OVERFLOW_BIT_OFFSET     0x1

/* Queue access register, queue 0 */
#define IX_QMGR_QUEACC0_OFFSET      0x0000

/* Size of queue access register in words */
#define IX_QMGR_QUEACC_SIZE         0x4/*words*/

/* Queue status register, queues 0-7 */
#define IX_QMGR_QUELOWSTAT0_OFFSET  (IX_QMGR_QUEACC0_OFFSET +\
(IX_QMGR_MAX_NUM_QUEUES * IX_QMGR_QUEACC_SIZE * IX_QMGR_NUM_BYTES_PER_WORD))

/* Queue status register, queues 8-15 */
#define IX_QMGR_QUELOWSTAT1_OFFSET  (IX_QMGR_QUELOWSTAT0_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue status register, queues 16-23 */
#define IX_QMGR_QUELOWSTAT2_OFFSET  (IX_QMGR_QUELOWSTAT1_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue status register, queues 24-31 */
#define IX_QMGR_QUELOWSTAT3_OFFSET  (IX_QMGR_QUELOWSTAT2_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue status register Q status bits mask */
#define IX_QMGR_QUELOWSTAT_QUE_STS_BITS_MASK 0xF

/* Size of queue 0-31 status register */
#define IX_QMGR_QUELOWSTAT_SIZE     0x4 /*words*/

/* The number of queues' status specified per word */
#define IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD 0x8

/* Queue UF/OF status register queues 0-15  */
#define IX_QMGR_QUEUOSTAT0_OFFSET   (IX_QMGR_QUELOWSTAT3_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)
/* Queue UF/OF status register queues 16-31 */
#define IX_QMGR_QUEUOSTAT1_OFFSET   (IX_QMGR_QUEUOSTAT0_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* The number of queues' underflow/overflow status specified per word */
#define IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD 0x10

/* Queue NE status register, queues 32-63   */
#define IX_QMGR_QUEUPPSTAT0_OFFSET  (IX_QMGR_QUEUOSTAT1_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue F status register, queues 32-63    */
#define IX_QMGR_QUEUPPSTAT1_OFFSET  (IX_QMGR_QUEUPPSTAT0_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Size of queue 32-63 status register */
#define IX_QMGR_QUEUPPSTAT_SIZE     0x2 /*words*/

/* The number of queues' status specified per word */
#define IX_QMGR_QUEUPPSTAT_NUM_QUE_PER_WORD 0x20

/* Queue INT source select register, queues 0-7   */
#define IX_QMGR_INT0SRCSELREG0_OFFSET (IX_QMGR_QUEUPPSTAT1_OFFSET   +\
                                       IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue INT source select register, queues 8-15  */
#define IX_QMGR_INT0SRCSELREG1_OFFSET (IX_QMGR_INT0SRCSELREG0_OFFSET+\
                                       IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue INT source select register, queues 16-23 */
#define IX_QMGR_INT0SRCSELREG2_OFFSET (IX_QMGR_INT0SRCSELREG1_OFFSET+\
                                       IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue INT source select register, queues 24-31 */
#define IX_QMGR_INT0SRCSELREG3_OFFSET (IX_QMGR_INT0SRCSELREG2_OFFSET+\
                                       IX_QMGR_NUM_BYTES_PER_WORD)

/* Size of interrupt source select reegister */
#define IX_QMGR_INT0SRCSELREG_SIZE  0x4 /*words*/

/* The number of queues' interrupt source select specified per word*/
#define IX_QMGR_INTSRC_NUM_QUE_PER_WORD 0x8

/* Queue INT enable register, queues 0-31  */
#define IX_QMGR_QUEIEREG0_OFFSET    (IX_QMGR_INT0SRCSELREG3_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue INT enable register, queues 32-63 */
#define IX_QMGR_QUEIEREG1_OFFSET    (IX_QMGR_QUEIEREG0_OFFSET      +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue INT register, queues 0-31  */
#define IX_QMGR_QINTREG0_OFFSET     (IX_QMGR_QUEIEREG1_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue INT register, queues 32-63 */
#define IX_QMGR_QINTREG1_OFFSET     (IX_QMGR_QINTREG0_OFFSET  +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Size of interrupt register */
#define IX_QMGR_QINTREG_SIZE        0x2 /*words*/

/* Number of queues' status specified per word */
#define IX_QMGR_QINTREG_NUM_QUE_PER_WORD 0x20

/* Number of bits per queue interrupt status */
#define IX_QMGR_QINTREG_BITS_PER_QUEUE 0x1
#define IX_QMGR_QINTREG_BIT_OFFSET 0x1

/* Size of address space not used by AQM */
#define IX_QMGR_AQM_UNUSED_ADDRESS_SPACE_SIZE_IN_BYTES  0x1bC0

/* Queue config register, queue 0 */
#define IX_QMGR_QUECONFIG_BASE_OFFSET (IX_QMGR_QINTREG1_OFFSET +\
                             IX_QMGR_NUM_BYTES_PER_WORD +\
                             IX_QMGR_AQM_UNUSED_ADDRESS_SPACE_SIZE_IN_BYTES)

/* Total size of configuration words */
#define IX_QMGR_QUECONFIG_SIZE      0x100

/* Start of SRAM queue buffer space */
#define IX_QMGR_QUEBUFFER_SPACE_OFFSET (IX_QMGR_QUECONFIG_BASE_OFFSET +\
                                 IX_QMGR_MAX_NUM_QUEUES * IX_QMGR_NUM_BYTES_PER_WORD)

/* Total bits in a word */
#define BITS_PER_WORD 32

/* Size of queue buffer space */
#define IX_QMGR_QUE_BUFFER_SPACE_SIZE 0x1F00

/*
 * This macro will return the address of the access register for the
 * queue  specified by qId
 */
#define IX_QMGR_Q_ACCESS_ADDR_GET(qId)\
        (((qId) * (IX_QMGR_QUEACC_SIZE * IX_QMGR_NUM_BYTES_PER_WORD))\
	 + IX_QMGR_QUEACC0_OFFSET)

/*
 * Variable declerations global to this file. Externs are followed by
 * statics.
 */
extern UINT32 aqmBaseAddress;

/*
 * Function declarations.
 */
void
ixQMgrAqmIfInit (void);

unsigned
ixQMgrAqmIfLog2 (unsigned number);

void
ixQMgrAqmIfQRegisterBitsWrite (IxQMgrQId qId, 
			       UINT32 registerBaseAddrOffset,
			       unsigned queuesPerRegWord,
			       UINT32 value);




void
ixQMgrAqmIfQStatusCheckValsCalc (IxQMgrQId qId,
				 IxQMgrSourceId srcSel,
				 unsigned int *statusWordOffset,
				 UINT32 *checkValue,
				 UINT32 *mask);

/*
 * The Xscale software allways deals with logical addresses and so the
 * base address of the AQM memory space is not a hardcoded value. This
 * function must be called before any other function in this component.
 * NO CHECKING is performed to ensure that the base address has been
 * set.
 */
void
ixQMgrAqmIfBaseAddressSet (UINT32 address);

/*
 * Get the base address of the AQM memory space.
 */
void
ixQMgrAqmIfBaseAddressGet (UINT32 *address);

/*
 * Get the sram base address
 */
void
ixQMgrAqmIfSramBaseAddressGet (UINT32 *address);

/*
 * Read a queue status
 */
void
ixQMgrAqmIfQueStatRead (IxQMgrQId qId,
			IxQMgrQStatus* status);

/*
 * Read the AQM interrupt register
 */
void
ixQMgrAqmIfQInterruptRegRead (IxQMgrDispatchGroup group, 
			      UINT32 *regVal);

/*
 * Set the interrupt source
 */
void
ixQMgrAqmIfIntSrcSelWrite (IxQMgrQId qId,
			   IxQMgrSourceId sourceId);

/*
 * Enable interruptson a queue
 */
void
ixQMgrAqmIfQInterruptEnable (IxQMgrQId qId);

/*
 * Disable interrupt on a quee
 */
void
ixQMgrAqmIfQInterruptDisable (IxQMgrQId qId);

/*
 * This function will read the specified queues' config register and
 * extract the read and write pointers.
 */
void
ixQMgrAqmIfQReadWritePointerGet (IxQMgrQId qId,
				 UINT32 **readPtr,
				 UINT32 **writePtr);

/*
 * Write the config register of the specified queue
 */
void
ixQMgrAqmIfQueCfgWrite (IxQMgrQId qId,
			IxQMgrQSizeInWords qSizeInWords,
			IxQMgrQEntrySizeInWords entrySizeInWords,
			UINT32 freeSRAMAddress);

/*
 * read fields from the config of the specified queue.
 */
void
ixQMgrAqmIfQueCfgRead (IxQMgrQId qId,
		       unsigned int numEntries,
		       UINT32 *baseAddress,
		       unsigned int *ne,
		       unsigned int *nf,
		       UINT32 *readPtr,
		       UINT32 *writePtr);

/*
 * Set the ne and nf watermark level on a queue.
 */
void
ixQMgrAqmIfWatermarkSet (IxQMgrQId qId,
			 unsigned ne,
			 unsigned nf);

/* Inspect an entry without moving the read pointer */
IX_STATUS
ixQMgrAqmIfQPeek (IxQMgrQId qId,
		  unsigned int entryIndex,
		  unsigned int *entry);

/* Modify an entry without moving the write pointer */
IX_STATUS
ixQMgrAqmIfQPoke (IxQMgrQId qId,
		  unsigned int entryIndex,
		  unsigned int *entry);

/*
 * Inline functions
 */

/*
 * This inline function is used by other QMgr components to write one
 * word to the specified address.
 */
IX_QMGR_AQMIF_INLINE void
ixQMgrAqmIfWordWrite (VUINT32 *address,
		      UINT32 word)
{
    *address = word;
}

/*
 * This inline function is used by other QMgr components to read a
 * word from the specified address.
 */
IX_QMGR_AQMIF_INLINE void
ixQMgrAqmIfWordRead (VUINT32 *address,
		     UINT32 *word)
{
    *word = *address;
}

/*
 * This inline function is used by other QMgr components to pop an
 * entry off the specified queue.
 */
IX_QMGR_AQMIF_INLINE void
ixQMgrAqmIfQPop (IxQMgrQId qId,
		 IxQMgrQEntrySizeInWords numWords,
		 UINT32 *entry)
{
    volatile UINT32 *accRegAddr;

    accRegAddr = (UINT32*)(aqmBaseAddress +
    			   IX_QMGR_Q_ACCESS_ADDR_GET(qId));

    switch (numWords)
    {
	case IX_QMGR_Q_ENTRY_SIZE1:
	    ixQMgrAqmIfWordRead (accRegAddr, entry);
	    break;
	case IX_QMGR_Q_ENTRY_SIZE2:
	    ixQMgrAqmIfWordRead (accRegAddr++, entry++);
	    ixQMgrAqmIfWordRead (accRegAddr, entry);
	    break;
	case IX_QMGR_Q_ENTRY_SIZE4:
	    ixQMgrAqmIfWordRead (accRegAddr++, entry++);
	    ixQMgrAqmIfWordRead (accRegAddr++, entry++);
	    ixQMgrAqmIfWordRead (accRegAddr++, entry++);
	    ixQMgrAqmIfWordRead (accRegAddr, entry);
	    break;
	default:
	    IX_QMGR_LOG_ERROR0("Invalid Q Entry size passed to ixQMgrAqmIfQPop");
	    break;
    }
}

/*
 * This inline function is used by other QMgr components to push an
 * entry to the specified queue.
 */
IX_QMGR_AQMIF_INLINE void
ixQMgrAqmIfQPush (IxQMgrQId qId,
		  IxQMgrQEntrySizeInWords numWords,
		  UINT32 *entry)
{
    volatile UINT32 *accRegAddr;

    accRegAddr = (UINT32*)(aqmBaseAddress +
    			   IX_QMGR_Q_ACCESS_ADDR_GET(qId));
    
    switch (numWords)
    {
	case IX_QMGR_Q_ENTRY_SIZE1:
	    ixQMgrAqmIfWordWrite (accRegAddr, *entry);
	    break;
	case IX_QMGR_Q_ENTRY_SIZE2:
	    ixQMgrAqmIfWordWrite (accRegAddr++, *entry++);
	    ixQMgrAqmIfWordWrite (accRegAddr, *entry);
	    break;
	case IX_QMGR_Q_ENTRY_SIZE4:
	    ixQMgrAqmIfWordWrite (accRegAddr++, *entry++);
	    ixQMgrAqmIfWordWrite (accRegAddr++, *entry++);
	    ixQMgrAqmIfWordWrite (accRegAddr++, *entry++);
	    ixQMgrAqmIfWordWrite (accRegAddr, *entry);
	    break;
	default:
	    IX_QMGR_LOG_ERROR0("Invalid Q Entry size passed to ixQMgrAqmIfQPush");
	    break;
    }
}

/*
 * The AQM interrupt registers contains a bit for each AQM queue
 * specifying the queue (s) that cause an interrupt to fire. This
 * function is called by IxQMGrDispatcher component.
 */
IX_QMGR_AQMIF_INLINE void
ixQMgrAqmIfQStatusRegsRead (IxQMgrDispatchGroup group, 
			    UINT32 *qStatusWords)
{
    volatile UINT32 *regAddress = NULL;

    if (group == IX_QMGR_QUELOW_GROUP)
    {
	regAddress = (UINT32*)(aqmBaseAddress +
			       IX_QMGR_QUELOWSTAT0_OFFSET);

	ixQMgrAqmIfWordRead (regAddress++, qStatusWords++);
	ixQMgrAqmIfWordRead (regAddress++, qStatusWords++);
	ixQMgrAqmIfWordRead (regAddress++, qStatusWords++);
	ixQMgrAqmIfWordRead (regAddress, qStatusWords);
    }
    else /* We have the upper queues */
    {
       /* Only need to read the Nearly Empty status register for
	* queues 32-63 as for therse queues the interrtupt source
	* condition is fixed to Nearly Empty
	*/
	regAddress = (UINT32*)(aqmBaseAddress +
			       IX_QMGR_QUEUPPSTAT0_OFFSET);
	ixQMgrAqmIfWordRead (regAddress, qStatusWords);
    }
}


/*
 * This function check if the status for a queue has changed between
 * 2 snapshots and if it has, that the status matches a particular
 * value after masking.
 */
IX_QMGR_AQMIF_INLINE BOOL
ixQMgrAqmIfQStatusCheck (UINT32 *oldQStatusWords,
			 UINT32 *newQStatusWords,
			 unsigned int statusWordOffset,			 
			 UINT32 checkValue,
			 UINT32 mask)
{
    if (((oldQStatusWords[statusWordOffset] & mask) != 
	 (newQStatusWords[statusWordOffset] & mask)) &&
	((newQStatusWords[statusWordOffset] & mask) == checkValue))
    {
	return TRUE;
    }

    return FALSE;
}

/*
 * The AQM interrupt register contains a bit for each AQM queue
 * specifying the queue (s) that cause an interrupt to fire. This
 * function is called by IxQMgrDispatcher component.
 */
IX_QMGR_AQMIF_INLINE void
ixQMgrAqmIfQInterruptRegRead (IxQMgrDispatchGroup group, 
			      UINT32 *regVal)
{
    volatile UINT32 *regAddress;

    if (group == IX_QMGR_QUELOW_GROUP)
    {
	regAddress = (UINT32*)(aqmBaseAddress +
			       IX_QMGR_QINTREG0_OFFSET);
    }
    else
    {
	regAddress = (UINT32*)(aqmBaseAddress +
			       IX_QMGR_QINTREG1_OFFSET);
    }

    ixQMgrAqmIfWordRead (regAddress, regVal);
}

/*
 * This inline function will read the status bit of a queue
 * specified by qId. If reset is TRUE the bit is cleared.
 */
IX_QMGR_AQMIF_INLINE BOOL
ixQMgrAqmIfRegisterBitCheck (IxQMgrQId qId, 
			     UINT32 registerBaseAddrOffset,
			     unsigned queuesPerRegWord,
			     unsigned relativeBitOffset,
			     BOOL reset)
{
    UINT32 actualBitOffset;
    volatile UINT32 *registerAddress;
    UINT32 registerWord;

    /*
     * Calculate the registerAddress
     * multiple queues split accross registers
     */
    registerAddress = (UINT32*)(aqmBaseAddress +
    				registerBaseAddrOffset +
    				((qId / queuesPerRegWord) *
    				 IX_QMGR_NUM_BYTES_PER_WORD));

    /*
     * Get the status word
     */
    ixQMgrAqmIfWordRead (registerAddress, &registerWord);
    
    /*
     * Calculate the actualBitOffset
     * status for multiple queues stored in one register
     */
    actualBitOffset = (relativeBitOffset + 1) <<
	((qId & (queuesPerRegWord - 1)) * (BITS_PER_WORD / queuesPerRegWord));

    /* Check if the status bit is set */
    if (registerWord & actualBitOffset)
    {
	/* Clear the bit if reset */
	if (reset)
	{
	    ixQMgrAqmIfWordWrite (registerAddress, registerWord & (~actualBitOffset));
	}
	return TRUE;
    }

    /* Bit not set */
    return FALSE;
}


/*
 * @ingroup IxQmgrAqmIfAPI
 *
 * @brief Read the underflow status of a queue 
 *
 * This inline function will read the underflow status of a queue
 * specified by qId.
 * 
 */
IX_QMGR_AQMIF_INLINE BOOL
ixQMgrAqmIfUnderflowCheck (IxQMgrQId qId)
{
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
	return (ixQMgrAqmIfRegisterBitCheck (qId,
					     IX_QMGR_QUEUOSTAT0_OFFSET,
					     IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD,
					     IX_QMGR_UNDERFLOW_BIT_OFFSET,
					     TRUE/*reset*/));
    }
    else
    {
	/* Qs 32-63 have no underflow status */
	return FALSE;
    }
}

/*
 * This inline function will read the overflow status of a queue
 * specified by qId.
 */
IX_QMGR_AQMIF_INLINE BOOL
ixQMgrAqmIfOverflowCheck (IxQMgrQId qId)
{
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
	return (ixQMgrAqmIfRegisterBitCheck (qId,
					     IX_QMGR_QUEUOSTAT0_OFFSET,
					     IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD,
					     IX_QMGR_OVERFLOW_BIT_OFFSET,
					     TRUE/*reset*/));
    }
    else
    {
	/* Qs 32-63 have no overflow status */
	return FALSE;
    }
}

/*
 * This inline function will read the status bits of a queue
 * specified by qId.
 */
IX_QMGR_AQMIF_INLINE UINT32
ixQMgrAqmIfQRegisterBitsRead (IxQMgrQId qId, 
			      UINT32 registerBaseAddrOffset,
			      unsigned queuesPerRegWord)
{
    volatile UINT32 *registerAddress;
    UINT32 registerWord;
    UINT32 statusBitsMask;
    UINT32 bitsPerQueue;

    bitsPerQueue = BITS_PER_WORD / queuesPerRegWord;

    /*
     * Calculate the registerAddress
     * multiple queues split accross registers
     */
    registerAddress = (UINT32*)(aqmBaseAddress +
				registerBaseAddrOffset +
				((qId / queuesPerRegWord) *
				 IX_QMGR_NUM_BYTES_PER_WORD));
    /*
     * Read the status word
     */
    ixQMgrAqmIfWordRead (registerAddress, &registerWord);
    

    /*
     * Calculate the mask for the status bits for this queue.
     */
    statusBitsMask = ((1 << bitsPerQueue) - 1);

    /*
     * Shift the status word so it is right justified
     */    
    registerWord >>= ((qId & (queuesPerRegWord - 1)) * bitsPerQueue);

    /*
     * Mask out all bar the status bits for this queue
     */
    return (registerWord &= statusBitsMask);
}

/*
 * This function is called by IxQMgrDispatcher to set the contents of
 * the AQM interrupt register.
 */
IX_QMGR_AQMIF_INLINE void
ixQMgrAqmIfQInterruptRegWrite (IxQMgrDispatchGroup group, 
			       UINT32 reg)
{
    volatile UINT32 *address;

    if (group == IX_QMGR_QUELOW_GROUP)
    {
	address = (UINT32*)(aqmBaseAddress +
			    IX_QMGR_QINTREG0_OFFSET);
    }
    else
    {
	address = (UINT32*)(aqmBaseAddress +
			    IX_QMGR_QINTREG1_OFFSET);
    }

    ixQMgrAqmIfWordWrite (address, reg);
}

/*
 * Read the status of a queue in the range 0-31.
 *
 * This function is used by other QMgr components to read the
 * status of the queue specified by qId.
 */
IX_QMGR_AQMIF_INLINE void
ixQMgrAqmIfQueLowStatRead (IxQMgrQId qId,
			   IxQMgrQStatus *status)
{
    /* Read the general status bits */
    *status = ixQMgrAqmIfQRegisterBitsRead (qId,
					    IX_QMGR_QUELOWSTAT0_OFFSET,
					    IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD);

    /* Check if the queue has overflowed */
    if (ixQMgrAqmIfOverflowCheck (qId))
    {
	*status |= IX_QMGR_Q_STATUS_OF_BIT_MASK;
    }

    /* Check if the queue has underflowed */
    if (ixQMgrAqmIfUnderflowCheck (qId))
    {
	*status |= IX_QMGR_Q_STATUS_UF_BIT_MASK;
    }
}

/*
 * This function will read the status of the queue specified
 * by qId.
 */
IX_QMGR_AQMIF_INLINE void
ixQMgrAqmIfQueUppStatRead (IxQMgrQId qId,
			   IxQMgrQStatus *status)
{
    /* Reset the status bits */
    *status = 0;

    /* 
     * Check if the queue is nearly empty,
     * N.b. QUPP stat register contains status for regs 32-63 at each
     *      bit position so subtract 32 to get bit offset
     */
    if (ixQMgrAqmIfRegisterBitCheck ((qId - IX_QMGR_MIN_QUEUPP_QID),
				     IX_QMGR_QUEUPPSTAT0_OFFSET,
				     IX_QMGR_QUEUPPSTAT_NUM_QUE_PER_WORD,
				     0/*relativeBitOffset*/,
				     FALSE/*!reset*/))
    {
	*status |= IX_QMGR_Q_STATUS_NE_BIT_MASK;
    }

    /* 
     * Check if the queue is full,
     * N.b. QUPP stat register contains status for regs 32-63 at each
     *      bit position so subtract 32 to get bit offset
     */
    if (ixQMgrAqmIfRegisterBitCheck ((qId - IX_QMGR_MIN_QUEUPP_QID),
				     IX_QMGR_QUEUPPSTAT1_OFFSET,
				     IX_QMGR_QUEUPPSTAT_NUM_QUE_PER_WORD,
				     0/*relativeBitOffset*/,
				     FALSE/*!reset*/))
    {
	*status |= IX_QMGR_Q_STATUS_F_BIT_MASK;
    }
}

/*
 * This function is used by other QMgr components to read the
 * status of the queue specified by qId.
 */
IX_QMGR_AQMIF_INLINE void
ixQMgrAqmIfQueStatRead (IxQMgrQId qId, 
			IxQMgrQStatus *qStatus)
{
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
	ixQMgrAqmIfQueLowStatRead (qId, qStatus);
    }
    else
    {
	ixQMgrAqmIfQueUppStatRead (qId, qStatus);
    }
}


/*
 * This function performs a mod division
 */
IX_QMGR_AQMIF_INLINE unsigned
ixQMgrAqmIfPow2NumDivide (unsigned numerator, 
			  unsigned denominator)
{
    /* Number is evenly divisable by 2 */
    return (numerator >> ixQMgrAqmIfLog2 (denominator));
}

#endif/*IXQMGRAQMIF_P_H*/
