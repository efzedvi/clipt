#----- Project dependent macros -----

VERSION = 0.1
INSTALL_ROOT=/usr/local
DEBUG_FLAG = -g
TARGET = clipt
SERVER_OBJS = clipt.o utils.o 
INCLUDES = includes.h utils.h  
MYLIBS = 
MY_INC = 


CC  = gcc
CXX = g++
AR = ar
RANLIB = ranlib
ARCH_INC=
ARCH_LIB = -lnsl
ARCH_DEFS+=-DHAVE_LIBWRAP
ARCH_DEFS+=-DHAVE_SYSLOG_H
ARCH_DEFS+=-DHAVE_FORK
ARCH_DEFS+=-DHAVE_INET_NTOA
ARCH_DEFS+=-DHAVE_UNISTD_H
ARCH_DEFS+=-DHAVE_NETINET_IN_H
ARCH_DEFS+=-DHAVE_ARPA_INET_H
ARCH_DEFS+=-DRETSIGTYPE=void
ARCH_DEFS+=-DSETPGRP_VOID
ARCH_CLEAN = 

#----- General macros -----

INSTALL_DIR=$(INSTALL_ROOT)/sbin
EXTRA_FLAGS = -O3 -DVERSION=\"$(VERSION)\" $(DEBUG_FLAG)  $(ARCH_DEFS) -DPACKAGE=\"$(TARGET)\" 
LIBS = $(MYLIBS) $(ARCH_LIB)
INCLUDE_FLAG = -I. $(MY_INC) $(ARCH_INC)
CFLAGS = -Wall $(EXTRA_FLAGS)
THIS_DIR := `basename $(CURDIR)`


#----- Rules -----

all: $(TARGET)

$(TARGET): $(SERVER_OBJS) 
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

.c.o: $(INCLUDES)
	$(CC) $(CFLAGS) $(INCLUDE_FLAG) -c $(*F).c -o $@

install: all 
	cp $(TARGET) $(INSTALL_DIR)
	cp clipt.sh /etc/init.d/

clean: 
	rm -f core *.o *~ $(TARGET) 

distclean: clean
	rm -f *.tgz tags

tgz: distclean
	@(cd .. ;  \
	tar cvfz $(THIS_DIR)/clipt_$(VERSION)_`date +%y%m%d%H%M`.tgz $(THIS_DIR)/* )
	
tags: 
	rm -f tags
	ctags *.h *.c*


.PHONY: clean distclean tgz tags 

