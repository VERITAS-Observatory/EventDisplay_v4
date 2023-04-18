/*! \class IPRCalculator
 *     \brief calculation and save IPR GRAPH
 *
 *     */

#include "VIPRCalculator.h"

VIPRCalculator::VIPRCalculator()
{
	fIPRTimeSlices = true;
	fIPRAverageTel = false;
	fPedPerTelescopeTypeMinCnt = 1.E5;  // minimal counter for IPR measurements
}

void VIPRCalculator::initialize()
{
	if( fDebug )
	{
		cout << "VIPRCalculator::initialize()" << endl;
	}
	
	// set the data readers
	initializeDataReader();
	if( !initializeDataReader() )
	{
		cout << "VIPRCalculator::initialize, error: cannot initialize data readers" << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	
	
}

void VIPRCalculator::definePedestalFile( std::vector<std::string> fPedFileNameC )
{
	fPedFile = fPedFileNameC;
}


TFile* VIPRCalculator::initializePedestalFile( int i_tel )
{
	string iPedFileName = fPedFile[getTeltoAna()[i_tel]].c_str();
	if( iPedFileName.find( ".root" ) == string::npos )
	{
		iPedFileName += ".root";
	}
	// open pedestal files
	TFile* iPedFile = new TFile( iPedFileName.c_str() );
	if( iPedFile->IsZombie() )
	{
		cout << "VIPRCalculator::initializePedestalFile error reading IPR graphs from ";
		cout << iPedFileName << endl;
		return nullptr;
	}
	// fpedcal_histo_storagegrams with IPR distributions are either
	// i) in a directory called distributions_TelType
	// ii) in the current directory
	cout << "Telescope type " << getTelType( getTeltoAna()[i_tel] ) << ": ";
	cout << "reading IPR histograms for summation window " << getSumWindow();
	cout << " from ";
	cout << iPedFileName << endl;
	return iPedFile;
	
}

float VIPRCalculator::getTsearch()
{
	float Tsearch = getDetectorGeometry()->getLengthOfSampleTimeSlice( getTelID() );
	if( getSearchWindowLast() < getNSamples() )
	{
		Tsearch *= ( getSearchWindowLast() - getSumFirst() ); // [ns]
	}
	else
	{
		Tsearch *= ( getNSamples() - getSumFirst() ); // [ns]
	}
	return Tsearch;
}

float VIPRCalculator::convertRate( unsigned int i_tel )
{
	// convert to Rate
	float nsToSec = 1E-9;
	float convToHz = 1.;
	if( nsToSec > 0. && getTsearch() > 0. )
	{
		convToHz /= ( nsToSec * getTsearch() );
	}
	else if( getRunParameter()->fImageCleaningParameters[i_tel]->fNNOpt_ifExplicitSampleTimeSlice
			 && getRunParameter()->fImageCleaningParameters[i_tel]->fNNOpt_sampleTimeSlice > 0
			 && getRunParameter()->fImageCleaningParameters[i_tel]->fNNOpt_nBinsADC > 0 )
	{
		// simple peak sensing: sim_telarray uses the maximum bin only
		// The values for sampleTimeSlice and nBinsADC are set in the cleaning parameter file
		// For example, for the currect (Apr 17) ASTRI simulation, it is set in sim_telarray as
		// fadc_mhz = 500 % MHz ==> sampleTimeSlice = 2 ns
		// fadc_sum_bins = nBinsADC = 25 % Number of ADC time intervals actually summed up.
		convToHz /= ( nsToSec
					  * getRunParameter()->fImageCleaningParameters[i_tel]->fNNOpt_sampleTimeSlice
					  * getRunParameter()->fImageCleaningParameters[i_tel]->fNNOpt_nBinsADC );
	}
	return convToHz;
	
}


/*
 HANDLING OF IPR FOR NO TIME SLICE CASE

*/

TH1F* VIPRCalculator::initializeIPRHistogram( unsigned int iSummationWindow, unsigned int i_tel )
{
	int iTelType = getTelType( getTeltoAna()[i_tel] );
	//get reference hpedPerTelescopeType for channel 1
	cout << "VIPRCalculator::initializeIPRHistogram initializing IPR graph for calculation of average of telescopes." << endl;
	TH1F* hIPR = 0;
	
	TFile* iPedFile = ( TFile* )initializePedestalFile( i_tel );
	if( iPedFile == nullptr )
	{
		return hIPR;
	}
	
	// fpedcal_histo_storagegrams with IPR distributions are either
	// i) in a directory called distributions_TelType
	// ii) in the current directory
	//cout << "VIPRCalculator::initializeIPRHistogram Telescope type:" << iTelType << endl;
	//cout << "reading IPR histograms for summation window " << iSummationWindow;
	
	stringstream Directory( stringstream::in | stringstream::out );
	Directory << "distributions_" << iTelType;
	if( !iPedFile->Get( Directory.str().c_str() ) )
	{
		return hIPR;
	}
	
	// get charge distribution for first channel as reference fpedcal_histo_storagegram
	stringstream hIPRname( stringstream::in | stringstream::out );
	hIPRname << "distributions_" << iTelType << "/hpedPerTelescopeType_" << iTelType << "_" << iSummationWindow << "_" << 1;
	
	//?
	TH1F* href = ( TH1F* )gDirectory->Get( hIPRname.str().c_str() );
	iPedFile->Close();
	
	if( !href )
	{
		cout << " Error: cannot find IPR histogram" << hIPRname.str().c_str();
		return hIPR;
	}
	//summary fpedcal_histo_storagegram
	if( getRunParameter()->fIgnoreDSTGains )
	{
		// work in dc
		hIPR = new TH1F( "", "", int( 1.5 * href->GetNbinsX() + 0.5 ), href->GetXaxis()->GetXmin(), href->GetXaxis()->GetXmax() );
		cout << "Error: " <<  href->GetNbinsX() << "  " << href->GetXaxis()->GetXmin() << "  " << href->GetXaxis()->GetXmax() << endl;
		return hIPR;
	}
	else
	{
		// work in pe
		hIPR = new TH1F( "", "", 1000, 0., 100. );
		return hIPR;
	}
}

TH1F* VIPRCalculator::FillIPRHistogram( unsigned int iSummationWindow, unsigned int i_tel )
{
	int iTelType = getTelType( getTeltoAna()[i_tel] );
	TH1F* hIPR = ( TH1F* )initializeIPRHistogram( iSummationWindow, i_tel );
	// check suffix of ped file
	TFile* iPedFile = ( TFile* )initializePedestalFile( i_tel );
	
	stringstream i_Directory( stringstream::in | stringstream::out );
	i_Directory << "distributions_" << iTelType;
	if( iPedFile->Get( i_Directory.str().c_str() ) )
	{
		iPedFile->cd( i_Directory.str().c_str() );
	}
	
	float i_gainCorrect = 1.;
	//hIPR->Reset();
	//        //float i_gainCorrect = 1.;
	for( unsigned int i = 0; i < getNChannels(); i++ )
	{
		if( getDetectorGeometry()->getAnaPixel()[i] > 0
				&& i < getDead().size() && !getDead()[i] )
		{
			stringstream i_Hname( stringstream::in | stringstream::out );
			i_Hname << "hpedPerTelescopeType_" << iTelType << "_" << iSummationWindow << "_" << i;
			TH1F* h = ( TH1F* )gDirectory->Get( i_Hname.str().c_str() );
			if( h )
			{
				float ped = 0;
				// default: pedestals are subtracted here
				// (for combined channel analysis: charges are filled already pedestal subtracted)
				// apply relative gain correction to integrated charges
				if( getRunParameter()->fCombineChannelsForPedestalCalculation == 0 )
				{
					ped = getPeds()[i];
				}
				for( int j = 1; j <= h->GetNbinsX(); j++ )
				{
					if( h->GetBinContent( j ) > 0. && getGains()[i] > 0 )
					{
						i_gainCorrect = getGains()[i];
						if( getHIGHQE_gainfactor( i ) > 0. )
						{
							i_gainCorrect *= getHIGHQE_gainfactor( i );
						}
						if( i_gainCorrect > 0. )
						{
							hIPR->Fill( ( h->GetBinCenter( j ) - ped * iSummationWindow ) / i_gainCorrect, h->GetBinContent( j ) );
						}
					}
				}
			}
		}
	}
	iPedFile->Close();
	return hIPR;
}

/*
IN CASE IPR IS GIVEN AS AVERAGE OF TELESCOPES, FOR TEL > 0 COPY IPR CALCULATED FOR TEL = 0.
*/

bool VIPRCalculator::copyIPRInitialized( unsigned int iSummationWindow, unsigned int i_tel )
{
	int iTelType = getTelType( getTeltoAna()[i_tel] );
	TGraphErrors* i_IPRGraph = getIPRGraph( iSummationWindow, true );
	
	setTelID( getTeltoAna()[0] );
	TGraphErrors* i_IPRGraph_Tel0 = getIPRGraph( iSummationWindow, true );
	
	setTelID( getTeltoAna()[i_tel] );
	
	if( !i_IPRGraph )
	{
		cout << "VIPRCalculator::copyIPRInitialized: no IPR graph found for telescope type " << iTelType << endl;
		return false;
	}
	if( !i_IPRGraph_Tel0 )
	{
		cout << "VIPRCalculator::copyIPRInitialized: no IPR graph found for telescope type " << getTeltoAna()[0] << endl;
		return false;
	}
	
	for( Int_t i = 0; i < i_IPRGraph_Tel0->GetN() ; i++ )
	{
		i_IPRGraph->SetPoint( i, i_IPRGraph_Tel0->GetPointX( i ), i_IPRGraph_Tel0->GetPointY( i ) );
	}
	
	i_IPRGraph->SetMinimum( 1 );
	i_IPRGraph->SetTitle( i_IPRGraph_Tel0->GetTitle() );
	i_IPRGraph->GetXaxis()->SetTitle( i_IPRGraph_Tel0->GetXaxis()->GetTitle() );
	i_IPRGraph->GetYaxis()->SetTitle( i_IPRGraph_Tel0->GetYaxis()->GetTitle() );
	i_IPRGraph->SetName( Form( "IPRcharge_TelType%d_SW%d", iTelType, iSummationWindow ) );
	
	return true;
	
}

TH1F* VIPRCalculator::calculateIPRGraphAveragedNoTimeSlices( unsigned int iSummationWindow, unsigned int i_tel )
{

	TH1F* hIPR = ( TH1F* )initializeIPRHistogram( iSummationWindow , 0 );
	int iTelType = getTelType( getTeltoAna()[i_tel] );
	
	for( unsigned int teltype = 0; teltype < getTeltoAna().size(); teltype++ )
	{
		setTelID( getTeltoAna()[teltype] );
		
		// first find dead channels
		// (ignore low gains here)
		findDeadChans( false );
		// calculate IPR graphs
		TFile* iPedFile = ( TFile* )initializePedestalFile( teltype );
		if( iPedFile == nullptr )
		{
			cout << "VIPRCalculator::calculateIPRGraphAveragedNoTimeSlices error reading IPR graphs from pedestal file ";
			TH1F* hNull = 0;
			return hNull;
		}
		
		// fpedcal_histo_storagegrams with IPR distributions are either
		// i) in a directory called distributions_TelType
		// ii) in the current directory
		cout << "VIPRCalculator::calculateIPRGraphAveragedNoTimeSlices Telescope type " << iTelType << ": ";
		stringstream Directory( stringstream::in | stringstream::out );
		Directory.str( std::string() );
		Directory << "distributions_" << iTelType;
		if( iPedFile->Get( Directory.str().c_str() ) )
		{
			iPedFile->cd( Directory.str().c_str() );
		}
		else
		{
			cout << "VIPRCalculator::calculateIPRGraphAveragedNoTimeSlices no directory: " << Directory.str().c_str() << endl;
		}
		
		///////////////////////////
		// average over all channels in one telescope
		float i_gainCorrect = 1.;
		for( unsigned int i = 0; i < getNChannels(); i++ )
		{
			if( getDetectorGeometry()->getAnaPixel()[i] > 0
					&& i < getDead().size() && !getDead()[i] )
			{
				stringstream HistName( stringstream::in | stringstream::out );
				HistName << "hpedPerTelescopeType_" << iTelType << "_" << iSummationWindow << "_" << i;
				TH1F* h = ( TH1F* )gDirectory->Get( HistName.str().c_str() );
				
				
				if( h )
				{
					float ped = 0;
					// default: pedestals are subtracted here
					// (for combined channel analysis: charges are filled already pedestal subtracted)
					// apply relative gain correction to integrated charges
					if( getRunParameter()->fCombineChannelsForPedestalCalculation == 0 )
					{
						ped = getPeds()[i];
					}
					// special treatment for ASTRI telescopes
					else if( getRunParameter()->fCombineChannelsForPedestalCalculation == 2 )
					{
						stringstream Pname( stringstream::in | stringstream::out );
						Pname << "hped_" << iTelType << "_" << iSummationWindow << "_" << i;
						TH1F* hP = ( TH1F* )gDirectory->Get( Pname.str().c_str() );
						if( hP )
						{
							ped = hP->GetMean();
						}
					}
					for( int j = 1; j <= h->GetNbinsX(); j++ )
					{
						if( h->GetBinContent( j ) > 0. && getGains()[i] > 0 )
						{
							if( i == 0 )
							{
								cout << "gain : " << getGains()[i] << endl;
							}
							i_gainCorrect = getGains()[i];
							if( getHIGHQE_gainfactor( i ) > 0. )
							{
								i_gainCorrect *= getHIGHQE_gainfactor( i );
							}
							if( i_gainCorrect > 0. )
							{
								hIPR->Fill( ( h->GetBinCenter( j ) - ped * iSummationWindow ) / i_gainCorrect, h->GetBinContent( j ) );
							}
						}
					}
				}
			}
		}
		iPedFile->Close();
		
	}
	hIPR->Scale( 1. / getTeltoAna().size() );
	float norm = getTeltoAna().size() * hIPR->Integral( 1, hIPR->GetNbinsX() );
	cout << "VIPRCalculator::calculateIPRGraphAveragedNoTimeSlices normalization of average IPR fpedcal_histo_storagegram " << norm;
	if( norm < fPedPerTelescopeTypeMinCnt )  //statistical limit for number of counts
	{
		cout << "VIPRCalculator::calculateIPRGraphAveragedNoTimeSlices WARNING: there is NOT enough statistics ";
		cout << " ( < " << fPedPerTelescopeTypeMinCnt << ") even when averaging over all telescopes." << endl;
		return hIPR;
	}
	else
	{
		cout << "VIPRCalculator::calculateIPRGraphsAveragedNoTimeSlices: there is enough statistics to average over telescopes. " << endl;
		return hIPR;
	}
}

TGraphErrors* VIPRCalculator::updateIPRGraph( TH1F* hIPR, unsigned int i_tel, int iSummationWindow )
{

	TGraphErrors* i_IPRGraph = new TGraphErrors( 1 );
	i_IPRGraph->SetTitle( "" );
	
	int z = 0;
	float norm = hIPR->Integral( 1, hIPR->GetNbinsX() );
	for( int i = 1; i <= hIPR->GetNbinsX(); i++ )
	{
		if( hIPR->GetBinContent( i ) > 5 )
		{
			double val = convertRate( i_tel ) * hIPR->Integral( i, hIPR->GetNbinsX() ) / norm;
			double valerr = convertRate( i_tel ) * sqrt( hIPR->Integral( i, hIPR->GetNbinsX() ) ) / norm;
			double charge_pe = hIPR->GetXaxis()->GetBinCenter( i ) * getTelescopeAverageFADCtoPhe();
			double charge_pe_bin_width = 0.5 * hIPR->GetXaxis()->GetBinWidth( i ) * getTelescopeAverageFADCtoPhe();
			
			i_IPRGraph->SetPoint( z, charge_pe, val );
			i_IPRGraph->SetPointError( z, charge_pe_bin_width, valerr );
			z++;
		}
	}
	i_IPRGraph->SetMinimum( 1 );
	i_IPRGraph->SetTitle( Form( "Rate vs Threshold. W_{RO}=%2.1f ns, W_{int}=%2.1f ns", getTsearch(),
								getDetectorGeometry()->getLengthOfSampleTimeSlice( getTelID() ) * iSummationWindow ) );
	if( getRunParameter()->fIgnoreDSTGains )
	{
		i_IPRGraph->GetXaxis()->SetTitle( "Threshold [FADC counts]" );
	}
	else
	{
		i_IPRGraph->GetXaxis()->SetTitle( "Threshold [p.e.]" );
	}
	i_IPRGraph->GetYaxis()->SetTitle( "Rate above Threshold [Hz]" );
	i_IPRGraph->SetName( Form( "IPRcharge_TelType%d_SW%d", ( int )getTelType( getTeltoAna()[i_tel] ), iSummationWindow ) );
	return i_IPRGraph;
}

bool VIPRCalculator::calculateIPRGraphsNoTimeSlices( unsigned int iSummationWindow, unsigned int i_tel )
{
	TDirectory* iG_CurrentDirectory = gDirectory;
	
	// get an IPR graph
	TGraphErrors* i_IPRGraph = getIPRGraph( iSummationWindow, true );
	if( !i_IPRGraph )
	{
		cout << "VIPRCalculator::calculateIPRGraphNoTimeSlices info: no IPR graph found for telescope type " << getTelType( getTeltoAna()[i_tel] ) << endl;
		return false;
	}
	
	//////////////////////////////////////////////////
	// average over all channels in one telescope
	
	TH1F* hIPR = ( TH1F* )FillIPRHistogram( iSummationWindow, i_tel );
	float norm = 0;
	if( hIPR )
	{
		norm = hIPR->Integral( 1, hIPR->GetNbinsX() );
		cout << "norm: " << norm << endl;
	}
	if( norm < fPedPerTelescopeTypeMinCnt )  //statistical limit for number of counts
	{
		fIPRAverageTel = true;
		cout << "VIPRCalculator::calculateIPRGraphNoTimeSlices WARNING: too few statistics to measure IPR curves ";
		cout << "(total counts available: " << norm << ", ";
		cout << "current limit " << fPedPerTelescopeTypeMinCnt << ")" << endl;
		cout << "IPR graphs will be provided as sum of " << getTeltoAna().size() << " telescopes statistics." << endl;
		cout << "VIPRCalculator::calculateIPRGraphNoTimeSlices fIPRAverageTel = " << fIPRAverageTel << endl;
		hIPR = calculateIPRGraphAveragedNoTimeSlices( iSummationWindow, i_tel );
		cout << "VIPRCalculator::calculateIPRGraphsNoTimeSlices norm IPR combined: " << hIPR->Integral( 1, hIPR->GetNbinsX() ) << endl;
	}
	if( norm == 0 )
	{
		cout << "VIPRCalculator::calculateIPRGraphsNoTimeSlices ERROR: no counts in IPR fpedcal_histo_storagegram !" << endl;
		return false;
	}
	i_IPRGraph = ( TGraphErrors* )updateIPRGraph( hIPR, i_tel, iSummationWindow );
	hIPR->Delete();
	iG_CurrentDirectory->cd();
	return true;
}


/*

	HANDLE IPR CALCULATION IF ASKED FOR IN TIME SLICES

*/

void VIPRCalculator::checkHistEmpty( const int telID, const int ts, const int pixel, const int sw )
{
	if( fpedcal_histo_storage[telID][ts][pixel][sw]->GetEntries() != 0 )
	{
		cout << "MK " <<  telID << " " << ts << " " << pixel << " " << sw << " " << fpedcal_histo_storage[telID][ts][pixel][sw]->GetEntries() << endl;
	}
}

vector<vector<vector<vector<TH1F*>>>> VIPRCalculator::getStorageHist()
{
	return fpedcal_histo_storage;
}

void VIPRCalculator::fillIPRPedestalHisto()
{
	fpedcal_histo_storage.resize( getTeltoAna().size() );
}

void VIPRCalculator::fillIPRPedestalHisto( const int telID, const vector<vector<vector<TH1F*>>>& fpedcal_histo )
{
	fpedcal_histo_storage[telID].push_back( fpedcal_histo[telID] );
}


TH1F* VIPRCalculator::getIPRPedestalHisto( const int telID, const int ts, const int pixel, const int sw )
{

	if( fpedcal_histo_storage.empty() )
	{
		cout << "empty" << endl;
	}
	if( fpedcal_histo_storage[telID][ts][pixel][sw] )
	{
		return fpedcal_histo_storage[telID][ts][pixel][sw];
	}
	else
	{
		return nullptr;
	}
}

//needs sliding window fpedcal_histo_storagegram!!
bool VIPRCalculator::calculateIPRGraphsTimeSlices( const int TimeSlice, const int iSummationWindow, const int i_tel )
{
	int iTelType = getTelType( getTeltoAna()[i_tel] );
	TDirectory* iG_CurrentDirectory = gDirectory;
	TGraphErrors* i_IPRGraph = getIPRGraphTimeSlice( true, TimeSlice );
	
	TFile* iPedFile = ( TFile* )initializePedestalFile( i_tel );
	if( iPedFile == nullptr )
	{
		cout << "VIPRCalculator::calculateIPRGraphsTimeSlices could not open pedestal file for telescope: " << iTelType;
		return false;
	}
	
	cout << "VIPRCalculator::calculateIPRGraphsTimeSlices for Telescope type " << iTelType << ": ";
	stringstream Directory( stringstream::in | stringstream::out );
	Directory.str( std::string() );
	Directory << "distributions_" << iTelType;
	if( iPedFile->Get( Directory.str().c_str() ) )
	{
		iPedFile->cd( Directory.str().c_str() );
	}
	else
	{
		cout << "VIPRCalculator::calculateIPRGraphsTimeSlices no directory: " << Directory.str().c_str() << endl;
	}
	
	//Simplify: always do this in digital counts. No need to check for initialization.
	TH1F* hIPR = new TH1F( "", "", 1000, 0., 100. );
	float i_gainCorrect = 1.;
	
	for( unsigned int i = 0; i < getNChannels(); i++ )
	{
		if( getDetectorGeometry()->getAnaPixel()[i] > 0 && i < getDead().size() && !getDead()[i] )
		{
		
			stringstream i_Hname( stringstream::in | stringstream::out );
			i_Hname << "hped_Tel" << iTelType << "_TS" << TimeSlice << "_Pix" << i << "_SW" << iSummationWindow;
			TH1F* h = ( TH1F* )gDirectory->Get( i_Hname.str().c_str() );
			
			if( h )
			{
				float ped = 0;
				if( getRunParameter()->fCombineChannelsForPedestalCalculation == 0 )
				{
					ped = getCalData()->getPedsTS_vector( false )[TimeSlice][i];
				}
				for( unsigned int j = 1; j <= h->GetNbinsX(); j++ )
				{
					if( h->GetBinContent( j ) > 0. && getGains()[i] > 0 )
					{
						i_gainCorrect = getGains()[i];
						if( getHIGHQE_gainfactor( i ) > 0. )
						{
							i_gainCorrect *= getHIGHQE_gainfactor( i );
						}
						if( i_gainCorrect > 0. )
						{
							//hIPR->Fill( ( h[i_tel][TimeSlice][i][iSummationWindow]->GetBinCenter( j ) - ped * iSummationWindow ) / i_gainCorrect, h[i_tel][TimeSlice][i][iSummationWindow]->GetBinContent( j ) );
							hIPR->Fill( ( h->GetBinCenter( 0 ) - ped * iSummationWindow ) / i_gainCorrect, h->GetBinContent( 0 ) );
						}
					}
				}
			}
		}
	}
	int z = 0;
	float norm = 0;
	if( hIPR )
	{
		norm = hIPR->Integral( 1, hIPR->GetNbinsX() );
		cout << "norm: " << norm << endl;
	}
	if( norm < fPedPerTelescopeTypeMinCnt )  //statistical limit for number of counts
	{
		fIPRAverageTel = true;
		cout << "VIPRCalculator::calculateIPRGraphTimeSlices WARNING: too few statistics to measure IPR curves ";
		cout << "(total counts available: " << norm << ", ";
		cout << "current limit " << fPedPerTelescopeTypeMinCnt << ")" << endl;
		cout << "IPR graphs will be provided as sum of " << getTeltoAna().size() << " telescopes statistics." << endl;
		cout << "VIPRCalculator::calculateIPRGraphNoTimeSlices fIPRAverageTel = " << fIPRAverageTel << endl;
		//hIPR = calculateIPRGraphAveragedNoTimeSlices( fPedFileNameC , iSummationWindow, i_tel );
		cout << "VIPRCalculator::calculateIPRGraphsNoTimeSlices norm IPR combined: " << hIPR->Integral( 1, hIPR->GetNbinsX() ) << endl;
	}
	if( norm == 0 )
	{
		cout << "VIPRCalculator::calculateIPRGraphsTimeSlices ERROR: no counts in IPR fpedcal_histo_storagegram !" << endl;
		return false;
	}
	
	for( int i = 1; i <= hIPR->GetNbinsX(); i++ )
	{
		if( hIPR->GetBinContent( i ) > 5 )
		{
			double val = convertRate( i_tel ) * hIPR->Integral( i, hIPR->GetNbinsX() ) / norm;
			double valerr = convertRate( i_tel ) * sqrt( hIPR->Integral( i, hIPR->GetNbinsX() ) ) / norm;
			double charge_pe = hIPR->GetXaxis()->GetBinCenter( i ) * getTelescopeAverageFADCtoPhe();
			double charge_pe_bin_width = 0.5 * hIPR->GetXaxis()->GetBinWidth( i ) * getTelescopeAverageFADCtoPhe();
			
			i_IPRGraph->SetPoint( z, charge_pe, val );
			i_IPRGraph->SetPointError( z, charge_pe_bin_width, valerr );
			z++;
		}
	}
	i_IPRGraph->SetMinimum( 1 );
	i_IPRGraph->SetTitle( Form( "Rate vs Threshold. W_{RO}=%2.1f ns, W_{int}=%2.1f ns", getTsearch(),
								getDetectorGeometry()->getLengthOfSampleTimeSlice( getTelID() ) * iSummationWindow ) );
	if( getRunParameter()->fIgnoreDSTGains )
	{
		i_IPRGraph->GetXaxis()->SetTitle( "Threshold [FADC counts]" );
	}
	else
	{
		i_IPRGraph->GetXaxis()->SetTitle( "Threshold [p.e.]" );
	}
	i_IPRGraph->GetYaxis()->SetTitle( "Rate above Threshold [Hz]" );
	i_IPRGraph->SetName( Form( "IPRcharge_TelType%d_SW%d_TS%d", iTelType, iSummationWindow, TimeSlice ) );
	hIPR->Delete();
	iPedFile->Close();
	iG_CurrentDirectory->cd();
	
	return true;
	
}

/*
 * calculate IPR graphs and write them to disk
 *
 * (loop over all telescopes)
 *
*/

bool VIPRCalculator::calculateIPRGraphs( std::vector<std::string> fPedFileNameC )
{

	definePedestalFile( fPedFileNameC );
	for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
	{
		setTelID( getTeltoAna()[i] );
		
		// first find dead channels
		// (ignore low gains here)
		findDeadChans( false );
		// calculate IPR graphs
		if( fIPRAverageTel == false )
		{
			calculateIPRGraphsNoTimeSlices( getSumWindow(), i );
		}
		else
		{
			copyIPRInitialized( getSumWindow(), i );
		}
		
		if( fIPRTimeSlices )
		{
			for( unsigned int ts = 0 ; ts < getCalData()->getPedsTS_vector( false ).size() ; ts++ )
			{
				cout << "MK ts: " << getCalData()->getPedsTS_vector( false ).size() << endl;
				calculateIPRGraphsTimeSlices( ts, getSumWindow(), i );
			}
		}
	}
	return true;
}


/*
 *
 * write IPR graphs to disk (one per telescope type)
 *
 */
bool VIPRCalculator::writeIPRgraphs( map<ULong64_t, vector<vector<TH1F*>>>& hped_vec, string iFile )
{
	TFile* fgraphs = 0;
	if( iFile.size() == 0 )
	{
		fgraphs = new TFile( getRunParameter()->fIPRdatabaseFile, "RECREATE" );
	}
	else
	{
		fgraphs = new TFile( iFile.c_str(), "UPDATE" );
	}
	if( fgraphs->IsZombie() )
	{
		cout << "VIPRCalculator::writeIPRgraphs error opening IPR output file: " << endl;
		cout << "\t" << fgraphs->GetName() << endl;
		return false;
	}
	
	// tree with conditions for these IRPs
	TTree* header = new TTree( "IPRheader", "IPR parameters" );
	unsigned int sumwin = 0;            // [FADC slices]
	unsigned int Nsamples = 0;          // [FADC slices]
	float ROwin = 0.;                   // [ns]
	float FADCslice = 0.;               // [ns]
	unsigned int SignalExtractor = 0;   // [ Selected Extractor ]
	header->Branch( "SignalExtractor", &SignalExtractor, "SignalExtractor/i" );
	header->Branch( "SummationWindow", &sumwin, "SummationWindow/i" );
	header->Branch( "Nsamples", &Nsamples, "Nsamples/i" );
	header->Branch( "FADCtimeSlice", &FADCslice, "FADCtimeSlice/F" );
	header->Branch( "ReadoutWindow", &ROwin, "ReadoutWindow/F" );
	
	// one graph per telescope ID
	map< ULong64_t, bool > iTelDone;
	for( unsigned int i = 0; i < getDetectorGeometry()->getTelType_list().size(); i++ )
	{
		iTelDone[getDetectorGeometry()->getTelType_list()[i]] = false;
	}
	
	for( unsigned int i = 0; i < getNTel(); i++ )
	{
		setTelID( i );
		if( iTelDone[getTelType( i )] )
		{
			continue;
		}
		
		if( hped_vec.find( getTelType( i ) ) == hped_vec.end() )
		{
			continue;
		}
		
		// loop over all summation windows
		for( unsigned int j = 0; j < hped_vec[getTelType( i )].size(); j++ )
		{
			// summation window
			int i_sw = j + 1;
			
			TGraphErrors* g = getIPRGraph( i_sw, false );
			if( !g )
			{
				continue;
			}
			SignalExtractor = getRunParameter()->fTraceIntegrationMethod.at( getTelID() );
			sumwin          = i_sw;
			Nsamples        = getNSamples();
			FADCslice       = getDetectorGeometry()->getLengthOfSampleTimeSlice( getTelID() ); //[ns]
			ROwin           = getDetectorGeometry()->getLengthOfSampleTimeSlice( getTelID() ) * ( float )getNSamples(); // [ns]
			
			header->Fill();
			g->Write();
		}
		
		iTelDone[getTelType( i )] = true;
	}
	header->Write();
	fgraphs->Close();
	fgraphs->Delete();
	
	return true;
}
