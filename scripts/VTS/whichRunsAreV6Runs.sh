#!/bin/bash
# from a run list, prints the list of runs that are considered V6 runs, after camera upgrade
# written by Nathan Kelley-Hoskins Aug 2013

ISPIPEFILE=`readlink /dev/fd/0` # check to see if input is from terminal, or from a pipe
if [[ "$ISPIPEFILE" =~ ^/dev/pts/[0-9]{1,2} ]] ; then # its a terminal (not a pipe)
	if ! [ $# -eq 1 ] ; then # the human didn't add any arguments, and we must tell them so
		echo "Prints the run numbers that are V6 runs."
		echo " $ `basename $0` <file of runs>"
		exit
	fi
fi

# list of run_id's to read in
RUNFILE=$1
if [ ! -e $RUNFILE ] ; then
	echo "File $RUNFILE could not be found in $PWD , sorry."
	exit	
fi
RUNLIST=`cat $RUNFILE`
#echo "RUNLIST:$RUNLIST"

# first run of V5 : 46642 : GAHughes, 2013-08-02
# first run of V6 : 63373  
# thus V6 is all runs greater than or equal to run number 63373
for i in ${RUNLIST[@]} ; do
	if [ "$i" -ge "63373" ] ; then 
		echo "$i"
	fi
done

exit

