/*! \file combineLookupTables
    \brief combine different lookup tablefiles into one tablefile

    Revision $Id$

    \author
    Gernot Maier
*/

#include "TClass.h"
#include "TDirectory.h"
#include "TDirectoryFile.h"
#include "TFile.h"
#include "TTree.h"
#include "TKey.h"
#include "TSystem.h"
#include "TROOT.h"

#include "VGlobalRunParameter.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void copyDirectory( TDirectory *source, char *hx = 0 );

vector< string > readListOfFiles( string iFile )
{
    vector< string > iList;

    ifstream is;
    is.open( iFile.c_str() );
    if( !is )
    {
        cout << "error while reading file list " << iFile << endl;
        cout << "exiting...." << endl;
        exit( 0 );
    }
    string is_line;

    while( getline( is, is_line ) )
    {
        iList.push_back( is_line );
    }

    is.close();

    return iList;
}


int main( int argc, char *argv[] )
{
    VGlobalRunParameter *iT = new VGlobalRunParameter();
    cout << endl;
    cout << "combineLookupTables (" << iT->getEVNDISP_VERSION() << ")" << endl;
    cout << "-----------------------------" << endl;
    cout << endl;

    if( argc < 2 )
    {
        cout << "combine several tables from different files into one single table file" << endl << endl;
        cout << "combineLookupTables <file with list of tables> <output file name>" << endl;
	cout << endl;
        exit( 0 );
    }
    string fListOfFiles = argv[1];
    string fOFile       = argv[2];

    vector< string > fInFiles = readListOfFiles( fListOfFiles );
    unsigned int nFiles = fInFiles.size();
    if( nFiles == 0 )
    {
        cout << "error: no files in file list" << endl;
        cout << "exiting...." << endl;
        exit( 0 );
    }
    cout << "combining " << nFiles << " table files into " << fOFile << endl;

    TFile *fROFile = new TFile( fOFile.c_str(), "UPDATE" );
    if( fROFile->IsZombie() )
    {
        cout << "error while opening combined file: " << fOFile << endl;
        exit( 0 );
    }
    TFile *fIn = 0;
    char hname[200];
    for( unsigned int f = 0; f < fInFiles.size(); f++ )
    {
        fIn = new TFile( fInFiles[f].c_str() );
        if( fIn->IsZombie() )
        {
            cout << "error while opening file: " << fInFiles[f] << endl;
            continue;
        }
        cout << "now reading file " << f << ": " << fInFiles[f] << endl;

        for( int r = 100; r < 99999; r++ )
        {
            sprintf( hname, "ze_%03d", r );
            if( fIn->Get( hname ) )
            {
                TDirectory *iSource = (TDirectory*)fIn->Get( hname );
                if( !iSource ) continue;
                fROFile->cd();
                cout << "\t copying " << hname << endl;
                copyDirectory( iSource );
            }
            else
            {
                sprintf( hname, "NOISE_%03d", r );
                if( fIn->Get( hname ) )
                {
                    TDirectory *iSource = (TDirectory*)fIn->Get( hname );
                    if( !iSource ) continue;
                    fROFile->cd();
                    sprintf( hname, "NOISE_%05d", r );
                    cout << "\t copying " << hname << "..." << endl;
                    copyDirectory( iSource, hname );
                    cout << "done" << endl;
                    continue;
                }
                sprintf( hname, "NOISE_%04d", r );
                if( fIn->Get( hname ) )
                {
                    TDirectory *iSource = (TDirectory*)fIn->Get( hname );
                    if( !iSource ) continue;
                    fROFile->cd();
                    sprintf( hname, "NOISE_%05d", r );
                    cout << "\t copying (4) " << hname << "..." << endl;
                    copyDirectory( iSource, hname );
                    cout << "done" << endl;
                    continue;
                }
                sprintf( hname, "NOISE_%05d", r );
                if( fIn->Get( hname ) )
                {
                    TDirectory *iSource = (TDirectory*)fIn->Get( hname );
                    if( !iSource ) continue;
                    fROFile->cd();
                    cout << "\t copying (5) " << hname << "..." << endl;
                    copyDirectory( iSource, 0 );
                    cout << "done" << endl;
                }
            }
        }
        fIn->Close();
    }

    fROFile->Close();
    cout << "finished..." << endl;
}


/*!
 *   from http://root.cern.ch/phpBB2/viewtopic.php?t=2789
 *
 *   Author: Rene Brun
 */
void copyDirectory( TDirectory *source, char *hx )
{
//copy all objects and subdirs of directory source as a subdir of the current directory
    TDirectory *savdir = gDirectory;
    TDirectory *adir = 0;

// 1. case: top directory exists
    if( hx ) adir = (TDirectory*)savdir->Get( hx );
    else     adir = (TDirectory*)savdir->Get( source->GetName() );

    if( !adir )
    {
// 2. case: make top directory
        if( hx ) adir = savdir->mkdir( hx );
        else     adir = savdir->mkdir(source->GetName());
        if( !adir )
        {
            cout << "error while creating directory " << source->GetName() << endl;
            cout << "exiting..." << endl;
            exit( 0 );
        }
    }
    adir->cd();
//loop on all entries of this directory
    TKey *key;
    TIter nextkey(source->GetListOfKeys());
    while ((key = (TKey*)nextkey()))
    {
        const char *classname = key->GetClassName();
        TClass *cl = gROOT->GetClass(classname);
        if (!cl) continue;
        if (cl->InheritsFrom("TDirectory"))
        {
            source->cd(key->GetName());
            TDirectory *subdir = gDirectory;
            adir->cd();
            copyDirectory(subdir);
            adir->cd();
        }
        else if (cl->InheritsFrom("TTree"))
        {
            TTree *T = (TTree*)source->Get(key->GetName());
            adir->cd();
            TTree *newT = T->CloneTree();
            newT->Write();
        }
        else
        {
            source->cd();
            TObject *obj = key->ReadObj();
            adir->cd();
            obj->Write();
            delete obj;
        }
    }
    adir->SaveSelf(kTRUE);
    savdir->cd();
}
