#!/bin/bash
# Author: Nathan Kelley-Hoskins
# Date: April 2014
#
# This file contains all of the functions shared by
# ANALYSIS.pipeline and SIMULATION.pipeline
#


if [[ "$SCIPIPE_COLOROPT" =~ nologs ]] ; then
	CONORM=""
	CORED=""    ; COTRED=""
	COPURPLE="" ; COTPURPLE=""
	COGREEN=""  ; COTGREEN=""
	COYELLOW="" ; COTYELLOW=""
	COCYAN=""   ; COTCYAN=""
	COBLUE=""   ; COTBLUE=""
else
	CONORM="\033[0;00m"
	CORED="\033[1;31m"    ; COTRED="\033[0;31m"
	COPURPLE="\033[1;35m" ; COTPURPLE="\033[0;35m"
	COGREEN="\033[1;32m"  ; COTGREEN="\033[0;32m"
	COYELLOW="\033[1;33m" ; COTYELLOW="\033[0;33m"
	COCYAN="\033[1;36m"   ; COTCYAN="\033[0;36m"
	COBLUE="\033[1;34m"   ; COTBLUE="\033[0;34m"
fi

# to let people know where functions are
BINNAME="$EVNDISPSYS/scripts/VTS/helper_scripts/pipeline_functions.sh"

# words that are treated as warnings,
# and we should highlight them yellow
WARNREGEX="warning|warn|recov"

# words that are treated as errors,
# and we should highlight them red
ERRORREGEX="error|segment|corrupt"

# extra place to duplicate output to stdout, look for >&5
exec 5>&1 

# spit text out to stderr, so user is
# more likely to see it
function echoerr(){ echo -e "$@" 1>&2 ; }

# count jobs in array-job-code '1-8:1'
function countArrayJobs {
    local ARRAYJOBCODE="$1"
    #echoerr "countArrayJobs: ARRAYJOBCODE '$ARRAYJOBCODE'"
    local REFORMT=$( echo "$ARRAYJOBCODE" | tr '-' ' ' | tr ':' ' ' | awk '{ printf "%s %s %s\n", $1, $3, $2 }')
    #echoerr "countArrayJobs: REFORMT $REFORMT"
    local COUNT=$( seq `echo $REFORMT` | wc -l )
    #echoerr "countArrayJobs: $COUNT"
    echo "$COUNT"
}

# check if any jobs are currently running,
# pause SLEEPTIME seconds between each check
# BY DESIGN: this function blocks until
#   all jobs are either 
# runnumbers are stored in file $1,
function checkIfJobsAreDone {
    local RUNNINGJOBS="$1"
    local STAGEFILE="$2"
    local SLEEPTIME="$3"
    local JOBLIST=""
    local JOBSWAITING=0
    local JOBSINPROGRESS=0
    local JOBSERRORED=0
    local ALLJOBS=""
    local JOBLINE=""
    local JOBSTATE=""
    local JOBSDONE=""
    local RUNNINGSTAGE=""
    local JL=""
    local JITERCODE=""
    local TOTALJOBS=""
    # RUNNINGJOBS should be the filename that contains
    # the simple list of jobnumbers to check if running
    if [ -e "$RUNNINGJOBS" ] ; then # the RUNNINGJOBS file exists and we can do stuff
        while true ; do
            JOBLIST=`cat $RUNNINGJOBS`
            JOBSWAITING=0
            JOBSINPROGRESS=0
            JOBSERRORED=0
            ALLJOBS=$( qstat -u $USER | tail -n +3 | awk '{printf "%s %s %s %s \"%s\"\n", $1, $3, $5, $9, $0 }')
            TOTALJOBS=0
            #echo "ALLJOBS:$ALLJOBS"
            while read -r AJOB ; do
                #echo "AJOB: '$AJOB'"
                JOBNUM=$(   echo "$AJOB" | tr '.' ' ' | awk '{ print $1 }' )
                #echo "JOBNUM: '$JOBNUM'"
                JOBARRAYCODE=$( echo "$AJOB" | tr '.' ' ' | awk '{ print $2 }' ) 
                #echo "JOBARRAYCODE: '$JOBARRAYCODE'"
                
                # figure out how many array jobs are 
                # associated with this job number
                if [[ -z "$JOBARRAYCODE" ]] ; then     
                    JOBARRAYN=1
                else
                    JOBARRAYN=$( countArrayJobs "$JOBARRAYCODE" )
                fi
                #echo "JOBARRAYN: '$JOBARRAYN'"
                TOTALJOBS=$(( TOTALJOBS + $JOBARRAYN ))
                
                # figure out how many jobs are in each state
                JOBLINE=$( echo "$ALLJOBS" | grep -P "^${JOBNUM}" )
                #echo "JOBLINE '$JOBLINE'"
                while read -r JL ; do
                    #echo "JL '$JL'"
                    JOBSTATE=$( echo "$JL" | awk '{print $3}' )
                    
                    # code for arrayjobs, i.e. "1-4:1"
                    JITERCODE=$( echo "$JL" | awk '{ print $4 }' )
                    ADDJOB=1
                    
                    # if we have a range of jobs "1-4:1", then we 
                    # need to count how many there are
                    if [[ "$JITERCODE" =~ [-:] ]] ; then 
                        echo "range of jobs!"
                        ADDJOB=$( seq `echo "$JITERCODE" | tr '-' ' ' | tr ':' ' ' | awk '{ printf "%s %s %s\n", $1, $3, $2 }'` | wc -l )
                    fi
                    #echo "JOBSTATE '$JOBSTATE'"
                    #echo "JOB:$AJOB"
                    #echo "  JOBLINE:$JOBLINE"
                    #echo "  JOBSTATE:$JOBSTATE"
                    if   [ "$JOBSTATE" == "qw" ] ; then JOBSWAITING=$((    JOBSWAITING    + $ADDJOB))
                    elif [ "$JOBSTATE" == "r"  ] ; then JOBSINPROGRESS=$(( JOBSINPROGRESS + $ADDJOB))
                    elif [ "$JOBSTATE" == "E"  ] ; then JOBSERRORED=$((    JOBSERRORED    + $ADDJOB))
                    fi
                    #echo "JOBSINPROGRESS '$JOBSINPROGRESS'"
                done < <(echo "$JOBLINE")
            done <<< "$JOBLIST"
            #echo "NJOBS `echo "$ALLJOBS" | wc -l`"
            #echo "JOBSINPROGRESS '$JOBSINPROGRESS'"
            JOBSDONE=$(( TOTALJOBS - (JOBSWAITING+JOBSINPROGRESS+JOBSERRORED) )) # how many jobs are not waiting, running, or errored
            RUNNINGSTAGE=$( cat "$STAGEFILE" )
            echo ; echo "$RUNNINGSTAGE jobs:"
            echo -e "  ${COTBLUE}$JOBSWAITING jobs waiting${CONORM}"
            echo -e "  ${COTCYAN}$JOBSINPROGRESS jobs in progress${CONORM}"
            echo -e "  ${COTGREEN}$JOBSDONE jobs complete${CONORM}"
            if [ "$JOBSERRORED" -gt "0" ] ; then # let the user know some jobs have errored
                echo -e "  ${CORED}$JOBSERRORED jobs in the error state${CONORM}"
            fi
            echo
            if [ "$((JOBSWAITING+JOBSINPROGRESS))" -le "0" ] ; then
                #echo -ne "\007" ## I beep for the users.
                if [[ "$JOBSERRORED" > 0 ]] ; then
                    echo ; echo -e "  ${COTYELLOW}Careful, some jobs are in the error state!${CONORM}"
                fi
                break
            else
                sleep "$SLEEPTIME"
            fi
        done
    else
        echo "Jobs-file '$RUNNINGJOBS' currently contains no running jobs."
    fi
    exit 0
}

# check if any jobs are currently running,
# pause SLEEPTIME seconds between each check
# BY DESIGN: this function blocks until
#   all jobs are either 
# runnumbers are stored in file $1,
function checkIfJobsAreDoneAlt {
    local RUNNINGJOBS="$1"
    local STAGEFILE="$2"
    local SLEEPTIME="$3"
    local JOBLIST=""
    local JOBSWAITING=0
    local JOBSINPROGRESS=0
    local JOBSERRORED=0
    local ALLJOBS=""
    local JOBLINE=""
    local JOBSTATE=""
    local JOBSDONE=""
    local RUNNINGSTAGE=""
    local JL=""
    local JITERCODE=""
    local TOTALJOBS=""
    # RUNNINGJOBS should be the filename that contains
    # the simple list of jobnumbers to check if running
    if [ -e "$RUNNINGJOBS" ] ; then # the RUNNINGJOBS file exists and we can do stuff
		
		# use subshell () so we can set IFS to \newline and read JOBLIST and JOBLINE
		# in for loops, rather than using 'read' all over the place (which caused buggy
		# behavior in this function)
		( IFS='
'
        while true ; do
            JOBLIST=`cat $RUNNINGJOBS`
			#echo "JOBLIST:"
			#echo "$JOBLIST"
            JOBSWAITING=0
            JOBSINPROGRESS=0
            JOBSERRORED=0
            ALLJOBS=$( qstat -u $USER | tail -n +3 | awk '{printf "%s %s %s %s \"%s\"\n", $1, $3, $5, $9, $0 }')
            TOTALJOBS=0
            #echo "ALLJOBS:$ALLJOBS"
			for AJOB in ${JOBLIST[@]} ; do
            #while read -r AJOB ; do
				#echo
                #echo "AJOB: '$AJOB'"
				#echo
                JOBNUM=$(   echo "$AJOB" | tr '.' ' ' | awk '{ print $1 }' )
                #echo "JOBNUM: '$JOBNUM'"
                JOBARRAYCODE=$( echo "$AJOB" | tr '.' ' ' | awk '{ print $2 }' ) 
                #echo "JOBARRAYCODE: '$JOBARRAYCODE'"
                
                # figure out how many array jobs are 
                # associated with this job number
                if [[ -z "$JOBARRAYCODE" ]] ; then     
                    JOBARRAYN=1
                else
                    JOBARRAYN=$( countArrayJobs "$JOBARRAYCODE" )
                fi
                #echo "JOBARRAYN: '$JOBARRAYN'"
                TOTALJOBS=$(( TOTALJOBS + $JOBARRAYN ))
                
                # figure out how many jobs are in each state
                JOBLINE=$( echo "$ALLJOBS" | grep -P "^${JOBNUM}" )
                #echo "JOBLINE '$JOBLINE'"
                #while read -r JL ; do
				for JL in $JOBLINE ; do
                    #echo "JL '$JL'"
                    JOBSTATE=$( echo "$JL" | awk '{print $3}' )
                    
                    # code for arrayjobs, i.e. "1-4:1"
                    JITERCODE=$( echo "$JL" | awk '{ print $4 }' )
                    ADDJOB=1
                    
                    # if we have a range of jobs "1-4:1", then we 
                    # need to count how many there are
                    if [[ "$JITERCODE" =~ [-:] ]] ; then 
                        echo "range of jobs!"
                        ADDJOB=$( seq `echo "$JITERCODE" | tr '-' ' ' | tr ':' ' ' | awk '{ printf "%s %s %s\n", $1, $3, $2 }'` | wc -l )
                    fi
                    #echo "JOBSTATE '$JOBSTATE'"
                    #echo "JOB:$AJOB"
                    #echo "  JOBLINE:$JOBLINE"
                    #echo "  JOBSTATE:$JOBSTATE"
                    if   [ "$JOBSTATE" == "qw" ] ; then JOBSWAITING=$((    JOBSWAITING    + $ADDJOB))
                    elif [ "$JOBSTATE" == "r"  ] ; then JOBSINPROGRESS=$(( JOBSINPROGRESS + $ADDJOB))
                    elif [ "$JOBSTATE" == "E"  ] ; then JOBSERRORED=$((    JOBSERRORED    + $ADDJOB))
                    fi
                    #echo "JOBSINPROGRESS '$JOBSINPROGRESS'"
                #done < <(echo "$JOBLINE")
				done
            #done <<< "$JOBLIST"
			done 
			#IFS=$OLDIFS
            #echo "NJOBS `echo "$ALLJOBS" | wc -l`"
            #echo "JOBSINPROGRESS '$JOBSINPROGRESS'"
            JOBSDONE=$(( TOTALJOBS - (JOBSWAITING+JOBSINPROGRESS+JOBSERRORED) )) # how many jobs are not waiting, running, or errored
            RUNNINGSTAGE=$( cat "$STAGEFILE" )
            echo ; echo "$RUNNINGSTAGE jobs:"
            echo -e "  ${COTBLUE}$JOBSWAITING jobs waiting${CONORM}"
            echo -e "  ${COTCYAN}$JOBSINPROGRESS jobs in progress${CONORM}"
            echo -e "  ${COTGREEN}$JOBSDONE jobs complete${CONORM}"
            if [ "$JOBSERRORED" -gt "0" ] ; then # let the user know some jobs have errored
                echo -e "  ${CORED}$JOBSERRORED jobs in the error state${CONORM}"
            fi
            echo
            if [ "$((JOBSWAITING+JOBSINPROGRESS))" -le "0" ] ; then
                #echo -ne "\007" ## I beep for the users.
                if [[ "$JOBSERRORED" > 0 ]] ; then
                    echo ; echo -e "  ${COTYELLOW}Careful, some jobs are in the error state!${CONORM}"
                fi
                break
            else
                sleep "$SLEEPTIME"
            fi
        done
		)
    else
        echo "Jobs-file '$RUNNINGJOBS' currently contains no running jobs."
    fi
    exit 0
}

# count how many strings match $searchstring
# only return the number, no formatting
function countStrings {
    local searchstring="$1"
    declare -a argAry1=("${!2}")
    #echo "argAry1: ${argAry1[@]}"
    local tmpcnt=0
    local ERRORCOUNT=0
    for i in "${argAry1[@]}" ; do
        tmpcnt=0
        tmpcnt=$( grep -c -E -i $searchstring "$i" )
        ERRORCOUNT=$((ERRORCOUNT+tmpcnt))
        #echo "  $tmpcnt : $i"
    done
    #echo " ERRORCOUNT : $ERRORCOUNT"
    echo "$ERRORCOUNT"
}

# display to terminal the number of warnings
# in a pretty formatted way
function formatwarncount {
    declare -a argAry1=("${!1}")
    local WARNCOUNT=$( countStrings "$WARNREGEX" argAry1[@] )
    local WARNCOUNTSTR=""
    local plural=" "
    if [[ $WARNCOUNT != 1 ]] ; then plural="s" 
    fi
    if [[ $WARNCOUNT  > 0 ]] ; then
        WARNCOUNTSTR=$( printf "${COTYELLOW}%3d warning%s${CONORM}" $WARNCOUNT "$plural" )
    else
        WARNCOUNTSTR=" "
    fi
    echo -e "$WARNCOUNTSTR"
}

# display to terminal the number of errors
# in a pretty formatted way
function formaterrorcount {
    declare -a argAry1=("${!1}")
    local ERRORCOUNT=$( countStrings "$ERRORREGEX" argAry1[@] )
    local ERRORCOUNTSTR=""
    local plural=" "
    if [[ "$ERRORCOUNT" != 1 ]] ; then plural="s" ; fi
    if [[ "$ERRORCOUNT"  > 0 ]] ; then
        ERRORCOUNTSTR=$( printf "${COTRED}%3d error%s${CONORM}" $ERRORCOUNT "$plural" )
    else
        ERRORCOUNTSTR=" "
    fi
    echo -e "$ERRORCOUNTSTR"

}

# see if 'warning', 'recov', 'error', 'segmentation', or $DEBUGREGEX
# occurred in the target file, and print the filename and the entire 
# line if they did.  Otherwise just print the logfile name
function checkLogFileForProblems {
    local TARGFILE="$1"
    local DEBUGREGEX="$2"
    local REALFILE="`readlink -f "$TARGFILE"`"
    if [ ! -e "$TARGFILE" ] ; then
        echo -e "$BINNAME checkLogFileForProblems: ${CORED}file '${TARGFILE}' not found... exiting.${CONORM}"
        exit
    fi
    #local OUTSTRING="`grep -iE 'recov|warning|error|segmentation' $TARGFILE`"
    local OUTSTRING="`grep -iE "${WARNREGEX}|${ERRORREGEX}" $TARGFILE`"
    OUTSTRING="$OUTSTRING `grep -P $DEBUGREGEX $TARGFILE`"
    #OUTSTRING=$( echo "$OUTSTRING" | tr ' ' '' )
    OUTSTRING=$( echo "$OUTSTRING" | sed -e 's/ //g' )
    if [ ! -z "$OUTSTRING" ] ; then # $OUTSTRING is not empty, print ALL THE MESSAGES
        echo -e "${COTBLUE}~~~~~~ Found in ${REALFILE}${CONORM}"
        
        # yellow for warning-patterns
        GREP_COLOR="0;33" grep --color=always -E -i "$WARNREGEX" $TARGFILE
        
        # red for error-patterns
        GREP_COLOR="0;31" grep --color=always -E -i "$ERRORREGEX" $TARGFILE
        
        # light blue for the debugging-patterns
        GREP_COLOR="0;35" grep --color=always -P $DEBUGREGEX $TARGFILE
    else
        echo -e "${COTBLUE}~~~~~~ Scanned ${TARGFILE}${CONORM}"
    fi
}

# look for $SEARCHSTRING withing file $TARGFILE
# prints 'run complete' if found, else prints 'run  incomplete!!'
function checkForSuccess {
    local TARGFILE="$1"
    local SEARCHSTR="$2"
    local REALFILE="`readlink -f "$TARGFILE"`"
    if [ ! -e "$TARGFILE" ] ; then
        echo -e "`basename $BINNAME`: ${CORED}file '${TARGFILE}' not found... exiting.${CONORM}"
        exit
    fi
    local OUTSTRING="`grep -i "$SEARCHSTR" $TARGFILE`"
    if [ -z "$OUTSTRING" ] ; then # OUTSTRING has length zero, and run may be bad
        echo -e "${COTRED}run looks bad!!${CONORM}"
        else # run is probably ok
        echo -e "run looks healthy"
    fi
}

# same as checkForSuccess, but print more text,
# including the full logfile name and line containing
# success text
function checkLogFileForStageSuccess {
    local TARGFILE="$1"
    local SEARCHSTR="$2"
    local REALFILE="`readlink -f "$TARGFILE"`"
    if [ ! -e "$TARGFILE" ] ; then
        echo -e "`basename $BINNAME`: ${CORED}file '${TARGFILE}' not found... exiting.${CONORM}"
        exit
    fi
    #local SEARCHSTR="...outputfile closed"
    local OUTSTRING="`grep -i "$SEARCHSTR" $TARGFILE`"
    if [ -z "$OUTSTRING" ] ; then
        echo -e "${COTBLUE}~~~~~~ From file ${REALFILE} :${CONORM}"
        echo -e "${COTBLUE}~~~~~~${CONORM} ${COTRED}Warning: `basename $TARGFILE .anasum.log` may not have ended properly...${CONORM}"
    else
        echo -e "${COTBLUE}~~~~~~ From file ${REALFILE} :${CONORM}"
        echo -e "${COTBLUE}~~~~~~${CONORM} Run seems healthy..."
        #GREP_COLOR="0;36" grep --color=always -i "$SEARCHSTR" $TARGFILE
    fi
}

# check if there are any running jobs,
# and exit if there are
function haltIfAnyJobsAreRunning {
    local RUNNINGJOBS="$1"
    local JOBSARERUNNING=false
    local JOBSTOCHECK=""
    local JOBDATA=""
    local NUMBEROFRUNNINGJOBS=0
    local STATUS=""
    if [ -e "$RUNNINGJOBS" ] ; then
        JOBSTOCHECK=`cat $RUNNINGJOBS`
        JOBDATA=$( qstat -u $USER | tail -n +3 | awk '{ print $1 }' )
        for AJOB in $JOBSTOCHECK ; do
            STATUS=$( echo "$JOBDATA" | grep "$AJOB" )
            #echo "STATUS:'$STATUS'"
            if [ ! -z "$STATUS" ] ; then
                #echo "checkIfAnyJobsAreRunning: Yes!"
                JOBSARERUNNING=true
                break
            fi
        done
    fi
    if $JOBSARERUNNING ; then
        echo "Jobs are currently running. Please wait."
        exit 0
    fi
}


# kill any running jobs in the joblist file
function killJobsInJobfile {
    local RUNNINGJOBS="$1"
    if [ -e "$RUNNINGJOBS" ] ; then
        JOBLIST=`cat $RUNNINGJOBS`
        for AJOB in $JOBLIST ; do
            qdel "$AJOB"
        done
        rm -rf $RUNNINGJOBS
    else
        echo "No currently running jobs to kill."
    fi
    exit 0
}

# get a batch job's exit status
#  0 = good
# !0 = bad
function getJobExitStatus {
    local JOBNUM="$1"
    local ESTAT=$( qacct -j "$JOBNUM" | grep exit_status | awk '{ print $2 }' )
}

function getParticleCodeFromParticleName {
    local name="$1"
    local code=0
    if   [[ "$name" == "gamma"    ]] ; then code="1"
    elif [[ "$name" == "electron" ]] ; then code="2"
    elif [[ "$name" == "proton"   ]] ; then code="14"
    elif [[ "$name" == "alpha"    ]] ; then code="402"
    else echo "error, unrecognized particle code '$name', exiting..." ; exit 1
    fi      
    return $code
}



function getTagArgFromTagline {
    local TAGNAME="$1"
    local TAGLINE="$2"
    echo "$TAGLINE"  | tr '+' '\n' | grep -P "${TAGNAME}=" | grep -oP "=.+" | tr -d '=' | sed -e 's|^ *||' -e 's| *$||'
}

function scanLogFile {
    local filetag=$1
    local filename=$2
    local debugregex=$3
    if [[ -f $filename ]] ; then
        checkLogFileForProblems "$filename" "$debugregex"
    else
        echo -e "${COTRED}~~~~~~ Error, $filetag file '$filename' doesn't exist...$CONORM"
    fi
}

# if part of the input path matches the path of $VERITAS_USER_LOG_DIR, replace the match with "$VULD"
# REQUIREMENT: following line must be in .zshenv:
# export VULD=$VERITAS_USER_LOG_DIR
function simplifyToVULD {
    local INPUT="$1"      # so that when it is printed to a terminal, it's much simpler for a human to read
    local CLEANINPUT=$( readlink -m "$INPUT" ) # clean up directory (no extra //'s)
    local OUTPUT=$( echo "$CLEANINPUT" | sed -e 's:'$VULD':$VULD/:g' )
    echo "$OUTPUT"
}

# if part of the input path matches the path of $EVNDISPSYS, replace the match with "$EVNDISPSYS"
function simplifyToEVNDISPSYS {
    local INPUT="$1"      # so that when it is printed to a terminal, it's much simpler for a human to read
    local CLEANINPUT=$( readlink -m "$INPUT"      ) # clean up directory (no extra //'s)
	local CLEANMATCH=$( readlink -m "$EVNDISPSYS" )
    local OUTPUT=$( echo "$CLEANINPUT" | sed -e 's:'$CLEANMATCH':$EVNDISPSYS/:g' )
    echo "$OUTPUT"
}

# if part of the input path matches the path of $VERITAS_EVNDISP_AUX_DIR, replace the match with "$VERITAS_EVNDISP_AUX_DIR"
function simplifyToAUXDir {
    local INPUT="$1"      # so that when it is printed to a terminal, it's much simpler for a human to read
    local CLEANINPUT=$( readlink -m "$INPUT"      ) # clean up directory (no extra //'s)
	local CLEANMATCH=$( readlink -m "$VERITAS_EVNDISP_AUX_DIR" )
    local OUTPUT=$( echo "$CLEANINPUT" | sed -e 's:'$CLEANMATCH':$VERITAS_EVNDISP_AUX_DIR:g' )
    echo "$OUTPUT"
}


# convert a raw file size in bytes
# to 'XXXkb, XXXMb, XXXGb
function formatFileSize {
    local BYTESIZE="$1"
    local OUTSTRING=""
    if [ -z "$BYTESIZE" ] ; then BYTESIZE=0 ; fi
    if   (( $BYTESIZE < 1000          )) ; then OUTSTRING=$( printf "%3dB " $(( BYTESIZE            )) )
    elif (( $BYTESIZE < 1000000       )) ; then OUTSTRING=$( printf "%3dKB" $(( BYTESIZE/1000       )) )
    elif (( $BYTESIZE < 1000000000    )) ; then OUTSTRING=$( printf "%3dMB" $(( BYTESIZE/1000000    )) )
    elif (( $BYTESIZE < 1000000000000 )) ; then OUTSTRING=$( printf "%3dGB" $(( BYTESIZE/1000000000 )) )
    fi
    echo "$OUTSTRING"
}

# if given a filename, check to see if its a valid filename,
# and that the file actually exists
# will return true if filename is invalid or doesn't exist
filenameIsNotHealthy() {
    local FILENAME=$1  # full filename to check
    local KEYWORD=$2   # keyword used to locate this file (OLOG,ELOG,EVNDISPLOG, etc)
    local HUMANNAME=$3 # human-readable name for this file
    local FOUNDIN=$4   # the logfile '$FILENAME' was found in, by grepping for '$KEYWORD'
    if [ -z "$FILENAME" ] ; then
        echoerr "${COTRED}Error, the ${COTYELLOW}${HUMANNAME}${COTRED} '$FILENAME' is an empty string, this should have been printed in logfile '$FOUNDIN' with keyword '$KEYWORD', skipping further checks of this run's files...$CONORM"
        return 0
    fi
    if [ ! -f "$FILENAME" ] ; then
        echoerr "${COTRED}Error, the ${COTYELLOW}${HUMANNAME}${COTRED} '$FILENAME' doesn't seem to exist, it was listed in $FOUNDIN with keyword '$KEYWORD', skipping further checks of this run's files...$CONORM"
        return 0
    fi

    return 1
}

# human-readable names for the batch's stdout and stderr logfiles,
# usually something like "scriptname.sh.oJOBNUMBER" or "scriptname.sh.eJOBNUMBER"
HUMANBATCHSTDOUT="Batch stdout log file"
HUMANBATCHSTDERR="Batch stderr log file"

# script for checking batch job exit statuses
BATCHCHECKSCRIPT="$EVNDISPSYS/scripts/VTS/helper_scripts/UTILITY.jobInfo"

# check job numbers listed in QSUBDATA,
# see if they completed properly,
# and print for the human
function checkBatchJobExits {
    local QSUBDATA=$(   cat "$1" )
    local RUNNUMBERS=$( cat "$2" )
    local JOBNUM=0
    local OUTTXT=""
    if command -v $BATCHCHECKSCRIPT > /dev/null 2>&1 ; then
        echo -e "${COTBLUE}checking batch exit statuses${CONORM}"
        for ARUN in $RUNNUMBERS; do
            #echo "ARUN:$ARUN" 2>&1
            JOBNUM=$( echo "$QSUBDATA" | grep -P "RUN $ARUN JOBID \d+" | awk '{ print $4 }' | tr '.' ' ' | awk '{ print $1 }' )
            #echo "  JOBNUM:'$JOBNUM'" 2>&1
            $BATCHCHECKSCRIPT "$JOBNUM" | awk '{ printf "run:'$ARUN'  %s\n", $0 }'
        done 
    else
        echo "can't find '$BATCHCHECKSCRIPT', not checking cluster job exit status..."
    fi
}

function tagString {
    local tagname=$1
    local tagarg=$2
    echo "+${tagname}=$tagarg"
}

function tagDir {
    local tagname=$1
    local tagarg=$2
    echo "${tagname}_$tagarg"
}


