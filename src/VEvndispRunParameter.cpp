/*! \class VEvndispRunParameter
  \brief input parameter storage class

  \date
  28/04/2004

  Revision $Id: VEvndispRunParameter.cpp,v 1.1.2.9.10.11.2.7.4.2.4.8.2.5.4.5.2.7.2.8.2.12.2.7 2011/04/06 11:57:57 gmaier Exp $

  \author
  Gernot Maier
*/

#include "VEvndispRunParameter.h"

ClassImp(VEvndispRunParameter)

VEvndispRunParameter::VEvndispRunParameter()
{
    SetName( "runparameterV2" );
    SetTitle( getEVNDISP_VERSION().c_str() );
    fEventDisplayUser = "";
    fEventDisplayHost = "";
    fEventDisplayDate = "";
// system parameters
    fEventDisplayBuildCompiler = "";
    fEventDisplayBuildCompilerVersion = "";
    fEventDisplayBuildArch = "";
    fEventDisplayBuildNode = "";
    fEventDisplayBuildDir = "";
    fEventDisplayBuildROOTVersion = "";
    fEventDisplayBuildROOTVersionInt = 0;
    fEventDisplaySystemInfo = 0;

// debug parameters
    fDebug = false;

// run parameters
#ifdef RUNWITHDB
    fuseDB = true;
#else
    fuseDB = false;
#endif
    frunmode = 0;
    fRunIsZeroSuppressed = false;
    frunnumber = -1;
    fsourcetype = 3;                              // 0 = rawdata, 1 = GrIsu simulation, 2 = MC in VBF format, 3 = rawdata in VBF, 4 = DST, 5 = multiple GrIsu file, 6 = PE file
    fsourcefile = "";
    fDBRunType = "";
    fDBRunStartTimeSQL = "";
    fDBRunStoppTimeSQL = "";
    fsimu_pedestalfile = "";
    fsimu_noiselevel   = 250;
    fsimu_pedestalfile_DefaultPed = 20.;
    fnevents = -10000;
    fIsMC = 0;
    fIgnoreCFGversions = false;
    fPrintAnalysisProgress = 25000;

// geometry/calibration parameters
    fNTelescopes = 4;                             // there is always at least one telescope
    fcamera.push_back( "veritasBC4N_090916_Autumn2009-4.1.5_EVNDISP.cfg" );
    fCalibrationDataType = 1;  // should be 0 for e.g. CTA DSTs
    fcalibrationfile = "";
    fLowGainCalibrationFile = "";
    fcalibrationrun = false;
    fLaserSumMin = 50000.;
    fGainFileNumber.push_back( 0 );
    fTOffFileNumber.push_back( 0 );
    fPedFileNumber.push_back( 0 );
    fLowGainMultiplierFileNumber.push_back( 0 );
    fPedLowGainFileNumber.push_back( 0 );
    fGainLowGainFileNumber.push_back( 0 );
    fTOffLowGainFileNumber.push_back( 0 );
    fPixFileNumber.push_back( 0 );

    fTelToAnalyze.push_back( 0 );

    fDeadChannelFile = "EVNDISP.validchannels.dat";

    fCameraCoordinateTransformX = 1.;
    fCameraCoordinateTransformY = 1.;

// pointing parameters
    fTargetName = "NONAME";
    fTargetDec = -999.;
    fTargetRA = -999.;
    fTargetDecOffset = 0;
    fTargetRAOffset = 0;
    felevation = -999.;
    fazimuth = -999.;
    fWobbleNorth = 0.;
    fWobbleEast = 0.;
    fCheckPointing = 999.;

#ifdef RUNWITHDB
    fDBTracking = true;
    fDBCameraRotationMeasurements = true;
    fDBVPM = false;
#else
    fDBTracking = false;
    fDBCameraRotationMeasurements = false;
    fDBVPM = false; 
#endif
    fDBTrackingCorrections = "";
    fPMTextFileDirectory = "";
    fPointingErrorX.push_back( 0. );
    fPointingErrorY.push_back( 0. );

    fGainCorrection.push_back( 1. );

    fLowGainPeds = true;

// analyzer parameters
    fUseFixedThresholds = false;
    fimagethresh.push_back( 5.0 );
    fborderthresh.push_back( 2.5 );
    fbrightnonimagetresh.push_back( 2.5 );
    fUseTimeCleaning = false;         //HP
    ftimecutpixel.push_back( 0.5 );   //HP
    ftimecutcluster.push_back( 2.0 ); //HP
    fminpixelcluster.push_back( 3 );  //HP
    floops.push_back( 2 );            //HP
    fsumfirst.push_back( 2 );
    fsumwindow.push_back( 18 );
    fsumwindowsmall.push_back( 12 );
    fImageLL=0;
    fLogLikelihoodLoss_min = 1.e3;
    fImageAnalysisFUIFactor = 2.;
    fFixWindowStart = false;
    fDoublePass = true;
    fDynamicIntegrationWindow = true;
    fRemoveIsolatedPixel = true;
    fFillImageBorderNeighbours = true;
    fTraceWindowShift.push_back( -1 );
    fDBSumWindowMaxTimedifference.push_back( 10. );
    fSumWindowStartAtT0Min = 1.e9;
    fSmoothDead = false;
    fUsePedEvents = true;
    fFADCChargeUnit = "DC";
    fperformFADCAnalysis = true;
// pedestal calculation in time slices
    fLowGainUsePedestalsInTimeSlices = false;
    fUsePedestalsInTimeSlices = true;
    fPedestalsInTimeSlices = true;
    fPedestalsLengthOfTimeSlice = 180.;            //!< [s]
    fPedestalsInTimeSlicesSumWindow = 4;
    fPedestalsInTimeSlicesSumFirst = 0;
    fL2TimeCorrect = true;
    fsetSpecialChannels = "EVNDISP.specialchannels.dat";
    ftracefit = -1.;
    ftracefitfunction = "ev";
    farrayanalysiscutfile = "EVNDISP.reconstruction.runparameter";

////////////////////////////////////////////////////////////////////////////////
// pulse timing (fraction of maximum where times are determined)
// OBSERVE: there should be a timing level with value 0.5 (for tzero calculations)
// OBSERVE: this vector is always symmetric around the entry at 1
//    fpulsetiminglevels.push_back( 0.2 );
    fpulsetiminglevels.push_back( 0.5 );
//    fpulsetiminglevels.push_back( 0.8 );
    fpulsetiminglevels.push_back( 1.0 );
    unsigned int i_fps = fpulsetiminglevels.size();
    if( i_fps > 1 )
    {
       for( unsigned int i = 0; i < i_fps - 1; i++ )
       {
	    fpulsetiminglevels.push_back( fpulsetiminglevels[i_fps-i-2] );
       }
    }
// get index for tzero and width
    setPulseZeroIndex();

    fWriteTriggerOnly = true;
    fShortTree = 0;
    fwriteMCtree = 1;
    fFillMCHistos = true;

// muon parameters
    fmuonmode = false;
    ffrogsmscwfile = "";

// Frogs parameters
    ffrogsmode = false;
    ffrogsRecID = -1;

// output parameters
    ffillhistos = false;                          // obsolete
    foutputfileName = "";

// MC parameters
// offset in telescope numbering (0 for old grisudet version (<3.0.0))
    ftelescopeNOffset = 1;
    fsampleoffset = 0;
    fUseVBFSampleLength = false;
    fMC_FADCTraceStart = 0;
    fgrisuseed = 0;
    ftracefile = "";
    fMCnoDead = false;
    fMCNdead = 0;
    fMCNdeadSeed = 0;
    fMCNdeadboard = 0;
    fMCScale = 1.;

// display parameters
    fdisplaymode = false;
    floopmode = false;
    fh = 680;                                     // height of main window
    fw = 1000;                                    // width of main window
    fh = 650;                                     // height of main window
    fw = 975;                                     // width of main window
    fShowPhotoDiode = false;
    fPlotRaw = false;
    fPlotPaper = false;
    fPlotAllInOneMethod = 0;

// internal parameters
    fwritepulses = 0;

// dst parameters
    fdstfile = "";
    fdstminntubes = -1;
    fdstwriteallpixel = true;

// parallaxwidth  // MS
    fPWmethod = -1;                               // MS default is to use cleaned CFD trigger map
    fPWcleanNeighbors = 2;                        // MS: default number of neighbors required for identifying center pixels in the trigger map
    fPWcleanThreshold = 26.0;                     // MS: default is about 5.3 dc/pe for VERITAS (5 sample integration window), i.e. cleaning of ~5 pe
    fPWlimit = 0;                                 // MS: default is no restriction on the number of trigger pixels transmitted to moment-generating function

}


void VEvndispRunParameter::print()
{
    print( 1 );
}


/*!
   \param iEv = 0: print important parameters only
              = 1: print in eventdisplay run
              = 2: print all parameters
*/
void VEvndispRunParameter::print( int iEv )
{
    if( fDebug ) cout << "VEvndispRunParameter::printParams()" << endl;

    cout << endl;
    if( iEv == 1 )
    {
        cout << "\t -----------------------------------------" << endl;
        if( frunmode == 0 && !fdisplaymode  )    cout << "\t       ANALYZING DATA" << endl;
	else if( frunmode == 0 && fdisplaymode ) cout << "\t       DISPLAYING DATA" << endl;
        else if( frunmode == 1 ) cout << "\t       CALCULATING PEDESTALS" << endl;
        else if( frunmode == 2 ) cout << "\t       CALCULATING GAINS AND TIME OFFSETS (high gain channels)" << endl;
        else if( frunmode == 3 ) cout << "\t       WRITING TRACE LIBRARY" << endl;
        else if( frunmode == 4 ) cout << "\t       WRITING DATA SUMMARY FILES" << endl;
        else if( frunmode == 5 ) cout << "\t       CALCULATING GAINS AND TIME OFFSETS (low gain channels)" << endl;
        else if( frunmode == 6 ) cout << "\t       CALCULATING PEDESTALS (low gain channels)" << endl;
        cout << "\t -----------------------------------------" << endl << endl;
    }
    if( iEv == 2 )
    {
        cout << "Eventdisplay version: " << getEVNDISP_VERSION() << endl;
        cout << "============================" << endl << endl;
    }

    cout << "RUN " << frunnumber << endl;
    cout << "Observatory: " << getObservatory();
    cout << " (lat " << getObservatory_Latitude_deg() << ", long " << getObservatory_Longitude_deg() << ", height " << getObservatory_Height_m() << "m)";
    cout << endl;
    cout << "File: " << fsourcefile << " (sourcetype " << fsourcetype;
    cout << ")" << endl;
    cout << "===========" << endl;
    cout << fEventDisplayDate << endl;
    if( fIsMC) cout << ", MC";
    if( fEventDisplayUser.size() > 0 ) cout << "User: " << fEventDisplayUser << " ";
    cout << "Host: " << fEventDisplayHost << endl;
    if( fEventDisplayBuildNode.size() > 0 ) cout << fEventDisplayBuildNode << endl;
    if( fEventDisplayBuildArch.size() > 0 ) cout << fEventDisplayBuildArch << "\t";
    if( fEventDisplayBuildCompiler.size() > 0 ) cout << fEventDisplayBuildCompiler << "\t";
    if( fEventDisplayBuildCompilerVersion.size() > 0 ) cout << fEventDisplayBuildCompilerVersion << endl;
    if( fEventDisplayBuildDir.size() > 0 )  cout << fEventDisplayBuildDir << endl;
    if( fEventDisplaySystemInfo )
    {
       cout << fEventDisplaySystemInfo->fModel << ", " << fEventDisplaySystemInfo->fOS;
       cout << ", " << fEventDisplaySystemInfo->fCpuType << ", " << fEventDisplaySystemInfo->fCpuSpeed << " MHz";
       cout << ", " << fEventDisplaySystemInfo->fPhysRam << " MB" << endl;
    }
    if( fEventDisplayBuildROOTVersion.size() > 0 ) cout << "ROOT version " << fEventDisplayBuildROOTVersion << endl;
       
    cout << endl;
    if( fTargetName.size() > 0 ) cout << "Target: " << fTargetName;
    if( fTargetDec > -99 ) cout << "\t Target: (dec=" << fTargetDec << ", ra=" << fTargetRA << ")" << endl;
    cout << "\t offsets (ra,dec): " <<  fTargetRAOffset << ", " << fTargetDecOffset << endl;
    cout << "\t wobble (north,east): " << fWobbleNorth << ", " << fWobbleEast << endl;
    cout << "\t pointing corrections (x,y): ";
    if( !fDBTracking )
    {
        for( unsigned int i = 0; i < fTelToAnalyze.size(); i++ )
        {
            cout << "\t T" << fTelToAnalyze[i]+1 << ": " << fPointingErrorX[fTelToAnalyze[i]] << ", " << fPointingErrorY[fTelToAnalyze[i]];
        }
    }
    else
    {
        cout << " use database" << endl;
    }
    if( fDBCameraRotationMeasurements ) cout << "using camera rotation values from DB" << endl;
    cout << endl;
    cout << endl;
    cout << "analyzing following telescope: ";
    for( unsigned int i = 0; i < fTelToAnalyze.size(); i++ ) cout << fTelToAnalyze[i]+1 << ", ";
    cout << endl;
    cout << "detector configuration file: " << fcamera[0];
    if( fIgnoreCFGversions ) cout << " (ignoring cfg version numbering)";
    cout << endl;
    if( fUseVBFSampleLength ) cout << "\t using number of FADC samples from cfg file" << endl;
    cout << endl;
    cout << "runmode: " << frunmode << endl;
    if( fnevents > 0 ) cout << "number of events to analyse: " << fnevents << endl;
    if( frunmode == 4 ) cout << "dstfile: " << fdstfile << " (mintubes: " << fdstminntubes << ")" << endl;
    cout << endl;
    if( fcalibrationfile.size() > 0 )         cout << "calibration file: " << fcalibrationfile << endl;
    if( fLowGainCalibrationFile.size() > 0 )  cout << "calibration file (low gain): " << fLowGainCalibrationFile << endl;
    else if( frunmode != 2 && frunmode != 5 && !fIsMC ) cout << "reading laser/flasher run numbers from database" << endl;
    if( frunmode == 2 ) cout << "lasermin: " << fLaserSumMin << endl;
    if( ( fsourcetype == 1 || fsourcetype == 2 || fsourcetype == 5 ) && fsimu_pedestalfile.size() > 0 )
    {
        cout << "calculate pedestals from " << fsimu_pedestalfile;
        cout << " with noise level " << fsimu_noiselevel;
        cout << " (default ped: " << fsimu_pedestalfile_DefaultPed << ")" << endl;
    }
    else if( fsourcetype == 1 ) cout << "calculate pedestals from " << fsourcefile << endl;
    if( frunmode == 1 && fUsePedEvents ) cout << "using pedestal events for pedestal calculation" << endl;
    if( frunmode == 6 )  cout << "using low gain events only for pedestal calculation" << endl;
    cout << endl;

    if( fCalibrationDataType == 0 ) cout << "no calibration data available" << endl;
    cout << "signal charge unit is " << fFADCChargeUnit << endl;
    if( fperformFADCAnalysis )   cout << "analysing FADC traces " << endl;
    else if( fsourcetype == 7 )  cout << "reading trace analysis results from DST file" << endl;

    if( frunmode == 0 || frunmode == 4 )
    {
        if( fUsePedestalsInTimeSlices || fLowGainUsePedestalsInTimeSlices ) cout << "pedestals in time slices: ";
        else cout << "pedestals constant over run" << endl;
        if( fUsePedestalsInTimeSlices ) cout << "high gain";
        if( fLowGainUsePedestalsInTimeSlices ) cout << " low gain";
        cout << endl;
        cout << "setting special channels (e.g. with L2 signal): " << fsetSpecialChannels << endl;
        cout << "pulse timing levels: ";
        for( unsigned int i = 0; i < fpulsetiminglevels.size(); i++ ) cout << fpulsetiminglevels[i] << ", ";
        cout << endl;
        cout << "correcting FADC times for crate jitter with L2 signals: " << fL2TimeCorrect << endl;
        if( fDoublePass )
        {
	    cout << "double pass cleaning ";
	    if( fDynamicIntegrationWindow ) cout << " (dynamical integration window) ";
            cout << " (low gain window shift: ";
            for( unsigned int i = 0; i < fTelToAnalyze.size(); i++ ) cout << fTraceWindowShift[i] << ", ";
            cout << ")";
        }
        else cout << "no double pass cleaning";
        cout << endl;
        if( fFixWindowStart )        cout << "using fixed window start" << endl;
	if( fMC_FADCTraceStart > 0 ) cout << "MC trace start: " << fMC_FADCTraceStart  << endl;
        if( ftracefit > -1. )        cout << "trace fitting: " << ftracefit << " with " << ftracefitfunction << endl;
        if( fSmoothDead )            cout << "smoothing dead pixels" << endl;
        if( fmuonmode )              cout << "muon ring analysis: " << fmuonmode << endl;
        if( fImageLL != 0 )
	{
	                             cout << "loglikelihood fitting of images: " << fImageLL;
                                     cout << " (using these images for the array reconstruction)";
                                     cout << endl;
        }
        if( fLogLikelihoodLoss_min < 1. ) cout << "loglikelihood fitting of images for images with loss > " << fLogLikelihoodLoss_min << endl;
        cout << "Fraction of image/border pixel under image ellipse fact (FUI-factor): " << fImageAnalysisFUIFactor << endl;
    }
    if( ftracefile.size() > 0 )      cout << "\t tracelib file: " << ftracefile << endl;
    if( fsourcetype == 1 || fsourcetype == 5 ) cout << "telescope numbering offset: " << ftelescopeNOffset << endl;
    if( fMCNdead > 0 ) cout << "Random dead channels: " << fMCNdead << " (seed " <<  fMCNdeadSeed << "), " << fMCNdeadboard << endl;
    if( fPlotPaper ) cout << " (paper plotting mode)";
    cout << endl;

    cout << "directories:" << endl;
    cout << "\t analysis data: " << getDirectory_EVNDISPAnaData() << endl;
    if( fsourcetype == 0 || fsourcetype == 2 || fsourcetype == 3 )
    {
       cout << "\t raw data: " << getDirectory_VBFRawData() << endl;
    }
    cout << "\t output: " << getDirectory_EVNDISPOutput() << endl;
    if( fShortTree ) cout << "shortened tree output " << endl;

// print analysis parameters
    if( iEv == 2 )
    {
        cout << endl;
        if( fUseFixedThresholds ) cout << "using fixed image/border thresholds" << endl;
        if( fUseTimeCleaning ) cout << "using time cleaning" << endl; //HP
                                                  // MS
        cout << "Parallaxwidth: trigger map input type: "<< fPWmethod <<endl;
                                                  // MS
        cout << "Parallaxwidth: number of neighbors required for cleaning: "<< fPWcleanNeighbors << endl;
        if( fPWmethod ==3 )
                                                  // MS
            cout << "Parallaxwidth: FADC cleaning threshold for identifying triggered pixels (for method 3): "<< fPWcleanThreshold << endl;
        for( unsigned int i = 0; i < fTelToAnalyze.size(); i++ )
        {
            cout << "Telescope " << i+1 << ": image/border/brightnonimage " << fimagethresh[i] << "/" << fborderthresh[i] << "/" << fbrightnonimagetresh[i];
            cout << ", window size/start: " << fsumwindow[i] << "/" << fsumfirst[i];
            if( fDoublePass )
            {
                cout << ", window size (doublepass): " << fsumwindowsmall[i];
                cout << ", window shift: " << fTraceWindowShift[i];
                cout << ", max TD: " << fDBSumWindowMaxTimedifference[i];
                cout << ", max T0 threshold " << fSumWindowStartAtT0Min;
            }
	    if( fUseFixedThresholds ) cout << " (fixed cleaning thresholds)" << endl;
	    if( fUseTimeCleaning ) cout << " (time cleaning)" << endl; //HP
            cout << endl;
            cout << "\t\t";
            cout << "pedestal file: " << fPedFileNumber[i];
            if( fPedLowGainFileNumber[i] > 0 ) cout << ", low gain pedestal file: " << fPedLowGainFileNumber[i];
            cout << ", gain file: " << fGainFileNumber[i];
            if( fGainLowGainFileNumber[i] > 0 ) cout << ", low gain gain file: " << fGainLowGainFileNumber[i];
            if( i < fGainCorrection.size() && TMath::Abs( fGainCorrection[i] - 1. ) > 0.001 ) cout << " (gain correction: " << fGainCorrection[i]  << ")";
            cout << ", toff file: " << fTOffFileNumber[i];
            if( fTOffLowGainFileNumber[i] > 0 ) cout << ", low gain toff file: " << fTOffLowGainFileNumber[i];
            cout << ", pixel file: " << fPixFileNumber[i] << endl;
        }
    }

}

void VEvndispRunParameter::setPulseZeroIndex()
{
    fpulsetiming_tzero_index = 9999;
    fpulsetiming_width_index = 9999;
    fpulsetiming_max_index   = 9999;
    for( unsigned int i = 0; i < fpulsetiminglevels.size(); i++ )
    {
       if( TMath::Abs( fpulsetiminglevels[i] - 1. ) < 1.e-4 && fpulsetiming_max_index == 9999 )
       {
          fpulsetiming_max_index = i;
	  break;
       }
    }
    for( unsigned int i = 0; i < fpulsetiminglevels.size(); i++ )
    {
       if( TMath::Abs( fpulsetiminglevels[i] - 0.5 ) < 1.e-4 && fpulsetiming_tzero_index == 9999 )
       {
          fpulsetiming_tzero_index = i;
       }
       else if( TMath::Abs( fpulsetiminglevels[i] - 0.5 ) < 1.e-4 && fpulsetiming_tzero_index != 9999 )
       {
          fpulsetiming_width_index = i;
	  break;
       }
    }
}

void VEvndispRunParameter::printCTA_DST()
{
    cout << "Eventdisplay version: " << getEVNDISP_VERSION() << endl;
    cout << "============================" << endl << endl;
    cout << fEventDisplayDate << endl;

    cout << "Observatory: " << getObservatory() << endl;

    cout << "RUN " << frunnumber << endl;
    cout << endl;
    cout << "source file " << fsourcefile << endl;
    cout << "number of telescope: " << fNTelescopes << endl;
    cout << "pulse timing levels: ";
    for( unsigned int i = 0; i < fpulsetiminglevels.size(); i++ ) cout << fpulsetiminglevels[i] << ", ";
    cout << "L2 timing correct: " << fL2TimeCorrect << endl;
    cout << endl;
}

void VEvndispRunParameter::setSystemParameters()
{
// get date
   TDatime t_time;
   fEventDisplayDate = t_time.AsSQLString();

// get host name
    fEventDisplayHost = gSystem->HostName();;
// get user name
    if( gSystem->GetUserInfo() )
    {
       fEventDisplayUser = gSystem->GetUserInfo()->fUser;
    }
// get system parameters
    fEventDisplayBuildCompiler = gSystem->GetBuildCompiler();
    fEventDisplayBuildCompilerVersion = gSystem->GetBuildCompilerVersion();
    fEventDisplayBuildArch = gSystem->GetBuildArch();
    fEventDisplayBuildNode = gSystem->GetBuildNode();
    fEventDisplayBuildDir = gSystem->GetBuildDir();
    gSystem->GetSysInfo( fEventDisplaySystemInfo );
// get root info
    fEventDisplayBuildROOTVersion = gROOT->GetVersion();
    fEventDisplayBuildROOTVersionInt = gROOT->GetVersionInt();
}
