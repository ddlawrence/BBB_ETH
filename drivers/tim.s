//
// Timer & RTC routines
//
.global rtc_init
.global rtc_isr
.global rtc_irq_count
.global year
.global month
.global day
.global hour
.global min
.global sec

.global tim_init
.global tim_delay

.syntax unified
.data
CM_PER_BASE              = 0x44E00000
CM_RTC_RTC_CLKCTRL       = 0x0   // spruh73l 8.1.12.6.1
CM_RTC_CLKSTCTRL         = 0x4   // spruh73l 8.1.12.6.2
CM_PER_TIMER7_CLKCTRL    = 0x7C  // spruh73l 8.1.12.1.25

SOC_RTC_0_REGS           = 0x44E3E000
RTC_CTRL_REG             = 0x40
RTC_STATUS_REG           = 0x44
RTC_INTERRUPTS_REG       = 0x48
RTC_OSC_REG              = 0x54
KICK0R                   = 0x6C
KICK1R                   = 0x70
SECONDS_REG              = 0x00
MINUTES_REG              = 0x04
HOURS_REG                = 0x08
DAYS_REG                 = 0x0C
MONTHS_REG               = 0x10
YEARS_REG                = 0x14
WEEKS_REG                = 0x18

DMTIMER7                 = 0x4804A000
TCLR                     = 0x38
TCRR                     = 0x3C
TWPS                     = 0x48
TSICR                    = 0x54
TIMER_1MS_COUNT          = 0x5DC0   // calibrated for 24MHz timer input clock

GPIO_CLEARDATAOUT        = 0x190
GPIO_SETDATAOUT          = 0x194

.text
//
// rtc module init
//
// @return   0=success or 1=fail
//
rtc_init:
    r_base .req r0
    ldr r_base, =CM_PER_BASE
    mov r1, 0x2
    str r1, [r_base, CM_RTC_RTC_CLKCTRL]  // MODULEMODE enab spruh73l 8.1.12.6.1
    mov r1, 0x2                           // transition ctrl SW_ WKUP spruh73l 8.1.12.6.2
    str r1, [r_base, CM_RTC_CLKSTCTRL]

    ldr r_base, =SOC_RTC_0_REGS
    ldr r1, =0x83E70B13          // disab write protection
    str r1, [r_base, KICK0R]
    ldr r1, =0x95A4F1E0
    str r1, [r_base, KICK1R]
    ldr r1, =0x48                // select 32k ext clk & enab, spruh73l 20.3.3.2, tab 20.82
    str r1, [r_base, RTC_OSC_REG]
    ldr r1, =0x1                 // RTC enab, functional & running, spruh73l tab 20.77
    str r1, [r_base, RTC_CTRL_REG]

1:  ldr r1, [r_base, RTC_STATUS_REG]
    and r1, r1, 0x1
    cmp	r1, 0x0                  // wait for BUSY bit to clear, spruh73l 20.3.5.15
    bne	1b

    ldr r1, =0x4                 // enab interrupt every second, spruh73l 20.3.5.16 
    str r1, [r_base, RTC_INTERRUPTS_REG] 
    mov r_base, #0x0
    bx lr
    .unreq r_base

//
// rtc interrupt service routine
//
// hacked from Al Selen at github.com/auselen
//
rtc_isr:
    ldr r0, =SOC_RTC_0_REGS   // retrieve time from RTC, spruh73l 20.3.3.5.1
    ldr r1, =sec              // C variable
    ldr r2, [r0, SECONDS_REG]
    str r2, [r1]
    ldr r1, =min              // C variable
    ldr r2, [r0, MINUTES_REG]
    str r2, [r1]
    ldr r1, =hour             // C variable
    ldr r2, [r0, HOURS_REG]
    str r2, [r1]
    ldr r1, =day              // C variable
    ldr r2, [r0, DAYS_REG]
    str r2, [r1]
    ldr r1, =month            // C variable
    ldr r2, [r0, MONTHS_REG]
    str r2, [r1]
    ldr r1, =year             // C variable
    ldr r2, [r0, YEARS_REG]
    str r2, [r1]
    ldr r1, =rtc_irq_count    // C variable   increment irq counter
    ldr r2, [r1]
    add r2, #0x1              // chk this is incrementing correctly see ands below
    str r2, [r1]

/* use this to blink LED via RTC
    ldr r0, =SOC_GPIO_1_REGS  // actuate LED USR1
    mov r1, 0x1<<21
    ands r2, #0x10            // flip-flop on rtc_irq_count   this endian quickfix works
    beq 1f
    str r1, [r0, GPIO_SETDATAOUT]
    bx lr
1:
    str r1, [r0, GPIO_CLEARDATAOUT]
*/
    bx lr
rtc_irq_count:  .word  0x0
year:           .word  0x0
month:          .word  0x0
day:            .word  0x0
hour:           .word  0x0
min:            .word  0x0
sec:            .word  0x0

//
// timer module init    poll mode version
//
// @return   0=success or 1=fail
//
tim_init:
    r_base .req r0
    ldr r_base, =CM_PER_BASE
    mov r1, 0x2
    str r1, [r_base, CM_PER_TIMER7_CLKCTRL]  // MODULEMODE enab spruh73n 8.1.12.1.25
1:  ldr r1, [r_base, CM_PER_TIMER7_CLKCTRL]  // wait for enab spruh73n tab 8-54
    tst r1, 0x2
    beq 1b

    ldr r_base, =DMTIMER7
    mov r1, 0x3              // start timer   auto-reload spruh73l 20.1.5.9 tab 20.19
    str r1, [r_base, TCLR]   // spruh73l 20.1.5
    ldr r1, [r_base, TSICR]
    mov r2, 0x4              // set posted mode bit spruh73l 20.1.3.9
    orr r1, r1, r2
    str r1, [r_base, TSICR]  // spruh73l 20.1.5.16   tab 20.26

    mov r_base, #0x0
    bx lr
    .unreq r_base

//
// timer delay    poll mode version
//
// NB this TWPS method uses loops  
//    later try using TLDR load reg & TTGR trigger reg
//    -OR- don't be so lazy and implement irq version!
// 
// @param msec       uint32, delay in milliseconds
//                   max limit   (msec * TIMER_1MS_COUNT) < 4.3E9
//
tim_delay:
    r_base .req r0
    mov r3, r0                // delay in msec
    ldr r1, =TIMER_1MS_COUNT
    mul r3, r3, r1            // delay in timer ticks

    ldr r_base, =DMTIMER7
1:  ldr r1, [r_base, TWPS]    // write post status   spruh73l 20.1.5.13 
    and r1, r1, 0x2           // TCRR write pending bit   tab 20.23
    cmp	r1, 0x0               // wait for bit to clear
    bne	1b
    mov r1, 0x0               // reset timer counter
    str r1, [r_base, TCRR]    // spruh73l 20.1.5.10 

1:  ldr r1, [r_base, TWPS]    // write post status   spruh73l 20.1.5.13 
    and r1, r1, 0x1           // TCLR write pending bit   tab 20.23
    cmp	r1, 0x0               // wait for bit to clear
    bne	1b
    ldr r1, [r_base, TCLR]
    mov r2, 0x1               // start timer bit   tab 20.19
    orr r1, r1, r2            // enable timer
    str r1, [r_base, TCLR]    // spruh73l 20.1.5.10 

1:  ldr r1, [r_base, TCRR]    // read counter   spruh73l 20.1.5.10 
    cmp	r1, r3                // test delay
    blt	1b

1:  ldr r1, [r_base, TWPS]    // write post status   spruh73l 20.1.5.13 
    and r1, r1, 0x1           // TCLR write pending bit   tab 20.23
    cmp	r1, 0x0               // wait for bit to clear
    bne	1b
    ldr r1, [r_base, TCLR]
    mvn r2, 0x1               // start timer bit   tab 20.19
    and r1, r1, r2            // disable timer
    str r1, [r_base, TCLR]    // spruh73l 20.1.5.10 

    bx lr

// eof
    