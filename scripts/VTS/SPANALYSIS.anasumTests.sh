# scripts to test analysis chain for many fields, cuts, etc.
#
# script used for developing code and testing
#
#  many hard wired parameters

#

# evndisp version
VERSION="v447"
# evndisp analysis date
DATE="d20140715"

# V4
SIMTYPE="GRISU"
EPOCH="V4"
LIST="/lustre/fs5/group/cta/users/maierg/VERITAS/analysis/Results/Crab/d20140624/Crab_V4.dat"


# V5
SIMTYPE="GRISU"
EPOCH="V5"
LIST="/lustre/fs5/group/cta/users/maierg/VERITAS/analysis/Results/Crab/d20140624/Crab_V5_Winter_20112012_gt65deg.dat"

# V6
SIMTYPE="CARE_June1425"
EPOCH="V6"
LIST="/lustre/fs5/group/cta/users/maierg/VERITAS/analysis/Results/Crab/d20140606//Crab_V6_gt65deg.dat"
LIST="runlist_releaseTesting"

# for C in moderate2tel moderate3tel hard2tel hard3tel superhard open
# for C in soft softExt moderateExt2tel hard2Exttel

# object
# for O in Crab M82 PKS1424p240 Tycho
for O in Tycho PKS1424p240 M82
do
    # cut
#    for C in moderate2tel
    for C in moderate2tel superhard hard2tel soft moderateopen softopen hardopen moderate3tel hard3tel softExt moderateExt2tel hard2Exttel
    do
        # reconstruction ID
        for ID in 0 1
        do 
           for D in v447
           do
              for BC in RE RB
              do
                    IDIR="$VERITAS_USER_DATA_DIR/analysis/Results/${D}/$O/${DATE}/${D}/RecID${ID}_${SIMTYPE}"
                    ODIR="$VERITAS_USER_DATA_DIR/analysis/Results/${D}/$O/${DATE}/${D}_${EPOCH}_anasum/${D}_ID${ID}_${C}_${BC}"
                    RUNPAR="$VERITAS_USER_DATA_DIR/analysis/Results/${D}/$O/${DATE}/runparameter.dat"
                    RLIST="$VERITAS_USER_DATA_DIR/analysis/Results/${D}/$O/${DATE}/${LIST}${EPOCH}.dat"
                    
                    echo $IDIR
                    echo $ODIR
                    echo $RUNPAR
                    echo $RLIST

                    rm -f $ODIR.log
               $EVNDISPSYS/bin/anasum -d $ODIR -i 1 -f $RUNPAR -l $ODIR/$C.anasum.dat -o $ODIR.root > $ODIR.log  
#                   ./ANALYSIS.anasum_parallel_from_runlist.sh ${RLIST} $ODIR $C ${BC} $RUNPAR $IDIR $SIMTYPE $ID
                   echo "DONE $ODIR"
               done
           done
        done
    done
done
