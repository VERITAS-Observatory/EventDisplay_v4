/*! \file combineEffectiveAreas.cpp
    \brief combine effective areas


    \author Gernot Maier
*/

#include "TChain.h"
#include "TFile.h"
#include "TH1D.h"
#include "TMath.h"
#include "TTree.h"

#include <iostream>
#include <stdlib.h>

#include <VGlobalRunParameter.h>

using namespace std;


void merge( char *ifile, char *outputfile, bool bFull = false )
{
    char hname[400];

    TChain f( "fEffArea" );
    sprintf( hname, "%s.root", ifile );
    f.Add( hname );
    f.ls();
    sprintf( hname, "%s.root", outputfile );
    cout << "merge to " << hname << endl;
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
        f.SetBranchStatus( "seff_L", 1 );
        f.SetBranchStatus( "seff_U", 1 );
        f.SetBranchStatus( "Rec_nbins", 1 );
        f.SetBranchStatus( "Rec_e0", 1 );
        f.SetBranchStatus( "Rec_eff", 1 );
        f.SetBranchStatus( "Rec_seff_L", 1 );
        f.SetBranchStatus( "Rec_seff_U", 1 );
// needed for compatibility to v3.30
        f.SetBranchStatus( "hEmc", 1 );
// needed for systematic error calculation
        f.SetBranchStatus( "hEsysMCRelative", 1 );
    }
    f.Merge( hname );

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

    sprintf( hname, "cat %s*.log > %s.log", ifile, outputfile );
    system( hname );
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
   cout << "combineEffectiveAreas (v" << VGlobalRunParameter::getEVNDISP_VERSION() << ")" << endl;
   cout << endl;

   merge( argv[1], argv[2], (bool)atoi(argv[3]) );

}


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
