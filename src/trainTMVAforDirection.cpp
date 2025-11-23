/*
 *
 */


#include "TChain.h"
#include "TCut.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom3.h"

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
static const vector< string > training_variables = {
    "Disp_T", "cen_x", "cen_y", "cosphi", "sinphi", "loss", "size", "dist",
    "width", "length", "asym", "tgrad_x"
};

/*
 * Train TMVA BDTs for direction reconstruction
*/
void train(TTree* treeTrain, TFile* tmvaFile, string TMVAOptions, const  unsigned int n_tel = 4)
{
    vector< string > tmvaTarget;
    vector< string > tmvaTargetName;
    tmvaTarget.push_back( "MCxoff" );   tmvaTarget.push_back( "MCyoff" );
    tmvaTargetName.push_back( "Xoff" ); tmvaTargetName.push_back( "Yoff" );

    for(unsigned int t = 0; t < tmvaTarget.size(); t++ )
    {
        ostringstream fac_name;
        fac_name << "DISPDir" << tmvaTargetName[t] << "_ntel" << n_tel;
        TMVA::Factory factory(
            fac_name.str().c_str(),
            tmvaFile,
            "!V:!Silent:Color:DrawProgressBar:AnalysisType=Regression");

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
        loader.AddSpectator("MCe0");

        loader.AddTarget(tmvaTarget[t].c_str(), tmvaTargetName[t].c_str());

        // Create separate trees for training and testing based on TrainingEvent variable
        TTree* trainTree = treeTrain->CopyTree("TrainingEvent==1");
        TTree* testTree  = treeTrain->CopyTree("TrainingEvent==0");
        // Setting directory to 0 keeps them in memory only and avoids writing them implicitly.
        if( trainTree ) trainTree->SetDirectory( nullptr );
        if( testTree )  testTree->SetDirectory( nullptr );

        loader.AddRegressionTree(trainTree, 1., TMVA::Types::kTraining);
        loader.AddRegressionTree(testTree, 1., TMVA::Types::kTesting);
        loader.PrepareTrainingAndTestTree("", "NormMode=NumEvents");

        factory.BookMethod(&loader, TMVA::Types::kBDT, ("BDT_" + tmvaTargetName[t]).c_str(), TMVAOptions.c_str());

        factory.TrainAllMethods();
        factory.TestAllMethods();
        factory.EvaluateAllMethods();
        factory.Delete();

        delete trainTree;
        delete testTree;
    }
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

    float arrBuf[n_tel_var][n_tel];

    for(unsigned v = 0; v < training_variables.size(); v++ )
    {
        data_tree.SetBranchAddress(training_variables[v].c_str(), arrBuf[v] );
    }
    double MCxoff;
    double MCyoff;
    double MCe0;
    unsigned int DispNImages;
    unsigned int DispTelList_T[4];
    data_tree.SetBranchAddress("MCxoff", &MCxoff );
    data_tree.SetBranchAddress("MCyoff", &MCyoff );
    data_tree.SetBranchAddress("MCe0", &MCe0);
    data_tree.SetBranchAddress("DispNImages", &DispNImages);
    data_tree.SetBranchAddress("DispTelList_T", DispTelList_T);

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

    // Add TrainingEvent variable for explicit train/test split
    bool TrainingEvent;
    outTree.Branch("TrainingEvent", &TrainingEvent);

    // Random number generator for train/test split
    TRandom3 rand(0);
    unsigned int trainingEvents = 0;
    unsigned int testingEvents = 0;
    // Balanced training configuration over training files
    // (approximate: assumes equal distribution of events per file)
    unsigned int maxTrainingEventsPerFile = maxEvents / inputFiles.size() * trainTestFraction;
    unsigned int maxTestingEventsPerFile = maxEvents / inputFiles.size() * (1.0 - trainTestFraction);
    int fileIndex = 0;

    unsigned int n_index = 0;
    Long64_t n = data_tree.GetEntries();
//    n = 1000;
    cout << "Total number of entries in training data (before cuts): " << n << endl;
    vector<unsigned int> perFileTrainingCounts(inputFiles.size(), 0);
    vector<unsigned int> perFileTestingCounts(inputFiles.size(), 0);
    for (Long64_t e = 0; e < n; ++e)
    {
        data_tree.GetEntry(e);
        // ensure balanced training set if requested (this is inefficient for small number of requested training events)
        fileIndex = data_tree.GetTreeNumber();
        if( fileIndex < 0 || fileIndex >= (int)inputFiles.size() ) fileIndex = 0;
        TrainingEvent = (rand.Rndm() < trainTestFraction) && (perFileTrainingCounts[fileIndex] < maxTrainingEventsPerFile);
        if ( TrainingEvent && (perFileTrainingCounts[fileIndex] > maxTrainingEventsPerFile) ) continue;
        if ( !TrainingEvent && (perFileTestingCounts[fileIndex] > maxTestingEventsPerFile) ) continue;

        if( DispNImages != n_tel ) continue;


        for (unsigned v = 0; v < n_tel_var; v++)
        {
            for (unsigned int i = 0; i < n_tel; i++)
            {
                n_index = DispTelList_T[i];
                outArr[v][i] = arrBuf[v][n_index];
            }
        }

        if( TrainingEvent ) {
            trainingEvents++;
            perFileTrainingCounts[fileIndex]++;
        } else {
            testingEvents++;
            perFileTestingCounts[fileIndex]++;
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

        outTree.Fill();
    }

    outTree.Write();

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
    string inputFileList="files.txt";
    string TMVAOptions="!V:NTrees=800:BoostType=Grad:Shrinkage=0.1:MaxDepth=4:MinNodeSize=1.0%";
    string trainingFileName = "train_events";
    float trainTestFraction = 0.5;
    unsigned int max_events = 10000;
    string outputFile = "dir_bdt.root";

    vector<string> inputFiles = fillInputFiles_fromList(inputFileList);
    TFile* tmvaFile = new TFile(outputFile.c_str(), "RECREATE");

    for(unsigned int i = 2; i <= 4; i++ )
    {
        cout << "Preparing training data for n_tel = " << i << endl;
        string train_file_name = prepare(
            inputFiles, trainingFileName, trainTestFraction, max_events, i);

        TFile *input_file = new TFile( train_file_name.c_str() );
        TTree *data_tree = (TTree*)input_file->Get("Training");

        tmvaFile->cd();
        train(data_tree, tmvaFile, TMVAOptions, i);

        input_file->Close();
        delete input_file;
   }

   tmvaFile->Close();
   delete tmvaFile;
}
