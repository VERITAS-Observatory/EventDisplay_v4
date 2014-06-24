/********************************************************************************************************************
 ********************************************************************************************************************
    \class VEnergyThreshold
    \brief determine energy thresholds


    This class can be used in three ways:

    i)   calculate energy thresholds from an effective area file
         (the latter must contain all histograms, no short version)

// create a new instance, create the output root file
VEnergyThreshold a( "myOutputFile.root" );

// open a file with effective areas
a.openEffectiveAreaFile( "myEffectiveAreas.root" );

// calculate energy thresholds
a.calculateEnergyThreshold();

// write results to disk
a.writeResults();

ii)   use class to get energy thresholds

// create a new instance, open the data file
VEnergyThreshold a( 0.0, "myOutputFile.root" );

// return energy threshold for a given histogram with systematic error
double e1 = a.getEnergy_maxSystematic( h1D_WithSystematicError, 0.1 );

// return energy threshold for a given graph with effective areas
double e2 = a.getEnergy_MaxEffectiveAreaFraction( graph_EffectiveAreas, 0.1 );

iii)  plot energy thresholds

// create a new instance, open the data file
VEnergyThreshold a( 0.0, "myOutputFile.root" );

// plot everything (for ze=20, wobble offset=0.5 deg, noise = 150, index = 2.4, az bin = 16)
a.plot_energyThresholds( "E_diffmax", 20., 0.5, 150, 2.4, 16 );

Energy thresholds are defined (currently) in 3 ways:

i)   peak of the differential counting rate
(output tree variable: E_diffmax)

ii)  at a specific maximum systematic error in the energy reconstruction
(i.e. 10%)
(output tree variable: E_sys10p, E_sys15p, E_sys20p (10, 15, 20%))

iii) at a fraction of the maximum value of the effective area (i.e.10%)
(output tree variable: E_effFract_05p, E_effFract_10p, E_effFract_20p, E_effFract_50p, E_effFract_90p)

\author Gernot Maier

********************************************************************************************************************
********************************************************************************************************************
********************************************************************************************************************/

#include "VEnergyThreshold.h"

VEnergyThreshold::VEnergyThreshold()
{
	fDebug = false;
	
	fEffArea = 0;
	
	fOutFile = 0;
	fTreeEth = 0;
	
	fEnergyThresholdFile = 0;
	fEnergyThresholdFixedValue = 0.001;
	fEnergyThresholdFileName = "";
	
	setPlottingStyle();
	setPlottingYaxis();
}


VEnergyThreshold::VEnergyThreshold( string ioutfilename )
{
	fDebug = false;
	
	fEffArea = 0;
	
	cout << "opening output file " << ioutfilename << endl;
	fOutFile = new TFile( ioutfilename.c_str(), "RECREATE" );
	if( fOutFile->IsZombie() )
	{
		cout << "Error opening output file " << ioutfilename << endl;
	}
	
	fTreeEth = 0;
	
	setPlottingStyle();
	setPlottingYaxis();
}


VEnergyThreshold::VEnergyThreshold( double iEnergyThresholdFixed, string iEnergyThresholdFile )
{
	fDebug = false;
	
	fEffArea = 0;
	
	fOutFile = 0;
	fTreeEth = 0;
	
	fEnergyThresholdFile = 0;
	
	fEnergyThresholdFixedValue = iEnergyThresholdFixed;
	fEnergyThresholdFileName = iEnergyThresholdFile;
	
	setPlottingStyle();
	setPlottingYaxis();
}


bool VEnergyThreshold::openEffectiveAreaFile( string iname )
{
	if( fDebug )
	{
		cout << "VEnergyThreshold::openEffectiveAreaFile " << iname << endl;
	}
	
	TChain* f = new TChain( "fEffArea" );
	f->Add( iname.c_str() );
	if( f->GetListOfFiles()->GetEntries() == 0 )
	{
		cout << "error opening " << iname << "(" << f->GetListOfFiles()->GetEntries() << ")" << endl;
		return false;
	}
	
	fEffArea = new CEffArea( f );
	cout << "reading effective areas from " << iname << endl;
	cout << "total number of effective areas: " << f->GetEntries() << endl;
	
	return true;
}


bool VEnergyThreshold::writeResults()
{
	if( !fOutFile->IsZombie() && fTreeEth )
	{
		cout << "writing threshold tree to " << fOutFile->GetName() << endl;
		fTreeEth->Write();
	}
	
	return true;
}


/*
    determine energy threshold by maximum in differential counting rate

    (use linear histogram, constant binning-> no difference between
     differential and non-differential histogram)
*/
bool VEnergyThreshold::calculateEnergyThreshold( bool bFit, int nentries )
{
	if( fDebug )
	{
		cout << "VEnergyThreshold::calculateEnergyThreshold " << fTreeEth << endl;
	}
	
	if( !fEffArea )
	{
		cout << "VEnergyThreshold::calculateEnergyThreshold() error reading effective area file" << endl;
		return false;
	}
	
	if( !fTreeEth )
	{
		if( !setUpThresholdTree() )
		{
			return false;
		}
	}
	
	if( nentries < 0 )
	{
		nentries = fEffArea->fChain->GetEntries();
	}
	if( nentries > fEffArea->fChain->GetEntries() )
	{
		nentries = fEffArea->fChain->GetEntries();
	}
	for( int i = 0; i < nentries; i++ )
	{
		fEffArea->GetEntry( i );
		
		if( i % 10000 == 0 && i > 0 )
		{
			cout << "\t now at entry " << i << endl;
		}
		
		feth = 0.;
		fesys_10p = 0.;
		fesys_15p = 0.;
		fesys_20p = 0.;
		
		feffFract_05p = 0;
		feffFract_10p = 0;
		feffFract_20p = 0;
		feffFract_50p = 0;
		feffFract_90p = 0;
		
		copyEntry();
		
		TH1D* hLin = fEffArea->hEcut500;
		if( hLin )
		{
			feth = getEnergyThreshold( hLin, true, bFit );
		}
		
		TProfile* hSys = fEffArea->hEsysMCRelative;
		if( hSys )
		{
			fesys_10p = getEnergy_maxSystematic( hSys, 0.10 );
			fesys_15p = getEnergy_maxSystematic( hSys, 0.15 );
			fesys_20p = getEnergy_maxSystematic( hSys, 0.20 );
		}
		TGraph* hG = fEffArea->gEffAreaRec;
		if( hG )
		{
			feffFract_05p = getEnergy_MaxEffectiveAreaFraction( hG, 0.05 );
			feffFract_10p = getEnergy_MaxEffectiveAreaFraction( hG, 0.10 );
			feffFract_20p = getEnergy_MaxEffectiveAreaFraction( hG, 0.20 );
			feffFract_50p = getEnergy_MaxEffectiveAreaFraction( hG, 0.50 );
			feffFract_90p = getEnergy_MaxEffectiveAreaFraction( hG, 0.90 );
		}
                if( TMath::Abs( fEffArea->index - 2.4 ) < 1.e-3 && TMath::Abs( fEffArea->azMax - 1000. ) < 1.e-3 )
                {
                    cout << "Threshold for index " << fEffArea->index << ": ";
                    cout << feth << " TeV, 10\% sys: " << fesys_10p << endl;
                }
		fTreeEth->Fill();
	}
	return true;
}


/*!
    calculate lower energy threshold from systematics:
    (interpolate between two closest values)
*/
double VEnergyThreshold::getEnergy_maxSystematic( TObject* h, double iSys )
{
	if( !h )
	{
		return 0.;
	}
	vector< double > x;
	vector< double > y;
	
	if( strcmp( "TProfile", h->ClassName() ) == 0 )
	{
		TProfile* p = ( TProfile* )h;
		for( int i = 1; i < p->GetNbinsX(); i++ )
		{
			if( p->GetBinError( i ) < 0.001 )
			{
				continue;
			}
			x.push_back( p->GetBinCenter( i ) );
			y.push_back( p->GetBinContent( i ) );
		}
	}
	else if( strcmp( "TGraphErrors", h->ClassName() ) == 0 )
	{
		double a, b;
		TGraphErrors* g = ( TGraphErrors* )h;
		for( int i = 0; i < g->GetN(); i++ )
		{
			g->GetPoint( i, a, b );
			x.push_back( a );
			y.push_back( b );
		}
	}
	
	double x1, x2, y1, y2, a, b;
	
	for( unsigned int i = 0; i < x.size(); i++ )
	{
		y1 =  y[i];
		if( y1 < iSys )
		{
			x1 = x[i];
			if( i > 1 )
			{
				y2 = y[i - 1];
				x2 = x[i - 1];
				
				if( x1 - x2 != 0. )
				{
					a = ( y1 - y2 ) / ( x1 - x2 );
					b = y1 - a * x1;
					if( a != 0. )
					{
						return TMath::Power( 10., ( iSys - b ) / a );
					}
					else
					{
						return 0.;
					}
				}
			}
			else
			{
				return TMath::Power( 10., x1 );
			}
		}
	}
	
	return 0.;
}


double VEnergyThreshold::getEnergyThreshold( TH1D* h, bool bLogEnergyAxis, bool bFit )
{
	if( !h )
	{
		return 0.;
	}
	
	int iMaxBin = h->GetMaximumBin();
	
	// weighed bin of three largest bins
	if( !bFit )
	{
		double iTotWeight = 0.;
		for( int tt = iMaxBin - 1; tt <= iMaxBin + 1; tt++ )
		{
			if( bLogEnergyAxis )
			{
				feth += h->GetBinContent( tt ) * TMath::Power( 10., h->GetXaxis()->GetBinCenter( tt ) );
			}
			else
			{
				feth += h->GetBinContent( tt ) * h->GetXaxis()->GetBinCenter( tt );
			}
			iTotWeight += h->GetBinContent( tt );
		}
		if( iTotWeight > 0. )
		{
			feth /= iTotWeight;
		}
	}
	// fit region around maximum by pol5
	else
	{
		double i_negX = 0.08;
		double i_posX = 0.1;
		if( bLogEnergyAxis )
		{
			i_negX = 0.2;
			i_posX = 0.3;
		}
		TF1 i_f( "i_f", "pol5", h->GetXaxis()->GetBinCenter( iMaxBin ) - i_negX, h->GetXaxis()->GetBinCenter( iMaxBin ) + i_posX );
		h->Fit( i_f.GetName(), "R0Q" );
		if( bLogEnergyAxis )
		{
			feth = TMath::Power( 10., i_f.GetMaximumX() );
		}
		else
		{
			feth = i_f.GetMaximumX();
		}
	}
	
	return feth;
}


void VEnergyThreshold::copyEntry()
{
	fze = fEffArea->ze;
	fAzBin = fEffArea->az;
	fAzMin  = fEffArea->azMin;
	fAzMax = fEffArea->azMax;
	fXoff = fEffArea->Xoff;
	fYoff = fEffArea->Yoff;
	fWoff = fEffArea->Woff;
	fTNoise = fEffArea->noise;
	fTNoisePE = fEffArea->noisePE;
	fTPedvar = fEffArea->pedvar;
	fSpectralIndex = fEffArea->index;
}


bool VEnergyThreshold::setUpThresholdTree()
{
	if( fDebug )
	{
		cout << "VEnergyThreshold::setUpThresholdTree  " << fTreeEth << endl;
	}
	if( fTreeEth )
	{
		return true;
	}
	
	if( !fOutFile )
	{
		cout << "VEnergyThreshold::setUpThresholdTree: no output file defined" << endl;
		return false;
	}
	if( fOutFile->IsZombie() )
	{
		cout << "Error in setting up trees: no output file found" << endl;
		return false;
	}
	
	fOutFile->cd();
	
	fTreeEth = new TTree( "fTreeEth", "thresholds in energy reconstruction" );
	fTreeEth->Branch( "ze", &fze, "ze/D" );
	fTreeEth->Branch( "az", &fAzBin, "az/I" );
	fTreeEth->Branch( "azMin", &fAzMin, "azMin/D" );
	fTreeEth->Branch( "azMax", &fAzMax, "azMax/D" );
	fTreeEth->Branch( "Xoff", &fXoff, "Xoff/D" );
	fTreeEth->Branch( "Yoff", &fYoff, "Yoff/D" );
	fTreeEth->Branch( "Woff", &fWoff, "Woff/D" );
	fTreeEth->Branch( "noise", &fTNoise, "noise/I" );
	fTreeEth->Branch( "noisePE", &fTNoisePE, "noisePE/D" );
	fTreeEth->Branch( "pedvar", &fTPedvar, "pedvar/D" );
	fTreeEth->Branch( "index", &fSpectralIndex, "index/D" );
	fTreeEth->Branch( "E_diffmax", &feth, "E_diffmax/D" );
	fTreeEth->Branch( "E_sys10p", &fesys_10p, "E_sys10p/D" );
	fTreeEth->Branch( "E_sys15p", &fesys_15p, "E_sys15p/D" );
	fTreeEth->Branch( "E_sys20p", &fesys_20p, "E_sys20p/D" );
	fTreeEth->Branch( "E_effFract_05p", &feffFract_05p, "E_effFract_05p/D" );
	fTreeEth->Branch( "E_effFract_10p", &feffFract_10p, "E_effFract_10p/D" );
	fTreeEth->Branch( "E_effFract_20p", &feffFract_20p, "E_effFract_20p/D" );
	fTreeEth->Branch( "E_effFract_50p", &feffFract_50p, "E_effFract_50p/D" );
	fTreeEth->Branch( "E_effFract_90p", &feffFract_90p, "E_effFract_90p/D" );
	
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

/*
   return energy threshold in TeV (linear energy axis)
*/
double VEnergyThreshold::getEnergyThreshold( VRunList* iRunData )
{
	if( fDebug )
	{
		cout << "VEnergyThreshold::getEnergyThreshold " << iRunData << " ";
		cout << fEnergyThresholdFileName.size() << " " << fEnergyThresholdFixedValue << endl;
	}
	
	// no file with energy thresholds given; return fixed value
	if( fEnergyThresholdFileName.size() <= 0 )
	{
		return fEnergyThresholdFixedValue;
	}
	
	// no run data given, return 0 energy threshold
	if( iRunData == 0 )
	{
		return 0.001;
	}
	
	// open file with energy thresholds
	if( !fEnergyThresholdFile && !openEnergyThresholdFile() )
	{
		return 0.001;
	}
	
	// interpolate between energy thresholds
	return interpolateEnergyThreshold( iRunData );
}


bool VEnergyThreshold::openEnergyThresholdFile()
{
	if( fEnergyThresholdFileName.size() <= 0 )
	{
		return false;
	}
	
	fEnergyThresholdFile = new TFile( fEnergyThresholdFileName.c_str() );
	if( fEnergyThresholdFile->IsZombie() )
	{
		cout << "Error opening file with energy thresholds: " << fEnergyThresholdFileName << endl;
		return false;
	}
	fTreeEth = ( TTree* )fEnergyThresholdFile->Get( "fTreeEth" );
	if( !fTreeEth )
	{
		return false;
	}
	int nentries = fTreeEth->GetEntries();
	
	cout << "reading " << fEnergyThresholdFile->GetName() << endl;
	cout << "total number of entries in energy threshold tree: " << nentries << endl;
	
	fTreeEth->SetBranchAddress( "ze", &fze );
	// DO THIS WITH INTEGERS!!!!!
	// read in all entries and see what values are available
	vector< double > fv_Ze;
	fv_Ze.reserve( nentries );
	
	for( int i = 0; i < fTreeEth->GetEntries(); i++ )
	{
		fTreeEth->GetEntry( i );
		
		fv_Ze.push_back( fze );
	}
	sort( fv_Ze.begin(), fv_Ze.end() );
	vector<double>::iterator p_d = unique( fv_Ze.begin(), fv_Ze.end() );
	fv_Ze.erase( p_d, fv_Ze.end() );
	
	return true;
}


/*
  interplate between different energy thresholds given in iRunData
*/
double VEnergyThreshold::interpolateEnergyThreshold( VRunList* iRunData )
{
	if( iRunData )
	{
		return 0.001;
	}
	return 0.001;
}


void VEnergyThreshold::plot_energyThresholds( string var, double ze, double woff, int noise, double index, int az, bool bPlot, string plot_option )
{
	if( !fTreeEth )
	{
		if( !openEnergyThresholdFile() )
		{
			cout << "error opening energy threshold file" << endl;
			return;
		}
	}
	
	char hname[600];
	
	gStyle->SetPadTopMargin( 0.1 );
	gStyle->SetTitleAlign( 23 );
	gStyle->SetTitleFontSize( 0.05 );
	gStyle->SetTitleX( 0.5 );
	
	vector< string > iDraw;
	vector< string > iCut;
	vector< string > iName;
	vector< string > iTitle;
	
	// energy threshold vs spectral index
	iName.push_back( "spectral index" );
	iDraw.push_back( var + "*1.e3:index" );
	sprintf( hname, "az == %d && noise == %d && TMath::Abs( ze - %f ) < 0.1 && TMath::Abs( Woff - %f ) < 0.05", az, noise, ze, woff );
	iCut.push_back( hname );
	sprintf( hname, "ze=%d deg, woff = %.2f deg, noise level = %d", ( int )ze, woff, noise );
	iTitle.push_back( hname );
	
	// energy threshold vs zenith angle
	iName.push_back( "zenith angle [deg]" );
	iDraw.push_back( var + "*1.e3:ze" );
	sprintf( hname, "az == %d && noise == %d && TMath::Abs( index - %f ) < 0.1 && TMath::Abs( Woff - %f ) < 0.05", az, noise, index, woff );
	iCut.push_back( hname );
	sprintf( hname, "woff = %.2f deg, noise level = %d, spectral index = %.1f", woff, noise, index );
	iTitle.push_back( hname );
	
	// energy threshold vs wobble offsets
	iName.push_back( "wobble offset [deg]" );
	iDraw.push_back( var + "*1.e3:Woff" );
	sprintf( hname, "az == %d && noise == %d && TMath::Abs( index - %f ) < 0.1 && TMath::Abs( ze - %f ) < 0.05", az, noise, index, ze );
	iCut.push_back( hname );
	sprintf( hname, "ze=%d deg, noise level = %d, spectral index = %.1f", ( int )ze, noise, index );
	iTitle.push_back( hname );
	
	// energy threshold vs pedestal variations
	iName.push_back( "pedestal variation" );
	iDraw.push_back( var + "*1.e3:pedvar" );
	sprintf( hname, "az == %d && TMath::Abs( Woff - %f ) < 0.05 && TMath::Abs( index - %f ) < 0.1 && TMath::Abs( ze - %f ) < 0.05", az, woff, index, ze );
	iCut.push_back( hname );
	sprintf( hname, "ze=%d deg, woff = %.2f deg, spectral index = %.1f", ( int )ze, woff, index );
	iTitle.push_back( hname );
	
	// energy threshold vs azimuth angle
	iName.push_back( "azimuth angle [deg]" );
	iDraw.push_back( var + "*1.e3:azMin+30." );
	sprintf( hname, "az != 16 && noise == %d && TMath::Abs( Woff - %f ) < 0.05 && TMath::Abs( index - %f ) < 0.1 && TMath::Abs( ze - %f ) < 0.05", noise, woff, index, ze );
	iCut.push_back( hname );
	sprintf( hname, "ze=%d deg, woff = %.2f deg, noise level = %d, spectral index = %.1f", ( int )ze, woff, noise, index );
	iTitle.push_back( hname );
	
	TCanvas* c = 0;
	
	for( unsigned int i = 0; i < iDraw.size(); i++ )
	{
		sprintf( hname, "c_%d", i );
		if( bPlot )
		{
			c = new TCanvas( hname, hname, 10 + i * 100, 100, 450, 450 );
			c->SetGridx( 0 );
			c->SetGridy( 0 );
			c->SetLeftMargin( 0.13 );
			c->Draw();
		}
		else
		{
			c = ( TCanvas* )gROOT->GetListOfCanvases()->FindObject( hname );
			if( !c )
			{
				continue;
			}
			c->cd();
		}
		
		fTreeEth->SetMarkerStyle( fPlottingMarkerStyle );
		fTreeEth->SetMarkerColor( fPlottingMarkerColor );
		fTreeEth->SetLineColor( fPlottingMarkerColor );
		fTreeEth->SetMarkerSize( fPlottingMarkerSize );
		fTreeEth->SetLineWidth( ( Width_t )fPlottingLineWidth );
		
		if( bPlot )
		{
			fTreeEth->Draw( iDraw[i].c_str(), iCut[i].c_str(), plot_option.c_str() );
		}
		else
		{
			fTreeEth->Draw( iDraw[i].c_str(), iCut[i].c_str(), ( plot_option + " same" ).c_str() );
		}
		TH2F* htemp = ( TH2F* )gPad->GetPrimitive( "htemp" );
		if( htemp )
		{
			htemp->SetTitle( "" );
			htemp->SetTitle( iTitle[i].c_str() );
			htemp->SetXTitle( iName[i].c_str() );
			htemp->SetYTitle( "energy threshold [GeV]" );
			htemp->GetYaxis()->SetTitleOffset( 1.6 );
			if( bPlot && TMath::Abs( fPlottingYmin - fPlottingYmax ) > 1.e-2 )
			{
				sprintf( hname, "htemp_H_%d", i );
				TH2F* h = new TH2F( hname, "", 100,  htemp->GetXaxis()->GetXmin(), htemp->GetXaxis()->GetXmax(), 100, fPlottingYmin, fPlottingYmax );
				h->SetStats( 0 );
				h->SetXTitle( htemp->GetXaxis()->GetTitle() );
				h->SetYTitle( htemp->GetYaxis()->GetTitle() );
				h->GetYaxis()->SetTitleOffset( htemp->GetYaxis()->GetTitleOffset() );
				h->Draw();
				fTreeEth->Draw( iDraw[i].c_str(), iCut[i].c_str(), ( plot_option + "same" ).c_str() );
			}
		}
		// search for last plotted graph in TPad
		TGraph* g = 0;
		TObjLink* lnk = gPad->GetListOfPrimitives()->FirstLink();
		while( lnk )
		{
			TObject* obj = lnk->GetObject();
			if( strcmp( obj->ClassName(), "TGraph" ) == 0 )
			{
				g = ( TGraph* )obj;
			}
			lnk = lnk->Next();
		}
		if( g )
		{
			g->SetLineWidth( ( Width_t )fPlottingLineWidth );
			g->Draw( plot_option.c_str() );
		}
		// draw titles
		/*        TText *tx = 0;
			if( i < iTitle.size() )
			{
			   tx = new TText( 0.5, 0.5, iTitle[i].c_str() );
			   tx->Draw();
		        } */
		c->Update();
	}
	
}


double VEnergyThreshold::getEnergy_MaxEffectiveAreaFraction( TObject* h, double iFrac )
{
	if( !h )
	{
		return 0.;
	}
	
	vector< double > x;
	vector< double > y;
	double max = 0.;
	
	if( strcmp( "TGraphErrors", h->ClassName() ) == 0 || strcmp( "TGraph", h->ClassName() ) == 0 || strcmp( "TGraphAsymmErrors", h->ClassName() ) == 0 )
	{
		TGraph* g = ( TGraph* )h;
		double a, b;
		for( int i = 0; i < g->GetN(); i++ )
		{
			g->GetPoint( i, a, b );
			x.push_back( a );
			y.push_back( b );
			if( b > max )
			{
				max = b;
			}
		}
	}
	
	if( max <= 0. )
	{
		return 0.;
	}
	
	for( unsigned int i = 0; i < x.size(); i++ )
	{
		if( y[i] / max > iFrac )
		{
			return TMath::Power( 10., x[i] );
		}
	}
	
	return 0.;
}
