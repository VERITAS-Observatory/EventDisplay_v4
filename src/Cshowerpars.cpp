/*
   Showerpars tree definition.
*/

#include "Cshowerpars.h"

Cshowerpars::Cshowerpars( TTree* tree, bool iMC, bool iShort )
{
    if(!tree )
    {
        return;
    }

    bMC = iMC;
    bDeRot = false;
    bShort = iShort;

    Init( tree );
}


Cshowerpars::~Cshowerpars()
{
    if(!fChain )
    {
        return;
    }
    delete fChain->GetCurrentFile();
}


Int_t Cshowerpars::GetEntry( Long64_t entry )
{
    // Read contents of entry.
    if(!fChain )
    {
        return 0;
    }

    return fChain->GetEntry( entry );
}


Long64_t Cshowerpars::LoadTree( Long64_t entry )
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


void Cshowerpars::Init( TTree* tree )
{

    // Set branch addresses
    if( tree == 0 )
    {
        return;
    }
    fChain = tree;
    fCurrent = -1;
    fChain->SetMakeClass( 1 );

    if( tree->GetBranchStatus( "MCe0" ) )
    {
        bMC = true;
    }
    if( tree->GetBranchStatus( "XoffDeRot" ) )
    {
        bDeRot = true;
    }

    fChain->SetBranchAddress( "runNumber", &runNumber );
    fChain->SetBranchAddress( "eventNumber", &eventNumber );
    if( fChain->GetBranchStatus( "eventStatus" ) )
    {
        fChain->SetBranchAddress( "eventStatus", &eventStatus );
    }
    else
    {
        eventStatus = 0;
    }
    fChain->SetBranchAddress( "MJD", &MJD );
    fChain->SetBranchAddress( "Time", &Time );
    if(!bShort )
    {
        fChain->SetBranchAddress( "dataFormat", &dataFormat );
    }
    else
    {
        dataFormat = 0;
    }
    fChain->SetBranchAddress( "NTel", &NTel );
    if(!bShort )
    {
        fChain->SetBranchAddress( "traceFit", &traceFit );
    }
    else
    {
        traceFit = 0;
    }
    fChain->SetBranchAddress( "TelElevation", TelElevation );
    fChain->SetBranchAddress( "TelAzimuth", TelAzimuth );

    if(!bMC )
    {
        fChain->SetBranchAddress( "TelDec", TelDec );
        fChain->SetBranchAddress( "TelRA", TelRA );
    }
    else
    {
        for( unsigned int i = 0; i < VDST_MAXTELESCOPES; i++ )
        {
            TelDec[i] = 0.;
            TelRA[i] = 0;
        }
    }

    if(!bMC && !bShort )
    {
        fChain->SetBranchAddress( "TelElevationVBF", TelElevationVBF );
        fChain->SetBranchAddress( "TelAzimuthVBF", TelAzimuthVBF );
        fChain->SetBranchAddress( "TelPointingMismatch", TelPointingMismatch );
        fChain->SetBranchAddress( "Tel_x_SC", Tel_x_SC );
        fChain->SetBranchAddress( "Tel_y_SC", Tel_y_SC );
        fChain->SetBranchAddress( "Tel_z_SC", Tel_z_SC );
        fChain->SetBranchAddress( "TargetElev", &TargetElev );
        fChain->SetBranchAddress( "TargetAzim", &TargetAzim );
        fChain->SetBranchAddress( "TargetDec", &TargetDec );
        fChain->SetBranchAddress( "TargetRA", &TargetRA );
    }
    else
    {
        for( unsigned int i = 0; i < VDST_MAXTELESCOPES; i++ )
        {
            TelElevationVBF[i] = 0.;
            TelAzimuthVBF[i] = 0.;
            TelPointingMismatch[i] = 0.;
            Tel_x_SC[i] = 0.;
            Tel_y_SC[i] = 0.;
            Tel_z_SC[i] = 0.;
        }
        TargetElev = 0.;
        TargetAzim = 0.;
        TargetDec = 0.;
        TargetRA = 0;
    }
    fChain->SetBranchAddress( "WobbleN", &WobbleN );
    fChain->SetBranchAddress( "WobbleE", &WobbleE );
    fChain->SetBranchAddress( "NTrig", &NTrig );
    fChain->SetBranchAddress( "LTrig", &LTrig );
    fChain->SetBranchAddress( "Trig_list", Trig_list, &b_Trig_list );
    if( fChain->GetBranchStatus( "Trig_type" ) )
    {
        fChain->SetBranchAddress( "Trig_type", Trig_type, &b_Trig_type );
    }
    else
    {
        for( unsigned int i = 0; i < VDST_MAXTELESCOPES; i++ )
        {
            Trig_type[i] = 0;
        }
    }
    fChain->SetBranchAddress( "NMethods", &NMethods );
    if(!bShort )
    {
        fChain->SetBranchAddress( "MethodID", MethodID );
    }
    else
    {
        for( unsigned int i = 0; i < VDST_MAXRECMETHODS; i++ )
        {
            MethodID[i] = 0;
        }
    }

    fChain->SetBranchAddress( "NImages", NImages );
    fChain->SetBranchAddress( "img2_ang", img2_ang );
    fChain->SetBranchAddress( "ImgSel", ImgSel );
    fChain->SetBranchAddress( "ImgSel_list", ImgSel_list, &b_ImgSel_list );
    fChain->SetBranchAddress( "Ze", Ze );
    fChain->SetBranchAddress( "Az", Az );
    fChain->SetBranchAddress( "Xoff", Xoff );
    fChain->SetBranchAddress( "Yoff", Yoff );
    if( bDeRot )
    {
        fChain->SetBranchAddress( "XoffDeRot", XoffDeRot );
        fChain->SetBranchAddress( "YoffDeRot", YoffDeRot );
    }
    else
    {
        for( unsigned int i = 0; i < VDST_MAXRECMETHODS; i++ )
        {
            XoffDeRot[i] = 0.;
            YoffDeRot[i] = 0.;
        }
    }
    if(!bShort )
    {
        fChain->SetBranchAddress( "stds", stds );
        if( fChain->GetBranchStatus( "dec" ) )
        {
            fChain->SetBranchAddress( "dec", dec );
            fChain->SetBranchAddress( "ra", ra );
        }
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
    fChain->SetBranchAddress( "Xcore", Xcore );
    fChain->SetBranchAddress( "Ycore", Ycore );
    if(!bShort )
    {
        fChain->SetBranchAddress( "Xcore_SC", Xcore_SC );
        fChain->SetBranchAddress( "Ycore_SC", Ycore_SC );
        fChain->SetBranchAddress( "stdp", stdp );
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
    fChain->SetBranchAddress( "Chi2", Chi2 );
    if( fChain->GetBranchStatus( "DispDiff" ) )
    {
        fChain->SetBranchAddress( "DispDiff", DispDiff );
    }
    else
    {
        for( unsigned int i = 0; i < VDST_MAXRECMETHODS; i++ )
        {
            DispDiff[i] = 0.;
        }
    }
    if( bMC )
    {
        fChain->SetBranchAddress( "MCprim", &MCprim );
        fChain->SetBranchAddress( "MCe0", &MCe0 );
        fChain->SetBranchAddress( "MCxcore", &MCxcore );
        fChain->SetBranchAddress( "MCycore", &MCycore );
        if(!bShort )
        {
            fChain->SetBranchAddress( "MCxcos", &MCxcos );
            fChain->SetBranchAddress( "MCycos", &MCycos );
        }
        else
        {
            MCxcos = 0.;
            MCycos = 0.;
        }
        fChain->SetBranchAddress( "MCze", &MCze );
        fChain->SetBranchAddress( "MCaz", &MCaz );
        fChain->SetBranchAddress( "MCxoff", &MCxoff );
        fChain->SetBranchAddress( "MCyoff", &MCyoff );
        if(!bShort )
        {
            fChain->SetBranchAddress( "MCxcore_SC", &MCxcore_SC );
            fChain->SetBranchAddress( "MCycore_SC", &MCycore_SC );
            fChain->SetBranchAddress( "MCzcore_SC", &MCzcore_SC );
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
    b_runNumber = fChain->GetBranch( "runNumber" );
    b_eventNumber = fChain->GetBranch( "eventNumber" );
    b_MJD = fChain->GetBranch( "MJD" );
    b_Time = fChain->GetBranch( "Time" );
    if(!bShort )
    {
        b_dataFormat = fChain->GetBranch( "dataFormat" );
    }
    else
    {
        b_dataFormat = 0;
    }
    b_NTel = fChain->GetBranch( "NTel" );
    b_traceFit = fChain->GetBranch( "traceFit" );
    b_TelElevation = fChain->GetBranch( "TelElevation" );
    b_TelAzimuth = fChain->GetBranch( "TelAzimuth" );
    if(!bMC )
    {
        b_TelDec = fChain->GetBranch( "TelDec" );
        b_TelRA = fChain->GetBranch( "TelRA" );
    }
    else
    {
        b_TelDec = 0;
        b_TelRA = 0;
    }
    if(!bShort )
    {
        b_TelElevationVBF = fChain->GetBranch( "TelElevationVBF" );
        b_TelAzimuthVBF = fChain->GetBranch( "TelAzimuthVBF" );
        b_TelPointingMismatch = fChain->GetBranch( "TelPointingMismatch" );
        b_Tel_x_SC = fChain->GetBranch( "Tel_x_SC" );
        b_Tel_y_SC = fChain->GetBranch( "Tel_y_SC" );
        b_Tel_z_SC = fChain->GetBranch( "Tel_z_SC" );
        b_TargetElev = fChain->GetBranch( "TargetElev" );
        b_TargetAzim = fChain->GetBranch( "TargetAzim" );
        b_TargetDec = fChain->GetBranch( "TargetDec" );
        b_TargetRA = fChain->GetBranch( "TargetRA" );
    }
    else
    {
        b_TelElevationVBF = 0;
        b_TelAzimuthVBF = 0;
        b_TelPointingMismatch = 0;
        b_Tel_x_SC = 0;
        b_Tel_y_SC = 0;
        b_Tel_z_SC = 0;
        b_TargetElev = 0;
        b_TargetAzim = 0;
        b_TargetDec = 0;
        b_TargetRA = 0;
    }
    b_WobbleN = fChain->GetBranch( "WobbleN" );
    b_WobbleE = fChain->GetBranch( "WobbleE" );
    b_NTrig = fChain->GetBranch( "NTrig" );
    b_LTrig = fChain->GetBranch( "LTrig" );
    b_NMethods = fChain->GetBranch( "NMethods" );
    if(!bShort )
    {
        b_MethodID = fChain->GetBranch( "MethodID" );
    }
    else
    {
        b_MethodID = 0;
    }
    b_NImages = fChain->GetBranch( "NImages" );
    b_ImgSel = fChain->GetBranch( "ImgSel" );
    b_ImgSel_list = fChain->GetBranch( "ImgSel_list" );
    if(!bShort )
    {
        b_img2_ang = fChain->GetBranch( "img2_ang" );
    }
    else
    {
        b_img2_ang = 0;
    }
    b_Ze = fChain->GetBranch( "Ze" );
    b_Az = fChain->GetBranch( "Az" );
    b_Xoff = fChain->GetBranch( "Xoff" );
    b_Yoff = fChain->GetBranch( "Yoff" );
    if( bDeRot )
    {
        b_XoffDeRot = fChain->GetBranch( "XoffDeRot" );
        b_YoffDeRot = fChain->GetBranch( "YoffDeRot" );
    }
    else
    {
        b_XoffDeRot = 0;
        b_YoffDeRot = 0;
    }
    if(!bShort )
    {
        b_stds = fChain->GetBranch( "stds" );
    }
    if( fChain->GetBranchStatus( "dec" ) )
    {
        b_dec = fChain->GetBranch( "dec" );
    }
    if( fChain->GetBranchStatus( "ra" ) )
    {
        b_ra = fChain->GetBranch( "ra" );
    }
    else
    {
        b_stds = 0;
        b_dec = 0;
        b_ra = 0;
    }
    b_Xcore = fChain->GetBranch( "Xcore" );
    b_Ycore = fChain->GetBranch( "Ycore" );
    if(!bShort )
    {
        b_Xcore_SC = fChain->GetBranch( "Xcore_SC" );
        b_Ycore_SC = fChain->GetBranch( "Ycore_SC" );
        b_stdp = fChain->GetBranch( "stdp" );
    }
    else
    {
        b_Xcore_SC = 0;
        b_Ycore_SC = 0;
        b_stdp = 0;
    }

    b_Chi2 = fChain->GetBranch( "Chi2" );
    b_DispDiff = fChain->GetBranch( "DispDiff" );
    if( bMC )
    {
        if(!bShort )
        {
            b_MCprim = fChain->GetBranch( "MCprim" );
        }
        else
        {
            b_MCprim = 0;
        }
        b_MCe0 = fChain->GetBranch( "MCe0" );
        b_MCxcore = fChain->GetBranch( "MCxcore" );
        b_MCycore = fChain->GetBranch( "MCycore" );
        if(!bShort )
        {
            b_MCxcos = fChain->GetBranch( "MCxcos" );
            b_MCycos = fChain->GetBranch( "MCycos" );
        }
        else
        {
            b_MCxcos = 0;
            b_MCycos = 0;
        }
        b_MCze = fChain->GetBranch( "MCze" );
        b_MCaz = fChain->GetBranch( "MCaz" );
        b_MCxoff = fChain->GetBranch( "MCxoff" );
        b_MCyoff = fChain->GetBranch( "MCyoff" );
        if(!bShort )
        {
            b_MCxcore_SC = fChain->GetBranch( "MCycore_SC" );
            b_MCycore_SC = fChain->GetBranch( "MCycore_SC" );
            b_MCycore_SC = fChain->GetBranch( "MCycore_SC" );
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
