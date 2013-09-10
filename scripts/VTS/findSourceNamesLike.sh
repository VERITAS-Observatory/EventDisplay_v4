#!/bin/bash
# prints list of sources that contain the first input argument
# examples:
# $ findSourceNamesLike.sh GRB111
#   Searching for sources that contain 'GRB111'
#   GRB111005B
#   GRB111029A
#   GRB111225A
# $ findSourceNamesLike.sh PSR
#   Searching for sources that contain 'PSR'
#   PSR B0355+54
#   PSR J0023+09
#   PSR J0357+3205
#   PSR J1023+0038
#   PSR J2214+3002
#   PSRJ0631+1036
#   PSRJ2021+3651



# if arg 

NAM="$1"

# warn the user if bad input argument format
if [ ! "$NAM" ] ; then
	echo ; echo "Need a search string for comparison"
	echo ; echo " $ `basename $0` \"GRB11\""
	echo "   Searching for sources that contain 'GRB111'"
	echo "   GRB111005B"
	echo "   GRB111029A"
	echo "   GRB111225A" ; echo
	exit
fi

echo "Searching for sources that contain '$NAM'"

# get url of veritas db
MYSQLDB=`grep '^\*[ \t]*DBSERVER[ \t]*mysql://' $VERITAS_EVNDISP_ANA_DIR/ParameterFiles/EVNDISP.global.runparameter | egrep -o '[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}'`
if [ ! -n "$MYSQLDB" ] ; then
    echo "* DBSERVER param not found in \$VERITAS_EVNDISP_ANA_DIR/ParameterFiles/EVNDISP.global.runparameter!"
    exit
fi 

# do the mysql query
MYSQL="mysql -u readonly -h $MYSQLDB -A"
COUNT=0
RUNINFOARRAY=() # our list of unique sources
while read -r RUNID SOURCEID; do
	if [[ "$RUNID" =~ ^[0-9]+$ ]] ; then
		FOUND=false
		for i in "${!RUNINFOARRAY[@]}" ; do
			if [ "$SOURCEID" == "${RUNINFOARRAY[$i]}" ] ; then # we already have this source in our list
				FOUND=true
			fi
		done
		if ! $FOUND ; then # we have a new source, and need to add it to the list of sources
			RUNINFOARRAY+=("$SOURCEID")
			COUNT=$((COUNT+1))
		fi
	fi
done < <($MYSQL -e " select run_id, source_id from VERITAS.tblRun_Info where source_id like '%$NAM%' and run_type = 'observing' and observing_mode = 'wobble' and db_start_time > '2010-01-01 00:00:00' and db_end_time < '2013-07-01 00:00:00' and config_mask = 15 ;")

# alphabetize our source list
OLDIFS="$IFS"
IFS=$'\n' sorted=($(sort <<<"${RUNINFOARRAY[*]}"))
printf "%s\n" "${sorted[@]}"

exit

