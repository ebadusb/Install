#
# makefile for Trima v5.0 Install Software
# $Header: //Bctquad3/HOME/BCT_Development/Install/Trima_v5/rcs/makefile 1.3 2001/08/20 15:48:01 ms10234 Exp ms10234 $
# NB:Do NOT save & try to run this file with DOS CR/LFs
# Set MKS option Configuration->Personal->Checkout->UNIX
# symptom:cant open ../installrules.mk
#

MAKEFILE=makefile
COMDIR=`getcomdir.sh`

include installrules.mk

all: source

source: force
        cd source; $(MAKE)
        
clean: force
        cd source; $(MAKE) $@

depend: force
        cd source; $(MAKE) $@

force: ;

# import COBE common project stuff
imports:
        installdirs.sh
        @echo importing from $(COMDIR)
# get libraries
        $(CP) $(COMDIR)/lib/cfgconvert* source/lib
# get include files
        $(CP) $(COMDIR)/convert/*.h* source/common


