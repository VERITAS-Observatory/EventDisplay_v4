/* \file VWPPhysSensitivityFile

    class to calculate sensitivities from effective area file,
    fill histogram and return them in order to write them into
    a simple root file

    NOTE: quit a few hardwired values (file names, offset angles)

*/

#include "VWPPhysSensitivityFile.h"

VWPPhysSensitivityFile::VWPPhysSensitivityFile()
{
	fDebug = false;

	fSubArray = "";
	fObservingTime_h = 0.;
	fObservatory = "CTA";

	fDataFile_gamma_onSource = "";
	fDataFile_gamma_cone = "";
	fDataFile_proton = "";
	fDataFile_proton_onSource = "";
	fDataFile_electron = "";
	fDataFile_electron_onSource = "";

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
	fProtRateSqDeg = 0;
	fElecRate = 0;
	fElecRateSqDeg = 0;
	fEffArea = 0;
	fEffArea80 = 0;
	fAngRes68 = 0;
	fAngRes80 = 0;
	fEres = 0;
	fEbias = 0;

	fOffsetCounter = 9999;
}

/*
 * initialize histograms
 *
 * iOffsetCounter == 9999: on source histograms
 */
bool VWPPhysSensitivityFile::initializeHistograms( int iEnergyXaxisNbins, double iEnergyXaxis_min, double iEnergyXaxis_max,
		int iEnergyTrue2DXaxisNbins, double iEnergyTrue2DXaxis_min, double iEnergyTrue2DXaxis_max,
		int iEnergyRec2DXaxisNbins, double iEnergyRec2DXaxis_min, double iEnergyRec2DXaxis_max,
		unsigned int iOffsetCounter )
{

	char hname[200];
	char htitle[200];
	fOffsetCounter = iOffsetCounter;

	// integrated sensitivity
	sprintf( hname, "IntSens" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fIntSensitivity = new TH1F( hname, "Int. Sens.", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fIntSensitivity->SetXTitle( "log_{10} (E_{th}/TeV)" );
	fIntSensitivity->SetYTitle( "E_{th} times IntegratedFluxSensitivity(E>E_{th}) [erg cm^{-2} s^{-1}]" );
	fIntSensitivity->Print();
	hisList.push_back( fIntSensitivity );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fIntSensitivity );
	}

	sprintf( hname, "IntSensCU" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fIntSensitivityCU = new TH1F( hname, "Int. Sens. (CU)", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fIntSensitivityCU->SetXTitle( "log_{10} (E_{th}/TeV)" );
	fIntSensitivityCU->SetYTitle( "Integral Flux Sensitivity (E>E_{th}) [C.U.]" );
	fIntSensitivityCU->Print();
	hisList.push_back( fIntSensitivityCU );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fIntSensitivityCU );
	}

	// sensitivity and background rates
	sprintf( hname, "DiffSens" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fSensitivity = new TH1F( hname, "Diff. Sens.", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fSensitivity->SetXTitle( "log_{10} (E/TeV)" );
	fSensitivity->SetYTitle( "E^{2} dF/dE [erg cm^{-2} s^{-1}]" );
	fSensitivity->Print();
	hisList.push_back( fSensitivity );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fSensitivity );
	}

	sprintf( hname, "DiffSensCU" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fSensitivityCU = new TH1F( hname, "Diff. Sens. (CU)", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fSensitivityCU->SetXTitle( "log_{10} (E/TeV)" );
	fSensitivityCU->SetYTitle( "Differential Flux Sensitivity [C.U.]" );
	fSensitivityCU->Print();
	hisList.push_back( fSensitivityCU );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fSensitivityCU );
	}

	// sensitivity limits (for on source only)
	if( fOffsetCounter == 9999 )
	{
		sprintf( hname, "DiffSens_Significance" );
		fSensitivityLimits.push_back( new TH1F( hname, "Diff. Sens. (significance limit)", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max ) );
		sprintf( hname, "DiffSens_EventNumber" );
		fSensitivityLimits.push_back( new TH1F( hname, "Diff. Sens. (event number limit)", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max ) );
		sprintf( hname, "DiffSens_SystematicsCuts" );
		fSensitivityLimits.push_back( new TH1F( hname, "Diff. Sens. (systematics limit)", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max ) );
		sprintf( hname, "DiffSens_OffEvents" );
		fSensitivityLimits.push_back( new TH1F( hname, "Diff. Sens. (off number limit)", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max ) );

		for( unsigned int i = 0; i < fSensitivityLimits.size(); i++ )
		{
			fSensitivityLimits[i]->SetXTitle( "log_{10} (E/TeV)" );
			fSensitivityLimits[i]->SetYTitle( "E^{2} dF/dE [erg cm^{-2} s^{-1}]" );
			fSensitivityLimits[i]->SetLineColor( i + 2 );
			fSensitivityLimits[i]->SetMarkerColor( i + 2 );
			fSensitivityLimits[i]->Print();
			hisList.push_back( fSensitivityLimits[i] );
			hisListToDiskDebug.push_back( fSensitivityLimits[i] );
		}

		for( unsigned int i = 0; i < fSensitivityLimits.size(); i++ )
		{
			sprintf( hname, "%s_CU", fSensitivityLimits[i]->GetName() );
			sprintf( htitle, "%s (CU)", fSensitivityLimits[i]->GetTitle() );
			fSensitivityCULimits.push_back( new TH1F( hname, htitle, iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max ) );
			fSensitivityCULimits[i]->SetXTitle( "log_{10} (E/TeV)" );
			fSensitivityCULimits[i]->SetYTitle( "Differential Flux Sensitivity [C.U.]" );
			fSensitivityCULimits[i]->SetLineColor( i + 2 );
			fSensitivityCULimits[i]->SetMarkerColor( i + 2 );
			fSensitivityCULimits[i]->Print();
			hisList.push_back( fSensitivityCULimits[i] );
			hisListToDiskDebug.push_back( fSensitivityCULimits[i] );
		}
	}

	sprintf( hname, "BGRate" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fBGRate = new TH1F( hname, "Background Rate", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fBGRate->SetXTitle( "log_{10} (E/TeV)" );
	fBGRate->SetYTitle( "background rate [1/s]" );
	hisList.push_back( fBGRate );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fBGRate );
	}

	sprintf( hname, "ProtRate" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fProtRate = new TH1F( hname, "Proton Rate", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fProtRate->SetXTitle( "log_{10} (E/TeV)" );
	fProtRate->SetYTitle( "background proton rate [1/s]" );
	hisList.push_back( fProtRate );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fProtRate );
	}

	sprintf( hname, "ElecRate" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fElecRate = new TH1F( hname, "Electron Rate", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fElecRate->SetXTitle( "log_{10} (E/TeV)" );
	fElecRate->SetYTitle( "background electron rate [1/s]" );
	hisList.push_back( fElecRate );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fElecRate );
	}

	sprintf( hname, "BGRatePerSqDeg" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fBGRateSqDeg = new TH1F( hname, "Background rate per square degree", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fBGRateSqDeg->SetXTitle( "log_{10} (E/TeV)" );
	fBGRateSqDeg->SetYTitle( "background rate [1/s/deg^{2}]" );
	hisList.push_back( fBGRateSqDeg );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fBGRateSqDeg );
	}

	sprintf( hname, "ProtRateSqDeg" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fProtRateSqDeg = new TH1F( hname, "Proton Rate per square degree", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fProtRateSqDeg->SetXTitle( "log_{10} (E/TeV)" );
	fProtRateSqDeg->SetYTitle( "background proton rate [1/s/deg^{2}]" );
	hisList.push_back( fProtRateSqDeg );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fProtRateSqDeg );
	}

	sprintf( hname, "ElecRateSqDeg" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fElecRateSqDeg = new TH1F( hname, "electron Rate per square degree", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fElecRateSqDeg->SetXTitle( "log_{10} (E/TeV)" );
	fElecRateSqDeg->SetYTitle( "background electron rate [1/s/deg^{2}]" );
	hisList.push_back( fElecRateSqDeg );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fElecRateSqDeg );
	}

	sprintf( hname, "EffectiveArea" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fEffArea = new TH1F( hname, "Effective Area", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fEffArea->SetXTitle( "log_{10} (E/TeV)" );
	fEffArea->SetYTitle( "effective area [m^{2}]" );
	hisList.push_back( fEffArea );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fEffArea );
	}

	sprintf( hname, "EffectiveAreaEtrue" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fEffAreaMC = new TH1F( hname, "Effective Area in true energy", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fEffAreaMC->SetXTitle( "log_{10} (E_{MC}/TeV)" );
	fEffAreaMC->SetYTitle( "effective area [m^{2}]" );
	fEffAreaMC->SetLineColor( 2 );
	hisList.push_back( fEffAreaMC );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fEffAreaMC );
	}

	sprintf( hname, "EffectiveArea80" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fEffArea80 = new TH1F( hname, "Effective Area for 80% point-source containment", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fEffArea80->SetXTitle( "log_{10} (E/TeV)" );
	fEffArea80->SetYTitle( "effective area [m^{2}]" );
	hisList.push_back( fEffArea80 );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fEffArea80 );
	}

	// angular resolution histograms
	sprintf( hname, "AngRes" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fAngRes68 = new TH1F( hname, "Angular resolution (68% containment)", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fAngRes68->SetXTitle( "log_{10} (E/TeV)" );
	fAngRes68->SetYTitle( "containment radius (68%) [deg]" );
	hisList.push_back( fAngRes68 );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fAngRes68 );
	}

	sprintf( hname, "AngRes80" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fAngRes80 = new TH1F( hname, "Angular resolution (80% containment)", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fAngRes80->SetXTitle( "log_{10} (E/TeV)" );
	fAngRes80->SetYTitle( "containment radius (80%) [deg]" );
	hisList.push_back( fAngRes80 );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fAngRes80 );
	}

	sprintf( hname, "ERes" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fEres = new TH1F( hname, "Energy resolution", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fEres->SetXTitle( "log_{10} (E/TeV)" );
	//   fEres->SetYTitle( "Relative Energy resolution (68% containment around Eres/Etrue=1)" );
	fEres->SetYTitle( "energy resolution (RMS)" );
	hisList.push_back( fEres );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fEres );
	}

	sprintf( hname, "Ebias" );
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s_%d", hname, fOffsetCounter );
	}
	fEbias = new TH1F( hname, "Energy bias (mean E_{rec}/E_{MC})", iEnergyXaxisNbins, iEnergyXaxis_min, iEnergyXaxis_max );
	fEbias->SetXTitle( "log_{10} (E/TeV)" );
	fEbias->SetYTitle( "energy reconstruction bias (<E_{rec}/E_{MC}>)" );
	hisList.push_back( fEbias );
	if( fOffsetCounter == 9999 )
	{
		hisListToDisk.push_back( fEbias );
	}

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

/*
   fill historgram vs distance to camera centre (off-axis sensitivities)

*/
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
	int nbins_woff = ( int ) iWobble_min.size();
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
	woff[nbins_woff] = iWobble_max[iWobble_max.size() - 1];

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
					// first element in loop: create new histograms
					if( j == 0 )
					{
						if( iClass == "TH1F" )
						{
							iHis2D[iHisName2D] = new TH2F( iHisName2D.c_str(), hisList[i]->GetTitle(),
														   hisList[i]->GetNbinsX(), hisList[i]->GetXaxis()->GetXmin(), hisList[i]->GetXaxis()->GetXmax(),
														   nbins_woff, woff );
							iHis2D[iHisName2D]->SetXTitle( hisList[i]->GetXaxis()->GetTitle() );
							iHis2D[iHisName2D]->SetYTitle( "off-axis angle [deg]" );
							iHis2D[iHisName2D]->SetZTitle( hisList[i]->GetYaxis()->GetTitle() );
							iHis2D[iHisName2D]->SetStats( 0 );
							hisListToDisk.push_back( iHis2D[iHisName2D] );
						}
						else if( iClass == "TH2F" || iClass == "TH2D" )
						{
							// get axes definitions
							Double_t xaxis[hisList[i]->GetNbinsX() + 1];
							for( int z = 0; z < hisList[i]->GetNbinsX(); z++ )
							{
								xaxis[z] = hisList[i]->GetXaxis()->GetBinLowEdge( z );
							}
							xaxis[hisList[i]->GetNbinsX()] = hisList[i]->GetXaxis()->GetBinUpEdge( hisList[i]->GetNbinsX() );
							Double_t yaxis[hisList[i]->GetNbinsY() + 1];
							for( int z = 0; z < hisList[i]->GetNbinsY(); z++ )
							{
								yaxis[z] = hisList[i]->GetYaxis()->GetBinLowEdge( z );
							}
							yaxis[hisList[i]->GetNbinsY()] = hisList[i]->GetYaxis()->GetBinUpEdge( hisList[i]->GetNbinsY() );

							iHis3D[iHisName2D] = new TH3F( iHisName2D.c_str(), hisList[i]->GetTitle(),
														   hisList[i]->GetNbinsX(), xaxis,
														   hisList[i]->GetNbinsY(), yaxis,
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
							iHis2D[iHisName2D]->SetBinContent( b, j + 1, hisList[i]->GetBinContent( b ) );
							iHis2D[iHisName2D]->SetBinError( b, j + 1, hisList[i]->GetBinError( b ) );
						}
					}
					if( ( iClass == "TH2F" || iClass == "TH2D" ) && iHis3D.find( iHisName2D ) != iHis3D.end() )
					{
						for( int bx = 0; bx <= hisList[i]->GetNbinsX(); bx++ )
						{
							for( int by = 0; by <= hisList[i]->GetNbinsY(); by++ )
							{
								iHis3D[iHisName2D]->SetBinContent( bx, by, j + 1, hisList[i]->GetBinContent( bx, by ) );
								iHis3D[iHisName2D]->SetBinError( bx, by, j + 1, hisList[i]->GetBinError( bx, by ) );
							}
						}
					}
				}
			}
		}
	}

	return true;
}

/*
 * fill IRFs and sensitivity histograms
 *
 */
bool VWPPhysSensitivityFile::fillHistograms1D( string iDataDirectory, bool iFill1D )
{
	// expect certain directory and file naming
	ostringstream iEffectiveAreaFile;
	if( fOffsetCounter < 9999 )
	{
		std::cout << " fDataFile_gamma_cone " << fDataFile_gamma_cone << std::endl;
		iEffectiveAreaFile << iDataDirectory << "/" << fDataFile_gamma_cone << fOffsetCounter << ".root";
	}
	else
	{
		if( isVTS() == 0 )
		{
			iEffectiveAreaFile << iDataDirectory << "/" << fDataFile_gamma_onSource  << "0.root";
		}
		else
		{
			std::cout << " fDataFile_gamma_onSource " << fDataFile_gamma_onSource << std::endl;
			iEffectiveAreaFile << iDataDirectory << "/" << fDataFile_gamma_onSource  << ".root";
		}
	}
	cout << endl;
	cout << "=================================================================" << endl;
	cout << "reading " << iEffectiveAreaFile.str() << endl;
	cout << endl;

	return ( fillIRFHistograms( iEffectiveAreaFile.str() ) && fillSensitivityHistograms( iDataDirectory, iFill1D ) );
}

/*

   read IRF histograms from effective area file and translate them to
   CTA WP Phys format
*/
bool VWPPhysSensitivityFile::fillIRFHistograms( string iEffectiveAreaFile, double iZe, double iWoff )
{

	std::cout << "VWPPhysSensitivityFile::fillHistograms1D " << std::endl;

	////////////////////////////////////////////////////////////////////////
	// instrument response function reader
	VInstrumentResponseFunctionReader i_IRF;
	///////////////////////////////////////////////////////////////////////
	// CTA data
	if( isVTS() == 0 )
	{
		if( !i_IRF.fillData( iEffectiveAreaFile.c_str(), iZe, iWoff ) )
		{
			cout << "VWPPhysSensitivityFile::fillHistograms1D error filling data from " << iEffectiveAreaFile.c_str() << endl;
			return false;
		}
	}
	///////////////////////////////////////////////////////////////////////
	// VERITAS data
	else
	{
		if( isVTS() == 5 && !i_IRF.fillData( iEffectiveAreaFile.c_str(), iZe, iWoff, 16, 2.0, 130 ) )
		{
			return false;
		}
		if( isVTS() == 6 && !i_IRF.fillData( iEffectiveAreaFile.c_str(), iZe, iWoff, 16, 2.0, 150 ) )
		{
			return false;
		}
	}
	// fill angular resolution histograms
	i_IRF.fillResolutionHistogram( fAngRes68, "68", "t_angular_resolution" );
	i_IRF.fillResolutionHistogram( fAngRes80, "80", "t_angular_resolution" );
	// fill energy resolution histograms
	i_IRF.fillResolutionHistogram( fEres, "68", "t_energy_resolution" );
	// fill energy bias histograms
	i_IRF.fillBiasHistograms( fEbias, "mean" );
	// fill effective area histograms (68%)
	i_IRF.fillEffectiveAreasHistograms( fEffArea, "", fEffAreaMC );
	// fill effective area histograms (80%)
	i_IRF.fillEffectiveAreasHistograms( fEffArea80, "80", 0 );
	char hname[2000];
	if( i_IRF.getMigrationMatrix() )
	{
		if( fOffsetCounter < 9999 )
		{
			sprintf( hname, "MigMatrix_%d", fOffsetCounter );
		}
		else
		{
			sprintf( hname, "MigMatrix" );
		}
		TH2F* hMigMatrix = ( TH2F* )i_IRF.getMigrationMatrix()->Clone( hname );
		hMigMatrix->SetTitle( "Migration Matrix" );
		hMigMatrix->SetStats( 0 );
		hMigMatrix->SetXTitle( "log_{10} (E_{rec}/TeV)" );
		hMigMatrix->SetYTitle( "log_{10} (E_{MC}/TeV)" );
		hisList.push_back( hMigMatrix );
		if( fOffsetCounter == 9999 )
		{
			hisListToDisk.push_back( hMigMatrix );
		}
	}
	if( i_IRF.getRecvsMCEnergy() )
	{
		if( fOffsetCounter < 9999 )
		{
			sprintf( hname, "EestOverEtrue_%d", fOffsetCounter );
		}
		else
		{
			sprintf( hname, "EestOverEtrue" );
		}
		TH2F* hhEsysMCRelative = ( TH2F* )i_IRF.getRecvsMCEnergy()->Clone( hname );
		hhEsysMCRelative->SetTitle( "E_{rec}/E_{MC} vs. E_{MC}" );
		hhEsysMCRelative->SetStats( 0 );
		hhEsysMCRelative->SetXTitle( "log_{10} (E_{MC}/TeV)" );
		hhEsysMCRelative->SetYTitle( "E_{rec}/E_{MC}" );
		hisList.push_back( hhEsysMCRelative );
		if( fOffsetCounter == 9999 )
		{
			hisListToDisk.push_back( hhEsysMCRelative );
		}
	}
	return true;
}

/*
   sensitivity histograms
*/
bool VWPPhysSensitivityFile::fillSensitivityHistograms( string iDataDirectory, bool iFill1D )
{
	char hname[2000];

	VSensitivityCalculator i_Sens;
	VSensitivityCalculator i_SensCU;
	// set Crab Nebula spectrum
	i_Sens.setEnergySpectrumfromLiterature( fCrabSpectrumFile, fCrabSpectrumID );
	i_SensCU.setEnergySpectrumfromLiterature( fCrabSpectrumFile, fCrabSpectrumID );
	// energy range (default CTA values)
	i_Sens.setEnergyRange_Lin( 0.01, 150. );
	i_SensCU.setEnergyRange_Lin( 0.01, 150. );
	// significance parameters (default CTA values)
	i_Sens.setSignificanceParameter( 5., 10., fObservingTime_h, 0.05, 0.2 );
	i_SensCU.setSignificanceParameter( 5., 10., fObservingTime_h, 0.05, 0.2 );
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

	//////////////////////////////////////////////////////////////
	// VTS only
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
	// (END VTS only)
	//////////////////////////////////////////////////////////////

	cout << "SETTING EFFECTIVE AREA SEARCH VALUES TO " << fSubArray << endl;
	//////////////////////////////////////////////////////////////////////////
	// effective area files
	////////////////////////
	// gamma offset
	if( fOffsetCounter < 9999 )
	{
		sprintf( hname, "%s/%s%d.root", iDataDirectory.c_str(), fDataFile_gamma_cone.c_str(), fOffsetCounter );
	}
	// gamma on source
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
	// proton on source
	if( fOffsetCounter == 9999 )
	{
		if( isVTS() == 0 )
		{
			sprintf( hname, "%s/%s0.root", iDataDirectory.c_str(), fDataFile_proton_onSource.c_str() );
		}
		else
		{
			sprintf( hname, "%s/%s.root", iDataDirectory.c_str(), fDataFile_proton.c_str() );
		}
	}
	// proton offset
	else
	{
		sprintf( hname, "%s/%s%d.root", iDataDirectory.c_str(), fDataFile_proton.c_str(), fOffsetCounter );
	}
	string iMC_Proton   = hname;
	// electron on source
	if( fOffsetCounter == 9999 )
	{
		if( isVTS() == 0 && fDataFile_electron_onSource.size() > 0 )
		{
			sprintf( hname, "%s/%s0.root", iDataDirectory.c_str(), fDataFile_electron_onSource.c_str() );
		}
		else if( fDataFile_electron.size() > 0 )
		{
			sprintf( hname, "%s/%s.root", iDataDirectory.c_str(), fDataFile_electron.c_str() );
		}
		else
		{
			sprintf( hname, "NOFILE" );
		}
	}
	// electron offset
	else
	{
		if( fDataFile_electron_onSource.size() > 0 )
		{
			sprintf( hname, "%s/%s%d.root", iDataDirectory.c_str(), fDataFile_electron.c_str(), fOffsetCounter );
		}
		else
		{
			sprintf( hname, "NOFILE" );
		}

	}
	string iMC_Electron = hname;

	// initialize sensitivity calculator

	// gammas
	std::cout << " iMC_Gamma " << iMC_Gamma << std::endl;
	i_Sens.setMonteCarloParameters( 1, fCrabSpectrumFile, fCrabSpectrumID, iMC_Gamma, 20.,
									i_Azbin_gamma, i_woff_gamma, i_noise_gamma, i_index_gamma, -10., 10., "ENERGY" );
	i_SensCU.setMonteCarloParameters( 1, fCrabSpectrumFile, fCrabSpectrumID, iMC_Gamma, 20.,
									  i_Azbin_gamma, i_woff_gamma, i_noise_gamma, i_index_gamma, -10., 10., "CU" );
	// protons
	i_Sens.setMonteCarloParameters( 14, fCosmicRaySpectrumFile, fProtonSpectrumID, iMC_Proton, 20.,
									i_Azbin_proton, i_woff_proton, i_noise_proton, i_index_proton, -10., 10., "ENERGY" );
	i_SensCU.setMonteCarloParameters( 14, fCosmicRaySpectrumFile, fProtonSpectrumID, iMC_Proton, 20.,
									  i_Azbin_proton, i_woff_proton, i_noise_proton, i_index_proton, -10., 10., "CU" );
	// electrons (spectral index)
	if( iMC_Electron.size() > 0 && iMC_Electron != "NOFILE" )
	{
		i_Sens.setMonteCarloParameters( 2, fCosmicRaySpectrumFile, fElectronSpectrumID, iMC_Electron, 20.,
										i_Azbin_electron, i_woff_electron, i_noise_electron, i_index_electron, -10., 10., "ENERGY" );
		i_SensCU.setMonteCarloParameters( 2, fCosmicRaySpectrumFile, fElectronSpectrumID, iMC_Electron, 20.,
										  i_Azbin_electron, i_woff_electron, i_noise_electron, i_index_electron, -10., 10., "CU" );
	}
	// fill sensitivity histograms
	bool iHighEnergyFilling = false;
	i_Sens.calculateSensitivityvsEnergyFromCrabSpectrum( "MC", "ENERGY", 0.2, 0.01, 1.e6 );
	i_Sens.fillSensitivityHistograms( fSensitivity, fBGRate, fBGRateSqDeg, fProtRate, fProtRateSqDeg, fElecRate, fElecRateSqDeg, iHighEnergyFilling );
	if( iFill1D )
	{
		i_Sens.fillSensitivityLimitsHistograms( fSensitivityLimits );
	}
	// re-use i_Sens to calculate the integrated sensitivity 0.2 -> -1
	i_Sens.calculateSensitivityvsEnergyFromCrabSpectrum( "MC", "ENERGY", -1, 0.01, 1.e6 );
	i_Sens.fillSensitivityHistograms( fIntSensitivity );

	i_SensCU.calculateSensitivityvsEnergyFromCrabSpectrum( "MC", "CU", 0.2, 0.01, 1.e6 );
	i_SensCU.fillSensitivityHistograms( fSensitivityCU, fBGRate, fBGRateSqDeg, fProtRate, fProtRateSqDeg, fElecRate, fElecRateSqDeg, iHighEnergyFilling );
	if( iFill1D )
	{
		i_SensCU.fillSensitivityLimitsHistograms( fSensitivityCULimits );
	}
	// re-use i_SensCU to calculate the integrated sensitivity 0.2 -> -1
	i_SensCU.calculateSensitivityvsEnergyFromCrabSpectrum( "MC", "CU", -1, 0.01, 1.e6 );
	i_SensCU.fillSensitivityHistograms( fIntSensitivityCU );

	return true;
}

bool VWPPhysSensitivityFile::initializeOutputFile( string iOutputFile )
{
	char hname[200];
	string iFileName = iOutputFile;
	if( fSubArray.size() > 0 && fObservingTime_h > 0. )
	{
		sprintf( hname, "%s.%s.%ds.root", iOutputFile.c_str(), fSubArray.c_str(), int( fObservingTime_h * 3600. ) );
		iFileName = hname;
	}
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
			if( hisListToDisk[i] && hisListToDisk[i]->GetEntries() > 0 )
			{
				cout << hisListToDisk[i]->GetName() << ", ";
				hisListToDisk[i]->Write();
			}
		}
		cout << endl;
		// debug histograms
		TDirectory* iD = fOutFile->mkdir( "debug", "additional debug histograms" );
		if( iD && iD->cd() )
		{
			for( unsigned int i = 0; i < hisListToDiskDebug.size(); i++ )
			{
				if( hisListToDiskDebug[i] && hisListToDiskDebug[i]->GetEntries() > 0 )
				{
					cout << hisListToDiskDebug[i]->GetName() << ", ";
					hisListToDiskDebug[i]->Write();
				}
			}
			cout << endl;
		}

		fOutFile->Close();
	}
	return true;
}

void VWPPhysSensitivityFile::setDataFiles( string iArray, int iRecID )
{

	std::cout << "VWPPhysSensitivityFile::setDataFiles " << fObservatory << std::endl;

	fSubArray = iArray;

	// set data files for CTA
	if( isVTS() == 0 )
	{
		// change here for ID change
		char hname[200];
		sprintf( hname, "%d", iRecID );
		fDataFile_gamma_onSource = "gamma_onSource." + fSubArray + "_ID" + hname + ".eff-";
		fDataFile_gamma_cone = "gamma_cone." + fSubArray + "_ID" + hname + ".eff-";
		fDataFile_proton = "proton." + fSubArray + "_ID" + hname + ".eff-";
		fDataFile_proton_onSource = "proton_onSource." + fSubArray + "_ID" + hname + ".eff-";
		fDataFile_electron = "electron." + fSubArray + "_ID" + hname + ".eff-";
		fDataFile_electron_onSource = "electron_onSource." + fSubArray + "_ID" + hname + ".eff-";
		if( fSubArray != "V5" && fSubArray != "V6" )
		{
			fDataFile_electron = "electron." + fSubArray + "_ID" + hname + ".eff-";
		}
		else
		{
			fDataFile_electron = "";
		}
	}
	// set data files for VERITAS (epoch V5)
	else if( isVTS() == 5 )
	{
		fDataFile_gamma_onSource = "gamma_20deg_050deg_NOISE130_ID30_SW07.eff";
		fDataFile_gamma_cone = "";
		fDataFile_proton = "proton_20deg_050deg_NOISE130_ID30_SW07.eff";
		fDataFile_electron = "electron_20deg_050deg_NOISE130_ID30_SW07.eff";
	}
	// set data files for VERITAS (epoch V6)
	else if( isVTS() == 6 )
	{
		fDataFile_gamma_onSource = "gamma_20deg_050deg_NOISE200_ID30_SW05.eff";
		fDataFile_gamma_cone = "";
		fDataFile_proton = "proton_20deg_050deg_NOISE200_ID30_SW05.eff";
		fDataFile_electron = "electron_20deg_050deg_NOISE200_ID30_SW05.eff";
	}
	else if( isVTS() == 9 ) // CTA Norm_Pointing (on cone 0-20)
	{
		char hname[200];
		sprintf( hname, "%d", iRecID );
		fDataFile_gamma_onSource = ""; // should not be used in this case
		fDataFile_gamma_cone = "gamma_0-20_Norm_Pointing." + fSubArray + "_ID" + hname + ".eff-";
		fDataFile_proton = "proton_0-20_rER_Norm_Pointing." + fSubArray + "_ID" + hname + ".eff-";
		fDataFile_electron = "";


	}

	std::cout << "fDataFile_gamma_cone " << fDataFile_gamma_cone << std::endl;
	std::cout << "fDataFile_proton     " << fDataFile_proton << std::endl;
}

/*

   check if current data set is a VERITAS data set

   return epoch number

*/
unsigned int VWPPhysSensitivityFile::isVTS()
{
	if( fObservatory == "V4" )
	{
		return 4;
	}
	if( fObservatory == "V5" )
	{
		return 5;
	}
	if( fObservatory == "V6" )
	{
		return 6;
	}
	if( fObservatory == "CTA_Norm_Pointing_7bin" )
	{
		return 9;
	}

	return 0;
}
