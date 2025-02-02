// Effective area data tree definition.

#ifndef CEffArea_h
#define CEffArea_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TProfile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TGraphAsymmErrors.h>

class CEffArea : public TObject
{
    public :
        TTree*          fChain;                   //!pointer to the analyzed TTree or TChain
        Int_t           fCurrent;                 //!current Tree number in a TChain

        // Declaration of leaf types
        Double_t        ze;
        Int_t           az;
        Double_t        azMin;
        Double_t        azMax;
        Double_t        Xoff;
        Double_t        Yoff;
        Double_t        Woff;
        Int_t           noise;
        Double_t        pedvar;
        Double_t        index;
        Int_t           nbins;
        Double_t        e0[1000];                 //[nbins]
        Double_t        eff[1000];                //[nbins]
        Double_t        seff_L[1000];             //[nbins]
        Double_t        seff_U[1000];             //[nbins]
        Int_t           Rec_nbins;
        Double_t        Rec_e0[1000];             //[nbins]
        Double_t        Rec_eff[1000];            //[nbins]
        Double_t        Rec_seff_L[1000];         //[nbins]
        Double_t        Rec_seff_U[1000];         //[nbins]
        TH1D*            hEmc;
        TH1D*            hEcut;
        TH1D*            hEcutUW;
        TH1D*            hEcut500;
        TH1D*            hEcutLin;
        TH1D*            hEcutRec;
        TH1D*            hEcutRecUW;
        TGraphAsymmErrors* gEffAreaMC;
        TGraphAsymmErrors* gEffAreaRec;
        TProfile*        hEmcSWeight;
        TProfile*        hEsysRec;
        TProfile*        hEsysMC;
        TProfile*        hEsysMCRelative;
        TH2F*            hEsysMCRelativeRMS;
        TH2F*            hEsysMCRelative2D;
        TH2F*            hEsys2D;
        TH2F*            hEmcCutCTA;
        TH2F*            hResponseMatrix;
        TProfile*        hResponseMatrixProfile;
        TH2F*            hResponseMatrixFineQC;
        TH2F*            hResponseMatrixQC;
        TH1D*            hhEcutTrigger;
        TH1D*            hhEcutFiducialArea;
        TH1D*            hhEcutStereoQuality;
        TH1D*            hhEcutTelType;
        TH1D*            hhEcutDirection;
        TH1D*            hhEcutGammaHadron;
        TH1D*            hhEcutEnergyReconstruction;
        TH1D*            hWeightedRate;

        // List of branches
        TBranch*        b_ze;                     //!
        TBranch*        b_az;                     //!
        TBranch*        b_azMin;                  //!
        TBranch*        b_azMax;                  //!
        TBranch*        b_Xoff;                   //!
        TBranch*        b_Yoff;                   //!
        TBranch*        b_Woff;                   //!
        TBranch*        b_noise;                  //!
        TBranch*        b_pedvar;                 //!
        TBranch*        b_index;                  //!
        TBranch*        b_nbins;                  //!
        TBranch*        b_e0;                     //!
        TBranch*        b_eff;                    //!
        TBranch*        b_seff_L;                 //!
        TBranch*        b_seff_U;                 //!
        TBranch*        b_Rec_nbins;              //!
        TBranch*        b_Rec_e0;                 //!
        TBranch*        b_Rec_eff;                //!
        TBranch*        b_Rec_seff_L;             //!
        TBranch*        b_Rec_seff_U;             //!
        TBranch*        b_hEmc;                   //!
        TBranch*        b_hEcut;                  //!
        TBranch*        b_hEcutUW;                  //!
        TBranch*        b_hEcut500;                  //!
        TBranch*        b_hEcutLin;               //!
        TBranch*        b_hEcutRec;               //!
        TBranch*        b_hEcutRecUW;               //!
        TBranch*        b_gEffAreaMC;             //!
        TBranch*        b_gEffAreaRec;            //!
        TBranch*        b_hEmcSWeight;            //!
        TBranch*        b_hEsysRec;               //!
        TBranch*        b_hEsysMC;                //!
        TBranch*        b_hEsysMCRelative;        //!
        TBranch*        b_hEsysMCRelativeRMS;        //!
        TBranch*        b_hEsysMCRelative2D;        //!
        TBranch*        b_hEsys2D;                //!
        TBranch*        b_hEmcCutCTA;                //!
        TBranch*        b_hResponseMatrix;                //!
        TBranch*        b_hResponseMatrixProfile;                //!
        TBranch*        b_hResponseMatrixQC;                //!
        TBranch*        b_hResponseMatrixFineQC;                //!
        TBranch*        b_hhEcutTrigger;   //!
        TBranch*        b_hhEcutFiducialArea;   //!
        TBranch*        b_hhEcutStereoQuality;   //!
        TBranch*        b_hhEcutTelType;   //!
        TBranch*        b_hhEcutDirection;   //!
        TBranch*        b_hhEcutGammaHadron;   //!
        TBranch*        b_hhEcutEnergyReconstruction;   //!
        TBranch*        b_hWeightedRate; //!

        CEffArea( TTree* tree = 0 );
        virtual ~CEffArea();
        virtual Int_t    GetEntry( Long64_t entry );
        virtual Long64_t LoadTree( Long64_t entry );
        virtual void     Init( TTree* tree );
        ClassDef( CEffArea, 3 );
};
#endif
