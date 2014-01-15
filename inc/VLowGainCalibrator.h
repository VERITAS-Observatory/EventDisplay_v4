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

	int iChanMon_start;
	int iChanMon_stop;
	int iChan_start;
	int iChan_stop;

	int fNLiveMonitor_min;
	bool fUseMedian;
	double fSumMonitor_min;

	double fFitPure_min;
	double fFitNPoints_min;	
	double fFitProb_min;
	double fFitB_max;

	TH1D * fMonitorChargeHist[fNTel];	
	unsigned int fNLightLevels[fNTel];

	vector<double> fLightLevelMean[fNTel];
	vector<double> fLightLevelWidth[fNTel];
	vector<double> fLightLevelMeanError[fNTel];
	vector<double> fLightLevelWidthError[fNTel];

	bool fHaveLevels;

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


	void setMonitorChargeOptions( int nLive_min=100, double sum_min=-100, bool useMedian=true) ;
	void setFitOptions( int n_min=2, double pure_min=0.8, double prob_min=0.01, double b_max=2.0 );
	bool fIsOk();
//	double fpeaks( double *x, double *par );
  
	bool setDST( TTree * dst );
	void resetVectors();

	bool makeMonitorChargeHists( );
	bool findLightLevels();
	int checkLightLevels( int tel, int iPeakSignificance=2, bool iDraw=false );
	bool calculateMeanCharges();

	bool doTheFit( ) ;

	bool terminate(  );
	void fillLightLevels( int tel, int iPeakSignificance=2 , bool iDraw=false );


	ClassDef(VLowGainCalibrator,2);


};

#endif
