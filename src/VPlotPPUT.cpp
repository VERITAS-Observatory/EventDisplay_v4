/*  \file VPlotPPUT.cpp
    \brief plot performance power per unit time for CTA site search



*/

#include "VPlotPPUT.h"

VPlotPPUT::VPlotPPUT()
{
	setDebug( false );
	
	setPPUTRange();
	setPPUTEnergyRange();
	setPlotAltitudeRange();
	setPlotBFieldRange();
	setPrintingOptions( "" );
}

void VPlotPPUT::printCanvas( TCanvas* c, string iPrint )
{
	if( !c )
	{
		return;
	}
	if( fPrintingOptions.size() == 0 )
	{
		return;
	}
	
	string iPName = fPrintingOptions + iPrint;
	
	c->Print( iPName.c_str() );
}

void VPlotPPUT::getMergedFigureOfMerits( VSiteData* iSite, float* fom, float* fom_error, string iDirectionString )
{
	TGraphAsymmErrors* iGraphSensitivity = iSite->getCombinedSensitivityGraph( true, iDirectionString );
	if( !iGraphSensitivity )
	{
		return;
	}
	
	// calculate figure of merrit:
	VPlotWPPhysSensitivity b;
	b.setCTARequirements( iSite->fSiteRequirementID );
	cout << iSite->fSiteName << "\t" << iDirectionString << endl;
	b.printSensitivityFigureOfMerit( iGraphSensitivity, f_pput_Energy_linTeV_min, f_pput_Energy_linTeV_max, iSite->fSiteName );
	
	*fom = b.getSensitivityFOM();
	*fom_error = b.getSensitivityFOM_error();
}

void VPlotPPUT::setPPUTRange( float pput_min, float pput_max )
{
	f_pput_min = pput_min;
	f_pput_max = pput_max;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void VPlotPPUT::plot( unsigned int iSiteRequirementID, string iDataList, bool bPlotPredictions )
{
	// linear fits are valid in these ranges
	double alt_min = 1300.;
	double alt_max = 4000.;
	double b_min = 0.;
	double b_max = 40.;
	
	vector< VSiteData* > fSite;
	
	unsigned int z_site = 0;
	for( ;; )
	{
		fSite.push_back( new VSiteData() );
		if( !fSite.back()->addDataSet( iDataList, z_site ) )
		{
			fSite.pop_back();
			break;
		}
		fSite.back()->fSiteRequirementID = iSiteRequirementID;
		z_site++;
	}
	if( iSiteRequirementID < 3 && f_pput_Energy_linTeV_min < 0. )
	{
		setPPUTEnergyRange( 0.03, 100. );
	}
	else if( f_pput_Energy_linTeV_min < 0. )
	{
		setPPUTEnergyRange( 0.03, 20. );
	}
	
	cout << "Total number of sites: " << fSite.size() << endl;
	for( unsigned int i = 0; i < fSite.size(); i++ )
	{
		fSite[i]->print();
	}
	
	// plotting preparation
	TCanvas* cPPUT_height = new TCanvas( "cEH", "PPUT vs height", 10, 10, 600, 500 );
	cPPUT_height->SetGridx( 0 );
	cPPUT_height->SetGridy( 0 );
	TH1D* hH = new TH1D( "hH", "", 100, f_plot_alt_min, f_plot_alt_max );
	hH->SetStats( 0 );
	hH->SetXTitle( "altitude [m]" );
	hH->SetYTitle( "PPUT" );
	hH->SetMinimum( f_pput_min );
	hH->SetMaximum( f_pput_max );
	hH->Draw();
	
	if( f_pput_min < 1. )
	{
		TLine* iHH = new TLine( hH->GetXaxis()->GetXmin(), 1., hH->GetXaxis()->GetXmax(), 1. );
		iHH->SetLineStyle( 2 );
		iHH->Draw();
	}
	
	TCanvas* cPPUT_B = new TCanvas( "cEB", "PPUT vs bfield", 510, 10, 600, 500 );
	cPPUT_B->SetGridx( 0 );
	cPPUT_B->SetGridy( 0 );
	
	TH1D* hB = new TH1D( "hB", "", 100, f_plot_b_min, f_plot_b_max );
	hB->SetStats( 0 );
	hB->SetXTitle( "B_{tr} [#muT]" );
	hB->SetYTitle( "PPUT" );
	hB->SetMinimum( f_pput_min );
	hB->SetMaximum( f_pput_max );
	hB->Draw();
	
	if( f_pput_min < 1. )
	{
		TLine* ihB = new TLine( hB->GetXaxis()->GetXmin(), 1., hB->GetXaxis()->GetXmax(), 1. );
		ihB->SetLineStyle( 2 );
		ihB->Draw();
	}
	
	
	////////////////////////////////////////////////////
	// PPUT analysis
	////////////////////////////////////////////////////
	
	TGraph2DErrors* iSens_vs_height_vs_bfield = new TGraph2DErrors( 1 );
	
	float fom = 0.;
	float fom_error = 0.;
	
	int z = 0;
	
	// loop over all sites
	for( unsigned int i = 0; i < fSite.size(); i++ )
	{
		if( !fSite[i] )
		{
			continue;
		}
		if( fSite[i]->fSiteName == "Fermi" )
		{
			continue;
		}
		////////////////////
		// define graphs
		
		TGraphAsymmErrors* iSens_vs_height = new TGraphAsymmErrors( 1 );
		iSens_vs_height->SetMarkerStyle( 20 + i );
		iSens_vs_height->SetMarkerSize( 1.5 );
		TGraphAsymmErrors* iSens_vs_height_N = new TGraphAsymmErrors( 1 );
		iSens_vs_height_N->SetMarkerStyle( 20 + i );
		iSens_vs_height_N->SetMarkerColor( 2 );
		iSens_vs_height_N->SetMarkerSize( 1.5 );
		iSens_vs_height_N->SetLineColor( 2 );
		TGraphAsymmErrors* iSens_vs_height_S = new TGraphAsymmErrors( 1 );
		iSens_vs_height_S->SetMarkerStyle( 20 + i );
		iSens_vs_height_S->SetMarkerColor( 3 );
		iSens_vs_height_S->SetMarkerSize( 1.5 );
		iSens_vs_height_S->SetLineColor( 3 );
		
		TGraphAsymmErrors* iSens_vs_bfield = new TGraphAsymmErrors( 1 );
		iSens_vs_bfield->SetMarkerStyle( 20 + i );
		iSens_vs_bfield->SetMarkerSize( 1.5 );
		TGraphAsymmErrors* iSens_vs_bfield_N = new TGraphAsymmErrors( 1 );
		iSens_vs_bfield_N->SetMarkerStyle( 20 + i );
		iSens_vs_bfield_N->SetMarkerColor( 2 );
		iSens_vs_bfield_N->SetMarkerSize( 1.5 );
		iSens_vs_bfield_N->SetLineColor( 2 );
		TGraphAsymmErrors* iSens_vs_bfield_S = new TGraphAsymmErrors( 1 );
		iSens_vs_bfield_S->SetMarkerStyle( 20 + i );
		iSens_vs_bfield_S->SetMarkerColor( 3 );
		iSens_vs_bfield_S->SetMarkerSize( 1.5 );
		iSens_vs_bfield_S->SetLineColor( 3 );
		
		///////////////////
		// north+south pointing average
		
		getMergedFigureOfMerits( fSite[i], &fom, &fom_error );
		
		iSens_vs_height->SetPoint( 0, fSite[i]->fSite_asl, fom );
		iSens_vs_height->SetPointError( i, 0., 0., fom_error, fom_error );
		
		iSens_vs_bfield->SetPoint( 0, fSite[i]->fSite_B_dB, fom );
		iSens_vs_bfield->SetPointError( 0, 0., 0., fom_error, fom_error );
		
		///////////////////
		// north pointing
		
		getMergedFigureOfMerits( fSite[i], &fom, &fom_error, "_0deg" );
		
		iSens_vs_height_N->SetPoint( 0, fSite[i]->fSite_asl, fom );
		iSens_vs_height_N->SetPointError( 0, 0., 0., fom_error, fom_error );
		
		iSens_vs_bfield_N->SetPoint( 0, TMath::Abs( fSite[i]->fSite_B_N ), fom );
		iSens_vs_bfield_N->SetPointError( 0, 0., 0., fom_error, fom_error );
		
		if( fSite[i]->fSite_asl > alt_min && fSite[i]->fSite_asl < alt_max )
		{
			iSens_vs_height_vs_bfield->SetPoint( z, fSite[i]->fSite_asl, TMath::Abs( fSite[i]->fSite_B_N ), fom );
			iSens_vs_height_vs_bfield->SetPointError( z, 0., 0., fom_error );
			z++;
		}
		
		///////////////////
		// south pointing
		
		getMergedFigureOfMerits( fSite[i], &fom, &fom_error, "_180deg" );
		
		iSens_vs_height_S->SetPoint( 0, fSite[i]->fSite_asl, fom );
		iSens_vs_height_S->SetPointError( 0, 0., 0., fom_error, fom_error );
		
		iSens_vs_bfield_S->SetPoint( 0, TMath::Abs( fSite[i]->fSite_B_S ), fom );
		iSens_vs_bfield_S->SetPointError( 0, 0., 0., fom_error, fom_error );
		
		if( fSite[i]->fSite_asl > alt_min && fSite[i]->fSite_asl < alt_max )
		{
			iSens_vs_height_vs_bfield->SetPoint( z, fSite[i]->fSite_asl, TMath::Abs( fSite[i]->fSite_B_S ), fom );
			iSens_vs_height_vs_bfield->SetPointError( z, 0., 0., fom_error );
			z++;
		}
		
		cPPUT_height->cd();
		iSens_vs_height->Draw( "p" );
		iSens_vs_height_N->Draw( "p" );
		iSens_vs_height_S->Draw( "p" );
		printCanvas( cPPUT_height, "-pput-vs-height.eps" );
		
		cPPUT_B->cd();
		iSens_vs_bfield_N->Draw( "p" );
		iSens_vs_bfield_S->Draw( "p" );
		printCanvas( cPPUT_B, "-pput-vs-bfield.eps" );
	}
	
	//////////////////////////////
	// fit 2D plane and plot
	TCanvas* cB2D = new TCanvas( "cEB2D", "FOM vs altitude vs bfield", 810, 10, 600, 500 );
	iSens_vs_height_vs_bfield->SetTitle( "" );
	
	cout << "Fitting height (par[1]) vs B-Field (par[2]) histogram." << endl;
	TF2* f2D = new TF2( "f2D", "[0]-[1]*x-[2]*y", alt_min, alt_max, b_min, b_max );
	f2D->SetParameter( 0, 1.7 );
	f2D->SetParameter( 1, 0.000118 );
	f2D->SetParameter( 2, 0.018 );
	iSens_vs_height_vs_bfield->Print();
	iSens_vs_height_vs_bfield->Fit( f2D, 0 );
	cout << "Chi2 = " << f2D->GetChisquare() << ", NDF = " << f2D->GetNDF() << endl;
	
	
	// 2D histogram prediction
	cB2D->cd();
	cB2D->SetRightMargin( 0.15 );
	cB2D->SetLeftMargin( 0.12 );
	//   TH2F *h2DP = new TH2F( "h2DP", "", 1000, alt_min, alt_max, b_min, b_max );
	TH2F* h2DP = ( TH2F* )f2D->CreateHistogram();
	if( h2DP )
	{
		h2DP->SetStats( 0 );
		h2DP->SetTitle( "" );
		h2DP->SetXTitle( "altitude [m]" );
		h2DP->SetYTitle( "B_{tr} [#muT]" );
		h2DP->SetZTitle( "PPUT" );
		h2DP->Draw( "colz" );
		
		for( unsigned int i = 0; i < fSite.size(); i++ )
		{
			TGraph* g_sites = new TGraph( 1 );
			g_sites->SetMarkerStyle( 20 + i );
			g_sites->SetPoint( 1, fSite[i]->fSite_asl, TMath::Abs( fSite[i]->fSite_B_N ) );
			g_sites->SetPoint( 2, fSite[i]->fSite_asl, TMath::Abs( fSite[i]->fSite_B_S ) );
			g_sites->Draw( "p" );
		}
		
	}
	printCanvas( cB2D, "-2D-PPUT.eps" );
	
	// plot predictions into the 1D plots
	if( bPlotPredictions && cPPUT_height && cPPUT_B )
	{
	
		for( unsigned int i = 0; i < fSite.size(); i++ )
		{
			cPPUT_height->cd();
			// fit doesn't work at very low altitude
			if( fSite[i]->fSite_asl < 1300. )
			{
				continue;
			}
			TGraph* g_asl_N = new TGraph( 1 );
			g_asl_N->SetMarkerStyle( 24 + i );
			if( 24 + i == 27 )
			{
				g_asl_N->SetMarkerStyle( 32 );
			}
			g_asl_N->SetMarkerColor( 2 );
			g_asl_N->SetPoint( 1, fSite[i]->fSite_asl, f2D->Eval( fSite[i]->fSite_asl, TMath::Abs( fSite[i]->fSite_B_N ) ) );
			g_asl_N->Draw( "p" );
			
			TGraph* g_asl_S = new TGraph( 1 );
			g_asl_S->SetMarkerStyle( 24 + i );
			if( 24 + i == 27 )
			{
				g_asl_S->SetMarkerStyle( 32 );
			}
			g_asl_S->SetMarkerColor( 3 );
			g_asl_S->SetPoint( 1, fSite[i]->fSite_asl, f2D->Eval( fSite[i]->fSite_asl, TMath::Abs( fSite[i]->fSite_B_S ) ) );
			g_asl_S->Draw( "p" );
			
			cPPUT_B->cd();
			
			TGraph* g_B_N = new TGraph( 1 );
			g_B_N->SetMarkerStyle( 24 + i );
			if( 24 + i == 27 )
			{
				g_B_N->SetMarkerStyle( 32 );
			}
			g_B_N->SetMarkerColor( 2 );
			g_B_N->SetPoint( 1, TMath::Abs( fSite[i]->fSite_B_N ), f2D->Eval( fSite[i]->fSite_asl, TMath::Abs( fSite[i]->fSite_B_N ) ) );
			g_B_N->Draw( "p" );
			
			TGraph* g_B_S = new TGraph( 1 );
			g_B_S->SetMarkerStyle( 24 + i );
			if( 24 + i == 27 )
			{
				g_B_S->SetMarkerStyle( 32 );
			}
			g_B_S->SetMarkerColor( 3 );
			g_B_S->SetPoint( 1, TMath::Abs( fSite[i]->fSite_B_S ), f2D->Eval( fSite[i]->fSite_asl, TMath::Abs( fSite[i]->fSite_B_S ) ) );
			g_B_S->Draw( "p" );
			
		}
		printCanvas( cPPUT_height, "-pput-vs-height-Prediction.eps" );
		printCanvas( cPPUT_B, "-pput-vs-bfield-Prediction.eps" );
	}
	
}
