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
 * testing.h
 *
 * Very simple testing framework
 */

#include <linux/list.h>
#include <stdbool.h>
#include <stdarg.h>

///
/// Test Constants
///
enum {
  TEST_FATAL   = -1,
  TEST_ERROR   =  1,
  TEST_WARNING =  2,
  TEST_OK      =  0
};

///
/// Test Diagnostic
///

struct TestDiagnostic
{
  int code;
  char *message;
  struct list_head list;
};

struct TestDiagnostic* new_TestDiagnostic(int code, const char *msg);
void free_TestDiagnostic(struct TestDiagnostic *d);

///
/// Test Result
///

struct TestResult
{
  struct TestDiagnostic diagnostics;
  bool fatal;  
  struct list_head list;
};

struct TestResult* new_TestResult(void);
void free_TestResult(struct TestResult *t);

///
/// Test Run
///

struct TestRun
{
  struct TestResult results;
  void (*dump)(struct TestRun *r);
};

struct TestRun* new_TestRun(void);
struct TestResult* processResult(struct TestRun *tr, struct TestResult *r);
void free_TestRun(struct TestRun *t);


void testOK(void);

void __testX(struct TestResult *t, int code, const char *fmt, ...);

#define testFatal(__t__, __fmt__, ...) \
  __testX(__t__, TEST_FATAL, __fmt__, ##__VA_ARGS__)

#define testError(__t__, __fmt__, ...) \
  __testX(__t__, TEST_ERROR, __fmt__, ##__VA_ARGS__)

#define testWarning(__t__, __fmt__, ...) \
  __testX(__t__, TEST_WARNING, __fmt__, ##__VA_ARGS__)

#define testOK(__t__, __fmt__, ...) \
  __testX(__t__, TEST_OK, __fmt__, ##__VA_ARGS__)

