//
// MMC0 routines supporting FAT32 filesystem
//
.syntax unified
.data
.global MMC0_REG_BASE
MMC0_REG_BASE            = 0x48060000
MMC_SYSCONFIG            = 0x110  // SD system configuration
MMC_SYSSTATUS            = 0x114  // SD system status
MMC_CON                  = 0x12c  // Configuration (func mode, card init...)
MMC_BLK                  = 0x204  // Block count and block size
MMC_ARG                  = 0x208  // command argument bit 38-8 of command format
MMC_CMD                  = 0x20c  // Command and transfer mode
MMC_HCTL                 = 0x228  // host control (power ,wake-up, transfer)
MMC_CAPA                 = 0x240  // capabilities of host controller
MMC_SYSCTL               = 0x22c  // SD System Control (reset, clocks & timeout)
MMC_STAT                 = 0x230  // SD Interrupt Status bits
MMC_IE                   = 0x234  // SD Interrupt Enable register
MMC_ISE                  = 0x238  // SD Interrupt Set Enable register

MMC_SYSCFG_SOFTRESET                   = 0x2
MMC_SYSSTATUS_RESETDONE                = 0x1
MMC_CAPA_VS_MASK                       = 0x7 << 24  // voltage mask
MMC_CAPA_VS18                          = 0x1 << 26  // 1.8 volt
MMC_CAPA_VS30                          = 0x1 << 25  // 3.0 volt
MMC_SYSCFG_AUTOIDLE                    = 0x1 << 0   // Internal clock gating strategy
MMC_SYSCFG_AUTOIDLE_EN                 = 0x1 << 0   // Automatic clock gating strategy
MMC_SYSCFG_ENAWAKEUP                   = 0x1 << 2   // Wake-up feature control
MMC_SYSCFG_ENAWAKEUP_EN                = 0x1 << 2   // Enable wake-up capability
MMC_SYSCFG_SIDLEMODE                   = 0x3 << 3   // Power management
MMC_SYSCFG_SIDLEMODE_IDLE              = 0x2 << 3   // Ack IDLE request switch to wake-up mode
MMC_SYSCFG_CLOCKACTIVITY               = 0x3 << 8   // Clock activity during wake-up
MMC_SYSCFG_CLOCKACTIVITY_OFF           = 0x0 << 8   // i/f and func clock can be switched off
MMC_SYSCFG_STANDBYMODE                 = 0x3 << 12  // Configuration for standby
MMC_SYSCFG_STANDBYMODE_WAKEUP_INTERNAL = 0x2 << 12  // wake-up mode based on internal knowledge
MMC_HCTL_IWE       = 0x1 << 24    // wake-up event on SD interrupt
MMC_HCTL_IWE_EN    = 0x1 << 24    // Enable wake-up on SD interrupt
MMC_CON_DW8        = 0x1 << 5     // 8-bit mode MMC select , For SD clear this bit
MMC_CON_DW8_1_4BIT   = 0x0 << 5   // 1 or 4 bits data width configuration(also set SD_HCTL)
MMC_HCTL_DTW       = 0x1 << 1     // Data xfer width.(must be set after a successful ACMD6)
MMC_HCTL_DTW_1BIT  = 0x0 << 1     // 1 bit transfer width
MMC_HCTL_DTW_4BIT  = 0x1          // 4 bit transfer width
MMC_HCTL_SDVS      = 0x7 << 9     // SD bus voltage select
MMC_HCTL_SDVS_VS30 = 0x6 << 9     // 3.0 V
MMC_HCTL_SDBP      = 0x1 << 8     // SD bus power
MMC_HCTL_SDBP_ON   = 0x1 << 8     // SD Power on (start card detect?)
MMC_SYSCTL_ICE     = 0x1 << 0     // Internal clock enable register 
MMC_SYSCTL_ICE_EN  = 0x1 << 0     // Enable internal clock
MMC_SYSCTL_CLKD    = 0x3ff << 6   // 10 bits clock frequency select
MMC_SYSCTL_CEN     = 0x1 << 2     // Card lock enable provide clock to the card
MMC_SYSCTL_CEN_EN  = 0x1 << 2     // Internal clock is stable  
MMC_SYSCTL_ICS        = 0x1 << 1  // Internal clock stable register 
MMC_SYSCTL_ICS_STABLE = 0x1 << 1  // Internal clock is stable  
MMC_IE_CC_EN          = 0x1 << 0  // Command complete interrupt enable
MMC_IE_CC_EN_EN       = 0x1 << 0  // Command complete Interrupts are enabled
MMC_IE_CC_EN_CLR      = 0x1 << 0  // Clearing is done by writing a 0x1
MMC_IE_TC_EN          = 0x1 << 1  // Transfer complete interrupt enable
MMC_IE_TC_EN_EN       = 0x1 << 1  // Transfer complete Interrupts are enabled
MMC_IE_TC_EN_CLR      = 0x1 << 1  // Clearing TC is done by writing a 0x1
MMC_IE_ERROR_MASK     = (0xff << 15 | 0x3 << 24 | 0x03 << 28)  // 0x337f8000
MMC_STAT_ERROR_MASK   = (0xff << 15 | 0x3 << 24 | 0x03 << 28)  // Jan Kees original mask
MMC_IE_CTO_EN         = 0x1 << 16 // Command TimeOut signal enab
MMC_IE_DTO_EN         = 0x1 << 20 // Data TimeOut signal enab
MMC_IE_CINS_EN        = 0x1 << 6  // Card Insert signal enab
MMC_IE_CREM_EN        = 0x1 << 7  // Card Remove signal enab
MMC_CON_INIT          = 0x1 << 1  // Send initialization stream (all cards)
MMC_CON_INIT_NOINIT   = 0x0 << 1  // Do nothing
MMC_CON_INIT_INIT     = 0x1 << 1  // Send initialization stream
MMC_STAT_CC           = 0x1 << 0  // Command complete status
MMC_STAT_CC_RAISED    = 0x1 << 0  // Command completed
MMC_CMD_MASK = ~(0x1<<30|0x1<<31|0x1<<18|0x1<<3)  // bits 30, 31 & 18 are reserved
MMC_CMD_RSP_TYPE      = (0x3 << 16)     // Response type
MMC_CMD_RSP_TYPE_48B_BUSY = (0x3 << 16) // Response len 48 bits with busy after response
MMC_BLK_NBLK          = 0xffff0000   // register MMC_BLK field NBLK
MMC_RSP               = 0x210     // Command response register base, 4 words total

SOC_PRCM_REGS         = 0x44E00000
CM_PER_MMC0_CLOCKCTRL = 0x3c
CM_PER_MMC0_CLOCKCTRL_MMODE_ENAB = 0x2

.text

.macro setreg32
    ldr	r4, [r0, r1]          // macro setreg32
    and	r3, r3, r2            // r0  register base address
    bic	r4, r4, r2            // r1  offset from base address
    orr	r3, r4, r3            // r2  mask
    str	r3, [r0, r1]          // r3  data
.endm

.macro mov32 regnum,number         // macro mov32
  movw \regnum,:lower16:\number    // r0 register number
  movt \regnum,:upper16:\number    // r1 number or expression
.endm

// mmc0 module init
//
// @return   0=success or 1=fail
//
.global mmc0_init
mmc0_init:
    r_base .req r0
    push {r4, lr}

    ldr r_base, =SOC_PRCM_REGS
    ldr r1, [r_base, CM_PER_MMC0_CLOCKCTRL]
    mov r2, CM_PER_MMC0_CLOCKCTRL_MMODE_ENAB   // enab OCP & CLKADPI clocks, spruh73n 18.3.2.1
    orr r1, r1, r2                             // not necessary
    str r1, [r_base, CM_PER_MMC0_CLOCKCTRL]
1:  ldr r1, [r_base, CM_PER_MMC0_CLOCKCTRL]
    cmp r1, CM_PER_MMC0_CLOCKCTRL_MMODE_ENAB   // wait for ENAB
    bne 1b

    ldr r_base, =MMC0_REG_BASE
    ldr r1, [r_base, MMC_SYSCONFIG]
    mvn r2, MMC_SYSCFG_SOFTRESET     // SOFTRESET, spruh73l 18.2.2.2, Table 18-15
    and r1, r1, r2
    mov r2, MMC_SYSCFG_SOFTRESET
    orr r1, r1, r2
    str r1, [r_base, MMC_SYSCONFIG]
1:  ldr r1, [r_base, MMC_SYSSTATUS]
    cmp r1, MMC_SYSSTATUS_RESETDONE  // wait for RESETDONE, spruh73l Table 18-16
    bne 1b

    ldr r_base, =MMC0_REG_BASE
    mov r1, MMC_CAPA                 // Set SD default capabilities
    mov r2, MMC_CAPA_VS_MASK
    mov r3, MMC_CAPA_VS18 | MMC_CAPA_VS30
    setreg32
    mov r1, MMC_SYSCONFIG            // wake-up configuration
    ldr r2, =(MMC_SYSCFG_AUTOIDLE | MMC_SYSCFG_ENAWAKEUP | MMC_SYSCFG_SIDLEMODE | MMC_SYSCFG_CLOCKACTIVITY | MMC_SYSCFG_STANDBYMODE)
    ldr r3, =(MMC_SYSCFG_AUTOIDLE_EN | MMC_SYSCFG_ENAWAKEUP_EN | MMC_SYSCFG_SIDLEMODE_IDLE | MMC_SYSCFG_CLOCKACTIVITY_OFF | MMC_SYSCFG_STANDBYMODE_WAKEUP_INTERNAL)
    setreg32
    mov r1, MMC_HCTL                 // Wake-up on sd interrupt SDIO
    ldr r2, =MMC_HCTL_IWE
    ldr r3, =MMC_HCTL_IWE_EN
    setreg32
/*
    mov r1, MMC_CON                  // Configure data and command transfer (1 bit mode)
    ldr r2, =MMC_CON_DW8             // 1GB copy 30min
    ldr r3, =MMC_CON_DW8_1_4BIT
    setreg32
    mov r1, MMC_HCTL
    ldr r2, =MMC_HCTL_DTW
    ldr r3, =MMC_HCTL_DTW_1BIT
    setreg32
*/
    mov r1, MMC_CON                  // Configure data and command transfer (4 bit mode)
    ldr r2, =MMC_CON_DW8             // 1GB copy 30min - NO CHANGE  (only 4 pins mux'd out)
    ldr r3, =MMC_CON_DW8_1_4BIT
    setreg32
    mov r1, MMC_HCTL
    ldr r2, =MMC_HCTL_DTW_4BIT
    ldr r3, =MMC_HCTL_DTW_4BIT
    setreg32
/*
    mov r1, MMC_CON                  // Configure data and command transfer (8 bit mode)
    ldr r2, =MMC_CON_DW8             // pin macros CONTROL_CONF_MMC0_DAT4 to 7 not found
    ldr r3, =MMC_CON_DW8
    setreg32
*/
    mov r1, MMC_HCTL                 // Configure card voltage
    ldr r2, =MMC_HCTL_SDVS
    ldr r3, =MMC_HCTL_SDVS_VS30
    setreg32
    mov r1, MMC_HCTL                 // Power on the host controller
    ldr r2, =MMC_HCTL_SDBP
    ldr r3, =MMC_HCTL_SDBP_ON
    setreg32

1:  ldr r1, [r_base, MMC_HCTL]       // wait for SDBP_POWER_ON set, spruh73l, Tab 18-31
    and r1, r1, MMC_HCTL_SDBP
    cmp r1, MMC_HCTL_SDBP_ON
    bne 1b

    mov r1, MMC_SYSCTL               // Enab internal clock & clock to card
    ldr r2, =MMC_SYSCTL_ICE
    ldr r3, =MMC_SYSCTL_ICE_EN
    setreg32
    mov r1, MMC_SYSCTL               // external clock enable
    ldr r2, =MMC_SYSCTL_CLKD         // TODO Fix, this one is very slow
//  trick:  mod slab2c temporarily func MMCBusFreqSet() in mmc_hwif.c
//          to print out the calculated clock freq, and use it above!
//          altho it is also slow

    ldr r3, =(0x3ff << 6)
    setreg32
    mov r1, MMC_SYSCTL
    ldr r2, =MMC_SYSCTL_CEN
    ldr r3, =MMC_SYSCTL_CEN_EN
    setreg32

1:  ldr r1, [r_base, MMC_SYSCTL]     // wait for internal clk stable, spruh73l, 18.4.1.18 
    and r1, r1, MMC_SYSCTL_ICS
    cmp r1, MMC_SYSCTL_ICS_STABLE
    bne 1b

    mov r1, MMC_IE                   // enab cmd complete interrupt, spruh73l, 18.3.3.2
    ldr r2, =MMC_IE_CC_EN            // Card Detection, Identification, and Selection
    ldr r3, =MMC_IE_CC_EN_EN
    setreg32
    mov r1, MMC_IE                   // enab transfer complete interrupt
    ldr r2, =MMC_IE_TC_EN
    ldr r3, =MMC_IE_TC_EN_EN
    setreg32
    mov r1, MMC_IE                   // enable error interrupts
    ldr r2, =MMC_IE_ERROR_MASK
    ldr r3, =0x0fffffff
// NB skip BADA interrupt it gets raised, unknown reason (maybe mask clobbers 0x337f8000)
    setreg32

    mov r1, MMC_STAT                 // purge error interrupts
    ldr r2, =MMC_STAT_ERROR_MASK
    ldr r3, =0xffffffff
    setreg32

    mov r1, MMC_CON                  // send init signal to host controller
                                     // does not actually send a cmd to card
    ldr r2, =MMC_CON_INIT
    ldr r3, =MMC_CON_INIT_INIT
    setreg32

    mov r1, 0x0                      // SD command 0, type other commands, no response
    str r1, [r_base, MMC_CMD]

1:  ldr r1, [r_base, MMC_STAT]       // wait for command complete, spruh73l 18.4.1.19 
    and r2, r1, MMC_STAT_CC
    cmp r2, MMC_STAT_CC_RAISED
    beq 1f
    tst r1, 0x8000                   // check for ERR interrupt
    beq 1b
    mov r0, 0x1                      // flag error & exit
    b mmc0_init_exit
1:
    mov r1, MMC_STAT                 // clear cc interrupt status
    ldr r2, =MMC_IE_CC_EN
    ldr r3, =MMC_IE_CC_EN_EN
    setreg32
    mov r1, MMC_CON                  // clr INIT bit - end init sequence
    ldr r2, =MMC_CON_INIT
    ldr r3, =MMC_CON_INIT_NOINIT
    setreg32
    mov r0, 0x0
mmc0_init_exit:
    pop {r4, pc}
    .unreq r_base

//
//  Enable set status of command/data complete/timeout card insert/remove interrupts
//
.global mmc0_irq_enab
mmc0_irq_enab:
    r_base .req r0
    push {r4, lr}
    ldr r_base, =MMC0_REG_BASE
    mov r1, MMC_ISE
    ldr r2, =(MMC_IE_CC_EN | MMC_IE_TC_EN | MMC_IE_CTO_EN | MMC_IE_DTO_EN | MMC_IE_CINS_EN | MMC_IE_CREM_EN)
    ldr r3, =(MMC_IE_CC_EN | MMC_IE_TC_EN | MMC_IE_CTO_EN | MMC_IE_DTO_EN | MMC_IE_CINS_EN | MMC_IE_CREM_EN)
    setreg32
    mov r1, MMC_IE
    ldr r3, =(MMC_IE_CC_EN | MMC_IE_TC_EN | MMC_IE_CTO_EN | MMC_IE_DTO_EN | MMC_IE_CINS_EN | MMC_IE_CREM_EN)
    setreg32
    pop {r4, pc}
    .unreq r_base

//
//  send a command to card
//
// @param command      uint32, SD CMD
// @param ARG          uint32, argument 
// @param NBLK         uint32, number of blocks in low 16 bits
//
// @return             nothing returned
//
.global mmc0_send_cmd
mmc0_send_cmd:
    push {r4, r5, r6, lr}
    r_base .req r4

    ldr r_base, =MMC0_REG_BASE
    ldr r3, [r_base, MMC_BLK]
    mov32 r5, MMC_BLK_NBLK
    bic r3, r3, r5              // clear the 2 hi bytes
    orr r2, r3, r2, lsl #0x10   // and shift in NBLK    spruh73n 18.4.1.8

    str r2, [r_base, MMC_BLK]   
    str r1, [r_base, MMC_ARG]   
    str r0, [r_base, MMC_CMD]   

    pop {r4, r5, r6, pc}
    .unreq r_base

//
//  set data timeout
//
// @param ticks        uint32, number of cycles wait 2^(ticks-13)
//                             before timeout error occurs
//                             ticks permissible range 13 to 27
//
// @return             nothing returned
//
.global mmc0_set_dto
mmc0_set_dto:
    push {r4, lr}
    r_base .req r4

    ldr r_base, =MMC0_REG_BASE
    ldr r1, [r_base, MMC_SYSCTL]
    bic r1, r1, #0xf << 0x10    // clear bits [16:19]

    sub r0, r0, #0xd            // h/w dependant    spruh73n 18.4.1.18
    and r0, r0, #0xf
    orr r1, r1, r0, lsl #0x10   // shift in DTO field
    str r1, [r_base, MMC_SYSCTL]

    pop {r4, pc}
    .unreq r_base

//
//  clear interrupt status
//
// @param bit_pattern     uint32, status bit(s)
//                                an interrupt status bit is cleared by writing a 1
//                                not all bits are valid.  see spruh73n 18.4.1.19
//
// @return             nothing returned
//
.global mmc0_clear_status
mmc0_clear_status:
    push {r4, lr}
    r_base .req r4

    ldr r_base, =MMC0_REG_BASE
    str r0, [r_base, MMC_STAT]

    pop {r4, pc}
    .unreq r_base

//
//  get a response from card
//
// @param address      *uint32, store response at this address 
//                             response is 4 words long
//
// @return             nothing returned
//
.global mmc0_get_resp
mmc0_get_resp:
    push {r4, lr}
    r_base .req r4

    ldr r_base, =MMC0_REG_BASE
    ldr r1, [r_base, MMC_RSP]
    str r1, [r0]   

    ldr r_base, =MMC0_REG_BASE
    ldr r1, [r_base, MMC_RSP + 0x4]
    str r1, [r0, 0x4]   

    ldr r_base, =MMC0_REG_BASE
    ldr r1, [r_base, MMC_RSP + 0x8]
    str r1, [r0, 0x8]   

    ldr r_base, =MMC0_REG_BASE
    ldr r1, [r_base, MMC_RSP + 0xc]
    str r1, [r0, 0xc]   

    pop {r4, pc}
    .unreq r_base

// eof
