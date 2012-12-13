//! VEvndispRunParameter  input parameter storage class
//
//  IMPORTANT   IMPORTANT    IMPORTANT   IMPORTANT   IMPORTANT    IMPORTANT
//
//   ClassDef VERSION NUMBER HAS TO BE INCREASED BY ONE AFTER ANY CHANGES HERE
// 
//  IMPORTANT   IMPORTANT    IMPORTANT   IMPORTANT   IMPORTANT    IMPORTANT

#ifndef VEvndispRunParameter_H
#define VEvndispRunParameter_H

#include <TDatime.h>
#include <TNamed.h>
#include <TROOT.h>
#include <TSystem.h>

#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

#include "VGlobalRunParameter.h"
#include "VImageCleaningRunParameter.h"

using namespace std;

class VEvndispRunParameter : public TNamed, public VGlobalRunParameter
{
    public:

// local host parameters
        string fEventDisplayUser;
        string fEventDisplayHost;
	string fEventDisplayDate;

// computer system parameters
        string fEventDisplayBuildCompiler;
        string fEventDisplayBuildCompilerVersion;
	string fEventDisplayBuildArch;
	string fEventDisplayBuildNode;
	string fEventDisplayBuildDir;
	SysInfo_t *fEventDisplaySystemInfo;

// root parameters
        string fEventDisplayBuildROOTVersion;
	int    fEventDisplayBuildROOTVersionInt;

// DB parameters
        bool   fuseDB;
	string fDBRunType;                        // run type according to DB
	string fDBRunStartTimeSQL;                // run start (SQL type)
	string fDBRunStoppTimeSQL;                // run stopp (SQL type)

// run parameters
        int    frunnumber;                        // runnumber
        int    frunmode;                          // run mode
	                                          //    (0=analysis, 1=pedestal calculation, 2=gain/toffset calculation,
						  //    5 = gain/toffset calculation for low gain channels, 6=pedestals for low gain channels)
        int    fsourcetype;                       // source type (0=rawdata,1=GrIsu,2=MC in vbf format,3=Rawdata in vbf,4=DST,5=multiple GrIsu file)
        bool   fRunIsZeroSuppressed;              // run is zero suppressed
        string fsourcefile;                       // name of data file
        int    fnevents;                          // total number of events to be analyzed

// output parameters
        bool   ffillhistos;                       // fill some analysis histograms
        string foutputfileName;                   // file name for analysis output (root file), -1 = no output

// debugging
        bool  fDebug;                             // print debug output
        unsigned int fPrintAnalysisProgress;      // print a line each time this number of events have been processed

// array/telescope geometry parameters
        unsigned int fNTelescopes;                // number of telescopes
        vector<string> fcamera;                   // name of camera configuration files
        vector< unsigned int > fTelToAnalyze;     // analyze only this telescope (Telescope 1 = 0!! )
	bool   fUseVBFSampleLength;               // use number of samples from VBF file (ignore .cfg file)

// pointing parameters
        string fTargetName;                       // target name
        double fTargetDec;                        // target declination [deg] (J2000)
        double fTargetRA;                         // target RA [deg] (J2000)
        double fTargetDecOffset;                  // target offset in dec [deg]
        double fTargetRAOffset;                   // target offset in RA [deg]
        double fWobbleNorth;                      // wobble offset NORTH (deg)
        double fWobbleEast;                       // wobble offset EAST (deg)
        double felevation;                        // elevation in [deg] (preli)
        double fazimuth;                          // azimuth in [deg] (preli)
        bool fDBTracking;                         // use tracking from DB
        string fDBTrackingCorrections;            // apply tracking corrections from this period (SQL time string), empty for use of measured values in db
        string fPMTextFileDirectory;              // pointing monitor text file directory
        bool fDBVPM;                              // use calibrated VPM tracking from database    
	bool fDBUncalibratedVPM;                  // use uncalibrated VPM tracking from database
        vector<double> fPointingErrorX;           // pointing error, in camera coordinates [deg]
        vector<double> fPointingErrorY;           // pointing error, in camera coordinates [deg]
        double fCheckPointing;                    // stop run if pointing difference between calculated pointing direction and vbf is larger than this value
	bool fDBCameraRotationMeasurements;       // read camera rotations from DB

// calibration (pedestals, gains, etc.)
	bool   fcalibrationrun;                   // true if this is a pedestal/gain/toff calculation run
        string fcalibrationfile;                  // file with file names for calibration
	bool freadCalibfromDB;                     // if true, calibration information are read in VOFFLINE DB
	int freadCalibfromDB_versionquery;         // require a given version of calibration
	bool freadCalibfromDB_save_file;           // calibration information read from the DB are stored in VGlobalRunParameter::getDirectory_EVNDISPCalibrationData() +/Tel_?
	string fLowGainCalibrationFile;           // file with file name for low-gain calibration 
	int fNCalibrationEvents;                  // events to be used for calibration 
        vector< int > fGainFileNumber;
        vector< int > fTOffFileNumber;
        vector< int > fPedFileNumber;
	vector< int > fTZeroFileNumber;
        vector< int > fPedLowGainFileNumber;
        vector< int > fGainLowGainFileNumber;
        vector< int > fTOffLowGainFileNumber;
	vector< int > fLowGainMultiplierFileNumber;
	vector< int > fTZeroLowGainFileNumber;
        vector< int > fPixFileNumber;
        vector< int > fPadFileNumber;
        string fDeadChannelFile;
        float fCameraCoordinateTransformX;        // multiply all X coordinates in camera by this value
        float fCameraCoordinateTransformY;        // multiply all Y coordinates in camera by this value
        vector< double > fGainCorrection;         // all gains are divided by this number
	bool   fIgnoreDSTGains;
        double fLaserSumMin;                      // minimal total charge sum for a event to be a laser event
        bool   fUsePedEvents;                     // use only true pedestal events (evttype=2) for pedestal analysis (default: off )
        bool   fLowGainPeds;                      // calculate pedestals from low gain channels only
        bool   fUsePedestalsInTimeSlices;         // use pedestal in time slices for image calculation (high gain)
        bool   fLowGainUsePedestalsInTimeSlices;  // use pedestals in time slices for image calculation (low gain)
        bool   fPedestalsInTimeSlices;            // calculate pedestal variations for tracking tests
        double fPedestalsLengthOfTimeSlice;       // length of a time slice for pedestal calculations (tracking test)
        int    fCalibrationSumWindow;             // sumwindow for all calibration calculation
        int    fCalibrationSumFirst;              // starting point all calibration calculation
	float  fCalibrationIntSumMin;             // minimum integrated charge in a channel and event to be taken into account in gain or tzero calibration runs
        string fsetSpecialChannels;               // set channels with L2 channels to correct for FADC crate time offsets (file name of channel settings)
        bool   fL2TimeCorrect;                    // use L2 pulses to correct FADC times (default: on )
	unsigned fCalibrationDataType;            // for DSTs: kind of calibration data available: 1: full (peds, pedvars, etc). 0: (no calibration data)

// FADC integration
	string      fFADCChargeUnit;              // FADC charge unit (DC or PE)
	vector< unsigned int > fTraceIntegrationMethod;   // trace integration method
	vector< unsigned int > fTraceIntegrationMethod_pass1;   // trace integration method for pass 1 (doublepass only)
        vector<int> fsumfirst;                    // parameter for window summation start (window 1)
        vector<int> fsumwindow_1;                 // parameter for window summation (window 1)
        vector<int> fsumwindow_2;                 // parameter for window summation (window 2)
	vector<int> fsumwindow_pass1;             // parameter for window summation (double pass - pass 1)
	bool   fFixWindowStart_sumwindow2;        // fixed window start for summation window 2
        bool   fFixWindowStart;                   // fix the location of the window (do not move depending on t0)
        bool   fDoublePass;                       // double pass image cleaning (default: off )
	bool   fDynamicIntegrationWindow;         // use a dynamic integration window (doublepass only)
        vector< int >    fTraceWindowShift;       // shift the summation window by value (in doublepass: low gain channels only, default: 0 )
                                                  // maximum difference between doublepass calculated window start and t0 (in samples, default: 10 )
        vector< double > fDBSumWindowMaxTimedifference;
        double fSumWindowStartAtT0Min;            // for pulses with peaks larger than this values, start summation window at t0 + fTraceWindowShift (doublepass only)
        double ftracefit;                         // tracefit mode or getquick mode (-1.=no fitting, 0=fit all PMTs, else: fit only PMTs with maximum ftracefit x tracerms
        string ftracefitfunction;                 // number of tracefit function (default=ev, others: grisu);
	bool   fperformFADCAnalysis;              // run FADC analysis (important e.g. for CTA DST files, where sim_tel results are available as well )

// FADC timing parameters
        vector< float > fpulsetiminglevels;       // levels at which timing of FADC pulses is calculated
	unsigned int fpulsetiming_tzero_index;
	unsigned int fpulsetiming_width_index;
	unsigned int fpulsetiming_max_index;

// image cleaning
        vector< VImageCleaningRunParameter* >  fImageCleaningParameters;

// image analysis
        int    fImageLL;                          // loglikelihood image parameterisation 0=off/1=on/2=verbose mode (default: 0=off )
        vector< float > fLogLikelihoodLoss_min;   // do loglikelihood image parameterisation if loss is larger than this value
	vector< int > fLogLikelihood_Ntubes_min; //  do loglikelihood image parameterisation if ntubes is larger than this value
        float  fImageAnalysisFUIFactor;           // FUI factor for image analysis
        bool   frecoverImagePixelNearDeadPixel;
        bool   fFillImageBorderNeighbours;
        bool   fSmoothDead;                       // smooth dead pixels (default: off )

// reconstruction parameter file
        string freconstructionparameterfile;      // reconstruction parameter file
// MC parameters
        string fsimu_pedestalfile;                // use external pedestal file for MC
        int    fsimu_noiselevel;                  // noise level used for this external file
        double fsimu_pedestalfile_DefaultPed;     // default pedestal for Grisu MC (must be the same is in grisu .cfg file)
        int    fIsMC;                             // source data is Monte Carlo (==1 write all MC events, ==2 write only triggered events)
        bool   fIgnoreCFGversions;                // ignore configuration file versions

        int fgrisuseed;
        int ftelescopeNOffset;                    // offset in telescope numbering (default: first tele = 0 ), for GrIsu Reader
        int fsampleoffset;                        // offset in samples
	unsigned int fMC_FADCTraceStart;          // start of FADC trace (in case the simulated trace is longer than needed)
        string ftracefile;                        // background trace library file for MC
        bool fMCnoDead;                           // ignore all dead channel labels in camera configuration files
        int fMCNdead;                             // number of pixels set randomly dead
        int fMCNdeadSeed;                         // seed for setting pixels randomly dead
        int fMCNdeadboard;                        // number of boards set randomly dead (10 dead pixels in a row)
        double fMCScale;                          // scale factor for MC data

// tree filling
        unsigned int fShortTree;                  // 0: full tree; 1: short tree
	unsigned int fwriteMCtree;                // 0: do not write MC tree
        bool fWriteTriggerOnly;                   // true: write triggered events for simulation only
	bool fFillMCHistos;                       // true: fill MC histograms with thrown events

// display parameters
        bool   fdisplaymode;                      // display mode or command line mode
        bool   floopmode;                         // infinite event loop
        unsigned int fw;                          // main window width
        unsigned int fh;                          // main window height
        bool fShowPhotoDiode;                     // show photodiode signal
        bool fPlotRaw;                            // plot raw values only, used fpeds = 0
        bool fPlotPaper;                          // clean up plots for papers and talks (no dead channels, no small text, ...)
        unsigned int fPlotAllInOneMethod;         // from which method are the angular reconstruction results to taken to plot in all in one window

// muon parameters
        bool fmuonmode;                           // search for complete muon rings, Martin

// Frogs parameters
        bool ffrogsmode;                          // for Frogs template Analysis, GH
	string ffrogsmscwfile;			  // frogs file for getting table energy
	int ffrogsRecID;			  // RecID or Cut_ID Frogs Uses - combine with table

// write pulse histograms to gain files
        int  fwriteLaserPulseN;                    // number of pulse histogram written to gain file
	bool fwriteAverageLaserPulse;              // write average laser pulse to file

// dst parameters
        string fdstfile;                          // dst output file name (root file)
        int fdstminntubes;                        // write only events with more than fdstminntubes ntubes into dst file
        bool fdstwriteallpixel;                   // write all information of all pixel into dst output files

// Parallaxwidth
        int fPWmethod;                            // how to make the trigger-map to calculate the trigger-level image parameters
        int fPWcleanNeighbors;                    // number of neighbors required for a center pixel to survive the cleaning procedure
        float fPWcleanThreshold;                  // cleaning threshold to use to determine hit pixels from the summed FADC charge (dc)
        int fPWlimit;                             // limits the number of pixels transmitted per sector, if =0, then the function is ignored and no cut is applied on the generation of the trigger map

// functions
        void print();
        void print( int iEV );
	void printCTA_DST();

        VEvndispRunParameter();
        ~VEvndispRunParameter() {}

	bool         doFADCAnalysis() { return fperformFADCAnalysis; }
	void         setPulseZeroIndex();
	void         setSystemParameters();

        ClassDef(VEvndispRunParameter,119); //(increase this number)
};
#endif
