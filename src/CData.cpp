/*
 * Reading of 'data' tree from mscw_energy output
 *
 * Allows a simplified 3-telescope reconstruction for
 * 4-telescope data (for MC data and for effective area
 * calculation only)
 *
 */

#include "CData.h"

CData::CData( TTree* tree, bool bMC, int iVersion, bool bShort )
{
    fMC = bMC;
    fShort = bShort;
    fVersion = iVersion;
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


Int_t CData::GetEntry( Long64_t entry, unsigned long int telescope_combination )
{
    if(!fChain )
    {
        return 0;
    }

    int a = fChain->GetEntry( entry );

    if( telescope_combination != 15 && NImages == 4 )
    {
        reconstruct_3tel_images(telescope_combination);
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
    fChain->SetBranchAddress( "MWR", &MWR );
    fChain->SetBranchAddress( "MLR", &MLR );
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
    b_MSCLT = fChain->GetBranch( "MSCLT" );
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
 * Redo stereo reconstruction for a selection of 3-telescope images.
 * Used for calculation of effective areas from 4-telescope mscw files
 *
 * Note! This is very fine tuned and should be used for effective area calculation only
 */
void CData::reconstruct_3tel_images(unsigned long int telescope_combination)
{
    bitset<sizeof(long int) * 4> tel_bitset(telescope_combination);
    NImages = (Int_t)tel_bitset.count();
    ImgSel = telescope_combination;
    unsigned int z = 0;
    SizeSecondMax = 0.;
    for(unsigned int t = 0; t < 4; t++ )
    {
        if( tel_bitset.test(t) )
        {
            ImgSel_list[z] = t;
            z++;
            if( size[t] > SizeSecondMax )
            {
                SizeSecondMax = size[t];
            }
        }
    }

    reconstruct_3tel_images_scaled_emission_height();
    reconstruct_3tel_images_scaled_variables();
    //    reconstruct_3tel_images_direction();
    //    reconstruct_3tel_images_energy();
}

/*
 * Calculate average emission height for 3-telescope image
 *
 * TODO requires telescope position vector
*/
void CData::reconstruct_3tel_images_scaled_emission_height()
{
    EmissionHeight = EmissionHeight;
    EmissionHeightChi2 = EmissionHeightChi2;
}

/*
 * Calculate mean scaled variables for 3-telescope image
 *
*/
void CData::reconstruct_3tel_images_scaled_variables()
{
    MSCW = -9999.;
    MSCL = -9999.;
    MWR = -9999.;
    MLR = -9999.;
}

/*

void reconstruct_3tel_images_direction()
{
    VDispAnalyzer i_dispAnalyzer;
    Chi2 = -9999.;

    Xoff = -9999.;
    Yoff = -9999.;
    Xoff_derot = -9999.;
    Yoff_derot = -9999.;
    Xoff_intersect = -9999.;
    Yoff_intersect = -9999.;
    DispAbsSumWeigth = -9999.;
    DispDiff = -9999.;

    Ze = Ze;

    Xcore = -9999.;
    Ycore = -9999.;
    // R_core[t]
}

void reconstruct_3tel_images_energy()
{
    ErecS = -9999.;
    EChi2S = EChi2S
    dES = dES;

    Erec // (?)
    EChi2 // (?)
    dE // (?)
}

*/

/*
 * - is applyMeanStereoShapeCuts correct? Loop over ntel?
 */
