#
# makefile for Trima v5.0 Install Software
# $Header: I:/BCT_Development/Install/Trima_v5/source/rcs/makefile 1.1 2001/08/17 15:34:17 ms10234 Exp $
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

# converts config.dat files from 5.0 to 4.0
DOWNGRADE50CONFIG=conv_config_50_to_40
DOWNGRADE50CONFIGSRC=cfgconv.cpp 
DOWNGRADE50CONFIGOBJ=${DOWNGRADE50CONFIGSRC:.cpp=.o}

SRCS=${DOWNGRADE50CONFIGSRC}
OBJS=${SRCS:.cpp=.o}

PROGRAMS=${DOWNGRADE50CONFIG} diff awk textto
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
        
$(DOWNGRADE50CONFIG):$(DOWNGRADE50CONFIGOBJ)
        $(LD) $(LDFLAGS) -l $(LIBDIR)/$(CFGCONVERTLIB) -o $(DOWNGRADE50CONFIG) $(DOWNGRADE50CONFIGOBJ)

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

