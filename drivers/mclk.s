//
//  master clock routines 
//
.syntax unified

.data

CM_PER_BASE             = 0x44e00000
CM_WKUP                 = 0x400
CM_IDLEST_DPLL_CORE     = CM_WKUP + 0x5C  // monitor master clock activity
CM_CLKSEL_DPLL_CORE     = CM_WKUP + 0x68  // DPLL MUL & DIV
CM_DIV_M4_DPLL_CORE     = CM_WKUP + 0x80  // CLKOUT1
CM_DIV_M5_DPLL_CORE     = CM_WKUP + 0x84  // CLKOUT2
CM_CLKMODE_DPLL_CORE    = CM_WKUP + 0x90  // DPLL mode control
CM_DIV_M6_DPLL_CORE     = CM_WKUP + 0xD8  // CLKOUT3

.macro	mov32 regnum,number      // macro mov32
  movw \regnum,:lower16:\number
  movt \regnum,:upper16:\number
.endm

.text	

//
// bump up master clock to 1GHz
//
// from Viktor www.filmprocess.ru
//
.global mclk_1GHz
mclk_1GHz:
  mov32  r0, CM_PER_BASE
  ldr r1, [r0, CM_CLKMODE_DPLL_CORE]
  bic r1, 0x07
  orr r1, 0x04                       // DPLL BYPASS MODE spruh73n 8.1.6.7.1
  str r1, [r0, CM_CLKMODE_DPLL_CORE]
1:
  ldr r1, [r0, CM_IDLEST_DPLL_CORE]
  tst r1, (1<<8)
  beq 1b

  mov32 r1, (1000 << 8) + 23         // DPLL  M=1000 MHz  range 2-2047
  str r1, [r0, CM_CLKSEL_DPLL_CORE]  // spruh73n 8.1.12.2.27
  
  mov r1, 10                         // CLKOUT1  M4=200 MHz
  str r1, [r0, CM_DIV_M4_DPLL_CORE]
  mov r1, 8                          // CLKOUT2  M5=250 MHz
  str r1, [r0, CM_DIV_M5_DPLL_CORE]
  mov r1, 4                          // CLKOUT3  M6=500 MHz
  str r1, [r0, CM_DIV_M6_DPLL_CORE]

  ldr r1, [r0, CM_CLKMODE_DPLL_CORE]
  bic r1, 0x07
  orr r1, 0x07                     // DPLL enab
  str r1, [r0, CM_CLKMODE_DPLL_CORE]
1:
  ldr r1, [r0, CM_IDLEST_DPLL_CORE]
  tst r1, 1
  beq 1b

  bx lr

//
// cache enab
//
// run this before mclk_1GHz() if your application 
// does not already enable cache
// from Viktor www.filmprocess.ru
//
.global cache_en
cache_en:
// CPU preset   mode ARM/Thumb
  mrs r0, CPSR             // read CPSR
  orr r0, 0x80 + 0x40      // set I/F bit
  msr CPSR, r0             // write back to disable IRQs
  dsb
  dmb

// enable data and instruction cache
  mrc p15, 0, r1, c1, c0, 0
  orr r1, (1<<2)           // enab data cache
  orr r1, (1<<12)          // enab instruction cache
  orr r1, (1<<11)          // including branch prediction
  mcr p15, 0, r1, c1, c0, 0
  dsb
  dmb

// enable cache L2 and parity control L1
  mrc p15, 0, r1, c1, c0, 1
  orr r1, (1<<1)           // enab L2 cache
  orr r1, (1<<3)           // enab L1 cache parity detection
  mcr p15, 0, r1, c1, c0, 1
  dsb
  dmb

  bx lr
