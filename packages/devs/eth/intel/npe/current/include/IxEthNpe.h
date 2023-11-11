/**
 * @file IxEthNpe.h
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
 * @defgroup IxEthNpe IXP4xx Ethernet NPE (IxEthNpe) API
 *
 * @brief Contains the API for Ethernet NPE
 *
 * @{
 */

/*--------------------------------------------------------------------------
 * APB Message IDs - XScale->NPE
 *------------------------------------------------------------------------*/

/**
 * @def IX_ETHNPE_X2P_NPE_HALT                  
 *
 * @brief Request from the XScale client for the NPE to immediately halt all 
 * execution and flush any mbufs in its possession 
 *
 * Any free mbuf held by the NPE receive process is flushed to the RxEnet
 * queue. Transmit path mbufs and FastPath descriptors (those in the 
 * Priority Queue and any one currently in the process of transmission) 
 * are immediately flushed to either the TxEnetDone queue or the FreeFP 
 * queue, depending on their point of origin. When an X2P_NPE_Halt message 
 * is issued, there must be no pending FastPath VC shutdowns.  Following a 
 * halt operation, a NPE may be brought into the "ready" state only by 
 * downloading the firmware again.
 */
#define IX_ETHNPE_X2P_NPE_HALT                  0x00

/**
 * @def IX_ETHNPE_X2P_ELT_SETPORTADDRESS
 *
 * @brief Indication from the XScale client that the attached Ethernet port's 
 * MAC address is equal to the speci-fied value and that the port ID of attached 
 * Ethernet port should be set to the specified value. 
 */
#define IX_ETHNPE_X2P_ELT_SETPORTADDRESS        0x10

/**
 * @def IX_ETHNPE_X2P_ELT_ACCESSREQUEST
 *
 * @brief Request from the XScale client for the NPE to relinquish control of the 
 * Ethernet Learning Tree and write it back to external memory (at the location 
 * specified in the last X2P_ELT_AccessRelease message). 
 */
#define IX_ETHNPE_X2P_ELT_ACCESSREQUEST         0x11

/**
 * @def IX_ETHNPE_X2P_ELT_ACCESSRELEASE
 *
 * @brief Indication from the XScale client that it has relinquished control of the 
 * Ethernet Learning Tree and has written an updated version of it, with its base 
 * node at the specified address (the base node is the empty node immediately 
 * preceding the true root node). 
 *
 * The tree will remain at the same location until the next X2P_ELT_AccessRelease 
 * message.
 */
#define IX_ETHNPE_X2P_ELT_ACCESSRELEASE         0x12

/**
 * @def IX_ETHNPE_X2P_ELT_INSERTADDRESS         
 *
 * @brief Indication from the XScale client that the NPE should insert the specified 
 * MAC address/Port ID into internal tree.
 */
#define IX_ETHNPE_X2P_ELT_INSERTADDRESS         0x13

/**
 * @def IX_ETHNPE_X2P_FP_SETETHERNETTYPE        
 *
 * @brief Indication from the XScale client that the universal FastPath Ethernet type 
 * should be set to the specified value. 
 *
 * This message is useful only in a "single stack" (IPv4 or IPv6) application. The 
 * Ethernet type will be template-specific in future Ethernet NPE firmware releases, 
 * in order to support "mixed stack" (IPv4 and IPv6) applications.
 */
#define IX_ETHNPE_X2P_FP_SETETHERNETTYPE        0x20

/**
 * @def IX_ETHNPE_X2P_FP_WRITETEMPMDF   
 *
 * @brief Indication from the XScale client that the specified modification template
 * should be loaded into the NPE modification template array at the specified index.
 */
#define IX_ETHNPE_X2P_FP_WRITETEMPMDF           0x21

/**
 * @def IX_ETHNPE_X2P_STATS_SHOW
 *
 * @brief Request from the XScale client for the current MAC port statistics data to be 
 * written to the (empty) statistics structure and the specified location in external 
 * memory.
 */
#define IX_ETHNPE_X2P_STATS_SHOW                0x30

/**
 * @def IX_ETHNPE_X2P_STATS_RESET
 *
 * @brief Request from the XScale client for the NPE to reset all of its internal 
 * MAC port statistics state variables. 
 *
 * As a side effect, this message entails an implicit request that the NPE write the 
 * current MAC port statistics into the MAC statistics structure at the specified 
 * location in external memory.
 */
#define IX_ETHNPE_X2P_STATS_RESET               0x31

/*--------------------------------------------------------------------------
 * APB Message IDs - NPE->XScale
 *------------------------------------------------------------------------*/

/**
 * @def IX_ETHNPE_P2X_NPE_STATUS                
 *
 * @brief Indication from the NPE of its current status.
 */
#define IX_ETHNPE_P2X_NPE_STATUS                0x00

/**
 * @def IX_ETHNPE_P2X_ELT_ACKPORTADDRESS        
 *
 * @brief Indication from the NPE that is has finished processing the previous 
 * X2P_ELT_SetPortAddress message.
 */
#define IX_ETHNPE_P2X_ELT_ACKPORTADDRESS        0x10

/**
 * @def IX_ETHNPE_P2X_ELT_ACCESSGRANT           
 *
 * @brief Indication from the NPE that it relinquished control of the Ethernet 
 * Learning Tree and has written it back to external memory at the specified base
 *  address.
 */
#define IX_ETHNPE_P2X_ELT_ACCESSGRANT           0x11

/**
 * @def IX_ETHNPE_P2X_ELT_BALANCEREQUEST        
 *
 * @brief Request from the NPE for the XScale client to insert the specified MAC 
 * address into the Ethernet Learning Tree and rebalance it (the NPE has run out of 
 * depth while attempting to insert the source MAC address itself). 
 * 
 * A P2X_ELT_AccessGrant message is implied (i.e. the NPE will have relinquished 
 * control of the tree and written it back to external memory prior to issuing this 
 * message).
 */
#define IX_ETHNPE_P2X_ELT_BALANCEREQUEST        0x12

/**
 * @def IX_ETHNPE_P2X_ELT_NEWADDRESS            
 *
 * @brief Indication from the NPE that it has just learned (i.e. inserted into its 
 * internal tree) the specified new MAC address.
 */
#define IX_ETHNPE_P2X_ELT_NEWADDRESS            0x13

/**
 * @def IX_ETHNPE_P2X_ELT_INSERTADDRESSACK      
 *
 * @brief Indication from the NPE that it has successfully enqueued (to the learning 
 * process) the MAC address from the previous X2P_ELT_Insert_Address message.
 */
#define IX_ETHNPE_P2X_ELT_INSERTADDRESSACK      0x14

/**
 * @def IX_ETHNPE_P2X_ELT_INSERTADDRESSNACK     
 *
 * @brief Indication from the NPE that it is unable to enqueue (to the learning 
 * process) the MAC address from the previous X2P_ELT_Insert_Address message.
 */
#define IX_ETHNPE_P2X_ELT_INSERTADDRESSNACK     0x15

/**
 * @def IX_ETHNPE_P2X_FP_WRITETEMPMDFACK        
 *
 * @brief Indication from the NPE that it is finished copying the FastPath modification 
 * template for the specified VC at the specified address.
 */
#define IX_ETHNPE_P2X_FP_WRITETEMPMDFACK        0x20

/**
 * @def IX_ETHNPE_P2X_FP_SHUTDOWNVCACK          
 *
 * @brief Indication from the NPE that it has processed a special VC shutdown 
 * description for the VC with the specified FastPath index.
 */
#define IX_ETHNPE_P2X_FP_SHUTDOWNVCACK          0x21

/**
 * @def IX_ETHNPE_P2X_STATS_REPORT              
 *
 * @brief Indication from the NPE that the current MAC port statistics are available
 * in the specified buffer.
 */
#define IX_ETHNPE_P2X_STATS_REPORT              0x30

/**
 * @def IX_ETHNPE_P2X_STATS_CLEAR_REPORT          
 *
 * @brief Indication from the NPE that the current MAC port statistics are cleared 
 */
#define IX_ETHNPE_P2X_STATS_CLEAR_REPORT        0x31


/*--------------------------------------------------------------------------
 * Queue Manager Queue entry bit field boundary definitions
 *------------------------------------------------------------------------*/

/**
 * @def MASK(hi,lo)
 *
 * @brief Macro for mask
 */
#define MASK(hi,lo)                    ((1 << ((hi) + 1)) - (1 << (lo)))

/**
 * @def BITS(x,hi,lo)
 *
 * @brief Macro for bits
 */
#define BITS(x,hi,lo)                  (((x) & MASK(hi,lo)) >> (lo))

/**
 * @def IX_ETHNPE_QM_Q_FIELD_NPEID_L
 *
 * @brief QMgr Queue NPE ID field left boundary
 */
#define IX_ETHNPE_QM_Q_FIELD_NPEID_L            31

/**
 * @def IX_ETHNPE_QM_Q_FIELD_NPEID_R
 *
 * @brief QMgr Queue NPE ID field right boundary
 */
#define IX_ETHNPE_QM_Q_FIELD_NPEID_R            31

/**
 * @def IX_ETHNPE_QM_Q_FIELD_FPBIT_L
 *
 * @brief QMgr Queue Fast Path bit field left boundary
 */
#define IX_ETHNPE_QM_Q_FIELD_FPBIT_L            31

/**
 * @def IX_ETHNPE_QM_Q_FIELD_FPBIT_R
 *
 * @brief QMgr Queue Fast Path bit field right boundary
 */
#define IX_ETHNPE_QM_Q_FIELD_FPBIT_R            31

/**
 * @def IX_ETHNPE_QM_Q_FIELD_PORTID_L
 *
 * @brief QMgr Queue Port ID field left boundary
 */
#define IX_ETHNPE_QM_Q_FIELD_PORTID_L           30

/**
 * @def IX_ETHNPE_QM_Q_FIELD_PORTID_R
 *
 * @brief QMgr Queue Port ID field right boundary
 */
#define IX_ETHNPE_QM_Q_FIELD_PORTID_R           28

/**
 * @def IX_ETHNPE_QM_Q_FIELD_PRIOR_L
 *
 * @brief QMgr Queue Priority field left boundary
 */
#define IX_ETHNPE_QM_Q_FIELD_PRIOR_L            30

/**
 * @def IX_ETHNPE_QM_Q_FIELD_PRIOR_R
 *
 * @brief QMgr Queue Priority field right boundary
 */
#define IX_ETHNPE_QM_Q_FIELD_PRIOR_R            28

/**
 * @def IX_ETHNPE_QM_Q_FIELD_ADDR_L 
 *
 * @brief QMgr Queue Address field left boundary
 */
#define IX_ETHNPE_QM_Q_FIELD_ADDR_L             27

/**
 * @def IX_ETHNPE_QM_Q_FIELD_ADDR_R
 *
 * @brief QMgr Queue Address field right boundary
 */
#define IX_ETHNPE_QM_Q_FIELD_ADDR_R              2

/**
 * @def IX_ETHNPE_QM_Q_FIELD_FPINDEX_L 
 *
 * @brief QMgr Queue Fast Path index field left boundary
 */
#define IX_ETHNPE_QM_Q_FIELD_FPINDEX_L          15

/**
 * @def IX_ETHNPE_QM_Q_FIELD_FPINDEX_R
 *
 * @brief QMgr Queue Fast Path index field right boundary
 */
#define IX_ETHNPE_QM_Q_FIELD_FPINDEX_R           1

/**
 * @def IX_ETHNPE_QM_Q_FIELD_FPSHTDN_R
 *
 * @brief QMgr Queue Fast Path shutdown field right boundary
 */
#define IX_ETHNPE_QM_Q_FIELD_FPSHTDN_L           0

/**
 * @def IX_ETHNPE_QM_Q_FIELD_FPSHTDN_R
 *
 * @brief QMgr Queue Fast Path shutdown field right boundary
 */
#define IX_ETHNPE_QM_Q_FIELD_FPSHTDN_R           0

/*--------------------------------------------------------------------------
 * Queue Manager Queue entry bit field masks
 *------------------------------------------------------------------------*/

/**
 * @def IX_ETHNPE_QM_Q_FREEENET_ADDR_MASK 
 *
 * @brief Macro to mask the Address field of the FreeEnet Queue Manager Entry
 */
#define IX_ETHNPE_QM_Q_FREEENET_ADDR_MASK \
            MASK (IX_ETHNPE_QM_Q_FIELD_ADDR_L, \
                  IX_ETHNPE_QM_Q_FIELD_ADDR_R)

/**
 * @def IX_ETHNPE_QM_Q_RXENET_NPEID_MASK  
 *
 * @brief Macro to mask the NPE ID field of the RxEnet Queue Manager Entry
 */
#define IX_ETHNPE_QM_Q_RXENET_NPEID_MASK \
            MASK (IX_ETHNPE_QM_Q_FIELD_NPEID_L, \
                  IX_ETHNPE_QM_Q_FIELD_NPEID_R)

/**
 * @def IX_ETHNPE_QM_Q_RXENET_PORTID_MASK 
 *
 * @brief Macro to mask the Port ID field of the Queue Manager RxEnet Queue entry
 */
#define IX_ETHNPE_QM_Q_RXENET_PORTID_MASK \
            MASK (IX_ETHNPE_QM_Q_FIELD_PORTID_L, \
                  IX_ETHNPE_QM_Q_FIELD_PORTID_R)

/**
 * @def IX_ETHNPE_QM_Q_RXENET_ADDR_MASK 
 *
 * @brief Macro to mask the Mbuf Address field of the RxEnet Queue Manager Entry
 */
#define IX_ETHNPE_QM_Q_RXENET_ADDR_MASK \
            MASK (IX_ETHNPE_QM_Q_FIELD_ADDR_L, \
                  IX_ETHNPE_QM_Q_FIELD_ADDR_R)

/**
 * @def IX_ETHNPE_QM_Q_TXENET_FPBIT_MASK 
 *
 * @brief Macro to mask the FastPath Flag field of the TxEnet Queue Manager Entry
 */
#define IX_ETHNPE_QM_Q_TXENET_FPBIT_MASK \
            MASK (IX_ETHNPE_QM_Q_FIELD_FPBIT_L, \
                  IX_ETHNPE_QM_Q_FIELD_FPBIT_R)

/**
 * @def IX_ETHNPE_QM_Q_TXENET_PRIOR_MASK
 *
 * @brief Macro to mask the Priority field of the TxEnet Queue Manager Entry
 */
#define IX_ETHNPE_QM_Q_TXENET_PRIOR_MASK \
            MASK (IX_ETHNPE_QM_Q_FIELD_PRIOR_L, \
                  IX_ETHNPE_QM_Q_FIELD_PRIOR_R)

/**
 * @def IX_ETHNPE_QM_Q_TXENET_ADDR_MASK 
 *
 * @brief Macro to mask the FP Descriptor Address field of the TxEnet Queue 
 * Manager Entry
 */
#define IX_ETHNPE_QM_Q_TXENET_ADDR_MASK \
            MASK (IX_ETHNPE_QM_Q_FIELD_ADDR_L, \
                  IX_ETHNPE_QM_Q_FIELD_ADDR_R)

/**
 * @def IX_ETHNPE_QM_Q_TXENET_FPINDEX_MASK 
 * 
 * @brief Macro to mask the FastPath Index field of the TxEnet Shutdown Indicator 
 * Queue Manager Entry
 */
#define IX_ETHNPE_QM_Q_TXENET_FPINDEX_MASK \
            MASK (IX_ETHNPE_QM_Q_FIELD_FPINDEX_L, \
                  IX_ETHNPE_QM_Q_FIELD_FPINDEX_R)

/**
 * @def IX_ETHNPE_QM_Q_TXENET_FPSHTDN_MASK 
 * 
 * @brief Macro to mask the FastPath VC Shutdown indicator field of the TxEnet 
 * Shutdown Indicator Queue Manager Entry
 */
#define IX_ETHNPE_QM_Q_TXENET_FPSHTDN_MASK \
            MASK (IX_ETHNPE_QM_Q_FIELD_FPSHTDN_L, \
                  IX_ETHNPE_QM_Q_FIELD_FPSHTDN_R)

/**
 * @def IX_ETHNPE_QM_Q_TXENETDONE_NPEID_MASK 
 *
 * @brief Macro to mask the NPE ID field of the TxEnetDone Queue Manager Entry
 */
#define IX_ETHNPE_QM_Q_TXENETDONE_NPEID_MASK \
            MASK (IX_ETHNPE_QM_Q_FIELD_NPEID_L, \
                  IX_ETHNPE_QM_Q_FIELD_NPEID_R)

/**
 * @def IX_ETHNPE_QM_Q_TXENETDONE_ADDR_MASK 
 * 
 * @brief Macro to mask the Mbuf Address field of the TxEnetDone Queue Manager 
 * Entry
 */
#define IX_ETHNPE_QM_Q_TXENETDONE_ADDR_MASK \
            MASK (IX_ETHNPE_QM_Q_FIELD_ADDR_L, \
                  IX_ETHNPE_QM_Q_FIELD_ADDR_R)

/**
 * @def IX_ETHNPE_QM_Q_FREEFP_ADDR_MASK
 * 
 * @brief Macro to mask the FP Descriptor Address field of the Fast Path FreeFP 
 * Queue Manager Entry
 */
#define IX_ETHNPE_QM_Q_FREEFP_ADDR_MASK \
            MASK (IX_ETHNPE_QM_Q_FIELD_ADDR_L, \
                  IX_ETHNPE_QM_Q_FIELD_ADDR_R)

/*--------------------------------------------------------------------------
 * Queue Manager Queue entry bit field value extraction macros
 *------------------------------------------------------------------------*/

/**
 * @def IX_ETHNPE_QM_Q_FREEENET_ADDR_VAL(x)
 *
 * @brief Extraction macro for Address field of FreeNet Queue Manager Entry 
 *
 * Pointer to an mbuf buffer descriptor
 */
#define IX_ETHNPE_QM_Q_FREEENET_ADDR_VAL(x) \
            ((x) & IX_ETHNPE_QM_Q_FREEENET_ADDR_MASK)

/**
 * @def IX_ETHNPE_QM_Q_RXENET_NPEID_VAL(x)
 *
 * @brief Extraction macro for NPE ID field of RxEnet Queue Manager Entry 
 *
 * Set to 0 for entries originating from the Eth0 NPE; 
 * Set to 1 for entries originating from the Eth1 NPE.
 */
#define IX_ETHNPE_QM_Q_RXENET_NPEID_VAL(x) \
            BITS (x, IX_ETHNPE_QM_Q_FIELD_NPEID_L, \
                     IX_ETHNPE_QM_Q_FIELD_NPEID_R)

/**
 * @def IX_ETHNPE_QM_Q_RXENET_PORTID_VAL(x)
 *
 * @brief Extraction macro for Port ID field of RxEnet Queue Manager Entry 
 *
 * 0-5: Assignable (by the XScale client) to any of the physical ports.
 * 6: It is reserved
 * 7: Indication that the NPE did not find the associated frame's destination MAC address within 
 * its internal filtering database.
 */
#define IX_ETHNPE_QM_Q_RXENET_PORTID_VAL(x) \
            BITS (x, IX_ETHNPE_QM_Q_FIELD_PORTID_L, \
                     IX_ETHNPE_QM_Q_Field_PortID_R)

/**
 * @def IX_ETHNPE_QM_Q_RXENET_ADDR_VAL(x)
 *
 * @brief Extraction macro for Address field of RxEnet Queue Manager Entry
 *
 * Pointer to an mbuf buffer descriptor
 */
#define IX_ETHNPE_QM_Q_RXENET_ADDR_VAL(x) \
            ((x) & IX_ETHNPE_QM_Q_RXENET_ADDR_MASK)

/**
 * @def IX_ETHNPE_QM_Q_TXENET_FPBIT_VAL(x
 *
 * @brief Extraction macro for Fast Path Bit field of TxEnet Queue Manager Entry 
 *
 * Macro to set to use either SlowPath/FastPath TxEnet Queue Manager Entry
 *
 * "1": if the mbuf address points to a FastPath descriptor and 
 * "0": if the mbuf address points to a SlowPath mbuf descriptor. 
 *
 * This bit is always cleared upon return from the Ethernet NPE (to either the 
 * TxEnetDone or FreeFP queue).
 */
#define IX_ETHNPE_QM_Q_TXENET_FPBIT_VAL(x) \
            BITS (x, IX_ETHNPE_QM_Q_FIELD_FPBIT_L, \
                     IX_ETHNPE_QM_Q_FIELD_FPBIT_R)

/**
 * @def IX_ETHNPE_QM_Q_TXENET_PRIOR_VAL(x)
 *
 * @brief Extraction macro for Priority field of TxEnet Queue Manager Entry
 *
 * Priority of the packet (as described in IEEE 802.1D) must be 0 for FastPath 
 * VC shutdown indicators.  This field is cleared upon return from the Ethernet 
 * NPE (to either the TxEnetDone or FreeFP queue).
 */
#define IX_ETHNPE_QM_Q_TXENET_PRIOR_VAL(x) \
            BITS (x, IX_ETHNPE_QM_Q_FIELD_PRIOR_L, \
                     IX_ETHNPE_QM_Q_FIELD_PRIOR_R)

/**
 * @def IX_ETHNPE_QM_Q_TXENET_ADDR_VAL(x)
 *
 * @brief Extraction macro for Address field of Queue Manager SlowPath TxEnet Queue 
 * Manager Entry
 *
 * Pointer to an mbuf buffer descriptor
 */
#define IX_ETHNPE_QM_Q_TXENET_ADDR_VAL(x) \
            ((x) & IX_ETHNPE_QM_Q_TXENET_ADDR_MASK)

/**
 * @def IX_ETHNPE_QM_Q_TXENET_FPINDEX_VAL(x)
 *
 * @brief Extraction macro for Fast Path Index field of FastPath TxEnet Queue 
 * Manager Entry
 *
 * Pointer to a FastPath descriptor
 */
#define IX_ETHNPE_QM_Q_TXENET_FPINDEX_VAL(x) \
            BITS (x, IX_ETHNPE_QM_Q_FIELD_FPINDEX_L, \
                     IX_ETHNPE_QM_Q_FIELD_FPINDEX_R)

/**
 * @def IX_ETHNPE_QM_Q_TXENET_FPSHTDN_VAL(x)
 *
 * @brief  Extraction macro for FastPath Shutdown field of TxEnet/RxFP VC Shutdown 
 * Indicator Queue Manager Entry
 */
#define IX_ETHNPE_QM_Q_TXENET_FPSHTDN_VAL(x) \
            BITS (x, IX_ETHNPE_QM_Q_FIELD_FPSHTDN_L, \
                     IX_ETHNPE_QM_Q_FIELD_FPSHTDN_R)

/**
 * @def IX_ETHNPE_QM_Q_TXENETDONE_NPEID_VAL(x)
 *
 * @brief Extraction macro for NPE ID field of TxEnetDone Queue Manager Entry
 *
 * Set to 0 for entries originating from the Eth0 NPE; set to 1 for en-tries 
 * originating from the Eth1 NPE.
 */
#define IX_ETHNPE_QM_Q_TXENETDONE_NPEID_VAL(x) \
            BITS (x, IX_ETHNPE_QM_Q_FIELD_NPEID_L, \
                     IX_ETHNPE_QM_Q_FIELD_NPEID_R)

/**
 * @def IX_ETHNPE_QM_Q_TXENETDONE_ADDR_VAL(x)
 *
 * @brief Extraction macro for Address field of TxEnetDone Queue Manager Entry
 *
 * Pointer to an mbuf buffer descriptor
 */
#define IX_ETHNPE_QM_Q_TXENETDONE_ADDR_VAL(x) \
            ((x) & IX_ETHNPE_QM_Q_TXENETDONE_ADDR_MASK)

/**
 * @def IX_ETHNPE_QM_Q_FREEFP_ADDR_VAL(x)
 *
 * @brief Extraction macro for Address field of FastPath FreeFP Queue Manager Entry
 *
 * Pointer to a FastPath descriptor
 */
#define IX_ETHNPE_QM_Q_FREEFP_ADDR_VAL(x) \
            ((x) & IX_ETHNPE_QM_Q_FREEFP_ADDR_MASK)

/*--------------------------------------------------------------------------
 * Queue Manager Queue entry construction macros
 *
 * These macros assume that all addresses are word-aligned and that all
 * input field values are within their allowable ranges.
 *------------------------------------------------------------------------*/

/**
 * @def IX_ETHNPE_QM_Q_FREEENET_ENTRY(addr)
 *
 * @brief Queue entry construction macros for FreeNet Queue Manager
 */
#define IX_ETHNPE_QM_Q_FREEENET_ENTRY(addr)         (addr)

/**
 * @def IX_ETHNPE_QM_Q_RXENET_ENTRY(addr,id,prt)
 *
 * @brief Queue entry construction macros for RxEnet Queue Manager
 */
#define IX_ETHNPE_QM_Q_RXENET_ENTRY(addr,id,prt)    ((id)  << 30 | \
                                                     (prt) << 28 | (addr))

/**
 * @def IX_ETHNPE_QM_Q_TXENET_ENTRY(addr,fp,pri)
 *
 * @brief Queue entry construction macros for TxEnet Queue Manager
 */
#define IX_ETHNPE_QM_Q_TXENET_ENTRY(addr,fp,pri)    ((fp)  << 30 | \
                                                     (pri) << 28 | (addr))

/**
 * @def IX_ETHNPE_QM_Q_TXENETDONE_ENTRY(addr,id)
 *
 * @brief Queue entry construction macros for TxEnetDone Queue Manager
 */
#define IX_ETHNPE_QM_Q_TXENETDONE_ENTRY(addr,id)    ((id) << 30 | (addr))

/**
 * @def IX_ETHNPE_QM_Q_VCSHUTDOWN_ENTRY(fpidx) 
 *
 * @brief Queue entry construction macros for VC Shutdown Queue Manager
 */
#define IX_ETHNPE_QM_Q_VCSHUTDOWN_ENTRY(fpidx)      ((fpidx) << 1 | 0x80000001)

/**
 * @def IX_ETHNPE_QM_Q_FREEFP_ENTRY(addr) 
 *
 * @brief Queue entry construction macros for FreeFP Queue Manager
 */
#define IX_ETHNPE_QM_Q_FREEFP_ENTRY(addr)           (addr)

/**
 *@}
 */
