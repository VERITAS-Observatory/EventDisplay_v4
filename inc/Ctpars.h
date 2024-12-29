// Image parameter tree 'tpars' definition.

#ifndef Ctpars_h
#define Ctpars_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

using namespace std;

class Ctpars
{
    public :
        bool            bMC;
        bool            bsdevxy;
        unsigned int    bShort;
        TTree*          fChain;                   //!pointer to the analyzed TTree or TChain
        Int_t           fCurrent;                 //!current Tree number in a TChain

        // Declaration of leave types
        Int_t           telID;
        Int_t           runNumber;
        Int_t           MJD;
        Double_t        Time;
        Int_t           eventNumber;
        Double_t        fimagethresh;
        Double_t        fborderthresh;
        Int_t           fsumfirst;
        Int_t           fsumwindow;
        Int_t           fsumwindow_2;
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
        Float_t         f_s;
        Float_t         f_d;
        Float_t         f_sdevxy;
        Float_t         length;
        Float_t         width;
        Float_t         size;
        Float_t         loss;
        Float_t         fracLow;
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
        UShort_t        nlowgain;
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
        Float_t         muonIPCorrectedSize;
        Int_t           muonValid;
        Int_t           houghMuonValid;
        Int_t           Fitstat;

        // List of branches
        TBranch*        b_telID;                  //!
        TBranch*        b_runNumber;              //!
        TBranch*        b_MJD;                    //!
        TBranch*        b_Time;                   //!
        TBranch*        b_eventNumber;            //!
        TBranch*        b_fimagethresh;           //!
        TBranch*        b_fborderthresh;          //!
        TBranch*        b_fsumfirst;              //!
        TBranch*        b_fsumwindow;             //!
        TBranch*        b_fsumwindow_2;             //!
        TBranch*        b_fsumwindowsmall;        //!
        TBranch*        b_LocalTrigger;           //!
        TBranch*        b_MCprim;                 //!
        TBranch*        b_MCe0;                   //!
        TBranch*        b_MCxcore;                //!
        TBranch*        b_MCycore;                //!
        TBranch*        b_MCxcos;                 //!
        TBranch*        b_MCycos;                 //!
        TBranch*        b_MCLocalTriggerTime;     //!
        //!
        TBranch*        b_MCLocalDelayedTriggerTime;
        TBranch*        b_MCTel_Xoff;             //!
        TBranch*        b_MCTel_Yoff;             //!
        TBranch*        b_meanPed_Image;          //!
        TBranch*        b_meanPedvar_Image;       //!
        TBranch*        b_cen_x;                  //!
        TBranch*        b_cen_y;                  //!
        TBranch*        b_f_s;                    //!
        TBranch*        b_f_d;                    //!
        TBranch*        b_f_sdevxy;               //!
        TBranch*        b_length;                 //!
        TBranch*        b_width;                  //!
        TBranch*        b_size;                   //!
        TBranch*        b_loss;                   //!
        TBranch*        b_fracLow;                //!
        TBranch*        b_fui;                    //!
        TBranch*        b_dist;                   //!
        TBranch*        b_azwidth;                //!
        TBranch*        b_alpha;                  //!
        TBranch*        b_los;                    //!
        TBranch*        b_miss;                   //!
        TBranch*        b_phi;                    //!
        TBranch*        b_cosphi;                 //!
        TBranch*        b_sinphi;                 //!
        TBranch*        b_ntubes;                 //!
        TBranch*        b_ntubesBNI;              //!
        TBranch*        b_ntrig;                  //!
        TBranch*        b_ntrig_per_patch;        //!
        TBranch*        b_nsat;                   //!
        TBranch*        b_nlowgain;               //!
        TBranch*        b_bad;                    //!
        TBranch*        b_max;                    //!
        TBranch*        b_index_of_max;           //!
        TBranch*        b_frac;                   //!
        TBranch*        b_asymmetry;              //!
        TBranch*        b_tgrad_x;                //!
        TBranch*        b_tgrad_y;                //!
        TBranch*        b_tgrad_r;                //!
        TBranch*        b_tint_x;                 //!
        TBranch*        b_tgrad_dx;               //!
        TBranch*        b_tint_dx;                //!
        TBranch*        b_tchisq_x;               //!
        TBranch*        b_tmin;                   //!
        TBranch*        b_tmax;                   //!
        TBranch*        b_tmean;                  //!
        TBranch*        b_Fitstat;                  //!
        TBranch*        b_muonX0;
        TBranch*        b_muonY0;
        TBranch*        b_muonRadius;
        TBranch*        b_muonRSigma;
        TBranch*        b_muonSize;
        TBranch*        b_muonIPCorrectedSize;
        TBranch*        b_muonValid;
        TBranch*        b_houghMuonValid;

        Ctpars( TTree* tree = 0, bool iMC = false, unsigned int iShort = false );
        virtual ~Ctpars();
        virtual Int_t    GetEntry( Long64_t entry );
        virtual Long64_t LoadTree( Long64_t entry );
        virtual void     Init( TTree* tree );
        virtual Bool_t   Notify();
        bool             has_sdevxy()
        {
            return bsdevxy;
        }
        bool             isMC()
        {
            return bMC;
        }
        unsigned int     isShort()
        {
            return bShort;
        }
};
#endif
