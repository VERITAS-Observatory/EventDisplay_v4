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
	fInterpolate = 0;
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
	point_source = false;
	esysfile = "";
	fWrite1DHistograms = false;
	fSpectralIndex = 2.0;
	fWobbleOffset = 500;     // integer of wobble offset * 100
	fNoiseLevel = 250;
	fTableFillingCut_NImages_min = 2;
	fTableFillingCut_WobbleCut_max = 15.;
	fminsize = 0.;
	fmaxdist = 50000.;
	fSelectRandom = -1.;
	fSelectRandomSeed = 17;
	//fMSCWSizecorrection = 1.;
	//fMSCLSizecorrection = 1.;
	//fEnergySizecorrection = 1.;
        fMSCWSizecorrection_mean=0;
        fMSCLSizecorrection_mean=0;
        fEnergySizecorrection_mean=0;
	
        for (Int_t k=0; k<VDST_MAXTELESCOPES; k++){
            // Allow for a large (200) number of telescopes. For VTS only the first 4 will be used
            // Default scalings set to 1. Will be replaced later with data from a runparameter file
	    fMSCWSizecorrection.push_back(1.);
            fMSCLSizecorrection.push_back(1.);
            fEnergySizecorrection.push_back(1.);
        }


	fLimitEnergyReconstruction = false;
	
	fMC_distance_to_cameracenter_min =  0.;
	fMC_distance_to_cameracenter_max =  1.e10;
	
	//	fNentries = TChain::kBigNumber;
	// Setting to ROOT 5 value to prevent ROOT 6 Errors
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
		else if( iTemp.find( "-esysfi" ) < iTemp.size() )
		{
			if( iTemp2.size() > 0 )
			{
				esysfile = iTemp2;
				i++;
			}
		}
		else if( iTemp.find( "-limitEnergyReconstruction" ) < iTemp.size() )
		{
			fLimitEnergyReconstruction = true;
		}
		else if( iTemp.find( "-interpolate" ) < iTemp.size() )
		{
			fInterpolate = atoi( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
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
		else if( iTemp.find( "-maxdist" ) < iTemp.size() && !( iTemp.find( "-maxdistancetocameracenter" ) < iTemp.size() ) )
		{
			fmaxdist = atof( iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() ).c_str() );
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
		else if( iTemp.find( "-CTAoffAxisBins" ) < iTemp.size() )
		{
			setCTA_MC_offaxisBins();
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
		else if( iTemp.find( "-sizecorrect" ) < iTemp.size() )
		{
                        float _scale;
                        int   sizecorr_ntel=0;
                        iTemp = iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() );
                        if ( iTemp.size() == (iTemp.substr( iTemp.find( "," ) + 1, iTemp.size())).size() ) 
                        {
                            // special case, user set only 1 value, assuming it is the same for all telescopes
                            _scale = atof( iTemp.substr( 0, iTemp.find( "," ) ).c_str() );
                            for (unsigned long int k=0; k<VDST_MAXTELESCOPES; k++){
                                fMSCWSizecorrection[k] = _scale;
                                fMSCLSizecorrection[k] = _scale;
                                fEnergySizecorrection[k] = _scale;
                            
                            }
                            fMSCWSizecorrection_mean = _scale;
                            fMSCLSizecorrection_mean = _scale;
                            fEnergySizecorrection_mean = _scale;
                        }                
                        else 
                        {                                                            
                            // read comma separated corrections
                            for (unsigned long int k=0; k<VDST_MAXTELESCOPES; k++)
                            {
                                _scale = atof( iTemp.substr( 0, iTemp.find( "," ) ).c_str() );
                                fMSCWSizecorrection[k] = _scale;
                                fMSCLSizecorrection[k] = _scale;
                                fEnergySizecorrection[k] = _scale;
                                fMSCWSizecorrection_mean += _scale;
                                fMSCLSizecorrection_mean += _scale;
                                fEnergySizecorrection_mean += _scale;
                                sizecorr_ntel++;
                                if ( iTemp.size() == (iTemp.substr( iTemp.find( "," ) + 1, iTemp.size())).size())
                                {
                                    break;
                                }
                                iTemp = iTemp.substr( iTemp.find( "," ) + 1, iTemp.size() );
                            }
                            fMSCWSizecorrection_mean = fMSCWSizecorrection_mean/sizecorr_ntel;
                            fMSCLSizecorrection_mean = fMSCLSizecorrection_mean/sizecorr_ntel;
                            fEnergySizecorrection_mean = fEnergySizecorrection_mean/sizecorr_ntel;
                        }
		}
		else if( iTemp.find( "-sizemscwcorrect" ) < iTemp.size() )
		{
                        float _scale;
                        int   sizecorr_ntel=0;
                        iTemp = iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() );

                        if ( iTemp.size() == (iTemp.substr( iTemp.find( "," ) + 1, iTemp.size())).size() ) 
                        {
                            // special case, user set only 1 value, assuming it is the same for all telescopes
                            _scale = atof( iTemp.substr( 0, iTemp.find( "," ) ).c_str() );
                            for (unsigned long int k=0; k<VDST_MAXTELESCOPES; k++){
                                fMSCWSizecorrection[k] = _scale;
                            
                            }
                            fMSCWSizecorrection_mean = _scale;
                        }                
                        else 
                        {                                                           
                            // read comma separated corrections
                            for (unsigned long int k=0; k<VDST_MAXTELESCOPES; k++)
                            {
                                _scale = atof( iTemp.substr( 0, iTemp.find( "," ) ).c_str() );
                                fMSCWSizecorrection[k] = _scale;
                                fMSCWSizecorrection_mean += _scale;
                                sizecorr_ntel++;
                                if ( iTemp.size() == (iTemp.substr( iTemp.find( "," ) + 1, iTemp.size())).size() )
                                {
                                    break;
                                }
                                iTemp = iTemp.substr( iTemp.find( "," ) + 1, iTemp.size() );
                            }
                            fMSCWSizecorrection_mean = fMSCWSizecorrection_mean/sizecorr_ntel;
                        }
		}
		else if( iTemp.find( "-sizemsclcorrect" ) < iTemp.size() )
		{
                        float _scale;
                        iTemp = iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() );
                        if ( iTemp.size() == (iTemp.substr( iTemp.find( "," ) + 1, iTemp.size())).size() ) 
                        {
                            // special case, user set only 1 value, assuming it is the same for all telescopes
                            _scale = atof( iTemp.substr( 0, iTemp.find( "," ) ).c_str() );
                            for (unsigned long int k=0; k<VDST_MAXTELESCOPES; k++){
                                fMSCLSizecorrection[k] = _scale;
                            
                            }
                            fMSCLSizecorrection_mean = _scale;
                        }                
                        else
                        {
                            // read comma separated corrections
                            for (unsigned long int k=0; k<VDST_MAXTELESCOPES; k++)
                            {
                                _scale = atof( iTemp.substr( 0, iTemp.find( "," ) ).c_str() );
                                fMSCLSizecorrection[k] = _scale;
                                fMSCLSizecorrection_mean += _scale;
                                sizecorr_ntel++;
                                if ( iTemp.size() == (iTemp.substr( iTemp.find( "," ) + 1, iTemp.size())).size())
                                {
                                    break;
                                }
                                iTemp = iTemp.substr( iTemp.find( "," ) + 1, iTemp.size() );
                            }
                            fMSCLSizecorrection_mean = fMSCLSizecorrection_mean/sizecorr_ntel;
                        }
		}
		else if( iTemp.find( "-sizeenergycorrect" ) < iTemp.size() )
		{
                        float _scale;
                        iTemp = iTemp.substr( iTemp.rfind( "=" ) + 1, iTemp.size() );
                        if ( iTemp.size() == (iTemp.substr( iTemp.find( "," ) + 1, iTemp.size())).size() ) 
                        {
                            // special case, user set only 1 value, assuming it is the same for all telescopes
                            _scale = atof( iTemp.substr( 0, iTemp.find( "," ) ).c_str() );
                            for (unsigned long int k=0; k<VDST_MAXTELESCOPES; k++){
                                fEnergySizecorrection[k] = _scale;
                            
                            }
                            fEnergySizecorrection_mean = _scale;
                        }                
                        else
                        {
                            // read comma separated corrections
                            for (unsigned long int k=0; k<VDST_MAXTELESCOPES; k++)
                            {
                                _scale = atof( iTemp.substr( 0, iTemp.find( "," ) ).c_str() );
                                fEnergySizecorrection[k] = _scale;
                                fEnergySizecorrection_mean += _scale;
                                if ( iTemp.size() == (iTemp.substr( iTemp.find( "," ) + 1, iTemp.size())).size())
                                {
                                    break;
                                }
                                iTemp = iTemp.substr( iTemp.find( "," ) + 1, iTemp.size() );
                            }
                            fEnergySizecorrection_mean = fEnergySizecorrection_mean/sizecorr_ntel;
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
		system( "cat $EVNDISPSYS/README/README.MSCW_ENERGY" );
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
	printCTA_MC_offaxisBins();
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
	if( readwrite == 'W' || readwrite == 'w' )
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
	if( iP == 2 )
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
	if( fLimitEnergyReconstruction )
	{
		cout << "limited energy tables" << endl;
	}
	if( !( readwrite == 'W' || readwrite == 'w' ) || iP == 2 )
	{
		if( esysfile.size() > 0 )
		{
			cout << "correct for systematic errors with " << esysfile << endl;
		}
	}
	else
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
	if( fInterpolate > 0 )
	{
		cout << "WARNING: interpolation switched off for efficiency reasons" << endl;
		/*        if( fInterpolate == 1 ) fInterpolateString = "simple";
		        else if( fInterpolate == 2 ) fInterpolateString = "gaussian";
		        cout << "interpolate lookup tables: " << fInterpolateString << endl; */
	}
       
        // Check the average scaling factors among all telescopes. If it deviates from 1, print it 
        if( fMSCWSizecorrection_mean!=0 && TMath::Abs( fMSCWSizecorrection_mean - 1. ) > 1.e-2 )
        {
                cout << "Mean size correction for mscw tables: " << fMSCWSizecorrection_mean << endl;
        }
        if( fMSCLSizecorrection_mean!=0 && TMath::Abs( fMSCLSizecorrection_mean - 1. ) > 1.e-2 )
        {
                cout << "Mean size correction for mscl tables: " << fMSCLSizecorrection_mean << endl;
        }
        if( fEnergySizecorrection_mean!=0 && TMath::Abs( fEnergySizecorrection_mean - 1. ) > 1.e-2 )
        {
                cout << "Mean size correction for energy tables: " << fEnergySizecorrection_mean << endl;
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

void VTableLookupRunParameter::setCTA_MC_offaxisBins()
{
	fCTA_MC_offaxisBin_min.clear();
	fCTA_MC_offaxisBin_max.clear();
	
	fCTA_MC_offaxisBin_min.push_back( 0.0 );
	fCTA_MC_offaxisBin_max.push_back( 1.0 );
	
	fCTA_MC_offaxisBin_min.push_back( 1.0 );
	fCTA_MC_offaxisBin_max.push_back( 2.0 );
	
	fCTA_MC_offaxisBin_min.push_back( 2.0 );
	fCTA_MC_offaxisBin_max.push_back( 3.0 );
	
	fCTA_MC_offaxisBin_min.push_back( 3.0 );
	fCTA_MC_offaxisBin_max.push_back( 3.5 );
	
	fCTA_MC_offaxisBin_min.push_back( 3.5 );
	fCTA_MC_offaxisBin_max.push_back( 4.0 );
	
	fCTA_MC_offaxisBin_min.push_back( 4.0 );
	fCTA_MC_offaxisBin_max.push_back( 4.5 );
	
	fCTA_MC_offaxisBin_min.push_back( 4.5 );
	fCTA_MC_offaxisBin_max.push_back( 5.0 );
	
	fCTA_MC_offaxisBin_min.push_back( 5.0 );
	fCTA_MC_offaxisBin_max.push_back( 5.5 );
	
	fCTA_MC_offaxisBin_min.push_back( 5.5 );
	fCTA_MC_offaxisBin_max.push_back( 6.0 );
}

void VTableLookupRunParameter::printCTA_MC_offaxisBins()
{
	if( fCTA_MC_offaxisBin_min.size() == 0 )
	{
		return;
	}
	
	cout << "setting the following off-axis bins for CTA analysis: " << endl;
	for( unsigned int i = 0; i < fCTA_MC_offaxisBin_min.size(); i++ )
	{
		cout << "   bin " << i << "\t min " << fCTA_MC_offaxisBin_min[i] << " deg, max " << fCTA_MC_offaxisBin_max[i] << " deg" << endl;
	}
}
