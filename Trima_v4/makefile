#
# makefile for Trima v5.0 Install Software
# $Header: I:/BCT_Development/Install/Trima_v5/rcs/makefile 1.4 2001/08/20 15:52:17 ms10234 Exp $
# NB:Do NOT save & try to run this file with DOS CR/LFs
# Set MKS option Configuration->Personal->Checkout->UNIX
# symptom:cant open ../installrules.mk
#

MAKEFILE=makefile
COMDIR=`getcomdir.sh`

include installrules.mk

all: _source

_source: force 
        cd source; $(MAKE) all
        
clean: force
        cd source; $(MAKE) $@

depend: force
        cd source; $(MAKE) $@

force:;

create_dependencies.mk: force
        touch source/dependencies.mk

dependencies.mk: force
        cd source; $(MAKE) $@

# import COBE common project stuff
imports: create_dependencies.mk
        installdirs.sh
        @echo importing from $(COMDIR)
# get libraries
        $(CP) $(COMDIR)/lib/cfgconvert* source/lib
# get include files
        $(CP) $(COMDIR)/convert/*.h* source/common


