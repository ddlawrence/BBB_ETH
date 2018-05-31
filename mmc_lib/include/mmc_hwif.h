//
//  Multi Media Card Hardware Interface  -  prototypes
//  
#include "hw_mmc.h"
#include "hw_types.h"

// Macros that can be passed to configure Standby mode
#define MMC_STANDBY_FORCE      (MMC_SYSCONFIG_STANDBYMODE_FORCE  << MMC_SYSCONFIG_STANDBYMODE_SHIFT)
#define MMC_STANDBY_NONE       (MMC_SYSCONFIG_STANDBYMODE_NOIDLE << MMC_SYSCONFIG_STANDBYMODE_SHIFT)
#define MMC_STANDBY_SMART      (MMC_SYSCONFIG_STANDBYMODE_SMART  << MMC_SYSCONFIG_STANDBYMODE_SHIFT)
#define MMC_STANDBY_SMARTWAKE  (MMC_SYSCONFIG_STANDBYMODE_SMARTWAKE << MMC_SYSCONFIG_STANDBYMODE_SHIFT)

// Macros that can be passed to configure clock activity during wake up period
#define MMC_CLOCK_OFF          (MMC_SYSCONFIG_CLOCKACTIVITY_NONE << MMC_SYSCONFIG_CLOCKACTIVITY_SHIFT)
#define MMC_FCLK_OFF           (MMC_SYSCONFIG_CLOCKACTIVITY_OCP << MMC_SYSCONFIG_CLOCKACTIVITY_SHIFT)
#define MMC_ICLK_OFF           (MMC_SYSCONFIG_CLOCKACTIVITY_FUNC << MMC_SYSCONFIG_CLOCKACTIVITY_SHIFT)
#define MMC_CLOCK_ON           (MMC_SYSCONFIG_CLOCKACTIVITY_BOTH << MMC_SYSCONFIG_CLOCKACTIVITY_SHIFT)

// Macros that can be passed to configure idle modes
#define MMC_SMARTIDLE_FORCE     (MMC_SYSCONFIG_SIDLEMODE_FORCE << MMC_SYSCONFIG_SIDLEMODE_SHIFT)
#define MMC_SMARTIDLE_NONE      (MMC_SYSCONFIG_SIDLEMODE_NOIDLE << MMC_SYSCONFIG_SIDLEMODE_SHIFT)
#define MMC_SMARTIDLE_SMART     (MMC_SYSCONFIG_SIDLEMODE_SMART << MMC_SYSCONFIG_SIDLEMODE_SHIFT)
#define MMC_SMARTIDLE_SMARTWAKE (MMC_SYSCONFIG_SIDLEMODE_SMARTWAKE << MMC_SYSCONFIG_SIDLEMODE_SHIFT)

// Macros that can be passed to configure wakeup modes
#define MMC_WAKEUP_ENABLE       (MMC_SYSCONFIG_ENAWAKEUP_ENABLED << MMC_SYSCONFIG_ENAWAKEUP_SHIFT)
#define MMC_WAKEUP_DISABLE      (MMC_SYSCONFIG_ENAWAKEUP_DISABLED << MMC_SYSCONFIG_ENAWAKEUP_SHIFT)

// Macros that can be passed to configure auto idle modes
#define MMC_AUTOIDLE_ENABLE     (MMC_SYSCONFIG_AUTOIDLE_ON << MMC_SYSCONFIG_AUTOIDLE_SHIFT)
#define MMC_AUTOIDLE_DISABLE    (MMC_SYSCONFIG_AUTOIDLE_OFF << MMC_SYSCONFIG_AUTOIDLE_SHIFT)

// Macros that can be used for SD controller DMA request configuration
#define MMC_DMAREQ_EDGETRIG    (MMC_CON_SDMA_LNE_EARLYDEASSERT << MMC_CON_DMA_LNE_SHIFT)
#define MMC_DMAREQ_LVLTRIG     (MMC_CON_SDMA_LNE_LATEDEASSERT << MMC_CON_DMA_LNE_SHIFT)

// Macros that can be used for SD controller dual rate configuration
#define MMC_DUALRATE_ENABLE    (MMC_CON_DDR_DDRMODE <<  MMC_CON_DDR_SHIFT)
#define MMC_DUALRATE_DISABLE   (MMC_CON_DDR_NORMALMODE << MMC_CON_DDR_SHIFT)

// Macros that can be used for selecting the bus/data width
#define MMC_BUS_WIDTH_8BIT    (0x8)
#define MMC_BUS_WIDTH_4BIT    (0x4)
#define MMC_BUS_WIDTH_1BIT    (0x1)

// Macros that can be used for starting/stopping a init stream
#define MMC_INIT_START         (MMC_CON_INIT_INITSTREAM << MMC_CON_INIT_SHIFT)
#define MMC_INIT_STOP          (MMC_CON_INIT_NOINIT << MMC_CON_INIT_SHIFT)

// Macros that can be used for setting drain type
#define MMC_OPENDRAIN          (MMC_CON_OD_OPENDRAIN << MMC_CON_OD_SHIFT)

#define MMC_NO_RESPONSE            (MMC_CMD_RSP_TYPE_NORSP << MMC_CMD_RSP_TYPE_SHIFT)
#define MMC_136BITS_RESPONSE       (MMC_CMD_RSP_TYPE_136BITS << MMC_CMD_RSP_TYPE_SHIFT)
#define MMC_48BITS_RESPONSE        (MMC_CMD_RSP_TYPE_48BITS << MMC_CMD_RSP_TYPE_SHIFT)
#define MMC_48BITS_BUSY_RESPONSE   (MMC_CMD_RSP_TYPE_48BITS_BUSY << MMC_CMD_RSP_TYPE_SHIFT)

#define MMC_CMD_TYPE_NORMAL        (MMC_CMD_CMD_TYPE_NORMAL << MMC_CMD_CMD_TYPE_SHIFT)
#define MMC_CMD_TYPE_SUSPEND       (MMC_CMD_CMD_TYPE_SUSPEND << MMC_CMD_CMD_TYPE_SHIFT)
#define MMC_CMD_TYPE_FUNCSEL       (MMC_CMD_CMD_TYPE_FUNC_SELECT << MMC_CMD_CMD_TYPE_SHIFT)
#define MMC_CMD_TYPE_ABORT         (MMC_CMD_CMD_TYPE_ABORT << MMC_CMD_CMD_TYPE_SHIFT)

#define MMC_CMD_DIR_READ           (MMC_CMD_DDIR_READ <<  MMC_CMD_DDIR_SHIFT)
#define MMC_CMD_DIR_WRITE          (MMC_CMD_DDIR_WRITE <<  MMC_CMD_DDIR_SHIFT)
#define MMC_CMD_DIR_DONTCARE       (MMC_CMD_DDIR_WRITE <<  MMC_CMD_DDIR_SHIFT)

// Macros that can be used for checking the present state of the host controller
#define MMC_CARD_WRITEPROT        (MMC_PSTATE_WP)
#define MMC_CARD_INSERTED         (MMC_PSTATE_CINS)        
#define MMC_CARD_STABLE           (MMC_PSTATE_CSS)        
#define MMC_BUFFER_READABLE       (MMC_PSTATE_BRE)        
#define MMC_BUFFER_WRITEABLE      (MMC_PSTATE_BWE)        
#define MMC_READ_INPROGRESS       (MMC_PSTATE_RTA)        
#define MMC_WRITE_INPROGRESS      (MMC_PSTATE_WTA)        

// Macros that can be used for configuring the power and transfer parameters
#define MMC_BUS_VOLT_1P8          (MMC_HCTL_SDVS_1V8 << MMC_HCTL_SDVS_SHIFT)
#define MMC_BUS_VOLT_3P0          (MMC_HCTL_SDVS_3V0 << MMC_HCTL_SDVS_SHIFT)
#define MMC_BUS_VOLT_3P3          (MMC_HCTL_SDVS_3V3 << MMC_HCTL_SDVS_SHIFT)
#define MMC_BUS_POWER_ON          (MMC_HCTL_SDBP_PWRON << MMC_HCTL_SDBP_SHIFT)
#define MMC_BUS_POWER_OFF         (MMC_HCTL_SDBP_PWROFF << MMC_HCTL_SDBP_SHIFT)
#define MMC_BUS_HIGHSPEED         (MMC_HCTL_HSPE_HIGHSPEED << MMC_HCTL_HSPE_SHIFT)
#define MMC_BUS_STDSPEED          (MMC_HCTL_HSPE_NORMALSPEED << MMC_HCTL_HSPE_SHIFT)

// Macros that can be used for setting the clock, timeout values
#define MMC_DATA_TIMEOUT(n)        ((((n) - 13) & 0xF) << MMC_SYSCTL_DTO_SHIFT)
#define MMC_CLK_DIVIDER(n)         ((n & 0x3FF) << MMC_SYSCTL_CLKD_SHIFT)
#define MMC_CARDCLOCK_ENABLE       (MMC_SYSCTL_CEN_ENABLE << MMC_SYSCTL_CEN_SHIFT)
#define MMC_CARDCLOCK_DISABLE      (MMC_SYSCTL_CEN_DISABLE << MMC_SYSCTL_CEN_SHIFT)
#define MMC_INTCLOCK_ON            (MMC_SYSCTL_ICE_OSCILLATE << MMC_SYSCTL_ICE_SHIFT)
#define MMC_INTCLOCK_OFF           (MMC_SYSCTL_ICE_STOP << MMC_SYSCTL_ICE_SHIFT)

// Macros that can be used for resetting controller lines
#define MMC_DATALINE_RESET         (MMC_SYSCTL_SRD)
#define MMC_CMDLINE_RESET          (MMC_SYSCTL_SRC)
#define MMC_ALL_RESET              (MMC_SYSCTL_SRA)

// Macros that can be used for interrupt enable/disable and status get operations
#define MMC_INTR_BADACCESS         (MMC_IE_BADA_ENABLE)
#define MMC_INTR_CARDERROR         (MMC_IE_CERR_ENABLE)
#define MMC_INTR_ADMAERROR         (MMC_IE_ADMAE_ENABLE)
#define MMC_INTR_ACMD12ERR         (MMC_IE_ACE_ENABLE)
#define MMC_INTR_DATABITERR        (MMC_IE_DEB_ENABLE)
#define MMC_INTR_DATACRCERR        (MMC_IE_DCRC_ENABLE)
#define MMC_INTR_DATATIMEOUT       (MMC_IE_DTO_ENABLE)
#define MMC_INTR_CMDINDXERR        (MMC_IE_CIE_ENABLE)
#define MMC_INTR_CMDBITERR         (MMC_IE_CEB_ENABLE)
#define MMC_INTR_CMDCRCERR         (MMC_IE_CCRC_ENABLE)
#define MMC_INTR_CMDTIMEOUT        (MMC_IE_CTO_ENABLE)
#define MMC_INTR_CARDINS           (MMC_IE_CINS_ENABLE)                    
#define MMC_INTR_BUFRDRDY          (MMC_IE_BRR_ENABLE)                    
#define MMC_INTR_BUFWRRDY          (MMC_IE_BWR_ENABLE)                    
#define MMC_INTR_TRNFCOMP          (MMC_IE_TC_ENABLE)
#define MMC_INTR_CMDCOMP           (MMC_IE_CC_ENABLE)

#define MMC_STAT_BADACCESS         (MMC_STAT_BADA)
#define MMC_STAT_CARDERROR         (MMC_STAT_CERR)
#define MMC_STAT_ADMAERROR         (MMC_STAT_ADMAE)
#define MMC_STAT_ACMD12ERR         (MMC_STAT_ACE)
#define MMC_STAT_DATABITERR        (MMC_STAT_DEB)
#define MMC_STAT_DATACRCERR        (MMC_STAT_DCRC)
#define MMC_STAT_ERR               (MMC_STAT_ERRI)
#define MMC_STAT_DATATIMEOUT       (MMC_STAT_DTO)
#define MMC_STAT_CMDINDXERR        (MMC_STAT_CIE)
#define MMC_STAT_CMDBITERR         (MMC_STAT_CEB)
#define MMC_STAT_CMDCRCERR         (MMC_STAT_CCRC)
#define MMC_STAT_CMDTIMEOUT        (MMC_STAT_CTO)
#define MMC_STAT_CARDINS           (MMC_STAT_CINS)                    
#define MMC_STAT_BUFRDRDY          (MMC_STAT_BRR)                    
#define MMC_STAT_BUFWRRDY          (MMC_STAT_BWR)                    
#define MMC_STAT_TRNFCOMP          (MMC_STAT_TC)
#define MMC_STAT_CMDCOMP           (MMC_STAT_CC)

#define MMC_SIGEN_BADACCESS        (MMC_ISE_BADA_SIGEN)
#define MMC_SIGEN_CARDERROR        (MMC_ISE_CERR_SIGEN)
#define MMC_SIGEN_ADMAERROR        (MMC_ISE_ADMAE_SIGEN)
#define MMC_SIGEN_ACMD12ERR        (MMC_ISE_ACE_SIGEN)
#define MMC_SIGEN_DATABITERR       (MMC_ISE_DEB_SIGEN)
#define MMC_SIGEN_DATACRCERR       (MMC_ISE_DCRC_SIGEN)
#define MMC_SIGEN_DATATIMEOUT      (MMC_ISE_DTO_SIGEN)
#define MMC_SIGEN_CMDINDXERR       (MMC_ISE_CIE_SIGEN)
#define MMC_SIGEN_CMDBITERR        (MMC_ISE_CEB_SIGEN)
#define MMC_SIGEN_CMDCRCERR        (MMC_ISE_CCRC_SIGEN)
#define MMC_SIGEN_CMDTIMEOUT       (MMC_ISE_CTO_SIGEN)
#define MMC_SIGEN_CARDINS          (MMC_ISE_CINS_SIGEN)                    
#define MMC_SIGEN_BUFRDRDY         (MMC_ISE_BRR_SIGEN)                    
#define MMC_SIGEN_BUFWRRDY         (MMC_ISE_BWR_SIGEN)                    
#define MMC_SIGEN_TRNFCOMP         (MMC_ISE_TC_SIGEN)
#define MMC_SIGEN_CMDCOMP          (MMC_ISE_CC_SIGEN)

// Macros that can be used for checking the capabilites of the controller
#define MMC_SUPPORT_VOLT_1P8       (MMC_CAPA_VS18)
#define MMC_SUPPORT_VOLT_3P0       (MMC_CAPA_VS30)
#define MMC_SUPPORT_VOLT_3P3       (MMC_CAPA_VS33)
#define MMC_SUPPORT_DMA            (MMC_CAPA_DS)
#define MMC_SUPPORT_HIGHSPEED      (MMC_CAPA_HSS)
#define MMC_SUPPORT_BLOCKLEN       (MMC_CAPA_MBL)

// Structure to save the I2C context
typedef struct MMCContext {
    unsigned int capa;
    unsigned int systemConfig;
    unsigned int ctrlInfo;
    unsigned int hctl;
    unsigned int sysCtl;
    unsigned int pState;
}MMCCONTEXT;

// Function prototypes
extern void MMCDataGet(unsigned int baseAddr, unsigned char *data, unsigned int len);
extern unsigned int MMCIsIntClockStable(unsigned int baseAddr, unsigned int retry);
extern unsigned int MMCIsXferComplete(unsigned int baseAddr, unsigned int retry);
extern unsigned int MMCIsCmdComplete(unsigned int baseAddr, unsigned int retry);
extern unsigned int MMCIntrStatusGet(unsigned int baseAddr, unsigned int flag);
extern void MMCDataTimeoutSet(unsigned int baseAddr, unsigned int timeout);
extern int MMCSetBusFreq(unsigned int baseAddr, unsigned int freq_in,
                   unsigned int freq_out, unsigned int bypass);
extern void MMCIntrStatusDisable(unsigned int baseAddr, unsigned int flag);
extern void MMCIntrStatusEnable(unsigned int baseAddr, unsigned int flag);
extern void MMCSupportedVoltSet(unsigned int baseAddr, unsigned int volt);
extern void MMCIntrStatusClear(unsigned int baseAddr, unsigned int flag);
extern void MMCSystemConfig(unsigned int baseAddr, unsigned int config);
// extern void MMCResponseGet(unsigned int baseAddr, unsigned int *rsp);
// ddlx extern void MMCBusWidthSet(unsigned int baseAddr, unsigned int width);
extern void MMCSetBusWidth(unsigned int baseAddr, unsigned int width);
extern void MMCBlkLenSet(unsigned int baseAddr, unsigned int blklen);
extern void MMCBusVoltSet(unsigned int baseAddr, unsigned int volt);
extern void MMCIntrEnable(unsigned int baseAddr, unsigned int flag);
extern unsigned int MMCIsCardWriteProtected(unsigned int baseAddr);
extern int MMCLinesReset(unsigned int baseAddr, unsigned int flag);
extern int MMCBusPower(unsigned int baseAddr, unsigned int pwr);
extern unsigned int MMCIsCardInserted(unsigned int baseAddr);
extern unsigned int MMCIsHSupported(unsigned int baseAddr);
extern int MMCInitStreamSend(unsigned int baseAddr);
extern int MMCSoftReset(unsigned int baseAddr);
extern int MMCBusFreqSet(unsigned int baseAddr, unsigned int freq_in,
                             unsigned int freq_out, unsigned int bypass);
// extern void MMCCommandSend(unsigned int baseAddr, unsigned int cmd,
//                               unsigned int cmdarg, void *data,
//                               unsigned int nblks, unsigned int dmaEn);

extern void MMCContextRestore(unsigned int mmcBase,
		                MMCCONTEXT *contextPtr);

extern void MMCContextSave(unsigned int mmcBase,
		             MMCCONTEXT *contextPtr);
		             
// eof
