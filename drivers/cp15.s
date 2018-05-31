//
// cp15 coprocessor routines
//
// originally written by TIdude 
//
.global CP15ICacheDisable
.global CP15DCacheDisable
.global CP15ICacheEnable
.global CP15DCacheEnable
.global CP15ICacheFlush
.global CP15DCacheCleanFlush
.global CP15DCacheClean
.global CP15DCacheFlush
.global CP15DCacheCleanBuff
.global CP15DCacheCleanFlushBuff
.global CP15DCacheFlushBuff
.global CP15ICacheFlushBuff
.global CP15Ttb0Set
.global CP15TlbInvalidate
.global CP15MMUDisable
.global CP15MMUEnable
.global CP15DomainAccessClientSet
.global CP15ControlFeatureDisable
.global CP15TtbCtlTtb0Config
.global CP15AuxControlFeatureEnable
.global CP15AuxControlFeatureDisable
.global CP15MainIdPrimPartNumGet
.global CP15VectorBaseAddrSet

.text

// disable Instruction cache
CP15ICacheDisable:
  push {lr}
  mrc p15, #0, r0, c1, c0, #0
  bic r0, r0, #0x1000
  mcr p15, #0, r0, c1, c0, #0
  bl CP15ICacheFlush
  pop {lr}
  bx lr

// disable the Data cache
CP15DCacheDisable:
  push {r4-r11, lr}
  mrc p15, #0, r0, c1, c0, #0
  bic r0, r0, #0x4
  mcr p15, #0, r0, c1, c0, #0
  bl CP15DCacheCleanFlush
  pop {r4-r11, lr}
  bx lr

// enable the I-cache
CP15ICacheEnable:
  mrc p15, #0, r0, c1, c0, #0
  orr r0, r0, #0x1000
  mcr p15, #0, r0, c1, c0, #0 
  bx lr

// enable the Data cache
CP15DCacheEnable:
  mrc p15, #0, r0, c1, c0, #0 
  orr r0, r0, #0x4
  mcr p15, #0, r0, c1, c0, #0
  bx lr

// Invalidate the entire Data/Unified Cache
CP15DCacheFlush:
  push {r4-r11}
  dmb
  mrc p15, #1, r0, c0, c0, #1  // Read CLID register
  ands r3, r0, #0x7000000       // Get Level of Coherency
  mov r3, r3, lsr #23
  beq ffinished
  mov r10, #0
floop1:
  add r2, r10, r10, lsr #1
  mov r1, r0, lsr r2
  and r1, r1, #7
  cmp r1, #2
  blt fskip
  mcr p15, #2, r10, c0, c0, #0
  isb
  mrc p15, #1, r1, c0, c0, #0
  and r2, r1, #7
  add r2, r2, #4
  ldr r4, _FLD_MAX_WAY
  ands r4, r4, r1, lsr #3
  clz r5, r4
  ldr r7, _FLD_MAX_IDX
  ands r7, r7, r1, lsr #13
floop2:
  mov r9, r4
floop3:
  orr r11, r10, r9, lsl r5
  orr r11, r11, r7, lsl r2
  mcr p15, #0, r11, c7, c6, #2
  subs r9, r9, #1
  bge floop3
  subs r7, r7, #1
  bge floop2
fskip:
  add r10, r10, #2
  cmp r3, r10
  bgt floop1

ffinished:
  dsb
  isb
  pop {r4-r11}
  bx lr

// clean the entire D Cache to PoC
CP15DCacheClean:
  push {r4-r11}
  dmb
  mrc p15, #1, r0, c0, c0, #1  // Read CLID register
  ands r3, r0, #0x7000000       // Get Level of Coherency
  mov r3, r3, lsr #23
  beq cfinished
  mov r10, #0
cloop1:
  add r2, r10, r10, lsr #1
  mov r1, r0, lsr r2
  and r1, r1, #7
  cmp r1, #2
  blt cskip
  mcr p15, #2, r10, c0, c0, #0
  isb
  mrc p15, #1, r1, c0, c0, #0
  and r2, r1, #7
  add r2, r2, #4
  ldr r4, _FLD_MAX_WAY
  ands r4, r4, r1, lsr #3
  clz r5, r4
  ldr r7, _FLD_MAX_IDX
  ands r7, r7, r1, lsr #13
cloop2:
  mov r9, r4
cloop3:
  orr r11, r10, r9, lsl r5
  orr r11, r11, r7, lsl r2
  mcr p15, #0, r11, c7, c10, #2
  subs r9, r9, #1
  bge cloop3
  subs r7, r7, #1
  bge cloop2
cskip:
  add r10, r10, #2
  cmp r3, r10
  bgt cloop1

cfinished:
  dsb
  isb
  pop {r4-r11}
  bx lr

// clean and invalidate the entire D Cache to PoC
CP15DCacheCleanFlush:
  push {r4-r11} 
  dmb   
  mrc p15, #1, r0, c0, c0, #1  // Read CLID register
  ands r3, r0, #0x7000000       // Get Level of Coherency
  mov r3, r3, lsr #23  
  beq finished  
  mov r10, #0 
loop1:
  add r2, r10, r10, lsr #1 
  mov r1, r0, lsr r2
  and r1, r1, #7 
  cmp r1, #2  
  blt skip 
  mcr p15, #2, r10, c0, c0, #0 
  isb 
  mrc p15, #1, r1, c0, c0, #0 
  and r2, r1, #7 
  add r2, r2, #4   
  ldr r4, _FLD_MAX_WAY 
  ands r4, r4, r1, lsr #3
  clz r5, r4
  ldr r7, _FLD_MAX_IDX
  ands r7, r7, r1, lsr #13 
loop2:
  mov r9, r4   
loop3:
  orr r11, r10, r9, lsl r5  
  orr r11, r11, r7, lsl r2 
  mcr p15, #0, r11, c7, c14, #2 
  subs r9, r9, #1 
  bge loop3 
  subs r7, r7, #1 
  bge loop2 
skip: 
  add r10, r10, #2 
  cmp r3, r10 
  bgt loop1 

finished:
  dsb
  isb 
  pop {r4-r11} 
  bx lr

// invalidate entire I Cache
CP15ICacheFlush:
  mov r0, #0
  mcr p15, #0, r0, c7, c5, #0
  dsb
  bx lr

// clean the D-cache/Unified  lines corresponding to the buffer 
// pointer upto the specified length to PoC.
// r0 - Start Address 
// r1 - Number of bytes to be cleaned
CP15DCacheCleanBuff:
  push {r14}
  add r14, r0, r1               // Calculate the end address
  dmb
  mrc p15, #0, r2, c0, c0, #1   // Read Cache Type Register
  ubfx r2, r2, #16, #4          // Extract the DMinLine
  mov r3, #2
  add r3, r3, r2
  mov r2, #1
  lsl r2, r2, r3                // Calculate the line size
   
  sub r3, r2, #1                // Calculate the mask
  bic r0, r0, r3                // Align to cache line boundary   
  tst r3, r14
  bic r14, r14, r3
  mcrne p15, #0, r14, c7, c10, #1 // Clean D/Unified to PoC by MVA

cleanloop:
  mcr p15, #0, r0 , c7, c10, #1 // Clean D/Unified to PoC by MVA
  adds r0, r0, r2                // Go to next line
  cmp r0, r14 
  blt cleanloop
 
  pop {r14}
  dsb
  bx lr

// clean and invalidate the D-cache/Unified lines corresponding to 
// the buffer pointer upto the specified length to PoC.
// r0 - Start Address 
// r1 - Number of bytes to be cleaned and flushed
CP15DCacheCleanFlushBuff:
  push {r14}
  add r14, r0, r1               // Calculate the end address
  dmb
  mrc p15, #0, r2, c0, c0, #1   // Read Cache Type Register
  ubfx r2, r2, #16, #4           // Extract the DMinLine
  mov r3, #2
  add r3, r3, r2
  mov r2, #1
  lsl r2, r2, r3                // Calculate the line size
   
  sub r3, r2, #1                // Calculate the mask
  bic r0, r0, r3                // Align to cache line boundary   
  tst r3, r14
  bic r14, r14, r3
  mcrne p15, #0, r14, c7, c14, #1 // Clean and Flush D/U line to PoC

cleanflushloop:  
  mcr p15, #0, r0 , c7, c14, #1 // Clean and Flush D/U line to PoC 
  adds r0, r0, r2                // Go to next line
  cmp r0, r14 
  blt cleanflushloop
 
  pop {r14}
  dsb
  bx lr

// invalidate the D-cache/Unified lines corresponding to 
// the buffer pointer upto the specified length to PoC.
// r0 - Start Address 
// r1 - Number of bytes to be flushed
CP15DCacheFlushBuff:
  push {r14}
  add r14, r0, r1               // Calculate the end address
  dmb
  mrc p15, #0, r2, c0, c0, #1   // Read Cache Type Register
  ubfx r2, r2, #16, #4          // Extract the DMinLine
  mov r3, #2
  add r3, r3, r2
  mov r2, #1
  lsl r2, r2, r3                // Calculate the line size

  sub r3, r2, #1                // Calculate the mask
  tst r3, r0 
  bic r0, r0, r3                // Align to cache line boundary   
  mcrne p15, #0, r0, c7, c14, #1  // Clean and Flush D/U line to PoC 
  addne r0, r0, r2
  tst r3, r14 
  bic r14, r14, r3    
  mcrne p15, #0, r14, c7, c14, #1 // Clean and Flush D/U line to PoC 
  B     dflushcmp

dflushloop:
  mcr p15, #0, r0 , c7, c6, #1  // Flush D/U line to PoC  
  adds r0, r0, r2                // Go to next line

dflushcmp:
  cmp r0, r14
  blt dflushloop
  pop {r14}
  dsb
  bx lr

// invalidate I-cache lines from the start address until 
// the length specified to PoU.
// r0 - Start Address 
// r1 - Number of bytes to be cleaned
CP15ICacheFlushBuff:
  push {r14}
  add r14, r0, r1               // Calculate the end address
  dmb
  mrc p15, #0, r2, c0, c0, #1   // Read Cache Type Register
  ubfx r2, r2, #0, #4            // Extract the DMinLine
  mov r3, #2
  add r3, r3, r2
  mov r2, #1
  lsl r2, r2, r3                // Calculate the line size
   
  sub r3, r2, #1                // Calculate the mask
  bic r0, r0, r3                // Align to cache line boundary   
  tst r3, r14
  bic r14, r14, r3
  mcrne p15, #0, r14, c7, c5, #1  // Invalidate by MVA to PoU

iflushloop:  
  mcr p15, #0, r0, c7, c5, #1   // Invalidate by MVA to PoU
  adds r0, r0, r2                // Go to next line
  cmp r0, r14 
  blt iflushloop
 
  pop {r14}
  dsb
  bx lr

// set TTB0 Register
// r0 - Translation Table Base Address
CP15Ttb0Set:
  mcr p15, #0, r0, c2, c0, #0
  dmb
  bx lr

// invalidate the TLB
CP15TlbInvalidate:
  mov r0, #0
  mcr p15, #0, r0, c8, c7, #0
  dsb
  bx lr

// disable MMU 
CP15MMUDisable:
  mcr p15, #0, r0, c8, c7, #0    // Invalidate TLB  
  mrc p15, #0, r0, c1, c0, #0  
  bic r0, r0, #1    
  mcr p15, #0, r0, c1, c0, #0    // Clear MMU bit
  dsb  
  bx lr

// enable MMU 
CP15MMUEnable:
  mrc p15, #0, r0, c1, c0, #0
  orr r0, r0, #0x1
  mcr p15, #0, r0, c1, c0, #0    // Set MMU Enable bit
  dsb
  bx lr

// set the domain access to client 
CP15DomainAccessClientSet:
  ldr r0, _CLIENTD 
  mcr p15, #0, r0, c3, c0, #0
  dsb
  bx lr


// disable specified features in CP15 control register
//  r0 -  features   Features to disable in Coprocessor 15 control register. 
//       'features' can take any OR a combination of the below  values. 
//        CP15_CONTROL_TEXREMAP - TEX remap flag 
//        CP15_CONTROL_ACCESSFLAG - Access flag Control 
//        CP15_CONTROL_ALIGN_CHCK - Alignment Fault Checking 
//        CP15_CONTROL_MMU - To enable MMU 
CP15ControlFeatureDisable:
  mrc p15, #0, r1, c1, c0, #0 
  bic r0, r1, r0  
  mcr p15, #0, r0, c1, c0, #0
  dsb
  bx lr

// enable specified features in CP15 control register
//  r0 -  features   Features to disable in Coprocessor 15 control register. 
//       'features' can take any OR a combination of the below  values. 
//        CP15_CONTROL_TEXREMAP - TEX remap flag 
//        CP15_CONTROL_ACCESSFLAG - Access flag Control 
//        CP15_CONTROL_ALIGN_CHCK - Alignment Fault Checking 
//        CP15_CONTROL_MMU - To enable MMU 
CP15ControlFeatureEnable:
  mrc p15, #0, r1, c1, c0, #0 
  orr r0, r1, r0  
  mcr p15, #0, r0, c1, c0, #0
  dsb
  bx lr

// configure the TTB control register to use only TTB0
CP15TtbCtlTtb0Config:
  mov r0, #0x0
  mcr p15, #0, r0, c2, c0, #2
  dsb
  bx lr

// set the specified fields in Auxiliary Control Register
// r0 - Bit Mask for the bits to be set in Auxiliary Control Register
CP15AuxControlFeatureEnable:
  mrc p15, #0, r1, c1, c0, #1 
  orr r0, r0, r1
  mcr p15, #0, r0, c1, c0, #1
  dsb
  bx lr

// clear the specified fields in Auxiliary Control Register
// r0 - Bit Mask for the bits to be cleared in Auxiliary Control Register
CP15AuxControlFeatureDisable:
  mrc p15, #0, r1, c1, c0, #1 
  bic r0, r1, r0
  mcr p15, #0, r0, c1, c0, #1
  dsb
  bx lr

// return the main ID register in r0
CP15MainIdPrimPartNumGet:
  mrc p15, #0, r0, c0, c0, #0
  ubfx r0, r0, #4, #12
  bx lr

// set vector table base address   ARM doc Cortex-A8 TRM DDI0344K 3.2.68
// r0 - Vector Base Address
CP15VectorBaseAddrSet:
  mcr p15, #0, r0, c12, c0, #0
  dsb
  bx lr

_CLIENTD: 
.word  0x55555555
_FLD_MAX_WAY:
.word  0x3ff
_FLD_MAX_IDX:
.word  0x7ff
