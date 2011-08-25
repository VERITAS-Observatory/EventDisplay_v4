//! VStereoHistograms holds all stereo histograms
//  Revision $Id: VStereoHistograms.h,v 1.19.2.1.18.1.6.1.2.1.10.1 2010/03/08 07:45:10 gmaier Exp $

#ifndef VSTEREOHISTOGRAMS_H
#define VSTEREOHISTOGRAMS_H

#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TKey.h"
#include "TList.h"
#include "TTree.h"

#include <iostream>
#include <map>
#include <string>

#include "VUtilities.h"

using namespace std;

class VStereoHistograms
{
    private:

        bool bIsOn;
        string fHisSuffix;
        double fBinSize;
        double fBinSizeUC;
        double fBinSizeEnergy;
        double fSkyMapSizeXmin;
        double fSkyMapSizeXmax;
        double fSkyMapSizeYmin;
        double fSkyMapSizeYmax;

        void defineHistograms( int i_count, int irun, string i_hsuffix, double ibinsize, double ibinsizeUC, double iEnergyBinSize, bool ion );
        bool readHistograms( TList*, string );

    public:

        TList *hisList;                           //!< list with all histograms
        TList *hListStereoParameterHistograms;    //! list with stereo parameters
                                                  //! list with random forest parameters
        TList *hListRandomForestParameterHistograms;
        TList *hListParameterHistograms;          //!< list with parameter histograms
        map< string, TH1* > hListNameofParameterHistograms;
        TList *hListEnergyHistograms;             //!< list with energy histograms
        TList *hListSkyMaps;                      //!< list with sky maps
        vector< string > hListNameofSkyMaps;      //!< list with histogram names of sky maps
        TList *hListSkyMapsUC;                    //!< list with sky maps (uncorrelated bins)

// data quality monitoring
        TH1D *hTriggerPatternBeforeCuts;
        TH1D *hTriggerPatternAfterCuts;
        TH1D *hImagePatternBeforeCuts;
        TH1D *hImagePatternAfterCuts;

// parameter histograms
        TH1D* htheta2;                            //!< Theta2 Histogram
        TH1D* hmean_width;                        //!< Mean Width Histogram
        TH1D* hmean_length;                       //!< Mean Length Histogram
        TH1D* hmean_dist;                         //!< Mean Distance Histogram
        TH2D* hcore;                              //!< Shower Core map (ground plane)
        TH1D* hmscw;                              //!< MSCW histogram
        TH1D* hmscl;                              //!< MSCL histogram
        TH2D* hmsc;                               //!< MSCW vs MSCL histogram
        TH1D* hZetaTau;                           //!< Theta2 plots toward Zeta Tau
        TH1D* hemiss;                             //!< mean emission height
        TH1D* hemissC2;                           //!< mean emission height Chi2
        TH1D* herecChi2;                          //!< chi2 from energy reconstruction

// random forest histograms
        TH1D* hrf;                                //!< random forest classifier

// energy histograms (logarithmic energy axis)
        TH1D* herecCounts;                        //!< reconstructed energy
        TH1D* herecRaw;                           //!< reconstructed differential energy spectrum
        TH1D* herec;                              //!< reconstructed differential energy spectrum, weighted by effective area
        TH2D* herecWeights;                       //!< weights vs.  reconstructed energy
        TProfile *herecEffectiveArea;             //!< effective area vs reconstructed energy
// energy histograms (linear energy axis)
        TH1D* hLinerecCounts;                     //!< reconstructed energy
        TH1D* hLinerecRaw;                        //!< reconstructed differential energy spectrum
        TH1D* hLinerec;                           //!< reconstructed differential energy spectrum, weighted by effective area
        TH2D* hLinerecWeights;                    //!< weights vs.  reconstructed energy
        TProfile *hLinerecEffectiveArea;          //!< effective area vs reconstructed energy

// sky maps (correlated)
        TH2D* hmap_stereoUC;                      //!< Sky map (correlated bins)
        TH2D* hmap_alphaUC;                       //!< Background normalisation map (correlated bins)
        TH2D* hmap_alpha_offUC;                   //!< Background normalisation map, off map for on run (correlated bins)
        TH2D* hmap_alphaNormUC;                   //!< Background normalisation map  (correlated bins)

// sky maps (uncorrelated)
        TH2D* hxyoff_stereo;                      //!< xyoff map on camera
        TH2D* hmap_stereo;                        //!< Sky map (correlated bins)
        TH2D* hmap_alpha;                         //!< Background normalisation map (correlated bins)
        TH2D* hmap_alpha_off;                     //!< Background normalisation map, off map for on run (correlated bins)
        TH2D* hmap_alphaNorm;                     //!< Background normalisation map  (correlated bins)

// rate lists
        TList *hisRateList;
        TH1D* hrate_1sec;                         //!< Event Rate Histogram (1 second bins)
        TH1D* hrate_10sec;                        //!< Event Rate Histogram (10 second bins)
        TH1D* hrate_1min;                         //!< Event Rate Histogram (1 minute bins)

        VStereoHistograms( string i_hsuffix, double ibinsize, double ibinsizeUC, double iEnergyBinSize, bool ion );
        void defineHistograms();
        void deleteParameterHistograms();
        void deleteSkyPlots();
        void makeRateHistograms( double, double );
        bool readParameterHistograms();
        bool readSkyPlots();
        void scaleDistributions( double );
        void setAlphaOff( TH2D *ioff, bool iuc );
        void setSkyMapSize( double xmin, double xmax, double ymin, double ymax );
        void writeObjects( string, string, TObject* );
        void writeHistograms();

};
#endif
