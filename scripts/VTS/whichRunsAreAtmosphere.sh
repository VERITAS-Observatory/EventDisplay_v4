#!/bin/bash
# from a run list, prints the list of runs that were taken in a specific atmosphere, summer(22) or winter(21)
# written by Nathan Kelley-Hoskins Sept 2013

CONORM="\e[0m"
CORED='\e[1;31m'

ISPIPEFILE=`readlink /dev/fd/0` # check to see if input is from terminal, or from a pipe
if [[ "$ISPIPEFILE" =~ ^/dev/pts/[0-9]{1,2} ]] ; then # its a terminal (not a pipe)
	if ! [ $# -eq 2 ] ; then # the human didn't add any arguments, and we must tell them so
		echo
		echo "From a runlist or pipe, prints the run numbers that are of a particular atmosphere."
		echo " $ `basename $0` [w|21|s|22] <file of runs>" ; echo
		echo "w = 21 = winter, s = 22 = summer" ; echo
		echo "Print list of summer runs:"
		echo " $ `basename $0` s myrunlist.dat" ; echo
		echo "Print list of winter runs:"
		echo " $ `basename $0` 21 myrunlist.dat" ; echo
		echo "Works with pipes : " 
		echo " $ cat myrunlist.dat | `basename $0` w" ; echo
		echo -e "${CORED}WARNING!${CONORM}"
		echo "   The summer/winter status is crudely calculated as "
		echo "      'summer is any run in the months of May through October, inclusive (months 5,6,7,8,9,10)'."
		echo "      'winter is any run in the months of November through April, inclusive (months 11,12,1,2,3,4)'."
		echo "   Since that is not the exact way Summer is defined (the boundary between summer/winter moves each year), tread carefully when using this script." ; echo
		exit
	fi
fi

# list of run_id's to read in
RUNFILE=$2
if [ ! -e $RUNFILE ] ; then
	echo "File $RUNFILE could not be found in $PWD , sorry."
	exit	
fi
RUNLIST=`cat $RUNFILE`
#echo "RUNLIST:$RUNLIST"

SUMMFLAG=false
WINTFLAG=false
LOWARG=`echo "$1" | tr '[A-Z]' '[a-z]'` # make all uppercase letters in arg 1 lowercase, for easier handling
#echo "\$LOWARG: '$LOWARG'"
if [[ "$LOWARG" == *w* ]] || [[ "$LOWARG" == "21" ]] ; then
	WINTFLAG=true
fi
if [[ "$LOWARG" == *s* ]] || [[ "$LOWARG" == "22" ]] ; then
	SUMMFLAG=true
fi
#echo "\$WINTFLAG:$WINTFLAG    \$SUMMFLAG:$SUMMFLAG"
if $WINTFLAG && $SUMMFLAG ; then
	echo "recognized both summer[s] and winter[w] in arg '$1', can only specify one or the other"
	exit
elif ! $WINTFLAG && ! $SUMMFLAG ; then
	echo "Need to specifiy an atmosphere: Argument 1 '$1' needs to be either 'w' or 's'."
	exit
fi

# get database url from parameter file
MYSQLDB=`grep '^\*[ \t]*DBSERVER[ \t]*mysql://' "$VERITAS_EVNDISP_ANA_DIR/ParameterFiles/EVNDISP.global.runparameter" | egrep -o '[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}'`
    
if [ ! -n "$MYSQLDB" ] ; then
    echo "* DBSERVER param not found in \$VERITAS_EVNDISP_ANA_DIR/ParameterFiles/EVNDISP.global.runparameter!"
    exit
#else
#    echo "MYSQLDB: $MYSQLDB"
fi 

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
		
		# is the run month between May(5) and Oct(10), inclusive?
		if $SUMMFLAG ; then
			if [ $MM -ge 5 -a $MM -le 10 ] ; then
				echo "$RUNID"
			fi
		elif $WINTFLAG ; then
			if [ $MM -le 4 -o $MM -ge 11 ] ; then
				echo "$RUNID"
			fi
		fi
		
	fi
# This is where the MYSQL command is executed, with the list of requested runs
# You have to do it this way, because using a pipe | calls the command in a
# subshell, and that prevents variables from being saved within the 'while' loop
# http://stackoverflow.com/questions/14585045/is-it-possible-to-avoid-pipes-when-reading-from-mysql-in-bash
done < <($MYSQL -e "USE VERITAS ; SELECT run_id, data_start_time FROM tblRun_Info WHERE $SUB")

exit

