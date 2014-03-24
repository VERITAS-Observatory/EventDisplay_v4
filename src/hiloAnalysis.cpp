/*! file hiloAnalysis

    (VERITAS only)
    \author Gareth Hughes

*/

#include "VGlobalRunParameter.h"
#include "VHiLoTools.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

void parseOptions( int argc, char* argv[] );

// Parameters
int iTel = -1;
int iSW  = -1;
int iRun1 = -1;
int iRun2 = -1;
string dataDir = "";

int main( int argc, char* argv[] )
{

	parseOptions( argc, argv );
	
	VHiLoTools* a = new VHiLoTools();
	
	if( iTel < 0 )
	{
		cout << "Error: iTel is less than 0" << endl;
		exit( -1 );
	}
	if( iSW < 0 )
	{
		cout << "Error: iSW is less than 0" << endl;
		exit( -1 );
	}
	if( iRun1 < 0 )
	{
		cout << "Error: iRun1 is less than 0" << endl;
		exit( -1 );
	}
	if( iRun2 < 0 )
	{
		cout << "Error: iRun2 is less than 0" << endl;
		exit( -1 );
	}
	if( dataDir == "" )
	{
		cout << "Error: dataDir not defined" << endl;
		exit( -1 );
	}
	
	a->makeRootFileFromDSTFile( iTel, iSW, iRun1, iRun2, dataDir );
	
	return 0;
	
}

void parseOptions( int argc, char* argv[] )
{
	while( 1 )
	{
		static struct option long_options[] =
		{
			{"help", no_argument, NULL, 'h'},
			{"run1", required_argument, 0, 'a'},
			{"run2", required_argument, 0, 'b'},
			{"telescope", required_argument, 0, 't'},
			{"sumwindow", required_argument, 0, 's'},
			{"dataDir", required_argument, 0, 'd'},
			{ 0, 0, 0, 0 }
		};
		
		int option_index = 0;
		int c = getopt_long( argc, argv, "ha:b:t:s:d:", long_options, &option_index );
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
				char* ENV;
				ENV = getenv( "EVNDISPSYS" );
				if( gSystem->Getenv( "EVNDISPSYS" ) )
				{
					char readme[500];
					sprintf( readme, "cat %s/README/README.HILOANALYSIS", ENV );
					system( readme );
				}
				else
				{
					cout << " no help available (environmental variable EVNDISPSYS not set)" << endl;
				}
				exit( 0 );
				break;
			case 'a':
				iRun1 = atoi( optarg );
				break;
			case 'b':
				iRun2 = atoi( optarg );
				break;
			case 't':
				iTel = atoi( optarg );
				break;
			case 's':
				iSW = atoi( optarg );
				break;
			case 'd':
				dataDir = optarg;
				break;
			case '?':
				break;
			default:
				abort();
		}
	}
	return;
}
