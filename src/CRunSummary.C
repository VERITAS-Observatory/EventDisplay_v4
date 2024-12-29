/*
 * Anasum run summary tree definition.
 */

#define CRunSummary_cxx

CRunSummary::CRunSummary( TTree* tree )
{
    fChain = 0;
    Init( tree );
}


CRunSummary::~CRunSummary()
{
    if( fChain && fChain->GetCurrentFile() )
    {
        delete fChain->GetCurrentFile();
    }
}


Int_t CRunSummary::GetEntry( Long64_t entry )
{
    // Read contents of entry.
    if(!fChain )
    {
        return 0;
    }
    return fChain->GetEntry( entry );
}


Long64_t CRunSummary::LoadTree( Long64_t entry )
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


void CRunSummary::Init( TTree* tree )
{
    if( tree == 0 )
    {
        return;
    }
    fChain = tree;
    fCurrent = -1;
    fChain->SetMakeClass( 1 );

    fChain->SetBranchAddress( "runOn", &runOn );
    fChain->SetBranchAddress( "runOff", &runOff );
    fChain->SetBranchAddress( "MJDOn", &MJDOn );
    fChain->SetBranchAddress( "MJDOff", &MJDOff );
    fChain->SetBranchAddress( "MJDrunstart", &MJDrunstart, &b_MJDrunstart );
    fChain->SetBranchAddress( "MJDrunstop", &MJDrunstop, &b_MJDrunstop );
    fChain->SetBranchAddress( "TargetName", TargetName, &b_TargetName );
    fChain->SetBranchAddress( "TargetRA", &TargetRA );
    fChain->SetBranchAddress( "TargetDec", &TargetDec );
    fChain->SetBranchAddress( "TargetRAJ2000", &TargetRAJ2000 );
    fChain->SetBranchAddress( "TargetDecJ2000", &TargetDecJ2000 );
    fChain->SetBranchAddress( "SkyMapCentreRAJ2000", &SkyMapCentreRAJ2000 );
    fChain->SetBranchAddress( "SkyMapCentreDecJ2000", &SkyMapCentreDecJ2000 );
    fChain->SetBranchAddress( "TargetShiftRAJ2000", &TargetShiftRAJ2000 );
    fChain->SetBranchAddress( "TargetShiftDecJ2000", &TargetShiftDecJ2000 );
    fChain->SetBranchAddress( "TargetShiftWest", &TargetShiftWest );
    fChain->SetBranchAddress( "TargetShiftNorth", &TargetShiftNorth );
    fChain->SetBranchAddress( "WobbleNorth", &WobbleNorth );
    fChain->SetBranchAddress( "WobbleWest", &WobbleWest );
    fChain->SetBranchAddress( "NTel", &NTel );
    fChain->SetBranchAddress( "TelList", TelList, &b_TelList );
    fChain->SetBranchAddress( "tOn", &tOn );
    fChain->SetBranchAddress( "tOff", &tOff );
    fChain->SetBranchAddress( "elevationOn", &elevationOn );
    fChain->SetBranchAddress( "azimuthOn", &azimuthOn );
    fChain->SetBranchAddress( "elevationOff", &elevationOff );
    fChain->SetBranchAddress( "azimuthOff", &azimuthOff );
    fChain->SetBranchAddress( "Theta2Max", &Theta2Max, &b_Theta2Max );
    fChain->SetBranchAddress( "RawRateOn", &RawRateOn );
    fChain->SetBranchAddress( "RawRateOff", &RawRateOff );
    if( fChain->GetBranchStatus( "pedvarsOn" ) )
    {
        fChain->SetBranchAddress( "pedvarsOn", &pedvarsOn );
        fChain->SetBranchAddress( "pedvarsOff", &pedvarsOff );
    }
    // no pedvars given, assume galactic source
    else
    {
        pedvarsOn = 8.1;
        pedvarsOff = 8.1;
    }
    fChain->SetBranchAddress( "NOn", &NOn );
    fChain->SetBranchAddress( "NOff", &NOff );
    fChain->SetBranchAddress( "NOffNorm", &NOffNorm );
    fChain->SetBranchAddress( "OffNorm", &OffNorm );
    fChain->SetBranchAddress( "Signi", &Signi );
    fChain->SetBranchAddress( "Rate", &Rate );
    fChain->SetBranchAddress( "RateE", &RateE );
    fChain->SetBranchAddress( "RateOff", &RateOff );
    fChain->SetBranchAddress( "RateOffE", &RateOffE );
    fChain->SetBranchAddress( "DeadTimeFracOn", &DeadTimeFracOn );
    fChain->SetBranchAddress( "DeadTimeFracOff", &DeadTimeFracOff );
    fChain->SetBranchAddress( "MaxSigni", &MaxSigni );
    fChain->SetBranchAddress( "MaxSigniX", &MaxSigniX );
    fChain->SetBranchAddress( "MaxSigniY", &MaxSigniY );
    Notify();
}


Bool_t CRunSummary::Notify()
{
    b_runOn = fChain->GetBranch( "runOn" );
    b_runOff = fChain->GetBranch( "runOff" );
    b_MJDOn = fChain->GetBranch( "MJDOn" );
    b_MJDOff = fChain->GetBranch( "MJDOff" );
    b_TargetRA = fChain->GetBranch( "TargetRA" );
    b_TargetDec = fChain->GetBranch( "TargetDec" );
    b_TargetRAJ2000 = fChain->GetBranch( "TargetRAJ2000" );
    b_TargetDecJ2000 = fChain->GetBranch( "TargetDecJ2000" );
    b_SkyMapCentreRAJ2000 = fChain->GetBranch( "SkyMapCentreRAJ2000" );
    b_SkyMapCentreDecJ2000 = fChain->GetBranch( "SkyMapCentreDecJ2000" );
    b_TargetShiftRAJ2000 = fChain->GetBranch( "TargetShiftRAJ2000" );
    b_TargetShiftDecJ2000 = fChain->GetBranch( "TargetShiftDecJ2000" );
    b_TargetShiftWest = fChain->GetBranch( "TargetShiftWest" );
    b_TargetShiftNorth = fChain->GetBranch( "TargetShiftNorth" );
    b_WobbleNorth = fChain->GetBranch( "WobbleNorth" );
    b_WobbleWest = fChain->GetBranch( "WobbleWest" );
    b_NTel = fChain->GetBranch( "NTel" );
    b_tOn = fChain->GetBranch( "tOn" );
    b_tOff = fChain->GetBranch( "tOff" );
    if( fChain->GetBranchStatus( "pedvarsOn" ) )
    {
        b_pedvarsOn = fChain->GetBranch( "pedvarsOn" );
        b_pedvarsOff = fChain->GetBranch( "pedvarsOff" );
    }
    else
    {
        b_pedvarsOn = 0;
        b_pedvarsOff = 0;
    }
    b_elevationOn = fChain->GetBranch( "elevationOn" );
    b_azimuthOn = fChain->GetBranch( "azimuthOn" );
    b_elevationOff = fChain->GetBranch( "elevationOff" );
    b_azimuthOff = fChain->GetBranch( "azimuthOff" );
    b_RawRateOn = fChain->GetBranch( "RawRateOn" );
    b_RawRateOff = fChain->GetBranch( "RawRateOff" );
    b_NOn = fChain->GetBranch( "NOn" );
    b_NOff = fChain->GetBranch( "NOff" );
    b_NOffNorm = fChain->GetBranch( "NOffNorm" );
    b_OffNorm = fChain->GetBranch( "OffNorm" );
    b_Signi = fChain->GetBranch( "Signi" );
    b_Rate = fChain->GetBranch( "Rate" );
    b_RateE = fChain->GetBranch( "RateE" );
    b_RateOff = fChain->GetBranch( "RateOff" );
    b_RateOffE = fChain->GetBranch( "RateOffE" );
    b_DeadTimeFracOn = fChain->GetBranch( "DeadTimeFracOn" );
    b_DeadTimeFracOff = fChain->GetBranch( "DeadTimeFracOff" );
    b_MaxSigni = fChain->GetBranch( "MaxSigni" );
    b_MaxSigniX = fChain->GetBranch( "MaxSigniX" );
    b_MaxSigniY = fChain->GetBranch( "MaxSigniY" );

    return kTRUE;
}
