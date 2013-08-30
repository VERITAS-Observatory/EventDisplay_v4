#!/bin/bash
# from a run list, prints the list of runs that are NOT on disk.
# written by Nathan Kelley-Hoskins Aug 2013

ISPIPEFILE=`readlink /dev/fd/0` # check to see if input is from terminal, or from a pipe
if [[ "$ISPIPEFILE" =~ ^/dev/pts/[0-9]{1,2} ]] ; then # its a terminal (not a pipe)
	if ! [ $# -eq 1 ] ; then # the human didnt add any arguments, and we must tell them so
		echo "Prints the run numbers that are NOT stored on disk."
		echo " $ `basename $0` <file of runs>"
		exit
	fi
fi

# read database url from $VERITAS_EVNDISP_AUX_DIR/ParameterFiles/EVNDISP.global.runparameter
MYSQLDB=`grep '^\*[ \t]*DBSERVER[ \t]*mysql://' $VERITAS_EVNDISP_ANA_DIR/ParameterFiles/EVNDISP.global.runparameter | egrep -o '[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}'`

if [ ! -n "$MYSQLDB" ] ; then
	echo "* DBSERVER param not found in \$VERITAS_EVNDISP_ANA_DIR/ParameterFiles/EVNDISP.global.runparameter!"
	exit
#else
#	echo "MYSQLDB: $MYSQLDB"
fi


# list of run_id's to read in
RUNFILE=$1
if [ ! -e $RUNFILE ] ; then
	echo "File $RUNFILE could not be found in $PWD , sorry."
	exit	
fi
RUNLIST=`cat $RUNFILE`

# mysql login info
MYSQL="mysql -u readonly -h $MYSQLDB -A"

# generate list of runs to ask for ( run_id = RUNID[1] OR run_id = RUNID[2] etc)
COUNT=0
SUB=""
for ARUN in $RUNLIST ; do
	if [[ "$COUNT" -eq 0 ]] ; then
		SUB="run_id = $ARUN"
	else 
		SUB="$SUB OR run_id = $ARUN"
	fi
	COUNT=$((COUNT+1))
done
#echo "SUB:"
#echo "$SUB"

# search through mysql result rows, where each row's elements
# are assigned to RUNID and RUNDATE
while read -r RUNID RUNDATE ; do
	if [[ "$RUNID" =~ ^[0-9]+$ ]] ; then
		
		# decode the date tag
		read YY MM DD HH MI SE <<< ${RUNDATE//[-:]/ }
		#echo "  YEARMONTHDAY:$YY$MM$DD"
		
		# generate the filename
		TARGFILE="$VERITAS_DATA_DIR/data/d$YY$MM$DD/$RUNID.cvbf"
		
		# test to see if the file exists
		#echo "  Does file exist: $TARGFILE"
		if [ ! -e $TARGFILE ] ; then
			#echo "    File does not exist, need to download"
			#echo "$RUNID $RUNDATE $TARGFILE"
			
			# if the cvbf file does not exist, print the run_id
			echo "$RUNID"
		fi
	fi
# This is where the MYSQL command is executed, with the list of requested runs
# You have to do it this way, because using a pipe | calls the command in a
# subshell, and that prevents variables from being saved within the 'while' loop
# http://stackoverflow.com/questions/14585045/is-it-possible-to-avoid-pipes-when-reading-from-mysql-in-bash
done < <($MYSQL -e "USE VERITAS ; SELECT run_id, data_start_time FROM tblRun_Info WHERE $SUB")

exit

