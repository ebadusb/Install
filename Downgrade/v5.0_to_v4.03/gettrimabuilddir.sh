#!/bin/sh
# ensure a file exists which contains the path to the proper directory
# from which to get the dish build stuff
#

if [ ! -f latest_trima_build_dir.mk ]; then
        cat /bctdev/BCT_Development/TrimaV4/Triam/trima_builds/latest_trima_build_dir.mk; 
else 
        cat latest_trima_build_dir.mk;
fi
exit 0
