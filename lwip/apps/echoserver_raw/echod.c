/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *         Simon Goldschmidt
 *
 *
 * \file       echoD.c
 *
 * \brief     It is an echoserver which echo's back data sent from client.
 */
#include "lwip/tcp.h"
#include "echod.h"
#include <string.h>

#define ECHO_SERVER_PORT      2000
#define ECHO_POLL_INTERVAL    4               // poll delay = X*500ms
#define ECHO_TCP_PRIO         TCP_PRIO_MAX    // priority for tcp pcbs
#define MAX_SIZE              4096

unsigned char mydata[MAX_SIZE];  // copy buffer for echoing data back to sender

//
// connections are normally closed
// reset recv and sent callbacks
//
void echo_close_conn(struct tcp_pcb *pcb) {

  tcp_recv(pcb, NULL);
  tcp_close(pcb);
  tcp_arg(pcb, NULL);
  tcp_sent(pcb, NULL);
}

//
// copy received data into local buffer then free corresponding pbuf 
// and echo local buffer back to client
// local buffer size is capped at 1460 assuming MTU limits  << NOT
//
err_t echo_send_data(struct tcp_pcb *pcb, struct pbuf *p) {
  err_t err;
  char *data;
  unsigned int cnt = 0, j, i;
  unsigned int len, tot_len;
  struct pbuf *temp = p;

  tot_len = p->tot_len;

  // traverse pbuf chain and store payload of each pbuf into buffer 
  do {
    data = (char*)p->payload;
    len  = p->len;
    for(i = 0,j = 0; i < len; i++,j++,cnt++) {
      mydata[cnt] = data[j];
    }
    p = p->next;
  } while(p!=NULL);

  pbuf_free(temp);  // free pbufs    

  // send data buffer over network with tcp header attached
  err = tcp_write(pcb, mydata, tot_len , TCP_WRITE_FLAG_COPY);

  return err;
}

//
// callback function - data has been received on this pcb
//
err_t echo_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
  err_t err_send;

  if (p != NULL) {
    tcp_recved(pcb, p->tot_len);  // inform TCP that data was taken
  }
  
  if ((err != ERR_OK) || (p == NULL)){
    if (p != NULL) {  // error or closed by other side
      pbuf_free(p);
    }
    echo_close_conn(pcb);
    return ERR_OK;
  }
  err_send = echo_send_data(pcb, p);
  return err_send;
}

//
// callback function - a new incoming connection that has been accepted on this pcb
//
err_t echo_accept(void *arg, struct tcp_pcb *pcb, err_t err) {
  LWIP_UNUSED_ARG(err);

  tcp_accepted(((struct tcp_pcb_listen*)arg));  // decrease listen backlog counter
  tcp_setprio(pcb, ECHO_TCP_PRIO);
  // Set up the various callback functions
  tcp_recv(pcb, echo_recv);
  tcp_err(pcb,  NULL);
  tcp_poll(pcb, NULL, ECHO_POLL_INTERVAL);
  tcp_sent(pcb, NULL);

  return ERR_OK;
}

//
// set up listening PCB and bind it to defined port
//
void echo_init(void) {
  struct tcp_pcb *pcb;

  pcb = tcp_new();
  tcp_bind(pcb, IP_ADDR_ANY, ECHO_SERVER_PORT);
  pcb = tcp_listen(pcb);
  // initialize callback arg and accept callback
  tcp_arg(pcb, pcb);
  tcp_accept(pcb, echo_accept);
}

// eof
