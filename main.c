//
// BeagleBone Black  Bare Metal UDP/TCP/MMC
//
// mixed C & assembly 
// demo Ethernet, MMC, MMU, DMA, Cache, IRQ, UART0, RTC and GPIO usage
//
// This is a modded port [TI Starterware] and other contributors 
//
// The BBB will tell you it's MAC address and IP# then it will spew 
// UDP packets out UART0 as they are received and log them.
// It will also respond to ping and HTTP requests  
//
// Hit the boot button to flash a USR LED (interrupt function check)
// Enter ? to display console functions on UART0 console
// USERLEDs will flash with IRQ activity for MMC and ethernet
// 
// lwIP is a Picasso!  It is quite intricate so it is largely untouched.  
// The raw TCP/IP interface and app run in the same thread.  See rawapi.txt.
// Data transfer is by DMA dedicated to PHY.  Ethernet i/o is handled by ISR.  
// lwIP middleware in lwiplib.c. ISRs, DMA and config logic in cpswif.c.  Lower 
// code is in eth_hwif.c and eth.s  There are many unused routines in lwiplib.c.  
// Removing them will save much memory.  Total image size is only 85k Bytes!  
// The ethernet drivers are pretty good and required very little tweaking to build.  
// lwIP has very cool DEBUG features in opt.h.  You can enable debug messages at 
// any level of the stack (ETH/IP/UDP/TCP/ARP/DHCP...) and follow [de]packetization 
// and examine headers as it happens.  
//
// Chan at www.elm-chan.org wrote a great multi-platform micro-filesystem utility.
// The MMC/FAT32 filesystem runs concurrently.  Data transfer is by DMA.  
// MMC i/o is handled by ISR.  MMC drivers were also well written and are mostly 
// detangled.  ff.c contains the FAT32 middleware.  diskio.c the underlying drivers
// then mmc_api.c and mmc_hwif.c and mmc.s underneath that.
//
// No IRQs within IRQs.  The h/w itself queues IRQs.  This is basic and fast but 
// the app programmer must manage IRQ traffic.  
//
// Built with GNU/ARM tools :) on platform Win32 :(
// Also builds with GNU tools on (Raspberry) Pi-Top laptop running Linux :) 
// Changes are marked with "// n!"
// Load/boot via UART, MMC or jtag   see the Makefile and load-script.
//
// Boot sequence:  
//   bootloader "MLO" is read from MMC.  The MLO bootloader loads image "app" 
//   via UART0/xmodem (terminal emulator minicom on Linux or ExtraPutty on Windows)
//   image is loaded to 0x80000000 and execution begins there
//   per Fast External Booting 26.1.6 spruh73n
//   - or -    boot via MMC card
//   rename "MLO_MMC" to "MLO" this will load/run "app" automatically 
//   All these are provided in the zipfile  
//
// Known bugs/TODO:
//   MMC file copy only 1Mbyte/sec - needs further debugging
//   move MLO bootloader to ROM - for dev/debug
//   move image to on-board NAND - for runtime release
//   UART XMODEM file transfer
//   implement web page serving from the filesystem
//   hotplug ethernet/lwIP init
//
// This is somewhat ready to build a tough little server that can be 
// integrated with sensors and actuators for Internet-of-Things.  Or you
// can strip it down to pure TCP/UDP for blockchain, torrent, firewall
// or encrypted communication.  It is totally self-contained and free.  
// It is not so easy at first glance, but it is do-able.  The building-
// blocks are all here.  Collaboration is most welcome... 
// 
// get involved   www.baremetal.tech            d7@hushmail.com
//
// LEGAL NOTICE:  This is abandoned knowledge.  It does not work.  
//                All references to TI and other corporations are 
//                provided for information purposes only.  
//
#include <stdint.h>
#include "bbb_main.h"
#include "soc_AM335x.h"
#include "beaglebone.h"
#include "locator.h"
#include "lwiplib.h"
#include "lwipopts.h"
#include "ff.h"
#include "mmc_api.h"
#include "con_uif.h"
#include "udp_apps.h"
#include "echod.h"
#include "httpd.h"

uint32_t uart0_irq_old=0, udp_rx_old=0;

//
// main
//
void main(void){
  uint32_t i, j, k, ipAddr, iblink=0, iled=1, irev=0, master;
  LWIP_IF lwipIfPort1;
  char *udp_buf_p[2], udp_buf[MAX_SIZE_UDP];  // udp app tx buffer

  udp_buf_p[1] = udp_buf;

  cache_en();
  mclk_1GHz();
  gpio_init(SOC_GPIO_1_REGS, (0xf << 21));  // enab USR LEDs, pin # 21-24
  uart0_init();
  tim_init();
  rtc_init();
  irq_init();
  mmu_init();
  mmc0_start();
  eth_init();

  ipAddr = 0;
  MACAddrGet(0, lwipIfPort1.macArray);  // get MAC address port 1
  ConsolePrintf("\r\nPort 1 MAC:  ");
  for(i = 0; i <= 5; i++) {
    hexprintbyte(lwipIfPort1.macArray[5-i]);
    uart_tx(SOC_UART_0_REGS, 0x20);
  }
  ConsolePrintf("\r\n");

  lwipIfPort1.instNum = 0;
  lwipIfPort1.slvPortNum = 1; 
  lwipIfPort1.ipAddr = 0; 
  lwipIfPort1.netMask = 0; 
  lwipIfPort1.gwAddr = 0; 
  lwipIfPort1.ipMode = IPADDR_USE_DHCP; 
  ipAddr = lwIPInit(&lwipIfPort1);
  if(ipAddr) {
    ConsolePrintf("\n   ***   UDP Peer 2 Peer  ***\r\n\n");
    ConsolePrintf("Port 1 IP Address: %d.%d.%d.%d\r\n", (ipAddr & 0xFF), 
     ((ipAddr >> 8) & 0xFF), ((ipAddr >> 16) & 0xFF), ((ipAddr >> 24) & 0xFF));
  } else {
    ConsolePrintf("Port 1 IP acquisition failed\r\n");
  }

  echo_init();  // init echo server

  httpd_init();  // init http server

  if(ipAddr == BONE1) {
    master = 1;
    udp_app_init(master);  // udp peer-master init
    ConsolePrintf("UDP Master init\r\n");
  } else {
    master = 0;
    udp_app_init(master);  // udp peer-slave init
    ConsolePrintf("UDP Slave init\r\n");
  }

  if(!FATFsMount(0, &sdCard, &CmdBuf[0])) ConsolePrintf("Filesystem mounted\n\r");

  CmdBufIdx = 0;            // init command line buffer
  CmdBuf[CmdBufIdx] = '\0';
  ConsolePrintf("Enter ? for help\n\r");
  j = 0x30;
  k = 0;
  while(1) {  // loop 4 ever
//
// application code here
// design it as a non-blocking state machine
// execution must return back to this loop
// custom console commands go in file con_uif.c and array CmdTable[]
//
    k++;
    if((k == 0xFFFFFFF) && !master) {  // intermittent UDP packet Tx if a slave
      k = 0;
      for(i = 0; i < 40; i++) udp_buf[i] = (char)j;
      udp_buf[40] = (char)0x0;
      Cmd_udptx(2, udp_buf_p);  // hack job using a console command inapropos
      j++;
      if(j > 0x39) j = 0x30;
    }
    if(udp_rx_old != udp_rx_count) {  // detect UDP packet Rx
      udp_rx_count &= 0x7fffffff;
      udp_rx_old = udp_rx_count;
      udp_rx_process();
    }

//
// end application code
//
    if(uart0_irq_old != uart0_irq_count) {  // detect console keystroke
      uart0_irq_count &= 0x7fffffff;
      uart0_irq_old = uart0_irq_count;
      if(uart0_rbuf > 0x0) {                // ASCII character received
        char_collect((char)uart0_rbuf);     // char process/execute
        uart0_rbuf = 0x0;
      }
    }

    if(iblink == 0x0) gpio_on(SOC_GPIO_1_REGS, iled<<21);  // blink USRLEDs
    if(iblink == 0xffff) {                 // blink intensity
      gpio_off(SOC_GPIO_1_REGS, iled<<21);
      if(irev == 0) iled = iled<<1;
      else iled = iled>>1;
      if(iled == 0x1) irev = 0x0;
      if(iled == 0x8) irev = 0x1;
    }
    iblink = (iblink + 1) & 0x3fffff;      // blink period
  }
}

// eof
