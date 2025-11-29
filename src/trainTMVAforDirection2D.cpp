/* \file trainTMVAforDirection.cpp
   \brief use TMVA methods for direction reconstruction

   Calculate direction (Xoff, Yoff) in camera coordinates using MLP regression and
   the following input:

   - Xoff/Yoff from weighted average of single-telescope disp predictions (VDispAnalyzer)
   - Xoff/Yoff from intersection of image axes (VDispAnalyzer)
   - telescope specific image parameters

   Separate training for 2, 3, and 4 telescope events.
 */


#include "TChain.h"
#include "TCut.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom3.h"

#include "TMVA/Config.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Factory.h"
#include "TMVA/Reader.h"

#include <iostream>
#include <sstream>
#include <string>

#include "VGlobalRunParameter.h"

using namespace std;

/*
 * Telescope-type training variables
*/
const unsigned int n_tel_var = 12;
// Disp_T needs to be first! Sequence matters for flattening
static const vector< string > training_variables = {
    "Disp_T", "cen_x", "cen_y", "cosphi", "sinphi", "loss", "size", "dist",
    "width", "length", "asym", "tgrad_x"
};

/*
 * Train TMVA MLPs for direction reconstruction
*/
void train(TTree* trainingTree, TTree* testingTree, TFile* tmvaFile, string iOutputDir, const unsigned int n_tel = 4)
{
    vector< string > tmvaTarget;
    vector< string > tmvaTargetName;
    tmvaTarget.push_back( "MCxoff" );   tmvaTarget.push_back( "MCyoff" );
    tmvaTargetName.push_back( "Xoff" ); tmvaTargetName.push_back( "Yoff" );

    // set output directory
    gSystem->mkdir( iOutputDir.c_str() );
    TString iOutputDirectory( iOutputDir.c_str() );
    gSystem->ExpandPathName( iOutputDirectory );
    ( TMVA::gConfig().GetIONames() ).fWeightFileDir = iOutputDirectory;

    // MLP
    ostringstream fac_name;
    fac_name << "DISPDirMLP_ntel" << n_tel;
    TMVA::Factory factory(
        fac_name.str().c_str(),
        tmvaFile,
        "!V:!Silent:Color:DrawProgressBar:AnalysisType=Regression");

    factory.SetVerbose( true );
    TMVA::DataLoader loader(fac_name.str().c_str());

    for( unsigned int v = 0; v < training_variables.size(); v++ )
    {
        for( unsigned int n = 0; n < n_tel; n++ )
        {
            ostringstream var;
            var << training_variables[v] << "_" << n;
            loader.AddVariable(var.str().c_str());
        }
    }
    for( unsigned int n = 0; n < n_tel; n++ )
    {
        ostringstream var;
        var << "disp_x_" << n;
        loader.AddVariable(var.str().c_str());
        var.str("");
        var << "disp_y_" << n;
        loader.AddVariable(var.str().c_str());
    }
    loader.AddVariable("Xoff_weighted_bdt");
    loader.AddVariable("Yoff_weighted_bdt");
    loader.AddVariable("Xoff_intersect");
    loader.AddVariable("Yoff_intersect");

    loader.AddSpectator("MCe0");

    loader.AddTarget("MCxoff", "Xoff_mva");
    loader.AddTarget("MCyoff", "Yoff_mva");

    loader.AddRegressionTree(trainingTree, 1., TMVA::Types::kTraining);
    loader.AddRegressionTree(testingTree, 1., TMVA::Types::kTesting);
    loader.PrepareTrainingAndTestTree("", "");

    unsigned N = loader.GetNVariables();
    unsigned h1  = 2 * N;
    unsigned h2  = N / 2;
    std::ostringstream hl;
    hl << h1 << "," << h2;
    string TMVAOptions = "!V:NCycles=600:HiddenLayers=" + hl.str() + ":NeuronType=tanh:VarTransform=Norm";
    cout << "Training TMVA MLPs with options: " << TMVAOptions << endl;

    factory.BookMethod(&loader, TMVA::Types::kMLP, "MLP_MultiTarget", TMVAOptions.c_str());

    factory.TrainAllMethods();
    factory.TestAllMethods();
    factory.EvaluateAllMethods();
    factory.Delete();
}

/*
   Prepare training data: flatten tel. type variables into fixed number of telescopes

   Note that Disp_T is an array with max length of number of images used for disp,
   while all other array variables are of length number of telescopes in the array.

*/
string prepare(vector<string> inputFiles, string trainingFileName, float trainTestFraction, unsigned int maxEvents, const unsigned int n_tel = 4)
{
    TChain data_tree("data");
    for(unsigned int f = 0; f < inputFiles.size(); f++ )
    {
        cout << "Adding file to training data: " << inputFiles[f] << endl;
        data_tree.Add(inputFiles[f].c_str());
    }

    const unsigned int n_tel_max = 4;
    float arrBuf[n_tel_var][n_tel_max];

    for(unsigned v = 0; v < training_variables.size(); v++ )
    {
        data_tree.SetBranchAddress(training_variables[v].c_str(), arrBuf[v] );
    }
    double MCxoff;
    double MCyoff;
    double MCe0;
    double Xoff;
    double Yoff;
    float Xoff_intersect;
    float Yoff_intersect;
    unsigned int DispNImages;
    unsigned int DispTelList_T[4];
    // BDT Xoff/Yoff result from averaging over all telescopes (in VDispAnalyzer)
    data_tree.SetBranchAddress("Xoff", &Xoff);
    data_tree.SetBranchAddress("Yoff", &Yoff);
    // Intersection method results
    data_tree.SetBranchAddress("Xoff_intersect", &Xoff_intersect );
    data_tree.SetBranchAddress("Yoff_intersect", &Yoff_intersect );
    data_tree.SetBranchAddress("MCe0", &MCe0);
    data_tree.SetBranchAddress("DispNImages", &DispNImages);
    data_tree.SetBranchAddress("DispTelList_T", DispTelList_T);
    // target variables
    data_tree.SetBranchAddress("MCxoff", &MCxoff );
    data_tree.SetBranchAddress("MCyoff", &MCyoff );

    ostringstream ofile_name;
    ofile_name << trainingFileName << "_ntel" << n_tel << ".root";
    TFile outFile(ofile_name.str().c_str(), "RECREATE" );

    // two trees for training (index 0) and testing (index 1)
    vector<TTree*> outTrees;
    float outArr[n_tel_var][n_tel];
    float disp_x[n_tel];
    float disp_y[n_tel];
    bool TrainingEvent;
    for(unsigned int i = 0; i < 2; i++ )
    {
        TTree* outTree = 0;
        if( i == 0 ) outTree = new TTree("Training", "Flattened training tree");
        else         outTree = new TTree("Testing", "Flattened testing tree");
        outTrees.push_back(outTree);
        for(unsigned v = 0; v < training_variables.size(); v++ )
        {
            for (unsigned int t = 0; t < n_tel; ++t)
            {
                string bname = training_variables[v] + "_" + std::to_string(t);
                outTrees.back()->Branch(bname.c_str(), &outArr[v][t]);
            }
        }
        for (unsigned int t = 0; t < n_tel; ++t)
        {
            outTrees.back()->Branch(("disp_x_" + std::to_string(t)).c_str(), &disp_x[t]);
            outTrees.back()->Branch(("disp_y_" + std::to_string(t)).c_str(), &disp_y[t]);
        }

        outTrees.back()->Branch("Xoff_weighted_bdt", &Xoff);
        outTrees.back()->Branch("Yoff_weighted_bdt", &Yoff);
        outTrees.back()->Branch("Xoff_intersect", &Xoff_intersect);
        outTrees.back()->Branch("Yoff_intersect", &Yoff_intersect);
        outTrees.back()->Branch("MCxoff", &MCxoff);
        outTrees.back()->Branch("MCyoff", &MCyoff);
        outTrees.back()->Branch("MCe0", &MCe0);
    }

    // Random number generator for train/test split
    TRandom3 rand(0);
    unsigned int trainingEvents = 0;
    unsigned int testingEvents = 0;
    // Balanced training configuration over training files
    // (approximate: assumes equal number of events per file)
    unsigned int maxTrainingEventsPerFile = maxEvents / inputFiles.size() * trainTestFraction;
    unsigned int maxTestingEventsPerFile = maxEvents / inputFiles.size() * (1.0 - trainTestFraction);
    int fileIndex = 0;

    unsigned int n_index = 0;
    Long64_t n = data_tree.GetEntries();
    cout << "Total number of entries in training data (before cuts): " << n << endl;
    vector<unsigned int> perFileTrainingCounts(inputFiles.size(), 0);
    vector<unsigned int> perFileTestingCounts(inputFiles.size(), 0);
    for (Long64_t e = 0; e < n; ++e)
    {
        data_tree.GetEntry(e);

        fileIndex = data_tree.GetTreeNumber();
        if( fileIndex < 0 || fileIndex >= (int)inputFiles.size() ) fileIndex = 0;
        TrainingEvent = (rand.Rndm() < trainTestFraction) && (perFileTrainingCounts[fileIndex] < maxTrainingEventsPerFile);
        if ( TrainingEvent && (perFileTrainingCounts[fileIndex] > maxTrainingEventsPerFile) ) continue;
        if ( !TrainingEvent && (perFileTestingCounts[fileIndex] > maxTestingEventsPerFile) ) continue;

        if( DispNImages != n_tel ) continue;

        for (unsigned v = 0; v < n_tel_var; v++)
        {
            for (unsigned int i = 0; i < DispNImages; i++)
            {
                if(v == 0) // Disp_T
                {
                    n_index = i; // for Disp_T, use index of telescope in flattened array
                }
                else
                {
                    n_index = DispTelList_T[i];
                }
                outArr[v][i] = arrBuf[v][n_index];
            }
        }

        for (unsigned int i = 0; i < DispNImages; i++)
        {
            disp_x[i] = outArr[0][i] * outArr[3][i]; // Disp_T * cosphi
            disp_y[i] = outArr[0][i] * outArr[4][i]; // Disp_T * sinphi
        }

        if( TrainingEvent )
        {
            trainingEvents++;
            perFileTrainingCounts[fileIndex]++;
            outTrees[0]->Fill();
        }
        else
        {
            testingEvents++;
            perFileTestingCounts[fileIndex]++;
            outTrees[1]->Fill();
        }

        // Skip to next file if both training and testing quotas are met
        if (perFileTrainingCounts[fileIndex] >= maxTrainingEventsPerFile &&
            perFileTestingCounts[fileIndex] >= maxTestingEventsPerFile)
        {
            while (e + 1 < n && data_tree.GetTreeNumber() == fileIndex) {
                ++e;
                data_tree.LoadTree(e);
            }
            continue;
        }

    }
    for(unsigned int i = 0; i < 2; i++ )
    {
        outTrees[i]->Write();
        delete outTrees[i];
    }

    cout << "Total training events: " << trainingEvents << ", testing events: " << testingEvents << endl;
    cout << "Per-file training and testing event counts:" << endl;
    for(size_t i = 0; i < inputFiles.size(); ++i) {
        cout << "  [" << i << "] " << inputFiles[i] << " -> " << perFileTrainingCounts[i] << " / " << perFileTestingCounts[i] << endl;
    }

    return ofile_name.str();
}

/*
 * read an ascii file with a list of mscw files
 * return a vector with all the files
*/
vector< string > fillInputFiles_fromList( string iList )
{
    vector< string > inputfile;

    ifstream is;
    is.open( iList.c_str(), ifstream::in );
    if(!is )
    {
        cout << "fillInputFiles_fromList() error reading list of input files: " << endl;
        cout << iList << endl << "exiting..." << endl;
        exit( EXIT_FAILURE );
    }
    cout << "Reading input file list: " << iList << endl;
    string iLine;
    while( getline( is, iLine ) )
    {
        if( iLine.size() > 0 )
            inputfile.push_back( iLine );
    }
    is.close();
    cout << "total number of input files " << inputfile.size() << endl;
    return inputfile;
}

int main( int argc, char* argv[] )
{
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
    if( argc < 6 )
    {
        cout << "./trainTMVAforDirection <list of input eventdisplay files (MC)> <output directory>" << endl;
        cout << "                        <training file name> <train vs test fraction> <max. number of events>" << endl;
        cout << "                        [TMVA options (optional)]" << endl;
        cout << endl;
        cout << "example: ./trainTMVAforDirection files.txt ./ train_events 0.5 100000" << endl;
        cout << endl;
        exit( EXIT_SUCCESS );
    }
    string inputFileList = argv[1];
    string outputDir = argv[2];
    string trainingFileName = argv[3];
    float trainTestFraction = atof(argv[4]);
    unsigned int max_events = atoi(argv[5]);
    if( argc >= 7 )
    {
        TMVAOptions = argv[6];
    }

    cout << "trainTMVAforDirection (" << VGlobalRunParameter::getEVNDISP_VERSION() << ")" << endl;
    cout << "------------------------------------" << endl;
    cout << endl;
    vector<string> inputFiles = fillInputFiles_fromList(inputFileList);
    cout << "Output directory for TMVA weight files: " << outputDir << endl;
    cout << "Train vs test fraction: " << trainTestFraction << ", max. events: " << max_events << endl;
    cout << endl;

    // Train separate MLPs for 2, 3, and 4 telescope multiplicity
    for(unsigned int i = 2; i <= 4; i++ )
    {
        cout << "Preparing training data for n_tel = " << i << endl;
        string train_file_name = prepare(
            inputFiles, outputDir + "/" + trainingFileName, trainTestFraction, max_events, i);

        TFile *input_file = new TFile( train_file_name.c_str() );
        TTree *training_tree = (TTree*)input_file->Get("Training");
        TTree *testing_tree = (TTree*)input_file->Get("Testing");

        string output_file = outputDir + "/dirMLPs_ntel" + to_string(i) + ".root";
        TFile* tmvaFile = new TFile((output_file).c_str(), "RECREATE");

        train(training_tree, testing_tree, tmvaFile, outputDir, i);

        tmvaFile->Close();
        delete tmvaFile;

        input_file->Close();
        delete input_file;
   }
}
