/*! \class VInstrumentResponseFunctionData
    \brief data class for instrumental response functions



*/


#include "VInstrumentResponseFunctionData.h"

VInstrumentResponseFunctionData::VInstrumentResponseFunctionData()
{
    fType = "";
    fType_numeric = 0;
    fName = "";
    fNTel = 0;
    fMCMaxCoreRadius = 0.;

    fData = 0;

    fListofResponseFunctionTypes.push_back( "angular_resolution" );    // fType_numeric == 0
    fListofResponseFunctionTypes.push_back( "core_resolution" );       // fType_numeric == 1
    fListofResponseFunctionTypes.push_back( "energy_resolution" );     // fType_numeric == 2

    setEnergyReconstructionMethod();

    fHistogramList = 0;
    setHistogramEbinning();
    setHistogramLogAngbinning();
    setArrayCentre();
}

bool VInstrumentResponseFunctionData::initialize( string iName, string iType, unsigned int iNTel, double iMCMaxCoreRadius )
{
    fType_numeric = testResponseFunctionType( iType );
    if( fType_numeric < 0 )
    {
        return false;
    }
    fType = iType;

    fName = iName;
    fNTel = iNTel;
    fMCMaxCoreRadius = iMCMaxCoreRadius;
    if( fMCMaxCoreRadius < 1.e-2 )
    {
        fMCMaxCoreRadius = 500.;
    }

    fHistogramList = new TList();

    // histograms
    vector< string > iHisName;
    vector< string > iHisXaxisName;
    vector< string > iHisYaxisName;
    vector< int >    iHisNbinsX;
    vector< double > iHisXmin;
    vector< double > iHisXmax;
    vector< int >    iHisNbinsY;
    vector< double > iHisYmin;
    vector< double > iHisYmax;
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE:   look at E_HISTOID before changing anything here
    //
    //         the sequence of the histogram definition must be
    //         exactly the same as the one of E_HISTOID
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // angular resolution plots
    if( fType == "angular_resolution" )
    {
        // angular difference vs. energy
        iHisName.push_back( "AngE0_" + fName );
        iHisXaxisName.push_back( "log_{10} energy [TeV]" );
        iHisYaxisName.push_back( "angular diff. (R,MC) [deg]" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 45000 );
        iHisYmin.push_back( 0. );
        iHisYmax.push_back( 5. );
        // (angular difference)^2 vs. energy
        iHisName.push_back( "AngE0_2_" + fName );
        iHisXaxisName.push_back( "log_{10} energy [TeV]" );
        iHisYaxisName.push_back( "(angular diff.)^{2} (R,MC) [deg^{2}]" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 45000 );
        iHisYmin.push_back( 0. );
        iHisYmax.push_back( 5. );
        // log(angular difference) vs reconstructed energy
        iHisName.push_back( "AngE0Log_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{rec} [TeV]" );
        iHisYaxisName.push_back( "log(angular diff. (R,MC) [deg])" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( fHistogrambinningAngular_Log );
        iHisYmin.push_back( fHistogrambinningAngular_Min_Log );
        iHisYmax.push_back( fHistogrambinningAngular_Max_Log );
        // angular resolution vs number of images per telescope
        iHisName.push_back( "AngNImages" + fName );
        iHisXaxisName.push_back( "number of images" );
        iHisYaxisName.push_back( "angular diff. (R,MC) [deg]" );
        iHisNbinsX.push_back( fNTel );
        iHisXmin.push_back( 0.5 );
        iHisXmax.push_back( 0.5 + fNTel );
        iHisNbinsY.push_back( 2500 );
        iHisYmin.push_back( 0. );
        iHisYmax.push_back( 5. );
        // angular resolution vs core distance
        iHisName.push_back( "AngCoreDistance" + fName );
        iHisXaxisName.push_back( "distance to array center [m]" );
        iHisYaxisName.push_back( "angular diff. (R,MC) [deg]" );
        iHisNbinsX.push_back( 50 );
        iHisXmin.push_back( 0. );
        iHisXmax.push_back( fMCMaxCoreRadius );
        iHisNbinsY.push_back( 2500 );
        iHisYmin.push_back( 0. );
        iHisYmax.push_back( 5. );
        // angular error vs. energy
        iHisName.push_back( "AngErrorE0_" + fName );
        iHisXaxisName.push_back( "log_{10} energy [TeV]" );
        iHisYaxisName.push_back( "angular error (R,MC) [deg]" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 2500 );
        iHisYmin.push_back(-5. );
        iHisYmax.push_back( 5. );
        // not defined here
        iHisName.push_back( "AngRelativeErrorE0_" + fName );
        iHisXaxisName.push_back( "log_{10} energy [TeV]" );
        iHisYaxisName.push_back( "relative angular error (R,MC)" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 1 );
        iHisYmin.push_back(-5. );
        iHisYmax.push_back( 5. );
        // angular difference vs. true energy
        iHisName.push_back( "AngEMC_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{MC} [TeV]" );
        iHisYaxisName.push_back( "angular diff. (R,MC) [deg]" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 9000 );
        iHisYmin.push_back( 0. );
        iHisYmax.push_back( 4.5 );
        // (angular difference)^2 vs. true energy
        iHisName.push_back( "AngEMC_2_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{MC} [TeV]" );
        iHisYaxisName.push_back( "(angular diff.)^{2} (R,MC) [deg^{2}]" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 9000 );
        iHisYmin.push_back( 0. );
        iHisYmax.push_back( 4.5 );
        // log(angular difference) vs true energy
        iHisName.push_back( "AngEMCLog_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{MC} [TeV]" );
        iHisYaxisName.push_back( "log(angular diff. (R,MC) [deg])" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( fHistogrambinningAngular_Log );
        iHisYmin.push_back( fHistogrambinningAngular_Min_Log );
        iHisYmax.push_back( fHistogrambinningAngular_Max_Log );
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////
    // core resolution plots
    else if( fType == "core_resolution" )
    {
        // core position difference vs. reconstructed energy
        iHisName.push_back( "CoreE0_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{rec} [TeV]" );
        iHisYaxisName.push_back( "core position diff. (R,MC) [m]" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 300 );
        iHisYmin.push_back( 0. );
        iHisYmax.push_back( 300. );
        // (core position difference)^2 vs. reconstructed energy
        iHisName.push_back( "CoreE0_2_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{rec} [TeV]" );
        iHisYaxisName.push_back( "(core position diff.)^{2} (R,MC) [m^{2}]" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 900 );
        iHisYmin.push_back( 0. );
        iHisYmax.push_back( 300.*300. );
        // log(angular difference) vs reconstructed energy
        iHisName.push_back( "CoreE0Log_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{rec} [TeV]" );
        iHisYaxisName.push_back( "log(core position diff. (R,MC) [m]" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 100 );
        iHisYmin.push_back(-1. );
        iHisYmax.push_back( 3. );
        // core position resolution vs number of images per telescope
        iHisName.push_back( "CoreNImages" + fName );
        iHisXaxisName.push_back( "number of images" );
        iHisYaxisName.push_back( "core position diff. (R,MC) [m]" );
        iHisNbinsX.push_back( fNTel );
        iHisXmin.push_back( 0.5 );
        iHisXmax.push_back( 0.5 + fNTel );
        iHisNbinsY.push_back( 300 );
        iHisYmin.push_back( 0. );
        iHisYmax.push_back( 300. );
        // core position resolution vs core distance
        iHisName.push_back( "CoreCoreDistance" + fName );
        iHisXaxisName.push_back( "distance to array center [m]" );
        iHisYaxisName.push_back( "core position diff. (R,MC) [m]" );
        iHisNbinsX.push_back( 50 );
        iHisXmin.push_back( 0. );
        iHisXmax.push_back( fMCMaxCoreRadius );
        iHisNbinsY.push_back( 300 );
        iHisYmin.push_back( 0. );
        iHisYmax.push_back( 300. );
        // core position error vs energy
        iHisName.push_back( "CoreErrorE0_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{rec} [TeV]" );
        iHisYaxisName.push_back( "core position error (R,MC) [m]" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 600 );
        iHisYmin.push_back(-300. );
        iHisYmax.push_back( 300. );
        // not defined here
        iHisName.push_back( "CoreRelativeErrorE0_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{rec} [TeV]" );
        iHisYaxisName.push_back( "core position angular error (R,MC)" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 1 );
        iHisYmin.push_back(-5. );
        iHisYmax.push_back( 5. );
        // core position difference vs. true energy
        iHisName.push_back( "CoreEMC_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{MC} [TeV]" );
        iHisYaxisName.push_back( "core position diff. (R,MC) [m]" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 300 );
        iHisYmin.push_back( 0. );
        iHisYmax.push_back( 300. );
        // (core position difference)^2 vs. true energy
        iHisName.push_back( "CoreEMC_2_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{MC} [TeV]" );
        iHisYaxisName.push_back( "(core position diff.)^{2} (R,MC) [m^{2}]" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 900 );
        iHisYmin.push_back( 0. );
        iHisYmax.push_back( 300.*300. );
        // log(angular difference) vs true energy
        iHisName.push_back( "CoreEMCLog_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{MC} [TeV]" );
        iHisYaxisName.push_back( "log(core position diff. (R,MC) [m]" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 100 );
        iHisYmin.push_back(-1. );
        iHisYmax.push_back( 3. );
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////
    // energy resolution plots
    else if( fType == "energy_resolution" )
    {
        // energy difference vs. reconstructed energy
        iHisName.push_back( "EnergE0_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{rec} [TeV]" );
        iHisYaxisName.push_back( "log_{10} E_{rec} - log_{10} E_{MC}" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 4500 );
        iHisYmin.push_back(-2. );
        iHisYmax.push_back( 2. );
        // (energy difference)^2 vs. reconstructed energy
        iHisName.push_back( "EnergE0_2_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{rec} [TeV]" );
        iHisYaxisName.push_back( "(log_{10} E_{rec} - log_{10} E_{MC})^2" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 4500 );
        iHisYmin.push_back( 0. );
        iHisYmax.push_back( 4. );
        // log(angular difference) vs true energy (probably useless)
        iHisName.push_back( "EnergyE0Log_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{rec} [TeV]" );
        iHisYaxisName.push_back( "log(log_{10} E_{rec} - log_{10} E_{MC})" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 100 );
        iHisYmin.push_back(-1. );
        iHisYmax.push_back( 3. );
        // energy resolution vs number of images per telescope
        iHisName.push_back( "EnergNImages" + fName );
        iHisXaxisName.push_back( "number of images" );
        iHisYaxisName.push_back( "log_{10} E_{rec} - log_{10} E_{MC}" );
        iHisNbinsX.push_back( fNTel );
        iHisXmin.push_back( 0.5 );
        iHisXmax.push_back( 0.5 + fNTel );
        iHisNbinsY.push_back( 2500 );
        iHisYmin.push_back(-2. );
        iHisYmax.push_back( 2. );
        // energy resolution vs core distance
        iHisName.push_back( "EnergCoreDistance" + fName );
        iHisXaxisName.push_back( "distance to array center [m]" );
        iHisYaxisName.push_back( "log_{10} E_{rec} - log_{10} E_{MC}" );
        iHisNbinsX.push_back( 50 );
        iHisXmin.push_back( 0. );
        iHisXmax.push_back( fMCMaxCoreRadius );
        iHisNbinsY.push_back( 2500 );
        iHisYmin.push_back(-2. );
        iHisYmax.push_back( 2. );
        // energy reconstruction error vs. energy (used for energy systematics)
        iHisName.push_back( "EnergErrorE0_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{rec} [TeV]" );
        iHisYaxisName.push_back( "log_{10} E_{rec} - log_{10} E_{MC}" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 2500 );
        iHisYmin.push_back(-2. );
        iHisYmax.push_back( 2. );
        // not defined here
        iHisName.push_back( "EnergyRelativeErrorE0_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{rec} [TeV]" );
        iHisYaxisName.push_back( "#Delta energy resolution" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 2500 );
        iHisYmin.push_back(-2. );
        iHisYmax.push_back( 2. );
        // energy difference vs. true energy
        iHisName.push_back( "EnergEMC_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{MC} [TeV]" );
        iHisYaxisName.push_back( "log_{10} E_{rec} - log_{10} E_{MC}" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 4500 );
        iHisYmin.push_back(-2. );
        iHisYmax.push_back( 2. );
        // (energy difference)^2 vs. true energy
        iHisName.push_back( "EnergEMC_2_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{MC} [TeV]" );
        iHisYaxisName.push_back( "(log_{10} E_{rec} - log_{10} E_{MC})^2" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 4500 );
        iHisYmin.push_back( 0. );
        iHisYmax.push_back( 4. );
        // log(angular difference) vs true energy (probably useless)
        iHisName.push_back( "EnergyEMCLog_" + fName );
        iHisXaxisName.push_back( "log_{10} energy_{MC} [TeV]" );
        iHisYaxisName.push_back( "log(log_{10} E_{rec} - log_{10} E_{MC})" );
        iHisNbinsX.push_back( fHistogrambinningEnergy_TeV_Log );
        iHisXmin.push_back( fHistogrambinningEnergy_Min_Tev_Log );
        iHisXmax.push_back( fHistogrambinningEnergy_Max_Tev_Log );
        iHisNbinsY.push_back( 100 );
        iHisYmin.push_back(-1. );
        iHisYmax.push_back( 3. );
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////
    // create histograms
    /////////////////////////////////////////////////////////////////////////////////////////////////
    for( unsigned int i = 0; i < iHisName.size(); i++ )
    {
        // 2D histo
        f2DHisto.push_back( new TH2D(( "h" + iHisName[i] ).c_str(), "", iHisNbinsX[i], iHisXmin[i], iHisXmax[i], iHisNbinsY[i], iHisYmin[i], iHisYmax[i] ) );
        f2DHisto.back()->SetXTitle( iHisXaxisName[i].c_str() );
        f2DHisto.back()->SetYTitle( iHisYaxisName[i].c_str() );
        fHistogramList->Add( f2DHisto.back() );

        // corresponding resolution graph
        fResolutionGraph.push_back( new TGraphErrors( 1 ) );
        fResolutionGraph.back()->SetName(( "g" + iHisName[i] ).c_str() );
        fResolutionGraph.back()->SetTitle();
        fHistogramList->Add( fResolutionGraph.back() );

        // containment probability
        fContainmentProbability.push_back( 0. );

    }

    return true;
}

int VInstrumentResponseFunctionData::testResponseFunctionType( string iType )
{
    for( unsigned int i = 0; i < fListofResponseFunctionTypes.size(); i++ )
    {
        if( iType == fListofResponseFunctionTypes[i] )
        {
            return ( int )i;
        }
    }

    cout << "VInstrumentResponseFunctionData::testResponseFunctionType() error: type not found: " << iType << endl;
    cout << "\t available response function types are: " << endl;
    for( unsigned int i = 0; i < fListofResponseFunctionTypes.size(); i++ )
    {
        cout << "\t" << fListofResponseFunctionTypes[i] << endl;
    }

    return -99;
}

/*
    expect that all quality checks happened before
*/
void VInstrumentResponseFunctionData::fill( double iWeight )
{
    if(!fData )
    {
        return;
    }

    // default is true here
    bool bPlotResolution_vs_reconstructedEnergy = true;

    // simple quality check (FOV shouldn't be larger than 50 deg)
    if( fData->Xoff < -50. || fData->Yoff < -50. )
    {
        return;
    }

    // get reconstructed energy
    double iErec_lin = -99.e6;
    if( fEnergyReconstructionMethod == 0 && fData->Erec > 0. )
    {
        iErec_lin = fData->Erec;
    }
    else if( fEnergyReconstructionMethod == 1 && fData->ErecS > 0. )
    {
        iErec_lin = fData->ErecS;
    }
    if( iErec_lin < 0. )
    {
        return;
    }

    double iDiff = -99.e6;
    double iError = -99.e6;
    double iErrorRelative = -99.e6;
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // angular resolution
    if( fType_numeric == 0 )
    {
        // angular difference
        iDiff = sqrt(( fData->Xoff - fData->MCxoff ) * ( fData->Xoff - fData->MCxoff ) +
                     ( fData->Yoff - fData->MCyoff ) * ( fData->Yoff - fData->MCyoff ) );
        // error
        iError = sqrt( fData->Xoff* fData->Xoff + fData->Xoff* fData->Xoff ) -
                 sqrt( fData->MCxoff* fData->MCxoff + fData->MCyoff* fData->MCyoff );
        // relative error (not sure if it is useful)
        iErrorRelative = -99.e6;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // core resolution
    else if( fType_numeric == 1 )
    {
        // core difference
        iDiff = sqrt(( fData->Xcore - fData->MCxcore ) * ( fData->Xcore - fData->MCxcore ) +
                     ( fData->Ycore - fData->MCycore ) * ( fData->Ycore - fData->MCycore ) );
        // core error
        iError = sqrt( fData->Xcore* fData->Xcore + fData->Xcore* fData->Xcore ) -
                 sqrt( fData->MCxcore* fData->MCxcore + fData->MCycore* fData->MCycore );
        // relative error
        if( sqrt( fData->MCxcore * fData->MCxcore + fData->MCycore * fData->MCycore ) > 0. )
        {
            iErrorRelative = iError / sqrt( fData->MCxcore* fData->MCxcore + fData->MCycore* fData->MCycore );
        }
        else
        {
            iErrorRelative = -99.e6;
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // energy resolution
    else if( fType_numeric == 2 )
    {
        if( fData->MCe0 > 0. )
        {
            iDiff = TMath::Abs( 1. - iErec_lin / fData->MCe0 );
        }
        iError = iDiff;
        if( fData->MCe0 > 0. )
        {
            iErrorRelative = ( iErec_lin - fData->MCe0 ) / fData->MCe0;
        }
        else
        {
            iErrorRelative = -99.e6;
        }
    }

    if(!bPlotResolution_vs_reconstructedEnergy )
    {
        iErec_lin = fData->MCe0;
    }


    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // fill histograms

    // difference vs energy
    if( E_DIFF < f2DHisto.size() && f2DHisto[E_DIFF] )
    {
        f2DHisto[E_DIFF]->Fill( log10( iErec_lin ), iDiff, iWeight );
    }
    // difference vs true energy
    if( E_DIFF_MC < f2DHisto.size() && f2DHisto[E_DIFF_MC] )
    {
        f2DHisto[E_DIFF_MC]->Fill( log10( fData->MCe0 ), iDiff, iWeight );
    }
    // squared difference vs energy
    if( E_DIFF2 < f2DHisto.size() && f2DHisto[E_DIFF2] )
    {
        f2DHisto[E_DIFF2]->Fill( log10( iErec_lin ), iDiff* iDiff, iWeight );
    }
    // squared difference vs true energy
    if( E_DIFF2_MC < f2DHisto.size() && f2DHisto[E_DIFF2_MC] )
    {
        f2DHisto[E_DIFF2_MC]->Fill( log10( fData->MCe0 ), iDiff* iDiff, iWeight );
    }
    // log10 difference vs energy
    if( E_LOGDIFF < f2DHisto.size() && f2DHisto[E_LOGDIFF] && iDiff > 0. )
    {
        f2DHisto[E_LOGDIFF]->Fill( log10( iErec_lin ), log10( iDiff ), iWeight );
    }
    // log10 difference vs true energy
    if( E_LOGDIFF_MC < f2DHisto.size() && f2DHisto[E_LOGDIFF_MC] && iDiff > 0. )
    {
        f2DHisto[E_LOGDIFF_MC]->Fill( log10( fData->MCe0 ), log10( iDiff ), iWeight );
    }
    // difference vs number of images
    if( E_NIMAG < f2DHisto.size() && f2DHisto[E_NIMAG] )
    {
        f2DHisto[E_NIMAG]->Fill( fData->NImages, iDiff, iWeight );
    }

    // difference vs core distance
    if( E_DIST < f2DHisto.size() && f2DHisto[E_DIST] )
    {
        f2DHisto[E_DIST]->Fill( sqrt(( fData->MCxcore - fArrayCentre_X ) * ( fData->MCxcore - fArrayCentre_X ) +
                                     ( fData->MCycore - fArrayCentre_Y ) * ( fData->MCycore - fArrayCentre_Y ) ), iDiff, iWeight );
    }

    // error vs energy
    if( E_ERROR < f2DHisto.size() && f2DHisto[E_ERROR] )
    {
        f2DHisto[E_ERROR]->Fill( log10( iErec_lin ), iError, iWeight );
    }

    // relative error vs energy
    if( E_RELA < f2DHisto.size() && f2DHisto[E_RELA] && iErrorRelative > -98.e6 )
    {
        f2DHisto[E_RELA]->Fill( log10( iErec_lin ), iErrorRelative, iWeight );
    }
}


bool VInstrumentResponseFunctionData::terminate( double iContainmentProbability, double iContainmentProbabilityError )
{
    // calculate XX% values (default is 68%)
    for( unsigned int i = 0; i < f2DHisto.size(); i++ )
    {
        fContainmentProbability[i] = iContainmentProbability;
        if( i != E_RELA )
        {
            calculateResolution( f2DHisto[i], fResolutionGraph[i], f2DHisto[i]->GetName(), iContainmentProbability, iContainmentProbabilityError );
        }
        // for relative plots get mean and spread from each bin in the histogram
        else
        {
            get_Profile_from_TH2D( f2DHisto[i], fResolutionGraph[i], "meanS" );
        }
    }

    return true;
}

/*!
    calculate ithresh (usually 68%) reconstruction accuracy from 2D histogram
*/
TList*  VInstrumentResponseFunctionData::calculateResolution( TH2D* iHistogram, TGraphErrors* iResult, string iHistoName, double iContainmentProbability, double iContainmentProbabilityError )
{
    if(!iHistogram || !iResult )
    {
        return 0;
    }

    TH1D* iTemp = 0;
    TList* hList = new TList();

    char iname[800];

    // set number of points in graph
    iResult->Set( iHistogram->GetNbinsX() );

    // temporary vectors
    vector< double > vEnergy;
    vector< double > vRes;
    vector< double > vResE;

    double i_energy = 0.;

    //////////////////////////////////////////////////////////////////////////////
    // loop over all energy bins and project each bin into a TH1D
    for( int i = 1; i <= iHistogram->GetNbinsX(); i++ )
    {

        // define temporary histogram and fill with projection
        if( iHistoName.size() > 0 )
        {
            sprintf( iname, "%s_%d", iHistoName.c_str(), i );
        }
        else
        {
            sprintf( iname, "iH_%d", i );
        }
        iTemp = iHistogram->ProjectionY( iname, i, i );
        sprintf( iname, "log_{10} E_{0} = %.2f", iHistogram->GetXaxis()->GetBinCenter( i ) );
        iTemp->SetTitle( iname );

        i_energy = iHistogram->GetXaxis()->GetBinCenter( i );

        //////////////////////////////////////////////////////////
        // calculate containment
        double iTotSum = 0.;
        // get total number of events in histogram
        for( int j = 1; j <= iTemp->GetNbinsX(); j++ )
        {
            iTotSum += iTemp->GetBinContent( j );
        }
        if( iTotSum  > 0. )
        {
            double iTempSum = 0.;
            for( int j = 1; j <= iTemp->GetNbinsX(); j++ )
            {
                iTempSum += iTemp->GetBinContent( j );
                if( iTempSum / iTotSum  > iContainmentProbability )
                {
                    vEnergy.push_back( i_energy );
                    vRes.push_back( iTemp->GetBinCenter( j ) );
                    //	       vResE.push_back( getResolutionErrorfromToyMC( iTemp->GetBinCenter( j ), iTemp->GetEntries() ) );
                    // require at least 20 events to calculate a good RMS
                    if( iTemp->GetEntries() > 20. )
                    {
                        vResE.push_back( iTemp->GetRMS() / sqrt( iTemp->GetEntries() ) );
                    }
                    else
                    {
                        vResE.push_back( 0. );
                    }
                    break;
                }
            }
        }
        if( iHistoName.size() > 0 )
        {
            hList->Add( iTemp );
        }
        else if( iTemp )
        {
            delete iTemp;
        }
    }

    // fill graph
    iResult->Set(( int )vEnergy.size() );
    for( unsigned i = 0; i < vEnergy.size(); i++ )
    {
        iResult->SetPoint( i, vEnergy[i], vRes[i] );
        iResult->SetPointError( i, 0., vResE[i] );
    }

    return hList;
}

double VInstrumentResponseFunctionData::getResolutionErrorfromToyMC( double i68, double iN )
{
    if( i68 < 0. || iN < 1. )
    {
        return 0.;
    }

    // number of times to run the experiment
    const int nRun = 100;

    // histogram with results from each experiment
    TH1D h68( "h68", "h68", 1000, 0., 1.5 );

    // histogram with angular differences
    TH1D hDiff( "hDiff", "", 1000, 0., 1.0 );

    // normal distribution
    TF1 f( "f", "gaus(0)", 0., 5. );
    f.SetParameter( 0, 1. );
    f.SetParameter( 1, 0. );
    // normalized to 2D distribution, see Minuit table 7.1
    f.SetParameter( 2, i68 / sqrt( 2.41 ) );

    double x = 0;
    double y = 0.;
    double q[] = { 0 };
    int nq = 1;
    double d[] = { 0.68 };

    for( int j = 0; j < nRun; j++ )
    {
        hDiff.Reset();
        for( int i = 0; i < iN; i++ )
        {
            x = f.GetRandom();
            y = f.GetRandom();

            hDiff.Fill( sqrt( x* x + y* y ) );
        }
        if( hDiff.GetEntries() > 0 )
        {
            hDiff.GetQuantiles( nq, q, d );
            h68.Fill( q[0] );
        }
    }

    return h68.GetRMS();
}


void VInstrumentResponseFunctionData::setData( double iZe, int iAz_bin, double iAz_min, double iAz_max, int iNoise, double iPedvars, double iIndex, double iXoff, double iYoff )
{
    fZe = iZe;
    fAz_bin = iAz_bin;
    fAz_min = iAz_min;
    fAz_max = iAz_max;
    fXoff = iXoff;
    fYoff = iYoff;
    fWobble = sqrt( fXoff* fXoff + fYoff* fYoff );
    fNoise = iNoise;
    fPedvars = iPedvars;
    fSpectralIndex = iIndex;
}

void VInstrumentResponseFunctionData::setPlottingStyle( int icolor, double iwidth, int imarker, double isize, int iFillStyle, int iLineStyle )
{
    /*   for( unsigned int i = 0; i < fResolutionGraph.size(); i++ )
       {
          if( fResolutionGraph[i] ) setGraphPlottingStyle( fResolutionGraph[i], icolor, iwidth, imarker, isize, iFillStyle, iLineStyle );
       } */
}
