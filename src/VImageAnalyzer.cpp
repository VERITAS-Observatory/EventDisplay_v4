/*! \class VImageAnalyzer
  \brief class for analyzing VERITAS data

  Revision $Id: VImageAnalyzer.cpp,v 1.90.2.6.4.11.10.10.2.10.4.8.2.3.2.3.2.5.2.1.2.5.2.9.2.7.2.3.2.1.2.1 2011/04/21 10:03:36 gmaier Exp $

  \author
  Jamie Holder
  Gernot Maier

*/

#include "VImageAnalyzer.h"

VImageAnalyzer::VImageAnalyzer()
{
    fDebug = getDebugFlag();
    if( fDebug ) cout << "VImageAnalyzer::VImageAnalyzer()" << endl;
    for( unsigned int i = 0; i < fNTel; i++ ) fCalibrated.push_back( false );
    fTraceLibrary = false;
    fRaw = false;
    fOutputfile = 0;
    fInit = false;

// image cleaning
    fVImageCleaning = new VImageCleaning( getData() );

// image parameterisation (Hillas parameters)
    fVImageParameterCalculation = new VImageParameterCalculation( getRunParameter()->fShortTree, getData() );
    fVImageParameterCalculation->setDebug( fDebug );
    fVImageParameterCalculation->setDetectorGeometry( getDetectorGeometry() );
    fVImageParameterCalculation->setDetectorGeometry( getDetectorGeometry() );

// initalize minuit for log likelihood method
    if( fRunPar->fImageLL ) fVImageParameterCalculation->initMinuit( fRunPar->fImageLL );
    else                    fVImageParameterCalculation->initMinuit( 1 );

// initialize root directories
    for( unsigned int i = 0; i < fNTel; i++ ) fAnaDir.push_back( 0 );
// initialize tgraphs
    for( unsigned int i = 0; i < fNTel; i++ )
    {
        fXGraph.push_back( new TGraphErrors(1) );
        fYGraph.push_back( new TGraphErrors(1) );
        fRGraph.push_back( new TGraphErrors(1) );
    }

}


VImageAnalyzer::~VImageAnalyzer()
{
    delete fVImageCleaning;
    delete fVImageParameterCalculation;
}


/*!
 */
void VImageAnalyzer::doAnalysis()
{
    if( fDebug ) cout << "VImageAnalyzer::doAnalysis() for telescope " << getTelID() << endl;
    setDebugLevel( 0 );

    getAnalysisTelescopeEventStatus()[getTelID()] = 0;

///////////////////////////////////////////////////////////////////////////////////////////
// first call -> initialize and read calibration data
//            -> find dead channels
//            -> book trees
// reset debug level
    if( fReader->getMaxChannels() > 0 && !getImageParameters()->getTree() )
    {
        initAnalysis();
// set special channel (L2 signal into each of the FADC crates)
	setSpecialChannels();
// find dead channels, fill tree for this set only (but: dead channels are time dependent)
        findDeadChans( false, true );
        findDeadChans( true, true );
        getAnaHistos()->fillDeadChannelTree( getDead( false ), getDead( true ) );
        fInit = true;
    }
// find dead channels (time dependence)
    else
    {
        findDeadChans( false, false );
        findDeadChans( true, false );
    }
    bool bInit = initEvent();

///////////////////////////////////////////////////////////////////////////////////////////
// don't do analysis if init event failed or if there was no array trigger
    if( !bInit || !getReader()->hasArrayTrigger() )
    {
        fillOutputTree();
        setSums( 0. );
	setPulseTiming( 0., true );
	setPulseTiming( 0., false );
        setImage( false );
        setBorder( false );
        setImageBorderNeighbour( false );
        setHiLo( false );
        getImageParameters()->reset();
        if( fRunPar->fImageLL ) getImageParameters( fRunPar->fImageLL )->reset();
        return;
    }
    if( getDebugFlag() ) cout << "VAnalysis:doAnalysis array trigger: " << getReader()->hasArrayTrigger() << endl;

///////////////////////////////////////////////////////////////////////////////////////////
// integrate pulses and calculate timing parameters 
    calcTZerosSums( getSumFirst(), getSumFirst()+getSumWindow(), getSumFirst(), getSumFirst()+getSumWindow() );

///////////////////////////////////////////////////////////////////////////////////////////
// correct for jitter timing
    if( fRunPar->fL2TimeCorrect ) FADCStopCorrect();

///////////////////////////////////////////////////////////////////////////////////////////
// apply timing correction from laser calibration
    timingCorrect();

///////////////////////////////////////////////////////////////////////////////////////////
// image cleaning
    if( getRunParameter()->fUseFixedThresholds )   fVImageCleaning->cleanImageFixed(getImageThresh(),getBorderThresh(), getBrightNonImageThresh() );
    else if( getRunParameter()->fUseTimeCleaning ) fVImageCleaning->cleanImagePedvarsWithTiming(getImageThresh(),getBorderThresh(), getBrightNonImageThresh(), getTimeCutPixel(), getTimeCutCluster(), getMinNumPixelsInCluster(), getNumLoops() ); //HP
    else                                           fVImageCleaning->cleanImagePedvars(getImageThresh(),getBorderThresh(),getBrightNonImageThresh(), false, false );

///////////////////////////////////////////////////////////////////////////////////////////
// MC trigger parameter
    if( fReader->isGrisuMC() ) setNTrigger();

///////////////////////////////////////////////////////////////////////////////////////////
// parallax width cleaning
    if( fRunPar->fPWmethod > -1 ) fVImageCleaning->cleanTriggerFixed( getImageThresh(), getBorderThresh() );

///////////////////////////////////////////////////////////////////////////////////////////
// correct for relative gains (from laser calibration)
    if( !fRunPar->fDoublePass ) gainCorrect();

///////////////////////////////////////////////////////////////////////////////////////////
// set parameters for image parameter calculation
    fVImageParameterCalculation->setDetectorGeometry( getDetectorGeometry() );
    fVImageParameterCalculation->setParameters( getImageParameters() );

///////////////////////////////////////////////////////////////////////////////////////////
// parallax width trigger parameter calculation
    if( fRunPar->fPWmethod > -1 ) fVImageParameterCalculation->calcTriggerParameters( getTrigger() );

///////////////////////////////////////////////////////////////////////////////////////////
// image parameter calculation
    fVImageParameterCalculation->calcParameters( getSums(),getImage(), getBorder(), getBrightNonImage(), getHiLo(), getImageBorderNeighbour() );
    fVImageParameterCalculation->calcTimingParameters( getTZeros(), getTOffsetvars(), getSums(), getImage(), getBorder(),(VEvndispData*)this );

///////////////////////////////////////////////////////////////////////////////////////////
// muon ring analysis
    if( fRunPar->fmuonmode && !fRunPar->fDoublePass )
    {
        fVImageParameterCalculation->muonRingFinder( getSums(),getImage(), getBorder() );
        fVImageParameterCalculation->muonPixelDistribution( getSums(),getImage(), getBorder() );
        fVImageParameterCalculation->sizeInMuonRing( getSums(),getImage(), getBorder(), getDead() );
    }

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
// second pass of image calculation with using time gradient to adjust sum window
//    (not possible for DST files)
    if( fRunPar->fDoublePass && fReader->getDataFormatNum() != 4 && fReader->getDataFormatNum() != 6   )
    {
///////////////////////////////////////////////////////////////////////////////////////////
// integrate pulses and calculate timing parameters taking time gradients over images into account
        if( fVImageParameterCalculation->getboolCalcGeo() && fVImageParameterCalculation->getboolCalcTiming() )
        {
            calcSecondTZerosSums();
        }

///////////////////////////////////////////////////////////////////////////////////////////
// smoothing of dead or disabled pixels (first part)
        if( fRunPar->fSmoothDead )
        {
            for( unsigned int i = 0; i < getDead().size(); i++ )
            {
                getDeadRecovered()[i] = false;
                savedDead[i] = getDead()[i];
                savedGains[i] = getGains()[i];
                savedDeadLow[i] = getDead(true)[i];
                savedGainsLow = getGains(true)[i];
            }
// dead tubes smoothing
            smoothDeadTubes();
        }
///////////////////////////////////////////////////////////////////////////////////////////
// image cleaning (second pass)
        if( getRunParameter()->fUseFixedThresholds )   fVImageCleaning->cleanImageFixed( getImageThresh(),getBorderThresh(), getBrightNonImageThresh() );
   	else if( getRunParameter()->fUseTimeCleaning ) fVImageCleaning->cleanImagePedvarsWithTiming(getImageThresh(),getBorderThresh(), getBrightNonImageThresh(), getTimeCutPixel(), getTimeCutCluster(), getMinNumPixelsInCluster(), getNumLoops() ); //HP
        else                                           fVImageCleaning->cleanImagePedvars( getImageThresh(),getBorderThresh(), getBrightNonImageThresh(), true, false );


///////////////////////////////////////////////////////////////////////////////////////////
// muon ring analysis (second pass)
        if( fRunPar->fmuonmode )
        {
	  fVImageParameterCalculation->muonRingFinder( getSums(),getImage(), getBorder() );
	  fVImageParameterCalculation->muonPixelDistribution( getSums(),getImage(), getBorder() );
	  fVImageParameterCalculation->sizeInMuonRing( getSums(),getImage(), getBorder(), getDead() );
	}

///////////////////////////////////////////////////////////////////////////////////////////
// smoothing of dead or disabled pixels (second part)
        if( fRunPar->fSmoothDead )
        {
            for( unsigned int i = 0; i < getDead().size(); i++ )
            {
                getDead()[i] = savedDead[i];
                getGains()[i] = savedGains[i];
                getDead(true)[i] = savedDeadLow[i];
                getGains(true)[i] = savedGainsLow[i];
            }
        }
///////////////////////////////////////////////////////////////////////////////////////////
// correct for relative gains (from laser calibration)
        gainCorrect();

///////////////////////////////////////////////////////////////////////////////////////////
// parallax width trigger parameter calculation
        if( fRunPar->fPWmethod > -1 )
	{
	   fVImageCleaning->cleanTriggerFixed( getImageThresh(), getBorderThresh() );
	   fVImageParameterCalculation->calcTriggerParameters( getTrigger() );
	}

///////////////////////////////////////////////////////////////////////////////////////////
// image parameter calculation
        fVImageParameterCalculation->calcParameters( getSums(), getImage(), getBorder(), getBrightNonImage(), getHiLo(), getImageBorderNeighbour() );
        fVImageParameterCalculation->calcTimingParameters( getTZeros(),getTOffsetvars(),getSums(),getImage(), getBorder(), (VEvndispData*)this );

///////////////////////////////////////////////////////////////////////////////////////////
// do a log likelihood analysis on events on the camera edge only
        if( getImageParameters()->ntubes > 0 && getImageParameters()->loss > fRunPar->fLogLikelihoodLoss_min )
        {
            fVImageParameterCalculation->setParametersLogL( getImageParameters() );
            setLLEst( fVImageParameterCalculation->calcLL( (VEvndispData*)this ) );
        }

    }

///////////////////////////////////////////////////////////////////////////////////////////
// log likelihood analysis
// calculate image parameters
    if( fRunPar->fImageLL && getImageParameters()->ntubes > 0 )
    {
//  do this only if geometrical calculation found border/image channels
        if( getImageParameters()->ntubes > 0 )
        {
            fVImageParameterCalculation->setParametersLogL( getImageParametersLogL() );
            setLLEst( fVImageParameterCalculation->calcLL( (VEvndispData*)this ) );
        }
        else getImageParameters( fRunPar->fImageLL )->reset();
    }
///////////////////////////////////////////////////////////////////////////////////////////
// fill results into output tree
    fillOutputTree();
}


void VImageAnalyzer::fillOutputTree()
{
    if( fDebug ) cout << "VImageAnalyzer::fillOutputTree()" << endl;

// fill some run quality histograms
//   getAnaHistos()->fill( getImage(), getBorder(), getSums(), getHiLo(), getTimeSinceRunStart() );
    getAnaHistos()->fillDiagnosticTree( getRunNumber(), getTelescopeEventNumber( getTelID() ), 0, 0, getFADCstopTZero(), getFADCstopSums() );

// fill some basic run parameters
    getImageParameters()->fimagethresh = getImageThresh();
    getImageParameters()->fborderthresh = getBorderThresh();
    getImageParameters()->fncluster_cleaned = getNcluster_cleaned(); // HP
    getImageParameters()->fncluster_uncleaned = getNcluster_uncleaned(); // HP
    getImageParameters()->fsumfirst = getSumFirst();
    getImageParameters()->fsumwindow = getSumWindow();
    getImageParameters()->fsumwindowsmall = getSumWindowSmall();
    if( fReader->hasLocalTrigger(getTelID()) ) getImageParameters()->fLocalTrigger = 1;
    else getImageParameters()->fLocalTrigger = 0;
//  there is always a trigger in the real data, but not in MC
    if( !fReader->isMC() ) getImageParameters()->fLocalTrigger = 1;
    getImageParameters()->bad = (unsigned short int)getNDead();
    getImageParameters()->badLow = (unsigned short int)getNDead(true);
// get MC information
    if( fReader->isMC() )
    {
        getImageParameters()->setMC();
        getImageParameters()->MCprimary = fReader->getMC_primary();
        getImageParameters()->MCenergy  = fReader->getMC_energy();
        getImageParameters()->MCxcore   = fReader->getMC_X();
        getImageParameters()->MCycore   = fReader->getMC_Y();
        getImageParameters()->MCxcos    = fReader->getMC_Xcos();
        getImageParameters()->MCycos    = fReader->getMC_Ycos();
        getImageParameters()->MCLocalTriggerTime = fReader->getLocalTriggerTime(getTelID());
        getImageParameters()->MCLocalDelayedTriggerTime = fReader->getLocalDelayedTriggerTime(getTelID());
        getImageParameters()->MCTel_Xoff= fReader->getMC_Xoffset();
        getImageParameters()->MCTel_Yoff= fReader->getMC_Yoffset();
    }
    getImageParameters()->eventStatus = getAnalysisTelescopeEventStatus()[getTelID()];
// fill the trees with the results
    getImageParameters()->fill();
    if( fRunPar->fImageLL )
    {
        getImageParametersLogL()->eventStatus = getAnalysisTelescopeEventStatus()[getTelID()];
        getImageParametersLogL()->fill();
    }
}


void VImageAnalyzer::initAnalysis()
{
    if( fDebug ) cout << "VImageAnalyzer::initAnalysis()" << endl;

    fVImageParameterCalculation->setParameters( getImageParameters() );
    if( fRunPar->fImageLL ) fVImageParameterCalculation->setParametersLogL( getImageParametersLogL() );

    initOutput();
    initTrees();

// temporary vectors for dead pixel smoothing
    savedDead.assign( getDead().size(), false );
    savedDeadLow.assign( getDead( true ).size(), false );
    savedGains.resize( getGains().size(), 1. );
    savedGainsLow.resize( getGains( true ).size(), 1. );
}


void VImageAnalyzer::terminate()
{
    if( fDebug ) cout << "VImageAnalyzer::terminate()" << endl;
    if ( fOutputfile != 0 && fRunPar->foutputfileName != "-1" && !fTraceLibrary )
    {
        fOutputfile->cd();
        fAnaDir[fTelID]->cd();
// write main output trees
        if( getImageParameters()->getTree() ) getImageParameters()->getTree()->Write();
        if( fRunPar->fImageLL ) getImageParametersLogL()->getTree()->Write();
// write dead channel tree
        TTree *iT = makeDeadChannelTree();
        if( iT ) iT->Write();
// write histograms
        getAnaHistos()->terminate( fOutputfile );
// write calibration summaries
        getCalData()->terminate( getDead( false ), getDead( true ) );
// write pointing data from db to disk (if available)
        if( getTelID() < getPointing().size() && getPointing()[getTelID()] ) getPointing()[getTelID()]->terminate( isMC() );
    }

    if( fTraceLibrary )
    {
        fTraceFile->cd();
        cout << "writing trace library to " << fTraceFile->GetName() << endl;
        cout << "\t total number of events in library: " << fTraceTree->GetEntries() << endl;
        fTraceTree->Write();
    }

    if( fRunPar->ftracefit >= 0. )
    {
        fOutputfile->cd();
        fAnaDir[fTelID]->cd();
        if( fOutputfile ) getFitTraceHandler()->terminate();
    }
}


/*!
     create ROOT output file

     create output directories (for each telescope one, Tel_1, Tel_2, ...)

*/
void VImageAnalyzer::initOutput()
{
    if( fDebug ) cout << "VImageAnalyzer::initOutput()" << endl;
// check if root outputfile exist, otherwise create it
    if( fOutputfile != 0 )
    {
        return;
    }
    if( fRunPar->foutputfileName != "-1" )
    {
// tree versioning numbers used in mscw_energy
        char i_textTitle[200];
        sprintf( i_textTitle, "VERSION %d", fRunPar->getEVNDISP_TREE_VERSION() );
        if( getRunParameter()->fShortTree ) sprintf( i_textTitle, "%s (short tree)", i_textTitle );
        fOutputfile = new TFile( fRunPar->foutputfileName.c_str(), "RECREATE", i_textTitle );
        if( fOutputfile->IsZombie() )
        {
            cout << endl;
            cout << "ERROR: unable to create eventdisplay output file: " << fRunPar->foutputfileName.c_str() << endl;
            cout << "exiting..." << endl;
            cout << endl;
            exit( 0 );
        }
    }

// creating the directories in the root output file
    for( unsigned int i = 0; i < fNTel; i++ ) setAnaDir( i );
}


void VImageAnalyzer::setAnaDir( unsigned int iTel )
{
    if( !fOutputfile ) return;
// create output directories in root files
    TDirectory *i_Directory = gDirectory;
    char i_rootDir[200];
    sprintf( i_rootDir, "Tel_%d", (int)iTel+1 );
    char i_rootDirTit[200];
    sprintf( i_rootDirTit, "analysis results for telescope %d (type %lld)", (int)iTel+1, getDetectorGeometry()->getTelType()[iTel] );
    fOutputfile->cd();
    if( !fOutputfile->FindObject( i_rootDir ) )
    {
        setRootDir( iTel, fOutputfile->mkdir( i_rootDir, i_rootDirTit ) );
    }
    i_Directory->cd();
}


/*!
   initialize the output trees

*/
void VImageAnalyzer::initTrees()
{
    if( fDebug ) cout << "VImageAnalyzer::initTrees() " <<  endl;

    fOutputfile->cd();
    if( !fAnaDir[getTelID()]->cd() )
    {
        cout << "VImageAnalyzer::initTrees() unable to enter directory: " << fAnaDir[getTelID()]->GetName() << endl;
    }

// now book the trees (for MC with additional MC block)
// tree versioning numbers used in mscw_energy
    char i_text[300];
    char i_textTitle[300];
    sprintf( i_text, "tpars" );
    sprintf( i_textTitle, "Event Parameters (Telescope %d, VERSION %d)", getTelID()+1, fRunPar->getEVNDISP_TREE_VERSION() );
    if( getRunParameter()->fShortTree ) sprintf( i_textTitle, "%s (short tree)", i_textTitle );
    fVImageParameterCalculation->getParameters()->initTree(  i_text, i_textTitle, fReader->isMC(), false );

// for log likelihood method, book a second image parameter tree
    if( fRunPar->fImageLL )
    {
        sprintf( i_text, "lpars" );
        sprintf( i_textTitle, "Event Parameters, loglikelihood (Telescope %d)", getTelID()+1 );
        fVImageParameterCalculation->getLLParameters()->initTree( i_text, i_textTitle, fReader->isMC(), true );
    }
}


bool VImageAnalyzer::initEvent()
{
    if( fDebug ) cout <<"VImageAnalyzer::initEvent" << endl;
    setSums( 0. );
    setPulseTiming( 0., true );
    setPulseTiming( 0., false );
    setTCorrectedSumFirst( getSumFirst() );
    setTCorrectedSumLast( getSumFirst() + getSumWindow() );
    setCurrentSummationWindow( getSumWindow() );
    setImage( false );
    setBorder( false );
    setImageBorderNeighbour( false );

// set all variables in getImageParameters() to zero
    getImageParameters()->reset();
    if( fRunPar->fImageLL ) getImageParameters( fRunPar->fImageLL )->reset();

//!Initialize this event
    getImageParameters()->fTelID = getTelID();

    getImageParameters()->time = getEventTime();
    getImageParameters()->MJD  = getEventMJD();

// get time since run start
    if( getAnaData()->fTimeSinceRunStart < 0. )
    {
        getAnaData()->fTimeSinceRunStart = 0.;
        getAnaData()->fTimeRunStart = getImageParameters()->time;
    }
    else
    {
        double idiff = getImageParameters()->time - getAnaData()->fTimeRunStart;
        if( idiff >= 0. ) getAnaData()->fTimeSinceRunStart = idiff;
        else              getAnaData()->fTimeSinceRunStart = 86400.-idiff;
    }

    getImageParameters()->eventNumber = getTelescopeEventNumber( getTelID() );
    getImageParameters()->nsamples = getNSamples();
    getImageParameters()->runNumber = getRunNumber();
    getImageParameters()->eventType = (unsigned short int)getReader()->getNewEventType();

    if( fReader->isGrisuMC() ) setNTrigger();

// fill high/low gain vector
    getImageParameters()->nsat = fillHiLo();

    if( fRunPar->fImageLL )
    {
        getImageParametersLogL()->time = getImageParameters()->time;
        getImageParametersLogL()->MJD = getImageParameters()->MJD;
        getImageParametersLogL()->eventNumber = getImageParameters()->eventNumber;
        getImageParametersLogL()->nsamples = getImageParameters()->nsamples;
        getImageParametersLogL()->runNumber = getImageParameters()->runNumber;
    }

//////////////////////////////////////////////////////////////////////////////////////////////
// get the telescope pointing
//////////////////////////////////////////////////////////////////////////////////////////////
// MC readers
// source file is Grisu MC, MC DST, or MC Pe
    if( fReader->isMC() )
    {
        fPointing[getTelID()]->setMC();
        if( getTelID() < fReader->getTelElevation().size() ) fPointing[getTelID()]->setTelElevation( fReader->getTelElevation()[getTelID()] );
        else                                                 fPointing[getTelID()]->setTelElevation( 0. );
        if( getTelID() < fReader->getTelAzimuth().size()   ) fPointing[getTelID()]->setTelAzimuth( fReader->getTelAzimuth()[getTelID()] );
        else                                                 fPointing[getTelID()]->setTelAzimuth( 0. );
    }
// set pointing direction from command line
    else if( getRunParameter()->felevation > 0. && getRunParameter()->fazimuth > 0. )
    {
        fPointing[getTelID()]->setTelElevation( getRunParameter()->felevation );
        fPointing[getTelID()]->setTelAzimuth( getRunParameter()->fazimuth );
    }
// set pointing direction with target
// target is set via command line, fPointing is initiated in run/VEventLoop::initEventLoop()
    else
    {
        bool iSet = true;
        if( fPointing[getTelID()]->isSet() && fPointing[getTelID()]->getTargetName() != "laser" )
        {
// this calculates telescope elevation and azimuth from telescope pointing in ra and dec
            fPointing[getTelID()]->setTelPointing( getImageParameters()->MJD, getImageParameters()->time );
            if (!fPointing[getTelID()]->isPrecessed())
            {
                fPointing[getTelID()]->precessTarget( getImageParameters()->MJD );
// set wobble offsets
                fPointing[getTelID()]->setWobbleOffset( getRunParameter()->fWobbleNorth, getRunParameter()->fWobbleEast );
                fPointing[getTelID()]->setTelPointing( getImageParameters()->MJD, getImageParameters()->time );
            }
        }
        else
        {
            iSet = false;
        }
        if( !iSet )
        {
            if( fNTel != 1 && getTelescopeEventNumber( getTelID() ) < 1 )
            {
                cout << "VImageAnalyzer::initEvent(): no telescope pointing" << endl;
                setNoPointing( true );
            }
            fPointing[getTelID()]->setTelElevation( 0. );
            fPointing[getTelID()]->setTelAzimuth( 0. );
        }
    }

    return true;
}


/*!
  this is preliminary, background events are expected in future

  using non image/border events for trace library
*/
void VImageAnalyzer::traceLibrary()
{
    if( fDebug ) cout << "VImageAnalyzer::traceLibrary" << endl;
// get size of fTrace array
    int i_ntubes = sizeof(fTrace)/sizeof(fTrace[0]);
    int i_nsamples = sizeof(fTrace[0])/sizeof(short int);
// init the data output file and tree
    if( fReader->getMaxChannels() > i_ntubes || (int)getNSamples() > i_nsamples )
    {
        cout << "void VImageAnalyzer::traceLibrary(): error, number of channels/samples to large ";
        cout << fReader->getMaxChannels() << "\t" << getNSamples() << endl;
        exit( -1 );
    }
// open output file and define output trees
    if( !fTraceLibrary )
    {
        char i_text[100];
        sprintf( i_text,"output/%d.traces.root",getRunNumber());
        fTraceFile = new TFile( i_text, "RECREATE" );
// for multi telescope mode: number in tree name is telescope number
        fTraceTree = new TTree( "trace_0", "tracelibrary" );
//      sprintf( i_text, "ftrace[%d][%d]/B", i_ntubes, i_nsamples );
// this is preliminary, tracelength should be variable
        sprintf( i_text, "ftrace[%d][%d]/S", 500, 64 );
        fTraceTree->Branch( "fTrace", fTrace, i_text );
    }
    doAnalysis();
// write pedestals to file
// (in principal not necessary, since pedestal file are available
// but don't want to be dependent on them in the simulation
    if( !fTraceLibrary )
    {
        fTraceFile->cd();
        double iPed, iPedvars, isumwindow;
        unsigned int iTubeNumber;
// for multi telescope mode: number in tree name is telescope number
        TTree iPedTree( "pedTree_0", "pedestal tree" );
        iPedTree.Branch( "tubeNumber", &iTubeNumber, "tubeNumber/i" );
        iPedTree.Branch( "ped", &iPed, "ped/D" );
        iPedTree.Branch( "pedvar", &iPedvars, "pedvar/D" );
        iPedTree.Branch( "sumwindow", &isumwindow, "sumwindow/D" );
        unsigned int nhits = fReader->getNumChannelsHit();
// (GM) was >= (why?)
        if( nhits > getPeds().size() )
        {
            cout << "void VImageAnalyzer::traceLibrary() error: ped vector size to small " << nhits << "\t" << getPeds().size() << endl;
            exit( -1 );
        }
        for( unsigned int i = 0; i < nhits; i++ )
        {
            unsigned int chanID = 0;
            try
            {
                chanID = fReader->getHitID(i);
                if( chanID < getHiLo().size() )
                {
                    iTubeNumber = chanID;
                    iPed = getPeds( getHiLo()[chanID], getEventTime() )[chanID];
                    iPedvars = getPedvars( getHiLo()[chanID], 0, false, getEventTime() )[chanID];
                    isumwindow = getSumWindow();
                    iPedTree.Fill();
                }
            }
            catch(...)
            {
                cout << "VImageAnalyzer::traceLibrary(), index out of range (fReader->getHitID) " << i << "(Telescope " << getTelID()+1 << ", event " << getEventNumber() << ")" << endl;
                continue;
            }
        }
        iPedTree.Write();
        fTraceLibrary = true;
    }
    vector< uint8_t > tempSample;

// don't use too large events (> 10 ntubes)
    if( fVImageParameterCalculation->getParameters()->ntubes < 10 )
    {
        unsigned int nhits = fReader->getNumChannelsHit();
        for (unsigned int i = 0; i < nhits; i++)
        {
            unsigned int chanID = 0;
            try
            {
                chanID = fReader->getHitID(i);
// exclude image and border pixels
                if ( !getImage()[chanID] && !getBorder()[chanID] )
                {
// exclude pixels with very high significances
                    if( getPedvars(getHiLo()[chanID])[chanID] > 0.  && getSums()[chanID] / getPedvars(getHiLo()[chanID])[chanID] < 10. )
                    {
                        fReader->selectHitChan(i);
                        tempSample = fReader->getSamplesVec();
                        for( unsigned int j = 0; j < tempSample.size(); j++ )
                        {
                            fTrace[chanID][j] = (short int)(tempSample[j]);
                        }
                    }
                }
            }
            catch(...)
            {
                cout << "VImageAnalyzer::traceLibrary(), index out of range (fReader->getHitID) " << i << "(Telescope " << getTelID()+1 << ", event " << getEventNumber() << ")" << endl;
                continue;
            }
        }
    }

// don't expect, that one tube is 3 times in a row a border/image pixel
    if( getTelescopeEventNumber( getTelID() ) % 3 == 0 )
    {
        fTraceTree->Fill();
    }
}


/*!
This reduces the effect of dead tubes by assigning them charge and timing values
derived by taking the average of the neighbouring tubes
*/
void VImageAnalyzer::smoothDeadTubes()
{
    unsigned int i_nchannel = getNChannels();

// loop over all channels
    for ( unsigned int i=0; i< i_nchannel; i++)
    {
// select dead channels
        if( getDetectorGeo()->getAnaPixel()[i] < 1 || !getDead()[i]) continue;
// exclude low gain channel from this estimation
        if( getHiLo()[i] ) continue;

        double ave_sum=0.;
        double ave_pedvar=0.;
        double ave_pedvar_small=0.;
        double ave_gain=0.;
        double ave_tzero=0.;
        double ave_toffvars=0.;
        double count=0.;
// loop over all neighbours of this dead pixel
        for (unsigned int j=0; j < getDetectorGeo()->getNeighbours()[i].size(); j++ )
        {
            unsigned int k = getDetectorGeo()->getNeighbours()[i][j];

            if( getDead()[k] ) continue;

            ave_sum+=getSums()[k];
            ave_pedvar+=getPedvars()[k];
            ave_pedvar_small+=getPedvarsSmall()[k];
            ave_gain+=getGains()[k];
            ave_tzero+=getTZeros()[k];
            ave_toffvars+=getTOffsetvars()[k];
            count +=1.;
        }
        if (count>0.)
        {
            ave_sum/=count;
            ave_pedvar/=count;
            ave_pedvar_small/=count;
            ave_gain/=count;
            ave_tzero/=count;
            ave_toffvars/=count;

// this does not work when dead channel list is time dependent
            getSums()[i]=ave_sum;
            getPedvars()[i]=ave_pedvar;
            getPedvarsSmall()[i]=ave_pedvar_small;
            getGains()[i]=ave_gain;
            getTZeros()[i]=ave_tzero;
            getTOffsetvars()[i]=ave_toffvars;
            getDead()[i]=0;
            getDeadRecovered()[i] = true;
        }
    }

}


/*!
   this is very preliminary and ugly

   useful debugging routine (and for some Leeds project students)
*/
void VImageAnalyzer::printTrace( int iChannel )
{
    vector< uint8_t > tempSample;
// calculate mean sum over all pixels
    double i_total=0;
    for(unsigned int i=0; i < getNChannels(); i++ ) i_total +=getSums()[i];
//if( i_total < 30000 ) return;
    double i_totSum = 0;
    double i_totSumN = 0;
    unsigned int nhits = fReader->getNumChannelsHit();
    for (unsigned int i=0; i<nhits; i++)
    {
        unsigned int chanID = 0;
        try
        {
            chanID = fReader->getHitID(i);
// exclude channels with fadc stop signal
            if( getTOffsets()[chanID] > 6. ) getDead()[chanID] = 8;
            if( !getDead()[chanID] )
            {
                i_totSum += getSums()[chanID];
                i_totSumN++;
            }
        }
        catch(...)
        {
            cout << "VImageAnalyzer::printTrace(), index out of range (fReader->getHitID) " << i << "(Telescope " << getTelID()+1 << ", event " << getEventNumber() << ")" << endl;
            continue;
        }
    }
    if( i_totSumN > 0. ) i_totSum /= i_totSumN;

// now print everything out
    cout << getTelescopeEventNumber( getTelID() ) << " " << getSums()[fReader->getHitID( iChannel )] << " " << i_totSum << "    ";
    unsigned int chanID = fReader->getHitID( iChannel );
    fReader->selectHitChan( chanID );
    tempSample = fReader->getSamplesVec();
    for( unsigned int j = 0; j < tempSample.size(); j++ )
    {
        cout << (int)tempSample[j] << " ";
    }
    cout << endl;

}


void VImageAnalyzer::shutdown()
{
    if( fDebug ) cout << "void VImageAnalyzer::shutdown()" << endl;
    fOutputfile->Close();
}


/*!
   calculate trigger vector

   (needed for grisu MC only)
*/
void VImageAnalyzer::setNTrigger()
{
    if( getDebugFlag() ) cout << "VImageAnalyzer::setNTrigger" << endl;
    getImageParameters()->ntrig = getReader()->getNumberofFullTrigger();

    if( getRunParameter()->fsourcetype == 6 ) return;

    std::vector<bool> triggered = getReader()->getFullTrigVec();
    unsigned int triggered_size = triggered.size();
    unsigned short max_num_in_patch = 0;
    float xj = 0.;
    float yj = 0.;
    float xi = 0.;
    float yi = 0.;
    unsigned short  num_in_patch = 0;

    for( unsigned int i=0; i < triggered_size; i++ )
    {
        if( triggered[i] && i < getDetectorGeo()->getNumChannels() )
        {
//! find position of triggered tube
            xi = getDetectorGeo()->getX()[i];
            yi = getDetectorGeo()->getY()[i];
            num_in_patch=1;
//! see how many other triggered tubes are within 0.3 degrees
            for (unsigned int j=0;j<triggered_size;j++)
            {
                if( triggered[j] && j!=i )
                {
		    bool i_bBreak = false;
		    for( unsigned int t = 0; t < getFADCstopTrig().size(); t++ )
		    {
		       if( j == getFADCstopTrig()[t] ) i_bBreak = true;
                    }
		    if( i_bBreak ) continue;
                    xj = getDetectorGeo()->getX()[j];
                    yj = getDetectorGeo()->getY()[j];
                    if( (xj-xi)*(xj-xi)+(yj-yi)*(yj-yi) < 0.09 ) num_in_patch+=1;
                }
            }
            if (num_in_patch>max_num_in_patch) max_num_in_patch=num_in_patch;
        }
    }
    getImageParameters()->ntrig_per_patch = max_num_in_patch;
}
