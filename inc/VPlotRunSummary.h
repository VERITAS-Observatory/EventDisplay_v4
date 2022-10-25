//! VPlotRunSummary different plot routines (rates vs. elevation etc)

#ifndef VPlotRunSummary_H
#define VPlotRunSummary_H

#include "CEffArea.h"
#include "CRunSummary.h"

#include "VAstronometry.h"
#include "VPlotUtilities.h"
#include "VStatistics.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLegend.h"
#include "TLine.h"
#include "TMath.h"
#include "TRandom.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class VPlotRunSummary : public VPlotUtilities, public TObject
{
	private:
		bool bIsZombie;
		
		TFile* fIn;
		TTree* t;
		CRunSummary* c;
		
		vector< double > fPhaseMin;
		vector< double > fPhaseMax;
		vector< double > fRunList;
		vector< vector< double > > fPhaseRunlist;
		vector< vector< double > > fPhasePerRun;
		vector< vector< double > > fPhaseRatePerRun;
		vector< vector< double > > fPhaseRateEPerRun;
		vector< vector< double > > fPhaseSigniPerRun;
		
		// daily values
		vector< double > fDayDate;
		vector< double > fDayMJD;
		vector< double > fDayPhase;
		vector< double > fDayDuration;
		vector< double > fDayNOn;
		vector< double > fDayNOff;
		vector< double > fDayAlpha;
		vector< double > fDayRate;
		vector< double > fDayRateE;
		vector< double > fDaySigni;
		
		void defineGraphsAndHistograms( string iname, int icolor, int imarker, bool b1129 );
		double getPhase( double iMJD );
		bool openfile( string ifile, int iTot = 1 );
		
		int fMinRun;
		int fMaxRun;
		vector< int > fListofExcludedRuns;
		
		double fMinElevation;
		double fMaxElevation;
		
		// orbital phase related variables
		unsigned int fNPhaseBins;
		double fPhaseOrbit;
		double fPhaseT0;
		double fPhasePoff;
		
	public:
	
		TList* gList;
		
		TH1D* hRunDuration;
		
		TH1D* hRate;
		TH1D* hSignificance;
		
		TGraphErrors* gRatevsTime;
		TGraphErrors* gRateOffvsTime;
		TGraphErrors* gSignificancevsTime;
		TGraphErrors* gCumSignificancevsTime;
		
		TGraphErrors* gRatevsElevation;
		TGraphErrors* gRateOffvsElevation;
		TGraphErrors* gRatevsElevationBinned;
		TGraphErrors* gRawRatevsElevation;
		TGraphErrors* gRawRateOffvsElevation;
		TGraphErrors* gRawRatevsElevationBinned;
		TGraphErrors* gSignificancevsElevation;
		TGraphErrors* gSignificancevsElevationBinned;
		
		TGraphErrors* gRatevsWobbleDirection;
		TGraphErrors* gRateOffvsWobbleDirection;
		TGraphErrors* gSignificancevsWobbleDirection;
		TGraphErrors* gElevationvsWobbleDirection;
		
		TGraphErrors* gRatevsWobbleOffset;
		TGraphErrors* gRateOffvsWobbleOffset;
		TGraphErrors* gSignificancevsWobbleOffset;
		
		TGraphErrors* gRatevsPhase;
		TGraphErrors* gRatevsPhaseBinned;
		TGraphErrors* gRawRatevsPhase;
		TGraphErrors* gRawRatevsPhaseBinned;
		TGraphErrors* gElevationvsPhase;
		TGraphErrors* gElevationvsPhaseBinned;
		TGraphErrors* gSignificancevsPhase;
		TGraphErrors* gSignificancevsPhaseBinned;
		
		TGraphErrors* gRatesvsTimeDaily;
		TGraphErrors* gRatevsPhaseDaily;
		TGraphErrors* gSignificancevsPhaseDaily;
		
		TH1D* hPedvars;
		TH1D* hAbsAzimuth;
		TH1D* hElevation;
		
		TH2D* hMaxSigniXY;
		
		VPlotRunSummary( string ifile, unsigned int iTot = 1, string iname = "A", int icolor = 1, int imarker = 20, bool b1129 = true );
		~VPlotRunSummary() {}
		void fill();
		void fillDailyRates( bool iCorrectForDeadTimes = false );
		bool IsZombie()
		{
			return bIsZombie;
		}
		void makeNewPhaseRunList( string ifile, string ilist );
		TCanvas* plot_stats();
		TCanvas* plot_cumulativeSignificance_vs_time()
		{
			return plot_cumSignificance();
		}
		TCanvas* plot_cumSignificance();
		TCanvas* plot_dailyRates( bool iDeadTimeCorrection = false );
		TCanvas* plot_rates( bool iOff = false );
		TCanvas* plot_ratevsElevation( bool iOff = false );
		TCanvas* plot_ratevsWobbleDirection( TCanvas* c = 0, bool bPlotBackground = false, bool bPlotIndividualRuns = false );
		void printDailyRates();
		void printPhaseList();
		// default: values for LS I +61 303
		void setPhaseValues( unsigned int inPhase = 10, double orbit = 26.4960, double t0 = 2443366.775, double poff = 0. );
		void setElevationRange( double iMin = 0., double iMax = 90. )
		{
			fMinElevation = iMin;
			fMaxElevation = iMax;
		}
		void setListofExcludedRuns( string iList );
		void setRunRange( int imin, int imax )
		{
			fMinRun = imin;
			fMaxRun = imax;
		}
		void writeRunTable();
		void writeRunTable( string iout, bool iPhases = false, bool iSignificancePerHour = false, bool iCorrectForDeadTimes = false );
		
		ClassDef( VPlotRunSummary, 2 );
};
#endif
