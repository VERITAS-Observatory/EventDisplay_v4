/*! \class VEnergyCorrection
    \brief  get systematic error in energy reconstruction and calculate mean height of maximum emission

    Revision $Id: VEnergyCorrection.cpp,v 1.1.2.1.36.3.2.1.2.1 2011/03/25 13:11:32 gmaier Exp $

    \author Gernot Maier
*/

#include "VEnergyCorrection.h"

VEnergyCorrection::VEnergyCorrection()
{
    fDebug = false;

    degrad = 45. / atan( 1. );
    raddeg = 1./degrad;

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


VEnergyCorrection::VEnergyCorrection( string iInputfile )
{
    fDebug = false;

    degrad = 45. / atan( 1. );
    raddeg = 1./degrad;

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
    for( unsigned int i = 0; i < fZe.size(); i++ ) fZeDouble.push_back( atof( fZe[i].c_str() ) );

// define some debug histograms
    double maxHeigth = 40.;

// height distribution
    hHeight = new TH1D( "hHeight", "", (int)(maxHeigth/2.), 0., maxHeigth );
    hHeight->SetStats( 0 );
    hHeight->SetLineWidth( 2 );
    hHeight->SetXTitle( "distance of maximum emission [km]" );
    hHeight->SetYTitle( "number of entries" );

    hHeightEsys = new TH2D( "hHeightEsys", "", (int)(10.*maxHeigth), 0., maxHeigth, 100, -0.5, 0.5 );
    hHeightEsys->SetStats( 0 );
    hHeightEsys->SetXTitle( "distance of maximum emission [km]" );
    hHeightEsys->SetYTitle( "log_{10} E_{rec} - log_{10} E_{MC}" );

    hHeightEsysProf = new TProfile( "hHeightEsysProf", "", (int)(10.*maxHeigth), 0., maxHeigth, -100., 100. );
    hHeightEsysProf->SetStats( 0 );
    hHeightEsysProf->SetXTitle( "distance of maximum emission [km]" );
    hHeightEsysProf->SetYTitle( "log_{10} E_{rec} - log_{10} E_{MC}" );

// read in output file

    if( iInputfile.size() == 0 )
    {
        cout << "VEnergyCorrection:: error opening file with energy correction curves" << endl;
        cout << "...exiting" << endl;
        exit( 0 );
    }
    iInputfile = testFileLocation( iInputfile, "./energyCorrection/" );
    fInputFile = new TFile( iInputfile.c_str() );
    if( fInputFile->IsZombie() )
    {
        cout << "VEnergyCorrection:: error opening file with energy correction curves: " << iInputfile << endl;
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
            fCorrectionCurves.push_back( (TF1*)gDirectory->Get( hname ) );
            fCorrectionCurvesXmin.push_back( fCorrectionCurves.back()->GetXmin() );
            fCorrectionCurvesXmax.push_back( fCorrectionCurves.back()->GetXmax() );
        }
        else
        {
            cout << "VEnergyCorrection: error reading in energy correction curve for zenith angle " << fZe[i] << endl;
        }
    }
}


VEnergyCorrection::~VEnergyCorrection()
{
    if( fInputFile ) fInputFile->Close();
}


void VEnergyCorrection::calculateEmissionHeight( double *cen_x, double *cen_y, double *dist, double *size, double *r, double* az, double* el )
{
    getEnergyCorrection( cen_x, cen_y, dist, size, r, az, el, false );
}


double VEnergyCorrection::getEnergyCorrection( double *cen_x, double *cen_y, double *dist, double *size, double *r, double* az, double* el )
{
    return getEnergyCorrection( cen_x, cen_y, dist, size, r, az, el, true );
}


double VEnergyCorrection::getEnergyCorrection( double *cen_x, double *cen_y, double *dist, double *size, double *r, double* az, double* el, bool bEcorrect )
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
		   fImageDistance = TMath::Tan( imageDistance( cen_x[i], cen_x[j], cen_y[i], cen_y[j] ) / degrad );
		   if( fImageDistance > 0. )
		   {
// get distance between the two telescopes in shower coordinates
		       fTelescopeDistanceSC = getTelescopeDistanceSC( i, j, az[i], el[i] );
// calculate emission height [km]
		       iEmissionHeightTemp = fTelescopeDistanceSC / fImageDistance / 1.e3;
		       if( bEcorrect ) iEsysTemp = getEnergyCorrectionFromFunction( iEmissionHeightTemp, el[i] );
		       else            iEsysTemp = 0.;
		       if( iEsysTemp > -99. )
		       {
			   iEsys += iEsysTemp;
			   iEsys2 += iEsysTemp*iEsysTemp;
// weight for pairwise emission height calculation
   			   iEmissionHeightWeightTemp = 1./((1./log10(size[i]))+(1./log10(size[j])));
			   iEmissionHeightWeight    += iEmissionHeightWeightTemp;
			   iEmissionHeight          += iEmissionHeightTemp * iEmissionHeightWeightTemp;
			   iEmissionHeight2         += iEmissionHeightTemp * iEmissionHeightTemp * iEmissionHeightWeightTemp;
			   iNEsys++;

		       }
		       if( nTPair < 1000 ) fEmissionHeightT[nTPair] = iEmissionHeightTemp;
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
        if( iNEsys > 1. ) fEsysError = sqrt( 1./(iNEsys-1.) * ( iEsys2 - 1./iNEsys * iEsys * iEsys ) );
        else              fEsysError = 0.;
        fEsys = iEsys / iNEsys;
// calculate mean emission height 
        fEmissionHeight = iEmissionHeight / iEmissionHeightWeight;
	iEmissionHeight2 /= iEmissionHeightWeight;
         if( iNEsys > 1. )    fEmissionHeightChi2 = sqrt( 1./(iNEsys-1.) * ( iEmissionHeight2 - fEmissionHeight * fEmissionHeight ) );
	 else                 fEmissionHeightChi2 = 0.;
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


void VEnergyCorrection::setTelescopePositions( unsigned int ntel, double *x, double *y, double *z )
{
    fNTel = ntel;
    for( unsigned int i = 0; i < ntel; i++ )
    {
        fTelX.push_back( x[i] );
        fTelY.push_back( y[i] );
        fTelZ.push_back( z[i] );
        if( fDebug )
        {
            cout << "VEnergyCorrection::setTelescopePositions " << i+1 << "\t" << fTelX.back() << "\t" << fTelY.back() << "\t" << fTelZ.back() << endl;
        }
    }
}


/*!
    get distance between telescopes in shower coordinates
*/
double VEnergyCorrection::getTelescopeDistanceSC( unsigned int iTel1, unsigned int iTel2, double az, double z )
{
    if( iTel1 >= fTelX.size() || iTel2 >= fTelX.size() )
    {
        cout << "VEnergyCorrection::getTelescopeDistanceSC error: telescope identifier out of range: " << fTelX.size() << "\t" << iTel1 << "\t" << iTel2 << endl;
        return -999.;
    }

    double s[3], t1[3], t2[3];
// (XX)    double m,d,dist;

    az /= degrad;
    z  /= degrad;

    s[0]    = sin(z)*cos(az);
    s[1]    = sin(z)*sin(az);
    s[2]    = cos(z);

    t1[0] = fTelX[iTel1];
    t1[1] = fTelY[iTel1];
    t1[2] = fTelZ[iTel1];

    t2[0] = fTelX[iTel2];
    t2[1] = fTelY[iTel2];
    t2[2] = fTelZ[iTel2];

    return line_point_distance( t1[0], t1[1], t1[2], z*degrad, az*degrad, t2[0], t2[1], t2[2] );
/* XX

    m =0.;
    for( int i = 0; i < 3; i++ ) m += (t1[i]-t2[i])*s[i];

    dist =0.;
    for( int i = 0; i < 3; i++ )
    {
        d = t1[i]+m*s[i]-t2[i];
        dist += d*d;
    }

    cout << "Coordinates: " << t1[0] << "\t" << t1[1] << "\t" << t1[2] << "\t" << t2[0] << "\t" << t2[1] << "\t" << t2[2] << "\t" << endl;
    cout << "DISTANCE (1): " << sqrt( dist ) << endl;
    cout << "DISTANCE (2): " << line_point_distance( t1[0], t1[1], t1[2], z*degrad, az*degrad, t2[0], t2[1], t2[2] ) << endl;
    cout << "DISTANCE (3): " << sqrt( (t1[0]-t2[0])*(t1[0]-t2[0]) + (t1[1]-t2[1])*(t1[1]-t2[1]) + (t1[2]-t2[2])*(t1[2]-t2[2]) ) << endl;

    return sqrt( dist ); */
}


double VEnergyCorrection::line_point_distance(double x1, double y1, double z1, double el, double az, double x, double y, double z)
{
    double a, a1, a2, a3, b;

    az = 180. - az;

    double cx = -1.*cos(el*(TMath::Pi()/180.))*cos(az*(TMath::Pi()/180.));
    double cy = -1.*cos(el*(TMath::Pi()/180.))*sin(az*(TMath::Pi()/180.));
    double cz = sin(el*(TMath::Pi()/180.));

    a1 = (y-y1)*cz - (z-z1)*cy;
    a2 = (z-z1)*cx - (x-x1)*cz;
    a3 = (x-x1)*cy - (y-y1)*cx;
    a  = a1*a1 + a2*a2 + a3*a3;
    b = cx*cx + cy*cy + cz*cz;

    if ( a<0. || b<= 0. ) return -1;

    return sqrt(a/b);
}



/*!
    calculate distance between two images in the camera
*/
double VEnergyCorrection::imageDistance( double c1x, double c2x, double c1y, double c2y )
{
    return sqrt( (c1x-c2x)*(c1x-c2x) + (c1y-c2y)*(c1y-c2y) );
}


double VEnergyCorrection::getEnergyCorrectionFromFunction( double iEmissionHeight, double iEl )
{
    double ze = 90. -iEl;

// zenith angle lower and upper limit
    unsigned int ize_low = 0;
    unsigned int ize_up = 0;

    if( ze <= fZeDouble[0] )
    {
        ize_low = ize_up = 0;
    }
    else if( ze >= fZeDouble.back() )
    {
        ize_low = ize_up = fZeDouble.size()-1;
    }
    else
    {
        for( unsigned int i = 0; i < fZeDouble.size(); i++ )
        {
            if( ze > fZeDouble[i] )
            {
                ize_low = (int)i;
                ize_up = (int)i+1;
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
        cout << "\t" << interpolate_WL( ze, fZeDouble[ize_low], fZeDouble[ize_up], e_low, e_up );
        cout << "\t" << fCorrectionCurvesXmin[ize_low] << "\t" << fCorrectionCurvesXmax[ize_low];
        cout << "\t" << fCorrectionCurvesXmin[ize_up] << "\t" << fCorrectionCurvesXmax[ize_up] << endl;
    }
    return interpolate_WL( ze, fZeDouble[ize_low], fZeDouble[ize_up], e_low, e_up );
}


/*!
    interpolate between two values

    ze [deg]

    weighted by cos ze
*/
double VEnergyCorrection::interpolate_WL( double ze, double ze1, double ze2, double w1, double w2 )
{
// don't interpolate if one or two values are not valid
    if( w1 < -90. && w2 < -90. ) return -99.;
    else if( w1 > -90. && w2 < -90. ) return w1;
    else if( w1 < -90. && w2 > -90. ) return w2;

// same zenith angle, don't interpolate
    if( ze1 == ze2 )
    {
        return (w1+w2)/2.;
    }

// interpolate
    double id, f1, f2;
    id = cos( ze1*raddeg ) - cos( ze2*raddeg );
    f1 = 1. - (cos( ze1*raddeg ) - cos( ze*raddeg )) / id;
    f2 = 1. - (cos( ze*raddeg ) - cos( ze2*raddeg )) / id;

    return (w1*f1+w2*f2);
}


string VEnergyCorrection::testFileLocation( string iFile, string iDirectory )
{
    ifstream is;
    is.open( iFile.c_str(), ifstream::in);
    if( !is )
    {
        char itemp[200];
        sprintf( itemp, "%s/%s", iDirectory.c_str(), iFile.c_str() );
        ifstream is2;
        is2.open( itemp, ifstream::in );
        if( !is2 )
        {
            cout << "testFileLocation: Error opening file: " << iFile << endl;
            iFile = "";
        }
        iFile = itemp;
    }
    is.close();
    return iFile;
}


void VEnergyCorrection::write()
{
    TDirectory *iDir = gDirectory;

    if( iDir->mkdir( "energyCorrection" )->cd() )
    {
        hHeight->Write();
        hHeightEsys->Write();
        hHeightEsysProf->Write();
    }
    iDir->cd();
}
