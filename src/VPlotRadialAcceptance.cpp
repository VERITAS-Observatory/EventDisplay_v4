/*  \class VPlotRadialAcceptance
    \brief plot radial acceptance curves

*/

#include "VPlotRadialAcceptance.h"

VPlotRadialAcceptance::VPlotRadialAcceptance( string iFile, int iAz )
{
	fDebug = false;
	
	setName( "radial acceptance" );
	
	setAxisRange();
	
	fAcceptanceFile = 0;
	fAcceptanceHisto = 0;
	fAcceptanceHistoFit = 0;
	fAcceptanceFunction = 0;
	hPhiDist = 0;
	hPhiDistDeRot = 0;
	
	if( iFile.size() > 0 )
	{
		openAcceptanceFile( iFile, 0, iAz );
	}
}


bool VPlotRadialAcceptance::openAcceptanceFile( string iFile, unsigned int iZeBin, int iAzBin )
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
	if( iAzBin >= 0 )
	{
		sprintf( hname, "az_%d", iAzBin );
		if( !fAcceptanceFile->cd( hname ) )
		{
			cout << "VPlotRadialAcceptance::openAcceptanceFile error, directory for the following az bin not found: " << iAzBin << endl;
			return false;
		}
	}
	cout << "reading acceptance histograms from " << gDirectory->GetName() << endl;
// read acceptance histogram from file
	sprintf( hname, "hAccZe_%d", iZeBin );
	fAcceptanceHisto = ( TH1F* )gDirectory->Get( hname );
	if( !fAcceptanceHisto )
	{
		cout << "VPlotRadialAcceptance::addAcceptanceFile: error finding acceptance histogram" << endl;
		cout << hname << endl;
		return false;
	}
// read AZ dependent acceptance histograms from file and fit functions from file
	fAcceptancePhiHisto.clear();
	fAcceptancePhiFitFunction.clear();
	fAcceptancePhiHistoDeRot.clear();
	fAcceptancePhiFitFunctionDeRot.clear();
	for( int i = 0; i < 16; i++ )
	{
		sprintf( hname, "hAccPhi_%d", i );
		fAcceptancePhiHisto.push_back( ( TH1F* )gDirectory->Get( hname ) );
		if( !fAcceptancePhiHisto.back() )
		{
			cout << "VPlotRadialAcceptance::addAcceptanceFile warning: could not find acceptance histogram " << hname << endl;
		}
		sprintf( hname, "fAccPhi_%d", i );
		fAcceptancePhiFitFunction.push_back( ( TF1* )gDirectory->Get( hname ) );
		if( !fAcceptancePhiFitFunction.back() )
		{
			cout << "VPlotRadialAcceptance::addAcceptanceFile warning: could not find acceptance fit function " << hname << endl;
		}
		sprintf( hname, "hAccPhiDerot_%d", i );
		fAcceptancePhiHistoDeRot.push_back( ( TH1F* )gDirectory->Get( hname ) );
		if( !fAcceptancePhiHistoDeRot.back() )
		{
			cout << "VPlotRadialAcceptance::addAcceptanceFile warning: could not find acceptance histogram " << hname << endl;
		}
		sprintf( hname, "fAccPhiDerot_%d", i );
		fAcceptancePhiFitFunctionDeRot.push_back( ( TF1* )gDirectory->Get( hname ) );
		if( !fAcceptancePhiFitFunctionDeRot.back() )
		{
			cout << "VPlotRadialAcceptance::addAcceptanceFile warning: could not find acceptance fit function " << hname << endl;
		}
	}
// read acceptance fit function from file
	sprintf( hname, "fAccZe_%d", iZeBin );
	fAcceptanceFunction = ( TF1* )gDirectory->Get( hname );
	if( !fAcceptanceFunction )
	{
		cout << "VPlotRadialAcceptance::addAcceptanceFile: error finding acceptance fit function" << endl;
		cout << hname << endl;
		return false;
	}
// read acceptance histogram (fit values) from file
	sprintf( hname, "hAccZe_%dFit", iZeBin );
	fAcceptanceHistoFit = ( TH1F* )gDirectory->Get( hname );
	if( !fAcceptanceHistoFit )
	{
		cout << "VPlotRadialAcceptance::addAcceptanceFile: error finding acceptance histogram (fitted)" << endl;
		cout << hname << endl;
		return false;
	}
// read AZ distributions
	hPhiDist = ( TH1F* )gDirectory->Get( "hPhiDist" );
	if( hPhiDist )
	{
		hPhiDist->Rebin( 5 );
	}
	hPhiDistDeRot = ( TH1F* )gDirectory->Get( "hPhiDistDeRot" );
	if( hPhiDistDeRot )
	{
		hPhiDistDeRot->Rebin( 5 );
	}
	
	return true;
}

/*

    plot acceptance curves

*/
TCanvas* VPlotRadialAcceptance::plotRadialAcceptance( TCanvas* cX )
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
		cX = new TCanvas( hname, htitle, 10, 10, 600, 600 );
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
		if( bPlotSame )
		{
			fAcceptanceHisto->Draw( "e same" );
		}
		else
		{
			fAcceptanceHisto->Draw( "e" );
		}
		fAcceptanceHisto->GetYaxis()->SetTitleOffset( 1.2 );
	}
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
TCanvas* VPlotRadialAcceptance::plotResiduals( TCanvas* cX, double i_res_min, double i_res_max, bool iPlotChi2 )
{
	if( !fAcceptanceFile || fAcceptanceFile->IsZombie() )
	{
		cout << "VPlotRadialAcceptance::plotResiduals() error: data missing";
		return 0;
	}
	
// canvas
	char hname[2000];
	if( cX )
	{
		cX->cd();
	}
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
		TH1D* hRes = VHistogramUtilities::get_ResidualHistogram_from_TF1( hname, fAcceptanceHisto, fAcceptanceFunction );
		
		if( hRes )
		{
			setHistogramPlottingStyle( hRes );
			hRes->SetTitle( "" );
			hRes->SetMinimum( i_res_min );
			hRes->SetMaximum( i_res_max );
			hRes->SetAxisRange( fAxis_x_min, fAxis_x_max );
			hRes->Draw();
			TLine* iL = new TLine( hRes->GetXaxis()->GetXmin(), 0., fAxis_x_max, 0. );
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
				TText* iT = new TText( 0.5 * fAxis_x_max, 0.7 * i_res_max, hname );
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


TCanvas* VPlotRadialAcceptance::plotPhiDependentRadialAcceptances( TCanvas* cX, int iIterator, bool iDeRot )
{
	if( !fAcceptanceFile || fAcceptanceFile->IsZombie() )
	{
		cout << "VPlotRadialAcceptance::plotPhiDependentRadialAcceptances() error: data missing";
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
		sprintf( hname, "cAcceptancePhi_%s_%d", VUtilities::removeSpaces( fName ).c_str(), ( int )iDeRot );
		string iTitle = fName;
		if( iDeRot )
		{
			fName += "(Phi dependent, derotated)";
		}
		else
		{
			fName += "(Phi dependent)";
		}
		cX = new TCanvas( hname, fName.c_str(), 60, 610, 600, 600 );
		cX->SetGridx( 0 );
		cX->SetGridy( 0 );
	}
	
	vector< TH1F* > iHisto;
	vector< TF1* > iF1;
	if( !iDeRot )
	{
		iHisto = fAcceptancePhiHisto;
		iF1    = fAcceptancePhiFitFunction;
	}
	else
	{
		iHisto = fAcceptancePhiHistoDeRot;
		iF1    = fAcceptancePhiFitFunctionDeRot;
	}
	
// plot all histograms and plot them
	int i_color = 1;
	for( unsigned int i = 0; i < iHisto.size(); i += iIterator )
	{
		if( i < iHisto.size() && iHisto[i] )
		{
			cout << "Phi: " << iHisto[i]->GetTitle() << "\t color " << i_color << endl;
			iHisto[i]->SetMinimum( fAxis_y_min );
			iHisto[i]->SetMaximum( fAxis_y_max );
			iHisto[i]->SetAxisRange( fAxis_x_min, fAxis_x_max );
			iHisto[i]->SetTitle( "" );
			setHistogramPlottingStyle( iHisto[i], i_color, 1., 1.5, 20 );
			if( bPlotSame )
			{
				iHisto[i]->Draw( "e same" );
			}
			else
			{
				iHisto[i]->Draw( "e" );
			}
			iHisto[i]->GetYaxis()->SetTitleOffset( 1.2 );
			bPlotSame = true;
			if( i < iF1.size() && iF1[i] )
			{
				setFunctionPlottingStyle( iF1[i], i_color, 1. );
				iF1[i]->SetLineStyle( 2 );
				iF1[i]->Draw( "same" );
			}
			
			i_color++;
		}
	}
	if( fAcceptanceFunction )
	{
		setFunctionPlottingStyle( fAcceptanceFunction, 1, 2 );
		fAcceptanceFunction->Draw( "same" );
	}
	
	return cX;
}

TCanvas*  VPlotRadialAcceptance::plotPhiDistributions( TCanvas* cX, int iColor )
{
	if( !fAcceptanceFile || fAcceptanceFile->IsZombie() )
	{
		cout << "VPlotRadialAcceptance::plotPhiDistributions() error: data missing";
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
		sprintf( hname, "cPhiDistribution%s", VUtilities::removeSpaces( fName ).c_str() );
		string iTitle = fName;
		fName += ", Phi distribution";
		cX = new TCanvas( hname, fName.c_str(), 60, 610, 600, 600 );
		cX->SetGridx( 0 );
		cX->SetGridy( 0 );
	}
	
	if( hPhiDist )
	{
		setHistogramPlottingStyle( hPhiDist, iColor );
		hPhiDist->SetLineWidth( 2 );
		hPhiDist = ( TH1F* )VHistogramUtilities::normalizeTH1( ( TH1* )hPhiDist, false );
		hPhiDist->GetXaxis()->SetTitle( "azimuth (camera) [deg]" );
		hPhiDist->SetMinimum( 0 );
		if( bPlotSame )
		{
			hPhiDist->Draw( "he same" );
		}
		else
		{
			hPhiDist->Draw( "he" );
		}
		
		/*        if( hPhiDistDeRot )
			{
			   hPhiDistDeRot = (TH1F*)VHistogramUtilities::normalizeTH1( (TH1*)hPhiDistDeRot, false );
			   setHistogramPlottingStyle( hPhiDistDeRot, 2 );
			   hPhiDistDeRot->Draw( "he same" );
		        } */
		TLine* iL = new TLine( hPhiDist->GetXaxis()->GetXmin(), 1., hPhiDist->GetXaxis()->GetXmax(), 1. );
		iL->SetLineStyle( 2 );
		iL->Draw();
	}
	return cX;
}
