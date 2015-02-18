#!/bin/bash
# Authors: Nathan Kelley-Hoskins, Henrike Fleischack
# Date: Oct 2013
#
# script for generating event display parameter file names for a list of runs
# to see more info, do:
# $ generateParamFiles.sh
#
# Notes:
#   - Some things are hard-coded, search for the word HARDCODE to see them
#   - There are 3 other datecodes which are hard-coded right now
#   - The atmosphere determination is hard-coded in the IsWinter function
#   - The V4, V5, and V6 determination is hard-coded
#

CONORM="\033[0;00m"
CORED="\033[1;31m"
COTRED="\033[0;31m"
COYEL="\033[1;33m"
function echoerr(){ echo -e "$@" 1>&2; } #for spitting out error text

# epoch file to load
METHOD="useparamfile"
EPOCHSPARAMFILE="$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/VERITAS.Epochs.runparameter"

function IsWinter {
	local date="$1"
    local month="${date:4:2}"
	local WinterCode="98"
	
	# get atmo boundary dates from param file
	if [[ "$METHOD" == "useparamfile" ]] ; then
		
		# get only lines that start with '*'
		local ATMOTHRESH=$( cat $EPOCHSPARAMFILE | grep -P "^\s??\*" | grep "ATMOSPHERE" )
		#echo "$ATMOTHRESH"
		
		# other vars
		local ATMOCODE=""
		local MINDATE=""
		local MAXDATE=""
		
		# flag for if we found the atmo
		local FOUNDATMO=false
		(IFS='
'
		for line in $ATMOTHRESH ; do
			#echoerr "line:$line"
			ATMOCODE=$( echo "$line" | awk '{ print $3 }' | grep -oP "\d+" )
			MINDATE=$(  echo "$line" | awk '{ print $4 }' | tr -d '-' | grep -oP "\d+" )
			MAXDATE=$(  echo "$line" | awk '{ print $5 }' | tr -d '-' | grep -oP "\d+" )
			#echoerr "  ATMOCODE:$ATMOCODE"
			#echoerr "  MINDATE: $MINDATE"
			#echoerr "  TARGDATE:$date"
			#echoerr "  MAXDATE: $MAXDATE"
			if (( "$date" >= "$MINDATE" )) && (( "$date" <= "$MAXDATE" )) ; then
				
				# winter
				if [[ "$ATMOCODE" == "21" ]] ; then
					#echo 1 	
					#echoerr "  $date - winter!"
					FOUNDATMO=true
					WinterCode="21"
					echo "$WinterCode"
					break
				# summer
				elif [[ "$ATMOCODE" == "22" ]] ; then
					#echo 2
					#echoerr "  $date - summer!"
					FOUNDATMO=true
					WinterCode="22"
					echo "$WinterCode"
					break
				fi
			fi
		done 
		)
		# 3 = did not find valid atmo range
		if [ ! $FOUNDATMO ] ; then
			#echo 3
			WinterCode="99"
			echo "$WinterCode"
		fi
	
	fi
	
	# hardcoded old method
	if [[ "$METHOD" == "usehardcoded" ]] ; then
		if
			# HARDCODE
			# atmosphere dates, the boundaries between summer and winter
			#    atmospheres change each year
			[ "$date" -gt "20071026" ] && [ "$date" -lt "20080420" ] ||
			[ "$date" -gt "20081113" ] && [ "$date" -lt "20090509" ] ||
			[ "$date" -gt "20091102" ] && [ "$date" -lt "20100428" ] ||
			[ "$date" -gt "20101023" ] && [ "$date" -lt "20110418" ] ||
			[ "$date" -gt "20111110" ] && [ "$date" -lt "20120506" ] ||
			[ "$date" -gt "20121029" ] && [ "$date" -lt "20130425" ] ; then
			WinterFlag=true
		elif [ "$date" -ge "20130425" -o "$date" -le "20071026" ] ; then
			# if summer/winter boundary not explicitly defined, define it via the month
			# may through october inclusive is 'summer'
			if   [ "$month" -ge 5 -a "$month" -le 10 ] ; then
				#echo 2 # summer
				WinterFlag=false
				WinterCode="22"
			# november through april inclusive is 'winter'
			elif [ "$month" -le 4 -o "$month" -ge 11 ] ; then
				#echo 1 # winter
				WinterFlag=true
				WinterCode="21"
			else
				#echo 3 # unassignable
				echoerr "Error, can only assign atmosphere to runs before 20130425, exiting..."
				exit 1
			fi
		else
			WinterFlag=false
		fi
	fi
}

function GetActiveTelComboCode {
	CONFMASK=$1
	CUTMASK=$2
	if [[ "$cutmask" == "NULL" ]] ; then CUTMASK="0" ; fi
	CUTMASK=$((15-CUTMASK))
	#echo "   GetActiveTelComboCode : configmask=$CONFMASK     cutmask=$CUTMASK"
	CONFIG=""
	DQM=""
	USE=""
	for tel in 1 2 3 4
	do
		CONFBIT=$(( $CONFMASK%2 ))
		if [ $CONFBIT -eq 1 ]
		then
			CONFIG=$CONFIG$tel
		fi
		CONFMASK=$(( $CONFMASK/2 ))
		CUTBIT=$(( $CUTMASK/(2**(4-$tel)) ))
		if [ $CUTBIT -eq 1 ]
		then
			CUT=$CUT$tel
		fi
		CUTMASK=$(( $CUTMASK%(2**(4-$tel)) ))
		if [ $CUTBIT -eq 1 ] && [ $CONFBIT -eq 1 ]
		then
			USE="$USE$tel"
		fi
	done
	#echo "   config '$CONFMASK',  cut '$CUTMASK', use '$USE'"
	TELCOMBOCODE="T$USE"
}


# directory to look for global param files if not in your $VERITAS_EVNDISP_AUX_DIR , DESY-specific, 
#GLOBALPARAMDIR="/lustre/fs5/group/cta/VERITAS/analysis/AnalysisData-VTS-v400/"   # HARDCODE
#GLOBALPARAMDIR="/lustre/fs5/group/cta/VERITAS/analysis/AnalysisData-VTS-v430/" # HARDCODE

ALLFILESGOOD=true
declare -a MISSINGFILELIST

function contains {
	local n=$#
	local value=${!n}
	for (( i=1 ; i<$# ; i++ )) ; do
		if [ "${!i}" == "${value}" ] ; then
			echo "y"
			return 0
		fi
	done
	echo "n"
	return 1
}

function addMissing {
	if [ $( contains "${MISSINGFILELIST[@]}" "$1" ) == "n" ] ; then
		MISSINGFILELIST+=("$1")
	fi
}

# search for param file, link to it if we dont have it, print errors if it doesn't exist
function huntForParameterFileName {
	local PFDIR=$1  # Subdir in $VERITAS_EVNDISP_AUX_DIR: GammaHadronCutFiles, RadialAcceptances, etc
	local PF=$2     # name of param file
	local RRRUN=$3
	if [ ! -e "$VERITAS_EVNDISP_AUX_DIR/$PFDIR/$PF" ] ; then # the file doesn't exist in our personal directory
		if [ -e "$VERITAS_EVNDISP_AUX_DIR/GlobalDir/$PFDIR/$PF" ] ; then # the file exists in the global dir, and we must link to it
			LINKTARG=$(readlink -m "$VERITAS_EVNDISP_AUX_DIR/GlobalDir/$PFDIR/$PF")
			LINKNAME="$VERITAS_EVNDISP_AUX_DIR/$PFDIR/$PF"
			echoerr "${COYEL}Warning, For run $RRRUN File $PFDIR/$PF"
			echoerr "   doesnt exist in your \$VERITAS_EVNDISP_AUX_DIR/$PFDIR ."
			echoerr "   Adding soft link to the global dir: "
			echoerr "      $LINKNAME"
			echoerr "      VVVVV"
			echoerr "      $LINKTARG ${CONORM}"
            echoerr " "
			rm -rf "$LINKNAME" ; ln -sf "$LINKTARG" "$LINKNAME" # delete the link if it exists, and make it
		else # the file doesnt exist anywhere, and the user needs to know
			echoerr "${CORED}Error: For run $RRRUN Param File Does Not Exist: \$VERITAS_EVNDISP_AUX_DIR/$PFDIR/$PF ${CONORM}"
			ALLFILESGOOD=false
			addMissing "\$VERITAS_EVNDISP_AUX_DIR/$PFDIR/$PF"
		fi
	fi
}

#echo "`basename $0`: \$\#: $#" >&2
HELPFLAG=false
ISPIPEFILE=`readlink /dev/fd/0` # check to see if input is from terminal, or from a pipe
if [[ "$ISPIPEFILE" =~ ^/dev/pts/[0-9]{1,2} ]] ; then # its a terminal (not a pipe)
	if ! [ "$#" -eq "2" ] ; then # the human didn't add the right # of arguments
		HELPFLAG=true  # and we must print help text then quickly exit
	fi
else # its a pipe, and we need to check for 3 args
	if ! [ "$#" -eq "1" ] ; then
		HELPFLAG=true
	fi
fi

# print help text and exit
if $HELPFLAG ; then
	echo
	echo "`basename $0`:"
	echo " Turn a runlist and settings into a list of Effective Area, GammaHadron, and Radial Acceptance files"
	echo "  $ `basename $0` <settingsline> <runlist>" ; echo
	echo " <settingsline> has the following format:"
	echo "    -optionname:optionval-optionname:optionval-optionname:optionval-"
	echo "    options for optionname include:"
	echo "        CUTS           : cutname"
	echo "        AUXVERRADEC    : 'auxv##', for radacc file"
	echo "        AUXVEREFFAREA  : 'auxv##', for effarea file"
	echo "        AUXVERTABLE    : 'auxv##', for table file"
	echo "        AUXFILEEVNVER  : 'v###', for event display version"
	echo "        AUXFILESIMDATE : simulation type and date, 'CARE_########' or 'GRISU_########'"
	echo "        AUXFILEMINTEL  : minimum number of telescopes"
	echo "        AUXFILESRCEXT  : source extension, either 'ExtendedSource' or 'Point'"
	echo "        AUXFILEDISP    : whether disp was used or not, 'yes' for disp, 'no' for not-disp"
	echo "        USEFROGS       : whether frogs was used or not, 'yes' for frogs, 'no' for not-frogs"
	echo
	echo " Output will have the format:"
	echo
	echo "R65742 ANASUM.GammaHadron.d20130411-cut-N3-Point-005CU-Soft.dat radialAcceptance-d20130411-cut-N3-Point-005CU-Soft-V6-T1234.root effArea-d20130411-cut-N3-Point-005CU-Soft-ATM21-V6-T1234-d20130521.root table_d20130521_GrIsuDec12_ATM21_V6_ID0"
	echo "R69538 ANASUM.GammaHadron.d20130411-cut-N3-Point-005CU-Soft.dat radialAcceptance-d20130411-cut-N3-Point-005CU-Soft-V6-T234.root effArea-d20130411-cut-N3-Point-005CU-Soft-ATM22-V6-T234-d20130521.root table_d20130521_GrIsuDec12_ATM22_V6_ID0"
	echo
	echo "where the columns are \"R<runnumber> <g/h cut file> <rad accept file> <eff area file> <table file>\""
	echo 
	echo " This program will then check that each file exists.  If a file is missing,"
	echo "    it will print to stderr in bright red text to let you know that its missing"
	echo
	echo " The recommended use is to save the output of `basename $0` to a file, "
	echo "    then grep for the line with \"R<runnumber>\", then grep for the "
	echo "    file prefix:"
	echo "       ANASUM.GammaHadron"
	echo "       radialAcceptance"
	echo "       effArea"
	echo "       table"
	echo "    to get the specific param file you want, like so: "
	echo "    $ grep R<runnumber> paramfilelist.dat | grep -oE \"\S*<fileprefix>\S*\""
	echo
	echo " Example:"
	echo "  $ generateParamFiles.sh soft 20130411 3 runlist.dat >| paramfilelist.dat"
	echo
	echo "  $ grep R65742 paramfilelist.dat | grep -oE \"\S*radialAcceptance\S*\""
	echo "  radialAcceptance-d20130411-cut-N3-Point-005CU-Soft-V6-T1234.root"
	echo
	echo "  $ grep R69553 paramfilelist.dat | grep -oE \"\S*GammaHadron\S*\""
	echo "  ANASUM.GammaHadron.d20130411-cut-N3-Point-005CU-Soft.dat"
	echo
	echo " The runlist is expected to have the format one runnumber per line:"
	echo "44322"
	echo "55840"
	echo "67627"
	echo "68221"
	echo "56471"
	echo
	echo "65742"
	echo "69538"
	echo "69553 # my comment on this run"
	echo "69689 # this run was pretty bad"
	echo "69680"
	echo
	echo "Any empty lines and any text after the run number (space separated) will be ignored."
	echo
	echo " Also, this is compatible with pipes, like so:"
	echo "  $ cat runlist.dat | whichRunsAreVersion.sh 5 | generateParamFiles.sh soft 20130411 3 > paramfilelist.dat"
	echo
	exit 0
fi

# list of run_id's to read in
RUNFILE="$2"
if [ ! -e $RUNFILE ] ; then
	echo "File $RUNFILE could not be found in $PWD , sorry." >&2 ; exit 1
fi
#RUNLIST=`cat $RUNFILE | awk '{print $1}'` won't remove empty lines
RUNLIST=`cat "$RUNFILE" | awk '!/^($|#)/{ print $1 }'` # removes empty lines and anything after the first number in each line
#echo "RUNLIST:$RUNLIST"

SETTINGLINE="$1"
#echo
#echo "SETTINGLINE:'$SETTINGLINE'" 2>&1
#
#echo

# parse settings from first argument
# SETTINGLINE must have the format:
# -OPTIONNAME:OPTIONVAL-OPTIONNAME:OPTIONVAL-OPTIONNAME:OPTIONVAL-OPTIONNAME:OPTIONVAL- etc.
ENERGYCODE=$(         echo "$SETTINGLINE" | grep -oP "~CUTS:\w+~"                        | grep -oP ":.+~" | tr -d ':' | tr -d '~')
AUXVER_RADACC=$(      echo "$SETTINGLINE" | grep -oP "~AUXVERRADEC:auxv\d+~"             | grep -oP ":.+~" | tr -d ':' | tr -d '~')
AUXVER_EFFAREA=$(     echo "$SETTINGLINE" | grep -oP "~AUXVEREFFAREA:auxv\d+~"           | grep -oP ":.+~" | tr -d ':' | tr -d '~')
AUXVER_TABLE=$(       echo "$SETTINGLINE" | grep -oP "~AUXVERTABLE:auxv\d+~"             | grep -oP ":.+~" | tr -d ':' | tr -d '~')
AUXFILE_EVNVER=$(     echo "$SETTINGLINE" | grep -oP "~AUXFILEEVNVER:v\d+~"              | grep -oP ":.+~" | tr -d ':' | tr -d '~')
AUXFILE_SIMDATE_GR=$( echo "$SETTINGLINE" | grep -oP "~AUXFILESIMDATEGR:[a-zA-Z0-9\-]+~" | grep -oP ":.+~" | tr -d ':' | tr -d '~')
AUXFILE_SIMDATE_CA=$( echo "$SETTINGLINE" | grep -oP "~AUXFILESIMDATECA:\w+~"            | grep -oP ":.+~" | tr -d ':' | tr -d '~')
AUXFILE_MINTEL=$(     echo "$SETTINGLINE" | grep -oP "~AUXFILEMINTEL:\d+~"               | grep -oP ":.+~" | tr -d ':' | tr -d '~')
AUXFILE_SRCEXT=$(     echo "$SETTINGLINE" | grep -oP "~AUXFILESRCEXT:\w+~"               | grep -oP ":.+~" | tr -d ':' | tr -d '~')
AUXFILE_DISP=$(       echo "$SETTINGLINE" | grep -oP "~AUXFILEDISP:\w+~"                 | grep -oP ":.+~" | tr -d ':' | tr -d '~')
USEFROGS=$(           echo "$SETTINGLINE" | grep -oP "~USEFROGS:\w+~"                    | grep -oP ":.+~" | tr -d ':' | tr -d '~')
USETMVABDT=$(         echo "$SETTINGLINE" | grep -oP "~USETMVABDT:\w+~"                  | grep -oP ":.+~" | tr -d ':' | tr -d '~')

#echoerr "ENERGYCODE:      '$ENERGYCODE'"
#echoerr "AUXVER_RADACC:   '$AUXVER_RADACC'"
#echoerr "AUXVER_EFFAREA:  '$AUXVER_EFFAREA'"
#echoerr "AUXVER_TABLE:    '$AUXVER_TABLE'"
#echoerr "AUXFILE_EVNVER:  '$AUXFILE_EVNVER'"
#echoerr "AUXFILE_SIMDATE_GR: '$AUXFILE_SIMDATE_GR'"
#echoerr "AUXFILE_SIMDATE_CA: '$AUXFILE_SIMDATE_CA'"
#echoerr "AUXFILE_MINTEL:  '$AUXFILE_MINTEL'"
#echoerr "AUXFILE_SRCEXT:  '$AUXFILE_SRCEXT'"
#echoerr "AUXFILE_DISP:    '$AUXFILE_DISP'"
#echoerr "USEFROGS:        '$USEFROGS'"

# test if any of the above are empty
EXITFLAG=false
if [[ -z "$ENERGYCODE" ]] ; then
	echoerr "Error, Unrecognized option in 'CUTS', exiting..."
	EXITFLAG=true
fi

if [[ -z "$AUXVER_RADACC" ]] ; then
	echoerr "Error, Unrecognized option in 'AUXVER_RADACC', exiting..."
	EXITFLAG=true
fi

if [[ -z "$AUXVER_EFFAREA" ]] ; then
	echoerr "Error, Unrecognized option in 'AUXVER_EFFAREA', exiting..."
	EXITFLAG=true
fi

if [[ -z "$AUXVER_TABLE" ]] ; then
	echoerr "Error, Unrecognized option in 'AUXVER_TABLE', exiting..."
	EXITFLAG=true
fi

if [[ -z "$AUXFILE_EVNVER" ]] ; then
	echoerr "Error, Unrecognized option in 'AUXFILE_EVNVER', exiting..."
	EXITFLAG=true
fi

if [[ -z "$AUXFILE_SIMDATE_GR" ]] ; then
	echoerr "Error, Unrecognized option in 'AUXFILE_SIMDATE_GR', exiting..."
	EXITFLAG=true
fi
if [[ -z "$AUXFILE_SIMDATE_CA" ]] ; then
	echoerr "Error, Unrecognized option in 'AUXFILE_SIMDATE_CA', exiting..."
	EXITFLAG=true
fi

if [[ -z "$AUXFILE_MINTEL" ]] ; then
	echoerr "Error, Unrecognized option in 'AUXFILE_MINTEL', exiting..."
	EXITFLAG=true
fi

if [[ -z "$AUXFILE_SRCEXT" ]] ; then
	echoerr "Error, Unrecognized option in 'AUXFILE_SRCEXT', exiting..."
	EXITFLAG=true
fi

if [[ -z "$AUXFILE_DISP" ]] ; then
	echoerr "Error, Unrecognized option in 'AUXFILE_DISP', exiting..."
	EXITFLAG=true
fi

if [[ -z "$USEFROGS" ]] ; then
	echoerr "Error, Unrecognized option in 'USEFROGS', exiting..."
	EXITFLAG=true
fi

if [[ -z "$USETMVABDT" ]] ; then
	echoerr "Error, Unrecognized option in 'USETMVABDT', exiting..."
  EXITFLAG=true
fi

if $EXITFLAG ; then
	echoerr "Error, see above problems, exiting..."
	exit 1
fi

# get database url from parameter file
EVNDISPPARAMFILE="$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/EVNDISP.global.runparameter"
MYSQLDB=`grep '^\*[ \t]*DBSERVER[ \t]*mysql://' "$EVNDISPPARAMFILE" | egrep -o '[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}'`  # extract the database url from $EVNDISPPARAMFILE
if [ ! -n "$MYSQLDB" ] ; then
    echoerr "* DBSERVER param not found in \$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/EVNDISP.global.runparameter!"
	exit 1
fi

# mysql login info
#echo "Database: $MYSQLDB"
MYSQL="mysql -u readonly -h $MYSQLDB -A"

# generate a mysql-formatted list of runs to ask for ( run_id = RUNID[1] OR run_id = RUNID[2] etc)
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
#echo "SUB:$SUB"

# Get Needed Data from VOFFLINE.tblRun_Analysis_Comments: cut_mask 
RACCMD="$MYSQL -e \"USE VOFFLINE ; SELECT run_id, tel_cut_mask FROM tblRun_Analysis_Comments WHERE $SUB\" "
#echo "RAC $ $RACCMD"
RACLINES=$( eval $RACCMD )
#echo "RACLINES:" ; echo "$RACLINES" ; echo

# Get Needed Data from VERITAS.tblRun_Info: date-of-run and telescope config_mask
RICMD="$MYSQL -e \"USE VERITAS ; SELECT run_id, data_start_time, config_mask FROM tblRun_Info WHERE $SUB\" "
#echo "RI $ $RICMD"
RILINES=$( eval $RICMD )
#echo "RILINES:" ; echo "$RILINES" ; echo

# epoch file to load
PARAMFILE="$VERITAS_EVNDISP_AUX_DIR/ParameterFiles/VERITAS.Epochs.runparameter"

# get only lines that start with '*'
EPOCHTHRESH=$( cat $PARAMFILE | grep -P "^\s??\*" | grep "EPOCH" | grep -P "V\d" )
#echo "$EPOCHTHRESH"

# find out what are the smallest and largest epochs to work with
# so we don't loop over V1, V2, V3.... V10, V11, etc
MINEPOCH=$( echo "$EPOCHTHRESH" | awk '{ print $3 }' | grep -oP "\d" | awk '{ if(min==""){min=$1}; if($1<min){min=$1};} END {print min }' )
MAXEPOCH=$( echo "$EPOCHTHRESH" | awk '{ print $3 }' | grep -oP "\d" | awk '{ if(max==""){max=$1}; if($1>max){max=$1};} END {print max }' )
#echo "EPOCHTHRESH:"
#echo "$EPOCHTHRESH"
#echo "MINEPOCH:$MINEPOCH"
#echo "MAXEPOCH:$MAXEPOCH"
DISPCODE="DISPCODE_ERROR"
METHCODE="METHCODE_ERROR"
if [[ "$AUXFILE_DISP" == "yes" ]] ; then
	#DISPCODE="DISP"
	METHCODE="DISP"
else
	#DISPCODE="GEO"
	METHCODE="GEO"
fi

# Loop over all runs in runlist
#ALLFILESGOOD=true
for i in ${RUNLIST[@]} ; do
	echo "$i"
	# figure out codes
	#NTELCODE="N$NTELARG"   # N2, N3, N4
	NTELCODE="NTel${AUXFILE_MINTEL}"   # N2, N3, N4
	
	#echo "   RACLINES:$RACLINES"
	TELCUTMASK=$( echo "$RACLINES" | grep -i "$i" | awk '{ print $2 }' )
	#echo "   TELCUTMASK:       $TELCUTMASK"
	TELCONFIGMASK=$( echo "$RILINES" | grep -i "$i" | awk '{ print $4 }' )
	#echo "   TELCONFIGMASK:    $TELCONFIGMASK"
	TELCOMBOCODE="T1"  # T1234, T123, T234, etc
	GetActiveTelComboCode $TELCONFIGMASK $TELCUTMASK
	#echo "   TELCOMBOCODE:$TELCOMBOCODE"
	
	ATMOCODE="ATM99" # ATM22 or ATM21
	DATASTARTTIMESTR=$( echo "$RILINES" | grep -i "$i" | awk '{ print $2 }' | tr -d '-' )
	#echo "   DATASTARTTIMESTR: $DATASTARTTIMESTR"
	WinterFlag=true
	WinterCode="00"
	WinterCode=$( IsWinter "$DATASTARTTIMESTR" )
	
	# old code
	#if $WinterFlag ; then ATMOCODE="ATM21" #echo "   Winter Run"
	#else                  ATMOCODE="ATM22" #echo "   Summer Run"
	#fi
	if [[ "$WinterCode" == "21" ]] ||
	   [[ "$WinterCode" == "22" ]] ; then
		ATMOCODE="ATM$WinterCode"
	else
		echoerr "${CORED}Error, for run '$i', no valid atmosphere date range defined for '$DATASTARTTIMESTR' in '$EPOCHSPARAMFILE' (WinterCode='$WinterCode'), exiting...$CONORM"
		exit 1
	fi
	#echoerr "  '$ATMOCODE'"
	
	# extra datecodes
	# HARDCODE
	#AREADATECODE="d20130521"  # second datecode in the effective area filename
	#GHCUTDATECODE="$DATECODE" # datecode at the beginning of the gamma-hadron cut filename
	#TABLEDATECODE="d20130521" # datecode at the beginning of the table filename
	
	# frogs code
	if [ "$USEFROGS" == "yes" ] ; then
		FROGSCODE="-FROGS"
	else
		FROGSCODE=""
	fi

  if [ "$USETMVABDT" == "yes" ] ; then
    TMVABDTCODE="-TMVA-BDT"
  else
    TMVABDTCODE=""
  fi

	# loop through all epochs between min and max
	for epoch in $(seq $MINEPOCH $MAXEPOCH) ; do
		
		# find out run boundaries for this
		MINRUN=$( echo "$EPOCHTHRESH" | grep -P "V$epoch" | awk '{ print $4 }' | grep -oP "\d+" )
		MAXRUN=$( echo "$EPOCHTHRESH" | grep -P "V$epoch" | awk '{ print $5 }' | grep -oP "\d+" )
		#echo "RUN:   '$run'"
		#echo "MINRUN:'$MINRUN'"
		#echo "MAXRUN:'$MAXRUN'"
		if (( "$i" <= "$MAXRUN" )) && (( "$i" >= "$MINRUN" )) ; then
			#echo "V"
			VERSIONCODE="V$epoch"
			break # break out of epoch loop, but not the runlist loop
		fi
		
	done
	#echoerr "  '$VERSIONCODE'"
	
  if [[ "$VERSIONCODE" == "V6" && "$VERSIONCODE" == "ATM21" ]] ; then
    SIMDATE="$AUXFILE_SIMDATE_CA"
  else
    SIMDATE="$AUXFILE_SIMDATE_GR"
  fi
    
	#echo "r${i}"
	
	# Example: GammaHadronCutFiles/ANASUM.GammaHadron.d20120322-cut-N2-Point-005CU-Soft.dat
	#GHCUTFILE="ANASUM.GammaHadron.d${GHCUTDATECODE}-cut-${NTELCODE}-Point-005CU-${ENERGYCODE}${FROGSCODE}.dat"
	GHCUTFILE="ANASUM.GammaHadron-Cut-${NTELCODE}-${AUXFILE_SRCEXT}-${ENERGYCODE}${TMVABDTCODE}.dat"
	huntForParameterFileName "GammaHadronCutFiles" "$GHCUTFILE" "$i"

	CUTSNAMERADACC="Cut-${NTELCODE}-${ENERGYCODE}"

	# Example: RadialAcceptances/radialAcceptance-d20130411-cut-N3-Point-005CU-Soft-V5-T234.root
	#ACCEPFILE="radialAcceptance-d${RADATECODE}-cut-${NTELCODE}-Point-005CU-${ENERGYCODE}-${VERSIONCODE}-${TELCOMBOCODE}.root"
	#ACCEPFILE="radialAcceptance-${AUXFILE_EVNVER}-${AUXVER_RADACC}-Cut-${NTELCODE}-${AUXFILE_SRCEXT}-${ENERGYCODE}-${DISPCODE}-${VERSIONCODE}-${TELCOMBOCODE}.root"
	#ACCEPFILE="radialAcceptance-${AUXFILE_EVNVER}-${AUXVER_RADACC}-$CUTSNAME-$METHCODE-${VERSIONCODE}-${TELCOMBOCODE}.root"
	#ACCEPFILE="radialAcceptance-${AUXFILE_EVNVER}-${AUXVER_RADACC}-$CUTSNAMERADACC-$METHCODE-${VERSIONCODE}-${TELCOMBOCODE}.root"
	#ACCEPFILE="radialAcceptance-${AUXFILE_EVNVER}-${AUXVER_RADACC}-${SIMDATE}-$CUTSNAMERADACC-$METHCODE-${VERSIONCODE}-${TELCOMBOCODE}.root"
  ACCEPFILE="radialAcceptance-${AUXFILE_EVNVER}-${AUXVER_RADACC}-$SIMDATE-Cut-$NTELCODE-$ENERGYCODE${TMVABDTCODE}-$METHCODE-$VERSIONCODE-$TELCOMBOCODE.root"
	huntForParameterFileName "RadialAcceptances" "$ACCEPFILE" "$i"
	
	CUTSNAME="Cut-${NTELCODE}-${AUXFILE_SRCEXT}-${ENERGYCODE}"

	# Example: EffectiveAreas/effArea-d20130411-cut-N3-Point-005CU-Soft-ATM22-V5-T1234-d20130521.root
	#echo -e "${COTYELLOW}Warning, Effective area probably depends on if we're using frogs or not.  Fix!!!!$CONORM" >&2
	#AREAFILE="effArea-d${EFDATECODE1}-cut-${NTELCODE}-Point-005CU-${ENERGYCODE}-${ATMOCODE}-${VERSIONCODE}-${TELCOMBOCODE}-d${EFDATECODE2}.root"
	#AREAFILE="effArea-${AUXFILE_EVNVER}-${AUXVER_EFFAREA}-${AUXFILE_SIMDATE}-Cut-${NTELCODE}-${AUXFILE_SRCEXT}-${ENERGYCODE}-${DISPCODE}-${VERSIONCODE}-${ATMOCODE}-${TELCOMBOCODE}.root"
	AREAFILE="effArea-${AUXFILE_EVNVER}-${AUXVER_EFFAREA}-${SIMDATE}-$CUTSNAME${TMVABDTCODE}-${METHCODE}-${VERSIONCODE}-${ATMOCODE}-${TELCOMBOCODE}.root"
	huntForParameterFileName "EffectiveAreas" "$AREAFILE" "$i"

	# Example: Tables/table_d20130521_GrIsuDec12_ATM22_V5_ID0
	#TABLEFILE="table_d${TADATECODE}_GrIsuDec12_${ATMOCODE}_${VERSIONCODE}_ID0"
	#TABLEFILE="table-${AUXFILE_EVNVER}-${AUXVER_TABLE}-${AUXFILE_SIMDATE}-${ATMOCODE}-${VERSIONCODE}-${DISPCODE}"
	TABLEFILE="table-${AUXFILE_EVNVER}-${AUXVER_TABLE}-${SIMDATE}-${ATMOCODE}-${VERSIONCODE}-${METHCODE}"
	huntForParameterFileName "Tables" "${TABLEFILE}.root" "$i"

	# output format, print to screen, for each run:
	#r<runid> <accepfile> <areafile> <ghcutfile> <tablefile>
	echo "RUN${i} $GHCUTFILE $ACCEPFILE $AREAFILE $TABLEFILE"

done

if ! $ALLFILESGOOD ; then
	echoerr ""
	echoerr "${CORED}Warning! Some needed files do not exist anywhere in \$VERITAS_EVNDISP_AUX_DIR or in the global dir `readlink -m $VERITAS_EVNDISP_AUX_DIR/GlobalDir`." >&2
	echoerr "${CORED}List of missing files:"
	
    # sort the unique missing files, for easy human reading
    REORDER="`echo ${MISSINGFILELIST[@]} | sed -e 's/ /\n/g' | sort`"
    echoerr "${COTRED}${REORDER}${CONORM}"
    
	echoerr "   Please check that the red files exist, and if not, TELL SOMEONE!!${CONORM}" >&2
	echoerr ""
	exit 1
else
    exit 0
fi

