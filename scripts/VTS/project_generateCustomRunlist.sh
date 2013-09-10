#!/bin/bash

# get veritas db url
MYSQLDB=`grep '^\*[ \t]*DBSERVER[ \t]*mysql://' $VERITAS_EVNDISP_ANA_DIR/ParameterFiles/EVNDISP.global.runparameter | egrep -o '[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}'`
if [ ! -n "$MYSQLDB" ] ; then
    echo "* DBSERVER param not found in \$VERITAS_EVNDISP_ANA_DIR/ParameterFiles/EVNDISP.global.runparameter!"
    exit
fi 

MYSQL="mysql -u readonly -h $MYSQLDB -A"
RUNINFOARRAY=()
while read -r RUNID ; do
	if [[ "$RUNID" =~ ^[0-9]+$ ]] ; then
		#echo "$RUNID"
		RUNINFOARRAY+=("$RUNID")
	fi
done < <($MYSQL -e " select run_id from VERITAS.tblRun_Info where source_id = 'Crab' and run_type = 'observing' and observing_mode = 'wobble' and weather <= 4 and duration > '00:15:00' and db_start_time > '2011-01-01 00:00:00' and db_end_time < '2013-07-01 00:00:00' and config_mask = 15 ;")

#echo "RUNINFOARRAY: ${RUNINFOARRAY[@]}"

COUNT=0
SUB=""
for ARUN in "${RUNINFOARRAY[@]}" ; do
	if [[ "$COUNT" -eq 0 ]] ; then
		SUB="run_id = $ARUN"
	else 
		SUB="$SUB OR run_id = $ARUN"
	fi
	COUNT=$((COUNT+1))
done
#echo "COUNT:$COUNT    SUB: $SUB"
if [ "$COUNT" -le "0" ] ; then
	exit
fi

FINALARRAY=()
COUNT=0
while read -r RUNID ; do
	if [[ "$RUNID" =~ ^[0-9]+$ ]] ; then
		COUNT=$((COUNT+1))
		#echo "$COUNT ~ $RUNID"
		FINALARRAY+=("$RUNID")
		echo "$RUNID"
	fi
done < <($MYSQL -e " select run_id from VOFFLINE.tblRun_Analysis_Comments where (status = 'good_run') and (tel_cut_mask is NULL or tel_cut_mask = 0) and ( data_category = 'science' ) and ( usable_duration > '00:15:00' ) and ( $SUB ) ;")

#echo "Final list: ${FINALARRAY[@]}"
exit

