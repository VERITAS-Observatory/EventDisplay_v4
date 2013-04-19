//! VFluxCalculation calculate fluxes and upper flux limits
// Revision $Id: VFluxCalculation.h,v 1.1.2.6.8.1.2.1.4.3 2011/04/08 13:48:13 gmaier Exp $
#ifndef VFLUXCALCULATION_H
#define VFLUXCALCULATION_H

#include "TCanvas.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TGraphErrors.h"
#include "TGraph2DErrors.h"
#include "TLine.h"
#include "TMath.h"
#include "TObject.h"
#include "TTree.h"

#include "CRunSummary.h"
#include "CEffArea.h"

#include "VDifferentialFlux.h"
#include "VStatistics.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

class VXRayData : public TObject
{
    private:

        bool bDebug;

    public:
// X-ray data
        vector< double > fMJD;
        vector< double > fPhase;
        vector< double > fPhi;
        vector< double > fFlux;
        vector< double > fFluxEup;
        vector< double > fFluxEdown;

        TGraphAsymmErrors *gFluxPhase;
        TGraphAsymmErrors *gFluxMJD;

        VXRayData();
        ~VXRayData() {}
        TGraphAsymmErrors* getFluxPhase() { return gFluxPhase; }
        TGraphAsymmErrors* getFluxMJD() { return gFluxMJD; }
        bool  readFile( string ifile, string tname = "RXTE", double iMJDmin = -1., double iMJDmax = -1. );
        void  reset();

        ClassDef(VXRayData,1);
};

class VFluxCalculation : public TObject
{
    private:
        vector< TFile* > fFile;
        bool bZombie;                             //!< no file or invalid file connected
        CRunSummary* fData;

        bool fDebug;

	bool fTimebinned;

// input parameters read from anasum file (from run summary tree)
        vector< double > fRunList;                //!< run number
        vector< double > fRunMJD;                 //!< MJD
        vector< double > fRunTOn;                 //!< life time [s]
        vector< double > fRunDeadTime;            //!< dead time fraction
        vector< double > fRunZe;                  //!< mean zenith angle [deg]
        vector< double > fRunWobbleOffset;        //!< wobble offset [deg]
        vector< double > fRunPedvars;             //!< pedvars
        vector< double > fRunNdiff;               //!< N_on - alpha * N_off
	vector< vector< double > > fIntraRunNdiff; // time binned intra run light curves
        vector< double > fRunNdiffE;              //!< error in N_on - alpha * N_off
	vector< vector< double > > fIntraRunNdiffE; 
        vector< double > fRunRate;                //!< gamma-ray rate in [1/min]
        vector< double > fRunRateE;               //!< error in gamma-ray rate in [1/min]
        vector< double > fRunNon;                 //!< N_on
	vector< vector< double > > fIntraRunNon;
        vector< double > fRunNoff;                //!< N_off
	vector< vector< double > > fIntraRunNoff;
        vector< double > fRunNorm;                //!< alpha
        vector< double > fRunSigni;               //!< significance
	vector< vector< double > > fIntraRunSigni;
        vector< double > fRunUFL;                 //!< upper flux limit in events (-99. if not set)
	vector< vector< double > > fIntraRunUFL;
	vector< double > fRunCI_lo_1sigma;        //!< counts: lower value of 1 sigma confidence interval
	vector< vector< double > > fIntraRunCI_lo_1sigma;
	vector< double > fRunCI_up_1sigma;        //!< counts: upper value of 1 sigma confidence interval
	vector< vector< double > > fIntraRunCI_up_1sigma;
	vector< double > fRunCI_lo_3sigma;        //!< counts: lower value of 3 sigma confidence interval
	vector< vector< double > > fIntraRunCI_lo_3sigma;
	vector< double > fRunCI_up_3sigma;        //!< counts: upper value of 3 sigma confidence interval
	vector< vector< double > > fIntraRunCI_up_3sigma;
	
// intermediate results
        vector< double > fRunEffArea;             //!< normalize effective area
	vector < vector< double > > fIntraRunEffArea;             
        vector< double > fRunFluxConstant;        //!< flux constant or upper flux limit constant per run
	vector< vector< double > > fIntraRunFluxConstant;
        vector< double > fRunFluxConstantE;       //!< error in flux constant or upper flux limit constant per run
	vector< vector< double > > fIntraRunFluxConstantE;
        vector< double > fRunFlux;                //!< flux or upper flux limit per run
	vector< vector< double > > fIntraRunFlux;
        vector< double > fRunFluxE;               //!< error in flux or upper flux limit per run
	vector< vector< double > > fIntraRunFluxE;
	vector< double > fRunFluxCI_lo_1sigma;    //!< flux: lower value of 1 sigma confidence interval
	vector< vector< double > > fIntraRunFluxCI_lo_1sigma;
	vector< double > fRunFluxCI_up_1sigma;    //!< flux: upper value of 1 sigma confidence interval
	vector< vector< double > > fIntraRunFluxCI_up_1sigma;
	vector< double > fRunFluxCI_lo_3sigma;    //!< flux: lower value of 3 sigma confidence interval
	vector< vector< double > > fIntraRunFluxCI_lo_3sigma;
	vector< double > fRunFluxCI_up_3sigma;    //!< flux: upper value of 3 sigma confidence interval
	vector< vector< double > > fIntraRunFluxCI_up_3sigma;

// spectral parameters (assuming power law)
        double fMinEnergy;                        //!< calculate flux limit above this energy [TeV]
        double fMaxEnergy;                        //!< maximum energy to be taken into account [TeV]
        double fE0;                               //!< calculate flux at this energy [TeV]
        double fAlpha;                            //!< assumed spectral index


// significance and upper flux limit parameters
        int    fLiMaEqu;
        double fThresholdSignificance;
        double fMinEvents;
        double fUpperLimit;
        int    fUpperLimitMethod;

// X-ray data
        VXRayData *fRXTE;

// graphs
	TGraphErrors *gFluxElevation;
	TCanvas *fCanvasFluxesVSMJD;
	TCanvas *fCanvasFluxesInBINs;

        void   calculateFluxes();
        void   calculateSignificancesAndUpperLimits();
        void   cleanRunList();
        void   closeFiles();
        double getFluxInCrabUnits( double iF, double iE, double iGamma = 2.49 );
        double getFluxInErgs( double iF, double iE );
        void   getIntegralEffectiveArea();
        void   getNumberOfEventsAboveEnergy( double iMinEnergy );
	double integrateEffectiveAreaInterval( double x0, double x1, double x2, double ieff_mean );
        bool   openDataFile( string ifile );
        bool   openDataFile( vector< string > ifile );
        void   reset();
        void   resetRunList();
        void   writeTexFileForFluxValues( string, vector< int > iMJD, vector< double > iFlux, vector< double > iFluxE, double iFac, string iL1, string iL2 );

    public:

        VFluxCalculation();
        VFluxCalculation( string ifile, unsigned int iTot = 1, int iRunMin = 0, int iRunMax = 100000, double iMJDMin = -99., double iMJDMax = -99., bool iDebug = false );
        VFluxCalculation( vector< string > ifile, unsigned int iTot, int iRunMin = 0, int iRunMax = 100000, double iMJDMin = -99., double iMJDMax = -99. );
       ~VFluxCalculation();

        void          calculateIntegralFlux( double iMinEnergy_TeV );
	TCanvas*      getFluxesVSMJDCanvas() { return fCanvasFluxesVSMJD; }
        TGraphErrors* plotFluxesVSMJD( char *iTex = 0, double iMJDOffset = 0., TCanvas *c = 0, int iMarkerColor = 1, int iMarkerStyle = 8, bool bDrawAxis = false, double iMinMJD = -1., double iMaxMJD = -1. );
	TGraphErrors* plotFluxesInBINs( int run = -1, char *iTex = 0, double iMJDOffset = 0., TCanvas *c = 0, int iMarkerColor = 1, int iMarkerStyle = 8, bool bDrawAxis = false );
        TGraphErrors* plotFluxesVSMJDDaily( char *iTex = 0, double iMJDOffset = 0. );
        TGraphErrors* plotFluxesVSElevation( bool iDraw = true );
        void          plotFluxesVSPedvars();
        void          plotFluxesVSWobbleOffset();
	bool          IsInRunList( int iRun );
        bool          IsZombie() {return bZombie; }
        void          getFlux( int irun, double& iFlux, double& iFluxE, double& iFluxUL );
	void          getFluxConfidenceInterval( int irun, double& iFlux_low, double& iFlux_up, bool b1Sigma = true );
        vector< double > getFlux() { return fRunFlux; }
        vector< double > getFluxError() { return fRunFluxE; }
        vector< double > getMJD() { return fRunMJD; }
        double           getMJD( int iRun );
        vector< double > getRunList() { return fRunList; }
	vector< double > getTOn() { return fRunTOn; }
        double        getFluxVsCrab( double iF, double iE, double iGamma = 2.49 ) { return getFluxInCrabUnits( iF, iE ,iGamma) * 100.; }
	unsigned int  getIndexOfRun( int iRun );
        double        getRate( int iRun );
        double        getRateError( int iRun );
        double        getNOn( int irun );
        double        getNOff( int irun );
        double        getAlpha( int irun );
	double        getRunElevation( int irun );
        double        getRunTime( int irun );
        TGraphAsymmErrors* getRXTEvsMJD() { if( fRXTE ) return fRXTE->gFluxMJD; else return 0; }
        TGraphAsymmErrors* getRXTEvsPhase() { if( fRXTE ) return fRXTE->gFluxPhase; else return 0; }
        double        getSignificance( int irun );
        unsigned int  loadRunList( int iRunMin = 0, int iRunMax = 100000, unsigned int iTot = 1, double iMJDMin = -99., double iMJDMax = -99. );
        void          printResults();
	void          printRunList();
        void          printDebugSummary();
        bool          readRXTE( string ifile );
        void          setDebug( bool iB ) { fDebug = iB; }
        void          setSpectralParameters( double iMinEnergy_TeV = 0., double E0 = 1., double alpha = -2.5 );
        void          setSignificanceParameters( double iThresholdSignificance = 3., double iMinEvents = 5, 
	                                         double iUpperLimit = 0.99, int iUpperlimitMethod = 0, int iLiMaEqu = 17 );
        void          setTimeBinnedAnalysis( bool iB = true ) { fTimebinned = iB; }
        void          writeResults( char *ifile );

        ClassDef(VFluxCalculation,11);
};
#endif
