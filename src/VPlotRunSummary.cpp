/*! \class VPlotRunSummary
    \brief different plot routines (rates vs. elevation etc)

*/

#include "VPlotRunSummary.h"

ClassImp( VPlotRunSummary )

VPlotRunSummary::VPlotRunSummary( string ifile, unsigned int iTot, string iname, int icolor, int imarker, bool b1129 )
{
	fMinRun = -1;
	fMaxRun = -1;

	setElevationRange();

	fNPhaseBins = 0;
	fPhaseOrbit = 0.;
	fPhaseT0 = 0.;
	fPhasePoff = 0.;

	bIsZombie = false;

	if( !openfile( ifile, iTot ) )
	{
		cout << "file not found " << ifile << endl;
		bIsZombie = true;
		return;
	}

	defineGraphsAndHistograms( iname, icolor, imarker, b1129 );
	default_settings();
}


void VPlotRunSummary::printDailyRates()
{
	if( !fIn->IsZombie() )
	{
		cout << "printing daily rates for " << fIn->GetName() << endl;
	}

	for( unsigned int i = 0; i < fDayDate.size(); i++ )
	{
		cout << ( int )fDayDate[i] << "\t";
		cout << ( int )fDayMJD[i] << "\t";
		cout << fDayNOn[i] << "\t" << fDayNOff[i] << "\t" << fDayAlpha[i] << "\t";
		cout << fDayRate[i] << " +- " << fDayRateE[i] << "\t";
		cout << fDaySigni[i] << "\t";
		cout << endl;
	}
}


/*!
    take a runlist, read in runs, compare with list of runs from root files,
    make new runlist for each phase using the lines from the old runlist
*/
void VPlotRunSummary::makeNewPhaseRunList( string ifile, string ilist )
{
	string iListName = ilist.substr( 0, ilist.rfind( "." ) );
	cout << "creating new run lists from " << ilist << endl;

	// read in run list
	vector< string > iListFileLine;

	ifstream is;
	is.open( ilist.c_str(), ifstream::in );
	if( !is )
	{
		cout << "error opening list file " << endl;
		return;
	}
	string iTemp;
	while( getline( is, iTemp ) )
	{
		if( iTemp.substr( 0, 1 ) == "*" )
		{
			iListFileLine.push_back( iTemp );
		}
	}
	is.close();
	cout << "total number of runs in list file: " << iListFileLine.size() << endl;
	cout << "total number of runs in root file: " << fRunList.size() << endl;

	char hname[600];
	// write a shell script to run anasum over all new runlist
	sprintf( hname, "%s_%dPhases.sh", iListName.c_str(), ( int )fPhaseRunlist.size() );
	cout << "output scripts is " << hname << endl;
	ofstream os_script( hname );
	if( !os_script )
	{
		cout << "error open anasum run script" << endl;
		return;
	}
	os_script << "# run script for anasum for phase analysis" << endl;
	os_script << "# source file: " << ifile << endl;
	os_script << "# source run list: " << ilist << endl;
	os_script << endl;

	// now loop over all phases, get the runs and write new run list
	for( unsigned int l = 0; l < fPhaseRunlist.size(); l++ )
	{
		// only do something if there are runs in this phase
		if( fPhaseRunlist[l].size() == 0 )
		{
			continue;
		}

		// open new output file
		sprintf( hname, "%s_%dPhases_%d.dat", iListName.c_str(), ( int )fPhaseRunlist.size(), l );
		cout << "\t writing phase " << l << " to " << hname << endl;
		cout << "\t (phase bin " << fPhaseMin[l] << " < " << fPhaseMax[l] << ")" << endl;
		ofstream os( hname );
		if( !os )
		{
			cout << "error opening new run list file for phase " << l << " (" << hname << ")" << endl;
			continue;
		}
		// loop over runs
		for( unsigned int ll = 0; ll < fPhaseRunlist[l].size(); ll++ )
		{
			for( unsigned int g = 0; g < iListFileLine.size(); g++ )
			{
				sprintf( hname, "%d", ( int )fPhaseRunlist[l][ll] );
				if( iListFileLine[g].find( hname ) < iListFileLine[g].size() )
				{
					os << iListFileLine[g] << endl;
					break;
				}
			}
		}
		os.close();
		// add a new line to the run script
		sprintf( hname, "%s_%dPhases_%d.dat", iListName.c_str(), ( int )fPhaseRunlist.size(), l );
		os_script << "./anasum -l " << hname << " -i " << ifile;
		sprintf( hname, "%s_%dPhases_%d.root", iListName.c_str(), ( int )fPhaseRunlist.size(), l );
		os_script << " -o " << hname;
		sprintf( hname, "%s_%dPhases_%d.txt", iListName.c_str(), ( int )fPhaseRunlist.size(), l );
		os_script << " > " << hname;
		os_script << endl;
	}
	os_script.close();
}


void VPlotRunSummary::printPhaseList()
{

	for( unsigned int l = 0; l < fPhaseRunlist.size(); l++ )
	{
		cout << endl;
		cout << "Phase bin " << l << " (" << fPhaseMin[l] << " < " << fPhaseMax[l] << ")" << endl;
		cout << "\t" << fPhaseRunlist[l].size() << " runs" << endl;
		cout << "-----------------------" << endl;
		for( unsigned int ll = 0; ll < fPhaseRunlist[l].size(); ll++ )
		{
			cout << "\t" << fPhaseRunlist[l][ll] << "\t" << fPhasePerRun[l][ll] << "\t";
			cout << fPhaseRatePerRun[l][ll] << "+-" << fPhaseRateEPerRun[l][ll] << "\t" << fPhaseSigniPerRun[l][ll] << endl;
		}
	}
}


void VPlotRunSummary::setPhaseValues( unsigned int inPhase, double orbit, double t0, double poff )
{
	fNPhaseBins = inPhase;
	fPhaseOrbit = orbit;
	fPhaseT0 = t0;
	fPhasePoff = poff;

	hRunDuration->SetBins( fNPhaseBins, 0. + poff, 1. + poff );
}


void VPlotRunSummary::defineGraphsAndHistograms( string iname, int icolor, int imarker, bool b1129 )
{
	// define graphs and histograms

	char hname[200];

	gList = new TList();

	sprintf( hname, "hRate_%s", iname.c_str() );
	hRate = new TH1D( hname, "", 100, 0., 20. );
	hRate->SetXTitle( "#gamma-ray rate [1/min]" );
	hRate->SetYTitle( "entries" );
	setHistogramPlottingStyle( hRate, icolor, 2, 2, imarker, 1, -1 );
	gList->Add( hRate );

	sprintf( hname, "hSignificance_%s", iname.c_str() );
	hSignificance = new TH1D( hname, "", 100, 0., 30. );
	hSignificance->SetXTitle( "significance" );
	hSignificance->SetYTitle( "entries" );
	setHistogramPlottingStyle( hSignificance, icolor, 2, 2, imarker, 1, -1 );
	gList->Add( hSignificance );

	int npoints = c->fChain->GetEntries() - 1;
	if( b1129 )
	{
		npoints -= 2;
	}

	sprintf( hname, "hRunDuration_%s", iname.c_str() );
	hRunDuration = new TH1D( hname, "", fNPhaseBins, 0., 1. );
	hRunDuration->SetStats( 0 );
	hRunDuration->SetXTitle( "orbital phase" );
	hRunDuration->SetYTitle( "observation time [min]" );
	hRunDuration->SetLineWidth( 2 );
	hRunDuration->SetLineColor( icolor );
	gList->Add( hRunDuration );

	gRatevsTime = new TGraphErrors( 1 );
	setGraphPlottingStyle( gRatevsTime, icolor, 2, imarker, 2 );

	gRateOffvsTime = new TGraphErrors( 1 );
	setGraphPlottingStyle( gRateOffvsTime, icolor, 2, imarker, 2 );

	gSignificancevsTime = new TGraphErrors( 1 );
	setGraphPlottingStyle( gSignificancevsTime, icolor, 2, imarker, 2 );

	gCumSignificancevsTime = new TGraphErrors( 1 );
	setGraphPlottingStyle( gCumSignificancevsTime, icolor, 2, imarker, 2 );

	gRatevsWobbleDirection = new TGraphErrors( 1 );
	setGraphPlottingStyle( gRatevsWobbleDirection, icolor, 2, imarker, 2 );

	gRateOffvsWobbleDirection = new TGraphErrors( 1 );
	setGraphPlottingStyle( gRateOffvsWobbleDirection, icolor, 2, imarker, 2 );

	gSignificancevsWobbleDirection = new TGraphErrors( 4 );
	setGraphPlottingStyle( gSignificancevsWobbleDirection, icolor, 2, imarker, 2 );

	gElevationvsWobbleDirection = new TGraphErrors( 4 );
	setGraphPlottingStyle( gElevationvsWobbleDirection, icolor, 2, imarker, 2 );

	gRatevsElevation = new TGraphErrors( 1 );
	setGraphPlottingStyle( gRatevsElevation, icolor, 2, imarker, 2 );

	gRateOffvsElevation = new TGraphErrors( 1 );
	setGraphPlottingStyle( gRateOffvsElevation, icolor, 2, imarker, 2 );

	gRatevsWobbleOffset = new TGraphErrors( 1 );
	setGraphPlottingStyle( gRatevsWobbleOffset, icolor, 2, imarker, 2 );

	gRateOffvsWobbleOffset = new TGraphErrors( 1 );
	setGraphPlottingStyle( gRateOffvsWobbleOffset, icolor, 2, imarker, 2 );

	gRatevsElevationBinned = new TGraphErrors( 5 );
	setGraphPlottingStyle( gRatevsElevationBinned, icolor, 2, imarker, 2 );

	gRawRatevsElevation = new TGraphErrors( npoints );
	setGraphPlottingStyle( gRawRatevsElevation, icolor, 2, imarker, 2 );

	gRawRateOffvsElevation = new TGraphErrors( npoints );
	setGraphPlottingStyle( gRawRateOffvsElevation, icolor, 2, imarker, 2 );

	gRawRatevsElevationBinned = new TGraphErrors( 5 );
	setGraphPlottingStyle( gRawRatevsElevationBinned, icolor, 2, imarker, 2 );

	gSignificancevsElevation = new TGraphErrors( npoints );
	setGraphPlottingStyle( gSignificancevsElevation, icolor, 2, imarker, 2 );

	gSignificancevsElevationBinned = new TGraphErrors( 5 );
	setGraphPlottingStyle( gSignificancevsElevationBinned, icolor, 2, imarker, 2 );

	sprintf( hname, "hMaxSigniXY_%s", iname.c_str() );
	hMaxSigniXY = new TH2D( hname, "", 200, -1., 1., 200, -1., 1. );
	hMaxSigniXY->SetXTitle( "x-position on sky of maximum significance [deg]" );
	hMaxSigniXY->SetYTitle( "y-position on sky of maximum significance [deg]" );
	hMaxSigniXY->SetStats( 0 );

	gRatevsPhase = new TGraphErrors( 1 );
	setGraphPlottingStyle( gRatevsPhase, icolor, 2, imarker, 2 );

	gSignificancevsPhase = new TGraphErrors( 1 );
	setGraphPlottingStyle( gSignificancevsPhase, icolor, 2, imarker, 2 );

	gElevationvsPhase = new TGraphErrors( 1 );
	setGraphPlottingStyle( gElevationvsPhase, icolor, 2, imarker, 2 );

	gRawRatevsPhase = new TGraphErrors( 1 );
	setGraphPlottingStyle( gRawRatevsPhase, icolor, 2, imarker, 2 );

	gRatevsPhaseBinned = new TGraphErrors( fNPhaseBins );
	setGraphPlottingStyle( gRatevsPhaseBinned, icolor, 2, imarker, 2 );

	gSignificancevsPhaseBinned = new TGraphErrors( fNPhaseBins );
	setGraphPlottingStyle( gSignificancevsPhaseBinned, icolor, 2, imarker, 2 );

	gElevationvsPhaseBinned = new TGraphErrors( fNPhaseBins );
	setGraphPlottingStyle( gElevationvsPhaseBinned, icolor, 2, imarker, 2 );

	gRawRatevsPhaseBinned = new TGraphErrors( fNPhaseBins );
	setGraphPlottingStyle( gRawRatevsPhaseBinned, icolor, 2, imarker, 2 );

	gRatesvsTimeDaily = new TGraphErrors( 1 );
	setGraphPlottingStyle( gRatesvsTimeDaily, icolor, 2, imarker, 2 );

	gRatevsPhaseDaily = new TGraphErrors( 1 );
	setGraphPlottingStyle( gRatevsPhaseDaily, icolor, 2, imarker, 2 );

	gSignificancevsPhaseDaily = new TGraphErrors( 1 );
	setGraphPlottingStyle( gSignificancevsPhaseDaily, icolor, 2, imarker, 2 );

	sprintf( hname, "hPedvars_%s", iname.c_str() );
	hPedvars = new TH1D( hname, "", 150, 0., 20. );
	hPedvars->SetXTitle( "mean pedestal variations" );
	setHistogramPlottingStyle( hPedvars );
	hPedvars->SetStats( 1 );

	sprintf( hname, "hAbsAzimuth_%s", iname.c_str() );
	hAbsAzimuth = new TH1D( hname, "", 25, 0., 180. );
	hAbsAzimuth->SetXTitle( "azimuth (absolute value) [deg]" );
	setHistogramPlottingStyle( hAbsAzimuth );
	hAbsAzimuth->SetStats( 1 );

	sprintf( hname, "hElevation_%s", iname.c_str() );
	hElevation = new TH1D( hname, "", 50, 20., 90. );
	hElevation->SetXTitle( "elevation [deg]" );
	setHistogramPlottingStyle( hElevation );
	hElevation->SetStats( 1 );

}


bool VPlotRunSummary::openfile( string file, int iTot )
{
	fIn = new TFile( file.c_str() );
	if( fIn->IsZombie() )
	{
		return false;
	}

	char hfile[200];
	if( iTot > 0 )
	{
		sprintf( hfile, "total_%d/stereo", iTot );
	}
	else if( iTot < 0 )
	{
		sprintf( hfile, "total_%d/stereo", -1 * iTot );
	}
	else
	{
		sprintf( hfile, "total/stereo" );
	}
	if( !fIn->cd( hfile ) )
	{
		return false;
	}

	t = ( TTree* )gDirectory->Get( "tRunSummary" );
	if( !t )
	{
		return false;
	}

	c = new CRunSummary( t );

	return true;
}


void VPlotRunSummary::fill()
{
	int nentries = c->fChain->GetEntries();

	// reset some histograms
	hRate->Reset();
	hSignificance->Reset();
	hRunDuration->Reset();
	hPedvars->Reset();
	hAbsAzimuth->Reset();
	hElevation->Reset();

	// mean rate calculation

	// mean vs wobble direction (in steps of 0.05)
	const int i_nWobble = 40;
	double i_ratevsWobbleDirectionX[i_nWobble];
	for( int i = 0; i < i_nWobble; i++ )
	{
		i_ratevsWobbleDirectionX[i] = 0.;
	}
	double i_ratevsWobbleDirectionXX[i_nWobble];
	for( int i = 0; i < i_nWobble; i++ )
	{
		i_ratevsWobbleDirectionXX[i] = 0.;
	}
	double i_rateOffvsWobbleDirectionX[i_nWobble];
	for( int i = 0; i < i_nWobble; i++ )
	{
		i_rateOffvsWobbleDirectionX[i] = 0.;
	}
	double i_rateOffvsWobbleDirectionXX[i_nWobble];
	for( int i = 0; i < i_nWobble; i++ )
	{
		i_rateOffvsWobbleDirectionXX[i] = 0.;
	}
	double i_ratevsWobbleDirectionN[i_nWobble];
	for( int i = 0; i < i_nWobble; i++ )
	{
		i_ratevsWobbleDirectionN[i] = 0.;
	}

	double i_SignificancevsWobbleDirectionX[i_nWobble];
	for( int i = 0; i < i_nWobble; i++ )
	{
		i_SignificancevsWobbleDirectionX[i] = 0.;
	}
	double i_SignificancevsWobbleDirectionXX[i_nWobble];
	for( int i = 0; i < i_nWobble; i++ )
	{
		i_SignificancevsWobbleDirectionXX[i] = 0.;
	}
	double i_SignificancevsWobbleDirectionN[i_nWobble];
	for( int i = 0; i < i_nWobble; i++ )
	{
		i_SignificancevsWobbleDirectionN[i] = 0.;
	}

	double i_ElevationvsWobbleDirectionX[i_nWobble];
	for( int i = 0; i < i_nWobble; i++ )
	{
		i_ElevationvsWobbleDirectionX[i] = 0.;
	}
	double i_ElevationvsWobbleDirectionXX[i_nWobble];
	for( int i = 0; i < i_nWobble; i++ )
	{
		i_ElevationvsWobbleDirectionXX[i] = 0.;
	}
	double i_ElevationvsWobbleDirectionN[i_nWobble];
	for( int i = 0; i < i_nWobble; i++ )
	{
		i_ElevationvsWobbleDirectionN[i] = 0.;
	}

	// mean vs elevation
	double i_ratevsElevationX[5];
	double i_ratevsElevationXX[5];
	double i_ratevsElevationN[5];
	double i_rawRatevsElevationX[5];
	double i_rawRatevsElevationXX[5];
	double i_rawRatevsElevationN[5];
	double i_SignificancevsElevationX[5];
	double i_SignificancevsElevationXX[5];
	double i_SignificancevsElevationN[5];
	for( int i = 0; i < 5; i++ )
	{
		i_ratevsElevationX[i] = 0.;
		i_ratevsElevationXX[i] = 0.;
		i_ratevsElevationN[i] = 0.;
		i_rawRatevsElevationX[i] = 0.;
		i_rawRatevsElevationXX[i] = 0.;
		i_rawRatevsElevationN[i] = 0.;
		i_SignificancevsElevationX[i] = 0.;
		i_SignificancevsElevationXX[i] = 0.;
		i_SignificancevsElevationN[i] = 0.;
	}

	//////////////////////////////////
	// phase plots

	// rate vs phase
	vector< double > i_ratevsPhaseX;
	vector< double > i_ratevsPhaseXX;
	vector< double > i_ratevsPhaseE;              //! only needed if there is only one run in the phase bin
	vector< double > i_ratevsPhaseN;
	// elevation vs phase
	vector< double > i_elevationvsPhaseX;
	vector< double > i_elevationvsPhaseXX;
	vector< double > i_elevationvsPhaseN;
	// raw rate vs phase
	vector< double > i_rawRatevsPhaseX;
	vector< double > i_rawRatevsPhaseXX;
	vector< double > i_rawRatevsPhaseN;
	// run list vs phase
	vector< double > itemp;
	vector< double > itempD;
	if( fNPhaseBins > 0 )
	{
		for( unsigned int j = 0; j < fNPhaseBins; j++ )
		{
			fPhaseMin.push_back( j * ( 1. / ( double )fNPhaseBins ) );
			fPhaseMax.push_back( ( j + 1 ) * ( 1. / ( double )fNPhaseBins ) );
			i_ratevsPhaseX.push_back( 0. );
			i_ratevsPhaseXX.push_back( 0. );
			i_ratevsPhaseE.push_back( 0. );
			i_ratevsPhaseN.push_back( 0. );
			i_rawRatevsPhaseX.push_back( 0. );
			i_rawRatevsPhaseXX.push_back( 0. );
			i_rawRatevsPhaseN.push_back( 0. );
			i_elevationvsPhaseX.push_back( 0. );
			i_elevationvsPhaseXX.push_back( 0. );
			i_elevationvsPhaseN.push_back( 0. );
			fPhaseRunlist.push_back( itemp );
			fPhasePerRun.push_back( itempD );
			fPhaseRatePerRun.push_back( itempD );
			fPhaseRateEPerRun.push_back( itempD );
			fPhaseSigniPerRun.push_back( itempD );
		}
	}

	//////////////////////////////////
	//////////////////////////////////
	// now loop over all runs
	// need additional counter because of skipped runs
	int t = 0;
	int t_f = 0;
	bool fSetRunRange = false;
	if( fMinRun < 0 )
	{
		fSetRunRange = true;
	}

	double cum_Non = 0.;
	double cum_Noff = 0.;
	double cum_alpha = 0.;
	double cum_t = 0.;

	bool bBreak = false;

	for( int i = 0; i < nentries; i++ )
	{
		c->GetEntry( i );

		///// test run range
		if( !fSetRunRange && ( ( fMinRun > 0 && c->runOn < fMinRun ) || ( fMaxRun > 0 && c->runOn > fMaxRun ) ) )
		{
			continue;
		}

		if( fSetRunRange && c->runOn > 0 && ( c->runOn < fMinRun || fMinRun < 0 ) )
		{
			fMinRun = c->runOn;
		}
		if( fSetRunRange && c->runOn > fMaxRun )
		{
			fMaxRun = c->runOn;
		}

		// loop over list of runs to be excluded
		bBreak = false;
		for( unsigned int r = 0; r < fListofExcludedRuns.size(); r++ )
		{
			if( fListofExcludedRuns[r] == c->runOn )
			{
				bBreak = true;
			}
		}
		if( bBreak )
		{
			continue;
		}

		if( c->elevationOn < fMinElevation || c->elevationOn > fMaxElevation )
		{
			continue;
		}

		// exclude runs with 0 rate and no error on rate
		if( c->runOn > 0 && !( TMath::Abs( c->Rate ) < 1.e-5 && TMath::Abs( c->RateE ) < 1.e-5 ) )
		{
			cout << "Run " << c->runOn << ": rate " << c->Rate << " +- " << c->RateE << " g/min";
			cout << ", elevation: " << c->elevationOn;
			cout << ", wobble: " << sqrt( c->WobbleNorth * c->WobbleNorth + c->WobbleWest * c->WobbleWest );
			cout << ", alpha: " << c->OffNorm;
			cout << endl;
			// 1D histograms
			hRate->Fill( c->Rate );
			hSignificance->Fill( c->Signi );
			hPedvars->Fill( c->pedvarsOn );
			hAbsAzimuth->Fill( TMath::Abs( c->azimuthOn ) );
			hElevation->Fill( c->elevationOn );

			gRatevsTime->SetPoint( t, c->MJDOn, c->Rate );
			if( c->tOn > 0. )
			{
				gRatevsTime->SetPointError( t, 0., sqrt( c->NOn + c->OffNorm * c->OffNorm * c->NOff ) / c->tOn * 60. );
			}

			gRateOffvsTime->SetPoint( t, c->MJDOn, c->RateOff );
			if( c->tOn > 0. )
			{
				gRateOffvsTime->SetPointError( t, 0., c->OffNorm * sqrt( c->NOff ) / c->tOn * 60. );
			}

			gSignificancevsTime->SetPoint( t, c->MJDOn, c->Signi );
			gSignificancevsTime->SetPointError( t, 0., 0. );

			// cumulative significance vs time
			if( c->OffNorm > 0 )
			{
				cum_Non += c->NOn;
				cum_Noff += c->NOff;
				cum_alpha += c->OffNorm * c->NOff;
				cum_t += c->tOn;

				if( cum_Noff > 0. )
				{
					gCumSignificancevsTime->SetPoint( t_f, cum_t / 60., VStatistics::calcSignificance( cum_Non, cum_Noff, cum_alpha / cum_Noff ) );
					gCumSignificancevsTime->SetPointError( t_f, 0., 0. );
					cout << "\t cumulative numbers: non: " << cum_Non << ", noff: " << cum_Noff << ", alpha: " << cum_alpha / cum_Noff;
					cout << ", time " << cum_t << ", significance " << VStatistics::calcSignificance( cum_Non, cum_Noff, cum_alpha / cum_Noff ) << endl;
					t_f++;
				}
			}

			// wobble direction plots
			int iAng = ( int )( sqrt( c->WobbleNorth * c->WobbleNorth + c->WobbleWest * c->WobbleWest ) * 20. + 0.5 );

			// (Temporary: dirty fix to get 1.43 point into 1.5 bin)
			if( iAng == 29 )
			{
				iAng = 30;
			}

			if( iAng < 40 )
			{
				i_ratevsWobbleDirectionX[iAng] += c->Rate;
				i_ratevsWobbleDirectionXX[iAng] += c->Rate * c->Rate;
				i_rateOffvsWobbleDirectionX[iAng] += c->RateOff;
				i_rateOffvsWobbleDirectionXX[iAng] += c->RateOff * c->RateOff;
				i_ratevsWobbleDirectionN[iAng]++;
				i_SignificancevsWobbleDirectionX[iAng] += c->Signi;
				i_SignificancevsWobbleDirectionXX[iAng] += c->Signi * c->Signi;
				i_SignificancevsWobbleDirectionN[iAng]++;

				i_ElevationvsWobbleDirectionX[iAng] += c->elevationOn;
				i_ElevationvsWobbleDirectionXX[iAng] += c->elevationOn * c->elevationOn;
				i_ElevationvsWobbleDirectionN[iAng]++;
			}

			// elevation plots
			if( c->elevationOn > 0. )
			{
				gRatevsElevation->SetPoint( t, c->elevationOn, c->Rate );
			}
			else
			{
				gRatevsElevation->SetPoint( t, c->elevationOff, c->Rate );
			}
			gRatevsElevation->SetPointError( t, 0., c->RateE );
			gRateOffvsElevation->SetPoint( t, c->elevationOff, c->RateOff );
			gRateOffvsElevation->SetPointError( t, 0., c->RateOffE );

			if( c->tOn > 0. )
			{
				gSignificancevsElevation->SetPoint( t, c->elevationOn, c->Signi / sqrt( c->tOn / 3600. ) );
			}
			gSignificancevsElevation->SetPointError( t, 0., 0. );

			gRawRatevsElevation->SetPoint( t, c->elevationOn, c->RawRateOn );
			gRawRatevsElevation->SetPointError( t, 0., 0. );

			gRawRateOffvsElevation->SetPoint( t, c->elevationOff, c->RawRateOff );
			gRawRateOffvsElevation->SetPointError( t, 0., 0. );

			gRatevsWobbleOffset->SetPoint( t, sqrt( c->WobbleNorth * c->WobbleNorth + c->WobbleWest * c->WobbleWest ), c->Rate );
			gRatevsWobbleOffset->SetPointError( t, 0., c->RateE );

			gRateOffvsWobbleOffset->SetPoint( t, sqrt( c->WobbleNorth * c->WobbleNorth + c->WobbleWest * c->WobbleWest ), c->RateOff );
			gRateOffvsWobbleOffset->SetPointError( t, 0., c->RateOffE );

			int iElevationBin = 0;
			double iEl = c->elevationOn;
			if( iEl < 1. )
			{
				iEl = c->elevationOff;
			}
			if( c->elevationOff < 60. )
			{
				iElevationBin = 0;
			}
			else if( c->elevationOff < 65. )
			{
				iElevationBin = 1;
			}
			else if( c->elevationOff < 70. )
			{
				iElevationBin = 2;
			}
			else if( c->elevationOff < 75. )
			{
				iElevationBin = 3;
			}
			else
			{
				iElevationBin = 4;
			}

			if( c->Rate > 0. )
			{
				i_ratevsElevationX[iElevationBin] += c->Rate;
				i_ratevsElevationXX[iElevationBin] += c->Rate * c->Rate;
				i_ratevsElevationN[iElevationBin]++;
			}
			if( c->RawRateOn > 0. )
			{
				i_rawRatevsElevationX[iElevationBin] += c->RawRateOn;
				i_rawRatevsElevationXX[iElevationBin] += c->RawRateOn * c->RawRateOn;
				i_rawRatevsElevationN[iElevationBin]++;
			}

			i_SignificancevsElevationX[iElevationBin] += c->Signi;
			i_SignificancevsElevationXX[iElevationBin] += c->Signi * c->Signi;
			i_SignificancevsElevationN[iElevationBin]++;

			fRunList.push_back( c->runOn );

			// phase plots
			if( fNPhaseBins > 0 )
			{

				double iPhase = getPhase( c->MJDOn );
				hRunDuration->Fill( iPhase, c->tOn / 60. );
				gRatevsPhase->SetPoint( t, iPhase, c->Rate );
				gRatevsPhase->SetPointError( t, 0., c->RateE );
				gRawRatevsPhase->SetPoint( t, iPhase, c->RawRateOn );
				gRawRatevsPhase->SetPointError( t, 0., 0. );
				gSignificancevsPhase->SetPoint( t, iPhase, c->Signi );
				gSignificancevsPhase->SetPointError( t, 0., 0. );
				gElevationvsPhase->SetPoint( t, iPhase, c->elevationOn );
				gElevationvsPhase->SetPointError( t, 0., 0. );
				for( unsigned int j = 0; j < fNPhaseBins; j++ )
				{
					if( iPhase > fPhaseMin[j]  && iPhase < fPhaseMax[j] )
					{
						i_ratevsPhaseX[j] += c->Rate;
						i_ratevsPhaseXX[j] += c->Rate * c->Rate;
						i_ratevsPhaseE[j] = c->RateE;
						i_ratevsPhaseN[j]++;
						i_rawRatevsPhaseX[j] += c->RawRateOn;
						i_rawRatevsPhaseXX[j] += c->RawRateOn * c->RawRateOn;
						i_rawRatevsPhaseN[j]++;
						i_elevationvsPhaseX[j] += c->elevationOn;
						i_elevationvsPhaseXX[j] += c->elevationOn * c->elevationOn;
						i_elevationvsPhaseN[j]++;
						fPhaseRunlist[j].push_back( c->runOn );
						fPhasePerRun[j].push_back( iPhase );
						fPhaseRatePerRun[j].push_back( c->Rate );
						fPhaseRateEPerRun[j].push_back( c->RateE );
						fPhaseSigniPerRun[j].push_back( c->Signi );
						break;
					}
				}
			}

			// position of maximum significance
			hMaxSigniXY->Fill( c->MaxSigniX, c->MaxSigniY );
			t++;
		}
	}
	cout << "range of runs: " << fMinRun << "\t" << fMaxRun;
	cout << ", number of runs: " << t << endl;

	/////////////////////////////
	// fill mean rates

	// set rate vs wobble direction
	int z = 0;
	for( int i = 0; i < 40; i++ )
	{
		if( i_ratevsWobbleDirectionN[i] > 1. )
		{
			gRatevsWobbleDirection->SetPoint( z, ( double )i / 20., i_ratevsWobbleDirectionX[i] / i_ratevsWobbleDirectionN[i] );
			gRatevsWobbleDirection->SetPointError( z, 0., sqrt( 1. / i_ratevsWobbleDirectionN[i] / ( i_ratevsWobbleDirectionN[i] - 1 ) * ( i_ratevsWobbleDirectionXX[i] - 1. / i_ratevsWobbleDirectionN[i]*i_ratevsWobbleDirectionX[i]*i_ratevsWobbleDirectionX[i] ) ) );

			gRateOffvsWobbleDirection->SetPoint( z, ( double )i / 20., i_rateOffvsWobbleDirectionX[i] / i_ratevsWobbleDirectionN[i] );
			gRateOffvsWobbleDirection->SetPointError( z, 0., sqrt( 1. / i_ratevsWobbleDirectionN[i] / ( i_ratevsWobbleDirectionN[i] - 1 ) * ( i_rateOffvsWobbleDirectionXX[i] - 1. / i_ratevsWobbleDirectionN[i]*i_rateOffvsWobbleDirectionX[i]*i_rateOffvsWobbleDirectionX[i] ) ) );

			z++;
		}
	}
	// significance vs wobble direction
	z = 0;
	for( int i = 0; i < 40; i++ )
	{
		if( i_SignificancevsWobbleDirectionN[i] > 1. )
		{
			gSignificancevsWobbleDirection->SetPoint( z, ( double )i / 20., i_SignificancevsWobbleDirectionX[i] / i_SignificancevsWobbleDirectionN[i] );
			gSignificancevsWobbleDirection->SetPointError( z, 0., sqrt( 1. / i_SignificancevsWobbleDirectionN[i] / ( i_SignificancevsWobbleDirectionN[i] - 1 ) * ( i_SignificancevsWobbleDirectionXX[i] - 1. / i_SignificancevsWobbleDirectionN[i]*i_SignificancevsWobbleDirectionX[i]*i_SignificancevsWobbleDirectionX[i] ) ) );
			z++;
		}
	}
	// elevation vs wobble direction
	z = 0;
	for( int i = 0; i < 40; i++ )
	{
		if( i_ElevationvsWobbleDirectionN[i] > 1. )
		{
			gElevationvsWobbleDirection->SetPoint( z, ( double )i / 20., i_ElevationvsWobbleDirectionX[i] / i_ElevationvsWobbleDirectionN[i] );
			gElevationvsWobbleDirection->SetPointError( z, 0., sqrt( 1. / i_ElevationvsWobbleDirectionN[i] / ( i_ElevationvsWobbleDirectionN[i] - 1 ) * ( i_ElevationvsWobbleDirectionXX[i] - 1. / i_ElevationvsWobbleDirectionN[i]*i_ElevationvsWobbleDirectionX[i]*i_ElevationvsWobbleDirectionX[i] ) ) );
			z++;
		}
	}

	// rate vs elevation
	double elevation = 55.;
	for( int i = 0; i < 5; i++ )
	{
		if( i == 0 )
		{
			elevation = 55.;
		}
		else if( i == 1 )
		{
			elevation = 62.5;
		}
		else if( i == 2 )
		{
			elevation = 67.5;
		}
		else if( i == 3 )
		{
			elevation = 72.5;
		}
		else
		{
			elevation = 80.;
		}

		if( i_ratevsElevationN[i] > 1. )
		{
			gRatevsElevationBinned->SetPoint( i, elevation, i_ratevsElevationX[i] / i_ratevsElevationN[i] );
			gRatevsElevationBinned->SetPointError( i, 0., sqrt( 1. / i_ratevsElevationN[i] / ( i_ratevsElevationN[i] - 1 ) * ( i_ratevsElevationXX[i] - 1. / i_ratevsElevationN[i]*i_ratevsElevationX[i]*i_ratevsElevationX[i] ) ) );
		}
		else if( i_ratevsElevationN[i] == 1 )
		{
			gRatevsElevationBinned->SetPoint( i, elevation, i_ratevsElevationX[i] );
			gRatevsElevationBinned->SetPointError( i, 0., 0. );
		}
		else
		{
			gRatevsElevationBinned->RemovePoint( i );
		}

		if( i_SignificancevsElevationN[i] > 1. )
		{
			gSignificancevsElevationBinned->SetPoint( i, elevation, i_SignificancevsElevationX[i] / i_SignificancevsElevationN[i] );
			gSignificancevsElevationBinned->SetPointError( i, 0., sqrt( 1. / i_SignificancevsElevationN[i] / ( i_SignificancevsElevationN[i] - 1 ) * ( i_SignificancevsElevationXX[i] - 1. / i_SignificancevsElevationN[i]*i_SignificancevsElevationX[i]*i_SignificancevsElevationX[i] ) ) );
		}
		else if( i_SignificancevsElevationN[i] == 1 )
		{
			gSignificancevsElevationBinned->SetPoint( i, elevation, i_SignificancevsElevationX[i] );
			gSignificancevsElevationBinned->SetPointError( i, 0., 0. );
		}
		else
		{
			gSignificancevsElevationBinned->RemovePoint( i );
		}

		if( i_rawRatevsElevationN[i] > 1. )
		{
			gRawRatevsElevationBinned->SetPoint( i, elevation, i_rawRatevsElevationX[i] / i_rawRatevsElevationN[i] );
			gRawRatevsElevationBinned->SetPointError( i, 0., sqrt( 1. / i_rawRatevsElevationN[i] / ( i_rawRatevsElevationN[i] - 1 ) * ( i_rawRatevsElevationXX[i] - 1. / i_rawRatevsElevationN[i]*i_rawRatevsElevationX[i]*i_rawRatevsElevationX[i] ) ) );
		}
		else if( i_rawRatevsElevationN[i] == 1. )
		{
			gRawRatevsElevationBinned->SetPoint( i, elevation, i_rawRatevsElevationX[i] );
			gRawRatevsElevationBinned->SetPointError( i, 0., 0. );
		}
		else
		{
			gRawRatevsElevationBinned->RemovePoint( i );
		}
	}

	// rate vs phase bins
	for( unsigned int j = 0; j < fNPhaseBins; j++ )
	{
		if( i_ratevsPhaseN[j] > 1. )
		{
			gRatevsPhaseBinned->SetPoint( j, ( fPhaseMin[j] + fPhaseMax[j] ) / 2., i_ratevsPhaseX[j] / i_ratevsPhaseN[j] );
			gRatevsPhaseBinned->SetPointError( j, 0., sqrt( 1. / i_ratevsPhaseN[j] / ( i_ratevsPhaseN[j] - 1 ) * ( i_ratevsPhaseXX[j] - 1. / i_ratevsPhaseN[j]*i_ratevsPhaseX[j]*i_ratevsPhaseX[j] ) ) );
		}
		else if( i_ratevsPhaseN[j] == 1. )
		{
			gRatevsPhaseBinned->SetPoint( j, ( fPhaseMin[j] + fPhaseMax[j] ) / 2., i_ratevsPhaseX[j] );
			gRatevsPhaseBinned->SetPointError( j, 0., i_ratevsPhaseE[j] );
		}
		else
		{
			gRatevsPhaseBinned->SetPoint( j, ( fPhaseMin[j] + fPhaseMax[j] ) / 2., -999 );

		}

		if( i_rawRatevsPhaseN[j] > 1. )
		{
			gRawRatevsPhaseBinned->SetPoint( j, ( fPhaseMin[j] + fPhaseMax[j] ) / 2., i_rawRatevsPhaseX[j] / i_rawRatevsPhaseN[j] );
			gRawRatevsPhaseBinned->SetPointError( j, 0., sqrt( 1. / i_rawRatevsPhaseN[j] / ( i_rawRatevsPhaseN[j] - 1 ) * ( i_rawRatevsPhaseXX[j] - 1. / i_rawRatevsPhaseN[j]*i_rawRatevsPhaseX[j]*i_rawRatevsPhaseX[j] ) ) );
		}
		else if( i_rawRatevsPhaseN[j] == 1. )
		{
			gRawRatevsPhaseBinned->SetPoint( j, ( fPhaseMin[j] + fPhaseMax[j] ) / 2., i_rawRatevsPhaseX[j] );
			gRawRatevsPhaseBinned->SetPointError( j, 0., 0. );
		}
		else
		{
			gRawRatevsPhaseBinned->SetPoint( j, ( fPhaseMin[j] + fPhaseMax[j] ) / 2., -999 );
		}
		if( i_elevationvsPhaseN[j] > 1. )
		{
			gElevationvsPhaseBinned->SetPoint( j, ( fPhaseMin[j] + fPhaseMax[j] ) / 2., i_elevationvsPhaseX[j] / i_elevationvsPhaseN[j] );
			gElevationvsPhaseBinned->SetPointError( j, 0., sqrt( 1. / i_elevationvsPhaseN[j] / ( i_elevationvsPhaseN[j] - 1 ) * ( i_elevationvsPhaseXX[j] - 1. / i_elevationvsPhaseN[j]*i_elevationvsPhaseX[j]*i_elevationvsPhaseX[j] ) ) );
		}
		else if( i_elevationvsPhaseN[j] == 1. )
		{
			gElevationvsPhaseBinned->SetPoint( j, ( fPhaseMin[j] + fPhaseMax[j] ) / 2., i_elevationvsPhaseX[j] );
			gElevationvsPhaseBinned->SetPointError( j, 0., 0. );
		}
		else
		{
			gElevationvsPhaseBinned->SetPoint( j, ( fPhaseMin[j] + fPhaseMax[j] ) / 2., -999 );
		}
	}

	// daily rates and significances
	fillDailyRates();
}


double VPlotRunSummary::getPhase( double iMJD )
{
	if( fNPhaseBins <= 0 || fPhaseOrbit == 0. )
	{
		return 0.;
	}

	return ( iMJD + 2400000.5 - fPhaseT0 ) / fPhaseOrbit - ( int )( ( iMJD + 2400000.5 - fPhaseT0 ) / fPhaseOrbit ) + fPhasePoff;
}


void VPlotRunSummary::fillDailyRates( bool iCorrectForDeadTimes )
{
	if( !c )
	{
		return;
	}

	fDayDate.clear();
	fDayMJD.clear();
	fDayPhase.clear();
	fDayDuration.clear();
	fDayNOn.clear();
	fDayNOff.clear();
	fDayAlpha.clear();
	fDayRate.clear();
	fDayRateE.clear();
	fDaySigni.clear();

	int iy, im, id, j;
	double fd;

	vector< double > iDate;
	bool iNewDay = false;
	for( unsigned int i = 0; i < c->fChain->GetEntries(); i++ )
	{
		c->GetEntry( i );
		if( c->runOn == -1 )
		{
			continue;
		}

		// get date from mjd
		VAstronometry::vlaDjcl( c->MJDOn, &iy, &im, &id, &fd, &j );
		iDate.push_back( iy * 10000 + im * 100 + id );
		if( iDate.size() == 1 )
		{
			iNewDay = true;
		}
		else if( iDate.size() > 1 && iDate[i - 1] != iDate.back() )
		{
			iNewDay = true;
		}
		else
		{
			iNewDay = false;
		}

		if( iNewDay )
		{
			// scale previous days
			if( fDayAlpha.size() > 0. )
			{
				if( fDayDuration.back() > 0. )
				{
					fDayAlpha.back() /= fDayDuration.back();
				}
			}
			// make new day
			fDayMJD.push_back( ( double )c->MJDOn );
			fDayDate.push_back( iDate.back() );
			fDayPhase.push_back( getPhase( c->MJDOn ) );
			fDayDuration.push_back( c->tOn / 60. );
			fDayNOn.push_back( c->NOn );
			fDayNOff.push_back( c->NOff );
			fDayAlpha.push_back( c->tOn / 60. * c->OffNorm );
		}
		else
		{
			fDayDuration.back() += c->tOn / 60.;
			if( !iCorrectForDeadTimes )
			{
				fDayNOn.back() += c->NOn;
				fDayNOff.back() += c->NOff;
			}
			else
			{
				if( c->DeadTimeFracOn > 0. )
				{
					fDayNOn.back() += c->NOn * ( 1. + c->DeadTimeFracOn );
				}
				if( c->DeadTimeFracOff > 0. )
				{
					fDayNOff.back() += c->NOff * ( 1. + c->DeadTimeFracOff );
				}
			}

			fDayAlpha.back() += c->tOn / 60. * c->OffNorm;
		}
	}
	// scale the last day
	if( fDayAlpha.size() > 0. )
	{
		if( fDayDuration.back() > 0. )
		{
			fDayAlpha.back() /= fDayDuration.back();
		}
	}

	// day wise rates and significances
	double ii_rate, ii_rateE, ii_rateSigni;
	for( unsigned int i = 0; i < fDayDate.size(); i++ )
	{
		if( fDayDuration[i] > 0. )
		{
			ii_rate = ( fDayNOn[i] - fDayNOff[i] * fDayAlpha[i] ) / fDayDuration[i];
			ii_rateSigni = VStatistics::calcSignificance( ( double )fDayNOn[i], ( double )fDayNOff[i], fDayAlpha[i], 17 );
			ii_rateE = sqrt( ( double )fDayNOn[i] + fDayAlpha[i] * fDayAlpha[i] * ( double )fDayNOff[i] ) / fDayDuration[i];
		}
		else
		{
			ii_rate = ii_rateE = ii_rateSigni = 0.;
		}
		if( TMath::Abs( ii_rate ) < 1.e-8 )
		{
			ii_rate = 0.;
		}

		fDayRate.push_back( ii_rate );
		fDayRateE.push_back( ii_rateE );
		fDaySigni.push_back( ii_rateSigni );
	}

	// fill graphs
	if( gRatevsPhaseDaily && gSignificancevsPhaseDaily )
	{
		gRatesvsTimeDaily->Set( ( int )fDayRate.size() );
		gRatevsPhaseDaily->Set( ( int )fDayRate.size() );
		gSignificancevsPhaseDaily->Set( ( int )fDaySigni.size() );

		for( unsigned int i = 0; i < fDayRate.size(); i++ )
		{
			gRatesvsTimeDaily->SetPoint( i, fDayMJD[i], fDayRate[i] );
			gRatesvsTimeDaily->SetPointError( i, 0., fDayRateE[i] );
			gRatevsPhaseDaily->SetPoint( i, fDayPhase[i], fDayRate[i] );
			gRatevsPhaseDaily->SetPointError( i, 0., fDayRateE[i] );

			gSignificancevsPhaseDaily->SetPoint( i, fDayPhase[i], fDaySigni[i] );
		}
	}

}


void VPlotRunSummary::writeRunTable()
{
	if( !c )
	{
		return;
	}

	for( unsigned int i = 0; i < c->fChain->GetEntries(); i++ )
	{
		c->GetEntry( i );

		if( c->elevationOn < fMinElevation || c->elevationOn > fMaxElevation )
		{
			continue;
		}

		// loop over list of runs to be excluded
		bool bBreak = false;
		for( unsigned int r = 0; r < fListofExcludedRuns.size(); r++ )
		{
			if( fListofExcludedRuns[r] == c->runOn )
			{
				bBreak = true;
			}
		}
		if( bBreak )
		{
			continue;
		}

		if( c->runOn == -1 )
		{
			cout << endl << " ALL RUNS (" << c->fChain->GetEntries() - 1 << ")";
		}
		else
		{
			cout << " " << c->runOn;
		}
		if( c->elevationOn > 0.5 )
		{
			cout << " " << setw( 2 ) << ( int )( c->elevationOn + 0.5 ) << " deg";
		}
		else
		{
			cout << " " << setw( 2 ) << ( int )( c->elevationOff + 0.5 ) << " deg";
		}
		cout << setw( 3 ) << ( int )( c->tOn / 60. + 0.5 ) << " min";
		cout << " NOn:" << setw( 4 ) << ( int )( c->NOn + 0.5 );
		cout << " NOff:" << setw( 4 ) << setprecision( 1 ) << c->NOffNorm;
		cout << " (" << setw( 4 ) << ( int )c->NOff;
		cout << ", norm=" << setprecision( 2 ) << fixed << c->OffNorm << ")";
		if( c->runOn == -1 )
		{
			cout << endl;
		}
		cout << setw( 5 ) << setprecision( 1 ) << showpos << c->Signi << " sigma,";
		cout << " Rates: " << setw( 5 ) << setprecision( 2 ) << c->Rate << " +- " << noshowpos << c->RateE << " g/min";
		cout << " (bck: " << c->RateOff << " 1/min)";
		cout << endl;
	}
}


/*!
    writes tables with results in latex format (longtable)
*/
void VPlotRunSummary::writeRunTable( string ioutfile, bool iPhases, bool iSignificancePerHour, bool iCorrectForDeadTimes )
{
	if( !c )
	{
		return;
	}

	ofstream is;
	is.open( ioutfile.c_str() );
	if( !is )
	{
		cout << "error opening output file " << endl;
		return;
	}
	cout << "writing run table to " << ioutfile << endl;

	vector< double > ifirstRun;                   // this is true if this is the first run of the day
	vector< double > iDate;
	vector< int > iMJD;
	int iy, im, id, j;
	double fd;
	vector< double > iPhase;

	for( unsigned int i = 0; i < c->fChain->GetEntries(); i++ )
	{
		c->GetEntry( i );
		if( c->runOn == -1 )
		{
			continue;
		}

		// get date from mjd
		VAstronometry::vlaDjcl( c->MJDOn, &iy, &im, &id, &fd, &j );
		iDate.push_back( iy * 10000 + im * 100 + id );
		iMJD.push_back( ( int )c->MJDOn );
		if( iDate.size() == 1 )
		{
			ifirstRun.push_back( 1 );
		}
		else if( iDate.size() > 1 && iDate[i - 1] != iDate.back() )
		{
			ifirstRun.push_back( 1 );
		}
		else
		{
			ifirstRun.push_back( 0 );
		}

		iPhase.push_back( getPhase( c->MJDOn ) );
	}
	fillDailyRates( iCorrectForDeadTimes );

	is << "\\documentclass[a4paper]{article}" << endl;
	is << "\\usepackage{longtable}" << endl;
	is << "\\usepackage{lscape}" << endl;

	is << "\\begin{document}" << endl;

	is << "%% result table produced from file " << fIn->GetName() << endl;
	is << endl;
	is << "\\begin{footnotesize}" << endl;
	is << "\\begin{landscape}" << endl;
	if( iPhases )
	{
		is << "\\begin{longtable}{c|c|c|c|c|c|c|c|c|c|c|c|c|c}" << endl;
		//      is << "run & date & MJD &  orbital & run & ele- & wobble & $N_{\\mathrm{On}}$ &   $N_{\\mathrm{Off}}$ & $\\alpha$ & rate & signi- & daily rate & daily \\\\" << endl;
		//      is << "    &      & &  phase   & duration & vation & offset (N,W) & & ($N_{\\mathrm{Off}}*\\alpha$)   & & [$\\gamma$/min] & ficance & [$\\gamma$/min] & sign. \\\\" << endl;
		is << "run & date & orbital & run & ele- & wobble & dead & $N_{\\mathrm{On}}$ &   $N_{\\mathrm{Off}}$ & $\\alpha$ & rate & signi- & daily rate & daily \\\\" << endl;
		is << "    &      & phase   & duration & vation & offset (N,W) & time (\\%) &  & ($N_{\\mathrm{Off}}*\\alpha$)   & & [$\\gamma$/min] & ficance & [$\\gamma$/min] & sign. \\\\" << endl;
	}
	else
	{
		is << "\\begin{longtable}{c|c|c|c|c|c|c|c|c|c|c|c|c}" << endl;
		is << "run & date & run & ele- & wobble & dead & $N_{On}$ & $N_{Off}$ & $\\alpha$ & rate & signi- & daily rate & daily \\\\" << endl;
		is << "    &      & duration & vation & offset (N,W) & time (\\%) &         & ($N_{Off}*\\alpha$) & & [$\\gamma$/min] & ficance   & [$\\gamma$/min] & sign. \\\\" << endl;
		if( iSignificancePerHour )
		{
			is << " & & & & & & & & & & & & $[\\sigma/\\sqrt{hour}]$ \\\\" << endl;
		}
	}
	is << "\\hline" << endl;
	is << "\\hline" << endl;
	is << "\\endhead" << endl;
	for( unsigned int i = 0; i < c->fChain->GetEntries(); i++ )
	{
		c->GetEntry( i );

		if( c->runOn == -1 )
		{
			continue;
		}

		if( ifirstRun[i] == 1 )
		{
			is << "\\hline" << endl;
		}

		is << c->runOn << " & ";
		is << ( int )iDate[i] << " & ";
		//       is << iMJD[i] << " & ";
		cout << "now at date " << ( int )iDate[i] << endl;
		if( iPhases )
		{
			is << setprecision( 2 ) << iPhase[i] << " & ";
		}
		is << ( int )( c->tOn / 60. + 0.5 ) << " min & ";
		if( c->elevationOn > 2. )
		{
			is << ( int )( c->elevationOn + 0.5 ) << "$^\\mathrm{o}$ & ";
		}
		else
		{
			is << ( int )( c->elevationOff + 0.5 ) << "$^\\mathrm{o}$ & ";
		}
		is << "(";
		if( fabs( c->WobbleNorth ) < 1.e-5 )
		{
			is << 0;
		}
		else
		{
			is << setprecision( 1 ) << c->WobbleNorth;
		}
		is << "$^\\mathrm{o}$, ";
		if( fabs( c->WobbleWest ) < 1.e-5 )
		{
			is << 0;
		}
		else
		{
			is << setprecision( 1 ) << c->WobbleWest;
		}
		is << "$^\\mathrm{o}$";
		is << ") &";
		is << setprecision( 2 ) <<  c->DeadTimeFracOn * 100.;
		is << " & ";
		is << ( int )( c->NOn ) << " & ";
		is << ( int )( c->NOff ) << " (" << ( int )c->NOffNorm << ") & ";
		is << setprecision( 2 ) << c->OffNorm << " & ";
		if( iCorrectForDeadTimes && c->DeadTimeFracOn > 0. )
		{
			is << setprecision( 2 ) << c->Rate*( 1. + c->DeadTimeFracOn ) << "$\\pm$" << setprecision( 2 ) << c->RateE*( 1. + c->DeadTimeFracOn ) << " & ";
		}
		else
		{
			is << setprecision( 2 ) << c->Rate << "$\\pm$" << setprecision( 2 ) << c->RateE << " & ";
		}
		is << setprecision( 2 ) << c->Signi << " & ";
		cout << "\t" << ifirstRun[i] << "\t" << c->runOn << endl;
		if( ifirstRun[i] == 1 )
		{
			for( unsigned t = 0; t < fDayDate.size(); t++ )
			{
				if( fDayDate[t] == iDate[i] )
				{
					is << fDayRate[t] << "$\\pm$" << fDayRateE[t] << " & ";
					// mean Crab rates for September 2006 (2-tel)
					/*					if( c->runOn < 31600 )
										{
											ig = 3.9;
										}
										// mean Crab rates for October 2006 (2-tel)
										else if( c->runOn < 32100 )
										{
											ig = 4.4;
										}
										// mean Crab rates for November 2006 (2-tel)
										else if( c->runOn < 32600 )
										{
											ig = 4.1;
										}
										// mean Crab rates for January 2007 (3-tel)
										else if( c->runOn < 33500 )
										{
											ig = 7.4;
										}
										// mean Crab rates for February 2007 (3-tel)
										else
										{
											ig = 7.7;
										}
										//		  is << fDayRate[t]/ig << "$\\pm$";
										// don't print daily rate in Crab units
										//		  is << fDayRateE[t]/ig << " & "; */
					if( !iSignificancePerHour )
					{
						is << fDaySigni[t];
					}
					else
					{
						if( fDayDuration[t] > 0. )
						{
							is << fDaySigni[t] / sqrt( fDayDuration[t] / 60. );
						}
						else
						{
							is << 0 << endl;
						}
					}
					is << " \\\\" << endl;
					cout << "\t rate: " << fDayRate[t] << "+-" << fDayRateE[t]  << " (" << fDaySigni[t] << ")" << endl;
					break;
				}
			}
		}
		else
		{
			is << " & ";
			is << " \\\\" << endl;
		}
	}
	is << "\\end{longtable}" << endl;
	is << "\\end{landscape}" << endl;
	is << "\\end{footnotesize}" << endl;
	is << endl;
	is << "\\end{document}" << endl;
	is.close();
}


TCanvas* VPlotRunSummary::plot_dailyRates( bool iDeadTimeCorrection )
{
	if( bIsZombie )
	{
		return 0;
	}
	fillDailyRates( iDeadTimeCorrection );

	char hname[200];
	sprintf( hname, "cR_%d", gRandom->Integer( 10000 ) );
	TCanvas* cR = new TCanvas( hname, "daily rates", 10, 10, 400, 400 );
	cR->SetGridx( 0 );
	cR->SetGridy( 0 );
	cR->Draw();

	gRatesvsTimeDaily->Draw( "ap" );
	gRatesvsTimeDaily->GetHistogram()->GetXaxis()->SetTitle( "MJD" );
	gRatesvsTimeDaily->GetHistogram()->GetYaxis()->SetTitle( "#gamma-ray rate [1/min]" );
	gRatesvsTimeDaily->GetHistogram()->GetXaxis()->SetNdivisions( 505 );
	if( gRatesvsTimeDaily->GetHistogram()->GetMinimum() < 0. && gRatesvsTimeDaily->GetHistogram()->GetMaximum() > 0. )
	{
		TLine* iL = new TLine( gRatesvsTimeDaily->GetHistogram()->GetXaxis()->GetXmin(), 0., gRatesvsTimeDaily->GetHistogram()->GetXaxis()->GetXmax(), 0. );
		iL->SetLineStyle( 2 );
		iL->Draw();
	}

	return cR;
}


TCanvas* VPlotRunSummary::plot_rates( bool iOff )
{
	if( bIsZombie )
	{
		return 0;
	}

	fill();

	TCanvas* cRR = new TCanvas( "cRR", "rates", 10, 10, 400, 400 );
	cRR->SetGridx( 0 );
	cRR->SetGridy( 0 );
	cRR->Draw();

	TGraphErrors* g = 0;
	if( iOff )
	{
		g = gRateOffvsTime;
	}
	else
	{
		g = gRatevsTime;
	}

	if( !g )
	{
		return 0;
	}

	g->Draw( "ap" );
	g->GetHistogram()->SetXTitle( "MJD" );
	if( iOff )
	{
		g->GetHistogram()->SetYTitle( "background rate [1/min]" );
	}
	else
	{
		g->GetHistogram()->SetYTitle( "#gamma-ray rate [1/min]" );
	}
	g->GetHistogram()->GetXaxis()->SetNdivisions( 505 );

	// draw zero line
	if( g->GetHistogram()->GetMinimum() < 0 )
	{
		TLine* iL = new TLine( g->GetHistogram()->GetXaxis()->GetXmin(), 0., g->GetHistogram()->GetXaxis()->GetXmax(), 0. );
		iL->SetLineStyle( 2 );
		iL->Draw();
	}

	return cRR;
}


TCanvas* VPlotRunSummary::plot_ratevsElevation( bool iOff )
{
	if( bIsZombie )
	{
		return 0;
	}

	fill();

	TCanvas* cRatevsElevation = new TCanvas( "cRatevsElevation", "Rate vs elevation", 10, 10, 400, 400 );
	cRatevsElevation->SetGridx( 0 );
	cRatevsElevation->SetGridy( 0 );
	cRatevsElevation->Draw();

	TGraphErrors* g = 0;
	if( !iOff )
	{
		g = gRatevsElevation;
	}
	else
	{
		g = gRateOffvsElevation;
	}
	if( !g )
	{
		return 0;
	}

	g->Draw( "ap" );
	g->GetHistogram()->SetXTitle( "elevation [deg]" );
	if( iOff )
	{
		g->GetHistogram()->SetYTitle( "background rate [1/min]" );
	}
	else
	{
		g->GetHistogram()->SetYTitle( "#gamma-ray rate [1/min]" );
	}

	// draw zero line
	if( g->GetHistogram()->GetMinimum() < 0 )
	{
		TLine* iL = new TLine( g->GetHistogram()->GetXaxis()->GetXmin(), 0., g->GetHistogram()->GetXaxis()->GetXmax(), 0. );
		iL->SetLineStyle( 2 );
		iL->Draw();
	}

	if( !iOff )
	{
		TCanvas* cRatevsElevationBinned = new TCanvas( "cRatevsElevationBinned", "Rate vs elevation", 410, 10, 400, 400 );
		cRatevsElevationBinned->SetGridx( 0 );
		cRatevsElevationBinned->SetGridy( 0 );
		cRatevsElevationBinned->Draw();

		gRatevsElevationBinned->Draw( "ap" );
		gRatevsElevationBinned->GetHistogram()->SetXTitle( "elevation [deg]" );
		gRatevsElevationBinned->GetHistogram()->SetYTitle( "#gamma-ray rate [1/min]" );
	}
	return cRatevsElevation;
}


/*
 * plot cumulative significance vs time
 *
 */
TCanvas* VPlotRunSummary::plot_cumSignificance()
{
	if( bIsZombie )
	{
		return 0;
	}

	fill();

	char hname[600];
	sprintf( hname, "cCS_%d", gRandom->Integer( 10000000 ) );
	TCanvas* cRR = new TCanvas( hname, "cumulative significance vs time", 10, 10, 400, 400 );
	cRR->SetGridx( 0 );
	cRR->SetGridy( 0 );
	cRR->Draw();

	gCumSignificancevsTime->SetLineWidth( ( Width_t )3. );
	gCumSignificancevsTime->SetMarkerStyle( 2 );
	gCumSignificancevsTime->SetMarkerSize( 1. );
	gCumSignificancevsTime->Draw( "apl" );
	gCumSignificancevsTime->GetHistogram()->SetXTitle( "time [min]" );
	gCumSignificancevsTime->GetHistogram()->SetYTitle( "cumulative significance" );

	// plot zero line
	if( gCumSignificancevsTime && gCumSignificancevsTime->GetHistogram() && gCumSignificancevsTime->GetHistogram()->GetMinimum() < 0. )
	{
		TLine* iL = new TLine( gCumSignificancevsTime->GetHistogram()->GetXaxis()->GetXmin(), 0., gCumSignificancevsTime->GetHistogram()->GetXaxis()->GetXmax(), 0. );
		iL->SetLineStyle( 2 );
		iL->Draw();
	}

	return cRR;
}


TCanvas* VPlotRunSummary::plot_ratevsWobbleDirection( TCanvas* c, bool bPlotBackground, bool bPlotIndividualRuns )
{
	if( bIsZombie )
	{
		return 0;
	}

	fill();

	// rates and significance vs wobble direction

	if( c == 0 )
	{
		c = new TCanvas( "cRatevsWobbleDirection", "cRatevsWobbleDirection", 10, 10, 400, 400 );
		c->Draw();

		gRatevsWobbleDirection->GetXaxis()->SetLimits( 0., 2. );
		gRatevsWobbleDirection->Draw( "ap" );
	}

	if( bPlotIndividualRuns )
	{
		gRatevsWobbleOffset->SetMarkerStyle( 25 );
		gRatevsWobbleOffset->Draw( "p" );
		gRateOffvsWobbleOffset->SetMarkerSize( 1. );
		gRateOffvsWobbleOffset->SetMarkerStyle( 25 );
		gRateOffvsWobbleOffset->Draw( "p" );
	}
	if( bPlotBackground )
	{
		gRateOffvsWobbleDirection->SetMarkerStyle( 21 );
		gRateOffvsWobbleDirection->SetMarkerSize( 1 );
		gRateOffvsWobbleDirection->SetLineStyle( 2 );
		gRateOffvsWobbleDirection->Draw( "cp" );
	}
	gRatevsWobbleDirection->Draw( "p" );
	gRatevsWobbleDirection->GetHistogram()->SetAxisRange( 0., 2. );
	gRatevsWobbleDirection->GetHistogram()->SetXTitle( "wobble direction [deg]" );
	gRatevsWobbleDirection->GetHistogram()->SetYTitle( "mean rate [#gamma/min]" );

	TCanvas* cSignificancevsWobbleDirection = new TCanvas( "cSignificancevsWobbleDirection", "cSignificancevsWobbleDirection", 410, 10, 400, 400 );
	cSignificancevsWobbleDirection->Draw();

	gSignificancevsWobbleDirection->GetXaxis()->SetLimits( 0., 2. );
	gSignificancevsWobbleDirection->Draw( "ap" );
	gSignificancevsWobbleDirection->GetHistogram()->SetAxisRange( 0., 2. );
	gSignificancevsWobbleDirection->GetHistogram()->SetXTitle( "wobble direction [deg]" );
	gSignificancevsWobbleDirection->GetHistogram()->SetYTitle( "mean significance" );
	gSignificancevsWobbleDirection->GetYaxis()->SetTitleOffset( 1.3 );

	TCanvas* cElevationvsWobbleDirection = new TCanvas( "cElevationvsWobbleDirection", "cElevationvsWobbleDirection", 820, 10, 400, 400 );
	cElevationvsWobbleDirection->Draw();

	gElevationvsWobbleDirection->GetXaxis()->SetLimits( 0., 2. );
	gElevationvsWobbleDirection->Draw( "ap" );
	gElevationvsWobbleDirection->GetHistogram()->SetAxisRange( 0., 2. );
	gElevationvsWobbleDirection->GetHistogram()->SetXTitle( "wobble direction [deg]" );
	gElevationvsWobbleDirection->GetHistogram()->SetYTitle( "mean Elevation" );
	gElevationvsWobbleDirection->GetYaxis()->SetTitleOffset( 1.3 );

	return c;
}


/*
   set list of excluded runs

   runs separated by spaces, example: setListofExcludedRuns( "1234 2345 2352" );

   to clear the list: setListofExcludedRuns("" );

*/
void VPlotRunSummary::setListofExcludedRuns( string iList )
{
	if( iList.size() == 0 )
	{
		fListofExcludedRuns.clear();
	}

	string itemp;
	istringstream is_stream( iList );
	while( !( is_stream >> std::ws ).eof() )
	{
		is_stream >> itemp;
		fListofExcludedRuns.push_back( ( int )atoi( itemp.c_str() ) );
	}
	cout << "list of excluded runs (total " << fListofExcludedRuns.size() << " runs)" << endl;
	for( unsigned int i = 0; i < fListofExcludedRuns.size(); i++ )
	{
		cout << fListofExcludedRuns[i] << " ";
	}
	cout << endl;
}


TCanvas* VPlotRunSummary::plot_stats()
{
	if( bIsZombie )
	{
		return 0;
	}

	fill();

	TCanvas* cPed = new TCanvas( "cRunSummaryStatsPeds", "pedestal variations", 10, 10, 400, 400 );
	cPed->Draw();

	if( hPedvars )
	{
		hPedvars->Draw();
	}

	TCanvas* cAz = new TCanvas( "cRunSummaryStatsAz", "azimuth (absolute value)", 450, 10, 400, 400 );
	cAz->Draw();

	if( hAbsAzimuth )
	{
		hAbsAzimuth->Draw();
	}

	TCanvas* cElevation = new TCanvas( "cRunSummaryStatsElevation", "elevation", 850, 10, 400, 400 );
	cElevation->Draw();

	if( hElevation )
	{
		hElevation->Draw();
	}

	return cPed;
}
