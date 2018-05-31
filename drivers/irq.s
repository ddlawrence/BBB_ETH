//
// interrupt routines
//
.syntax unified
.data
SOC_AINTC_REGS          = 0x48200000  // BBB ARM Interrupt Controller base address
INTC_SYSCONFIG          = 0x10
INTC_SYSSTATUS          = 0x14
INTC_SIR_IRQ            = 0x40
INTC_CONTROL            = 0x48
INTC_IRQ_PRIORITY       = 0x60
INTC_THRESHOLD          = 0x68
INTC_SIR_IRQ_ACTIVEIRQ  = 0x7F
INTC_MIR_CLEAR0         = 0x88
INTC_MIR_CLEAR1         = 0xA8
INTC_MIR_CLEAR2         = 0xC8

.text
//
// ARM interrupt controller init
//
.global irq_init
irq_init:
    r_base .req r0
    ldr r_base, =SOC_AINTC_REGS

    mov r1, 0x2
    str r1, [r_base, INTC_SYSCONFIG]   // soft reset AINT controller spruh73l 6.5.1.2
1:  ldr	r1, [r_base, INTC_SYSSTATUS]
    and r1, r1, 0x1
    cmp	r1, 0x1                        // test for reset complete spruh73l 6.5.1.3
    bne	1b

// priority masking threshold disabled  INTC_THRESHOLD def=0xff spruh73n 6.1.1.2.2, 6.5.1.11
// all interrupt lines are priority 0x0 and type IRQ  INTC_ILR_x def=0x0  spruh73n 6.5.1.44

// unmask interrupts on peripheral side spruh73l 6.2.1 & 6.3
    mov r1, 0x1 << 12
    str r1, [r_base, INTC_MIR_CLEAR0]    // INTC_MIR_CLEAR0 #12 SYS_INT_EDMACOMPINT
    mov r1, 0x1 << 13
    str r1, [r_base, INTC_MIR_CLEAR0]    // INTC_MIR_CLEAR0 #13 SYS_INT_EDMAMPERR
    mov r1, 0x1
    str r1, [r_base, INTC_MIR_CLEAR1]    // INTC_MIR_CLEAR1 #32 GPIOINT2A (for GPIOIRQ0)
    mov r1, 0x1 << 9
    str r1, [r_base, INTC_MIR_CLEAR1]    // INTC_MIR_CLEAR1 #41 3PGSWRXINT0 
    mov r1, 0x1 << 10
    str r1, [r_base, INTC_MIR_CLEAR1]    // INTC_MIR_CLEAR1 #42 3PGSWTXINT0  
    mov r1, 0x1
    str r1, [r_base, INTC_MIR_CLEAR2]    // INTC_MIR_CLEAR2 #64 SYS_INT_MMCSD0INT
    mov r1, 0x1 << 8
    str r1, [r_base, INTC_MIR_CLEAR2]    // INTC_MIR_CLEAR2 #72 UART0INT
    mov r1, 0x1 << 11
    str r1, [r_base, INTC_MIR_CLEAR2]    // INTC_MIR_CLEAR2 #75 RTCINT
//    mov r1, 0x1 << 31
//    str r1, [r_base, INTC_MIR_CLEAR2]    // INTC_MIR_CLEAR2 #95 TINT7

// spruh73l 26.1.3.2 default boot procedure uses address 0x4030CE00
// for base of RAM exception vectors.  see tab 26.3 for vector addresses
    ldr r_base, =0x4030CE24   // register UND in interrupt vector
    ldr r1, =und_isr
    str r1, [r_base]
    ldr r_base, =0x4030CE38   // register IRQ in interrupt vector
    ldr r1, =irq_isr
    str r1, [r_base]
    dsb

    mrs r1, cpsr
    bic r1, r1, #0x80  // enable IRQ, ie unmask IRQ bit of cpsr
    msr cpsr_c, r1     // 9.2.3.1 & fig 2.3 ARM System Developer’s Guide, Sloss et al

    bx lr
    .unreq r_base

//
// UND  undefined interrupt service routine
//
.global und_isr
und_isr:
    stmfd sp!, {r0-r1, r12, lr} // align stack to 8-byte boundary AAPCS
    mrs r12, spsr               // save spsr

    ldr r0, =0x44E09000         // this is SOC_UART_0_REGS
    mov r1, 0x55                // print "UND" (not needed remove later)
    bl uart_tx
    mov r1, 0x4E
    bl uart_tx
    mov r1, 0x44
    bl uart_tx
    mov r1, 0x0A
    bl uart_tx

    msr spsr, r12                // restore spsr
    ldmfd sp!, {r0-r1, r12, pc}^ // ^ restore cpsr from spsr.  do not adjust lr with UND 
                                 // the instruction is undefined so skip over it

//
// IRQ interrupt service routine
//
// hacked from Al Selen, github.com/auselen
// & Matthijs van Duin, TI.com e2e forum
// no priority sorting, non-nested interrupt handler per Sloss, 9.3.1
// designed for very lean ISRs that are simple, fast and no loops
.global irq_isr
irq_isr:
    stmfd sp!, {r0-r3, lr}      // must align stack to 8-byte boundary  AAPCS
    mrs r12, spsr
    stmfd r13!, {r12}           // save spsr

    mrs r1, cpsr
    orr r1, r1, #0x80           // disable IRQ
    msr cpsr_c, r1 

    ldr r0, =SOC_AINTC_REGS
    ldr r1, [r0, INTC_SIR_IRQ]  // fetch SIR_IRQ register spruh73l 6.2.2 & 6.5.1.4
    cmp r1, 0x80                // check spurious irq (only bits 0-6 are valid)
    bge irq_isr_exit

    adr r2, irq_vector          // jump to specific irq isr
    ldr r2, [r2, r1, lsl 2]
    blx r2

irq_isr_exit:
    ldr r0, =SOC_AINTC_REGS
    ldr r1, =0x1                // NewIRQAgr bit, reset IRQ output
    str r1, [r0, INTC_CONTROL]  // spruh73l 6.2.2 & 6.5.1.6
    dsb

    mrs r1, cpsr
    bic r1, r1, #0x80           // re-enable IRQ
    msr cpsr_c, r1

    ldmfd r13!, {r12}           // restore spsr
    msr spsr, r12
    ldmfd sp!, {r0-r3, lr}
    subs pc, lr, #4             // adjust lr for pipeline & return to normal execution

//
// enab IRQ
//
.global IntMasterIRQEnable
IntMasterIRQEnable:
    mrs r0, cpsr
    bic r0, r0, #0x80
    msr cpsr_c, r0
    bx lr

//
// disab IRQ
//
.global IntMasterIRQDisable
IntMasterIRQDisable:
    mrs r0, cpsr
    orr r0, r0, #0x80
    msr cpsr_c, r0
    bx lr

//
// read I & F bits of CPSR
//
.global IntMasterStatusGet
IntMasterStatusGet:
    mrs r0, cpsr
    and r0, r0, #0xc0
    bx lr

//
// default interrupt handler - do diddley
//
def:
    bx lr

//
//  ARM Cortex-A8 Interrupts  spruh73l 6.3
//
irq_vector:
.word	def                  // 0   Cortex-A8 ICECrusher
.word	def                  // 1   Cortex-A8 debug tx
.word	def                  // 2   Cortex-A8 debug rx
.word	def                  // 3   Cortex-A8 PMU
.word	def                  // 4   ELM
.word	def                  // 5   SSM WFI
.word	def                  // 6   SSM
.word	def                  // 7   External IRQ ("NMI")
.word	def                  // 8   L3 firewall error
.word	def                  // 9   L3 interconnect debug error
.word	def                  // 10  L3 interconnect non-debug error
.word	def                  // 11  PRCM MPU irq
.word DMACompletionISR     // 12  DMA client 0
.word DMACCErrorISR        // 13  DMA protection error
.word	def                  // 14  DMA CC error
.word	def                  // 15  Watchdog 0
.word	def                  // 16  ADC / Touchscreen controller
.word	def                  // 17  USB queue manager and CPPI
.word	def                  // 18  USB port 0
.word	def                  // 19  USB port 1
.word	def                  // 20  PRUSS host event 0
.word	def                  // 21  PRUSS host event 1
.word	def                  // 22  PRUSS host event 2
.word	def                  // 23  PRUSS host event 3
.word	def                  // 24  PRUSS host event 4
.word	def                  // 25  PRUSS host event 5
.word	def                  // 26  PRUSS host event 6
.word	def                  // 27  PRUSS host event 7
.word	def                  // 28  MMC 1
.word	def                  // 29  MMC 2
.word	def                  // 30  I2C 2
.word	def                  // 31  eCAP 0
.word gpio_isr             // 32  GPIO 2 irq 0  -  GPIOINT2A
.word	def                  // 33  GPIO 2 irq 1
.word	def                  // 34  USB wakeup
.word	def                  // 35  PCIe wakeup
.word	def                  // 36  LCD controller
.word	def                  // 37  SGX530 error in IMG bus
.word	def                  // 38  reserved
.word	def                  // 39  ePWM 2
.word	def                  // 40  Ethernet core 0 rx low on bufs
.word eth0_rx_isr          // 41  Ethernet core 0 rx dma completion
.word eth0_tx_isr          // 42  Ethernet core 0 tx dma completion
.word	def                  // 43  Ethernet core 0 misc irq
.word	def                  // 44  UART 3
.word	def                  // 45  UART 4
.word	def                  // 46  UART 5
.word	def                  // 47  eCAP 1
.word	def                  // 48  reserved
.word	def                  // 49  reserved
.word	def                  // 50  reserved
.word	def                  // 51  reserved
.word	def                  // 52  DCAN 0 irq 0
.word	def                  // 53  DCAN 0 irq 1
.word	def                  // 54  DCAN 0 parity
.word	def                  // 55  DCAN 1 irq 0
.word	def                  // 56  DCAN 1 irq 1
.word	def                  // 57  DCAN 1 parity
.word	def                  // 58  ePWM 0 TZ
.word	def                  // 59  ePWM 1 TZ
.word	def                  // 60  ePWM 2 TZ
.word	def                  // 61  eCAP 2
.word	def                  // 62  GPIO 3 irq 0
.word	def                  // 63  GPIO 3 irq 1
.word mmc0_isr             // 64  MMC 0
.word	def                  // 65  SPI 0
.word	def                  // 66  Timer 0
.word	def                  // 67  Timer 1
.word	def                  // 68  Timer 2
.word	def                  // 69  Timer 3
.word	def                  // 70  I2C 0
.word	def                  // 71  I2C 1
.word uart0_isr            // 72  UART 0  -  UART0INT
.word	def                  // 73  UART 1
.word	def                  // 74  UART 2
.word rtc_isr              // 75  RTC periodic  -  RTCINT
.word	def                  // 76  RTC alarm
.word	def                  // 77  System mailbox irq 0
.word	def                  // 78  Wakeup-M3
.word	def                  // 79  eQEP 0
.word	def                  // 80  McASP 0 out
.word	def                  // 81  McASP 0 in
.word	def                  // 82  McASP 1 out
.word	def                  // 83  McASP 1 in
.word	def                  // 84  reserved
.word	def                  // 85  reserved
.word	def                  // 86  ePWM 0
.word	def                  // 87  ePWM 1
.word	def                  // 88  eQEP 1
.word	def                  // 89  eQEP 2
.word	def                  // 90  External DMA/IRQ pin 2
.word	def                  // 91  Watchdog 1
.word	def                  // 92  Timer 4
.word	def                  // 93  Timer 5
.word	def                  // 94  Timer 6
.word	def // DMTimerIsr     // 95  Timer 7  -  TINT7
.word	def                  // 96  GPIO 0 irq 0
.word	def                  // 97  GPIO 0 irq 1
.word	def                  // 98  GPIO 1 irq 0
.word	def                  // 99  GPIO 1 irq 1
.word	def                  // 100 GPMC
.word	def                  // 101 EMIF 0 error
.word	def                  // 102 reserved
.word	def                  // 103 reserved
.word	def                  // 104 reserved
.word	def                  // 105 reserved
.word	def                  // 106 reserved
.word	def                  // 107 reserved
.word	def                  // 108 reserved
.word	def                  // 109 reserved
.word	def                  // 110 reserved
.word	def                  // 111 reserved
.word	def                  // 112 DMA TC 0 error
.word	def                  // 113 DMA TC 1 error
.word	def                  // 114 DMA TC 2 error
.word	def                  // 115 Touchscreen Pen
.word	def                  // 116 reserved
.word	def                  // 117 reserved
.word	def                  // 118 reserved
.word	def                  // 119 reserved
.word	def                  // 120 SmartReflex 0 (MPU)
.word	def                  // 121 SmartReflex 1 (core)
.word	def                  // 122 reserved
.word	def                  // 123 External DMA/IRQ pin 0
.word	def                  // 124 External DMA/IRQ pin 1
.word	def                  // 125 SPI 1
.word	def                  // 126 reserved
.word	def                  // 127 reserved

// eof
