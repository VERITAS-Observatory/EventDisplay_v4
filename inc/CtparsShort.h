//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Feb  8 14:35:45 2007 by ROOT version 5.10/00
// from TTree tpars/Event Parameters (Telescope 1)
// found on file: output/32855.root
//////////////////////////////////////////////////////////
//
//   adjusted to mscw_energy
//
//
//  Revision $Id: CtparsShort.h,v 1.1.2.1.10.1.2.1.4.2.10.1.2.1.2.4 2010/09/29 10:09:54 gmaier Exp $
//
//
//   DO NOT OVERWRITE !!!!
//
//   (GM)
///////////////////////////////////////////

#ifndef CtparsShort_h
#define CtparsShort_h

#define TREES_VERSION2

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

using namespace std;

class CtparsShort
{
    public :
        bool            bMC;
        bool            bShort;
        int             fVersion;
        TTree          *fChain;                   //!pointer to the analyzed TTree or TChain
        Int_t           fCurrent;                 //!current Tree number in a TChain

// Declaration of leave types
#ifdef TREES_VERSION2
        Int_t           telID;
        Int_t           runNumber;
        Int_t           MJD;
        Double_t        Time;
        Int_t           eventNumber;
        Double_t        fimagethresh;
        Double_t        fborderthresh;
        Int_t           fsumfirst;
        Int_t           fsumwindow;
        Int_t           fsumwindowsmall;
        Short_t         LocalTrigger;
        Short_t           MCprim;
        Float_t        MCe0;
        Float_t        MCxcore;
        Float_t        MCycore;
        Float_t        MCxcos;
        Float_t        MCycos;
        Float_t        MCLocalTriggerTime;
        Float_t        MCLocalDelayedTriggerTime;
        Float_t        MCTel_Xoff;
        Float_t        MCTel_Yoff;
        Float_t         meanPed_Image;
        Float_t         meanPedvar_Image;
        Float_t         cen_x;
        Float_t         cen_y;
        Float_t         length;
        Float_t         width;
        Float_t         size;
        Float_t         loss;
        Float_t         fui;
        Float_t         dist;
        Float_t         azwidth;
        Float_t         alpha;
        Float_t         los;
        Float_t         miss;
        Float_t         phi;
        Float_t         cosphi;
        Float_t         sinphi;
        UShort_t        ntubes;
        UShort_t        ntubesBNI;
        UShort_t        ntrig;
        UShort_t        ntrig_per_patch;
        UShort_t        nsat;
        UShort_t        bad;
        Float_t         max[3];
        UShort_t        index_of_max[3];
        Float_t         frac[3];
        Float_t         asymmetry;
        Float_t         tgrad_x;
        Float_t         tint_x;
        Float_t         tgrad_dx;
        Float_t         tint_dx;
        Float_t         tchisq_x;
        Float_t         muonX0;
        Float_t         muonY0;
        Float_t         muonRadius;
        Float_t         muonRSigma;
        Float_t         muonSize;
        Int_t           muonValid;
#else
        Int_t           telID;
        Int_t           runNumber;
        Int_t           MJD;
        Double_t        Time;
        Int_t           eventNumber;
        Double_t        fimagethresh;
        Double_t        fborderthresh;
        Int_t           fsumfirst;
        Int_t           fsumwindow;
        Int_t           fsumwindowsmall;
        Short_t         LocalTrigger;
        Int_t           MCprim;
        Double_t        MCe0;
        Double_t        MCxcore;
        Double_t        MCycore;
        Double_t        MCxcos;
        Double_t        MCycos;
        Double_t        MCLocalTriggerTime;
        Double_t        MCLocalDelayedTriggerTime;
        Double_t        MCTel_Xoff;
        Double_t        MCTel_Yoff;
        Double_t        cen_x;
        Double_t        cen_y;
        Double_t        length;
        Double_t        width;
        Double_t        size;
        Double_t        loss;
        Double_t        dist;
        Double_t        azwidth;
        Double_t        alpha;
        Double_t        los;
        Double_t        miss;
        Double_t        phi;
        Double_t        cosphi;
        Double_t        sinphi;
        Int_t           ntubes;
        Int_t           ntrig;
        Int_t           ntrig_per_patch;
        Int_t           nsat;
        Int_t           bad;
        Double_t        max[3];
        Int_t           index_of_max[3];
        Double_t        frac[3];
        Double_t        asymmetry;
        Double_t        tgrad_x;
        Double_t        tgrad_y;
        Double_t        tgrad_r;
        Double_t        tint_x;
        Double_t        tint_y;
        Double_t        tint_r;
        Double_t        tgrad_dx;
        Double_t        tgrad_dy;
        Double_t        tgrad_dr;
        Double_t        tint_dx;
        Double_t        tint_dy;
        Double_t        tint_dr;
        Double_t        tchisq_x;
        Double_t        tchisq_y;
        Double_t        tchisq_r;
        Double_t        tmin;
        Double_t        tmax;
        Double_t        tmean;
#endif

// List of branches
        TBranch        *b_telID;                  //!
        TBranch        *b_runNumber;              //!
        TBranch        *b_MJD;                    //!
        TBranch        *b_Time;                   //!
        TBranch        *b_eventNumber;            //!
        TBranch        *b_fimagethresh;           //!
        TBranch        *b_fborderthresh;          //!
        TBranch        *b_fsumfirst;              //!
        TBranch        *b_fsumwindow;             //!
        TBranch        *b_fsumwindowsmall;        //!
        TBranch        *b_LocalTrigger;           //!
        TBranch        *b_MCprim;                 //!
        TBranch        *b_MCe0;                   //!
        TBranch        *b_MCxcore;                //!
        TBranch        *b_MCycore;                //!
        TBranch        *b_MCxcos;                 //!
        TBranch        *b_MCycos;                 //!
        TBranch        *b_MCLocalTriggerTime;     //!
                                                  //!
        TBranch        *b_MCLocalDelayedTriggerTime;
        TBranch        *b_MCTel_Xoff;             //!
        TBranch        *b_MCTel_Yoff;             //!
        TBranch        *b_meanPed_Image;          //!
        TBranch        *b_meanPedvar_Image;       //!
        TBranch        *b_cen_x;                  //!
        TBranch        *b_cen_y;                  //!
        TBranch        *b_length;                 //!
        TBranch        *b_width;                  //!
        TBranch        *b_size;                   //!
        TBranch        *b_loss;                   //!
        TBranch        *b_fui;                    //!
        TBranch        *b_dist;                   //!
        TBranch        *b_azwidth;                //!
        TBranch        *b_alpha;                  //!
        TBranch        *b_los;                    //!
        TBranch        *b_miss;                   //!
        TBranch        *b_phi;                    //!
        TBranch        *b_cosphi;                 //!
        TBranch        *b_sinphi;                 //!
        TBranch        *b_ntubes;                 //!
        TBranch        *b_ntubesBNI;              //!
        TBranch        *b_ntrig;                  //!
        TBranch        *b_ntrig_per_patch;        //!
        TBranch        *b_nsat;                   //!
        TBranch        *b_bad;                    //!
        TBranch        *b_max;                    //!
        TBranch        *b_index_of_max;           //!
        TBranch        *b_frac;                   //!
        TBranch        *b_asymmetry;              //!
        TBranch        *b_tgrad_x;                //!
        TBranch        *b_tgrad_y;                //!
        TBranch        *b_tgrad_r;                //!
        TBranch        *b_tint_x;                 //!
        TBranch        *b_tint_y;                 //!
        TBranch        *b_tint_r;                 //!
        TBranch        *b_tgrad_dx;               //!
        TBranch        *b_tgrad_dy;               //!
        TBranch        *b_tgrad_dr;               //!
        TBranch        *b_tint_dx;                //!
        TBranch        *b_tint_dy;                //!
        TBranch        *b_tint_dr;                //!
        TBranch        *b_tchisq_x;               //!
        TBranch        *b_tchisq_y;               //!
        TBranch        *b_tchisq_r;               //!
        TBranch        *b_tmin;                   //!
        TBranch        *b_tmax;                   //!
        TBranch        *b_tmean;                  //!

        CtparsShort(TTree *tree = 0, bool iMC = false, int iVersion = 2, bool iShort = false );
        virtual ~CtparsShort();
        virtual Int_t    Cut(Long64_t entry);
        virtual Int_t    GetEntry(Long64_t entry);
        virtual Long64_t LoadTree(Long64_t entry);
        virtual void     Init(TTree *tree);
        virtual void     Loop();
        virtual Bool_t   Notify();
        virtual void     Show(Long64_t entry = -1);
        bool             isMC() { return bMC; }
        bool             isShort() { return bShort; }
};
#endif

#ifdef CtparsShort_cxx

CtparsShort::CtparsShort(TTree *tree, bool iMC, int iVersion, bool iShort )
{
    if( !tree ) return;

    bMC = iMC;
    bShort = iShort;
    fVersion = iVersion;

    Init(tree);
}


CtparsShort::~CtparsShort()
{
    if (!fChain) return;
    delete fChain->GetCurrentFile();
}


Int_t CtparsShort::GetEntry(Long64_t entry)
{
// Read contents of entry.
    if (!fChain) return 0;
    return fChain->GetEntry(entry);
}


Long64_t CtparsShort::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
    if (!fChain) return -5;
    Long64_t centry = fChain->LoadTree(entry);
    if (centry < 0) return centry;
    if (fChain->IsA() != TChain::Class()) return centry;
    TChain *chain = (TChain*)fChain;
    if (chain->GetTreeNumber() != fCurrent)
    {
        fCurrent = chain->GetTreeNumber();
        Notify();
    }
    return centry;
}


void CtparsShort::Init(TTree *tree)
{

// Set branch addresses
    if (tree == 0) return;
    fChain = tree;
    fCurrent = -1;
    fChain->SetMakeClass(1);

    fChain->SetBranchAddress("eventNumber", &eventNumber );
    if( fVersion > 3 ) fChain->SetBranchAddress("meanPed_Image", &meanPed_Image );
    else               meanPed_Image = 0;
    if( fVersion > 3 ) fChain->SetBranchAddress("meanPedvar_Image", &meanPedvar_Image );
    else               meanPedvar_Image = 0;
    fChain->SetBranchAddress("cen_x",&cen_x);
    fChain->SetBranchAddress("cen_y",&cen_y);
    fChain->SetBranchAddress("length",&length);
    fChain->SetBranchAddress("width",&width);
    fChain->SetBranchAddress("size",&size);
    if( fVersion > 2 ) fChain->SetBranchAddress("loss",&loss);
    else               loss = 0.;
    if( fVersion > 6 ) fChain->SetBranchAddress("fui", &fui);
    else               fui = 0.;
    fChain->SetBranchAddress("dist",&dist);
    fChain->SetBranchAddress("ntubes",&ntubes);
    if( !bShort )
    {
        fChain->SetBranchAddress("alpha",&alpha);
        fChain->SetBranchAddress("los",&los);
        fChain->SetBranchAddress("phi",&phi);
    }
    else
    {
        alpha = 0.;
        los = 0.;
        phi = 0.;
    }
    fChain->SetBranchAddress("cosphi",&cosphi);
    fChain->SetBranchAddress("sinphi",&sinphi);
    if( fChain->GetBranchStatus( "ntubesBNI" ) ) fChain->SetBranchAddress("ntubesBNI", &ntubesBNI );
    else ntubesBNI = 0;
    fChain->SetBranchAddress("nsat",&nsat);
    fChain->SetBranchAddress("max",max);
    fChain->SetBranchAddress("index_of_max",index_of_max);
    fChain->SetBranchAddress("asymmetry",&asymmetry);
    fChain->SetBranchAddress("tgrad_x",&tgrad_x);
    if( !bShort )
    {
        fChain->SetBranchAddress("tchisq_x",&tchisq_x);
    }
    else
    {
        tchisq_x = 0.;
    }
    Notify();
}


Bool_t CtparsShort::Notify()
{

// Get branch pointers
    b_eventNumber = fChain->GetBranch("eventNumber" );
    if( fVersion > 2 ) b_meanPed_Image = fChain->GetBranch("meanPed_Image" );
    else               b_meanPed_Image = 0;
    if( fVersion > 2 ) b_meanPedvar_Image = fChain->GetBranch("meanPedvar_Image" );
    else               b_meanPedvar_Image = 0;
    b_cen_x = fChain->GetBranch("cen_x");
    b_cen_y = fChain->GetBranch("cen_y");
    b_length = fChain->GetBranch("length");
    b_width = fChain->GetBranch("width");
    b_size = fChain->GetBranch("size");
    if( fVersion > 2 ) b_loss = fChain->GetBranch("loss");
    else               b_loss = 0;
    if( fVersion > 6 ) b_fui = fChain->GetBranch("fui" );
    else               b_fui = 0;
    b_dist = fChain->GetBranch("dist");
    b_ntubes = fChain->GetBranch("ntubes");
    if( !bShort )
    {
        b_alpha = fChain->GetBranch("alpha");
        b_los = fChain->GetBranch("los");
        b_phi = fChain->GetBranch("phi");
        b_cosphi = fChain->GetBranch("cosphi");
        b_sinphi = fChain->GetBranch("sinphi");
    }
    else
    {
        b_alpha = 0;
        b_los = 0;
        b_phi = 0;
        b_cosphi = 0;
        b_sinphi = 0;
    }
    if( fChain->GetBranchStatus( "ntubesBNI" ) ) b_ntubesBNI = fChain->GetBranch("ntubesBNI" );
    else ntubesBNI = 0;
    b_nsat = fChain->GetBranch("nsat");
    b_max = fChain->GetBranch("max");
    b_index_of_max = fChain->GetBranch("index_of_max");
    b_asymmetry = fChain->GetBranch("asymmetry");
    b_tgrad_x = fChain->GetBranch("tgrad_x");
    if( !bShort )
    {
        b_tchisq_x = fChain->GetBranch("tchisq_x");
    }
    else
    {
        b_tchisq_x = 0;
    }

    return kTRUE;
}


void CtparsShort::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
    if (!fChain) return;
    fChain->Show(entry);
}


Int_t CtparsShort::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
    return 1;
}
#endif                                            // #ifdef CtparsShort_cxx
