# Copyright 1999 Gambro BCT
# 
# $Header: I:/BCT_Development/Common/rcs/compileropts.mk 1.4 1999/08/16 21:32:49 TD10216 Exp $
# PURPOSE: COBE Common compiler options
# CHANGELOG:
# $Log: compileropts.mk $
# Revision 1.4  1999/08/16 21:32:49  TD10216
# IT4002 (back out)
# Revision 1.2  1999/07/16 13:30:52  TD10216
# IT4136
# Revision 1.1  1999/05/25 16:38:44  TD10216
# Initial revision


# for using wpp386/wcc386
# use wpp/wcc386 because router doesnt compile c files correctly
CPP=wpp386
CC=wcc386

# the version of gmake we have does not recognize the ifeq stuff
#ifeq (${CC},"wpp386")
# include directives
ID=-i
SYSINC=/usr/include
# see option descriptions above
CFLAGS=-ei -zp8 -zq -w4 -d2 $(INCLS) $(DEV_FLAG)
# change this when the -zp option above is changed
PACKOPT=P8
#else
# compilation flags
# -zp8=8 byte structure alignment -Q=quiet mode, -c=no executable, 
# -g=debug, -w4=lots of warnings, -d2=debug level, -WC,-ei=enums as ints
# when using cc - this doesnt work in router
#CFLAGS=-Q -zp8 -c -g -w4 -d2 "-WC,-ei" $(INCLS) $(DEV_FLAG)
# change this when the -zp option above is changed
#PACKOPT=P8
# include directive for using cc
#ID=-I
#SYSINC=
#endif

# compilation rules
.SUFFIXES:	.cpp .c
.cpp.o:
	${CPP} ${CFLAGS} ${<}

.c.o:
	$(CC) ${CFLAGS} ${<}
