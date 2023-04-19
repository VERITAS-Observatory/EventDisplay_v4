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

void VIPRCalculator::definePedestalFile( std::vector<std::string> fPedFileNameCalibrator )
{
	for( unsigned int i = 0; i < fPedFileNameCalibrator.size() ; i++)
	{
	        fPedFileNameC.push_back(fPedFileNameCalibrator[i]);
	}
}

bool VIPRCalculator::calculateIPRGraphs(std::vector<std::string> fPedFileNameCalibrator )
{

        definePedestalFile(fPedFileNameCalibrator);
	for( unsigned int i = 0; i < getTeltoAna().size(); i++ )
=======

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
>>>>>>> d875f786fff9a004b915438d724c168de7974e22
	{
		setTelID( getTeltoAna()[i] );
		
		// first find dead channels
		// (ignore low gains here)
		findDeadChans( false );
		// calculate IPR graphs
		if( fIPRAverageTel == false )
		{
			calculateIPRGraphs( fPedFileNameC[getTeltoAna()[i]], getSumWindow(), getTelType( getTeltoAna()[i] ), i );
		}
		else
		{
			copyIPRTelAveraged( getSumWindow(), getTeltoAna()[i], i );
		}
	}
<<<<<<< HEAD
=======
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
>>>>>>> d875f786fff9a004b915438d724c168de7974e22
	return true;
}

/*
 * calculate IPR graphs and write them to disk
 *
 * (this is done per telescope type)
 *
 */

<<<<<<< HEAD
bool VIPRCalculator::copyIPRTelAveraged( unsigned int iSummationWindow, ULong64_t iTelType, unsigned int i_tel )
{

	TGraphErrors* i_IPRGraph = getIPRGraph( iSummationWindow, true );
	
	setTelID( getTeltoAna()[0] );
	TGraphErrors* i_IPRGraph_Tel0 = getIPRGraph( iSummationWindow, true );
	
	setTelID( getTeltoAna()[i_tel] );
	
	if( !i_IPRGraph )
=======
	HANDLE IPR CALCULATION IF ASKED FOR IN TIME SLICES

*/

void VIPRCalculator::checkHistEmpty( const int telID, const int ts, const int pixel, const int sw )
{
	if( fpedcal_histo_storage[telID][ts][pixel][sw]->GetEntries() != 0 )
>>>>>>> d875f786fff9a004b915438d724c168de7974e22
	{
		cout << "VIPRCalculator::copyIPRTelAveraged: no IPR graph found for telescope type " << iTelType << endl;
		return false;
	}
<<<<<<< HEAD
	if( !i_IPRGraph_Tel0 )
=======
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
>>>>>>> d875f786fff9a004b915438d724c168de7974e22
	{
		cout << "VIPRCalculator::copyIPRTelAveraged: no IPR graph found for telescope type " << getTeltoAna()[0] << endl;
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
	i_IPRGraph->SetName( Form( "IPRcharge_TelType%d_SW%d", ( int )iTelType, iSummationWindow ) );
	
	return true;
	
}

<<<<<<< HEAD
bool VIPRCalculator::calculateIPRGraphs( string iPedFileName, unsigned int iSummationWindow, ULong64_t iTelType, unsigned int i_tel )
=======
//needs sliding window fpedcal_histo_storagegram!!
bool VIPRCalculator::calculateIPRGraphsTimeSlices( const int TimeSlice, const int iSummationWindow, const int i_tel )
>>>>>>> d875f786fff9a004b915438d724c168de7974e22
{
	TH1F* hIPR = ( TH1F* )initializeIPRHistogram( iSummationWindow, getTeltoAna()[i_tel] );
	TDirectory* iG_CurrentDirectory = gDirectory;
<<<<<<< HEAD
	
	// get an IPR graph
	TGraphErrors* i_IPRGraph = getIPRGraph( iSummationWindow, true );
	if( !i_IPRGraph )
=======
	TGraphErrors* i_IPRGraph = getIPRGraphTimeSlice( true, TimeSlice );
	
	TFile* iPedFile = ( TFile* )initializePedestalFile( i_tel );
	if( iPedFile == nullptr )
>>>>>>> d875f786fff9a004b915438d724c168de7974e22
	{
		cout << "VIPRCalculator::calculateIPRGraphs info: no IPR graph found for telescope type " << iTelType << endl;
		return false;
	}
<<<<<<< HEAD
	// check suffix of ped file
	if( iPedFileName.find( ".root" ) == string::npos )
=======
	
	cout << "VIPRCalculator::calculateIPRGraphsTimeSlices for Telescope type " << iTelType << ": ";
	stringstream Directory( stringstream::in | stringstream::out );
	Directory.str( std::string() );
	Directory << "distributions_" << iTelType;
	if( iPedFile->Get( Directory.str().c_str() ) )
>>>>>>> d875f786fff9a004b915438d724c168de7974e22
	{
		iPedFileName += ".root";
	}
	// open pedestal files
	TFile iPedFile( iPedFileName.c_str() );
	if( iPedFile.IsZombie() )
	{
		cout << "VIPRCalculator::calculateIPRGraphs error reading IPR graphs from ";
		cout << iPedFileName << endl;
		return false;
	}
	// histograms with IPR distributions are either
	// i) in a directory called distributions_TelType
	// ii) in the current directory
	cout << "Telescope type " << iTelType << ": ";
	cout << "reading IPR histograms for summation window " << iSummationWindow;
	cout << " from ";
	cout << iPedFileName << endl;
	
<<<<<<< HEAD
	stringstream i_Directory( stringstream::in | stringstream::out );
	i_Directory << "distributions_" << iTelType;
	if( iPedFile.Get( i_Directory.str().c_str() ) )
	{
		iPedFile.cd( i_Directory.str().c_str() );
	}
=======
	//Simplify: always do this in digital counts. No need to check for initialization.
	TH1F* hIPR = new TH1F( "", "", 1000, 0., 100. );
	float i_gainCorrect = 1.;
>>>>>>> d875f786fff9a004b915438d724c168de7974e22
	
	hIPR->Reset();
	
	//////////////////////////////////////////////////
	// average over all channels in one telescope
	float i_gainCorrect = 1.;
	for( unsigned int i = 0; i < getNChannels(); i++ )
	{
		if( getDetectorGeometry()->getAnaPixel()[i] > 0
				&& i < getDead().size() && !getDead()[i] )
		{
<<<<<<< HEAD
			stringstream i_Hname( stringstream::in | stringstream::out );
			i_Hname << "hpedPerTelescopeType_" << iTelType << "_" << iSummationWindow << "_" << i;
=======
		
			stringstream i_Hname( stringstream::in | stringstream::out );
			i_Hname << "hped_Tel" << iTelType << "_TS" << TimeSlice << "_Pix" << i << "_SW" << iSummationWindow;
>>>>>>> d875f786fff9a004b915438d724c168de7974e22
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
				// special treatment for ASTRI telescopes
				else if( getRunParameter()->fCombineChannelsForPedestalCalculation == 2 )
				{
					stringstream i_Pname( stringstream::in | stringstream::out );
					i_Pname << "hped_" << iTelType << "_" << iSummationWindow << "_" << i;
					TH1F* hP = ( TH1F* )gDirectory->Get( i_Pname.str().c_str() );
					if( hP )
					{
						ped = hP->GetMean();
					}
				}
<<<<<<< HEAD
				for( int j = 1; j <= h->GetNbinsX(); j++ )
=======
				for( unsigned int j = 1; j <= h->GetNbinsX(); j++ )
>>>>>>> d875f786fff9a004b915438d724c168de7974e22
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
<<<<<<< HEAD
							hIPR->Fill( ( h->GetBinCenter( j ) - ped * iSummationWindow ) / i_gainCorrect, h->GetBinContent( j ) );
=======
							//hIPR->Fill( ( h[i_tel][TimeSlice][i][iSummationWindow]->GetBinCenter( j ) - ped * iSummationWindow ) / i_gainCorrect, h[i_tel][TimeSlice][i][iSummationWindow]->GetBinContent( j ) );
							hIPR->Fill( ( h->GetBinCenter( 0 ) - ped * iSummationWindow ) / i_gainCorrect, h->GetBinContent( 0 ) );
>>>>>>> d875f786fff9a004b915438d724c168de7974e22
						}
					}
				}
			}
		}
	}
<<<<<<< HEAD
	
	int z = 0;
	float norm = hIPR->Integral( 1, hIPR->GetNbinsX() );
	if( norm < fPedPerTelescopeTypeMinCnt )  //statistical limit for number of counts
	{
		fIPRAverageTel = true;
		cout << "Telescope " << iTelType << ": ";
		cout << "VIPRCalculator::calculateIPRGraphs WARNING: too few statistics to measure IPR curves ";
		cout << "(total counts available: " << norm << ", ";
		cout << "current limit " << fPedPerTelescopeTypeMinCnt << ")" << endl;
		cout << "IPR graphs will be provided as sum of " << getTeltoAna().size() << " telescopes statistics." << endl;
		cout << "VIPRCalculator::calculateIPRGraphs(): fIPRAverageTel = " << fIPRAverageTel << endl;
		hIPR = calculateIPRGraphAveraged( iSummationWindow );
		cout << "VIPRCalculator::calculateIPRGraphs norm IPR combined: " << hIPR->Integral( 1, hIPR->GetNbinsX() ) << endl;
	}
	if( norm == 0 )
	{
		cout << "VIPRCalculator::calculateIPRGraphs ERROR: no counts in IPR histogram !" << endl;
		return false;
	}
	// convert to Rate
	float nsToSec = 1E-9;
	float Tsearch = getDetectorGeometry()->getLengthOfSampleTimeSlice( getTelID() );
	if( getSearchWindowLast() < getNSamples() )
	{
		Tsearch *= ( getSearchWindowLast() - getSumFirst() ); // [ns]
	}
	else
	{
		Tsearch *= ( getNSamples() - getSumFirst() ); // [ns]
	}
	float convToHz = 1.;
	if( nsToSec > 0. && Tsearch > 0. )
	{
		convToHz /= ( nsToSec * Tsearch );
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
=======
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
>>>>>>> d875f786fff9a004b915438d724c168de7974e22
	
	for( int i = 1; i <= hIPR->GetNbinsX(); i++ )
	{
		if( hIPR->GetBinContent( i ) > 5 )
		{
<<<<<<< HEAD
			double val = convToHz * hIPR->Integral( i, hIPR->GetNbinsX() ) / norm;
			double valerr = convToHz * sqrt( hIPR->Integral( i, hIPR->GetNbinsX() ) ) / norm;
=======
			double val = convertRate( i_tel ) * hIPR->Integral( i, hIPR->GetNbinsX() ) / norm;
			double valerr = convertRate( i_tel ) * sqrt( hIPR->Integral( i, hIPR->GetNbinsX() ) ) / norm;
>>>>>>> d875f786fff9a004b915438d724c168de7974e22
			double charge_pe = hIPR->GetXaxis()->GetBinCenter( i ) * getTelescopeAverageFADCtoPhe();
			double charge_pe_bin_width = 0.5 * hIPR->GetXaxis()->GetBinWidth( i ) * getTelescopeAverageFADCtoPhe();
			
			i_IPRGraph->SetPoint( z, charge_pe, val );
			i_IPRGraph->SetPointError( z, charge_pe_bin_width, valerr );
			z++;
<<<<<<< HEAD
		}
	}
	
	i_IPRGraph->SetMinimum( 1 );
	i_IPRGraph->SetTitle( Form( "Rate vs Threshold. W_{RO}=%2.1f ns, W_{int}=%2.1f ns", Tsearch,
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
	i_IPRGraph->SetName( Form( "IPRcharge_TelType%d_SW%d", ( int )iTelType, iSummationWindow ) );
	hIPR->Delete();
	
	iPedFile.Close();
	
	iG_CurrentDirectory->cd();
	
	return true;
}

TH1F* VIPRCalculator::initializeIPRHistogram( unsigned int iSummationWindow, unsigned int iTelType )
=======
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
>>>>>>> d875f786fff9a004b915438d724c168de7974e22
{
	//get reference hpedPerTelescopeType for channel 1
	cout << "VIPRCalculator::initializeIPRHistogram initializing IPR graph for calculation of average of telescopes." << endl;
	TH1F* hIPR = 0;
	
	
	int TelType = getTelType( iTelType );
	string PedFile_name = fPedFileNameC[iTelType];
	
	// check suffix of ped file
	if( PedFile_name.find( ".root" ) == string::npos )
	{
		PedFile_name += ".root";
	}
	// open pedestal files
	TFile PedFile( PedFile_name.c_str() );
	
	if( PedFile.IsZombie() )
	{
		cout << "VIPRCalculator::calculateIPRGraphsStatistics: error reading IPR graphs from ";
		cout << PedFile_name << endl;
		return hIPR;
	}
	
	// histograms with IPR distributions are either
	// i) in a directory called distributions_TelType
	// ii) in the current directory
	cout << "Telescope type " << TelType << ": ";
	cout << "reading IPR histograms for summation window " << iSummationWindow;
	cout << " from ";
	cout << PedFile_name << endl;
	
	stringstream Directory( stringstream::in | stringstream::out );
	Directory << "distributions_" << TelType;
	if( !PedFile.Get( Directory.str().c_str() ) )
	{
		return hIPR;
	}
	
	// get charge distribution for first channel as reference histogram
	stringstream hIPRname( stringstream::in | stringstream::out );
	hIPRname << "distributions_" << TelType << "/hpedPerTelescopeType_" << TelType << "_" << iSummationWindow << "_" << 1;
	
	//?
	TH1F* href = ( TH1F* )gDirectory->Get( hIPRname.str().c_str() );
	PedFile.Close();
	
	if( !href )
	{
		cout << " Error: cannot find IPR histogram " << hIPRname.str().c_str();
		cout << " in file:" << PedFile_name << " ... exiting " << endl;
		return hIPR;
	}
	
	
	//summary histogram
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

TH1F* VIPRCalculator::calculateIPRGraphAveraged( unsigned int iSummationWindow )
{

	TH1F* hIPR = ( TH1F* )initializeIPRHistogram( iSummationWindow, getTeltoAna()[0] );
	
	for( unsigned int teltype = 0; teltype < getTeltoAna().size(); teltype++ )
	{
		setTelID( getTeltoAna()[teltype] );
		
		// first find dead channels
		// (ignore low gains here)
		findDeadChans( false );
		// calculate IPR graphs
		string PedFile_name = fPedFileNameC[getTeltoAna()[teltype]];
		int TelType = getTelType( getTeltoAna()[teltype] );
		
		if( PedFile_name.find( ".root" ) == string::npos )
		{
			PedFile_name += ".root";
		}
		// open pedestal files
		TFile PedFile( PedFile_name.c_str() );
		if( PedFile.IsZombie() )
		{
			cout << "VIPRCalculator::calculateIPRGraphAveraged error reading IPR graphs from ";
			cout << PedFile_name << endl;
			TH1F* hNull = 0;
			return hNull;
		}
		// histograms with IPR distributions are either
		// i) in a directory called distributions_TelType
		// ii) in the current directory
		cout << "VIPRCalculator::calculateIPRGraphAveraged Telescope type " << TelType << ": ";
		cout << "reading IPR histograms for summation window " << iSummationWindow;
		cout << " from ";
		cout << PedFile_name << endl;
		stringstream Directory( stringstream::in | stringstream::out );
		Directory.str( std::string() );
		Directory << "distributions_" << TelType;
		cout << Directory.str().c_str() << endl;
		if( PedFile.Get( Directory.str().c_str() ) )
		{
			PedFile.cd( Directory.str().c_str() );
		}
		else
		{
			cout << "VIPRCalculator::calculateIPRGraphAveraged no directory: " << Directory.str().c_str() << endl;
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
				HistName << "hpedPerTelescopeType_" << TelType << "_" << iSummationWindow << "_" << i;
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
						Pname << "hped_" << TelType << "_" << iSummationWindow << "_" << i;
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
		PedFile.Close();
		
	}
	hIPR->Scale( 1. / getTeltoAna().size() );
	float norm = getTeltoAna().size() * hIPR->Integral( 1, hIPR->GetNbinsX() );
	cout << "VIPRCalculator::calculateIPRGraphAveraged normalization of average IPR histogram " << norm;
	cout << ". Returning IPR histogram." << endl;
	if( norm < fPedPerTelescopeTypeMinCnt )  //statistical limit for number of counts
	{
		cout << "VIPRCalculator::calculateIPRGraphAveraged WARNING: there is NOT enough statistics ";
		cout << " ( < " << fPedPerTelescopeTypeMinCnt << ") even when averaging over all telescopes." << endl;
		return hIPR;
	}
	else
	{
		cout << "VIPRCalculator::calculateIPRGraphsStatistics: there is enough statistics to average over telescopes. " << endl;
		return hIPR;
	}
}

