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
}

void VPlotWPPhysSensitivity::reset()
{
    fData.clear();
}

bool VPlotWPPhysSensitivity::addDataSet( VPlotWPPhysSensitivityData* iData, bool iInit )
{
    fData.push_back( new VPlotWPPhysSensitivityData() );
    fData.back()->fAnalysis = iData->fAnalysis;
    fData.back()->fSubArray = iData->fSubArray;
    fData.back()->fObservationTime_s = iData->fObservationTime_s;
    fData.back()->fCameraOffset_deg = iData->fCameraOffset_deg;

    fData.back()->fPlottingColor = iData->fPlottingColor;
    fData.back()->fPlottingLineStyle = iData->fPlottingLineStyle;
    fData.back()->fPlottingFillStyle = iData->fPlottingFillStyle;
    fData.back()->fLegend = iData->fLegend;

// put file name together
    if( iInit )
    {
       if( !initialize( fData.back() ) )
       {
	   cout << "Data set not found: " << fData.back()->fAnalysis << endl;
	   return false;
       }
    }
// file name is given
    else
    {
       fData.back()->fSensitivityFileName = iData->fSensitivityFileName;
    }
// check if file exists
    TFile iF( fData.back()->fSensitivityFileName.c_str() );
    if( !iF.IsZombie() ) fData.back()->fFileExists = true;
    else                 fData.back()->fFileExists = false;
    iF.Close();
       
    fData.back()->print();

    return true;
}

bool VPlotWPPhysSensitivity::addDataSet( string iAnalysis, string iSubArray, double iObservationTime_s, double iOffset_deg,
                                         string iLegend, int iColor, int iLineStyle, int iFillStyle )
{
    VPlotWPPhysSensitivityData i_temp;
    i_temp.fAnalysis = iAnalysis;
    i_temp.fSubArray = iSubArray;
    i_temp.fObservationTime_s = iObservationTime_s;
    i_temp.fCameraOffset_deg = iOffset_deg;

    i_temp.fPlottingColor = iColor;
    i_temp.fPlottingLineStyle = iLineStyle;
    i_temp.fPlottingFillStyle = iFillStyle;
    i_temp.fLegend = iLegend;

    return addDataSet( &i_temp, true );

    return false;
}

/*

    set file names and legends for different analyses

*/
bool VPlotWPPhysSensitivity::initialize( VPlotWPPhysSensitivityData* iData )
{
   if( !iData ) return false;

   char hname[2000];
///////////////////////////////////////////////////////////////////////////////////////////////
// set correct file names depending on the analysis
   ostringstream iTemp;
// DESY analysis
   if( iData->fAnalysis.find( "DESY" ) != string::npos )
   {
      sprintf( hname, "%ds", (int)(iData->fObservationTime_s) );
      iTemp << "data/DESY/" << iData->fAnalysis << "." << iData->fSubArray << "." << hname << ".root";
   }
   else if( iData->fAnalysis.find( "DIV" ) != string::npos )
   {
      sprintf( hname, "%ds", (int)(iData->fObservationTime_s) );
      iTemp << "data/DIV/" << iData->fAnalysis << "." << iData->fSubArray << "." << hname << ".root";
   }
   else if( iData->fAnalysis == "VTS" )
   {
      sprintf( hname, "%.1fh", iData->fObservationTime_s/3600. );
      iTemp << "data/VTS/VTS." << iData->fSubArray << "." << hname << ".root";
   }
   else if( iData->fAnalysis == "ISDC" )
   {
      sprintf( hname, "%.1f", iData->fObservationTime_s/3600. );
      iTemp << "data/ISDC/ISDC_2000m_KonradB_optimal_"  << iData->fSubArray << "_" << hname;
      iTemp << "h_20deg_20110615.root";
   }
   else if( iData->fAnalysis == "ISDC.3700m" )
   {
      sprintf( hname, "%.1f", iData->fObservationTime_s/3600. );
      iTemp << "data/ISDC/ISDC_3700m_optimal_"  << iData->fSubArray << "_" << hname;
      iTemp << "h_20deg_20110615.root";
   }
   else if( iData->fAnalysis == "ISDC.moon" )
   {
      sprintf( hname, "%.1f", iData->fObservationTime_s/3600. );
      iTemp << "data/ISDC/ISDC_2000m_moonlight_optimal_"  << iData->fSubArray << "_" << hname;
      iTemp << "h_20deg_20110615.root";
   }
   else if( iData->fAnalysis == "IFAE" )
   {
      if( iData->fObservationTime_s/3600. > 1. ) sprintf( hname, "%d", (int)(iData->fObservationTime_s/3600.) );
      else                                       sprintf( hname, "%.1f", iData->fObservationTime_s/3600. );
      iTemp << "data/IFAEPerformanceBCDEINANB_Nov2011/Subarray" << iData->fSubArray;
      if( iData->fSubArray == "B" || iData->fSubArray == "C" ) iTemp << "_IFAE_" << hname << "hours_20111121.root";
      else                                             iTemp << "_IFAE_" << hname << "hours_20111109.root";
   }
   else if( iData->fAnalysis == "IFAE_OFFAXIS" )
   {
      if( iData->fObservationTime_s/3600. > 1. ) sprintf( hname, "%d", (int)(iData->fObservationTime_s/3600.) );
      else                                       sprintf( hname, "%.1f", iData->fObservationTime_s/3600. );
// Nov 2011
//	       iTemp << "data/IFAEOffaxisPerformanceBEI_Nov2011/Subarray" << iData->fSubArray;
//	       iTemp << "_IFAE_" << hname << "hours_20111121_offaxis.root";
// May 2012
      iTemp << "data/IFAE_May2012/Subarray" << iData->fSubArray;
      iTemp << "_IFAE_" << hname << "hours_20120510_offaxis.root";
   }
   else if( iData->fAnalysis.substr( 0, 8 )  == "Subarray" )
   {
      if( iData->fObservationTime_s/3600. > 1. ) sprintf( hname, "%d", (int)(iData->fObservationTime_s/3600.) );
      else                                       sprintf( hname, "%.1f", iData->fObservationTime_s/3600. );
// IFAE May 2013
//      iTemp << "data/IFAE_May2013/Subarray" << iData->fSubArray;
// July 2013
//      iTemp << "data/IFAE_July2013/" << iData->fAnalysis;
// Sept 2013
      iTemp << "data/IFAE_Sept2013/" << iData->fAnalysis;
      iTemp << "_merged_IFAE_" << hname << "hours_20130901.root";
   }
   else if( iData->fAnalysis == "HD_KB" || iData->fAnalysis == "MPIK" )
   {
      if( iData->fObservationTime_s/3600. > 1. ) sprintf( hname, "%d", (int)(iData->fObservationTime_s/3600.) );
      else                                       sprintf( hname, "%.1f", iData->fObservationTime_s/3600. );
      iTemp << "data/data_KB/kb_" << iData->fSubArray;
      iTemp << "_" << hname << "h_20deg_v3.root";
   }
   else if( iData->fAnalysis == "ParisMVA" )
   {
      if( iData->fObservationTime_s/3600. > 1. ) sprintf( hname, "%d", (int)(iData->fObservationTime_s/3600.) );
      else                                       sprintf( hname, "%.1f", iData->fObservationTime_s/3600. );
      iTemp << "data/ParisMVA/Subarray" << iData->fSubArray;
      iTemp << "_ParisMVA_" << hname << "hours.root";
   }
   else 
   {
       cout << "VPlotWPPhysSensitivity::initialize() warning: unknown analysis: " << iData->fAnalysis << endl;
       return false;
   }

   iData->fSensitivityFileName = iTemp.str();

// set legend (don't overwrite existing legend)
   if( iData->fLegend == "DEFAULT" )
   {
      sprintf( hname, "%s (%s, %.1f h, %.1f deg)", iData->fSubArray.c_str(), iData->fAnalysis.c_str(), 
						   iData->fObservationTime_s/3600., iData->fCameraOffset_deg  );
      iData->fLegend = hname;
   }

   return true;
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
       if( fData[i]->fFileExists )
       {
	  fIRF->addInstrumentResponseData( fData[i]->fSensitivityFileName, 20., fData[i]->fCameraOffset_deg, 
	                                   0, 2.4, 200, "A_MC", fData[i]->fPlottingColor, fData[i]->fPlottingLineStyle,
					   21, 0.5 );
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
	       if( i_mz > 0 )
	       {
		  fProjectionSensitivityvsCameraOffset[i]->SetPoint( z, fData[iDataSet]->fCameraOffset_deg, i_m / i_mz );
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

bool VPlotWPPhysSensitivity::plotSensitivityRatio( string iPrint, double ymin, double ymax, unsigned int iRelativeDataSetID )
{
    char hname[200];
    char htitle[200];
    sprintf( hname, "cSensRatio_%d", iRelativeDataSetID );
    sprintf( htitle, "ratio of sensitivities (relative to %d)", iRelativeDataSetID );
    TCanvas *cSensRatio = new TCanvas( hname, htitle, 200, 200, 900, 600 );
    cSensRatio->SetGridy( 0 );
    cSensRatio->SetGridx( 0 );

    sprintf( hname, "hSensRatio_%d", iRelativeDataSetID );
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

    if( fData.size() > 0 && fData[0]->gSensitivity )
    {
       iL->SetLineColor( fData[0]->gSensitivity->GetLineColor() );

// loop over all data sets and divide it by the first
       for( unsigned int i = 1; i < fData.size(); i++ )
       {
          TGraphAsymmErrors* g = new TGraphAsymmErrors();
	  VHistogramUtilities::divide( g, fData[i]->gSensitivity, fData[0]->gSensitivity );
	  if( g->GetN() > 0 )
	  {
	     setGraphPlottingStyle( g, fData[i]->gSensitivity->GetMarkerColor() );
	     g->Draw( "p" );
          }
       }
    }
    if( cSensRatio )
    {
      plotLegend( cSensRatio, true, false );
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
   iEmin_TeV = log10( iEmin_TeV );
   iEmax_TeV = log10( iEmax_TeV );
   for( unsigned int i = 0; i < fData.size(); i++ )
   {
       double m = 1.;
       double dm = 0.;
       double z = 0.;
       if( fData[i]->gSensitivity )
       {
           double x = 0.;
	   double y = 0.;
	   double dy = 0.;
	   double req = 0.;
	   for( int p = 0; p < fData[i]->gSensitivity->GetN(); p++ )
	   {
	       fData[i]->gSensitivity->GetPoint( p, x, y );
	       dy = 0.5 * ( fData[i]->gSensitivity->GetErrorYlow(p)+fData[i]->gSensitivity->GetErrorYhigh(p) );
// excluding the lower bin containing iEmin_TeV, including the bin with iEmax_TeV
	       if( iEmin_TeV < x - fData[i]->gSensitivity->GetErrorX( p )
	        && iEmax_TeV > x )
	       {
		  req = VCTASensitivityRequirements::Flux_req50_E2erg_south( TMath::Power( 10., x ) );
	          m  *= req / y;
		  if( y > 0. ) dm += dy*dy*req*req/y/y/y/y;
		  z++;
               }
           }
        }
	if( z > 0 )
	{
	   m = TMath::Power( m, 1./z );
	   dm = m*sqrt( dm );
	   dm = 1./z * TMath::Power( m, 1./z - 1. ) * dm;
	   cout << "Figure of merit (calculated from sensitivity) for " << fData[i]->fAnalysis;
	   cout << "(" << z << " points): " << setprecision ( 4 ) << m;
	   cout << " +- " << dm << endl;
        }
    }
}
  




/*

    plot sensitivities and data rates for different data sets

*/
bool VPlotWPPhysSensitivity::plotSensitivity( string iPrint, double iMinSensitivity, double iMaxSensitivity, string iUnit )
{
   TCanvas *cSens = 0;
   TCanvas *cBck = 0;

   initialProjectedSensitivityPlots();

// loop over all data sets
   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      VSensitivityCalculator *a = new VSensitivityCalculator();
      a->setMonteCarloParametersCTA_MC( fData[i]->fSensitivityFileName, fData[i]->fCameraOffset_deg, fCrabSpectraFile, fCrabSpectraID );
      a->setEnergyRange_Lin( fMinEnergy_TeV, fMaxEnergy_TeV );
      a->setPlotCanvasSize( 900, 600 );
      a->setPlottingStyle( fData[i]->fPlottingColor, fData[i]->fPlottingLineStyle, 2., 1, 2., fData[i]->fPlottingFillStyle );
      if( iUnit == "ENERGY" )  a->setFluxRange_ENERG( iMinSensitivity, iMaxSensitivity );
      else if( iUnit == "CU" ) a->setFluxRange_CU( iMinSensitivity, iMaxSensitivity );
      TCanvas *c_temp = a->plotDifferentialSensitivityvsEnergyFromCrabSpectrum( cSens, "CTA-PHYS", fData[i]->fPlottingColor, iUnit, 0.2, 0.01 );
      if( c_temp ) cSens = c_temp;
      if( i == 0 ) c_temp = a->plotSignalBackgroundRates( cBck, true, 2.e-7, 14. );   // plot also protons and electrons
      else         c_temp = a->plotSignalBackgroundRates( cBck, false, 2.e-7, 14. );
      if( c_temp ) cBck = c_temp;
      fillProjectedSensitivityPlot( i, a );
      fData[i]->gSensitivity = (TGraphAsymmErrors*)a->getSensitivityGraph();
       if( fPlotCTARequirementsID >= 0 && fPlotCTARequirements )
       {
	  fPlotCTARequirements->plotRequirement_DifferentialSensitivity( cSens, fPlotCTARequirementGoals, iUnit );
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
	  if( cSens ) cSens->Print( hname );
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
      if( fData[i]->fFileExists && fData[i]->fLegend.size() > 0 )
      {
	 TGraph *g = new TGraph( 1 );
	 g->SetLineColor( fData[i]->fPlottingColor );
	 g->SetLineStyle( fData[i]->fPlottingLineStyle );
	 g->SetFillStyle( fData[i]->fPlottingFillStyle );
	 g->SetFillColor( fData[i]->fPlottingColor );
	 g->SetMarkerColor( fData[i]->fPlottingColor );
	 g->SetMarkerStyle( 1 );
	 if( fData[i]->fLegend.size() > 0 )
	 {
	   iL->AddEntry( g, fData[i]->fLegend.c_str(), "lF" );
         }
      }
   }
   if( iL->GetNRows() > 0 ) iL->Draw();
   return true; 
}

bool VPlotWPPhysSensitivity::addDataSets( string iDataSettxtFile )
{
   ifstream is;
   is.open( iDataSettxtFile.c_str(), ifstream::in );
   if( !is )
   {
      cout << "VPlotWPPhysSensitivity::addDataSets error opening data set txt file: " << iDataSettxtFile << endl;
      return false;
   }
   string is_line;

   unsigned int z = 1;
   while( getline( is, is_line ) )
   {
      VPlotWPPhysSensitivityData i_temp;
      istringstream is_stream( is_line );
      if( !is_stream.eof() )
      { 
         is_stream >> i_temp.fAnalysis;
// ignore lines with '#' in the beginning
	 if( i_temp.fAnalysis == "#" ) continue;
      }
      if( !is_stream.eof() ) is_stream >> i_temp.fSubArray;
      if( !is_stream.eof() ) is_stream >> i_temp.fObservationTime_s;
      else                   i_temp.fObservationTime_s = 50.*3600.;
      if( !is_stream.eof() ) is_stream >> i_temp.fCameraOffset_deg;
      else                   i_temp.fCameraOffset_deg = 0.;
      if( !is_stream.eof() ) is_stream >> i_temp.fPlottingColor;
      else                   i_temp.fPlottingColor = z;
      if( !is_stream.eof() ) is_stream >> i_temp.fPlottingLineStyle;
      else                   i_temp.fPlottingLineStyle = 1;
      if( !is_stream.eof() ) is_stream >> i_temp.fPlottingFillStyle;
      else                   i_temp.fPlottingFillStyle = 3001;
      if( !is_stream.eof() )
      {
         i_temp.fLegend = is_stream.str().substr( is_stream.tellg(), is_stream.str().size() );
      }
      else
      {
         char hname[200];
	 if( i_temp.fObservationTime_s < 1800. ) sprintf( hname, "%s (%ds)", i_temp.fSubArray.c_str(), (int)i_temp.fObservationTime_s );
	 else                                    sprintf( hname, "%s (%.1fh)", i_temp.fSubArray.c_str(), i_temp.fObservationTime_s / 3600. );
         i_temp.fLegend = hname;
      }

      addDataSet( &i_temp, true );

      z++;
   }
   is.close();

   return true;
}

vector< string > VPlotWPPhysSensitivity::getListOfArrays()
{
   vector< string > iT;
   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      iT.push_back( fData[i]->fSubArray );
   }
   return iT;
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
   

// ====================================================================================
// ====================================================================================
// ====================================================================================

VPlotWPPhysSensitivityData::VPlotWPPhysSensitivityData()
{
   fAnalysis = "";
   fFileExists = false;
   fSensitivityFileName = ""; 
   fObservationTime_s = 50.*3600.;
   fSubArray = "E";
   fCameraOffset_deg = 0.;

   fPlottingColor = 1;
   fPlottingLineStyle = 1;
   fPlottingFillStyle = 3001;
   fLegend = "";

   gSensitivity = 0;
}

/*

   print data set

*/
void VPlotWPPhysSensitivityData::print()
{
   cout << fSensitivityFileName << ": " << fObservationTime_s/3600. << "h, array ";
   cout << fSubArray << ", offset " << fCameraOffset_deg <<  " deg" << endl;
   cout << "\t (color " << fPlottingColor << ", line " << fPlottingLineStyle << ")" << endl;
}

