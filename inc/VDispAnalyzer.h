//! VDispAnalyzer wrapper class for all modified disp analysis

#ifndef VDispAnalyzer_H
#define VDispAnalyzer_H

#include "TFile.h"
#include "TMath.h"

#include <iostream>
#include <string>
#include <vector>

#include "VDispTableAnalyzer.h"
#include "VGrIsuAnalyzer.h"
#include "VStatistics.h"
#include "VTMVADispAnalyzer.h"

using namespace std;

class VDispAnalyzer
{
    private:

        bool                fDebug;

        bool                bZombie;

        string              fDispMethod;

        VDispTableAnalyzer* fDispTableAnalyzer;
        VTMVADispAnalyzer*  fTMVADispAnalyzer;

        float fAxesAngles_min;
        unsigned int fNImages_min;
        float fdistance_max;
        float floss_max;
        float fFui_min;
        int   fntubes_min;
        float fWidth_min;
        float fFitstat_min;
        bool  fDispErrorWeighting;
        float fDispErrorExponential;

        // disp direction reconstruction
        float f_disp;
        float f_dispE;
        float f_dispDiff;
        float f_xs;
        float f_ys;
        float f_angdiff;
        vector< float > fdisp_xs_T;
        vector< float > fdisp_ys_T;
        vector< float > fdisp_xy_weight_T;
        float fdisp_sum_abs_weigth;
        vector< float > fdisp_T;
        vector< unsigned int > fdisplist_T;

        // disp energy reconstruction
        float fdisp_energy;
        float fdisp_energy_chi;
        float fdisp_energy_dEs;
        vector< float > fdisp_energy_T;
        unsigned int   fdisp_energy_NT;
        int  fdisp_energyQL;

        // disp core reconstruction
        vector< float > fdisp_core_T;

        vector<ULong64_t> fTelescopeTypeList;
        vector<float> fTelescopeFOV;

        unsigned int find_smallest_diff_element(
            vector< vector< float > >& i_sign,
            vector< float >& x, vector< float >& y,
            vector< float >& cosphi, vector< float >& sinphi,
            vector< float >& tel_pointing_dx, vector< float >& tel_pointing_dy,
            vector< float >& v_disp, vector< float >& v_weight );
        vector< vector< float > > get_sign_permutation_vector( unsigned int x_size );

    public:

        VDispAnalyzer();
        ~VDispAnalyzer() {}

        void calculateEnergies( unsigned int i_ntel, float iArrayElevation, float iArrayAzimuth,
                                ULong64_t* iTelType,
                                float* img_size, float* img_cen_x, float* img_cen_y,
                                float* img_cosphi, float* img_sinphi,
                                float* img_width, float* img_length, float* img_asym,
                                float* img_tgrad, float* img_loss, int* img_ntubes,
                                double* img_weight,
                                double xoff_4, double yoff_4,
                                float* iR, float iEHeight,
                                double iMCEnergy,
                                float* img_fui,
                                float* img_pedvar,
                                int* img_fitstat );

        void  calculateMeanDirection( float& xs, float& ys,
                                      vector< float >& x, vector< float >& y,
                                      vector< float >& cosphi, vector< float >& sinphi,
                                      vector< float >& v_disp, vector< float >& v_weight,
                                      vector< float >& tel_pointing_dx,
                                      vector< float >& tel_pointing_dy,
                                      float& dispdiff,
                                      float x_off4, float yoff_4,
                                      bool UseIntersectForHeadTail = false );

        void calculateMeanDispDirection( unsigned int i_ntel, float iArrayElevation, float iArrayAzimuth,
                                         ULong64_t* iTelType,
                                         float* img_size, float* img_cen_x, float* img_cen_y,
                                         float* img_cosphi, float* img_sinphi,
                                         float* img_width, float* img_length, float* img_asym,
                                         float* img_tgrad, float* img_loss, int* img_ntubes,
                                         double* img_weight,
                                         double xoff_4, double yoff_4,
                                         vector< float >& dispErrorT,
                                         vector< float >& dispSignT,
                                         float* img_fui,
                                         float* img_pedvar,
                                         double* pointing_dx, double* pointing_dy,
                                         bool UseIntersectForHeadTail,
                                         int* img_fitstat );

        void calculateMeanEnergy( vector< float >& disp_energy_T, float* img_size, double* img_weight );
        void calculateMeanShowerDirection( vector< float >& v_x, vector< float >& v_y, vector< float >& v_weight,
                                           float& xs, float& ys, float& dispdiff, unsigned int iMaxN );

        vector< float > calculateExpectedDirectionError_or_Sign( unsigned int i_ntel, float iArrayElevation, float iArrayAzimuth,
                ULong64_t* iTelType,
                float* img_size, float* img_cen_x, float* img_cen_y,
                float* img_cosphi, float* img_sinphi,
                float* img_width, float* img_length, float* img_asym,
                float* img_tgrad, float* img_loss, int* img_ntubes,
                double* img_weight,
                double xoff_4, double yoff_4,
                float* img_fui, float* img_pedvar, int* img_fitstat );

        float evaluate( float iWidth, float iLength, float iAsymm, float iDist,
                        float iSize, float iPedvar, float itgrad, float iLoss,
                        float icen_x, float icen_y, float xoff_4, float yoff_4, ULong64_t iTelType,
                        float iZe, float iAz, float iRcore, float iFui, float iNtubes,
                        bool b2D = true );
        float getAngDiff()
        {
            return f_angdiff;
        }
        float getDisp()
        {
            return f_disp;
        }
        float getDispDiff()
        {
            return f_dispDiff;
        }
        float getDispE()
        {
            return f_dispE;
        }
        float getEnergy();
        float getEnergyChi2();
        float getEnergydES();
        int   getEnergyQualityLabel()
        {
            return fdisp_energyQL;
        }
        float getEnergyT( unsigned int iTelescopeNumber );
        unsigned int   getEnergyNT()
        {
            return fdisp_energy_NT;
        }

        float getXcoordinate_disp()
        {
            return f_xs;
        }
        float getXcoordinate_disp( unsigned int i );
        float getYcoordinate_disp()
        {
            return f_ys;
        }
        float getYcoordinate_disp( unsigned int i );
        vector< float >& getXYWeight_disp()
        {
            return fdisp_xy_weight_T;
        }
        vector< float >& get_disp()
        {
            return fdisp_T;
        }
        float get_disp( unsigned int i )
        {
            if( i < fdisp_T.size() )
            {
                return fdisp_T[i];
            }
            return -999.;
        }
        unsigned int get_disp_tel_list( unsigned i )
        {
            if( i < fdisplist_T.size() )
            {
                return fdisplist_T[i];
            }
            return 0;
        }
        float getXYWeight_disp( unsigned int i )
        {
            if( i < fdisp_xy_weight_T.size() )
            {
                return fdisp_xy_weight_T[i];
            }
            return -999.;
        }
        float get_abs_sum_disp_weight()
        {
            return fdisp_sum_abs_weigth;
        }
        bool  initialize( string iFile, string iDispMethod, string iDispType = "BDTDisp" );
        bool  isZombie()
        {
            return bZombie;
        }
        void  setDebug( bool iFDebug = false )
        {
            fDebug = iFDebug;
        }
        void  setDispErrorWeighting( bool iW = false, float iWeight = 5. )
        {
            fDispErrorWeighting = iW;
            fDispErrorExponential = iWeight;
        }
        void  setQualityCuts( unsigned int iNImages_min = 0, float iAxesAngles_min = 0.,
                              float imaxdist = 1.e5, float imaxloss = 1.,
                              float iminfui = 0., float iminwidth = -1., float iminfitstat = -10.,
                              int iminntubes = 0 )
        {
            fAxesAngles_min = iAxesAngles_min;
            fNImages_min    = iNImages_min;
            fdistance_max   = imaxdist;
            floss_max       = imaxloss;
            fFui_min        = iminfui;
            fWidth_min      = iminwidth;
            fFitstat_min    = iminfitstat;
            fntubes_min     = iminntubes;
        }
        void  setTelescopeTypeList( vector<ULong64_t> iTelescopeTypeList );
        void  setTelescopeFOV( vector< float > iTelFOV )
        {
            fTelescopeFOV = iTelFOV;
        }
        void  setZombie( bool iB = true )
        {
            bZombie = iB;
        }
        void  terminate();
};

#endif
