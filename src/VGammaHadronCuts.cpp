/*! \class VGammaHadronCuts
  \brief class containing parameter cut definitions

  List of cut selectors:

  gamma/hadron cut selector values consist of two digits: ID1+ID2*10

  ID2:

     0: apply gamma/hadron cuts on parameters in given data tree
     1: apply gamma/hadron cuts on probabilities given by a friend to the data tree (e.g. random forest analysis)
     2: same as 2
     3: apply cuts on probabilities given by a friend to the data tree already at the level of
        the event quality level
     4: TMVA gamma/hadron separation
     5: XGBoost gamma/hadron separation

  ID1:

     0: apply cuts on MSCW/MSCL (mean reduced scaled width/length)
     1: apply cuts on mean width/length (no lookup tables necessary)
     2: no cut applied (always passed)
     3: apply cuts on MWR/MLR (mean scaled width/length)

  Example:

    cut selector = 0 : apply MSCW/MSCL cuts (default)
    cut selector = 22 : apply event probability cuts
    cut selector = 10 : apply cuts from a tree AND apply MSCW/MSCL cuts
    cut selector = 42: apply TMVA gamma/hadron separation AND apply mean width/length cuts (default TMVA-BDT cut)
    cut selector = 52: apply XGBoost gamma/hadron separation AND apply mean width/length cuts

*/

#include "VGammaHadronCuts.h"

VGammaHadronCuts::VGammaHadronCuts()
{
    setDebug( false );

    resetCutValues();

    fStats = 0;

    fAnalysisType = GEO;
    fGammaHadronCutSelector = 0;
    bMCCuts = false;

    fData = 0;
    fNTel = 0;
    fNLTrigs = 0;
    fDataDirectory = "";
    fInstrumentEpoch = "NOT_SET";

    // mean width/length/distance
    fMeanImageWidth = 0.;
    fMeanImageLength = 0.;
    fMeanImageDistance = 0.;

    // phase cuts
    fOrbitalPhase_min = -1.;
    fOrbitalPhase_max = 1.e10;
    fUseOrbitalPhaseCuts = false;

    // TMVA evaluator
    fTMVAEvaluator = 0;
    fTMVA_MVAMethod = "";
    fTMVAWeightFileIndex_Emin = 0;
    fTMVAWeightFileIndex_Emax = 0;
    fTMVAWeightFileIndex_Zmin = 0;
    fTMVAWeightFileIndex_Zmax = 0;
    fTMVAWeightFile = "";
    fTMVASignalEfficiency.clear();
    fTMVA_MVACut.clear();
    fTMVA_EvaluationResult = -99.;
    fTMVAEvaluatorResults = 0;

    setArrayCentre();
}

void VGammaHadronCuts::initialize()
{
    fStats = new VGammaHadronCutsStatistics();
    fStats->initialize();
}

VGammaHadronCuts::~VGammaHadronCuts()
{
    if( fStats )
    {
        delete fStats;
    }
}

void VGammaHadronCuts::resetCutStatistics()
{
    if( fStats )
    {
        fStats->reset();
    }
}

void VGammaHadronCuts::resetCutValues()
{
    // array cuts
    fCut_Theta2_min = -100.;
    fCut_Theta2_max = 100.;
    fCut_Chi2_min = -1.e6;
    fCut_Chi2_max = 1.e10;
    fCut_MeanImageDistance_min = -100;
    fCut_MeanImageDistance_max = 100;
    fCut_MeanImageLength_min = -100;
    fCut_MeanImageLength_max = 100;
    fCut_MeanImageWidth_min = -100;
    fCut_MeanImageWidth_max = 100;
    fCut_Size_min = -1000;
    fCut_Size_max = 1e10;
    fCut_Ntubes_min = 4;
    fCut_Ntubes_max = 50000;
    fCut_MSCW_min   = -1.;
    fCut_MSCW_max   = 1.;
    fCut_MSCL_min   = -1.;
    fCut_MSCL_max   = 1.;
    fCut_MSW_min    = -1.;
    fCut_MSW_max    = 10.;
    fCut_MSL_min    = -1.;
    fCut_MSL_max    = 10.;
    fCut_CameraFiducialSize_min = -1.;
    fCut_CameraFiducialSize_max = 100.;
    fCut_CameraFiducialSize_MC_min = -100.;
    fCut_CameraFiducialSize_MC_max = 100.;
    fCut_dE_min = -99.;
    fCut_dE_max = 1.e12;
    fCut_EChi2_min = 0.;
    fCut_EChi2_max = 99999.;
    fCut_Erec_min = 0.;
    fCut_Erec_max = 1.e10;
    fCut_Emmission_min = 0.;
    fCut_Emmission_max = 1.e12;
    fCut_NImages_min = 0;
    fCut_NImages_max = 100000;

    fCut_AverageCoreDistanceToTelescopes_min = -99.;
    fCut_AverageCoreDistanceToTelescopes_max = 99999.;
    fCut_MinimumCoreDistanceToTelescopes_max = 1.e10;

    fCut_SizeSecondMax_min = -1000;
    fCut_SizeSecondMax_max = 1.e10;

    fCut_DispIntersectDiff_min = -1000.;
    fCut_DispIntersectDiff_max = 1.e10;
    fCut_DispIntersectSuccess = 1;

    fOrbitalPhase_min = -1.;
    fOrbitalPhase_max = 1.e10;

    fCut_CoreDistanceToArrayCentreX_min = -1.e10;
    fCut_CoreDistanceToArrayCentreX_max =  1.e10;
    fCut_CoreDistanceToArrayCentreY_min = -1.e10;
    fCut_CoreDistanceToArrayCentreY_max =  1.e10;
    fCut_CoreDistanceEdgeSize = 0.;
}


/*

    read cuts from a text file

    iPrint == 0 : suppress printing of any statements to screen

*/
bool VGammaHadronCuts::readCuts( string i_cutfilename, int iPrint )
{
    // reset trigger vector
    fNLTrigs = 0;
    fCut_ImgSelect.clear();

    // open text file
    ifstream is;
    i_cutfilename = VUtilities::testFileLocation( i_cutfilename, "GammaHadronCutFiles", true );
    if( iPrint == 1 )
    {
        cout << "\t reading analysis cuts from " << i_cutfilename << endl;
    }
    else if( iPrint == 2 )
    {
        cout << "reading analysis cuts from " << i_cutfilename << endl;
    }

    is.open( gSystem->ExpandPathName( i_cutfilename.c_str() ), ifstream::in );
    if(!is )
    {
        cout << "VGammaHadronCuts::readCuts: cut input file not found, " << i_cutfilename << endl;
        return false;
    }
    string is_line;
    string temp;
    string iCutVariable;
    while( getline( is, is_line ) )
    {
        if( is_line.size() > 0 )
        {
            istringstream is_stream( is_line );
            is_stream >> temp;
            if( temp != "*" )
            {
                continue;
            }
            is_stream >> iCutVariable;
            //////////////////////////////////////
            // gamma/hadron cut selectors
            if( iCutVariable == "cutselection" )
            {
                if(!( is_stream >> std::ws ).eof() )
                {
                    is_stream >> fGammaHadronCutSelector;
                }
            }
            /////////////////////////////////////
            // stereo ('array') telescope cuts
            else if( iCutVariable == "arraywidth" )
            {
                is_stream >> temp;
                fCut_MeanImageWidth_min = ( atof( temp.c_str() ) );
                is_stream >> temp;
                fCut_MeanImageWidth_max = ( atof( temp.c_str() ) );
            }
            else if( iCutVariable == "arraylength" )
            {
                is_stream >> temp;
                fCut_MeanImageLength_min = ( atof( temp.c_str() ) );
                is_stream >> temp;
                fCut_MeanImageLength_max = ( atof( temp.c_str() ) );
            }
            else if( iCutVariable == "arrayntubes" )
            {
                is_stream >> temp;
                fCut_Ntubes_min = ( atoi( temp.c_str() ) );
                is_stream >> temp;
                fCut_Ntubes_max = ( atoi( temp.c_str() ) );
            }
            else if( iCutVariable == "arrayntel" || iCutVariable == "nnimages" )
            {
                is_stream >> temp;
                fCut_NImages_min = ( atoi( temp.c_str() ) );
                is_stream >> temp;
                fCut_NImages_max = ( atoi( temp.c_str() ) );
            }
            else if( iCutVariable == "arraysize" )
            {
                is_stream >> temp;
                fCut_Size_min = ( atof( temp.c_str() ) );
                is_stream >> temp;
                fCut_Size_max = ( atof( temp.c_str() ) );
                // write warning out about size cuts and number of telescopes
                if( fNTel > 2 && fCut_Size_min > 0. )
                {
                    cout << "--------- VGammaHadronCuts warning: ignoring size cut for data with more than 2 telescopes ------" << endl;
                }
            }
            else if( iCutVariable == "arraychi2" )
            {
                is_stream >> temp;
                fCut_Chi2_min = ( atof( temp.c_str() ) );
                is_stream >> temp;
                fCut_Chi2_max = ( atof( temp.c_str() ) );
            }
            else if( iCutVariable == "arraydist" )
            {
                is_stream >> temp;
                fCut_MeanImageDistance_min = ( atof( temp.c_str() ) );
                is_stream >> temp;
                fCut_MeanImageDistance_max = ( atof( temp.c_str() ) );
            }
            else if( iCutVariable == "mscw" || iCutVariable == "arraymscw" )
            {
                is_stream >> temp;
                fCut_MSCW_min = ( atof( temp.c_str() ) );
                is_stream >> temp;
                fCut_MSCW_max = ( atof( temp.c_str() ) );
            }
            else if( iCutVariable == "mscl" || iCutVariable == "arraymscl" )
            {
                is_stream >> temp;
                fCut_MSCL_min = ( atof( temp.c_str() ) );
                is_stream >> temp;
                fCut_MSCL_max = ( atof( temp.c_str() ) );
            }
            else if( iCutVariable == "msw" || iCutVariable == "arraymsw" )
            {
                is_stream >> temp;
                fCut_MSW_min = ( atof( temp.c_str() ) );
                is_stream >> temp;
                fCut_MSW_max = ( atof( temp.c_str() ) );
            }
            else if( iCutVariable == "msl" || iCutVariable == "arraymsl" )
            {
                is_stream >> temp;
                fCut_MSL_min = ( atof( temp.c_str() ) );
                is_stream >> temp;
                fCut_MSL_max = ( atof( temp.c_str() ) );
            }
            else if( iCutVariable == "mc_xy_off" || iCutVariable == "arrayxyoff_mc" || iCutVariable == "mc_xyoff" || iCutVariable == "cameraedge_mc" )
            {
                is_stream >> temp;
                fCut_CameraFiducialSize_MC_min = ( atof( temp.c_str() ) );
                is_stream >> temp;
                fCut_CameraFiducialSize_MC_max = ( atof( temp.c_str() ) );
                bMCCuts = true;
            }

            else if( iCutVariable == "xy_off" || iCutVariable == "arrayxyoff" || iCutVariable == "xyoff" || iCutVariable == "cameraedge" )
            {
                is_stream >> temp;
                fCut_CameraFiducialSize_min = ( atof( temp.c_str() ) );
                is_stream >> temp;
                fCut_CameraFiducialSize_max = ( atof( temp.c_str() ) );
            }
            else if( iCutVariable == "telcoredistance" )
            {
                if(!( is_stream >> std::ws ).eof() )
                {
                    is_stream >> fCut_MinimumCoreDistanceToTelescopes_max;
                }
            }
            else if( iCutVariable == "arraycore" )
            {
                is_stream >> temp;
                fCut_AverageCoreDistanceToTelescopes_min = ( atof( temp.c_str() ) );
                is_stream >> temp;
                fCut_AverageCoreDistanceToTelescopes_max = ( atof( temp.c_str() ) );
            }
            // these allow to deselect certain telescope combinations (changed from ltrig to ImgSel)
            else if( iCutVariable == "arrayltrig" || iCutVariable == "allowedImageCombinations" )
            {
                if( fNTel == 0 || fNTel > 10 )
                {
                    cout << "VGammaHadronCuts::readCuts warning: cut identifier " << temp << " ignored for ";
                    cout << "current telescope configuration ( " << fNTel << " telescopes)" << endl;
                    continue;
                }
                // calculate how many possible telescope combinations exist (16 for 4 telescopes)
                if( fNLTrigs == 0 )
                {
                    int num_ltrigs = 0;
                    for( unsigned int i = 0; i < fNTel; i++ )
                    {
                        num_ltrigs += int( pow( 2., double( i ) ) );
                    }
                    num_ltrigs += 1;
                    fNLTrigs = num_ltrigs;
                    for( unsigned int i = 0; i < fNLTrigs; i++ )
                    {
                        fCut_ImgSelect.push_back( 1 );
                    }
                }
                // telescope combination
                is_stream >> temp;
                int index = ( atoi( temp.c_str() ) );
                // on = 0 or off = 1
                is_stream >> temp;
                int i_select = atoi( temp.c_str() );
                // check epoch
                bool i_useTheseCuts = true;
                if(!( is_stream >> std::ws ).eof() )
                {
                    is_stream >> temp;
                    if( fInstrumentEpoch.size() > 1 && temp != fInstrumentEpoch.substr( 0, 2 ) )
                    {
                        i_useTheseCuts = false;
                    }
                }
                // check telescope combinations
                if(!( is_stream >> std::ws ).eof() )
                {
                    is_stream >> temp;
                    if( fTelToAnalyze.size() > 0 && temp != getTelToAnalyzeString() )
                    {
                        i_useTheseCuts = false;
                    }
                }
                // set telescope combinations on or off
                if( i_useTheseCuts )
                {
                    if( index < 0 )
                    {
                        for( unsigned int i = 0; i < fCut_ImgSelect.size(); i++ )
                        {
                            fCut_ImgSelect[i] = i_select;
                        }
                    }
                    else if( index < ( int )fCut_ImgSelect.size() )
                    {
                        fCut_ImgSelect[index] = i_select;
                    }
                }
            }
            // phase cuts
            else if( iCutVariable == "orbitalPhase" )
            {
                is_stream >> temp;
                fOrbitalPhase_min = atof( temp.c_str() );
                if(!( is_stream >> std::ws ).eof() )
                {
                    fOrbitalPhase_max = atof( temp.c_str() );
                }
                else
                {
                    fOrbitalPhase_max = 1.e10;
                }
                fUseOrbitalPhaseCuts = true;
            }

            // energy reconstruction cuts
            else if( iCutVariable == "arrayechi2" )
            {
                is_stream >> temp;
                fCut_EChi2_min = atof( temp.c_str() );
                is_stream >> temp;
                fCut_EChi2_max = atof( temp.c_str() );
            }
            else if( iCutVariable == "arraydE" )
            {
                is_stream >> temp;
                fCut_dE_min = atof( temp.c_str() );
                is_stream >> temp;
                fCut_dE_max = atof( temp.c_str() );
            }
            else if( iCutVariable == "arrayerec" )
            {
                is_stream >> temp;
                fCut_Erec_min = atof( temp.c_str() );
                is_stream >> temp;
                fCut_Erec_max = atof( temp.c_str() );
            }
            else if( iCutVariable == "arraydispdiff" )
            {
                is_stream >> temp;
                fCut_DispIntersectDiff_min = atof( temp.c_str() );
                is_stream >> temp;
                fCut_DispIntersectDiff_max = atof( temp.c_str() );
                if(!( is_stream >> std::ws ).eof() )
                {
                    is_stream >> temp;
                    fCut_DispIntersectSuccess = atoi( temp.c_str() );
                }
            }
            else if( iCutVariable == "arrayemission" || iCutVariable == "emissionheight" )
            {
                is_stream >> temp;
                fCut_Emmission_min = atof( temp.c_str() );
                is_stream >> temp;
                fCut_Emmission_max = atof( temp.c_str() );
            }
            else if( iCutVariable == "sizesecondmax" )
            {
                float isize_min = -1000.;
                float isize_max = 1.e10;
                if(!( is_stream >> std::ws ).eof() )
                {
                    is_stream >> temp;
                    isize_min = atof( temp.c_str() );
                }
                if(!( is_stream >> std::ws ).eof() )
                {
                    is_stream >> temp;
                    isize_max = atof( temp.c_str() );
                }
                // check instrument epoch
                if(!( is_stream >> std::ws ).eof() )
                {
                    is_stream >> temp;
                    if( fInstrumentEpoch.size() > 1 && temp == fInstrumentEpoch.substr( 0, 2 ) )
                    {
                        fCut_SizeSecondMax_min = isize_min;
                        fCut_SizeSecondMax_max = isize_max;
                    }
                }
                else
                {
                    fCut_SizeSecondMax_min = isize_min;
                    fCut_SizeSecondMax_max = isize_max;
                }
            }
            ////////////////////////////////////////////////////////////////////////////////////////////////////
            // TMVA values
            else if( iCutVariable == "TMVAPARAMETER" )
            {
                if(!( is_stream >> std::ws ).eof() )
                {
                    is_stream >> temp;
                    if( fInstrumentEpoch.size() > 0 && temp == fInstrumentEpoch.substr( 0, 2 ) )
                    {
                        while(!( is_stream >> std::ws ).eof() )
                        {
                            if(!( is_stream >> std::ws ).eof() )
                            {
                                is_stream >> fTMVA_MVAMethod;
                            }
                            // files should have endings _fTMVAWeightFileIndex_min to _fTMVAWeightFileIndex_max
                            if(!( is_stream >> std::ws ).eof() )
                            {
                                is_stream >> fTMVAWeightFileIndex_Emin;
                            }
                            if(!( is_stream >> std::ws ).eof() )
                            {
                                is_stream >> fTMVAWeightFileIndex_Emax;
                            }
                            if(!( is_stream >> std::ws ).eof() )
                            {
                                is_stream >> fTMVAWeightFileIndex_Zmin;
                            }
                            if(!( is_stream >> std::ws ).eof() )
                            {
                                is_stream >> fTMVAWeightFileIndex_Zmax;
                            }
                            string iWeightFileDirectory;
                            if(!( is_stream >> std::ws ).eof() )
                            {
                                is_stream >> iWeightFileDirectory;
                            }
                            if( fInstrumentEpoch.size() > 1 )
                            {
                                iWeightFileDirectory.replace(
                                    iWeightFileDirectory.find( fInstrumentEpoch.substr( 0, 1 ) ),
                                    2, fInstrumentEpoch );
                            }
                            string iWeightFileName;
                            if(!( is_stream >> std::ws ).eof() )
                            {
                                is_stream >> iWeightFileName;
                            }
                            fTMVAWeightFile = gSystem->ExpandPathName( iWeightFileDirectory.c_str() );
                            // check of path name is complete
                            if( gSystem->AccessPathName( fTMVAWeightFile.c_str() ) )
                            {
                                fTMVAWeightFile = VGlobalRunParameter::getDirectory_EVNDISPAnaDataTMP() + fTMVAWeightFile;
                                if( gSystem->AccessPathName( fTMVAWeightFile.c_str() ) )
                                {
                                    cout << "VGammaHadronCuts::readCuts error,";
                                    cout << " weight file directory not found: ";
                                    cout << fTMVAWeightFile << endl;
                                    cout << "exiting..." << endl;
                                    exit( EXIT_FAILURE );
                                }
                            }
                            fTMVAWeightFile += iWeightFileName;
                            break;
                        }
                    }
                }
            }
            else if( iCutVariable == "TMVASignalEfficiency" )
            {
                while(!( is_stream >> std::ws ).eof() )
                {
                    unsigned int iKey = 0;
                    double iS = 0.;
                    if(!( is_stream >> std::ws ).eof() )
                    {
                        is_stream >> iKey;
                    }
                    if(!( is_stream >> std::ws ).eof() )
                    {
                        is_stream >> iS;
                    }
                    fTMVASignalEfficiency[iKey] = iS;
                }
            }
            else if( iCutVariable == "TMVA_MVACut" )
            {
                if(!( is_stream >> std::ws ).eof() )
                {
                    is_stream >> temp;
                    if( temp == fInstrumentEpoch )
                    {
                        while(!( is_stream >> std::ws ).eof() )
                        {
                            unsigned int iKey = 0;
                            double iS = 0.;
                            if( is_stream >> iKey && is_stream >> iS )
                            {
                                fTMVA_MVACut[iKey] = iS;
                            }
                        }
                    }
                }
            }
            ////////////////////////////////////////////////////////////////////////////////////////////////////
            // direction cut values
            ////////////////////////////////////////////////////////////////////////////////////////////////////

            // fixed theta2 cut
            else if( iCutVariable == "arraytheta2" || iCutVariable == "theta2cut" )
            {
                is_stream >> temp;
                fCut_Theta2_min = ( atof( temp.c_str() ) );
                is_stream >> temp;
                fCut_Theta2_max = ( atof( temp.c_str() ) );
            }
        }
    }
    cout << "========================================" << endl;
    if( fGammaHadronCutSelector / 10 == 4 )
    {
        fAnalysisType = MVAAnalysis;
    }
    else if( fGammaHadronCutSelector / 10 == 5 )
    {
        fAnalysisType = XGBoostAnalysis;
    }
    else
    {
        fAnalysisType = GEO;
    }

    return true;
}

void VGammaHadronCuts::printDirectionCuts()
{
    cout << "Theta2 cut: ";
    if( fCut_Theta2_min > 0. )
    {
        cout << fCut_Theta2_min << " < Theta^2 [deg^2]";
        if( fCut_Theta2_max > 0. )
        {
            cout << "< " << fCut_Theta2_max;
        }
    }
    else if( fCut_Theta2_max > 0. )
    {
        cout << "Theta^2 [deg^2] < " << fCut_Theta2_max;
    }
    else
    {
        cout << "no Theta^2 cut set";
    }
    cout << endl;

    if( fCut_DispIntersectDiff_max < 1.e9 )
    {
        cout << "Direction cut on difference between disp and intersection method: ";
        cout << ", minimum : " << fCut_DispIntersectDiff_min << " [deg] ";
        cout << ", maximum : " << fCut_DispIntersectDiff_max << " [deg] ";
        if( fCut_DispIntersectSuccess > 0 )
        {
            cout << " (require success of both reconstruction methods)";
        }
        cout << endl;
    }
}

void VGammaHadronCuts::printCutSummary()
{
    cout << "-----------------------------------------------------------------------------------------" << endl;
    cout << "VGammaHadronCuts::printCutSummary()";
    cout << " (ntel=" << fTelToAnalyze.size() << ": T" << getTelToAnalyzeString() << ") ";
    cout << endl;
    cout << "Gamma/hadron cut selector: " << fGammaHadronCutSelector << endl;
    if( fInstrumentEpoch != "NOT_SET" )
    {
        cout << "Instrument epoch selected: " << fInstrumentEpoch << endl;
    }
    else
    {
        cout << "Instrument epoch not set" << endl;
    }

    // direction cuts
    printDirectionCuts();

    //////////////////////////////////////////////////////////////////////////////////////////////
    // gamma/hadron cuts

    // mean reduced scaled cuts
    if( fGammaHadronCutSelector % 10 < 1 )
    {
        cout << "Shape cuts: ";
        cout << fCut_MSCW_min << " < MSCW < " << fCut_MSCW_max;
        cout << ", " << fCut_MSCL_min << " < MSCL < " << fCut_MSCL_max << endl;
    }
    // mean cuts
    else if( fGammaHadronCutSelector % 10 == 1 )
    {
        cout << "Shape cuts: ";
        cout << fCut_MeanImageWidth_min  << " < mean width < " << fCut_MeanImageWidth_max;
        cout << ", " << fCut_MeanImageLength_min << " < mean length < " << fCut_MeanImageLength_max << endl;
    }
    // mean scaled cuts
    else if( fGammaHadronCutSelector % 10 == 3 )
    {
        cout << "Shape cuts: ";
        cout << fCut_MSW_min << " < MWR < " << fCut_MSW_max;
        cout << ", " << fCut_MSL_min << " < MLR < " << fCut_MSL_max << endl;
    }
    cout << "Average core distance < " << fCut_AverageCoreDistanceToTelescopes_max << " m";
    cout << " (max distance to telescopes (mintel) " << fCut_MinimumCoreDistanceToTelescopes_max << " m)";
    // phase cuts
    if( useOrbitalPhaseCuts() )
    {
        cout << endl;
        cout << "Orbital Phase bin ( " << fOrbitalPhase_min << ", " << fOrbitalPhase_max << " )";
    }
    // TMVA cuts
    if( useTMVACuts() )
    {
        cout << endl;
        cout << "TMVA gamma/hadron separation with MVA method " << fTMVA_MVAMethod;
        cout << endl;
        cout << "weight files: " << fTMVAWeightFile;
        cout << " (" << fTMVAWeightFileIndex_Emin << "," << fTMVAWeightFileIndex_Emax << ")";
        cout << " (" << fTMVAWeightFileIndex_Zmin << "," << fTMVAWeightFileIndex_Zmax << ")" << endl;
        if( fDebug )
        {
            printSignalEfficiency();
            printTMVA_MVACut();
        }
    }
    // XGBoost cuts
    if( useXGBoostCuts() )
    {
        cout << "XGBoost gamma/hadron separation with fixed 70\% signal efficiency" << endl;
    }
    // other cut parameters
    if( fNTel == 2 )
    {
        cout << ", size > " << fCut_Size_min;
    }
    cout << endl;
    cout << "Fiducial area (camera) < " << fCut_CameraFiducialSize_max << " deg, ";
    cout << " stereo reconstruction: " << fCut_Chi2_min << " <= sChi2 <= " << fCut_Chi2_max << endl;
    cout << "Energy reconstruction: ";
    cout << fCut_EChi2_min << " <= EChi2 <= " << fCut_EChi2_max;
    cout << ", " << fCut_dE_min << " < dE < " << fCut_dE_max;
    cout << ", " << fCut_Erec_min << " < Erec < " << fCut_Erec_max << endl;
    cout << "SizeSecondMax: " << fCut_SizeSecondMax_min << " < SizeSecondMax < " << fCut_SizeSecondMax_max;
    cout << ", " << fCut_Emmission_min << " < Emission height < " << fCut_Emmission_max;
    cout << endl;
    cout << "NImage cut: " << fCut_NImages_min << " <= Ntel <= " << fCut_NImages_max;
    cout << endl;
    if( bMCCuts )
    {
        cout << "MC cuts: " << fCut_CameraFiducialSize_MC_min << " < fiducial area (camera) < " << fCut_CameraFiducialSize_MC_max << " deg ";
        cout << endl;
    }
    if( fCut_ImgSelect.size() > 0 )
    {
        cout << "Tel-combinations: ";
        for( unsigned int i = 0; i < fCut_ImgSelect.size(); i++ )
        {
            cout << i << ": " << fCut_ImgSelect[i];
            if( i < fCut_ImgSelect.size() - 1 )
            {
                cout << ", ";
            }
        }
        cout << endl;
    }
    cout << "-----------------------------------------------------------------------------------------" << endl;
}


/*
  ensure event quality, reasonable output from the table variables, etc
*/
bool VGammaHadronCuts::applyStereoQualityCuts( unsigned int iEnergyReconstructionMethod, bool bCount, int iEntry, bool fIsOn )
{
    /////////////////////////////////////////////////////////////////////////////////
    // apply number of images cut
    if( fData->NImages < fCut_NImages_min || fData->NImages > fCut_NImages_max )
    {
        if( bCount && fStats )
        {
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eNImages );
        }
        return false;
    }

    // require good pointing
    if( fData->Array_PointingStatus != 0 )
    {
        if( bCount && fStats )
        {
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
            fStats->updateCutCounter( VGammaHadronCutsStatistics::ePointing );
        }
        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////
    // require certain quality in stereo reconstruction
    if( fData->Chi2 < fCut_Chi2_min || fData->Chi2 > fCut_Chi2_max )
    {
        if( bCount && fStats )
        {
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eArrayChi2 );
        }
        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////
    if( iEnergyReconstructionMethod != 99 )
    {
        // quality cut for MSCW/L reconstruction cuts
        if( fGammaHadronCutSelector % 10 < 1 && ( fData->MSCW < -50. || fData->MSCL < -50. ) )
        {
            if( bCount && fStats )
            {
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eMSC_Quality );
            }
            return false;
        }
        // quality cut for MWR/MLR reconstruction cuts
        if( fGammaHadronCutSelector % 10 == 3 && ( fData->MWR < -50. || fData->MLR < -50. ) )
        {
            if( bCount && fStats )
            {
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eMSC_Quality );
            }
            return false;
        }

        /////////////////////////////////////////////////////////////////////////////////
        // check energy reconstruction
        double iErec  = getReconstructedEnergy( iEnergyReconstructionMethod );
        double iEchi2 = getReconstructedEnergyChi2( iEnergyReconstructionMethod );

        if( iErec > 0. && iEchi2 <= fCut_EChi2_min )
        {
            if( bCount && fStats )
            {
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eErec );
            }
            return false;
        }
        if( iErec < fCut_Erec_min )
        {
            if( bCount && fStats )
            {
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eErec );
            }
            return false;
        }
        if( iErec > fCut_Erec_max )
        {
            if( bCount && fStats )
            {
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eErec );
            }
            return false;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////
    // check core positions
    // closest distance between telescope and core
    double iR_min = 1.e10;
    // average distance to telescopes with images
    double iR = 0.;
    double iNTR = 0.;
    for( int i = 0; i < fData->NImages; i++ )
    {
        if( fData->ImgSel_list[i] >= fNTel )
        {
            continue;
        }

        if( fData->R_core[fData->ImgSel_list[i]] > 0. )
        {
            iR += fData->R_core[fData->ImgSel_list[i]];
            iNTR++;
            if( fData->R_core[fData->ImgSel_list[i]] < iR_min )
            {
                iR_min = fData->R_core[fData->ImgSel_list[i]];
            }
        }
    }
    if( iNTR > 0. )
    {
        iR /= iNTR;
    }

    // apply cut on distance
    if( iR < fCut_AverageCoreDistanceToTelescopes_min || iR > fCut_AverageCoreDistanceToTelescopes_max )
    {
        if( bCount && fStats )
        {
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eCorePos );
        }
        return false;
    }
    if( iR_min > fCut_MinimumCoreDistanceToTelescopes_max )
    {
        if( bCount && fStats )
        {
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eCorePos );
        }
        return false;
    }


    /////////////////////////////////////////////////////////////////////////////////
    // apply image selection cuts (check which telescopes were used in the reconstruction)
    if( fCut_ImgSelect.size() > 0 )
    {
        if( fData->ImgSel < fCut_ImgSelect.size() )
        {
            if(!fCut_ImgSelect[fData->ImgSel] )
            {
                if( bCount && fStats )
                {
                    fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
                    fStats->updateCutCounter( VGammaHadronCutsStatistics::eLTrig );
                }
                return false;
            }
        }
        else
        {
            if( bCount && fStats )
            {
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eLTrig );
            }
            return false;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////
    // apply cuts on size second max
    if( fData->SizeSecondMax < fCut_SizeSecondMax_min || fData->SizeSecondMax > fCut_SizeSecondMax_max )
    {
        if( bCount && fStats )
        {
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eSizeSecondMax );
        }
        return false;
    }

    /////////////////////////////////////////////////////////////////////////
    // apply cut on difference between stereo intersection and disp method
    // (for stereo method only: this should always pass)
    float i_disp_diff = sqrt(
                            ( fData->get_Xoff() - fData->Xoff_intersect ) * ( fData->get_Xoff() - fData->Xoff_intersect ) +
                            ( fData->get_Yoff() - fData->Yoff_intersect ) * ( fData->get_Yoff() - fData->Yoff_intersect ) );
    if( fCut_DispIntersectSuccess && ( fData->Xoff_intersect < -90. || fData->Yoff_intersect < -90. ) )
    {
        if( bCount && fStats )
        {
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eArrayDispDiff );
        }
        return false;
    }
    if( i_disp_diff < fCut_DispIntersectDiff_min || i_disp_diff > fCut_DispIntersectDiff_max )
    {
        if( bCount && fStats )
        {
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eArrayDispDiff );
        }
        return false;
    }

    return true;
}


/*!
  cuts apply in energies in linear scale!

  iEnergyReconstructionMethod == 100: return always true
*/
bool VGammaHadronCuts::applyEnergyReconstructionQualityCuts( unsigned int iEnergyReconstructionMethod, bool bCount )
{
    double iErec = getReconstructedEnergy( iEnergyReconstructionMethod );
    double iErecChi2 = getReconstructedEnergyChi2( iEnergyReconstructionMethod );
    double idE = getReconstructedEnergydE( iEnergyReconstructionMethod );

    if( iEnergyReconstructionMethod == 100 )
    {
        return true;
    }

    if( iErecChi2 < fCut_EChi2_min || iErecChi2 > fCut_EChi2_max )
    {
        if( bCount && fStats )
        {
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eEnergyRec );
        }
        return false;
    }
    if( iErec < fCut_Erec_min || iErec > fCut_Erec_max )
    {
        if( bCount && fStats )
        {
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eEnergyRec );
        }
        return false;
    }
    if( idE < fCut_dE_min || idE > fCut_dE_max )
    {
        if( bCount && fStats )
        {
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eEnergyRec );
        }
        return false;
    }

    return true;
}

/*
  apply gamma/hadron separation cut
*/
bool VGammaHadronCuts::isGamma( int i, bool bCount, bool fIsOn )
{
    /////////////////////////////////////////////////////////////////////////////
    // apply box cuts  (e.g. MSCW/MSCL or MWR/MLR)
    if( fGammaHadronCutSelector % 10 <= 3 )
    {
        if( fDebug )
        {
            cout << "VGammaHadronCuts::isGamma: applyStereoShapeCuts" << endl;
        }
        if(!applyStereoShapeCuts() )
        {
            if( bCount && fStats )
            {
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eIsGamma );
            }
            return false;
        }
        // all cut selectors >= 10 are different
        if( fGammaHadronCutSelector < 10 )
        {
            return true;
        }
    }
    /////////////////////////////////////////////////////////////////////////////
    // apply cut using TMVA reader
    if( useTMVACuts() )
    {
        if( fDebug )
        {
            cout << "VGammaHadronCuts::isGamma: applyTMVACut" << endl;
        }
        if(!applyTMVACut( i ) )
        {
            if( bCount && fStats )
            {
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eIsGamma );
            }
            return false;
        }
    }
    else if( useXGBoostCuts() )
    {
        if( fDebug )
        {
            cout << "VGammaHadronCuts::isGamma: applyXGBoostCut" << endl;
        }
        if(!applyXGBoostCut( i ) )
        {
            if( bCount && fStats )
            {
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eIsGamma );
            }
            return false;
        }
    }

    return true;
}

/*

   use TMVA reader and apply cuts

*/
bool VGammaHadronCuts::applyTMVACut( int i )
{
    if( fDebug )
    {
        cout << "VGammaHadronCuts::applyTMVACut event " << i;
        cout << ", signal efficiency " << fTMVASignalEfficiency.size();
        cout << ", MVA cut " << fTMVA_MVACut.size();
        cout << " (" << fTMVAEvaluator << ")";
        cout << endl;
    }
    fTMVA_EvaluationResult = -99.;
    if( fTMVAEvaluator )
    {
        bool i_TMVA_Evaluation = fTMVAEvaluator->evaluate( false, true );
        fTMVA_EvaluationResult = fTMVAEvaluator->getTMVA_EvaluationResult();
        return i_TMVA_Evaluation;
    }

    return false;
}

/*

    apply XGBoost cuts

*/
bool VGammaHadronCuts::applyXGBoostCut( int i )
{
    if( fDebug )
    {
        cout << "VGammaHadronCuts::applyXGBoostCut event " << i;
        cout << ", prediction " << fData->GH_Gamma_Prediction;
        cout << ", is gamma (70\% signal efficiency) " << (bool)fData->GH_Is_Gamma;
        cout << endl;
    }
    return (bool)fData->GH_Is_Gamma;
}


/*!

    apply mean reduced scaled cuts

*/
bool VGammaHadronCuts::applyMeanReducedScaledStereoShapeCuts()
{
    if( fData->MSCW > fCut_MSCW_max )
    {
        return false;
    }
    if( fData->MSCW < fCut_MSCW_min )
    {
        return false;
    }
    if( fData->MSCL > fCut_MSCL_max )
    {
        return false;
    }
    if( fData->MSCL < fCut_MSCL_min )
    {
        return false;
    }

    return true;
}

/*!

    apply mean scaled cuts

*/
bool VGammaHadronCuts::applyMeanScaledStereoShapeCuts()
{
    if( fData->MWR > fCut_MSW_max )
    {
        return false;
    }
    if( fData->MWR < fCut_MSW_min )
    {
        return false;
    }
    if( fData->MLR > fCut_MSL_max )
    {
        return false;
    }
    if( fData->MLR < fCut_MSL_min )
    {
        return false;
    }

    return true;
}

/*

   apply mean shape cuts

   (not scaled width, this calculation is MC independent)

*/
bool VGammaHadronCuts::applyMeanStereoShapeCuts()
{
    fMeanImageWidth = 0.;
    fMeanImageLength = 0.;
    fMeanImageDistance = 0.;
    int intel = 0;
    // loop over all telescopes and calculate mean values
    for( unsigned int i = 0; i < fNTel; i++ )
    {
        if( fData->ntubes[i] >= fCut_Ntubes_max ||  fData->ntubes[i] <= fCut_Ntubes_min )
        {
            continue;
        }
        if( fData->size[i] <= fCut_Size_min || fData->size[i] >= fCut_Size_max )
        {
            continue;
        }

        fMeanImageWidth += fData->width[i];
        fMeanImageLength += fData->length[i];
        fMeanImageDistance += fData->dist[i];
        intel++;
    }
    if( intel > 0 )
    {
        fMeanImageWidth      /= ( double )intel;
        fMeanImageLength     /= ( double )intel;
        fMeanImageDistance   /= ( double )intel;
    }
    else
    {
        return false;
    }
    // apply cuts
    if( fMeanImageDistance <= fCut_MeanImageDistance_min || fMeanImageDistance >= fCut_MeanImageDistance_max )
    {
        return false;
    }
    if( fMeanImageLength <= fCut_MeanImageLength_min || fMeanImageLength >= fCut_MeanImageLength_max )
    {
        return false;
    }
    if( fMeanImageWidth <= fCut_MeanImageWidth_min || fMeanImageWidth >= fCut_MeanImageWidth_max )
    {
        return false;
    }

    return true;
}

/*

   stereo shape cuts

*/
bool VGammaHadronCuts::applyStereoShapeCuts()
{
    /////////////////////////////////////////////////////////////////////////////
    // cut selector 2 is always true
    if( fGammaHadronCutSelector % 10 == 2 )
    {
        return true;
    }

    /////////////////////////////////////////////////////////////////////////////
    // apply cuts
    /////////////////////////////////////////////////////////////////////////////
    // MSCW/MSCL cuts
    if( fGammaHadronCutSelector % 10 < 1 )
    {
        if(!applyMeanReducedScaledStereoShapeCuts() )
        {
            return false;
        }
    }
    /////////////////////////////////////////////////////////////////////////////
    // mean width/length cuts
    // (not scaled width, this calculation is MC independent)
    else if( fGammaHadronCutSelector % 10 == 1 )
    {
        if(!applyMeanStereoShapeCuts() )
        {
            return false;
        }
    }
    /////////////////////////////////////////////////////////////////////////////
    // MWR/MLR cuts
    else if( fGammaHadronCutSelector % 10 == 3 )
    {
        if(!applyMeanScaledStereoShapeCuts() )
        {
            return false;
        }
    }

    /////////////////////////////////////////////////////////////////////////////
    // emission height cuts
    if( fData->EmissionHeight > 0. && fData->EmissionHeight > fCut_Emmission_max )
    {
        return false;
    }
    if( fData->EmissionHeight > 0. && fData->EmissionHeight < fCut_Emmission_min )
    {
        return false;
    }

    return true;
}


/*
  initialize gamma/hadron cuts

  (see description of cuts at the top of this file)
*/
void VGammaHadronCuts::initializeCuts( int irun, string iFile )
{
    // TMVA cuts
    if( useTMVACuts() )
    {
        if(!initTMVAEvaluator( fTMVAWeightFile,
                               fTMVAWeightFileIndex_Emin, fTMVAWeightFileIndex_Emax,
                               fTMVAWeightFileIndex_Zmin, fTMVAWeightFileIndex_Zmax ) )
        {
            cout << "VGammaHadronCuts::initializeCuts: failed setting TMVA reader for " << fTMVAWeightFile;
            cout << "(" << fTMVAWeightFileIndex_Emin << "," << fTMVAWeightFileIndex_Emax << ")" << endl;
            cout << "(" << fTMVAWeightFileIndex_Zmin << "," << fTMVAWeightFileIndex_Zmax << ")" << endl;
            cout << "exiting..." << endl;
            exit( EXIT_FAILURE );
        }
    }
    // phase cuts
    if( useOrbitalPhaseCuts() )
    {
        if( irun > 0 )
        {
            initPhaseCuts( irun );
        }
        else if( iFile.size() > 0 )
        {
            initPhaseCuts( iFile );
        }
        else
        {
            cout << "VGammaHadronCuts::initializeCuts: failed setting phase cuts for " << irun << " " << iFile << endl;
            cout << "exiting..." << endl;
            exit( EXIT_FAILURE );
        }
    }
}

/*
 * initialize TMVA evaluator
 *
 */
bool VGammaHadronCuts::initTMVAEvaluator( string iTMVAFile,
        unsigned int iTMVAWeightFileIndex_Emin, unsigned int iTMVAWeightFileIndex_Emax,
        unsigned int iTMVAWeightFileIndex_Zmin, unsigned int iTMVAWeightFileIndex_Zmax )
{
    TDirectory* cDir = gDirectory;

    fTMVAEvaluator = new VTMVAEvaluator();

    fTMVAEvaluator->setDebug( fDebug );
    // constant signal efficiency
    if( fTMVASignalEfficiency.size() > 0 )
    {
        fTMVAEvaluator->setSignalEfficiency( fTMVASignalEfficiency );
    }
    // fixed threshold or (for TMVA) a fixed MVA cut value
    else if( fTMVA_MVACut.size() > 0 )
    {
        fTMVAEvaluator->setTMVACutValue( fTMVA_MVACut );
    }
    else
    {
        cout << "VGammaHadronCuts::initTMVAEvaluator error: unclear TMVA cut settings" << endl;
        cout << "\t MVA Signal Efficiency: " << fTMVASignalEfficiency.size() << endl;
        cout << "\t MVA Threshold: " << fTMVA_MVACut.size() << endl;
        cout << "exiting... " << endl;
        exit( EXIT_FAILURE );
    }
    fTMVAEvaluator->setTMVAMethod( fTMVA_MVAMethod );
    // read MVA weight files; set MVA cut values (e.g. find optimal values)
    if(!fTMVAEvaluator->initializeWeightFiles( iTMVAFile, iTMVAWeightFileIndex_Emin, iTMVAWeightFileIndex_Emax,
            iTMVAWeightFileIndex_Zmin, iTMVAWeightFileIndex_Zmax ) )
    {
        cout << "VGammaHadronCuts::initTMVAEvaluator: error while initializing TMVA weight files" << endl;
        cout << "exiting... " << endl;
        exit( EXIT_FAILURE );
    }

    fTMVAEvaluatorResults = fTMVAEvaluator->getTMVAEvaluatorResults();
    fTMVAEvaluator->printSignalEfficiency();

    if( cDir )
    {
        cDir->cd();
    }

    return !fTMVAEvaluator->IsZombie();
}

bool VGammaHadronCuts::setDataTree( CData* idata )
{
    fData = idata;

    if(!fData )
    {
        return false;
    }

    if( fTMVAEvaluator )
    {
        fTMVAEvaluator->initializeDataStructures( fData );
    }

    return true;
}

bool VGammaHadronCuts::initPhaseCuts( int irun )
{
    ostringstream iFile;
    iFile << fDataDirectory << "/" << irun << ".mscw.rf.root";

    return initPhaseCuts( iFile.str() );
}


bool VGammaHadronCuts::initPhaseCuts( string iDir )
{
    TDirectory* cDir = gDirectory;

    fPhaseCut_File = new TFile( iDir.c_str() );
    if( fPhaseCut_File->IsZombie() )
    {
        cout << "Error while opening file with phase cuts: " << iDir << endl;
        exit( 0 );
    }
    cout << "\t opening file with phase cuts: " << fPhaseCut_File->GetName() << endl;

    fPhaseCut_Tree = ( TTree* )gDirectory->Get( "phase" );
    if(!fPhaseCut_Tree )
    {
        cout << "Error: could not find tree with phase cuts" << endl;
        exit( 0 );
    }
    fPhaseCut_Tree->SetBranchAddress( "phase", &fOrbitalPhase );

    if( cDir )
    {
        cDir->cd();
    }

    return true;
}


/*

   check that event is inside a certain region in the camera

*/
bool VGammaHadronCuts::applyInsideFiducialAreaCut( bool bCount )
{

    return applyInsideFiducialAreaCut( getReconstructedXoff(), getReconstructedYoff(), bCount );
}

bool VGammaHadronCuts::applyInsideFiducialAreaCut( float Xoff, float Yoff, bool bCount )
{
    double xy = Xoff * Xoff + Yoff * Yoff;

    if( xy > fCut_CameraFiducialSize_max * fCut_CameraFiducialSize_max )
    {
        if( bCount && fStats )
        {
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eXYoff );
        }
        return false;
    }

    if( fCut_CameraFiducialSize_min >= 0. )
    {
        if( xy < fCut_CameraFiducialSize_min * fCut_CameraFiducialSize_min )
        {
            if( bCount && fStats )
            {
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eXYoff );
            }
            return false;
        }
    }

    return true;
}

/*

    cut on Monte Carlo source position
    (camera coordinates)

*/
bool VGammaHadronCuts::applyMCXYoffCut( double xoff, double yoff, bool bCount )
{
    if(!fData->isMC() )
    {
        return true;
    }

    double xy = xoff * xoff + yoff * yoff;

    if( xy > fCut_CameraFiducialSize_MC_max * fCut_CameraFiducialSize_MC_max )
    {
        if( bCount && fStats )
        {
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eMC_XYoff );
        }
        return false;
    }

    if( fCut_CameraFiducialSize_MC_min >= 0. )
    {
        if( xy < fCut_CameraFiducialSize_MC_min * fCut_CameraFiducialSize_MC_min )
        {
            if( bCount && fStats )
            {
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eMC_XYoff );
            }
            return false;
        }
    }

    return true;
}

/*
   apply cut on event direction (theta2 cut)

  * function is called for effective areas calculations only (MC)
  * the cut might be energy dependent (if a function is defined)

  x0, y0:   calculate theta2 relative to these points (-99999. if relative to MCx/yoff)

*/
bool VGammaHadronCuts::applyDirectionCuts( unsigned int fEnergyReconstructionMethod, bool bCount, double x0, double y0 )
{
    double theta2 = 0.;

    // define reference direction
    // (default is simulated direction (MCxoff/MCyoff)
    if( x0 < -99990. )
    {
        if( fData->isMC() )
        {
            x0 = fData->MCxoff;
        }
        else
        {
            x0 = 0.;
        }
    }
    if( y0 < -99990. )
    {
        if( fData->isMC() )
        {
            y0 = fData->MCyoff;
        }
        else
        {
            y0 = 0.;
        }
    }

    // calculate theta2
    theta2 = ( getReconstructedXoff() - x0 ) * ( getReconstructedXoff() - x0 ) + ( getReconstructedYoff() - y0 ) * ( getReconstructedYoff() - y0 );

    // fetch theta2 cut (max)
    double i_theta2_cut_max = getTheta2Cut_max();

    // direction cut (ring around center of camera)
    if( theta2 < i_theta2_cut_max && theta2 > fCut_Theta2_min )
    {
        return true;
    }

    if( bCount && fStats )
    {
        fStats->updateCutCounter( VGammaHadronCutsStatistics::eDirection );
    }

    return false;
}

/*!
    update statistics
*/
void VGammaHadronCuts::newEvent( bool iFillTree )
{
    // fill previous event
    if( iFillTree && fStats->getCounterValue( VGammaHadronCutsStatistics::eTot > 0 ) )
    {
        fStats->fill();
    }

    fStats->updateCutCounter( VGammaHadronCutsStatistics::eTot );
}


void VGammaHadronCuts::terminate()
{
    SetName( "GammaHadronCuts" );

    if( fStats->getDataTree() )
    {
        fStats->terminate();
        fStats->getDataTree()->Write();
    }
    if( fTMVAEvaluatorResults )
    {
        fTMVAEvaluatorResults->Write();
    }
    else
    {
        cout << "No TMVAEvaluator Results." << endl;
    }

    Write();
}

void VGammaHadronCuts::printSignalEfficiency()
{
    if( fTMVASignalEfficiency.size() == 0 )
    {
        cout << "no signal efficiency set, already optimized?" << endl;
        return;
    }

    map< unsigned int, double >::iterator iIter;
    for( iIter = fTMVASignalEfficiency.begin(); iIter != fTMVASignalEfficiency.end(); iIter++ )
    {
        cout << "signal efficiency for energy bin " << iIter->first << ": ";
        cout << iIter->second << endl;
    }
}

void VGammaHadronCuts::printTMVA_MVACut()
{
    if( fTMVA_MVACut.size() == 0 )
    {
        cout << "no MVA cut set" << endl;
        return;
    }

    map< unsigned int, double >::iterator iIter;
    for( iIter = fTMVA_MVACut.begin(); iIter != fTMVA_MVACut.end(); iIter++ )
    {
        cout << "MVA cut for energy/zenith bin " << iIter->first << ": ";
        cout << iIter->second << endl;
    }
}


/*

    apply orbital or pulsar phase cuts - info is store in a tree called PHASE

*/
bool VGammaHadronCuts::applyPhaseCut( int i )
{
    if( useOrbitalPhaseCuts() )
    {
        if( fPhaseCut_Tree && fPhaseCut_Tree->GetEntry( i ) )
        {
            if( fOrbitalPhase >= fOrbitalPhase_min && fOrbitalPhase < fOrbitalPhase_max )
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    return true;

}


string VGammaHadronCuts::getTelToAnalyzeString()
{
    stringstream iTemp;
    sort( fTelToAnalyze.begin(), fTelToAnalyze.end() );
    for( unsigned int i = 0; i < fTelToAnalyze.size(); i++ )
    {
        iTemp << fTelToAnalyze[i] + 1;
    }

    return iTemp.str();
}

double VGammaHadronCuts::getReconstructedEnergy( unsigned int iEnergyReconstructionMethod )
{
    if(!fData )
    {
        return -99.;
    }
    return fData->get_Erec( iEnergyReconstructionMethod );
}

double VGammaHadronCuts::getReconstructedEnergyChi2( unsigned int iEnergyReconstructionMethod )
{
    if(!fData )
    {
        return -99.;
    }
    if( iEnergyReconstructionMethod == 0 )
    {
        return fData->EChi2;
    }
    else if( iEnergyReconstructionMethod == 1 )
    {
        return fData->EChi2S;
    }
    return -99.;
}

double VGammaHadronCuts::getReconstructedEnergydE( unsigned int iEnergyReconstructionMethod )
{
    if(!fData )
    {
        return -99.;
    }
    if( iEnergyReconstructionMethod == 0 )
    {
        return fData->dE;
    }
    else if( iEnergyReconstructionMethod == 1 )
    {
        return fData->dES;
    }
    return -99.;
}

double VGammaHadronCuts::getReconstructedXoff()
{
    if(!fData )
    {
        return -9999.;
    }
    return fData->get_Xoff( 0 );
}

double VGammaHadronCuts::getReconstructedYoff()
{
    if(!fData )
    {
        return -9999.;
    }
    return fData->get_Yoff( 0 );
}

double VGammaHadronCuts::getReconstructedXcore()
{
    if(!fData )
    {
        return -9999.;
    }
    return fData->Xcore;
}

double VGammaHadronCuts::getReconstructedYcore()
{
    if(!fData )
    {
        return -9999.;
    }
    return fData->Ycore;
}
