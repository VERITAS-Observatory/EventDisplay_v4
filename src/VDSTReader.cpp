/*! \class VDSTReader

     reading class for DST sources files

*/

#include <VDSTReader.h>

VDSTReader::VDSTReader( string isourcefile, bool iMC, int iNTel, bool iDebug )
{
	fDebug = iDebug;
	if( fDebug )
	{
		cout << "VDSTReader::VDSTReader" << endl;
	}

	fNTelescopes = iNTel;
	fSourceFileName = isourcefile;

	setPerformFADCAnalysis( true );

	fDSTtreeEvent = 0;

	fMC = iMC;

	fDSTTree = new VDSTTree();

	// open source file and init tree
	init();
}


TTree* VDSTReader::getMCTree()
{
	if( !isMC() )
	{
		return 0;
	}

	if( !fDSTfile )
	{
		return 0;
	}
	if( fDSTfile->IsZombie() )
	{
		return 0;
	}

	return ( TTree* )fDSTfile->Get( "mc" );
}


/*!

    call this once per run (open source file, init tree, ...)

*/
bool VDSTReader::init()
{
	if( fDebug )
	{
		cout << " VDSTReader::init()" << endl;
	}

	// open file
	fDSTfile = new TFile( fSourceFileName.c_str() );
	if( fDSTfile->IsZombie() )
	{
		cout << "Error reading DST source file: " << fSourceFileName << endl;
		cout << "... exiting ... " << endl;
		exit( -1 );
	}

	// get and init tree
	if( !fDSTfile->Get( "dst" ) || !fDSTfile->Get( "telconfig" ) )
	{
		cout << "Error reading DST tree from dst file" << endl;
		cout << "... exiting ... " << endl;
		exit( -1 );
	}
	fDSTTree->initDSTTree( ( TTree* )fDSTfile->Get( "dst" ), ( TTree* )fDSTfile->Get( "telconfig" ) );
	for( unsigned int i = 0; i < fNTelescopes; i++ )
	{
		fNChannel.push_back( fDSTTree->getDSTNChannels( i ) );
	}

	// init data vectors
	for( unsigned int i = 0; i < fNTelescopes; i++ )
	{
		valarray< double > i_temp( 0., fNChannel[i] );
		vector< unsigned int > i_tempS( fNChannel[i], 0 );
		vector< bool > i_tempB( fNChannel[i], true );
		vector< bool > i_tempF( fNChannel[i], false );
		fSums.push_back( i_temp );
		fPe.push_back( i_temp );
		vector< valarray< double > > i_temp_VV;
		for( unsigned int t = 0; t < VDST_MAXTIMINGLEVELS; t++ )
		{
			i_temp_VV.push_back( i_temp );
		}
		fTracePulseTiming.push_back( i_temp_VV );
		fTraceMax.push_back( i_temp );
		fRawTraceMax.push_back( i_temp );
		fDead.push_back( i_tempS );
		fFullHitVec.push_back( i_tempB );
		fFullTrigVec.push_back( i_tempB );
		fHiLo.push_back( i_tempF );
		fNumberofFullTrigger.push_back( 0 );
		fTelAzimuth.push_back( 0. );
		fNumSamples.push_back( 0 );
		fTelElevation.push_back( 0. );
		fDSTvltrig.push_back( false );
		fDSTl2trig_type.push_back( 0 );
		fLTtime.push_back( 0. );
		fLDTtime.push_back( 0. );
		// FADC Trace
		vector< uint16_t > i_trace_sample( VDST_MAXSUMWINDOW, 0 );
		vector< vector< uint16_t > > i_trace_sample_VV;
		for( unsigned int t = 0; t < VDST_MAXCHANNELS; t++ )
		{
			i_trace_sample_VV.push_back( i_trace_sample );
		}
		fFADCTrace.push_back( i_trace_sample_VV );
	}
	fDummySample.assign( VDST_MAXSUMWINDOW, 0 );

	return fDSTTree->isMC();
}


bool VDSTReader::setTelescopeID( unsigned int iTelID )
{
	if( iTelID < fNTelescopes )
	{
		fTelID = iTelID;
	}
	else
	{
		return false;
	}

	return true;
}


bool VDSTReader::getNextEvent()
{
	if( fDebug )
	{
		cout << "VDSTReader::getNextEvent()" << endl;
	}

	if( !fDSTTree || !fDSTTree->getDSTTree() )
	{
		return false;
	}

	int i_succ = 0;
	i_succ = fDSTTree->getDSTTree()->GetEntry( fDSTtreeEvent );

	// no next event
	if( i_succ <= 0 )
	{
		setEventStatus( 999 );
		return false;
	}
	if( fDSTTree->getDSTNTel() < fNTelescopes )
	{
		cout << "VDSTReader::getNextEvent: error, mismatch in total number of telescopes ";
		cout << fNTelescopes << "\t" << fDSTTree->getDSTNTel() << endl;
		return false;
	}

	// fill data vectors
	for( unsigned int i = 0; i < fNTelescopes; i++ )
	{
		fTelAzimuth[i] = fDSTTree->getDSTTelAzimuth( i );
		fTelElevation[i] = fDSTTree->getDSTTelElevation( i );

		fDSTTree->setTelCounter( i );

		for( unsigned int j = 0; j < fNChannel[i]; j++ )
		{
			fSums[i][j] = fDSTTree->getDSTSums( j );
			fPe[i][j] = fDSTTree->getDSTPe( j );
			for( unsigned int t = 0; t < fDSTTree->getDSTpulsetiminglevelsN(); t++ )
			{
				fTracePulseTiming[i][t][j] = fDSTTree->getDSTpulsetiming( j, t );
			}
			fHiLo[i][j] = fDSTTree->getDSTHiLo( j );
			fTraceMax[i][j] = fDSTTree->getDSTMax( j );
			fRawTraceMax[i][j] = fDSTTree->getDSTRawMax( j );
			fDead[i][j] = fDSTTree->getDSTDead( j );
			fFullTrigVec[i][j] = fDSTTree->getTrigL1( j );
		}
		fNumberofFullTrigger[i] = fDSTTree->getNTrigL1( i );
	}
	// get local trigger
	for( unsigned int i = 0; i < fNTelescopes; i++ )
	{
		fDSTvltrig[i] = fDSTTree->getDSTLocalTrigger( i );
	}
	// get local trigger time
	if( fMC )
	{
		for( unsigned int i = 0; i < fNTelescopes; i++ )
		{
			fLTtime[i] = fDSTTree->getDSTLocalTriggerTime( i );
			fLDTtime[i] = fDSTTree->getDSTLocalDelayedTriggerTime( i );
			fDSTl2trig_type[i] = fDSTTree->getDSTL2TriggerType( i );
		}
	}
	// get FADC trace
	if( fPerformFADCAnalysis && fDSTTree->getFADC() )
	{
		for( unsigned int i = 0; i < fNTelescopes; i++ )
		{
			fDSTTree->setTelCounter( i );
			// telescope is not read out
			if( fNumSamples[i] == 0 )
			{
				continue;
			}
			for( unsigned int j = 0; j < fNChannel[i]; j++ )
			{
				for( unsigned short int k = 0; k < fNumSamples[i]; k++ )
				{
					fFADCTrace[i][j][k] = fDSTTree->getDSTTrace( j, k );
				}
			}
		}
	}

	// successful event
	setEventStatus( 1 );
	// increment tree event number
	fDSTtreeEvent++;
	return true;
}


std::pair<bool, uint32_t> VDSTReader::getChannelHitIndex( uint32_t hit )
{
	if( hit < fSums[fTelID].size() )
	{
		return std::make_pair( true, hit );
	}
	return std::make_pair( false, ( uint32_t ) 0 );
}


uint32_t VDSTReader::getHitID( uint32_t i )
{
	if( i < fSums[fTelID].size() )
	{
		return i;
	}
	return 0;
}


void VDSTReader::setTrigger( vector<bool> iImage, vector<bool> iBorder )
{
	/*   if( fFullTrigVec[fTelID].size() != iImage.size() || fFullTrigVec[fTelID].size() != iBorder.size() )
	   {
	      cout << "VDSTReader::setTrigger error: trigger/image/border vectors have different sizes ";
	      cout << fFullTrigVec[fTelID].size() << "\t" << iImage.size() << "\t" << iBorder.size() << endl;
	   }
	   fNumberofFullTrigger[fTelID] = 0;
	   for( unsigned int i = 0; i < fFullTrigVec[fTelID].size(); i++ )
	   {
	       if( iImage[i] || iBorder[i] ) fFullTrigVec[fTelID][i] = true;
	       else fFullTrigVec[fTelID][i] = false;
	       fNumberofFullTrigger[fTelID]++;
	}
	*/
}

VMonteCarloRunHeader* VDSTReader::getMonteCarloHeader()
{
	if( fDSTfile )
	{
		return ( VMonteCarloRunHeader* )fDSTfile->Get( "MC_runheader" );
	}
	return 0;
}

vector< uint16_t > VDSTReader::getSamplesVec16Bit()
{
	if( fTelID < fFADCTrace.size() )
	{
		if( fSelectedHitChannel < fFADCTrace[fTelID].size() )
		{
			return fFADCTrace[fTelID][fSelectedHitChannel];
		}
	}

	return fDummySample16Bit;
}

vector< uint8_t > VDSTReader::getSamplesVec()
{
	if( fPerformFADCAnalysis && fTelID < fFADCTrace.size() )
	{
		if( fSelectedHitChannel < fFADCTrace[fTelID].size() )
		{
			for( unsigned int i = 0; i < getNumSamples(); i++ )
			{
				fDummySample[i] = ( uint8_t )fFADCTrace[fTelID][fSelectedHitChannel][i];
			}
			return fDummySample;
		}
	}
	fDummySample.assign( VDST_MAXSUMWINDOW, 0 );

	return fDummySample;
}

uint8_t  VDSTReader::getSample( unsigned channel, unsigned sample, bool iNewNoiseTrace )
{
	return ( uint8_t )getSample16Bit( channel, sample, iNewNoiseTrace );
}

uint16_t VDSTReader::getSample16Bit( unsigned channel, unsigned sample, bool iNewNoiseTrace )
{
	if( fPerformFADCAnalysis && fTelID < fFADCTrace.size() )
	{
		if( channel < fFADCTrace[fTelID].size() )
		{
			if( sample < fFADCTrace[fTelID][channel].size() )
			{
				return fFADCTrace[fTelID][channel][sample];
			}
		}
	}
	iNewNoiseTrace = true;

	return 3;
}

bool VDSTReader::isZeroSuppressed( unsigned int iChannel )
{
	if( fDSTTree->getZeroSupppressed( getTelescopeID(), iChannel ) == 0 )
	{
		return false;
	}

	return true;
}
