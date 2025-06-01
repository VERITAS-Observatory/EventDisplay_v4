// Effective area data tree definition.

#include "CEffArea.h"

CEffArea::CEffArea( TTree* tree )
{
    // if parameter tree is not specified (or zero), connect the file
    // used to generate this class and read the Tree.
    if( tree == 0 )
    {
        TFile* f = ( TFile* )gROOT->GetListOfFiles()->FindObject( "effectiveArea.root" );
        if(!f )
        {
            f = new TFile( "effectiveArea.root" );
        }
        tree = ( TTree* )gDirectory->Get( "fEffArea" );

    }
    Init( tree );
}


CEffArea::~CEffArea()
{
    if(!fChain )
    {
        return;
    }
    delete fChain->GetCurrentFile();
}


Int_t CEffArea::GetEntry( Long64_t entry )
{
    // Read contents of entry.
    if(!fChain )
    {
        return 0;
    }
    return fChain->GetEntry( entry );
}


Long64_t CEffArea::LoadTree( Long64_t entry )
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
    if(!fChain->InheritsFrom( TChain::Class() ) )
    {
        return centry;
    }
    TChain* chain = ( TChain* )fChain;
    if( chain->GetTreeNumber() != fCurrent )
    {
        fCurrent = chain->GetTreeNumber();
    }
    return centry;
}


void CEffArea::Init( TTree* tree )
{
    hEmc = 0;
    hEcut = 0;
    hEcutUW = 0;
    hEcut500 = 0;
    hEcutLin = 0;
    hEcutRec = 0;
    hEcutRecUW = 0;
    gEffAreaMC = 0;
    gEffAreaRec = 0;
    hEmcSWeight = 0;
    hEsysRec = 0;
    hEsysMC = 0;
    hEsysMCRelative = 0;
    hEsysMCRelativeRMS = 0;
    hEsysMCRelative2D = 0;
    hEsys2D = 0;
    hEmcCutCTA = 0;
    hResponseMatrix = 0;
    hResponseMatrixProfile = 0;
    hResponseMatrixQC = 0;
    hResponseMatrixFineQC = 0;
    hhEcutTrigger = 0;
    hhEcutFiducialArea = 0;
    hhEcutStereoQuality = 0;
    hhEcutTelType = 0;
    hhEcutDirection = 0;
    hhEcutGammaHadron = 0;
    hhEcutEnergyReconstruction = 0;
    hWeightedRate = 0;
    // Set branch addresses and branch pointers
    if(!tree )
    {
        return;
    }
    fChain = tree;
    fCurrent = -1;
    fChain->SetMakeClass( 1 );

    fChain->SetBranchAddress( "ze", &ze, &b_ze );
    fChain->SetBranchAddress( "az", &az, &b_az );
    fChain->SetBranchAddress( "azMin", &azMin, &b_azMin );
    fChain->SetBranchAddress( "azMax", &azMax, &b_azMax );
    if( fChain->GetBranchStatus( "Xoff" ) )
    {
        fChain->SetBranchAddress( "Xoff", &Xoff, &b_Xoff );
        fChain->SetBranchAddress( "Yoff", &Yoff, &b_Yoff );
    }
    else
    {
        Xoff = 0.;
        Yoff = 0.;
    }
    fChain->SetBranchAddress( "Woff", &Woff, &b_Woff );
    fChain->SetBranchAddress( "noise", &noise, &b_noise );
    fChain->SetBranchAddress( "pedvar", &pedvar, &b_pedvar );
    if( fChain->GetBranchStatus( "index" ) )
    {
        fChain->SetBranchAddress( "index", &index, &b_index );
    }
    else
    {
        index = 0.;
    }
    if( fChain->GetBranchStatus( "nbins" ) )
    {
        fChain->SetBranchAddress( "nbins", &nbins, &b_nbins );
        fChain->SetBranchAddress( "e0", e0, &b_e0 );
        fChain->SetBranchAddress( "eff", eff, &b_eff );
    }
    else
    {
        nbins = 0;
        for( int i = 0; i < 1000; i++ )
        {
            e0[i] = 0.;
            eff[i] = 0.;
        }
    }
    if( fChain->GetBranchStatus( "seff_L" ) )
    {
        fChain->SetBranchAddress( "seff_L", seff_L, &b_seff_L );
        fChain->SetBranchAddress( "seff_U", seff_U, &b_seff_U );
    }
    else
    {
        for( int i = 0; i < 1000; i++ )
        {
            seff_L[i] = 0.;
            seff_U[i] = 0.;
        }
    }
    if( fChain->GetBranchStatus( "Rec_seff_L" ) )
    {
        fChain->SetBranchAddress( "Rec_seff_L", Rec_seff_L, &b_Rec_seff_L );
        fChain->SetBranchAddress( "Rec_seff_U", Rec_seff_U, &b_Rec_seff_U );
    }
    else
    {
        for( int i = 0; i < 1000; i++ )
        {
            Rec_seff_L[i] = 0.;
            Rec_seff_U[i] = 0.;
        }
    }
    if( fChain->GetBranchStatus( "Rec_nbins" ) )
    {
        fChain->SetBranchAddress( "Rec_nbins", &Rec_nbins, &b_Rec_nbins );
        fChain->SetBranchAddress( "Rec_e0", Rec_e0, &b_Rec_e0 );
        fChain->SetBranchAddress( "Rec_eff", Rec_eff, &b_Rec_eff );
    }
    else
    {
        Rec_nbins = 0;
        for( int i = 0; i < 1000; i++ )
        {
            Rec_e0[i] = 0.;
            Rec_eff[i] = 0.;
        }
    }
    if( fChain->GetBranchStatus( "hEmc" ) )
    {
        fChain->SetBranchAddress( "hEmc", &hEmc, &b_hEmc );
    }
    else
    {
        hEmc = 0;
    }
    if( fChain->GetBranchStatus( "hEcut" ) )
    {
        fChain->SetBranchAddress( "hEcut", &hEcut, &b_hEcut );
        if( fChain->GetBranchStatus( "hEcutUW" ) )
        {
            fChain->SetBranchAddress( "hEcutUW", &hEcutUW, &b_hEcutUW );
        }
        else
        {
            hEcutUW = 0;
        }
        fChain->SetBranchAddress( "hEcut500", &hEcut500, &b_hEcut500 );
        fChain->SetBranchAddress( "hEcutRec", &hEcutRec, &b_hEcutRec );
        if( fChain->GetBranchStatus( "hEcutRecUW" ) )
        {
            fChain->SetBranchAddress( "hEcutRecUW", &hEcutRecUW, &b_hEcutRecUW );
        }
        else
        {
            hEcutRecUW = 0;
        }
        fChain->SetBranchAddress( "gEffAreaMC", &gEffAreaMC, &b_gEffAreaMC );
        fChain->SetBranchAddress( "gEffAreaRec", &gEffAreaRec, &b_gEffAreaRec );
        fChain->SetBranchAddress( "hEmcSWeight", &hEmcSWeight, &b_hEmcSWeight );
        fChain->SetBranchAddress( "hEsysRec", &hEsysRec, &b_hEsysRec );
        fChain->SetBranchAddress( "hEsysMC", &hEsysMC, &b_hEsysMC );
        fChain->SetBranchAddress( "hEsys2D", &hEsys2D, &b_hEsys2D );
        fChain->SetBranchAddress( "hEmcCutCTA", &hEmcCutCTA, &b_hEmcCutCTA );
        fChain->SetBranchAddress( "hResponseMatrix", &hResponseMatrix, &b_hResponseMatrix );
        if( fChain->GetBranchStatus( "hResponseMatrixQC" ) )
        {
            fChain->SetBranchAddress( "hResponseMatrixQC", &hResponseMatrixQC, &b_hResponseMatrixQC );
        }
        else
        {
            hResponseMatrixQC = 0;
        }
        if( fChain->GetBranchStatus( "hResponseMatrixProfile" ) )
        {
            fChain->SetBranchAddress( "hResponseMatrixProfile", &hResponseMatrixProfile, &b_hResponseMatrixProfile );
        }
        else
        {
            hResponseMatrixProfile = 0;
        }
        if( fChain->GetBranchStatus( "hResponseMatrixFineQC" ) )
        {
            fChain->SetBranchAddress( "hResponseMatrixFineQC", &hResponseMatrixFineQC, &b_hResponseMatrixFineQC );
        }
        else
        {
            hResponseMatrixFineQC = 0;
        }
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
        hEmcSWeight = 0;
        hEsysRec = 0;
        hEsysMC = 0;
        hEsysMCRelative = 0;
        hEsysMCRelativeRMS = 0;
        hEsys2D = 0;
        hEmcCutCTA = 0;
        hResponseMatrix = 0;
        hResponseMatrixProfile = 0;
        hResponseMatrixQC = 0;
        hResponseMatrixFineQC = 0;
    }
    if( fChain->GetBranchStatus( "hEcutLin" ) )
    {
        fChain->SetBranchAddress( "hEcutLin", &hEcutLin, &b_hEcutLin );
    }
    else
    {
        hEcutLin = 0;
    }
    if( fChain->GetBranchStatus( "hEsysMCRelative" ) )
    {
        fChain->SetBranchAddress( "hEsysMCRelative", &hEsysMCRelative, &b_hEsysMCRelative );
    }
    else
    {
        hEsysMCRelative = 0;
    }
    if( fChain->GetBranchStatus( "hEsysMCRelativeRMS" ) )
    {
        fChain->SetBranchAddress( "hEsysMCRelativeRMS", &hEsysMCRelativeRMS, &b_hEsysMCRelativeRMS );
    }
    else
    {
        hEsysMCRelativeRMS = 0;
    }
    if( fChain->GetBranchStatus( "hEsysMCRelative2D" ) )
    {
        fChain->SetBranchAddress( "hEsysMCRelative2D", &hEsysMCRelative2D, &b_hEsysMCRelative2D );
    }
    else
    {
        hEsysMCRelative2D = 0;
    }
    if( fChain->GetBranchStatus( "hhEcutTrigger" ) )
    {
        fChain->SetBranchAddress( "hhEcutTrigger", &hhEcutTrigger, &b_hhEcutTrigger );
        fChain->SetBranchAddress( "hhEcutFiducialArea", &hhEcutFiducialArea, &b_hhEcutFiducialArea );
        fChain->SetBranchAddress( "hhEcutStereoQuality", &hhEcutStereoQuality, &b_hhEcutStereoQuality );
        fChain->SetBranchAddress( "hhEcutTelType", &hhEcutTelType, &b_hhEcutTelType );
        fChain->SetBranchAddress( "hhEcutDirection", &hhEcutDirection, &b_hhEcutDirection );
        fChain->SetBranchAddress( "hhEcutEnergyReconstruction", &hhEcutEnergyReconstruction, &b_hhEcutEnergyReconstruction );
        fChain->SetBranchAddress( "hhEcutGammaHadron", &hhEcutGammaHadron, &b_hhEcutGammaHadron );
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
    if( fChain->GetBranchStatus( "hWeightedRate" ) )
    {
        fChain->SetBranchAddress( "hWeightedRate", &hWeightedRate, &b_hWeightedRate );
    }
    else
    {
        hWeightedRate = 0;
    }
}
