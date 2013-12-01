/*! \class VPlotWPPhysSensitivity

*/

#include "VPlotWPPhysSensitivity.h"
// #include "reqcurve.h"

VPlotWPPhysSensitivity::VPlotWPPhysSensitivity()
{
   fIRF = 0;
   setEnergyRange_Lin_TeV();
   setCrabSpectraFile();
   fPlotCTARequirements = 0;
   setPlotCTARequirements();

   fSensitivityFOM = -1.;
   fSensitivityFOM_error = -1.;

   fPlotCTARequirementsID = -99;
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

/*

   plot IRFs for all data sets

*/
bool VPlotWPPhysSensitivity::plotIRF( string iPrint, double iEffAreaMin, double iEffAreaMax, double iEnergyResolutionMax )
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
	     if( fData[i]->fSiteFile_exists[j])
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
    TCanvas *c = fIRF->plotEffectiveArea( 2.e7 );
    plotLegend( c, true );
    if( iPrint.size() > 0 )
    {
       sprintf( hname, "%s-EffArea.eps", iPrint.c_str() );
       if( c ) c->Print( hname );
    }
    if( fPlotCTARequirementsID >= 0 && fPlotCTARequirements )
    {
// effective area requirements are all goals
       fPlotCTARequirements->plotRequirement_EffectiveArea( c, true );
    }
////////////////////////////
// angular resolution (68%)
    c = fIRF->plotAngularResolution();
    plotLegend( c, false );
    if( iPrint.size() > 0 )
    {
       sprintf( hname, "%s-AngRes.eps", iPrint.c_str() );
       if( c ) c->Print( hname );
    }
    if( fPlotCTARequirementsID >= 0 && fPlotCTARequirements )
    {
       fPlotCTARequirements->plotRequirement_AngularResolution( c, fPlotCTARequirementGoals );
    }
// angular resolution (80%)
    c = fIRF->plotAngularResolution( "energy", "80" );
    plotLegend( c, false );
////////////////////////////
// energy resolution
    c = fIRF->plotEnergyResolution( iEnergyResolutionMax );
    plotLegend( c, false );
    if( iPrint.size() > 0 )
    {
       sprintf( hname, "%s-ERes.eps", iPrint.c_str() );
       if( c ) c->Print( hname );
    }
    if( fPlotCTARequirementsID >= 0 && fPlotCTARequirements )
    {
       fPlotCTARequirements->plotRequirement_EnergyResolution( c, fPlotCTARequirementGoals );
    }
// energy bias
    c = fIRF->plotEnergyReconstructionBias( "mean", -0.5, 0.5 );
    plotLegend( c, false );
    if( iPrint.size() > 0 )
    {
       sprintf( hname, "%s-EBias.eps", iPrint.c_str() );
       if( c ) c->Print( hname );
    }

    return true;
}

void VPlotWPPhysSensitivity::initialProjectedSensitivityPlots()
{
// (hard coded energies here...not good)
//   fProjectionEnergy_min_logTeV.push_back( log10( 10.001 ) );   fProjectionEnergy_max_logTeV.push_back( log10( 10.001 ) );
   fProjectionEnergy_min_logTeV.push_back( log10( 1.0 ) );       fProjectionEnergy_max_logTeV.push_back( log10( 10. ) );  
//   fProjectionEnergy_min_logTeV.push_back( log10( 10.0 ) );       fProjectionEnergy_max_logTeV.push_back( log10( 110. ) );  
//   fProjectionEnergy_min_logTeV.push_back( log10( 0.08 ) );     fProjectionEnergy_max_logTeV.push_back( log10( 0.08 ) ); 
 
   char hname[200];
   for( unsigned int i = 0; i < fProjectionEnergy_min_logTeV.size(); i++ )
   {
       fProjectionSensitivityvsCameraOffset.push_back( new TGraphAsymmErrors( ) );
       sprintf( hname, "fProjectionSensitivityvsCameraOffset_%d", i );
       fProjectionSensitivityvsCameraOffset.back()->SetName( hname );
       fProjectionSensitivityvsCameraOffset.back()->SetTitle( "" );

       setGraphPlottingStyle( fProjectionSensitivityvsCameraOffset.back(), i+1, 1., 20, 1.5 );
   }
}


void VPlotWPPhysSensitivity::fillProjectedSensitivityPlot( unsigned int iDataSet, VSensitivityCalculator *a )
{
   if( !a ) return;
   if( iDataSet >= fData.size() ) return;

   TGraphAsymmErrors *g = a->getSensitivityGraph();
   if( a )
   {
       VHistogramUtilities h;
       for( unsigned int i = 0; i < fProjectionEnergy_min_logTeV.size(); i++ )
       {
          if( i < fProjectionSensitivityvsCameraOffset.size() && fProjectionSensitivityvsCameraOffset[i] )
	  {
	      int z = fProjectionSensitivityvsCameraOffset[i]->GetN();
              int iBin_min = h.findBinInGraph( g, fProjectionEnergy_min_logTeV[i] );
	      int iBin_max = h.findBinInGraph( g, fProjectionEnergy_max_logTeV[i] );
	      double i_m = 0.;
	      double i_m_lE = 0.;
	      double i_m_hE = 0.;
	      double i_mz = 0.;
	      for( int b = iBin_min; b <= iBin_max; b++ )
	      {
		 if( b >= 0 )
		 {
// check that this is not below or above the highest bin
		     double x = 0.;
		     double y = 0.;
		     g->GetPoint( b, x, y );

		     i_m += y;
		     i_m_lE += g->GetErrorYlow( b )*g->GetErrorYlow( b );
		     i_m_hE += g->GetErrorYhigh( b)*g->GetErrorYhigh( b);
		     i_mz++;
                  }
               }
	       if( i_mz > 0 && fData[iDataSet]->fCameraOffset_deg.size() > 0 )
	       {
		  fProjectionSensitivityvsCameraOffset[i]->SetPoint( z, fData[iDataSet]->fCameraOffset_deg[0], i_m / i_mz );
		  fProjectionSensitivityvsCameraOffset[i]->SetPointEYhigh( z, sqrt( i_m_hE / i_mz ) );
		  fProjectionSensitivityvsCameraOffset[i]->SetPointEYlow( z, sqrt( i_m_lE / i_mz ) );
               }
          }
       }
   }
}

TCanvas* VPlotWPPhysSensitivity::plotProjectedSensitivities( TCanvas *c, double iMaxOffSet, int iColor )
{
    TCanvas *cC = 0;
    if( c ) cC = c;
    else
    {
        cC = new TCanvas( "cCProjection", "projected off axis sensitivities", 800, 100, 400, 400 );
	cC->SetGridx( 0 );
	cC->SetGridy( 0 );
	cC->SetLeftMargin( 0.13 );
	cC->Draw();
	TH1D *hnull = new TH1D( "hnull", "", 10, 0., iMaxOffSet + 0.5 );
	hnull->SetStats( 0 );
	hnull->SetXTitle( "distance to camera center [deg]" );
	hnull->SetYTitle( "relative sensitivity" );
	hnull->SetMinimum( 0. );
	hnull->SetMaximum( 1.3 );
	hnull->Draw();
        TLine *iLi = new TLine( hnull->GetXaxis()->GetXmin(), 1., hnull->GetXaxis()->GetXmax(), 1. );
	iLi->Draw();
    }
    cC->cd();

    TLegend *iL = new TLegend( 0.15, 0.15, 0.4, 0.4 );
    char hname[200];

    unsigned int z = 0;
    for( unsigned int i = 0; i < fProjectionEnergy_min_logTeV.size(); i++ )
    {
        cout << "Plotting projected sensitivity for energy " << fProjectionEnergy_min_logTeV[i] << endl;
	if( i < fProjectionSensitivityvsCameraOffset.size() && fProjectionSensitivityvsCameraOffset[i] )
	{
// normalize graphs to average of first two points
	    if( fProjectionSensitivityvsCameraOffset[i]->GetN() > 2 )
	    {
		double x = 0.;
		double y = 0.;
	        double y_norm = 0.;
		fProjectionSensitivityvsCameraOffset[i]->GetPoint( 0, x, y_norm );
		fProjectionSensitivityvsCameraOffset[i]->GetPoint( 0, x, y );
//		fProjectionSensitivityvsCameraOffset[i]->GetPoint( 1, x, y );
		y_norm = 0.5*( y + y_norm );
		double y_normE_low = sqrt( fProjectionSensitivityvsCameraOffset[i]->GetErrorYlow( 0 )
		                          *fProjectionSensitivityvsCameraOffset[i]->GetErrorYlow( 0 )
//					 + fProjectionSensitivityvsCameraOffset[i]->GetErrorYlow( 1 )
//					 * fProjectionSensitivityvsCameraOffset[i]->GetErrorYlow( 1 )
					 );
		double y_normE_high = sqrt( fProjectionSensitivityvsCameraOffset[i]->GetErrorYhigh( 0 )
		                          *fProjectionSensitivityvsCameraOffset[i]->GetErrorYhigh( 0 )
//					 + fProjectionSensitivityvsCameraOffset[i]->GetErrorYhigh( 1 )
//					 * fProjectionSensitivityvsCameraOffset[i]->GetErrorYhigh( 1 )
					 );

	        for( int p = 0; p < fProjectionSensitivityvsCameraOffset[i]->GetN(); p++ )
	        {
		    fProjectionSensitivityvsCameraOffset[i]->GetPoint( p, x, y );
		    if( y > 0. ) fProjectionSensitivityvsCameraOffset[i]->SetPoint( p, x, y_norm / y );
		    else         fProjectionSensitivityvsCameraOffset[i]->SetPoint( p, x, 0. );
		    fProjectionSensitivityvsCameraOffset[i]->SetPointEYhigh( p, 
		             VMathsandFunctions::getRatioError( y_norm, y, y_normE_high,
			                                                   fProjectionSensitivityvsCameraOffset[i]->GetErrorYhigh( p ) ) );
		    fProjectionSensitivityvsCameraOffset[i]->SetPointEYlow( p, 
		             VMathsandFunctions::getRatioError( y_norm, y, y_normE_low,
			                                                   fProjectionSensitivityvsCameraOffset[i]->GetErrorYlow( p ) ) ); 
		}
            }
	    if( iColor > 0 ) setGraphPlottingStyle( fProjectionSensitivityvsCameraOffset[i], iColor, 1., 20, 1.5 );
	    fProjectionSensitivityvsCameraOffset[i]->Draw( "pl" );
	    sprintf( hname, "%.2f TeV", TMath::Power( 10., fProjectionEnergy_min_logTeV[i] ) );
	    iL->AddEntry( fProjectionSensitivityvsCameraOffset[i], hname, "lp" );
            z++;
        }
    }
    if( iColor < 0 ) iL->Draw();

    return cC;
}

bool VPlotWPPhysSensitivity::plotSensitivityRatio( string iPrint, double ymin, double ymax )
{
    char hname[200];
    char htitle[200];
    sprintf( hname, "cSensRatio" );
    sprintf( htitle, "ratio of sensitivities" );
    TCanvas *cSensRatio = new TCanvas( hname, htitle, 200, 200, 900, 600 );
    cSensRatio->SetGridy( 0 );
    cSensRatio->SetGridx( 0 );

    sprintf( hname, "hSensRatio" );
    TH1D *hNull = new TH1D( hname, "", 10, log10( fMinEnergy_TeV ), log10( fMaxEnergy_TeV ) );
    hNull->SetStats( 0 );
    hNull->SetXTitle( "log_{10} energy [TeV]" );
    hNull->SetYTitle( "Sensitivity ratio" );
    hNull->SetMinimum( ymin );
    hNull->SetMaximum( ymax );
    hNull->Draw( "" );
    hNull->Draw( "AH" );

    plot_nullHistogram( cSensRatio, hNull, true , false, 1.2, fMinEnergy_TeV, fMaxEnergy_TeV );

    TLine *iL = new TLine( log10( fMinEnergy_TeV ), 1., log10( fMaxEnergy_TeV ), 1. );
    iL->SetLineStyle( 2 );
    iL->SetLineWidth( 3 );
    iL->Draw();

    TGraph *gRelG = 0;
    if( fPlotCTARequirements )
    {
       gRelG = fPlotCTARequirements->getRequiredDifferentalSensitivity();
       if( !gRelG ) return false;
       iL->SetLineColor( gRelG->GetLineColor() );
    }
    else
    {
       cout << "Error: CTA requirements not found" << endl;
       return false;
    }

// loop over all data sets and divide it by the first
    for( unsigned int i = 0; i < fData.size(); i++ )
    {
       if( !fData[i] ) continue;

       for( unsigned int j = 0; j < fData[i]->fGraphSensitivity.size(); j++ )
       {
          if( fData[i]->fGraphSensitivity[j] )
	  {
	     TGraphAsymmErrors* g = new TGraphAsymmErrors();
	     if( gRelG ) VHistogramUtilities::divide( g, fData[i]->fGraphSensitivity[j], gRelG );
	     if( g->GetN() > 0 )
	     {
		setGraphPlottingStyle( g, fData[i]->fGraphSensitivity[j]->GetMarkerColor(), 1., 20., 1., fData[i]->fPlottingFillStyle[j], fData[i]->fPlottingLineStyle[j] );
		g->Draw( "p" );
	     }
          }
       }
    }
// plot goal sensitivity
   if( fPlotCTARequirementGoals )
   {
      TGraphAsymmErrors* gRelGoal = (TGraphAsymmErrors*)fPlotCTARequirements->getGoalDifferentialSensitivity();
      if( gRelGoal )
      {
         TGraphAsymmErrors* g = new TGraphAsymmErrors();
	 if( gRelG ) VHistogramUtilities::divide( g, gRelGoal, gRelG );
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
	  sprintf( hname, "%s-SensitivityRatio.eps", iPrint.c_str() );
	  if( cSensRatio ) cSensRatio->Print( hname );
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

void VPlotWPPhysSensitivity::printSensitivityFigureOfMerit( TGraphAsymmErrors *gSensitivity, double iEmin_TeV, double iEmax_TeV,
                                                            string iAnalysis )
{
   if( !gSensitivity ) return;
   if( fPlotCTARequirementsID < 0 ) return;

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
      dy = 0.5 * ( gSensitivity->GetErrorYlow(p)+gSensitivity->GetErrorYhigh(p) );
// excluding the lower bin containing iEmin_TeV, including the bin with iEmax_TeV
      if( iEmin_TeV < x - gSensitivity->GetErrorX( p )
       && iEmax_TeV > x )
      {
// south 50h
	    if( fPlotCTARequirementsID == 0 )
	    {
	       req = VCTASensitivityRequirements::Flux_req50_E2erg_south( TMath::Power( 10., x ) );
	    }
// south 5 h
	    else if( fPlotCTARequirementsID == 1 )
	    {
	       req = VCTASensitivityRequirements::Flux_req5_E2erg_south( TMath::Power( 10., x ) );
	    }
// south 0.5h
	    else if( fPlotCTARequirementsID == 2 )
	    {
	       req = VCTASensitivityRequirements::Flux_req05_E2erg_south( TMath::Power( 10., x ) );
	    }
// north 50h
	    else if( fPlotCTARequirementsID == 3 )
	    {
	       req = VCTASensitivityRequirements::Flux_req50_E2erg_north( TMath::Power( 10., x ) );
	    }
// north 5h
	    else if( fPlotCTARequirementsID == 4 )
	    {
	       req = VCTASensitivityRequirements::Flux_req5_E2erg_north( TMath::Power( 10., x ) );
	    }
// north 0.5h
	    else if( fPlotCTARequirementsID == 5 )
	    {
	       req = VCTASensitivityRequirements::Flux_req05_E2erg_north( TMath::Power( 10., x ) );
	    }
	    if( y > 0 )
	    {
	       m  *= req / y;
	       dm += dy*dy*req*req/y/y/y/y;
            }

	    z++;
       }
   }
   if( z > 0 )
   {
     m = TMath::Power( m, 1./z );
     dm = m*sqrt( dm );
     dm = 1./z * TMath::Power( m, 1./z - 1. ) * dm;
     cout << "PPUT " << iAnalysis << " (calculated from sensitivity, ID" << fPlotCTARequirementsID << ", ";
     cout << z << " points), energy range [";
     cout <<  TMath::Power( 10., iEmin_TeV ) << ", " << TMath::Power( 10., iEmax_TeV ) << "]: ";
     cout << "\t PPUT = " << setprecision ( 4 ) << m << " +- " << dm << endl;
     fSensitivityFOM = m;
     fSensitivityFOM_error = dm;
   }
}


/*

    plot sensitivities and data rates for different data sets

*/
bool VPlotWPPhysSensitivity::plotSensitivity( string iPrint, double iMinSensitivity, double iMaxSensitivity, string iUnit )
{
   TCanvas *cSens = 0;
   TCanvas *cSensInter = 0;
   TCanvas *cBck = 0;

//   initialProjectedSensitivityPlots();

// loop over all data sets
   unsigned int z = 0;
   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      if( !fData[i] || !fData[i]->checkIntegrity() ) continue;
      for( unsigned j = 0; j < fData[i]->fSiteFileName.size(); j++ )
      {
	 VSensitivityCalculator *a = new VSensitivityCalculator();
	 a->setMonteCarloParametersCTA_MC( fData[i]->fSiteFileName[j], fData[i]->fCameraOffset_deg[j], fCrabSpectraFile, fCrabSpectraID );
	 a->setEnergyRange_Lin( fMinEnergy_TeV, fMaxEnergy_TeV );
	 a->setPlotCanvasSize( 900, 600 );
	 a->setPlottingStyle( fData[i]->fPlottingColor[j], fData[i]->fPlottingLineStyle[j], 2., 1, 2., fData[i]->fPlottingFillStyle[j] );
	 if( iUnit == "ENERGY" )  a->setFluxRange_ENERG( iMinSensitivity, iMaxSensitivity );
	 else if( iUnit == "CU" ) a->setFluxRange_CU( iMinSensitivity, iMaxSensitivity );
	 TCanvas *c_temp = 0;
	 c_temp = a->plotDifferentialSensitivityvsEnergyFromCrabSpectrum( cSens, "CTA-PHYS", fData[i]->fPlottingColor[j], iUnit, 
	  		                                                 0.2, fData[i]->fSiteFile_Emin[j], fData[i]->fSiteFile_Emax[j] );
	 if( c_temp ) cSens = c_temp;
	 if( z == 0 ) c_temp = a->plotSignalBackgroundRates( cBck, true, 2.e-7, 14. );   // plot also protons and electrons
	 else         c_temp = a->plotSignalBackgroundRates( cBck, false, 2.e-7, 14. );
	 if( c_temp ) cBck = c_temp;
	 fillProjectedSensitivityPlot( z, a );
	 fData[i]->fGraphSensitivity[j] = (TGraphAsymmErrors*)a->getSensitivityGraph();
	 if( z == 0 && fPlotCTARequirementsID >= 0 && fPlotCTARequirements )
	 {
	     fPlotCTARequirements->plotRequirement_DifferentialSensitivity( cSens, fPlotCTARequirementGoals, iUnit );
	 }
	 z++;
     }
// plot a second window with interpolated sensitivities
     TGraphAsymmErrors *iGraphSensitivity = fData[i]->getCombinedSensitivityGraph( true, "");
     if( iGraphSensitivity )
     {
	VSensitivityCalculator *b = new VSensitivityCalculator();
	b->setMonteCarloParametersCTA_MC( "", 0., fCrabSpectraFile, fCrabSpectraID );
	b->setSensitivityGraph( iGraphSensitivity );
	b->setPlotCanvasSize( 900, 600 );
	b->setPlotCanvasName( "testCanvas" );
	b->setFluxRange_ENERG( iMinSensitivity, iMaxSensitivity );
	if( fData[i]->fPlottingColor.size() > 0 )
	{
	   b->setPlottingStyle( fData[i]->fPlottingColor[0], fData[i]->fPlottingLineStyle[0], 2., 1, 2., fData[i]->fPlottingFillStyle[0] );
        }
	if( i == 0 ) cSensInter = b->plotSensitivityvsEnergyFromCrabSpectrum( 0, i+1, iUnit, 0.2 );
	else         cSensInter = b->plotSensitivityvsEnergyFromCrabSpectrum( cSensInter, i+1, iUnit, 0.2 );
	if( fPlotCTARequirementsID >= 0 && fPlotCTARequirements )
	{
	    fPlotCTARequirements->plotRequirement_DifferentialSensitivity( cSensInter, fPlotCTARequirementGoals, iUnit );
	}
     }
   }
// print results
   if( cSens )
   {
      plotLegend( cSens, false );
      if( iPrint.size() > 0 )
      {
	  char hname[2000];
	  sprintf( hname, "%s-Sensitivity.eps", iPrint.c_str() );
	  cSens->Print( hname );
      }
   }
   if( cSensInter )
   {
      plotLegend( cSensInter, false );
      if( iPrint.size() > 0 )
      {
	  char hname[2000];
	  sprintf( hname, "%s-SensitivityInter.eps", iPrint.c_str() );
	  cSensInter->Print( hname );
      }
   }
   if( cBck )
   {
      plotLegend( cBck, false );
      if( iPrint.size() > 0 )
      {
	  char hname[2000];
	  sprintf( hname, "%s-BRates.eps", iPrint.c_str() );
	  if( cBck ) cBck->Print( hname );
      }
   }

   return true;
}

bool VPlotWPPhysSensitivity::plotLegend( TCanvas *c, bool iDown, bool iLeft, bool iAddFirst )
{
   if( !c ) return false;
   c->cd();

   double x = 0.2+0.35;
   if( iLeft ) x = 0.15;
   double y = 0.65;
   if( iDown ) y -= 0.50;
   double y_yp = y+0.18;
   if( fData.size() == 2 )
   {
      y_yp -= 0.1;
   }
   TLegend *iL = new TLegend( x, y, x+0.30, y_yp );
   iL->SetFillColor( 0 );

   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      if( i == 0 && !iAddFirst ) continue;
      for( unsigned int j = 0; j < fData[i]->fSiteFile_exists.size(); j++ )
      {
	 if( fData[i]->fSiteFile_exists[j] && fData[i]->fLegend.size() > 0 )
	 {
	    TGraph *g = new TGraph( 1 );
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
   if( iL->GetNRows() > 0 ) iL->Draw();
   return true; 
}

bool VPlotWPPhysSensitivity::addDataSets( string iDataSettxtFile )
{
   unsigned int z_site = 0;
   for( ;; )
   {
       fData.push_back( new VSiteData() );
       if( !fData.back()->addDataSet( iDataSettxtFile, z_site ) )
       {
	  fData.pop_back();
          break;
       }
       z_site++;
   }

   if( fData.size() == 0 ) return false;

   return true;
}


bool VPlotWPPhysSensitivity::setPlotCTARequirements( int iRequirementID, bool iPlotRequirementGoals )
{
   fPlotCTARequirementGoals = iPlotRequirementGoals;
   fPlotCTARequirementsID = iRequirementID;
   if( fPlotCTARequirementsID >= 0 )
   {
      fPlotCTARequirements = new VCTARequirements();
      return fPlotCTARequirements->setRequirement( fPlotCTARequirementsID );
   }

   return false;
}
   
