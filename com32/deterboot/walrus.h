/******************************************************************************
 *
 * Walrus Test Framework
 * Copyright Ryan Goodfellow 2017, all rights reserved
 * GPLv3+
 *
 *****************************************************************************/

#include <netinet/in.h>

struct WTFTest
{
  const char *collector;
  const char *test;
  const char *participant;
  int counter;
};

int __WTFxxx(struct WTFTest *t, const char *level, const char *fmt, ...);

#define WTFerror(__t__, __fmt__, ...) \
  __WTFxxx(__t__, "error", __fmt__, ##__VA_ARGS__)

#define WTFwarning(__t__, __fmt__, ...) \
  __WTFxxx(__t__, "warning", __fmt__, ##__VA_ARGS__)

#define WTFok(__t__, __fmt__, ...) \
  __WTFxxx(__t__, "ok", __fmt__, ##__VA_ARGS__)

static const int 
  REDIS_PORT = 6379,
  SUCCESS = 0,
  FAILURE = 1;
