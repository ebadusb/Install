#
# Copyright 1999 Gambro BCT
# 
# $Header: I:/BCT_Development/Common/rcs/linkeropts.mk 1.5 1999/08/16 21:33:09 TD10216 Exp $
# PURPOSE: linker options for COBE Common projects
# CHANGELOG:
# $Log: linkeropts.mk $
# Revision 1.5  1999/08/16 21:33:09  TD10216
# IT4002 (back out)
# Revision 1.3  1999/06/04 14:36:36  TD10216
# IT3953
# Revision 1.1  1999/05/25 16:38:46  TD10216
# Initial revision

# setup link
# for using wlink
#LD=wlink

# non-portable link flags settable in individual makefiles.
# see stacksize.mk
STACKSIZE=
Stackmacro=
# non-portable set offset directive (tools/corrupt/makefile)
# see offset.mk
OFFSET=
Offsetmacro=

#ifeq (${LD},wlink)
# same flags as for ld below
#MAPOPT=op MAP
#LDFLAGS=op quiet form qnx flat de all op priv=1 op c libp ../lib:/usr/watcom/10.6/usr/lib:/usr/lib op MAP libpath $(LIBDIR) $(Offsetmacro) $(Stackmacro)
#Li=l
#Name= name
#Fd=f
#else
# for using ld (actually cc)
# -T1=create a root-only executable, -g3=debug level -M=create a map
# non-portable set stack size directive (gui.mak,safety.mak)
MAPOPT="-Wl,op map=$@"
LDFLAGS=-T1 -g3 -L$(LIBDIR) $(Offsetmacro) $(Stackmacro)
Fd=
Name= -o 
Li=-l
#endif


# library build command
# -b=no backup file, -c=case sensitive symbols in library
# -n=always create new libray, -q=no banner/copyright
LIB_BLD_CMD=wlib -b -c -n -q
