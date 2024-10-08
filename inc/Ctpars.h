//////////////////////////////////////////////////////////
//
//   adjusted to mscw_energy
//
//   DO NOT OVERWRITE !!!!
//
///////////////////////////////////////////

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
        virtual void     Loop();
        virtual Bool_t   Notify();
        virtual void     Show( Long64_t entry = -1 );
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

#ifdef Ctpars_cxx

/*

    optimization of tree reading:

    bShort = 0:  read all branches
    bShort = 1:  read limited number of branches needed for lookup table analysis
    bShort = 2:  read limited number of branched needed for lookup table filling

*/
Ctpars::Ctpars( TTree* tree, bool iMC, unsigned int iShort )
{
    if(!tree )
    {
        return;
    }
    bMC = iMC;
    bShort = iShort;
    // forward I/O
    // is supposed to speed up reading significantly
    // problem: large memory consumption (>12 GB for certain cta arrays)
    //    tree->SetCacheSize(10000000);
    bsdevxy = false;

    Init( tree );
}


Ctpars::~Ctpars()
{
    if(!fChain )
    {
        return;
    }
    delete fChain->GetCurrentFile();
}


Int_t Ctpars::GetEntry( Long64_t entry )
{
    // Read contents of entry.
    if(!fChain )
    {
        return 0;
    }
    return fChain->GetEntry( entry );
}


Long64_t Ctpars::LoadTree( Long64_t entry )
{
    // Set the environment to read one entry
    if(!fChain )
    {
        return -5;
    }
    Long64_t centry = fChain->LoadTree( entry );
    if( centry < 0 )
    {
        return centry;
    }
    if( fChain->IsA() != TChain::Class() )
    {
        return centry;
    }
    TChain* chain = ( TChain* )fChain;
    if( chain->GetTreeNumber() != fCurrent )
    {
        fCurrent = chain->GetTreeNumber();
        Notify();
    }
    return centry;
}


void Ctpars::Init( TTree* tree )
{

    // Set branch addresses
    if( tree == 0 )
    {
        return;
    }
    fChain = tree;
    fCurrent = -1;
    fChain->SetMakeClass( 1 );
    /////////////////////////////////////////////////
    //    bShort = 2:  read limited number of branched needed for lookup table filling
    if( bShort <= 2 )
    {
        if( fChain->GetBranchStatus( "meanPedvar_Image" ) )
        {
            fChain->SetBranchAddress( "meanPedvar_Image", &meanPedvar_Image );
        }
        else
        {
            meanPedvar_Image = 0;
        }
        fChain->SetBranchAddress( "ntubes", &ntubes );
        fChain->SetBranchAddress( "length", &length );
        fChain->SetBranchAddress( "width", &width );
        fChain->SetBranchAddress( "size", &size );
        fChain->SetBranchAddress( "cen_x", &cen_x );
        fChain->SetBranchAddress( "cen_y", &cen_y );
        fChain->SetBranchAddress( "loss", &loss );
        fChain->SetBranchAddress( "cosphi", &cosphi );
        fChain->SetBranchAddress( "sinphi", &sinphi );
        if( fChain->GetBranchStatus( "fui" ) )
        {
            fChain->SetBranchAddress( "fui", &fui );
        }
        else
        {
            fui = 0.;
        }
    }
    //    bShort = 1:  read limited number of branches needed for lookup table analysis
    if( bShort <= 1 )
    {
        if( fChain->GetBranchStatus( "eventNumber" ) )
        {
            fChain->SetBranchAddress( "eventNumber", &eventNumber );
        }
        else
        {
            eventNumber = 0;
        }
        if( fChain->GetBranchStatus( "meanPed_Image" ) )
        {
            fChain->SetBranchAddress( "meanPed_Image", &meanPed_Image );
        }
        else
        {
            meanPed_Image = 0;
        }
        if( fChain->GetBranchStatus( "f_d" ) )
        {
            fChain->SetBranchAddress( "f_d", &f_d );
            bsdevxy = true;
        }
        else
        {
            f_d = 0.;
        }
        if( fChain->GetBranchStatus( "f_s" ) )
        {
            fChain->SetBranchAddress( "f_s", &f_s );
        }
        else
        {
            f_s = 0.;
        }
        if( fChain->GetBranchStatus( "f_sdevxy" ) )
        {
            fChain->SetBranchAddress( "f_sdevxy", &f_sdevxy );
        }
        else
        {
            f_sdevxy = 0.;
        }

        if( fChain->GetBranchStatus( "fracLow" ) )
        {

            fChain->SetBranchAddress( "fracLow", &fracLow );
        }
        else
        {
            fracLow = 0.;
        }
        fChain->SetBranchAddress( "dist", &dist );
        if( fChain->GetBranchStatus( "ntubesBNI" ) )
        {
            fChain->SetBranchAddress( "ntubesBNI", &ntubesBNI );
        }
        else
        {
            ntubesBNI = 0;
        }
        fChain->SetBranchAddress( "nsat", &nsat );
        if( fChain->GetBranchStatus( "nlowgain" ) )
        {
            fChain->SetBranchAddress( "nlowgain", &nlowgain );
        }
        else
        {
            nlowgain = 0;
        }
        fChain->SetBranchAddress( "asymmetry", &asymmetry );
        fChain->SetBranchAddress( "tgrad_x", &tgrad_x );
        fChain->SetBranchAddress( "Fitstat", &Fitstat );
        // reset variables which are not read out
        alpha = 0.;
        los = 0.;
        phi = 0.;
        max[0] = 0.;
        max[1] = 0.;
        max[2] = 0.;
        index_of_max[0] = 0;
        index_of_max[1] = 0;
        index_of_max[2] = 0;
        tchisq_x = 0.;

        //muon analysis//
        if( fChain->GetBranchStatus( "muonX0" ) )
        {
            fChain->SetBranchAddress( "muonX0", &muonX0 );
        }
        else
        {
            muonX0 = 0;
        }
        if( fChain->GetBranchStatus( "muonY0" ) )
        {
            fChain->SetBranchAddress( "muonY0", &muonY0 );
        }
        else
        {
            muonY0 = 0;
        }
        if( fChain->GetBranchStatus( "muonRadius" ) )
        {
            fChain->SetBranchAddress( "muonRadius", &muonRadius );
        }
        else
        {
            muonRadius = 0;
        }
        if( fChain->GetBranchStatus( "muonRSigma" ) )
        {
            fChain->SetBranchAddress( "muonRSigma", &muonRSigma );
        }
        else
        {
            muonRSigma = 0;
        }
        if( fChain->GetBranchStatus( "muonSize" ) )
        {
            fChain->SetBranchAddress( "muonSize", &muonSize );
        }
        else
        {
            muonSize = 0;
        }
        if( fChain->GetBranchStatus( "muonSize" ) )
        {
            fChain->SetBranchAddress( "muonSize", &muonSize );
        }
        else
        {
            muonSize = 0;
        }
        if( fChain->GetBranchStatus( "muonIPCorrectedSize" ) )
        {
            fChain->SetBranchAddress( "muonIPCorrectedSize", &muonIPCorrectedSize );
        }
        else
        {
            muonIPCorrectedSize = 0;
        }
        if( fChain->GetBranchStatus( "muonValid" ) )
        {
            fChain->SetBranchAddress( "muonValid", &muonValid );
        }
        else
        {
            muonValid = 0;
        }
        if( fChain->GetBranchStatus( "houghMuonValid" ) )
        {
            fChain->SetBranchAddress( "houghMuonValid", &houghMuonValid );
        }
        else
        {
            houghMuonValid = 0;
        }

    }
    // bShort == 0: read all branches
    if( bShort == 0 )
    {
        fChain->SetBranchAddress( "alpha", &alpha );
        fChain->SetBranchAddress( "los", &los );
        fChain->SetBranchAddress( "phi", &phi );
        fChain->SetBranchAddress( "max", max );
        fChain->SetBranchAddress( "index_of_max", index_of_max );
        fChain->SetBranchAddress( "tchisq_x", &tchisq_x );
    }
    Notify();
}

/*
    Get branch pointers
*/
Bool_t Ctpars::Notify()
{

    b_eventNumber = 0;
    b_meanPed_Image = 0;
    b_meanPedvar_Image = 0;
    b_length = 0;
    b_width = 0;
    b_size = 0;
    b_cen_x = 0;
    b_cen_y = 0;
    b_f_s = 0;
    b_f_d = 0;
    b_f_sdevxy = 0;
    b_loss = 0;
    b_fracLow = 0;
    b_alpha = 0;
    b_los = 0;
    b_phi = 0;
    b_cosphi = 0;
    b_sinphi = 0;
    b_ntubesBNI = 0;
    b_nlowgain = 0;
    b_fui = 0;
    b_max = 0;
    b_index_of_max = 0;
    b_tchisq_x = 0;
    b_Fitstat = 0;
    b_muonX0 = 0;
    b_muonY0 = 0;
    b_muonRadius = 0;
    b_muonRSigma = 0;
    b_muonSize = 0;
    b_muonIPCorrectedSize = 0;
    b_muonValid = 0;
    b_houghMuonValid = 0;

    // get branch pointers
    if( bShort <= 2 )
    {
        if( fChain->GetBranchStatus( "meanPedvar_Image" ) )
        {
            b_meanPedvar_Image = fChain->GetBranch( "meanPedvar_Image" );
            fChain->AddBranchToCache( b_meanPedvar_Image );
        }
        else
        {
            b_meanPedvar_Image = 0;
        }
        b_ntubes = fChain->GetBranch( "ntubes" );
        fChain->AddBranchToCache( b_ntubes );
        b_length = fChain->GetBranch( "length" );
        fChain->AddBranchToCache( b_length );
        b_width = fChain->GetBranch( "width" );
        fChain->AddBranchToCache( b_width );
        b_size = fChain->GetBranch( "size" );
        fChain->AddBranchToCache( b_size );
        b_cen_x = fChain->GetBranch( "cen_x" );
        fChain->AddBranchToCache( b_cen_x );
        b_cen_y = fChain->GetBranch( "cen_y" );
        fChain->AddBranchToCache( b_cen_y );
        b_cosphi = fChain->GetBranch( "cosphi" );
        b_sinphi = fChain->GetBranch( "sinphi" );
        if( fChain->GetBranchStatus( "fui" ) )
        {
            b_fui = fChain->GetBranch( "fui" );
        }
    }
    if( bShort <= 1 )
    {
        if( fChain->GetBranchStatus( "eventNumber" ) )
        {
            b_eventNumber = fChain->GetBranch( "eventNumber" );
        }
        if( fChain->GetBranchStatus( "meanPed_Image" ) )
        {
            b_meanPed_Image = fChain->GetBranch( "meanPed_Image" );
        }
        else
        {
            b_meanPed_Image = 0;
        }
        if( fChain->GetBranchStatus( "f_d" ) )
        {
            b_f_d = fChain->GetBranch( "f_d" );
            fChain->AddBranchToCache( b_f_d );
        }
        if( fChain->GetBranchStatus( "f_s" ) )
        {
            b_f_s = fChain->GetBranch( "f_s" );
            fChain->AddBranchToCache( b_f_s );
        }
        if( fChain->GetBranchStatus( "f_sdevxy" ) )
        {
            b_f_sdevxy = fChain->GetBranch( "f_sdevxy" );
            fChain->AddBranchToCache( b_f_sdevxy );
        }
        b_loss = fChain->GetBranch( "loss" );
        fChain->AddBranchToCache( b_loss );
        if( fChain->GetBranchStatus( "fracLow" ) )
        {
            b_fracLow = fChain->GetBranch( "fracLow" );
            fChain->AddBranchToCache( b_fracLow );
        }
        b_dist = fChain->GetBranch( "dist" );
        fChain->AddBranchToCache( b_dist );
        if( fChain->GetBranchStatus( "ntubesBNI" ) )
        {
            b_ntubesBNI = fChain->GetBranch( "ntubesBNI" );
        }
        b_nsat = fChain->GetBranch( "nsat" );
        fChain->AddBranchToCache( b_nsat );
        if( fChain->GetBranchStatus( "nlowgain" ) )
        {
            b_nlowgain = fChain->GetBranch( "nlowgain" );
            fChain->AddBranchToCache( b_nlowgain );
        }
        b_asymmetry = fChain->GetBranch( "asymmetry" );
        fChain->AddBranchToCache( b_asymmetry );
        b_tgrad_x = fChain->GetBranch( "tgrad_x" );
        fChain->AddBranchToCache( b_tgrad_x );
        b_Fitstat = fChain->GetBranch( "Fitstat" );
        fChain->AddBranchToCache( b_Fitstat );
        //muon
        if( fChain->GetBranchStatus( "muonX0" ) )
        {
            b_muonX0 = fChain->GetBranch( "muonX0" );
            fChain->AddBranchToCache( b_muonX0 );
        }
        if( fChain->GetBranchStatus( "muonY0" ) )
        {
            b_muonY0 = fChain->GetBranch( "muonY0" );
            fChain->AddBranchToCache( b_muonY0 );
        }
        if( fChain->GetBranchStatus( "muonRadius" ) )
        {
            b_muonRadius = fChain->GetBranch( "muonRadius" );
            fChain->AddBranchToCache( b_muonRadius );
        }
        if( fChain->GetBranchStatus( "muonRSigma" ) )
        {
            b_muonRSigma = fChain->GetBranch( "muonRSigma" );
            fChain->AddBranchToCache( b_muonRSigma );
        }
        if( fChain->GetBranchStatus( "muonSize" ) )
        {
            b_muonSize = fChain->GetBranch( "muonSize" );
            fChain->AddBranchToCache( b_muonSize );
        }
        if( fChain->GetBranchStatus( "muonIPCorrectedSize" ) )
        {
            b_muonIPCorrectedSize = fChain->GetBranch( "muonIPCorrectedSize" );
            fChain->AddBranchToCache( b_muonIPCorrectedSize );
        }
        if( fChain->GetBranchStatus( "muonValid" ) )
        {
            b_muonValid = fChain->GetBranch( "muonValid" );
            fChain->AddBranchToCache( b_muonValid );
        }
        if( fChain->GetBranchStatus( "houghMuonValid" ) )
        {
            b_houghMuonValid = fChain->GetBranch( "houghMuonValid" );
            fChain->AddBranchToCache( b_houghMuonValid );
        }
    }
    if( bShort == 0 )
    {
        b_alpha = fChain->GetBranch( "alpha" );
        b_los = fChain->GetBranch( "los" );
        b_phi = fChain->GetBranch( "phi" );
        b_max = fChain->GetBranch( "max" );
        b_index_of_max = fChain->GetBranch( "index_of_max" );
        b_tchisq_x = fChain->GetBranch( "tchisq_x" );
    }

    return kTRUE;
}


void Ctpars::Show( Long64_t entry )
{
    // Print contents of entry.
    // If entry is not specified, print current entry
    if(!fChain )
    {
        return;
    }
    fChain->Show( entry );
}


#endif                                            // #ifdef Ctpars_cxx
