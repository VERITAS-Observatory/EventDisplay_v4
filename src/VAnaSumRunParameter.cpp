/*! \class VAnaSumRunParameter
 *
 *
*/

#include "VAnaSumRunParameter.h"

VAnaSumRunParameterDataClass::VAnaSumRunParameterDataClass()
{
	fEventDisplayVersion = "";
	
	fRunOn = 0;
    fRunOnFileName = "";
	fRunOff = 0;
    fRunOffFileName = "";
	
	fMJDOn = 0.;
	fMJDOff = 0.;

    fMJDOnStart = 0.;
    fMJDOnStop = 0.;
	
	fTarget = "";
	fTargetRAJ2000 = 0.;
	fTargetDecJ2000 = -90.;
	fTargetRA = 0.;
	fTargetDec = 0.;

    fArrayPointingRA = 0.;
    fArrayPointingDec = 0.;
    fArrayPointingRAJ2000 = 0.;
    fArrayPointingDecJ2000 = 0.;
	
	fPairOffset = 0.;
	
	fWobbleNorth = 0.;                            // [deg]
	fWobbleWest = 0.;                             // [deg]
	fWobbleNorthMod = 0.;                         // [deg]
	fWobbleWestMod = 0.;                          // [deg]
	
	fSkyMapCentreNorth = 0.;
	fSkyMapCentreWest = 0.;
	fSkyMapCentreRAJ2000 = 0.;
	fSkyMapCentreDecJ2000 = 0.;
	
	fTargetShiftNorth = 0.;
	fTargetShiftWest = 0.;
	fTargetShiftRAJ2000 = 0.;
	fTargetShiftDecJ2000 = 0.;
	
	fNTel = 4;
    fTelToAna = "";
	fMaxTelID = fNTel;
	
	fBackgroundModel = 0;
	fSourceRadius = 0.;                           // actually radius^2
	fmaxradius = 0.;                              // maximum accepted distance from camera center [deg]
	
	fCutFile = "";
	
	fAcceptanceFile = "";
	
	fEffectiveAreaFile = "";                      // file with effective areas, use NOFILE if not avaible
	
	// smoothing algorithm (don't use it if you don't know it)
	fNBoxSmooth = 0;
	
	// ON/OFF MODEL
	fOO_alpha = 0.;
	
	// RING BACKGROUND MODEL
	fRM_RingRadius = 0.;                          // ring radius [deg]
	fRM_RingWidth = 0.;                           // ring width [deg]
	
	// REFLECTED REGION MODEL
	fRE_distanceSourceOff = 0.2;                  // minimal distance of off source regions in number of background regions from the source region
	fRE_nMinoffsource = 3;                        // minmum number of off source regions (default 3)
	fRE_nMaxoffsource = 7;                        // maximum number of off source regions (default 7)
	
	// TEMPLATE MODEL
	fTE_mscw_min = 0.;
	fTE_mscw_max = 0.;
	fTE_mscl_min = 0.;
	fTE_mscl_max = 0.;

        f2DAcceptanceMode = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////


VAnaSumRunParameter::VAnaSumRunParameter()
{
	// default version number (important for reading of run parameters from root file)
	fVersion = 6;
	
	// bin size for sky maps [deg]
	//   must be the same for all runs (add up sky maps)
	fSkyMapBinSize   = 0.01;
	fSkyMapBinSizeUC = 0.05;
	fSkyMapSizeXmin = -2.;
	fSkyMapSizeXmax =  2.;
	fSkyMapSizeYmin = -2.;
	fSkyMapSizeYmax =  2.;
	
	// sky maps are centred around this point
	fSkyMapCentreNorth = 0.;
	fSkyMapCentreWest = 0.;
	fSkyMapCentreRAJ2000 = 0.;
	fSkyMapCentreDecJ2000 = 0.;
	
	// position relative to which 1D histograms are filled
	fTargetShiftNorth = 0.;                       // [deg]
	fTargetShiftWest = 0.;                        // [deg];
	fTargetShiftRAJ2000 = 0.;
	fTargetShiftDecJ2000 = 0.;
	
	// parameter for energy spectra (in log E)
	fEnergyReconstructionSpectralIndex = 2.5;
	fEnergyReconstructionMethod = 1;
	fEffectiveAreaVsEnergyMC = 1;             // default: use effective areas vs reconstructed energy (accurate method)
	fEnergySpectrumBinSize = 0.05;
	fEnergyEffectiveAreaSmoothingIterations = -1;
	fEnergyEffectiveAreaSmoothingThreshold = -1.;
	fDeadTimeCalculationMethod = 0;
	
	// background model
	fTMPL_fBackgroundModel = 0;
	fTMPL_RM_RingRadius = 0.;
	fTMPL_RM_RingWidth = 0.;
	fTMPL_RE_distanceSourceOff = 0.;
	fTMPL_RE_nMinoffsource = 0;
	fTMPL_RE_nMaxoffsource = 0;
	fTMPL_RE_RemoveOffRegionsRandomly = false;
	
	// cut, effective areas and acceptance files
	fTMPL_SourceRadius = 0.;
	fTMPL_maxradius = 0.;
	fTMPL_CutFile = "";
	fTMPL_AcceptanceFile = "";
	fTMPL_EffectiveAreaFile = "";
	
	// star catalogue
	fStarCatalogue = "Hipparcos_MAG8_1997.dat";
	// minimum brightness of stars
	fStarMinBrightness = 6.;
	fStarBand = "B";
	// do not exclude regions around stars by default
	fStarExlusionRadius = -1.;
	
	// length of time intervalls in seconds for rate plots and short term histograms
	fTimeIntervall = 4. * 60.;
	
    // likelihood analysis
    fLikelihoodAnalysis = false;
	
	// Write all events to DL3 Tree
	fWriteAllEvents = false;
    // Write Dataon/dataoff trees
    fWriteDataOnOffTrees = false;

	// if 0, use default 1D radial acceptance
	// if >0, use alternate 2D-dependent acceptance
	f2DAcceptanceMode = 0 ; // USE2DACCEPTANCE
	
	// for deadtime fraction storage
	fScalarDeadTimeFrac = 0.0 ;
	
	// set monte carlo zenith angles
	setMCZenith();
}


int VAnaSumRunParameter::returnWithError( string iL, string iM, string iC )
{
	cout << endl;
	cout << iL << endl;
	cout << iM << endl;
	if( iC.size() > 0 )
	{
		cout << "correct writing: " << endl;
		cout << iC << endl;
	}
	return 0;
}

/*

    read run parameters from an ascii file

*/
int VAnaSumRunParameter::readRunParameter( string i_filename )
{
	ifstream is;
	is.open( i_filename.c_str(), ifstream::in );
	if( !is )
	{
		string itemp = getDirectory_EVNDISPParameterFiles();
		itemp += "/" + i_filename;
		is.open( itemp.c_str(), ifstream::in );
		if( !is )
		{
			cout << "no file found to read run parameters: " << itemp << endl;
			exit( 0 );
		}
		i_filename = itemp;
	}
	cout << "Reading anasum parameters from " << i_filename << " :" << endl;
	cout << endl;
	string is_line;
	string temp;
	string temp2;
	
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
			// print runparameter to stdout
			cout << is_line << endl;
			if( (is_stream>>std::ws).eof() )
			{
				return returnWithError( "VAnaSumRunParameter::readRunParameter: not enough parameters", is_line );
			}
			is_stream >> temp;
			if( (is_stream>>std::ws).eof() )
			{
				return returnWithError( "VAnaSumRunParameter::readRunParameter: not enough parameters", is_line );
			}
			is_stream >> temp2;
			if( temp == "TIMEMASKFILE" )
			{
				fTimeMaskFile = temp2;
				// check if timemask file needs an additional path
				ifstream is_test;
				is_test.open( fTimeMaskFile.c_str(), ifstream::in );
				if( !is_test )
				{
					string iDIR_temp = i_filename.substr( 0, i_filename.rfind( "/" ) );
					iDIR_temp += "/" + fTimeMaskFile;
					is_test.open( iDIR_temp.c_str(), ifstream::in );
					if( !is_test )
					{
						cout << "Error opening time mask file: " << fTimeMaskFile << endl;
						cout << "exiting..." << endl;
						exit( 0 );
					}
					else
					{
						fTimeMaskFile = iDIR_temp;
					}
				}
				is_test.close();
			}
			else if( temp == "GAMMAHADRONCUT" )
			{
				fTMPL_CutFile = temp2;
			}
			else if( temp == "RADIALACCEPTANCEFILE" )
			{
				fTMPL_AcceptanceFile = temp2;
			}
			else if( temp == "EFFECTIVEAREAFILE" )
			{
				fTMPL_EffectiveAreaFile = temp2;
			}
			else if( temp == "REFLECTEDREGION" )
			{
				fTMPL_fBackgroundModel = eREFLECTEDREGION;
				fTMPL_RE_distanceSourceOff = atof( temp2.c_str() );
				if( !(is_stream>>std::ws).eof() )
				{
					is_stream >> temp2;
					fTMPL_RE_nMinoffsource = atoi( temp2.c_str() );
				}
				else
				{
					returnWithError( "VAnaSumRunparameter: not enough parameters: ", is_line, "* REFLECTEDREGION dist noff_min noff_max" );
				}
				if( !(is_stream>>std::ws).eof() )
				{
					is_stream >> temp2;
					fTMPL_RE_nMaxoffsource = atoi( temp2.c_str() );
				}
				else
				{
					returnWithError( "VAnaSumRunparameter: not enough parameters: ", is_line, "* REFLECTEDREGION dist noff_min noff_max" );
				}
			}
			else if( temp == "REFLECTEDREGION_OFFREMOVAL" )
			{
				if( !(is_stream>>std::ws).eof() )
				{
					is_stream >> temp2;
					fTMPL_RE_RemoveOffRegionsRandomly = bool( atoi( temp2.c_str() ) );
				}
			}
			else if( temp == "RINGBACKGROUND" )
			{
				fTMPL_fBackgroundModel = eRINGMODEL;
				fTMPL_RM_RingRadius = atof( temp2.c_str() );
				if( !(is_stream>>std::ws).eof() )
				{
					is_stream >> temp2;
					fTMPL_RM_RingWidth = atof( temp2.c_str() );
				}
				else
				{
					returnWithError( "VAnaSumRunparameter: not enough parameters: ", is_line, "* RINGBACKGROUND ring_radius ring_area" );
				}
			}
			else if( temp == "SKYMAPBINSIZE" )
			{
				fSkyMapBinSize = atof( temp2.c_str() );
			}
			else if( temp == "SKYMAPBINSIZEUC" )
			{
				fSkyMapBinSizeUC = atof( temp2.c_str() );
			}
			else if( temp == "SKYMAPSIZEX" )
			{
				fSkyMapSizeXmin = atof( temp2.c_str() );
                fSkyMapSizeXmin = -1. *TMath::Abs( fSkyMapSizeXmin );
                fSkyMapSizeXmax = TMath::Abs( fSkyMapSizeXmin );
			}
			else if( temp == "SKYMAPSIZEY" )
			{
				fSkyMapSizeYmin = atof( temp2.c_str() );
                fSkyMapSizeYmin = -1. *TMath::Abs( fSkyMapSizeYmin );
                fSkyMapSizeYmax = TMath::Abs( fSkyMapSizeYmin );
			}
			else if( temp == "BRIGHTSTARCATALOGUE" )
			{
				fStarCatalogue = temp2;
			}
			else if( temp == "BRIGHTSTARBRIGHTNESS" )
			{
				fStarMinBrightness = atof( temp2.c_str() );
			}
			else if( temp == "BRIGHTSTARBAND" )
			{
				fStarBand = temp2;
			}
			else if( temp == "BRIGHTSTAREXCLUDEFROMBACKGROUNDRADIUS" )
			{
				fStarExlusionRadius = atof( temp2.c_str() );
				if( fStarExlusionRadius < 1.e-5 )
				{
					fStarExlusionRadius = 0.;
				}
			}
                        else if( temp == "BRIGHTSTARSETTINGS" )
                        {
                                fStarMinBrightness = atof( temp2.c_str() );
                                string iStarBand = "";
                                if( !(is_stream>>std::ws).eof() )
                                {
                                    is_stream >> fStarExlusionRadius;
                                }
                                if( fStarExlusionRadius < 1.e-5 )
                                {
                                        fStarExlusionRadius = 0.;
                                }
                                if( !(is_stream>>std::ws).eof() )
                                {
                                    is_stream >> fStarBand;
                                }
                        }
			else if( temp == "SKYMAPCENTRE_XY" )
			{
				if( checkNumberOfArguments( is_line ) != 4 )
				{
					return returnWithError( "VAnaSumRunparameter: not enough parameters: ", is_line, "* SKYMAPCENTRE_XY x y" );
				}
				
				fSkyMapCentreWest = -1.*atof( temp2.c_str() );
				is_stream >> temp2;
				fSkyMapCentreNorth = -1.*atof( temp2.c_str() );
			}
			else if( temp == "SKYMAPCENTRE_RADECJ2000_DEG" )
			{
				if( checkNumberOfArguments( is_line ) != 4 )
				{
					return returnWithError( "VAnaSumRunparameter: not enough parameters: ", is_line,
											"* SKYMAPCENTRE_RADECJ2000_DEG (RA(deg) DEC(deg)" );
				}
				fSkyMapCentreRAJ2000 = atof( temp2.c_str() );
				is_stream >> temp2;
				fSkyMapCentreDecJ2000 = atof( temp2.c_str() );
			}
			else if( temp == "SKYMAPCENTRE_RADECJ2000_HOUR" )
			{
				if( checkNumberOfArguments( is_line ) != 8 )
				{
					return returnWithError( "VAnaSumRunparameter: not enough parameters: ", is_line,
											"* SKYMAPCENTRE_RADECJ2000_HOUR RA(Hour Min Sec)  DEC(Deg Min Sec)" );
				}
				double d_tt = 0.;
				d_tt += atof( temp2.c_str() );
				is_stream >> temp2;
				d_tt += atof( temp2.c_str() ) / 60.;
				is_stream >> temp2;
				d_tt += atof( temp2.c_str() ) / 3600.;
				fSkyMapCentreRAJ2000 = d_tt / 24. * 360.;
				// dec
                                string iDec1;
                                string iDec2;
                                string iDec3;
				is_stream >> iDec1;
				is_stream >> iDec2;
				is_stream >> iDec3;
                                fSkyMapCentreDecJ2000 = getDeclinationFromStrings( iDec1, iDec2, iDec3 );
			}
			else if( temp == "TARGETXYSHIFT" )
			{
				if( checkNumberOfArguments( is_line ) != 4 )
				{
					return returnWithError( "VAnaSumRunparameter: not enough parameters: ", is_line, "* TARGETXYSHIFT x y" );
				}
				
				fTargetShiftWest = -1.*atof( temp2.c_str() );
				is_stream >> temp2;
				fTargetShiftNorth = -1.*atof( temp2.c_str() );
			}
			else if( temp == "TARGETPOSITION_RADECJ2000_DEG" )
			{
				if( checkNumberOfArguments( is_line ) != 4 )
				{
					return returnWithError( "VAnaSumRunparameter: not enough parameters: ", is_line, "* TARGETPOSITION_RADECJ2000_DEG (RA(deg) DEC(deg)" );
				}
				
				fTargetShiftRAJ2000 = atof( temp2.c_str() );
				is_stream >> temp2;
				fTargetShiftDecJ2000 = atof( temp2.c_str() );
			}
			else if( temp == "TARGETPOSITION_RADECJ2000_HOUR" )
			{
				if( checkNumberOfArguments( is_line ) != 8 )
				{
					return returnWithError( "VAnaSumRunparameter: not enough parameters: ", is_line, "* TARGETPOSITION_RADECJ2000_HOUR RA(Hour Min Sec)  DEC(Deg Min Sec)" );
				}
				
				double d_tt = 0.;
				d_tt += atof( temp2.c_str() );
				is_stream >> temp2;
				d_tt += atof( temp2.c_str() ) / 60.;
				is_stream >> temp2;
				d_tt += atof( temp2.c_str() ) / 3600.;
				fTargetShiftRAJ2000 = d_tt / 24. * 360.;
				// dec
                string iDec1;
                string iDec2;
                string iDec3;
				is_stream >> iDec1;
				is_stream >> iDec2;
				is_stream >> iDec3;
                fTargetShiftDecJ2000 = getDeclinationFromStrings( iDec1, iDec2, iDec3 );
			}
			
			else if( temp == "REGIONTOEXCLUDE" || temp == "REGIONTOEXCLUDE_RADECJ2000_DEG" )
			{
				if( checkNumberOfArguments( is_line ) < 5 )
				{
					return returnWithError( "VAnaSumRunparameter: not enough parameters: ", is_line, "* REGIONTOEXCLUDE (West(deg)  North(deg)  Radius(deg)) (or * REGIONTOEXCLUDE_RADECJ2000_DEG (RA(deg) DEC(deg) Radius(deg)) OR * REGIONTOEXCLUDE (West(deg)  North(deg)  Radius1(deg) Radius2(deg) RotAngle(deg)) (or * REGIONTOEXCLUDE_RADECJ2000_DEG (RA(deg) DEC(deg) Radius1(deg) Radius2(deg) RotAngle(deg)). Check if you want extended or point source!" );
				}
				fExclusionRegions.push_back( new VAnaSumRunParameterListOfExclusionRegions() );
				
				if( temp == "REGIONTOEXCLUDE" )
				{
					fExclusionRegions.back()->fExcludeFromBackground_West = -1.* ( double )atof( temp2.c_str() );
					is_stream >> temp2;
					fExclusionRegions.back()->fExcludeFromBackground_North = -1.* ( double )atof( temp2.c_str() );
				}
				else if( temp == "REGIONTOEXCLUDE_RADECJ2000_DEG" )
				{
					fExclusionRegions.back()->fExcludeFromBackground_RAJ2000 = ( double )atof( temp2.c_str() );
					is_stream >> temp2;
					fExclusionRegions.back()->fExcludeFromBackground_DecJ2000 = ( double )atof( temp2.c_str() );
				}
				// for circular region
				if( checkNumberOfArguments( is_line ) == 5 )
				{
					is_stream >> temp2;
                                	fExclusionRegions.back()->fExcludeFromBackground_Radius1 = ( double )atof( temp2.c_str() );
					fExclusionRegions.back()->fExcludeFromBackground_Radius2 = fExclusionRegions.back()->fExcludeFromBackground_Radius1;
					fExclusionRegions.back()->fExcludeFromBackground_RotAngle = 0.;
				}
				// for ellipsoidal region
				else if( checkNumberOfArguments( is_line ) == 7 )
				{
					is_stream >> temp2;
					fExclusionRegions.back()->fExcludeFromBackground_Radius1 = ( double )atof( temp2.c_str() );
					is_stream >> temp2;
					fExclusionRegions.back()->fExcludeFromBackground_Radius2 = ( double )atof( temp2.c_str() );
					is_stream >> temp2;
					fExclusionRegions.back()->fExcludeFromBackground_RotAngle = ( double ) atof( temp2.c_str() );
				}
				else if( checkNumberOfArguments( is_line ) == 6 )
				{
					return returnWithError( "VAnaSumRunparameter: not enough parameters: ", is_line, "* REGIONTOEXCLUDE (West(deg)  North(deg)  Radius1(deg) Radius2(deg) RotAngle(deg) (or * REGIONTOEXCLUDE_RADECJ2000_DEG (RA(deg) DEC(deg) Radius1(deg) Radius2(deg) RotAngle(deg))" );
				}
				else
                                {
                                        return returnWithError( "VAnaSumRunparameter: not enough parameters: ", is_line, "Check if you want point or extended source in AnasumRunParameter file!" );
                                }
			}
			else if( temp == "REGIONTOEXCLUDE_RADECJ2000_HOUR" )
			{
				if( checkNumberOfArguments( is_line ) < 9 )
				{
					return returnWithError( "VAnaSumRunparameter: not enough parameters: ", is_line, "* REGIONTOEXCLUDE_RADECJ2000_HOUR (RA(Hour Min Sec)  DEC(Deg Min Sec)  Radius(deg)) OR * REGIONTOEXCLUDE_RADECJ2000_HOUR (RA(Hour Min Sec)  DEC(Deg Min Sec)  Radius1(deg) Radius2(deg) RotAngle(deg)). Check if you want extended or point source!" );
				}
				fExclusionRegions.push_back( new VAnaSumRunParameterListOfExclusionRegions() );
				
				// ra
				double d_tt = 0.;
				d_tt += ( double )atof( temp2.c_str() );
				is_stream >> temp2;
				d_tt += ( double )atof( temp2.c_str() ) / 60.;
				is_stream >> temp2;
				d_tt += ( double )atof( temp2.c_str() ) / 3600.;
				fExclusionRegions.back()->fExcludeFromBackground_RAJ2000 = d_tt / 24. * 360.;
				// dec
                                string iDec1;
                                string iDec2;
                                string iDec3;
				is_stream >> iDec1;
				is_stream >> iDec2;
				is_stream >> iDec3;
                                fExclusionRegions.back()->fExcludeFromBackground_DecJ2000 = getDeclinationFromStrings( iDec1, iDec2, iDec3 );
				// for circular region
				if( checkNumberOfArguments( is_line ) == 9 )
                                {
                                        is_stream >> temp2;
                                        fExclusionRegions.back()->fExcludeFromBackground_Radius1 = ( double )atof( temp2.c_str() );
                                        fExclusionRegions.back()->fExcludeFromBackground_Radius2 = fExclusionRegions.back()->fExcludeFromBackground_Radius1;
					fExclusionRegions.back()->fExcludeFromBackground_RotAngle = 0.;
					cout << "Rotation angle of circular exclusion region is " << fExclusionRegions.back()->fExcludeFromBackground_RotAngle << endl;
                                }
				// for ellipsoidal region
				else if( checkNumberOfArguments( is_line ) == 11 )
                                {
                                        is_stream >> temp2;
                                        fExclusionRegions.back()->fExcludeFromBackground_Radius1 = ( double )atof( temp2.c_str() );
                                        is_stream >> temp2;
                                        fExclusionRegions.back()->fExcludeFromBackground_Radius2 = ( double )atof( temp2.c_str() );
					is_stream >> temp2;
					fExclusionRegions.back()->fExcludeFromBackground_RotAngle = ( double )atof( temp2.c_str() );
                                }
                                else
                                {
                                        cout << "Error: Not enough events for point or extended source! Check the arguments of circular and ellipsoidal regions!" << endl;
                                }
			}
			else if( temp == "ENERGYBINSIZE" )
			{
				fEnergySpectrumBinSize = atof( temp2.c_str() );
			}
			else if( temp == "ENERGYEFFECTIVEAREAS" )
			{
				if( temp2 == "MC" )
				{
					fEffectiveAreaVsEnergyMC = 0;
				}
				else if( temp2 == "REC" )
				{
					fEffectiveAreaVsEnergyMC = 1;
				}
				else
				{
					cout << "Unknown parameter for ENERGYEFFECTIVEAREAS in parameter file " << i_filename << ": " << temp2 << endl;
					cout << "use MC or REC (default)" << endl;
					return 0;
				}
			}
			else if( temp == "ENERGYRECONSTRUCTIONMETHOD" )
			{
				fEnergyReconstructionMethod = ( unsigned int )atoi( temp2.c_str() );
				// print a clear warning if method 0 is selected
				if( fEnergyReconstructionMethod == 0 )
				{
					cout << endl;
					cout << "WARNING: energy reconstruction 0 is no longer valid. For any standard analysise, please use method 1 by:" << endl;
					cout << "  open your anasum run parameter file and replace " << endl;
					cout << "* ENERGYRECONSTRUCTIONMETHOD 0" << endl;
					cout << "   by " << endl;
					cout << "* ENERGYRECONSTRUCTIONMETHOD 1" << endl;
					cout << "(if you really want to use method 0, you will have to look into the code to find the detour" << endl;
					return 0;
				}
				// horrible detour to make sure that users don't use the wrong method
				else if( fEnergyReconstructionMethod == 100 )
				{
					cout << "Warning: using energy reconstruction method 0" << endl;
					fEnergyReconstructionMethod = 0;
				}
				else if( fEnergyReconstructionMethod > 1 )
				{
					cout << "Unknown parameter for ENERGYRECONSTRUCTIONMETHOD in parameter file " << i_filename << ": " << temp2 << endl;
					cout << "allowed values are 0 and 1" << endl;
					return 0;
				}
			}
			else if( temp == "DEADTIMECALCULATIONMETHOD" )
			{
				fDeadTimeCalculationMethod = atoi( temp2.c_str() );
				if( fDeadTimeCalculationMethod != 0 && fDeadTimeCalculationMethod != 1 )
				{
					cout << "Unknown dead time calculation method (0=scalar method, 1=time difference)" << endl;
					return 0;
				}
			}
			else if( temp == "RATEINTERVALLLENGTH" )
			{
				fTimeIntervall = atof( temp2.c_str() ) * 60.;
			}
			// expect spectral index positive
			else if( temp == "ENERGYSPECTRALINDEX" )
			{
				fEnergyReconstructionSpectralIndex = fabs( atof( temp2.c_str() ) );
			}
			// effective area smoothing
			else if( temp == "ENERGYEFFAREASMOOTHITER" )
			{
				fEnergyEffectiveAreaSmoothingIterations = atoi( temp2.c_str() );
			}
			else if( temp == "ENERGYEFFAREASMOOTHTHRESH" )
			{
				fEnergyEffectiveAreaSmoothingThreshold = atof( temp2.c_str() );
			}
			////////////////////////////////////////////
			// Option USE2DACCEPTANCE within ANASUM.runparameter
			// * USE2DACCEPTANCE 0
			//     use normal radial acceptance
			// * USE2DACCEPTANCE 1
			//     use simple 2d acceptance model
			else if( temp == "USE2DACCEPTANCE" )
			{
				f2DAcceptanceMode = ( unsigned int )atoi( temp2.c_str() ) ;
			}
			/// enable likelihood analysis ///
			else if (temp == "ENABLELIKELIHOOD")
			{
				unsigned int tmpLikelihood = ( unsigned int )atoi( temp2.c_str() ) ;
				if( tmpLikelihood == 1)
				{
					fLikelihoodAnalysis = true;
				}
				else 
				{
					fLikelihoodAnalysis = false;
				}
				
			}
            // Write all events to DL3 run. This will write out also hadronic events and
            // add the MVA score and IsGamma to the tree.
            else if ( temp == "WRITEALLEVENTS" )
            {
                unsigned int tmpWriteAll = ( unsigned int )atoi( temp2.c_str() ) ;
                if( tmpWriteAll == 1 )
                {
                    fWriteAllEvents = true;
                }
            }
            else if( temp == "WRITEDATAONOFFEVENTS" )
            {
                if( (unsigned int)atoi( temp2.c_str() ) == 1 )
                {
                    fWriteDataOnOffTrees = true;
                }
            }
			else
			{
				cout << "Warning: unknown line in parameter file " << i_filename << ": " << endl;
				cout << is_line << endl;
			}
		}
	}
	if( fTMPL_CutFile.size() > 0 )
	{
		fTMPL_SourceRadius = readSourceRadius( fTMPL_CutFile );
		if( fTMPL_SourceRadius <= 0. )
		{
			cout << "error in reading run parameters: ";
			cout << "invalid source radius " << fTMPL_SourceRadius << endl;
			exit( -1 );
		}
		fTMPL_RM_RingWidth = getRingWidth( TMath::Pi() * fTMPL_SourceRadius, fTMPL_RM_RingRadius, fTMPL_RM_RingWidth );
		fTMPL_maxradius = readMaximumDistance( fTMPL_CutFile );
		if( fTMPL_maxradius <  0. )
		{
			cout << "error in reading run parameters: ";
			cout << "invalid maximum distance " << fTMPL_maxradius << endl;
			exit( -1 );
		}
	}
	else
	{
		fTMPL_SourceRadius = 0.1;
		fTMPL_maxradius = 2.0;
	}
	// prelimary: require same extension in x and y
	if( fabs( fSkyMapSizeXmax - fSkyMapSizeYmax ) > 1.e-3 )
	{
		return returnWithError( "VAnaSumRunParameter::readRunParameter: x and y extension of the sky map should be the same (preliminary)", "" );
	}
	is.close();
	cout << "========================================================" << endl;
	cout << "        end reading run parameters                      " << endl;
	cout << "========================================================" << endl;
	cout << endl;
	
	return 1;
}

int VAnaSumRunParameter::loadShortFileList( string i_listfilename, string iDataDir, bool bTotalAnalysisOnly )
{
	int i_nline = 0;
	ifstream is;
	is.open( i_listfilename.c_str(), ifstream::in );
	if( !is )
	{
		cout << " VAnaSumRunParameter::loadShortFileList error: file with list of runs not found : " << i_listfilename << endl;
		cout << "exiting..." << endl;
		exit( -1 );
	}
	string is_line;
	string temp;
	VAnaSumRunParameterDataClass i_sT;
	reset( i_sT );
	
	cout << "Reading short run list from " << i_listfilename << endl;
	
	while( getline( is, is_line ) )
	{
		if( is_line.size() > 0 )
		{
			istringstream is_stream( is_line );
			is_stream >> temp;
			cout << "RUN NUMBER " << temp << endl;
			i_sT.fRunOn = atoi( temp.c_str() );
			i_sT.fRunOff = atoi( temp.c_str() );
			// open mscw file and read out telescope participating in analysis
			temp = iDataDir + "/" + temp + ".mscw.root";
			TFile iF( temp.c_str(), "READ" );
			if( iF.IsZombie() )
			{
				cout << "VAnaSumRunParameter::loadShortFileList: error, data file not found: ";
				cout << temp << endl;
				exit( -1 );
			}
			VEvndispRunParameter* iPar = ( VEvndispRunParameter* )iF.Get( "runparameterV2" );
			if( !iPar )
			{
				cout << "VAnaSumRunParameter::loadShortFileList: error, no run parameters found in: ";
				cout << temp << endl;
				exit( -1 );
			}
			char hTelToAna[200];
			for( unsigned int i = 0; i < iPar->fTelToAnalyze.size(); i++ )
			{
				if( i == 0 )
				{
					sprintf( hTelToAna, "%d", iPar->fTelToAnalyze[i] + 1 );
				}
				else
				{
					sprintf( hTelToAna, "%s%d", hTelToAna, iPar->fTelToAnalyze[i] + 1 );
				}
			}
			iF.Close();
			i_sT.fTelToAna = hTelToAna;
			i_sT.fCutFile = fTMPL_CutFile;
			i_sT.fSourceRadius = fTMPL_SourceRadius;
			i_sT.fBackgroundModel = fTMPL_fBackgroundModel;
			i_sT.fmaxradius = fTMPL_maxradius;
			i_sT.fEffectiveAreaFile = fTMPL_EffectiveAreaFile;
			if( i_sT.fTelToAna == "1234" )
			{
				i_sT.fEffectiveAreaFile += ".root";
			}
			else
			{
				i_sT.fEffectiveAreaFile += "_T" + i_sT.fTelToAna + ".root";
			}
			i_sT.fAcceptanceFile = fTMPL_AcceptanceFile;
			if( i_sT.fTelToAna == "1234" )
			{
				i_sT.fAcceptanceFile += ".root";
			}
			else
			{
				i_sT.fAcceptanceFile += "_T" + i_sT.fTelToAna + ".root";
			}
			if( i_sT.fBackgroundModel == eRINGMODEL )
			{
				i_sT.fRM_RingRadius = fTMPL_RM_RingRadius;
				i_sT.fRM_RingWidth  = fTMPL_RM_RingWidth;
			}
			else if( i_sT.fBackgroundModel == eREFLECTEDREGION )
			{
				i_sT.fRE_distanceSourceOff = fTMPL_RE_distanceSourceOff;
				i_sT.fRE_nMinoffsource = fTMPL_RE_nMinoffsource;
				i_sT.fRE_nMaxoffsource = fTMPL_RE_nMaxoffsource;
			}
			else
			{
				cout << "VAnaSumRunParameter error: ";
				cout << " unknown background model " << i_sT.fBackgroundModel << endl;
				cout << "\t or" << endl;
				cout << "VAnaSumRunParameter warning: ";
				cout << " short runlist not implemented yet for this background model " << i_sT.fBackgroundModel << endl;
			}
			
			// fill the runlist vector
			i_sT.f2DAcceptanceMode = f2DAcceptanceMode ; // USE2DACCEPTANCE
			fRunList.push_back( i_sT );
			// fill the runlist map
			fMapRunList[i_sT.fRunOn] = fRunList.back();
			++i_nline;
			
		}
	}
	
	return i_nline;
}

int VAnaSumRunParameter::loadSimpleFileList( string i_listfilename )
{
	int i_nline = 0;
	ifstream is;
	is.open( i_listfilename.c_str(), ifstream::in );
	if( !is )
	{
		cout << " VAnaSumRunParameter:::loadSimpleFileList error: file with list of runs not found : " << i_listfilename << endl;
		cout << "exiting..." << endl;
		exit( -1 );
	}
	string is_line;
	string temp;
	VAnaSumRunParameterDataClass i_sT;
	reset( i_sT );
	
	cout << "Reading simple run list from: " << i_listfilename << endl;
	
	while( getline( is, is_line ) )
	{
		if( is_line.size() > 0 )
		{
			istringstream is_stream( is_line );
			is_stream >> temp;
			// read run list
			i_sT.fRunOn = atoi( temp.c_str() );
			i_sT.fRunOff = atoi( temp.c_str() );
			// fill the runlist vector
			i_sT.f2DAcceptanceMode = f2DAcceptanceMode ; // USE2DACCEPTANCE
			fRunList.push_back( i_sT );
			// fill the runlist map
			fMapRunList[i_sT.fRunOn] = fRunList.back();
			++i_nline;
		}
	}
	return i_nline;
}

/*

   read (old style) long run list

*/
int VAnaSumRunParameter::loadLongFileList( string i_listfilename, bool bShortList, bool bTotalAnalysisOnly )
{
	int i_nline = 0;
	ifstream is;
	is.open( i_listfilename.c_str(), ifstream::in );
	if( !is )
	{
		cout << " VAnaSumRunParameter::loadLongFileList error: file with list of runs not found : " << i_listfilename << endl;
		cout << "exiting..." << endl;
		exit( -1 );
	}
	string is_line;
	string temp;
	VAnaSumRunParameterDataClass i_sT;
	reset( i_sT );
	
	cout << "Reading long run list from (S" << bShortList << ", TA" << bTotalAnalysisOnly << "): " << i_listfilename << endl;
	
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
			
			int narg = checkNumberOfArguments( is_line );
			// check version number
			if( narg == 3 )
			{
				is_stream >> temp;
				
				if( temp == "VERSION" || temp == "Version" || temp == "version" )
				{
					is_stream >> temp;
					fVersion = atoi( temp.c_str() );
					continue;
				}
			}
			checkNumberOfArguments( -1, narg, i_listfilename, is_line, fVersion, bShortList );
			is_stream >> temp;
			// read run list
			i_sT.fRunOn = atoi( temp.c_str() );
			is_stream >> temp;
			i_sT.fRunOff = atoi( temp.c_str() );
			
			// short list, only read run numbers and target name
			if( bShortList )
			{
				// fill the runlist vector
				i_sT.f2DAcceptanceMode = f2DAcceptanceMode ; // USE2DACCEPTANCE
				fRunList.push_back( i_sT );
				// fill the runlist map
				fMapRunList[i_sT.fRunOn] = fRunList.back();
				++i_nline;
				continue;
			}
			
			// offset in min between on and off run (positive if off run after on run)
			// (now read from VEvndispRunParameter)
			if( fVersion < 6 )
			{
				is_stream >> temp;
				i_sT.fPairOffset = atof( temp.c_str() );
			}
			// cut selector (now in cut file, therefore ignored)
			is_stream >> temp;
			// cut file
			if( fVersion < 7 )
			{
				is_stream >> temp;
				i_sT.fCutFile = temp;
				// source radius (actually (source radius)^2 )
				// (read theta2 cut from cut file)
				if( !bTotalAnalysisOnly )
				{
					i_sT.fSourceRadius = readSourceRadius( i_sT.fCutFile );
				}
				else
				{
					i_sT.fSourceRadius = 0.1;
				}
				if( i_sT.fSourceRadius <= 0. )
				{
					cout << "VAnaSumRunParameter::loadLongFileList: error in run list: " << endl;
					cout << is_line << endl;
					cout << "invalid source radius " << i_sT.fSourceRadius << endl;
					exit( -1 );
				}
			}
			// background model
			is_stream >> temp;
			if( temp == "RE" )
			{
				i_sT.fBackgroundModel = eREFLECTEDREGION;
			}
			else if( temp == "RB" )
			{
				i_sT.fBackgroundModel = eRINGMODEL;
			}
			else if( temp == "OO" || temp == "ONOFF" )
			{
				i_sT.fBackgroundModel = eONOFF;
			}
			else if( temp == "TML" )
			{
				i_sT.fBackgroundModel = eTEMPLATE;
			}
			else
			{
				i_sT.fBackgroundModel = atoi( temp.c_str() );
			}
			checkNumberOfArguments( i_sT.fBackgroundModel, narg, i_listfilename, is_line, fVersion, bShortList );
			// maximum distance for events from camera center
			// (read maximum distance from cut file)
			if( fVersion < 2 )
			{
				is_stream >> temp;
				i_sT.fmaxradius = atof( temp.c_str() );
			}
			else if( fVersion < 7 )
			{
				if( !bTotalAnalysisOnly )
				{
					i_sT.fmaxradius =  readMaximumDistance( i_sT.fCutFile );
				}
				else
				{
					i_sT.fmaxradius = 2.0;
				}
				if( i_sT.fmaxradius < 0. )
				{
					cout << "VAnaSumRunParameter::loadLongFileList: error in run list: " << endl;
					cout << is_line << endl;
					cout << "invalid maximum distance " << i_sT.fmaxradius << endl;
					exit( -1 );
				}
			}
			// file for effective areas
			is_stream >> temp;
			i_sT.fEffectiveAreaFile = temp;
			// cuts are in the effective area files
			if( fVersion >= 7 )
			{
				// check if IRF runparameters are consistent with ANASUM.runparameter file
				checkAnasumParameter( i_sT.fEffectiveAreaFile );
				
				i_sT.fCutFile = temp;
				// source radius (actually (source radius)^2 )
				// (read theta2 cut from cut file)
				if( !bTotalAnalysisOnly )
				{
					readCutParameter( i_sT.fCutFile, i_sT.fSourceRadius, i_sT.fmaxradius );
				}
				else
				{
					i_sT.fSourceRadius = 0.1;
					i_sT.fmaxradius = 2.0;
				}
				if( i_sT.fSourceRadius <= 0. )
				{
					cout << "VAnaSumRunParameter::loadLongFileList: error in run list: " << endl;
					cout << is_line << endl;
					cout << "invalid source radius " << i_sT.fSourceRadius << endl;
					exit( -1 );
				}
			}
			// background model dependend parameters
			//
			//	  if( i_sT.fBackgroundModel == eONOFF )
			//	  {
			// nothing here
			//	  }
			if( i_sT.fBackgroundModel == eRINGMODEL )
			{
				is_stream >> temp;
				i_sT.fRM_RingRadius = atof( temp.c_str() );
				is_stream >> temp;
				i_sT.fRM_RingWidth  = getRingWidth( TMath::Pi() * i_sT.fSourceRadius, i_sT.fRM_RingRadius, atof( temp.c_str() ) );
				is_stream >> temp;
				i_sT.fAcceptanceFile = temp;
			}
			
			else if( i_sT.fBackgroundModel == eREFLECTEDREGION )
			{
				is_stream >> temp;
				i_sT.fRE_distanceSourceOff = atof( temp.c_str() );
				is_stream >> temp;
				i_sT.fRE_nMinoffsource = atoi( temp.c_str() );
				is_stream >> temp;
				i_sT.fRE_nMaxoffsource = atoi( temp.c_str() );
				is_stream >> temp;
				i_sT.fAcceptanceFile = temp;
			}
			
			/////////////////
			
			else if( i_sT.fBackgroundModel == eTEMPLATE )
			{
				is_stream >> temp;
				i_sT.fTE_mscw_min = atof( temp.c_str() );
				is_stream >> temp;
				i_sT.fTE_mscw_max = atof( temp.c_str() );
				is_stream >> temp;
				i_sT.fTE_mscl_min = atof( temp.c_str() );
				is_stream >> temp;
				i_sT.fTE_mscl_max = atof( temp.c_str() );
				cout << "DO NOT USE " << endl;
				exit( 0 );
			}
			else if( i_sT.fBackgroundModel == eONOFF )
			{
				// off runs are weighted the same as on runs
				i_sT.fOO_alpha = 1.;
			}
			// fill the runlist vector
			i_sT.f2DAcceptanceMode = f2DAcceptanceMode ; // USE2DACCEPTANCE
			fRunList.push_back( i_sT );
			// fill the runlist map
			fMapRunList[i_sT.fRunOn] = fRunList.back();
			++i_nline;
		}
	}
	
	return i_nline;
}


void VAnaSumRunParameter::printStereoParameter( int ion )
{
	for( unsigned int i = 0; i < fRunList.size(); i++ )
	{
		if( fRunList[i].fRunOn == ion )
		{
			printStereoParameter( i );
		}
	}
}


void VAnaSumRunParameter::printStereoParameter( unsigned int i )
{
	if( i < fRunList.size() )
	{
		int ioff = fRunList[i].fRunOff;
		
		cout << "Stereo analysis for run: " << fRunList[i].fRunOn << "\t" << ioff;
		cout << "  (run " << i + 1 << " out of " << fRunList.size() << ")";
		if( fRunList[i].fEventDisplayVersion.size() > 0 )
		{
			cout << ", eventdisplay version " << fRunList[i].fEventDisplayVersion;
		}
		cout << endl;
		cout << "\t Object: " << fRunList[i].fTarget << endl;
		cout << "\t Wobble: (N" << fRunList[i].fWobbleNorth << ", W" << fRunList[i].fWobbleWest << ")";
		cout << ", sky maps centred at (ra,dec) (" << fSkyMapCentreRAJ2000 << ", " << fSkyMapCentreDecJ2000 << ")";
		cout << ", target shift: (N" << fRunList[i].fTargetShiftNorth << ", W" << fRunList[i].fTargetShiftWest << ")";
		cout << " (RA/DEC)_J2000 [" << fRunList[i].fTargetShiftRAJ2000 << ", " << fRunList[i].fTargetShiftDecJ2000 << "]" <<  endl;
		if( fExclusionRegions.size() > 0 )
		{
			cout << "\t region excluded from background estimation: " << endl;
			for( unsigned int l = 0; l < fExclusionRegions.size(); l++ )
			{
				if( !fExclusionRegions[l] )
				{
					continue;
				}
				cout << "\t       ";
				cout << ( l + 1 ) << ":region to exclude: (N " << fExclusionRegions[l]->fExcludeFromBackground_North;
				cout <<                             ", W " << fExclusionRegions[l]->fExcludeFromBackground_West;
				//cout << ", R " << fExclusionRegions[l]->fExcludeFromBackground_Radius << ", ID " << fExclusionRegions[l]->fExcludeFromBackground_StarID;
				cout << ", R1 " << fExclusionRegions[l]->fExcludeFromBackground_Radius1 << ", R2 " << fExclusionRegions[l]->fExcludeFromBackground_Radius2 << ", theta " << fExclusionRegions[l]->fExcludeFromBackground_RotAngle << ", ID " << fExclusionRegions[l]->fExcludeFromBackground_StarID;
				if( fExclusionRegions[l]->fExcludeFromBackground_StarName.size() > 0 )
				{
					cout << " (" << fExclusionRegions[l]->fExcludeFromBackground_StarName << ")";
				}
				if( l <= fExclusionRegions.size() - 1 )
				{
					cout << " )" << endl;
				};
			}
		}
		cout << "\t number of telescopes: " << fRunList[i].fNTel << endl;
		cout << "\t time intervall for rate plots: " << fTimeIntervall << " s (" << fTimeIntervall / 60. << " min)" << endl;
		cout << "\t effective areas from " << fRunList[i].fEffectiveAreaFile << endl;
		cout << "\t sky plot binning [deg] " << fSkyMapBinSize << "\t" << fSkyMapBinSizeUC << endl;
		cout << "\t sky plot size [deg]: " << fSkyMapSizeXmin << " < X < " << fSkyMapSizeXmax;
		cout << ", " << fSkyMapSizeYmin << " < Y < " << fSkyMapSizeYmax << endl;
		cout << "\t energy spectra parameters (binsize, log10): " << fEnergySpectrumBinSize;
		if( fEffectiveAreaVsEnergyMC == 0 )
		{
			cout << " (use effective area A_MC)";
		}
		else if( fEffectiveAreaVsEnergyMC == 1 )
		{
			cout << " (use effective area A_REC)";
		}
		cout << ", Method " << fEnergyReconstructionMethod << endl;
		cout << "\t dead time calculation method: ";
		if( fDeadTimeCalculationMethod == 0 )
		{
			cout << "scalar method" << endl;
		}
		else
		{
			cout << "tdiff method" << endl;
		}
		
		cout << "\t background model: ";
		if( fRunList[i].fBackgroundModel == eONOFF )
		{
			cout << "ON/OFF BACKGROUND MODEL" << endl;
			cout << "\t theta2 cut: " << fRunList[i].fSourceRadius << " deg2" << endl;
			cout << "\t maximum distance to camera center: " << fRunList[i].fmaxradius << endl;
		}
		else if( fRunList[i].fBackgroundModel == eRINGMODEL )
		{
			cout << "RING BACKROUND MODEL" << endl;
			cout << "\t theta2 cut: " << fRunList[i].fSourceRadius << " deg2" << endl;
			cout << "\t ring radius: " << fRunList[i].fRM_RingRadius << " deg" << endl;
			cout << "\t ring width: " << fRunList[i].fRM_RingWidth << " deg" << endl;
			cout << "\t area ratio source region to ring: " << 2 * fRunList[i].fRM_RingRadius* fRunList[i].fRM_RingWidth / fRunList[i].fSourceRadius << endl;
			cout << "\t acceptance file: " << fRunList[i].fAcceptanceFile << endl;
			cout << "\t maximum distance to camera center: " << fRunList[i].fmaxradius << " deg" << endl;
		}
		else if( fRunList[i].fBackgroundModel == eREFLECTEDREGION )
		{
			cout << "REFLECTED REGIONS BACKGROUND MODEL";
			if( fTMPL_RE_RemoveOffRegionsRandomly )
			{
				cout << " (excess regions are removed randomly)";
			}
			cout << endl;
			cout << "\t theta2 cut: " << fRunList[i].fSourceRadius << " deg2" << endl;
			cout << "\t distance to source region: " << fRunList[i].fRE_distanceSourceOff << endl;
			cout << "\t minimum number of off source regions: " << fRunList[i].fRE_nMinoffsource << endl;
			cout << "\t maximum number of off source regions: " << fRunList[i].fRE_nMaxoffsource << endl;
			cout << "\t acceptance file (for theta2 plots): " << fRunList[i].fAcceptanceFile << endl;
			cout << "\t maximum distance to camera center: " << fRunList[i].fmaxradius << " deg" << endl;
		}
		
		////////////////////////////////
		
		else if( fRunList[i].fBackgroundModel == eTEMPLATE )
		{
			cout << "TEMPLATE BACKGROUND MODEL" << endl;
		}
	}
	cout << "\t NBoxsmooth: " << fRunList[i].fNBoxSmooth << endl;
	cout << endl;
}


int VAnaSumRunParameter::checkNumberOfArguments( string is )
{
	// get rid of trailing spaces
	while( is.rfind( " " ) > is.size() - 2 )
	{
		is = is.substr( 0, is.size() - 1 );
	}
	// Need to remove newline character from the string
	// since it is counted as an additional parameter 
	is.erase(std::remove(is.begin(), is.end(), '\n'), is.end());
	is.erase(std::remove(is.begin(), is.end(), '\r'), is.end());
	istringstream is_stream( is );
	string itemp;
	int z = 0;
	while( !(is_stream>>std::ws).eof() )
	{
		is_stream >> itemp;
		z++;
	}
	return z;
}


void VAnaSumRunParameter::checkNumberOfArguments( int im, int narg, string i_listfilename, string is_line, int iversion, bool bShortList )
{
	if( bShortList && narg > 3 )
	{
		return;
	}
	
	int n_tot = 0;
	if( im == -1 )
	{
		n_tot = 10;
	}
	else if( im == 0 )
	{
		n_tot = 12;
	}
	else if( im == 1 )
	{
		n_tot = 16;
	}
	else if( im == 2 )
	{
		n_tot = 16;
	}
	else if( im == 3 )
	{
		n_tot = 15;
	}
	else
	{
		cout << "VAnaSumRunParameter::checkNumberOfArguments error: unknown background model" << endl;
		cout << "exiting..." << endl;
		exit( -1 );
	}
	
	// wobble offsets removed with version >=3
	if( iversion > 2 )
	{
		n_tot -= 2;
	}
	if( iversion > 3 )
	{
		n_tot -= 2;
	}
	if( iversion > 4 && ( im == 1 || im == 3 ) )
	{
		n_tot -= 1;
	}
	if( iversion > 5 )
	{
		n_tot -= 1;    // no more RA offset for off runs
	}
	if( iversion > 6 )
	{
		n_tot -= 1;    // no more cut file
	}
	
	if( ( im == -1 && narg < n_tot ) || ( im >= 0 && narg != n_tot ) )
	{
		cout << "error: not enough/too many parameter in " << i_listfilename << ": " << endl;
		cout << is_line << endl;
		cout << "expected " << n_tot << " parameter, found " << narg << " parameters" << endl;
		cout << "(" << im << ", " << narg << ", run list version " << iversion;
                if( bShortList ) cout << ", shortlist";
                cout << ")" << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
}


void VAnaSumRunParameter::reset( VAnaSumRunParameterDataClass it )
{
	it.fRunOn = 0;
	it.fRunOff = 0;
	it.fTarget = "target";
	it.fPairOffset = 0.;
	it.fWobbleNorth = 0.;
	it.fWobbleWest = 0.;
	it.fWobbleNorthMod = 0.;
	it.fWobbleWestMod = 0.;
	it.fNTel = 4;
	it.fMaxTelID = it.fNTel;
	it.fTelToAnalyze.clear();
	it.fBackgroundModel = 0;
	it.fSourceRadius = 0.;
	it.fmaxradius = 0.;
	
	it.fAcceptanceFile = "";
	
	it.fNBoxSmooth = 0;
	it.fOO_alpha = 0.;
	
	it.fRM_RingRadius = 0.;
	it.fRM_RingWidth = 0.;
	it.fAcceptanceFile = "";
	
	it.fRE_distanceSourceOff = 0.;
	it.fRE_nMinoffsource = 0;
	it.fRE_nMaxoffsource = 0;
	
	it.fAcceptanceFile = "";
	
	it.fTE_mscw_min = 0.;
	it.fTE_mscw_max = 0.;
	it.fTE_mscl_min = 0.;
	it.fTE_mscl_max = 0.;
}


double VAnaSumRunParameter::getRingWidth( double a_on, double rr, double rat )
{
	if( rr == 0. )
	{
		return 0.;
	}
	
	return rat / 4. / TMath::Pi() / rr * a_on * 2.;
}


double VAnaSumRunParameter::readSourceRadius( string ifile )
{
	VGammaHadronCuts iC;
	iC.setNTel( 1 );  // irrelevant - but suppresses some warnings
	if( !iC.readCuts( ifile, 0 ) )
	{
		return -1;
	};
	
	if( iC.getTheta2Cut_max() < 0. && iC.getDirectionCutSelector() == 2 )
	{
		return iC.getAngularResolutionAbsoluteMaximum();
	}
	return iC.getTheta2Cut_max();
}

/*
 * read some parameters from gamma/hadron cuts (root file)
 *
 */
bool VAnaSumRunParameter::readCutParameter( string ifile, double& iSourceRadius, double& iMaximumDistance )
{
	iSourceRadius = -1.;
	iMaximumDistance = -1.;
	
	string iEffFile = VUtilities::testFileLocation( ifile, "EffectiveAreas", true );
	
	TFile* iF  = new TFile( iEffFile.c_str() );
	if( iF->IsZombie() )
	{
		cout << "VAnaSumRunParameter::readSourceRadius error opening file to read direction cuts: " << endl;
		cout << "\t" << ifile << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	VGammaHadronCuts* iC = ( VGammaHadronCuts* )iF->Get( "GammaHadronCuts" );
	if( !iC )
	{
		cout << "VAnaSumRunParameter::readSourceRadius error reading direction cut from file: " << endl;
		cout << "\t" << ifile << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	if( iC->getTheta2Cut_max() < 0. && iC->getDirectionCutSelector() == 2 )
	{
		iSourceRadius = iC->getAngularResolutionAbsoluteMaximum();
	}
	else
	{
		iSourceRadius = iC->getTheta2Cut_max();
	}
	iMaximumDistance = iC->fCut_CameraFiducialSize_max;
	
	if( iF )
	{
		iF->Close();
	}
	
	return true;
}

/*
 *  Check if requested anasum runparameter make sense, i.e.,
 *  - energy reconstruction method should be the same as in effective area file
 *  - spectral index should be in the range of simulated index range
 *
 */
bool VAnaSumRunParameter::checkAnasumParameter( string ifile )
{
	string iEffFile = VUtilities::testFileLocation( ifile, "EffectiveAreas", true );
	TFile* iF  = new TFile( iEffFile.c_str() );
	if( iF->IsZombie() )
	{
		cout << "VAnaSumRunParameter::checkAnasumParameter error opening file to read IRF parameters: " << endl;
		cout << "\t" << ifile << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	VInstrumentResponseFunctionRunParameter* iIRF = ( VInstrumentResponseFunctionRunParameter* )iF->Get( "makeEffectiveArea_runparameter" );
	if( !iIRF )
	{
		cout << "VAnaSumRunParameter::checkAnasumParameter error reading IRF parameter from file: " << endl;
		cout << "\t" << ifile << endl;
		cout << "exiting..." << endl;
		exit( EXIT_FAILURE );
	}
	else
	{
		// check energy reconstruction method
		if( iIRF->fEnergyReconstructionMethod != fEnergyReconstructionMethod )
		{
			cout << "VAnaSumRunParameter::checkAnasumParameter error in energy reconstruction method specified in runparameter file. " << endl;
			cout << "\t Effective area file (" << ifile << ") uses energy reconstruction method " << iIRF->fEnergyReconstructionMethod << endl;
			cout << "\t but energy reconstruction method " << fEnergyReconstructionMethod << " is requested in the anasum runparameter file. " << endl;
			cout << "exiting..." << endl;
			exit( EXIT_FAILURE );
		}
		// check spectral index range
		double iIndexMin = iIRF->fSpectralIndexMin;
		double iIndexMax = iIRF->fSpectralIndexMin + iIRF->fNSpectralIndex * iIRF->fSpectralIndexStep;
		if( fEnergyReconstructionSpectralIndex < iIndexMin || fEnergyReconstructionSpectralIndex > iIndexMax )
		{
			cout << "VAnaSumRunParameter::checkAnasumParameter warning: spectral index out of range. " << endl;
			cout << "\t Requested spectral index (" << fEnergyReconstructionSpectralIndex << ") is outsided the range simulated ["
				 << iIndexMin << "-" << iIndexMax << "]." << endl << endl;
		}
	}
	
	if( iF )
	{
		iF->Close();
	}
	
	return true;
}



double VAnaSumRunParameter::readMaximumDistance( string ifile )
{
	VGammaHadronCuts iC;
	iC.setNTel( 1 );  // irrelevant - but suppressed printing of warnings to screen
	if( !iC.readCuts( ifile, 0 ) )
	{
		return -1;
	}
	
	return iC.fCut_CameraFiducialSize_max;
}


unsigned int VAnaSumRunParameter::getMaxNumberofTelescopes()
{
	unsigned int iMax = 0;
	for( unsigned int i = 0; i < fRunList.size(); i++ )
	{
		if( fRunList[i].fNTel > iMax )
		{
			iMax = fRunList[i].fNTel;
		}
	}
	return iMax;
}


bool VAnaSumRunParameter::setTargetShifts( unsigned int i )
{
	if( i < fRunList.size() )
	{
		if( fMapRunList.find( fRunList[i].fRunOn ) != fMapRunList.end() )
		{
			fMapRunList[fRunList[i].fRunOn].fTargetShiftWest = fRunList[i].fTargetShiftWest;
			fMapRunList[fRunList[i].fRunOn].fTargetShiftNorth = fRunList[i].fTargetShiftNorth;
			fMapRunList[fRunList[i].fRunOn].fTargetShiftRAJ2000 = fTargetShiftRAJ2000;
			fMapRunList[fRunList[i].fRunOn].fTargetShiftDecJ2000 = fTargetShiftDecJ2000;
		}
		return true;
	}
	return false;
}

bool VAnaSumRunParameter::setSkyMapCentreJ2000( unsigned int i )
{
	if( i < fRunList.size() )
	{
		if( fMapRunList.find( fRunList[i].fRunOn ) != fMapRunList.end() )
		{
			fMapRunList[fRunList[i].fRunOn].fSkyMapCentreRAJ2000  = fRunList[i].fSkyMapCentreRAJ2000;
			fMapRunList[fRunList[i].fRunOn].fSkyMapCentreDecJ2000 = fRunList[i].fSkyMapCentreDecJ2000;
		}
		return true;
	}
	return false;
}

bool VAnaSumRunParameter::setRunTimes( unsigned int i, double iMJDStart, double iMJDStopp )
{
       if( i >= fRunList.size() )
       {
            return false;
       }
       fRunList[i].fMJDOnStart = iMJDStart;
       fRunList[i].fMJDOnStop = iMJDStopp;
       if( fMapRunList.find( fRunList[i].fRunOn ) != fMapRunList.end() )
       {
           fMapRunList[fRunList[i].fRunOn].fMJDOnStart = iMJDStart;
           fMapRunList[fRunList[i].fRunOn].fMJDOnStop = iMJDStopp;
       }
       return true;
}


bool VAnaSumRunParameter::setTargetRADecJ2000( unsigned int i )
{
	if( i < fRunList.size() )
	{
		if( fMapRunList.find( fRunList[i].fRunOn ) != fMapRunList.end() )
		{
			fMapRunList[fRunList[i].fRunOn].fTargetRAJ2000 = fRunList[i].fTargetRAJ2000;
			fMapRunList[fRunList[i].fRunOn].fTargetDecJ2000 = fRunList[i].fTargetDecJ2000;
            fMapRunList[fRunList[i].fRunOn].fTarget = fRunList[i].fTarget;
		}
		// set centre of stereo maps (if this parameter is not set in the file runparameter.dat)
		if( TMath::Abs( fSkyMapCentreNorth ) < 1.e-8 && TMath::Abs( fSkyMapCentreWest ) < 1.e-8
				&& TMath::Abs( fSkyMapCentreRAJ2000 ) < 1.e-8 && TMath::Abs( fSkyMapCentreDecJ2000 ) < 1.e-8 )
		{
			fRunList[i].fSkyMapCentreNorth    = 0.;
			fRunList[i].fSkyMapCentreWest     = 0.;
			fRunList[i].fSkyMapCentreRAJ2000  = fRunList[i].fTargetRAJ2000;
			fRunList[i].fSkyMapCentreDecJ2000 = fRunList[i].fTargetDecJ2000;

			if( fMapRunList.find( fRunList[i].fRunOn ) != fMapRunList.end() )
			{
				fMapRunList[fRunList[i].fRunOn].fSkyMapCentreNorth    = 0.;
				fMapRunList[fRunList[i].fRunOn].fSkyMapCentreWest     = 0.;
				fMapRunList[fRunList[i].fRunOn].fSkyMapCentreRAJ2000  = fRunList[i].fTargetRAJ2000;;
				fMapRunList[fRunList[i].fRunOn].fSkyMapCentreDecJ2000 = fRunList[i].fTargetDecJ2000;
			}
		}
		return true;
	}
	return false;
}

void VAnaSumRunParameter::setArrayPointing( 
        unsigned int i,
        pair< double, double > i_radec,
        pair< double, double > i_radecJ2000 )
{
    if( i >= fRunList.size() ) return;

    fRunList[i].fArrayPointingRA = i_radec.first;
    fRunList[i].fArrayPointingDec = i_radec.second;
    fRunList[i].fArrayPointingRAJ2000 = i_radecJ2000.first;
    fRunList[i].fArrayPointingDecJ2000 = i_radecJ2000.second;
    if( fMapRunList.find( fRunList[i].fRunOn ) != fMapRunList.end() )
    {
       fMapRunList[fRunList[i].fRunOn].fArrayPointingRA = i_radec.first;
       fMapRunList[fRunList[i].fRunOn].fArrayPointingDec = i_radec.second;
       fMapRunList[fRunList[i].fRunOn].fArrayPointingRAJ2000 = i_radecJ2000.first;
       fMapRunList[fRunList[i].fRunOn].fArrayPointingDecJ2000 = i_radecJ2000.second;
    }
}

void VAnaSumRunParameter::setTargetRADec_currentEpoch( unsigned int i, double ra, double dec )
{
	if( i < fRunList.size() )
	{
		fRunList[i].fTargetRA = ra;
		fRunList[i].fTargetDec = dec;
		if( fMapRunList.find( fRunList[i].fRunOn ) != fMapRunList.end() )
		{
			fMapRunList[fRunList[i].fRunOn].fTargetRA = ra;
			fMapRunList[fRunList[i].fRunOn].fTargetDec = dec;
		}
	}
}


void VAnaSumRunParameter::getEventdisplayRunParameter( string fDatadir )
{
	cout << "\t reading run parameter from mscw file (may take a second...)" << endl;
	char i_temp[200];
	int i_run;
	string i_treename = "data";
	string fPrefix = "";
	string fSuffix = ".mscw.root";
	for( unsigned int i = 0; i < fRunList.size(); i++ )
	{
		i_run = fRunList[i].fRunOn;
		
		sprintf( i_temp, "%s/%s%d%s", fDatadir.c_str(), fPrefix.c_str(), i_run, fSuffix.c_str() );
		TFile* i_f = new TFile( i_temp );
		if( i_f->IsZombie() )
		{
			cout << "VAnaSumRunParameter::getEventdisplayRunParameter fatal error: file not found, " << i_temp << endl;
			exit( -1 );
		}
		VEvndispRunParameter* iParV2 = ( VEvndispRunParameter* )i_f->Get( "runparameterV2" );
		if( iParV2 )
		{
			fRunList[i].fEventDisplayVersion = iParV2->getEVNDISP_VERSION();
			fRunList[i].fTarget = iParV2->fTargetName;
			fRunList[i].fTargetRAJ2000 = iParV2->fTargetRA;
			fRunList[i].fTargetDecJ2000 = iParV2->fTargetDec;
			fRunList[i].fWobbleNorth = iParV2->fWobbleNorth;
			fRunList[i].fWobbleWest = -1.*iParV2->fWobbleEast;
			fRunList[i].fWobbleNorthMod = iParV2->fWobbleNorth;
			fRunList[i].fWobbleWestMod = -1.*iParV2->fWobbleEast;
            cout << "Run " << fRunList[i].fRunOn << ":";
            cout << "\t pointing from mscw file is ";
            cout << " (ra,dec (J2000)) = (" << fRunList[i].fTargetRAJ2000 << ", " << fRunList[i].fTargetDecJ2000;
            cout << ")" << endl;
			fRunList[i].fNTel = ( int )iParV2->fTelToAnalyze.size();
			fRunList[i].fTelToAnalyze = iParV2->fTelToAnalyze;
		}
		i_f->Close();
		// get maximum telescope ID
		fRunList[i].fMaxTelID = 0;
		for( unsigned int t = 0; t < fRunList[i].fTelToAnalyze.size(); t++ )
		{
			if( fRunList[i].fTelToAnalyze[t] > fRunList[i].fMaxTelID )
			{
				fRunList[i].fMaxTelID = fRunList[i].fTelToAnalyze[t];
			}
		}
		// go from T1 = 0 to T1 = 1
		fRunList[i].fMaxTelID += 1;
	}
	//////////////////////////////////////////////////////////////////////////////
	// off runs
	// (called only for on/off observation mode (or when run_on != run_off)
	for( unsigned int i = 0; i < fRunList.size(); i++ )
	{
		if( fRunList[i].fRunOn != fRunList[i].fRunOff )
		{
			i_run = fRunList[i].fRunOff;
			
			sprintf( i_temp, "%s%s%d%s", fDatadir.c_str(), fPrefix.c_str(), i_run, fSuffix.c_str() );
			TFile* i_f = new TFile( i_temp );
			if( i_f->IsZombie() )
			{
				cout << "VAnaSumRunParameter::getEventdisplayRunParameter fatal error: file not found, " << i_temp << endl;
				exit( -1 );
			}
			VEvndispRunParameter* iParV2 = ( VEvndispRunParameter* )i_f->Get( "runparameterV2" );
			if( iParV2 )
			{
				fRunList[i].fPairOffset = iParV2->fTargetRAOffset * 24. * 60. / 360.;
				cout << fRunList[i].fPairOffset << endl;
			}
			i_f->Close();
		}
	}
}

/*!
    observe that these are hardwired values according to the VERITAS simulation sets
*/
void VAnaSumRunParameter::setMCZenith()
{
	fMCZe.clear();
	
	fMCZe.push_back( 0.0 );
	fMCZe.push_back( 20.0 );
	fMCZe.push_back( 30.0 );
	fMCZe.push_back( 35.0 );
	fMCZe.push_back( 40.0 );
	fMCZe.push_back( 45.0 );
	fMCZe.push_back( 50.0 );
	fMCZe.push_back( 55.0 );
	fMCZe.push_back( 60.0 );
	fMCZe.push_back( 65.0 );
}


bool VAnaSumRunParameter::writeListOfExcludedSkyRegions()
{
	TTree tEx( "tExcludedRegions", "list of regions excluded from background calculation" );
	float x = 0.;
	float y = 0.;
	//float r = 0.;
	float r1 = 0.;
	float r2 = 0.;
	float theta = 0;
	float decJ2000 = 0.;
	float raJ2000 = 0.;
	int id = 0;
	float iStarBrightness_V = 0;
	float iStarBrightness_B = 0;
	tEx.Branch( "x", &x, "x/F" );
	tEx.Branch( "y", &y, "y/F" );
	//tEx.Branch( "r", &r, "r/F" );
	tEx.Branch( "r1", &r1, "r1/F");
	tEx.Branch( "r2", &r2, "r2/F");
	tEx.Branch( "theta", &theta, "theta/F");
	tEx.Branch( "decj2000", &decJ2000, "decJ2000/F" );
	tEx.Branch( "raj2000", &raJ2000, "raJ2000/F" );
	tEx.Branch( "star_id", &id, "star_id/I" );
	tEx.Branch( "Vmag", &iStarBrightness_V, "Vmag/F" );
	tEx.Branch( "Bmag", &iStarBrightness_B, "Bmag/F" );
	
	for( unsigned int i = 0; i < fExclusionRegions.size(); i++ )
	{
		x = fExclusionRegions[i]->fExcludeFromBackground_West;
		y = fExclusionRegions[i]->fExcludeFromBackground_North;
		//r = fExclusionRegions[i]->fExcludeFromBackground_Radius;
		r1 = fExclusionRegions[i]->fExcludeFromBackground_Radius1;
		r2 = fExclusionRegions[i]->fExcludeFromBackground_Radius2;
		theta = fExclusionRegions[i]->fExcludeFromBackground_RotAngle;
		decJ2000 = fExclusionRegions[i]->fExcludeFromBackground_DecJ2000;
		raJ2000 = fExclusionRegions[i]->fExcludeFromBackground_RAJ2000;
		id = fExclusionRegions[i]->fExcludeFromBackground_StarID;
		iStarBrightness_V = fExclusionRegions[i]->fExcludeFromBackground_StarBrightness_V;
		iStarBrightness_B = fExclusionRegions[i]->fExcludeFromBackground_StarBrightness_B;
		
		tEx.Fill();
	}
	
	tEx.Write();
	
	return true;
}

/*

    read list of exclusion regions from a anasum file

*/
bool VAnaSumRunParameter::getListOfExcludedSkyRegions( TFile* f )
{
	if( !f )
	{
		return false;
	}
	
	TTree* tEx = ( ( TTree* )f->Get( "total_1/stereo/tExcludedRegions" ) );
	if( !tEx )
	{
		return false;
	}
	
	float x = 0.;
	float y = 0.;
	float r1 = 0.;
	float r2 = -99.;
	float theta = 0.;
        bool  bOldStyleExclusionRegions = false;
	float decJ2000 = 0.;
	float raJ2000 = 0.;
	int id = 0;
	float iV = 0.;
	float iB = 0.;
	tEx->SetBranchAddress( "x", &x );
	tEx->SetBranchAddress( "y", &y );
        // keep backwards compatibility to circular exclusion regions
        if( tEx->GetBranchStatus( "r" ) )
        {
	    tEx->SetBranchAddress( "r", &r1 );
            theta = 0.;
            bOldStyleExclusionRegions = true;
        }
        else
        {
            tEx->SetBranchAddress( "r1", &r1 );
            tEx->SetBranchAddress( "r2", &r2 );
            tEx->SetBranchAddress( "theta", &theta );
        }
	tEx->SetBranchAddress( "star_id", &id );
	if( tEx->GetBranch( "decj2000" ) )
	{
		tEx->SetBranchAddress( "decj2000", &decJ2000 );
	}
	if( tEx->GetBranch( "raj2000" ) )
	{
		tEx->SetBranchAddress( "raj2000", &raJ2000 );
	}
	if( tEx->GetBranch( "Vmag" ) )
	{
		tEx->SetBranchAddress( "Vmag", &iV );
	}
	if( tEx->GetBranch( "Bmag" ) )
	{
		tEx->SetBranchAddress( "Bmag", &iB );
	}
	
	for( unsigned int i = 0; i < tEx->GetEntries(); i++ )
	{
		tEx->GetEntry( i );
		fExclusionRegions.push_back( new VAnaSumRunParameterListOfExclusionRegions() );
		fExclusionRegions.back()->fExcludeFromBackground_West = x;
		fExclusionRegions.back()->fExcludeFromBackground_North = y;
		fExclusionRegions.back()->fExcludeFromBackground_Radius1 = r1;
                if( bOldStyleExclusionRegions )
                {
                    fExclusionRegions.back()->fExcludeFromBackground_Radius2 = r1;
                }
                else
                {
                    fExclusionRegions.back()->fExcludeFromBackground_Radius2 = r2;
                }
		fExclusionRegions.back()->fExcludeFromBackground_RotAngle = theta;
		fExclusionRegions.back()->fExcludeFromBackground_StarID = id;
		fExclusionRegions.back()->fExcludeFromBackground_DecJ2000 = decJ2000;
		fExclusionRegions.back()->fExcludeFromBackground_RAJ2000  = raJ2000;
		fExclusionRegions.back()->fExcludeFromBackground_StarBrightness_V = iV;
		fExclusionRegions.back()->fExcludeFromBackground_StarBrightness_B = iB;
	}
	
	delete tEx;
	
	return true;
}

/* 
 * calculate declination from hour string
 *
*/
double VAnaSumRunParameter::getDeclinationFromStrings( string iDec1, string iDec2, string iDec3 )
{
    double d_tt = 0.;
    if( atof( iDec1.c_str() ) < 0. )
    {
         d_tt = atof( iDec1.c_str() ) - atof( iDec2.c_str() ) / 60. - atof( iDec3.c_str() ) / 3600.;
    }
    else
    {
         d_tt = atof( iDec1.c_str() ) + atof( iDec2.c_str() ) / 60. + atof( iDec3.c_str() ) / 3600.;
    }
    return d_tt;
}

void VAnaSumRunParameter::sortRunList()
{
    sort( fRunList.begin(), fRunList.end() );
}


//==================================================================================
// list of exclusion regions
//==================================================================================

VAnaSumRunParameterListOfExclusionRegions::VAnaSumRunParameterListOfExclusionRegions()
{
	fExcludeFromBackground_West  = -9999.;
	fExcludeFromBackground_North = -9999.;
	//fExcludeFromBackground_Radius = 0.;
	fExcludeFromBackground_Radius1 = 0.;
	fExcludeFromBackground_Radius2 = 0.;
	fExcludeFromBackground_RotAngle = 0.;
	fExcludeFromBackground_StarID = -1;
	fExcludeFromBackground_StarName = "";
	fExcludeFromBackground_DecJ2000 = -9999.;
	fExcludeFromBackground_RAJ2000  = -9999.;
	fExcludeFromBackground_StarBrightness_V = -9999.;
	fExcludeFromBackground_StarBrightness_B = -9999.;
}

/* Deconstructor for vtables error */
/*
VAnaSumRunParameter::~VAnaSumRunParameter()
{

}
*/
