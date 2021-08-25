#
# File:
#    Makefile
#
# Description:
#    Makefile for dalmaRol test programs
#

ARCH=${shell uname -m}
KERNEL_VERSION=${shell uname -r}
CC			= gcc
AR                      = ar
RANLIB                  = ranlib
CFLAGS			= -Wall -Wno-unused -g -I. -I../ \
			  -L. -L../
CFLAGS			+= -ldalmaRol
SRC			= $(wildcard *.c)
PROGS			= $(SRC:.c=)


all: $(PROGS)

clean distclean:
	@rm -f $(PROGS) *~

%: %.c
	echo "Making $@"
	$(CC) $(CFLAGS) -o $@ $(@:%=%.c) -lrt

.PHONY: all clean distclean
