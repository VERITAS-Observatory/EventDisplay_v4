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
#include <string>
#include <valarray>
#include <vector>

using namespace std;

class VCalibrationData
{
    private:

        unsigned int fTelID;
	unsigned int fSumWindow;

	valarray< double > fValArrayDouble;

        TList *hisList;
        TFile *fPedFile;
        TFile *fGainFile;
        TFile *fToffFile;
        TFile *fLowGainPedFile;
        TFile *fLowGainGainFile;
        TFile *fLowGainMultiplierFile;
        TFile *fLowGainToffFile;

        string fCalDirName;
        string fPedFileName;
        string fGainFileName;
        string fToffFileName;
        string fLowGainPedFileName;
        string fLowgainGainFileName;
        string fLowGainToffFileName;
        string fLowGainMultiplierFileName;

        VVirtualDataReader  *fReader;

	unsigned int setSummationWindow( unsigned int iSumWindow );

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
        TH1F* fPedDistribution;                   //!< mean pedestal distribution

        TH1F* fPedvarsDistribution;               //!< pedestal variance distribution
        valarray<double>  fPedrms;                //!< pedestal variance
        vector< double > fVmeanPedvars;           //!< mean pedestal variance (for different sumwindows)
        vector< double > fVmeanRMSPedvars;        //!< RMS pedestal variance (for different sumwindows)

        TH1F* fTOffsetsDistribution;              //!< toffset distribution
        TH1F* fTOffsetVarsDistribution;           //!< toffset vars distribution
        valarray<double> fTOffsets;               //!< time offsets
        valarray<double> fTOffsetvars;            //!< time offset variance
        valarray<double> fFADCStopOffsets;        //!< time offsets due to FADC Stop
        valarray<double> fGains;                  //!< gains
	valarray< bool > fGains_DefaultSetting;   //!< gain value is set to default value
        TH1F* fGainsDistribution;                 //!< gain distribution
        TH1F* fGainVarsDistribution;              //!< gain var distribution
        valarray<double> fGainvars;               //!< gain variance

// low gain channels
        bool fBoolLowGainPedestals;
        TH1F* fLowGainPedDistribution;            //!< mean pedestal distribution  (low gain)
        TH1F* fLowGainPedvarDistribution;         //!< pedestal variance distribution (low gain)

        valarray<double> fLowGainPedsrms;         //!< low gain mean pedestal variance
        double fmeanLowGainPedvars;
        double fmeanRMSLowGainPedvars;
        vector< double > fVmeanLowGainPedvars;
        vector< double > fVmeanRMSLowGainPedvars;

// low gain multiplier
        vector< valarray<double> > fLowGainMultiplier;
        vector< valarray<double> > fLowGainMultiplierError;
        vector< double >           fLowGainMultiplier_Mean;
        vector< double >           fLowGainMultiplier_RMS;
        TH1F*                      fLowGainMultiplierDistribution;
	unsigned int               fLowGainMultiplier_FixedSummationWindow;   //! if this is set to a reasonable value: all summation window requests are ignored

// low gain: time offsets
        bool fBoolLowGainTOff;
        TH1F* fLowGainTOffsetsDistribution;       //!< toffset distribution
        TH1F* fLowGainTOffsetVarsDistribution;    //!< toffset vars distribution
        valarray<double> fLowGainTOffsets;        //!< time offsets
        valarray<double> fLowGainTOffsetvars;     //!< time offset variance
        valarray<double> fLowGainGains;           //!< gains
	valarray< bool > fLowGainGains_DefaultSetting;   //!< gain value is set to default value
        bool fBoolLowGainGains;
        TH1F* fLowGainGainsDistribution;          //!< gain distribution
        TH1F* fLowGainGainVarsDistribution;       //!< gain var distribution
        valarray<double> fLowGainGainvars;        //!< gain variance

        VCalibrationData( unsigned int iTel, string iDir, string iPedfile, string iGainfile, string iTofffile, 
	                                                   string iPedLowGainfile, string iGainLowGainFile = "", string iToffLowGainFile = "", string iLowGainMultFile = "" );
       ~VCalibrationData() {}

        void initialize( unsigned int iChannel, unsigned int nSamples = 24, bool iTimeSlices = true, bool iLowGainTimeSlices = false, bool iDebug = false );
        TH1F* getPedDist( bool iHiLo = false ) { if( !iHiLo )     return fPedDistribution; else return fLowGainPedDistribution; }
        TH1F* getPedvarsDist( bool iHiLo = false ) { if( !iHiLo ) return fPedvarsDistribution; else return fLowGainPedvarDistribution; }
        TH1F* getToffsetDist( bool iHiLo = false ) { if( !iHiLo ) return fTOffsetsDistribution; else return fLowGainTOffsetsDistribution; }
        TH1F* getToffsetVarsDist( bool iHiLo = false ) { if( !iHiLo ) return fTOffsetVarsDistribution; else return fLowGainTOffsetVarsDistribution; }
        TH1F* getGainDist( bool iHiLo = false ) { if( !iHiLo )    return fGainsDistribution; else return fLowGainGainsDistribution; }
        TH1F* getGainVarsDist( bool iHiLo = false ) { if( !iHiLo ) return fGainVarsDistribution; else return fLowGainGainVarsDistribution; }
        TH1F* getHistoGain( unsigned int iTel, unsigned int iChannel, bool iLowGain = false );
        TH1F* getHistoGainLowGain( unsigned int iTel, unsigned int iChannel ) { return getHistoGain( iTel, iChannel, true ); }
        TH1F* getHistoPed( unsigned int iTel, unsigned int iChannel, unsigned int iWindowsize, bool iLowGain = false );
        TH1F* getHistoPedLowGain( unsigned int iTel, unsigned int iChannel, unsigned int iWindowsize );
        TH1F* getHistoToff( unsigned int iTel, unsigned int iChannel, bool iLowGain = false );
        TH1F* getHistoToffLowGain( unsigned int iTel, unsigned int iChannel ) { return getHistoToff( iTel, iChannel, true ); }

	valarray< double >& getLowGainMultiplier( unsigned int iSumWindow = 9999 );
	valarray< double >& getLowGainMultiplierError( unsigned int iSumWindow = 9999 );
	double              getMeanLowGainMultiplier( unsigned int iSumWindow = 9999 );
	double              getRMSLowGainMultiplier( unsigned int iSumWindow = 9999 );
        TH1F*               getLowGainMultiplierDistribution() { return fLowGainMultiplierDistribution; }    // PRELI: should be a vector of size nsumwindows

        string getGainFileName() { return fGainFileName; }
        string getPedFileName() { return fPedFileName; }
        string getToffFileName() { return fToffFileName; }
        string getLowGainGainFileName() { return fLowgainGainFileName; }
        string getLowGainPedFileName() { return fLowGainPedFileName; }
        string getLowGainToffFileName() { return fLowGainToffFileName; }
        string getLowGainMultFileName() { return fLowGainMultiplierFileName; }

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
	bool     setLowGainMultiplierFixedSummationWindow( unsigned int iSumWindow = 9999 );
        bool     setLowGainMultiplier( double iV, unsigned int iChannel, unsigned int iSumWindow );
	bool     setMeanLowGainMultiplier( double g, unsigned int iSumWindow = 9999 );
	bool     setRMSLowGainMultiplier( double g, unsigned int iSumWindow = 9999 );
        void     setPeds( unsigned int iChannel, double iPed, bool iLowGain = false, double iTime = -99. );
        void     setReader( VVirtualDataReader* f ) { fReader = f; }
	void     setSumWindows( unsigned int isw ) { fSumWindow = isw; }
        bool     terminate( vector< unsigned int > a, vector< unsigned int > b, bool iDST = false );
        bool     usePedestalsInTimeSlices( bool iB ) { if( !iB ) return fUsePedestalsInTimeSlices; else return fLowGainUsePedestalsInTimeSlices; }

};
#endif
