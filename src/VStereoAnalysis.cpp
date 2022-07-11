/*! \class VStereoAnalysis
    \brief class for producing histograms from parameterized stereo VERITAS data

*/

#include "VStereoAnalysis.h"

VStereoAnalysis::VStereoAnalysis( bool ion, string i_hsuffix, VAnaSumRunParameter* irunpara, vector< TDirectory* > iDirRun,
								  TDirectory* iDirTot, string iDataDir, int iRandomSeed, bool iTotalAnalysisOnly )
{
	fDebug = false;

	fDataFile = 0;
	fInstrumentEpoch = "NOT_SET";
	fDirTot = iDirTot;
	fDirTotRun = iDirRun;
	bTotalAnalysisOnly = iTotalAnalysisOnly;

	// set default values
	fIsOn = ion;
	// do full sky plots
	fNoSkyPlots = false;

	gMeanEffectiveArea = 0;
	gTimeBinnedMeanEffectiveArea = 0;
	gMeanEsys_MC = 0;

	gMeanEffectiveAreaMC = 0;
	hResponseMatrix = 0;

	fHisCounter = 0;
	fTotCount = 0.;

	fMeanAzimuth = 0.;
	fMeanElevation = 0.;
	fNMeanElevation = 0.;

	fTreeSelectedEvents = 0;

	fRunPara = irunpara;
	fDL3EventTree = 0;
	fDeadTimeStorage = 0.;

	fVsky = new VSkyCoordinates() ;
	fVsky->supressStdoutText( true ) ;
	fVsky->setObservatory( VGlobalRunParameter::getObservatory_Longitude_deg(),
						   VGlobalRunParameter::getObservatory_Latitude_deg() );

	// calculating run start, end and duration (verifies data trees)
	if( !bTotalAnalysisOnly )
	{
		setRunTimes();
	}

	// targets and exclusion regions
	if( !bTotalAnalysisOnly )
	{
		defineAstroSource();
	}

	///////////////////////////////
	// define histograms

	// combined results
	iDirTot->cd();
	fHistoTot = new VStereoHistograms( i_hsuffix, fRunPara->fSkyMapBinSize, fRunPara->fSkyMapBinSizeUC,
									   fRunPara->fEnergySpectrumBinSize, fRunPara->fTimeIntervall, -1, -1, fIsOn );
	fHistoTot->setSkyMapSize( fRunPara->fSkyMapSizeXmin, fRunPara->fSkyMapSizeXmax, fRunPara->fSkyMapSizeYmin, fRunPara->fSkyMapSizeYmax );

	// one set of histograms for each run
	if( iDirRun.size() != fRunPara->fRunList.size() )
	{
		cout << "VStereoAnalysis::VStereoAnalysis fatal error, directory and run list different ";
		cout << iDirRun.size() << "\t" << fRunPara->fRunList.size() << endl;
		exit( EXIT_FAILURE );
	}
	// define histograms and rate counters
	vector< double > i_v;
	for( unsigned int i = 0; i < fRunPara->fRunList.size(); i++ )
	{
		iDirRun[i]->cd();
		fHisto.push_back( new VStereoHistograms( i_hsuffix, fRunPara->fSkyMapBinSize, fRunPara->fSkyMapBinSizeUC,
						  fRunPara->fEnergySpectrumBinSize, fRunPara->fTimeIntervall,
						  f_t_in_s_min[fIsOn ? fRunPara->fRunList[i].fRunOn : fRunPara->fRunList[i].fRunOff],
						  f_t_in_s_max[fIsOn ? fRunPara->fRunList[i].fRunOn : fRunPara->fRunList[i].fRunOff], fIsOn ) );
		fHisto.back()->setSkyMapSize( fRunPara->fSkyMapSizeXmin, fRunPara->fSkyMapSizeXmax,
									  fRunPara->fSkyMapSizeYmin, fRunPara->fSkyMapSizeYmax );
		if( fIsOn )
		{
			fHisto.back()->setRunNumber( fRunPara->fRunList[i].fRunOn );
		}
		else
		{
			fHisto.back()->setRunNumber( fRunPara->fRunList[i].fRunOff );
		}

		// define dead time calculators
		fDeadTime.push_back( new VDeadTime( fIsOn ) );

		// rate plots
		fRateCounts.push_back( i_v );
		fRateTime.push_back( i_v );
		fRateTimeIntervall.push_back( i_v );

	}

	// define the time mask
	fTimeMask = new VTimeMask();

	// define the cuts
	fCuts = new VGammaHadronCuts();
	char hname[200];
	if( fIsOn )
	{
		sprintf( hname, "GammaHadronCuts" );
	}
	else
	{
		sprintf( hname, "GammaHadronCuts_off" );
	}
	fCuts->SetName( hname );
	fCuts->resetCutValues();
	fCuts->setDataTree( 0 );
	fCuts->setDataDirectory( iDataDir );

	// define the background model
	fMap   = new VStereoMaps( false, iRandomSeed, fRunPara->fTMPL_RE_RemoveOffRegionsRandomly );
	fMapUC = new VStereoMaps( true,  iRandomSeed, fRunPara->fTMPL_RE_RemoveOffRegionsRandomly );
}


/*!
 *
 * establish run times (mean, start, end and duration) for a list of runs
 *
 */
void VStereoAnalysis::setRunTimes()
{
	cout << endl << "-----------------------------------------------------------------------" << endl;
	cout << "Checking data trees " << ( fIsOn ? "(ON runs)" : "(OFF runs)" ) << endl;
	cout << "\t Run \t| Start (MJD : secs)\t| End (MJD : secs)\t| Duration (secs [mins])" << endl;

	for( unsigned int i = 0; i < fRunPara->fRunList.size(); i++ )
	{
		int i_run = fIsOn ? fRunPara->fRunList[i].fRunOn : fRunPara->fRunList[i].fRunOff;

		CData* c = getDataFromFile( i_run );

		cout << setRunTimes( c ) << endl;
		if( fIsOn )
		{
			fRunPara->fRunList[i].fMJDOn = getMJD( i_run );
		}
		else
		{
			fRunPara->fRunList[i].fMJDOff = getMJD( i_run );
		}

		closeDataFile();
	}
}


/*!
 *
 * establish run times (mean, start, end and duration) of a single run
 *
 */
string  VStereoAnalysis::setRunTimes( CData* iData )
{
	ostringstream ires( "" );
	ires.setf( ios_base::fixed, ios_base::floatfield );

	double i_min, i_minMJD, i_minUTC = 0.;
	double i_max, i_maxMJD, i_maxUTC = 0.;
	double i_dur = 0.;

	fDataRun = iData;
	int i_run = getDataRunNumber();

	fDataRun->GetEntry( 1 );
	i_min = fDataRun->Time;
	f_t_in_s_min[i_run] = i_min;
	i_minMJD = fDataRun->MJD;
	i_minUTC = VSkyCoordinatesUtilities::getUTC( ( int )i_minMJD, i_min );

	int i_nentries = ( int )fDataRun->fChain->GetEntries() - 2;
	fDataRun->GetEntry( i_nentries );
	i_max = fDataRun->Time;
	f_t_in_s_max[i_run] = i_max;
	i_maxMJD = fDataRun->MJD;
	i_maxUTC = VSkyCoordinatesUtilities::getUTC( ( int )i_maxMJD, i_max );

	i_dur = ( i_maxUTC - i_minUTC ) * 24 * 60 * 60;

	fRunMJDStart[i_run] = i_minUTC;
	fRunMJDStopp[i_run] = i_maxUTC;
	fRunMJD[i_run] = ( i_maxUTC + i_minUTC ) / 2.;
	fRunDuration[i_run] = i_dur;

	ires.precision( 0 );
	ires << "\t " << i_run << "\t| ";

	ires <<  i_minMJD;
	ires.precision( 2 );
	ires << " : " << i_min;

	ires.precision( 0 );
	ires << "\t| " << i_maxMJD;
	ires.precision( 2 );
	ires << " : " << i_max;

	ires << "\t| " << i_dur  << " [" << i_dur / 60. << "]";

	return ires.str();
}


/*
 *
 * Get run number from current data tree
 *
 */
int VStereoAnalysis::getDataRunNumber() const
{
	if( !fDataRun )
	{
		cout << "VStereoAnalysis::getDataRunNumber error: no data tree." << endl;
	}
	else
	{
		if( fDataRun->GetEntry( 0 ) == -1 )
		{
			cout << "VStereoAnalysis::getDataRunNumber error: can't seem to access tree." << endl;
		}
		else if( fDataRun->GetEntry( 0 ) == 0 )
		{
			cout << "VStereoAnalysis::getDataRunNumber error: tree is empty." << endl;
			fDataRun->fChain->Print();
		}
		else
		{
			return fDataRun->runNumber;
		}
	}

	exit( EXIT_FAILURE );
	return 0;
}


/*!
 *
 *  fill all histograms and maps
 *  (main event loop)
 *
 *  int irun  runnumber to be analyzed, set irun = -1 to analyze all runs in data chain
 *
 *  return number of events passing all cuts
 *
 */
double VStereoAnalysis::fillHistograms( int icounter, int irun, double iAzMin, double iAzMax, double iPedVar )
{
	if( fDebug )
	{
		cout << "DEBUG double VStereoAnalysis::fillHistograms() "  << icounter << "\t" << irun << endl;
	}

	fHisCounter = icounter;

	////////////////////////////////////////////////
	// combine all histograms from all runs
	if( irun < 0 )
	{
		fHisCounter = -1;
		return combineHistograms();
	}

	////////////////////////////////////////////////
	// analyze individual run
	if( fIsOn )
	{
		cout << endl << "------------------------------------------------------------------------" << endl;
		cout << "Filling ON histograms for run " << irun << " -----------------------------" << endl;
	}
	else
	{
		cout << endl << "------------------------------------------------------------------------" << endl;
		cout << "Filling OFF histograms for run " << irun << " -----------------------------" << endl;
	}

	// set pointer to data tree (run wise)
	fDataRun = getDataFromFile( irun );
	if( fDataRun == 0 || fDataRunTree == 0 )
	{
		cout << "VStereoAnalysis::fillHistograms error, no data tree " << endl;
		cout << "\t" << fDataRun << "\t" << fDataRunTree << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	if( fHisCounter > ( int )fHisto.size() )
	{
		cout << "VStereoAnalysis::fillHistograms invalid run number " << irun << "\t" << fHisCounter << "\t" << fHisto.size() << endl;
		exit( EXIT_FAILURE );
	}

	double iMJDStart = 0.;
	double iMJDStopp = 0.;
	if( getDataRunNumber() != irun )
	{
		cout << "VStereoAnalysis::fillHistograms warning: given run (" << irun;
        cout << ") does not match run of given tree (" << getDataRunNumber() << ")" << endl;
	}
	else
	{
		if( fRunMJDStart.find( irun ) == fRunMJDStart.end() )
		{
			setRunTimes( fDataRun );
		}
		iMJDStart = fRunMJDStart[irun];
		if( fRunMJDStopp.find( irun ) == fRunMJDStopp.end() )
		{
			setRunTimes( fDataRun );
		}
		iMJDStopp = fRunMJDStopp[irun];
	}
	//////////////////////////////////////////
	// boolean for gamma/hadron cuts

	// event is gamma-ray like according to VGammaHadronCuts
	bIsGamma = false;
	// event direction is inside search region (e.g. reflected region)
	bool bDirectionCuts = false;
	// successfull energy reconstruction
	bool bEnergyQualityCuts = false;

	// rate vectors
	vector< double > iRateCounts;
	vector< double > iRateTime;
	vector< double > iRateTimeIntervall;

	// initialize time mask
	fTimeMask->setMask( irun, iMJDStart, iMJDStopp, fRunPara->fTimeMaskFile );
    fRunPara->setRunTimes( icounter, iMJDStart, iMJDStopp );

	// initialize cuts
	setCuts( fRunPara->fRunList[fHisCounter], irun );

	// define histograms
	fDirTotRun[fHisCounter]->cd();
	fHisto[fHisCounter]->setRunNumber( irun );
	fHisto[fHisCounter]->defineHistograms();

	// adjust dead time calculator
	TDirectoryFile* iDeadtimeDirectory = 0;
	if( fDataFile )
	{
		iDeadtimeDirectory = ( TDirectoryFile* )fDataFile->Get( "deadTimeHistograms" );
	}
	if( iDeadtimeDirectory )
	{
		fDeadTime[fHisCounter]->readHistograms( iDeadtimeDirectory );
	}
	else
	{
		fDeadTime[fHisCounter]->defineHistograms();
	}

	// adjust axis in rate histograms
	fHisto[fHisCounter]->makeRateHistograms( iMJDStart, iMJDStopp );

	// set map properties
	fMap->setData( fDataRun );
	fMap->setTargetShift( fRunPara->fRunList[fHisCounter].fTargetShiftWest, fRunPara->fRunList[fHisCounter].fTargetShiftNorth );
	fMap->setRegionToExclude( fRunPara->fExclusionRegions );
	fMap->setNoSkyPlots( fNoSkyPlots );
	fMap->setRunList( fRunPara->fRunList[fHisCounter] );
	fMap->setHistograms( fHisto[fHisCounter]->hmap_stereo, 
                         fHisto[fHisCounter]->hmap_alpha, 
                         fHisto[fHisCounter]->hmap_MeanSignalBackgroundAreaRatio );

	fMapUC->setData( fDataRun );
	fMapUC->setTargetShift( fRunPara->fRunList[fHisCounter].fTargetShiftWest, fRunPara->fRunList[fHisCounter].fTargetShiftNorth );
	fMapUC->setRegionToExclude( fRunPara->fExclusionRegions );
	fMapUC->setNoSkyPlots( fNoSkyPlots );
	fMapUC->setRunList( fRunPara->fRunList[fHisCounter] );
	fMapUC->setHistograms( fHisto[fHisCounter]->hmap_stereoUC, fHisto[fHisCounter]->hmap_alphaUC, 0 );

	// initialize gamma/hadron cuts
	fCuts->setDataTree( fDataRun );

	// tree with selected events
	init_TreeWithSelectedEvents( irun, fIsOn );

	if( fIsOn )
	{
		init_DL3Tree( irun, fHisCounter );
	}

	// spectral energy reconstruction (effective areas, etc.)
	// effective area class
	VEffectiveAreaCalculator fEnergy( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, iAzMin, iAzMax, iPedVar,
				  fRunPara->fEnergyReconstructionSpectralIndex, fRunPara->fMCZe,
				  fRunPara->fEnergyEffectiveAreaSmoothingIterations,
				  fRunPara->fEnergyEffectiveAreaSmoothingThreshold, 
                  fRunPara->fEffectiveAreaVsEnergyMC,
				  fRunPara->fLikelihoodAnalysis);

	double iEnergyWeighting = 1.;
	double iErec = 0.;
	double iErecChi2 = 0.;
	double iPedVar_temp = 0.;
	double iXoff = 0.;
	double iYoff = 0.;
	// for time-check save old-time and new-time
	// variable to set the real duration of each time bin
	double time_of_EVENT = 0;
	int index_time_bin_NOW  = 1;

	double i_UTC = 0.;
	double i_xderot = -99.;
	double i_yderot = -99.;
	double i_theta2 = -99.;

	// mean direction values
	fMeanAzimuth = 0.;
	fMeanElevation = 0.;
	fNMeanElevation = 0.;
	double iDirectionOffset = 0.;

	// get number of entries from data tree
	Int_t nentries = Int_t( fDataRun->fChain->GetEntries() );
	if( fDebug )
	{
		cout << "DEBUG double VStereoAnalysis::fillHistograms() reading chain " << fDataRun->fChain->GetName() << "\t" << nentries << endl;
	}
	cout << "\t number of entries for this run: " << nentries << endl;

	double i_count = 0.;
	int nentries_run = 0;

	/////////////////////////////////////////////////////////////////////
	// loop over all entries/events in the data tree
	for( int i = 0; i < nentries; i++ )
	{
		fDataRun->GetEntry( i );

		if( fDataRun->runNumber == irun )
		{
			// count how many entries are in this run
			nentries_run++;

			// UTC time
			i_UTC = VSkyCoordinatesUtilities::getUTC( fDataRun->MJD, fDataRun->Time );

			// phase cuts - this is also a time cut that adds to the previously initialized mask
			if( !fCuts->applyPhaseCut( i ) )
			{
				fTimeMask->setMaskDuringPhaseCuts( i_UTC );
				continue;
			}
			// make time cut
			if( !fTimeMask->checkAgainstMask( i_UTC ) )
			{
				continue;
			}

			// fill rate histograms
			fHisto[fHisCounter]->hrate_1sec->Fill( i_UTC );
			fHisto[fHisCounter]->hrate_10sec->Fill( i_UTC );
			fHisto[fHisCounter]->hrate_1min->Fill( i_UTC );

			// dead time calculation
			if( !iDeadtimeDirectory )
			{
				fDeadTime[fHisCounter]->fillDeadTime( fDataRun->Time );
			}

			// get energy (depending on energy reconstruction method)
			iErec = fCuts->getReconstructedEnergy( fRunPara->fEnergyReconstructionMethod );
			iErecChi2 = fCuts->getReconstructedEnergyChi2( fRunPara->fEnergyReconstructionMethod );
			// get shower direction (depending on shower reconstruction method)
			iXoff = fCuts->getReconstructedXoff();
			iYoff = fCuts->getReconstructedYoff();

			////////////////////////////////////////////////
			// apply all quality cuts
			//
			// check if event is outside fiducial area
			if( !fCuts->applyInsideFiducialAreaCut() )
			{
				continue;
			}

			// stereo quality cuts (e.g. successful direction, mscw, mscl reconstruction)
			if( !fCuts->applyStereoQualityCuts( fRunPara->fEnergyReconstructionMethod, false, i , fIsOn ) )
			{
				continue;
			}

			// fill image and trigger pattern histograms
			fHisto[fHisCounter]->hTriggerPatternBeforeCuts->Fill( fDataRun->LTrig );
			fHisto[fHisCounter]->hImagePatternBeforeCuts->Fill( fDataRun->ImgSel );

			iDirectionOffset = sqrt( iXoff * iXoff + iYoff * iYoff );
			getDerotatedCoordinates( icounter, i_UTC, iXoff, iYoff,  i_xderot, i_yderot );

			// gamma/hadron cuts
			bIsGamma = fCuts->isGamma( i, false, fIsOn );

			// fill on/offstereo maps and direction cut
			i_theta2 = -99;
			bDirectionCuts = fMap->fill( fIsOn, i_xderot, i_yderot, fCuts->getTheta2Cut_max( iErec ),
										 fDataRun->Ze, iErec, fDataRun->runNumber, bIsGamma, i_theta2 );
			bDirectionCuts = fMapUC->fill( fIsOn, i_xderot, i_yderot, fCuts->getTheta2Cut_max( iErec ),
										   fDataRun->Ze, iErec, fDataRun->runNumber, bIsGamma, i_theta2 );

			// energy reconstruction cut
			bEnergyQualityCuts = fCuts->applyEnergyReconstructionQualityCuts( fRunPara->fEnergyReconstructionMethod );

			/////////////////////////////////////////////////////////////////////////////////////////////////////////
			// following histograms (theta2, mscw, mscl, core position, etc.)  assume source at given target position

			// theta2 ---
			fMap->calculateTheta2( fIsOn, i_xderot, i_yderot );
			// theta2 values for debugging
			for( unsigned int dex = 0; dex < 25; dex++ )
			{
				fDataRun->theta2_All[dex] = fMap->getTheta2_All()[dex];
			}

			for( unsigned int t = 0; t < fMap->getTheta2_length(); t++ )
			{
				fHisto[fHisCounter]->htheta2->Fill( fMap->getTheta2()[t], fMap->getTheta2_weigth()[t] );
			}
			// fill theta for tree with selected events
			if( fMap->getTheta2_length() > 0 )
			{
				fDataRun->theta2 = fMap->getTheta2()[0];
			}
			else
			{
				fDataRun->theta2 = -1;
			}

			/////////////////////////////////////////////////////////
			// histograms after shape (or other gamma/hadron separation cuts) cuts only
			if( bIsGamma )
			{
				fHisto[fHisCounter]->hxyoff_stereo->Fill( iXoff, iYoff );
			}

			/////////////////////////////////////////////////////////
			// histograms after direction cuts only
			//
			if( bDirectionCuts )
			{
				// mean width/length/distance histograms
				fHisto[fHisCounter]->hmean_width->Fill( fCuts->getMeanImageWidth() );
				fHisto[fHisCounter]->hmean_length->Fill( fCuts->getMeanImageLength() );
				fHisto[fHisCounter]->hmean_dist->Fill( fCuts->getMeanImageDistance() );
				if( fDataRun->MSCW > -50. )
				{
					fHisto[fHisCounter]->hmscw->Fill( fDataRun->MSCW );
				}
				if( fDataRun->MSCL > -50. )
				{
					fHisto[fHisCounter]->hmscl->Fill( fDataRun->MSCL );
				}
				if( fDataRun->MSCW > -50. && fDataRun->MSCL > -50. )
				{
					fHisto[fHisCounter]->hmsc->Fill( fDataRun->MSCW, fDataRun->MSCL );
				}
				// probability threshold cuts
				if( fCuts->getProbabilityCut_Selector() > 0. )
				{
					fHisto[fHisCounter]->hrf->Fill( fCuts->getProbabilityCut_Selector() );
				}
				// mean emission height histograms
				if( fDataRun->EmissionHeight > 0. )
				{
					fHisto[fHisCounter]->hemiss->Fill( fDataRun->EmissionHeight );
					fHisto[fHisCounter]->hemissC2->Fill( fDataRun->EmissionHeightChi2 );
				}
				// chi2 of energy reconstruction
				if( iErecChi2 > 0. )
				{
					fHisto[fHisCounter]->herecChi2->Fill( iErecChi2 );
				}
				// fill a tree with the selected events (after direction cut only)
				fill_TreeWithSelectedEvents( fDataRun, i_xderot, i_yderot, i_theta2 );
			}

			// fill a tree with current event for DL3 converter
			if( fIsOn && ( bIsGamma || fRunPara->fWriteAllEvents ))
			{
				fill_DL3Tree( fDataRun, i_xderot, i_yderot, icounter, i_UTC );
			}
			/////////////////////////////////////////////////////////
			// histograms after gamma and energy reconstruction cuts
			if( bIsGamma && bEnergyQualityCuts )
			{
				// number of events as expected in a theta2 circle at the given offset
				double iWeight = 1.;
				// solid angle of this bin
				double i_ymax = fHisto[fHisCounter]->herecCounts2D_vs_distance->GetYaxis()->GetBinUpEdge(
									fHisto[fHisCounter]->herecCounts2D_vs_distance->GetYaxis()->FindBin( iDirectionOffset ) );
				double i_ymin = fHisto[fHisCounter]->herecCounts2D_vs_distance->GetYaxis()->GetBinLowEdge(
									fHisto[fHisCounter]->herecCounts2D_vs_distance->GetYaxis()->FindBin( iDirectionOffset ) );
				double iSoli = 2. * TMath::Pi() * ( 1. - cos( i_ymax * TMath::Pi() / 180. ) );
				iSoli       -= 2. * TMath::Pi() * ( 1. - cos( i_ymin * TMath::Pi() / 180. ) );
				iWeight = fCuts->getTheta2Cut_max( iErec );
				if( iWeight > 0. )
				{
					iWeight = 2. * TMath::Pi() * ( 1. - cos( sqrt( iWeight )  * TMath::Pi() / 180. ) );
					iWeight /= iSoli;
					fHisto[fHisCounter]->herecCounts2D_vs_distance->Fill( log10( iErec ), iDirectionOffset, iWeight );
				}
			}

			/////////////////////////////////////////////////////////
			// histograms after all cuts ( shape and direction cuts )
			//
			if( bIsGamma && bDirectionCuts )
			{
				// image and trigger pattern
				fHisto[fHisCounter]->hTriggerPatternAfterCuts->Fill( fDataRun->LTrig );
				fHisto[fHisCounter]->hImagePatternAfterCuts->Fill( fDataRun->ImgSel );
				// make core plots
				fHisto[fHisCounter]->hcore->Fill( fCuts->getReconstructedXcore(), fCuts->getReconstructedYcore() );
				// ##################################
				// spectral energy reconstruction
				// apply energy reconstruction cuts
				if( bEnergyQualityCuts )
				{
					// require valid effective area valid for this event
					// effective areas depend on ze, wobble offset, pedestal variation, etc
					if( fDataRun->meanPedvar_Image > 0. )
					{
						iPedVar_temp = fDataRun->meanPedvar_Image;
					}
					else
					{
						iPedVar_temp = iPedVar;
					}
					// get 1 / effective area
					iEnergyWeighting = fEnergy.getEffectiveArea( iErec, fDataRun->Ze,
									   iDirectionOffset, iPedVar_temp,
									   fRunPara->fEnergyReconstructionSpectralIndex, true,
									   fRunPara->fEffectiveAreaVsEnergyMC );

					// fill energy histograms: require a valid effective area value
					if( iEnergyWeighting > 0. )
					{
						// energy histogram (counts per bin)
						fHisto[fHisCounter]->herecCounts->Fill( log10( iErec ) );
						fHisto[fHisCounter]->herecCounts2DtimeBinned->Fill( log10( iErec ), ( ( double )fDataRun->Time - f_t_in_s_min[irun] ) );
						fHisto[fHisCounter]->hLinerecCounts->Fill( iErec );
						fHisto[fHisCounter]->hLinerecCounts2DtimeBinned->Fill( iErec , ( ( double )fDataRun->Time - f_t_in_s_min[irun] ) );
						fHisto[fHisCounter]->herecWeights->Fill( log10( iErec ), log10( 1. / iEnergyWeighting ) );
						fHisto[fHisCounter]->hLinerecWeights->Fill( iErec, log10( 1. / iEnergyWeighting ) );
						fHisto[fHisCounter]->herecEffectiveArea->Fill( log10( iErec ), 1. / iEnergyWeighting );
						fHisto[fHisCounter]->hLinerecEffectiveArea->Fill( iErec, 1. / iEnergyWeighting );
						// filling the effective area for each time bin
						double time_of_previous_EVENT = time_of_EVENT;
						time_of_EVENT = ( ( double )fDataRun->Time - f_t_in_s_min[irun]);
						double index_time_bin_PREVIOUS_EVENT = index_time_bin_NOW;
						index_time_bin_NOW = fHisto[fHisCounter]->hRealDuration1DtimeBinned->FindFixBin(time_of_EVENT);

						if(time_of_previous_EVENT>time_of_EVENT) std::cout<<"ERROR events are not ordered chronolically "<<std::endl;

						if(index_time_bin_PREVIOUS_EVENT != index_time_bin_NOW){
						 //--- we just got into a new time bin
						 // getting the effective area for the time bin we just left
						 fEnergy.setTimeBin( fHisto[fHisCounter]->hRealDuration1DtimeBinned->GetBinCenter(index_time_bin_PREVIOUS_EVENT) );
						 fEnergy.setTimeBinnedMeanEffectiveArea();
						 fEnergy.resetTimeBin();
						}
					}
				}
				// mean azimuth and elevation (telescope orientation)
				// (get first running telescope)
				for( unsigned int t = 0; t < fRunPara->fRunList[fHisCounter].fTelToAnalyze.size(); t++ )
				{
					// for some runs LTrig is not filled!
					if( ( ( t + 1 ) & fDataRun->LTrig ) || ( fDataRun->LTrig == 0 ) )
					{
						fMeanAzimuth  = VSkyCoordinatesUtilities::addToMeanAzimuth( fMeanAzimuth, fDataRun->TelAzimuth[fRunPara->fRunList[fHisCounter].fTelToAnalyze[t]] );
						fMeanElevation += fDataRun->TelElevation[fRunPara->fRunList[fHisCounter].fTelToAnalyze[t]];
						fNMeanElevation++;
						break;
					}
				}
			}
			// event counter
			if( bIsGamma && bDirectionCuts )
			{
				i_count++;
				fTimeMask->countOn( i_UTC );      // keep track of gamma ON counts for rate plots
			}
		}
	}
	// END: loop over all entries/events in the data tree
	/////////////////////////////////////////////////////////////////////

	// filling the effective area for last time bin
	// fill energy histograms: require a valid effective area value
    if( iEnergyWeighting > 0. )
    {
         fEnergy.setTimeBin( fHisto[fHisCounter]->hRealDuration1DtimeBinned->GetBinCenter(index_time_bin_NOW) );
         fEnergy.setTimeBinnedMeanEffectiveArea();
         fEnergy.resetTimeBin();
    }
    // filling the histo with the duration of the time bin
    // looping over the mask seconds
    for(unsigned int i_s = 0 ; i_s < fTimeMask->getMaskSize() ; i_s++)
    {
         if(fTimeMask->getMask()[i_s])
         {
              // dead time is taken into account for each second
              double dead_time_fraction = fDeadTime[fHisCounter]->getDeadTimeFraction( ( double )i_s + 0.5,fRunPara->fDeadTimeCalculationMethod );
              fHisto[fHisCounter]->hRealDuration1DtimeBinned->Fill(i_s,1-dead_time_fraction);
         }

    }

	// fill rate vectors
	fTimeMask->getIntervalRates( iRateCounts, iRateTime, iRateTimeIntervall, fRunPara->fTimeIntervall );
	fRateCounts[fHisCounter] = iRateCounts;
	fRateTime[fHisCounter] = iRateTime;
	fRateTimeIntervall[fHisCounter] = iRateTimeIntervall;

	// finalize sky maps
	fMap->finalize( fIsOn, fCuts->getProbabilityCutAlpha( fIsOn ) );
	fMapUC->finalize( fIsOn, fCuts->getProbabilityCutAlpha( fIsOn ) );

	fTotCount += i_count;

	// calculate mean elevation
	if( fNMeanElevation > 0. )
	{
		fMeanAzimuth   /= fNMeanElevation;
		fMeanElevation /= fNMeanElevation;
	}
	// get mean effective area
	gMeanEffectiveArea = ( TGraphAsymmErrors* )fEnergy.getMeanEffectiveArea();
	if( gMeanEffectiveArea )
	{
		gMeanEffectiveArea = ( TGraphAsymmErrors* )gMeanEffectiveArea->Clone();
	}
	// get mean energy systematic histogram (needed possibly for energy threshold determination)
	gMeanEsys_MC = ( TGraphErrors* )fEnergy.getMeanSystematicErrorHistogram();
	if( gMeanEsys_MC )
	{
		gMeanEsys_MC = ( TGraphErrors* )gMeanEsys_MC->Clone();
	}

	// Likelihood analysis
	if ( fRunPara->fLikelihoodAnalysis )
	{

		gMeanEffectiveAreaMC = ( TGraphAsymmErrors* )fEnergy.getMeanEffectiveAreaMC();
		if( gMeanEffectiveAreaMC )
		{
			gMeanEffectiveAreaMC = ( TGraphAsymmErrors* )gMeanEffectiveAreaMC->Clone();
		}

		hResponseMatrix = ( TH2F* ) fEnergy.getMeanResponseMatrix();
		if ( hResponseMatrix )
		{
			hResponseMatrix = ( TH2F* )hResponseMatrix->Clone();
        }
        else
        {
            cout << "\t error: no response matrix found" << endl;
			cout << "\t Creating empty TH2D" << endl;
			hResponseMatrix = new TH2F("hResponseMatrix", "hResponseMatrix", 10, -1, 1, 10 , -1 , 1 );
        }
        if( fIsOn )
        {
             hResponseMatrix->SetName( "hResponseMatrix_on" );
        }
        else
        {
             hResponseMatrix->SetName( "hResponseMatrix_off" );
        }
	}

	// get mean effective area for TIME BINs
	gTimeBinnedMeanEffectiveArea = ( TGraph2DErrors* )fEnergy.getTimeBinnedMeanEffectiveArea()->Clone();
	// get dead time
	if( fHisCounter == 0 )
	{
		fDeadTimeStorage = fDeadTime[fHisCounter]->calculateDeadTime();
	}
	else
	{
		fDeadTime[fHisCounter]->calculateDeadTime();
	}
	fDeadTime[fHisCounter]->printDeadTime();

	// get mean run times after time cuts
	fRunExposure[irun] = fTimeMask->getEffectiveDuration();
	fRunMJD[irun] = fTimeMask->getMeanUTC_Mask();
	fTimeMask->printMask( 100, kTRUE );
	fTimeMask->printMeanTime( kTRUE );
	return i_count;
}


/*
 *
 * write created histograms to the appropriate directories and tidy up
 *
 */
void VStereoAnalysis::writeHistograms( bool bOn )
{
	if( fDebug )
	{
		cout << "DEBUG void VStereoAnalysis::writeHistograms()" << endl;
	}
	if( fHisCounter < 0 )
	{
		fHistoTot->writeHistograms();
	}
	else
	{
		if( fCuts )
		{
			fCuts->Write();
		}
		fTimeMask->writeObjects();
		if( bOn )
		{
			VTimeMask* iTimeMask = ( VTimeMask* )fTimeMask->Clone();
			iTimeMask->Write( "vtimemask" );
		}
		fHisto[fHisCounter]->writeHistograms();

		// need to grab fScalarDeadTimeFrac while fDeadTime histograms are intact,
		fRunPara->fScalarDeadTimeFrac = fDeadTime[fHisCounter]->getDeadTimeFraction( fTimeMask->getMask(), fRunPara->fDeadTimeCalculationMethod );

		fDeadTime[fHisCounter]->writeHistograms();
		// copy effective areas and radial acceptance to anasum output file
		if( bOn )
		{
			fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gMeanEffectiveArea );
			fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gTimeBinnedMeanEffectiveArea );
			fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gMeanEsys_MC );

			// Both MC and REC  effective areas are required for Binned Likelihood analysis
			if ( fRunPara->fLikelihoodAnalysis )
			{
                                if( gMeanEffectiveAreaMC )
                                {
                                    // (SOB) When nOn = 0 gMeanEffectiveAreaMC_on(E) = 0
                                    // VLikelihoodFitter will check gMeanEffectiveAreaMC_on::integral > 1
                                    gMeanEffectiveAreaMC->SetTitle("gMeanEffectiveAreaMC_on");
                                    gMeanEffectiveAreaMC->SetName("gMeanEffectiveAreaMC_on");

                                    fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gMeanEffectiveAreaMC );
                                }
                                if( hResponseMatrix )
                                {
                                    hResponseMatrix->SetTitle("hResponseMatrix_on");
                                    hResponseMatrix->SetName("hResponseMatrix_on");
                                    fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", hResponseMatrix );
                                }
			}
			if( fRunPara->fRunList[fHisCounter].fAcceptanceFile.size() > 0 
                        && fRunPara->fRunList[fHisCounter].fAcceptanceFile != "IGNOREACCEPTANCE" )
			{
				fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fAcceptanceFile, "RadialAcceptances", 0 );
			}
		}
		else
		{
			char hname[1000];
			if( gMeanEffectiveArea )
			{
				sprintf( hname, "%s_off", gMeanEffectiveArea->GetName() );
				gMeanEffectiveArea->SetName( hname );
			}
			if( gTimeBinnedMeanEffectiveArea )
			{
				sprintf( hname, "%s_off", gTimeBinnedMeanEffectiveArea->GetName() );
				gTimeBinnedMeanEffectiveArea->SetName( hname );
			}
			fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gMeanEffectiveArea );
			fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gTimeBinnedMeanEffectiveArea );

			// Both MC and REC  effective areas are required for Binned Likelihood analysis
                        // (SOB) A set of Off MC effective areas and response matrix are used as a back up
                        // If zero On counts the Off set will be used
			if ( fRunPara->fLikelihoodAnalysis )
			{
				if( gMeanEffectiveAreaMC )
				{
					gMeanEffectiveAreaMC->SetTitle("gMeanEffectiveAreaMC_off");
					gMeanEffectiveAreaMC->SetName("gMeanEffectiveAreaMC_off");
					fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gMeanEffectiveAreaMC );
				}
				if( hResponseMatrix )
				{
					hResponseMatrix->SetTitle("hResponseMatrix_off");
					hResponseMatrix->SetName("hResponseMatrix_off");
					fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", hResponseMatrix );
				}
			}
		}
		if( fDL3EventTree && fIsOn )
		{
		     write_DL3Tree() ;
		}
        if( fTreeSelectedEvents && fRunPara->fWriteDataOnOffTrees ) 
        {
            fTreeSelectedEvents->Write();
        }
	}
}


void VStereoAnalysis::writeDebugHistograms()
{
	if( fDebug )
	{
		cout << "DEBUG void VStereoAnalysis::writeDebugHistograms()" << endl;
	}

	TDirectory* iDir = gDirectory;

	if( iDir->mkdir( "debug" )->cd() )
	{
		if( fMap && fMap->getAux_hisList() )
		{
			fMap->getAux_hisList()->Write();
			fMap->getAux_hisList()->Delete();
		}
	}

	iDir->cd();
}

/*

    divide on by off alpha histograms

    this should only be called for OFF stereo analysis

*/
void VStereoAnalysis::scaleAlpha( TH2D* halpha_on, bool bUC )
{
	if( fIsOn )
	{
		cout << "VStereoAnalysis::scaleAlpha() error: this function should only be called for OFF stereo analysis" << endl;
		cout << "(this must be a coding error, please report)" << endl;
		exit( EXIT_FAILURE );
	}
	TH2D* halpha_off = 0;
	TH2D* hmap_alphaNorm = 0;
	// uncorrelated maps
	if( bUC )
	{
		if( fHisCounter < 0 )
		{
			halpha_off = fHistoTot->hmap_alphaUC;
			hmap_alphaNorm = fHistoTot->hmap_alphaNormUC;
		}
		else
		{
			halpha_off = fHisto[fHisCounter]->hmap_alphaUC;
			hmap_alphaNorm = fHisto[fHisCounter]->hmap_alphaNormUC;
		}
	}
	// correlated maps
	else
	{
		if( fHisCounter < 0 )
		{
			halpha_off = fHistoTot->hmap_alpha;
			hmap_alphaNorm = fHistoTot->hmap_alphaNorm;
		}
		else
		{
			halpha_off = fHisto[fHisCounter]->hmap_alpha;
			hmap_alphaNorm = fHisto[fHisCounter]->hmap_alphaNorm;
		}
	}
	if( !halpha_off || !hmap_alphaNorm )
	{
		cout << "VStereoAnalysis::scaleAlpha: fatal error, cannot find histograms ";
		cout << halpha_off << "\t" << hmap_alphaNorm << endl;
		exit( EXIT_FAILURE );
	}

	// halpha_on: on alpha histogram
	// halpha_off: off alpha histogram
	// hmap_alphaNorm: alpha histogram used in significance calculations (alphaNorm)
	// (this is slightly different for average alpha calculation)
	for( int i = 1; i <= halpha_off->GetNbinsX(); i++ )
	{
		for( int j = 1; j <= halpha_off->GetNbinsY(); j++ )
		{
			if( halpha_off->GetBinContent( i, j ) > 0. )
			{
				// this one is used for the sky maps
				hmap_alphaNorm->SetBinContent( i, j, halpha_on->GetBinContent( i, j ) / halpha_off->GetBinContent( i, j ) );
			}
			else
			{
				hmap_alphaNorm->SetBinContent( i, j, 0. );
			}
		}
	}
}


/*!
 *   combine histograms from all runs
 *
 *   this function shall be called at the end of the analysis
 */
double VStereoAnalysis::combineHistograms()
{
	unsigned int n_histo = fHisto.size();

	TDirectory* iDir = gDirectory;
	fDirTot->cd();
	fHistoTot->defineHistograms();

	// list of trees with selected events
	iDir->cd();

	///////////////////////////////////////////////////
	// loop over all runs (= all available histograms = n_histo)
	for( unsigned h = 0; h < n_histo; h++ )
	{
		fDirTotRun[h]->cd();
		// read in sky plots from disk
		fHisto[h]->readSkyPlots();

		/////////////////////////////
		// UNCORRELATED PLOTS
		int nxbin = fHistoTot->hmap_stereoUC->GetNbinsX();
		int nybin = fHistoTot->hmap_stereoUC->GetNbinsY();
		for( int i = 1; i <= nxbin; i++ )
		{
			for( int j = 1; j <= nybin; j++ )
			{
				// calculate average normalization (alpha) factor
				if( fHisto[h]->hmap_alphaUC && fHisto[h]->hmap_alphaUC->GetBinContent( i, j ) > 0.
						&& fHisto[h]->h_combine_map_alpha_offUC && fHisto[h]->h_combine_map_alpha_offUC->GetBinContent( i, j ) > 0. )
				{
					fHistoTot->hmap_stereoUC->SetBinContent( i, j, fHisto[h]->hmap_stereoUC->GetBinContent( i, j ) + fHistoTot->hmap_stereoUC->GetBinContent( i, j ) );
					// calculate average alpha
					if( fHisto[h]->h_combine_map_stereo_onUC && fHisto[h]->h_combine_map_stereo_offUC )
					{
						float alphaUC = 0.;
						if( fIsOn && fHisto[h]->h_combine_map_alpha_offUC->GetBinContent( i, j ) != -1. )
						{
							alphaUC = fHisto[h]->h_combine_map_alpha_offUC->GetBinContent( i, j ) / ( 1. + fHisto[h]->h_combine_map_alpha_offUC->GetBinContent( i, j ) );
						}
						else if( !fIsOn )
						{
							alphaUC = 1. / ( 1. + fHisto[h]->h_combine_map_alpha_offUC->GetBinContent( i, j ) );
						}
						alphaUC *= ( fHisto[h]->h_combine_map_stereo_onUC->GetBinContent( i, j ) + fHisto[h]->h_combine_map_stereo_offUC->GetBinContent( i, j ) );
						fHistoTot->hmap_alphaUC->SetBinContent( i, j, fHistoTot->hmap_alphaUC->GetBinContent( i, j ) + alphaUC );
					}
				}
			}
		}
		//////////////////////////////
		// CORRELATED PLOTS
		nxbin = fHistoTot->hmap_stereo->GetNbinsX();
		nybin = fHistoTot->hmap_stereo->GetNbinsY();
		for( int i = 1; i <= nxbin; i++ )
		{
			for( int j = 1; j <= nybin; j++ )
			{
				// calculate average normalization (alpha) factor
				if( fHisto[h]->hmap_alpha && fHisto[h]->hmap_alpha->GetBinContent( i, j ) > 0.
						&& fHisto[h]->h_combine_map_alpha_off && fHisto[h]->h_combine_map_alpha_off->GetBinContent( i, j ) > 0. )
				{
					fHistoTot->hmap_stereo->SetBinContent( i, j, fHisto[h]->hmap_stereo->GetBinContent( i, j ) + fHistoTot->hmap_stereo->GetBinContent( i, j ) );
					// calculate average alpha
					if( fHisto[h]->h_combine_map_stereo_on && fHisto[h]->h_combine_map_stereo_off )
					{
						float alpha = 0.;
						if( fIsOn && fHisto[h]->h_combine_map_alpha_off->GetBinContent( i, j ) != -1. )
						{
							alpha = fHisto[h]->h_combine_map_alpha_off->GetBinContent( i, j ) / ( 1. + fHisto[h]->h_combine_map_alpha_off->GetBinContent( i, j ) );
						}
						else if( !fIsOn )
						{
							alpha = 1. / ( 1. + fHisto[h]->h_combine_map_alpha_off->GetBinContent( i, j ) );
						}
						alpha *= ( fHisto[h]->h_combine_map_stereo_on->GetBinContent( i, j ) + fHisto[h]->h_combine_map_stereo_off->GetBinContent( i, j ) );
						fHistoTot->hmap_alpha->SetBinContent( i, j, fHistoTot->hmap_alpha->GetBinContent( i, j ) + alpha );
					}
				}
			}
		}
		fHisto[h]->deleteSkyPlots();
		iDir->cd();
	}  // (end loop over all histograms)

	//////////////////////////////////////
	// errors in sky maps (counting error)
	for( int i = 1; i <= fHistoTot->hmap_stereoUC->GetNbinsX(); i++ )
	{
		for( int j = 1; j <= fHistoTot->hmap_stereoUC->GetNbinsY(); j++ )
		{
			if( fHistoTot->hmap_stereoUC->GetBinContent( i, j ) > 0 )
			{
				fHistoTot->hmap_stereoUC->SetBinError( i, j, sqrt( fHistoTot->hmap_stereoUC->GetBinContent( i, j ) ) );
			}
		}
	}
	for( int i = 1; i <= fHistoTot->hmap_stereo->GetNbinsX(); i++ )
	{
		for( int j = 1; j <= fHistoTot->hmap_stereo->GetNbinsY(); j++ )
		{
			if( fHistoTot->hmap_stereo->GetBinContent( i, j ) > 0 )
			{
				fHistoTot->hmap_stereo->SetBinError( i, j, sqrt( fHistoTot->hmap_stereo->GetBinContent( i, j ) ) );
			}
		}
	}

	//////////////////////////////////////
	// combine parameter (1D) histograms
	for( unsigned int h = 0; h < n_histo; h++ )
	{
		iDir->cd();
		fDirTotRun[h]->cd();
		fHisto[h]->readParameterHistograms();
		TIter next( fHistoTot->hListParameterHistograms );
		TIter nexth( fHisto[h]->hListParameterHistograms );
		while( TH1* h1 = ( TH1* )next() )
		{
			TH1* h2 = ( TH1* )nexth();
			if( !h1 || !h2 )
			{
				continue;
			}

			string iTemp = h1->GetName();
			if( iTemp.find( "2D" ) != string::npos )
			{
				continue;
			}
			if( iTemp.find( "RealDuration1DtimeBinned" ) != string::npos )
			{
				continue;
			}
			h1->Add( h2 );
		}
		fHisto[h]->deleteParameterHistograms();
	}
	iDir->cd();

	// combine rate vectors (in time intervalls)
	for( unsigned int h = 0; h < n_histo; h++ )
	{
		for( unsigned int i = 0; i <  fRateCounts[h].size(); i++ )
		{
			fRateCountsTot.push_back( fRateCounts[h][i] );
		}
		for( unsigned int i = 0; i <  fRateTime[h].size(); i++ )
		{
			fRateTimeTot.push_back( fRateTime[h][i] );
		}
		for( unsigned int i = 0; i <  fRateTimeIntervall[h].size(); i++ )
		{
			fRateTimeIntervallTot.push_back( fRateTimeIntervall[h][i] );
		}
	}

	iDir->cd();
	return fTotCount;
}

TH1D* VStereoAnalysis::getMeanSignalBackgroundAreaRatio()
{
	if( fHisCounter < 0 )
	{
		return fHistoTot->hmap_MeanSignalBackgroundAreaRatio;
	}

	return fHisto[fHisCounter]->hmap_MeanSignalBackgroundAreaRatio;
}

TH1D* VStereoAnalysis::getMeanSignalBackgroundAreaRatioUC()
{
	if( fHisCounter < 0 )
	{
		return fHistoTot->hmap_MeanSignalBackgroundAreaRatioUC;
	}

	return fHisto[fHisCounter]->hmap_MeanSignalBackgroundAreaRatioUC;
}


TH2D* VStereoAnalysis::getAlpha()
{
	if( fDebug )
	{
		cout << "VStereoAnalysis::getAlpha() " << fHisCounter << endl;
	}

	if( fHisCounter < 0 )
	{
		return fHistoTot->hmap_alpha;
	}

	return fHisto[fHisCounter]->hmap_alpha;
}


TH2D* VStereoAnalysis::getAlphaUC()
{
	if( fDebug )
	{
		cout << "VStereoAnalysis::getAlphaUC() " << fHisCounter << endl;
	}

	if( fHisCounter < 0 )
	{
		return fHistoTot->hmap_alphaUC;
	}

	return fHisto[fHisCounter]->hmap_alphaUC;
}


TH2D* VStereoAnalysis::getAlphaNorm()
{
	if( fDebug )
	{
		cout << "VStereoAnalysis::getAlphaNorm() " << fHisCounter << endl;
	}

	if( fHisCounter < 0 )
	{
		return fHistoTot->hmap_alphaNorm;
	}

	return fHisto[fHisCounter]->hmap_alphaNorm;
}


TH2D* VStereoAnalysis::getAlphaNormUC()
{
	if( fDebug )
	{
		cout << "VStereoAnalysis::getAlphaNormUC() " << fHisCounter << endl;
	}

	if( fHisCounter < 0 )
	{
		return fHistoTot->hmap_alphaNormUC;
	}

	return fHisto[fHisCounter]->hmap_alphaNormUC;
}


TList* VStereoAnalysis::getHisList()
{
	if( fHisCounter < 0 )
	{
		return fHistoTot->hisList;
	}

	return fHisto[fHisCounter]->hisList;
}


TList* VStereoAnalysis::getSkyHistograms( bool bUC )
{
	if( fDebug )
	{
		cout << "VStereoAnalysis::getSkyHistograms() " << fHisCounter << "\t" << bUC << endl;
	}

	// uncorrelated plot
	if( bUC )
	{
		if( fHisCounter < 0 )
		{
			return fHistoTot->hListSkyMapsUC;
		}
		return fHisto[fHisCounter]->hListSkyMapsUC;
	}
	// correlated plots
	if( fHisCounter < 0 )
	{
		return fHistoTot->hListSkyMaps;
	}
	return fHisto[fHisCounter]->hListSkyMaps;
}


TList* VStereoAnalysis::getParameterHistograms()
{
	if( fHisCounter < 0 )
	{
		return fHistoTot->hListParameterHistograms;
	}

	return fHisto[fHisCounter]->hListParameterHistograms;
}


TH2D* VStereoAnalysis::getStereoSkyMapUC()
{
	if( fDebug )
	{
		cout << "VStereoAnalysis::getStereoSkyMapUC()" << "\t" << fHisCounter << endl;
	}

	if( fHisCounter < 0 )
	{
		return fHistoTot->hmap_stereoUC;
	}

	return fHisto[fHisCounter]->hmap_stereoUC;
}


TH2D* VStereoAnalysis::getStereoSkyMap()
{
	if( fDebug )
	{
		cout << "VStereoAnalysis::getStereoSkyMap()" << "\t" << fHisCounter << endl;
	}

	if( fHisCounter < 0 )
	{
		return fHistoTot->hmap_stereo;
	}

	return fHisto[fHisCounter]->hmap_stereo;
}


double VStereoAnalysis::getDeadTimeFraction()
{
	if( fDebug )
	{
		cout << "VStereoAnalysis::getDeadTimeFraction()" << endl;
	}

	if( fHisCounter < 0 )
	{
		return 0.;
	}

	if( fHisCounter < ( int )fDeadTime.size() )
	{
		// dead time depending on time mask
		if( fTimeMask && fTimeMask->getMask().size() > 0 )
		{
			return fDeadTime[fHisCounter]->getDeadTimeFraction( fTimeMask->getMask(), fRunPara->fDeadTimeCalculationMethod );
		}
		return fDeadTime[fHisCounter]->getDeadTimeFraction( -1, fRunPara->fDeadTimeCalculationMethod );
	}

	return 0.;
}

void VStereoAnalysis::astro_check_for_valid_coordinates( unsigned int runlist_iter )
{
    // (this is the target of observation)
    /////////////////////////////////////////////////////////
    if( fRunPara->fRunList[runlist_iter].fTargetDecJ2000 < -89.99 )
    {
        cout << "ERROR in VStereoAnalysis::astro_check_for_valid_coordinates: invalid target ";
        cout << fRunPara->fRunList[runlist_iter].fTarget << endl;
        cout << "\t run " << fRunPara->fRunList[runlist_iter].fRunOn << "\t" << fRunPara->fRunList[runlist_iter].fTarget;
        cout << fRunPara->fRunList[runlist_iter].fTargetDecJ2000 << "\t" << fRunPara->fRunList[runlist_iter].fTargetShiftDecJ2000 << endl;
        exit( EXIT_FAILURE );
    }
}

void VStereoAnalysis::astro_set_skymap_center_from_runparameters( unsigned int runlist_iter )
{
    /////////////////////////////////////////////////////////
    // Case 1: sky map centre is given as xy offset [deg] in runparameter file
    if( TMath::Abs( fRunPara->fSkyMapCentreNorth ) > 1.e-8 || TMath::Abs( fRunPara->fSkyMapCentreWest ) > 1.e-8 )
    {
        fRunPara->fRunList[runlist_iter].fSkyMapCentreWest  = fRunPara->fSkyMapCentreWest;
        fRunPara->fRunList[runlist_iter].fSkyMapCentreNorth = fRunPara->fSkyMapCentreNorth;
        double i_decDiff =  0.;   // offset in dec
        double i_raDiff = 0.;     // offset in ra
        VSkyCoordinatesUtilities::getWobbleOffset_in_RADec(
                fRunPara->fRunList[runlist_iter].fSkyMapCentreNorth,
                fRunPara->fRunList[runlist_iter].fSkyMapCentreWest,
                fRunPara->fRunList[runlist_iter].fTargetRAJ2000, fRunPara->fRunList[runlist_iter].fTargetDecJ2000,
                i_decDiff, i_raDiff );
        fRunPara->fRunList[runlist_iter].fSkyMapCentreRAJ2000  = fRunPara->fRunList[runlist_iter].fTargetRAJ2000 + i_raDiff;
        fRunPara->fRunList[runlist_iter].fSkyMapCentreDecJ2000 = fRunPara->fRunList[runlist_iter].fTargetDecJ2000 + i_decDiff;
    }
    // Case 2: sky map centre is given in J200 in runparameter file
    // (this is in almost all analysis the usual/default case)
    else if( TMath::Abs( fRunPara->fSkyMapCentreRAJ2000 ) > 1.e-8 )
    {
        fRunPara->fRunList[runlist_iter].fSkyMapCentreRAJ2000  = fRunPara->fSkyMapCentreRAJ2000;
        fRunPara->fRunList[runlist_iter].fSkyMapCentreDecJ2000 = fRunPara->fSkyMapCentreDecJ2000;
        fRunPara->fRunList[runlist_iter].fSkyMapCentreWest =
            VSkyCoordinatesUtilities::getTargetShiftWest( 
                    fRunPara->fRunList[runlist_iter].fTargetRAJ2000, fRunPara->fRunList[runlist_iter].fTargetDecJ2000,
                    fRunPara->fSkyMapCentreRAJ2000, fRunPara->fSkyMapCentreDecJ2000 ) * -1.;
        fRunPara->fRunList[runlist_iter].fSkyMapCentreNorth =
            VSkyCoordinatesUtilities::getTargetShiftNorth( 
                    fRunPara->fRunList[runlist_iter].fTargetRAJ2000, fRunPara->fRunList[runlist_iter].fTargetDecJ2000,
                    fRunPara->fSkyMapCentreRAJ2000, fRunPara->fSkyMapCentreDecJ2000 );
    }
    // if not set in runparameter file: set to target direction
    else
    {
        fRunPara->fRunList[runlist_iter].fSkyMapCentreRAJ2000 = fRunPara->fRunList[runlist_iter].fTargetRAJ2000;
        fRunPara->fRunList[runlist_iter].fSkyMapCentreDecJ2000 = fRunPara->fRunList[runlist_iter].fTargetDecJ2000;
        fRunPara->fSkyMapCentreRAJ2000 = fRunPara->fRunList[runlist_iter].fSkyMapCentreRAJ2000;
        fRunPara->fSkyMapCentreDecJ2000 = fRunPara->fRunList[runlist_iter].fSkyMapCentreDecJ2000;
    }
}

void VStereoAnalysis::astro_set_skymap_centershift_from_runparameters( unsigned int runlist_iter )
{
    /////////////////////////////////////////////////////////
    // from runparameter file: set and get target shifts
    // (calculated relative to sky map centre)
    // (this is the position where all 1D histograms (theta2, energy spectra, etc) are calculated)
    if( fIsOn )
    {
        if( TMath::Abs( fRunPara->fTargetShiftDecJ2000 ) > 1.e-8 || TMath::Abs( fRunPara->fTargetShiftRAJ2000 ) > 1.e-8 )
        {
            fRunPara->fRunList[runlist_iter].fTargetShiftWest = VSkyCoordinatesUtilities::getTargetShiftWest( 
                    fRunPara->fRunList[runlist_iter].fTargetRAJ2000, fRunPara->fRunList[runlist_iter].fTargetDecJ2000,
                    fRunPara->fTargetShiftRAJ2000, fRunPara->fTargetShiftDecJ2000 );
            fRunPara->fRunList[runlist_iter].fTargetShiftNorth = -1.*VSkyCoordinatesUtilities::getTargetShiftNorth( 
                    fRunPara->fRunList[runlist_iter].fTargetRAJ2000, fRunPara->fRunList[runlist_iter].fTargetDecJ2000,
                    fRunPara->fTargetShiftRAJ2000, fRunPara->fTargetShiftDecJ2000 );

            fRunPara->fRunList[runlist_iter].fTargetShiftWest  += fRunPara->fRunList[runlist_iter].fSkyMapCentreWest;
            fRunPara->fRunList[runlist_iter].fTargetShiftNorth += fRunPara->fRunList[runlist_iter].fSkyMapCentreNorth;
        }
        else
        {
            fRunPara->fRunList[runlist_iter].fTargetShiftWest  = fRunPara->fTargetShiftWest;
            fRunPara->fRunList[runlist_iter].fTargetShiftNorth = fRunPara->fTargetShiftNorth;
        }
        fRunPara->fRunList[runlist_iter].fTargetShiftWest *= -1.;
        fRunPara->fTargetShiftWest = fRunPara->fRunList[runlist_iter].fTargetShiftWest;
        fRunPara->fTargetShiftNorth = fRunPara->fRunList[runlist_iter].fTargetShiftNorth;
        fRunPara->setTargetShifts( runlist_iter );
    }
}

double VStereoAnalysis::astro_get_mjd( unsigned int runlist_iter )
{
    double iMJD = ( double )fRunPara->fRunList[runlist_iter].fMJDOn;
    if( !fIsOn )
    {
        iMJD = ( double )fRunPara->fRunList[runlist_iter].fMJDOff;
    }
    return iMJD;
}

/*
 * convert wobble offsets from angle in the sky
 * to offsets in RA/Dec
 */
pair< double, double > VStereoAnalysis::astro_get_wobbleoffset_radec( unsigned int runlist_iter, bool bPrint )
{
    // calculate wobble offset in ra/dec for current epoch
    pair< double, double > i_radec_diff;

    VSkyCoordinatesUtilities::getWobbleOffset_in_RADec( 
            fRunPara->fRunList[runlist_iter].fWobbleNorth, 
            -1.*fRunPara->fRunList[runlist_iter].fWobbleWest,
            fRunPara->fRunList[runlist_iter].fTargetDec,
            fRunPara->fRunList[runlist_iter].fTargetRA,
            i_radec_diff.second,
            i_radec_diff.first );
    if( i_radec_diff.first < -180. )
    {
        i_radec_diff.first += 360.;
    }
    if( fIsOn && bPrint )
    {
        cout << "\tWobble offsets (currE): N: ";
        cout << fRunPara->fRunList[runlist_iter].fWobbleNorth;
        cout << " W: " << fRunPara->fRunList[runlist_iter].fWobbleWest;
        cout << ",  RA " << i_radec_diff.first << ", Dec " << i_radec_diff.second << endl;
    }
    return i_radec_diff;
}

/*
 * array pointing (center of pointing FOV)
 * in current epoch
 */
pair< double, double > VStereoAnalysis::astro_get_arraypointing( unsigned int runlist_iter, bool bPrint )
{
    pair< double, double > ra_dec_wobbleoffset = astro_get_wobbleoffset_radec( runlist_iter, bPrint );

    pair< double, double > i_radec_arraypointing;
    i_radec_arraypointing.first = fRunPara->fRunList[runlist_iter].fTargetRA   + ra_dec_wobbleoffset.first;
    i_radec_arraypointing.second = fRunPara->fRunList[runlist_iter].fTargetDec + ra_dec_wobbleoffset.second;

    return i_radec_arraypointing;
}

/*
 * array pointing (center of pointing FOV)
 * in J2000
 */
pair< double, double > VStereoAnalysis::astro_get_arraypointingJ2000( unsigned int runlist_iter )
{
    pair< double, double > i_radec_arraypointing = astro_get_arraypointing(runlist_iter);
    // correct for precession (from current epoch to J2000=MJD51544)
    VSkyCoordinatesUtilities::precessTarget( 
            51544., 
            i_radec_arraypointing.first, i_radec_arraypointing.second,
            astro_get_mjd(runlist_iter), true );
    return i_radec_arraypointing;
}

void VStereoAnalysis::astro_calculate_modified_wobbleoffset( unsigned int runlist_iter )
{
    pair< double, double > ra_dec_arraypointing = astro_get_arraypointingJ2000(runlist_iter);

    double i_WobbleJ2000_West = VSkyCoordinatesUtilities::getTargetShiftWest( 
                                fRunPara->fRunList[runlist_iter].fTargetRAJ2000, 
                                fRunPara->fRunList[runlist_iter].fTargetDecJ2000,
                                ra_dec_arraypointing.first,
                                ra_dec_arraypointing.second) * -1.;
    double i_WobbleJ2000_North = VSkyCoordinatesUtilities::getTargetShiftNorth( 
                                 fRunPara->fRunList[runlist_iter].fTargetRAJ2000, 
                                 fRunPara->fRunList[runlist_iter].fTargetDecJ2000,
                                 ra_dec_arraypointing.first,
                                 ra_dec_arraypointing.second);

    // modify wobble offsets for centering of sky maps
    fRunPara->fRunList[runlist_iter].fWobbleNorthMod = i_WobbleJ2000_North - fRunPara->fRunList[runlist_iter].fSkyMapCentreNorth;
    fRunPara->fRunList[runlist_iter].fWobbleWestMod  = i_WobbleJ2000_West  - fRunPara->fRunList[runlist_iter].fSkyMapCentreWest;

    if( fIsOn )
    {
        cout << "\tWobble offsets (J2000): N: " << i_WobbleJ2000_North << " W: " << i_WobbleJ2000_West << endl;
        cout << "\tSky maps centred at (ra,dec (J2000)) (";
        cout << fRunPara->fRunList[runlist_iter].fSkyMapCentreRAJ2000 << ", ";
        cout << fRunPara->fRunList[runlist_iter].fSkyMapCentreDecJ2000 << ")";
        cout << endl;
        cout << "\tTelescopes pointing to: (ra,dec (J2000)) (";
        cout << ra_dec_arraypointing.first << ", ";
        cout << ra_dec_arraypointing.second << ")";
        cout << ", N: " << fRunPara->fRunList[runlist_iter].fWobbleNorthMod;
        cout << " W: " << fRunPara->fRunList[runlist_iter].fWobbleWestMod << endl;
        cout << "\t1D-histograms calculated at (x,y): ";
        cout << fRunPara->fRunList[runlist_iter].fTargetShiftNorth << ", " << fRunPara->fRunList[runlist_iter].fTargetShiftWest;
        if( TMath::Abs( fRunPara->fTargetShiftDecJ2000 ) > 1.e-8 &&  TMath::Abs( fRunPara->fTargetShiftRAJ2000 ) > 1.e-8 )
        {
            cout << " (ra,dec (J2000)) " << fRunPara->fTargetShiftRAJ2000 << ", " << fRunPara->fTargetShiftDecJ2000;
        }
        cout << endl;
    }
}

/*
 * precess telescope pointing from J2000 to current epoch
 * and return current epoch coordinates
 */
pair< double, double > VStereoAnalysis::astro_calculate_ra_dec_currentEpoch( unsigned int runlist_iter )
{
    pair< double, double > i_radec;
    i_radec.first = fRunPara->fRunList[runlist_iter].fTargetRAJ2000 * TMath::DegToRad();
    i_radec.second = fRunPara->fRunList[runlist_iter].fTargetDecJ2000 * TMath::DegToRad();

    VSkyCoordinatesUtilities::precessTarget( astro_get_mjd( runlist_iter ), i_radec.first, i_radec.second );
    // set target coordinates into run parameter list
    fRunPara->setTargetRADec_currentEpoch( 
            runlist_iter,
            i_radec.first * TMath::RadToDeg(),
            i_radec.second * TMath::RadToDeg() );

    return i_radec;
}

void VStereoAnalysis::astro_print_pointing( unsigned int runlist_iter )
{
    if( !fIsOn ) return;
    // print some information on targeting/pointing to screen
    cout << "Run " << fRunPara->fRunList[runlist_iter].fRunOn << " ---------------------------" << endl;
    // print target info to screen
    cout << "\tTarget: " << fRunPara->fRunList[runlist_iter].fTarget << " (ra,dec)=(";
    cout << fRunPara->fRunList[runlist_iter].fTargetRA << ", " << fRunPara->fRunList[runlist_iter].fTargetDec << ")";
    cout << " (precessed, MJD=" << astro_get_mjd( runlist_iter ) << "), ";
    cout << "(ra,dec (J2000)) = (";
    cout << fRunPara->fRunList[runlist_iter].fTargetRAJ2000 << ", ";
    cout << fRunPara->fRunList[runlist_iter].fTargetDecJ2000 << ")";
    if( TMath::Abs( fRunPara->fRunList[runlist_iter].fPairOffset ) > 1.e-2 )
    {
        cout << ", pair offset [min]: " << fRunPara->fRunList[runlist_iter].fPairOffset;
    }
    cout << endl;
}

/*
 * set and get the regions to exclude
 * (these are calculated relative to the sky map centre)
*/
void VStereoAnalysis::astro_set_exclusionsregions( unsigned int runlist_iter )
{
    astro_setup_star_cataloge(runlist_iter);

    for( unsigned int k = 0 ; k < fRunPara->fExclusionRegions.size(); k++ )
    {
        if( fRunPara->fExclusionRegions[k]->fExcludeFromBackground_DecJ2000 < -90. )
        {
            continue;
        }
        fRunPara->fExclusionRegions[k]->fExcludeFromBackground_West  = 
             VSkyCoordinatesUtilities::getTargetShiftWest( 
                fRunPara->fRunList[runlist_iter].fSkyMapCentreRAJ2000,
                fRunPara->fRunList[runlist_iter].fSkyMapCentreDecJ2000,
                fRunPara->fExclusionRegions[k]->fExcludeFromBackground_RAJ2000,
                fRunPara->fExclusionRegions[k]->fExcludeFromBackground_DecJ2000 );

        fRunPara->fExclusionRegions[k]->fExcludeFromBackground_North = 
             VSkyCoordinatesUtilities::getTargetShiftNorth( 
                fRunPara->fRunList[runlist_iter].fSkyMapCentreRAJ2000,
                fRunPara->fRunList[runlist_iter].fSkyMapCentreDecJ2000,
                fRunPara->fExclusionRegions[k]->fExcludeFromBackground_RAJ2000,
                fRunPara->fExclusionRegions[k]->fExcludeFromBackground_DecJ2000 );

        if( TMath::Abs( fRunPara->fExclusionRegions[k]->fExcludeFromBackground_North ) < 1.e-8 )
        {
            fRunPara->fExclusionRegions[k]->fExcludeFromBackground_North = 0.;
        }
        if( TMath::Abs( fRunPara->fExclusionRegions[k]->fExcludeFromBackground_West ) < 1.e-8 )
        {
                fRunPara->fExclusionRegions[k]->fExcludeFromBackground_West  = 0.;
        }
    }
}

/* 
 * set up star catalogue and exclusion regions
 * (all in J2000)
 */
void VStereoAnalysis::astro_setup_star_cataloge( unsigned int runlist_iter )
{
    if( !fIsOn ) return;

    fAstro.back()->initStarCatalogue( 
            fRunPara->fStarCatalogue,
            astro_get_mjd(runlist_iter),
            fRunPara->fSkyMapSizeXmin, fRunPara->fSkyMapSizeXmax,
            fRunPara->fSkyMapSizeYmin, fRunPara->fSkyMapSizeYmax,
            fRunPara->fRunList[runlist_iter].fSkyMapCentreRAJ2000, fRunPara->fRunList[runlist_iter].fSkyMapCentreDecJ2000 );
    VStarCatalogue* iStarCatalogue = fAstro.back()->getStarCatalogue();
    if( !iStarCatalogue )
    {
        cout << "VStereoAnalysis::astro_setup_star_cataloge() error: star catalogue not found: " << fRunPara->fStarCatalogue << endl;
        cout << "exiting..." << endl;
        exit( EXIT_FAILURE );
    }
    // remove double entries
    iStarCatalogue->purge();
    if( iStarCatalogue->getListOfStarsinFOV().size() == 0 ) return;

    cout << "\tbright stars (magnitude brighter than " << fRunPara->fStarMinBrightness << ", exclusion radius ";
    cout << fRunPara->fStarExlusionRadius << " deg, " << fRunPara->fStarBand << "-band) in field of view (J2000): " << endl;
    cout << "\t\t ID \t RA \t Dec \t magnitude " << endl;
    for( unsigned int i = 0; i < iStarCatalogue->getListOfStarsinFOV().size(); i++ )
    {
        if( !iStarCatalogue->getListOfStarsinFOV()[i]
          || iStarCatalogue->getListOfStarsinFOV()[i]->getBrightness( fRunPara->fStarBand ) 
              >= fRunPara->fStarMinBrightness
          || fRunPara->fStarExlusionRadius <= 0. )
        {
            continue;
        }
        iStarCatalogue->getListOfStarsinFOV()[i]->printStar_for_anasum( fRunPara->fStarBand );

        // check if this region is already excluded (avoid dublications)
        bool b_isExcluded = false;
        for( unsigned int e = 0; e < fRunPara->fExclusionRegions.size(); e++ )
        {
            if( fRunPara->fExclusionRegions[e]->fExcludeFromBackground_StarID >= 0
             && fRunPara->fExclusionRegions[e]->fExcludeFromBackground_StarID == (int)iStarCatalogue->getListOfStarsinFOV()[i]->fStarID )
            {
                b_isExcluded = true;
            }
        }
        if( b_isExcluded ) continue;
        fRunPara->fExclusionRegions.push_back( new VAnaSumRunParameterListOfExclusionRegions() );
        fRunPara->fExclusionRegions.back()->fExcludeFromBackground_RAJ2000 = iStarCatalogue->getListOfStarsinFOV()[i]->fRA2000;
        fRunPara->fExclusionRegions.back()->fExcludeFromBackground_DecJ2000 = iStarCatalogue->getListOfStarsinFOV()[i]->fDec2000;
        fRunPara->fExclusionRegions.back()->fExcludeFromBackground_Radius1 = fRunPara->fStarExlusionRadius;
        fRunPara->fExclusionRegions.back()->fExcludeFromBackground_Radius2 = fRunPara->fStarExlusionRadius;
        fRunPara->fExclusionRegions.back()->fExcludeFromBackground_North = 0.;
        fRunPara->fExclusionRegions.back()->fExcludeFromBackground_West = 0.;
        fRunPara->fExclusionRegions.back()->fExcludeFromBackground_StarID = ( int )iStarCatalogue->getListOfStarsinFOV()[i]->fStarID;
        fRunPara->fExclusionRegions.back()->fExcludeFromBackground_StarName = iStarCatalogue->getListOfStarsinFOV()[i]->fStarName;
        fRunPara->fExclusionRegions.back()->fExcludeFromBackground_StarBrightness_V = iStarCatalogue->getListOfStarsinFOV()[i]->fBrightness_V;
        fRunPara->fExclusionRegions.back()->fExcludeFromBackground_StarBrightness_B = iStarCatalogue->getListOfStarsinFOV()[i]->fBrightness_B;
    }
    iStarCatalogue->purge();
}

/*
  setup all coordinate systems

  - set targets
  - set sky map centres
  - set area to calculate 1D histograms and energy spectra
  - set exclusion regions for background calculation

  Note that throughout the analysis and mapfilling, the coordinate system of choice is J2000

*/
void VStereoAnalysis::defineAstroSource()
{
	if( fDebug )
	{
		cout << "VStereoAnalysis::defineAstroSource()" << endl;
	}

	if( fIsOn )
	{
		cout << endl << "-----------------------------------------------------------------------" << endl;
		cout << "Defining targets and exclusion regions" << endl;
	}

	for( unsigned int i = 0; i < fRunPara->fRunList.size(); i++ )
	{
        astro_check_for_valid_coordinates( i );
        astro_set_skymap_center_from_runparameters( i );
        astro_set_skymap_centershift_from_runparameters( i );

        astro_calculate_ra_dec_currentEpoch( i );
        astro_print_pointing( i );
        astro_calculate_modified_wobbleoffset( i );
        fRunPara->setArrayPointing(
                i,
                astro_get_arraypointing(i, false),
                astro_get_arraypointingJ2000(i) );

		// fill run parameter values
		fRunPara->setTargetRADecJ2000( i );
		fRunPara->setTargetShifts( i );
		fRunPara->setSkyMapCentreJ2000( i );

		// define source and tracking class
		fAstro.push_back( new VSkyCoordinates() );
        fAstro.back()->setTelRADec_deg( astro_get_arraypointing( i, false ) );

		fAstro.back()->setObservatory( 
                fRunPara->getObservatory_Longitude_deg(), 
                fRunPara->getObservatory_Latitude_deg() );

        astro_set_exclusionsregions( i );
	}
}


void VStereoAnalysis::setCuts( VAnaSumRunParameterDataClass iL, int irun )
{
	if( iL.fCutFile != "" )
	{
		// read cuts from root file
		if( iL.fCutFile.find( ".root" ) != string::npos )
		{
			string iEffFile = VUtilities::testFileLocation( iL.fCutFile, "EffectiveAreas", true );

			TFile* iF  = new TFile( iEffFile.c_str() );
			if( iF->IsZombie() )
			{
				cout << "VStereoAnalysis::setCuts error opening file to read cuts: " << endl;
				cout << "\t" << iEffFile << endl;
				cout << "exiting..." << endl;
				exit( EXIT_FAILURE );
			}
			VGammaHadronCuts* iC = ( VGammaHadronCuts* )iF->Get( "GammaHadronCuts" );
			if( !iC )
			{
				cout << "VStereoAnalysis::setCuts error reading cuts from file: " << endl;
				cout << "\t" << iEffFile << endl;
				cout << "exciting..." << endl;
				exit( EXIT_FAILURE );
			}
			fCuts = iC;
			iF->Close();
		}
		// read cuts from text file
		else
		{
			fCuts->setNTel( iL.fMaxTelID );
			fCuts->setInstrumentEpoch( fInstrumentEpoch );
			fCuts->setTelToAnalyze( fTelToAnalyze );
			fCuts->readCuts( iL.fCutFile );
			fCuts->setTheta2Cut( iL.fSourceRadius );
		}
	}
	else
	{
		fCuts->resetCutValues();
	}
	fCuts->initializeCuts( irun );
	fCuts->printCutSummary();
}


vector< double > VStereoAnalysis::getRateCounts()
{
	if( fHisCounter < 0 )
	{
		return fRateCountsTot;
	}

	if( fHisCounter < ( int )fRateCounts.size() )
	{
		return fRateCounts[fHisCounter];
	}

	// this shouldn't happen
	vector< double > f;
	return f;
}


vector< double > VStereoAnalysis::getRateTime()
{
	if( fHisCounter < 0 )
	{
		return fRateTimeTot;
	}

	if( fHisCounter < ( int )fRateTime.size() )
	{
		return fRateTime[fHisCounter];
	}

	// this shouldn't happen
	vector< double > f;
	return f;
}


vector< double > VStereoAnalysis::getRateTimeIntervall()
{
	if( fHisCounter < 0 )
	{
		return fRateTimeIntervallTot;
	}

	if( fHisCounter < ( int )fRateTimeIntervall.size() )
	{
		return fRateTimeIntervall[fHisCounter];
	}

	// this shouldn't happen
	vector< double > f;
	return f;
}


TList* VStereoAnalysis::getEnergyHistograms()
{
	if( fHisCounter < 0 )
	{
		return fHistoTot->hListEnergyHistograms;
	}
	else if( fHisCounter < ( int )fHisto.size() )
	{
		return fHisto[fHisCounter]->hListEnergyHistograms;
	}

	return 0;
}


TH1D* VStereoAnalysis::getTheta2()
{
	if( fHisCounter < 0 )
	{
		return fHistoTot->htheta2;
	}
	else if( fHisCounter < ( int )fHisto.size() )
	{
		return fHisto[fHisCounter]->htheta2;
	}

	return 0;
}


double VStereoAnalysis::getRawRate()
{
	if( fHisCounter < 0 )
	{
		return 0.;
	}
	else if( fHisCounter < ( int )fHisto.size() )
	{
		return fHisto[fHisCounter]->hrate_1sec->GetEntries();
	}

	return 0.;
}


CData* VStereoAnalysis::getDataFromFile( int i_runNumber )
{
	CData* c = 0;
	for( unsigned int i = 0; i < fRunPara->fRunList.size(); i++ )
	{
		int i_run = fIsOn ? fRunPara->fRunList[i].fRunOn : fRunPara->fRunList[i].fRunOff;

		if( i_runNumber > 0 && i_runNumber != i_run )
		{
			continue;
		}

		string iFileName = fIsOn ? fRunPara->fRunList[i].fRunOnFileName : fRunPara->fRunList[i].fRunOffFileName;

		fDataFile = new TFile( iFileName.c_str() );
		if( fDataFile->IsZombie() )
		{
			cout << "VStereoAnalysis::getDataFromFile() error opening file " << iFileName << endl;
			cout << "exiting..." << endl;
			exit( EXIT_FAILURE );
		}
		fDataRunTree = ( TTree* )fDataFile->Get( "data" );
		if( !fDataRunTree )
		{
			cout << "VStereoAnalysis::getDataFromFile() error: cannot find data tree in " << iFileName << endl;
			cout << "exiting..." << endl;
			exit( EXIT_FAILURE );
		}
		c = new CData( fDataRunTree );
		// read current (major) epoch from data file
		VEvndispRunParameter* i_runPara = ( VEvndispRunParameter* )fDataFile->Get( "runparameterV2" );
		if( i_runPara )
		{
			fInstrumentEpoch = i_runPara->getInstrumentEpoch( true );
			fTelToAnalyze = i_runPara->fTelToAnalyze;
		}
		else
		{
			cout << "VStereoAnalysis::getDataFromFile() warning: epoch of current file " << endl;
			cout << "and active telescope combination cannot be determined; " << endl;
			cout << "this might lead to a wrong choice in the gamma/hadron cuts - please check" << endl;
			fInstrumentEpoch = "NOT_FOUND";
		}
	}
	return c;
}


bool VStereoAnalysis::closeDataFile()
{
	if( fDataFile )
	{
		fDataFile->Close();
	}

	return true;
}


bool VStereoAnalysis::terminate()
{
	closeDataFile();

	return true;
}

bool VStereoAnalysis::init_TreeWithSelectedEvents( int irun, bool isOn )
{
	if( fTreeSelectedEvents )
	{
		delete fTreeSelectedEvents;
	}
	if( !fRunPara )
	{
		return false;
	}

	char hname[200];
	char htitle[200];
	if( isOn )
	{
		sprintf( hname, "data_on" );
		sprintf( htitle, "selected events (on) for run %d", irun );
	}
	else
	{
		sprintf( hname, "data_off" );
		sprintf( htitle, "selected events (off) for run %d", irun );
	}
	fTreeSelectedEvents = new TTree( hname, htitle );
	if( !isOn )
	{
		fTreeSelectedEvents->SetLineColor( 2 );
	}

	fTreeSelectedEvents->Branch( "runNumber", &fTreeSelected_runNumber, "runNumber/I" );
	fTreeSelectedEvents->Branch( "eventNumber", &fTreeSelected_eventNumber, "eventNumber/I" );
	fTreeSelectedEvents->Branch( "MJD", &fTreeSelected_MJD, "MJD/I" );
	fTreeSelectedEvents->Branch( "Time", &fTreeSelected_Time, "Time/D" );
	fTreeSelectedEvents->Branch( "NImages", &fTreeSelected_NImages, "NImages/I" );
	fTreeSelectedEvents->Branch( "ImgSel", &fTreeSelected_ImgSel, "ImgSel/l" );
	fTreeSelectedEvents->Branch( "theta2", &fTreeSelected_theta2, "theta2/D" );
	fTreeSelectedEvents->Branch( "Xoff", &fTreeSelected_Xoff, "Xoff/D" );
	fTreeSelectedEvents->Branch( "Yoff", &fTreeSelected_Yoff, "Yoff/D" );
	fTreeSelectedEvents->Branch( "Xoff_derot", &fTreeSelected_Xoff_derot, "Xoff_derot/D" );
	fTreeSelectedEvents->Branch( "Yoff_derot", &fTreeSelected_Yoff_derot, "Yoff_derot/D" );
	fTreeSelectedEvents->Branch( "Xcore", &fTreeSelected_Xcore, "Xcore/D" );
	fTreeSelectedEvents->Branch( "Ycore", &fTreeSelected_Ycore, "Ycore/D" );
	fTreeSelectedEvents->Branch( "MSCW", &fTreeSelected_MSCW, "MSCW/D" );
	fTreeSelectedEvents->Branch( "MSCL", &fTreeSelected_MSCL, "MSCL/D" );
	fTreeSelectedEvents->Branch( "MWR", &fTreeSelected_MWR, "MWR/D" );
	fTreeSelectedEvents->Branch( "MLR", &fTreeSelected_MLR, "MLR/D" );
	fTreeSelectedEvents->Branch( "Erec", &fTreeSelected_Erec, "Erec/D" );
	fTreeSelectedEvents->Branch( "EChi2", &fTreeSelected_EChi2, "EChi2/D" );
	fTreeSelectedEvents->Branch( "ErecS", &fTreeSelected_ErecS, "ErecS/D" );
	fTreeSelectedEvents->Branch( "EChi2S", &fTreeSelected_EChi2S, "EChi2S/D" );
	fTreeSelectedEvents->Branch( "EmissionHeight", &fTreeSelected_EmissionHeight, "EmissionHeight/F" );
	fTreeSelectedEvents->Branch( "EmissionHeightChi2", &fTreeSelected_EmissionHeightChi2, "EmissionHeightChi2/F" );
	fTreeSelectedEvents->Branch( "SizeSecondMax", &fTreeSelected_SizeSecondMax, "SizeSecondMax/D" );
	fTreeSelectedEvents->Branch( "MVA", &fTreeSelected_MVA, "MVA/D" );
	fTreeSelectedEvents->Branch( "IsGamma", &fTreeSelected_IsGamma, "IsGamma/i" );

	return true;
}

void VStereoAnalysis::reset_TreeWithSelectedEvents()
{
	fTreeSelected_runNumber = 0;
	fTreeSelected_eventNumber = 0;
	fTreeSelected_MJD = 0;
	fTreeSelected_Time = 0.;
	fTreeSelected_NImages = 0;
	fTreeSelected_ImgSel = 0;
	fTreeSelected_theta2 = 0.;
	fTreeSelected_Xoff = 0.;
	fTreeSelected_Yoff = 0.;
	fTreeSelected_Xoff_derot = 0.;
	fTreeSelected_Yoff_derot = 0.;
	fTreeSelected_Xcore = 0.;
	fTreeSelected_Ycore = 0.;
	fTreeSelected_MSCW = 0.;
	fTreeSelected_MSCL = 0.;
	fTreeSelected_MWR = 0.;
	fTreeSelected_MLR = 0.;
	fTreeSelected_Erec = 0.;
	fTreeSelected_EChi2 = 0.;
	fTreeSelected_ErecS = 0.;
	fTreeSelected_EChi2S = 0.;
	fTreeSelected_EmissionHeight = 0.;
	fTreeSelected_EmissionHeightChi2 = 0.;
	fTreeSelected_SizeSecondMax = 0.;
	fTreeSelected_MVA = -99.;
	fTreeSelected_IsGamma = 0;
}

void VStereoAnalysis::fill_TreeWithSelectedEvents( CData* c, double i_xderot, double i_yderot, double i_theta2 )
{
	if( !c )
	{
		return;
	}

	fTreeSelected_runNumber = c->runNumber;
	fTreeSelected_eventNumber = c->eventNumber;
	fTreeSelected_MJD = c->MJD;
	fTreeSelected_Time = c->Time;
	fTreeSelected_NImages = c->NImages;
	fTreeSelected_ImgSel = c->ImgSel;
	fTreeSelected_theta2 = i_theta2;
	fTreeSelected_Xoff = c->Xoff;
	fTreeSelected_Yoff = c->Yoff;
	fTreeSelected_Xoff_derot = c->Xoff_derot;
	fTreeSelected_Yoff_derot = c->Yoff_derot;
	fTreeSelected_Xcore = c->Xcore;
	fTreeSelected_Ycore = c->Ycore;
	fTreeSelected_MSCW = c->MSCW;
	fTreeSelected_MSCL = c->MSCL;
	fTreeSelected_MWR = c->MWR;
	fTreeSelected_MLR = c->MLR;
	fTreeSelected_Erec = c->Erec;
	fTreeSelected_EChi2 = c->EChi2;
	fTreeSelected_ErecS = c->ErecS;
	fTreeSelected_EChi2S = c->EChi2S;
	fTreeSelected_EmissionHeight = c->EmissionHeight;
	fTreeSelected_EmissionHeightChi2 = c->EmissionHeightChi2;
	fTreeSelected_SizeSecondMax = c->SizeSecondMax;
	if( fCuts )
	{
		fTreeSelected_MVA = fCuts->getTMVA_EvaluationResult();
	}
	else
	{
		fTreeSelected_MVA = -99.;
	}

	if( bIsGamma )
	{
		fTreeSelected_IsGamma = 1;
	}
	else
	{
		fTreeSelected_IsGamma = 0;
	}

	if( fTreeSelectedEvents )
	{
		fTreeSelectedEvents->Fill();
	}

}

void VStereoAnalysis::getDerotatedCoordinates( unsigned int icounter,  double i_UTC, double x, double y, double& x_derot, double& y_derot )
{
	if( icounter >= fAstro.size() || !fAstro[icounter] )
	{
		return;
	}

	// (!!!! Y coordinate reflected in eventdisplay for version < v.3.43 !!!!)
	// ( don't change signs if you don't know why! )
	fAstro[icounter]->derotateCoords( i_UTC, x, -1.*y, x_derot, y_derot );
	y_derot *= -1.;

	VSkyCoordinatesUtilities::convert_derotatedCoordinates_to_J2000( i_UTC, 
            fRunPara->fRunList[icounter].fTargetRAJ2000,
			fRunPara->fRunList[icounter].fTargetDecJ2000,
			x_derot, y_derot );
}

double VStereoAnalysis::getWobbleNorth()
{
	if( fRunPara && fHisCounter >= 0 && fHisCounter < ( int )fRunPara->fRunList.size() )
	{
		return fRunPara->fRunList[fHisCounter].fWobbleNorthMod;
	}

	return 0.;
}

double VStereoAnalysis::getWobbleWest()
{
	if( fRunPara && fHisCounter >= 0 && fHisCounter < ( int )fRunPara->fRunList.size() )
	{
		return fRunPara->fRunList[fHisCounter].fWobbleWestMod;
	}

	return 0.;
}

/*
 * initialize event tree for DL3
*/
bool VStereoAnalysis::init_DL3Tree( int irun, int icounter )
{
	if( fDL3EventTree )
	{
		delete fDL3EventTree;
	}
	if( !fRunPara )
	{
		return false;
	}

	char htitle[200];
	sprintf( htitle, "DL3 event list for run %d", irun );
	fDL3EventTree = new TTree( "DL3EventTree", htitle );

	fDL3EventTree->Branch( "runNumber",      &fDL3EventTree_runNumber,      "runNumber/I" );
	fDL3EventTree->Branch( "eventNumber",    &fDL3EventTree_eventNumber,    "eventNumber/I" );
	fDL3EventTree->Branch( "timeOfDay",      &fDL3EventTree_Time,           "timeOfDay/D" );
	fDL3EventTree->Branch( "MJD",            &fDL3EventTree_MJD,            "MJD/I" );
	fDL3EventTree->Branch( "Energy",         &fDL3EventTree_Erec,           "Erec/D" );
	fDL3EventTree->Branch( "Energy_Err",     &fDL3EventTree_Erec_Err,       "Erec_Err/D" );
	fDL3EventTree->Branch( "XCore",          &fDL3EventTree_XGroundCore,    "XCore/D" );
	fDL3EventTree->Branch( "YCore",          &fDL3EventTree_YGroundCore,    "YCore/D" );
	fDL3EventTree->Branch( "Xderot",         &fDL3EventTree_Xderot,         "Xderot/D" );
	fDL3EventTree->Branch( "Yderot",         &fDL3EventTree_Yderot,         "Yderot/D" );
	fDL3EventTree->Branch( "NImages",        &fDL3EventTree_NImages,        "NImages/I" );
	fDL3EventTree->Branch( "ImgSel",         &fDL3EventTree_ImgSel,         "ImgSel/l" );
    fDL3EventTree->Branch( "MeanPedvar",     &fDL3EventTree_MeanPedvar,     "MeanPedvar/D" );
	fDL3EventTree->Branch( "MSCW",           &fDL3EventTree_MSCW,           "MSCW/D" );
	fDL3EventTree->Branch( "MSCL",           &fDL3EventTree_MSCL,           "MSCL/D" );
	fDL3EventTree->Branch( "RA"            , &fDL3EventTree_RA,             "RA/D" );
	fDL3EventTree->Branch( "DEC"           , &fDL3EventTree_DEC,            "DEC/D" );
	fDL3EventTree->Branch( "Az"            , &fDL3EventTree_Az,             "Az/D" );
	fDL3EventTree->Branch( "El"            , &fDL3EventTree_El,             "El/D" );
	fDL3EventTree->Branch( "EmissionHeight", &fDL3EventTree_EmissionHeight, "EmissionHeight/D" );
	fDL3EventTree->Branch( "Xoff"          , &fDL3EventTree_Xoff          , "Xoff/D" );
	fDL3EventTree->Branch( "Yoff"          , &fDL3EventTree_Yoff          , "Yoff/D" );
	fDL3EventTree->Branch( "Acceptance"    , &fDL3EventTree_Acceptance    , "Acceptance/D" );
    if ( fRunPara->fWriteAllEvents )
    {
        fDL3EventTree->Branch( "MVA"       , &fDL3EventTree_MVA,            "MVA/D" );
        fDL3EventTree->Branch( "IsGamma"   , &fDL3EventTree_IsGamma,        "IsGamma/I" );
    }
	cout << endl;

	// init radial acceptance class
    if( icounter < (int)fRunPara->fRunList.size() )
    {
        fDL3_Acceptance = new VRadialAcceptance( fRunPara->fRunList[icounter].fAcceptanceFile ) ;
        fDL3_Acceptance->Set2DAcceptanceMode( fRunPara->fRunList[icounter].f2DAcceptanceMode ) ;
    }
    else
    {
        fDL3_Acceptance = 0;
    }
	return true;
}

/*
 *  fill a new event into the DL3 tree
 */
void VStereoAnalysis::fill_DL3Tree( CData* c , double i_xderot, double i_yderot, unsigned int icounter, double i_UTC )
{
	if( !c )
	{
		return;
	}

	fDL3EventTree_runNumber      = c->runNumber;
	fDL3EventTree_eventNumber    = c->eventNumber;
	fDL3EventTree_Time           = c->Time;         // Time of day (seconds) of gamma ray event
	fDL3EventTree_MJD            = c->MJD;
	fDL3EventTree_Xoff           = c->Xoff;         // Gamma Point-Of-Origin, in camera coodinates (deg)
	fDL3EventTree_Yoff           = c->Yoff;         // Gamma Point-Of-Origin, in camera coodinates (deg)
	fDL3EventTree_Xderot         = i_xderot;        // Derotated Gamma Point-Of-Origin (deg, RA)
	fDL3EventTree_Yderot         = i_yderot;        // Derotated Gamma Point-Of-Origin (deg, DEC)
    if( fCuts )
    {
       fDL3EventTree_Erec = fCuts->getReconstructedEnergy( fRunPara->fEnergyReconstructionMethod );
       fDL3EventTree_Erec_Err = fCuts->getReconstructedEnergydE( fRunPara->fEnergyReconstructionMethod ); // Reconstructed Gamma Energy (TeV) Error
    }
    else
    {
       fDL3EventTree_Erec       = 0.;
       fDL3EventTree_Erec_Err   = 0.;
    }
	fDL3EventTree_XGroundCore    = c->Xcore;        // Gamma Ray Core-Ground intersection location (north)
	fDL3EventTree_YGroundCore    = c->Ycore;        // Gamma Ray Core-Ground intersection location (east)
	fDL3EventTree_NImages        = c->NImages;      // Number of images used in reconstruction
	fDL3EventTree_ImgSel         = c->ImgSel;       // binary code describing which telescopes had images
    fDL3EventTree_MeanPedvar     = c->meanPedvar_Image; // average pedvar
	fDL3EventTree_MSCW           = c->MSCW;        // mean scaled width
	fDL3EventTree_MSCL           = c->MSCL;        // mean scaled length
	fDL3EventTree_EmissionHeight = c->EmissionHeight ; // height of shower maximum (in km) above telescope z-plane
    if( fDL3_Acceptance )
    {
        fDL3EventTree_Acceptance     = fDL3_Acceptance->getAcceptance( c->Xoff, c->Yoff );
    }
    else
    {
        fDL3EventTree_Acceptance     = 0.;
    }
    // get event ra and dec
    if( icounter < fRunPara->fRunList.size() )
    {
        double i_Spherical_RA  = 0.;
        double i_Spherical_DEC = 0.;
        VAstronometry::vlaDtp2s( fDL3EventTree_Xderot * TMath::DegToRad(),
                  fDL3EventTree_Yderot * TMath::DegToRad(),
                  fRunPara->fRunList[icounter].fArrayPointingRAJ2000 * TMath::DegToRad(), 
                  fRunPara->fRunList[icounter].fArrayPointingDecJ2000 * TMath::DegToRad(),
                  &i_Spherical_RA, &i_Spherical_DEC);
        fDL3EventTree_RA  = i_Spherical_RA * TMath::RadToDeg();
        fDL3EventTree_DEC = i_Spherical_DEC * TMath::RadToDeg();

        // Convert from spherical RA and DEC to Azimuth and Zenith
        // convert to degrees and do calculation
        fVsky->setTargetJ2000( i_Spherical_DEC * TMath::RadToDeg(), i_Spherical_RA * TMath::RadToDeg() );
        fVsky->precessTarget( fDL3EventTree_MJD, 0 ) ;

        // calculate new param
        fVsky->updatePointing( fDL3EventTree_MJD, fDL3EventTree_Time ) ;
        fDL3EventTree_Az = fVsky->getTargetAzimuth();
        fDL3EventTree_El = fVsky->getTargetElevation();

    }
    else
    {
        fDL3EventTree_RA = 0.;
        fDL3EventTree_DEC = 0.;
        fDL3EventTree_Az = 0.;
        fDL3EventTree_El = 0.;
    }
    if ( fCuts && fRunPara->fWriteAllEvents )
    {
        fDL3EventTree_MVA = fCuts->getTMVA_EvaluationResult();
        if ( bIsGamma )
        {
            fDL3EventTree_IsGamma = 1;
        }
        else
        {
            fDL3EventTree_IsGamma = 0;
        }
    }

	if( fDL3EventTree )
	{
		fDL3EventTree->Fill();
	}
}

/*
 * write tree with events for DL3 export
 *
*/
void VStereoAnalysis::write_DL3Tree()
{
	fDL3EventTree->Write();

	fRunPara->SetName( "VAnaSumRunParameter" );
	fRunPara->Write() ;

    // cleanup
    if( fDL3_Acceptance )
    {
        delete fDL3_Acceptance;
    }
}
