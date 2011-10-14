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
    else if(  bIntegral && bUnit == "CU" )
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
       b.setWriteParticleNumberFile( "particleNumbers.tmp.root" );
// energy range to be plotted
       b.setEnergyRange_Lin( 0.01, 150. );

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
       cout << "NOTE: plotting MC spectrum in limited energy range only (due to missing exposure)" << endl;
       if( bIntegral ) b.plotIntegralSensitivityvsEnergyFromCrabSpectrum( c, "MC", 2, iFluxUnit, 0.16, 0.003 );
       else            b.plotDifferentialSensitivityvsEnergyFromCrabSpectrum( c, "MC", 2, iFluxUnit, 0.2, 0.03 );
    }
    return;
// plot different limitations in sensitivity calculation
    a.plotSensitivityLimitations( c );

////////////////////////////////////////////////////////////////////////
// plot legend
// (might need some user adjustments)

// estimate vertical size of legend
    double iY = 0.;
    if( iData_anasumFile1 ) iY += 0.05;
    if( iData_anasumFile2 ) iY += 0.05;
    if( iMC_Gamma && iMC_Proton ) iY += 0.05;
    for( unsigned int i = 0; i < fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles.size(); i++ ) iY += 0.05;
    if( iY > 0.4 ) iY = 0.4;

    TLegend *iL = new TLegend( 0.6, 0.85-iY, 0.85, 0.6+iY );
    TGraph *g = 0;
    if( iData_anasumFile1 )
    {
       g = new TGraph( 1 );
       g->SetLineWidth( 2 );
       iL->AddEntry( g, "VERITAS 2010/2011 data", "L" );
    }
    if( iData_anasumFile2 )
    {
       g = new TGraph( 1 );
       g->SetLineWidth( 2 );
       g->SetLineColor( 2 );
       g->SetLineStyle( 1 );
       iL->AddEntry( g, "VERITAS 2010/2011 Soft cuts", "L" );
    }

    if( iMC_Gamma && iMC_Proton )
    {
       g = new TGraph( 1 );
       g->SetLineWidth( 2 );
       g->SetLineColor( 2 );
       g->SetLineStyle( 1 );
       if( iMC_Gamma )iL->AddEntry( g, "CTA MC std", "L" );
    }
    for( unsigned int i = 0; i < fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles.size(); i++ )
    {
       g = new TGraph( 1 );
       g->SetLineWidth( 2 );
       if( i < fPD.fSensitivityvsEnergyFromTextTFile_LineColor.size() ) g->SetLineColor( fPD.fSensitivityvsEnergyFromTextTFile_LineColor[i] );
       if( i < fPD.fSensitivityvsEnergyFromTextTFile_LineStyle.size() ) g->SetLineStyle( fPD.fSensitivityvsEnergyFromTextTFile_LineStyle[i] );
       iL->AddEntry( g, fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles[i].c_str(), "L" ); 
    }
    iL->Draw(); 

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

   EXPERT USE ONLY

   (probably needs some work)

*/
void plotDifferentialSensitivityRatioCTA( string iFluxUnit = "PFLUX", 
                                          char *iMC_Gamma1 = 0, char *iMC_Proton1 = 0, char *iMC_Helium1 = 0, char *iMC_Electron1 = 0, 
					  char *iMC_Gamma2 = 0, char *iMC_Proton2 = 0, char *iMC_Helium2 = 0, char *iMC_Electron2 = 0,
					  unsigned int iCrabSpec_ID = 6)
{

  cout << "ROUTINE NOT READY FOR USE" << endl;
  return;
  // get values for plotting
  bool bIntegral = false;
  getPlottingData( bIntegral, iFluxUnit );
  if( !fPD.bSet )
    {
      cout << "plotSensitivity error: no plotting data set for " << iFluxUnit << "\t" << bIntegral << endl;
      return;
    }

// sensitivity plotter
  VSensitivityCalculator a;
  a.setEnergyRange_Lin( fPD.fPlotting_energy_min_TeV, fPD.fPlotting_energy_max_TeV );           // y-axis range: in 1/cm2/s
  a.setFluxRange_PFLUX( fPD.fPlotting_flux_min_PFLUX, fPD.fPlotting_flux_max_PFLUX );           // x-axis range: in TeV
  a.setPlotCanvasSize( 900, 600 );                  // size of canvase

// set Crab Nebula spectrum
  a.setEnergySpectrumfromLiterature( fPD.fESpecDataFile_CrabNebula, 1 );
  a.setEnergySpectrumfromLiterature_ID( iCrabSpec_ID );

//////////////////////////////////////////////////////////////////
// plot sensitivity canvas
  TCanvas *c = a.plotCanvas_SensitivityvsEnergy( iFluxUnit, bIntegral );
  if( !c ) return;

//////////////////////////////////////////////////////////////////
// plot sensitivities from text files
    for( unsigned int i = 0; i < fPD.fSensitivityvsEnergyFromTextTFile.size(); i++ )
    {
        a.plotSensitivityvsEnergyFromTextTFile(c, fPD.fSensitivityvsEnergyFromTextTFile[i], fPD.fSensitivityvsEnergyFromTextTFile_LineColor[i], 3, fPD.fSensitivityvsEnergyFromTextTFile_LineStyle[i], iFluxUnit );
    }
////////////////////////////////////////////////////////////////////////
// plot MC 1
  if( iMC_Gamma1 && iMC_Proton1 )
    {
      VSensitivityCalculator b;
// set significance parameters
      b.setSignificanceParameter( 5., 10., 50., 50. );
      b.setDebug( false );
// draw some debugging plots
      char hname[600];
      sprintf( hname, "plotDebug_%d", bIntegral );
      b.setPlotDebug( hname );
// energy range to be plotted
      b.setEnergyRange_Lin( 0.5, 100. );

      i_Azbin_gamma = 0;
      i_index_gamma = 2.5;
      i_noise_gamma = 250;
      i_woff_gamma = 0.;
      
      i_Azbin_proton = 0;
      i_index_proton = 2.6;
      i_noise_proton = 250;
      i_woff_proton = 0.;  
      cout << "SETTING EFFECTIVE AREA SEARCH VALUES TO CTA" << endl; 

// gammas
      b.setMonteCarloParameters(1, fPD.fESpecDataFile_CrabNebula, iCrabSpec_ID, iMC_Gamma1, 20., i_Azbin_gamma, i_woff_gamma, i_noise_gamma, i_index_gamma );
// protons
      b.setMonteCarloParameters(14, fPD.fESpecDataFile_CosmicRays, 0, iMC_Proton1, 20., i_Azbin_proton, i_woff_proton, i_noise_proton, i_index_proton );
// helium
      if( iMC_Helium1 )
	{
          b.setMonteCarloParameters( 402, fPD.fESpecDataFile_CosmicRays, 1, iMC_Helium1, 20., 0, 0.0, 200, 2.0 );
	}
// electrons
      if( iMC_Electron1 )
	{
          b.setMonteCarloParameters( 2, fPD.fESpecDataFile_CosmicRays, 2, iMC_Electron1, 20., 0, 0.0, 200, 3.0 );
	}

// energy range determined by looking at number of noff events (need off events to determine sensitivity)
      cout << "NOTE: plotting MC spectrum in limited energy range only (due to missing exposure)" << endl;
      if( bIntegral ) b.plotIntegralSensitivityvsEnergyFromCrabSpectrum( c, "MC", 2, iFluxUnit, 0.16, 0.003 );
      else            b.plotDifferentialSensitivityvsEnergyFromCrabSpectrum( c, "MC", 2, iFluxUnit, 0.2, 0.03 );
    }
////////////////////////////////////////////////
// plot MC 2
  if( iMC_Gamma2 && iMC_Proton2 )
    {
      VSensitivityCalculator b;
      b.setDebug( true );
// draw some debugging plots
      char hname[600];
      sprintf( hname, "plotDebug_%d", bIntegral );
      b.setPlotDebug( hname );
// energy range to be plotted
      b.setEnergyRange_Lin( 0.5, 100. );

      i_Azbin_gamma = 0;
      i_index_gamma = 2.5;
      i_noise_gamma = 250;
      i_woff_gamma = 0.;
      
      i_Azbin_proton = 0;
      i_index_proton = 2.6;
      i_noise_proton = 250;
      i_woff_proton = 0.;  
      cout << "SETTING EFFECTIVE AREA SEARCH VALUES TO CTA" << endl; 

// gammas
      b.setMonteCarloParameters(1, fPD.fESpecDataFile_CrabNebula, iCrabSpec_ID, iMC_Gamma2, 20., i_Azbin_gamma, i_woff_gamma, i_noise_gamma, i_index_gamma );
// protons
      b.setMonteCarloParameters(14, fPD.fESpecDataFile_CosmicRays, 0, iMC_Proton2, 20., i_Azbin_proton, i_woff_proton, i_noise_proton, i_index_proton );
// helium
      if( iMC_Helium2 )
	{
          b.setMonteCarloParameters( 402, fPD.fESpecDataFile_CosmicRays, 1, iMC_Helium2, 20., 0, 0.0, 200, 2.0 );
	}
// electrons
      if( iMC_Electron2 )
	{
          b.setMonteCarloParameters( 2, fPD.fESpecDataFile_CosmicRays, 2, iMC_Electron2, 20., 0, 0.0, 250, 3.0 );
	}

// energy range determined by looking at number of noff events (need off events to determine sensitivity)
      cout << "NOTE: plotting MC spectrum in limited energy range only (due to missing exposure)" << endl;
      if( bIntegral ) b.plotIntegralSensitivityvsEnergyFromCrabSpectrum( c, "MC", 2, iFluxUnit, 0.16, 0.003 );
      else            b.plotDifferentialSensitivityvsEnergyFromCrabSpectrum( c, "MC", 3, iFluxUnit, 0.2, 0.03 );
    }

  a.plotSensitivityLimitations( c );


////////////////////////////////////////////////////////////////////////
// plot legend

// estimate vertical size of legend
  double iY = 0.05;
  //  if( ifile1 ) iY += 0.05;
  //  if( iData_anasumFile2 ) iY += 0.05;
  if( iMC_Gamma1 && iMC_Proton1 ) iY += 0.05;
  if( iMC_Gamma2 && iMC_Proton2 ) iY += 0.05;
  for( unsigned int i = 0; i < fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles.size(); i++ ) iY += 0.05;
  if( iY > 0.4 ) iY = 0.4;

  TLegend *iL = new TLegend( 0.6, 0.85-iY, 0.85, 0.6+iY );
//   TGraph *g = 0;
//   if( iMC_Gamma1 && iMC_Proton1 )
//     {
//       g = new TGraph( 1 );
//       g->SetLineWidth( 2 );
//       g->SetLineColor( 2 );
//       g->SetLineStyle( 1 );
//       if( iMC_Gamma1 )iL->AddEntry( g, "EVNDISP (#tels > 3)", "L" );
//     }
//   if( iMC_Gamma2 && iMC_Proton2 )
//     {
//       g = new TGraph( 1 );
//       g->SetLineWidth( 2 );
//       g->SetLineColor( 3 );
//       g->SetLineStyle( 1 );
//       if( iMC_Gamma2 )iL->AddEntry( g, "EVNDISP (TMVA)", "L" );
//     }

//   for( unsigned int i = 0; i < fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles.size(); i++ )
//     {
//       g = new TGraph( 1 );
//       g->SetLineWidth( 2 );
//       if( i < fPD.fSensitivityvsEnergyFromTextTFile_LineColor.size() ) g->SetLineColor( fPD.fSensitivityvsEnergyFromTextTFile_LineColor[i] );
//       if( i < fPD.fSensitivityvsEnergyFromTextTFile_LineStyle.size() ) g->SetLineStyle( fPD.fSensitivityvsEnergyFromTextTFile_LineStyle[i] );
//       iL->AddEntry( g, fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles[i].c_str(), "L" ); 
//     }
//   iL->Draw(); 



}


/*

   EXPERT USE ONLY

   (probably needs some work)

*/
void plotSensitivityRatio( string iFluxUnit = "PFLUX", 
			   char *iMC_Gamma1 = 0, char *iMC_Proton1 = 0, char *iMC_Electron1 = 0, 
			   char *iMC_Gamma2 = 0, char *iMC_Proton2 = 0, char *iMC_Electron2 = 0, 
			   char *iMC_Gamma3 = 0, char *iMC_Proton3 = 0, char *iMC_Electron3 = 0, 
			   char *iMC_Gamma4 = 0, char *iMC_Proton4 = 0, char *iMC_Electron4 = 0, 
			   char *iMC_Gamma5 = 0, char *iMC_Proton5 = 0, char *iMC_Electron5 = 0, 
			   unsigned int iCrabSpec_ID = 6)
{
  // get values for plotting
  bool bIntegral = false;
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
  a.setPlotCanvasSize( 900, 600 );                  // size of canvase

// set Crab Nebula spectrum
  a.setEnergySpectrumfromLiterature( fPD.fESpecDataFile_CrabNebula, 1 );
  a.setEnergySpectrumfromLiterature_ID( iCrabSpec_ID );

//////////////////////////////////////////////////////////////////
// plot sensitivity canvas
  TCanvas *c = a.plotCanvas_SensitivityvsEnergy( iFluxUnit, bIntegral );
  if( !c ) return;

//////////////////////////////////////////////////////////////////
// plot sensitivities from text files
  for( unsigned int i = 0; i < fPD.fSensitivityvsEnergyFromTextTFile.size(); i++ )
    {
      a.plotSensitivityvsEnergyFromTextTFile(c, fPD.fSensitivityvsEnergyFromTextTFile[i], fPD.fSensitivityvsEnergyFromTextTFile_LineColor[i], 3, fPD.fSensitivityvsEnergyFromTextTFile_LineStyle[i], iFluxUnit );
    }

////////////////////////////////////////////////////////////////////////
// plot MC 1
  if( iMC_Gamma1 && iMC_Proton1 )
    {
      VSensitivityCalculator b;
      b.setDebug( false );
// draw some debugging plots
      char hname[600];
      sprintf( hname, "plotDebug_%d", bIntegral );
      b.setPlotDebug( hname );
// energy range to be plotted
      b.setEnergyRange_Lin( 0.5, 100. );

      i_Azbin_gamma = 0;
      i_index_gamma = 2.5;
      i_noise_gamma = 250;
      i_woff_gamma = 0.;
      
      i_Azbin_proton = 0;
      i_index_proton = 2.6;
      i_noise_proton = 250;
      i_woff_proton = 0.;  
      cout << "SETTING EFFECTIVE AREA SEARCH VALUES TO CTA" << endl; 

// gammas
      b.setMonteCarloParameters(1, fPD.fESpecDataFile_CrabNebula, iCrabSpec_ID, iMC_Gamma1, 20., i_Azbin_gamma, i_woff_gamma, i_noise_gamma, i_index_gamma );
// protons
      b.setMonteCarloParameters(14, fPD.fESpecDataFile_CosmicRays, 0, iMC_Proton1, 20., i_Azbin_proton, i_woff_proton, i_noise_proton, i_index_proton );
// electrons
      if( iMC_Electron1 )
	{
          b.setMonteCarloParameters( 2, fPD.fESpecDataFile_CosmicRays, 2, iMC_Electron1, 20., 0, 0.0, 200, 3.0 );
	}

// energy range determined by looking at number of noff events (need off events to determine sensitivity)
      cout << "NOTE: plotting MC spectrum in limited energy range only (due to missing exposure)" << endl;
      if( bIntegral ) b.plotIntegralSensitivityvsEnergyFromCrabSpectrum( c, "MC", 1, iFluxUnit, 0.16, 0.003 );
      else            b.plotDifferentialSensitivityvsEnergyFromCrabSpectrum( c, "MC", 1, iFluxUnit, 0.2, 0.03 );

    }
////////////////////////////////////////////////
// plot MC 2
  if( iMC_Gamma2 && iMC_Proton2 )
    {
      VSensitivityCalculator b;
      b.setDebug( true );
// draw some debugging plots
      char hname[600];
      sprintf( hname, "plotDebug_%d", bIntegral );
      b.setPlotDebug( hname );
// energy range to be plotted
      b.setEnergyRange_Lin( 0.5, 100. );

      i_Azbin_gamma = 0;
      i_index_gamma = 2.5;
      i_noise_gamma = 250;
      i_woff_gamma = 0.;
      
      i_Azbin_proton = 0;
      i_index_proton = 2.6;
      i_noise_proton = 250;
      i_woff_proton = 0.;  
      cout << "SETTING EFFECTIVE AREA SEARCH VALUES TO CTA" << endl; 

// gammas
      b.setMonteCarloParameters(1, fPD.fESpecDataFile_CrabNebula, iCrabSpec_ID, iMC_Gamma2, 20., i_Azbin_gamma, i_woff_gamma, i_noise_gamma, i_index_gamma );
// protons
      b.setMonteCarloParameters(14, fPD.fESpecDataFile_CosmicRays, 0, iMC_Proton2, 20., i_Azbin_proton, i_woff_proton, i_noise_proton, i_index_proton );
// electrons
      if( iMC_Electron2 )
	{
          b.setMonteCarloParameters( 2, fPD.fESpecDataFile_CosmicRays, 2, iMC_Electron2, 20., 0, 0.0, 250, 3.0 );
	}

// energy range determined by looking at number of noff events (need off events to determine sensitivity)
      cout << "NOTE: plotting MC spectrum in limited energy range only (due to missing exposure)" << endl;
      if( bIntegral ) b.plotIntegralSensitivityvsEnergyFromCrabSpectrum( c, "MC", 2, iFluxUnit, 0.16, 0.003 );
      else            b.plotDifferentialSensitivityvsEnergyFromCrabSpectrum( c, "MC", 2, iFluxUnit, 0.2, 0.03 );
    }

////////////////////////////////////////////////////////////////////////
// plot MC 3
  if( iMC_Gamma3 && iMC_Proton3 )
    {
      VSensitivityCalculator b;
      b.setDebug( false );
// draw some debugging plots
      char hname[600];
      sprintf( hname, "plotDebug_%d", bIntegral );
      b.setPlotDebug( hname );
// energy range to be plotted
      b.setEnergyRange_Lin( 0.5, 100. );

      i_Azbin_gamma = 0;
      i_index_gamma = 2.5;
      i_noise_gamma = 250;
      i_woff_gamma = 0.;
      
      i_Azbin_proton = 0;
      i_index_proton = 2.6;
      i_noise_proton = 250;
      i_woff_proton = 0.;  
      cout << "SETTING EFFECTIVE AREA SEARCH VALUES TO CTA" << endl; 

// gammas
      b.setMonteCarloParameters(1, fPD.fESpecDataFile_CrabNebula, iCrabSpec_ID, iMC_Gamma3, 20., i_Azbin_gamma, i_woff_gamma, i_noise_gamma, i_index_gamma );
// protons
      b.setMonteCarloParameters(14, fPD.fESpecDataFile_CosmicRays, 0, iMC_Proton3, 20., i_Azbin_proton, i_woff_proton, i_noise_proton, i_index_proton );
// electrons
      if( iMC_Electron3 )
	{
          b.setMonteCarloParameters( 2, fPD.fESpecDataFile_CosmicRays, 2, iMC_Electron3, 20., 0, 0.0, 200, 3.0 );
	}

// energy range determined by looking at number of noff events (need off events to determine sensitivity)
      cout << "NOTE: plotting MC spectrum in limited energy range only (due to missing exposure)" << endl;
      if( bIntegral ) b.plotIntegralSensitivityvsEnergyFromCrabSpectrum( c, "MC", 3, iFluxUnit, 0.16, 0.003 );
      else            b.plotDifferentialSensitivityvsEnergyFromCrabSpectrum( c, "MC", 3, iFluxUnit, 0.2, 0.03 );

    }
////////////////////////////////////////////////
// plot MC 4
  if( iMC_Gamma4 && iMC_Proton4 )
    {
      VSensitivityCalculator b;
      b.setDebug( true );
// draw some debugging plots
      char hname[600];
      sprintf( hname, "plotDebug_%d", bIntegral );
      b.setPlotDebug( hname );
// energy range to be plotted
      b.setEnergyRange_Lin( 0.5, 100. );

      i_Azbin_gamma = 0;
      i_index_gamma = 2.5;
      i_noise_gamma = 250;
      i_woff_gamma = 0.;
      
      i_Azbin_proton = 0;
      i_index_proton = 2.6;
      i_noise_proton = 250;
      i_woff_proton = 0.;  
      cout << "SETTING EFFECTIVE AREA SEARCH VALUES TO CTA" << endl; 

// gammas
      b.setMonteCarloParameters(1, fPD.fESpecDataFile_CrabNebula, iCrabSpec_ID, iMC_Gamma4, 20., i_Azbin_gamma, i_woff_gamma, i_noise_gamma, i_index_gamma );
// protons
      b.setMonteCarloParameters(14, fPD.fESpecDataFile_CosmicRays, 0, iMC_Proton4, 20., i_Azbin_proton, i_woff_proton, i_noise_proton, i_index_proton );
// electrons
      if( iMC_Electron4 )
	{
          b.setMonteCarloParameters( 2, fPD.fESpecDataFile_CosmicRays, 2, iMC_Electron4, 20., 0, 0.0, 250, 3.0 );
	}

// energy range determined by looking at number of noff events (need off events to determine sensitivity)
      cout << "NOTE: plotting MC spectrum in limited energy range only (due to missing exposure)" << endl;
      if( bIntegral ) b.plotIntegralSensitivityvsEnergyFromCrabSpectrum( c, "MC", 4, iFluxUnit, 0.16, 0.003 );
      else            b.plotDifferentialSensitivityvsEnergyFromCrabSpectrum( c, "MC", 4, iFluxUnit, 0.2, 0.03 );
    }


////////////////////////////////////////////////
// plot MC 5
  if( iMC_Gamma5 && iMC_Proton5 )
    {
      VSensitivityCalculator b;
      b.setDebug( true );
// draw some debugging plots
      char hname[600];
      sprintf( hname, "plotDebug_%d", bIntegral );
      b.setPlotDebug( hname );
// energy range to be plotted
      b.setEnergyRange_Lin( 0.5, 100. );

      i_Azbin_gamma = 0;
      i_index_gamma = 2.5;
      i_noise_gamma = 250;
      i_woff_gamma = 0.;
      
      i_Azbin_proton = 0;
      i_index_proton = 2.6;
      i_noise_proton = 250;
      i_woff_proton = 0.;  
      cout << "SETTING EFFECTIVE AREA SEARCH VALUES TO CTA" << endl; 

// gammas
      b.setMonteCarloParameters(1, fPD.fESpecDataFile_CrabNebula, iCrabSpec_ID, iMC_Gamma5, 20., i_Azbin_gamma, i_woff_gamma, i_noise_gamma, i_index_gamma );
// protons
      b.setMonteCarloParameters(14, fPD.fESpecDataFile_CosmicRays, 0, iMC_Proton5, 20., i_Azbin_proton, i_woff_proton, i_noise_proton, i_index_proton );
// electrons
      if( iMC_Electron5 )
	{
          b.setMonteCarloParameters( 2, fPD.fESpecDataFile_CosmicRays, 2, iMC_Electron5, 20., 0, 0.0, 250, 3.0 );
	}

// energy range determined by looking at number of noff events (need off events to determine sensitivity)
      cout << "NOTE: plotting MC spectrum in limited energy range only (due to missing exposure)" << endl;
      if( bIntegral ) b.plotIntegralSensitivityvsEnergyFromCrabSpectrum( c, "MC", 5, iFluxUnit, 0.16, 0.003 );
      else            b.plotDifferentialSensitivityvsEnergyFromCrabSpectrum( c, "MC", 5, iFluxUnit, 0.2, 0.03 );
    }


  //  a.plotSensitivityLimitations( c );


////////////////////////////////////////////////////////////////////////
// plot legend

// estimate vertical size of legend
  double iY = 0.05;
  //  if( ifile1 ) iY += 0.05;
  //  if( iData_anasumFile2 ) iY += 0.05;
  if( iMC_Gamma1 && iMC_Proton1 ) iY += 0.05;
  if( iMC_Gamma2 && iMC_Proton2 ) iY += 0.05;
  if( iMC_Gamma3 && iMC_Proton3 ) iY += 0.05;
  if( iMC_Gamma4 && iMC_Proton4 ) iY += 0.05;
  if( iMC_Gamma5 && iMC_Proton5 ) iY += 0.05;
  for( unsigned int i = 0; i < fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles.size(); i++ ) iY += 0.05;
  if( iY > 0.4 ) iY = 0.4;

  TLegend *iL = new TLegend( 0.6, 0.85-iY, 0.85, 0.6+iY );
  TGraph *g = 0;
  if( iMC_Gamma1 && iMC_Proton1 )
    {
      g = new TGraph( 1 );
      g->SetLineWidth( 2 );
      g->SetLineColor( 1 );
      g->SetLineStyle( 1 );
      if( iMC_Gamma1 )iL->AddEntry( g, "ID1", "L" );
    }
  if( iMC_Gamma2 && iMC_Proton2 )
    {
      g = new TGraph( 1 );
      g->SetLineWidth( 2 );
      g->SetLineColor( 2 );
      g->SetLineStyle( 1 );
      if( iMC_Gamma2 )iL->AddEntry( g, "ID2", "L" );
    }
  if( iMC_Gamma3 && iMC_Proton3 )
    {
      g = new TGraph( 1 );
      g->SetLineWidth( 2 );
      g->SetLineColor( 3 );
      g->SetLineStyle( 1 );
      if( iMC_Gamma3 )iL->AddEntry( g, "ID3", "L" );
    }
  if( iMC_Gamma4 && iMC_Proton4 )
    {
      g = new TGraph( 1 );
      g->SetLineWidth( 2 );
      g->SetLineColor( 4 );
      g->SetLineStyle( 1 );
      if( iMC_Gamma4 )iL->AddEntry( g, "ID4", "L" );
    }
  if( iMC_Gamma5 && iMC_Proton5 )
    {
      g = new TGraph( 1 );
      g->SetLineWidth( 2 );
      g->SetLineColor( 5 );
      g->SetLineStyle( 1 );
      if( iMC_Gamma5 )iL->AddEntry( g, "ID5", "L" );
    }

  for( unsigned int i = 0; i < fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles.size(); i++ )
    {
      g = new TGraph( 1 );
      g->SetLineWidth( 2 );
      if( i < fPD.fSensitivityvsEnergyFromTextTFile_LineColor.size() ) g->SetLineColor( fPD.fSensitivityvsEnergyFromTextTFile_LineColor[i] );
      if( i < fPD.fSensitivityvsEnergyFromTextTFile_LineStyle.size() ) g->SetLineStyle( fPD.fSensitivityvsEnergyFromTextTFile_LineStyle[i] );
      iL->AddEntry( g, fPD.fSensitivityvsEnergyFromTextTFile_LegendTitles[i].c_str(), "L" ); 
    }
  iL->Draw(); 
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
      hGammaEffArea->Draw( "same" );
   }

// background rates
   c = (TCanvas*)gROOT->GetListOfCanvases()->FindObject( "plotDebug_0_0" );
   if( c && hBGRate )
   {
      c->cd();
      hBGRate->Scale( 60. * 60. * iObservationTime_hours );
      hBGRate->SetLineWidth( 2 );
      hBGRate->SetLineColor( iColor );
      hBGRate->Draw( "same" );
   }

// sensitivity
   c = (TCanvas*)gROOT->GetListOfCanvases()->FindObject( "iCanvas" );
   if( c && hDiffSens )
   {
      c->cd();
      hDiffSens->SetLineWidth( 2 );
      hDiffSens->SetLineColor( iColor );
      hDiffSens->Draw( "same" );
   }
}
