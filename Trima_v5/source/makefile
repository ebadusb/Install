#
# makefile for Trima v5.0 Install Software
# $Header: I:/Trima_Project/Source/service/rcs/service.mak 1.23 1999/04/20 14:50:04 TD10216 Exp $
# NB:Do NOT save & try to run this file with DOS CR/LFs
# Set MKS option Configuration->Personal->Checkout->UNIX
# symptom:cant open ../installrules.mk
#

# define include files
INSTALLRULES=../installrules.mk
# pull in install project make environment
include $(INSTALLRULES)

LOCAL_TOP=..
COMDIR=`$(LOCAL_TOP)/getcomdir.sh`
INSTALLSHAREDTARGETS=$(LOCAL_TOP)/installsharedtargets.mk

# this is non-portable
STACKSIZE=64
#include $(LOCAL_TOP)/stacksize.mk

MAKEFILE=makefile
BUILDQNX=/bin/buildqnx

# converts config.dat files from 4.0 to 5.0
UPGRADE40CONFIG=conv_config_40_to_50
UPGRADE40CONFIGSRC=cfgconv.cpp 
UPGRADE40CONFIGOBJ=${UPGRADE40CONFIGSRC:.cpp=.o}

SRCS=$(UPGRADE40CONFIGSRC)
OBJS=${SRCS:.cpp=.o}

PROGRAMS=$(UPGRADE40CONFIG) diff awk textto
all: $(PROGRAMS) boot

boot :  hard.install
        WORKDIR=$$PWD; cd /boot; \
        ${BUILDQNX} $$WORKDIR/hard.install $$WORKDIR/install_boot_image; \
        cd $$WORKDIR; 
        
diff : /bin/diff
        cp -f /bin/diff .
        
awk : /bin/awk
        cp -f /bin/awk .
        
textto : /usr/bin/textto
        cp -f /usr/bin/textto .
        
$(UPGRADE40CONFIG):$(UPGRADE40CONFIGOBJ)
        $(LD) $(LDFLAGS) -l $(LIBDIR)/$(CFGCONVERTLIB) -o $(UPGRADE40CONFIG) $(UPGRADE40CONFIGOBJ)

# import COBE common project stuff
imports:
        installdirs.sh
        @echo importing from $(COMDIR)
# get libraries
        $(CP) $(COMDIR)/lib/cfgconvert* lib
# get include files
        $(CP) $(COMDIR)/convert/*.h* common

# include clean & other targets
include $(INSTALLSHAREDTARGETS)

