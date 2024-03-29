anasum - make sky plots, fill energy spectra etc.
=================================================
(VERITAS analysis only)

required options are:
	-l --runlist       [run list file name, see runlist.dat for an example]
	-d --datadir       [directory for input ROOT files (from mscw_energy)]
	-o --outfile       [output ROOT file name (default output.root)]
	-f --parameterfile [analysis parameter file, default=ANASUM.runparameter]

optional options are:
	-i --runType       [type of input data: 0 (default) = mscw files; 1 = anasum single run result files]
	-u --infile        [read anasum outputfile, do the calculations for new runs and redo combined plots (data directory with option -l is required)]
	-r --randomseed    [seed for random generator, default=17]

--------------------------------------------------------

Example:

   standard full stereo analysis:

	 $EVNDISPSYS/bin/anasum -l runlist.dat -d data/Crab/ -o crab_analysis.root -f ANASUM.runparameter

   script to run analysis in parallel:

         $EVNDISPSYS/scripts/VTS/VTS.ANASUM.sub_analyseParallel_data.sh

--------------------------------------------------------

Lists and parameter files:

   analysis parameter file with binning, coordinates of sky map pointings, exclusion regions etc:

	see example $VERITAS_EVNDISP_AUX_DIR/ParameterFiles/ANASUM.runparameter

   run list:

        see example $VERITAS_EVNDISP_AUX_DIR/ParameterFiles/ANASUM.runlist

   definition of gamma/hadron cuts:
	
	see example $VERITAS_EVNDISP_AUX_DIR/ParameterFiles/ANASUM.GammaHadron.dat

	Please check VERITAS analysis wiki for the standard cut files

   list of time cuts (to remove an arbitrary time interval during a run):

	see example $VERITAS_EVNDISP_AUX_DIR/ParameterFiles/ANASUM.timemask.dat

	The time cut file is defined in the analysis parameter file (keyword TIMEMASKFILE)

--------------------------------------------------------

Required instrument response function files:

   effective areas, see $VERITAS_EVNDISP_AUX_DIR/EffectiveAreas

   radial acceptances, see $VERITAS_EVNDISP_AUX_DIR/RadialAcceptances
	
--------------------------------------------------------

use the plotting tools in the shared library to plot the results:

         on the root command line do:

	 .L $EVNDISPSYS/lib/libVAnaSum.so

	 VPlotAnasumHistograms a("crab_analysis.root");
	 a.help();
	 a.plot_radec();

	 VEnergySpectrum b("crab_analysis.root");
	 b.plot();
