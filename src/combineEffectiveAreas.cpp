/*! \file combineEffectiveAreas.cpp
    \brief combine effective areas calculated for a one combination of ze,az,woff... into a single large file


    \author Gernot Maier
*/

#include "TChain.h"
#include "TFile.h"
#include "TH1D.h"
#include "TMath.h"
#include "TTree.h"

#include <iostream>
#include <stdlib.h>
#include <string>

#include <VGlobalRunParameter.h>

using namespace std;


void merge( string ifile, char *outputfile, bool bFull = false )
{
    char hname[2000];

    TChain f( "fEffArea" );
    if( ifile.find( ".root" ) != string::npos ) sprintf( hname, "%s", ifile.c_str() );
    else                                        sprintf( hname, "%s.root", ifile.c_str() );
    int i_nMerged = f.Add( hname );
    if( i_nMerged == 0 )
    {
       cout << "error: no files found to merge: " << endl;
       cout << "\t" << hname << endl;
       cout << "exiting.." << endl;
       exit( -1 );
    }
    sprintf( hname, "%s.root", outputfile );
    cout << "merging " << i_nMerged << " files to " << hname << endl;

// set branches to be included in merged files
    if( !bFull )
    {
        f.SetBranchStatus( "*", 0 );
        f.SetBranchStatus( "ze", 1 );
        f.SetBranchStatus( "az", 1 );
        f.SetBranchStatus( "azMin", 1 );
        f.SetBranchStatus( "azMax", 1 );
        f.SetBranchStatus( "Xoff", 1 );
        f.SetBranchStatus( "Yoff", 1 );
        f.SetBranchStatus( "Woff", 1 );
        f.SetBranchStatus( "noise", 1 );
        f.SetBranchStatus( "pedvar", 1 );
        f.SetBranchStatus( "index", 1 );
        f.SetBranchStatus( "nbins", 1 );
        f.SetBranchStatus( "e0", 1 );
        f.SetBranchStatus( "eff", 1 );
// errors not needed for standard analysis
//        f.SetBranchStatus( "seff_L", 1 );
//        f.SetBranchStatus( "seff_U", 1 );
        f.SetBranchStatus( "Rec_nbins", 1 );
        f.SetBranchStatus( "Rec_e0", 1 );
        f.SetBranchStatus( "Rec_eff", 1 );
// errors not needed for standard analysis
//        f.SetBranchStatus( "Rec_seff_L", 1 );
//        f.SetBranchStatus( "Rec_seff_U", 1 );
// needed for compatibility to v3.30
//        f.SetBranchStatus( "hEmc", 1 );
// needed for systematic error calculation
        f.SetBranchStatus( "hEsysMCRelative", 1 );
    }
    f.Merge( hname );
    cout << "done.." << endl;

// get one example of hEmc
// (this is needed later to get the binning right)
    TFile *fO = new TFile( hname, "UPDATE" );
    if( fO->IsZombie() )
    {
        cout << "error writing hEmc to output file" << endl;
        return;
    }
    TH1D *hEmc = 0;
    f.SetBranchAddress( "hEmc", &hEmc );
    f.GetEntry( 0 );
    fO->cd();
    if( hEmc )
    {
        hEmc->Reset();
        hEmc->Write();
    }
    fO->Close();

// merge all log files
    if( ifile.find( ".root" ) != string::npos )
    {
       sprintf( hname, "cat %s*.log > %s.log", ifile.substr( ifile.size()-5, ifile.size() ).c_str(), outputfile );
    }
    else
    {
       sprintf( hname, "cat %s*.log > %s.log", ifile.c_str(), outputfile );
    }
    cout << "merge log files into " << hname << endl;
    system( hname );
    cout << "done..";
}

int main( int argc, char *argv[] )
{
   if( argc < 4 )
   {
       cout << endl;
       cout << "combineEffectiveAreas <effective area files> <combined file> <write all histograms (default=false)>" << endl;
       cout << endl;
       cout << "   <effective area files>    without .root suffix (e.g. effArea*. Note need of \"...\")" << endl;
       cout << endl;
       exit( 0 );
   }
   cout << endl;
   cout << "combineEffectiveAreas (" << VGlobalRunParameter::getEVNDISP_VERSION() << ")" << endl;
   cout << "------------------------------------" << endl;
   cout << endl;

   merge( argv[1], argv[2], (bool)atoi(argv[3]) );

   cout << endl << "end combineEffectiveAreas" << endl;

}

/*

   the following function is not used

*/
void mergeSelectedEvents( char *ifile, char *outfile, double ze = -1, int az = -1, double Woff = -1, int noise = -1, double index = -1. )
{
    char hname[400];

    TChain f( "fEffArea" );
    sprintf( hname, "%s*.root", ifile );
    f.Add( hname );
    cout << "total number of entries in input tree: " << f.GetEntries() << endl;

    double fze = 0;
    int faz = 0;
    double fwoff = 0.;
    int fnoise = 0;
    double findex = 0.;

    f.SetBranchAddress("ze", &fze );
    f.SetBranchAddress("az", &faz );
    f.SetBranchAddress("Woff", &fwoff );
    f.SetBranchAddress("noise", &fnoise );
    f.SetBranchAddress("index", &findex );

// output file
    sprintf( hname, "%s.root", outfile );
    TFile *ofile = new TFile( hname, "RECREATE" );
    TTree *o = (TTree*)f.CloneTree( 0 );

    for( int i = 0; i < f.GetEntries(); i++ )
    {
        f.GetEntry( i );

        if( ze > 0 && TMath::Abs( ze - fze ) > 0.01 ) continue;
        if( az > 0 && az != faz ) continue;
        if( Woff > 0.1 && TMath::Abs( Woff - fwoff ) > 0.01 ) continue;
        if( noise > 0 && noise != fnoise ) continue;
        if( index > 0 && TMath::Abs( index - findex ) > 0.01 ) continue;

        cout << "fill " << fze << " " << faz << " " << fwoff << " " << fnoise << " " << findex << endl;

        o->Fill();
    }
    cout << "total number of entries in output tree: " << o->GetEntries() << endl;

    o->Write();
    ofile->Close();

}
