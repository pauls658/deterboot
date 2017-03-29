/* -------------------------------------------------------------------------- *
 *
 *   Copyright 2017 Deter-Project - All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 53 Temple Place Ste 330,
 *   Boston MA 02111-1307, USA; either version 2 of the License, or
 *   (at your option) any later version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */


/*
 * deterboot.c
 *
 * This module implements the Deter stage-0 boot loader functionality
 */

#include <syslinux/pxe.h>
#include <dhcp.h>
#include <stdio.h>
#include <lwip/sockets.h>
#include "deterboot.h"

int getNetInfo(struct NetInfo *info)
{
  void *dhcpdata = NULL;
  size_t dhcplen = 0;
  int err = pxe_get_cached_info(
      PXENV_PACKET_TYPE_DHCP_ACK, 
      &dhcpdata, 
      &dhcplen);

  if(err) 
  {
    printf("Failed to get network information\n");
    return err;
  }

  struct dhcp_packet *pkt = (struct dhcp_packet*)dhcpdata;
  info->myAddr.s_addr = pkt->yiaddr;
  info->bossAddr.s_addr = pkt->siaddr;

  return 0;
}

size_t ask(struct Question *q)
{
  struct sockaddr_in dest;
  dest.sin_family = AF_INET;
  dest.sin_port = htons(q->port);
  dest.sin_addr = q->who;
  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  struct sockaddr_in reply;
  reply.sin_family = AF_INET;
  reply.sin_port = htons(q->response_port);
  reply.sin_addr = q->me;
  int rsock = socket(AF_INET, SOCK_DGRAM, 0);
  
  int result = QUESTION_OK;

  int err = bind(rsock, (struct sockaddr*)&reply, sizeof(reply));

  q->out_sz = sendto(
      sock, 
      q->what, 
      q->what_size, 
      0, 
      (struct sockaddr*)&dest, 
      sizeof(dest));

  if(q->out_sz < q->what_size)
  {
    result |= QUESTION_PARTIAL_SEND;
  }

  dest.sin_family = AF_INET;
  dest.sin_port = htons(q->port);
  dest.sin_addr = q->who;
  q->in_sz = recvfrom(
      rsock, 
      q->response, 
      q->response_size, 
      0, 
      (struct sockaddr*)&dest, 
      sizeof(dest));

  closesocket(sock);
  closesocket(rsock);

  return result;
}
