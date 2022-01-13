/*! \file printRunParameter
    \brief print run parameters from mscw or eventdisplay file to screen


    \author Gernot Maier
*/

#include "VEvndispRunParameter.h"
#include "VTableLookupRunParameter.h"
#include "VGlobalRunParameter.h"
#include "VEvndispReconstructionParameter.h"
#include "VMonteCarloRunHeader.h"

#include "TFile.h"
#include "TKey.h"
#include "TTree.h"

#include <iostream>

using namespace std;

bool readRunParameter( TFile* fIn, string iPara )
{
	if( !fIn )
	{
		return false;
	}
	
	VEvndispRunParameter* fPar = 0;
	
	fPar = ( VEvndispRunParameter* )fIn->Get( "runparameterV2" );
        // possibly a DST file
	if( !fPar )
	{
		fPar = ( VEvndispRunParameter* )fIn->Get( "runparameterDST" );
	}
        // possibly a anasum file -> check first (!) run directory
        if( !fPar )
        {
           TIter next( fIn->GetListOfKeys() );
           TKey *key = 0;
           while( ( key = ( TKey * )next() ) )
           {
               string key_name = key->GetName();
               if( key_name.find( "run_" ) != string::npos )
               {
                   fPar = ( VEvndispRunParameter* )fIn->Get( (key_name+"/stereo/runparameterV2").c_str() );
                   break;
               }
           }
        }
	if( !fPar )
	{
		return false;
	}
	
	if( iPara == "-mcsourcefile" )
	{
		cout << fPar->fsourcefile << endl;
	}
	else if( iPara == "-date" )
	{
		cout << fPar->fDBRunStartTimeSQL << endl;
	}
	else if( iPara == "-mjd" )
	{
		cout << fPar->fDBDataStartTimeMJD << endl;
	}
	else if( iPara == "-teltoana" )
	{
		for( unsigned int i = 0; i < fPar->fTelToAnalyze.size(); i++ )
		{
			cout << fPar->fTelToAnalyze[i] + 1;
		}
		cout << endl;
	}
	else if( iPara == "-atmosphere" )
	{
		cout << fPar->fAtmosphereID << endl;
	}
	else if( iPara == "-epoch" )
	{
		cout << fPar->getInstrumentEpoch() << endl;
	}
	else if( iPara == "-majorepoch" )
	{
		cout << fPar->getInstrumentEpoch( true ) << endl;
	}
	else if( iPara == "-runtype" )
	{
		cout << fPar->fDBRunType << endl;
	}
	else if( iPara == "-evndispreconstructionparameterfile" )
	{
		cout << fPar->freconstructionparameterfile << endl;
	}
	else if( iPara.find( "runinfo" ) != string::npos )
	{
		cout << fPar->getInstrumentEpoch( false,
                                                  iPara.find( "updated-runinfo" ) != string::npos ) << "\t";
		cout << fPar->getInstrumentEpoch( true ) << "\t";
		cout << fPar->getAtmosphereID( iPara.find( "updated-runinfo" ) != string::npos ) << "\t";
		cout << fPar->fDBRunType << "\t";
		for( unsigned int i = 0; i < fPar->fTelToAnalyze.size(); i++ )
		{
			cout << fPar->fTelToAnalyze[i] + 1;
		}
		cout << endl;
	}
	
	return true;
}

bool readWobbleOffset( TFile *fIn, bool printInteger )
{
	if( !fIn )
	{
		return false;
	}
	VEvndispRunParameter *fPar = ( VEvndispRunParameter* )fIn->Get( "runparameterV2" );
        if( fPar )
        {
             cout << "Wobble offset: ";
             if( printInteger )
             {
                 cout << TMath::Nint( sqrt( fPar->fWobbleNorth*fPar->fWobbleNorth + fPar->fWobbleEast*fPar->fWobbleEast ) * 100. );
             }
             else
             {
                 cout << sqrt( fPar->fWobbleNorth*fPar->fWobbleNorth + fPar->fWobbleEast*fPar->fWobbleEast );
             }
             cout << endl;
             return true;
        }
        return false;
}


bool readMeanElevation( TFile *fIn )
{
	if( !fIn )
	{
		return false;
	}
        // get total number of telescopes available
        TTree *telconfig = (TTree*)fIn->Get( "telconfig" );
        if( !telconfig )
        {
            return false;
        }
        unsigned int iNTel = (unsigned int)telconfig->GetEntries();
        if( iNTel >= VDST_MAXTELESCOPES ) iNTel = VDST_MAXTELESCOPES;
        TTree *data = (TTree*)fIn->Get( "data" );
        if( data )
        {
            Double_t TelElevation[VDST_MAXTELESCOPES];
            data->SetBranchAddress( "TelElevation", TelElevation );
            data->GetEntry( 0 );
            double iMean_f = 0.;
            double iMeanN = 0.;
            for( unsigned int i = 0; i < iNTel; i++ )
            {
                if( TelElevation[i] > 5. )
                {
                    iMean_f += TelElevation[i];
                    iMeanN++;
                }
            }
            if( data->GetEntries() > 1 )
            {
                data->GetEntry( data->GetEntries() - 1 );
                for( unsigned int i = 0; i < iNTel; i++ )
                {
                    if( TelElevation[i] > 5. )
                    {
                        iMean_f += TelElevation[i];
                        iMeanN++;
                    }
                }
            }
            if( iMeanN > 0. )
            {
                iMean_f /= iMeanN;
                cout << "Average elevation: " << iMean_f << endl;
            }
        }
        else
        {
            cout << "not implemented" << endl;
        }

        return true;
}
	


bool readMCParameter( TFile* fIn, string iPara )
{
	if( !fIn )
	{
		return false;
	}
	
	VMonteCarloRunHeader* fMC = 0;
	
	fMC = ( VMonteCarloRunHeader* )fIn->Get( "MC_runheader" );
	if( !fMC )
	{
		return false;
	}
	
	if( iPara == "-mcaz" )
	{
		fMC->printMCAz();
	}
	else if( iPara == "-runnumber" )
	{
		fMC->printRunNumber();
	}
	else
	{
		return false;
	}
	
	return true;
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
			exit( 0 );
		}
	}
	
	if( argc != 2 && argc != 3 )
	{
		cout << endl;
		cout << "printRunParameter " << VGlobalRunParameter::getEVNDISP_VERSION() << endl;
		cout << "==========================" << endl;
		cout << endl;
		cout << "usage: printRunParameter <file> [opt]" << endl;
		cout << endl;
		cout << "print run parameters stored in eventdisplay or mscw_energy file" << endl;
		cout << endl;
		cout << "   options: " << endl;
		cout << "      -mcaz         print MC azimuth angle" << endl;
		cout << "      -runnumber    print MC run number" << endl;
		cout << "      -mcsourcefile print source file name" << endl;
		cout << "      -date         print date of run" << endl;
		cout << "      -mjd          print mjd of run" << endl;
		cout << "      -epoch        print epoch of this run" << endl;
		cout << "      -majorepoch   print major epoch of this run" << endl;
		cout << "      -atmosphere   print corsika ID of atmospheric condition of this run" << endl;
		cout << "      -runtype      print run type, eg observing, obsFilter etc." << endl;
		cout << "      -teltoana     print telescope combination used in analysis" << endl;
		cout << "      -evndispreconstructionparameterfile print evndisp reconstruction parameter file" << endl;
		cout << "      -runinfo      print relevant run info in one line" << endl;
                cout << "      -updated-runinfo print relevant run info in one line (update epoch from VERITAS.Epochs.runparameter)" << endl;
                cout << "      -elevation    print (rough) average elevation" << endl;
                cout << "      -wobble       print wobble offset" << endl;
                cout << "      -wobbleInt    print wobble offset (as integer, x100)" << endl;
		cout << endl;
		exit( 0 );
	}
	// command line option
	string fOption = "";
	if( argc == 3 )
	{
		fOption = argv[2];
	}
	if( fOption.size() == 0 )
	{
		cout << endl;
		cout << "printRunParameter " << VGlobalRunParameter::getEVNDISP_VERSION() << endl;
		cout << "==========================" << endl;
	}
	
	// open file
        gErrorIgnoreLevel = kError;
	TFile* fIn = new TFile( argv[1] );
	if( fIn->IsZombie() )
	{
		cout << "error: file not found: " << argv[1] << endl;
		cout << "exiting..." << endl;
		exit( 0 );
	}
	
	if( fOption.size() > 0 )
	{
                if( fOption == "-elevation" )
                {
                        readMeanElevation( fIn );
                }
                else if( fOption.find( "-wobble" ) != string::npos )
                {
                       readWobbleOffset( fIn, (fOption.find( "-wobbleInt" ) != string::npos) );
                }
		else if( fOption == "-mcaz" || fOption == "-runnumber" )
		{
			readMCParameter( fIn, fOption );
		}
		else
		{
			readRunParameter( fIn, fOption );
		}
		exit( 0 );
	}
	
	VEvndispRunParameter* fPar = 0;
	
	fPar = ( VEvndispRunParameter* )fIn->Get( "runparameterV2" );
	if( !fPar )
	{
		fPar = ( VEvndispRunParameter* )fIn->Get( "runparameterDST" );
	}
        // possibly a anasum file -> check first (!) run directory
        if( !fPar )
        {
           TIter next( fIn->GetListOfKeys() );
           TKey *key = 0;
           while( ( key = ( TKey * )next() ) )
           {
               string key_name = key->GetName();
               if( key_name.find( "run_" ) != string::npos )
               {
                   fPar = ( VEvndispRunParameter* )fIn->Get( (key_name+"/stereo/runparameterV2").c_str() );
                   cout << "Reading run parameters from key_name" << endl;
                   break;
               }
           }
        }
	
	if( fPar )
	{
		if( fPar->fEventDisplayUser != "CTA-DST" )
		{
			fPar->print( 2 );
		}
		else
		{
			fPar->printCTA_DST();
		}
	}
	
	// array analysis cuts
	
	VEvndispReconstructionParameter* fArrayCuts = 0;
	
	fArrayCuts = ( VEvndispReconstructionParameter* )fIn->Get( "EvndispReconstructionParameter" );
	if( fArrayCuts )
	{
		cout << endl << endl;
		cout << "===========================================" << endl;
		cout << "===========================================" << endl;
		fArrayCuts->print_arrayAnalysisCuts();
	}
	
	VTableLookupRunParameter* fTPar = 0;
	
	fTPar = ( VTableLookupRunParameter* )fIn->Get( "TLRunParameter" );
	
	if( fTPar )
	{
		cout << endl << endl;
		cout << "===========================================" << endl;
		cout << "===========================================" << endl;
		fTPar->print( 2 );
	}
	
	VMonteCarloRunHeader* fMC = 0;
	
	//    if( fPar && fPar->fEventDisplayUser != "CTA-DST" )
	{
		fMC = ( VMonteCarloRunHeader* )fIn->Get( "MC_runheader" );
		if( fMC )
		{
			cout << endl << endl;
			cout << "===========================================" << endl;
			cout << "===========================================" << endl;
			fMC->print();
		}
	}
	
	
	fIn->Close();
	
}
