/*! \class VPlotWPPhysSensitivity

*/

#include "VPlotWPPhysSensitivity.h"

VPlotWPPhysSensitivity::VPlotWPPhysSensitivity()
{
   fIRF = 0;
   setEnergyRange_Lin_TeV();
   setCrabSpectraFile();
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
   if( iData->fLegend.size() == 0 )
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
// effective areas
    TCanvas *c = fIRF->plotEffectiveArea();
    plotLegend( c, true );
    if( iPrint.size() > 0 )
    {
       sprintf( hname, "%s-EffArea.eps", iPrint.c_str() );
       if( c ) c->Print( hname );
    }
// angular resolution (68%)
    c = fIRF->plotAngularResolution();
    plotLegend( c, false );
    if( iPrint.size() > 0 )
    {
       sprintf( hname, "%s-AngRes.eps", iPrint.c_str() );
       if( c ) c->Print( hname );
    }
// angular resolution (80%)
    c = fIRF->plotAngularResolution( "energy", "80" );
    plotLegend( c, false );
// energy resolution
    c = fIRF->plotEnergyResolution( iEnergyResolutionMax );
    plotLegend( c, false );
    if( iPrint.size() > 0 )
    {
       sprintf( hname, "%s-ERes.eps", iPrint.c_str() );
       if( c ) c->Print( hname );
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

/*

    plot sensitivities and data rates for different data sets

*/
bool VPlotWPPhysSensitivity::plotSensitivity( string iPrint, double iMinSensitivity, double iMaxSensitivity, string iUnit )
{
   TCanvas *cSens = 0;
   TCanvas *cBck = 0;

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
      if( i == 0 ) c_temp = a->plotSignalBackgroundRates( cBck, true, 2.e-7, 1. );   // plot also protons and electrons
      else         c_temp = a->plotSignalBackgroundRates( cBck, false, 2.e-7, 1. );
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

bool VPlotWPPhysSensitivity::plotLegend( TCanvas *c, bool iDown )
{
   if( !c ) return false;
   c->cd();

   double x = 0.2+0.35;
   double y = 0.65;
   if( iDown ) y -= 0.5;
   TLegend *iL = new TLegend( x, y, x+0.30, y+0.22 );
   iL->SetFillColor( 0 );

   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      if( fData[i]->fFileExists )
      {
	 TGraph *g = new TGraph( 1 );
	 g->SetLineColor( fData[i]->fPlottingColor );
	 g->SetLineStyle( fData[i]->fPlottingLineStyle );
	 g->SetMarkerStyle( 1 );
	 if( fData[i]->fLegend.size() > 0 ) iL->AddEntry( g, fData[i]->fLegend.c_str(), "l" );
      }
   }
   iL->Draw();
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
      if( !is_stream.eof() ) is_stream >> i_temp.fAnalysis;
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
      if( !is_stream.eof() ) is_stream >> i_temp.fLegend;
      else                   i_temp.fLegend = "";

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
