// Reading of 'telconfig' tree.

#ifndef Ctelconfig_h
#define Ctelconfig_h

#include "VGlobalRunParameter.h"

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TMath.h>

#include <vector>

class Ctelconfig
{
    public :
        TTree*          fChain;                   //!pointer to the analyzed TTree or TChain
        Int_t           fCurrent;                 //!current Tree number in a TChain

        UInt_t          NTel;
        Int_t           TelID;
        ULong64_t       TelType;
        UInt_t          TelID_hyperArray;
        Float_t         TelX;
        Float_t         TelY;
        Float_t         TelZ;
        Int_t           NMirrors;
        Float_t         MirrorArea;
        Float_t         FOV;
        Float_t         FocalLength;
        Float_t         CameraScaleFactor;
        Float_t         CameraCentreOffset;
        Float_t         CameraRotation;
        UInt_t          NPixel;
        UInt_t          NSamples;
        UInt_t          NGains;
        Float_t         HiLoScale;
        Int_t           HiLoThreshold;
        Float_t         HiLoOffset;
        Float_t         XTubeMM[VDST_MAXCHANNELS];//[NPixel]
        Float_t         YTubeMM[VDST_MAXCHANNELS];//[NPixel]
        Float_t         RTubeMM[VDST_MAXCHANNELS];//[NPixel]
        //[NPixel]
        Float_t         XTubeDeg[VDST_MAXCHANNELS];
        //[NPixel]
        Float_t         YTubeDeg[VDST_MAXCHANNELS];
        //[NPixel]
        Float_t         RTubeDeg[VDST_MAXCHANNELS];

        // List of branches
        TBranch*        b_NTel;                   //!
        TBranch*        b_TelID;                  //!
        TBranch*        b_TelType;                //!
        TBranch*        b_TelID_hyperArray;       //!
        TBranch*        b_TelX;                   //!
        TBranch*        b_TelY;                   //!
        TBranch*        b_TelZ;                   //!
        TBranch*        b_NMirrors;   //!
        TBranch*        b_MirrorArea;   //!
        TBranch*        b_FOV;   //!
        TBranch*        b_FocalLength;            //!
        TBranch*        b_CameraScaleFactor;      //!
        TBranch*        b_CameraCentreOffset;     //!
        TBranch*        b_CameraRotation;         //!
        TBranch*        b_NPixel;                 //!
        TBranch*        b_NSamples;               //!
        TBranch*        b_NGains;   //!
        TBranch*        b_HiLoScale;   //!
        TBranch*        b_HiLoThreshold;   //!
        TBranch*        b_HiLoOffset;   //!
        TBranch*        b_XTubeMM;                //!
        TBranch*        b_YTubeMM;                //!
        TBranch*        b_RTubeMM;                //!
        TBranch*        b_XTubeDeg;               //!
        TBranch*        b_YTubeDeg;               //!
        TBranch*        b_RTubeDeg;               //!

        Ctelconfig( TTree* tree = 0 );
        virtual ~Ctelconfig();
        virtual double   getArrayCentreX();
        virtual double   getArrayCentreY();
        virtual double   getArrayMaxSize();
        virtual Int_t    GetEntry( Long64_t entry );
        virtual unsigned int getNTel();
        virtual Long64_t LoadTree( Long64_t entry );
        virtual void     Init( TTree* tree );
        virtual bool     IsZombie();
};
#endif
