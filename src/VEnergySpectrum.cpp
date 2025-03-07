/*! class VEnergySpectrum
    analyse and plot spectral energy distributions


*/

#include "VEnergySpectrum.h"

VEnergySpectrum::VEnergySpectrum()
{
    fDebug = 0;

    bAsciiDataFile = false;

    fDataSetName = "E";
    fTotalRun = -1;

    initializeRunVariables();
}

VEnergySpectrum::VEnergySpectrum( string iFile, string iname, int irun, bool iSourceTypeIsAscii )
{
    fDebug = 0;
    bAsciiDataFile = iSourceTypeIsAscii;

    fDataSetName = iname;
    fTotalRun = irun;

    initializeRunVariables();

    openDataFile( iFile, fTotalRun, bAsciiDataFile );
}

bool VEnergySpectrum::openDataFile( string iFile, int irun, bool iSourceTypeIsAscii )
{
    fTotalRun = irun;
    bAsciiDataFile = iSourceTypeIsAscii;
    setConfidenceLevel( 0.68 );

    // open anasum file
    if(!bAsciiDataFile )
    {
        if(!openFile( iFile, fTotalRun ) )
        {
            bZombie = true;
            return false;
        }
    }
    else
    {
        if(!openAsciiFile( iFile ) )
        {
            bZombie = true;
            return false;
        }
    }
    return true;
}


void VEnergySpectrum::initializeRunVariables()
{

    nRebinner = 0;
    bUseRebinner = false;

    bCombineRuns = false;

    bZombie = false;

    hErecCountsOn = 0;
    hErecCountsOff = 0;
    hErecTotalTime = 0;
    hErecTotalTimeDeadTimeCorrected = 0;
    hEffArea = 0;

    fAnalysisEnergyBinning = -1.;
    bEnergyAxisLinear = false;

    fEnergyThresholdFixedValue = 0;
    fEnergyThresholdFileName = "";

    fTotalNormalisationFactor = 0.;
    fTotalObservationTime = 0.;
    fTotalObservationTimeDeadTimeCorrected = 0.;

    fAnalysisMinEnergy = 0.;
    fAnalysisMaxEnergy = 1.e10;

    // default values
    setAddHistogramParameters();
    setEnergyBinning();
    setSignificanceParameters();
    setEnergyThresholdDefinition();
    setErrorCalculationMethod();
    setOffsetdistance();

    // set default fitting parameters
    setSpectralFitFunction();
    setSpectralFitFluxNormalisationEnergy();
    setSpectralFitRangeLin();
    setSpectralFitPlottingStyle();


    // set some plotting parameters
    setPlottingSpectralWeightForBinCenter();
    setEnergyInBinDefinition();
    fPlottingCanvas = 0;
    setPlottingYaxis();
    setPlottingMultiplierIndex();
    setPlottingEnergyRangeLog();
    setPlottingLogEnergyAxis();
    setPlottingStyle();
    setPlottingUpperLimits();

    gEnergySpectrum = 0;
    fEnergySpectrumFit = 0;
    setConfidenceLevel( 0.68 );

}


void VEnergySpectrum::setEnergyBinning( double iB )
{
    fAnalysisEnergyBinning = iB;
}

/*

   energies in TeV

*/
void VEnergySpectrum::setEnergyRangeLinear( double xmin, double xmax )
{
    fAnalysisMinEnergy = xmin;
    fAnalysisMaxEnergy = xmax;
}

/*

   energies in log TeV

*/
void VEnergySpectrum::setEnergyRangeLog( double xmin, double xmax )
{
    fAnalysisMinEnergy = TMath::Power( 10., xmin );
    fAnalysisMaxEnergy = TMath::Power( 10., xmax );
}

/*

   combine energy spectra from all runs

*/
bool VEnergySpectrum::combineRuns()
{
    vector< int > i_temp;
    return combineRuns( i_temp );
}

/*

   read differential flux points from an ascii file

*/

bool VEnergySpectrum::openAsciiFile( string iFile )
{
    // assume that upper and lower errors are given
    fErrorCalculationMethod = "UPDOWN";

    // open ascii file
    ifstream is;
    is.open( iFile.c_str(), ifstream::in );
    if(!is )
    {
        cout << "VEnergySpectrum::openAsciiFile:: error reading data from " << iFile << endl;
        return false;
    }
    string is_line;
    while( getline( is, is_line ) )
    {
        if( is_line.size() <= 0 )
        {
            continue;
        }
        // comment
        if( is_line.substr( 0, 1 ) == "*" )
        {
            continue;
        }

        // new data vector
        VDifferentialFlux i_flux;

        istringstream is_stream( is_line );

        is_stream >> i_flux.Energy;
        i_flux.EnergyWeightedMean = i_flux.Energy;
        is_stream >> i_flux.DifferentialFlux;
        is_stream >> i_flux.DifferentialFluxError_low;
        if( i_flux.DifferentialFluxError_low > 0. )
        {
            i_flux.DifferentialFluxError_low = i_flux.DifferentialFlux - i_flux.DifferentialFluxError_low;
        }
        is_stream >> i_flux.DifferentialFluxError_up;
        if( i_flux.DifferentialFluxError_up > 0. )
        {
            i_flux.DifferentialFluxError_up = i_flux.DifferentialFluxError_up - i_flux.DifferentialFlux;
        }

        if( i_flux.DifferentialFluxError_low > 0. && i_flux.DifferentialFluxError_low > 0. )
        {
            i_flux.DifferentialFluxError = sqrt( i_flux.DifferentialFluxError_low* i_flux.DifferentialFluxError_low
                                                 + i_flux.DifferentialFluxError_up* i_flux.DifferentialFluxError_up );
        }
        else
        {
            i_flux.DifferentialFluxError = -99.;
        }

        // add this bin to data vector
        fDifferentialFlux.push_back( i_flux );

    }
    is.close();

    return true;
}

/*

   combine energy spectra from runs in the given run list

*/
bool VEnergySpectrum::combineRuns( vector< int > runlist, bool bLinearX )
{
    if( isZombie() )
    {
        return false;
    }

    if( fDebug == 1 )
    {
        cout << "VEnergySpectrum::combineRuns " << runlist.size() << endl;
    }

    if( bUseRebinner )
    {
        cout << "VEnergySpectrum::combineRuns: using rebinner" << endl;
    }
    else
    {
        cout << "VEnergySpectrum::combineRuns: not using rebinner" << endl;
    }

    // lin or log axis
    bEnergyAxisLinear = bLinearX;

    // check if runs are available and read run values from anasum file
    readRunList( runlist, fTotalRun );

    // total observation time
    fTotalObservationTime = 0.;
    fTotalObservationTimeDeadTimeCorrected = 0.;
    double i_obsTime = 0.;
    // total normalisation factor
    fTotalNormalisationFactor = 0.;
    // total number of off events
    double fTotalNOff = 0.;

    // energy threshold calculator (see different definitions of energy thresholds)
    VEnergyThreshold iEnergyThresholdCalculator( fEnergyThresholdFixedValue, fEnergyThresholdFileName );

    ///////////////////////////////////////////////////////////////////////
    // loop over all runs in run list
    int z = 0;
    string hname;
    cout << "combining " << fRunList.size() << " runs" << endl;
    if( fRunList.size() == 0 )
    {
        return false;
    }
    double i_nonCounts_counter = 0.;
    double i_noffCounts_counter = 0.;
    for( unsigned int i = 0; i < fRunList.size(); i++ )
    {
        // differential counting histogram 'on'
        if( bLinearX )
        {
            hname = "hLinerecCounts_on";
        }
        else
        {
            hname = "herecCounts_on";
            if( fOffsetDistance > -998. )
            {
                hname = "herecCounts2D_vs_distance_on";
            }
        }
        TH1D* i_hErecCountsOn = ( TH1D* )getHistogram( hname, fRunList[i].runnumber, "energyHistograms", fOffsetDistance );
        // differential counting histogram 'off'
        if( bLinearX )
        {
            hname = "hLinerecCounts_off";
        }
        else
        {
            hname = "herecCounts_off";
            if( fOffsetDistance > -998. )
            {
                hname = "herecCounts2D_vs_distance_off";
            }
        }
        TH1D* i_hErecCountOff = ( TH1D* )getHistogram( hname, fRunList[i].runnumber, "energyHistograms", fOffsetDistance );
        // histogram with systematic errors (same for on and off)
        hname = "gMeanEnergySystematicError";
        TGraphErrors* i_hEsys = ( TGraphErrors* )getHistogram(
                                    hname, fRunList[i].runnumber, "EffectiveAreas", fOffsetDistance );
        if( fAnalysisEnergyThresholdDefinition == 1 && !i_hEsys )
        {
            cout << "WARNING: histogram with systematic error in energy reconstruction not found";
            cout << " (run " << fRunList[i].runnumber << ")" << endl;
        }
        // get effective area
        TH1* i_hEffAreaP = ( TH1* )getHistogram(
                               "herecEffectiveArea_on", fRunList[i].runnumber, "energyHistograms", fOffsetDistance );
        TGraphErrors* i_gEff = new TGraphErrors( 0 );
        VHistogramUtilities i_hisUtl;
        i_hisUtl.get_Graph_from_Histogram( i_hEffAreaP, i_gEff );

        if( fAnalysisEnergyThresholdDefinition == 2 && !i_gEff )
        {
            // second choice: try to get off effective areas (might even have better statistics)
            hname = "gMeanEffectiveArea_off";
            i_gEff = ( TGraphErrors* )getHistogram( hname, fRunList[i].runnumber, "EffectiveAreas", fOffsetDistance );
            if(!i_gEff )
            {
                cout << "WARNING: no mean effective area graph found, ignoring run ";
                cout << " (run " << fRunList[i].runnumber << ")" << endl;
                continue;
            }
        }

        if(!i_hErecCountsOn || !i_hErecCountOff )
        {
            cout << "histograms not found for run " << fRunList[i].runnumber << endl;
            continue;
        }
        // total number of signal and background event numbers
        i_nonCounts_counter  += i_hErecCountsOn->GetEntries();
        i_noffCounts_counter += i_hErecCountOff->GetEntries();

        /////////////////////////////////////////////////////
        // get energy threshold (may depend on ze, az, ...)
        // (for energy threshold definitions see VEnergyThreshold )
        /////////////////////////////////////////////////////
        // no energy threshold
        if( fAnalysisEnergyThresholdDefinition == 0 )
        {
            fRunList[i].energyThreshold = 0.;
        }
        // energy threshold: systematic smaller then given value
        else if( fAnalysisEnergyThresholdDefinition == 1 )
        {
            fRunList[i].energyThreshold = iEnergyThresholdCalculator.getEnergy_maxSystematic(
                                              i_hEsys, fAnalysisMaxEnergySystematic );
        }
        // energy threshold: effective area > given fraction of maximum effective area (typical value is 10%)
        else if( fAnalysisEnergyThresholdDefinition == 2 )
        {
            fRunList[i].energyThreshold = iEnergyThresholdCalculator.getEnergy_MaxEffectiveAreaFraction(
                                              i_gEff, fAnalysisMaxEffectiveAreaFraction );
        }
        // energy threshold given by fixed value (e.g. 0.5 TeV)
        else if( fAnalysisEnergyThresholdDefinition == 3 )
        {
            fRunList[i].energyThreshold = iEnergyThresholdCalculator.getEnergy_fixedValue();
        }
        // print run info
        if( fDebug == 1 )
        {
            fRunList[i].print();
        }

        /////////////////////////////////////////////////////
        // rebin energy spectra according to user input
        /////////////////////////////////////////////////////
        if( bUseRebinner && nRebinner == 0 && newBinningGroupings.size() > 0 )
        {
            setOriginalBinner( i_hErecCountsOn );
        }

        rebinEnergySpectrum( i_hErecCountsOn, fAnalysisEnergyBinning, bLinearX );
        rebinEnergySpectrum( i_hErecCountOff, fAnalysisEnergyBinning, bLinearX );
        // clone first histogram and make summary histogram
        if( z == 0 )
        {
            // differential counting histogram 'on'
            if( bLinearX )
            {
                hname = "hLinerecCounts_on";
            }
            else
            {
                hname = "herecCounts_on";
            }
            hErecCountsOn = ( TH1D* )i_hErecCountsOn->Clone( hname.c_str() );
            rebinEnergySpectrum( hErecCountsOn, fAnalysisEnergyBinning, bLinearX );
            hErecCountsOn->Reset();
            hErecCountsOn->SetEntries( 0 );
            // differential counting histogram 'off'
            if( bLinearX )
            {
                hname = "hLinerecCounts_off";
            }
            else
            {
                hname = "herecCounts_off";
            }
            hErecCountsOff = ( TH1D* )i_hErecCountOff->Clone( hname.c_str() );
            rebinEnergySpectrum( hErecCountsOff, fAnalysisEnergyBinning, bLinearX );
            hErecCountsOff->Reset();
            hErecCountsOff->SetEntries( 0 );
            // histogram with total observation time
            if( bLinearX )
            {
                hname = "hLinerecTotalTime";
            }
            else
            {
                hname = "herecTotalTime";
            }
            hErecTotalTime = ( TH1D* )i_hErecCountsOn->Clone( hname.c_str() );
            rebinEnergySpectrum( hErecTotalTime, fAnalysisEnergyBinning, bLinearX );
            hErecTotalTime->Reset();
            hErecTotalTime->SetEntries( 0 );
            hErecTotalTime->SetYTitle( "observing time [s]" );
            // histogram with total observation time
            if( bLinearX )
            {
                hname = "hLinerecTotalTimeDeadTimeCorrected";
            }
            else
            {
                hname = "hErecTotalTimeDeadTimeCorrected";
            }
            hErecTotalTimeDeadTimeCorrected = ( TH1D* )i_hErecCountsOn->Clone( hname.c_str() );
            rebinEnergySpectrum( hErecTotalTimeDeadTimeCorrected, fAnalysisEnergyBinning, bLinearX );
            hErecTotalTimeDeadTimeCorrected->Reset();
            hErecTotalTimeDeadTimeCorrected->SetEntries( 0 );
            hErecTotalTimeDeadTimeCorrected->SetYTitle( "observing time [s]" );
            // histogram with mean effective area
            if( bLinearX )
            {
                hname = "hLinEffArea";
            }
            else
            {
                hname = "hEffArea";
            }
            hEffArea = ( TH1D* )i_hErecCountsOn->Clone( hname.c_str() );
            rebinEnergySpectrum( hEffArea, fAnalysisEnergyBinning, bLinearX );
            if( hEffArea )
            {
                hEffArea->Reset();
            }
        }

        /////////////////////////////////////////////////
        // combine histograms
        /////////////////////////////////////////////////

        // observation time (note: already corrected for dead time)
        i_obsTime = fRunList[i].tOn;
        // calculate total observation time (take the energy threshold into account)
        addValueToHistogram( hErecTotalTime, i_obsTime, fRunList[i].energyThreshold, bLinearX );
        fTotalObservationTime += i_obsTime;
        addValueToHistogram( hErecTotalTimeDeadTimeCorrected, i_obsTime* fRunList[i].deadTimeFraction, fRunList[i].energyThreshold, bLinearX );
        fTotalObservationTimeDeadTimeCorrected += i_obsTime * fRunList[i].deadTimeFraction;
        // add current histogram to combined histogram (take energy threshold into account)
        // (counting histograms are not dead time corrected)
        addHistogram( hErecCountsOn, i_hErecCountsOn, fRunList[i].energyThreshold, bLinearX );
        addHistogram( hErecCountsOff, i_hErecCountOff, fRunList[i].energyThreshold, bLinearX );

        // calculation of mean normalisation factor
        fTotalNormalisationFactor += fRunList[i].NOff * fRunList[i].alpha;
        fTotalNOff += fRunList[i].NOff;

        // calculation of mean effective area (weighted by observation time)
        addValueToHistogram( hEffArea, i_gEff, i_obsTime, fRunList[i].energyThreshold, bLinearX );

        z++;
    }
    if( fDebug == 3 )
    {
        cout << "COUNTING HISTOGRAMS: ";
        cout << i_nonCounts_counter << "\t" << i_noffCounts_counter << endl;
    }
    if( fDebug == 3 )
    {
        cout << "COUNTING HISTOGRAMS (total) after rebinning: ";
        cout << hErecCountsOn->GetEntries() << "\t" << hErecCountsOff->GetEntries() << endl;
    }
    // normalize effective area
    hEffArea->Divide( hErecTotalTime );

    // calculate weighted mean normalisation factor
    if( fTotalNOff > 0. )
    {
        fTotalNormalisationFactor /= fTotalNOff;
    }

    cout << "total time [s]: " << fTotalObservationTime;
    cout << " (dead time corrected [s]: " << fTotalObservationTimeDeadTimeCorrected << ", ";
    cout << "total norm factor: " << fTotalNormalisationFactor << ")" << endl;

    bCombineRuns = true;

    return true;
}

/*

   dN/dE to dE

*/
void VEnergySpectrum::multiplyEnergySpectrumbydE( TH1* h, bool bLinearEnergyAxis )
{
    if( fDebug == 1 )
    {
        cout << "VEnergySpectrum::multiplyEnergySpectrumbydE " << h << " " << bLinearEnergyAxis << endl;
    }
    if(!h )
    {
        return;
    }

    double ehigh = 0.;
    double elow = 0.;

    for( int i = 1; i <= h->GetNbinsX(); i++ )
    {
        if( bLinearEnergyAxis )
        {
            elow = h->GetBinLowEdge( i );
            ehigh = h->GetBinLowEdge( i ) + h->GetBinWidth( i );
        }
        else
        {
            elow = pow( 10., h->GetBinLowEdge( i ) );
            ehigh = pow( 10., h->GetBinLowEdge( i ) + h->GetBinWidth( i ) );
        }

        if( ehigh - elow > 0. )
        {
            h->SetBinContent( i, h->GetBinContent( i ) * ( ehigh - elow ) );
            h->SetBinError( i, h->GetBinError( i ) * ( ehigh - elow ) );
        }
    }
}


/*

   dN to dN/dE

*/
void VEnergySpectrum::divideEnergySpectrumbydE( TH1* h, bool bLinearEnergyAxis )
{
    if( fDebug == 1 )
    {
        cout << "VEnergySpectrum::divideEnergySpectrumbydE " << h << " " << bLinearEnergyAxis << endl;
    }

    if(!h )
    {
        return;
    }

    double ehigh = 0.;
    double elow = 0.;

    for( int i = 1; i <= h->GetNbinsX(); i++ )
    {
        if( bLinearEnergyAxis )
        {
            elow = h->GetBinLowEdge( i );
            ehigh = h->GetBinLowEdge( i ) + h->GetBinWidth( i );
        }
        else
        {
            elow = pow( 10., h->GetBinLowEdge( i ) );
            ehigh = pow( 10., h->GetBinLowEdge( i ) + h->GetBinWidth( i ) );
        }

        if( ehigh - elow > 0. )
        {
            h->SetBinContent( i, h->GetBinContent( i ) / ( ehigh - elow ) );
            h->SetBinError( i, h->GetBinError( i ) / ( ehigh - elow ) );
        }
    }
}

/*

    set significance parameters

    (decide where to plot an upper limit and where a differential flux point)

*/
void VEnergySpectrum::setSignificanceParameters( double iSig, double iMinEvents, double iUL, int iLiAndMa, int iULAlgo )
{
    fAnalysisSignificance = iSig;
    fAnalysisMinEvents = iMinEvents;
    fAnalysisUpperLimits = iUL;
    fAnalysisLiAndMaEquation = iLiAndMa;
    fAnalysisUpperLimitAlgorithm = iULAlgo;
}


/*!
    add observation time from current run

    this calculation takes the energy threshold into account
*/
void VEnergySpectrum::addValueToHistogram( TH1* h, double iTObs, double iEThreshold, bool bLinearX )
{
    if( fDebug == 1 )
    {
        cout << "VEnergySpectrum::addValueToHistogram " << h << " " << iEThreshold << " " << bLinearX << endl;
    }
    if(!h )
    {
        return;
    }

    // log energy axis
    if(!bLinearX && iEThreshold > 0. )
    {
        iEThreshold = log10( iEThreshold );
    }
    else if(!bLinearX )
    {
        iEThreshold = -1.e20;
    }

    for( int i = 1; i <= h->GetNbinsX(); i++ )
    {
        if(!fAnalysisHistogramAddingUseLowEdge && h->GetBinLowEdge( i ) >= iEThreshold )
        {
            h->SetBinContent( i, h->GetBinContent( i ) + iTObs );
        }
        if( fAnalysisHistogramAddingUseLowEdge && ( h->GetBinLowEdge( i ) + h->GetBinWidth( i ) ) >= iEThreshold )
        {
            h->SetBinContent( i, h->GetBinContent( i ) + iTObs );
        }
    }
}

/*

   add values from a graph to a histogram (evaluated at each bin)

*/
void VEnergySpectrum::addValueToHistogram( TH1* h, TGraph* g, double iTObs, double iEThreshold, bool bLinearX )
{
    if( fDebug == 1 )
    {
        cout << "VEnergySpectrum::addValueToHistogram " << h << " " << iEThreshold << " " << bLinearX << endl;
    }
    if(!h )
    {
        return;
    }
    if(!g )
    {
        return;
    }

    // log energy axis
    if(!bLinearX && iEThreshold > 0. )
    {
        iEThreshold = log10( iEThreshold );
    }
    else if(!bLinearX )
    {
        iEThreshold = -1.e20;
    }

    for( int i = 1; i <= h->GetNbinsX(); i++ )
    {
        double x = VMathsandFunctions::getMeanEnergyInBin( fEnergyInBinDefinition, h->GetXaxis()->GetBinLowEdge( i ),
                   h->GetXaxis()->GetBinUpEdge( i ), fPlottingSpectralWeightForBinCenter );
        if( x < -1.e90 )
        {
            continue;
        }

        if( g->Eval( x ) > 0. )
        {
            if(!fAnalysisHistogramAddingUseLowEdge && h->GetBinLowEdge( i ) >= iEThreshold )
            {
                h->SetBinContent( i, h->GetBinContent( i ) + g->Eval( x ) * iTObs );
            }
            if( fAnalysisHistogramAddingUseLowEdge && ( h->GetBinLowEdge( i ) + h->GetBinWidth( i ) ) >= iEThreshold )
            {
                h->SetBinContent( i, h->GetBinContent( i ) + g->Eval( x ) * iTObs );
            }
        }
    }
}


/*
    this function is destructive to h2
*/
void VEnergySpectrum::addHistogram( TH1* h1, TH1* h2, double iEThreshold, bool bLinearX )
{
    if( fDebug == 1 )
    {
        cout << "VEnergySpectrum::addHistogram " << h1 << " " << h2 << " " << iEThreshold << " " << bLinearX << endl;
    }

    if(!h1 || !h2 )
    {
        return;
    }

    if( h1->GetNbinsX() != h2->GetNbinsX() )
    {
        cout << "Error in addHistogram( TH1*h1, TH1* h2, iEThreshold, bool bLinearX ): histograms have different bin definitions" << endl;
        cout << "\t" << h1->GetNbinsX() << " " << h2->GetNbinsX() << endl;
        return;
    }

    if( iEThreshold > 0. )
    {
        if(!bLinearX )
        {
            iEThreshold = log10( iEThreshold );
        }

        for( int i = 0; i <= h2->GetNbinsX(); i++ )
        {
            // (default)
            if(!fAnalysisHistogramAddingUseLowEdge && h2->GetBinLowEdge( i ) < iEThreshold )
            {
                h2->SetBinContent( i, 0. );
                h2->SetBinError( i, 0. );
            }
            else if( fAnalysisHistogramAddingUseLowEdge && ( h2->GetBinLowEdge( i ) + h2->GetBinWidth( i ) ) < iEThreshold )
            {
                h2->SetBinContent( i, 0. );
                h2->SetBinError( i, 0. );
            }
        }
    }
    h1->Add( h2 );
}


/*!
    this function is for a logarithmic energy axis

    iFluxes = true: return dN and not dN/dE
*/
void VEnergySpectrum::rebinEnergySpectrum( TH1D* h, double iER, bool bLinearX )
{
    if(!h || iER < 0. )
    {
        return;
    }

    // histogram name
    string itemp = h->GetName();

    // get current binning of energy axis
    double iBW = h->GetXaxis()->GetBinWidth( 1 );
    if( iBW - iER > 1.e-5 && !bUseRebinner )
    {
        cout << "VEnergySpectrum::rebinEnergySpectrum: error: cannot rebin to smaller than existing bins" << endl;
        cout << "current binning: " << iBW;
        cout << ", requested binning: " << iER;
        cout << " (" << TMath::Abs( iBW - iER ) << " )" << endl;
        return;
    }
    int ngroup = int( iER / iBW + 0.01 );
    if( fabs(( double )ngroup * iBW - iER ) > 1.e-5 && !bUseRebinner )
    {
        cout << "VEnergySpectrum::rebinEnergySpectrum error: rebinning only possible in multiples of " << iBW << endl;
        cout << "\t" << ngroup << "\t" << iER << "\t" << iBW << endl;
        return;
    }
    if( fDebug == 2 )
    {
        cout << "REBIN HISTOGRAMS: " << itemp;
        if( bLinearX )
        {
            cout << " (linear)";
        }
        else
        {
            cout << " (log)";
        }
        cout << ": " << iER << endl;
        cout << "\t bin width: " << iBW << ", ngroup " << ngroup << endl;
    }
    if( ngroup == 1 && !bUseRebinner )
    {
        return;
    }

    // counting and timing histograms are simply rebinned
    if( itemp.find( "Counts" ) < itemp.size() || itemp.find( "TotalTime" ) < itemp.size() )
    {
        if( bUseRebinner )
        {
            TH1D* htemp = ( TH1D* )setVariableBinning( h );
            *h = *htemp;
        }
        else
        {
            h->Rebin( ngroup );
        }
        return;
    }
    return;
}

/*!

    calculate differential fluxes (points in energy spectrum)

*/
void VEnergySpectrum::calculateDifferentialFluxes()
{
    if( bAsciiDataFile )
    {
        return;
    }
    if(!hErecCountsOn || !hErecCountsOff || isZombie() )
    {
        return;
    }
    if( fDebug == 1 )
    {
        cout << "VEnergySpectrum::calculateDifferentialFluxes() " << hErecCountsOn->GetNbinsX() << endl;
    }

    // reset data vector
    fDifferentialFlux.clear();

    double x = 0.;

    // keep track of on/off events and lost events
    double i_non = 0.;
    double i_noff = 0.;
    double i_non_cuts = 0.;
    double i_noff_cuts = 0.;
    double i_non_lost[4];
    double i_noff_lost[4];
    for( int i = 0; i < 4; i++ )
    {
        i_non_lost[i] = 0.;
        i_noff_lost[i] = 0.;
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // loop over all bins in energy spectrum energy spectrum
    if( fDebug >= 2 )
    {
        cout << "VEnergySpectrum::calculateDifferentialFluxes(): starting loop over herec bins: " << hErecCountsOn->GetNbinsX() << endl;
    }
    for( int i = 1; i <= hErecCountsOn->GetNbinsX(); i++ )
    {
        // sum up number of on and off events
        i_non  += hErecCountsOn->GetBinContent( i );
        i_noff += hErecCountsOff->GetBinContent( i );

        // debug output
        if( fDebug == 3 )
        {
            cout << setprecision( 3 );
            cout << "bin " << i << ", E " << hErecCountsOn->GetBinCenter( i ) << "\t" << hErecCountsOn->GetBinWidth( i ) << endl;
            cout << "\thErec " << fixed << "\t" << ( int )i_non << "\t" << ( int )i_noff << endl;
            cout << "\thOn  " << hErecCountsOn->GetBinCenter( i ) << "\t" << hErecCountsOn->GetBinContent( i );
            cout << "\t" << i_non << endl;
            cout << "\thOff " << hErecCountsOff->GetBinCenter( i ) << "\t" << hErecCountsOff->GetBinContent( i );
            cout << "\t" << i_noff << endl;
        }
        /////////////////////////////////////////////////
        // remove points outside wanted energy bin
        // (lower limit)
        if( fAnalysisMinEnergy > 0. && hErecCountsOn->GetXaxis()->GetBinLowEdge( i ) <= log10( fAnalysisMinEnergy ) )
        {
            i_non_lost[0]  += hErecCountsOn->GetBinContent( i );
            i_noff_lost[0] += hErecCountsOff->GetBinContent( i );
            if( fDebug == 3 )
            {
                cout << "\t\t failed low energy cuts: " << hErecCountsOn->GetXaxis()->GetBinLowEdge( i );
                cout << "\t" << log10( fAnalysisMinEnergy ) << endl;
                cout << "\t\t\t (total number of lost event due to low energy cut: ";
                cout << i_non_lost[0] << ", " << i_noff_lost[0] << ")" << endl;
            }
            continue;
        }
        // (upper limit)
        if( fAnalysisMaxEnergy > 0. && hErecCountsOn->GetXaxis()->GetBinUpEdge( i ) > log10( fAnalysisMaxEnergy ) )
        {
            i_non_lost[1]  += hErecCountsOn->GetBinContent( i );
            i_noff_lost[1] += hErecCountsOff->GetBinContent( i );
            if( fDebug == 3 )
            {
                cout << "\t\t failed high energy cuts: " << hErecCountsOn->GetXaxis()->GetBinUpEdge( i );
                cout << "\t" << log10( fAnalysisMaxEnergy ) << endl;
                cout << "\t\t\t (total number of lost event due to high energy cut: ";
                cout << i_non_lost[1] << ", " << i_noff_lost[1] << ")" << endl;
            }
            continue;
        }
        /////////////////////////////////////////////////
        // remove points with no counts at all
        if( hErecCountsOn->GetBinContent( i ) < 0. || hErecCountsOff->GetBinContent( i ) < 0. )
        {
            i_non_lost[2]  += hErecCountsOn->GetBinContent( i );
            i_noff_lost[2] += hErecCountsOff->GetBinContent( i );
            if( fDebug == 3 )
            {
                cout << "\t\t failed positive flux criteria (total number of lost event due failed positive flux criteria): ";
                cout << i_non_lost[2] << ", " << i_noff_lost[2] << endl;
            }
            continue;
        }

        /////////////////////////////////////////////////
        // remove points with no normalisation factor
        if( fTotalNormalisationFactor <= 0. )
        {
            i_non_lost[3]  += hErecCountsOn->GetBinContent( i );
            i_noff_lost[3] += hErecCountsOff->GetBinContent( i );
            if( fDebug == 3 )
            {
                cout << "\t\t failed flux normalization criteria: " << fTotalNormalisationFactor << endl;
                cout << "\t\t\t (total number of lost event due failed flux norm criteria: ";
                cout << i_non_lost[3] << ", " << i_noff_lost[3] << ")" << endl;
            }
            continue;
        }
        /////////////////////////////////////////////////
        // new data vector
        VDifferentialFlux i_flux;

        // energy (mean point of a log10 bin)
        x = hErecCountsOn->GetXaxis()->GetBinCenter( i );

        if(!bEnergyAxisLinear )
        {
            i_flux.Energy = TMath::Power( 10., x );
        }
        else
        {
            i_flux.Energy = x;
        }
        // lower and upper edge of energy bin
        if(!bEnergyAxisLinear )
        {
            i_flux.Energy_lowEdge = TMath::Power( 10., hErecCountsOn->GetXaxis()->GetBinLowEdge( i ) );
            i_flux.Energy_upEdge  = TMath::Power( 10., hErecCountsOn->GetXaxis()->GetBinUpEdge( i ) );
        }
        else
        {
            i_flux.Energy_lowEdge = hErecCountsOn->GetXaxis()->GetBinLowEdge( i );
            i_flux.Energy_upEdge  = hErecCountsOn->GetXaxis()->GetBinUpEdge( i );
        }
        // adjust energy (e.g. to spectral weighted bin value)
        double x = VMathsandFunctions::getMeanEnergyInBin( fEnergyInBinDefinition, hErecCountsOn->GetXaxis()->GetBinLowEdge( i ),
                   hErecCountsOn->GetXaxis()->GetBinUpEdge( i ), fPlottingSpectralWeightForBinCenter );
        if( fEnergyInBinDefinition > 90 )
        {
            cout << "VEnergySpectrum::calculateDifferentialFluxes() invalid fEnergyInBinDefinition: " << fEnergyInBinDefinition << endl;
            cout << "  allowed values: 0, 1,2 " << endl;
            return;
        }
        if( x < -1.e90 )
        {
            cout << "VEnergySpectrum::calculateDifferentialFluxes() invalid energy : ";
            cout << hErecCountsOn->GetXaxis()->GetBinLowEdge( i ) << "\t" << hErecCountsOn->GetXaxis()->GetBinUpEdge( i ) << endl;
            cout << "error" << endl;
            return;
        }

        if(!bEnergyAxisLinear )
        {
            i_flux.EnergyWeightedMean = TMath::Power( 10., x );
        }
        else
        {
            i_flux.EnergyWeightedMean = x;
        }

        // energy interval dE
        if( bEnergyAxisLinear )
        {
            i_flux.dE = hErecCountsOn->GetXaxis()->GetBinUpEdge( i ) - hErecCountsOn->GetXaxis()->GetBinLowEdge( i );
            i_flux.Energy_lowEdge = hErecCountsOn->GetXaxis()->GetBinLowEdge( i );
            i_flux.Energy_upEdge = hErecCountsOn->GetXaxis()->GetBinUpEdge( i );
        }
        else
        {
            i_flux.dE = TMath::Power( 10., hErecCountsOn->GetXaxis()->GetBinUpEdge( i ) ) - TMath::Power( 10., hErecCountsOn->GetXaxis()->GetBinLowEdge( i ) );
            i_flux.Energy_lowEdge =  TMath::Power( 10., hErecCountsOn->GetXaxis()->GetBinLowEdge( i ) );
            i_flux.Energy_upEdge =  TMath::Power( 10., hErecCountsOn->GetXaxis()->GetBinUpEdge( i ) );
        }

        // get on and off numbers for this bin
        i_flux.NOn        = hErecCountsOn->GetBinContent( i );
        i_flux.NOn_error  = sqrt( i_flux.NOn );
        i_non_cuts       += i_flux.NOn;

        i_flux.NOff       = hErecCountsOff->GetBinContent( i );
        i_flux.NOff_error = sqrt( i_flux.NOff );
        i_noff_cuts      += i_flux.NOff;
        if( fDebug == 3 )
        {
            cout << "\t\t total counts in bin after cuts: " << i << "\t" << i_non_cuts << "\t" << i_noff_cuts << endl;
        }
        i_flux.NOff_alpha = fTotalNormalisationFactor;
        // remove all empty bins
        if( i_flux.NOn == 0 && i_flux.NOff == 0 )
        {
            if( fDebug == 3 )
            {
                cout << "\t\t no counts: " << i_flux.NOn << "\t" << i_flux.NOff << endl;
            }
            continue;
        }
        // observation time
        i_flux.ObsTime = hErecTotalTimeDeadTimeCorrected->GetBinContent( i );
        if( i_flux.ObsTime < 1.e-5 )
        {
            if( fDebug == 3 )
            {
                cout << "\t\t no observation time " << endl;
            }
            continue;
        }

        // calculate significance (using Li&Ma, note: event number might be too low here for Li & Ma)
        i_flux.Significance = VStatistics::calcSignificance( i_flux.NOn, i_flux.NOff, fTotalNormalisationFactor, fAnalysisLiAndMaEquation );

        // check that if this bin is significant using Rolke et al method
        TRolke i_Rolke;
        i_Rolke.SetCLSigmas( fAnalysisSignificance );
        i_Rolke.SetPoissonBkgKnownEff(( int )i_flux.NOn, ( int )i_flux.NOff, 1. / fTotalNormalisationFactor, 1. );

        ///////////////////////////////////////////////////////////////
        // calculate flux (check first that this is a significant bin)
        ///////////////////////////////////////////////////////////////
        if((( fErrorCalculationMethod == "Rolke" && i_Rolke.GetLowerLimit() > 0 )
                || ( fErrorCalculationMethod == "Poisson" && i_flux.Significance > fAnalysisSignificance ) )
                && i_flux.NOn > fAnalysisMinEvents )
        {
            // calculate flux and Poissonian flux error from event numbers
            double i_ndiff = i_flux.NOn - i_flux.NOff * fTotalNormalisationFactor;
            if( fErrorCalculationMethod == "Rolke" )
            {
                // this is wrong, but people want to have a mean value instead of a 1 sigma range
                i_Rolke.SetCLSigmas( 1.e-4 );
                i_ndiff = 0.5 * ( i_Rolke.GetLowerLimit() + i_Rolke.GetUpperLimit() );
            }
            i_flux.DifferentialFlux  = i_ndiff;
            i_flux.DifferentialFlux /= i_flux.dE;
            i_flux.DifferentialFlux /= i_flux.ObsTime;
            if( hEffArea && hEffArea->GetBinContent( hEffArea->FindBin( log10( i_flux.Energy ) ) ) > 0. )
            {
                i_flux.DifferentialFlux /= ( hEffArea->GetBinContent( hEffArea->FindBin( log10( i_flux.Energy ) ) ) * 1.e4 );
                i_flux.EffectiveArea = hEffArea->GetBinContent( hEffArea->FindBin( log10( i_flux.Energy ) ) );
            }
            else
            {
                continue;
            }
            // calculate asymmetric flux errors using TRolke
            i_Rolke.SetCLSigmas( 1. );
            if( i_ndiff != 0. )
            {
                i_flux.DifferentialFluxError_low = TMath::Abs(( i_ndiff - i_Rolke.GetLowerLimit() ) * i_flux.DifferentialFlux / i_ndiff );
                i_flux.DifferentialFluxError_up  = TMath::Abs(( i_Rolke.GetUpperLimit() - i_ndiff ) * i_flux.DifferentialFlux  / i_ndiff );
                // recalculate poissonian flux error
                i_flux.DifferentialFluxError =  sqrt( i_flux.NOn + fTotalNormalisationFactor* fTotalNormalisationFactor* i_flux.NOff )
                                                * i_flux.DifferentialFlux / i_ndiff;
            }
            // that probably does not make sense
            else
            {
                cout << "SHOULD NOT BE HERE" << endl;
            }
        }
        ///////////////////////////////
        // calculate upper flux limit
        ///////////////////////////////////////
        else
        {
            i_flux.DifferentialFlux = VStatistics::calcUpperLimit( i_flux.NOn, i_flux.NOff, fTotalNormalisationFactor,
                                      fAnalysisUpperLimits, fAnalysisUpperLimitAlgorithm );
            // scale  upper flux to right value
            if( i_flux.DifferentialFlux > 0. && hEffArea )
            {
                if( i_flux.dE > 0. && i_flux.ObsTime > 0. && hEffArea->GetBinContent( hEffArea->FindBin( log10( i_flux.Energy ) ) ) > 0. )
                {
                    i_flux.DifferentialFlux /= i_flux.dE;
                    i_flux.DifferentialFlux /= i_flux.ObsTime;
                    i_flux.DifferentialFlux /= ( hEffArea->GetBinContent( hEffArea->FindBin( log10( i_flux.Energy ) ) ) * 1.e4 );
                    i_flux.EffectiveArea = hEffArea->GetBinContent( hEffArea->FindBin( log10( i_flux.Energy ) ) );
                }
            }
            else
            {
                i_flux.DifferentialFlux = -1.e99;
            }
            // flux error is negative for upper flux value
            i_flux.DifferentialFluxError = -1.;
        }
        i_flux.fillEvent( fRunList_MJD_min, fRunList_MJD_max );
        // add this bin to data vector
        fDifferentialFlux.push_back( i_flux );

        // debug output
        if( fDebug >= 2 )
        {
            cout << setprecision( 8 ) << "E " << i_flux.Energy_lowEdge << " - " << i_flux.Energy_upEdge;
            cout << ", NOn " << i_flux.NOn;
            cout << ", NOff " << i_flux.NOff;
            cout << ", NDif " << i_flux.NOn - i_flux.NOff* fTotalNormalisationFactor;
            cout << ", Sign " << i_flux.Significance;
            cout << ", Norm " << fTotalNormalisationFactor;
            cout << ", TOn " << i_flux.ObsTime;
            cout << ", Flux " << scientific << i_flux.DifferentialFlux;
            cout << ", (F2 " << ( i_flux.NOn - i_flux.NOff* fTotalNormalisationFactor ) / i_flux.dE / i_flux.ObsTime
                 / hEffArea->GetBinContent( hEffArea->FindBin( log10( i_flux.Energy ) ) ) * 1.e-4 << ")";
            cout << fixed << endl;
            cout << endl;
        }
    }  // end of loop over all energy bins
    ///////////////////////////////////////////

    if( fDebug >= 2 )
    {
        cout << "total events " << fixed << setprecision( 8 ) << i_non << " " << i_noff;
        cout << " " << fTotalNormalisationFactor << " " << i_non - fTotalNormalisationFactor* i_noff << endl;
    }
}


/*
   print differential fluxes

   bSED = true: print vF_v fluxes to be used in VSpectralEnergyDistribution
*/
void VEnergySpectrum::printDifferentialFluxes( bool bSED )
{
    //for( unsigned int i = 0; i < fDifferentialFlux.size(); i++ ) fDifferentialFlux[i].print( bSED );
    cout << "\n#----------------------------------------------------------------------------------------------------------------------------#" << endl;
    cout << "# <E>         E_min       E_max    dE         (dN/dE & Err or UL & 0.0)     NOn  Err      NOff  Err      alpha      sign     T "  << endl;
    cout << "# TeV          TeV        TeV      TeV              1/cm^2/s/TeV                                                    sigma    s " << endl;
    for( unsigned int i = 0; i < fDifferentialFlux.size(); i++ )
    {
        fDifferentialFlux[i].printClean( bSED );
    }
    cout << "#------------------------------------------------------------------------------------------------------------------------------#" << endl;
}

/*

    plot the differential energy spectrum taking significances, upper limits, etc into account

*/
TCanvas* VEnergySpectrum::plot( TCanvas* c )
{
    if( isZombie() )
    {
        return 0;
    }

    // combine histogram of energy spectra from all the runs in the current run lists (herec)
    if(!bCombineRuns )
    {
        combineRuns();
    }
    // calculate differential fluxes (taking significance parameters into account); some bins might be upper limit
    calculateDifferentialFluxes();

    TH1D* hNull = 0;
    // setup canvas for plotting
    if( c == 0 )
    {
        char hname[600];
        char htitle[600];
        sprintf( hname, "c_%s", fDataSetName.c_str() );
        sprintf( htitle, "energy spectrum (%s)", fDataSetName.c_str() );
        c = new TCanvas( hname, htitle, 10, 10, 600, 600 );
        c->SetGridx( 0 );
        c->SetGridy( 0 );
        gPad->SetLeftMargin( 0.13 );

        sprintf( hname, "hnull_%s", fDataSetName.c_str() );
        hNull = new TH1D( hname, "", 100, log10( fPlottingMinEnergy ), log10( fPlottingMaxEnergy ) );
        hNull->SetMinimum( fPlottingYaxisMin );
        hNull->SetMaximum( fPlottingYaxisMax );
        hNull->SetStats( 0 );
        hNull->SetXTitle( "log_{10} energy [TeV]" );
        // y-axis: taking multiplication by E^fPlottingMultiplierIndex into account
        hNull->SetYTitle( "dN/dE [cm^{-2}s^{-1}TeV^{-1}]" );
        if( fPlottingMultiplierIndex > 1. )
        {
            sprintf( hname, "E^{%.2f} dN/dE [cm^{-2}s^{-1}TeV^{%.2f}]", fPlottingMultiplierIndex, fPlottingMultiplierIndex - 1. );
            hNull->SetYTitle( hname );
        }
        hNull->GetYaxis()->SetTitleOffset( 1.6 );

        // plot an empty histogram with the right axes
        plot_nullHistogram( c, hNull, fPlottingLogEnergyAxis, true, hNull->GetYaxis()->GetTitleOffset(), fPlottingMinEnergy, fPlottingMaxEnergy );
        c->SetLogy( 1 );
    }
    if( c )
    {
        c->cd();
    }

    // plot the spectral energy points
    plot_energySpectrum();

    fPlottingCanvas = c;

    return c;
}

/*

   fill and plot energy spectrum graph

   plot upper limits

   return pointer to graph with differential flux points

*/
TGraphAsymmErrors* VEnergySpectrum::plot_energySpectrum()
{
    // expect that vector with differential fluxes is filled
    if( fDifferentialFlux.size() == 0 )
    {
        return 0;
    }

    // loop over vector with differential fluxes and plot fluxes or upper flux limits
    for( unsigned int i = 0; i < fDifferentialFlux.size(); i++ )
    {
        // upper flux limits
        // (plot as arrows)
        if( fDifferentialFlux[i].DifferentialFluxError < 0. && fPlottingUpperLimits )
        {
            if( fDifferentialFlux[i].DifferentialFlux > fPlottingYaxisMin && fDifferentialFlux[i].DifferentialFlux < fPlottingYaxisMax )
            {
                double i_energy = log10( fDifferentialFlux[i].Energy );
                double i_flux  = fDifferentialFlux[i].DifferentialFlux * TMath::Power( fDifferentialFlux[i].Energy, fPlottingMultiplierIndex );
                TArrow* fUL = new TArrow( i_energy, i_flux, i_energy, i_flux * 0.25, 0.03, "|-|>" );
                fUL->SetLineColor( fPlottingColor );
                fUL->SetFillColor( fPlottingColor );
                fUL->SetLineWidth( 1 );
                fUL->Draw();
            }
        }
    }
    // graph with differential flux points
    gEnergySpectrum = getEnergySpectrumGraph();
    if( gEnergySpectrum )
    {
        gEnergySpectrum->Draw( "p" );
        return gEnergySpectrum;
    }

    // not successful (!)
    return 0;
}

/*

   fill graph with differential flux points (ignoring points which will be upper limits)

*/
TGraphAsymmErrors* VEnergySpectrum::getEnergySpectrumGraph()
{
    unsigned int z = 0;        // counter

    for( unsigned int i = 0; i < fDifferentialFlux.size(); i++ )
    {
        // differential flux error <0:  this is an upper flux limit; ignore
        if( fDifferentialFlux[i].DifferentialFluxError < 0. )
        {
            continue;
        }

        // first point: create graph with spectral energies
        // (isn't there a delete missing?)
        if( z == 0 )
        {
            gEnergySpectrum = new TGraphAsymmErrors( 1 );
            gEnergySpectrum->SetMarkerColor( fPlottingColor );
            gEnergySpectrum->SetLineColor( fPlottingColor );
            gEnergySpectrum->SetMarkerSize( fPlottingMarkerSize );
            gEnergySpectrum->SetMarkerStyle( fPlottingMarkerStyle );
        }
        // error on flux
        gEnergySpectrum->SetPoint( z, log10( fDifferentialFlux[i].EnergyWeightedMean ),
                                   fDifferentialFlux[i].DifferentialFlux* TMath::Power( fDifferentialFlux[i].Energy, fPlottingMultiplierIndex ) );
        // error on differential flux
        if( fErrorCalculationMethod == "Poisson" )
        {
            gEnergySpectrum->SetPointEYhigh( z, fDifferentialFlux[i].DifferentialFluxError* TMath::Power( fDifferentialFlux[i].Energy, fPlottingMultiplierIndex ) );
            gEnergySpectrum->SetPointEYlow( z, fDifferentialFlux[i].DifferentialFluxError* TMath::Power( fDifferentialFlux[i].Energy, fPlottingMultiplierIndex ) );
        }
        else if( fErrorCalculationMethod == "Rolke" || fErrorCalculationMethod == "UPDOWN" )
        {
            gEnergySpectrum->SetPointEYhigh( z, fDifferentialFlux[i].DifferentialFluxError_up* TMath::Power( fDifferentialFlux[i].Energy, fPlottingMultiplierIndex ) );
            gEnergySpectrum->SetPointEYlow( z, fDifferentialFlux[i].DifferentialFluxError_low* TMath::Power( fDifferentialFlux[i].Energy, fPlottingMultiplierIndex ) );
        }
        z++;
    }
    // return graph
    if( z > 0 && gEnergySpectrum )
    {
        gEnergySpectrum->Print();
        return gEnergySpectrum;
    }

    // not successful (!)
    return 0;
}

/*

   fit to the energy spectrum

   fit function is set with setSpectralFitFunction()

   fSpectralFitFunction == 0 :  power law
   fSpectralFitFunction == 1 :  power law with exponential cutoff
   fSpectralFitFunction == 2 :  broken power law

*/
TF1* VEnergySpectrum::fitEnergySpectrum( string iname, bool bDraw )
{
    // new fitter
    // (delete missing?)
    fSpectralFitter = new VSpectralFitter( iname );
    // set fit function (see IDs in function comments)
    fSpectralFitter->setSpectralFitFunction( fSpectralFitFunction );
    // all energies in TeV
    fSpectralFitter->setSpectralFitFluxNormalisationEnergy( fSpectralFitFluxNormalisationEnergy );
    fSpectralFitter->setSpectralFitRangeLin( fSpectralFitEnergy_min, fSpectralFitEnergy_max );
    fSpectralFitter->setPlottingStyle( fPlottingEnergySpectrumFitLineColor, fPlottingEnergySpectrumFitLineStyle, fPlottingEnergySpectrumFitLineWidth );

    // get energy spectrum
    gEnergySpectrum = getEnergySpectrumGraph();
    fSpectralFitter->setCL( fCL );
    if( gEnergySpectrum )
    {
        // perform the fit
        TF1* f = fSpectralFitter->fit( gEnergySpectrum );
        if( f )
        {
            // draw everything
            if( bDraw )
            {
                f->Draw( "same" );
            }
            // print results
            cout << "Fit Results" << endl;
            fSpectralFitter->print();
            // return pointer to fit function
            return f;
        }
    }

    // not successful (!)
    return 0;
}

double VEnergySpectrum::calculateIntegralFluxFromFitFunction( double iMinEnergy_TeV, double iMaxEnergy_TeV )
{
    if( fSpectralFitter )
    {
        printf( "Flux F(E > %.2f TeV) [cm^-2 s^-1]: ", iMinEnergy_TeV );
        printf( "%.2e +- ", fSpectralFitter->getIntegralFlux( iMinEnergy_TeV, iMaxEnergy_TeV ) );
        printf( "%.2e\n",   fSpectralFitter->getIntegralFluxError( iMinEnergy_TeV, iMaxEnergy_TeV ) );
    }

    return 0.;
}

/*

    set plotting style for fit function

*/
void VEnergySpectrum::setSpectralFitPlottingStyle( int iColor, int iStyle, float iWidth )
{
    fPlottingEnergySpectrumFitLineColor = iColor;
    fPlottingEnergySpectrumFitLineStyle = iStyle;
    fPlottingEnergySpectrumFitLineWidth = iWidth;
}


/*

    write fit results (e.g. index and flux normalization) into the canvas

*/
void VEnergySpectrum::plotFitValues()
{
    if(!fPlottingCanvas )
    {
        cout << "VEnergySpectrum::plotFitValues() error: no canvas to plot things" << endl;
        return;
    }
    fPlottingCanvas->cd();

    if(!fSpectralFitter || !fSpectralFitter->getSpectralFitFunction() )
    {
        cout << "VEnergySpectrum::plotFitValues() error: no fit function" << endl;
        return;
    }
    TF1* fEnergy = fSpectralFitter->getSpectralFitFunction();

    char hname[500];
    TLatex* tL1 = new TLatex();
    sprintf( hname, "dN/dE =" );
    tL1->SetNDC( 1 );
    tL1->SetTextSize( 0.030 );
    tL1->DrawLatex( 0.18, 0.23, hname );
    TLatex* tL2 = new TLatex();
    // get exponent (assume negative exponent)
    int i_expV = ( int )( TMath::Log10( fEnergy->GetParameter( 0 ) ) - 0.5 );
    // get mantissa
    double i_manV = fEnergy->GetParameter( 0 ) * TMath::Power( 10, -1.*( double )i_expV );
    double i_manE = fEnergy->GetParError( 0 ) * TMath::Power( 10, -1.*( double )i_expV );
    // get spectral index
    double i_indexV = fEnergy->GetParameter( 1 ) - fPlottingMultiplierIndex;
    double i_indexE = fEnergy->GetParError( 1 );
    // cutoff energy
    double i_ecutoffV;
    double i_ecutoffE;
    // curvature index
    double i_curvatureV;
    double i_curvatureE;
    // break energy and second slope for a BPL fit
    double i_breakenergyV;
    double i_breakenergyE;
    double i_index2V;
    double i_index2E;

    if( fSpectralFitFunction == 0 )
    {
        // 1) simple power law
        sprintf( hname, "(%.2f#pm%.2f)#times 10^{%d} (E/%.2f TeV)^{%.2f#pm%.2f} [cm^{-2}s^{-1}TeV^{-1}]", i_manV, i_manE, i_expV, fSpectralFitter->getSpectralFitNormalisationEnergy(), i_indexV, i_indexE );
    }
    else if( fSpectralFitFunction == 1 )
    {
        // get energy cutoff
        i_ecutoffV = fEnergy->GetParameter( 2 );
        i_ecutoffE = fEnergy->GetParError( 2 );

        // 2) power law with exponential cutoff
        sprintf( hname, "(%.2f#pm%.2f)#times 10^{%d} (E/%.2f TeV)^{%.2f#pm%.2f}e^{-E/(%.2f#pm%.2f)} [cm^{-2}s^{-1}TeV^{-1}]", i_manV, i_manE, i_expV, fSpectralFitter->getSpectralFitNormalisationEnergy(), i_indexV, i_indexE, i_ecutoffV, i_ecutoffE );
    }
    else if( fSpectralFitFunction == 2 )
    {
        // get break energy and second index
        i_breakenergyV = fEnergy->GetParameter( 3 );
        i_breakenergyE = fEnergy->GetParError( 3 );
        i_index2V = fEnergy->GetParameter( 2 );
        i_index2E = fEnergy->GetParError( 2 );

        // 3) broken power law
        sprintf( hname, "#splitline{(%.2f#pm%.2f)#times 10^{%d} (E/(%.2f#pm%0.02f) TeV)^{#Gamma}[cm^{-2}s^{-1}TeV^{-1}];}{   E<%0.2f, #Gamma = %0.2f#pm%0.2f else #Gamma = %0.2f#pm%0.2f}", i_manV, i_manE, i_expV, i_breakenergyV, i_breakenergyE, i_breakenergyV, i_indexV, i_indexE, i_index2V, i_index2E );
    }
    else if( fSpectralFitFunction == 3 )
    {
        // get curvature index
        i_curvatureV = fEnergy->GetParameter( 2 );
        i_curvatureE = fEnergy->GetParError( 2 );

        // 4) curved power law
        sprintf( hname, "(%.2f#pm%.2f)#times 10^{%d} (E/%.2f TeV)^{%.2f#pm%.2f + (%.2f#pm%.2f)E} [cm^{-2}s^{-1}TeV^{-1}]", i_manV, i_manE, i_expV, fSpectralFitter->getSpectralFitNormalisationEnergy(), i_indexV, i_indexE, i_curvatureV, i_curvatureE );
    }
    else if( fSpectralFitFunction == 4 )
    {
        // get curvature index
        i_curvatureV = fEnergy->GetParameter( 2 );
        i_curvatureE = fEnergy->GetParError( 2 );

        // 4) curved power law
        sprintf( hname, "(%.2f#pm%.2f)#times 10^{%d} (E/%.2f TeV)^{%.2f#pm%.2f + (%.2f#pm%.2f)lg E/%.2f} [cm^{-2}s^{-1}TeV^{-1}]", i_manV, i_manE, i_expV, fSpectralFitter->getSpectralFitNormalisationEnergy(), i_indexV, i_indexE, i_curvatureV, i_curvatureE, fSpectralFitter->getSpectralFitNormalisationEnergy() );
    }


    tL2->SetNDC( 1 );
    tL2->SetTextSize( 0.030 );
    tL2->DrawLatex( 0.18, 0.19, hname );
    TLatex* tL3 = new TLatex();
    double irc2 = 0.;
    if( fEnergy->GetNDF() > 0. )
    {
        irc2 = fEnergy->GetChisquare() / fEnergy->GetNDF();
    }
    sprintf( hname, "#chi^{2}/dof=%.2f/%d (%.1f)", fEnergy->GetChisquare(), ( int )fEnergy->GetNDF(), irc2 );
    tL3->SetNDC( 1 );
    tL3->SetTextSize( 0.030 );
    tL3->DrawLatex( 0.18, 0.15, hname );
}

/*

    plot residuals between fit function and differential energy spectrum

*/
TCanvas* VEnergySpectrum::plotResiduals( TCanvas* c, TF1* f )
{
    if(!fSpectralFitter )
    {
        return 0;
    }
    if(!gEnergySpectrum )
    {
        return 0;
    }

    gEnergySpectrumFitResiduals = new TGraphAsymmErrors( gEnergySpectrum->GetN() );
    gEnergySpectrumFitResiduals->SetMarkerColor( fPlottingColor );
    gEnergySpectrumFitResiduals->SetLineColor( fPlottingColor );
    gEnergySpectrumFitResiduals->SetMarkerSize( fPlottingMarkerSize );
    gEnergySpectrumFitResiduals->SetMarkerStyle( fPlottingMarkerStyle );

    if( fSpectralFitter && !f )
    {
        f = fSpectralFitter->getSpectralFitFunction();
        if(!f )
        {
            return 0;
        }
    }

    double x, y;
    double ye_l = 0.;
    double ye_u = 0.;
    for( int i = 0; i < gEnergySpectrumFitResiduals->GetN(); i++ )
    {
        gEnergySpectrum->GetPoint( i, x, y );
        if( fErrorCalculationMethod == "Poisson" )
        {
            ye_l = gEnergySpectrum->GetErrorY( i );
            ye_u = gEnergySpectrum->GetErrorY( i );
        }
        else
        {
            ye_l = gEnergySpectrum->GetErrorYlow( i );
            ye_u = gEnergySpectrum->GetErrorYhigh( i );
        }
        if( f->Eval( x ) > 0. )
        {
            gEnergySpectrumFitResiduals->SetPoint( i, x, ( y - f->Eval( x ) ) / f->Eval( x ) );
            gEnergySpectrumFitResiduals->SetPointEYhigh( i, ye_u / f->Eval( x ) );
            gEnergySpectrumFitResiduals->SetPointEYlow( i, ye_l / f->Eval( x ) );
        }
    }
    if(!c )
    {
        char hname[600];
        char htitle[600];
        sprintf( hname, "cRes_%s", fDataSetName.c_str() );
        sprintf( htitle, "energy spectrum, residuals (%s)", fDataSetName.c_str() );
        c = new TCanvas( hname, htitle, 700, 10, 500, 500 );
        c->SetGridx( 0 );
        c->SetGridy( 0 );
        gPad->SetLeftMargin( 0.13 );

        sprintf( hname, "hNullGF_%s", fDataSetName.c_str() );
        TH1D* hNullGF = new TH1D( hname, "", 100, log10( fPlottingMinEnergy ), log10( fPlottingMaxEnergy ) );
        hNullGF->SetStats( 0 );
        hNullGF->SetXTitle( "log_{10} energy [TeV]" );
        hNullGF->SetYTitle( "(f_{rec} - f_{fit}) / f_{fit}" );
        hNullGF->GetYaxis()->SetTitleOffset( 1.3 );
        hNullGF->SetMaximum( 1. );
        hNullGF->SetMinimum(-1. );

        plot_nullHistogram( c, hNullGF, fPlottingLogEnergyAxis, false, hNullGF->GetYaxis()->GetTitleOffset(), fPlottingMinEnergy, fPlottingMaxEnergy );
    }
    c->cd();

    gEnergySpectrumFitResiduals->Draw( "p" );

    return c;
}

TCanvas* VEnergySpectrum::plotMeanEffectiveArea( TCanvas* c, double i_effMin, double i_effMax )
{
    if(!hEffArea )
    {
        return 0;
    }
    if(!c )
    {
        char hname[600];
        char htitle[600];
        sprintf( hname, "ceff_%s", fDataSetName.c_str() );
        sprintf( htitle, "effective area vs energy (%s)", fDataSetName.c_str() );
        c = new TCanvas( hname, htitle, 1200, 10, 500, 500 );
        c->SetGridx( 0 );
        c->SetGridy( 0 );
        c->SetLogy( 0 );
        gPad->SetLeftMargin( 0.13 );

        sprintf( hname, "hNullEF_%s", fDataSetName.c_str() );
        TH1D* hNullGT = new TH1D( hname, "", 100, log10( fPlottingMinEnergy ), log10( fPlottingMaxEnergy ) );
        hNullGT->SetStats( 0 );
        hNullGT->SetXTitle( "log_{10} energy_{rec} [TeV]" );
        hNullGT->SetYTitle( "effective area [m^{2}" );
        hNullGT->GetYaxis()->SetTitleOffset( 1.3 );
        hNullGT->SetMinimum( i_effMin );
        if( i_effMax < 0. )
        {
            hNullGT->SetMaximum( hEffArea->GetMaximum() * 1.2 );
        }
        else
        {
            hNullGT->SetMaximum( i_effMax );
        }
        hNullGT->SetMinimum( 0.5 );
        hNullGT->Draw();
    }
    c->cd();

    hEffArea->SetLineColor( fPlottingColor );
    hEffArea->Draw( "same" );

    return c;
}

/*

    plot histogram with events per energy bin

*/
TCanvas*  VEnergySpectrum::plotCountingHistograms( TCanvas* c )
{
    if(!hErecCountsOn || !hErecCountsOff )
    {
        return 0;
    }

    if(!c )
    {
        char hname[600];
        char htitle[600];
        sprintf( hname, "cGT_%s", fDataSetName.c_str() );
        sprintf( htitle, "counts vs energy (%s)", fDataSetName.c_str() );
        c = new TCanvas( hname, htitle, 800, 10, 500, 500 );
        c->SetGridx( 0 );
        c->SetGridy( 0 );
        c->SetLogy( 1 );
        gPad->SetLeftMargin( 0.13 );

        sprintf( hname, "hNullGT_%s", fDataSetName.c_str() );
        TH1D* hNullGT = new TH1D( hname, "", 100, log10( fPlottingMinEnergy ), log10( fPlottingMaxEnergy ) );
        hNullGT->SetStats( 0 );
        hNullGT->SetXTitle( "log_{10} energy [TeV]" );
        hNullGT->SetYTitle( "counts per bin" );
        hNullGT->GetYaxis()->SetTitleOffset( 1.3 );
        hNullGT->SetMaximum( hErecCountsOn->GetMaximum() * 1.2 );
        hNullGT->SetMinimum( 0.5 );
        hNullGT->Draw();
    }
    c->cd();

    hErecCountsOn->Draw( "same" );
    TH1D* hErecCountsOff_Clone = ( TH1D* )hErecCountsOff->Clone();
    hErecCountsOff_Clone->Scale( fTotalNormalisationFactor );
    hErecCountsOff_Clone->Draw( "same" );

    // get counting values used for energy reconstruction
    TGraph* gErecOn = new TGraph( 1 );
    gErecOn->SetMarkerStyle( 24 );
    gErecOn->SetMarkerColor( 1 );
    gErecOn->SetLineColor( 1 );
    TGraph* gErecOff = new TGraph( 1 );
    gErecOff->SetMarkerStyle( 25 );
    gErecOff->SetMarkerColor( 2 );
    gErecOff->SetLineColor( 2 );
    cout << "Counting histogram on:  color 1" << endl;
    cout << "Counting histogram off: color 2" << endl;

    double i_non = 0.;
    double i_noff = 0.;
    for( unsigned int i = 0; i < fDifferentialFlux.size(); i++ )
    {
        gErecOn->SetPoint( i, log10( fDifferentialFlux[i].EnergyWeightedMean ), fDifferentialFlux[i].NOn );
        gErecOff->SetPoint( i, log10( fDifferentialFlux[i].EnergyWeightedMean ), fDifferentialFlux[i].NOff* fTotalNormalisationFactor );
        i_non += fDifferentialFlux[i].NOn;
        i_noff += fDifferentialFlux[i].NOff * fTotalNormalisationFactor;
    }
    gErecOn->Draw( "p" );
    gErecOff->Draw( "p" );

    return c;
}

/*

    plot life time (dead time corrected) vs energy

*/
TCanvas* VEnergySpectrum::plotLifeTimevsEnergy( TCanvas* c )
{
    if(!getTotalTimeHistogram() )
    {
        return 0;
    }

    // total time (not deadtime corrected)
    TH1D* h = ( TH1D* )getTotalTimeHistogram( false )->Clone();
    h->Scale( 1. / 60. );                         //! s -> min
    h->SetFillColor( 9 );
    h->SetFillStyle( 3002 );

    if(!c )
    {
        char hname[600];
        char htitle[600];
        sprintf( hname, "cLT_%s", fDataSetName.c_str() );
        sprintf( htitle, "life time vs energy (%s)", fDataSetName.c_str() );
        c = new TCanvas( hname, htitle, 800, 10, 500, 500 );
        c->SetGridx( 0 );
        c->SetGridy( 0 );
        gPad->SetLeftMargin( 0.13 );

        sprintf( hname, "hNullLT_%s", fDataSetName.c_str() );
        TH1D* hNullLT = new TH1D( hname, "", 100, log10( fPlottingMinEnergy ), log10( fPlottingMaxEnergy ) );
        hNullLT->SetStats( 0 );
        hNullLT->SetXTitle( "log_{10} energy [TeV]" );
        hNullLT->SetYTitle( "life time [min]" );
        hNullLT->GetYaxis()->SetTitleOffset( 1.7 );
        hNullLT->SetMaximum( h->GetMaximum() * 1.2 );
        hNullLT->SetMinimum( 0. );

        plot_nullHistogram( c, hNullLT, fPlottingLogEnergyAxis, false, hNullLT->GetYaxis()->GetTitleOffset(), fPlottingMinEnergy, fPlottingMaxEnergy );
    }
    c->cd();

    h->Draw( "same" );

    // total time (deadtime corrected)
    if( getTotalTimeHistogram( true ) )
    {
        TH1D* h = ( TH1D* )getTotalTimeHistogram( true )->Clone();
        h->Scale( 1. / 60. );                         //! s -> min
        h->SetLineColor( 2 );
        h->Draw( "same" );
    }

    return c;
}

/*

   write number of events and significances at each point of the energy spectrum

*/
void VEnergySpectrum::plotEventNumbers( Double_t ts )
{
    if(!fPlottingCanvas )
    {
        cout << "Error: no canvas to plot things" << endl;
        return;
    }
    fPlottingCanvas->cd();

    char hnum[500];
    for( unsigned int i = 0; i < fDifferentialFlux.size(); i++ )
    {
        sprintf( hnum, "%.1f events (%.1f #sigma)", ( fDifferentialFlux[i].NOn - fTotalNormalisationFactor* fDifferentialFlux[i].NOff ),
                 fDifferentialFlux[i].Significance );

        double y  = ( fDifferentialFlux[i].DifferentialFlux + fDifferentialFlux[i].DifferentialFluxError ) * 1.75;
        // upper flux limit
        if( fDifferentialFlux[i].DifferentialFluxError < 0. )
        {
            y = fDifferentialFlux[i].DifferentialFlux * 1.75;
        }

        TLatex* iT = new TLatex( log10( fDifferentialFlux[i].Energy ), y, hnum );
        iT->SetTextSize( ts );
        iT->SetTextAngle( 90. );
        iT->Draw();
    }
}

/*

    rebinning: set up template histogram

*/
void VEnergySpectrum::setOriginalBinner( TH1* h )
{
    nOriginalBinner = ( TH1D* )h->Clone( "nOriginalBinner" );
    nOriginalBinner->SetTitle( "nOriginalBinner" );

    vector < Double_t > z;

    for( unsigned int i = 0; i < newBinningGroupings.size(); i++ )
    {
        z.push_back( nOriginalBinner->GetBinLowEdge( newBinningGroupings[i] ) );
    }

    unsigned int Nbins = z.size();
    Double_t binsx[Nbins];

    for( unsigned int i = 0; i < Nbins; i++ )
    {
        binsx[i] = z[i];
    }

    nRebinner = new TH1D( "nRebinner", "nRebinner", Nbins - 1, binsx );
    for( Int_t i = 1; i <= nRebinner->GetNbinsX(); i++ )
    {
        cout << "nRebinner: new lower bin edges [TeV]: " << TMath::Power( 10, nRebinner->GetBinLowEdge( i ) ) << endl;
    }
    cout << "nRebinner: number of bins: " << Nbins << endl;
}

/*

   rebin histograms (variable binning)

*/
TH1* VEnergySpectrum::setVariableBinning( TH1* a )
{
    Double_t xbins[nRebinner->GetNbinsX() + 1];

    for( Int_t i = 1; i <= nRebinner->GetNbinsX(); i++ )
    {
        xbins[i - 1] = nRebinner->GetBinLowEdge( i );
    }

    xbins[nRebinner->GetNbinsX()] = nRebinner->GetBinLowEdge( nRebinner->GetNbinsX() ) + nRebinner->GetBinWidth( nRebinner->GetNbinsX() );


    return ( TH1* )a->Rebin( nRebinner->GetNbinsX(), a->GetName(), xbins );
}

void VEnergySpectrum::setRebinBool( bool i_rebin )
{
    bUseRebinner = i_rebin;
}

bool VEnergySpectrum::setEnergyInBinDefinition( unsigned int iEF )
{
    if( iEF < 3 )
    {
        fEnergyInBinDefinition = iEF;
        return true;
    }

    cout << "VEnergySpectrum::setEnergyInBinDefinition error: energy bin definition out of range: " << iEF << endl;
    cout << "possible values: 0, 1, 2" << endl;

    fEnergyInBinDefinition = 99;

    return false;
}

/*
 *  get energy of upper edge of the last filled bin
 *
 *  require at least two bins filled
 *
 */
double VEnergySpectrum::getUpperEdgeofLastFilledEnergyBin( double iMinNdiff, double iSignificance )
{
    if( fDifferentialFlux.size() > 1 )
    {
        for( unsigned int i = fDifferentialFlux.size() - 1; i > 1; i-- )
        {
            double n_diff1 = fDifferentialFlux[i].NOn - fDifferentialFlux[i].NOff_alpha * fDifferentialFlux[i].NOff;
            double n_diff2 = fDifferentialFlux[i - 1].NOn - fDifferentialFlux[i - 1].NOff_alpha * fDifferentialFlux[i - 1].NOff;
            if( n_diff1 > iMinNdiff && n_diff2 > iMinNdiff
                    && fDifferentialFlux[i].Significance > iSignificance
                    && fDifferentialFlux[i - 1].Significance > iSignificance )
            {
                return fDifferentialFlux[i].Energy_upEdge;
            }
        }
    }

    return 9999.;
}

/*
 *  get energy of lower edge of the first filled bin
 *
 */
double VEnergySpectrum::getLowerEdgeofFirstFilledEnergyBin( double iMinNdiff, double iSignificance )
{
    if( fDifferentialFlux.size() > 0 )
    {
        for( unsigned int i = 0; i < fDifferentialFlux.size(); i++ )
        {
            double n_diff = fDifferentialFlux[i].NOn - fDifferentialFlux[i].NOff_alpha * fDifferentialFlux[i].NOff;
            if( n_diff > iMinNdiff && fDifferentialFlux[i].Significance > iSignificance )
            {
                return fDifferentialFlux[i].Energy_lowEdge;
            }
        }
    }

    return 9999.;
}


void VEnergySpectrum::printEnergyBins()
{
    if( hErecCountsOn )
    {
        cout << "Printing energy binning: " << endl;
        for( int i = 1; i <= hErecCountsOn->GetNbinsX(); i++ )
        {
            cout << "Energy bin " << i << ":";
            printf( " bin edges in log10(E/TeV): [%.2f,%.2f],", hErecCountsOn->GetBinLowEdge( i ), hErecCountsOn->GetBinLowEdge( i ) + hErecCountsOn->GetBinWidth( i ) );
            printf( " bin edges in E/TeV: [%.2f,%.2f]", TMath::Power( 10., hErecCountsOn->GetBinLowEdge( i ) ),
                    TMath::Power( 10., hErecCountsOn->GetBinLowEdge( i ) + hErecCountsOn->GetBinWidth( i ) ) );
            cout << endl;
        }
    }
}

/*

    set error and significance calculation method

    Rolke method is more suitable to bins with low number of events, Li & Ma is good for bins with >10 bins

*/
bool VEnergySpectrum::setErrorCalculationMethod( string iMethod )
{
    if( iMethod == "Rolke" || iMethod == "Poisson" )
    {
        fErrorCalculationMethod = iMethod;
        return true;
    }
    cout << "VEnergySpectrum::setErrorCalculationMethod error: unknown error calculation method (should be Rolke/Poisson)" << endl;

    return false;
}

/*
    check rebinning parameters and return numbers if bins to combine
*/
int VEnergySpectrum::getRebinningGrouping( TH1* h, double iNewBinWidth )
{
    // get current binning of energy axis
    // (assume all bins are the same: is this ok?)
    double iBW = h->GetXaxis()->GetBinWidth( 1 );
    if( iBW - iNewBinWidth > 1.e-5 && !bUseRebinner )
    {
        cout << "VEnergySpectrum::checkRebinningParameters: error: cannot rebin to smaller than existing bins" << endl;
        cout << "current binning: " << iBW;
        cout << ", requested binning: " << iNewBinWidth;
        cout << " (" << TMath::Abs( iBW - iNewBinWidth ) << " )" << endl;
        return 0;
    }
    int ngroup = int( iNewBinWidth / iBW + 0.01 );
    if( fabs(( double )ngroup * iBW - iNewBinWidth ) > 1.e-5 && !bUseRebinner )
    {
        cout << "VEnergySpectrum::checkRebinningParameters error: rebinning only possible in multiples of " << iBW << endl;
        cout << "\t" << ngroup << "\t" << iNewBinWidth << "\t" << iBW << endl;
        return 0;
    }

    return ngroup;
}

/*

   short cut to plot the Crab Nebula spectra (sets axis correctly, etc)

*/
TCanvas* VEnergySpectrum::plotCrabNebulaSpectrum( double iPlottingMultiplierIndex, double i_FitStart_TevLin, double i_FitStop_TeVLin, double i_EnergyBinningLog10, int iFitFunctionID )
{
    // binning and statistics
    setEnergyBinning( i_EnergyBinningLog10 );
    setEnergyRangeLinear( 0.10, 500. );
    setPlottingMultiplierIndex( iPlottingMultiplierIndex );

    // plotting
    setPlottingEnergyRangeLinear( 0.05, 200. );
    setPlottingYaxis( 5e-17, 1.e-7 );
    if( iPlottingMultiplierIndex > 2. )
    {
        setPlottingYaxis( 9.e-12, 2.e-10 );
    }

    // plot & fit
    TCanvas* c = plot();

    if( iFitFunctionID >= 0 )
    {
        setSpectralFitFunction( iFitFunctionID );
        setSpectralFitFluxNormalisationEnergy( 1. );
        // set fit range
        if( i_FitStart_TevLin > 0. && i_FitStop_TeVLin > 0. )
        {
            setSpectralFitRangeLin( i_FitStart_TevLin, i_FitStop_TeVLin );
        }
        // set fit range from first to last point
        else
        {
            if( gEnergySpectrum )
            {
                double x, y;
                double i_xmin = 1.e9;
                double i_xmax = -1.e9;
                for( int p = 0; p < gEnergySpectrum->GetN(); p++ )
                {
                    gEnergySpectrum->GetPoint( p, x, y );
                    x = TMath::Power( 10., x );
                    if( x > i_xmax )
                    {
                        i_xmax = x;
                    }
                    if( x < i_xmin )
                    {
                        i_xmin = x;
                    }
                }
                setSpectralFitRangeLin( i_xmin, i_xmax );
            }
        }
        fitEnergySpectrum();
        if( iPlottingMultiplierIndex < 2. )
        {
            plotFitValues();
            plotEventNumbers();
        }
        plotResiduals();
    }
    plotLifeTimevsEnergy();
    plotCountingHistograms();
    plotMeanEffectiveArea();

    VEnergySpectrumfromLiterature l( "$VERITAS_EVNDISP_AUX_DIR/AstroData/TeV_data/EnergySpectrum_literatureValues_CrabNebula.dat" );
    l.setPlottingMultiplierIndex( iPlottingMultiplierIndex );

    l.setPlottingStyle( 2, 2, 2, 25 );
    l.listValues( 1 );
    l.plot( 1, c );
    l.setPlottingStyle( 3, 2, 2, 25 );
    l.listValues( 2 );
    l.plot( 2, c );
    l.setPlottingStyle( 4, 2, 2, 25 );
    l.listValues( 3 );
    l.plot( 3, c );
    l.setPlottingStyle( 5, 2, 2, 25 );
    l.listValues( 4 );
    l.plot( 4, c );
    l.setPlottingStyle( 6, 2, 2, 25 );
    l.listValues( 5 );
    l.plot( 5, c );
    l.setPlottingStyle( 7, 2, 2, 25 );
    l.listValues( 6 );
    l.plot( 6, c );
    l.setPlottingStyle( 8, 2, 2, 25 );
    l.listValues( 7 );
    l.plot( 7, c );
    if( l.isValidID( 10 ) )
    {
        l.setPlottingStyle( 47, 2, 2, 25 );
        l.listValues( 10 );
        l.plot( 10, c );
    }
    l.setPlottingStyle( 800, 4, 4, 25 );
    l.listValues( 11 );
    l.plot( 11, c );

    plot( c );

    return c;
}

/*
 *  write spectral points to csv file
 *
 *  for iDiffFlux == 0: use differential flux vector
 *
 */
bool VEnergySpectrum::writeSpectralPointsToCSVFile( string iOFileName,
        TGraphAsymmErrors* iDiffFlux )
{
    ofstream os;
    os.open( gSystem->ExpandPathName( iOFileName.c_str() ) );
    if(!os )
    {
        cout << "VEnergySpectrum::writeSpectralPointsToCSVFile error opening " << iOFileName << endl;
        return false;
    }
    cout << "writing ecsv file " << iOFileName << endl;

    os << "# \%ECSV 0.9" << endl;
    os << "# ---" << endl;
    os << "# datatype:" << endl;
    os << "# - {name: e_ref, unit: TeV, datatype: float32}" << endl;
    os << "# - {name: e_min, unit: TeV, datatype: float32}" << endl;
    os << "# - {name: e_max, unit: TeV, datatype: float32}" << endl;
    os << "# - {name: dnde, unit: cm-2 s-1 TeV-1, datatype: float32}" << endl;
    os << "# - {name: dnde_errn, unit: cm-2 s-1 TeV-1, datatype: float32}" << endl;
    os << "# - {name: dnde_errp, unit: cm-2 s-1 TeV-1, datatype: float32}" << endl;
    os << "# - {name: dnde_ul, unit: cm-2 s-1 TeV-1, datatype: float32}" << endl;
    os << "# - {name: n_on, datatype: float32}" << endl;
    os << "# - {name: n_off, datatype: float32}" << endl;
    os << "# - {name: norm, datatype: float32}" << endl;
    os << "# - {name: signi, datatype: float32}" << endl;
    os << "# - UL_CONF: 0.95" << endl;
    os << "# meta: !!omap" << endl;
    os << "e_ref e_min e_max dnde dnde_errn dnde_errp dnde_ul n_on n_off norm signi" << endl;
    if( iDiffFlux == 0 )
    {
        for( unsigned int i = 0; i < fDifferentialFlux.size(); i++ )
        {
            os << fDifferentialFlux[i].Energy << "    ";
            os << fDifferentialFlux[i].Energy_lowEdge << "    ";
            os << fDifferentialFlux[i].Energy_upEdge << "    ";
            if( fDifferentialFlux[i].DifferentialFluxError_low > 0. )
            {
                os << fDifferentialFlux[i].DifferentialFlux << "    ";
                os << fDifferentialFlux[i].DifferentialFluxError_low << "    ";
                os << fDifferentialFlux[i].DifferentialFluxError_up << "    ";
                os << "nan     ";
            }
            else
            {
                os << "nan     nan nan    ";
                os << fDifferentialFlux[i].DifferentialFlux << "    ";
            }
            os << fDifferentialFlux[i].NOn << "    ";
            os << fDifferentialFlux[i].NOff << "    ";
            os << fTotalNormalisationFactor << "    ";
            os << fDifferentialFlux[i].Significance;
            os << endl;
        }
    }
    else
    {
        for( int i = 0 ; i < iDiffFlux->GetN(); i++ )
        {
            os << iDiffFlux->GetX()[i] << "    ";
            os << iDiffFlux->GetX()[i] - iDiffFlux->GetErrorXlow( i ) << "    ";
            os << iDiffFlux->GetX()[i] + iDiffFlux->GetErrorXhigh( i ) << "    ";
            os << iDiffFlux->GetY()[i] << "    ";
            os << iDiffFlux->GetErrorYlow( i ) << "    ";
            os << iDiffFlux->GetErrorYhigh( i ) << "    ";
            os << 999.;
            os << endl;
        }
    }
    os.close();

    return true;
}
