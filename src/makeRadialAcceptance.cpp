/*! \file  makeRadialAcceptance
 *  \brief calculate radial acceptance from data
 *
 *   use off events which pass all gamma/hadron separation cuts
 *
 */

#include "CData.h"
#include "TFile.h"

#include "VEvndispRunParameter.h"
#include "VRadialAcceptance.h"
#include "VGammaHadronCuts.h"
#include "VAnaSumRunParameter.h"

#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>
#include <sys/stat.h>

using namespace std;

int parseOptions( int argc, char* argv[] );
string listfilename = "";
string cutfilename = "";
string simpleListFileName = "";
string outfile = "acceptance.root";
unsigned int ntel = 4;                   // this shouldn't be changed unless you really unterstand why
string datadir = "../eventdisplay/output";
int entries = -1;
string histdir = "" ;
struct stat sb ;
string fInstrumentEpoch = "NOT_SET";
double fMaxDistanceAllowed = 2.5;

int main( int argc, char* argv[] )
{
	// print version only
	if( argc == 2 )
	{
		string fCommandLine = argv[1];
		if( fCommandLine == "-v" || fCommandLine == "--version" )
		{
			VGlobalRunParameter fRunPara;
			cout << fRunPara.getEVNDISP_VERSION() << endl;
			exit( EXIT_SUCCESS );
		}
	}
	
	VAnaSumRunParameter* fRunPara = new VAnaSumRunParameter();
	
	cout << endl;
	cout << "makeRadialAcceptance " << fRunPara->getEVNDISP_VERSION() << endl << endl;
	cout << "determine radial acceptance from off events (after cuts)" << endl;
	cout << endl;
	
	// read options from command line
	parseOptions( argc, argv );
	
	// read file list from run list file
	if( listfilename.size() > 0 )
	{
		fRunPara->loadLongFileList( listfilename, true );
		if( fRunPara->getRunListVersion() < 4 )
		{
			cout << "require run list >= 4" << endl;
			cout << "...exiting" << endl;
			exit( EXIT_FAILURE );
		}
		fRunPara->getEventdisplayRunParameter( datadir );
	}
	else if( simpleListFileName.size() > 0 )
	{
		fRunPara->loadSimpleFileList( simpleListFileName );
	}
	else
	{
		cout << "error reading run list" << endl;
		cout << "exiting..";
		exit( EXIT_FAILURE );
	}
	
	// read gamma/hadron cuts from cut file
	VGammaHadronCuts* fCuts = new VGammaHadronCuts();
	fCuts->setInstrumentEpoch( fInstrumentEpoch );
	fCuts->setNTel( ntel );
	if( cutfilename.size() > 0 )
	{
		if( !fCuts->readCuts( cutfilename ) )
		{
			cout << "error reading cut file: " << cutfilename << endl;
			cout << "exiting..." << endl;
			exit( EXIT_FAILURE );
		}
	}
	else
	{
		cout << "error: no gamma/hadron cut file given" << endl;
		cout << "(command line option -c)" << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	
	cout << "total number of files to read: " << fRunPara->fRunList.size() << endl;
	
	char ifile[1800];
	
	// create output file
	TFile* fo = new TFile( outfile.c_str(), "RECREATE" );
	if( fo->IsZombie() )
	{
		cout << "makeRadialAcceptances: error opening output file " << outfile << endl;
		return false;
	}
	cout << endl << "writing acceptance curves to " << fo->GetName() << endl;
	TDirectory* facc_dir = ( TDirectory* )fo;
	
	// create acceptance object
	VRadialAcceptance* facc = new VRadialAcceptance( fCuts, fRunPara, fMaxDistanceAllowed );
	
	// set facc to write extra histograms if necessary
	if( histdir.size() > 0 )
	{
		if( stat( histdir.c_str(), &sb ) == 0 && S_ISDIR( sb.st_mode ) ) // then directory 'histdir' exists
		{
			//facc->SetExtraHistogramMode( 1 ) ;
			facc->SetExtraHistogramDirectory( histdir ) ;
		}
		else
		{
			cout << "Error, directory specified by makeRadialAcceptance -w option '" << histdir << "' does not exist, exiting..." << endl;
			return false ;
		}
	}
	
	// az dependent measurement
	vector< VRadialAcceptance* > facc_az;
	vector< string > fDirName;
	vector< string > fDirTitle;
	vector< TDirectory* > facc_az_dir;
	vector< double > iAz_min;
	vector< double > iAz_max;
	
	iAz_min.push_back( 337.5 );
	iAz_max.push_back( 22.5 );
	for( unsigned int i = 1; i < 8; i++ )
	{
		iAz_min.push_back( -22.5 + 45. * ( double )i );
		iAz_max.push_back( 22.5 + 45. * ( double )i );
	}
	for( unsigned int i = 0; i < iAz_min.size(); i++ )
	{
		if( facc_dir )
		{
			facc_dir->cd();
		}
		sprintf( ifile, "az_%d", i );
		fDirName.push_back( ifile );
		sprintf( ifile, "AZ dependend radial acceptance, %.2f < az < %.2f", iAz_min[i], iAz_max[i] );
		fDirTitle.push_back( ifile );
		facc_az_dir.push_back( fo->mkdir( fDirName.back().c_str(), fDirTitle.back().c_str() ) );
		if( facc_az_dir.back() )
		{
			facc_az_dir.back()->cd();
			facc_az.push_back( new VRadialAcceptance( fCuts, fRunPara ) );
			facc_az.back()->setAzCut( iAz_min[i], iAz_max[i] );
			cout << "initializing AZ dependend radial acceptance class for ";
			cout << iAz_min[i] << " < az <= " <<   iAz_max[i] << endl;
		}
	}
	
	for( unsigned int i = 0; i < fRunPara->fRunList.size(); i++ )
	{
		sprintf( ifile, "%s/%d.mscw.root", datadir.c_str(), fRunPara->fRunList[i].fRunOff );
		cout << "now chaining " << ifile << " (wobble offset " << -1.*fRunPara->fRunList[i].fWobbleNorth << ", " << fRunPara->fRunList[i].fWobbleWest << ")" << endl;
		// test if file exists
		TFile fTest( ifile );
		if( fTest.IsZombie() )
		{
			cout << "error: file not found, " << ifile << endl;
			exit( EXIT_FAILURE );
		}
		// get data tree
		TTree* c = ( TTree* )fTest.Get( "data" );
		CData* d = new CData( c, false, 5, true );
		
		// Check number of telescopes in run
		VEvndispRunParameter* iParV2 = ( VEvndispRunParameter* )fTest.Get( "runparameterV2" );
		if( iParV2 )
		{
			if( ntel != iParV2->fTelToAnalyze.size() )
			{
				cout << endl;
				cout << "error: Number of Telecopes ntel " << ntel << " does not equal number in run " << iParV2->fTelToAnalyze.size() << " (defaul ntel 4)." << endl;
				exit( EXIT_FAILURE );
			}
		}
		
		// set gamma/hadron cuts
		fCuts->initializeCuts( fRunPara->fRunList[i].fRunOff, datadir );
		// pointer to data tree
		fCuts->setDataTree( d );
		
		if( !d )
		{
			cout << "makeRadialAcceptance: no data tree defined: run " << fRunPara->fRunList[i].fRunOff << endl;
			return false;
		}
		// data trees and cuts
		int nentries = d->fChain->GetEntries();
		if( entries > 0 )
		{
			nentries = entries;
		}
		
		cout << "filling acceptance curves with " << nentries << " events (before cuts)" << endl;
		
		if( fCuts )
		{
			fCuts->printCutSummary();
		}
		
		int neventStats = 0;
		int i_entries_after_cuts = 0;
		
		// loop over all entries in data trees and fill acceptance curves
		for( int n = 0; n < nentries; n++ )
		{
			d->GetEntry( n );
			
			if( n == 0 and d->isMC() )
			{
				cout << "\t (analysing MC data)" << endl;
			}
			
			neventStats = facc->fillAcceptanceFromData( d, n );
			
			for( unsigned int a = 0; a < facc_az.size(); a++ )
			{
				if( facc_az[a] )
				{
					facc_az[a]->fillAcceptanceFromData( d, n );
				}
			}
			
			if( neventStats < 0 )
			{
				break;
			}
			i_entries_after_cuts += neventStats;
		}
		cout << "total number of entries after cuts: " << i_entries_after_cuts << endl;
		cout << endl << endl;
		
		fTest.Close();
	}
	
	// write acceptance files to disk
	
	facc->calculate2DBinNormalizationConstant() ;
	facc->terminate( facc_dir );
	for( unsigned int a = 0; a < facc_az_dir.size(); a++ )
	{
		if( facc_az[a] )
		{
			facc_az[a]->terminate( facc_az_dir[a] );
		}
	}
	
	fo->Close();
	cout << "closing radial acceptance file: " << fo->GetName() << endl;
	
	cout << "exiting.." << endl;
}


/*!
    read in command line parameters
*/
int parseOptions( int argc, char* argv[] )
{
	while( 1 )
	{
		static struct option long_options[] =
		{
			{"help", no_argument, 0, 'h'},
			{"runlist", required_argument, 0, 'l'},
			{"srunlist", required_argument, 0, 's'},
			{"cutfile", required_argument, 0, 'c'},
			{"instrumentepoch", required_argument, 0, 'i'},
			{"maxdistance", required_argument, 0, 'm'},
			{"outfile", required_argument, 0, 'o'},
			{"entries", required_argument, 0, 'n'},
			{"datadir", required_argument, 0, 'd'},
			{"writehists", optional_argument, 0, 'w'},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		int c = getopt_long( argc, argv, "ht:s:l:e:m:o:i:d:n:c:w:", long_options, &option_index );
		if( optopt != 0 )
		{
			cout << "error: unknown option" << endl;
			cout << "exiting..." << endl;
			exit( EXIT_FAILURE );
		}
		if( argc == 1 )
		{
			c = 'h';
		}
		if( c == -1 )
		{
			break;
		}
		switch( c )
		{
			case 0:
				if( long_options[option_index].flag != 0 )
				{
					break;
				}
				printf( "option %s", long_options[option_index].name );
				if( optarg )
				{
					printf( " with arg %s", optarg );
				}
				printf( "\n" );
				break;
			case 'h':
				cout << endl;
				cout << "Options are:" << endl;
				cout << "-l --runlist [anasum-style run list file name, runlist on/off like]" << endl;
				cout << "-s --srunlist [simple run list file name]" << endl;
				cout << "-c --cutfile [cut file name]" << endl;
				cout << "-i --instrumentepoch [instrument epoch (e.g. V6)" << endl;
				cout << "-d --datadir [directory for input mscw root files]" << endl;
				cout << "-o --outfile [output ROOT file name]" << endl;
				cout << "-e --entries [number of entries]" << endl;
				cout << "-m --maxdist [max distance from camera centre (deg)]" << endl;
				cout << "-w --writehists [directory]" << endl ;
				cout << endl;
				exit( EXIT_SUCCESS );
				break;
			case 'd':
				cout << "Directory for input Files is " << optarg << endl;
				datadir = optarg;
				break;
			case 'o':
				cout << "Output File Name is " << optarg << endl;
				outfile = optarg;
				break;
			case 'l':
				cout << "Run List File Name is " << optarg << endl;
				listfilename = optarg;
				break;
			case 's':
				cout << "Simple List File Name is " << optarg << endl;
				simpleListFileName = optarg;
				break;
			case 'c':
				cutfilename = optarg;
				cout << "Cut File Name is " << cutfilename << endl;
				break;
			case 'm':
				fMaxDistanceAllowed = atof( optarg );
				cout << "Maximum allowed distance from camera centre: " << fMaxDistanceAllowed << endl;
				break;
			case 'i':
				fInstrumentEpoch = optarg;
				cout << "Instrument epoch is " << fInstrumentEpoch << endl;
				break;
			case 'e':
				entries = ( int )atoi( optarg );
				break;
			case 'w':
				histdir = optarg;
				cout << "Extra histograms will be written to " << histdir << endl;
			case '?':
				break;
			default:
				exit( EXIT_SUCCESS );
		}
	}
	return optind;
}
