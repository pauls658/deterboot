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
 * main.c
 *
 * This module is the Deter stage-0 boot loader entry point
 */

#include <string.h>
#include <stdio.h>
#include <consoles.h>
#include <net.h>
#include <lwip/sockets.h>

#include "deterboot.h"

/*
 * Forward declarations
 */
struct NetInfo netInfo(void);
struct BootWhatResponse bootInfo(const struct NetInfo *netinfo);
void bootMfs(const struct BootWhatResponse *br);
void unknownBootType(const struct BootWhatResponse *br);

#define die(__msg__, ...) \
  printf(__msg__, ##__VA_ARGS__); \
  exit(EXIT_FAILURE);


int main(void)
{
  console_ansi_std();
  printf("Deter Boot\n");

  struct NetInfo netinfo = netInfo();
  struct BootWhatResponse bootinfo = bootInfo(&netinfo);

  switch(bootinfo.what->type)
  {
    case BIBOOTWHAT_TYPE_MFS: bootMFS(bootinfo.what->what.mfs); break;
    default:  unknownBootType(&bootinfo);
  }


  return EXIT_SUCCESS;
}

//
// get the bootp network info from pxe
//
struct NetInfo netInfo(void)
{
  struct NetInfo netinfo;
  int err = getNetInfo(&netinfo);
  if(err) 
  {
    die("getNetInfo failed %d", err);
  }
  return netinfo;
}

//
// ask boss what to boot
//
struct BootWhatResponse bootInfo(const struct NetInfo *netinfo)
{
  struct BootWhatResponse br;
  int err = bootWhat(netinfo, &br);
  
  if(err != BOOTWHAT_OK)
  {
    die("boot-what comms failure %d\n", err);
  }
  
  if(br.info.opcode != BIOPCODE_REPLY)
  {
    die("unexpected opcode: %d\n", br.info.opcode);
  }
  return br;
}

///
/// boot a linux-mfs
///
void bootMfs(const struct BootWhatResponse *br)
{
  char *mfsArgs = br->what->what.mfs;

  // build syslinux linux bootloader command
  char *fmt = "linux.c32 %s BOOT=live console=tty1 quiet";
  size_t n = snprintf(NULL, 0, fmt, mfsArgs);
  char *cmd = malloc(n+1);
  snprintf(cmd, n+1, fmt, mfsArgs);

  //have syslinux execute the linux bootloader
  syslinux_run_command(cmd);

  free(cmd);
}

///
/// Not sure what we got from bootinfo
///
void unknownBootType(const struct BootWhatResponse *b)
{
  die("unknown bootinfo type: %d\n", b->what->type);
}
