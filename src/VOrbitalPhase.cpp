/*! \class VOrbitalPhase
    \brief calculation of the orbital phase

*/

#include "VOrbitalPhase.h"

VOrbitalPhase::VOrbitalPhase()
{
    fOrbit = 0;
    fRefMJD = 0;
}


bool VOrbitalPhase::initialize( int argc, char* argv[] )
{

    // =============================================
    // reading command line parameters
    // =============================================
    // read command line parameters
    int i = 1;
    while( i++ < argc )
    {
        string iTemp = argv[i - 1];
        string iTemp2 = "";
        if( i < argc )
        {
            iTemp2 = argv[i];
        }
        if( iTemp.find( "-help" ) < iTemp.size() )
        {
            printHelp();
            return false;
        }
        if( iTemp.find( "-input" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                finputfile = iTemp2;
                i++;
            }
        }
        else if( iTemp.find( "-output" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                foutputfile = iTemp2;
                i++;
            }
        }
        else if( iTemp.find( "-orbit" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                fOrbit = atof( iTemp2.c_str() );
                i++;
            }
        }
        else if( iTemp.find( "-refMJD" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                fRefMJD = atof( iTemp2.c_str() );
                i++;
            }
        }
    }
    
    // =============================================
    // end of reading command line parameters
    // =============================================
    
    // require inputfile name
    if( finputfile.size() == 0 )
    {
        cout << "error: no input file" << endl;
        cout << "...exiting" << endl;
        return false;
    }
    // require orbit
    if( fOrbit == 0 )
    {
        cout << "error: no binary orbit given" << endl;
        cout << "...exiting" << endl;
        return false;
    }
    // require refmjd
    if( fRefMJD == 0 )
    {
        cout << "error: no reference time given" << endl;
        cout << "...exiting" << endl;
        return false;
    }
    
    // set output file name
    if( foutputfile.size() == 0 )
    {
        if( finputfile.find( "*" ) < finputfile.size() )
        {
            foutputfile = "orb.root";
        }
        else
        {
            foutputfile = finputfile.substr( 0, finputfile.rfind( "." ) );
            foutputfile += ".orb.root";
        }
    }
    
    
    setInputFile();
    setOutputFile( "recreate" );
    
    return true;
}


////////////////////////////////////////////////////////////////////
// printing the orbital parameters

void VOrbitalPhase::printBinaryPars()
{
    cout << "Binary parameters:" << endl;
    cout << "  Orbit (days) " << fOrbit << endl;
    cout << "  Time of ascending node (mjd) " << fRefMJD << endl;
    cout << endl;
}

////////////////////////////////////////////////////////////////////
// printing the orbital parameters

void VOrbitalPhase::printHelp()
{
    cout << "command line parameters:" << endl;
    cout << "  -input    mscw file as input (required)"  << endl;
    cout << "  -orbit    orbit of the binary, in days (required)"  << endl;
    cout << "  -refMJD   time of ascenting node in MJD, reference time for phase=0 (required)"  << endl;
    cout << "  -output   output file,in general will be mscw.orb.root" << endl;
    cout << endl;
}

////////////////////////////////////////////////////////////////////
// opens input file and access the data tree

bool VOrbitalPhase::setInputFile()
{

    fInFile  = new TFile( finputfile.c_str() );
    if( fInFile->IsZombie() )
    {
        cout << "VOrbitalPhase::setInputFile error while opening output file " << finputfile << endl;
    }
    
    // get data tree
    fDataRunTree = ( TTree* )fInFile->Get( "data" );
    fDataRun = new CData( fDataRunTree );
    
    fNentries = fDataRun->fChain->GetEntries();
    
    cout.precision( 10 );
    
    return true;
}


////////////////////////////////////////////////////////////////////
// fills the new tree

void VOrbitalPhase::fill()
{

    for( int i = 0; i < fNentries; i++ )
    {
    
        fDataRun->GetEntry( i );
        
        double mjd_event = fDataRun->MJD + fDataRun->Time / 24. / 60. / 60.;
        calculatePhase( mjd_event );
        if( i % 10000 == 0 )
        {
            cout << mjd_event << " " << fOrbitalPhase <<  endl;
        }
        
        fOTree->Fill();
    }
    
}


////////////////////////////////////////////////////////////////////
// write everything to disk

bool VOrbitalPhase::terminate()
{

    if( fOutFile )
    {
        cout << "Writing events to " << fOutFile->GetName() << endl;
        fOutFile->cd();
        
        cout << endl << "\t total number of events in output tree: " << fOTree->GetEntries() << endl;
        
        if( fOTree->GetEntries() == fNentries )
        {
            fOTree->Write( "", TObject::kOverwrite );
        }
        else
        {
            cout << "\t error, different number of entries in input and output trees " << endl;
        }
        
        copyInputFile();
        
        fOutFile->Close();
        cout << "... outputfile closed" << endl;
    }
    
    
    return true;
}


////////////////////////////////////////////////////////////////////

bool VOrbitalPhase::setOutputFile( string iOption )
{

    if( fOutFile == fInFile && iOption == "recreate" )
    {
        cout << "VOrbitalPhase::setOutputFile error: can't overwrite inputfile" << endl;
        exit(-1 );
    }
    
    // open output file
    fOutFile = new TFile( foutputfile.c_str(), iOption.c_str() );
    if( fOutFile->IsZombie() )
    {
        cout << "VOrbitalPhase::setOutputFile error while opening output file " << foutputfile << "\t" << iOption << endl;
        
    }
    
    // define output tree
    char ITT[2000];
    sprintf( ITT, "Orbital Phase information" );
    
    fOTree = new TTree( "phase", ITT );
    fOTree->SetMaxTreeSize( 1000 * Long64_t( 2000000000 ) );
    
    fOTree->Branch( "orbit", &fOrbit, "orbit/D" );
    fOTree->Branch( "refMJD", &fRefMJD, "refMJD/D" );
    fOTree->Branch( "phase", &fOrbitalPhase, "phase/D" );
    
    return true;
}

////////////////////////////////////////////////////////////////////

void VOrbitalPhase::calculatePhase( double djm )
{

    double t0 = 2400000.5 + fRefMJD;
    
    //cout << "MJD: " << djm << "\t" << fixed <<  djm + 2400000.5 << endl;
    
    
    // calculate phase
    fOrbitalPhase = ( djm + 2400000.5 - t0 ) / fOrbit - ( int )(( djm + 2400000.5 - t0 ) / fOrbit );
    //  cout << "PHASE: " << phase << endl;
    
}


////////////////////////////////////////////////////////////////////

void VOrbitalPhase::copyInputFile()
{

    cout << "\t copying input file info " << endl;
    copyDirectory( fInFile );
    cout << "done" << endl;
    
}


////////////////////////////////////////////////////////////////////

void VOrbitalPhase::copyDirectory( TDirectory* source )
{

    //   from http://root.cern.ch/phpBB2/viewtopic.php?t=2789
    //   Author: Rene Brun
    TDirectory* savdir = gDirectory;
    TDirectory* adir;
    if( source == fInFile )
    {
        adir = savdir;
    }
    else
    {
        adir = savdir->mkdir( source->GetName() );
    }
    
    adir->cd();
    
    //loop on all entries of this directory
    TKey* key;
    TIter nextkey( source->GetListOfKeys() );
    while(( key = ( TKey* )nextkey() ) )
    {
        const char* classname = key->GetClassName();
        TClass* cl = gROOT->GetClass( classname );
        if(!cl )
        {
            continue;
        }
        if( cl->InheritsFrom( "TDirectory" ) )
        {
            source->cd( key->GetName() );
            TDirectory* subdir = gDirectory;
            adir->cd();
            copyDirectory( subdir );
            adir->cd();
        }
        else if( cl->InheritsFrom( "TTree" ) )
        {
            TTree* T = ( TTree* )source->Get( key->GetName() );
            adir->cd();
            TTree* newT = T->CloneTree();
            newT->Write();
        }
        else
        {
            source->cd();
            TObject* obj = key->ReadObj();
            adir->cd();
            obj->Write();
            delete obj;
        }
    }
    
    
}
