# arm-linux-gnueabihf  toolchain for Linux based compiling
# arm-none-eabi  toolchain for Windows based compiling
# CROSSCOMPILE = arm-linux-gnueabihf-
CROSSCOMPILE = arm-none-eabi-

.SUFFIXES:
.SUFFIXES: .c .s .h .o
DRV=drivers/
DRVC=drivers_C/
OBJ=obj/

CFLAGS = -mcpu=cortex-a8 -marm -Wall -O2 -nostdlib -nostartfiles -ffreestanding \
  -fstack-usage -Wstack-usage=16384 -I $(DRV) \
  -I $(DRVC) \
  -I $(DRVC)include \
  -I lwip \
  -I lwip/apps/udp_raw \
  -I lwip/apps/echoserver_raw \
  -I lwip/apps/httpserver_raw \
  -I lwip/ports/cpsw \
  -I lwip/ports/cpsw/include \
  -I lwip/ports/cpsw/netif \
  -I lwip/src/include \
  -I lwip/src/include/ipv4 \
  -I lwip/src/include/lwip \
  -I mmc_lib \
  -I mmc_lib/include \
  -I mmc_lib/fatfs/src

VPATH = $(DRV) lwip/ports/cpsw lwip/ports/cpsw/netif lwip/src/core

all : rts.elf

$(OBJ)main.o : main.c
	$(CROSSCOMPILE)gcc $(CFLAGS) -c main.c -o $(OBJ)main.o

$(OBJ)%.o : $(DRV)%.s
	$(CROSSCOMPILE)gcc $(CFLAGS) -c $< -o $@

$(OBJ)%.o : $(DRVC)%.c
	$(CROSSCOMPILE)gcc $(CFLAGS) -c $< -o $@

$(OBJ)lwiplib.o : lwip/ports/cpsw/lwiplib.c lwip/ports/cpsw/netif/cpswif.c
	$(CROSSCOMPILE)gcc $(CFLAGS) -c lwip/ports/cpsw/lwiplib.c -o $(OBJ)lwiplib.o

$(OBJ)%.o : mmc_lib/%.c
	$(CROSSCOMPILE)gcc $(CFLAGS) -c $< -o $@

$(OBJ)diskio.o : mmc_lib/fatfs/port/diskio.c
	$(CROSSCOMPILE)gcc $(CFLAGS) -c $< -o $@

$(OBJ)ff.o : mmc_lib/fatfs/src/ff.c
	$(CROSSCOMPILE)gcc $(CFLAGS) -c $< -o $@

$(OBJ)udp_apps.o : lwip/apps/udp_raw/udp_apps.c
	$(CROSSCOMPILE)gcc $(CFLAGS) -c lwip/apps/udp_raw/udp_apps.c -o $(OBJ)udp_apps.o

$(OBJ)echod.o : lwip/apps/echoserver_raw/echod.c
	$(CROSSCOMPILE)gcc $(CFLAGS) -c lwip/apps/echoserver_raw/echod.c -o $(OBJ)echod.o

$(OBJ)httpd.o : lwip/apps/httpserver_raw/httpd.c
	$(CROSSCOMPILE)gcc $(CFLAGS) -c lwip/apps/httpserver_raw/httpd.c -o $(OBJ)httpd.o

rts.elf : memmap.lds $(OBJ)*.o
	$(CROSSCOMPILE)ld -o rts.elf -T memmap.lds $(OBJ)startup.o $(OBJ)irq.o $(OBJ)mclk.o \
	$(OBJ)gpio.o $(OBJ)uart.o $(OBJ)tim.o $(OBJ)cp15.o $(OBJ)mmc.o $(OBJ)eth.o \
	$(OBJ)dma.o $(OBJ)mmu.o $(OBJ)mmc_hwif.o $(OBJ)mmc_api.o $(OBJ)diskio.o \
	$(OBJ)ff.o $(OBJ)con_uif.o $(OBJ)eth_hwif.o $(OBJ)mdio.o $(OBJ)phy.o $(OBJ)lwiplib.o \
	$(OBJ)libc.o $(OBJ)udp_apps.o $(OBJ)httpd.o $(OBJ)echod.o $(OBJ)main.o 

#	$(CROSSCOMPILE)objcopy rts.elf rts.bin -O srec
# srec format above for jtag loading (ie binary format with a short header)
# binary format below for MMC and UART booting
	$(CROSSCOMPILE)objcopy rts.elf app -O binary
	
	$(CROSSCOMPILE)objdump -M reg-names-raw -D rts.elf > rts.lst
#	$(CROSSCOMPILE)objdump -d -S -h -t rts.elf > rts.dmp

clean :
	-@del *.dmp *.lst *.elf
# NB never ever delete any object files from obj/ 
#    it will mess up the rule for making rts.elf
