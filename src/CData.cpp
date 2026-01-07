/*
 * Reading of 'data' tree from mscw_energy output
 *
 * Allows a simplified 3-telescope reconstruction for
 * 4-telescope data (for MC data and for effective area
 * calculation only)
 *
 */

#include "CData.h"


CData::CData( TTree* tree, bool bMC, bool bShort, TTree* stereoTree, TTree *ghTree )
{
    fMC = bMC;
    fShort = bShort;
    fVersion = 6;
    fTelescopeCombination = 0;

    fStereoFriendTree = stereoTree;
    Init( tree );
    if( fStereoFriendTree )
    {
        fStereoFriendTree->SetBranchAddress( "Dir_Xoff", &Dir_Xoff );
        fStereoFriendTree->SetBranchAddress( "Dir_Yoff", &Dir_Yoff );
        fStereoFriendTree->SetBranchAddress( "Dir_Erec", &Dir_Erec );
    }
    else
    {
        Dir_Xoff = -9999.;
        Dir_Yoff = -9999.;
        Dir_Erec = -9999.;
    }
    fGHFriendTree = ghTree;
    if( fGHFriendTree )
    {
        fGHFriendTree->SetBranchAddress( "GH_Gamma_Prediction", &GH_Gamma_Prediction );
        fGHFriendTree->SetBranchAddress( "Is_Gamma_70, ", &GH_Is_Gamma );
    }
    else
    {
        GH_Gamma_Prediction = -9999.;
        GH_Is_Gamma = false;
    }
}

CData::CData( TTree* tree, bool bMC, bool bShort, string stereo_suffix, string gamma_hadron_suffix )
    : CData(tree, bMC, bShort,
        getXGBTree(stereo_suffix, "StereoAnalysis"),
        getXGBTree(gamma_hadron_suffix, "Classification"))
{
}


CData::~CData()
{
    if(!fChain )
    {
        return;
    }
    for( unsigned int i = 0; i < fXGBFiles.size(); i++ )
    {
        if( fXGBFiles[i] )
        {
            fXGBFiles[i]->Close();
            delete fXGBFiles[i];
        }
    }
    fXGBFiles.clear();
    delete fChain->GetCurrentFile();
}



Int_t CData::GetEntry( Long64_t entry )
{
    if(!fChain )
    {
        return 0;
    }

    int a = fChain->GetEntry( entry );
    if( fStereoFriendTree )
    {
        fStereoFriendTree->GetEntry( entry );
    }
    if( fGHFriendTree )
    {
        fGHFriendTree->GetEntry( entry );
    }

    if( fTelescopeCombination > 0 && fTelescopeCombination != 15 )
    {
        reconstruct_3tel_images( fTelescopeCombination );
    }
    return a;
}


Long64_t CData::LoadTree( Long64_t entry )
{
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
    fChain->SetBranchAddress( "ArrayPointing_Azimuth", &ArrayPointing_Azimuth );
    fChain->SetBranchAddress( "ArrayPointing_Elevation", &ArrayPointing_Elevation );
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
        fChain->SetBranchAddress( "MCprimary", &MCprimary );
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


    fChain->SetBranchAddress( "LTrig", &LTrig );
    fChain->SetBranchAddress( "NTrig", &NTrig );
    fChain->SetBranchAddress( "NImages", &NImages );
    fChain->SetBranchAddress( "ImgSel", &ImgSel );
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

    fChain->SetBranchAddress( "SizeSecondMax", &SizeSecondMax );

    if( tree->GetBranchStatus( "NTtype" ) )
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
        fChain->SetBranchAddress( "loss", loss );
        fChain->SetBranchAddress( "max1", max1 );
        fChain->SetBranchAddress( "max2", max2 );
        fChain->SetBranchAddress( "max3", max3 );
        fChain->SetBranchAddress( "maxindex1", maxindex1 );
        fChain->SetBranchAddress( "maxindex2", maxindex2 );
        fChain->SetBranchAddress( "maxindex3", maxindex3 );
        fChain->SetBranchAddress( "width", width );
        fChain->SetBranchAddress( "length", length );
        fChain->SetBranchAddress( "ntubes", ntubes );
        fChain->SetBranchAddress( "nsat", nsat );
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
    fChain->SetBranchAddress( "R_core", R_core );
    if(!fShort )
    {
        fChain->SetBranchAddress( "MSCWT", MSCWT );
        fChain->SetBranchAddress( "MSCWTSigma", MSCWTSigma );
        fChain->SetBranchAddress( "MSCLT", MSCLT );
        fChain->SetBranchAddress( "MSCLTSigma", MSCLTSigma );
    }
    else
    {
        for( int i = 0; i < VDST_MAXTELESCOPES; i++ )
        {
            MSCWT[i] = 0.;
            MSCWTSigma[i] = 0.;
            MSCLT[i] = 0.;
            MSCLTSigma[i] = 0.;
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
    fChain->SetBranchAddress( "MWR", &MWR );
    fChain->SetBranchAddress( "MLR", &MLR );
    fChain->SetBranchAddress( "Erec", &Erec );
    fChain->SetBranchAddress( "EChi2", &EChi2 );
    fChain->SetBranchAddress( "ErecS", &ErecS );
    fChain->SetBranchAddress( "EChi2S", &EChi2S );
    if( tree->GetBranchStatus( "dE" ) )
    {
        fChain->SetBranchAddress( "dE", &dE );
        fChain->SetBranchAddress( "dES", &dES );
    }
    else
    {
        dE = 0.;
        dES = 0.;
    }
    if( fChain->GetBranchStatus( "ErecQL" ) )
    {
        fChain->SetBranchAddress( "ErecQL", &ErecQL );
        fChain->SetBranchAddress( "NErecT", &NErecT );
    }
    else
    {
        ErecQL = 0;
        NErecT = 0;
    }
    EmissionHeight = -99.;
    fChain->SetBranchAddress( "EmissionHeight", &EmissionHeight );
    fChain->SetBranchAddress( "EmissionHeightChi2", &EmissionHeightChi2 );
    fChain->SetBranchAddress( "NTelPairs", &NTelPairs );
    if(!fShort && fChain->GetBranchStatus( "EmissionHeightT" ) )
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
    b_meanPedvar_Image = fChain->GetBranch( "meanPedvar_Image" );
    b_meanPedvar_ImageT = fChain->GetBranch( "meanPedvar_ImageT" );
    b_SizeSecondMax = fChain->GetBranch( "SizeSecondMax" );
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
    b_R_core = fChain->GetBranch( "R_core" );
    b_MSCWT = fChain->GetBranch( "MSCWT" );
    b_MSCWTSigma = fChain->GetBranch( "MSCWTSigma" );
    b_MSCLT = fChain->GetBranch( "MSCLT" );
    b_MSCLTSigma = fChain->GetBranch( "MSCLTSigma" );
    b_E = fChain->GetBranch( "E" );
    b_ES = fChain->GetBranch( "ES" );
    b_NMSCW = fChain->GetBranch( "NMSCW" );
    b_MSCW = fChain->GetBranch( "MSCW" );
    b_MSCL = fChain->GetBranch( "MSCL" );
    b_MWR = fChain->GetBranch( "MWR" );
    b_MLR = fChain->GetBranch( "MLR" );
    b_Erec = fChain->GetBranch( "Erec" );
    b_EChi2 = fChain->GetBranch( "EChi2" );
    b_ErecS = fChain->GetBranch( "ErecS" );
    b_EChi2S = fChain->GetBranch( "EChi2S" );
    b_EmissionHeight = fChain->GetBranch( "EmissionHeight" );
    b_EmissionHeightChi2 = fChain->GetBranch( "EmissionHeightChi2" );
    b_NTelPairs = fChain->GetBranch( "NTelPairs" );
    b_EmissionHeightT = fChain->GetBranch( "EmissionHeightT" );
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

/*
   Get Gamma/hadron decision from XGB friend tree.
*/
float CData::get_GH_Gamma_Prediction()
{
    return GH_Gamma_Prediction;
}

/*
  Get Gamma/hadron decision from XGB friend tree.
*/
bool CData::is_GH_Gamma()
{
    return GH_Is_Gamma;
}


/*
* Get energy depending on analysis method
* Methods:
*
* 0: return friend tree result (if available) or Erec
* 1: return ErecS
* 2: return Erec
* 3: return friend tree result
*/
float CData::get_Erec( unsigned iMethod )
{
    if( iMethod == 0 && fStereoFriendTree )
    {
        return Dir_Erec;
    }
    else if( iMethod == 1 )
    {
        return ErecS;
    }
    else if( iMethod == 2 )
    {
        return Erec;
    }
    else if( iMethod == 3 )
    {
        return Dir_Erec;
    }
    return Erec;
}

/*
 * Get stereo Xoff depending on analysis method
 *
 * Methods:
 *
 * 0: return friend tree result (if available or Xoff)
 * 1: return Xoff
 * 2: return Xoff_intersect
 * 3: return friend tree result
 */
float CData::get_Xoff( unsigned iMethod )
{
    if( iMethod == 0 && fStereoFriendTree )
    {
        return Dir_Xoff;
    }
    else if( iMethod == 1 )
    {
        return Xoff;
    }
    else if( iMethod == 2 )
    {
        return Xoff_intersect;
    }
    else if( iMethod == 3 )
    {
        return Dir_Xoff;
    }
    return ( float )Xoff;
}


/*
 * Get stereo Yoff depending on analysis method
 *
 * Methods:
 *
 * 0: return friend tree result (if available or Yoff)
 * 1: return Yoff
 * 2: return Yoff_intersect
 * 3: return friend tree result
 */
float CData::get_Yoff( unsigned iMethod )
{
    if( iMethod == 0 && fStereoFriendTree )
    {
        return Dir_Yoff;
    }
    else if( iMethod == 1 )
    {
        return Yoff;
    }
    else if( iMethod == 2 )
    {
        return Yoff_intersect;
    }
    else if( iMethod == 3 )
    {
        return Dir_Yoff;
    }
    return ( float )Yoff;
}

pair<float, float> CData::get_XYoff_derot( unsigned int iMethod )
{
    float tmp_xoff = get_Xoff( iMethod );
    float tmp_yoff = get_Yoff( iMethod );
    if( tmp_xoff < -990. || tmp_yoff < -990. ) return {-999., -999.};
    if( fMC ) return {tmp_xoff, tmp_yoff};

    float rot_angle = VSkyCoordinatesUtilities::getDerotationAngleFromGroundCoordinates( MJD, Time, ArrayPointing_Azimuth, ArrayPointing_Elevation );

    return
    {
        tmp_xoff * cos( rot_angle ) - tmp_yoff * sin( rot_angle ),
        tmp_yoff * cos( rot_angle ) + tmp_xoff * sin( rot_angle ),
    };
}


/*
 * Redo stereo reconstruction for a selection of 3-telescope events.
 * Used for calculation of effective areas from 4-telescope mscw files.
 *
 * Note! This is very fine tuned and should be used for effective area calculation only
 *
 *
 * Approximations:
 *
 * - lookup table values like MSCWT or MSCL are read from lookup tables using the 4-tel
 *   reconstructed shower core and direction
 * - dispEnergy per telescope is calculated using the 4-tel reconstructed shower core
 *   and direction
 *
 * Following variables are approximated or not calculated correctly:
 *
 * - img2_ang
 * - ErecS, EChi2S, dES (lookup table based energy reconstruction results)
 *
 */
void CData::reconstruct_3tel_images( unsigned long int telescope_combination )
{
    reconstruct_3tel_reset_variables();
    bitset<sizeof(long int ) * 4> tel_bitset( telescope_combination );
    bitset<sizeof(long int ) * 4> tel_nimages( 0 );
    int NImages_saved = ( int )NImages;

    // update list of available images
    UInt_t tel_list[VDST_MAXTELESCOPES];
    for( unsigned int i = 0; i < VDST_MAXTELESCOPES; i++ )
    {
        tel_list[i] = 0;
    }
    unsigned int z = 0;
    for( int i = 0; i < NImages; i++ )
    {
        unsigned int t = ImgSel_list[i];
        if( tel_bitset.test( t ) )
        {
            tel_list[z] = t;
            tel_nimages.set( t );
            z++;
        }
        else
        {
            size[t] = -99.;
            ntubes[t] = 0;
            MSCWT[t] = -9999.;
            MSCLT[t] = -9999.;
        }
    }
    // ImgSel_list lists the available images (e.g., 2, 3, 4) and is of length NImages
    NImages = ( UShort_t )z;
    ImgSel = tel_nimages.to_ulong();
    for( int i = 0; i < NImages_saved; i++ )
    {
        ImgSel_list[i] = tel_list[i];
    }
    // require at least two images to continue
    if( NImages < 2 )
    {
        return;
    }

    float SizeFirstMax = -1000.;
    SizeSecondMax = -100.;
    for( int i = 0; i < NImages; i++ )
    {
        unsigned int t = ImgSel_list[i];
        if( size[t] > SizeSecondMax )
        {
            if( size[t] > SizeFirstMax )
            {
                SizeSecondMax = SizeFirstMax;
                SizeFirstMax = size[t];
            }
            else
            {
                SizeSecondMax = size[t];
            }
        }
    }

    reconstruct_3tel_images_scaled_emission_height();
    reconstruct_3tel_images_scaled_variables();
    reconstruct_3tel_images_direction();
    reconstruct_3tel_images_energy();
}

/*
 * Reset all variables relevant for 3-telescope reconstruction.
 *
 */
void CData::reconstruct_3tel_reset_variables()
{
    SizeSecondMax = -9999.;
    EmissionHeight = -9999.;
    EmissionHeightChi2 = -9999.;
    MSCW = -9999.;
    MSCL = -9999.;
    MWR = -9999.;
    MLR = -9999.;
    Xoff = Xoff_derot = -9999.;
    Yoff = Yoff_derot = -9999.;
    DispAbsSumWeigth = -9999.;
    DispDiff = Chi2 = -9999.;
    Xoff_intersect = Yoff_intersect = -9999.;
    Xcore = -9999.;
    Ycore = -9999.;
    Erec = -9999.;
    EChi2 = -9999.;
    dE = -999.;
    NErecT = 0;
    ErecQL = -99;

    // not filled in 3-tel reconstruction
    // (requires access to lookup tables or storage of ErecST)
    ErecS = -9999.;
    EChi2S = -9999.;
    dES = -999.;

}

/*
 * Calculate average emission height for 3-telescope events.
 *
*/
void CData::reconstruct_3tel_images_scaled_emission_height()
{
    VEmissionHeightCalculator fEmissionHeightCalculator;
    fEmissionHeightCalculator.setTelescopePositions( fTelX.size(), fTelX.data(), fTelY.data(), fTelZ.data() );
    fEmissionHeightCalculator.getEmissionHeight( cen_x, cen_y, size, ArrayPointing_Azimuth, ArrayPointing_Elevation );
    EmissionHeight = ( float )fEmissionHeightCalculator.getMeanEmissionHeight();
    EmissionHeightChi2 = ( float )fEmissionHeightCalculator.getMeanEmissionHeightChi2();
    if( EmissionHeightChi2 <= 0. )
    {
        EmissionHeightChi2 = 1.e-10;
    }
}

/*
 * Calculate mean scaled variables for 3-telescope events.
 *
*/
void CData::reconstruct_3tel_images_scaled_variables()
{
    MSCW = VMeanScaledVariables::mean_reduced_scaled_variable( 4, width, MSCWT, MSCWTSigma );
    MSCL = VMeanScaledVariables::mean_reduced_scaled_variable( 4, length, MSCLT, MSCLTSigma );
    MWR = VMeanScaledVariables::mean_scaled_variable( 4, width, size, MSCWT );
    MLR = VMeanScaledVariables::mean_scaled_variable( 4, length, size, MSCLT );
}

/*
 * Reconstruct shower direction and core for 3-telescope events.
 *
 * Using the 4-telescope disp results approximates the exact result:
 *
 * - cross is among the most relevant parameters for the dispBDTs, and
 *   disp is calculating using the 4-telescope cross
 * - angular resolutions are better than they should be (approx 0.02 deg at 1 TeV)
 *
*/
void CData::reconstruct_3tel_images_direction()
{
    // intersection method
    double img_weight[fTelX.size()];
    for( unsigned int t = 0; t < fTelX.size(); t++ ) img_weight[t] = 1.;
    VSimpleStereoReconstructor i_SR;
    i_SR.initialize( 2, fStereoMinAngle );
    i_SR.reconstruct_direction(
        fTelX.size(),
        ArrayPointing_Elevation, ArrayPointing_Azimuth,
        fTelX.data(), fTelY.data(), fTelZ.data(),
        size, cen_x, cen_y, cosphi, sinphi, width, length, img_weight );
    Xoff_intersect = i_SR.fShower_Xoffset;
    Yoff_intersect = i_SR.fShower_Yoffset;

    // disp method
    vector<float> v_x, v_y, v_weight;
    float xs, ys, dispdiff;
    for( int i = 0; i < NImages; i++ )
    {
        unsigned int t = ImgSel_list[i];
        v_x.push_back( DispXoff_T[t] );
        v_y.push_back( DispYoff_T[t] );
        v_weight.push_back( DispWoff_T[t] );
    }

    VDispAnalyzer i_dispAnalyzer;
    i_dispAnalyzer.calculateMeanShowerDirection( v_x, v_y, v_weight, xs, ys, dispdiff );
    // expect that this is called for MC only
    Xoff = Xoff_derot = xs;
    Yoff = Yoff_derot = ys;
    DispAbsSumWeigth = i_dispAnalyzer.get_abs_sum_disp_weight();
    DispDiff = Chi2 = dispdiff;

    // Average/min angle between images not updated.
    // approximiation for events which are reduced to 2-tel images;
    // the cut on the minimal angle between images is not applied.
    img2_ang = -9999.;

    i_SR.fillShowerDirection( Xoff, Yoff );
    Ze    = i_SR.fShower_Ze;
    Az    = i_SR.fShower_Az;

    // core reconstruction
    i_SR.reconstruct_core(
        fTelX.size(),
        ArrayPointing_Elevation, ArrayPointing_Azimuth,
        Xoff, Yoff,
        fTelX.data(), fTelY.data(), fTelZ.data(),
        size, cen_x, cen_y, cosphi, sinphi, width, length, img_weight );
    Xcore = i_SR.fShower_Xcore;
    Ycore = i_SR.fShower_Ycore;
    bitset<sizeof(long int ) * 4> tel_nimages( ImgSel );
    for( unsigned int t = 0; t < fTelX.size(); t++ )
    {
        R_core[t] = -9999.;
        if( tel_nimages.test( t ) && Ze >= 0. && Xcore > -9998. && Ycore > -9998. )
        {
            R_core[t] = VUtilities::line_point_distance(
                            Ycore, -1.*Xcore, 0., Ze, Az, fTelY[t], -1.*fTelX[t], fTelZ[t] );
        }
    }
}

/*
 * Reconstruct shower energy for 3-telescope events.
 *
*/
void CData::reconstruct_3tel_images_energy()
{
    VDispAnalyzer i_dispAnalyzer;
    vector< float > disp_energy_T;
    for( unsigned int t = 0; t < fTelX.size(); t++ )
    {
        disp_energy_T.push_back( E[t] );
    }
    i_dispAnalyzer.calculateMeanEnergy( disp_energy_T, size, 0 );
    Erec = i_dispAnalyzer.getEnergy();
    EChi2 = i_dispAnalyzer.getEnergyChi2();
    dE = i_dispAnalyzer.getEnergydES();
    NErecT = i_dispAnalyzer.getEnergyNT();
    ErecQL = i_dispAnalyzer.getEnergyQualityLabel();
}

/*
 * initialize stereo reconstruction for 3-telescope
 *
 */
void CData::initialize_3tel_reconstruction(
    unsigned long int telescope_combination,
    double stereo_reconstruction_min_angle, vector< double > tel_x, vector< double > tel_y, vector< double > tel_z )
{
    fTelescopeCombination = telescope_combination;
    fStereoMinAngle = stereo_reconstruction_min_angle;
    fTelX = tel_x;
    fTelY = tel_y;
    fTelZ = tel_z;
}


/*
   Read XGB friend tree for gamma/hadron separation and stereo reconstruction
*/
/*

*/
TTree *CData::getXGBTree(string file_suffix, string tree_name)
{
    if( file_suffix == "" || file_suffix != "None" )
    {
        return 0;
    }

    string iFileName = iFileName.replace(iFileName.find( ".root" ), 5, "." + file_suffix + ".root" );
    TFile *iFile = new TFile( iFileName.c_str());
    if( iFile->IsZombie() )
    {
        cout << "CData Error: cannot open XGB file " << iFileName << endl;
        exit( EXIT_FAILURE );
    }
    TTree* iXGB_tree = ( TTree* )iFile->Get( tree_name.c_str() );
    if(!iXGB_tree )
    {
        cout << "CData Error: cannot find " << tree_name << " tree in " << iFileName << endl;
        exit( EXIT_FAILURE );
    }
    fXGBFiles.push_back(iFile);
    cout << "Adding " << tree_name << " tree from " << iFileName << endl;
    return iXGB_tree;
}
