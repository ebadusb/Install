#!/bin/sh
# ensure a file exists which contains the path to the proper directory
# from which to get the COBE common stuff
#

if [ ! -f latest_common_build_dir.mk ]; then
        cat /bctdev/BCT_Development/Common/common_builds/latest_common_build_dir.mk; 
else 
        cat latest_common_build_dir.mk;
fi
exit 0
