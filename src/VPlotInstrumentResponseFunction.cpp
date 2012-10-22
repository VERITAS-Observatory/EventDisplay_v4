/*! \file  VPlotInstrumentResponseFunction.cpp
    \brief effective area plotter

    \author
    Gernot Maier
*/

#include "VPlotInstrumentResponseFunction.h"

VPlotInstrumentResponseFunction::VPlotInstrumentResponseFunction()
{
   fDebug = false;

   fName = "EA";

   fTF1_fitResolution = 0;
   setResolutionFitting();
   setCanvasSize();

   setPlottingDefaults();

}

void VPlotInstrumentResponseFunction::setPlottingDefaults()
{
   setPlottingAxis( "energy_Lin", "X", false, 0.005, 200., "energy [TeV]" );
   setPlottingAxis( "distance_Lin", "X", false, 0., 500., "distance [m]" );
   setPlottingAxis( "nimages_Lin", "X", false, 0., 5., "number of images" );

   setPlottingAxis( "effarea_Lin", "Y", true, 1.0, 5.e7, "effective area [m^{2}]" );
   setPlottingAxis( "angularesolution_Lin", "Y", false, 0., 0.25, "angular resolution [deg]" );
   setPlottingAxis( "coreresolution_Lin", "Y", false, 0., 40.0, "core resolution [m]" );
   setPlottingAxis( "energyresolution_Lin", "Y", false, 0., 0.40, "energy resolution" );

}

bool VPlotInstrumentResponseFunction::addInstrumentResponseData( int iDataID, string iFileList )
{
    ifstream is;
    is.open( iFileList.c_str(), ifstream::in);
    if( !is )
    {
        cout << "VPlotInstrumentResponseFunction::addInstrumentResponseData: error opening input file list: " << iFileList << endl;
        return false;
    }
    string is_line;
    string temp;
    while( getline( is, is_line ) )
    {
         istringstream is_stream( is_line );
         is_stream >> temp;
         if( temp != "*" ) continue;
         is_stream >> temp;
// check set number
         if( atoi( temp.c_str() ) != iDataID ) continue;
// read this line
         fData.push_back( new VInstrumentResponseFunctionReader() );
         fData.back()->setDebug( fDebug );
         fData.back()->setPlottingStyle( fData.size()+1, 1, 2., 20+fData.size() );
         fData.back()->fillData( is_line, iDataID );
    }
    is.close();

    listDataSets();

    return true;
}

bool VPlotInstrumentResponseFunction::addInstrumentResponseData( string iFile, string iA_MC )
{
   return addInstrumentResponseData( iFile, 20., 0.0, 0, 0, 200, iA_MC );
}

bool VPlotInstrumentResponseFunction::addInstrumentResponseData( string iFile, double iZe, double iWoff,
                                                                 int iAzBin, double iIndex, int iNoise, string iA_MC,
								 int iColor, int iLineStyle,
								 int iMarkerStyle, float iMarkerSize )
{
// read effective areas
    VInstrumentResponseFunctionReader *iTempEffectiveArea = new VInstrumentResponseFunctionReader();
    iTempEffectiveArea->setDebug( fDebug );
    if( iColor < 0 ) iColor = fData.size()+1;
    if( iLineStyle < 0 ) iLineStyle = 1;
    if( iMarkerStyle < 0 ) iMarkerStyle =  20+fData.size();
    if( iMarkerSize < 0 )  iMarkerSize  = 2.;
    iTempEffectiveArea->setPlottingStyle( iColor, iLineStyle, 2., iMarkerStyle, iMarkerSize );
    if( !iTempEffectiveArea->fillData( iFile, iZe, iWoff, iAzBin, iIndex, iNoise, iA_MC ) )
    {
       cout << "VPlotInstrumentResponseFunction::addInstrumentResponseData() error filling effective area data" << endl;
       return false;
    }
    fData.push_back( iTempEffectiveArea );

    listDataSets();

    return true;
}

bool VPlotInstrumentResponseFunction::removeInstrumentResponseData( int iDataSetID )
{
    if( !checkDataSetID( iDataSetID ) ) return false;

    fData.erase( fData.begin() + iDataSetID );

    listDataSets();

    return true;
}

void VPlotInstrumentResponseFunction::resetInstrumentResponseData()
{
    fData.clear();   // this is not a clean way to get rid of the data -> fix
}

TCanvas* VPlotInstrumentResponseFunction::plotEffectiveArea( double iEffAreaMax_m2 )
{
    if( fData.size() == 0 ) return 0;

// set maximum value in effective area axis
    if( iEffAreaMax_m2 > 0. ) getPlottingAxis( "effarea_Lin" )->fMaxValue = iEffAreaMax_m2;

    char hname[200];

    sprintf( hname, "cEA_EFF" );
    TCanvas *iEffectiveAreaPlottingCanvas = new TCanvas( hname, "effective area", 10, 10, fCanvasSize_X, fCanvasSize_Y );
    iEffectiveAreaPlottingCanvas->SetGridx( 0 );
    iEffectiveAreaPlottingCanvas->SetGridy( 0 );
    iEffectiveAreaPlottingCanvas->SetLeftMargin( 0.15 );
    iEffectiveAreaPlottingCanvas->SetRightMargin( 0.07 );

    TH1D *heff = new TH1D( "heff","", 100, log10( getPlottingAxis( "energy_Lin" )->fMinValue ), log10( getPlottingAxis( "energy_Lin" )->fMaxValue ) );
    heff->SetStats( 0 );
    heff->SetXTitle( "log_{10} energy [TeV]" );
    heff->SetYTitle( "effective area [m^{2}]" );
    heff->SetMinimum( getPlottingAxis( "effarea_Lin" )->fMinValue );
    heff->SetMaximum( getPlottingAxis( "effarea_Lin" )->fMaxValue );
    heff->Draw("");
    heff->Draw("AH");

    plot_nullHistogram( iEffectiveAreaPlottingCanvas, heff, getPlottingAxis( "energy_Lin" )->fLogAxis,
			   getPlottingAxis( "effarea_Lin" )->fLogAxis, 1.3,
			   getPlottingAxis( "energy_Lin" )->fMinValue, getPlottingAxis( "energy_Lin" )->fMaxValue );

    int z = 0;
    for( unsigned int i = 0; i < fData.size(); i++ )
    {
       TGraphAsymmErrors *g = 0;

       if( fData[i]->fA_MC == "A_MC" ) g = fData[i]->gEffArea_MC;
       else if( fData[i]->fA_MC == "A_PROB" )
       {
          g = fData[i]->gEffArea_Prob;
       }
       else                            g = fData[i]->gEffArea_Rec;

       if( !g )
       {
          cout << "VPlotInstrumentResponseFunction::plotEffectiveArea() warning: no graph found for data set " << i << endl;
          continue;
       }

       if( g->GetN() > 0. )
       {
          if( fDebug ) g->Print();
          g->Draw( fData[i]->fPlotOption.c_str() );
          z++;
       }
    } 
    if( z > 0 && getPlottingAxis( "effarea_Lin" )->fLogAxis ) iEffectiveAreaPlottingCanvas->SetLogy( 1 );

    return iEffectiveAreaPlottingCanvas;
}

bool VPlotInstrumentResponseFunction::checkDataSetID( unsigned int iDataSetID )
{
    if( iDataSetID >= fData.size() )
    {
       cout << "Error: data set ID out of range. Should be <" << fData.size() << endl;
       return false;
    }
    return true;
}

void VPlotInstrumentResponseFunction::plotCutEfficiency( unsigned int iDataSetID )
{
    if( !checkDataSetID( iDataSetID ) ) return;

    char hname[200];

    sprintf( hname, "cEA_cuteff_%d", iDataSetID );
    TCanvas* iCutEfficencyPlottingCanvas = new TCanvas( hname, "cut efficiency", 10, 10, fCanvasSize_X, fCanvasSize_Y );
    iCutEfficencyPlottingCanvas->SetGridx( 0 );
    iCutEfficencyPlottingCanvas->SetGridy( 0 );

    sprintf( hname, "hceff_%d", iDataSetID );
    TH1D *hceff = new TH1D( hname,"", 100, log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ), log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ) );
    hceff->SetStats( 0 );
    hceff->SetXTitle( "log_{10} energy [TeV]" );
    hceff->SetYTitle( "cut efficiency" );
    hceff->SetMinimum( 1. );
    if( fData[iDataSetID]->hCutEfficiency.size() > 0 && fData[iDataSetID]->hCutEfficiency[0] )
    {
       hceff->SetMaximum( fData[iDataSetID]->hCutEfficiency[0]->GetMaximum()*1.5 );
    }
    hceff->Draw("");
    hceff->Draw("AH");

    plot_nullHistogram( iCutEfficencyPlottingCanvas, hceff, getPlottingAxis( "energy_Lin" )->fLogAxis, true, hceff->GetYaxis()->GetTitleOffset(), getPlottingAxis( "energy_Lin" ) ->fMinValue, getPlottingAxis( "energy_Lin" ) ->fMaxValue );

    int z = 0;
    for( unsigned int i = 0; i < fData[iDataSetID]->hCutEfficiency.size(); i++ )
    {
       if( fData[iDataSetID]->hCutEfficiency[i] )
       {
          fData[iDataSetID]->hCutEfficiency[i]->Draw( "same" );
          cout << i+1 << "\t" << fData[iDataSetID]->hCutEfficiency[i]->GetName();
          cout << " (color: " << fData[iDataSetID]->hCutEfficiency[i]->GetMarkerColor();
          cout << ", marker: " << fData[iDataSetID]->hCutEfficiency[i]->GetMarkerStyle() << ")";
          cout << endl;
          z++;
       }
    } 
    if( z > 0 ) iCutEfficencyPlottingCanvas->SetLogy( 1 );
}

void VPlotInstrumentResponseFunction::plotEnergyReconstructionBias2D( unsigned int iDataSetID, double iYmin, double iYmax )
{
    if( !checkDataSetID( iDataSetID ) ) return;

    char hname[200];
    char htitle[200];

    sprintf( hname, "cREA_Eerr_%d", iDataSetID );
    sprintf( htitle, "relative error in energy reconstruction (%d)", iDataSetID );
    TCanvas* iEnergyReconstructionErrorCanvas = new TCanvas( hname, htitle, 610, 10, fCanvasSize_X, fCanvasSize_Y );
    iEnergyReconstructionErrorCanvas->SetGridx( 0 );
    iEnergyReconstructionErrorCanvas->SetGridy( 0 );
    iEnergyReconstructionErrorCanvas->Draw();

    if( fData[iDataSetID]->hEsysMCRelative2D )
    {
       fData[iDataSetID]->hEsysMCRelative2D->SetTitle( "" );
       fData[iDataSetID]->hEsysMCRelative2D->GetYaxis()->SetTitleOffset( 1.2 );
       fData[iDataSetID]->hEsysMCRelative2D->SetStats( 0 );
       fData[iDataSetID]->hEsysMCRelative2D->SetAxisRange( iYmin, iYmax, "Y" );
       fData[iDataSetID]->hEsysMCRelative2D->SetAxisRange( log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ), 
                                                           log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ), "X" );
       if( fData[iDataSetID]->hEsysMCRelative2D->GetEntries() > 0. ) iEnergyReconstructionErrorCanvas->SetLogz( 1 );
       fData[iDataSetID]->hEsysMCRelative2D->Draw( "colz" );
// line at 1
       TLine *iL = new TLine( log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ), 1., log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ), 1. );
       iL->SetLineStyle( 2 );
       iL->Draw();
    } 
}


void VPlotInstrumentResponseFunction::plotEnergyReconstructionLogBias2D( unsigned int iDataSetID, string iM, double iYmin, double iYmax )
{
    if( !checkDataSetID( iDataSetID ) ) return;

    char hname[200];
    char htitle[200];

    sprintf( hname, "cEA_Eerr_%d", iDataSetID );
    sprintf( htitle, "error in energy reconstruction (%d)", iDataSetID );
    TCanvas* iEnergyReconstructionErrorCanvas = new TCanvas( hname, htitle, 610, 10, fCanvasSize_X, fCanvasSize_Y );
    iEnergyReconstructionErrorCanvas->SetGridx( 0 );
    iEnergyReconstructionErrorCanvas->SetGridy( 0 );
    iEnergyReconstructionErrorCanvas->Draw();

    if( fData[iDataSetID]->hEsys )
    {
       fData[iDataSetID]->hEsys->SetTitle( "" );
       fData[iDataSetID]->hEsys->GetYaxis()->SetTitleOffset( 1.2 );
       fData[iDataSetID]->hEsys->SetStats( 0 );
       fData[iDataSetID]->hEsys->SetAxisRange( iYmin, iYmax, "Y" );
       fData[iDataSetID]->hEsys->SetAxisRange( log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ), log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ), "X" );
       if( fData[iDataSetID]->hEsys->GetEntries() > 0. ) iEnergyReconstructionErrorCanvas->SetLogz( 1 );
       fData[iDataSetID]->hEsys->Draw( "colz" );
// plot energy systematics
       if( iM == "mean"     && fData[iDataSetID]->gEnergyLogBias_Mean )   fData[iDataSetID]->gEnergyLogBias_Mean->Draw( "p" );
       if( iM == "median"   && fData[iDataSetID]->gEnergyLogBias_Median ) fData[iDataSetID]->gEnergyLogBias_Median->Draw( "p" );
       
// line at 0
       TLine *iL = new TLine( log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ), 0., log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ), 0. );
       iL->SetLineStyle( 2 );
       iL->Draw();
    }
}

void VPlotInstrumentResponseFunction::plotEnergyReconstructionMatrix( unsigned int iDataSetID, bool bFineBinning )
{
    if( !checkDataSetID( iDataSetID ) ) return;

    char hname[200];
    char htitle[200];

    sprintf( hname, "cEA_Ematrix_%d_%d", iDataSetID, bFineBinning );
    sprintf( htitle, "energy reconstruction matrix (%d)", iDataSetID );
    if( bFineBinning ) sprintf( htitle, "%s (fine binning)", htitle );
    TCanvas *iEnergyReconstructionMatrixCanvas = new TCanvas( hname, htitle, 610, 10, fCanvasSize_X, fCanvasSize_Y );
    iEnergyReconstructionMatrixCanvas->SetGridx( 0 );
    iEnergyReconstructionMatrixCanvas->SetGridy( 0 );
    iEnergyReconstructionMatrixCanvas->SetLeftMargin( 0.11 );
    iEnergyReconstructionMatrixCanvas->SetRightMargin( 0.13 );

    if( bFineBinning && fData[iDataSetID]->hERecMatrix )
    {
       fData[iDataSetID]->hERecMatrix->SetTitle( "" );
       fData[iDataSetID]->hERecMatrix->GetYaxis()->SetTitleOffset( 1.2 );
       fData[iDataSetID]->hERecMatrix->SetStats( 0 );
       if( fData[iDataSetID]->hERecMatrix->GetEntries() > 0. ) iEnergyReconstructionMatrixCanvas->SetLogz( 1 );
       fData[iDataSetID]->hERecMatrix->SetXTitle( "log_{10} energy_{rec} [TeV]" );
       fData[iDataSetID]->hERecMatrix->SetYTitle( "log_{10} energy_{MC} [TeV]" );
       fData[iDataSetID]->hERecMatrix->SetAxisRange( log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ), log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ), "X" );
       fData[iDataSetID]->hERecMatrix->SetAxisRange( log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ), log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ), "Y" );
       fData[iDataSetID]->hERecMatrix->Draw( "colz" );

// diagonal
       TLine *iL = new TLine( log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ),
                              log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ), 
			      log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ), 
			      log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ) );
       iL->SetLineStyle( 2 );
       iL->Draw();
    }
    else if( fData[iDataSetID]->hERecMatrixCoarse )
    {
       fData[iDataSetID]->hERecMatrixCoarse->SetTitle( "" );
       fData[iDataSetID]->hERecMatrixCoarse->GetYaxis()->SetTitleOffset( 1.2 );
       fData[iDataSetID]->hERecMatrixCoarse->SetStats( 0 );
       if( fData[iDataSetID]->hERecMatrixCoarse->GetEntries() > 0. ) iEnergyReconstructionMatrixCanvas->SetLogz( 1 );
       fData[iDataSetID]->hERecMatrixCoarse->SetXTitle( "log_{10} energy_{rec} [TeV]" );
       fData[iDataSetID]->hERecMatrixCoarse->SetYTitle( "log_{10} energy_{MC} [TeV]" );
       fData[iDataSetID]->hERecMatrixCoarse->SetAxisRange( log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ), log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ), "X" );
       fData[iDataSetID]->hERecMatrixCoarse->SetAxisRange( log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ), log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ), "Y" );
       fData[iDataSetID]->hERecMatrixCoarse->Draw( "colz" );

// diagonal
       TLine *iL = new TLine( log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ),
                              log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ), 
			      log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ), 
			      log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ) );
       iL->SetLineStyle( 2 );
       iL->Draw();
    }
}


void VPlotInstrumentResponseFunction::plotCutEfficiencyRatio( unsigned int iDataSetID, unsigned int iCutID, double iPlotMaximum )
{
    if( !checkDataSetID( iDataSetID ) && iDataSetID < 999 ) return;

    char hname[200];
    char htitle[200];

    sprintf( hname, "cEA_cuteffratio_%d_%d", iDataSetID, iCutID );
    sprintf( htitle, "cut efficiency ratio (%d, %d)", iDataSetID, iCutID ); 
    TCanvas *iCutEfficencyRatioPlottingCanvas = new TCanvas( hname, htitle, 10, 10, 600, 600 );
    iCutEfficencyRatioPlottingCanvas->SetGridx( 1 );
    iCutEfficencyRatioPlottingCanvas->SetGridy( 1 );

    sprintf( hname, "hceffratio_%d", iDataSetID );
    TH1D *hceff = new TH1D( hname,"", 100, log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ), log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ) );
    hceff->SetStats( 0 );
    hceff->SetXTitle( "log_{10} energy [TeV]" );
    hceff->SetYTitle( "cut efficiency (ratio)" );
    hceff->SetMinimum( 0. );
    hceff->SetMaximum( iPlotMaximum );
    hceff->Draw("");
    hceff->Draw("AH");

    plot_nullHistogram( iCutEfficencyRatioPlottingCanvas, hceff, getPlottingAxis( "energy_Lin" )->fLogAxis, false, hceff->GetYaxis()->GetTitleOffset(), getPlottingAxis( "energy_Lin" ) ->fMinValue, getPlottingAxis( "energy_Lin" ) ->fMaxValue );

    if( iDataSetID < 999 )
    {
       for( unsigned int i = 0; i < fData[iDataSetID]->hCutEfficiencyRelativePlots.size(); i++ )
       {
	  if( fData[iDataSetID]->hCutEfficiencyRelativePlots[i] )
	  {
	     fData[iDataSetID]->hCutEfficiencyRelativePlots[i]->Draw( "same" );
	     cout << i+1 << "\t" << fData[iDataSetID]->hCutEfficiencyRelativePlots[i]->GetName();
	     cout << " (color: " << fData[iDataSetID]->hCutEfficiencyRelativePlots[i]->GetMarkerColor();
	     cout << ", marker: " << fData[iDataSetID]->hCutEfficiencyRelativePlots[i]->GetMarkerStyle() << ")";
	     cout << endl;
	  }
       } 
    }
    else
    {
       for( unsigned int i = 0; i < fData.size(); i++ )
       {
          if( iCutID < fData[i]->hCutEfficiencyRelativePlots.size() && fData[i]->hCutEfficiencyRelativePlots[iCutID] )
	  {
	     fData[i]->hCutEfficiencyRelativePlots[iCutID]->SetMarkerStyle( 20+i );
	     fData[i]->hCutEfficiencyRelativePlots[iCutID]->Draw( "same" );
	     cout << i+1 << "\t" << fData[i]->hCutEfficiencyRelativePlots[iCutID]->GetName() << endl;
	     cout << " (color: " << fData[i]->hCutEfficiencyRelativePlots[iCutID]->GetMarkerColor() << ")" << endl;
          }
       }
    }
	      
}

void VPlotInstrumentResponseFunction::listDataSets()
{
   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      cout << i << "\t";
      cout << fData[i]->isZombie() << "\t";
      cout << fData[i]->fFile << "\t";
      cout << endl;
   }
}

unsigned int VPlotInstrumentResponseFunction::getNumberOfGoodDataSets()
{
   unsigned int z = 0;
   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      if( fData[i] && !fData[i]->isZombie() ) z++;
   }
   return z;
}

void VPlotInstrumentResponseFunction::plotEffectiveAreaRatio( unsigned int iDataSetID, double ymin, double ymax )
{
    if( !checkDataSetID( iDataSetID ) ) return;
    
    char hname[200];

    sprintf( hname, "cEA_EFFRatio" );
    TCanvas *iEffectiveAreaRatioPlottingCanvas = new TCanvas( hname, "effective area ratio", 10, 10, fCanvasSize_X, fCanvasSize_Y );
    iEffectiveAreaRatioPlottingCanvas->SetGridx( 0 );
    iEffectiveAreaRatioPlottingCanvas->SetGridy( 0 );

    TH1D *heffR = new TH1D( "heffR","", 100, log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ), log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ) );
    heffR->SetStats( 0 );
    heffR->SetXTitle( "log_{10} energy [TeV]" );
    heffR->SetYTitle( "ratio of effective areas" );
    heffR->SetMinimum( ymin );
    heffR->SetMaximum( ymax );
    heffR->Draw("");
    heffR->Draw("AH");

    plot_nullHistogram( iEffectiveAreaRatioPlottingCanvas, heffR, getPlottingAxis( "energy_Lin" )->fLogAxis, false, 1.3, getPlottingAxis( "energy_Lin" ) ->fMinValue, getPlottingAxis( "energy_Lin" ) ->fMaxValue );

    TLine *iL = new TLine( log10(getPlottingAxis( "energy_Lin" ) ->fMinValue), 1., log10(getPlottingAxis( "energy_Lin" ) ->fMaxValue), 1. );
    iL->SetLineWidth( 2 );
    iL->SetLineStyle( 1 );
    iL->Draw();

    for( unsigned int i = 0; i < fData.size(); i++ )
    {
       if( i == iDataSetID ) continue;
       TGraphAsymmErrors *g = 0;

       if( fData[i]->fA_MC == "A_MC" )
       {
          fData[i]->calculateEffectiveAreaRatios( fData[iDataSetID]->gEffArea_MC );
          g = fData[i]->gEffArea_MC_Ratio;
       }
       else if( fData[i]->fA_MC == "A_PROB" )
       {
          fData[i]->calculateEffectiveAreaRatios( fData[iDataSetID]->gEffArea_Prob );
          g = fData[i]->gEffArea_Prob_Ratio;
       }
       else
       {
          fData[i]->calculateEffectiveAreaRatios( fData[iDataSetID]->gEffArea_Rec );
          g = fData[i]->gEffArea_Rec_Ratio;
       }

       if( !g ) continue;

       g->Draw( fData[i]->fPlotOption.c_str() );
    } 
}

TCanvas* VPlotInstrumentResponseFunction::plotEnergyResolution( double ymax )
{
    if( fDebug ) cout << "VPlotInstrumentResponseFunction::plotEnergyResolution " << ymax << endl;

// canvas
    char hname[200];
    sprintf( hname, "cEA_energyResolution" );
    TCanvas *iEnergyResolutionPlottingCanvas = new TCanvas( hname, "energy resolution", 10, 10, fCanvasSize_X, fCanvasSize_Y );
    iEnergyResolutionPlottingCanvas->SetGridx( 0 );
    iEnergyResolutionPlottingCanvas->SetGridy( 0 );
    iEnergyResolutionPlottingCanvas->SetLeftMargin( 0.15 );
    iEnergyResolutionPlottingCanvas->SetRightMargin( 0.07 );

// plotting frame
    TH1D *he0 = 0;
    if( getPlottingAxis( "energy_Lin" )->fMinValue > 0. && getPlottingAxis( "energy_Lin" )->fMaxValue > 0. )
    {
       he0 = new TH1D( "he0","", 100, log10( getPlottingAxis( "energy_Lin" )->fMinValue ), log10( getPlottingAxis( "energy_Lin" )->fMaxValue ) );
       he0->SetStats( 0 );
       he0->SetXTitle( "log_{10} energy [TeV]" );
       he0->SetYTitle( "energy resolution #Delta E" );
       he0->SetMinimum( 0. );
       he0->SetMaximum( ymax );
    }
    else
    {
       cout << "VPlotInstrumentResponseFunction::plotEnergyResolution error: negative energy axis: ";
       cout << getPlottingAxis( "energy_Lin" )->fMinValue << "\t" << getPlottingAxis( "energy_Lin" )->fMaxValue << endl;
       return 0;
    }
    plot_nullHistogram( iEnergyResolutionPlottingCanvas, he0, getPlottingAxis( "energy_Lin" )->fLogAxis, 
                        false, he0->GetYaxis()->GetTitleOffset()*1.3, 
			getPlottingAxis( "energy_Lin" )->fMinValue, getPlottingAxis( "energy_Lin" )->fMaxValue );

    for( unsigned int i = 0; i < fData.size(); i++ )
    {
       if( fData[i]->gEnergyResolution )
       {
          fData[i]->gEnergyResolution->Draw( "pl" );
       }
    }

    return iEnergyResolutionPlottingCanvas;
}

void VPlotInstrumentResponseFunction::plotEnergySpectra( bool iWeighted, double iYMax )
{
    char hname[200];
    char htitle[200];

    if( iWeighted )
    {
       sprintf( hname, "cEA_energy" );
       sprintf( htitle, "energy spectra (spectral weighted)" );
    }
    else            
    {
       sprintf( hname, "cEA_energyUW" );
       sprintf( htitle, "energy spectra (not spectral weighted)" );
    }
    TCanvas* iEnergySpectraPlottingCanvas = new TCanvas( hname, htitle, 10, 10, fCanvasSize_X, fCanvasSize_Y );
    iEnergySpectraPlottingCanvas->SetGridx( 0 );
    iEnergySpectraPlottingCanvas->SetGridy( 0 );
    iEnergySpectraPlottingCanvas->SetLeftMargin( 0.15 );
    iEnergySpectraPlottingCanvas->SetRightMargin( 0.07 );

    if( iWeighted ) sprintf( hname, "he0" );
    else            sprintf( hname, "he0UW" );
    TH1D *he0 = new TH1D( hname,"", 100, log10( getPlottingAxis( "energy_Lin" )->fMinValue ),
                                         log10( getPlottingAxis( "energy_Lin" )->fMaxValue ) );
    he0->SetStats( 0 );
    he0->SetXTitle( "log_{10} energy [TeV]" );
    he0->SetYTitle( "number of events/bin" );
    he0->SetMinimum( 0.5 );
    if( fData.size() > 0 && fData[0]->hEmc && iYMax < 0. )
    {
       he0->SetMaximum( fData[0]->hEmc->GetMaximum()*1.5 );
    }
    else he0->SetMaximum( iYMax );
    he0->Draw("");
    he0->Draw("AH");

    plot_nullHistogram( iEnergySpectraPlottingCanvas, he0, getPlottingAxis( "energy_Lin" )->fLogAxis, true, he0->GetYaxis()->GetTitleOffset()*1.3, 
                                                           getPlottingAxis( "energy_Lin" )->fMinValue, getPlottingAxis( "energy_Lin" )->fMaxValue );

    for( unsigned int i = 0; i < fData.size(); i++ )
    {
       if( fData[i]->hEmc )      fData[i]->hEmc->Draw( "same" );
       if( iWeighted )
       {
	  if( fData[i]->hEcut )     fData[i]->hEcut->Draw( "same" );
	  if( fData[i]->hEcut_rec ) fData[i]->hEcut_rec->Draw( "same" );
       }
       else
       {
	  if( fData[i]->hEcutUW )     fData[i]->hEcutUW->Draw( "same" );
	  if( fData[i]->hEcut_recUW ) fData[i]->hEcut_recUW->Draw( "same" );
       }
    }
    iEnergySpectraPlottingCanvas->SetLogy( 1 );
}

void VPlotInstrumentResponseFunction::plotEnergyReconstructionLogBias( string iM, double ymin, double ymax )
{
    plotEnergyReconstructionBias( iM, ymin, ymax, true );
}

TCanvas* VPlotInstrumentResponseFunction::plotEnergyReconstructionBias( string iM, double ymin, double ymax, bool iLogBias )
{
    char hname[200];
    char htitle[200];

    sprintf( hname, "cEA_energy_bias_%d", (int)iLogBias );
    if( iLogBias ) sprintf( htitle, "log energy bias" );
    else           sprintf( htitle, "energy bias" );
    TCanvas* iEnergySystematicsPlottingCanvas = new TCanvas( hname, htitle, 10, 10, fCanvasSize_X, fCanvasSize_Y );
    iEnergySystematicsPlottingCanvas->SetGridx( 0 );
    iEnergySystematicsPlottingCanvas->SetGridy( 0 );
    iEnergySystematicsPlottingCanvas->SetLeftMargin( 0.15 );
    iEnergySystematicsPlottingCanvas->SetRightMargin( 0.07 );

    sprintf( hname, "he0_sys" );
    if( iLogBias ) sprintf( hname, "he0_sysL" );
    TH1D *he0_sys = new TH1D( hname,"", 100, log10( getPlottingAxis( "energy_Lin" ) ->fMinValue ), log10( getPlottingAxis( "energy_Lin" ) ->fMaxValue ) );
    he0_sys->SetStats( 0 );
    he0_sys->SetXTitle( "log_{10} energy [TeV]" );
    if( iLogBias ) he0_sys->SetYTitle( "energy bias (log_{10} E_{rec}/E_{MC})" );
    else           he0_sys->SetYTitle( "energy bias (E_{rec}-E_{MC})/E_{MC}" );
    he0_sys->SetMinimum( ymin );
    he0_sys->SetMaximum( ymax );
    he0_sys->Draw("");
    he0_sys->Draw("AH");

    plot_nullHistogram( iEnergySystematicsPlottingCanvas, he0_sys, getPlottingAxis( "energy_Lin" )->fLogAxis, 
                        false, he0_sys->GetYaxis()->GetTitleOffset()*1.3, 
			getPlottingAxis( "energy_Lin" )->fMinValue, getPlottingAxis( "energy_Lin" ) ->fMaxValue );

    for( unsigned int i = 0; i < fData.size(); i++ )
    {
       if( iLogBias )
       {
	  if( iM == "mean" && fData[i]->gEnergyLogBias_Mean )
	  {
	     fData[i]->gEnergyLogBias_Mean->Draw( "p" );
	     if( fDebug ) fData[i]->gEnergyLogBias_Mean->Print();
	  }
	  else if( iM == "median" && fData[i]->gEnergyLogBias_Median )
	  {
	     fData[i]->gEnergyLogBias_Median->Draw( "p" );
	     if( fDebug ) fData[i]->gEnergyLogBias_Median->Print();
	  }
	  else cout << "no (log) graph found (" << iM << ")" << endl;
       }
       else
       {
	  if( iM == "mean" && fData[i]->gEnergyBias_Mean )
	  {
	     fData[i]->gEnergyBias_Mean->Draw( "pl" );
	     if( fDebug ) fData[i]->gEnergyBias_Mean->Print();
	  }
	  else if( iM == "median" && fData[i]->gEnergyBias_Median )
	  {
	     fData[i]->gEnergyBias_Median->Draw( "p" );
	     if( fDebug ) fData[i]->gEnergyBias_Median->Print();
	  }
	  else cout << "no (lin) graph found (" << iM << ")" << endl;
       }
    }

    return iEnergySystematicsPlottingCanvas;
}

TCanvas* VPlotInstrumentResponseFunction::plotAngularResolution2D( unsigned int iDataSetID, string iXaxis, string iProbabilityString, double iEnergySlice_GeV )
{
   string iResolutionTreeName = "t_angular_resolution";
   if( iProbabilityString != "68" ) iResolutionTreeName += "_0" + iProbabilityString +"p";
   return plotResolution2D( iDataSetID, 
                            "angres" + iProbabilityString, "angular resolution vs " + iXaxis,
                            "angular resolution (" + iProbabilityString + "%) [deg]",
                            getPlottingAxis( "angularesolution_Lin" )->fMinValue,
			    getPlottingAxis( "angularesolution_Lin" )->fMaxValue, iResolutionTreeName, iXaxis, iEnergySlice_GeV );
}

TCanvas* VPlotInstrumentResponseFunction::plotAngularResolution( string iXaxis, string iProbabilityString, double iMax )
{
   string iResolutionTreeName = "t_angular_resolution";
   if( iMax > 0. ) getPlottingAxis( "angularesolution_Lin" )->fMaxValue = iMax;
   if( iProbabilityString != "68" ) iResolutionTreeName += "_0" + iProbabilityString +"p";
   return plotResolution( "angres"  + iProbabilityString, "angular resolution vs " + iXaxis + "(" + iProbabilityString + "%)",
                          "angular resolution [deg]",
                          getPlottingAxis( "angularesolution_Lin" )->fMinValue,
			  getPlottingAxis( "angularesolution_Lin" )->fMaxValue, iResolutionTreeName, iXaxis );
}

TCanvas* VPlotInstrumentResponseFunction::plotCoreResolution( string iXaxis )
{
   return plotResolution( "coreres", "core resolution vs " + iXaxis, "core resolution [m]",
                          getPlottingAxis( "coreresolution_Lin" )->fMinValue, 
			  getPlottingAxis( "coreresolution_Lin" )->fMaxValue, "t_core_resolution", iXaxis );
}

TCanvas* VPlotInstrumentResponseFunction::plotCoreResolution2D( unsigned int iDataSetID, string iXaxis )
{
   return plotResolution2D( iDataSetID, "coreres", "core resolution vs " + iXaxis, "core resolution [m]",
                            getPlottingAxis( "coreresolution_Lin" )->fMinValue, 
			    getPlottingAxis( "coreresolution_Lin" )->fMaxValue, "t_core_resolution", iXaxis );
}

TCanvas* VPlotInstrumentResponseFunction::plotEnergyResolution( string iXaxis )
{
     return plotResolution( "energyres", "energy resolution vs energy", "energy resolution",
                            getPlottingAxis( "energyresolution_Lin" )->fMinValue,
			    getPlottingAxis( "energyresolution_Lin" )->fMaxValue, "t_energy_resolution", "energy" );
}

TCanvas* VPlotInstrumentResponseFunction::plotEnergyResolution2D( unsigned int iDataSetID, string iXaxis )
{
     return plotResolution2D( iDataSetID, "energyres", "energy resolution vs energy", "energy resolution",
                              getPlottingAxis( "energyresolution_Lin" )->fMinValue,
			      getPlottingAxis( "energyresolution_Lin" )->fMaxValue, "t_energy_resolution", "energy" );
}

TCanvas* VPlotInstrumentResponseFunction::plotResolution2D( unsigned int iDataSetID, string iName,
                                                            string iCanvasTitle, string iYTitle,
							    double iYmin, double iYmax,
							    string iResolutionTreeName, string iXaxis,
							    double iEnergySlice_GeV )
{
    if( !checkDataSetID( iDataSetID ) ) return 0;

    if( fData.size() == 0 ) return 0;

    char hname[800];
    char htitle[800];

    unsigned int i_Plotting_Selector = 0;
    double i_Plotting_Min = 0.;
    double i_Plotting_Max = 0.;
    double i_Plotting_L_Min = 0.;
    double i_Plotting_L_Max = 0.;
    bool   i_Plotting_log = false;
    string iXTitle = "";
    if( iXaxis == "energy" )        i_Plotting_Selector = VInstrumentResponseFunctionData::E_DIFF;
    else if( iXaxis == "nimages" )  i_Plotting_Selector = VInstrumentResponseFunctionData::E_NIMAG;
    else if( iXaxis == "distance" ) i_Plotting_Selector = VInstrumentResponseFunctionData::E_DIST;
    string iPlottingAxis = iXaxis + "_Lin";
    if( getPlottingAxis( iPlottingAxis ) )
    {
       i_Plotting_Min = getPlottingAxis( iPlottingAxis )->fMinValue;
       i_Plotting_Max = getPlottingAxis( iPlottingAxis )->fMaxValue;
       if( iXaxis == "energy" )
       {
          if( getPlottingAxis( iPlottingAxis )->fMinValue > 0. ) i_Plotting_L_Min = log10( getPlottingAxis( iPlottingAxis )->fMinValue );
          else              
          {
             cout << "VPlotInstrumentResponseFunction::plotResolution2D log min axis range <=0: " << getPlottingAxis( iPlottingAxis )->fMinValue;
             return 0;
          }
          if( getPlottingAxis( iPlottingAxis )->fMaxValue > 0. ) i_Plotting_L_Max = log10( getPlottingAxis( iPlottingAxis )->fMaxValue );
          else              
          {
             cout << "VPlotInstrumentResponseFunction::plotResolution2D log max axis range <=0: " << getPlottingAxis( iPlottingAxis )->fMaxValue;
             return 0;
          }
       }
       else
       {
          i_Plotting_L_Min = i_Plotting_Min;
          i_Plotting_L_Max = i_Plotting_Max;
       }
       i_Plotting_log = getPlottingAxis( iPlottingAxis )->fLogAxis;
       iXTitle = getPlottingAxis( iPlottingAxis )->fAxisTitle;
    }
    else
    {
       cout << "VPlotInstrumentResponseFunction::plotResolution2D:: X-axis not found" << endl;
       cout << "(available X-axes: energy, nimages, distance)" << endl;
       return 0;
    }
    if( fDebug )
    {
        cout << "Axis range: ";
        cout << "X " << i_Plotting_L_Min << "\t" << i_Plotting_L_Max << "\t" << i_Plotting_log;
        cout << ", Y " << iYmin << "\t" << iYmax;
        cout << endl;
    }

// create canvas
    sprintf( hname, "c2D%s_%s_%d_%d", iName.c_str(), iXaxis.c_str(), iDataSetID, (int)iEnergySlice_GeV );
    sprintf( htitle, "%s (data set %d)", iCanvasTitle.c_str(), iDataSetID );
    TCanvas* iResolutionPlottingCanvas = new TCanvas( hname, htitle, 210, 10, fCanvasSize_X, fCanvasSize_Y );
    iResolutionPlottingCanvas->SetGridx( 0 );
    iResolutionPlottingCanvas->SetGridy( 0 );
    iResolutionPlottingCanvas->SetLeftMargin( 0.13 );
    iResolutionPlottingCanvas->SetRightMargin( 0.13 );

// get 2D histo 
    TH2D *h = 0;
    for( unsigned int j = 0; j < fData[iDataSetID]->fIRF_TreeNames.size(); j++ )
    {
        if( fData[iDataSetID]->fIRF_TreeNames[j] == iResolutionTreeName )
        {
             if( j < fData[iDataSetID]->fIRF_Data.size() && fData[iDataSetID]->fIRF_Data[j] &&
	          i_Plotting_Selector < fData[iDataSetID]->fIRF_Data[j]->f2DHisto.size() )
             {
                    h = fData[iDataSetID]->fIRF_Data[j]->f2DHisto[i_Plotting_Selector];
             }
// plot everything
             if( h )
             {
// get containment probability
                if( j < fData[iDataSetID]->fIRF_Data.size() && fData[iDataSetID]->fIRF_Data[j] 
		  && i_Plotting_Selector < fData[iDataSetID]->fIRF_Data[j]->fContainmentProbability.size() )
                {
                    sprintf( hname, "%s (%d%%)", h->GetYaxis()->GetTitle(), (int)(fData[iDataSetID]->fIRF_Data[j]->fContainmentProbability[i_Plotting_Selector]*100.) );
                    h->SetYTitle( hname );
                }
                setHistogramPlottingStyle( h, -99. );
                h->SetAxisRange( i_Plotting_L_Min, i_Plotting_L_Max, "X" );
                h->SetAxisRange( iYmin, iYmax, "Y" );
		cout << iYmin << "\t" << iYmax << endl;
// plot 2D histogram
		if( iEnergySlice_GeV < 0. )
		{
		   h->GetYaxis()->SetTitleOffset( 1.5 );
		   h->Draw( "colz" );
                }
// plot a slice of the 2D histogram
                else
		{
		   h->SetAxisRange( -1., -1., "Y" );
		   sprintf( hname, "%s_%d_%d", h->GetName(), (int)iEnergySlice_GeV, h->GetXaxis()->FindBin( log10( iEnergySlice_GeV ) ) );
		   TH1D *h1D = h->ProjectionY( hname, h->GetXaxis()->FindBin( log10( iEnergySlice_GeV ) ), h->GetXaxis()->FindBin( log10( iEnergySlice_GeV ) ) );
		   if( h1D )
		   {
		      h1D->GetXaxis()->SetTitleOffset( 1.2 );
//		      h1D = get_Cumulative_Histogram( h1D, true, true );
		      h1D->Draw();
                   }
                }
                if( fDebug )
                {
                   cout << "HISTOGRAM " << h->GetName() << endl;
                   cout << "\t entries: " << h->GetEntries() << endl;
                }
                break;
             } 
       }
    }
    if( !h )
    {
       cout << "VPlotInstrumentResponseFunction::plotResolution2D() warning: no histogram found for data set " << iDataSetID << endl;
    }

    return iResolutionPlottingCanvas;
}

/*!

   plot e.g. angular resolution

*/
TCanvas*  VPlotInstrumentResponseFunction::plotResolution( string iName, string iCanvasTitle, string iYTitle,
							   double iYmin, double iYmax, string iResolutionTreeName, string iXaxis )
{
    if( fData.size() == 0 ) return 0;

    char hname[200];

    unsigned int i_Plotting_Selector = 0;
// plotting axis
    double i_Plotting_Min = 0.;
    double i_Plotting_Max = 0.;
    double i_Plotting_L_Min = 0.;
    double i_Plotting_L_Max = 0.;
    bool   i_Plotting_log = false;
    string iXTitle = "";
    if(      iXaxis == "energy" )   i_Plotting_Selector = VInstrumentResponseFunctionData::E_DIFF;
    else if( iXaxis == "nimages" )  i_Plotting_Selector = VInstrumentResponseFunctionData::E_NIMAG;
    else if( iXaxis == "distance" ) i_Plotting_Selector = VInstrumentResponseFunctionData::E_DIST;
    string iPlottingAxis = iXaxis + "_Lin";
    if( getPlottingAxis( iPlottingAxis ) )
    {
       i_Plotting_Min = getPlottingAxis( iPlottingAxis )->fMinValue;
       i_Plotting_Max = getPlottingAxis( iPlottingAxis )->fMaxValue;
       if( iXaxis == "energy" )
       {
          if( getPlottingAxis( iPlottingAxis )->fMinValue > 0. ) i_Plotting_L_Min = log10( getPlottingAxis( iPlottingAxis )->fMinValue );
          else              
          {
             cout << "VPlotInstrumentResponseFunction::plotResolution log min axis range <=0: " << getPlottingAxis( iPlottingAxis )->fMinValue;
             return 0;
          }
          if( getPlottingAxis( iPlottingAxis )->fMaxValue > 0. ) i_Plotting_L_Max = log10( getPlottingAxis( iPlottingAxis )->fMaxValue );
          else              
          {
             cout << "VPlotInstrumentResponseFunction::plotResolution log max axis range <=0: " << getPlottingAxis( iPlottingAxis )->fMaxValue;
             return 0;
          }
       }
       else
       {
          i_Plotting_L_Min = i_Plotting_Min;
          i_Plotting_L_Max = i_Plotting_Max;
       }
       i_Plotting_log = getPlottingAxis( iPlottingAxis )->fLogAxis;
       iXTitle = getPlottingAxis( iPlottingAxis )->fAxisTitle;
    }
    else
    {
       cout << "VPlotInstrumentResponseFunction::plotResolution:: X-axis not found" << endl;
       cout << "(available X-axes: energy, nimages, distance)" << endl;
       return 0;
    }
    if( fDebug )
    {
        cout << "Axis range: ";
        cout << "X " << i_Plotting_L_Min << "\t" << i_Plotting_L_Max << "\t" << i_Plotting_log;
        cout << ", Y " << iYmin << "\t" << iYmax;
        cout << endl;
    }

// create canvas
    sprintf( hname, "c%s_%s", iName.c_str(), iXaxis.c_str() );
    TCanvas* iResolutionPlottingCanvas = new TCanvas( hname, iCanvasTitle.c_str(), 210, 10, fCanvasSize_X, fCanvasSize_Y );
    iResolutionPlottingCanvas->SetGridx( 0 );
    iResolutionPlottingCanvas->SetGridy( 0 );
    iResolutionPlottingCanvas->SetLeftMargin( 0.13 );

    sprintf( hname, "har_%s_%s", iName.c_str(), iXaxis.c_str() );
    TH1D* har = new TH1D( hname,"", 100, i_Plotting_L_Min, i_Plotting_L_Max );
    har->SetStats( 0 );
    har->SetXTitle( iXTitle.c_str() );
    har->SetYTitle( iYTitle.c_str() );
    har->SetMinimum( iYmin );
    har->SetMaximum( iYmax );
    har->Draw("");
    har->Draw("AH");

    plot_nullHistogram( iResolutionPlottingCanvas, har, i_Plotting_log, false, 1.6, i_Plotting_Min, i_Plotting_Max );

// get resolution graphs for the whole data sample
    int z = 0;
    for( unsigned int i = 0; i < fData.size(); i++ )
    {
// get graph
       TGraphErrors *g = 0;
       for( unsigned int j = 0; j < fData[i]->fIRF_TreeNames.size(); j++ )
       {
           if( fData[i]->fIRF_TreeNames[j] == iResolutionTreeName )
           {
                if( j < fData[i]->fIRF_Data.size() && fData[i]->fIRF_Data[j]
		 && i_Plotting_Selector < fData[i]->fIRF_Data[j]->fResolutionGraph.size() )
                {
// try to get EVNDISP resolution graph
                    g = fData[i]->fIRF_Data[j]->fResolutionGraph[i_Plotting_Selector];
                }
// get containment probability (for first data set only)
                if( g && i == 0 )
                {
                   if( j < fData[i]->fIRF_Data.size() && fData[i]->fIRF_Data[j] 
		    && i_Plotting_Selector < fData[i]->fIRF_Data[j]->fContainmentProbability.size() )
                   {
                       sprintf( hname, "%s (%d%%)", har->GetYaxis()->GetTitle(), 
		                                    (int)(fData[i]->fIRF_Data[j]->fContainmentProbability[i_Plotting_Selector]*100.) );
                       har->SetYTitle( hname );
                       plot_nullHistogram( iResolutionPlottingCanvas, har, i_Plotting_log, false, 1.6, i_Plotting_Min, i_Plotting_Max );
                   }
                } 
           }
       }
// plot null histogram
       if( i == 0 )
       {
           plot_nullHistogram( iResolutionPlottingCanvas, har, i_Plotting_log, false, 1.6, i_Plotting_Min, i_Plotting_Max );
       }
       if( !g )
       {
// try to get CTA resolution graph
	   if( iName.find( "angres" ) != string::npos )
	   {
	       if( iName.find( "80" ) != string::npos )
	       {
	          g = fData[i]->gAngularResolution80;
               }
	       else
	       {
	          g = fData[i]->gAngularResolution;
               }
	   }
	   if( !g )
	   {
	     cout << "VPlotInstrumentResponseFunction::plotResolution() warning: no graph found for data set " << i << endl;
             continue;
           }
       }
       else if( g->GetN() == 0 )
       {
          cout << "VPlotInstrumentResponseFunction::plotResolution() warning: graph without points in data set " << i << endl;
          cout << "(" << fData[i]->fIRF_TreeNames.size() << ", " << iResolutionTreeName << ")" << endl;
          cout << "(";
          for( unsigned int j = 0; j < fData[i]->fIRF_TreeNames.size(); j++ ) cout << fData[i]->fIRF_TreeNames[j] << " ";
          cout << ")" << endl;
          continue;
       }
       if( g )
       {
          if( fDebug ) g->Print();
          g->Draw( fData[i]->fPlotOption.c_str() );
          if( fFunction_fitResolution.size() > 0 ) fitResolution( g );
          z++;
       }
    }

    return iResolutionPlottingCanvas;
}

bool VPlotInstrumentResponseFunction::setResolutionFitting( string iFitFunction, double iFitXmin, double iFitXmax )
{
   fFunction_fitResolution = iFitFunction;
   fXmin_fitResolution = iFitXmin;
   fXmax_fitResolution = iFitXmax;

   return true;
}

bool VPlotInstrumentResponseFunction::fitResolution( TGraphErrors *g  )
{
   if( !g ) return false;

   char hname[400];
   sprintf( hname, "fitResolution" );
   fTF1_fitResolution = new TF1( hname, fFunction_fitResolution.c_str(), fXmin_fitResolution, fXmax_fitResolution );

   g->Fit( fTF1_fitResolution, "R" );

   return true;
}

bool VPlotInstrumentResponseFunction::write_fitResolutionFunction( string iOutName, string iName )
{
    if( !fTF1_fitResolution )
    {
       cout << "PlotInstrumentResponseFunction::write_fitResolutionFunction error: no function defined" << endl;
       return false;
    }
    TFile f( iOutName.c_str(), "RECREATE" );
    if( f.IsZombie() )
    {
       cout << "PlotInstrumentResponseFunction::write_fitResolutionFunction error opening output file: " << iOutName << endl;
       return false;
    }
    if( iName.size() > 0 ) fTF1_fitResolution->SetName( iName.c_str() );
    fTF1_fitResolution->Write();
    f.Close();

    return true;
}

TH1D* VPlotInstrumentResponseFunction::getTheta2Histogram( unsigned int iDataSetID, double i_Energy_TeV_lin )
{
    if( !checkDataSetID( iDataSetID ) ) return 0;

    if( fData.size() == 0 ) return 0;

    string iResolutionTreeName = "t_angular_resolution";

    unsigned int i_Plotting_Selector = VInstrumentResponseFunctionData::E_DIFF2;


// get 2D histo 
    TH2D *h2D = 0;
    TH1D *h1D = 0;
    for( unsigned int j = 0; j < fData[iDataSetID]->fIRF_TreeNames.size(); j++ )
    {
        if( fData[iDataSetID]->fIRF_TreeNames[j] == iResolutionTreeName )
        {
             if( j < fData[iDataSetID]->fIRF_Data.size() && fData[iDataSetID]->fIRF_Data[j] && i_Plotting_Selector < fData[iDataSetID]->fIRF_Data[j]->f2DHisto.size() )
             {
                h2D = fData[iDataSetID]->fIRF_Data[j]->f2DHisto[i_Plotting_Selector];
// plot everything
                if( h2D )
                {
                   if( i_Energy_TeV_lin > 0. )
                   {
                       char iname[600];
                       sprintf( iname, "%s_%d_%f", h2D->GetName(), iDataSetID, i_Energy_TeV_lin );
                       h1D = h2D->ProjectionY( iname, h2D->GetXaxis()->FindBin( log10(i_Energy_TeV_lin) )-2, h2D->GetXaxis()->FindBin( log10(i_Energy_TeV_lin) )+2 );
                       setHistogramPlottingStyle( h1D, iDataSetID+1, 3. );
                   }
                   else h1D = 0;
                }
             }
        }
    }

    return h1D;
}

void VPlotInstrumentResponseFunction::plotTheta2( double iTheta2AxisMax, bool iCumulative )
{
   vector< double > i_temp_vector;
   plotTheta2( i_temp_vector, iTheta2AxisMax, iCumulative );
}

void VPlotInstrumentResponseFunction::plotTheta2( vector< double > i_Energy_TeV_lin, double iTheta2AxisMax, bool iCumulative )
{
   if( i_Energy_TeV_lin.size() == 0 )
   {
      i_Energy_TeV_lin.push_back( 0.3 );
      i_Energy_TeV_lin.push_back( 0.5 );
      i_Energy_TeV_lin.push_back( 1.0 );
      i_Energy_TeV_lin.push_back( 5.0 );
   }

   char hname[600];
   sprintf( hname, "Theta2_ID_%d", iCumulative );
   TCanvas *c = new TCanvas( hname, hname, 10, 10, 600, 600 );
   c->Divide( TMath::Nint( sqrt( i_Energy_TeV_lin.size() ) ), TMath::Nint( sqrt( i_Energy_TeV_lin.size() ) ) );
   for( unsigned int j = 0; j < i_Energy_TeV_lin.size(); j++ )
   {
      c->cd( j+1 );
      gPad->SetGridx( 0 );
      gPad->SetGridy( 0 );
// histogram frame
      sprintf( hname, "hTheta2_ID_%d_%d", iCumulative, j );
      TH1D *hnull = new TH1D( hname, "", 100, 0., iTheta2AxisMax );
      hnull->SetXTitle( "#Theta^{2}" );
      hnull->SetMaximum( 1.1 );
      hnull->SetStats( 0 );
      plot_nullHistogram( (TPad*)gPad, hnull, false, false, 1.3, 0., iTheta2AxisMax );
      hnull->GetXaxis()->SetNdivisions( 505 );
      hnull->Draw();

      sprintf( hname, "%.1f TeV", i_Energy_TeV_lin[j] );
      TText *iT = new TText( iTheta2AxisMax*0.6, hnull->GetMaximum()*0.7, hname );
      iT->Draw();

   }

// loop over all data sets
   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      for( unsigned int j = 0; j < i_Energy_TeV_lin.size(); j++ )
      {
         c->cd( j+1 );
         gPad->SetGridx( 0 );
         gPad->SetGridy( 0 );

         TH1D *h = getTheta2Histogram( i, i_Energy_TeV_lin[j] );
         if( h )
         {
            if( iCumulative ) h = get_Cumulative_Histogram( h, true, true );
            else
            {
// rebin
               h->Rebin( 2 );
// normalize
               if( h->GetMaximum() > 0. ) h->Scale( 1./h->GetMaximum() );
            }
            h->Draw( "same" );
         }
      }
    }
}

