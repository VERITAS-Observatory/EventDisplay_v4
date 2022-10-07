/*! \file VInstrumentResponseFunctionReader

    data class for effective area plotting


*/

#include "VInstrumentResponseFunctionReader.h"

VInstrumentResponseFunctionReader::VInstrumentResponseFunctionReader()
{
	fIsZombie = true;
	fDebug = false;
	
	fGammaHadronCuts_directionCut_selector = 0;
	
	fFile = "";
	fA_MC = "A_MC";
	fZe = 0.;
	fWoff = 0.;
	fAzbin = 0;
	fIndex = 0.;
	fNoise = 0;
	fPlotOption = "pl";
	fColor = 1;
	fLineStyle = 1;
	fMarkerStyle = 20;
	fLegend = "";
	
	gEffArea_MC = 0;
	gEffArea_MC_Ratio = 0;
	gEffArea_Rec = 0;
	gEffArea_Rec_Ratio = 0;
	
	hEmc = 0;
	hEcut = 0;
	hEcut_rec = 0;
	hEcutUW = 0;
	hEcut_recUW = 0;
	hEsys = 0;
	hERecMatrix = 0;
	hERecMatrixProfile = 0;
	hERecMatrixCoarse = 0;
	hEsysMCRelative = 0;
	hEsysMCRelative2D = 0;
	gEnergyResolution = 0;
	gEnergyBias_Mean = 0;
	gEnergyBias_Median = 0;
	gEnergyLogBias_Mean = 0;
	gEnergyLogBias_Median = 0;
	gAngularResolution = 0;
	gAngularResolution80 = 0;
	gEffArea_Recp80 = 0;
	hWeightedRate = 0;
	
	initializeIRFData();
}

bool VInstrumentResponseFunctionReader::initializeIRFData()
{
	// read angular and core resolution
	fIRF_TreeNames.push_back( "t_angular_resolution" );
	fIRF_TreeNames.push_back( "t_angular_resolution_080p" );
	fIRF_TreeNames.push_back( "t_core_resolution" );
	fIRF_TreeNames.push_back( "t_energy_resolution" );
	
	for( unsigned int i = 0; i < fIRF_TreeNames.size(); i++ )
	{
		vector< TH2D* > iTemp;
		fIRF_Data.push_back( 0 );
	}
	
	return true;
}

bool VInstrumentResponseFunctionReader::fillData( string iDataLine, int iDataID )
{
	if( iDataLine.size() == 0 )
	{
		fIsZombie = true;
		return false;
	}
	string temp;
	
	istringstream is_stream( iDataLine );
	is_stream >> temp;
	if( temp != "*" )
	{
		return false;
	}
	is_stream >> temp;
	// check set number
	if( atoi( temp.c_str() ) != iDataID )
	{
		return false;
	}
	// read this line
	is_stream >> fFile;
	is_stream >> temp;
	fZe = atof( temp.c_str() );
	is_stream >> temp;
	fAzbin = atoi( temp.c_str() );
	is_stream >> temp;
	fWoff = atof( temp.c_str() );
	is_stream >> temp;
	fIndex = atof( temp.c_str() );
	is_stream >> temp;
	fNoise = atoi( temp.c_str() );
	is_stream >> fA_MC;
	
	// plotting options
	is_stream >> fPlotOption;
	is_stream >> temp;
	fPlottingColor = atoi( temp.c_str() );
	is_stream >> temp;
	fPlottingLineStyle = atoi( temp.c_str() );
	is_stream >> temp;
	fPlottingMarkerStyle = atoi( temp.c_str() );
	fLegend = is_stream.str();
	fLegend = is_stream.str().substr( is_stream.tellg(), is_stream.str().size() );
	
	return fillData();
}

bool VInstrumentResponseFunctionReader::fillData( string iFile, double iZe, double iWoff, int iAzBin, double iIndex, int iNoise, string iA_MC )
{
	fFile = iFile;
	fZe = iZe;
	fWoff = iWoff;
	fAzbin = iAzBin;
	fIndex = iIndex;
	fNoise = iNoise;
	fA_MC = iA_MC;
	
	return fillData();
}

bool VInstrumentResponseFunctionReader::fillData()
{
	// read in all the necessary data from the effective area tree
	
	if( !getDataFromFile() )
	{
		fIsZombie = true;
		return false;
	}
	
	// calculate ratio of cut - efficiencies
	if( !calculateCutEfficiencies() )
	{
		return false;
	}
	
	fIsZombie = false;
	
	return true;
}

/*

    read response functions from CTA file

    see http://www.cta-observatory.org/ctawpcwiki/index.php/WP_MC for documentation

*/
bool VInstrumentResponseFunctionReader::getDataFromCTAFile()
{
	bool bLinX = false;  // energy axis for effective areas
	
	TH1F* h = 0;
	// gamma-ray effective area vs reconstruction energy
	h = get_CTA_IRF_Histograms( "EffectiveArea", fWoff );
	if( !h )
	{
		h = get_CTA_IRF_Histograms( "harea_gamma", fWoff );
		if( !h )
		{
			return false;
		}
		bLinX = true;
	}
	gEffArea_Rec = new TGraphAsymmErrors( 1 );
	gEffArea_Rec->SetName( "gEffArea_Rec" );
	setGraphPlottingStyle( gEffArea_Rec );
	get_Graph_from_Histogram( h, gEffArea_Rec, false, bLinX );
	// gamma-ray effective area vs true energy
	h = get_CTA_IRF_Histograms( "EffectiveAreaEtrue", fWoff );
	if( !h )
	{
		h = get_CTA_IRF_Histograms( "harea_gamma", fWoff );
		if( h )
		{
			bLinX = true;
		}
		else
		{
			h = get_CTA_IRF_Histograms( "EffectiveArea", fWoff );
		}
	}
	if( h )
	{
		gEffArea_MC = new TGraphAsymmErrors( 1 );
		gEffArea_MC->SetName( "gEffArea_MC" );
		setGraphPlottingStyle( gEffArea_MC );
		get_Graph_from_Histogram( h, gEffArea_MC, false, bLinX, -1., log10( fEnergyLinTeV_min ), log10( fEnergyLinTeV_max ) );
	}
	else
	{
		gEffArea_MC = 0;
	}
	
	///////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////
	// name and axis units are not consistent in the CTA files!!!
	///////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////
	// energy resolution
	h = 0;
	gEnergyResolution = new TGraphErrors( 1 );
	h = ( TH1F* )get_CTA_IRF_Histograms( "ERes", fWoff );
	if( !h )
	{
		h = ( TH1F* )get_CTA_IRF_Histograms( "EnResol_RMS", fWoff );
	}
	if( h )
	{
		get_Graph_from_Histogram( h, gEnergyResolution, true, 0., log10( fEnergyLinTeV_min ), log10( fEnergyLinTeV_max ) );
		setGraphPlottingStyle( gEnergyResolution );
	}
	///////////////////////////////////////////////////////////////
	// energy bias
	h = 0;
	gEnergyBias_Mean = new TGraphErrors( 1 );
	h = ( TH1F* )get_CTA_IRF_Histograms( "Ebias", fWoff );
	if( !h )
	{
		h = ( TH1F* )get_CTA_IRF_Histograms_from2D( "EestOverEtrue", -1. );
	}
	if( h )
	{
		get_Graph_from_Histogram( h, gEnergyBias_Mean, true, -100., log10( fEnergyLinTeV_min ), log10( fEnergyLinTeV_max ) );
		setGraphPlottingStyle( gEnergyBias_Mean );
	}
	///////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////
	// 68% angular resolution
	h = 0;
	gAngularResolution = new TGraphErrors( 1 );
	h = ( TH1F* )get_CTA_IRF_Histograms( "AngRes", fWoff );
	// try Paris style file
	if( !h )
	{
		h = ( TH1F* )get_CTA_IRF_Histograms( "AngResolution68", fWoff );
		// arcmin -> deg
		if( h )
		{
			h->Scale( 1. / 60. );
		}
	}
	if( h )
	{
		get_Graph_from_Histogram( h, gAngularResolution, true, 0., log10( fEnergyLinTeV_min ), log10( fEnergyLinTeV_max ) );   // ignore errors in resolution graph
		setGraphPlottingStyle( gAngularResolution );
	}
	
	///////////////////////////////////////////////////////////////
	// 80% angular resolution
	h = 0;
	gAngularResolution80 = new TGraphErrors( 1 );
	h = ( TH1F* )get_CTA_IRF_Histograms( "AngRes80", fWoff );
	// try Paris style file
	if( !h )
	{
		h = ( TH1F* )get_CTA_IRF_Histograms( "AngResolution80", fWoff );
		// arcmin -> deg
		if( h )
		{
			h->Scale( 1. / 60. );
		}
	}
	if( h )
	{
		get_Graph_from_Histogram( h, gAngularResolution80, true, 0., log10( fEnergyLinTeV_min ), log10( fEnergyLinTeV_max ) );   // ignore errors in resolution graph
		setGraphPlottingStyle( gAngularResolution80 );
	}
	if( h )
	{
		get_Graph_from_Histogram( h, gAngularResolution80, true );   // ignore errors in resolution graph
		setGraphPlottingStyle( gAngularResolution80 );
	}
	
	return true;
}

/*
 * read IRF from a effective area file
 * (MC case only - from combined files)
 *
 */
bool VInstrumentResponseFunctionReader::fill_from_effectiveAreaFromH2( TTree* t )
{
	if( !t )
	{
		return false;
	}
	bool bFound = false;
	
	Float_t ze;
	UShort_t az;
	Float_t Woff;
	UShort_t noise;
	UShort_t nbins;
	Float_t  e0[600];
	Float_t eff[600];
	UShort_t nbins_esys;
	Float_t e0_esys[600];
	Float_t esys_rel[600];
	Int_t binsx;
	Float_t minx;
	Float_t maxx;
	Int_t binsy;
	Float_t miny;
	Float_t maxy;
	Int_t binsxy;
	Float_t value[10000];
	
	t->SetBranchAddress( "ze", &ze );
	t->SetBranchAddress( "az", &az );
	t->SetBranchAddress( "Woff", &Woff );
	t->SetBranchAddress( "noise", &noise );
	t->SetBranchAddress( "nbins", &nbins );
	t->SetBranchAddress( "e0", e0 );
	t->SetBranchAddress( "eff", eff );
	t->SetBranchAddress( "nbins_esys", &nbins_esys );
	t->SetBranchAddress( "e0_esys", e0_esys );
	t->SetBranchAddress( "esys_rel", esys_rel );
	t->SetBranchAddress( "hEsysMCRelative2D_binsx", &binsx );
	t->SetBranchAddress( "hEsysMCRelative2D_minx", &minx );
	t->SetBranchAddress( "hEsysMCRelative2D_maxx", &maxx );
	t->SetBranchAddress( "hEsysMCRelative2D_binsy", &binsy );
	t->SetBranchAddress( "hEsysMCRelative2D_miny", &miny );
	t->SetBranchAddress( "hEsysMCRelative2D_maxy", &maxy );
	t->SetBranchAddress( "hEsysMCRelative2D_binsxy", &binsxy );
	t->SetBranchAddress( "hEsysMCRelative2D_value", value );
	
	for( int j = 0; j < t->GetEntries(); j++ )
	{
		t->GetEntry( j );
		if( fDebug )
		{
			cout << "VInstrumentResponseFunctionReader::getDataFromFile: reading event " << j << endl;
		}
		
		// ignore all checks if there is only one entry in this tree
		if( t->GetEntries() > 1 )
		{
			// azimuth
			if( fDebug )
			{
				cout << "\t AZ: tree entry " << j << "\t az " << az << "\t az bin " << fAzbin << endl;
			}
			if( az != fAzbin )
			{
				continue;
			}
			// wobble offset
			if( fDebug )
			{
				cout << "\t Woff: " << j << "\t" << Woff << "\t" << fWoff << endl;
			}
			if( TMath::Abs( Woff - fWoff ) > 0.05 )
			{
				continue;
			}
			// noise level
			if( fDebug )
			{
				cout << "\t Noise: " << j << "\t" << noise << "\t" << fNoise << endl;
			}
			if( noise != fNoise )
			{
				continue;
			}
			// zenith angle
			if( fDebug )
			{
				cout << "\t Ze: " << j << "\t" << ze << "\t" << fZe << endl;
			}
			if( TMath::Abs( ze - fZe ) > 3. )
			{
				continue;
			}
		}
		cout << "\t FOUND EFFECTIVE AREA (entry " << j << ")" << endl;
		bFound = true;
		gEffArea_MC = new TGraphAsymmErrors( nbins );
		for( int k = 0; k < nbins; k++ )
		{
			gEffArea_MC->SetPoint( k, e0[k], eff[k] );
		}
		setGraphPlottingStyle( gEffArea_MC );
		gEnergyBias_Mean = new TGraphErrors( nbins_esys );
		for( int k = 0; k < nbins_esys; k++ )
		{
			gEnergyBias_Mean->SetPoint( k, e0_esys[k], esys_rel[k] );
		}
		setGraphPlottingStyle( gEnergyBias_Mean );
		hEsysMCRelative2D = new TH2D( "hEsysMCRelative2D", "",
									  binsx, minx, maxx,
									  binsy, miny, maxy );
		hEsysMCRelative2D->SetXTitle( "energy_{MC} [TeV]" );
		hEsysMCRelative2D->SetYTitle( "energy bias E_{rec}/E_{MC}" );
		for( int i = 0; i < binsxy; i++ )
		{
			int nx = i % binsx;
			int ny = ( i - nx ) / binsx;
			hEsysMCRelative2D->SetBinContent( nx + 1, ny + 1, value[i] );
		}
		
		break;
	}
	
	
	return bFound;
}

/*
 * read IRF from a effective area file
 *
 */
bool VInstrumentResponseFunctionReader::fill_from_effectiveArea( TTree* t )
{
	if( !t )
	{
		return false;
	}
	CEffArea* c = new CEffArea( t );
	
	bool bFound = false;
	for( int j = 0; j < c->fChain->GetEntries(); j++ )
	{
		c->GetEntry( j );
		
		if( fDebug )
		{
			cout << "VInstrumentResponseFunctionReader::getDataFromFile: reading event " << j << endl;
		}
		
		// ignore all checks if there is only one entry in this tree
		if( c->fChain->GetEntries() > 1 )
		{
			// azimuth
			if( fDebug )
			{
				cout << "\t AZ: tree entry " << j << "\t az " << c->az << "\t az bin " << fAzbin << endl;
			}
			if( c->az != fAzbin )
			{
				continue;
			}
			// spectral index
			if( fDebug )
			{
				cout << "\t Index: " << j << "\t" << c->index << "\t " << fIndex << endl;
			}
			if( TMath::Abs( c->index - fIndex ) > 0.05 )
			{
				continue;
			}
			// wobble offset
			if( fDebug )
			{
				cout << "\t Woff: " << j << "\t" << c->Woff << "\t" << fWoff << endl;
			}
			if( TMath::Abs( c->Woff - fWoff ) > 0.05 )
			{
				continue;
			}
			// noise level
			if( fDebug )
			{
				cout << "\t Noise: " << j << "\t" << c->noise << "\t" << fNoise << endl;
			}
			if( c->noise != fNoise )
			{
				continue;
			}
			// zenith angle
			if( fDebug )
			{
				cout << "\t Ze: " << j << "\t" << c->ze << "\t" << fZe << endl;
			}
			if( TMath::Abs( c->ze - fZe ) > 3. )
			{
				continue;
			}
		}
		cout << "\t FOUND EFFECTIVE AREA (entry " << j << ")" << endl;
		bFound = true;
		
		// get effective areas (these are effective area after the usual angular cut)
		if( c->gEffAreaMC )
		{
			gEffArea_MC  = ( TGraphAsymmErrors* )c->gEffAreaMC->Clone();
			setGraphPlottingStyle( gEffArea_MC );
		}
		else if( c->nbins > 0 )
		{
			gEffArea_MC  = new TGraphAsymmErrors( c->nbins );
			for( int k = 0; k < c->nbins; k++ )
			{
				gEffArea_MC->SetPoint( k, c->e0[k], c->eff[k] );
				gEffArea_MC->SetPointEYlow( k, c->seff_L[k] );
				gEffArea_MC->SetPointEYhigh( k, c->seff_U[k] );
			}
			setGraphPlottingStyle( gEffArea_MC );
		}
		else
		{
			gEffArea_MC = 0;
		}
		if( c->gEffAreaRec )
		{
			gEffArea_Rec = ( TGraphAsymmErrors* )c->gEffAreaRec->Clone();
			setGraphPlottingStyle( gEffArea_Rec );
		}
		else if( c->Rec_nbins > 0 )
		{
			gEffArea_Rec  = new TGraphAsymmErrors( c->Rec_nbins );
			for( int k = 0; k < c->Rec_nbins; k++ )
			{
				gEffArea_Rec->SetPoint( k, c->Rec_e0[k], c->Rec_eff[k] );
				gEffArea_Rec->SetPointEYlow( k, c->Rec_seff_L[k] );
				gEffArea_Rec->SetPointEYhigh( k, c->Rec_seff_U[k] );
			}
			setGraphPlottingStyle( gEffArea_Rec );
		}
		else
		{
			gEffArea_Rec = 0;
		}
		// get energy counting histogram
		if( c->hEmc )
		{
			hEmc = ( TH1D* )c->hEmc->Clone();
			setHistogramPlottingStyle( hEmc );
		}
		if( c->hEcut )
		{
			hEcut = ( TH1D* )c->hEcut->Clone();
			setHistogramPlottingStyle( hEcut );
		}
		if( c->hEcutRec )
		{
			hEcut_rec = ( TH1D* )c->hEcutRec->Clone();
			setHistogramPlottingStyle( hEcut_rec );
			hEcut_rec->SetMarkerStyle( hEcut->GetMarkerStyle() + 4 );
		}
		if( c->hEcutUW )
		{
			hEcutUW = ( TH1D* )c->hEcutUW->Clone();
			setHistogramPlottingStyle( hEcutUW );
		}
		if( c->hEcutRecUW )
		{
			hEcut_recUW = ( TH1D* )c->hEcutRecUW->Clone();
			setHistogramPlottingStyle( hEcut_recUW );
			hEcut_recUW->SetMarkerStyle( hEcutUW->GetMarkerStyle() + 4 );
		}
		// get energy reconstruction matrix
		hERecMatrix = ( TH2D* )c->hEmcCutCTA;
		hERecMatrixProfile = ( TProfile* )c->hResponseMatrixProfile;
		hERecMatrixCoarse = ( TH2D* )c->hResponseMatrix;
		hERecMatrixQC = ( TH2D* )c->hResponseMatrixFineQC;
		hERecMatrixCoarseQC = ( TH2D* )c->hResponseMatrixQC;
		// get error in energy reconstruction
		hEsys = ( TH2D* )c->hEsys2D;
		// erec/emc
		hEsysMCRelative = ( TProfile* )c->hEsysMCRelative;
		hEsysMCRelative2D = ( TH2D* )c->hEsysMCRelative2D;
		// get energy resolution (!!)
		//       getEnergyResolutionPlot( (TProfile*)c->hEsysMCRelative );
		// energy resolution calculation as 68% value
		//       getEnergyResolutionPlot68( (TH2D*)c->hEsysMCRelative2D );
		// energy resolution is RMS
		getEnergyResolutionPlot( ( TH2D* )c->hEsysMCRelativeRMS );
		//       getEnergyResolutionPlot( (TProfile*)c->hEsysRec );
		setGraphPlottingStyle( gEnergyResolution );
		// get energy bias
		gEnergyBias_Mean = get_Profile_from_TH2D( ( TH2D* )c->hEsysMCRelativeRMS, 0, "mean", 1, -10., 0. );
		setGraphPlottingStyle( gEnergyBias_Mean );
		gEnergyBias_Median = get_Profile_from_TH2D( ( TH2D* )c->hEsysMCRelativeRMS, 0, "median", 1, -10., 0. );
		setGraphPlottingStyle( gEnergyBias_Median, 1, 1., 7 );
		gEnergyLogBias_Mean = get_Profile_from_TH2D( ( TH2D* )c->hEsys2D, 0, "mean", 1, -10. );
		setGraphPlottingStyle( gEnergyLogBias_Mean, 1, 1., 7 );
		gEnergyLogBias_Median = get_Profile_from_TH2D( ( TH2D* )c->hEsys2D, 0, "median", 1, -10. );
		setGraphPlottingStyle( gEnergyLogBias_Median );
		// get rate histograms
		hWeightedRate = ( TH1D* )c->hWeightedRate;
		// get cut efficiencies
		if( c->hhEcutTrigger )
		{
			hCutEfficiency.push_back( ( TH1D* )c->hhEcutTrigger->Clone() );
		}
		else
		{
			hCutEfficiency.push_back( 0 );
		}
		if( c->hhEcutFiducialArea )
		{
			hCutEfficiency.push_back( ( TH1D* )c->hhEcutFiducialArea->Clone() );
		}
		else
		{
			hCutEfficiency.push_back( 0 );
		}
		if( c->hhEcutStereoQuality )
		{
			hCutEfficiency.push_back( ( TH1D* )c->hhEcutStereoQuality->Clone() );
		}
		else
		{
			hCutEfficiency.push_back( 0 );
		}
		if( c->hhEcutTelType )
		{
			hCutEfficiency.push_back( ( TH1D* )c->hhEcutTelType->Clone() );
		}
		else
		{
			hCutEfficiency.push_back( 0 );
		}
		if( c->hhEcutDirection )
		{
			hCutEfficiency.push_back( ( TH1D* )c->hhEcutDirection->Clone() );
		}
		else
		{
			hCutEfficiency.push_back( 0 );
		}
		if( c->hhEcutEnergyReconstruction )
		{
			hCutEfficiency.push_back( ( TH1D* )c->hhEcutEnergyReconstruction->Clone() );
		}
		else
		{
			hCutEfficiency.push_back( 0 );
		}
		if( c->hhEcutGammaHadron )
		{
			hCutEfficiency.push_back( ( TH1D* )c->hhEcutGammaHadron->Clone() );
		}
		else
		{
			hCutEfficiency.push_back( 0 );
		}
		for( unsigned int i = 0; i < hCutEfficiency.size(); i++ )
		{
			setHistogramPlottingStyle( hCutEfficiency[i], i + 1, 1., 1.5, fPlottingMarkerStyle );
		}
		
		break;
	}
	return bFound;
}

/*!

    read IRF from a root file

    might be a

    - evndisp IRF file (produced with makeEffectiveArea)
    - CTA WP MC response file

*/
bool VInstrumentResponseFunctionReader::getDataFromFile()
{
	if( fDebug )
	{
		cout << "VInstrumentResponseFunctionReader::getDataFromFile" << endl;
	}
	
	TFile* iFile = new TFile( fFile.c_str() );
	if( iFile->IsZombie() )
	{
		return false;
	}
	bool bFound = false;
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// read effective areas
	// (take into account that in combined file effective areas are in a separate file)
	TTree* t = 0;
	if( fA_MC == "A_REC" || iFile->Get( "fEffAreaH2F" ) == 0 )
	{
		t = ( TTree* )iFile->Get( "fEffArea" );
		bFound = fill_from_effectiveArea( t );
	}
	else
	{
		t = ( TTree* )iFile->Get( "fEffAreaH2F" );
		bFound = fill_from_effectiveAreaFromH2( t );
	}
	// not effective area tree - is this a CTA file?
	if( !t )
	{
		// try to read CTA file
		if( !getDataFromCTAFile() )
		{
			cout << "Error: effective area histogram not found in CTA-style file" << endl;
			return false;
		}
		else
		{
			return true;
		}
	}
	
	//////////////////////////////////////////////////////////////
	// read resolution files
	
	for( unsigned int i = 0; i < fIRF_TreeNames.size(); i++ )
	{
		cout << "reading IRF " << fIRF_TreeNames[i].c_str() << endl;
		TTree* t_a = ( TTree* )iFile->Get( fIRF_TreeNames[i].c_str() );
		fIRF_Data[i] = getIRFFromFile( t_a, i );
		if( !fIRF_Data[i] )
		{
			cout << " ...not found (index " << i << ")" << endl;
		}
		else
		{
			fIRF_Data[i]->setPlottingStyle( getPlottingColor(), getPlottingLineWidth(), getPlottingMarkerStyle(), getPlottingMarkerSize(), getPlottingFillStyle(), getPlottingLineStyle() );
			cout << " ...found" << endl;
			bFound = true;
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// now fill effective areas for 80% containment radius
	//
	// for this: read theta2 cut, get containment radius for this theta2 cut,
	//           then scale the effective areas accordingly
	
	// read gamma/hadron cuts
	VGammaHadronCuts* i_cuts = ( VGammaHadronCuts* )iFile->Get( "GammaHadronCuts" );
	if( i_cuts )
	{
		// check if theta2 graph was used
		fGammaHadronCuts_directionCut_selector = i_cuts->getDirectionCutSelector();
		// read theta2 from disk
		if( gEffArea_Rec )
		{
			double i_x = 0;
			double i_y = 0.;
			int z = 0;
			for( unsigned int i = 0; i < fIRF_TreeNames.size(); i++ )
			{
				if( fIRF_TreeNames[i] == "t_angular_resolution" && fIRF_Data[i] )
				{
					// 2D angular difference histogram
					TH2D* hRes = ( TH2D* )fIRF_Data[i]->f2DHisto[VInstrumentResponseFunctionData::E_DIFF];
					if( hRes )
					{
						z = 0;
						gEffArea_Recp80 = new TGraphAsymmErrors( 1 );
						setGraphPlottingStyle( gEffArea_Recp80, 2 );
						// loop over all energies (x-axis of hRes) and get containment level for given theta2 cut
						float i_ffactor_80p = 1.;
						for( int p = 0; p < gEffArea_Rec->GetN(); p++ )
						{
							gEffArea_Rec->GetPoint( p, i_x, i_y );
							i_ffactor_80p = i_cuts->getTheta2Cut_max( TMath::Power( 10., i_x ) );
							if( i_ffactor_80p > 0. )
							{
								i_ffactor_80p = sqrt( i_ffactor_80p );
							}
							// integrate and calculate contaiment level for this bin in log10 energy
							int i_bin = hRes->GetXaxis()->FindBin( i_x );
							double iTot = 0.;
							for( int j = 1; j <= hRes->GetNbinsY(); j++ )
							{
								iTot += hRes->GetBinContent( i_bin, j );
							}
							if( iTot < 1.e-12 )
							{
								continue;
							}
							double iSum = 0.;
							for( int j = 1; j <= hRes->GetNbinsY(); j++ )
							{
								iSum += hRes->GetBinContent( i_bin, j );
								if( hRes->GetYaxis()->GetBinCenter( j ) > i_ffactor_80p )
								{
									if( iSum / iTot > 0. )
									{
										gEffArea_Recp80->SetPoint( z, i_x, i_y * 0.8 / iSum * iTot );
										gEffArea_Recp80->SetPointEYhigh( z, gEffArea_Rec->GetErrorYhigh( p )  * 0.8 / iSum * iTot );
										gEffArea_Recp80->SetPointEYlow( z, gEffArea_Rec->GetErrorYlow( p )  * 0.8 / iSum * iTot );
										z++;
									}
									break;
								}
							}
						}
					}
					break; // found the angular resolution tree
				}
			}
		}
	}
	else
	{
		fGammaHadronCuts_directionCut_selector = 0;
	}
	
	if( !bFound )
	{
		return false;
	}
	
	return true;
}



VInstrumentResponseFunctionData* VInstrumentResponseFunctionReader::getIRFFromFile( TTree* t, unsigned int iIRFIndex )
{
	if( !t )
	{
		return 0;
	}
	
	VInstrumentResponseFunctionData* c = new VInstrumentResponseFunctionData();
	TBranch* br = t->GetBranch( "IRF" );
	br->SetAddress( &c );
	
	for( int j = 0; j < t->GetEntries(); j++ )
	{
		t->GetEntry( j );
		
		if( fDebug )
		{
			cout << "VInstrumentResponseFunctionReader::getDataFromFile (resolution data): reading event " << j << endl;
		}
		
		// check that there is data for this tree entry
		if( !c )
		{
			continue;
		}
		
		// ignore all values if there is only one entry in this tree
		if( t->GetEntries() > 1 )
		{
			// azimuth
			if( fDebug )
			{
				cout << "IRF AZ: " << j << ", found: " << c->fAz_bin << ", searched for: " << fAzbin << endl;
			}
			if( c->fAz_bin != fAzbin )
			{
				continue;
			}
			// spectral index
			if( fDebug )
			{
				cout << "IRF Index: " << j << ", found: " << c->fSpectralIndex << ", searched for: " << fIndex << endl;
			}
			if( TMath::Abs( c->fSpectralIndex - fIndex ) > 0.05 )
			{
				continue;
			}
			// wobble offset
			if( fDebug )
			{
				cout << "IRF Woff: " << j << ", found: " << c->fWobble << ", searched for: " << fWoff << endl;
			}
			if( TMath::Abs( c->fWobble - fWoff ) > 0.05 )
			{
				continue;
			}
			// noise level
			if( fDebug )
			{
				cout << "IRF Noise: " << j << ", found: " << c->fNoise << ", searched for: " << fNoise << endl;
			}
			if( c->fNoise != fNoise )
			{
				continue;
			}
			// zenith angle
			if( fDebug )
			{
				cout << "IRF Ze: " << j << ", found: " << c->fZe << ", searched for: " << fZe << endl;
			}
			if( TMath::Abs( c->fZe - fZe ) > 3. )
			{
				continue;
			}
		}
		if( c && c->fResolutionGraph.size() > 0 )
		{
			for( unsigned int r = 0; r < c->fResolutionGraph.size(); r++ )
			{
				if( c->fResolutionGraph[r] )
				{
					setGraphPlottingStyle( c->fResolutionGraph[r] );
				}
			}
		}
		
		return ( VInstrumentResponseFunctionData* )c->Clone();
		
	}
	
	return 0;
}

void VInstrumentResponseFunctionReader::getEnergyResolutionPlot68( TH2D* iP, double iMinEnergy, double iReferenceValue )
{
	if( !iP )
	{
		gEnergyResolution = 0;
		return;
	}
	
	gEnergyResolution = new TGraphErrors( 1 );
	gEnergyResolution->SetMarkerStyle( 20 );
	gEnergyResolution->SetMarkerSize( 2 );
	gEnergyResolution->SetLineWidth( 2 );
	gEnergyResolution->SetTitle( "" );
	gEnergyResolution->SetName( "" );
	setGraphPlottingStyle( gEnergyResolution );
	
	int zz = 0;
	double e_res = 0.;
	for( int b = 1; b <= iP->GetNbinsX(); b++ )
	{
		TH1D* h = iP->ProjectionY( "p_x", b, b + 1 );
		if( h && h->GetEntries() > 3. )
		{
			// calculate quantiles
			double xq[3];
			double yq[3];
			/*	    xq[0] = 0.5-0.6826895/2.;
				    xq[1] = 0.5;
				    xq[2] = 0.5+0.6826895/2.;
				    h->GetQuantiles( 3, yq, xq );
			            if( iP->GetXaxis()->GetBinCenter( b ) < iMinEnergy ) continue;
			// +-1 sigma around median
				    e_res = (yq[2]-yq[0])*0.5; */
			// 68% distribution around 1 (bb_ref, expected value)
			TH1D hh( "h", "", h->GetNbinsX(), 0., h->GetXaxis()->GetXmax() - 1. );
			double bb_ref = iReferenceValue;
			// < -998: relative to mean
			if( iReferenceValue < -998. )
			{
				bb_ref = h->GetMean();
			}
			// >  998: relative to median
			else if( iReferenceValue > 998. )
			{
				xq[0] = 0.50;
				h->GetQuantiles( 1, yq, xq );
				bb_ref = yq[0];
			}
			// fill 1D histogram before integration
			for( int bb = 1; bb <= h->GetNbinsX(); bb++ )
			{
				if( h->GetBinCenter( bb ) < bb_ref )
				{
					hh.Fill( bb_ref - h->GetBinCenter( bb ), h->GetBinContent( bb ) );
				}
				else
				{
					hh.Fill( h->GetBinCenter( bb ) - bb_ref, h->GetBinContent( bb ) );
				}
			}
			xq[0] = 0.68;
			hh.GetQuantiles( 1, yq, xq );
			e_res = yq[0];
			gEnergyResolution->SetPoint( zz, iP->GetXaxis()->GetBinCenter( b ), e_res );
			if( h->GetEntries() > 1. )
			{
				gEnergyResolution->SetPointError( zz, 0., h->GetRMS() / sqrt( h->GetEntries() - 1. ) );
			}
			else
			{
				gEnergyResolution->SetPointError( zz, 0., 0. );
			}
			zz++;
		}
	}
	return;
}

void VInstrumentResponseFunctionReader::getEnergyResolutionPlot( TH2D* iP, double iMinEnergy )
{
	if( !iP )
	{
		gEnergyResolution = 0;
		return;
	}
	
	gEnergyResolution = new TGraphErrors( 1 );
	gEnergyResolution->SetMarkerStyle( 20 );
	gEnergyResolution->SetMarkerSize( 2 );
	gEnergyResolution->SetLineWidth( 2 );
	gEnergyResolution->SetTitle( "" );
	gEnergyResolution->SetName( "" );
	setGraphPlottingStyle( gEnergyResolution );
	
	int zz = 0;
	for( int b = 1; b <= iP->GetNbinsX(); b++ )
	{
		TH1D* h = iP->ProjectionY( "p_x", b, b );
		if( h && h->GetEntries() > 10. )
		{
			if( iP->GetXaxis()->GetBinCenter( b ) < iMinEnergy )
			{
				continue;
			}
			gEnergyResolution->SetPoint( zz, iP->GetXaxis()->GetBinCenter( b ), h->GetRMS() );
			gEnergyResolution->SetPointError( zz, 0., h->GetRMS() / sqrt( h->GetEntries() - 1. ) );
			zz++;
		}
	}
	return;
}


void VInstrumentResponseFunctionReader::getEnergyResolutionPlot( TProfile* iP, int i_rebin, double iMinEnergy )
{
	if( !iP )
	{
		gEnergyResolution = 0;
		return;
	}
	
	iP->Rebin( i_rebin );
	
	gEnergyResolution = new TGraphErrors( 1 );
	gEnergyResolution->SetMarkerStyle( 20 );
	gEnergyResolution->SetMarkerSize( 2 );
	gEnergyResolution->SetLineWidth( 2 );
	gEnergyResolution->SetTitle( "" );
	gEnergyResolution->SetName( "" );
	setGraphPlottingStyle( gEnergyResolution );
	
	string iErrorOption = iP->GetErrorOption();
	
	int zz = 0;
	for( int b = 1; b <= iP->GetNbinsX(); b++ )
	{
		if( iP->GetBinEntries( b ) > 3. )
		{
			if( iP->GetXaxis()->GetBinCenter( b ) < iMinEnergy )
			{
				continue;
			}
			if( iErrorOption == "s" )
			{
				gEnergyResolution->SetPoint( zz, iP->GetXaxis()->GetBinCenter( b ), iP->GetBinError( b ) );
				if( iP->GetBinEntries( b ) > 0. )
				{
					gEnergyResolution->SetPointError( zz, 0., iP->GetBinError( b ) / sqrt( iP->GetBinEntries( b ) - 1. ) );
				}
				else
				{
					gEnergyResolution->SetPointError( zz, 0., 0. );
				}
			}
			else
			{
				gEnergyResolution->SetPoint( zz, iP->GetXaxis()->GetBinCenter( b ), iP->GetBinError( b )*sqrt( iP->GetBinEntries( b ) - 1. ) );
				gEnergyResolution->SetPointError( zz, 0., iP->GetBinError( b ) );
			}
			zz++;
		}
	}
	return;
}


bool VInstrumentResponseFunctionReader::calculateCutEfficiencies()
{
	char hname[200];
	// create histograms
	for( unsigned int i = 0; i < hCutEfficiency.size(); i++ )
	{
		if( hCutEfficiency[i] )
		{
			sprintf( hname, "%s_R", hCutEfficiency[i]->GetName() );
			hCutEfficiencyRelativePlots.push_back( ( TH1D* )hCutEfficiency[i]->Clone( hname ) );
			hCutEfficiencyRelativePlots.back()->Reset();
		}
		else
		{
			hCutEfficiencyRelativePlots.push_back( 0 );
		}
	}
	// calculate relative plots
	for( unsigned int i = 0; i < hCutEfficiencyRelativePlots.size(); i++ )
	{
		if( hCutEfficiencyRelativePlots[i] )
		{
			hCutEfficiencyRelativePlots[i]->Divide( hCutEfficiency[i], hCutEfficiency[0] );
		}
	}
	
	return true;
}

bool VInstrumentResponseFunctionReader::calculateEffectiveAreaRatios( TGraphAsymmErrors* g0 )
{
	if( !g0 )
	{
		return false;
	}
	
	gEffArea_MC_Ratio  = calculateEffectiveAreaRatios( g0, gEffArea_MC );
	gEffArea_Rec_Ratio = calculateEffectiveAreaRatios( g0, gEffArea_Rec );
	
	return true;
}

TGraphAsymmErrors* VInstrumentResponseFunctionReader::calculateEffectiveAreaRatios( TGraphAsymmErrors* g0, TGraphAsymmErrors* g1 )
{
	if( !g0 || !g1 )
	{
		return 0;
	}
	
	TGraphAsymmErrors* g = new TGraphAsymmErrors( 1 );
	
	double e = 0.;
	int z = 0;
	
	double x0 = 0.;
	double y0 = 0.;
	double y0_U = 0.;
	double y0_L = 0.;
	
	double x1 = 0.;
	double y1 = 0.;
	double y1_U = 0.;
	double y1_L = 0.;
	
	for( int f = 0; f < g1->GetN(); f++ )
	{
		g1->GetPoint( f, x1, y1 );
		y1_U = g1->GetErrorYhigh( f );
		y1_L = g1->GetErrorYlow( f );
		
		for( int k = 0; k < g0->GetN(); k++ )
		{
			//	   g0->GetPoint( k, x0, y0 );
			x0 = x1;
			y0 = g0->Eval( x0 );
			y0_U = g0->GetErrorYhigh( k );
			y0_L = g0->GetErrorYlow( k );
			
			if( y0 > 0. )
			{
				if( TMath::Abs( x0 - x1 ) < 0.001 )
				{
					g->SetPoint( z, x0, y1 / y0 );
					
					// note: standard error propagation not correct in this case
					e = y1_U * y1_U / y0 / y0 + y1 * y1 / y0 / y0 / y0 / y0 * y0_U * y0_U;
					g->SetPointEYhigh( z, sqrt( e ) );
					// (Preli)
					g->SetPointEYhigh( z, 0. );
					
					e = y1_L * y1_L / y0 / y0 + y1 * y1 / y0 / y0 / y0 / y0 * y0_L * y0_L;
					g->SetPointEYlow( z, sqrt( e ) );
					// (Preli)
					g->SetPointEYlow( z, 0. );
					
					z++;
				}
			}
		}
	}
	setGraphPlottingStyle( g );
	
	return g;
}


bool VInstrumentResponseFunctionReader::fillEffectiveAreasHistograms( TH1F* hEffRec, string iContainmentRadius, TH1F* hEff_MC )
{
	if( !hEffRec )
	{
		return false;
	}
	TGraphAsymmErrors* g = gEffArea_Rec;
	
	double i_EffAreaScaleFactor = 1.;
	// make sure that 80% containment radius effective areas are available
	if( fGammaHadronCuts_directionCut_selector > 0 && iContainmentRadius.size() > 0 )
	{
		if( iContainmentRadius == "80" )
		{
			cout << "VInstrumentResponseFunctionReader::fillEffectiveAreasHistograms() warning: " << endl;
			cout << "\t assuming that effective areas are calculated using a 68\% containment on direction" << endl;
			cout << "\t (this is hard coded!!!)" << endl;
			i_EffAreaScaleFactor = 0.80 / 0.68;
		}
	}
	else if( iContainmentRadius.size() > 0 && iContainmentRadius == "80" )
	{
		cout << "VInstrumentResponseFunctionReader::fillEffectiveAreasHistograms: ";
		cout << "found scaled effective areas for 80\% case " << endl;
		if( gEffArea_Recp80 )
		{
			g = gEffArea_Recp80;
		}
		else
		{
			return false;
		}
	}
	
	if( g )
	{
		double x = 0.;
		double y = 0.;
		for( int i = 0; i < g->GetN(); i++ )
		{
			g->GetPoint( i, x, y );
			if( y > 0. )
			{
				hEffRec->SetBinContent( hEffRec->FindBin( x ), y  * i_EffAreaScaleFactor );
				hEffRec->SetBinError( hEffRec->FindBin( x ), 0.5 * ( g->GetErrorYlow( i ) + g->GetErrorYhigh( i ) ) );
			}
		}
	}
	else
	{
		cout << "VInstrumentResponseFunctionReader::fillEffectiveAreasHistograms() warning: " << endl;
		cout << "\t no effective are graph found" << endl;
		return false;
	}
	if( gEffArea_MC && hEff_MC )
	{
		double x = 0.;
		double y = 0.;
		for( int i = 0; i < gEffArea_MC->GetN(); i++ )
		{
			gEffArea_MC->GetPoint( i, x, y );
			if( y > 0. )
			{
				hEff_MC->SetBinContent( hEff_MC->FindBin( x ), y );
				hEff_MC->SetBinError( hEff_MC->FindBin( x ), 0.5 * ( gEffArea_MC->GetErrorYlow( i ) + gEffArea_MC->GetErrorYhigh( i ) ) );
			}
		}
	}
	
	return true;
}

bool VInstrumentResponseFunctionReader::fillBiasHistograms( TH1F* h, string iMeanOrMedian )
{
	if( !h )
	{
		return false;
	}
	
	TGraphErrors* g = 0;
	if( iMeanOrMedian == "median" )
	{
		g = gEnergyBias_Median;
	}
	else if( iMeanOrMedian == "mean" )
	{
		g = gEnergyBias_Mean;
	}
	else
	{
		cout << "VInstrumentResponseFunctionReader::fillBiasHistograms warning: unknown string: " << iMeanOrMedian << endl;
		cout << "\t allowed values: mean or median" << endl;
		return false;
	}
	if( !g )
	{
		cout << "VInstrumentResponseFunctionReader::fillBiasHistograms warning: no bias graph found" << endl;
		return false;
	}
	if( g->GetN() < 2 )
	{
		cout << "VInstrumentResponseFunctionReader::fillBiasHistograms warning: bias graph with no points" << endl;
		return false;
	}
	// reset histogram binning
	double x_axis[g->GetN() + 1];
	// Obs: assume fixed binning:
	double i_binWidth = 0.5 * ( g->GetX()[1] - g->GetX()[0] );
	for( int i = 0; i < g->GetN(); i++ )
	{
		x_axis[i] = g->GetX()[i] - i_binWidth;
	}
	x_axis[g->GetN()] = g->GetX()[g->GetN() - 1] + i_binWidth;
	h->SetBins( g->GetN(), x_axis );
	// fill histogram
	double x = 0.;
	double y = 0.;
	for( int i = 0; i < g->GetN(); i++ )
	{
		g->GetPoint( i, x, y );
		h->SetBinContent( i + 1, y );
		h->SetBinError( i + 1, g->GetErrorY( i ) );
	};
	
	return true;
}



bool VInstrumentResponseFunctionReader::fillResolutionHistogram( TH1F* h, string iContainmentRadius, string iResolutionTreeName )
{
	if( !h )
	{
		return false;
	}
	
	if( iContainmentRadius != "68" )
	{
		iResolutionTreeName += "_0" + iContainmentRadius + "p";
	}
	
	for( unsigned int j = 0; j < fIRF_TreeNames.size(); j++ )
	{
		if( fIRF_TreeNames[j] == iResolutionTreeName )
		{
			if( j < fIRF_Data.size() && fIRF_Data[j] )
			{
				// try to get EVNDISP resolution graph
				TGraphErrors* g = fIRF_Data[j]->fResolutionGraph[VInstrumentResponseFunctionData::E_DIFF];
				if( iResolutionTreeName == "t_energy_resolution" )
				{
					g = gEnergyResolution;
				}
				if( g )
				{
					double x = 0.;
					double y = 0.;
					for( int i = 0; i < g->GetN(); i++ )
					{
						g->GetPoint( i, x, y );
						if( y > 0. )
						{
							h->SetBinContent( h->FindBin( x ), y );
						}
					}
				}
			}
		}
	}
	return true;
}
