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
 * deterboot-test.c
 *
 * This module tests the Deter stage-0 boot loader code
 */

#include <string.h>
#include <stdio.h>
#include <consoles.h>
#include <syslinux/pxe.h>
#include <dhcp.h>
#include <net.h>
#include <netinet/in.h>
#include <lwip/sockets.h>

#include "deterboot.h"
#include "testing.h"
#include "walrus.h"

void testk(int result);

int test_getNetInfo(void);
int test_bootWhat_mfs(void);

//struct TestResult* test_bootWhat_mfs(void);
  
struct NetInfo netinfo;

struct WTFTest wtf = {
  .collector = "192.168.1.247", 
  .test = "bootwait", 
  .participant = "deterboot",
  .counter = 0
};

int main(void)
{
  lwip_socket_init();

  WTFok(&wtf, "starting boot0 test");

  testk(test_getNetInfo());
  testk(test_bootWhat_mfs());

  printf("boot0 test finished\n");
}

void testk(int result)
{
  if(result != TEST_OK)
  {
    exit(1);
  }
}

int test_getNetInfo(void)
{
  int err = getNetInfo(&netinfo);
  if(err) 
  {
    WTFerror(&wtf, "getNetInfo failed %d", err);
    return TEST_ERROR;
  }

  u32_t bossExpected = inet_addr("192.168.252.1"); 
  if(bossExpected != netinfo.bossAddr.s_addr)
  {
    WTFerror(&wtf, "unexpected boss address %s", inet_ntoa(netinfo.bossAddr));
    return TEST_ERROR;
  }

  WTFok(&wtf, "getNetInfo test finished");
  return TEST_OK;
}

int test_bootWhat_mfs(void)
{
  struct BootWhatResponse br;
  int err = bootWhat(&netinfo, &br);
  
  if(err != BOOTWHAT_OK)
  {
    WTFerror(&wtf, "boot-what comms failure %d", err);
    return TEST_ERROR;
  }
  
  if(br.info.opcode != BIOPCODE_REPLY)
  {
    WTFerror(&wtf, "unexpected opcode: %d", br.info.opcode);
    return TEST_ERROR;
  }

  if(br.what->type != BIBOOTWHAT_TYPE_MFS)
  {
    WTFerror(&wtf, "unexpected bootwhat type: %d", br.what->type);
    return TEST_ERROR;
  }

  if(strcmp(br.what->what.mfs, "http://192.168.252.1/linux-mfs") != 0)
  {
    WTFerror(&wtf, "unexpected mfs path: %s\n", br.what->what.mfs);
    return TEST_ERROR;
  }
  
  WTFok(&wtf, "boot-what wait test finished - jumping into mfs now ...");

  err = bootMFS(br.what->what.mfs);

  if(err)
  {
    WTFerror(&wtf, "booting mfs failed %d", err);
  }
  
  return TEST_OK;
}

#if 0
struct TestResult* test_bootWhat_mfs(void)
{
  struct TestResult *r = new_TestResult();

  struct BootWhatResponse br;
  int err = bootWhat(&netinfo, &br);
  
  if(err != BOOTWHAT_OK)
  {
    testFatal(r, "boot-what comms failure %d", err);
    return r;
  }
  
  if(br.info.opcode != BIOPCODE_REPLY)
  {
    testError(r, "unexpected opcode: %d\n", br.info.opcode);
    return r;
  }

  if(br.what->type != BIBOOTWHAT_TYPE_MFS)
  {
    testError(r, "unexpected bootwhat type: %d\n", br.what->type);
    return r;
  }

  printf("load mfs: %s\n", br.what->what.mfs);

  /*
  if(strcmp(br.what->what.mfs, "http://192.168.33.1/images/linux-mfs") != 0)
  {
    testError(r, "unexpected mfs path: %s\n", br.what->what.mfs);
    return r;
  }

  const int sz = 1024;
  void *buf = malloc(sz);
  err = loadMFS(br.what->what.mfs, &buf, sz);

  if(err != LOADMFS_OK)
  {
    testError(r, "error loading mfs: %d", err);
  }

  char *mfs_data = buf;
  printf("mfs data: %s\n", mfs_data);
  
  testError(r, "boot-what MFS failed to load MFS");
  */
  return r;
}
#endif
