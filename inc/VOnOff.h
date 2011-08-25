//! VOnOff
//  Revision $Id: VOnOff.h,v 1.16.8.2.28.1.2.2 2010/05/27 08:44:52 gmaier Exp $

#ifndef VONOFF_H
#define VONOFF_H

#include "TDirectory.h"
#include "TH1D.h"
#include "TH1I.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TList.h"
#include "TMath.h"
#include <iomanip>
#include <iostream>
#include <map>
#include <string>

#include "VStatistics.h"

using namespace std;

class VOnOff
{
    private:
        bool fDebug;

        TList *hList;
        TList *hQList;
        TList *hSList;
        TList *hPList;

        TList *hListSkyHistograms;
        TList *hListStereoParameterHistograms;
        TList *hListRandomForestHistograms;
        TList *hListEnergyHistograms;

        TH1D  *h1Dsig;
        TH2D  *hmap_stereo_sig;
        TH1D  *hTheta2_diff;

        double fMaxSigma;
        double fMaxSigmaX;
        double fMaxSigmaY;

        void cleanSigHistogram( TH2D*, double );
        void createHistograms( TList*, TList* );
        void setTitles( TH1*, string, string, string );

    public:
        VOnOff();
        ~VOnOff();
        void doOnOff( TList *ion, TList *ioff, double i_norm );
        void doOnOffforParameterHistograms( TList *ipon, TList *ipoff, double i_norm, double i_norm_alpha, bool isCombined );
        void doOnOffforSkyHistograms( TList *ipon, TList *ipoff, double i_norm, TH2D *ialpha, bool isCombined );
        void doQfactors( TList *ion, TList *ioff, double i_norm );
        TH2D* do2DSignificance( TH2D *ion, TH2D *ioff, double i_norm );
        TH2D* do2DSignificance( TH2D *ion, TH2D *ioff, TH2D *halpha );
        TH2D* do2DSignificance( TH2D *ion, TH2D *ioff, TH2D *halpha, string ititle );
        void fill1DSignificanceHistogram( double rmax = 1.e10 );
        TList* getEnergyHistograms();
        double getMaxSigma();
        double getMaxSigmaX();
        double getMaxSigmaY();
        TH1D*  getTheta2() { return hTheta2_diff; }
        void writeHistograms( TH2D*, TH2D* );
        void writeHistograms() { writeHistograms( 0, 0 ); }
        void writeMonoHistograms();
};
#endif
