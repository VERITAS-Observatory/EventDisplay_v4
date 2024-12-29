// Showerpars tree definition.

#ifndef Cshowerpars_h
#define Cshowerpars_h

#include "VGlobalRunParameter.h"

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

class Cshowerpars
{
    public :
        bool            bMC;
        bool            bDeRot;
        bool            bShort;

        TTree*          fChain;                   //!pointer to the analyzed TTree or TChain
        Int_t           fCurrent;                 //!current Tree number in a TChain

        // Declaration of leave types
        Int_t           runNumber;
        Int_t           eventNumber;
        UInt_t          eventStatus;
        Int_t           MJD;
        Double_t        Time;
        Int_t           dataFormat;
        UInt_t          NTel;
        Int_t           traceFit;
        //[NTel]
        Float_t         TelElevation[VDST_MAXTELESCOPES];
        //[NTel]
        Float_t         TelAzimuth[VDST_MAXTELESCOPES];
        //[NTel]
        Float_t         TelElevationVBF[VDST_MAXTELESCOPES];
        //[NTel]
        Float_t         TelAzimuthVBF[VDST_MAXTELESCOPES];
        //[NTel]
        Float_t         TelPointingMismatch[VDST_MAXTELESCOPES];
        //[NTel]
        Float_t         TelDec[VDST_MAXTELESCOPES];
        Float_t         TelRA[VDST_MAXTELESCOPES];//[NTel]
        //[NTel]
        Float_t         PointingErrorX[VDST_MAXTELESCOPES];
        //[NTel]
        Float_t         PointingErrorY[VDST_MAXTELESCOPES];
        Float_t         Tel_x_SC[VDST_MAXTELESCOPES];
        Float_t         Tel_y_SC[VDST_MAXTELESCOPES];
        Float_t         Tel_z_SC[VDST_MAXTELESCOPES];
        Float_t         TargetElev;
        Float_t         TargetAzim;
        Float_t         TargetDec;
        Float_t         TargetRA;
        Float_t         WobbleN;
        Float_t         WobbleE;
        UInt_t          NTrig;
        ULong64_t       LTrig;
        //[NTrig]
        UShort_t        Trig_list[VDST_MAXTELESCOPES];
        UShort_t        Trig_type[VDST_MAXTELESCOPES];
        UInt_t          NMethods;
        //[NMethods]
        UShort_t        MethodID[VDST_MAXRECMETHODS];
        //[NMethods]
        UShort_t        NImages[VDST_MAXRECMETHODS];
        //[NMethods]
        ULong64_t       ImgSel[VDST_MAXRECMETHODS];
        //[NMethods]
        UChar_t         ImgSel_list[VDST_MAXRECMETHODS][VDST_MAXTELESCOPES];
        //[NMethods]
        Float_t         img2_ang[VDST_MAXRECMETHODS];
        Float_t         Ze[VDST_MAXRECMETHODS];   //[NMethods]
        Float_t         Az[VDST_MAXRECMETHODS];   //[NMethods]
        Float_t         Xoff[VDST_MAXRECMETHODS]; //[NMethods]
        Float_t         Yoff[VDST_MAXRECMETHODS]; //[NMethods]
        //[NMethods]
        Float_t         XoffDeRot[VDST_MAXRECMETHODS];
        //[NMethods]
        Float_t         YoffDeRot[VDST_MAXRECMETHODS];
        Float_t         stds[VDST_MAXRECMETHODS]; //[NMethods]
        Float_t         dec[VDST_MAXRECMETHODS];  //[NMethods]
        Float_t         ra[VDST_MAXRECMETHODS];   //[NMethods]
        Float_t         Xcore[VDST_MAXRECMETHODS];//[NMethods]
        Float_t         Ycore[VDST_MAXRECMETHODS];//[NMethods]
        //[NMethods]
        Float_t         Xcore_SC[VDST_MAXRECMETHODS];
        //[NMethods]
        Float_t         Ycore_SC[VDST_MAXRECMETHODS];
        Float_t         stdp[VDST_MAXRECMETHODS]; //[NMethods]
        Float_t         Chi2[VDST_MAXRECMETHODS]; //[NMethods]
        Float_t         DispDiff[VDST_MAXRECMETHODS];
        Int_t           MCprim;
        Float_t        MCe0;
        Float_t        MCxcore;
        Float_t        MCycore;
        Float_t        MCxcos;
        Float_t        MCycos;
        Float_t        MCze;
        Float_t        MCaz;
        Float_t        MCxoff;
        Float_t        MCyoff;
        Float_t        MCxcore_SC;
        Float_t        MCycore_SC;
        Float_t        MCzcore_SC;

        // List of branches
        TBranch*        b_runNumber;              //!
        TBranch*        b_eventNumber;            //!
        TBranch*        b_eventStatus;            //!
        TBranch*        b_MJD;                    //!
        TBranch*        b_Time;                   //!
        TBranch*        b_dataFormat;             //!
        TBranch*        b_NTel;                   //!
        TBranch*        b_traceFit;               //!
        TBranch*        b_TelElevation;           //!
        TBranch*        b_TelAzimuth;             //!
        TBranch*        b_TelElevationVBF;        //!
        TBranch*        b_TelAzimuthVBF;          //!
        TBranch*        b_TelPointingMismatch;    //!
        TBranch*        b_TelDec;                 //!
        TBranch*        b_TelRA;                  //!
        TBranch*        b_Tel_x_SC;               //!
        TBranch*        b_Tel_y_SC;               //!
        TBranch*        b_Tel_z_SC;               //!
        TBranch*        b_TargetElev;             //!
        TBranch*        b_TargetAzim;             //!
        TBranch*        b_TargetDec;              //!
        TBranch*        b_TargetRA;               //!
        TBranch*        b_WobbleN;                //!
        TBranch*        b_WobbleE;                //!
        TBranch*        b_NTrig;                  //!
        TBranch*        b_LTrig;                  //!
        TBranch*        b_Trig_list;              //!
        TBranch*        b_Trig_type;              //!
        TBranch*        b_NMethods;               //!
        TBranch*        b_MethodID;               //!
        TBranch*        b_NImages;                //!
        TBranch*        b_ImgSel;                 //!
        TBranch*        b_ImgSel_list;            //!
        TBranch*        b_img2_ang;               //!
        TBranch*        b_Ze;                     //!
        TBranch*        b_Az;                     //!
        TBranch*        b_Xoff;                   //!
        TBranch*        b_Yoff;                   //!
        TBranch*        b_XoffDeRot;                   //!
        TBranch*        b_YoffDeRot;                   //!
        TBranch*        b_stds;                   //!
        TBranch*        b_dec;                    //!
        TBranch*        b_ra;                     //!
        TBranch*        b_Xcore;                  //!
        TBranch*        b_Ycore;                  //!
        TBranch*        b_Xcore_SC;               //!
        TBranch*        b_Ycore_SC;               //!
        TBranch*        b_stdp;                   //!
        TBranch*        b_Chi2;                   //!
        TBranch*        b_DispDiff;               //!
        TBranch*        b_MCprim;                 //!
        TBranch*        b_MCe0;                   //!
        TBranch*        b_MCxcore;                //!
        TBranch*        b_MCycore;                //!
        TBranch*        b_MCxcos;                 //!
        TBranch*        b_MCycos;                 //!
        TBranch*        b_MCze;                   //!
        TBranch*        b_MCaz;                   //!
        TBranch*        b_MCxoff;                 //!
        TBranch*        b_MCyoff;                 //!
        TBranch*        b_MCxcore_SC;             //!
        TBranch*        b_MCycore_SC;             //!
        TBranch*        b_MCzcore_SC;             //!

        Cshowerpars( TTree* tree = 0, bool iMC = false, bool iShort = false );
        virtual ~Cshowerpars();
        virtual Int_t    GetEntry( Long64_t entry );
        virtual Long64_t LoadTree( Long64_t entry );
        virtual void     Init( TTree* tree );
        virtual Bool_t   Notify();
        bool             isMC()
        {
            return bMC;
        }
        bool             isShort()
        {
            return bShort;
        }
};
#endif
