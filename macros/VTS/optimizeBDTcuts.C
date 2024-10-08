/* optimize TMVA cuts and print optimized cuts to screen
 *
 * requires as input gamma/hadron rates files prepared with
 * pre-cuts (e.g., ANASUM.GammaHadron-Cut-NTel2-PointSource-Moderate-TMVA-Preselection.dat)
 *
 * usage: e.g.
 * root -l -q -b 'optimizeBDTcuts.C("../../tmva/rates.root", "../../tmva", "V6", 0, 3, 0, 2)'
 *
*/

R__LOAD_LIBRARY($EVNDISPSYS / lib / libVAnaSum.so );


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
    string epoch = "V6",
    int weightFileIndex_Emin = 0, int weightFileIndex_Emax = 3,
    int weightFileIndex_Zmin = 0., int weightFileIndex_Zmax = 3.,
    double observing_time_h = 5.,
    double significance = 5.,
    double min_source_events = 10. )
{
    VTMVAEvaluator a;

    // a.setPrintPlotting( true );
    // a.setPlotEfficiencyPlotsPerBin( true );
    // conversion factor 3600.:
    // - particle number graphs are in 1/s
    // - observing time is in hours
    a.setParticleNumberFile( rateFile.c_str(), 3600. );
    // optimisation metric for the given observation time
    // - significance
    // - >= 10 signal events
    // - alpha = 1./5.
    a.setSensitivityOptimizationParameters(
        significance, min_source_events, observing_time_h, 1. / 5. );
    // maximum signal efficiency allowed
    a.setSensitivityOptimizationFixedSignalEfficiency( 0.90 );
    a.setSensitivityOptimizationMinSourceStrength( 0.00001 );

    // reading of weight files and optimisation
    ostringstream iFullWeightFileName;
    iFullWeightFileName << weightFileDir << "/BDT";
    a.initializeWeightFiles(
        iFullWeightFileName.str(),
        weightFileIndex_Emin, weightFileIndex_Emax,
        weightFileIndex_Zmin, weightFileIndex_Zmax );

    // plotting
    a.plotSignalAndBackgroundEfficiencies();

    // print results to screen
    a.printSignalEfficiency();
    a.printOptimizedMVACutValues( epoch );
}
