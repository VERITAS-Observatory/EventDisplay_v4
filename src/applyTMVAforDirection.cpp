#include <iostream>
#include <sstream>
#include <string>

#include "TFile.h"
#include "TTree.h"

#include "TMVA/Reader.h"

#include "VGlobalRunParameter.h"

using namespace std;


/*
 * Telescope-type training variables
 * (required to be the same as in training)
*/
const unsigned int n_tel_var = 12;
// Disp_T needs to be first! Sequence matters for flattening
static const vector< string > training_variables = {
    "Disp_T", "cen_x", "cen_y", "cosphi", "sinphi", "loss", "size", "dist",
    "width", "length", "asym", "tgrad_x"
};


void evaluate( string iInputFile, string iTMVAWeightsDir, string iOutputFile )
{
    const unsigned int n_tel_max = 4;

    TFile outputFile( iOutputFile.c_str(), "RECREATE" );
    TTree* outTree = new TTree(
        "DispDirection", "Direction reconstruction results based on BDT regression");
    float dir_Xoff;
    float dir_Yoff;
    outTree->Branch("Dir_Xoff", &dir_Xoff, "Dir_Xoff/F");
    outTree->Branch("Dir_Yoff", &dir_Yoff, "Dir_Yoff/F");

    // input data
    TFile inputFile(iInputFile.c_str(), "READ");
    if( inputFile.IsZombie() )
    {
        cout << "Error opening input file: " << iInputFile << endl;
        exit( EXIT_FAILURE );
    }
    TTree *data_tree = (TTree*)inputFile.Get("data");
    if( !data_tree )
    {
        cout << "Error: TTree 'data' not found in file: " << iInputFile << endl;
        exit( EXIT_FAILURE );
    }
    unsigned int DispNImages;
    unsigned int DispTelList_T[4];
    double Xoff_weighted_bdt;
    double Yoff_weighted_bdt;
    float Xoff_intersect;
    float Yoff_intersect;
    float arrBuf[n_tel_var][n_tel_max];
    for(unsigned v = 0; v < training_variables.size(); v++ )
    {
        data_tree->SetBranchAddress(training_variables[v].c_str(), arrBuf[v] );
    }
    data_tree->SetBranchAddress("DispNImages", &DispNImages);
    data_tree->SetBranchAddress("DispTelList_T", DispTelList_T);
    data_tree->SetBranchAddress("Xoff", &Xoff_weighted_bdt);
    data_tree->SetBranchAddress("Yoff", &Yoff_weighted_bdt);
    data_tree->SetBranchAddress("Xoff_intersect", &Xoff_intersect );
    data_tree->SetBranchAddress("Yoff_intersect", &Yoff_intersect );

    // init TMVA evaluator
    // (variables defined separately from data tree for flattening)
    float mva_arrBuf[n_tel_var][n_tel_max];
    float mva_disp_x[n_tel_max];
    float mva_disp_y[n_tel_max];
    float mva_Xoff_intersect;
    float mva_Yoff_intersect;
    float mva_Xoff_weighted_bdt;
    float mva_Yoff_weighted_bdt;

    cout << "Input file: " << iInputFile << endl;
    cout << "TMVA weights directory: " << iTMVAWeightsDir << endl;

    // - first dimension: X, Y, second dimension: n_tel = 2,3,4
    vector< vector< TMVA::Reader* > > fTMVAReader;
    for( unsigned int xy = 0; xy < 2; xy++ )
    {
        string coord = ( xy == 0 ) ? "X" : "Y";
        vector< TMVA::Reader* > iTMVAReader;
        for( unsigned int i = 2; i <= n_tel_max; i++ )
        {
            ostringstream weightFileName;
            weightFileName << iTMVAWeightsDir << "/DISPDir" << coord << "off_ntel" << i << "_BDT_" << coord << "off.weights.xml";
            cout << "  weight file " << weightFileName.str() << endl;
            TMVA::Reader* reader = new TMVA::Reader("!Color:!Silent");
            // Add variables to reader
            for( unsigned int v = 0; v < training_variables.size(); v++ )
            {
                for( unsigned int n = 0; n < i; n++ )
                {
                    ostringstream var;
                    var << training_variables[v] << "_" << n;
                    reader->AddVariable(var.str().c_str(), &mva_arrBuf[v][n]);
                    cout << "  var " << xy << "\t" << i << "\t" << var.str() << endl;
                }
            }
            ostringstream var_x;
            var_x << "disp_x_" << i;
            reader->AddVariable(var_x.str().c_str(), &mva_disp_x[i-2]);
            ostringstream var_y;
            var_y << "disp_y_" << i;
            reader->AddVariable(var_y.str().c_str(), &mva_disp_y[i-2]);
            if( xy == 0 )
            {
                reader->AddVariable("Xoff_weighted_bdt", &mva_Xoff_weighted_bdt);
                reader->AddVariable("Xoff_intersect", &mva_Xoff_intersect);
            }
            else
            {
                reader->AddVariable("Yoff_weighted_bdt", &mva_Yoff_weighted_bdt);
                reader->AddVariable("Yoff_intersect", &mva_Yoff_intersect);
            }
            ostringstream mva_name;
            mva_name << "BDT_" << xy << "\t" << i << endl;
            if( !reader->BookMVA(mva_name.str().c_str(), weightFileName.str().c_str()) )
            {
                cout << "Error: cannot find TMVA weight file: " << weightFileName.str() << endl;
                exit( EXIT_FAILURE );
            }
            iTMVAReader.push_back(reader);
        }
        fTMVAReader.push_back(iTMVAReader);
    }

    cout << "Evaluating TMVA for direction reconstruction..." << endl;

    Long64_t n = data_tree->GetEntries();
    cout << "Total number of entries in data: " << n << endl;
    for (Long64_t e = 0; e < n; ++e)
    {
        data_tree->GetEntry(e);

        mva_Xoff_intersect = Xoff_intersect;
        mva_Yoff_intersect = Yoff_intersect;
        mva_Xoff_weighted_bdt = (float)Xoff_weighted_bdt;
        mva_Yoff_weighted_bdt = (float)Yoff_weighted_bdt;

        if( DispNImages < 2 || DispNImages > n_tel_max )
        {
            cout << "Skipping event " << e << " with DispNImages = " << DispNImages << endl;
            continue;
        }
        // copy variables for TMVA evaluation
        for(unsigned v = 0; v < training_variables.size(); v++ )
        {
            for (unsigned int i = 0; i < DispNImages; i++)
            {
                unsigned int n_index = (v == 0) ? i : DispTelList_T[i];
                mva_arrBuf[v][i] = arrBuf[v][n_index];
            }
        }
        for (unsigned int i = 0; i < DispNImages; i++)
        {
            unsigned int n_index = DispTelList_T[i];
            mva_disp_x[i] = arrBuf[0][i] * arrBuf[3][n_index]; // Disp_T * cosphi
            mva_disp_y[i] = arrBuf[0][i] * arrBuf[4][n_index]; // Disp_T * sinphi
        }
        // evaluate TMVA
        dir_Xoff = fTMVAReader[0][DispNImages-2]->EvaluateMVA("BDT");
        dir_Yoff = fTMVAReader[1][DispNImages-2]->EvaluateMVA("BDT");

        outTree->Fill();
    }

    outputFile.cd();
    outTree->Write();
    outputFile.Close();

    cout << "Evaluation completed. Results saved to " << iOutputFile << endl;

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
            exit( EXIT_SUCCESS );
        }
    }
    cout << endl;

    cout << "applyTMVAforDirection (" << VGlobalRunParameter::getEVNDISP_VERSION() << ")" << endl;

    if( argc < 4 )
    {
        cout << "./applyTMVAforDirection <input mscw file> <TMVA weights directory> <output file> " << endl;
        cout << endl << endl;
        exit( EXIT_SUCCESS );
    }

    string fInputFile = argv[1];
    string fTMVAWeightsDir = argv[2];
    string fOutputFile = argv[3];

    evaluate( fInputFile, fTMVAWeightsDir, fOutputFile );

}
