/*! \class VTableLookupRunParameter
    \brief parameter storage class

*/

#include "VTableLookupRunParameter.h"

ClassImp( VTableLookupRunParameter )

VTableLookupRunParameter::VTableLookupRunParameter()
{
	fDebug = 0;
	
	outputfile = "";
	tablefile = "";
	ze = -1.;
	isMC = false;
	fUpdateInstrumentEpoch = true;
	fUseMedianEnergy = 1;
	fPE = false;
	fInterpolateString = "";
	readwrite = 'R';
	writeoption = "recreate";
	fMinRequiredShowerPerBin = 5.;
	bNoNoTrigger = true;
	fUseSelectedImagesOnly = true;
	bWriteReconstructedEventsOnly = 1;
	bShortTree = false;
	bWriteMCPars = true;
	rec_method = 0;
	fWrite1DHistograms = false;
	fSpectralIndex = 2.0;
	fWobbleOffset = 500;     // integer of wobble offset * 100
	fNoiseLevel = 250;
	fTableFillingCut_NImages_min = 2;
	fTableFillingCut_WobbleCut_max = 15.;
	fminsize = 0.;
	fmaxdist = 50000.;
	fmaxloss = 1.;
	fSelectRandom = -1.;
	fSelectRandomSeed = 17;
	fRerunStereoReconstruction = false;
	fRerunStereoReconstruction_minAngle = -1.;
	fRerunStereoReconstruction_BDTNImages_max = 4;
	fRerunStereoReconstruction_BDTFileName = "";
	fDispError_BDTFileName = "";
	fDispError_BDTWeight = 5.;
	fDispSign_BDTFileName = "";
	fDisp_UseIntersectForHeadTail = false;
	fQualityCutLevel = 0;
	
	fLimitEnergyReconstruction = false;
	
	fMC_distance_to_cameracenter_min =  0.;
	fMC_distance_to_cameracenter_max =  1.e10;
	
	fNentries = 1234567890;
	fMaxRunTime = 1.e9;
	
	printpara = "";
	
	meanpedvars = 0.;
}


bool VTableLookupRunParameter::fillParameters( int argc, char* argv[] )
{
	// check number of command line parameters
	if( argc < 2 )
	{
		printHelp();
		return false;
	}
	// =============================================
	// reading command line parameters
	// =============================================
	// read command line parameters
	int i = 1;
	while( i++ < argc )
	{
		string iTemp = argv[i - 1];
		string iTemp2 = "";
		if( i < argc )
		{
			iTemp2 = argv[i];
		}
		if( iTemp.find( "-help" ) < iTemp.size() )
		{
			printHelp();
			return false;
		}
		if( ( iTemp.find( "-input" ) < iTemp.size() || iTemp.find( "-sourcefile" ) < iTemp.size() )
				&& !( iTemp.find( "-inputfilelist" ) < iTemp.size() ) )
		{
			if( iTemp2.size() > 0 )
			{
				inputfile.push_back( iTemp2 );
				i++;
			}
		}
		else if( iTemp.find( "-inputfilelist" ) < iTemp.size() )
		{
			if( iTemp2.size() > 0 )
			{
				fillInputFile_fromList( iTemp2 );
				i++;
			}
		}
		else if( iTemp.find( "-o" ) < iTemp.size() )
		{
			if( iTemp2.size() > 0 )
			{
				outputfile = iTemp2;
				i++;
			}
		}
		else if( iTemp.find( "printrunparameters" ) < iTemp.size() )
		{
			if( iTemp2.size() > 0 )
			{
				printpara = iTemp2;
				i++;
			}
			return true;
		}
		else if( iTemp.find( "useMedian" ) < iTemp.size() )
		{
			fUseMedianEnergy = atoi( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		else if( iTemp.find( "updateEpoch" ) < iTemp.size() )
		{
			fUpdateInstrumentEpoch = ( bool )atoi( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		else if( iTemp.find( "noise" ) < iTemp.size() )
		{
			fNoiseLevel = atoi( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		else if( iTemp.find( "minshowerperbin" ) < iTemp.size() )
		{
			fMinRequiredShowerPerBin = atof( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		else if( iTemp.find( "woff" ) < iTemp.size() )
		{
			fWobbleOffset = atoi( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
			// wobble offset is given in float
			if( fWobbleOffset < 10 )
			{
				fWobbleOffset = ( int )( atof( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() ) * 1000 + 0.5 );
			}
		}
		else if( iTemp.find( "-table" ) < iTemp.size() )
		{
			if( iTemp2.size() > 0 )
			{
				tablefile = iTemp2;
				i++;
			}
		}
		else if( iTemp.find( "-fill" ) < iTemp.size() )
		{
			int iT = atoi( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
			if( iT == 1 )
			{
				readwrite = 'W';
			}
			else if( iT == 0 )
			{
				readwrite = 'R';
			}
			else
			{
				cout << "unknown parameter, choose 1=fill or 2=read lookup tables" << endl;
				return false;
			}
		}
		// rerun the stero reconstruction
		else if( iTemp.find( "-redo_stereo_reconstruction" ) < iTemp.size() )
		{
			fRerunStereoReconstruction = true;
		}
		// new minimum angle between image axes for simple stereo reconstruction
		else if( iTemp.find( "-minangle_stereo_reconstruction" ) < iTemp.size() )
		{
			fRerunStereoReconstruction_minAngle = atof( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		// BDT directory and file name for disp stereo reconstruction (direction)
		else if( iTemp.find( "-tmva_filename_stereo_reconstruction" ) < iTemp.size() )
		{
			if( iTemp2.size() > 0 )
			{
				fRerunStereoReconstruction_BDTFileName = iTemp2;
				i++;
			}
		}
		// DISP BDT reconstruction is applied for images with up to this multiplicity
		else if( iTemp.find( "-tmva_nimages_max_stereo_reconstruction" ) < iTemp.size() )
		{
			fRerunStereoReconstruction_BDTNImages_max = ( unsigned int )( atoi( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() ) );
			if( fRerunStereoReconstruction_BDTNImages_max > 40000 )
			{
				cout << "VTableLookupRunParameter::fillParameters() error:";
				cout << " maximum number of images for TMVA disp reconstruction is 4";
				cout << " (selection was " << fRerunStereoReconstruction_BDTNImages_max << ")" << endl;
				cout << "exiting..." << endl;
				exit( EXIT_FAILURE );
			}
		}
		// BDT directory and file name for disp stereo reconstruction (disperror)
		else if( iTemp.find( "-tmva_filename_disperror_reconstruction" ) < iTemp.size() )
		{
			if( iTemp2.size() > 0 )
			{
				fDispError_BDTFileName = iTemp2;
				i++;
			}
		}
		else if( iTemp.find( "-tmva_disperror_weight" ) < iTemp.size() )
		{
			if( iTemp2.size() > 0 )
			{
				fDispError_BDTWeight = atof( iTemp2.c_str() );
				i++;
			}
		}
		// BDT directory and file name for disp stereo reconstruction (dispsign)
		else if( iTemp.find( "-tmva_filename_dispsign_reconstruction" ) < iTemp.size() )
		{
			if( iTemp2.size() > 0 )
			{
				fDispSign_BDTFileName = iTemp2;
				i++;
			}
		}
		// BDTdisp use intersection for head tail uncertainty
		else if( iTemp.find( "-disp_use_intersect" ) < iTemp.size() )
		{
			fDisp_UseIntersectForHeadTail = true;
		}
		else if( iTemp.find( "-qualitycutlevel" ) < iTemp.size() )
		{
			if( iTemp2.size() > 0 )
			{
				fQualityCutLevel = ( unsigned int )( atoi( iTemp2.c_str() ) );
				i++;
			}
		}
		else if( iTemp.find( "-ze" ) < iTemp.size() )
		{
			ze = atof( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		else if( iTemp.find( "-selectRandom" ) < iTemp.size() && !( iTemp.find( "-selectRandomSeed" ) < iTemp.size() ) )
		{
			fSelectRandom = atof( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
			if( fSelectRandom > 1. || fSelectRandom < 0. )
			{
				cout << "Error: probability has to be in [0,1]: " << fSelectRandom << endl;
				return false;
			}
		}
		else if( iTemp.find( "-selectRandomSeed" ) < iTemp.size() )
		{
			fSelectRandomSeed = atoi( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		else if( iTemp.find( "-minImages" ) < iTemp.size() )
		{
			fTableFillingCut_NImages_min = ( unsigned int )atoi( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		else if( iTemp.find( "-spectralIndex" ) < iTemp.size() )
		{
			fSpectralIndex = atof( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		else if( iTemp.find( "-maxdist" ) < iTemp.size()
				 && !( iTemp.find( "-maxdistancetocameracenter" ) < iTemp.size() )
				 && !( iTemp.find( "-maxdistfraction" ) < iTemp.size() ) )
		{
			fmaxdist = atof( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		else if( iTemp.find( "-maxloss" ) < iTemp.size() )
		{
			fmaxloss = atof( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		else if( iTemp.find( "-maxdistancetocameracenter" ) < iTemp.size() )
		{
			fMC_distance_to_cameracenter_max  = atof( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		else if( iTemp.find( "-mindistancetocameracenter" ) < iTemp.size() )
		{
			fMC_distance_to_cameracenter_min  = atof( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
			// looking at squared differences!!
			if( fMC_distance_to_cameracenter_min < 0. )
			{
				fMC_distance_to_cameracenter_min = 0.;
			}
		}
		else if( iTemp.find( "-add_mc_spectral_index" ) < iTemp.size() )
		{
			fAddMC_spectral_index.push_back( atof( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() ) );
		}
		else if( iTemp.find( "-minsize" ) < iTemp.size() )
		{
			fminsize = atof( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		else if( iTemp.find( "-debug" ) < iTemp.size() )
		{
			fDebug = ( unsigned int )atoi( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		else if( iTemp.find( "-arrayrecid" ) < iTemp.size() || iTemp.find( "-recid" ) < iTemp.size() )
		{
			rec_method = atoi( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		else if( iTemp.find( "-update" ) < iTemp.size() )
		{
			bool iT = ( bool )atoi( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
			if( iT )
			{
				writeoption = "update";
			}
			else
			{
				writeoption = "recreate";
			}
		}
		else if( iTemp.find( "-noNo" ) < iTemp.size() )
		{
			bNoNoTrigger = false;
		}
		else if( iTemp.find( "-writeReconstructedEventsOnly" ) < iTemp.size() )
		{
			if( iTemp.rfind( "=" ) != string::npos )
			{
				bWriteReconstructedEventsOnly = atoi( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
			}
			else
			{
				bWriteReconstructedEventsOnly = 0;
			}
		}
		else if( iTemp.find( "-short" ) < iTemp.size() )
		{
			bShortTree = true;
		}
		else if( iTemp.find( "-pe" ) < iTemp.size() )
		{
			fPE = true;
		}
		else if( iTemp.find( "-nomctree" ) < iTemp.size() )
		{
			bWriteMCPars = false;
		}
		else if( iTemp.find( "-write1DHistograms" ) < iTemp.size() )
		{
			fWrite1DHistograms = true;
		}
		else if( iTemp.find( "maxnevents" ) < iTemp.size() )
		{
			fNentries = atoi( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
		else if( iTemp.find( "maxruntime" ) < iTemp.size() )
		{
			fMaxRunTime = atof( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
		}
	}
	// filling of tables requires Monte Carlo
	if( readwrite == 'W' )
	{
		isMC = true;
	}
	// =============================================
	// end of reading command line parameters
	// =============================================
	
	// require inputfile name
	if( inputfile.size() == 0 )
	{
		cout << "error: no input file" << endl;
		cout << "...exiting" << endl;
		return false;
	}
	// require table file
	if( tablefile.size() == 0 )
	{
		cout << "error: no lookup table file" << endl;
		cout << "...exiting" << endl;
		return false;
	}
	
	// set output file name (mainly for VTS analysis with a single inputfile)
	if( outputfile.size() == 0 && inputfile.size() == 1 )
	{
		// wildcards for input file
		if( inputfile[0].find( "*" ) < inputfile[0].size() )
		{
			outputfile = "mscw.root";
		}
		// no wildcards for input file
		else
		{
			outputfile = inputfile[0].substr( 0, inputfile[0].rfind( "." ) );
			outputfile += ".mscw.root";
		}
	}
	// for VTS analysis with a single inputfile: get telescope combinations
	if( inputfile.size() == 1 && inputfile[0].find( "*" ) == string::npos )
	{
		readTelescopeToAnalyze( inputfile[0] );
	}
	
	return true;
}

/*
 * read telescope combination for analysis
 *
 * this works only for a small number of telescopes (<10)
 */
bool VTableLookupRunParameter::readTelescopeToAnalyze( string iFile )
{
	fTelToAnalyse.clear();
	
	TFile iF( iFile.c_str() );
	if( iF.IsZombie() )
	{
		cout << "VTableLookupRunParameter::readTelescopeToAnalyze warning: could not open input file to read run parameters" << endl;
		cout << "\t " << iFile.c_str() << endl;
		return false;
	}
	
	// read telescopes to analyse from eventdisplay run parameter list
	vector< unsigned int > iRunParT;
	VEvndispRunParameter* iPar = ( VEvndispRunParameter* )iF.Get( "runparameterV2" );
	if( iPar )
	{
		iRunParT = iPar->fTelToAnalyze;
	}
	else
	{
		cout << "VTableLookupRunParameter::readTelescopeToAnalyze warning: could not find evndisp run parameters (runparameterV2)" << endl;
		return false;
	}
	// cross check if one of the has been switched off in the analysis
	VEvndispReconstructionParameter* iA = ( VEvndispReconstructionParameter* )iF.Get( "EvndispReconstructionParameter" );
	if( iA && iPar )
	{
		// this works only if number of telescopes = number of telescope types
		if( rec_method < ( int )iA->fLocalUseImage.size() && iPar->fNTelescopes == iA->fLocalUseImage[rec_method].size() )
		{
			for( unsigned int i = 0; i < iRunParT.size(); i++ )
			{
				if( iRunParT[i] < iA->fLocalUseImage[rec_method].size() && iA->fLocalUseImage[rec_method][iRunParT[i]] )
				{
					fTelToAnalyse.push_back( iRunParT[i] );
				}
			}
			cout << "Following telescopes are included in the analysis: ";
			for( unsigned int i = 0; i < fTelToAnalyse.size(); i++ )
			{
				cout << " T" << fTelToAnalyse[i] + 1;
			}
			cout << endl;
		}
	}
	else
	{
		cout << "VTableLookupRunParameter::readTelescopeToAnalyze warning: could not find evndisp reconstruction parameters (EvndispReconstructionParameter)" << endl;
		return false;
	}
	
	return true;
}


void VTableLookupRunParameter::printHelp()
{
	if( gSystem->Getenv( "EVNDISPSYS" ) )
	{
		int syst_ret = system( "cat $EVNDISPSYS/README/README.MSCW_ENERGY" );
		if( syst_ret == -1 )
		{
			cout << "VTableLookupRunParameter::printHelp() error: could not find helper file in README directory" << endl;
		}
	}
	else
	{
		cout << "VTableLookupRunParameter::printHelp() no help available (environmental variable EVNDISPSYS not set)" << endl;
	}
	return;
}


void VTableLookupRunParameter::print( int iP )
{
	cout << "mscw_energy VERSION " << getEVNDISP_VERSION() << endl;
	cout << endl;
	cout << "debug level " << fDebug << endl;
	cout << "lookuptable: " << tablefile << endl;
	cout << endl;
	cout << "evndisp reconstruction parameter ID: " << rec_method << endl;
	cout << endl;
	cout << "input file(s): ";
	for( unsigned int i = 0; i < inputfile.size(); i++ )
	{
		cout << "\t" << inputfile[i] << endl;
	}
	if( isMC )
	{
		cout << " (input data is MC)";
	}
	if( fPE )
	{
		cout << " (input data is PE)";
	}
	cout << endl;
	if( readwrite != 'W' && readwrite != 'w' )
	{
		cout << "output file: " << outputfile << endl;
		if( bWriteReconstructedEventsOnly >= 0 )
		{
			cout << "writing reconstructed events only (" << bWriteReconstructedEventsOnly << ")" << endl;
		}
	}
	else
	{
		cout << "filling lookup tables for: ";
		cout << " zenith " << ze << ", direction offset " << fWobbleOffset << "(x0.01) [deg], ";
		cout << "noise level " << fNoiseLevel << ", spectral index " << fSpectralIndex << endl;
		if( fWrite1DHistograms )
		{
			cout << "write 1D histograms to disk" << endl;
		}
		cout << "\t minimum telescope multiplicity: " << fTableFillingCut_NImages_min << endl;
		cout << "\t distance to camera: > " << fMC_distance_to_cameracenter_min << " [deg], <";
		cout << fMC_distance_to_cameracenter_max << " [deg]" << endl;
	}
	if( fRerunStereoReconstruction )
	{
		cout << "\t rerunning stereo reconstruction" << endl;
		if( fRerunStereoReconstruction_BDTFileName.size() > 0 )
		{
			cout << "\t reading BDT TMVA files from " << fRerunStereoReconstruction_BDTFileName << endl;
			cout << "\t BDT TMVA stereo reconstruction is applied for events with <= ";
			cout << fRerunStereoReconstruction_BDTNImages_max << " images" << endl;
			if( fmaxdist < 1.e3 )
			{
				cout << "\t BDT TMVA stereo reconstruction distance cut < " << fmaxdist << endl;
			}
			if( fmaxloss < 1. )
			{
				cout << "\t BDT TMVA stereo reconstruction loss cut < " << fmaxloss << endl;
			}
			cout << "\t Head/tail uncertainty: ";
			if( fDisp_UseIntersectForHeadTail )
			{
				cout << " use intersetion method to resolve" << endl;
			}
			else
			{
				cout << " use smallest diff" << endl;
			}
		}
	}
	if( iP == 2 && isMC )
	{
		cout << "zenith angle " << ze << " [deg], wobble offset " << fWobbleOffset / 100. << " [deg], noise level " << fNoiseLevel << endl;
	}
	if( fSelectRandom > 0. )
	{
		cout << "random event selection: " << fSelectRandom << ", seed:" << fSelectRandomSeed << endl;
	}
	if( fUseSelectedImagesOnly )
	{
		cout << "\t use evndisp image selection" << endl;
	}
	else
	{
		cout << "\t use all images" << endl;
	}
	if( readwrite == 'W' )
	{
		cout << "minimum number of showers required per lookup table bin: " << fMinRequiredShowerPerBin << endl;
	}
	if( fUseMedianEnergy == 1 )
	{
		cout << "use median of energy distributions" << endl;
	}
	else if( fUseMedianEnergy == 2 )
	{
		cout << "use median+mpv of energy distributions" << endl;
	}
	else
	{
		cout << "use mean of energy distributions" << endl;
	}
	if( fUpdateInstrumentEpoch )
	{
		cout << "updating instrument epoch from default epoch file" << endl;
	}
	
	if( iP >= 1 )
	{
		cout << endl;
		if( meanpedvars > 0. )
		{
			cout << "mean pedvars: " << meanpedvars << endl;
			cout << "mean pedvars per telescope: ";
			for( unsigned int i = 0; i < pedvars.size(); i++ )
			{
				cout << pedvars[i] << "/";
			}
			cout << endl;
		}
		else
		{
			cout << "no pedvar information available" << endl;
		}
	}
}

bool VTableLookupRunParameter::fillInputFile_fromList( string iList )
{
	ifstream is;
	is.open( iList.c_str(), ifstream::in );
	if( !is )
	{
		cout << "VTableLookupRunParameter::fillInputFile_fromList() error reading list of input files: " << endl;
		cout << iList << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	cout << "VTableLookupRunParameter::fillInputFile_fromList() reading input file list: " << endl;
	cout << iList << endl;
	string iLine;
	while( getline( is, iLine ) )
	{
		if( iLine.size() > 0 )
		{
			inputfile.push_back( iLine );
		}
	}
	is.close();
	
	cout << "total number of input files " << inputfile.size() << endl;
	
	return true;
}
