/*! \file   plot_CTA
    \brief  CTA related plotting macros

*/


/*
     plot array layout

     INPUT:

     ifile        eventdisplay or mscw root file
     iname        name of array (e.g. J)
     iMarkerMult  increase marker size by this factor
     xmax/ymax    maximum extension of array to plot (in [m])

     OUTPUT:

     plots

*/
void plot_array( char *ifile, char *iname = 0, double iMarkerMult = 1., double xmax = 1450., double ymax = 1450. )
{
    TFile *f1 = new TFile( ifile );
    if( f1->IsZombie() ) return;

    TTree *t = (TTree*)f1->Get( "telconfig");
    if( !t ) return;

    TCanvas *c = new TCanvas( "c2", "array layout", 10, 10, 600, 600 );
    c->SetGridx( 0 );
    c->SetGridy( 0 );
    c->SetRightMargin( 0.05 );
    c->SetLeftMargin( 0.13 );
    c->Draw();

    TH2D *hnull = new TH2D( "hnull", "", 100, -1.*xmax, xmax, 100, -1.*ymax, ymax );
    hnull->SetStats( 0 );
    hnull->SetXTitle( "x [m]" );
    hnull->SetYTitle( "y [m]" );
    hnull->GetYaxis()->SetTitleOffset( 1.6 );

    hnull->Draw();

    telconfig->SetMarkerStyle( 20 );
    telconfig->SetMarkerColor( 2 );
    telconfig->SetMarkerSize( 1.5 * iMarkerMult );
    telconfig->Draw("TelY:TelX", "TelType/100000 > 400 ", "same" );
    telconfig->Draw("TelY:TelX", "TelType==141304909", "same" );

    telconfig->SetMarkerStyle( 20 );
    telconfig->SetMarkerColor( 1 );
    telconfig->SetMarkerSize( 1.0 * iMarkerMult );
    telconfig->Draw("TelY:TelX", "TMath::Abs(TelType/100000-100)<5", "same" );

    telconfig->SetMarkerStyle( 20 );
    telconfig->SetMarkerColor( 3 );
    telconfig->SetMarkerSize( 0.7 * iMarkerMult );
    telconfig->Draw("TelY:TelX", "TelType/100000<40", "same" );

    if( iname )
    {
       TText *it = new TText( 0.35*xmax, 0.8*ymax, iname );
       it->Draw();
    }
}
     
