#
# Trima project standard make rules and variables
# $Header: I:/BCT_Development/Install/Trima_v5/rcs/installrules.mk 1.1 2001/08/17 15:34:12 ms10234 Exp $
# $Log: installrules.mk $
# Revision 1.1  2001/08/17 15:34:12  ms10234
# Initial revision
# Revision 1.9  2001/06/22 13:33:49  sb07663
# Uncommented development flag IT4891
# Revision 1.8  2000/08/31 18:55:44  sb07663
# removed temp proc lib
# Revision 1.7  2000/04/07 14:50:10  MS10234
# changed the INCLS variable to assign with a +=
# Revision 1.6  2000/04/06 21:52:49  BD10648
# Set the standard include directories for compilation.
# Revision 1.5  2000/04/04 19:32:30  MS10234
# changed include declarations
# Revision 1.4  2000/03/15 17:02:23  MS10234
# Added trima common library
# Revision 1.3  2000/03/07 17:38:03  MS10234
# changed proc lib name
# Revision 1.2  2000/02/28 21:16:47  SB07663
# IT4604: added low level and common directories to include path
# Revision 1.1  2000/02/15 20:55:56  JR08014
# Revision 1.6  1999/10/05 13:40:04  JR08014
# Revision 1.5  1999/06/30 22:09:56  TD10216
# IT3908
# Revision 1.4  1999/06/03 02:43:23  BS04481
# Remove set for PHTK_PATH as it is incorrect for QNX 4.25
# Add to create_trima instead for node24 builds.
# Revision 1.3  1999/05/28 19:33:07  TD10216
# IT3950
# Revision 1.2  1999/05/27 23:34:56  TD10216
# IT3950
#

# set shell for all users
SHELL=/bin/sh

# development or production build flag - null out for production build
DEV_FLAG=

# name of makefile. If it's not makefile, set this variable after
# including this file
MAKEFILE=makefile

# set this to anywhere to find incs and libs, set incls to use it
TOP_DIR=.

# defined as relative to current dir - changed for dirs lower than
# proc or safety...like procview/src
LOCAL_TOP=.

# include flags
INCLS+=$(ID). $(ID)common $(ID)$(SYSINC)

# define trima libraries
LIBDIR=lib
CFGCONVERTLIB=cfgconvert.lib

# 
QNXLIBS= \
	$(Li) photon_s $(Li) beta3r $(Li) photon3r $(Li) phcontrib      \
	$(Li) phexlib $(Li) phrtlib $(Li) Aplib3r $(Li) phexlib3r       \
	$(Li) socket

# clean command
RM=/bin/rm -f

# import command
CP=/bin/cp -f

