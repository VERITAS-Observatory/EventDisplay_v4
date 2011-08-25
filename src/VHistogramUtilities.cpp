/*! \class VHistogramUtilities
    \brief utility class to manipulate histograms

*/

#include "VHistogramUtilities.h"

VHistogramUtilities::VHistogramUtilities()
{
   setDebug( false );
}



/*
    calculate residuals between a 1D histogram and a function

    input:

    string iname :  histogram name for return histogram
    TH1D *h :       input 1D histogram
    TF1 *f :        input 1D function

    return:

    TH1D* :         residual
*/
TH1D* VHistogramUtilities::get_ResidualHistogram_from_TF1( string iname, TH1D *h, TF1 *f )
{
    if( iname.size() == 0 ) return 0;
    if( !h || !f ) return 0;

    TH1D *hDiff = new TH1D( iname.c_str(), "", h->GetNbinsX(), h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax() );

    for( int i = 1; i <= hDiff->GetNbinsX(); i++ )
    {
        if( h->GetBinContent( i ) )
        {
            hDiff->SetBinContent( i, ( h->GetBinContent( i ) - f->Eval( h->GetBinCenter( i ) ) ) / h->GetBinContent( i )  );
            hDiff->SetBinError( i, h->GetBinError( i ) / h->GetBinContent( i ) );
        }
    }

    return hDiff;
}



/*
    get systematic error in reconstruction from a 2D histogram
    (essentially a profile with mean or median)

    options for iMeanType:
    
       mean   :  error bars are the error of the mean
       meanS  :  error bars are the width of the distribution
       median :  median value

*/
TGraphErrors* VHistogramUtilities::get_Profile_from_TH2D( TH2D *iP, TGraphErrors *g, string iMeanType, int rbin, double iXaxisValue )
{
    if( !iP ) return 0;

    if( g == 0 ) g = new TGraphErrors( 1 );

    int zz = 0;
//////////////////////////////////////////////////////////////////////
// median
    if( iMeanType == "median" )
    {
       double i_a[] = { 0.32, 0.5, 0.68 };
       double i_b[] = { 0.0,  0.0, 0.0  };

       for( int b = 1; b <= iP->GetNbinsX(); b++ )
       {
	   if( iP->GetXaxis()->GetBinCenter( b ) < iXaxisValue ) continue;

	   TH1D *h = (TH1D*)iP->ProjectionY( "a", b, b );
	   if( h && h->GetEntries() > 3. )
	   {
	       h->GetQuantiles( 3, i_b, i_a );

	       g->SetPoint( zz, iP->GetXaxis()->GetBinCenter( b ), i_b[1] );
	       g->SetPointError( zz, 0., (i_b[0]+i_b[2])/2./TMath::Sqrt( h->GetEntries() ) );
	       zz++;
	   }
       }
    }
//////////////////////////////////////////////////////////////////////
// mean
    else
    {
       for( int b = 1; b <= iP->GetNbinsX(); b++ )
       {
	   if( iP->GetXaxis()->GetBinCenter( b ) < iXaxisValue ) continue;

	   TH1D *h = (TH1D*)iP->ProjectionY( "a", b, b );
	   if( h && h->GetEntries() > 3. )
	   {
	      g->SetPoint( zz, iP->GetXaxis()->GetBinCenter( b ), h->GetMean() );
	      if( iMeanType == "meanS" ) g->SetPointError( zz, 0., h->GetRMS() );
	      else                       g->SetPointError( zz, 0., h->GetRMS()/TMath::Sqrt( h->GetEntries() ) );   
	      zz++;
           }
	   if( h ) delete h;
       }
    }

//////////////////////////////////////////////////////////////////////
// rebinning
    if( rbin == 2 )
    {
        int z = 0;
        for( int b = 0; b < g->GetN()-1; b = b+2 )
        {
            double x1, y1, x2, y2 ;

            g->GetPoint( b, x1, y1 );
            g->GetPoint( b+1, x2, y2 );
            g->SetPoint( z, (x1+x2)/2., (y1+y2)/2. );
            z++;
        }
        g->Set( (int)(g->GetN()/rbin ) );
    }
    return g;
}

TH1D* VHistogramUtilities::get_Cumulative_Histogram(  TH1D* iH_in, bool iNormalize, bool iLeft_to_right )
{
   if( !iH_in ) return 0;

// clone input histogram
   char hname[600];
   sprintf( hname, "%s_CUMU", iH_in->GetName() );
   TH1D* iH_out = (TH1D*)iH_in->Clone( hname );
   iH_out->Reset();

   if( iLeft_to_right )
   {
       iH_out->SetBinContent( 1, iH_in->GetBinContent( 1 ) );
// loop over all bins
       for( int i = 2; i <= iH_in->GetNbinsX(); i++ )
       {
          iH_out->SetBinContent( i, iH_in->GetBinContent( i ) + iH_out->GetBinContent( i - 1 ) );
       }
    }
    else
    {
       iH_out->SetBinContent( iH_in->GetNbinsX(), iH_in->GetBinContent( iH_in->GetNbinsX() ) );
// loop over all bins
       for( int i = iH_in->GetNbinsX(); i > i; i-- )
       {
          iH_out->SetBinContent( i, iH_in->GetBinContent( i ) + iH_out->GetBinContent( i + 1 ) );
       }
    }
    if( iNormalize && iH_out->GetBinContent( iH_out->GetNbinsX() ) > 0. ) iH_out->Scale( 1./iH_out->GetBinContent( iH_out->GetNbinsX() ) ); 

    iH_out->SetMaximum( iH_out->GetMaximum() * 1.2 );

    return iH_out;
}


TH1D* VHistogramUtilities::get_Bin_Distribution( TH2D *h, int ion, double rmax, double rSource, bool iDiff, TH2D *hTest )
{
    if( !h ) return 0;

// no distance cut for rmax < 6
    if( rmax < 0. ) rmax = 1.e6;

// get binning
    int nbin = 100;
    double xmin = 9999999.;
    double xmax = h->GetMaximum()*1.5;
    for( int i = 1; i <= h->GetNbinsX(); i++ )
    {
        for( int j = 1; j <= h->GetNbinsY(); j++ )
        {
            if( h->GetBinContent(i,j)>-9999. &&  h->GetBinContent(i,j) < xmin ) xmin = h->GetBinContent(i,j);
        }
    }
    if( xmin < 0. ) xmin *= 1.5;
    else            xmin *= 0.8;
    if( xmax < 10. ) nbin = 50;

    xmin = -7.1;
    xmax = 10.9;
    nbin = 100;

    char hname[200];
    if( iDiff ) sprintf( hname, "hdiff1D_%d_%f", ion, rSource );
    else        sprintf( hname, "hsig1D_%d_%f", ion, rSource );

    TH1D *h1D;
    if( gDirectory->Get( hname ) )
    {
        h1D = (TH1D*)gDirectory->Get( hname );
        h1D->Reset();
    }
    else
    {
        h1D = new TH1D( hname, "", nbin, xmin, xmax );
    }

// fill the histogram
    for( int i = 1; i <= h->GetNbinsX(); i++ )
    {
        double x_r = h->GetXaxis()->GetBinCenter( i );
        for( int j = 1; j < h->GetNbinsY(); j++ )
        {
// exclude bins with no on counts
            if( hTest && hTest->GetBinContent( i, j ) == 0. ) continue;

            double y_r = h->GetYaxis()->GetBinCenter( j );
            if( TMath::Sqrt( x_r*x_r + y_r*y_r ) > rmax ) continue;

// exclude source bins
            if( TMath::Sqrt( x_r*x_r + y_r*y_r ) > rSource ) h1D->Fill( h->GetBinContent( i, j ) );
        }
    }
    return h1D;
}
