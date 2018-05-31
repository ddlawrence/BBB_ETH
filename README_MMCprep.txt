Preparing a Bootable MMC

Follow procedure in pdf:  AM335X StarterWare Booting And Flashing

Windows Instructions for BOOTABLE MMC formatting:
MUST format with:  HP USB Disk Storage Format Tool v2.0.6 
Other formatters (from SDcard Corp) do not work.  Possibly because 
their default sector size is not 4096.  This is an uncertain area.  

Linux Instructions for BOOTABLE MMC formatting:
NB: for bbb boot-ability: heads=255, sects=63 & FAT32 bootable partition 1
# parted /dev/sda1 
    print, ... , help
    align-check 1
    set 1 boot on
    unit chs print  (cyls, heads, sects)
    quit
# mkfs.vfat -n <vol_name> -F 32 /dev/sda1
then remove media & reinsert.  it should auto mount.
=====================

After compiling/linking your application don't forget to run: 

    $(CROSSCOMPILE)objcopy rts.elf app -O binary

This reformats the image for UART or MMC booting.  It is in Makefile.  
=====================

For MMC booting you must prepend the image with a small header 
to indicate load address and size with the ti image program:  
tiimage 0x80000000 NONE app my_app

Rename "my_app" to "app"
Then copy the files named "app" and "MLO" to the MMCard root directory.  

The MLO bootloader must be the version for MMC booting, not the UART 
version.  Both versions are provided pre-built.  Do not monkey with them.  
=====================

If you are modding/rebuilding the MLO, the bootloader must be 
prepended with a header also:
tiimage 0x402F0400 MMCSD boot.bin MLO
=====================

Blurb from AM335X StarterWare Booting And Flashing p8:
Stage 1: On reset, the ROM code loads the MLO bootloader by reading the 
file "MLO" from the MMCard and copying it to the address embedded 
on its header, 0x402F0400, then execution jumps to that address.  
Stage 2: The MLO bootloader loads the application by reading the file 
named "app" from the MMCard and copying it to the address embedded on 
its header, 0x80000000.  Then execution jumps to that address.  
Stage 3: The application executes.

This is the fast-boot procedure as I understand it.
