/*! \file combineLookupTables
    \brief combine different lookup tablefiles into a single tablefile

*/

#include "TClass.h"
#include "TDirectory.h"
#include "TDirectoryFile.h"
#include "TFile.h"
#include "TH2F.h"
#include "TTree.h"
#include "TKey.h"
#include "TSystem.h"
#include "TROOT.h"

#include "VGlobalRunParameter.h"
#include "VHistogramUtilities.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// list with noise levels
vector< int > fNoiseLevel;

void copyDirectory( TDirectory* source, const char* hx, vector< string > hist_to_copy, float noise_tolerance );

/*
 * noise directory names are determined in the lookup table code using the
 * mean pedvar level. These can vary by a small around from simulation to
 * simulation file. We search here therefore for very similar noise levels,
 * and return those directory names if available
 */
string check_for_similar_noise_values( const char* hx, float noise_tolerance )
{
	string iTemp = hx;
	
	if( iTemp.find( "_" ) != string::npos )
	{
		int i_noise = atoi( iTemp.substr( iTemp.find( "_" ) + 1, iTemp.size() ).c_str() );
		for( unsigned int i = 0; i < fNoiseLevel.size(); i++ )
		{
			if( TMath::Abs( fNoiseLevel[i] - i_noise ) < noise_tolerance )
			{
				char hname[200];
				sprintf( hname, "NOISE_%05d", fNoiseLevel[i] );
				iTemp = hname;
				cout << "\t found similar noise level, save into directory: " << iTemp << "\t" << fNoiseLevel[i];
				cout << " (" << fNoiseLevel.size() << ")" << endl;
				return iTemp;
			}
		}
		fNoiseLevel.push_back( i_noise );
		cout << "\t new noise level directory: " << iTemp << "(" << fNoiseLevel.size() << ")" << endl;
	}
	
	return iTemp;
}

vector< string > readListOfFiles( string iFile )
{
	vector< string > iList;
	
	ifstream is;
	is.open( iFile.c_str() );
	if( !is )
	{
		cout << "error while reading file list " << iFile << endl;
		cout << "exiting...." << endl;
		exit( EXIT_FAILURE );
	}
	string is_line;
	
	while( getline( is, is_line ) )
	{
		iList.push_back( is_line );
	}
	
	is.close();
	
	return iList;
}


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
			exit( EXIT_FAILURE );
		}
	}
	
	
	VGlobalRunParameter* iT = new VGlobalRunParameter();
	cout << endl;
	cout << "combineLookupTables (" << iT->getEVNDISP_VERSION() << ")" << endl;
	cout << "-----------------------------" << endl;
	cout << endl;
	
	if( argc < 2 )
	{
		cout << "combine several tables from different files into one single table file" << endl << endl;
		cout << "combineLookupTables <file with list of tables> <output file name> [histogram types to copy] [noise tolerance]" << endl;
		cout << endl;
		cout << "[histogram types]:    all, mpv, median (default)" << endl;
		cout << "[noise tolerance]:    tolerance for combining NSB bins (default==20)" << endl;
		cout << endl;
		exit( EXIT_FAILURE );
	}
	string fListOfFiles = argv[1];
	string fOFile       = argv[2];
	string histogram_types = "median";
	if( argc >= 4 )
	{
		histogram_types = argv[3];
	}
	float noise_tolerance = 20.;
	if( argc == 5 )
	{
		noise_tolerance = atof( argv[4] );
	}
	
	vector< string > hist_to_copy;
	if( histogram_types == "all" )
	{
		hist_to_copy.push_back( "median" );
		hist_to_copy.push_back( "Median" );
		hist_to_copy.push_back( "mpv" );
	}
	else if( histogram_types == "median" )
	{
		hist_to_copy.push_back( "median" );
		hist_to_copy.push_back( "Median" );
	}
	else if( histogram_types == "mpv" )
	{
		hist_to_copy.push_back( "mpv" );
	}
	else
	{
		cout << "unknown histogram type (use all/median/mpv)" << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	cout << "Copying histograms of type: ";
	for( unsigned int i = 0; i < hist_to_copy.size(); i++ )
	{
		cout << hist_to_copy[i] << "  ";
	}
	cout << endl;
	
	vector< string > fInFiles = readListOfFiles( fListOfFiles );
	unsigned int nFiles = fInFiles.size();
	if( nFiles == 0 )
	{
		cout << "error: no files in file list" << endl;
		cout << "exiting...." << endl;
		exit( EXIT_FAILURE );
	}
	cout << "combining " << nFiles << " table files into " << fOFile << endl;
	
	TFile* fROFile = new TFile( fOFile.c_str(), "RECREATE" );
	if( fROFile->IsZombie() )
	{
		cout << "error while opening combined file: " << fOFile << endl;
		exit( EXIT_FAILURE );
	}
	TFile* fIn = 0;
	for( unsigned int f = 0; f < fInFiles.size(); f++ )
	{
		fIn = new TFile( fInFiles[f].c_str() );
		if( fIn->IsZombie() )
		{
			cout << "error while opening file: " << fInFiles[f] << endl;
			continue;
		}
		cout << "now reading file " << f << ": " << fInFiles[f] << endl;
		
		//loop on all entries of this directory
		TKey* key;
		TIter nextkey( fIn->GetListOfKeys() );
		while( ( key = ( TKey* )nextkey() ) )
		{
			const char* classname = key->GetClassName();
			TClass* cl = gROOT->GetClass( classname );
			if( !cl )
			{
				continue;
			}
			if( cl->InheritsFrom( "TDirectory" ) )
			{
				TDirectory* iSource = ( TDirectory* )key->ReadObj();
				if( !iSource )
				{
					continue;
				}
				fROFile->cd();
				const char* hname = iSource->GetName();
				cout << "\t copying " << hname << endl;
				copyDirectory( iSource, hname, hist_to_copy, noise_tolerance );
			}
		}
		fIn->Close();
	}
	
	fROFile->Close();
	cout << endl;
	cout << "total number of noise levels found: " << fNoiseLevel.size() << " (is this ok? check!)" << endl;
	cout << "finished..." << endl;
}


/*!
 *   from http://root.cern.ch/phpBB2/viewtopic.php?t=2789
 *
 *   Author: Rene Brun
 */
void copyDirectory( TDirectory* source,
					const char* hx,
					vector< string > hist_to_copy,
					float noise_tolerance )
{
	//copy all objects and subdirs of directory source as a subdir of the current directory
	TDirectory* savdir = gDirectory;
	TDirectory* adir = 0;
	
	// 1. case: top directory exists (NOISE_...)
	if( hx )
	{
		string noise_dir = check_for_similar_noise_values( hx, noise_tolerance );
		adir = ( TDirectory* )savdir->Get( noise_dir.c_str() );
	}
	else
	{
		adir = ( TDirectory* )savdir->Get( source->GetName() );
	}
	
	if( !adir )
	{
		// 2. case: make top directory
		if( hx )
		{
			adir = savdir->mkdir( hx );
		}
		else
		{
			adir = savdir->mkdir( source->GetName() );
		}
		if( !adir )
		{
			cout << "error while creating directory " << source->GetName() << endl;
			cout << "exiting..." << endl;
			exit( EXIT_FAILURE );
		}
	}
	adir->cd();
	//loop on all entries of this directory
	TKey* key;
	TIter nextkey( source->GetListOfKeys() );
	while( ( key = ( TKey* )nextkey() ) )
	{
		const char* classname = key->GetClassName();
		TClass* cl = gROOT->GetClass( classname );
		if( !cl )
		{
			continue;
		}
		if( cl->InheritsFrom( "TDirectory" ) )
		{
			string iName = key->GetName();
			if( iName == "histos1D" )
			{
				continue;
			}
			source->cd( key->GetName() );
			TDirectory* subdir = gDirectory;
			adir->cd();
			copyDirectory( subdir, 0, hist_to_copy, noise_tolerance );
			adir->cd();
		}
		else
		{
			source->cd();
			TObject* obj = key->ReadObj();
			string iName = obj->GetName();
			if( iName.find( "new" ) < string::npos )
			{
				cout << "writing " << iName << "\t" << obj->GetName() << endl;
				cout << gDirectory->GetPath() << endl;
			}
			// copy only median and mpv histogram
			for( unsigned int i = 0; i < hist_to_copy.size(); i++ )
			{
				if( iName.find( hist_to_copy[i].c_str() ) != string::npos )
				{
					adir->cd();
					obj->Write( iName.c_str() );
					break;
				}
			}
			delete obj;
		}
	}
	adir->SaveSelf( kTRUE );
	savdir->cd();
}
