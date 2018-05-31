//
//  Multi Media Card - Hardware Interface
//  
//  h/w driver routines originally written by TIdude
//
#include "soc_AM335x.h"
#include "hw_types.h"
#include "mmc_hwif.h"
#include "hw_mmc.h"
#include "hw_control_AM335x.h"
#include "hw_cm_per.h"
#include "hw_cm_dpll.h"
#include "bbb_main.h"

//
// Soft reset MMC/SD controller
//
// param    baseAddr      Base Address of MMC controller
//
// return   0   on reset success
//          -1   on reset fail
//
int MMCSoftReset(unsigned int baseAddr) {
  volatile unsigned int timeout = 0xFFFF;

  HWREG(baseAddr + MMC_SYSCONFIG) |= MMC_SYSCONFIG_SOFTRESET;
  do {
    if ((HWREG(baseAddr + MMC_SYSSTATUS) & MMC_SYSSTATUS_RESETDONE) ==
      MMC_SYSSTATUS_RESETDONE) break;
  } while(timeout--);
  if (0 == timeout) return -1;
  else return 0;
}

//
// Soft reset MMC/SD controller lines
//
// param    baseAddr      Base Address of MMC controller
// param    flag          reset flags indicating the lines to be reset.
//
// flag can take the following values
// MMC_DATALINE_RESET
// MMC_CMDLINE_RESET
// MMC_ALL_RESET
//
// return   0   on reset success
//          -1   on reset fail
//
int MMCLinesReset(unsigned int baseAddr, unsigned int flag) {
  volatile unsigned int timeout = 0xFFFF;

  HWREG(baseAddr + MMC_SYSCTL) |= flag;
  do {
    if ((HWREG(baseAddr + MMC_SYSCTL) & flag) == flag) break;
  } while(timeout--);
  if (0 == timeout) return -1;
  else return 0;
}

//
// Configure MMC/SD controller standby, idle and wakeup modes
//
// param    baseAddr      Base Address of MMC controller
// param    config        The standby, idle and wakeup modes
//
// flag can take the values (or a combination of the following)
//     MMC_STANDBY_xxx - Standby mode configuration
//     MMC_CLOCK_xxx - Clock mode configuration
//     MMC_SMARTIDLE_xxx - Smart IDLE mode configuration
//     MMC_WAKEUP_xxx - Wake up configuration
//     MMC_AUTOIDLE_xxx - Auto IDLE mode configuration
//
// return  None.
//
void MMCSystemConfig(unsigned int baseAddr, unsigned int config) {

  HWREG(baseAddr + MMC_SYSCONFIG) &= ~(MMC_SYSCONFIG_STANDBYMODE |
   MMC_SYSCONFIG_CLOCKACTIVITY | MMC_SYSCONFIG_SIDLEMODE |
   MMC_SYSCONFIG_ENAWAKEUP | MMC_SYSCONFIG_AUTOIDLE);
  HWREG(baseAddr + MMC_SYSCONFIG) |= config;
}

//
// Configure MMC/SD bus width
//
// param    baseAddr      Base Address of MMC controller
// param    width         SD/MMC bus width
//
// width can take the values
//     MMC_BUS_WIDTH_8BIT
//     MMC_BUS_WIDTH_4BIT
//     MMC_BUS_WIDTH_1BIT
//
// return  None.
//
//
void MMCSetBusWidth(unsigned int baseAddr, unsigned int width) {

  switch (width) {
    case MMC_BUS_WIDTH_8BIT: 
      HWREG(baseAddr + MMC_CON) |= MMC_CON_DW8;
    break;
    case MMC_BUS_WIDTH_4BIT:
      HWREG(baseAddr + MMC_CON) &= ~MMC_CON_DW8;
      HWREG(baseAddr + MMC_HCTL) |= (MMC_HCTL_DTW_4_BITMODE << MMC_HCTL_DTW_SHIFT);
    break;
    case MMC_BUS_WIDTH_1BIT:
      HWREG(baseAddr + MMC_CON) &= ~MMC_CON_DW8;
      HWREG(baseAddr + MMC_HCTL) &= ~MMC_HCTL_DTW;
      HWREG(baseAddr + MMC_HCTL) |= (MMC_HCTL_DTW_1_BITMODE << MMC_HCTL_DTW_SHIFT);
    break;
  }
}

//
// Configure MMC/SD bus voltage
//
// param    baseAddr      Base Address of MMC controller
// param    volt          SD/MMC bus voltage
//
// volt can take the values
//     MMC_BUS_VOLT_1P8
//     MMC_BUS_VOLT_3P0
//     MMC_BUS_VOLT_3P3
//
// return  None.
//
void MMCBusVoltSet(unsigned int baseAddr, unsigned int volt) {

  HWREG(baseAddr + MMC_HCTL) &= ~MMC_HCTL_SDVS;
  HWREG(baseAddr + MMC_HCTL) |= volt;
}

//
// Turn MMC/SD bus power on / off
//
// param    baseAddr      Base Address of MMC controller
// param    pwr           power on / off setting
//
// pwr can take the values
//     MMC_BUS_POWER_ON
//     MMC_BUS_POWER_OFF
//
// return  0 if the operation succeeded
//         -1 if the operation failed
//
int MMCBusPower(unsigned int baseAddr, unsigned int pwr) {
  volatile unsigned int timeout = 0xFFFFF;

  HWREG(baseAddr + MMC_HCTL) = (HWREG(baseAddr + MMC_HCTL) & ~MMC_HCTL_SDBP) | pwr;
  if (pwr == MMC_BUS_POWER_ON) {
    do {
      if ((HWREG(baseAddr + MMC_HCTL) & MMC_HCTL_SDBP) != 0) break;
    } while(timeout--);
  }
  if (timeout == 0) return -1;
  else return 0;
}

//
// Turn Internal clocks on / off
//
// param    baseAddr      Base Address of MMC controller
// param    pwr           clock on / off setting
//
// pwr can take the values
//     MMC_INTCLOCK_ON
//     MMC_INTCLOCK_OFF
//
// return  0 if the operation succeeded
//         -1 if the operation failed
//
int MMCIntClock(unsigned int baseAddr, unsigned int pwr) {

  HWREG(baseAddr + MMC_SYSCTL) = (HWREG(baseAddr + MMC_SYSCTL) & ~MMC_SYSCTL_ICE) | pwr;
  if (pwr == MMC_INTCLOCK_ON) {
    if(MMCIsIntClockStable(baseAddr, 0xFFFF) == 0) return -1;
  }
  return 0;
}

//
// Get internal clock stable status
//
// param    baseAddr      Base Address of MMC controller
// param    retry         retry times to poll for stable
//
//  NB:  if retry is zero the status is not polled. If it is non-zero status
//       is polled for retry times
//
// return  1 if the clock is stable
//          0 if the clock is not stable
//
unsigned int MMCIsIntClockStable(unsigned int baseAddr, unsigned int retry) {
  volatile unsigned int status = 0;

  do {
    status = (HWREG(baseAddr + MMC_SYSCTL) & MMC_SYSCTL_ICS) >> MMC_SYSCTL_ICS_SHIFT;
    if ((status == 1) || (retry == 0)) break;
  } while (retry--);
  return status;
}

//
// Set supported voltage list
//
// param    baseAddr      Base Address of MMC controller
// param    volt          Supported bus voltage
//
// volt can take the values (or a combination of)
//     MMC_SUPPORT_VOLT_1P8
//     MMC_SUPPORT_VOLT_3P0
//     MMC_SUPPORT_VOLT_3P3
//
// return  None.
//
void MMCSupportedVoltSet(unsigned int baseAddr, unsigned int volt) {

  HWREG(baseAddr + MMC_CAPA) &= ~(MMC_CAPA_VS18 | MMC_CAPA_VS30 | MMC_CAPA_VS33);
  HWREG(baseAddr + MMC_CAPA) |= volt;
}

//
// Check if controller supports high speed
//
// param    baseAddr      Base Address of MMC controller
//
// return   0 if high speed is not supported
//          1 if high speed is supported
//
unsigned int MMCIsHSupported(unsigned int baseAddr) {

  return (HWREG(baseAddr + MMC_CAPA) & MMC_CAPA_HSS) >> MMC_CAPA_HSS_SHIFT;
}

//
// Set data timeout value
//
// param    baseAddr      Base Address of MMC controller
// param    timeout       data time out value
//
// Timeout value is exponential of 2
//
//   NB: Please use MMC_DATA_TIMEOUT(n) for setting this value
//        13 <= n <= 27
//
// return  None
//
void MMCDataTimeoutSet(unsigned int baseAddr, unsigned int timeout) {

  HWREG(baseAddr + MMC_SYSCTL) &= ~(MMC_SYSCTL_DTO);
  HWREG(baseAddr + MMC_SYSCTL) |= timeout;
}    

//
// Set output bus frequency
//
// param    baseAddr      Base Address of MMC controller
// param    freq_in       The input/ref frequency to the controller
// param    freq_out      The required output frequency on the bus
// param    bypass        If the reference clock is to be bypassed
//
// return   0  on clock enable success
//          -1  on clock enable fail
//
// NB: If clock is set properly, clocks are enabled to card on return
//
int MMCSetBusFreq(unsigned int baseAddr, unsigned int freq_in,
                   unsigned int freq_out, unsigned int bypass) {
  volatile unsigned int clkd = 0;
  volatile unsigned int regVal = 0;

  // First enable the internal clocks 
  if (MMCIntClock(baseAddr, MMC_INTCLOCK_ON) == -1) return -1;
  if (bypass == 0) {
    // Calculate and program the divisor 
    clkd = freq_in / freq_out;
    clkd = (clkd < 2) ? 2 : clkd;
    clkd = (clkd > 1023) ? 1023 : clkd;
    // Do not cross the required freq 
    while((freq_in/clkd) > freq_out) {
      if (clkd == 1023)	return -1;  // failed to set clock freq 
      clkd++;
    }
    regVal = HWREG(baseAddr + MMC_SYSCTL) & ~MMC_SYSCTL_CLKD;
    HWREG(baseAddr + MMC_SYSCTL) = regVal | (clkd << MMC_SYSCTL_CLKD_SHIFT);
    // Wait for interface clock stabilization 
    if(MMCIsIntClockStable(baseAddr, 0xFFFF) == 0) return -1;
    // Enable clock to the card 
    HWREG(baseAddr + MMC_SYSCTL) |= MMC_SYSCTL_CEN;
  }
  return 0;
}    

//
// send INIT stream to card
//
// param    baseAddr    Base Address of MMC controller
//
// return   0  If INIT command could be initiated
//          -1  If INIT command could not be completed/initiated
//
int MMCInitStreamSend(unsigned int baseAddr) {
  unsigned int status;

  // Enable command completion status to be set 
  MMCIntrStatusEnable(baseAddr, MMC_SIGEN_CMDCOMP);
  // Initiate INIT command 
  HWREG(baseAddr + MMC_CON) |= MMC_CON_INIT;
  HWREG(baseAddr + MMC_CMD) = 0x00;
  status = MMCIsCmdComplete(baseAddr, 0xFFFF);
  HWREG(baseAddr + MMC_CON) &= ~MMC_CON_INIT;
  // Clear all status 
  mmc0_clear_status(0xFFFFFFFF);
  return status;
}

//
// Enables the controller events to set flags in status register
//
// param    baseAddr    Base Address of MMC controller
// param    flag        Specific event required;
//
// flag can take the following (or combination of) values
// MMC_INTR_xxx
//
//   NB: This function only enables the reflection of events in status register
//       To enable events to generate a h/w interrupt request see MMCIntrEnable()
//
// return   none
//
void MMCIntrStatusEnable(unsigned int baseAddr, unsigned int flag) {

  HWREG(baseAddr + MMC_IE) |= flag;
}

//
// Disables controller events to set flags in status register
//
// param    baseAddr    Base Address of MMC controller
// param    flag        Specific event required;
//
// flag can take the following (or combination of) values
// MMC_INTR_xxx
//
//   NB: This function only enables reflection of events in status register.
//       To disable events to generate a h/w interrupt request see MMCIntrEnable()
//
// return   none
//
void MMCIntrStatusDisable(unsigned int baseAddr, unsigned int flag) {

  HWREG(baseAddr + MMC_IE) &= ~flag;
}

//
// Enab controller events to generate a h/w interrupt request
//
// param    baseAddr    Base Address of MMC controller
// param    flag        Specific event required;
//
// flag can take the following (or combination of) values
// MMC_SIGEN_xxx
//
// return   none
//
void MMCIntrEnable(unsigned int baseAddr, unsigned int flag) {

  HWREG(baseAddr + MMC_ISE) |= flag;
  MMCIntrStatusEnable(baseAddr, flag);
}

//
// get status bits from controller 
//
// param    baseAddr    Base Address of MMC controller
// param    flag        Specific status required;
//
// flag can take the following (or combination of) values
// MMC_STAT_xxx
//
// return   status flags
//
unsigned int MMCIntrStatusGet(unsigned int baseAddr, unsigned int flag) {

  return HWREG(baseAddr + MMC_STAT) & flag;
}

//
//  Checks if command is complete
//
// param     baseAddr    Base Address of MMC controller
// param     retry       retry times to poll for completion
//
// return   1 if command is complete
//          0 if command is not complete
//
unsigned int MMCIsCmdComplete(unsigned int baseAddr, unsigned int retry) {
  volatile unsigned int status = 0;
  do {
    status = (HWREG(baseAddr + MMC_STAT) & MMC_STAT_CC) >> MMC_STAT_CC_SHIFT;
      if (( 1 == status) || (0  == retry)) break;
  } while (retry--);
  return status;
}

//
//  Checks if transfer is complete
//
// param     baseAddr    Base Address of MMC controller
// param     retry       retry times to poll for completion
//
// return   1 if transfer is complete
//           0 if transfer is not complete
//
unsigned int MMCIsXferComplete(unsigned int baseAddr, unsigned int retry) {
  volatile unsigned int status = 0;
  do {
    status = (HWREG(baseAddr + MMC_STAT) & MMC_STAT_TC) >> MMC_STAT_TC_SHIFT;
    if ((1 == status) || (0 == retry)) break;
  } while (retry--);
  return status;
}

//
//  Set the block length/size for data transfer
//
// param     baseAddr    Base Address of MMC controller
// param     blklen      Command to be passed to the controller/card
// 
//   NB: blklen should be within the limits specified by the controller/card
//
// return   none
//
void MMCBlkLenSet(unsigned int baseAddr, unsigned int blklen) {

  HWREG(baseAddr + MMC_BLK) &= ~MMC_BLK_BLEN;
  HWREG(baseAddr + MMC_BLK) |= blklen;
}

//
//  Send data to card from conntroller
//
// param     baseAddr    Base Address of MMC controller
// param     data        pointer to buffer which is to be filled with the data
// param     len         length of the data
//
//   NB: this function reads the data in chunks of 32 bits (4-byte words).
//       thus len should be multiple of 4-byte words
//
// return   none
//
void MMCDataGet(unsigned int baseAddr, unsigned char *data, unsigned int len) {
  unsigned int i;

  for (i = 0; i < len/4; i++) ((unsigned int*)data)[i] = HWREG(baseAddr + MMC_DATA);
}

//
//  Check if card is inserted and detected
//
// param     baseAddr    Base Address of MMC controller
//
// return   0  if card is not inserted and detected
//          1  if card is inserted and detected
//
// NB: that this functional may not be available for all instances of the
// controler. This function, is only useful of the controller has a dedicated
// card detect pin. If not, the card detection mechanism is application
// implementation specific
//
unsigned int MMCCardInserted() {

  return (HWREG(SOC_MMC_0_REGS + MMC_PSTATE) & MMC_PSTATE_CINS) >> MMC_PSTATE_CINS_SHIFT;
}

//
//  Check if card is write protected
//
// param     baseAddr    Base Address of MMC
//
// return   0  if card is not write protected
//          1  if card is write protected
// NB: that this functional may not be available for all instances of the
// controler. This function, is only useful of the controller has a dedicated
// write protect detect pin. If not, the write protect detection mechanism is
// application implementation specific
//
unsigned int MMCIsCardWriteProtected(unsigned int baseAddr) {

  return (HWREG(baseAddr + MMC_PSTATE) & MMC_PSTATE_WP) >> MMC_PSTATE_WP_SHIFT;
}

//
// This API can be used to save register context of MMC
//
// param    mmcBase       Base address of MMC instance
// param    contextPtr    Pointer to structure where MMC register
//                        context need to be saved
//
// return  None
//
void MMCContextSave(unsigned int mmcBase, MMCCONTEXT *contextPtr) {

    contextPtr->capa = HWREG(mmcBase + MMC_CAPA);
    contextPtr->systemConfig = HWREG(mmcBase + MMC_SYSCONFIG);
    contextPtr->ctrlInfo = HWREG(mmcBase + MMC_CON);
    contextPtr->sysCtl = HWREG(mmcBase + MMC_SYSCTL);
    contextPtr->pState = HWREG(mmcBase + MMC_PSTATE);
    contextPtr->hctl = HWREG(mmcBase + MMC_HCTL);
}

//
// This API can be used to restore register context of MMC
//
// param    mmcBase       Base address of MMC instance
// param    contextPtr    Pointer to the structure where MMC register
//                        context need to be saved
//
// return  None
//
void MMCContextRestore(unsigned int mmcBase, MMCCONTEXT *contextPtr) {

  HWREG(mmcBase + MMC_SYSCONFIG) = contextPtr->systemConfig;
  HWREG(mmcBase + MMC_SYSCTL) = contextPtr->sysCtl;
  HWREG(mmcBase + MMC_CAPA) = contextPtr->capa;
  HWREG(mmcBase + MMC_CON) = contextPtr->ctrlInfo;
  HWREG(mmcBase + MMC_HCTL) = contextPtr->hctl;
  HWREG(mmcBase + MMC_PSTATE) = contextPtr->pState;
}

void MMCModuleClkCfg(void) {  // works fast without this

    HWREG(SOC_PRCM_REGS + CM_PER_MMC0_CLKCTRL) |= 
                             CM_PER_MMC0_CLKCTRL_MODULEMODE_ENABLE;
    while((HWREG(SOC_PRCM_REGS + CM_PER_MMC0_CLKCTRL) & 
      CM_PER_MMC0_CLKCTRL_MODULEMODE) != CM_PER_MMC0_CLKCTRL_MODULEMODE_ENABLE);
}

// eof
