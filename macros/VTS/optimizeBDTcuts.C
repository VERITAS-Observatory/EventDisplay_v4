/* optimize TMVA cuts and print optimized cuts to screen
 *
 * requires as input gamma/hadron rates files prepared with
 * pre-cuts (e.g., ANASUM.GammaHadron-Cut-NTel2-PointSource-Moderate-TMVA-Preselection.dat)
 *
 * usage: e.g.
 * root -l -q -b 'optimizeBDTcuts.C("../../tmva/rates.root", "../../tmva", 0, 3, 0, 1)'
 *
*/

R__LOAD_LIBRARY( $EVNDISPSYS/lib/libVAnaSum.so );


void help()
{
	cout << endl;
	cout << "optimize BDT cuts" << endl;
	cout << "------------------------------------------------------------------------------" << endl;
	cout << " indicate particle rate file (.root) and weight file directory" << endl;
}


void optimizeBDTcuts(
	string rateFile = "rates.root",
	string weightFileDir = "./",
	int weightFileIndex_Emin = 0, int weightFileIndex_Emax = 3,
	int weightFileIndex_Zmin = 0., int weightFileIndex_Zmax = 3.,
	double observing_time_h = 10.,
	double significance = 5. )
{
	VTMVAEvaluator a;
	
	// parameter setting
	a.setPrintPlotting( true );
	a.setPlotEfficiencyPlotsPerBin( true );
	a.setParticleNumberFile( rateFile.c_str(), 3600. );
	a.setSensitivityOptimizationParameters( significance, 10., observing_time_h, 1. / 5. );
	a.setSensitivityOptimizationFixedSignalEfficiency( 0.90 );
	a.setSensitivityOptimizationMinSourceStrength( 0.00001 );
	
	// reading of weight files and optimisation
	ostringstream iFullWeightFileName;
	iFullWeightFileName << weightFileDir << "/BDT";
	a.initializeWeightFiles(
		iFullWeightFileName.str(),
		weightFileIndex_Emin, weightFileIndex_Emax,
		weightFileIndex_Zmin, weightFileIndex_Zmax,
		-1, "VTS" );
		
	// plotting
	a.plotSignalAndBackgroundEfficiencies();
	
	// print results to screen
	a.printSignalEfficiency();
	a.printOptimizedMVACutValues( "V6" );
}
