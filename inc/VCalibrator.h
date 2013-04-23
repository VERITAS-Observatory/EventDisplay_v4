//! VCalibrator calibration class (calculation of pedestals/gains/toffsets), reading and writing of all calibration data

#ifndef VCALIBRATOR_H
#define VCALIBRATOR_H

#include <VImageBaseAnalyzer.h>
#include <VPedestalCalculator.h>
#include <VDB_CalibrationInfo.h>

#include "TFile.h"
#include "TH1F.h"
#include "TLeaf.h"
#include "TMath.h"
#include "TProfile.h"
#include "TSystem.h"
#include "TTree.h"

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class VCalibrator : public VImageBaseAnalyzer
{
    private:
        int fCalibrationfileVersion;

        map< ULong64_t, int > fNumberPedestalEvents;        //!< number of events used in pedestal analysis
        vector<int> fNumberGainEvents;            //!< number of events used in gain and toffset analysis
	vector<int> fNumberTZeroEvents;

	TFile *fPedSingleOutFile;
        map< ULong64_t, TFile* > fPedOutFile;
        map< ULong64_t, vector< vector<TH1F* > > > hped_vec;  //<! one histogram per telescope/channel/sumwindow
        TFile *opfgain;
        TFile *opftoff;
        vector<TH1F* > hgain;
        vector<TProfile* > hpulse;
        vector<TProfile* > htcpulse;
        vector<TH1F* > htoff;
        vector<TProfile* > htoff_vs_sum;

// average Tzero calculation
        vector< TFile * > fTZeroOutFile;
// one histogram per telescope and channel
	vector< vector< TH1F* > > htzero;

// all calibration data is summarized in the following trees (one tree per telescope)
        TTree* fToffTree;

        vector< string > fPedFileNameC;
        vector< string > fPadFileNameC;
        vector< string > fGainFileNameC;
        vector< string > fToffFileNameC;
        vector< string > fPixFileNameC;
	vector< string > fTZeroFileNameC;
        vector< bool > fBlockTel;
        vector< string > fLowGainPedFileNameC;
        vector< string > fLowGainGainFileNameC;
        vector< string > fLowGainToffFileNameC;
        vector< string > fLowGainMultiplierNameC;
	vector< string > fLowGainTZeroFileNameC;

	TTree *fillCalibrationSummaryTree( unsigned int itel, string iName, vector<TH1F* > h );
        bool   fillPedestalTree( unsigned int tel, VPedestalCalculator *iP );
        void getCalibrationRunNumbers();
	int  getCalibrationRunNumbers_fromCalibFile();
	unsigned int getNumberOfEventsUsedInCalibration( vector< int > iE, int iTelID );
	unsigned int getNumberOfEventsUsedInCalibration( map< ULong64_t, int > iE, int iTelID );
	TFile* getPedestalRootFile( ULong64_t iTel );
	int  readLowGainCalibrationValues_fromCalibFile( string iVariable = "LOWGAINPED", unsigned int iTel = 9999, int iSumWindow = 9999 );
	string getCalibrationFileName( int iTel, int irun, string iSuffix );
        void readCalibrationData( bool iPeds, bool iGains );
        bool readCalibrationData( string iSourceFile );
	void readfromVOFFLINE_DB(int gain_or_toff, string &iFile, vector< unsigned int >& VchannelList, vector< double >& Vmean, vector< double >& Vrms );
        void readGains( bool iLowGain = false );
        bool readLowGainMultiplier( int iSumWindow );
        bool readPeds( string iFile, bool, unsigned int );
	bool readPeds_from_grisufile( string iFile, bool, unsigned int );
	bool readPeds_from_rootfile( string iFile, bool, unsigned int );
	bool readPeds_from_textfile( string iFile, bool iLowGain, unsigned int i_SumWindow );
        void readPixelstatus();
        void readTOffsets( bool iLowGain = false );
        bool readAverageTZeros( bool iLowGain = false );
	void setCalibrationFileNames();

        void writeGains( bool iLowGain = false );
        void writePeds( bool iLowGain = false );
        void writePeds( bool iLowGain, VPedestalCalculator *iP, bool iWriteAsciiFile = true );
        void writeTOffsets( bool iLowGain = false  );
	void writeAverageTZeros( bool iLowGain = false  );


    public:
        VCalibrator();
        ~VCalibrator() {}

	void calculateAverageTZero( bool iLowGain = false );
        void calculatePedestals( bool iLowGain = false );
        void calculateGainsAndTOffsets( bool iLowGain = false );
	unsigned int getNumberOfEventsUsedInCalibration( int iTelID, int iType );
        void initialize();
        void terminate( VPedestalCalculator* );
};
#endif
