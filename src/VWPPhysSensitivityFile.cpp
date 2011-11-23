/* \file VWPPhysSensitivityFile

*/

#include "VWPPhysSensitivityFile.h"

VWPPhysSensitivityFile::VWPPhysSensitivityFile()
{
    fDebug = false;

    fSubArray = "";
    fObservingTime_h = 0.;

    fDataFile_gamma_onSource = "";
    fDataFile_gamma_cone10 = "";
    fDataFile_proton = "";
    fDataFile_electron = "";

    fOutFile = 0;

    fCrabSpectrumFile = "";
    fCrabSpectrumID = 0;
    fCosmicRaySpectrumFile = "";
    fProtonSpectrumID = 0;
    fElectronSpectrumID = 2;

    fSensitivity = 0;
    fBGRate = 0;
    fBGRateSqDeg = 0;
    fProtRate = 0;
    fElecRate = 0;
    fEffArea = 0;
    fEffArea80 = 0;
    fAngRes68 = 0;
    fAngRes80 = 0;
    fEres = 0; 

    fSensitivity2D = 0;
    fBGRate2D = 0;
    fBGRateSqDeg2D = 0;
    fProtRate2D = 0;
    fElecRate2D = 0;
    fEffArea2D = 0;
    fEffArea802D = 0;
    fAngRes682D = 0;
    fAngRes802D = 0;
    fEres2D = 0; 

    hisList2D = 0;
}


bool VWPPhysSensitivityFile::initializeHistograms( int iEnergyXaxisNbins, double iEnergyXaxis_min, double iEnergyXaxis_max,
                               int iEnergyTrue2DXaxisNbins, double iEnergyTrue2DXaxis_min, double iEnergyTrue2DXaxis_max,
                               int iEnergyRec2DXaxisNbins, double iEnergyRec2DXaxis_min, double iEnergyRec2DXaxis_max )
{

   hisList = new TList();

// sensitivity and background rates
   fSensitivity = new TH1F( "DiffSens", "Diff. Sens.", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fSensitivity->SetXTitle( "log_{10} (E/TeV)" );
   fSensitivity->SetYTitle( "E^{2} dF/dE [erg cm^{-2} s^{-1}]" );
   fSensitivity->Print();
   hisList->Add( fSensitivity );

   fBGRate = new TH1F( "BGRate", "Background Rate", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fBGRate->SetXTitle( "log_{10} (E/TeV)" );
   fBGRate->SetYTitle( "background rate [1/s]" );
   hisList->Add( fBGRate );

   fProtRate = new TH1F( "ProtRate", "Proton Rate", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fProtRate->SetXTitle( "log_{10} (E/TeV)" );
   fProtRate->SetYTitle( "background rate [1/s]" );
   hisList->Add( fProtRate );

   fElecRate = new TH1F( "ElecRate", "Electron Rate", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fElecRate->SetXTitle( "log_{10} (E/TeV)" );
   fElecRate->SetYTitle( "background rate [1/s]" );
   hisList->Add( fElecRate );

   fBGRateSqDeg = new TH1F( "BGRatePerSqDeg", "Background rate per square deg", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fBGRateSqDeg->SetXTitle( "log_{10} (E/TeV)" );
   fBGRateSqDeg->SetYTitle( "background rate [1/s/deg^{2}]" );
   hisList->Add( fBGRateSqDeg );

   fEffArea = new TH1F( "EffectiveArea", "Effective Area", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fEffArea->SetXTitle( "log_{10} (E/TeV)" );
   fEffArea->SetYTitle( "effective area [m^{2}]" );
   hisList->Add( fEffArea );

   fEffAreaMC = new TH1F( "EffectiveAreaEtrue", "Effective Area in true energy", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fEffAreaMC->SetXTitle( "log_{10} (E_{MC}/TeV)" );
   fEffAreaMC->SetYTitle( "effective area [m^{2}]" );
   hisList->Add( fEffAreaMC );

   fEffArea80 = new TH1F( "EffectiveArea80", "Effective Area for 80% point-source containment", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fEffArea80->SetXTitle( "log_{10} (E/TeV)" );
   fEffArea80->SetYTitle( "effective area [m^{2}]" );
   hisList->Add( fEffArea80 );

// angular resolution histograms
   fAngRes68 = new TH1F( "AngRes", "Angular resolution (68% containment)", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fAngRes68->SetXTitle( "log_{10} (E/TeV)" );
   fAngRes68->SetYTitle( "containment radius (68%) [deg]" );
   hisList->Add( fAngRes68 );

   fAngRes80 = new TH1F( "AngRes80", "Angular resolution (80% containment)", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fAngRes80->SetXTitle( "log_{10} (E/TeV)" );
   fAngRes80->SetYTitle( "containment radius (80%) [deg]" );
   hisList->Add( fAngRes80 );

   fEres = new TH1F( "ERes", "Energy resolution", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fEres->SetXTitle( "log_{10} (E/TeV)" );
   fEres->SetYTitle( "Relative Energy resolution (68% containment around Eres/Etrue=1)" );
   hisList->Add( fEres );

// loop over all histograms
   TIter next( hisList );
   while( TH1 *obj = (TH1*)next() )
   {
      if( obj )
      {
         obj->SetStats( 0 );
	 obj->GetXaxis()->SetTitleOffset( 1.1 );
	 obj->GetYaxis()->SetTitleOffset( 1.3 );
      }
   }
   return true;
}

bool VWPPhysSensitivityFile:: initializeWobbleOffsets( vector< double > iWobble_min, vector< double > iWobble_max, string iHistogramDimension )
{

// define 2D histograms
   if( iHistogramDimension == "2D" )
   {
      hisList2D = new TList();

   }

   return true;
}

bool VWPPhysSensitivityFile::fillHistograms( string iDataDirectory )
{
   return fillHistograms1D( iDataDirectory );
}

bool VWPPhysSensitivityFile::fillHistograms1D( string iDataDirectory )
{
////////////////////////////////////////////////////////////////////////
// instrument response function reader
   VInstrumentResponseFunctionReader i_IRF;
   if( !i_IRF.fillData( iDataDirectory + fDataFile_gamma_onSource + "0.root" ) ) return false;
// fill angular resolution histograms
   i_IRF.fillResolutionHistogram( fAngRes68, "68", "t_angular_resolution" );
   i_IRF.fillResolutionHistogram( fAngRes68, "80", "t_angular_resolution" );
   i_IRF.fillResolutionHistogram( fEres, "68", "t_energy_resolution" );
// fill effective area histograms
   i_IRF.fillEffectiveAreasHistograms( fEffArea, "", fEffAreaMC );
   i_IRF.fillEffectiveAreasHistograms( fEffArea80, "80" );
   if( i_IRF.getMigrationMatrix() )
   {
      TH2F *hMigMatrix = (TH2F*)i_IRF.getMigrationMatrix()->Clone( "MigMatrix" );
      hMigMatrix->SetTitle( "Migration Matrix" );
      hMigMatrix->SetStats( 0 );
      hMigMatrix->SetXTitle( "log_{10} (E_{rec}/TeV)" );
      hMigMatrix->SetYTitle( "log_{10} (E_{MC}/TeV)" );
      hisList->Add( hMigMatrix );
   }
   if( i_IRF.getRecvsMCEnergy() ) 
   {
      TH2F *hhEsysMCRelative = (TH2F*)i_IRF.getRecvsMCEnergy()->Clone( "EestOverEtrue" );
      hhEsysMCRelative->SetTitle( "Eest/Etrue vs. Etrue" );
      hhEsysMCRelative->SetStats( 0 );
      hhEsysMCRelative->SetXTitle( "log_{10} (E_{rec}/TeV)" );
      hhEsysMCRelative->SetYTitle( "E_{rec}/E_{MC}" );
      hisList->Add( hhEsysMCRelative );
   }

////////////////////////////////////////////////////////////////////////
// sensitivity plots
    VSensitivityCalculator i_Sens;
// set Crab Nebula spectrum
    i_Sens.setEnergySpectrumfromLiterature( fCrabSpectrumFile, fCrabSpectrumID );
// energy range to be plotted
    i_Sens.setEnergyRange_Lin( 0.01, 150. );
// significance parameters
    i_Sens.setSignificanceParameter( 5., 10., fObservingTime_h, 0.05, 0.2 );
//////////////////////////////////////////////////////////////////////////
// select bins and index from gamma and proton effective area files
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
// effective area files
    string iMC_Gamma    = iDataDirectory + fDataFile_gamma_onSource + "0.root";
    string iMC_Proton   = iDataDirectory + fDataFile_proton + "0.root";
    string iMC_Electron = iDataDirectory + fDataFile_electron + "0.root";
// gammas
    i_Sens.setMonteCarloParameters(1, fCrabSpectrumFile, fCrabSpectrumID, iMC_Gamma, 20.,
                                 i_Azbin_gamma, i_woff_gamma, i_noise_gamma, i_index_gamma );
// protons
    i_Sens.setMonteCarloParameters(14, fCosmicRaySpectrumFile, fProtonSpectrumID , iMC_Proton, 20.,
			      i_Azbin_proton, i_woff_proton, i_noise_proton, i_index_proton );
// electrons (spectral index)
    if( iMC_Electron.size() > 0 )
    {
       i_Sens.setMonteCarloParameters( 2, fCosmicRaySpectrumFile, fElectronSpectrumID, iMC_Electron, 20., 0, 0.0, 250, 3.0 );
    }
    i_Sens.calculateSensitivityvsEnergyFromCrabSpectrum( "MC", "ENERGY", 0.2, 0.01, 1.e6 );
    i_Sens.fillSensitivityHistograms( fSensitivity, fBGRate, fBGRateSqDeg, fProtRate, fElecRate );

    return true;
}

bool VWPPhysSensitivityFile::initializeOutputFile( string iOutputFile )
{
   string iFileName = iOutputFile + "." + fSubArray + ".root";
// define output file
   fOutFile = new TFile( iFileName.c_str(), "RECREATE" );
   if( fOutFile->IsZombie() )
   {
       cout << "error opening output file: " << iFileName << endl;
       cout << "exiting..." << endl;
       return false;
   }
   fOutFile->SetTitle( "EVNDISP sensitivity calculation" );

   return true;
}

bool VWPPhysSensitivityFile::terminate()
{
   if( fOutFile )
   {
      fOutFile->cd();
      cout << "writing histograms to " << fOutFile->GetName() << endl;
      if( hisList )
      {
         hisList->Print();
         hisList->Write();
      }
      fOutFile->Close();
   }
   return true;
}

void VWPPhysSensitivityFile::setSubArray( string iA )
{
    fSubArray = iA;

    fDataFile_gamma_onSource = "gamma_onSource." + fSubArray + "_ID0.eff-";
    fDataFile_gamma_cone10 = "gamma_cone10." + fSubArray + "_ID0.eff-";
    fDataFile_proton = "proton." + fSubArray + "_ID0.eff-";
    fDataFile_electron = "electron." + fSubArray + "_ID0.eff-";
}
