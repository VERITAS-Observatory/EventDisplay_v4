/*
 *
 * - fix number of training events; way more testing events
 * - several input files at different zenith and wobble offsets
 * - smearing to avoid issues with discrete values (wobble offsets?)
 * - different multiplicities
 * - can we write X and Y into the same file?
 * - can we write different multiplicities into the same file?
 *
 *
 */


#include "TFile.h"
#include "TCut.h"
#include "TTree.h"

#include "TMVA/DataLoader.h"
#include "TMVA/Factory.h"
#include "TMVA/Reader.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

static const vector< string > training_variables = {
    "Disp_T", "cen_x", "cen_y", "cosphi", "sinphi", "loss", "size", "dist",
    "width", "length", "asym", "tgrad_x"
};

void train(TTree* treeTrain, string outputFile, const  unsigned int n_tel = 4)
{
    vector< string > tmvaTarget;
    vector< string > tmvaTargetName;
    tmvaTarget.push_back( "MCxoff" );   tmvaTarget.push_back( "MCyoff" );
    tmvaTargetName.push_back( "Xoff" ); tmvaTargetName.push_back( "Yoff" );

    for(unsigned int t = 0; t < tmvaTarget.size(); t++ )
    {
        TFile* tmvaFile = new TFile( (outputFile + "_" + tmvaTargetName[t] + ".root").c_str(), "RECREATE" );
        TMVA::Factory factory(("DIR_" + tmvaTargetName[t]).c_str(), tmvaFile, "!V:!Silent:Color:DrawProgressBar:AnalysisType=Regression");

        TMVA::DataLoader loader(("DL_" + tmvaTargetName[t]).c_str());

        for( unsigned int v = 0; v < training_variables.size(); v++ )
        {
            for( unsigned int n = 0; n < n_tel; n++ )
            {
                ostringstream var;
                var << training_variables[v] << "_" << n;
                loader.AddVariable(var.str().c_str());
            }
        }

        loader.AddSpectator("MCe0");

        loader.AddTarget(tmvaTarget[t].c_str(), tmvaTargetName[t].c_str());

        loader.AddRegressionTree(treeTrain, 1.);
        loader.PrepareTrainingAndTestTree("","SplitMode=Random:NormMode=NumEvents");

        factory.BookMethod(&loader, TMVA::Types::kBDT, ("BDT_" + tmvaTargetName[t]).c_str(),
            "!V:NTrees=800:BoostType=Grad:Shrinkage=0.1:MaxDepth=4:MinNodeSize=1.0%");

        factory.TrainAllMethods();
        factory.TestAllMethods();
        factory.EvaluateAllMethods();
        factory.Delete();
        tmvaFile->Close();
        delete tmvaFile;
    }
}


string prepare(string inputFileName, string trainingFileName, const unsigned int n_tel = 4)
{
   const unsigned int n_tel_var = 12;

    TFile *input_file = new TFile(inputFileName.c_str());
    if( input_file->IsZombie() )
    {
        cout << "Error reading " << inputFileName << endl;
        exit( EXIT_FAILURE );
    }
    TTree *data_tree = (TTree*)input_file->Get("data");
    if( !data_tree )
    {
        cout << "Error reading data tree from " << inputFileName << endl;
        exit( EXIT_FAILURE );
    }

    float arrBuf[n_tel_var][n_tel];

    for(unsigned v = 0; v < training_variables.size(); v++ )
    {
        data_tree->SetBranchAddress(training_variables[v].c_str(), arrBuf[v] );
    }
    double MCxoff;
    double MCyoff;
    double MCe0;
    data_tree->SetBranchAddress("MCxoff", &MCxoff );
    data_tree->SetBranchAddress("MCyoff", &MCyoff );
    data_tree->SetBranchAddress("MCe0", &MCe0);

    ostringstream ofile_name;
    ofile_name << trainingFileName << "_ntel" << n_tel << ".root";
    TFile outFile(ofile_name.str().c_str(), "RECREATE" );
    TTree outTree("Training", "Flattened training tree");

    float outArr[n_tel_var][n_tel];
    for(unsigned v = 0; v < training_variables.size(); v++ )
    {
        for (unsigned int i = 0; i < n_tel; ++i)
        {
            string bname = training_variables[v] + "_" + std::to_string(i);
            outTree.Branch(bname.c_str(), &outArr[v][i]);
        }
    }
    outTree.Branch("MCxoff", &MCxoff);
    outTree.Branch("MCyoff", &MCyoff);
    outTree.Branch("MCe0", &MCe0);

    Long64_t n = data_tree->GetEntries();
    n = 10000;
    for (Long64_t e = 0; e < n; ++e)
    {
        data_tree->GetEntry(e);

        for (unsigned v = 0; v < n_tel_var; v++)
        {
            for (unsigned int i = 0; i < n_tel; i++)
            {
                outArr[v][i] = arrBuf[v][i];
            }
        }
        outTree.Fill();
    }

    outTree.Write();

    return ofile_name.str();
}

int main( int argc, char* argv[] )
{
    string inputFileName = "/lustre/fs23/group/veritas/IRFPRODUCTION/v492/AP/CARE_202404/V6_2016_2017_ATM61_gamma/MSCW_RECID0_DISP/30deg_0.5wob_NOISE200.mscw.root";
    string trainingFileName = "train_evnents";

    for(unsigned int i = 4; i <= 4; i++ )
    {
        string train_file_name = prepare(inputFileName, trainingFileName, i);

        TFile *input_file = new TFile( train_file_name.c_str() );
        TTree *data_tree = (TTree*)input_file->Get("Training");

        ostringstream output_file_name;
        output_file_name << "dir_bdt_ntel" << i << ".root";

        train(data_tree, output_file_name.str());
   }
}
