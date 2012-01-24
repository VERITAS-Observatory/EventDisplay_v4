/*! \class VStereoAnalysis
  \brief class for producing histograms from parameterized stereo VERITAS data
  \author
  Jamie Holder 
  Gernot Maier

  Revision $Id: VStereoAnalysis.cpp,v 1.47.2.9.4.9.10.2.2.5.4.3.2.10.2.14.2.1.4.1.2.8.2.7.2.3.2.4 2011/03/31 14:50:58 gmaier Exp $
*/

#include "VStereoAnalysis.h"

VStereoAnalysis::VStereoAnalysis( bool ion, string i_hsuffix, VAnaSumRunParameter *irunpara, vector< TDirectory* > iDirRun, 
                                  TDirectory *iDirTot, string iDataDir, int iRandomSeed, bool iTotalAnalysisOnly )
{
      fDebug = false;

      fDataFile = 0;
      fDirTot = iDirTot;
      fDirTotRun = iDirRun;
      bTotalAnalysisOnly = iTotalAnalysisOnly;

      fMeanSourceDec = 0.;
      fMeanSourceRA = 0.;

// simulate background and source, overwrites data!
      bSimulate = false;
      if( bSimulate )
      {
      	cout << "***********************************" << endl;
      	cout << "  SIMULATED SOURCE AND BACKGROUND  " << endl;
      	cout << "***********************************" << endl;
      }

// set default values
      fIsOn = ion;
// do full sky plots
      fNoSkyPlots = false;

      gMeanEffectiveAreaEmc = 0;
      gMeanEffectiveAreaErec = 0;
      gTimeBinnedMeanEffectiveAreaEmc = 0;
      gTimeBinnedMeanEffectiveAreaErec = 0;
      gMeanEsys_MC = 0;

      fHisCounter = 0;
      fTotCount = 0.;

      fMeanAzimuth = 0.;
      fMeanElevation = 0.;
      fNMeanElevation = 0.;

      fTreeSelectedEvents = 0;

      fRunPara = irunpara;

      fWobbleNorth = 0.;
      fWobbleWest = 0.;

// calculating run start, end and duration (verifies data trees)
      if( !bTotalAnalysisOnly ) setRunTimes();

// TIMEEFF GM: what happens if this element does not exist?
      double f_t_tot_min= f_t_in_s_min[fIsOn ? fRunPara->fRunList[0].fRunOn : fRunPara->fRunList[0].fRunOff];
      double f_t_tot_max= f_t_in_s_max[fIsOn ? fRunPara->fRunList[fRunPara->fRunList.size()-1].fRunOn : fRunPara->fRunList[fRunPara->fRunList.size()-1].fRunOff];

// targets and exclusion regions
      if( !bTotalAnalysisOnly ) defineAstroSource();

///////////////////////////////
// define histograms

// combined results
      iDirTot->cd();
      fHistoTot = new VStereoHistograms( i_hsuffix, fRunPara->fSkyMapBinSize, fRunPara->fSkyMapBinSizeUC, 
                                         fRunPara->fEnergySpectrumBinSize, fRunPara->fTimeIntervall, f_t_tot_min ,f_t_tot_max, fIsOn );
      fHistoTot->setSkyMapSize( fRunPara->fSkyMapSizeXmin, fRunPara->fSkyMapSizeXmax, fRunPara->fSkyMapSizeYmin, fRunPara->fSkyMapSizeYmax );

// one set of histograms for each run
      if( iDirRun.size() != fRunPara->fRunList.size() )
      {
      	cout << "VStereoAnalysis::VStereoAnalysis fatal error, directory and run list different ";
      	cout << iDirRun.size() << "\t" << fRunPara->fRunList.size() << endl;
      	exit( -1 );
      }
      vector< double > i_v;
      for( unsigned int i = 0; i < fRunPara->fRunList.size(); i++ )
      {
      	iDirRun[i]->cd();
      	if( fIsOn )
      	{
      	   fHisto.push_back( new VStereoHistograms( i_hsuffix, fRunPara->fSkyMapBinSize, fRunPara->fSkyMapBinSizeUC, 
	                                            fRunPara->fEnergySpectrumBinSize, fRunPara->fTimeIntervall,
						    f_t_in_s_min[fIsOn ? fRunPara->fRunList[i].fRunOn : fRunPara->fRunList[i].fRunOff],
						    f_t_in_s_max[fIsOn ? fRunPara->fRunList[i].fRunOn : fRunPara->fRunList[i].fRunOff], fIsOn ) );
           fHisto.back()->setSkyMapSize( fRunPara->fSkyMapSizeXmin, fRunPara->fSkyMapSizeXmax, fRunPara->fSkyMapSizeYmin, fRunPara->fSkyMapSizeYmax );
      	}
      	else
      	{
	   fHisto.push_back( new VStereoHistograms( i_hsuffix, fRunPara->fSkyMapBinSize, fRunPara->fSkyMapBinSizeUC,
	                                            fRunPara->fEnergySpectrumBinSize, fRunPara->fTimeIntervall,
						    f_t_in_s_min[fIsOn ? fRunPara->fRunList[i].fRunOn : fRunPara->fRunList[i].fRunOff],
						    f_t_in_s_max[fIsOn ? fRunPara->fRunList[i].fRunOn : fRunPara->fRunList[i].fRunOff], fIsOn  ) );
	   fHisto.back()->setSkyMapSize( fRunPara->fSkyMapSizeXmin, fRunPara->fSkyMapSizeXmax, fRunPara->fSkyMapSizeYmin, fRunPara->fSkyMapSizeYmax );
      	}
// define dead time calculators
      	fDeadTime.push_back( new VDeadTime( fIsOn ) );

// rate plots
      	fRateCounts.push_back( i_v );
      	fRateTime.push_back( i_v );
      	fRateTimeIntervall.push_back( i_v );

      }                                             //fRunList.size()

// define the time mask
      fTimeMask = new VTimeMask();

//! define the cuts
      fCuts = new VGammaHadronCuts();
      fCuts->resetCutValues();
      fCuts->setDataTree( 0 );
      fCuts->setDataDirectory( iDataDir );

// define the background model
      fMap   = new VStereoMaps( false, iRandomSeed );
      fMapUC = new VStereoMaps( true,  iRandomSeed );
}


/*!
 *
 * establish run times (mean, start, end and duration) for a list of runs
 *
 */
void VStereoAnalysis::setRunTimes()
{
    cout << endl <<"-----------------------------------------------------------------------"<< endl;
    cout << "Checking data trees " << ( fIsOn ? "(ON runs)" : "(OFF runs)" ) << endl;
    cout << "\t Run \t| Start (MJD : secs)\t| End (MJD : secs)\t| Duration (secs [mins])"<< endl;

    for( unsigned int i = 0; i < fRunPara->fRunList.size(); i++ )
    {
      	int i_run = fIsOn ? fRunPara->fRunList[i].fRunOn : fRunPara->fRunList[i].fRunOff;

      	CData *c = getDataFromFile( i_run );

      	cout << setRunTimes( c ) << endl;
      	if ( fIsOn )  fRunPara->fRunList[i].fMJDOn = getMJD( i_run );
      	else          fRunPara->fRunList[i].fMJDOff = getMJD( i_run );

      	closeDataFile();
    }
}


/*!
 *
 * establish run times (mean, start, end and duration) of a single run
 *
 */
string  VStereoAnalysis::setRunTimes( CData *iData )
{
      ostringstream ires("");
      ires.setf( ios_base::fixed, ios_base::floatfield );

      double i_min, i_minMJD, i_minUTC = 0.;
      double i_max, i_maxMJD, i_maxUTC = 0.;
      double i_dur=0.;

      fDataRun = iData;
      int i_run = getDataRunNumber();

      fDataRun->GetEntry(1);
      i_min = fDataRun->Time;
      f_t_in_s_min[i_run]=i_min;
      i_minMJD = fDataRun->MJD;
      i_minUTC = VSkyCoordinatesUtilities::getUTC( (int)i_minMJD, i_min );

      int i_nentries = (int)fDataRun->fChain->GetEntries() - 2;
      fDataRun->GetEntry(i_nentries);
      i_max = fDataRun->Time;
      f_t_in_s_max[i_run]=i_max;
      i_maxMJD = fDataRun->MJD;
      i_maxUTC = VSkyCoordinatesUtilities::getUTC( (int)i_maxMJD, i_max );

      i_dur = ( i_maxUTC - i_minUTC )*24*60*60;

      fRunMJDStart[i_run] = i_minUTC;
      fRunMJDStopp[i_run] = i_maxUTC;
      fRunMJD[i_run] = (i_maxUTC+i_minUTC)/2.;
      fRunDuration[i_run] = i_dur;

      ires.precision( 0 );
      ires <<"\t "<< i_run <<"\t| ";

      ires <<  i_minMJD;
      ires.precision( 2 );
      ires <<" : "<< i_min;

      ires.precision( 0 );
      ires <<"\t| " << i_maxMJD;
      ires.precision( 2 );
      ires << " : "<< i_max;

      ires <<"\t| " << i_dur  << " ["<< i_dur/60. << "]";

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
      	cout<<"VStereoAnalysis::getDataRunNumber error: no data tree."<<endl;
   }
   else
   {
     if ( fDataRun->GetEntry( 0 ) == -1 )         cout <<"VStereoAnalysis::getDataRunNumber error: can't seem to access tree." << endl;
     else if ( fDataRun->GetEntry( 0 ) == 0 )
     {
	  cout <<"VStereoAnalysis::getDataRunNumber error: tree is empty."<< endl;
	  fDataRun->fChain->Print();
     }
     else 
     {
	return fDataRun->runNumber;
     }
   }

   exit( -1 );
   return 0;
}


/*! 
 *
 *  fill all histograms and maps
 *  (main loop)
 *
 *  int irun  runnumber to be analyzed, set irun = -1 to analyze all runs in data chain
 *
 *  return number of events passing all cuts
 *
 */
double VStereoAnalysis::fillHistograms( int icounter, int irun, double iAzMin, double iAzMax, double iPedVar )
{
    if( fDebug ) cout << "DEBUG double VStereoAnalysis::fillHistograms() "  << icounter << "\t" << irun << endl;

    fHisCounter = icounter;

////////////////////////////////////////////////
// combine all histograms from all runs
    if( irun < 0 )
    {
      	fHisCounter = -1;
      	return combineHistograms();
    }
////////////////////////////////////////////////

////////////////////////////////////////////////
// analyze individual run
    if( fIsOn )  cout << endl << "Filling ON histograms for run " << irun <<" -----------------------------"<< endl;
    else         cout << endl << "Filling OFF histograms for run " << irun <<" -----------------------------"<< endl;

// set pointer to data tree (run wise)
    fDataRun = getDataFromFile( irun );
    if( fDataRun == 0 || fDataRunTree == 0 )
    {
      	cout << "VStereoAnalysis::fillHistograms error, no data tree " << endl;
      	cout << "\t" << fDataRun << "\t" << fDataRunTree << endl;
      	cout << "exiting..." << endl;
      	exit( -1 );
    }
    if( fHisCounter > (int)fHisto.size() )
    {
      	cout << "VStereoAnalysis::fillHistograms invalid run number " << irun << "\t" << fHisCounter << "\t" << fHisto.size() << endl;
      	exit( -1 );
    }

    double iMJDStart = 0.;
    double iMJDStopp = 0.;
    if( getDataRunNumber() != irun )
    {
      	cout <<"VStereoAnalysis::fillHistograms warning: given run ("<< irun <<") does not match run of given tree ("<< getDataRunNumber() <<")"<< endl;
    }
    else
    {
      	if( fRunMJDStart.find( irun ) == fRunMJDStart.end() ) setRunTimes( fDataRun );
      	iMJDStart = fRunMJDStart[irun];
      	if( fRunMJDStopp.find( irun ) == fRunMJDStopp.end() ) setRunTimes( fDataRun );
      	iMJDStopp = fRunMJDStopp[irun];
    }
//////////////////////////////////////////
// boolean for gamma/hadron cuts

// event is gamma-ray like according to VGammaHadronCuts
   bool bIsGamma = false;
// event direction is inside search region (e.g. reflected region)
   bool bDirectionCuts = false;

// rate vectors
   vector< double > iRateCounts;
   vector< double > iRateTime;
   vector< double > iRateTimeIntervall;

// get effective area time bin vector
   int i_t_bins = int((f_t_in_s_max[irun]-f_t_in_s_min[irun])/fRunPara->fTimeIntervall + 0.5);
// TIMEEFF GM: is i_t_bins always > 0?
   double i_time_intervall=(f_t_in_s_max[irun]-f_t_in_s_min[irun])/((double)i_t_bins);

   double iEffAreaTimeBin[i_t_bins+1];
   for( int i=0; i<i_t_bins+1; i++)
   {
      iEffAreaTimeBin[i]=f_t_in_s_min[irun]+i*i_time_intervall;
   }

// initialize time mask
   fTimeMask->setMask( irun, iMJDStart, iMJDStopp, fRunPara->fTimeMaskFile );

// initialize cuts
   setCuts( fRunPara->fRunList[fHisCounter], irun );

// define histograms
   fDirTotRun[fHisCounter]->cd();
   fHisto[fHisCounter]->defineHistograms();

// adjust dead time calculator
   TDirectoryFile *iDeadtimeDirectory = 0;
   if( fDataFile ) iDeadtimeDirectory = (TDirectoryFile*)fDataFile->Get( "deadTimeHistograms" );
   if( iDeadtimeDirectory ) fDeadTime[fHisCounter]->readHistograms( iDeadtimeDirectory );
   else                     fDeadTime[fHisCounter]->defineHistograms();

// adjust axis in rate histograms
   fHisto[fHisCounter]->makeRateHistograms( iMJDStart, iMJDStopp );

// set map properties
   fMap->setData( fDataRun );
   fMap->setTargetShift( fRunPara->fRunList[fHisCounter].fTargetShiftWest, fRunPara->fRunList[fHisCounter].fTargetShiftNorth );
   fMap->setRegionToExclude(fRunPara->fExcludeFromBackground_West, fRunPara->fExcludeFromBackground_North,fRunPara->fExcludeFromBackground_Radius );
   fMap->setNoSkyPlots( fNoSkyPlots );
   fMap->setRunList( fRunPara->fRunList[fHisCounter] );
   fMap->setHistograms( fHisto[fHisCounter]->hmap_stereo, fHisto[fHisCounter]->hmap_alpha );

   fMapUC->setData( fDataRun );
   fMapUC->setTargetShift( fRunPara->fRunList[fHisCounter].fTargetShiftWest, fRunPara->fRunList[fHisCounter].fTargetShiftNorth );
   fMapUC->setRegionToExclude(fRunPara->fExcludeFromBackground_West, fRunPara->fExcludeFromBackground_North,fRunPara->fExcludeFromBackground_Radius );
   fMapUC->setNoSkyPlots( fNoSkyPlots );
   fMapUC->setRunList( fRunPara->fRunList[fHisCounter] );
   fMapUC->setHistograms( fHisto[fHisCounter]->hmap_stereoUC, fHisto[fHisCounter]->hmap_alphaUC );

// initialize gamma/hadron cuts
   fCuts->setDataTree( fDataRun );

// tree with selected events
   init_TreeWithSelectedEvents( irun, fIsOn );

// wobble offsets
   fWobbleNorth = fRunPara->fRunList[fHisCounter].fWobbleNorthMod;
   fWobbleWest  = fRunPara->fRunList[fHisCounter].fWobbleWestMod;

// spectral energy reconstruction (effective areas, etc.)
// effective area class
   VEffectiveAreaCalculator fEnergy( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, iAzMin, iAzMax, iPedVar,
				     fRunPara->fEnergyReconstructionSpectralIndex, fRunPara->fMCZe,
       				     fRunPara->fEnergyEffectiveAreaSmoothingIterations, 
      				     fRunPara->fEnergyEffectiveAreaSmoothingThreshold, fRunPara->bEffectiveAreaVsEnergyMC );
   double iEnergyWeighting = 1.;
   double iErec = 0.;
   double iErecChi2 = 0.;
   double iPedVar_temp = 0.;
// for time-check save old-time and new-time
   double oldtime = 0.;
   double newtime = 0.;

   double i_UTC = 0.;
   double i_xderot = 0.;
   double i_yderot = 0.;

// mean direction values
   fMeanAzimuth = 0.;
   fMeanElevation = 0.;
   fNMeanElevation = 0.;
   double iDirectionOffset = 0.;

// sign change for eventdisplay v.343 and higher
// (do not change until you really know what it means)
   double fEVDVersionSign = 1.;
   if( fRunPara->fRunList[fHisCounter].fEventDisplayVersion.size() > 0 )
   {
     float i_Version = atof( fRunPara->fRunList[fHisCounter].fEventDisplayVersion.substr( 2, 
			     fRunPara->fRunList[fHisCounter].fEventDisplayVersion.size() ).c_str() );
     if( i_Version > 3.42 ) fEVDVersionSign = -1.;
   }

// get number of entries from data tree
   Int_t nentries = Int_t(fDataRun->fChain->GetEntries());
   if( fDebug ) cout << "DEBUG double VStereoAnalysis::fillHistograms() reading chain " << fDataRun->fChain->GetName() << "\t" << nentries << endl;
   cout << "\t number of entries for this run: " << nentries << endl;

   double i_count = 0.;
   int nentries_run = 0;

/////////////////////////////////////////////////////////////////////
// loop over all entries in the data tree
   for( int i = 0; i < nentries; i++ )
   {
      	fDataRun->GetEntry( i );

      	if( fDataRun->runNumber == irun )
      	{
// count how many entries are in this run
	     nentries_run++;

// UTC time
	     i_UTC = VSkyCoordinatesUtilities::getUTC( fDataRun->MJD,fDataRun->Time );

// phase cuts - this is also a time cut that adds to the previously initialized mask
	     if( !fCuts->applyPhaseCut(i) ) 
	     {
		  fTimeMask->setMaskDuringPhaseCuts( i_UTC);
		  continue;
	     }
// make time cut
	     if ( !fTimeMask->checkAgainstMask( i_UTC ) ) continue;

// fill rate histograms
	     fHisto[fHisCounter]->hrate_1sec->Fill(  i_UTC );
	     fHisto[fHisCounter]->hrate_10sec->Fill( i_UTC );
	     fHisto[fHisCounter]->hrate_1min->Fill(  i_UTC );

// dead time calculation
	     if( !iDeadtimeDirectory ) fDeadTime[fHisCounter]->fillDeadTime( fDataRun->Time );

// get energy (depending on energy reconstruction method)
	     if( fRunPara->fFrogs == 1 )
	     {
	       iErec = fDataRun->frogsEnergy;
	       iErecChi2 = 0.0;
	     } 
	     else
	     {
		if( fRunPara->fEnergyReconstructionMethod == 0 )
		{
		    iErec = fDataRun->Erec;
		    iErecChi2 = fDataRun->EChi2;
		}
		else if( fRunPara->fEnergyReconstructionMethod == 1 )
		{
		    iErec = fDataRun->ErecS;
		    iErecChi2 = fDataRun->EChi2S;
		}
	     }

////////////////////////////////////////////////
// apply all quality cuts
//
// check if event is outside fiducial area
	     if( !fCuts->applyInsideFiducialAreaCut() ) continue;

// stereo quality cuts (e.g. successful direction, mscw, mscl reconstruction)

//	     if ( fRunPara->fFrogs != 1 )
//	     {
		if( !fCuts->applyStereoQualityCuts( fRunPara->fEnergyReconstructionMethod, false, i , fIsOn) ) continue;
//	     }

// fill image and trigger pattern histograms
	     fHisto[fHisCounter]->hTriggerPatternBeforeCuts->Fill( fDataRun->LTrig  );
	     fHisto[fHisCounter]->hImagePatternBeforeCuts->Fill(   fDataRun->ImgSel );

// direction offset
	     iDirectionOffset = sqrt( getXoff()*getXoff() + getYoff()*getYoff() );

// derotate coordinates
// (!!!! Y coordinate reflected in eventdisplay for version < v.3.43 !!!!)
// ( don't change signs if you don't know why! )
	     fAstro[icounter]->derotateCoords(i_UTC, getXoff(), fEVDVersionSign * getYoff(), i_xderot, i_yderot);
             i_yderot *= -1.;

// gamma/hadron separation cuts
	     bIsGamma = fCuts->isGamma( i,false, fIsOn);

// fill on/offstereo maps
	     bDirectionCuts = fMap->fill(   fIsOn, i_xderot, i_yderot, fDataRun->Ze, iErec, fDataRun->runNumber, bIsGamma );
	     bDirectionCuts = fMapUC->fill( fIsOn, i_xderot, i_yderot, fDataRun->Ze, iErec, fDataRun->runNumber, bIsGamma );

// following histograms (theta2, mscw, mscl, core position, etc.)  assume source at given target position

// theta2 ---
	     fMap->calculateTheta2( fIsOn, i_xderot, i_yderot );
// theta2 values for debugging
	     for(unsigned int dex = 0; dex < 25; dex++)
	     {
		  fDataRun->theta2_All[dex] = fMap->getTheta2_All()[dex];
	     }

	     for( unsigned int t = 0; t < fMap->getTheta2_length(); t++ )
	     {
		fHisto[fHisCounter]->htheta2->Fill( fMap->getTheta2()[t], fMap->getTheta2_weigth()[t] );
             }
// fill theta for tree with selected events
	     if( fMap->getTheta2_length() > 0 ) fDataRun->theta2 = fMap->getTheta2()[0];
	     else                               fDataRun->theta2 = -1;

/////////////////////////////////////////////////////////
// histograms after shape (or other gamma/hadron separation cuts) cuts only
	     if( bIsGamma )
	     {
	       fHisto[fHisCounter]->hxyoff_stereo->Fill( getXoff(), getYoff() );
	     }

/////////////////////////////////////////////////////////
// histograms after direction cuts only
//
	     if( bDirectionCuts )
	     {
// mean width/length/distance histograms
	        fHisto[fHisCounter]->hmean_width->Fill(  fCuts->getMeanWidth() );
	        fHisto[fHisCounter]->hmean_length->Fill( fCuts->getMeanLength() );
	        fHisto[fHisCounter]->hmean_dist->Fill(   fCuts->getMeanDistance() );
	        if( fDataRun->MSCW > -50. ) fHisto[fHisCounter]->hmscw->Fill( fDataRun->MSCW );
	        if( fDataRun->MSCL > -50. ) fHisto[fHisCounter]->hmscl->Fill( fDataRun->MSCL );
	        if( fDataRun->MSCW > -50. && fDataRun->MSCL > -50. ) fHisto[fHisCounter]->hmsc->Fill( fDataRun->MSCW, fDataRun->MSCL );
// probability threshold cuts
		if( fCuts->getProbabilityCut_Selector() > 0. ) fHisto[fHisCounter]->hrf->Fill( fCuts->getProbabilityCut_Selector() );
// mean emission height histograms
		if( fDataRun->EmissionHeight > 0. )
		{
		   fHisto[fHisCounter]->hemiss->Fill( fDataRun->EmissionHeight );
		   fHisto[fHisCounter]->hemissC2->Fill( fDataRun->EmissionHeightChi2 );
		} 
// chi2 of energy reconstruction
		if( iErecChi2 > 0. ) fHisto[fHisCounter]->herecChi2->Fill( iErecChi2 );
// fill a tree with the selected events (after direction cut only)
		fill_TreeWithSelectedEvents( fDataRun );
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
		fHisto[fHisCounter]->hcore->Fill(getXcore(),getYcore());
// apply energy reconstruction cuts
	        if( fCuts->applyEnergyReconstructionQualityCuts( fRunPara->fEnergyReconstructionMethod ) )
	        {
// raw energies
		    fHisto[fHisCounter]->herecCounts->Fill( log10( iErec ) );
		    fHisto[fHisCounter]->herecCounts2DtimeBinned->Fill( log10( iErec ), ((double)fDataRun->Time - f_t_in_s_min[irun]));
		    fHisto[fHisCounter]->hLinerecCounts->Fill( iErec );
		    fHisto[fHisCounter]->hLinerecCounts2DtimeBinned->Fill( iErec , ((double)fDataRun->Time - f_t_in_s_min[irun]));
		    fHisto[fHisCounter]->herecRaw->Fill( log10( iErec ) );
		    fHisto[fHisCounter]->herecRaw2DtimeBinned->Fill( log10( iErec ), ((double)fDataRun->Time - f_t_in_s_min[irun]));
		    fHisto[fHisCounter]->hLinerecRaw->Fill( iErec );
		    fHisto[fHisCounter]->hLinerecRaw2DtimeBinned->Fill( iErec , ((double)fDataRun->Time - f_t_in_s_min[irun]));

// write the Effective Area to disk for each Time BIN
// Therefore if ((double)fDataRun->Time crosses the binning limit, start new EffectiveArea in Time BIN - reset and get Mean
		    oldtime = newtime;
		    newtime = ((double)fDataRun->Time);
      					 
		    for( int bini=1; bini<i_t_bins+1; bini++)
		    {
		       if( oldtime < iEffAreaTimeBin[bini])
		       {
			  if( newtime >= iEffAreaTimeBin[bini])
			  {
// get mean effective area in Time BIN
			      fEnergy.setTimeBin(iEffAreaTimeBin[bini]-i_time_intervall/2-f_t_in_s_min[irun]);
			      fEnergy.setTimeBinnedMeanEffectiveAreaEMC();            
			      fEnergy.setTimeBinnedMeanEffectiveAreaErec();
			      fEnergy.resetTimeBin();
			   }
		        }
   		    }
// get 1 / effective area
// effective areas depend on ze, wobble offset, pedestal variation, etc
		    if( fDataRun->meanPedvar_Image > 0. ) iPedVar_temp = fDataRun->meanPedvar_Image;
		    else                                  iPedVar_temp = iPedVar;

		    iEnergyWeighting = fEnergy.getEffectiveArea( iErec, fDataRun->Ze, 
								 iDirectionOffset, iPedVar_temp, 
								 fRunPara->fEnergyReconstructionSpectralIndex, true, 
								 fRunPara->bEffectiveAreaVsEnergyMC );
// fill energy histograms
		    fHisto[fHisCounter]->herec->Fill( log10( iErec ), iEnergyWeighting );
		    fHisto[fHisCounter]->hLinerec->Fill( iErec, iEnergyWeighting );
		    fHisto[fHisCounter]->hLinerec2DtimeBinned->Fill( iErec, ((double)fDataRun->Time - f_t_in_s_min[irun]), iEnergyWeighting );
		    fHisto[fHisCounter]->herec2DtimeBinned->Fill( log10( iErec ), ((double)fDataRun->Time - f_t_in_s_min[irun]), iEnergyWeighting);
		    if( iEnergyWeighting > 0. )
		    {
			 fHisto[fHisCounter]->herecWeights->Fill( log10( iErec ), log10( 1./iEnergyWeighting ) );
			 fHisto[fHisCounter]->hLinerecWeights->Fill( iErec, log10( 1./iEnergyWeighting ) );
			 fHisto[fHisCounter]->herecEffectiveArea->Fill( log10( iErec ), 1./iEnergyWeighting );
			 fHisto[fHisCounter]->hLinerecEffectiveArea->Fill( iErec, 1./iEnergyWeighting );
      		    }
      	       }
// mean azimuth and elevation (telescope orientation)
// (get first running telescope)
	       for( unsigned int t = 0; t < fRunPara->fRunList[fHisCounter].fTelToAnalyze.size(); t++ )
	       {
// for some runs LTrig is not filled!
		  if( ( (t+1) & fDataRun->LTrig ) || ( fDataRun->LTrig == 0 ) )
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

      fEnergy.setTimeBin(iEffAreaTimeBin[i_t_bins]-i_time_intervall/2-f_t_in_s_min[irun]);
      fEnergy.setTimeBinnedMeanEffectiveAreaEMC();            
      fEnergy.setTimeBinnedMeanEffectiveAreaErec();
      fEnergy.resetTimeBin();

// fill rate vectors
      fTimeMask->getIntervalRates( iRateCounts, iRateTime, iRateTimeIntervall, fRunPara->fTimeIntervall );
      fRateCounts[fHisCounter] = iRateCounts;
      fRateTime[fHisCounter] = iRateTime;
      fRateTimeIntervall[fHisCounter] = iRateTimeIntervall;

      fMap->finalize( fIsOn, fCuts->getProbabilityCutAlpha(fIsOn) );
      fMapUC->finalize( fIsOn, fCuts->getProbabilityCutAlpha(fIsOn) );

      fTotCount += i_count;

// calculate mean elevation
      if( fNMeanElevation > 0. )
      {
      	fMeanAzimuth   /= fNMeanElevation;
      	fMeanElevation /= fNMeanElevation;
      }

// get mean effective area
      if( fEnergy.getMeanEffectiveArea( true ) )
      {
          gMeanEffectiveAreaEmc = (TGraphAsymmErrors*)fEnergy.getMeanEffectiveArea( true )->Clone();
      }
      else 
      {
         gMeanEffectiveAreaEmc = 0;
      }
      if( fEnergy.getMeanEffectiveArea( false ) )
      {
          gMeanEffectiveAreaErec = (TGraphAsymmErrors*)fEnergy.getMeanEffectiveArea( false )->Clone();
      }
      else
      { 
         gMeanEffectiveAreaErec = 0;
      }
// get mean energy systematic histogram (needed possibly for energy threshold determination)
      gMeanEsys_MC = (TGraphErrors*)fEnergy.getMeanSystematicErrorHistogram();
      if( gMeanEsys_MC ) gMeanEsys_MC = (TGraphErrors*)gMeanEsys_MC->Clone();

// get mean effective area for TIME BINs
      if( fEnergy.getTimeBinnedMeanEffectiveArea( true ) )
      {
         gTimeBinnedMeanEffectiveAreaEmc = (TGraph2DErrors*)fEnergy.getTimeBinnedMeanEffectiveArea( true )->Clone();
      }
      else
      {
         gTimeBinnedMeanEffectiveAreaEmc = 0; 
      } 
      if( fEnergy.getTimeBinnedMeanEffectiveArea( false ) )
      {
         gTimeBinnedMeanEffectiveAreaErec = (TGraph2DErrors*)fEnergy.getTimeBinnedMeanEffectiveArea( false )->Clone();
      }
      else 
      { 
         gTimeBinnedMeanEffectiveAreaErec = 0; 
      } 
// get dead time
      fDeadTime[fHisCounter]->calculateDeadTime();
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
      if( fDebug ) cout << "DEBUG void VStereoAnalysis::writeHistograms()" << endl;
      if( fHisCounter < 0 )
      {
         fHistoTot->writeHistograms();
      }
      else
      {
      	fTimeMask->writeObjects();
      	fHisto[fHisCounter]->writeHistograms();
      	fDeadTime[fHisCounter]->writeHistograms();
// copy effective areas and radial acceptance to anasum output file
      	if( bOn )
      	{
	     fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gMeanEffectiveAreaEmc );
	     fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gMeanEffectiveAreaErec );
	     fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gTimeBinnedMeanEffectiveAreaEmc );
	     fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gTimeBinnedMeanEffectiveAreaErec );
	     fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gMeanEsys_MC );
	     if( fRunPara->fRunList[fHisCounter].fAcceptanceFile.size() > 0 )
	     {
		 fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fAcceptanceFile, "RadialAcceptances", 0 );
	     }
	}
      	else
      	{
	     char hname[1000];
	     if( gMeanEffectiveAreaEmc )
	     {
		     sprintf( hname, "%s_off", gMeanEffectiveAreaEmc->GetName() );
		     gMeanEffectiveAreaEmc->SetName( hname );
	     }
	     if( gTimeBinnedMeanEffectiveAreaEmc )
	     {
		     sprintf( hname, "%s_off", gTimeBinnedMeanEffectiveAreaEmc->GetName() );
		     gTimeBinnedMeanEffectiveAreaEmc->SetName( hname );
	     }
	     fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gMeanEffectiveAreaEmc );
	     fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gTimeBinnedMeanEffectiveAreaEmc );
	     if( gMeanEffectiveAreaErec )
	     {
		     sprintf( hname, "%s_off", gMeanEffectiveAreaErec->GetName() );
		     gMeanEffectiveAreaErec->SetName( hname );
	     }
	     if( gTimeBinnedMeanEffectiveAreaErec )
	     {
		     sprintf( hname, "%s_off", gTimeBinnedMeanEffectiveAreaErec->GetName() );
		     gTimeBinnedMeanEffectiveAreaErec->SetName( hname );
	     }
	     fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gMeanEffectiveAreaErec );
	     fHisto[fHisCounter]->writeObjects( fRunPara->fRunList[fHisCounter].fEffectiveAreaFile, "EffectiveAreas", gTimeBinnedMeanEffectiveAreaErec );
      	}
      	if( fTreeSelectedEvents ) fTreeSelectedEvents->AutoSave();
      }
}


void VStereoAnalysis::writeDebugHistograms()
{
      if( fDebug ) cout << "DEBUG void VStereoAnalysis::writeDebugHistograms()" << endl;

      TDirectory *iDir = gDirectory;

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


void VStereoAnalysis::scaleAlpha( double inorm,  TH2D *halpha_on, TH2D *h_ON, TH2D *h_OFF, bool bUC, int icounter)
{

      TH2D *halpha_off = 0;
      TH2D *hmap_stereo_off = 0;
      TH2D *hmap_alphaNorm = 0;
      if( bUC )
      {
      	if( fHisCounter < 0 ) halpha_off = fHistoTot->hmap_alphaUC;
      	else                  halpha_off = fHisto[fHisCounter]->hmap_alphaUC;
      	if( fHisCounter < 0 ) hmap_alphaNorm = fHistoTot->hmap_alphaNormUC;
      	else                  hmap_alphaNorm = fHisto[fHisCounter]->hmap_alphaNormUC;
      	if( fHisCounter < 0 ) hmap_stereo_off = fHistoTot->hmap_stereoUC;
      	else                  hmap_stereo_off = fHisto[fHisCounter]->hmap_stereoUC;
      }
      else
      {
      	if( fHisCounter < 0 ) halpha_off = fHistoTot->hmap_alpha;
      	else                  halpha_off = fHisto[fHisCounter]->hmap_alpha;
      	if( fHisCounter < 0 ) hmap_alphaNorm = fHistoTot->hmap_alphaNorm;
      	else                  hmap_alphaNorm = fHisto[fHisCounter]->hmap_alphaNorm;
      	if( fHisCounter < 0 ) hmap_stereo_off = fHistoTot->hmap_stereo;
      	else                  hmap_stereo_off = fHisto[fHisCounter]->hmap_stereo;
      }
      if( !halpha_off || !hmap_alphaNorm || !hmap_stereo_off )
      {
      	cout << "VStereoAnalysis::scaleAlpha: fatal error, cannot find histograms ";
      	cout << halpha_off << "\t" << hmap_alphaNorm << "\t" << hmap_stereo_off << endl;
      	exit( -1 );
      }

// halpha_on: on alpha histogram
// halpha_off: off alpha histogram
// hmap_alphaNorm: alpha histogram used in significance calculations (alphaNorm)
      halpha_off->Scale( inorm );

//////////////////////////////////////////////////////////////////////
// FOV model only
//////////////////////////////////////////////////////////////////////
      if( fHisCounter > -1 && fRunPara->fRunList[fHisCounter].fBackgroundModel == eFOV)
      {
      // We set the information for the source, so the events coming from the source won't be taken...
      	VRadialAcceptance *fAcceptance = 0;
      	if( !bUC ) fAcceptance = fMap->getAcceptance();
      	else       fAcceptance = fMapUC->getAcceptance();
      	if( !fAcceptance )
      	{
	     cout << "Error in VStereoAnalysis::scaleAlpha, no acceptance curves available" << endl;
	     cout << "exiting..." << endl;
	     exit( 0 );
      	}
      	double nevts_on = 0.;
      	double nevts_off = 0.;

      	double cx = 0.;
      	double cy = 0.;

      	int i_xoff =  h_ON->GetXaxis()->FindBin(  fRunPara->fRunList[fHisCounter].fWobbleWestMod ) - h_ON->GetXaxis()->FindBin( 0. );
      	int j_yoff =  h_ON->GetYaxis()->FindBin(  fRunPara->fRunList[fHisCounter].fWobbleNorthMod ) - h_ON->GetYaxis()->FindBin( 0. );

// Loop on all the bins of On and Off maps to calculate the total number of events
      	for(Int_t i = 1; i < h_ON->GetNbinsX(); i++)
      	{
	     for(Int_t j = 1; j < h_ON->GetNbinsY(); j++)
	     {
		  cx = h_ON->GetXaxis()->GetBinCenter( i + i_xoff );
		  cy = h_ON->GetYaxis()->GetBinCenter( j + j_yoff );

// If the events are not in the source region, they will be calculated
		  if( fAcceptance->isExcludedfromBackground( cx,cy ) ) continue;
		  else
		  {
		       nevts_on += h_ON->GetBinContent(i ,j );
		       nevts_off += h_OFF->GetBinContent(i,j);
		  }
	     }
      	}

// Calculation of the normalisation factor for the Off map, so that it's a the same level as the On
      	double alpha_norm = (nevts_on / nevts_off);
      	if(fHisCounter >=0 ) halpha_off->Scale( 1.0/alpha_norm );

// normalize alpha (essentially 1./alpha_off)
      	for( int i = 1; i <= halpha_off->GetNbinsX(); i++ )
      	{
	    for( int j = 1; j <= halpha_off->GetNbinsY(); j++ )
	    {
		if( halpha_off->GetBinContent( i, j ) > 0. ) hmap_alphaNorm->SetBinContent( i, j, (halpha_on->GetBinContent( i, j)/halpha_off->GetBinContent( i, j )) );
      		else hmap_alphaNorm->SetBinContent( i, j, 0. );
	    }
      	}
      }
//////////////////////////////////////////////////////////////////////
// everything except FOV model
//////////////////////////////////////////////////////////////////////
      else
      {
// individual runs
      	if( fHisCounter > -1 )
      	{
// normalize alpha (essentially 1./alpha_off)
	  for( int i = 1; i <= halpha_off->GetNbinsX(); i++ )
	  {
	       for( int j = 1; j <= halpha_off->GetNbinsY(); j++ )
	       {
		    if( halpha_off->GetBinContent( i, j ) > 0. )
		    {
			 hmap_alphaNorm->SetBinContent( i, j, halpha_on->GetBinContent( i, j)/halpha_off->GetBinContent( i, j ) );
			 if( halpha_on->GetBinContent( i, j ) > 0. )
			 {
			     halpha_off->SetBinContent( i, j, halpha_off->GetBinContent( i, j)/halpha_on->GetBinContent( i, j ) );
			 }
			 else halpha_off->SetBinContent( i, j, 0. );
			 halpha_on->SetBinContent( i, j, 1. );
		    }
		    else hmap_alphaNorm->SetBinContent( i, j, 0. );
	       }
	  }
      }
// combined runs
      else
      {
      	for( int i = 1; i <= halpha_off->GetNbinsX(); i++ )
      	{
          for( int j = 1; j <= halpha_off->GetNbinsY(); j++ )
       	  {
// loop over all sky maps
	     if( hmap_stereo_off->GetBinContent( i, j ) > 0. )
	     {
	        hmap_alphaNorm->SetBinContent( i, j, halpha_off->GetBinContent( i, j ) / hmap_stereo_off->GetBinContent( i, j ) );
      	     }
             else hmap_alphaNorm->SetBinContent( i, j, 0. );
     	  }
      	}
      }
   }
}


/*!
 *   combine histograms from all runs
 *
 *   calculate combined normalisation factor for sigificance calculation
 */
double VStereoAnalysis::combineHistograms()
{
      unsigned int n_histo = fHisto.size();

      TDirectory *iDir = gDirectory;
      fDirTot->cd();
      fHistoTot->defineHistograms();
// list of trees with selected events
      TList *iTreeList = new TList();

      iDir->cd();

      for( unsigned h = 0; h < n_histo; h++ )
      {
      	fDirTotRun[h]->cd();
      // read in tree with selected events
      	if( fDirTotRun[h]->Get( "data_on" ) ) iTreeList->Add( fDirTotRun[h]->Get( "data_on" ) );
      	else if( fDirTotRun[h]->Get( "data_off" ) ) iTreeList->Add( fDirTotRun[h]->Get( "data_off" ) );
      // read in sky plots from disk
      	fHisto[h]->readSkyPlots();

      // UNCORRELATED PLOTS
      	int nxbin = fHistoTot->hmap_stereoUC->GetNbinsX();
      	int nybin = fHistoTot->hmap_stereoUC->GetNbinsY();
      	for( int i = 1; i <= nxbin; i++ )
      	{
      		for( int j = 1; j <= nybin; j++ )
      		{
      			if( fHisto[h]->hmap_alpha_offUC && fHisto[h]->hmap_alpha_offUC->GetBinContent( i, j ) > 0. )
      			{
      			     fHistoTot->hmap_stereoUC->SetBinContent( i, j, fHisto[h]->hmap_stereoUC->GetBinContent( i, j ) + fHistoTot->hmap_stereoUC->GetBinContent( i, j ) );
      			     fHistoTot->hmap_alphaUC->SetBinContent( i, j, 1./fHisto[h]->hmap_alphaUC->GetBinContent( i, j )*fHisto[h]->hmap_stereoUC->GetBinContent( i, j ) + fHistoTot->hmap_alphaUC->GetBinContent( i, j ) );
      			}
      		}
      	}

      // CORRELATED PLOTS
      	nxbin = fHistoTot->hmap_stereo->GetNbinsX();
      	nybin = fHistoTot->hmap_stereo->GetNbinsY();
      	for( int i = 1; i <= nxbin; i++ )
      	{
      		for( int j = 1; j <= nybin; j++ )
      		{
      			if( fHisto[h]->hmap_alpha_off && fHisto[h]->hmap_alpha_off->GetBinContent( i, j ) > 0. )
      			{
      			     fHistoTot->hmap_stereo->SetBinContent( i, j, fHisto[h]->hmap_stereo->GetBinContent( i, j ) + fHistoTot->hmap_stereo->GetBinContent( i, j ) );
      			     fHistoTot->hmap_alpha->SetBinContent( i, j, 1./fHisto[h]->hmap_alpha->GetBinContent( i, j )*fHisto[h]->hmap_stereo->GetBinContent( i, j ) + fHistoTot->hmap_alpha->GetBinContent( i, j ) );
      			}
      		}
      	}
      	fHisto[h]->deleteSkyPlots();
      	iDir->cd();
      }

// combine parameter (1D) histograms
      for( unsigned int h = 0; h < n_histo; h++ )
      {
      	iDir->cd();
      	fDirTotRun[h]->cd();
      	fHisto[h]->readParameterHistograms();
      	TIter next( fHistoTot->hListParameterHistograms );
      	TIter nexth( fHisto[h]->hListParameterHistograms );
      	while( TH1 *h1 = (TH1*)next() )
      	{
      		TH1 *h2 = (TH1*)nexth();

      		h1->Add( h2 );
      	}
      	fHisto[h]->deleteParameterHistograms();
      }
      iDir->cd();

// combine rate vectors (in time intervalls)

      for( unsigned int h = 0; h < n_histo; h++ )
      {
      	for( unsigned int i = 0; i <  fRateCounts[h].size(); i++ ) fRateCountsTot.push_back( fRateCounts[h][i] );
      	for( unsigned int i = 0; i <  fRateTime[h].size(); i++ ) fRateTimeTot.push_back( fRateTime[h][i] );
      	for( unsigned int i = 0; i <  fRateTimeIntervall[h].size(); i++ ) fRateTimeIntervallTot.push_back( fRateTimeIntervall[h][i] );
      }

// read in tree with selected events and make combined tree
      iDir->cd();
      for( unsigned h = 0; h < n_histo; h++ )
      {
      	fDirTotRun[h]->cd();
      	if( fDirTotRun[h]->Get( "data_on" ) ) iTreeList->Add( fDirTotRun[h]->Get( "data_on" ) );
      	else if( fDirTotRun[h]->Get( "data_off" ) ) iTreeList->Add( fDirTotRun[h]->Get( "data_off" ) );
      }
      iDir->cd();
      return fTotCount;
}


TH2D* VStereoAnalysis::getAlpha()
{
      if( fDebug ) cout << "VStereoAnalysis::getAlpha() " << fHisCounter << endl;

      if( fHisCounter < 0 ) return fHistoTot->hmap_alpha;

      return fHisto[fHisCounter]->hmap_alpha;
}


TH2D* VStereoAnalysis::getAlphaUC()
{
      if( fDebug ) cout << "VStereoAnalysis::getAlphaUC() " << fHisCounter << endl;

      if( fHisCounter < 0 ) return fHistoTot->hmap_alphaUC;

      return fHisto[fHisCounter]->hmap_alphaUC;
}


TH2D* VStereoAnalysis::getAlphaNorm()
{
      if( fDebug ) cout << "VStereoAnalysis::getAlphaNorm() " << fHisCounter << endl;

      if( fHisCounter < 0 ) return fHistoTot->hmap_alphaNorm;

      return fHisto[fHisCounter]->hmap_alphaNorm;
}


TH2D* VStereoAnalysis::getAlphaNormUC()
{
      if( fDebug ) cout << "VStereoAnalysis::getAlphaNormUC() " << fHisCounter << endl;

      if( fHisCounter < 0 ) return fHistoTot->hmap_alphaNormUC;

      return fHisto[fHisCounter]->hmap_alphaNormUC;
}


TList* VStereoAnalysis::getHisList()
{
      if( fHisCounter < 0 ) return fHistoTot->hisList;

      return fHisto[fHisCounter]->hisList;
}


TList *VStereoAnalysis::getSkyHistograms( bool bUC )
{
      if( fDebug ) cout << "VStereoAnalysis::getSkyHistograms() " << fHisCounter << "\t" << bUC << endl;

// uncorrelated plot
      if( bUC )
      {
      	if( fHisCounter < 0 ) return fHistoTot->hListSkyMapsUC;
      	return fHisto[fHisCounter]->hListSkyMapsUC;
      }
// correlated plots
      if( fHisCounter < 0 ) return fHistoTot->hListSkyMaps;
      return fHisto[fHisCounter]->hListSkyMaps;
}


TList *VStereoAnalysis::getParameterHistograms()
{
      if( fHisCounter < 0 ) return fHistoTot->hListParameterHistograms;

      return fHisto[fHisCounter]->hListParameterHistograms;
}


TH2D* VStereoAnalysis::getStereoSkyMapUC()
{
      if( fDebug ) cout << "VStereoAnalysis::getStereoSkyMapUC()" << "\t" << fHisCounter << endl;

      if( fHisCounter < 0 ) return fHistoTot->hmap_stereoUC;

      return fHisto[fHisCounter]->hmap_stereoUC;
}


TH2D* VStereoAnalysis::getStereoSkyMap()
{
      if( fDebug ) cout << "VStereoAnalysis::getStereoSkyMap()" << "\t" << fHisCounter << endl;

      if( fHisCounter < 0 ) return fHistoTot->hmap_stereo;

      return fHisto[fHisCounter]->hmap_stereo;
}


double VStereoAnalysis::getDeadTimeFraction()
{
      if( fDebug ) cout << "VStereoAnalysis::getDeadTimeFraction()" << endl;

      if( fHisCounter < 0 ) return 0.;

      if( fHisCounter < (int)fDeadTime.size() ) return fDeadTime[fHisCounter]->getDeadTimeFraction();

      return 0.;
}


void VStereoAnalysis::setAlphaOff( TH2D *ih, bool iuc )
{
      fHisto[fHisCounter]->setAlphaOff( ih, iuc );
}


void VStereoAnalysis::setAlphaOff( TH2D *ih, TH2D *ihUC )
{
      fHisto[fHisCounter]->setAlphaOff( ih, false );
      fHisto[fHisCounter]->setAlphaOff( ihUC, true );
}


/*
  (this is called for each run)

  - set targets
  - set sky map centres
  - set area to calculate 1D histograms and energy spectra
  - set exclusion regions for background calculation
*/
void VStereoAnalysis::defineAstroSource()
{
      if( fDebug ) cout << "VStereoAnalysis::defineAstroSource()" << endl;
      VTargets fTarget;
      double i_ra=0;
      double i_dec=0;
      double i_off = 0.;
      if (fIsOn) cout     << endl <<"-----------------------------------------------------------------------"<< endl
        		    << "Defining targets and exclusion regions"<< endl;

      for( unsigned int i = 0; i < fRunPara->fRunList.size(); i++ )
      {
/////////////////////////////////////////////////////////
// set source coordinates
// (this is the direction of observation)
/////////////////////////////////////////////////////////
      	if( fRunPara->fRunList[i].fTargetDecJ2000 > -85. )
      	{
      		fTarget.setTarget( fRunPara->fRunList[i].fTarget, fRunPara->fRunList[i].fTargetRAJ2000, fRunPara->fRunList[i].fTargetDecJ2000 );
      		fRunPara->setTargetRADecJ2000( i,fTarget.getTargetRAJ2000()*TMath::RadToDeg() , fTarget.getTargetDecJ2000()*TMath::RadToDeg() );
      	}
      	else if( fTarget.selectTargetbyName( fRunPara->fRunList[i].fTarget ) )
      	{
      		if( fIsOn ) fRunPara->setTargetRADecJ2000( i,fTarget.getTargetRAJ2000()*TMath::RadToDeg() , fTarget.getTargetDecJ2000()*TMath::RadToDeg());
      	}
      	else
      	{
      		cout << "ERROR in VStereoAnalysis::defineAstroSource: invalid target " << fRunPara->fRunList[i].fTarget << endl;
      		cout << "\t run " << fRunPara->fRunList[i].fRunOn << "\t" << fRunPara->fRunList[i].fTarget << endl;
      		exit( 0 );
      	}

/////////////////////////////////////////////////////////
// set the sky map centre
      	if( TMath::Abs( fRunPara->fSkyMapCentreNorth ) > 1.e-8 || TMath::Abs( fRunPara->fSkyMapCentreWest ) > 1.e-8 )
      	{
      		fRunPara->fRunList[i].fSkyMapCentreWest  = fRunPara->fSkyMapCentreWest;
      		fRunPara->fRunList[i].fSkyMapCentreNorth = fRunPara->fSkyMapCentreNorth;
      		i_dec = fTarget.getTargetDecJ2000();  // in [rad]
      		i_ra  = fTarget.getTargetRAJ2000();
      		double i_decDiff =  0.;
      		double i_raDiff = 0.;
      		VSkyCoordinatesUtilities::getWobbleOffsets( fRunPara->fRunList[i].fSkyMapCentreNorth, 
      		                                            fRunPara->fRunList[i].fSkyMapCentreWest, 
      							    i_dec*TMath::RadToDeg(), i_ra*TMath::RadToDeg(), i_decDiff, i_raDiff );
      		fRunPara->fRunList[i].fSkyMapCentreRAJ2000  = fRunPara->fRunList[i].fTargetRAJ2000 + i_raDiff;
      		fRunPara->fRunList[i].fSkyMapCentreDecJ2000 = fRunPara->fRunList[i].fTargetDecJ2000 + i_decDiff;
      	}
      	if( TMath::Abs( fRunPara->fSkyMapCentreRAJ2000  ) > 1.e-8 || TMath::Abs( fRunPara->fSkyMapCentreDecJ2000 ) )
      	{
      		fRunPara->fRunList[i].fSkyMapCentreRAJ2000 = fRunPara->fSkyMapCentreRAJ2000;
      		fRunPara->fRunList[i].fSkyMapCentreDecJ2000 = fRunPara->fSkyMapCentreDecJ2000;
      		fRunPara->fRunList[i].fSkyMapCentreWest  = fTarget.getTargetShiftWest( fRunPara->fSkyMapCentreRAJ2000, fRunPara->fSkyMapCentreDecJ2000 ) * -1.;
      		fRunPara->fRunList[i].fSkyMapCentreNorth = fTarget.getTargetShiftNorth( fRunPara->fSkyMapCentreRAJ2000, fRunPara->fSkyMapCentreDecJ2000 );
      		if( TMath::Abs( fRunPara->fRunList[i].fSkyMapCentreWest ) < 1.e-4 ) fRunPara->fRunList[i].fSkyMapCentreWest = 0.;
      		if( TMath::Abs( fRunPara->fRunList[i].fSkyMapCentreNorth) < 1.e-4 ) fRunPara->fRunList[i].fSkyMapCentreNorth = 0.;
      	}

      /////////////////////////////////////////////////////////
      // set and get target shifts
      // (calculated relative to sky map centre)
      // (this is where all 1D histograms (theta2, energy spectra, etc) are calculated)
      	if( fIsOn )
      	{
      	     if( TMath::Abs( fRunPara->fTargetShiftDecJ2000 ) > 1.e-8 || TMath::Abs( fRunPara->fTargetShiftRAJ2000 ) > 1.e-8 )
      	     {
      		  fRunPara->fRunList[i].fTargetShiftWest  = fTarget.getTargetShiftWest( fRunPara->fTargetShiftRAJ2000, fRunPara->fTargetShiftDecJ2000 );
      		  fRunPara->fRunList[i].fTargetShiftNorth = -1.*fTarget.getTargetShiftNorth( fRunPara->fTargetShiftRAJ2000, fRunPara->fTargetShiftDecJ2000 );
      		  fRunPara->fRunList[i].fTargetShiftWest  += fRunPara->fRunList[i].fSkyMapCentreWest;
      		  fRunPara->fRunList[i].fTargetShiftNorth += fRunPara->fRunList[i].fSkyMapCentreNorth;
      		  if( TMath::Abs( fRunPara->fRunList[i].fTargetShiftWest ) < 1.e-4 ) fRunPara->fRunList[i].fTargetShiftWest = 0.;
      		  if( TMath::Abs( fRunPara->fRunList[i].fTargetShiftNorth ) < 1.e-4 ) fRunPara->fRunList[i].fTargetShiftNorth = 0.;
      	     }
      	     else
      	     {
      		  fRunPara->fRunList[i].fTargetShiftWest  = fRunPara->fTargetShiftWest;
      		  fRunPara->fRunList[i].fTargetShiftNorth = fRunPara->fTargetShiftNorth;
      	     }
      	     fRunPara->fRunList[i].fTargetShiftWest *= -1.;
      	     fRunPara->fTargetShiftWest = fRunPara->fRunList[i].fTargetShiftWest;
      	     fRunPara->fTargetShiftNorth = fRunPara->fRunList[i].fTargetShiftNorth;
      	     fRunPara->setTargetShifts( i, fRunPara->fRunList[i].fTargetShiftWest, fRunPara->fRunList[i].fTargetShiftNorth, 
      	                                   fRunPara->fTargetShiftRAJ2000, fRunPara->fTargetShiftDecJ2000 );
      	}
      /////////////////////////////////////////////////////////

      	i_dec = fTarget.getTargetDecJ2000();      // in [rad]
      	i_ra  = fTarget.getTargetRAJ2000();
      // precess source coordinates
      	double iMJD = 0.;
      	if( fIsOn ) iMJD = (double)fRunPara->fRunList[i].fMJDOn;
      	else        iMJD = (double)fRunPara->fRunList[i].fMJDOff;
      	VSkyCoordinatesUtilities::precessTarget( iMJD, i_ra, i_dec);

      	if( fIsOn )
      	{
      		cout << "Run " << fRunPara->fRunList[i].fRunOn <<" ---------------------------"<< endl;
      	// set target coordinates into run parameter list
      		fRunPara->setTargetRADec( i, i_ra*TMath::RadToDeg(), i_dec*TMath::RadToDeg() );
      	// print target info to screen
      		cout << "\tTarget: " << fRunPara->fRunList[i].fTarget << " (ra,dec)=(";
      		cout << fRunPara->fRunList[i].fTargetRA << ", " << fRunPara->fRunList[i].fTargetDec << ")";
      		cout << " (precessed, MJD=" << iMJD << "), ";
      		cout << "(ra,dec)_J2000 = (" << fRunPara->fRunList[i].fTargetRAJ2000 << ", " << fRunPara->fRunList[i].fTargetDecJ2000 << ")";
      		cout << ", pair offset [min]: " << fRunPara->fRunList[i].fPairOffset  << endl;
      	}
      // get wobble offsets in ra,dec
      	double i_decDiff =  0.;
      	double i_raDiff = 0.;
      	VSkyCoordinatesUtilities::getWobbleOffsets( fRunPara->fRunList[i].fWobbleNorth, fRunPara->fRunList[i].fWobbleWest, i_dec*TMath::RadToDeg(), i_ra*TMath::RadToDeg(), i_decDiff, i_raDiff );
      	fRunPara->fRunList[i].fDecOffset = i_decDiff;
      	fRunPara->fRunList[i].fRaOffset = i_raDiff;
      	if( fabs( fRunPara->fRunList[i].fRaOffset ) < 1.e-5 ) fRunPara->fRunList[i].fRaOffset = 0.;

      // modify wobble offsets for centering of sky maps
      	fRunPara->fRunList[i].fWobbleNorthMod = fRunPara->fRunList[i].fWobbleNorth - fRunPara->fRunList[i].fSkyMapCentreNorth;
      	fRunPara->fRunList[i].fWobbleWestMod  = fRunPara->fRunList[i].fWobbleWest  - fRunPara->fRunList[i].fSkyMapCentreWest;

      ///////////////////////////////////////////////////////////////////
      // some printout
      	if( fIsOn )
      	{
      		cout << "\tWobble offsets: N: " << fRunPara->fRunList[i].fWobbleNorth << " W: " << fRunPara->fRunList[i].fWobbleWest << ", ";
      		cout << " RA: " << fRunPara->fRunList[i].fRaOffset << " Dec: " << fRunPara->fRunList[i].fDecOffset << endl;
      		cout << "\tTelescopes pointing to: (ra,dec (J2000)) (" << fRunPara->fRunList[i].fTargetRAJ2000+fRunPara->fRunList[i].fRaOffset << ", ";
      		cout << fRunPara->fRunList[i].fTargetDecJ2000+fRunPara->fRunList[i].fDecOffset << ")" << endl;
      		cout << "\tSky maps centred at (ra,dec (J2000)) (" << fRunPara->fRunList[i].fSkyMapCentreRAJ2000 << ", " << fRunPara->fRunList[i].fSkyMapCentreDecJ2000 << ")";
      		cout << ", N: " << fRunPara->fRunList[i].fWobbleNorthMod << " W: " << fRunPara->fRunList[i].fWobbleWestMod << endl;
      		cout << "\t1D-histograms calculated at (x,y): " << fRunPara->fRunList[i].fTargetShiftNorth << ", " << fRunPara->fRunList[i].fTargetShiftWest;
      		if( TMath::Abs( fRunPara->fTargetShiftDecJ2000 ) > 1.e-8 &&  TMath::Abs( fRunPara->fTargetShiftRAJ2000 ) > 1.e-8 )
      		{
      			cout << " (ra,dec (J2000)) " << fRunPara->fTargetShiftRAJ2000 << ", " << fRunPara->fTargetShiftDecJ2000;
      		}
      		cout << endl;
      	}

      // define offset for off run (usually 30. min, specified in runlist file)
      // offset is as well defined for on run if on and off runnumber are the same
      	i_off = 0.;
      	if( !fIsOn || fRunPara->fRunList[i].fRunOn == fRunPara->fRunList[i].fRunOff ) i_off = fRunPara->fRunList[i].fPairOffset/60./24.*360./TMath::RadToDeg();

      // define source and tracking class
      	fAstro.push_back( new VAstroSource( i_ra + i_raDiff/TMath::RadToDeg() + i_off ,i_dec + i_decDiff/TMath::RadToDeg(), i_ra, i_dec  ) );
      	fAstro.back()->setObservatory( fRunPara->getObservatory_Longitude_deg(), fRunPara->getObservatory_Latitude_deg() );
      	if( fIsOn )
      	{
      		fAstro.back()->init( fRunPara->fStarCatalogue, iMJD, fRunPara->fSkyMapSizeXmin, fRunPara->fSkyMapSizeXmax, fRunPara->fSkyMapSizeYmin, fRunPara->fSkyMapSizeYmax, fRunPara->fRunList[i].fSkyMapCentreRAJ2000, fRunPara->fRunList[i].fSkyMapCentreDecJ2000 );
      		fAstro.back()->getStarCatalogue()->purge();
      		if( fAstro.back()->getStarCatalogue()->getListOfStarsinFOV().size() > 0 )
      		{
      			cout << "\tbright stars (magnitude brighter than " << fRunPara->fStarMinBrightness << ", exclusion radius ";
      			cout << fRunPara->fStarExlusionRadius << " deg) in field of view (J2000): " << endl;
      			cout << "\t\t ID \t RA \t Dec \t magnitude " << endl;
      			double i_brightness = 100.;
      			for( unsigned int i = 0; i < fAstro.back()->getStarCatalogue()->getListOfStarsinFOV().size(); i++ )
      			{
      				if( fRunPara->fStarBand == "V" )      i_brightness = fAstro.back()->getStarCatalogue()->getListOfStarsinFOV()[i].fBrightness_V;
      				else if( fRunPara->fStarBand == "B" ) i_brightness = fAstro.back()->getStarCatalogue()->getListOfStarsinFOV()[i].fBrightness_B;

      				if( i_brightness < fRunPara->fStarMinBrightness )
      				{
      					cout << "\t\t" << fAstro.back()->getStarCatalogue()->getListOfStarsinFOV()[i].fStarID << "\t";
      					cout << fAstro.back()->getStarCatalogue()->getListOfStarsinFOV()[i].fRA2000 << "\t";
      					cout << fAstro.back()->getStarCatalogue()->getListOfStarsinFOV()[i].fDec2000 << "\t";
      					cout << i_brightness << " (" << fRunPara->fStarBand << " band)";
      					cout << endl;
      				// add this to exclusion regions
      					if( fRunPara->fStarExlusionRadius > 0. )
      					{
      					// check if this region is already excluded
      						bool b_isExcluded = false;
      						for( unsigned int e = 0; e < fRunPara->fExcludeFromBackground_StarID.size(); e++ )
      						{
      							if( fRunPara->fExcludeFromBackground_StarID[e] >= 0 && fRunPara->fExcludeFromBackground_StarID[e] == (int)fAstro.back()->getStarCatalogue()->getListOfStarsinFOV()[i].fStarID ) b_isExcluded = true;
      						}
      						if( !b_isExcluded )
      						{
      							fRunPara->fExcludeFromBackground_RAJ2000.push_back( fAstro.back()->getStarCatalogue()->getListOfStarsinFOV()[i].fRA2000 );
      							fRunPara->fExcludeFromBackground_DecJ2000.push_back( fAstro.back()->getStarCatalogue()->getListOfStarsinFOV()[i].fDec2000 );
      							fRunPara->fExcludeFromBackground_Radius.push_back( fRunPara->fStarExlusionRadius );
      							fRunPara->fExcludeFromBackground_North.push_back( 0. );
      							fRunPara->fExcludeFromBackground_West.push_back( 0. );
      							fRunPara->fExcludeFromBackground_StarID.push_back( (int)fAstro.back()->getStarCatalogue()->getListOfStarsinFOV()[i].fStarID );
      						}
      					}
      				}
      			}
      		}
      		fAstro.back()->getStarCatalogue()->purge();
      	}
      // doesn't work if different sources are analyzed with RA <> 360 deg
      /////////////////////////////////////////////////////////
      // set and get the regions to exclude
      // (this is set relative to the sky map centre)
      	for(unsigned int k = 0 ; k < fRunPara->fExcludeFromBackground_North.size(); k++)
      	{
      		if( fRunPara->fExcludeFromBackground_DecJ2000[k] > -90. )
      		{
      			fRunPara->fExcludeFromBackground_West[k]  = -1.* fTarget.getTargetShiftWest( fRunPara->fExcludeFromBackground_RAJ2000[k], fRunPara->fExcludeFromBackground_DecJ2000[k] ) * -1.;
      			fRunPara->fExcludeFromBackground_North[k] = -1.* fTarget.getTargetShiftNorth( fRunPara->fExcludeFromBackground_RAJ2000[k], fRunPara->fExcludeFromBackground_DecJ2000[k] );
      			fRunPara->fExcludeFromBackground_West[k]  += fRunPara->fRunList[i].fSkyMapCentreWest;
      			fRunPara->fExcludeFromBackground_North[k] += fRunPara->fRunList[i].fSkyMapCentreNorth;
      			if( TMath::Abs( fRunPara->fExcludeFromBackground_North[k] ) < 1.e-4 ) fRunPara->fExcludeFromBackground_North[k] = 0.;
      			if( TMath::Abs( fRunPara->fExcludeFromBackground_West[k] ) < 1.e-4 )  fRunPara->fExcludeFromBackground_West[k]  = 0.;
      		}
      		else
      		{
      			fRunPara->fExcludeFromBackground_West[k]  *= 1.;
      			fRunPara->fExcludeFromBackground_North[k] *= -1.;
      		}
      		fRunPara->fExcludeFromBackground_West[k]  *= -1.;
      	}
      /////////////////////////////////////////////////////////
      	fMeanSourceRA  += i_ra;
      	fMeanSourceDec += i_dec;
      }
// calculate mean ra/dec
      if( fAstro.size() > 0. )
      {
      	fMeanSourceDec = fMeanSourceDec / (double)fAstro.size() * 180. / TMath::Pi();
      	fMeanSourceRA  = fMeanSourceRA / (double)fAstro.size() * 180. / TMath::Pi();
      }
}


void VStereoAnalysis::setCuts( sRunPara iL, int irun )
{
       
      if( iL.fCutFile != "" )
      {
      	fCuts->setNTel( iL.fMaxTelID );
      	fCuts->readCuts( iL.fCutFile );
      	fCuts->setTheta2Cut( iL.fSourceRadius );
      }
      else fCuts->resetCutValues();
      fCuts->initializeCuts( irun );
      fCuts->printCutSummary();
}


vector< double > VStereoAnalysis::getRateCounts()
{
      if( fHisCounter < 0 ) return fRateCountsTot;

      if( fHisCounter < (int)fRateCounts.size() ) return fRateCounts[fHisCounter];

// this shouldn't happen
      vector< double > f;
      return f;
}


vector< double > VStereoAnalysis::getRateTime()
{
      if( fHisCounter < 0 ) return fRateTimeTot;

      if( fHisCounter < (int)fRateTime.size() ) return fRateTime[fHisCounter];

// this shouldn't happen
      vector< double > f;
      return f;
}


vector< double > VStereoAnalysis::getRateTimeIntervall()
{
      if( fHisCounter < 0 ) return fRateTimeIntervallTot;

      if( fHisCounter < (int)fRateTimeIntervall.size() ) return fRateTimeIntervall[fHisCounter];

// this shouldn't happen
      vector< double > f;
      return f;
}


/*!

   needed in VRunSummary for spectral fit

*/
TH1D* VStereoAnalysis::getEnergyHistogram()
{
      if( fHisCounter < 0 ) return fHistoTot->herec;
      else if( fHisCounter < (int)fHisto.size() )  return fHisto[fHisCounter]->herec;

      return 0;
}


TList* VStereoAnalysis::getEnergyHistograms()
{
      if( fHisCounter < 0 ) return fHistoTot->hListEnergyHistograms;
      else if( fHisCounter < (int)fHisto.size() )  return fHisto[fHisCounter]->hListEnergyHistograms;

      return 0;
}


TH1D* VStereoAnalysis::getTheta2()
{
      if( fHisCounter < 0 ) return fHistoTot->htheta2;
      else if( fHisCounter < (int)fHisto.size() )  return fHisto[fHisCounter]->htheta2;

      return 0;
}


double VStereoAnalysis::getTargetDec()
{
      if( fHisCounter < 0 ) return fMeanSourceDec;
      else if( fHisCounter < (int)fAstro.size() ) return fAstro[fHisCounter]->getSourceDec() * 180. / TMath::Pi();

      return 0.;
}


double VStereoAnalysis::getTargetRA()
{
      if( fHisCounter < 0 ) return fMeanSourceRA;
      else if( fHisCounter < (int)fAstro.size() ) return fAstro[fHisCounter]->getSourceRA() * 180. / TMath::Pi();

      return 0.;
}


double VStereoAnalysis::getRawRate()
{
      if( fHisCounter < 0 ) return 0.;
      else if( fHisCounter < (int)fHisto.size() ) return fHisto[fHisCounter]->hrate_1sec->GetEntries();

      return 0.;
}


CData* VStereoAnalysis::getDataFromFile( int i_runNumber )
{
      CData *c = 0;
      for( unsigned int i = 0; i < fRunPara->fRunList.size(); i++ )
      {
      	int i_run = fIsOn ? fRunPara->fRunList[i].fRunOn : fRunPara->fRunList[i].fRunOff;

      	if( i_runNumber > 0 && i_runNumber != i_run ) continue;

      	string iFileName = fIsOn ? fRunPara->fRunList[i].fRunOnFileName : fRunPara->fRunList[i].fRunOffFileName;

      	fDataFile = new TFile( iFileName.c_str() );
      	if( fDataFile->IsZombie() )
      	{
      		cout << "VStereoAnalysis::getDataFromFile() error opening file " << iFileName << endl;
      		exit( -1 );
      	}
      	fDataRunTree = (TTree*)fDataFile->Get( "data" );
      	if( !fDataRunTree )
      	{
      		cout << "VStereoAnalysis::getDataFromFile() error: cannot find data tree in " << iFileName << endl;
      		exit( -1 );
      	}
      	if ( fRunPara->fFrogs == 1  )
      	{
      	fDataFrogsTree = (TTree*)fDataFile->Get( "frogspars" );
      	  if( !fDataFrogsTree )
      	  {
      		  cout << "VStereoAnalysis::getDataFromFile() error: cannot find frogspars tree in " << iFileName << endl;
      	  	  exit( -1 );
      	  }
      	  fDataRunTree->AddFriend(fDataFrogsTree);
      	}
      	c = new CData( fDataRunTree );

      }
      return c;
}


bool VStereoAnalysis::closeDataFile()
{
      if( fDataFile ) fDataFile->Close();

      return true;
}


bool VStereoAnalysis::terminate()
{
      closeDataFile();

      return true;
}

bool VStereoAnalysis::init_TreeWithSelectedEvents( int irun, bool isOn )
{
   if( fTreeSelectedEvents ) delete fTreeSelectedEvents;
   if( !fRunPara ) return false;

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
   if( !isOn ) fTreeSelectedEvents->SetLineColor( 2 );

   fTreeSelectedEvents->Branch( "runNumber", &fTreeSelected_runNumber, "runNumber/I" );
   fTreeSelectedEvents->Branch( "eventNumber", &fTreeSelected_eventNumber, "eventNumber/I" );
   fTreeSelectedEvents->Branch( "MJD", &fTreeSelected_MJD, "MJD/I" );
   fTreeSelectedEvents->Branch( "Time", &fTreeSelected_Time, "Time/D" );
   fTreeSelectedEvents->Branch( "NImages", &fTreeSelected_NImages, "NImages/I" );
   fTreeSelectedEvents->Branch( "ImgSel", &fTreeSelected_ImgSel, "ImgSel/l" );
   fTreeSelectedEvents->Branch( "theta2", &fTreeSelected_theta2, "theta2/D" );
   fTreeSelectedEvents->Branch( "Xoff", &fTreeSelected_Xoff, "Xoff/D" );
   fTreeSelectedEvents->Branch( "Yoff", &fTreeSelected_Yoff, "Yoff/D" );
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
   if ( fRunPara->fFrogs == 1 )
   {
     fTreeSelectedEvents->Branch("frogsEventID", &fTreeSelescted_frogsEventID, "frogsEventID/I" );
     fTreeSelectedEvents->Branch("frogsGSLConStat", &fTreeSelescted_frogsGSLConStat, "frogsGSLConStat/I" );
     fTreeSelectedEvents->Branch("frogsNB_iter", &fTreeSelescted_frogsNB_iter, "frogsNB_iter/I" );
     fTreeSelectedEvents->Branch("frogsXS", &fTreeSelescted_frogsXS, "frogsXS/D" );
     fTreeSelectedEvents->Branch("frogsXSerr", &fTreeSelescted_frogsXSerr, "frogsXSerr/D" );
     fTreeSelectedEvents->Branch("frogsYS", &fTreeSelescted_frogsYS, "frogsYS/D" );
     fTreeSelectedEvents->Branch("frogsYSerr", &fTreeSelescted_frogsYSerr, "frogsYSerr/D" );
     fTreeSelectedEvents->Branch("frogsXP", &fTreeSelescted_frogsXP, "frogsXP/D" );
     fTreeSelectedEvents->Branch("frogsXPerr", &fTreeSelescted_frogsXPerr, "frogsXPerr/D" );
     fTreeSelectedEvents->Branch("frogsYP", &fTreeSelescted_frogsYP, "frogsYP/D" );
     fTreeSelectedEvents->Branch("frogsYPerr", &fTreeSelescted_frogsYPerr, "frogsYPerr/D" );
     fTreeSelectedEvents->Branch("frogsEnergy", &fTreeSelescted_frogsEnergy, "frogsEnergy/D" );
     fTreeSelectedEvents->Branch("frogsEnergyerr", &fTreeSelescted_frogsEnergyerr, "frogsEnergyerr/D" );
     fTreeSelectedEvents->Branch("frogsLambda", &fTreeSelescted_frogsLambda, "frogsLambda/D" );
     fTreeSelectedEvents->Branch("frogsLambdaerr", &fTreeSelescted_frogsLambdaerr, "frogsLambdaerr/D" );
     fTreeSelectedEvents->Branch("frogsGoodnessImg", &fTreeSelescted_frogsGoodnessImg, "frogsGoodnessImg/D" );
     fTreeSelectedEvents->Branch("frogsNpixImg", &fTreeSelescted_frogsNpixImg, "frogsNpixImg/I" );
     fTreeSelectedEvents->Branch("frogsGoodnessBkg", &fTreeSelescted_frogsGoodnessBkg, "frogsGoodnessBkg/D" );
     fTreeSelectedEvents->Branch("frogsNpixBkg", &fTreeSelescted_frogsNpixBkg, "frogsNpixBkg/I" );
/*
     fTreeSelectedEvents->Branch("frogsXPStart", &fTreeSelescted_frogsXPStart, "frogsXPStart/F" );
     fTreeSelectedEvents->Branch("frogsYPStart", &fTreeSelescted_frogsYPStart, "frogsYPStart/F" );
     fTreeSelectedEvents->Branch("frogsXPED", &fTreeSelescted_frogsXPED, "frogsXPED/F" );
     fTreeSelectedEvents->Branch("frogsYPED", &fTreeSelescted_frogsYPED, "frogsYPED/F" );
     fTreeSelectedEvents->Branch("frogsXSStart", &fTreeSelescted_frogsXSStart, "frogsXSStart/F" );
     fTreeSelectedEvents->Branch("frogsYSStart", &fTreeSelescted_frogsYSStart, "frogsYSStart/F" );
*/
  }

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

        fTreeSelescted_frogsEventID = 0;
        fTreeSelescted_frogsGSLConStat = 0;
        fTreeSelescted_frogsNB_iter = 0;
        fTreeSelescted_frogsXS = 0.;
        fTreeSelescted_frogsXSerr = 0.;
        fTreeSelescted_frogsYS = 0.;
        fTreeSelescted_frogsYSerr = 0.;
        fTreeSelescted_frogsXP = 0.;
        fTreeSelescted_frogsXPerr = 0.;
        fTreeSelescted_frogsYP = 0.;
        fTreeSelescted_frogsYPerr = 0.;
        fTreeSelescted_frogsEnergy = 0.;
        fTreeSelescted_frogsEnergyerr = 0.;
        fTreeSelescted_frogsLambda = 0.;
        fTreeSelescted_frogsLambdaerr = 0.;
        fTreeSelescted_frogsGoodnessImg = 0.;
        fTreeSelescted_frogsNpixImg = 0;
        fTreeSelescted_frogsGoodnessBkg = 0.;
        fTreeSelescted_frogsNpixBkg = 0;
/*
        fTreeSelescted_frogsXPStart = 0.;
        fTreeSelescted_frogsYPStart = 0.;
        fTreeSelescted_frogsXPED = 0.;
        fTreeSelescted_frogsYPED = 0.;
        fTreeSelescted_frogsXSStart = 0.;
        fTreeSelescted_frogsYSStart = 0.;
*/

}

void VStereoAnalysis::fill_TreeWithSelectedEvents( CData *c )
{
      if( !c ) return;

      fTreeSelected_runNumber = c->runNumber;
      fTreeSelected_eventNumber = c->eventNumber;
      fTreeSelected_MJD = c->MJD;
      fTreeSelected_Time = c->Time;
      fTreeSelected_NImages = c->NImages;
      fTreeSelected_ImgSel = c->ImgSel;
      fTreeSelected_theta2 = c->theta2;
      fTreeSelected_Xoff = c->Xoff;
      fTreeSelected_Yoff = c->Yoff;
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

        fTreeSelescted_frogsEventID = c->frogsEventID;
        fTreeSelescted_frogsGSLConStat = c->frogsGSLConStat;
        fTreeSelescted_frogsNB_iter = c->frogsNB_iter;
        fTreeSelescted_frogsXS = c->frogsXS;
        fTreeSelescted_frogsXSerr = c->frogsXSerr;
        fTreeSelescted_frogsYS = c->frogsYS;
        fTreeSelescted_frogsYSerr = c->frogsYSerr;
        fTreeSelescted_frogsXP = c->frogsXP;
        fTreeSelescted_frogsXPerr = c->frogsXPerr;
        fTreeSelescted_frogsYP = c->frogsYP;
        fTreeSelescted_frogsYPerr = c->frogsYPerr;
        fTreeSelescted_frogsEnergy = c->frogsEnergy;
        fTreeSelescted_frogsEnergyerr = c->frogsEnergyerr;
        fTreeSelescted_frogsLambda = c->frogsLambda;
        fTreeSelescted_frogsLambdaerr = c->frogsLambdaerr;
        fTreeSelescted_frogsGoodnessImg = c->frogsGoodnessImg;
        fTreeSelescted_frogsNpixImg = c->frogsNpixImg;
        fTreeSelescted_frogsGoodnessBkg = c->frogsGoodnessBkg;
        fTreeSelescted_frogsNpixBkg = c->frogsNpixBkg;
/*
        fTreeSelescted_frogsXPStart = c->frogsXPStart;
        fTreeSelescted_frogsYPStart = c->frogsYPStart;
        fTreeSelescted_frogsXPED = c->frogsXPED;
        fTreeSelescted_frogsYPED = c->frogsYPED;
        fTreeSelescted_frogsXSStart = c->frogsXSStart;
        fTreeSelescted_frogsYSStart = c->frogsYSStart;
*/
	if( fTreeSelectedEvents ) fTreeSelectedEvents->Fill();
}
