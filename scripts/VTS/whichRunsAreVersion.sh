#!/bin/bash
# from a run list, prints the list of runs that are considered V4 runs, before T1 was moved
# written by Nathan Kelley-Hoskins Aug 2013

#echo "\$#:$#   \$1:$1   \$2:$2   \$3:$3   \$4:$4"

ISPIPEFILE=`readlink /dev/fd/0` # check to see if input is from terminal, or from a pipe
#echo "\$ISPIPEFILE: '$ISPIPEFILE'"
if [[ "$ISPIPEFILE" =~ ^/dev/pts/[0-9]{1,2} ]] ; then # its a terminal (not a pipe)
	if ! [ $# -eq 2 ] ; then # the human didn't add any arguments, and we must tell them so
		echo "Prints the run numbers that are of the specific run versions runs."
		echo "  For just V4 runs, do"
		echo "    $ `basename $0` 4 <file of runs>"
		echo "  For just V5 runs, do"
		echo "    $ `basename $0` 5 <file of runs>"
		echo "  To print all V5 *and* V6 runs, do"
		echo "    $ `basename $0` 56 <file of runs>"
		echo "  Can also work with pipes:"
		echo "    $ cat runlist.dat | whichRunsAreVersion.sh 56"
		exit
	fi
fi

# if we find 4, 5, or 6 in $1, then set appropriate flags
V4FLAG=false
if [[ "$1" == *4* ]] ; then
	V4FLAG=true ; fi
V5FLAG=false
if [[ "$1" == *5* ]] ; then
	V5FLAG=true ; fi
V6FLAG=false
if [[ "$1" == *6* ]] ; then
	V6FLAG=true ; fi

# list of run_id's to read in
RUNFILE=$2
if [ ! -e $RUNFILE ] ; then
	echo "File $RUNFILE could not be found in $PWD , sorry."
	exit	
fi
RUNLIST=`cat $RUNFILE`
#echo "RUNLIST:$RUNLIST"

# first run of V5 : 46642 
# first run of V6 : 63373  
for i in ${RUNLIST[@]} ; do
	if $V4FLAG ; then
		if [ "$i" -le "46641" ] ; then 
			echo "$i"
		fi
	fi
	if $V5FLAG ; then
		if [ "$i" -ge "46642" -a "$i" -le "63372" ] ; then 
			echo "$i"
		fi
	fi
	if $V6FLAG ; then
		if [ "$i" -ge "63373" ] ; then 
			echo "$i"
		fi
	fi
done

exit

