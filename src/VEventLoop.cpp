/*!  \class VEventLoop
  \brief  main event loop, steering of analysis and event display

  \author Gernot Maier
*/

#include "VEventLoop.h"

//! standard constructor
/*!
    \param irunparameter Pointer to run parameters (from command line or config file)
*/
VEventLoop::VEventLoop( VEvndispRunParameter *irunparameter )
{
    fDebug = irunparameter->fDebug;
    if( fDebug ) cout << "VEventLoop::VEventLoop()" << endl;

    fRunPar = irunparameter;

// total number of telescopes
    fNTel = fRunPar->fNTelescopes;
    if( fNTel < 1 )
    {
       cout << "VEventLoop::VEventLoop error: no telescopes defined" << endl;
       exit( -1 );
    }

// data readers
    fReader = 0;                                  // this pointer is used in the program for accesing any data source (raw or MC)
#ifndef NOVBF
    fRawDataReader = 0;
#endif
    fGrIsuReader = 0;
    fDSTReader = 0;
    fPEReader = 0;

    bCheckTelescopePositions = true;
    fBoolPrintSample.assign( fNTel, true );

    fAnalyzeMode = true;
    fRunMode = (E_runmode)fRunPar->frunmode;
    fEventNumber = 0;
    fNextEventStatus = false;
    fEndCalibrationRunNow = false;
    fBoolSumWindowChangeWarning = 0;

    setRunNumber( fRunPar->frunnumber );

// get the detector settings from the configuration files and set the cameras
    setDetectorGeometry( fNTel, fRunPar->fcamera, fRunPar->getDirectory_EVNDISPDetectorGeometry() );

// set which telescopes should be analyzed
    setTeltoAna( fRunPar->fTelToAnalyze );

// set dead channel text
    setDeadChannelText();

// read reconstruction parameters
    if( !get_reconstruction_parameters( fRunPar->freconstructionparameterfile ) )
    {
       cout << "VEventLoop error while reading file with reconstruction parameters:" << endl;
       cout << fRunPar->freconstructionparameterfile << endl;
       exit( -1 );
    }

// set standard tracehandler
    if( fRunPar->ftracefit < 0. ) fTraceHandler = new VTraceHandler();
// set fit tracehandler
    else
    {
        if( fDebug ) cout << "setting fit tracehandler" << endl;
        fFitTraceHandler = new VFitTraceHandler( fRunPar->ftracefitfunction );
        fFitTraceHandler->setFitThresh( fRunPar->ftracefit );
        fTraceHandler = (VTraceHandler*)fFitTraceHandler;
    }
    if( getRunParameter()->fTraceIntegrationMethod.size() > 0 ) fTraceHandler->setTraceIntegrationmethod( getRunParameter()->fTraceIntegrationMethod[0] );
    fTraceHandler->setMC_FADCTraceStart( getRunParameter()->fMC_FADCTraceStart );
    fTraceHandler->setPulseTimingLevels( getRunParameter()->fpulsetiminglevels );

// initialize calibrator (one for all telescopes)
    if( fCalibrated.size() == 0 ) for( unsigned int i = 0; i < fNTel; i++ ) fCalibrated.push_back( false );
    fCalibrator = new VCalibrator();

// create data summarizer
    fDST = new VDST( (fRunMode==R_DST), (fRunPar->fsourcetype==1||fRunPar->fsourcetype==2||fRunPar->fsourcetype==6) );

// create analyzer (one for all telescopes)
    fAnalyzer = new VImageAnalyzer();
// create new pedestal calculator
    fPedestalCalculator = new VPedestalCalculator();

// create array analyzer
    fArrayAnalyzer = new VArrayAnalyzer();

#ifndef NOGSL
// Frogs Stuff
    fFrogs = new VFrogs();
#endif

// reset cut strings and variables
    resetRunOptions();
}


VEventLoop::~VEventLoop()
{
}


/*!
    print basic run infos (file name, runnumber, etc.) to standard output
*/
void VEventLoop::printRunInfos()
{
    if( fDebug ) cout << "VEventLoop::printRunInfos()" << endl;
// telescope parameters
    cout << endl << "Analysis parameters: " << endl;
    for( unsigned int i = 0; i < fRunPar->fTelToAnalyze.size(); i++ )
    {
        setTelID( fRunPar->fTelToAnalyze[i] );

        cout << "Telescope " << fRunPar->fTelToAnalyze[i]+1;
	if( i < getDetectorGeometry()->getTelType().size() ) cout << " (type " << getDetectorGeometry()->getTelType()[i] << ")";
	cout << endl;
	if( fRunPar->fTraceIntegrationMethod[fRunPar->fTelToAnalyze[i]] )
	{
	   cout << "\t trace integration method: \t" << fRunPar->fTraceIntegrationMethod[fRunPar->fTelToAnalyze[i]];
	   if( fRunPar->fDoublePass ) cout << "  (doublepass, integration method pass 1: " << fRunPar->fTraceIntegrationMethod_pass1[fRunPar->fTelToAnalyze[i]] << ")";
	   cout << endl;
	   cout << "\t start of summation window: \t" << fRunPar->fsumfirst[fRunPar->fTelToAnalyze[i]];
	   cout << "\t(shifted by " << fRunPar->fTraceWindowShift[i] << " samples";
	   if( fRunPar->fDoublePass ) cout << ", max T0 threshold " << fRunPar->fSumWindowStartAtT0Min << " d.c.)" << endl;
	   else                       cout << ")" << endl;
	   cout << "\t length of summation window: \t" << fRunPar->fsumwindow_1[fRunPar->fTelToAnalyze[i]];
	   cout << "/" << fRunPar->fsumwindow_2[fRunPar->fTelToAnalyze[i]];
	   if( fRunPar->fDoublePass ) cout << "\t length of first pass summation window (double pass): \t" << fRunPar->fsumwindow_pass1[fRunPar->fTelToAnalyze[i]];
	   cout << endl;
        }
	else
	{
	   cout << "\t no trace integration" << endl;
        }
	getImageCleaningParameter()->print();
        if( getCalData()->getLowGainMultiplierDistribution() && getCalData()->getLowGainMultiplierDistribution()->GetEntries() > 0 )
        {
            cout << "\t low gain multiplier: \t" << setprecision( 3 ) << getCalData()->getLowGainMultiplierDistribution()->GetMean();
	    if( getCalData()->getLowGainMultiplierDistribution()->GetRMS() > 1.e-3 )
	    {
	       cout << "+-" << getCalData()->getLowGainMultiplierDistribution()->GetRMS();
            }
        }
        else cout << "\t (no low gain multiplier distributions)";
	cout << endl;
	if( TMath::Abs( fRunPar->fGainCorrection[fRunPar->fTelToAnalyze[i]] ) - 1. > 1.e-2 )
	{
	   cout << "\t additional gain correction: " << fRunPar->fGainCorrection[fRunPar->fTelToAnalyze[i]];
        }
	cout << "\t LL edge fit: \t\t loss > " << fRunPar->fLogLikelihoodLoss_min[i] << "\t ntubes > " << fRunPar->fLogLikelihood_Ntubes_min[i] << endl;
    }
}


/*!
   reset all run parameters, open files, set eventnumbers, etc.

  handle with file names, get run numbers, set analyzer default values
*/
bool VEventLoop::initEventLoop()
{
    return initEventLoop( fRunPar->fsourcefile );
}


/*!
   reset all run parameters, open files, set eventnumbers, etc.

   handle with file names, get run numbers, set analyzer default values, read calibration files

   \param iFileName data source file
*/
bool VEventLoop::initEventLoop( string iFileName )
{
    if( fDebug ) cout << "VEventLoop::initEventLoop()" << endl;
    fRunPar->fsourcefile = iFileName;
    fEventNumber = 0;

// check if file exists (bizzare return value)
    if( gSystem->AccessPathName( iFileName.c_str() ) && fRunPar->fsourcetype != 5 )
    {
        cout << endl;
        cout << "VEventLoop::initEventLoop error; sourcefile not found: |" << iFileName << "|" << endl;
        exit( 0 );
    }

// set the data readers and open data files
//     (different file formats)
#ifndef NOVBF
    try
    {
// ============================
// sourcefile has raw data format (prototype or vbf)
        if( fRunPar->fsourcetype == 0 || fRunPar->fsourcetype == 2 || fRunPar->fsourcetype == 3 )
        {
            if( fRawDataReader != 0 ) delete fRawDataReader;
            if ( fRunPar->fsourcetype == 0)
            {
                fRawDataReader = new VRawDataReader( fRunPar->fsourcefile, fRunPar->fsourcetype, fRunPar->fNTelescopes, fDebug );
            }
            else
            {
                fRawDataReader = new VBFDataReader( fRunPar->fsourcefile, fRunPar->fsourcetype, fRunPar->fNTelescopes, fDebug );
/////////////////////////////////////////////////////////////////////
// open temporary file (do make sure that event numbering is correct)
// get number of samples
                VBFDataReader i_tempReader( fRunPar->fsourcefile, fRunPar->fsourcetype, fRunPar->fNTelescopes, fDebug );
// loop over several events
// (note: MC might have no telescope event data until the first triggered event)
// (note: zero suppressed data)
		vector< bool > i_nSampleSet( fRunPar->fTelToAnalyze.size(), false );
		unsigned int i_counter = 0;
		for( ;; )
		{
		   i_tempReader.getNextEvent();
		   for( unsigned int i = 0; i < fRunPar->fTelToAnalyze.size(); i++ )
		   {
		     i_tempReader.setTelescopeID( fRunPar->fTelToAnalyze[i] );
// set number of samples
		     if( i_tempReader.getNumSamples() != 0 )
		     {
		        setNSamples( fRunPar->fTelToAnalyze[i], i_tempReader.getNumSamples() );
			i_nSampleSet[i] = true;
                     }
		   }
		   unsigned int z = 0;
		   for( unsigned int i = 0; i < i_nSampleSet.size(); i++ )
		   {
		       if( i_nSampleSet[i] ) z++;
                   }
		   if( z == i_nSampleSet.size() ) break;
		   i_counter++;
		   if( i_counter == 1000 )
		   {
		      cout << "VEventLoop warning: could not find number of samples in the first 1000 events" << endl;
                   }
		   if( i_counter > 9999 ) break;
                } 
		if( getNSamples() == 0 || i_counter > 9999 ) 
		{
		   cout << "VEventLoop::initEventLoop error: could not find any telescope events to determine sample length" << endl;
		   cout << "exciting..." << endl;
		   exit( -1 );
                }
///////////////////////////////////////////////////////////////
            }
// sourcefile is MC vbf file; noise is read from separate file
            if( fRawDataReader && fRunPar->fsourcetype == 2 && fRunPar->fsimu_pedestalfile.size() > 0 )
            {
                fRawDataReader->initTraceNoiseGenerator( 0, fRunPar->fsimu_pedestalfile, getDetectorGeo(), fRunPar->fsumwindow_1, 
		                                         fDebug, fRunPar->fgrisuseed, fRunPar->fsimu_pedestalfile_DefaultPed, fRunPar->fGainCorrection );
            }
        }
    }
// something went wrong, probably wrong filename
    catch (VFileException ex)
    {
        cout << ex.what() << endl;
//      cout << "data file not found, exiting: " << fRunPar->fsourcefile << endl;
// !!! no solution, should be something else
        if( !fRunPar->fdisplaymode ) exit( -1 );
        else return false;
    }
#endif
// ============================
// sourcefile has MC grisu format
    if( fRunPar->fsourcetype == 1 )
    {
	 if( fGrIsuReader != 0 ) delete fGrIsuReader;
	 fGrIsuReader = new VGrIsuReader( getDetectorGeo(), getDetectorGeo()->getNumTelescopes(), fRunPar->fsourcefile, fRunPar->fsumwindow_1, 
	                                  fRunPar->ftelescopeNOffset, fRunPar->fsampleoffset, fRunPar->fMCScale, false, fDebug, fRunPar->fgrisuseed, 
					  fRunPar->fsimu_pedestalfile, fRunPar->fIgnoreCFGversions );
	 fGrIsuReader->setTraceFile( fRunPar->ftracefile );
    }

// ============================
// source has MC grisu format (multiple files)
    else if( fRunPar->fsourcetype == 5 )
    {
	 if( fMultipleGrIsuReader != 0 ) delete fMultipleGrIsuReader;
	 fMultipleGrIsuReader = new VMultipleGrIsuReader( fNTel, "", fRunPar->fTelToAnalyze, fDebug );
	 fMultipleGrIsuReader->init( getDetectorGeo(), fRunPar->fsourcefile, fRunPar->fsumwindow_1, 
	                             fRunPar->ftelescopeNOffset, fRunPar->fsampleoffset, fRunPar->fMCScale, false, 
				     fRunPar->fgrisuseed, fRunPar->fsimu_pedestalfile, true, fRunPar->fsimu_pedestalfile_DefaultPed );
	 fMultipleGrIsuReader->setTraceFile( fRunPar->ftracefile );
    }
// ============================
// sourcefile has DST format
    else if( fRunPar->fsourcetype == 4 || fRunPar->fsourcetype == 7 )
    {
	 if( fDSTReader != 0 ) delete fDSTReader;
	 fDSTReader = new VDSTReader( fRunPar->fsourcefile, fRunPar->fIsMC, fRunPar->fNTelescopes, getNChannels(), fDebug );
	 if( fDSTReader->isMC() && fRunPar->fIsMC == 0 ) fRunPar->fIsMC = 1;
	 for( unsigned int i = 0; i <  fRunPar->fTelToAnalyze.size(); i++ )
	 {
	    fDSTReader->setNumSamples( fRunPar->fTelToAnalyze[i], getNSamples( fRunPar->fTelToAnalyze[i] ) );
         }
    }
// sourcefile has PE format
    else if( fRunPar->fsourcetype == 6 )
    {
	 if( fPEReader != 0 ) delete fPEReader;
	 fPEReader = new VPEReader( fRunPar->fsourcefile, fRunPar->fTelToAnalyze, getDetectorGeo(), fDebug );
    }
// ============================
// set the data readers for all inherent classes
    initializeDataReader();

// set event number vector
    fTelescopeEventNumber.assign( fNTel, 0 );
// set event times
    fEventMJD.assign( fNTel, 0 );
    fEventTime.assign( fNTel, 0. );

// set number of channels
    for( unsigned int i = 0; i <  fRunPar->fTelToAnalyze.size(); i++ )
    {
	 setTelID( fRunPar->fTelToAnalyze[i] );
// this is to ignore the photodiode
	 if( fReader->getMaxChannels() - getNChannels() != 1 ) setNChannels( fReader->getMaxChannels() );
// check telescope configuration
	 if( fReader->getMaxChannels() == 0 )
	 {
	     cout << "VEventLoop::initEventLoop error: telescope " << fRunPar->fTelToAnalyze[i] << " with 0 channels" << endl;
	     exit( -1 );
	 }
    }

// create calibrators, analyzers, etc. at first event
    if( fCalibrator) fCalibrator->initialize();

    initializeAnalyzers();
    if( fPedestalCalculator && fRunPar->fPedestalsInTimeSlices )
    {
	 fPedestalCalculator->initialize( (fRunMode == R_PED),  getNChannels(), fRunPar->fPedestalsLengthOfTimeSlice, fRunPar->fCalibrationSumFirst, fRunPar->fCalibrationSumWindow );
    }
// print run informations
    printRunInfos();

////////////////////////////////////////////////////////////////////////////////
// set array pointing (values valid for all telescope)
////////////////////////////////////////////////////////////////////////////////
    cout << endl;
    cout << "----------------------" << endl;
    cout << "Initialize pointing..." << endl;
    fArrayPointing = new VArrayPointing();
    fArrayPointing->setObservatory( fRunPar->getObservatory_Longitude_deg(), fRunPar->getObservatory_Latitude_deg() );
///////////////////////////////////////////////////////////////////////////////////////////
// Monte Carlo file
    if( fRunPar->fIsMC != 0 ) fArrayPointing->setMC();
///////////////////////////////////////////////////////////////////////////////////////////
// data
    else
    {
///////////////////////////////////////////////////////////////////////////////////////////
// set target by name (this means target coordinates must be hard wired into VTargets.cpp)
// (this should not be used in any serious analysis!!)
///////////////////////////////////////////////////////////////////////////////////////////
    if( fRunPar->fTargetName.size() > 0 && fRunPar->fTargetDec < -90. )
    {
	cout << "A " << fRunPar->fTargetName << "\t" << fRunPar->fTargetDec << endl;
	if( !fArrayPointing->setTarget( fRunPar->fTargetName ) )
	{
	      cout << endl;
	      cout << "...exiting" << endl;
	      exit( 0 );
	}
     }
///////////////////////////////////////////////////////////////////////////////////////////
// set target coordinates from command line or from DB
///////////////////////////////////////////////////////////////////////////////////////////
    else if( fRunPar->fTargetDec > -99. && fRunPar->fTargetRA > -99. )
    {
       fArrayPointing->setTargetName( fRunPar->fTargetName );
       fArrayPointing->setTargetJ2000( fRunPar->fTargetDec, fRunPar->fTargetRA );
    }
    }
// add any offsets to the pointing [J2000]
    fArrayPointing->setPointingOffset( fRunPar->fTargetRAOffset, fRunPar->fTargetDecOffset );

///////////////////////////////////////////////////////////////////////////////////////////
// set pointing for all telescopes
///////////////////////////////////////////////////////////////////////////////////////////
    for( unsigned int i = 0; i < getNTel(); i++ )
    {
        if( isTeltoAna( i ) )
	{
	   fPointing.push_back( new VPointing( i ) );
	   fPointing.back()->setObservatory( fRunPar->getObservatory_Longitude_deg(), fRunPar->getObservatory_Latitude_deg() );
///////////////////////////////////////////////////////////////////////////////////////////
// Monte Carlo file
	   if( fRunPar->fIsMC != 0 ) fPointing.back()->setMC();
///////////////////////////////////////////////////////////////////////////////////////////
// data
	   else
	   {
///////////////////////////////////////////////////////////////////////////////////////////
// set target by name (this means target coordinates must be hard wired into VTargets.cpp)
// (this should not be used in any serious analysis!!)
///////////////////////////////////////////////////////////////////////////////////////////
	       if( fRunPar->fTargetName.size() > 0 && fRunPar->fTargetDec < -90. )
	       {
		   if( !fPointing.back()->setTarget( fRunPar->fTargetName ) )
		   {
		       cout << endl;
		       cout << "...exiting" << endl;
		       exit( 0 );
		   }
	       }
///////////////////////////////////////////////////////////////////////////////////////////
// set target coordinates from command line or from DB
///////////////////////////////////////////////////////////////////////////////////////////
	       else if( fRunPar->fTargetDec > -99. && fRunPar->fTargetRA > -99. )
	       {
		   fPointing.back()->setTargetName( fRunPar->fTargetName );
		   fPointing.back()->setTargetJ2000( fRunPar->fTargetDec, fRunPar->fTargetRA );
	       }
	   }
// add any offsets to the pointing [J2000]
	   fPointing.back()->setPointingOffset( fRunPar->fTargetRAOffset, fRunPar->fTargetDecOffset );
// set pointing error
	   if( fRunPar->fDBTracking )
	   {
	     fPointing.back()->getPointingFromDB( fRunPar->frunnumber, fRunPar->fDBTrackingCorrections, fRunPar->fPMTextFileDirectory, 
	                                          fRunPar->fDBVPM, fRunPar->fDBUncalibratedVPM );
           }
	   else
	   {
	       fPointing.back()->setPointingError( fRunPar->fPointingErrorX[i], fRunPar->fPointingErrorY[i] );
	   }
        }
	else
	{
	   fPointing.push_back( 0 );
        }
    }
// set coordinates in run parameter
// (J2000)
    if( getArrayPointing() )
    {
        getRunParameter()->fTargetDec = getArrayPointing()->getTargetDecJ2000();
        getRunParameter()->fTargetRA  = getArrayPointing()->getTargetRAJ2000();
    }

    return true;
}


void VEventLoop::initializeAnalyzers()
{
    if( fDebug ) cout << "VEventLoop::initializeAnalyzers()" << endl;

// initialize the analyzers
    if( fAnalyzer && fRunMode != R_PED && fRunMode != R_PEDLOW && fRunMode != R_GTO && fRunMode != R_GTOLOW
                  && fRunMode != R_TZERO && fRunMode != R_TZEROLOW )
    {
        fAnalyzer->initializeDataReader();
        fAnalyzer->initOutput();
    }
    if( fArrayAnalyzer && fRunMode != R_PED && fRunMode != R_PEDLOW && fRunMode != R_GTO && fRunMode != R_GTOLOW 
                       && fRunMode != R_TZERO && fRunMode != R_TZEROLOW )
    {
       fArrayAnalyzer->initializeDataReader();
       fArrayAnalyzer->initOutput();
       fArrayAnalyzer->initTree();
    }
    if( fDST ) fDST->initialize();

// set analysis data storage classes
// (slight inconsistency, produce VImageAnalyzerData for all telescopes, not only for the requested ones
    if( fAnaData.size() == 0 )
    {
        for( unsigned int i = 0; i < fNTel; i++ )
        {
            if( i < fAnaDir.size() && fAnaDir[i] ) fAnaDir[i]->cd();
            setTelID( i );
            fAnaData.push_back( new VImageAnalyzerData( i, fRunPar->fShortTree, (fRunMode == R_PED || fRunMode == R_PEDLOW || 
	                                                                         fRunMode == R_GTO || fRunMode == R_GTOLOW ||
										 fRunMode == R_TZERO || fRunMode == R_TZEROLOW ) ) );
            int iseed = fRunPar->fMCNdeadSeed;
            if( iseed != 0 ) iseed += i;
            fAnaData.back()->initialize( getNChannels(), getReader()->getMaxChannels(), (getTraceFit()>-1.), 
	                                 getDebugFlag(), iseed, getNSamples(), 
					 getRunParameter()->fpulsetiminglevels.size(), getRunParameter()->fpulsetiming_tzero_index, 
					 getRunParameter()->fpulsetiming_width_index );
            if( fRunMode == R_DST )
            {
                fAnaData.back()->initializeMeanPulseHistograms();
                fAnaData.back()->initializeIntegratedChargeHistograms();
            }
	    fAnaData.back()->setTraceIntegrationMethod( getRunParameter()->fTraceIntegrationMethod[i] );
        }
// reading special channels for all requested telescopes
        for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
	{
	   if( getTeltoAna()[i] < fAnaData.size() && fAnaData[getTeltoAna()[i]] )
	   {
	      fAnaData[getTeltoAna()[i]]->readSpecialChannels( getRunNumber(), fRunPar->fsetSpecialChannels, getRunParameter()->getDirectory_EVNDISPParameterFiles() );
           }
        }
// initialize cleaning
        for( unsigned int i = 0; i < fNTel; i++ )
	{
	   setTelID( i );
	   if( getImageCleaningParameter() )
	   {
	      getImageCleaningParameter()->initialize();
	   }
	   else
	   {
	      cout << "VEventLoop::initializeAnalyzers() error initializing image cleaning for telescope " << getTelID()+1 << endl;
	      exit( -1 );
           }
        }
    }
//  old FADC channels in Telescope 1 handle low gain differently
//    require doublepass to find pulse at the right position
//    if( !fReader->isMC() && !fRunPar->fDoublePass ) getDetectorGeo()->setLowGainMultiplier( 0, 1. );
}


/*!
     clean up and call analysis outputfile writer
*/
void VEventLoop::shutdown()
{
    if( fDebug ) cout << "VEventLoop::shutdown()" << endl;
    endOfRunInfo();
    cout << endl << "-----------------------------------------------" << endl;
// (GM): write a root file even if no event was analyzed
//    if( fEventNumber >= 0 )
    {
// write detector parameter tree to disk
        if( fOutputfile != 0 && fRunPar->foutputfileName != "-1" )
        {
            fOutputfile->cd();
        }
        else if( fRunPar->frunmode != R_PED && fRunPar->frunmode != R_PEDLOW && fRunMode != R_GTO && fRunMode != R_GTOLOW && fRunMode != R_TZERO && fRunMode != R_TZEROLOW )
        {
            cout << "VEventLoop::shutdown: Error accessing output file" << endl;
        }
// write run parameter to disk
        if( fRunPar->frunmode != R_PED && fRunPar->frunmode != R_GTO && fRunPar->frunmode != R_GTOLOW
	 && fRunPar->frunmode != R_PEDLOW && fRunPar->frunmode != R_TZERO && fRunPar->frunmode != R_TZEROLOW )
	{
	   fRunPar->Write();
        }
// analysis or trace library mode
        if( fRunPar->frunmode == R_ANA )
        {
// write information about detector to disk
            if( getDetectorTree() )
            {
                if( fDebug ) cout << "\t writing detector tree: " << getDetectorTree()->GetName() << endl;
                getDetectorTree()->Write();
            }
// write array analysis results
            if( fArrayAnalyzer ) fArrayAnalyzer->terminate();
#ifndef NOGSL
            if( fRunPar->ffrogsmode ) fFrogs->terminate();
#endif
// write analysis results for each telescope
            if( fAnalyzer )
            {
                for( unsigned int i = 0; i < fRunPar->fTelToAnalyze.size(); i++ )
                {
                    fAnalyzer->setTelID( fRunPar->fTelToAnalyze[i] );
                    fAnalyzer->terminate();
                }
            }
// write pedestal variation calculations to output file
            if( ( fRunPar->frunmode == R_ANA ) && fRunPar->fPedestalsInTimeSlices && fPedestalCalculator )
            {
                fPedestalCalculator->terminate();
            }
	    if( getReader()->getMonteCarloHeader() )
	    {
	       fOutputfile->cd();
	       getReader()->getMonteCarloHeader()->print();
	       getReader()->getMonteCarloHeader()->Write();
	    }
// close output file here (!! CLOSE !!)
            fAnalyzer->shutdown();
        }
// write calibration/analysis results for each telescope
        else if( fRunPar->frunmode == R_PED || fRunPar->frunmode == R_GTO || fRunPar->frunmode == R_GTOLOW || fRunPar->frunmode == R_PEDLOW
	      || fRunPar->frunmode == R_TZERO || fRunPar->frunmode == R_TZEROLOW )
        {
            VPedestalCalculator *iP = 0;
            if( fRunPar->frunmode == R_PED && fRunPar->fPedestalsInTimeSlices && fPedestalCalculator )
            {
                iP = fPedestalCalculator;
                fPedestalCalculator->terminate( false );
            }
            if( fCalibrator ) fCalibrator->terminate( iP );
        }
// write data summary
        else if( fDST && fRunPar->frunmode == R_DST ) fDST->terminate();
    }
// delete readers
    if( fRunPar->fsourcetype != 0 && fGrIsuReader ) delete fGrIsuReader;
    if( fDebug ) cout << "VEventLoop::shutdown() ... finished" << endl;
// final check of output file; just open and close it again
    if( fRunMode == R_ANA )
    {
       if( fDebug ) cout << "VEventLoop::shutdown: final check of output file" << endl;
       TFile f( fRunPar->foutputfileName.c_str(), "READ" );
       if( f.IsZombie() )
       {
	   cout << "Error: problem with eventdisplay output file: " << fRunPar->foutputfileName << endl;
       }
       else
       {
           cout << "Final checks on result file (seems to be OK): " << fRunPar->foutputfileName << endl;
       }
// FROGS finishing here
#ifndef NOGSL
    if( fRunPar->ffrogsmode ) fFrogs->finishFrogs(&f);
#endif
       f.Close();
    }
// end of analysis
    cout << endl;
    cout << "END OF ANALYSIS, exiting..." << endl;
    cout << "===========================" << endl;
    cout << endl;
}


/*!
  \param gEv goto this event number
  (gEv==0 means reset file and goto event number 1 )
*/
void VEventLoop::gotoEvent( int gEv )
{
    if( fDebug ) cout << "VEventLoop::gotoEvent() " << gEv << endl;
    bool i_res = false;
// goto event number 0, which means, reset file and look for first event
    if( gEv == 0 )
    {
// reset file, intialize calibrator and analyzer
        initEventLoop( fRunPar->fsourcefile );
        return;
    }
// goto event number gEv (backward in sourcefile)
// event number is smaller than current eventnumber
    else if(  gEv - int(fEventNumber) < 0 )
    {
// reset file, start at the beginning and search for this event
        initEventLoop( fRunPar->fsourcefile );
        gotoEvent( gEv );
        return;
    }
// goto eventnumber gEv (forward in sourcefile)
    else
    {
        fAnalyzeMode = false;
// go forward in file and search for event gEv
        while ( (int)fEventNumber != gEv )
        {
            i_res = nextEvent();
            if( fReader->getEventStatus() > 998 )
            {
                i_res = -1;
                break;
            }
        }
// event number larger than number of events in file
        if( i_res <= 0 )
        {
            cout <<  "VEventLoop::gotoEvent( int gEv ): event not found: " << gEv << endl;
            return;
        }
// event found, analyze it
        if( i_res ) analyzeEvent();
    }
}


/*!
   \param iEvents number of events to be analyzed (iEvents = -1, analyze all events in current file)

   \return is always true
*/
bool VEventLoop::loop( int iEvents )
{
    if( fDebug ) cout << "VEventLoop::loop()" << endl;
// print a statement every 5000 events
    int i = 0;
    bool iEventStatus = true;
    fNumberofIncompleteEvents = 0;
    fNumberofGoodEvents = 0;

// Skip to start eventnumber
    if( fRunPar->fFirstEvent > 0 ) gotoEvent( fRunPar->fFirstEvent );

    while( ( i < iEvents || iEvents < 0 ) && iEventStatus )
    {
        iEventStatus = nextEvent();
        if( !iEventStatus )
        {
            if( fReader->getEventStatus() > 998 || fEndCalibrationRunNow ) break;
            else
            {
                fNumberofIncompleteEvents++;
            }
            iEventStatus = true;
        }
        else fNumberofGoodEvents++;
        if( i == 0 ) cout << endl << "\t starting analysis " << endl;
        else if ( fRunPar->fPrintAnalysisProgress > 0 && i % fRunPar->fPrintAnalysisProgress == 0 ) cout << "\t now at event " << i << endl;
        i++;
    }
    terminate( i );
    return true;
}


/*!
  checking event cuts only in analysis mode

  \return
    true if getting the next event was succesful
*/
bool VEventLoop::nextEvent()
{
    if( fDebug ) cout << "VEventLoop::nextEvent()" << endl;
    int i_cut = 1;
    do                                            // => while( i_cut == 0 );
    {
// get next event from data reader and check
// if there is a next event (or EOF) ??
        if( !fReader->getNextEvent() )
        {
// check if this getNextEvent() failed due to an invalid event
            if( fReader->getEventStatus() < 999 ) return false;
            else
            {
                cout << "!!! void VEventLoop::nextEvent(): no next event (end of file)" << endl;
// if the display is run in the loop mode, goto event 0 and start again
                if( fRunPar->floopmode )
                {
                    gotoEvent( 0 );
                    nextEvent();
                    continue;
                }
                else return false;
            }
            return false;
        }
        fReader->setTelescopeID( getTeltoAna()[0] );
	if( !fReader->hasFADCTrace() ) 
	{
	   for( unsigned int i = 0; i < getRunParameter()->fTraceIntegrationMethod.size(); i++ )
	   {
	      getRunParameter()->fTraceIntegrationMethod[i] = 0;
           }
        }
        fillTriggerVectors();
///////////////////////////////////////////////////////////
// set eventnumbers
///////////////////////////////////////////////////////////
// event numbers for array event
        if( fReader->getArrayTrigger() )                     fEventNumber = int( fReader->getArrayTrigger()->getEventNumber() );
        else if (fReader->isMC() || fReader->isDST() )       fEventNumber = int( fReader->getEventNumber() );
        else                                                 fEventNumber = 99999999;
// event numbers for telescope events
        for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
        {
            fReader->setTelescopeID( getTeltoAna()[i] );
            getTelescopeEventNumber()[getTeltoAna()[i]] = fReader->getEventNumber();
        }
///////////////////////////////////////////////////////////
// in displaymode, look for user interaction
        if( fRunPar->fdisplaymode ) gSystem->ProcessEvents();
// analyze event ( do this always except if searching for a specifing event number in the file)
        if( fAnalyzeMode )
	{
	   i_cut = analyzeEvent();
        }
    } while( i_cut == 0 && fNextEventStatus );
// user cut failed
    if( i_cut < 0 ) return false;
    return true;
}


/*!
     check run mode and call the anlyzers
*/
int VEventLoop::analyzeEvent()
{
    if( fDebug )
    {
        cout << "VEventLoop::analyzeEvent()" << endl;
        cout << "\t now at event " << getEventNumber() << endl;
        cout << "----------------------------------------" << endl;
    }
// analysis is running
    fAnalyzeMode = true;
    int i_cut = 0;
    int i_cutTemp = 0;

// short cut for dst writing
    if( fRunMode == R_DST && fDST )
    {
        fDST->fill();
        return 1;
    }

////////////////////////////////////
// analyze all requested telescopes
////////////////////////////////////
    for( unsigned int i = 0; i < fRunPar->fTelToAnalyze.size(); i++ )
    {
        setTelID( fRunPar->fTelToAnalyze[i] );

// check number of samples
	if( getTelID() < fBoolPrintSample.size() && fBoolPrintSample[getTelID()] && !isDST_MC() )
	{
	   cout << "setting sample length for telescope " << getTelID()+1 << " to " << getNSamples() << endl;
	   fBoolPrintSample[getTelID()] = false;
// make sure that calibration sum window is not too long
	   if( fRunMode == R_PED || fRunMode == R_TZERO )
	   {
	      if( fRunPar->fCalibrationSumFirst + fRunPar->fCalibrationSumWindow > (int)getNSamples() )
	      {
	         cout << "VEventLoop::analyzeEvent: resetting calibration sum window from ";
		 cout << fRunPar->fCalibrationSumWindow;
		 fRunPar->fCalibrationSumWindow = getNSamples() - fRunPar->fCalibrationSumFirst;
	         cout << " to " << fRunPar->fCalibrationSumWindow;
		 cout << " (sum first at " << fRunPar->fCalibrationSumFirst;
		 cout << ", min sum per channel " << fRunPar->fCalibrationIntSumMin;
		 cout << ")" << endl;
              }
           }
        }
// quit when number of samples if set to '0'
        if( getNSamples() == 0 && fRunPar->fsourcetype != 7 )
	{
	   cout << "VEventLoop::analyzeEvent() error: retrieved sample length of zero" << endl;
	   cout << "exciting..." << endl;
	   exit( -1 );
        }

// check the requested sumwindow is not larger than the number of samples
        if( (int)getNSamples() < (int)fRunPar->fsumwindow_1[fRunPar->fTelToAnalyze[i]] )
        {
            if( fBoolSumWindowChangeWarning < 1 && fRunPar->fsourcetype != 7 && fRunPar->fsourcetype != 6 && fRunPar->fsourcetype != 4 )
	    {
	         cout << "VEventLoop::analyzeEvent: resetting summation window 1 from ";
		 cout << fRunPar->fsumwindow_1[fRunPar->fTelToAnalyze[i]] << " to " << getNSamples() << endl;
            }
            fRunPar->fsumwindow_1[fRunPar->fTelToAnalyze[i]] = getNSamples();
            fBoolSumWindowChangeWarning = 1;
        }
        if( (int)getNSamples() < (int)fRunPar->fsumwindow_2[fRunPar->fTelToAnalyze[i]] )
        {
            if( fBoolSumWindowChangeWarning < 1 && fRunPar->fsourcetype != 7 && fRunPar->fsourcetype != 6 && fRunPar->fsourcetype != 4 )
	    {
	         cout << "VEventLoop::analyzeEvent: resetting summation window 2 from ";
		 cout << fRunPar->fsumwindow_2[fRunPar->fTelToAnalyze[i]] << " to " << getNSamples() << endl;
            }
            fRunPar->fsumwindow_2[fRunPar->fTelToAnalyze[i]] = getNSamples();
            fBoolSumWindowChangeWarning = 1;
        }
        if( (int)getNSamples() < fRunPar->fsumwindow_pass1[fRunPar->fTelToAnalyze[i]] )
        {
            if( fBoolSumWindowChangeWarning < 2 && fRunPar->fsourcetype != 7 && fRunPar->fsourcetype != 6 && fRunPar->fsourcetype != 4 )
	    {
	       cout << "VEventLoop::analyzeEvent: resetting double pass summation window from ";
	       cout << fRunPar->fsumwindow_pass1[fRunPar->fTelToAnalyze[i]] << " to " << getNSamples() << endl;
            }
            fRunPar->fsumwindow_pass1[fRunPar->fTelToAnalyze[i]] = getNSamples();
            fBoolSumWindowChangeWarning = 2;
        } 
// get event time
        setEventTimeFromReader();

        switch( fRunMode )
        {
/////////////////
// analysis
            case R_ANA:                           // analysis mode
// ignore pedestal events (important for VBF only)
#ifndef NOVBF
                if( fReader->getATEventType() != VEventType::PED_TRIGGER )
#endif
		{
		   if( !fRunPar->fWriteTriggerOnly || fReader->hasArrayTrigger() )
		   {
		      fAnalyzer->doAnalysis();
                   }
// check user cuts
// (single telescope cuts don't work
// what to do:
// determine cut result for each telescope
// AND then with vector of cut telescopes )
		   i_cutTemp = checkCuts();
		   if( i_cut > 0 && !fCutTelescope ) i_cut = 1;
		   else                              i_cut = i_cutTemp;
                }
                break;
/////////////////
// pedestal calculation
            case R_PED:
                i_cut = 1;
                if (fRunPar->fUsePedEvents)
                {
// look for pedestal events
#ifndef NOVBF
                    if (fReader->getATEventType()==VEventType::PED_TRIGGER)
#endif
		    {
		       fCalibrator->calculatePedestals();
                    }
                }
                else
                {
                    if( !fReader->wasLossyCompressed() ) fCalibrator->calculatePedestals();
                }
                break;
/////////////////
// low gain pedestal calculation
            case R_PEDLOW:
                i_cut = 1;
                if( !fReader->wasLossyCompressed() ) fCalibrator->calculatePedestals(true);
                break;
/////////////////
// gain and toffset calculation
            case R_GTO: 
                i_cut = 1;
// don't use pedestal events for gain calculation (vbf file only)
#ifndef NOVBF
                if( fReader->getATEventType() != VEventType::PED_TRIGGER )
#endif
		{
		    fCalibrator->calculateGainsAndTOffsets();
                }
                break;
/////////////////
// gains/toffset calculation for low gain channels
            case R_GTOLOW:
                i_cut = 1;
// don't use pedestal events for gain calculation (vbf file only)
#ifndef NOVBF
                if( fReader->getATEventType() != VEventType::PED_TRIGGER )
#endif
		{
		   fCalibrator->calculateGainsAndTOffsets(true);
                }
                break;
/////////////////
// mean tzero calculation
            case R_TZERO:
                i_cut = 1;
// don't use pedestal events for tzero calculation (vbf file only)
#ifndef NOVBF
                if( fReader->getATEventType() != VEventType::PED_TRIGGER &&  fReader->hasArrayTrigger() )
#endif
		{
		    fCalibrator->calculateAverageTZero();
                }
                break;
/////////////////
// mean tzero calculation (low gain)
            case R_TZEROLOW:
                i_cut = 1;
// don't use pedestal events for tzero calculation (vbf file only)
#ifndef NOVBF
                if( fReader->getATEventType() != VEventType::PED_TRIGGER )
#endif
		{
		    fCalibrator->calculateAverageTZero( true );
                }
                break;
/////////////////
// this should not happen
            default:
                break;

        }
    }
/////////////////////////////////////////////////////////////////////////
// ARRAY ANALYSIS
    if( fRunMode != R_PED && fRunMode != R_GTO && fRunMode != R_GTOLOW && fRunMode != R_PEDLOW && fRunMode != R_TZERO && fRunMode != R_TZEROLOW )
    {
#ifndef NOVBF
       if( fReader->getATEventType() != VEventType::PED_TRIGGER )
#endif
       {
          fArrayAnalyzer->doAnalysis();
// GH Frogs Analysis
#ifndef NOGSL
          if( fRunPar->ffrogsmode )
	  {
             fFrogs->doFrogsStuff(fEventNumber);
          }
#endif 
       }
    }

/////////////////////////////////////////////////////////////////////////
// calculate pedestals in time slices
// (this should be done here because MJD, time from array is used)
    if( fRunMode == R_ANA || fRunMode == R_PED )
    {
        for( unsigned int i = 0; i < fRunPar->fTelToAnalyze.size(); i++ )
        {
// for pedestal calculation: require pedestal event (vbf file only)
#ifndef NOVBF
            if( fRunMode == R_PED && fRunPar->fUsePedEvents && fReader->getATEventType() != VEventType::PED_TRIGGER ) continue;
#endif

            setTelID( fRunPar->fTelToAnalyze[i] );

            if( fRunPar->fPedestalsInTimeSlices && !fReader->wasLossyCompressed() ) fPedestalCalculator->doAnalysis( fRunMode == R_PEDLOW );
        }
    }
// these cuts are important for display mode only
    if( fRunMode != R_PED && fRunMode != R_GTO && fRunMode != R_GTOLOW && fRunMode != R_PEDLOW && fRunMode != R_TZERO && fRunMode != R_TZEROLOW )
    {
       i_cut = int( checkArrayCuts() == 1 && i_cut > 0 );
    }

// number of event in calibration runs
    if( fRunMode == R_PED || fRunMode == R_PEDLOW || fRunMode == R_TZERO || fRunMode == R_TZEROLOW )
    {
        if( fRunPar->fNCalibrationEvents > 0 )
	{
           if( (int)fCalibrator->getNumberOfEventsUsedInCalibration( -1, fRunMode ) > fRunPar->fNCalibrationEvents )
	   {
	      i_cut = -1;
	      fEndCalibrationRunNow = true;
           }
        }
    }

    return i_cut;
}


int VEventLoop::checkArrayCuts()
{
    if( fDebug ) cout << "VEventLoop::checkArrayCuts()" << endl;
// donnot apply array cuts when analysing one telescopes only
    if( getTeltoAna().size() < 2 ) return 1;
// array cuts
    if( (int)getShowerParameters()->fNTrig < fNCutNArrayTrigger ) return 0;
    if( (int)getShowerParameters()->fShowerNumImages[0] < fNCutArrayImages ) return 0;

    return 1;
}


/*!
   using mechanism of ROOT trees in method TTree::Draw( .., char *selection, ..)
   (see http://root.cern.ch/root/html400/TTree.html#TTree:Draw)

   example: plot only events with alpha<10 and more than 10 tubes
   write into the cut box: alpha<10&&ntubes>10.
*/
int VEventLoop::checkCuts()
{
    if( fDebug ) cout << "VEventLoop::checkCuts()" << endl;
    if( !fChangedCut ) return 1;                  // no cuts are applied

    int i_cut;
// number of triggered channels
    int i_numtrig = 0;
    i_numtrig = getImageParameters()->ntubes;
// (GM) was 3
    if( i_numtrig < 0 || i_numtrig < fNCutNTrigger[getTelID()] ) return 0;

// very ugly, but no better idea... (as well no better idea from ROOT people)
// define temporarly a root tree and use its selection mechanism
    if( fStringCut[getTelID()].length() > 0 )
    {
        TTree i_tree( "i_tree", "" );
        float cen_x, cen_y, length, width, size, azwidth, alpha, los, miss, phi, cosphi, sinphi, dist, asymmetry;
        float muonSize, muonRadius, muonRSigma;   // muon Martin
        short int fLocalTrigger;
        float MCenergy;
        unsigned short int ntubes, bad, nlowgain, nsat;
        int muonValid;
	unsigned short int eventType;

	i_tree.Branch( "eventType", &eventType, "eventType/s" );
        i_tree.Branch( "cen_x", &cen_x, "cen_x/F" );
        i_tree.Branch( "cen_y", &cen_y, "cen_y/F" );
        i_tree.Branch( "length", &length, "length/F" );
        i_tree.Branch( "size", &size, "size/F" );
        i_tree.Branch( "width", &width, "width/F" );
        i_tree.Branch( "dist", &dist, "dist/F" );
        i_tree.Branch( "asymmetry", &asymmetry, "asymmetry/F" );
        i_tree.Branch( "azwidth", &azwidth, "azwidth/F" );
        i_tree.Branch( "alpha", &alpha, "alpha/F" );
        i_tree.Branch( "los", &los, "los/F" );
        i_tree.Branch( "miss", &miss, "miss/F" );
        i_tree.Branch( "phi", &phi, "phi/F" );
        i_tree.Branch( "cosphi", &cosphi, "cosphi/F" );
        i_tree.Branch( "sinphi", &sinphi, "sinphi/F" );
        i_tree.Branch( "nlowgain", &nlowgain, "nlowgain/s" );
        i_tree.Branch( "nsat", &nsat, "nsat/s" );
        i_tree.Branch( "ntubes", &ntubes, "ntubes/s" );
        i_tree.Branch( "bad", &bad, "bad/s" );
        i_tree.Branch( "MCenergy", &MCenergy, "MCenergy/F" );
        i_tree.Branch( "fLocalTrigger", &fLocalTrigger, "fLocalTrigger/S" );
        i_tree.Branch( "muonRadius", &muonRadius, "muonRadius/F" );
        i_tree.Branch( "muonRSigma", &muonRSigma, "muonRSigma/F" );
        i_tree.Branch( "muonSize", &muonSize, "muonSize/F" );
        i_tree.Branch( "muonValid", &muonValid, "muonValid/I" );

	eventType = fAnalyzer->getImageParameters()->eventType;
        cen_x = fAnalyzer->getImageParameters()->cen_x;
        cen_y = fAnalyzer->getImageParameters()->cen_y;
        length = fAnalyzer->getImageParameters()->length;
        width = fAnalyzer->getImageParameters()->width;
        size = fAnalyzer->getImageParameters()->size;
        dist = fAnalyzer->getImageParameters()->dist;
        asymmetry = fAnalyzer->getImageParameters()->asymmetry;
        azwidth = fAnalyzer->getImageParameters()->azwidth;
        alpha = fAnalyzer->getImageParameters()->alpha;
        los = fAnalyzer->getImageParameters()->los;
        miss = fAnalyzer->getImageParameters()->phi;
        cosphi = fAnalyzer->getImageParameters()->cosphi;
        sinphi = fAnalyzer->getImageParameters()->sinphi;
        ntubes = fAnalyzer->getImageParameters()->ntubes;
        nsat = fAnalyzer->getImageParameters()->nsat;
	nlowgain = fAnalyzer->getImageParameters()->nlowgain;
        bad = fAnalyzer->getImageParameters()->bad;
        MCenergy = fAnalyzer->getImageParameters()->MCenergy;
        fLocalTrigger = fAnalyzer->getImageParameters()->fLocalTrigger;
        muonSize = fAnalyzer->getImageParameters()->muonSize;
        muonRadius = fAnalyzer->getImageParameters()->muonRadius;
        muonRSigma = fAnalyzer->getImageParameters()->muonRSigma;
        muonValid = fAnalyzer->getImageParameters()->muonValid;

        i_tree.Fill();
        i_cut = int(i_tree.Draw( "alpha", fStringCut[getTelID()].c_str(), "goff" ));
        return i_cut;
    }
// end off ugly stuff
    return 1;
}


/*!
     reset cut strings and variables
 */
void VEventLoop::resetRunOptions()
{
    if( fDebug ) cout << "VEventLoop::resetRunOptions()" << endl;
    fCutTelescope = false;
    fChangedCut = false;
    fNCutNArrayTrigger = 0;
    fNCutArrayImages = 0;
    for( unsigned int i = 0; i < getNTel(); i++ )
    {
        fNCutNTrigger.push_back( 0 );
        fStringCut.push_back( "" );
    }
}


/*!
      will be implemented as soon as all features of the vbf reader is used
*/
void VEventLoop::previousEvent()
{
    cout << "not implemented, fFileRead->getPrevEvent crashes always" << endl;
}


/*!
    analyze only events with more than iNtrigger triggered PMTs

   \param iNtrigger more than iNtrigger tubes triggered in this event
*/
void VEventLoop::setCutNTrigger( int iNtrigger )
{
    if( fDebug ) cout << "VEventLoop::setCutNTrigger()" << endl;
    fNCutNTrigger[fTelID] = iNtrigger;
    fChangedCut = true;
}


void VEventLoop::setCutNArrayTrigger( int iNtrigger )
{
    if( fDebug ) cout << "VEventLoop::setCutNArrayTrigger()" << endl;
    fNCutNArrayTrigger = iNtrigger;
    fChangedCut = true;
}


void VEventLoop::setCutNArrayImages( int iNImages )
{
    if( fDebug ) cout << "VEventLoop::setCutNArrayImages()" << endl;
    fNCutArrayImages = iNImages;
    fChangedCut = true;
}


/*!
    set the user cut string

   \param iCut string with ROOT tree cut (like in TTree::draw()), used in checkCuts()
*/
void VEventLoop::setCutString( string iCut )
{
    if( fDebug ) cout << "VEventLoop::setCutString()" << endl;
    fStringCut[getTelID()] = iCut;
    fChangedCut = true;
}


void VEventLoop::fillTriggerVectors()
{
// first check how many telescope participate
    unsigned int iNTrig = 0;
    for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
    {
        if( getReader()->hasLocalTrigger( getTeltoAna()[i] ) ) iNTrig++;
    }
// now fill the corresponding fields per telescope
    for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
    {
        if( getReader()->hasLocalTrigger( getTeltoAna()[i] ) && iNTrig < getTriggeredTel()[getTeltoAna()[i]].size() )
        {
            getTriggeredTel()[getTeltoAna()[i]][iNTrig]++;
        }
    }
// fill total number of multiplicities
    if( iNTrig < getTriggeredTelN().size() ) getTriggeredTelN()[iNTrig]++;
}


void VEventLoop::terminate( int iAna )
{
    cout << endl;
    cout << "---------------------------------------------------------------------------------------------------------" << endl;
    cout << "End of event loop, finishing up..." << endl;
    cout << endl;
    if( fReader->getNIncompleteEvents().size() > 0 && fReader->getNIncompleteEvents().size() >= getTeltoAna().size() )
    {
        cout << "Number of incomplete events:" << endl;
        for( unsigned int t = 0; t < getTeltoAna().size(); t++ )
        {
            cout << "\t Telescope " << getTeltoAna()[t]+1 << ": " << fReader->getNIncompleteEvents()[getTeltoAna()[t]] << endl;
        }
    }
    cout << endl << "Analyzed " << iAna << " events" << endl;
}

/*

    setting event times from reader

    take care of non-working GPS clocks (apply majority rules)

*/ 
void VEventLoop::setEventTimeFromReader()
{
/////////////////////////////////////////////////////////////////////////
// ignore event time calculation for DSTs
   if( fReader->getDataFormatNum() == 4 || fReader->getDataFormatNum() == 6 ) return;

/////////////////////////////////////////////////////////////////////////
// event times setting for VBF sources
#ifndef NOVBF
/////////////////////////////////////////////////////////////////////////
    unsigned int iCurrentTelID = getTelID();

    VGPSDecoder fGPS;

// decode the GPS time
    fGPS.decode( fReader->getGPS0(), fReader->getGPS1(), fReader->getGPS2(), fReader->getGPS3(), fReader->getGPS4() );

// time is given in seconds per day
    if( getTelID() < fEventTime.size() ) fEventTime[getTelID()] = fGPS.getHrs() *60.*60.+ fGPS.getMins()*60.+fGPS.getSecs();

//  fGPS.getDays() gives day in year, so I calculate the MJD for
//  1st January of this year, then add fGPS.getDays()-1.
    int j = 0;
    double dMJD = 0;
    int iGPSYear = fReader->getATGPSYear()+2000;
    slaCldj(iGPSYear, 1, 1, &dMJD, &j);
    dMJD += fGPS.getDays() - 1.;

// horrible fudge to deal with broken T3 clock (for MJD=54101 only)
    if( getTelID() == 2 && dMJD==54101) dMJD += 100;

// set MJD
    if( getTelID() < fEventTime.size() ) fEventMJD[getTelID()] = (int)dMJD;

///////////////// /////////////////////////////////// /////////////////
// test if all times are the same, apply majority rule otherwise
///////////////// /////////////////////////////////// /////////////////
// allow differences between event times of this value
    double i_max_time_diff = 1.e-4;
// check number of telescopes
    if( getNTel() > VDST_MAXTELESCOPES )
    {
        cout << " VEventLoop::setEventTimeFromReader: warning, cannot apply majority rule to times, too many telescopes: ";
	cout << VDST_MAXTELESCOPES << " " << getNTel() << endl;
        return;
    }
    map< unsigned int, double > i_telescope_time;
    map< unsigned int, int > i_telescope_timeN;
    map< unsigned int, double > i_MJD;

// get times for each telescope
    for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
    {
        getReader()->setTelescopeID( getTeltoAna()[i] );

        fGPS.decode(fReader->getGPS0(), fReader->getGPS1(), fReader->getGPS2(), fReader->getGPS3(), fReader->getGPS4());

        i_telescope_time[getTeltoAna()[i]] = fGPS.getHrs() *60.*60.+ fGPS.getMins()*60.+fGPS.getSecs();
        i_telescope_timeN[getTeltoAna()[i]] = 0;
//! fGPS.getDays() gives day in year, so I calculate the MJD for
//! 1st January of this year, then add fGPS.getDays()-1.
        int  j;
        double dMJD;
        slaCldj (fReader->getATGPSYear()+2000, 1, 1, &dMJD, &j);
        dMJD += fGPS.getDays() - 1.;
        if( fReader->isGrisuMC() && dMJD == 51543 ) dMJD = 54383;
        i_MJD[getTeltoAna()[i]] = dMJD;
    }
// count equal times
    for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
    {
        for( unsigned int j = 0; j < getTeltoAna().size(); j++ )
        {
            if( i == j ) continue;

            if( fabs( i_telescope_time[getTeltoAna()[i]] - i_telescope_time[getTeltoAna()[j]] ) < i_max_time_diff ) i_telescope_timeN[getTeltoAna()[i]]++;
        }
    }
// get time with most majority votes
    int z_max = getTeltoAna()[0];
    int i_nold = 0;
    for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
    {
        if( i_telescope_timeN[getTeltoAna()[i]] > i_nold )
        {
            z_max = getTeltoAna()[i];
            i_nold = i_telescope_timeN[getTeltoAna()[i]];
        }
    }
    if( fabs( i_telescope_time[z_max] - fEventTime[getTelID()] ) > i_max_time_diff )
    {
        fEventTime[getTelID()] = i_telescope_time[z_max];
    }
// check if all MJDs are the same (use same routines as for time)
    for( unsigned int i = 0; i < getTeltoAna().size(); i++ ) i_telescope_timeN[getTeltoAna()[i]] = 0;
    for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
    {
        for( unsigned int j = 0; j < getTeltoAna().size(); j++ )
        {
            if( i == j ) continue;

            if( fabs( i_MJD[getTeltoAna()[i]] - i_MJD[getTeltoAna()[j]] ) < i_max_time_diff ) i_telescope_timeN[getTeltoAna()[i]]++;
        }
    }
// get MJD with majority rule
    z_max = getTeltoAna()[0];
    i_nold = 0;
    for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
    {
        if( i_telescope_timeN[getTeltoAna()[i]] > i_nold )
        {
            z_max = getTeltoAna()[i];
            i_nold = i_telescope_timeN[getTeltoAna()[i]];
        }
    }
    if( fabs( i_MJD[z_max] - fEventMJD[getTelID()] ) > i_max_time_diff )
    {
        fEventMJD[getTelID()] = (int)i_MJD[z_max];
    }
/////////////////////////////////////////////////////////////////////
// check telescope position of T1
    if( getTelID() == 0 && bCheckTelescopePositions && !isMC() ) checkTelescopePositions( fEventMJD[getTelID()] );
/////////////////////////////////////////////////////////////////////
// end of time fixes
/////////////////////////////////////////////////////////////////////
    setTelID( iCurrentTelID );
#endif
}


/*!
    check if T1 is at the right place after the move in Summer 2009

    first run with new array was 55075
*/
void VEventLoop::checkTelescopePositions( int iMJD )
{
    if( iMJD > 55075 )
    {
// check X-position only, should be enough
        if( getDetectorGeometry() && getDetectorGeometry()->getTelXpos().size() > 0 && ( TMath::Abs( getDetectorGeometry()->getTelXpos()[getTelID()] - 135. ) > 1. && TMath::Abs( getDetectorGeometry()->getTelXpos()[getTelID()] - 91.8 ) > 1. ) )
        {
            cout << "==============================================" << endl;
            cout << "WARNING: T1 possibly not at the right position" << endl;
            cout << "==============================================" << endl;
        }
    }
}
