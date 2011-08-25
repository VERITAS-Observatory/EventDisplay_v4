//! VMonoPlots
//  Revision $Id: VMonoPlots.h,v 1.4.2.1.4.1.30.1 2010/03/08 07:45:09 gmaier Exp $

#ifndef VMONOPLOTS_H
#define VMONOPLOTS_H

#include "CData.h"

#include "VGammaHadronCuts.h"
#include "VAstroSource.h"
#include "VAnaSumRunParameter.h"
#include "VASlalib.h"
#include "VSkyCoordinatesUtilities.h"
#include "VTargets.h"

#include <iostream>
#include <string>

#include "TChain.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TList.h"

using namespace std;

class VMonoPlots
{
    public:

        VMonoPlots( bool ison, CData *tree, string i_hsuffix, VAnaSumRunParameter *i_para, unsigned int itelid );
        ~VMonoPlots() {}

        double fillHistograms( unsigned int telid, int irun );
        TList *getHisList() { return hisList; }
        TList *getSkyHistograms() { return hListSkyMaps; }
        TList *getParameterHistograms() { return hListParameterHistograms; }
        TH2D  *getSkyMap() { return hmap; }
        void   setEta( double ie ) { f_eta = ie; }
        void   setCuts( VGammaHadronCuts* i_cuts ) { fCuts = i_cuts; }
        void   setR0( double ir ) { f_r0 = ir; }
        void   writeHistograms();

    private:

        bool fIsOn;

        void calcSourceLocation(double i_eta, double & i_xorig, double & i_yorig);
        void initializeRun( int irun );
        void makeCentroidsMap();
        void makeTwoD_BoxSmooth();

//! boolean inline functions to apply the cuts
        bool cutAlpha()
        {
            if(fData->alpha[fTelID] > fCuts->fAlpha_min && fData->alpha[fTelID] < fCuts->fAlpha_max )
                return true; else return false;
        }
        bool cutLength()
        {
            if(fData->length[fTelID] > fCuts->fLength_min && fData->length[fTelID] < fCuts->fLength_max )
                return true; else return false;
        }
        bool cutWidth()
        {
            if(fData->width[fTelID] > fCuts->fWidth_min && fData->width[fTelID] < fCuts->fWidth_max )
                return true; else return false;
        }
        bool cutDistance()
        {
            if(fData->dist[fTelID] > fCuts->fDistance_min && fData->dist[fTelID] < fCuts->fDistance_max )
                return true; else return false;
        }
        bool cutLoS()
        {
            if(fData->los[fTelID] > fCuts->fLos_min && fData->los[fTelID] < fCuts->fLos_max )
                return true; else return false;
        }
        bool cutAsymm()
        {
            if(fData->asym[fTelID] > fCuts->fAsymm_min && fData->asym[fTelID] < fCuts->fAsymm_max )
                return true; else return false;
        }
        bool cutSize()
        {
            if(fData->size[fTelID] > fCuts->fSize_min && fData->size[fTelID] < fCuts->fSize_max )
                return true; else return false;
        }
        bool cutMaxOne() { if(fData->max1[fTelID] > fCuts->fMaxone_min ) return true; else return false; }

        bool  fDebug;

        unsigned int fTelID;                      //!< Telescope 1 = 0

        CData *fData;

        TList *hisList;
        TList *hListParameterHistograms;
        TList *hListSkyMaps;

// parameter histograms
        TH1D* halpha;                             //!< Alpha Histogram
        TH1D* hlength;                            //!< Length Histogram
        TH1D* hwidth;                             //!< Width Histogram
        TH1D* hdist;                              //!< Distance Histogram
        TH1D* hlos;                               //!< Length/Size Histogram
        TH1D* hasymm;                             //!< Asymmetry Histogram
        TH1D* htgrad_x;                           //!< tgradx Histogram
        TH1D* htdist;                             //!< tdist Histogram
        TH1D* hntubes;                            //!< Histogram of number of pixels in image
        TH1D* hlogsize;                           //!< Histogram of log10(size)

// sky maps
        TH2D* hcen_cam;                           //!< Map of image centroids on the camera (i.e. no derotation)
        TH2D* hcen_sky;                           //!< Map of image centroids on the sky (i.e. after derotation)
        TH2D* hmap;                               //!< Map of reconstructed source position

        VAnaSumRunParameter *fRunPar;
        VAstroSource* fAstro;                     //!< Astronomical source parameters for this analysis
        VGammaHadronCuts* fCuts;                          //!< Parameter Cuts

        double f_eta;                             //!< source location parameter
        double f_r0;                              //!< radius of the smoothing box
};
#endif
