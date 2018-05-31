//
// udp_apps.h  prototypes
//
#include "lwip/tcp.h"

extern u32_t udp_rx_count;

extern void udp_app_init(u32_t master);
extern void udp_rx_process();

#define MAX_SIZE_UDP          256
#define PORT_DEST             0xC000          // 49152  1st dynamic/ephemeral port
#define BONE1                 0xE702A8C0      // 192.168.2.231 master beaglebone
#define BONE2                 0xDF02A8C0      // 192.168.2.223 slave beaglebone
// BONE1 & BONE2 IP addresses assigned by router - yours will differ
// the master listens for packets and the slave transmits packets
// lwIP stores IPs little-endian
