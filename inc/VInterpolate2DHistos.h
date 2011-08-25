//! VInterpolate2DHistos interpolate empty bins in 2D histograms
//  Revision $Id: VInterpolate2DHistos.h,v 1.1.2.1.4.1.12.1.6.1.12.1 2010/03/08 08:00:49 gmaier Exp $

#ifndef VINTERPOLATE2DHISTOS_H
#define VINTERPOLATE2DHISTOS_H

#include "TH2F.h"
#include "TMath.h"
#include "TProfile2D.h"
#include "TRandom3.h"

#include <iostream>
#include <string>

using namespace std;

class VInterpolate2DHistos
{
    private:
        TRandom3 *fRandom;

    public:

        VInterpolate2DHistos( int iseed = 0 );
        ~VInterpolate2DHistos() {}

        TH2F* doSimpleInterpolation( TH2F*, string, int, int, bool );
        TH2F* doGaussianInterpolation( TH2F* h, string iname, TH2F *hNevents = 0, int nGausN = 1, double nWidth = 1. );
};
#endif
