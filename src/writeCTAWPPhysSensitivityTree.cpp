/* \file writeCTAWPPhysSensitivityTree summary sensitivity for different sub arrays

   \author Gernot Maier
*/

#include "TFile.h"
#include "TH1F.h"
#include "TTree.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;

class VTelescopeData
{
    public:

    ULong64_t fTelType;
    string    fTelTypeName;
    float     fTel_x;
    float     fTel_y;

    VTelescopeData();
   ~VTelescopeData() {}
};

VTelescopeData::VTelescopeData()
{
    fTelType = 0;
    fTelTypeName = "";
    fTel_x = 0.;
    fTel_y = 0.;
};


class VSensitivityTree
{
     private:

     TFile *fOutputFile;
     TTree *fDataTree;

     //////////////////////////////////////
     // variables defining the array
     Char_t fArray[300];
     int fNTel;
     static const int fNumTelTypes = 5;
     int fNTelType[fNumTelTypes];                    // types of telescopes: 0 = LST, 1 = MST, 2 = 7m SST, 3 = 4m SST, 4 = SC-MST
     float fAverageTelescopeDistance[fNumTelTypes];  // average telescope distance to closest telescope

     ///////////////////////////////////////
     // variables defining the performance

     // 5 bins per decade, from -2 to 2 = 21
     static const int fNEnergyBins = 21;
     float fEnergy_logTeV[fNEnergyBins];      // mid point of log energy bin

     vector< string > fVarName;
     vector< float* > fVar;
     vector< float* > fVarError;

     void reset();

     public:

     VSensitivityTree();
    ~VSensitivityTree() {}
     bool fillEvent( string iSubArrayName );
     bool initialize( string iOutputFileName, string iName );
     bool terminate();
};

VSensitivityTree::VSensitivityTree()
{
   fOutputFile = 0;
   fDataTree = 0;

   // variable name
   fVarName.push_back( "DiffSens" );
   fVarName.push_back( "DiffSensCU" );
   fVarName.push_back( "IntSens" );
   fVarName.push_back( "IntSensCU" );
   fVarName.push_back( "AngRes" );
   fVarName.push_back( "AngRes80" );
   fVarName.push_back( "ERes" );
   fVarName.push_back( "Ebias" );
   fVarName.push_back( "BGRate" );
   fVarName.push_back( "ProtRate" );
   fVarName.push_back( "ElecRate" );
   fVarName.push_back( "BGRatePerSqDeg" );
   fVarName.push_back( "ProtRateSqDeg" );
   fVarName.push_back( "ElecRateSqDeg" );
   fVarName.push_back( "EffectiveArea" );
   fVarName.push_back( "EffectiveAreaEtrue" );
   fVarName.push_back( "EffectiveArea80" );

   for( unsigned int i = 0; i < fVarName.size(); i++ )
   {
       fVar.push_back( new float[fNEnergyBins] );
       fVarError.push_back( new float[fNEnergyBins] );
   }

   reset();
}

void VSensitivityTree::reset()
{
    sprintf( fArray, "noArrayName" );
    fNTel = 0;
    for( int i = 0; i < fNumTelTypes; i++ )
    {
       fNTelType[i] = 0;
       fAverageTelescopeDistance[i] = 0.;
    }

    for( int i = 0; i < fNEnergyBins; i++ )
    {
       fEnergy_logTeV[i] = 0.;
    }
    for( unsigned int v = 0; v < fVar.size(); v++ )
    {
        for( int i = 0; i < fNEnergyBins; i++ )
        {
            fVar[v][i] = 0.;
            fVarError[v][i] = 0.;
        }
    }
}

bool VSensitivityTree::fillEvent( string iSubArrayName )
{
    // this should go!
    string iTelescopePointingDirection = "_0deg";
    string iObservingTime = "180000s";

    reset();

    // array name
    sprintf( fArray, "%s", iSubArrayName.c_str() );

    char hname[800];
    //////////////////////////////////////////////// 
    // read telconfig tree from MSCW files
    sprintf( hname, "/lustre/fs9/group/cta/users/maierg/CTA/analysis/AnalysisData/prod2-LeoncitoPP-NS/%s", iSubArrayName.c_str() );
    sprintf( hname, "%s/Analysis-ID0-d20140309-onAxis/gamma_onSource.%s_ID0%s-prod2-LeoncitoPP-NS-1.mscw.root", hname, iSubArrayName.c_str(), iTelescopePointingDirection.c_str() );

    cout << "reading telconfig from " << hname << endl;

    TFile iT( hname );
    if( iT.IsZombie() )
    {
       cout << "Error reading file " << hname << endl;
       return false;
    }
    TTree *t = (TTree*)iT.Get( "telconfig" );
    if( !t )
    {
       cout << "Error telconfig tree from " << hname << endl;
       return false;
    }
    cout << "Telconfig tree found with " << t->GetEntries() << " telescopes" << endl;
    
    ULong64_t iTelType = 0;
    float iTelX = 0.; 
    float iTelY = 0.; 
    
    t->SetBranchAddress( "TelType", &iTelType );
    t->SetBranchAddress( "TelX", &iTelX );
    t->SetBranchAddress( "TelY", &iTelY );

    fNTel = t->GetEntries();
   
    // telescope data
    vector< vector< VTelescopeData* > > fTelescopeData;
    vector< VTelescopeData* > i_TempTD;
    for( int i = 0; i < fNumTelTypes; i++ )
    {
       fTelescopeData.push_back( i_TempTD );
    }

 
    // loop over all entries to count different telescope types
    for( int i = 0; i < t->GetEntries(); i++ )
    {   
                t->GetEntry( i );

                // LSTs (Type 0: 23m-LSTs)
                if( iTelType == 138704810 || iTelType == 141305009 || iTelType == 141305109 )
                {   
                        fNTelType[0]++; 
                        if( fTelescopeData.size() > 0 )
                        {
                            fTelescopeData[0].push_back( new VTelescopeData() );
                            fTelescopeData[0].back()->fTelTypeName = "23m-LST";
                            fTelescopeData[0].back()->fTel_x = iTelX;
                            fTelescopeData[0].back()->fTel_y = iTelY;
                        }
                }   
                // standard MSTs (type 1: 12m-MSTs)
                else if( iTelType == 10007818 || iTelType == 10408418 || iTelType == 10008118 )
                {
                        fNTelType[1]++; 
                        if( fTelescopeData.size() > 1 )
                        {
                            fTelescopeData[1].push_back( new VTelescopeData() );
                            fTelescopeData[1].back()->fTelTypeName = "12m-MST";
                            fTelescopeData[1].back()->fTel_x = iTelX;
                            fTelescopeData[1].back()->fTel_y = iTelY;
                        }
                }   
                // standard SSTs (type 2: 4m SSTs)
                else if( iTelType == 201509515 )
                {
                        fNTelType[2]++; 
                        if( fTelescopeData.size() > 2 )
                        {
                            fTelescopeData[2].push_back( new VTelescopeData() );
                            fTelescopeData[2].back()->fTelTypeName = "4m-SC-SST";
                            fTelescopeData[2].back()->fTel_x = iTelX;
                            fTelescopeData[2].back()->fTel_y = iTelY;
                        }
                }   
                // large SSTs (type 3: 7m SSTs)
                else if( iTelType == 3709725 || iTelType == 3709425 || iTelType == 3710125 )
                {
                        fNTelType[3]++; 
                        if( fTelescopeData.size() > 3 )
                        {
                            fTelescopeData[3].push_back( new VTelescopeData() );
                            fTelescopeData[3].back()->fTelTypeName = "7m-DC-SST";
                            fTelescopeData[3].back()->fTel_x = iTelX;
                            fTelescopeData[3].back()->fTel_y = iTelY;
                        }
                }   
                // SC-MSTs (type 4)
                else if( iTelType ==201509515 )
                {
                        fNTelType[4]++; 
                        if( fTelescopeData.size() > 4 )
                        {
                            fTelescopeData[4].push_back( new VTelescopeData() );
                            fTelescopeData[4].back()->fTelTypeName = "MST-SCT";
                            fTelescopeData[4].back()->fTel_x = iTelX;
                            fTelescopeData[4].back()->fTel_y = iTelY;
                        }
                }
                else
                {
                        cout << "unknown telescope type: " << iTelType << endl;
                }
    }
    iT.Close();

    ////////////////////////
    // calculate average distance to closest telescope (per telescope type)
    float i_dist = 0.;

    // loop over all telescope types
    for( unsigned int i = 0; i < fTelescopeData.size(); i++ )
    {
        float dist_av = 0.;
        float dist_N = 0.;
        for( unsigned int j = 0; j < fTelescopeData[i].size(); j++ )
        {
            float i_min = 1.e10;
            for( unsigned int k = 0; k < fTelescopeData[i].size(); k++ )
            {
                if( j == k ) 
                {
                   continue;
                }
                i_dist  = (fTelescopeData[i][j]->fTel_x-fTelescopeData[i][k]->fTel_x)*(fTelescopeData[i][j]->fTel_x-fTelescopeData[i][k]->fTel_x);
                i_dist += (fTelescopeData[i][j]->fTel_y-fTelescopeData[i][k]->fTel_y)*(fTelescopeData[i][j]->fTel_y-fTelescopeData[i][k]->fTel_y);
                i_dist  = sqrt( i_dist );
                if( i_dist < i_min )
                {
                    i_min = i_dist;
                }
            }
            if( i_min < 1.e9 && i_min > 0. )
            {
                dist_av += i_min;
                dist_N++;
            }
        }
        if( dist_N > 0. )
        {
           fAverageTelescopeDistance[i] = dist_av / dist_N;
        } 
    }


    //////////////////////////////////////////////// 
    // read sensitivities
    sprintf( hname, "/lustre/fs9/group/cta/users/maierg/CTA/analysis/WPPhys.LeoncitoPP_arrayStudy/" );
    sprintf( hname, "%s/DESY.d20140718.Erec1.L1.ID0%sNIM2.prod2-LeoncitoPP-NS.%s.%s.root", hname, iTelescopePointingDirection.c_str(), iSubArrayName.c_str(), iObservingTime.c_str() );

    cout << "reading IRFs from " << hname << endl;

    TFile iP( hname );
    if( iP.IsZombie() )
    {
       cout << "Error reading file " << hname << endl;
       return false;
    }


    TH1F *h = (TH1F*)iP.Get( "DiffSens" );
    if( h )
    {
        if( h->GetNbinsX() != fNEnergyBins )
        {
            cout << "error reading IRFs, different number of energy bins: " << fNEnergyBins << "\t" << h->GetNbinsX() << endl;
            return false;
        }
        for( int i = 0; i < h->GetNbinsX(); i++ )
        {
            fEnergy_logTeV[i] = h->GetBinCenter( i );
        }
    }
    for( unsigned int i = 0; i < fVarName.size(); i++ )
    {
         h = (TH1F*)iP.Get( fVarName[i].c_str() );
         if( !h )
         {
             cout << "error finding histogram " << fVarName[i] << endl;
             continue;
         }
         for( int e = 0; e < fNEnergyBins; e++ )
         {
             int i_bin = h->FindBin( fEnergy_logTeV[e] );
             fVar[i][e] = h->GetBinContent( i_bin );
             fVarError[i][e] = h->GetBinError( i_bin );
         }
    }
    iP.Close();

    //////////////////////////////////////////////// 
    // fill results
    fDataTree->Fill();

    return true;
}

/*
 *   open output file and initialize tree structure
 *
 */
bool VSensitivityTree::initialize( string iOutputFileName, string iName )
{
    fOutputFile = new TFile( iOutputFileName.c_str(), "RECREATE" );
    if( fOutputFile->IsZombie() )
    {
         cout << "VSensitivityTree::initialize error opening output file" << endl;
         return false;
    }

    // define data tree
    char hname[200];
    char htitle[200];

    fDataTree = new TTree( "IRFData", iName.c_str() );

    fDataTree->Branch( "Array", &fArray, "Array/C" );
    fDataTree->Branch( "NTel", &fNTel, "NTel/I" );
    sprintf( hname, "NTelType[%d]", fNumTelTypes );
    sprintf( htitle, "NTelType[%d]/I", fNumTelTypes );
    fDataTree->Branch( hname, fNTelType, htitle );

    sprintf( hname, "AverageTelescopeDistance[%d]", fNumTelTypes );
    sprintf( htitle, "AverageTelescopeDistance[%d]/F", fNumTelTypes );
    fDataTree->Branch( hname, fAverageTelescopeDistance, htitle );

    sprintf( hname, "Energy_logTeV[%d]", fNEnergyBins );
    sprintf( htitle, "Energy_logTeV[%d]/F", fNEnergyBins );
    fDataTree->Branch( hname, fEnergy_logTeV, htitle );

    for( unsigned int i = 0; i < fVar.size(); i++ )
    {
        sprintf( hname, "%s[%d]", fVarName[i].c_str(), fNEnergyBins );
        sprintf( htitle, "%s[%d]/F", fVarName[i].c_str(), fNEnergyBins );
        fDataTree->Branch( hname, fVar[i], htitle );

        sprintf( hname, "%sError[%d]", fVarName[i].c_str(), fNEnergyBins );
        sprintf( htitle, "%sError[%d]/F", fVarName[i].c_str(), fNEnergyBins );
        fDataTree->Branch( hname, fVarError[i], htitle );
    }

    return true;
}

bool VSensitivityTree::terminate()
{
   if( fOutputFile && fOutputFile->cd() )
   {
       if( fDataTree )
       {
           fDataTree->Write();
       }

       cout << "closing output file " << fOutputFile->GetName() << endl;
       fOutputFile->Close();
   }

   return true;
}


/////////////////////////////////////////////////////
// main
/////////////////////////////////////////////////////


int main( int argc, char* argv[] )
{
	/////////////////////
	// input parameters
	if( argc != 3 )
	{
		cout << endl;
		cout << "./writeCTAWPPhysSensitivityTree <list of sub array> <output file>" << endl;
		cout << endl;
		exit( 0 );
	}
        cout << endl;
        cout << "writeCTAWPPhysSensitivityTree"  << endl;
        cout << "-----------------------------" << endl;
        cout << endl;
	string fSubArrayList = argv[1];
        string fOutputfile = argv[2];


        // loop over all sub arrays and load files
        vector< string > fSubArray;

        ifstream is; 
        is.open( fSubArrayList.c_str(), ifstream::in );
        if( !is )
        {   
                cout << "error reading list of arrays from: " << endl;
                cout << fSubArrayList << endl;
                cout << "exiting..." << endl;
                exit( EXIT_FAILURE );
        }   
        cout << "reading list of arrays from " << endl;
        cout << fSubArrayList << endl;
        string iLine;
        while( getline( is, iLine ) ) 
        {   
                if( iLine.size() > 0 ) 
                {   
                        fSubArray.push_back( iLine );
                }   
        }   
        is.close();
    
        cout << "total number of subarrays: " << fSubArray.size() << endl;

        /////////////////////////////////////////
        // initialize and fill data tree
    
        VSensitivityTree *fData = new VSensitivityTree();
        if( !fData->initialize( fOutputfile, "test data" ) )
        {
           exit( EXIT_FAILURE );
        }

        for( unsigned int i = 0; i < fSubArray.size(); i++ )
        {
            cout << endl << endl;
            cout << "now filling" << fSubArray[i] << endl;
            cout << "=========================================" << endl;
            cout << endl;
            fData->fillEvent( fSubArray[i] );
        }

        fData->terminate();

        return true;
}

