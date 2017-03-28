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

  printf("me: %s\n", inet_ntoa(info->myAddr));
  printf("boss: %s\n", inet_ntoa(info->bossAddr));

  return 0;
}
