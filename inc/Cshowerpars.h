//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Feb  8 14:35:23 2007 by ROOT version 5.10/00
// from TTree showerpars/Shower Parameters
// found on file: output/32855.root
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//
//   Adjusted to mscw_energy
//
//   DO NOT OVERWRITE BY DOING SIMPLY A showerpars->MakeClass !!!!
//
//   Revision $Id: Cshowerpars.h,v 1.1.2.2.16.1.2.1.4.1.4.1.10.3 2010/03/08 08:00:48 gmaier Exp $
//
//
//   (GM)
//
////////////////////////////////////////////

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
        int             fVersion;

        TTree          *fChain;                   //!pointer to the analyzed TTree or TChain
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
        UInt_t          LTrigS;
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
        UInt_t          ImgSelS[VDST_MAXRECMETHODS];
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
        TBranch        *b_runNumber;              //!
        TBranch        *b_eventNumber;            //!
        TBranch        *b_MJD;                    //!
        TBranch        *b_Time;                   //!
        TBranch        *b_dataFormat;             //!
        TBranch        *b_NTel;                   //!
        TBranch        *b_traceFit;               //!
        TBranch        *b_TelElevation;           //!
        TBranch        *b_TelAzimuth;             //!
        TBranch        *b_TelElevationVBF;        //!
        TBranch        *b_TelAzimuthVBF;          //!
        TBranch        *b_TelPointingMismatch;    //!
        TBranch        *b_TelDec;                 //!
        TBranch        *b_TelRA;                  //!
        TBranch        *b_Tel_x_SC;               //!
        TBranch        *b_Tel_y_SC;               //!
        TBranch        *b_Tel_z_SC;               //!
        TBranch        *b_TargetElev;             //!
        TBranch        *b_TargetAzim;             //!
        TBranch        *b_TargetDec;              //!
        TBranch        *b_TargetRA;               //!
        TBranch        *b_WobbleN;                //!
        TBranch        *b_WobbleE;                //!
        TBranch        *b_NTrig;                  //!
        TBranch        *b_LTrig;                  //!
        TBranch        *b_Trig_list;              //!
        TBranch        *b_Trig_type;              //!
        TBranch        *b_NMethods;               //!
        TBranch        *b_MethodID;               //!
        TBranch        *b_NImages;                //!
        TBranch        *b_ImgSel;                 //!
        TBranch        *b_ImgSel_list;            //!
        TBranch        *b_img2_ang;               //!
        TBranch        *b_Ze;                     //!
        TBranch        *b_Az;                     //!
        TBranch        *b_Xoff;                   //!
        TBranch        *b_Yoff;                   //!
        TBranch        *b_XoffDeRot;                   //!
        TBranch        *b_YoffDeRot;                   //!
        TBranch        *b_stds;                   //!
        TBranch        *b_dec;                    //!
        TBranch        *b_ra;                     //!
        TBranch        *b_Xcore;                  //!
        TBranch        *b_Ycore;                  //!
        TBranch        *b_Xcore_SC;               //!
        TBranch        *b_Ycore_SC;               //!
        TBranch        *b_stdp;                   //!
        TBranch        *b_Chi2;                   //!
        TBranch        *b_MCprim;                 //!
        TBranch        *b_MCe0;                   //!
        TBranch        *b_MCxcore;                //!
        TBranch        *b_MCycore;                //!
        TBranch        *b_MCxcos;                 //!
        TBranch        *b_MCycos;                 //!
        TBranch        *b_MCze;                   //!
        TBranch        *b_MCaz;                   //!
        TBranch        *b_MCxoff;                 //!
        TBranch        *b_MCyoff;                 //!
        TBranch        *b_MCxcore_SC;             //!
        TBranch        *b_MCycore_SC;             //!
        TBranch        *b_MCzcore_SC;             //!

        Cshowerpars(TTree *tree = 0, bool iMC = false, int iVersion = 2, bool iShort = false );
        virtual ~Cshowerpars();
        virtual Int_t    Cut(Long64_t entry);
        virtual Int_t    GetEntry(Long64_t entry);
        int              getTreeVersion() { return fVersion; }
        virtual Long64_t LoadTree(Long64_t entry);
        virtual void     Init(TTree *tree);
        virtual void     Loop();
        virtual Bool_t   Notify();
        virtual void     Show(Long64_t entry = -1);
        bool             isMC() { return bMC; }
        bool             isShort() { return bShort; }
};
#endif

#ifdef Cshowerpars_cxx

Cshowerpars::Cshowerpars(TTree *tree, bool iMC, int iVersion, bool iShort )
{
    if( !tree ) return;

    bMC = iMC;
    bDeRot = false;
    bShort = iShort;
    fVersion = iVersion;

    Init(tree);
}


Cshowerpars::~Cshowerpars()
{
    if (!fChain) return;
    delete fChain->GetCurrentFile();
}


Int_t Cshowerpars::GetEntry(Long64_t entry)
{
// Read contents of entry.
    if (!fChain) return 0;

    int a = fChain->GetEntry(entry);

    if( a > 0 && fVersion < 6 )
    {
        LTrig = (ULong64_t)LTrigS;
        for( unsigned int i = 0; i < VDST_MAXRECMETHODS; i++ ) ImgSel[i] = (ULong64_t)ImgSelS[i];
    }
    return a;
}


Long64_t Cshowerpars::LoadTree(Long64_t entry)
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


void Cshowerpars::Init(TTree *tree)
{

// Set branch addresses
    if (tree == 0) return;
    fChain = tree;
    fCurrent = -1;
    fChain->SetMakeClass(1);

    if( tree->GetBranchStatus( "MCe0" ) ) bMC = true;
    if( tree->GetBranchStatus( "XoffDeRot" ) ) bDeRot = true;

    fChain->SetBranchAddress("runNumber",&runNumber);
    fChain->SetBranchAddress("eventNumber",&eventNumber);
    fChain->SetBranchAddress("MJD",&MJD);
    fChain->SetBranchAddress("Time",&Time);
    if( !bShort ) fChain->SetBranchAddress("dataFormat",&dataFormat);
    else          dataFormat = 0;
    fChain->SetBranchAddress("NTel",&NTel);
    if( !bShort ) fChain->SetBranchAddress("traceFit",&traceFit);
    else          traceFit = 0;
    fChain->SetBranchAddress("TelElevation",TelElevation);
    fChain->SetBranchAddress("TelAzimuth",TelAzimuth);
    if( !bShort )
    {
        fChain->SetBranchAddress("TelElevationVBF",TelElevationVBF);
        fChain->SetBranchAddress("TelAzimuthVBF",TelAzimuthVBF);
        fChain->SetBranchAddress("TelPointingMismatch",TelPointingMismatch);
        fChain->SetBranchAddress("TelDec", TelDec );
        fChain->SetBranchAddress("TelRA", TelRA );
        fChain->SetBranchAddress("Tel_x_SC",Tel_x_SC);
        fChain->SetBranchAddress("Tel_y_SC",Tel_y_SC);
        fChain->SetBranchAddress("Tel_z_SC",Tel_z_SC);
        fChain->SetBranchAddress("TargetElev", &TargetElev );
        fChain->SetBranchAddress("TargetAzim", &TargetAzim );
        fChain->SetBranchAddress("TargetDec", &TargetDec );
        fChain->SetBranchAddress("TargetRA", &TargetRA );
    }
    else
    {
        for( unsigned int i = 0; i < VDST_MAXTELESCOPES; i++ )
        {
            TelElevationVBF[i] = 0.;
            TelAzimuthVBF[i] = 0.;
            TelPointingMismatch[i] = 0.;
            TelDec[i] = 0.;
            TelRA[i] = 0;
            Tel_x_SC[i] = 0.;
            Tel_y_SC[i] = 0.;
            Tel_z_SC[i] = 0.;
        }
        TargetElev = 0.;
        TargetAzim = 0.;
        TargetDec = 0.;
        TargetRA = 0;
    }
    fChain->SetBranchAddress("WobbleN", &WobbleN );
    fChain->SetBranchAddress("WobbleE", &WobbleE );
    fChain->SetBranchAddress("NTrig",&NTrig);
    if( fVersion < 6 ) fChain->SetBranchAddress("LTrig",&LTrigS );
    else               fChain->SetBranchAddress("LTrig",&LTrig );
    if( fVersion > 6 ) fChain->SetBranchAddress("Trig_list", Trig_list, &b_Trig_list);
    else
    {
        for( unsigned int i = 0; i < VDST_MAXTELESCOPES; i++ ) Trig_list[i] = 0;
    }
    if( fChain->GetBranchStatus( "Trig_type" ) ) fChain->SetBranchAddress("Trig_type", Trig_type, &b_Trig_type );
    else
    {
       for( unsigned int i = 0; i < VDST_MAXTELESCOPES; i++ ) Trig_type[i] = 0;
    }
    fChain->SetBranchAddress("NMethods", &NMethods );
    if( !bShort ) fChain->SetBranchAddress("MethodID", MethodID );
    else
    {
        for( unsigned int i = 0; i < VDST_MAXRECMETHODS; i++ ) MethodID[i] = 0;
    }

    fChain->SetBranchAddress("NImages",NImages);
    fChain->SetBranchAddress("img2_ang", img2_ang );
    if( fVersion < 6 ) fChain->SetBranchAddress("ImgSel",ImgSelS);
    else               fChain->SetBranchAddress("ImgSel",ImgSel);
    if( fVersion > 6 ) fChain->SetBranchAddress("ImgSel_list", ImgSel_list, &b_ImgSel_list);
    else
    {
        for( unsigned int i = 0; i < VDST_MAXRECMETHODS; i++ )
        {
            for( unsigned int j = 0; j < VDST_MAXTELESCOPES; j++ ) ImgSel_list[i][j] = 0;
        }
    }
    fChain->SetBranchAddress("Ze",Ze);
    fChain->SetBranchAddress("Az",Az);
    fChain->SetBranchAddress("Xoff",Xoff);
    fChain->SetBranchAddress("Yoff",Yoff);
    if( bDeRot )
    {
       fChain->SetBranchAddress("XoffDeRot",XoffDeRot);
       fChain->SetBranchAddress("YoffDeRot",YoffDeRot);
    }
    else
    {
       for( unsigned int i = 0; i < VDST_MAXRECMETHODS; i++ )
       {
	  XoffDeRot[i] = 0.;
	  YoffDeRot[i] = 0.;
       }
    }
    if( !bShort )
    {
        fChain->SetBranchAddress("stds",stds);
        fChain->SetBranchAddress("dec",dec);
        fChain->SetBranchAddress("ra",ra);
    }
    else
    {
        for( unsigned int i = 0; i < VDST_MAXRECMETHODS; i++ )
        {
            stds[i] = 0.;
            dec[i] = 0.;
            ra[i] = 0.;
        }
    }
    fChain->SetBranchAddress("Xcore",Xcore);
    fChain->SetBranchAddress("Ycore",Ycore);
    if( !bShort )
    {
        fChain->SetBranchAddress("Xcore_SC",Xcore_SC);
        fChain->SetBranchAddress("Ycore_SC",Ycore_SC);
        fChain->SetBranchAddress("stdp",stdp);
    }
    else
    {
        for( unsigned int i = 0; i < VDST_MAXRECMETHODS; i++ )
        {
            Xcore_SC[i] = 0.;
            Ycore_SC[i] = 0.;
            stdp[i] = 0.;
        }
    }
    fChain->SetBranchAddress("Chi2",Chi2);

    if( bMC )
    {
	if( fVersion > 7 ) fChain->SetBranchAddress("MCprim", &MCprim );
        else               MCprim = 0;
        fChain->SetBranchAddress("MCe0", &MCe0 );
        fChain->SetBranchAddress("MCxcore", &MCxcore );
        fChain->SetBranchAddress("MCycore", &MCycore );
        if( !bShort )
        {
            fChain->SetBranchAddress("MCxcos", &MCxcos );
            fChain->SetBranchAddress("MCycos", &MCycos );
        }
        else
        {
            MCxcos = 0.;
            MCycos = 0.;
        }
        fChain->SetBranchAddress("MCze", &MCze );
        fChain->SetBranchAddress("MCaz", &MCaz );
        fChain->SetBranchAddress("MCxoff", &MCxoff );
        fChain->SetBranchAddress("MCyoff", &MCyoff );
        if( !bShort )
        {
            fChain->SetBranchAddress("MCxcore_SC", &MCxcore_SC );
            fChain->SetBranchAddress("MCycore_SC", &MCycore_SC );
            fChain->SetBranchAddress("MCzcore_SC", &MCzcore_SC );
        }
        else
        {
            MCxcore_SC = 0.;
            MCycore_SC = 0.;
            MCzcore_SC = 0.;
        }
    }
    Notify();
}


Bool_t Cshowerpars::Notify()
{
    b_runNumber = fChain->GetBranch("runNumber");
    b_eventNumber = fChain->GetBranch("eventNumber");
    b_MJD = fChain->GetBranch("MJD");
    b_Time = fChain->GetBranch("Time");
    if( !bShort ) b_dataFormat = fChain->GetBranch("dataFormat");
    else          b_dataFormat = 0;
    b_NTel = fChain->GetBranch("NTel");
    b_traceFit = fChain->GetBranch("traceFit");
    b_TelElevation = fChain->GetBranch("TelElevation");
    b_TelAzimuth = fChain->GetBranch("TelAzimuth");
    if( !bShort )
    {
        b_TelElevationVBF = fChain->GetBranch("TelElevationVBF");
        b_TelAzimuthVBF = fChain->GetBranch("TelAzimuthVBF");
        b_TelPointingMismatch = fChain->GetBranch("TelPointingMismatch");
        b_TelDec = fChain->GetBranch("TelDec");
        b_TelRA = fChain->GetBranch("TelRA");
        b_Tel_x_SC = fChain->GetBranch("Tel_x_SC");
        b_Tel_y_SC = fChain->GetBranch("Tel_y_SC");
        b_Tel_z_SC = fChain->GetBranch("Tel_z_SC");
        b_TargetElev = fChain->GetBranch("TargetElev");
        b_TargetAzim = fChain->GetBranch("TargetAzim");
        b_TargetDec = fChain->GetBranch("TargetDec");
        b_TargetRA = fChain->GetBranch("TargetRA");
    }
    else
    {
        b_TelElevationVBF = 0;
        b_TelAzimuthVBF = 0;
        b_TelPointingMismatch = 0;
        b_TelDec = 0;
        b_TelRA = 0;
        b_Tel_x_SC = 0;
        b_Tel_y_SC = 0;
        b_Tel_z_SC = 0;
        b_TargetElev = 0;
        b_TargetAzim = 0;
        b_TargetDec = 0;
        b_TargetRA = 0;
    }
    b_WobbleN = fChain->GetBranch("WobbleN");
    b_WobbleE = fChain->GetBranch("WobbleE");
    b_NTrig = fChain->GetBranch("NTrig");
    b_LTrig = fChain->GetBranch("LTrig");
    b_NMethods = fChain->GetBranch("NMethods" );
    if( !bShort ) b_MethodID = fChain->GetBranch("MethodID" );
    else          b_MethodID = 0;
    b_NImages = fChain->GetBranch("NImages");
    b_ImgSel = fChain->GetBranch("ImgSel");
    b_ImgSel_list = fChain->GetBranch( "ImgSel_list" );
    if( !bShort ) b_img2_ang = fChain->GetBranch("img2_ang");
    else          b_img2_ang = 0;
    b_Ze = fChain->GetBranch("Ze");
    b_Az = fChain->GetBranch("Az");
    b_Xoff = fChain->GetBranch("Xoff");
    b_Yoff = fChain->GetBranch("Yoff");
    if( bDeRot )
    {
       b_XoffDeRot = fChain->GetBranch("XoffDeRot");
       b_YoffDeRot = fChain->GetBranch("YoffDeRot");
    }
    else
    {
       b_XoffDeRot = 0;
       b_YoffDeRot = 0;
    }
    if( !bShort )
    {
        b_stds = fChain->GetBranch("stds");
        b_dec = fChain->GetBranch("dec");
        b_ra = fChain->GetBranch("ra");
    }
    else
    {
        b_stds = 0;
        b_dec = 0;
        b_ra = 0;
    }
    b_Xcore = fChain->GetBranch("Xcore");
    b_Ycore = fChain->GetBranch("Ycore");
    if( !bShort )
    {
        b_Xcore_SC = fChain->GetBranch("Xcore_SC");
        b_Ycore_SC = fChain->GetBranch("Ycore_SC");
        b_stdp = fChain->GetBranch("stdp");
    }
    else
    {
        b_Xcore_SC = 0;
        b_Ycore_SC = 0;
        b_stdp = 0;
    }
    b_Chi2 = fChain->GetBranch("Chi2");
    if( bMC )
    {
        if( !bShort ) b_MCprim = fChain->GetBranch("MCprim");
        else          b_MCprim = 0;
        b_MCe0 = fChain->GetBranch("MCe0");
        b_MCxcore = fChain->GetBranch("MCxcore");
        b_MCycore = fChain->GetBranch("MCycore");
        if( !bShort )
        {
            b_MCxcos = fChain->GetBranch("MCxcos");
            b_MCycos = fChain->GetBranch("MCycos");
        }
        else
        {
            b_MCxcos = 0;
            b_MCycos = 0;
        }
        b_MCze = fChain->GetBranch("MCze");
        b_MCaz = fChain->GetBranch("MCaz");
        b_MCxoff = fChain->GetBranch("MCxoff");
        b_MCyoff = fChain->GetBranch("MCyoff");
        if( !bShort )
        {
            b_MCxcore_SC = fChain->GetBranch("MCycore_SC");
            b_MCycore_SC = fChain->GetBranch("MCycore_SC");
            b_MCycore_SC = fChain->GetBranch("MCycore_SC");
        }
        else
        {
            b_MCxcore_SC = 0;
            b_MCycore_SC = 0;
            b_MCycore_SC = 0;
        }
    }

    return kTRUE;
}


void Cshowerpars::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
    if (!fChain) return;
    fChain->Show(entry);
}


Int_t Cshowerpars::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
    return 1;
}
#endif                                            // #ifdef Cshowerpars_cxx
