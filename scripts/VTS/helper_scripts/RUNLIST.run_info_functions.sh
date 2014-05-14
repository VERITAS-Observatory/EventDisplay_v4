#!/bin/bash

getRunArrayVersion() {
    RUN=$1
    if   (( $RUN < 46642 )) ; then EPOCH="V4"
    elif (( $RUN > 63373 )) ; then EPOCH="V6"
    else
        EPOCH="V5"
    fi
    echo $EPOCH
}

getRunAtmosphere() {
    RUN=$1

    # get database url from parameter file
    MYSQLDB=`grep '^\*[ \t]*DBSERVER[ \t]*mysql://' "$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/EVNDISP.global.runparameter" | egrep -o '[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}'`
    if [[ -z "$MYSQLDB" ]] ; then
       return -1   # error
    fi

    # mysql login info
    MYSQL="mysql -u readonly -h $MYSQLDB -D VERITAS -A"

    # read run info from database
    while read -r RUNID RUNTYPE RUNDATE ; do
        if [[ "$RUNID" =~ ^[0-9]+$ ]] ; then
            RUNMODE=$RUNTYPE
            read YY MM DD HH MI SE <<< ${RUNDATE//[-:]/ }
        fi
    done < <($MYSQL -e "USE VERITAS ; SELECT run_id, run_type, data_start_time FROM tblRun_Info WHERE run_id = $RUN")
    date="$YY$MM$DD"
    month="$MM"

    # set atmosphere based on run date
    if [[ "$date" > 20071026 && "$date" < 20080420 ]] ||
       [[ "$date" > 20081113 && "$date" < 20090509 ]] ||
       [[ "$date" > 20091102 && "$date" < 20100428 ]] ||
       [[ "$date" > 20101023 && "$date" < 20110418 ]] ||
       [[ "$date" > 20111110 && "$date" < 20120506 ]] ||
       [[ "$date" > 20121029 && "$date" < 20130425 ]] ;
    then
        ATMO=21     # winter
    elif (( "$date" >= 20130425 )) || (("$date" <= 20071026 )); then
        # don't have specific dates for summer/winter boundary, so we will generalize to the months
        # May through October inclusive is 'summer'
        if (( "$month" >= 5 )) && (( "$month" <= 10 )); then
            ATMO=22     # summer
        # November through April inclusive is 'winter'
        elif (( "$month" <= 4 )) || (( "$month" >= 11 )); then
            ATMO=21     # winter
        else
            echo "error"
            return
        fi
    else
        ATMO=22     #summer
    fi

    # add suffix for observing mode
    [[ "$RUNMODE" == "obsFilter" ]] && ATMO="${ATMO}_UV"
    [[ "$RUNMODE" == "obsLowHV" ]]  && ATMO="${ATMO}_RedHV"

    echo $ATMO
}

