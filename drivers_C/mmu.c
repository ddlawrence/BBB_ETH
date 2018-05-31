//
//  Memory Management Unit
//  
//  functions originally written by TIdude
//  configure ARMv7a MMU
//
#include "mmu.h"
#include "cp15.h"

#define START_ADDR_OCMC                 0x40300000
#define START_ADDR_DDR                  0x80000000
#define START_ADDR_DEV                  0x44000000
#define START_ADDR_ROM                  0x00020000  // ROM interrupt vectors here

#define NUM_SECTIONS_DDR                512
#define NUM_SECTIONS_DEV                960
#define NUM_SECTIONS_OCMC               1
#define NUM_SECTIONS_ROM                1

#define MMU_PAGETABLE_ENTRY_FAULT       0x00
#define MMU_PAGEBOUND_SHIFT             20
#define MMU_PG_SUPSECT_SIZE_SHIFT       14
#define MMU_PGADDR_MASK                 0xFFF00000
#define MMU_PGTYPE_MASK                 0x00040002

// translation table sits here, 4096 entries, on a 16k boundary
static volatile unsigned int pageTable[MMU_PAGETABLE_NUM_ENTRY] 
            __attribute__((aligned(MMU_PAGETABLE_ALIGN_SIZE)));

//
//  Map a specific region for Virtual Address to Physical Address translation 
//  All regions are flat-mapped:  Virtual Address = Physical Address
//  Regions can be specified with Memory Type, Inner/Outer Cache settings, 
//  Security settings and Access Permissions
//          
void mmu_regionMap(REGION *region) {
  unsigned int *ptEntryPtr;
  unsigned int ptEntry;
  int i;

  // Get first entry in page table 
  ptEntryPtr = region->masterPtPtr + (region->startAddr >> MMU_PAGEBOUND_SHIFT);
  // Set pointer to last entry 
  ptEntryPtr += (region->numPages - 1); 
  // Get start Address MSB 3 nibbles. Ignore extended address 
  ptEntry = (region->startAddr & region->pgType) & MMU_PGADDR_MASK;
  // Update the table entry with memory attributes and
  // Access Permissions and Security.
  // All the regions will be marked as global.
  ptEntry |= ((MMU_PGTYPE_MASK & region->pgType) 
             | region->accsCtrl | region->memAttrib | region->secureType);
  // Set entries in page table to region attributes 
  for(i = (region->numPages - 1); i >= 0; i--)
   *ptEntryPtr-- = ptEntry + (i << MMU_PAGEBOUND_SHIFT) ;
}

//
//  initialize the mmu
//  single level paging only
//  TTB0 only is used for page table walking
//
void mmu_init(void) {
  unsigned int i;

  // DDR memory region.  Normal memory with R/W access in user/priv modes.  
  // Cache attributes: Inner - Write through, No Write Allocate
  //                   Outer - Write Back, Write Allocate
  REGION regionDdr={MMU_PGTYPE_SECTION, START_ADDR_DDR, NUM_SECTIONS_DDR,
                    MMU_MEMTYPE_NORMAL_NON_SHAREABLE(MMU_CACHE_WT_NOWA, MMU_CACHE_WB_WA),
                    MMU_REGION_NON_SECURE, MMU_AP_PRV_RW_USR_RW, (unsigned int*)pageTable};
  // OCMC RAM region. Same Attributes as DDR
  REGION regionOcmc={MMU_PGTYPE_SECTION, START_ADDR_OCMC, NUM_SECTIONS_OCMC,
                     MMU_MEMTYPE_NORMAL_NON_SHAREABLE(MMU_CACHE_WT_NOWA, MMU_CACHE_WB_WA),
                     MMU_REGION_NON_SECURE, MMU_AP_PRV_RW_USR_RW, (unsigned int*)pageTable};
  // Device Memory region between OCMC & DDR.  R/W access in user/priv modes, exec never
  REGION regionDev={MMU_PGTYPE_SECTION, START_ADDR_DEV, NUM_SECTIONS_DEV,
                    MMU_MEMTYPE_DEVICE_SHAREABLE, MMU_REGION_NON_SECURE,
                    MMU_AP_PRV_RW_USR_RW | MMU_SECTION_EXEC_NEVER, (unsigned int*)pageTable};
  // ROM region where the exception vector resides
  REGION regionROM={MMU_PGTYPE_SECTION, START_ADDR_ROM, NUM_SECTIONS_ROM,
                    MMU_MEMTYPE_NORMAL_NON_SHAREABLE(MMU_CACHE_WT_NOWA, MMU_CACHE_WB_WA),
                    MMU_REGION_NON_SECURE, MMU_AP_PRV_RW_USR_RW, (unsigned int*)pageTable};
  CP15TlbInvalidate();  // Invalidate the TLB entries
  CP15DomainAccessClientSet();  // Set domain access rights
  CP15ControlFeatureDisable( CP15_CONTROL_TEXREMAP    // disable TEX remapping
                           | CP15_CONTROL_ACCESSFLAG  // access flag usage
                           | CP15_CONTROL_ALIGN_CHCK  // alignment check 
                           | CP15_CONTROL_MMU); 
  CP15TtbCtlTtb0Config();  // config TTB Control Register for TTB0
  // reset page table entries with fault 0x0 
  for(i = MMU_PAGETABLE_NUM_ENTRY; i !=0; i--) pageTable[i]=0x0;
  mmu_regionMap(&regionDdr);  // map and go
  mmu_regionMap(&regionOcmc);
  mmu_regionMap(&regionDev);
  mmu_regionMap(&regionROM);
  CP15Ttb0Set((unsigned int)pageTable);  // Set TTB0 register, Translation Table Base Address
  CP15MMUEnable();                        // Enable MMU 
}

// eof
