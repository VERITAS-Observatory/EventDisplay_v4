//! VTraceHandler

#ifndef VTRACE_H
#define VTRACE_H

#include <fstream>
#include <iostream>
#include <stdint.h>
#include <vector>

#include "VVirtualDataReader.h"

using namespace std;

class VTraceHandler
{
    private:
                                                  //!< linear interpolation
        double getLinInterpol( double y5, int x1, double y1, int x2, double y2 );

    protected:
        unsigned int    fTraceIntegrationMethod;  //   set trace integration method (see setter in source file for definition)
        vector< double >  fpTrace;                //!< the FADC trace
	unsigned int fpulsetiming_maxPV;
	unsigned int fpulsetiminglevels_size;
	vector< float > fpulsetiminglevels;       //!< levels in fraction of maximum for pulse timing calculation
	vector< float > fpulsetiming;             //!< pulse timing vector (refilled for each call)
        int fpTrazeSize;                          //!< length of the FADC trace
        double fMax;                              //!< Max value for this trace
        double fPed;                              //!< Ped value for this trace
        double fPedrms;                           //!< Ped rms for this trace (needed for VFitTraceHandler)
        double fSum;                              //!< Sum value for this trace
	double fTraceAverageTime;                 //!< average time for this trace
	int    fSumWindowFirst;
	int    fSumWindowLast;
        unsigned int fChanID;                     //!< channel ID
        bool fHiLo;                               //!< hilo bit
        int fDynamicRange;                        //!< dynamic range
        int fMaxThreshold;
	unsigned int fMC_FADCTraceStart;          // start of FADC trace (in case the simulated trace is longer than needed)

	double   getQuickMaximumSum( int iIntegrationWindow, bool fRaw = false );
	void     reset();

    public:
        VTraceHandler();
        virtual ~VTraceHandler(){};

                                                  //!< pass the trace values
        virtual void setTrace(vector< uint8_t >, double, double, unsigned int);
        virtual void setTrace(vector< uint16_t >, double, double, unsigned int);
                                                  //!< pass the trace values (with hilo)
        virtual void setTrace(vector< uint8_t >, double, double, unsigned int, double );
        virtual void setTrace(vector< uint16_t >, double, double, unsigned int, double );
                                                  //!< pass the trace values (with hilo)
        virtual void setTrace( VVirtualDataReader* iReader, unsigned int iNSamples, double ped, double pedrms,
	                       unsigned int iChanID, unsigned int iHitID, double iHilo = -1. );
// methods for getting quick trace parameters between specified limits
        void   apply_lowgain( double );
        void   calcQuickPed(int ,int);
        double getQuickSum(int ,int, bool);
        double getQuickTZero(int ,int);
        double getQuickTZero(int ,int, int );
        void   getQuickMax(int ,int, double&, int& );
        void   getQuickMax(int ,int, double&, int&, unsigned int& );
        double getQuickPulseWidth( int fFirst, int fLast, double fPed );
        double getPed() { return fPed; }
        double getMax() { return fMax; }
        double getSum() { return fSum; }
        vector< double >& getTrace() { return fpTrace; }
	double getTraceAverageTime() { return fTraceAverageTime; }
	int    getTraceIntegrationFirst() { return fSumWindowFirst; }
	int    getTraceIntegrationLast()  { return fSumWindowLast; }
// virtual functions
        virtual double getTraceWidth( int fFirst, int fLast, double fPed ) { return getQuickPulseWidth( fFirst, fLast, fPed ); }
        virtual double getTraceSum( int fFirst, int fLast, bool fRaw );
        virtual double getTraceTZero( int fFirst, int fLast ) { return getQuickTZero( fFirst, fLast ); }
        virtual double getTraceTZero( int fFirst, int fLast, int fTFirst ) { return getQuickTZero( fFirst, fLast, fTFirst ); }
	virtual vector< float >& getPulseTiming( int fFirst, int fLast, int fTFirst, int fTLast );
        virtual double getTraceMax();
        virtual double getTraceMax( unsigned int &n255, double iHiLo = 6. ); // get maximum value in trace
        virtual void   getTraceMax( double&, double& );
        virtual void   getTraceMax( int fFirst, int fLast, double &max, int &maxpos ) { getQuickMax( fFirst, fLast, max, maxpos ); }
        virtual void   getTraceMax( int fFirst, int fLast, double &max, int &maxpos, unsigned int &n255 ) { getQuickMax( fFirst, fLast, max, maxpos, n255 ); }
        virtual double getTraceChi2() { return 0.; }
        virtual int    getTraceFitStat()          //!< 3 means successful fit
        {
            return 3;
        }
        virtual double getTraceFallTime() { return 0.; }
        virtual double getTraceFallTime( double fPed, double ystart, double ystop  ) { return 0.; }
        virtual double getTraceNorm() { return 0.; }
        virtual double getTraceRiseTime() { return 0.; }
        virtual double getTraceRiseTime( double fPed, double ystart, double ystop  ) { return 0.; }
        void    setDynamicRange( int iD ) { fDynamicRange = iD; }
        void    setMaxThreshold( int iD ) { fMaxThreshold = iD; }
	void    setMC_FADCTraceStart( unsigned int iO = 0 ) { fMC_FADCTraceStart = iO; }
	void    setPulseTimingLevels( vector< float > iP );
	bool    setTraceIntegrationmethod( unsigned int iT = 0 );
};
#endif
