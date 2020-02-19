/*! \class VTableLookupDataHandler
  \brief data class for mscw and energy reconstruction

  reads evndisp output trees, fill results from mscw and energy reconstruction

*/

#include "VTableLookupDataHandler.h"

VTableLookupDataHandler::VTableLookupDataHandler( bool iwrite, VTableLookupRunParameter* iT )
{
	fTLRunParameter = iT;
	if( !fTLRunParameter )
	{
		cout << "VTableLookupDataHandler::VTableLookupDataHandler error: to table lookup run parameters" << endl;
		exit( 0 );
	}
	fDebug = fTLRunParameter->fDebug;
	fwrite = iwrite;
	fNEntries = fTLRunParameter->fNentries;
	fEventDisplayFileFormat = 2;
	fTshowerpars = 0;
	fshowerpars = 0;
	fTmodel3Dpars = 0;
	fOTree = 0;
	fShortTree = fTLRunParameter->bShortTree;
	bWriteMCPars = fTLRunParameter->bWriteMCPars;
	fNTel = 0;
	fNTelComb = 0;
	fTtelconfig = 0;
	foutputfile = "";
	fEmissionHeightCalculator = new VEmissionHeightCalculator();
	
	fEventStatus = true;
	
	// random number generator is needed only for random selection of events (optional)
	fRandom = new TRandom3();
	setSelectRandom( fTLRunParameter->fSelectRandom, fTLRunParameter->fSelectRandomSeed );
	
	fDeadTime = new VDeadTime();
	fDeadTime->defineHistograms( 0, true );
	
	fOutFile = 0;
	
	fNMethods = 0;
	fMethod = fTLRunParameter->rec_method;
	
	fIsMC = false;
	fMCEnergy = 0.;
	fZe = 0.;
	fEventCounter = 0;
	
	fEventWeight = 1.;
	fIsModel3D = false;
	
	/////////////////////////////////////////////////////////////////////////////////////
	// weighting of energy spectrum
	// MC input spectrum
	fMCSpectralIndex = 2.0;
	fMCMinEnergy = 0.05;
	fMCMaxEnergy = 50.;
	// spectral index events are weighted to
	fSpectralIndex = 2.0;
	/////////////////////////////////////////////////////////////////////////////////////
	
	// MC spectra histograms
	hisList = new TList();
	hE0mc = new TH1D( "hE0mc", "MC energy spectrum", 1000, -2., 2. );
	hE0mc->SetXTitle( "log_{10} energy_{MC} [TeV]" );
	hE0mc->SetYTitle( "number of events" );
	hE0mc->Sumw2();
	hisList->Add( hE0mc );
	
	hDE0mc = new TH2D( "hDE0mc", "distance vs. MC primary energy", 1000, -2., 2., 1000, 0., 2000. );
	hDE0mc->SetXTitle( "log_{10} energy_{MC} [TeV]" );
	hDE0mc->SetYTitle( "distance to shower core [m]" );
	hDE0mc->SetZTitle( "number of events" );
	hDE0mc->Sumw2();
	hisList->Add( hDE0mc );
	
	hXYmc = new TH2D( "hXYmc", "MC core distribution", 2000, -2000., 2000., 2000, -2000., 2000. );
	hXYmc->Sumw2();
	hXYmc->SetXTitle( "x [m]" );
	hXYmc->SetYTitle( "y [m]" );
	hisList->Add( hXYmc );
	
	hWE0mc = new TH2D( "hWE0mc", "ang. dist. vs. energy", 1000, -2., 2, 500, 0., 10. );
	hWE0mc->SetTitle( "ang. distance vs. primary energy" );
	hWE0mc->SetXTitle( "log_{10} E_{MC} [TeV]" );
	hWE0mc->SetYTitle( "distance to camera center [deg]" );
	hWE0mc->SetZTitle( "number of showers" );
	hisList->Add( hWE0mc );
	
	hZe = new TH1D( "hZe", "cos(Ze)", 100, 0., 1.1 );
	hZe->SetXTitle( "cos(Ze)" );
	hisList->Add( hZe );
	
	// same with triggered events
	hE0trig = new TH1D( "hE0trig", "MC energy spectrum (triggered events)", 1000, -2., 2. );
	hE0trig->SetXTitle( "log_{10} energy_{MC} [TeV]" );
	hE0trig->SetYTitle( "number of events" );
	hE0trig->Sumw2();
	hisList->Add( hE0trig );
	
	hDE0trig = new TH2D( "hDE0trig", "distance vs. MC primary energy (triggered events)", 1000, -2., 2., 1000, 0., 2000. );
	hDE0trig->SetXTitle( "log_{10} energy_{MC} [TeV]" );
	hDE0trig->SetYTitle( "distance to shower core [m]" );
	hDE0trig->SetZTitle( "number of events" );
	hDE0trig->Sumw2();
	hisList->Add( hDE0trig );
	
	hXYtrig = new TH2D( "hXYtrig", "core distribution (triggered events)", 2000, -2000., 2000., 2000, -2000., 2000. );
	hXYtrig->Sumw2();
	hXYtrig->SetXTitle( "x [m]" );
	hXYtrig->SetYTitle( "y [m]" );
	hisList->Add( hXYtrig );
	
	hWE0trig = new TH2D( "hWE0trig", "ang. dist. vs. energy", 1000, -2., 2, 500, 0., 10. );
	hWE0trig->SetTitle( "ang. distance vs. primary energy (triggered events)" );
	hWE0trig->SetXTitle( "log_{10} E_{MC} [TeV]" );
	hWE0trig->SetYTitle( "distance to camera center [deg]" );
	hWE0trig->SetZTitle( "number of showers" );
	hisList->Add( hWE0trig );
	
	// time cuts
	fMaxTotalTime = fTLRunParameter->fMaxRunTime;
	fTotalTime = 0.;
	fTotalTime0 = 0.;
	
	resetAll();
	
}


void VTableLookupDataHandler::fill()
{
	if( !fOTree )
	{
		return;
	}
	
	if( fTLRunParameter->bWriteReconstructedEventsOnly >= 0 || fTLRunParameter->bWriteReconstructedEventsOnly == -2 )
	{
		if( isReconstructed() )
		{
			fOTree->Fill();
		}
	}
	else
	{
		fOTree->Fill();
	}
}


void VTableLookupDataHandler::fillMChistograms()
{
	if( fIsMC )
	{
		// fill histograms with all simulated events
		double ilogE = log10( fMCEnergy );
		double idist = sqrt( fMCxcore * fMCxcore + fMCycore * fMCycore );
		double ioff  = sqrt( fMCxoff * fMCxoff + fMCyoff * fMCyoff );
		
		hE0mc->Fill( ilogE );
		hDE0mc->Fill( ilogE, idist );
		hWE0mc->Fill( ilogE, ioff );
		hXYmc->Fill( fMCxcore, fMCycore );
		hZe->Fill( fMCze );
		// fill histograms with all triggered events (require array trigger, at least 2 telescopes)
		if( fNTrig >= 2 )
		{
			hE0trig->Fill( ilogE );
			hDE0trig->Fill( ilogE, idist );
			hWE0trig->Fill( ilogE, ioff );
			hXYtrig->Fill( fMCxcore, fMCycore );
		}
		if( fNTrig > 0 )
		{
			hTrigPattern->Fill( LTrig );
		}
	}
}


double VTableLookupDataHandler::getMCDistance()
{
	return sqrt( fMCxcore * fMCxcore + fMCycore * fMCycore );
}


bool VTableLookupDataHandler::getNextEvent()
{
	return getNextEvent( false );
}

/*!
    return values:

    true:     get successfully an event
    false:    end of data chain or time limit exceeded
*/
bool VTableLookupDataHandler::getNextEvent( bool bShort )
{
	if( fEventCounter < fNEntries && fTotalTime < fMaxTotalTime )
	{
		if( !randomSelected() )
		{
			fEventCounter++;
			return true;
		}
		fEventWeight = 1.;
		
		int iNE = 1;
		if( fEventDisplayFileFormat >= 2 )
		{
			iNE = fillNextEvent( bShort );
		}
		else
		{
			cout << "unknown eventdisplay file format: " << fEventDisplayFileFormat << endl;
			cout << "(possible old format? Format version: " << fEventDisplayFileFormat << ")" << endl;
			cout << "...exiting" << endl;
			exit( -1 );
		}
		if( iNE == -1 )
		{
			return false;
		}
		// dead time calculation
		if( !fIsMC && getEventNumber() != 999999 )
		{
			fDeadTime->fillDeadTime( time );
		}
		
		// return false for non-valid (maybe not reconstructed?) event
		if( iNE == 0 )
		{
			return true;
		}
		
		if( !fIsMC )
		{
			ftheta2 = ( fYoff_derot - fWobbleN ) * ( fYoff_derot - fWobbleN ) + ( fXoff_derot - fWobbleE ) * ( fXoff_derot - fWobbleE );
		}
		else
		{
			ftheta2 = ( fXoff - fMCxoff ) * ( fXoff - fMCxoff ) + ( fYoff - fMCyoff ) * ( fYoff - fMCyoff );
		}
		
		// calculate distances
		calcDistances( fNImages );
		// calculate emission height (not for writing of tables)
		if( fNImages > 1 && !fwrite )
		{
			calcEmissionHeights();
		}
		
		setEventWeightfromMCSpectrum();
	}
	else
	{
		return false;
	}
	return true;
}

int VTableLookupDataHandler::fillNextEvent( bool bShort )
{
	if( !fshowerpars->GetEntry( fEventCounter ) )
	{
		return -1;
	}
	// count all events
	fNStats_All++;
	////////////////////////////////////////////////////
	// read first all entries needed for run modes (filling and reading)
	fNMethods = fshowerpars->NMethods;
	if( fMethod >= fNMethods )
	{
		cout << "VTableLookupDataHandler::fillNextEvent() error, invalid array reconstruction record" << endl;
		cout << "\t maximum number of records are " << fNMethods << " (request is " << fMethod << ")" << endl;
		cout << "exiting..." << endl;
		exit( 0 );
	}
	fNImages = fshowerpars->NImages[fMethod];
	fchi2 = fshowerpars->Chi2[fMethod];
	// for table filling: check as soon as possible if the event is useful
	if( fwrite && !isReconstructed() )
	{
		fEventStatus = false;
		fEventCounter++;
		fNStats_Chi2Cut++;
		return 0;
	}
	// fill MC parameters
	if( fIsMC )
	{
		fMCEnergy = fshowerpars->MCe0;
		fMCaz = fshowerpars->MCaz;
		fMCze = fshowerpars->MCze;
		fMCxcore = fshowerpars->MCxcore;
		fMCycore = fshowerpars->MCycore;
		fMCxoff = fshowerpars->MCxoff;
		fMCyoff = fshowerpars->MCyoff;
		if( !bShort && !fShortTree && !fwrite )
		{
			fMCxcore_SC = fshowerpars->MCxcore_SC;
			fMCycore_SC = fshowerpars->MCycore_SC;
			fMCxcos = fshowerpars->MCxcos;
			fMCycos = fshowerpars->MCycos;
		}
	}
	
	// the following variables are not set in table filling mode
	if( !fwrite )
	{
		runNumber = fshowerpars->runNumber;
		eventNumber = fshowerpars->eventNumber;
		if( fDebug > 1 )
		{
			cout << "===============================================================================" << endl;
			cout << "SHOWERPARS EVENT " << fshowerpars->eventNumber << "\t" << fEventCounter << "\t";
			cout << fshowerpars->NImages[fMethod] << "\t" << fshowerpars->Chi2[fMethod] << endl;
		}
		time = fshowerpars->Time;
		if( fEventCounter == 0 )
		{
			fTotalTime0 = time;
		}
		fTotalTime = time - fTotalTime0;
		
		for( unsigned int i = 0; i < fNTel; i++ )
		{
			fTelElevation[i] = fshowerpars->TelElevation[i];
			fTelAzimuth[i] = fshowerpars->TelAzimuth[i];
		}
		if( !fIsMC )
		{
			for( unsigned int i = 0; i < fNTel; i++ )
			{
				fTelDec[i] = fshowerpars->TelDec[i];
				fTelRA[i] = fshowerpars->TelRA[i];
			}
			if( !fShortTree )
			{
				MJD = fshowerpars->MJD;
			}
		}
		fNTrig = fshowerpars->NTrig;
		
		if( !bShort )
		{
			LTrig = ( ULong64_t )fshowerpars->LTrig;
		}
	}
	
	fZe = fshowerpars->Ze[fMethod];
	fAz = fshowerpars->Az[fMethod];
	fXcore = fshowerpars->Xcore[fMethod];
	fYcore = fshowerpars->Ycore[fMethod];
	if( TMath::IsNaN( fXcore ) || TMath::IsNaN( fYcore ) )
	{
		fXcore =  -999999.;
		fYcore =  -999999.;
		fEventCounter++;
		fEventStatus = false;
		if( fDebug > 1 )
		{
			cout << "\t RECONSTRUCTED CORE NAN" << endl;
		}
		return 0;
	}
	fXoff = fshowerpars->Xoff[fMethod];
	fYoff = fshowerpars->Yoff[fMethod];
	fXoff_derot = fshowerpars->XoffDeRot[fMethod];
	fYoff_derot = fshowerpars->YoffDeRot[fMethod];
	fDispDiff = fshowerpars->DispDiff[fMethod];
	
	fImgSel = ( ULong64_t )fshowerpars->ImgSel[fMethod];
	unsigned int ii = 0;
	for( unsigned int i = 0; i < getNTelTypes(); i++ )
	{
		NImages_Ttype[i] = 0;
	}
	for( unsigned int i = 0; i < getNTel(); i++ )
	{
		fImgSel_list[i] = ( bool )fshowerpars->ImgSel_list[fMethod][i];
		if( fImgSel_list[i] )
		{
			fImgSel_list_short[ii] = i;
			// count the number of telescopes of this type
			NImages_Ttype[getTelType_arraycounter( i )]++;
			ii++;
		}
	}
	
	fimg2_ang = fshowerpars->img2_ang[fMethod];
	if( !bShort && !fShortTree )
	{
		fRA = fshowerpars->ra[fMethod];
		fDec = fshowerpars->dec[fMethod];
		fstdS = fshowerpars->stds[fMethod];
		fXcore_SC = fshowerpars->Xcore_SC[fMethod];
		fYcore_SC = fshowerpars->Ycore_SC[fMethod];
		fstdP = fshowerpars->stdp[fMethod];
	}
	
	// for filling of lookup tables: first do quality cuts, if not return
	if( fwrite )
	{
		if( !cut( true ) )
		{
			fEventCounter++;
			fEventStatus = false;
			if( fDebug > 1 )
			{
				cout << "\t CUT FAILED" << endl;
			}
			return 0;
		}
		else
		{
			fEventStatus = true;
		}
	}
	
	// loop over all telescopes
	bitset<8 * sizeof( unsigned long )> i_nimage; // for imagepattern
	i_nimage.reset();
	
	Double_t SizeFirstMax_temp = -1000.;
	Double_t SizeSecondMax_temp = -100.;
	for( unsigned int i = 0; i < fNTel; i++ )
	{
		bool fReadTPars = false;
		if( i < ftpars.size() && ftpars[i] )
		{
			fReadTPars = true;
		}
		if( ( fTLRunParameter->bWriteReconstructedEventsOnly >= 0 )
				|| fTLRunParameter->bWriteReconstructedEventsOnly == -2 || fwrite )
		{
			if( fImgSel_list[i] )
			{
				fReadTPars = true;
			}
			else
			{
				fReadTPars = false;
			}
		}
		// read only those telescope which were part of the reconstruction
		if( fReadTPars )
		{
			if( !ftpars[i] )
			{
				cout << "VTableLookupDataHandler::fillNextEvent error: tree tpars not found (telescope " << i + 1 << ")" << endl;
				cout << "\t(run " << runNumber << ", " << eventNumber << ")" << endl;
				exit( -1 );
			}
			ftpars[i]->GetEntry( fEventCounter );
			
			fdist[i] = ftpars[i]->dist;
                        // MNR: We need to correct here the sizes too.
			fsize[i] = ftpars[i]->size * fTLRunParameter->fEnergySizecorrection[i];
			fsize2[i] = ftpars[i]->size2 * fTLRunParameter->fEnergySizecorrection[i];
			floss[i] = ftpars[i]->loss;
			ffracLow[i] = ftpars[i]->fracLow;
			fwidth[i] = ftpars[i]->width;
			flength[i] = ftpars[i]->length;
			
			if( fsize[i] > SizeSecondMax_temp )
			{
				if( fsize[i] > SizeFirstMax_temp )
				{
					SizeSecondMax_temp = SizeFirstMax_temp;
					SizeFirstMax_temp = fsize[i];
				}
				else
				{
					SizeSecondMax_temp = fsize[i];
				}
			}
			
			if( !bShort )
			{
				fCurrentNoiseLevel[i] = ftpars[i]->meanPedvar_Image;
				fmeanPedvar_ImageT[i] = ftpars[i]->meanPedvar_Image;
				fntubes[i] = ftpars[i]->ntubes;
				fnsat[i] = ftpars[i]->nsat;
				fnlowgain[i] = ftpars[i]->nlowgain;
				falpha[i] = ftpars[i]->alpha;
				flos[i] = ftpars[i]->los;
				fasym[i] = ftpars[i]->asymmetry;
				fcen_x[i] = ftpars[i]->cen_x;
				fcen_y[i] = ftpars[i]->cen_y;
				fcosphi[i] = ftpars[i]->cosphi;
				fsinphi[i] = ftpars[i]->sinphi;
				fmax1[i] = ftpars[i]->max[0];
				fmax2[i] = ftpars[i]->max[1];
				fmax3[i] = ftpars[i]->max[2];
				fmaxindex1[i] = ftpars[i]->index_of_max[0];
				fmaxindex2[i] = ftpars[i]->index_of_max[1];
				fmaxindex3[i] = ftpars[i]->index_of_max[2];
				ftgrad_x[i] = ftpars[i]->tgrad_x;
				ftchisq_x[i] = ftpars[i]->tchisq_x;
				fFitstat[i] = ftpars[i]->Fitstat;
			}
		}
		else
		{
			resetImageParameters( i );
		}
		fweight[i] = 1.;
		
		if( !bShort && fntubes[i] > 4 && i < i_nimage.size() && i < 10 )
		{
			i_nimage.set( i, 1 );
		}
	}
	fmeanPedvar_Image = calculateMeanNoiseLevel( true );
	if( !bShort && fNTrig >= 2 && i_nimage.to_ulong() > 0 && getNTel() < 10 )
	{
		hImagePattern->Fill( ( ( double )i_nimage.to_ulong() ) );
	}
	
	if( SizeSecondMax_temp > 0 )
	{
		fSizeSecondMax = SizeSecondMax_temp;
	}
	
	fEventCounter++;
	return 1;
}

bool VTableLookupDataHandler::checkIfFilesInChainAreRecovered( TChain* c )
{
	if( !c )
	{
		cout << "VTableLookupDataHandler::checkIfFilesInChainAreRecovered() error: no chain" << endl;
		return true;
	}
	
	TObjArray* fileElements = c->GetListOfFiles();
	if( !fileElements )
	{
		cout << "VTableLookupDataHandler::checkIfFilesInChainAreRecovered() error: no files in chain" << endl;
		return true;
	}
	TChainElement* chEl = 0;
	TIter next( fileElements );
	while( ( chEl = ( TChainElement* )next() ) )
	{
		TFile* ifInput = new TFile( chEl->GetTitle() );
		if( ifInput->IsZombie() )
		{
			cout << "VTableLookupDataHandler::checkIfFilesInChainAreRecovered() error: file cannot be recovered; possibly not complete" << endl;
			cout << "\t " << chEl->GetTitle() << endl;
			return true;
		}
		if( ifInput->TestBit( TFile::kRecovered ) )
		{
			cout << "VTableLookupDataHandler::checkIfFilesInChainAreRecovered() error: file recovered; possibly not complete" << endl;
			cout << "\t " << chEl->GetTitle() << endl;
			return true;
		}
		// check if input data includes Model3D parameters
		fKeyModel3D = ifInput->FindKey( "model3Dpars" );
		if( fKeyModel3D != 0 )
		{
			fIsModel3D = true;
		}
		
		ifInput->Close();
	}
	
	return false;
}

bool VTableLookupDataHandler::setInputFile( vector< string > iInput )
{
	finputfile = iInput;
	// need to find suffix .root: add it if it doesn't exist
	for( unsigned int i = 0; i < finputfile.size(); i++ )
	{
		if( finputfile[i].find( ".root" ) == string::npos )
		{
			cout << "TableLookupDataHandler::setInputFile: adding .root suffix to file name" << endl;
			finputfile[i] += ".root";
		}
		cout << "opening file(s): " << endl;
		cout << finputfile[i] << endl;
	}
	
	//////////////////////////////////////////////////////////////////////////////////////
	// get telescope configuration
	// get it from the telescope configuration tree (if avaible), else assume two telescope setup
	fTtelconfig = new TChain( "telconfig" );
	int iNFil_sum = 0;
	for( unsigned int i = 0; i < finputfile.size(); i++ )
	{
		int iNFil = fTtelconfig->Add( finputfile[i].c_str() );
		if( iNFil == 0 )
		{
			cout << "error: no file(s) in chain" << endl;
			exit( -1 );
		}
		iNFil_sum += iNFil;
	}
	cout << iNFil_sum << " file(s) in chain " << endl;
	// don't check each file for CTA sims -> this is very inefficent and it takes a long time
	if( !fTLRunParameter->fPE && fTLRunParameter->readwrite != 'W' )
	{
		if( checkIfFilesInChainAreRecovered( fTtelconfig ) )
		{
			cout << "VTableLookupDataHandler::setInputFile() error: some file are not properly closed" << endl;
			cout << "exit..." << endl;
			exit( -1 );
		}
	}
	
	fList_of_Tel_type.clear();
	if( fTtelconfig )
	{
		ftelconfig = new Ctelconfig( fTtelconfig );
		ftelconfig->GetEntry( 0 );
		// read in telescope configuration
		fNTel = ftelconfig->NTel;
		if( fNTel > getMaxNbrTel() )
		{
			cout << "VTableLookupDataHandler::setInputFile: error too many telescopes " << fNTel << "\t" << getMaxNbrTel() << endl;
			exit( -1 );
		}
		fNTelComb = ( unsigned int )TMath::Nint( TMath::Power( 2., ( double )fNTel ) );
		for( unsigned int i = 0; i < fNTel; i++ )
		{
			ftelconfig->GetEntry( i );
			fTelX[i] = ftelconfig->TelX;
			fTelY[i] = ftelconfig->TelY;
			fTelZ[i] = ftelconfig->TelZ;
			fFocalLength[i] = ftelconfig->FocalLength;
			fTel_type[i] = ftelconfig->TelType;
			if( fList_of_Tel_type.find( ftelconfig->TelType ) != fList_of_Tel_type.end() )
			{
				fList_of_Tel_type[ftelconfig->TelType]++;
			}
			else
			{
				fList_of_Tel_type[ftelconfig->TelType] = 1;
			}
		}
		// number of different telescope types
		fNTelTypes = ( int )fList_of_Tel_type.size();
	}
	else
	{
		cout << "VTableLookupDataHandler::setInputFile error: no telescope configurations found " << endl;
		cout << "...exiting" << endl;
		exit( -1 );
	}
	
	// print everything to the screen
	cout << endl << "total number of telescopes: " << fNTel << endl;
	initializeTelTypeVector();
	for( unsigned int i = 0; i < fNTel; i++ )
	{
		cout << "\t telescope " << i + 1 << "\t";
		cout << "x:" << fTelX[i] << " [m]\ty:" << fTelY[i] << " [m]\tz:" << fTelZ[i] << " [m]\t";
		cout << "type " << fTel_type[i] << " (type counter " << getTelType_arraycounter( i ) << ")";
		cout << endl;
	}
	cout << endl;
	cout << "list of telescope types (" << fList_of_Tel_type.size() << "): ";
	for( fList_of_Tel_type_iterator = fList_of_Tel_type.begin(); fList_of_Tel_type_iterator != fList_of_Tel_type.end();
			fList_of_Tel_type_iterator++ )
	{
		cout << "  " << fList_of_Tel_type_iterator->first << " (" << fList_of_Tel_type_iterator->second << " telescopes)";
	}
	cout << endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// copy telescope positions to emission height calculator
	fEmissionHeightCalculator->setTelescopePositions( fNTel, fTelX, fTelY, fTelZ );
	
	// define trigger histogram
	Int_t nBinsX = 0;
	if( fNTel < 28 )
	{
		nBinsX = ( Int_t )pow( 2., ( double )fNTel );
	}
	else
	{
		nBinsX = 28;
	}
	hTrigPattern = new TH1D( "hTrigPattern", "", nBinsX, 0., ( double )nBinsX );
	hTrigPattern->SetStats( 0 );
	hTrigPattern->SetFillColor( 4 );
	hTrigPattern->SetYTitle( "% of all triggered events" );
	hTrigPattern->GetYaxis()->SetTitleOffset( 1.2 );
	// might result in dodgy root files for large numbers of telescopes
	//    hisList->Add( hTrigPattern );
	
	hImagePattern = new TH1D( "hImagePattern", "", nBinsX, 0., ( double )nBinsX );
	hImagePattern->SetStats( 0 );
	hImagePattern->SetFillColor( 4 );
	hImagePattern->SetYTitle( "% of all reconstructed events" );
	hImagePattern->GetYaxis()->SetTitleOffset( 1.2 );
	// might result in dodgy root files for large numbers of telescopes
	//    hisList->Add( hImagePattern );
	
	sTrigPattern.push_back( "Tel.1" );
	sTrigPattern.push_back( "Tel.2" );
	sTrigPattern.push_back( "Tel.1+2" );
	if( fNTel >=  3 )
	{
		sTrigPattern.push_back( "Tel.3" );
		sTrigPattern.push_back( "Tel.1+3" );
		sTrigPattern.push_back( "Tel.2+3" );
		sTrigPattern.push_back( "Tel.1+2+3" );
	}
	if( fNTel == 4 )
	{
		sTrigPattern.push_back( "Tel.4" );
		sTrigPattern.push_back( "Tel.1+4" );
		sTrigPattern.push_back( "Tel.2+4" );
		sTrigPattern.push_back( "Tel.1+2+4" );
		sTrigPattern.push_back( "Tel.3+4" );
		sTrigPattern.push_back( "Tel.1+3+4" );
		sTrigPattern.push_back( "Tel.2+3+4" );
		sTrigPattern.push_back( "Tel.1+2+3+4" );
	}
	// this does not work for too many telescopes
	if( fNTel > 4 && fNTel < 10 )
	{
		char hLabl[200];
		for( int i = ( int )pow( 2., 4. ); i < ( int )pow( 2., ( double )fNTel ); i++ )
		{
			sprintf( hLabl, "%d", i );
			sTrigPattern.push_back( hLabl );
		}
	}
	for( unsigned int i = 0; i < sTrigPattern.size(); i++ )
	{
		hTrigPattern->GetXaxis()->SetBinLabel( i + 2, sTrigPattern[i].c_str() );
		hImagePattern->GetXaxis()->SetBinLabel( i + 2, sTrigPattern[i].c_str() );
	}
	
	char iName[100];
	char iDir[1000];
	unsigned int bShort = false;
	// get shower parameter tree
	fTshowerpars = new TChain( "showerpars" );
	// get model3Dpars tree
	if( fIsModel3D )
	{
		fTmodel3Dpars = new TChain( "model3Dpars" );
	}
	
	for( unsigned int i = 0; i < finputfile.size(); i++ )
	{
		fTshowerpars->Add( finputfile[i].c_str() );
		if( fIsModel3D )
		{
			fTmodel3Dpars->Add( finputfile[i].c_str() );
		}
	}
	if( !fTshowerpars )
	{
		cout << "VTableLookupDataHandler::setInputFile: error while retrieving data trees (2)" << endl;
		cout << "exiting..." << endl;
		exit( -1 );
	}
	// check validity of showerpars tree
	if( !fTshowerpars->GetBranchStatus( "runNumber" ) )
	{
		cout << "VTableLookupDataHandler::setInputFile: error while retrieving data trees (2b)" << endl;
		cout << "exiting..." << endl;
		exit( -1 );
	}
	// check if input data is MC
	if( fTshowerpars->GetBranchStatus( "MCe0" ) )
	{
		fIsMC = true;
		cout << "input data is of Monte Carlo type" << endl;
	}
	else
	{
		fIsMC = false;
		cout << "input data is not Monte Carlo type" << endl;
	}
	
	// update runparameters
	fTLRunParameter->update( fTshowerpars );
	// don't set maximum number of entries anymore (not needed, and it takes a long time)
	//    if( fNEntries == 0 || fNEntries >=fTshowerpars->GetEntries() ) fNEntries = fTshowerpars->GetEntries();
	// get file format version of eventdisplay (tree version)
	if( fTLRunParameter )
	{
		fEventDisplayFileFormat = fTLRunParameter->getEVNDISP_TREE_VERSION();
		bShort                  = ( unsigned int )fTLRunParameter->getEVNDISP_TREE_isShort( fTshowerpars->GetTree() );
	}
	// check file format
	if( fEventDisplayFileFormat >= 2 )
	{
		if( bShort )
		{
			cout << "input data is of eventdisplay short tree output format (" << bShort << ")" << endl;
		}
		fshowerpars = new Cshowerpars( fTshowerpars, fIsMC, fEventDisplayFileFormat, bShort );
		fIsMC = fshowerpars->isMC();
	}
	else
	{
		fEventDisplayFileFormat = 1;
	}
	// for table filling: minimizing reading of trees
	if( fwrite )
	{
		bShort = 2;
	}
	
	// get individual image parameter trees
	TChain* iT;
	for( unsigned int i = 0; i < fNTel; i++ )
	{
		sprintf( iName, "tpars" );
		iT = new TChain( iName );
		for( unsigned int f = 0; f < finputfile.size(); f++ )
		{
			sprintf( iDir, "%s/Tel_%d/tpars", finputfile[f].c_str(), i + 1 );
			iT->Add( iDir );
		}
		if( !iT )
		{
			cout << "VTableLookupDataHandler::setInputFile: error while retrieving data trees (3)" << endl;
			exit( -1 );
		}
		// get first entry to check if chain is there
		gErrorIgnoreLevel = 5000;
		if( iT->GetEntry( 0 ) > 0 )
		{
			if( fEventDisplayFileFormat >= 2 )
			{
				if( fEventDisplayFileFormat < 5 )
				{
					if( iT->GetBranchStatus( "loss" ) )
					{
						fEventDisplayFileFormat = 3;
					}
					if( iT->GetBranchStatus( "meanPedvar_Image" ) )
					{
						fEventDisplayFileFormat = 5;
					}
				}
				ftpars.push_back( new Ctpars( iT, fIsMC, fEventDisplayFileFormat, bShort ) );
			}
		}
		else
		{
			ftpars.push_back( 0 );
		}
		gErrorIgnoreLevel = 0;
	}
	cout << "reading eventdisplay file format version " << fEventDisplayFileFormat;
	if( fIsMC )
	{
		cout << " (source files are Monte Carlo)";
	}
	cout << endl;
	
	////////////////////////////////////////////////////////////////////////////////////
	// calculating median of pedvar distribution (not if input data is of PE format)
	fNoiseLevel.clear();
	fCurrentNoiseLevel.assign( fNTel, 0. );
	for( unsigned int i = 0; i < fNTel; i++ )
	{
		// standard data format
		if( !fTLRunParameter->fPE )
		{
			if( fDebug > 1 )
			{
				cout << "VTableLookupDataHandler::setInputFile() calculating pedvar for telescope " << i + 1 << endl;
			}
			sprintf( iName, "calib_%d", i + 1 );
			TChain iPedVars( iName );
			for( unsigned int f = 0; f < finputfile.size(); f++ )
			{
				gErrorIgnoreLevel = 5000;
				sprintf( iDir, "%s/Tel_%d/calib_%d", finputfile[f].c_str(), i + 1, i + 1 );
				if( !iPedVars.Add( iDir ) )
				{
					cout << "VTableLookupDataHandler::setInputFile: error while retrieving pedvars trees" << endl;
					cout << "exiting..." << endl;
					exit( -1 );
				}
				if( iPedVars.GetEntries() == 0 )
				{
					// backwards compatibility: read calibration tree from a different directory (note: this produces a root error message)
					sprintf( iDir, "%s/Tel_%d/calibration/calib_%d", finputfile[f].c_str(), i + 1, i + 1 );
					if( !iPedVars.Add( iDir ) )
					{
						cout << "VTableLookupDataHandler::setInputFile: error while retrieving pedvars trees" << endl;
						cout << "exiting..." << endl;
						exit( -1 );
					}
				}
				gErrorIgnoreLevel = 0;
			}
			gErrorIgnoreLevel = 5000;
			double pedvar = 0.;
			double mpedvar = 0.;
			double npedvar = 0.;
			double state = 0;
			iPedVars.SetBranchAddress( "pedvar", &pedvar );
			if( iPedVars.GetBranchStatus( "state" ) )
			{
				iPedVars.SetBranchAddress( "state", &state );
			}
			
			sprintf( iName, "ht_%d", i + 1 );
			TH1D h( iName, "", 1000, 0., 50. );
			
			if( fDebug > 1 )
			{
				cout << "VTableLookupDataHandler::setInputFile() calculating pedvar for telescope ";
				cout << i + 1 << ", number of entries: " << iPedVars.GetEntries() << endl;
			}
			for( int n = 0; n < iPedVars.GetEntries(); n++ )
			{
				iPedVars.GetEntry( n );
				
				if( pedvar > 0. && state == 0 )
				{
					mpedvar += pedvar;
					npedvar++;
					h.Fill( pedvar );
				}
			}
			double xq[1];
			double yq[1];
			xq[0] = 0.5;
			yq[0] = 0.;
			if( h.GetEntries() > 0. )
			{
				h.GetQuantiles( 1, yq, xq );
			}
			fNoiseLevel.push_back( yq[0] );
			gErrorIgnoreLevel = 0;
			if( fDebug > 1 )
			{
				cout << "VTableLookupDataHandler::setInputFile() calculating pedvar for telescope (results): " << i + 1 << "\t" << yq[0] << endl;
			}
		}
		// PE format -> ignore noise level calculation
		else
		{
			fNoiseLevel.push_back( 0. );
		}
	}
	if( fDebug )
	{
		cout << "VTableLookupDataHandler::setInputFile() END" << endl;
	}
	
	return fIsMC;
}


/*!

set data output file and define output tree

\param iOutput output file name
\param iOption 'RECREATE' or 'UPDATE'
*/
bool VTableLookupDataHandler::setOutputFile( string iOutput, string iOption, string tablefile )
{
	foutputfile = iOutput;
	
	if( fNTel == 0 )
	{
		cout << "VTableLookupDataHandler::setOutputFile error: no telescopes" << endl;
		exit( -1 );
	}
	
	for( unsigned int i = 0; i < finputfile.size(); i++ )
	{
		if( foutputfile == finputfile[i] && iOption == "recreate" )
		{
			cout << "VTableLookupDataHandler::setOutputFile error: can't overwrite inputfile" << endl;
			cout << "\t" << finputfile[i] << endl;
			exit( -1 );
		}
	}
	
	// open output file
	fOutFile = new TFile( foutputfile.c_str(), iOption.c_str() );
	if( fOutFile->IsZombie() )
	{
		cout << "VTableLookupDataHandler::setOutputFile error while opening output file " << foutputfile << "\t" << iOption << endl;
		exit( -1 );
	}
	// define output tree
	char iTT[2000];
	
	if( fEventDisplayFileFormat < 6 )
	{
		sprintf( iTT, "MSWC and energy lookup results (%s) VERSION %d", tablefile.c_str(), fEventDisplayFileFormat + 1 );
	}
	else
	{
		sprintf( iTT, "MSWC and energy lookup results (%s) VERSION %d", tablefile.c_str(), fEventDisplayFileFormat );
	}
	fOTree = new TTree( "data", iTT );
	fOTree->SetMaxTreeSize( 1000 * Long64_t( 2000000000 ) );
	
	fOTree->Branch( "runNumber", &runNumber, "runNumber/I" );
	fOTree->Branch( "eventNumber", &eventNumber, "eventNumber/I" );
	if( !fShortTree )
	{
		fOTree->Branch( "MJD", &MJD, "MJD/I" );
	}
	if( !fShortTree )
	{
		fOTree->Branch( "Time",  &time,  "Time/D" );
	}
	sprintf( iTT, "TelElevation[%d]/D", fNTel );
	fOTree->Branch( "TelElevation", fTelElevation, iTT );
	sprintf( iTT, "TelAzimuth[%d]/D", fNTel );
	fOTree->Branch( "TelAzimuth", fTelAzimuth, iTT );
	sprintf( iTT, "TelDec[%d]/D", fNTel );
	if( !fShortTree )
	{
		fOTree->Branch( "TelDec", fTelDec, iTT );
	}
	sprintf( iTT, "TelRA[%d]/D", fNTel );
	if( !fShortTree )
	{
		fOTree->Branch( "TelRA", fTelRA, iTT );
	}
	if( !fShortTree )
	{
		fOTree->Branch( "TargetElev", &fTargetElev, "TargetElev/D" );
	}
	if( !fShortTree )
	{
		fOTree->Branch( "TargetAz", &fTargetAz, "TargetAz/D" );
	}
	if( !fShortTree )
	{
		fOTree->Branch( "TargetDec", &fTargetDec, "TargetDec/D" );
	}
	if( !fShortTree )
	{
		fOTree->Branch( "TargetRA", &fTargetRA, "TargetRA/D" );
	}
	fOTree->Branch( "WobbleN", &fWobbleN, "WobbleN/D" );
	fOTree->Branch( "WobbleE", &fWobbleE, "WobbleE/D" );
	
	// MC parameters
	if( fIsMC )
	{
		fOTree->Branch( "MCprimary", &fMCPrimary, "MCprimary/I" );
		fOTree->Branch( "MCe0", &fMCEnergy, "MCe0/D" );
		fOTree->Branch( "MCxcore", &fMCxcore, "MCxcore/D" );
		fOTree->Branch( "MCycore", &fMCycore, "MCycore/D" );
		sprintf( iTT, "MCR[%d]/D", fNTel );
		// (nowhere needed)        fOTree->Branch( "MCR", fMCR, iTT );
		if( !fShortTree )
		{
			fOTree->Branch( "MCxcore_SC", &fMCxcore_SC, "MCxcore_SC/D" );
		}
		if( !fShortTree )
		{
			fOTree->Branch( "MCycore_SC", &fMCycore_SC, "MCycore_SC/D" );
		}
		if( !fShortTree )
		{
			fOTree->Branch( "MCxcos", &fMCxcos, "MCxcos/D" );
		}
		if( !fShortTree )
		{
			fOTree->Branch( "MCycos", &fMCycos, "MCycos/D" );
		}
		fOTree->Branch( "MCaz", &fMCaz, "MCaz/D" );
		fOTree->Branch( "MCze", &fMCze, "MCze/D" );
		fOTree->Branch( "MCxoff", &fMCxoff, "MCxoff/D" );
		fOTree->Branch( "MCyoff", &fMCyoff, "MCyoff/D" );
	}
	
	fOTree->Branch( "LTrig", &LTrig, "LTrig/l" );
	fOTree->Branch( "NTrig", &fNTrig, "NTrig/i" );
	fOTree->Branch( "NImages", &fNImages, "NImages/I" );
	fOTree->Branch( "ImgSel", &fImgSel, "ImgSel/l" );
	
	// telescope type related variables
	fOTree->Branch( "ImgSel_list",  fImgSel_list_short, "ImgSel_list[NImages]/i" );
	fOTree->Branch( "NTtype", &fNTelTypes, "NTtype/I" );
	fOTree->Branch( "NImages_Ttype", NImages_Ttype, "NImages_Ttype[NTtype]/i" );
	
	fOTree->Branch( "img2_ang", &fimg2_ang, "img2_ang/D" );
	fOTree->Branch( "RecID", &fMethod, "RecID/I" );
	fOTree->Branch( "Ze", &fZe, "Ze/D" );
	fOTree->Branch( "Az", &fAz, "Az/D" );
	if( !fShortTree )
	{
		fOTree->Branch( "ra", &fRA, "ra/D" );
	}
	if( !fShortTree )
	{
		fOTree->Branch( "dec", &fDec, "dec/D" );
	}
	fOTree->Branch( "Xoff", &fXoff, "Xoff/D" );
	fOTree->Branch( "Yoff", &fYoff, "Yoff/D" );
	fOTree->Branch( "Xoff_derot", &fXoff_derot, "Xoff_derot/D" );
	fOTree->Branch( "Yoff_derot", &fYoff_derot, "Yoff_derot/D" );
	if( !fShortTree )
	{
		fOTree->Branch( "stdS", &fstdS, "stdS/D" );
	}
	if( !fShortTree )
	{
		fOTree->Branch( "theta2", &ftheta2, "theta2/D" );
	}
	if( !fShortTree )
	{
		fOTree->Branch( "theta2_All", &ftheta2_All, "theta2_All[25]/D" );
	}
	fOTree->Branch( "Xcore", &fXcore, "Xcore/D" );
	fOTree->Branch( "Ycore", &fYcore, "Ycore/D" );
	if( !fShortTree )
	{
		fOTree->Branch( "Xcore_SC", &fXcore_SC, "Xcore_SC/D" );
	}
	if( !fShortTree )
	{
		fOTree->Branch( "Ycore_SC", &fYcore_SC, "Ycore_SC/D" );
	}
	fOTree->Branch( "stdP", &fstdP, "stdP/D" );
	fOTree->Branch( "Chi2", &fchi2, "Chi2/D" );
	
	fOTree->Branch( "meanPedvar_Image", &fmeanPedvar_Image, "meanPedvar_Image/F" );
	
	if( !fShortTree )
	{
		sprintf( iTT, "meanPedvar_ImageT[%d]/F", fNTel );
		fOTree->Branch( "meanPedvar_ImageT", fmeanPedvar_ImageT, iTT );
		sprintf( iTT, "dist[%d]/D", fNTel );
		fOTree->Branch( "dist", fdist, iTT );
		sprintf( iTT, "size[%d]/D", fNTel );
		fOTree->Branch( "size", fsize, iTT );
		sprintf( iTT, "size2[%d]/D", fNTel );
		fOTree->Branch( "size2", fsize2, iTT );
		sprintf( iTT, "loss[%d]/D", fNTel );
		fOTree->Branch( "loss", floss, iTT );
		sprintf( iTT, "fracLow[%d]/D", fNTel );
		fOTree->Branch( "fracLow", ffracLow, iTT );
		sprintf( iTT, "max1[%d]/D", fNTel );
		fOTree->Branch( "max1", fmax1, iTT );
		sprintf( iTT, "max2[%d]/D", fNTel );
		fOTree->Branch( "max2", fmax2, iTT );
		sprintf( iTT, "max3[%d]/D", fNTel );
		fOTree->Branch( "max3", fmax3, iTT );
		sprintf( iTT, "maxindex1[%d]/I", fNTel );
		fOTree->Branch( "maxindex1", fmaxindex1, iTT );
		sprintf( iTT, "maxindex2[%d]/I", fNTel );
		fOTree->Branch( "maxindex2", fmaxindex2, iTT );
		sprintf( iTT, "maxindex3[%d]/I", fNTel );
		fOTree->Branch( "maxindex3", fmaxindex3, iTT );
		sprintf( iTT, "width[%d]/D", fNTel );
		fOTree->Branch( "width", fwidth, iTT );
		sprintf( iTT, "length[%d]/D", fNTel );
		fOTree->Branch( "length", flength, iTT );
		sprintf( iTT, "ntubes[%d]/I", fNTel );
		fOTree->Branch( "ntubes", fntubes, iTT );
		sprintf( iTT, "nsat[%d]/s", fNTel );
		fOTree->Branch( "nsat", fnsat, iTT );
		sprintf( iTT, "nlowgain[%d]/s", fNTel );
		fOTree->Branch( "nlowgain", fnlowgain, iTT );
		sprintf( iTT, "alpha[%d]/D", fNTel );
		fOTree->Branch( "alpha", falpha, iTT );
		sprintf( iTT, "los[%d]/D", fNTel );
		fOTree->Branch( "los", flos, iTT );
		sprintf( iTT, "asym[%d]/D", fNTel );
		fOTree->Branch( "asym", fasym, iTT );
		sprintf( iTT, "cen_x[%d]/D", fNTel );
		fOTree->Branch( "cen_x", fcen_x, iTT );
		sprintf( iTT, "cen_y[%d]/D", fNTel );
		fOTree->Branch( "cen_y", fcen_y, iTT );
		sprintf( iTT, "cosphi[%d]/D", fNTel );
		fOTree->Branch( "cosphi", fcosphi, iTT );
		sprintf( iTT, "sinphi[%d]/D", fNTel );
		fOTree->Branch( "sinphi", fsinphi, iTT );
		sprintf( iTT, "tgrad_x[%d]/D", fNTel );
		fOTree->Branch( "tgrad_x", ftgrad_x, iTT );
		sprintf( iTT, "tchisq_x[%d]/D", fNTel );
		fOTree->Branch( "tchisq_x", ftchisq_x, iTT );
		sprintf( iTT, "Fitstat[%d]/I", fNTel );
		fOTree->Branch( "Fitstat", fFitstat, iTT );
	}
	fOTree->Branch( "DispDiff", &fDispDiff, "DispDiff/D" );
	// Model3D parameters
	if( fIsModel3D )
	{
		fOTree->Branch( "Smax3D", &fSmax3D, "Smax3D/D" );
		fOTree->Branch( "sigmaL3D", &fsigmaL3D, "sigmaL3D/D" );
		fOTree->Branch( "sigmaT3D", &fsigmaT3D, "sigmaT3D/D" );
		fOTree->Branch( "Nc3D", &fNc3D, "Nc3D/D" );
		fOTree->Branch( "Xcore3D", &fXcore3D, "Xcore3D/D" );
		fOTree->Branch( "Ycore3D", &fYcore3D, "Ycore3D/D" );
		fOTree->Branch( "Xoff3D", &fXoff3D, "Xoff3D/D" );
		fOTree->Branch( "Yoff3D", &fYoff3D, "Yoff3D/D" );
		fOTree->Branch( "XoffDeRot3D", &fXoffDeRot3D, "fXoffDeRot3D/D" );
		fOTree->Branch( "YoffDeRot3D", &fYoffDeRot3D, "fYoffDeRot3D/D" );
		fOTree->Branch( "Goodness3D", &fGoodness3D, "Goodness3D/D" );
		fOTree->Branch( "Depth3D", &fDepth3D, "Depth3D/D" );
		fOTree->Branch( "RWidth3D", &fRWidth3D, "RWidth3D/D" );
		fOTree->Branch( "ErrRWidth3D", &fErrRWidth3D, "ErrRWidth3D/D" );
		fOTree->Branch( "Converged3D", &fConverged3D, "Converged3D/b" );
	}
	
	sprintf( iTT, "R[%d]/D", fNTel );
	fOTree->Branch( "R", fR, iTT );
	sprintf( iTT, "MSCWT[%d]/D", fNTel );
	if( !fShortTree )
	{
		fOTree->Branch( "MSCWT", ftmscw, iTT );
	}
	sprintf( iTT, "MSCLT[%d]/D", fNTel );
	if( !fShortTree )
	{
		fOTree->Branch( "MSCLT", ftmscl, iTT );
	}
	sprintf( iTT, "MSCWTSigma[%d]/F", fNTel );
	if( !fShortTree )
	{
		fOTree->Branch( "MSCWTSigma", ftmscw_sigma, iTT );
	}
	sprintf( iTT, "MSCLTSigma[%d]/F", fNTel );
	if( !fShortTree )
	{
		fOTree->Branch( "MSCLTSigma", ftmscl_sigma, iTT );
	}
	sprintf( iTT, "E[%d]/D", fNTel );
	fOTree->Branch( "E", fE, iTT );
	sprintf( iTT, "ES[%d]/D", fNTel );
	fOTree->Branch( "ES", fES, iTT );
	
	sprintf( iTT, "NMSCW/I" );
	if( !fShortTree )
	{
		fOTree->Branch( "NMSCW", &fnmscw, iTT );
	}
	sprintf( iTT, "MSCW/D" );
	fOTree->Branch( "MSCW", &fmscw, iTT );
	sprintf( iTT, "MSCL/D" );
	fOTree->Branch( "MSCL", &fmscl, iTT );
	sprintf( iTT, "MWR/F" );
	fOTree->Branch( "MWR", &fmwr, iTT );
	sprintf( iTT, "MLR/F" );
	fOTree->Branch( "MLR", &fmlr, iTT );
	sprintf( iTT, "Erec/D" );
	fOTree->Branch( "Erec", &fenergy, iTT );
	sprintf( iTT, "EChi2/D" );
	fOTree->Branch( "EChi2", &fechi2, iTT );
	sprintf( iTT, "dE/D" );
	fOTree->Branch( "dE", &fdE, iTT );
	sprintf( iTT, "Esys/F" );
	if( !fShortTree )
	{
		fOTree->Branch( "Esys", &fesys, iTT );
	}
	sprintf( iTT, "EsysVar/F" );
	if( !fShortTree )
	{
		fOTree->Branch( "EsysVar", &fesysVar, iTT );
	}
	sprintf( iTT, "EsysDist/F" );
	if( !fShortTree )
	{
		fOTree->Branch( "EsysDist", &fesysDist, iTT );
	}
	sprintf( iTT, "ErecS/D" );
	fOTree->Branch( "ErecS", &fenergyS, iTT );
	sprintf( iTT, "EChi2S/D" );
	fOTree->Branch( "EChi2S", &fechi2S, iTT );
	sprintf( iTT, "dES/D" );
	fOTree->Branch( "dES", &fdES, iTT );
	
	sprintf( iTT, "EmissionHeight/F" );
	fOTree->Branch( "EmissionHeight", &fEmissionHeightMean, iTT );
	sprintf( iTT, "EmissionHeightChi2/F" );
	fOTree->Branch( "EmissionHeightChi2", &fEmissionHeightChi2, iTT );
	sprintf( iTT, "NTelPairs/i" );
	fOTree->Branch( "NTelPairs", &fNTelPairs, iTT );
	fOTree->Branch( "SizeSecondMax", &fSizeSecondMax, "SizeSecondMax/D" );
	
	sprintf( iTT, "fEmissionHeightT[NTelPairs]/F" );
	if( !fShortTree )
	{
		fOTree->Branch( "EmissionHeightT", fEmissionHeightT, iTT );
	}
	for( unsigned int i = 0; i < getMaxNbrTel(); i++ )
	{
		fEmissionHeightT[i] = -99.;
	}
	
	readRunParameter();
	
	return true;
}


bool VTableLookupDataHandler::readRunParameter()
{

	// get run parameter (only if a single inputfile is read)
	if( fEventDisplayFileFormat > 1 )
	{
		// get list of files in chain
		TObjArray* fileElements = fTshowerpars->GetListOfFiles();
		TChainElement* chEl = 0;
		TIter next( fileElements );
		chEl = ( TChainElement* )next();
		
		TFile ifInput( chEl->GetTitle() );
		if( !ifInput.IsZombie() )
		{
			cout << "reading eventdisplay run parameters from " << ifInput.GetName() << endl;
			TNamed* iR = ( TNamed* )ifInput.Get( "runparameter" );
			if( iR )
			{
				fOutFile->cd();
				iR->Write();
			}
			VEvndispRunParameter* iPar = ( VEvndispRunParameter* ) ifInput.Get( "runparameterV2" );
			VEvndispReconstructionParameter* iA = ( VEvndispReconstructionParameter* )ifInput.Get( "EvndispReconstructionParameter" );
                        VMonteCarloRunHeader* iMC = ( VMonteCarloRunHeader* )ifInput.Get( "MC_runheader" );
                        if( iMC )
                        {
                             fTLRunParameter->ze = iMC->getMeanZenithAngle_Deg();
                        }
			vector< unsigned int > iTelToAnalyze;
			if( iPar )
			{
				if( fTLRunParameter->fTelToAnalyse.size() > 0 )
				{
					iPar->fTelToAnalyze = fTLRunParameter->fTelToAnalyse;
				}
				else if( iA )
				{
					//copied from VTableLookupRunParameter.cpp
					vector< unsigned int > iRunParT = iPar->fTelToAnalyze;
					// this works only if number of telescopes = number of telescope types
					if( fTLRunParameter->rec_method < ( int )iA->fLocalUseImage.size() && iPar->fNTelescopes == iA->fLocalUseImage[fTLRunParameter->rec_method].size() )
					{
						for( unsigned int i = 0; i < iRunParT.size(); i++ )
						{
							if( iRunParT[i] < iA->fLocalUseImage[fTLRunParameter->rec_method].size() && iA->fLocalUseImage[fTLRunParameter->rec_method][iRunParT[i]] )
							{
								iTelToAnalyze.push_back( iRunParT[i] );
							}
						}
					}
					iPar->fTelToAnalyze = iTelToAnalyze;
				}
				
				fOutFile->cd();
				iPar->Write();
			}
		}
		ifInput.Close();
		fOutFile->cd();
	}
	
	return true;
}

void VTableLookupDataHandler::printCutStatistics()
{
	cout << "---------------------------------------------------------------------------------------------------" << endl;
	cout << "Cut statistics: " << endl;
	if( fNStats_All == 0 )
	{
		cout << "\t no events..." << endl;
		return;
	}
	unsigned int nTOT = fNStats_All;
	
	cout << "\t number of events considered: \t\t" << fNStats_All << " (" << ( float )fNStats_All / ( float )fNStats_All << ")" << endl;
	nTOT -= fNStats_NImagesCut;
	cout << "\t removed by >= " << fTLRunParameter->fTableFillingCut_NImages_min  << " images: \t\t" << fNStats_NImagesCut;
	cout << " (fraction removed/# of events left: " << ( float )fNStats_NImagesCut / ( float )fNStats_All << "; " << nTOT << ")" << endl;
	nTOT = nTOT - fNStats_Chi2Cut + fNStats_NImagesCut;
	cout << "\t removed by Chi2 >=0:   \t\t" << fNStats_Chi2Cut;
	cout << " (fraction removed/# of events left: " << ( float )fNStats_Chi2Cut / ( float )fNStats_All << "; " << nTOT << ")" << endl;
	cout << "\t number of reconstructed events:   \t\t" << fNStats_Rec;
	cout << " (fraction of reconstructed events: " << ( float )fNStats_Rec / ( float )fNStats_All << "; " << nTOT << ")" << endl;
	
	nTOT -= fNStats_CoreErrorCut;
	cout << "\t removed by cut on core misreconstruction: \t\t" << fNStats_CoreErrorCut;
	cout << " (fraction removed/# of events left: " << ( float )fNStats_CoreErrorCut / ( float )fNStats_All << "; " << nTOT << ")" << endl;
	nTOT = nTOT - fNStats_WobbleCut + fNStats_CoreErrorCut;
	cout << "\t removed by wobble cut (<" << fTLRunParameter->fTableFillingCut_WobbleCut_max << "): \t\t" << fNStats_WobbleCut;
	cout << " (fraction removed/# of events left: " << ( float )fNStats_WobbleCut / ( float )fNStats_All << "; " << nTOT << ")" << endl;
	nTOT = nTOT - fNStats_WobbleMinCut + fNStats_WobbleCut;
	cout << "\t removed by MC wobble min cut (>" << fMC_distance_to_cameracenter_min << "): \t\t" << fNStats_WobbleMinCut;
	cout << " (fraction removed/# of events left: " << ( float )fNStats_WobbleMinCut / ( float )fNStats_All << "; " << nTOT << ")" << endl;
	nTOT -= fNStats_WobbleMaxCut;
	cout << "\t removed by wobble max cut (<" << fMC_distance_to_cameracenter_max << "): \t\t" << fNStats_WobbleMaxCut;
	cout << " (fraction removed/# of events left: " << ( float )fNStats_WobbleMaxCut / ( float )fNStats_All << "; " << nTOT << ")" << endl;
	cout << "---------------------------------------------------------------------------------------------------" << endl;
}

/*!
  write everything to disk
*/
bool VTableLookupDataHandler::terminate( TNamed* iM )
{
	printCutStatistics();
	// scale pattern histogram to %
	if( hTrigPattern->GetEntries() > 0 )
	{
		hTrigPattern->Scale( 1. / hTrigPattern->GetEntries() * 100. );
	}
	if( hImagePattern->GetEntries() > 0 )
	{
		hImagePattern->Scale( 1. / hImagePattern->GetEntries() * 100. );
	}
	if( fOutFile )
	{
		cout << "writing data to " << fOutFile->GetName() << endl;
		fOutFile->cd();
		
		cout << endl << "\t total number of events in output tree: " << fOTree->GetEntries() << endl << endl;
		fOTree->Write( "", TObject::kOverwrite );
		
		if( iM )
		{
			cout << "\t writing table lookup run parameter" << endl;
			iM->Write();
		}
		else
		{
			cout << "\t no table lookup run parameter to write" << endl;
		}
		
		if( fTtelconfig )
		{
			cout << "\t writing telescope configuration" << endl;
			copy_telconfig();
		}
		else
		{
			cout << "\t no telescope configuration to write" << endl;
		}
		fOutFile->cd();
		
		if( fIsMC )
		{
			cout << "\t writing MC debug histograms" << endl;
			hisList->Write();
		}
		// see if there is a dead time object on file, if not: write the one filled here
		// (note: at this stage, the scalars cannot be used and the dead time might be
		//         underestimated)
		if( !fIsMC )
		{
			writeDeadTimeHistograms();
		}
		
		if( fIsMC )
		{
			bool iMCTree_exists = copyMCRunheader();
			if( bWriteMCPars && iMCTree_exists )
			{
				copyMCTree();
			}
			copyMCHistograms();
		}
		
		///////////////////////////////////////////////////////////////////////////
		// copy TTree 'pointingDataReduced' and 'deadPixelRegistry' from evndisp.<>.root to mscw.<>.root
		if( finputfile.size() > 1 )
		{
			cout << "Warning, VTableLookupDataHandler->finputfile.size() isn't 1, not sure which input file to copy TTree 'pointingDataReduced' from, copying from file finputfile[0]:" << finputfile[0] << endl;
		}
		// not sure why we don't want to do this for MC
		if( finputfile.size() > 0 && !fIsMC )
		{
			TFile* inpMscwFile = new TFile( finputfile[0].c_str(), "READ" ) ;
			fOutFile->cd();
			TTree* iTree       = ( TTree* )inpMscwFile->Get( "pointingDataReduced" );
			if( iTree ) 
			{
				TTree* newtree     = iTree->CloneTree();
				if( newtree ) 
				{
					newtree->Write();
				}
				else 
				{
					cout << "VTableLookupDataHandler::terminate Warning: Unable to clone tree " << iTree->GetName() << endl;
				}
			}
			else 
			{
				cout << "VTableLookupDataHandler::terminate Warning: Unable to find tree pointingDataReduced in file " << inpMscwFile->GetName() << endl;
			}	

			TTree* jTree = ( TTree* )inpMscwFile->Get( "deadPixelRegistry" ) ;
			// deadPixelRegistry may not exist, only try to copy it if it's there
			if( jTree )
			{
				fOutFile->cd() ;
				TTree* newtree2 = jTree->CloneTree() ;
				newtree2->Write() ;
			}
			
		}
		else if( !fIsMC )
		{
			cout << "Warning, VTableLookupDataHandler->finputfile has size 0, unable to copy TTree 'pointingDataReduced' to file " << fOutFile->GetName() << endl;
		}
		
		
		fOutFile->Close();
		cout << "...outputfile closed" << endl;
		cout << "(" << fOutFile->GetName() << ")" << endl;
	}
	
	return true;
}

void VTableLookupDataHandler::writeDeadTimeHistograms()
{
	if( finputfile.size() > 1 )
	{
		cout << "VTableLookupDataHandler::writeDeadTimeHistograms() error: ";
		cout << "analysis of several files at once not allowed " << endl;
		cout << "(dead times will be wrong)" << endl;
		exit( -1 );
	}
	
	// use chain to get list of files
	TChain iTel( "telconfig" );
	int iNFil = 0;
	for( unsigned int i = 0; i < finputfile.size(); i++ )
	{
		iNFil += iTel.Add( finputfile[i].c_str() );
	}
	
	if( iNFil > 0 && fDeadTime )
	{
		TFile* f = iTel.GetFile();
		if( f )
		{
			TDirectoryFile* iDeadtimeDirectory = ( TDirectoryFile* )f->Get( "deadTimeHistograms" );
			if( iDeadtimeDirectory )
			{
				fDeadTime->readHistograms( iDeadtimeDirectory );
			}
			fDeadTime->calculateDeadTime();
			fDeadTime->printDeadTime();
			fDeadTime->writeHistograms();
		}
	}
}

void VTableLookupDataHandler::copy_telconfig()
{
	TChain iMC( "telconfig" );
	int iNFil = 0;
	for( unsigned int i = 0; i < finputfile.size(); i++ )
	{
		iNFil += iMC.Add( finputfile[i].c_str() );
	}
	
	if( iNFil > 0 )
	{
		TFile* f = iMC.GetFile();
		if( f )
		{
			if( f->Get( "telconfig" ) )
			{
				TTree* t = ( TTree* )f->Get( "telconfig" );
				fOutFile->cd();
				TTree* n = t->CloneTree();
				n->Write();
			}
		}
	}
}

/*
   copy MC run header from first file

   OBS: assume that all run headers in all files are the same!!!!

   check additionally if MCpars tree exists
*/
bool VTableLookupDataHandler::copyMCRunheader()
{
	// use chain to get list of files
	TChain iTel( "telconfig" );
	int iNFil = 0;
	for( unsigned int i = 0; i < finputfile.size(); i++ )
	{
		iNFil += iTel.Add( finputfile[i].c_str() );
	}
	
	if( iNFil > 0 )
	{
		TFile* f = iTel.GetFile();
		if( f )
		{
			if( ( VMonteCarloRunHeader* )f->Get( "MC_runheader" ) )
			{
				fOutFile->cd();
				f->Get( "MC_runheader" )->Write();
				cout << "\t MC run header found and copied" << endl;
			}
			if( f->Get( "MCpars" ) )
			{
				return true;
			}
		}
	}
	return false;
}


void VTableLookupDataHandler::copyMCTree()
{
	TChain iMC( "MCpars" );
	int iNFil = 0;
	for( unsigned int i = 0; i < finputfile.size(); i++ )
	{
		iNFil += iMC.Add( finputfile[i].c_str() );
	}
	
	if( iNFil > 0 && iMC.GetEntries() > 0 )
	{
		cout << "\t copying MC tree with " << iMC.GetEntries() << " entries..." << flush;
		iMC.Merge( fOutFile, 0, "keep" );
		cout << "done " << endl;
	}
}

void VTableLookupDataHandler::copyMCHistograms()
{
	vector< double > i_az_min;
	vector< double > i_az_max;
	vector< double > i_spectral_index = fTLRunParameter->fAddMC_spectral_index;;
	if( i_spectral_index.size() == 1 )
	{
		cout << "\t VTableLookupDataHandler::copyMCHistograms: reducing spectral index range to one value: ";
		cout << i_spectral_index[0] << endl;
	}
	VEffectiveAreaCalculatorMCHistograms* iMC_his = 0;
	if( fTshowerpars )
	{
		// loop over all files and add MC histograms
		TObjArray* fileElements = fTshowerpars->GetListOfFiles();
		if( !fileElements )
		{
			cout << "VTableLookupDataHandler::copyMCHistograms(): no list of files found" << endl;
			return;
		}
		TChainElement* chEl = 0;
		TIter next( fileElements );
		unsigned int z = 0;
		while( ( chEl = ( TChainElement* )next() ) )
		{
			TFile* ifInput = new TFile( chEl->GetTitle() );
			if( !ifInput->IsZombie() )
			{
				if( z == 0 )
				{
					iMC_his = ( VEffectiveAreaCalculatorMCHistograms* )ifInput->Get( "MChistos" );
					if( iMC_his && i_spectral_index.size() > 0 )
					{
						iMC_his->matchDataVectors( i_az_min, i_az_max, i_spectral_index );
					}
				}
				else
				{
					if( iMC_his )
					{
						VEffectiveAreaCalculatorMCHistograms* iMC_his_temp =
							( VEffectiveAreaCalculatorMCHistograms* )ifInput->Get( "MChistos" );
						if( iMC_his_temp && i_spectral_index.size() > 0 )
						{
							iMC_his_temp->matchDataVectors( i_az_min, i_az_max, i_spectral_index );
							iMC_his->add( iMC_his_temp );
						}
					}
					ifInput->Close();
				}
				z++;
			}
		}
		if( iMC_his && fOutFile )
		{
			cout << "\t writing MC histograms" << endl;
			iMC_his->print();
			fOutFile->cd();
			iMC_his->Write();
		}
	}
}


void VTableLookupDataHandler::reset()
{
	for( unsigned int i = 0; i < fNTel; i++ )
	{
		fR[i] = -99.;
		fE[i] = -99.;
		fES[i] = -99.;
		ftmscl[i] = -99.;
		ftmscw[i] = -99.;
		ftmscw_sigma[i] = -99.;
		ftmscl_sigma[i] = -99.;
	}
	fnmscw = 0;
	fmscl = -99.;
	fmscw = -99.;
	fmwr  = -99.;
	fmlr  = -99.;
	fenergy = -99.;
	fechi2 = -99.;
	fdE = -99.;
	fesys = 0.;
	fesysVar = 0.;
	fesysDist = 0.;
	fenergyS = -99.;
	fechi2S = -99.;
	fdES = -99.;
	fXoff = -99.;
	fYoff = -99.;
	fXoff_derot = -99.;
	fYoff_derot = -99.;
	fXcore = -99.;
	fYcore = -99.;
	fstdP = -99.;
	
	fEmissionHeightMean = -99.;
	fEmissionHeightChi2 = -99.;
	
	fmeanPedvar_Image = 0.;
	
	resetImageParameters();
}


/*!
  calculate distances between telescopes and reconstructed shower core

*/
void VTableLookupDataHandler::calcDistances( int nimages )
{
	// check for successfull reconstruction
	if( nimages > 1 && fZe >= 0. && fYcore > -9998. && fYcore > -9998. )
	{
		for( unsigned int tel = 0; tel < fNTel; tel++ )
		{
			if( fImgSel_list[tel] )
			{
				fR[tel] = VUtilities::line_point_distance( fYcore, -1.*fXcore, 0., fZe, fAz, fTelY[tel], -1.*fTelX[tel], fTelZ[tel] );
			}
			else
			{
				fR[tel] = -99.;
			}
		}
	}
	else
	{
		for( unsigned int tel = 0; tel < fNTel; tel++ )
		{
			fR[tel] = -99.;
		}
	}
}



void VTableLookupDataHandler::resetImageParameters()
{
	for( unsigned int i = 0; i < fNTel; i++ )
	{
		resetImageParameters( i );
	}
}


void VTableLookupDataHandler::resetImageParameters( unsigned int i )
{
	if( i > getMaxNbrTel() )
	{
		return;
	}
	
	fdist[i] = 0.;
	fsize[i] = 0.;
	fsize2[i] = 0.;
	floss[i] = 0.;
	ffracLow[i] = 0.;
	fwidth[i] = 0.;
	flength[i] = 0.;
	fmeanPedvar_ImageT[i] = 0.;
	if( fwrite )
	{
		return;
	}
	
	fntubes[i] = 0;
	fnsat[i] = 0;
	fnlowgain[i] = 0;
	falpha[i] = 0.;
	flos[i] = 0.;
	fasym[i] = 0.;
	fcen_x[i] = 0.;
	fcen_y[i] = 0.;
	fcosphi[i] = 0.;
	fsinphi[i] = 0.;
	fmax1[i] = 0.;
	fmax2[i] = 0.;
	fmax3[i] = 0.;
	fmaxindex1[i] = 0;
	fmaxindex2[i] = 0;
	fmaxindex3[i] = 0;
	ftgrad_x[i] = 0.;
	ftchisq_x[i] = 0.;
	fFitstat[i] = 0;
}

bool VTableLookupDataHandler::isReconstructed()
{
	if( fchi2 < 0 )
	{
		return false;
	}
	if( fNImages < 2 )
	{
		return false;
	}
	
	return true;
}


void VTableLookupDataHandler::calcEmissionHeights()
{
	fEmissionHeightCalculator->getEmissionHeight( fcen_x, fcen_y, fdist, fsize, fR, fTelAzimuth, fTelElevation );
	fNTelPairs = fEmissionHeightCalculator->getNTelPairs();
	fEmissionHeightMean = ( float )fEmissionHeightCalculator->getMeanEmissionHeight();
	fEmissionHeightChi2 = ( float )fEmissionHeightCalculator->getMeanEmissionHeightChi2();
	if( fEmissionHeightChi2 <= 0. )
	{
		fEmissionHeightChi2 = 1.e-10;
	}
	for( unsigned int i = 0; i < fNTelPairs; i++ )
	{
		if( i >= getMaxNbrTel() || i >= fEmissionHeightCalculator->getEmissionHeights().size() )
		{
			break;
		}
		
		fEmissionHeightT[i] = ( float )fEmissionHeightCalculator->getEmissionHeights()[i];
	}
}


void VTableLookupDataHandler::setEventWeightfromMCSpectrum()
{
	fEventWeight = 1.;
	
	if( TMath::Abs( fSpectralIndex - fMCSpectralIndex ) < 1.e-2 )
	{
		fEventWeight = 1.;
	}
	else if( fSpectralIndex > fMCSpectralIndex )
	{
		double alpha = TMath::Power( fMCMinEnergy, -1.*fMCSpectralIndex ) / TMath::Power( fMCMinEnergy, -1.*fSpectralIndex );
		
		fEventWeight = alpha * TMath::Power( fMCEnergy, -1.*fSpectralIndex ) / TMath::Power( fMCEnergy, -1.*fMCSpectralIndex );
	}
	else if( fSpectralIndex < fMCSpectralIndex )
	{
		double alpha = TMath::Power( fMCMaxEnergy, -1.*fMCSpectralIndex ) / TMath::Power( fMCMaxEnergy, -1.*fSpectralIndex );
		
		fEventWeight = alpha * TMath::Power( fMCEnergy, -1.*fSpectralIndex ) / TMath::Power( fMCEnergy, -1.*fMCSpectralIndex );
	}
}


double VTableLookupDataHandler::getZe()
{
	return fZe;
}


/*!

  return copy of fMCEnergy as an array

  (note that mscw modifies these values)

*/
double* VTableLookupDataHandler::getMCEnergyArray()
{
	for( unsigned int i = 0; i < fNTel; i++ )
	{
		fMCEnergyArray[i] = getMCEnergy();
	}
	return fMCEnergyArray;
}


void VTableLookupDataHandler::resetAll()
{
	fEventStatus = true;
	runNumber = 0;
	eventNumber = 0;
	MJD = 0;
	time = 0;
	for( unsigned int i = 0; i < getMaxNbrTel(); i++ )
	{
		fTelElevation[i] = 0.;
		fTelAzimuth[i] = 0.;
		fTelDec[i] = 0.;
		fTelRA[i] = 0.;
	}
	fTargetElev = 0.;
	fTargetAz = 0.;
	fTargetDec = 0.;
	fTargetRA = 0.;
	fWobbleN = 0.;
	fWobbleE = 0.;
	fMCPrimary = -99;
	fMCEnergy = 0.;
	fMCxcore = 0.;
	fMCycore = 0.;
	fMCxcore_SC = 0.;
	fMCycore_SC = 0.;
	fMCxcos = 0.;
	fMCycos = 0.;
	fMCaz = 0.;
	fMCze = 0.;
	fMCxoff = 0.;
	fMCyoff = 0.;
	LTrig = 0;
	fNTrig = 0;
	fNImages = 0;
	fImgSel = 0;
	fNTelTypes = 0;
	for( unsigned int i = 0; i < getMaxNbrTel(); i++ )
	{
		fImgSel_list[i] = false;
		fImgSel_list_short[i] = 0;
		NImages_Ttype[i] = 0;
	}
	fimg2_ang = 0.;
	fZe = 0.;
	fAz = 0.;
	fRA = 0.;
	fDec = 0.;
	fXoff = 0.;
	fYoff = 0.;
	fXoff_derot = 0.;
	fYoff_derot = 0.;
	fstdS = 0.;
	ftheta2 = 0.;
	fXcore = 0.;
	fYcore = 0.;
	fXcore_SC = 0.;
	fYcore_SC = 0.;
	fstdP = 0.;
	fchi2 = 0.;
	fmeanPedvar_Image = 0.;
	for( unsigned int i = 0; i < getMaxNbrTel(); i++ )
	{
		fdist[i] = 0.;
		fsize[i] = 0.;
		fsize2[i] = 0.;
		fsizeCorr[i] = 0.;
		fsize_telType[i] = 0.;
		floss[i] = 0.;
		ffracLow[i] = 0.;
		fmax1[i] = 0.;
		fmax2[i] = 0.;
		fmax3[i] = 0.;
		fmaxindex1[i] = 0;
		fmaxindex2[i] = 0;
		fmaxindex3[i] = 0;
		fwidth[i] = 0.;
		flength[i] = 0.;
		fntubes[i] = 0;
		fmeanPedvar_ImageT[i] = 0.;
		fnsat[i] = 0;
		fnlowgain[i] = 0;
		falpha[i] = 0.;
		flos[i] = 0.;
		fasym[i] = 0.;
		fcen_x[i] = 0.;
		fcen_y[i] = 0.;
		fcosphi[i] = 0.;
		fsinphi[i] = 0.;
		ftgrad_x[i] = 0.;
		fFitstat[i] = 0;
		ftchisq_x[i] = 0.;
		fR[i] = 0.;
		fR_telType[i] = 0.;
		ftmscw[i] = 0.;
		ftmscl[i] = 0.;
		ftmscw_sigma[i] = 0.;
		ftmscl_sigma[i] = 0.;
		fE[i] = 0.;
		fES[i] = 0.;
	}
	for( unsigned int i = 0; i < 25; i++ )
	{
		ftheta2_All[i] = 99.;
	}
	fnmscw = 0;
	fmscw = 0.;
	fmscl = 0.;
	fmwr  = 0.;
	fmlr  = 0.;
	fenergy = 0.;
	fechi2 = 0.;
	fdE = 0.;
	fesys = 0.;
	fesysVar = 0.;
	fesysDist = 0.;
	fenergyS = 0.;
	fechi2S = 0.;
	fdES = 0.;
	fEmissionHeightMean = 0.;
	fEmissionHeightChi2 = 0.;
	fNTelPairs = 0;
	for( unsigned int i = 0; i < getMaxNbrTel(); i++ )
	{
		fEmissionHeightT[i] = 0.;
	}
	fSizeSecondMax = 0.;
	
	fTotalTime = 0.;
	fTotalTime0 = 0.;
	
	fMC_distance_to_cameracenter_min = 0.;
	fMC_distance_to_cameracenter_max = 1.e10;
	fDispDiff = 0;
	// Model3D parameters
	fSmax3D = 0;
	fsigmaL3D = 0;
	fsigmaT3D = 0;
	fNc3D = 0;
	fXcore3D = 0;
	fYcore3D = 0;
	fXoff3D = 0;
	fYoff3D = 0;
	fXoffDeRot3D = 0;
	fYoffDeRot3D = 0;
	fGoodness3D = 0;
	fDepth3D = 0;
	fRWidth3D = 0;
	fErrRWidth3D = 0;
	fConverged3D = false;
	
	// cut efficiency counter
	fNStats_All = 0;
	fNStats_Rec = 0;
	fNStats_NImagesCut = 0;
	fNStats_Chi2Cut = 0;
	fNStats_CoreErrorCut = 0;
	fNStats_WobbleCut = 0;
	fNStats_WobbleMinCut = 0;
	fNStats_WobbleMaxCut = 0;
}


/*!
  apply cuts on successfull reconstruction to input data
*/
bool VTableLookupDataHandler::cut( bool bWrite )
{
	// require at least two images
	if( fNImages < ( int )fTLRunParameter->fTableFillingCut_NImages_min )
	{
		fNStats_NImagesCut++;
		return false;
	}
	
	// number of reconstructed events
	fNStats_Rec++;
	
	if( getWobbleOffset() > fTLRunParameter->fTableFillingCut_WobbleCut_max )
	{
		fNStats_WobbleCut++;
		return false;
	}
	
	if( bWrite )
	{
		if( fMCxoff * fMCxoff + fMCyoff * fMCyoff < fMC_distance_to_cameracenter_min * fMC_distance_to_cameracenter_min )
		{
			fNStats_WobbleMinCut++;
			return false;
		}
		if( fMCxoff * fMCxoff + fMCyoff * fMCyoff > fMC_distance_to_cameracenter_max * fMC_distance_to_cameracenter_max )
		{
			fNStats_WobbleMaxCut++;
			return false;
		}
	}
	
	return true;
}


bool VTableLookupDataHandler::randomSelected()
{
	// random event selection
	if( fSelectRandom > 0. )
	{
		if( fRandom->Uniform() > fSelectRandom )
		{
			return false;
		}
	}
	return true;
}


void VTableLookupDataHandler::setSelectRandom( double iF, int iS )
{
	if( iF > 1. )
	{
		cout << "VTableLookupDataHandler::setSelectRandom error: random selector outside intervall [0,1]: " << iF << endl;
		exit( 0 );
	}
	
	fSelectRandom = iF;
	fSelectRandomSeed = iS;
	fRandom->SetSeed( iS );
}


/*
   calculates mean noise level over all telescopes with a valid image

   can use current noise level from time dependent pedestal variations
*/
double VTableLookupDataHandler::calculateMeanNoiseLevel( bool bCurrentNoiseLevel )
{
	double z = 0.;
	double m = 0.;
	
	// time dependent pedestal variations
	if( bCurrentNoiseLevel )
	{
		// use bit coded image list for older showerpars tree format or for small total numbers of telescopes
		if( fEventDisplayFileFormat < 7 || getNTel() < 5 )
		{
			bitset<8 * sizeof( unsigned long )> i_nimage( fImgSel );
			for( unsigned int i = 0; i < fCurrentNoiseLevel.size(); i++ )
			{
				if( i >= i_nimage.size() )
				{
					cout << "ERROR: too many telescope for calculateMeanNoiseLevel: " << i << "\t" << i_nimage.size() << endl;
					cout << "\t " << fImgSel << endl;
					continue;
				}
				if( fCurrentNoiseLevel[i] > 0. && i_nimage.test( i ) )
				{
					m += fCurrentNoiseLevel[i];
					z++;
				}
			}
		}
		// this should work even for very large telescopes
		else
		{
			for( unsigned int i = 0; i < fCurrentNoiseLevel.size(); i++ )
			{
				if( fCurrentNoiseLevel[i] > 0. && fImgSel_list[i] )
				{
					m += fCurrentNoiseLevel[i];
					z++;
				}
			}
		}
	}
	// pedestal variations are constant over whole run
	else
	{
		for( unsigned int i = 0; i < fNoiseLevel.size(); i++ )
		{
			if( fNoiseLevel[i] > 0. )
			{
				m += fNoiseLevel[i];
				z++;
			}
		}
	}
	if( z > 0. )
	{
		return m / z;
	}
	
	return 0.;
}


void VTableLookupDataHandler::setNEntries( int iN )
{
	if( ( iN < fNEntries && iN > 0 ) || fNEntries == 0 )
	{
		fNEntries = iN;
	}
}


/*
   return most propable telescope elevation (majority vote)
*/
double VTableLookupDataHandler::getTelElevation()
{
	vector< unsigned int > i_votes( getNTel(), 0 );
	
	for( unsigned int i = 0; i < getNTel(); i++ )
	{
		for( unsigned int j = 0; j < getNTel(); j++ )
		{
			if( i != j )
			{
				if( TMath::Abs( fTelElevation[i] - fTelElevation[j] ) < 0.2 )
				{
					i_votes[i]++;
				}
			}
		}
	}
	// get telescope with maximum votes
	unsigned int i_max = 0;
	for( unsigned int i = 0; i < i_votes.size(); i++ )
	{
		if( i_votes[i] > i_votes[i_max] )
		{
			i_max = i;
		}
	}
	
	cout << "\treading telescope elevation from telescope " << i_max + 1 << " (from vote casting): ";
	cout << fTelElevation[i_max] << " deg" << endl;
	
	return fTelElevation[i_max];
}

double* VTableLookupDataHandler::getDistanceToCore( ULong64_t iTelType )
{
	unsigned int z = 0;
	for( unsigned int i = 0; i < getNTel(); i++ )
	{
		if( fTel_type[i] == iTelType )
		{
			fR_telType[z] = fR[i];
			z++;
		}
	}
	return fR_telType;
}

// Old classes (global one-for-all-telescope scaling)
double* VTableLookupDataHandler::getSize( double iSizeCorrection, bool iSelectedImagesOnly, bool iSize2 )
{
	for( unsigned int i = 0; i < getNTel(); i++ )
	{
		if( iSelectedImagesOnly && !fImgSel_list[i] )
		{
			fsizeCorr[i] = -99.;
			continue;
		}
		if( !iSize2 )
		{
			fsizeCorr[i] = fsize[i] * iSizeCorrection;
		}
		else
		{
			fsizeCorr[i] = fsize2[i] * iSizeCorrection;
		}
	}
	return fsizeCorr;
}

double* VTableLookupDataHandler::getSize( double iSizeCorrection,  ULong64_t iTelType, bool iSelectedImagesOnly, bool iSize2 )
{
	unsigned int z = 0;
	for( unsigned int i = 0; i < getNTel(); i++ )
	{
		if( fTel_type[i] == iTelType )
		{
			if( iSelectedImagesOnly && !fImgSel_list[i] )
			{
				fsize_telType[z] = -99.;
				z++;
				continue;
			}
			if( !iSize2 )
			{
				fsize_telType[z] = fsize[i] * iSizeCorrection;
			}
			else
			{
				fsize_telType[z] = fsize2[i] * iSizeCorrection;
			}
			z++;
		}
	}
	return fsize_telType;
}

double* VTableLookupDataHandler::getSize( vector<double> iSizeCorrection, bool iSelectedImagesOnly, bool iSize2 )
{
	for( unsigned int i = 0; i < getNTel(); i++ )
	{
		if( iSelectedImagesOnly && !fImgSel_list[i] )
		{
			fsizeCorr[i] = -99.;
			continue;
		}
		if( !iSize2 )
		{
			fsizeCorr[i] = fsize[i] * iSizeCorrection[i];
		}
		else
		{
			fsizeCorr[i] = fsize2[i] * iSizeCorrection[i];
		}
	}
	return fsizeCorr;
}

double* VTableLookupDataHandler::getSize( vector<double> iSizeCorrection,  ULong64_t iTelType, bool iSelectedImagesOnly, bool iSize2 )
{
	unsigned int z = 0;
	for( unsigned int i = 0; i < getNTel(); i++ )
	{
		if( fTel_type[i] == iTelType )
		{
			if( iSelectedImagesOnly && !fImgSel_list[i] )
			{
				fsize_telType[z] = -99.;
				z++;
				continue;
			}
			if( !iSize2 )
			{
				fsize_telType[z] = fsize[i] * iSizeCorrection[i];
			}
			else
			{
				fsize_telType[z] = fsize2[i] * iSizeCorrection[i];
			}
			z++;
		}
	}
	return fsize_telType;
}

double* VTableLookupDataHandler::getWidth( ULong64_t iTelType )
{
	unsigned int z = 0;
	for( unsigned int i = 0; i < getNTel(); i++ )
	{
		if( fTel_type[i] == iTelType )
		{
			fwidth_telType[z] = fwidth[i];
			z++;
		}
	}
	return fwidth_telType;
}

double* VTableLookupDataHandler::getLength( ULong64_t iTelType )
{
	unsigned int z = 0;
	for( unsigned int i = 0; i < getNTel(); i++ )
	{
		if( fTel_type[i] == iTelType )
		{
			flength_telType[z] = flength[i];
			z++;
		}
	}
	return flength_telType;
}

double* VTableLookupDataHandler::getDistance( ULong64_t iTelType )
{
	unsigned int z = 0;
	for( unsigned int i = 0; i < getNTel(); i++ )
	{
		if( fTel_type[i] == iTelType )
		{
			fdist_telType[z] = fdist[i];
			z++;
		}
	}
	return fdist_telType;
}

unsigned int VTableLookupDataHandler::getTelType_arraycounter( unsigned int iTelID )
{
	if( iTelID < fTel_type_counter.size() )
	{
		return fTel_type_counter[iTelID];
	}
	
	return 999999;
}

void VTableLookupDataHandler::initializeTelTypeVector()
{
	fTel_type_counter.clear();
	for( unsigned int iTelID = 0; iTelID < fNTel; iTelID++ )
	{
		unsigned int z = 0;
		for( fList_of_Tel_type_iterator = fList_of_Tel_type.begin();
				fList_of_Tel_type_iterator != fList_of_Tel_type.end(); fList_of_Tel_type_iterator++ )
		{
			if( fTel_type[iTelID] == fList_of_Tel_type_iterator->first )
			{
				fTel_type_counter.push_back( z );
			}
			z++;
		}
	}
}

