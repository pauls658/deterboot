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
 * This module implements the Deter stage-0 boot loader
 */

#include <string.h>
#include <stdio.h>
#include <consoles.h>
#include <syslinux/pxe.h>
#include <dhcp.h>
#include <net.h>
#include <netinet/in.h>
#include <lwip/sockets.h>

struct NetInfo
{
  struct in_addr myAddr,
                 bossAddr;
};

int getNetInfo(struct NetInfo *);

int main()
{
  console_ansi_std();
  printf("Deter Boot\n");

  struct NetInfo netinfo;
  int err = getNetInfo(&netinfo);
  if(err) return 1;

  struct sockaddr_in dest;
  dest.sin_family = AF_INET;
  dest.sin_port = htons(4747);
  dest.sin_addr = netinfo.bossAddr;

  printf("preparing muffin man\n");

  lwip_socket_init();
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  char *data = "do you know the muffin man?";
  int n = sendto(
      sock, 
      (void*)data, strlen(data), 
      0, 
      (struct sockaddr*)&dest, sizeof(dest)
    );

  printf("muffin man sent (%d)\n", n);

  printf("awating muffin response\n");
  char buf[128];
  n = recvfrom(
      sock, 
      buf, 
      sizeof(buf), 
      0, 
      (struct sockaddr*)&dest, sizeof(dest)
  );
  printf("muffin response (%d) %.*s\n", n, n, buf);

  

  return 0;
}

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

  printf("me: %s\n", inet_ntoa(info->myAddr));
  printf("boss: %s\n", inet_ntoa(info->bossAddr));

  return 0;
}
