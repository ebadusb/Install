#!/bin/sh
# ensure a file exists which contains the path to the proper directory
# from which to get the dish build stuff
#

if [ ! -f latest_disk_build_dir.mk ]; then
        cat /bctdev/BCT_Development/TrimaV4/DiskBuild/disk_builds/latest_install_build_dir.mk; 
else 
        cat latest_disk_build_dir.mk;
fi
exit 0
