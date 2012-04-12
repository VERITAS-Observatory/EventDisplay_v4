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
#include "TH2F.h"
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

void copyDirectory( TDirectory *source, const char *hx = 0 );

TH2F* reduceHistogramSize( TH2F *h = 0 );

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

    TFile *fROFile = new TFile( fOFile.c_str(), "RECREATE" );
    if( fROFile->IsZombie() )
    {
        cout << "error while opening combined file: " << fOFile << endl;
        exit( 0 );
    }
    TFile *fIn = 0;
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
       TKey *key;
       TIter nextkey( fIn->GetListOfKeys() );
       while( (key = (TKey*)nextkey()) )
       {
	   const char *classname = key->GetClassName();
	   TClass *cl = gROOT->GetClass(classname);
	   if (!cl) continue;
	   if ( cl->InheritsFrom("TDirectory") )
	   {
	      TDirectory *iSource = (TDirectory*)key->ReadObj();
	      if( !iSource ) continue;
	      fROFile->cd();
	      const char *hname = iSource->GetName();
	      cout << "\t copying " << hname << endl;
	      copyDirectory( iSource, hname );
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
void copyDirectory( TDirectory *source, const char *hx )
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
        else
        {
            source->cd();
            TObject *obj = key->ReadObj();
	    string iName = obj->GetName();
	    TH2F *hNew = 0;
	    if( iName.find( "median" ) != string::npos 
	       || iName.find( "Median" ) != string::npos )
	    {
	       hNew = reduceHistogramSize( (TH2F*)obj );
	       adir->cd();
//	       obj->Write();
	       if( hNew )
	       {
		  hNew->SetName( obj->GetName() );
	          hNew->Write();
               }
            }
            delete obj;
	    if( hNew ) delete hNew;
        }
    }
    adir->SaveSelf(kTRUE);
    savdir->cd();
}

TH2F* reduceHistogramSize( TH2F *h )
{
   if( !h ) return 0;

   bool bFilled = false;

   int nBinX_min = -1;
   int nBinX_max = -1;
   int nBinY_max = -1;

// get minimum x
   bFilled = false;
   for( int i = 1; i <= h->GetNbinsX(); i++ )
   {
      for( int j = 1; j <= h->GetNbinsY(); j++ )
      {
         if( h->GetBinContent( i, j ) > 0. )
	 {
	    bFilled = true;
	    break;
         }
      }
      if( bFilled )
      {
         nBinX_min = i;
	 break;
      }
   }
// get maximum x
   bFilled = false;
   for( int i = h->GetNbinsX(); i > 0; i-- )
   {
      for( int j = 1; j <= h->GetNbinsY(); j++ )
      {
         if( h->GetBinContent( i, j ) > 0. )
	 {
	    bFilled = true;
	    break;
         }
      }
      if( bFilled )
      {
         nBinX_max = i;
	 break;
      }
   }
// get maximum y
   bFilled = false;
   for( int i = h->GetNbinsY(); i > 0; i-- )
   {
      for( int j = 1; j <= h->GetNbinsX(); j++ )
      {
         if( h->GetBinContent( j, i ) > 0. )
	 {
	    bFilled = true;
	    break;
         }
      }
      if( bFilled )
      {
         nBinY_max = i;
	 break;
      }
   }
// create new histogram with reduced binning
   char hname[200];
   sprintf( hname, "new_%s", h->GetName() );

   float xmin = h->GetXaxis()->GetBinLowEdge( 1 );
   if( nBinX_min > 0 ) xmin = h->GetXaxis()->GetBinLowEdge( nBinX_min );
   float xmax = h->GetXaxis()->GetBinLowEdge( 1 ) +  h->GetXaxis()->GetBinWidth( 1 );
   if( nBinX_max > 0 ) xmax = h->GetXaxis()->GetBinLowEdge( nBinX_max ) + h->GetXaxis()->GetBinWidth( nBinX_max );
   float ymin = h->GetYaxis()->GetBinLowEdge( 1 );
   float ymax = h->GetYaxis()->GetBinLowEdge( 1 ) +  h->GetYaxis()->GetBinWidth( 1 );
   if( ymax > 0 ) ymax = h->GetYaxis()->GetBinLowEdge( nBinY_max ) + h->GetYaxis()->GetBinWidth( nBinY_max ); 	

   TH2F *hNew = new TH2F( hname, h->GetTitle(), nBinX_max - nBinX_min + 1, xmin, xmax, nBinY_max, ymin, ymax );
   hNew->Sumw2();

   for( int i = 1; i <= h->GetNbinsX(); i++ )
   {
       for( int j = 1; j <= h->GetNbinsY(); j++ )
       {
          if( h->GetBinContent( i, j ) > 0. )
	  {
	     hNew->SetBinContent( hNew->GetXaxis()->FindBin( h->GetXaxis()->GetBinCenter( i ) ), hNew->GetYaxis()->FindBin( h->GetYaxis()->GetBinCenter( j ) ), h->GetBinContent( i, j ) );
	     hNew->SetBinError(   hNew->GetXaxis()->FindBin( h->GetXaxis()->GetBinCenter( i ) ), hNew->GetYaxis()->FindBin( h->GetYaxis()->GetBinCenter( j ) ), h->GetBinError( i, j ) );
          }
       }
   }

   return hNew;
}
