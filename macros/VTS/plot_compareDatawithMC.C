/*! \file  plot_compareDatawithMC
 *  \brief compare image and shower parameter distribution of simulations and on/off data
 *
 *  ==========================================================================================
 *  THIS MACRO HAS BEEN REPLACED BY A CLASS IN THE SHARED LIBRARY: VPlotCompareDataWithMC
 *  ==========================================================================================
 *
 *  \Author Gernot Maier
 *
 */

#include <iostream>
#include <string>
#include <vector>

#include "TBox.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TLine.h"
#include "TMath.h"
#include "TStyle.h"
#include "TTree.h"

using namespace std;

void plot_energyDependentDistributions( TDirectory* fDir, string iVariable, int iRebin, double x_min, double x_max );
void setHistogramAtt( TH1D* his, int icolor, double iwidth, double isize, int imarker, int irebin );

void help()
{
	cout << endl;
	cout << "compare image and shower parameter distribution of simulations and on/off data" << endl;
	cout << "------------------------------------------------------------------------------" << endl;
	cout << endl;
	cout << "shower parameter distributions:  stereo_parameter(  char *ffile = \"stereo_compare.root\", bool bPoster = false )  " << endl << endl;
	cout << "mscw/mscl energy dependent:      msc_plots( char *ffile = \"stereo_compare.root\", bool bPoster = false )  " << endl << endl;
	cout << "mwr/mlr energy dependent:        mwr_plots( char *ffile = \"stereo_compare.root\", bool bPoster = false )  " << endl << endl;
	cout << "multiplicity plots:              multiplicity_plots( char *ffile = \"stereo_compare.root\" ) " << endl << endl;
	cout << "emission height:                 emission_height( char *ffile = \"stereo_compare.root\" )" << endl << endl;
	cout << "core plots:                      core_plots( char *ffile = \"stereo_compare.root\" )" << endl;
	cout << "core distance plots:             distance_plots( char *ffile = \"stereo_compare.root\", int fNTel, bool bPoster = false )" << endl << endl;
	cout << "centroid plots:                  centroids( char *ffile = \"stereo_compare.root\", int fNtel )" << endl << endl;
	cout << "image parameter distributions:   single_telescope( int telid = 1, char *ifile = \"stereo_compare.root\", bool iOneCanvas = true, bool bPoster = false )" << endl << endl;
	cout << endl;
}

void reflectSims( TH1D* h )
{
	if( !h )
	{
		return;
	}
	
	vector< double > c;
	vector< double > ce;
	
	for( int i = 0; i <= h->GetNbinsX(); i++ )
	{
		c.push_back( h->GetBinContent( i ) );
		ce.push_back( h->GetBinError( i ) );
	}
	
	for( unsigned t = 0; t < c.size(); t++ )
	{
		h->SetBinContent( h->GetNbinsX() - t, c[t] );
		h->SetBinError( h->GetNbinsX() - t, ce[t] );
	}
}


void setAxisTitles( TH2D* h, char* iS, int iTel )
{
	if( !h )
	{
		return;
	}
	char htit[200];
	sprintf( htit, "%s - Telescope %d (%s)", h->GetXaxis()->GetTitle(), iTel, iS );
	h->SetXTitle( htit );
	sprintf( htit, "%s - Telescope %d (%s)", h->GetYaxis()->GetTitle(), iTel, iS );
	h->SetYTitle( htit );
}

TDirectory* openFile( char* ifile )
{
	TFile* fIn = new TFile( ifile );
	if( fIn->IsZombie() )
	{
		return 0;
	}
	
	return ( TDirectory* )fIn;
}

void plotLegend( TH1D* hsims, TH1D* hdiff, double x0 = 0.5 )
{
	if( !hsims || !hdiff )
	{
		return;
	}
	
	TLegend* iLegend = new TLegend( x0, 0.68, 0.85 + ( x0 - 0.5 ), 0.85 );
	iLegend->AddEntry( hsims, "simulations", "pl" );
	iLegend->AddEntry( hdiff, "On-Off (Crab data)", "pl" );
	iLegend->Draw();
}

/*!
  get scale factor between simulations and data

  bContents = 1:   scale to same contents
  bContents = 2:   scale to same maximum value
  bContents = 3:   scale to same maximum value
*/
void getScaling( TH1D* h_sims, TH1D* h_diff, double& s_sims, double& s_diff,
				 int bContents = 1, double xmin = -9999., double xmax = 9999. )
{
	if( !h_sims || !h_diff )
	{
		return;
	}
	double z = 0.;
	////////////////////////////////////
	// scale to same contents (integral)
	if( bContents == 1 )
	{
		int i_min = 1;
		int i_max = h_sims->GetNbinsX();
		if( xmin > -9998 )
		{
			i_min = h_sims->GetXaxis()->FindBin( xmin );
		}
		if( xmax <  9998 )
		{
			i_max = h_sims->GetXaxis()->FindBin( xmax );
		}
		for( int i = i_min; i <= i_max; i++ )
		{
			z += h_sims->GetBinContent( i );
		}
		s_sims = z;
		z = 0;
		i_min = 1;
		i_max = h_diff->GetNbinsX();
		if( xmin > -9998 )
		{
			i_min = h_diff->GetXaxis()->FindBin( xmin );
		}
		if( xmax <  9998 )
		{
			i_max = h_diff->GetXaxis()->FindBin( xmax );
		}
		for( int i = i_min; i <= i_max; i++ )
		{
			if( h_diff->GetBinContent( i ) > 0 )
			{
				z += h_diff->GetBinContent( i );
			}
		}
		
		s_diff = 1.;
		cout << " Bin Content:  data:" << z << "\t sims:" << s_sims << endl;
		if( s_sims > 0. )
		{
			s_sims = z / s_sims;
		}
	}
	//////////////////////////////////
	// scale to same maximum
	else if( bContents == 2 )
	{
		s_sims = h_sims->GetMaximum();
		z      = h_diff->GetMaximum();
		cout << h_sims->GetName() << " Maximum : data:" << z << "\t sims: " << s_sims << endl;
		if( s_sims > 0. )
		{
			s_sims = z / s_sims;
		}
		s_diff = 1.;
	}
	//////////////////////////////////
	// scale to peak (three bins around maximum)
	else if( bContents == 3 )
	{
		int imaxbin = h_sims->GetMaximumBin();
		s_sims = h_sims->GetBinContent( imaxbin );
		if( imaxbin > 1 )
		{
			s_sims += h_sims->GetBinContent( imaxbin - 1 );
		}
		if( imaxbin < h_sims->GetNbinsX() )
		{
			s_sims += h_sims->GetBinContent( imaxbin + 1 );
		}
		
		imaxbin = h_diff->GetMaximumBin();
		z = h_diff->GetBinContent( imaxbin );
		if( imaxbin > 1 )
		{
			z += h_diff->GetBinContent( imaxbin - 1 );
		}
		if( imaxbin < h_diff->GetNbinsX() )
		{
			z += h_diff->GetBinContent( imaxbin + 1 );
		}
		if( s_sims > 0. )
		{
			s_sims = z / s_sims;
		}
		s_diff = 1.;
	}
	
	// make sure that results are positiv
	if( s_sims < 0. )
	{
		s_sims *= -1.;
		s_diff *= -1.;
	}
	cout << "Scaling: SIMS " << s_sims << "\t" << z << "\t DIFF " << s_diff << endl;
}

void getScaling( TDirectory* fDir, double& s_sims, double& s_diff, string his = "MSCW",
				 int bContents = 1, double xmin = -9999., double xmax = 9999. )
{
	if( fDir == 0 )
	{
		cout << "NO SCALING POSSIBLE, no file" << endl;
		s_sims = 1.;
		s_diff = 1.;
		return;
	}
	cout << "scale on histograms " << his << ", scale to";
	if( bContents == 1 )
	{
		cout << " histogram contents" << endl;
	}
	else if( bContents == 2 )
	{
		cout << " histogram maximum" << endl;
	}
	else if( bContents == 3 )
	{
		cout << " histogram maximum (peak)" << endl;
	}
	
	char hname[200];
	sprintf( hname, "h%s_SIMS", his.c_str() );
	TH1D* h_sims = ( TH1D* )fDir->Get( hname );
	sprintf( hname, "h%s_DIFF", his.c_str() );
	TH1D* h_diff = ( TH1D* )fDir->Get( hname );
	if( !h_sims || !h_diff )
	{
		cout << "NO SCALING POSSIBLE, no histograms " << h_sims << " " << h_diff << endl;
		s_sims = 1.;
		s_diff = 1.;
		return;
	}
	
	getScaling( h_sims, h_diff, s_sims, s_diff, bContents, xmin, xmax );
}


void setHistogramAtt( TH2D* his, double imin )
{
	if( !his )
	{
		return;
	}
	
	if( imin > -900. )
	{
		his->SetMinimum( imin );
	}
	his->SetStats( 0 );
	his->GetYaxis()->SetTitleOffset( 1.3 );
}

void setHistogramAtt( TH1D* his, int icolor, double iwidth, double isize )
{
	setHistogramAtt( his, icolor, iwidth, isize, 1, 1 );
}

void setHistogramAtt( TH1D* his, int icolor, double iwidth, double isize, int irebin )
{
	setHistogramAtt( his, icolor, iwidth, isize, 0, irebin );
}

void setHistogramAtt( TH1D* his, int icolor, double iwidth, double isize, int imarker, int irebin )
{
	his->SetLineColor( icolor );
	his->SetMarkerColor( icolor );
	his->SetLineWidth( ( Width_t )iwidth );
	his->SetMarkerSize( isize );
	if( imarker != 0 )
	{
		his->SetMarkerStyle( imarker );
	}
	his->SetStats( 0 );
	his->GetYaxis()->SetTitleOffset( 1.3 );
	if( irebin != 1 )
	{
		his->Rebin( irebin );
	}
}

TF1* do_theta2Fit( TH1D* h, int icolor = 1, int istyle = 1 )
{
	if( !h )
	{
		return 0;
	}
	
	cout << "now fitting " << h->GetName() << endl;
	
	char hname[200];
	sprintf( hname, "fT2_%s", h->GetName() );
	TF1* fTheta2 = new TF1( hname, "[0]*([1]/2./[2]/[2]*TMath::Exp(-x/2./[2]/[2])+(1-[1])/2./[3]/[3]*TMath::Exp(-x/2./[3]/[3]))", 0., 0.3 );
	fTheta2->SetParameter( 0, 5. );
	fTheta2->SetParameter( 1, 0.5 );
	fTheta2->SetParLimits( 1, 0., 1. );
	fTheta2->SetParameter( 2, 0.03 );
	fTheta2->SetParLimits( 2, 0., 1. );
	fTheta2->SetParameter( 3, 0.09 );
	fTheta2->SetParLimits( 3, 0., 1. );
	fTheta2->SetLineColor( icolor );
	fTheta2->SetLineStyle( istyle );
	
	h->Fit( fTheta2, "REM0" );
	//  cout << "CHI2 " << fTheta2->GetChisquare()/htheta2_diff->GetXaxis()->GetXmax() << endl;
	
	cout << endl;
	
	return fTheta2;
}

void plotRelativePlots( char* i_CanvasName, char* i_CanvasTitle, TH1D* h1, TH1D* h2, double xmin, double xmax )
{
	if( !h1 || !h2 )
	{
		return;
	}
	
	char hname[600];
	char htitle[600];
	sprintf( hname, "rel_%s", i_CanvasName );
	sprintf( htitle, "%s rel. plot", i_CanvasTitle );
	TCanvas* cRel = new TCanvas( hname, htitle, 200, 200, 400, 400 );
	cRel->SetGridx( 0 );
	cRel->SetGridy( 0 );
	
	sprintf( hname, "rel_%s", h1->GetName() );
	TH1D* hR = ( TH1D* )h1->Clone( hname );
	setHistogramAtt( hR, 1, 3, 1, 20, 1 );
	hR->Divide( h2 );
	hR->SetMinimum( 0.3 );
	hR->SetMaximum( 3.0 );
	hR->GetYaxis()->SetTitle( "sims / data" );
	hR->SetAxisRange( xmin, xmax );
	
	hR->Draw();
	
	TLine* iL = new TLine( xmin, 1., xmax, 1. );
	iL->SetLineStyle( 2 );
	iL->Draw();
}

void multiplicity_plots( char* ffile = "stereo_compare.root" )
{
	gStyle->SetPadGridX( 0 );
	gStyle->SetPadGridY( 0 );
	gStyle->SetPalette( 1 );
	
	TDirectory* fDir = openFile( ffile );
	
	// get the scaling between simulations and data
	double s_sims = 1.;
	double s_diff = 1.;
	getScaling( fDir, s_sims, s_diff, "MSCW", 1 );
	
	char hname[600];
	char htitle[600];
	
	// canvases
	sprintf( hname, "cTrigger_%s", ffile );
	sprintf( htitle, "multiplicity plots (%s)", ffile );
	TCanvas* cTrigger = new TCanvas( hname, htitle, 10, 10, 800, 400 );
	cTrigger->SetGridx( 0 );
	cTrigger->SetGridy( 0 );
	cTrigger->Divide( 2, 1 );
	
	sprintf( hname, "cTriggerRel_%s", ffile );
	sprintf( htitle, "multiplicity plots (relative dist., %s)", ffile );
	TCanvas* cTriggerRel = new TCanvas( hname, htitle, 410, 10, 800, 400 );
	cTriggerRel->SetGridx( 0 );
	cTriggerRel->SetGridy( 0 );
	cTriggerRel->Divide( 2, 1 );
	
	TH1D* hNImages_SIMS = ( TH1D* )fDir->Get( "hNimages_SIMS" );
	TH1D* hNImages_DIFF = ( TH1D* )fDir->Get( "hNimages_DIFF" );
	TH1D* hImgSel_SIMS = ( TH1D* )fDir->Get( "hImgSel_SIMS" );
	TH1D* hImgSel_DIFF = ( TH1D* )fDir->Get( "hImgSel_DIFF" );
	
	if( !hNImages_SIMS || !hNImages_DIFF || !hImgSel_SIMS || !hImgSel_DIFF )
	{
		return;
	}
	
	setHistogramAtt( hNImages_SIMS, 2, 3, 1, 20, 1 );
	setHistogramAtt( hNImages_DIFF, 1, 3, 1, 21, 1 );
	if( hNImages_SIMS->GetEntries() > 0 )
	{
		hNImages_SIMS->Scale( s_sims );
	}
	if( hNImages_DIFF->GetEntries() > 0 )
	{
		hNImages_DIFF->Scale( s_diff );
	}
	
	cTrigger->cd( 1 );
	gPad->SetGridx( 0 );
	gPad->SetGridy( 0 );
	hNImages_SIMS->SetMaximum( hNImages_SIMS->GetMaximum() * 1.2 );
	hNImages_SIMS->Draw();
	hNImages_SIMS->SetYTitle( "number of showers [a.u.]" );
	hNImages_DIFF->Draw( "same" );
	
	setHistogramAtt( hImgSel_SIMS, 2, 3, 1, 20, 1 );
	setHistogramAtt( hImgSel_DIFF, 1, 3, 1, 21, 1 );
	if( hImgSel_SIMS->GetEntries() > 0 )
	{
		hImgSel_SIMS->Scale( s_sims );
	}
	if( hImgSel_DIFF->GetEntries() > 0 )
	{
		hImgSel_DIFF->Scale( s_diff );
	}
	
	cTrigger->cd( 2 );
	gPad->SetGridx( 0 );
	gPad->SetGridy( 0 );
	hImgSel_SIMS->SetMaximum( hImgSel_SIMS->GetMaximum() * 1.2 );
	hImgSel_SIMS->Draw();
	hImgSel_SIMS->SetYTitle( "number of showers [a.u.]" );
	hImgSel_DIFF->Draw( "same" );
	
	// relative plots
	cTriggerRel->cd( 1 );
	sprintf( hname, "%s_RE", hNImages_SIMS->GetName() );
	TH1D* hNImages_Rel = ( TH1D* )hNImages_SIMS->Clone( hname );
	hNImages_Rel->Divide( hNImages_DIFF );
	hNImages_Rel->SetYTitle( "sims/data" );
	hNImages_Rel->SetMinimum( 0.3 );
	hNImages_Rel->SetMaximum( 3.0 );
	setHistogramAtt( hNImages_Rel, 1, 1, 1, 21, 1 );
	hNImages_Rel->Draw();
	TLine* iLNI = new TLine( hNImages_Rel->GetXaxis()->GetXmin(), 1., hNImages_Rel->GetXaxis()->GetXmax(), 1. );
	iLNI->SetLineStyle( 2 );
	iLNI->Draw();
	
	cTriggerRel->cd( 2 );
	sprintf( hname, "%s_RE", hImgSel_SIMS->GetName() );
	TH1D* hImgSel_Rel = ( TH1D* )hImgSel_SIMS->Clone( hname );
	hImgSel_Rel->Divide( hImgSel_DIFF );
	hImgSel_Rel->SetYTitle( "sims/data" );
	hImgSel_Rel->SetMinimum( 0.3 );
	hImgSel_Rel->SetMaximum( 3.0 );
	setHistogramAtt( hImgSel_Rel, 1, 1, 1, 21, 1 );
	hImgSel_Rel->Draw();
	TLine* iLIS = new TLine( hImgSel_Rel->GetXaxis()->GetXmin(), 1., hImgSel_Rel->GetXaxis()->GetXmax(), 1. );
	iLIS->SetLineStyle( 2 );
	iLIS->Draw();
	
}

/*

  plot three single canvases for a certain histogram

*/
void plot_singleCanvas( char* ffile, string iHistoName, string iCanvasTitle, double iHistoXAxisMax )
{
	gStyle->SetPadGridX( 0 );
	gStyle->SetPadGridY( 0 );
	gStyle->SetPalette( 1 );
	
	TDirectory* fDir = openFile( ffile );
	
	char hname[600];
	char htitle[600];
	// canvas with on/off histograms
	sprintf( hname, "c%sOO_%s", iHistoName.c_str(), ffile );
	sprintf( htitle, "%s (on/off, %s)", iCanvasTitle.c_str(), ffile );
	TCanvas* cEMHOO = new TCanvas( hname, htitle, 410, 410, 400, 400 );
	cEMHOO->SetGridx( 0 );
	cEMHOO->SetGridy( 0 );
	
	sprintf( hname, "%s_ON", iHistoName.c_str() );
	TH1D* hHistogram_ON = ( TH1D* )fDir->Get( hname );
	sprintf( hname, "%s_OFF", iHistoName.c_str() );
	TH1D* hHistogram_OFF = ( TH1D* )fDir->Get( hname );
	if( hHistogram_ON && hHistogram_OFF )
	{
		setHistogramAtt( hHistogram_ON, 3, 3, 1, 20, 1 );
		setHistogramAtt( hHistogram_OFF, 4, 3, 1, 20, 1 );
		
		hHistogram_ON->SetMaximum( hHistogram_ON->GetMaximum() * 1.4 );
		hHistogram_ON->SetAxisRange( 0., iHistoXAxisMax );
		hHistogram_ON->Draw();
		hHistogram_OFF->Draw( "same" );
	}
	
	// get the scaling between simulations and data
	double s_sims = 1.;
	double s_diff = 1.;
	getScaling( fDir, s_sims, s_diff, "MSCW", 1 );
	
	sprintf( hname, "c%s_%s", iHistoName.c_str(), ffile );
	sprintf( htitle, "%s (%s)", iCanvasTitle.c_str(), ffile );
	TCanvas* cEMH = new TCanvas( hname, htitle, 610, 410, 400, 400 );
	cEMH->SetGridx( 0 );
	cEMH->SetGridy( 0 );
	
	sprintf( hname, "%s_SIMS", iHistoName.c_str() );
	TH1D* hHistogram_SIMS = ( TH1D* )fDir->Get( hname );
	sprintf( hname, "%s_DIFF", iHistoName.c_str() );
	TH1D* hHistogram_DIFF = ( TH1D* )fDir->Get( hname );
	
	if( !hHistogram_SIMS || !hHistogram_DIFF )
	{
		return;
	}
	
	setHistogramAtt( hHistogram_SIMS, 2, 3, 1, 20, 1 );
	setHistogramAtt( hHistogram_DIFF, 1, 3, 1, 21, 1 );
	cout << "SCALE " << s_sims << "\t" << s_diff << endl;
	if( hHistogram_SIMS->GetEntries() > 0 )
	{
		hHistogram_SIMS->Scale( s_sims );
	}
	if( hHistogram_DIFF->GetEntries() > 0 && TMath::Abs( s_sims - 1. ) > 1.e-4 )
	{
		hHistogram_DIFF->Scale( s_diff );
	}
	
	hHistogram_SIMS->SetMaximum( hHistogram_SIMS->GetMaximum() * 1.4 );
	hHistogram_SIMS->SetAxisRange( 0., iHistoXAxisMax );
	hHistogram_SIMS->Draw();
	hHistogram_DIFF->Draw( "same" );
	
	plotRelativePlots( hname, htitle, hHistogram_SIMS, hHistogram_DIFF, 0., iHistoXAxisMax );
}

void emission_height( char* ffile = "stereo_compare.root", double iEmissionHeightMax = 40. )
{
	plot_singleCanvas( ffile, "hEmissionHeight", "emission height", iEmissionHeightMax );
}


/*

    plot mscw and mscl energy dependent

*/
void msc_plots( char* ffile = "stereo_compare.root", int iRebin = 4 )
{
	gStyle->SetPadGridX( 0 );
	gStyle->SetPadGridY( 0 );
	gStyle->SetPalette( 1 );
	
	TDirectory* fDir = openFile( ffile );
	
	plot_energyDependentDistributions( fDir, "MSCW", iRebin, -1.5, 2.5 );
	plot_energyDependentDistributions( fDir, "MSCL", iRebin, -1.5, 2.5 );
}

void plot_energyDependentDistributions( TDirectory* fDir, string iVariable, int iRebin, double x_min, double x_max )
{
	if( fDir == 0 )
	{
		return;
	}
	
	double KSProb = 0;
	double KSSig = 0;
	double Chi2Prob = 0.;
	double Chi2Sig = 0.;
	
	// get the scaling between simulations and data
	double s_sims = 1.;
	double s_diff = 1.;
	//  getScaling( fDir, s_sims, s_diff, iVariable.c_str(), 2 );
	
	char hname[600];
	char htitle[600];
	sprintf( hname, "c_%s_%s", iVariable.c_str(), fDir->GetName() );
	sprintf( htitle, "%s (%s)", iVariable.c_str(), fDir->GetName() );
	TCanvas* c_MS = new TCanvas( hname, htitle, 100, 10, 900, 600 );
	c_MS->SetGridx( 0 );
	c_MS->SetGridy( 0 );
	c_MS->Divide( 3, 2 );
	
	sprintf( hname, "h%sErec_SIMS", iVariable.c_str() );
	TH2D* h_sims = ( TH2D* )fDir->Get( hname );
	sprintf( hname, "h%sErec_DIFF", iVariable.c_str() );
	TH2D* h_diff = ( TH2D* )fDir->Get( hname );
	
	if( !h_sims || !h_diff )
	{
		return;
	}
	
	// loop over all bins in energy
	for( int i = 1; i <= h_sims->GetXaxis()->GetNbins(); i++ )
	{
		sprintf( hname, "h%sErec_SIMS_%d", iVariable.c_str(), i );
		TH1D* hSims = h_sims->ProjectionY( hname, i, i );
		setHistogramAtt( hSims, 2, 1, 1, 20, iRebin );
		sprintf( hname, "h_%sErec_diff_%d", iVariable.c_str(), i );
		TH1D* hDiff = h_diff->ProjectionY( hname, i, i );
		setHistogramAtt( hDiff, 1, 1, 1, 21, iRebin );
		
		getScaling( hSims, hDiff, s_sims, s_diff, 3 );
		
		if( hSims->GetEntries() > 0 )
		{
			hSims->Scale( s_sims );
		}
		if( hSims->GetEntries() > 0 )
		{
			hDiff->Scale( s_diff );
		}
		
		hSims->SetAxisRange( x_min, x_max );
		hDiff->SetAxisRange( x_min, x_max );
		hSims->SetMaximum( hSims->GetMaximum() * 1.8 );
		hSims->SetMinimum( 0. );
		
		// calculate matching of distributions
		KSProb = hSims->KolmogorovTest( hDiff );
		KSSig  = TMath::ErfInverse( 1. - KSProb ) * TMath::Sqrt( 2. );
		Chi2Prob = hDiff->Chi2Test( hSims, "WW" );
		Chi2Sig = TMath::ErfInverse( 1. - Chi2Prob ) * TMath::Sqrt( 2. );
		
		// draw histograms
		c_MS->cd( i );
		hSims->Draw();
		hDiff->Draw( "same" );
		
		TLine* lLine = new TLine( 0., hSims->GetMinimum(), 0., hSims->GetMaximum() );
		lLine->SetLineStyle( 2 );
		lLine->Draw();
		
		sprintf( hname, "%.1f < log_{10} E_{rec} < %.1f", h_sims->GetXaxis()->GetBinLowEdge( i ),  h_sims->GetXaxis()->GetBinUpEdge( i ) );
		TLatex* iT = new TLatex( x_min + 0.1 * ( x_max - x_min ), 0.9 * hSims->GetMaximum(), hname );
		iT->SetTextSize( iT->GetTextSize() * 0.6 );
		iT->Draw();
		sprintf( hname, "mean (MC): %.2f#pm %.2f, mean (data): %.2f#pm %.2f", hSims->GetMean(), hSims->GetRMS(), hDiff->GetMean(), hDiff->GetRMS() );
		TLatex* iM = new TLatex( x_min + 0.1 * ( x_max - x_min ), 0.84 * hSims->GetMaximum(), hname );
		iM->SetTextSize( iM->GetTextSize() * 0.6 );
		iM->Draw();
		sprintf( hname, "KS-test | P = %1.2e (%1.1f #sigma)", KSProb, KSSig );
		TLatex* iK = new TLatex( x_min + 0.1 * ( x_max - x_min ), 0.78 * hSims->GetMaximum(), hname );
		iK->SetTextSize( iK->GetTextSize() * 0.6 );
		iK->Draw();
		sprintf( hname, "Chi2 | P = %1.2e (%1.1f #sigma)", Chi2Prob, Chi2Sig );
		TLatex* iC = new TLatex( x_min + 0.1 * ( x_max - x_min ), 0.72 * hSims->GetMaximum(), hname );
		iC->SetTextSize( iC->GetTextSize() * 0.6 );
		iC->Draw();
	}
	
}

/*
 *
 * plot stereo parameter
 *
 */
void stereo_parameter( char* ffile = "stereo_compare.root", bool bPoster = false )
{
	gStyle->SetPadGridX( 0 );
	gStyle->SetPadGridY( 0 );
	gStyle->SetPalette( 1 );
	
	TDirectory* fDir = openFile( ffile );
	
	// two canvases, one with sims and diff, one with on/off histograms
	
	char hname[600];
	char htitle[600];
	TCanvas* cOO = new TCanvas( "cOO", "on/off (stereo parameter)", 100, 10, 600, 600 );
	cOO->SetGridx( 0 );
	cOO->SetGridy( 0 );
	cOO->Divide( 2, 2 );
	
	sprintf( hname, "cSD_%s", ffile );
	sprintf( htitle, "sims/diff (stereo parameter, %s)", ffile );
	TCanvas* cSD = new TCanvas( hname, htitle, 10, 10, 600, 600 );
	cSD->SetGridx( 0 );
	cSD->SetGridy( 0 );
	cSD->Divide( 2, 2 );
	
	TCanvas* cT2oo = 0;
	TCanvas* cT2sim = 0;
	TCanvas* cLT2oo = 0;
	TCanvas* cLT2sim = 0;
	TCanvas* cMSCWoo = 0;
	TCanvas* cMSCWsim = 0;
	TCanvas* cMSCLoo = 0;
	TCanvas* cMSCLsim = 0;
	
	if( bPoster )
	{
		cT2oo = new TCanvas( "cT2oo", "cT2oo", 10, 10, 400, 400 );
		cT2oo->SetGridx( 0 );
		cT2oo->SetGridy( 0 );
		
		cT2sim = new TCanvas( "cT2sim", "cT2sim", 10, 510, 400, 400 );
		cT2sim->SetGridx( 0 );
		cT2sim->SetGridy( 0 );
		
		cLT2oo = new TCanvas( "cLT2oo", "cLT2oo", 210, 10, 400, 400 );
		cLT2oo->SetGridx( 0 );
		cLT2oo->SetGridy( 0 );
		
		cLT2sim = new TCanvas( "cLT2sim", "cLT2sim", 210, 510, 400, 400 );
		cLT2sim->SetGridx( 0 );
		cLT2sim->SetGridy( 0 );
		
		cMSCWoo = new TCanvas( "cMSCWoo", "cMSCWoo", 450, 10, 400, 400 );
		cMSCWoo->SetGridx( 0 );
		cMSCWoo->SetGridy( 0 );
		
		cMSCWsim = new TCanvas( "cMSCWsim", "cMSCWsim", 450, 510, 400, 400 );
		cMSCWsim->SetGridx( 0 );
		cMSCWsim->SetGridy( 0 );
		
		cMSCLoo = new TCanvas( "cMSCLoo", "cMSCLoo", 900, 10, 400, 400 );
		cMSCLoo->SetGridx( 0 );
		cMSCLoo->SetGridy( 0 );
		
		cMSCLsim = new TCanvas( "cMSCLsim", "cMSCLsim", 900, 510, 400, 400 );
		cMSCLsim->SetGridx( 0 );
		cMSCLsim->SetGridy( 0 );
	}
	
	// theta2 < 0.02
	//
	
	TH1D* ht2_sims = ( TH1D* )fDir->Get( "htheta2_SIMS" );
	if( bPoster )
	{
		setHistogramAtt( ht2_sims, 2, 1, 2, 20, 1 );
	}
	else
	{
		setHistogramAtt( ht2_sims, 2, 1, 1, 20, 1 );
	}
	ht2_sims->SetYTitle( "number of shower [a.u.]" );
	
	TH1D* ht2_diff = ( TH1D* )fDir->Get( "htheta2_DIFF" );
	if( bPoster )
	{
		setHistogramAtt( ht2_diff, 1, 3, 2, 25, 1 );
	}
	else
	{
		setHistogramAtt( ht2_diff, 1, 1, 1, 25, 1 );
	}
	
	TH1D* ht2_on = ( TH1D* )fDir->Get( "htheta2_ON" );
	setHistogramAtt( ht2_on, 3, 1, 1, 20, 1 );
	ht2_on->SetYTitle( "number of shower [a.u.]" );
	
	TH1D* ht2_off = ( TH1D* )fDir->Get( "htheta2_OFF" );
	setHistogramAtt( ht2_off, 4, 1, 1, 21, 1 );
	
	ht2_sims->SetAxisRange( 0., 0.05 );
	ht2_on->SetAxisRange( 0., 0.2 );
	
	// get the scaling between simulations and data
	double s_sims = 1.;
	double s_diff = 1.;
	getScaling( fDir, s_sims, s_diff, "theta2", 1 );
	
	ht2_sims->Scale( s_sims );
	ht2_diff->Scale( s_diff );
	
	// plot everything
	
	cSD->cd( 1 );
	gPad->SetLeftMargin( 0.14 );
	ht2_sims->GetYaxis()->SetTitleOffset( 1.5 );
	ht2_sims->Draw();
	ht2_diff->Draw( "same" );
	
	cOO->cd( 1 );
	ht2_on->Draw();
	ht2_off->Draw( "same" );
	
	if( bPoster )
	{
		cT2oo->cd();
		ht2_on->Draw();
		ht2_off->Draw( "same" );
		
		cT2sim->cd();
		// fit both theta2 functions
		TF1* ft2_sims = do_theta2Fit( ht2_sims, 2, 2 );
		ft2_sims->SetLineStyle( 2 );
		ft2_sims->SetLineColor( 2 );
		TF1* ft2_diff = do_theta2Fit( ht2_diff );
		ft2_diff->SetLineStyle( 1 );
		ft2_diff->SetLineColor( 1 );
		
		ht2_sims->SetMinimum( -10 );
		ht2_sims->Draw();
		TBox* iB = new TBox( 0., ht2_sims->GetMinimum(), 0.035, ht2_sims->GetMaximum() );
		iB->SetFillColor( 18 );
		iB->SetFillStyle( 1001 );
		iB->Draw();
		ht2_sims->Draw( "axis same" );
		ht2_sims->Draw( "same" );
		ht2_diff->Draw( "same" );
		ft2_sims->Draw( "same" );
		ft2_diff->Draw( "same" );
		
		
		plotLegend( ht2_sims, ht2_diff );
	}
	
	// theta2 < 0.02
	//
	
	TH1D* hlt2_sims = ( TH1D* )fDir->Get( "hltheta2_SIMS" );
	if( bPoster )
	{
		setHistogramAtt( hlt2_sims, 2, 3, 2, 20, 1 );
	}
	else
	{
		setHistogramAtt( hlt2_sims, 2, 3, 1, 20, 1 );
	}
	hlt2_sims->SetYTitle( "number of shower [a.u.]" );
	
	TH1D* hlt2_diff = ( TH1D* )fDir->Get( "hltheta2_DIFF" );
	if( bPoster )
	{
		setHistogramAtt( hlt2_diff, 1, 3, 2, 25, 1 );
	}
	else
	{
		setHistogramAtt( hlt2_diff, 1, 3, 1, 25, 1 );
	}
	
	TH1D* hlt2_on = ( TH1D* )fDir->Get( "hltheta2_ON" );
	setHistogramAtt( hlt2_on, 3, 3, 1, 20, 1 );
	
	TH1D* hlt2_off = ( TH1D* )fDir->Get( "hltheta2_OFF" );
	setHistogramAtt( hlt2_off, 4, 3, 1, 21, 1 );
	
	getScaling( fDir, s_sims, s_diff, "ltheta2", 2 );
	hlt2_sims->Scale( s_sims );
	hlt2_diff->Scale( s_diff );
	
	hlt2_sims->SetAxisRange( -5., -1. );
	hlt2_on->SetAxisRange( -5., 2. );
	
	cSD->cd( 2 );
	gPad->SetLeftMargin( 0.14 );
	hlt2_sims->GetYaxis()->SetTitleOffset( 1.5 );
	hlt2_sims->SetMaximum( hlt2_sims->GetMaximum() * 1.3 );
	hlt2_sims->Draw();
	hlt2_diff->Draw( "same" );
	
	cOO->cd( 2 );
	hlt2_on->Draw();
	hlt2_off->Draw( "same" );
	
	if( bPoster )
	{
		cLT2oo->cd();
		hlt2_on->Draw();
		hlt2_off->Draw( "same" );
		
		cLT2sim->cd();
		hlt2_sims->Draw();
		hlt2_diff->Draw( "same" );
		plotLegend( hlt2_sims, hlt2_diff );
	}
	
	
	// MSCW
	//
	
	TH1D* hmscw_sims = ( TH1D* )fDir->Get( "hMSCW_SIMS" );
	if( bPoster )
	{
		setHistogramAtt( hmscw_sims, 2, 1, 1, 20, 2 );
	}
	else
	{
		setHistogramAtt( hmscw_sims, 2, 1, 1, 20, 2 );
	}
	hmscw_sims->SetYTitle( "number of shower [a.u.]" );
	
	TH1D* hmscw_diff = ( TH1D* )fDir->Get( "hMSCW_DIFF" );
	if( bPoster )
	{
		setHistogramAtt( hmscw_diff, 1, 1, 1, 25, 2 );
	}
	else
	{
		setHistogramAtt( hmscw_diff, 1, 1, 1, 25, 2 );
	}
	
	TH1D* hmscw_on = ( TH1D* )fDir->Get( "hMSCW_ON" );
	setHistogramAtt( hmscw_on, 3, 1, 1, 20, 1 );
	hmscw_on->SetYTitle( "number of shower [a.u.]" );
	
	TH1D* hmscw_off = ( TH1D* )fDir->Get( "hMSCW_OFF" );
	setHistogramAtt( hmscw_off, 4, 1, 1, 21, 1 );
	
	hmscw_sims->SetAxisRange( -1., 1. );
	getScaling( fDir, s_sims, s_diff, "MSCW", 2, -0.5, 0.5 );
	//   getScaling( fDir, s_sims, s_diff, "MSCW", 1, -0.5, 0.5 );
	if( hmscw_sims->GetEntries() > 0 )
	{
		hmscw_sims->Scale( s_sims );
	}
	if( hmscw_diff->GetEntries() > 0 )
	{
		hmscw_diff->Scale( s_diff );
	}
	
	cSD->cd( 3 );
	gPad->SetLeftMargin( 0.14 );
	hmscw_sims->GetYaxis()->SetTitleOffset( 1.5 );
	hmscw_sims->SetMaximum( hmscw_sims->GetMaximum() * 1.3 );
	hmscw_sims->Draw();
	hmscw_diff->Draw( "same" );
	TLine* lmscw = new TLine( 0., hmscw_sims->GetMinimum(), 0., hmscw_sims->GetMaximum() );
	lmscw->SetLineStyle( 2 );
	lmscw->Draw();
	
	cOO->cd( 3 );
	hmscw_on->Draw( "" );
	hmscw_off->Draw( "same" );
	
	if( bPoster )
	{
		cMSCWoo->cd();
		hmscw_on->Draw( "" );
		hmscw_off->Draw( "same" );
		
		
		cMSCWsim->cd();
		hmscw_sims->SetMinimum( 0. );
		hmscw_sims->Draw( "e" );
		hmscw_diff->Draw( "e same" );
		plotLegend( hmscw_sims, hmscw_diff );
		TLine* iL = new TLine( 0., hmscw_sims->GetMinimum(), 0., hmscw_sims->GetMaximum() );
		iL->SetLineStyle( 2 );
		iL->Draw();
	}
	
	
	
	// MSCL
	//
	
	TH1D* hmscl_sims = ( TH1D* )fDir->Get( "hMSCL_SIMS" );
	if( bPoster )
	{
		setHistogramAtt( hmscl_sims, 2, 3, 2, 21, 1 );
	}
	else
	{
		setHistogramAtt( hmscl_sims, 2, 3, 1, 21, 2 );
	}
	hmscl_sims->SetYTitle( "number of shower [a.u.]" );
	
	TH1D* hmscl_on = ( TH1D* )fDir->Get( "hMSCL_ON" );
	setHistogramAtt( hmscl_on, 3, 3, 1, 20, 1 );
	
	TH1D* hmscl_off = ( TH1D* )fDir->Get( "hMSCL_OFF" );
	setHistogramAtt( hmscl_off, 4, 3, 1, 21, 1 );
	
	TH1D* hmscl_diff = ( TH1D* )fDir->Get( "hMSCL_DIFF" );
	if( bPoster )
	{
		setHistogramAtt( hmscl_diff, 1, 3, 2, 25, 1 );
	}
	else
	{
		setHistogramAtt( hmscl_diff, 1, 3, 1, 25, 2 );
	}
	hmscl_diff->SetLineWidth( 3 );
	hmscl_diff->SetStats( 0 );
	
	hmscl_sims->SetAxisRange( -1., 1. );
	hmscl_on->SetAxisRange( -2., 10. );
	getScaling( fDir, s_sims, s_diff, "MSCL", 1, -0.75, 0.75 );
	if( hmscl_sims->GetEntries() > 0 )
	{
		hmscl_sims->Scale( s_sims );
	}
	if( hmscl_diff->GetEntries() > 0 )
	{
		hmscl_diff->Scale( s_diff );
	}
	
	cSD->cd( 4 );
	gPad->SetLeftMargin( 0.14 );
	hmscl_sims->GetYaxis()->SetTitleOffset( 1.5 );
	hmscl_sims->SetMaximum( hmscl_sims->GetMaximum() * 1.3 );
	hmscl_sims->Draw();
	hmscl_diff->Draw( "same" );
	TLine* lmscl = new TLine( 0., hmscl_sims->GetMinimum(), 0., hmscl_sims->GetMaximum() );
	lmscl->SetLineStyle( 2 );
	lmscl->Draw();
	
	cOO->cd( 4 );
	hmscl_on->Draw();
	hmscl_off->Draw( "same" );
	
	if( bPoster )
	{
		cMSCLoo->cd();
		hmscl_on->Draw( "" );
		hmscl_off->Draw( "same" );
		
		cMSCLsim->cd();
		hmscl_sims->Draw( "e " );
		hmscl_diff->Draw( "e same" );
		plotLegend( hmscl_sims, hmscl_diff );
		TLine* iL = new TLine( 0., hmscl_sims->GetMinimum(), 0., hmscl_sims->GetMaximum() );
		iL->SetLineStyle( 2 );
		iL->Draw();
	}
}

void core_plots( char* ifile = "stereo_compare.root", int iScaling = 1 )
{
	gStyle->SetPadGridX( 0 );
	gStyle->SetPadGridY( 0 );
	gStyle->SetPalette( 1 );
	
	TDirectory* fDir = openFile( ifile );
	
	//
	TCanvas* cOCore = new TCanvas( "cOCore", "on/off (core positions)", 100, 10, 600, 600 );
	cOCore->SetGridx( 0 );
	cOCore->SetGridy( 0 );
	cOCore->Divide( 2, 2 );
	
	TCanvas* cSCore = new TCanvas( "cSCore", "sims/diff (core positions)", 10, 10, 600, 600 );
	cSCore->SetGridx( 0 );
	cSCore->SetGridy( 0 );
	cSCore->Divide( 2, 2 );
	
	// xcore
	//
	TH1D* hXcore_sims = ( TH1D* )fDir->Get( "hXcore_SIMS" );
	setHistogramAtt( hXcore_sims, 2, 3, 1, 20, 8 );
	hXcore_sims->SetMaximum( hXcore_sims->GetMaximum() * 1.5 );
	
	TH1D* hXcore_on = ( TH1D* )fDir->Get( "hXcore_ON" );
	setHistogramAtt( hXcore_on, 3, 3, 2, 8 );
	
	TH1D* hXcore_off = ( TH1D* )fDir->Get( "hXcore_OFF" );
	setHistogramAtt( hXcore_off, 4, 3, 2, 8 );
	
	TH1D* hXcore_diff = ( TH1D* )fDir->Get( "hXcore_DIFF" );
	setHistogramAtt( hXcore_diff, 1, 3, 1, 21, 8 );
	
	hXcore_sims->SetAxisRange( -250., 250. );
	hXcore_on->SetAxisRange( -250., 250. );
	
	double nSims = 0.;
	double nDiff = 0.;
	
	getScaling( fDir, nSims, nDiff, "Xcore", iScaling );
	hXcore_diff->Scale( nDiff );
	hXcore_sims->Scale( nSims );
	
	cSCore->cd( 1 );
	hXcore_sims->Draw();
	hXcore_diff->Draw( "same" );
	plotLegend( hXcore_sims, hXcore_diff, 0.13 );
	
	cOCore->cd( 1 );
	hXcore_on->Draw();
	hXcore_off->Draw( "same" );
	
	// Ycore
	//
	
	TH1D* hYcore_sims = ( TH1D* )fDir->Get( "hYcore_SIMS" );
	setHistogramAtt( hYcore_sims, 2, 3, 1, 20, 8 );
	hYcore_sims->SetMaximum( hYcore_sims->GetMaximum() * 1.5 );
	
	TH1D* hYcore_on = ( TH1D* )fDir->Get( "hYcore_ON" );
	setHistogramAtt( hYcore_on, 3, 3, 2, 8 );
	
	TH1D* hYcore_off = ( TH1D* )fDir->Get( "hYcore_OFF" );
	setHistogramAtt( hYcore_off, 4, 3, 2, 8 );
	
	TH1D* hYcore_diff = ( TH1D* )fDir->Get( "hYcore_DIFF" );
	setHistogramAtt( hYcore_diff, 1, 3, 1, 21, 8 );
	
	hYcore_sims->SetAxisRange( -250., 250. );
	hYcore_on->SetAxisRange( -250., 250. );
	
	getScaling( fDir, nSims, nDiff, "Ycore", iScaling );
	hYcore_diff->Scale( nDiff );
	hYcore_sims->Scale( nSims );
	
	cSCore->cd( 2 );
	hYcore_sims->Draw();
	hYcore_diff->Draw( "same" );
	plotLegend( hYcore_sims, hYcore_diff, 0.13 );
	
	cOCore->cd( 2 );
	hYcore_on->Draw();
	hYcore_off->Draw( "same" );
	
	// XY plot
	
	cOCore->cd( 3 );
	TH2D* hXYcore_on = ( TH2D* )fDir->Get( "hXYcore_ON" );
	setHistogramAtt( hXYcore_on, 1. );
	hXYcore_on->SetXTitle( "core position X (ON) [m]" );
	hXYcore_on->SetYTitle( "core position Y (ON) [m]" );
	
	hXYcore_on->Draw( "colz" );
	
	cOCore->cd( 4 );
	TH2D* hXYcore_off = ( TH2D* )fDir->Get( "hXYcore_OFF" );
	setHistogramAtt( hXYcore_off, 1. );
	hXYcore_off->SetXTitle( "core position X (OFF) [m]" );
	hXYcore_off->SetYTitle( "core position Y (OFF) [m]" );
	
	hXYcore_off->Draw( "colz" );
	
	cSCore->cd( 3 );
	TH2D* hXYcore_diff = ( TH2D* )fDir->Get( "hXYcore_DIFF" );
	setHistogramAtt( hXYcore_diff, 1. );
	hXYcore_diff->SetXTitle( "core position X (ON-OFF) [m]" );
	hXYcore_diff->SetYTitle( "core position Y (ON-OFF) [m]" );
	
	hXYcore_diff->Draw( "colz" );
	
	cSCore->cd( 4 );
	TH2D* hXYcore_sims = ( TH2D* )fDir->Get( "hXYcore_SIMS" );
	setHistogramAtt( hXYcore_sims, 1. );
	
	hXYcore_sims->SetXTitle( "core position X (SIMS) [m]" );
	hXYcore_sims->SetYTitle( "core position Y (SIMS) [m]" );
	hXYcore_sims->Draw( "colz" );
}

void centroids( char* ifile = "stereo_compare.root", int fNTel = 4 )
{
	gStyle->SetPadGridX( 0 );
	gStyle->SetPadGridY( 0 );
	
	TDirectory* fDir = openFile( ifile );
	
	TCanvas* cOCentro = new TCanvas( "cOCentro", "on/off (centroids plots)", 100, 10, 600, fNTel * 300 );
	cOCentro->SetGridx( 0 );
	cOCentro->SetGridy( 0 );
	cOCentro->Divide( 2, fNTel );
	
	TCanvas* cSCentro = new TCanvas( "cSCentro", "sims/diff (centroids plots)", 10, 10, 600, fNTel * 300 );
	cSCentro->SetGridx( 0 );
	cSCentro->SetGridy( 0 );
	cSCentro->Divide( 2, fNTel );
	
	int iC = 1;
	
	TH2D* hCenXY_sims[200];
	TH2D* hCenXY_diff[200];
	
	TH2D* hCenXY_off[200];
	TH2D* hCenXY_on[200];
	
	char hname[200];
	for( int i = 0; i < fNTel; i++ )
	{
		sprintf( hname, "hcen_xy%d_SIMS", i + 1 );
		hCenXY_sims[i] = ( TH2D* )fDir->Get( hname );
		setHistogramAtt( hCenXY_sims[i], -999. );
		setAxisTitles( hCenXY_sims[i], "sims", i + 1 );
		
		sprintf( hname, "hcen_xy%d_DIFF", i + 1 );
		hCenXY_diff[i] = ( TH2D* )fDir->Get( hname );
		setHistogramAtt( hCenXY_diff[i], 0.01 );
		setAxisTitles( hCenXY_diff[i], "on-off", i + 1 );
		
		sprintf( hname, "hcen_xy%d_ON", i + 1 );
		hCenXY_on[i] = ( TH2D* )fDir->Get( hname );
		setHistogramAtt( hCenXY_on[i], -999. );
		setAxisTitles( hCenXY_on[i], "on", i + 1 );
		
		sprintf( hname, "hcen_xy%d_OFF", i + 1 );
		hCenXY_off[i] = ( TH2D* )fDir->Get( hname );
		setHistogramAtt( hCenXY_off[i], -999. );
		setAxisTitles( hCenXY_off[i], "off", i + 1 );
		
		cSCentro->cd( iC );
		hCenXY_sims[i]->Draw( "contz" );
		
		cOCentro->cd( iC );
		hCenXY_on[i]->Draw( "contz" );
		
		iC++;
		cSCentro->cd( iC );
		hCenXY_diff[i]->Draw( "contz" );
		
		cOCentro->cd( iC );
		hCenXY_off[i]->Draw( "contz" );
		
		iC++;
	}
}

/*

  plot distance from telescope to shower core

*/

void distance_plots( char* ifile = "stereo_compare.root", int fNTel = 4, bool bPoster = false )
{
	gStyle->SetPadGridX( 0 );
	gStyle->SetPadGridY( 0 );
	
	TDirectory* fDir = openFile( ifile );
	
	TCanvas* cODist = new TCanvas( "cODist", "on/off (distance plots)", 100, 10, 900, fNTel * 150 );
	cODist->SetGridx( 0 );
	cODist->SetGridy( 0 );
	cODist->Divide( 3, fNTel );
	
	TCanvas* cSDist = new TCanvas( "cSDist", "sims/diff (distance plots)", 10, 10, 900, fNTel * 150 );
	cSDist->SetGridx( 0 );
	cSDist->SetGridy( 0 );
	cSDist->Divide( 4, fNTel );
	
	char hname[200];
	char htitle[200];
	
	TCanvas* cdistsim[100];
	for( int i = 0; i < 100; i++ )
	{
		cdistsim[i] = 0;
	}
	if( fNTel > 100 )
	{
		cout << "too many telescopes ..." << endl;
		return;
	}
	if( bPoster )
	{
		for( int i = 0; i < fNTel; i++ )
		{
			sprintf( hname, "cdistsim_%d", i );
			sprintf( htitle, "distance plot (Telescope %d)", i + 1 );
			cdistsim[i] = new TCanvas( hname, htitle, 10 + i * 200, 10 + i * 200, 400, 400 );
			cdistsim[i]->SetGridx( 0 );
			cdistsim[i]->SetGridy( 0 );
		}
	}
	
	// get histogram scaling
	double s_sims = 1.;
	double s_diff = 1.;
	
	int iCsi = 1;
	int iCoo = 1;
	
	TH1D* hR_sims[200];
	TH1D* hR_diff[200];
	TH1D* hR_on[200];
	TH1D* hR_off[200];
	
	TH2D* hdistR_sims[200];
	TH2D* hdistR_diff[200];
	TH2D* hdistR_on[200];
	TH2D* hdistR_off[200];
	
	TH1D* hrel = 0;
	for( int i = 0; i < fNTel; i++ )
	{
		// R
		sprintf( hname, "hR%d_SIMS", i + 1 );
		hR_sims[i] = ( TH1D* )fDir->Get( hname );
		if( bPoster )
		{
			setHistogramAtt( hR_sims[i], 2, 3, 2, 20, 1 );
		}
		else
		{
			setHistogramAtt( hR_sims[i], 2, 3, 1, 20, 1 );
		}
		hR_sims[i]->SetMaximum( hR_sims[i]->GetMaximum() * 1.3 );
		hR_sims[i]->SetYTitle( "number of shower [a.u.]" );
		
		sprintf( hname, "hR%d_DIFF", i + 1 );
		hR_diff[i] = ( TH1D* )fDir->Get( hname );
		if( bPoster )
		{
			setHistogramAtt( hR_diff[i], 1, 3, 2, 21, 1 );
		}
		else
		{
			setHistogramAtt( hR_diff[i], 1, 3, 1, 21, 1 );
		}
		
		sprintf( hname, "hR%d_ON", i + 1 );
		hR_on[i] = ( TH1D* )fDir->Get( hname );
		setHistogramAtt( hR_on[i], 3, 3, 2 );
		
		sprintf( hname, "hR%d_OFF", i + 1 );
		hR_off[i] = ( TH1D* )fDir->Get( hname );
		setHistogramAtt( hR_off[i], 4, 3, 2 );
		
		sprintf( hname, "R%d", i + 1 );
		getScaling( fDir, s_sims, s_diff, hname, 1 );
		if( hR_sims[i]->GetEntries() > 0 )
		{
			hR_sims[i]->Scale( s_sims );
		}
		if( hR_diff[i]->GetEntries() > 0 )
		{
			hR_diff[i]->Scale( s_diff );
		}
		
		cSDist->cd( iCsi );
		hR_sims[i]->Draw( "e" );
		hR_diff[i]->Draw( "e same" );
		
		cODist->cd( iCoo );
		hR_on[i]->Draw( "e" );
		hR_off[i]->Draw( "e same" );
		
		if( bPoster )
		{
			cdistsim[i]->cd();
			
			hR_sims[i]->Draw( "e" );
			hR_diff[i]->Draw( "e same" );
			plotLegend( hR_sims[i], hR_diff[i] );
		}
		
		iCsi++;
		iCoo++;
		
		// relative plots
		if( hR_sims[i] && hR_diff[i] )
		{
			sprintf( hname, "hR_RE_%d", i );
			hrel = ( TH1D* )hR_sims[i]->Clone( hname );
			hrel->Divide( hR_diff[i] );
			hrel->SetYTitle( "sims/data" );
			hrel->SetMinimum( 0.3 );
			hrel->SetMaximum( 3.0 );
			setHistogramAtt( hrel, 1, 1, 1, 21, 1 );
			cSDist->cd( iCsi );
			hrel->Draw( "e" );
			TLine* iLine = new TLine( hrel->GetXaxis()->GetXmin(), 1., hrel->GetXaxis()->GetXmax(), 1. );
			iLine->SetLineStyle( 2 );
			iLine->Draw();
		}
		iCsi++;
		
		// distR
		//
		sprintf( hname, "hdistR%d_SIMS", i + 1 );
		hdistR_sims[i] = ( TH2D* )fDir->Get( hname );
		setHistogramAtt( hdistR_sims[i], -999. );
		setAxisTitles( hdistR_sims[i], "sims", i + 1 );
		hdistR_sims[i]->SetAxisRange( 0., 1.5, "Y" );
		
		sprintf( hname, "hdistR%d_DIFF", i + 1 );
		hdistR_diff[i] = ( TH2D* )fDir->Get( hname );
		setHistogramAtt( hdistR_diff[i], 0.001 );
		setAxisTitles( hdistR_diff[i], "on-off", i + 1 );
		hdistR_diff[i]->SetAxisRange( 0., 1.5, "Y" );
		
		sprintf( hname, "hdistR%d_ON", i + 1 );
		hdistR_on[i] = ( TH2D* )fDir->Get( hname );
		setHistogramAtt( hdistR_on[i], -999. );
		setAxisTitles( hdistR_on[i], "on", i + 1 );
		hdistR_on[i]->SetAxisRange( 0., 1.5, "Y" );
		
		sprintf( hname, "hdistR%d_OFF", i + 1 );
		hdistR_off[i] = ( TH2D* )fDir->Get( hname );
		setHistogramAtt( hdistR_off[i], -999. );
		setAxisTitles( hdistR_off[i], "off", i + 1 );
		hdistR_off[i]->SetAxisRange( 0., 1.5, "Y" );
		
		// draw everything
		cSDist->cd( iCsi );
		hdistR_sims[i]->Draw( "contz" );
		
		cODist->cd( iCoo );
		hdistR_on[i]->Draw( "contz" );
		
		iCsi++;
		iCoo++;
		
		cSDist->cd( iCsi );
		hdistR_diff[i]->Draw( "contz" );
		
		cODist->cd( iCoo );
		hdistR_off[i]->Draw( "contz" );
		
		iCsi++;
		iCoo++;
	}
	
}

/*
 * compare single telescope (image) parameters
 *
 * T1 = 1, T2 = 2
 *
 * plot is SIMSDIFF, ONOFF, REL
 *
 */
void single_telescope( int telid = 1, char* ifile = "stereo_compare.root", string iPlot = "SIMSDIFF", bool iOneCanvas = true,
					   bool bPoster = false, int iScalingMethod = 1, int i_rebin = 2, bool plotKS = false )
{

	if( iPlot != "SIMSDIFF" && iPlot != "ONOFF" && iPlot != "REL" )
	{
		cout << "error: unknown plotting mode (allowed are SIMSDIFF, ONOFF, REL)" << endl;
		return;
	}
	// open file
	TDirectory* fDir = openFile( ifile );
	
	double KSProb = 0;
	double KSSig = 0;
	char text[1000];
	// scaling factor
	double s_sims = 1.;
	double s_diff = 1.;
	char htitle[600];
	sprintf( htitle, "width_%d", telid );
	getScaling( fDir, s_sims, s_diff, htitle, iScalingMethod );
	
	//////////////////////////////////////
	// histogram names to be plotted
	vector< string > hname;
	vector< int >    f_rebin;
	vector< int >   f_logy;
	vector< double > f_x_min;
	vector< double > f_x_max;
	hname.push_back( "width" );
	f_rebin.push_back( i_rebin );
	f_logy.push_back( 0 );
	f_x_min.push_back( 0. );
	f_x_max.push_back( 0.25 );
	hname.push_back( "length" );
	f_rebin.push_back( i_rebin );
	f_logy.push_back( 0 );
	f_x_min.push_back( 0. );
	f_x_max.push_back( 0.50 );
	hname.push_back( "dist" );
	f_rebin.push_back( i_rebin );
	f_logy.push_back( 0 );
	f_x_min.push_back( 0. );
	f_x_max.push_back( 2.10 );
	hname.push_back( "size" );
	f_rebin.push_back( i_rebin );
	f_logy.push_back( 1 );
	f_x_min.push_back( 2. );
	f_x_max.push_back( 6.00 );
	hname.push_back( "size2" );
	f_rebin.push_back( i_rebin );
	f_logy.push_back( 1 );
	f_x_min.push_back( 2. );
	f_x_max.push_back( 6.00 );
	hname.push_back( "nlowgain" );
	f_rebin.push_back( 1 );
	f_logy.push_back( 1 );
	f_x_min.push_back( 0. );
	f_x_max.push_back( 40. );
	hname.push_back( "los" );
	f_rebin.push_back( i_rebin );
	f_logy.push_back( 0 );
	f_x_min.push_back( 0. );
	f_x_max.push_back( 40. );
	hname.push_back( "asym" );
	f_rebin.push_back( i_rebin );
	f_logy.push_back( 0 );
	f_x_min.push_back( -2.0 );
	f_x_max.push_back( 2.0 );
	hname.push_back( "cen_x" );
	f_rebin.push_back( i_rebin );
	f_logy.push_back( 0 );
	f_x_min.push_back( -2.0 );
	f_x_max.push_back( 2.0 );
	hname.push_back( "cen_y" );
	f_rebin.push_back( i_rebin );
	f_logy.push_back( 0 );
	f_x_min.push_back( -2.0 );
	f_x_max.push_back( 2.0 );
	hname.push_back( "ntubes" );
	f_rebin.push_back( 1 );
	f_logy.push_back( 1 );
	f_x_min.push_back( 0. );
	f_x_max.push_back( 40. );
	hname.push_back( "mscwt" );
	f_rebin.push_back( i_rebin );
	f_logy.push_back( 0 );
	f_x_min.push_back( 0.5 );
	f_x_max.push_back( 1.5 );
	hname.push_back( "msclt" );
	f_rebin.push_back( i_rebin );
	f_logy.push_back( 0 );
	f_x_min.push_back( 0.5 );
	f_x_max.push_back( 1.5 );
	hname.push_back( "loss" );
	f_rebin.push_back( i_rebin );
	f_logy.push_back( 1 );
	f_x_min.push_back( 0. );
	f_x_max.push_back( 0.25 );
	hname.push_back( "tgrad_x" );
	f_rebin.push_back( i_rebin );
	f_logy.push_back( 0 );
	f_x_min.push_back( -7.5 );
	f_x_max.push_back( 7.5 );
	
	// loop over all histograms and plot them
	char hn[600];
	char cn[600];
	char ct[600];
	
	TH1D* hsims = 0;
	TH1D* hdiff = 0;
	TH1D* hon = 0;
	TH1D* hoff = 0;
	TH1D* hrel = 0;
	
	TCanvas* hc = 0;
	// canvas for all in one
	if( iOneCanvas )
	{
		sprintf( cn, "image parameter comparision (telescope %d, file %s, %s)", telid, ifile, iPlot.c_str() );
		sprintf( ct, "cimage_%d_%s_%s", telid, iPlot.c_str(), ifile );
		hc = new TCanvas( ct, cn, 10, 10, 1300, 800 );
		hc->SetGridx( 0 );
		hc->SetGridy( 0 );
		hc->Divide( 5, 3 );
	}
	TLegend* iL = 0;
	
	/////////////////////////////////////////////////
	// loop over all histograms and plot them
	for( unsigned int j = 0; j < hname.size(); j++ )
	{
		sprintf( hn, "h%s_%d_SIMS", hname[j].c_str(), telid );
		hsims = ( TH1D* )fDir->Get( hn );
		if( !hsims )
		{
			cout << "sims histogram not found " << hn << endl;
			continue;
		}
		sprintf( hn, "h%s_%d_DIFF", hname[j].c_str(), telid );
		hdiff = ( TH1D* )fDir->Get( hn );
		if( !hdiff )
		{
			cout << "diff histogram not found " << hn << endl;
			continue;
		}
		sprintf( hn, "h%s_%d_ON", hname[j].c_str(), telid );
		hon = ( TH1D* )fDir->Get( hn );
		if( !hon )
		{
			cout << "on histogram not found " << hn << endl;
			continue;
		}
		sprintf( hn, "h%s_%d_OFF", hname[j].c_str(), telid );
		hoff = ( TH1D* )fDir->Get( hn );
		if( !hoff )
		{
			cout << "off histogram not found " << hn << endl;
			continue;
		}
		sprintf( htitle, "%s_%d", hname[j].c_str(), telid );
		getScaling( fDir, s_sims, s_diff, htitle, iScalingMethod );
		// normalize sims histograms to data histograms
		hsims->Scale( s_sims );
		hdiff->Scale( s_diff );
		// rebin histograms
		hsims->Rebin( f_rebin[j] );
		hdiff->Rebin( f_rebin[j] );
		hon->Rebin( f_rebin[j] );
		hoff->Rebin( f_rebin[j] );
		// relative histograms
		if( hsims && hdiff )
		{
			sprintf( hn, "h%s_%d_RE", hname[j].c_str(), telid );
			hrel = ( TH1D* )hsims->Clone( hn );
			hrel->Divide( hdiff );
		}
		
		if( iOneCanvas )
		{
			TPad* g = ( TPad* )hc->cd( j + 1 );
			g->SetGridx( 0 );
			g->SetGridy( 0 );
			if( iPlot != "REL" )
			{
				g->SetLogy( f_logy[j] );
			}
		}
		else
		{
			sprintf( cn, "c%d%s%s", telid, iPlot.c_str(), hname[j].c_str() );
			sprintf( ct, "%s (telescope %d, %s)", hname[j].c_str(), telid, iPlot.c_str() );
			int xmax = 600;
			if( bPoster )
			{
				xmax = 400;
			}
			hc = new TCanvas( cn, ct, 10, 100, xmax, 400 );
			hc->SetGridx( 0 );
			hc->SetGridy( 0 );
			hc->SetLogy( f_logy[j] );
			hc->cd();
		}
		iL = new TLegend( 0.58 , 0.68, 0.85, 0.85 );
		
		setHistogramAtt( hsims, 2, 0.5, 1, 20, 1 );
		if( bPoster )
		{
			setHistogramAtt( hsims, 2, 3, 2, 20, 1 );
		}
		setHistogramAtt( hdiff, 1, 0.5, 1, 21, 1 );
		if( bPoster )
		{
			setHistogramAtt( hdiff, 1, 3, 2, 21, 1 );
		}
		setHistogramAtt( hon, 3, 1, 1, 20, 1 );
		if( bPoster )
		{
			setHistogramAtt( hon, 2, 3, 2, 20, 1 );
		}
		setHistogramAtt( hoff, 4, 1, 1, 21, 1 );
		if( bPoster )
		{
			setHistogramAtt( hoff, 1, 3, 2, 21, 1 );
		}
		setHistogramAtt( hrel, 9, 1, 1, 20, 1 );
		if( bPoster )
		{
			setHistogramAtt( hrel, 1, 3, 2, 21, 1 );
		}
		
		hdiff->SetYTitle( "number of shower [a.u.]" );
		hdiff->SetMaximum( hdiff->GetMaximum() * 1.5 );
		hrel->SetYTitle( "sims/data" );
		hrel->SetMinimum( 0.03 );
		hrel->SetMaximum( 3.0 );
		
		if( hdiff->GetXaxis()->GetXmin() > f_x_min[j] )
		{
			f_x_min[j] = hdiff->GetXaxis()->GetXmin();
		}
		if( hdiff->GetXaxis()->GetXmax() < f_x_max[j] )
		{
			f_x_max[j] = hdiff->GetXaxis()->GetXmax();
		}
		hdiff->SetAxisRange( f_x_min[j], f_x_max[j] );
		hsims->SetAxisRange( f_x_min[j], f_x_max[j] );
		hon->SetAxisRange( f_x_min[j], f_x_max[j] );
		hoff->SetAxisRange( f_x_min[j], f_x_max[j] );
		hrel->SetAxisRange( f_x_min[j], f_x_max[j] );
		
		////////////////////////////////////////////////
		// difference plots
		if( iPlot == "SIMSDIFF" )
		{
			hdiff->Draw( "cle" );
			hsims->Draw( "cle same" );
			sprintf( cn, "telescope %d", telid );
			iL->AddEntry( hdiff, cn, "pl" );
			sprintf( cn, "simulations" );
			iL->AddEntry( hsims, cn, "pl" );
			
			if( !gPad->GetLogy() )
			{
				TLine* iL0 = new TLine( hdiff->GetXaxis()->GetXmin(), 0., hdiff->GetXaxis()->GetXmax(), 0. );
				iL0->SetLineStyle( 2 );
				iL0->Draw();
			}
		}
		////////////////////////////////////////////////
		// on/off plots
		else if( iPlot == "ONOFF" )
		{
			hon->Draw( "cle" );
			hoff->Draw( "cle same" );
			sprintf( cn, "on telescope %d", telid );
			iL->AddEntry( hon, cn, "pl" );
			sprintf( cn, "off telescope %d", telid );
			iL->AddEntry( hoff, cn, "pl" );
		}
		////////////////////////////////////////////////
		// relative plots
		else if( iPlot == "REL" )
		{
			hrel->Draw( "cle" );
		}
		if( !bPoster && iPlot != "REL" )
		{
			iL->Draw();
		}
		
		// line for mscwt and msclt histograms
		if( iPlot != "REL" )
		{
			if( hname[j] == "mscwt" || hname[j] == "msclt" )
			{
				TLine* iLine = new TLine( 1., hdiff->GetMinimum(), 1., hdiff->GetMaximum() );
				iLine->SetLineStyle( 2 );
				iLine->Draw();
			}
		}
		// line at 1 for relative plots
		else
		{
			TLine* iLine = new TLine( f_x_min[j], 1., f_x_max[j], 1. );
			iLine->SetLineStyle( 2 );
			iLine->Draw();
		}
		if( iPlot == "SIMSDIFF" || iPlot == "REL" )
		{
			// calculate probabilities of agreement
			KSProb = hsims->KolmogorovTest( hdiff );
			KSSig = TMath::ErfInverse( 1 - KSProb ) * TMath::Sqrt( 2 );
			if( KSProb != 0 )
			{
				sprintf( text, "%s | KS P = %1.2e (%1.1f #sigma)", hname[j].c_str(), KSProb, KSSig );
			}
			else
			{
				sprintf( text, "%s | KS P = %1.2e (#infty #sigma)", hname[j].c_str(), KSProb );
			}
			if( plotKS )
			{
				cout << text << endl;
			}
			
			TLatex* iT = new TLatex();
			iT->SetText( 0.11, 0.92, text );
			iT->SetNDC();
			iT->Draw();
		}
	}
}

void fitTheta2( TH1D* h, int icolor = 1, int imarker = 20 )
{
	TCanvas* cx = new TCanvas( "cx", "", 10, 10, 400, 400 );
	cx->SetGridx( 0 );
	cx->SetGridy( 0 );
	
	h->SetAxisRange( 0., 0.1 );
	
	TF1* fTheta2 = new TF1( "fTheta2", "[0]*([1]/2./[2]/[2]*TMath::Exp(-x/2./[2]/[2])+(1-[1])/2./[3]/[3]*TMath::Exp(-x/2./[3]/[3]))", 0., 0.1 );
	fTheta2->SetParameter( 0, 5. );
	fTheta2->SetParameter( 1, 0.5 );
	fTheta2->SetParLimits( 1, 0., 1. );
	fTheta2->SetParameter( 2, 0.03 );
	fTheta2->SetParLimits( 2, 0., 1. );
	fTheta2->SetParameter( 3, 0.09 );
	fTheta2->SetParLimits( 3, 0., 1. );
	fTheta2->SetLineStyle( 2 );
	fTheta2->SetLineColor( 3 );
	fTheta2->SetLineWidth( 4 );
	
	h->Fit( "fTheta2", "R" );
	cout << "CHI2 " << fTheta2->GetChisquare() << endl;
	
	h->SetMarkerStyle( imarker );
	h->SetMarkerColor( icolor );
	h->SetMarkerSize( 3 );
	
	h->Draw();
}

void plot_msc( char* ffile = "stereo_compare.root", char* offFile = 0, char* helium = 0, char* proton = 0, double xmin = -1.5, double xmax = 4., string ivar = "MSCW" )
{
	gStyle->SetPadGridX( 0 );
	gStyle->SetPadGridY( 0 );
	
	char hname[200];
	
	TDirectory* fDir = openFile( ffile );
	
	TCanvas* cMSCsim = 0;
	sprintf( hname, "c%sSim", ivar.c_str() );
	cMSCsim = new TCanvas( hname, hname, 450, 510, 400, 400 );
	cMSCsim->SetGridx( 0 );
	cMSCsim->SetGridy( 0 );
	
	sprintf( hname, "h%s_SIMS", ivar.c_str() );
	TH1D* hMSC_sims = ( TH1D* )fDir->Get( hname );
	setHistogramAtt( hMSC_sims, 2, 3, 2, 24, 1 );
	hMSC_sims->SetYTitle( "number of showers [a.u.]" );
	
	sprintf( hname, "h%s_DIFF", ivar.c_str() );
	TH1D* hMSC_diff = ( TH1D* )fDir->Get( hname );
	setHistogramAtt( hMSC_diff, 1, 3, 2, 21, 1 );
	
	sprintf( hname, "h%s_OFF", ivar.c_str() );
	TH1D* hMSC_off = ( TH1D* )fDir->Get( hname );
	setHistogramAtt( hMSC_off, 4, 3, 2, 21, 1 );
	hMSC_off->Rebin( 2 );
	
	// get the scaling between simulations and data
	double s_sims = 1.;
	double s_diff = 1.;
	getScaling( fDir, s_sims, s_diff, ivar.c_str(), true );
	hMSC_sims->SetAxisRange( xmin, xmax );
	if( hMSC_sims->GetEntries() > 0 )
	{
		hMSC_sims->Scale( s_sims );
	}
	if( hMSC_diff->GetEntries() > 0 )
	{
		hMSC_diff->Scale( s_diff * 1.15 );
	}
	if( hMSC_off->GetMaximum() > 0 )
	{
		hMSC_off->Scale( hMSC_diff->GetMaximum() / hMSC_off->GetMaximum() / 2. );
	}
	
	cMSCsim->cd();
	hMSC_sims->SetMaximum( hMSC_sims->GetMaximum() * 1.3 );
	if( ivar == "MSCW" )
	{
		hMSC_sims->SetXTitle( "mean scaled width" );
	}
	else
	{
		hMSC_sims->SetXTitle( "mean scaled length" );
	}
	hMSC_sims->Draw( "e" );
	TBox* iB = new TBox( -1.5, 0., 0.5,  hMSC_sims->GetMaximum() );
	iB->SetFillColor( 5 );
	iB->SetFillStyle( 1001 );
	iB->SetFillColor( 18 );
	iB->Draw();
	hMSC_sims->Draw( "axis same" );
	hMSC_sims->Draw( "e same" );
	hMSC_diff->Draw( "e same" );
	//   hMSC_off->Draw( "e same" );
	
	// get off histogram from anasum output file
	if( offFile )
	{
		TFile* fOff = new TFile( offFile );
		if( !fOff->IsZombie() )
		{
			fOff->cd( "total/stereo/stereoParameterHistograms/" );
			if( ivar == "MSCW" )
			{
				sprintf( hname, "hmscw_off" );
			}
			else if( ivar == "MSCL" )
			{
				sprintf( hname, "hmscl_off" );
			}
			TH1D* hMSC_off_ana = ( TH1D* )gDirectory->Get( hname );
			if( !hMSC_off_ana )
			{
				cout << "histogram not found: " << hname << endl;
				return;
			}
			
			hMSC_off_ana->Rebin( 2 );
			hMSC_off_ana->SetLineColor( 4 );
			hMSC_off_ana->SetMarkerColor( 4 );
			hMSC_off_ana->SetLineWidth( 2 );
			if( hMSC_off_ana->GetMaximum() > 0 )
			{
				hMSC_off_ana->Scale( hMSC_diff->GetMaximum() / hMSC_off_ana->GetMaximum() / 2. );
			}
			hMSC_off_ana->Draw( "same" );
		}
	}
	if( helium && proton )
	{
		int nbins = 50;
		TH1D* hHelium = 0;
		TH1D* hProton = 0;
		TH1D* hCR = new TH1D( "hCR", "", nbins, -5., 15. );
		hCR->SetLineWidth( 2 );
		hCR->SetLineColor( 3 );
		hCR->SetMarkerColor( 3 );
		hCR->Sumw2();
		hCR->SetStats( 0 );
		TFile* fH = new TFile( helium );
		if( !fH->IsZombie() )
		{
			TTree* t = ( TTree* )gDirectory->Get( "data" );
			hHelium = new TH1D( "hHelium", "", nbins, -5., 15. );
			hHelium->Sumw2();
			sprintf( hname, "%s > -99. &&Chi2>=0&&(Xoff*Xoff+Yoff*Yoff)<0.1", ivar.c_str() );
			t->Project( hHelium->GetName(), ivar.c_str(), hname );
		}
		hHelium->SetStats( 0 );
		TFile* fP = new TFile( proton );
		if( !fP->IsZombie() )
		{
			TTree* t = ( TTree* )gDirectory->Get( "data" );
			hProton = new TH1D( "hProton", "", nbins, -5., 15. );
			hProton->Sumw2();
			sprintf( hname, "%s > -99. &&Chi2>=0&&(Xoff*Xoff+Yoff*Yoff)<0.1", ivar.c_str() );
			t->Project( hProton->GetName(), ivar.c_str(), hname );
		}
		// now add everything up assuming 70% proton and 30% helium
		// N_HE = 2.55e7, N_p = 6.6e7
		double iHEScale = 6.6e7 / 2.55e7 * 0.3;
		cout << "scaling helium histogram by " << iHEScale << endl;
		hCR->Add( hHelium, hProton, iHEScale, 1. );
		
		if( hCR->GetMaximum() > 0. )
		{
			hCR->Scale( hMSC_diff->GetMaximum() / hCR->GetMaximum() / 2. * 1.05 );
		}
		hCR->Draw( "same" );
	}
	TLegend* iLegend = new TLegend( 0.4, 0.60, 0.85, 0.85 );
	iLegend->AddEntry( hMSC_sims, "simulations (#gamma-rays)", "pl" );
	iLegend->AddEntry( hMSC_diff, "On-Off (Crab data)", "pl" );
	//  iLegend->AddEntry( hCR, "simulations (Cosmic-rays)", "l" );
	//  iLegend->AddEntry( hMSC_off_ana, "data (Cosmic-rays)", "l" );
	iLegend->Draw();
}


/*

    plot mean width plots (energy dependent)
*/
void mwr_plots( char* ffile = "stereo_compare.root" )
{
	gStyle->SetPadGridX( 0 );
	gStyle->SetPadGridY( 0 );
	gStyle->SetPalette( 1 );
	
	TDirectory* fDir = openFile( ffile );
	
	plot_energyDependentDistributions( fDir, "MWR", 1, 0., 2.5 );
	plot_energyDependentDistributions( fDir, "MLR", 1, 0., 2.5 );
	
	return;
}
