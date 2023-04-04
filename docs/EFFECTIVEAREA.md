makeEffectiveArea - determination of effective areas and instrument response functions
======================================================================================

Executable: $EVNDISPSYS/bin/makeEffectiveArea

Input data is in the form of result file from the lookup table analysis ( *.mscw.root files )

Command line syntax:

    makeEffectiveArea <runparameter file> <name of output effective area ROOT file>

---------------------------------------------------

Required auxiliary files:

   file with analysis run parameters
       see example $VERITAS_EVNDISP_AUX_DIR/Parameterfiles/EFFECTIVEAREA.runparameter

   definition of gamma/hadron cuts
       see example $VERITAS_EVNDISP_AUX_DIR/ParameterFiles/ANASUM.GammaHadron.dat

---------------------------------------------------

for efficient usage, see scripts for typical usage:

[VTS] $EVNDISPSYS/scripts/VTS/VTS.EFFAREA.sub_analyse.sh and $EVNDISPSYS/scripts/VTS/VTS.EFFAREA.qsub_analyse.sh

