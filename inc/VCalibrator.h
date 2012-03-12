//! VCalibrator calibration class (calculation of pedestals/gains/toffsets), reading and writing of all calibration data

#ifndef VCALIBRATOR_H
#define VCALIBRATOR_H

#include <VImageBaseAnalyzer.h>
#include <VPedestalCalculator.h>

#include "TFile.h"
#include "TH1F.h"
#include "TMath.h"
#include "TProfile.h"
#include "TSystem.h"
#include "TTree.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class VCalibrator : public VImageBaseAnalyzer
{
    private:
        int fCalibrationfileVersion;

        vector<int> fNumberPedestalEvents;        //!< number of events used in pedestal analysis
        vector<int> fNumberGainEvents;            //!< number of events used in gain and toffset analysis

        vector< TFile* > fPedOutFile;
                                                  //<! one histogram per telescope/channel/sumwindow
        vector< vector< vector<TH1F* > > > hped_vec;
        TFile *opfgain;
        TFile *opftoff;
        vector<TH1F* > hgain;
        vector<TProfile* > hpulse;
        vector<TProfile* > htcpulse;
        vector<TH1F* > htoff;
        vector<TProfile* > htoff_vs_sum;

// all calibration data is summarized in the following trees (one tree per telescope)
        TTree* fGainTree;
        TTree* fToffTree;

        vector< string > fPedFileNameC;
        vector< string > fPadFileNameC;
        vector< string > fGainFileNameC;
        vector< string > fToffFileNameC;
        vector< string > fPixFileNameC;
        vector< bool > fBlockTel;
        vector< string > fLowGainPedFileNameC;
        vector< string > fLowGainGainFileNameC;
        vector< string > fLowGainToffFileNameC;
        vector< string > fLowGainMultiplierNameC;

        TTree *fillGainTree( unsigned int tel );
        bool   fillPedestalsInTimeSlices( unsigned int tel, VPedestalCalculator *iP );
        TTree* fillToffTree( unsigned int tel );
        void getCalibrationRunNumbers();
	int  getCalibrationRunNumbers_fromCalibFile();
	int  getLowGainCalibrationRunNumbers_fromCalibFile();
	string getCalibrationFileName( int iTel, int irun, string iSuffix );
        void readCalibrationData( bool iPeds, bool iGains );
        bool readCalibrationData( string iSourceFile );
        void readGains( bool iLowGain = false );
        bool readLowGainMultiplier( bool iSmall );
        bool readPeds( string iFile, bool, unsigned int );
	bool readPeds_from_grisufile( string iFile, bool, unsigned int );
	bool readPeds_from_rootfile( string iFile, bool, unsigned int );
	bool readPeds_from_textfile( string iFile, bool iLowGain, unsigned int i_SumWindow );
        void readPixelstatus();
        void readTOffsets( bool iLowGain = false );
	void setCalibrationFileNames();

        void writeGains( bool iLowGain = false );
        void writePeds( bool iLowGain = false );
        void writePeds( bool iLowGain, VPedestalCalculator *iP );
        void writeTOffsets( bool iLowGain = false  );

    public:
        VCalibrator();
        ~VCalibrator() {}

        void calculatePedestals( bool iLowGain = false );
        void calculateGainsAndTOffsets( bool iLowGain = false );
        void initialize();
        void terminate( VPedestalCalculator* );
};
#endif
