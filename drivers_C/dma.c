//
//  DMA Configuration and Initialization
//  
//  functions originally written by TIdude
//
#define am335x
#define TRUE        1
#define FALSE       0

#include "dma.h"
#include "hw_types.h"

unsigned int regionId;

//
//  DMA Initialization
//  
//  This function initializes the DMA Driver
//  Clears the error specific registers (EMCR/EMCRh, QEMCR, CCERRCLR) &
//  initialize the Queue Number Registers
//
//  param   baseAdd                  Memory address of the DMA instance used 
//
//  param   queNum                   Event Queue Number to which the channel
//                                   will be mapped (valid only for the
//                                   Master Channel (DMA/QDMA) request) 
//
//   return None
//
//  NB      The regionId is the shadow region(0 or 1) used and the,
//          Event Queue used is either (0 or 1). There are only four shadow 
//          regions and only two event Queues
//
void DMAInit(unsigned int baseAdd, unsigned int queNum) {
    unsigned int count = 0;
    unsigned int i = 0;
    
    regionId = (unsigned int)0u;
    // Clear the Event miss Registers
    HWREG(baseAdd + DMACC_EMCR) = DMA_SET_ALL_BITS;
    HWREG(baseAdd + DMACC_EMCRH) = DMA_SET_ALL_BITS;
    HWREG(baseAdd + DMACC_QEMCR) = DMA_SET_ALL_BITS;
    // Clear CCERR register
    HWREG(baseAdd + DMACC_CCERRCLR) = DMA_SET_ALL_BITS;
    // FOR TYPE DMA Enable DMA (0 - 64) channels in DRAE and DRAEH register
    HWREG(baseAdd + DMACC_DRAE(regionId)) = DMA_SET_ALL_BITS;
    HWREG(baseAdd + DMACC_DRAEH(regionId)) = DMA_SET_ALL_BITS;
    if((DMA_REVID_AM335X == DMAVersionGet())) {           
         for(i = 0; i < 64; i++) {
              // All events are one to one mapped with channels
              HWREG(baseAdd + DMACC_DCHMAP(i)) = i << 5;
         }
    }
    // Initialize DMA Queue Number Registers
    for (count = 0;count < SOC_DMA_NUM_DMACH; count++) {
        HWREG(baseAdd + DMACC_DMAQNUM(count >> 3u)) &= DMACC_DMAQNUM_CLR(count);
        HWREG(baseAdd + DMACC_DMAQNUM(count >> 3u)) |= DMACC_DMAQNUM_SET(count,queNum);
    }
    // FOR TYPE QDMA Enable DMA (0 - 64) channels in DRAE register
    HWREG(baseAdd + DMACC_QRAE(regionId)) = DMA_SET_ALL_BITS;
    // Initialize the QDMA Queue Number Registers
    for (count = 0;count < SOC_DMA_NUM_QDMACH; count++) {
        HWREG(baseAdd + DMACC_QDMAQNUM) &= DMACC_QDMAQNUM_CLR(count);
        HWREG(baseAdd + DMACC_QDMAQNUM) |= DMACC_QDMAQNUM_SET(count,queNum);
    }
}

//
// This API return the revision Id of the peripheral.
//
// param    baseAdd     Memory address of the DMA instance used 
//
//   return  None
//
unsigned int DMAPeripheralIdGet(unsigned int baseAdd) {
    return (HWREG(baseAdd + DMACC_REVID));
}

//
//   Enable channel to Shadow region mapping
//
// This API allocates DMA/QDMA channels or TCCs, and the same resources are 
// enabled in the shadow region specific register (DRAE/DRAEH/QRAE).
// Here only one shadow region is used since, there is only one Master.
//
//  param    baseAdd     Memory address of the DMA instance used 
//
//  param    chtype      (DMA/QDMA) Channel
//                        For Example: For DMA it is,
//                        DMA_CHANNEL_TYPE_DMA 
//
//  param    chNum       Allocated channel number 
//
//  chtype can have values
//        DMA_CHANNEL_TYPE_DMA\n
//        DMA_CHANNEL_TYPE_QDMA
//
//   return  None
//
void DMAEnableChInShadowReg(unsigned int baseAdd, unsigned int chType, 
                             unsigned int chNum) {
    // Allocate the DMA/QDMA channel //
    if (DMA_CHANNEL_TYPE_DMA == chType) {
        // FOR TYPE DMA// 
        if(chNum < 32) {
            // Enable the DMA channel in the DRAE registers
            HWREG(baseAdd + DMACC_DRAE(regionId)) |= (0x01u << chNum);
        } else {
            // Enable the DMA channel in the DRAEH registers
            HWREG(baseAdd + DMACC_DRAEH(regionId)) |= (0x01u << (chNum - 32));
        }
    } else if (DMA_CHANNEL_TYPE_QDMA== chType) {
        // FOR TYPE QDMA Enable QDMA channel in DRAE/DRAEH registers
        HWREG(baseAdd + DMACC_QRAE(regionId)) |= 0x01u << chNum;
    }
}

//
//   Disable channel to Shadow region mapping
//
// This API allocates DMA/QDMA channels or TCCs, and the same resources are 
// enabled in the shadow region specific register (DRAE/DRAEH/QRAE).
// Here only one shadow region is used since, there is only one Master.
//
// param    baseAdd   Memory address of the DMA instance used 
//
// param    chtype    (DMA/QDMA) Channel
//                      For Example: For DMA it is,
//                      DMA_CHANNEL_TYPE_DMA 
//
// param    chNum      Allocated channel number 
//
//  chtype can have values
//        DMA_CHANNEL_TYPE_DMA\n
//        DMA_CHANNEL_TYPE_QDMA
//
//   return  None
//
void DMADisableChInShadowReg(unsigned int baseAdd, unsigned int chType, 
                               unsigned int chNum) {
    // Allocate the DMA/QDMA channel
    if (DMA_CHANNEL_TYPE_DMA == chType) {
        // FOR TYPE DMA
         if(chNum < 32) { 
              // Enable the DMA channel in the DRAE registers
              HWREG(baseAdd + DMACC_DRAE(regionId)) &= ~(0x01u << chNum);
         } else {
              // Enable the DMA channel in the DRAEH registers //
              HWREG(baseAdd + DMACC_DRAEH(regionId)) &= ~(0x01u << (chNum - 32));
         }
    } else if (DMA_CHANNEL_TYPE_QDMA== chType) {
        // FOR TYPE QDMA //
        // Enable the QDMA channel in the DRAE/DRAEH registers //
        HWREG(baseAdd + DMACC_QRAE(regionId)) &= ((~0x01u) << chNum);
    }
}

//
//  This function maps DMA channel to any of the PaRAM sets
//           in the PaRAM memory map.
//
//  param    baseAdd   Memory address of the DMA instance used.
//
//  param    channel   The DMA channel number required to be mapped.
//
//  param    paramSet  It specifies the paramSet to which DMA channel
//                     required to be mapped.
//
//   return  None
//
void DMAChannelToParamMap(unsigned int baseAdd, unsigned int channel,
                            unsigned int paramSet) {
    HWREG(baseAdd + DMACC_DCHMAP(channel)) = paramSet << 5;
} 

//
//    Map channel to Event Queue
//
//  This API maps DMA/QDMA channels to the Event Queue
//
//  param   baseAdd    Memory address of the DMA instance used 
//
//  param   chtype     (DMA/QDMA) Channel
//                     For Example: For QDMA it is
//                     DMA_CHANNEL_TYPE_QDMA 
//
//  param   chNum      Allocated channel number 
//
//  param   evtQNum    Event Queue Number to which the channel
//                     will be mapped (valid only for the
//                     Master Channel (DMA/QDMA) request) 
//
//  chtype can have values
//        DMA_CHANNEL_TYPE_DMA\n
//        DMA_CHANNEL_TYPE_QDMA
//
//   return  None
//
void DMAMapChToEvtQ(unsigned int baseAdd, unsigned int chType, 
                      unsigned int chNum, unsigned int evtQNum) {
    if (DMA_CHANNEL_TYPE_DMA == chType) {
        // Associate DMA Channel to Event Queue
        HWREG(baseAdd + DMACC_DMAQNUM((chNum) >> 3u)) &= DMACC_DMAQNUM_CLR(chNum);
        HWREG(baseAdd + DMACC_DMAQNUM((chNum) >> 3u)) |= 
                      DMACC_DMAQNUM_SET((chNum), evtQNum);
    }
    else if (DMA_CHANNEL_TYPE_QDMA == chType) {
        // Associate QDMA Channel to Event Queue
        HWREG(baseAdd + DMACC_QDMAQNUM) |= 
                       DMACC_QDMAQNUM_SET(chNum, evtQNum);
    }
}

//
//    Remove Mapping of channel to Event Queue
//
//  This API Unmaps DMA/QDMA channels to the Event Queue allocated
//
//  param   baseAdd    Memory address of the DMA instance used 
//
//  param   chtype     (DMA/QDMA) Channel
//                     For Example: For DMA it is
//                     DMA_CHANNEL_TYPE_DMA 
//
//  param   chNum      Allocated channel number 
//
//  chtype can have values
//        DMA_CHANNEL_TYPE_DMA\n
//        DMA_CHANNEL_TYPE_QDMA
//
//   return  None
//
void DMAUnmapChToEvtQ(unsigned int baseAdd, unsigned int chType, 
                        unsigned int chNum) {
    if (DMA_CHANNEL_TYPE_DMA == chType) {
        // Unmap DMA Channel to Event Queue
        HWREG(baseAdd + DMACC_DMAQNUM((chNum) >> 3u)) |= DMACC_DMAQNUM_CLR(chNum);
    } else if (DMA_CHANNEL_TYPE_QDMA == chType) {
        // Unmap QDMA Channel to Event Queue
        HWREG(baseAdd + DMACC_QDMAQNUM) |= DMACC_QDMAQNUM_CLR(chNum);
    }
}

//
//    Enables the user to map a QDMA channel to PaRAM set
//          This API Needs to be called before programming the paRAM sets for
//          the QDMA Channels.Application needs to maitain the paRAMId 
//          provided by this API.This paRAMId is used to set paRAM and get
//          paRAM. Refer corresponding API's for more details.
//
//  param   baseAdd                  Memory address of the DMA instance used 
//
//  param   chNum                    Allocated QDMA channel number 
//
//  param   paRaMID                  PaRAM Id to which the QDMA channel will be
//                                   mapped to.
//
//   return None
//
//  Note : The PaRAMId requested must be greater than 32(SOC_DMA_NUM_DMACH). 
//         and lesser than SOC_DMA_NUM_DMACH + chNum  Because, the first 
//         32 PaRAM's are directly mapped to first 32 DMA channels and (32 - 38)
//         for QDMA Channels. (32 - 38) is assigned by driver in this API. 
//
void DMAMapQdmaChToPaRAM(unsigned int baseAdd, unsigned int chNum,
                           unsigned int *paRAMId) {
    // First 32 channels are for DMA only 
    // if (((*paRAMId) > SOC_DMA_NUM_DMACH) && ((*paRAMId) < SOC_DMA_NUM_DMACH+SOC_DMA_NUM_QDMACH))  // 
    if ((SOC_DMA_NUM_DMACH + chNum) == (*paRAMId)) {
        // Map Parameter RAM Set Number for specified channelId
        HWREG(baseAdd + DMACC_QCHMAP(chNum)) &= DMACC_QCHMAP_PAENTRY_CLR;
        HWREG(baseAdd + DMACC_QCHMAP(chNum)) |= DMACC_QCHMAP_PAENTRY_SET(*paRAMId);
    } else {
        (*paRAMId) = (SOC_DMA_NUM_DMACH + chNum);
        // Map Parameter RAM Set Number for specified channelId
        HWREG(baseAdd + DMACC_QCHMAP(chNum)) &= DMACC_QCHMAP_PAENTRY_CLR;
        HWREG(baseAdd + DMACC_QCHMAP(chNum)) |= DMACC_QCHMAP_PAENTRY_SET(*paRAMId);
    }
}

//
//   Assign a Trigger Word to the specified QDMA channel
//
// This API sets the Trigger word for the specific QDMA channel in the QCHMAP
// Register. Default QDMA trigger word is CCNT.
//
// param   baseAdd             Memory address of the DMA instance used 
//
// param   chNum               QDMA Channel which needs to be assigned
//                             the Trigger Word
//
// param   trigWord            The Trigger Word for the QDMA channel.
//                             Trigger Word is the word in the PaRAM
//                             Register Set which, when written to by CPU,
//                             will start the QDMA transfer automatically.
//
//  return  None
//
void DMASetQdmaTrigWord(unsigned int baseAdd, unsigned int chNum,
                          unsigned int trigWord) {
     // Set the Trigger Word //
     HWREG(baseAdd + DMACC_QCHMAP(chNum)) |= DMACC_QCHMAP_TRWORD_SET(trigWord);
}

//
//  Enables the user to Clear any missed event
//
//  param    baseAdd                Memory address of the DMA instance used 
//
//  param    chNum                  Allocated channel number 
// 
//   return  None
//
void DMAClrMissEvt(unsigned int baseAdd, unsigned int chNum) {
    if(chNum < 32) {
         //clear SECR to clean any previous NULL request
         HWREG(baseAdd + DMACC_S_SECR(regionId)) = (0x01u << chNum);
         //clear EMCR to clean any previous NULL request
         HWREG(baseAdd + DMACC_EMCR) |= (0x01u <<  chNum);
    } else { 
         HWREG(baseAdd + DMACC_S_SECRH(regionId)) = (0x01u << (chNum - 32));
         //clear EMCRH to clean any previous NULL request
         HWREG(baseAdd + DMACC_EMCRH) |= (0x01u <<  (chNum - 32));
    }
}

//
//  Enables the user to Clear any QDMA missed event
//
//  param    baseAdd                Memory address of the DMA instance used 
//
//  param    chNum                  Allocated channel number 
// 
//   return  None
//
void DMAQdmaClrMissEvt(unsigned int baseAdd, unsigned int chNum) {
    //clear SECR to clean any previous NULL request
    HWREG(baseAdd + DMACC_S_QSECR(regionId)) = (0x01u << chNum);
    //clear EMCR to clean any previous NULL request
    HWREG(baseAdd + DMACC_QEMCR) |= (0x01u <<  chNum);
}

//
//  Enables the user to Clear any Channel controller Errors
//
//  param    baseAdd              Memory address of the DMA instance used 
//
//  param    Flags                Masks to be passed 
//
//  Flags can have values:
//
//  DMACC_CLR_TCCERR            Clears the TCCERR bit in the DMACC 
//                                ERR Reg\n
//  DMACC_CLR_QTHRQ0            Queue threshold error clear for queue 0 
//  DMACC_CLR_QTHRQ1            Queue threshold error clear for queue 1
//
//   return  None
//
void DMAClrCCErr(unsigned int baseAdd, unsigned int Flags) {
    // (CCERRCLR) - clear channel controller error register
    HWREG(baseAdd + DMACC_CCERRCLR) = Flags;
}

//
//  Enables the user to Set an event. This API helps user to manually 
//           set events to initiate DMA transfer requests.
//
//  param    baseAdd                Memory address of the DMA instance used
//
//  param    chNum                  Allocated channel number.
//
//   return  None
//
//  Note :   This API is generally used during Manual transfers 
//
void DMASetEvt(unsigned int baseAdd, unsigned int chNum) {
    if(chNum < 32) { 
         // (ESR) - set corresponding bit to set a event
         HWREG(baseAdd + DMACC_S_ESR(regionId)) |= (0x01u <<  chNum);
    } else {
         // (ESRH) - set corresponding bit to set a event
         HWREG(baseAdd + DMACC_S_ESRH(regionId)) |= (0x01u << (chNum - 32));
    }
}

//
//  Enables the user to Clear an event.
//
//  param    baseAdd                Memory address of the DMA instance used 
//
//  param    chNum                  Allocated channel number 
//
//   return  None
//
//  Note :   This API is generally used during Manual transfers 
//
void DMAClrEvt(unsigned int baseAdd, unsigned int chNum) {
    if(chNum < 32) {
         // (ECR) - set corresponding bit to clear a event
         HWREG(baseAdd + DMACC_S_ECR(regionId)) |= (0x01u <<  chNum);
    } else {
         // (ECRH) - set corresponding bit to clear a event
         HWREG(baseAdd + DMACC_S_ECRH(regionId)) |= (0x01u << (chNum - 32));
    }
}

//
//  Enables the user to enable an DMA event.
//
//  param    baseAdd                Memory address of the DMA instance used 
//
//  param    chNum                  Allocated channel number 
//
//   return  None
//
//  Note :   Writes of 1 to the bits in EESR sets the corresponding event 
//           bits in EER. This is generally used for Event Based transfers 
//
void DMAEnableDmaEvt(unsigned int baseAdd, unsigned int chNum) {
    if(chNum < 32) { 
         // (EESR) - set corresponding bit to enable DMA event
         HWREG(baseAdd + DMACC_S_EESR(regionId)) |= (0x01u <<  chNum);
    } else { 
         // (EESRH) - set corresponding bit to enable DMA event
         HWREG(baseAdd + DMACC_S_EESRH(regionId)) |= (0x01u << (chNum - 32));
    }
}

//
//  Enables the user to Disable an DMA event.
//
//  param    baseAdd                Memory address of the DMA instance used 
//
//  param    chNum                  Allocated channel number 
//
//   return  None
//
//  Note :   Writes of 1 to the bits in EECR clear the corresponding event bits 
//           in EER; writes of 0 have no effect.. This is generally used for 
//           Event Based transfers 
//
void DMADisableDmaEvt(unsigned int baseAdd, unsigned int chNum) {
    if(chNum < 32) {
         // (EECR) - set corresponding bit to disable event
         HWREG(baseAdd + DMACC_S_EECR(regionId)) |= (0x01u <<  chNum);
    } else {
         // (EECRH) - set corresponding bit to disable event                      //
         HWREG(baseAdd + DMACC_S_EECRH(regionId)) |= (0x01u <<  chNum);
    }
}

//
//  Enables the user to enable an QDMA event.
//
//  param    baseAdd                Memory address of the DMA instance used 
//
//  param    chNum                  Allocated channel number 
//
//   return  None
//
//  Note :   Writes of 1 to the bits in QEESR sets the corresponding event 
//              bits in QEER 
//
void DMAEnableQdmaEvt(unsigned int baseAdd, unsigned int chNum) {
    // (QEESR) - set corresponding bit to enable QDMA event
    HWREG(baseAdd + DMACC_S_QEESR(regionId)) = (0x01u << chNum);
}

//
//  Enables the user to disable an QDMA event.
//
//  param    baseAdd                Memory address of the DMA instance used 
//
//  param    chNum                  Allocated channel number 
//
//   return  None
//
//  Note :   Writes of 1 to the bits in QEECR clears the corresponding event 
//              bits in QEER 
//
void DMADisableQdmaEvt(unsigned int baseAdd, unsigned int chNum) {
    // (QEESR) - set corresponding bit to enable QDMA event
    HWREG(baseAdd + DMACC_S_QEECR(regionId)) = (0x01u << chNum);
}

//
//  This returns DMA CC error status.
//
//  param    baseAdd                Memory address of the DMA instance used 
//
//   return  value                  Status of the Interrupt Pending Register
//
unsigned int DMAGetCCErrStatus(unsigned int baseAdd) {
    unsigned int IntrStatusVal = 0;
    IntrStatusVal = (unsigned int)HWREG(baseAdd + DMACC_CCERR);
    return IntrStatusVal;
}

//
//  This function returns interrupts status of those events
//           which is less than 32.
//
//  param    baseAdd                Memory address of the DMA instance used 
//
//   return  value                  Status of the Interrupt Pending Register
//
unsigned int DMAGetIntrStatus(unsigned int baseAdd) {
    unsigned int IntrStatusVal = 0;

    IntrStatusVal = (unsigned int)HWREG(baseAdd + DMACC_S_IPR(regionId));
    return IntrStatusVal;
}

//
//  This function returns interrupts status of those events
//           which is greater than 32.
//
//  param    baseAdd                Memory address of the DMA instance used 
//
//   return  value                  Status of the Interrupt Pending Register
//
unsigned int DMAIntrStatusHighGet(unsigned int baseAdd) {
    unsigned int IntrStatusVal = 0;

    IntrStatusVal = (unsigned int)HWREG(baseAdd + DMACC_S_IPRH(regionId));
    return IntrStatusVal;
}

//
//  This returns error interrupt status for those events whose
//           event number is less than 32.
//
//  param    baseAdd                Memory address of the DMA instance used 
//
//  return   value                  Status of the Interrupt Pending Register
//
unsigned int DMAGetErrIntrStatus(unsigned int baseAdd) {
    unsigned int IntrStatusVal = 0;

    IntrStatusVal = (unsigned int)HWREG(baseAdd + DMACC_EMR);
    return IntrStatusVal;
}

//
//  This returns error interrupt status for those events whose
//           event number is greater than 32.
//
//  param    baseAdd                Memory address of the DMA instance used 
//
//  return   value                  Status of the Interrupt Pending Register
//
unsigned int DMAErrIntrHighStatusGet(unsigned int baseAdd) {
    unsigned int IntrStatusVal = 0;

    IntrStatusVal = (unsigned int)HWREG(baseAdd + DMACC_EMRH);
    return IntrStatusVal;
}

//
//  This returns QDMA error interrupt status.
//
//  param    baseAdd            Memory address of the DMA instance used 
//
//  return   value              Status of the QDMA Interrupt Pending Register
//
unsigned int DMAQdmaGetErrIntrStatus(unsigned int baseAdd) {
    unsigned int IntrStatusVal = 0;
    IntrStatusVal = (unsigned int)HWREG(baseAdd + DMACC_QEMR);

    return IntrStatusVal;
}

//
//  Enables the user to enable the transfer completion interrupt 
//           generation by the DMACC for all DMA/QDMA channels.
//
//  param    baseAdd                Memory address of the DMA instance used 
//
//  param    chNum                  Allocated channel number 
//
//  return   None
//
//  Note :   To set any interrupt bit in IER, a 1 must be written to the 
//           corresponding interrupt bit in the interrupt enable set register.
//
void DMAEnableEvtIntr(unsigned int baseAdd, unsigned int chNum) {
    if(chNum < 32) {
         //  Interrupt Enable Set Register (IESR)
         HWREG(baseAdd + DMACC_S_IESR(regionId)) |= (0x01u <<  chNum);
    } else {
         //  Interrupt Enable Set Register (IESRH)
         HWREG(baseAdd + DMACC_S_IESRH(regionId)) |= (0x01u << (chNum - 32));
    }
}

//
//  Enables the user to clear CC interrupts
//
//  param    baseAdd                Memory address of the DMA instance used 
//
//  param    chNum                  Allocated channel number 
//
//  return   None
//
//  Note :   Writes of 1 to the bits in IECR clear the corresponding interrupt 
//           bits in the interrupt enable registers (IER); writes of 0 have 
//           no effect 
//
void DMADisableEvtIntr(unsigned int baseAdd, unsigned int chNum) {
    if(chNum < 32) {
         // Interrupt Enable Clear Register (IECR)
         HWREG(baseAdd + DMACC_S_IECR(regionId)) |= (0x01u <<  chNum);
    } else {
         // Interrupt Enable Clear Register (IECRH)
         HWREG(baseAdd + DMACC_S_IECRH(regionId)) |= (0x01u << (chNum - 32));
    }
}

//
//  Enables the user to Clear an Interrupt.
//
//  param    baseAdd                Memory address of the DMA instance used.
//
//  param    value                  Value to be set to clear the Interrupt Status.
//
//  return   None
//
void DMAClrIntr(unsigned int baseAdd, unsigned int value) {
    if(value < 32) {
         HWREG(baseAdd + DMACC_S_ICR(regionId)) = (1u << value);
    } else {
         HWREG(baseAdd + DMACC_S_ICRH(regionId)) = (1u << (value - 32));
    }
}


//
//  Retrieve existing PaRAM set associated with specified logical
//           channel (DMA/Link).
//  
//  param    baseAdd                Memory address of the DMA instance used 
//  
//  param    chNum                  Logical Channel whose PaRAM set is
//                                  requested 
//  
//  param    currPaRAM              User gets the existing PaRAM here 
//  
//  return   None
//
void DMAGetPaRAM(unsigned int baseAdd, unsigned int PaRAMId, 
                   DMACCPaRAMEntry* currPaRAM) {
    unsigned int i = 0;
    unsigned int *sr;
    unsigned int *ds = (unsigned int *)currPaRAM;

    sr = (unsigned int *)(baseAdd + DMACC_OPT(PaRAMId));
    for(i=0;i<DMACC_PARAM_ENTRY_FIELDS;i++) *ds++ = *sr++;
}

//
// Retrieve existing PaRAM set associated with specified logical
//          channel (QDMA).
// 
// param    baseAdd                Memory address of the DMA instance used 
// 
// param    chNum                  Logical Channel whose PaRAM set is
//                                 requested 
// 
// param    currPaRAM              User gets the existing PaRAM here 
// 
// return   None
//
void DMAQdmaGetPaRAM(unsigned int baseAdd, unsigned int chNum,
                       unsigned int paRAMId, DMACCPaRAMEntry* currPaRAM) {
    unsigned int i = 0, *sr, *ds = (unsigned int *)currPaRAM;

    sr = (unsigned int *)(baseAdd + DMACC_OPT(paRAMId));
    for(i=0;i<DMACC_PARAM_ENTRY_FIELDS;i++) {
        *ds=*sr;
        ds++;
        sr++;
    }
}

//
// Copy the user specified PaRAM Set onto the PaRAM Set associated 
//          with the logical channel (DMA/Link).
//
// This API takes a PaRAM Set as input and copies it onto the actual PaRAM Set
// associated with the logical channel. OPT field of the PaRAM Set is written
// first and the CCNT field is written last.
//
//
// param    baseAdd                Memory address of the DMA instance used 
//
// param    chNum                  Logical Channel whose PaRAM set is
//                                 requested 
//
// param    newPaRAM               Parameter RAM set to be copied onto existing
//                                 PaRAM 
//
// return   None
//
void DMASetPaRAM(unsigned int baseAdd, unsigned int chNum,
                   DMACCPaRAMEntry* newPaRAM) {
    unsigned int PaRAMId = chNum; // PaRAM mapped to channel Number
    unsigned int i = 0;
    unsigned int *sr = (unsigned int *)newPaRAM;
    volatile unsigned int *ds;
 
//#define DMACC_PaRAM_BASE            (0x4000)
//#define DMACC_OPT(n)                (DMACC_PaRAM_BASE + 0x0 + (0x20 * n))
    ds = (unsigned int *)(baseAdd + DMACC_OPT(PaRAMId));
    for(i=0; i < DMACC_PARAM_ENTRY_FIELDS; i++) {
        *ds = *sr;
        ds++;
        sr++;
    }
}

//
// Copy the user specified PaRAM Set onto the PaRAM Set associated 
//          with the logical channel (QDMA only).
//
// This API takes a PaRAM Set as input and copies it onto the actual PaRAM Set
// associated with the logical channel. OPT field of the PaRAM Set is written
// first and the CCNT field is written last.
//
//
// param    baseAdd                Memory address of the DMA instance used 
//
// param    chNum                  Logical Channel whose PaRAM set is
//                                 requested 
//
// param   paRaMID                 PaRAM Id to which the QDMA channel is
//                        mapped to. 
//
// param    newPaRAM               Parameter RAM set to be copied onto existing 
//                                 PaRAM 
//
// return   None
//
void DMAQdmaSetPaRAM(unsigned int baseAdd, unsigned int chNum,
                       unsigned int paRAMId, DMACCPaRAMEntry* newPaRAM) {
    unsigned int i = 0, *ds, *sr = (unsigned int *)newPaRAM;
 
    ds = (unsigned int *)(baseAdd + DMACC_OPT(paRAMId));
    for(i=0;i<DMACC_PARAM_ENTRY_FIELDS;i++) {
        *ds=*sr;
         ds++;
         sr++;
    }
}

//
// Set a particular PaRAM set entry of the specified PaRAM set
//
// param    baseAdd           Memory address of the DMA instance used 
//
// param    paRaMID           PaRAM Id to which the QDMA channel is
//                            mapped to.
//
// param    paRAMEntry        Specify the PaRAM set entry which needs
//                            to be set.
//
// param    newPaRAMEntryVal  The new field setting. Make sure this field is 
//                            packed for setting certain fields in paRAM.
//
//  paRAMEntry can have values:
//
//  DMACC_PARAM_ENTRY_OPT
//  DMACC_PARAM_ENTRY_SRC
//  DMACC_PARAM_ENTRY_ACNT_BCNT
//  DMACC_PARAM_ENTRY_DST
//  DMACC_PARAM_ENTRY_SRC_DST_BIDX
//  DMACC_PARAM_ENTRY_LINK_BCNTRLD
//  DMACC_PARAM_ENTRY_SRC_DST_CIDX
//  DMACC_PARAM_ENTRY_CCNT
// 
// return   None
//
// NB       This API should be used while setting the PaRAM set entry
//          for QDMA channels. If DMAQdmaSetPaRAMEntry() used,
//          it will trigger the QDMA channel before complete
//          PaRAM set entry is written.
//
void DMAQdmaSetPaRAMEntry(unsigned int baseAdd, unsigned int paRAMId,
                            unsigned int paRAMEntry, unsigned int newPaRAMEntryVal) {
    if((paRAMEntry > DMACC_PARAM_ENTRY_OPT) || (paRAMEntry < DMACC_PARAM_ENTRY_CCNT)) {
        HWREG(baseAdd + DMACC_OPT(paRAMId) + (unsigned int)(paRAMEntry * 0x04)) = 
         newPaRAMEntryVal;
    }
}

//
// Get a particular PaRAM entry of the specified PaRAM set
//
// param    baseAdd           Memory address of the DMA instance used 
//
// param    paRaMID           PaRAM Id to which the QDMA channel is
//                            mapped to.
//
// param    paRAMEntry        Specify the PaRAM set entry which needs
//                            to be read.
//
//  paRAMEntry can have values:
//
//  DMACC_PARAM_ENTRY_OPT
//  DMACC_PARAM_ENTRY_SRC
//  DMACC_PARAM_ENTRY_ACNT_BCNT
//  DMACC_PARAM_ENTRY_DST
//  DMACC_PARAM_ENTRY_SRC_DST_BIDX
//  DMACC_PARAM_ENTRY_LINK_BCNTRLD
//  DMACC_PARAM_ENTRY_SRC_DST_CIDX
//  DMACC_PARAM_ENTRY_CCNT
//
// return   paRAMEntryVal     The value of the paRAM field pointed by the  
//                            paRAMEntry.
//
// NB       This API should be used while reading the PaRAM set entry
//          for QDMA channels. And the paRAMEntryVal is a packed value for
//          certain fields of paRAMEntry.The user has to make sure the value 
//          is unpacked appropriately.
//          For example, the third field is A_B_CNT. Hence he will have to
//          unpack it to two 16 bit fields to get ACNT and BCNT.
//
unsigned int DMAQdmaGetPaRAMEntry(unsigned int baseAdd, unsigned int paRAMId,
                                    unsigned int paRAMEntry) {
    unsigned int paRAMEntryVal = 0;
    if((paRAMEntry > DMACC_PARAM_ENTRY_OPT) || (paRAMEntry < DMACC_PARAM_ENTRY_CCNT)) {
        paRAMEntryVal = HWREG(baseAdd + DMACC_OPT(paRAMId) +
                                (unsigned int)(paRAMEntry * 0x04));
    }
    return(paRAMEntryVal);
}

//
//   Request a DMA/QDMA/Link channel.
//
//  Each channel (DMA/QDMA/Link) must be requested  before initiating a DMA
//  transfer on that channel.
//
//  This API is used to allocate a logical channel (DMA/QDMA/Link) along with
//  the associated resources. For DMA and QDMA channels, TCC and PaRAM Set are
//  also allocated along with the requested channel.
//  
//  User can request a specific logical channel by passing the channel number
//  in 'chNum'.
//  
//  For DMA/QDMA channels, after allocating all the DMA resources, this API
//  sets the TCC field of the OPT PaRAM Word with the allocated TCC. It also sets
//  the event queue for the channel allocated. The event queue needs to be
//  specified by the user.
//  
//  For DMA channel, it also sets the DCHMAP register.
//  
//  For QDMA channel, it sets the QCHMAP register and CCNT as trigger word and
//  enables the QDMA channel by writing to the QEESR register.
//
//  param   baseAdd                  Memory address of the DMA instance used 
//
//  param   chtype                   (DMA/QDMA) Channel
//                                    For Example: For DMA it is
//                                    DMA_CHANNEL_TYPE_DMA 
//
//  param   chNum                    This is the channel number requested for a
//                                   particular event 
//
//  param   tccNum                   The channel number on which the
//                                   completion/error interrupt is generated.
//                                   Not used if user requested for a Link
//                                   channel 
//
//  param   evtQNum                  Event Queue Number to which the channel
//                                   will be mapped (valid only for the
//                                   Master Channel (DMA/QDMA) request) 
//
//  return   TRUE if parameters are valid, else FALSE
//
unsigned int DMARequestChannel(unsigned int baseAdd, unsigned int chType, 
               unsigned int chNum, unsigned int tccNum, unsigned int evtQNum) {
    unsigned int retVal = FALSE;

    if (chNum < SOC_DMA_NUM_DMACH) {
        // Enable the DMA channel in the enabled in the shadow region
        // specific register
        DMAEnableChInShadowReg(baseAdd, chType, chNum);
        DMAMapChToEvtQ( baseAdd, chType, chNum, evtQNum);
        if (DMA_CHANNEL_TYPE_DMA == chType) {
            // Interrupt channel nums are < 32
            if (tccNum < SOC_DMA_NUM_DMACH) {
            // Enable the Event Interrupt
                DMAEnableEvtIntr(baseAdd, chNum);
                retVal = TRUE;
            }
            HWREG(baseAdd + DMACC_OPT(chNum)) &= DMACC_OPT_TCC_CLR;
            HWREG(baseAdd + DMACC_OPT(chNum)) |= DMACC_OPT_TCC_SET(tccNum);
        } else if (DMA_CHANNEL_TYPE_QDMA== chType) {
            // Interrupt channel nums are < 8 //
            if (tccNum < SOC_DMA_NUM_QDMACH) {
                // Enable the Event Interrupt
                DMAEnableEvtIntr(baseAdd, chNum);
                retVal = TRUE;
            } 
            HWREG(baseAdd + DMACC_OPT(chNum)) &= DMACC_OPT_TCC_CLR;
            HWREG(baseAdd + DMACC_OPT(chNum)) |= DMACC_OPT_TCC_SET(tccNum);
        }
    }
    return retVal;
}

//
//   Free the specified channel (DMA/QDMA/Link) and its associated
//            resources (PaRAM Set, TCC etc) and removes various mappings.
//  
//  For Link channels, this API only frees the associated PaRAM Set.
//  
//  For DMA/QDMA channels, it does the following operations:
//  1) Disable any ongoing transfer on the channel,\n
//  2) Remove the channel to Event Queue mapping,\n
//  3) For DMA channels, clear the DCHMAP register, if available\n
//  4) For QDMA channels, clear the QCHMAP register,\n
//  5) Frees the DMA/QDMA channel in the end 
//
//  param   baseAdd                  Memory address of the DMA instance used 
//
//  param   chtype              (DMA/QDMA) Channel
//                     For Example: For QDMA it is,
//                     DMA_CHANNEL_TYPE_QDMA 
//
//  param   chNum                    This is the channel number requested for a
//                      particular event 
//
//  param   trigMode                 Mode of triggering start of transfer 
//
//  param   tccNum                   The channel number on which the
//                                   completion/error interrupt is generated.
//                                   Not used if user requested for a Link
//                                   channel 
//
//  param   evtQNum                  Event Queue Number to which the channel
//                                   will be unmapped (valid only for the
//                                   Master Channel (DMA/QDMA) request) 
//
//  trigMode can have values:
//        DMA_TRIG_MODE_MANUAL\n
//        DMA_TRIG_MODE_QDMA\n
//        DMA_TRIG_MODE_EVENT
//
//  return   TRUE if parameters are valid else return FALSE
//
unsigned int DMAFreeChannel(unsigned int baseAdd, unsigned int chType, 
                              unsigned int chNum, unsigned int trigMode,
                              unsigned int tccNum, unsigned int evtQNum) {
    unsigned int retVal = FALSE;

    if (chNum < SOC_DMA_NUM_DMACH) {
        DMADisableTransfer(baseAdd, chNum, trigMode);
        // Disable the DMA channel in the shadow region specific register 
        DMADisableChInShadowReg(baseAdd, chType, chNum);
        DMAUnmapChToEvtQ( baseAdd, chType, chNum);
        if (DMA_CHANNEL_TYPE_DMA == chType) {
            // Interrupt channel nums are < 32
            if (tccNum < SOC_DMA_NUM_DMACH) {
            // Disable the DMA Event Interrupt
                DMADisableEvtIntr(baseAdd, chNum);
                retVal = TRUE;
            }
        } else if (DMA_CHANNEL_TYPE_QDMA== chType) {
            // Interrupt channel nums are < 8 //
            if (tccNum < SOC_DMA_NUM_QDMACH) {
                // Disable the QDMA Event Interrupt
                DMADisableEvtIntr(baseAdd, chNum);
                retVal = TRUE;
            }
        }
    }
    return retVal;
}

//
//   Start DMA transfer on the specified channel.
//
//  There are multiple ways to trigger an DMA transfer. The triggering mode
//  option allows choosing from the available triggering modes: Event,
//  Manual or QDMA.
//  
//  In event triggered, a peripheral or an externally generated event triggers
//  the transfer. This API clears the Event and Event Miss Register and then
//  enables the DMA channel by writing to the EESR.
//  
//  In manual triggered mode, CPU manually triggers a transfer by writing a 1
//  in the Event Set Register ESR. This API writes to the ESR to start the 
//  transfer.
//  
//  In QDMA triggered mode, a QDMA transfer is triggered when a CPU (or other
//  DMA programmer) writes to the trigger word of the QDMA channel PaRAM set
//  (auto-triggered) or when the DMACC performs a link update on a PaRAM set
//  that has been mapped to a QDMA channel (link triggered). This API enables
//  the QDMA channel by writing to the QEESR register.
//
//  param   baseAdd         Memory address of the DMA instance used 
//
//  param   chNum           Channel being used to enable transfer 
//
//  param   trigMode        Mode of triggering start of transfer (Manual,
//                          QDMA or Event) 
//
//  trigMode can have values:
//        DMA_TRIG_MODE_MANUAL\n
//        DMA_TRIG_MODE_QDMA\n
//        DMA_TRIG_MODE_EVENT\n
//
//  return   retVal         TRUE or FALSE depending on the param passed 
//
//
unsigned int DMAEnableTransfer(unsigned int baseAdd, unsigned int chNum, 
                                 unsigned int trigMode) {
    unsigned int retVal = FALSE;

    switch (trigMode) {
        case DMA_TRIG_MODE_MANUAL :
            if (chNum < SOC_DMA_NUM_DMACH) {
                DMASetEvt(baseAdd, chNum);
                retVal = TRUE;
            }
        break;
        case DMA_TRIG_MODE_QDMA :
            if (chNum < SOC_DMA_NUM_QDMACH) {
                DMAEnableQdmaEvt(baseAdd, chNum);
                retVal = TRUE;
	    }
        break;
        case DMA_TRIG_MODE_EVENT :
            if (chNum < SOC_DMA_NUM_DMACH) {
                //clear SECR & EMCR to clean any previous NULL request
                DMAClrMissEvt(baseAdd, chNum);
                // Set EESR to enable event                               //
                DMAEnableDmaEvt(baseAdd, chNum);
                retVal = TRUE;
            }
        break;
        default :
            retVal = FALSE;
        break;
    }
    return retVal;
}

//
//  Disable DMA transfer on the specified channel
//  
//  There are multiple ways by which an DMA transfer could be triggered.
//  The triggering mode option allows choosing from the available triggering
//  modes.
//  
//  To disable a channel which was previously triggered in manual mode,
//  this API clears the Secondary Event Register and Event Miss Register,
//  if set, for the specific DMA channel.
//  
//  To disable a channel which was previously triggered in QDMA mode, this
//  API clears the QDMA Event Enable Register, for the specific QDMA channel.
//  
//  To disable a channel which was previously triggered in event mode, this API
//  clears the Event Enable Register, Event Register, Secondary Event Register
//  and Event Miss Register, if set, for the specific DMA channel.
//
//
//  param   baseAdd         Memory address of the DMA instance used 
//
//  param   chNum           Channel being used to enable transfer 
//
//  param   trigMode        Mode of triggering start of transfer (Manual,
//                          QDMA or Event) 
//
//  trigMode can have values:
//        DMA_TRIG_MODE_MANUAL
//        DMA_TRIG_MODE_QDMA
//        DMA_TRIG_MODE_EVENT
//
//  return   retVal         TRUE or FALSE depending on the param passed 
//
unsigned int DMADisableTransfer(unsigned int baseAdd, unsigned int chNum, 
                                  unsigned int trigMode) {
    unsigned int retVal = FALSE;

    switch (trigMode) {
        case DMA_TRIG_MODE_MANUAL :
            if (chNum < SOC_DMA_NUM_DMACH) {
                DMAClrEvt(baseAdd, chNum);
                retVal = TRUE;
            }
        break;
        case DMA_TRIG_MODE_QDMA :
            if (chNum < SOC_DMA_NUM_QDMACH) {
                DMADisableQdmaEvt(baseAdd, chNum);
                retVal = TRUE;
            }
        break;
	case DMA_TRIG_MODE_EVENT :
            if (chNum < SOC_DMA_NUM_DMACH) {
                //clear SECR & EMCR to clean any previous NULL request
                DMAClrMissEvt(baseAdd, chNum);
                // Set EESR to enable event                               //
                DMADisableDmaEvt(baseAdd, chNum);
                retVal = TRUE;
            }
        break;
	default :
            retVal = FALSE;
        break;
    }
    return retVal;
}


//
//    Clears Event Register and Error Register for a specific
//          DMA channel and brings back DMA to its initial state.
//
//  This API clears the Event register, Event Miss register, Event Enable
//  register for a specific DMA channel. It also clears the CC Error register.
//
//  param   baseAdd         Memory address of the DMA instance used 
//
//  param   chNum           This is the channel number requested for a
//                          particular event 
//
//  param   evtQNum         Event Queue Number to which the channel
//                          will be unmapped (valid only for the
//                          Master Channel (DMA/QDMA) request) 
//
//  return  none 
//
void DMAClearErrorBits(unsigned int baseAdd, unsigned int chNum, 
                        unsigned int evtQNum) {

    if(chNum < SOC_DMA_NUM_DMACH) {
         if(chNum < 32) {
              HWREG(baseAdd + DMACC_S_EECR(regionId)) = (0x01u << chNum);
              // Write to EMCR to clear the corresponding EMR bit
              HWREG(baseAdd + DMACC_EMCR) = (0x01u << chNum);
              // Clears the SER
              HWREG(baseAdd + DMACC_S_SECR(regionId)) = (0x01u << chNum);
         } else {
              HWREG(baseAdd + DMACC_S_EECRH(regionId)) = (0x01u << (chNum - 32));
              // Write to EMCR to clear the corresponding EMR bit
              HWREG(baseAdd + DMACC_EMCRH) = (0x01u << (chNum - 32));
              // Clear the SER
              HWREG(baseAdd + DMACC_S_SECRH(regionId)) = (0x01u << (chNum - 32));
         }
    }
    // Clear global CC Error Register
    if (0 == evtQNum) {
         HWREG(baseAdd + DMACC_CCERRCLR) &= (DMACC_CCERRCLR_QTHRXCD0 | 
                                DMACC_CCERRCLR_TCCERR);
    } else if(1 == evtQNum) {
         HWREG(baseAdd + DMACC_CCERRCLR) &= (DMACC_CCERRCLR_QTHRXCD1 | 
                                DMACC_CCERRCLR_TCCERR);
    } else if(2 == evtQNum) {
         HWREG(baseAdd + DMACC_CCERRCLR) &= (DMACC_CCERRCLR_QTHRXCD2 | 
                                DMACC_CCERRCLR_TCCERR);
    } else if(3 == evtQNum) {
         HWREG(baseAdd + DMACC_CCERRCLR) &= (DMACC_CCERRCLR_QTHRXCD2 | 
                                DMACC_CCERRCLR_TCCERR);
    }
}

//
//  This API enables to evaluate the subsequent errors. On writing 
//           to the EEVAL register DMACC error interrupt will be reasserted, 
//           if there are any outstanding error bits set due to subsequent 
//           error conditions.
//
//  param   baseAddr         Memory address of the DMA instance used.
//
//  return   None
//
void DMACCErrorEvaluate(unsigned int baseAddr) {

    HWREG(baseAddr + DMACC_EEVAL) = DMACC_EEVAL_EVAL << DMACC_EEVAL_EVAL_SHIFT;
}

//
//  DMA Deinitialization
//  
//  This function deinitializes the DMA Driver
//  Clears the error specific registers (EMCR/EMCRh, QEMCR, CCERRCLR) &
//  deinitialize the Queue Number Registers
//  
//  param   baseAdd         Memory address of the DMA instance used 
//  
//  param   queNum          Event Queue used 
//  
//  return   None
//  
//  NB        The regionId is the shadow region(0 or 1) used and the,
//            Event Queue used is either (0 or 1). There are only two shadow regions
//            and only two event Queues
//
void DMADeinit(unsigned int baseAdd, unsigned int queNum) {
    unsigned int count = 0;

    // Disable the DMA (0 - 62) channels in the DRAE register
    HWREG(baseAdd + DMACC_DRAE(regionId)) = DMA_CLR_ALL_BITS;
    HWREG(baseAdd + DMACC_DRAEH(regionId)) = DMA_CLR_ALL_BITS;
    DMAClrCCErr(baseAdd, DMACC_CLR_TCCERR);
    // Clear the Event miss Registers
    HWREG(baseAdd + DMACC_EMCR)  = DMA_SET_ALL_BITS;
    HWREG(baseAdd + DMACC_EMCRH) = DMA_SET_ALL_BITS;
    // Clear CCERR register
    HWREG(baseAdd + DMACC_CCERRCLR) = DMA_SET_ALL_BITS;
    // Deinitialize the Queue Number Registers
    for (count = 0;count < SOC_DMA_NUM_DMACH; count++) {
        HWREG(baseAdd + DMACC_DMAQNUM(count >> 3u)) &= DMACC_DMAQNUM_CLR(count);
    }
    for (count = 0;count < SOC_DMA_NUM_QDMACH; count++) {
        HWREG(baseAdd + DMACC_QDMAQNUM) &= DMACC_QDMAQNUM_CLR(count);
    }
}

//
//  This API can be used to save the register context for DMA
//
//  param   baseAdd         Memory address of the DMA instance used 
//
//  param   edmaCntxPtr     Pointer to the structure where the context
//                          needs to be saved.
//
//  return   None
//
void DMAContextSave(unsigned int baseAddr, DMACONTEXT *edmaCntxPtr) {
    unsigned int i;
    unsigned int maxPar;

    // Get the Channel mapping reg Val
    for(i = 0; i < 64; i++) {
         // All events are one to one mapped with the channels
         edmaCntxPtr->dchMap[i] = HWREG(baseAddr + DMACC_DCHMAP(i));
    }
    // Get DMA Queue Number Register Val
    for(i=0; i < 8; i++) {   
        edmaCntxPtr->dmaQNum[i] = HWREG(baseAddr + DMACC_DMAQNUM((i)));    
    }         
    // Get the DMA Region Access Enable Register val
    edmaCntxPtr->regAccEnableLow = HWREG(baseAddr + DMACC_DRAE(0));            
    edmaCntxPtr->regAccEnableHigh = HWREG(baseAddr + DMACC_DRAEH(0));            
    // Get Event Set Register value
    edmaCntxPtr->eventSetRegLow  = HWREG(baseAddr + DMACC_S_ESR(0));                
    edmaCntxPtr->eventSetRegHigh = HWREG(baseAddr + DMACC_S_ESRH(0));                    
    // Get Event Enable Set Register value
    edmaCntxPtr->enableEvtSetRegLow = HWREG(baseAddr + DMACC_S_EER(0));                   
    edmaCntxPtr->enableEvtSetRegHigh = HWREG(baseAddr + DMACC_S_EERH(0));                       
    // Get Interrupt Enable Set Register value
    edmaCntxPtr->intEnableSetRegLow =  HWREG(baseAddr + DMACC_S_IER(0));                    
    edmaCntxPtr->intEnableSetRegHigh =  HWREG(baseAddr + DMACC_S_IERH(0));                        
    maxPar = 128;
    if((DMA_REVID_AM335X == DMAVersionGet())) maxPar = 256;
    for(i = 0; i < maxPar; i++) {
        // Get the  PaRAM  values
        DMAGetPaRAM(baseAddr, i, 
                      (struct DMACCPaRAMEntry *)(&(edmaCntxPtr->dmaParEntry[i])));
    }	                      
}

//
//  This API can be used to restore the register context for DMA
//
//  param   baseAdd         Memory address of the DMA instance used 
//
//  param   edmaCntxPtr     Pointer to the structure where the context
//                          needs to be restored from.
//
//  return   None
//
void DMAContextRestore(unsigned int baseAddr, DMACONTEXT *edmaCntxPtr) {
    unsigned int i;
    unsigned int maxPar;

    // set the Channel mapping reg Val
    for(i = 0; i < 64; i++) {
         // All events are one to one mapped with the channels
         HWREG(baseAddr + DMACC_DCHMAP(i)) = edmaCntxPtr->dchMap[i];
    }
    // set DMA Queue Number Register Val
    for(i=0; i < 8; i++) {
	HWREG(baseAddr + DMACC_DMAQNUM((i))) = edmaCntxPtr->dmaQNum[i];    
    }         
    // set the DMA Region Access Enable Register val
    HWREG(baseAddr + DMACC_DRAE(0)) = edmaCntxPtr->regAccEnableLow;
    HWREG(baseAddr + DMACC_DRAEH(0)) = edmaCntxPtr->regAccEnableHigh;            
    // set Event Set Register value
    HWREG(baseAddr + DMACC_S_ESR(0)) = edmaCntxPtr->eventSetRegLow;                
    HWREG(baseAddr + DMACC_S_ESRH(0)) = edmaCntxPtr->eventSetRegHigh;                    
    // set Event Enable Set Register value
    HWREG(baseAddr + DMACC_S_EESR(0)) = edmaCntxPtr->enableEvtSetRegLow;                   
    HWREG(baseAddr + DMACC_S_EESRH(0)) = edmaCntxPtr->enableEvtSetRegHigh;                       
    // set Interrupt Enable Set Register value
    HWREG(baseAddr + DMACC_S_IESR(0)) = edmaCntxPtr->intEnableSetRegLow;                    
    HWREG(baseAddr + DMACC_S_IESRH(0)) = edmaCntxPtr->intEnableSetRegHigh;                        
    maxPar = 128;
    if((DMA_REVID_AM335X == DMAVersionGet())) maxPar = 256;
    for(i = 0; i < maxPar; i++) {
        // Get the  PaRAM  values
        DMASetPaRAM(baseAddr, i, 
             (struct DMACCPaRAMEntry *)(&(edmaCntxPtr->dmaParEntry[i])));
    }	                   
}

// added from platform/beaglebone/edma.c :

#include "hw_control_AM335x.h"
#include "soc_AM335x.h"
#include "hw_cm_per.h"

//  
// This API returns a unique number which identifies itself  
//         with the DMA IP in AM335x SoC.  
// param   None  
// return  This returns a number '2' which is unique to DMA IP in AM335x.
//
unsigned int DMAVersionGet(void) {

    return 2;
}

void dma_clk_cfg(void) {

    // Configuring clocks for DMA TPCC and TPTCs
    // Writing to MODULEMODE field of CM_PER_TPCC_CLKCTRL register
    HWREG(SOC_CM_PER_REGS + CM_PER_TPCC_CLKCTRL) |=
          CM_PER_TPCC_CLKCTRL_MODULEMODE_ENABLE;
    // Waiting for MODULEMODE field to reflect the written value
    while(CM_PER_TPCC_CLKCTRL_MODULEMODE_ENABLE !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_TPCC_CLKCTRL) &
          CM_PER_TPCC_CLKCTRL_MODULEMODE));
    // Writing to MODULEMODE field of CM_PER_TPTC0_CLKCTRL register
    HWREG(SOC_CM_PER_REGS + CM_PER_TPTC0_CLKCTRL) |=
          CM_PER_TPTC0_CLKCTRL_MODULEMODE_ENABLE;
    // Waiting for MODULEMODE field to reflect the written value
    while(CM_PER_TPTC0_CLKCTRL_MODULEMODE_ENABLE !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_TPTC0_CLKCTRL) &
           CM_PER_TPTC0_CLKCTRL_MODULEMODE));
    // Writing to MODULEMODE field of CM_PER_TPTC1_CLKCTRL register
    HWREG(SOC_CM_PER_REGS + CM_PER_TPTC1_CLKCTRL) |=
          CM_PER_TPTC1_CLKCTRL_MODULEMODE_ENABLE;
    // Waiting for MODULEMODE field to reflect the written value
    while(CM_PER_TPTC1_CLKCTRL_MODULEMODE_ENABLE !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_TPTC1_CLKCTRL) &
           CM_PER_TPTC1_CLKCTRL_MODULEMODE));
    // Writing to MODULEMODE field of CM_PER_TPTC2_CLKCTRL register
    HWREG(SOC_CM_PER_REGS + CM_PER_TPTC2_CLKCTRL) |=
          CM_PER_TPTC2_CLKCTRL_MODULEMODE_ENABLE;
    // Waiting for MODULEMODE field to reflect the written value
    while(CM_PER_TPTC2_CLKCTRL_MODULEMODE_ENABLE !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_TPTC2_CLKCTRL) &
           CM_PER_TPTC2_CLKCTRL_MODULEMODE));
	//	DMA in non-idle mode
	HWREG(0x49800010) = 0x00000028;
	HWREG(0x49900010) = 0x00000028;
	HWREG(0x49a00010) = 0x00000028;
    // Waiting for IDLEST field in CM_PER_TPCC_CLKCTRL
    while((CM_PER_TPCC_CLKCTRL_IDLEST_FUNC <<
           CM_PER_TPCC_CLKCTRL_IDLEST_SHIFT) !=
           (HWREG(SOC_CM_PER_REGS + CM_PER_TPCC_CLKCTRL) &
            CM_PER_TPCC_CLKCTRL_IDLEST));
    // Waiting for IDLEST field in CM_PER_TPTC0_CLKCTRL
    while((CM_PER_TPTC0_CLKCTRL_IDLEST_FUNC <<
           CM_PER_TPTC0_CLKCTRL_IDLEST_SHIFT) !=
           (HWREG(SOC_CM_PER_REGS + CM_PER_TPTC0_CLKCTRL) &
            CM_PER_TPTC0_CLKCTRL_IDLEST));
    // Waiting for STBYST field in CM_PER_TPTC0_CLKCTRL
    while((CM_PER_TPTC0_CLKCTRL_STBYST_FUNC <<
           CM_PER_TPTC0_CLKCTRL_STBYST_SHIFT) !=
           (HWREG(SOC_CM_PER_REGS + CM_PER_TPTC0_CLKCTRL) &
            CM_PER_TPTC0_CLKCTRL_STBYST));
    // Waiting for IDLEST field in CM_PER_TPTC1_CLKCTRL
    while((CM_PER_TPTC1_CLKCTRL_IDLEST_FUNC <<
           CM_PER_TPTC1_CLKCTRL_IDLEST_SHIFT) !=
           (HWREG(SOC_CM_PER_REGS + CM_PER_TPTC1_CLKCTRL) &
            CM_PER_TPTC1_CLKCTRL_IDLEST));
    // Waiting for STBYST field in CM_PER_TPTC1_CLKCTRL
    while((CM_PER_TPTC1_CLKCTRL_STBYST_FUNC <<
           CM_PER_TPTC1_CLKCTRL_STBYST_SHIFT) !=
           (HWREG(SOC_CM_PER_REGS + CM_PER_TPTC1_CLKCTRL) &
            CM_PER_TPTC1_CLKCTRL_STBYST));
    // Waiting for IDLEST field in CM_PER_TPTC2_CLKCTRL
    while((CM_PER_TPTC2_CLKCTRL_IDLEST_FUNC <<
           CM_PER_TPTC2_CLKCTRL_IDLEST_SHIFT) !=
           (HWREG(SOC_CM_PER_REGS + CM_PER_TPTC2_CLKCTRL) &
            CM_PER_TPTC2_CLKCTRL_IDLEST));
    // Waiting for STBYST field in CM_PER_TPTC2_CLKCTRL
    while((CM_PER_TPTC2_CLKCTRL_STBYST_FUNC <<
           CM_PER_TPTC2_CLKCTRL_STBYST_SHIFT) !=
           (HWREG(SOC_CM_PER_REGS + CM_PER_TPTC2_CLKCTRL) &
            CM_PER_TPTC2_CLKCTRL_STBYST));
}

// eof
