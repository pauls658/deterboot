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
 * testing.c
 *
 * Very simple testing framework
 */

#include <string.h>
#include <stdio.h>
#include "testing.h"


///
/// Internal function forward declarations
///
void __testrunShutdown(struct TestRun *r);
void __testrunDump(struct TestRun *r);


/// ===========================================================================
//
/// API Implementation
//
/// ===========================================================================


//
// Test Diagnostics
// ----------------------------------------------------------------------------

struct TestDiagnostic* 
new_TestDiagnostic(int code, const char *msg)
{
  struct TestDiagnostic *d = malloc(sizeof(struct TestDiagnostic));
  d->code = code;
  if(msg == NULL)
  {
    d->message = NULL;
  }
  else
  {
    d->message = malloc(strlen(msg)+1);
    if(d->message == NULL)
    {
      printf("[TESTING ERROR]: Test Diagnostic memory allocation failed\n");
      return NULL;
    }
    strcpy(d->message, msg);
  }

  return d;
}

void free_TestDiagnostic(struct TestDiagnostic *d)
{
  free(d->message);
  free(d);
}


//
// Test Result
// ----------------------------------------------------------------------------

struct TestResult* new_TestResult(void)
{
  struct TestResult *t = malloc(sizeof(struct TestResult));
  INIT_LIST_HEAD(&t->diagnostics.list);
  t->fatal = false;
  return t;
}

void free_TestResult(struct TestResult *t)
{
  while(!list_empty(&t->diagnostics.list))
  {
    struct list_head *h = t->diagnostics.list.next;
    struct TestDiagnostic *d = list_entry(h, struct TestDiagnostic, list);
    list_del(h);
    free_TestDiagnostic(d);
  }
  free_TestResult(t);
}

//
// Test Run
// ----------------------------------------------------------------------------

struct TestRun* new_TestRun(void)
{
  struct TestRun *t = malloc(sizeof(struct TestRun));
  t->dump = __testrunDump;
  INIT_LIST_HEAD(&t->results.list);
  return t;
}

struct TestResult *processResult(struct TestRun *tr, struct TestResult *r)
{
  list_add_tail(&r->list, &tr->results.list);
  if(r->fatal)
  {
    __testrunShutdown(tr);
  }

  return r;
}

void free_TestRun(struct TestRun *t)
{
  while(!list_empty(&t->results.list))
  {
    struct list_head *n = t->results.list.next;
    struct TestResult *r = list_entry(&n, struct TestResult, list);
    list_del(n);
    free_TestResult(r);
  }
  free_TestRun(t);
}


/// ===========================================================================
///
/// Internal functions
///
/// ===========================================================================

void __testX(struct TestResult *t, int code, const char *fmt, ...)
{
  va_list vl1, vl2;
  va_start(vl1, fmt);
  va_copy(vl2, vl1);

  size_t sz = vsnprintf(NULL, 0, fmt, vl1);
  va_end(vl1);
  char *buf = malloc(sz+1);
  vsnprintf(buf, sz+1, fmt, vl2);
  va_end(vl2);
  buf[sz] = 0;

  
  struct TestDiagnostic *td = new_TestDiagnostic(code, buf);
  list_add_tail(&td->list, &t->diagnostics.list);

  free(buf);
}

void __testrunShutdown(struct TestRun *r)
{
  r->dump(r);
  exit(1);
}

void __testrunDump(struct TestRun *r)
{
  struct TestResult *result;
  list_for_each_entry(result, &(r->results.list), list)
  {
    struct TestDiagnostic *diagnostic;
    list_for_each_entry(diagnostic, &(result->diagnostics.list), list)
    {
      char *level = "";
      switch(diagnostic->code)
      {
        case -1: level = "FATAL"; break;
        case  1: level = "ERROR"; break;
        case  2: level = "WARNING"; break;
        case  0: level = "OK"; break;
        default: level = "?";
      }
      printf("[%s]  %s\n", level, diagnostic->message);

    }
  }
}

