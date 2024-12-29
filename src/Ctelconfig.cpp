/*
 * Reading of 'telconfig' tree.
*/

#include "Ctelconfig.h"

telconfig::Ctelconfig( TTree* tree )
{
    Init( tree );
}


Ctelconfig::~Ctelconfig()
{
    if(!fChain )
    {
        return;
    }
    delete fChain->GetCurrentFile();
}

bool Ctelconfig::IsZombie()
{
    if( fChain )
    {
        return false;
    }

    return true;
}


Int_t Ctelconfig::GetEntry( Long64_t entry )
{
    if(!fChain )
    {
        return 0;
    }
    return fChain->GetEntry( entry );
}


Long64_t Ctelconfig::LoadTree( Long64_t entry )
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


void Ctelconfig::Init( TTree* tree )
{
    if(!tree )
    {
        return;
    }
    fChain = tree;
    fCurrent = -1;
    fChain->SetMakeClass( 1 );

    NTel = 0;
    fChain->SetBranchAddress( "NTel", &NTel, &b_NTel );
    if( fChain->GetBranchStatus( "TelID" ) )
    {
        fChain->SetBranchAddress( "TelID", &TelID, &b_TelID );
    }
    else
    {
        TelID = 0;
    }
    if( fChain->GetBranchStatus( "TelType" ) )
    {
        fChain->SetBranchAddress( "TelType", &TelType, &b_TelType );
    }
    else
    {
        TelType = 1;
    }
    if( fChain->GetBranchStatus( "TelID_hyperArray" ) )
    {
        fChain->SetBranchAddress( "TelID_hyperArray", &TelID_hyperArray, &b_TelID_hyperArray );
    }
    else
    {
        TelID_hyperArray = 0;
    }
    fChain->SetBranchAddress( "TelX", &TelX, &b_TelX );
    fChain->SetBranchAddress( "TelY", &TelY, &b_TelY );
    fChain->SetBranchAddress( "TelZ", &TelZ, &b_TelZ );
    if( fChain->GetBranchStatus( "NMirrors" ) )
    {
        fChain->SetBranchAddress( "NMirrors", &NMirrors, &b_NMirrors );
    }
    else
    {
        NMirrors = 0;
    }
    if( fChain->GetBranchStatus( "MirrorArea" ) )
    {
        fChain->SetBranchAddress( "MirrorArea", &MirrorArea, &b_MirrorArea );
    }
    else
    {
        MirrorArea = 0;
    }
    if( fChain->GetBranchStatus( "FOV" ) )
    {
        fChain->SetBranchAddress( "FOV", &FOV, &b_FOV );
    }
    else
    {
        FOV = 0;
    }
    fChain->SetBranchAddress( "FocalLength", &FocalLength, &b_FocalLength );
    fChain->SetBranchAddress( "CameraScaleFactor", &CameraScaleFactor, &b_CameraScaleFactor );
    fChain->SetBranchAddress( "CameraCentreOffset", &CameraCentreOffset, &b_CameraCentreOffset );
    fChain->SetBranchAddress( "CameraRotation", &CameraRotation, &b_CameraRotation );
    fChain->SetBranchAddress( "NPixel", &NPixel, &b_NPixel );
    fChain->SetBranchAddress( "NSamples", &NSamples, &b_NSamples );
    if( fChain->GetBranchStatus( "NGains" ) )
    {
        fChain->SetBranchAddress( "NGains", &NGains, &b_NGains );
    }
    else
    {
        NGains = 0;
    }
    if( fChain->GetBranchStatus( "HiLoScale" ) )
    {
        fChain->SetBranchAddress( "HiLoScale", &HiLoScale, &b_HiLoScale );
    }
    else
    {
        HiLoScale = 0.;
    }
    if( fChain->GetBranchStatus( "HiLoThreshold" ) )
    {
        fChain->SetBranchAddress( "HiLoThreshold", &HiLoThreshold, &b_HiLoThreshold );
    }
    else
    {
        HiLoThreshold = 0;
    }
    if( fChain->GetBranchStatus( "HiLoOffset" ) )
    {
        fChain->SetBranchAddress( "HiLoOffset", &HiLoOffset, &b_HiLoOffset );
    }
    else
    {
        HiLoOffset = 0.;
    }
    fChain->SetBranchAddress( "XTubeMM", XTubeMM, &b_XTubeMM );
    fChain->SetBranchAddress( "YTubeMM", YTubeMM, &b_YTubeMM );
    fChain->SetBranchAddress( "RTubeMM", RTubeMM, &b_RTubeMM );
    fChain->SetBranchAddress( "XTubeDeg", XTubeDeg, &b_XTubeDeg );
    fChain->SetBranchAddress( "YTubeDeg", YTubeDeg, &b_YTubeDeg );
    fChain->SetBranchAddress( "RTubeDeg", RTubeDeg, &b_RTubeDeg );
}


double Ctelconfig::getArrayCentreX()
{
    if( fChain )
    {
        double iX = 0.;
        double iN = 0.;
        for( Long64_t i = 0; i < fChain->GetEntries(); i++ )
        {
            fChain->GetEntry( i );

            iX += TelX;
            iN++;
        }
        if( iN > 0. )
        {
            return iX / iN;
        }
    }
    return -1.e99;
}

double Ctelconfig::getArrayCentreY()
{
    if( fChain )
    {
        double iY = 0.;
        double iN = 0.;
        for( Long64_t i = 0; i < fChain->GetEntries(); i++ )
        {
            fChain->GetEntry( i );

            iY += TelY;
            iN++;
        }
        if( iN > 0. )
        {
            return iY / iN;
        }
    }
    return -1.e99;
}

double Ctelconfig::getArrayMaxSize()
{
    if( fChain )
    {
        double iXc = getArrayCentreX();
        double iYc = getArrayCentreY();
        double iMax = 0.;
        for( Long64_t i = 0; i < fChain->GetEntries(); i++ )
        {
            fChain->GetEntry( i );

            if( TMath::Sqrt(( TelX - iXc ) * ( TelX - iXc ) + ( TelY - iYc ) * ( TelY - iYc ) ) > iMax )
            {
                iMax = TMath::Sqrt(( TelX - iXc ) * ( TelX - iXc ) + ( TelY - iYc ) * ( TelY - iYc ) );
            }
        }

        return iMax;
    }

    return 0.;
}

unsigned int Ctelconfig::getNTel()
{
    if( NTel > 0 )
    {
        return NTel;
    }

    if( fChain && fChain->GetEntries() > 0 )
    {
        fChain->GetEntry( 0 );
        return NTel;
    }

    return 0;
}
