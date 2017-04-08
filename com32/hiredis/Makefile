## -----------------------------------------------------------------------
##
## Copyright (C) 2010-2011 Salvatore Sanfilippo <antirez at gmail dot com>
## Copyright (C) 2010-2011 Pieter Noordhuis <pcnoordhuis at gmail dot com>
## Copyright (C) 2017 Ryan Goodfellow <rgoodfel at isi dot edu>
## This file is released under the BSD license, see the COPYING file
##
## -----------------------------------------------------------------------

##
## hiredis module
##

VPATH = $(SRC)
include $(MAKEDIR)/elf.mk

#LNXLIBS	   = $(objdir)/com32/libutil/libutil_lnx.a

CFLAGS += -I$(topdir)/core
CFLAGS += -I$(topdir)/core/lwip/src/include
CFLAGS += -I$(topdir)/core/lwip/src/include/ipv4

LIBOBJS = net.o hiredis.o sds.o read.o

all: libhiredis.c32

net.o: net.c fmacros.h net.h hiredis.h read.h sds.h
hiredis.o: hiredis.c fmacros.h hiredis.h read.h sds.h net.h
sds.o: sds.c sds.h
#async.o: async.c fmacros.h async.h hiredis.h read.h sds.h net.h dict.c dict.h
read.o: read.c fmacros.h read.h sds.h

libhiredis.elf : $(LIBOBJS)
	$(LD) $(LDFLAGS) -soname $(patsubst %.elf,%.c32,$(@F)) -o $@ $^

tidy dist:
	rm -f *.o *.lo *.a *.lst *.elf .*.d *.tmp

clean: tidy
	rm -f *.lnx

spotless: clean
	rm -f *.lss *.c32 *.com
	rm -f *~ \#*

install:

-include .*.d
