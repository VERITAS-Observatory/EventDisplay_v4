// Reading of 'data' tree for mscw_energy output.

#ifndef CData_h
#define CData_h

#include <TChain.h>
#include <TFile.h>

#include "VEmissionHeightCalculator.h"
#include "VGlobalRunParameter.h"
#include "VMeanScaledVariables.h"
#include "VDispAnalyzer.h"
#include "VSimpleStereoReconstructor.h"
#include "VSkyCoordinatesUtilities.h"

#include <bitset>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

class CData
{
    private :
        unsigned long int fTelescopeCombination;
        double fStereoMinAngle;
        vector< double > fTelX;
        vector< double > fTelY;
        vector< double > fTelZ;

        void reconstruct_3tel_images( long unsigned int );
        void reconstruct_3tel_images_direction();
        void reconstruct_3tel_images_energy();
        void reconstruct_3tel_images_scaled_emission_height();
        void reconstruct_3tel_images_scaled_variables();
        void reconstruct_3tel_reset_variables();

    public :

        bool            fMC;

        bool            fShort;
        int             fVersion;
        TTree*          fChain;
        Int_t           fCurrent;

        Int_t           runNumber;
        Int_t           eventNumber;
        Int_t           MJD;
        Double_t        Time;
        Double_t        TelElevation[VDST_MAXTELESCOPES];
        Double_t        TelAzimuth[VDST_MAXTELESCOPES];
        Double_t        TelDec[VDST_MAXTELESCOPES];
        Double_t        TelRA[VDST_MAXTELESCOPES];
        Float_t         ArrayPointing_Elevation;
        Float_t         ArrayPointing_Azimuth;
        UInt_t          Array_PointingStatus;

        Int_t           MCprimary;
        Double_t        MCe0;
        Double_t        MCxcore;
        Double_t        MCycore;
        Double_t        MCxcore_SC;
        Double_t        MCycore_SC;
        Double_t        MCxcos;
        Double_t        MCycos;
        Double_t        MCaz;
        Double_t        MCze;
        Double_t        MCxoff;
        Double_t        MCyoff;

        ULong64_t       LTrig;
        UInt_t          NTrig;
        Int_t           NImages;
        ULong64_t       ImgSel;
        UInt_t          ImgSel_list[VDST_MAXTELESCOPES];
        Int_t           NTtype;
        UInt_t		    NImages_Ttype[VDST_MAXTELESCOPES];
        Double_t        img2_ang;
        Double_t        Ze;
        Double_t        Az;
        Double_t        ra;
        Double_t        dec;
        Double_t        Xoff;
        Double_t        Yoff;
        Double_t        Xoff_derot;
        Double_t        Yoff_derot;
        Double_t        stdS;
        Float_t         theta2;
        Double_t        Xcore;
        Double_t        Ycore;
        Double_t        Xcore_SC;
        Double_t        Ycore_SC;
        Double_t        stdP;
        Double_t        Chi2;
        Float_t         meanPedvar_Image;
        Float_t         meanPedvar_ImageT[VDST_MAXTELESCOPES];
        Float_t         dist[VDST_MAXTELESCOPES];
        Float_t         size[VDST_MAXTELESCOPES];
        Float_t         fraclow[VDST_MAXTELESCOPES];
        Float_t         loss[VDST_MAXTELESCOPES];
        Float_t         max1[VDST_MAXTELESCOPES];
        Float_t         max2[VDST_MAXTELESCOPES];
        Float_t         max3[VDST_MAXTELESCOPES];
        Int_t           maxindex1[VDST_MAXTELESCOPES];
        Int_t           maxindex2[VDST_MAXTELESCOPES];
        Int_t           maxindex3[VDST_MAXTELESCOPES];
        Float_t         width[VDST_MAXTELESCOPES];
        Float_t         length[VDST_MAXTELESCOPES];
        Int_t           ntubes[VDST_MAXTELESCOPES];
        Int_t           ntubesBNI[VDST_MAXTELESCOPES];
        UShort_t        nsat[VDST_MAXTELESCOPES];
        UShort_t        nlowgain[VDST_MAXTELESCOPES];
        Float_t         alpha[VDST_MAXTELESCOPES];
        Float_t         los[VDST_MAXTELESCOPES];
        Float_t         asym[VDST_MAXTELESCOPES];
        Float_t         cen_x[VDST_MAXTELESCOPES];
        Float_t         cen_y[VDST_MAXTELESCOPES];
        Float_t         cosphi[VDST_MAXTELESCOPES];
        Float_t         sinphi[VDST_MAXTELESCOPES];
        Float_t         tgrad_x[VDST_MAXTELESCOPES];
        Float_t         tchisq_x[VDST_MAXTELESCOPES];
        Int_t           Fitstat[VDST_MAXTELESCOPES];
        Float_t         DispXoff_T[VDST_MAXTELESCOPES];
        Float_t         DispYoff_T[VDST_MAXTELESCOPES];
        Float_t         DispWoff_T[VDST_MAXTELESCOPES];
        Float_t         Disp_T[VDST_MAXTELESCOPES];
        Float_t         R_core[VDST_MAXTELESCOPES];
        Float_t         MSCWT[VDST_MAXTELESCOPES];
        Float_t         MSCWTSigma[VDST_MAXTELESCOPES];
        Float_t         MSCLT[VDST_MAXTELESCOPES];
        Float_t         MSCLTSigma[VDST_MAXTELESCOPES];
        Float_t         E[VDST_MAXTELESCOPES];
        Float_t         ES[VDST_MAXTELESCOPES];
        Int_t           NMSCW;
        Float_t         MSCW;
        Float_t         MSCL;
        Float_t         MWR;
        Float_t         MLR;
        Float_t         Erec;
        Float_t         EChi2;
        Float_t         dE;        // Error on Erec
        Float_t         ErecS;
        Float_t         EChi2S;
        Float_t         dES;       // Error on ErecS
        Int_t           NErecT;
        Int_t           ErecQL;
        Double_t        SizeSecondMax;
        Float_t         EmissionHeight;
        Float_t         EmissionHeightChi2;
        UInt_t          NTelPairs;
        Float_t         EmissionHeightT[VDST_MAXTELESCOPES * VDST_MAXTELESCOPES];
        Double_t        DispDiff;  // from disp method
        Float_t         DispAbsSumWeigth;
        Float_t         Xoff_intersect;
        Float_t         Yoff_intersect;

        TBranch*        b_runNumber;              //!
        TBranch*        b_eventNumber;            //!
        TBranch*        b_MJD;                    //!
        TBranch*        b_Time;                   //!
        TBranch*        b_TelElevation;           //!
        TBranch*        b_TelAzimuth;             //!
        TBranch*        b_TelDec;                 //!
        TBranch*        b_TelRA;                  //!
        TBranch*        b_Array_PointingStatus;   //!
        TBranch*        b_ArrayPointing_Elevation;
        TBranch*        b_ArrayPointing_Azimuth;
        // MC parameter
        TBranch*        b_MCprimary;
        TBranch*        b_MCe0;                   //!
        TBranch*        b_MCxcore;                //!
        TBranch*        b_MCycore;                //!
        TBranch*        b_MCxcore_SC;             //!
        TBranch*        b_MCycore_SC;             //!
        TBranch*        b_MCxcos;                 //!
        TBranch*        b_MCycos;                 //!
        TBranch*        b_MCaz;                   //!
        TBranch*        b_MCze;                   //!
        TBranch*        b_MCxoff;                 //!
        TBranch*        b_MCyoff;                 //!

        TBranch*        b_LTrig;                  //!
        TBranch*        b_NTrig;                  //!
        TBranch*        b_NImages;                //!
        TBranch*        b_ImgSel;                 //!
        TBranch*        b_img2_ang;               //!
        TBranch*        b_Ze;                     //!
        TBranch*        b_Az;                     //!
        TBranch*        b_ra;                     //!
        TBranch*        b_dec;                    //!
        TBranch*        b_Xoff;                   //!
        TBranch*        b_Yoff;                   //!
        TBranch*        b_Xoff_derot;             //!
        TBranch*        b_Yoff_derot;             //!
        TBranch*        b_stdS;                   //!
        TBranch*        b_theta2;                 //!
        TBranch*        b_Xcore;                  //!
        TBranch*        b_Ycore;                  //!
        TBranch*        b_Xcore_SC;               //!
        TBranch*        b_Ycore_SC;               //!
        TBranch*        b_stdP;                   //!
        TBranch*        b_Chi2;                   //!
        TBranch*        b_meanPedvar_Image;       //!
        TBranch*        b_meanPedvar_ImageT;      //!
        TBranch*        b_dist;                   //!
        TBranch*        b_size;                   //!
        TBranch*        b_fraclow;                //!
        TBranch*        b_loss;                   //!
        TBranch*        b_max1;                   //!
        TBranch*        b_max2;                   //!
        TBranch*        b_max3;                   //!
        TBranch*        b_maxindex1;              //!
        TBranch*        b_maxindex2;              //!
        TBranch*        b_maxindex3;              //!
        TBranch*        b_width;                  //!
        TBranch*        b_length;                 //!
        TBranch*        b_ntubes;                 //!
        TBranch*        b_ntubesBNI;              //!
        TBranch*        b_nsat;                   //!
        TBranch*        b_nlowgain;               //!
        TBranch*        b_alpha;                  //!
        TBranch*        b_los;                    //!
        TBranch*        b_asym;                   //!
        TBranch*        b_cen_x;                  //!
        TBranch*        b_cen_y;                  //!
        TBranch*        b_cosphi;                 //!
        TBranch*        b_sinphi;                 //!
        TBranch*        b_tgrad_x;                //!
        TBranch*        b_Fitstat;                //!
        TBranch*        b_tchisq_x;               //!
        TBranch*        b_R_core;                 //!
        TBranch*        b_MSCWT;                  //!
        TBranch*        b_MSCWTSigma;             //!
        TBranch*        b_MSCLT;                  //!
        TBranch*        b_MSCLTSigma;             //!
        TBranch*        b_E;                      //!
        TBranch*        b_ES;                     //!
        TBranch*        b_NMSCW;                  //!
        TBranch*        b_MSCW;                   //!
        TBranch*        b_MSCL;                   //!
        TBranch*        b_MWR;                    //!
        TBranch*        b_MLR;                    //!
        TBranch*        b_Erec;                   //!
        TBranch*        b_EChi2;                  //!
        TBranch*        b_ErecS;                  //!
        TBranch*        b_EChi2S;                 //!
        TBranch*        b_ErecQL;                 //!
        TBranch*        b_NErecT;                //!
        TBranch*        b_SizeSecondMax;          //!
        TBranch*        b_EmissionHeight;         //!
        TBranch*        b_EmissionHeightChi2;     //!
        TBranch*        b_NTelPairs;              //!
        TBranch*        b_EmissionHeightT;        //!
        TBranch*        b_DispDiff; //disp
        TBranch*        b_DispAbsSumWeigth;
        TBranch*        b_Xoff_intersect;
        TBranch*        b_Yoff_intersect;

        TTree*          fStereoFriendTree;              //!
        float         Dir_Xoff;                 //!
        float         Dir_Yoff;                 //!
        float         Dir_Erec;                 //!
        TTree*          fGHFriendTree;                 //!
        float         GH_Gamma_Prediction;      //!
        bool          GH_Is_Gamma;               //!
        vector<TFile*> fXGBFiles;                //!

        CData( TTree* tree = 0, bool bMC = false, bool bShort = false, TTree* stereoTree = 0, TTree* ghTree = 0 );
        CData( TTree* tree, bool bMC, bool bShort, string stereo_suffix, string gamma_hadron_suffix );
        virtual ~CData();
        virtual Int_t    GetEntry( Long64_t entry );
        float get_Erec( unsigned int method = 0 );
        float get_Xoff( unsigned int method = 0 );
        float get_Yoff( unsigned int method = 0 );
        TTree* getXGBTree( string suffix, string tree_name );
        pair<float, float> get_XYoff_derot( unsigned int method = 0 );
        virtual Long64_t LoadTree( Long64_t entry );
        float            get_GH_Gamma_Prediction();
        bool             is_GH_Gamma();
        virtual void     Init( TTree* tree );
        virtual Bool_t   Notify();
        bool             isMC()
        {
            return fMC;
        }
        int              getVersion()
        {
            return fVersion;
        }
        void initialize_3tel_reconstruction(
            unsigned long int telescope_combination,
            double stereo_reconstruction_min_angle, vector< double > tel_x, vector< double > tel_y, vector< double > tel_z );
};
#endif
