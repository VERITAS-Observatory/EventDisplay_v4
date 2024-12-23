//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri Sep 22 23:32:42 2006 by ROOT version 5.06/00
// from TTree data/MSWC and energy lookup results
// found on file: 31391.mscw.root
//////////////////////////////////////////////////////////

#ifndef CData_h
#define CData_h

#include <TChain.h>
#include <TFile.h>

#include "VGlobalRunParameter.h"

#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

class CData
{
    public :

        bool            fMC;
        bool            fBOOLtheta2_All;
        bool            fBOOLteltype;
        bool            fBOOLdE;

        bool            fShort;
        int             fVersion;
        TTree*          fChain;                   //!pointer to the analyzed TTree or TChain
        Int_t           fCurrent;                 //!current Tree number in a TChain

        // Declaration of leave types
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
        // MC parameters
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

        UInt_t          LTrigS;
        ULong64_t       LTrig;
        UInt_t          NTrig;
        Int_t           NImages;
        UInt_t          ImgSelS;
        ULong64_t       ImgSel;
        UInt_t          ImgSel_list[VDST_MAXTELESCOPES];
        Int_t           NTtype;
        UInt_t		NImages_Ttype[VDST_MAXTELESCOPES];
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
        Double_t        theta2;
        Double_t        Xcore;
        Double_t        Ycore;
        Double_t        Xcore_SC;
        Double_t        Ycore_SC;
        Double_t        stdP;
        Double_t        Chi2;
        Float_t         meanPedvar_Image;
        Float_t         meanPedvar_ImageT[VDST_MAXTELESCOPES];
        Double_t        dist[VDST_MAXTELESCOPES];
        Double_t        size[VDST_MAXTELESCOPES];
        Double_t        fraclow[VDST_MAXTELESCOPES];
        Double_t        loss[VDST_MAXTELESCOPES];
        Double_t        max1[VDST_MAXTELESCOPES];
        Double_t        max2[VDST_MAXTELESCOPES];
        Double_t        max3[VDST_MAXTELESCOPES];
        Int_t           maxindex1[VDST_MAXTELESCOPES];
        Int_t           maxindex2[VDST_MAXTELESCOPES];
        Int_t           maxindex3[VDST_MAXTELESCOPES];
        Double_t        width[VDST_MAXTELESCOPES];
        Double_t        length[VDST_MAXTELESCOPES];
        Int_t           ntubes[VDST_MAXTELESCOPES];
        Int_t           ntubesBNI[VDST_MAXTELESCOPES];
        UShort_t        nsat[VDST_MAXTELESCOPES];
        UShort_t        nlowgain[VDST_MAXTELESCOPES];
        Double_t        alpha[VDST_MAXTELESCOPES];
        Double_t        los[VDST_MAXTELESCOPES];
        Double_t        asym[VDST_MAXTELESCOPES];
        Double_t        cen_x[VDST_MAXTELESCOPES];
        Double_t        cen_y[VDST_MAXTELESCOPES];
        Double_t        cosphi[VDST_MAXTELESCOPES];
        Double_t        sinphi[VDST_MAXTELESCOPES];
        Double_t        tgrad_x[VDST_MAXTELESCOPES];
        Double_t        tchisq_x[VDST_MAXTELESCOPES];
        Int_t           Fitstat[VDST_MAXTELESCOPES];
        Float_t         DispXoff_T[VDST_MAXTELESCOPES];
        Float_t         DispYoff_T[VDST_MAXTELESCOPES];
        Float_t         DispWoff_T[VDST_MAXTELESCOPES];
        Float_t         Disp_T[VDST_MAXTELESCOPES];
        Double_t        R[VDST_MAXTELESCOPES];
        Double_t        MSCWT[VDST_MAXTELESCOPES];
        Double_t        MSCLT[VDST_MAXTELESCOPES];
        Double_t        E[VDST_MAXTELESCOPES];
        Double_t        ES[VDST_MAXTELESCOPES];
        Int_t           NMSCW;
        Double_t        MSCW;
        Double_t        MSCL;
        Float_t         MWR;
        Float_t         MLR;
        Float_t        Erec;
        Float_t        EChi2;
        Float_t        dE;        // Error on Erec
        Float_t        ErecS;
        Float_t        EChi2S;
        Float_t        dES;       // Error on ErecS
        Double_t        SizeSecondMax;
        Double_t        theta2_All[25];
        Float_t         EmissionHeight;
        Float_t         EmissionHeightChi2;
        UInt_t          NTelPairs;
        //[NTelPairs]
        Float_t         EmissionHeightT[VDST_MAXTELESCOPES * VDST_MAXTELESCOPES];
        Double_t        DispDiff;  // from disp method
        Float_t         DispAbsSumWeigth;
        Float_t         Xoff_intersect;
        Float_t         Yoff_intersect;

        // List of branches
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
        TBranch*        b_R;                      //!
        TBranch*        b_MSCWT;                  //!
        TBranch*        b_MSCLT;                  //!
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
        TBranch*        b_SizeSecondMax;          //!
        TBranch*        b_theta2_All;             //!
        TBranch*        b_EmissionHeight;         //!
        TBranch*        b_EmissionHeightChi2;     //!
        TBranch*        b_NTelPairs;              //!
        TBranch*        b_EmissionHeightT;        //!
        TBranch*        b_DispDiff; //disp
        TBranch*        b_DispAbsSumWeigth;
        TBranch*        b_Xoff_intersect;
        TBranch*        b_Yoff_intersect;

        CData( TTree* tree = 0, bool bMC = false, int iVersion = 5, bool bShort = false );
        virtual ~CData();
        virtual Int_t    Cut( Long64_t entry );
        virtual Int_t    GetEntry( Long64_t entry );
        virtual Long64_t LoadTree( Long64_t entry );
        virtual void     Init( TTree* tree );
        virtual void     Loop();
        virtual Bool_t   Notify();
        virtual void     Show( Long64_t entry = -1 );
        bool             isMC()
        {
            return fMC;
        }
        int              getVersion()
        {
            return fVersion;
        }
};
#endif

#ifdef CData_cxx

CData::CData( TTree* tree, bool bMC, int iVersion, bool bShort )
{
    fMC = bMC;
    fShort = bShort;
    fVersion = iVersion;
    fBOOLtheta2_All = false;
    fBOOLteltype = false;
    fBOOLdE = false;

    Init( tree );
}


CData::~CData()
{
    if(!fChain )
    {
        return;
    }
    delete fChain->GetCurrentFile();
}


Int_t CData::GetEntry( Long64_t entry )
{
    // Read contents of entry.
    if(!fChain )
    {
        return 0;
    }

    int a = fChain->GetEntry( entry );
    if( a > 0 && fVersion < 6 )
    {
        LTrig = ( ULong64_t )LTrigS;
        ImgSel = ( ULong64_t )ImgSelS;
    }
    return a;
}


Long64_t CData::LoadTree( Long64_t entry )
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


void CData::Init( TTree* tree )
{

    // Set branch addresses
    if( tree == 0 )
    {
        return;
    }

    // get version number
    string itemp = tree->GetTitle();
    if( itemp.find( "VERSION" ) < itemp.size() )
    {
        fVersion = atoi( itemp.substr( itemp.find( "VERSION" ) + 7, itemp.size() ).c_str() );
    }
    // data is in a chain -> get first tree and version number from it
    else if( tree->IsA() == TChain::Class() )
    {
        if( tree->LoadTree( 0 ) >= 0 )
        {
            string itemp = "";
            if( tree->GetTree() )
            {
                itemp = tree->GetTree()->GetTitle();
            }
            if( itemp.find( "VERSION" ) < itemp.size() )
            {
                fVersion = atoi( itemp.substr( itemp.find( "VERSION" ) + 7, itemp.size() ).c_str() );
            }
        }
    }
    // test if this is a MC file
    if( tree->GetBranchStatus( "MCe0" ) )
    {
        fMC = true;
    }
    if( tree->GetBranchStatus( "theta2_All" ) )
    {
        fBOOLtheta2_All = true;
    }
    // test if teltype branches exist
    if( tree->GetBranchStatus( "NTtype" ) )
    {
        fBOOLteltype = true;
    }
    // test if dE branches exist
    if( tree->GetBranchStatus( "dE" ) )
    {
        fBOOLdE = true;
    }

    fChain = tree;
    fCurrent = -1;
    fChain->SetMakeClass( 1 );

    fChain->SetBranchAddress( "runNumber", &runNumber );
    fChain->SetBranchAddress( "eventNumber", &eventNumber );
    if(!fShort )
    {
        fChain->SetBranchAddress( "MJD", &MJD );
        fChain->SetBranchAddress( "Time", &Time );
    }
    else
    {
        MJD = 0;
        Time = 0;
    }
    fChain->SetBranchAddress( "TelElevation", TelElevation );
    fChain->SetBranchAddress( "TelAzimuth", TelAzimuth );
    if(!fShort )
    {
        fChain->SetBranchAddress( "TelDec", TelDec );
        fChain->SetBranchAddress( "TelRA", TelRA );
    }
    else
    {
        for( unsigned int i = 0; i < VDST_MAXTELESCOPES; i++ )
        {
            TelDec[i] = 0.;
            TelRA[i] = 0.;
        }
    }
    fChain->SetBranchAddress( "ArrayPointing_Azimuth", &ArrayPointing_Azimuth);
    fChain->SetBranchAddress( "ArrayPointing_Elevation", &ArrayPointing_Elevation);
    if( fChain->GetBranchStatus( "Array_PointingStatus" ) )
    {
        fChain->SetBranchAddress( "Array_PointingStatus", &Array_PointingStatus );
    }
    else
    {
        Array_PointingStatus = 0;
    }

    // MC tree
    if( fMC )
    {
        if( fVersion > 7 )
        {
            fChain->SetBranchAddress( "MCprimary", &MCprimary );
        }
        else
        {
            MCprimary = -99;
        }
        fChain->SetBranchAddress( "MCe0", &MCe0 );
        fChain->SetBranchAddress( "MCxcore", &MCxcore );
        fChain->SetBranchAddress( "MCycore", &MCycore );
        if(!fShort )
        {
            fChain->SetBranchAddress( "MCxcore_SC", &MCxcore_SC );
            fChain->SetBranchAddress( "MCycore_SC", &MCycore_SC );
            fChain->SetBranchAddress( "MCxcos", &MCxcos );
            fChain->SetBranchAddress( "MCycos", &MCycos );
        }
        else
        {
            MCxcore_SC = MCycore_SC = MCxcos = MCycos = 0.;
        }
        fChain->SetBranchAddress( "MCaz", &MCaz );
        fChain->SetBranchAddress( "MCze", &MCze );
        fChain->SetBranchAddress( "MCxoff", &MCxoff );
        fChain->SetBranchAddress( "MCyoff", &MCyoff );
    }
    else
    {
        MCprimary = -99;
        MCe0 = 0.;
        MCxcore = 0.;
        MCycore = 0.;
        MCxcore_SC = 0.;
        MCycore_SC = 0.;
        MCxcos = 0.;
        MCycos = 0.;
        MCaz = 0.;
        MCze = 0.;
        MCxoff = 0.;
        MCyoff = 0.;
    }


    if( fVersion < 6 )
    {
        fChain->SetBranchAddress( "LTrig", &LTrigS );
    }
    else
    {
        fChain->SetBranchAddress( "LTrig", &LTrig );
    }
    fChain->SetBranchAddress( "NTrig", &NTrig );
    fChain->SetBranchAddress( "NImages", &NImages );
    if( fVersion < 6 )
    {
        fChain->SetBranchAddress( "ImgSel", &ImgSelS );
    }
    else
    {
        fChain->SetBranchAddress( "ImgSel", &ImgSel );
    }
    fChain->SetBranchAddress( "img2_ang", &img2_ang );
    fChain->SetBranchAddress( "Ze", &Ze );
    fChain->SetBranchAddress( "Az", &Az );
    if(!fShort )
    {
        fChain->SetBranchAddress( "ra", &ra );
        fChain->SetBranchAddress( "dec", &dec );
    }
    else
    {
        ra = dec = 0.;
    }
    fChain->SetBranchAddress( "Xoff", &Xoff );
    fChain->SetBranchAddress( "Yoff", &Yoff );
    if( fChain->GetBranchStatus( "Xoff_derot" ) )
    {
        fChain->SetBranchAddress( "Xoff_derot", &Xoff_derot );
    }
    if( fChain->GetBranchStatus( "Yoff_derot" ) )
    {
        fChain->SetBranchAddress( "Yoff_derot", &Yoff_derot );
    }

    if(!fShort )
    {
        fChain->SetBranchAddress( "stdS", &stdS );
        fChain->SetBranchAddress( "theta2", &theta2 );
    }
    else
    {
        stdS = 0.;
        theta2 = 0.;
    }
    fChain->SetBranchAddress( "Xcore", &Xcore );
    fChain->SetBranchAddress( "Ycore", &Ycore );
    if(!fShort )
    {
        fChain->SetBranchAddress( "Xcore_SC", &Xcore_SC );
        fChain->SetBranchAddress( "Ycore_SC", &Ycore_SC );
    }
    else
    {
        Xcore_SC = Ycore_SC = 0.;
    }
    fChain->SetBranchAddress( "stdP", &stdP );
    fChain->SetBranchAddress( "Chi2", &Chi2 );
    if( fVersion > 4 )
    {
        fChain->SetBranchAddress( "meanPedvar_Image", &meanPedvar_Image );
        if(!fShort )
        {
            fChain->SetBranchAddress( "meanPedvar_ImageT", meanPedvar_ImageT );
        }
        else
        {
            for( unsigned int i = 0; i < VDST_MAXTELESCOPES; i++ )
            {
                meanPedvar_ImageT[i] = 0.;
            }
        }

    }
    else
    {
        meanPedvar_Image = 0.;
        for( int i = 0; i < VDST_MAXTELESCOPES; i++ )
        {
            meanPedvar_ImageT[i] = 0.;
        }
    }

    fChain->SetBranchAddress( "SizeSecondMax", &SizeSecondMax );

    if( fBOOLtheta2_All )
    {
        fChain->SetBranchAddress( "theta2_All", &theta2_All );
    }
    else
    {
        for( unsigned int dex = 0; dex < 25; dex++ )
        {
            theta2_All[dex] = 99.0;
        }
    }

    if( fBOOLteltype )
    {
        fChain->SetBranchAddress( "ImgSel_list", ImgSel_list );
        fChain->SetBranchAddress( "NTtype", &NTtype );
        fChain->SetBranchAddress( "NImages_Ttype", NImages_Ttype );
    }
    else
    {
        NTtype = 0;
        for( unsigned int tt = 0; tt < VDST_MAXTELESCOPES; tt++ )
        {
            NImages_Ttype[tt] = 0;
        }
    }

    if(!fShort )
    {
        fChain->SetBranchAddress( "dist", dist );
        fChain->SetBranchAddress( "size", size );
        if( fChain->GetBranchStatus( "fracLow" ) )
        {
            fChain->SetBranchAddress( "fracLow", fraclow );
        }
        else
        {
            for( int i = 0; i < VDST_MAXTELESCOPES; i++ )
            {
                fraclow[i] = 0.;
            }
        }
        if( fVersion > 2 )
        {
            fChain->SetBranchAddress( "loss", loss );
        }
        else
        {
            for( int i = 0; i < VDST_MAXTELESCOPES; i++ )
            {
                loss[i] = 0.;
            }
        }

        fChain->SetBranchAddress( "max1", max1 );
        fChain->SetBranchAddress( "max2", max2 );
        fChain->SetBranchAddress( "max3", max3 );
        fChain->SetBranchAddress( "maxindex1", maxindex1 );
        fChain->SetBranchAddress( "maxindex2", maxindex2 );
        fChain->SetBranchAddress( "maxindex3", maxindex3 );
        fChain->SetBranchAddress( "width", width );
        fChain->SetBranchAddress( "length", length );
        fChain->SetBranchAddress( "ntubes", ntubes );
        if( fVersion > 2 )
        {
            fChain->SetBranchAddress( "nsat", nsat );
        }
        else
        {
            for( int i = 0; i < VDST_MAXTELESCOPES; i++ )
            {
                nsat[i] = 0;
            }
        }
        if( fChain->GetBranchStatus( "nlowgain" ) )
        {
            fChain->SetBranchAddress( "nlowgain", nlowgain );
        }
        else
        {
            for( int i = 0; i < VDST_MAXTELESCOPES; i++ )
            {
                nlowgain[i] = 0;
            }
        }
        if( fChain->GetBranchStatus( "ntubesBNI" ) )
        {
            fChain->SetBranchAddress( "ntubesBNI", ntubesBNI );
        }
        else
        {
            for( int i = 0; i < VDST_MAXTELESCOPES; i++ )
            {
                ntubesBNI[i] = 0;
            }
        }
        fChain->SetBranchAddress( "alpha", alpha );
        fChain->SetBranchAddress( "los", los );
        fChain->SetBranchAddress( "asym", asym );
        fChain->SetBranchAddress( "cen_x", cen_x );
        fChain->SetBranchAddress( "cen_y", cen_y );
        fChain->SetBranchAddress( "cosphi", cosphi );
        fChain->SetBranchAddress( "sinphi", sinphi );
        fChain->SetBranchAddress( "tgrad_x", tgrad_x );
        fChain->SetBranchAddress( "Fitstat", Fitstat );
        fChain->SetBranchAddress( "DispXoff_T", DispXoff_T );
        fChain->SetBranchAddress( "DispYoff_T", DispYoff_T );
        fChain->SetBranchAddress( "DispWoff_T", DispWoff_T );
        fChain->SetBranchAddress( "Disp_T", Disp_T );
        fChain->SetBranchAddress( "tchisq_x", tchisq_x );
    }
    else
    {
        for( int i = 0; i < VDST_MAXTELESCOPES; i++ )
        {
            dist[i] = 0.;
            size[i] = 0.;
            fraclow[i] = 0.;
            loss[i] = 0.;
            max1[i] = 0.;
            max2[i] = 0.;
            max3[i] = 0.;
            maxindex1[i] = 0;
            maxindex2[i] = 0;
            maxindex3[i] = 0;
            width[i] = 0.;
            length[i] = 0.;
            ntubes[i] = 0;
            nsat[i] = 0;
            nlowgain[i] = 0;
            ntubesBNI[i] = 0;
            alpha[i] = 0.;
            los[i] = 0.;
            asym[i] = 0.;
            cen_x[i] = 0.;
            cen_y[i] = 0.;
            cosphi[i] = 0.;
            sinphi[i] = 0.;
            tgrad_x[i] = 0.;
            Fitstat[i] = 0;
            DispXoff_T[i] = 0.;
            DispYoff_T[i] = 0.;
            DispWoff_T[i] = 0.;
            Disp_T[i] = 0.;
            tchisq_x[i] = 0.;
        }
    }
    fChain->SetBranchAddress( "R", R );
    if(!fShort )
    {
        fChain->SetBranchAddress( "MSCWT", MSCWT );
        fChain->SetBranchAddress( "MSCLT", MSCLT );
    }
    else
    {
        for( int i = 0; i < VDST_MAXTELESCOPES; i++ )
        {
            MSCWT[i] = 0.;
            MSCLT[i] = 0.;
        }
    }
    if(!fShort )
    {
        fChain->SetBranchAddress( "ES", ES );
        fChain->SetBranchAddress( "E", E );
    }
    else
    {
        for( int i = 0; i < VDST_MAXTELESCOPES; i++ )
        {
            ES[i] = 0.;
            E[i] = 0.;
        }
    }
    if(!fShort )
    {
        fChain->SetBranchAddress( "NMSCW", &NMSCW );
    }
    else
    {
        NMSCW = 0;
    }
    fChain->SetBranchAddress( "MSCW", &MSCW );
    fChain->SetBranchAddress( "MSCL", &MSCL );
    if( fVersion > 3 )
    {
        fChain->SetBranchAddress( "MWR", &MWR );
        fChain->SetBranchAddress( "MLR", &MLR );
    }
    else
    {
        MWR = 0.;
        MLR = 0.;
    }
    fChain->SetBranchAddress( "Erec", &Erec );
    fChain->SetBranchAddress( "EChi2", &EChi2 );
    fChain->SetBranchAddress( "ErecS", &ErecS );
    fChain->SetBranchAddress( "EChi2S", &EChi2S );
    if( fBOOLdE )
    {
        fChain->SetBranchAddress( "dE", &dE );
        fChain->SetBranchAddress( "dES", &dES );
    }
    else
    {
        dE = 0.;
        dES = 0.;
    }
    if( fVersion > 3 )
    {
        EmissionHeight = -99.;
        fChain->SetBranchAddress( "EmissionHeight", &EmissionHeight );
        fChain->SetBranchAddress( "EmissionHeightChi2", &EmissionHeightChi2 );
        fChain->SetBranchAddress( "NTelPairs", &NTelPairs );
        if(!fShort )
        {
            fChain->SetBranchAddress( "EmissionHeightT", EmissionHeightT );
        }
        else
        {
            for( unsigned int i = 0; i < VDST_MAXTELESCOPES * VDST_MAXTELESCOPES; i++ )
            {
                EmissionHeightT[i] = 0.;
            }
        }
    }
    else
    {
        EmissionHeight = -999.;
        EmissionHeightChi2 = -999.;
        NTelPairs = 0;
        for( unsigned int i = 0; i < VDST_MAXTELESCOPES * VDST_MAXTELESCOPES; i++ )
        {
            EmissionHeightT[i] = 0.;
        }
    }
    if( fChain->GetBranchStatus( "DispDiff" ) )
    {
        fChain->SetBranchAddress( "DispDiff", &DispDiff );
    }
    else
    {
        DispDiff = 0.;
    }
    if( fChain->GetBranchStatus( "DispAbsSumWeigth" ) )
    {
        fChain->SetBranchAddress( "DispAbsSumWeigth", &DispAbsSumWeigth );
    }
    else
    {
        DispAbsSumWeigth = 0.;
    }
    if( fChain->GetBranchStatus( "Xoff_intersect" ) )
    {
        fChain->SetBranchAddress( "Xoff_intersect", &Xoff_intersect );
    }
    else
    {
        Xoff_intersect = 0.;
    }
    if( fChain->GetBranchStatus( "Yoff_intersect" ) )
    {
        fChain->SetBranchAddress( "Yoff_intersect", &Yoff_intersect );
    }
    else
    {
        Yoff_intersect = 0.;
    }

    Notify();
}


Bool_t CData::Notify()
{
    // The Notify() function is called when a new file is opened. This
    // can be either for a new TTree in a TChain or when when a new TTree
    // is started when using PROOF. Typically here the branch pointers
    // will be retrieved. It is normally not necessary to make changes
    // to the generated code, but the routine can be extended by the
    // user if needed.

    // Get branch pointers
    b_runNumber = fChain->GetBranch( "runNumber" );
    b_eventNumber = fChain->GetBranch( "eventNumber" );
    b_MJD = fChain->GetBranch( "MJD" );
    b_Time = fChain->GetBranch( "Time" );
    b_TelElevation = fChain->GetBranch( "TelElevation" );
    b_TelAzimuth = fChain->GetBranch( "TelAzimuth" );
    b_TelDec = fChain->GetBranch( "TelDec" );
    b_TelRA = fChain->GetBranch( "TelRA" );
    b_ArrayPointing_Elevation = fChain->GetBranch( "ArrayPointing_Elevation" );
    b_ArrayPointing_Azimuth = fChain->GetBranch( "ArrayPointing_Azimuth" );

    if( fMC )
    {
        b_MCprimary = fChain->GetBranch( "MCprimary" );
        b_MCe0 = fChain->GetBranch( "MCe0" );
        b_MCxcore  = fChain->GetBranch( "MCxcore" );
        b_MCycore  = fChain->GetBranch( "MCycore" );
        b_MCxcore_SC  = fChain->GetBranch( "MCxcore_SC" );
        b_MCycore_SC  = fChain->GetBranch( "MCycore_SC" );
        b_MCxcos = fChain->GetBranch( "MCxcos" );
        b_MCycos = fChain->GetBranch( "MCycos" );
        b_MCaz = fChain->GetBranch( "MCaz" );
        b_MCze = fChain->GetBranch( "MCze" );
        b_MCxoff = fChain->GetBranch( "MCxoff" );
        b_MCyoff = fChain->GetBranch( "MCyoff" );
    }

    b_LTrig = fChain->GetBranch( "LTrig" );
    b_NTrig = fChain->GetBranch( "NTrig" );
    b_NImages = fChain->GetBranch( "NImages" );
    b_ImgSel = fChain->GetBranch( "ImgSel" );
    b_img2_ang = fChain->GetBranch( "img2_ang" );
    b_Ze = fChain->GetBranch( "Ze" );
    b_Az = fChain->GetBranch( "Az" );
    b_ra = fChain->GetBranch( "ra" );
    b_dec = fChain->GetBranch( "dec" );
    b_Xoff_derot = fChain->GetBranch( "Xoff_derot" );
    b_Yoff_derot = fChain->GetBranch( "Yoff_derot" );
    b_Xoff = fChain->GetBranch( "Xoff" );
    b_Yoff = fChain->GetBranch( "Yoff" );
    b_stdS = fChain->GetBranch( "stdS" );
    b_theta2 = fChain->GetBranch( "theta2" );
    b_Xcore = fChain->GetBranch( "Xcore" );
    b_Ycore = fChain->GetBranch( "Ycore" );
    b_Xcore_SC = fChain->GetBranch( "Xcore_SC" );
    b_Ycore_SC = fChain->GetBranch( "Ycore_SC" );
    b_stdP = fChain->GetBranch( "stdP" );
    b_Chi2 = fChain->GetBranch( "Chi2" );
    if( fVersion > 4 )
    {
        b_meanPedvar_Image = fChain->GetBranch( "meanPedvar_Image" );
        b_meanPedvar_ImageT = fChain->GetBranch( "meanPedvar_ImageT" );
    }
    else
    {
        b_meanPedvar_Image = 0;
        b_meanPedvar_ImageT = 0;
    }

    b_SizeSecondMax = fChain->GetBranch( "SizeSecondMax" );

    if( fBOOLtheta2_All )
    {
        b_theta2_All = fChain->GetBranch( "theta2_All" );
    }
    else
    {
        b_theta2_All = 0;
    }

    b_dist = fChain->GetBranch( "dist" );
    b_size = fChain->GetBranch( "size" );
    b_fraclow = fChain->GetBranch( "fraclow" );
    b_max1 = fChain->GetBranch( "max1" );
    b_max2 = fChain->GetBranch( "max2" );
    b_max3 = fChain->GetBranch( "max3" );
    b_maxindex1 = fChain->GetBranch( "maxindex1" );
    b_maxindex2 = fChain->GetBranch( "maxindex2" );
    b_maxindex3 = fChain->GetBranch( "maxindex3" );
    b_width = fChain->GetBranch( "width" );
    b_length = fChain->GetBranch( "length" );
    b_ntubes = fChain->GetBranch( "ntubes" );
    b_ntubesBNI = fChain->GetBranch( "ntubesBNI" );
    b_alpha = fChain->GetBranch( "alpha" );
    b_los = fChain->GetBranch( "los" );
    b_asym = fChain->GetBranch( "asym" );
    b_cen_x = fChain->GetBranch( "cen_x" );
    b_cen_y = fChain->GetBranch( "cen_y" );
    b_cosphi = fChain->GetBranch( "cosphi" );
    b_sinphi = fChain->GetBranch( "sinphi" );
    b_tgrad_x = fChain->GetBranch( "tgrad_x" );
    b_Fitstat = fChain->GetBranch( "Fitstat" );
    b_tchisq_x = fChain->GetBranch( "tchisq_x" );
    b_R = fChain->GetBranch( "R" );
    b_MSCWT = fChain->GetBranch( "MSCWT" );
    b_MSCLT = fChain->GetBranch( "MSCLT" );
    b_E = fChain->GetBranch( "E" );
    b_ES = fChain->GetBranch( "ES" );
    b_NMSCW = fChain->GetBranch( "NMSCW" );
    b_MSCW = fChain->GetBranch( "MSCW" );
    b_MSCL = fChain->GetBranch( "MSCL" );
    if( fVersion > 3 )
    {
        b_MWR = fChain->GetBranch( "MWR" );
        b_MLR = fChain->GetBranch( "MLR" );
    }
    else
    {
        b_MWR = 0;
        b_MLR = 0;
    }
    b_Erec = fChain->GetBranch( "Erec" );
    b_EChi2 = fChain->GetBranch( "EChi2" );
    b_ErecS = fChain->GetBranch( "ErecS" );
    b_EChi2S = fChain->GetBranch( "EChi2S" );
    if( fVersion > 3 )
    {
        b_EmissionHeight = fChain->GetBranch( "EmissionHeight" );
        b_EmissionHeightChi2 = fChain->GetBranch( "EmissionHeightChi2" );
        b_NTelPairs = fChain->GetBranch( "NTelPairs" );
        b_EmissionHeightT = fChain->GetBranch( "EmissionHeightT" );
    }
    else
    {
        b_EmissionHeight = 0;
        b_EmissionHeightChi2 = 0;
        b_NTelPairs = 0;
        b_EmissionHeightT = 0;
    }
    if( fChain->GetBranchStatus( "DispDiff" ) )
    {
        b_DispDiff = fChain->GetBranch( "DispDiff" );
    }
    else
    {
        b_DispDiff = 0;
    }
    if( fChain->GetBranchStatus( "DispAbsSumWeigth" ) )
    {
        b_DispAbsSumWeigth = fChain->GetBranch( "DispAbsSumWeigth" );
    }
    else
    {
        b_DispAbsSumWeigth = 0;
    }
    if( fChain->GetBranchStatus( "Xoff_intersect" ) )
    {
        b_Xoff_intersect = fChain->GetBranch( "Xoff_intersect" );
    }
    else
    {
        b_Xoff_intersect = 0;
    }
    if( fChain->GetBranchStatus( "Yoff_intersect" ) )
    {
        b_Yoff_intersect = fChain->GetBranch( "Yoff_intersect" );
    }
    else
    {
        b_Yoff_intersect = 0;
    }

    return kTRUE;
}


void CData::Show( Long64_t entry )
{
    // Print contents of entry.
    // If entry is not specified, print current entry
    if(!fChain )
    {
        return;
    }
    fChain->Show( entry );
}


Int_t CData::Cut( Long64_t entry )
{
    // This function may be called from Loop.
    // returns  1 if entry is accepted.
    // returns -1 otherwise.
    entry = 0;

    return 1;
}
#endif                                            // #ifdef CData_cxx
