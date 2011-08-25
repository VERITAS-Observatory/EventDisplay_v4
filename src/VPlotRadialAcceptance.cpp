/*  \class VPlotRadialAcceptance
    \brief plot radial acceptance curves

*/

#include "VPlotRadialAcceptance.h"

VPlotRadialAcceptance::VPlotRadialAcceptance( string iFile )
{
   fDebug = false;

   setName( "radial acceptance" );
   
   if( iFile.size() > 0 ) readAcceptanceFile( iFile, 0 );
}


bool VPlotRadialAcceptance::readAcceptanceFile( string iFile, unsigned int iZeBin )
{
// open acceptance file
   fAcceptanceFile = new TFile( iFile.c_str() );
   if( fAcceptanceFile->IsZombie() )
   {
       cout << "VPlotRadialAcceptance::addAcceptanceFile: error adding acceptance file" << endl;
       cout << iFile << endl;
       return false;
   }
   char hname[200];
// read acceptance histogram from file
   sprintf( hname, "hAccZe_%d", iZeBin );
   fAcceptanceHisto = (TH1D*)fAcceptanceFile->Get( hname );
   if( !fAcceptanceHisto )
   {
       cout << "VPlotRadialAcceptance::addAcceptanceFile: error finding acceptance histogram" << endl;
       cout << hname << endl;
       return false;
   }
// read acceptance fit function from file
   sprintf( hname, "fAccZe_%d", iZeBin );
   fAcceptanceFunction = (TF1*)fAcceptanceFile->Get( hname );
   if( !fAcceptanceFunction )
   {
       cout << "VPlotRadialAcceptance::addAcceptanceFile: error finding acceptance fit function" << endl;
       cout << hname << endl;
       return false;
   }
// read acceptance histogram (fit values) from file
   sprintf( hname, "hAccZeFit_%d", iZeBin );
   fAcceptanceHistoFit = (TH1D*)fAcceptanceFile->Get( hname );
   if( !fAcceptanceHistoFit )
   {
       cout << "VPlotRadialAcceptance::addAcceptanceFile: error finding acceptance histogram (fitted)" << endl;
       cout << hname << endl;
       return false;
   }

   return true;
}

/*

    plot all acceptance curves

*/
TCanvas* VPlotRadialAcceptance::plot( TCanvas *cX )
{
    if( !fAcceptanceFile || fAcceptanceFile->IsZombie() )
    {
       cout << "VPlotRadialAcceptance::plot() error: data missing";
       return 0;
    }

// canvas
    if( cX ) cX->cd();
    else
    {
       char hname[2000];
       char htitle[2000];
       sprintf( hname, "cAcceptance_%s", VUtilities::removeSpaces( fName ).c_str() );
       sprintf( htitle, "%s", fName.c_str() );
       cX = new TCanvas( hname, htitle, 10, 10, 400, 400 );
       cX->SetGridx( 0 );
       cX->SetGridy( 0 );
    }

// plot all histograms and plot them

    if( fAcceptanceHisto )
    {
       fAcceptanceHisto->SetMinimum( 0.0 );
       fAcceptanceHisto->SetMaximum( 1.5 );
       fAcceptanceHisto->SetAxisRange( 0., 3.5 );
       fAcceptanceHisto->SetTitle( "" );
       setHistogramPlottingStyle( fAcceptanceHisto, getPlottingColor(), 1., 1.5, 20 );
       fAcceptanceHisto->Draw( "e" );
       fAcceptanceHisto->GetYaxis()->SetTitleOffset( 1.2 );
    }
    if( fAcceptanceHisto )
    {
       fAcceptanceHisto->Draw( "e same" );
    }
    if( fAcceptanceHistoFit ) 
    {
       fAcceptanceHistoFit->Draw( "same" );
    }
    if( fAcceptanceFunction )
    {
       fAcceptanceFunction->Draw( "same" );
    }

    return cX;
}

/*
  
    plot residuals between fit function and measured histogram

*/
TCanvas* VPlotRadialAcceptance::plotResiduals( TCanvas *cX, double i_res_min, double i_res_max )
{
    if( !fAcceptanceFile || fAcceptanceFile->IsZombie() )
    {
       cout << "VPlotRadialAcceptance::plotResiduals() error: data missing";
       return 0;
    }

// canvas
    char hname[2000];
    if( cX ) cX->cd();
    else
    {
       char htitle[2000];
       sprintf( hname, "cAcceptanceResiduals_%s", VUtilities::removeSpaces( fName ).c_str() );
       sprintf( htitle, "%s (residuals)", fName.c_str() );
       cX = new TCanvas( hname, htitle, 420, 10, 400, 400 );
       cX->SetGridx( 0 );
       cX->SetGridy( 0 );
    }

    if( fAcceptanceHisto )
    {
       sprintf( hname, "%s_residual", fAcceptanceHisto->GetName() );
       TH1D *hRes = VHistogramUtilities::get_ResidualHistogram_from_TF1( hname, fAcceptanceHisto, fAcceptanceFunction );

       if( hRes )
       {
	  setHistogramPlottingStyle( hRes );
	  hRes->SetTitle( "" );
	  hRes->SetMinimum( i_res_min );
	  hRes->SetMaximum( i_res_max );
	  hRes->Draw();
	  TLine *iL = new TLine( hRes->GetXaxis()->GetXmin(), 0., hRes->GetXaxis()->GetXmax(), 0. );
	  iL->Draw();
       }
    }

    return cX;
}
