/* \file VWPPhysSensitivityFile

*/

#include "VWPPhysSensitivityFile.h"

VWPPhysSensitivityFile::VWPPhysSensitivityFile()
{
    fDebug = false;

    fSubArray = "";
    fObservingTime_h = 0.;
    fObservatory = "CTA";

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
    fSensitivityCU = 0;
    fBGRate = 0;
    fBGRateSqDeg = 0;
    fProtRate = 0;
    fElecRate = 0;
    fEffArea = 0;
    fEffArea80 = 0;
    fAngRes68 = 0;
    fAngRes80 = 0;
    fEres = 0; 
    fEbias = 0;

    fOffsetCounter = 9999;

}


bool VWPPhysSensitivityFile::initializeHistograms( int iEnergyXaxisNbins, double iEnergyXaxis_min, double iEnergyXaxis_max,
                               int iEnergyTrue2DXaxisNbins, double iEnergyTrue2DXaxis_min, double iEnergyTrue2DXaxis_max,
                               int iEnergyRec2DXaxisNbins, double iEnergyRec2DXaxis_min, double iEnergyRec2DXaxis_max,
			       unsigned int iOffsetCounter )
{

   char hname[200];
   fOffsetCounter = iOffsetCounter;

// sensitivity and background rates
   sprintf( hname, "DiffSens" );
   if( fOffsetCounter < 9999 ) sprintf( hname, "%s_%d", hname, fOffsetCounter );
   fSensitivity = new TH1F( hname, "Diff. Sens.", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fSensitivity->SetXTitle( "log_{10} (E/TeV)" );
   fSensitivity->SetYTitle( "E^{2} dF/dE [erg cm^{-2} s^{-1}]" );
   fSensitivity->Print();
   hisList.push_back( fSensitivity );
   if( fOffsetCounter == 9999 ) hisListToDisk.push_back( fSensitivity );

   sprintf( hname, "DiffSensCU" );
   if( fOffsetCounter < 9999 ) sprintf( hname, "%s_%d", hname, fOffsetCounter );
   fSensitivityCU = new TH1F( hname, "Diff. Sens. (CU)", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fSensitivityCU->SetXTitle( "log_{10} (E/TeV)" );
   fSensitivityCU->SetYTitle( "Differential Flux Sensitivity [C.U.]" );
   fSensitivityCU->Print();
   hisList.push_back( fSensitivityCU );
   if( fOffsetCounter == 9999 ) hisListToDisk.push_back( fSensitivityCU );

   sprintf( hname, "BGRate" );
   if( fOffsetCounter < 9999 ) sprintf( hname, "%s_%d", hname, fOffsetCounter );
   fBGRate = new TH1F( hname, "Background Rate", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fBGRate->SetXTitle( "log_{10} (E/TeV)" );
   fBGRate->SetYTitle( "background rate [1/s]" );
   hisList.push_back( fBGRate );
   if( fOffsetCounter == 9999 ) hisListToDisk.push_back( fBGRate );

   sprintf( hname, "ProtRate" );
   if( fOffsetCounter < 9999 ) sprintf( hname, "%s_%d", hname, fOffsetCounter );
   fProtRate = new TH1F( hname, "Proton Rate", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fProtRate->SetXTitle( "log_{10} (E/TeV)" );
   fProtRate->SetYTitle( "background rate [1/s]" );
   hisList.push_back( fProtRate );
   if( fOffsetCounter == 9999 ) hisListToDisk.push_back( fProtRate );

   sprintf( hname, "ElecRate" );
   if( fOffsetCounter < 9999 ) sprintf( hname, "%s_%d", hname, fOffsetCounter );
   fElecRate = new TH1F( hname, "Electron Rate", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fElecRate->SetXTitle( "log_{10} (E/TeV)" );
   fElecRate->SetYTitle( "background rate [1/s]" );
   hisList.push_back( fElecRate );
   if( fOffsetCounter == 9999 ) hisListToDisk.push_back( fElecRate );

   sprintf( hname, "BGRatePerSqDeg" );
   if( fOffsetCounter < 9999 ) sprintf( hname, "%s_%d", hname, fOffsetCounter );
   fBGRateSqDeg = new TH1F( hname, "Background rate per square deg", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fBGRateSqDeg->SetXTitle( "log_{10} (E/TeV)" );
   fBGRateSqDeg->SetYTitle( "background rate [1/s/deg^{2}]" );
   hisList.push_back( fBGRateSqDeg );
   if( fOffsetCounter == 9999 ) hisListToDisk.push_back( fBGRateSqDeg );

   sprintf( hname, "EffectiveArea" );
   if( fOffsetCounter < 9999 ) sprintf( hname, "%s_%d", hname, fOffsetCounter );
   fEffArea = new TH1F( hname, "Effective Area", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fEffArea->SetXTitle( "log_{10} (E/TeV)" );
   fEffArea->SetYTitle( "effective area [m^{2}]" );
   hisList.push_back( fEffArea );
   if( fOffsetCounter == 9999 ) hisListToDisk.push_back( fEffArea );

   sprintf( hname, "EffectiveAreaEtrue" );
   if( fOffsetCounter < 9999 ) sprintf( hname, "%s_%d", hname, fOffsetCounter );
   fEffAreaMC = new TH1F( hname, "Effective Area in true energy", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fEffAreaMC->SetXTitle( "log_{10} (E_{MC}/TeV)" );
   fEffAreaMC->SetYTitle( "effective area [m^{2}]" );
   hisList.push_back( fEffAreaMC );
   if( fOffsetCounter == 9999 ) hisListToDisk.push_back( fEffAreaMC );

   sprintf( hname, "EffectiveArea80" );
   if( fOffsetCounter < 9999 ) sprintf( hname, "%s_%d", hname, fOffsetCounter );
   fEffArea80 = new TH1F( hname, "Effective Area for 80% point-source containment", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fEffArea80->SetXTitle( "log_{10} (E/TeV)" );
   fEffArea80->SetYTitle( "effective area [m^{2}]" );
   hisList.push_back( fEffArea80 );
   if( fOffsetCounter == 9999 ) hisListToDisk.push_back( fEffArea80 );

// angular resolution histograms
   sprintf( hname, "AngRes" );
   if( fOffsetCounter < 9999 ) sprintf( hname, "%s_%d", hname, fOffsetCounter );
   fAngRes68 = new TH1F( hname, "Angular resolution (68% containment)", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fAngRes68->SetXTitle( "log_{10} (E/TeV)" );
   fAngRes68->SetYTitle( "containment radius (68%) [deg]" );
   hisList.push_back( fAngRes68 );
   if( fOffsetCounter == 9999 ) hisListToDisk.push_back( fAngRes68 );

   sprintf( hname, "AngRes80" );
   if( fOffsetCounter < 9999 ) sprintf( hname, "%s_%d", hname, fOffsetCounter );
   fAngRes80 = new TH1F( hname, "Angular resolution (80% containment)", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fAngRes80->SetXTitle( "log_{10} (E/TeV)" );
   fAngRes80->SetYTitle( "containment radius (80%) [deg]" );
   hisList.push_back( fAngRes80 );
   if( fOffsetCounter == 9999 ) hisListToDisk.push_back( fAngRes80 );

   sprintf( hname, "ERes" );
   if( fOffsetCounter < 9999 ) sprintf( hname, "%s_%d", hname, fOffsetCounter );
   fEres = new TH1F( hname, "Energy resolution", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fEres->SetXTitle( "log_{10} (E/TeV)" );
//   fEres->SetYTitle( "Relative Energy resolution (68% containment around Eres/Etrue=1)" );
   fEres->SetYTitle( "energy resolution (RMS)" );
   hisList.push_back( fEres );
   if( fOffsetCounter == 9999 ) hisListToDisk.push_back( fEres );

   sprintf( hname, "Ebias" );
   if( fOffsetCounter < 9999 ) sprintf( hname, "%s_%d", hname, fOffsetCounter );
   fEbias = new TH1F( hname, "Energy bias (mean E_{rec}/E_{MC})", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
   fEbias->SetXTitle( "log_{10} (E/TeV)" );
   fEbias->SetYTitle( "energy reconstruction bias (<E_{rec}/E_{MC}>)" );
   hisList.push_back( fEbias );
   if( fOffsetCounter == 9999 ) hisListToDisk.push_back( fEbias );

// loop over all histograms
   for( unsigned int i = 0; i < hisList.size(); i++ )
   {
      if( hisList[i] )
      {
         hisList[i]->SetStats( 0 );
	 hisList[i]->GetXaxis()->SetTitleOffset( 1.1 );
	 hisList[i]->GetYaxis()->SetTitleOffset( 1.3 );
      }
   }
   return true;
}

bool VWPPhysSensitivityFile::fillHistograms2D( vector< double > iWobble_min, vector< double > iWobble_max )
{
   cout << endl << endl << "=================================================================================" << endl;
   if( iWobble_min.size() != iWobble_max.size() )
   {
      cout << "VWPPhysSensitivityFile::fillHistograms2D: error, inconsistent dimensions of wobble vectors" << endl;
      return false;
   }
   char hname[200];

// wobble offset might be variable binning
   int nbins_woff = (int) iWobble_min.size();
   if( nbins_woff > 2000 )
   {
       cout << "VWPPhysSensitivityFile::fillHistograms2D: cannot handle more than 2000 offset bins" << endl;
       return false;
   }
   Double_t woff[2000];
   for( int i = 0; i < nbins_woff; i++ )
   {
      woff[i] = iWobble_min[i];
   }
   woff[nbins_woff] = iWobble_max[iWobble_max.size()-1];

// fill all multidimensional histograms
   map< string, TH2F* > iHis2D;
   map< string, TH3F* > iHis3D;
   for( unsigned int i = 0; i < hisList.size(); i++ )
   {
      if( hisList[i] )
      {
	 string iHisName = hisList[i]->GetName();
	 string iClass   = hisList[i]->ClassName();
	 string iHisName2D;
	 for( unsigned int j = 0; j < iWobble_min.size(); j++ )
	 {
	      sprintf( hname, "_%d", j );
	      iHisName2D = iHisName.substr( 0, iHisName.find( hname ) ) + "_offaxis";
	      if( iHisName.find( hname ) != string::npos )
	      {
// firs element in loop: create new histograms
		if( j == 0 )
		{
		   if( iClass == "TH1F" )
		   {
		      iHis2D[iHisName2D] = new TH2F( iHisName2D.c_str(), hisList[i]->GetTitle(), 
		                                hisList[i]->GetNbinsX(), hisList[i]->GetXaxis()->GetXmin(), hisList[i]->GetXaxis()->GetXmax(),
						nbins_woff, woff );
		      iHis2D[iHisName2D]->SetXTitle( hisList[i]->GetXaxis()->GetTitle() );
		      iHis2D[iHisName2D]->SetYTitle( "off-axis angle [deg]" );
		      iHis2D[iHisName2D]->SetStats( 0 );
		      hisListToDisk.push_back( iHis2D[iHisName2D] );
                   }
		   else if( iClass == "TH2F" || iClass == "TH2D" )
		   {
		      iHis3D[iHisName2D] = new TH3F( iHisName2D.c_str(), hisList[i]->GetTitle(), 
		                                hisList[i]->GetNbinsX(), hisList[i]->GetXaxis()->GetXbins()->GetArray(),
		                                hisList[i]->GetNbinsY(), hisList[i]->GetYaxis()->GetXbins()->GetArray(),
						nbins_woff, woff );
		      iHis3D[iHisName2D]->SetXTitle( hisList[i]->GetXaxis()->GetTitle() );
		      iHis3D[iHisName2D]->SetYTitle( hisList[i]->GetYaxis()->GetTitle() );
		      iHis3D[iHisName2D]->SetZTitle( "off-axis angle [deg]" );
		      iHis3D[iHisName2D]->SetStats( 0 );
		      hisListToDisk.push_back( iHis3D[iHisName2D] );
                   }
		}
// fill histograms
		if( iClass == "TH1F" && iHis2D.find( iHisName2D ) != iHis2D.end() )
                {
		   for( int b = 0; b < hisList[i]->GetNbinsX(); b++ )
		   {
		      iHis2D[iHisName2D]->SetBinContent( b, j+1, hisList[i]->GetBinContent( b ) );
		      iHis2D[iHisName2D]->SetBinError( b, j+1, hisList[i]->GetBinError( b ) );
                   }
                }
		if( (iClass == "TH2F" || iClass == "TH2D") && iHis3D.find( iHisName2D ) != iHis3D.end() )
                {
		   for( int bx = 0; bx < hisList[i]->GetNbinsX(); bx++ )
		   {
		      for( int by = 0; by < hisList[i]->GetNbinsY(); by++ )
		      {
			 iHis3D[iHisName2D]->SetBinContent( bx, by, j+1, hisList[i]->GetBinContent( bx, by ) );
			 iHis3D[iHisName2D]->SetBinError( bx, by, j+1, hisList[i]->GetBinError( bx, by ) );
                      }
                   }
                }
	     }
         }
      }
   }

   return true;
}


bool VWPPhysSensitivityFile::fillHistograms1D( string iDataDirectory )
{
   char hname[2000];
////////////////////////////////////////////////////////////////////////
// instrument response function reader
   VInstrumentResponseFunctionReader i_IRF;
   if( fOffsetCounter < 9999 )
   {
      sprintf( hname, "%s/%s%d.root", iDataDirectory.c_str(), fDataFile_gamma_cone10.c_str(), fOffsetCounter );
   }
   else
   {
      if( isVTS() == 0 )
      {
         sprintf( hname, "%s/%s0.root", iDataDirectory.c_str(), fDataFile_gamma_onSource.c_str() );
      }	  
      else
      {
         sprintf( hname, "%s/%s.root", iDataDirectory.c_str(), fDataFile_gamma_onSource.c_str() );
      }	 
   }
   cout << endl;
   cout << "=================================================================" << endl;
   cout << "reading " << hname << endl;
   cout << endl;
// CTA data 
   if( isVTS() == 0 )
   {
      if( !i_IRF.fillData( hname ) ) return false;
   }
// VERITAS data
   else
   {
      if( isVTS() == 5 && !i_IRF.fillData( hname, 20, 0.5, 16, 2.0, 130 ) ) return false;
      if( isVTS() == 6 && !i_IRF.fillData( hname, 20, 0.5, 16, 2.0, 200 ) ) return false;
   }
// fill angular resolution histograms
   i_IRF.fillResolutionHistogram( fAngRes68, "68", "t_angular_resolution" );
   i_IRF.fillResolutionHistogram( fAngRes80, "80", "t_angular_resolution" );
   i_IRF.fillResolutionHistogram( fEres, "68", "t_energy_resolution" );
// fill energy bias histograms
   i_IRF.fillBiasHistograms( fEbias, "mean" );
// fill effective area histograms
   i_IRF.fillEffectiveAreasHistograms( fEffArea, "", fEffAreaMC );
   i_IRF.fillEffectiveAreasHistograms( fEffArea80, "80" );
   if( i_IRF.getMigrationMatrix() )
   {
      if( fOffsetCounter < 9999 ) sprintf( hname, "MigMatrix_%d", fOffsetCounter );
      else                        sprintf( hname, "MigMatrix" );
      TH2F *hMigMatrix = (TH2F*)i_IRF.getMigrationMatrix()->Clone( hname );
      hMigMatrix->SetTitle( "Migration Matrix" );
      hMigMatrix->SetStats( 0 );
      hMigMatrix->SetXTitle( "log_{10} (E_{rec}/TeV)" );
      hMigMatrix->SetYTitle( "log_{10} (E_{MC}/TeV)" );
      hisList.push_back( hMigMatrix );
      if( fOffsetCounter == 9999 ) hisListToDisk.push_back( hMigMatrix );
   }
   if( i_IRF.getRecvsMCEnergy() ) 
   {
      if( fOffsetCounter < 9999 ) sprintf( hname, "EestOverEtrue_%d", fOffsetCounter );
      else                        sprintf( hname, "EestOverEtrue" );
      TH2F *hhEsysMCRelative = (TH2F*)i_IRF.getRecvsMCEnergy()->Clone( hname );
      hhEsysMCRelative->SetTitle( "E_{rec}/E_{MC} vs. E_{MC}" );
      hhEsysMCRelative->SetStats( 0 );
      hhEsysMCRelative->SetXTitle( "log_{10} (E_{rec}/TeV)" );
      hhEsysMCRelative->SetYTitle( "E_{rec}/E_{MC}" );
      hisList.push_back( hhEsysMCRelative );
      if( fOffsetCounter == 9999 ) hisListToDisk.push_back( hhEsysMCRelative );
   }

////////////////////////////////////////////////////////////////////////
// sensitivity plots
    VSensitivityCalculator i_Sens;
    VSensitivityCalculator i_SensCU;
// set Crab Nebula spectrum
    i_Sens.setEnergySpectrumfromLiterature( fCrabSpectrumFile, fCrabSpectrumID );
    i_SensCU.setEnergySpectrumfromLiterature( fCrabSpectrumFile, fCrabSpectrumID );
// energy range to be plotted
    i_Sens.setEnergyRange_Lin( 0.01, 150. );
    i_SensCU.setEnergyRange_Lin( 0.01, 150. );
// significance parameters
    i_Sens.setSignificanceParameter( 5., 10., fObservingTime_h, 0.05, 0.2 );
    i_SensCU.setSignificanceParameter( 5., 10., fObservingTime_h, 0.05, 0.2 );
// energy axis
    i_Sens.setUseEffectiveAreas_vs_reconstructedEnergy( true );
    i_SensCU.setUseEffectiveAreas_vs_reconstructedEnergy( true );
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

    int i_Azbin_electron = 0;
    double i_index_electron = 3.0;
    int i_noise_electron = 250;
    double i_woff_electron = 0.;  

    if( fSubArray == "V5" )
    {
       i_Azbin_gamma = 16;
       i_index_gamma = 2.4;
       i_noise_gamma = 130;
       i_woff_gamma = 0.5;
       i_Azbin_proton = i_Azbin_gamma;
       i_index_proton = 2.6;
       i_noise_proton = i_noise_gamma;
       i_woff_proton = 0.;

       i_Azbin_electron = i_Azbin_gamma;
       i_index_electron = 3.0;
       i_noise_electron = i_noise_gamma;
       i_woff_electron = 0.;
    }
    else if( fSubArray == "V6" )
    {
       i_Azbin_gamma = 16;
       i_index_gamma = 2.0;
       i_noise_gamma = 200;
       i_woff_gamma = 0.5;
       i_Azbin_proton = i_Azbin_gamma;
       i_index_proton = 2.0;
       i_noise_proton = i_noise_gamma;
       i_woff_proton = 0.;

       i_Azbin_electron = i_Azbin_gamma;
       i_index_electron = 3.0;
       i_noise_electron = i_noise_gamma;
       i_woff_electron = 0.;
    }

    cout << "SETTING EFFECTIVE AREA SEARCH VALUES TO " << fSubArray << endl; 
//////////////////////////////////////////////////////////////////////////
// effective area files
    if( fOffsetCounter < 9999 ) sprintf( hname, "%s/%s%d.root", iDataDirectory.c_str(), fDataFile_gamma_cone10.c_str(), fOffsetCounter );
    else
    {
      if( isVTS() == 0 )
      {
	 sprintf( hname, "%s/%s0.root", iDataDirectory.c_str(), fDataFile_gamma_onSource.c_str() );
      }
      else
      {
	 sprintf( hname, "%s/%s.root", iDataDirectory.c_str(), fDataFile_gamma_onSource.c_str() );
      }
    }
    string iMC_Gamma    = hname;
    if( fOffsetCounter == 9999 )
    {
       if( isVTS() == 0 )
       {
	  sprintf( hname, "%s/%s0.root", iDataDirectory.c_str(), fDataFile_proton.c_str() );
       }
       else
       {
	  sprintf( hname, "%s/%s.root", iDataDirectory.c_str(), fDataFile_proton.c_str() );
       }
    }	  
    else sprintf( hname, "%s/%s%d.root", iDataDirectory.c_str(), fDataFile_proton.c_str(), fOffsetCounter );
    string iMC_Proton   = hname;
    if( fOffsetCounter == 9999 )
    {
       if( isVTS() == 0 && fDataFile_electron.size() > 0 )
       {
          sprintf( hname, "%s/%s0.root", iDataDirectory.c_str(), fDataFile_electron.c_str() );
       }
       else if( fDataFile_electron.size() > 0 )
       {
          sprintf( hname, "%s/%s.root", iDataDirectory.c_str(), fDataFile_electron.c_str() );
       }
       else sprintf( hname, "NOFILE" );
    }
    else                         sprintf( hname, "%s/%s%d.root", iDataDirectory.c_str(), fDataFile_electron.c_str(), fOffsetCounter );
    string iMC_Electron = hname;
// gammas
    i_Sens.setMonteCarloParameters(1, fCrabSpectrumFile, fCrabSpectrumID, iMC_Gamma, 20.,
                                 i_Azbin_gamma, i_woff_gamma, i_noise_gamma, i_index_gamma );
    i_SensCU.setMonteCarloParameters(1, fCrabSpectrumFile, fCrabSpectrumID, iMC_Gamma, 20.,
                                 i_Azbin_gamma, i_woff_gamma, i_noise_gamma, i_index_gamma );
// protons
    i_Sens.setMonteCarloParameters(14, fCosmicRaySpectrumFile, fProtonSpectrumID , iMC_Proton, 20.,
			      i_Azbin_proton, i_woff_proton, i_noise_proton, i_index_proton );
    i_SensCU.setMonteCarloParameters(14, fCosmicRaySpectrumFile, fProtonSpectrumID , iMC_Proton, 20.,
			      i_Azbin_proton, i_woff_proton, i_noise_proton, i_index_proton );
// electrons (spectral index)
    if( iMC_Electron.size() > 0 && iMC_Electron != "NOFILE" )
    {
       i_Sens.setMonteCarloParameters( 2, fCosmicRaySpectrumFile, fElectronSpectrumID, iMC_Electron, 20.,
                              i_Azbin_electron, i_woff_electron, i_noise_electron, i_index_electron );
       i_SensCU.setMonteCarloParameters( 2, fCosmicRaySpectrumFile, fElectronSpectrumID, iMC_Electron, 20.,
                              i_Azbin_electron, i_woff_electron, i_noise_electron, i_index_electron );
    }
    i_Sens.calculateSensitivityvsEnergyFromCrabSpectrum( "MC", "ENERGY", 0.2, 0.01, 1.e6 );
    i_Sens.fillSensitivityHistograms( fSensitivity, fBGRate, fBGRateSqDeg, fProtRate, fElecRate );

    i_SensCU.calculateSensitivityvsEnergyFromCrabSpectrum( "MC", "CU", 0.2, 0.01, 1.e6 );
    i_SensCU.fillSensitivityHistograms( fSensitivityCU, fBGRate, fBGRateSqDeg, fProtRate, fElecRate );

    return true;
}

bool VWPPhysSensitivityFile::initializeOutputFile( string iOutputFile )
{
   char hname[200];
   sprintf( hname, "%s.%s.%.1fh.root", iOutputFile.c_str(), fSubArray.c_str(), fObservingTime_h );
   string iFileName = hname;
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
      for( unsigned int i = 0; i < hisListToDisk.size(); i++ )
      {
	 if( hisListToDisk[i] )
	 {
	    cout << "\t" << hisListToDisk[i]->GetName() << endl;
	    hisListToDisk[i]->Write();
	 }
      }
      fOutFile->Close();
   }
   return true;
}

void VWPPhysSensitivityFile::setSubArray( string iA )
{
    fSubArray = iA;

    if( isVTS() == 0 )
    {
// change here for ID change
       fDataFile_gamma_onSource = "gamma_onSource." + fSubArray + "_ID0.eff-";
       fDataFile_gamma_cone10 = "gamma_cone10." + fSubArray + "_ID0.eff-";
       fDataFile_proton = "proton." + fSubArray + "_ID0.eff-";
       fDataFile_electron = "electron." + fSubArray + "_ID0.eff-";
       if( fSubArray != "V5" && fSubArray != "V6" ) fDataFile_electron = "electron." + fSubArray + "_ID0.eff-";
       else                                         fDataFile_electron = "";
    }
    else if( isVTS() == 5 )
    {
       fDataFile_gamma_onSource = "gamma_20deg_050deg_NOISE130_ID30_SW07.eff";
       fDataFile_gamma_cone10 = "";
       fDataFile_proton = "proton_20deg_050deg_NOISE130_ID30_SW07.eff";
       fDataFile_electron = "electron_20deg_050deg_NOISE130_ID30_SW07.eff";
    }
    else if( isVTS() == 6 )
    {
       fDataFile_gamma_onSource = "gamma_20deg_050deg_NOISE200_ID30_SW05.eff";
       fDataFile_gamma_cone10 = "";
       fDataFile_proton = "proton_20deg_050deg_NOISE200_ID30_SW05.eff";
       fDataFile_electron = "electron_20deg_050deg_NOISE200_ID30_SW05.eff";
    } 
}

unsigned int VWPPhysSensitivityFile::isVTS() 
{
    if( fObservatory == "V4" ) return 4;
    if( fObservatory == "V5" ) return 5;
    if( fObservatory == "V6" ) return 6;

    return 0;
}
