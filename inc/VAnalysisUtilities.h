//! VAnalysisUtilities
// Revision $Id: VAnalysisUtilities.h,v 1.1.2.7.8.2.2.10.2.3.2.3 2011/04/08 13:48:13 gmaier Exp $
#ifndef VAnalysisUtilities_H
#define VAnalysisUtilities_H

#include "TCanvas.h"
#include "TDirectory.h"
#include "TF1.h"
#include "TFile.h"
#include "TGaxis.h"
#include "TH1.h"
#include "TMath.h"
#include "TObject.h"
#include "TGraph.h"
#include "TSystem.h"
#include "TTree.h"

#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "CRunSummary.h"
#include "VRunList.h"
#include "VStatistics.h"

using namespace std;

class VAnalysisUtilities : public TNamed
{
    protected:
        unsigned int fDebug;
        bool bZombie;

	//some run summary parameters
	double fSkyMapCentreDecJ2000;
	double fSkyMapCentreRAJ2000;
	double fTargetShiftWest;
	double fTargetShiftNorth;
	double fTargetDec;
	double fTargetRA; 
	double fTargetDecJ2000;
	double fTargetRAJ2000; 

	// anasum data file
        TFile *fAnasumDataFile;

// run list
        double fRunList_MJD_min;
        double fRunList_MJD_max;
        vector< VRunList > fRunList;

// phase folding
	double  fPhase_MJD0;
	double  fPhase_Period_days;

// run list cuts
        double  fRunListCut_MJD_min;
        double  fRunListCut_MJD_max;
	double  fRunListCut_Phase_min;
	double  fRunListCut_Phase_max;

        bool openEnergyThresholdFile();
	bool readTargetCoordinatesFromtRunSummary( TTree* t, int irun );

    public:

        VAnalysisUtilities();
       ~VAnalysisUtilities() {}

        bool     closeFile();
        TGraph*  calcCumulativeSig(int iTot);
        TObject* getHistogram( string, int, string );
	TChain*  getTreeWithSelectedEvents(string iFile, bool iOn);
	double   getNormalisationFactor(int iRun=-1);
	vector< VRunList >& getRunList() { return fRunList; }
        double   getRunList_MJD_min() { return fRunList_MJD_min; }
        double   getRunList_MJD_max() { return fRunList_MJD_max; }
	CRunSummary* getRunSummaryTree( int iTot );
	double   getSkyMapCentreRAJ2000()  { return fSkyMapCentreRAJ2000; }
	double   getSkyMapCentreDecJ2000() { return fSkyMapCentreDecJ2000; }
	double   getTargetShiftWest() { return fTargetShiftWest; }
	double   getTargetShiftNorth() { return fTargetShiftNorth; }
        bool     IsZombie() { return bZombie; }
        bool     openFile( string ifile, int irun = -1, bool iStereo = true, bool iPrint = true );
	void     printRunList();
        bool     readRunList();
        bool     readRunList( vector< int > irunlist, int iTot = 1 );
        void     setDebug( unsigned int iDebug = true ) { fDebug = iDebug; }
	void     setPhaseFoldingValues( double iZeroPhase_MJD = -99., double iPhase_Days = 99. ) { fPhase_MJD0 = iZeroPhase_MJD; fPhase_Period_days = iPhase_Days; }
	void     setRunListMJDRange( double iMJDMin = 0., double iMDJMax = 0. ) { fRunList_MJD_min = iMJDMin; fRunList_MJD_max = iMDJMax; }
	void     setRunListCutMJDRange( double iMJDMin = -1., double iMDJMax = -1. ) { fRunListCut_MJD_min = iMJDMin; fRunListCut_MJD_max = iMDJMax; }
	void     setRunListCutPhaseRange( double iPhaseMin = -1., double iPhaseMax = -1. ) { fRunListCut_Phase_min = iPhaseMin; fRunListCut_Phase_max = iPhaseMax; }

        ClassDef(VAnalysisUtilities,9);
};
#endif
