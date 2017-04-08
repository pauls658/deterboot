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

struct TestResult* test_getNetInfo(void);
struct TestResult* test_muffinTest(void);
struct TestResult* test_bootWhat_wait(void);
struct TestResult* test_bootWhat_mfs(void);
  
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

  //struct TestRun *tr = new_TestRun();
  
  WTFok(&wtf, "grrrrrr!");
  printf("wtf finished\n");

  //processResult(tr, test_getNetInfo());
  //processResult(tr, test_muffinTest());
  //processResult(tr, test_bootWhat_wait());
  //processResult(tr, test_bootWhat_mfs());
  
  //tr->dump(tr);
}

struct TestResult* test_getNetInfo(void)
{
  struct TestResult *r = new_TestResult();

  int err = getNetInfo(&netinfo);
  if(err) 
  {
    testFatal(r, "getNetInfo failed %d", err);
  }

  u32_t bossExpected = inet_addr("192.168.252.1"); 
  if(bossExpected != netinfo.bossAddr.s_addr)
  {
    testError(r, "unexpected boss address %s", inet_ntoa(netinfo.bossAddr));
  }

  WTFok(&wtf, "getNetInfo test finished");
  testOK(r, "getNetInfo test finished");

  return r;
}

struct TestResult* test_muffinTest(void)
{
  struct TestResult *r = new_TestResult();

  char buf[256];
  char *what = "Do you know the muffin man?";
  struct Question question = {
    .who = netinfo.bossAddr,
    .port = 4747,
    .response_port = 4747,
    .what = what,
    .what_size = strlen(what),
    .me = netinfo.myAddr,
    .response = buf,
    .response_size = 256
  };

  int err = ask(&question);
  if(err != QUESTION_OK)
  {
    testFatal(r, "muffin challenge comms failure %d", err);
  }
  
  char *expectedResponse = "The muffin man is ME!";
  size_t len = strlen(expectedResponse);
  if(strncmp(expectedResponse, question.response, len) != 0)
  {
    testFatal(r, 
        "unexpected muffin challenge response: %.*s", 
        question.in_sz, 
        question.response);
  }

  testOK(r, "muffin challenge finished");
    

  return r;
}

struct TestResult* test_bootWhat_wait(void)
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
    testError(r, "unexpected opcode: %d", br.info.opcode);
    return r;
  }

  if(br.what->type != BIBOOTWHAT_TYPE_WAIT)
  {
    testError(r, "unexpected bootwhat type: %d", br.what->type);
  }
  
  testOK(r, "boot-what wait test finished");
  return r;
}

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
