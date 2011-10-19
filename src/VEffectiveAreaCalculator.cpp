/*! \class VEffectiveAreaCalculator
 *  \brief calculate effective areas and energy spectra
 *
 *  \author
 *  Gernot Maier
 *
 *  Revision $Id: VEffectiveAreaCalculator.cpp,v 1.23.2.14.4.9.10.1.2.11.4.1.2.15.2.11.6.2.2.11.2.18.2.1.2.7 2011/03/31 14:50:58 gmaier Exp $
 */

#include "VEffectiveAreaCalculator.h"

/*!
 *  CALLED FOR CALCULATION OF EFFECTIVE AREAS
 *
 *  this constructor is called for FILLING of the effective area tree
 *  (calculation of effective areas)
 *
 */
VEffectiveAreaCalculator::VEffectiveAreaCalculator( VInstrumentResponseFunctionRunParameter *iRunPara, VGammaHadronCuts* icuts )
{
    if( !iRunPara )
	{
	   cout << "VEffectiveAreaCalculator: no run parameters given" << endl;
	   cout << "...exiting..." << endl;
	   exit( -1 );
    }
    reset();

// no effective area file present
    bNOFILE = true;

// number of bins for histograms
    nbins = iRunPara->fEnergyAxisBins_log10;

// cuts
    fCuts = icuts;
    fZe.push_back( iRunPara->fze );
    fAreaRadius.push_back( iRunPara->fCoreScatterRadius );
    fScatterMode.push_back( iRunPara->fCoreScatterMode );
    for( unsigned int i = 0; i < fZe.size(); i++ )
    {
        fXWobble.push_back( 0. );
        fYWobble.push_back( 0. );
        fNoise.push_back( 0 );
        fPedVar.push_back( 0. );
    }
    setIgnoreEnergyReconstructionCuts( iRunPara->fIgnoreEnergyReconstructionQuality );
    setIsotropicArrivalDirections( iRunPara->fIsotropicArrivalDirections );
    setCTA( iRunPara->fTelescopeTypeCuts );
    setWobbleOffset( iRunPara->fXoff, iRunPara->fYoff );
    setNoiseLevel( iRunPara->fnoise, iRunPara->fpedvar );

// spectral weighting class
    fSpectralWeight = new VSpectralWeight();
	setMonteCarloEnergyRange( iRunPara->fMCEnergy_min, iRunPara->fMCEnergy_max, TMath::Abs( iRunPara->fMCEnergy_index ) );

// define output tree (all histograms are written to this tree)
    hisTreeList = new TList();

// these histograms are filled into the output tree
    char hname[400];
    char htitle[400];

// define range and binning of energy axis of effective area plots
    cout << "histogram parameters (bins, log10(Emin), log10(Emax)): " << nbins << ", " << fEnergyAxis_minimum_defaultValue << ", " << fEnergyAxis_maximum_defaultValue << endl;
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

    sprintf( hname, "gRecProb" );
    gRecProb = new TGraphAsymmErrors( 1 );
    gRecProb->SetName( hname );
    gRecProb->SetTitle( htitle );
    hisTreeList->Add( gRecProb );

// spectral weight
    sprintf( hname, "hEmcSWeight" );
    hEmcSWeight = new TProfile( hname, htitle, nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, 0., 1.e12 );
    hEmcSWeight->SetXTitle( "log_{10} energy_{MC} [TeV]" );
    hEmcSWeight->SetYTitle( "spectral weight" );
    hisTreeList->Add( hEmcSWeight );

// histograms for energy reconstruction quality
    sprintf( hname, "hEsysRec" );
    hEsysRec = new TProfile( hname, htitle, nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, -1000., 1000., "s" );
    hEsysRec->SetXTitle( "energy_{rec} [TeV]" );
    hEsysRec->SetYTitle( "log_{10} E_{rec} - log_{10} E_{MC}" );
    hisTreeList->Add( hEsysRec );

    sprintf( hname, "hEsysMC" );
    hEsysMC = new TProfile( hname, htitle, nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, -1000., 1000., "s" );
    hEsysMC->SetXTitle( "energy_{MC} [TeV]" );
    hEsysMC->SetYTitle( "log_{10} E_{rec} - log_{10} E_{MC}" );
    hisTreeList->Add( hEsysMC );

    sprintf( hname, "hEsysMCRelative" );
    hEsysMCRelative = new TProfile( hname, htitle, nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, -1000., 1000., "s" );
    hEsysMCRelative->SetXTitle( "energy_{MC} [TeV]" );
    hEsysMCRelative->SetYTitle( "systematic error (E_{rec} - E_{MC})/E_{MC}" );
    hisTreeList->Add( hEsysMCRelative );

    sprintf( hname, "hEsys2D" );
    hEsys2D = new TH2D( hname, htitle, nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, 100, -0.98, 2.02 );
    hEsys2D->SetXTitle( "energy_{MC} [TeV]" );
    hEsys2D->SetYTitle( "log_{10} E_{rec} - log_{10} E_{MC}" );
    hisTreeList->Add( hEsys2D );

    sprintf( hname, "hEmcCut" );
    hEmcCut = new TH2D( hname, htitle, 2*nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue, 2*nbins, fEnergyAxis_minimum_defaultValue, fEnergyAxis_maximum_defaultValue );
    hEmcCut->SetXTitle( "energy_{MC} [TeV]" );
    hEmcCut->SetYTitle( "energy_{rec} [TeV]" );
    hisTreeList->Add( hEmcCut );

// individual cuts
    vector< string > iCutName;
    iCutName.push_back( "hEcutTrigger" );
    iCutName.push_back( "hEcutFiducialArea" );
    iCutName.push_back( "hEcutStereoQuality" );
    iCutName.push_back( "hEcutTelType" );
    iCutName.push_back( "hEcutDirection" );
    iCutName.push_back( "hEcutGammaHadron" );
    iCutName.push_back( "hEcutEnergyReconstruction" );

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
                                                  // effective area vs MC energy
    fEffArea->Branch( "eff", eff, "eff[nbins]/D" );
    fEffArea->Branch( "seff_L", seff_L, "seff_L[nbins]/D" );
    fEffArea->Branch( "seff_U", seff_U, "seff_U[nbins]/D" );
                                                  //
    fEffArea->Branch( "Rec_nbins", &Rec_nbins, "Rec_nbins/I" );
                                                  // log10( energy ) in [TeV]
    fEffArea->Branch( "Rec_e0", Rec_e0, "Rec_e0[Rec_nbins]/D" );
                                                  // effective area vs reconstructed energy
    fEffArea->Branch( "Rec_eff", Rec_eff, "Rec_eff[Rec_nbins]/D" );
    fEffArea->Branch( "Rec_seff_L", Rec_seff_L, "Rec_seff_L[Rec_nbins]/D" );
    fEffArea->Branch( "Rec_seff_U", Rec_seff_U, "Rec_seff_U[Rec_nbins]/D" );
    fEffArea->Branch( hisTreeList, 64000, 1 );
    fEffArea->SetMarkerStyle( 20 );

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
            if( hEmc ) iT_TH1D.push_back( (TH1D*)hEmc->Clone( hname ) );
            else       iT_TH1D.push_back( 0 );
        }
        hVEmc.push_back( iT_TH1D );

        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEcut_%d_%d", i, j );
            if( hEcut ) iT_TH1D.push_back( (TH1D*)hEcut->Clone( hname ) );
            else        iT_TH1D.push_back( 0 );
        }
        hVEcut.push_back( iT_TH1D );

        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEcutUW_%d_%d", i, j );
            if( hEcutUW ) iT_TH1D.push_back( (TH1D*)hEcutUW->Clone( hname ) );
            else          iT_TH1D.push_back( 0 );
        }
        hVEcutUW.push_back( iT_TH1D );

        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEcutLin_%d_%d", i, j );
            if( hEcutLin ) iT_TH1D.push_back( (TH1D*)hEcutLin->Clone( hname ) );
            else           iT_TH1D.push_back( 0 );
        }
        hVEcutLin.push_back( iT_TH1D );

       iT_TH1D.clear();
       for( unsigned int j = 0; j < fVMinAz.size(); j++ )
       {
           sprintf( hname, "hVEcut500_%d_%d", i, j );
           if( hEcut500 ) iT_TH1D.push_back( (TH1D*)hEcut500->Clone( hname ) );
           else        iT_TH1D.push_back( 0 );
       }
       hVEcut500.push_back( iT_TH1D );

        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEcutRec_%d_%d", i, j );
            if( hEcutRec ) iT_TH1D.push_back( (TH1D*)hEcutRec->Clone( hname ) );
            else           iT_TH1D.push_back( 0 );
        }
        hVEcutRec.push_back( iT_TH1D );

        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEcutRecUW_%d_%d", i, j );
            if( hEcutRecUW ) iT_TH1D.push_back( (TH1D*)hEcutRecUW->Clone( hname ) );
            else           iT_TH1D.push_back( 0 );
        }
        hVEcutRecUW.push_back( iT_TH1D );

        iT_TProfile.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEmcSWeight_%d_%d", i, j );
            if( hEmcSWeight ) iT_TProfile.push_back( (TProfile*)hEmcSWeight->Clone( hname ) );
            else              iT_TProfile.push_back( 0 );
        }
        hVEmcSWeight.push_back( iT_TProfile );

// histograms for energy reconstruction quality
        iT_TProfile.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEsysRec_%d_%d", i, j );
            if( hEsysRec ) iT_TProfile.push_back( (TProfile*)hEsysRec->Clone( hname ) );
            else           iT_TProfile.push_back( 0 );
        }
        hVEsysRec.push_back( iT_TProfile );

        iT_TProfile.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEsysMC_%d_%d", i, j );
            if( hEsysMC ) iT_TProfile.push_back( (TProfile*)hEsysMC->Clone( hname ) );
            else          iT_TProfile.push_back( 0 );
        }
        hVEsysMC.push_back( iT_TProfile );

        iT_TProfile.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEsysMCRelative_%d_%d", i, j );
            if( hEsysMCRelative ) iT_TProfile.push_back( (TProfile*)hEsysMCRelative->Clone( hname ) );
            else                  iT_TProfile.push_back( 0 );
        }
        hVEsysMCRelative.push_back( iT_TProfile );

        iT_TH2D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEsys2D_%d_%d", i, j );
            if( hEsys2D ) iT_TH2D.push_back( (TH2D*)hEsys2D->Clone( hname ) );
            else          iT_TH2D.push_back( 0 );
        }
        hVEsys2D.push_back( iT_TH2D );

        iT_TH2D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVEmcCut_%d_%d", i, j );
            if( hEmcCut ) iT_TH2D.push_back( (TH2D*)hEmcCut->Clone( hname ) );
            else          iT_TH2D.push_back( 0 );
        }
        hVEmcCut.push_back( iT_TH2D );
    }
}


/*!
 *
 *  CALLED TO USE EFFECTIVE AREAS
 *
 *  this constructor is called to GET the effective area from a file and calculate energy spectra
 *
 *  called from anasum
 *
 */
VEffectiveAreaCalculator::VEffectiveAreaCalculator( string iInputFile, double azmin, double azmax, double ipedvar, double iSpectralIndex, vector< double > iMCZe, int iSmoothIter, double iSmoothThreshold, bool iEffectiveAreaVsEnergyMC )
{
    reset();

// MC intervalls
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
    bEffectiveAreaVsEnergyMC = iEffectiveAreaVsEnergyMC;

// mean effective area
    gMeanEffectiveAreaEMC = new TGraphAsymmErrors(1);
    gMeanEffectiveAreaEMC->SetName( "gMeanEffectiveAreaEMC" );
    gMeanEffectiveAreaEMC->SetMarkerStyle( 20 );
    gMeanEffectiveAreaErec = new TGraphAsymmErrors(1);
    gMeanEffectiveAreaErec->SetName( "gMeanEffectiveAreaErec" );
    gMeanEffectiveAreaErec->SetMarkerStyle( 20 );
    gMeanEffectiveAreaErec->SetMarkerColor( 2 );
    gMeanEffectiveAreaErec->SetLineColor( 2 );

// current directory
    fGDirectory = gDirectory;

// test if input file with effective areas exists
    iInputFile = VUtilities::testFileLocation( iInputFile, "EffectiveAreas", true );
    if( iInputFile.size() == 0 ) exit( 0 );

    TFile fIn( iInputFile.c_str() );
    if( fIn.IsZombie() )
    {
        cout << "Error opening file with effective areas: " << iInputFile << endl;
        exit( 0 );
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
    if( getEffectiveAreasFromFitFunction( (TTree*)gDirectory->Get( "EffFit" ), azmin, azmax, iSpectralIndex ) )
    {
        bEffectiveAreasareFunctions = true;
    }
    else if( initializeEffectiveAreasFromHistograms( (TTree*)gDirectory->Get( "fEffArea" ), (TH1D*)gDirectory->Get( "hEmc" ), azmin, azmax, iSpectralIndex, ipedvar ) )
    {
        bEffectiveAreasareHistograms = true;
    }
    if( !bEffectiveAreasareHistograms && !bEffectiveAreasareFunctions )
    {
        cout << "VEffectiveAreaCalculator ERROR: no effective areas found" << endl;
        cout << "all energy spectra will be invalid" << endl;
        bNOFILE = true;
    }
    fIn.Close();

    if( fGDirectory ) fGDirectory->cd();
}


double VEffectiveAreaCalculator::getAzMean( double azmin, double azmax )
{
// mean azimuth angle
    double iAzMean = 0.;
    if( azmin > 120. && azmax < -120. ) azmax += 360.;
    else if( azmin < -150. && azmax > 120. ) azmin += 360.;

    iAzMean = 0.5 * (azmin + azmax );
    if( iAzMean > 180. ) iAzMean -= 360.;

    return iAzMean;
}


/*
 *  CALLED TO USE EFFECTIVE AREAS
 */
bool VEffectiveAreaCalculator::getEffectiveAreasFromFitFunction( TTree* iEffFit, double azmin, double azmax, double iSpectralIndex )
{
    if( !iEffFit ) return false;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// this is currently not functional
//
    cout << "EFFECTIVE AREA FROM FIT FUNCTON IS CURRENTLY NOT FUNCTIONAL. USE HISTOGRAMS." << endl;
    return false;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

    TF1 *ifEff = 0;
    int iAMC;
//
// mean azimuth angle
    double iAzMean = getAzMean( azmin, azmax );

    double TazMin, TazMax, index;
    iEffFit->SetBranchAddress("Ze", &ze );
    iEffFit->SetBranchAddress( "AzMin", &TazMin );
    iEffFit->SetBranchAddress( "AzMax", &TazMax );
    iEffFit->SetBranchAddress( "Index", &index );
    iEffFit->SetBranchAddress("fEff", &ifEff );
    iEffFit->SetBranchAddress("AMC", &iAMC );

    for( int i = 0; i < iEffFit->GetEntries(); i++ )
    {
        iEffFit->GetEntry( i );

        if( (int)bEffectiveAreaVsEnergyMC != iAMC )
        {
            if( bEffectiveAreaVsEnergyMC ) cout << "VEffectiveAreaCalculator::getEffectiveAreasFromFitFunction: expected effective area vs MC energy" << endl;
            else                           cout << "VEffectiveAreaCalculator::getEffectiveAreasFromFitFunction: expected effective area vs reconstructed energy" << endl;
            return false;
        }
        if( fabs( iSpectralIndex - index ) > 0.01 ) continue;

        if( fabs( TazMin ) > 5.e2 || fabs( TazMax ) > 5.e2 ) continue;

// test az bin
// expect bin like [135,-135]
        if( TazMin > TazMax )
        {
            if( iAzMean < TazMin && iAzMean > TazMax ) continue;
        }
        else
        {
            if( iAzMean < TazMin || iAzMean > TazMax ) continue;
        }
        cout <<  ",(" << ze << ", " << TazMin << " - " << TazMax << ")";

        fZe.push_back( ze );
        if( ifEff )
        {
            fEffAreaFitFunction.push_back( (TF1*)ifEff->Clone() );
            fGDirectory->Append( fEffAreaFitFunction.back() );
        }
    }
    cout << endl;
    cout << "\t use fitted function for effective areas (number of zenith angle intervalls: " << fZe.size() << ")" << endl;

    return true;
}


vector< double > VEffectiveAreaCalculator::interpolate_effectiveArea( double iV, double iVLower, double iVupper, vector< double > iElower, vector< double > iEupper, bool iCos )
{
    vector< double > i_temp;
    if( iElower.size() == iEupper.size() )
    {
        i_temp.assign( iElower.size(), 0. );
        for( unsigned int i = 0; i < iElower.size(); i++ )
        {
            i_temp[i] = interpolate_WL( iV, iVLower, iVupper, iElower[i], iEupper[i], iCos );
        }
        return i_temp;
    }

    return i_temp;
}


/*
 *
 *  CALLED TO USE EFFECTIVE AREAS
 *
 *
 */
bool VEffectiveAreaCalculator::initializeEffectiveAreasFromHistograms( TTree *iEffArea, TH1D* i_hEMC, double azmin, double azmax,
                                                      double iSpectralIndex, double ipedvar )
{
    if( !iEffArea ) return false;
    if( !i_hEMC )
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
    int i_az = 0;

////////////////////////////
// define input tree
    double TazMin, TazMax, index;
    double Tpedvar = 1.;
    TProfile *i_hEsysMCRelative = 0;
    iEffArea->SetBranchAddress( "az", &i_az );
    iEffArea->SetBranchAddress( "azMin", &TazMin );
    iEffArea->SetBranchAddress( "azMax", &TazMax );
    if( iEffArea->GetBranchStatus( "pedvar" ) ) iEffArea->SetBranchAddress( "pedvar", &Tpedvar );
    else Tpedvar = -1.;
    iEffArea->SetBranchAddress( "index", &index );
    iEffArea->SetBranchAddress( "ze", &ze );
    iEffArea->SetBranchAddress( "Woff", &fWoff );
    iEffArea->SetBranchAddress( "nbins", &nbins );
    iEffArea->SetBranchAddress( "e0", e0 );
    iEffArea->SetBranchAddress( "eff", eff );
    iEffArea->SetBranchAddress( "seff_L", seff_L );
    iEffArea->SetBranchAddress( "seff_U", seff_U );
    iEffArea->SetBranchAddress( "Rec_nbins", &Rec_nbins );
    iEffArea->SetBranchAddress( "Rec_e0", Rec_e0 );
    iEffArea->SetBranchAddress( "Rec_eff", Rec_eff );
    iEffArea->SetBranchAddress( "Rec_seff_L", Rec_seff_L );
    iEffArea->SetBranchAddress( "Rec_seff_U", Rec_seff_U );
    if( iEffArea->GetBranchStatus( "hEsysMCRelative" ) ) iEffArea->SetBranchAddress( "hEsysMCRelative", &i_hEsysMCRelative );
    if( iEffArea->GetEntries() == 0 ) return false;

////////////////////////////////////////////////////////////////////////////////////
// prepare the energy vectors
// (binning should be the same for all entries in the effective area tree)
////////////////////////////////////////////////////////////////////////////////////

    iEffArea->GetEntry( 0 );

    if( !i_hEMC )
    {
        cout << "VEffectiveAreaCalculator::initializeEffectiveAreasFromHistograms error: no effective area histogram found" << endl;
        return false;
    }

    fEff_E0.clear();
    fEff_E0.swap( fEff_E0 );
    for( int b = 1; b <= i_hEMC->GetNbinsX(); b++ ) fEff_E0.push_back( i_hEMC->GetBinCenter( b ) );
    fNBins = fEff_E0.size();

    fVMeanEffectiveAreaEMC.assign( i_hEMC->GetNbinsX(), 0. );
    fVMeanEffectiveAreaErec.assign( i_hEMC->GetNbinsX(), 0. );
    vector< double > i_temp_Eff( i_hEMC->GetNbinsX(), 0. );
    vector< double > i_temp_EffL( i_hEMC->GetNbinsX(), 0. );
    vector< double > i_temp_EffU( i_hEMC->GetNbinsX(), 0. );
    vector< double > i_temp_Esys;
    if( i_hEsysMCRelative ) i_temp_Esys.assign( i_hEsysMCRelative->GetNbinsX(), 0. );
    vector< double > i_temp_EsysE;
    if( i_hEsysMCRelative ) i_temp_EsysE.assign( i_hEsysMCRelative->GetNbinsX(), 0. );
    if( i_hEsysMCRelative )
    {
        for( int i = 1; i <= i_hEsysMCRelative->GetNbinsX(); i++ ) fEff_EsysMCRelative_EnergyAxis.push_back( i_hEsysMCRelative->GetXaxis()->GetBinCenter( i ) );
    }

    cout << "\t selecting effective areas for mean az " << iAzMean << " deg, spectral index ";
    cout << iSpectralIndex << ", noise level " << ipedvar << endl;
    cout << "\t\ttotal number of curves: " << iEffArea->GetEntries();
    cout << ", total number of bins on energy axis: " << fNBins << endl;

    fNMeanEffectiveAreaEMC = 0;
    if( gMeanEffectiveAreaEMC )
    {
        for( int i = 0; i < gMeanEffectiveAreaEMC->GetN(); i++ )
        {
            gMeanEffectiveAreaEMC->SetPoint( i, 0., 0. );
            gMeanEffectiveAreaEMC->SetPointError( i, 0., 0., 0., 0. );
        }
    }
    fNMeanEffectiveAreaErec = 0;
    if( gMeanEffectiveAreaErec )
    {
        for( int i = 0; i < gMeanEffectiveAreaErec->GetN(); i++ )
        {
            gMeanEffectiveAreaErec->SetPoint( i, 0., 0. );
            gMeanEffectiveAreaErec->SetPointError( i, 0., 0., 0., 0. );
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
    double i_index_index_selected = 0.;

    double iInvMax = 1.e5;
    int iIndexAz = 0;
    double iInvMean = 0.;

    bool fLotsOfPrintOuts = false;

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
// loop over all entries in effective area tree
// (not sure if this is really necessary, in the end a few entries are only needed)
    for( int i = 0; i < iEffArea->GetEntries(); i++ )
    {
        iEffArea->GetEntry( i );

// check binning of effective areas
        if( i_hEMC && (int)i_hEMC->GetNbinsX() != (int)fNBins )
        {
            cout << "VEffectiveAreaCalculator::initializeEffectiveAreasFromHistograms error: effective area curve with different binning";
            cout << " " << i_hEMC->GetNbinsX() << " " << fNBins << endl;
            cout << "abort reading effective area tree..." << endl;
            return false;
        }
///////////////////////////////////////////////////
// check the azimuth range
///////////////////////////////////////////////////

// expect a couple of az bins and then a last bin with the average azimuth bin; typically [-1000., 1000.]
// this is the sign to read effective areas
        if( fabs( TazMin ) > 5.e2 || fabs( TazMax ) > 5.e2 )
        {
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
            if( !i_ze_F )
            {
                for( unsigned int w = 0; w < fMCZe.size(); w++ )
                {
                    if( fabs( fMCZe[w] - ze ) < 2.0 )
                    {
                        fZe.push_back( fMCZe[w] );
                        break;
                    }
                }
                i_index_ze = fZe.size()-1;
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
                if( !i_woff_F )
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
                    if( !i_noise_F )
                    {
                        fEff_Noise[i_index_ze][i_index_woff].push_back( Tpedvar );
                        i_index_noise = fEff_Noise[i_index_ze][i_index_woff].size() -1;
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
                                i_index_index_selected = fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise][s];
                                break;
                            }
                        }
                        if( !i_index_F )
                        {
                            fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise].push_back( index );
                            i_index_index = fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise].size() - 1;
                            i_index_index_selected = fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise][i_index_index];
                        }
                    }
                    else
                    {
                        vector< double > itemp;
                        itemp.push_back( index );
                        fEff_SpectralIndex[i_index_ze][i_index_woff].push_back( itemp );
                        i_index_index = fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise].size() - 1;
                        i_index_index_selected = fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise][i_index_index];
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
                    i_index_index_selected = fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise][i_index_index];
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
                i_index_index_selected = fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise][i_index_index];
            }
            unsigned int i_ID = i_index_index + 100 * ( i_index_noise + 100 * ( i_index_woff + 100 * i_index_ze ) );
            if( fLotsOfPrintOuts )
	    {
	        cout << i_index_ze << " " << i_index_woff << " " << i_index_noise << " " << i_index_index << "\t" << i_ID << endl;
            }

///////////////////////////////////////////////////
// read effective area and load into maps
///////////////////////////////////////////////////
            if( nbins <= 0 )
            {
                cout << "WARNING : incomplete effective areas for id " << i_ID << endl;
		cout << i_index_index << "\t" << i_index_noise << "\t" << i_index_woff << "\t" << i_index_ze << endl;
                cout << "in bool VEffectiveAreaCalculator::initializeEffectiveAreasFromHistograms(";
		cout << "TTree *iEffArea, double azmin, double azmax, double iSpectralIndex )" << endl;
		cout << "Missing effective area:";
		if( i_index_ze < fZe.size() ) cout << " ze = " << fZe[i_index_ze] << " [deg],"; 
		if( i_index_ze < fEff_WobbleOffsets.size() && i_index_woff < fEff_WobbleOffsets[i_index_ze].size() )
		{
		   cout << " woff = " << fEff_WobbleOffsets[i_index_ze][i_index_woff] << " [deg]";
                }
		if( i_index_ze < fEff_Noise.size() && 
		    i_index_woff < fEff_Noise[i_index_ze].size() && 
		    i_index_noise < fEff_Noise[i_index_ze][i_index_woff].size() )
		{
		   cout << " noise = " << fEff_Noise[i_index_ze][i_index_woff][i_index_noise];
                }
		if( i_index_ze < fEff_SpectralIndex.size() && i_index_woff < fEff_SpectralIndex[i_index_ze].size() && 
		    i_index_noise < fEff_SpectralIndex[i_index_ze][i_index_woff].size() &&
		    i_index_index < fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise].size() )
		{
		   cout << " spectral index = " << fEff_SpectralIndex[i_index_ze][i_index_woff][i_index_noise][i_index_index];
                }
		cout << endl;
                cout << "please check if this falls into your parameter space" << endl;
            }
// effective areas vs MC energy
            for( unsigned int e = 0; e < fNBins; e++ )
            {
                i_temp_Eff[e] = 0.;
                i_temp_EffL[e] = 0.;
                i_temp_EffU[e] = 0.;
                for( int j = 0; j < nbins; j++ )
                {
                    if( TMath::Abs( e0[j] - fEff_E0[e] ) < 1.e-5 )
                    {
                        i_temp_Eff[e] = eff[j];
                        i_temp_EffL[e] = seff_L[j];
                        i_temp_EffU[e] = seff_U[j];
                    }
                }
            }
            fEffMC_Eff[i_ID] = i_temp_Eff;
            fEffMC_EffL[i_ID] = i_temp_EffL;
            fEffMC_EffU[i_ID] = i_temp_EffU;
// effective area vs reconstruction energy
            for( unsigned int e = 0; e < fNBins; e++ )
            {
                i_temp_Eff[e] = 0.;
                i_temp_EffL[e] = 0.;
                i_temp_EffU[e] = 0.;
                for( int j = 0; j < Rec_nbins; j++ )
                {
                    if( TMath::Abs( Rec_e0[j] - fEff_E0[e] ) < 1.e-5 )
                    {
                        i_temp_Eff[e] = Rec_eff[j];
                        i_temp_EffL[e] = Rec_seff_L[j];
                        i_temp_EffU[e] = Rec_seff_U[j];
                    }
                }
            }
            fEffRec_Eff[i_ID] = i_temp_Eff;
            fEffRec_EffL[i_ID] = i_temp_EffL;
            fEffRec_EffU[i_ID] = i_temp_EffU;

            if( i_hEsysMCRelative )
            {
                for( int ti = 1; ti <= i_hEsysMCRelative->GetNbinsX(); ti++ )
                {
                    i_temp_Esys[ti-1] = i_hEsysMCRelative->GetBinContent( ti );
                    i_temp_EsysE[ti-1] = i_hEsysMCRelative->GetBinError( ti );
                }
            }
            fEff_EsysMCRelative[i_ID] = i_temp_Esys;
            fEff_EsysMCRelativeE[i_ID] = i_temp_EsysE;

// this is neeeded only if there are no azimuth dependent effective areas
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
            if( iT > 180. ) iT = fabs( iT - 360. );
            if( iT < iInvMax )
            {
                iInvMax = iT;
                iIndexAz = i;
            }
            if( iAzMean < 0. && iAzMean > TazMax ) continue;
            else if( iAzMean > 0. && iAzMean < TazMin ) continue;
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
        exit( -1 );
    }
    cout << "\t use histograms for effective areas (number of zenith angle intervalls: " << fZe.size() << "; ";
    for( unsigned int i = 0; i < fZe.size()-1; i++ ) cout << fZe[i] << ", ";
    if( fZe.size() > 0 ) cout << fZe[fZe.size()-1];
    cout << ")" << endl;
    if( bEffectiveAreaVsEnergyMC ) cout << "\t (effective area vs MC energy)" << endl;
    else                           cout << "\t (effective area vs reconstructed energy)" << endl;

    if( fSmoothIter > 0 )
    {
        smoothEffectiveAreas( fEffRec_Eff );
        smoothEffectiveAreas( fEffMC_Eff );
    }

///////////////////////////////////////////////////
    if( fLotsOfPrintOuts )
    {
        cout << "ze size " << fZe.size() << endl;
        for( unsigned int z = 0; z < fZe.size(); z++ )
        {
            cout << "ze " << z << " " << fZe[z] << endl;
            if( z < fEff_WobbleOffsets.size() )
            {
                cout << "\t woff size " << fEff_WobbleOffsets[z].size() << endl;
                for( unsigned int w = 0; w < fEff_WobbleOffsets[z].size(); w++ )
                {
                    cout << "\t woff " << w << " " << fEff_WobbleOffsets[z][w] << endl;
                    if( z < fEff_Noise.size() && w < fEff_Noise[z].size() )
                    {
                        cout << "\t\t noise size " << fEff_Noise[z][w].size() << endl;
                        for( unsigned n = 0; n < fEff_Noise[z][w].size(); n++ )
                        {
                            cout << "\t\t noise " << n << " " << fEff_Noise[z][w][n] << endl;
                            if( z < fEff_SpectralIndex.size() && w < fEff_SpectralIndex[z].size() && n < fEff_SpectralIndex[z][w].size() )
                            {
                                cout << "\t\t\t index size " << fEff_SpectralIndex[z][w][n].size() << endl;
                                for( unsigned s = 0; s < fEff_SpectralIndex[z][w][n].size(); s++ )
                                {
                                    cout << "\t\t\t index " << s << " " << fEff_SpectralIndex[z][w][n][s] << endl;
                                    unsigned int i_ID = s + 100 * ( n + 100 * ( w + 100 * z ) );
                                    if( fEffRec_Eff.find( i_ID ) != fEffRec_Eff.end() ) cout << "\t\t\t map " << fEffRec_Eff[i_ID].size() << endl;
                                }
                            }
                        }
                    }
                }
            }
        }
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
        if( fEffAreaFitFunction[i] ) fEffAreaFitFunction[i]->Delete();
    }
    if( gMeanEffectiveAreaEMC ) delete gMeanEffectiveAreaEMC;
    if( gMeanEffectiveAreaErec ) delete gMeanEffectiveAreaErec;
    if( gMeanSystematicErrorGraph ) delete gMeanSystematicErrorGraph;
}


void VEffectiveAreaCalculator::reset()
{
    degrad = 45./ atan( 1. );
    raddeg = 1./degrad;

    fNBins = 0;
    gMeanEffectiveAreaEMC = 0;
    gMeanEffectiveAreaErec = 0;

    bNOFILE = true;
    fGDirectory = 0;

    fSpectralIndex = 2.0;

    fEnergyAxis_minimum_defaultValue = -2.;
    fEnergyAxis_maximum_defaultValue = 4.;

    fCuts = 0;

    fTNoise = 0;
    fTNoisePE = 0.;
    fTPedvar = 0.;

    fAzBin = 0;
    fMinAz = -1.e3;
    fMaxAz = 1.e3;
    bEffectiveAreaVsEnergyMC = false;

    hEmc = 0;
    hEcut = 0;
    hEcutUW = 0;
    hEcutLin = 0;
    hEcutRec = 0;
    hEcutRecUW = 0;
    hEcut500 = 0;
    gEffAreaMC = 0;
    gEffAreaRec = 0;
    gRecProb = 0;
    hEmcSWeight = 0;
    hEsysRec = 0;
    hEsysMC = 0;
    hEsysMCRelative = 0;
    hEsys2D = 0;
    hEmcCut = 0;
    fEffArea = 0;
    hisTreeList = 0;
    bEffectiveAreasareFunctions = false;
    bEffectiveAreasareHistograms = false;

    fEffArea = 0;
    ze = 0.;
    nbins = 60;
    Rec_nbins = 0;
    for( int i = 0; i < 1000; i++ )
    {
        e0[i] = 0.;
        eff[i] = 0.;
        seff_L[i] = 0.;
        seff_U[i] = 0.;
        Rec_e0[i] = 0.;
        Rec_eff[i] = 0.;
        Rec_seff_L[i] = 0.;
        Rec_seff_U[i] = 0.;
    }

    fEffectiveAreas_meanZe = 0.;
    fEffectiveAreas_meanWoff = 0.;
    fEffectiveAreas_meanPedVar = 0.;
    fEffectiveAreas_meanIndex = 0.;
    fEffectiveAreas_meanN = 0.;

    gMeanSystematicErrorGraph = 0;
}

bool VEffectiveAreaCalculator::getMonteCarloSpectra( VEffectiveAreaCalculatorMCHistograms *iMC_histo )
{
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
	        hVEmc[s][i_az] = (TH1D*)iMC_histo->getHistogram_Emc( i_az, s )->Clone( hname );
            }
	    else hVEmc[s][i_az] = 0;
          }
          if( s < hVEmcSWeight.size() && i_az < hVEmcSWeight[s].size() )
          {
            sprintf( hname, "hVEmcSWeight_%d_%d", s, i_az );
            if( iMC_histo->getHistogram_EmcWeight( i_az, s ) )
	    {
	       hVEmcSWeight[s][i_az] = (TProfile*)iMC_histo->getHistogram_EmcWeight( i_az, s )->Clone( hname );
            }
	    else hVEmcSWeight[s][i_az] = 0;
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
bool VEffectiveAreaCalculator::fill( TH1D *hE0mc, CData *d,
                                     VEffectiveAreaCalculatorMCHistograms *iMC_histo, unsigned int iMethod )
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

// do not require successfull energy reconstruction
    if( fIgnoreEnergyReconstruction ) iMethod = 100;

//////////////////////////////////////////////////////////////////
// total Monte Carlo core scatter area (depends on CORSIKA shower core scatter mode)
    double totarea = 0.;
    if( fScatterMode[ize] == "VIEWCONE" )
    {
        totarea = fAreaRadius[ize]*fAreaRadius[ize]*TMath::Pi();
    }
    else if( fScatterMode[ize] == "FLAT" )
    {
        totarea = fAreaRadius[ize]*fAreaRadius[ize]*TMath::Pi()*cos( fZe[ize]*raddeg );
    }
    else
    {
        cout << "VEffectiveAreaCalculator::fill ERROR: unknown CORSIKA scatter mode: " << fScatterMode[ize] << endl;
        return false;
    }

//////////////////////////////////////////////////////////////////
// print some run information
    cout << endl;
    cout << "calculating effective areas: " << endl;
    cout << "\t zenith angle: " << fZe[ize];
    cout << ", wobble offset (x,y): " << fXWobble[ize] << ", " << fYWobble[ize];
    cout << " (" << sqrt(fXWobble[ize]*fXWobble[ize]+fYWobble[ize]*fYWobble[ize]) << " deg)" << endl;
    cout << "\t noise level: " << fNoise[ize] << " (pedvar: " << fPedVar[ize] << ")" << endl;
    cout << "\t area (" << fScatterMode[ize] << ") [m]: " << totarea;
    cout << " (scatter radius " << fAreaRadius[ize] << " [m])" << endl;
    cout << "\t energy reconstruction method: " << iMethod << endl;
    if( fIsotropicArrivalDirections ) cout << "\t assuming isotropic arrival directions" << endl;

    cout << endl;
    if( fSpectralWeight ) fSpectralWeight->print();
    cout << endl;

// make sure that all data pointers exist
    if( !d || !iMC_histo ) return false;

// spectral weight
    double i_weight = 1.;
// reconstructed energy (TeV, log10)
    double eRec = 0.;
    double eRecLin = 0.;
// MC energy (TeV, log10)
    double eMC = 0.;

////////////////////////////////////////////////////////////////////////////
// get MC histograms
   if( !getMonteCarloSpectra( iMC_histo ) )
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
    cout << "\t total number of data events: " << d_nentries << endl;
    for( Long64_t i = 0; i < d_nentries; i++ )
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
        if( bDebugCuts ) cout << "#0 CUT MC " << fCuts->applyMCXYoffCut( d->MCxoff, d->MCyoff, false ) << endl;

        if( !fCuts->applyMCXYoffCut( d->MCxoff, d->MCyoff, true ) ) continue;

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
	    cout << "\t" << fCuts->applyStereoQualityCuts( iMethod, false, i, true) << endl;
         }

// apply fiducial area cuts
         if( !fCuts->applyInsideFiducialAreaCut( true ) ) continue;
         hEcutSub[1]->Fill( eMC, 1. );

// apply reconstruction quality cuts
         if( !fCuts->applyStereoQualityCuts( iMethod, true, i , true) ) continue;
         hEcutSub[2]->Fill( eMC, 1. );

// apply telescope type cut (e.g. for CTA simulations only)
         if( fCTA )
	 {
           if( bDebugCuts ) cout << "#2 Cut NTELType " << fCuts->applyTelTypeTest( false ) << endl;
	   if(!fCuts->applyTelTypeTest( true ) ) continue;
         }
         hEcutSub[3]->Fill( eMC, 1. );

//////////////////////////////////////
// apply direction cut
//    
// point source cut; use MC shower direction as reference direction
         if( !fIsotropicArrivalDirections )
         {
            if( !fCuts->applyDirectionCuts( iMethod, true ) ) continue;                  
         }
// background cut; use (0,0) as reference direction
// (command line option -d)
         else
         {
            if( !fCuts->applyDirectionCuts( iMethod, true, 0., 0. ) ) continue;          
         }
         hEcutSub[4]->Fill( eMC, 1. );

//////////////////////////////////////
// apply gamma hadron cuts
         if( bDebugCuts )
	 {
	    cout << "#3 CUT ISGAMMA " << fCuts->isGamma(i) << endl;
         }
         if( !fCuts->isGamma( i, true ) ) continue;
         hEcutSub[5]->Fill( eMC, 1. );
	 
// apply energy reconstruction quality cut
         if( !fIgnoreEnergyReconstruction )
         {
	    if( bDebugCuts ) cout << "#4 EnergyReconstructionQualityCuts " << fCuts->applyEnergyReconstructionQualityCuts( iMethod ) << endl;
            if( !fCuts->applyEnergyReconstructionQualityCuts( iMethod, true ) ) continue;
         }
         hEcutSub[6]->Fill( eMC, 1. );
// skip event if no energy has been reconstructed
// get energy according to which reconstruction method
         if( iMethod == 0 && d->Erec > 0. )
         {
             eRec = log10( d->Erec );
             eRecLin = d->Erec ;
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
	 else continue;
         
// loop over all az bins
         for( unsigned int i_az = 0; i_az < fVMinAz.size(); i_az++ )
         {
// check at what azimuth bin we are
             if( fZe[ize] > 3. )
             {
// confine MC az to -180., 180.
        	 if( d->MCaz > 180. ) d->MCaz -= 360.;
// expect bin like [135,-135]
        	 if( fVMinAz[i_az] > fVMaxAz[i_az] )
        	 {
        	     if( d->MCaz < fVMinAz[i_az] && d->MCaz > fVMaxAz[i_az] ) continue;
        	 }
// expect bin like [-135,-45.]
        	 else
        	 {
        	     if( d->MCaz < fVMinAz[i_az] || d->MCaz > fVMaxAz[i_az] ) continue;
        	 }
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
               else i_weight = 0.;
// fill true MC energy (hVEmc is in true MC energies)
               if( hVEcut[s][i_az] )           hVEcut[s][i_az]->Fill( eMC, i_weight );
               if( hVEcutUW[s][i_az] )         hVEcutUW[s][i_az]->Fill( eMC, 1. );
               if( hVEcut500[s][i_az] )        hVEcut500[s][i_az]->Fill( eMC, i_weight );
               if( hVEcutLin[s][i_az] )        hVEcutLin[s][i_az]->Fill( eMC, i_weight );
               if( hVEcutRec[s][i_az] )        hVEcutRec[s][i_az]->Fill( eRec, i_weight );
               if( hVEcutRecUW[s][i_az] )      hVEcutRecUW[s][i_az]->Fill( eRec, 1. );
               if( hVEsysRec[s][i_az] )        hVEsysRec[s][i_az]->Fill( eRec, eRec - eMC );
               if( hVEsysMC[s][i_az] )         hVEsysMC[s][i_az]->Fill( eMC, eRec - eMC );
               if( hVEsysMCRelative[s][i_az] ) hVEsysMCRelative[s][i_az]->Fill( eMC, ( eRecLin - d->MCe0 ) / d->MCe0 );
               if( hVEsys2D[s][i_az] )         hVEsys2D[s][i_az]->Fill( eMC, eRec - eMC );
               if( hVEmcCut[s][i_az] )         hVEmcCut[s][i_az]->Fill( eMC, eRec );
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
    fTNoisePE = (double)(fNoise[ize]) / 0.15 * 1.e9;
    fTPedvar = fPedVar[ize];
    fXoff = fXWobble[ize];
    fYoff = fYWobble[ize];
    fWoff = sqrt( fXoff*fXoff + fYoff*fYoff );

// loop over all spectral index
    for( unsigned int s = 0; s < fVSpectralIndex.size(); s++ )
    {
        fSpectralIndex = fVSpectralIndex[s];
// loop over all az bins
        for( unsigned int i_az = 0; i_az < fVMinAz.size(); i_az++ )
        {
            fAzBin = (int)i_az;
            fMinAz = fVMinAz[i_az];
            fMaxAz = fVMaxAz[i_az];

// bayesdivide works only for weights == 1
// errors might be wrong, since histograms are filled with weights != 1
            if( !binomialDivide( gEffAreaMC, hVEcut[s][i_az], hVEmc[s][i_az] ) )
	    {
	       cout << "VEffectiveAreaCalculator::fill: error calculating effective area vs MC energy" << endl;
	       cout << "s : " << s << " , az: " << i_az << endl;
            }
            if( !binomialDivide( gRecProb, hVEcut[s][i_az], hVEmc[s][i_az] ) )
	    {
	       cout << "VEffectiveAreaCalculator::fill: error calculating effective area vs prob energy" << endl;
	       cout << "s : " << s << " , az: " << i_az << endl;
            }
            if( !binomialDivide( gEffAreaRec, hVEcutRec[s][i_az], hVEmc[s][i_az] ) )
	    {
	       cout << "VEffectiveAreaCalculator::fill: error calculating effective area vs rec energy" << endl;
	       cout << "s : " << s << " , az: " << i_az << endl;
            }

            nbins = gEffAreaMC->GetN();

            for( int i = 0; i < 1000; i++ )
            {
                e0[i] = 0.;
                eff[i] = 0.;
                seff_L[i] = 0.;
                seff_U[i] = 0.;
                Rec_e0[i] = 0.;
                Rec_eff[i] = 0.;
                Rec_seff_L[i] = 0.;
                Rec_seff_U[i] = 0.;
            }
            double x = 0.;
	    double y = 0.;
            double iSf = 1.;
            for( int i = 0; i < nbins; i++ )
            {
                gEffAreaMC->GetPoint( i, x, y );
                e0[i] = x;
                iSf = totarea;
                eff[i] = y * iSf;
                seff_L[i] = gEffAreaMC->GetErrorYlow( i )*iSf;
                seff_U[i] = gEffAreaMC->GetErrorYhigh( i )*iSf;
                gEffAreaMC->SetPoint( i, x, eff[i] );
                gEffAreaMC->SetPointEYlow( i, seff_L[i] );
                gEffAreaMC->SetPointEYhigh( i, seff_U[i] );
            }

            Rec_nbins = gEffAreaRec->GetN();
            for( int i = 0; i < Rec_nbins; i++ )
            {
                gEffAreaRec->GetPoint( i, x, y );
                Rec_e0[i] = x;
// this is an approximation, since scatter area is defined over E_MC (GM: don't understand this comment)
                iSf = totarea;
                Rec_eff[i] = y * iSf;
                Rec_seff_L[i] = gEffAreaRec->GetErrorYlow( i )*iSf;
                Rec_seff_U[i] = gEffAreaRec->GetErrorYhigh( i )*iSf;
                gEffAreaRec->SetPoint( i, x, Rec_eff[i] );
                gEffAreaRec->SetPointEYlow( i, Rec_seff_L[i] );
                gEffAreaRec->SetPointEYhigh( i, Rec_seff_U[i] );
            }
            resetHistograms( ize );

// copy all histograms
            copyHistograms( hEmc, hVEmc[s][i_az], false );
            copyHistograms( hEcut, hVEcut[s][i_az], false );
            copyHistograms( hEcutUW, hVEcutUW[s][i_az], false );
            copyHistograms( hEcut500, hVEcut500[s][i_az], false );
            copyHistograms( hEcutLin, hVEcutLin[s][i_az], false );
            copyHistograms( hEcutRec, hVEcutRec[s][i_az], false );
            copyHistograms( hEcutRecUW, hVEcutRecUW[s][i_az], false );
            copyProfileHistograms( hEmcSWeight, hVEmcSWeight[s][i_az], "" );
            copyProfileHistograms( hEsysRec,  hVEsysRec[s][i_az], "s" );
            copyProfileHistograms( hEsysMC, hVEsysMC[s][i_az], "s" );
            copyProfileHistograms( hEsysMCRelative, hVEsysMCRelative[s][i_az], "s" );
            copyHistograms( hEsys2D, hVEsys2D[s][i_az], true );
            copyHistograms( hEmcCut, hVEmcCut[s][i_az], true );

            fEffArea->Fill();
        }
    }

    fCuts->printCutStatistics();

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
double VEffectiveAreaCalculator::getEffectiveArea( double erec, double ze, double woff, double iPedVar, double iSpectralIndex, bool bAddtoMeanEffectiveArea, bool iEffectiveAreaVsEnergyMC )
{
    if( bNOFILE ) return 1.;

///////////////////////////////////////////
// read effective areas from histograms
// (this is the default case)
///////////////////////////////////////////
    if( bEffectiveAreasareHistograms ) return getEffectiveAreasFromHistograms( erec, ze, woff, iPedVar, iSpectralIndex, bAddtoMeanEffectiveArea, iEffectiveAreaVsEnergyMC );

///////////////////////////////////////////
// read effective areas from functions
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
        else if( ze >= fZe[fZe.size()-1] )
        {
            ize_low = ize_up = fZe.size()-1;
        }
        else
        {
            for( unsigned int i = 0; i < fZe.size(); i++ )
            {
                if( ze > fZe[i] )
                {
                    ize_low = (int)i;
                    ize_up = (int)i+1;
                }
            }
        }
        double ie_zelow = 0.;
        double ie_zeup = 0.;
        if( bEffectiveAreasareFunctions )  getEffectiveAreasFromFitFunction( ize_low, ize_up, lerec, ie_zelow, ie_zeup );
        else
        {
            ie_zelow = 0.;
            ie_zeup =  0.;
        }

// interpolate between zenith angles (weighted by cos(ze))
        double ieff = interpolate_WL( ze, fZe[ize_low], fZe[ize_up], ie_zelow, ie_zeup, true );

// return inverse
        if( ieff != 0. ) return 1./ieff;
    }

    return 1.;
}


/*!
 *  CALLED TO USE EFFECTIVE AREAS
 */
void VEffectiveAreaCalculator::getEffectiveAreasFromFitFunction( unsigned int ize_low, unsigned int ize_up, double lerec, double &ie_zelow, double &ie_zeup )
{
    if( ize_low < fEffAreaFitFunction.size() && fEffAreaFitFunction[ize_low] ) ie_zelow = fEffAreaFitFunction[ize_low]->Eval( lerec );
    if( ize_up  < fEffAreaFitFunction.size() && fEffAreaFitFunction[ize_up]  ) ie_zeup  = fEffAreaFitFunction[ize_up]->Eval( lerec );
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
double VEffectiveAreaCalculator::getEffectiveAreasFromHistograms( double erec, double ze, double woff, double iPedVar, double iSpectralIndex, bool bAddtoMeanEffectiveArea, bool iEffectiveAreaVsEnergyMC )
{
    vector< double > i_eff_temp( fNBins, 0. );

// log10 of energy
    if( erec <= 0. ) return 0.;
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
    for( unsigned int i = 0; i < i_ze_bins.size(); i++ )
    {
////////////////////////////////////////////////////////
// get lower and upper wobble offset bins
////////////////////////////////////////////////////////
        if( i_ze_bins[i] < fEff_WobbleOffsets.size() )
        {
            vector< unsigned int > i_woff_bins = getUpperLowBins( fEff_WobbleOffsets[i_ze_bins[i]], woff );
            vector< vector< double > > i_woff_eff_temp( 2, i_eff_temp );
            for( unsigned int w = 0; w < i_woff_bins.size(); w++ )
            {
////////////////////////////////////////////////////////
// get lower and upper noise bins
////////////////////////////////////////////////////////
                if( i_ze_bins[i] < fEff_Noise.size() && i_woff_bins[w] < fEff_Noise[i_ze_bins[i]].size() )
                {
                    vector< unsigned int > i_noise_bins = getUpperLowBins( fEff_Noise[i_ze_bins[i]][i_woff_bins[w]], iPedVar );
                    vector< vector< double > > i_noise_eff_temp( 2, i_eff_temp );
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
                            if( iEffectiveAreaVsEnergyMC )
                            {
                                i_noise_eff_temp[n] = interpolate_effectiveArea( iSpectralIndex, fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[n]][i_index_bins[0]], fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[n]][i_index_bins[1]], fEffMC_Eff[i_ID_0], fEffMC_Eff[i_ID_1], false );
                            }
                            else
                            {
                                i_noise_eff_temp[n] = interpolate_effectiveArea( iSpectralIndex, fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[n]][i_index_bins[0]], fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[n]][i_index_bins[1]], fEffRec_Eff[i_ID_0], fEffRec_Eff[i_ID_1], false );
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
                            return 0.;
                        }
////////////////////////////////////////////////////////
                    }
                    i_woff_eff_temp[w] = interpolate_effectiveArea( iPedVar, fEff_Noise[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[0]], fEff_Noise[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[1]], i_noise_eff_temp[0], i_noise_eff_temp[1], false );
                }
                else
                {
                    cout << "VEffectiveAreaCalculator::getEffectiveAreasFromHistograms error: noise index out of range: " << i_ze_bins[i] << " " << fEff_Noise.size();
                    if( i_ze_bins[i] < fEff_Noise.size() ) cout << " " << i_woff_bins[w] << " " << fEff_Noise[i_ze_bins[i]].size() << endl;
                    cout << endl;
                    return 0.;
                }
            }
            i_ze_eff_temp[i] = interpolate_effectiveArea( woff, fEff_WobbleOffsets[i_ze_bins[i]][i_woff_bins[0]], fEff_WobbleOffsets[i_ze_bins[i]][i_woff_bins[1]], i_woff_eff_temp[0], i_woff_eff_temp[1], false );
        }
        else
        {
            cout << "VEffectiveAreaCalculator::getEffectiveAreasFromHistograms error: woff index out of range: " << i_ze_bins[i] << " " << fEff_WobbleOffsets.size() << endl;
            return 0.;
        }
    }
    i_eff_temp = interpolate_effectiveArea( ze, fZe[i_ze_bins[0]], fZe[i_ze_bins[1]], i_ze_eff_temp[0], i_ze_eff_temp[1], true );

    if( fEff_E0.size() == 0 ) return 1.;

// mean effective area calculation
    if( bAddtoMeanEffectiveArea && iEffectiveAreaVsEnergyMC && fVMeanEffectiveAreaEMC.size() == i_eff_temp.size() )
    {
        for( unsigned int i = 0; i < i_eff_temp.size(); i++ )
        {
            fVMeanEffectiveAreaEMC[i] += i_eff_temp[i];
        }
        fNMeanEffectiveAreaEMC++;
    }
    if( bAddtoMeanEffectiveArea && !iEffectiveAreaVsEnergyMC && fVMeanEffectiveAreaErec.size() == i_eff_temp.size() )
    {
        for( unsigned int i = 0; i < i_eff_temp.size(); i++ )
        {
            fVMeanEffectiveAreaErec[i] += i_eff_temp[i];
        }
        fNMeanEffectiveAreaErec++;
    }

/////////////////////////////////////////
// effective area for a specific energy
/////////////////////////////////////////
    double i_eff_e = 1.;
    unsigned int ie0_low = 0;
    unsigned int ie0_up = 0;

    if( lerec <= fEff_E0[0] ) ie0_low = ie0_up = 0;
    else if( lerec > fEff_E0[fEff_E0.size()-1] ) ie0_low = ie0_up = fEff_E0.size()-1;
    else
    {
        for( unsigned int j = 1; j < fEff_E0.size()-1; j++ )
        {
            if( lerec > fEff_E0[j] )
            {
                ie0_low = j;
                ie0_up = j+1;
            }
        }
    }

///////////////////////////////////
// linear interpolate between energies
///////////////////////////////////
    if( fEff_E0[ie0_up] != fEff_E0[ie0_low] )
    {
        i_eff_e  = ( fEff_E0[ie0_up] - lerec )  /  ( fEff_E0[ie0_up] - fEff_E0[ie0_low] ) * i_eff_temp[ie0_low];
        i_eff_e += ( lerec - fEff_E0[ie0_low] ) /  ( fEff_E0[ie0_up] - fEff_E0[ie0_low] ) * i_eff_temp[ie0_up];
    }
    else i_eff_e = 0.5*( i_eff_temp[ie0_low] + i_eff_temp[ie0_up] );

    if( i_eff_e > 0. ) return 1./i_eff_e;

    return 1.;
}


/*!
 *  CALLED TO USE EFFECTIVE AREAS

    interpolate between two values

    ze [deg]

    weighted by cos ze
*/
double VEffectiveAreaCalculator::interpolate_WL( double ze, double ze1, double ze2, double w1, double w2, bool iCos )
{
// don't interpolate if one or two values are not valid
    if( w1 < -90. || w2 < -90. ) return -99.;

// same zenith angle, don't interpolate
    if( fabs( ze1 - ze2 ) < 1.e-3 ) return (w1+w2)/2.;

// interpolate
    double id, f1, f2;
    if( iCos )
    {
        id = cos( ze1*raddeg ) - cos( ze2*raddeg );
        f1 = 1. - (cos( ze1*raddeg ) - cos( ze*raddeg )) / id;
        f2 = 1. - (cos( ze*raddeg ) - cos( ze2*raddeg )) / id;
    }
    else
    {
        id = ze1 - ze2;
        f1 = 1. - ( ze1 - ze ) / id;
        f2 = 1. - ( ze  - ze2 ) / id;
    }

    return (w1*f1+w2*f2);
}


/*
 *  CALLED FOR CALCULATION OF EFFECTIVE AREAS
 */
void VEffectiveAreaCalculator::resetHistograms( unsigned int ize )
{
    if( ize >= fZe.size() ) return;

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

    sprintf( htitle, "trigger/rec. probability (%.1f deg)", fZe[ize] );
    gRecProb->SetTitle( htitle );

    hEsysRec->Reset();
    sprintf( htitle, "energy reconstruction (%.1f deg)", fZe[ize] );
    hEsysRec->SetTitle( htitle );

    hEsysMC->Reset();
    sprintf( htitle, "energy reconstruction (%.1f deg)", fZe[ize] );
    hEsysMC->SetTitle( htitle );

    hEsysMCRelative->Reset();
    sprintf( htitle, "energy reconstruction (%.1f deg)", fZe[ize] );
    hEsysMCRelative->SetTitle( htitle );

    hEsys2D->Reset();
    sprintf( htitle, "energy reconstruction, distributions (%.1f deg)", fZe[ize] );
    hEsys2D->SetTitle( htitle );

    hEmcCut->Reset();
    sprintf( htitle, "reconstructed energy vs. MC energy (%.1f deg)", fZe[ize] );
    hEmcCut->SetTitle( htitle );

    hEmcSWeight->Reset();
    sprintf( htitle, "spectral weights (%.1f deg)", fZe[ize] );
    hEmcSWeight->SetTitle( htitle );

}


void VEffectiveAreaCalculator::setWobbleOffset( double x, double y )
{
    if( fXWobble.size() == 0 ) fXWobble.push_back( x );
    else                       fXWobble[0] = x;
    if( fYWobble.size() == 0 ) fYWobble.push_back( y );
    else                       fYWobble[0] = y;

}

void VEffectiveAreaCalculator::setNoiseLevel( int iN, double iP )
{
   if( fNoise.size() == 0 ) fNoise.push_back( iN );
   else                     fNoise[0] = iN;

   if( fPedVar.size() == 0 ) fPedVar.push_back( iP );
   else                      fPedVar[0] = iP;
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


bool VEffectiveAreaCalculator::binomialDivide( TGraphAsymmErrors *g, TH1D *hrec, TH1D *hmc )
{
    if( !g )
    {
       cout << "VEffectiveAreaCalculator::binomialDivide error: no return graph given" << endl;
       return false;
    }
    if( !hrec )
    {
       cout << "VEffectiveAreaCalculator::binomialDivide error: no histogram with reconstructed events given" << endl;
       return false;
    }
    if( !hmc )
    {
       cout << "VEffectiveAreaCalculator::binomialDivide error: no histogram with simulated events given" << endl;
       return false;
    }

    int z = 0;
    double pj = 0.;
    double pr = 0.;
    double pm = 0.;
    double sj = 0.;

    for( int b = 1; b <= hmc->GetNbinsX(); b++ )
    {
        if( hmc->GetBinContent( b ) > 0 && hrec->GetBinContent( b ) > 0 )
        {
            pj = hrec->GetBinContent( b ) / hmc->GetBinContent( b );
            pr = hrec->GetBinError( b );
            pm = hmc->GetBinError( b );
            if( pj != 1. )
	    {
	       sj = TMath::Abs( ( ( 1.-2.*pj)*pr*pr + pj*pj*pm*pm)/(hmc->GetBinContent( b )*hmc->GetBinContent( b )) );
            }
            else           sj = 0.;
            sj = sqrt( sj );

            g->SetPoint( z, hmc->GetBinCenter( b ), pj );
            g->SetPointError( z, 0., 0., sj, sj );
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

    for( CI p = m.begin(); p!= m.end(); ++ p )
    {
        vector< double > itemp = p->second;

        for( int l = 0; l < fSmoothIter; l++ )
        {
            for( unsigned int k = 1; k < itemp.size() - 1; k++ )
            {
                if( itemp[k-1] <= 0 || itemp[k] <= 0. || itemp[k+1] <= 0. ) continue;

// upwards outlier
                if( itemp[k] / itemp[k-1] > (1.+fSmoothThreshold) && itemp[k] / itemp[k+1] > (1.+fSmoothThreshold) )
                {
                    itemp[k] = 0.5 * ( itemp[k-1] + itemp[k+1] );
                }

// downwards outlier
                if( itemp[k] / itemp[k-1] < (1.-fSmoothThreshold) && itemp[k] / itemp[k+1] < (1.-fSmoothThreshold) )
                {
                    itemp[k] = 0.5 * ( itemp[k-1] + itemp[k+1] );
                }
            }
        }
    }
}


void VEffectiveAreaCalculator::copyProfileHistograms( TProfile *h1,  TProfile*h2, string iopt )
{
    if( !h1 || !h2 ) return;

    string iEOption = h1->GetErrorOption();

    for( int b = 0; b <= h2->GetNbinsX(); b++ )
    {
        h1->SetBinContent( b, h2->GetBinContent( b ) * h2->GetBinEntries( b ) );

        if( TMath::Abs( h2->GetBinError( b ) ) < 1.e-4 ) h1->SetBinError( b, 0 );
        else
        {
            if( h2->GetBinEntries( b ) > 0. )
            {
                double iE = h2->GetBinError(b);
                if( iEOption != "s" ) iE = h2->GetBinError(b) * sqrt( h2->GetBinEntries( b ) );

                h1->SetBinError( b,  sqrt( h2->GetBinEntries( b ) * ( h2->GetBinContent( b ) *  h2->GetBinContent( b ) + iE*iE ) ) );
            }
            else h1->SetBinError( b, 0. );
        }
        h1->SetBinEntries( b, h2->GetBinEntries( b ) );
    }
    h1->SetEntries( h2->GetEntries() );
}


void VEffectiveAreaCalculator::copyHistograms( TH1 *h1,  TH1 *h2, bool b2D )
{
    if( !h1 || !h2 ) return;

    if( !b2D )
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
TGraphAsymmErrors* VEffectiveAreaCalculator::getMeanEffectiveArea( bool iEffectiveAreaVsEnergyMC )
{
    TGraphAsymmErrors *g = 0;
    unsigned int n = 0;
    vector< double > v;
    if( iEffectiveAreaVsEnergyMC )
    {
        g = gMeanEffectiveAreaEMC;
        n = fNMeanEffectiveAreaEMC;
        v = fVMeanEffectiveAreaEMC;
    }
    else
    {
        g = gMeanEffectiveAreaErec;
        n = fNMeanEffectiveAreaErec;
        v = fVMeanEffectiveAreaErec;
    }

    if( g && n > 0 )
    {
        for( int i = 0; i < g->GetN(); i++ )
        {
            g->SetPoint( i, 0., 0. );
            g->SetPointError( i, 0., 0., 0., 0. );
        }
        int z = 0;
        for( unsigned int i = 0; i < v.size(); i++ )
        {
            if( v[i] > 0. )
            {
                g->SetPoint( z, fEff_E0[i], v[i] / n );
                z++;
            }
        }
        return g;
    }

    return 0;
}


void VEffectiveAreaCalculator::resetHistogramsVectors( unsigned int ize )
{
    for( unsigned int i = 0; i < hVEmc.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEmc[i].size(); j++ )
        {
            if( hVEmc[i][j] ) hVEmc[i][j]->Reset();
        }
    }
    for( unsigned int i = 0; i < hVEcut.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEcut[i].size(); j++ )
        {
            if( hVEcut[i][j] ) hVEcut[i][j]->Reset();
        }
    }
    for( unsigned int i = 0; i < hVEcutUW.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEcutUW[i].size(); j++ )
        {
            if( hVEcutUW[i][j] ) hVEcutUW[i][j]->Reset();
        }
    }
    for( unsigned int i = 0; i < hVEcutLin.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEcutLin[i].size(); j++ )
        {
            if( hVEcutLin[i][j] ) hVEcutLin[i][j]->Reset();
        }
    }
    for( unsigned int i = 0; i < hVEcut500.size(); i++ )
    {
       for( unsigned int j = 0; j < hVEcut500[i].size(); j++ )
       {
          if( hVEcut500[i][j] ) hVEcut500[i][j]->Reset();
       }
    }
    for( unsigned int i = 0; i < hVEcutRec.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEcutRec[i].size(); j++ )
        {
            if( hVEcutRec[i][j] ) hVEcutRec[i][j]->Reset();
        }
    }
    for( unsigned int i = 0; i < hVEcutRecUW.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEcutRecUW[i].size(); j++ )
        {
            if( hVEcutRecUW[i][j] ) hVEcutRecUW[i][j]->Reset();
        }
    }
    for( unsigned int i = 0; i < hVEmcSWeight.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEmcSWeight[i].size(); j++ )
        {
            if( hVEmcSWeight[i][j] ) hVEmcSWeight[i][j]->Reset();
        }
    }
    for( unsigned int i = 0; i < hVEsysRec.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEsysRec[i].size(); j++ )
        {
            if( hVEsysRec[i][j] ) hVEsysRec[i][j]->Reset();
        }
    }
    for( unsigned int i = 0; i < hVEsysMC.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEsysMC[i].size(); j++ )
        {
            if( hVEsysMC[i][j] ) hVEsysMC[i][j]->Reset();
        }
    }
    for( unsigned int i = 0; i < hVEsysMCRelative.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEsysMCRelative[i].size(); j++ )
        {
            if( hVEsysMCRelative[i][j] ) hVEsysMCRelative[i][j]->Reset();
        }
    }
    for( unsigned int i = 0; i < hVEsys2D.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEsys2D[i].size(); j++ )
        {
            if( hVEsys2D[i][j] ) hVEsys2D[i][j]->Reset();
        }
    }
    for( unsigned int i = 0; i < hVEmcCut.size(); i++ )
    {
        for( unsigned int j = 0; j < hVEmcCut[i].size(); j++ )
        {
            if( hVEmcCut[i][j] ) hVEmcCut[i][j]->Reset();
        }
    }
}


TH1D* VEffectiveAreaCalculator::getHistogramhEmc()
{
    if( !hEmc ) return 0;

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
    vector< double > i_eff_tempE;

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
        if( i_ze_bins[i] < fEff_WobbleOffsets.size() )
        {
            vector< unsigned int > i_woff_bins = getUpperLowBins( fEff_WobbleOffsets[i_ze_bins[i]], fEffectiveAreas_meanWoff );
            vector< vector< double > > i_woff_eff_temp( 2, hX );
            for( unsigned int w = 0; w < i_woff_bins.size(); w++ )
            {
////////////////////////////////////////////////////////
// get lower and upper noise bins
////////////////////////////////////////////////////////
                if( i_ze_bins[i] < fEff_Noise.size() && i_woff_bins[w] < fEff_Noise[i_ze_bins[i]].size() )
                {
                    vector< unsigned int > i_noise_bins = getUpperLowBins( fEff_Noise[i_ze_bins[i]][i_woff_bins[w]], fEffectiveAreas_meanPedVar );
                    vector< vector< double > > i_noise_eff_temp( 2, hX );
                    for( unsigned int n = 0; n < i_noise_bins.size(); n++ )
                    {
////////////////////////////////////////////////////////
// get lower and upper spectral index bins
////////////////////////////////////////////////////////
                        if( i_ze_bins[i] < fEff_SpectralIndex.size() && i_woff_bins[w] < fEff_SpectralIndex[i_ze_bins[i]].size() && i_noise_bins[n] < fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]].size() )
                        {
                            vector< unsigned int > i_index_bins = getUpperLowBins( fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[n]], fEffectiveAreas_meanIndex );
                            unsigned int i_ID_0 = i_index_bins[0] + 100 * ( i_noise_bins[n] + 100 * ( i_woff_bins[w] + 100 * i_ze_bins[i] ) );
                            unsigned int i_ID_1 = i_index_bins[1] + 100 * ( i_noise_bins[n] + 100 * ( i_woff_bins[w] + 100 * i_ze_bins[i] ) );
                            i_noise_eff_temp[n] = interpolate_effectiveArea( fEffectiveAreas_meanIndex, fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[n]][i_index_bins[0]], fEff_SpectralIndex[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[n]][i_index_bins[1]], fEff_EsysMCRelative[i_ID_0], fEff_EsysMCRelative[i_ID_1], false );
// don't interpolate errors, assume they are more or less constant
                            i_eff_tempE = fEff_EsysMCRelativeE[i_ID_0];

                        }
                    }
                    i_woff_eff_temp[w] = interpolate_effectiveArea( fEffectiveAreas_meanPedVar, fEff_Noise[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[0]], fEff_Noise[i_ze_bins[i]][i_woff_bins[w]][i_noise_bins[1]], i_noise_eff_temp[0], i_noise_eff_temp[1], false );
                }
            }
            i_ze_eff_temp[i] = interpolate_effectiveArea( fEffectiveAreas_meanWoff, fEff_WobbleOffsets[i_ze_bins[i]][i_woff_bins[0]], fEff_WobbleOffsets[i_ze_bins[i]][i_woff_bins[1]], i_woff_eff_temp[0], i_woff_eff_temp[1], false );
        }
    }
    i_eff_temp = interpolate_effectiveArea( fEffectiveAreas_meanZe, fZe[i_ze_bins[0]], fZe[i_ze_bins[1]], i_ze_eff_temp[0], i_ze_eff_temp[1], true );
    if( fEff_EsysMCRelative_EnergyAxis.size() == i_eff_temp.size() )
    {
        unsigned int z = 0;
        for( unsigned int i = 0; i < i_eff_temp.size(); i++ )
        {
            if( i_eff_tempE[i] > 0. )
            {
                gMeanSystematicErrorGraph->SetPoint( z, fEff_EsysMCRelative_EnergyAxis[i], i_eff_temp[i] );
                gMeanSystematicErrorGraph->SetPointError( z, 0, i_eff_tempE[i] );
                z++;
            }
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
