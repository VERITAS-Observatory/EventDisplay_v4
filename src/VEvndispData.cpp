/*! \class VEvndispData
    \brief central data class

   \attention

     fReader is not static! Call of VEvndispData::initializeDataReader() necessary  in constructors of all inherit classes

   Revision $Id: VEvndispData.cpp,v 1.34.6.1.2.1.10.1.2.4.4.2.2.4.2.5.2.6.4.1.2.4.2.11.2.5.2.6.2.1 2011/04/21 10:03:36 gmaier Exp $

   \author
     Gernot Maier

*/

#include "VEvndispData.h"

VEvndispData::VEvndispData()
{
    fReader = 0;
//   fDetectorGeo = 0;
}


/*!
 * this function should be called only once in the initialization
 */
void VEvndispData::setTeltoAna( vector< unsigned int > iT )
{
    fTeltoAna = iT;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// fExpectedEventStatus is used in VDisplay -> don't allow to display more than 8*sizeof(size_t) telescope (64?)
//
    bitset<8*sizeof(size_t)> ib;
    for( unsigned int i = 0; i < iT.size(); i++ )
    {
        if( iT[i] >= ib.size() )
        {
	   if( getRunParameter()->fdisplaymode )
	   {
	       cout << "Error: maximum telescopes ID allowed in display mode: " << ib.size();
	       cout << " (try to set " << iT[i] << ")" << endl;
	       if( getNTel() >= ib.size() ) fExpectedEventStatus = 0;
	       else
	       {
		  cout << "exiting..." << endl;
		  exit( 0 );
               }
	   }
	   else
	   {
	       cout << "Warning: telescope ID larger then " << ib.size() << ": " << iT[i] << endl;
	       cout << "(some of the variable will be in overflow (LTrig, ImgSel), no impact on analysis results)" << endl;
	   } 
	   fExpectedEventStatus = 0;
        }
	else
	{
	   ib.set( iT[i], 1 );
        }
    } 
    if( fExpectedEventStatus != 0 ) fExpectedEventStatus = (unsigned long int) ib.to_ulong();
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// init event counting statistics
    vector< int > itemp;
    itemp.assign( fNTel+1, 0 );
    fTriggeredTel.assign( fNTel, itemp );
    fTriggeredTelN.assign( getTeltoAna().size()+1, 0 );

// analysis event status
    fAnalysisArrayEventStatus = 0;
    fAnalysisTelescopeEventStatus.assign( fNTel, 0 );
}


void VEvndispData::setTelID( unsigned int iTel )
{
    if( iTel < fNTel )
    {
        fTelID = iTel;
        if( fReader != 0 ) fReader->setTelescopeID( iTel );
        else
        {
            cout << "VEvndispData::setTelID: fatal error: fReader == 0" << endl;
            exit( -1 );
        }
        if( fDetectorGeo != 0 ) fDetectorGeo->setTelID( iTel );
        else cout << "VEvndispData::setTelID warning: fDetectorGeo == 0" << endl;
    }
    else
    {
        cout << "VEvndispData::setTelescope: error: invalid telescope number " << iTel << "\t" << fTelID << endl;
    }
}


bool VEvndispData::initializeDataReader()
{
    if( getDebugFlag() ) cout << "VEvndispData::initializeDataReader()" << endl;

    if( fGrIsuReader != 0 ) fReader = (VVirtualDataReader*)fGrIsuReader;
#ifndef NOVBF
    else if( fRawDataReader != 0 ) fReader = (VVirtualDataReader*)fRawDataReader;
#endif
    else if( fDSTReader != 0 ) fReader = (VVirtualDataReader*)fDSTReader;
    else if( fMultipleGrIsuReader != 0 ) fReader = (VVirtualDataReader*)fMultipleGrIsuReader;
    else if( fPEReader != 0 ) fReader = (VVirtualDataReader*)fPEReader;

    if( !fReader ) return false;

    fReader->setTeltoAna( fTeltoAna );
// PRELI: require right now that all telescopes should have FADCs (or not)
    bool iperformFADCAnalysis = true;
    for( unsigned int i = 0; i < getRunParameter()->fTraceIntegrationMethod.size(); i++ )
    {
       if( getRunParameter()->fTraceIntegrationMethod[i] == 0 ) iperformFADCAnalysis = false;
    }
    getRunParameter()->fperformFADCAnalysis = iperformFADCAnalysis;
    fReader->setPerformFADCAnalysis( iperformFADCAnalysis );

    return true;
}


void VEvndispData::testDataReader()
{
    if( fReader == 0 )
    {
        if( fRunPar->fsourcetype == 1 && fGrIsuReader != 0 )
        {
            fReader = (VVirtualDataReader*)fGrIsuReader;
        }
#ifndef NOVBF
        else if( fRunPar->fsourcetype == 0 && fRawDataReader != 0 )
        {
            fReader = (VVirtualDataReader*)fRawDataReader;
        }
#endif
        else if( fRunPar->fsourcetype == 4  && fDSTReader != 0 )
        {
            fReader = (VVirtualDataReader*)fDSTReader;
        }
        else if( fRunPar->fsourcetype == 5  && fMultipleGrIsuReader != 0 )
        {
            fReader = (VVirtualDataReader*)fMultipleGrIsuReader;
        }
        else if( fRunPar->fsourcetype == 6 && fPEReader != 0 )
        {
            fReader = (VVirtualDataReader*)fPEReader;
        }
        else if( fRunPar->fsourcetype == 7  && fDSTReader != 0 )
        {
            fReader = (VVirtualDataReader*)fDSTReader;
        }
        else
        {
            cout << "VEvndispData::testDataReader() error: no reader found" << endl;
            exit( -1 );
        }
    }
}


void VEvndispData::resetAnaData()
{
    if( fTelID < fAnaData.size() )
    {
        fAnaData[fTelID]->fSums = 0.;
        fAnaData[fTelID]->fTCorrectedSumFirst = fRunPar->fsumfirst[fTelID];
        fAnaData[fTelID]->fTCorrectedSumLast = fRunPar->fsumfirst[fTelID] + fRunPar->fsumwindow_1[fTelID];
        fAnaData[fTelID]->fCurrentSummationWindow = fRunPar->fsumwindow_1[fTelID];

	fAnaData[fTelID]->fTemplateMu = 0;

        if( getTraceFit() > -1. )
        {
            fAnaData[fTelID]->fRiseTime = 0.;
            fAnaData[fTelID]->fFallTime = 0.;
            fAnaData[fTelID]->fChi2 = 0.;
        }
    }
}

/*

   define detector geometry (telescope positions, pixel position, pixel numbering, etc)

*/
void VEvndispData::setDetectorGeometry( unsigned int iNTel, vector< string > iCamera, string iDir )
{
    if( fDebug )
    {
        cout << "VEvndispData::setDetectorGeometry " << iNTel << "\t" << iDir << endl;
        for( unsigned int i = 0; i < iCamera.size(); i++ ) cout << "\t T" << i+1 << "\t" << iCamera[i] << endl;
    }
// read detector geometry from a configuration file and/or DB    
// (all cases but DSTs)
    if( getRunParameter()->fsourcetype != 7 && getRunParameter()->fsourcetype != 4 )
    {
        fDetectorGeo = new VDetectorGeometry( iNTel, iCamera, iDir, fDebug, 
	                                      getRunParameter()->fCameraCoordinateTransformX, getRunParameter()->fCameraCoordinateTransformY, 
					      getRunParameter()->fsourcetype );
// get camera rotations from the DB
	if( getRunParameter()->fDBCameraRotationMeasurements )
	{
	   fDetectorGeo->readDetectorGeometryFromDB( getRunParameter()->fDBRunStartTimeSQL, getRunParameter()->fDBCameraRotationMeasurements );
        }
    }
// for DST files: read detector geometry from DST file
// (telconfig tree)
    else
    {
        fDetectorGeo = new VDetectorGeometry( iNTel, fDebug );
	fDetectorGeo->setSourceType( getRunParameter()->fsourcetype );
        TFile iDetectorFile( getRunParameter()->fsourcefile.c_str() );
        if( iDetectorFile.IsZombie() )
        {
            cout << "VEvndispData::setDetectorGeometry error opening detector file: " << getRunParameter()->fsourcefile << endl;
            exit( -1 );
        }
        TTree *iTree = (TTree*)iDetectorFile.Get( "telconfig" );
        if( !iTree )
        {
            cout << "VEvndispData::setDetectorGeometry error: cannot find detector tree (telconfig) in " << getRunParameter()->fsourcefile << endl;
            exit( -1 );
        }
        VDetectorTree iDetectorTree;
        iDetectorTree.readDetectorTree( fDetectorGeo, iTree );
        if( fDebug ) cout << "VEvndispData::setDetectorGeometry reading detector geometry from DST file" << endl;
    }
// print most important parameters of the detector
    if( fDetectorGeo ) fDetectorGeo->print( false );
}

TTree* VEvndispData::getDetectorTree() 
{
   if( fDetectorTree ) return fDetectorTree->getTree();

   fDetectorTree = new VDetectorTree();
   fDetectorTree->fillDetectorTree( fDetectorGeo );

   return fDetectorTree->getTree();
}




void VEvndispData::setDeadChannelText()
{
    if( fDebug ) cout << "VEvndispData::setDeadChannelText()" << endl;

    fDeadChannelText.clear();

    fDeadChannelText.push_back( "working" );
    fDeadChannelText.push_back( "dead: out of pedestal range" );
    fDeadChannelText.push_back( "dead: small absolute pedvars" );
    fDeadChannelText.push_back( "dead: small relative pedvars" );
    fDeadChannelText.push_back( "dead: large relative pedvars" );
    fDeadChannelText.push_back( "dead: outside gain range" );
    fDeadChannelText.push_back( "dead: small/large gain vars" );
    fDeadChannelText.push_back( "dead: large gain deviation" );
    fDeadChannelText.push_back( "dead: large time offset" );
    fDeadChannelText.push_back( "disabled: FADC stop signal" );
    fDeadChannelText.push_back( "disabled: masked" );
    fDeadChannelText.push_back( "disabled: user set" );
    fDeadChannelText.push_back( "disabled: MC set" );
}

bool VEvndispData::get_reconstruction_parameters( string ifile )
{
    fEvndispReconstructionParameter = new VEvndispReconstructionParameter( getDetectorGeometry()->getTelType(), getRunParameter() );
    fEvndispReconstructionParameter->SetName( "EvndispReconstructionParameter" );

    unsigned int iNMethods = 0;
    if( ifile.size() > 0 )
    {
// read array analysis cuts
        if( ifile.find( "/" ) != string::npos ) 
	{
	   iNMethods = fEvndispReconstructionParameter->read_arrayAnalysisCuts( ifile );
        }
	else
	{
	   iNMethods = fEvndispReconstructionParameter->read_arrayAnalysisCuts( getRunParameter()->getDirectory_EVNDISPParameterFiles() + "/" + ifile );
        }
        if( !iNMethods )
        {
            return false;
        }
	if( fRunPar->frunmode != 1 && fRunPar->frunmode != 2 && fRunPar->frunmode != 5 && fRunPar->frunmode != 6 )
	{
	   fEvndispReconstructionParameter->print_arrayAnalysisCuts();
        }
        if( fShowerParameters ) fShowerParameters->setNArrayReconstructionMethods( iNMethods );
    }
    else return false;

    return true;
}


/*!
    dump all tree data for current event to stdout
*/
void VEvndispData::dumpTreeData()
{
//////////////////////////////////////////////////////////////////////////////
//
// THIS PRODUCES A SEGMENTATION FAULT
//
//////////////////////////////////////////////////////////////////////////////
/*    if( fDetectorTree )
    {
        cout << endl << "telescopes" << endl;
	cout << fDetectorTree->getTree()->GetName() << endl;
	fDetectorTree->getTree()->Print();

        fDetectorTree->getTree()->Scan();
        cout << endl;
    } */
//////////////////////////////////////////////////////////////////////////////
    if( fShowerParameters ) fShowerParameters->getTree()->Show( -1 );
    for( unsigned int i = 0; i < fNTel; i++ )
    {
        setTelID( i );
        cout << "===============================================================" << endl;
        cout << "Telescope " << i+1 << ":" << endl;
        if( getImageParameters() )
        {
            if( getImageParameters()->getTree() ) getImageParameters()->getTree()->Show( -1 );
	    else cout << "\t no image parameter tree found..." << endl;
        }
	else
	{
	   cout << "\t no image parameters found..." << endl;
        }
        if( getRunParameter()->fImageLL )
        {
            cout << "Log-likelihood tree: " << endl;
            if( getImageParameters( getRunParameter()->fImageLL )->getTree() ) getImageParameters( getRunParameter()->fImageLL )->getTree()->Show( -1 );
        }
    }
}


void VEvndispData::setDead( unsigned int iChannel, unsigned int iDead, bool iLowGain, bool iFullSet, bool iReset )
{
    if( iFullSet )
    {
        if( iChannel < getDead( iLowGain ).size() ) getDead( iLowGain )[iChannel] = iDead;
        bitset<8*sizeof(uint32_t)> idead = getDead( iLowGain )[iChannel];
        for( unsigned int i = 0; i < idead.size(); i++ )
        {
            if( idead.test( i ) )
            {
                getDeadUI( iLowGain )[iChannel] = i;
                break;
            }
        }
    }
    else
    {
        if( iDead == 0 ) return;

        if( iChannel < getDead( iLowGain ).size() )
        {
            bitset<8*sizeof(uint32_t)> ibit_dead = getDead( iLowGain )[iChannel];
            if( !iReset ) ibit_dead.set( iDead, 1 );
            else          ibit_dead.set( iDead, 0 );
            getDead( iLowGain )[iChannel] = ibit_dead.to_ulong();
        }
        if( iChannel < getDeadUI( iLowGain ).size() )
        {
            getDeadUI( iLowGain )[iChannel] = iDead;
        }
    }
}


void VEvndispData::endOfRunInfo()
{
    cout << endl;
    cout << "Event statistics:" << endl;
    cout << "-----------------" << endl;
// print this only for a small number of telescopes
    if( fTriggeredTel.size() < 10 )
    {
       cout << "\t Multiplicity:  | ";
       for( unsigned int i = 0; i < fTriggeredTel[0].size(); i++ ) cout << i << "\t";
       cout << endl;
       cout << "\t ----------------------------------------------" << endl;
       for( unsigned int i = 0; i < fTriggeredTel.size(); i++ )
       {
	   cout << "\t Telescope   " << i+1 << ": | ";
	   for( unsigned int j = 0; j < fTriggeredTel[i].size(); j++ )
	   {
	       cout << fTriggeredTel[i][j] << "\t";
	   }
	   cout << endl;
       }
       cout << endl;
    }
    for( unsigned int i = 0; i < fTriggeredTelN.size(); i++ )
    {
        cout << "\t number of " << i << "-fold events: " << fTriggeredTelN[i] << endl;
    }
    cout << "-----------------------------------------------" << endl;
}


void VEvndispData::printDeadChannels( bool iLowGain )
{
    unsigned int idead = 0;
    cout << endl;
    cout << "Dead channel ";
    if( !iLowGain ) cout << "(high gain channel)";
    else            cout << "(low gain channel)";
    cout << " list for Telescope " << getTelID()+1 << endl;
    cout << "==================================" << endl;
    for( unsigned int i = 0; i < getDead( iLowGain ).size(); i++ )
    {
        if( getDead( iLowGain )[i] > 0 )
        {
            bitset<8*sizeof(uint32_t)> i_dead = getDead( iLowGain )[i];
            cout << "\t " << i << "\t";
            for( unsigned j = 0; j < i_dead.size(); j++ )
            {
                if( i_dead.test( j ) && j < fDeadChannelText.size() ) cout << fDeadChannelText[j] << "\t";
            }
            cout << endl;
            if( !i_dead.test( 9 ) ) idead++;
        }
    }
    cout << "Total number of dead channels ";
    if( !iLowGain ) cout << "(high gain channel)";
    else            cout << "(low gain channel)";
    cout << " for Telescope " << getTelID()+1 << ": " << idead << endl;
    cout << "==================================" << endl;
}


VImageParameter* VEvndispData::getImageParameters( int iselect )
{
    if( iselect > 0 ) return fAnaData[fTelID]->fImageParameterLogL;

    return fAnaData[fTelID]->fImageParameter;
}


bool VEvndispData::initializeDeadChannelFinder()
{
    for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
    {
        bool itelStats = getTelescopeStatus( getTeltoAna()[i] );

        fDeadChannelDefinition_HG.push_back( new VDeadChannelFinder( getRunParameter()->frunmode, getTeltoAna()[i], false, fReader->isMC() ) );
        fDeadChannelDefinition_LG.push_back( new VDeadChannelFinder( getRunParameter()->frunmode, getTeltoAna()[i], true, fReader->isMC() ) );
        if( itelStats && getRunParameter()->fsourcetype != 6 
	              && getRunParameter()->fsourcetype != 4
	              && getRunParameter()->fsourcetype != 7 )
        {
	    if( getRunParameter()->fDeadChannelFile.size() > 0 )
	    {
                fDeadChannelDefinition_HG.back()->readDeadChannelFile( getRunParameter()->getDirectory_EVNDISPParameterFiles() + "/" + getRunParameter()->fDeadChannelFile );
            }
            fDeadChannelDefinition_HG.back()->printDeadChannelDefinition();
	    if( getRunParameter()->fDeadChannelFile.size() > 0 )
	    {
	       fDeadChannelDefinition_LG.back()->readDeadChannelFile( getRunParameter()->getDirectory_EVNDISPParameterFiles() + "/" + getRunParameter()->fDeadChannelFile );
            }
            fDeadChannelDefinition_LG.back()->printDeadChannelDefinition();
        }
	else if(  getRunParameter()->fsourcetype == 7 || getRunParameter()->fsourcetype == 4 )
	{
	   cout << "Reading dead channel settings from DST file for";
	   cout << " telescope " << getTeltoAna()[i]+1 << endl;
	}
	else
	{
	   cout << "Ignoring dead channel settings for this run mode (";
	   cout << getRunParameter()->fsourcetype << ") and ";
	   cout << " telescope " << getTeltoAna()[i]+1 << endl;
        }
    }
    return true;
}


bool VEvndispData::getTelescopeStatus( unsigned int iTelID )
{
    for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
    {
        if( getTeltoAna()[i] == iTelID ) return true;
    }
    return false;
}


valarray<double>& VEvndispData::getPeds( bool iLowGain, double iTime )
{
    if( !getRunParameter()->fPlotRaw )
    {
        if( iTime < 90. ) return fCalData[fTelID]->getPeds( iLowGain, getEventTime() );
        else              return fCalData[fTelID]->getPeds( iLowGain, iTime );
    }
    fPlotRawPedestals.resize( getNChannels(), getDetectorGeometry()->getDefaultPedestal() );

    return fPlotRawPedestals;
}

valarray<double>& VEvndispData::getPedvars( bool iLowGain, unsigned int iSW, double iTime )
{
   if( iSW == 0 ) iSW = getSumWindow();
   if( iTime < -90. ) return fCalData[fTelID]->getPedvars( iLowGain, iSW, getEventTime() );

   return fCalData[fTelID]->getPedvars( iLowGain, iSW, iTime );
}


void VEvndispData::setDebugLevel( int i )
{
    fDebugLevel = i;
    if( i > 0 ) fNDebugMessages++;
}


int VEvndispData::getDebugLevel()
{
    int nDebugMessagesMax = 100;

// allow a maximum of 100 debug messages
    if( fNDebugMessages > nDebugMessagesMax ) return 2;
    else if( fNDebugMessages == nDebugMessagesMax )
    {
        cout << "VEvndispData:: - more than " << nDebugMessagesMax << " error messages or warnings" << endl;
        cout << " ---------- stopped printout of error messages ---------------------------" << endl;
        fNDebugMessages++;
        return 2;
    }

    return fDebugLevel;
}


void VEvndispData::setCurrentSummationWindow( unsigned int imin, unsigned int imax )
{
    unsigned int iT = 0;
    if( imax > imin ) iT = imax - imin;

// this should never happen
    if( iT > getNSamples() )
    {
        cout << "VEvndispData::setCurrentSummationWindow (a) error: summation window too large: " << imin << "\t" << imax << "\t" << iT << endl;
        iT = 0;
    }
    fAnaData[fTelID]->fCurrentSummationWindow = iT;

}


void VEvndispData::setCurrentSummationWindow( unsigned int iChannel, unsigned int imin, unsigned int imax )
{
    if( imax > imin ) fAnaData[fTelID]->fCurrentSummationWindow[iChannel] = imax - imin;
    else              fAnaData[fTelID]->fCurrentSummationWindow[iChannel] = 0;

// this should never happen
    if( fAnaData[fTelID]->fCurrentSummationWindow[iChannel] > getNSamples() )
    {
        cout << "VEvndispData::setCurrentSummationWindow (b) error: summation window too large: ";
	cout << fTelID << "\t" << iChannel << "\t" << imin << "\t" << imax << "\t" << fAnaData[fTelID]->fCurrentSummationWindow[iChannel] << endl;
        fAnaData[fTelID]->fCurrentSummationWindow[iChannel] = 0;
    }
}


unsigned int VEvndispData::getTeltoAnaID( unsigned int iTelID )
{
    for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
    {
        if( getTeltoAna()[i] == iTelID ) return i;
    }
    return 0;
}


VDeadChannelFinder* VEvndispData::getDeadChannelFinder( bool iLowGain )
{
    if( !iLowGain ) return fDeadChannelDefinition_HG[getTeltoAnaID()];

    return fDeadChannelDefinition_LG[getTeltoAnaID()];
}

bool VEvndispData::isTeltoAna( unsigned int iTel )
{
   for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
   {
      if( getTeltoAna()[i] == iTel ) return true;
   }

   return false;
}

vector< valarray<double> >& VEvndispData::getPulseTiming( bool iCorrected )
{
   if( iCorrected ) return fAnaData[fTelID]->fPulseTimingCorrected;

   return fAnaData[fTelID]->fPulseTimingUncorrected;
}

void VEvndispData::setPulseTiming( vector< valarray< double > > iPulseTiming, bool iCorrected )
{
   if( iCorrected ) fAnaData[fTelID]->fPulseTimingCorrected   = iPulseTiming;
   else             fAnaData[fTelID]->fPulseTimingUncorrected = iPulseTiming;
}

/*!
    reset pulse timing
*/
void VEvndispData::setPulseTiming( float iP, bool iCorrected )
{
   for( unsigned int i = 0; i < getPulseTiming( iCorrected ).size(); i++ )
   {
      getPulseTiming( iCorrected )[i] = iP;
   }
}

void VEvndispData::setPulseTiming( unsigned int iChannel, vector< float > iP, bool iCorrected )
{
// check pulse timing vector sizes
   if( iP.size() != getPulseTiming( iCorrected ).size() )
   {
      cout << "VEvndispData::setPulseTiming( unsigned int iChannel, vector< float > iP ) vector size problem: ";
      cout << iP.size() << "\t" << getPulseTiming( iCorrected ).size() << endl;
      return;
   }
// fill all values
   for( unsigned int i = 0; i < getPulseTiming( iCorrected ).size(); i++ )
   {
      if( iChannel < getPulseTiming( iCorrected )[i].size() )
      {
         getPulseTiming( iCorrected )[i][iChannel] = iP[i];
      }
   }
}

void VEvndispData::setPulseTimingCorrection( unsigned int iChannel, double iCorrection )
{
// pulse timing parameters <= fpulsetiming_tzero_index are corrected
// (all times later are pulse widths)
   if( getRunParameter()->fpulsetiming_tzero_index < getPulseTiming( true ).size() )
   {
      for( unsigned int i = 0; i <= getRunParameter()->fpulsetiming_tzero_index; i++ )
      {
	  if( iChannel < getPulseTiming( true )[i].size() )
	  {
	     getPulseTiming( true )[i][iChannel] += iCorrection;
	  }
      }
   }
}

unsigned int VEvndispData::getLargestSumWindow()
{
   unsigned int iSW = 0;

   if( getSumWindow() > iSW )       iSW = getSumWindow();
   if( getSumWindow_2() > iSW )     iSW = getSumWindow_2();
   if( getSumWindow_Pass1() > iSW ) iSW = getSumWindow_Pass1();

   return iSW;
}

unsigned int VEvndispData::getLargestSumWindow( unsigned int iTelID )
{
   unsigned int iSW = 0;

   if( getSumWindow() > iSW )       iSW = getSumWindow( iTelID );
   if( getSumWindow_2() > iSW )     iSW = getSumWindow_2( iTelID );
   if( getSumWindow_Pass1() > iSW ) iSW = getSumWindow_Pass1( iTelID );

   return iSW;
}

/*
   make sure that summation window is not larger than readout window

*/
unsigned int VEvndispData::checkSummationWindow( unsigned int iTelID, unsigned int iSumWindow )
{
    if( iSumWindow <= getNSamples( iTelID ) ) return iSumWindow;

    return getNSamples( iTelID );
}

bool VEvndispData::setSums( valarray< double > iVSum )
{
   if( iVSum.size() != getNChannels() )
   {
      cout << "VEvndispData::setSums() error: setting wrong vector size for integrated charges: " << getNChannels() << "\t" << iVSum.size() << endl;
      cout << "exiting..." << endl;
      exit( -1 );
   }
   fAnaData[fTelID]->fSums = iVSum;
   return true;
}

bool VEvndispData::setAverageTZero( unsigned int iChannel, double iTZero, bool iLowGain )
{
    if( !iLowGain )
    {
      if( iChannel < fCalData[fTelID]->fAverageTzero.size() ) fCalData[fTelID]->fAverageTzero[iChannel] = iTZero;
      else return false;
    }
    else
    {
       if( iChannel < fCalData[fTelID]->fLowGainAverageTzero.size() ) fCalData[fTelID]->fLowGainAverageTzero[iChannel] = iTZero;
       else return false;
    }
    return true;
}

bool VEvndispData::setAverageTZerovars( unsigned int iChannel, double iTZero, bool iLowGain )
{
    if( !iLowGain )
    {
      if( iChannel < fCalData[fTelID]->fAverageTzerovars.size() ) fCalData[fTelID]->fAverageTzerovars[iChannel] = iTZero;
      else return false;
    }
    else
    {
       if( iChannel < fCalData[fTelID]->fLowGainAverageTzerovars.size() ) fCalData[fTelID]->fLowGainAverageTzerovars[iChannel] = iTZero;
       else return false;
    }
    return true;
}

ULong64_t VEvndispData::getTelType( unsigned int iTelID )
{
    if( getDetectorGeometry() )
    {
        if( iTelID < getDetectorGeometry()->getTelType().size() )
	{
	    return getDetectorGeometry()->getTelType()[iTelID];
        }
    }
    return 99999;
}


bool VEvndispData::fDebug = false;
int VEvndispData::fDebugLevel = 0;
int VEvndispData::fNDebugMessages = 0;

// run data
int VEvndispData::fRunNumber = 0;
VEvndispRunParameter* VEvndispData::fRunPar = 0;

// telescope data
unsigned int VEvndispData::fNTel = 1;
unsigned int VEvndispData::fTelID = 0;
vector< unsigned int > VEvndispData::fTeltoAna;
VDetectorGeometry* VEvndispData::fDetectorGeo = 0;
VDetectorTree* VEvndispData::fDetectorTree = 0;

// pointing
VArrayPointing* VEvndispData::fArrayPointing = 0;
vector<VPointing*> VEvndispData::fPointing;
bool VEvndispData::fNoTelescopePointing = false;

// reader
VGrIsuReader* VEvndispData::fGrIsuReader = 0;
VMultipleGrIsuReader* VEvndispData::fMultipleGrIsuReader = 0;
#ifndef NOVBF
VBaseRawDataReader* VEvndispData::fRawDataReader = 0;
#endif
VDSTReader* VEvndispData::fDSTReader = 0;
VPEReader* VEvndispData::fPEReader = 0;

// event data
unsigned int VEvndispData::fEventNumber = 0;
vector< unsigned int > VEvndispData::fTelescopeEventNumber;
unsigned int VEvndispData::fEventType = 0;
unsigned int VEvndispData::fNumberofGoodEvents = 0;
unsigned int VEvndispData::fNumberofIncompleteEvents = 0;
unsigned long int VEvndispData::fExpectedEventStatus = 99;
unsigned int VEvndispData::fAnalysisArrayEventStatus = 0;
vector< unsigned int > VEvndispData::fAnalysisTelescopeEventStatus;

vector< vector< int > > VEvndispData::fTriggeredTel;
vector< int > VEvndispData::fTriggeredTelN;
int VEvndispData::fArrayEventMJD = 0;
int VEvndispData::fArrayPreviousEventMJD = 0;
double VEvndispData::fArrayEventTime = 0.;
vector< int > VEvndispData::fEventMJD;
vector< double > VEvndispData::fEventTime;

// trace handler
VTraceHandler* VEvndispData::fTraceHandler = 0;
VFitTraceHandler* VEvndispData::fFitTraceHandler = 0;

//calibration data
vector< bool > VEvndispData::fCalibrated;
vector< VCalibrationData* > VEvndispData::fCalData;

// dead channel finders
vector< VDeadChannelFinder* > VEvndispData::fDeadChannelDefinition_HG;
vector< VDeadChannelFinder* > VEvndispData::fDeadChannelDefinition_LG;

// analyzer data
TFile* VEvndispData::fOutputfile = 0;
vector< TDirectory* > VEvndispData::fAnaDir;
vector< VImageAnalyzerData* > VEvndispData::fAnaData;
VShowerParameters* VEvndispData::fShowerParameters = 0;
VMCParameters* VEvndispData::fMCParameters = 0;
VEvndispReconstructionParameter* VEvndispData::fEvndispReconstructionParameter = 0;
VFrogParameters* VEvndispData::fFrogParameters = 0;
//vector< VFrogImageData* > VEvndispData::fFrogData;

// timing graphs
vector< TGraphErrors* > VEvndispData::fXGraph;
vector< TGraphErrors* > VEvndispData::fYGraph;
vector< TGraphErrors* > VEvndispData::fRGraph;

// dead channel text
vector< string > VEvndispData::fDeadChannelText;

//  default pedestals for plotraw option
valarray<double> VEvndispData::fPlotRawPedestals;
