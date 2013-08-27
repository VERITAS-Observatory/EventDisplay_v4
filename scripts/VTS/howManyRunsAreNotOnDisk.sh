#!/bin/bash
# written by Nathan Kelley-Hoskins Aug 2013

ISPIPEFILE=`readlink /dev/fd/0` # check to see if input is from terminal, or from a pipe
if [[ "$ISPIPEFILE" =~ ^/dev/pts/[0-9]{1,2} ]] ; then # its a terminal (not a pipe)
	if ! [ $# -eq 1 ] ; then # the human didn't add any arguments, and we must tell them so
		echo "`basename $0` needs a runnumber"
		echo "`basename $0` <filename>"
		exit
	fi
fi

cat $1 | whichRunsAreNotOnDisk.sh | wc -l
exit

if [ "$1" != "" ] ; then
	if [ -f "$1" ] ; then
		whichRunsAreNotOnDisk.sh $1 | wc -l
	else 
		echo "File $1 doesn't exist!" ; exit
	fi
else 
	echo "Needs filename of runlist..."
fi

