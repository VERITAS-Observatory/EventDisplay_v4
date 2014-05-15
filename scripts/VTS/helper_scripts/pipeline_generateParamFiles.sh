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

function IsWinter {
	local date="$1"
    local month="${date:4:2}"
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
        # november through april inclusive is 'winter'
        elif [ "$month" -le 4 -o "$month" -ge 11 ] ; then
            #echo 1 # winter
            WinterFlag=true
        else
            #echo 3 # unassignable
            echoerr "Error, can only assign atmosphere to runs before 20130425, exiting..." ; exit 1
        fi
	else
		WinterFlag=false
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


# directory to look for global param files if not in your $VERITAS_EVNDISP_ANA_DIR , DESY-specific, 
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
	local PFDIR=$1  # Subdir in $VERITAS_EVNDISP_ANA_DIR: GammaHadronCutFiles, RadialAcceptances, etc
	local PF=$2     # name of param file
	local RRRUN=$3
	if [ ! -e "$VERITAS_EVNDISP_ANA_DIR/$PFDIR/$PF" ] ; then # the file doesn't exist in our personal directory
		if [ -e "$VERITAS_EVNDISP_ANA_DIR/GlobalDir/$PFDIR/$PF" ] ; then # the file exists in the global dir, and we must link to it
			LINKTARG=$(readlink -m "$VERITAS_EVNDISP_ANA_DIR/GlobalDir/$PFDIR/$PF")
			LINKNAME="$VERITAS_EVNDISP_ANA_DIR/$PFDIR/$PF"
			echoerr "${COYEL}Warning, For run $RRRUN File $PFDIR/$PF"
			echoerr "   doesnt exist in your \$VERITAS_EVNDISP_ANA_DIR/$PFDIR ."
			echoerr "   Adding soft link to the global dir: "
			echoerr "      $LINKNAME"
			echoerr "      VVVVV"
			echoerr "      $LINKTARG ${CONORM}"
            echoerr " "
			rm -rf "$LINKNAME" ; ln -sf "$LINKTARG" "$LINKNAME" # delete the link if it exists, and make it
		else # the file doesnt exist anywhere, and the user needs to know
			echoerr "${CORED}Error: For run $RRRUN Param File Does Not Exist! \$VERITAS_EVNDISP_ANA_DIR/$PFDIR/$PF ${CONORM}"
			ALLFILESGOOD=false
			addMissing "\$VERITAS_EVNDISP_ANA_DIR/$PFDIR/$PF"
		fi
	fi
}

#echo "`basename $0`: \$\#: $#" >&2
HELPFLAG=false
ISPIPEFILE=`readlink /dev/fd/0` # check to see if input is from terminal, or from a pipe
if [[ "$ISPIPEFILE" =~ ^/dev/pts/[0-9]{1,2} ]] ; then # its a terminal (not a pipe)
	if ! [ "$#" -eq "9" ] ; then # the human didn't add the right # of arguments
		HELPFLAG=true  # and we must print help text then quickly exit
	fi
else # its a pipe, and we need to check for 3 args
	if ! [ "$#" -eq "8" ] ; then
		HELPFLAG=true
	fi
fi

# print help text and exit
if $HELPFLAG ; then
	echo
	echo "`basename $0`:"
	echo " Turn a runlist into a list of Effective Area, GammaHadron, and Radial Acceptance files"
	echo "  $ `basename $0` [soft|moderate|hard] <GHdatecode> <RAdatecode> <EFdatecode1> <EFdatecode2> <TAdatecode> <ntel> <runlist>" ; echo
	echo " Example:"
	echo "  $ `basename $0` soft 20130411 3 myrunlist.dat" ; echo
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
RUNFILE=$9
if [ ! -e $RUNFILE ] ; then
	echo "File $RUNFILE could not be found in $PWD , sorry." >&2 ; exit 1
fi
#RUNLIST=`cat $RUNFILE | awk '{print $1}'` won't remove empty lines
RUNLIST=`cat "$RUNFILE" | awk '!/^($|#)/{ print $1 }'` # removes empty lines and anything after the first number in each line
#echo "RUNLIST:$RUNLIST"

# energy arguement, only between 4 (soft/hard) and 8 (moderate) letters
if [[ $1 =~ ^[a-zA-Z]{4,8}$ ]] ; then
	ENERGYARG=$( echo "$1" | tr "[:upper:]" "[:lower:]" ) # force all to lowercase
	#echo "Setting ENERGYARG=$ENERGYARG"
else
	echo -e "${CORED}Error, 1st arg '$1' must be the energy cut, either 'soft', 'moderate', or 'hard'.  Exiting.${CONORM}" >&2 ; exit 1
fi

# date argument, 8-digit number
if [[ $2 =~ ^[0-9]{8}$ ]] ; then # its valid!
	#DATEARG="$2"   # 8-digit number
    GHCUTDATECODE="$2"
	#echo "Setting DATEARG=$DATEARG"
else
	echo -e "${CORED}Error, 2nd arg '$2' must be an 8-digit number, YYYYMMDD. Exiting.${CONORM}" >&2 ; exit 1
fi
if [[ $3 =~ ^[0-9]{8}$ ]] ; then # its valid!
	#DATEARG="$2"   # 8-digit number
    RADATECODE="$3"
	#echo "Setting DATEARG=$DATEARG"
else
	echo -e "${CORED}Error, 3rd arg '$2' must be an 8-digit number, YYYYMMDD. Exiting.${CONORM}" >&2 ; exit 1
fi
if [[ $4 =~ ^[0-9]{8}$ ]] ; then # its valid!
	#DATEARG="$2"   # 8-digit number
    EFDATECODE1="$4"
	#echo "Setting DATEARG=$DATEARG"
else
	echo -e "${CORED}Error, 4th arg '$2' must be an 8-digit number, YYYYMMDD. Exiting.${CONORM}" >&2 ; exit 1
fi
if [[ $5 =~ ^[0-9]{8}$ ]] ; then # its valid!
	#DATEARG="$2"   # 8-digit number
    EFDATECODE2="$5"
	#echo "Setting DATEARG=$DATEARG"
else
	echo -e "${CORED}Error, 5th arg '$2' must be an 8-digit number, YYYYMMDD. Exiting.${CONORM}" >&2 ; exit 1
fi
if [[ $6 =~ ^[0-9]{8}$ ]] ; then # its valid!
	#DATEARG="$2"   # 8-digit number
    TADATECODE="$6"
	#echo "Setting DATEARG=$DATEARG"
else
	echo -e "${CORED}Error, 6th arg '$2' must be an 8-digit number, YYYYMMDD. Exiting.${CONORM}" >&2 ; exit 1
fi

# number of telescopes, 1-digit number
if [[ $7 =~ ^[0-9]{1}$ ]] ; then
	if [[ "$7" == "2" || "$7" == "3" ]] ; then
		NTELARG="$7"   # 1-digit number
		#echo "Setting NTELARG=$NTELARG"
	else
		echo -e "${CORED}Error, 7th arg '$7' must be the number of telescopes to use, either '2' or '3'. Exiting.${CONORM}" >&2 ; exit 1
	fi
else
	echo -e "${CORED}Error, 7th arg '$7' must be a 1 digit number, the number of telescopes to use ('2' or '3'). Exiting.${CONORM}" >&2 ; exit 1
fi

# USEFROGS= yes/no
if [[ $8 =~ ^(yes|no)$ ]] ; then
	USEFROGS="$8"	
else
	echo -e "${CORED}Error, 8th arg '$8' (usefrogs?) must be either yes or no. Exiting.${CONORM}" >&2 ;
	exit 1
fi

# energy regime
ENERGYCODE="BADENERGYCODE_PLEASECHECK"
if   [[ "$ENERGYARG" == "soft"     ]] ; then ENERGYCODE="Soft"
elif [[ "$ENERGYARG" == "moderate" ]] ; then ENERGYCODE="Moderate"
elif [[ "$ENERGYARG" == "hard"     ]] ; then ENERGYCODE="Hard"     ; fi

# can only use 2 telescopes on soft cuts
if [ "$NTELARG" -eq "2" ] && [[ ! "$ENERGYARG" == "soft" ]] ; then
	echo -e "${CORED}Error, can only use 2 telescopes with soft cuts.  Exiting.${CONORM}" >&2 ; exit 1
fi

# get database url from parameter file
EVNDISPPARAMFILE="$VERITAS_EVNDISP_ANA_DIR/ParameterFiles/EVNDISP.global.runparameter"
MYSQLDB=`grep '^\*[ \t]*DBSERVER[ \t]*mysql://' "$EVNDISPPARAMFILE" | egrep -o '[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}\.[[:alpha:]]{1,20}'`  # extract the database url from $EVNDISPPARAMFILE
if [ ! -n "$MYSQLDB" ] ; then
    echo "* DBSERVER param not found in \$VERITAS_EVNDISP_ANA_DIR/ParameterFiles/EVNDISP.global.runparameter!" >&2 ; exit 1
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

# Loop over all runs in runlist
#ALLFILESGOOD=true
for i in ${RUNLIST[@]} ; do
	#echo "$i"
	# figure out codes
	#DATECODE="d$DATEARG"   # d20130411 : general datecode at the beginning of the acceptance file and effective area file
	NTELCODE="N$NTELARG"   # N2, N3, N4
	
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
	IsWinter "$DATASTARTTIMESTR" 
	if $WinterFlag ; then ATMOCODE="ATM21" #echo "   Winter Run"
	else                  ATMOCODE="ATM22" #echo "   Summer Run"
	fi
	
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

	# array version
	# HARDCODE
	if   [ "$i" -le "46641"                     ] ; then VERSIONCODE="V4"
	elif [ "$i" -ge "46642" -a "$i" -le "63372" ] ; then VERSIONCODE="V5"
	elif [                     "$i" -ge "63373" ] ; then VERSIONCODE="V6" ; fi
	
	#echo "r${i}"
	
	# Example: GammaHadronCutFiles/ANASUM.GammaHadron.d20120322-cut-N2-Point-005CU-Soft.dat
	GHCUTFILE="ANASUM.GammaHadron.d${GHCUTDATECODE}-cut-${NTELCODE}-Point-005CU-${ENERGYCODE}${FROGSCODE}.dat"
	huntForParameterFileName "GammaHadronCutFiles" "$GHCUTFILE" "$i"

	# Example: RadialAcceptances/radialAcceptance-d20130411-cut-N3-Point-005CU-Soft-V5-T234.root
	ACCEPFILE="radialAcceptance-d${RADATECODE}-cut-${NTELCODE}-Point-005CU-${ENERGYCODE}-${VERSIONCODE}-${TELCOMBOCODE}.root"
	huntForParameterFileName "RadialAcceptances" "$ACCEPFILE" "$i"

	# Example: EffectiveAreas/effArea-d20130411-cut-N3-Point-005CU-Soft-ATM22-V5-T1234-d20130521.root
	echo -e "${COTYELLOW}Warning, Effective area probably depends on if we're using frogs or not.  Fix!!!!$CONORM" >&2
	AREAFILE="effArea-d${EFDATECODE1}-cut-${NTELCODE}-Point-005CU-${ENERGYCODE}-${ATMOCODE}-${VERSIONCODE}-${TELCOMBOCODE}-d${EFDATECODE2}.root"
	huntForParameterFileName "EffectiveAreas" "$AREAFILE" "$i"

	# Example: Tables/table_d20130521_GrIsuDec12_ATM22_V5_ID0
	TABLEFILE="table_d${TADATECODE}_GrIsuDec12_${ATMOCODE}_${VERSIONCODE}_ID0"
	huntForParameterFileName "Tables" "${TABLEFILE}.root" "$i"

	# output format, print to screen, for each run:
	#r<runid> <accepfile> <areafile> <ghcutfile> <tablefile>
	echo "RUN${i} $GHCUTFILE $ACCEPFILE $AREAFILE $TABLEFILE"

done

if ! $ALLFILESGOOD ; then
	echoerr ""
	echoerr "${CORED}Warning! Some needed files do not exist anywhere in \$VERITAS_EVNDISP_ANA_DIR or in the global dir `readlink -m $VERITAS_EVNDISP_ANA_DIR/GlobalDir`." >&2
	echoerr "${CORED}List of missing files:"
	for i in "${MISSINGFILELIST[@]}" ; do
		echoerr "${COTRED}$i${CONORM}"
	done
	echoerr "   Please check that the red files exist, and if not, TELL SOMEONE!!${CONORM}" >&2
	echoerr ""
	exit 1
else
    exit 0
fi

