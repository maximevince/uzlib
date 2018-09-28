##
## tgunzip  -  gzip decompressor example
##
## GCC makefile (Linux, FreeBSD, BeOS and QNX)
##
## Copyright (c) 2003 by Joergen Ibsen / Jibz
## All Rights Reserved
##
## http://www.ibsensoftware.com/
##

target  = uzf
objects = uzf.o

COPT    = -O0 -g
COPT    += -DNO_DICT=1 -DNO_CB=1
CFLAGS  = -Wall -I../../src $(COPT)
LDFLAGS = $(CFLAGS) -s -Wl,-Map,ld.map

.PHONY: all clean

all: $(target)

$(target): $(objects) $(libs)
	$(CC) $(LDFLAGS) -o $@ $^ $(libs)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) $(objects) $(target)
