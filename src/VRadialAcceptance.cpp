/*! \class VRadialAcceptance
 *  \brief radial acceptance for a given point on the sky
 *
 *
 *    \author
 *    Gernot Maier
 */

#include "VRadialAcceptance.h"

/*!
 *   use acceptance curve from simulation
 */
VRadialAcceptance::VRadialAcceptance()
{
	reset();
	
	fAcceptanceFunctionDefined = false;
}


/*!
 *  ******************************************************************
 *          reading acceptance curves
 *  ******************************************************************
 *  this constructor is called from anasum to get acceptance for a specific event
 */
VRadialAcceptance::VRadialAcceptance( string ifile )
{
	reset();
	
	ifile = VUtilities::testFileLocation( ifile, "RadialAcceptances/", true );
	if( ifile.size() == 0 )
	{
		exit( 0 );
	}
	
	fAccFile = new TFile( ifile.c_str() );
	if( fAccFile->IsZombie() )
	{
		cout << "VRadialAcceptance::VRadialAcceptance error reading acceptance file " << ifile << endl;
		exit( -1 );
	}
	
	char hname[200];
	int i = 0;
	for( ;; )
	{
		sprintf( hname, "fAccZe_%d", i );
		TF1* iF = ( TF1* )gDirectory->Get( hname );
		if( iF )
		{
			fAccZe.push_back( iF );
			fAcceptanceFunctionDefined = true;
		}
		else
		{
			break;
		}
		i++;
	}
	cout << "\t total number of acceptance curves: " << fAccZe.size() << " (found in " << ifile;
	
	// count number of raw files used to calculate acceptances
	TKey* key;
	TIter nextkey( fAccFile->GetListOfKeys() );
	while( ( key = ( TKey* )nextkey() ) )
	{
		TObject* obj = key->ReadObj();
		string itemp = obj->GetName();
		if( itemp.find( "hAccRun_" ) < itemp.size() )
		{
			fNumberOfRawFiles++;
		}
	}
	cout << ", calculated from " << fNumberOfRawFiles << " files)" << endl;
}


/*!

 *  ******************************************************************
 *          calculating acceptance curves
 *  ******************************************************************
 *   this constructor is called for determination of radial acceptance curves with makeAcceptance

 */
VRadialAcceptance::VRadialAcceptance( VGammaHadronCuts* icuts, VAnaSumRunParameter* irunpar )
{
	reset();
	
	hList = new TList();
	hListNormalizeHistograms = new TList();
	hListFitHistograms = new TList();
	
	fCuts = icuts;
	if( !fCuts )
	{
		cout << "VRadialAcceptance error: no gamma/hadron separation cuts defined" << endl;
		cout << "exiting..";
		exit( -1 );
	}
	// maximum distance to camera center for which events are taken into account:
	fCut_CameraFiducialSize_max = fCuts->fCut_CameraFiducialSize_max;
	
	fRunPar = irunpar;
	
	// upper limit for zenith angle interal (18 == [0,18.])
	/*   fZe.push_back( 20. );
	   fZe.push_back( 30. );
	   fZe.push_back( 40. ); */
	fZe.push_back( 70. );
	
	// maximal offset
	double xymax = 5.0;
	int nxybin = 50;
	
	// range used to normalise acceptance histograms
	fAccZeFitMinBin = 2;
	fAccZeFitMaxBin = 5;
	
	hscale = new TH1F( "hscale", "", nxybin, 0., xymax );
	for( int i = 1; i < nxybin; i++ )
	{
		hscale->SetBinContent( i, TMath::Pi()*xymax * xymax / ( ( double )( nxybin * nxybin ) ) * ( 2 * i - 1 ) );
	}
	for( int i = 1; i < nxybin; i++ )
	{
		hscale->SetBinError( i, 0. );
	}
	hList->Add( hscale );
	
	hPhiDist = new TH1F( "hPhiDist", "", nxybin, -180., 180. );
	hPhiDist->SetXTitle( "azimuth (camera coordinates) [deg]" );
	hList->Add( hPhiDist );
	hPhiDistDeRot = new TH1F( "hPhiDistDeRot", "", nxybin, -180., 180. );
	hPhiDistDeRot->SetXTitle( "azimuth (derotated camera coordinates) [deg]" );
	hList->Add( hPhiDistDeRot );
	
	char hname[200];
	char htitle[200];
	double ize = 0.;
	for( unsigned int i = 0; i < fZe.size(); i++ )
	{
		cout << "zenith angle interval: ";
		if( i == 0 )
		{
			cout << 0;
		}
		else
		{
			cout << fZe[i - 1];
		}
		cout << " - " << fZe[i] << endl;
		
		sprintf( hname, "hAccZe_%d", i );
		sprintf( htitle, "%.0f < el < %.0f", 90. - fZe[i], 90. - ize );
		hAccZe.push_back( new TH1F( hname, htitle, nxybin, 0., xymax ) );
		hAccZe.back()->SetXTitle( "distance to camera center [deg]" );
		hAccZe.back()->SetYTitle( "relative rate" );
		hAccZe.back()->SetMarkerSize( 2 );
		hAccZe.back()->SetLineWidth( 2 );
		hAccZe.back()->Sumw2();
		hList->Add( hAccZe.back() );
		hListNormalizeHistograms->Add( hAccZe.back() );
		hListFitHistograms->Add( hAccZe.back() );
		
		ize = fZe[i];
	}
	// 2D histogram (not normalized)
	
	double hr = 3.0 ;
	int hn = 40 ;
	hXYAccTot = new TH2F( "hXYaccTot", "",  hn, -hr, hr, hn, -hr, hr );
	hXYAccTot->SetXTitle( "x_{off} [deg]" );
	hXYAccTot->SetYTitle( "y_{off} [deg]" );
	hXYAccTot->Sumw2();
	hList->Add( hXYAccTot );
	hXYAccTotDeRot = new TH2F( "hXYAccTotDeRot", "",  hn, -hr, hr, hn, -hr, hr );
	hXYAccTotDeRot->SetXTitle( "x_{off,derot} [deg]" );
	hXYAccTotDeRot->SetYTitle( "y_{off,derot} [deg]" );
	hXYAccTotDeRot->Sumw2();
	hList->Add( hXYAccTotDeRot );
	
	// azimuth dependent radial acceptance histograms
	fPhiMin.clear();
	fPhiMax.clear();
	fPhiMin.push_back( 135.0 );
	fPhiMax.push_back( -165.0 );
	fPhiMin.push_back( 150.0 );
	fPhiMax.push_back( -150.0 );
	fPhiMin.push_back( -180. );
	fPhiMax.push_back( -120. );
	for( int i = 0; i < 13; i++ )
	{
		fPhiMin.push_back( fPhiMin.back() + 22.5 );
		fPhiMax.push_back( fPhiMax.back() + 22.5 );
	}
	for( unsigned int i = 0; i < fPhiMin.size(); i++ )
	{
		// camera coordinates
		sprintf( hname, "hAccPhi_%d", i );
		sprintf( htitle, "%.0f < Phi < %.0f", fPhiMin[i], fPhiMax[i] );
		hAccPhi.push_back( new TH1F( hname, htitle, nxybin, 0., xymax ) );
		hAccPhi.back()->SetXTitle( "distance to camera center [deg]" );
		hAccPhi.back()->SetYTitle( "relative rate" );
		hAccPhi.back()->SetMarkerSize( 2 );
		hAccPhi.back()->SetLineWidth( 2 );
		hAccPhi.back()->Sumw2();
		hList->Add( hAccPhi.back() );
		hListNormalizeHistograms->Add( hAccPhi.back() );
		hListFitHistograms->Add( hAccPhi.back() );
		// derotated camera coordinates
		sprintf( hname, "hAccPhiDerot_%d", i );
		sprintf( htitle, "%.0f < Phi < %.0f (derot)", fPhiMin[i], fPhiMax[i] );
		hAccPhiDerot.push_back( new TH1F( hname, htitle, nxybin, 0., xymax ) );
		hAccPhiDerot.back()->SetXTitle( "distance to camera center [deg]" );
		hAccPhiDerot.back()->SetYTitle( "relative rate" );
		hAccPhiDerot.back()->SetMarkerSize( 2 );
		hAccPhiDerot.back()->SetLineWidth( 2 );
		hAccPhiDerot.back()->Sumw2();
		hList->Add( hAccPhiDerot.back() );
		hListNormalizeHistograms->Add( hAccPhiDerot.back() );
		hListFitHistograms->Add( hAccPhiDerot.back() );
	}
	
	
	// for PhiDependentSlice hists
	phi_minphi = 0.0 ;
	phi_maxphi = 2 * TMath::Pi() ;
	phi_nbins  = 18 ;
	phi_minradius = 1.25 ; // band will be from 1.25 deg to 1.75 deg
	phi_maxradius = 1.75 ;
	
	// for RadiusDependentSlice hists
	rad_minrad = 0.0 ;
	rad_maxrad = 2.0 ;
	rad_nbins  = 12  ;
	rad_phiwidth = TMath::Pi() / 6 ; // Slice000 will be from 330 deg to 30 deg, etc
	int centerdeg = 0;
	
	// global 1D Slice Histograms
	sprintf( hname, "hXYAccTotDeRotPhiDependentSlice" ) ;
	sprintf( htitle, "1D histogram from Xoff_derot and Yoff_derot, with All Events, PhiDependentSlice (doughnut)\n, %d Bins from %3.1f to %3.1f radians, from Radius %4.2f to %4.2f deg", phi_nbins, phi_minphi, phi_maxphi, phi_minradius, phi_maxradius ) ;
	hXYAccTotDeRotPhiDependentSlice = new TH1F( hname, htitle, phi_nbins, phi_minphi, phi_maxphi ) ;
	
	sprintf( hname, "hXYAccTotDeRotRadiusDependentSlice" ) ;
	sprintf( htitle, "1D histogram from Xoff_derot and Yoff_derot, with All Events, RadiusDependentSlice%03d (Pie Slice)\n, from Phi %3.1f to %3.1f radians", centerdeg, centerdeg - ( rad_phiwidth * 180 / 3.1415 ), centerdeg + ( rad_phiwidth * 180 / 3.1415 ) ) ;
	hXYAccTotDeRotRadiusDependentSlice000 = new TH1F( hname, htitle, rad_nbins, rad_minrad, rad_maxrad ) ;
	
	// 2D histograms, sorted by ImgSel
	TH2F* tmphist2 ;
	TH1F* tmphist1 ;
	for( int i = 0 ; i <= 15 ; i++ )
	{
		sprintf( hname, "hAccImgSel_%d", i ) ;
		sprintf( htitle, "2D histogram of Xoff_derot and Yoff_derot, with ImgSel=%d", i ) ;
		tmphist2 = new TH2F( hname, htitle, hn, -1.0 * hr, hr, hn, -1.0 * hr, hr ) ;
		hXYAccImgSel.push_back( tmphist2 ) ;
		
		sprintf( hname, "hAccImgSelPreDeRot_%d", i ) ;
		sprintf( htitle, "2D histogram of Xoff and Yoff, with ImgSel=%d", i ) ;
		tmphist2 = new TH2F( hname, htitle, hn, -1.0 * hr, hr, hn, -1.0 * hr, hr ) ;
		hXYAccImgSelPreDeRot.push_back( tmphist2 ) ;
		
		sprintf( hname, "hAccImgSelPhiDependentSlice_%d", i ) ;
		sprintf( htitle, "1D histogram from Xoff_derot and Yoff_derot, with ImgSel=%d, PhiDependentSlice\n, %d Bins from %3.1f to %3.1f radians, from Radius %4.2f to %4.2f deg", i, phi_nbins, phi_minphi, phi_maxphi, phi_minradius, phi_maxradius ) ;
		tmphist1 = new TH1F( hname, htitle, phi_nbins, phi_minphi, phi_maxphi ) ;
		hXYAccImgSelPhiDependentSlice.push_back( tmphist1 ) ;
		
		sprintf( hname, "hAccImgSelRadiusDependentSlice%03d_%d", centerdeg, i ) ;
		sprintf( htitle, "1D histogram from Xoff_derot and Yoff_derot, with ImgSel=%d, RadiusDependentSlice%03d (Pie Slice)\n, from Phi %3.1f to %3.1f radians", i, centerdeg, centerdeg - ( rad_phiwidth * 180 / 3.1415 ), centerdeg + ( rad_phiwidth * 180 / 3.1415 ) ) ;
		tmphist1 = new TH1F( hname, htitle, rad_nbins, rad_minrad, rad_maxrad ) ;
		hXYAccImgSelRadiusDependentSlice000.push_back( tmphist1 ) ;
		
	}
	
	
	// 2D histograms, sorted by NImages
	for( int i = 0 ; i <= 4 ; i++ )
	{
		sprintf( hname, "hAccNImages_%d", i ) ;
		sprintf( htitle, "2D histogram of Xoff_derot and Yoff_derot, with NImages=%d", i ) ;
		tmphist2 = new TH2F( hname, htitle, hn, -1.0 * hr, hr, hn, -1.0 * hr, hr ) ;
		hXYAccNImages.push_back( tmphist2 ) ;
		
		sprintf( hname, "hAccNImagesPreDeRot_%d", i ) ;
		sprintf( htitle, "2D histogram of Xoff and Yoff, with NImages=%d", i ) ;
		tmphist2 = new TH2F( hname, htitle, hn, -1.0 * hr, hr, hn, -1.0 * hr, hr ) ;
		hXYAccNImagesPreDeRot.push_back( tmphist2 ) ;
		
		sprintf( hname, "hAccNImagesPhiDependentSlice_%d", i ) ;
		sprintf( htitle, "1D histogram from Xoff_derot and Yoff_derot, with NImages=%d, PhiDependentSlice\n, %d Bins from %3.1f to %3.1f radians, from Radius %4.2f to %4.2f deg", i, phi_nbins, phi_minphi, phi_maxphi, phi_minradius, phi_maxradius ) ;
		tmphist1 = new TH1F( hname, htitle, phi_nbins, phi_minphi, phi_maxphi ) ;
		hXYAccNImagesPhiDependentSlice.push_back( tmphist1 ) ;
		
		sprintf( hname, "hAccNImagesRadiusDependentSlice%03d_%d", centerdeg, i ) ;
		sprintf( htitle, "2D histogram of Xoff_derot and Yoff_derot, with NImages=%d, RadiusDependentSlice%03d (Pi Slice)\n from %3.1f to %3.1f radians", i, centerdeg, centerdeg - ( rad_phiwidth * 180 / 3.1415 ), centerdeg + ( rad_phiwidth * 180 / 3.1415 ) ) ;
		tmphist1 = new TH1F( hname, htitle, rad_nbins, rad_minrad, rad_maxrad ) ;
		hXYAccNImagesRadiusDependentSlice000.push_back( tmphist1 ) ;
		
	}
	
	// run dependent acceptance curves
	for( unsigned int i = 0; i < fRunPar->fRunList.size(); i++ )
	{
		sprintf( hname, "hAccRun_%d", fRunPar->fRunList[i].fRunOff );
		sprintf( htitle, "run %d", fRunPar->fRunList[i].fRunOff );
		hAccRun.push_back( new TH1F( hname, htitle, nxybin, 0., xymax ) );
		hAccRun.back()->SetXTitle( "distance to camera center [deg]" );
		hAccRun.back()->SetYTitle( "relative rate" );
		hAccRun.back()->SetMarkerSize( 2 );
		hAccRun.back()->SetLineWidth( 2 );
		hAccRun.back()->Sumw2();
		hList->Add( hAccRun.back() );
		hListNormalizeHistograms->Add( hAccRun.back() );
		
		sprintf( hname, "hXYAccRun_%d", fRunPar->fRunList[i].fRunOff );
		sprintf( htitle, "run %d", fRunPar->fRunList[i].fRunOff );
		hXYAccRun.push_back( new TH2F( hname, htitle, 40, -2., 2., 40, -2., 2. ) );
		hXYAccRun.back()->SetXTitle( "x_{off} [deg]" );
		hXYAccRun.back()->SetYTitle( "y_{off} [deg]" );
		hList->Add( hXYAccRun.back() );
	}
	
	
}


VRadialAcceptance::~VRadialAcceptance()
{
	if( fAccFile )
	{
		delete fAccFile;
	}
}


void VRadialAcceptance::reset()
{
	fNumberOfRawFiles = 0.;
	
	fAcceptanceFunctionDefined = false;
	
	fRunPar = 0;
	fCuts = 0;
	hList = 0;
	hListNormalizeHistograms = 0;
	hListFitHistograms = 0;
	
	fXs = 0.;
	fYs = 0.;
	fRs = 0.;
	fDs = 0.;
	fMaxDistanceAllowed = 5.;
	fCut_CameraFiducialSize_max = fMaxDistanceAllowed;
	
	hscale = 0;
	hPhiDist = 0;
	hPhiDistDeRot = 0;
	hXYAccTot = 0;
	hXYAccTotDeRot = 0;
	fAccFile = 0;
	
	f2DAcceptanceMode = 0 ;
	f2DBinNormalizationConstant = 0 ;
	
	eventcount = 0 ;
	
	fXE.clear();
	fYE.clear();
	fRE.clear();
	
	hXYAccImgSel.clear();
	hXYAccNImages.clear();
	hXYAccImgSelPreDeRot.clear();
	hXYAccNImagesPreDeRot.clear();
	
	setEnergyReconstructionMethod();
	setAzCut();
	
	fExtraHistogramMode = 0 ;
	fExtraHistogramDir = "" ;
}


/*!

    get radial acceptance

    (ignore here any zenith angle acceptance)

    note: x,y are in derotated coordinates
 */
double VRadialAcceptance::getAcceptance( double x, double y, double erec, double ze )
{

	if( f2DAcceptanceMode == 0 )  // then we give a normal, 1D radial acceptance
	{
	
		double idist = sqrt( x * x + y * y );
		double iacc = 1.;
		
		if( fAcceptanceFunctionDefined && fAccZe.size() > 0 )
		{
			if( idist > fAccZe[0]->GetXmax() )
			{
				iacc = 0.;
			}
			else
			{
				iacc = fAccZe[0]->Eval( idist );
			}
			if( iacc > 1. )
			{
				iacc = 1.;
			}
			if( iacc < 0. )
			{
				iacc = 0.;
			}
		}
		
		return iacc;
		
	}
	// USE2DACCEPTANCE
	else if( f2DAcceptanceMode == 1 )  // then we give an acceptance based on 2D histogram in Xoff_derot and Yoff_derot
	{
		double iacc = 1.0 ;
		int xbin = hXYAccTotDeRot->GetXaxis()->FindBin( x ) ;
		int ybin = hXYAccTotDeRot->GetYaxis()->FindBin( y ) ;
		iacc = hXYAccTotDeRot->GetBinContent( xbin, ybin ) / f2DBinNormalizationConstant ;
		return iacc;
	}
	else
	{
		cout << "ERROR: getAcceptance() not defined for f2DAcceptanceMode > 0 " << endl;
		exit( -1 ) ;
		return -1000.0 ;
	}
	
	
}


/*!
 *  correction factor is 1/acceptance
 *
 */
double VRadialAcceptance::getCorrectionFactor( double x, double y, double erec )
{
	double r = getAcceptance( x, y, erec );
	if( r > 0. )
	{
		return 1. / r;
	}
	
	return 0.;
}


/*!
 *    define here region in the sky which are excluded in the analysis
 *
 *     x,y are camera coordinates (not wobble shifted)
 */
bool VRadialAcceptance::isExcluded( double x, double y )
{
	if( ( x * x + y * y ) > fMaxDistanceAllowed * fMaxDistanceAllowed )
	{
		return true;
	}
	
	return false;
}


/*!
 *    define here regions which are to be excluded from background analysis
 *
 *    - source region
 *    - out of camera
 *    - other excluded regions
 *
 *     x,y are camera coordinates (not wobble shifted: relative to the camera center)
 */
bool VRadialAcceptance::isExcludedfromBackground( double x, double y )
{
	// event outside fiducial area
	if( isExcluded( x, y ) )
	{
		return true;
	}
	
	// source region (source radius + safety (fDS))
	if( fDs >= 0. && ( ( x - fXs ) * ( x - fXs ) + ( y - fYs ) * ( y - fYs ) ) < ( ( fRs + fDs ) * ( fRs + fDs ) ) )
	{
		return true;
	}
	
	//Other regions to exclude from background (read from runparameter)
	for( unsigned int i = 0; i < fXE.size(); i ++ )
	{
		if( ( x - fXE[i] ) * ( x - fXE[i] ) + ( y - fYE[i] ) * ( y - fYE[i] ) < fRE[i]*fRE[i] )
		{
			return true;
		}
	}
	return false;
}


/*
 *     x,y are camera coordinates (not wobble shifted)
 */
bool VRadialAcceptance::isExcludedfromSource( double x, double y )
{
	return isExcluded( x, y );
}

/*

   set the position of the potential gamma-ray source in camera coordinates

*/
void VRadialAcceptance::setSource( double x, double y, double r, double idist, double imaxdist )
{
	fXs = x;
	fYs = y;
	fRs = r;
	fDs = idist;
	fMaxDistanceAllowed = imaxdist;
}

void VRadialAcceptance::setRegionToExcludeAcceptance( vector<double> x, vector<double> y, vector<double> r )
{
	fXE = x;
	fYE = y;
	fRE = r;
	if( fXE.size() != fYE.size() || fXE.size() != fRE.size() )
	{
		cout << "VRadialAcceptance::setRegionToExcludeAcceptance: error: vectors of exclusion regions have different size: ";
		cout << fXE.size() << " " << fYE.size() << " " << fRE.size() << endl;
	}
}

/*

    apply gamma/hadron cuts and fill radial acceptance histograms

*/
int VRadialAcceptance::fillAcceptanceFromData( CData* iData, int entry )
{
	if( !iData )
	{
		cout << "VRadialAcceptance::fillAcceptanceFromData: no data tree defined" << endl;
		return -1;
	}
	
	double idist = 0;
	double i_Phi = 0.;
	bool bPassed = false;
	
	// apply some basic quality cuts
	if( fCuts->applyInsideFiducialAreaCut() && fCuts->applyStereoQualityCuts( fEnergyReconstructionMethod, false, entry, true ) )
	{
		// gamma/hadron cuts
		if( !fCuts->isGamma( entry, false ) )
		{
			return 0;
		}
		
		// az cut
		bool bFill = false;
		if( fAzCut_min < fAzCut_max )
		{
			if( iData->Az > fAzCut_min && iData->Az <= fAzCut_max )
			{
				bFill = true;
			}
		}
		else
		{
			if( iData->Az < fAzCut_max || iData->Az > fAzCut_min )
			{
				bFill = true;
			}
		}
		if( !bFill )
		{
			return 0;
		}
		// no more cuts after this statement
		bPassed = true;
		
		idist = sqrt( iData->Xoff * iData->Xoff + iData->Yoff * iData->Yoff );
		
		// fill 2D distribution of events
		hXYAccTot->Fill( iData->Xoff, iData->Yoff );
		hXYAccTotDeRot->Fill( iData->Xoff_derot, iData->Yoff_derot );
		
		hXYAccImgSel[iData->ImgSel]->Fill( iData->Xoff_derot, iData->Yoff_derot ) ;
		hXYAccImgSelPreDeRot[iData->ImgSel]->Fill( iData->Xoff, iData->Yoff ) ;
		hXYAccNImages[iData->NImages]->Fill( iData->Xoff_derot, iData->Yoff_derot ) ;
		hXYAccNImagesPreDeRot[iData->NImages]->Fill( iData->Xoff, iData->Yoff ) ;
		
		// 1D histograms
		// Radius Dependent Histograms
		eventradius = sqrt( iData->Xoff_derot * iData->Xoff_derot + iData->Yoff_derot * iData->Yoff_derot ) ;
		eventphi    = atan2( iData->Yoff_derot, iData->Xoff_derot ) ; // radians
		if( eventphi < 0.0 )
		{
			eventphi += 2 * TMath::Pi() ;    // atan2 is from -pi to pi, we want 0 to 2pi
		}
		
		// PhiDependentSlice Fill
		if( eventradius > phi_minradius && eventradius < phi_maxradius )
		{
			hXYAccTotDeRotPhiDependentSlice->Fill( eventphi ) ;
			hXYAccImgSelPhiDependentSlice[iData->ImgSel]->Fill( eventphi ) ;
			hXYAccNImagesPhiDependentSlice[iData->NImages]->Fill( eventphi ) ;
		}
		
		// RadiusDependentSlice000 Fill
		if( eventphi > 0.0 - rad_phiwidth && eventphi < 0.0 + rad_phiwidth )
		{
			hXYAccTotDeRotRadiusDependentSlice000->Fill( eventradius ) ;
			hXYAccImgSelRadiusDependentSlice000[iData->ImgSel]->Fill( eventradius ) ;
			hXYAccNImagesRadiusDependentSlice000[iData->NImages]->Fill( eventradius ) ;
		}
		
		// fill zenith angle dependent histograms
		for( unsigned int j = 0; j < fZe.size(); j++ )
		{
			if( iData->Ze < fZe[j] )
			{
				if( idist > 0. )
				{
					hAccZe[j]->Fill( idist );
				}
				break;
			}
		}
		
		// fill azimuth angle dependend histograms (camera coordinates)
		i_Phi = atan2( iData->Yoff, iData->Xoff ) * TMath::RadToDeg();
		hPhiDist->Fill( i_Phi );
		
		for( unsigned int j = 0; j < fPhiMin.size(); j++ )
		{
			bFill = false;
			if( i_Phi > fPhiMin[j] && i_Phi < fPhiMax[j] )
			{
				bFill = true;
			}
			else
			{
				if( fPhiMin[j] > fPhiMax[j] )
				{
					if( i_Phi < fPhiMin[j] && i_Phi > fPhiMax[j] )
					{
						bFill = false;
					}
					else
					{
						bFill = true;
					}
				}
			}
			if( bFill && idist > 0. )
			{
				hAccPhi[j]->Fill( idist );
			}
		}
		// fill azimuth angle dependend histograms (derotated camera coordinates)
		i_Phi = atan2( iData->Yoff_derot, iData->Xoff_derot ) * TMath::RadToDeg();
		hPhiDistDeRot->Fill( i_Phi );
		for( unsigned int j = 0; j < fPhiMin.size(); j++ )
		{
			bool bFill = false;
			if( i_Phi > fPhiMin[j] && i_Phi < fPhiMax[j] )
			{
				bFill = true;
			}
			else
			{
				if( fPhiMin[j] > fPhiMax[j] )
				{
					if( i_Phi < fPhiMin[j] && i_Phi > fPhiMax[j] )
					{
						bFill = false;
					}
					else
					{
						bFill = true;
					}
				}
			}
			if( bFill &&  idist > 0. )
			{
				hAccPhiDerot[j]->Fill( idist );
			}
		}
		// fill run dependent histograms
		for( unsigned int j = 0; j < fRunPar->fRunList.size(); j++ )
		{
			if( iData->runNumber == fRunPar->fRunList[j].fRunOff )
			{
				if( idist > 0. && j < hAccRun.size() )
				{
					hAccRun[j]->Fill( idist );
				}
				if( j < hXYAccRun.size() )
				{
					hXYAccRun[j]->Fill( iData->Xoff, iData->Yoff );
				}
				break;
			}
		}
	}
	
	if( bPassed )
	{
		return 1;
	}
	
	return 0;
}

/*

    called for making radial acceptances

*/
bool VRadialAcceptance::terminate( TDirectory* iDirectory )
{
	if( !iDirectory->cd() )
	{
		cout << "VRadialAcceptance::terminate() error accessing directory  " << iDirectory->GetName() << endl;
		exit( -1 );
	}
	/////////////////////////////////////
	// normalize radial acceptance histograms
	// scale everything to mean value of first three bins
	if( fAccZeFitMinBin == fAccZeFitMaxBin )
	{
		cout << "Error: normalisation range for acceptance curves not well defined: " << fAccZeFitMinBin << "\t" << fAccZeFitMaxBin << endl;
	}
	else
	{
		double isc = 0.;
		double i_normBin = ( double )( fAccZeFitMaxBin - fAccZeFitMinBin );
		cout << "VRadialAcceptance::terminate: scaling histograms to bins " << fAccZeFitMinBin << " to " << fAccZeFitMaxBin << endl;
		// scale all histograms in hListNormalizeHistograms
		TIter next( hListNormalizeHistograms );
		while( TH1F* h = ( TH1F* )next() )
		{
			scaleArea( h );
			isc = 0.;
			i_normBin = 0.;
			for( unsigned int j = fAccZeFitMinBin; j < fAccZeFitMaxBin; j++ )
			{
				if( h->GetBinError( j ) > 0. )
				{
					isc +=  h->GetBinContent( j ) / ( h->GetBinError( j ) * h->GetBinError( j ) );
					i_normBin += 1. / h->GetBinError( j ) / h->GetBinError( j );
				}
			}
			if( i_normBin > 0. )
			{
				isc /= i_normBin;
			}
			if( isc > 0 )
			{
				h->Scale( 1. / isc );
			}
		}
	}
	
	
	/////////////////////////////////////
	// analyze and fit histograms
	string i_hname;
	TIter next( hListFitHistograms );
	while( TH1F* h = ( TH1F* )next() )
	{
		// fit function
		i_hname = h->GetName();
		i_hname.replace( 0, 1, "f" );
		TF1* ffit = new TF1( i_hname.c_str(), VRadialAcceptance_fit_acceptance_function, 0., fCut_CameraFiducialSize_max, 5 );
		ffit->SetNpx( 1000 );
		ffit->SetParameter( 0, -0.3 );
		ffit->SetParameter( 1, -0.6 );
		ffit->SetParameter( 2, +0.6 );
		ffit->SetParameter( 3, -0.2 );
		ffit->SetParameter( 4, 0.2 );
		ffit->SetParLimits( 4, 0., 0.5 );
		hList->Add( ffit );
		// fit histogram
		i_hname = h->GetName();
		i_hname += "Fit";
		TH1F* hfit = new TH1F( i_hname.c_str(), h->GetTitle(), h->GetNbinsX(), h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax() );
		hfit->SetXTitle( h->GetXaxis()->GetTitle() );
		hfit->SetYTitle( h->GetYaxis()->GetTitle() );
		hList->Add( hfit );
		
		// fill the fitting histogram
		for( int j = 1; j < hfit->GetNbinsX(); j++ )
		{
			hfit->SetBinContent( j, h->GetBinContent( j ) ) ;
		}
		
		// fit the data and fill histograms
		cout << "fitting acceptance curves (" << h->GetName() << ") ..." << endl << endl;
		double i_eval = 0.;
		hfit->Fit( ffit, "0REM" );
		hfit->SetBins( 1000, 0., 5. );
		// replace bin content by values from the fit function (set max to 1 and min to 0)
		for( int j = 1; j < hfit->GetNbinsX(); j++ )
		{
			i_eval = ffit->Eval( hfit->GetBinCenter( j ) );
			if( hfit->GetBinCenter( j ) > ffit->GetXmax() )
			{
				i_eval = 0.;
			}
			else if( hfit->GetBinCenter( j ) < ffit->GetMaximumX() )
			{
				i_eval = 1.;
			}
			else if( i_eval < 0. )
			{
				i_eval = 0.;
			}
			else if( i_eval >= 1. )
			{
				i_eval = 1.;
			}
			
			hfit->SetBinContent( j, i_eval );
		}
	}
	cout << "-----------------------------------------" << endl << endl;
	
	// write total number of entries in each histogram (ze) to screen
	cout << "total number of events per zenith angle bin " << endl;
	for( unsigned int i = 0; i < hAccZe.size(); i++ )
	{
		cout << "\t" << hAccZe[i]->GetName() << "\t" << hAccZe[i]->GetEntries() << endl;
	}
	
	cout << endl << "writing acceptance curves to " << iDirectory->GetName() << endl;
	
	hList->Write();
	
	// write cuts to disk
	if( fCuts )
	{
		fCuts->SetName( "GammaHadronCuts" );
		fCuts->Write();
	}
	
	return true;
}


void VRadialAcceptance::scaleArea( TH1F* h )
{
	double iA = 0.;
	
	for( int i = 1; i <= h->GetNbinsX(); i++ )
	{
		double iL = h->GetXaxis()->GetBinLowEdge( i );
		double iU = h->GetXaxis()->GetBinLowEdge( i ) + h->GetXaxis()->GetBinWidth( i );
		
		iA = TMath::Pi() * ( iU * iU - iL * iL );
		
		if( iA > 0. )
		{
			h->SetBinContent( i, h->GetBinContent( i ) / iA );
			h->SetBinError( i, h->GetBinError( i ) / iA );
		}
	}
}

/*
   fit function for acceptance curves

   pol5 with x < x0 == 1 and f(x0)' == 0

   p0 = a2
*/
Double_t VRadialAcceptance_fit_acceptance_function( Double_t* x, Double_t* par )
{
	double f = 0.;
	
	double a2 = par[0];
	double a3 = par[1];
	double a4 = par[2];
	double a5 = par[3];
	double x0 = par[4];
	
	double a1 = -1.*( x0 * ( 2.*a2 + x0 * ( 3.*a3 + x0 * ( 4.*a4 + x0 * 5. * a5 ) ) ) );
	double a0 = 1. - ( x0 * ( a1 + x0 * ( a2 + x0 * ( a3 + x0 * ( a4 + x0 * a5 ) ) ) ) );
	
	if( x[0] < x0 )
	{
		f = 1.;
	}
	else
	{
		f = a0 + x[0] * ( a1 + x[0] * ( a2 + x[0] * ( a3 + x[0] * ( a4 + x[0] * a5 ) ) ) );
	}
	return f;
}

/////////////////////////////////////
// get normalization factor for 2d histogram bins
// 2dacceptance = bincontent(Xoff,Yoff)/normfactor
// normfactor = avg bin content from all bins < 0.3 deg from center
// using 2d histogram of derotated Xoff and Yoff CData->Xoff_derot, CData->Yoff_derot
double VRadialAcceptance::calculate2DBinNormalizationConstant( double radius ) // radius in degrees
{
	cout << "VRadialAcceptance::calculate2DBinNormalizationConstant" << endl;
	
	double normconst = 1.0 ;
	int nbinsx = hXYAccTotDeRot->GetNbinsX() ;
	int nbinsy = hXYAccTotDeRot->GetNbinsY() ;
	int nbins = nbinsx * nbinsy ;
	int i_binx, i_biny, i_binz ;
	double bincentx, bincenty, iradius, binCont;
	int avgbincount = 0 ;
	double avgbintotal = 0.0 ;
	
	// loop over all bins in hXYAccTot
	for( int i_bin = 1 ; i_bin <= nbins ; i_bin++ )
	{
		// find bin center and content
		hXYAccTotDeRot->GetBinXYZ( i_bin, i_binx, i_biny, i_binz ) ;
		bincentx = hXYAccTotDeRot->GetXaxis()->GetBinCenter( i_binx ) ;
		bincenty = hXYAccTotDeRot->GetYaxis()->GetBinCenter( i_biny ) ;
		iradius  = sqrt( bincentx * bincentx + bincenty * bincenty ) ;
		binCont  = hXYAccTotDeRot->GetBinContent( i_binx, i_biny ) ;
		if( iradius < radius )  // bin center < 0.3 deg
		{
			avgbintotal += binCont ;
			avgbincount++ ;
		}
	}
	
	// Write extra histograms
	if( fExtraHistogramMode > 0 )
	{
		char buff[250] ;
		cout << "VRadialAcceptance::calculate2DBinNormalizationConstant - writing hists" << endl;
		// write 2d hist to text file
		if( hXYAccTotDeRot != 0 )
		{
			cout << "hXYAccTotDeRot go" << endl;
			sprintf( buff, "%s/2dAcceptanceHist", fExtraHistogramDir.c_str() ) ;
			string outname = buff ;
			Write2DHistToTextFile( hXYAccTotDeRot, outname ) ;
		}
		if( hXYAccTot != 0 )
		{
			cout << "hXYAccTot go" << endl;
			sprintf( buff, "%s/2dAcceptanceHistPreDeRot", fExtraHistogramDir.c_str() ) ;
			string outname = buff ;
			Write2DHistToTextFile( hXYAccTot, outname ) ;
		}
		
		if( hXYAccTotDeRotRadiusDependentSlice000 != 0 )
		{
			sprintf( buff, "%s/2dAcceptanceHist.RadiusDependentSlice000", fExtraHistogramDir.c_str() ) ;
			string buff2 = buff ;
			Write1DHistToTextFile( hXYAccTotDeRotRadiusDependentSlice000, buff2, 1 ) ;
		}
		
		if( hXYAccTotDeRotPhiDependentSlice != 0 )
		{
			sprintf( buff, "%s/2dAcceptanceHist.PhiDependentSlice", fExtraHistogramDir.c_str() ) ;
			string buff2 = buff ;
			Write1DHistToTextFile( hXYAccTotDeRotPhiDependentSlice, buff2, 1 ) ;
		}
		
		int i ;
		// write ImgSel 2d hists to text files
		if( hXYAccImgSel.empty() == 0 )
		{
			cout << "hXYAccImgSel go" << endl;
			for( i = 0 ; i <= 15 ; i++ )
			{
				sprintf( buff, "%s/ImgSel%d", fExtraHistogramDir.c_str(), i ) ;
				string buff2 = buff ;
				Write2DHistToTextFile( hXYAccImgSel[i], buff2 );
			}
		}
		
		// ImgSel RadiusDependentSlice
		if( hXYAccImgSelRadiusDependentSlice000.empty() == 0 )
		{
			cout << "hXYAccImgSelRadiusDependentSlice000 go" << endl;
			for( i = 0 ; i <= 15 ; i++ )
			{
				sprintf( buff, "%s/ImgSel%d.RadiusDependentSlice000", fExtraHistogramDir.c_str(), i ) ;
				string buff2 = buff ;
				Write1DHistToTextFile( hXYAccImgSelRadiusDependentSlice000[i], buff2, 1 ) ;
			}
		}
		
		// ImgSel RadiusDependentSlice
		if( hXYAccNImagesRadiusDependentSlice000.empty() == 0 )
		{
			cout << "hXYAccNImagesRadiusDependentSlice000 go" << endl;
			for( i = 0 ; i <= 4 ; i++ )
			{
				sprintf( buff, "%s/NImages%d.RadiusDependentSlice000", fExtraHistogramDir.c_str(), i ) ;
				string buff2 = buff ;
				Write1DHistToTextFile( hXYAccNImagesRadiusDependentSlice000[i], buff2, 1 ) ;
			}
		}
		
		// PhiDependentSlice
		if( hXYAccImgSelPhiDependentSlice.empty() == 0 )
		{
			cout << "hXYAccImgSelPhiDependentSlice go" << endl;
			for( i = 0 ; i <= 15 ; i++ )
			{
				sprintf( buff, "%s/ImgSel%d.PhiDependentSlice", fExtraHistogramDir.c_str(), i ) ;
				string buff2 = buff ;
				Write1DHistToTextFile( hXYAccImgSelPhiDependentSlice[i], buff2 , 2 ) ;
			}
		}
		if( hXYAccNImagesPhiDependentSlice.empty() == 0 )
		{
			cout << "hXYAccNImagesPhiDependentSlice go" << endl;
			for( i = 0 ; i <= 4 ; i++ )
			{
				sprintf( buff, "%s/NImages%d.PhiDependentSlice", fExtraHistogramDir.c_str(), i ) ;
				string buff2 = buff ;
				Write1DHistToTextFile( hXYAccNImagesPhiDependentSlice[i], buff2 , 2 ) ;
			}
		}
		
		// write ImgSel 2d hists to text files
		cout << "hXYAccImgSelPreDeRot prep" << endl;
		if( hXYAccImgSelPreDeRot.empty() == 0 )
		{
			for( i = 0 ; i <= 15 ; i++ )
			{
				sprintf( buff, "%s/ImgSelPreDeRot%d", fExtraHistogramDir.c_str(), i ) ;
				string buff2 = buff ;
				Write2DHistToTextFile( hXYAccImgSelPreDeRot[i], buff2 );
			}
		}
		
		// write NImages 2d hists to text files
		cout << "hXYAccNImages prep" << endl;
		if( hXYAccNImages.empty() == 0 )
		{
			cout << "hXYAccNImages go" << endl;
			for( i = 0 ; i <= 4 ; i++ )
			{
				sprintf( buff, "%s/NImages%d", fExtraHistogramDir.c_str(), i ) ;
				string buff2 = buff ;
				Write2DHistToTextFile( hXYAccNImages[i], buff2 );
			}
		}
		
		// write NImages 2d hists to text files
		cout << "hXYAccNImagesPreDeRot prep" << endl;
		if( hXYAccNImagesPreDeRot.empty() == 0 )
		{
			cout << "hXYAccNImagesPreDeRot go" << endl;
			for( i = 0 ; i <= 4 ; i++ )
			{
				sprintf( buff, "%s/NImagesPreDeRot%d", fExtraHistogramDir.c_str(), i ) ;
				string buff2 = buff ;
				Write2DHistToTextFile( hXYAccNImagesPreDeRot[i], buff2 );
			}
		}
	}
	
	if( avgbincount <= 0 )
	{
		cout << "Error: calculate2DBinNormalizationConstant(" << radius << ") : no bins of VRadialAcceptance->hXYAccTotDeRot within radius " << radius << " of center." << endl;
		return 1.0 ;
	}
	else
	{
		normconst = avgbintotal / avgbincount ;
		f2DBinNormalizationConstant = normconst ;
		return normconst ;
	}
	return normconst ;
	
}

// write out 1d hist to a text file
// mostly so other programs (i.e. mathematica) can work with the data
void VRadialAcceptance::Write1DHistToTextFile( TH1F* hist, string& basename, int histtype )
{
	cout << "Write1DHistToTextFile:" << basename.c_str() << endl ;
	// setup
	ofstream datafile ;
	char filename[200] ;
	sprintf( filename, "%s.dat", basename.c_str() ) ;
	datafile.open( filename ) ;
	
	// loop over bins
	int binx ;
	double bincenx, bincont ;
	char dataline[100] ;
	int nxbins = hist->GetNbinsX() ;
	int i ;
	for( binx = 1 ; binx <= nxbins ; binx++ )
	{
		i = ( int )hist->GetBin( binx ) ;
		if( hist->IsBinOverflow( i ) || hist->IsBinUnderflow( i ) )
		{
			continue ;
		}
		bincenx = hist->GetXaxis()->GetBinCenter( binx ) ;
		bincont = hist->GetBinContent( binx ) ;
		sprintf( dataline, "%d %d %f %f", i, binx, bincenx, bincont ) ;
		datafile << dataline << endl ;
	}
	datafile.close() ;
	
	// write metafile, a text file for storing extra info about the histogram
	// (anything that is not bin coordinates or content)
	cout << "fRunPar->fRunList.size(): prep..." << endl ;
	//cout << "fRunPar->fRunList.size():" << fRunPar->fRunList.size() << endl ;
	//cout << ", calculated from " << fNumberOfRawFiles << " files)" << endl;
	//int nruns = fRunPar->fRunList.size() ;
	int nruns = ( int )fNumberOfRawFiles ;
	cout << "fRunPar->fRunList.size(): " << fNumberOfRawFiles << endl ;
	//sprintf( hname, "hAccRun_%d", fRunPar->fRunList[i].fRunOff );
	double nentries = hist->GetEntries() ;
	ofstream metafile ;
	sprintf( filename, "%s.meta", basename.c_str() ) ;
	metafile.open( filename ) ;
	metafile << "datafileheaders i binx bincentx bincont" << endl ;
	metafile << "nxbins " << nxbins << endl ;
	metafile << "nentries " << nentries << endl ;
	metafile << "nruns " << nruns << endl ;
	if( histtype == 1 )   // 1 = RadiusDependentSlice
	{
		metafile << "rad_minrad "   << rad_minrad << endl;
		metafile << "rad_maxrad "   << rad_maxrad << endl;
		metafile << "rad_phiwidth " << rad_phiwidth << endl;
		metafile << "notes Slice of Pie! Bin position depends on Radius" << endl;
	}
	else if( histtype == 2 )  // 2 = PhiDependentSlice
	{
		metafile << "phi_minphi " << phi_minphi << endl;
		metafile << "phi_maxphi " << phi_maxphi << endl;
		metafile << "phi_minrad " << phi_minradius << endl;
		metafile << "phi_maxrad " << phi_maxradius << endl;
		metafile << "notes Doughnut! Bin position depends on Phi" << endl;
	}
	metafile << "ImgSelMeta ImgSelNumber T1234" << endl ;
	metafile << "ImgSel 0 0000 T" << endl ;
	metafile << "ImgSel 1 1000 T1" << endl ;
	metafile << "ImgSel 2 0100 T2" << endl ;
	metafile << "ImgSel 3 1100 T12" << endl ;
	metafile << "ImgSel 4 0010 T3" << endl ;
	metafile << "ImgSel 5 1010 T13" << endl ;
	metafile << "ImgSel 6 0110 T23" << endl ;
	metafile << "ImgSel 7 1110 T123" << endl ;
	metafile << "ImgSel 8 0001 T4" << endl ;
	metafile << "ImgSel 9 1001 T14" << endl ;
	metafile << "ImgSel 10 0101 T24" << endl ;
	metafile << "ImgSel 11 1101 T124" << endl ;
	metafile << "ImgSel 12 0011 T34" << endl ;
	metafile << "ImgSel 13 1011 T134" << endl ;
	metafile << "ImgSel 14 0111 T234" << endl ;
	metafile << "ImgSel 15 1111 T1234" << endl ;
	metafile.close() ;
}

// write out 2d hist to a text file
// mostly so other programs (i.e. mathematica) can work with the data
void VRadialAcceptance::Write2DHistToTextFile( TH2F* hist, string& basename )
{
	cout << "Write2DHistToTextFile:" << basename.c_str() << endl ;
	// setup
	ofstream datafile ;
	char filename[200] ;
	sprintf( filename, "%s.dat", basename.c_str() ) ;
	datafile.open( filename ) ;
	
	// loop over bins
	int binx, biny ;
	double bincenx, binceny, bincont ;
	char dataline[100] ;
	int nxbins = hist->GetNbinsX() ;
	int nybins = hist->GetNbinsY() ;
	int i ;
	for( binx = 1 ; binx <= nxbins ; binx++ )
	{
		for( biny = 1 ; biny <= nybins ; biny++ )
		{
			i = ( int )hist->GetBin( binx, biny ) ;
			if( hist->IsBinOverflow( i ) || hist->IsBinUnderflow( i ) )
			{
				continue ;
			}
			bincenx = hist->GetXaxis()->GetBinCenter( binx ) ;
			binceny = hist->GetYaxis()->GetBinCenter( biny ) ;
			bincont = hist->GetBinContent( binx, biny ) ;
			sprintf( dataline, "%d %d %d %f %f %f", i, binx, biny, bincenx, binceny, bincont ) ;
			datafile << dataline << endl ;
		}
	}
	datafile.close() ;
	
	// write metafile, a text file for storing extra info about the histogram
	// (anything that is not bin coordinates or content)
	cout << "fRunPar->fRunList.size(): prep..." << endl ;
	//cout << "fRunPar->fRunList.size():" << fRunPar->fRunList.size() << endl ;
	//cout << ", calculated from " << fNumberOfRawFiles << " files)" << endl;
	//int nruns = fRunPar->fRunList.size() ;
	int nruns = ( int )fNumberOfRawFiles ;
	cout << "fRunPar->fRunList.size(): " << fNumberOfRawFiles << endl ;
	//sprintf( hname, "hAccRun_%d", fRunPar->fRunList[i].fRunOff );
	double nentries = hist->GetEntries() ;
	ofstream metafile ;
	sprintf( filename, "%s.meta", basename.c_str() ) ;
	metafile.open( filename ) ;
	metafile << "datafileheaders i binx biny bincentx bincenty bincont" << endl ;
	metafile << "nxbins " << nxbins << endl ;
	metafile << "nybins " << nybins << endl ;
	metafile << "nentries " << nentries << endl ;
	metafile << "nruns " << nruns << endl ;
	metafile << "ImgSelMeta ImgSelNumber T1234" << endl ;
	metafile << "ImgSel 0 0000 T" << endl ;
	metafile << "ImgSel 1 1000 T1" << endl ;
	metafile << "ImgSel 2 0100 T2" << endl ;
	metafile << "ImgSel 3 1100 T12" << endl ;
	metafile << "ImgSel 4 0010 T3" << endl ;
	metafile << "ImgSel 5 1010 T13" << endl ;
	metafile << "ImgSel 6 0110 T23" << endl ;
	metafile << "ImgSel 7 1110 T123" << endl ;
	metafile << "ImgSel 8 0001 T4" << endl ;
	metafile << "ImgSel 9 1001 T14" << endl ;
	metafile << "ImgSel 10 0101 T24" << endl ;
	metafile << "ImgSel 11 1101 T124" << endl ;
	metafile << "ImgSel 12 0011 T34" << endl ;
	metafile << "ImgSel 13 1011 T134" << endl ;
	metafile << "ImgSel 14 0111 T234" << endl ;
	metafile << "ImgSel 15 1111 T1234" << endl ;
	metafile.close() ;
	
}

/////////////////////////////////
// if set to >= 1, will use alternate 2D acceptance radial acceptances
int VRadialAcceptance::Set2DAcceptanceMode( int mode )
{
	if( mode >= 0 )
	{
		f2DAcceptanceMode = mode ;
	}
	
	if( f2DAcceptanceMode > 0 )
	{
	
		// load 2d hist hXYAccTotDeRot from file
		hXYAccTotDeRot = ( TH2F* )gDirectory->Get( "hXYAccTotDeRot" );
		if( ! hXYAccTotDeRot )
		{
			cout << "Error, Radial Acceptance File " << fAccFile->GetName() << " does not contain the TH2F histogram 'hXYAccTotDeRot', required for calculating 2D acceptance.  Suggest using a newer Acceptance File, or use 1D Radial Acceptance mode instead of 2D Acceptance mode." << endl;
			exit( -1 ) ;
		}
		
		// calculate normalization constant
		calculate2DBinNormalizationConstant() ;
	}
	
	return f2DAcceptanceMode ;
}

void VRadialAcceptance::SetExtraHistogramDirectory( string histdir )
{
	fExtraHistogramMode = 1 ;
	fExtraHistogramDir = histdir ;
}

int VRadialAcceptance::SetExtraHistogramMode( int ehm )
{
	fExtraHistogramMode = ehm ;
	return fExtraHistogramMode ;
}
