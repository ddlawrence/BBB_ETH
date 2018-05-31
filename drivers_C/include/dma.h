//
//  DMA Configuration and Initialization  -  prototypes
//  

#include <stdio.h>
#include <stdlib.h>
#include "hw_dmacc.h"
#include "hw_dmatc.h"

/** Maximum number of DMA Shadow regions available */
#define DMA_MAX_REGIONS                     (2u)

/** Number of PaRAM Sets available */
#define DMA_NUM_PARAMSET                    (128u)

/** Number of Event Queues available */
#define DMA_0_NUM_EVTQUE                    (2u)

/** Number of Transfer Controllers available */
#define DMA_0_NUM_TC                        (2u)

/** Interrupt no. for Transfer Completion */
#define DMA_0_CC_XFER_COMPLETION_INT        (11u)

/** Interrupt no. for CC Error */
#define DMA_0_CC0_ERRINT                    (12u)

/** Interrupt no. for TCs Error */
#define DMA_0_TC0_ERRINT                    (13u)
#define DMA_0_TC1_ERRINT                    (32u)

#define DMACC_DMA_CHANNELS                  (32u)
#define DMACC_QDMA_CHANNELS                 (8u)

/** DMAQNUM bits Clear */
#define DMACC_DMAQNUM_CLR(chNum)            ( ~ (0x7u << (((chNum) % 8u) \
                                                                      * 4u)))
/** DMAQNUM bits Set */
#define DMACC_DMAQNUM_SET(chNum,queNum)     ((0x7u & (queNum)) << \
                                                       (((chNum) % 8u) * 4u))
/** QDMAQNUM bits Clear */
#define DMACC_QDMAQNUM_CLR(chNum)           ( ~ (0x7u << ((chNum) * 4u)))
/** QDMAQNUM bits Set */
#define DMACC_QDMAQNUM_SET(chNum,queNum)    ((0x7u & (queNum)) << \
                                                           ((chNum) * 4u))

#define DMACC_QCHMAP_PAENTRY_CLR            (~DMACC_QCHMAP_PAENTRY)
/** QCHMAP-PaRAMEntry bitfield Set */
#define DMACC_QCHMAP_PAENTRY_SET(paRAMId)   (((DMACC_QCHMAP_PAENTRY >> \
                                              DMACC_QCHMAP_PAENTRY_SHIFT) & \
                                              (paRAMId)) << \
                                              DMACC_QCHMAP_PAENTRY_SHIFT)
/** QCHMAP-TrigWord bitfield Clear */
#define DMACC_QCHMAP_TRWORD_CLR             (~DMACC_QCHMAP_TRWORD)
/** QCHMAP-TrigWord bitfield Set */
#define DMACC_QCHMAP_TRWORD_SET(paRAMId)    (((DMACC_QCHMAP_TRWORD >> \
                                              DMACC_QCHMAP_TRWORD_SHIFT) & \
                                              (paRAMId)) << \
                                              DMACC_QCHMAP_TRWORD_SHIFT)

/** OPT-TCC bitfield Clear */
#define DMACC_OPT_TCC_CLR                   (~DMACC_OPT_TCC)

/** OPT-TCC bitfield Set */
#define DMACC_OPT_TCC_SET(tcc)              (((DMACC_OPT_TCC >> \
                                              DMACC_OPT_TCC_SHIFT) & \
                                              (tcc)) << DMACC_OPT_TCC_SHIFT)

#define DMA_SET_ALL_BITS                    (0xFFFFFFFFu)
#define DMA_CLR_ALL_BITS                    (0x00000000u)

#define DMACC_COMPL_HANDLER_RETRY_COUNT     (10u)
#define DMACC_ERR_HANDLER_RETRY_COUNT       (10u)

#define DMA_TRIG_MODE_MANUAL                (0u)
#define DMA_TRIG_MODE_QDMA                  (1u)
#define DMA_TRIG_MODE_EVENT                 (2u)

#define DMA_CHANNEL_TYPE_DMA                (0u)
#define DMA_CHANNEL_TYPE_QDMA               (1u)


#define DMA_XFER_COMPLETE                   (0u)
#define DMA_CC_DMA_EVT_MISS                 (1u)
#define DMA_CC_QDMA_EVT_MISS                (2u)

#define DMA_SYNC_A                          (0u)
#define DMA_SYNC_AB                         (1u)

#define DMACC_CLR_TCCERR                     DMACC_CCERRCLR_TCCERR
#define DMACC_CLR_QTHRQ0                     DMACC_CCERRCLR_QTHRXCD0
#define DMACC_CLR_QTHRQ1                     DMACC_CCERRCLR_QTHRXCD1


/* paRAMEntry Fields*/
    /**
     * The OPT field (Offset Address 0x0 Bytes)
     */
#define    DMACC_PARAM_ENTRY_OPT            (0x0u)

    /**
     * The SRC field (Offset Address 0x4 Bytes)
     */
#define    DMACC_PARAM_ENTRY_SRC            (0x1u)

    /**
     * The (ACNT+BCNT) field (Offset Address 0x8 Bytes)
     */
#define    DMACC_PARAM_ENTRY_ACNT_BCNT      (0x2u)

    /**
     * The DST field (Offset Address 0xC Bytes)
     */
#define    DMACC_PARAM_ENTRY_DST            (0x3u)

    /**
     * The (SRCBIDX+DSTBIDX) field (Offset Address 0x10 Bytes)
     */
#define    DMACC_PARAM_ENTRY_SRC_DST_BIDX   (0x4u)

    /**
     * The (LINK+BCNTRLD) field (Offset Address 0x14 Bytes)
     */
#define    DMACC_PARAM_ENTRY_LINK_BCNTRLD   (0x5u)

    /**
     * The (SRCCIDX+DSTCIDX) field (Offset Address 0x18 Bytes)
     */
#define    DMACC_PARAM_ENTRY_SRC_DST_CIDX   (0x6u)

    /**
     * The (CCNT+RSVD) field (Offset Address 0x1C Bytes)
     */
#define    DMACC_PARAM_ENTRY_CCNT           (0x7u)


/** The offset for each PaRAM Entry field  */
#define    DMACC_PARAM_FIELD_OFFSET         (0x4u)

/** Number of PaRAM Entry fields
  * OPT, SRC, A_B_CNT, DST, SRC_DST_BIDX, LINK_BCNTRLD, SRC_DST_CIDX
  * and CCNT
  */
#define    DMACC_PARAM_ENTRY_FIELDS         (0x8u)


#define    DMA_REVID_AM335X                  (0x02u)

#define SOC_DMA_NUM_DMACH                 64
#define SOC_DMA_NUM_QDMACH                8
#define SOC_DMA_NUM_PARAMSETS             256
#define SOC_DMA_NUM_EVQUE                 4
#define SOC_DMA_CHMAPEXIST                0
#define SOC_DMA_NUM_REGIONS               8
#define SOC_DMA_MEMPROTECT                0

/** Number of TCCS available */
#define DMA_NUM_TCC                        SOC_DMA_NUM_DMACH

/**
 * \brief DMA Parameter RAM Set in User Configurable format
 *
 * This is a mapping of the DMA PaRAM set provided to the user
 * for ease of modification of the individual fields
 */
typedef struct DMACCPaRAMEntry {
        /** OPT field of PaRAM Set */
        unsigned int opt;

        /**
         * \brief Starting byte address of Source
         * For FIFO mode, srcAddr must be a 256-bit aligned address.
         */
        unsigned int srcAddr;

        /**
         * \brief Number of bytes in each Array (ACNT)
         */
        unsigned short aCnt;

        /**
         * \brief Number of Arrays in each Frame (BCNT)
         */
        unsigned short bCnt;

        /**
         * \brief Starting byte address of destination
         * For FIFO mode, destAddr must be a 256-bit aligned address.
         * i.e. 5 LSBs should be 0.
         */
        unsigned int destAddr;

        /**
         * \brief Index between consec. arrays of a Source Frame (SRCBIDX)
         */
        short  srcBIdx;

        /**
         * \brief Index between consec. arrays of a Destination Frame (DSTBIDX)
         */
        short  destBIdx;

        /**
         * \brief Address for linking (AutoReloading of a PaRAM Set)
         * This must point to a valid aligned 32-byte PaRAM set
         * A value of 0xFFFF means no linking
         */
        unsigned short linkAddr;

        /**
         * \brief Reload value of the numArrInFrame (BCNT)
         * Relevant only for A-sync transfers
         */
        unsigned short bCntReload;

        /**
         * \brief Index between consecutive frames of a Source Block (SRCCIDX)
         */
        short  srcCIdx;

        /**
         * \brief Index between consecutive frames of a Dest Block (DSTCIDX)
         */
        short  destCIdx;

        /**
         * \brief Number of Frames in a block (CCNT)
         */
        unsigned short cCnt;

        /**
         * \brief  This field is Reserved. Write zero to this field.
         */
        unsigned short rsvd;

}DMACCPaRAMEntry;

/*
** Structure to store the DMA context
*/
typedef struct edmaContext {
    /* Channel mapping reg Val */
    unsigned int dchMap[64];
    /* DMA Queue Number Register Val */    
    unsigned int dmaQNum[8];    
    
    /* DMA Region Access Enable Register val */    
    unsigned int regAccEnableLow;    
    unsigned int regAccEnableHigh;        

    /* Event Set Register value */    
    unsigned int eventSetRegLow;
    unsigned int eventSetRegHigh;    
    
    /* Enable Event Set Register value */    
    unsigned int enableEvtSetRegLow;
    unsigned int enableEvtSetRegHigh;
    
    /* Interrupt Enable Set Register value */            
    unsigned int intEnableSetRegLow;        
    unsigned int intEnableSetRegHigh;    
    
    struct DMACCPaRAMEntry dmaParEntry[512];    
    
} DMACONTEXT;

void DMAInit(unsigned int baseAdd, unsigned int queNum);

void DMAEnableChInShadowReg(unsigned int baseAdd, unsigned int chType, unsigned int chNum);

void DMADisableChInShadowReg(unsigned int baseAdd, unsigned int chType, unsigned int chNum);

void DMAMapChToEvtQ(unsigned int baseAdd,
                      unsigned int chType,
                      unsigned int chNum,
                      unsigned int evtQNum);

void DMAUnmapChToEvtQ(unsigned int baseAdd, unsigned int chType, unsigned int chNum);

void DMAMapQdmaChToPaRAM(unsigned int baseAdd, unsigned int chNum, unsigned int *paRAMId);

void DMASetQdmaTrigWord(unsigned int baseAdd, unsigned int chNum, unsigned int trigWord);

void DMAClrMissEvt(unsigned int baseAdd, unsigned int chNum);

void DMAQdmaClrMissEvt(unsigned int baseAdd, unsigned int chNum);

void DMAClrCCErr(unsigned int baseAdd, unsigned int Flags);

void DMASetEvt(unsigned int baseAdd, unsigned int chNum);

void DMAClrEvt(unsigned int baseAdd, unsigned int chNum);

void DMAEnableDmaEvt(unsigned int baseAdd, unsigned int chNum);

void DMADisableDmaEvt(unsigned int baseAdd, unsigned int chNum);

void DMAEnableQdmaEvt(unsigned int baseAdd, unsigned int chNum);

void DMADisableQdmaEvt(unsigned int baseAdd, unsigned int chNum);

unsigned int DMAGetIntrStatus(unsigned int baseAdd);

void DMAEnableEvtIntr(unsigned int baseAdd, unsigned int chNum);

void DMADisableEvtIntr(unsigned int baseAdd, unsigned int chNum);

void DMAClrIntr(unsigned int baseAdd, unsigned int value);

void DMAGetPaRAM(unsigned int baseAdd,
                   unsigned int chNum,
                   DMACCPaRAMEntry* currPaRAM);

void DMAQdmaGetPaRAM(unsigned int baseAdd,
                       unsigned int chNum,
                       unsigned int paRAMId,
                       DMACCPaRAMEntry* currPaRAM);

void DMASetPaRAM(unsigned int baseAdd,
                   unsigned int chNum,
                   DMACCPaRAMEntry* newPaRAM);

void DMAQdmaSetPaRAM(unsigned int baseAdd,
                       unsigned int chNum,
                       unsigned int paRAMId,
                       DMACCPaRAMEntry* newPaRAM);

void DMAQdmaSetPaRAMEntry(unsigned int baseAdd,
                            unsigned int paRAMId,
                            unsigned int paRAMEntry,
                            unsigned int newPaRAMEntryVal);

unsigned int DMAQdmaGetPaRAMEntry(unsigned int baseAdd,
                                    unsigned int paRAMId,
                                    unsigned int paRAMEntry);

unsigned int DMARequestChannel(unsigned int baseAdd, unsigned int chType,
                                 unsigned int chNum, unsigned int tccNum,
                                 unsigned int evtQNum);

unsigned int DMAFreeChannel(unsigned int baseAdd, unsigned int chType,
                              unsigned int chNum, unsigned int trigMode,
                              unsigned int tccNum, unsigned int evtQNum);

unsigned int DMAEnableTransfer(unsigned int baseAdd,
                                 unsigned int chNum,
                                 unsigned int trigMode);

unsigned int DMADisableTransfer(unsigned int baseAdd,
                                  unsigned int chNum,
                                  unsigned int trigMode);

void DMAClearErrorBits(unsigned int baseAdd,
                         unsigned int chNum,
                         unsigned int evtQNum);

unsigned int DMAGetCCErrStatus(unsigned int baseAdd);

unsigned int DMAGetErrIntrStatus(unsigned int baseAdd);

unsigned int DMAQdmaGetErrIntrStatus(unsigned int baseAdd);

void DMACCErrorEvaluate(unsigned int baseAddr);

void DMADeinit(unsigned int baseAdd, unsigned int queNum);

unsigned int DMAVersionGet(void);

unsigned int DMAPeripheralIdGet(unsigned int baseAdd);
unsigned int DMAIntrStatusHighGet(unsigned int baseAdd);
unsigned int DMAErrIntrHighStatusGet(unsigned int baseAdd);

void DMAChannelToParamMap(unsigned int baseAdd, unsigned int channel, 
                            unsigned int paramSet);

extern void DMAContextSave(unsigned int baseAddr, DMACONTEXT *edmaCntxPtr);
extern void DMAContextRestore(unsigned int baseAddr, DMACONTEXT *edmaCntxPtr);

//  eof
