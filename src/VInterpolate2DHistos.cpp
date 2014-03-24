/*! \class VInterpolate2DHistos
 *  \brief interpolate empty bins in 2D histograms
 *
 *
 *  \author Gernot Maier
 *
 */

#include "VInterpolate2DHistos.h"

VInterpolate2DHistos::VInterpolate2DHistos( int iseed )
{
	fRandom = new TRandom3( iseed );
}


TH2F* VInterpolate2DHistos::doSimpleInterpolation( TH2F* h, string iname, int iwidth, int maxiter, bool bError )
{
	if( !h )
	{
		return 0;
	}
	
	char hname[600];
	sprintf( hname, "%s_%s", h->GetName(), iname.c_str() );
	TH2F* hs = ( TH2F* )h->Clone( hname );
	
	double z = 0.;
	double imean = 0.;
	double ierror = 0.;
// iterate several times over histogram
	for( int k = 0; k < maxiter; k++ )
	{
		TH2F* htemp = ( TH2F* )hs->Clone();
		for( int i = 1; i <= hs->GetNbinsX(); i++ )
		{
			for( int j = 1; j <= hs->GetNbinsY(); j++ )
			{
// test if bin has contents
				if( hs->GetBinContent( i, j ) <= 0. )
				{
// get mean of all bins around this bin
					imean = 0.;
					ierror = 0.;
					z = 0.;
					for( int ii = i - iwidth; ii <= i + iwidth; ii++ )
					{
						for( int jj = j - iwidth; jj <= j + iwidth; jj++ )
						{
							if( ii > 0 && jj > 0 && htemp->GetBinContent( ii, jj ) > 0. )
							{
								if( !bError )
								{
									imean += htemp->GetBinContent( ii, jj );
								}
								else
								{
									imean += htemp->GetBinContent( ii, jj ) * htemp->GetBinContent( ii, jj );
								}
								ierror += htemp->GetBinError( ii, jj ) * htemp->GetBinError( ii, jj );
								z++;
							}
						}
					}
					if( z > 0. )
					{
						if( !bError )
						{
							hs->SetBinContent( i, j, imean / z );
						}
						else
						{
							hs->SetBinContent( i, j, sqrt( imean ) / z );
						}
						hs->SetBinError( i, j, sqrt( ierror ) / z );
					}
				}
			}
		}
		delete htemp;
	}
	return hs;
}


TH2F* VInterpolate2DHistos::doGaussianInterpolation( TH2F* h, string iname, TH2F* hNevents, int nGausN, double nWidth )
{
	if( !h || !hNevents )
	{
		return 0;
	}
	
	char hname[600];
	sprintf( hname, "%s_%s", h->GetName(), iname.c_str() );
	TH2F* hs = ( TH2F* )h->Clone( hname );
	
	sprintf( hname, "%s_%s_%s", h->GetName(), iname.c_str(), "temp" );
	TProfile2D h2D( hname, "", hs->GetNbinsX(), hs->GetXaxis()->GetXmin(), hs->GetXaxis()->GetXmax(), hs->GetNbinsY(), hs->GetYaxis()->GetXmin(), hs->GetYaxis()->GetXmax(), -100., 100. );
	
	double x = 0.;
	double y = 0.;
	double xc = 0.;
	double yc = 0.;
	double z = 0.;
	int n = 0;
	
	double xWidth = hs->GetXaxis()->GetBinWidth( 1 );
	double yWidth = hs->GetYaxis()->GetBinWidth( 1 );
	
	for( int i = 1; i <= hs->GetNbinsX(); i++ )
	{
		xc = hs->GetXaxis()->GetBinCenter( i );
		for( int j = 1; j <= hs->GetNbinsY(); j++ )
		{
			yc = hs->GetYaxis()->GetBinCenter( j );
			z  = hs->GetBinContent( i, j );
			n  = ( int )( hNevents->GetBinContent( i, j ) * nGausN );
			
// loop over all bin entries
			for( int k = 0; k < n; k++ )
			{
				x = fRandom->Gaus( xc, xWidth * nWidth / 2. );
				y = fRandom->Gaus( yc, yWidth * nWidth / 2. );
				h2D.Fill( x, y, z );
			}
		}
	}
	
// copy histograms
	for( int i = 1; i <= hs->GetNbinsX(); i++ )
	{
		for( int j = 1; j <= hs->GetNbinsY(); j++ )
		{
			hs->SetBinContent( i, j, h2D.GetBinContent( i, j ) );
		}
	}
	return hs;
}
