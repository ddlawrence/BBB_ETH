//
//  FAT32 - MMC interface
//
//  disk I/O module skeleton for FatFs
//  this file was originally from elm-chan
//  it was modded by TIdude for bbb mmc I/O
//  the functions herein are called by the 
//  elm-chan FAT32 filesystem routines in ff.c
//  the functions herein call the mmc_api routines
//
#include "bbb_main.h"
#include "diskio.h"
#include "hw_types.h"
#include "mmc_api.h"
#include "ff.h"

// extern unsigned int year, month, day, hour, min, sec;
extern uint32_t year, month, day, hour, min, sec;

#define DEBUG                 1

typedef struct _fatDevice {
    void *dev;              // Pointer to underlying device/controller
    FATFS *fs;              // File system pointer
	unsigned int initDone;  // state
}fatDevice;

#define DRIVE_NUM_MMC      0
#define DRIVE_NUM_MAX      2

fatDevice fat_devices[DRIVE_NUM_MAX];

//
// Initialize Disk Drive
//
DSTATUS
disk_initialize(BYTE bValue) {               // Physical drive number (0)
	unsigned int status;

    if (DRIVE_NUM_MAX <= bValue) {
        return STA_NODISK;
    }
    if ((DRIVE_NUM_MMC == bValue) && (fat_devices[bValue].initDone != 1)) {
        mmcCardInfo *card = (mmcCardInfo *) fat_devices[bValue].dev;

        status = MMCCardInit(card->ctrl);  // MMC/SD Card init
        if (status == 0) {
            ConsolePrintf("\r\nCard Init Failed \r\n");
            return STA_NOINIT;
        } else {
#if DEBUG
            if (card->cardType == MMC_CARD_SD) {
                ConsolePrintf("\n\rSD Card version %d", card->sd_ver);
                if (card->highCap) {
                    ConsolePrintf(" high capacity");
                }
                if (card->tranSpeed == SD_TRANSPEED_50MBPS) {
                    ConsolePrintf(" high speed");
                }
                ConsolePrintf("\n\n\r");
            } else {
              if (card->cardType == MMC_CARD_MMC) ConsolePrintf("\n\rMMC Card\n\r", -1);
            }
#endif
            if (card->cardType == MMC_CARD_SD) {  // Set bus width
              MMCBusWidthSet(card->ctrl);
            }
            MMCTranSpeedSet(card->ctrl);  // Transfer speed
         }
         fat_devices[bValue].initDone = 1;
    }
    return 0;
}

//
// Returns the current status of a drive                                 
//
DSTATUS disk_status (
    BYTE drv)                   // Physical drive number (0) 
{
	return 0;
}

//
// This function reads sector(s) from the disk drive                     
//
DRESULT disk_read (
    BYTE drv,               // Physical drive number (0) 
    BYTE* buff,             // Pointer to the data buffer to store read data 
    DWORD sector,           // Physical drive number (0) 
    BYTE count)             // Sector count (1..255) 
{
	if (drv == DRIVE_NUM_MMC)
	{
		mmcCardInfo *card = fat_devices[drv].dev;

gpio_on(SOC_GPIO_1_REGS, 0x1<<21);  // flash LED USR1  n!

    	// READ BLOCK 
		if (MMCReadCmdSend(card->ctrl, buff, sector, count) == 1) {

gpio_off(SOC_GPIO_1_REGS, 0x1<<21);  // flash LED USR1  n!

      return RES_OK;
		}
  }
  return RES_ERROR;
}

//
// This function writes sector(s) to the disk drive                     
//
#if _READONLY == 0
DRESULT disk_write (
    BYTE ucDrive,           // Physical drive number (0) 
    const BYTE* buff,      // Pointer to the data to be written 
    DWORD sector,          // Start sector number (LBA) 
    BYTE count)            // Sector count (1..255) 
{
	if (ucDrive == DRIVE_NUM_MMC)	{
		mmcCardInfo *card = fat_devices[ucDrive].dev;

gpio_on(SOC_GPIO_1_REGS, 0x2<<21);  // flash LED USR2  n!

tim_delay(1);  // delay required for large files (1G file copy takes half an hour!)
               //      FR_RW_ERROR 0x8                 - TODO n!
      // WRITE BLOCK 
	  if(MMCWriteCmdSend(card->ctrl,(BYTE*) buff, sector, count) == 1) {

gpio_off(SOC_GPIO_1_REGS, 0x2<<21);  // flash LED USR2  n!

      return RES_OK;
		}
  }
  return RES_ERROR;
}
#endif // _READONLY 

//
// Miscellaneous Functions                                               
//
DRESULT disk_ioctl (
    BYTE drv,               // Physical drive number (0) 
    BYTE ctrl,              // Control code 
    void *buff)             // Buffer to send/receive control data 
{
	return RES_OK;
}

//
// User Provided Timer Function for FatFs module           
//
// This is a real time clock service to be called from     
// FatFs module. Any valid time must be returned even if   
// the system does not support a real time clock.          

DWORD get_fattime (void) {
  return ((((year >> 4) & 0x7) * 10 + (year & 0xF)) << 25)  // year 0 = 1980
       | ((((month >> 4) & 0x1) * 10 + (month & 0xF)) << 21)
       | ((((day >> 4) & 0x3) * 10 + (day & 0xF)) << 16)
       | ((((hour >> 4) & 0x3) * 10 + (hour & 0xF)) << 11)
       | ((((min >> 4) & 0x7) * 10 + (min & 0xF)) << 5)
       | ((((sec >> 4) & 0x7) * 10 + (sec & 0xF)) >> 1);  // 5 bits ie 2 sec resolution
}

// eof
