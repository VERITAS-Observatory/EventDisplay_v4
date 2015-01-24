/* \file optimizeBDTcuts.C
 *
 * \brief optimize BDT cuts
 *
 * \author Maria Krause
 *
 */

#include <iostream>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TLine.h"
#include "TMath.h"
#include "TStyle.h"
#include "TTree.h"

using namespace std; 

void help()
{
        cout << endl;
        cout << "optimize the BDT cuts" << endl;
        cout << "------------------------------------------------------------------------------" << endl;
        cout << endl;
}

void optimizeBDTcuts( char* particleraterootfile, string weightFileDir, double observing_time = 20., double min_sourcestrength = 0.00001, int weightFileIndex_Emin = 0, int weightFileIndex_Emax = 3, int weightFileIndex_Zmin = 0., int weightFileIndex_Zmax = 3., double timeparticlerate = 3600., double significance = 5., int min_events = 10, double min_backgroundrateratio = 1./5., double min_backgroundevents = 0., double signalefficiency = 0.90, double energyStepSize = -1)
{
	
	VTMVAEvaluator a;
	a.setPlotEfficiencyPlotsPerBin( false );
	a.setParticleNumberFile( particleraterootfile, timeparticlerate );
	a.setSensitivityOptimizationParameters( significance, min_events, observing_time, min_backgroundrateratio, min_backgroundevents);  
	a.setSensitivityOptimizationFixedSignalEfficiency( signalefficiency );
	a.setSensitivityOptimizationMinSourceStrength( min_sourcestrength );  
	ostringstream iFullWeightFileName;
	iFullWeightFileName << weightFileDir << "/BDT";
	a.initializeWeightFiles( iFullWeightFileName.str().c_str(), weightFileIndex_Emin, weightFileIndex_Emax, weightFileIndex_Zmin, weightFileIndex_Zmax, energyStepSize, "VTS" );

}
