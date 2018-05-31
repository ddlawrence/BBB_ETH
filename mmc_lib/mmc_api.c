//
//  Multi Media Card - Application Program Interface
//  
//  originally written by TIdude
//  these functions call mmc_hwif routines
//
#include "bbb_main.h"
#include "string.h"
#include "dma.h"
#include "dma_event.h"
#include "ff.h"
#include "mmc_hwif.h"
#include "mmc_api.h"
#include "con_uif.h"
#include "hw_control_AM335x.h"
#include "soc_AM335x.h"

#define EVT_QUEUE_NUM            0         // DMA Event queue number - MMC
#define REGION_NUMBER            0         // DMA Region number - MMC
#define MMC_BLK_SIZE             512
#define MMC_DATA_SIZE            512       // Global data pointer
#define MMC_IN_FREQ              96000000  // 96MHz
#define MMC_INIT_FREQ            400000    // 400kHz 
#define MMC_CARD_DETECT_PINNUM   6


// Fat devices registered 
#ifndef fatDevice
typedef struct _fatDevice {
    void *dev;               // Pointer to underlying device/controller 
    FATFS *fs;               // File system pointer 
    unsigned int initDone;   // state 
}fatDevice;
#endif
extern fatDevice fat_devices[2];

mmcCardInfo sdCard;          // SD card info struc 
mmcCtrlInfo ctrlInfo;        // Controller info struc

// DMA callback function array - MMC
static void (*cb_Fxn[DMA_NUM_TCC]) (unsigned int tcc);

extern char *CwdBuf;

// Global flags for interrupt handling - MMC
volatile unsigned int sdBlkSize = MMC_BLK_SIZE;
volatile unsigned int callbackOccured = 0; 
volatile unsigned int xferCompFlag = 0; 
volatile unsigned int dataTimeout = 0;
volatile unsigned int cmdCompFlag = 0;
volatile unsigned int cmdTimeout = 0; 
volatile unsigned int errFlag = 0;

#define DATA_RESPONSE_WIDTH       (SOC_CACHELINE_SIZE)
// Cache size aligned data buffer (minimum of 64 bytes) for command response
static unsigned char dataBuffer[DATA_RESPONSE_WIDTH]
                               __attribute__((aligned(SOC_CACHELINE_SIZE)));

// Current FAT fs state
FATFS FatFsObj  __attribute__ ((aligned (SOC_CACHELINE_SIZE)));
DIR DirObject;
FILINFO FileInfoObject;
FIL FileObject  __attribute__ ((aligned (SOC_CACHELINE_SIZE)));
FIL fileObjectWrite  __attribute__ ((aligned (SOC_CACHELINE_SIZE)));

//
// check mmc0 command status
//
// wait for cmd completion flag   which is set by isr
//
unsigned int mmc0_get_cmd_status(mmcCtrlInfo *ctrl) {
  unsigned int status = 0;

  while ((cmdCompFlag == 0) && (cmdTimeout == 0));  // block here
  if (cmdCompFlag) {
    status = 1;
    cmdCompFlag = 0;
  }
  if (cmdTimeout) {
    status = 0;
    cmdTimeout = 0;
  }
  return status;
}

//
// check mmc0 transfer status
//
// wait for transfer completion flag   which is set by isr
//
unsigned int mmc0_get_xfer_status(mmcCtrlInfo *ctrl) {
  unsigned int status = 0;
  volatile unsigned int timeOut = 0xFFFF;

  while ((xferCompFlag == 0) && (dataTimeout == 0));  // block here
  if (xferCompFlag) {
    status = 1;
    xferCompFlag = 0;
  }
  if (dataTimeout) {
    status = 0;
    dataTimeout = 0;
  }
  if (HWREG(ctrl->memBase + MMC_CMD) & MMC_CMD_DP) {  // poll for callback 
    nothing(); //  this is necessary prog hangs  - TODO
    while(callbackOccured == 0 && ((timeOut--) != 0));  // block here
    callbackOccured = 0;
    if(timeOut == 0) status = 0;
  }
  ctrlInfo.dmaEnable = 0;
  return status;
}

//
// setup a DMA receive transfer - MMC
//
void MMCRxDMAsetup(void *ptr, unsigned int blkSize, unsigned int nblks) {
  DMACCPaRAMEntry paramSet;
  paramSet.srcAddr    = ctrlInfo.memBase + MMC_DATA;
  paramSet.destAddr   = (unsigned int)ptr;
  paramSet.srcBIdx    = 0;
  paramSet.srcCIdx    = 0;
  paramSet.destBIdx   = 4;
  paramSet.destCIdx   = (unsigned short)blkSize;
  paramSet.aCnt       = 0x4;
  paramSet.bCnt       = (unsigned short)blkSize/4;
  paramSet.cCnt       = (unsigned short)nblks;
  paramSet.bCntReload = 0x0;
  paramSet.linkAddr   = 0xffff;
  paramSet.opt        = 0;
  paramSet.opt |= ((DMA_CHA_MMC0_RX << DMACC_OPT_TCC_SHIFT) & DMACC_OPT_TCC);
  paramSet.opt |= (1 << DMACC_OPT_TCINTEN_SHIFT);  // Tx complete irq enab
  paramSet.opt |= (1 << 0);  // read FIFO : SRC Constant addr mode
  paramSet.opt |= (2 << 8);  // SRC FIFO width 32 bit 
  paramSet.opt |= (1 << 2);  // AB-Sync mode 
  DMASetPaRAM(SOC_DMA0CC_0_REGS, DMA_CHA_MMC0_RX, &paramSet);
  DMAEnableTransfer(SOC_DMA0CC_0_REGS, DMA_CHA_MMC0_RX, DMA_TRIG_MODE_EVENT);
}

//
// setup a DMA transmit transfer - MMC
//
void MMCTxDMAsetup(void *ptr, unsigned int blkSize, unsigned int blks) {
  DMACCPaRAMEntry paramSet;
  paramSet.srcAddr    = (unsigned int)ptr;
  paramSet.destAddr   = ctrlInfo.memBase + MMC_DATA;
  paramSet.srcBIdx    = 4;
  paramSet.srcCIdx    = blkSize;
  paramSet.destBIdx   = 0;
  paramSet.destCIdx   = 0;
  paramSet.aCnt       = 0x4;
  paramSet.bCnt       = (unsigned short)blkSize/4;
  paramSet.cCnt       = (unsigned short)blks;
  paramSet.bCntReload = 0x0;
  paramSet.linkAddr   = 0xffff;
  paramSet.opt        = 0;
  paramSet.opt |= ((DMA_CHA_MMC0_TX << DMACC_OPT_TCC_SHIFT) & DMACC_OPT_TCC);
  paramSet.opt |= (1 << DMACC_OPT_TCINTEN_SHIFT);  // Tx complete irq enab
  paramSet.opt |= (1 << 1);  // read FIFO : DST constant addr mode 
  paramSet.opt |= (2 << 8);  // DST FIFO width 32 bit 
  paramSet.opt |= (1 << 2);  // AB-Sync mode
  DMASetPaRAM(SOC_DMA0CC_0_REGS, DMA_CHA_MMC0_TX, &paramSet);
  DMAEnableTransfer(SOC_DMA0CC_0_REGS, DMA_CHA_MMC0_TX, DMA_TRIG_MODE_EVENT);
}

//
//  setup a DMA transfer - MMC
//
void MMCXferSetup(mmcCtrlInfo *ctrl, unsigned char rwFlag, void *ptr,
                  unsigned int blkSize, unsigned int nBlks) {
  callbackOccured = 0;
  xferCompFlag = 0;
  if (rwFlag == 1) MMCRxDMAsetup(ptr, blkSize, nBlks);
  else MMCTxDMAsetup(ptr, blkSize, nBlks);

  ctrl->dmaEnable = 1;
  MMCBlkLenSet(ctrl->memBase, blkSize);
}

//
// MMC0 Send Cmd
//
unsigned int MMC0CmdSend(mmcCtrlInfo *ctrl, mmcCmd *c) {
  unsigned int cmd, cmdType, rspType, cmdDir, nblks;
  unsigned int dataPresent, status = 0;

  // set CMD_TYPE field of SD_CMD register   see spruh73n Table 18-24
  cmdType = 0x0;               // other commands
  if (c->flags & 0x2) {
    cmdType = 0x1 < 0x16;      // bus suspend
  } else if (c->flags & 0x4) {
    cmdType = 0x2 << 0x16;     // function select
  } else if (c->flags & 0x8) {
    cmdType = 0x3 << 0x16;     // i/o abort
  }
  // set DDIR field of SD_CMD register   see spruh73n Table 18-24
  if (c->flags & 0x80) cmdDir = 0x1 << 0x4;  // Read Card to Host
  else cmdDir = 0x0;                         // Write Host to Card
  if (c->flags & 0x40) {  // data present  (DP field)
    dataPresent = 0x1;
    nblks = c->nblks;
  } else {
    dataPresent = 0x0;
    nblks = 0;
  }
  // set field RSP_TYPE of SD_CMD register    see spruh73n Table 18-24
  if (c->flags & 0x1) {
    rspType = 0x0;             // no response
  } else if (c->flags & 0x20) {
    rspType = 1 << 0x10;       // response length 136 bits
  } else if (c->flags & 0x10) {
    rspType = 3 << 0x10;       // response length 48 bits with BUSY
  } else rspType = 2 << 0x10;  // response length 48 bits
  cmd = (c->idx << 0x18) | cmdType | rspType | cmdDir;  // spruh73n 18.4.1.10
  if (dataPresent) {
    mmc0_clear_status(MMC_STAT_TC);
    mmc0_set_dto(27);  // set data_timeout  prob only need to do this only in init - TODO
            // data present select   bit21
            // multi/single block select   bit5
            // block count enable   bit1
    cmd |= (MMC_CMD_DP | MMC_CMD_MSBS | MMC_CMD_BCE);
  }
  if (1 == ctrl->dmaEnable) cmd |= MMC_CMD_DE;  // dma enable bit0 spruh73n 18.4.1.10
  mmc0_send_cmd(cmd, c->arg, nblks);
  status = mmc0_get_cmd_status(ctrl);  // a flag routine
  if (status == 1) mmc0_get_resp(c->rsp);
  return status;
}

//
// MMC Send App Cmd
//
unsigned int MMCAppCmdSend(mmcCtrlInfo *ctrl, mmcCmd *c) {
  unsigned int status = 0;
  mmcCmd capp;

  capp.idx = SD_CMD(55);  // APP cmd preceded by CMD55
  capp.flags = 0;
  capp.arg = ctrl->card->rca << 16;
  status = MMC0CmdSend(ctrl, &capp);
  if (status == 0) return 0;  // CMD55 failed must return
  status = MMC0CmdSend(ctrl, c);
  return status;
}

//
// MMC Set Bus Width
//
unsigned int MMCBusWidthSet(mmcCtrlInfo *ctrl) {
  mmcCardInfo *card = ctrl->card;
  unsigned int status = 0;
  mmcCmd capp;

  capp.idx = SD_CMD(6);
  capp.arg = SD_BUS_WIDTH_1BIT;
  capp.flags = 0;
  if (ctrl->busWidth & SD_BUS_WIDTH_4BIT) {
    if (card->busWidth & SD_BUS_WIDTH_4BIT) {
      capp.arg = SD_BUS_WIDTH_4BIT;
    }
  }
  capp.arg = capp.arg >> 1;
  status = MMCAppCmdSend(ctrl, &capp);
  if (1 == status) {
    if (capp.arg == 0) MMCSetBusWidth(ctrl->memBase, SD_BUS_WIDTH_1BIT);
    else MMCSetBusWidth(ctrl->memBase, SD_BUS_WIDTH_4BIT);  // 4 bit width only
  }
  return status;
}

//
// MMC Set Xfer Speed
//
unsigned int MMCTranSpeedSet(mmcCtrlInfo *ctrl) {
  mmcCardInfo *card = ctrl->card;
  mmcCmd cmd;
  int status;
  unsigned int speed, cmdStatus = 0;

  MMCXferSetup(ctrl, 1, dataBuffer, 64, 1);
  cmd.idx = SD_CMD(6);
  cmd.arg = ((SD_SWITCH_MODE & SD_CMD6_GRP1_SEL) | (SD_CMD6_GRP1_HS));
  cmd.flags = SD_CMDRSP_READ | SD_CMDRSP_DATA;
  cmd.nblks = 1;
  cmd.data = (signed char*)dataBuffer;
  cmdStatus = MMC0CmdSend(ctrl, &cmd);
  if (cmdStatus == 0) return 0;
  cmdStatus = mmc0_get_xfer_status(ctrl);
  if (cmdStatus == 0) return 0;
  // Invalidate data cache
  CP15DCacheFlushBuff((unsigned int) dataBuffer, DATA_RESPONSE_WIDTH);
  speed = card->tranSpeed;

// THIS IS HIGHLY SUSPICIOUS WHAT THE F!@#$ IS dataBuffer[16] ?    - TODO
  if ((dataBuffer[16] & 0xF) == SD_CMD6_GRP1_HS) card->tranSpeed = SD_TRANSPEED_50MBPS;
  if (speed == SD_TRANSPEED_50MBPS) {
    status = MMCSetBusFreq(ctrl->memBase, ctrl->ipClk, 50000000, 0);
    ctrl->opClk = 50000000;
  } else {                
// ConsolePrintf("MBPS 25\n");  // transpeed 25 MBPS normally
                                // 50MBPS spoof is no faster!  - TODO
                                // 8 bit bus no pinout - find maybe related
    status = MMCSetBusFreq(ctrl->memBase, ctrl->ipClk, 25000000, 0);
    ctrl->opClk = 25000000;
  }
  if (status != 0) return 0;  // fail
  return 1;
}

//
// MMC Card Reset
//
unsigned int MMCCardReset(mmcCtrlInfo *ctrl) {
  unsigned int status = 0;
  mmcCmd cmd;

  cmd.idx = SD_CMD(0);
  cmd.flags = SD_CMDRSP_NONE;
  cmd.arg = 0;
  status = MMC0CmdSend(ctrl, &cmd);
  return status;  // success 1   fail 0
}

//
// MMC Send Stop Cmd
//
unsigned int MMCStopCmdSend(mmcCtrlInfo *ctrl) {
  unsigned int status = 0;
  mmcCmd cmd;

  cmd.idx  = SD_CMD(12);
  cmd.flags = SD_CMDRSP_BUSY;
  cmd.arg = 0;
  MMC0CmdSend(ctrl, &cmd);
  status = mmc0_get_xfer_status(ctrl);
  return status;  // success 1    fail 0
}

//
// MMC Chk Card Type
//
unsigned int MMCCardTypeCheck(mmcCtrlInfo * ctrl) {
  unsigned int status;
  mmcCmd cmd;

  // Card type can be found by sending CMD55. If card responds,
  // it is a SD card. Else assume it is a MMC Card
  cmd.idx = SD_CMD(55);
  cmd.flags = 0;
  cmd.arg = 0;
  status = MMCAppCmdSend(ctrl, &cmd);
  return status;
}

//
// MMC Card Init
//
unsigned int MMCCardInit(mmcCtrlInfo *ctrl) {
  mmcCardInfo *card = ctrl->card;
  unsigned int retry = 0xFFFF, status = 0;
  mmcCmd cmd;

  memset(ctrl->card, 0, sizeof(mmcCardInfo));
  card->ctrl = ctrl;
  status = MMCCardReset(ctrl);  // CMD0 reset card
  if (status == 0) return 0;
  status = MMCCardTypeCheck(ctrl);  // SDcard 1   non SDcard 0
  if (status == 1) {
    ctrl->card->cardType = MMC_CARD_SD;
    status = MMCCardReset(ctrl);
    if (status == 0) return 0;
    cmd.idx = SD_CMD(8);  // send operating voltage
    cmd.flags = 0;
    cmd.arg = (SD_CHECK_PATTERN | SD_VOLT_2P7_3P6);
    status = MMC0CmdSend(ctrl, &cmd);
    if (status == 0) {
      // cmd fail prob version < 2.0   support high voltage cards only
    }
    // Go ahead and send ACMD41, with host capabilities
    cmd.idx = SD_CMD(41);
    cmd.flags = 0;
    cmd.arg = SD_OCR_HIGH_CAPACITY | SD_OCR_VDD_WILDCARD;
    status = MMCAppCmdSend(ctrl,&cmd);
    if (status == 0) return 0;
    // Poll until card status (BIT31 of OCR) is powered up */
    do {
      cmd.idx = SD_CMD(41);
      cmd.flags = 0;
      cmd.arg = SD_OCR_HIGH_CAPACITY | SD_OCR_VDD_WILDCARD;
      MMCAppCmdSend(ctrl,&cmd);
    } while (!(cmd.rsp[0] & ((unsigned int)BIT(31))) && retry--);
    if (retry == 0) return 0;  // No point in continuing
    card->ocr = cmd.rsp[0];
    card->highCap = (card->ocr & SD_OCR_HIGH_CAPACITY) ? 1 : 0;
    cmd.idx = SD_CMD(2);  // Send CMD2 card id register
    cmd.flags = SD_CMDRSP_136BITS;
    cmd.arg = 0;
    status = MMC0CmdSend(ctrl, &cmd);
    memcpy(card->raw_cid, cmd.rsp, 16);
    if (status == 0) return 0;
    cmd.idx = SD_CMD(3);  // Send CMD3 card relative address
    cmd.flags = 0;
    cmd.arg = 0;
    status = MMC0CmdSend(ctrl, &cmd);
    card->rca = SD_RCA_ADDR(cmd.rsp[0]);
    if (status == 0) return 0;
    cmd.idx = SD_CMD(9);  // get card specific data
    cmd.flags = SD_CMDRSP_136BITS;
    cmd.arg = card->rca << 16;
    status = MMC0CmdSend(ctrl, &cmd);
    memcpy(card->raw_csd, cmd.rsp, 16);
    if (status == 0) return 0;
    if (SD_CARD_CSD_VERSION(card)) {
      card->tranSpeed = SD_CARD1_TRANSPEED(card);
      card->blkLen = 1 << (SD_CARD1_RDBLKLEN(card));
      card->size = SD_CARD1_SIZE(card);
      card->nBlks = card->size / card->blkLen;
    } else {
      card->tranSpeed = SD_CARD0_TRANSPEED(card);
      card->blkLen = 1 << (SD_CARD0_RDBLKLEN(card));
      card->nBlks = SD_CARD0_NUMBLK(card);
      card->size = SD_CARD0_SIZE(card);
    }
    // Set data block length to 512 (for byte addressing cards)
    if( !(card->highCap) ) {
      cmd.idx = SD_CMD(16);
      cmd.flags = SD_CMDRSP_NONE;
      cmd.arg = 512;
      status = MMC0CmdSend(ctrl, &cmd);
      if (status == 0) return 0;
      else card->blkLen = 512;
    }
    cmd.idx = SD_CMD(7);  // card select
    cmd.flags = SD_CMDRSP_BUSY;
    cmd.arg = card->rca << 16;
    status = MMC0CmdSend(ctrl, &cmd);
    if (status == 0) return 0;  // fail
    // Send ACMD51, to get the SD Configuration register details.
    // Note, this needs data transfer (on data lines).
    cmd.idx = SD_CMD(55);
    cmd.flags = 0;
    cmd.arg = card->rca << 16;
    status = MMC0CmdSend(ctrl, &cmd);
    if (status == 0) return 0;  // fail
    MMCXferSetup(ctrl, 1, dataBuffer, 8, 1);
    cmd.idx = SD_CMD(51);
    cmd.flags = SD_CMDRSP_READ | SD_CMDRSP_DATA;
    cmd.arg = card->rca << 16;
    cmd.nblks = 1;
    cmd.data = (signed char*)dataBuffer;
    status = MMC0CmdSend(ctrl, &cmd);
    if (status == 0) return 0;  // fail
    status = mmc0_get_xfer_status(ctrl);
    if (status == 0) return 0;  // fail
    CP15DCacheFlushBuff((unsigned int)dataBuffer, DATA_RESPONSE_WIDTH);
    card->raw_scr[0] = (dataBuffer[3] << 24) | (dataBuffer[2] << 16) | \
                       (dataBuffer[1] << 8) | (dataBuffer[0]);
    card->raw_scr[1] = (dataBuffer[7] << 24) | (dataBuffer[6] << 16) | \
	               (dataBuffer[5] << 8) | (dataBuffer[4]);
    card->sd_ver = SD_CARD_VERSION(card);
    card->busWidth = SD_CARD_BUSWIDTH(card);
  } else return 0;  // fail
  return 1;  // success
}

//
// arg    mmcCtrlInfo      mmc control information
// arg    ptr              address where data is to write
// arg    block            start block where data is to be written
// arg    nblks            number of blocks to write
//
// return  1 success | 0 fail
//
unsigned int MMCWriteCmdSend(mmcCtrlInfo *ctrl, void *ptr, unsigned int block,
                               unsigned int nblks) {
  mmcCardInfo *card = ctrl->card;
  unsigned int status = 0, address;
  mmcCmd cmd;

  // Address is in blks for high cap cards and bytes for std cards
  if (card->highCap) address = block;
  else address = block * card->blkLen;
  CP15DCacheCleanBuff((unsigned int) ptr, (512 * nblks));  // clean data cache
  MMCXferSetup(ctrl, 0, ptr, 512, nblks);
  cmd.flags = SD_CMDRSP_WRITE | SD_CMDRSP_DATA;
  cmd.arg = address;
  cmd.nblks = nblks;
  if (nblks > 1) {
    cmd.idx = SD_CMD(25);
    cmd.flags |= SD_CMDRSP_ABORT;
  } else cmd.idx = SD_CMD(24);
  status = MMC0CmdSend(ctrl, &cmd);
  if (status == 0) return 0;
  status = mmc0_get_xfer_status(ctrl);
  if (status == 0) return 0;
  if (cmd.nblks > 1) {
    status = MMCStopCmdSend(ctrl);  // send a stop
    if (status == 0) return 0;  //  fail
  }
  return 1;  // success
}

//
// arg    mmcCtrlInfo      mmc control information
// arg    ptr              address where read data is to be stored 
// arg    block            start block where data is to be read
// arg    nblks            number of blocks to read
//
// return  1 success | 0 fail
//
unsigned int MMCReadCmdSend(mmcCtrlInfo *ctrl, void *ptr, unsigned int block,
                              unsigned int nblks) {
  mmcCardInfo *card = ctrl->card;
  unsigned int status = 0, address;
  mmcCmd cmd;

  // Address is in blks for high cap cards and in actual bytes
  // for standard capacity cards
  if (card->highCap) address = block;
  else address = block * card->blkLen;
  MMCXferSetup(ctrl, 1, ptr, 512, nblks);
  cmd.flags = SD_CMDRSP_READ | SD_CMDRSP_DATA;
  cmd.arg = address;
  cmd.nblks = nblks;
  if (nblks > 1) {
    cmd.flags |= SD_CMDRSP_ABORT;
    cmd.idx = SD_CMD(18);
  } else cmd.idx = SD_CMD(17);
  status = MMC0CmdSend(ctrl, &cmd);
  if (status == 0) return 0;
  status = mmc0_get_xfer_status(ctrl);
  if (status == 0) return 0;
  if (cmd.nblks > 1) {
    status = MMCStopCmdSend(ctrl);  // send a stop
    if (status == 0) return 0;  // fail
  }
  CP15DCacheFlushBuff((unsigned int) ptr, (512 * nblks));  // invalidate data cache
  return 1;  // success
}

//
// callback from DMA Completion Handler - MMC
//
static void callback(unsigned int tccNum) {
  callbackOccured = 1;
  DMADisableTransfer(SOC_DMA0CC_0_REGS, tccNum, DMA_TRIG_MODE_EVENT);
}

//
// DMA config - MMC
//
void mmc0_dma_config(void) {
  dma_clk_cfg();
  DMAInit(SOC_DMA0CC_0_REGS, EVT_QUEUE_NUM);
  // request DMA channel and TCC for MMC tx
  DMARequestChannel(SOC_DMA0CC_0_REGS, DMA_CHANNEL_TYPE_DMA,
                    DMA_CHA_MMC0_TX, DMA_CHA_MMC0_TX, EVT_QUEUE_NUM);
  // register callback function for tx
  cb_Fxn[DMA_CHA_MMC0_TX] = &callback;
  // request DMA channel and TCC for MMC rx 
  DMARequestChannel(SOC_DMA0CC_0_REGS, DMA_CHANNEL_TYPE_DMA,
                    DMA_CHA_MMC0_RX, DMA_CHA_MMC0_RX, EVT_QUEUE_NUM);
  // registering callback function for rx
  cb_Fxn[DMA_CHA_MMC0_RX] = &callback;
}

//
//  DMA channel controller completion IRQ service routine - MMC
//
void DMACompletionISR(void) {
  volatile unsigned int pendingIrqs;
  volatile unsigned int isIPR = 0;
  unsigned int indexl, i=0;

  indexl = 1;
  isIPR = DMAGetIntrStatus(SOC_DMA0CC_0_REGS);
  if(isIPR) {
    while ((i < DMACC_COMPL_HANDLER_RETRY_COUNT)&& (indexl != 0)) {
      indexl = 0;
      pendingIrqs = DMAGetIntrStatus(SOC_DMA0CC_0_REGS);
      while (pendingIrqs) {
        if(pendingIrqs & 1) {
          // if callback function not provided
          // while requesting TCC, its TCC specific bit
          // in IPR register will NOT be cleared.
          // here write to ICR to clear corresponding IPR bits 
          DMAClrIntr(SOC_DMA0CC_0_REGS, indexl);
          if (cb_Fxn[indexl] != NULL) {
            (*cb_Fxn[indexl])(indexl);
          }
        }
        ++indexl;
        pendingIrqs >>= 1;
      }
      i++;
    }
  }
}
//
//  DMA channel controller error IRQ service routine - MMC
//
void DMACCErrorISR(void) {
  volatile unsigned int evtqueNum = 0;  // Event Queue Num
  volatile unsigned int pendingIrqs, isIPRH=0, isIPR=0, i=0, index;

  pendingIrqs = 0x0;
  index = 0x1;
  isIPR  = DMAGetIntrStatus(SOC_DMA0CC_0_REGS);
  isIPRH = DMAIntrStatusHighGet(SOC_DMA0CC_0_REGS);
  if((isIPR | isIPRH ) || (DMAQdmaGetErrIntrStatus(SOC_DMA0CC_0_REGS) != 0)
     || (DMAGetCCErrStatus(SOC_DMA0CC_0_REGS) != 0)) {
    // loop for DMACC_ERR_HANDLER_RETRY_COUNT
    // break when no pending interrupt is found
    while ((i < DMACC_ERR_HANDLER_RETRY_COUNT) && (index != 0)) {
      index = 0;
      if(isIPR) pendingIrqs = DMAGetErrIntrStatus(SOC_DMA0CC_0_REGS);
      else pendingIrqs = DMAErrIntrHighStatusGet(SOC_DMA0CC_0_REGS);
      while (pendingIrqs) {  // Process all pending interrupts
        if(pendingIrqs & 1) {
          // write to EMCR to clear the corresponding EMR bits 
          // clear any SER
          if(isIPR) DMAClrMissEvt(SOC_DMA0CC_0_REGS, index);
          else DMAClrMissEvt(SOC_DMA0CC_0_REGS, index + 32);
        }
        ++index;
        pendingIrqs >>= 1;
      }
      index = 0;
      pendingIrqs = DMAQdmaGetErrIntrStatus(SOC_DMA0CC_0_REGS);
      while (pendingIrqs) {  // process pending interrupts
        if(pendingIrqs & 1) {
          // write to QEMCR to clear corresponding QEMR bits, clear any QSER
          DMAQdmaClrMissEvt(SOC_DMA0CC_0_REGS, index);
        }
        ++index;
        pendingIrqs >>= 1;
      }
      index = 0;
      pendingIrqs = DMAGetCCErrStatus(SOC_DMA0CC_0_REGS);
      if (pendingIrqs != 0) {
        // process all pending CC error interrupts
        // queue threshold error for different event queues
        for (evtqueNum = 0; evtqueNum < SOC_DMA_NUM_EVQUE; evtqueNum++) {
          if((pendingIrqs & (1 << evtqueNum)) != 0) {
            // clear error interrupt
            DMAClrCCErr(SOC_DMA0CC_0_REGS, (1 << evtqueNum));
          }
        }
        // transfer completion code error 
        if ((pendingIrqs & (1 << DMACC_CCERR_TCCERR_SHIFT)) != 0) {
          DMAClrCCErr(SOC_DMA0CC_0_REGS, (0x01u << DMACC_CCERR_TCCERR_SHIFT));
        }
        ++index;
      }
      i++;
    }
  }
}

//
// Mux out MMC0 and init MMC data structures
//
void mmc0_config(void) {
  pinmux(CONTROL_CONF_MMC0_DAT3, 0x30); // MUXMODE 0, SLOW SLEW, RX ACTIVE
  pinmux(CONTROL_CONF_MMC0_DAT2, 0x30);
  pinmux(CONTROL_CONF_MMC0_DAT1, 0x30);
  pinmux(CONTROL_CONF_MMC0_DAT0, 0x30);
  pinmux(CONTROL_CONF_MMC0_CLK, 0x30);
  pinmux(CONTROL_CONF_MMC0_CMD, 0x30);
  pinmux(CONTROL_CONF_SPI0_CS1, 0x35); // MUXMODE 5, SLOW SLEW, RX ACTIVE

  ctrlInfo.memBase = SOC_MMC_0_REGS;
  ctrlInfo.intrMask = (MMC_INTR_CMDCOMP | MMC_INTR_CMDTIMEOUT |
                       MMC_INTR_DATATIMEOUT | MMC_INTR_TRNFCOMP);
  ctrlInfo.busWidth = (SD_BUS_WIDTH_1BIT | SD_BUS_WIDTH_4BIT);
  ctrlInfo.highspeed = 1;
  ctrlInfo.ocr = (SD_OCR_VDD_3P0_3P1 | SD_OCR_VDD_3P1_3P2);
  ctrlInfo.card = &sdCard;
  ctrlInfo.ipClk = MMC_IN_FREQ;
  ctrlInfo.opClk = MMC_INIT_FREQ;
  ctrlInfo.cdPinNum = MMC_CARD_DETECT_PINNUM;
  
  sdCard.ctrl = &ctrlInfo;

  callbackOccured = 0;
  xferCompFlag = 0;
  dataTimeout = 0;
  cmdCompFlag = 0;
  cmdTimeout = 0;
}

//
// mmc0 init stuff
//
void mmc0_start(void) {
  mmc0_dma_config();
  mmc0_config();
  mmc0_init();
  mmc0_irq_enab();
}

//
//  mount file system
//
//  NB  This performs a logical mount only 
//  ie card is physically initialized in f_open()
//
FRESULT FATFsMount(unsigned int driveNum, void *ptr, char *Cmd_Buf) {
  FRESULT res=FR_NOT_READY;
  strcpy(CwdBuf, "/");
  strcpy(Cmd_Buf, "\0");
  res = f_mount(driveNum, &FatFsObj);
  fat_devices[driveNum].dev = ptr;
  fat_devices[driveNum].fs = &FatFsObj;
  fat_devices[driveNum].initDone = 0;
  return res;
}

//
//  mmc0 IRQ service routine
//
void mmc0_isr(void) {
  volatile unsigned int status = 0;

  status = MMCIntrStatusGet(ctrlInfo.memBase, 0xffffffff);
  mmc0_clear_status(status);
  if (status & MMC_STAT_CMDCOMP) cmdCompFlag = 1;  // command complete irq
  if (status & MMC_STAT_ERR) {                     // error irq
    errFlag = status & 0xFFFF0000;
    if (status & MMC_STAT_CMDTIMEOUT) cmdTimeout = 1;
    if (status & MMC_STAT_DATATIMEOUT) dataTimeout = 1;
  }
  if (status & MMC_STAT_TRNFCOMP) xferCompFlag = 1;
  if (status & MMC_STAT_CARDINS) {                 // card inserted irq
    FATFsMount(0, &sdCard, &CmdBuf[0]);
    ConsolePrintf("\n\r");  // print current working dir
    CmdLineProcess(&CmdBuf[0]);
  }
  if (status & MMC_STAT_CREM) {                    // card removed irq
    CwdBuf[0] = 0x0;        // wipe current working directory
    callbackOccured = 0;
    xferCompFlag = 0;
    dataTimeout = 0;
    cmdCompFlag = 0;
    cmdTimeout = 0;
    mmc0_init();
    mmc0_irq_enab();
    ConsolePrintf("\n\rPlease insert card... ");
  }
}

// eof
