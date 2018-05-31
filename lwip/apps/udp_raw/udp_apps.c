//
// UDP application functions
//
#include <string.h>
#include "bbb_main.h"
#include "lwip/tcp.h"
#include "echod.h"
#include "udp.h"
#include "ipv4/lwip/ip_addr.h"
#include "mmc_api.h"
#include "con_uif.h"
#include "udp_apps.h"
#include "ff.h"

FIL udp_fo  __attribute__ ((aligned (SOC_CACHELINE_SIZE)));  // UDP file object
FRESULT fr_udp_log = FR_NOT_READY;
static char udp_buf[DATA_BUF_SIZE] __attribute__ ((aligned (SOC_CACHELINE_SIZE)));
u16_t pkt_len;

static struct udp_pcb *pcb;  // UDP protocol control block struct
static ip_addr_t ip_d, ip_s, *ip_dest=&ip_d, *ip_src=&ip_s;
static u32_t udplogflag=0;
u32_t udp_rx_count=0;

//
//  UDP transmit ASCII data entered on the console
//
//  udptx - Console Command
//
//  argc  the argument count.  Must be 2
//  argv  char* array of tokens
//
//  This routine will transmit the char string stored 
//  at argv[1] via UDP with checksum
//
int Cmd_udptx(int argc, char *argv[]) {
  u32_t tot_len;
  err_t err;
  struct pbuf *p;

  tot_len = strlen(argv[1]);
  if((argc < 2) || !tot_len) {
    ConsolePrintf("No data to send\r\n");
    return 0;
  }
  if(tot_len > MAX_SIZE_UDP) tot_len = MAX_SIZE_UDP;
  p = pbuf_alloc(PBUF_RAW, tot_len, PBUF_RAM);
  strcpy(p->payload, argv[1]);
  p->tot_len = tot_len;
  p->len = tot_len;
  p->type = PBUF_RAM;
  ConsolePrintf("Tx payload >%s< len=%d\r\n", p->payload, p->tot_len);
  err = udp_send(pcb, p);
  if (err != ERR_OK) {
    ConsolePrintf("udp_write error: %s\r\n", lwip_strerr(err));
  }
  pbuf_free(p);  // release packet buffer

  return 0;
}

//
//  UDP Log Utility
//
//  udplog - Console Command
//
//  argc  the argument count
//  argv  char* array of tokens  
//        first token is "udplog" string
//        second token is filename of logfile
//
//  This routine will activate logging received UDP packets 
//  into file specified by token argv[1].
//  The file will be created if it does not exist.
//  The file will be overwritten if it does exist.
//  The file is in directory /udplogs     mkdir this directory!
//  Use standard DOS filename convention ABCDEFGH.EXT
//  Stop logging and close logfile with udplog0
//
int Cmd_udplog(int argc, char *argv[]) {
  char namebuf[PATH_BUF_SIZE] = "/udplogs/";

  if(udplogflag) return 0;
  strcat(namebuf, argv[1]);
  fr_udp_log = f_open(&udp_fo, namebuf, FA_WRITE | FA_CREATE_ALWAYS);
  if(fr_udp_log == FR_OK) udplogflag = 1;
  else udplogflag = 0;

  return 0;
}

//
//  UDP Log Stop Utility
//
//  udplog0 - Console Command
//
//  Stop logging and close logfile
//
int Cmd_udplog0(int argc, char *argv[]) {

  if(udplogflag) {
    f_close(&udp_fo);
    udplogflag = 0;
  }
  return 0;
}

//
// callback function - a UDP datagram has been received on this pcb
//
// run in IRQ mode
//
void udp_incoming(void *arg, struct udp_pcb *pcb, struct pbuf *p, 
                  struct ip_addr *addr, u16_t port) {
//  u32_t arg_val, *arg_p;

//  arg_p = (u32_t *)arg;  // arg is from udp_app_init()  now it does nothing
//  arg_val = *arg_p;      // but it is useful for parameter passing
//  *arg_p = ++arg_val;

  pkt_len = p->tot_len;
  strncpy(udp_buf, p->payload, pkt_len);  // copy payload
  udp_buf[pkt_len] = 0x0;                 // terminate payload buffer
  pbuf_free(p);                           // release packet buffer
  udp_rx_count++;

  return;
}

//
// process received UDP packet
//
// run in SYS mode, not IRQ mode 
// do not call from an ISR!  f_write() causes an MMC IRQ
//
void udp_rx_process() {
  u16_t bw=0;

  ConsolePrintf("packet#%d contents>%s<%d bytes\r\n", udp_rx_count, udp_buf, pkt_len);
  if(udplogflag) {                 // write packet to mmc logfile
    udp_buf[pkt_len] = '\n';       // terminate payload buffer with newline
    fr_udp_log = f_write(&udp_fo, udp_buf, pkt_len+1, &bw);
    if (fr_udp_log || bw < pkt_len) ConsolePrintf("log write error\r\n");  // error or MMC full
    fr_udp_log = f_sync(&udp_fo);  // flush MMC cache
    if (fr_udp_log) ConsolePrintf("log sync error\r\n");
  }

  return;
}

//
// set up UDP PCB and bind/connect it to defined port
//
void udp_app_init(u32_t master) {
  static u32_t recv_arg=0x0;    // static dec impt here   persistent var
  void *recv_arg_p=&recv_arg;

  ip_d.addr = BONE1;
  ip_s.addr = BONE2;
  pcb = udp_new();
  if (pcb == NULL) {
    ConsolePrintf("Cannot obtain pcb\r\n");
    return;
  }
  if(master) {                                        // listen UDP
    udp_recv(pcb, udp_incoming, (void *)recv_arg_p);  // register callback
    udp_bind(pcb, ip_dest, PORT_DEST);                // local ip addr/port
  } else {                                            // transmit UDP
    udp_bind(pcb, ip_src, PORT_DEST);                 // local ip addr/port
    udp_connect(pcb, ip_dest, PORT_DEST);             // remote ip addr/port
  }
  return;
}

// eof
