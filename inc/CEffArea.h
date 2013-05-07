//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Apr 10 17:00:24 2008 by ROOT version 5.18/00
// from TTree fEffArea/effective area values
// found on file: effectiveArea.root
//////////////////////////////////////////////////////////

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
        TTree          *fChain;                   //!pointer to the analyzed TTree or TChain
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
        Double_t        noisePE;
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
        Int_t           Prob_nbins;
        Double_t        Prob_e0[1000];             //[nbins]
        Double_t        Prob_eff[1000];            //[nbins]
        Double_t        Prob_seff_L[1000];         //[nbins]
        Double_t        Prob_seff_U[1000];         //[nbins]
        TH1D            *hEmc;
        TH1D            *hEcut;
        TH1D            *hEcutUW;
        TH1D            *hEcut500;
        TH1D            *hEcutLin;
        TH1D            *hEcutRec;
        TH1D            *hEcutRecUW;
        TGraphAsymmErrors *gEffAreaMC;
        TGraphAsymmErrors *gEffAreaRec;
        TGraphAsymmErrors *gEffAreaProb;
        TGraphAsymmErrors *gRecProb;
        TProfile        *hEmcSWeight;
        TProfile        *hEsysRec;
        TProfile        *hEsysMC;
        TProfile        *hEsysMCRelative;
	TH2D            *hEsysMCRelativeRMS;
        TH2D            *hEsysMCRelative2D;
        TH2D            *hEsys2D;
        TH2D            *hEmcCutCTA;
	TH2D            *hResponseMatrix;
	TH1D            *hhEcutTrigger;
	TH1D            *hhEcutFiducialArea;
	TH1D            *hhEcutStereoQuality;
	TH1D            *hhEcutTelType;
	TH1D            *hhEcutDirection;
	TH1D            *hhEcutGammaHadron;
	TH1D            *hhEcutEnergyReconstruction;

// List of branches
        TBranch        *b_ze;                     //!
        TBranch        *b_az;                     //!
        TBranch        *b_azMin;                  //!
        TBranch        *b_azMax;                  //!
        TBranch        *b_Xoff;                   //!
        TBranch        *b_Yoff;                   //!
        TBranch        *b_Woff;                   //!
        TBranch        *b_noise;                  //!
        TBranch        *b_noisePE;                //!
        TBranch        *b_pedvar;                 //!
        TBranch        *b_index;                  //!
        TBranch        *b_nbins;                  //!
        TBranch        *b_e0;                     //!
        TBranch        *b_eff;                    //!
        TBranch        *b_seff_L;                 //!
        TBranch        *b_seff_U;                 //!
        TBranch        *b_Rec_nbins;              //!
        TBranch        *b_Rec_e0;                 //!
        TBranch        *b_Rec_eff;                //!
        TBranch        *b_Rec_seff_L;             //!
        TBranch        *b_Rec_seff_U;             //!
        TBranch        *b_Prob_nbins;              //!
        TBranch        *b_Prob_e0;                 //!
        TBranch        *b_Prob_eff;                //!
        TBranch        *b_Prob_seff_L;             //!
        TBranch        *b_Prob_seff_U;             //!
        TBranch        *b_hEmc;                   //!
        TBranch        *b_hEcut;                  //!
        TBranch        *b_hEcutUW;                  //!
        TBranch        *b_hEcut500;                  //!
        TBranch        *b_hEcutLin;               //!
        TBranch        *b_hEcutRec;               //!
        TBranch        *b_hEcutRecUW;               //!
        TBranch        *b_gEffAreaMC;             //!
        TBranch        *b_gEffAreaRec;            //!
        TBranch        *b_gEffAreaProb;            //!
        TBranch        *b_gRecProb;               //!
        TBranch        *b_hEmcSWeight;            //!
        TBranch        *b_hEsysRec;               //!
        TBranch        *b_hEsysMC;                //!
        TBranch        *b_hEsysMCRelative;        //!
        TBranch        *b_hEsysMCRelativeRMS;        //!
        TBranch        *b_hEsysMCRelative2D;        //!
        TBranch        *b_hEsys2D;                //!
        TBranch        *b_hEmcCutCTA;                //!
        TBranch        *b_hResponseMatrix;                //!
	TBranch        *b_hhEcutTrigger;   //!
	TBranch        *b_hhEcutFiducialArea;   //!
	TBranch        *b_hhEcutStereoQuality;   //!
	TBranch        *b_hhEcutTelType;   //!
	TBranch        *b_hhEcutDirection;   //!
	TBranch        *b_hhEcutGammaHadron;   //!
	TBranch        *b_hhEcutEnergyReconstruction;   //!

        CEffArea(TTree *tree=0);
        virtual ~CEffArea();
        virtual Int_t    Cut(Long64_t entry);
        virtual Int_t    GetEntry(Long64_t entry);
        virtual Long64_t LoadTree(Long64_t entry);
        virtual void     Init(TTree *tree);
        virtual void     Loop();
        virtual Bool_t   Notify();
        virtual void     Show(Long64_t entry = -1);
        ClassDef(CEffArea,3);
};
#endif

#ifdef CEffArea_cxx

ClassImp(CEffArea)

CEffArea::CEffArea(TTree *tree)
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
    if (tree == 0)
    {
        TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("effectiveArea.root");
        if (!f)
        {
            f = new TFile("effectiveArea.root");
        }
        tree = (TTree*)gDirectory->Get("fEffArea");

    }
    Init(tree);
}


CEffArea::~CEffArea()
{
    if (!fChain) return;
    delete fChain->GetCurrentFile();
}


Int_t CEffArea::GetEntry(Long64_t entry)
{
// Read contents of entry.
    if (!fChain) return 0;
    return fChain->GetEntry(entry);
}


Long64_t CEffArea::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
    if (!fChain) return -5;
    Long64_t centry = fChain->LoadTree(entry);
    if (centry < 0) return centry;
    if (!fChain->InheritsFrom(TChain::Class()))  return centry;
    TChain *chain = (TChain*)fChain;
    if (chain->GetTreeNumber() != fCurrent)
    {
        fCurrent = chain->GetTreeNumber();
        Notify();
    }
    return centry;
}


void CEffArea::Init(TTree *tree)
{
// The Init() function is called when the selector needs to initialize
// a new tree or chain. Typically here the branch addresses and branch
// pointers of the tree will be set.
// It is normally not necessary to make changes to the generated
// code, but the routine can be extended by the user if needed.
// Init() will be called many times when running on PROOF
// (once per file to be processed).

// Set object pointer
    hEmc = 0;
    hEcut = 0;
    hEcutUW = 0;
    hEcut500 = 0;
    hEcutLin = 0;
    hEcutRec = 0;
    hEcutRecUW = 0;
    gEffAreaMC = 0;
    gEffAreaRec = 0;
    gEffAreaProb = 0;
    gRecProb = 0;
    hEmcSWeight = 0;
    hEsysRec = 0;
    hEsysMC = 0;
    hEsysMCRelative = 0;
    hEsysMCRelativeRMS = 0;
    hEsysMCRelative2D = 0;
    hEsys2D = 0;
    hEmcCutCTA = 0;
    hResponseMatrix = 0;
   hhEcutTrigger = 0;
   hhEcutFiducialArea = 0;
   hhEcutStereoQuality = 0;
   hhEcutTelType = 0;
   hhEcutDirection = 0;
   hhEcutGammaHadron = 0;
   hhEcutEnergyReconstruction = 0;
// Set branch addresses and branch pointers
    if (!tree) return;
    fChain = tree;
    fCurrent = -1;
    fChain->SetMakeClass(1);

    fChain->SetBranchAddress("ze", &ze, &b_ze);
    fChain->SetBranchAddress("az", &az, &b_az);
    fChain->SetBranchAddress("azMin", &azMin, &b_azMin);
    fChain->SetBranchAddress("azMax", &azMax, &b_azMax);
    fChain->SetBranchAddress("Xoff", &Xoff, &b_Xoff);
    fChain->SetBranchAddress("Yoff", &Yoff, &b_Yoff);
    fChain->SetBranchAddress("Woff", &Woff, &b_Woff);
    fChain->SetBranchAddress("noise", &noise, &b_noise);
    if( fChain->GetBranchStatus("noisePE") ) fChain->SetBranchAddress("noisePE", &noisePE, &b_noisePE);
    else                                     noisePE = 0.;
    fChain->SetBranchAddress("pedvar", &pedvar, &b_pedvar);
    fChain->SetBranchAddress("index", &index, &b_index);
    fChain->SetBranchAddress("nbins", &nbins, &b_nbins);
    fChain->SetBranchAddress("e0", e0, &b_e0);
    fChain->SetBranchAddress("eff", eff, &b_eff);
    if( fChain->GetBranchStatus( "seff_L" ) )
    {
        fChain->SetBranchAddress("seff_L", seff_L, &b_seff_L);
        fChain->SetBranchAddress("seff_U", seff_U, &b_seff_U);
        fChain->SetBranchAddress("Rec_seff_L", Rec_seff_L, &b_Rec_seff_L);
        fChain->SetBranchAddress("Rec_seff_U", Rec_seff_U, &b_Rec_seff_U);
    }
    else
    {
       for( int i = 0; i < 1000; i++ )
       {
          seff_L[i] = 0.;
	  seff_U[i] = 0.;
	  Rec_seff_L[i] = 0.;
	  Rec_seff_U[i] = 0.;
       }
    }
    fChain->SetBranchAddress("Rec_nbins", &Rec_nbins, &b_Rec_nbins);
    fChain->SetBranchAddress("Rec_e0", Rec_e0, &b_Rec_e0);
    fChain->SetBranchAddress("Rec_eff", Rec_eff, &b_Rec_eff);
    if(  fChain->GetBranchStatus( "Prob_nbins" ) )
    {
       fChain->SetBranchAddress("Prob_nbins", &Prob_nbins, &b_Prob_nbins);
       fChain->SetBranchAddress("Prob_e0", Prob_e0, &b_Prob_e0);
       fChain->SetBranchAddress("Prob_eff", Prob_eff, &b_Prob_eff);
       fChain->SetBranchAddress("Prob_seff_L", Prob_seff_L, &b_Prob_seff_L);
       fChain->SetBranchAddress("Prob_seff_U", Prob_seff_U, &b_Prob_seff_U);
    }
    else
    {
       Prob_nbins = 0;
       for( int i = 0; i < 1000; i++ )
       {
          Prob_e0[i] = 0.;
	  Prob_eff[i] = 0.;
	  Prob_seff_L[i] = 0.;
	  Prob_seff_U[i] = 0.;
       }
    }
    if( fChain->GetBranchStatus( "hEmc" ) ) fChain->SetBranchAddress("hEmc", &hEmc, &b_hEmc);
    else                                    hEmc = 0;
    if( fChain->GetBranchStatus("hEcut") )
    {
        fChain->SetBranchAddress("hEcut", &hEcut, &b_hEcut);
	if( fChain->GetBranchStatus("hEcutUW" ) ) fChain->SetBranchAddress("hEcutUW", &hEcutUW, &b_hEcutUW );
	else hEcutUW = 0;
        fChain->SetBranchAddress("hEcut500", &hEcut500, &b_hEcut500);
        fChain->SetBranchAddress("hEcutRec", &hEcutRec, &b_hEcutRec);
        if( fChain->GetBranchStatus("hEcutRecUW" ) ) fChain->SetBranchAddress("hEcutRecUW", &hEcutRecUW, &b_hEcutRecUW );
	else hEcutRecUW = 0;
        fChain->SetBranchAddress("gEffAreaMC", &gEffAreaMC, &b_gEffAreaMC);
        fChain->SetBranchAddress("gEffAreaRec", &gEffAreaRec, &b_gEffAreaRec);
        fChain->SetBranchAddress("gEffAreaProb", &gEffAreaProb, &b_gEffAreaProb);
        fChain->SetBranchAddress("gRecProb", &gRecProb, &b_gRecProb);
        fChain->SetBranchAddress("hEmcSWeight", &hEmcSWeight, &b_hEmcSWeight);
        fChain->SetBranchAddress("hEsysRec", &hEsysRec, &b_hEsysRec);
        fChain->SetBranchAddress("hEsysMC", &hEsysMC, &b_hEsysMC);
        fChain->SetBranchAddress("hEsys2D", &hEsys2D, &b_hEsys2D);
        fChain->SetBranchAddress("hEmcCutCTA", &hEmcCutCTA, &b_hEmcCutCTA);
        fChain->SetBranchAddress("hResponseMatrix", &hResponseMatrix, &b_hResponseMatrix );
    }
    else
    {
        hEmc = 0;
        hEcut = 0;
        hEcutUW = 0;
        hEcut500 = 0;
        hEcutRec = 0;
        hEcutRecUW = 0;
        gEffAreaMC = 0;
        gEffAreaRec = 0;
        gEffAreaProb = 0;
        gRecProb = 0;
        hEmcSWeight = 0;
        hEsysRec = 0;
        hEsysMC = 0;
        hEsysMCRelative = 0;
        hEsysMCRelativeRMS = 0;
        hEsysMCRelative2D = 0;
        hEsys2D = 0;
        hEmcCutCTA = 0;
	hResponseMatrix = 0;
    }
    if( fChain->GetBranchStatus( "hEcutLin" ) )  fChain->SetBranchAddress("hEcutLin", &hEcutLin, &b_hEcutLin);
    else                                         hEcutLin = 0;
    if( fChain->GetBranchStatus( "hEsysMCRelative" ) ) fChain->SetBranchAddress("hEsysMCRelative", &hEsysMCRelative, &b_hEsysMCRelative);
    else                                               hEsysMCRelative = 0;
    if( fChain->GetBranchStatus( "hEsysMCRelativeRMS" ) ) fChain->SetBranchAddress("hEsysMCRelativeRMS", &hEsysMCRelativeRMS, &b_hEsysMCRelativeRMS);
    else                                               hEsysMCRelativeRMS = 0;
    if( fChain->GetBranchStatus( "hEsysMCRelative2D" ) ) fChain->SetBranchAddress("hEsysMCRelative2D", &hEsysMCRelative2D, &b_hEsysMCRelative2D);
    else                                               hEsysMCRelative2D = 0;
    if( fChain->GetBranchStatus( "hhEcutTrigger" ) )
    {
	fChain->SetBranchAddress("hhEcutTrigger", &hhEcutTrigger, &b_hhEcutTrigger);
	fChain->SetBranchAddress("hhEcutFiducialArea", &hhEcutFiducialArea, &b_hhEcutFiducialArea);
	fChain->SetBranchAddress("hhEcutStereoQuality", &hhEcutStereoQuality, &b_hhEcutStereoQuality);
	fChain->SetBranchAddress("hhEcutTelType", &hhEcutTelType, &b_hhEcutTelType);
	fChain->SetBranchAddress("hhEcutDirection", &hhEcutDirection, &b_hhEcutDirection);
	fChain->SetBranchAddress("hhEcutEnergyReconstruction", &hhEcutEnergyReconstruction, &b_hhEcutEnergyReconstruction);
	fChain->SetBranchAddress("hhEcutGammaHadron", &hhEcutGammaHadron, &b_hhEcutGammaHadron);
    }
    else
    {
       hhEcutTrigger = 0;
       hhEcutFiducialArea = 0;
       hhEcutStereoQuality = 0;
       hhEcutTelType = 0;
       hhEcutDirection = 0;
       hhEcutGammaHadron = 0;
       hhEcutEnergyReconstruction = 0;
    }

    Notify();
}


Bool_t CEffArea::Notify()
{
// The Notify() function is called when a new file is opened. This
// can be either for a new TTree in a TChain or when when a new TTree
// is started when using PROOF. It is normally not necessary to make changes
// to the generated code, but the routine can be extended by the
// user if needed. The return value is currently not used.

    return kTRUE;
}


void CEffArea::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
    if (!fChain) return;
    fChain->Show(entry);
}


Int_t CEffArea::Cut(Long64_t entry)
{
    entry = 1;
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
    return 1;
}
#endif                                            // #ifdef CEffArea_cxx
