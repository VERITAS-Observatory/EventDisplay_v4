/*! \class VTMVAEvaluator
    \brief use a TMVA weight file for energy dependent gamma/hadron separation

*/

#include "VTMVAEvaluator.h"

VTMVAEvaluator::VTMVAEvaluator()
{
    fIsZombie = false;

    setDebug();

    fData = 0;
    fTMVAEvaluatorResults = 0;
    fAverageZenithPerRun = -9999.;

    reset();
}

void VTMVAEvaluator::reset()
{
    // variables used for gamma/hadron separation
    fNImages = 0.;
    fMSCW = 0.;
    fMSCL = 0.;
    fMWR = 0.;
    fMLR = 0.;
    fEmissionHeight = 0.;
    fEmissionHeightChi2_log10 = 0.;
    fEChi2S = 0.;
    fEChi2S_log10 = 0.;
    fdES = 0.;
    fSizeSecondMax_log10 = 0;
    fCoreDist = 0.;
    fDispDiff = 0.;
    fDispDiff_log10 = 0.;
    fDispAbsSumWeigth = 0.;
    fDummy = 0.;
    fWidth_r1 = fWidth_r2 = fWidth_r3 = fWidth_r4 = 0.0f;
    fLength_r1 = fLength_r2 = fLength_r3 = fLength_r4 = 0.0f;
    fLoss_r1 = fLoss_r2 = fLoss_r3 = fLoss_r4 = 0.0f;
    fRcore_r1 = fRcore_r2 = fRcore_r3 = fRcore_r4 = 0.0f;

    setTMVACutValue();
    setSignalEfficiency();
    setSpectralIndexForEnergyWeighting();
    setParticleNumberFile();
    setPlotEfficiencyPlotsPerBin();
    setPrintPlotting();
    setSensitivityOptimizationParameters();
    setSensitivityOptimizationFixedSignalEfficiency();
    setSensitivityOptimizationMinSourceStrength();
    setTMVAMethod();
    setTMVAErrorFraction();
    fTMVA_EvaluationResult = -99.;
    fTMVACutValueNoVec = -99.;

    setSmoothAndInterPolateMVAValues();
}

/*

    get list of training variables from XML file

*/
vector< string > VTMVAEvaluator::getTrainingVariables( string iXMLFile, vector< bool >& iSpectator )
{
    vector< string > iVar;
    if( fDebug )
    {
        cout << endl;
        cout << "reading list of variables from TMVA XML file: " << iXMLFile << endl;
    }
    // open TMVA XML file
    // NOTE: extreme dependence on the structure of the TMVA XML file
    ifstream is;
    is.open( iXMLFile.c_str(), ifstream::in );
    if(!is )
    {
        cout << "VTMVAEvaluator::getTrainingVariable error: cannot open TMVA weight file: " << iXMLFile << endl;
        return iVar;
    }
    string is_line;
    string iTemp;

    int nVar = 0;

    while( getline( is, is_line ) )
    {

        // number of variables
        if( is_line.find( "NVar=\"" ) != string::npos )
        {
            nVar = atoi( is_line.substr( is_line.find( "NVar=\"" ) + 6, is_line.size() - is_line.find( "\">" ) - 1 ).c_str() );
            if( fDebug )
            {
                cout << "\t reading TMVA XML file: number of variables is " << nVar << endl;
            }
        }
        // AGAIN, NOTE: extreme dependence on the structure of the TMVA XML file
        if( is_line.find( "Expression=\"" ) != string::npos )
        {
            iVar.push_back( is_line.substr( is_line.find( "Expression=\"" ) + 12, is_line.find( "Label=" ) -
                                            is_line.find( "Expression=\"" ) - 14 ) );
            if( is_line.find( "SpecIndex" ) != string::npos )
            {
                iSpectator.push_back( true );
            }
            else
            {
                iSpectator.push_back( false );
            }
            if( fDebug )
            {
                cout << "\t reading TMVA XML file: new variable: " << iVar.back() << endl;
            }
        }
    }
    is.close();

    return iVar;
}

/*
 * return BDT file name
 *
*/
string VTMVAEvaluator::getBDTFileName( string iWeightFileName, unsigned int i_E_index, unsigned int i_Z_index, string iSuffix )
{
    ostringstream iFullFileName;
    if( iWeightFileName.size() > 0 )
    {
        iFullFileName << iWeightFileName;
    }
    else
    {
        iFullFileName << fTMVAMethodName;
    }
    iFullFileName << "_" << i_E_index;
    iFullFileName << "_" << i_Z_index;
    if( iSuffix.find( "xml" ) != string::npos )
    {
        iFullFileName << "_" << fTMVAMethodName;
    }
    iFullFileName << iSuffix;
    return iFullFileName.str();
}

/*

    initialize TMVA readers

*/
bool VTMVAEvaluator::initializeWeightFiles( string iWeightFileName,
        unsigned int iWeightFileIndex_Emin, unsigned int iWeightFileIndex_Emax,
        unsigned int iWeightFileIndex_Zmin, unsigned int iWeightFileIndex_Zmax )
{
    //////////////////////////////
    // sanity checks
    if( iWeightFileName.size() == 0 )
    {
        cout << "VTMVAEvaluator::initializeWeightFiles error: no file name" << endl;
        fIsZombie = true;
        return false;
    }
    if( iWeightFileIndex_Emin > iWeightFileIndex_Emax )
    {
        cout << "VTMVAEvaluator::initializeWeightFiles: min energy bin larger than maximum: ";
        cout << iWeightFileIndex_Emin << " > " << iWeightFileIndex_Emax << endl;
        fIsZombie = true;
        return false;
    }
    if( iWeightFileIndex_Zmin > iWeightFileIndex_Zmax )
    {
        cout << "VTMVAEvaluator::initializeWeightFiles: min zenith bin larger than maximum: ";
        cout << iWeightFileIndex_Zmin << " > " << iWeightFileIndex_Zmax << endl;
        fIsZombie = true;
        return false;
    }

    //////////////////////////////
    // reset data vector
    fTMVAData.clear();

    /////////////////////////////////////////
    // number of energy and zenith bins bins
    unsigned int iNbinE = iWeightFileIndex_Emax - iWeightFileIndex_Emin + 1;
    unsigned int iNbinZ = iWeightFileIndex_Zmax - iWeightFileIndex_Zmin + 1;

    cout << "VTMVAEvaluator::initializeWeightFiles: reading energy and zenith bins from TMVA root files ";
    cout << "(nbinE: " << iNbinE << ", nbinZ: " << iNbinZ << ")" << endl;

    // Moved into helper for readability
    if(!buildTMVADataFromFiles( iWeightFileName, iWeightFileIndex_Emin, iWeightFileIndex_Emax,
                                iWeightFileIndex_Zmin, iWeightFileIndex_Zmax ) )
    {
        return false;
    }

    // initialize TMVA readers (moved into helper for readability)
    if(!setupTMVAReaders( iWeightFileIndex_Emin, iWeightFileIndex_Emax,
                          iWeightFileIndex_Zmin, iWeightFileIndex_Zmax ) )
    {
        return false;
    }

    // smooth and interpolate
    if( fParticleNumberFileName.size() > 0 && fSmoothAndInterpolateMVAValues )
    {
        smoothAndInterPolateMVAValue( 0, 0,
                                      iWeightFileIndex_Emin, iWeightFileIndex_Emax,
                                      iWeightFileIndex_Zmin, iWeightFileIndex_Zmax );
    }

    // print some info to screen
    cout << "VTMVAEvaluator: Initialized " << fTMVAData.size() << " MVA readers " << endl;

    fillTMVAEvaluatorResults();

    return true;
}

// Helper: build TMVA data by scanning root/xml files and pushing per-bin metadata
bool VTMVAEvaluator::buildTMVADataFromFiles( const std::string& iWeightFileName,
        unsigned int iWeightFileIndex_Emin, unsigned int iWeightFileIndex_Emax,
        unsigned int iWeightFileIndex_Zmin, unsigned int iWeightFileIndex_Zmax )
{
    char hname[800];
    unsigned int iNbinE = iWeightFileIndex_Emax - iWeightFileIndex_Emin + 1;
    unsigned int iNbinZ = iWeightFileIndex_Zmax - iWeightFileIndex_Zmin + 1;

    cout << "VTMVAEvaluator::initializeWeightFiles: reading energy and zenith bins from TMVA root files ";
    cout << "(nbinE: " << iNbinE << ", nbinZ: " << iNbinZ << ")" << endl;

    unsigned int iMinMissingBin = 0;
    for( unsigned int i = 0; i < iNbinE; i++ )
    {
        unsigned int jMinMissingBin = 0;
        for( unsigned int j = 0; j < iNbinZ; j++ )
        {
            string iTMVAName = getBDTFileName( "",
                                               iWeightFileIndex_Emin + i, iWeightFileIndex_Zmin + j );
            string iFullFileName = getBDTFileName( iWeightFileName,
                                                   iWeightFileIndex_Emin + i, iWeightFileIndex_Zmin + j, ".root" );
            string iFullFileNameXML = getBDTFileName( iWeightFileName,
                                      iWeightFileIndex_Emin + i, iWeightFileIndex_Zmin + j, "_0.weights.xml" );

            bool bGoodRun = true;
            VTMVARunDataEnergyCut* iEnergyData = 0;
            VTMVARunDataZenithCut* iZenithData = 0;
            if( gSystem->AccessPathName( iFullFileName.c_str() ) )
            {
                bGoodRun = false;
            }
            else
            {
                TFile iF( iFullFileName.c_str() );
                if( iF.IsZombie() )
                {
                    bGoodRun = false;
                }
                else
                {
                    iEnergyData = ( VTMVARunDataEnergyCut* )iF.Get( "fDataEnergyCut" );
                    iZenithData = ( VTMVARunDataZenithCut* )iF.Get( "fDataZenithCut" );
                    if(!iEnergyData || !iZenithData )
                    {
                        cout << "  No energy or zenith cut data" << endl;
                        bGoodRun = false;
                    }
                    sprintf( hname, "Method_%s/%s_0/MVA_%s_0_effS",
                             fTMVAMethodName.c_str(), fTMVAMethodName.c_str(), fTMVAMethodName.c_str() );
                    if(!iF.Get( hname ) )
                    {
                        cout << "  No signal efficiency histogram found (" << hname << ")" << endl;
                        bGoodRun = false;
                    }
                }
                if(!bGoodRun )
                {
                    if( i == iMinMissingBin || j == jMinMissingBin )
                    {
                        cout << "VTMVAEvaluator::initializeWeightFiles() warning: ";
                        cout << "TMVA root file not found or incomplete file (" << i << ") " << endl;
                        cout << iFullFileName << endl;
                        if( i == iMinMissingBin )
                        {
                            cout << "  assume this is a low-energy empty bin (bin number " << i << ";";
                            cout << " number of missing energy bins: " << iMinMissingBin + 1 << ")" << endl;
                            iMinMissingBin++;
                        }
                        if( j == jMinMissingBin )
                        {
                            cout << "  assume this is a zenith empty bin (bin number " << j << ";";
                            cout << " number of missing zenith bins: " << jMinMissingBin + 1 << ")" << endl;
                            jMinMissingBin++;
                        }
                        continue;
                    }
                    else if( i == ( iWeightFileIndex_Emax ) || j == ( iWeightFileIndex_Zmax ) )
                    {
                        cout << "VTMVAEvaluator::initializeWeightFiles(): TMVA root file not found " << iFullFileName << endl;
                        if( i == ( iWeightFileIndex_Emax ) )
                        {
                            cout << "  assume this is a high-energy empty bin (bin number " << i << ")" << endl;
                        }
                        if( j == ( iWeightFileIndex_Zmax ) )
                        {
                            cout << "  assume this is a high-zenith empty bin (bin number " << j << ")" << endl;
                        }
                        continue;
                    }
                    else
                    {
                        cout << "VTMVAEvaluator::initializeWeightFiles: warning: problem while initializing energies from TMVA root file ";
                        cout << iFullFileName << endl;
                        cout << "(this might be not a problem if the sensitive energy range of the given array is relatively small)" << endl;
                        continue;
                    }
                }
                if(!iEnergyData )
                {
                    cout << "VTMVAEvaluator::initializeWeightFiles: warning: problem while reading energies from TMVA root file ";
                    cout << iFullFileName << endl;
                    fIsZombie = true;
                    return false;
                }

                fTMVAData.push_back( new VTMVAEvaluatorData() );
                fTMVAData.back()->fEnergyBin = i;
                fTMVAData.back()->fZenithBin = j;
                fTMVAData.back()->fEnergyCut_Log10TeV_min = iEnergyData->fEnergyCut_Log10TeV_min;
                fTMVAData.back()->fEnergyCut_Log10TeV_max = iEnergyData->fEnergyCut_Log10TeV_max;
                fTMVAData.back()->fSpectralWeightedMeanEnergy_Log10TeV =
                    VMathsandFunctions::getSpectralWeightedMeanEnergy( fTMVAData.back()->fEnergyCut_Log10TeV_min,
                            fTMVAData.back()->fEnergyCut_Log10TeV_max,
                            fSpectralIndexForEnergyWeighting );
                if( iZenithData )
                {
                    fTMVAData.back()->fZenithCut_min = iZenithData->fZenithCut_min;
                    fTMVAData.back()->fZenithCut_max = iZenithData->fZenithCut_max;
                }
                else
                {
                    fTMVAData.back()->fZenithCut_min = 0.;
                    fTMVAData.back()->fZenithCut_max = 90.;
                }

                fTMVAData.back()->fSignalEfficiency = getSignalEfficiency( iWeightFileIndex_Emin + i,
                                                      iEnergyData->fEnergyCut_Log10TeV_min,
                                                      iEnergyData->fEnergyCut_Log10TeV_max,
                                                      iWeightFileIndex_Zmin + j,
                                                      fTMVAData.back()->fZenithCut_min,
                                                      fTMVAData.back()->fZenithCut_max );
                fTMVAData.back()->fTMVACutValue = getTMVACutValue( iWeightFileIndex_Emin + i,
                                                  iEnergyData->fEnergyCut_Log10TeV_min,
                                                  iEnergyData->fEnergyCut_Log10TeV_max,
                                                  iWeightFileIndex_Zmin + j,
                                                  fTMVAData.back()->fZenithCut_min,
                                                  fTMVAData.back()->fZenithCut_max );
                fTMVAData.back()->fBackgroundEfficiency = -99.;
                fTMVAData.back()->fTMVAOptimumCutValueFound = false;
                fTMVAData.back()->fSourceStrengthAtOptimum_CU = 0.;

                sprintf( hname, "bin %d, %.2f < log10(E) < %.2f, %.2f < Ze < %.2f)",
                         ( int )( fTMVAData.size() - 1 ), fTMVAData.back()->fEnergyCut_Log10TeV_min, fTMVAData.back()->fEnergyCut_Log10TeV_max,
                         fTMVAData.back()->fZenithCut_min, fTMVAData.back()->fZenithCut_max );
                fTMVAData.back()->SetTitle( hname );

                sprintf( hname, "%d%d", i, j );
                fTMVAData.back()->fTMVAMethodTag = hname;
                sprintf( hname, "%d_%d", i, j );
                fTMVAData.back()->fTMVAMethodTag_2 = hname;
                fTMVAData.back()->fTMVAName = iTMVAName;
                fTMVAData.back()->fTMVAFileName = iFullFileName;
                fTMVAData.back()->fTMVAFileNameXML = iFullFileNameXML;
            }
        }
    }

    if( fTMVAData.size() == 0 )
    {
        fIsZombie = true;
        return false;
    }
    for( unsigned int i = 0; i < fTMVAData.size(); i++ )
    {
        fTMVAData[i]->print();
    }
    return true;
}

// Helper: bind one variable or spectator to reader by name (same mapping as original)
void VTMVAEvaluator::bindVariableByName( TMVA::Reader* reader, const std::string& varName, bool isSpectator )
{
    if( isSpectator )
    {
        reader->AddSpectator( varName.c_str(), &fDummy );
        return;
    }
    if( varName == "MSCW" ) reader->AddVariable( "MSCW", &fMSCW );
    else if( varName == "MSCL" ) reader->AddVariable( "MSCL", &fMSCL );
    else if( varName == "EmissionHeight" ) reader->AddVariable( "EmissionHeight", &fEmissionHeight );
    else if( varName == "log10(EmissionHeightChi2)" ) reader->AddVariable( "log10(EmissionHeightChi2)", &fEmissionHeightChi2_log10 );
    else if( varName == "NImages" ) reader->AddVariable( "NImages", &fNImages );
    else if( varName == "dE" ) reader->AddVariable( "dE", &fdES );
    else if( varName == "dES" ) reader->AddVariable( "dES", &fdES );
    else if( varName == "log10(SizeSecondMax)" ) reader->AddVariable( "log10(SizeSecondMax)", &fSizeSecondMax_log10 );
    else if( varName == "EChi2S" ) reader->AddVariable( "EChi2S", &fEChi2S );
    else if( varName == "log10(EChi2S)" ) reader->AddVariable( "log10(EChi2S)", &fEChi2S_log10 );
    else if( varName == "sqrt(Xcore*Xcore+Ycore*Ycore)" ) reader->AddVariable( "sqrt(Xcore*Xcore+Ycore*Ycore)", &fCoreDist );
    else if( varName == "DispDiff" ) reader->AddVariable( "DispDiff", &fDispDiff );
    else if( varName == "log10(DispDiff)" ) reader->AddVariable( "log10(DispDiff)", &fDispDiff_log10 );
    else if( varName == "DispAbsSumWeigth" ) reader->AddVariable( "DispAbsSumWeigth", &fDispAbsSumWeigth );
    // Ranked per-telescope variables
    else if( varName == "Width_r1" ) reader->AddVariable( "Width_r1", &fWidth_r1 );
    else if( varName == "Width_r2" ) reader->AddVariable( "Width_r2", &fWidth_r2 );
    else if( varName == "Width_r3" ) reader->AddVariable( "Width_r3", &fWidth_r3 );
    else if( varName == "Width_r4" ) reader->AddVariable( "Width_r4", &fWidth_r4 );
    else if( varName == "Length_r1" ) reader->AddVariable( "Length_r1", &fLength_r1 );
    else if( varName == "Length_r2" ) reader->AddVariable( "Length_r2", &fLength_r2 );
    else if( varName == "Length_r3" ) reader->AddVariable( "Length_r3", &fLength_r3 );
    else if( varName == "Length_r4" ) reader->AddVariable( "Length_r4", &fLength_r4 );
    else if( varName == "Loss_r1" ) reader->AddVariable( "Loss_r1", &fLoss_r1 );
    else if( varName == "Loss_r2" ) reader->AddVariable( "Loss_r2", &fLoss_r2 );
    else if( varName == "Loss_r3" ) reader->AddVariable( "Loss_r3", &fLoss_r3 );
    else if( varName == "Loss_r4" ) reader->AddVariable( "Loss_r4", &fLoss_r4 );
    else if( varName == "Rcore_r1" ) reader->AddVariable( "Rcore_r1", &fRcore_r1 );
    else if( varName == "Rcore_r2" ) reader->AddVariable( "Rcore_r2", &fRcore_r2 );
    else if( varName == "Rcore_r3" ) reader->AddVariable( "Rcore_r3", &fRcore_r3 );
    else if( varName == "Rcore_r4" ) reader->AddVariable( "Rcore_r4", &fRcore_r4 );
    else
    {
        // default: treat unknowns as spectators to preserve behavior
        reader->AddSpectator( varName.c_str(), &fDummy );
    }
}

// Helper: build readers, bind variables, book MVA, and optionally optimize sensitivity
bool VTMVAEvaluator::setupTMVAReaders( unsigned int iWeightFileIndex_Emin, unsigned int iWeightFileIndex_Emax,
                                       unsigned int iWeightFileIndex_Zmin, unsigned int iWeightFileIndex_Zmax )
{
    for( unsigned int b = 0; b < fTMVAData.size(); b++ )
    {
        fTMVAData[b]->fTMVAReader = new TMVA::Reader();
        if( fDebug )
        {
            cout << "INITIALIZE TMVA file: " << fTMVAData[b]->fTMVAFileName << endl;
        }

        if( fTMVACutValueNoVec < -1. && fSignalEfficiencyNoVec > 0. )
        {
            getValuesFromEfficiencyHistograms( b );
        }
        else if( fTMVACutValueNoVec > -1. )
        {
            fTMVAData[b]->fSignalEfficiency = -99.;
            getValuesFromEfficiencyHistograms( b );
        }
        fTMVAData[b]->fTMVAOptimumCutValueFound = false;

        if( fDebug )
        {
            cout << "reading TMVA XML weight file: " << fTMVAData[b]->fTMVAFileNameXML << endl;
        }

        vector< bool > iVariableIsASpectator;
        vector< string > iTrainingVariables = getTrainingVariables( fTMVAData[b]->fTMVAFileNameXML, iVariableIsASpectator );

        for( unsigned int t = 0; t < iTrainingVariables.size(); t++ )
        {
            bindVariableByName( fTMVAData[b]->fTMVAReader, iTrainingVariables[t], iVariableIsASpectator[t] );
        }
        if( fDebug )
        {
            cout << "Following " << iTrainingVariables.size() << " variables have been found and are used for TMVA separation: " << endl;
            for( unsigned int t = 0; t < iTrainingVariables.size(); t++ )
            {
                cout << "\t" << iTrainingVariables[t];
                if( iVariableIsASpectator[t] )
                {
                    cout << " (spectator)";
                }
                cout << endl;
            }
        }
        if(!fTMVAData[b]->fTMVAReader->BookMVA(
                    fTMVAData[b]->fTMVAMethodTag_2.c_str(),
                    fTMVAData[b]->fTMVAFileNameXML.c_str() ) )
        {
            cout << "VTMVAEvaluator::initializeWeightFiles: error while initializing TMVA reader from weight file ";
            cout << fTMVAData[b]->fTMVAFileNameXML << endl;
            fIsZombie = true;
            return false;
        }

        if( fParticleNumberFileName.size() > 0 )
        {
            cout << endl;
            cout << "======================= optimize sensitivity =======================" << endl;
            if(!optimizeSensitivity( b ) )
            {
                cout << "VTMVAEvaluator::initializeWeightFiles: error while calculating optimized sensitivity" << endl;
                return false;
            }
            cout << "======================= end optimize sensitivity =======================" << endl;
            cout << endl;
        }
    }
    return true;
}

/*
 *  copy TMVA data vectors
 *
 * VTMVAEvaluatorResults are written to output files and used in
 * anasum, effective areas code and sensitivity calculation
 */
void VTMVAEvaluator::fillTMVAEvaluatorResults()
{
    if(!fTMVAEvaluatorResults )
    {
        fTMVAEvaluatorResults = new VTMVAEvaluatorResults;
        fTMVAEvaluatorResults->SetName( "TMVAEvaluatorResults" );
    }
    if( fTMVAEvaluatorResults )
    {
        fTMVAEvaluatorResults->fTMVAData = fTMVAData;
    }
}

TH1F* VTMVAEvaluator::getEfficiencyHistogram( string iName, TFile* iF, string iMethodTag_2 )
{
    if(!iF )
    {
        return 0;
    }

    char hname[800];
    sprintf( hname, "Method_%s/%s_%s/MVA_%s_%s_%s", fTMVAMethodName.c_str(),
             fTMVAMethodName.c_str(), iMethodTag_2.c_str(),
             fTMVAMethodName.c_str(), iMethodTag_2.c_str(), iName.c_str() );

    // read signal efficiency histogram
    TH1F* eff = ( TH1F* )iF->Get( hname );
    if(!eff )
    {
        sprintf( hname, "Method_%s/%s_0/MVA_%s_0_%s", fTMVAMethodName.c_str(),
                 fTMVAMethodName.c_str(),
                 fTMVAMethodName.c_str(), iName.c_str() );
        eff = ( TH1F* )iF->Get( hname );
        if(!eff )
        {
            cout << "VTMVAEvaluator::getEfficiencyHistogram() error finding efficiency histogram " << hname;
            cout << " from " << iF->GetName() << endl;
            return 0;
        }
    }
    return eff;
}

/*

   get TMVA cut values
   (e.g. signal efficiency for a given MVA cut or
         MVA cut for a given signal efficiency

*/

bool VTMVAEvaluator::getValuesFromEfficiencyHistograms( unsigned int b )
{
    if( b >= fTMVAData.size() )
    {
        return false;
    }
    // make sure that default values are set
    if( fTMVAData[b]->fTMVACutValue > -1. )
    {
        fTMVAData[b]->fSignalEfficiency = fTMVAData[b]->fBackgroundEfficiency = -99.;
    }
    else if( fTMVAData[b]->fSignalEfficiency > 0. )
    {
        fTMVAData[b]->fTMVACutValue = fTMVAData[b]->fBackgroundEfficiency = -99.;
    }
    else if( fTMVAData[b]->fBackgroundEfficiency > 0. )
    {
        fTMVAData[b]->fTMVACutValue = fTMVAData[b]->fSignalEfficiency = -99.;
    }

    // check file name for consistency
    if( fTMVAData[b]->fTMVAFileName.size() == 0 )
    {
        return false;
    }

    TFile iTMVAFile( fTMVAData[b]->fTMVAFileName.c_str() );
    if( iTMVAFile.IsZombie() )
    {
        cout << "VTMVAEvaluator::getValuesFromEfficiencyHistograms() error reading TMVA root file: " << fTMVAData[b]->fTMVAFileName << endl;
        return false;
    }
    TH1F* effS = getEfficiencyHistogram( "effS", &iTMVAFile, fTMVAData[b]->fTMVAMethodTag_2 );
    TH1F* effB = getEfficiencyHistogram( "effB", &iTMVAFile, fTMVAData[b]->fTMVAMethodTag_2 );
    if(!effS || !effB )
    {
        return false;
    }

    if( fDebug )
    {
        cout << "VTMVAEvaluator::getValuesFromEfficiencyHistograms: evaluating " << iTMVAFile.GetName() << endl;
    }
    // get MVA cut for a given signal efficiency
    if( fTMVAData[b]->fSignalEfficiency > 0. )
    {
        fTMVAData[b]->fTMVACutValue = effS->GetBinCenter( effS->FindLastBinAbove( fTMVAData[b]->fSignalEfficiency ) );
        fTMVAData[b]->fBackgroundEfficiency = effB->GetBinContent( effB->GetXaxis()->FindBin( fTMVAData[b]->fTMVACutValue ) );

        cout << "TMVA CUT VALUE FOR SIGNAL EFFICIENCY " << fTMVAData[b]->fSignalEfficiency << ": " << fTMVAData[b]->fTMVACutValue;
        cout << " (bin " << effS->FindLastBinAbove( fTMVAData[b]->fSignalEfficiency ) << ")" << endl;
    }
    // get signal efficiency from histogram
    else if( fTMVAData[b]->fTMVACutValue > -1. )
    {
        fTMVAData[b]->fSignalEfficiency     = effS->GetBinContent( effS->GetXaxis()->FindBin( fTMVAData[b]->fTMVACutValue ) );
        fTMVAData[b]->fBackgroundEfficiency = effB->GetBinContent( effB->GetXaxis()->FindBin( fTMVAData[b]->fTMVACutValue ) );

        if( fDebug )
        {
            cout << "Signal efficiency for TMVA cut value " << fTMVAData[b]->fTMVACutValue << ": " << fTMVAData[b]->fSignalEfficiency;
            cout << " (bin " << effS->GetXaxis()->FindBin( fTMVAData[b]->fTMVACutValue ) << ")" << endl;
        }
    }
    // get MVA cut for a given background efficiency
    else if( fTMVAData[b]->fBackgroundEfficiency > 0. )
    {
        fTMVAData[b]->fTMVACutValue = effB->GetBinCenter( effB->FindLastBinAbove( fTMVAData[b]->fBackgroundEfficiency ) );
        fTMVAData[b]->fSignalEfficiency = effS->GetBinContent( effS->GetXaxis()->FindBin( fTMVAData[b]->fTMVACutValue ) );

        cout << "TMVA CUT VALUE FOR SIGNAL EFFICIENCY " << fTMVAData[b]->fBackgroundEfficiency << ": " << fTMVAData[b]->fTMVACutValue;
        cout << " (bin " << effB->FindLastBinAbove( fTMVAData[b]->fBackgroundEfficiency ) << ")" << endl;
    }

    iTMVAFile.Close();

    return true;
}

/*!

    evaluate this event using the MVA and return passed/not passed

*/
bool VTMVAEvaluator::evaluate( bool interpolate_mva, bool use_average_zenith_angle )
{
    if( fDebug )
    {
        cout << "VTMVAEvaluator::evaluate (" << fData << ")" << endl;
    }
    // copy event data
    if( fData )
    {
        fNImages        = ( float )fData->NImages;
        fMSCW           = fData->MSCW;
        fMSCL           = fData->MSCL;
        fMWR            = fData->MWR;
        fMLR            = fData->MLR;
        fEmissionHeight = fData->EmissionHeight;
        if( fData->EmissionHeightChi2 > 0. )
        {
            fEmissionHeightChi2_log10 = TMath::Log10( fData->EmissionHeightChi2 );
        }
        else
        {
            fEmissionHeightChi2_log10 = -10.;    // !!! not clear what the best value is
        }
        fEChi2S         = fData->EChi2S;
        if( fEChi2S > 0. )
        {
            fEChi2S_log10 = TMath::Log10( fEChi2S );
        }
        else
        {
            fEChi2S_log10 = 0.;    // !!! not clear what the best value is
        }
        fdES            = fData->dES;
        fSizeSecondMax_log10 = fData->SizeSecondMax;
        if( fSizeSecondMax_log10 > 0. )
        {
            fSizeSecondMax_log10 = TMath::Log10( fSizeSecondMax_log10 );
        }
        else
        {
            fSizeSecondMax_log10 = 0.;    // !!! not clear what the best value is
        }
        fCoreDist = sqrt( fData->Xcore* fData->Xcore + fData->Ycore* fData->Ycore );
        fDispDiff = fData->DispDiff;
        if( fDispDiff > 0. )
        {
            fDispDiff_log10 = log10( fDispDiff );
        }
        else
        {
            fDispDiff_log10 = 0.;    // !!! not clear what the best value is
        }
        fDispAbsSumWeigth = fData->DispAbsSumWeigth;
        // Compute ranked per-telescope variables using ImgSel mask
        // Always compute to keep logic simple; unused values are harmless
        fWidth_r1 = fWidth_r2 = fWidth_r3 = fWidth_r4 = 0.0f;
        fLength_r1 = fLength_r2 = fLength_r3 = fLength_r4 = 0.0f;
        fLoss_r1 = fLoss_r2 = fLoss_r3 = fLoss_r4 = 0.0f;
        fRcore_r1 = fRcore_r2 = fRcore_r3 = fRcore_r4 = 0.0f;
        // Build selected index list from lower 4 bits of ImgSel
        int selIdx[4];
        int selCount = 0;
        for( int ii = 0; ii < 4; ii++ )
        {
            if( ( fData->ImgSel & ( 1ULL << ii ) ) != 0 )
            {
                selIdx[selCount++] = ii;
            }
        }
        if( selCount > 0 )
        {
            // selection sort selected indices by size desc
            for( int i = 0; i < selCount; i++ )
            {
                int maxj = i;
                for( int j = i + 1; j < selCount; j++ )
                {
                    if( fData->size[selIdx[j]] > fData->size[selIdx[maxj]] )
                    {
                        maxj = j;
                    }
                }
                if( maxj != i )
                {
                    int t = selIdx[i];
                    selIdx[i] = selIdx[maxj];
                    selIdx[maxj] = t;
                }
            }
            float Wr[4] = { 0 }, Lr[4] = { 0 }, Lo[4] = { 0 }, Rc[4] = { 0 };
            int k = 0;
            for( ; k < selCount && k < 4; k++ )
            {
                int s = selIdx[k];
                Wr[k] = fData->width[s];
                Lr[k] = fData->length[s];
                Lo[k] = fData->loss[s];
                Rc[k] = fData->R_core[s];
            }
            int lastSrc = selIdx[selCount - 1];
            for( ; k < 4; k++ )
            {
                Wr[k] = fData->width[lastSrc];
                Lr[k] = fData->length[lastSrc];
                Lo[k] = fData->loss[lastSrc];
                Rc[k] = fData->R_core[lastSrc];
            }
            fWidth_r1 = Wr[0]; fWidth_r2 = Wr[1]; fWidth_r3 = Wr[2]; fWidth_r4 = Wr[3];
            fLength_r1 = Lr[0]; fLength_r2 = Lr[1]; fLength_r3 = Lr[2]; fLength_r4 = Lr[3];
            fLoss_r1 = Lo[0]; fLoss_r2 = Lo[1]; fLoss_r3 = Lo[2]; fLoss_r4 = Lo[3];
            fRcore_r1 = Rc[0]; fRcore_r2 = Rc[1]; fRcore_r3 = Rc[2]; fRcore_r4 = Rc[3];
        }
    }
    else
    {
        return false;
    }

    // find correct bin (e.g. depending on energy or zenith)
    double i_ze = fData->Ze;
    if( use_average_zenith_angle && fAverageZenithPerRun >= 0. )
    {
        i_ze = fAverageZenithPerRun;
    }
    if( fData->ErecS <= 0. )
    {
        return false;
    }
    unsigned int iDataBin = getDataBin( log10( fData->ErecS ), i_ze );
    if( fDebug )
    {
        cout << "VTMVAEvaluator::evaluate: data bin " << iDataBin;
        cout << ", MVA Method Tag " << fTMVAData[iDataBin]->fTMVAMethodTag;
        cout << ", MVA Cut value " << fTMVAData[iDataBin]->fTMVACutValue;
        cout << endl;
    }
    // invalid data bin (e.g., event zenith is outside range of TMVA evaluator)
    if( iDataBin > 9998 )
    {
        return false;
    }

    fTMVA_EvaluationResult = -99.;
    if( interpolate_mva )
    {
        fTMVA_EvaluationResult = interpolate_mva_evaluation();
    }
    else
    {
        fTMVA_EvaluationResult = fTMVAData[iDataBin]->fTMVAReader->EvaluateMVA( fTMVAData[iDataBin]->fTMVAMethodTag_2 );
    }

    // apply MVA cut (cut value not interpolated in zenith)
    if( fTMVA_EvaluationResult < fTMVAData[iDataBin]->fTMVACutValue )
    {
        return false;
    }
    return true;
}


/*
 * Interpolate between MVA responses in zenith angle
 * (training in energy axis is on overlapping energy bins)
 *
*/
double VTMVAEvaluator::interpolate_mva_evaluation()
{
    if(!fData || fData->ErecS <= 0 )
    {
        return 9999.;
    }
    set<unsigned int> data_bins;
    for( unsigned int i = 0; i < fTMVAData.size(); i++ )
    {
        data_bins.insert( getDataBin( log10( fData->ErecS ), 0.5 * ( fTMVAData[i]->fZenithCut_max + fTMVAData[i]->fZenithCut_min ) ) );
    }
    TGraph iG(( int )data_bins.size() );
    unsigned int i = 0;
    for( set<unsigned int>::iterator it = data_bins.begin(); it != data_bins.end(); ++it )
    {
        iG.SetPoint(
            i,
            0.5 * ( fTMVAData[*it]->fZenithCut_max + fTMVAData[*it]->fZenithCut_min ),
            fTMVAData[*it]->fTMVAReader->EvaluateMVA( fTMVAData[*it]->fTMVAMethodTag_2 )
        );
        i++;
    }
    return iG.Eval( fData->Ze, 0 ); // linear interpolation
}


/*
 *
 *   get bin number for current event
 */
unsigned int VTMVAEvaluator::getDataBin()
{
    if(!fData || fData->ErecS <= 0 )
    {
        return 9999;
    }
    return getDataBin( log10( fData->ErecS ), fData->Ze );
}


unsigned int VTMVAEvaluator::getDataBin( double iErec, double iZe )
{
    double       i_Diff_Energy = 1.e10;           // difference between energy of current event and mean bin energy
    double       iMeanEnergy = 0.;
    double       iMeanEnergy_min = 1.e10;

    unsigned int iBin = 9999;
    for( unsigned int i = 0; i < fTMVAData.size(); i++ )
    {
        //   get zenith bin for the current zenith (read from fData)
        if(( iZe > fTMVAData[i]->fZenithCut_min && iZe <= fTMVAData[i]->fZenithCut_max ) || iZe < -998. )
        {
            // mean energy of this energy bin (possibly spectral weighted)
            iMeanEnergy = VMathsandFunctions::getMeanEnergyInBin(
                              2,
                              fTMVAData[i]->fEnergyCut_Log10TeV_min,
                              fTMVAData[i]->fEnergyCut_Log10TeV_max,
                              fSpectralIndexForEnergyWeighting );
            // check which energy bin is closest
            if( TMath::Abs( iMeanEnergy - iErec ) < i_Diff_Energy )
            {
                i_Diff_Energy = TMath::Abs( iMeanEnergy - iErec );
                iBin = i;
                iMeanEnergy_min = iMeanEnergy;
            }
        }
    }
    if( fDebug && iBin < fTMVAData.size() )
    {
        cout << "VTMVAEvaluator::getDataBin: " << iBin << endl;
        fTMVAData[iBin]->print();
        cout << "\t closest mean energy " << iMeanEnergy_min;
        cout << ", log10 energy_tested " << iErec << ", distance: " << i_Diff_Energy ;
        cout << "\t" << fSpectralIndexForEnergyWeighting << endl;
    }

    return iBin;
}

bool VTMVAEvaluator::initializeDataStrutures( CData* iC )
{
    fData = iC;

    if(!fData )
    {
        fIsZombie = true;
        return false;
    }
    calculate_average_zenith_angle();

    return true;
}

/*
 * plot signal and background efficiencies
 *
 */
TGraphAsymmErrors* VTMVAEvaluator::plotSignalAndBackgroundEfficiencies(
    bool iLogY, double iYmin, double iMVA_min, double iMVA_max )
{
    if( fTMVAData.size() == 0 )
    {
        cout << "TMVAEvaluator::plotSignalAndBackgroundEfficiencies error: signal efficiency vector with size 0" << endl;
        return 0;
    }

    // fill graphs
    TGraphAsymmErrors* igSignal = new TGraphAsymmErrors( 1 );
    TGraphAsymmErrors* igSignalOpt = new TGraphAsymmErrors( 1 );
    TGraphAsymmErrors* igBck = new TGraphAsymmErrors( 1 );
    TGraphAsymmErrors* igBckOpt = new TGraphAsymmErrors( 1 );
    TGraphAsymmErrors* igCVa = new TGraphAsymmErrors( 1 );
    TGraphAsymmErrors* igCVaOpt = new TGraphAsymmErrors( 1 );

    unsigned int z_opt = 0;
    unsigned int z_noOpt = 0;

    double iMinBck = 1.;

    for( unsigned int i = 0; i < fTMVAData.size(); i++ )
    {
        if(!fTMVAData[i] )
        {
            continue;
        }

        if( fTMVAData[i]->fSignalEfficiency < 0. || fTMVAData[i]->fBackgroundEfficiency < 0. )
        {
            cout << "VTMVAEvaluator::plotSignalAndBackgroundEfficiencies: ";
            cout << "signal / background efficiency histograms not found " << endl;
            cout << fTMVAData[i]->fSignalEfficiency << "\t" << fTMVAData[i]->fBackgroundEfficiency << endl;
            continue;
        }
        if( fTMVAData[i]->fTMVAOptimumCutValueFound )
        {
            igSignal->SetPoint( z_opt, fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV, fTMVAData[i]->fSignalEfficiency );
            igSignal->SetPointEXlow( z_opt, fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV - fTMVAData[i]->fEnergyCut_Log10TeV_min );
            igSignal->SetPointEXhigh( z_opt, fTMVAData[i]->fEnergyCut_Log10TeV_max - fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV );

            igBck->SetPoint( z_opt, fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV, fTMVAData[i]->fBackgroundEfficiency );
            igBck->SetPointEXlow( z_opt, fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV - fTMVAData[i]->fEnergyCut_Log10TeV_min );
            igBck->SetPointEXhigh( z_opt, fTMVAData[i]->fEnergyCut_Log10TeV_max - fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV );

            igCVa->SetPoint( z_opt, fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV, fTMVAData[i]->fTMVACutValue );
            igCVa->SetPointEXlow( z_opt, fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV - fTMVAData[i]->fEnergyCut_Log10TeV_min );
            igCVa->SetPointEXhigh( z_opt, fTMVAData[i]->fEnergyCut_Log10TeV_max - fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV );

            z_opt++;
        }
        else if( fTMVAData[i]->fTMVACutValue > -90. )
        {
            igSignalOpt->SetPoint( z_noOpt, fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV, fTMVAData[i]->fSignalEfficiency );
            igSignalOpt->SetPointEXlow( z_noOpt, fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV - fTMVAData[i]->fEnergyCut_Log10TeV_min );
            igSignalOpt->SetPointEXhigh( z_noOpt, fTMVAData[i]->fEnergyCut_Log10TeV_max - fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV );

            igBckOpt->SetPoint( z_noOpt, fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV, fTMVAData[i]->fBackgroundEfficiency );
            igBckOpt->SetPointEXlow( z_noOpt, fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV - fTMVAData[i]->fEnergyCut_Log10TeV_min );
            igBckOpt->SetPointEXhigh( z_noOpt, fTMVAData[i]->fEnergyCut_Log10TeV_max - fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV );

            igCVaOpt->SetPoint( z_noOpt, fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV, fTMVAData[i]->fTMVACutValue );
            igCVaOpt->SetPointEXlow( z_noOpt, fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV - fTMVAData[i]->fEnergyCut_Log10TeV_min );
            igCVaOpt->SetPointEXhigh( z_noOpt, fTMVAData[i]->fEnergyCut_Log10TeV_max - fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV );

            z_noOpt++;
        }
        if( fTMVAData[i]->fBackgroundEfficiency < iMinBck )
        {
            iMinBck = fTMVAData[i]->fBackgroundEfficiency;
        }
    }

    // plot everything
    TCanvas* iCanvas = new TCanvas( "cSignalAndBackgroundEfficiencies", "signal and background efficiencies",
                                    10, 10, 400, 400 );
    iCanvas->SetGridx( 0 );
    iCanvas->SetGridy( 0 );
    iCanvas->SetLeftMargin( 0.13 );
    if( iLogY )
    {
        iCanvas->SetLogy();
    }
    else
    {
        iCanvas->SetLogy( 0 );
    }
    iCanvas->Draw();

    double iE_min =  1.e99;
    double iE_max = -1.e99;
    for( unsigned int i = 0; i < fTMVAData.size(); i++ )
    {
        if( fTMVAData[i] && fTMVAData[i]->fEnergyCut_Log10TeV_min < iE_min )
        {
            iE_min = fTMVAData[i]->fEnergyCut_Log10TeV_min;
        }
        if( fTMVAData[i] && fTMVAData[i]->fEnergyCut_Log10TeV_max > iE_max )
        {
            iE_max = fTMVAData[i]->fEnergyCut_Log10TeV_max;
        }
    }

    TH1D* hnull = new TH1D( "hnullcSignalAndBackgroundEfficiencies", "", 100, iE_min, iE_max );
    hnull->SetStats( 0 );
    hnull->SetXTitle( "energy [TeV]" );
    hnull->SetYTitle( "signal/background efficiency" );
    hnull->SetMinimum( iYmin );
    hnull->SetMaximum( 1. );
    plot_nullHistogram( iCanvas, hnull, false, false, 1.5, iE_min, iE_max );

    setGraphPlottingStyle( igSignal, 1, 1., 20 );
    setGraphPlottingStyle( igSignalOpt, 1, 1., 24 );
    if( igBck )
    {
        setGraphPlottingStyle( igBck, 2, 1., 21 );
    }
    if( igBckOpt )
    {
        setGraphPlottingStyle( igBckOpt, 2, 1., 25 );
    }

    igSignal->Draw( "pl" );
    if( z_noOpt )
    {
        igSignalOpt->Draw( "pl" );
    }
    if( igBck )
    {
        igBck->Draw( "pl" );
    }
    if( igBckOpt && z_noOpt > 0 )
    {
        igBckOpt->Draw( "pl" );
    }
    if( fPrintPlotting )
    {
        iCanvas->Print( "MVA-SignalBackgroundEfficiency.pdf" );
    }

    // plot MVA cut value
    if( igCVa )
    {
        TCanvas* iCVACanvas = new TCanvas( "iCVACanvas", "MVA cut value", 500, 10, 400, 400 );
        iCVACanvas->SetGridx( 0 );
        iCVACanvas->SetGridy( 0 );

        TH1D* hnull = new TH1D( "hnullcMVACuts", "", 100, iE_min, iE_max );
        hnull->SetStats( 0 );
        hnull->SetXTitle( "energy [TeV]" );
        hnull->SetYTitle( "MVA cut variable" );
        hnull->SetMinimum( iMVA_min );
        hnull->SetMaximum( iMVA_max );
        plot_nullHistogram( iCanvas, hnull, false, false, 1.3, iE_min, iE_max );
        setGraphPlottingStyle( igCVa, 1, 1., 20 );
        igCVa->Draw( "p" );
        if( igCVaOpt && z_noOpt > 0 )
        {
            setGraphPlottingStyle( igCVaOpt, 1, 1., 24 );
            igCVaOpt->Draw( "p" );
        }
        if( fPrintPlotting )
        {
            iCVACanvas->Print( "MVA-MVACut.pdf" );
        }
    }

    return igCVa;
}

void VTMVAEvaluator::setSignalEfficiency( double iSignalEfficiency )
{
    fSignalEfficiencyMap[9999] = iSignalEfficiency;

    fSignalEfficiencyNoVec = iSignalEfficiency;
}

void VTMVAEvaluator::setSignalEfficiency( map< unsigned int, double > iSignalEfficiencyMap )
{
    fSignalEfficiencyMap = iSignalEfficiencyMap;

    // set to value >0: indicates that signal efficiency had been set from outsite
    fSignalEfficiencyNoVec = 1.;
}

void VTMVAEvaluator::setTMVACutValue( map< unsigned int, double > iMVA )
{
    fTMVACutValueMap = iMVA;

    fTMVACutValueNoVec = 1.;
}

void VTMVAEvaluator::setTMVACutValue( double iE )
{
    fTMVACutValueNoVec = iE;
}

void VTMVAEvaluator::printSignalEfficiency()
{
    if( fTMVAData.size() == 0 )
    {
        return;
    }

    cout << endl;
    cout << "======================= VTMVAEvaluator: signal (background) efficiency =======================" << endl;
    for( unsigned int i = 0; i < fTMVAData.size(); i++ )
    {
        if( fTMVAData[i] )
        {
            cout << "E [" << showpoint << setprecision( 3 );
            cout << fTMVAData[i]->fEnergyCut_Log10TeV_min << "," << fTMVAData[i]->fEnergyCut_Log10TeV_max << "] TeV";
            cout << ", Ze [" << fTMVAData[i]->fZenithCut_min << "," << fTMVAData[i]->fZenithCut_max << "] deg";
            cout << " (bin " << i << "):\t ";
            cout << fTMVAData[i]->fSignalEfficiency;
            if( fTMVAData[i]->fBackgroundEfficiency > 0. )
            {
                cout << "\t(" << fTMVAData[i]->fBackgroundEfficiency << ")";
            }
            cout << "\t MVACut: " << fTMVAData[i]->fTMVACutValue;
            if( fParticleNumberFileName.size() > 0 )
            {
                if( fTMVAData[i]->fTMVAOptimumCutValueFound )
                {
                    cout << " (optimum reached for " << fTMVAData[i]->fSourceStrengthAtOptimum_CU << " CU)";
                }
                else
                {
                    cout << " (no optimum reached (" << fTMVAData[i]->fSourceStrengthAtOptimum_CU << " CU)";
                }
            }
        }
        cout << endl;
    }
    cout << noshowpoint << endl;
}

/*
 * print MVA cut values after optimization to be used
 * in gamma/hadron cuts files
 */
void VTMVAEvaluator::printOptimizedMVACutValues( string iEpoch )
{
    cout << "Printing Optimised cuts for gamma/hadron cut values" << endl;
    cout << "\t this is only correct for an energyStepSize of -1" << endl;
    cout << "* TMVA_MVACut " << iEpoch;
    for( unsigned int i = 0; i < fTMVAData.size(); i++ )
    {
        cout << " " << fTMVAData[i]->fTMVAMethodTag;
        cout << " " << fTMVAData[i]->fTMVACutValue;
    }
    cout << endl;
    cout << "(first digit: energy bin, second digit: zenith bin)" << endl;
}

/*

    calculate the optimal signal to noise ratio for a given particle number spectrum

    this routine is possibly too complicated

    - main problem is how to deal with low statistics bins

*/

bool VTMVAEvaluator::optimizeSensitivity( unsigned int iDataBin )
{
    // valid data bin
    if( iDataBin >= fTMVAData.size() || !fTMVAData[iDataBin] )
    {
        return false;
    }

    // print some info on optimization parameters to screen
    printSensitivityOptimizationParameters();

    //////////////////////////////////////////////////////
    // read file with  NOn and Noff graphs
    // (contains signal and background rate vs energy)
    // (created from effective areas with quality cuts applied only,
    TFile iPN( fParticleNumberFileName.c_str() );
    if( iPN.IsZombie() )
    {
        cout << "VTVMAEvaluator::optimizeSensitivity error:" << endl;
        cout << " cannot read particle number file " << fParticleNumberFileName << endl;
        if( fParticleNumberFileName.size() == 0 )
        {
            cout << "VTMVAEvaluator::optimizeSensitivity error: no particle number file given" << endl;
        }
        return false;
    }
    cout << "TVMAEvaluator::optimizeSensitivity reading: " << fParticleNumberFileName << endl;
    // get the NOn (signal + background) and NOff (background) graphs
    TGraph* i_on = readNonNoffGraphsFromFile(
                       &iPN,
                       fTMVAData[iDataBin]->fZenithCut_min, fTMVAData[iDataBin]->fZenithCut_max,
                       true );
    TGraph* i_of = readNonNoffGraphsFromFile(
                       &iPN,
                       fTMVAData[iDataBin]->fZenithCut_min, fTMVAData[iDataBin]->fZenithCut_max,
                       false );
    if(!i_on || !i_of )
    {
        cout << "VTVMAEvaluator::optimizeSensitivity error:" << endl;
        cout << " cannot read graphs from particle number file " << endl;
        cout << i_on << "\t" << i_of << endl;
        return false;
    }

    //////////////////////////////////////////////////////
    // get mean energy of the considered bins
    // interval [fTMVAData[iDataBin]->fEnergyCut_Log10TeV_min,fTMVAData[iDataBin]->fEnergyCut_Log10TeV_max]
    // make sure that energy is not lower or higher then minimum/maximum bins in the rate graphs

    double x = 0.;
    double p = 0.;

    for( int ii = 0; ii < i_on->GetN(); ii++ )
    {
        i_on->GetPoint( ii, x, p );

        if( p > 0. && ( x + i_on->GetErrorX( ii ) <= fTMVAData[iDataBin]->fEnergyCut_Log10TeV_max
                        || x <= fTMVAData[iDataBin]->fEnergyCut_Log10TeV_max ) )
        {
            if( fTMVAData[iDataBin]->fSpectralWeightedMeanEnergy_Log10TeV < x )
            {
                if( x + i_on->GetErrorX( ii ) <= fTMVAData[iDataBin]->fEnergyCut_Log10TeV_max )
                {
                    fTMVAData[iDataBin]->fSpectralWeightedMeanEnergy_Log10TeV = x + 0.97 * i_on->GetErrorX( ii );
                }
                else
                {
                    fTMVAData[iDataBin]->fSpectralWeightedMeanEnergy_Log10TeV = x;
                }
            }
            break;
        }
    }
    ///////////
    // make sure that selected energy is not beyond the valid range of the graph

    // get the value of the energy, zenith and particle rate for the last index of the array
    i_on->GetPoint( i_on->GetN() - 1, x, p );

    // energy is beyond - set it to 0.8*last value
    if( fTMVAData[iDataBin]->fSpectralWeightedMeanEnergy_Log10TeV > x )
    {
        fTMVAData[iDataBin]->fSpectralWeightedMeanEnergy_Log10TeV = TMath::Log10( TMath::Power( 10., x ) * 0.8 );
    }
    ///////////////////////////////////////////////////////////////////////////////
    // get number of events (after quality cuts) at this energy from on/off graphs
    double Non = 0.;
    double Nof = 0.;
    double Ndif = 0.;

    ///////////////////////////////////////////////////////////////////////////////
    // Interpolate between the values of the TGraph2D
    //
    // Convert the observing time in seconds as the particle rate is given in 1/seconds
    // Get the value of the middle of the energy and zenith angle bin
    Non = i_on->Eval( fTMVAData[iDataBin]->fSpectralWeightedMeanEnergy_Log10TeV )
          * fOptimizationObservingTime_h * fParticleNumberFile_Conversion_Rate_to_seconds;
    Nof = i_of->Eval( fTMVAData[iDataBin]->fSpectralWeightedMeanEnergy_Log10TeV )
          * fOptimizationObservingTime_h * fParticleNumberFile_Conversion_Rate_to_seconds;

    if( Nof < 0. )
    {
        Nof = 0.;
    }
    Ndif = Non - Nof;

    cout << "VTVMAEvaluator::optimizeSensitivity event numbers: ";
    cout << " non = " << Non;
    cout << " noff = " << Nof;
    cout << " ndiff = " << Ndif << " (1 CU)" << endl;
    cout << "VTVMAEvaluator::optimizeSensitivity event numbers: ";
    cout << " (data bin " << iDataBin;
    cout << ",  weighted mean energy ";
    cout << TMath::Power( 10., fTMVAData[iDataBin]->fSpectralWeightedMeanEnergy_Log10TeV ) << " [TeV], ";
    cout << fTMVAData[iDataBin]->fSpectralWeightedMeanEnergy_Log10TeV << "), ";
    cout << " Ebin [" << fTMVAData[iDataBin]->fEnergyCut_Log10TeV_min;
    cout << ", " << fTMVAData[iDataBin]->fEnergyCut_Log10TeV_max << "]";
    cout << endl;

    ///////////////////////////////////////////////////////////////////
    // get signal and background efficiency histograms from TMVA files

    TFile iTMVAFile( fTMVAData[iDataBin]->fTMVAFileName.c_str() );
    if( iTMVAFile.IsZombie() )
    {
        cout << "VTVMAEvaluator::optimizeSensitivity error:" << endl;
        cout << " cannot read TMVA file " << fTMVAData[iDataBin]->fTMVAFileName;
        cout << " (bin " << iDataBin << ")" << endl;
        return false;
    }
    // get signal and background efficiency histograms
    TH1F* effS = getEfficiencyHistogram( "effS", &iTMVAFile, fTMVAData[iDataBin]->fTMVAMethodTag_2 );
    TH1F* effB = getEfficiencyHistogram( "effB", &iTMVAFile, fTMVAData[iDataBin]->fTMVAMethodTag_2 );
    if(!effS || !effB )
    {
        cout << "VTVMAEvaluator::optimizeSensitivity error:" << endl;
        cout << " cannot find signal and/or background efficiency histogram(s)" << endl;
        cout << effS << "\t" << effB << endl;
        return false;
    }

    //////////////////////////////////////////////////////////////////////////
    // optimization starts here
    //////////////////////////////////////////////////////////////////////////
    double i_Signal_to_sqrtNoise = 0.;

    double i_TMVACutValue_AtMaximum = -99.;
    double i_SourceStrength_atMaximum = 0.;
    double i_SignalEfficiency_AtMaximum = -99.;
    double i_BackgroundEfficiency_AtMaximum = -99.;
    double i_Signal_to_sqrtNoise_atMaximum = 0.;

    TGraph* iGSignal_to_sqrtNoise = 0;
    TGraph* iGSignalEvents        = 0;
    TGraph* iGBackgroundEvents    = 0;
    TGraph* iGSignal_to_sqrtNoise_Smooth = 0;

    //////////////////////////////////////////////////////
    // loop over different source strengths (in Crab Units)
    // (hardwired: start at 0.001 CU to 30 CU)
    unsigned int iSourceStrengthStepSizeN =
        ( unsigned int )(( log10( 30. ) - log10( fOptimizationMinSourceStrength ) ) / 0.005 );
    cout << "VTVMAEvaluator::optimizeSensitivity(), source strength steps: " << iSourceStrengthStepSizeN << endl;
    for( unsigned int s = 0; s < iSourceStrengthStepSizeN; s++ )
    {
        double iSourceStrength = log10( fOptimizationMinSourceStrength ) + s * 0.005;
        iSourceStrength = TMath::Power( 10., iSourceStrength );

        // source (excess) events
        Ndif = ( Non - Nof ) * iSourceStrength;

        // first quick pass to see if there is a change of reaching the required fOptimizationSourceSignificance
        // (needed to speed up the calculation)
        // (ignore any detail, no optimization of angular cut)
        bool bPassed = false;
        for( int i = 1; i < effS->GetNbinsX(); i++ )
        {
            if( effB->GetBinContent( i ) > 0. && Nof > 0. )
            {
                if( fOptimizationBackgroundAlpha > 0. )
                {
                    i_Signal_to_sqrtNoise = VStatistics::calcSignificance(
                                                effS->GetBinContent( i ) * Ndif + effB->GetBinContent( i ) * Nof,
                                                effB->GetBinContent( i ) * Nof / fOptimizationBackgroundAlpha,
                                                fOptimizationBackgroundAlpha );
                    // check significance criteria
                    if( i_Signal_to_sqrtNoise > fOptimizationSourceSignificance )
                    {
                        bPassed = true;
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }
        // no chance to pass significance criteria -> continue to next energy bin
        if(!bPassed )
        {
            continue;
        }

        //////////////////////////////////////////////////////
        // now loop over signal and background efficiency levels
        i_Signal_to_sqrtNoise = 0.;
        i_TMVACutValue_AtMaximum = -99.;
        i_SourceStrength_atMaximum = 0.;
        i_SignalEfficiency_AtMaximum = -99.;
        i_BackgroundEfficiency_AtMaximum = -99.;
        i_Signal_to_sqrtNoise_atMaximum = 0.;

        iGSignal_to_sqrtNoise = new TGraph( 1 );
        iGSignalEvents        = new TGraph( 1 );
        iGBackgroundEvents    = new TGraph( 1 );

        int z = 0;
        int z_SB = 0;
        // loop over all signal efficiency bins
        for( int i = 1; i < effS->GetNbinsX(); i++ )
        {
            if( effB->GetBinContent( i ) > 0. && Nof > 0. )
            {
                if( fOptimizationBackgroundAlpha > 0. )
                {
                    // optimize signal/sqrt(noise)
                    i_Signal_to_sqrtNoise = VStatistics::calcSignificance(
                                                effS->GetBinContent( i ) * Ndif + effB->GetBinContent( i ) * Nof,
                                                effB->GetBinContent( i ) * Nof / fOptimizationBackgroundAlpha,
                                                fOptimizationBackgroundAlpha );
                }
                else
                {
                    i_Signal_to_sqrtNoise = 0.;
                }
                if( fDebug )
                {
                    cout << "___________________________________________________________" << endl;
                    cout << i << "\t" << Non << "\t" << effS->GetBinContent( i )  << "\t";
                    cout << Nof << "\t" << effB->GetBinContent( i ) << "\t";
                    cout << Ndif << endl;
                    cout << "\t" << effS->GetBinContent( i ) * Ndif;
                    cout << "\t" << effS->GetBinContent( i ) * Ndif + effB->GetBinContent( i ) * Nof;
                    cout << "\t" << effS->GetBinContent( i ) * Non + effB->GetBinContent( i ) * Nof;
                    cout << "\t" << effB->GetBinContent( i ) * Nof << endl;
                }
                if( effS->GetBinContent( i ) * Ndif > 0. )
                {
                    iGSignalEvents->SetPoint( z_SB, effS->GetBinCenter( i ),  effS->GetBinContent( i ) * Ndif );
                    iGBackgroundEvents->SetPoint( z_SB, effS->GetBinCenter( i ), effB->GetBinContent( i ) * Nof );
                    z_SB++;
                }
                // check that a minimum number of off events is available
                if( effB->GetBinContent( i ) * Nof < fOptimizationMinBackGroundEvents )
                {
                    if( fDebug )
                    {
                        cout << "\t number of background events lower than ";
                        cout << fOptimizationMinBackGroundEvents << ": setting signal/sqrt(noise) to 0; bin " << i << endl;
                    }
                    i_Signal_to_sqrtNoise = 0.;
                }
                // add results to a graph
                if( iGSignal_to_sqrtNoise && i_Signal_to_sqrtNoise > 1.e-2 )
                {
                    iGSignal_to_sqrtNoise->SetPoint( z, effS->GetBinCenter( i ), i_Signal_to_sqrtNoise );
                    if( fDebug )
                    {
                        cout << "\t SET " << z << "\t" << effS->GetBinCenter( i ) << "\t" << i_Signal_to_sqrtNoise << endl;
                    }
                    z++;
                }
                if( fDebug )
                {
                    cout << "\t z " << z << "\t" << i_Signal_to_sqrtNoise << endl;
                    cout << "___________________________________________________________" << endl;
                }
            }
        } // END loop over all signal efficiency bins
        /////////////////////////
        // determine position of maximum significance
        // fill a histogram from these values, smooth it, and determine position of maximum significance
        double i_xmax = -99.;
        if( iGSignal_to_sqrtNoise )
        {
            TGraphSmooth* iGSmooth = new TGraphSmooth( "s" );
            iGSignal_to_sqrtNoise_Smooth = iGSmooth->SmoothKern( iGSignal_to_sqrtNoise, "normal", 0.05, 100 );
            // get maximum values
            double x = 0.;
            double y = 0.;
            double i_ymax = -99.;
            for( int i = 0; i < iGSignal_to_sqrtNoise_Smooth->GetN(); i++ )
            {
                iGSignal_to_sqrtNoise_Smooth->GetPoint( i, x, y );
                if( y > i_ymax )
                {
                    i_ymax = y;
                    i_xmax = x;
                }
                // stop after first maximum (makes maximum research more robust to fluctuations of the
                // background efficiency)
                else if( y < i_ymax )
                {
                    break;
                }
            }
            i_SignalEfficiency_AtMaximum     = effS->GetBinContent( effS->FindBin( i_xmax ) );
            i_BackgroundEfficiency_AtMaximum = effB->GetBinContent( effB->FindBin( i_xmax ) );
            i_TMVACutValue_AtMaximum         = i_xmax;
            i_Signal_to_sqrtNoise_atMaximum  = i_ymax;
            i_SourceStrength_atMaximum       = iSourceStrength;
        }
        ///////////////////////////////////////////////////////
        // check if value if really at the optimum or if information is missing from background efficiency curve
        // (check if maximum was find in the last bin or if next bin content is zero)
        if(( effB->FindBin( i_xmax ) + 1  < effB->GetNbinsX() && effB->GetBinContent( effB->FindBin( i_xmax ) + 1 ) < 1.e-10 )
                || ( effB->FindBin( i_xmax ) == effB->GetNbinsX() ) )
        {
            if( fDebug )
            {
                cout << "VTMVAEvaluator::optimizeSensitivity: no optimum found" << endl;
                cout << "\t sampling of background cut efficiency not sufficient" << endl;
                if( effB->FindBin( i_xmax ) + 1  < effB->GetNbinsX() )
                {
                    cout << "\t bin " << effB->FindBin( i_xmax ) << "\t" << " bin content ";
                    cout << effB->GetBinContent( effB->FindBin( i_xmax ) + 1 ) << endl;
                }
            }
            // now check slope of sqrtNoise curve (if close to constant -> maximum reached)
            if( iGSignal_to_sqrtNoise_Smooth->Eval( i_TMVACutValue_AtMaximum ) > 0.
                    && iGSignal_to_sqrtNoise_Smooth->Eval( i_TMVACutValue_AtMaximum - 0.02 ) /
                    iGSignal_to_sqrtNoise_Smooth->Eval( i_TMVACutValue_AtMaximum ) > 0.98 )
            {
                cout << "VTMVAEvaluator::optimizeSensitivity: recovered energy bin ";
                cout << iGSignal_to_sqrtNoise_Smooth->Eval( i_TMVACutValue_AtMaximum - 0.02 ) /
                     iGSignal_to_sqrtNoise_Smooth->Eval( i_TMVACutValue_AtMaximum );
                cout << " (" << iDataBin << ")" << endl;
                fTMVAData[iDataBin]->fTMVAOptimumCutValueFound = true;
            }
            fTMVAData[iDataBin]->fTMVAOptimumCutValueFound = false;
        }
        else
        {
            fTMVAData[iDataBin]->fTMVAOptimumCutValueFound = true;
        }

        // check detection criteria
        if( i_Signal_to_sqrtNoise_atMaximum > fOptimizationSourceSignificance
                && Ndif < fOptimizationMinSignalEvents )
        {
            cout << "\t passed significance but not signal events criterium";
            cout << " (" << iSourceStrength << " CU): ";
            cout << "sig " << i_Signal_to_sqrtNoise_atMaximum;
            cout << ", Ndif " << Ndif << endl;
        }
        if( i_Signal_to_sqrtNoise_atMaximum > fOptimizationSourceSignificance
                && Ndif > fOptimizationMinSignalEvents )
        {
            break;
        }

        // delete graphs
        // (not in last step, keep them there for plotting)
        if( s != iSourceStrengthStepSizeN - 1 )
        {
            if( iGSignal_to_sqrtNoise )
            {
                delete iGSignal_to_sqrtNoise;
            }
            if( iGSignalEvents )
            {
                delete iGSignalEvents;
            }
            if( iGBackgroundEvents )
            {
                delete iGBackgroundEvents;
            }
            if( iGSignal_to_sqrtNoise_Smooth )
            {
                delete iGSignal_to_sqrtNoise_Smooth;
            }
        }
    } // end of loop over source strength
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////
    // check if signal efficiency is above allowed value
    if( i_SignalEfficiency_AtMaximum > fOptimizationFixedSignalEfficiency )
    {
        if( fOptimizationFixedSignalEfficiency > 0.99 )
        {
            i_TMVACutValue_AtMaximum         = effS->GetBinCenter( effS->GetNbinsX() - 1 );
            i_BackgroundEfficiency_AtMaximum = effB->GetBinContent( effS->GetNbinsX() - 1 );
        }
        else
        {
            for( int i = 1; i < effS->GetNbinsX(); i++ )
            {
                if( effS->GetBinContent( i ) < fOptimizationFixedSignalEfficiency )
                {
                    i_TMVACutValue_AtMaximum         = effS->GetBinCenter( i );
                    i_BackgroundEfficiency_AtMaximum = effB->GetBinContent( i );
                    break;
                }
            }
        }
        cout << "VTMVAEvaluator::optimizeSensitivity: setting signal efficiency to ";
        cout << fOptimizationFixedSignalEfficiency;
        cout << " (from " << i_SignalEfficiency_AtMaximum << ")" << endl;
        i_SignalEfficiency_AtMaximum = fOptimizationFixedSignalEfficiency;
    }
    else
    {
        cout << "VTMVAEvaluator::optimizeSensitivity: signal efficiency at maximum (";
        cout << i_SourceStrength_atMaximum << " CU) is ";
        cout << i_SignalEfficiency_AtMaximum << " with a significance of " << i_Signal_to_sqrtNoise_atMaximum << endl;
        cout << "\t Ndiff = " << Ndif << endl;
    }
    cout << "\t MVA parameter: " << i_TMVACutValue_AtMaximum;
    cout << ", background efficiency: " << i_BackgroundEfficiency_AtMaximum << endl;
    ////////////////////////////////////////////////////////////////

    // fill results into data vectors
    fTMVAData[iDataBin]->fSignalEfficiency           = i_SignalEfficiency_AtMaximum;
    fTMVAData[iDataBin]->fBackgroundEfficiency       = i_BackgroundEfficiency_AtMaximum;
    fTMVAData[iDataBin]->fTMVACutValue               = i_TMVACutValue_AtMaximum;
    fTMVAData[iDataBin]->fSourceStrengthAtOptimum_CU = i_SourceStrength_atMaximum;

    // plot optimization procedure and event numbers
    if( bPlotEfficiencyPlotsPerBin )
    {
        plotEfficiencyPlotsPerBin( iDataBin, iGSignal_to_sqrtNoise, iGSignal_to_sqrtNoise_Smooth,
                                   effS, effB, iGSignalEvents, iGBackgroundEvents );
    }

    return true;
}

/*
 smoothing of optimal cut value vs energy curves

 (energy axis only)

*/
void VTMVAEvaluator::smoothAndInterPolateMVAValue_EnergyOnly(
    TH1F* effS, TH1F* effB )
{
    int z = 0;
    // fill graph to be smoothed
    TGraph* iG = new TGraph( 1 );
    for( unsigned int i = 0; i < fTMVAData.size(); i++ )
    {
        if( fTMVAData[i] )
        {
            if( fTMVAData[i]->fTMVAOptimumCutValueFound )
            {
                iG->SetPoint( z, fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV, fTMVAData[i]->fTMVACutValue );
                z++;
            }
        }
    }
    // smooth graph
    TGraph* iGout = new TGraph( 1 );
    TGraphSmooth* iGSmooth = new TGraphSmooth( "t" );
    iGout = ( TGraph* )iGSmooth->SmoothKern( iG, "normal", 0.5, 100 );

    // fill smoothed and interpolated values into MVA vector
    // set all points to 'optimized'
    for( unsigned int i = 0; i < fTMVAData.size(); i++ )
    {
        if( fTMVAData[i] )
        {
            cout << "\t TMVA values: unsmoothed at ";
            cout << TMath::Power( 10., fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV );
            cout << " TeV, \t" << fTMVAData[i]->fTMVACutValue;
            fTMVAData[i]->fTMVACutValue = iGout->Eval( fTMVAData[i]->fSpectralWeightedMeanEnergy_Log10TeV );
            cout << ", smoothed " << fTMVAData[i]->fTMVACutValue;
            if(!fTMVAData[i]->fTMVAOptimumCutValueFound )
            {
                cout << " (interpolated non-optimal value)";
            }
            cout << " (" << i << ")" << endl;
            fTMVAData[i]->fTMVAOptimumCutValueFound = true;

            // get efficiency histograms
            TFile iTMVAFile( fTMVAData[i]->fTMVAFileName.c_str() );
            TH1F* effS = getEfficiencyHistogram( "effS", &iTMVAFile, fTMVAData[i]->fTMVAMethodTag_2 );
            TH1F* effB = getEfficiencyHistogram( "effB", &iTMVAFile, fTMVAData[i]->fTMVAMethodTag_2 );

            if( effS )
            {
                fTMVAData[i]->fSignalEfficiency = effS->GetBinContent( effS->FindBin( fTMVAData[i]->fTMVACutValue ) );
            }
            if( effB )
            {
                fTMVAData[i]->fBackgroundEfficiency = effB->GetBinContent( effB->FindBin( fTMVAData[i]->fTMVACutValue ) );
                // background efficiency might be zero -> fill it with first non-zero value
                if( fTMVAData[i]->fBackgroundEfficiency < 1.e-8 )
                {
                    int iS = effB->FindBin( fTMVAData[i]->fTMVACutValue );
                    for( int j = iS; j > 0; j-- )
                    {
                        if( effB->GetBinContent( j ) > 0. )
                        {
                            fTMVAData[i]->fBackgroundEfficiency = effB->GetBinContent( j );
                            break;
                        }
                    }
                }
            }
        }
    }
}

/*

 smoothing of optimal cut value vs energy curves

 (energy and zenith angle dependent)

*/
void VTMVAEvaluator::smoothAndInterPolateMVAValue_Energy_and_Zenith(
    TH1F* effS, TH1F* effB,
    unsigned int iWeightFileIndex_Emin, unsigned int iWeightFileIndex_Emax,
    unsigned int iWeightFileIndex_Zmin, unsigned int iWeightFileIndex_Zmax )
{

    unsigned int Ebins = iWeightFileIndex_Emax - iWeightFileIndex_Emin + 1;
    unsigned int ZEbins = iWeightFileIndex_Zmax - iWeightFileIndex_Zmin + 1;

    // 2D histogram to be smoothed (energy and zenith angle dependence)
    TH2D* iH2 = new TH2D( "h1", "Smooth cut values",
                          Ebins, iWeightFileIndex_Emin, iWeightFileIndex_Emax,
                          ZEbins, iWeightFileIndex_Zmin, iWeightFileIndex_Zmax );
    Double_t effS_value[Ebins * ZEbins];
    for( unsigned int l = 0; l < fTMVAData.size(); l++ )
    {
        if( fTMVAData[l] )
        {
            TFile iTMVAFile( fTMVAData[l]->fTMVAFileName.c_str() );
            if( iTMVAFile.IsZombie() )
            {
                cout << "VTMVAEvaluator: error reading file with cut efficiencies: ";
                cout << fTMVAData[l]->fTMVAFileName << endl;
                cout << "Exiting..." << endl;
                exit( EXIT_FAILURE );
            }
            TH1F* effS = getEfficiencyHistogram( "effS", &iTMVAFile, fTMVAData[l]->fTMVAMethodTag_2 );
            effS_value[l] = effS->GetBinContent( effS->FindBin( fTMVAData[l]->fTMVACutValue ) );
        }
    }
    for( unsigned int z = 0; z < fTMVAData.size(); z++ )
    {
        if(!fTMVAData[z] )
        {
            continue;
            cout << "VTMVAEvaluator: error reading file with cut efficiencies for bins: ";
            cout << ", entry " << z << endl;
            cout << "Exiting..." << endl;
            exit( EXIT_FAILURE );
        }
        // get signal efficiency histograms and cut values
        TFile iTMVAFile( fTMVAData[z]->fTMVAFileName.c_str() );
        TH1F* effS = getEfficiencyHistogram( "effS", &iTMVAFile, fTMVAData[z]->fTMVAMethodTag_2 );
        if( fTMVAData[z]->fTMVAOptimumCutValueFound )
        {
            iH2->SetBinContent( fTMVAData[z]->fEnergyBin, fTMVAData[z]->fZenithBin, fTMVAData[z]->fTMVACutValue );
        }
        // bins without optimal cut value and not in highest energy bin
        else if(!fTMVAData[z]->fTMVAOptimumCutValueFound
                && ( fTMVAData[z]->fZenithBin < ZEbins )
                && ( fTMVAData[z]->fEnergyBin != Ebins - 1 ) )
        {
            for( int k = 0; k < effS->GetNbinsX(); k++ )
            {
                // search for similar cut efficiency in neighbouring bin
                unsigned int i_alt_index = z + 1;
                if( z > 0 )
                {
                    i_alt_index = z - 1;
                }
                if( i_alt_index < fTMVAData.size()
                        && TMath::Abs( effS->GetBinContent( k ) - effS_value[i_alt_index] ) < 0.001 )
                {
                    fTMVAData[z]->fTMVACutValue = effS->GetBinCenter( k );
                    effS_value[z] = effS->GetBinContent( k );
                }
            }
        }
        // bins without optimal cut value and in highest energy bin
        else if(!fTMVAData[z]->fTMVAOptimumCutValueFound
                && ( fTMVAData[z]->fZenithBin < ZEbins ) && ( fTMVAData[z]->fEnergyBin == Ebins - 1 ) )
        {
            for( unsigned int l = 0; l < ZEbins; l++ )
            {
                if( effS_value[fTMVAData.size() - l] > 1.e-10 )
                {
                    for( int k = 0; k < effS->GetNbinsX(); k++ )
                    {
                        if( TMath::Abs( effS->GetBinContent( k ) - effS_value[fTMVAData.size() - l] ) < 0.0001 )
                        {
                            fTMVAData[z]->fTMVACutValue = effS->GetBinCenter( k );
                            effS_value[z] = effS->GetBinContent( k );
                        }
                    }
                }
            }
            if( fTMVAData[z]->fTMVACutValue == -99 )
            {
                cout << "Error: no optimal cut value found for this bin" << endl;
            }
            cout << "\t TMVA values: at " << TMath::Power( 10., fTMVAData[z]->fSpectralWeightedMeanEnergy_Log10TeV );
            cout << " TeV, \t" << fTMVAData[z]->fTMVACutValue;
            if(!fTMVAData[z]->fTMVAOptimumCutValueFound )
            {
                cout << " (interpolated non-optimal value)";
            }
            cout << ", signal efficiency: " << effS_value[z];
            cout << " (" << z << ")" << endl;
        }
    }
}

/*

 smoothing of optimal cut value vs energy curves

 missing (non-optimized) are interpolated

 note: signal and background efficiencies are not updated

*/
void VTMVAEvaluator::smoothAndInterPolateMVAValue(
    TH1F* effS, TH1F* effB,
    unsigned int iWeightFileIndex_Emin, unsigned int iWeightFileIndex_Emax,
    unsigned int iWeightFileIndex_Zmin, unsigned int iWeightFileIndex_Zmax )
{
    if( fTMVAData.size() == 0 )
    {
        return;
    }

    cout << "Smooth and interpolate MVA cut values" << endl;

    //////////////////////////////////////////////
    // energy dependent TMVA cut optimization only
    if( iWeightFileIndex_Zmax == iWeightFileIndex_Zmin )
    {
        smoothAndInterPolateMVAValue_EnergyOnly( effS, effB );
    }
    // energy and zenith angle dependent
    else
    {
        smoothAndInterPolateMVAValue_Energy_and_Zenith(
            effS, effB,
            iWeightFileIndex_Emin, iWeightFileIndex_Emax,
            iWeightFileIndex_Zmin, iWeightFileIndex_Zmax );
    }
}

void VTMVAEvaluator::plotEfficiencyPlotsPerBin( unsigned int iBin, TGraph* iGSignal_to_sqrtNoise,
        TGraph* iGSignal_to_sqrtNoise_Smooth, TH1F* hEffS, TH1F* hEffB,
        TGraph* iGSignalEvents, TGraph* iGBackgroundEvents )
{
    char hname[800];
    char htitle[800];

    if( iBin >= fTMVAData.size() || !fTMVAData[iBin] )
    {
        return;
    }

    // signal and noise plot
    if( hEffS )
    {
        sprintf( hname, "cEfficiencyPlotPerEnergy_%d", iBin );
        sprintf( htitle, "efficiency plots (%s)", fTMVAData[iBin]->GetTitle() );
        TCanvas* iCanvas = new TCanvas( hname, htitle, 10, 10 + iBin * 30, 400, 400 );
        iCanvas->SetGridx( 0 );
        iCanvas->SetGridy( 0 );
        iCanvas->SetLeftMargin( 0.13 );
        iCanvas->Draw();

        hEffS->SetStats( 0 );
        hEffS->SetTitle( "" );
        hEffS->SetLineWidth( 3 );
        hEffS->GetYaxis()->SetTitleOffset( 1.5 );
        hEffS->SetXTitle( "MVA value #Tau" );
        hEffS->SetYTitle( "signal/background efficiency" );
        hEffS->DrawCopy();

        if( hEffB )
        {
            hEffB->SetStats( 0 );
            hEffB->SetTitle( "" );
            hEffB->SetLineColor( 2 );
            hEffB->SetLineWidth( 3 );
            hEffB->SetLineStyle( 9 );
            hEffB->DrawCopy( "same" );
        }

        if( iBin < fTMVAData.size() && fTMVAData[iBin] )
        {
            TLine* iL = new TLine( fTMVAData[iBin]->fTMVACutValue, hEffS->GetMinimum(), fTMVAData[iBin]->fTMVACutValue, hEffS->GetMaximum() );
            iL->SetLineStyle( 2 );
            iL->SetLineWidth( 3 );
            iL->Draw();
        }

        if( fPrintPlotting )
        {
            sprintf( hname, "MVAOpt-SignalBackgroundEfficiency-%s.pdf", fTMVAData[iBin]->fTMVAName.c_str() );
            iCanvas->Print( hname );
        }
    }

    // signal to noise
    if( iGSignal_to_sqrtNoise )
    {
        sprintf( hname, "cSignalToSqrtNoise_%d", iBin );
        sprintf( htitle, "signal / sqrt( noise ) (%s)", fTMVAData[iBin]->GetTitle() );
        TCanvas* iCanvas = new TCanvas( hname, htitle, 425, 10 + iBin * 30, 400, 400 );
        iCanvas->SetLeftMargin( 0.12 );
        iCanvas->SetGridx( 0 );
        iCanvas->SetGridy( 0 );

        sprintf( hname, "hSignalToSqrtNoise_%d", iBin );
        TH1F* iSignal_to_sqrtNoise = new TH1F( hname, "", 100, -1., 1. );
        iSignal_to_sqrtNoise->SetMinimum( 0. );
        iSignal_to_sqrtNoise->SetMaximum( 6. );
        iSignal_to_sqrtNoise->SetXTitle( "MVA value #Tau" );
        iSignal_to_sqrtNoise->SetYTitle( "significance" );
        iSignal_to_sqrtNoise->GetYaxis()->SetTitleOffset( 1.3 );
        iSignal_to_sqrtNoise->SetStats( 0 );
        iSignal_to_sqrtNoise->Draw();

        iGSignal_to_sqrtNoise->SetTitle( "" );
        setGraphPlottingStyle( iGSignal_to_sqrtNoise, 4, 1., 20, 0.5, 0, 1 );

        iGSignal_to_sqrtNoise->Draw( "pl" );

        if( iGSignal_to_sqrtNoise_Smooth )
        {
            setGraphPlottingStyle( iGSignal_to_sqrtNoise_Smooth, 2, 1., 20, 0.5 );
            iGSignal_to_sqrtNoise_Smooth->Draw( "pl" );
        }

        if( iBin < fTMVAData.size() && fTMVAData[iBin] )
        {
            TLine* iL = new TLine( fTMVAData[iBin]->fTMVACutValue, iGSignal_to_sqrtNoise->GetHistogram()->GetMinimum(),
                                   fTMVAData[iBin]->fTMVACutValue, iGSignal_to_sqrtNoise->GetHistogram()->GetMaximum() );
            iL->SetLineStyle( 2 );
            iL->Draw();
        }
        if( fPrintPlotting )
        {
            sprintf( hname, "MVAOpt-SignalToSqrtNoise-%s.pdf", fTMVAData[iBin]->fTMVAName.c_str() );
            iCanvas->Print( hname );
        }
    }

    // signal and background events numbers
    if( iGBackgroundEvents )
    {
        sprintf( hname, "cEventNumbers_%d", iBin );
        sprintf( htitle, "event numbers (%s)", fTMVAData[iBin]->GetTitle() );
        TCanvas* iCanvas = new TCanvas( hname, htitle, 850, 10 + iBin * 30, 400, 400 );
        iCanvas->SetLeftMargin( 0.13 );
        iCanvas->SetGridx( 0 );
        iCanvas->SetGridy( 0 );

        sprintf( hname, "hBC_%d", iBin );
        TH1D* hnull = new TH1D( hname, "", 100, -1., 1. );
        hnull->SetXTitle( "cut value" );
        hnull->SetYTitle( "number of events" );
        hnull->SetMinimum( 1.e-3 );
        double x = 0.;
        double y = 0.;
        double y_max = 0.;
        for( int i = 0; i < iGBackgroundEvents->GetN(); i++ )
        {
            iGBackgroundEvents->GetPoint( i, x, y );
            if( y > y_max )
            {
                y_max = y;
            }
        }
        for( int i = 0; i < iGSignalEvents->GetN(); i++ )
        {
            iGSignalEvents->GetPoint( i, x, y );
            if( y > y_max )
            {
                y_max = y;
            }
        }
        hnull->SetMaximum( y_max * 1.5 );
        hnull->SetStats( 0 );
        hnull->GetYaxis()->SetTitleOffset( 1.5 );
        hnull->DrawCopy();

        setGraphPlottingStyle( iGBackgroundEvents, 1, 1., 20 );
        iGBackgroundEvents->Draw( "pl" );

        if( iGSignalEvents )
        {
            setGraphPlottingStyle( iGSignalEvents, 2, 2. );
            iGSignalEvents->Draw( "pl" );
        }

        if( iBin < fTMVAData.size() && fTMVAData[iBin] )
        {
            TLine* iL = new TLine( fTMVAData[iBin]->fTMVACutValue, hnull->GetMinimum(),
                                   fTMVAData[iBin]->fTMVACutValue, hnull->GetMaximum() );
            iL->SetLineStyle( 2 );
            iL->Draw();
            TLine* iLMinBack = new TLine( hnull->GetXaxis()->GetXmin(), fOptimizationMinBackGroundEvents,
                                          hnull->GetXaxis()->GetXmax(), fOptimizationMinBackGroundEvents );
            iLMinBack->SetLineStyle( 2 );
            iLMinBack->Draw();
        }
    }

}

void VTMVAEvaluator::setTMVAMethod( string iMethodName )
{
    fTMVAMethodName = iMethodName;
}

void VTMVAEvaluator::setSensitivityOptimizationFixedSignalEfficiency( double iOptimizationFixedSignalEfficiency )
{
    fOptimizationFixedSignalEfficiency = iOptimizationFixedSignalEfficiency;
}

void VTMVAEvaluator::setSensitivityOptimizationMinSourceStrength( double iOptimizationMinSourceStrength )
{
    fOptimizationMinSourceStrength = iOptimizationMinSourceStrength;
}

double VTMVAEvaluator::getValueFromMap( map< unsigned int, double > iDataMap, double iDefaultData,
                                        unsigned int iEnergyBin, double iE_min_log10, double iE_max_log10,
                                        unsigned int iZenithBin, double iZ_min, double iZ_max, string iVariable )
{
    if( iDataMap.size() == 0 )
    {
        return iDefaultData;
    }

    map< unsigned int, double >::iterator iIter;

    for( iIter = iDataMap.begin(); iIter != iDataMap.end(); iIter++ )
    {
        // data does not depend on energy
        if( iIter->first > 9998 )
        {
            return iIter->second;
        }

        // data signal efficiency
        if( iIter->first == ( iEnergyBin * 10 + iZenithBin ) )
        {
            cout << "VTMVAEvaluator::getValueFromMap (" << iVariable << "): ";
            cout << "E [" << iE_min_log10 << ", " << iE_max_log10 << "], bin ";
            cout << iEnergyBin << ": ";
            cout << "Zen [" << iZ_min << ", " << iZ_max << "], bin ";
            cout << iZenithBin << ":\t ";
            cout << iIter->first << "\t" << iIter->second << endl;
            return iIter->second;
        }
    }

    cout << "VTMVAEvaluator::getValueFromMap: warning, couldn't find a data value (" << iVariable << ") for energy bin ";
    cout << iEnergyBin << ", E=[ " << iE_min_log10 << ", " << iE_max_log10 << "] and zenith bin ";
    cout << iZenithBin << ", Zen=[ " << iZ_min << ", " << iZ_max << "] " << endl;

    return -1.;
}

/*
 * return signal efficiency at a given energy
 *
 */
double VTMVAEvaluator::getSignalEfficiency( unsigned int iEnergyBin, double iE_min_log10, double iE_max_log10, unsigned int iZenithBin, double iZ_min, double iZ_max )
{
    return getValueFromMap( fSignalEfficiencyMap, fSignalEfficiencyNoVec, iEnergyBin, iE_min_log10, iE_max_log10, iZenithBin, iZ_min, iZ_max, "SignalEfficiency" );
}

double VTMVAEvaluator::getTMVACutValue( unsigned int iEnergyBin, double iE_min_log10, double iE_max_log10, unsigned int iZenithBin, double iZ_min, double iZ_max )
{
    return getValueFromMap( fTMVACutValueMap, fTMVACutValueNoVec, iEnergyBin, iE_min_log10, iE_max_log10, iZenithBin, iZ_min, iZ_max, "MVA_CUT" );
}



void VTMVAEvaluator::printSensitivityOptimizationParameters()
{
    cout << "VTMAEvaluator: MVA cut parameter is optimized for: " << endl;
    cout << "\t" << fOptimizationObservingTime_h << " hours of observing time" << endl;
    cout << "\t" << fOptimizationSourceSignificance << " minimum significance" << endl;
    cout << "\t" << fOptimizationMinSignalEvents << " minimum number of on events" << endl;
    cout << "\t" << fOptimizationBackgroundAlpha << " signal to background area ratio" << endl;
    cout << "\t" << fOptimizationFixedSignalEfficiency << " maximum signal efficiency" << endl;
    cout << "\t" << fOptimizationMinSourceStrength << " minimum source strength" << endl;
}

vector< double > VTMVAEvaluator::getBackgroundEfficiency()
{
    vector< double > iA;
    for( unsigned int i = 0; i < fTMVAData.size(); i++ )
    {
        if( fTMVAData[i] )
        {
            iA.push_back( fTMVAData[i]->fBackgroundEfficiency );
        }
    }
    return iA;
}

vector< double > VTMVAEvaluator::getSignalEfficiency()
{
    vector< double > iA;
    for( unsigned int i = 0; i < fTMVAData.size(); i++ )
    {
        if( fTMVAData[i] )
        {
            iA.push_back( fTMVAData[i]->fSignalEfficiency );
        }
    }
    return iA;
}

vector< double > VTMVAEvaluator::getTMVACutValue()
{
    vector< double > iA;
    for( unsigned int i = 0; i < fTMVAData.size(); i++ )
    {
        if( fTMVAData[i] )
        {
            iA.push_back( fTMVAData[i]->fTMVACutValue );
        }
    }
    return iA;
}

vector< bool > VTMVAEvaluator::getOptimumCutValueFound()
{
    vector< bool > iA;
    for( unsigned int i = 0; i < fTMVAData.size(); i++ )
    {
        if( fTMVAData[i] )
        {
            iA.push_back( fTMVAData[i]->fTMVAOptimumCutValueFound );
        }
    }
    return iA;
}

/*
 * read graph with on/off numbers as a function of energy and possibly zenith angle
 * interpolates between zenith range and return a TGraph
 *
 */
TGraph* VTMVAEvaluator::readNonNoffGraphsFromFile(
    TFile* iF, double i_ze_min, double i_ze_max, bool bIsOn )
{
    if(!iF && iF->IsZombie() )
    {
        return 0;
    }
    TObject* i_N = 0;
    if( bIsOn )
    {
        i_N = iF->Get( "gONRate" );
    }
    else
    {
        i_N = iF->Get( "gBGRate" );
    }
    return ( TGraph* )fillfromGraph2D( i_N, i_ze_min, i_ze_max );
}

/*
 * fill by interpolation a TGraph2D into TGraph (if necessary)
 *
 */
TGraph* VTMVAEvaluator::fillfromGraph2D( TObject* i_G, double i_ze_min, double i_ze_max )
{
    if(!i_G )
    {
        return 0;
    }

    string i_c = i_G->ClassName();
    // graph is 2D: interpolate and fill into a TGraph
    if( i_c.find( "TGraph2D" ) != string::npos )
    {
        TGraph2D* iG2D = ( TGraph2D* )i_G;
        TGraph* iG1D = new TGraph( iG2D->GetN() );
        Double_t* x = iG2D->GetX();
        // TEMP: weight by cos?
        double ze_mean = 0.5 * ( i_ze_min + i_ze_max );

        for( int i = 0; i < iG2D->GetN(); i++ )
        {
            Double_t y = iG2D->Interpolate( x[i], ze_mean );
            iG1D->SetPoint( i, x[i], y );
        }
        return iG1D;
    }

    // graph is already 1D
    return ( TGraph* )i_G;
}

/*
 * Calculate rough average zenith angle per run
 *
 * Optionally used for selection of data bin for MVA evaluation
 */
void VTMVAEvaluator::calculate_average_zenith_angle()
{
    if(!fData ) return;

    double i_ze = 0.;
    double i_n = 0.;

    fData->GetEntry( 0 );
    i_ze = fData->ArrayPointing_Elevation;
    i_n++;
    Long64_t nEntries = fData->fChain->GetEntries();
    if( nEntries > 1 )
    {
        fData->GetEntry( nEntries / 2 );
        i_ze += fData->ArrayPointing_Elevation;
        i_n++;
        fData->GetEntry( nEntries - 1 );
        i_ze += fData->ArrayPointing_Elevation;
        i_n++;
    }
    if( i_n > 0. )
    {
        fAverageZenithPerRun = 90. - i_ze / i_n;
    }
    cout << "VTMVAEvaluator: average zenith " << fAverageZenithPerRun << endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

VTMVAEvaluatorData::VTMVAEvaluatorData()
{
    fTMVAName = "";
    fTMVAFileName = "";
    fTMVAFileNameXML = "";
    fTMVAMethodTag = "";
    fTMVAMethodTag_2 = "";
    fEnergyBin = 0;
    fEnergyCut_Log10TeV_min = -99.;
    fEnergyCut_Log10TeV_max = -99.;
    fSpectralWeightedMeanEnergy_Log10TeV = -99.;
    fZenithBin = 0;
    fZenithCut_min = -99.;
    fZenithCut_max = -99.;

    fSignalEfficiency = -99.;
    fBackgroundEfficiency = -99.;
    fTMVACutValue = -99.;
    fTMVAOptimumCutValueFound = false;
    fSourceStrengthAtOptimum_CU = -99.;

    // transients
    fTMVAReader = 0;
}

void VTMVAEvaluatorData::print()
{
    cout << "\t file " << fTMVAFileName << endl;
    cout << "\t energy bin [" << fEnergyCut_Log10TeV_min << "," << fEnergyCut_Log10TeV_max << "] ";
    cout << "(mean energy " << fSpectralWeightedMeanEnergy_Log10TeV << ")";
    cout << ", zenith bin [" << fZenithCut_min << "," << fZenithCut_max << "]";
    cout << endl;
}
