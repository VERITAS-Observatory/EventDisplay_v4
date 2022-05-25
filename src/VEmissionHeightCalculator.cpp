/*! \class VEmissionHeightCalculator
    \brief  get systematic error in energy reconstruction and calculate mean height of maximum emission

    
*/

#include "VEmissionHeightCalculator.h"

VEmissionHeightCalculator::VEmissionHeightCalculator()
{
	fDebug = false;
	
	fNTel = 0;
	fEsys = 0.;
	fEsysError = 0.;
	fNTelPairs = 0;
	fEmissionHeight = 0.;
	fEmissionHeightChi2 = 0.;
	fEmissionHeightT.assign( 1000, -99. );
	
	fInputFile = 0;
	hHeight = 0;
	hHeightEsys = 0;
	hHeightEsysProf = 0;
}


VEmissionHeightCalculator::VEmissionHeightCalculator( string iInputfile )
{
	fDebug = false;
	
	fNTel = 0;
	fEsys = 0.;
	fEsysError = 0.;
	fNTelPairs = 0;
	fEmissionHeight = 0.;
	fEmissionHeightChi2 = 0.;
	fEmissionHeightT.assign( 1000, -99. );
	
	// hardwired available zenith angle
	fZe.push_back( "00" );
	fZe.push_back( "20" );
	fZe.push_back( "30" );
	fZe.push_back( "40" );
	for( unsigned int i = 0; i < fZe.size(); i++ )
	{
		fZeDouble.push_back( atof( fZe[i].c_str() ) );
	}
	
	// define some debug histograms
	double maxHeigth = 40.;
	
	// height distribution
	hHeight = new TH1D( "hHeight", "", ( int )( maxHeigth / 2. ), 0., maxHeigth );
	hHeight->SetStats( 0 );
	hHeight->SetLineWidth( 2 );
	hHeight->SetXTitle( "distance of maximum emission [km]" );
	hHeight->SetYTitle( "number of entries" );
	
	hHeightEsys = new TH2D( "hHeightEsys", "", ( int )( 10.*maxHeigth ), 0., maxHeigth, 100, -0.5, 0.5 );
	hHeightEsys->SetStats( 0 );
	hHeightEsys->SetXTitle( "distance of maximum emission [km]" );
	hHeightEsys->SetYTitle( "log_{10} E_{rec} - log_{10} E_{MC}" );
	
	hHeightEsysProf = new TProfile( "hHeightEsysProf", "", ( int )( 10.*maxHeigth ), 0., maxHeigth, -100., 100. );
	hHeightEsysProf->SetStats( 0 );
	hHeightEsysProf->SetXTitle( "distance of maximum emission [km]" );
	hHeightEsysProf->SetYTitle( "log_{10} E_{rec} - log_{10} E_{MC}" );
	
	// read in output file
	
	if( iInputfile.size() == 0 )
	{
		cout << "VEmissionHeightCalculator:: error opening file with energy correction curves" << endl;
		cout << "...exiting" << endl;
		exit( 0 );
	}
	iInputfile = VUtilities::testFileLocation( iInputfile, "./energyCorrection/", true );
	fInputFile = new TFile( iInputfile.c_str() );
	if( fInputFile->IsZombie() )
	{
		cout << "VEmissionHeightCalculator:: error opening file with energy correction curves: " << iInputfile << endl;
		cout << "...exiting" << endl;
		exit( 0 );
	}
	cout << "reading energy correction curves from " << iInputfile << endl << endl;
	char hname[200];
	for( unsigned int i = 0; i < fZe.size(); i++ )
	{
		sprintf( hname, "fFun_%s", fZe[i].c_str() );
		if( gDirectory->Get( hname ) )
		{
			fCorrectionCurves.push_back( ( TF1* )gDirectory->Get( hname ) );
			fCorrectionCurvesXmin.push_back( fCorrectionCurves.back()->GetXmin() );
			fCorrectionCurvesXmax.push_back( fCorrectionCurves.back()->GetXmax() );
		}
		else
		{
			cout << "VEmissionHeightCalculator: error reading in energy correction curve for zenith angle " << fZe[i] << endl;
		}
	}
}


VEmissionHeightCalculator::~VEmissionHeightCalculator()
{
	if( fInputFile )
	{
		fInputFile->Close();
	}
}


void VEmissionHeightCalculator::getEmissionHeight( double* cen_x, double* cen_y, double* dist, double* size, double* r, double* az, double* el )
{
	getEnergyCorrectionOrEmissionHeight( cen_x, cen_y, dist, size, r, az, el, false );
}


double VEmissionHeightCalculator::getEnergyCorrection( double* cen_x, double* cen_y, double* dist, double* size, double* r, double* az, double* el )
{
	return getEnergyCorrectionOrEmissionHeight( cen_x, cen_y, dist, size, r, az, el, true );
}


double VEmissionHeightCalculator::getEnergyCorrectionOrEmissionHeight( double* cen_x, double* cen_y, double* dist, double* size, double* r, double* az, double* el, bool bEcorrect )
{
	double iEsys = 0.;
	double iEsys2 = 0.;
	double iEmissionHeight = 0.;
	double iEmissionHeightWeight = 0.;
	double iEmissionHeightWeightTemp = 0.;
	double iEmissionHeight2 = 0.;
	double iEmissionHeightTemp = 0.;
	double iEsysTemp = 0.;
	double iNEsys = 0.;
	
	double fTelescopeDistanceSC = 0.;
	double fImageDistance = 0.;
	// counter for telescope pairs
	int nTPair = 0;
	
	// reset emission heights
	fEmissionHeight = 0.;
	fEmissionHeightChi2 = 0.;
	// loop over all telescope pairs
	for( unsigned int i = 0; i < fNTel; i++ )
	{
		for( unsigned int j = i; j < fNTel; j++ )
		{
			// require elevation > 0. (test if telescope is present in analysis)
			if( i != j )
			{
				if( el[i] > 0. && el[j] > 0. && size[i] > 0 && size[j] > 0 )
				{
					// get tangens of distance between the two image centroids
					fImageDistance = TMath::Tan( imageDistance( cen_x[i], cen_x[j], cen_y[i], cen_y[j] ) / TMath::RadToDeg() );
					if( fImageDistance > 0. )
					{
						// get distance between the two telescopes in shower coordinates
						fTelescopeDistanceSC = getTelescopeDistanceSC( i, j, az[i], el[i] );
						// calculate emission height [km]
						iEmissionHeightTemp = fTelescopeDistanceSC / fImageDistance / 1.e3;
						if( bEcorrect )
						{
							iEsysTemp = getEnergyCorrectionFromFunction( iEmissionHeightTemp, el[i] );
						}
						else
						{
							iEsysTemp = 0.;
						}
						if( iEsysTemp > -99. )
						{
							iEsys += iEsysTemp;
							iEsys2 += iEsysTemp * iEsysTemp;
							// weight for pairwise emission height calculation
							iEmissionHeightWeightTemp = 1. / ( ( 1. / log10( size[i] ) ) + ( 1. / log10( size[j] ) ) );
							iEmissionHeightWeight    += iEmissionHeightWeightTemp;
							iEmissionHeight          += iEmissionHeightTemp * iEmissionHeightWeightTemp;
							iEmissionHeight2         += iEmissionHeightTemp * iEmissionHeightTemp * iEmissionHeightWeightTemp;
							iNEsys++;
							
						}
						if( nTPair < 1000 )
						{
							fEmissionHeightT[nTPair] = iEmissionHeightTemp;
						}
					}
					nTPair++;
				}
			}
		}
	}
	// return mean correction factor (mean of values from all telescope pairs)
	fNTelPairs = nTPair;
	if( iNEsys > 0. && iEmissionHeightWeight > 0. )
	{
		// calculate correction factor
		if( iNEsys > 1. )
		{
			fEsysError = sqrt( 1. / ( iNEsys - 1. ) * ( iEsys2 - 1. / iNEsys * iEsys * iEsys ) );
		}
		else
		{
			fEsysError = 0.;
		}
		fEsys = iEsys / iNEsys;
		// calculate mean emission height
		fEmissionHeight = iEmissionHeight / iEmissionHeightWeight;
		iEmissionHeight2 /= iEmissionHeightWeight;
		if( iNEsys > 1. )
		{
			fEmissionHeightChi2 = sqrt( 1. / ( iNEsys - 1. ) * ( iEmissionHeight2 - fEmissionHeight * fEmissionHeight ) );
		}
		else
		{
			fEmissionHeightChi2 = 0.;
		}
		// fill histograms
		if( bEcorrect )
		{
			hHeight->Fill( fEmissionHeight );
			hHeightEsys->Fill( fEmissionHeight, fEsys );
			hHeightEsysProf->Fill( fEmissionHeight, fEsys );
		}
	}
	else
	{
		fEsys = 0.;
		fEsysError = 0.;
		fEmissionHeight = 0.;
	}
	if( fDebug )
	{
		cout << "mean energy correction factor: " << fEsys << "\t" << fEsysError << "\t" << fEmissionHeight << "\t" << iNEsys <<  endl;
	}
	return fEsys;
}


void VEmissionHeightCalculator::setTelescopePositions( unsigned int ntel, double* x, double* y, double* z )
{
	fNTel = ntel;
	for( unsigned int i = 0; i < ntel; i++ )
	{
		fTelX.push_back( x[i] );
		fTelY.push_back( y[i] );
		fTelZ.push_back( z[i] );
		if( fDebug )
		{
			cout << "VEmissionHeightCalculator::setTelescopePositions " << i + 1 << "\t" << fTelX.back() << "\t" << fTelY.back() << "\t" << fTelZ.back() << endl;
		}
	}
}


/*!
    get distance between telescopes in shower coordinates
*/
double VEmissionHeightCalculator::getTelescopeDistanceSC( unsigned int iTel1, unsigned int iTel2, double az, double z )
{
	if( iTel1 >= fTelX.size() || iTel2 >= fTelX.size() )
	{
		cout << "VEmissionHeightCalculator::getTelescopeDistanceSC error: telescope identifier out of range: " << fTelX.size() << "\t" << iTel1 << "\t" << iTel2 << endl;
		return -999.;
	}
	
	double t1[3], t2[3];
	
	az /= TMath::RadToDeg();
	z  /= TMath::RadToDeg();
	
	t1[0] = fTelX[iTel1];
	t1[1] = fTelY[iTel1];
	t1[2] = fTelZ[iTel1];
	
	t2[0] = fTelX[iTel2];
	t2[1] = fTelY[iTel2];
	t2[2] = fTelZ[iTel2];
	
	return VUtilities::line_point_distance( t1[0], t1[1], t1[2], 90. - z * TMath::RadToDeg(), az * TMath::RadToDeg(), t2[0], t2[1], t2[2] );
}


/*!
    calculate distance between two images in the camera
*/
double VEmissionHeightCalculator::imageDistance( double c1x, double c2x, double c1y, double c2y )
{
	return sqrt( ( c1x - c2x ) * ( c1x - c2x ) + ( c1y - c2y ) * ( c1y - c2y ) );
}


double VEmissionHeightCalculator::getEnergyCorrectionFromFunction( double iEmissionHeight, double iEl )
{
	double ze = 90. - iEl;
	
	// zenith angle lower and upper limit
	unsigned int ize_low = 0;
	unsigned int ize_up = 0;
	
	if( ze <= fZeDouble[0] )
	{
		ize_low = ize_up = 0;
	}
	else if( ze >= fZeDouble.back() )
	{
		ize_low = ize_up = fZeDouble.size() - 1;
	}
	else
	{
		for( unsigned int i = 0; i < fZeDouble.size(); i++ )
		{
			if( ze > fZeDouble[i] )
			{
				ize_low = ( int )i;
				ize_up = ( int )i + 1;
			}
		}
	}
	
	double e_low = 0.;
	if( ize_low < fCorrectionCurves.size() && fCorrectionCurves[ize_low] )
	{
		if( iEmissionHeight < fCorrectionCurvesXmin[ize_low] )
		{
			e_low = fCorrectionCurves[ize_low]->Eval( fCorrectionCurvesXmin[ize_low] );
		}
		else if( iEmissionHeight > fCorrectionCurvesXmax[ize_low] )
		{
			e_low = fCorrectionCurves[ize_low]->Eval( fCorrectionCurvesXmax[ize_low] );
		}
		else
		{
			e_low = fCorrectionCurves[ize_low]->Eval( iEmissionHeight );
		}
	}
	double e_up = 0.;
	if( ize_up < fCorrectionCurves.size() && fCorrectionCurves[ize_up] )
	{
		if( iEmissionHeight < fCorrectionCurvesXmin[ize_up] )
		{
			e_up = fCorrectionCurves[ize_up]->Eval( fCorrectionCurvesXmin[ize_up] );
		}
		else if( iEmissionHeight > fCorrectionCurvesXmax[ize_up] )
		{
			e_up = fCorrectionCurves[ize_up]->Eval( fCorrectionCurvesXmax[ize_up] );
		}
		else
		{
			e_up = fCorrectionCurves[ize_up]->Eval( iEmissionHeight );
		}
	}
	if( fDebug )
	{
		cout << "\t" << ze << "\t" << ize_low << "\t" << ize_up << "\t" << fZeDouble.size() << "\t" << iEmissionHeight << "\t" << e_low << "\t" << e_up;
		cout << "\t" << interpolate( e_low, fZeDouble[ize_low], e_up, fZeDouble[ize_up], ze, true, 0.5, -90. );
		cout << "\t" << fCorrectionCurvesXmin[ize_low] << "\t" << fCorrectionCurvesXmax[ize_low];
		cout << "\t" << fCorrectionCurvesXmin[ize_up] << "\t" << fCorrectionCurvesXmax[ize_up] << endl;
	}
	return interpolate( e_low, fZeDouble[ize_low], e_up, fZeDouble[ize_up], ze, true, 0.5, -90. );
}


void VEmissionHeightCalculator::write()
{
	TDirectory* iDir = gDirectory;
	
	if( iDir->mkdir( "energyCorrection" )->cd() )
	{
		hHeight->Write();
		hHeightEsys->Write();
		hHeightEsysProf->Write();
	}
	iDir->cd();
}


/*!
    interpolate between two zenith angles (iCos = true )
    interpolate between two values (iCos = false )

    ze [deg]

    weighted by cos ze

    NOTE: COPY FROM VSTATISTICS.H; NOT CLEAR WHY THIS IS NEEDED
*/
double VEmissionHeightCalculator::interpolate( double w1, double ze1, double w2, double ze2, double ze, bool iCos,
		double iLimitforInterpolation, double iMinValidValue )
{
	// don't interpolate if both values are not valid
	if( w1 < iMinValidValue && w2 < iMinValidValue )
	{
		return -99.;
	}
	
	// same x-value, don't interpolate
	if( fabs( ze1 - ze2 ) < 1.e-3 )
	{
		if( w1 < iMinValidValue )
		{
			return w2;
		}
		else if( w2 < iMinValidValue )
		{
			return w1;
		}
		return ( w1 + w2 ) / 2.;
	}
	
	// interpolate
	double id = 0.;
	double f1 = 0.;
	double f2 = 0.;
	if( iCos )
	{
		id = cos( ze1 * TMath::DegToRad() ) - cos( ze2 * TMath::DegToRad() );
		f1 = 1. - ( cos( ze1 * TMath::DegToRad() ) - cos( ze * TMath::DegToRad() ) ) / id;
		f2 = 1. - ( cos( ze * TMath::DegToRad() ) - cos( ze2 * TMath::DegToRad() ) ) / id;
	}
	else
	{
		id = ze1 - ze2;
		f1 = 1. - ( ze1 - ze ) / id;
		f2 = 1. - ( ze  - ze2 ) / id;
	}
	
	// one of the values is not valid:
	// return valid value only when f1 or f2 > iLimitforInterPolation
	if( w1 > iMinValidValue && w2 < iMinValidValue )
	{
		if( f1 > iLimitforInterpolation )
		{
			return w1;
		}
		else
		{
			return -99.;
		}
	}
	else if( w1 < iMinValidValue && w2 > iMinValidValue )
	{
		if( f2 > iLimitforInterpolation )
		{
			return w2;
		}
		else
		{
			return -99.;
		}
	}
	
	return ( w1 * f1 + w2 * f2 );
}

