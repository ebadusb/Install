#
# $Header: K:/BCT_Development/AmproBiosUpdate/rcs/makefile 1.2 2003/11/05 18:01:11Z jl11312 Exp $
# $Log: makefile $
# Revision 1.2  2003/11/05 18:01:11Z  jl11312
# - updated to support safety BIOS update
# Revision 1.1  2003/10/08 16:41:01Z  jl11312
# Initial revision
#

all : ampro_cbios_update ampro_sbios_update

ampro_cbios_update: ampro_cbios_update.cpp ampro_support.cpp flash_support.cpp control_image.cpp
	cc -T0 -o ampro_cbios_update ampro_cbios_update.cpp

ampro_sbios_update: ampro_sbios_update.cpp ampro_support.cpp flash_support.cpp safety_image.cpp
	cc -T0 -o ampro_sbios_update ampro_sbios_update.cpp
 
