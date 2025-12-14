//! VStereoAnalysis    class for producing one dimensional histograms from parameterized VERITAS data.

#ifndef VStereoAnalysis_H
#define VStereoAnalysis_H

#include "CData.h"
#include "VGammaHadronCuts.h"
#include "VAnaSumRunParameter.h"
#include "VAstronometry.h"
#include "VEvndispRunParameter.h"
#include "VTimeMask.h"
#include "VDeadTime.h"
#include "VEffectiveAreaCalculator.h"
#include "VStereoHistograms.h"
#include "VStereoMaps.h"
#include "VSkyCoordinates.h"
#include "VSkyCoordinatesUtilities.h"
#include "VPointingDB.h"

#include "TDirectory.h"
#include "TDirectoryFile.h"
#include "TGraphAsymmErrors.h"
#include "TGraph2DErrors.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TList.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TTree.h"
#include "TParameter.h"
#include "TObject.h"
#include "TNamed.h"

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace std;

class VStereoAnalysis
{
    public:

        VStereoAnalysis( bool isOnrun, string i_hsuffix, VAnaSumRunParameter* irunpara,
                         vector< TDirectory* > iRDir, TDirectory* iDir, string iDataDir, int iRandomSeed, bool iTotalAnalysisOnly );
        ~VStereoAnalysis() {}
        double fillHistograms( int icounter, int irun, double AzMin = -1.e3, double AzMax = 1.e3, double iPedVar = -1. );
        TH2D*  getAlpha();
        TH2D*  getAlphaUC();
        TH2D*  getAlphaNorm();
        TH2D*  getAlphaNormUC();
        double getDeadTimeFraction();
        double getEffectiveExposure( int i_run )
        {
            return ( fRunExposure.find( i_run ) != fRunExposure.end() ? fRunExposure[i_run] : 0. );
        }
        TList* getEnergyHistograms();
        TList* getHisList();
        double getMeanAzimuth()
        {
            return fMeanAzimuth;
        }
        double getMeanElevation()
        {
            return fMeanElevation;
        }
        TH1D*  getMeanSignalBackgroundAreaRatio();
        TH1D* getMeanSignalBackgroundAreaRatioUC();
        double getMJD( int i_run )
        {
            return ( fRunMJD.find( i_run ) != fRunMJD.end() ? fRunMJD[i_run] : 0. );
        }
        TList* getParameterHistograms();
        double getRawRate();                      //! return number of entries in rate histograms
        vector< double > getRateCounts();
        vector< double > getRateTime();
        vector< double > getRateTimeIntervall();
        map< int, double > getRunMJD() const
        {
            return fRunMJD;
        }
        map< int, double > getRunDuration() const
        {
            return fRunDuration;
        }
        TList* getSkyHistograms( bool bUC );
        TH2D*  getStereoSkyMap();
        TH2D*  getStereoSkyMapUC();
        TH1D*  getTheta2();
        double getWobbleNorth();
        double getWobbleWest();
        TTree* getTreeWithSelectedEvents()
        {
            return fTreeSelectedEvents;
        }
        void   scaleAlpha( TH2D* halpha_on, bool bUC );
        void   setCuts( VAnaSumRunParameterDataClass iL, int irun );
        void   setNoSkyPlots( bool iS )
        {
            fNoSkyPlots = iS;
        }
        void   setRunExposure( map< int, double > iExpl )
        {
            fRunExposure = iExpl;
        }
        void   setRunMJD( map< int, double > iRunMJD )
        {
            fRunMJD = iRunMJD;
        }
        void   setRunTimes();
        string setRunTimes( CData* iData );
        bool   terminate();
        void   writeDebugHistograms();
        void   writeHistograms( bool bOn );


    private:

        bool fDebug;
        bool bIsGamma;
        bool bTotalAnalysisOnly;

        bool fIsOn;
        bool fNoSkyPlots;                         //! do full sky plots (if false, analysed source region only)

        VAnaSumRunParameter* fRunPara;

        TGraphAsymmErrors* gMeanEffectiveArea;
        TGraph2DErrors*    gTimeBinnedMeanEffectiveArea;
        TGraphErrors*      gMeanEsys_MC;

        TGraphAsymmErrors* gMeanEffectiveAreaMC;
        TH2F*			   hResponseMatrix;

        VStereoMaps* fMap;
        VStereoMaps* fMapUC;
        int fHisCounter;
        map< int, double > fRunMJDStart;
        map< int, double > fRunMJDStopp;
        map< int, double > fRunMJD;               // Default value is mid-point; modified to mean time of accepted events by fillHistograms
        // If fRunMJD is defined from a VRunSummary.fRunMJD it will contain extra ON/OFF runs
        // because the VRunSummary.fRunMJD is a union of all the runs:
        // always access via the run index to be sure of getting the correct run
        map< int, double > fRunDuration;          // Raw run length from data tree
        map< int, double > fRunExposure;          // Open portion of time mask
        vector< VStereoHistograms* > fHisto;
        VStereoHistograms* fHistoTot;

        TTree* fTreeSelectedEvents;
        int    fTreeSelected_runNumber;
        int    fTreeSelected_eventNumber;
        int    fTreeSelected_MJD;
        double fTreeSelected_Time;
        int    fTreeSelected_NImages;
        ULong64_t fTreeSelected_ImgSel;
        double fTreeSelected_theta2;
        double fTreeSelected_Xoff;
        double fTreeSelected_Yoff;
        double fTreeSelected_Xoff_derot;
        double fTreeSelected_Yoff_derot;
        double fTreeSelected_Xcore;
        double fTreeSelected_Ycore;
        float  fTreeSelected_MSCW;
        float  fTreeSelected_MSCL;
        double fTreeSelected_MWR;
        double fTreeSelected_MLR;
        float fTreeSelected_Erec;
        float fTreeSelected_EChi2;
        float fTreeSelected_ErecS;
        float fTreeSelected_EChi2S;
        float fTreeSelected_EmissionHeight;
        float fTreeSelected_EmissionHeightChi2;
        double fTreeSelected_SizeSecondMax;
        UInt_t fTreeSelected_IsGamma;

        TTree* fDL3EventTree;
        int     fDL3EventTree_runNumber;
        int     fDL3EventTree_eventNumber;
        double  fDL3EventTree_Time;
        int     fDL3EventTree_MJD;
        double  fDL3EventTree_Xoff;
        double  fDL3EventTree_Yoff;
        double  fDL3EventTree_Xderot;
        double  fDL3EventTree_Yderot;
        double  fDL3EventTree_RA;
        double  fDL3EventTree_DEC;
        double  fDL3EventTree_Erec;
        double  fDL3EventTree_Erec_Err;
        double  fDL3EventTree_XGroundCore;
        double  fDL3EventTree_YGroundCore;
        int     fDL3EventTree_NImages;
        ULong64_t fDL3EventTree_ImgSel;
        double  fDL3EventTree_MeanPedvar;
        double  fDL3EventTree_MSCW;
        double  fDL3EventTree_MSCL;
        double  fDL3EventTree_Az ;
        double  fDL3EventTree_El ;
        double  fDL3EventTree_EmissionHeight ;
        double  fDL3EventTree_Acceptance ;
        double  fDL3EventTree_MVA;
        UInt_t  fDL3EventTree_IsGamma;
        VRadialAcceptance* fDL3_Acceptance;

        double  fDeadTimeStorage ;
        //double fullMJD ;
        VSkyCoordinates* fVsky ;  // for RADec to AzimElev conversion

        double fTreeSelected_MVA;

        double fTotCount;

        map < int, double > f_t_in_s_min;
        map < int, double > f_t_in_s_max;
        double fMeanAzimuth;
        double fMeanElevation;
        double fNMeanElevation;

        CData* fDataRun;
        TTree* fDataRunTree;
        TFile* fDataFile;
        TDirectory* fDataDirFile;
        TTree* fXGB_tree;
        string fInstrumentEpochMinor;
        vector< unsigned int > fTelToAnalyze;

        vector< VSkyCoordinates* > fAstro;        //!< Astronomical source parameters for this analysis
        VGammaHadronCuts* fCuts;                  //!< Parameter Cuts
        VTimeMask* fTimeMask;                     //!< Time Cuts

        // dead time calculators
        vector< VDeadTime* > fDeadTime;

        // rate counters
        vector< vector< double > > fRateCounts;
        vector< vector< double > > fRateTime;
        vector< vector< double > > fRateTimeIntervall;
        vector< double > fRateCountsTot;
        vector< double > fRateTimeTot;
        vector< double > fRateTimeIntervallTot;

        // directories
        TDirectory* fDirTot;
        vector< TDirectory* > fDirTotRun;

        void astro_check_for_valid_coordinates( unsigned int runlist_iter );
        void astro_set_skymap_center_from_runparameters( unsigned int runlist_iter );
        void astro_set_skymap_centershift_from_runparameters( unsigned int runlist_iter );
        pair< double, double > astro_calculate_ra_dec_currentEpoch( unsigned int runlist_iter );
        pair< double, double > astro_get_wobbleoffset_radec( unsigned int runlist_iter, bool bPrint = true );
        pair< double, double > astro_get_arraypointing( unsigned int runlist_iter, bool bPrint = true );
        pair< double, double > astro_get_arraypointingJ2000( unsigned int runlist_iter );
        double astro_get_mjd( unsigned int runlist_iter );
        void astro_print_pointing( unsigned int runlist_iter );
        void astro_calculate_modified_wobbleoffset( unsigned int runlist_iter );
        void astro_set_exclusionsregions( unsigned int runlist_iter );
        void astro_setup_star_cataloge( unsigned int runlist_iter );

        double combineHistograms();
        void   defineAstroSource();
        bool   closeDataFile();
        CData* getDataFromFile( int i_runNumber );

        void fill_TreeWithSelectedEvents( CData*, double, double, double );
        bool init_TreeWithSelectedEvents( int, bool );
        void reset_TreeWithSelectedEvents();

        void fill_DL3Tree( CData* c,
                           double i_xderot, double i_yderot,
                           unsigned int icounter, double i_UTC );
        bool init_DL3Tree( int irun, int icounter );
        void write_DL3Tree();

        // derotation and J2000
        void getDerotatedCoordinates( unsigned int, double i_UTC, double x, double y, double& x_derot, double& y_derot );

        int  getDataRunNumber() const;            // Check for existence of fDataRun and try to retrieve run number from first entry of the tree
};
#endif
