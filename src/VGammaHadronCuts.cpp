/*! \class VGammaHadronCuts
  \brief class containing parameter cut definitions

  Revision $Id: VGammaHadronCuts.cpp,v 1.19.2.3.4.4.4.1.8.3.4.3.2.6.2.7.2.1.4.2.2.5.2.18.2.4.2.10 2011/04/11 16:10:27 gmaier Exp $

  List of cut selectors:

  cut selector values consist of two digits: ID1+ID2*10

  ID2:

     0: apply gamma/hadron cuts on parameters in given data tree
     1: apply gamma/hadron cuts on probabilities given by a friend to the data tree (e.g. random forest analysis)
     2: same as 2
     3: apply cuts on probabilities given by a friend to the data tree already at the level of the event quality level (e.g. of use for analysis of certain binary phases only) 
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


  \author
  Jamie Holder, Gernot Maier

*/

#include "VGammaHadronCuts.h"

VGammaHadronCuts::VGammaHadronCuts()
{
    resetCutValues();

    fCutSelector = 0;
    bMCCuts = false;

    fData = 0;
    fNTel = 0;
    fNLTrigs = 0;
    fDataDirectory = "";

    fMeanWidth = 0.;
    fMeanLength = 0.;
    fMeanDistance = 0.;

// use probabilities for cuts
    fProbabilityCut_File = 0;
    fProbabilityCut_Tree = 0;
    fProbabilityCut_QualityFlag = 0;
    fProbabilityCut_NSelectors = VANACUTS_PROBSELECTIONCUTS_MAX;
    fProbabilityCut_ProbID = 0;
    for( unsigned int i = 0; i < fProbabilityCut_NSelectors; i++ ) fProbabilityCut_SelectionCut[i] = -1.;

// TMVA evaluator
    fTMVAEvaluator = 0;
    fTMVAWeightFileIndex_min = 0;
    fTMVAWeightFileIndex_max = 0;
    fTMVAWeightFile = "";
    fTMVAIgnoreTheta2Cut = false;
    fTMVASignalEfficiency = -99.;
    fTMVAProbabilityThreshold = -99.;
    fTMVAOptimizeSignalEfficiencyParticleNumberFile = "";
    fTMVAOptimizeSignalEfficiencySourceStrengthCU = 0.001;     // optimize for mili-Crab source

// energy dependent theta2 cut
    fFileNameAngRes = "";
    fFileAngRes = 0;
    fF1AngResName = "";
    fF1AngRes = 0;
    fAngRes_ScalingFactor = 1.;
    fAngRes_AbsoluteMinimum = 0.;
    fAngRes_AbsoluteMaximum = 1.e99;
    fIRFAngRes = 0;

// statistics
    fStats = new VGammaHadronCutsStats(); 

    setArrayCentre();
}

VGammaHadronCuts::~VGammaHadronCuts()
{
   if( fStats ) delete fStats;
}


void VGammaHadronCuts::resetCutValues()
{
//! Best cuts for data from telescope 1 (as of 2005)

    fAlpha_min=0.0;
    fAlpha_max=8.0;
    fDistance_min=0.27;
    fDistance_max=1.06;
    fMaxone_min=-100.;
    fMaxtwo_min=-100.;
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
    fMaxone_min=-100.;
    fMaxtwo_min=-100.;

    fArrayDistance_min=-100;
    fArrayDistance_max=100;
    fArrayTheta2_min=-100;
    fArrayTheta2_max=100;
    fArrayChi2_min=-1e6;
    fArrayChi2_max=1e99;
    fArrayLength_min=-100;
    fArrayLength_max=100;
    fArrayWidth_min=-100;
    fArrayWidth_max=100;
    fArrayAsymm_min=-100;
    fArrayAsymm_max=100;
    fArraySize_min=-1000;
    fArraySize_max=1e99;
    fArrayNtubes_min = 4;
    fArrayNtubes_max = 5000;
    fArrayNTel_min = 2;                           //!< use here >= !!!!
    fArrayNTel_max = 100000;
    fArrayMSCW_min = -1.;
    fArrayMSCW_max = 1.;
    fArrayMSCL_min = -1.;
    fArrayMSCL_max = 1.;
    fArrayMSW_min = -1.;
    fArrayMSW_max = 10.;
    fArrayMSL_min = -1.;
    fArrayMSL_max = 10.;
    fArrayxyoff_min = -1.;
    fArrayxyoff_max = 100.;
    fArrayxyoff_MC_min = -1.;
    fArrayxyoff_MC_max = 100.;
    fArrayCore_min = -99.;
    fArrayCore_max = 99999.;
    fArraydE_min = -99.;
    fArraydE_max = 1.e12;
    fArrayEChi2_min = 0.;
    fArrayEChi2_max = 99999.;
    fArrayErec_min = 0.;
    fArrayErec_max = 1.e10;
    fArrayEmmission_min = 0.;
    fArrayEmmission_max = 1.e12;
    fArrayNImages_min = 0;
    fArrayNImages_max = 100000;

    fArraySizeSecondMax_min = -1000;  
    fArraySizeSecondMax_max = 1.e99; 

    fProbabilityCut = 0.5;

    fCoreX_min = -1.e10;
    fCoreX_max =  1.e10;
    fCoreY_min = -1.e10;
    fCoreY_max =  1.e10;
    fCoreEdge = 0.;
}


bool VGammaHadronCuts::readCuts( string i_cutfilename )
{
    return readCuts( i_cutfilename, true );
}


bool VGammaHadronCuts::readCuts(string i_cutfilename, bool iPrint )
{
    if( iPrint ) return readCuts( i_cutfilename, 1 );

    return readCuts( i_cutfilename, 0 );
}


bool VGammaHadronCuts::readCuts(string i_cutfilename, int iPrint )
{
// reset trigger vector
    fNLTrigs = 0;
    fArrayLTrig.clear();

    ifstream is;
    i_cutfilename = VUtilities::testFileLocation( i_cutfilename, "ParameterFiles", true );
    if( iPrint == 1 )      cout << "\t reading analysis cuts from " << i_cutfilename << endl;
    else if( iPrint == 2 ) cout << "reading analysis cuts from " << i_cutfilename << endl;

    is.open(i_cutfilename.c_str(),ifstream::in);
    if(!is)
    {
        cout << "VGammaHadronCuts::readCuts: cut input file not found, " << i_cutfilename << endl;
        return false;
    }
    string is_line;
    string temp;
    while( getline( is, is_line ) )
    {
        if(  is_line.size() > 0 )
        {
            istringstream is_stream( is_line );
            is_stream >> temp;
            if( temp != "*" ) continue;
            is_stream >> temp;
            if( temp == "alpha" )
            {
                is_stream >> temp;
                fAlpha_min=(atof(temp.c_str()));
                is_stream >> temp;
                fAlpha_max=(atof(temp.c_str()));
            }
            if( temp == "length" )
            {
                is_stream >> temp;
                fLength_min=(atof(temp.c_str()));
                is_stream >> temp;
                fLength_max=(atof(temp.c_str()));
            }
            if( temp == "width" )
            {
                is_stream >> temp;
                fWidth_min=(atof(temp.c_str()));
                is_stream >> temp;
                fWidth_max=(atof(temp.c_str()));
            }
            if( temp == "los" )
            {
                is_stream >> temp;
                fLos_min=(atof(temp.c_str()));
                is_stream >> temp;
                fLos_max=(atof(temp.c_str()));
            }
            if( temp == "maxonetwo" )
            {
                is_stream >> temp;
                fMaxone_min=(atof(temp.c_str()));
                is_stream >> temp;
                fMaxtwo_min=(atof(temp.c_str()));
            }
            if( temp == "asymm" )
            {
                is_stream >> temp;
                fAsymm_min=(atof(temp.c_str()));
                is_stream >> temp;
                fAsymm_max=(atof(temp.c_str()));
            }
            if( temp == "distance" )
            {
                is_stream >> temp;
                fDistance_min=(atof(temp.c_str()));
                is_stream >> temp;
                fDistance_max=(atof(temp.c_str()));
            }
            if( temp == "size" )
            {
                is_stream >> temp;
                fSize_min=(atof(temp.c_str()));
                is_stream >> temp;
                fSize_max=(atof(temp.c_str()));
            }
            if( temp == "arraywidth" )
            {
                is_stream >> temp;
                fArrayWidth_min=(atof(temp.c_str()));
                is_stream >> temp;
                fArrayWidth_max=(atof(temp.c_str()));
            }
            if( temp == "arraylength" )
            {
                is_stream >> temp;
                fArrayLength_min=(atof(temp.c_str()));
                is_stream >> temp;
                fArrayLength_max=(atof(temp.c_str()));
            }
            if( temp == "arrayasymm" )
            {
                is_stream >> temp;
                fArrayAsymm_min=(atof(temp.c_str()));
                is_stream >> temp;
                fArrayAsymm_max=(atof(temp.c_str()));
            }
            if( temp == "arrayntubes" )
            {
                is_stream >> temp;
                fArrayNtubes_min = (atoi(temp.c_str()));
                is_stream >> temp;
                fArrayNtubes_max =(atoi(temp.c_str()));
            }
            if( temp == "arrayntel" )
            {
                is_stream >> temp;
                fArrayNImages_min = (atoi(temp.c_str()));
                is_stream >> temp;
                fArrayNImages_max =(atoi(temp.c_str()));
            }
            if( temp == "arraysize" )
            {
                is_stream >> temp;
                fArraySize_min=(atof(temp.c_str()));
                is_stream >> temp;
                fArraySize_max=(atof(temp.c_str()));
// write warning out about size cuts and number of telescopes
                if( fNTel > 2 && fArraySize_min > 0. )
                {
                    cout << "--------- VGammaHadronCuts warning: ignoring size cut for data with more than 2 telescopes ------" << endl;
                }
            }
            if( temp == "arraychi2" )
            {
                is_stream >> temp;
                fArrayChi2_min=(atof(temp.c_str()));
                is_stream >> temp;
                fArrayChi2_max=(atof(temp.c_str()));
            }
            if( temp == "arraydist" )
            {
                is_stream >> temp;
                fArrayDistance_min=(atof(temp.c_str()));
                is_stream >> temp;
                fArrayDistance_max=(atof(temp.c_str()));
            }
            if( temp == "corearea" )
            {
                is_stream >> temp;
                fCoreX_min = (atof(temp.c_str()));
                is_stream >> temp;
                fCoreX_max = (atof(temp.c_str()));
                is_stream >> temp;
                fCoreY_min = (atof(temp.c_str()));
                is_stream >> temp;
                fCoreY_max = (atof(temp.c_str()));
                if( !is_stream.eof() )
                {
                    is_stream >> temp;
                    fCoreEdge = atof( temp.c_str() );
                }
            }
            if( temp == "arraytheta2" )
            {
                is_stream >> temp;
                fArrayTheta2_min=(atof(temp.c_str()));
                is_stream >> temp;
                fArrayTheta2_max=(atof(temp.c_str()));
            }
            if( temp == "mscw" || temp == "arraymscw" )
            {
                is_stream >> temp;
                fArrayMSCW_min=(atof(temp.c_str()));
                is_stream >> temp;
                fArrayMSCW_max=(atof(temp.c_str()));
            }
            if( temp == "mscl" || temp == "arraymscl" )
            {
                is_stream >> temp;
                fArrayMSCL_min=(atof(temp.c_str()));
                is_stream >> temp;
                fArrayMSCL_max=(atof(temp.c_str()));
            }
            if( temp == "msw" || temp == "arraymsw" )
            {
                is_stream >> temp;
                fArrayMSW_min=(atof(temp.c_str()));
                is_stream >> temp;
                fArrayMSW_max=(atof(temp.c_str()));
            }
            if( temp == "msl" || temp == "arraymsl" )
            {
                is_stream >> temp;
                fArrayMSL_min=(atof(temp.c_str()));
                is_stream >> temp;
                fArrayMSL_max =(atof(temp.c_str()));
            }
            if( temp == "mc_xy_off" || temp == "arrayxyoff_mc" || temp == "mc_xyoff" || temp == "cameraedge_mc" )
            {
                is_stream >> temp;
                fArrayxyoff_MC_min=(atof(temp.c_str()));
                is_stream >> temp;
                fArrayxyoff_MC_max=(atof(temp.c_str()));
                bMCCuts = true;
            }

            if( temp == "xy_off" || temp == "arrayxyoff" || temp == "xyoff" || temp == "cameraedge" )
            {
                is_stream >> temp;
                fArrayxyoff_min=(atof(temp.c_str()));
                is_stream >> temp;
                fArrayxyoff_max=(atof(temp.c_str()));
            }
            if( temp == "arraycore" )
            {
                is_stream >> temp;
                fArrayCore_min = (atof(temp.c_str()));
                is_stream >> temp;
                fArrayCore_max = (atof(temp.c_str()));
            }
// these allow to deselect certain telescope combinations (changed from ltrig to ImgSel)
            if( temp == "arrayltrig" || temp == "selectImageCombination" )
            {
                if( fNTel == 0 || fNTel > 10 ) continue;
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
                    for( unsigned int i = 0; i < fNLTrigs; i++ ) fArrayLTrig.push_back(1);
                }

                is_stream >> temp;
                int index = (atoi(temp.c_str()));
                is_stream >> temp;
                if( index < 0 )
                {
                    for( unsigned int i = 0; i < fArrayLTrig.size(); i++ ) fArrayLTrig[i] = atoi(temp.c_str());
                }
                else if( index < (int)fArrayLTrig.size() )
                {
                    fArrayLTrig[index] = atoi(temp.c_str());
                }

            }
// probability cut variables (e.g. random forest)
            if( temp == "RFthresh" || temp == "Probthresh" )
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

            
// to define the lower bounds in probablity cut ranges  (e.g. random forest)
            if( temp == "RFCutLowerVals" )
            {
                while(!is_stream.eof() )
                {
                    
                    is_stream >> temp;
                    fProbabilityCutRangeLower.push_back(atof(temp.c_str()));
                }
            }

// to define the upper bounds in probablity cut ranges  (e.g. random forest)
            if( temp == "RFCutUpperVals" )
            {
                while(!is_stream.eof() )
                {
                    
                    is_stream >> temp;
                    fProbabilityCutRangeUpper.push_back(atof(temp.c_str()));
                }
            }

// to define the upper bounds in probablity cut ranges  (e.g. random forest)
            if( temp == "RFProbID" )
            {
                    is_stream >> temp;
                    fProbabilityCut_ProbID = atoi( temp.c_str() );
            }
            
            
// energy reconstruction cuts
            if( temp == "arrayechi2" )
            {
                is_stream >> temp;
                fArrayEChi2_min = atof(temp.c_str());
                is_stream >> temp;
                fArrayEChi2_max = atof(temp.c_str());
            }
            if( temp == "arraydE" )
            {
                is_stream >> temp;
                fArraydE_min = atof(temp.c_str());
                is_stream >> temp;
                fArraydE_max = atof(temp.c_str());
            }
            if( temp == "arrayerec" )
            {
                is_stream >> temp;
                fArrayErec_min = atof(temp.c_str());
                is_stream >> temp;
                fArrayErec_max = atof(temp.c_str());
            }
            if( temp == "arrayemission" )
            {
                is_stream >> temp;
                fArrayEmmission_min = atof(temp.c_str());
                is_stream >> temp;
                fArrayEmmission_max = atof(temp.c_str());
            }
            if( temp == "sizesecondmax" )         //AMC
            {
                is_stream >> temp;
                fArraySizeSecondMax_min = atof(temp.c_str());
                is_stream >> temp;
                fArraySizeSecondMax_max = atof(temp.c_str());
            }                                     //AMC
            if( temp == "teltypegroup" )
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
////////////////////////////////////////////////////////////////////////////////////////////////////
// TMVA values
            if( temp == "TMVACUTS" )
            {
               if( !is_stream.eof() ) is_stream >> fTMVASignalEfficiency;
// probability threshold not important for box cuts
               if( !is_stream.eof() ) is_stream >> fTMVAProbabilityThreshold;
// files should have endings _fTMVAWeightFileIndex_min to _fTMVAWeightFileIndex_max
               if( !is_stream.eof() ) is_stream >> fTMVAWeightFileIndex_min;
               if( !is_stream.eof() ) is_stream >> fTMVAWeightFileIndex_max;
               if( !is_stream.eof() ) is_stream >> fTMVAWeightFile;
// do not apply theta2 cut in TMVA
// (results from TMVA are read from XML file and applied later by hand; 
// necessary to get solid angle information into the sensitivity calculator)
	       if( !is_stream.eof() ) is_stream >> fTMVAIgnoreTheta2Cut;
            }
	    if( temp == "TMVASIGNALEFFICIENCY" )
	    {
	       if( !is_stream.eof() ) is_stream >> fTMVAOptimizeSignalEfficiencySourceStrengthCU;
	       if( !is_stream.eof() ) is_stream >> fTMVAOptimizeSignalEfficiencyParticleNumberFile;
////////////////////////////////////////////////////////////////////////////////////////////////////
            }
// read in values for energy dependent theta2 cut (TEMPORARY: MC only)
// * theta2file <root file> <scale factor> <minimum theta> <maximum theta> <function name>
// (note that fF1AngResName == "IRF" means that the graph from the IRF file is extrapolated)
            if( temp == "theta2file" )
            {
                 if( !is_stream.eof() ) is_stream >> fFileNameAngRes;
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
                 }
                 if( !is_stream.eof() )
                 {
                    is_stream >> fF1AngResName;
                 }
                 else fF1AngResName = "fitAngRes";

                 if( !initAngularResolutionFile() )
		 {
		    cout << "VGammaHadronCuts::readCuts error: error initialzing angular resolution file" << endl;
		    cout << "exiting..." << endl;
		    exit( -1 );
                 }
            }   
////////////////////////////////////////////////////////////////////////////////////////////////////
        }
    }
    return true;
}

void VGammaHadronCuts::printCutSummary()
{
    cout << "\t -----------------------------------------------------------------------------------------" << endl;
    cout << "\t cuts " << fCutSelector << " (ntel=" << fNTel << "): ";
// theta2 cut
    if( fArrayTheta2_min > 0. )
    {
        cout << fArrayTheta2_min << " < Theta^2 [deg^2]";
        if( fArrayTheta2_max > 0. ) cout << "< " << fArrayTheta2_max;
    }
    else if( fArrayTheta2_max > 0. ) cout << "Theta^2 [deg^2] < " << fArrayTheta2_max;
    else                             cout << "no fixed Theta^2 cut";

// mean reduced scaled cuts
    if( fCutSelector % 10 < 1 )
    {
        cout << ", " << fArrayMSCW_min << " < MSCW < " << fArrayMSCW_max;
        cout << ", " << fArrayMSCL_min << " < MSCL < " << fArrayMSCL_max << ", ";
        cout << "core distance < " << fArrayCore_max << " m";
    }
// mean cuts
    else if( fCutSelector % 10 == 1 )
    {
        cout << ", " << fArrayWidth_min  << " < mean width < " << fArrayWidth_max;
        cout << ", " << fArrayLength_min << " < mean length < " << fArrayLength_max << ", ";
        cout << "core distance < " << fArrayCore_max << " m";
    }
// mean scaled cuts
    else if( fCutSelector % 10 == 3 )
    {
        cout << ", " << fArrayMSW_min << " < MWR < " << fArrayMSW_max;
        cout << ", " << fArrayMSL_min << " < MLR < " << fArrayMSL_max << ", ";
        cout << "core distance < " << fArrayCore_max << " m";
    }
// probability cuts
    if( fCutSelector / 10 >= 1 && fCutSelector / 10 <= 3 )
    {
        cout << endl << "\t       ";
        cout << "event probability threshold: " << fProbabilityCut << " (element " << fProbabilityCut_ProbID << ")";
        if( fCutSelector / 10 == 3 ) cout << " (applied as quality cut)";
        if( fProbabilityCut_File ) cout << ", read from " << fProbabilityCut_File->GetName();
    }
// TMVA cuts
    if( fCutSelector / 10 == 4 )
    {
        cout << endl << "\t       ";
        cout << "TMVA gamma/hadron separation.";
	if( fTMVAIgnoreTheta2Cut ) cout << " (ignore theta2 cut in TMVA)";
	cout << endl;
        cout << "\t       weight files: " << fTMVAWeightFile << " (" << fTMVAWeightFileIndex_min << "," << fTMVAWeightFileIndex_max << ")" << endl;
	if( fTMVAOptimizeSignalEfficiencyParticleNumberFile.size() > 0. )
	{
	   cout << "\t using optimal signal efficiency for " << fTMVAOptimizeSignalEfficiencySourceStrengthCU << " Crab source" << endl;
	   cout << "\t reading particle counts from " << fTMVAOptimizeSignalEfficiencyParticleNumberFile << endl;
	}
	else
	{
           cout << "\t       signal efficiency: " << fTMVASignalEfficiency;
	   cout << ", probability threshold: " << fTMVAProbabilityThreshold << endl;
        }
    }
// other cut parameters
    if( fNTel == 2 ) cout << ", size > " << fArraySize_min;
    cout << endl;
    cout << "\t       ";
    cout << "fiducial area (camera) < " << fArrayxyoff_max << " deg, ";
    cout << fArrayEChi2_min << " <= EChi2 <= " << fArrayEChi2_max;
    cout << ", " << fArraydE_min << " < dE < " << fArraydE_max;
    cout << ", " << fArrayErec_min << " < Erec < " << fArrayErec_max;
    cout << ", " << fArrayEmmission_min << " < Emission height < " << fArrayEmmission_max;
    cout << endl;
    cout << "\t       ";
    cout << "fiducial area (core): [" << fCoreX_min - fCoreEdge << "," << fCoreX_max + fCoreEdge;
    cout << "," << fCoreY_min - fCoreEdge << "," << fCoreY_max + fCoreEdge << "] m";
    cout << ", " << fArrayNImages_min << " <= Ntel <= " << fArrayNImages_max;
    cout << endl;
    if( bMCCuts )
    {
        cout << "\t       MC cuts: " << fArrayxyoff_MC_min << " < fiducial area (camera) < " << fArrayxyoff_MC_max << " deg ";
        cout << endl;
    }
    if( fArrayLTrig.size() > 0 )
    {
        cout << "\t       tel-combinations: ";
        for( unsigned int i = 0; i < fArrayLTrig.size(); i++ )
        {
            cout << i << ": " << fArrayLTrig[i];
            if( i < fArrayLTrig.size() - 1 ) cout << ", ";
        }
        cout << endl;
    }
    if( fNTelTypeCut.size() > 0 )
    {
           for( unsigned int j = 0; j < fNTelTypeCut.size(); j++ ) fNTelTypeCut[j]->print();
    }
    if( fF1AngRes )
    {
        cout << "\t       ";
        cout << "reading theta2 cut from angular resolution file: " << fFileAngRes->GetName();
        cout << " (scale factor " << fAngRes_ScalingFactor;
        cout << ", minimum : " << fAngRes_AbsoluteMinimum << " [deg] ";
	cout << ", maximum : " << fAngRes_AbsoluteMaximum << " [deg])" << endl;
    }
    cout << "\t -----------------------------------------------------------------------------------------" << endl;
}


/*
  ensure event quality, reasonable output from the table variables, etc
*/
bool VGammaHadronCuts::applyStereoQualityCuts( unsigned int iEnergyReconstructionMethod, bool bCount, int iEntry, bool fIsOn)
{

/////////////////////////////////////////////////////////////////////////////////
// require certain quality in stereo reconstruction
    if( fData->Chi2 < fArrayChi2_min || fData->Chi2 > fArrayChi2_max )
    {
        if( bCount )
        {
            fStats->fN[eStereoQuality]++;
            fStats->fN[eArrayChi2]++;
        }
        return false;
    }

/////////////////////////////////////////////////////////////////////////////////
// apply number of images cut
    if( fData->NImages < fArrayNImages_min || fData->NImages > fArrayNImages_max )
    {
        if( bCount )
        {
            fStats->fN[eStereoQuality]++;
            fStats->fN[eNImages]++;
        }
        return false;
    }

/////////////////////////////////////////////////////////////////////////////////
    if( iEnergyReconstructionMethod != 99 )
    {
// MSCW/L reconstruction cuts
        if(  fCutSelector % 10 < 1 && ( fData->MSCW < -50. || fData->MSCL < -50. ) )
        {
            if( bCount )
            {
                fStats->fN[eMSC_Quality]++;
                fStats->fN[eNImages]++;
            }
            return false;
        }
// MWR/MLR reconstruction cuts
        if(  fCutSelector % 10 == 3 && ( fData->MWR < -50. || fData->MLR < -50. ) )
        {
            if( bCount )
            {
                fStats->fN[eMSC_Quality]++;
                fStats->fN[eNImages]++;
            }
            return false;
        }

/////////////////////////////////////////////////////////////////////////////////
// check energy reconstruction
        if( iEnergyReconstructionMethod == 0 )
        {
            if( fData->Erec > 0. && fData->EChi2 <= 0. ) return false;
            if( fData->Erec < fArrayErec_min )
            {
                if( bCount )
                {
                    fStats->fN[eStereoQuality]++;
                    fStats->fN[eErec]++;
                }
                return false;
            }
            if( fData->Erec > fArrayErec_max )
            {
                if( bCount )
                {
                    fStats->fN[eStereoQuality]++;
                    fStats->fN[eErec]++;
                }
                return false;
            }
        }
        else if( iEnergyReconstructionMethod == 1 )
        {
            if( fData->ErecS > 0. && fData->EChi2S <= 0. ) return false;
            if( fData->ErecS < fArrayErec_min )
            {
                if( bCount )
                {
                    fStats->fN[eStereoQuality]++;
                    fStats->fN[eErec]++;
                }
                return false;
            }
            if( fData->ErecS > fArrayErec_max )
            {
                if( bCount )
                {
                    fStats->fN[eStereoQuality]++;
                    fStats->fN[eErec]++;
                }
                return false;
            }
        }
    }

/////////////////////////////////////////////////////////////////////////////////
// check core positions
    double iR = 0.;
    double iNTR = 0.;
    for( unsigned int i = 0; i < fNTel; i++ )
    {
        if( fData->R[i] > 0. )
        {
            iR += fData->R[i];
            iNTR++;
        }
    }
    if( iNTR > 0. ) iR /= iNTR;

    if( iR < fArrayCore_min || iR > fArrayCore_max )
    {
        if( bCount )
        {
            fStats->fN[eStereoQuality]++;
            fStats->fN[eCorePos]++;
        }
        return false;
    }

/////////////////////////////////////////////////////////////////////////////////
// apply image selection cuts (check which telescopes were used in the reconstruction)
    if( fArrayLTrig.size() > 0 )
    {
        if( fData->ImgSel < fArrayLTrig.size() )
        {
            if( !fArrayLTrig[fData->ImgSel] )
            {
                if( bCount )
                {
                    fStats->fN[eStereoQuality]++;
                    fStats->fN[eLTrig]++;
                }
             return false;
            }
        }
        else 
        {
            if( bCount )
            {
                fStats->fN[eStereoQuality]++;
                fStats->fN[eLTrig]++;
            }
            return false;
        }
    }

/////////////////////////////////////////////////////////////////////////////////
// apply cuts on second max
    if(fData->SizeSecondMax < fArraySizeSecondMax_min || fData->SizeSecondMax > fArraySizeSecondMax_max)
    {
        if( bCount )
        {
            fStats->fN[eStereoQuality]++;
            fStats->fN[eSizeSecondMax]++;
        }
        return false;
    }

/////////////////////////////////////////////////////////////////////////
// apply cut selector from probability tree
    if( fCutSelector / 10 == 3 )
    {
        if( !applyProbabilityCut( iEntry , fIsOn) )
        {
            if( !bCount ) fStats->fN[eStereoQuality]++;
            return false;
        }
    }

    return true;
}


bool VGammaHadronCuts::applyEnergyReconstructionQualityCuts()
{
    return applyEnergyReconstructionQualityCuts( 0 );
}


/*!
  cuts apply in energies in linear scale!

  iEnergyReconstructionMethod == 100: return always true
*/
bool VGammaHadronCuts::applyEnergyReconstructionQualityCuts( unsigned int iEnergyReconstructionMethod, bool bCount )
{
    fStats->fN[eEnergyRec] = 0;
    if( iEnergyReconstructionMethod == 0 )
    {
        if( fData->EChi2 < fArrayEChi2_min ) return false;
        if( fData->EChi2 > fArrayEChi2_max ) return false;
        if( fData->Erec < fArrayErec_min )   return false;
        if( fData->Erec > fArrayErec_max )   return false;
        if( fData->dE < fArraydE_min )       return false;
        if( fData->dE > fArraydE_max )       return false;
    }
    else if( iEnergyReconstructionMethod == 1 )
    {
        if( fData->EChi2S < fArrayEChi2_min ) return false;
        if( fData->EChi2S > fArrayEChi2_max ) return false;
        if( fData->ErecS < fArrayErec_min )   return false;
        if( fData->ErecS > fArrayErec_max )   return false;
        if( fData->dES < fArraydE_min )       return false;
        if( fData->dES > fArraydE_max )       return false;
    }
    else if( iEnergyReconstructionMethod == 100 )
    {
        return true;
    }
    // unknown reconstruction method
    else return false;

    return true;
}

/*

  apply gamma/hadron separation cut

*/

bool VGammaHadronCuts::isGamma( int i, bool bCount, bool fIsOn)
{

/////////////////////////////////////////////////////////////////////////////
// apply box cuts  (e.g. MSCW/MSCL or MWR/MLR)
    if( fCutSelector % 10 <= 3 )
    {
        if( !applyStereoShapeCuts() )
        {
            if( !bCount ) fStats->fN[eIsGamma]++;
            return false;
        }
// all cut selectors >= 10 are different
        if( fCutSelector < 10 ) return true;
    }

/////////////////////////////////////////////////////////////////////////////
// apply probability threshold cut (e.g. random forest cuts)
    if( fCutSelector / 10 == 1 || fCutSelector / 10 == 2 )
    {
        if( !applyProbabilityCut( i, fIsOn ) )
        {
            if( !bCount ) fStats->fN[eIsGamma]++;
            return false;
        }
    }

/////////////////////////////////////////////////////////////////////////////
// apply cut using TMVA reader
    if( fCutSelector / 10 == 4 )
    {
       if( !applyTMVACut( i, fIsOn ) )
       {
          if( !bCount ) fStats->fN[eIsGamma]++;
          return false;
       }
    }
          

    return true;
}

/*
   
   use TMVA reader and apply cuts

*/
bool VGammaHadronCuts::applyTMVACut( int i, bool fIsOn )
{

   if( fTMVAEvaluator )
   {
      return fTMVAEvaluator->evaluate( fTMVASignalEfficiency, fTMVAProbabilityThreshold );
   }

   return false;
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
    if( fData->MSCW > fArrayMSCW_max ) return false;
    if( fData->MSCW < fArrayMSCW_min ) return false;
    if( fData->MSCL > fArrayMSCL_max ) return false;
    if( fData->MSCL < fArrayMSCL_min ) return false;

    return true;
}

/*!

    apply mean scaled cuts

*/
bool VGammaHadronCuts::applyMeanScaledStereoShapeCuts()
{
    if( fData->MWR > fArrayMSW_max ) return false;
    if( fData->MWR < fArrayMSW_min ) return false;
    if( fData->MLR > fArrayMSL_max ) return false;
    if( fData->MLR < fArrayMSL_min ) return false;

    return true;
}

/*

   apply mean shape cuts

   (not scaled width, this calculation is MC independent)

*/
bool VGammaHadronCuts::applyMeanStereoShapeCuts()
{
    fMeanWidth = 0.;
    fMeanLength = 0.;
    fMeanDistance = 0.;
    int intel = 0;
    double iasym = 0.;
// loop over all telescopes and calculate mean values
    for( unsigned int i = 0; i < fNTel; i++ )
    {
	 if( fData->ntubes[i] >= fArrayNtubes_max ||  fData->ntubes[i] <= fArrayNtubes_min ) continue;
	 if( fData->size[i] <= fArraySize_min || fData->size[i] >= fArraySize_max ) continue;

	 fMeanWidth += fData->width[i];
	 fMeanLength += fData->length[i];
	 fMeanDistance += fData->dist[i];
	 iasym += fData->asym[i];
	 intel++;
    }
    if( intel >= fArrayNTel_min  )
    {
	 fMeanWidth  /= (double)intel;
	 fMeanLength /= (double)intel;
	 fMeanDistance   /= (double)intel;
	 iasym   /= (double)intel;
    }
// apply cuts
    if( fMeanDistance <= fArrayDistance_min || fMeanDistance >= fArrayDistance_max ) return false;
    if( fMeanLength <= fArrayLength_min || fMeanLength >= fArrayLength_max )         return false;
    if( fMeanWidth <= fArrayWidth_min || fMeanWidth >= fArrayWidth_max )             return false;
    if( iasym <= fArrayAsymm_min || iasym >= fArrayAsymm_max )                       return false;

    return true;
}

/*

   stereo shape cuts

*/
bool VGammaHadronCuts::applyStereoShapeCuts()
{
/////////////////////////////////////////////////////////////////////////////
// cut selector 2 is always true
    if( fCutSelector % 10 == 2 ) return true;

/////////////////////////////////////////////////////////////////////////////
// apply cuts
/////////////////////////////////////////////////////////////////////////////
// MSCW/MSCL cuts
    if( fCutSelector % 10 < 1 )
    {
        if( !applyMeanReducedScaledStereoShapeCuts() ) return false;
    }
/////////////////////////////////////////////////////////////////////////////
// mean width/length cuts
// (not scaled width, this calculation is MC independent)
    else if( fCutSelector % 10 == 1 )
    {
       if( !applyMeanStereoShapeCuts() ) return false;
    }
/////////////////////////////////////////////////////////////////////////////
// MWR/MLR cuts
    else if( fCutSelector % 10 == 3 )
    {
       if( !applyMeanScaledStereoShapeCuts() ) return false;
    }

/////////////////////////////////////////////////////////////////////////////
// emission height cuts
    if( fData->EmissionHeight > 0. && fData->EmissionHeight > fArrayEmmission_max ) return false;
    if( fData->EmissionHeight > 0. && fData->EmissionHeight < fArrayEmmission_min ) return false;

    return true;
}


/*
  select gamma/hadron cuts

  (see description of cuts at the top of this file)
*/
void VGammaHadronCuts::selectCuts( int iC, int irun, string iFile )
{
    fCutSelector = iC;

// probability cuts
    if( fCutSelector / 10 >= 1 && fCutSelector / 10 <= 3 )
    {
        if( irun > 0 )              initProbabilityCuts( irun );
        else if( iFile.size() > 0 ) initProbabilityCuts( iFile );
        else
        {
            cout << "VGammaHadronCuts::selectCuts: failed setting cuts for " << irun << " " << iFile << endl;
            cout << "exiting..." << endl;
            exit( -1 );
        }
    }
// TMVA cuts
    else if( fCutSelector / 10 == 4 )
    {
        if( !initTMVAEvaluator( fTMVAWeightFile, fTMVAWeightFileIndex_min, fTMVAWeightFileIndex_max ) )
        {
           cout << "VGammaHadronCuts::selectCuts: failed setting TMVA reader for " << fTMVAWeightFile;
           cout << "(" << fTMVAWeightFileIndex_min << "," << fTMVAWeightFileIndex_max << ")" << endl;
           cout << "exiting..." << endl;
           exit( -1 );
        }
    }

}

bool VGammaHadronCuts::initTMVAEvaluator( string iTMVAFile, unsigned int iTMVAWeightFileIndex_min, unsigned int iTMVAWeightFileIndex_max )
{
    TDirectory *cDir = gDirectory;

    fTMVAEvaluator = new VTMVAEvaluator();
    if( fTMVAOptimizeSignalEfficiencyParticleNumberFile.size() > 0. )
    {
       fTMVAEvaluator->setSensitivityOptimizationParameters( fTMVAOptimizeSignalEfficiencySourceStrengthCU );
       fTMVAEvaluator->setParticleNumberFile( fTMVAOptimizeSignalEfficiencyParticleNumberFile );
    }
    else
    {
       fTMVAEvaluator->setSignalEfficiency( fTMVASignalEfficiency );
    }
    fTMVAEvaluator->initializeWeightFiles( iTMVAFile, iTMVAWeightFileIndex_min, iTMVAWeightFileIndex_max );
    fTMVAEvaluator->printSignalEfficiency();
    fTMVAEvaluator->setIgnoreTheta2Cut( fTMVAIgnoreTheta2Cut );
    fTMVABoxCut_Theta2_max = fTMVAEvaluator->getBoxCut_Theta2_Graph();
    if( fTMVABoxCut_Theta2_max )
    {
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

/*

   check that event is inside a certain region in the camera

*/
bool VGammaHadronCuts::applyInsideFiducialAreaCut( bool bCount )
{
    double xy = fData->Xoff*fData->Xoff + fData->Yoff*fData->Yoff;

    if( xy > fArrayxyoff_max*fArrayxyoff_max )
    {
        if( bCount ) fStats->fN[eXYoff]++;
        return false;
    }

    if( fArrayxyoff_min >= 0. )
    {
        if( xy < fArrayxyoff_min*fArrayxyoff_min )
        {
            if( bCount ) fStats->fN[eXYoff]++;
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
    if( !isMCCuts() ) return true;

    double xy = xoff*xoff + yoff*yoff;

    if( xy > fArrayxyoff_MC_max*fArrayxyoff_MC_max )
    {
        if( bCount ) fStats->fN[eMC_XYoff]++;
        return false;
    }

    if( fArrayxyoff_MC_min >= 0. )
    {
        if( xy < fArrayxyoff_MC_min*fArrayxyoff_MC_min )
        {
            if( bCount ) fStats->fN[eMC_XYoff]++;
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

   if( bCount && !icut ) fStats->fN[eTelType]++;

   return icut;
}


/*!
  check if core is inside a certain area relative to centre of array (ground, not shower coordinates)

  fCoreY_min && fCoreY_max == 0: cut area is circle with radius fCoreX_max
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
    if( TMath::Abs( fCoreY_min ) < 1.e-5 && TMath::Abs( fCoreY_max ) < 1.e-5 )
    {
        if( sqrt( xcore*xcore+ycore*ycore ) < fCoreX_min ) return false;
        if( sqrt( xcore*xcore+ycore*ycore ) > fCoreX_max ) return false;
    }
    else
///////////////////////////////////////////////
// box cut
    {
        if( xcore < fCoreX_min - fCoreEdge ) return false;
        if( xcore > fCoreX_max + fCoreEdge ) return false;
        if( ycore < fCoreY_min - fCoreEdge ) return false;
        if( ycore > fCoreY_max + fCoreEdge ) return false;
    }

    return true;
}


void VGammaHadronCuts::setShowerCoreCuts( double xmin, double xmax, double ymin, double ymax, double iEdge )
{
    fCoreX_min = xmin;
    fCoreX_max = xmax;
    fCoreY_min = ymin;
    fCoreY_max = ymax;
    if( iEdge >= 0. ) fCoreEdge = iEdge;
    cout << "setting shower core cuts: " << fCoreX_min << "\t" << fCoreX_max << "\t" << fCoreY_min << "\t" << fCoreY_max << "\t" << fCoreEdge << endl;
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
   if( fEnergyReconstructionMethod == 0 )      i_theta2_cut_max = getTheta2Cut_max( fData->Erec );
   else if( fEnergyReconstructionMethod == 1 ) i_theta2_cut_max = getTheta2Cut_max( fData->ErecS );

// direction cut (ring around center of camera)
   if( theta2 < i_theta2_cut_max && theta2 > fArrayTheta2_min )
   {
      return true;
   }

   if( bCount ) fStats->fN[eDirection]++;
   return false;
}

/*
   fetch theta2 cut (might be energy dependent)

   e      :   [TeV] energy (linear)
*/
double VGammaHadronCuts::getTheta2Cut_max( double e )
{
    double theta2_cut_max = -1.;

//////////////////////////////////////////////
// energy independent theta2 cut
//////////////////////////////////////////////
    if( fArrayTheta2_max > 0. )
    {
       return fArrayTheta2_max;
    } 

//////////////////////////////////////////////
// energy dependent theta2 cut
//////////////////////////////////////////////

/////////////////////////////////////////////
// use fit function to angular resolution
    if( fF1AngRes && e > 0. )
    {
       e = log10( e );

// for e outside of functions range, return edge values
       if( e < fF1AngRes->GetXmin() )       e = fF1AngRes->GetXmin();
       else if( e > fF1AngRes->GetXmax() )  e = fF1AngRes->GetXmax();

// get angular resolution and apply scaling factor
       theta2_cut_max  = fF1AngRes->Eval( e ) * fAngRes_ScalingFactor;
       if( theta2_cut_max < fAngRes_AbsoluteMinimum ) return fAngRes_AbsoluteMinimum*fAngRes_AbsoluteMinimum;
       if( theta2_cut_max > fAngRes_AbsoluteMaximum ) return fAngRes_AbsoluteMaximum*fAngRes_AbsoluteMaximum;
       theta2_cut_max *= theta2_cut_max;
    }
/////////////////////////////////////////////
// use IRF graph of angular resolution
    else if( fIRFAngRes && e > 0. )
    {
       e = log10( e );
// for e outside of functions range, return edge values
       if( fIRFAngRes->GetN() > 0 && fIRFAngRes->GetX() && fIRFAngRes->GetY() )
       {
          if( e < fIRFAngRes->GetX()[0] )                    return fIRFAngRes->GetY()[0];
	  if( e > fIRFAngRes->GetX()[fIRFAngRes->GetN()-1] ) return fIRFAngRes->GetY()[fIRFAngRes->GetN()-1];
       }
       theta2_cut_max  = fIRFAngRes->Eval( e ) * fAngRes_ScalingFactor;
       if( theta2_cut_max < fAngRes_AbsoluteMinimum ) return fAngRes_AbsoluteMinimum*fAngRes_AbsoluteMinimum;
       if( theta2_cut_max > fAngRes_AbsoluteMaximum ) return fAngRes_AbsoluteMaximum*fAngRes_AbsoluteMaximum;
       theta2_cut_max *= theta2_cut_max;
    }
/////////////////////////////////////////////
// use TMVA determined cut
    else if( fTMVAEvaluator && e > 0. && fTMVAIgnoreTheta2Cut )
    {
       e = log10( e );

       theta2_cut_max = fTMVAEvaluator->getBoxCut_Theta2( e );
       if( theta2_cut_max < fAngRes_AbsoluteMinimum ) return fAngRes_AbsoluteMinimum*fAngRes_AbsoluteMinimum;
       if( theta2_cut_max > fAngRes_AbsoluteMaximum ) return fAngRes_AbsoluteMaximum*fAngRes_AbsoluteMaximum;
    }
/////////////////////////////////////////////
// use a vector with theta2 cuts
    else if( e > 0. && fTMVABoxCut_Theta2_max )
    {
       return fTMVABoxCut_Theta2_max->Eval( log10( e ) );
    }

   return theta2_cut_max;
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
       cout << "VGammaHadronCuts::initAngularResolutionFile: error open angular resolution file: " << fFileNameAngRes << endl;
       return false;
   }

/////////////////////////
// get energy dependent theta values (probably angular resolution)

// read angular resolution as function (TF1) from a fit
   if( fF1AngResName != "IRF" )
   {
// get energy dependent theta values (probably angular resolution)
      fF1AngRes = (TF1*)fFileAngRes->Get( fF1AngResName.c_str() )->Clone();
      if( !fF1AngRes )
      {
	  cout << "VGammaHadronCuts::initAngularResolutionFile: error finding angular resolution function with name " << fF1AngResName << " in " << fFileAngRes->GetName() << endl;
	  return false;
      }
      cout << "VGammaHadronCuts::initAngularResolutionFile: read angular resolution fit function from file " << fF1AngResName << " : " << endl;
      fF1AngRes->Print();
   }
// read angular resolution from instrument response function tree
   else
   {
      TTree *t = (TTree*)fFileAngRes->Get( "t_angular_resolution" );
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
      }
      cout << "VGammaHadronCuts::initAngularResolutionFile: read angular resolution graph from file " << fF1AngResName << " : " << endl;
      fIRFAngRes->Print();
   }
   if( fFileAngRes ) fFileAngRes->Close();

   return true;
}

/*!

     update statistics

*/
void VGammaHadronCuts::newEvent()
{
   fStats->fN[eTot]++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
      if( fTelType_counter[i] < c->NTtype ) ntel_type += c->NImages_Ttype[fTelType_counter[i]];
   }
    // OBS! >=
   if( ntel_type >= fNTelType_min ) return true;

   return false;
}

void VNTelTypeCut::print()
{
   cout << "\t       type cut: mintel > " << fNTelType_min << " for types ";
   for( unsigned int i = 0; i < fTelType_counter.size(); i++ ) cout << fTelType_counter[i] << " ";
   cout << endl;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VGammaHadronCutsStats::VGammaHadronCutsStats()
{
   fName.push_back( "Tot          " );
   fName.push_back( "MC_XYoff     " );
   fName.push_back( "XYoff        " );
   fName.push_back( "StereoQuality" );
   fName.push_back( "ArrayChi2    " );
   fName.push_back( "NImages      " );
   fName.push_back( "MSC_Quality  " );
   fName.push_back( "Erec         " );
   fName.push_back( "CorePos      " );
   fName.push_back( "LTrig        " );
   fName.push_back( "SizeSecondMax" );
   fName.push_back( "TelType      " );
   fName.push_back( "Direction    " );
   fName.push_back( "IsGamma      " );
   fName.push_back( "EnergyRec    " );

   reset();
}

void VGammaHadronCutsStats::reset()
{
   fN.clear();

    // the vector will have exactly the name of EN_AnaCutsStats
   for( unsigned int i = 0; i < fName.size(); i++ )
   {
      fN.push_back( 0 );
   }
}

void VGammaHadronCutsStats::printCutStatistics()
{
   cout << endl;
   cout << "\t cut statistics: " << endl;
   for( unsigned int i = 0; i < fName.size(); i++ )
   {
      cout << "\t\t" << fName[i] << "\t";
      if( i < fN.size() ) cout << fN[i] << endl;
   }
   cout << endl;
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
