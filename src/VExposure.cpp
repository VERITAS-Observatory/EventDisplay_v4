/*! \class VExposure
    \brief calculate VERITAS exposure

    hardcoded latitude/longitude of VERITAS

    Revision $Id: VExposure.cpp,v 1.1.2.3.2.2.2.1.4.1.2.5.4.2 2011/04/06 11:16:55 gmaier Exp $

    \author Gernot Maier
*/

#include "VExposure.h"

ClassImp(VExposure)

VExposure::VExposure( int nBinsL, int nBinsB )
{
    fDebug = false;

// VERITAS values
    fObsLongitude = 110.952 * atan(1.)/45;
    fObsLatitude =   31.675 * atan(1.)/45.;

    fStartDate = "2007-09-01";
    fStopDate =  "2009-12-31";

    fAcceptance = 0;

    fPlotExtendedSources = true;
    fPlotSourceNames = true;
    fVDB_ObservingSources = 0;

    setMaximumIntegrationRadius();

    double iBMin = -90.;
    double iBMax = 90.;

    fMapGal2D = new TH2D( "fMapGal2D", "", nBinsL, -180., 180., nBinsB, iBMin, iBMax );
    fMapGal2D->SetStats( 0 );
    fMapGal2D->SetXTitle( "Galactic longitude [deg]" );
    fMapGal2D->SetYTitle( "Galactic latitude [deg]" );

    fRadAccMapGal2D = new TH2D( "fRadAccMapGal2D", "", nBinsL, -180., 180., nBinsB, iBMin, iBMax );
    fRadAccMapGal2D->SetStats( 0 );
    fRadAccMapGal2D->SetXTitle( "Galactic longitude [deg]" );
    fRadAccMapGal2D->SetYTitle( "Galactic latitude [deg]" );

    fMapGal2D_aitoff = new TH2D( "fMapGal2D_aitoff", "", nBinsL, -180., 180., (int)(nBinsB*2./3.), iBMin, iBMax );
    fMapGal2D_aitoff->SetStats( 0 );
    fMapGal2D_aitoff->SetXTitle( "Galactic longitude [deg]" );
    fMapGal2D_aitoff->SetYTitle( "Galactic latitude [deg]" );

    fRadAccMapGal2D_aitoff = new TH2D( "fRadAccMapGal2D_aitoff", "", nBinsL, -180., 180., (int)(nBinsB*2./3.), iBMin, iBMax );
    fRadAccMapGal2D_aitoff->SetStats( 0 );
    fRadAccMapGal2D_aitoff->SetXTitle( "Galactic longitude [deg]" );
    fRadAccMapGal2D_aitoff->SetYTitle( "Galactic latitude [deg]" );

}


/* 
   set more contours for the colz map
*/
void VExposure::set_plot_style()
{
    const Int_t NRGBs = 5;
    const Int_t NCont = 255;

    Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
    Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
    Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
    Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    gStyle->SetNumberContours(NCont);
}


void VExposure::setTimeRange( string iStart, string iStop )
{
    fStartDate = iStart;
    fStopDate  = iStop;
}


bool VExposure::setPlannedObservation(vector<double> ra, vector<double> dec, vector<double> t)
{
  
  for (int unsigned i=0; i<ra.size(); i++)
    {
      double i_b=0;
      double i_l=0;

      slaEqgal( ra[i] / 180. * TMath::Pi(), dec[i] / 180. * TMath::Pi(), &i_l, &i_b );
      fRunGalLong1958.push_back( i_l * 180./TMath::Pi() );
      fRunGalLat1958.push_back( i_b * 180./TMath::Pi() );
      fRunDuration.push_back( t[i] );
      if (fDebug) cout << " " << ra[i] << " " << dec[i] << " ";
      if (fDebug) cout << " " << fRunGalLong1958.back() << " " << fRunGalLat1958.back() << " " << flush;
    }

  return true;
}

bool VExposure::readFromDB()
{
    string iTempS;
    iTempS = getDBServer() + "/VERITAS";
    TSQLServer *f_db = TSQLServer::Connect( iTempS.c_str(), "readonly", "" );
    if( !f_db )
    {
        cout << "error connecting to db" << endl;
        return false;
    }
    cout << "reading from " << iTempS << endl;

    char c_query[1000];

    sprintf( c_query, "select * from tblRun_Info where db_start_time >= \"%s.000000\"  and db_start_time < \"%s.160000\"", fStartDate.c_str(), fStopDate.c_str() );

    cout << c_query << endl;

    TSQLResult *db_res = f_db->Query( c_query );
    if( !db_res )
    {
        return false;
    }

    int fNRows = db_res->GetRowCount();

    cout << "number of runs read form db: " << fNRows << endl;

    string itemp;
    for( int j = 0; j < fNRows; j++ )
    {
        TSQLRow *db_row = db_res->Next();

        if( !db_row ) break;

//////
// all fields should be defined (check if field #19 is there)
        if( !db_row->GetField( 19 ) ) continue;
        itemp = db_row->GetField( 19 );
// don't use laser or charge injection runs
        if( itemp == "NOSOURCE" ) continue;

//////
// check if this run is an observing run
        if( !db_row->GetField( 1 ) ) continue;
        itemp = db_row->GetField( 1 );
        if( itemp != "observing" ) continue;

//////
// get run status
        if( !db_row->GetField( 3 ) ) continue;
        itemp = db_row->GetField( 3 );
// don't use aborted runs
        if( itemp == "aborted" ) continue;
// don't use runs which are started only
        if( itemp == "started" ) continue;
// don't use runs which are defined only
        if( itemp == "defined" ) continue;
// don't use runs which are prepared only
        if( itemp == "prepared" ) continue;

        if( fDebug ) cout << j << " " << db_row->GetField( 19 ) <<  " ";
        if( fDebug ) cout << db_row->GetField( 1 ) << " " << db_row->GetField( 3 ) << " " << flush;

//////
// get source coordinates
        double iRa = 0.;
        double iDec = 0.;
        itemp = db_row->GetField( 19 );
        if( !getDBSourceCoordinates( f_db, itemp, iDec, iRa ) )
        {
            cout << "No coordinates available: " << db_row->GetField( 0 ) << " " << itemp << endl;
            continue;
        } 

        fRunSourceID.push_back( itemp );
        fRunRA.push_back( iRa );
        fRunDec.push_back( iDec );

        fRunoffsetRA.push_back( atof( db_row->GetField( 15 ) ) * 180./TMath::Pi() );
        fRunoffsetDec.push_back( atof( db_row->GetField( 16 ) ) * 180./TMath::Pi() );
        fRunConfigMask.push_back( atoi( db_row->GetField( 10 ) ) );

//////
// get galactic coordinates
        double i_b = 0.;
        double i_l = 0.;
        iRa  += fRunoffsetRA.back();
        iDec += fRunoffsetDec.back();

        slaEqgal( iRa / 180. * TMath::Pi(), iDec / 180. * TMath::Pi(), &i_l, &i_b );
        fRunGalLong1958.push_back( i_l * 180./TMath::Pi() );
        fRunGalLat1958.push_back( i_b * 180./TMath::Pi() );
        if( fDebug ) cout << " " << iRa << " " << iDec << " ";
        if( fDebug ) cout << " " << fRunGalLong1958.back() << " " << fRunGalLat1958.back() << " " << flush;

//////
// calculate MJD, etc.
        int iMJD = 0;
        double iTime1 = 0.;
        double iTime2 = 0.;

	if( !db_row->GetField( 6 ) ) continue;
        itemp = db_row->GetField( 6 );
        getDBMJDTime( itemp, iMJD, iTime1, true );
        fRunStartMJD.push_back( (double)iMJD + iTime1/86400 );
        if( fDebug ) cout << "T1 " << iMJD << " " << iTime1 << " ";
	if( !db_row->GetField( 7 ) ) continue;
        itemp = db_row->GetField( 7 );
        if( fDebug ) cout << itemp << " " << flush;
        getDBMJDTime( itemp, iMJD, iTime2, true );
        if( fDebug ) cout << "T2 " << iMJD << " " << iTime2 << " ";
        fRunStopMJD.push_back( (double)iMJD + iTime2/86400 );
        fRunDuration.push_back( iTime2 - iTime1 );

        if( fDebug ) cout << fRunDuration.back() << " " << flush;

        itemp = db_row->GetField( 0 );
        if( fDebug ) cout << "RUN " << itemp << " " << flush;
        fRun.push_back( atoi( itemp.c_str() ) );

//       itemp = db_row->GetField( 2 );
        fRunStatus.push_back( "" );

//////
// get local coordinates

        double ha = 0.;
        double iSid = 0.;
        double az, el;
// get Greenwich sideral time
        iSid = slaGmsta( (double)iMJD, (iTime1+iTime2)/2./86400. );
// calculate local sideral time
        iSid = iSid - fObsLongitude;
// calculate hour angle
        ha = slaDranrm( iSid - iRa / 180. * TMath::Pi() );
// get horizontal coordinates
        slaDe2h( ha, iDec / 180. * TMath::Pi(), fObsLatitude, &az, &el );
// fill vectors
        fRunTelElevation.push_back( el * 180. / TMath::Pi() );
        fRunTelAzimuth.push_back( az * 180. / TMath::Pi() );

////////////////////////////////////////////////////////////////////////
        if( fDebug ) cout << fRunStatus.back() << " " << fRun.back() << flush;

        if( fDebug ) cout << endl;
    }
    if( f_db ) f_db->Close();

    cout << "total number of runs in runlist: " << fRunStatus.size() << endl;

    return true;
}


void VExposure::resetDataVectors()
{
    fRun.clear();
    fRunConfigMask.clear();
    fRunStartMJD.clear();
    fRunStopMJD.clear();
    fRunDuration.clear();
    fRunRA.clear();
    fRunDec.clear();
    fRunoffsetRA.clear();
    fRunoffsetDec.clear();
    fRunGalLong1958.clear();
    fRunGalLat1958.clear();
    fRunTelElevation.clear();
    fRunTelAzimuth.clear();
}


bool VExposure::readRootFile( string iRFile, double iMinMJD, double iMaxMJD )
{
    resetDataVectors();

    TFile f( iRFile.c_str() );
    if( f.IsZombie() ) return false;

    int iRun = 0;
    Char_t iSourceID[300];
    unsigned int iConfigMask = 0;
    double iRunStartMJD = 0.;
    double iRunStoppMJD = 0.;
    double iRunDuration = 0.;
    double iRunRA = 0.;
    double iRunDec = 0.;
    double iRunRAoffset = 0.;
    double iRunDecoffset = 0.;
    double l = 0.;
    double b = 0.;
    double el = 0.;
    double az = 0.;

    TTree *t = (TTree*)f.Get( "fRunTable" );
    if( !t ) return false;

    t->SetBranchAddress( "runNumber", &iRun );
    t->SetBranchAddress( "sourceID", &iSourceID );
    t->SetBranchAddress( "configmask", &iConfigMask );
    t->SetBranchAddress( "MJDstart", &iRunStartMJD );
    t->SetBranchAddress( "MJDstop", &iRunStoppMJD );
    t->SetBranchAddress( "runLength", &iRunDuration );
    t->SetBranchAddress( "ra", &iRunRA );
    t->SetBranchAddress( "dec", &iRunDec );
    t->SetBranchAddress( "raOffset", &iRunRAoffset );
    t->SetBranchAddress( "decOffset", &iRunDecoffset );
    t->SetBranchAddress( "l", &l );
    t->SetBranchAddress( "b", &b );
    t->SetBranchAddress( "telEl", &el );
    t->SetBranchAddress( "telAz", &az );

    double i_tot = 0.;

    for( int i = 0; i < t->GetEntries(); i++ )
    {
        t->GetEntry( i );

	if( iMinMJD > 0. && iRunStartMJD < iMinMJD ) continue;
	if( iMaxMJD > 0. && iRunStartMJD > iMaxMJD ) continue;

        fRun.push_back( iRun );
        fRunSourceID.push_back( iSourceID );
        fRunConfigMask.push_back( iConfigMask );
        fRunStartMJD.push_back( iRunStartMJD );
        fRunStopMJD.push_back( iRunStoppMJD );
        fRunDuration.push_back( iRunDuration );
        i_tot += iRunDuration;
        fRunRA.push_back( iRunRA );
        fRunDec.push_back( iRunDec );
        fRunoffsetRA.push_back( iRunRAoffset );
        fRunoffsetDec.push_back( iRunDecoffset );
        if( l > 180. ) l -= 360.;
        fRunGalLong1958.push_back( l );
        fRunGalLat1958.push_back( b );
        fRunTelElevation.push_back( el );
        fRunTelAzimuth.push_back( az );
    }

    f.Close();

    cout << "total number of entries read in: " << fRunGalLat1958.size() << endl;
    cout << "total observing time: " << i_tot / 3600. << " [h]" << endl;

    return true;
}


bool VExposure::writeRootFile( string iOfile )
{
    if( iOfile.size() <= 0 ) return false;

    int iRun;
    Char_t iRunSourceID[300];
    unsigned int iConfigMask;
    double iRunStartMJD;
    double iRunStoppMJD;
    double iRunDuration;
    double iRunRA;
    double iRunDec;
    double iRunRAoffset;
    double iRunDecoffset;
    double l;
    double b;
    double el;
    double az;

    TFile f( iOfile.c_str(), "RECREATE" );
    if( f.IsZombie() ) return false;
    cout << "writing data to " << f.GetName() << endl;

    char hname[2000];
    sprintf( hname, "DB entries, %s to %s", fStartDate.c_str(), fStopDate.c_str() );
    TTree t( "fRunTable", hname );
    t.Branch( "runNumber", &iRun, "runNumber/I" );
    t.Branch( "sourceID", &iRunSourceID, "sourceID/C" );
    t.Branch( "configmask", &iConfigMask, "configmask/i" );
    t.Branch( "MJDstart", &iRunStartMJD, "MJDstart/D" );
    t.Branch( "MJDstop", &iRunStoppMJD, "MJDstop/D" );
    t.Branch( "runLength", &iRunDuration, "runLength/D" );
    t.Branch( "ra", &iRunRA, "ra/D" );
    t.Branch( "dec", &iRunDec, "dec/D" );
    t.Branch( "raOffset", &iRunRAoffset, "raOffset/D" );
    t.Branch( "decOffset", &iRunDecoffset, "decOffset/D" );
    t.Branch( "l", &l, "l/D" );
    t.Branch( "b", &b, "b/D" );
    t.Branch( "telEl", &el, "telEl/D" );
    t.Branch( "telAz", &az, "telAz/D" );

    for( unsigned int i = 0; i < fRun.size(); i++ )
    {
        iRun = fRun[i];
        if( fRunSourceID[i].size() < 300 ) sprintf( iRunSourceID, "%s", fRunSourceID[i].c_str() );
        else                               sprintf( iRunSourceID, "%s", fRunSourceID[i].substr( 0, 299 ).c_str() );
        iConfigMask = fRunConfigMask[i];
        iRunStartMJD = fRunStartMJD[i];
        iRunStoppMJD = fRunStopMJD[i];
        iRunDuration = fRunDuration[i];
        iRunRA = fRunRA[i];
        iRunDec = fRunDec[i];
        iRunRAoffset = fRunoffsetRA[i];
        iRunDecoffset = fRunoffsetDec[i];
        l = fRunGalLong1958[i];
        b = fRunGalLat1958[i];
        el = fRunTelElevation[i];
        az = fRunTelAzimuth[i];

        t.Fill();
    }
    t.Write();

    f.Close();

    return true;
}


void VExposure::fillExposureMap()
{
    cout << "fill exposure" << endl;

    fMapGal2D->Reset();
    fRadAccMapGal2D->Reset();
    fMapGal2D_aitoff->Reset();
    fRadAccMapGal2D_aitoff->Reset();

    int    i_r_b = (int)(fMaximumIntegrationRadius/fMapGal2D->GetYaxis()->GetBinWidth( 2 )+0.5);

    double r_dist = 0.;

    for( unsigned int i = 0; i < fRunGalLong1958.size(); i++ )
    {
        double l = fRunGalLong1958[i];
        if( l > 180. ) l -= 360.;
        int i_l = fMapGal2D->GetXaxis()->FindBin( l );
        int i_b = fMapGal2D->GetYaxis()->FindBin( fRunGalLat1958[i] );

        int i_b_start = i_b - i_r_b;
        if( i_b_start < 1 ) i_b_start = 1;
        int i_b_stop = i_b + i_r_b;
        if( i_b_stop > fMapGal2D->GetNbinsY() ) i_b_stop = fMapGal2D->GetNbinsY();

        for( int b = i_b_start; b <= i_b_stop; b++ )
        {
            double b_pos = fMapGal2D->GetYaxis()->GetBinCenter( b );

// calculate extension in longitude
            int i_r_l = 1;
            if( cos(b_pos*TMath::Pi()/180.) > 0. ) i_r_l = (int)(fMaximumIntegrationRadius/cos(b_pos*TMath::Pi()/180.)/fMapGal2D->GetXaxis()->GetBinWidth( 2 )+0.5);
            else                                   i_r_l = 0;
            int i_l_start = i_l - i_r_l;
            if( i_l_start < 1 ) i_l_start = 1;
            int i_l_stop = i_l + i_r_l;
            if( i_l_stop > fMapGal2D->GetNbinsX() ) i_l_stop = fMapGal2D->GetNbinsX();

            for( int l = i_l_start; l <= i_l_stop; l++ )
            {
                double l_pos = fMapGal2D->GetXaxis()->GetBinCenter( l );

                r_dist = slaDsep( l_pos*TMath::Pi()/180., b_pos*TMath::Pi()/180., fRunGalLong1958[i]*TMath::Pi()/180., fRunGalLat1958[i]*TMath::Pi()/180. ) * 180./TMath::Pi();
                if( r_dist < fMaximumIntegrationRadius && fRunDuration[i] > 0. )
                {
// galactic longitudes are from 180. to -180.
                    l_pos *= -1.;
                    fMapGal2D->Fill( l_pos, b_pos, fRunDuration[i]/3600. );
                    fRadAccMapGal2D->Fill( l_pos, b_pos, fRunDuration[i]/3600. * getAcceptance( r_dist ) );
                }
            }
        }
    }
    cout << "entries " << fMapGal2D->GetEntries() << endl;

/////////////////////////////////
// calculate aitoff projection
    double al = 0.;
    double ab = 0.;
    double xl = 0.;
    double xb = 0.;
    int bl = 0;
    int bb = 0;

    for( int i = 1; i <= fMapGal2D->GetNbinsX(); i++ )
    {
        xl = fMapGal2D->GetXaxis()->GetBinCenter( i );
        for( int j = 1; j <= fMapGal2D->GetNbinsY(); j++ )
        {
            xb = fMapGal2D->GetYaxis()->GetBinCenter( j );

            aitoff2xy( -1.*xl, xb, al, ab );

            bl = fMapGal2D_aitoff->GetXaxis()->FindBin( al );
            bb = fMapGal2D_aitoff->GetYaxis()->FindBin( ab );

            fMapGal2D_aitoff->SetBinContent( bl, bb, fMapGal2D->GetBinContent( i, j ) );
            fRadAccMapGal2D_aitoff->SetBinContent( bl, bb, fRadAccMapGal2D->GetBinContent( i, j ) );
        }
    }
// now plot everything

    gStyle->SetPalette( 1 );
    set_plot_style();

}


TCanvas* VExposure::plot( double ilmin, double ilmax, double ibmin, double ibmax, unsigned int iReturnCanvas )
{
    fPlottingCanvas.clear();
    fPlottingCanvas.push_back( plot2DGalactic( "cGal", "Exposure", 10, 10, 600, 400, fMapGal2D, ibmin, ibmax, ilmin, ilmax, false ) );
    fPlottingCanvas.push_back( plot2DGalactic( "cGal_aitoff", "Exposure (aitoff)", 10, 450, 600, 400, fMapGal2D_aitoff, ibmin, ibmax, ilmin, ilmax, true ) );
    fPlottingCanvas.push_back( plot2DGalactic( "cAccGal", "Acceptance corrected exposure", 650, 10, 600, 400, fRadAccMapGal2D, ibmin, ibmax, ilmin, ilmax, false ) );
    fPlottingCanvas.push_back( plot2DGalactic( "cAccGal_aitoff", "Acceptance corrected exposure (aitoff)", 650, 450, 600, 400, fRadAccMapGal2D_aitoff, ibmin, ibmax, ilmin, ilmax, true ) );

    if( iReturnCanvas < fPlottingCanvas.size() ) return fPlottingCanvas[iReturnCanvas];

    return 0;
}


void VExposure::plot_HESSSkySurvey( TCanvas *c )
{
    if( !c ) return;

    c->cd();

// first phase
    TBox *ib1 = new TBox( -30., -3., 30., 3 );
    ib1->SetFillStyle( 0 );
    ib1->SetLineColor( 14 );
    ib1->SetLineStyle( 2 );
    ib1->Draw();
    TBox *ib2 = new TBox( 30., -3., 80., 3 );
    ib2->SetFillStyle( 0 );
    ib2->SetLineColor( 14 );
    ib2->SetLineStyle( 2 );
    ib2->Draw();
    TBox *ib3 = new TBox( -60., -3., -30., 3 );
    ib3->SetFillStyle( 0 );
    ib3->SetLineColor( 14 );
    ib3->SetLineStyle( 2 );
    ib3->Draw();
}


TCanvas* VExposure::plot2DGalactic( string iName, string iTitle, int ix, int iy, int iwx, int iwy, TH2D *h, double ibmin, double ibmax, double ilmin, double ilmax, bool bAitoff )
{
    if( !h ) return 0;

// canvas
    TCanvas *cGal = new TCanvas( iName.c_str(), iTitle.c_str(), ix, iy, iwx, iwy );
    cGal->SetGridx( 0 );
    cGal->SetGridy( 0 );
    cGal->SetRightMargin( 0.15 );
    cGal->Draw();

// set axis ranges of galactic map
    h->SetAxisRange( ibmin, ibmax, "Y" );
    h->SetAxisRange( -1.*ilmax, -1.*ilmin, "X" );

    h->Draw( "A colz" );
    h->SetZTitle( "exposure [h]" );

// plot axis

    TF1 *IncValues = new TF1("IncValues", "-x", ilmin, ilmax );
    IncValues->SetNpx( 500 );

    TGaxis *decAxis = new TGaxis( cGal->GetUxmin()-ilmax-h->GetXaxis()->GetBinWidth( 1 )/2., cGal->GetUymin()+ibmin-h->GetXaxis()->GetBinWidth( 1 )/2., cGal->GetUxmin()-ilmax-h->GetXaxis()->GetBinWidth( 1 )/2., cGal->GetUymin()+ibmax + h->GetXaxis()->GetBinWidth( 1 )/2., ibmin, ibmax, 505, "-"  );
    decAxis->SetTitleFont(h->GetXaxis()->GetTitleFont());
    decAxis->SetTitleSize(h->GetXaxis()->GetTitleSize());
    decAxis->SetTitleOffset( h->GetXaxis()->GetTitleOffset() );
    decAxis->SetLabelFont(h->GetXaxis()->GetLabelFont());
    decAxis->SetLabelSize(h->GetXaxis()->GetLabelSize());
    decAxis->SetLabelColor(h->GetXaxis()->GetLabelColor());
    decAxis->SetLabelOffset( 0.020 );
    decAxis->SetTextColor(h->GetXaxis()->GetTitleColor());
    decAxis->SetLineColor(h->GetXaxis()->GetAxisColor());
    decAxis->SetTitle( "galactic longitude [deg]" );
//  decAxis->SetNdivisions( 510 );
    decAxis->SetTitle( "galactic latitude [deg]" );
    decAxis->Draw();

//   TGaxis *raLowerAxis = new TGaxis( cGal->GetUxmin()-ilmax - h->GetYaxis()->GetBinWidth( 1 )/2., cGal->GetUymin()+ibmin-h->GetYaxis()->GetBinWidth(1)/2. , cGal->GetUxmin()-ilmin+h->GetYaxis()->GetBinWidth( 1 )/2., cGal->GetUymin()+ibmin -h->GetYaxis()->GetBinWidth(1)/2. , "IncValues", 4 );
    TGaxis *raLowerAxis = new TGaxis( cGal->GetUxmin()-ilmax - h->GetYaxis()->GetBinWidth( 1 )/2., cGal->GetUymin()+ibmin-h->GetYaxis()->GetBinWidth(1)/2. , cGal->GetUxmin()-ilmin+h->GetYaxis()->GetBinWidth( 1 )/2., cGal->GetUymin()+ibmin -h->GetYaxis()->GetBinWidth(1)/2. , "IncValues", 4 );
    raLowerAxis->SetTitleFont(h->GetXaxis()->GetTitleFont());
    raLowerAxis->SetTitleSize(h->GetXaxis()->GetTitleSize());
    raLowerAxis->SetTitleOffset( h->GetXaxis()->GetTitleOffset() );
    raLowerAxis->SetLabelFont(h->GetXaxis()->GetLabelFont());
    raLowerAxis->SetLabelSize(h->GetXaxis()->GetLabelSize());
    raLowerAxis->SetLabelColor(h->GetXaxis()->GetLabelColor());
    raLowerAxis->SetLabelOffset( 0.011 );
    raLowerAxis->SetTextColor(h->GetXaxis()->GetTitleColor());
    raLowerAxis->SetLineColor(h->GetXaxis()->GetAxisColor());
    raLowerAxis->SetTitle( "galactic longitude [deg]" );
//   raLowerAxis->SetNdivisions( 510 );
    raLowerAxis->Draw();

    for( unsigned int t = 0; t < fCatalogue.size(); t++ )
    {
        if( fCatalogue[t].size() > 0 )
	{
	   analyseCatalogue( fCatalogue[t], ibmin, ibmax, ilmin, ilmax, h, bAitoff, fCatalogueMarkerStyle[t], fCatalogueMarkerColor[t], fCatalogueTextAngle[t] );
        }
    }

// draw aitoff coordinate system
    if( bAitoff ) drawAitoffCoordinateSystem();

    cGal->Update();

    return cGal;
}


/*
   copied from ROOT forum #6650
*/
void VExposure::drawAitoffCoordinateSystem()
{
    double radeg = TMath::Pi() / 180.;
    double la, lo, x, y, z;

    const int Nl = 11;                            // Number of drawn latitudes
    const int NL = 10;                            // Number of drawn longitudes
    int       M  = 60;

    TGraph  *latitudes[Nl];
    TGraph  *longitudes[NL];

    for( int j = 0; j < Nl; ++j)
    {
        latitudes[j] = new TGraph();

        la = -90.+180./(Nl-1)*j;
        for( int i = 0; i < M+1; ++i )
        {
            lo = -180. + 360./M*i;
            z  = sqrt(1+cos(la*radeg)*cos(lo*radeg/2.));
            x  = 180.*cos(la*radeg)*sin(lo*radeg/2.)/z;
            y  = 90.*sin(la*radeg)/z;
            latitudes[j]->SetPoint(i,x,y);
        }
    }
    for( int j = 0; j < Nl; ++j ) latitudes[j]->Draw("l");

    for( int j=0; j <NL; ++j)
    {
        longitudes[j] = new TGraph();
        lo = -180.+360./(NL-1)*j;
        for( int i = 0; i < M+1;++i)
        {
            la = -90.+180./M*i;
            z  = sqrt(1+cos(la*radeg)*cos(lo*radeg/2.));
            x  = 180.*cos(la*radeg)*sin(lo*radeg/2.)/z;
            y  = 90.*sin(la*radeg)/z;
            longitudes[j]->SetPoint(i,x,y);
        }
    }

    for( int j = 0; j < NL; ++j ) longitudes[j]->Draw("l");
}


void VExposure::analyseCatalogue( string iCatalogue, double ibmin, double ibmax, double ilmin, double ilmax, TH2D *h, bool bAitoff, int iMarkerStyle, int iMarkerColor, double iTextAngle )
{
    VStarCatalogue *s = new VStarCatalogue();
    s->init( 54626., iCatalogue );
//    s->printCatalogue();

    double l = 0.;
    double b = 0.;
    string l_name = "";

    double ib_range = ibmax - ibmin;
    double il_range = ilmax - ilmin;

    cout << "total number of objects in catalogue: " << s->getNStar() << endl;
    for( unsigned int i = 0; i < s->getNStar(); i++ )
    {
        l = s->getStar_l(i);
        b = s->getStar_b(i);
        l_name = s->getStarName( i );

        if( iCatalogue == "VERITASDB" && l_name.substr( 0, 2 ) == "SS" ) continue;
        if( iCatalogue == "VERITASDB" && l_name.substr( 0, 3 ) == "GRB" ) continue;

        if( l > 180. ) l -= 360.;
        if( l > ilmin && l < ilmax && b > ibmin && b < ibmax )
        {
// aitoff projections?
            double ab = b;
            double al = -1.*l;
            if( bAitoff ) aitoff2xy( l, b, al, ab );
// extended?
            if( s->getStarMajorDiameter(i) > 0. && fPlotExtendedSources )
            {
                TEllipse *m = new TEllipse( al, ab, s->getStarMajorDiameter(i), s->getStarMajorDiameter(i) );
                m->SetFillStyle( 0 );
                m->SetLineColor( iMarkerColor );
                m->Draw();
            }
            else
            {
                TMarker *m = new TMarker( al, ab, 5 );
                m->SetMarkerColor( iMarkerColor );
                m->SetMarkerStyle( iMarkerStyle );
                m->Draw();
            }

            if( h )
            {
                if( bAitoff ) aitoff2xy( l, b, al, ab );
                else
                {
                    al = -1.*l;
                    ab = b;
                }
                int il = h->GetXaxis()->FindBin( al );
                int ib = h->GetYaxis()->FindBin( ab );
                if( h->GetBinContent( il, ib ) > 0. )
                {
                    cout << h->GetBinContent( il, ib )*60.;
                    cout << "  [min] exposure on object " << l_name;
                    cout << "(l,b)=(" << l << ", " << b << ")\t";
                    cout << endl;
                }
                if( fPlotSourceNames )
                {
		    if( bAitoff )
		    {
		       al = al+il_range*0.01 * ( cos( iTextAngle*TMath::DegToRad() ) + sin( iTextAngle*TMath::DegToRad() ) );
		       ab = ab+ib_range*0.01 * ( -1.*sin( iTextAngle*TMath::DegToRad() ) + cos( iTextAngle*TMath::DegToRad() ) );
		    }
		    else
		    {
		       al = -1.*l+il_range*0.01 * ( cos( iTextAngle*TMath::DegToRad() ) + sin( iTextAngle*TMath::DegToRad() ) );
		       ab = b+ib_range*0.01 * ( -1.*sin( iTextAngle*TMath::DegToRad() ) + cos( iTextAngle*TMath::DegToRad() ) );
                    }
                    TText *t = new TText( al, ab, l_name.c_str() );
                    t->SetTextColor( iMarkerColor );
                    t->SetTextSize( t->GetTextSize()*0.3 );
                    t->SetTextAngle( iTextAngle );
                    t->Draw();
                }
            }
        }
    }
}


void VExposure::getDBMJDTime( string itemp, int &MJD, double &Time, bool bStrip )
{
    if( itemp.size() < 16 )
    {
        MJD = 0;
        Time = 0.;
        return;
    }
    if( bStrip )
    {
        itemp.replace( itemp.find( "-" ), 1, "" );
        itemp.replace( itemp.find( "-" ), 1, "" );
        itemp.replace( itemp.find( " " ), 1, "" );
        itemp.replace( itemp.find( ":" ), 1, "" );
        itemp.replace( itemp.find( ":" ), 1, "" );
    }
    int y, m, d, h, min, s, ms, l;
    double gMJD;
// get y, m, d
    y = atoi( itemp.substr( 0, 4 ).c_str() );
    m = atoi( itemp.substr( 4, 2 ).c_str() );
    d = atoi( itemp.substr( 6, 2 ).c_str() );
    h = atoi( itemp.substr( 8, 2 ).c_str() );
    min = atoi( itemp.substr( 10, 2 ).c_str() );
    s = atoi( itemp.substr( 12, 2 ).c_str() );
    if( !bStrip ) ms = atoi( itemp.substr( 14, 3 ).c_str() );
    else          ms = 0;
// calculate MJD
    slaCldj( y, m, d, &gMJD, &l );
    MJD = (int)gMJD;
    Time = h*60.*60.+min*60.+s + ms/1.e3;
}


bool VExposure::getDBSourceCoordinates( TSQLServer *f_db, string iSource, double &iEVNTargetDec, double &iEVNTargetRA )
{
    if( !f_db ) return false;

    if( !fVDB_ObservingSources )
    {
       fVDB_ObservingSources = new VDB_ObservingSources();
       if( !fVDB_ObservingSources->fill( f_db ) ) return false;
    }

    VDB_ObservingSources_Data *iTemp = fVDB_ObservingSources->get_ObservingSources_Data( iSource );
    if( iTemp )
    {
       iEVNTargetDec = iTemp->fDec;
       iEVNTargetRA  = iTemp->fRA;
       return true;
    }

    iEVNTargetDec = 0.;
    iEVNTargetRA = 0.;
    return false;
}


double VExposure::getAcceptance( double r )
{
    if( r < 0. || r > fAcceptance_MaxDistance ) return 0.;

    double iacc = 1.;
    if( fAcceptance )
    {
        iacc = fAcceptance->Eval( r );
    }
    if( iacc < 0. ) iacc = 0.;

    return iacc;
}


bool VExposure::readAcceptanceCurveFromFile( string iFile, double iAcceptance_MaxDistance )
{
    if( iFile.size() <= 0 ) return false;

    cout << "reading acceptance curves from " << iFile << endl;
    TFile *fAcceptanceFile = new TFile( iFile.c_str() );
    if( fAcceptanceFile->IsZombie() ) return false;

    fAcceptance = (TF1*)fAcceptanceFile->Get( "fAccZe_0" );

    if( !fAcceptance )
    {
        cout << "Warning: acceptance curve not found in file: " << iFile << endl;
        return false;
    }

    fAcceptance_MaxDistance = iAcceptance_MaxDistance;

    return true;
}

/*
    apply DQM to a single run


    (still some hardwired numbers)
*/
bool VExposure::doDQM( unsigned int iID, double iMinDuration )
{
//   if( iID >= (unsigned int)fRun.size() ) return false;

// check run duration
   if( fRunDuration[iID] < iMinDuration ) return false;

// HARDWIRED: require at least three telescopes
   if( fRunConfigMask[iID] == 0 )  return false;
   if( fRunConfigMask[iID] == 1 )  return false;
   if( fRunConfigMask[iID] == 2 )  return false;
   if( fRunConfigMask[iID] == 3 )  return false;
   if( fRunConfigMask[iID] == 4 )  return false;
   if( fRunConfigMask[iID] == 5 )  return false;
   if( fRunConfigMask[iID] == 6 )  return false;
   if( fRunConfigMask[iID] == 8 )  return false;
   if( fRunConfigMask[iID] == 9 )  return false;
   if( fRunConfigMask[iID] == 10 )  return false;
   if( fRunConfigMask[iID] == 12 )  return false;

   return true;
}


void VExposure::printListOfRuns( string iCatalogue, double iR, double iMinDuration, string iTeVCatalogue, double r_min, string iEventListFile )
{
    VStarCatalogue *s = new VStarCatalogue();
    s->init( 54626., iCatalogue );

    VStarCatalogue *tev = new VStarCatalogue();
    if( iTeVCatalogue.size() > 0 ) tev->init( 54626., iTeVCatalogue );

// file for event list
    TFile *fEventListfile = 0;
    TEventList *fEventList_tevcat = 0;
    TEventList *fEventList_inFOV = 0;
    TEventList *fEventList_inFOV_noTevcat = 0;
    if( iEventListFile.size() > 0 )
    {
       fEventListfile = new TFile( iEventListFile.c_str(), "RECREATE" );
       if( fEventListfile->IsZombie() ) return;
       fEventList_tevcat = new TEventList( "tevcatList" );
       fEventList_inFOV = new TEventList( "veritas" );
       fEventList_inFOV_noTevcat = new TEventList( "V_noPointing" );
    }


    char iTexTable[2000];
    set< string > iVERITAS_targets;

    double r_centre = 0.;
    double r_VA_object = 0.;

////////////////////////////////////////////////////////
// loop over all objects in catalogue
    cout << "total number of objects in catalogue: " << s->getNStar() << endl;
    for( unsigned int i = 0; i < s->getNStar(); i++ )
    {
	double r_tot = 0.;
	double r_tot_V5 = 0.;

	double r_centre_mean = 0.;
	double r_VA_object_mean = 0.;
	double r_N = 0.;
	double r_meanElevation = 0.;

	cout << "Testing now ";
	cout << s->getStarName( i );
	cout << " (l,b) = " << s->getStar_l(i) << ", " << s->getStar_b(i);
	cout << " (ra,dec) = " << s->getStarRA2000( i ) << ", " << s->getStarDec2000( i );
	cout << endl;


////////////////////////////////////////////////////////
// check if this catalogue object is in second catalogue
	int tev_select = -1;

// loop over second catalogue
        for( unsigned int j = 0; j < tev->getNStar(); j++ )
	{
	   double r = slaDsep(s->getStarRA2000(i)*TMath::Pi()/180.,s->getStarDec2000(i)*TMath::Pi()/180.,tev->getStarRA2000(j)*TMath::Pi()/180.,tev->getStarDec2000(j)*TMath::Pi()/180. ) * 180./TMath::Pi();
	   if( r < r_min ) 
	   {
	      tev_select = (int)j;
	      cout << "tevCAT: " << tev->getStarName( j ) << endl;
	      if( fEventList_tevcat ) fEventList_tevcat->Enter( i );
	      break;
           }
        }
	if( tev_select > 0 )
	{
	   cout << "\t OBJECT " << s->getStarName( i ) << " is nearby " << tev->getStarName( tev_select ) << endl;
	   cout << endl;
        }

////////////////////////////////////////////////////////////////
// now check if object is in FOV

// loop over all runs 
	r_tot = 0.;
	r_tot_V5 = 0.;
	iVERITAS_targets.clear();
	for( unsigned int j = 0; j < fRunRA.size(); j++ )
	{
// calculate distance of catalogue object to camera center
            r_centre = slaDsep(s->getStarRA2000(i)*TMath::Pi()/180.,s->getStarDec2000(i)*TMath::Pi()/180.,
	               (fRunRA[j]+fRunoffsetRA[j])*TMath::Pi()/180.,(fRunDec[j]+fRunoffsetDec[j])*TMath::Pi()/180. ) * 180./TMath::Pi();
// do dqm
	    if( !doDQM(j, iMinDuration ) ) continue;

// check if the catalogue object is close enough
            if( r_centre < iR )
	    {
// calculate distance of catalogue object to VERITAS object
	       r_VA_object = slaDsep(s->getStarRA2000(i)*TMath::Pi()/180.,s->getStarDec2000(i)*TMath::Pi()/180.,
	                             fRunRA[j]*TMath::Pi()/180.,fRunDec[j]*TMath::Pi()/180. ) * 180./TMath::Pi();
// total time on object (all array configurations)
	       r_tot += fRunDuration[j];
// total time on object (new array configuration only)
	       if( fRun[j] > 46642 )  r_tot_V5 += fRunDuration[j];
// print some information about this run
	       cout << "\tRUN " << fRun[j];
	       if( fRunSourceID[j].size() > 0 ) cout << "\t" << fRunSourceID[j];
	       cout << "\t MJD " << fRunStartMJD[j];
	       cout << "\t CONFIGMASK " << fRunConfigMask[j];
	       cout << "\t (ra,dec)=(" << fRunRA[j] << "," << fRunDec[j] << ")";
	       cout << "\t DIST " << r_centre << " deg";
	       cout << "\t DIST (VTS pointing) " << r_VA_object << " deg";
	       cout << endl;
// mean calculation
	       r_centre_mean += r_centre;
	       r_VA_object_mean += r_VA_object;
	       r_meanElevation += fRunTelElevation[j];
	       r_N++;
	       iVERITAS_targets.insert( fRunSourceID[j] );
             }
         }
// print summary only if some data was taken 
         if( r_tot > 0. )
	 {
	     if( fEventList_inFOV ) fEventList_inFOV->Enter( i );
	     cout << "\t\t total time: " << r_tot/3600. << " [h] (V5: " << r_tot_V5/3600. << ")" << endl;
	     if( r_N > 0. )
	     {
	        cout << "\t\t mean distance to camera centre: " << r_centre_mean/r_N << " [deg]" << endl;
		cout << "\t\t mean distance to VERITAS object: " << r_VA_object_mean/r_N << " [deg]" << endl;
		cout << "\t\t mean elevation: " << r_meanElevation/r_N << " [deg]" << endl;
		if( fEventList_inFOV && r_VA_object_mean/r_N > 0.1 ) fEventList_inFOV_noTevcat->Enter( i );
             }
///////////////////////////////////
// add a line to the tex table
///////////////////////////////////
// green: known TeV source
             if( tev_select >= 0 && tev->getStarName( tev_select ) != "NONAME" )  sprintf( iTexTable, "{\\color{green} %s}", s->getStarName( i ).c_str() );
	     else if( r_tot > 3600.*3. &&  r_VA_object_mean/r_N > 0.1 ) sprintf( iTexTable, "{\\color{red} %s}", s->getStarName( i ).c_str() );
	     else sprintf( iTexTable, "%s ", s->getStarName( i ).c_str() );
	     sprintf( iTexTable, "%s & (%.1f,%.1f) & ", iTexTable, s->getStar_l(i), s->getStar_b(i) );
	     if( s->getStarFlux(i).size() > 0 )
	     {
	        sprintf( iTexTable, "%s %.1f/%.1f & ", iTexTable, s->getStarFlux(i)[s->getStarFlux(i).size()-1]/1.e-10, s->getStarSpectralIndex(i) );
		fTexTable_EFlux_min = s->getStarFluxEnergyMin(i)[s->getStarFlux(i).size()-1];
		fTexTable_EFlux_max = s->getStarFluxEnergyMax(i)[s->getStarFlux(i).size()-1];
             }
	     else                               sprintf( iTexTable, "%s & -", iTexTable );
	     set<string>::iterator it_set; 
	     int z = 0;
	     for( it_set = iVERITAS_targets.begin(); it_set != iVERITAS_targets.end(); it_set++ )
	     {
	         string a = VUtilities::search_and_replace( *it_set, "_", " " );
		 if( iVERITAS_targets.size() == 1 ) sprintf( iTexTable, "%s %s  ", iTexTable, a.c_str() );
	         else if( z < (int)(iVERITAS_targets.size()) - 1 ) sprintf( iTexTable, "%s %s, ", iTexTable, a.c_str() );
//		 else                                  sprintf( iTexTable, "%s %s ", iTexTable, a.c_str() );                                
                 else                                  sprintf( iTexTable, "%s ...", iTexTable );
		 if( z > 0 ) break;
		 z++;
             } 
	     sprintf( iTexTable, "%s & %.1f", iTexTable, r_VA_object_mean/r_N );
	     sprintf( iTexTable, "%s & %.1f", iTexTable, r_centre_mean/r_N );
	     sprintf( iTexTable, "%s & %d", iTexTable, (int)(r_meanElevation/r_N ) );
	     sprintf( iTexTable, "%s & %.1f (%.1f)", iTexTable, r_tot/3600., r_tot_V5/3600. );
	     if( tev_select >= 0 && tev->getStarName( tev_select ) != "NONAME" )
	     {
		 string a = VUtilities::search_and_replace( tev->getStarName( tev_select ), "_", " " );
	         sprintf( iTexTable, "%s & {\\color{green} %s}", iTexTable, a.c_str() );
             }
	     else
	     {
		 if( s->getStarType(i).size() > 1 ) sprintf( iTexTable, "%s & {\\textit %s}", iTexTable, s->getStarType(i).c_str() );
		 else                               sprintf( iTexTable, "%s &", iTexTable );
		 if( s->getStarAssociations(i).size() > 0 )
		 {
		    sprintf( iTexTable, "%s, ", iTexTable );
		    z = 0;
		    sprintf( iTexTable, "%s", iTexTable );
		    for( unsigned int o = 0; o < s->getStarAssociations(i).size(); o++ ) 
		    {
		       if( s->getStarAssociations(i)[o].size() > 3 )
		       {
			  if( z < (int)s->getStarAssociations(i).size()-1 ) sprintf( iTexTable, "%s %s,", iTexTable, s->getStarAssociations(i)[o].c_str() );
			  else                                            sprintf( iTexTable, "%s %s", iTexTable, s->getStarAssociations(i)[o].c_str() );

			  if( z > 2 ) break;
			  z++;
		       }
		    } 
                 }
             }
//	     else sprintf( iTexTable, "%s & ", iTexTable );
	     sprintf( iTexTable, "%s \\\\", iTexTable );
	     string iTe = iTexTable;
	     fTexTable.push_back( iTe );
         }
    }
    if( fEventList_tevcat ) fEventList_tevcat->Write();
    if( fEventList_inFOV ) fEventList_inFOV->Write();
    if( fEventList_inFOV_noTevcat ) fEventList_inFOV_noTevcat->Write();
    if( fEventListfile ) fEventListfile->Close();
}

void VExposure::printTexTable()
{
   if( fTexTable.size() == 0 ) 
   {
      cout << "Error: no entries in tex table" << endl;
      return;
   }
// print header
   cout << "\\documentclass{article}" << endl;
   cout << "\\usepackage[pdftex]{graphicx,color}" << endl;
   cout << "\\usepackage{longtable}" << endl;
   cout << "\\usepackage{lscape}" << endl;
   cout << "\\usepackage{a4wide}" << endl;
   cout << "\\usepackage{nopageno}" << endl;
   cout << "\\usepackage[pdftex,colorlinks=true,bookmarks=true,bookmarksopen=true]{hyperref}" << endl;
   cout << "\\pagestyle{plain}" << endl;
   cout << "\\renewcommand{\\baselinestretch}{1.5}" << endl;
   cout << "\\begin{document}" << endl;

// print table
   cout << "\\begin{footnotesize}" << endl;
   cout << "\\begin{landscape}" << endl;
   cout << "\\begin{longtable}{l|c|c|c|c|c|c|c|c}" << endl;
   cout << "Name & (l,b) & Flux/index                                                                  & VERITAS & distance to    & average             & mean      & total time                & remarks \\\\" << endl;
   cout << "     &       & (" << (int)fTexTable_EFlux_min << "," << (int)fTexTable_EFlux_max << ") GeV & target  & VERITAS        & distance to         & elevation & (new array                & \\\\" << endl;
   cout << "     &       &  $10^{-10}$                                                                        &         & target         & camera centre       &           & configuration)            &       \\\\" << endl;
   cout << "     &       &          [ph/cm$^{-2}$ s$^{-1}$]                                                              &           &    [deg]       &   [deg]             &  [deg]    &   [h]                     &       \\\\" << endl;
   cout << "\\endhead" << endl;
   cout << "\\hline" << endl;
   for( unsigned int i = 0; i < fTexTable.size(); i++ )
   {
      cout << fTexTable[i] << endl;
      cout << "\\hline" << endl;
   }
   cout << "\\end{longtable}" << endl;
   cout << "\\end{landscape}" << endl;
   cout << "\\end{footnotesize}" << endl;

   cout << "\\end{document}" << endl;



}


void VExposure::printListOfRuns( double il, double ib, double iR, double iMinDuration, string iDQMfileList, string ofile, unsigned int iVerbose )
{
    double r_dist = 0.;
    double r_tot = 0.;

// read list with runs which passed dqm
    vector< int > i_vDQMList;
    if( iDQMfileList.size() > 0 )
    {
        ifstream is;
        is.open( iDQMfileList.c_str(), ifstream::in);
        if( !is )
        {
            cout << "DQM file list not found: " << iDQMfileList << endl;
            return;
        }
        string is_line;
        while( getline( is, is_line ) )
        {
            if( atoi( is_line.c_str() ) > 0 ) i_vDQMList.push_back( atoi( is_line.c_str() ) );
        }
        is.close();
    }
    ofstream os;
    if( ofile.size() > 0 )
    {
        os.open( ofile.c_str() );
        if( !os )
        {
            cout << "Error writing list of runs to " << ofile << endl;
            return;
        }
    }

    if( iVerbose == 1 ) cout << "Number of runs:  " << fRunGalLong1958.size() << endl;
    else if( iVerbose == 2 && fRunGalLong1958.size() > 0 ) cout << "Number of runs: " << fRunGalLong1958.size() << endl;
    for( unsigned int i = 0; i < fRunGalLong1958.size(); i++ )
    {
        r_dist = slaDsep( il*TMath::Pi()/180., ib*TMath::Pi()/180., fRunGalLong1958[i]*TMath::Pi()/180., fRunGalLat1958[i]*TMath::Pi()/180. ) * 180./TMath::Pi();

        if( r_dist < iR && fRunDuration[i] > iMinDuration )
        {
// check if this file passed the dqm
            bool bPassed = false;
            for( unsigned int q = 0; q < i_vDQMList.size(); q++ )
            {
                if( fRun[i] == i_vDQMList[q] )
                {
                    bPassed = true;
                    break;
                }
            }
            if( i_vDQMList.size() == 0 ) bPassed = true;
            if( !bPassed )
            {
                if( iVerbose > 0 && i_vDQMList.size() > 0 ) cout << fRun[i] << " FAILED DQM" << endl;
                continue;
            }

            r_tot += fRunDuration[i];
            cout << "\t\tRUN " << fRun[i];
            if( fRunSourceID[i].size() > 0 ) cout << "\t" << fRunSourceID[i];
            cout << "\t MJD " << fRunStartMJD[i];
            cout << "\t CONFIGMASK " << fRunConfigMask[i];
            cout << "\t DIST " << r_dist << " deg";
            cout << endl;
            if( os )
            {
// do not write T1T2 runs to disk
                if( fRunConfigMask[i] != 3 ) os << fRun[i] << endl;
            }
        }
    }
    if( iVerbose > 0 ) cout << "\t\t total time: " << r_tot/3600. << " [h]" << endl;
    if( os ) os.close();

}


/*
   copied from ROOT forum #6650
*/
void VExposure::aitoff2xy(Double_t l, Double_t b, Double_t &Al, Double_t &Ab)
{
    Double_t x, y;

    Double_t alpha2 = (l/2)*TMath::DegToRad();
    Double_t delta  = b*TMath::DegToRad();
    Double_t r2     = TMath::Sqrt(2.);
    Double_t f      = 2*r2/TMath::Pi();
    Double_t cdec   = TMath::Cos(delta);
    Double_t denom  = TMath::Sqrt(1. + cdec*TMath::Cos(alpha2));
    x      = cdec*TMath::Sin(alpha2)*2.*r2/denom;
    y      = TMath::Sin(delta)*r2/denom;
    x     *= TMath::RadToDeg()/f;
    y     *= TMath::RadToDeg()/f;
    x *= -1.;                                     // for a skymap swap left<->right
    Al = x;
    Ab = y;
}


void VExposure::plotMarker( double l, double b, double r, string iText, int iMarkerStyle, int iMarkerColor, int iMarkerSize )
{
    if( fPlottingCanvas.size() == 0 ) return;

    for( unsigned int i = 0; i < fPlottingCanvas.size(); i++ )
    {
        if( !fPlottingCanvas[i] ) continue;

        fPlottingCanvas[i]->cd();

        double ab = b;
        double al = -1.*l;
        string i_cname = fPlottingCanvas[i]->GetName();
        if( i_cname.find( "aitoff" ) < i_cname.size() ) aitoff2xy( l, b, al, ab );
// extended?
        if( r > 0. )
        {
            TEllipse *m = new TEllipse( al, ab, r, r );
            m->SetFillStyle( 0 );
            m->Draw();
        }
        else
        {
            TMarker *m = new TMarker( al, ab, iMarkerStyle );
            m->SetMarkerColor( iMarkerColor );
            m->SetMarkerSize( iMarkerSize );
            m->Draw();
        }

        if( iText.size() > 0 )
        {
            al = -1.*l+10.*0.01;
            ab = b+10.*0.01;
            TText *t = new TText( al, ab, iText.c_str() );
            t->SetTextSize( t->GetTextSize()*0.3 );
            t->SetTextColor( iMarkerColor );
            t->SetTextAngle( 45. );
            t->Draw();
        }
        fPlottingCanvas[i]->Update();
    }
}


void VExposure::addCatalogue( string iC, int iMarker, int iColor, double iAngle  )
{
    fCatalogue.push_back( iC );
    fCatalogueMarkerColor.push_back( iColor );
    fCatalogueMarkerStyle.push_back( iMarker );
    fCatalogueTextAngle.push_back( iAngle );
}


bool VExposure::removeCataloge( unsigned int iB )
{
    if( iB < fCatalogue.size() )
    {
        fCatalogue.erase( fCatalogue.begin() + iB );
        fCatalogueMarkerColor.erase( fCatalogueMarkerColor.begin() + iB );
        fCatalogueMarkerStyle.erase( fCatalogueMarkerStyle.begin() + iB );
        fCatalogueTextAngle.erase( fCatalogueTextAngle.begin() + iB );
        return true;
    }
    return false;
}


void VExposure::listCatalogues()
{
    cout << "list of catalogues: " << endl;
    for( unsigned int i = 0; i < fCatalogue.size(); i++ )
    {
        cout << i << "\t" << fCatalogue[i] << "\t" << fCatalogueMarkerColor[i] << "\t";
        cout << fCatalogueMarkerStyle[i] << "\t" << fCatalogueTextAngle[i] <<
            cout << endl;
    }
}


void VExposure::plotTimeDifferencesbetweenRuns()
{
    fTimeDifferencesBetweenRuns = new TH1D( "hTimeDifferenceBetweenRuns", "", 1000, 0., 100. );
    fTimeDifferencesBetweenRuns->SetXTitle( "time difference [min]" );
    fTimeDifferencesBetweenRuns->SetYTitle( "# of runs" );
    fTimeDifferencesBetweenRuns->SetStats( 0 );

    if( fRunStartMJD.size() < 2 ) return;

    double iDiff = 0.;
    for( unsigned int i = 0; i < fRunStartMJD.size() - 1; i++ )
    {
        iDiff  = fRunStartMJD[i+1] - fRunStopMJD[i];
        iDiff *= 24. * 60.;

        fTimeDifferencesBetweenRuns->Fill( iDiff );
    }

    TCanvas *cTimeDiff = new TCanvas( "cTimeDiff", "time difference between runs", 10, 10, 400, 400 );
    cTimeDiff->SetGridx( 0 );
    cTimeDiff->SetGridy( 0 );
    if( fTimeDifferencesBetweenRuns->GetEntries() > 0. ) cTimeDiff->SetLogy( 1 );

    fTimeDifferencesBetweenRuns->SetAxisRange( 0., 10. );
    fTimeDifferencesBetweenRuns->Draw();
}
