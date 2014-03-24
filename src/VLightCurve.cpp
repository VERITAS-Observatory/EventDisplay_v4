/*! \class VLightCurve
    \brief light curves


*/

#include "VLightCurve.h"

VLightCurve::VLightCurve()
{
	fDayInterval = 1.;
	fFluxCombined = 0;
	
	fDataType = "";                       // possible data types are TeV_anasum, TeV_ascii, XRT_ascii
	
	fEnergy_min_TeV = 1.;
	fEnergy_max_TeV = -99.;
	
	fLightCurveGraph = 0;
	fCanvasLightCurve = 0;
	fCanvasPhaseDistribution = 0;
	fMCRandomizedPhaseogram = 0;
	fMCRandomizedPhaseogramProf = 0;
	
	fObservingInvervallHisto = 0;
	
	setSignificanceParameters();
	setSpectralParameters();
	
	setPlottingParameter( -99., -99. );
	fRateAxisTitleUnSet = false;
	setLightCurveAxis();
	setPhaseFoldingValues( -99., -99., false );
}

/*

    read XRT light curve from a ASCII file

*/
bool VLightCurve::initializeXRTLightCurve( string iXRTFile, double iMJDMin, double iMJDMax )
{
	fDataType = "XRT_ascii";
	
	// read in ascii file
	readASCIIFile( iXRTFile, iMJDMin, iMJDMax );
	if( isZombie() )
	{
		return false;
	}
	
	// plotting range
	if( fPlottingMJDMin < 0. )
	{
		fPlottingMJDMin = getLightCurveMJD_min() - 5.;
	}
	if( fPlottingMJDMax < 0. )
	{
		fPlottingMJDMax = getLightCurveMJD_max() + 5.;
	}
	
	return true;
}

/*

    read TeV fluxes from ASCII file

*/
bool VLightCurve::initializeTeVLightCurve( string iASCIIFile, double iFluxMultiplier )
{
	// make sure that this is really a ascii file
	if( iASCIIFile.find( ".root" ) != string::npos )
	{
		return initializeTeVLightCurve( iASCIIFile, 1., -1., -1. );
	}
	fDataType = "TeV_ascii";
	
	// read in ascii file
	readASCIIFile( iASCIIFile, -1., -1., iFluxMultiplier );
	if( isZombie() )
	{
		return false;
	}
	
	// plotting range
	if( fPlottingMJDMin < 0. )
	{
		fPlottingMJDMin = getLightCurveMJD_min() - 5.;
	}
	if( fPlottingMJDMax < 0. )
	{
		fPlottingMJDMax = getLightCurveMJD_max() + 5.;
	}
	
	return true;
}

/*

    read fluxes etc from anasum file

*/
bool VLightCurve::initializeTeVLightCurve( string iAnaSumFile, double iDayInterval, double iMJDMin, double iMJDMax )
{
	fDataType = "TeV_anasum";
	
	fDayInterval = iDayInterval;
	fAnaSumFile = iAnaSumFile;
	
	// read full file to get an overview of all available runs
	fFluxCombined = new VFluxCalculation( fAnaSumFile, 1, 0, 100000, -99., -99., false );
	if( fFluxCombined->IsZombie() )
	{
		cout << "VLightCurve::initializeLightCurve error reading anasum file" << endl;
		return false;
	}
	fFluxCombined->setTimeBinnedAnalysis( false );
	
	/////////////////////////////////
	// setup time bins
	
	// get timing vectors (expect them to be sorted from earliest to latest)
	vector< double > fMJD    = fFluxCombined->getMJD();
	vector< double > fRunTOn = fFluxCombined->getTOn();
	if( fMJD.size() < 2 )
	{
		cout << "VLightCurve::initializeLightCurve error, time vector too small: " << fMJD.size() << endl;
		return false;
	}
	
	// get MJD for the case that no time limits are given
	if( iMJDMin < 0 )
	{
		iMJDMin = TMath::Floor( fMJD[0] );
	}
	if( iMJDMax < 0 )
	{
		iMJDMax = TMath::Floor( fMJD[fMJD.size() - 2] ) + 1;
	}
	
	// plotting parameters
	if( fPlottingMJDMin < 0. )
	{
		fPlottingMJDMin = iMJDMin - 5.;
	}
	if( fPlottingMJDMax < 0. )
	{
		fPlottingMJDMax = iMJDMax + 5.;
	}
	
	unsigned int iNbins = ( unsigned int )TMath::Nint( ( iMJDMax - iMJDMin ) / fDayInterval + 0.5 );
	if( iNbins == 0 && iMJDMax > iMJDMin )
	{
		iNbins = 1;
	}
	if( iMJDMax < 0 )
	{
		iMJDMax = iMJDMin + iNbins * fDayInterval;
	}
	
	cout << "Defining " << iNbins << " time " << ( iNbins == 1 ? "interval" : "intervals" ) << " from " << iMJDMin << " to " << iMJDMax << endl;
	
	// resetting light curve data vector
	fLightCurveData.clear();
	
	// fill all light curve bins (might be a run, or XX days long)
	for( unsigned int i = 0; i < iNbins; i++ )
	{
		unsigned int z = 0;
		// loop over all runs (using the MJD vector)
		for( unsigned int j = 0; j < fMJD.size() - 1; j++ )
		{
			// check if there are data in this MJD interval
			if( fMJD[j] >= iMJDMin + i * fDayInterval && fMJD[j] < iMJDMin + ( i + 1 ) * fDayInterval && fMJD[j] < iMJDMax )
			{
				// get observing epoch
				if( z == 0 )
				{
					fLightCurveData.push_back( new VLightCurveData() );
					if( iMJDMin + ( i + 1 ) * fDayInterval < iMJDMax )
					{
						fLightCurveData.back()->setMJDInterval( iMJDMin + i * fDayInterval, iMJDMin + ( i + 1 ) * fDayInterval );
					}
					else
					{
						fLightCurveData.back()->setMJDInterval( iMJDMin + i * fDayInterval, iMJDMax );
					}
				}
				if( fLightCurveData.back() )
				{
					fLightCurveData.back()->fRunList.push_back( fFluxCombined->getRunList()[j] );
				}
				z++;
			}
		}
	}
	// update phase folding values
	updatePhaseFoldingValues();
	
	return true;
}

bool VLightCurve::fill( double iEMin_TeV, double iEMax_TeV, bool iPrint )
{
	fEnergy_min_TeV = iEMin_TeV;
	fEnergy_max_TeV = iEMax_TeV;
	
	cout << "filling data for data type " << fDataType << endl;
	
	if( fDataType == "TeV_anasum" )
	{
		return fillTeV_anasum( iPrint );
	}
	else if( fDataType == "TeV_ascii" )
	{
		return fillTeV_ascii( iPrint );
	}
	else if( fDataType == "XRT_ascii" )
	{
		return fillXRT_ascii( iPrint );
	}
	
	return false;
}

bool VLightCurve::fillTeV_ascii( bool iPrint )
{
	if( iPrint )
	{
		printLightCurve();
	}
	return true;
}

bool VLightCurve::fillXRT_ascii( bool iPrint )
{
	if( iPrint )
	{
		printLightCurve();
	}
	
	return true;
}

bool VLightCurve::fillTeV_anasum( bool iPrint )
{
	// loop over all light curve data and calculate fluxes
	for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
	{
		if( fLightCurveData[i] )
		{
			fLightCurveData[i]->setFluxCalculationEnergyInterval( fEnergy_min_TeV, fEnergy_max_TeV );
			fLightCurveData[i]->fillTeVEvndispData( fAnaSumFile, fThresholdSignificance, fMinEvents,
													fUpperLimit, fUpperLimitMethod, fLiMaEqu, fMinEnergy, fE0, fAlpha );
		}
	}
	
	if( iPrint )
	{
		printLightCurve();
	}
	
	return true;
}

/*

      plot a light curve for the given data

      TCanvas* iCanvasLightCurve      	0 for a new canvas, or pointer to an existing canvas
      string iCanvasName              	canvas name (for new canvas)
      int iPlotConfidenceInterval 	plot rates as usual (0; Non-Noff with Poisson error)
      					or
					1 sigma confidence intervalls using Rolke et al (iPlotConfidenceInterval >= 0; number is plotting color)

*/
TCanvas* VLightCurve::plotLightCurve( TCanvas* iCanvasLightCurve, string iCanvasName, int iPlotConfidenceInterval, string iPlottingOption, double iMaxMJDError )
{
	char hname[800];
	char htitle[800];
	
	TH1D* hLightCurve = 0;
	
	// canvas
	if( !iCanvasLightCurve )
	{
		sprintf( hname, "%s", iCanvasName.c_str() );
		if( fName.size() > 0 )
		{
			sprintf( htitle, "%s", fName.c_str() );
		}
		else
		{
			sprintf( htitle, "light curve" );
		}
		if( fPhase_Period_days > 0. )
		{
			sprintf( hname, "%s_%d_%d_%d_%d", iCanvasName.c_str(), ( int )fPhase_MJD0, ( int )fPhase_Period_days,
					 ( int )fPhasePlotting, ( int )( fEnergy_min_TeV * 1000. ) );
			sprintf( htitle, "%s, T_{0}=%.1f, period=%.1f days", fName.c_str(), fPhase_MJD0, fPhase_Period_days );
		}
		
		fCanvasLightCurve = new TCanvas( hname, htitle, 10, 10, getPlottingCanvasX(), getPlottingCanvasY() );
		fCanvasLightCurve->SetGridx( 0 );
		fCanvasLightCurve->SetGridy( 0 );
		
		// histogram values
		if( fPlottingMJDMin < 0. && fPlottingMJDMax < 0. )
		{
			fPlottingMJDMin = getLightCurveMJD_min();
			fPlottingMJDMin = getLightCurveMJD_max();
		}
		double i_xmin = fPlottingMJDMin;
		double i_xmax = fPlottingMJDMax;
		sprintf( hname, "MJD" );
		sprintf( htitle, "hLightCurve" );
		if( fPhase_Period_days > 0. )
		{
			if( fPhasePlotting )
			{
				i_xmin = 0.;
				i_xmax = 1.;
				sprintf( hname, "phase" );
			}
			else
			{
				i_xmin = 0.;
				i_xmax = fPhase_Period_days;
				sprintf( hname, "phase folded days" );
			}
			sprintf( htitle, "%s_%d_%d_%d", htitle, ( int )fPhase_MJD0, ( int )fPhase_Period_days, ( int )fPhasePlotting );
		}
		
		hLightCurve = new TH1D( htitle, "", 100, i_xmin, i_xmax );
		hLightCurve->SetStats( 0 );
		hLightCurve->SetXTitle( hname );
		hLightCurve->GetXaxis()->CenterTitle( true );
		hLightCurve->SetYTitle( getLightCurveAxisTitle().c_str() );
		hLightCurve->SetMinimum( getLightCurveAxisRange_Min() );
		hLightCurve->SetMaximum( getLightCurveAxisRange_Max() );
		hLightCurve->Draw( "" );
		hLightCurve->Draw( "AH" );
		
		cout << "Plotting range: " << fPlottingMJDMin << " < MJD < " << fPlottingMJDMax;
		cout << ", " << hLightCurve->GetMinimum() << " < rate < " << hLightCurve->GetMaximum() << endl;
		
		plot_nullHistogram( fCanvasLightCurve, hLightCurve, false, true, 1.2, fPlottingMJDMin, fPlottingMJDMax );
		
	}
	// canvas exists - get histogram in canvas
	else
	{
		fCanvasLightCurve = iCanvasLightCurve;
		fCanvasLightCurve->cd();
		sprintf( htitle, "hLightCurve" );
		if( fPhase_Period_days > 0. )
		{
			sprintf( htitle, "%s_%d_%d_%d", htitle, ( int )fPhase_MJD0, ( int )fPhase_Period_days, ( int )fPhasePlotting );
		}
		hLightCurve = ( TH1D* )fCanvasLightCurve->GetListOfPrimitives()->FindObject( htitle );
		if( !hLightCurve )
		{
			hLightCurve = ( TH1D* )fCanvasLightCurve->GetListOfPrimitives()->FindObject( "hLightCurve" );
		}
		if( !hLightCurve )
		{
			cout << "VLightCurve::plot: no light curve histogram found with name " << htitle << endl;
			return fCanvasLightCurve;
		}
	}
	
	////////////////////////////////////////////////////////////////////////
	// flux and upper flux plotting
	
	fLightCurveGraph = new TGraphAsymmErrors( 0 );
	
	// loop over all measured values
	unsigned int z = 0;
	for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
	{
		// x-axis: MJD or phase
		double iMJD_mean =  fLightCurveData[i]->getMJD();
		double iMJD_error = fLightCurveData[i]->getMJDError();
		// phase dependent analysis
		if( fPhase_Period_days > 0. )
		{
			iMJD_mean = getPhase( iMJD_mean );
			if( fPhasePlotting )
			{
				iMJD_error /= fPhase_Period_days;
			}
		}
		
		/////////////
		// plot fluxes or confidence intervals
		if( fLightCurveData[i] && fLightCurveData[i]->getFluxError() > 0. )
		{
			if( iPlotConfidenceInterval < 0 )
			{
				fLightCurveGraph->SetPoint( z, iMJD_mean, fLightCurveData[i]->fFlux );
				if( iMaxMJDError > 0. && iMJD_error > iMaxMJDError )
				{
					fLightCurveGraph->SetPointEXlow( z, 0. );
					fLightCurveGraph->SetPointEXhigh( z, 0. );
				}
				else
				{
					fLightCurveGraph->SetPointEXlow( z, iMJD_error );
					fLightCurveGraph->SetPointEXhigh( z, iMJD_error );
				}
				fLightCurveGraph->SetPointEYlow( z, fLightCurveData[i]->fFluxErrorDown );
				fLightCurveGraph->SetPointEYhigh( z, fLightCurveData[i]->fFluxErrorUp );
			}
			else
			{
				// plot coincidence intervals
				double iFMean = 0.5 * ( fLightCurveData[i]->fRunFluxCI_lo_1sigma + fLightCurveData[i]->fRunFluxCI_up_1sigma );
				fLightCurveGraph->SetPoint( z, iMJD_mean, iFMean );
				// (minimum width -> make sure that line is visible...)
				if( iMJD_error < 0.3 && !( fPhase_Period_days > 0. ) )
				{
					iMJD_error = 0.3;
				}
				fLightCurveGraph->SetPointError( z, iMJD_error, iMJD_error,
												 iFMean - fLightCurveData[i]->fRunFluxCI_lo_1sigma, fLightCurveData[i]->fRunFluxCI_up_1sigma - iFMean );
			}
			z++;
		}
		/////////////
		// plot upper flux limits
		else if( fLightCurveData[i]->fUpperFluxLimit > 0. )
		{
			double UL = fLightCurveData[i]->fUpperFluxLimit;
			cout << UL << endl;
			TArrow* fUL = new TArrow( iMJD_mean, UL, iMJD_mean, UL - 0.05 * hLightCurve->GetMaximum(), 0.01, "|-|>" );
			setArrowPlottingStyle( fUL );
			fUL->Draw();
		}
		/////////////
		// plot observational periods
		else
		{
			cout << "no plotting for flux intervall " << i << "\t" << fLightCurveData[i]->fMJD_min << "\t" << fLightCurveData[i]->fMJD_max << ":" << endl;
			cout << "\t" << fLightCurveData[i]->fFlux << " +- " << fLightCurveData[i]->getFluxError() << " , UL" << fLightCurveData[i]->fUpperFluxLimit << endl;
		}
	}
	if( fLightCurveGraph->GetN() > 0 )
	{
		setGraphPlottingStyle( ( TGraph* )fLightCurveGraph );
		if( iPlotConfidenceInterval < 0 )
		{
			fLightCurveGraph->Draw( iPlottingOption.c_str() );
		}
		else
		{
			fLightCurveGraph->SetFillColor( iPlotConfidenceInterval );
			fLightCurveGraph->Draw( "2" );
		}
	}
	
	return fCanvasLightCurve;
}

void VLightCurve::setPlottingParameter( double iPlottingMJDMin, double iPlottingMJDMax )
{
	fPlottingMJDMin = iPlottingMJDMin;
	fPlottingMJDMax = iPlottingMJDMax;
}


void VLightCurve::setSignificanceParameters( double iSig, double iMinEvents, double iUL, int iULAlgo, int iLiAndMa )
{
	fThresholdSignificance = iSig;
	fMinEvents = iMinEvents;
	fUpperLimit = iUL;
	fLiMaEqu = iLiAndMa;
	fUpperLimitMethod = iULAlgo;
}


void VLightCurve::setSpectralParameters( double iMinEnergy, double  E0, double alpha, double iMaxEnergy )
{
	fMinEnergy = iMinEnergy;
	fMaxEnergy = iMaxEnergy;
	fE0 = E0;
	fAlpha = alpha;
}


bool VLightCurve::plotObservingPeriods( TCanvas* iCanvasLightCurve, string iDataFile, int iColor )
{
	if( !iCanvasLightCurve || !iCanvasLightCurve->cd() )
	{
		return false;
	}
	
	iDataFile = "";
	iColor = 1;
	
	// read text file with observing periods
	
	/*
	
	    to ob added
	
	*/
	
	return true;
}

void VLightCurve::setLightCurveAxis( double iYmin, double iYmax, string iAxisTitle )
{
	fRateAxisMin = iYmin;
	fRateAxisMax = iYmax;
	fRateAxisTitle = iAxisTitle;
}

/*

   fill a histogram with time intervalls between flux measurements

*/
TH1D* VLightCurve::fillObservingIntervallHistogram( bool bPlot, double iPlotMax, string iName, string iTitle )
{
	if( !fObservingInvervallHisto )
	{
		fObservingInvervallHisto = new TH1D( iName.c_str(), "", 10000, 0., 1000. );
		fObservingInvervallHisto->SetXTitle( "observing interval #Delta t [d]" );
		setHistogramPlottingStyle( fObservingInvervallHisto );
	}
	else
	{
		fObservingInvervallHisto->Reset();
	}
	
	int iLastBin = 0;
	double iLast = 0.;
	// get first bin with data
	for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
	{
		if( fLightCurveData[i] )
		{
			iLast = fLightCurveData[i]->getMJD();
			iLastBin = i;
			break;
		}
	}
	// fill histogram
	for( unsigned int i = iLastBin; i < fLightCurveData.size(); i++ )
	{
		if( fLightCurveData[i] )
		{
			fObservingInvervallHisto->Fill( fLightCurveData[i]->getMJD() - iLast );
			iLast = fLightCurveData[i]->getMJD();
		}
	}
	
	if( bPlot )
	{
		iName = "cL" + iName;
		TCanvas* c = new TCanvas( iName.c_str(), iTitle.c_str(), 10, 10, 400, 400 );
		c->SetGridx( 0 );
		c->SetGridx( 0 );
		
		fObservingInvervallHisto->SetAxisRange( 0., iPlotMax );
		fObservingInvervallHisto->Draw();
	}
	
	return fObservingInvervallHisto;
}

/*

    fill up gas in light curve by using information from information from other phases

    new light curve is written to a ascii light curve file

    iGapsToFill_days :   all gaps larger than this value is filled
    iPhaseBinning :      binning assumed for average phase binned light curve

*/
bool VLightCurve::fillLightCurveMCPhaseFolded( string iOutFile, double iGapsToFill_days, double iPhaseBinning, bool bPlotDebug )
{
	bool bFillRandomMC = false;
	
	if( iPhaseBinning <= 0. )
	{
		return false;
	}
	
	// fill histogram with time intervalls between measurements
	fillObservingIntervallHistogram( false );
	
	// fill mean phase folded light curve
	TProfile hPTemp( "hT", "", int( 1. / iPhaseBinning ), 0., 1., getFlux_Min(), getFlux_Max() );
	for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
	{
		if( fLightCurveData[i] )
		{
			hPTemp.Fill( fLightCurveData[i]->getPhase(), fLightCurveData[i]->fFlux );
		}
	}
	// copy light curve data
	vector< VLightCurveData* > iMCLightCurveData;
	for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
	{
		if( fLightCurveData[i] && bFillRandomMC )
		{
			iMCLightCurveData.push_back( new VLightCurveData( ( *fLightCurveData[i] ) ) );
		}
	}
	
	// now start at first flux entry and fill gaps larger than iGapsToFill_days
	for( unsigned int i = 0; i < fLightCurveData.size() - 1; i++ )
	{
		if( bFillRandomMC )
		{
			if( fLightCurveData[i] && fLightCurveData[i + 1] )
			{
				if( fLightCurveData[i + 1]->getMJD() - fLightCurveData[i]->getMJD() > iGapsToFill_days )
				{
					double iMJD_new = fLightCurveData[i]->fMJD_Data_min;
					while( iMJD_new < fLightCurveData[i + 1]->fMJD_Data_min )
					{
						double iMJD_new_min = iMJD_new + fObservingInvervallHisto->GetRandom();
						double iMJD_new_max = iMJD_new_min + getMeanObservationInterval();
						cout << "MJD " << iMJD_new_min << "\t" << iMJD_new_max << endl;
						if( hPTemp.GetBinContent( hPTemp.FindBin( getPhase( iMJD_new_min ) ) ) > 0. )
						{
							VLightCurveData* iL = new VLightCurveData();
							iL->fMJD_Data_min = iMJD_new_min;
							iL->fMJD_Data_max = iMJD_new_max;
							
							iL->fFlux =   hPTemp.GetBinContent( hPTemp.FindBin( getPhase( iMJD_new_min ) ) )
										  + gRandom->Gaus( 0., hPTemp.GetBinError( hPTemp.FindBin( getPhase( iMJD_new_min ) ) ) );
							iL->setFluxError( TMath::Abs( gRandom->Gaus( 0., getFluxError_Mean() ) ) );
							
							iMCLightCurveData.push_back( iL );
						}
						iMJD_new = iMJD_new_max;
					}
				}
			}
		}
		// add to every flux point at phase <0.5 a point a phase +0.5
		// (note some hardcode stuff)
		else
		{
			if( fLightCurveData[i] )
			{
				VLightCurveData* iL = new VLightCurveData();
				iL->fMJD_Data_min = fLightCurveData[i]->fMJD_Data_min;
				iL->fMJD_Data_max = iL->fMJD_Data_min + fLightCurveData[i]->getMJDError();
				iL->fFlux = fLightCurveData[i]->fFlux;
				iL->setFluxError( fLightCurveData[i]->getFluxError() );
				
				iMCLightCurveData.push_back( iL );
				
				if( getPhase( fLightCurveData[i]->getMJD() ) < 0.5 )
				{
					iL = new VLightCurveData();
					iL->fMJD_Data_min = fLightCurveData[i]->fMJD_Data_min + fPhase_Period_days / 2.;
					iL->fMJD_Data_max = iL->fMJD_Data_min + fLightCurveData[i]->getMJDError();
					
					iL->fFlux = fLightCurveData[i]->fFlux;
					iL->setFluxError( fLightCurveData[i]->getFluxError() );
					
					iMCLightCurveData.push_back( iL );
				}
			}
		}
	}
	// sort new light curve
	VLightCurveDataLessThan vlt;
	sort( iMCLightCurveData.begin(), iMCLightCurveData.end(), vlt );
	
	cout << "new MC light curve with " << iMCLightCurveData.size() << " data points" << endl;
	cout << "\t (assuming a period of " << fPhase_Period_days << " days)" << endl;
	
	// write light curve
	writeASCIIFile( iOutFile, iMCLightCurveData );
	
	return true;
}


double VLightCurve::getLightCurveAxisRange_Min()
{
	double iFluxMin = 1.e99;
	
	if( fRateAxisMin >= -1.e5 )
	{
		iFluxMin = fRateAxisMin;
	}
	else
	{
		for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
		{
			if( fLightCurveData[i] && fLightCurveData[i]->fFluxErrorDown > 0. && fLightCurveData[i]->fFlux - fLightCurveData[i]->fFluxErrorDown < iFluxMin )
			{
				iFluxMin = fLightCurveData[i]->fFlux - fLightCurveData[i]->fFluxErrorDown;
			}
			else if( fLightCurveData[i] && fLightCurveData[i]->fUpperFluxLimit > 0. && fLightCurveData[i]->fUpperFluxLimit < iFluxMin )
			{
				iFluxMin = fLightCurveData[i]->fUpperFluxLimit;
			}
		}
	}
	
	return iFluxMin;
}

double VLightCurve::getLightCurveAxisRange_Max()
{
	double iFluxMax = -1.;
	
	if( fRateAxisMax >= 0. )
	{
		iFluxMax = fRateAxisMax;
	}
	else
	{
		for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
		{
			if( fLightCurveData[i] && fLightCurveData[i]->fFluxErrorUp > 0. && fLightCurveData[i]->fFlux + fLightCurveData[i]->fFluxErrorUp > iFluxMax )
			{
				iFluxMax = fLightCurveData[i]->fFlux + fLightCurveData[i]->fFluxErrorUp;
			}
			else if( fLightCurveData[i] && fLightCurveData[i]->fUpperFluxLimit > 0. && fLightCurveData[i]->fUpperFluxLimit > iFluxMax )
			{
				iFluxMax = fLightCurveData[i]->fUpperFluxLimit;
			}
		}
	}
	
	return iFluxMax;
}

string VLightCurve::getLightCurveAxisTitle()
{
	char hname[1000];
	
	if( fRateAxisTitle == "tevRate" )
	{
		if( fEnergy_max_TeV < 0. )
		{
			// determine number of decimal places (do not allow more than three)
			if( ( int )( fEnergy_min_TeV * 10. ) % 10 == 0 && ( int )( fEnergy_min_TeV * 100. ) % 10 == 0 && ( int )( fEnergy_min_TeV * 1000. ) % 10 == 0 )
			{
				sprintf( hname, "Flux (E>%.0f TeV) [cm^{-2}s^{-1}]", fEnergy_min_TeV );
			}
			else if( ( int )( fEnergy_min_TeV * 100. ) % 10 == 0 && ( int )( fEnergy_min_TeV * 1000. ) % 10 == 0 )
			{
				sprintf( hname, "Flux (E>%.1f TeV) [cm^{-2}s^{-1}]", fEnergy_min_TeV );
			}
			else if( ( int )( fEnergy_min_TeV * 1000. ) % 10 == 0 )
			{
				sprintf( hname, "Flux (E>%.2f TeV) [cm^{-2}s^{-1}]", fEnergy_min_TeV );
			}
			else
			{
				sprintf( hname, "Flux (E>%.3f TeV) [cm^{-2}s^{-1}]", fEnergy_min_TeV );
			}
		}
		else
		{
			sprintf( hname, "Flux ( %.1f - %.1f TeV) [cm^{-2}s^{-1}]", fEnergy_min_TeV, fEnergy_max_TeV );
		}
		
		string iTemp = hname;
		fRateAxisTitle = hname;
		return iTemp;
	}
	else if( fRateAxisTitleUnSet )
	{
		fRateAxisTitle = "no title";
	}
	
	
	return fRateAxisTitle;
}

/*

   show impact of error on period on phaseograms by using randomizing the period and filling a 2D histogram

*/
bool VLightCurve::fillRandomizedPhaseogram( double iMCCycles, double iPhaseError_low, double iPhaseErrorUp, string iHisName, double iHisMin_y, double iHisMax_y )
{
	// 2D histogram
	if( fMCRandomizedPhaseogram == 0 )
	{
		fMCRandomizedPhaseogram = new TH2D( iHisName.c_str(), "", 100, 0., 1., 100, iHisMin_y, iHisMax_y );
		fMCRandomizedPhaseogram->SetStats( 0 );
	}
	else
	{
		fMCRandomizedPhaseogram->Reset();
	}
	// profile histogram
	if( fMCRandomizedPhaseogramProf == 0 )
	{
		iHisName += "prof";
		fMCRandomizedPhaseogramProf = new TProfile( iHisName.c_str(), "", 100, 0., 1., iHisMin_y, iHisMax_y );
		fMCRandomizedPhaseogramProf->SetStats( 0 );
	}
	else
	{
		fMCRandomizedPhaseogramProf->Reset();
	}
	
	// keep old period
	double iPhase_Period_days_backup = fPhase_Period_days;
	
	double x = 0.;
	double y = 0.;
	double iPhaseOrbit = 0.;
	double iPhaseOrbitError = 0.;
	
	for( unsigned int i = 0; i < iMCCycles; i++ )
	{
		if( gRandom->Uniform( 1. ) < 0.5 )
		{
			iPhaseOrbitError = -1.*TMath::Abs( gRandom->Gaus( 0., iPhaseError_low ) );
		}
		else
		{
			iPhaseOrbitError = TMath::Abs( gRandom->Gaus( 0., iPhaseErrorUp ) );
		}
		iPhaseOrbit = iPhase_Period_days_backup + iPhaseOrbitError;
		
		setPhaseFoldingValues( fPhase_MJD0, iPhaseOrbit, fPhasePlotting );
		
		for( unsigned int j = 0; j < fLightCurveData.size(); j++ )
		{
			// second MC cycle: take errors in flux into account
			x = getPhase( fLightCurveData[j]->getMJD() );
			for( unsigned int k = 0; k < iMCCycles; k++ )
			{
				y = gRandom->Gaus( fLightCurveData[j]->fFlux, fLightCurveData[j]->getFluxError() );
				
				fMCRandomizedPhaseogram->Fill( x, y );
				fMCRandomizedPhaseogramProf->Fill( x, y );
			}
		}
	}
	
	// restore old phase values
	setPhaseFoldingValues( fPhase_MJD0, iPhase_Period_days_backup, fPhasePlotting );
	
	return true;
}

/*

   plot phase distribution for different flux states

*/
TCanvas* VLightCurve::plotPhaseDistribution( TCanvas* iCanvasPhaseDist, string iCanvasName, string iFluxState, int iColor )
{
	char hname[800];
	char htitle[800];
	
	cout << "plotPhaseDistribution" << endl;
	
	TH1D* hPhaseDist = 0;
	
	if( !iCanvasPhaseDist )
	{
		sprintf( hname, "%s", iCanvasName.c_str() );
		if( fName.size() > 0 )
		{
			sprintf( htitle, "phase distribution: %s", fName.c_str() );
		}
		else
		{
			sprintf( htitle, "phase distribution" );
		}
		
		fCanvasPhaseDistribution = new TCanvas( hname, htitle, 10, 10, 400, 400 );
		fCanvasPhaseDistribution->SetGridx( 0 );
		fCanvasPhaseDistribution->SetGridy( 0 );
		fCanvasPhaseDistribution->Draw();
	}
	
	// histograms
	sprintf( hname, "hPhaseDist_%d_%d_%d_%s", ( int )fPhase_MJD0, ( int )fPhase_Period_days, ( int )fPhasePlotting, iFluxState.c_str() );
	hPhaseDist = new TH1D( hname, "", 50, 0., 1. );
	hPhaseDist->SetStats( 0 );
	hPhaseDist->GetXaxis()->CenterTitle( true );
	hPhaseDist->SetXTitle( "orbital phase" );
	hPhaseDist->SetLineColor( iColor );
	
	// fill histogram
	for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
	{
		if( fLightCurveData[i] )
		{
			if( iFluxState.size() == 0 || fLightCurveData[i]->fFluxState == iFluxState )
			{
				hPhaseDist->Fill( fLightCurveData[i]->getPhase() );
			}
		}
	}
	
	if( !iCanvasPhaseDist )
	{
		iCanvasPhaseDist = fCanvasPhaseDistribution;
		
		iCanvasPhaseDist->cd();
		
		hPhaseDist->Draw();
	}
	else
	{
		fCanvasPhaseDistribution = iCanvasPhaseDist;
		
		iCanvasPhaseDist->cd();
		
		hPhaseDist->Draw( "same" );
	}
	
	return fCanvasPhaseDistribution;
}
