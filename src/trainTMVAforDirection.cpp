/* \file trainTMVAforDirection.cpp
   \brief use TMVA methods for direction reconstruction

   Calculate direction (Xoff, Yoff) in camera coordinates using BDT regression and
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
 * Train TMVA BDTs for direction reconstruction
*/
void train(TTree* events, TFile* tmvaFile, string TMVAOptions, const unsigned int n_tel = 4)
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
        for( unsigned int n = 0; n < n_tel; n++ )
        {
            ostringstream var;
            if( tmvaTarget[t] == "MCxoff" )
            {
                var << "disp_x_" << n;
                loader.AddVariable(var.str().c_str());
            }
            else
            {
                var << "disp_y_" << n;
                loader.AddVariable(var.str().c_str());
            }
        }
        if( tmvaTarget[t] == "MCxoff" )
        {
            loader.AddVariable("Xoff_weighted_bdt");
            loader.AddVariable("Xoff_intersect");
        }
        else
        {
           loader.AddVariable("Yoff_weighted_bdt");
           loader.AddVariable("Yoff_intersect");
        }

        loader.AddSpectator("MCe0");

        loader.AddTarget(tmvaTarget[t].c_str(), tmvaTargetName[t].c_str());

        // Create separate trees for training and testing based on TrainingEvent variable
        TTree* trainTree = events->CopyTree("TrainingEvent==1");
        TTree* testTree  = events->CopyTree("TrainingEvent==0");

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
    TTree outTree("Training", "Flattened training tree");

    float outArr[n_tel_var][n_tel];
    float disp_x[n_tel];
    float disp_y[n_tel];
    for(unsigned v = 0; v < training_variables.size(); v++ )
    {
        for (unsigned int i = 0; i < n_tel; ++i)
        {
            string bname = training_variables[v] + "_" + std::to_string(i);
            outTree.Branch(bname.c_str(), &outArr[v][i]);
        }
    }
    for (unsigned int i = 0; i < n_tel; ++i)
    {
        outTree.Branch(("disp_x_" + std::to_string(i)).c_str(), &disp_x[i]);
        outTree.Branch(("disp_y_" + std::to_string(i)).c_str(), &disp_y[i]);
    }
    outTree.Branch("Xoff_weighted_bdt", &Xoff);
    outTree.Branch("Yoff_weighted_bdt", &Yoff);
    outTree.Branch("Xoff_intersect", &Xoff_intersect);
    outTree.Branch("Yoff_intersect", &Yoff_intersect);
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
            n_index = DispTelList_T[i];
            disp_x[i] = outArr[0][i] * outArr[3][n_index]; // Disp_T * cosphi
            disp_y[i] = outArr[0][i] * outArr[4][n_index]; // Disp_T * sinphi
        }

        if( TrainingEvent )
        {
            trainingEvents++;
            perFileTrainingCounts[fileIndex]++;
        }
        else
        {
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
    if( argc < 6 )
    {
        cout << "./trainTMVAforDirection <list of input eventdisplay files (MC)> <output file>" << endl;
        cout << "                        <training file name> <train vs test fraction> <max. number of events>" << endl;
        cout << "                        [TMVA options (optional)]" << endl;
        cout << endl;
        cout << "example: ./trainTMVAforDirection files.txt dir_bdt.root train_events 0.5 100000" << endl;
        cout << endl;
        exit( EXIT_SUCCESS );
    }
    string inputFileList = argv[1];
    string outputFile = argv[2];
    string trainingFileName = argv[3];
    float trainTestFraction = atof(argv[4]);
    unsigned int max_events = atoi(argv[5]);
    string TMVAOptions="!V:NTrees=800:BoostType=Grad:Shrinkage=0.1:MaxDepth=4:MinNodeSize=1.0%";
    if( argc >= 7 )
    {
        TMVAOptions = argv[6];
    }

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
