/*! \class VGammaHadronCuts
  \brief class containing parameter cut definitions

  Revision $Id: VGammaHadronCuts.cpp,v 1.19.2.3.4.4.4.1.8.3.4.3.2.6.2.7.2.1.4.2.2.5.2.18.2.4.2.10 2011/04/11 16:10:27 gmaier Exp $

  List of cut selectors:

  gamma/hadron cut selector values consist of two digits: ID1+ID2*10

  ID2:

     0: apply gamma/hadron cuts on parameters in given data tree
     1: apply gamma/hadron cuts on probabilities given by a friend to the data tree (e.g. random forest analysis)
     2: same as 2
     3: apply cuts on probabilities given by a friend to the data tree already at the level of
        the event quality level (e.g. of use for analysis of certain binary phases only) 
     4: TMVA gamma/hadron separation

  ID1:

     0: apply cuts on MSCW/MSCL (mean reduced scaled width/length)
     1: apply cuts on mean width/length (no lookup tables necessary)
     2: no cut applied (always passed)
     3: apply cuts on MWR/MLR (mean scaled width/length) 

  Example:

    cut selector = 0 : apply MSCW/MSCL cuts (default)
    cut selector = 22 : apply event probability cuts
    cut selector = 10 : apply cuts from a tree AND apply MSCW/MSCL cuts 

  direction cut selector value consist of one digit:

     0: energy independent direction cut (theta2 cut; set fArrayTheta2_max and fArrayTheta2_min)
        (in cut file: *theta2cut <theta2cut_min> <theta2cut_max>
     1: energy dependent TF1 function
        (in cut file: *theta2file ...)
     2: graph from instrument response function calculation
        (in cut file: *theta2file ... IRF)
     3: TMVA: use gamma/hadron part evaluation (no direction cuts applied in VGammaHadronCuts)
     4: TMVA: get direction cut from TMVA evaluator for each event
     5: TMVA: get direction cut from box cut graph obtained during initialization of TMVA evaluator

  \author
  Jamie Holder, Gernot Maier

*/

#include "VGammaHadronCuts.h"

VGammaHadronCuts::VGammaHadronCuts()
{
    setDebug( false ); 

    resetCutValues();

    fStats = 0;

    fGammaHadronCutSelector = 0;
    fDirectionCutSelector = 0;
    bMCCuts = false;

    fData = 0;
    fNTel = 0;
    fNLTrigs = 0;
    fDataDirectory = "";

// mean width/length/distance
    fMeanImageWidth = 0.;
    fMeanImageLength = 0.;
    fMeanImageDistance = 0.;

// probabilities cuts
    fProbabilityCut_File = 0;
    fProbabilityCut_Tree = 0;
    fProbabilityCut_QualityFlag = 0;
    fProbabilityCut_NSelectors = VANACUTS_PROBSELECTIONCUTS_MAX;
    fProbabilityCut_ProbID = 0;
    for( unsigned int i = 0; i < fProbabilityCut_NSelectors; i++ ) fProbabilityCut_SelectionCut[i] = -1.;

// phase cuts
    fOrbitalPhase_min = -1.;
    fOrbitalPhase_max = 1.e99;
    fUseOrbitalPhaseCuts = false;

// TMVA evaluator
    fTMVAEvaluator = 0;
    fTMVA_MVAMethod = "";
    fTMVAWeightFileIndex_min = 0;
    fTMVAWeightFileIndex_max = 0;
    fTMVAWeightFile = "";
    fTMVASignalEfficiency.clear();
    fTMVA_MVACut.clear();
// Note: for TMVA is this not the probability threshold but the MVA cut value
    fTMVAProbabilityThreshold = -99.;
    fTMVAOptimizeSignalEfficiencyParticleNumberFile = "";
    fTMVAOptimizeSignalEfficiencySignificance_Min = 5.;
    fTMVAOptimizeSignalEfficiencySignalEvents_Min = 10.;
    fTMVAOptimizeSignalEfficiencyObservationTime_h = 50.;
    fTMVAFixedSignalEfficiencyMinEnergy = 1.e99;
    fTMVAFixedSignalEfficiencyAboveMinEnergy = 1.;
    fTMVABoxCut_Theta2_max = 0;
    fTMVA_EvaluationResult = -99.;

// energy dependent theta2 cut
    fFileNameAngRes = "";
    fFileAngRes = 0;
    fF1AngResName = "";
    fF1AngRes = 0;
    fAngRes_ScalingFactor = 1.;
    fAngRes_AbsoluteMinimum = 0.;
    fAngRes_AbsoluteMaximum = 1.e99;
    fIRFAngRes = 0;
    fAngResContainmentProbability = 0;

// frogs
    frogsGoodnessImgCut = 0.;

    setArrayCentre();
}

void VGammaHadronCuts::initialize()
{
// statistics
    fStats = new VGammaHadronCutsStatistics(); 
    fStats->initialize();
}

VGammaHadronCuts::~VGammaHadronCuts()
{
   if( fStats ) delete fStats;
}

void VGammaHadronCuts::resetCutStatistics()
{
   if( fStats ) fStats->reset();
}

void VGammaHadronCuts::resetCutValues()
{
// single telescope cuts
//! Best cuts for data from telescope 1 (as of 2005)
    fAlpha_min=0.0;
    fAlpha_max=8.0;
    fDistance_min=0.27;
    fDistance_max=1.06;
    fLos_min=0.0;
    fLos_max=0.00017;
    fLength_min=0.12;
    fLength_max=0.38;
    fWidth_min=0.05;
    fWidth_max=0.12;
    fAsymm_min=0.0;
    fAsymm_max=10.0;
    fSize_min=-1000;
    fSize_max=1.e99;

// array cuts
    fCut_Theta2_min = -100.;
    fCut_Theta2_max = 100.;
    fCut_Chi2_min = -1.e6;
    fCut_Chi2_max = 1.e99;
    fCut_MeanImageDistance_min = -100;
    fCut_MeanImageDistance_max = 100;
    fCut_MeanImageLength_min=-100;
    fCut_MeanImageLength_max=100;
    fCut_MeanImageWidth_min=-100;
    fCut_MeanImageWidth_max=100;
    fCut_Size_min=-1000;
    fCut_Size_max=1e99;
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
    fCut_MinimumCoreDistanceToTelescopes_max = 1.e99;

    fCut_SizeSecondMax_min = -1000;  
    fCut_SizeSecondMax_max = 1.e99; 

    fProbabilityCut = 0.5;

    fOrbitalPhase_min = -1.;
    fOrbitalPhase_max = 1.e99;

    fCut_CoreDistanceToArrayCentreX_min = -1.e10;
    fCut_CoreDistanceToArrayCentreX_max =  1.e10;
    fCut_CoreDistanceToArrayCentreY_min = -1.e10;
    fCut_CoreDistanceToArrayCentreY_max =  1.e10;
    fCut_CoreDistanceEdgeSize = 0.;
}


/*

    read cuts from a text file

    iPrint == 0 : surpress printing of any statements to screen

*/
bool VGammaHadronCuts::readCuts( string i_cutfilename, int iPrint )
{
// reset trigger vector
    fNLTrigs = 0;
    fCut_ImgSelect.clear();

// open text file
    ifstream is;
    i_cutfilename = VUtilities::testFileLocation( i_cutfilename, "GammaHadronCutFiles", true );
    if( iPrint == 1 )      cout << "\t reading analysis cuts from " << i_cutfilename << endl;
    else if( iPrint == 2 ) cout << "reading analysis cuts from " << i_cutfilename << endl;

    is.open( gSystem->ExpandPathName( i_cutfilename.c_str() ),ifstream::in);
    if(!is)
    {
        cout << "VGammaHadronCuts::readCuts: cut input file not found, " << i_cutfilename << endl;
        return false;
    }
    string is_line;
    string temp;
    string iCutVariable;
    while( getline( is, is_line ) )
    {
        if(  is_line.size() > 0 )
        {
            istringstream is_stream( is_line );
            is_stream >> temp;
            if( temp != "*" ) continue;
            is_stream >> iCutVariable;
//////////////////////////////////////
// choose gamma/hadron cut selectors and direction cut selector
            if( iCutVariable == "cutselection" )
            {
               if( !is_stream.eof() ) is_stream >> fGammaHadronCutSelector;
               if( !is_stream.eof() ) is_stream >> fDirectionCutSelector;
            }
/////////////////////////////////////
// single telescope cuts
            else if( iCutVariable == "alpha" )
            {
                is_stream >> temp;
                fAlpha_min=(atof(temp.c_str()));
                is_stream >> temp;
                fAlpha_max=(atof(temp.c_str()));
            }
            else if( iCutVariable == "length" )
            {
                is_stream >> temp;
                fLength_min=(atof(temp.c_str()));
                is_stream >> temp;
                fLength_max=(atof(temp.c_str()));
            }
            else if( iCutVariable == "width" )
            {
                is_stream >> temp;
                fWidth_min=(atof(temp.c_str()));
                is_stream >> temp;
                fWidth_max=(atof(temp.c_str()));
            }
            else if( iCutVariable == "los" )
            {
                is_stream >> temp;
                fLos_min=(atof(temp.c_str()));
                is_stream >> temp;
                fLos_max=(atof(temp.c_str()));
            }
            else if( iCutVariable == "asymm" )
            {
                is_stream >> temp;
                fAsymm_min=(atof(temp.c_str()));
                is_stream >> temp;
                fAsymm_max=(atof(temp.c_str()));
            }
            else if( iCutVariable == "distance" )
            {
                is_stream >> temp;
                fDistance_min=(atof(temp.c_str()));
                is_stream >> temp;
                fDistance_max=(atof(temp.c_str()));
            }
            else if( iCutVariable == "size" )
            {
                is_stream >> temp;
                fSize_min=(atof(temp.c_str()));
                is_stream >> temp;
                fSize_max=(atof(temp.c_str()));
            }
/////////////////////////////////////
// stereo ('array') telescope cuts
            else if( iCutVariable == "arraywidth" )
            {
                is_stream >> temp;
                fCut_MeanImageWidth_min=(atof(temp.c_str()));
                is_stream >> temp;
                fCut_MeanImageWidth_max=(atof(temp.c_str()));
            }
            else if( iCutVariable == "arraylength" )
            {
                is_stream >> temp;
                fCut_MeanImageLength_min=(atof(temp.c_str()));
                is_stream >> temp;
                fCut_MeanImageLength_max=(atof(temp.c_str()));
            }
            else if( iCutVariable == "arrayntubes" )
            {
                is_stream >> temp;
                fCut_Ntubes_min = (atoi(temp.c_str()));
                is_stream >> temp;
                fCut_Ntubes_max =(atoi(temp.c_str()));
            }
            else if( iCutVariable == "arrayntel" || iCutVariable == "nnimages" )
            {
                is_stream >> temp;
                fCut_NImages_min = (atoi(temp.c_str()));
                is_stream >> temp;
                fCut_NImages_max =(atoi(temp.c_str()));
            }
            else if( iCutVariable == "arraysize" )
            {
                is_stream >> temp;
                fCut_Size_min=(atof(temp.c_str()));
                is_stream >> temp;
                fCut_Size_max=(atof(temp.c_str()));
// write warning out about size cuts and number of telescopes
                if( fNTel > 2 && fCut_Size_min > 0. )
                {
                    cout << "--------- VGammaHadronCuts warning: ignoring size cut for data with more than 2 telescopes ------" << endl;
                }
            }
            else if( iCutVariable == "arraychi2" )
            {
                is_stream >> temp;
                fCut_Chi2_min=(atof(temp.c_str()));
                is_stream >> temp;
                fCut_Chi2_max=(atof(temp.c_str()));
            }
            else if( iCutVariable == "arraydist" )
            {
                is_stream >> temp;
                fCut_MeanImageDistance_min=(atof(temp.c_str()));
                is_stream >> temp;
                fCut_MeanImageDistance_max=(atof(temp.c_str()));
            }
// NOT USED AND THEREFORE DISABLED
/*            else if( iCutVariable == "corearea" )
            {
                is_stream >> temp;
                fCut_CoreDistanceToArrayCentreX_min = (atof(temp.c_str()));
                is_stream >> temp;
                fCut_CoreDistanceToArrayCentreX_max = (atof(temp.c_str()));
                is_stream >> temp;
                fCut_CoreDistanceToArrayCentreY_min = (atof(temp.c_str()));
                is_stream >> temp;
                fCut_CoreDistanceToArrayCentreY_max = (atof(temp.c_str()));
                if( !is_stream.eof() )
                {
                    is_stream >> temp;
                    fCut_CoreDistanceEdgeSize = atof( temp.c_str() );
                }
            } */
            else if( iCutVariable == "mscw" || iCutVariable == "arraymscw" )
            {
                is_stream >> temp;
                fCut_MSCW_min=(atof(temp.c_str()));
                is_stream >> temp;
                fCut_MSCW_max=(atof(temp.c_str()));
            }
            else if( iCutVariable == "mscl" || iCutVariable == "arraymscl" )
            {
                is_stream >> temp;
                fCut_MSCL_min=(atof(temp.c_str()));
                is_stream >> temp;
                fCut_MSCL_max=(atof(temp.c_str()));
            }
            else if( iCutVariable == "msw" || iCutVariable == "arraymsw" )
            {
                is_stream >> temp;
                fCut_MSW_min=(atof(temp.c_str()));
                is_stream >> temp;
                fCut_MSW_max=(atof(temp.c_str()));
            }
            else if( iCutVariable == "msl" || iCutVariable == "arraymsl" )
            {
                is_stream >> temp;
                fCut_MSL_min=(atof(temp.c_str()));
                is_stream >> temp;
                fCut_MSL_max =(atof(temp.c_str()));
            }
            else if( iCutVariable == "mc_xy_off" || iCutVariable == "arrayxyoff_mc" || iCutVariable == "mc_xyoff" || iCutVariable == "cameraedge_mc" )
            {
                is_stream >> temp;
                fCut_CameraFiducialSize_MC_min=(atof(temp.c_str()));
                is_stream >> temp;
                fCut_CameraFiducialSize_MC_max=(atof(temp.c_str()));
                bMCCuts = true;
            }

            else if( iCutVariable == "xy_off" || iCutVariable == "arrayxyoff" || iCutVariable == "xyoff" || iCutVariable == "cameraedge" )
            {
                is_stream >> temp;
                fCut_CameraFiducialSize_min=(atof(temp.c_str()));
                is_stream >> temp;
                fCut_CameraFiducialSize_max=(atof(temp.c_str()));
            }
	    else if( iCutVariable == "telcoredistance" )
	    {
	       if( !is_stream.eof() ) is_stream >> fCut_MinimumCoreDistanceToTelescopes_max;
            }
            else if( iCutVariable == "arraycore" )
            {
                is_stream >> temp;
                fCut_AverageCoreDistanceToTelescopes_min = (atof(temp.c_str()));
                is_stream >> temp;
                fCut_AverageCoreDistanceToTelescopes_max = (atof(temp.c_str()));
            }
// these allow to deselect certain telescope combinations (changed from ltrig to ImgSel)
            else if( iCutVariable == "arrayltrig" || iCutVariable == "allowedImageCombinations" )
            {
                if( fNTel == 0 || fNTel > 10 )
		{
		   cout << "VGammaHadronCuts::readCuts warning: cut identifier " << temp << " ignored for ";
		   cout << "current telescope configuration ( "<< fNTel << "telescopes)" << endl;
		   continue;
                }
// calculate how many possible telescope combinations exist
                if( fNLTrigs == 0 )
                {
                    int num_ltrigs = 0;
                    for( unsigned int i = 0; i < fNTel; i++ )
                    {
                        num_ltrigs += int(pow(2.,double(i)));
                    }
                    num_ltrigs += 1;
                    fNLTrigs = num_ltrigs;
                    for( unsigned int i = 0; i < fNLTrigs; i++ ) fCut_ImgSelect.push_back(1);
                }

                is_stream >> temp;
                int index = (atoi(temp.c_str()));
                is_stream >> temp;
                if( index < 0 )
                {
                    for( unsigned int i = 0; i < fCut_ImgSelect.size(); i++ ) fCut_ImgSelect[i] = atoi(temp.c_str());
                }
                else if( index < (int)fCut_ImgSelect.size() )
                {
                    fCut_ImgSelect[index] = atoi(temp.c_str());
                }

            }
// probability cut variables (e.g. random forest)
            else if( iCutVariable == "RFthresh" || iCutVariable == "Probthresh" )
            {
                is_stream >> temp;
                fProbabilityCut = (atof(temp.c_str()));
                if( !is_stream.eof() )
                {
                    is_stream >> temp;
                    fProbabilityCut_ProbID = atoi( temp.c_str() );
                }
                else fProbabilityCut_ProbID = 0;
            }
// phase cuts
            else if (iCutVariable == "orbitalPhase") 
	    {
	       is_stream >> temp;
	       fOrbitalPhase_min = atof(temp.c_str());
	       if( !is_stream.eof() ) fOrbitalPhase_max = atof(temp.c_str());
	       else                   fOrbitalPhase_max = 1.e99;
	       fUseOrbitalPhaseCuts = true;
            }

// to define the lower bounds in probablity cut ranges  (e.g. random forest)
            else if( iCutVariable == "RFCutLowerVals" )
            {
                while(!is_stream.eof() )
                {
                    is_stream >> temp;
                    fProbabilityCutRangeLower.push_back(atof(temp.c_str()));
                }
            }

// to define the upper bounds in probablity cut ranges  (e.g. random forest)
            else if( iCutVariable == "RFCutUpperVals" )
            {
                while(!is_stream.eof() )
                {
                    is_stream >> temp;
                    fProbabilityCutRangeUpper.push_back(atof(temp.c_str()));
                }
            }

// to define the upper bounds in probablity cut ranges  (e.g. random forest)
            else if( iCutVariable == "RFProbID" )
            {
                    is_stream >> temp;
                    fProbabilityCut_ProbID = atoi( temp.c_str() );
            }
            
            
// energy reconstruction cuts
            else if( iCutVariable == "arrayechi2" )
            {
                is_stream >> temp;
                fCut_EChi2_min = atof(temp.c_str());
                is_stream >> temp;
                fCut_EChi2_max = atof(temp.c_str());
            }
            else if( iCutVariable == "arraydE" )
            {
                is_stream >> temp;
                fCut_dE_min = atof(temp.c_str());
                is_stream >> temp;
                fCut_dE_max = atof(temp.c_str());
            }
            else if( iCutVariable == "arrayerec" )
            {
                is_stream >> temp;
                fCut_Erec_min = atof(temp.c_str());
                is_stream >> temp;
                fCut_Erec_max = atof(temp.c_str());
            }
            else if( iCutVariable == "arrayemission" || iCutVariable == "emissionheight" )
            {
                is_stream >> temp;
                fCut_Emmission_min = atof(temp.c_str());
                is_stream >> temp;
                fCut_Emmission_max = atof(temp.c_str());
            }
            else if( iCutVariable == "sizesecondmax" )     
            {
                is_stream >> temp;
                fCut_SizeSecondMax_min = atof(temp.c_str());
                is_stream >> temp;
                fCut_SizeSecondMax_max = atof(temp.c_str());
            }       
// telescope type dependent cut on number of images 
// syntax:  teltype_nnimages <min images> <tel type counter>
            else if( iCutVariable == "teltypegroup" || iCutVariable == "teltype_nnimages" )
            {
                fNTelTypeCut.push_back( new VNTelTypeCut() );
                is_stream >> temp;
                fNTelTypeCut.back()->fNTelType_min = atoi( temp.c_str() );
                while( !is_stream.eof() )
                {
                    is_stream >> temp;
                    fNTelTypeCut.back()->fTelType_counter.push_back( atoi( temp.c_str() ) );
                }
            }
	    else if( iCutVariable == "goodnessimgcut" )
	    {
	        is_stream >> temp;
                frogsGoodnessImgCut = atoi( temp.c_str()  );
	    }
////////////////////////////////////////////////////////////////////////////////////////////////////
// TMVA values
            else if( iCutVariable == "TMVAPARAMETER" )
            {
	       if( !is_stream.eof() ) is_stream >> fTMVA_MVAMethod;
// files should have endings _fTMVAWeightFileIndex_min to _fTMVAWeightFileIndex_max
               if( !is_stream.eof() ) is_stream >> fTMVAWeightFileIndex_min;
               if( !is_stream.eof() ) is_stream >> fTMVAWeightFileIndex_max;
	       string iWeightFileDirectory;
               if( !is_stream.eof() ) is_stream >> iWeightFileDirectory;
	       string iWeightFileName;
               if( !is_stream.eof() ) is_stream >> iWeightFileName;
	       fTMVAWeightFile = gSystem->ExpandPathName( iWeightFileDirectory.c_str() );
	       fTMVAWeightFile += iWeightFileName;
            }
	    else if( iCutVariable == "TMVACUTS" )
	    {
	       if( !is_stream.eof() ) is_stream >> fTMVAOptimizeSignalEfficiencySignificance_Min;
	       if( !is_stream.eof() ) is_stream >> fTMVAOptimizeSignalEfficiencySignalEvents_Min;
	       if( !is_stream.eof() ) is_stream >> fTMVAOptimizeSignalEfficiencyObservationTime_h;
	       if( !is_stream.eof() ) is_stream >> fTMVAOptimizeSignalEfficiencyParticleNumberFile;
	       if( !is_stream.eof() ) is_stream >> fTMVAFixedSignalEfficiencyMinEnergy;
	       if( !is_stream.eof() ) is_stream >> fTMVAFixedSignalEfficiencyAboveMinEnergy;
            }
	    else if( iCutVariable == "TMVASignalEfficiency" )
	    {
	       unsigned int iE = 0;
	       double iS = 0.;
	       if( !is_stream.eof() ) is_stream >> iE;
	       if( !is_stream.eof() ) is_stream >> iS;
	       fTMVASignalEfficiency[iE] = iS;
            }
	    else if( iCutVariable == "TMVA_MVACut" )
	    {
	       unsigned int iE = 0;
	       double iS = 0.;
	       if( !is_stream.eof() ) is_stream >> iE;
	       if( !is_stream.eof() ) is_stream >> iS;
	       fTMVA_MVACut[iE] = iS;
////////////////////////////////////////////////////////////////////////////////////////////////////
            }
////////////////////////////////////////////////////////////////////////////////////////////////////
// direction cut values
////////////////////////////////////////////////////////////////////////////////////////////////////

// fixed theta2 cut
            else if( iCutVariable == "arraytheta2" || iCutVariable == "theta2cut" ) 
            {
                is_stream >> temp;
                fCut_Theta2_min=(atof(temp.c_str()));
                is_stream >> temp;
                fCut_Theta2_max=(atof(temp.c_str()));
            }
// read in values for energy dependent theta2 cut 
// * theta2file <root file> <function name>
// (note that fF1AngResName == "IRF" means that the graph from the IRF file is extrapolated)
            else if( iCutVariable == "theta2file" )
            {
                 if( !is_stream.eof() )
		 {
		    string iFileNameAngRes;
		    is_stream >> iFileNameAngRes;
		    if( fFileNameAngRes.find( "$" ) != string::npos )
		    {
		       fFileNameAngRes = gSystem->ExpandPathName( iFileNameAngRes.c_str() );
                    }
                 }
                 if( !is_stream.eof() )
                 {
                    is_stream >> fF1AngResName;
                 }
                 else fF1AngResName = "fitAngRes";
            }

// use angular resolution calculated for example from same data with makeEffectiveArea
            else if( iCutVariable == "angres" )
	    {
	       if( !is_stream.eof() ) is_stream >> fAngResContainmentProbability;    // should be an integer; probability x 100
            }
// theta2 scaling
// * theta2scaling <scale factor> <minimum theta> <maximum theta>
	    else if( iCutVariable == "theta2scaling" )
	    {
                 if( !is_stream.eof() )
                 {
		      is_stream >> temp;
		      fAngRes_ScalingFactor = atof( temp.c_str() );
                 }
                 if( !is_stream.eof() )
                 { 
		      is_stream >> temp;
		      fAngRes_AbsoluteMinimum = atof( temp.c_str() );
                 }
                 if( !is_stream.eof() )
                 { 
		      is_stream >> temp;
		      fAngRes_AbsoluteMaximum = atof( temp.c_str() );
		      fCut_Theta2_max = fAngRes_AbsoluteMaximum*fAngRes_AbsoluteMaximum;
                 }
            }   
////////////////////////////////////////////////////////////////////////////////////////////////////
        }
    }
    if( fFileNameAngRes.size() > 0 )
    {
       if( !initAngularResolutionFile() )
       {
	   cout << "VGammaHadronCuts::readCuts error: error initializing angular resolution file" << endl;
	   cout << "exiting..." << endl;
	   exit( -1 );
       }
    }
    return true;
}

void VGammaHadronCuts::printDirectionCuts()
{
   cout << "Direction cuts (cut selector " << fDirectionCutSelector << ")" << endl;

// theta2 cut (no energy dependence)
    if( fDirectionCutSelector == 0 )
    {
       cout << "Theta2 cut: ";
       if( fCut_Theta2_min > 0. )
       {
	   cout << fCut_Theta2_min << " < Theta^2 [deg^2]";
	   if( fCut_Theta2_max > 0. ) cout << "< " << fCut_Theta2_max;
       }
       else if( fCut_Theta2_max > 0. )  cout << "Theta^2 [deg^2] < " << fCut_Theta2_max;
       else                             cout << "no Theta^2 cut set";
       cout << endl;
    }
// theta cut from TF1 function (energy dependent)
    else if( fFileAngRes && fF1AngRes )
    {
        cout << "Direction cut from angular resolution function in file: ";
	cout << fFileAngRes->GetName();
	cout << "(" << fF1AngRes->GetName() << ")";
    }
// theta cut from IRF graph (energy depedendent)
    else if( fIRFAngRes || fDirectionCutSelector == 2 )
    {
        cout << "Direction cut from IRF graph " << endl;
	if( fIRFAngRes ) fIRFAngRes->Print();
	else if( getAngularResolutionContainmentRadius() > 0. )
	{
	   cout << " (calculated from same file, containment probability " << ((double)getAngularResolutionContainmentRadius())/100. << ")" << endl; 
	}
	else
	{
	    cout << endl;
	    cout << "VGammaHadronCuts::printDirectionCuts WARNING: no function found" << endl;
        }
    }
// theta cut using TMVA
    else if( fDirectionCutSelector == 3 || fDirectionCutSelector == 4 || fDirectionCutSelector == 5 )
    {
        cout << "Direction cut from TMVA " << endl; 
	if( fDirectionCutSelector == 3 )
	{
	   cout << "using TMVA evaluator (part of gamma/hadron cuts)" << endl;
        }
	if( fDirectionCutSelector == 4 || fDirectionCutSelector == 5 )
	{
	   if( !fTMVAEvaluator || !fTMVAEvaluator->getTMVAThetaCutVariable() )
	   {
	      cout << "VGammaHadronCuts::printDirectionCuts WARNING: no theta2 cut defined in TMVA method" << endl;
           }
	   if( fDirectionCutSelector == 5 && fTMVABoxCut_Theta2_max )
	   {
	      cout << " found box cut graph: " << endl;
	      fTMVABoxCut_Theta2_max->Print();
	   }
        }
    }
    cout << "Direction cut scale factor " << fAngRes_ScalingFactor;
    cout << ", minimum : " << fAngRes_AbsoluteMinimum << " [deg] ";
    cout << ", maximum : " << fAngRes_AbsoluteMaximum << " [deg]" << endl;

}

void VGammaHadronCuts::printCutSummary()
{
    cout << "-----------------------------------------------------------------------------------------" << endl;
    cout << "VGammaHadronCuts::printCutSummary()";
    cout << " (ntel=" << fNTel << "): ";
    cout << endl;
    cout << "Gamma/hadron cut selector: " << fGammaHadronCutSelector << endl;

// direction cuts
   printDirectionCuts();

//////////////////////////////////////////////////////////////////////////////////////////////
// gamma/hadron cuts

// mean reduced scaled cuts
    if( fGammaHadronCutSelector % 10 < 1 )
    {
	cout << "Shape cuts: ";
        cout << fCut_MSCW_min << " < MSCW < " << fCut_MSCW_max;
        cout << ", " << fCut_MSCL_min << " < MSCL < " << fCut_MSCL_max << ", ";
    }
// mean cuts
    else if( fGammaHadronCutSelector % 10 == 1 )
    {
	cout << "Shape cuts: ";
        cout << fCut_MeanImageWidth_min  << " < mean width < " << fCut_MeanImageWidth_max;
        cout << ", " << fCut_MeanImageLength_min << " < mean length < " << fCut_MeanImageLength_max << ", ";
    }
// mean scaled cuts
    else if( fGammaHadronCutSelector % 10 == 3 )
    {
	cout << "Shape cuts: ";
        cout << fCut_MSW_min << " < MWR < " << fCut_MSW_max;
        cout << ", " << fCut_MSL_min << " < MLR < " << fCut_MSL_max << ", ";
    }
    cout << "average core distance < " << fCut_AverageCoreDistanceToTelescopes_max << " m";
    cout << " (max distance to telescopes (mintel) " << fCut_MinimumCoreDistanceToTelescopes_max << " m)";
// probability cuts
    if( fGammaHadronCutSelector / 10 >= 1 && fGammaHadronCutSelector / 10 <= 3 )
    {
        cout << endl;
        cout << "event probability threshold: " << fProbabilityCut << " (element " << fProbabilityCut_ProbID << ")";
        if( fGammaHadronCutSelector / 10 == 3 ) cout << " (applied as quality cut)";
        if( fProbabilityCut_File ) cout << ", read from " << fProbabilityCut_File->GetName();
    }
// phase cuts
    if( useOrbitalPhaseCuts() )
    {
        cout << endl;
        cout << "Orbital Phase bin ( " << fOrbitalPhase_min << ", " << fOrbitalPhase_max << " )";
    }
// TMVA cuts
    if( fGammaHadronCutSelector / 10 == 4 )
    {
        cout << endl;
        cout << "TMVA gamma/hadron separation with MVA method " << fTMVA_MVAMethod;
	cout << endl;
        cout << "weight files: " << fTMVAWeightFile << " (" << fTMVAWeightFileIndex_min << "," << fTMVAWeightFileIndex_max << ")" << endl;
	if( fTMVAOptimizeSignalEfficiencyParticleNumberFile.size() > 0. )
	{
	   cout << "using optimal signal efficiency with a requirement of ";
	   cout << fTMVAOptimizeSignalEfficiencySignificance_Min << " sigma and at least ";
	   cout << fTMVAOptimizeSignalEfficiencySignalEvents_Min << " signal events in ";
	   cout << fTMVAOptimizeSignalEfficiencyObservationTime_h << " h observing time" << endl;
	   cout << "reading particle counts from " << fTMVAOptimizeSignalEfficiencyParticleNumberFile << endl;
	   cout << "\t fixed signal efficiency above energy  " << fTMVAFixedSignalEfficiencyMinEnergy;
	   cout << " (fixed to " << fTMVAFixedSignalEfficiencyAboveMinEnergy << ")" << endl;
	}
	else
	{
           printSignalEfficiency();
	   printTMVA_MVACut();
        }
    }
// other cut parameters
    if( fNTel == 2 ) cout << ", size > " << fCut_Size_min;
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
/*    cout << "Fiducial area (shower core): [" << fCut_CoreDistanceToArrayCentreX_min - fCut_CoreDistanceEdgeSize << ",";
    cout << fCut_CoreDistanceToArrayCentreX_max + fCut_CoreDistanceEdgeSize;
    cout << "," << fCut_CoreDistanceToArrayCentreY_min - fCut_CoreDistanceEdgeSize << ",";
    cout << fCut_CoreDistanceToArrayCentreY_max + fCut_CoreDistanceEdgeSize << "] m"; */
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
            if( i < fCut_ImgSelect.size() - 1 ) cout << ", ";
        }
        cout << endl;
    }
    if( fNTelTypeCut.size() > 0 )
    {
           for( unsigned int j = 0; j < fNTelTypeCut.size(); j++ ) fNTelTypeCut[j]->print();
    }
    cout << "-----------------------------------------------------------------------------------------" << endl;
}


/*
  ensure event quality, reasonable output from the table variables, etc
*/
bool VGammaHadronCuts::applyStereoQualityCuts( unsigned int iEnergyReconstructionMethod, bool bCount, int iEntry, bool fIsOn)
{

/////////////////////////////////////////////////////////////////////////////////
// require certain quality in stereo reconstruction
    if( fData->Chi2 < fCut_Chi2_min || fData->Chi2 > fCut_Chi2_max )
    {
        if( bCount )
        {
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eArrayChi2 );
        }
        return false;
    }

/////////////////////////////////////////////////////////////////////////////////
// apply number of images cut
    if( fData->NImages < fCut_NImages_min || fData->NImages > fCut_NImages_max )
    {
        if( bCount )
        {
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
            fStats->updateCutCounter( VGammaHadronCutsStatistics::eNImages );
        }
        return false;
    }

/////////////////////////////////////////////////////////////////////////////////
    if( iEnergyReconstructionMethod != 99 )
    {
// quality cut for MSCW/L reconstruction cuts
        if(  fGammaHadronCutSelector % 10 < 1 && ( fData->MSCW < -50. || fData->MSCL < -50. ) )
        {
            if( bCount )
            {
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eMSC_Quality );
            }
            return false;
        }
// quality cut for MWR/MLR reconstruction cuts
        if(  fGammaHadronCutSelector % 10 == 3 && ( fData->MWR < -50. || fData->MLR < -50. ) )
        {
            if( bCount )
            {
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
                fStats->updateCutCounter( VGammaHadronCutsStatistics::eMSC_Quality );
            }
            return false;
        }

/////////////////////////////////////////////////////////////////////////////////
// check energy reconstruction
        if( iEnergyReconstructionMethod == 0 )
        {
            if( fData->Erec > 0. && fData->EChi2 <= 0. )
	    {
                if( bCount )
                {
                    fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
                    fStats->updateCutCounter( VGammaHadronCutsStatistics::eErec );
                }
	        return false;
            }
            if( fData->Erec < fCut_Erec_min )
            {
                if( bCount )
                {
                    fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
                    fStats->updateCutCounter( VGammaHadronCutsStatistics::eErec );
                }
                return false;
            }
            if( fData->Erec > fCut_Erec_max )
            {
                if( bCount )
                {
                    fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
                    fStats->updateCutCounter( VGammaHadronCutsStatistics::eErec );
                }
                return false;
            }
        }
        else if( iEnergyReconstructionMethod == 1 || iEnergyReconstructionMethod == 2 )
        {
            if( fData->ErecS > 0. && fData->EChi2S <= 0. )
	    {
                if( bCount )
                {
                    fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
                    fStats->updateCutCounter( VGammaHadronCutsStatistics::eErec );
                }
	        return false;
            }
            if( fData->ErecS < fCut_Erec_min )
            {
                if( bCount )
                {
                    fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
                    fStats->updateCutCounter( VGammaHadronCutsStatistics::eErec );
                }
                return false;
            }
            if( fData->ErecS > fCut_Erec_max )
            {
                if( bCount )
                {
                    fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
                    fStats->updateCutCounter( VGammaHadronCutsStatistics::eErec );
                }
                return false;
            }
        }
    }

/////////////////////////////////////////////////////////////////////////////////
// check core positions
// (average distance to telescopes with images)
    double iR = 0.;
    double iR_min = 1.e99;
    double iNTR = 0.;
    for( int i = 0; i < fData->NImages; i++ )
    {
	if( fData->ImgSel_list[i] >= fNTel ) continue;

        if( fData->R[fData->ImgSel_list[i]] > 0. )
        {
            iR += fData->R[fData->ImgSel_list[i]];
            iNTR++;
	    if( fData->R[fData->ImgSel_list[i]] < iR_min ) iR_min = fData->R[fData->ImgSel_list[i]];
        }
    }
    if( iNTR > 0. ) iR /= iNTR;

    if( iR < fCut_AverageCoreDistanceToTelescopes_min || iR > fCut_AverageCoreDistanceToTelescopes_max )
    {
        if( bCount )
        {
	    fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
	    fStats->updateCutCounter( VGammaHadronCutsStatistics::eCorePos );
        }
        return false;
    }
    if( iR_min > fCut_MinimumCoreDistanceToTelescopes_max )
    {
        if( bCount )
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
            if( !fCut_ImgSelect[fData->ImgSel] )
            {
                if( bCount )
                {
	            fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
	            fStats->updateCutCounter( VGammaHadronCutsStatistics::eLTrig );
                }
		return false;
            }
        }
        else 
        {
            if( bCount )
            {
	        fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
	        fStats->updateCutCounter( VGammaHadronCutsStatistics::eLTrig );
            }
            return false;
        }
    }

/////////////////////////////////////////////////////////////////////////////////
// apply cuts on second max
    if(fData->SizeSecondMax < fCut_SizeSecondMax_min || fData->SizeSecondMax > fCut_SizeSecondMax_max)
    {
        if( bCount )
        {
	    fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
	    fStats->updateCutCounter( VGammaHadronCutsStatistics::eSizeSecondMax );
        }
        return false;
    }

/////////////////////////////////////////////////////////////////////////
// apply cut selector from probability tree
    if( fGammaHadronCutSelector / 10 == 3 )
    {
        if( !applyProbabilityCut( iEntry , fIsOn) )
        {
            if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eStereoQuality );
            return false;
        }
    }

    return true;
}


/*!
  cuts apply in energies in linear scale!

  iEnergyReconstructionMethod == 100: return always true
*/
bool VGammaHadronCuts::applyEnergyReconstructionQualityCuts( unsigned int iEnergyReconstructionMethod, bool bCount )
{
    if( iEnergyReconstructionMethod == 0 )
    {
        if( fData->EChi2 < fCut_EChi2_min || fData->EChi2 > fCut_EChi2_max )
	{
	   if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eEnergyRec );
	   return false;
        }
        if( fData->Erec < fCut_Erec_min || fData->Erec > fCut_Erec_max )
	{
	   if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eEnergyRec );
	   return false;
        }
        if( fData->dE < fCut_dE_min || fData->dE > fCut_dE_max )
	{
	   if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eEnergyRec );
	   return false;
        }
        if( fData->dE > fCut_dE_max )       return false;
    }
    else if( iEnergyReconstructionMethod == 1 )
    {
        if( fData->EChi2S < fCut_EChi2_min || fData->EChi2S > fCut_EChi2_max )
	{
	   if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eEnergyRec );
	   return false;
        }
        if( fData->ErecS < fCut_Erec_min || fData->ErecS > fCut_Erec_max )
	{
	   if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eEnergyRec );
	   return false;
        }
        if( fData->dES < fCut_dE_min || fData->dES > fCut_dE_max )
	{
	   if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eEnergyRec );
	   return false;
        }
    }
    else if( iEnergyReconstructionMethod == 2 )
    {
        if( fData->EChi2S < fCut_EChi2_min || fData->EChi2S > fCut_EChi2_max )
	{
	   if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eEnergyRec );
	   return false;
        }
        if( fData->ErecS < fCut_Erec_min || fData->ErecS > fCut_Erec_max )
	{
	   if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eEnergyRec );
	   return false;
        }
        if( fData->dES < fCut_dE_min || fData->dES > fCut_dE_max )
	{
	   if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eEnergyRec );
	   return false;
        }
    }
    else if( iEnergyReconstructionMethod == 100 )
    {
        return true;
    }
// unknown reconstruction method
    else
    {
       if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eError );
       return false;
    }

    return true;
}

/*

  apply gamma/hadron separation cut

*/

bool VGammaHadronCuts::isGamma( int i, bool bCount, bool fIsOn)
{
    if( fDebug )
    {
       cout << "VGammaHadronCuts::isGamma: event " << i;
       cout << ", cut selector " << fGammaHadronCutSelector;
       cout << " (" << fGammaHadronCutSelector % 10 << ", " << fGammaHadronCutSelector / 10 << ")";
       cout << endl;
    }

/////////////////////////////////////////////////////////////////////////////
// apply box cuts  (e.g. MSCW/MSCL or MWR/MLR)
    if( fGammaHadronCutSelector % 10 <= 3 )
    {
	if( fDebug ) cout << "VGammaHadronCuts::isGamma: applyStereoShapeCuts" << endl; 
        if( !applyStereoShapeCuts() )
        {
            if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eIsGamma );
            return false;
        }
// all cut selectors >= 10 are different
        if( fGammaHadronCutSelector < 10 ) return true;
    }
/////////////////////////////////////////////////////////////////////////////
// apply probability threshold cut (e.g. random forest cuts)
    if( fGammaHadronCutSelector / 10 == 1 || fGammaHadronCutSelector / 10 == 2 )
    {
	if( fDebug ) cout << "VGammaHadronCuts::isGamma: applyProbabilityCut" << endl; 
        if( !applyProbabilityCut( i, fIsOn ) )
        {
            if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eIsGamma );
            return false;
        }
    }
/////////////////////////////////////////////////////////////////////////////
// apply cut using TMVA reader
    else if( fGammaHadronCutSelector / 10 == 4 )
    {
        if( fDebug ) cout << "VGammaHadronCuts::isGamma: applyTMVACut" << endl; 
        if( !applyTMVACut( i ) )
        {
           if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eIsGamma );
           return false;
        }
    }
/////////////////////////////////////////////////////////////////////////////
// apply Frogs Cuts
    else if( fGammaHadronCutSelector / 10 == 5 )
    {
        if( fDebug ) cout << "VGammaHadronCuts::isGamma: apply Frogs Cuts" << endl; 
        if( !applyFrogsCut( i, fIsOn ) )
        {
           if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eIsGamma );
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
      cout << ", probability threshold/MVA cut " << fTMVA_MVACut.size();
      cout << " (" << fTMVAEvaluator << ")";
      cout << endl;
   }
   fTMVA_EvaluationResult = -99.;
   if( fTMVAEvaluator )
   {
      bool i_TMVA_Evaluation = fTMVAEvaluator->evaluate();
      fTMVA_EvaluationResult = fTMVAEvaluator->getTMVA_EvaluationResult();
      return i_TMVA_Evaluation;
   }

   return false;
}

/*
   appy Cuts to frogs data.
*/

bool VGammaHadronCuts::applyFrogsCut( int i, bool fIsOn )
{
   if( fDebug )
   {
      cout << "VGammaHadronCuts::applyFrogsCut event " << i << endl;
   }

    int NTELCUT;
    double SGCuts;

// 1 % Crab

    if( fData->frogsEnergy > -99. && fData->frogsEnergy < -0.6 ) SGCuts = 0.65;
    if( fData->frogsEnergy > -0.6 && fData->frogsEnergy < -0.4 ) SGCuts = 1.00;
    if( fData->frogsEnergy > -0.4 && fData->frogsEnergy < -0.2 ) SGCuts = 1.70;
    if( fData->frogsEnergy > -0.2 && fData->frogsEnergy <  0.0 ) SGCuts = 2.50;
    if( fData->frogsEnergy >  0.0 && fData->frogsEnergy <  99. ) SGCuts = 500.;

    if( fData->frogsEnergy > -99. && fData->frogsEnergy < -0.6 ) NTELCUT = 2;
    if( fData->frogsEnergy > -0.6 && fData->frogsEnergy < -0.4 ) NTELCUT = 2;
    if( fData->frogsEnergy > -0.4 && fData->frogsEnergy < -0.2 ) NTELCUT = 2;
    if( fData->frogsEnergy > -0.2 && fData->frogsEnergy <  0.0 ) NTELCUT = 1;
    if( fData->frogsEnergy >  0.0 && fData->frogsEnergy <  99. ) NTELCUT = 0;

    int NTEL=0;
    if( fData->frogsTelGoodnessImg0 > -10. && fData->frogsTelGoodnessImg0 < SGCuts ) NTEL++;
    if( fData->frogsTelGoodnessImg1 > -10. && fData->frogsTelGoodnessImg1 < SGCuts ) NTEL++;
    if( fData->frogsTelGoodnessImg2 > -10. && fData->frogsTelGoodnessImg2 < SGCuts ) NTEL++;
    if( fData->frogsTelGoodnessImg3 > -10. && fData->frogsTelGoodnessImg3 < SGCuts ) NTEL++;

    if( NTEL <= NTELCUT ) return false;

   return true;

}

/*

  use cut selector from a friend tree to the given data tree

*/
bool VGammaHadronCuts::applyProbabilityCut( int i, bool fIsOn )
{
// tree exists and has the requested entry
   if( fProbabilityCut_Tree && fProbabilityCut_Tree->GetEntry( i ) )
   {
// check cut quality
      if( fProbabilityCut_QualityFlag > 0 )
      {
            if(fProbabilityCutRangeLower.size() != fProbabilityCutRangeUpper.size())
            {
                cout << "Error in definitions of RF probablity ranges" << endl
                      << "RFCutLowerVals and  RFCutLowerVals have different numbers of entries in cut file" << endl;
                exit( -1 );
            }
            else
            {
                if( fProbabilityCut_ProbID < fProbabilityCut_NSelectors && fProbabilityCut_NSelectors < VANACUTS_PROBSELECTIONCUTS_MAX )
                {
                    for(unsigned int dex = 0; dex < fProbabilityCutRangeLower.size(); dex++)
                    {
                        if( fIsOn && fProbabilityCut_SelectionCut[fProbabilityCut_ProbID] >= fProbabilityCutRangeLower[dex]
                                  && fProbabilityCut_SelectionCut[fProbabilityCut_ProbID] <  fProbabilityCutRangeUpper[dex] )
                        {
                            return true;
                        }
                        if( !fIsOn && fProbabilityCut_SelectionCut[fProbabilityCut_ProbID] >= -fProbabilityCutRangeLower[dex]
                                   && fProbabilityCut_SelectionCut[fProbabilityCut_ProbID] <  -fProbabilityCutRangeUpper[dex] )
                        {
                            return true;
                        }
                    }
                }
           }
      }
    }

    return false;
}

/*!

    apply mean reduced scaled cuts

*/
bool VGammaHadronCuts::applyMeanReducedScaledStereoShapeCuts()
{
    if( fData->MSCW > fCut_MSCW_max ) return false;
    if( fData->MSCW < fCut_MSCW_min ) return false;
    if( fData->MSCL > fCut_MSCL_max ) return false;
    if( fData->MSCL < fCut_MSCL_min ) return false;

    return true;
}

/*!

    apply mean scaled cuts

*/
bool VGammaHadronCuts::applyMeanScaledStereoShapeCuts()
{
    if( fData->MWR > fCut_MSW_max ) return false;
    if( fData->MWR < fCut_MSW_min ) return false;
    if( fData->MLR > fCut_MSL_max ) return false;
    if( fData->MLR < fCut_MSL_min ) return false;

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
	 if( fData->ntubes[i] >= fCut_Ntubes_max ||  fData->ntubes[i] <= fCut_Ntubes_min ) continue;
	 if( fData->size[i] <= fCut_Size_min || fData->size[i] >= fCut_Size_max )          continue;

	 fMeanImageWidth += fData->width[i];
	 fMeanImageLength += fData->length[i];
	 fMeanImageDistance += fData->dist[i];
	 intel++;
    }
    if( intel > 0 )
    {
	 fMeanImageWidth      /= (double)intel;
	 fMeanImageLength     /= (double)intel;
	 fMeanImageDistance   /= (double)intel;
    }
    else return false;
// apply cuts
    if( fMeanImageDistance <= fCut_MeanImageDistance_min || fMeanImageDistance >= fCut_MeanImageDistance_max ) return false;
    if( fMeanImageLength <= fCut_MeanImageLength_min || fMeanImageLength >= fCut_MeanImageLength_max )         return false;
    if( fMeanImageWidth <= fCut_MeanImageWidth_min || fMeanImageWidth >= fCut_MeanImageWidth_max )             return false;

    return true;
}

/*

   stereo shape cuts

*/
bool VGammaHadronCuts::applyStereoShapeCuts()
{
/////////////////////////////////////////////////////////////////////////////
// cut selector 2 is always true
    if( fGammaHadronCutSelector % 10 == 2 ) return true;

/////////////////////////////////////////////////////////////////////////////
// apply cuts
/////////////////////////////////////////////////////////////////////////////
// MSCW/MSCL cuts
    if( fGammaHadronCutSelector % 10 < 1 )
    {
        if( !applyMeanReducedScaledStereoShapeCuts() ) return false;
    }
/////////////////////////////////////////////////////////////////////////////
// mean width/length cuts
// (not scaled width, this calculation is MC independent)
    else if( fGammaHadronCutSelector % 10 == 1 )
    {
       if( !applyMeanStereoShapeCuts() ) return false;
    }
/////////////////////////////////////////////////////////////////////////////
// MWR/MLR cuts
    else if( fGammaHadronCutSelector % 10 == 3 )
    {
       if( !applyMeanScaledStereoShapeCuts() ) return false;
    }

/////////////////////////////////////////////////////////////////////////////
// emission height cuts
    if( fData->EmissionHeight > 0. && fData->EmissionHeight > fCut_Emmission_max ) return false;
    if( fData->EmissionHeight > 0. && fData->EmissionHeight < fCut_Emmission_min ) return false;

    return true;
}


/*
  initialize gamma/hadron cuts

  (see description of cuts at the top of this file)
*/
void VGammaHadronCuts::initializeCuts( int irun, string iFile )
{
    cout << "VGammaHadronCuts::initializeCuts selector ID ";
    cout << fGammaHadronCutSelector;
    cout << ", " << fDirectionCutSelector << endl;

// probability cuts
    if( fGammaHadronCutSelector / 10 >= 1 && fGammaHadronCutSelector / 10 <= 3 )
    {
        if( irun > 0 )              initProbabilityCuts( irun );
        else if( iFile.size() > 0 ) initProbabilityCuts( iFile );
        else
        {
            cout << "VGammaHadronCuts::initializeCuts: failed setting probability cuts for " << irun << " " << iFile << endl;
            cout << "exiting..." << endl;
            exit( -1 );
        }
    }
// TMVA cuts
    else if( fGammaHadronCutSelector / 10 == 4 )
    {
        if( !initTMVAEvaluator( fTMVAWeightFile, fTMVAWeightFileIndex_min, fTMVAWeightFileIndex_max ) )
        {
           cout << "VGammaHadronCuts::initializeCuts: failed setting TMVA reader for " << fTMVAWeightFile;
           cout << "(" << fTMVAWeightFileIndex_min << "," << fTMVAWeightFileIndex_max << ")" << endl;
           cout << "exiting..." << endl;
           exit( -1 );
        }
    }
// phase cuts
    if( useOrbitalPhaseCuts() )
    {
       if( irun > 0 )              initPhaseCuts( irun );
       else if( iFile.size() > 0 ) initPhaseCuts( iFile );
       else
       {
            cout << "VGammaHadronCuts::initializeCuts: failed setting phase cuts for " << irun << " " << iFile << endl;
            cout << "exiting..." << endl;
            exit( -1 );
        }
    }
}

bool VGammaHadronCuts::initTMVAEvaluator( string iTMVAFile, unsigned int iTMVAWeightFileIndex_min, unsigned int iTMVAWeightFileIndex_max )
{
    TDirectory *cDir = gDirectory;

    fTMVAEvaluator = new VTMVAEvaluator();
    fTMVAEvaluator->setDebug( fDebug );
// set parameters for optimal MVA cut value search
    if( fTMVAOptimizeSignalEfficiencyParticleNumberFile.size() > 0. )
    {
       fTMVAEvaluator->setSensitivityOptimizationParameters( fTMVAOptimizeSignalEfficiencySignificance_Min,
                                                             fTMVAOptimizeSignalEfficiencySignalEvents_Min,
                                                             fTMVAOptimizeSignalEfficiencyObservationTime_h,
							     1./5. );
       fTMVAEvaluator->setSensitivityOptimizationFixedSignalEfficiency( fTMVAFixedSignalEfficiencyMinEnergy, 
                                                                        fTMVAFixedSignalEfficiencyAboveMinEnergy );
       fTMVAEvaluator->setParticleNumberFile( fTMVAOptimizeSignalEfficiencyParticleNumberFile );
    }
// set a constant signal efficiency
    else if( fTMVASignalEfficiency.size() > 0 )
    {
       fTMVAEvaluator->setSignalEfficiency( fTMVASignalEfficiency );
    }
// set a fixed probability threshold or (for TMVA) a fixed MVA cut value
    else if( fTMVA_MVACut.size() > 0 )
    {
       fTMVAEvaluator->setTMVACutValue( fTMVA_MVACut );
    }
    else
    {
       cout << "VGammaHadronCuts::initTMVAEvaluator error: unclear TMVA cut settings" << endl;
       cout << "\t fTMVASignalEfficiency: " << fTMVASignalEfficiency.size() << endl;
       cout << "\t fTMVAProbabilityThreshold: " << fTMVA_MVACut.size() << endl;
       cout << "exiting... " << endl;
       exit( -1 );
    }
    fTMVAEvaluator->setTMVAMethod( fTMVA_MVAMethod );
// read MVA weight files; set MVA cut values (e.g. find optimal values)
    if( !fTMVAEvaluator->initializeWeightFiles( iTMVAFile, iTMVAWeightFileIndex_min, iTMVAWeightFileIndex_max ) )
    {
       cout << "VGammaHadronCuts::initTMVAEvaluator: error while initializing TMVA weight files" << endl;
       cout << "exiting... " << endl;
       exit( -1 );
    }
    fTMVAEvaluator->printSignalEfficiency();

    if( fDirectionCutSelector == 3 ) fTMVAEvaluator->setIgnoreTheta2Cut( false );
    else                             fTMVAEvaluator->setIgnoreTheta2Cut( true  );
    fTMVABoxCut_Theta2_max = fTMVAEvaluator->getBoxCut_Theta2_Graph();
    if( fTMVABoxCut_Theta2_max )
    {
       cout << "VGammaHadronCuts::initTMVAEvaluator: found theta2_max graph from TMVA" << endl;
       fTMVABoxCut_Theta2_max->SetName( "TMVAtheta2" );
    }

    if( cDir ) cDir->cd();

    return !fTMVAEvaluator->IsZombie();
}

bool VGammaHadronCuts::setDataTree( CData* idata )
{
    fData = idata;

    if( !fData ) return false;

    if( fTMVAEvaluator ) fTMVAEvaluator->initializeDataStrutures( fData );

    return true;
}


bool VGammaHadronCuts::initProbabilityCuts( int irun )
{
    ostringstream iFile;
    iFile << fDataDirectory << "/" << irun << ".mscw.rf.root";

    return initProbabilityCuts( iFile.str() );
}


bool VGammaHadronCuts::initProbabilityCuts( string iFile )
{
    TDirectory *cDir = gDirectory;

    fProbabilityCut_File = new TFile( iFile.c_str() );
    if( fProbabilityCut_File->IsZombie() )
    {
        cout << "Error while opening file with probability cuts: " << iFile << endl;
        exit( 0 );
    }
    cout << "\t opening file with probability cuts: " << fProbabilityCut_File->GetName() << endl;

    fProbabilityCut_Tree = (TTree*)gDirectory->Get( "rf" );
    if( !fProbabilityCut_Tree )
    {
        cout << "Error: could not find tree with probability cuts" << endl;
        exit( 0 );
    }
    fProbabilityCut_Tree->SetBranchAddress( "cut", &fProbabilityCut_QualityFlag );
    if( fProbabilityCut_Tree->GetBranchStatus( "Ng" ) ) fProbabilityCut_Tree->SetBranchAddress( "Ng", &fProbabilityCut_NSelectors );
    else                                                fProbabilityCut_NSelectors = 2;
    fProbabilityCut_Tree->SetBranchAddress( "g", fProbabilityCut_SelectionCut );

    if( cDir ) cDir->cd();

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
    TDirectory *cDir = gDirectory;

    fPhaseCut_File = new TFile( iDir.c_str() );
    if( fPhaseCut_File->IsZombie() )
    {
        cout << "Error while opening file with phase cuts: " << iDir << endl;
        exit( 0 );
    }
    cout << "\t opening file with phase cuts: " << fPhaseCut_File->GetName() << endl;

    fPhaseCut_Tree = (TTree*)gDirectory->Get( "phase" );
    if( !fPhaseCut_Tree )
    {
        cout << "Error: could not find tree with phase cuts" << endl;
        exit( 0 );
    }
    fPhaseCut_Tree->SetBranchAddress( "phase", &fOrbitalPhase );

    if( cDir ) cDir->cd();

    return true;
}


/*

   check that event is inside a certain region in the camera

*/
bool VGammaHadronCuts::applyInsideFiducialAreaCut( bool bCount )
{

    if( fData->fFrogs == 1 )
     return applyInsideFiducialAreaCut( fData->frogsXS, fData->frogsYS, bCount );
    else 
     return applyInsideFiducialAreaCut( fData->Xoff, fData->Yoff, bCount );
}

bool VGammaHadronCuts::applyInsideFiducialAreaCut( float Xoff, float Yoff, bool bCount )
{
    double xy = Xoff*Xoff + Yoff*Yoff;

    if( xy > fCut_CameraFiducialSize_max*fCut_CameraFiducialSize_max )
    {
        if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eXYoff );
        return false;
    }

    if( fCut_CameraFiducialSize_min >= 0. )
    {
        if( xy < fCut_CameraFiducialSize_min*fCut_CameraFiducialSize_min )
        {
            if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eXYoff );
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
    if( !fData->isMC() ) return true;

    double xy = xoff*xoff + yoff*yoff;

    if( xy > fCut_CameraFiducialSize_MC_max*fCut_CameraFiducialSize_MC_max )
    {
        if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eMC_XYoff );
        return false;
    }

    if( fCut_CameraFiducialSize_MC_min >= 0. )
    {
        if( xy < fCut_CameraFiducialSize_MC_min*fCut_CameraFiducialSize_MC_min )
        {
            if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eMC_XYoff );
            return false;
        }
    }

    return true;
}

/*

   check telescope type (e.g. remove all LSTs)

*/
bool VGammaHadronCuts::applyTelTypeTest( bool bCount )
{
   bool icut = false;

   if( fNTelTypeCut.size() == 0 ) return true;

   for( unsigned int i = 0; i < fNTelTypeCut.size(); i++ ) icut = ( icut || fNTelTypeCut[i]->test( fData ) );

   if( bCount && !icut ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eTelType );

   return icut;
}


/*!

  NOTE: THIS IS NOTE USED

  check if core is inside a certain area relative to centre of array (ground, not shower coordinates)

  fCut_CoreDistanceToArrayCentreY_min && fCut_CoreDistanceToArrayCentreY_max == 0: cut area is circle with radius fCut_CoreDistanceToArrayCentreX_max
  otherwise: cut area is a rectangle

  iMC = true:   use MC core
  iMC = false:  use reconstructed core position
*/
bool VGammaHadronCuts::applyShowerCoreCuts( bool iMC )
{
    double xcore = fData->Xcore - fArrayCentre_X;
    double ycore = fData->Ycore - fArrayCentre_Y;
// use MC core position
    if( iMC )
    {
        xcore = fData->MCxcore - fArrayCentre_X;
        ycore = fData->MCycore - fArrayCentre_Y;
    }
///////////////////////////////////////////////
// radius cuts (Y_min and Y_max is 0)
    if( TMath::Abs( fCut_CoreDistanceToArrayCentreY_min ) < 1.e-5 && TMath::Abs( fCut_CoreDistanceToArrayCentreY_max ) < 1.e-5 )
    {
        if( sqrt( xcore*xcore+ycore*ycore ) < fCut_CoreDistanceToArrayCentreX_min ) return false;
        if( sqrt( xcore*xcore+ycore*ycore ) > fCut_CoreDistanceToArrayCentreX_max ) return false;
    }
    else
///////////////////////////////////////////////
// box cut
    {
        if( xcore < fCut_CoreDistanceToArrayCentreX_min - fCut_CoreDistanceEdgeSize ) return false;
        if( xcore > fCut_CoreDistanceToArrayCentreX_max + fCut_CoreDistanceEdgeSize ) return false;
        if( ycore < fCut_CoreDistanceToArrayCentreY_min - fCut_CoreDistanceEdgeSize ) return false;
        if( ycore > fCut_CoreDistanceToArrayCentreY_max + fCut_CoreDistanceEdgeSize ) return false;
    }

    return true;
}

/* DISABLED DO NOT USE
void VGammaHadronCuts::setShowerCoreCuts( double xmin, double xmax, double ymin, double ymax, double iEdge )
{
    fCut_CoreDistanceToArrayCentreX_min = xmin;
    fCut_CoreDistanceToArrayCentreX_max = xmax;
    fCut_CoreDistanceToArrayCentreY_min = ymin;
    fCut_CoreDistanceToArrayCentreY_max = ymax;
    if( iEdge >= 0. ) fCut_CoreDistanceEdgeSize = iEdge;
    cout << "setting shower core cuts: " << fCut_CoreDistanceToArrayCentreX_min << "\t" << fCut_CoreDistanceToArrayCentreX_max;
    cout << "\t" << fCut_CoreDistanceToArrayCentreY_min << "\t" << fCut_CoreDistanceToArrayCentreY_max << "\t" << fCut_CoreDistanceEdgeSize << endl;
}
*/

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
      if( fData->isMC() ) x0 = fData->MCxoff;
      else                x0 = 0.;
   }
   if( y0 < -99990. )
   {
      if( fData->isMC() ) y0 = fData->MCyoff;
      else                y0 = 0.;
   }

// calculate theta2
   theta2 = (fData->Xoff-x0)*(fData->Xoff-x0) + (fData->Yoff-y0)*(fData->Yoff-y0);

////////////////////////////////////////////////////////////////////////////////////////////////////
// fetch theta2 cut (max) (might be energy dependent)
   double i_theta2_cut_max = -1.;
   if( fEnergyReconstructionMethod == 0 )
   {
      i_theta2_cut_max = getTheta2Cut_max( fData->Erec );
   }
   else if( fEnergyReconstructionMethod == 1 || fEnergyReconstructionMethod == 2 )
   {
      i_theta2_cut_max = getTheta2Cut_max( fData->ErecS );
   }

// direction cut (ring around center of camera)
   if( theta2 < i_theta2_cut_max && theta2 > fCut_Theta2_min )
   {
      return true;
   }

   if( bCount ) fStats->updateCutCounter( VGammaHadronCutsStatistics::eDirection );

   return false;
}

/*
   fetch theta2 cut (might be energy dependent)

   e      :   [TeV] energy (linear)
*/
double VGammaHadronCuts::getTheta2Cut_max( double e )
{
    double theta_cut_max = -1.;

//////////////////////////////////////////////
// energy independent theta2 cut
//////////////////////////////////////////////
    if( fDirectionCutSelector == 0 )
    {
       theta_cut_max = TMath::Sqrt( fCut_Theta2_max );   // will be later squared
    } 
//////////////////////////////////////////////
// energy dependent theta2 cut
//////////////////////////////////////////////
    if( e > 0. )
    {
       e = log10( e );

/////////////////////////////////////////////
// use a function to get the angular resolution
       if( fDirectionCutSelector == 1 && fF1AngRes )
       {
// energy outside of functions range:, return edge values
	  if( e < fF1AngRes->GetXmin() )       e = fF1AngRes->GetXmin();
	  else if( e > fF1AngRes->GetXmax() )  e = fF1AngRes->GetXmax();

// get angular resolution and apply scaling factor
	  theta_cut_max  = fF1AngRes->Eval( e );
       }
/////////////////////////////////////////////
// use IRF graph for angular resolution
       else if( (fDirectionCutSelector == 1 || fDirectionCutSelector == 2 ) && fIRFAngRes )
       {
// get theta2 cut
	  theta_cut_max  = fIRFAngRes->Eval( e );

// for e outside of graph range, return edge values
	  if( fIRFAngRes->GetN() > 0 && fIRFAngRes->GetX() && fIRFAngRes->GetY() )
	  {
	     if( e < fIRFAngRes->GetX()[0] )                    theta_cut_max = fIRFAngRes->GetY()[0];
	     if( e > fIRFAngRes->GetX()[fIRFAngRes->GetN()-1] ) theta_cut_max = fIRFAngRes->GetY()[fIRFAngRes->GetN()-1];
	  }
       }
/////////////////////////////////////////////
// use TMVA determined cut
       else if ( fDirectionCutSelector == 3 )
       {
          theta_cut_max = -1.;
       }
       else if( fDirectionCutSelector == 4 && fTMVAEvaluator && fTMVAEvaluator->getTMVAThetaCutVariable() )
       {
           theta_cut_max = fTMVAEvaluator->getBoxCut_Theta2( e );
	   if( theta_cut_max > 0. ) theta_cut_max = TMath::Sqrt( theta_cut_max );
	   else                     theta_cut_max = 0.;
       }
/////////////////////////////////////////////
// use a graph with theta2 cuts
       else if( fDirectionCutSelector == 5 && fTMVABoxCut_Theta2_max )
       {
	  theta_cut_max = fTMVABoxCut_Theta2_max->Eval( log10( e ) );
// for e outside of graph range, return edge values
	  if( fTMVABoxCut_Theta2_max->GetN() > 0 && fTMVABoxCut_Theta2_max->GetX() && fTMVABoxCut_Theta2_max->GetY() )
	  {
	     if( e < fTMVABoxCut_Theta2_max->GetX()[0] ) 
	     {
	         theta_cut_max = fTMVABoxCut_Theta2_max->GetY()[0];
             }
	     if( e > fTMVABoxCut_Theta2_max->GetX()[fTMVABoxCut_Theta2_max->GetN()-1] )
	     {
	         theta_cut_max = fTMVABoxCut_Theta2_max->GetY()[fTMVABoxCut_Theta2_max->GetN()-1];
             }
	  }
	  if( theta_cut_max > 0. ) theta_cut_max = TMath::Sqrt( theta_cut_max );
	  else                     theta_cut_max = 0.;
       }
    }

// apply scale factors
    theta_cut_max *= fAngRes_ScalingFactor;
    if( theta_cut_max < fAngRes_AbsoluteMinimum ) return fAngRes_AbsoluteMinimum*fAngRes_AbsoluteMinimum;
    if( theta_cut_max > fAngRes_AbsoluteMaximum ) return fAngRes_AbsoluteMaximum*fAngRes_AbsoluteMaximum;

    return theta_cut_max*theta_cut_max;
}

/*

   read angular resolution from root file

*/

bool VGammaHadronCuts::initAngularResolutionFile()
{

// open angular resolution file
   fFileAngRes = new TFile( fFileNameAngRes.c_str() );
   if( fFileAngRes->IsZombie() )
   {
// try it at the default location
       string iTempFileL = VGlobalRunParameter::getDirectory_EVNDISPAnaData();
       iTempFileL += "/GammaHadronCutFiles/" + fFileNameAngRes;
       fFileAngRes = new TFile( iTempFileL.c_str() );
       if( fFileAngRes->IsZombie() )
       {
	  cout << "VGammaHadronCuts::initAngularResolutionFile: error open angular resolution file: " << fFileNameAngRes << endl;
	  cout << " (tried also " << iTempFileL << ")" << endl;
	  return false;
       }
   }

/////////////////////////
// get energy dependent theta values (probably angular resolution)

// read angular resolution as function (TF1) from a fit
   if( fF1AngResName != "IRF" && fF1AngResName != "GRAPH" )
   {
// get energy dependent theta values (probably angular resolution)
      if( fFileAngRes->Get( fF1AngResName.c_str() ) )
      {
	 fF1AngRes = (TF1*)fFileAngRes->Get( fF1AngResName.c_str() )->Clone();
	 cout << "VGammaHadronCuts::initAngularResolutionFile: read angular resolution fit function from file " << fF1AngResName << " : " << endl;
	 fF1AngRes->Print();
      }
      else
      {
	  cout << "VGammaHadronCuts::initAngularResolutionFile: error finding angular resolution function with name ";
	  cout << fF1AngResName << " in " << fFileAngRes->GetName() << endl;
	  return false;
      }
   }
// read angular resolution from instrument response function tree
   else if( fF1AngResName == "IRF" )
   {
      char iTreeName[200];
      sprintf( iTreeName, "t_angular_resolution" );
      if( getAngularResolutionContainmentRadius() - 68 != 0 ) sprintf( iTreeName, "%s_%03dp", iTreeName, getAngularResolutionContainmentRadius() );

      cout << "VGammaHadronCuts::initAngularResolutionFile: reading angular resolution graph from file (" << iTreeName << "):" << endl;
      cout << fFileAngRes->GetName() << endl;
      TTree *t = (TTree*)fFileAngRes->Get( iTreeName );
      if( !t )
      {
         cout << "VGammaHadronCuts::initAngularResolutionFile: error finding tree with instrument response function for angular resolution " << endl;
	 return false;
      }
// TODO: this should go in the future
// (currently there should be only one entry in the angular resolution tree)
      if( t->GetEntries() != 1 )
      {
         cout << "VGammaHadronCuts::initAngularResolutionFile error: invalid number of entries in instrument response function tree ";
	 cout << " (should be 1, is " << t->GetEntries() << " )" << endl;
	 cout << "   (as we do not know which tree entry to select...)" << endl;
	 return false;
      }
// read the tree data
      VInstrumentResponseFunctionData* c = 0;
      TBranch *br = t->GetBranch( "IRF" );
      br->SetAddress( &c );
      t->GetEntry( 0 );
      if( c && c->fResolutionGraph.size() > VInstrumentResponseFunctionData::E_DIFF+1 && c->fResolutionGraph[VInstrumentResponseFunctionData::E_DIFF] )
      { 
          fIRFAngRes = (TGraphErrors*)c->fResolutionGraph[VInstrumentResponseFunctionData::E_DIFF]->Clone();
	  if( !fIRFAngRes )
	  {
	     cout << "VGammaHadronCuts::initAngularResolutionFile error: reading angular resolution graph from " << fF1AngResName << endl;
	     return false;
          }
	  fIRFAngRes->SetName( "IRFAngRes" );
      }
   }
// read a single graph with angular resolution from file
   else if( fF1AngResName == "GRAPH" )
   {
       TGraphErrors *g =(TGraphErrors*)fFileAngRes->Get( "gAngularResolution" );
       if( !g )
       {
          cout << "VGammaHadronCuts::initAngularResolutionFile error: reading angular resolution graph from " << fF1AngResName << endl;
	  return false;
       }
       setIRFGraph( g );
   }
   if( fFileAngRes ) fFileAngRes->Close();

   return true;
}

bool VGammaHadronCuts::setIRFGraph( TGraphErrors *g )
{
   if( !g )
   {
      cout << "VGammaHadronCuts::setIRFGraph warning: IRF pointer is zero" << endl;
      return false;
   }

   fIRFAngRes = (TGraphErrors*)g->Clone();
   fIRFAngRes->SetName( "IRFAngRes" );

// print results
//   cout << "replaced IRF graph for direction cut" << endl;
//   printDirectionCuts();

   return true;
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


double VGammaHadronCuts::getProbabilityCutAlpha(bool fIsOn)
{
    if(fProbabilityCutRangeLower.size() != fProbabilityCutRangeUpper.size())
    {
        cout << "Error in definitions of RF probablity ranges" << endl;
        cout << "RFCutLowerVals and RFCutLowerVals have different numbers of entries in cut file" << endl;
        exit(-1);
    }
//////////////////////////////////////////////////////////////
// return 1 if probability cuts are not set
    if( fProbabilityCutRangeLower.size() == 0 && fProbabilityCutRangeUpper.size() == 0 ) 
    {
       return 1.;
    }
//////////////////////////////////////////////////////////////

    double on_size = 0;
    double off_size = 0;
    for(unsigned int i = 0; i < fProbabilityCutRangeLower.size(); i++)
    {
        if(fProbabilityCutRangeLower[i] >= 0 && fProbabilityCutRangeUpper[i] >= 0)
        {
            on_size = on_size + (fProbabilityCutRangeUpper[i] - fProbabilityCutRangeLower[i]);
        }
        else if(fProbabilityCutRangeLower[i] <= 0 && fProbabilityCutRangeUpper[i] <= 0)
        {
            off_size = off_size - (fProbabilityCutRangeUpper[i] - fProbabilityCutRangeLower[i]);
        }
        else
        {
           cout << "Error in definitions of RF probablity ranges" << endl;
           cout << "One pair of RFCutLowerVals and RFCutLowerVals values have oposite sign" << endl;
        }
    }
    
    if(fIsOn)
    {
        return on_size;
    }
    else
    {
        return off_size;
    }

}

void VGammaHadronCuts::terminate()
{
   SetName( "GammaHadronCuts" );

   if( fStats->getDataTree() )
   {
      fStats->terminate();
      fStats->getDataTree()->Write();
   }   

   Write();
}

void VGammaHadronCuts::printSignalEfficiency()
{
   if( fTMVASignalEfficiency.size() == 0 )
   {
       cout << "no signal efficiency set" << endl;
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
      cout << "MVA cut for energy bin " << iIter->first << ": ";
      cout << iIter->second << endl;
   }
}


/*

    apply orbital or pulsar phase cuts - info is store in a tree called PHASE

*/
bool VGammaHadronCuts::applyPhaseCut(int i)
{
   if( useOrbitalPhaseCuts() )
   {
      if( fPhaseCut_Tree && fPhaseCut_Tree->GetEntry(i) )
      {
	 if( fOrbitalPhase >= fOrbitalPhase_min && fOrbitalPhase < fOrbitalPhase_max )
	 {
	     return true;
	 }
	 else return false;
      }
      else return false;
   }
  
  return true;
 
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// cut on number of images per telescope type depend
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VNTelTypeCut::VNTelTypeCut()
{
   fNTelType_min = 0;
}


bool VNTelTypeCut::test( CData *c )
{
   if( !c ) return false;

   unsigned int ntel_type = 0;
   for( unsigned int i = 0; i < fTelType_counter.size(); i++ )
   {
      if( (int)fTelType_counter[i] < c->NTtype ) ntel_type += c->NImages_Ttype[fTelType_counter[i]];
   }
// OBS! >=
   if( ntel_type >= fNTelType_min ) return true;

   return false;
}

void VNTelTypeCut::print()
{
   cout << "\t       type cut: mintel >= " << fNTelType_min << " for types ";
   for( unsigned int i = 0; i < fTelType_counter.size(); i++ ) cout << fTelType_counter[i] << " ";
   cout << endl;
}

