/*! \class VEffectiveAreaCalculator
 *  \brief calculate effective areas and energy spectra
 *
 */

#include "VEffectiveAreaCalculator.h"

/*!
 *  CALLED FOR CALCULATION OF EFFECTIVE AREAS
 *
 *  this constructor is called for FILLING of the effective area tree
 *  (calculation of effective areas)
 *
 */
VEffectiveAreaCalculator::VEffectiveAreaCalculator( VInstrumentResponseFunctionRunParameter* iRunPara, VGammaHadronCuts* icuts )
{
    fRunPara = iRunPara;
    if(!fRunPara )
    {
        cout << "VEffectiveAreaCalculator: no run parameters given" << endl;
        cout << "...exiting..." << endl;
        exit( EXIT_FAILURE );
    }
    reset();

    // no effective area file present
    bNOFILE = true;

    // number of energy bins (general)
    nbins = fRunPara->fEnergyAxisBins_log10;
    nbins_MC = fRunPara->fEnergyAxisBins_log10;

    // bin definition for 2D histograms (allows coarser binning in energy)
    fBiasBin       = fRunPara->fBiasBin;
    fhistoNEbins   = fRunPara->fhistoNEbins;
    fLogAngularBin = fRunPara->fLogAngularBin;
    fResponseMatricesEbinning = fRunPara->fResponseMatricesEbinning;

    // this should not be changed
    fEnergyAxis_minimum_defaultValue = -2.;
    fEnergyAxis_maximum_defaultValue =  4.;
    fLogAngular_minimum_defaultValue = -4.;
    fLogAngular_maximum_defaultValue =  1.;

    fGauss = new TF1( "fGauss", "gaus", -2.5, 2.5 );

    // cuts
    fCuts = icuts;
    fZe.push_back( fRunPara->fze );
    fAreaRadius.push_back( fRunPara->fCoreScatterRadius );
    fScatterMode.push_back( fRunPara->fCoreScatterMode );
    for( unsigned int i = 0; i < fZe.size(); i++ )
    {
        fXWobble.push_back( 0. );
        fYWobble.push_back( 0. );
        fNoise.push_back( 0 );
        fPedVar.push_back( 0. );
    }
    setIgnoreEnergyReconstructionCuts( fRunPara->fIgnoreEnergyReconstructionQuality );
    setIsotropicArrivalDirections( fRunPara->fIsotropicArrivalDirections );
    setTelescopeTypeCuts( fRunPara->fTelescopeTypeCuts );
    setWobbleOffset( fRunPara->fXoff, fRunPara->fYoff );
    setNoiseLevel( fRunPara->fnoise, fRunPara->fpedvar );

    // spectral weighting class
    fSpectralWeight = new VSpectralWeight();
    setMonteCarloEnergyRange( fRunPara->fMCEnergy_min, fRunPara->fMCEnergy_max, TMath::Abs( fRunPara->fMCEnergy_index ) );

    // define output tree (all histograms are written to this tree)
    hisTreeList = new TList();

    // these histograms are filled into the output tree
    char hname[400];
    char htitle[400];

    // define range and binning of energy axis of effective area plots
    cout << "histogram parameters (bins, log10(Emin), log10(Emax)): " << nbins;
    cout << ", " << fEnergyAxis_minimum_defaultValue << ", " << fEnergyAxis_maximum_defaultValue << endl;
    sprintf( htitle, "title" );

    sprintf( hname, "hEmc" );
    hEmc = new TH1D( hname, htitle, nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue );
    hEmc->Sumw2();
    hEmc->SetXTitle( "energy_{MC} [TeV]" );
    hEmc->SetYTitle( "entries" );
    hisTreeList->Add( hEmc );

    sprintf( hname, "hEcut" );
    hEcut = new TH1D( hname, htitle, nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue );
    hEcut->Sumw2();
    hEcut->SetXTitle( "energy_{MC} [TeV]" );
    hEcut->SetYTitle( "entries" );
    hisTreeList->Add( hEcut );

    sprintf( hname, "hEcutUW" );
    hEcutUW = new TH1D( hname, htitle, nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue );
    hEcutUW->Sumw2();
    hEcutUW->SetXTitle( "energy_{MC} [TeV]" );
    hEcutUW->SetYTitle( "entries (unweighted)" );
    hisTreeList->Add( hEcutUW );

    sprintf( hname, "hEcutLin" );
    hEcutLin  = new TH1D( hname, htitle, 500, 0.03, 4.03 );
    hEcutLin->Sumw2();
    hEcutLin->SetXTitle( "energy_{MC} [TeV]" );
    hEcutLin->SetYTitle( "entries" );
    hisTreeList->Add( hEcutLin );

    sprintf( hname, "hEcut500" );
    hEcut500 = new TH1D( hname, htitle, 500, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue );
    hEcut500->Sumw2();
    hEcut500->SetXTitle( "energy_{MC} [TeV]" );
    hEcut500->SetYTitle( "entries" );
    hisTreeList->Add( hEcut500 );

    sprintf( hname, "hEcutRec" );
    hEcutRec = new TH1D( hname, htitle, nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue );
    hEcutRec->Sumw2();
    hEcutRec->SetXTitle( "energy_{rec} [TeV]" );
    hEcutRec->SetYTitle( "entries" );
    hisTreeList->Add( hEcutRec );

    sprintf( hname, "hEcutRecUW" );
    hEcutRecUW = new TH1D( hname, htitle, nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue );
    hEcutRecUW->Sumw2();
    hEcutRecUW->SetXTitle( "energy_{rec} [TeV]" );
    hEcutRecUW->SetYTitle( "entries" );
    hisTreeList->Add( hEcutRecUW );

    sprintf( hname, "hEcutNoTh2" );
    hEcutNoTh2 = new TH1D( hname, htitle, nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue );
    hEcutNoTh2->Sumw2();
    hEcutNoTh2->SetXTitle( "energy_{MC} [TeV]" );
    hEcutNoTh2->SetYTitle( "entries" );
    hisTreeList->Add( hEcutNoTh2 );

    sprintf( hname, "hEcutRecNoTh2" );
    hEcutRecNoTh2 = new TH1D( hname, htitle, nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue );
    hEcutRecNoTh2->Sumw2();
    hEcutRecNoTh2->SetXTitle( "energy_{rec} [TeV]" );
    hEcutRecNoTh2->SetYTitle( "entries" );
    hisTreeList->Add( hEcutRecNoTh2 );

    sprintf( hname, "gEffAreaMC" );
    gEffAreaMC = new TGraphAsymmErrors( 1 );
    gEffAreaMC->SetName( hname );
    gEffAreaMC->SetTitle( htitle );
    hisTreeList->Add( gEffAreaMC );

    sprintf( hname, "gEffAreaRec" );
    gEffAreaRec = new TGraphAsymmErrors( 1 );
    gEffAreaRec->SetName( hname );
    gEffAreaRec->SetTitle( htitle );
    hisTreeList->Add( gEffAreaRec );

    sprintf( hname, "gEffAreaNoTh2MC" );
    gEffAreaNoTh2MC = new TGraphAsymmErrors( 1 );
    gEffAreaNoTh2MC->SetName( hname );
    gEffAreaNoTh2MC->SetTitle( htitle );
    //hisTreeList->Add( gEffAreaNoTh2MC );

    sprintf( hname, "gEffAreaNoTh2Rec" );
    gEffAreaNoTh2Rec = new TGraphAsymmErrors( 1 );
    gEffAreaNoTh2Rec->SetName( hname );
    gEffAreaNoTh2Rec->SetTitle( htitle );
    //hisTreeList->Add( gEffAreaNoTh2Rec );

    // spectral weight
    sprintf( hname, "hEmcSWeight" );
    hEmcSWeight = new TProfile( hname, htitle, nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, 0., 1.e12 );
    hEmcSWeight->SetXTitle( "log_{10} energy_{MC} [TeV]" );
    hEmcSWeight->SetYTitle( "spectral weight" );
    hisTreeList->Add( hEmcSWeight );

    // histograms for energy reconstruction quality
    sprintf( hname, "hEsysRec" );
    hEsysRec = new TProfile( hname, htitle, fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, -1000., 1000., "s" );
    hEsysRec->SetXTitle( "energy_{rec} [TeV]" );
    hEsysRec->SetYTitle( "log_{10} E_{rec} - log_{10} E_{MC}" );
    hisTreeList->Add( hEsysRec );

    sprintf( hname, "hEsysMC" );
    hEsysMC = new TProfile( hname, htitle, fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, -1000., 1000., "s" );
    hEsysMC->SetXTitle( "energy_{MC} [TeV]" );
    hEsysMC->SetYTitle( "log_{10} E_{rec} - log_{10} E_{MC}" );
    hisTreeList->Add( hEsysMC );

    sprintf( hname, "hEsysMCRelative" );
    hEsysMCRelative = new TProfile( hname, htitle, fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, -1000., 1000., "s" );
    hEsysMCRelative->SetXTitle( "energy_{MC} [TeV]" );
    hEsysMCRelative->SetYTitle( "energy bias (E_{rec}-E_{MC})/E_{MC}" );
    hisTreeList->Add( hEsysMCRelative );

    sprintf( hname, "hEsysMCRelativeRMS" );
    hEsysMCRelativeRMS = new TH2F( hname, htitle, fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, 1000, -5., 5. );
    hEsysMCRelativeRMS->SetXTitle( "energy_{MC} [TeV]" );
    hEsysMCRelativeRMS->SetYTitle( "energy bias (E_{rec}-E_{MC})/E_{MC}" );
    hisTreeList->Add( hEsysMCRelativeRMS );

    sprintf( hname, "hEsysMCRelative2D" );
    hEsysMCRelative2D = new TH2F( hname, htitle,
                                  fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue,
                                  fBiasBin, 0., 3. );
    hEsysMCRelative2D->SetXTitle( "energy_{MC} [TeV]" );
    hEsysMCRelative2D->SetYTitle( "energy bias E_{rec}/E_{MC}" );
    hisTreeList->Add( hEsysMCRelative2D );

    sprintf( hname, "hEsysMCRelative2DNoDirectionCut" );
    hEsysMCRelative2DNoDirectionCut = new TH2F( hname,
        "energy reconstruction, after gamma-selection cuts",
        fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue,
        fBiasBin, 0., 3. ); // default binning y:300 -> 150
    hEsysMCRelative2DNoDirectionCut->SetXTitle( "energy_{MC} [TeV]" );
    hEsysMCRelative2DNoDirectionCut->SetYTitle( "energy bias E_{rec}/E_{MC}" );
    hisTreeList->Add( hEsysMCRelative2DNoDirectionCut );

    sprintf( hname, "hEsys2D" );
    hEsys2D = new TH2F( hname, htitle, fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, 100, -0.98, 2.02 );
    hEsys2D->SetXTitle( "energy_{MC} [TeV]" );
    hEsys2D->SetYTitle( "log_{10} E_{rec} - log_{10} E_{MC}" );
    hisTreeList->Add( hEsys2D );

    sprintf( hname, "hResponseMatrix" );
    hResponseMatrix = new TH2F( hname, htitle, fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue,
                                fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue );

    hResponseMatrix->SetYTitle( "energy_{MC} [TeV]" );
    hResponseMatrix->SetXTitle( "energy_{rec} [TeV]" );
    hisTreeList->Add( hResponseMatrix );

    sprintf( hname, "hResponseMatrixProfile" );
    hResponseMatrixProfile = new TProfile( hname, htitle, fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue,
                                           fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue );
    hResponseMatrixProfile->SetYTitle( "energy_{MC} [TeV]" );
    hResponseMatrixProfile->SetXTitle( "energy_{rec} [TeV]" );
    hisTreeList->Add( hResponseMatrixProfile );

    sprintf( hname, "hResponseMatrixQC" );
    hResponseMatrixQC = new TH2F( hname, htitle, fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue,
                                  fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue );
    hResponseMatrixQC->SetYTitle( "energy_{MC} [TeV]" );
    hResponseMatrixQC->SetXTitle( "energy_{rec} [TeV]" );
    hisTreeList->Add( hResponseMatrixQC );

    // following CTA WP Phys binning convention
    sprintf( hname, "hEmcCutCTA" );
    hEmcCutCTA = new TH2F( hname, htitle, fResponseMatricesEbinning, -2.3, 2.7, fResponseMatricesEbinning, -2.3, 2.7 );
    hEmcCutCTA->SetYTitle( "energy_{MC} [TeV]" );
    hEmcCutCTA->SetXTitle( "energy_{rec} [TeV]" );
    hisTreeList->Add( hEmcCutCTA );

    sprintf( hname, "hResponseMatrixFine" );
    hResponseMatrixFine = new TH2F( hname, htitle, fResponseMatricesEbinning, -2.3, 2.7, fResponseMatricesEbinning, -2.3, 2.7 );
    hResponseMatrixFine->SetYTitle( "energy_{MC} [TeV]" );
    hResponseMatrixFine->SetXTitle( "energy_{rec} [TeV]" );
    //hisTreeList->Add( hResponseMatrixFine );

    sprintf( hname, "hResponseMatrixFineQC" );
    hResponseMatrixFineQC = new TH2F( hname, htitle, fResponseMatricesEbinning, -2.3, 2.7, fResponseMatricesEbinning, -2.3, 2.7 );
    hResponseMatrixFineQC->SetYTitle( "energy_{MC} [TeV]" );
    hResponseMatrixFineQC->SetXTitle( "energy_{rec} [TeV]" );
    hisTreeList->Add( hResponseMatrixFineQC );

    // response matrix after gamma/hadron separation but
    // without direction cut
    sprintf( hname, "hResponseMatrixNoDirectionCut" );
    hResponseMatrixNoDirectionCut = new TH2F( hname, "migration matrix, after gamma-selection cuts",
        fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue,
        fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue );
    hResponseMatrixNoDirectionCut->SetYTitle( "energy_{MC} [TeV]" );
    hResponseMatrixNoDirectionCut->SetXTitle( "energy_{rec} [TeV]" );
    //hisTreeList->Add( hResponseMatrixNoDirectionCut );

    sprintf( hname, "hResponseMatrixFineNoDirectionCut" );
    hResponseMatrixFineNoDirectionCut = new TH2F( hname, "migration matrix, after gamma-selection cuts, fine binning", fResponseMatricesEbinning, -2.3, 2.7, fResponseMatricesEbinning, -2.3, 2.7 );
    hResponseMatrixFineNoDirectionCut->SetYTitle( "energy_{MC} [TeV]" );
    hResponseMatrixFineNoDirectionCut->SetXTitle( "energy_{rec} [TeV]" );
    //hisTreeList->Add( hResponseMatrixFineNoDirectionCut );

    // angular difference histogram (vs reconstructed energy)
    sprintf( hname, "hAngularDiff_2D" );
    hAngularDiff_2D = new TH2F( hname, "angular difference histogram (vs reconstructed energy)",
                                fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, //25, -1.9, 3.5,
                                9000, 0., 4.5 );
    hAngularDiff_2D->SetXTitle( "energy_{rec} [TeV]" );
    hAngularDiff_2D->SetYTitle( "angular diff. (R,MC) [deg]" );
    //hisTreeList->Add( hAngularDiff_2D );

    // angular difference histogram (vs true energy)
    sprintf( hname, "hAngularDiffEmc_2D" );
    hAngularDiffEmc_2D = new TH2F( hname, "angular difference histogram (vs true energy)",
                                   fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, //25, -1.9, 3.5,
                                   9000, 0., 4.5 );
    hAngularDiffEmc_2D->SetXTitle( "energy_{MC} [TeV]" );
    hAngularDiffEmc_2D->SetYTitle( "angular diff. (R,MC) [deg]" );
    //hisTreeList->Add( hAngularDiffEmc_2D );

    // log angular difference histogram (vs reconstructed energy)
    sprintf( hname, "hAngularLogDiff_2D" );
    hAngularLogDiff_2D = new TH2F( hname, "log angular difference histogram (vs reconstructed energy)",
                                   fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, //25, -1.9, 3.5,
                                   fLogAngularBin, fLogAngular_minimum_defaultValue, fLogAngular_maximum_defaultValue );
    hAngularLogDiff_2D->SetXTitle( "energy_{rec} [TeV]" );
    hAngularLogDiff_2D->SetYTitle( "log_{10}(angular diff. (R,MC) [deg])" );
    //hisTreeList->Add( hAngularLogDiff_2D );

    // log angular difference histogram (vs true energy)
    sprintf( hname, "hAngularLogDiffEmc_2D" );
    hAngularLogDiffEmc_2D = new TH2F( hname, "log angular difference histogram (vs true energy)",
                                      fhistoNEbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, //25, -1.9, 3.5,
                                      fLogAngularBin, fLogAngular_minimum_defaultValue, fLogAngular_maximum_defaultValue );
    hAngularLogDiffEmc_2D->SetXTitle( "energy_{MC} [TeV]" );
    hAngularLogDiffEmc_2D->SetYTitle( "log_{10}(angular diff. (R,MC) [deg])" );
    hisTreeList->Add( hAngularLogDiffEmc_2D );

    // weighted rate
    // (use CTA binning, 5 bins per decade)
    sprintf( hname, "hWeightedRate" );
    hWeightedRate = new TH1D( hname, htitle, 30, -2.9, 3.1 );
    hWeightedRate->Sumw2();
    hWeightedRate->SetXTitle( "energy_{rec} [TeV]" );
    hWeightedRate->SetYTitle( "entries" );
    hisTreeList->Add( hWeightedRate );

    // weighted rate
    // (finner binning, primarily used for VTS analysis)
    sprintf( hname, "hWeightedRate005" );
    hWeightedRate005 = new TH1D( hname, "weighted rates (005 binning)", 120, -2.9, 3.1 );
    hWeightedRate005->Sumw2();
    hWeightedRate005->SetXTitle( "energy_{rec} [TeV]" );
    hWeightedRate005->SetYTitle( "entries" );
    //hisTreeList->Add( hWeightedRate005 );

    // angular resolution graphs
    for( unsigned int i = 0; i < fRunPara->fAzMin.size(); i++ )
    {
        fGraph_AngularResolution68p.push_back( 0 );
        fGraph_AngularResolution95p.push_back( 0 );
        hVAngularDiff_2D.push_back( 0 );
        hVAngularDiffEmc_2D.push_back( 0 );
        hVAngularLogDiff_2D.push_back( 0 );
        hVAngularLogDiffEmc_2D.push_back( 0 );
    }

    // individual cuts
    vector< string > iCutName;
    iCutName.push_back( "hEcutTrigger" );
    iCutName.push_back( "hEcutFiducialArea" );
    iCutName.push_back( "hEcutStereoQuality" );
    iCutName.push_back( "hEcutTelType" );
    iCutName.push_back( "hEcutDirection" );
    iCutName.push_back( "hEcutEnergyReconstruction" );
    iCutName.push_back( "hEcutGammaHadron" );

    for( unsigned int i = 0; i < iCutName.size(); i++ )
    {
        sprintf( hname, "h%s", iCutName[i].c_str() );
        hEcutSub.push_back( new TH1D( hname, htitle, nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue ) );
        hEcutSub.back()->Sumw2();
        hEcutSub.back()->SetXTitle( "energy_{MC} [TeV]" );
        hEcutSub.back()->SetYTitle( "entries" );
        hisTreeList->Add( hEcutSub.back() );
    }


    fEffArea = new TTree( "fEffArea", "effective area values" );
    fEffArea->Branch( "ze", &ze, "ze/D" );
    fEffArea->Branch( "az", &fAzBin, "az/I" );
    fEffArea->Branch( "azMin", &fMinAz, "azMin/D" );
    fEffArea->Branch( "azMax", &fMaxAz, "azMax/D" );
    fEffArea->Branch( "Xoff", &fXoff, "Xoff/D" );
    fEffArea->Branch( "Yoff", &fYoff, "Yoff/D" );
    fEffArea->Branch( "Woff", &fWoff, "Woff/D" );
    fEffArea->Branch( "noise", &fTNoise, "noise/I" );
    fEffArea->Branch( "noisePE", &fTNoisePE, "noisePE/D" );
    fEffArea->Branch( "pedvar", &fTPedvar, "pedvar/D" );
    fEffArea->Branch( "index", &fSpectralIndex, "index/D" );
    //
    fEffArea->Branch( "nbins", &nbins, "nbins/I" );
    fEffArea->Branch( "e0", e0, "e0[nbins]/D" );  // log10( energy ) in [TeV]
    fEffArea->Branch( "eff", eff, "eff[nbins]/D" ); // effective area vs MC energy
    fEffArea->Branch( "eff_error", eff_error, "eff_error[nbins]/F" );
    fEffArea->Branch( "effNoTh2", effNoTh2, "effNoTh2[nbins]/D" ); // effective area vs MC energy without direction cuts
    fEffArea->Branch( "effNoTh2_error", effNoTh2_error, "effNoTh2_error[nbins]/F" );
    fEffArea->Branch( "seff_L", seff_L, "seff_L[nbins]/D" );
    fEffArea->Branch( "seff_U", seff_U, "seff_U[nbins]/D" );
    fEffArea->Branch( "Rec_nbins", &Rec_nbins, "Rec_nbins/I" );
    fEffArea->Branch( "Rec_e0", Rec_e0, "Rec_e0[Rec_nbins]/D" ); // log10( energy ) in [TeV]
    fEffArea->Branch( "Rec_eff", Rec_eff, "Rec_eff[Rec_nbins]/D" ); // effective area vs reconstructed energy (approximation)
    fEffArea->Branch( "Rec_eff_error", Rec_eff_error, "Rec_eff_error[Rec_nbins]/F" ); // effective area vs reconstructed energy (approximation, error)
    fEffArea->Branch( "Rec_effNoTh2", Rec_effNoTh2, "Rec_effNoTh2[Rec_nbins]/D" );  // effective area vs reconstructed energy (approximation)
    fEffArea->Branch( "Rec_effNoTh2_error", Rec_effNoTh2_error, "Rec_effNoTh2_error[Rec_nbins]/F" ); // effective area vs reconstructed energy (approximation, error)
    fEffArea->Branch( "esys_rel", esys_rel, "esys_rel[nbins]/F" );
    fEffArea->Branch( "Rec_seff_L", Rec_seff_L, "Rec_seff_L[Rec_nbins]/D" );
    fEffArea->Branch( "Rec_seff_U", Rec_seff_U, "Rec_seff_U[Rec_nbins]/D" );

    fEffArea->Branch( "Rec_angRes_p68", Rec_angRes_p68, "Rec_angRes_p68[Rec_nbins]/F" );
    fEffArea->Branch( "Rec_angRes_p80", Rec_angRes_p80, "Rec_angRes_p80[Rec_nbins]/F" );

    // For reconstructing the response matrices
    fEffArea->Branch( "nbins_ResMat", &nbins_ResMat, "nbins_ResMat/I" );
    fEffArea->Branch( "ResMat_MC", ResMat_MC, "ResMat_MC[nbins_ResMat]/D" );
    fEffArea->Branch( "ResMat_Rec", ResMat_Rec, "ResMat_Rec[nbins_ResMat]/D" );
    fEffArea->Branch( "ResMat_Rec_Err", ResMat_Rec_Err, "ResMat_Rec_Err[nbins_ResMat]/D" );
    fEffArea->Branch( hisTreeList, 64000, 1 );
    fEffArea->SetMarkerStyle( 20 );

    fAcceptance_AfterCuts_tree = new TTree( "Acceptance_AfterCuts", "Info to conctruct background map" );
    fAcceptance_AfterCuts_tree->Branch( "Xoff_aC", &fXoff_aC, "Xoff_aC/D" );
    fAcceptance_AfterCuts_tree->Branch( "Yoff_aC", &fYoff_aC, "Yoff_aC/D" );
    fAcceptance_AfterCuts_tree->Branch( "Xoff_derot_aC", &fXoff_derot_aC, "Xoff_derot_aC/D" );
    fAcceptance_AfterCuts_tree->Branch( "Yoff_derot_aC", &fYoff_derot_aC, "Yoff_derot_aC/D" );
    fAcceptance_AfterCuts_tree->Branch( "Erec", &fErec, "Erec/D" );
    fAcceptance_AfterCuts_tree->Branch( "EMC", &fEMC, "EMC/D" );
    fAcceptance_AfterCuts_tree->Branch( "CRweight", &fCRweight, "CRweight/D" );

    fsolid_angle_norm_done = false;
    fsolid_angle_norm = 1.;

}


/*

   set up vectors with histograms for azimuth and spectral index bins

*/
void VEffectiveAreaCalculator::initializeHistograms( vector< double > iAzMin, vector< double > iAzMax, vector< double > iSpectralIndex )
{
    fVMinAz = iAzMin;
    fVMaxAz = iAzMax;
    fVSpectralIndex = iSpectralIndex;

    char hname[400];

    vector< TH1D* > iT_TH1D;
    vector< TH2D* > iT_TH2D;
    vector< TH2F* > iT_TH2F;
    vector< TProfile* > iT_TProfile;

    for( unsigned int i = 0; i < fVSpectralIndex.size(); i++ )
    {
        iT_TH2D.clear();
        iT_TProfile.clear();

        // histograms for effective area calculation
        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEmc_%d_%d", i, j );
            if( hEmc )
            {
                iT_TH1D.push_back(( TH1D* )hEmc->Clone( hname ) );
            }
            else
            {
                iT_TH1D.push_back( 0 );
            }
        }
        hVEmc.push_back( iT_TH1D );

        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEcut_%d_%d", i, j );
            if( hEcut )
            {
                iT_TH1D.push_back(( TH1D* )hEcut->Clone( hname ) );
            }
            else
            {
                iT_TH1D.push_back( 0 );
            }
        }
        hVEcut.push_back( iT_TH1D );

        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEcutUW_%d_%d", i, j );
            if( hEcutUW )
            {
                iT_TH1D.push_back(( TH1D* )hEcutUW->Clone( hname ) );
            }
            else
            {
                iT_TH1D.push_back( 0 );
            }
        }
        hVEcutUW.push_back( iT_TH1D );

        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEcutLin_%d_%d", i, j );
            if( hEcutLin )
            {
                iT_TH1D.push_back(( TH1D* )hEcutLin->Clone( hname ) );
            }
            else
            {
                iT_TH1D.push_back( 0 );
            }
        }
        hVEcutLin.push_back( iT_TH1D );

        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEcut500_%d_%d", i, j );
            if( hEcut500 )
            {
                iT_TH1D.push_back(( TH1D* )hEcut500->Clone( hname ) );
            }
            else
            {
                iT_TH1D.push_back( 0 );
            }
        }
        hVEcut500.push_back( iT_TH1D );

        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEcutRec_%d_%d", i, j );
            if( hEcutRec )
            {
                iT_TH1D.push_back(( TH1D* )hEcutRec->Clone( hname ) );
            }
            else
            {
                iT_TH1D.push_back( 0 );
            }
        }
        hVEcutRec.push_back( iT_TH1D );

        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEcutRecUW_%d_%d", i, j );
            if( hEcutRecUW )
            {
                iT_TH1D.push_back(( TH1D* )hEcutRecUW->Clone( hname ) );
            }
            else
            {
                iT_TH1D.push_back( 0 );
            }
        }
        hVEcutRecUW.push_back( iT_TH1D );

        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEcutNoTh2_%d_%d", i, j );
            if( hEcutRecUW )
            {
                iT_TH1D.push_back(( TH1D* )hEcutNoTh2->Clone( hname ) );
            }
            else
            {
                iT_TH1D.push_back( 0 );
            }
        }
        hVEcutNoTh2.push_back( iT_TH1D );

        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEcutRecNoTh2_%d_%d", i, j );
            if( hEcutRecUW )
            {
                iT_TH1D.push_back(( TH1D* )hEcutRecNoTh2->Clone( hname ) );
            }
            else
            {
                iT_TH1D.push_back( 0 );
            }
        }
        hVEcutRecNoTh2.push_back( iT_TH1D );

        iT_TProfile.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEmcSWeight_%d_%d", i, j );
            if( hEmcSWeight )
            {
                iT_TProfile.push_back(( TProfile* )hEmcSWeight->Clone( hname ) );
            }
            else
            {
                iT_TProfile.push_back( 0 );
            }
        }
        hVEmcSWeight.push_back( iT_TProfile );

        // histograms for energy reconstruction quality
        iT_TProfile.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEsysRec_%d_%d", i, j );
            if( hEsysRec )
            {
                iT_TProfile.push_back(( TProfile* )hEsysRec->Clone( hname ) );
            }
            else
            {
                iT_TProfile.push_back( 0 );
            }
        }
        hVEsysRec.push_back( iT_TProfile );

        iT_TProfile.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEsysMC_%d_%d", i, j );
            if( hEsysMC )
            {
                iT_TProfile.push_back(( TProfile* )hEsysMC->Clone( hname ) );
            }
            else
            {
                iT_TProfile.push_back( 0 );
            }
        }
        hVEsysMC.push_back( iT_TProfile );

        iT_TProfile.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEsysMCRelative_%d_%d", i, j );
            if( hEsysMCRelative )
            {
                iT_TProfile.push_back(( TProfile* )hEsysMCRelative->Clone( hname ) );
            }
            else
            {
                iT_TProfile.push_back( 0 );
            }
        }
        hVEsysMCRelative.push_back( iT_TProfile );

        iT_TH2F.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEsysMCRelativeRMS_%d_%d", i, j );
            if( hEsysMCRelativeRMS )
            {
                iT_TH2F.push_back(( TH2F* )hEsysMCRelativeRMS->Clone( hname ) );
            }
            else
            {
                iT_TH2F.push_back( 0 );
            }
        }
        hVEsysMCRelativeRMS.push_back( iT_TH2F );


        iT_TH2F.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEsysMCRelative2D_%d_%d", i, j );
            if( hEsysMCRelative2D )
            {
                iT_TH2F.push_back(( TH2F* )hEsysMCRelative2D->Clone( hname ) );
            }
            else
            {
                iT_TH2F.push_back( 0 );
            }
        }
        hVEsysMCRelative2D.push_back( iT_TH2F );

        iT_TH2F.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEsysMCRelative2DNoDirectionCut_%d_%d", i, j );
            if( hEsysMCRelative2DNoDirectionCut )
            {
                iT_TH2F.push_back(( TH2F* )hEsysMCRelative2DNoDirectionCut->Clone( hname ) );
            }
            else
            {
                iT_TH2F.push_back( 0 );
            }
        }
        hVEsysMCRelative2DNoDirectionCut.push_back( iT_TH2F );

        iT_TH2F.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEsys2D_%d_%d", i, j );
            if( hEsys2D )
            {
                iT_TH2F.push_back(( TH2F* )hEsys2D->Clone( hname ) );
            }
            else
            {
                iT_TH2F.push_back( 0 );
            }
        }
        hVEsys2D.push_back( iT_TH2F );

        iT_TH2F.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEmcCutCTA_%d_%d", i, j );
            if( hEmcCutCTA )
            {
                iT_TH2F.push_back(( TH2F* )hEmcCutCTA->Clone( hname ) );
            }
            else
            {
                iT_TH2F.push_back( 0 );
            }
        }
        hVEmcCutCTA.push_back( iT_TH2F );



        iT_TH2F.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVResponseMatrix_%d_%d", i, j );
            if( hResponseMatrix )
            {
                iT_TH2F.push_back(( TH2F* )hResponseMatrix->Clone( hname ) );
            }
            else
            {
                iT_TH2F.push_back( 0 );
            }
        }
        hVResponseMatrix.push_back( iT_TH2F );

        iT_TH2F.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVResponseMatrixFine_%d_%d", i, j );
            if( hResponseMatrixFine )
            {
                iT_TH2F.push_back(( TH2F* )hResponseMatrixFine->Clone( hname ) );
            }
            else
            {
                iT_TH2F.push_back( 0 );
            }
        }
        hVResponseMatrixFine.push_back( iT_TH2F );

        iT_TH2F.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVResponseMatrixNoDirectionCut_%d_%d", i, j );
            if( hResponseMatrixNoDirectionCut )
            {
                iT_TH2F.push_back(( TH2F* )hResponseMatrixNoDirectionCut->Clone( hname ) );
            }
            else
            {
                iT_TH2F.push_back( 0 );
            }
        }
        hVResponseMatrixNoDirectionCut.push_back( iT_TH2F );

        iT_TH2F.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVResponseMatrixFineNoDirectionCut_%d_%d", i, j );
            if( hResponseMatrixFineNoDirectionCut )
            {
                iT_TH2F.push_back(( TH2F* )hResponseMatrixFineNoDirectionCut->Clone( hname ) );
            }
            else
            {
                iT_TH2F.push_back( 0 );
            }
        }
        hVResponseMatrixFineNoDirectionCut.push_back( iT_TH2F );

        iT_TProfile.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVResponseMatrixProfile_%d_%d", i, j );
            if( hResponseMatrixProfile )
            {
                iT_TProfile.push_back(( TProfile* )hResponseMatrixProfile->Clone( hname ) );
            }
            else
            {
                iT_TProfile.push_back( 0 );
            }
        }
        hVResponseMatrixProfile.push_back( iT_TProfile );

        iT_TH2F.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVResponseMatrixQC_%d_%d", i, j );
            if( hResponseMatrixQC )
            {
                iT_TH2F.push_back(( TH2F* )hResponseMatrixQC->Clone( hname ) );
            }
            else
            {
                iT_TH2F.push_back( 0 );
            }
        }
        hVResponseMatrixQC.push_back( iT_TH2F );

        iT_TH2F.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVResponseMatrixFineQC_%d_%d", i, j );
            if( hResponseMatrixFineQC )
            {
                iT_TH2F.push_back(( TH2F* )hResponseMatrixFineQC->Clone( hname ) );
            }
            else
            {
                iT_TH2F.push_back( 0 );
            }
        }
        hVResponseMatrixFineQC.push_back( iT_TH2F );

        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVWeightedRate_%d_%d", i, j );
            if( hWeightedRate )
            {
                iT_TH1D.push_back(( TH1D* )hWeightedRate->Clone( hname ) );
            }
            else
            {
                iT_TH1D.push_back( 0 );
            }
        }
        hVWeightedRate.push_back( iT_TH1D );
        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVWeightedRate005_%d_%d", i, j );
            if( hWeightedRate005 )
            {
                iT_TH1D.push_back(( TH1D* )hWeightedRate005->Clone( hname ) );
            }
            else
            {
                iT_TH1D.push_back( 0 );
            }
        }
        hVWeightedRate005.push_back( iT_TH1D );

        vector< vector< TH1D* > > i_temp;
        for( unsigned int e = 0; e < hEcutSub.size(); e++ )
        {
            sprintf( hname, "hV%s", hEcutSub[e]->GetName() );
            iT_TH1D.clear();
            for( unsigned int j = 0; j < fVMinAz.size(); j++ )
            {
                if( hEcutSub[e] )
                {
                    iT_TH1D.push_back(( TH1D* )hEcutSub[e]->Clone( hname ) );
                }
                else
                {
                    iT_TH1D.push_back( 0 );
                }
            }
            i_temp.push_back( iT_TH1D );
        }
        hVEcutSub.push_back( i_temp );
    }
}


/*!
 *
 *  CALLED TO USE (READ) EFFECTIVE AREAS
 *
 *  this constructor is called to GET the effective area from a file and calculate energy spectra
 *
 *  called from anasum
 *
 */
VEffectiveAreaCalculator::VEffectiveAreaCalculator( string iInputFile, double azmin, double azmax, double ipedvar,
        double iSpectralIndex, vector< double > iMCZe,
        int iSmoothIter, double iSmoothThreshold, int iEffectiveAreaVsEnergyMC, bool iLikelihoodAnalysis )
{
    reset();

    fRunPara = 0;

    // MC intervals
    fMCZe = iMCZe;

    // no effective area file present
    bNOFILE = false;
    // no input file available, return always 1 for all corrections
    if( iInputFile == "NOFILE" )
    {
        bNOFILE = true;
        return;
    }
    // effective area smoothing
    fSmoothIter = iSmoothIter;
    fSmoothThreshold = iSmoothThreshold;

    // no weighting
    fSpectralWeight = 0;

    // effective areas vs E_MC or E_rec
    fEffectiveAreaVsEnergyMC = iEffectiveAreaVsEnergyMC;

    // likelihood analysis true/false
    bLikelihoodAnalysis = iLikelihoodAnalysis;
    hMeanResponseMatrix = 0;
    hres_bins = 0;
    // mean effective area
    gMeanEffectiveArea = new TGraphAsymmErrors( 1 );
    gMeanEffectiveArea->SetName( "gMeanEffectiveArea" );
    gMeanEffectiveArea->SetMarkerStyle( 20 );
    gMeanEffectiveArea->SetMarkerColor( fEffectiveAreaVsEnergyMC + 1 );
    gMeanEffectiveArea->SetLineColor( fEffectiveAreaVsEnergyMC + 1 );


    gMeanEffectiveAreaMC = new TGraphAsymmErrors( 1 );
    gMeanEffectiveAreaMC->SetName( "gMeanEffectiveAreaMC" );
    gMeanEffectiveAreaMC->SetMarkerStyle( 20 );
    gMeanEffectiveAreaMC->SetMarkerColor( fEffectiveAreaVsEnergyMC + 2 );
    gMeanEffectiveAreaMC->SetLineColor( fEffectiveAreaVsEnergyMC + 2 );

    // mean effective area for Time BINS

    gTimeBinnedMeanEffectiveArea = new TGraph2DErrors( 1 );
    gTimeBinnedMeanEffectiveArea->SetName( "gTimeBinnedMeanEffectiveArea" );
    gTimeBinnedMeanEffectiveArea->SetMarkerStyle( 20 );
    gTimeBinnedMeanEffectiveArea->SetMarkerColor( fEffectiveAreaVsEnergyMC + 1 );
    gTimeBinnedMeanEffectiveArea->SetLineColor( fEffectiveAreaVsEnergyMC + 1 );

    for( int i = 0; i < gTimeBinnedMeanEffectiveArea->GetN(); i++ )
    {
        gTimeBinnedMeanEffectiveArea->SetPoint( i, 0., 0., 0. );
        gTimeBinnedMeanEffectiveArea->SetPointError( i, 0., 0., 0. );
    }
    fNTimeBinnedMeanEffectiveAreaMC = 0;


    // current directory
    fGDirectory = gDirectory;

    // test if input file with effective areas exists
    if( iInputFile.find( "IGNOREEFFECTIVEAREA" ) == string::npos )
    {
        iInputFile = VUtilities::testFileLocation( iInputFile, "EffectiveAreas", true );
        if( iInputFile.size() == 0 )
        {
            exit( EXIT_FAILURE );
        }
    }
    if( iInputFile.find( "IGNOREEFFECTIVEAREA" ) != string::npos )
    {
        cout << "ignoring effective areas: ";
        cout << "all energy spectra will be invalid" << endl;
        bNOFILE = true;
    }
    else
    {
        TFile fIn( iInputFile.c_str() );
        if( fIn.IsZombie() )
        {
            cout << "Error opening file with effective areas: " << iInputFile << endl;
            exit(-1 );
        }
        cout << "\t reading effective areas from " << fIn.GetName() << endl;

        // test which kind of file is available
        //    i) effective areas values for each energy bin (bEffectiveAreasareHistograms = true)
        //   ii) fit functions to effective area histograms (bEffectiveAreasareFunctions = true)
        //       (favorable, to avoid binning effects)
        //
        //  prefer fit function over histograms
        //
        bEffectiveAreasareFunctions = false;
        bEffectiveAreasareHistograms = false;
        if( getEffectiveAreasFromFitFunction(( TTree* )gDirectory->Get( "EffFit" ), azmin, azmax, iSpectralIndex ) )
        {
            bEffectiveAreasareFunctions = true;
        }
        else if( initializeEffectiveAreasFromHistograms(( TTree* )gDirectory->Get( "fEffArea" ),
                 ( TH1D* )gDirectory->Get( "hEmc" ),
                 azmin, azmax, iSpectralIndex, ipedvar,
                 ( TTree* )gDirectory->Get( "fEffAreaH2F" ) ) )
        {
            bEffectiveAreasareHistograms = true;
        }
        if(!bEffectiveAreasareHistograms && !bEffectiveAreasareFunctions )
        {
            cout << "VEffectiveAreaCalculator ERROR: no effective areas found" << endl;
            cout << "all energy spectra will be invalid" << endl;
            bNOFILE = true;
        }
        fIn.Close();
        if( fGDirectory )
        {
            fGDirectory->cd();
        }
    }
}


double VEffectiveAreaCalculator::getAzMean( double azmin, double azmax )
{
    // mean azimuth angle
    double iAzMean = 0.;
    if( azmin > 120. && azmax < -120. )
    {
        azmax += 360.;
    }
    else if( azmin < -150. && azmax > 120. )
    {
        azmin += 360.;
    }

    iAzMean = 0.5 * ( azmin + azmax );
    if( iAzMean > 180. )
    {
        iAzMean -= 360.;
    }

    return iAzMean;
}


/*
 *  CALLED TO READ/USE EFFECTIVE AREAS
 */
bool VEffectiveAreaCalculator::getEffectiveAreasFromFitFunction( TTree* iEffFit, double azmin, double azmax, double iSpectralIndex )
{
    if(!iEffFit )
    {
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////
    // this is currently not functional
    //
    cout << "EFFECTIVE AREA FROM FIT FUNCTON IS CURRENTLY NOT FUNCTIONAL. USE HISTOGRAMS." << endl;
    return false;
    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

    TF1* ifEff = 0;
    int iAMC;
    //
    // mean azimuth angle
    double iAzMean = getAzMean( azmin, azmax );

    double TazMin, TazMax, index;
    iEffFit->SetBranchAddress( "Ze", &ze );
    iEffFit->SetBranchAddress( "AzMin", &TazMin );
    iEffFit->SetBranchAddress( "AzMax", &TazMax );
    iEffFit->SetBranchAddress( "Index", &index );
    iEffFit->SetBranchAddress( "fEff", &ifEff );
    iEffFit->SetBranchAddress( "AMC", &iAMC );

    for( int i = 0; i < iEffFit->GetEntries(); i++ )
    {
        iEffFit->GetEntry( i );

        if( iAMC == 1 && fEffectiveAreaVsEnergyMC != 0 )
        {
            cout << "VEffectiveAreaCalculator::getEffectiveAreasFromFitFunction: expected effective area vs MC energy" << endl;
            return false;
        }
        if( iAMC == 0 && fEffectiveAreaVsEnergyMC != 1 )
        {
            cout << "VEffectiveAreaCalculator::getEffectiveAreasFromFitFunction: expected effective area vs reconstructed energy" << endl;
            return false;
        }
        if( fabs( iSpectralIndex - index ) > 0.01 )
        {
            continue;
        }

        if( fabs( TazMin ) > 5.e2 || fabs( TazMax ) > 5.e2 )
        {
            continue;
        }

        // test az bin
        // expect bin like [135,-135]
        if( TazMin > TazMax )
        {
            if( iAzMean < TazMin && iAzMean > TazMax )
            {
                continue;
            }
        }
        else
        {
            if( iAzMean < TazMin || iAzMean > TazMax )
            {
                continue;
            }
        }
        cout <<  ",(" << ze << ", " << TazMin << " - " << TazMax << ")";

        fZe.push_back( ze );
        if( ifEff )
        {
            fEffAreaFitFunction.push_back(( TF1* )ifEff->Clone() );
            fGDirectory->Append( fEffAreaFitFunction.back() );
        }
    }
    cout << endl;
    cout << "\t use fitted function for effective areas (number of zenith angle intervals: " << fZe.size() << ")" << endl;

    return true;
}

void VEffectiveAreaCalculator::multiplyByScatterArea( TGraphAsymmErrors* g )
{
    if(!g )
    {
        return;
    }
    double x = 0.;
    double y = 0.;
    for( int i = 0; i < g->GetN(); i++ )
    {
        g->GetPoint( i, x, y );
        g->SetPoint( i, x, y* fMC_ScatterArea );
        g->SetPointEYlow( i, g->GetErrorYlow( i ) * fMC_ScatterArea );
        g->SetPointEYhigh( i, g->GetErrorYhigh( i ) * fMC_ScatterArea );
    }
}

/*

   copy angular resolution values to tree variable

*/
void VEffectiveAreaCalculator::fillAngularResolution( unsigned int i_az, bool iContainment_95p )
{
    if( iContainment_95p && i_az < fGraph_AngularResolution68p.size() && fGraph_AngularResolution68p[i_az] )
    {
        // get first and last energy bin
        double i_emin = 1.e5;
        double i_emax = 1.e-5;
        double x = 0.;
        double y = 0.;
        for( int i = 0; i < fGraph_AngularResolution95p[i_az]->GetN(); i++ )
        {
            fGraph_AngularResolution95p[i_az]->GetPoint( i, x, y );
            if( x < i_emin )
            {
                i_emin = x;
            }
            if( x > i_emax )
            {
                i_emax = x;
            }
        }
        for( int i = 0; i < Rec_nbins; i++ )
        {
            if( Rec_e0[i] > i_emin && Rec_e0[i] < i_emax )
            {
                Rec_angRes_p80[i]  = fGraph_AngularResolution95p[i_az]->Eval( Rec_e0[i] );
            }
        }
    }
    else if(!iContainment_95p && i_az < fGraph_AngularResolution68p.size() && fGraph_AngularResolution68p[i_az] )
    {
        double i_emin = 0.;
        double i_emax = 0.;
        double x = 0.;
        double y = 0.;
        for( int i = 0; i < fGraph_AngularResolution68p[i_az]->GetN(); i++ )
        {
            fGraph_AngularResolution68p[i_az]->GetPoint( i, x, y );
            if( x < i_emin )
            {
                i_emin = x;
            }
            if( x > i_emax )
            {
                i_emax = x;
            }
        }
        fGraph_AngularResolution68p[i_az]->GetPoint( 0, i_emin, y );
        fGraph_AngularResolution68p[i_az]->GetPoint( fGraph_AngularResolution68p[i_az]->GetN(), i_emax, y );
        for( int i = 0; i < Rec_nbins; i++ )
        {
            if( Rec_e0[i] > i_emin && Rec_e0[i] < i_emax )
            {
                Rec_angRes_p68[i] = fGraph_AngularResolution68p[i_az]->Eval( Rec_e0[i] );
            }
        }
    }
}

void VEffectiveAreaCalculator::setAngularResolution2D( unsigned int i_az, vector< TH2D* > h )
{
    if( i_az < hVAngularDiff_2D.size() && h.size() == 4 )
    {
        hVAngularDiff_2D[i_az] = ( TH2F* ) h[0];
        hVAngularLogDiff_2D[i_az] = ( TH2F* ) h[1];
        hVAngularDiffEmc_2D[i_az] = ( TH2F* ) h[2];
        hVAngularLogDiffEmc_2D[i_az] = ( TH2F* ) h[3];
    }
}

void VEffectiveAreaCalculator::setAngularResolutionGraph( unsigned int i_az, TGraphErrors* g, bool iAngContainment_95p )
{
    if( i_az < fGraph_AngularResolution68p.size() && g )
    {
        if( iAngContainment_95p )
        {
            fGraph_AngularResolution95p[i_az] = g;
        }
        else
        {
            fGraph_AngularResolution68p[i_az] = g;
        }
    }
}

vector< double > VEffectiveAreaCalculator::interpolate_effectiveArea( double iV, double iVLower, double iVupper,
        vector< double > iElower, vector< double > iEupper, bool iCos )
{
    vector< double > i_temp;
    if( iElower.size() == iEupper.size() )
    {
        i_temp.assign( iElower.size(), 0. );
        for( unsigned int i = 0; i < iElower.size(); i++ )
        {
            i_temp[i] = VStatistics::interpolate( iElower[i], iVLower, iEupper[i], iVupper, iV, iCos, 0.5, -90. );
        }
        return i_temp;
    }

    return i_temp;
}



// Interpolating between two response matrices
TH2F* VEffectiveAreaCalculator::interpolate_responseMatrix( double iV, double iVlower, double iVupper,
        TH2F* iElower, TH2F* iEupper, bool iCos )
{
    if(!iElower || !iEupper )
    {
        return 0;
    }
    TH2F* hTemp = ( TH2F* )iElower->Clone();

    double tmpInt = 0;
    for( int i = 0 ; i < hTemp->GetXaxis()->GetNbins(); i++ )
    {
        for( int j = 0; j < hTemp->GetYaxis()->GetNbins(); j++ )
        {
            tmpInt = VStatistics::interpolate( iElower->GetBinContent( i, j ), iVlower, iEupper->GetBinContent( i, j ), iVupper, iV, iCos, 0.5, -90. );
            hTemp->SetBinContent( i, j, tmpInt );
        }
    }
    return hTemp;
}

/*
 * align energy axis of an IRF  array (e.g. effective area)
 *
 * this ensures that IRF vectors are always of same length
 *
 */
template <typename T> vector< T > VEffectiveAreaCalculator::get_irf_vector(
    int i_nbins,
    T* i_e0,
    T* i_irf )
{
    vector< T > i_temp_IRF( fNBins, 0. );

    for( unsigned int e = 0; e < fNBins; e++ )
    {
        for( int j = 0; j < i_nbins; j++ )
        {
            if( TMath::Abs( i_e0[j] - fEff_E0[e] ) < 1.e-5 )
            {
                i_temp_IRF[e] = i_irf[j];
            }
        }
    }
    return i_temp_IRF;
}

TH2F* VEffectiveAreaCalculator::get_irf2D_vector( int nbinsx, float x_min, float x_max,
        int nbinsy, float y_min, float y_max,
        float* value )
{
    if(!value || nbinsx == 0 || nbinsy == 0 )
    {
        return 0;
    }

    TH2F* h = new TH2F( "h2", "",
                        nbinsx, x_min, x_max,
                        nbinsy, y_min, y_max );
    for( int i = 0; i < nbinsx * nbinsy; i++ )
    {
        int nx = i % nbinsx;
        int ny = ( i - nx ) / nbinsx;
        h->SetBinContent( nx + 1, ny + 1, value[i] );
    }
    return h;
}

/*
 *
 *  CALLED TO USE EFFECTIVE AREAS
 *
 *
 */
bool VEffectiveAreaCalculator::initializeEffectiveAreasFromHistograms(
    TTree* iEffArea, TH1D* i_hEMC,
    double azmin, double azmax,
    double iSpectralIndex, double ipedvar,
    TTree* iEffAreaH2F )
{
    if(!iEffArea )
    {
        return false;
    }
    if(!i_hEMC )
    {
        cout << "----- Warning -----" << endl;
        cout << "  no MC histogram found to determine energy binning " << endl;
        cout << "   assume default binning: ";
        cout << "   " << fEnergyAxis_minimum_defaultValue << " " << fEnergyAxis_maximum_defaultValue;
        cout << " " << nbins;
        cout << endl;
        cout << "---- End of Warning ----" << endl;
        i_hEMC = new TH1D( "hEmc", "", nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue );
    }
    // mean azimuth angle
    double iAzMean = getAzMean( azmin, azmax );

    ////////////////////////////
    // define input tree
    double TazMin, TazMax, index;
    double Tpedvar = 1.;
    iEffArea->SetBranchAddress( "azMin", &TazMin );
    iEffArea->SetBranchAddress( "azMax", &TazMax );
    if( iEffArea->GetBranchStatus( "pedvar" ) )
    {
        iEffArea->SetBranchAddress( "pedvar", &Tpedvar );
    }
    else
    {
        Tpedvar = -1.;
    }
    iEffArea->SetBranchAddress( "index", &index );
    iEffArea->SetBranchAddress( "ze", &ze );
    iEffArea->SetBranchAddress( "Woff", &fWoff );
    iEffArea->SetBranchAddress( "Rec_nbins", &nbins );
    iEffArea->SetBranchAddress( "Rec_e0", e0 );
    iEffArea->SetBranchAddress( "Rec_eff", eff );
    /////////////////////////////////////////////////
    // reading of effective area tree with values
    // as function of true energy
    int fH2F_treecounter_offset = 0;
    UShort_t fH2F_nbins_esys = 0;
    float fH2F_esys_rel[1000];
    int fH2F_EsysMCRelative2D_nbinsx = 0;
    float fH2F_EsysMCRelative2D_minx = 0.;
    float fH2F_EsysMCRelative2D_maxx = 0.;
    int fH2F_EsysMCRelative2D_nbinsy = 0;
    float fH2F_EsysMCRelative2D_miny = 0.;
    float fH2F_EsysMCRelative2D_maxy = 0.;
    int fH2F_EsysMCRelative2D_binsxy = 0;
    float fH2F_EsysMCRelative2D_value[10000];
    if( iEffAreaH2F )
    {
        fH2F_treecounter_offset = iEffArea->GetEntries() / iEffAreaH2F->GetEntries();
        if( fH2F_treecounter_offset != 20 )
        {
            cout << "Warning in effective area reading: expected ratio of entries between";
            cout << " effective area trees to be 20" << endl;
        }
        iEffAreaH2F->SetBranchAddress( "nbins_esys", &fH2F_nbins_esys );
        iEffAreaH2F->SetBranchAddress( "e0_esys", &fH2F_e0_esys );
        iEffAreaH2F->SetBranchAddress( "esys_rel", &fH2F_esys_rel );

        // Binned likelihood analysis requires
        // MC effective areas and response matrix
        // Getting MC eff
        if( bLikelihoodAnalysis )
        {
            // MC energies and effective areas
            iEffAreaH2F->SetBranchAddress( "nbins", &nbins_MC );
            iEffAreaH2F->SetBranchAddress( "e0", e0_MC );
            iEffAreaH2F->SetBranchAddress( "eff", eff_MC );
            // Response Matrix
            iEffAreaH2F->SetBranchAddress( "hEsysMCRelative2D_binsx", &fH2F_EsysMCRelative2D_nbinsx );
            iEffAreaH2F->SetBranchAddress( "hEsysMCRelative2D_minx", &fH2F_EsysMCRelative2D_minx );
            iEffAreaH2F->SetBranchAddress( "hEsysMCRelative2D_maxx", &fH2F_EsysMCRelative2D_maxx );
            iEffAreaH2F->SetBranchAddress( "hEsysMCRelative2D_binsy", &fH2F_EsysMCRelative2D_nbinsy );
            iEffAreaH2F->SetBranchAddress( "hEsysMCRelative2D_miny", &fH2F_EsysMCRelative2D_miny );
            iEffAreaH2F->SetBranchAddress( "hEsysMCRelative2D_maxy", &fH2F_EsysMCRelative2D_maxy );
            iEffAreaH2F->SetBranchAddress( "hEsysMCRelative2D_binsxy", &fH2F_EsysMCRelative2D_binsxy );
            iEffAreaH2F->SetBranchAddress( "hEsysMCRelative2D_value", fH2F_EsysMCRelative2D_value );
        }
    }
    if( iEffArea->GetEntries() == 0 )
    {
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // prepare the energy vectors
    // (binning should be the same for all entries in the effective area tree)
    ////////////////////////////////////////////////////////////////////////////////////
    iEffArea->GetEntry( 0 );
    if(!i_hEMC )
    {
        cout << "VEffectiveAreaCalculator::initializeEffectiveAreasFromHistograms error: no effective area histogram found" << endl;
        return false;
    }
    fEff_E0.clear();
    fEff_E0.swap( fEff_E0 );
    for( int b = 1; b <= i_hEMC->GetNbinsX(); b++ )
    {
        fEff_E0.push_back( i_hEMC->GetBinCenter( b ) );
    }
    fNBins = fEff_E0.size();

    fVMeanEffectiveArea.assign( i_hEMC->GetNbinsX(), 0. );
    fVMeanEffectiveAreaMC.assign( i_hEMC->GetNbinsX(), 0. );
    fVTimeBinnedMeanEffectiveArea.assign( i_hEMC->GetNbinsX(), 0. );
    if( bLikelihoodAnalysis )
    {
        fVTimeBinnedMeanEffectiveAreaMC.assign( i_hEMC->GetNbinsX(), 0. );
    }

    cout << "\t selecting effective areas for mean az " << iAzMean << " deg, spectral index ";
    cout << iSpectralIndex << ", noise level " << ipedvar << endl;
    cout << "\t\ttotal number of curves: " << iEffArea->GetEntries();
    cout << ", total number of bins on energy axis: " << fNBins << endl;

    fNMeanEffectiveArea = 0;
    fNMeanEffectiveAreaMC = 0;
    if( gMeanEffectiveArea )
    {
        for( int i = 0; i < gMeanEffectiveArea->GetN(); i++ )
        {
            gMeanEffectiveArea->SetPoint( i, 0., 0. );
            gMeanEffectiveArea->SetPointError( i, 0., 0., 0., 0. );
        }
    }
    fNTimeBinnedMeanEffectiveArea = 0;
    if( gTimeBinnedMeanEffectiveArea )
    {
        for( int i = 0; i < gTimeBinnedMeanEffectiveArea->GetN(); i++ )
        {
            gTimeBinnedMeanEffectiveArea->SetPoint( i, 0., 0., 0. );
            gTimeBinnedMeanEffectiveArea->SetPointError( i, 0., 0., 0. );
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////
    // fill ze, woff, noise, index vectors
    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////
    bool i_ze_F = false;
    unsigned int i_index_ze = 0;
    bool i_woff_F = false;
    unsigned int i_index_woff = 0;
    bool i_noise_F = false;
    unsigned int i_index_noise = 0;
    bool i_index_F = false;
    unsigned int i_index_index = 0;

    int count_max_az_bins = 0;
    double iInvMax = 1.e5;
    int iIndexAz = 0;
    double iInvMean = 0.;
    ///////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////
    // loop over all entries in effective area tree
    // (not sure if this is really necessary, in the end a few entries are only needed)
    for( int i = 0; i < iEffArea->GetEntries(); i++ )
    {
        iEffArea->GetEntry( i );

        ///////////////////////////////////////////////////
        // check the azimuth range
        ///////////////////////////////////////////////////

        // expect a couple of az bins and then a last bin with the average azimuth bin; typically [-1000., 1000.]
        // this is the sign to read effective areas
        // iIndexAz should be selected according to the mean az of the run
        // (az is checked at the end of this loop
        // effective areas used are those from iIndexAz
        if( fabs( TazMin ) > 5.e2 || fabs( TazMax ) > 5.e2 )
        {
            // number of az bin required for access of iEffAreaH2F
            if( count_max_az_bins == 0 )
            {
                count_max_az_bins = i;
                cout << "\t\t number of az bin: " << count_max_az_bins << endl;
            }
            iInvMax = 1.e5;
            iEffArea->GetEntry( iIndexAz );

            ///////////////////////////////////////////////////
            // zenith angle
            ///////////////////////////////////////////////////
            i_ze_F = false;
            for( unsigned z = 0; z < fZe.size(); z++ )
            {
                // this has to be a relatively large value due to wobble offsets up to 2.0 deg
                if( fabs( fZe[z] - ze ) < 2.0 )
                {
                    i_index_ze = z;
                    i_ze_F = true;
                    break;
                }
            }
            if(!i_ze_F )
            {
                for( unsigned int w = 0; w < fMCZe.size(); w++ )
                {
                    if( fabs( fMCZe[w] - ze ) < 2.0 )
                    {
                        fZe.push_back( fMCZe[w] );
                        break;
                    }
                }
                i_index_ze = fZe.size() - 1;
            }
            ///////////////////////////////////////////////////
            // wobble offset
            ///////////////////////////////////////////////////
            if( i_index_ze < fEff_WobbleOffsets.size() )
            {
                i_woff_F = false;
                for( unsigned z = 0; z < fEff_WobbleOffsets[i_index_ze].size(); z++ )
                {
                    if( fabs( fEff_WobbleOffsets[i_index_ze][z] - fWoff ) < 0.05 )
                    {
                        i_index_woff = z;
                        i_woff_F = true;
                        break;
                    }
                }
                if(!i_woff_F )
                {
                    fEff_WobbleOffsets[i_index_ze].push_back( fWoff );
                    i_index_woff = fEff_WobbleOffsets[i_index_ze].size() - 1;
                }
            }
            else
            {
                vector< double > itemp;
                itemp.push_back( fWoff );
                fEff_WobbleOffsets.push_back( itemp );
                i_index_woff = fEff_WobbleOffsets[i_index_ze].size() - 1;
            }
            ///////////////////////////////////////////////////
            // noise level
            ///////////////////////////////////////////////////
            if( i_index_ze < fEff_Noise.size() )
            {
                if( i_index_woff < fEff_Noise[i_index_ze].size() )
                {
                    i_noise_F = false;
                    for( unsigned w = 0; w < fEff_Noise[i_index_ze][i_index_woff].size(); w++ )
                    {
                        if( fabs( fEff_Noise[i_index_ze][i_index_woff][w] - Tpedvar ) < 0.005 )
                        {
                            i_index_noise = w;
                            i_noise_F = true;
                            break;
                        }
                    }
                    if(!i_noise_F )
                    {
                        fEff_Noise[i_index_ze][i_index_woff].push_back( Tpedvar );
                        i_index_noise = fEff_Noise[i_index_ze][i_index_woff].size() - 1;
                    }
                }
                else
                {
                    vector< double > itemp;
                    itemp.push_back( Tpedvar );
                    fEff_Noise[i_index_ze].push_back( itemp );
                    i_index_noise = fEff_Noise[i_index_ze][i_index_woff].size() - 1;
                }
            }
            else
            {
                vector< double > itemp;
                itemp.push_back( Tpedvar );
                vector< vector< double > > iitemp;
                iitemp.push_back( itemp );
                fEff_Noise.push_back( iitemp );
                i_index_noise = fEff_Noise[i_index_ze][i_index_woff].size() - 1;
            }
            ///////////////////////////////////////////////////
            // spectral index
            ///////////////////////////////////////////////////
            if( i_index_ze < fEff_SpectralIndex.size() )
            {
                if( i_index_woff < fEff_SpectralIndex[i_index_ze].size() )
                {
                    if( i_index_noise < fEff_SpectralIndex[i_index_ze][i_index_woff].size() )
                    {
                        i_index_F = false;
                        for( unsigned s = 0; s < fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise].size(); s++ )
                        {
                            if( fabs( fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise][s] - index ) < 0.005 )
                            {
                                i_index_index = s;
                                i_index_F = true;
                                break;
                            }
                        }
                        if(!i_index_F )
                        {
                            fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise].push_back( index );
                            i_index_index = fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise].size() - 1;
                        }
                    }
                    else
                    {
                        vector< double > itemp;
                        itemp.push_back( index );
                        fEff_SpectralIndex[i_index_ze][i_index_woff].push_back( itemp );
                        i_index_index = fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise].size() - 1;
                    }
                }
                else
                {
                    vector< double > itemp;
                    itemp.push_back( index );
                    vector< vector< double > > iitemp;
                    iitemp.push_back( itemp );
                    fEff_SpectralIndex[i_index_ze].push_back( iitemp );
                    i_index_index = fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise].size() - 1;
                }
            }
            else
            {
                vector< double > itemp;
                itemp.push_back( index );
                vector< vector< double > > iitemp;
                iitemp.push_back( itemp );
                vector< vector< vector< double > > > iiitemp;
                iiitemp.push_back( iitemp );
                fEff_SpectralIndex.push_back( iiitemp );
                i_index_index = fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise].size() - 1;
            }
            unsigned int i_ID = i_index_index + 100 * ( i_index_noise + 100 * ( i_index_woff + 100 * i_index_ze ) );
            ///////////////////////////////////////////////////
            // read effective area and load them into maps
            ///////////////////////////////////////////////////
            //
            fEffArea_map[i_ID] = get_irf_vector<double>( nbins, e0, eff );

            // read 2D histograms
            if( iEffAreaH2F && count_max_az_bins > 0 )
            {
                iEffAreaH2F->GetEntry(
                    count_max_az_bins
                    * ( iIndexAz / ( fH2F_treecounter_offset* count_max_az_bins ) )
                    + iIndexAz % count_max_az_bins );
            }

            fEff_EsysMCRelative[i_ID].resize( fH2F_nbins_esys, 0. );
            for( unsigned int it = 0; it < fH2F_nbins_esys; it++ )
            {
                fEff_EsysMCRelative[i_ID][it] = fH2F_esys_rel[it];
            }

            if( bLikelihoodAnalysis )
            {
                // Getting MC effective areas
                vector< float > v_mc = get_irf_vector<float >( nbins_MC,
                                       e0_MC,
                                       eff_MC );
                fEffAreaMC_map[i_ID].resize( v_mc.size(), 0. );
                for( unsigned int it = 0; it < v_mc.size(); it++ )
                {
                    fEffAreaMC_map[i_ID][it] = v_mc[it];
                }
                TH2F* i_hEsysMCRelative2D = get_irf2D_vector( fH2F_EsysMCRelative2D_nbinsx,
                                            fH2F_EsysMCRelative2D_minx,
                                            fH2F_EsysMCRelative2D_maxx,
                                            fH2F_EsysMCRelative2D_nbinsy,
                                            fH2F_EsysMCRelative2D_miny,
                                            fH2F_EsysMCRelative2D_maxy,
                                            fH2F_EsysMCRelative2D_value );
                if( i_hEsysMCRelative2D )
                {
                    fEsysMCRelative2D_map[i_ID] = ( TH2F* )i_hEsysMCRelative2D->Clone();
                    i_hEsysMCRelative2D->SetDirectory( 0 );
                    i_hEsysMCRelative2D->AddDirectory( kFALSE );
                }
                else
                {
                    fEsysMCRelative2D_map[i_ID] = 0;
                }
            }
            // this is needed only if there are no azimuth dependent effective areas
            iIndexAz++;

            continue;
        }
        ///////////////////////////////////////////////////
        // test az bin
        ///////////////////////////////////////////////////
        // expect bin like [135,-135]
        iInvMean = getAzMean( TazMin, TazMax );
        if( TazMax < 0. && TazMin > 0. )
        {
            double iT =  fabs( iAzMean - iInvMean );
            if( iT > 180. )
            {
                iT = fabs( iT - 360. );
            }
            if( iT < iInvMax )
            {
                iInvMax = iT;
                iIndexAz = i;
            }
            if( iAzMean < 0. && iAzMean > TazMax )
            {
                continue;
            }
            else if( iAzMean > 0. && iAzMean < TazMin )
            {
                continue;
            }
        }
        else
        {
            if( fabs( iAzMean - iInvMean ) < iInvMax )
            {
                iInvMax = fabs( iAzMean - iInvMean );
                iIndexAz = i;
            }
        }
    }
    ///////////////////////////////////////////////////
    if( fZe.size() == 0 )
    {
        cout << "Error: no effective areas found in effective area tree" << endl;
        exit(-1 );
    }
    cout << "\t use histograms for effective areas (number of zenith angle intervals: " << fZe.size() << "; ";
    for( unsigned int i = 0; i < fZe.size() - 1; i++ )
    {
        cout << fZe[i] << ", ";
    }
    if( fZe.size() > 0 )
    {
        cout << fZe[fZe.size() - 1];
    }
    cout << ")" << endl;
    cout << "\t (effective area vs reconstructed energy)" << endl;
    if( fSmoothIter > 0 )
    {
        smoothEffectiveAreas( fEffArea_map );
    }
    ///////////////////////////////////////////////////
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

VEffectiveAreaCalculator::~VEffectiveAreaCalculator()
{
    for( unsigned int i = 0; i < fEffAreaFitFunction.size(); i++ )
    {
        if( fEffAreaFitFunction[i] )
        {
            fEffAreaFitFunction[i]->Delete();
        }
    }
    if( gMeanEffectiveArea )
    {
        delete gMeanEffectiveArea;
    }
    if( gTimeBinnedMeanEffectiveArea )
    {
        delete gTimeBinnedMeanEffectiveArea;
    }
    if( gMeanSystematicErrorGraph )
    {
        delete gMeanSystematicErrorGraph;
    }
    if( gMeanEffectiveAreaMC )
    {
        delete gMeanEffectiveAreaMC;
    }
    if( hMeanResponseMatrix )
    {
        delete hMeanResponseMatrix;
    }
}


void VEffectiveAreaCalculator::reset()
{
    fNBins = 0;
    gMeanEffectiveArea = 0;
    gTimeBinnedMeanEffectiveArea = 0;

    gMeanEffectiveAreaMC = 0;
    hMeanResponseMatrix = 0;
    hres_bins = 0;
    fMC_ScatterArea = 0.;

    bNOFILE = true;
    fGDirectory = 0;

    fSpectralIndex = 2.0;

    // Important: changing this means probably that the values used in
    // VEffectiveAreaCalculatorMCHistograms have to be changed as well
    fEnergyAxis_minimum_defaultValue = -2.0;
    fEnergyAxis_maximum_defaultValue = 4.0;

    fCuts = 0;

    fTNoise = 0;
    fTNoisePE = 0.;
    fTPedvar = 0.;

    fAzBin = 0;
    fMinAz = -1.e3;
    fMaxAz = 1.e3;
    fEffectiveAreaVsEnergyMC = 1;

    hEmc = 0;
    hEcut = 0;
    hEcutUW = 0;
    hEcutLin = 0;
    hEcutRec = 0;
    hEcutRecUW = 0;
    hEcut500 = 0;
    hEcutNoTh2 = 0;
    hEcutRecNoTh2 = 0;
    gEffAreaMC = 0;
    gEffAreaRec = 0;
    gEffAreaNoTh2MC = 0;
    gEffAreaNoTh2Rec = 0;
    hEmcSWeight = 0;
    hEsysRec = 0;
    hEsysMC = 0;
    hEsysMCRelative = 0;
    hEsysMCRelativeRMS = 0;
    hEsysMCRelative2D = 0;
    hEsysMCRelative2DNoDirectionCut = 0;
    hEsys2D = 0;
    hResponseMatrix = 0;
    hResponseMatrixProfile = 0;
    hResponseMatrixQC = 0;
    hEmcCutCTA = 0;
    hResponseMatrixNoDirectionCut = 0;
    hResponseMatrixFine = 0;
    hResponseMatrixFineQC = 0;
    hResponseMatrixFineNoDirectionCut = 0;
    hAngularDiff_2D = 0;
    hAngularDiffEmc_2D = 0;
    hAngularLogDiff_2D = 0;
    hAngularLogDiffEmc_2D = 0;
    fEffArea = 0;
    hisTreeList = 0;
    hWeightedRate = 0;
    hWeightedRate005 = 0;
    bEffectiveAreasareFunctions = false;
    bEffectiveAreasareHistograms = false;

    setStatisticsOption();

    fEffArea = 0;
    ze = 0.;
    nbins = 60;
    nbins_MC = 60;
    Rec_nbins = 0;
    for( int i = 0; i < 1000; i++ )
    {
        e0[i] = 0.;
        e0_MC[i] = 0.;
        eff[i] = 0.;
        eff_error[i] = 0.;
        effNoTh2[i] = 0.;
        effNoTh2_error[i] = 0.;
        esys_rel[i] = 0.;
        seff_L[i] = 0.;
        seff_U[i] = 0.;
        Rec_e0[i] = 0.;
        Rec_eff[i] = 0.;
        Rec_eff_error[i] = 0.;
        Rec_effNoTh2[i] = 0.;
        Rec_effNoTh2_error[i] = 0.;
        Rec_seff_L[i] = 0.;
        Rec_seff_U[i] = 0.;
        ResMat_MC[i] = 0.;
        ResMat_Rec[i] = 0.;
        ResMat_Rec_Err[i] = 0.;
        Rec_angRes_p68[i] = 0.;
        Rec_angRes_p80[i] = 0.;

    }

    fEffectiveAreas_meanZe = 0.;
    fEffectiveAreas_meanWoff = 0.;
    fEffectiveAreas_meanPedVar = 0.;
    fEffectiveAreas_meanIndex = 0.;
    fEffectiveAreas_meanN = 0.;

    gMeanSystematicErrorGraph = 0;

    fXoff_aC = -99;
    fYoff_aC = -99;
    fXoff_derot_aC = -99;
    fYoff_derot_aC = -99;
    fErec = -99;
    fEMC = -99;
    fCRweight = -99;


}

double VEffectiveAreaCalculator::getMCSolidAngleNormalization()
{
    double iSolAngleNorm = 1.;
    if( fCuts && fRunPara )
    {
        if( fRunPara->fViewcone_max > 0. && fCuts->fCut_CameraFiducialSize_MC_max > 0. && fCuts->fCut_CameraFiducialSize_MC_max < 1000.
                && fCuts->fCut_CameraFiducialSize_MC_max < fRunPara->fViewcone_max )
        {
            // solid angle of simulated showers
            double iSN_mc = ( 1. - cos( fRunPara->fViewcone_max* TMath::DegToRad() ) );
            if( fRunPara->fViewcone_min > 0. )
            {
                iSN_mc -= ( 1. - cos( fRunPara->fViewcone_min* TMath::DegToRad() ) );
            }
            // solid angle of angular bin
            double iSN_cu = ( 1. - cos( fCuts->fCut_CameraFiducialSize_MC_max* TMath::DegToRad() ) );
            if( fCuts->fCut_CameraFiducialSize_MC_min > 0. )
            {
                iSN_cu -= ( 1. - cos( fCuts->fCut_CameraFiducialSize_MC_min* TMath::DegToRad() ) );
            }

            if( iSN_mc > 0. )
            {
                iSolAngleNorm = iSN_cu / iSN_mc;
            }
        }
    }

    return iSolAngleNorm;
}

/*

   read spectra of MC events from mscw file (filled in eventdisplay)

*/
bool VEffectiveAreaCalculator::getMonteCarloSpectra( VEffectiveAreaCalculatorMCHistograms* iMC_histo )
{
    // get solid angle normalization
    double iSolAngleNorm = getMCSolidAngleNormalization();
    cout << "VEffectiveAreaCalculator::getMonteCarloSpectra: solid angle normalization factor: " << iSolAngleNorm << endl;

    char hname[800];
    // loop over az bins
    // (MC az [-180., 180.])
    for( unsigned int i_az = 0; i_az < fVMinAz.size(); i_az++ )
    {
        // loop over all spectral index
        for( unsigned int s = 0; s < fVSpectralIndex.size(); s++ )
        {
            if( s < hVEmc.size() && i_az < hVEmc[s].size() )
            {
                sprintf( hname, "hVEmc_%d_%d", s, i_az );
                if( iMC_histo->getHistogram_Emc( i_az, s ) )
                {
                    hVEmc[s][i_az] = ( TH1D* )iMC_histo->getHistogram_Emc( i_az, s )->Clone( hname );
                    if( hVEmc[s][i_az] )
                    {
                        hVEmc[s][i_az]->Scale( iSolAngleNorm );
                    }
                    if( hVEmc[s][i_az] && fRunPara && fRunPara->fIgnoreFractionOfEvents > 0. )
                    {
                        hVEmc[s][i_az]->Scale(( 1.0 - fRunPara->fIgnoreFractionOfEvents ) );
                    }
                }
                else
                {
                    hVEmc[s][i_az] = 0;
                }
            }
            if( s < hVEmcSWeight.size() && i_az < hVEmcSWeight[s].size() )
            {
                sprintf( hname, "hVEmcSWeight_%d_%d", s, i_az );
                if( iMC_histo->getHistogram_EmcWeight( i_az, s ) )
                {
                    hVEmcSWeight[s][i_az] = ( TProfile* )iMC_histo->getHistogram_EmcWeight( i_az, s )->Clone( hname );
                    if( hVEmcSWeight[s][i_az] )
                    {
                        hVEmcSWeight[s][i_az]->Scale( iSolAngleNorm );
                    }
                    if( hVEmcSWeight[s][i_az] && fRunPara && fRunPara->fIgnoreFractionOfEvents > 0. )
                    {
                        hVEmcSWeight[s][i_az]->Scale(( 1.0 - fRunPara->fIgnoreFractionOfEvents ) );
                    }
                }
                else
                {
                    hVEmcSWeight[s][i_az] = 0;
                }
            }
        }
    }

    return true;
}

/*
 *
 *  CALLED FOR CALCULATION OF EFFECTIVE AREAS
 *
 */
bool VEffectiveAreaCalculator::fill( TH1D* hE0mc, CData* d,
                                     VEffectiveAreaCalculatorMCHistograms* iMC_histo, unsigned int iMethod )
{
    bool bDebugCuts = false;          // lots of debug output

    // make sure that vectors are initialized
    unsigned int ize = 0;      // should always be zero
    if( ize >= fZe.size() )
    {
        cout << "VEffectiveAreaCalculator::fill error: vectors are not correctly initialized " << fZe.size() << "\t" << ize << endl;
        return false;
    }
    resetHistogramsVectors( ize );

    // do not require successful energy reconstruction
    if( fIgnoreEnergyReconstruction )
    {
        iMethod = 100;
    }

    //////////////////////////////////////////////////////////////////
    // total Monte Carlo core scatter area (depends on CORSIKA shower core scatter mode)
    fMC_ScatterArea = 0.;
    if( fScatterMode[ize] == "VIEWCONE" )
    {
        fMC_ScatterArea = fAreaRadius[ize] * fAreaRadius[ize] * TMath::Pi();
    }
    else if( fScatterMode[ize] == "FLAT" )
    {
        fMC_ScatterArea = fAreaRadius[ize] * fAreaRadius[ize] * TMath::Pi() * cos( fZe[ize] * TMath::DegToRad() );
    }
    else
    {
        cout << "VEffectiveAreaCalculator::fill ERROR: unknown CORSIKA scatter mode: " << fScatterMode[ize] << endl;
        return false;
    }
    // reset unique event counter
    fUniqueEventCounter.clear();
    Long64_t iSuccessfullEventStatistics = 0;

    //////////////////////////////////////////////////////////////////
    // print some run information
    cout << endl;
    cout << "calculating effective areas: " << endl;
    cout << "\t zenith angle: " << fZe[ize];
    cout << ", wobble offset (x,y): " << fXWobble[ize] << ", " << fYWobble[ize];
    cout << " (" << sqrt( fXWobble[ize]*fXWobble[ize] + fYWobble[ize]*fYWobble[ize] ) << " deg)" << endl;
    cout << "\t noise level: " << fNoise[ize] << " (pedvar: " << fPedVar[ize] << ")" << endl;
    cout << "\t area (" << fScatterMode[ize] << ") [m^2]: " << fMC_ScatterArea;
    cout << " (scatter radius " << fAreaRadius[ize] << " [m])" << endl;
    cout << "\t energy reconstruction method: " << iMethod << endl;
    if( fIsotropicArrivalDirections )
    {
        cout << "\t assuming isotropic arrival directions" << endl;
    }
    if( fRunPara && fRunPara->fIgnoreFractionOfEvents > 0. )
    {
        cout << "\t ignore first " << fRunPara->fIgnoreFractionOfEvents * 100. << " % of events" << endl;
    }

    cout << endl;
    if( fSpectralWeight )
    {
        fSpectralWeight->print();
    }
    else
    {
        cout << "(no specral weight given)" << endl;
    }
    cout << endl;

    // make sure that all data pointers exist
    if(!d || !iMC_histo )
    {
        cout << "VEffectiveAreaCalculator::fill error: no data tree or MC histograms: " << endl;
        cout << d << "\t" << iMC_histo << endl;
        return false;
    }

    // spectral weight
    double i_weight = 1.;
    // reconstructed energy (TeV, log10)
    double eRec = 0.;
    double eRecLin = 0.;
    // MC energy (TeV, log10)
    double eMC = 0.;

    ////////////////////////////////////////////////////////////////////////////
    // get MC histograms
    if(!getMonteCarloSpectra( iMC_histo ) )
    {
        cout << "VEffectiveAreaCalculator::fill error while getting MC spectra" << endl;
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    // reset cut statistics
    fCuts->resetCutStatistics();

    ///////////////////////////////////////////////////////
    // get full data set and loop over all entries
    ///////////////////////////////////////////////////////
    Long64_t d_nentries = d->fChain->GetEntries();
    Long64_t i_start = 0;
    if( fRunPara && fRunPara->fIgnoreFractionOfEvents > 0. )
    {
        i_start = ( Long64_t )( fRunPara->fIgnoreFractionOfEvents* d_nentries );
    }
    cout << "\t total number of data events: " << d_nentries << " (start at event " << i_start << ")" << endl;

    //--- for the CR normalisation filling Acceptance tree total number of simulated is needed
    //-- WARNING if the rule for the azimuth bin changes in VInstrumentResponseFunctionRunParameter the following line must be adapted!!!!
    unsigned int number_of_az_bin = fRunPara->fAzMin.size();
    int az_bin_index = 0;// if no azimuth bin, all events are in bin 0. if azimuth bin, all event are in the last bin.
    if( number_of_az_bin > 0 )
    {
        az_bin_index = ( int ) number_of_az_bin - 1;
    }

    for( Long64_t i = i_start; i < d_nentries; i++ )
    {
        d->GetEntry( i );

        // update cut statistics
        fCuts->newEvent();

        if( bDebugCuts )
        {
            cout << "============================== " << endl;
            cout << "EVENT entry number " << i << endl;
        }

        // apply MC cuts
        if( bDebugCuts )
        {
            cout << "#0 CUT MC " << fCuts->applyMCXYoffCut( d->MCxoff, d->MCyoff, false ) << endl;
        }

        if(!fCuts->applyMCXYoffCut( d->MCxoff, d->MCyoff, true ) )
        {
            continue;
        }

        // log of MC energy
        eMC = log10( d->MCe0 );

        // fill trigger cuts
        hEcutSub[0]->Fill( eMC, 1. );

        ////////////////////////////////
        // apply general quality and gamma/hadron separation cuts

        // apply reconstruction cuts
        if( bDebugCuts )
        {
            cout << "#1 CUT applyInsideFiducialAreaCut ";
            cout << fCuts->applyInsideFiducialAreaCut();
            cout << "\t" << fCuts->applyStereoQualityCuts( iMethod, false, i, true ) << endl;
        }

        // apply fiducial area cuts
        if(!fCuts->applyInsideFiducialAreaCut( true ) )
        {
            continue;
        }
        hEcutSub[1]->Fill( eMC, 1. );

        // apply reconstruction quality cuts
        if(!fCuts->applyStereoQualityCuts( iMethod, true, i, true ) )
        {
            continue;
        }
        hEcutSub[2]->Fill( eMC, 1. );

        // apply telescope type cut (e.g. for CTA simulations)
        if( fTelescopeTypeCutsSet )
        {
            if( bDebugCuts )
            {
                cout << "#2 Cut NTELType " << fCuts->applyTelTypeTest( false ) << endl;
            }
            if(!fCuts->applyTelTypeTest( true ) )
            {
                continue;
            }
        }
        hEcutSub[3]->Fill( eMC, 1. );


        //////////////////////////////////////
        // apply direction cut
        //
        // point source cut; use MC shower direction as reference direction
        bool bDirectionCut = false;
        if(!fIsotropicArrivalDirections )
        {
            if(!fCuts->applyDirectionCuts( iMethod, true ) )
            {
                bDirectionCut = true;
            }
        }
        // background cut; use (0,0) as reference direction
        // (command line option -d)
        else
        {
            if(!fCuts->applyDirectionCuts( iMethod, true, 0., 0. ) )
            {
                bDirectionCut = true;
            }
        }


        if(!bDirectionCut )
        {
            hEcutSub[4]->Fill( eMC, 1. );
        }

        //////////////////////////////////////
        // apply energy reconstruction quality cut
        if(!fIgnoreEnergyReconstruction )
        {
            if( bDebugCuts )
            {
                cout << "#4 EnergyReconstructionQualityCuts " << fCuts->applyEnergyReconstructionQualityCuts( iMethod ) << endl;
            }
            if(!fCuts->applyEnergyReconstructionQualityCuts( iMethod, true ) )
            {
                continue;
            }
        }

        if(!bDirectionCut )
        {
            hEcutSub[5]->Fill( eMC, 1. );
        }

        // skip event if no energy has been reconstructed
        // get energy according to which reconstruction method
        if( iMethod == 0 && d->Erec > 0. )
        {
            eRec = log10( d->Erec );
            eRecLin = d->Erec;
        }
        else if( iMethod == 1 && d->ErecS > 0. )
        {
            eRec = log10( d->ErecS );
            eRecLin = d->ErecS;
        }
        else if( fIgnoreEnergyReconstruction )
        {
            eRec = log10( d->MCe0 );
            eRecLin = d->MCe0;
        }
        else
        {
            continue;
        }

        ///////////////////////////////////////////
        // fill response matrix after quality cuts

        if(!bDirectionCut )
        {
            // loop over all az bins
            for( unsigned int i_az = 0; i_az < fVMinAz.size(); i_az++ )
            {
                // check at what azimuth bin we are
                if( fZe[ize] > 3. )
                {
                    // confine MC az to -180., 180.
                    if( d->MCaz > 180. )
                    {
                        d->MCaz -= 360.;
                    }
                    // expect bin like [135,-135]
                    if( fVMinAz[i_az] > fVMaxAz[i_az] )
                    {
                        if( d->MCaz < fVMinAz[i_az] && d->MCaz > fVMaxAz[i_az] )
                        {
                            continue;
                        }
                    }
                    // expect bin like [-135,-45.]
                    else
                    {
                        if( d->MCaz < fVMinAz[i_az] || d->MCaz > fVMaxAz[i_az] )
                        {
                            continue;
                        }
                    }
                }
                // loop over all spectral index
                for( unsigned int s = 0; s < fVSpectralIndex.size(); s++ )
                {
                    if( hVResponseMatrixQC[s][i_az] )
                    {
                        hVResponseMatrixQC[s][i_az]->Fill( eRec, eMC );
                    }
                    if( hVResponseMatrixFineQC[s][i_az] )
                    {
                        hVResponseMatrixFineQC[s][i_az]->Fill( eRec, eMC );
                    }
                }
            }
        }

        //////////////////////////////////////
        // apply gamma hadron cuts
        if( bDebugCuts )
        {
            cout << "#3 CUT ISGAMMA " << fCuts->isGamma( i ) << endl;
        }
        if(!fCuts->isGamma( i, true ) )
        {
            continue;
        }
        if(!bDirectionCut )
        {
            hEcutSub[6]->Fill( eMC, 1. );
        }

        // unique event counter
        if(!bDirectionCut )
        {
            iSuccessfullEventStatistics++;
        }

        // loop over all az bins
        for( unsigned int i_az = 0; i_az < fVMinAz.size(); i_az++ )
        {
            // check at what azimuth bin we are
            if( fZe[ize] > 3. )
            {
                // confine MC az to -180., 180.
                if( d->MCaz > 180. )
                {
                    d->MCaz -= 360.;
                }
                // expect bin like [135,-135]
                if( fVMinAz[i_az] > fVMaxAz[i_az] )
                {
                    if( d->MCaz < fVMinAz[i_az] && d->MCaz > fVMaxAz[i_az] )
                    {
                        continue;
                    }
                }
                // expect bin like [-135,-45.]
                else
                {
                    if( d->MCaz < fVMinAz[i_az] || d->MCaz > fVMaxAz[i_az] )
                    {
                        continue;
                    }
                }
            }

            //fill tree with acceptance information after cuts (needed to construct background model in ctools)
            if(!bDirectionCut && fRunPara->fgetXoff_Yoff_afterCut )
            {
                fXoff_aC = d->Xoff;
                fYoff_aC = d->Yoff;
                fXoff_derot_aC = d->Xoff_derot;
                fYoff_derot_aC = d->Yoff_derot;
                fErec = eRecLin;
                fEMC  = d->MCe0;
                fCRweight = getCRWeight( d->MCe0, hVEmc[0][az_bin_index], true );  //So that the acceptance can be normalised to the CR spectrum.
                // when running on gamma, this should return 1.
                fAcceptance_AfterCuts_tree->Fill();
            }


            // loop over all spectral index
            for( unsigned int s = 0; s < fVSpectralIndex.size(); s++ )
            {
                // weight by spectral index
                if( fSpectralWeight )
                {
                    fSpectralWeight->setSpectralIndex( fVSpectralIndex[s] );
                    i_weight = fSpectralWeight->getSpectralWeight( d->MCe0 );
                }
                else
                {
                    i_weight = 0.;
                }

                ////////////////////////////////////////////
                // fill effective areas before direction cut
                if( hVEcutNoTh2[s][i_az] )
                {
                    hVEcutNoTh2[s][i_az]->Fill( eMC, i_weight );
                }
                if( hVEcutRecNoTh2[s][i_az] )
                {
                    hVEcutRecNoTh2[s][i_az]->Fill( eRec, i_weight );
                }
                // fill response matrix (migration matrix) before
                // direction cut
                if( hVResponseMatrixNoDirectionCut[s][i_az] )
                {
                    hVResponseMatrixNoDirectionCut[s][i_az]->Fill( eRec, eMC, i_weight );
                }
                if( hVResponseMatrixFineNoDirectionCut[s][i_az] )
                {
                    hVResponseMatrixFineNoDirectionCut[s][i_az]->Fill( eRec, eMC, i_weight );
                }
                if( hVEsysMCRelative2DNoDirectionCut[s][i_az] )
                {
                    hVEsysMCRelative2DNoDirectionCut[s][i_az]->Fill( eMC, eRecLin / d->MCe0, i_weight );
                }

                /////////////////////////
                // apply direction cut
                if( bDirectionCut )
                {
                    continue;
                }

                /////////////////////////////////////////////
                // after gamma/hadron and after direction cut

                // fill true MC energy (hVEmc is in true MC energies)
                if( hVEcut[s][i_az] )
                {
                    hVEcut[s][i_az]->Fill( eMC, i_weight );
                }
                if( hVEcutUW[s][i_az] )
                {
                    hVEcutUW[s][i_az]->Fill( eMC, 1. );
                }
                if( hVEcut500[s][i_az] )
                {
                    hVEcut500[s][i_az]->Fill( eMC, i_weight );
                }
                if( hVEcutLin[s][i_az] )
                {
                    hVEcutLin[s][i_az]->Fill( eMC, i_weight );
                }
                if( hVEcutRec[s][i_az] )
                {
                    hVEcutRec[s][i_az]->Fill( eRec, i_weight );
                }
                if( hVEcutRecUW[s][i_az] )
                {
                    hVEcutRecUW[s][i_az]->Fill( eRec, 1. );
                }
                if( hVEsysRec[s][i_az] )
                {
                    hVEsysRec[s][i_az]->Fill( eRec, eRec - eMC );
                }
                if( hVEsysMC[s][i_az] )
                {
                    hVEsysMC[s][i_az]->Fill( eMC, eRec - eMC );
                }
                if( hVEsysMCRelative[s][i_az] )
                {
                    hVEsysMCRelative[s][i_az]->Fill( eMC, ( eRecLin - d->MCe0 ) / d->MCe0 );
                }
                if( hVEsysMCRelativeRMS[s][i_az] )
                {
                    hVEsysMCRelativeRMS[s][i_az]->Fill( eMC, ( eRecLin - d->MCe0 ) / d->MCe0 );
                }
                if( hVEsysMCRelative2D[s][i_az] )
                {
                    hVEsysMCRelative2D[s][i_az]->Fill( eMC, eRecLin / d->MCe0 );
                }
                if( hVEsys2D[s][i_az] )
                {
                    hVEsys2D[s][i_az]->Fill( eMC, eRec - eMC );
                }
                if( hVEmcCutCTA[s][i_az] )
                {
                    hVEmcCutCTA[s][i_az]->Fill( eRec, eMC );
                }
                // migration matrix (coarse binning)
                if( hVResponseMatrix[s][i_az] )
                {
                    hVResponseMatrix[s][i_az]->Fill( eRec, eMC );
                }
                // migration matrix (fine binning)
                if( hVResponseMatrixFine[s][i_az] )
                {
                    hVResponseMatrixFine[s][i_az]->Fill( eRec, eMC, i_weight );
                }

                if( hVResponseMatrixProfile[s][i_az] )
                {
                    hVResponseMatrixProfile[s][i_az]->Fill( eRec, eMC );
                }
                // events weighted by CR spectra
                if( hVWeightedRate[s][i_az] )
                {
                    hVWeightedRate[s][i_az]->Fill( eRec, getCRWeight( d->MCe0, hVEmc[s][i_az] ) );
                }
                if( hVWeightedRate005[s][i_az] )
                {
                    hVWeightedRate005[s][i_az]->Fill( eRec, getCRWeight( d->MCe0, hVEmc[s][i_az] ) );
                }
            }
        }
        // don't do anything between here and the end of the loop! Never!
    }                                             // end of loop
    /////////////////////////////////////////////////////////////////////////////



    /////////////////////////////////////////////////////////////////////////////
    //
    // calculate effective areas and fill output trees
    //
    /////////////////////////////////////////////////////////////////////////////

    ze = fZe[ize];
    fTNoise = fNoise[ize];
    // WARNING: hardwired values - not used to my knowledge anywhere? (GM)
    fTNoisePE = ( double )( fNoise[ize] ) / 0.15 * 1.e9;
    fTPedvar = fPedVar[ize];
    fXoff = fXWobble[ize];
    fYoff = fYWobble[ize];
    fWoff = sqrt( fXoff* fXoff + fYoff* fYoff );

    // loop over all spectral index
    for( unsigned int s = 0; s < fVSpectralIndex.size(); s++ )
    {
        fSpectralIndex = fVSpectralIndex[s];
        // loop over all az bins
        for( unsigned int i_az = 0; i_az < fVMinAz.size(); i_az++ )
        {
            fAzBin = ( int )i_az;
            fMinAz = fVMinAz[i_az];
            fMaxAz = fVMaxAz[i_az];

            // bayesdivide works only for weights == 1
            // errors might be wrong, since histograms are filled with weights != 1
            if(!binomialDivide( gEffAreaMC, hVEcut[s][i_az], hVEmc[s][i_az] ) )
            {
                cout << "VEffectiveAreaCalculator::fill: error calculating effective area vs MC energy" << endl;
                cout << "s : " << s << " , az: " << i_az << endl;
            }
            if(!binomialDivide( gEffAreaRec, hVEcutRec[s][i_az], hVEmc[s][i_az] ) )
            {
                cout << "VEffectiveAreaCalculator::fill: error calculating effective area vs rec energy" << endl;
                cout << "s : " << s << " , az: " << i_az << endl;
            }
            if(!binomialDivide( gEffAreaNoTh2MC, hVEcutNoTh2[s][i_az], hVEmc[s][i_az] ) )
            {
                cout << "VEffectiveAreaCalculator::fill: error calculating effective area before cuts vs MC energy" << endl;
                cout << "s : " << s << " , az: " << i_az << endl;
            }
            if(!binomialDivide( gEffAreaNoTh2Rec, hVEcutRecNoTh2[s][i_az], hVEmc[s][i_az] ) )
            {
                cout << "VEffectiveAreaCalculator::fill: error calculating effective area before cuts vs rec energy" << endl;
                cout << "s : " << s << " , az: " << i_az << endl;
            }
            // normalize response matrices
            VHistogramUtilities::normalizeTH2D_y( hVResponseMatrix[s][i_az] );
            VHistogramUtilities::normalizeTH2D_y( hVResponseMatrixQC[s][i_az] );
            VHistogramUtilities::normalizeTH2D_y( hVResponseMatrixNoDirectionCut[s][i_az] );

            for( int i = 0; i < 1000; i++ )
            {
                e0[i] = 0.;
                eff[i] = 0.;
                effNoTh2[i] = 0.;
                effNoTh2_error[i] = 0.;
                eff_error[i] = 0.;
                esys_rel[i] = 0.;
                seff_L[i] = 0.;
                seff_U[i] = 0.;
                Rec_e0[i] = 0.;
                Rec_eff[i] = 0.;
                Rec_effNoTh2[i] = 0.;
                Rec_seff_L[i] = 0.;
                Rec_seff_U[i] = 0.;
                Rec_eff_error[i] = 0.;
                Rec_effNoTh2_error[i] = 0.;
                Rec_angRes_p68[i] = 0.;
                Rec_angRes_p80[i] = 0.;
            }
            double x = 0.;
            double y = 0.;
            // effective area vs MC energy
            nbins = gEffAreaMC->GetN();
            // effective area vs reconstructed energy (approx)
            Rec_nbins = gEffAreaRec->GetN();

            // New version, hopefully more compact.
            // 1) Multiply effective areas by scatter area.
            multiplyByScatterArea( gEffAreaMC );
            multiplyByScatterArea( gEffAreaRec );
            multiplyByScatterArea( gEffAreaNoTh2MC );
            multiplyByScatterArea( gEffAreaNoTh2Rec );

            // 2) Fetch from the graph the eff area arrays and errors.
            for( int i = 0; i < nbins; i++ )
            {
                gEffAreaMC->GetPoint( i, x, y );
                e0[i] = x;
                eff[i] = y;
                seff_L[i] = gEffAreaMC->GetErrorYlow( i );
                seff_U[i] = gEffAreaMC->GetErrorYhigh( i );
                eff_error[i] = 0.5 * ( seff_L[i] + seff_U[i] );
                // Note!
                // hVEsysMCRelative with a different binning (lower)
                // than nbins; leads pairs of identical entries
                // in esys_rel
                if( hVEsysMCRelative[s][i_az] )
                {
                    esys_rel[i] = hVEsysMCRelative[s][i_az]->GetBinContent(
                                      hVEsysMCRelative[s][i_az]->GetXaxis()->FindBin( e0[i] ) );
                }
                // Save also the NoDirectionCut eff areas
                gEffAreaNoTh2MC->GetPoint( i, x, y );
                effNoTh2[i] = y;
                effNoTh2_error[i] = 0.5 * ( gEffAreaNoTh2MC->GetErrorYlow( i ) +
                                            gEffAreaNoTh2MC->GetErrorYhigh( i )
                                          );
            }
            for( int i = 0; i < Rec_nbins; i++ )
            {
                gEffAreaRec->GetPoint( i, x, y );
                Rec_e0[i] = x;
                // this is an approximation, since scatter area is defined over E_MC (GM: don't understand this comment)
                Rec_eff[i] = y;
                Rec_seff_L[i] = gEffAreaRec->GetErrorYlow( i );
                Rec_seff_U[i] = gEffAreaRec->GetErrorYhigh( i );
                Rec_eff_error[i] = 0.5 * ( Rec_seff_L[i] + Rec_seff_U[i] );
                // Save also the NoDirectionCut eff areas
                gEffAreaNoTh2Rec->GetPoint( i, x, y );
                Rec_effNoTh2[i] = y;
                Rec_effNoTh2_error[i] = 0.5 * ( gEffAreaNoTh2Rec->GetErrorYlow( i ) +
                                                gEffAreaNoTh2Rec->GetErrorYhigh( i )
                                              );
            }


            // copy all histograms
            resetHistograms( ize );

            // Histograms that depend on Erec -> add them depending on the index
            copyHistograms( hEcutRec, hVEcutRec[s][i_az], false );
            copyHistograms( hEcutRecUW, hVEcutRecUW[s][i_az], false );
            copyProfileHistograms( hEsysRec,  hVEsysRec[s][i_az] );

            // Histograms that depend on EMC -> add only the ones for the first index.
            if( s == 0 )
            {
                copyHistograms( hEmc, hVEmc[s][i_az], false );
                copyHistograms( hEcut, hVEcut[s][i_az], false );
                copyHistograms( hEcutUW, hVEcutUW[s][i_az], false );
                copyHistograms( hEcut500, hVEcut500[s][i_az], false );
                copyHistograms( hEcutLin, hVEcutLin[s][i_az], false );
                copyProfileHistograms( hEmcSWeight, hVEmcSWeight[s][i_az] );
                copyProfileHistograms( hEsysMC, hVEsysMC[s][i_az] );
                copyProfileHistograms( hEsysMCRelative, hVEsysMCRelative[s][i_az] );
                copyHistograms( hEsysMCRelativeRMS, hVEsysMCRelativeRMS[s][i_az], true );
                copyHistograms( hEsysMCRelative2D, hVEsysMCRelative2D[s][i_az], true );
                copyHistograms( hEsysMCRelative2DNoDirectionCut, hVEsysMCRelative2DNoDirectionCut[s][i_az], true );
                copyHistograms( hEmcCutCTA, hVEmcCutCTA[s][i_az], true );
                copyHistograms( hEsys2D, hVEsys2D[s][i_az], true );
                copyHistograms( hResponseMatrix, hVResponseMatrix[s][i_az], true );
                copyHistograms( hResponseMatrixQC, hVResponseMatrixQC[s][i_az], true );
                copyHistograms( hResponseMatrixNoDirectionCut, hVResponseMatrixNoDirectionCut[s][i_az], true );
                copyProfileHistograms( hResponseMatrixProfile, hVResponseMatrixProfile[s][i_az] );
                copyHistograms( hResponseMatrixFine, hVResponseMatrixFine[s][i_az], true );
                copyHistograms( hResponseMatrixFineQC, hVResponseMatrixFineQC[s][i_az], true );
                copyHistograms( hResponseMatrixFineNoDirectionCut, hVResponseMatrixFineNoDirectionCut[s][i_az], true );
                copyHistograms( hWeightedRate, hVWeightedRate[s][i_az], false );
                copyHistograms( hWeightedRate005, hVWeightedRate005[s][i_az], false );
            }

            for( unsigned int e = 0; e < hEcutSub.size(); e++ )
            {
                copyHistograms( hEcutSub[e], hVEcutSub[s][e][i_az], false );
            }

            copyHistograms( hAngularDiff_2D, hVAngularDiff_2D[i_az], true );
            copyHistograms( hAngularLogDiff_2D, hVAngularLogDiff_2D[i_az], true );
            if( s == 0 )
            {
                // Fill only for the first index.
                copyHistograms( hAngularDiffEmc_2D, hVAngularDiffEmc_2D[i_az], true );
                copyHistograms( hAngularLogDiffEmc_2D, hVAngularLogDiffEmc_2D[i_az], true );
            }

            // fill angular resolution vs energy
            fillAngularResolution( i_az, false );
            fillAngularResolution( i_az, true );


            // Assuming that the error on the energy reconstruction is aprroximatly gaussian
            TH2F* hResponseMatrixFineQC_rebined = ( TH2F* ) hVResponseMatrixFineQC[s][i_az]->Rebin2D( 2, 2, "hResponseMatrixFineQC_rebined" );
            // nbins_ResMat = hVResponseMatrixFineQC[s][i_az]->GetYaxis()->GetNbins();
            nbins_ResMat = hResponseMatrixFineQC_rebined->GetYaxis()->GetNbins();
            for( int i_ybin = 0 ; i_ybin < nbins_ResMat ; i_ybin++ )
            {
                fGauss->SetParameter( 0, 1 );
                fGauss->SetParameter( 1, hResponseMatrixFineQC_rebined->GetYaxis()->GetBinCenter( i_ybin ) );
                fGauss->SetParameter( 2, 0.2 );

                // Getting a slice
                // TH1D *i_slice = hVResponseMatrixFineQC[s][i_az]->ProjectionX("i_slice_Project", i_ybin,i_ybin);
                TH1F* i_slice = ( TH1F* ) hResponseMatrixFineQC_rebined->ProjectionX( "i_slice_Project", i_ybin, i_ybin );

                // Fitting quietly
                // i_slice->Fit("fGauss","0q");
                // cout << "GetEntries() : " << i_slice->GetEntries() << " " << " GetSumOfWeights() : " << i_slice->GetSumOfWeights() << endl;
                if( i_slice->GetEntries() == 0 || i_slice->GetSumOfWeights() == 0 )
                {

                    //	cout << " Getting fit for " <<  hVResponseMatrixFineQC[s][i_az]->GetYaxis()->GetBinCenter(i_ybin)
                    //     	     << " " << fGauss->GetParameter(1) << " " << fGauss->GetParameter(2)
                    //     	     << endl;

                    // No data to fit to
                    // ResMat_MC[i_ybin] = hVResponseMatrixFineQC[s][i_az]->GetYaxis()->GetBinCenter(i_ybin);
                    ResMat_MC[i_ybin] = hResponseMatrixFineQC_rebined->GetYaxis()->GetBinCenter( i_ybin );
                    ResMat_Rec[i_ybin] = -999;
                    ResMat_Rec_Err[i_ybin] = 1;

                }
                else
                {
                    i_slice->Fit( "fGauss", "0q" );

                    // cout << " Getting fit for " <<  hVResponseMatrixFineQC[s][i_az]->GetYaxis()->GetBinCenter(i_ybin)
                    //     << " " << fGauss->GetParameter(1) << " " << fGauss->GetParameter(2)
                    //     << " Fit Status: " << gMinuit->fCstatu
                    //     << " fStatus: " << gMinuit->fStatus << endl;

                    // Assuming Gaussian Fit
                    // ResMat_MC[i_ybin] = hVResponseMatrixFineQC[s][i_az]->GetYaxis()->GetBinCenter(i_ybin);
                    ResMat_MC[i_ybin] = hResponseMatrixFineQC_rebined->GetYaxis()->GetBinCenter( i_ybin );
                    ResMat_Rec[i_ybin] = fGauss->GetParameter( 1 );
                    ResMat_Rec_Err[i_ybin] = fGauss->GetParameter( 2 );

                }

                //		ResMat_MC[i_ybin] = hVResponseMatrixFineQC[s][i_az]->GetYaxis()->GetBinCenter(i_ybin);
                //		ResMat_Rec[i_ybin] = fGauss->GetParameter(1);
                //		ResMat_Rec_Err[i_ybin] = fGauss->GetParameter(2);

                delete i_slice;
            }

            delete hResponseMatrixFineQC_rebined;

            fEffArea->Fill();
        }
    }

    fCuts->printCutStatistics();
    cout << "\t total number of events after cuts: " << iSuccessfullEventStatistics << endl;

    return true;
}


/*!
 *
 *  CALLED TO USE EFFECTIVE AREAS
 *
 * interpolate between effective area from different zenith angles with cos weights
 *
 *   return value is 1/effective area
 *
 *
 */
double VEffectiveAreaCalculator::getEffectiveArea( double erec, double ze, double woff, double iPedVar, double iSpectralIndex,
        bool bAddtoMeanEffectiveArea, int iEffectiveAreaVsEnergyMC )
{
    if( bNOFILE )
    {
        return 1.;
    }

    ///////////////////////////////////////////
    // read effective areas from histograms
    // (this is the default case)
    ///////////////////////////////////////////
    if( bEffectiveAreasareHistograms )
    {
        return getEffectiveAreasFromHistograms( erec, ze, woff, iPedVar, iSpectralIndex, bAddtoMeanEffectiveArea, iEffectiveAreaVsEnergyMC );
    }

    ///////////////////////////////////////////
    // read effective areas from functions
    // (do not do this unless you know what your are doing)
    ///////////////////////////////////////////

    if( bEffectiveAreasareFunctions )
    {
        // get upper and lower zenith angle bins
        unsigned int ize_low = 0;
        unsigned int ize_up = 0;

        // log10 of energy
        double lerec = log10( erec );

        if( ze <= fZe[0] )
        {
            ize_low = ize_up = 0;
        }
        else if( ze >= fZe[fZe.size() - 1] )
        {
            ize_low = ize_up = fZe.size() - 1;
        }
        else
        {
            for( unsigned int i = 0; i < fZe.size(); i++ )
            {
                if( ze > fZe[i] )
                {
                    ize_low = ( int )i;
                    ize_up = ( int )i + 1;
                }
            }
        }
        double ie_zelow = 0.;
        double ie_zeup = 0.;
        if( bEffectiveAreasareFunctions )
        {
            getEffectiveAreasFromFitFunction( ize_low, ize_up, lerec, ie_zelow, ie_zeup );
        }
        else
        {
            ie_zelow = 0.;
            ie_zeup =  0.;
        }

        // interpolate between zenith angles (weighted by cos(ze))
        double ieff = VStatistics::interpolate( ie_zelow, fZe[ize_low], ie_zeup, fZe[ize_up], ze, true, 0.5, -90. );

        // return inverse
        if( ieff != 0. )
        {
            return 1. / ieff;
        }
    }

    return 1.;
}


/*!
 *  CALLED TO USE EFFECTIVE AREAS
 */
void VEffectiveAreaCalculator::getEffectiveAreasFromFitFunction( unsigned int ize_low, unsigned int ize_up, double lerec, double& ie_zelow, double& ie_zeup )
{
    if( ize_low < fEffAreaFitFunction.size() && fEffAreaFitFunction[ize_low] )
    {
        ie_zelow = fEffAreaFitFunction[ize_low]->Eval( lerec );
    }
    if( ize_up  < fEffAreaFitFunction.size() && fEffAreaFitFunction[ize_up] )
    {
        ie_zeup  = fEffAreaFitFunction[ize_up]->Eval( lerec );
    }
}


/*
      this function always returns a vector of size 2
*/
vector< unsigned int > VEffectiveAreaCalculator::getUpperLowBins( vector< double > i_values, double d )
{
    vector< unsigned int > i_temp( 2, 0 );

    double i_min = 1.e10;
    unsigned int i_min_index = 0;
    double i_max = -1.e10;
    unsigned int i_max_index = 0;

    for( unsigned int i = 0; i < i_values.size(); i++ )
    {
        if( i_values[i] < i_min )
        {
            i_min = i_values[i];
            i_min_index = i;
        }
        if( i_values[i] > i_max )
        {
            i_max = i_values[i];
            i_max_index = i;
        }
    }
    if( d < i_min )
    {
        i_temp[0] = i_min_index;
        i_temp[1] = i_min_index;
        return i_temp;
    }
    if( d > i_max )
    {
        i_temp[0] = i_max_index;
        i_temp[1] = i_max_index;
        return i_temp;
    }

    // look for closest values
    double i_diff_upper = 1.e10;
    unsigned int i_diff_upper_index = 0;
    double i_diff_lower = 1.e10;
    unsigned int i_diff_lower_index = 0;
    for( unsigned int i = 0; i < i_values.size(); i++ )
    {
        if( i_values[i] - d > 0. && i_values[i] - d < i_diff_upper )
        {
            i_diff_upper = i_values[i] - d;
            i_diff_upper_index = i;
        }
        if( d - i_values[i] > 0. && d - i_values[i] < i_diff_lower )
        {
            i_diff_lower = d - i_values[i];
            i_diff_lower_index = i;
        }
    }
    i_temp[0] = i_diff_lower_index;
    i_temp[1] = i_diff_upper_index;

    return i_temp;
}


/*!
 *  CALLED TO USE EFFECTIVE AREAS
 *
 *  return effective area value for given ze, woff, iPedVar, ...
 *
 *
 */
double VEffectiveAreaCalculator::getEffectiveAreasFromHistograms( double erec, double ze, double woff, double iPedVar, double iSpectralIndex,
        bool bAddtoMeanEffectiveArea, int iEffectiveAreaVsEnergyMC )
{
    vector< double > i_eff_temp( fNBins, 0. );

    // These will need to be defined regardless
    TH2F* i_Res_temp = 0;
    vector< TH2F*  > i_ze_Res_temp;
    vector< double > i_eff_MC_temp;

    // Response Matrix
    vector< double > i_ResMat_MC_temp;
    vector< double > i_ResMat_Rec_temp;
    vector< double > i_ResMat_Rec_Err_temp;


    vector< vector < double > > i_ze_eff_MC_temp;

    // Response Matrix
    //vector< vector < double > > i_ze_ResMat_MC_temp;
    //vector< vector < double > > i_ze_ResMat_Rec_temp;
    //vector< vector < double > > i_ze_ResMat_Rec_Err_temp;



    // log10 of energy
    if( erec <= 0. )
    {
        return 0.;
    }
    double lerec = log10( erec );

    // calculate mean values
    fEffectiveAreas_meanZe     += ze;
    fEffectiveAreas_meanWoff   += woff;
    fEffectiveAreas_meanPedVar += iPedVar;
    fEffectiveAreas_meanIndex   = iSpectralIndex;
    fEffectiveAreas_meanN++;

    ////////////////////////////////////////////////////////
    // get upper and lower zenith angle bins
    ////////////////////////////////////////////////////////
    vector< unsigned int > i_ze_bins = getUpperLowBins( fZe, ze );
    vector< vector< double > > i_ze_eff_temp( 2, i_eff_temp );

    if( bLikelihoodAnalysis )
    {
        // Temp Response Matrix Vector
        i_eff_MC_temp.assign( nbins_MC, 0. );

        // Temp EffectiveAreas Vector
        i_ze_eff_MC_temp.resize( 2 );
        i_ze_eff_MC_temp[0].resize( i_eff_MC_temp.size() );
        i_ze_eff_MC_temp[1].resize( i_eff_MC_temp.size() );

        i_ze_Res_temp.assign( 2, 0 );
    }

    for( unsigned int i = 0; i < i_ze_bins.size(); i++ )
    {
        ////////////////////////////////////////////////////////
        // get lower and upper wobble offset bins
        ////////////////////////////////////////////////////////
        if( i_ze_bins[i] < fEff_WobbleOffsets.size() )
        {
            vector< unsigned int > i_woff_bins = getUpperLowBins( fEff_WobbleOffsets[i_ze_bins[i]], woff );
            vector< vector< double > > i_woff_eff_temp( 2, i_eff_temp );
            vector< vector< double > > i_woff_eff_MC_temp;

            vector <TH2F*> i_woff_Res_temp;

            if( bLikelihoodAnalysis )
            {
                // Temp EffectiveAreas Vector
                i_woff_eff_MC_temp.resize( 2 );

                i_woff_eff_MC_temp[0].resize( i_eff_MC_temp.size() );
                i_woff_eff_MC_temp[1].resize( i_eff_MC_temp.size() );

                i_woff_Res_temp.assign( 2, 0 );

            }

            for( unsigned int w = 0; w < i_woff_bins.size(); w++ )
            {
                ////////////////////////////////////////////////////////
                // get lower and upper noise bins
                ////////////////////////////////////////////////////////
                if( i_ze_bins[i] < fEff_Noise.size() && i_woff_bins[w] < fEff_Noise[i_ze_bins[i]].size() )
                {
                    vector< unsigned int > i_noise_bins = getUpperLowBins( fEff_Noise[i_ze_bins[i]][i_woff_bins[w]], iPedVar );
                    vector< vector< double > > i_noise_eff_temp( 2, i_eff_temp );
                    vector< vector< double > > i_noise_eff_MC_temp;

                    vector <TH2F*> i_noise_Res_temp;

                    if( bLikelihoodAnalysis )
                    {
                        // Temp EffectiveAreas Vector
                        i_noise_eff_MC_temp.resize( 2 );

                        i_noise_eff_MC_temp[0].resize( i_eff_MC_temp.size() );
                        i_noise_eff_MC_temp[1].resize( i_eff_MC_temp.size() );

                        i_noise_Res_temp.assign( 2, 0 );
                    }

                    for( unsigned int n = 0; n < i_noise_bins.size(); n++ )
                    {
                        ////////////////////////////////////////////////////////
                        // get lower and upper spectral index bins
                        ////////////////////////////////////////////////////////
                        if( i_ze_bins[i] < fEff_SpectralIndex.size()
                                && i_woff_bins[w] < fEff_SpectralIndex[i_ze_bins[i]].size()
                                && i_noise_bins[n] < fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]].size() )
                        {
                            vector< unsigned int > i_index_bins = getUpperLowBins( fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[n]], iSpectralIndex );
                            unsigned int i_ID_0 = i_index_bins[0] + 100 * ( i_noise_bins[n] + 100 * ( i_woff_bins[w] + 100 * i_ze_bins[i] ) );
                            unsigned int i_ID_1 = i_index_bins[1] + 100 * ( i_noise_bins[n] + 100 * ( i_woff_bins[w] + 100 * i_ze_bins[i] ) );
                            i_noise_eff_temp[n] = interpolate_effectiveArea( iSpectralIndex,
                                                  fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[n]][i_index_bins[0]],
                                                  fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[n]][i_index_bins[1]],
                                                  fEffArea_map[i_ID_0],
                                                  fEffArea_map[i_ID_1], false );

                            if( bLikelihoodAnalysis )
                            {
                                i_index_bins.clear();
                                // Response matrix only filled for first index bin
                                i_index_bins = getUpperLowBins( fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[n]],
                                                                fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[n]][0] );
                                i_ID_0 = i_index_bins[0] + 100 * ( i_noise_bins[n] + 100 * ( i_woff_bins[w] + 100 * i_ze_bins[i] ) );

                                i_noise_eff_MC_temp[n] = fEffAreaMC_map[i_ID_0];
                                if( fEsysMCRelative2D_map[i_ID_0] )
                                {
                                    i_noise_Res_temp[n] = ( TH2F* ) fEsysMCRelative2D_map[i_ID_0]->Clone();
                                }
                                else
                                {
                                    i_noise_Res_temp[n] = 0;
                                }
                            }
                        }
                        else
                        {
                            cout << "VEffectiveAreaCalculator::getEffectiveAreasFromHistograms error: spectral index index out of range: ";
                            cout << i_ze_bins[i] << " " << fEff_SpectralIndex.size();
                            if( i_ze_bins[i] < fEff_SpectralIndex.size() )
                            {
                                cout << " " << i_woff_bins[w] << " " << fEff_SpectralIndex[i_ze_bins[i]].size();
                            }
                            if( i_woff_bins[w] < fEff_SpectralIndex[i_ze_bins[i]].size() )
                            {
                                cout << " " << i_noise_bins[n] <<  fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]].size();
                            }
                            cout << endl;
                            return -1.;
                        }
                        ////////////////////////////////////////////////////////
                    }
                    i_woff_eff_temp[w] = interpolate_effectiveArea( iPedVar,
                                         fEff_Noise[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[0]],
                                         fEff_Noise[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[1]],
                                         i_noise_eff_temp[0],
                                         i_noise_eff_temp[1], false );

                    if( bLikelihoodAnalysis )
                    {

                        i_woff_eff_MC_temp[w] = interpolate_effectiveArea( iPedVar,
                                                fEff_Noise[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[0]],
                                                fEff_Noise[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[1]],
                                                i_noise_eff_MC_temp[0],
                                                i_noise_eff_MC_temp[1], false );


                        i_woff_Res_temp[w] = interpolate_responseMatrix( iPedVar,
                                             fEff_Noise[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[0]],
                                             fEff_Noise[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[1]],
                                             i_noise_Res_temp[0], i_noise_Res_temp[1], false );
                    }

                }
                else
                {
                    cout << "VEffectiveAreaCalculator::getEffectiveAreasFromHistograms error: noise index out of range: " << i_ze_bins[i] << " " << fEff_Noise.size();
                    if( i_ze_bins[i] < fEff_Noise.size() )
                    {
                        cout << " " << i_woff_bins[w] << " " << fEff_Noise[i_ze_bins[i]].size() << endl;
                    }
                    cout << endl;
                    return -1.;
                }
            }
            i_ze_eff_temp[i] = interpolate_effectiveArea( woff,
                               fEff_WobbleOffsets[i_ze_bins[i]][i_woff_bins[0]],
                               fEff_WobbleOffsets[i_ze_bins[i]][i_woff_bins[1]],
                               i_woff_eff_temp[0],
                               i_woff_eff_temp[1], false );
            if( bLikelihoodAnalysis )
            {

                i_ze_eff_MC_temp[i] = interpolate_effectiveArea( woff,
                                      fEff_WobbleOffsets[i_ze_bins[i]][i_woff_bins[0]],
                                      fEff_WobbleOffsets[i_ze_bins[i]][i_woff_bins[1]],
                                      i_woff_eff_MC_temp[0],
                                      i_woff_eff_MC_temp[1], false );


                i_ze_Res_temp[i] = interpolate_responseMatrix( woff,
                                   fEff_WobbleOffsets[i_ze_bins[i]][i_woff_bins[0]],
                                   fEff_WobbleOffsets[i_ze_bins[i]][i_woff_bins[1]],
                                   i_woff_Res_temp[0], i_woff_Res_temp[1], false );
            }

        }
        else
        {
            cout << "VEffectiveAreaCalculator::getEffectiveAreasFromHistograms error: woff index out of range: ";
            cout << i_ze_bins[i] << " " << fEff_WobbleOffsets.size() << endl;
            return -1.;
        }
    }
    i_eff_temp = interpolate_effectiveArea( ze, fZe[i_ze_bins[0]], fZe[i_ze_bins[1]], i_ze_eff_temp[0], i_ze_eff_temp[1], true );

    if( bLikelihoodAnalysis )
    {
        i_eff_MC_temp = interpolate_effectiveArea( ze, fZe[i_ze_bins[0]], fZe[i_ze_bins[1]], i_ze_eff_MC_temp[0], i_ze_eff_MC_temp[1], true );
        i_Res_temp = interpolate_responseMatrix( ze, fZe[i_ze_bins[0]], fZe[i_ze_bins[1]], i_ze_Res_temp[0], i_ze_Res_temp[1], false );
    }


    if( fEff_E0.size() == 0 )
    {
        return -1.;
    }

    // mean effective area calculation
    if( bAddtoMeanEffectiveArea && fVMeanEffectiveArea.size() == i_eff_temp.size() )
    {
        for( unsigned int i = 0; i < i_eff_temp.size(); i++ )
        {
            fVMeanEffectiveArea[i] += i_eff_temp[i];
        }
        fNMeanEffectiveArea++;
    }

    if( bAddtoMeanEffectiveArea && fVTimeBinnedMeanEffectiveArea.size() == i_eff_temp.size() )
    {
        for( unsigned int i = 0; i < i_eff_temp.size(); i++ )
        {
            fVTimeBinnedMeanEffectiveArea[i] += i_eff_temp[i];
        }
        fNTimeBinnedMeanEffectiveArea++;

    }


    // Setting Mean MC EffectiveAreas and Response Matrix
    if( bLikelihoodAnalysis )
    {
        // Adding to mean effective area (MC)
        if( bAddtoMeanEffectiveArea && fVTimeBinnedMeanEffectiveAreaMC.size() == i_eff_MC_temp.size() )
        {
            for( unsigned int i = 0; i < fEff_E0.size() ; i++ )
            {
                if( i_eff_MC_temp[i] > 1.e-9 )
                {
                    fVTimeBinnedMeanEffectiveAreaMC[i] += i_eff_MC_temp[i];
                }
            }
            fNTimeBinnedMeanEffectiveAreaMC++;
        }
        // adding to mean response matrix
        addMeanResponseMatrix( i_Res_temp );

    }


    /////////////////////////////////////////
    // effective area for a specific energy
    /////////////////////////////////////////
    double i_eff_e = 1.;
    unsigned int ie0_low = 0;
    unsigned int ie0_up = 0;

    if( lerec <= fEff_E0[0] )
    {
        ie0_low = ie0_up = 0;
    }
    else if( lerec > fEff_E0[fEff_E0.size() - 1] )
    {
        ie0_low = ie0_up = fEff_E0.size() - 1;
    }
    else
    {
        for( unsigned int j = 1; j < fEff_E0.size() - 1; j++ )
        {
            if( lerec > fEff_E0[j] )
            {
                ie0_low = j;
                ie0_up = j + 1;
            }
        }
    }

    ///////////////////////////////////
    // linear interpolate between energies
    ///////////////////////////////////

    i_eff_e = VStatistics::interpolate( i_eff_temp[ie0_low], fEff_E0[ie0_low], i_eff_temp[ie0_up], fEff_E0[ie0_up], lerec, false );

    if( i_eff_e > 0. )
    {
        return 1. / i_eff_e;
    }

    return -1.;
}

// reset the sum of effective areas

void VEffectiveAreaCalculator::resetTimeBin()
{
    for( unsigned int i = 0; i < fVTimeBinnedMeanEffectiveArea.size(); i++ )
    {
        fVTimeBinnedMeanEffectiveArea[i] = 0;
    }
    fNTimeBinnedMeanEffectiveArea = 0;

    for( unsigned int i = 0; i < fVTimeBinnedMeanEffectiveAreaMC.size(); i++ )
    {
        fVTimeBinnedMeanEffectiveAreaMC[i] = 0;
    }
    fNTimeBinnedMeanEffectiveAreaMC = 0;
}

// Set the vector with Time Binning

void VEffectiveAreaCalculator::setTimeBin( double time )
{
    timebins.push_back( time );
}


/*
 *  CALLED FOR CALCULATION OF EFFECTIVE AREAS
 */
void VEffectiveAreaCalculator::resetHistograms( unsigned int ize )
{
    if( ize >= fZe.size() )
    {
        return;
    }

    char htitle[200];

    hEmc->Reset();
    sprintf( htitle, "energy spectrum (%1.f deg)", fZe[ize] );
    hEmc->SetTitle( htitle );

    hEcut->Reset();
    sprintf( htitle, "energy spectrum, after cuts (%1.f deg)", fZe[ize] );
    hEcut->SetTitle( htitle );

    hEcutUW->Reset();
    sprintf( htitle, "unweighted energy spectrum, after cuts (%1.f deg)", fZe[ize] );
    hEcutUW->SetTitle( htitle );

    hEcutLin->Reset();
    sprintf( htitle, "energy spectrum, after cuts (%1.f deg)", fZe[ize] );
    hEcutLin->SetTitle( htitle );

    hEcut500->Reset();
    sprintf( htitle, "energy spectrum, after cuts (%1.f deg)", fZe[ize] );
    hEcut500->SetTitle( htitle );

    hEcutRec->Reset();
    sprintf( htitle, "energy spectrum, after cutRecs (%1.f deg)", fZe[ize] );
    hEcutRec->SetTitle( htitle );

    hEcutRecUW->Reset();
    sprintf( htitle, "unweighted energy spectrum, after cutRecs (%1.f deg)", fZe[ize] );
    hEcutRecUW->SetTitle( htitle );

    sprintf( htitle, "effective area vs E_{MC} (%.1f deg)", fZe[ize] );
    gEffAreaMC->SetTitle( htitle );

    sprintf( htitle, "effective area vs E_{rec} (%.1f deg)", fZe[ize] );
    gEffAreaRec->SetTitle( htitle );

    sprintf( htitle, "effective area before cuts vs E_{MC} (%.1f deg)", fZe[ize] );
    gEffAreaNoTh2MC->SetTitle( htitle );

    sprintf( htitle, "effective area before cuts vs E_{rec} (%.1f deg)", fZe[ize] );
    gEffAreaNoTh2Rec->SetTitle( htitle );

    hEsysRec->Reset();
    sprintf( htitle, "energy reconstruction (%.1f deg)", fZe[ize] );
    hEsysRec->SetTitle( htitle );

    hEsysMC->Reset();
    sprintf( htitle, "energy reconstruction (%.1f deg)", fZe[ize] );
    hEsysMC->SetTitle( htitle );

    hEsysMCRelative->Reset();
    sprintf( htitle, "energy reconstruction (%.1f deg)", fZe[ize] );
    hEsysMCRelative->SetTitle( htitle );

    hEsysMCRelative2D->Reset();
    sprintf( htitle, "energy reconstruction (%.1f deg)", fZe[ize] );
    hEsysMCRelative2D->SetTitle( htitle );

    hEsys2D->Reset();
    sprintf( htitle, "energy reconstruction, distributions (%.1f deg)", fZe[ize] );
    hEsys2D->SetTitle( htitle );

    hEmcCutCTA->Reset();
    sprintf( htitle, "migration matrix (fine binning, %.1f deg)", fZe[ize] );
    hEmcCutCTA->SetTitle( htitle );

    hResponseMatrixFineQC->Reset();
    sprintf( htitle, "migration matrix, after quality cuts (fine binning, %.1f deg)", fZe[ize] );
    hResponseMatrixFineQC->SetTitle( htitle );

    hResponseMatrix->Reset();
    sprintf( htitle, "migration matrix (%.1f deg)", fZe[ize] );
    hResponseMatrix->SetTitle( htitle );

    hResponseMatrixQC->Reset();
    sprintf( htitle, "migration matrix, after quality cuts (%.1f deg)", fZe[ize] );
    hResponseMatrixQC->SetTitle( htitle );

    hResponseMatrixProfile->Reset();
    sprintf( htitle, "migration matrix (%.1f deg)", fZe[ize] );
    hResponseMatrixProfile->SetTitle( htitle );

    hEmcSWeight->Reset();
    sprintf( htitle, "spectral weights (%.1f deg)", fZe[ize] );
    hEmcSWeight->SetTitle( htitle );

    hAngularDiff_2D->Reset();
    sprintf( htitle, "hAngularDiff_2D" );
    hAngularDiff_2D->SetTitle( htitle );

    hAngularDiffEmc_2D->Reset();
    sprintf( htitle, "hAngularDiffEmc_2D" );
    hAngularDiffEmc_2D->SetTitle( htitle );

    hAngularLogDiff_2D->Reset();
    sprintf( htitle, "hAngularLogDiff_2D" );
    hAngularLogDiff_2D->SetTitle( htitle );

    hAngularLogDiffEmc_2D->Reset();
    sprintf( htitle, "hAngularLogDiffEmc_2D" );
    hAngularLogDiffEmc_2D->SetTitle( htitle );
}


void VEffectiveAreaCalculator::setWobbleOffset( double x, double y )
{
    if( fXWobble.size() == 0 )
    {
        fXWobble.push_back( x );
    }
    else
    {
        fXWobble[0] = x;
    }
    if( fYWobble.size() == 0 )
    {
        fYWobble.push_back( y );
    }
    else
    {
        fYWobble[0] = y;
    }

}

void VEffectiveAreaCalculator::setNoiseLevel( int iN, double iP )
{
    if( fNoise.size() == 0 )
    {
        fNoise.push_back( iN );
    }
    else
    {
        fNoise[0] = iN;
    }

    if( fPedVar.size() == 0 )
    {
        fPedVar.push_back( iP );
    }
    else
    {
        fPedVar[0] = iP;
    }
}

/*!
    set azimuth cut for effective area fillings
*/
void VEffectiveAreaCalculator::setAzimuthCut( int iAzBin, double iAzMin, double iAzMax )
{
    fAzBin = iAzBin;
    fMinAz = iAzMin;
    fMaxAz = iAzMax;
}


bool VEffectiveAreaCalculator::binomialDivide( TGraphAsymmErrors* g, TH1D* hrec, TH1D* hmc )
{
    if(!g )
    {
        cout << "VEffectiveAreaCalculator::binomialDivide error: no return graph given" << endl;
        return false;
    }
    if(!hrec )
    {
        cout << "VEffectiveAreaCalculator::binomialDivide error: no histogram with reconstructed events given" << endl;
        return false;
    }
    if(!hmc )
    {
        cout << "VEffectiveAreaCalculator::binomialDivide error: no histogram with simulated events given" << endl;
        return false;
    }

    int z = 0;
    double pj = 0.;
    double pr = 0.;
    double pm = 0.;
    double sj_low = 0.;
    double sj_up = 0.;

    for( int b = 1; b <= hmc->GetNbinsX(); b++ )
    {
        if( hmc->GetBinContent( b ) > 0 && hrec->GetBinContent( b ) > 0 )
        {
            pj = hrec->GetBinContent( b ) / hmc->GetBinContent( b );
            // error calculation for effective areas
            //    choose method with setStatisticsOption()
            //  this far from being straightforward!
            //  none of the methods works consistently, therefore the simplest (normal) solution
            //  is used.
            //  note: approach is only correct for unweighted histograms
            //
            // error calculation assuming normal distributions
            if(!fClopperPearson )
            {
                pr = hrec->GetBinError( b );
                pm = hmc->GetBinError( b );
                if( pj != 1. )
                {
                    sj_low = TMath::Abs((( 1. - 2.*pj ) * pr* pr + pj* pj* pm* pm ) / ( hmc->GetBinContent( b ) * hmc->GetBinContent( b ) ) );
                }
                else
                {
                    sj_low = 0.;
                }
                sj_low = sqrt( sj_low );
                sj_up  = sj_low;
            }
            // Clopper-Pearson error calculation
            else
            {
                sj_low = pj - TEfficiency::ClopperPearson(( int )hmc->GetBinContent( b ), ( int )hrec->GetBinContent( b ), 0.6827, false );
                sj_up  = TEfficiency::ClopperPearson(( int )hmc->GetBinContent( b ), ( int )hrec->GetBinContent( b ), 0.6827, true ) - pj;
            }

            // fill effective area graphs
            g->SetPoint( z, hmc->GetBinCenter( b ), pj );
            g->SetPointError( z, 0., 0., sj_low, sj_up );
            z++;
        }
    }
    g->Set( z );

    return true;
}


void VEffectiveAreaCalculator::smoothEffectiveAreas( map< unsigned int, vector< double > > m )
{
    cout << "\t smooth effective areas, parameters: " << fSmoothIter << ", " << fSmoothThreshold << endl;

    typedef map< unsigned int, vector< double > >::const_iterator CI;

    for( CI p = m.begin(); p != m.end(); ++ p )
    {
        vector< double > itemp = p->second;

        for( int l = 0; l < fSmoothIter; l++ )
        {
            for( unsigned int k = 1; k < itemp.size() - 1; k++ )
            {
                if( itemp[k - 1] <= 0 || itemp[k] <= 0. || itemp[k + 1] <= 0. )
                {
                    continue;
                }

                // upwards outlier
                if( itemp[k] / itemp[k - 1] > ( 1. + fSmoothThreshold ) && itemp[k] / itemp[k + 1] > ( 1. + fSmoothThreshold ) )
                {
                    itemp[k] = 0.5 * ( itemp[k - 1] + itemp[k + 1] );
                }

                // downwards outlier
                if( itemp[k] / itemp[k - 1] < ( 1. - fSmoothThreshold ) && itemp[k] / itemp[k + 1] < ( 1. - fSmoothThreshold ) )
                {
                    itemp[k] = 0.5 * ( itemp[k - 1] + itemp[k + 1] );
                }
            }
        }
    }
}


void VEffectiveAreaCalculator::copyProfileHistograms( TProfile* h1,  TProfile* h2 )
{
    if(!h1 || !h2 )
    {
        return;
    }

    string iEOption = h1->GetErrorOption();

    for( int b = 0; b <= h2->GetNbinsX(); b++ )
    {
        h1->SetBinContent( b, h2->GetBinContent( b ) * h2->GetBinEntries( b ) );

        if( TMath::Abs( h2->GetBinError( b ) ) < 1.e-4 )
        {
            h1->SetBinError( b, 0. );
        }
        else
        {
            if( h2->GetBinEntries( b ) > 0. )
            {
                double iE = h2->GetBinError( b );
                //		h1->SetBinError( b, iE );
                if( iEOption != "s" )
                {
                    iE = h2->GetBinError( b ) * sqrt( h2->GetBinEntries( b ) );
                }
                h1->SetBinError( b,  sqrt( h2->GetBinEntries( b ) * ( h2->GetBinContent( b ) *  h2->GetBinContent( b ) + iE* iE ) ) );
            }
            else
            {
                h1->SetBinError( b, 0. );
            }
        }
        h1->SetBinEntries( b, h2->GetBinEntries( b ) );
    }
    h1->SetEntries( h2->GetEntries() );
}


void VEffectiveAreaCalculator::copyHistograms( TH1* h1,  TH1* h2, bool b2D )
{
    if(!h1 || !h2 )
    {
        return;
    }

    if(!b2D )
    {
        for( int b = 0; b <= h2->GetNbinsX(); b++ )
        {
            h1->SetBinContent( b, h2->GetBinContent( b ) );
            h1->SetBinError( b, h2->GetBinError( b ) );
        }
    }
    else
    {
        for( int b = 0; b <= h2->GetNbinsX(); b++ )
        {
            for( int b2 = 0; b2 <= h2->GetNbinsY(); b2++ )
            {
                h1->SetBinContent( b, b2, h2->GetBinContent( b, b2 ) );
                h1->SetBinError( b, b2, h2->GetBinError( b, b2 ) );
            }
        }
    }
    h1->SetEntries( h2->GetEntries() );

}


/*

    return mean effective area for given run

    values are filled ....

*/
TGraphAsymmErrors* VEffectiveAreaCalculator::getMeanEffectiveArea()
{
    if( gMeanEffectiveArea && fNMeanEffectiveArea > 0 )
    {
        gMeanEffectiveArea->Set( 0 );
        int z = 0;
        for( unsigned int i = 0; i < fVMeanEffectiveArea.size(); i++ )
        {
            if( fVMeanEffectiveArea[i] > 0. )
            {
                gMeanEffectiveArea->SetPoint( z, fEff_E0[i], fVMeanEffectiveArea[i] / fNMeanEffectiveArea );
                z++;
            }
        }
        return gMeanEffectiveArea;
    }

    return 0;
}

TGraphAsymmErrors* VEffectiveAreaCalculator::getMeanEffectiveAreaMC()
{
    if( gMeanEffectiveAreaMC && fEffAreaMC_time.size() > 0 )
    {
        gMeanEffectiveArea->Set( 0 );
        double z = 0;
        double x = 0.;
        double y = 0.;

        // set energies
        for( unsigned int i = 0; i < fEff_E0.size(); i++ )
        {
            gMeanEffectiveAreaMC->SetPoint( i, fEff_E0[i], 0. );
        }

        for( unsigned int i = 0; i < fEffAreaMC_time.size(); i++ )
        {
            for( unsigned int j = 0; j < fEffAreaMC_time[i].size(); j++ )
            {
                gMeanEffectiveAreaMC->GetPoint( j, x, y );
                gMeanEffectiveAreaMC->SetPoint( j, x, y + fEffAreaMC_time[i][j] );
            }
        }

        z = ( double )fEffAreaMC_time.size();
        for( int i = 0; i < gMeanEffectiveAreaMC->GetN(); i++ )
        {
            if( z > 0. )
            {
                gMeanEffectiveAreaMC->GetPoint( i, x, y );
                gMeanEffectiveAreaMC->SetPoint( i, x, y / z );
            }
        }
        return gMeanEffectiveAreaMC;
    }

    return 0;
}



TGraph2DErrors* VEffectiveAreaCalculator::getTimeBinnedMeanEffectiveArea()
{
    int z = 0;

    if( gTimeBinnedMeanEffectiveArea )
    {
        gTimeBinnedMeanEffectiveArea->Set( 0 );
        for( unsigned int i = 0; i < fEffArea_time.size(); i++ )
        {
            for( unsigned int j = 0; j < fEffArea_time[i].size(); j++ )
            {
                if( fEffArea_time[i][j] > 0. )
                {
                    gTimeBinnedMeanEffectiveArea->SetPoint( z, fEff_E0[j], fEffArea_time[i][j], timebins[i] );
                    gTimeBinnedMeanEffectiveArea->SetPointError( z, 0, 0, 0 );
                    z++;
                }
            }
        }
        return gTimeBinnedMeanEffectiveArea;
    }

    return 0;
}


void VEffectiveAreaCalculator::setTimeBinnedMeanEffectiveArea()
{
    vector < double > inter;

    for( unsigned int i = 0; i < fVTimeBinnedMeanEffectiveArea.size(); i++ )
    {
        inter.push_back( fVTimeBinnedMeanEffectiveArea[i] / fNTimeBinnedMeanEffectiveArea );
    }

    fEffArea_time.push_back( inter );

    if( bLikelihoodAnalysis )
    {
        vector < double > interMC;
        for( unsigned int i = 0; i < fVTimeBinnedMeanEffectiveAreaMC.size(); i++ )
        {
            if( fNTimeBinnedMeanEffectiveAreaMC > 0. )
            {
                interMC.push_back( fVTimeBinnedMeanEffectiveAreaMC[i] / fNTimeBinnedMeanEffectiveAreaMC );
            }
            else
            {
                interMC.push_back( 0. );
            }
        }

        fEffAreaMC_time.push_back( interMC );
    }
}


void VEffectiveAreaCalculator::resetHistogramsVectors( unsigned int ize )
{
    for( unsigned int i = 0; i < hVEmc.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEmc[i].size(); j++ )
        {
            if( hVEmc[i][j] )
            {
                hVEmc[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEcut.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEcut[i].size(); j++ )
        {
            if( hVEcut[i][j] )
            {
                hVEcut[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEcutUW.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEcutUW[i].size(); j++ )
        {
            if( hVEcutUW[i][j] )
            {
                hVEcutUW[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEcutLin.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEcutLin[i].size(); j++ )
        {
            if( hVEcutLin[i][j] )
            {
                hVEcutLin[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEcut500.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEcut500[i].size(); j++ )
        {
            if( hVEcut500[i][j] )
            {
                hVEcut500[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEcutRec.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEcutRec[i].size(); j++ )
        {
            if( hVEcutRec[i][j] )
            {
                hVEcutRec[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEcutRecUW.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEcutRecUW[i].size(); j++ )
        {
            if( hVEcutRecUW[i][j] )
            {
                hVEcutRecUW[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEcutNoTh2.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEcutNoTh2[i].size(); j++ )
        {
            if( hVEcutNoTh2[i][j] )
            {
                hVEcutNoTh2[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEcutRecNoTh2.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEcutRecNoTh2[i].size(); j++ )
        {
            if( hVEcutRecNoTh2[i][j] )
            {
                hVEcutRecNoTh2[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEmcSWeight.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEmcSWeight[i].size(); j++ )
        {
            if( hVEmcSWeight[i][j] )
            {
                hVEmcSWeight[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEsysRec.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEsysRec[i].size(); j++ )
        {
            if( hVEsysRec[i][j] )
            {
                hVEsysRec[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEsysMC.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEsysMC[i].size(); j++ )
        {
            if( hVEsysMC[i][j] )
            {
                hVEsysMC[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEsysMCRelative.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEsysMCRelative[i].size(); j++ )
        {
            if( hVEsysMCRelative[i][j] )
            {
                hVEsysMCRelative[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEsysMCRelativeRMS.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEsysMCRelativeRMS[i].size(); j++ )
        {
            if( hVEsysMCRelativeRMS[i][j] )
            {
                hVEsysMCRelativeRMS[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEsysMCRelative2D.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEsysMCRelative2D[i].size(); j++ )
        {
            if( hVEsysMCRelative2D[i][j] )
            {
                hVEsysMCRelative2D[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEsysMCRelative2DNoDirectionCut.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEsysMCRelative2DNoDirectionCut[i].size(); j++ )
        {
            if( hVEsysMCRelative2DNoDirectionCut[i][j] )
            {
                hVEsysMCRelative2DNoDirectionCut[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEsys2D.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEsys2D[i].size(); j++ )
        {
            if( hVEsys2D[i][j] )
            {
                hVEsys2D[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVEmcCutCTA.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEmcCutCTA[i].size(); j++ )
        {
            if( hVEmcCutCTA[i][j] )
            {
                hVEmcCutCTA[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVResponseMatrix.size(); i++ )
    {
        for( unsigned int j = 0; j < hVResponseMatrix[i].size(); j++ )
        {
            if( hVResponseMatrix[i][j] )
            {
                hVResponseMatrix[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVResponseMatrixFine.size(); i++ )
    {
        for( unsigned int j = 0; j < hVResponseMatrixFine[i].size(); j++ )
        {
            if( hVResponseMatrixFine[i][j] )
            {
                hVResponseMatrixFine[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVResponseMatrixNoDirectionCut.size(); i++ )
    {
        for( unsigned int j = 0; j < hVResponseMatrixNoDirectionCut[i].size(); j++ )
        {
            if( hVResponseMatrixNoDirectionCut[i][j] )
            {
                hVResponseMatrixNoDirectionCut[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVResponseMatrixFineNoDirectionCut.size(); i++ )
    {
        for( unsigned int j = 0; j < hVResponseMatrixFineNoDirectionCut[i].size(); j++ )
        {
            if( hVResponseMatrixFineNoDirectionCut[i][j] )
            {
                hVResponseMatrixFineNoDirectionCut[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVResponseMatrixProfile.size(); i++ )
    {
        for( unsigned int j = 0; j < hVResponseMatrixProfile[i].size(); j++ )
        {
            if( hVResponseMatrixProfile[i][j] )
            {
                hVResponseMatrixProfile[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVResponseMatrixQC.size(); i++ )
    {
        for( unsigned int j = 0; j < hVResponseMatrixQC[i].size(); j++ )
        {
            if( hVResponseMatrixQC[i][j] )
            {
                hVResponseMatrixQC[i][j]->Reset();
            }
        }
    }
    for( unsigned int i = 0; i < hVResponseMatrixFineQC.size(); i++ )
    {
        for( unsigned int j = 0; j < hVResponseMatrixFineQC[i].size(); j++ )
        {
            if( hVResponseMatrixFineQC[i][j] )
            {
                hVResponseMatrixFineQC[i][j]->Reset();
            }
        }
    }

    for( unsigned int i = 0; i < hVWeightedRate.size(); i++ )
    {
        for( unsigned int j = 0; j < hVWeightedRate[i].size(); j++ )
        {
            if( hVWeightedRate[i][j] )
            {
                hVWeightedRate[i][j]->Reset();
            }
        }
    }

}


TH1D* VEffectiveAreaCalculator::getHistogramhEmc()
{
    if(!hEmc )
    {
        return 0;
    }

    hEmc->Reset();

    return hEmc;
}


TGraphErrors* VEffectiveAreaCalculator::getMeanSystematicErrorHistogram()
{
    if( fEffectiveAreas_meanN <= 0. )
    {
        return 0;
    }
    fEffectiveAreas_meanZe /= fEffectiveAreas_meanN;
    fEffectiveAreas_meanWoff /= fEffectiveAreas_meanN;
    fEffectiveAreas_meanPedVar /= fEffectiveAreas_meanN;

    gMeanSystematicErrorGraph = new TGraphErrors( 1 );
    gMeanSystematicErrorGraph->SetName( "gMeanEnergySystematicError" );
    gMeanSystematicErrorGraph->SetMarkerStyle( 20 );
    vector< double > hX;
    vector< double > i_eff_temp;

    ////////////////////////////////////////////////////////
    // get upper and lower zenith angle bins
    ////////////////////////////////////////////////////////
    vector< unsigned int > i_ze_bins = getUpperLowBins( fZe, fEffectiveAreas_meanZe );
    vector< vector< double > > i_ze_eff_temp( 2, hX );
    for( unsigned int i = 0; i < i_ze_bins.size(); i++ )
    {
        ////////////////////////////////////////////////////////
        // get lower and upper wobble offset bins
        ////////////////////////////////////////////////////////
        if( i_ze_bins[i] >= fEff_WobbleOffsets.size() )
        {
            continue;
        }

        vector< unsigned int > i_woff_bins = getUpperLowBins( fEff_WobbleOffsets[i_ze_bins[i]], fEffectiveAreas_meanWoff );
        vector< vector< double > > i_woff_eff_temp( 2, hX );
        for( unsigned int w = 0; w < i_woff_bins.size(); w++ )
        {
            ////////////////////////////////////////////////////////
            // get lower and upper noise bins
            ////////////////////////////////////////////////////////
            if( i_ze_bins[i] >= fEff_Noise.size() )
            {
                continue;
            }
            if( i_woff_bins[w] >= fEff_Noise[i_ze_bins[i]].size() )
            {
                continue;
            }

            vector< unsigned int > i_noise_bins = getUpperLowBins( fEff_Noise[i_ze_bins[i]][i_woff_bins[w]],
                                                  fEffectiveAreas_meanPedVar );
            vector< vector< double > > i_noise_eff_temp( 2, hX );
            for( unsigned int n = 0; n < i_noise_bins.size(); n++ )
            {
                ////////////////////////////////////////////////////////
                // get lower and upper spectral index bins
                ////////////////////////////////////////////////////////
                if( i_ze_bins[i] >= fEff_SpectralIndex.size() )
                {
                    continue;
                }
                if( i_woff_bins[w] >= fEff_SpectralIndex[i_ze_bins[i]].size() )
                {
                    continue;
                }
                if( i_noise_bins[n] >= fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]].size() )
                {
                    continue;
                }
                vector< unsigned int > i_index_bins = getUpperLowBins( fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[n]],
                                                      fEffectiveAreas_meanIndex );
                unsigned int i_ID_0 = i_index_bins[0] + 100 * ( i_noise_bins[n] + 100 * ( i_woff_bins[w] + 100 * i_ze_bins[i] ) );
                unsigned int i_ID_1 = i_index_bins[1] + 100 * ( i_noise_bins[n] + 100 * ( i_woff_bins[w] + 100 * i_ze_bins[i] ) );
                i_noise_eff_temp[n] = interpolate_effectiveArea(
                                          fEffectiveAreas_meanIndex,
                                          fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[n]][i_index_bins[0]],
                                          fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[n]][i_index_bins[1]],
                                          fEff_EsysMCRelative[i_ID_0], fEff_EsysMCRelative[i_ID_1], false );
            }
            i_woff_eff_temp[w] = interpolate_effectiveArea( fEffectiveAreas_meanPedVar,
                                 fEff_Noise[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[0]],
                                 fEff_Noise[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[1]],
                                 i_noise_eff_temp[0], i_noise_eff_temp[1], false );
        }
        i_ze_eff_temp[i] = interpolate_effectiveArea( fEffectiveAreas_meanWoff,
                           fEff_WobbleOffsets[i_ze_bins[i]][i_woff_bins[0]],
                           fEff_WobbleOffsets[i_ze_bins[i]][i_woff_bins[1]],
                           i_woff_eff_temp[0], i_woff_eff_temp[1], false );
    }


    i_eff_temp = interpolate_effectiveArea( fEffectiveAreas_meanZe,
                                            fZe[i_ze_bins[0]],
                                            fZe[i_ze_bins[1]],
                                            i_ze_eff_temp[0], i_ze_eff_temp[1], true );

    unsigned int z = 0;
    for( unsigned int i = 0; i < i_eff_temp.size(); i++ )
    {
        if( TMath::Abs( i_eff_temp[i] ) > 1.e-7 )
        {
            gMeanSystematicErrorGraph->SetPoint( z, fH2F_e0_esys[i], i_eff_temp[i] );
            z++;
        }
    }

    return gMeanSystematicErrorGraph;
}


bool VEffectiveAreaCalculator::setMonteCarloEnergyRange( double iMin, double iMax, double iMCIndex )
{
    if( fSpectralWeight )
    {
        fSpectralWeight->setMCParameter( iMCIndex, iMin, iMax );
        return true;
    }

    cout << "VEffectiveAreaCalculator::setMonteCarloEnergyRange error: not spectral weighter defined" << endl;
    return false;
}
/*
 * weight to correct MC spectrum to CR spectrum
 *
 * note that this requires the MC spectrum to be a power law
 *
 */
double VEffectiveAreaCalculator::getCRWeight( double iEMC_TeV_lin, TH1* h, bool for_back_map )
{
    if(!h || !fRunPara )
    {
        return 1.;
    }

    if(!fRunPara->fCREnergySpectrum )
    {
        return 1.;
    }



    // normalization of MC spectrum
    double c_ig = 0.;
    if( fRunPara->fIgnoreFractionOfEvents > 0. )
    {
        c_ig = fRunPara->fIgnoreFractionOfEvents;
    }
    double c_mc = ( 1.0 - c_ig ) * h->GetEntries()
                  * (-1.*TMath::Abs( fRunPara->fMCEnergy_index ) + 1. )
                  / ( TMath::Power( fRunPara->fMCEnergy_max, -1.*TMath::Abs( fRunPara->fMCEnergy_index ) + 1. )
                      - TMath::Power( fRunPara->fMCEnergy_min, -1.*TMath::Abs( fRunPara->fMCEnergy_index ) + 1. ) );

    // number of MC events in this energy bin
    double n_mc = c_mc * TMath::Power( iEMC_TeV_lin, -1.*TMath::Abs( fRunPara->fMCEnergy_index ) );

    // number of expected CR events /min/sr
    double n_cr = fMC_ScatterArea * fRunPara->fCREnergySpectrum->Eval( log10( iEMC_TeV_lin ) ) * 1.e4 * 60.;
    // fRunPara->fCREnergySpectrum->Eval( log10(iEMC_TeV_lin) ) returns the differential flux multiplied by the energy

    // (ctools) for the acceptance map construction, the weight is in #/s ()
    if( for_back_map )
    {
        //need to normalize this number considering the cone in which the particle are simulated (the offset bin is not consider so the binning can be changed later).
        if(! fsolid_angle_norm_done )
        {
            Calculate_Bck_solid_angle_norm();
        }
        n_cr = fsolid_angle_norm * n_cr / 60.;
        if( n_mc != 0. )
        {
            return n_cr / n_mc;
        }
        else
        {
            return 0.;
        }
    }
    else
    {
        // getMCSolidAngleNormalization() return a ratio of solid angle (only for gamma? not sure this thing returning something else than 1 here, ever...)
        if( getMCSolidAngleNormalization() > 0. )
        {
            n_cr /= getMCSolidAngleNormalization();    // do we want to leave this line here?
        }
        // the solid angle normalization is done in VSensitivityCalculator
        // return #/min/sr
        if( n_mc != 0. )
        {
            return n_cr / n_mc;
        }
        return 0.;
    }
}

// Calculate the ratio between the solid angle of the cone where the MC have been done and the solid angle of the offset bin considered
void VEffectiveAreaCalculator::Calculate_Bck_solid_angle_norm()
{


    if( fRunPara->fViewcone_max > 0. )
    {
        // solid angle in which the particule have been simulated
        double SolidAngle_MCScatterAngle  =  2 * TMath::Pi() * ( 1. - cos( fRunPara->fViewcone_max* TMath::DegToRad() ) );

        fsolid_angle_norm = SolidAngle_MCScatterAngle;
        fsolid_angle_norm_done = true;

    }


    return;
}


// Adding Response matrix to the time averaged
void VEffectiveAreaCalculator::addMeanResponseMatrix( TH2F* i_hTmp )
{
    if(!i_hTmp )
    {
        return;
    }

    // If hMeanResponseMatrix doesn't exist
    if(!hMeanResponseMatrix )
    {
        //cout << "\t\t\tVEffectiveAreaCalculator::addMeanResponseMatrix Creating new histogram" << endl;
        VHistogramUtilities::normalizeTH2D_x( i_hTmp );
        hMeanResponseMatrix = ( TH2F* )i_hTmp->Clone();
        hMeanResponseMatrix->Sumw2();

    }
    else
    {
        hMeanResponseMatrix->Add( i_hTmp );
        VHistogramUtilities::normalizeTH2D_x( hMeanResponseMatrix );
    }

}
