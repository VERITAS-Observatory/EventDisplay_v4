#!/bin/bash
# written by Nathan Kelley-Hoskins Aug 2013

ISPIPEFILE=`readlink /dev/fd/0` # check to see if input is from terminal, or from a pipe
if [[ "$ISPIPEFILE" =~ ^/dev/pts/[0-9]{1,2} ]] ; then # its a terminal (not a pipe)
	if ! [ $# -eq 1 ] ; then # the human didn't add any arguments, and we must tell them so
		echo "`basename $0` needs a runlist file"
		echo "`basename $0` <filename>"
		exit
	fi
fi

# see if we can find whichRunsAreNotOnDisk.sh, which this script needs to run
LOC=`command -v whichRunsAreNotOnDisk.sh`
if [ ! -n "$LOC" ] ; then 
	echo "Unable to run `basename $0` ..."
	echo "\$EVNDISPSYS/scripts/VTS/whichRunsAreNotOnDisk.sh needs to be in your \$PATH in order to use this script."
	echo "exiting..."
	exit
fi

cat $1 | whichRunsAreNotOnDisk.sh | wc -l
exit

