//! VAnaSumRunParameter  storage class for run parameter
//  Revision $Id: VAnaSumRunParameter.h,v 1.1.2.3.4.4.12.2.4.2.2.2.2.4.2.1.6.2.2.3.2.1 2010/12/16 10:20:47 gmaier Exp $

#ifndef VANASUMRUNPARAMETER_H
#define VANASUMRUNPARAMETER_H

#include "TBranch.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TMath.h"
#include "TTree.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "VGammaHadronCuts.h"
#include "VEvndispRunParameter.h"
#include "VGlobalRunParameter.h"

using namespace std;

enum e_background { eONOFF, eRINGMODEL, eREFLECTEDREGION, eFOV, eTEMPLATE };

class sRunPara
{
    public:

        string fEventDisplayVersion;

        int fRunOn;
        string fRunOnFileName;
        int fRunOff;
        string fRunOffFileName;

        double fMJDOn;
        double fMJDOff;

        string fTarget;
        double fTargetRAJ2000;
        double fTargetDecJ2000;
        double fTargetRA;                         // [deg], precessed
        double fTargetDec;                        // [deg], precessed
        double fPairOffset;

        double fRaOffset;                         // [deg]
        double fDecOffset;                        // [deg]
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

                                                  //[deg]
        vector< double > fExcludeFromBackground_North;
                                                  //[deg]
        vector< double > fExcludeFromBackground_West;
                                                  //[deg]
        vector< double > fExcludeFromBackground_DecJ2000;
                                                  //[deg]
        vector< double > fExcludeFromBackground_RAJ2000;
                                                  //[deg]
        vector< double > fExcludeFromBackground_Radius;
        vector< int >    fExcludeFromBackground_StarID;

        unsigned int fNTel;                       // number of telescopes
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
        double fRM_RingWidthUC;                   // ring widht for uncorrelated sky maps [deg]
        double fRM_offdist;                       // minimum distance of background events from source region [deg]

// REFLECTED REGION MODEL
        double fRE_distanceSourceOff;             // minimal distance of off source regions in number of background regions from the source region
        int fRE_nMinoffsource;                    // minmum number of off source regions (default 3)
        int fRE_nMaxoffsource;                    // maximum number of off source regions (default 7)

// FOV BACKGROUND MODEL
        double fFOV_SourceRadius;                 //!< source radius [deg] (to be use to exclude the source)
        double fFOV_offdist;                      //!< minimum distance of background events from source region [deg]

// TEMPLATE MODEL
        double fTE_mscw_min;
        double fTE_mscw_max;
        double fTE_mscl_min;
        double fTE_mscl_max;

        sRunPara();
        ~sRunPara() {}
};

class VAnaSumRunParameter : public VGlobalRunParameter
{
    private:

        int fVersion;

        int checkNumberOfArguments( string is );
        void checkNumberOfArguments( int im, int narg, string isf, string isl, int iversion, bool ishortlist );
        double getRingWidth( double sr, double rr, double rat );
        double readMaximumDistance( string );
        double readSourceRadius( string iCutFile );
        int  returnWithError( string iL, string iM, string iC = "" );
        void reset( sRunPara );
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

                                                  //[deg]
        vector< double > fExcludeFromBackground_North;
                                                  //[deg]
        vector< double > fExcludeFromBackground_West;
                                                  //[deg]
        vector< double > fExcludeFromBackground_DecJ2000;
                                                  //[deg]
        vector< double > fExcludeFromBackground_RAJ2000;
                                                  //[deg]
        vector< double > fExcludeFromBackground_Radius;
        vector< int >    fExcludeFromBackground_StarID;

// energy reconstruction
        double fEnergyReconstructionSpectralIndex;
        unsigned int fEnergyReconstructionMethod;
        double fEnergySpectrumBinSize;
        double fEnergyFitMin;                     // in log10 [TeV]
        double fEnergyFitMax;                     // in log10 [TeV]
        int    fEffectiveAreaVsEnergyMC;
        int fEnergyEffectiveAreaSmoothingIterations;
        double fEnergyEffectiveAreaSmoothingThreshold;
        vector< double > fMCZe;                   // zenith angle intervall for Monte Carlo
	int fFrogs;

// default pedestal variations
        double fDefaultPedVar;

// vector with all run parameters
        vector< sRunPara > fRunList;
// map with all run parameters (sorted after onrun)
        map< int, sRunPara > fMapRunList;

        string fStarCatalogue;
        double fStarMinBrightness;
        string fStarBand;
        double fStarExlusionRadius;

        string fTimeMaskFile;

        VAnaSumRunParameter();
        ~VAnaSumRunParameter() {}
        unsigned int getMaxNumberofTelescopes();
        int getInputFileVersionNumber() { return fVersion; }
        void getEventdisplayRunParameter( string );
        void getWobbleOffsets( string );
        int getRunListVersion() { return fVersion; }
        int loadFileList(string i_listfilename, bool bShortList = false, bool bTotalAnalysisOnly = false );
        void printStereoParameter( unsigned int icounter );
        void printStereoParameter( int irun );
        int  readRunParameter( string i_filename );
        bool setTargetRADecJ2000( unsigned int i, double ra, double dec );
        bool setTargetRADec( unsigned int i, double ra, double dec );
        bool setTargetShifts( unsigned int i, double west, double north, double ra, double dec );
        bool writeListOfExcludedSkyRegions();

};
#endif
