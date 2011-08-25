/*! \class VRadialAcceptance
 *  \brief radial acceptance for a given point on the sky
 *
 *   OBSERVE: some hard-wired stuff
 *
 *    \author
 *    Gernot Maier 
 *
 *  Revision $Id: VRadialAcceptance.cpp,v 1.15.2.5.4.2.10.1.2.1.4.1.2.2.2.3.8.4.2.4.2.2.2.1 2011/03/31 14:50:58 gmaier Exp $
 */

#include "VRadialAcceptance.h"

/*!
 *   use acceptance curve from simulation
 */
VRadialAcceptance::VRadialAcceptance()
{
    reset();

    fAcceptanceFunctionDefined = false;

    initAcceptancefromSimulations();
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
    if( ifile.size() == 0 ) exit( 0 );

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
        TF1 *iF = (TF1*)gDirectory->Get( hname );
        if( iF )
        {
            fAccZe.push_back( iF );
            fAcceptanceFunctionDefined = true;
        }
        else     break;
        i++;
    }
    cout << "\t total number of acceptance curves: " << fAccZe.size() << " (found in " << ifile;

// count number of raw files used to calculate acceptances
    TKey *key;
    TIter nextkey( fAccFile->GetListOfKeys() );
    while ((key = (TKey*)nextkey()))
    {
        TObject *obj = key->ReadObj();
        string itemp = obj->GetName();
        if( itemp.find( "hAccRun_" ) < itemp.size() ) fNumberOfRawFiles++;
    }
    cout << ", calculated from " << fNumberOfRawFiles << " files)" << endl;
}


/*!
 *  ******************************************************************
 *          making acceptance curves
 *  ******************************************************************
 *   this constructor is called for determination of radial acceptance curves with getAcceptance
 */
VRadialAcceptance::VRadialAcceptance( VGammaHadronCuts* icuts, VAnaSumRunParameter *irunpar )
{
    reset();

    hList = new TList();

    fCuts = icuts;
    fRunPar = irunpar;

// upper limit for zenith angle interal (18 == [0,18.])
/*   fZe.push_back( 20. );
   fZe.push_back( 30. );
   fZe.push_back( 40. ); */
    fZe.push_back( 70. );

    double xymax = 2.0;
    int nxybin = 20;

// range used to normalise acceptance histograms
    fAccZeFitMinBin = 1;
    fAccZeFitMaxBin = 5;

    hscale = new TH1D( "hscale", "", nxybin, 0., xymax );
    for( int i = 1; i < nxybin; i++ ) hscale->SetBinContent( i, TMath::Pi()*xymax*xymax/((double)(nxybin*nxybin))*(2*i-1));
    for( int i = 1; i < nxybin; i++ ) hscale->SetBinError( i, 0. );
    hList->Add( hscale );

    char hname[200];
    char htitle[200];
    double ize = 0.;
    for( unsigned int i = 0; i < fZe.size(); i++ )
    {
        cout << "zenith angle intervall: ";
        if( i == 0 ) cout << 0;
        else         cout << fZe[i-1];
        cout << " - " << fZe[i] << endl;

        sprintf( hname, "hAccZe_%d", i );
        sprintf( htitle, "%.0f < el < %.0f", 90.-fZe[i], 90.-ize );
        hAccZe.push_back( new TH1D( hname, htitle, nxybin, 0., xymax ) );
        hAccZe.back()->SetXTitle( "distance to camera center [deg]" );
        hAccZe.back()->SetYTitle( "relative rate" );
        hAccZe.back()->SetMarkerSize( 2 );
        hAccZe.back()->SetLineWidth( 2 );
        hAccZe.back()->Sumw2();
        hList->Add( hAccZe.back() );

        sprintf( hname, "fAccZe_%d", i );
        fAccZe.push_back( new TF1( hname, VRadialAcceptance_fit_acceptance_function, 0., 2.5, 5 ) );
        fAccZe.back()->SetNpx( 1000 );
        fAccZe.back()->SetParameter( 0, -0.3 );
        fAccZe.back()->SetParameter( 1, -0.6 );
        fAccZe.back()->SetParameter( 2, +0.6 );
        fAccZe.back()->SetParameter( 3, -0.2 );
        fAccZe.back()->SetParameter( 4, 0.2 );
        fAccZe.back()->SetParLimits( 4, 0., 0.5 );
        hList->Add( fAccZe.back() );

        sprintf( hname, "hAccZeFit_%d", i );
        sprintf( htitle, "%.0f < el < %.0f",  90.-fZe[i], 90.-ize );
        hAccZeFit.push_back( new TH1D( hname, htitle, nxybin, 0., xymax ) );
        hAccZeFit.back()->SetXTitle( "distance to camera center [deg]" );
        hAccZeFit.back()->SetYTitle( "relative rate" );
        hAccZeFit.back()->SetLineWidth( 1 );
        hList->Add( hAccZeFit.back() );

        ize = fZe[i];
    }
// azimuth dependent acceptance curves
    for( int i = 0; i < 6; i++ ) fAz.push_back( 60. + (double)i*60. );
    double iAz = 0;
    for( unsigned int i = 0; i < fAz.size(); i++ )
    {
        sprintf( hname, "hAccAz_%d", i );
        sprintf( htitle, "%.0f < Az < %.0f", iAz, fAz[i] );
        hAccAz.push_back( new TH1D( hname, htitle, nxybin, 0., xymax ) );
        hAccAz.back()->SetXTitle( "distance to camera center [deg]" );
        hAccAz.back()->SetYTitle( "relative rate" );
        hAccAz.back()->SetMarkerSize( 2 );
        hAccAz.back()->SetLineWidth( 2 );
        hAccAz.back()->Sumw2();
        hList->Add( hAccAz.back() );

        sprintf( hname, "hAccXY_%d", i );
        sprintf( htitle, "%.0f < Az < %.0f", iAz, fAz[i] );
        hXYAcc.push_back( new TH2D( hname, htitle, 20, -2., 2., 20, -2., 2. ) );
        hXYAcc.back()->SetXTitle( "x_{off} [deg]" );
        hXYAcc.back()->SetYTitle( "y_{off} [deg]" );
        hList->Add( hXYAcc.back() );

        iAz = fAz[i];
    }
    hXYAccTot = new TH2D( "hXYaccTot", "",  40, -2., 2., 40, -2., 2. );
    hXYAccTot->SetXTitle( "x_{off} [deg]" );
    hXYAccTot->SetYTitle( "y_{off} [deg]" );
    hList->Add( hXYAccTot );

// run dependent acceptance curves
    for( unsigned int i = 0; i < fRunPar->fRunList.size(); i++ )
    {
        sprintf( hname, "hAccRun_%d", fRunPar->fRunList[i].fRunOff );
        sprintf( htitle, "run %d", fRunPar->fRunList[i].fRunOff );
        hAccRun.push_back( new TH1D( hname, htitle, nxybin, 0., xymax ) );
        hAccRun.back()->SetXTitle( "distance to camera center [deg]" );
        hAccRun.back()->SetYTitle( "relative rate" );
        hAccRun.back()->SetMarkerSize( 2 );
        hAccRun.back()->SetLineWidth( 2 );
        hAccRun.back()->Sumw2();
        hList->Add( hAccRun.back() );
    }

}


VRadialAcceptance::~VRadialAcceptance()
{
    if( fAccFile ) delete fAccFile;
    if( fAccp5 ) delete fAccp5;
}


void VRadialAcceptance::reset()
{
    degrad = 45./atan(1.);

    fNumberOfRawFiles = 0.;

    fAcceptanceFunctionDefined = false;

    fRunPar = 0;
    fCuts = 0;
    hList = 0;
    fAstro = 0;

    fXs = 0.;
    fYs = 0.;
    fRs = 0.;
    fDs = 0.;
    fMs = 5.;

    fSimuAcceptance = false;
    fAccp5 = 0;
    hAccp5 = 0;
    hscale = 0;
    hXYAccTot = 0;
    fAccFile = 0;

    fXE.clear();
    fYE.clear();
    fRE.clear();
}


void VRadialAcceptance::initAcceptancefromSimulations()
{
    fSimuAcceptance = true;

//   cout << "\t using acceptance curve from simulations" << endl;

//
// Acceptance curve from a crude fit to simulation data for 300 GeV
//
// ignore energy dependence
//
// flux(2,"V2/electron_V2_9_0_2202.root");
//

    if( !gDirectory->Get( "fAccp5" ) )
    {
        fAccp5 = new TF1( "fAccp5", "[0]+x*([1]+x*([2]+x*([3]+x*([4]+x*[5]))))", 0., 3.5 );
        fAccp5->SetParameter( 0, 1./1.02 );
        fAccp5->SetParameter( 1, 0.393958/1.14 );
        fAccp5->SetParameter( 2, -1.799709/1.14 );
        fAccp5->SetParameter( 3, 1.115985/1.14 );
        fAccp5->SetParameter( 4, -0.271452/1.14 );
        fAccp5->SetParameter( 5, 0.023679/1.14 );
        fAccp5->SetNpx( 1000 );
    }

    if( !gDirectory->Get( "hAccp5" ) )
    {
        hAccp5 = new TH1D( "hAccp5", "radial acceptance curve", 1000, 0., 3.5 );
        hAccp5->SetXTitle( "distance to camera center [deg]" );
        hAccp5->SetYTitle( "relative acceptance" );
        hAccp5->SetLineWidth( 2 );
        hAccp5->SetStats( 0 );
        for( int i = 1; i <= hAccp5->GetNbinsX(); i++ ) hAccp5->SetBinContent( i, getAcceptance( hAccp5->GetBinCenter( i ), 0., 0. ) );
    }
}


/*!
 *   get radial acceptance
 */
double VRadialAcceptance::getAcceptance( double x, double y, double erec, double ze )
{
    double idist = sqrt( x*x + y*y );

    double iacc = 1.;

//////////////////////////////
//  from simulations (no zenith angle/energy dependence)
/////////////////////////////

    if( fSimuAcceptance )
    {
// fAccp5 very crude fit, set to 1 for idist < 0.15
        if( idist < 0.15 ) iacc = 1.;
// fAccp5 very crude fit, set to 0 for idist > 2.5
        else if( idist > 2.5 ) iacc = 0.;
        else iacc = fAccp5->Eval( idist );
    }
//////////////////////////////
//  from data
/////////////////////////////
    else
    {
        if( fAcceptanceFunctionDefined )
        {
            if( idist > 2.5 ) iacc = 0.;
            else iacc = fAccZe[0]->Eval( idist );
            if( iacc > 1. ) iacc = 1.;
            if( iacc < 0. ) iacc = 0.;
        }
    }

    return iacc;
}


/*!
 *  correction factor is 1/acceptance
 *
 */
double VRadialAcceptance::getCorrectionFactor( double x, double y, double erec )
{
    double r = getAcceptance( x, y, erec );
    if( r > 0. ) return 1./r;

    return 0.;
}


/*!
 *    define here region in the sky which are to be excluded in the analysis
 *
 *     x,y are camera coordinates (not wobble shifted)
 */
bool VRadialAcceptance::isExcluded( double x, double y )
{
    if( (x*x + y*y) > fMs * fMs ) return true;

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
    if( x*x + y*y > fMs * fMs ) return true;

// source region (source radius + safety (fDS))
    if( fDs >= 0. && ( (x-fXs)*(x-fXs)+(y-fYs)*(y-fYs)) < ((fRs+fDs)*(fRs+fDs)) ) return true;

//Other regions to exclude from background (read from runparameter)
    for( unsigned int i = 0; i< fXE.size(); i ++)
    {
        if( (x-fXE[i]) * (x-fXE[i]) + (y-fYE[i]) * (y-fYE[i]) < fRE[i]*fRE[i] ) return true;
    }
    return false;
}


/*
 *     x,y are camera coordinates (not wobble shifted)
 */
bool VRadialAcceptance::isExcludedfromSource( double x, double y )
{
    return isExcluded( x, y );

    return false;
}


void VRadialAcceptance::setSource( double x, double y, double r, double idist, double imaxdist )
{
    fXs = x;
    fYs = y;
    fRs = r;
    fDs = idist;
    fMs = imaxdist;
}


void VRadialAcceptance::setSource( double x, double y, double r, double idist )
{
    setSource( x, y, r, idist, 5. );
}


void VRadialAcceptance::setRegionToExcludeAcceptance( vector<double> x, vector<double> y, vector<double> r)
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


bool VRadialAcceptance::fillAcceptanceFromData( CData *iData )
{
    if( !iData )
    {
        cout << "VRadialAcceptance::fillAcceptanceFromData: no data tree defined" << endl;
        return false;
    }

    int nentries = iData->fChain->GetEntries();

    cout << "filling acceptance curves with " << nentries << " events (before cuts)" << endl;

    if( fCuts ) fCuts->printCutSummary();

    double idist = 0;
    double iaz = 0.;

    int i_entries_after_cuts = 0;

    int iInitRun = 0;

////////////////////////////////////////////////////////
// loop over input data tree
    cout << "start analyses..." << endl;
    for( int i = 0; i < nentries; i++ )
    {
        iData->GetEntry( i );

        if( i == 0 and iData->isMC() ) cout << "\t (analysing MC data)" << endl;

// first event for this run
        if( !iData->isMC() && iInitRun != iData->runNumber )
        {
            for( unsigned r = 0; r < fRunPar->fRunList.size(); r++ )
            {
                if( fRunPar->fRunList[r].fRunOff == iData->runNumber )
                {
                    iInitRun = iData->runNumber;
// set targets and get coordinates
                    VTargets fTarget;
// set source coordinates from VRunParameter or from list of targets
                    if( fRunPar->fRunList[r].fTargetDecJ2000 > -85. )
                    {
                        fTarget.setTarget( fRunPar->fRunList[r].fTarget, fRunPar->fRunList[r].fTargetRAJ2000, fRunPar->fRunList[r].fTargetDecJ2000 );
                    }
                    else if ( !fTarget.selectTargetbyName( fRunPar->fRunList[r].fTarget ) )
                    {
                        cout << "ERROR in VRadialAcceptance::fillAcceptanceFromData: invalid target " << fRunPar->fRunList[r].fTarget << endl;
                        cout << "\t" << fRunPar->fRunList[r].fTargetDecJ2000 << endl;
                        exit( 0 );
                    }
// get MJD
                    double i_dec = fTarget.getTargetDecJ2000();
                    double i_ra  = fTarget.getTargetRAJ2000();
                    double iMJD = (double)iData->MJD;
                    cout << "Target: " << fRunPar->fRunList[r].fTarget << " (ra,dec)=(";
                    cout << i_ra*degrad << "," << i_dec*degrad << ")" << endl;
                    VSkyCoordinatesUtilities::precessTarget( iMJD, i_ra, i_dec);
                    cout << "(precessed, MJD=" << iMJD << ", (ra,dec)=(";
                    cout << i_ra*degrad << "," << i_dec*degrad << "))" << endl;
                    cout << endl;

                    double i_off = fRunPar->fRunList[r].fPairOffset/60./24.*360./degrad;
                    double i_raDiff = 0.;
                    double i_decDiff = 0.;
                    VSkyCoordinatesUtilities::getWobbleOffsets( fRunPar->fRunList[r].fWobbleNorth, -1.*fRunPar->fRunList[r].fWobbleWest, i_dec*degrad, i_ra*degrad, i_decDiff, i_raDiff );
// does this need a delete?
                    fAstro = new VAstroSource( i_ra - i_raDiff/degrad + i_off ,i_dec - i_decDiff/degrad, i_ra, i_dec );
                    break;
                }
                else fAstro = 0;
            }
        }

// apply some basic quality cuts
        if( fCuts->applyInsideFiducialAreaCut() && fCuts->applyStereoQualityCuts( 0, false, i , true) )
        {
// gamma/hadron cuts
            if( !fCuts->isGamma( 0, false ) ) continue;
//	 if( !fCuts->applyEnergyReconstructionQualityCuts() ) continue;

// now fill histograms
            i_entries_after_cuts++;

// fill zenith angle dependent histograms
            for( unsigned int j = 0; j < fZe.size(); j++ )
            {
                if( iData->Ze < fZe[j] )
                {
                    idist = sqrt( iData->Xoff*iData->Xoff + iData->Yoff*iData->Yoff );
                    if( idist > 0 ) hAccZe[j]->Fill( idist );
                    break;
                }
            }
            double i_xoff = iData->Xoff;
            double i_yoff = iData->Yoff;
            if( iData->isMC() && fAstro ) fAstro->derotateCoords( VSkyCoordinatesUtilities::getUTC(iData->MJD,iData->Time), iData->Xoff, iData->Yoff, i_xoff, i_yoff );
// fill azimuth angle dependent histograms (telescope azimuth)
            for( unsigned int j = 0; j < fAz.size(); j++ )
            {
                iaz = atan2(i_yoff, i_xoff ) * 45./atan(1.)+180.;
                if( iaz < fAz[j] )
                {
                    idist = sqrt( i_xoff*i_xoff + i_yoff*i_yoff );
                    if( idist > 0 ) hAccAz[j]->Fill( idist );
// xy acceptance
                    hXYAcc[j]->Fill( i_xoff, i_yoff );
                    break;
                }
            }
            hXYAccTot->Fill( i_xoff, i_yoff );
// fill run dependent histograms
            for( unsigned int j = 0; j < fRunPar->fRunList.size(); j++ )
            {
                if( iData->runNumber == fRunPar->fRunList[j].fRunOff )
                {
                    idist = sqrt( iData->Xoff*iData->Xoff + iData->Yoff*iData->Yoff );
                    if( idist > 0 ) hAccRun[j]->Fill( idist );
                    break;
                }
            }
        }
    }
    cout << "total number of entries after cuts: " << i_entries_after_cuts << endl;
    cout << endl << endl;

    return true;
}


bool VRadialAcceptance::terminate( string ofile )
{
// scale everything by area of ring
// scale everything to mean value of first three bins
    double isc = 0.;
    if( fAccZeFitMinBin == fAccZeFitMaxBin )
    {
        cout << "Error: normalisation range for acceptance curves not well defined: " << fAccZeFitMinBin << "\t" << fAccZeFitMaxBin << endl;
    }
    else
    {
        double i_normBin = (double)(fAccZeFitMaxBin - fAccZeFitMinBin );
// zenith angle histograms
        for( unsigned int i = 0; i < hAccZe.size(); i++ )
        {
            scaleArea( hAccZe[i] );
            isc = 0.;
            i_normBin = 0.;
            for( unsigned int j = fAccZeFitMinBin; j < fAccZeFitMaxBin; j++ )
            {
                if( hAccZe[i]->GetBinError( j ) > 0. )
                {
                    isc +=  hAccZe[i]->GetBinContent( j )/(hAccZe[i]->GetBinError( j ) * hAccZe[i]->GetBinError( j ));
                    i_normBin += 1./hAccZe[i]->GetBinError( j )/hAccZe[i]->GetBinError( j );
                }
            }
            if( i_normBin > 0. ) isc /= i_normBin;
            if( isc > 0 ) hAccZe[i]->Scale( 1./isc );
        }
// az histograms
        for( unsigned int i = 0; i < hAccAz.size(); i++ )
        {
            scaleArea( hAccAz[i] );
            isc = 0.;
            for( unsigned int j = fAccZeFitMinBin; j < fAccZeFitMaxBin; j++ ) isc +=  hAccAz[i]->GetBinContent( j )/i_normBin;
            if( isc > 0 ) hAccAz[i]->Scale( 1./isc );
        }
// run wise histograms
        for( unsigned int i = 0; i < hAccRun.size(); i++ )
        {
            isc = 0.;
            scaleArea( hAccRun[i] );
            for( unsigned int j = fAccZeFitMinBin; j < fAccZeFitMaxBin; j++ ) isc +=  hAccRun[i]->GetBinContent( j )/i_normBin;
            if( isc > 0 ) hAccRun[i]->Scale( 1./isc );
        }
    }

// analyze ze histograms

// fill the fitting histogram
    for( unsigned int i = 0; i < hAccZe.size(); i++ )
    {
        for( int j = 1; j < hAccZeFit[i]->GetNbinsX(); j++ )
        {
            if( j < 0 ) hAccZeFit[i]->SetBinContent( j, 1. );
            else        hAccZeFit[i]->SetBinContent( j, hAccZe[i]->GetBinContent( j )) ;
        }
    }

// fit the data and fill histograms
    cout << "fitting acceptance curves ..." << endl << endl;
    double i_eval = 0.;
    for( unsigned int i = 0; i < hAccZe.size(); i++ )
    {
        hAccZeFit[i]->Fit( fAccZe[i], "0REM" );
        hAccZeFit[i]->SetBins( 1000, 0., 5. );
        for( int j = 1; j < hAccZeFit[i]->GetNbinsX(); j++ )
        {
            i_eval = fAccZe[i]->Eval( hAccZeFit[i]->GetBinCenter( j ) );
            if( hAccZeFit[i]->GetBinCenter( j ) > 3.5 ) i_eval = 0.;
            else if( hAccZeFit[i]->GetBinCenter( j ) < fAccZe[i]->GetMaximumX() ) i_eval = 1.;
            else if( i_eval < 0. )  i_eval = 0.;
            else if( i_eval >= 1. ) i_eval = 1.;

            hAccZeFit[i]->SetBinContent( j, i_eval );
        }
    }
    cout << "-----------------------------------------" << endl << endl;

// write total number of entries in each histogram (ze) to screen
    cout << "total number of events per zenith angle bin " << endl;
    for( unsigned int i = 0; i < hAccZe.size(); i++ )
    {
        cout << "\t" << hAccZe[i]->GetName() << "\t" << hAccZe[i]->GetEntries() << endl;
    }

// write everything to disk

    TFile *fo = new TFile( ofile.c_str(), "RECREATE" );
    if( fo->IsZombie() )
    {
        cout << "VRadialAcceptance::fillAcceptanceFromData error opening output file " << ofile << endl;
        return false;
    }
    cout << endl << "writing acceptance curves to " << ofile << endl;

    hList->Write();

    fo->Close();

    return true;
}


void VRadialAcceptance::scaleArea( TH1D *h )
{
    double iA = 0.;

    for( int i = 1; i <= h->GetNbinsX(); i++ )
    {
        double iL = h->GetXaxis()->GetBinLowEdge( i );
        double iU = h->GetXaxis()->GetBinLowEdge( i ) + h->GetXaxis()->GetBinWidth( i );

        iA = TMath::Pi() * ( iU*iU - iL*iL );

        if( iA > 0. )
        {
            h->SetBinContent( i, h->GetBinContent(i)/iA );
            h->SetBinError( i, h->GetBinError(i)/iA );
        }
    }
}


double VRadialAcceptance::getNumberofRawFiles()
{
    return fNumberOfRawFiles;
}


/*
   fit function for acceptance curves

   pol5 with x < x0 == 1 and f(x0)' == 0

   p0 = a2
*/
Double_t VRadialAcceptance_fit_acceptance_function(Double_t *x, Double_t *par)
{
    double f = 0.;

    double a2 = par[0];
    double a3 = par[1];
    double a4 = par[2];
    double a5 = par[3];
    double x0 = par[4];

    double a1 = -1.*( x0 * ( 2.*a2 + x0 * ( 3.*a3 + x0 * ( 4.*a4 + x0 * 5. * a5 ) ) ) );
    double a0 = 1. - (x0 * (    a1 + x0 * (    a2 + x0 * (    a3 + x0 * ( a4 + x0 * a5 ) ) ) ) );

    if( x[0] < x0 ) f = 1.;
    else
    {
        f = a0 + x[0] * (    a1 + x[0] * (    a2 + x[0] * (    a3 + x[0] * ( a4 + x[0] * a5 ) ) ) );
    }
    return f;
}
