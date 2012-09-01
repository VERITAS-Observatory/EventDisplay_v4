/*! \class VPlotWPPhysSensitivity

*/

#include "VPlotWPPhysSensitivity.h"

VPlotWPPhysSensitivity::VPlotWPPhysSensitivity()
{
   fIRF = 0;
   setEnergyRange_Lin_TeV();
}


void VPlotWPPhysSensitivity::addAnalysis( string iAnalysis, int iColor, int iLineStyle, int iFillStyle )
{
   fAnalysis.push_back( iAnalysis );
   fAnalysisColor.push_back( iColor );
   fAnalysisLineStyle.push_back( iLineStyle );
   fAnalysisFillStyle.push_back( iFillStyle );
}

void VPlotWPPhysSensitivity::addCameraOffset( double iCameraOffset_deg, int iColor, int iLineStyle, int iFillStyle )
{
   fCameraOffset_deg.push_back( iCameraOffset_deg );
   fCameraOffsetColor.push_back( iColor );
   fCameraOffsetLineStyle.push_back( iLineStyle );
   fCameraOffsetFillStyle.push_back( iFillStyle );
}

void VPlotWPPhysSensitivity::addObservationTime( double iObsTime, int iColor, int iLineStyle, int iFillStyle )
{
   fObservationTime_H.push_back( iObsTime );
   fObservationTimeColor.push_back( iColor );
   fObservationTimeLineStyle.push_back( iLineStyle );
   fObservationTimeFillStyle.push_back( iFillStyle );
}

void VPlotWPPhysSensitivity::addSubArray( string iArray, int iColor, int iLineStyle, int iFillStyle )
{
   fSubArray.push_back( iArray );
   fSubArrayColor.push_back( iColor );
   fSubArrayLineStyle.push_back( iLineStyle );
   fSubArrayFillStyle.push_back( iFillStyle );
}

bool VPlotWPPhysSensitivity::initialize()
{
   char hname[200];
   fSensitivityFile.clear();
   fLegend.clear();
   fPlottingColor.clear();
   fPlottingLineStyle.clear();

   for( unsigned int a = 0; a < fSubArray.size(); a++ )
   {
      for( unsigned int t = 0; t < fObservationTime_H.size(); t++ )
      {
	 for( unsigned int i = 0; i < fAnalysis.size(); i++ )
	 {
	    ostringstream iTemp;
	    if( fAnalysis[i].find( "DESY" ) != string::npos )
	    {
	       sprintf( hname, "%.1fh", fObservationTime_H[t] );
	       iTemp << "data/DESY/" << fAnalysis[i] << "." << fSubArray[a] << "." << hname << ".root";
	    }
	    else if( fAnalysis[i] == "VTS" )
	    {
	       sprintf( hname, "%.1fh", fObservationTime_H[t] );
	       iTemp << "data/VTS/VTS." << fSubArray[a] << "." << hname << ".root";
	    }
	    else if( fAnalysis[i] == "ISDC" )
	    {
	       sprintf( hname, "%.1f", fObservationTime_H[t] );
	       iTemp << "data/ISDC/ISDC_2000m_KonradB_optimal_"  << fSubArray[a] << "_" << hname;
	       iTemp << "h_20deg_20110615.root";
	    }
	    else if( fAnalysis[i] == "ISDC.3700m" )
	    {
	       sprintf( hname, "%.1f", fObservationTime_H[t] );
	       iTemp << "data/ISDC/ISDC_3700m_optimal_"  << fSubArray[a] << "_" << hname;
	       iTemp << "h_20deg_20110615.root";
	    }
	    else if( fAnalysis[i] == "ISDC.moon" )
	    {
	       sprintf( hname, "%.1f", fObservationTime_H[t] );
	       iTemp << "data/ISDC/ISDC_2000m_moonlight_optimal_"  << fSubArray[a] << "_" << hname;
	       iTemp << "h_20deg_20110615.root";
	    }
	    else if( fAnalysis[i] == "IFAE" )
	    {
	       if( fObservationTime_H[t] > 1. ) sprintf( hname, "%d", (int)fObservationTime_H[t] );
	       else                             sprintf( hname, "%.1f", fObservationTime_H[t] );
	       iTemp << "data/IFAEPerformanceBCDEINANB_Nov2011/Subarray" << fSubArray[a];
	       if( fSubArray[a] == "B" || fSubArray[a] == "C" ) iTemp << "_IFAE_" << hname << "hours_20111121.root";
	       else                                             iTemp << "_IFAE_" << hname << "hours_20111109.root";
	    }
	    else if( fAnalysis[i] == "IFAE_OFFAXIS" )
	    {
	       if( fObservationTime_H[t] > 1. ) sprintf( hname, "%d", (int)fObservationTime_H[t] );
	       else                             sprintf( hname, "%.1f", fObservationTime_H[t] );
// Nov 2011
//	       iTemp << "data/IFAEOffaxisPerformanceBEI_Nov2011/Subarray" << fSubArray[a];
//	       iTemp << "_IFAE_" << hname << "hours_20111121_offaxis.root";
// May 2012
	       iTemp << "data/IFAE_May2012/Subarray" << fSubArray[a];
	       iTemp << "_IFAE_" << hname << "hours_20120510_offaxis.root";
            }
	    else if( fAnalysis[i] == "HD_KB" || fAnalysis[i] == "MPIK" )
	    {
	       if( fObservationTime_H[t] > 1. ) sprintf( hname, "%d", (int)fObservationTime_H[t] );
	       else                             sprintf( hname, "%.1f", fObservationTime_H[t] );
	       iTemp << "data/data_KB/kb_" << fSubArray[a];
	       iTemp << "_" << hname << "h_20deg_v3.root";
            }
	    else if( fAnalysis[i] == "ParisMVA" )
	    {
	       if( fObservationTime_H[t] > 1. ) sprintf( hname, "%d", (int)fObservationTime_H[t] );
	       else                             sprintf( hname, "%.1f", fObservationTime_H[t] );
	       iTemp << "data/ParisMVA/Subarray" << fSubArray[a];
	       iTemp << "_ParisMVA_" << hname << "hours.root";
            }
	    else 
	    {
	        cout << "VPlotWPPhysSensitivity::initialize() warning: unknown analysis: " << fAnalysis[i] << endl;
	        continue;
            }
	    if( fCameraOffset_deg.size() == 0 )
	    {
	       fSensitivityFile.push_back( iTemp.str() );
	       if( fAnalysis[i] != "DESY" ) sprintf( hname, "%s (%s, %.1f h)", fSubArray[a].c_str(), fAnalysis[i].c_str(), fObservationTime_H[t] );
	       else                         sprintf( hname, "%s (%s, %.1f h)", fSubArray[a].c_str(), "DESY.ultra3.2000m", fObservationTime_H[t] );
// check that sensitivity file exists
               TFile iF( fSensitivityFile.back().c_str() );
	       if( !iF.IsZombie() ) fLegend.push_back( hname );
	       else                 fLegend.push_back( "" );
	       iF.Close();
// plotting colors/line and fill styles (note hierarchy)
               if( fAnalysisColor[i] > 0 )                 fPlottingColor.push_back( fAnalysisColor[i] );
	       else if( fSubArrayColor[a] > 0 )            fPlottingColor.push_back( fSubArrayColor[a] );
	       else if( fObservationTimeColor[t] > 0 )     fPlottingColor.push_back( fObservationTimeColor[t] );
	       else                                        fPlottingColor.push_back( 1 );

	       if( fAnalysisLineStyle[i] > 0 )             fPlottingLineStyle.push_back( fAnalysisLineStyle[i] );
	       else if( fSubArrayLineStyle[a] > 0 )        fPlottingLineStyle.push_back( fSubArrayLineStyle[a] );
	       else if( fObservationTimeLineStyle[t] > 0 ) fPlottingLineStyle.push_back( fObservationTimeLineStyle[t] );
	       else                                        fPlottingLineStyle.push_back( t+1 );

	       if( fAnalysisFillStyle[i] > 0 )             fPlottingFillStyle.push_back( fAnalysisFillStyle[i] );
	       else if( fSubArrayFillStyle[a] > 0 )        fPlottingFillStyle.push_back( fSubArrayFillStyle[a] );
	       else if( fObservationTimeFillStyle[t] > 0 ) fPlottingFillStyle.push_back( fObservationTimeFillStyle[t] );
	       else                                        fPlottingFillStyle.push_back( 1001 );

	       fIRFCameraOffset_deg.push_back( 0. );
            }
	    else
	    {
	       for( unsigned int c = 0; c < fCameraOffset_deg.size(); c++ )
	       {
		  fSensitivityFile.push_back( iTemp.str() );
		  if( fCameraOffset_deg.size() == 1 )
		  {
		     if( fAnalysis[i] != "DESY" ) sprintf( hname, "%s (%s, %.1f h)", fSubArray[a].c_str(), fAnalysis[i].c_str(), fObservationTime_H[t] );
	             else  sprintf( hname, "%s (%s, %.1f h)", fSubArray[a].c_str(), "DESY.ultra3.2000m", fObservationTime_H[t] );
                  }
		  else
		  {
	             if( fAnalysis[i] != "DESY" ) sprintf( hname, "%s (%s, %.1f h, %.1f deg)", fSubArray[a].c_str(), fAnalysis[i].c_str(), fObservationTime_H[t], fCameraOffset_deg[c] );
	             else  sprintf( hname, "%s (%s, %.1f h, %.1f deg)", fSubArray[a].c_str(), "DESY.ultra3.2000m", fObservationTime_H[t], fCameraOffset_deg[c] );
                  }
		  TFile iF( fSensitivityFile.back().c_str() );
		  if( !iF.IsZombie() ) fLegend.push_back( hname );
		  else                 fLegend.push_back( "" );
		  iF.Close();
// plotting colors and line styles (note hierarchy)
		  if( fAnalysisColor[i] > 0 )                 fPlottingColor.push_back( fAnalysisColor[i] );
		  else if( fSubArrayColor[a] > 0 )            fPlottingColor.push_back( fSubArrayColor[a] );
		  else if( fCameraOffsetColor[c] > 0 )        fPlottingColor.push_back( fCameraOffsetColor[c] );
		  else if( fObservationTimeColor[t] > 0 )     fPlottingColor.push_back( fObservationTimeColor[t] );
		  else fPlottingColor.push_back( 1 );

		  if( fAnalysisLineStyle[i] > 0 )             fPlottingLineStyle.push_back( fAnalysisLineStyle[i] );
		  else if( fSubArrayLineStyle[a] > 0 )        fPlottingLineStyle.push_back( fSubArrayLineStyle[a] );
		  else if( fCameraOffsetLineStyle[c] > 0 )    fPlottingLineStyle.push_back( fCameraOffsetLineStyle[c] );
		  else if( fObservationTimeLineStyle[t] > 0 ) fPlottingLineStyle.push_back( fObservationTimeLineStyle[t] );
		  else                                        fPlottingLineStyle.push_back( t+1 );

		  if( fAnalysisFillStyle[i] > 0 )             fPlottingFillStyle.push_back( fAnalysisFillStyle[i] );
		  else if( fSubArrayFillStyle[a] > 0 )        fPlottingFillStyle.push_back( fSubArrayFillStyle[a] );
		  else if( fCameraOffsetFillStyle[c] > 0 )    fPlottingFillStyle.push_back( fCameraOffsetFillStyle[c] );
		  else if( fObservationTimeFillStyle[t] > 0 ) fPlottingFillStyle.push_back( fObservationTimeFillStyle[t] );
		  else                                        fPlottingFillStyle.push_back( t+1 );

		  fIRFCameraOffset_deg.push_back( fCameraOffset_deg[c] );
               }
            }
         }
      }
   }

// print data sets
   for( unsigned int i = 0; i < fSensitivityFile.size(); i++ )
   {
      cout << fSensitivityFile[i] << "\t" << fPlottingColor[i] << "\t" << fPlottingLineStyle[i] << "( " << fLegend[i] << ")" << endl;
   }
   if( fSensitivityFile.size() == 0 )
   {
      cout << "no data sets defined" << endl;
   }

   return true;
}

void VPlotWPPhysSensitivity::addSensitivityFile( string iSensitivityFile, string iLegend, int iColor, int iLineStyle, int iFillStyle )
{
    fSensitivityFile.push_back( iSensitivityFile );
    fLegend.push_back( iLegend );
    fPlottingColor.push_back( iColor );
    fPlottingLineStyle.push_back( iLineStyle );
    fPlottingFillStyle.push_back( iFillStyle );
    fIRFCameraOffset_deg.push_back( 0. );
}
  
bool VPlotWPPhysSensitivity::plotIRF( string iPrint, double iEffAreaMin, double iEffAreaMax, double iEnergyResolutionMax )
{
    fIRF = new VPlotInstrumentResponseFunction();

    fIRF->setCanvasSize( 400, 400 );
    fIRF->setPlottingAxis( "energy_Lin", "X", true, fMinEnergy_TeV, fMaxEnergy_TeV, "energy [TeV]" );
    fIRF->setPlottingAxis( "effarea_Lin", "X", true, iEffAreaMin, iEffAreaMax );
    fIRF->setPlottingAxis( "energyresolution_Lin", "X", false, 0., 0.7 );

    for( unsigned int i = 0; i < fSensitivityFile.size(); i++ )
    {
       fIRF->addInstrumentResponseData( fSensitivityFile[i], 20., fIRFCameraOffset_deg[i], 0, 2.4, 200, "A_MC", 
                                        fPlottingColor[i], fPlottingLineStyle[i], 21, 0.5 );
       fIRF->addInstrumentResponseData( fSensitivityFile[i], 20., fIRFCameraOffset_deg[i], 0, 2.4, 200, "A_REC", 
                                        fPlottingColor[i], fPlottingLineStyle[i]+1, 21, 0.5 );
    }

    char hname[2000];

    TCanvas *c = 0;

    c = fIRF->plotEffectiveArea();
    plotLegend( c, true );
    if( iPrint.size() > 0 )
    {
       sprintf( hname, "%s-EffArea.eps", iPrint.c_str() );
       if( c ) c->Print( hname );
    }
    c = fIRF->plotAngularResolution();
    plotLegend( c, false );
    if( iPrint.size() > 0 )
    {
       sprintf( hname, "%s-AngRes.eps", iPrint.c_str() );
       if( c ) c->Print( hname );
    }
    c = fIRF->plotAngularResolution( "energy", "80" );
    plotLegend( c, false );
    c = fIRF->plotEnergyResolution( iEnergyResolutionMax );
    plotLegend( c, false );
    if( iPrint.size() > 0 )
    {
       sprintf( hname, "%s-ERes.eps", iPrint.c_str() );
       if( c ) c->Print( hname );
    }
    c = fIRF->plotEnergyReconstructionBias( "mean", -0.5, 0.5 );
    plotLegend( c, false );
    if( iPrint.size() > 0 )
    {
       sprintf( hname, "%s-EBias.eps", iPrint.c_str() );
       if( c ) c->Print( hname );
    }

    return true;
}

bool VPlotWPPhysSensitivity::plotLegend( TCanvas *c, bool iDown )
{
   if( !c ) return false;
   c->cd();

   double x = 0.2+0.35;
   double y = 0.65;
   if( iDown ) y -= 0.5;
   TLegend *iL = new TLegend( x, y, x+0.30, y+0.22 );
   iL->SetFillColor( 0 );

   for( unsigned int i = 0; i < fLegend.size(); i++ )
   {
      TGraph *g = new TGraph( 1 );
      g->SetLineColor( fPlottingColor[i] );
      g->SetLineStyle( fPlottingLineStyle[i] );
      g->SetMarkerStyle( 1 );
      if( fLegend[i].size() > 0 ) iL->AddEntry( g, fLegend[i].c_str(), "l" );
   }
   iL->Draw();
   return true; 
}

bool VPlotWPPhysSensitivity::plotSensitivity( string iPrint, double iMinSensitivity, double iMaxSensitivity, string iUnit )
{
   string iCrabFile = "$CTA_EVNDISP_ANA_DIR/AstroData/TeV_data/EnergySpectrum_literatureValues_CrabNebula.dat";
   unsigned int iCrabID = 6;

   TCanvas *cSens = 0;
   TCanvas *cBck = 0;
   for( unsigned int i = 0; i < fSensitivityFile.size(); i++ )
   {
      VSensitivityCalculator *a = new VSensitivityCalculator();
      a->setMonteCarloParametersCTA_MC( fSensitivityFile[i], fIRFCameraOffset_deg[i], iCrabFile, iCrabID );
      a->setEnergyRange_Lin( fMinEnergy_TeV, fMaxEnergy_TeV );
      a->setPlotCanvasSize( 900, 600 );
      a->setPlottingStyle( fPlottingColor[i], fPlottingLineStyle[i], 2., 1, 2., fPlottingFillStyle[i] );
      if( iUnit == "ENERGY" )  a->setFluxRange_ENERG( iMinSensitivity, iMaxSensitivity );
      else if( iUnit == "CU" ) a->setFluxRange_CU( iMinSensitivity, iMaxSensitivity );
      TCanvas *c_temp = a->plotDifferentialSensitivityvsEnergyFromCrabSpectrum( cSens, "CTA-PHYS", fPlottingColor[i], iUnit, 0.2, 0.01 );
      if( c_temp ) cSens = c_temp;
      if( i == 0 ) c_temp = a->plotSignalBackgroundRates( cBck, true );   // plot protons and electrons
      else         c_temp = a->plotSignalBackgroundRates( cBck, false );
      if( c_temp ) cBck = c_temp;
   }
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


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

VWPPhysMinimumRequirements::VWPPhysMinimumRequirements( string iName )
{
   fName = iName;

   fEnergyRange_TeV_Min = -1.;
   fEnergyRange_TeV_Max = -1.;
   fEnergyThreshold_TeV = -1.;
}

bool VWPPhysMinimumRequirements::readWPPhysMinimumRequirements( string iFile )
{
   return true;
}
