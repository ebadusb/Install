#!/bin/sh
# makes directories s/t lower level make cleanexes & installs wont fail
INSTALLDIRS="source/lib source/common"

for i in $INSTALLDIRS; do
	if [ ! -d $i ]; then
		echo making $i;
		mkdir $i;
	fi
done
exit 0
