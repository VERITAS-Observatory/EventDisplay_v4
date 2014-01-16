#ifndef VLowGainCalibrator_h
#define VLowGainCalibrator_h

#include "TCanvas.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TMath.h"
#include "TRandom.h"
#include "TTree.h"
#include "TMarker.h"

#include "TSystem.h"
#include "TH1.h"
#include "TH2F.h"
#include "TF1.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TText.h"
#include "TString.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "TSpectrum.h"
#include "TVirtualFitter.h"
#include "TMinuit.h"


using namespace std;

class VLowGainCalibrator {

//	private:
	public:

	enum status { GOOD, NO_POINTS, BAD_CHI2, NOT_PROPORTIONAL };
	bool fDEBUG;
	

	const static int fNTel=4;
	const static int fNPix=500;	
	int fWindow;	

	int fRun;

	TTree * fDsttree;	

	int fChanMon_start;
	int fChanMon_stop;
	int fChan_start;
	int fChan_stop;

	int fNLiveMonitor_min;		//min number of good monitor channels to use the events
	bool fUseMedian;		//use median or mean of monitor charges in one event
	double fSumMonitor_min;		//min sum to be considered a monitor channel
	double fLightLevelWidth;	//Width of light levels (in terms of sigma)
	

	double fFitPure_min;		//min. purity of a light level to be considered for the fit. Eg 0.8 -> 80% of events have to be in high gain/low gain.
	double fFitNPoints_min;		//min. number of points for the fit (high/low is fitted separately)
	double fFitProb_min;		//min. fit probability assuming chi2 distribution.
	double fFitB_max;		//max of abs. value of b (if fitting a*x+b, not used currently).

	TH1D * fMonitorChargeHist[fNTel];	
	unsigned int fNLightLevels[fNTel];

	vector<double> fLightLevelMean[fNTel];
	vector<double> fLightLevelSigma[fNTel];
	vector<double> fLightLevelMeanError[fNTel];
	vector<double> fLightLevelSigmaError[fNTel];

	vector<int> fN[fNTel][fNPix][2];	//number of entries per level, per tel, per pixel, per high/low gain.
	
	vector<double> fX[fNTel][fNPix][2];
	vector<double> fY[fNTel][fNPix][2];	
	vector<double> fX2[fNTel][fNPix][2];
	vector<double> fY2[fNTel][fNPix][2];
	vector<double> fXY[fNTel][fNPix][2];
	
	double mHi[fNTel][fNPix] ;
	double mLo[fNTel][fNPix] ;


	TFile * fOutfile[fNTel];
	TFile * fInfile;

	TTree * fOuttree[fNTel];
	int fTree_Channel;
	double fTree_m[2];
	double fTree_mErr[2];
	double fTree_chi2[2];
	int fTree_ndf[2];
	status fTree_status[2];

	TTree * fDebugtree[fNTel];
	int fTree_eventNumber;
	double fTree_QMon;
	double fTree_QMonMean;
	int fTree_level;
	double fTree_Q;
	int fTree_hilo;
	
	vector<double> fLMult;
	vector<int> fDebugChannels;

	bool isNewPixel( int tel, int iChan);


	unsigned int eventNumber;
	float sum[4][2900];
	float sum2[4][2900];
	UShort_t sumfirst[4][2900];
	UShort_t HiLo[4][2900];
	UShort_t sumwindow[4][2900];
	UShort_t dead[4][2900];

	double calcMonitorCharge(int tel, int ientry=-1);
	double calcMeanMonitorCharge(int tel , int ientry=-1);
	double calcMedianMonitorCharge(int tel , int ientry=-1);

//	public:
	VLowGainCalibrator(int run, int sw, bool isInnerHigh, TString dir= "./"); 
	~VLowGainCalibrator();

	void setDebug( bool debug=true ) { fDEBUG = debug ; }  
	void resetLightLevels( );
	void resetLightLevels( int tel);
	void setMonitorChargeOptions( int nLive_min=100, double sum_min=-100, bool useMedian=true, double width=2) ;
	void setFitOptions( int n_min=2, double pure_min=0.8, double prob_min=0.01, double b_max=2.0 );
	bool fIsOk();
  
	bool makeMonitorChargeHists( );
	int  checkLightLevels( int tel, bool iDraw=false );
	bool calculateMeanCharges();

	bool doTheFit( ) ;

	bool terminate(  );
	void findLightLevels( int tel, int iPeakSignificance=2 , bool iDraw=false );
	bool findLightLevels(bool iDraw=false );
	void setLowGainMultiplierUsedInDST(double lmult=6.0 ) { fLMult.assign(fNTel, lmult); } 

	void setDebugChannels( vector<int> channels )		{ fDebugChannels=channels ; }
	void addDebugChannel( int channel )			{ fDebugChannels.push_back(channel); }
	void setAllDebugChannels() 				{ fDebugChannels.clear(); for(int i=fChan_start; i<fChan_stop; i++) fDebugChannels.push_back(i); }
	bool isDebugChannel(int channel )			{ return std::find( fDebugChannels.begin(), fDebugChannels.end(), channel ) != fDebugChannels.end(); }   
	ClassDef(VLowGainCalibrator,3);


};

#endif
