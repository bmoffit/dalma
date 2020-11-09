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
ifdef CODA_LIB

INC_CMSG	?= -I${CODA}/common/include
LIB_CMSG	?= -L${CODA_LIB}

endif

# Safe defaults for installation
LINUXVME_LIB	?= ../lib
LINUXVME_INC	?= ../include
LINUXVME_BIN	?= ../bin

CC			= gcc
AR                      = ar
RANLIB                  = ranlib
INCS			= -I. ${INC_CMSG}
CFLAGS			= -D_GNU_SOURCE
CFLAGS			+= -rdynamic
CFLAGS		       += -L. ${LIB_CMSG}
ifdef DEBUG
	CFLAGS 	       += -Wall -g
else
	CFLAGS 	       += -O2
endif

SRC			= dalma.c
DEPS			= $(SRC:.c=.d)
OBJ			= $(SRC:%.c=%.o)
LIBS			=
PROG			= dalma

all: $(LIBS) $(PROG)

%.o: %.c
	@echo " CC     $@"
	${Q}$(CC) $(CFLAGS) $(INCS) -c -o $@ $<

$(PROG): dalma.c
	@echo " CC     $@"
	${Q}$(CC) $(CFLAGS)  -lrt -ldl -lcmsg -lcmsgRegex $(INCS) -o $@ $<

%.so: $(SRC)
	@echo " CC     $@"
	${Q}$(CC) -fpic -shared $(CFLAGS) -lrt -ldl -lcmsg -lcmsgRegex $(INCS) -o $(@:%.a=%.so) $(SRC)

%.a: $(OBJ)
	@echo " AR     $@"
	${Q}$(AR) ru $@ $<
	@echo " RANLIB $@"
	${Q}$(RANLIB) $@

install: $(LIBS) $(PROG)
	@echo " CP     ${LIBS}"
	${Q}cp ${LIBS} $(LINUXVME_LIB)/
	@echo " CP     ${PROG}"
	${Q}cp ${PROG} $(LINUXVME_BIN)/

coda_install: $(LIBS) $(PROG)
	@echo " CODACP ${LIBS}"
	${Q}cp ${LIBS} $(CODA_LIB)/
	@echo " CODACP ${PROG}"
	${Q}cp ${PROG} $(CODA_BIN)/

%.d: %.c
	@echo " DEP    $@"
	@set -e; rm -f $@; \
	$(CC) -MM -shared $(INCS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(DEPS)

clean:
	@rm -vf ${PROG} *.{o,a,so} *~

.PHONY: clean
