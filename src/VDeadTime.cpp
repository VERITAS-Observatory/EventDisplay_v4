/*! \class VDeadTime
    \brief dead time calculator


        if( fReader->getArrayTrigger() && fReader->getArrayTrigger()->hasTenMHzClockArray() )
	{
	   cout << "DEAD TIME 0 " << fReader->getArrayTrigger()->getTenMHzClockArray()[0] << endl;
	   cout << "DEAD TIME 1 " << fReader->getArrayTrigger()->getTenMHzClockArray()[1] << endl;
        }

    \author  Gernot Maier

    Revision $Id: VDeadTime.cpp,v 1.4.2.1.22.2.14.1 2010/03/08 07:45:08 gmaier Exp $
*/

#include "VDeadTime.h"

VDeadTime::VDeadTime( bool iIsOn )
{
    bIsOn = iIsOn;

    fDeadTimeMiss = 0.;
    fDeadTimeMS = 0.;
    fDeadTimeFrac = 0.;

    fTFitMin = 0.004;
    fTFitMax = 0.015;

    hTimeDiff = 0;
    hTimeDiff2D = 0;
    hTimeDiffLog = 0;
    hFTimeDiff = 0;
    hgDeadTime = 0;
    hNEventTime = 0;
    hisList = 0;
    reset();
}


void VDeadTime::reset()
{
    ft0 = 0.;
    fRunStart = 0.;

    if( hTimeDiff ) hTimeDiff->Reset();
    if( hTimeDiffLog ) hTimeDiffLog->Reset();
    if( hTimeDiff2D ) hTimeDiff2D->Reset();
}


void VDeadTime::defineHistograms()
{
    hisList = new TList();

    char hname[200];

    if( bIsOn ) sprintf( hname, "hTimeDiff_on" );
    else        sprintf( hname, "hTimeDiff_off" );
    hTimeDiff = new TH1D( hname, "time difference between events (lin)", 50000, 0., 0.2 );
    hTimeDiff->SetXTitle( "time difference [s]" );
    hTimeDiff->SetYTitle( "number of entries" );
    hTimeDiff->SetFillColor( 8 );
    hisList->Add( hTimeDiff );

    if( bIsOn ) sprintf( hname, "hTimeDiffLog_on" );
    else        sprintf( hname, "hTimeDiffLog_off" );
    hTimeDiffLog = new TH1D( hname, "time difference between events (log)", 500, -4., -0.5 );
    hTimeDiffLog->SetXTitle( "log_{10} time difference [s]" );
    hTimeDiffLog->SetYTitle( "number of entries" );
    hTimeDiffLog->SetLineWidth( 2 );
    hTimeDiffLog->SetFillColor( 8 );
    hisList->Add( hTimeDiffLog );

    if( bIsOn ) sprintf( hname, "hTimeDiff2D_on" );
    else        sprintf( hname, "hTimeDiff2D_off" );
    hTimeDiff2D = new TH2D( hname, "time difference between events vs run time", 20, 0., 40., 4000, 0., 0.05 );
    hTimeDiff2D->SetXTitle( "event time [min]" );
    hTimeDiff2D->SetYTitle( "time difference [s]" );
    hTimeDiff2D->SetZTitle( "number of entries" );
    hTimeDiff2D->SetFillColor( 8 );
    hisList->Add( hTimeDiff2D );

    if( bIsOn ) sprintf( hname, "hgDeadTime_on" );
    else        sprintf( hname, "hgDeadTime_off" );
    hgDeadTime = new TGraphErrors( hTimeDiff2D->GetNbinsX() );
    hgDeadTime->SetName( hname );
    hgDeadTime->SetTitle( "" );
    hgDeadTime->SetMarkerStyle( 20 );
    hgDeadTime->SetMarkerSize( 2 );
    hisList->Add( hgDeadTime );

    if( bIsOn ) sprintf( hname, "hNEventTime_on" );
    else        sprintf( hname, "hNEventTime_off" );
    hNEventTime = new TH1D( hname, "rate", 80, 0., 40. );
    hNEventTime->SetXTitle( "event time [min]" );
    hNEventTime->SetYTitle( "number of entries" );
    hNEventTime->SetFillColor( 8 );
    hisList->Add( hNEventTime );

    if( bIsOn ) sprintf( hname, "hTimeDiffLog_on" );

    if( bIsOn ) sprintf( hname, "hFTimeDiff_on" );
    else        sprintf( hname, "hFTimeDiff_off" );
//    hFTimeDiff = new TF1( hname, "expo", fTFitMin, fTFitMax );
    hFTimeDiff = new TF1( hname, "expo", 1.e-4, 0.2 );
    hFTimeDiff->SetLineColor( 2 );
    hisList->Add( hFTimeDiff );

}


double VDeadTime::fillDeadTime( double time )
{
    double tdiff = time - ft0;

    if( tdiff < 100. )
    {
        if( hTimeDiff ) hTimeDiff->Fill( tdiff );
        if( hTimeDiffLog && tdiff > 0. ) hTimeDiffLog->Fill( log10( tdiff ) );
        if( fRunStart > 0. )
        {
            hTimeDiff2D->Fill( ( time - fRunStart ) / 60., tdiff );
            hNEventTime->Fill( ( time - fRunStart ) / 60. );
        }
    }
    ft0 = time;

    if( fRunStart == 0. ) fRunStart = time;

    return tdiff;
}


double VDeadTime::calculateDeadTime()
{
    if( !hTimeDiff || !hFTimeDiff ) return 0.;
    if( hTimeDiff->GetEntries() <= 0. ) return 0.;

// assume exponential distributions of dead times
    TF1 fFit( "fFit", "expo", fTFitMin, fTFitMax );
    hTimeDiff->Fit( &fFit, "Q0R" );
    hFTimeDiff->SetParameter( 0, fFit.GetParameter( 0 ) );
    hFTimeDiff->SetParameter( 1, fFit.GetParameter( 1 ) );

    double ix = hTimeDiff->GetBinCenter( 1 );
    double nmiss = 0.;

// this method doesn't work with spikes at small delta t
/*
    while( (hFTimeDiff->Eval( ix ) - hTimeDiff->GetBinContent( jbin ))/hFTimeDiff->Eval( ix ) > 0.5 )
    {
        nmiss += hFTimeDiff->Eval( ix ) - hTimeDiff->GetBinContent( jbin );
        jbin++;
        ix = hTimeDiff->GetBinCenter( jbin );
    } */
// go left in delta t histogram from 0.01, first 0 bin defines dead time
// (fails for very short runs)
//    for( int i = hTimeDiff->FindBin( 0.01 ); i > 2; i-- )
    for( int i = hTimeDiff->FindBin( hTimeDiff->GetMean() ); i > 2; i-- )
    {
// require  at least three zero bins
        if( hTimeDiff->GetBinContent( i ) == 0 && hTimeDiff->GetBinContent( i - 1 ) == 0 && hTimeDiff->GetBinContent( i - 2 ) == 0 )
        {
            ix = hTimeDiff->GetBinCenter( i );
            break;
        }
    }

    fDeadTimeMiss = nmiss;
    fDeadTimeMS = ix * 1000.;
    fDeadTimeFrac = 1. - TMath::Power( TMath::E(), ix * hFTimeDiff->GetParameter( 1 ) );
    if( fDeadTimeFrac < 1.e-5 ) fDeadTimeFrac = 0.;

// get dead time dependent on time diff

    int i_np = 0;
    for( int i = 1; i <= hTimeDiff2D->GetNbinsX(); i++ )
    {
        TH1D *h = hTimeDiff2D->ProjectionY( "h_tempXDGS", i, i, "e" );
        if( h && h->GetEntries() > 1000 )
        {
            h->Fit( &fFit, "Q0R" );
            ix = h->GetBinCenter( 1 );
            for( int b = h->FindBin( 0.005 ); b > 0; b-- )
            {
                if( h->GetBinContent( b ) == 0 )
                {
                    ix = h->GetBinCenter( b );
                    if( h->GetBinContent( b ) < 5 ) ix = h->GetBinCenter( b+1 );
                    break;
                }
            }
            hgDeadTime->SetPoint( i-1, hTimeDiff2D->GetXaxis()->GetBinCenter( i ),  (1. - TMath::Power( TMath::E(), ix * fFit.GetParameter( 1 ) ) ) * 100. );
            double iDE = 0.;
            iDE += ix*TMath::Power( TMath::E(), ix * fFit.GetParameter( 1 ) ) * fFit.GetParError( 1 ) * ix*TMath::Power( TMath::E(), ix * fFit.GetParameter( 1 ) ) * fFit.GetParError( 1 );
            iDE += fFit.GetParameter( 1 ) * TMath::Power( TMath::E(), ix * fFit.GetParameter( 1 ) )*h->GetBinWidth( 1 ) * fFit.GetParameter( 1 ) * TMath::Power( TMath::E(), ix * fFit.GetParameter( 1 ) )*h->GetBinWidth( 1 ) / 4.;
            iDE = sqrt( iDE );
            hgDeadTime->SetPointError( i-1, 0., iDE * 100. );

            i_np++;

            delete h;
        }
    }
    hgDeadTime->Set( i_np );

    return fDeadTimeMS;
}


void VDeadTime::printDeadTime()
{
    cout << "\t dead time [ms] " << fDeadTimeMS << ", fraction of missing events: " << fDeadTimeFrac * 100.;
    cout << "% (" << fDeadTimeMiss << ", ";
    if( hTimeDiff ) cout << hTimeDiff->GetEntries() << ")" << endl;
}


TList* VDeadTime::getDeadTimeHistograms()
{
    if( hisList ) return hisList;

    return 0;
}


void VDeadTime::writeHistograms()
{
    TDirectory *iDir = gDirectory;

    TDirectory *wDir = 0;

    iDir->cd();
    wDir = (TDirectory*)iDir->Get( "deadTimeHistograms" );
    if( !wDir ) iDir->mkdir( "deadTimeHistograms" )->cd();
    else        wDir->cd();

    if( hisList ) hisList->Write();

// remove all objects created with new in this class
    hisList->Delete();
    delete hisList;
    hisList = 0;

    iDir->cd();
}


bool VDeadTime::readHistograms( TDirectoryFile *iDir )
{
    if( !iDir ) return false;

    if( hisList )
    {
        hisList->Delete();
    }
    else
    {
        hisList = new TList();
    }

    cout << "\t reading dead time histograms from file " << iDir->GetPath() << endl;

    hTimeDiff = (TH1D*)iDir->Get( "hTimeDiff_on" );
    if( !bIsOn ) hTimeDiff->SetName( "hTimeDiff_off" );
    hisList->Add( hTimeDiff );
    hTimeDiffLog = (TH1D*)iDir->Get( "hTimeDiffLog_on" );
    if( !bIsOn ) hTimeDiffLog->SetName( "hTimeDiffLog_off" );
    hisList->Add( hTimeDiffLog );
    hTimeDiff2D = (TH2D*)iDir->Get( "hTimeDiff2D_on" );
    if( !bIsOn ) hTimeDiff2D->SetName( "hTimeDiff2D_off" );
    hisList->Add( hTimeDiff2D );
    hgDeadTime = (TGraphErrors*)iDir->Get( "hgDeadTime_on" );
    if( !bIsOn ) hgDeadTime->SetName( "hgDeadTime_off" );
    hisList->Add( hgDeadTime );
    hNEventTime = (TH1D*)iDir->Get( "hNEventTime_on" );
    if( !bIsOn ) hNEventTime->SetName( "hNEventTime_off" );
    hisList->Add( hNEventTime );
    hFTimeDiff = (TF1*)iDir->Get( "hFTimeDiff_on" );
    if( !bIsOn ) hFTimeDiff->SetName( "hFTimeDiff_off" );
    hisList->Add( hFTimeDiff );

    return true;
}
