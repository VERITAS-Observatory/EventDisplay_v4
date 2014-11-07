# scripts to test analysis chain for many fields, cuts, etc.
#
# script used for developing code and testing
#
#  many hard wired parameters

# evndisp version
VERSION="v470"
# evndisp analysis date
DATE="d20140801"
# run list
LIST="runlist_releaseTesting"
# evndisp run parameter files
ERUN="EVNDISP.reconstruction.runparameter.DISP"
ERUN="EVNDISP.reconstruction.runparameter"
# reconstruction methods (should match IDs)
RECMETHODS=( "GEO" "DISP" )
# ANALYSIS METHOD
ANAMETHOD="GEO"
ANAMETHOD="FROGS"

# V4
SIMTYPE="GRISU-SW6"
EPOCH="V4"

# V5
SIMTYPE="GRISU-SW6"
EPOCH="V5"

# V6
SIMTYPE="CARE_June1425"
EPOCH="V6"

# PKS1424p240: V4, V5, V6
# M82: V4, V5, V6
# Tycho: V6
# Crab: V4, V5, V6
# SS 433: V4, V5

# object
# for O in Crab M82 PKS1424p240 Tycho
# for O in Tycho PKS1424p240 M82
# for O in Tycho PKS1424p240 W44 SS433
# for O in M82 PKS1424p240 Crab W44 SS433
for O in Crab
do
    RLIST="$VERITAS_USER_DATA_DIR/analysis/Results/${VERSION}/$O/${DATE}/${LIST}${EPOCH}.dat"
    ODIR="$VERITAS_USER_DATA_DIR/analysis/Results/${VERSION}/$O/${DATE}/${VERSION}"
    mkdir -p $ODIR
    ########### START EVNDISP
#    ./ANALYSIS.evndisp.sh $RLIST $ODIR $ERUN
#    continue
    ########### END EVNDISP
    # reconstruction ID
    for ID in 0
    do
        if [[ $ANAMETHOD = "GEO" ]] || [[ $ANAMETHOD = "DISP" ]]
        then
            IDIR="$VERITAS_USER_DATA_DIR/analysis/Results/$VERSION/${O}/${DATE}/${D}/${VERSION}/RecID${ID}_${SIMTYPE}"
        else
            IDIR="$VERITAS_USER_DATA_DIR/analysis/Results/$VERSION/${O}/${DATE}/${D}/${VERSION}_Frogs/"
        fi
        RM=${RECMETHODS[$ID]}
        ########### START MSCW
#        ./ANALYSIS.mscw_energy.sh table-$VERSION-auxv01-${SIMTYPE}-ATM21-${EPOCH}-${RM} $RLIST $ODIR $ID $IDIR
#        continue
        ########### END MSCW
        # cut
#        for C in BDT-moderate moderate2tel moderate3tel hard2tel soft
#        for C in moderate2tel moderate3tel hard2tel soft 
        for C in frogs
        do 
#              for BC in RE RB
              for BC in RE
              do
                    ODIR="$VERITAS_USER_DATA_DIR/analysis/Results/$VERSION/${O}/${DATE}/${VERSION}_${EPOCH}_anasum_${ANAMETHOD}/${ANAMETHOD}_ID${ID}_${C}_${BC}"
                    RUNPAR="$VERITAS_USER_DATA_DIR/analysis/Results/$VERSION/${O}/${DATE}/runparameter.dat"
                    
                    echo $IDIR
                    echo $ODIR
                    echo $RUNPAR
                    echo $RLIST

                    rm -f $ODIR.log
#                    $EVNDISPSYS/bin/anasum -d $ODIR -i 1 -f $RUNPAR -l $ODIR/$C.anasum.dat -o $ODIR.root > $ODIR.log
                   ./ANALYSIS.anasum_parallel_from_runlist.sh ${RLIST} $ODIR $C ${BC} $RUNPAR $IDIR $SIMTYPE $ANAMETHOD 21
                   echo "DONE $ODIR"
               done
        done
    done
done
