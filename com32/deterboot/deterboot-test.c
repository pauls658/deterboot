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

int main(void)
{
  struct TestRun *tr = new_TestRun();

  processResult(tr, test_getNetInfo());
  
  tr->dump(tr);
}

struct TestResult* test_getNetInfo(void)
{
  struct TestResult *r = new_TestResult();

  struct NetInfo netinfo;
  int err = getNetInfo(&netinfo);
  if(err) 
  {
    testFatal(r, "getNetInfo failed %d", err);
  }

  testOK(r, "getNetInfo test passed");

  return r;
}
