#
# Copyright 1999 Gambro BCT
#
# $Header: I:/BCT_Development/Install/Trima_v5/rcs/installsharedtargets.mk 1.1 2001/08/17 15:34:12 ms10234 Exp $
#
# PURPOSE: define shared make targets for COBE projects
#
# CHANGELOG:
# $Log: installsharedtargets.mk $
# Revision 1.1  2001/08/17 15:34:12  ms10234
# Initial revision
# Revision 1.3  2000/05/26 21:27:13  MS10234
# Added dependencies.mk for dependency generation
# Revision 1.2  1999/05/26 15:34:34  TD10216
# IT3950
#

# get compiler options
include $(LOCAL_TOP)/compileropts.mk

# get linker options
include $(LOCAL_TOP)/linkeropts.mk

# use gmake '-' to ignore result of command
clean:
	-$(RM) $(OBJS) $(PROGRAMS) $(MISC)

# NOTE:set INSTALLDIR before including this file
cleanexes:
	-$(RM) $(PROGRAMS)

#
# Dependency script - add new program source files to SRCS
#
WCC=/usr/watcom/10.6/bin/wpp386
WCCFLAGS=-w0 $(DEV_FLAG)
force:;

# generate list of source files for make depend
.sources: force
	@echo generating source list
	@-${RM} .sources
	@for j in ${SRCS}; do \
	    echo $$j >> .sources; \
	done

#
# NOTE: makefile will be placed in MAKEFILE variable
#
depend: .sources
	@echo generating dependencies
	@-${RM} dependencies.mk
	@for i in `cat .sources`; do \
		echo $$i; \
	    $(WCC) -pl ${INCLS} $$i | \
		grep '^#line 1 ' | \
		grep -v '/usr/include' | \
		grep -v '/stl/' | \
		sed -e "s/#line 1 /$$i:/" | \
		sed -e "s/\.c:/\.o:/" | \
		sed -e "s/\.cpp/\.o/" | \
		sed -e "s/\"//g" | \
		sort -u - \
	    >> dependencies.mk; \
	done
#-----------------------------------------------------------------------------------
# Uncomment these lines to put the dependencies at the bottom of the
#  makefile
#-----------------------------------------------------------------------------------
#	@mv $(MAKEFILE) $(MAKEFILE).bak
#	@cat $(MAKEFILE).bak | sed \
#	    -e '/^## dependencies.*this line\.\.\. ##/,$$ d' >> $(MAKEFILE)
#	@echo "## dependencies will be put after this line... ##" >> $(MAKEFILE)
#	@cat dependencies.mk >> $(MAKEFILE)
#	@-${RM} .sources dependencies.mk $(MAKEFILE).bak
#-----------------------------------------------------------------------------------

dependencies.mk: force
        @touch $@

include dependencies.mk

