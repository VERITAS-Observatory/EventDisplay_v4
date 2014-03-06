//! VCalibrationData  data class for calibration data

#ifndef VCALIBRATIONDATA_H
#define VCALIBRATIONDATA_H

#include "TDirectory.h"
#include "TError.h"
#include "TFile.h"
#include "TH1F.h"
#include "TList.h"
#include "TTree.h"

#include "VVirtualDataReader.h"

#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <valarray>
#include <vector>
#include <map>

using namespace std;

class VCalibrationData
{
    private:

        unsigned int fTelID;
	unsigned int fSumWindow;

	valarray< double > fValArrayDouble;

	enum E_PEDTYPE { C_PED, C_GAIN, C_TOFF, C_PEDLOW, C_GAINLOW, C_TOFFLOW, C_LOWGAIN, C_TZERO, C_TZEROLOW };

        TList *hisList;

        string fCalDirName;
	vector< string > fFileName;
	vector< TFile* > fFile;
	vector< string > fHistoName;
	vector< TH1F* >  fHisto_mean;
	vector< TH1F* >  fHisto_variance;

        VVirtualDataReader  *fReader;

	TH1F* getHistogram( unsigned int iTel, unsigned int iChannel, unsigned int iWindowSize,  VCalibrationData::E_PEDTYPE, ULong64_t iTelType = 99999 );
	TH1F* getHistoDist( int iType, bool iDist );

    public:
        bool fPedFromPLine;
        bool fUsePedestalsInTimeSlices;
        bool fLowGainUsePedestalsInTimeSlices;

        vector<int> fChannelStatus;               //!< pixel status

// pedestals
        valarray< int > fTS_MJD;                  //!< MJD for pedestals in time slices
        valarray< double > fTS_time;              //!< time for pedestals in time slices
        valarray<double> fPeds;                   //!< mean pedestal
        valarray< valarray < double > > fTS_Peds; //!< time dependant mean pedestal  [time slice][channel]
        vector< valarray< double > > fVPedvars;   //!< pedestal variance (for different sumwindows) [summation window][channel]
        valarray< valarray< valarray < double > > > fTS_fVPedvars; //!< time dependant pedestal variance [time slice][summation window][channel]
        valarray< double > fTS_ped_temp;
        double fTS_ped_temp_time;
        map< unsigned int, valarray< double > > fTS_pedvar_temp;
        map< unsigned int, double > fTS_pedvar_temp_time;

        valarray< valarray< double > > fTS_fVmeanPedvars;       //! time dependent mean pedestal variation [time slice][summation window]
        valarray< valarray< double > > fTS_fVmeanRMSPedvars;    //!< time dependent RMS of pedestal variation [time slice][summation window]

        valarray<double> fLowGainPeds;                          //!< low gain mean pedestal
        valarray < valarray < double > > fLowGainTS_Peds;       //!< time dependant mean pedestal (low gain)
        vector< valarray<double> > fVLowGainPedvars;            //!< low gain variance (sumwindow)
        valarray< valarray< valarray < double > > > fLowGainTS_fVPedvars; //!< time dependant pedestal variance (for different sumwindows)
        valarray< valarray< double > > fLowGainTS_fVmeanPedvars;          //! time dependent mean pedestal variation [time slice][summation window]
        valarray< valarray< double > > fLowGainTS_fVmeanRMSPedvars;       //!< time dependent RMS of pedestal variation [time slice][summation window]

// high gain channels
        valarray<double>  fPedrms;                //!< pedestal variance
        vector< double > fVmeanPedvars;           //!< mean pedestal variance (for different sumwindows)
        vector< double > fVmeanRMSPedvars;        //!< RMS pedestal variance (for different sumwindows)

        valarray<double> fFADCStopOffsets;        //!< time offsets due to FADC Stop
        valarray<double> fTOffsets;               //!< time offsets
        valarray<double> fTOffsetvars;            //!< time offset variance
        valarray<double> fGains;                  //!< gains
	valarray< bool > fGains_DefaultSetting;   //!< gain value is set to default value
        valarray<double> fGainvars;               //!< gain variance
	valarray<double> fAverageTzero;
	valarray<double> fAverageTzerovars;

// low gain channels
        bool fBoolLowGainPedestals;

        valarray<double> fLowGainPedsrms;         //!< low gain mean pedestal variance
        double fmeanLowGainPedvars;
        double fmeanRMSLowGainPedvars;
        vector< double > fVmeanLowGainPedvars;
        vector< double > fVmeanRMSLowGainPedvars;

// low gain multipliers
/*
We now have 2 types of low gain multipliers. Both are currently assumed to be the same for all channels in a telescope, but can vary between telescopes. Both are read in from the calibrationlist.lowgain.dat. 

fLowGainMultiplier_Trace is used for plotting the trace, for the trace max, and as a default if the correct multiplier is not available.

fLowGainMultiplier_Sum should be used for everything involving integrated traces. It has 2 'indices'. The first one is the 'nominal sumwindow' (eg. 6 for sums, 16 for sums2). The second one is the actual sumwindow used in a given pixel/event, depending on the shower/trace timing. The low gain multipliers are determined from HiLo runs by comparing the charge-vs-monitor charge ratio in the high-gain range, read out at the 'nominal' sum window, and the charge-vs-monitor-charge ratio in the low gain range, read out at the 'actual' window.

In the analysis, the setTrace function is always called with the fLowGainMultiplier_Trace. In the setSums / setSums2 function, we multiply the trace integral with the lowgainsumcorrection, which is just the ratio of fLowGainMultiplier_Sum over fLowGainMultiplier_Trace for a given sum window. This function returns 1 if the fLowGainMultiplier_Sum is 0, i.e. doesn't exist for a given sumwindow.

The low gain multiplier values are saved in a map so that we are flexible about which combination of sum windows are read in. The 'keys' are pairs of ints, corresponding to the 2 indices. Because of that, it can be hard to see just from the map which sumwindows are available. Sometimes we might want to check if the user has specified a nominal sumwindow for which she hasn't supplied the low gain multipliers. For that purpose, we also keep track of the nominal sum windows for which *some* low gain multiplier was read in in the fLowGainDefaultSumWindows vector.
*/	

	double fLowGainMultiplier_Trace;				// This corresponds to the 'old' LG multiplier -> is used for Max, traces, ...
	std::map< std::pair<int, int>, double> fLowGainMultiplier_Sum;  // new low gain multipliers. 1st index: sw for sum / sum2 , ie small/large integration window. 2nd index: actual sumwindow that was read out.	
	valarray<double> fLowGainMultiplier_Camera;			// for plotting. 
	vector<int> fLowGainDefaultSumWindows;				// remember default sumwindows for which low gain multipliers have been read in. 


// average tzero
        double fAverageTZero_highgain;
        double fAverageTZero_lowgain;

// low gain: time offsets
        bool fBoolLowGainTOff;
        valarray<double> fLowGainTOffsets;        //!< time offsets
        valarray<double> fLowGainTOffsetvars;     //!< time offset variance
        valarray<double> fLowGainAverageTzero;        //!< time offsets
        valarray<double> fLowGainAverageTzerovars;     //!< time offset variance
        valarray<double> fLowGainGains;           //!< gains
	valarray< bool > fLowGainGains_DefaultSetting;   //!< gain value is set to default value
        bool fBoolLowGainGains;
        valarray<double> fLowGainGainvars;        //!< gain variance

        VCalibrationData( unsigned int iTel, string iDir, string iPedfile, string iGainfile, string iTofffile, 
	                                                   string iPedLowGainfile, string iGainLowGainFile = "", 
							   string iToffLowGainFile = "", string iLowGainMultFile = "",
							   string iTzerofile = "", string iTzeroLowfile = "",
							   string iObservatory = "VTS" );
       ~VCalibrationData() {}

        TH1F* getHistoGain( unsigned int iTel, unsigned int iChannel, bool iLowGain = false );
        TH1F* getHistoPed( unsigned int iTel, unsigned int iChannel, unsigned int iWindowsize, bool iLowGain = false, ULong64_t iTelType = 99999 );
        TH1F* getHistoToff( unsigned int iTel, unsigned int iChannel, bool iLowGain = false );
        TH1F* getHistoAverageTzero( unsigned int iTel, unsigned int iChannel, bool iLowGain = false );

	TH1F* getPedDist( bool iHiLo = false ) { if( iHiLo ) return getHistoDist( C_PED, true ); else return getHistoDist( C_PEDLOW, true ); }
	TH1F* getPedvarsDist( bool iHiLo = false ) { if( iHiLo ) return getHistoDist( C_PED, false ); else return getHistoDist( C_PEDLOW, false ); }
        TH1F* getToffsetDist( bool iHiLo = false ) { if( iHiLo ) return getHistoDist( C_TOFF, true ); else return getHistoDist( C_TOFFLOW, true ); }
        TH1F* getToffsetVarsDist( bool iHiLo = false ) {  if( iHiLo ) return getHistoDist( C_TOFF, false ); else return getHistoDist( C_TOFFLOW, false ); }
        double getAverageTZero( bool iLowGain = false );
        void  setAverageTZero( double iAverageTzero = 0., bool iLowGain = false );
	TH1F* getAverageTzerosetDist( bool iHiLo = false ) {  if( iHiLo ) return getHistoDist( C_TZERO, true ); else return getHistoDist( C_TZEROLOW, true ); }
        TH1F* getGainDist( bool iHiLo = false ) { if( iHiLo ) return getHistoDist( C_GAIN, true ); else return getHistoDist( C_GAINLOW, true ); }
        TH1F* getGainVarsDist( bool iHiLo = false ) { if( iHiLo ) return getHistoDist( C_GAIN, false ); else return getHistoDist( C_GAINLOW, false ); }
        TH1F* getLowGainMultiplierDistribution() { return getHistoDist( C_LOWGAIN, true ); }

        void initialize( unsigned int iChannel, unsigned int nSamples = 24, bool iTimeSlices = true,
	                 bool iLowGainTimeSlices = false, bool iPedsFromPLine = false, bool iReadCalibDB = false,
			 bool i_isDSTMC = false, bool iDebug = false, int iRunMode = -1, bool isTelToAna = true );

	double getLowGainMultiplier_Trace( ) { return fLowGainMultiplier_Trace ; }
	valarray<double>& getLowGainMultiplier_Camera( ) { return fLowGainMultiplier_Camera ; }
	double getLowGainMultiplier_Sum( int iSumWindow, int jSumWindow ) ;

	double getLowGainSumCorrection(int iSumWindow,int jSumWindow,bool HiLo=true) ;
	vector<int>& getLowGainDefaultSumWindows() { return fLowGainDefaultSumWindows; }

        unsigned int getTelID() { return fTelID; }
        valarray< int >&     getMJDTS_vector() { return fTS_MJD; }
        valarray< double >&  getTimeTS_vector() { return fTS_time; }
        valarray < valarray < double > >& getPedsTS_vector( bool iLowGain = false ) { if( !iLowGain ) return fTS_Peds; else return fLowGainTS_Peds;  }
        valarray< valarray< valarray < double > > >& getPedvarsVTS_vector( bool iLowGain = false ) { if( !iLowGain ) return fTS_fVPedvars; else return fLowGainTS_fVPedvars; }
        valarray < valarray < double > >& getMeanPedvarsVTS_vector( bool iLowGain = false ) { if( !iLowGain ) return fTS_fVmeanPedvars; else return fLowGainTS_fVmeanPedvars; }
        valarray < valarray < double > >& getMeanRMSPedvarsVTS_vector( bool iLowGain = false ) { if( !iLowGain ) return fTS_fVmeanRMSPedvars; else return fLowGainTS_fVmeanRMSPedvars; }

        double   getmeanPedvars( bool iLowGain = false, unsigned int iSumWindow = 0 );
        double   getmeanRMSPedvars( bool iLowGain, unsigned int iSumWindow );
        void     getmeanPedvars( double &imean, double &irms, bool iLowGain = false, unsigned int iSumWindow = 0, double iTime = -99. );
	unsigned int getNSummationWindows() { return fVLowGainPedvars.size(); }
	double   getPed_min( bool iLowGain = false );
	double   getPed_max( bool iLowGain = false );
        valarray<double>& getPeds( bool iLowGain = false, double iTime = -99. );
        valarray<double>& getPedvars( bool iLowGain = false, unsigned int iSW = 0, double iTime = -99. );
        unsigned int getTSTimeIndex( double iTime, unsigned int& i1, unsigned int& i2, double& ifrac1, double& ifrac2 );

	void     recoverLowGainPedestals();
	bool 	setLowGainMultiplier_Trace( double lmult) { fLowGainMultiplier_Trace = lmult; fLowGainMultiplier_Camera=lmult ; return true;} 
	bool 	setLowGainMultiplier_Sum( int iSumWindow, int jSumWindow ,double lmult) ; 

        void     setPeds( unsigned int iChannel, double iPed, bool iLowGain = false );
        void     setReader( VVirtualDataReader* f ) { fReader = f; }
	void     setSumWindows( unsigned int isw ) { fSumWindow = isw; }
        bool     terminate( vector< unsigned int > a, vector< unsigned int > b, bool iDST = false );
        bool     usePedestalsInTimeSlices( bool iB ) { if( !iB ) return fUsePedestalsInTimeSlices; else return fLowGainUsePedestalsInTimeSlices; }

};
#endif
