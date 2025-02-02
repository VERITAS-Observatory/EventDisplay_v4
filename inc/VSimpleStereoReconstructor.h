//! VSimpleStereoReconstructor a simple direction and core reconstruction analysis

#ifndef VSimpleStereoReconstructor_H
#define VSimpleStereoReconstructor_H

#include <iostream>
#include <vector>

#include "TMath.h"

#include "VGrIsuAnalyzer.h"
#include "VSkyCoordinatesUtilities.h"

using namespace std;

class VSimpleStereoReconstructor : public VGrIsuAnalyzer
{
    private:

        unsigned int fNImages_min;
        float fAxesAngles_min;

        // telescope pointing
        double  fTelElevation;
        double  fTelAzimuth;


        bool fillShowerCore( float ximp, float yimp );
        void reset();

    public:

        // results shower direction
        float fiangdiff;
        float fmean_iangdiff;
        float fShower_Xoffset;
        float fShower_Yoffset;
        float fShower_stdS;
        float fShower_Chi2;
        float fShower_Ze;
        float fShower_Az;
        float fShower_DispDiff;

        // results core position
        float fShower_Xcore;
        float fShower_Ycore;
        float fShower_stdP;


        VSimpleStereoReconstructor();
        ~VSimpleStereoReconstructor() {}

        bool fillShowerDirection( float xoff, float yoff );
        void initialize( unsigned int iNImages_min = 0, float iAxesAngles_min = 0. );
        bool reconstruct_direction( unsigned int i_ntel,
                                    double iTelElevation, double iTelAzimuth,
                                    double* iTelX,
                                    double* iTelY,
                                    double* iTelZ,
                                    float* img_size,
                                    float* img_cen_x,
                                    float* img_cen_y,
                                    float* img_cosphi,
                                    float* img_sinphi,
                                    float* img_width,
                                    float* img_length,
                                    double* img_weight );
        bool reconstruct_core( unsigned int i_ntel,
                               double iTelElevation, double iTelAzimuth,
                               double iShowerDir_xs, double iShowerDir_ys,
                               double* iTelX,
                               double* iTelY,
                               double* iTelZ,
                               float* img_size,
                               float* img_cen_x,
                               float* img_cen_y,
                               float* img_cosphi,
                               float* img_sinphi,
                               float* img_width,
                               float* img_length,
                               double* img_weight );

};

#endif
