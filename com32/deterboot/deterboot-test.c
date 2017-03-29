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

struct TestResult* test_getNetInfo(void);
struct TestResult* test_muffinTest(void);
struct TestResult* test_bootWhat(void);
  
struct NetInfo netinfo;

int main(void)
{
  lwip_socket_init();

  struct TestRun *tr = new_TestRun();

  processResult(tr, test_getNetInfo());
  //processResult(tr, test_muffinTest());
  processResult(tr, test_bootWhat());
  
  tr->dump(tr);
}

struct TestResult* test_getNetInfo(void)
{
  struct TestResult *r = new_TestResult();

  int err = getNetInfo(&netinfo);
  if(err) 
  {
    testFatal(r, "getNetInfo failed %d", err);
  }

  u32_t bossExpected = inet_addr("192.168.33.1"); 
  if(bossExpected != netinfo.bossAddr.s_addr)
  {
    testError(r, "unexpected boss address %s", inet_ntoa(netinfo.bossAddr));
  }

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

struct TestResult* test_bootWhat(void)
{
  struct TestResult *r = new_TestResult();

  struct boot_info bi_out = {
    .version = BIVERSION_CURRENT,
    .opcode = BIOPCODE_BOOTWHAT_REQUEST,
    .status = 0
  };
  memset(&bi_out.data, 0, MAX_BOOT_DATA);

  struct boot_info bi_in;
  memset(&bi_in, 0, sizeof(struct boot_info));

  struct Question question = {
    .who = netinfo.bossAddr,
    .port = 6969,
    .response_port = 9696,
    .what = &bi_out,
    .what_size = sizeof(struct boot_info),
    .me = netinfo.myAddr,
    .response = &bi_in,
    .response_size = sizeof(struct boot_info)
  };

  int err = ask(&question);
  if(err != QUESTION_OK)
  {
    testFatal(r, "boot-what comms failure %d", err);
  }
  
  if(bi_in.opcode != BIOPCODE_BOOTWHAT_REPLY)
  {
    testError("unexpected opcode: %d\n", bi_in.opcode);
    return r;
  }

  struct boot_what *bw = (struct boot_what*)&bi_in.data;
  
  if(bw->type != BIBOOTWHAT_TYPE_WAIT)
  {
    testError("unexpected bootwhat type: %d\n", bw->type);
  }


  testOK(r, "boot-what test finished");
  return r;
}
