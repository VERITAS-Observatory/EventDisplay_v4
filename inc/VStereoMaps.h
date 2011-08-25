//! VStereoMaps   model the background
//  Revision $Id: VStereoMaps.h,v 1.12.2.1.4.2.16.1.2.2.2.1.8.1.2.3 2010/08/03 20:37:25 amccann Exp $

#ifndef VStereoMaps_H
#define VStereoMaps_H

#include "CData.h"

#include "VRadialAcceptance.h"

#include "TH2D.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TTree.h"

#include <iostream>

using namespace std;

struct sRE_REGIONS
{
    int noff;                                     //!< number of source regions
    vector< double > xoff;                        //!< x-position of center of off source region
    vector< double > yoff;                        //!< y-position of center of off source region
    vector< double > roff;                        //!< radius of off source region
};

class VStereoMaps
{
    private:
        sRunPara fRunList;
        CData *fData;

        double fTargetShiftWest;
        double fTargetShiftNorth;

// regions excluded from sky maps
        vector<double> vXTOEXCLUDE;
        vector<double> vYTOEXCLUDE;
        vector<double> vRTOEXCLUDE;
        vector<double> vXTOEXCLUDE_CameraCoordinates;
        vector<double> vYTOEXCLUDE_CameraCoordinates;

        VRadialAcceptance *fAcceptance;

        bool bUncorrelatedSkyMaps;
        bool fNoSkyPlots;                         //!< do full sky analysis (if false, analyse source region only)

        double fNLoop;                            //!< calculation of bin area in fiducal area with random number method

        TH2D *hmap_stereo;
        TH2D *hmap_alpha;
        TH2D *hmap_ratio;
        int fSourcePositionBinX;
        int fSourcePositionBinY;

        TRandom3 *fRandom;

        int fInitRun;

        double f_r0;                              //!< smoothing radius

        void makeTwoDStereo_BoxSmooth( double, double, double );
        void makeTwoDStereo_BoxSmooth( double, double, double, double );

// theta2 calculation
        unsigned int fTheta2_length;
        vector< double > fTheta2;
        vector< double > fTheta2_weight;
        vector< double > fTheta2_weightREonly;
        vector< double > fTheta2_All;

        void initialize_theta2();

// RING BACKGROUND MODEL
        TFile *fRM_file;

        bool fill_RingBackgroundModel( double, double, double, double, int, bool );
        bool initialize_RingBackgroundModel( bool iIsOn );
        void RM_calculate_norm();
        void RM_getAlpha( bool );

// REFLECTED REGION MODEL:
        vector< vector< sRE_REGIONS > > fRE_off;  //!< off region parameters
        double fRE_roffTemp;                      //!< radius of off source region

        bool fill_ReflectedRegionModel( double, double, int, bool );
        void RE_getAlpha( bool iIsOn );
        bool initialize_ReflectedRegionModel();
        void initialize_ReflectedRegionHistograms();

// histograms related to reflected region model
        TH2D *hRE_NRegions;
        TTree *hRE_regions;

        bool initialize_Histograms();
        TList *hAuxHisList;                       //!< histograms needed for various calculations

// some variables needed for efficient filling
        double f_RE_binXW;
        double f_RE_binYW;
        int f_RE_xstart;
        int f_RE_xstopp;
        int f_RE_ystart;
        int f_RE_ystopp;
        double f_RE_AreaNorm;
        int f_RE_WW;
        int f_RE_WN;

// FOV BACKGROUND MODEL
        bool fill_FOVBackgroundModel( double, double, double, double, int, bool );
        bool initialize_FOVBackgroundModel( bool iIsOn );
        void FOVM_getAlpha( bool );

// TEMPLATE BACKGROUND MODEL

        void TE_fillAlpha( int, double, double, bool );

// etc
        void   cleanup();                         // delete all objects not needed anymore
        bool   defineAcceptance();
                                                  //!< return if event is in on region
        bool   fillOn( double x_sky, double y_sky, double ze, double erec, int irun, bool ishapecuts );
                                                  //!< return if event is in off region
        bool   fillOff( double x_sky, double y_sky, double ze, double erec, int irun, bool ishapecuts );
        double phiInt( double );

    public:
        TH1D *hAux_theta2On;                      //
        TH1D *hAux_theta2Off;
        TH1D *hAux_theta2Ratio;

        VStereoMaps( double, bool, int );
        ~VStereoMaps() {}

        void              calculateTheta2( bool, double, double );
        bool              fill( bool is_on, double x_sky, double y_sky, double ze, double erec, int irun, bool ishapecuts );
        void              finalize( bool iIsOn , double OnOff_Alpha = 1.0);
        VRadialAcceptance*      getAcceptance() { return fAcceptance; }
        unsigned int      getTheta2_length() { return fTheta2_length; }
        vector< double >& getTheta2() { return fTheta2; }
        vector< double >& getTheta2_weigth() { return fTheta2_weight; }
        vector< double >& getTheta2_weigthREonly() { return fTheta2_weightREonly; }
        vector< double >& getTheta2_All() { return fTheta2_All;}
        TList*            getAux_hisList() { return hAuxHisList; }
        void              setData( CData *c ) { fData = c; }
        void              setHistograms( TH2D*, TH2D* );
        void              setNoSkyPlots( bool iS ) { fNoSkyPlots = iS; }
        void              setRunList( sRunPara iL );
        void              setTargetShift( double iW, double iN );
        void              setRegionToExclude( vector<double> iX, vector<double> iY, vector<double> iR );
};
#endif
