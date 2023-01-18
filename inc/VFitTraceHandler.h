//! VFitTraceHandler trace handling with fit function

#ifndef VFITTRACE_H
#define VFITTRACE_H

#include <math.h>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>

#include "TDirectory.h"
#include "TF1.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TMinuit.h"

#include "VTraceHandler.h"
#include "VVirtualDataReader.h"

using namespace std;

//!< definition of fit function to trace
double VFitTraceHandler_tracefunction( double* x, double* p );
//! definition of fit function to trace (grisu version)
double VFitTraceHandler_tracefunction_Grisu( double* x, double* p );

class VFitTraceHandler : public VTraceHandler
{

	private:
		string fFitFunction;                      //!< ev or grisu
		TF1* fF1Trace;                            //!< fit function
		TF1* fF1TraceGr;                          //!< fit function (from grisu)
		TH1D* fH1Trace;                           //!< histogram with values from fit function
		TH1D* fH1TraceData;                       //!< histogram with trace data
		int fMaxSamples;                          //!< sample maximum (usually 64)
		bool fFitted;                             //!< true after trace fit
		bool fMinuitPrint;                        //!< if true, long printout from minuit
		
		double fChi2;                             //!< Chi2 for this trace
		double fTraceNorm;                        //!< trace normalisation
		double fRT;                               //!< rise time for this trace (obs: different definitions for different fit functions)
		double fFT;                               //!< fall time for this trace (obs: different definitions for different fit functions)
		double fTraceMax;                         //!< maximum of trace
		double fTraceMaxX;                        //!< position of maximum of trace
		
		unsigned int fMC_FADCTraceStart;          // start of FADC trace (in case the simulated trace is longer than needed)
		
		// analysis histograms
		TH1D* fHfitstat;
		int fnstat;                               //!< fit status ( bad if < 3 )
		TH1D* fHchi2;
		TH2D* fHxbar;                             //!< fit parameter xbar vs. tube number
		TH2D* fHsigma;                            //!< fit parameter sigma vs. tube number
		TH2D* fHalpha;                            //!< fit parameter alpha vs. tube number
		
		double fFitThresh;                        //!< threshold above trace is fitted
		
		void fitTrace( unsigned int chanID );
		
	public:
		VFitTraceHandler( string );
		~VFitTraceHandler();
		
		TF1*   getFitFun()                        //!< get fit function
		{
			return fF1Trace;
		}
		TH1D*  getFitHis();                       //!< get histogram of fit function
		bool   getFitted()                        //!< status of tracehandler, true after fit
		{
			return fFitted;
		}
		TH1D*  getHfitstat()                      //!< get histogram with TMinuit fit status
		{
			return fHfitstat;
		}
		TH1D*  getHchi2()                         //!< get histogram with chi2 values of fit
		{
			return fHchi2;
		}
		//!< at 50% of maximum
		double getTraceWidth( int fFirst, int fLast, double fPed );
		double getTraceChi2()
		{
			return fChi2;
		}
		int    getTraceFitStat()
		{
			return fnstat;
		}
		double getTraceFallTime()                 //!< fall parameter of fit function
		{
			return fFT;
		}
		//!< fall parameter of fit function (ystart, ystop in %)
		double getTraceFallTime( double fPed, double ystart, double ystop );
		double getTraceRiseTime()                 //!< rise parameter of fit function
		{
			return fRT;
		}
		//!< rise parameter of fit function (ystart, ystop in %)
		double getTraceRiseTime( double fPed, double ystart, double ystop );
		double getTraceAverageTime()
		{
			return 0.;
		}
		double getTraceMax()
		{
			return -1.*fTraceMax - fPed;
		}
		//!< get maximum value in trace
		void   getTraceMax( int, int, double&, int& );
		void   getTraceMax( double&, double& );   //!< get maximum value in trace
		double getTraceNorm()                     //!< return trace normalisation
		{
			return fTraceNorm;
		}
		double getTraceSum( int, int, bool );     //!< get sum from integration of fit function
		double getTraceTZero( int, int );         //!< get pulse start (at half maximum of the rising edge of the pulse)
		bool   setFitFunction( string );          //!< set fit function to use
		void   setFitThresh( double ithres )      //!< threshold above PMT is fitted (don't fit traces with no signal)
		{
			fFitThresh = ithres;
		}
		void   setMinuitPrint( bool iPrint )      //!< if true, long printout of fitting
		{
			fMinuitPrint = iPrint;
		}
		void    setMC_FADCTraceStart( unsigned int iO = 0 )
		{
			fMC_FADCTraceStart = iO;
		}
		//!< set trace, pedestal, pedestal rms
		void   setTrace( vector< uint8_t >, double, double, unsigned int );
		//!< set trace, pedestal, pedestal rms
		void   setTrace( vector< uint8_t >, double, double, unsigned int, double );
		//!< set trace, pedestal, pedestal rms
		void   setTrace( vector< uint16_t >, double, double, unsigned int );
		//!< set trace, pedestal, pedestal rms
		void   setTrace( vector< uint16_t >, double, double, unsigned int, double );
		//!< set trace, pedestal, pedestal rms
		void   setTrace( VVirtualDataReader* iReader, unsigned int iNSamples, double ped, double pedrms,
						 unsigned int iChanID, unsigned int iHitID, double iHilo = -1. );
		void   terminate();                       //!< write analysis histograms to disk
};
#endif
