/* \class VLombScargle
   \brief spectral analysis of unevenly sampled data (after Lomb/Scargle)

   see e.g. Scargle, J., ApJ 263, 835 (1982)

   TODO:

   calculate of error on resulting period (shuffling of light curve inside errors)

*/

#include "VLombScargle.h"

VLombScargle::VLombScargle()
{
    fDebug = false;

    fPeriodigramGraph = 0;
    fPeriodigramHisto = 0;
    fPeriodigramCanvas = 0;

    setFrequencyRange();

    vector< double > iL;
    vector< int > iN;
    iL.push_back( 0.95 );
    iN.push_back( 2 );
    iL.push_back( 0.99 );
    iN.push_back( 2 );
    iL.push_back( 0.9999 );
    iN.push_back( 4 );
    iL.push_back( 0.999999 );
    iN.push_back( 6 );
    setProbabilityLevels( iL, iN );
}

void VLombScargle::setProbabilityLevels( vector< double > iProbabilityLevels )
{
    fProbabilityLevels = iProbabilityLevels;
}

void VLombScargle::setProbabilityLevels( vector< double > iProbabilityLevels, vector< int > iProbabilityLevelDigits )
{
    fProbabilityLevels = iProbabilityLevels;
    fProbabilityLevelDigits = iProbabilityLevelDigits;
}

/*

   calculate powers for the given range of frequencies

*/
void VLombScargle::fillPeriodigram( bool iShuffle )
{
    fVPeriodigram.clear();
    fVFrequency.clear();

    double iMean = getFlux_Mean();
    double iVar  = getFlux_Variance();

    if( iMean < -1.e98 || iVar == 0. )
    {
        return;
    }

    double f = 0.;
    double w = 0.;
    double tau = 0.;

    for( unsigned int i = 0; i < fNFrequencies; i++ )
    {
        // frequency
        f  =  fFrequency_min + ( double )i * ( fFrequency_max - fFrequency_min ) / (( double )fNFrequencies );
        f += 0.5 * ( fFrequency_max - fFrequency_min ) / (( double )fNFrequencies );
        w  = 2.* TMath::Pi() * f;

        // tau
        double i_sin = 0.;
        double i_cos = 0.;
        for( unsigned j = 0; j < fLightCurveData.size(); j++ )
        {
            if( fLightCurveData[j] )
            {
                i_sin += TMath::Sin( 2.*w* fLightCurveData[j]->getMJD() );
                i_cos += TMath::Cos( 2.*w* fLightCurveData[j]->getMJD() );
            }
        }
        tau = TMath::ATan2( i_sin, i_cos ) / 2. / w;

        // LS power

        double i_A_num = 0.;
        double i_A_den = 0.;
        double i_B_num = 0.;
        double i_B_den = 0.;
        double i_wtau  = 0.;
        double i_fdev  = 0.;

        for( unsigned j = 0; j < fLightCurveData.size(); j++ )
        {
            unsigned int k = j;
            // shuffle light curve for toy MC
            if( iShuffle && fRandom )
            {
                k = fRandom->Integer( fLightCurveData.size() );
            }
            if( fLightCurveData[j] && fLightCurveData[k] )
            {
                i_wtau = w * ( fLightCurveData[k]->getMJD() - tau );
                i_fdev = fLightCurveData[j]->fFlux - iMean;

                i_A_num += i_fdev * TMath::Cos( i_wtau );

                i_A_den += TMath::Cos( i_wtau ) * TMath::Cos( i_wtau );

                i_B_num += i_fdev * TMath::Sin( i_wtau );

                i_B_den += TMath::Sin( i_wtau ) * TMath::Sin( i_wtau );
            }
        }
        if( i_A_den > 0. && i_B_den > 0. )
        {
            fVFrequency.push_back( f );
            fVPeriodigram.push_back(( i_A_num* i_A_num / i_A_den + i_B_num* i_B_num / i_B_den ) / 2. / iVar );
        }
    }
}


TGraph* VLombScargle::getPeriodigramGraph()
{
    fPeriodigramGraph = new TGraph( 1 );
    setGraphPlottingStyle( fPeriodigramGraph, 1 );

    unsigned int z = 0;
    for( unsigned int i = 0; i < fVFrequency.size(); i++ )
    {
        if( i < fVPeriodigram.size() )
        {
            fPeriodigramGraph->SetPoint( z, fVFrequency[i], fVPeriodigram[i] );
            z++;
        }
    }
    return fPeriodigramGraph;
}

TH1D* VLombScargle::getPeriodigramHistogram( string iName )
{
    cout << "creating histogram for " << fNFrequencies << "(" << fFrequency_min << "," << fFrequency_max << ")" << endl;
    fPeriodigramHisto = new TH1D( iName.c_str(), "", fNFrequencies, fFrequency_min, fFrequency_max );
    setHistogramPlottingStyle( fPeriodigramHisto, 1, 1., 1., 1, 1, 0 );
    fPeriodigramHisto->GetYaxis()->SetTitleOffset( 1.1 );

    for( unsigned int i = 0; i < fVFrequency.size(); i++ )
    {
        if( i < fVPeriodigram.size() )
        {
            fPeriodigramHisto->SetBinContent( fPeriodigramHisto->FindBin( fVFrequency[i] ), fVPeriodigram[i] );
        }
    }
    return fPeriodigramHisto;
}

void VLombScargle::setFrequencyRange( unsigned int iNFrequencies, double iFrequency_min, double iFrequency_max )
{
    fNFrequencies = iNFrequencies;
    fFrequency_min = iFrequency_min;
    fFrequency_max = iFrequency_max;
}

void VLombScargle::plotProbabilityLevels( bool iPlotinColor )
{
    if(!fPeriodigramCanvas )
    {
        return;
    }
    if(!fPeriodigramHisto )
    {
        return;
    }

    fPeriodigramCanvas->cd();

    // trials = number of frequencies
    double i_z = 0.;
    char hname[100];

    for( unsigned int i = 0; i < fProbabilityLevels.size(); i++ )
    {
        i_z = -1.*log( 1. - exp( log( fProbabilityLevels[i] ) / (( double )fNFrequencies ) ) );

        if( i_z > fPeriodigramHisto->GetMinimum() && i_z < fPeriodigramHisto->GetMaximum() )
        {
            TLine* i_L = new TLine( fPeriodigramHisto->GetXaxis()->GetXmin(), i_z, fPeriodigramHisto->GetXaxis()->GetXmax(), i_z );
            i_L->SetLineStyle( 2 );
            if( iPlotinColor )
            {
                i_L->SetLineColor( i + 1 );
            }
            i_L->Draw();
            if( i < fProbabilityLevelDigits.size() )
            {
                sprintf( hname, "%.*f%%", fProbabilityLevelDigits[i], fProbabilityLevels[i] );
            }
            else
            {
                sprintf( hname, "%f%%", fProbabilityLevels[i] );
            }
            TText* iT = new TText( 0.8 * fPeriodigramHisto->GetXaxis()->GetXmax(), i_z, hname );
            iT->SetTextSize( 0.025 );
            iT->Draw();
        }
    }
}

/*

    calculate the probability levels with help of a toy MC
    (test if time structure, e.g. gaps, influence the periodigram)

    shuffle times and fluxes randomly (iMCCycles times)


*/
void VLombScargle::plotProbabilityLevelsFromToyMC( unsigned int iMCCycles, unsigned int iSeed, bool iPlotinColor )
{
    if(!fPeriodigramCanvas )
    {
        return;
    }
    if(!fPeriodigramHisto )
    {
        return;
    }

    if(!fRandom )
    {
        fRandom = new TRandom3();
    }
    fRandom->SetSeed( iSeed );

    // 2D histogram for counting
    double y_max = 1000.;
    if( fPeriodigramHisto->GetMaximum() > 0. )
    {
        y_max = fPeriodigramHisto->GetMaximum();
    }
    TH2D hC( "hC", "", fNFrequencies, fFrequency_min, fFrequency_max, 10000, 0., y_max );

    // shuffle light curves and fill histogram
    for( unsigned int i = 0; i < iMCCycles; i++ )
    {
        if( i % 500 == 0 )
        {
            cout << "filling MC cycle " << i << endl;
        }
        fillPeriodigram( true );

        for( unsigned int j = 0; j < fVFrequency.size(); j++ )
        {
            hC.Fill( fVFrequency[j], fVPeriodigram[j] );
        }
    }

    // calculate probability levels
    cout << "calculating probability levels" << endl;

    TCanvas* c = new TCanvas( "c", "Toy MC probability level calculation (debug histogram)", 400, 500, 500, 500 );
    c->SetGridx( 0 );
    c->SetGridy( 0 );
    hC.DrawCopy( "colz" );

    for( unsigned int i = 0; i < fProbabilityLevels.size(); i++ )
    {
        TGraph* iG = new TGraph( 1 );
        if( iPlotinColor )
        {
            setGraphPlottingStyle( iG, i + 1, 1., 1, 1., 0, 3 );
        }
        else
        {
            setGraphPlottingStyle( iG );
        }
        unsigned int z = 0;

        for( int j = 1; j <= hC.GetXaxis()->GetNbins(); j++ )
        {
            TH1D* iH = ( TH1D* )hC.ProjectionY( "_hy", j, j + 1 );
            iH = get_Cumulative_Histogram( iH, true, true );

            for( int k = 1; k <= iH->GetXaxis()->GetNbins(); k++ )
            {
                if( TMath::Abs( iH->GetBinContent( k ) - 1. ) > 1.e-10 )
                {
                    // trials
                    double iProb = TMath::Power( iH->GetBinContent( k ), ( double )fNFrequencies );
                    if( iProb > fProbabilityLevels[i] )
                    {
                        double iY = iH->GetBinLowEdge( k );
                        iG->SetPoint( z, hC.GetBinCenter( j ), iY );
                        z++;
                        break;
                    }
                }
            }
        }
        if( iG->GetN() > 1 )
        {
            fPeriodigramCanvas->cd();
            iG->Draw( "l" );
        }
    }
}


void VLombScargle::plotPeriodigram( string iXTitle, string iYTitle, bool bLogX )
{
    char hname[800];
    char htitle[800];
    sprintf( hname, "cLS_%d_%d_%d", fNFrequencies, ( int )fFrequency_min, ( int )fFrequency_max );
    sprintf( htitle, "Lomb-Scargle Periodigram (n_{f}=%d, f_{min}=%.1f, f_{max}=%.1f", fNFrequencies, fFrequency_min, fFrequency_max );

    if(!fPeriodigramCanvas )
    {
        fPeriodigramCanvas = new TCanvas( hname, htitle, 10, 10, 500, 500 );
        fPeriodigramCanvas->SetGridx( 0 );
        fPeriodigramCanvas->SetGridy( 0 );
        if( bLogX )
        {
            fPeriodigramCanvas->SetLogx( 1 );
        }
        fPeriodigramCanvas->Draw();
    }
    else
    {
        fPeriodigramCanvas->cd();
    }

    fillPeriodigram();

    sprintf( hname, "hLS_%d_%d_%d", fNFrequencies, ( int )fFrequency_min, ( int )fFrequency_max );
    TH1D* hLS = getPeriodigramHistogram( hname );

    if( hLS )
    {
        if( iXTitle.size() > 0 )
        {
            hLS->SetXTitle( iXTitle.c_str() );
        }
        if( iYTitle.size() > 0 )
        {
            hLS->SetYTitle( iYTitle.c_str() );
        }
        hLS->GetXaxis()->SetNdivisions( 505 );
        hLS->Draw();
    }
}

void VLombScargle::plotFrequencyLine( double iFrequencyLine_plot, int iColor )
{
    if(!fPeriodigramCanvas )
    {
        return;
    }
    if(!fPeriodigramHisto )
    {
        return;
    }

    if( iFrequencyLine_plot > 0. )
    {
        TLine* iL = new TLine( iFrequencyLine_plot, fPeriodigramHisto->GetMinimum(), iFrequencyLine_plot, fPeriodigramHisto->GetMaximum() );
        iL->SetLineColor( iColor );
        iL->SetLineStyle( 2 );
        iL->SetLineWidth( 2 );
        iL->Draw();
    }
}
