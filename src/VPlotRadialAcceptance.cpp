/*  \class VPlotRadialAcceptance
    \brief plot radial acceptance curves

*/

#include "VPlotRadialAcceptance.h"

VPlotRadialAcceptance::VPlotRadialAcceptance( string iFile )
{
   fDebug = false;

   setName( "radial acceptance" );

   setAxisRange();

   fAcceptanceFile = 0;
   fAcceptanceHisto = 0;
   fAcceptanceHistoFit = 0;
   fAcceptanceFunction = 0;
   
   if( iFile.size() > 0 ) openAcceptanceFile( iFile, 0 );
}


bool VPlotRadialAcceptance::openAcceptanceFile( string iFile, unsigned int iZeBin )
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

    plot acceptance curves

*/
TCanvas* VPlotRadialAcceptance::plot( TCanvas *cX )
{
    if( !fAcceptanceFile || fAcceptanceFile->IsZombie() )
    {
       cout << "VPlotRadialAcceptance::plot() error: data missing";
       return 0;
    }

    bool bPlotSame = false;
// canvas
    if( cX )
    {
       cX->cd();
       bPlotSame = true;
    }
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
       fAcceptanceHisto->SetMinimum( fAxis_y_min );
       fAcceptanceHisto->SetMaximum( fAxis_y_max );
       fAcceptanceHisto->SetAxisRange( fAxis_x_min, fAxis_x_max );
       fAcceptanceHisto->SetTitle( "" );
       setHistogramPlottingStyle( fAcceptanceHisto, getPlottingColor(), 1., 1.5, 20 );
       if( bPlotSame ) fAcceptanceHisto->Draw( "e same" );
       else            fAcceptanceHisto->Draw( "e" );
       fAcceptanceHisto->GetYaxis()->SetTitleOffset( 1.2 );
    }
//    if( fAcceptanceHisto )
//    {
//       fAcceptanceHisto->Draw( "e same" );
//    }
/*    if( fAcceptanceHistoFit ) 
    {
       setHistogramPlottingStyle( fAcceptanceHistoFit, getPlottingColor(), 1., 1.5, 20 );
       fAcceptanceHistoFit->Draw( "same" );
    } */
    if( fAcceptanceFunction )
    {
       setFunctionPlottingStyle( fAcceptanceFunction, getPlottingColor() );
       fAcceptanceFunction->Draw( "same" );
    }

    return cX;
}

/*
  
    plot residuals between fit function and measured histogram

*/
TCanvas* VPlotRadialAcceptance::plotResiduals( TCanvas *cX, double i_res_min, double i_res_max, bool iPlotChi2 )
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
	  hRes->SetAxisRange( fAxis_x_min, fAxis_x_max );
	  hRes->Draw();
	  TLine *iL = new TLine( hRes->GetXaxis()->GetXmin(), 0., fAxis_x_max, 0. );
	  iL->Draw();

	  if( iPlotChi2 )
	  {
	     double sum2 = 0.;
	     int n = 0;
	     for( int i = 1; i <= fAcceptanceHisto->GetNbinsX(); i++ )
	     {
	        if( fAcceptanceHisto->GetBinContent( i ) > 0. && fAcceptanceHisto->GetBinError( i ) > 0. )
		{
		   sum2 += ( fAcceptanceHisto->GetBinContent( i ) - fAcceptanceFunction->Eval( fAcceptanceHisto->GetBinCenter( i ) ) ) 
		         * ( fAcceptanceHisto->GetBinContent( i ) - fAcceptanceFunction->Eval( fAcceptanceHisto->GetBinCenter( i ) ) )
			 / fAcceptanceHisto->GetBinError( i ) / fAcceptanceHisto->GetBinError( i );
		   n++;
		}
             }
	     sprintf( hname, "Fit Chi2/N: %.2f/%d", sum2, n );
	     TText *iT = new TText( 0.5*fAxis_x_max, 0.7*i_res_max, hname );
	     iT->Draw();
          }
       }
    }

    return cX;
}

void VPlotRadialAcceptance::setAxisRange( double x_min, double x_max, double y_min, double y_max )
{
   fAxis_x_min = x_min;
   fAxis_x_max = x_max;
   fAxis_y_min = y_min;
   fAxis_y_max = y_max;
}

