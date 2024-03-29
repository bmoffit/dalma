###############################################################################
#  Copyright (c) 2020        Southeastern Universities Research Association,
#                            Thomas Jefferson National Accelerator Facility
#
#    This software was developed under a United States Government license
#    described in the NOTICE file included as part of this distribution.
#
#    Author:  Bryan Moffit
#             moffit@jlab.org                   Jefferson Lab, MS-12B3
#             Phone: (757) 269-5660             12000 Jefferson Ave.
#             Fax:   (757) 269-5800             Newport News, VA 23606
#
###############################################################################
#
# Description:
#    Makefile for the dalogMsg archiver (dalma)
#
###############################################################################

# Uncomment DEBUG line, to include some debugging info ( -g and -Wall)
DEBUG	?= 1
QUIET	?= 1
#
ifeq ($(QUIET),1)
        Q = @
else
        Q =
endif

ARCH	?= $(shell uname -m)

# Check for CODA 3 environment
ifndef CODA_LIB
	CODA 		= /daqfs/coda/3.10_devel
	CODA_LIB	= ${CODA}/Linux-x86_64/lib
endif

INC_CMSG	?= -I${CODA}/common/include
LIB_CMSG	?= -L${CODA_LIB}


# Safe defaults for installation
LINUXVME_LIB	?= ../lib
LINUXVME_INC	?= ../include
LINUXVME_BIN	?= ../bin

CC			= gcc
AR                      = ar
RANLIB                  = ranlib
INCS			= -I. ${INC_CMSG}
LIBINCS			= -I.
CFLAGS			= -D_GNU_SOURCE
CFLAGS			+= -rdynamic
CFLAGS		       += -L. ${LIB_CMSG}
LIBCFLAGS	        = ${CFLAGS} -L.
ifdef DEBUG
	CFLAGS 	       += -Wall -g
else
	CFLAGS 	       += -O2
endif

SRC			= dalma.c
DEPS			= $(SRC:.c=.d)
PROG			= dalma
LIBSRC			= dalmaRolLib.c
LIBHDR			= dalmaRolLib.h
LIB			= libdalmaRol.so


all: $(PROG) $(LIB)

$(PROG): dalma.c
	@echo " CC     $@"
	${Q}$(CC) $(CFLAGS)  -lrt -ldl -lcmsg -lcmsgRegex $(INCS) -o $@ $<

%.so: $(LIBSRC)
	@echo " CC     $@"
	${Q}$(CC) -fpic -shared $(LIBCFLAGS) $(LIBINCS) -o $@ $(LIBSRC)

%.d: %.c
	@echo " DEP    $@"
	@set -e; rm -f $@; \
	$(CC) -MM -shared $(INCS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1 $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(DEPS)

clean:
	@rm -vf ${PROG} *.{d,o,a,so} *~

.PHONY: clean
