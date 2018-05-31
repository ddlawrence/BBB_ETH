
BeagleBone Black  Bare Metal UDP/TCP/MMC

mixed C & assembly 
demo Ethernet, MMC, MMU, DMA, Cache, IRQ, UART0, RTC and GPIO usage

This is a modded port [TI Starterware] and other contributors 

The BBB will tell you it's MAC address and IP# then it will spew UDP packets out UART0 as they are received and log them.
It will also respond to ping and HTTP requests  

Hit the boot button to flash a USR LED (interrupt function check) Enter ? to display console functions on UART0 console
USERLEDs will flash with IRQ activity for MMC and ethernet 

lwIP is a Picasso!  It is quite intricate so it is largely untouched.  The raw TCP/IP interface and app run in the same thread.  See rawapi.txt.
Data transfer is by DMA dedicated to PHY.  Ethernet i/o is handled by ISR.  lwIP middleware in lwiplib.c. ISRs, DMA and config logic in cpswif.c.  Lower 
code is in eth_hwif.c and eth.s  There are many unused routines in lwiplib.c.  Removing them will save much memory.  Total image size is only 85k Bytes!  
The ethernet drivers are pretty good and required very little tweaking to build.  lwIP has very cool DEBUG features in opt.h.  You can enable debug messages at 
any level of the stack (ETH/IP/UDP/TCP/ARP/DHCP...) and follow [de]packetization and examine headers as it happens.  

Chan at www.elm-chan.org wrote a great multi-platform micro-filesystem utility.
The MMC/FAT32 filesystem runs concurrently.  Data transfer is by DMA.  MMC i/o is handled by ISR.  MMC drivers were also well written and are mostly 
detangled.  ff.c contains the FAT32 middleware.  diskio.c the underlying drivers then mmc_api.c and mmc_hwif.c and mmc.s underneath that.

No IRQs within IRQs.  The h/w itself queues IRQs.  This is basic and fast but the app programmer must manage IRQ traffic.  

Built with GNU/ARM tools :-) on platform Win32 :-(
Also builds with GNU tools on (Raspberry) Pi-Top laptop running Linux :-) 

ps If the project does not build correctly, get the tested zipfile from www.baremetal.tech
