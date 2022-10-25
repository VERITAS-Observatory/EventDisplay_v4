/*! \class VPlotWPPhysSensitivity


*/

#include "VPlotWPPhysSensitivity.h"
// #include "reqcurve.h"

VPlotWPPhysSensitivity::VPlotWPPhysSensitivity()
{
	fIRF = 0;
	setEnergyRange_Lin_TeV();
	setCrabSpectraFile();
	setPlotCTARequirements();
	
	fSensitivityFOM = -1.;
	fSensitivityFOM_error = -1.;
	
	fPlotCTARequirementsID = -99;
	
	fUseIntegratedSensitivityForOffAxisPlots = false;
}

void VPlotWPPhysSensitivity::reset()
{
	fData.clear();
}

bool VPlotWPPhysSensitivity::addDataSet( VSiteData* iData )
{
	fData.push_back( new VSiteData() );
	fData.back()->fSiteName = iData->fSiteName;
	fData.back()->fArray = iData->fArray;
	fData.back()->fObservationTime_s = iData->fObservationTime_s;
	fData.back()->fCameraOffset_deg = iData->fCameraOffset_deg;
	fData.back()->fSiteFile_Emin = iData->fSiteFile_Emin;
	fData.back()->fSiteFile_Emax = iData->fSiteFile_Emax;
	
	fData.back()->fPlottingColor = iData->fPlottingColor;
	fData.back()->fPlottingLineStyle = iData->fPlottingLineStyle;
	fData.back()->fPlottingFillStyle = iData->fPlottingFillStyle;
	fData.back()->fLegend = iData->fLegend;
	
	return true;
}

bool VPlotWPPhysSensitivity::addDataSet( string iAnalysis, string iSubArray, double iObservationTime_s, double iOffset_deg,
		string iLegend, int iColor, int iLineStyle, int iFillStyle )
{
	VSiteData i_temp;
	i_temp.fSiteName = iAnalysis;
	i_temp.fArray.push_back( iSubArray );
	i_temp.fObservationTime_s.push_back( iObservationTime_s );
	i_temp.fCameraOffset_deg.push_back( iOffset_deg );
	
	i_temp.fPlottingColor.push_back( iColor );
	i_temp.fPlottingLineStyle.push_back( iLineStyle );
	i_temp.fPlottingFillStyle.push_back( iFillStyle );
	i_temp.fLegend.push_back( iLegend );
	
	return addDataSet( &i_temp );
}

bool VPlotWPPhysSensitivity::addDataSets( string iDataSettxtFile, string iDirectionString )
{
	std::cout << "VPlotWPPhysSensitivity::addDataSets " << std::endl;
	unsigned int z_site = 0;
	for( ;; )
	{
		cout << "A " << z_site << endl;
		fData.push_back( new VSiteData() );
		if( !fData.back()->addDataSet( iDataSettxtFile, z_site, iDirectionString ) )
		{
			fData.pop_back();
			break;
		}
		z_site++;
	}
	
	if( fData.size() == 0 )
	{
		return false;
	}
	
	return true;
}


/*

   plot IRFs for all data sets

*/
bool VPlotWPPhysSensitivity::plotIRF( string iPrint, double iEffAreaMin, double iEffAreaMax, double iEnergyResolutionMax, TPad* iEffAreaPad, TPad* iAngResPad, TPad* iEResPad )
{
	fIRF = new VPlotInstrumentResponseFunction();
	
	fIRF->setCanvasSize( 400, 400 );
	fIRF->setPlottingAxis( "energy_Lin", "X", true, fMinEnergy_TeV, fMaxEnergy_TeV, "energy [TeV]" );
	fIRF->setPlottingAxis( "effarea_Lin", "X", true, iEffAreaMin, iEffAreaMax );
	fIRF->setPlottingAxis( "energyresolution_Lin", "X", false, 0., 0.7 );
	
	for( unsigned int i = 0; i < fData.size(); i++ )
	{
		if( fData[i] )
		{
			for( unsigned int j = 0; j < fData[j]->fSiteFileName.size(); j++ )
			{
				if( fData[i]->fSiteFile_exists[j] )
				{
					fIRF->addInstrumentResponseData( fData[i]->fSiteFileName[j], 20., fData[i]->fCameraOffset_deg[j],
													 0, 2.4, 200, "A_MC", fData[i]->fPlottingColor[j], 3,
													 21, 0.75, fData[i]->fSiteFile_Emin[j], fData[i]->fSiteFile_Emax[j] );
				}
			}
		}
	}
	
	char hname[2000];
	////////////////////////////
	// effective areas
	TCanvas* c = fIRF->plotEffectiveArea( -1., 2.e7, iEffAreaPad );
	plotLegend( c, true );
	if( iPrint.size() > 0 )
	{
		sprintf( hname, "%s-EffArea.pdf", iPrint.c_str() );
		if( c )
		{
			c->Print( hname );
		}
	}
	////////////////////////////
	// angular resolution (68%)
	c = fIRF->plotAngularResolution();
	plotLegend( c, false );
	if( iPrint.size() > 0 )
	{
		sprintf( hname, "%s-AngRes.pdf", iPrint.c_str() );
		if( c )
		{
			c->Print( hname );
		}
	}
	// angular resolution (80%)
	c = fIRF->plotAngularResolution( "energy", "80", -1.e99, iAngResPad );
	plotLegend( c, false );
	////////////////////////////
	// energy resolution
	c = fIRF->plotEnergyResolution( iEnergyResolutionMax, iEResPad );
	plotLegend( c, false );
	if( iPrint.size() > 0 )
	{
		sprintf( hname, "%s-ERes.pdf", iPrint.c_str() );
		if( c )
		{
			c->Print( hname );
		}
	}
	// energy bias
	c = fIRF->plotEnergyReconstructionBias( "mean", -0.5, 0.5 );
	plotLegend( c, false );
	if( iPrint.size() > 0 )
	{
		sprintf( hname, "%s-EBias.pdf", iPrint.c_str() );
		if( c )
		{
			c->Print( hname );
		}
	}
	
	return true;
}

void VPlotWPPhysSensitivity::initialProjectedSensitivityPlots()
{
	fProjectionEnergy_min_logTeV.clear();
	fProjectionEnergy_max_logTeV.clear();
	// (hard coded energies here...not good)
	if( !fUseIntegratedSensitivityForOffAxisPlots )
	{
		fProjectionEnergy_min_logTeV.push_back( log10( 10.0 ) );
		fProjectionEnergy_max_logTeV.push_back( log10( 10.0 ) );
		fProjectionEnergy_min_logTeV.push_back( log10( 1.0 ) );
		fProjectionEnergy_max_logTeV.push_back( log10( 3.0 ) );
		fProjectionEnergy_min_logTeV.push_back( log10( 0.4 ) );
		fProjectionEnergy_max_logTeV.push_back( log10( 0.6 ) );
		fProjectionEnergy_min_logTeV.push_back( log10( 0.10 ) );
		fProjectionEnergy_max_logTeV.push_back( log10( 0.125 ) );
	}
	// integrated sensitivity
	else
	{
		fProjectionEnergy_min_logTeV.push_back( log10( 0.128 ) ); // choose 128 GeV the be at the lower end of the corresponding bin on the log axis)
		fProjectionEnergy_max_logTeV.push_back( log10( 0.128 ) );
	}
	// graphs
	for( unsigned int i = 0; i < fData.size(); i++ )
	{
		if( fProjectionSensitivityvsCameraOffset.find( fData[i]->fReferenceSiteName ) == fProjectionSensitivityvsCameraOffset.end() )
		{
			char hname[200];
			for( unsigned int j = 0; j < fProjectionEnergy_min_logTeV.size(); j++ )
			{
				fProjectionSensitivityvsCameraOffset[fData[i]->fReferenceSiteName].push_back( new TGraphAsymmErrors( ) );
				sprintf( hname, "fProjectionSensitivityvsCameraOffset_%s_%d", fData[i]->fReferenceSiteName.c_str(), j );
				fProjectionSensitivityvsCameraOffset[fData[i]->fReferenceSiteName].back()->SetName( hname );
				fProjectionSensitivityvsCameraOffset[fData[i]->fReferenceSiteName].back()->SetTitle( "" );
				
				setGraphPlottingStyle( fProjectionSensitivityvsCameraOffset[fData[i]->fReferenceSiteName].back(), i + 1, 1., 20, 1.5 );
			}
		}
	}
}


void VPlotWPPhysSensitivity::fillProjectedSensitivityPlot( unsigned int iDataSet, TGraphAsymmErrors* g )
{
	if( !g )
	{
		return;
	}
	if( iDataSet >= fData.size() )
	{
		return;
	}
	
	if( fProjectionSensitivityvsCameraOffset.find( fData[iDataSet]->fReferenceSiteName ) == fProjectionSensitivityvsCameraOffset.end() )
	{
		return;
	}
	
	if( g )
	{
		VHistogramUtilities h;
		//////////////////////////////
		// loop over all energy bins
		for( unsigned int i = 0; i < fProjectionEnergy_min_logTeV.size(); i++ )
		{
			if( i < fProjectionSensitivityvsCameraOffset[fData[iDataSet]->fReferenceSiteName].size()
					&& fProjectionSensitivityvsCameraOffset[fData[iDataSet]->fReferenceSiteName][i] )
			{
				int z = fProjectionSensitivityvsCameraOffset[fData[iDataSet]->fReferenceSiteName][i]->GetN();
				int iBin_min = h.findBinInGraph( g, fProjectionEnergy_min_logTeV[i] );
				int iBin_max = h.findBinInGraph( g, fProjectionEnergy_max_logTeV[i] );
				double i_m = 0.;
				double i_m_lE = 0.;
				double i_m_hE = 0.;
				double i_mz = 0.;
				// average over energies
				for( int b = iBin_min; b <= iBin_max; b++ )
				{
					if( b >= 0 )
					{
						// check that this is not below or above the highest bin
						double x = 0.;
						double y = 0.;
						g->GetPoint( b, x, y );
						
						i_m += y;
						i_m_lE += g->GetErrorYlow( b ) * g->GetErrorYlow( b );
						i_m_hE += g->GetErrorYhigh( b ) * g->GetErrorYhigh( b );
						i_mz++;
					}
				}
				if( i_mz > 0 && fData[iDataSet]->fCameraOffset_deg.size() > 0 )
				{
					fProjectionSensitivityvsCameraOffset[fData[iDataSet]->fReferenceSiteName][i]->SetPoint( z, fData[iDataSet]->fCameraOffset_deg[0], i_m / i_mz );
					fProjectionSensitivityvsCameraOffset[fData[iDataSet]->fReferenceSiteName][i]->SetPointEYhigh( z, sqrt( i_m_hE / i_mz ) );
					fProjectionSensitivityvsCameraOffset[fData[iDataSet]->fReferenceSiteName][i]->SetPointEYlow( z, sqrt( i_m_lE / i_mz ) );
				}
			}
		}
	}
}

TCanvas* VPlotWPPhysSensitivity::plotProjectedSensitivities( TCanvas* c, double iMaxOffSet, int iColor )
{
	TCanvas* cC = 0;
	if( c )
	{
		cC = c;
	}
	else
	{
		cC = new TCanvas( "cCProjection", "projected off-axis sensitivities", 800, 100, 400, 400 );
		cC->SetGridx( 0 );
		cC->SetGridy( 0 );
		cC->SetLeftMargin( 0.13 );
		cC->Draw();
		TH1D* hnull = new TH1D( "hnull", "", 10, 0., iMaxOffSet + 0.5 );
		hnull->SetStats( 0 );
		hnull->SetXTitle( "distance to camera center [deg]" );
		hnull->SetYTitle( "relative sensitivity" );
		hnull->SetMinimum( 0. );
		hnull->SetMaximum( 1.5 );
		hnull->Draw();
		TLine* iLi = new TLine( hnull->GetXaxis()->GetXmin(), 1., hnull->GetXaxis()->GetXmax(), 1. );
		iLi->Draw();
	}
	cC->cd();
	
	TLegend* iL = new TLegend( 0.15, 0.15, 0.4, 0.4 );
	char hname[200];
	
	map< string, vector< TGraphAsymmErrors* > >::iterator i_fProjectionSensitivityvsCameraOffset_iter;
	
	for( i_fProjectionSensitivityvsCameraOffset_iter = fProjectionSensitivityvsCameraOffset.begin();
			i_fProjectionSensitivityvsCameraOffset_iter != fProjectionSensitivityvsCameraOffset.end();
			i_fProjectionSensitivityvsCameraOffset_iter++ )
	{
		for( unsigned int i = 0; i < fProjectionEnergy_min_logTeV.size(); i++ )
		{
			if( i < i_fProjectionSensitivityvsCameraOffset_iter->second.size() && i_fProjectionSensitivityvsCameraOffset_iter->second[i] )
			{
				TGraphAsymmErrors* iGraph = i_fProjectionSensitivityvsCameraOffset_iter->second[i];
				iGraph->Print();
				// normalize graphs to average of first two points
				double x = 0.;
				double y = 0.;
				double y_norm = 0.;
				iGraph->GetPoint( 0, x, y_norm );
				iGraph->GetPoint( 0, x, y );
				y_norm = 0.5 * ( y + y_norm );
				double y_normE_low  = sqrt( iGraph->GetErrorYlow( 0 ) * iGraph->GetErrorYlow( 0 ) );
				double y_normE_high = sqrt( iGraph->GetErrorYhigh( 0 ) * iGraph->GetErrorYhigh( 0 ) );
				
				for( int p = 0; p < iGraph->GetN(); p++ )
				{
					iGraph->GetPoint( p, x, y );
					if( y > 0. )
					{
						iGraph->SetPoint( p, x, y_norm / y );
					}
					else
					{
						iGraph->SetPoint( p, x, 0. );
					}
					iGraph->SetPointEYhigh( p, VMathsandFunctions::getRatioError( y_norm, y, y_normE_high, iGraph->GetErrorYhigh( p ) ) );
					iGraph->SetPointEYlow( p, VMathsandFunctions::getRatioError( y_norm, y, y_normE_low, iGraph->GetErrorYlow( p ) ) );
				}
				setGraphPlottingStyle( iGraph, i + 1, 1., 20, 1.0 );
				iGraph->Draw( "pl" );
				sprintf( hname, "%.2f TeV", TMath::Power( 10., fProjectionEnergy_min_logTeV[i] ) );
				iL->AddEntry( iGraph, hname, "lp" );
			}
		}
	}
	//
	if( iColor < 0 && fProjectionEnergy_min_logTeV.size() > 1 )
	{
		iL->Draw();
	}
	
	return cC;
}

bool VPlotWPPhysSensitivity::plotSensitivityRatio( string iPrint, double ymin, double ymax, bool iRatoToGoal, TPad* iSensRatio )
{
	char hname[200];
	char htitle[200];
	sprintf( hname, "cSensRatio_%d", ( int )iRatoToGoal );
	if( iRatoToGoal )
	{
		sprintf( htitle, "ratio of sensitivities (to goal)" );
	}
	else
	{
		sprintf( htitle, "ratio of sensitivities (to requirement)" );
	}
	TCanvas* cSensRatio = 0;
	if( !iSensRatio )
	{
		cSensRatio = new TCanvas( hname, htitle, 200, 200, 900, 600 );
		cSensRatio->SetGridy( 0 );
		cSensRatio->SetGridx( 0 );
	}
	else
	{
		cSensRatio = ( TCanvas* )iSensRatio;
	}
	cSensRatio->cd();
	
	sprintf( hname, "hSensRatio_%d", ( int )iRatoToGoal );
	TH1D* hNull = new TH1D( hname, "", 10, log10( fMinEnergy_TeV ), log10( fMaxEnergy_TeV ) );
	hNull->SetStats( 0 );
	hNull->SetXTitle( "log_{10} energy [TeV]" );
	if( iRatoToGoal )
	{
		hNull->SetYTitle( "Sensitivity ratio to goal sensitivity" );
	}
	else
	{
		hNull->SetYTitle( "Sensitivity ratio to required sensitivity" );
	}
	hNull->SetMinimum( ymin );
	hNull->SetMaximum( ymax );
	hNull->Draw( "" );
	hNull->Draw( "AH" );
	
	plot_nullHistogram( cSensRatio, hNull, true, false, 1.2, fMinEnergy_TeV, fMaxEnergy_TeV );
	
	TLine* iL = new TLine( log10( fMinEnergy_TeV ), 1., log10( fMaxEnergy_TeV ), 1. );
	iL->SetLineStyle( 2 );
	iL->SetLineWidth( 3 );
	iL->Draw();
	
	TGraph* gRelG = 0;
	
	// loop over all data sets and divide it by the first
	for( unsigned int i = 0; i < fData.size(); i++ )
	{
		if( !fData[i] )
		{
			continue;
		}
		
		for( unsigned int j = 0; j < fData[i]->fGraphSensitivity.size(); j++ )
		{
			if( fData[i]->fGraphSensitivity[j] )
			{
				TGraphAsymmErrors* g = new TGraphAsymmErrors();
				if( gRelG )
				{
					VHistogramUtilities::divide( g, fData[i]->fGraphSensitivity[j], gRelG );
				}
				if( g->GetN() > 0 )
				{
					setGraphPlottingStyle( g, fData[i]->fGraphSensitivity[j]->GetMarkerColor(), 1., 20., 0.5, fData[i]->fPlottingFillStyle[j], fData[i]->fPlottingLineStyle[j] );
					g->Draw( "p" );
				}
			}
		}
	}
	// plot goal sensitivity
	if( fPlotCTARequirementGoals )
	{
		TGraphAsymmErrors* gRelGoal = 0;
		if( gRelGoal )
		{
			TGraphAsymmErrors* g = new TGraphAsymmErrors();
			if( gRelG )
			{
				VHistogramUtilities::divide( g, gRelGoal, gRelG );
			}
			if( g->GetN() > 0 )
			{
				g->SetLineStyle( 2 );
				g->SetLineColor( kGray );
				g->Draw( "l" );
			}
		}
	}
	if( cSensRatio )
	{
		plotLegend( cSensRatio, false, false );
		// print results
		if( iPrint.size() > 0 )
		{
			char hname[2000];
			if( iRatoToGoal )
			{
				sprintf( hname, "%s-SensitivityRatioGoal.pdf", iPrint.c_str() );
			}
			else
			{
				sprintf( hname, "%s-SensitivityRatio.pdf", iPrint.c_str() );
			}
			if( cSensRatio )
			{
				cSensRatio->Print( hname );
			}
		}
	}
	
	return true;
}

void VPlotWPPhysSensitivity::printSensitivityFigureOfMerit( double iEmin_TeV, double iEmax_TeV )
{
	for( unsigned int i = 0; i < fData.size(); i++ )
	{
		if( fData[i] )
		{
			for( unsigned int j = 0; j < fData[i]->fGraphSensitivity.size(); j++ )
			{
				if( fData[i]->fGraphSensitivity[j] )
				{
					printSensitivityFigureOfMerit( fData[i]->fGraphSensitivity[j], iEmin_TeV, iEmax_TeV, fData[i]->fSiteName );
				}
			}
		}
	}
}

void VPlotWPPhysSensitivity::printSensitivityFigureOfMerit( TGraphAsymmErrors* gSensitivity, double iEmin_TeV, double iEmax_TeV, string iAnalysis )
{
	if( !gSensitivity )
	{
		return;
	}
	return;
	
	iEmin_TeV = log10( iEmin_TeV );
	iEmax_TeV = log10( iEmax_TeV );
	
	double m = 1.;
	double dm = 0.;
	double z = 0.;
	double x = 0.;
	double y = 0.;
	double dy = 0.;
	double req = 0.;
	
	for( int p = 0; p < gSensitivity->GetN(); p++ )
	{
		gSensitivity->GetPoint( p, x, y );
		dy = 0.5 * ( gSensitivity->GetErrorYlow( p ) + gSensitivity->GetErrorYhigh( p ) );
		// excluding the lower bin containing iEmin_TeV, including the bin with iEmax_TeV
		if( iEmin_TeV < x - gSensitivity->GetErrorX( p )
				&& iEmax_TeV > x )
		{
			if( y > 0 )
			{
				m  *= req / y;
				dm += dy * dy * req * req / y / y / y / y;
			}
			
			z++;
		}
	}
	if( z > 0 )
	{
		m = TMath::Power( m, 1. / z );
		dm = m * sqrt( dm );
		dm = 1. / z * TMath::Power( m, 1. / z - 1. ) * dm;
		cout << "PPUT " << iAnalysis << " (calculated from sensitivity, Req ID" << fPlotCTARequirementsID << ", ";
		cout << z << " points), energy range [";
		cout <<  TMath::Power( 10., iEmin_TeV ) << ", " << TMath::Power( 10., iEmax_TeV ) << "]: ";
		cout << "\t PPUT = " << std::setprecision( 2 ) << std::fixed << m << "+-" << dm << endl;
		fSensitivityFOM = m;
		fSensitivityFOM_error = dm;
	}
}


/*

    plot sensitivities and data rates for different data sets

*/
bool VPlotWPPhysSensitivity::plotSensitivity( string iPrint, double iMinSensitivity, double iMaxSensitivity, string iUnit, TPad* iSensitivityPad, TPad* iSensitivityRatioPad, TPad* iBckPad )
{
	TCanvas* cIntSens = 0;
	TCanvas* cSens = 0;
	TCanvas* cSensInter = ( TCanvas* )iSensitivityPad;
	TCanvas* cBck = ( TCanvas* )iBckPad;
	
	initialProjectedSensitivityPlots();
	
	////////////////////////////////////////////////////////
	// loop over all data sets
	unsigned int z = 0;
	for( unsigned int i = 0; i < fData.size(); i++ )
	{
		if( !fData[i] || !fData[i]->checkIntegrity() )
		{
			continue;
		}
		for( unsigned j = 0; j < fData[i]->fSiteFileName.size(); j++ )
		{
			cout << "NOW AT " << fData[i]->fSiteFileName[j] << ", " << fData[i]->fCameraOffset_deg[j] << endl;
			VSensitivityCalculator* a = new VSensitivityCalculator();
			a->setMonteCarloParametersCTA_MC( fData[i]->fSiteFileName[j], fData[i]->fCameraOffset_deg[j], fCrabSpectraFile, fCrabSpectraID );
			a->setEnergyRange_Lin( fMinEnergy_TeV, fMaxEnergy_TeV );
			a->setPlotCanvasSize( 900, 600 );
			a->setPlottingStyle( fData[i]->fPlottingColor[j], fData[i]->fPlottingLineStyle[j], 2., 1, 2., fData[i]->fPlottingFillStyle[j] );
			if( iUnit == "ENERGY" )
			{
				a->setFluxRange_ENERG( iMinSensitivity, iMaxSensitivity );
			}
			else if( iUnit == "CU" )
			{
				a->setFluxRange_CU( iMinSensitivity, iMaxSensitivity );
			}
			TCanvas* c_temp = 0;
			// cSens name = cSensitivity (default from VSensitivityCalculator::fPlot_CanvasName )
			c_temp = a->plotDifferentialSensitivityvsEnergyFromCrabSpectrum( cSens, "CTA-PHYS", fData[i]->fPlottingColor[j], iUnit,
					 0.2, fData[i]->fSiteFile_Emin[j], fData[i]->fSiteFile_Emax[j] );
			if( c_temp )
			{
				cSens = c_temp;
			}
			// cIntSens name (changing VSensitivityCalculator::fPlot_CanvasName default value)
			//	 a->setPlotCanvasName("cIntegratedSensitivity" );
			//	 c_temp = a->plotIntegralSensitivityvsEnergyFromCrabSpectrum( cIntSens, "CTA-PHYS", fData[i]->fPlottingColor[j], iUnit,
			//	  		                                                  fData[i]->fSiteFile_Emin[j], fData[i]->fSiteFile_Emax[j] );
			//	 if( c_temp ) cIntSens = c_temp;
			c_temp = a->plotSignalBackgroundRates( cBck, ( z == 0 ), 2.e-7, 14. ); // plot also protons and electrons
			if( c_temp )
			{
				cBck = c_temp;
			}
			fData[i]->fGraphSensitivity[j] = ( TGraphAsymmErrors* )a->getSensitivityGraph();
			z++;
		}
		////////////////////////////////////////////////////////
		// plot a second window with interpolated sensitivities
		TGraphAsymmErrors* iGraphSensitivity = fData[i]->getCombinedSensitivityGraph( true, "" );
		if( iGraphSensitivity )
		{
			VSensitivityCalculator* b = new VSensitivityCalculator();
			b->setMonteCarloParametersCTA_MC( "", fData[i]->fCameraOffset_deg[0], fCrabSpectraFile, fCrabSpectraID );
			b->setSensitivityGraph( iGraphSensitivity );
			b->setPlotCanvasSize( 900, 600 );
			b->setPlotCanvasName( "cSensitivityInterpolated", "sensitivity vs energy (interpolated)" );
			b->setFluxRange_ENERG( iMinSensitivity, iMaxSensitivity );
			if( fData[i]->fPlottingColor.size() > 0 )
			{
				b->setPlottingStyle( fData[i]->fPlottingColor[0], fData[i]->fPlottingLineStyle[0], 2., 1, 2., fData[i]->fPlottingFillStyle[0] );
			}
			cSensInter = b->plotSensitivityvsEnergyFromCrabSpectrum( cSensInter, i + 1, iUnit, 0.2, ( i == 0 ) );
			if( fUseIntegratedSensitivityForOffAxisPlots )
			{
				TGraphAsymmErrors* iGraphIntegratedSensitivity = fData[i]->getCombinedSensitivityGraph( true, "", true );
				fillProjectedSensitivityPlot( i, iGraphIntegratedSensitivity );
			}
			else
			{
				fillProjectedSensitivityPlot( i, iGraphSensitivity );
			}
		}
	}
	/////////////////////////////
	// print results
	if( cIntSens )
	{
		plotLegend( cIntSens, false );
		if( iPrint.size() > 0 )
		{
			char hname[2000];
			sprintf( hname, "%s-IntegratedSensitivity.pdf", iPrint.c_str() );
			cIntSens->Print( hname );
		}
	}
	if( cSens )
	{
		plotLegend( cSens, false );
		if( iPrint.size() > 0 )
		{
			char hname[2000];
			sprintf( hname, "%s-Sensitivity.pdf", iPrint.c_str() );
			cSens->Print( hname );
		}
	}
	if( cSensInter )
	{
		plotLegend( cSensInter, false );
		if( iPrint.size() > 0 )
		{
			char hname[2000];
			sprintf( hname, "%s-SensitivityInter.pdf", iPrint.c_str() );
			cSensInter->Print( hname );
		}
	}
	if( cBck )
	{
		plotLegend( cBck, false );
		if( iPrint.size() > 0 )
		{
			char hname[2000];
			sprintf( hname, "%s-BRates.pdf", iPrint.c_str() );
			if( cBck )
			{
				cBck->Print( hname );
			}
		}
	}
	
	return true;
}

bool VPlotWPPhysSensitivity::plotLegend( TCanvas* c, bool iDown, bool iLeft, bool iAddFirst )
{
	if( !c )
	{
		return false;
	}
	c->cd();
	
	double x = 0.2 + 0.35;
	if( iLeft )
	{
		x = 0.15;
	}
	double y = 0.65;
	if( iDown )
	{
		y -= 0.50;
	}
	double y_yp = y + 0.18;
	if( fData.size() == 2 )
	{
		y_yp -= 0.1;
	}
	TLegend* iL = new TLegend( x, y, x + 0.30, y_yp );
	iL->SetFillColor( 0 );
	
	for( unsigned int i = 0; i < fData.size(); i++ )
	{
		if( i == 0 && !iAddFirst )
		{
			continue;
		}
		for( unsigned int j = 0; j < fData[i]->fSiteFile_exists.size(); j++ )
		{
			if( fData[i]->fSiteFile_exists[j] && fData[i]->fLegend.size() > 0 )
			{
				TGraph* g = new TGraph( 1 );
				g->SetLineColor( fData[i]->fPlottingColor[j] );
				g->SetLineStyle( fData[i]->fPlottingLineStyle[j] );
				g->SetFillStyle( fData[i]->fPlottingFillStyle[j] );
				g->SetFillColor( fData[i]->fPlottingColor[j] );
				g->SetMarkerColor( fData[i]->fPlottingColor[j] );
				g->SetMarkerStyle( 1 );
				if( fData[i]->fLegend[j].size() > 0 && fData[i]->fLegend[j].find( "NO_LEGEND" ) == string::npos )
				{
					iL->AddEntry( g, fData[i]->fLegend[j].c_str(), "lF" );
				}
			}
		}
	}
	if( iL->GetNRows() > 0 )
	{
		iL->Draw();
	}
	return true;
}


bool VPlotWPPhysSensitivity::setPlotCTARequirements( int iRequirementID, bool iPlotRequirementGoals )
{
	fPlotCTARequirementGoals = iPlotRequirementGoals;
	fPlotCTARequirementsID = iRequirementID;
	
	return false;
}

