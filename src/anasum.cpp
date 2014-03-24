/*! \file anasum.cpp
    \brief main program to create an analysis summary (VERITAS data analysis chain)


   \author( Jamie Holder, Gernot Maier )
*/

#include "VAnaSum.h"
#include "VGlobalRunParameter.h"

#include <getopt.h>
#include <iostream>
#include <string>

using namespace std;

int parseOptions( int argc, char* argv[] );

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// parameters read in from command line
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// input run list
string listfilename = "";
string listShortfilename = "";
// output file with all anasum results
string outfile = "output.ansum.root";
// run types:
//   0:  sequentiell analysis of a list file
//   1:  combine a list of runs and do a combined analysis
unsigned int runType = 0;
// location of data files (might be mscw file (run type 0) or anasum result file (run type 1 )
string datadir = "";
// run parameter file
string fRunParameterfile = "ANASUM.runparameter";
// analysis types: mono and stereo
unsigned int analysisType = 3;
// mono analysis only
int singletel = 0;
// for usage of random generators: see VStereoMaps.cpp
int fRandomSeed = 17;
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

   count number of command line arguments

*/
bool testCommandlineArguments()
{
// MONO ANALYSIS PROBABLY DOES NOT WORK ANYMORE
	if( analysisType != 3 && analysisType != 4 )
	{
		cout << "Mono analysis not well tested and disabled; DO NOT USE!" << endl;
		return false;
	}
// require a runlist file
	if( listfilename.size() < 1 && listShortfilename.size() < 1 )
	{
		cout << "error: missing required command line argument --runlist (-l) or --shortlist (-s)" << endl;
		return false;
	}
// require data directory
	if( datadir.size() < 1 )
	{
		cout << "error: missing required command line argument --datadir (-d)" << endl;
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
	cout << endl << "VERITAS Analysis Summary (University of Delaware & DESY) ";
	cout << " (version " << VGlobalRunParameter::getEVNDISP_VERSION() << ")" << endl;
	cout <<         "==========================================================================" << endl;
	cout << endl;
	
	parseOptions( argc, argv );
	
	if( !testCommandlineArguments() )
	{
		exit( 0 );
	}
	
// initialize analysis
	VAnaSum* anasum = new VAnaSum( datadir, analysisType );
	anasum->initialize( listfilename, listShortfilename, singletel - 1, runType, outfile, fRandomSeed, fRunParameterfile );
	cout << endl;
	
// mono analysis (GM: THIS HAS NOT BEEN USED SINCE PROTOTYPE TIMES - PROBABLY DOES NOT WORK ANYMORE)
	if( analysisType == 0 || analysisType == 1 )
	{
		anasum->doMonoAnalysis( ( analysisType == 1 ) || ( analysisType == 5 ) );
	}
// stereo analysis (default)
	else if( analysisType == 3 || analysisType == 4 )
	{
		anasum->doStereoAnalysis( ( analysisType == 3 ) || ( analysisType == 5 ) );
	}
	else
	{
		cout << endl << "error: unknown run analysisType" << endl;
		exit( -1 );
	}
// clean up and write results to disk
	anasum->terminate();
	
	cout << endl << "analysis results written to " << outfile << endl;
	
	return 0;
}


int parseOptions( int argc, char* argv[] )
{
	while( 1 )
	{
		static struct option long_options[] =
		{
			{"help", no_argument, 0, 'h'},
			{"runlist", required_argument, 0, 'l'},
			{"shortlist", required_argument, 0, 'k'},
			{"analysisType", required_argument, 0, 'm'},
			{"outfile", required_argument, 0, 'o'},
			{"datadir", required_argument, 0, 'd'},
			{"singletelescope", required_argument, 0, 's'},
			{"randomseed", required_argument, 0, 'r'},
			{"runType", required_argument, 0, 'i'},
			{"parameterfile",  required_argument, 0, 'f'},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		int c = getopt_long( argc, argv, "h:l:k:m:o:d:s:r:i:u:f:g", long_options, &option_index );
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
				if( gSystem->Getenv( "EVNDISPSYS" ) )
				{
					system( "cat $EVNDISPSYS/README/README.ANASUM" );
				}
				else
				{
					cout << " no help find (environmental variable EVNDISPSYS not set)" << endl;
				}
				exit( 0 );
				break;
			case 'd':
				datadir = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'i':
				runType = ( unsigned int )atoi( optarg );
				break;
			case 'm':
				analysisType = atoi( optarg );
				break;
			case 's':
				singletel = atoi( optarg );
				break;
			case 'l':
				listfilename = optarg;
				break;
			case 'k':
				listShortfilename = optarg;
				break;
			case 'r':
				fRandomSeed = atoi( optarg );
				break;
			case 'f':
				fRunParameterfile = optarg;
				break;
			case '?':
				break;
			default:
				abort();
		}
	}
	return optind;
}
