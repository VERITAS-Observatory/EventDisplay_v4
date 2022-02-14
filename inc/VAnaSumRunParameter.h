//! VAnaSumRunParameter  storage class for run parameter

#ifndef VANASUMRUNPARAMETER_H
#define VANASUMRUNPARAMETER_H

#include "TBranch.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TMath.h"
#include "TTree.h"
#include "TNamed.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "VGammaHadronCuts.h"
#include "VInstrumentResponseFunctionRunParameter.h"
#include "VEvndispRunParameter.h"
#include "VGlobalRunParameter.h"

using namespace std;

enum e_background { eONOFF, eRINGMODEL, eREFLECTEDREGION, eTEMPLATE };

class VAnaSumRunParameterListOfExclusionRegions
{
	public:
	
		double fExcludeFromBackground_North;    //[deg]
		double fExcludeFromBackground_West;     //[deg]
		double fExcludeFromBackground_DecJ2000; //[deg]
		double fExcludeFromBackground_RAJ2000;  //[deg]
		double fExcludeFromBackground_Radius1;   //[deg]
		double fExcludeFromBackground_Radius2;   //[deg]
		double fExcludeFromBackground_RotAngle;  //[deg]
		int    fExcludeFromBackground_StarID;
		string fExcludeFromBackground_StarName;
		double fExcludeFromBackground_StarBrightness_V;
		double fExcludeFromBackground_StarBrightness_B;
		
		VAnaSumRunParameterListOfExclusionRegions();
		~VAnaSumRunParameterListOfExclusionRegions() {}
};


class VAnaSumRunParameterDataClass : public TNamed
{

	public:
	
		string fEventDisplayVersion;
		
		int fRunOn;
		string fRunOnFileName;
		int fRunOff;
		string fRunOffFileName;
		
		double fMJDOn;
		double fMJDOff;

                double fMJDOnStart;
                double fMJDOnStop;
		
		string fTarget;
		double fTargetRAJ2000;
		double fTargetDecJ2000;
		double fTargetRA;                         // [deg], precessed
		double fTargetDec;                        // [deg], precessed
		double fPairOffset;
		
		double fWobbleNorth;                      // [deg]
		double fWobbleWest;                       // [deg]
		double fWobbleNorthMod;                   // [deg] (modified model: shifted by fSkMapCentreNorth)
		double fWobbleWestMod;                    // [deg] (modified model: shifted by fSkMapCentreWest)
		
		double fSkyMapCentreNorth;                // [deg]
		double fSkyMapCentreWest;                 // [deg]
		double fSkyMapCentreRAJ2000;              // [deg]
		double fSkyMapCentreDecJ2000;             // [deg]
		
		double fTargetShiftNorth;                 // [deg]
		double fTargetShiftWest;                  // [deg]
		double fTargetShiftRAJ2000;               // [deg]
		double fTargetShiftDecJ2000;              // [deg]
		
		vector< VAnaSumRunParameterListOfExclusionRegions* > fExclusionRegions;   // list of exclusion regions
		
		unsigned int fNTel;                       // number of telescopes
		string   fTelToAna;
		unsigned int fMaxTelID;
		vector< unsigned int > fTelToAnalyze;
		
		int fBackgroundModel;
		double fSourceRadius;                     // actually radius^2
		double fmaxradius;                        // maximum accepted distance from camera center [deg]
		
		string fCutFile;
		
		string fAcceptanceFile;                   // file with acceptance corrections
		
		string fEffectiveAreaFile;                // file with effective areas, use NOFILE if not avaible
		
		// all models
		unsigned int fNBoxSmooth;
		
		// ON/OFF MODEL
		double fOO_alpha;
		
		// RING BACKGROUND MODEL
		double fRM_RingRadius;                    // ring radius [deg]
		double fRM_RingWidth;                     // ring width [deg]
		
		// REFLECTED REGION MODEL
		double fRE_distanceSourceOff;             // minimal distance of off source regions in number of background regions from the source region
		int fRE_nMinoffsource;                    // minmum number of off source regions (default 3)
		int fRE_nMaxoffsource;                    // maximum number of off source regions (default 7)
		
		// TEMPLATE MODEL
		double fTE_mscw_min;
		double fTE_mscw_max;
		double fTE_mscl_min;
		double fTE_mscl_max;
		
		int f2DAcceptanceMode ; // USE2DACCEPTANCE
		
		VAnaSumRunParameterDataClass();
		~VAnaSumRunParameterDataClass() {}
		ClassDef( VAnaSumRunParameterDataClass, 3 );
};

class VAnaSumRunParameter : public TNamed, public VGlobalRunParameter
{
	private:
	
		int fVersion;
		
		int checkNumberOfArguments( string is );
		void checkNumberOfArguments( int im, int narg, string isf, string isl, int iversion, bool ishortlist );
                double getDeclinationFromStrings( string iDec1, string iDec2, string iDec3 );
		double getRingWidth( double sr, double rr, double rat );
		bool   readCutParameter( string ifile, double& iSourceRadius, double& iMaximumDistance );
		bool   checkAnasumParameter( string ifile );
		double readMaximumDistance( string );
		double readSourceRadius( string iCutFile );
		int  returnWithError( string iL, string iM, string iC = "" );
		void reset( VAnaSumRunParameterDataClass );
		void setMCZenith();
		
	public:
	
		// bin sizes and sky map sizes
		double fTimeIntervall;                    // length of time intervalls in seconds for rate plots and short term histograms
		double fSkyMapBinSize;                    // bin size for sky maps [deg]
		double fSkyMapBinSizeUC;                  // bin size for uncorrelated sky maps [deg]
		double fSkyMapSizeXmin;                   // [deg]
		double fSkyMapSizeXmax;                   // [deg]
		double fSkyMapSizeYmin;                   // [deg]
		double fSkyMapSizeYmax;                   // [deg]
		
		// position relative to which 1D histograms are filled
		double fTargetShiftNorth;
		double fTargetShiftWest;
		double fTargetShiftRAJ2000;               // [deg]
		double fTargetShiftDecJ2000;              // [deg]
		
		double fSkyMapCentreNorth;                // [deg]
		double fSkyMapCentreWest;                 // [deg]
		double fSkyMapCentreRAJ2000;              // [deg]
		double fSkyMapCentreDecJ2000;             // [deg]
		
		
		// energy reconstruction
		double fEnergyReconstructionSpectralIndex;
		unsigned int fEnergyReconstructionMethod;
		double fEnergySpectrumBinSize;
		int    fEffectiveAreaVsEnergyMC;
		int    fEnergyEffectiveAreaSmoothingIterations;
		double fEnergyEffectiveAreaSmoothingThreshold;
		vector< double > fMCZe;                   // zenith angle intervall for Monte Carlo
		
		// dead time calculation method
		int  fDeadTimeCalculationMethod;
		
		int f2DAcceptanceMode ; // USE2DACCEPTANCE
		
		// advanced analysis codes
		bool fModel3D;
		bool fDirectionModel3D;
		
                // add all events to DL3 tree, no gh cuts but add BDT score and IsGamma 
                bool fWriteAllEvents;

		// vector with all run parameters
		vector< VAnaSumRunParameterDataClass > fRunList;
		// map with all run parameters (sorted after onrun)
		map< int, VAnaSumRunParameterDataClass > fMapRunList;
		
		// background model
		int    fTMPL_fBackgroundModel;
		
		// RING BACKGROUND MODEL
		double fTMPL_RM_RingRadius;                  // ring radius [deg]
		double fTMPL_RM_RingWidth;                   // ring width [deg]
		
		// REFLECTED REGION MODEL
		double fTMPL_RE_distanceSourceOff;          // minimal distance of off source regions in number of background regions from the source region
		int    fTMPL_RE_nMinoffsource;              // minmum number of off source regions (default 3)
		int    fTMPL_RE_nMaxoffsource;              // maximum number of off source regions (default 7)
		bool   fTMPL_RE_RemoveOffRegionsRandomly;   // removal of excess off regions
		
		// analysis TMPL file
		string fTMPL_CutFile;
		double fTMPL_SourceRadius;
		double fTMPL_maxradius;
		string fTMPL_AcceptanceFile;
		string fTMPL_EffectiveAreaFile;

		// Likelihood Spectral Analysis
		bool fLikelihoodAnalysis;
		
		vector< VAnaSumRunParameterListOfExclusionRegions* > fExclusionRegions;   // list of exclusion regions
		
		// star exclusion regions
		string fStarCatalogue;
		double fStarMinBrightness;
		string fStarBand;
		double fStarExlusionRadius;
		
		// for saving the deadtime fraction
		double fScalarDeadTimeFrac ;
		
		string fTimeMaskFile;
		
		VAnaSumRunParameter();
		~VAnaSumRunParameter() {} ;
		unsigned int getMaxNumberofTelescopes();
		int  getInputFileVersionNumber()
		{
			return fVersion;
		}
		void getEventdisplayRunParameter( string );
		int  getRunListVersion()
		{
			return fVersion;
		}
		int  loadSimpleFileList( string i_listfilename );
		int  loadLongFileList( string i_listfilename, bool bShortList = false, bool bTotalAnalysisOnly = false );
		int  loadShortFileList( string i_listfilename, string iDataDir, bool bTotalAnalysisOnly = false );
		void printStereoParameter( unsigned int icounter );
		void printStereoParameter( int irun );
		int  readRunParameter( string i_filename );
                bool setRunTimes( unsigned int irun, double iMJDStart, double iMJDStopp );
		bool setSkyMapCentreJ2000( unsigned int i, double ra, double dec );
		bool setTargetRADecJ2000( unsigned int i, double ra, double dec, string iTargetName );
		bool setTargetRADec_currentEpoch( unsigned int i, double ra, double dec );
		bool setTargetShifts( unsigned int i, double west, double north, double ra, double dec );
		bool writeListOfExcludedSkyRegions();
		bool getListOfExcludedSkyRegions( TFile* f );
		
		ClassDef( VAnaSumRunParameter, 14 ) ;
};
#endif
