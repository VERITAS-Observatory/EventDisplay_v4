/* \file plot_sensitivity.C
   \brief example script of how to plot integral or differential sensitivities

   do help() for help...

   plot sensitivities from Crab Nebula data or simulations (CTA or VERITAS)

   Author: Gernot Maier, Heike Prokoph

*/

#include "TCanvas.h"
#include "TGraph.h"
#include "TLegend.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

void getPlottingData( bool bIntegral, string bUnit );
void plotSensitivity( char *iData_anasumFile1, char *iData_anasumFile2, bool bIntegral,
                      char *iMC_Gamma, char *iMC_Proton, char *iMC_Helium, char *iMC_Electron,
		      string iFluxUnit, unsigned int iCrabSpec_ID );

void plotDebugComparisionPlots( string iFile, int iColor, double iObservationTime_hours );

/*

*/
void help()
{
    cout << endl;
    cout << "plot sensitivity from Crab Nebula data (VTS) or MC effective areas files (VTS and CTA)" << endl;
    cout << "--------------------------------------------------------------------------------------" << endl;
    cout << endl;
    cout << endl;
    cout << "plot differential sensitivity: " << endl;
    cout << "------------------------------"  << endl;
    cout << endl;
    cout << "plotDifferentialSensitivity( string fFluxUnit, char *ifile1 = 0, char *ifile2 = 0, char *iMC_Gamma = 0, char *iMC_Proton = 0, ... ) " << endl;
    cout << endl;
    cout << "flux units: " << endl;
    cout << "\t PFLUX \t Flux Sensitivity [cm^{-2} s^{-1} TeV^{-1}]" << endl;
    cout << "\t ENERG \t E^{2} x Flux Sensitivity [erg cm^{-2} s^{-1}]" << endl;
    cout << "\t CU \t Flux Sensitivity in Crab Units [C.U.]" << endl;
}

/*

   plotting data (axes values, sensitivity text files, etc)

*/
struct cSensitivityPlottingData
{
    bool bSet;
    bool bIntegral;

    string fESpecDataFile_CrabNebula;
    string fESpecDataFile_CosmicRays;
    double fPlotting_flux_min_PFLUX;
    double fPlotting_flux_max_PFLUX;
    double fPlotting_energy_min_TeV;
    double fPlotting_energy_max_TeV;
    vector< string > fSensitivityvsEnergyFromTextTFile;
    vector< string > fSensitivityvsEnergyFromTextTFile_LegendTitles;
    vector< int >    fSensitivityvsEnergyFromTextTFile_LineColor;
    vector< int >    fSensitivityvsEnergyFromTextTFile_LineStyle;

};

cSensitivityPlottingData fPD;

/*!
 
     get plotting parameters 
     (axis dimensions, etc)

*/
void getPlottingData( bool bIntegral, string bUnit )
{
    fPD.bSet = false;
    fPD.fSensitivityvsEnergyFromTextTFile.clear();
    fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles.clear();
    fPD.fSensitivityvsEnergyFromTextTFile_LineColor.clear();
    fPD.fSensitivityvsEnergyFromTextTFile_LineStyle.clear();

// Crab Nebula and Cosmic ray fluxes
    fPD.fESpecDataFile_CrabNebula = "$EVNDISPDATA/AstroData/TeV_data/EnergySpectrum_literatureValues_CrabNebula.dat";
    fPD.fESpecDataFile_CosmicRays = "$EVNDISPDATA/AstroData/TeV_data/EnergySpectrum_literatureValues_CR.dat";

// energy axis (min and max in TeV)
    fPD.fPlotting_energy_min_TeV = 0.01;
    fPD.fPlotting_energy_max_TeV = 150.00;

//////////////////
// integral flux
    cout << "FILLING " << bIntegral << "\t" << bUnit << endl;
    if( bIntegral && bUnit == "PFLUX" )
    {
       fPD.bIntegral = true;
       fPD.fPlotting_flux_min_PFLUX = 2.e-15;
       fPD.fPlotting_flux_max_PFLUX = 8.e-09;
       fPD.fSensitivityvsEnergyFromTextTFile.push_back( "$EVNDISPDATA/AstroData/TeV_data/sensitivity/HESS_IntegralSensitivity_Achieved_Moriond2009.txt" );
       fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles.push_back( "HESS" );
       fPD.fSensitivityvsEnergyFromTextTFile.push_back( "$EVNDISPDATA/AstroData/TeV_data/sensitivity/MAGIC_IntegralSensitivity_Data_Colin2010_PFLUX.txt" );
       fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles.push_back( "MAGIC" );
       fPD.fSensitivityvsEnergyFromTextTFile.push_back( "$EVNDISPDATA/AstroData/TeV_data/sensitivity/MAGICII_IntegralSensitivity_MC_Colin2010_PFLUX.txt" );
       fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles.push_back( "MAGIC II" );
       fPD.fSensitivityvsEnergyFromTextTFile.push_back( "$EVNDISPDATA/AstroData/TeV_data/sensitivity/GLAST5Y_IntegralSensitivity_LOI_2009_PFLUX.txt" );
       fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles.push_back( "Fermi LAT 5y" );

       fPD.bSet = true;
    }
    else if( bIntegral && bUnit == "CU" )
    {
       fPD.bIntegral = true;
       fPD.fPlotting_flux_min_PFLUX = 2.e-15;
       fPD.fPlotting_flux_max_PFLUX = 8.e-09;
      
       fPD.bSet = true;
    }
////////////////////
// differential flux
    else if( !bIntegral && bUnit == "PFLUX" )
    {
       fPD.bIntegral = false;
       fPD.fPlotting_flux_min_PFLUX = 2.e-15;
       fPD.fPlotting_flux_max_PFLUX = 8.e-09;
       fPD.fSensitivityvsEnergyFromTextTFile.push_back( "$EVNDISPDATA/AstroData/TeV_data/sensitivity/CTA_Typical_DifferentialSensitivity_020dE_LOI_2009_PFLUX.txt" );
       fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles.push_back( "CTA_LOI" );

       fPD.bSet = true;
    }
    else if( !bIntegral && bUnit == "CU" )
    {
        fPD.fSensitivityvsEnergyFromTextTFile.push_back( "$EVNDISPDATA/AstroData/TeV_data/sensitivity/CTA_Typical_DifferentialSensitivity_020dE_Zurich2009_CU.txt" );
        fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles.push_back( "KB_Zurich2009" );

        fPD.bSet = true;
    }
    else if( !bIntegral && bUnit == "ENERGY" )
    {
        fPD.fSensitivityvsEnergyFromTextTFile.push_back( "$EVNDISPDATA/AstroData/TeV_data/sensitivity/CTA_Typical_DifferentialSensitivity_020dE_Zurich2009_CU.txt" );
        fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles.push_back( "KB_Zurich2009" );

        fPD.bSet = true;
    }

// marker colors and line styles
    for( unsigned int i = 0; i < fPD.fSensitivityvsEnergyFromTextTFile.size(); i++ )
    {
        fPD.fSensitivityvsEnergyFromTextTFile_LineColor.push_back( 6+i );
        fPD.fSensitivityvsEnergyFromTextTFile_LineStyle.push_back( 6 );
    }
}

////////////////////////////////////////////////////////////////////////////////////
/*

   plot integral sensitivity from measured data (using Crab spectra) and MC

*/
////////////////////////////////////////////////////////////////////////////////////
void plotIntegralSensitivity( string iFluxUnit = "PFLUX",
                              char *ifile1 = 0, char *ifile2 = 0,
			      char *iMC_Gamma = 0, char *iMC_Proton = 0, char *iMC_Helium = 0, char *iMC_Electron = 0,
			      unsigned int iCrabSpec_ID = 6 )
{
     plotSensitivity( ifile1, ifile2, true, iMC_Gamma, iMC_Proton, iMC_Helium, iMC_Electron, iFluxUnit, iCrabSpec_ID );
}

////////////////////////////////////////////////////////////////////////////////////
/*

   plot differential sensitivity from measured data (using Crab spectra) and MC


*/
////////////////////////////////////////////////////////////////////////////////////
void plotDifferentialSensitivity( string iFluxUnit = "PFLUX",
                                  char *ifile1 = 0, char *ifile2 = 0,
				  char *iMC_Gamma = 0, char *iMC_Proton = 0, char *iMC_Helium = 0, char *iMC_Electron = 0,
				  unsigned int iCrabSpec_ID = 6 )
{
     plotSensitivity( ifile1, ifile2, false, iMC_Gamma, iMC_Proton, iMC_Helium, iMC_Electron, iFluxUnit, iCrabSpec_ID );
}


////////////////////////////////////////////////////////////////////////////////////
/*

   plot sensitivity 

   this function should not be called by the user. Use plotIntegralSensitivity() or plotDifferentialSensitivity()

   iData_anasumFile1        anasum file from Crab Nebula analysis
   iData_anasumFile2        anasum file from Crab Nebula analysis
   iMC_Gamma                effective area file for primary gamma rays
   iMC_Proton               effective area file for primary protons
   iMC_Helium               effective area file for primary helium
   iMC_Electron             effective area file for primary electron
   iFluxUnit                flux units (see help() )
   iCrabSpec_ID             Crab Nebula spectrum ID (read from text file with spectral parameters)

*/
////////////////////////////////////////////////////////////////////////////////////
void plotSensitivity( char *iData_anasumFile1, char *iData_anasumFile2, bool bIntegral,
                      char *iMC_Gamma, char *iMC_Proton, char *iMC_Helium, char *iMC_Electron,
		      string iFluxUnit, unsigned int iCrabSpec_ID )
{

// get values for plotting
    getPlottingData( bIntegral, iFluxUnit );
    if( !fPD.bSet )
    {
       cout << "plotSensitivity error: no plotting datat set for " << iFluxUnit << "\t" << bIntegral << endl;
       return;
    }

// sensitivity plotter
    VSensitivityCalculator a;
    a.setEnergyRange_Lin( fPD.fPlotting_energy_min_TeV, fPD.fPlotting_energy_max_TeV );           // x-axis range: in TeV
    a.setFluxRange_PFLUX( fPD.fPlotting_flux_min_PFLUX, fPD.fPlotting_flux_max_PFLUX );           // y-axis range: in 1/cm2/s
    a.setPlotCanvasSize( 900, 600 );                                                              // size of canvases

// set Crab Nebula spectrum used for relative flux calculation
    if( !a.setEnergySpectrumfromLiterature( fPD.fESpecDataFile_CrabNebula, 1 ) ) return;
    a.setEnergySpectrumfromLiterature_ID( iCrabSpec_ID );

// lots of debug output
    a.setDebug( false );

//////////////////////////////////////////////////////////////////
// plot sensitivity canvas
    TCanvas *c = a.plotCanvas_SensitivityvsEnergy( iFluxUnit, bIntegral );
    if( !c ) return;

//////////////////////////////////////////////////////////////////
// plot sensitivities from text files
    for( unsigned int i = 0; i < fPD.fSensitivityvsEnergyFromTextTFile.size(); i++ )
    {
        a.plotSensitivityvsEnergyFromTextTFile( c, fPD.fSensitivityvsEnergyFromTextTFile[i],
	                                        fPD.fSensitivityvsEnergyFromTextTFile_LineColor[i], 3, fPD.fSensitivityvsEnergyFromTextTFile_LineStyle[i],
						iFluxUnit );
    }

//////////////////////////////////////////////////////////////////////
// calculate sensitivities from data taken towards the Crab Nebula
//////////////////////////////////////////////////////////////////////
    if( iData_anasumFile1 != 0 )
    {
        if( bIntegral ) a.plotIntegralSensitivityvsEnergyFromCrabSpectrum( c, iData_anasumFile1, 1, iFluxUnit, 0.2, 100. );
	else            a.plotDifferentialSensitivityvsEnergyFromCrabSpectrum( c, iData_anasumFile1, 1, iFluxUnit, 0.2, 0.2, 100. );
    }
// plot second file
    if( iData_anasumFile2 != 0 )
    {
        if( bIntegral ) a.plotIntegralSensitivityvsEnergyFromCrabSpectrum( c, iData_anasumFile2, 3, iFluxUnit, 0.05, 100. );
	else            a.plotDifferentialSensitivityvsEnergyFromCrabSpectrum( c, iData_anasumFile2, 3, iFluxUnit, 0.2, 0.2, 100. );
    }


////////////////////////////////////////////////////////////////////////
// calculate sensitivities from Monte Carlo effective areas
////////////////////////////////////////////////////////////////////////
    if( iMC_Gamma && iMC_Proton )
    {
       VSensitivityCalculator b;
       b.setDebug( false );             // creates lots of output
// set Crab Nebula spectrum
       b.setEnergySpectrumfromLiterature( fPD.fESpecDataFile_CrabNebula, iCrabSpec_ID );
// draw some debugging plots
       char hname[600];
       sprintf( hname, "plotDebug_%d", bIntegral );
       b.setPlotDebug( hname );
// energy range to be plotted
       b.setEnergyRange_Lin( 0.01, 150. );
// significance parameters
//       b.setSignificanceParameter( 5., 10., 50., 10. );
       b.setSignificanceParameter( 5., 1.e-5, 50., 1.e-5 );

//////////////////////////////////////////////////////////////////////////
// select bins and index from gamma and proton effective area files
// VERITAS
/*
       int i_Azbin_gamma = 0;
       double i_index_gamma = 2.0;
       int i_noise_gamma = 200;
       double i_woff_gamma = 0.5;

       int i_Azbin_proton = 0;
       double i_index_proton = 2.4;
       int i_noise_proton = 200;
       double i_woff_proton = 0.;  
       cout << "SETTING EFFECTIVE AREA SEARCH VALUES TO VERITAS" << endl; 
*/
// CTA
       int i_Azbin_gamma = 0;
       double i_index_gamma = 2.5;
       int i_noise_gamma = 250;
       double i_woff_gamma = 0.;

       int i_Azbin_proton = 0;
       double i_index_proton = 2.6;
       int i_noise_proton = 250;
       double i_woff_proton = 0.;  
       cout << "SETTING EFFECTIVE AREA SEARCH VALUES TO CTA" << endl; 
//////////////////////////////////////////////////////////////////////////


// gammas
       b.setMonteCarloParameters(1, fPD.fESpecDataFile_CrabNebula, iCrabSpec_ID, iMC_Gamma, 20.,
                                 i_Azbin_gamma, i_woff_gamma, i_noise_gamma, i_index_gamma );
// protons
       b.setMonteCarloParameters(14, fPD.fESpecDataFile_CosmicRays, 0, iMC_Proton, 20.,
                                 i_Azbin_proton, i_woff_proton, i_noise_proton, i_index_proton );
// helium (spectral index?)
       if( iMC_Helium )
       {
          b.setMonteCarloParameters( 402, fPD.fESpecDataFile_CosmicRays, 1, iMC_Helium, 20., 0, 0.0, 200, 2.0 );
       }
// electrons (spectral index?)
       if( iMC_Electron )
       {
          b.setMonteCarloParameters( 2, fPD.fESpecDataFile_CosmicRays, 2, iMC_Electron, 20., 0, 0.0, 250, 3.0 );
       }

// energy range determined by looking at number of noff events (need off events to determine sensitivity)
       if( bIntegral ) b.plotIntegralSensitivityvsEnergyFromCrabSpectrum( c, "MC", 1, iFluxUnit, 0.16, 0.003 );
       else            b.plotDifferentialSensitivityvsEnergyFromCrabSpectrum( c, "MC", 1, iFluxUnit, 0.2, 0.03 );
    }
    return;
// plot different limitations in sensitivity calculation
    a.plotSensitivityLimitations( c );
}

////////////////////////////////////////////////////////////////////////////
/*

  plot sensitivity vs time for a given gamma-ray and background rate from the Crab)

*/
void plotSensitivityVStime()
{
    VSensitivityCalculator a;
// V330, theta2 < 0.15, size > 500, mscw/mscl<0.5 (May 2008)
    a.addDataSet( 4.97, 0.12, 0.09, "VERITAS (Autumn 2009)" );
// from Crab data set Autumn 2009. LL R4 theta2 < 0.08 (mscw<0.35 && mscl<0.7; April 2010)
    a.addDataSet( 5.19, 0.27, 0.09, "VERITAS (Spring 2009)" );

    vector< double > s;
    s.push_back( 0.01 );
    s.push_back( 0.05 );
    s.push_back( 0.30 );
    a.list_sensitivity( 0 );
    a.setSourceStrengthVector_CU( s );

    TCanvas *c = a.plotObservationTimevsFlux( 0, 0, 2 );
//    a.plotObservationTimevsFlux( 1, c, 1, 2 );
    a.plotObservationTimevsFlux( 0, c, 2 );
} 

/*

   plot some debug plots for comparision

*/
void plotDebugComparisionPlots( string iFileName, int iColor, double iObservationTime_hours )
{
   TH1F *hGammaEffArea = 0;
   TH1F *hBGRate = 0;
   TH1F *hDiffSens = 0;
   TCanvas *c = 0;

   TFile *iFile = new TFile( iFileName.c_str() );
   if( iFile->IsZombie() ) return;

   hGammaEffArea = (TH1F*)iFile->Get( "EffectiveArea" );
   hBGRate       = (TH1F*)iFile->Get( "BGRate" );
   hDiffSens     = (TH1F*)iFile->Get( "DiffSens" );

// effective area canvas
   c = (TCanvas*)gROOT->GetListOfCanvases()->FindObject( "plotDebug_0_1" );
   if( c && hGammaEffArea )
   {
      c->cd();
      hGammaEffArea->SetLineWidth( 2 );
      hGammaEffArea->SetLineColor( iColor );
      hGammaEffArea->SetMarkerColor( iColor );
      hGammaEffArea->Draw( "same" );
   }

// background rates
   c = (TCanvas*)gROOT->GetListOfCanvases()->FindObject( "plotDebug_0_0" );
   if( c && hBGRate )
   {
      c->cd();
      hBGRate->Scale( 60. * iObservationTime_hours );
      hBGRate->SetLineWidth( 2 );
      hBGRate->SetLineColor( iColor );
      hBGRate->SetMarkerColor( iColor );
      hBGRate->Draw( "same" );
   }

// sensitivity
   c = (TCanvas*)gROOT->GetListOfCanvases()->FindObject( "iCanvas" );
   if( c && hDiffSens )
   {
      c->cd();
      hDiffSens->SetLineWidth( 2 );
      hDiffSens->SetLineColor( iColor );
      hDiffSens->SetMarkerColor( iColor );
      hDiffSens->Draw( "same" );
   }
}

/*

    write files with particle number spectra for on (gamma) and off (protons+electrons) counts

    files are needed e.g. for setting the optimal cut value for TMVA cuts

    use writeAllParticleNumberFiles() for writing the files for all sub arrays

*/
void writeParticleNumberFile( char *iMC_Gamma = 0, char *iMC_Proton = 0, char *iMC_Electron = 0,
                              unsigned int iCrabSpec_ID = 6, char *iParticleNumberFile = "particleNumbers.tmp.root" )
{
    string iESpecDataFile_CrabNebula = "$EVNDISPDATA/AstroData/TeV_data/EnergySpectrum_literatureValues_CrabNebula.dat";
    string iESpecDataFile_CosmicRays = "$EVNDISPDATA/AstroData/TeV_data/EnergySpectrum_literatureValues_CR.dat";

    if( iMC_Gamma && iMC_Proton )
    {
       VSensitivityCalculator b;
       b.setDebug( false );             // creates lots of output
// set Crab Nebula spectrum
       b.setEnergySpectrumfromLiterature( iESpecDataFile_CrabNebula, iCrabSpec_ID );
// draw some debugging plots
       b.setWriteParticleNumberFile( iParticleNumberFile );
// CTA
       int i_Azbin_gamma = 0;
       double i_index_gamma = 2.5;
       int i_noise_gamma = 250;
       double i_woff_gamma = 0.;

       int i_Azbin_proton = 0;
       double i_index_proton = 2.6;
       int i_noise_proton = 250;
       double i_woff_proton = 0.;  
       cout << "SETTING EFFECTIVE AREA SEARCH VALUES TO CTA" << endl; 
//////////////////////////////////////////////////////////////////////////

// gammas
       b.setMonteCarloParameters( 1, iESpecDataFile_CrabNebula, iCrabSpec_ID, iMC_Gamma, 20.,
                                  i_Azbin_gamma, i_woff_gamma, i_noise_gamma, i_index_gamma );
// protons
       b.setMonteCarloParameters( 14, iESpecDataFile_CosmicRays, 0, iMC_Proton, 20.,
                                  i_Azbin_proton, i_woff_proton, i_noise_proton, i_index_proton );
// electrons (spectral index?)
       if( iMC_Electron )
       {
          b.setMonteCarloParameters( 2, iESpecDataFile_CosmicRays, 2, iMC_Electron, 20., 0, 0.0, 250, 3.0 );
       }
       b.calculateParticleNumberGraphs_MC( 0.2 );
    }
}

/*

    write files with particle number spectra for on (gamma) and off (protons+electrons) counts

    files are needed e.g. for setting the optimal cut value for TMVA cuts

    (for all sub arrays)

*/
void writeAllParticleNumberFiles( char *iMC_Gamma = "gamma_onSource", char *iMC_Proton = "proton", char *iMC_Electron = "electron",
                                  int iOffSetCounter = 0, int iRecID = 0 )
{
   vector< string > SubArray;
   SubArray.push_back( "A" );
   SubArray.push_back( "B" );
   SubArray.push_back( "C" );
   SubArray.push_back( "D" );
   SubArray.push_back( "E" );
   SubArray.push_back( "F" );
   SubArray.push_back( "G" );
   SubArray.push_back( "H" );
   SubArray.push_back( "I" ); 
   SubArray.push_back( "J" );
   SubArray.push_back( "K" );
   SubArray.push_back( "NA" );
   SubArray.push_back( "NB" );
   SubArray.push_back( "s4-1-120" );
   SubArray.push_back( "s4-2-120" ); 
   SubArray.push_back( "s4-2-85" ); 

   char iGamma[200];
   char iProton[200];
   char iElectron[200];
   char iParticleNumberFile[200];

   for( unsigned int i = 0; i < SubArray.size(); i++ )
   {
      cout << "STARTING SUBARRAY " << SubArray[i] << endl;

      sprintf( iGamma, "%s.%s_ID%d.eff-%d.root", iMC_Gamma, SubArray[i].c_str(), iRecID, iOffSetCounter );
      sprintf( iProton, "%s.%s_ID%d.eff-%d.root", iMC_Proton, SubArray[i].c_str(), iRecID, iOffSetCounter );
      sprintf( iElectron, "%s.%s_ID%d.eff-%d.root", iMC_Electron, SubArray[i].c_str(), iRecID, iOffSetCounter );

      sprintf( iParticleNumberFile, "ParticleNumbers.%s.root", SubArray[i].c_str() );

      writeParticleNumberFile( iGamma, iProton, iElectron, 6, iParticleNumberFile );

   }
}
