//! VEvndispData  central event data class
// Revision $Id: VEvndispData.h,v 1.39.2.1.2.1.2.5.10.7.2.7.4.5.2.3.2.6.2.1.2.1.2.1.2.4.2.7.2.6.2.3.2.1 2011/04/21 10:03:37 gmaier Exp $

#ifndef VDATA_H
#define VDATA_H

#include <VImageAnalyzerData.h>
#include <VEvndispReconstructionParameter.h>
#include <VCalibrationData.h>
#include <VDeadChannelFinder.h>
#include <VDetectorGeometry.h>
#include <VDetectorTree.h>
#include <VDSTReader.h>
#include <VGrIsuReader.h>
#include <VMCParameters.h>
#include <VMultipleGrIsuReader.h>
#ifndef NOVBF
#include <VBaseRawDataReader.h>
#endif
#include <VPEReader.h>
#include <VEvndispRunParameter.h>
#include <VFitTraceHandler.h>
#include <VShowerParameters.h>
#include <VFrogParameters.h>
#include <VSkyCoordinates.h>
#include <VTraceHandler.h>

#include "TDirectory.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TTree.h"

#include <bitset>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class VEvndispData
{
    private:

    protected:
        static bool fDebug;
        static int  fDebugLevel;
        static int  fNDebugMessages;

// global run parameters
        static int fRunNumber;                    //!< run number
        static VEvndispRunParameter *fRunPar;          //!< all command line/configuration parameters

// telescope data
        static unsigned int fNTel;                //!< total number of telescopes
        static unsigned int fTelID;               //!< telescope number of current telescope
        static vector< unsigned int > fTeltoAna;  //!< analyze only this subset of telescopes (this is dynamic and can change from event to event)
// telescope pointing (one per telescope)
        static vector< VSkyCoordinates* > fPointing;
// no pointing information
        static bool fNoTelescopePointing;
// cameras
        static VDetectorGeometry *fDetectorGeo;
        static VDetectorTree* fDetectorTree;

// reader
        VVirtualDataReader  *fReader;
        static VGrIsuReader *fGrIsuReader;
        static VMultipleGrIsuReader *fMultipleGrIsuReader;
#ifndef NOVBF
        static VBaseRawDataReader *fRawDataReader;
#endif
        static VDSTReader    *fDSTReader;
        static VPEReader *fPEReader;

// event data
        static unsigned int fEventNumber;         //!< current event number (array event)
                                                  //!< event number of telescope event
        static vector< unsigned int > fTelescopeEventNumber;
        static unsigned int fEventType;           //!< current event type
        static vector< int > fEventMJD;           //!< MJD of current event
        static vector< double > fEventTime;       //!< time of current event

        static vector< vector< int > > fTriggeredTel;
        static vector< int > fTriggeredTelN;

// event status from data reader
        static unsigned long int fExpectedEventStatus;
        static unsigned int fNumberofGoodEvents;
        static unsigned int fNumberofIncompleteEvents;
// event status from analysis
                                                  //!< 0: good event
        static unsigned int fAnalysisArrayEventStatus;
                                                  //!< 0: good event
        static vector< unsigned int > fAnalysisTelescopeEventStatus;

// global trace handler
        static VTraceHandler *fTraceHandler;
        static VFitTraceHandler *fFitTraceHandler;

// calibrator and calibration data
        static vector< bool > fCalibrated;        //!< this telescope is calibrated
                                                  //! data class for calibration data
        static vector< VCalibrationData* > fCalData;
// dead channel finder
        static vector< VDeadChannelFinder* > fDeadChannelDefinition_HG;
        static vector< VDeadChannelFinder* > fDeadChannelDefinition_LG;

//  analysis cuts
                                                  //!< cuts for array analysis
        static VEvndispReconstructionParameter* fArrayAnalysisCuts;

// analysis results
        static TFile *fOutputfile;                //!< root output file for image parameter trees, histograms, etc.
        static vector< TDirectory* > fAnaDir;     //! directories in root file
        static vector< VImageAnalyzerData* > fAnaData; //!< data class with analysis results for each telescope
                                                  //!< data class with analysis results from all telescopes
        static VShowerParameters *fShowerParameters;
        static VFrogParameters *fFrogParameters;
        static VMCParameters *fMCParameters;      //!< data class with MC parameters

// timing results
        static vector< TGraphErrors* > fXGraph;   //!< Long axis timing graph
        static vector< TGraphErrors* > fYGraph;   //!< Short axis timing graph
        static vector< TGraphErrors* > fRGraph;   //!< Radial timing graph

// default pedestals for plotraw option
        static valarray<double> fPlotRawPedestals;

// set detector geometry
        void                setDetectorGeometry( unsigned int iNTel, vector< string > icamera , string idir );

// names of dead channels
        static vector< string > fDeadChannelText;

    public:
        VEvndispData();
        ~VEvndispData() {}
        void                dumpTreeData();       //!< print all tree data to stdout
        void                endOfRunInfo();       //!< print some statistics at end of run
        bool                get_array_analysis_cuts( string ifile );
// getters apply always to current telescope (fTelID) if telID is not a function argument
        VImageAnalyzerData*      getAnaData( unsigned int iTel ) { if( iTel < fAnaData.size() ) return fAnaData[iTel]; }
        VImageAnalyzerData*      getAnaData() { return fAnaData[fTelID]; }
        vector< TDirectory* > getAnaDirectories() { return fAnaDir; }
        VImageAnalyzerHistograms*     getAnaHistos() { return fAnaData[fTelID]->fAnaHistos; }
        VImageAnalyzerHistograms*     getAnaHistos( unsigned int itelID ) { return fAnaData[itelID]->fAnaHistos; }
        VEvndispReconstructionParameter* getArrayAnalysisCuts() { return fArrayAnalysisCuts; }
        unsigned int        getAnalysisArrayEventStatus() { return fAnalysisArrayEventStatus; }
        vector< unsigned int >& getAnalysisTelescopeEventStatus() { return fAnalysisTelescopeEventStatus; }
        vector<bool>&       getBorder() {return fAnaData[fTelID]->fBorder;}
        double              getBorderThresh() { return fRunPar->fborderthresh[fTelID]; }
        double              getBrightNonImageThresh() { return fRunPar->fbrightnonimagetresh[fTelID]; }
        VCalibrationData*   getCalData() { return fCalData[fTelID]; }
        VCalibrationData*   getCalData( unsigned int iTel ) { if( iTel < fCalData.size() ) return fCalData[iTel]; else return 0; }
        VCalibrationData*   getCalibrationData() { return fCalData[fTelID]; }
        VCalibrationData*   getCalibrationData( unsigned int iTel ) { if( iTel < fCalData.size() ) return fCalData[iTel]; else return 0; }
        vector<int>&        getChannelStatus() { return fCalData[fTelID]->fChannelStatus; }
        bool                getCalibrated() { return fCalibrated[fTelID]; }
        VEvndispData*              getData() { return this; }
        vector<unsigned int>&       getDead( bool iLowGain = false ) { if( !iLowGain ) return fAnaData[fTelID]->fDead; else return fAnaData[fTelID]->fLowGainDead; }
        vector<unsigned int>&       getMasked() { return fAnaData[fTelID]->fMasked; }
        vector<bool>&       getDeadRecovered( bool iLowGain = false ) { if( !iLowGain ) return fAnaData[fTelID]->fDeadRecovered; else return fAnaData[fTelID]->fLowGainDeadRecovered; }
        VDeadChannelFinder* getDeadChannelFinder( bool iLowGain = false );
        unsigned int        getNDead( bool iLowGain = false ) { if( !iLowGain ) return fAnaData[fTelID]->fNDead; else return fAnaData[fTelID]->fLowGainNDead; }
        vector<unsigned int>&       getDeadUI( bool iLowGain = false ) { if( !iLowGain ) return fAnaData[fTelID]->fDeadUI; else return fAnaData[fTelID]->fLowGainDeadUI; }
        vector<string>&     getDeadChannelText() { return fDeadChannelText; }
        bool                getDebugFlag() { return fDebug; }
        int                 getDebugLevel();
        VDetectorGeometry*  getDetectorGeo() const { return fDetectorGeo; }
        VDetectorGeometry*  getDetectorGeometry() const { return fDetectorGeo; }
        TTree*              getDetectorTree();
        int                 getEventMJD() { return fEventMJD[fTelID]; }
        vector< int >&      getEventMJDVector() { return fEventMJD; }
        unsigned int        getEventNumber() { return fEventNumber; }
        string              getEventDisplayVersion() { return getRunParameter()->getEVNDISP_VERSION(); }
        double              getEventTime() { return fEventTime[fTelID]; }
        vector< double >&   getEventTimeVector() { return fEventTime; }
        unsigned int        getEventType() { return fEventType; }
        unsigned long int   getExpectedEventStatus() { return fExpectedEventStatus; }
        valarray<double>&   getFADCStopOffsets() { return fCalData[fTelID]->fFADCStopOffsets; }
        vector< double >&   getFADCstopSums() { return fAnaData[fTelID]->fFADCstopSum; }
        vector< unsigned int >&  getFADCstopTrig() { return fAnaData[fTelID]->getFADCstopTrigChannelID(); }
        vector< double >&   getFADCstopTZero() { return fAnaData[fTelID]->fFADCstopTZero; }
        bool                getFillMeanTraces() { return fAnaData[fTelID]->fFillMeanTraces; }
        bool                getFillPulseSum() { return fAnaData[fTelID]->fFillPulseSum; }
        TH1F*               getGainDist( bool iLowGain = false ) {  if( !iLowGain ) return fCalData[fTelID]->fGainsDistribution; else return fCalData[fTelID]->fLowGainGainsDistribution; }
        TH1F*               getGainVarsDist( bool iLowGain = false ) { if( !iLowGain ) return fCalData[fTelID]->fGainVarsDistribution; else return fCalData[fTelID]->fLowGainGainVarsDistribution; }
        valarray<double>&   getGains( bool iLowGain = false ) { if( !iLowGain ) return fCalData[fTelID]->fGains; else return fCalData[fTelID]->fLowGainGains; }
	valarray< bool >&   getGains_DefaultValue(  bool iLowGain = false ) { if( !iLowGain ) return fCalData[fTelID]->fGains_DefaultSetting; else return fCalData[fTelID]->fLowGainGains_DefaultSetting; }
        valarray<double>&   getGainvars( bool iLowGain = false ) { if( !iLowGain ) return fCalData[fTelID]->fGainvars; else return fCalData[fTelID]->fLowGainGainvars; }
	double              getHIGHQE_gainfactor( unsigned int iChannel ) { return fAnaData[fTelID]->getHIGHQE_gainfactor( iChannel ); }
        vector<bool>&       getHiLo() { return fAnaData[fTelID]->fHiLo; }
        VImageAnalyzerHistograms*     getHistograms() { return fAnaData[fTelID]->fAnaHistos; }
        vector<bool>&       getImage() { return fAnaData[fTelID]->fImage; }
        vector<bool>&       getImageBorderNeighbour() { return fAnaData[fTelID]->fImageBorderNeighbour; }
        VImageParameter*   getImageParameters() { return fAnaData[fTelID]->fImageParameter; }
        VImageParameter*   getImageParameters( int );
        VImageParameter*   getImageParametersLogL() { return fAnaData[fTelID]->fImageParameterLogL; }
        double              getImageThresh() { return fRunPar->fimagethresh[fTelID]; }
        vector<int>&        getImageUser() { return fAnaData[fTelID]->fImageUser; }
        vector<bool>&       getLLEst() { return fAnaData[fTelID]->fLLEst; }
        TList*              getIntegratedChargeHistograms() { return fAnaData[fTelID]->getIntegratedChargeHistograms(); }
        double              getMeanLowGainMultiplier( bool bSmall = false ) { if( !bSmall ) return fCalData[fTelID]->fmeanLowGainMult; else return fCalData[fTelID]->fmeanLowGainMultSmall; }
        double              getRMSLowGainMultiplier( bool bSmall = false ) { if( !bSmall ) return fCalData[fTelID]->frmsLowGainMult; else return fCalData[fTelID]->frmsLowGainMultSmall; }
        valarray< double >& getLowGainMultiplier( bool bSmall = false ) { if( !bSmall ) return fCalData[fTelID]->fLowGainMult; else return fCalData[fTelID]->fLowGainMultSmall; }
        valarray< double >& getLowGainMultiplierError( bool bSmall = false ) { if( !bSmall ) return fCalData[fTelID]->fLowGainMultError; else return fCalData[fTelID]->fLowGainMultErrorSmall; }
        bool                getLowGainPedestals() { return fCalData[fTelID]->fBoolLowGainPedestals; }
        bool                getLowGainGains() { return fCalData[fTelID]->fBoolLowGainGains; }
        bool                getLowGainTOff() { return fCalData[fTelID]->fBoolLowGainTOff; }
        VMCParameters*      getMCParameters() { return fMCParameters; }
	unsigned int        getMC_FADC_TraceStart() { return fRunPar->fMC_FADCTraceStart; }

        double              getmeanPedvars( bool iLowGain = false, unsigned int iSumWindow = 0,  bool iSumWindowSmall = false ) { return getCalData()->getmeanPedvars( iLowGain, iSumWindow, iSumWindowSmall ); }
        double              getmeanRMSPedvars( bool iLowGain = false, unsigned int iSumWindow = 0,  bool iSumWindowSmall = false ) { return getCalData()->getmeanRMSPedvars( iLowGain, iSumWindow, iSumWindowSmall, getEventTime() ); }
        void                getmeanPedvars( double &imean, double &irms, bool iLowGain = false, unsigned int iSumWindow = 0,  bool iSumWindowSmall = false, double iTime = -99. ) { if( iTime < -90. ) getCalData()->getmeanPedvars( imean, irms, iLowGain, iSumWindow, iSumWindowSmall, getEventTime() ); else getCalData()->getmeanPedvars( imean, irms, iLowGain, iSumWindow, iSumWindowSmall, getEventTime() ); }

        vector< double >&   getmeanPedvarsAllSumWindow( bool iLowGain = false ) { if( !iLowGain ) return fCalData[fTelID]->fVmeanPedvars; else return fCalData[fTelID]->fVmeanLowGainPedvars; }
        vector< double >&   getmeanRMSPedvarsAllSumWindow( bool iLowGain = false ) { if( !iLowGain ) return fCalData[fTelID]->fVmeanRMSPedvars; else return fCalData[fTelID]->fVmeanRMSLowGainPedvars; }
        TList*              getMeanPulseHistograms() { return fAnaData[fTelID]->getMeanPulseHistograms(); }
        unsigned int        getNChannels() { return fDetectorGeo->getNChannels( fTelID ); }
        vector<bool>&       getBrightNonImage() { return fAnaData[fTelID]->fBrightNonImage; }
        bool                getNoPointing() { return fNoTelescopePointing; }
        unsigned int        getNSamples() { return fDetectorGeo->getNSamples( fTelID ); }
        unsigned int        getNTel() const { return fNTel; }
        TFile*              getOutputFile() { return fOutputfile; }
        bool                getPedsFromPLine() { return fCalData[fTelID]->fPedFromPLine; }
	double              getPed_min( bool iLowGain = false ) { return fCalData[fTelID]->getPed_min( iLowGain ); }
	double              getPed_max( bool iLowGain = false ) { return fCalData[fTelID]->getPed_max( iLowGain ); }
        TH1F*               getPedDist( bool iLowGain = false ) { if( !iLowGain ) return fCalData[fTelID]->fPedDistribution; else return fCalData[fTelID]->fLowGainPedDistribution; }
        TH1F*               getPedvarsDist( bool iLowGain = false ) { if( !iLowGain ) return fCalData[fTelID]->fPedvarsDistribution; else return fCalData[fTelID]->fLowGainPedvarDistribution; }
        TH1F*               getPedLowGainDist() { return fCalData[fTelID]->fLowGainPedDistribution; }
        TH1F*               getPedvarsLowGainDist() { return fCalData[fTelID]->fLowGainPedvarDistribution; }

///////////////// pedestals /////////////////////////////////
// getters for pedestals
        valarray<double>&   getPeds( bool iLowGain = false, double iTime = -99. );
        valarray<double>&   getPedsLowGain( double iTime = -99. ) { return getPeds( true, iTime ); }

// getters for pedestal variation
        valarray<double>&   getPedvars( bool iLowGain = false, unsigned int iSW = 0, bool iSumWindowSmall = false, double iTime = -99. );
        valarray<double>&   getPedvars( unsigned int iSW, bool iLowGain = false ) { return getPedvars( iLowGain, iSW ); }
        valarray<double>&   getPedvarsSmall( bool iLowGain = false ) { return getPedvars( iLowGain, getSumWindowSmall(), true ); }
        valarray<double>&   getPedvarsLowGain() { return getPedvars( true ); }
        valarray<double>&   getPedvarsLowGainSmall() { return getPedvars( true, getSumWindowSmall(), true ); }

        vector< valarray<double> >& getPedvarsAllSumWindows(  bool iLowGain = false ) { if( !iLowGain ) return fCalData[fTelID]->fVPedvars; else return fCalData[fTelID]->fVLowGainPedvars; }
// getter for pedestal rms
        valarray<double>&   getPedrms( bool iLowGain = false ) { if( !iLowGain ) return fCalData[fTelID]->fPedrms; else return fCalData[fTelID]->fLowGainPedsrms; }
        valarray<double>&   getPedsLowGainrms() { return fCalData[fTelID]->fLowGainPedsrms; }
// padding stuff (probably out of date)
/////////////// end pedestals //////////////////////////////

        valarray<double>&   getRawTZeros() { return fAnaData[fTelID]->getTZeros( false ); }
        VVirtualDataReader* getReader() { return fReader; }
        int                 getRunNumber() { return fRunNumber; }
        VEvndispRunParameter*    getRunParameter() { return fRunPar; }
        VShowerParameters*  getShowerParameters() { return fShowerParameters; }
        VFrogParameters*    getFrogParameters() { return fFrogParameters; }
        unsigned int        getSumFirst() { return fRunPar->fsumfirst[fTelID]; }
        valarray<double>&   getSums() { return fAnaData[fTelID]->fSums; }
        unsigned int        getSumWindow() { return fRunPar->fsumwindow[fTelID]; }
        unsigned int        getSumWindowSmall() { return fRunPar->fsumwindowsmall[fTelID]; }
        unsigned int        getSumWindow( unsigned int iTelID ) { if( iTelID < fRunPar->fsumwindow.size() ) return fRunPar->fsumwindow[iTelID]; else return 0; }
        unsigned int        getSumWindowSmall( unsigned int iTelID ) { if( iTelID < fRunPar->fsumwindowsmall.size() ) return fRunPar->fsumwindowsmall[iTelID]; else return 0; }
        valarray< unsigned int >& getCurrentSumWindow() { return fAnaData[fTelID]->fCurrentSummationWindow; }
        int                 getSumWindowShift() { return fRunPar->fTraceWindowShift[fTelID]; }
        double              getDBSumWindowMaxTimedifference() { return fRunPar->fDBSumWindowMaxTimedifference[fTelID]; }
        valarray<unsigned int>& getTCorrectedSumFirst() { return fAnaData[fTelID]->fTCorrectedSumFirst; }
        valarray<unsigned int>& getTCorrectedSumLast() { return fAnaData[fTelID]->fTCorrectedSumLast; }
        unsigned int        getTelescopeEventNumber( unsigned int iTelID ) { if( iTelID < fTelescopeEventNumber.size() ) return fTelescopeEventNumber[iTelID]; else return 0; }
        vector< unsigned int >& getTelescopeEventNumber() { return fTelescopeEventNumber; }
        bool                getTelescopeStatus( unsigned int iTelID );
        unsigned int        getTelID() { return fTelID; }
        unsigned int        getTeltoAnaID() { return getTeltoAnaID( fTelID ); }
        unsigned int        getTeltoAnaID( unsigned int iTelID );
        vector< unsigned int>& getTeltoAna() { return fTeltoAna; }
        double              getTimeSinceRunStart() { return fAnaData[fTelID]->fTimeSinceRunStart; }
        TH1F*               getToffsetDist( bool iLowGain = false ) { if( !iLowGain ) return fCalData[fTelID]->fTOffsetsDistribution; else return fCalData[fTelID]->fLowGainTOffsetsDistribution; }
        TH1F*               getToffsetVarsDist( bool iLowGain = false ) { if( !iLowGain ) return fCalData[fTelID]->fTOffsetVarsDistribution; else return fCalData[fTelID]->fLowGainTOffsetVarsDistribution; }
        valarray<double>&   getTOffsets( bool iLowGain = false ) { if( !iLowGain ) return fCalData[fTelID]->fTOffsets; else return fCalData[fTelID]->fLowGainTOffsets; }
        valarray<double>&   getTOffsetvars( bool iLowGain = false ) { if( !iLowGain ) return fCalData[fTelID]->fTOffsetvars; else return fCalData[fTelID]->fLowGainTOffsetvars; }
        double              getTraceFit() { return fRunPar->ftracefit; }
        valarray<double>&   getTraceFitChi2() { return fAnaData[fTelID]->fChi2; }
        valarray<double>&   getTraceFitFallTime() { return fAnaData[fTelID]->fFallTime; }
        valarray<double>&   getTraceFitFallTimeParameter() { return fAnaData[fTelID]->fFallTimePar; }
        valarray<double>&   getTraceFitNorm() { return fAnaData[fTelID]->fTraceNorm; }
        valarray<double>&   getTraceFitRiseTime() { return fAnaData[fTelID]->fRiseTime; }
        valarray<double>&   getTraceFitRiseTimeParameter() { return fAnaData[fTelID]->fRiseTimePar; }
        valarray<double>&   getTraceMax() { return fAnaData[fTelID]->fTraceMax; }
        valarray<unsigned int>& getTraceN255() { return fAnaData[fTelID]->fTraceN255; }
        valarray<double>&   getTraceRawMax() { return fAnaData[fTelID]->fRawTraceMax; }
        valarray<double>&   getTraceWidth() { return fAnaData[fTelID]->getTraceWidth( true ); }
        VTraceHandler*      getTraceHandler() { return fTraceHandler; }
        vector<bool>&       getTrigger()          // MS
        {
            return fAnaData[fTelID]->fTrigger;
        }
        vector< vector< int > >& getTriggeredTel() { return fTriggeredTel; }
        vector< int >&      getTriggeredTelN() { return fTriggeredTelN; }
        VFitTraceHandler*   getFitTraceHandler() { return fFitTraceHandler; }
	vector< valarray< double > >& getPulseTiming( bool iCorrected = true );
        valarray<double>&   getTZeros() { return fAnaData[fTelID]->getTZeros( true ); }
        TGraphErrors*       getXGraph() { return fXGraph[fTelID]; }
        TGraphErrors*       getYGraph() { return fYGraph[fTelID]; }
        TGraphErrors*       getRGraph() { return fRGraph[fTelID]; }
        vector< VSkyCoordinates* >    getPointing() { return fPointing; }
        void                incrementNumberofIncompleteEvents() { fNumberofIncompleteEvents++; }
        void                incrementNumberofGoodEvents() { fNumberofGoodEvents++; }
                                                  //!< set pointer to data reader
        bool                initializeDataReader();
        bool                initializeDeadChannelFinder();
        bool                isMC() { return getReader()->isMC(); }
	bool                isDST_MC() { if( isMC() && (fRunPar->fsourcetype == 4 || fRunPar->fsourcetype == 7) ) return true; else return false; }
	bool                isTeltoAna( unsigned int iTel );
        void                printDeadChannels( bool iLowGain = false );
        void                resetAnaData();
        void                setAnalysisArrayEventStatus( unsigned int i ) { fAnalysisArrayEventStatus = i; }
        void                setAnaData() { fAnaData[fTelID]->initialize( fDetectorGeo->getNChannels( fTelID ), getReader()->getMaxChannels(), (getTraceFit()>-1.), getDebugFlag(), getRunParameter()->fMCNdeadSeed, getNSamples(), getRunParameter()->fpulsetiminglevels.size(), getRunParameter()->fpulsetiming_tzero_index, getRunParameter()->fpulsetiming_width_index ); }
        void                setBorder( bool iBo ) {  fAnaData[fTelID]->fBorder.assign( fDetectorGeo->getNChannels( fTelID ), iBo ); }
        void                setBorder( unsigned int iChannel, bool iBo ) { fAnaData[fTelID]->fBorder[iChannel] = iBo; }
        void                setBorderThresh( double ithresh ) { fRunPar->fborderthresh[fTelID] = ithresh; }
        void                setBrightNonImageThresh( double ithresh ) { fRunPar->fbrightnonimagetresh[fTelID] = ithresh; }
        void                setCalData() { fCalData[fTelID]->initialize( fDetectorGeo->getNChannels( fTelID ), getDebugFlag() ); }
        void                setCalibrated() { if( fTelID < fCalibrated.size() ) fCalibrated[fTelID] = true; }
        void                setCalibrated( bool iCal ) { if( fTelID < fCalibrated.size() ) fCalibrated[fTelID] = iCal; }
        void                setCurrentSummationWindow( unsigned int iw ) { fAnaData[fTelID]->fCurrentSummationWindow = iw; }
        void                setCurrentSummationWindow( unsigned int imin, unsigned int imax );
        void                setCurrentSummationWindow( unsigned int iChannel, unsigned int imin, unsigned int imax );
        void                setDead( unsigned int iDead, bool iLowGain = false ) { if( !iLowGain ) fAnaData[fTelID]->fDead.assign( fDetectorGeo->getNChannels( fTelID ), iDead ); else fAnaData[fTelID]->fLowGainDead.assign( fDetectorGeo->getNChannels( fTelID ), iDead ); }
        void                setDead( unsigned int iChannel, unsigned int iDead, bool iLowGain = false, bool iFullSet = false, bool iReset = false );
        void                setNDead( unsigned int iN, bool iLowGain = false ) { if( !iLowGain ) fAnaData[fTelID]->fNDead = iN; else fAnaData[fTelID]->fLowGainNDead = iN; }
        void                setDeadChannelText();
        void                setFADCStopOffsets( double iOffset ) { fCalData[fTelID]->fFADCStopOffsets = iOffset; }
        void                setFADCStopOffsets( unsigned int iChannel, double iOffset ) { fCalData[fTelID]->fFADCStopOffsets[iChannel] = iOffset; }
        void                setGains( double iGain, bool iLowGain = false ) { if( !iLowGain ) fCalData[fTelID]->fGains = iGain; else fCalData[fTelID]->fLowGainGains; }
        void                setGains( unsigned int iChannel, double iGain, bool iLowGain = false ) { if( !iLowGain ) fCalData[fTelID]->fGains[iChannel] = iGain; else fCalData[fTelID]->fLowGainGains[iChannel] = iGain; }
	void                setGains_DefaultValue( bool iV, bool iLowGain = false ) { if( !iLowGain ) fCalData[fTelID]->fGains_DefaultSetting = iV; else fCalData[fTelID]->fLowGainGains_DefaultSetting = iV; }
        void                setGainvars( unsigned int iChannel, double iGainvar, bool iLowGain = false ) { if( !iLowGain ) fCalData[fTelID]->fGainvars[iChannel] = iGainvar; else fCalData[fTelID]->fLowGainGainvars[iChannel] = iGainvar; }
        void                setGainvars( double iGainvar, bool iLowGain = false ) { if( !iLowGain ) fCalData[fTelID]->fGainvars = iGainvar; else fCalData[fTelID]->fLowGainGainvars = iGainvar; }
        void                setHiLo( bool iHL ) {  fAnaData[fTelID]->fHiLo.assign( fDetectorGeo->getNChannels( fTelID ), iHL ); }
        void                setHiLo( unsigned int iChannel, bool iHL ) {  fAnaData[fTelID]->fHiLo[iChannel] = iHL; }
        void                setHistoFilling( bool ifill) { fRunPar->ffillhistos = ifill; }
        void                setImage( bool iIm ) {  fAnaData[fTelID]->fImage.assign( fDetectorGeo->getNChannels( fTelID ), iIm ); }
        void                setImageBorderNeighbour( bool iIm ) {  fAnaData[fTelID]->fImageBorderNeighbour.assign( fDetectorGeo->getNChannels( fTelID ), iIm ); }
        void                setImage( unsigned int iChannel, bool iIm ) {  fAnaData[fTelID]->fImage[iChannel] = iIm; }
        void                setImageThresh( double ithresh ) { fRunPar->fimagethresh[fTelID] = ithresh; }
        void                setImageUser( int iIu ) { fAnaData[fTelID]->fImageUser.assign( fDetectorGeo->getNChannels( fTelID ), iIu ); }
        void                setImageUser( unsigned int iChannel, int iIu ) { fAnaData[fTelID]->fImageUser[iChannel] = iIu; }
        void                setLowGainPedestals() { fCalData[fTelID]->fBoolLowGainPedestals = true; }
        void                setLowGainGains() { fCalData[fTelID]->fBoolLowGainGains = true; }
        void                setLowGainTOff() { fCalData[fTelID]->fBoolLowGainTOff = true; }
        void                setLLEst( vector<bool> iEst ) { fAnaData[fTelID]->fLLEst = iEst; }
        void                setLowGainMultiplier( double iV, unsigned int iChannel, bool iSmall = false ) { fCalData[fTelID]->setLowGainMultiplier( iV, iChannel, iSmall ); }
        void                setMeanLowGainMultiplier( double g, bool bSmall = false ) { if( !bSmall ) fCalData[fTelID]->fmeanLowGainMult = g; else fCalData[fTelID]->fmeanLowGainMultSmall = g; }
        void                setRMSLowGainMultiplier( double g, bool bSmall = false ) { if( !bSmall ) fCalData[fTelID]->frmsLowGainMult = g; else fCalData[fTelID]->frmsLowGainMultSmall = g; }
        void                setNChannels( unsigned int iChan ) { fDetectorGeo->setNChannels( fTelID, iChan ); }
        void                setBrightNonImage( bool iIm ) {  fAnaData[fTelID]->fBrightNonImage.assign( fDetectorGeo->getNChannels( fTelID ), iIm ); }
        void                setBrightNonImage( unsigned int iChannel, bool iIm ) {  fAnaData[fTelID]->fBrightNonImage[iChannel] = iIm; }
        void                setNoPointing( bool iP ) { fNoTelescopePointing = iP; }
        void                setNSamples( unsigned int iSamp ) { fDetectorGeo->setNSamples( fTelID, iSamp, fRunPar->fUseVBFSampleLength ); }
        void                setNSamples( unsigned int iTelID, unsigned int iSamp ) { fDetectorGeo->setNSamples( iTelID, iSamp, fRunPar->fUseVBFSampleLength ); }
        void                setDebugLevel( int i );
/////////////// time image cleaning /////////////////////
	double           getTimeCutPixel() { return fRunPar->ftimecutpixel[fTelID]; }; //HP
	double           getTimeCutCluster() { return fRunPar->ftimecutcluster[fTelID]; }; //HP
	int              getMinNumPixelsInCluster() { return fRunPar->fminpixelcluster[fTelID]; }; //HP
	int              getNumLoops() { return fRunPar->floops[fTelID]; }; //HP

	void             setClusterNpix( int iID, int clusterNpix )    { fAnaData[fTelID]->fClusterNpix[iID] = clusterNpix; } //HP
        vector<int>&     getClusterNpix() { return fAnaData[fTelID]->fClusterNpix;  } //HP
        void             setClusterID( unsigned int iChannel, int iID ) { fAnaData[fTelID]->fClusterID[iChannel] = iID; } //HP
        vector<int>&     getClusterID() { return fAnaData[fTelID]->fClusterID;  } //HP
        void             setMainClusterID( int iID ) { fAnaData[fTelID]->fMainClusterID = iID; } //HP
	int              getMainClusterID() { return fAnaData[fTelID]->fMainClusterID; } //HP

	void             setClusterSize( int iID, double clustersize ) { fAnaData[fTelID]->fClusterSize[iID] = clustersize; } //HP
        vector<double>&  getClusterSize() { return fAnaData[fTelID]->fClusterSize;  } //HP
	void             setClusterTime( int iID, double clustertime ) { fAnaData[fTelID]->fClusterTime[iID] = clustertime; } //HP
        vector<double>&  getClusterTime() { return fAnaData[fTelID]->fClusterTime;  } //HP

	void             setClusterCenx( int iID, double clustercenx ) { fAnaData[fTelID]->fClusterCenx[iID] = clustercenx; } //HP
        vector<double>&  getClusterCenx() { return fAnaData[fTelID]->fClusterCenx;  } //HP
	void             setClusterCeny( int iID, double clusterceny ) { fAnaData[fTelID]->fClusterCeny[iID] = clusterceny; } //HP
	vector<double>&  getClusterCeny() { return fAnaData[fTelID]->fClusterCeny;  } //HP

	void             setNcluster_cleaned( int i_Ncluster ) { fAnaData[fTelID]->fncluster_cleaned = i_Ncluster; }; //HP
	int              getNcluster_cleaned() { return fAnaData[fTelID]->fncluster_cleaned; }; //HP
	void             setNcluster_uncleaned( int i_Ncluster ) { fAnaData[fTelID]->fncluster_uncleaned = i_Ncluster; }; //HP
	int              getNcluster_uncleaned() { return fAnaData[fTelID]->fncluster_uncleaned; }; //HP
/////////////// pedestals /////////////////////
        void                setPeds( unsigned int iChannel, double iPed, bool iLowGain = false ) { fCalData[fTelID]->setPeds( iChannel, iPed, iLowGain ); }
/////////////// end pedestals /////////////////////
        void                setPedsFromPLine() { fCalData[fTelID]->fPedFromPLine = true; }
        void                setRootDir( unsigned int iTel, TDirectory *iDir ) { fAnaDir[iTel] = iDir; }
        void                setRunNumber( int iRunN ) { fRunNumber = iRunN; }
        void                setSumFirst( int iSum ) { fRunPar->fsumfirst[fTelID] = iSum; }
        void                setSums( double iSum ) { fAnaData[fTelID]->fSums = iSum; }
        void                setSums( unsigned int iChannel, double iSum ) { fAnaData[fTelID]->fSums[iChannel] = iSum; }
        void                setSums( valarray< double > iVSum ) { fAnaData[fTelID]->fSums = iVSum; }
        void                setSumWindow( int iWindow ) { fRunPar->fsumwindow[fTelID] = iWindow; }
        void                setTCorrectedSumFirst( unsigned int iT ) { fAnaData[fTelID]->fTCorrectedSumFirst = iT; }
        void                setTCorrectedSumFirst( unsigned int iChannel, unsigned int iT ) { fAnaData[fTelID]->fTCorrectedSumFirst[iChannel] = iT; }
        void                setTCorrectedSumLast( unsigned int iT ) { fAnaData[fTelID]->fTCorrectedSumLast = iT; }
        void                setTCorrectedSumLast( unsigned int iChannel, unsigned int iT ) { fAnaData[fTelID]->fTCorrectedSumLast[iChannel] = iT; }
        void                setTelID( unsigned int iTel );
        void                setTeltoAna( vector< unsigned int > iT );
        void                setTOffsets( double iToff, bool iLowGain = false ) { if( !iLowGain ) fCalData[fTelID]->fTOffsets = iToff; else fCalData[fTelID]->fLowGainTOffsets = iToff; }
        void                setTOffsets( unsigned int iChannel, double iToff, bool iLowGain = false ) { if( !iLowGain ) fCalData[fTelID]->fTOffsets[iChannel] = iToff; else fCalData[fTelID]->fLowGainTOffsets[iChannel] = iToff; }
        void                setTOffsetvars( double iToffv, bool iLowGain = false ) { if( !iLowGain ) fCalData[fTelID]->fTOffsetvars = iToffv; else fCalData[fTelID]->fLowGainTOffsetvars = iToffv; }
        void                setTOffsetvars( unsigned int iChannel, double iToffv, bool iLowGain = false ) { if( !iLowGain ) fCalData[fTelID]->fTOffsetvars[iChannel] = iToffv; else fCalData[fTelID]->fLowGainTOffsetvars[iChannel] = iToffv; }
        void                setTrace( unsigned int iChannel, vector< double > fT, bool iHiLo, double iPeds ) { fAnaData[fTelID]->setTrace( iChannel, fT, iHiLo, iPeds ); }
        void                setTraceChi2( unsigned int iChannel, double iS ) { fAnaData[fTelID]->fChi2[iChannel] = iS; }
        void                setTraceChi2( double iV ) { fAnaData[fTelID]->fChi2 = iV; }
        void                setTraceFallTime( unsigned int iChannel, double iS ) { fAnaData[fTelID]->fFallTime[iChannel] = iS; }
        void                setTraceFallTime( double iV ) { fAnaData[fTelID]->fFallTime = iV; }
        void                setTraceFallTimeParameter( unsigned int iChannel, double iS ) { fAnaData[fTelID]->fFallTimePar[iChannel] = iS; }
        void                setTraceFallTimeParameter( double iV ) { fAnaData[fTelID]->fFallTimePar = iV; }
        void                setTraceMax( unsigned int iChannel, double iS ) { fAnaData[fTelID]->fTraceMax[iChannel] = iS; }
        void                setTraceMax( double iV ) { fAnaData[fTelID]->fTraceMax= iV; }
        void                setTraceMax( valarray< double > iV ) { fAnaData[fTelID]->fTraceMax = iV; }
        void                setTraceN255( unsigned int iS ) { fAnaData[fTelID]->fTraceN255 = iS; }
        void                setTraceN255( unsigned int iChannel, unsigned int iS ) { fAnaData[fTelID]->fTraceN255[iChannel] = iS; }
        void                setTraceRawMax( unsigned int iChannel, double iS ) { fAnaData[fTelID]->fRawTraceMax[iChannel] = iS; }
        void                setTraceRawMax( double iV ) { fAnaData[fTelID]->fRawTraceMax= iV; }
        void                setTraceRawMax( valarray< double > iV ) { fAnaData[fTelID]->fRawTraceMax = iV; }
        void                setTraceNorm( unsigned int iChannel, double iS ) { fAnaData[fTelID]->fTraceNorm[iChannel] = iS; }
        void                setTraceNorm( double iV ) { fAnaData[fTelID]->fTraceNorm = iV; }
        void                setTraceRiseTime( unsigned int iChannel, double iS ) { fAnaData[fTelID]->fRiseTime[iChannel] = iS; }
        void                setTraceRiseTime( double iV ) { fAnaData[fTelID]->fRiseTime = iV; }
        void                setTraceRiseTimeParameter( unsigned int iChannel, double iS ) { fAnaData[fTelID]->fRiseTimePar[iChannel] = iS; }
        void                setTraceRiseTimeParameter( double iV ) { fAnaData[fTelID]->fRiseTimePar = iV; }
        void                setTraceWidth( double iV ) { fAnaData[fTelID]->fTraceWidth = iV; }
        void                setTraceWidth( unsigned int iChannel, double iS ) { fAnaData[fTelID]->fTraceWidth[iChannel] = iS; }
        void                setTraceWidth( valarray< double > iV ) { fAnaData[fTelID]->fTraceWidth = iV; }
        void                setTrigger( bool iIm )// MS
        {
            fAnaData[fTelID]->fTrigger.assign( fDetectorGeo->getNChannels( fTelID ), iIm );
        }
                                                  // MS
        void                setTrigger( unsigned int iChannel, bool iIm )
        {
            fAnaData[fTelID]->fTrigger[iChannel] = iIm;
        }
	void                setPulseTiming( vector< valarray< double > > iPulseTiming, bool iCorrected );
	void                setPulseTiming( float iTZero, bool iCorrected );
	void                setPulseTiming( unsigned int iChannel, vector< float > iTZero, bool iCorrected );
	void                setPulseTimingCorrection( unsigned int iChannel, double iCorrection );
        void                setXGraph(TGraphErrors* igraph) {fXGraph[fTelID]=igraph;}
        void                setYGraph(TGraphErrors* igraph) {fYGraph[fTelID]=igraph;}
        void                setRGraph(TGraphErrors* igraph) {fRGraph[fTelID]=igraph;}
        bool                usePedestalsInTimeSlices( bool iLowGain = false ) { if( !iLowGain ) return getRunParameter()->fUsePedestalsInTimeSlices; else return getRunParameter()->fLowGainUsePedestalsInTimeSlices; }

        void                testDataReader();     //!< check if reader is available, set pointers
};
#endif
