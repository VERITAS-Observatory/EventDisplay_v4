//! VStereoAnalysis    class for producing one dimensional histograms from parameterized VERITAS data.
//  Revision $Id: VStereoAnalysis.h,v 1.20.2.3.4.2.12.2.4.1.2.1.2.4.8.2.2.3 2010/04/01 08:34:15 gmaier Exp $

#ifndef VStereoAnalysis_H
#define VStereoAnalysis_H

#include "CData.h"

#include "VGammaHadronCuts.h"
#include "VAnaSumRunParameter.h"
#include "VAstroSource.h"
#include "VASlalib.h"
#include "VTimeMask.h"
#include "VDeadTime.h"
#include "VEffectiveAreaCalculator.h"
#include "VStereoHistograms.h"
#include "VStereoMaps.h"
#include "VSkyCoordinatesUtilities.h"
#include "VTargets.h"

#include "TDirectory.h"
#include "TDirectoryFile.h"
#include "TGraphAsymmErrors.h"
#include "TGraph2DErrors.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TList.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TTree.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

class VStereoAnalysis
{
    public:

        VStereoAnalysis( bool isOnrun, string i_hsuffix, VAnaSumRunParameter* irunpara,
	                 vector< TDirectory* > iRDir, TDirectory *iDir, string iDataDir, int iRandomSeed, bool iTotalAnalysisOnly );
        ~VStereoAnalysis() {}
        double fillHistograms( int icounter, int irun, double AzMin, double AzMax, double iPedVar );
        TH2D  *getAlpha();
        TH2D  *getAlphaUC();
        TH2D  *getAlphaNorm();
        TH2D  *getAlphaNormUC();
        double getDeadTimeFraction();
        TH1D  *getEnergyHistogram();
        TList *getEnergyHistograms();
        double getMeanAzimuth() { return fMeanAzimuth; }
        double getMeanElevation() { return fMeanElevation; }
        double getRawRate();                      //! return number of entries in rate histograms
        TH1D  *getTheta2();
        TList *getHisList();
        vector< double > getRateCounts();
        vector< double > getRateTime();
        vector< double > getRateTimeIntervall();
        TList *getParameterHistograms();
        TList *getSkyHistograms( bool bUC );
        double getWobbleNorth() { return fWobbleNorth; }
        double getWobbleWest() { return fWobbleWest; }
        TTree* getTreeWithSelectedEvents() { return fTreeSelectedEvents; }
        void   scaleAlpha( double inorm, TH2D *hon, TH2D *h_ON, TH2D *h_OFF, bool buc, int incounter);
        void   setAlphaOff( TH2D *ih, bool bUncorrelated );
        void   setAlphaOff( TH2D *ih, TH2D *ihUC );
        void   setCuts( sRunPara iL, int irun );
        void   setNoSkyPlots( bool iS ) { fNoSkyPlots = iS; }
        void   setRunTimes();
        string setRunTimes( CData *iData );
        bool   terminate();
        void   useScaledCuts( bool ib ) { fScaledCuts = ib; }
        void   writeDebugHistograms();
        void   writeHistograms( bool bOn );

        TH2D* getStereoSkyMap();
        TH2D* getStereoSkyMapUC();
        double getTargetRA();
        double getTargetDec();

        map< int, double > getRunMJD() const { return fRunMJD; }
        double getMJD( int i_run ) { return ( fRunMJD.find( i_run ) != fRunMJD.end() ? fRunMJD[i_run] : 0. ); }
        map< int, double > getRunDuration() const { return fRunDuration; }
        double getEffectiveExposure( int i_run ) { return ( fRunExposure.find( i_run ) != fRunExposure.end() ? fRunExposure[i_run] : 0.); }

        void setRunMJD( map< int, double > iRunMJD ) { fRunMJD = iRunMJD; }

    private:

        bool fDebug;
        double degrad;
	int fTimeBinCounter; // counter for EffArea Time BINs

        bool bTotalAnalysisOnly;

        bool bSimulate;

        bool fIsOn;
        bool fNoSkyPlots;                         //! do full sky plots (if false, analysed source region only)

        double fMeanSourceDec;
        double fMeanSourceRA;

        TRandom3 *fRandom;

        VAnaSumRunParameter *fRunPara;

        TGraphAsymmErrors *gMeanEffectiveAreaErec;
        TGraphAsymmErrors *gMeanEffectiveAreaEmc;
	TGraph2DErrors    *gTimeBinnedMeanEffectiveAreaErec;
        TGraph2DErrors    *gTimeBinnedMeanEffectiveAreaEmc;
        TGraphErrors      *gMeanEsys_MC;

        VStereoMaps* fMap;
        VStereoMaps* fMapUC;
        int fHisCounter;
        map< int, double > fRunMJDStart;
        map< int, double > fRunMJDStopp;
        map< int, double > fRunMJD;               // Default value is mid-point; modified to mean time of accepted events by fillHistograms
// If fRunMJD is defined from a VRunSummary.fRunMJD it will contain extra ON/OFF runs
// because the VRunSummary.fRunMJD is a union of all the runs:
// always access via the run index to be sure of getting the correct run
        map< int, double > fRunDuration;          // Raw run length from data tree
        map< int, double > fRunExposure;          // Open portion of time mask
        vector< VStereoHistograms* > fHisto;
        VStereoHistograms* fHistoTot;

        TTree *fTreeSelectedEvents;
	int    fTreeSelected_runNumber;
	int    fTreeSelected_eventNumber;
	int    fTreeSelected_MJD;
	double fTreeSelected_Time;
	int    fTreeSelected_NImages;
	ULong64_t fTreeSelected_ImgSel;
	double fTreeSelected_theta2;
	double fTreeSelected_Xoff;
	double fTreeSelected_Yoff;
	double fTreeSelected_Xcore;
	double fTreeSelected_Ycore;
	double fTreeSelected_MSCW;
	double fTreeSelected_MSCL;
	double fTreeSelected_MWR;
	double fTreeSelected_MLR;
	double fTreeSelected_Erec;
	double fTreeSelected_EChi2;
	double fTreeSelected_ErecS;
	double fTreeSelected_EChi2S;
	float fTreeSelected_EmissionHeight;
	float fTreeSelected_EmissionHeightChi2;

        double fTotCount;

	map < int, double > f_t_in_s_min;
	map < int, double > f_t_in_s_max;
        double fMeanAzimuth;
        double fMeanElevation;
        double fNMeanElevation;

        double fWobbleNorth;
        double fWobbleWest;

        bool fScaledCuts;                         //!< if true, used mean scaled cuts, otherwise mean cuts

        int  getDataRunNumber() const;            // Check for existence of fDataRun and try to retrieve run number from first entry of the tree
        CData *fDataRun;
	TTree *fDataRunTree;
	TTree *fDataFrogsTree;
        TFile *fDataFile;

        vector< VAstroSource* > fAstro;           //!< Astronomical source parameters for this analysis
        VGammaHadronCuts* fCuts;                          //!< Parameter Cuts
        VTimeMask* fTimeMask;                     //!< Time Cuts

// dead time calculators
        vector< VDeadTime* > fDeadTime;

// rate counters
        vector< vector< double > > fRateCounts;
        vector< vector< double > > fRateTime;
        vector< vector< double > > fRateTimeIntervall;
        vector< double > fRateCountsTot;
        vector< double > fRateTimeTot;
        vector< double > fRateTimeIntervallTot;

// directories
        TDirectory *fDirTot;
        vector< TDirectory* > fDirTotRun;

        double combineHistograms();
        void   defineAstroSource();
        bool   closeDataFile();
        CData* getDataFromFile( int i_runNumber );

	void fill_TreeWithSelectedEvents( CData* );
	bool init_TreeWithSelectedEvents( int, bool );
	void reset_TreeWithSelectedEvents();
};
#endif
