/*! \class VSensitivityCalculator
    \brief calculate sensitivity from data rates, Monte Carlo, or energy spectra

## plot and list observation time necessary for a given source strength

- input: gamma-ray, background rates from Crab Nebula data, background normalisation parameter

VSensitivityCalculator a;
// set gamma-ray, background rates from Crab Nebula data, background normalisation parameter
a.addDataSet( 4.7, 0.21, 0.1, "A" );
// set range of source strengths to be covered (in Crab Units)
a.setSourceStrengthRange_CU(1.e-3, 40. );
// set source strengths (in Crab Units) to be plotted with guided lines
vector< double > v;
v.push_back( 1. ); v.push_back( 0.1 ); v.push_back( 0.03 ); v.push_back( 0.01 );
a.setSourceStrengthVector_CU( v );

// plot observation time necessary for a given source strength
a.plotObservationTimevsFlux( 0, 0, 2 );
// list observation time necessary for a given source strength
a.list_sensitivity( 0 );

## plot integral and differential sensitivity vs energy using a reconstructed Crab spectrum

- input: anasum results file from Crab analysis

VSensitivityCalculator a;
// read Crab spectrum and plot integral sensitivity in Crab Unit (CU)
// (replace "CU" by "PFLUX" for particle flux [1/m2/s] or energy flux "ENERGY' [erg/cm2/s])
TCanvas *c = a.plotIntegralSensitivityvsEnergyFromCrabSpectrum(0, "myCrabFile.root", 1, "CU" )
// plot a sensitivity curve from literature on top of it
a.plotSensitivityvsEnergyFromTextTFile( c, "SensitivityFromTextFile_CU", 2, 2, 2, "CU" );

// plot differential sensitivity (4 bins per decade)
a.plotDifferentialSensitivityvsEnergyFromCrabSpectrum( 0, "myCrabFile.root", 1, "ENERGY", 0.25 );

## plot integral and  differential sensitivity vs energy using MC

-input: MC gamma and proton effective area

VSensitivityCalculator a;
// set MC parameter for gamma-ray simulations
a.setMonteCarloParameters(1, "../data/TeV_data/EnergySpectrum_literatureValues.dat", 1, "/Users/Shared/data/AGIS/Set-Ia/effectiveAreas/CT_SignalNoise_StdCuts_NT2/effectiveArea-CFG1-0100m-ID1.root" );
// set MC parameter for proton simulations
a.setMonteCarloParameters(14, "../data/TeV_data/EnergySpectrum_literatureValues_CR.dat", 0, "/Users/Shared/data/AGIS/Set-Ib/effectiveAreas/effectiveArea-CFG1-0100m-ID1.root", 20., 0, 0.5, 150, 2.5, 2. );
// plot integral sensitivity
a.plotIntegralSensitivityvsEnergyFromCrabSpectrum( 0, "MC" );

Revision $Id: VSensitivityCalculator.cpp,v 1.1.2.3.4.2.2.13.2.22.4.5 2011/02/15 16:12:56 gmaier Exp $

\author Gernot Maier
*/

#include "VSensitivityCalculator.h"

ClassImp(VSensitivityCalculator)

VSensitivityCalculator::VSensitivityCalculator()
{
    fConstant_Flux_To_Ergs = TMath::Qe() / 1.e-7;
    setDebug( false );

    fEnergySpectrumfromLiterature = 0;
    fEnergySpectrumfromLiterature_ID = 0;

    hnull = 0;

    reset();
}


void VSensitivityCalculator::reset()
{
    fCurrentDataSet = 0;

    fPlotDebugName = "";

// Li & Ma formula for significance calculation (use 17, 5 or 9)
    fLiAndMaEqu = 17;

// source strength values (in Crab units)
// units: fraction of source strenght
    fSourceStrength_min = 0.003;
    fSourceStrength_max = 3.5;
    fSourceStrength_step = 0.005;
    setSourceStrengthVector_CU();
// values of Crab fluxes to be plotted as lines into the sensitivity vs energy graph (in Crab Units)
    fPlottingCrabFlux_CU.push_back( 1.e0 );
    fPlottingCrabFlux_CU.push_back( 1.e-1 );
    fPlottingCrabFlux_CU.push_back( 1.e-2 );
    fPlottingCrabFlux_CU.push_back( 1.e-3 );
    fPlottingCrabFlux_CU.push_back( 1.e-4 );

// sensitivity graph
    gSensitivityvsEnergy = 0;

    setPlotCanvasSize();
    setEnergyRange_Log();
    setFluxRange_PFLUX();
    setFluxRange_ENERG();
    setFluxRange_CU();
    setObservationTimeRange();
    setSignificanceParameter();
    setBackgroundMissingParticleFraction();

    fGraphObsvsTime.clear();
    fData.clear();
}


/*
   iD = dat set ID
   iObservationTime = total observation time [h]
   iSignificance    = required significance  [sigma]
   iMinEvents       = minimum number of events required

   energy           = energy on linear scale [TeV]

   return sensitivity as fraction of data set used
*/
double VSensitivityCalculator::getSensitivity( unsigned int iD, double iObservationTime, double iSignificance, double iMinEvents, double iMinBackgroundEvents, double energy )
{
    if( !checkDataSet( iD, "getSensitivity" ) ) return 0.;

    double f = 0.;
    double s = 0.;
    double t = iObservationTime * 60.;            // h -> min

// fSignal = gamma-ray + background rates in source region
// fBackground = background counts in off source regions
// (observe that this is different from the standard definition used in addDataSet() (which are rates, etc))

// calculate signal (non - alpha * noff) rate
    double n_diff = fData[iD].fSignal - fData[iD].fBackground * fData[iD].fAlpha;

    if( fDebug && energy > 0. )
    {
        cout << "getSensitivity " << iD << " energy: " << energy << "\t obstime: " << iObservationTime;
	cout << "\t sig > " << iSignificance << "\t events > " << iMinEvents;
	cout << "\t min number of background events > " << iMinBackgroundEvents << endl;
        cout << "\t signal+background rate: " << fData[iD].fSignal;
	cout << "\t background events: " << fData[iD].fBackground;
	cout << "\t background rate: " <<  fData[iD].fBackground*fData[iD].fAlpha << "\t alpha: " << fData[iD].fAlpha << endl;
	cout << "\t signal rate: " << n_diff;
	cout << "\t nsourcestrengths " << fSourceStrength.size() << endl;
    }

    bool bSuccess = false;

    for( unsigned int n = fSourceStrength.size()-1; n > 0; n-- )
    {
        f = fSourceStrength[n];

        s = VStatistics::calcSignificance( t * ( f * n_diff + fData[iD].fBackground * fData[iD].fAlpha), t*fData[iD].fBackground, fData[iD].fAlpha, fLiAndMaEqu );

// keep statistics to show where the limitations are
	if( energy > 0. )
	{
	   int i_energy = (int)(energy*1.e3);
	   if( s < iSignificance )                                fSignificanceLimited[i_energy] = s;
	   if( s > iSignificance && t * f * n_diff < iMinEvents ) fMinEventsLimited[i_energy] = t * f * n_diff;
	   if( s > iSignificance &&  t * fData[iD].fBackground < fBackgroundEvents_min ) fMinBackgroundEventsLimited[i_energy] = t * fData[iD].fBackground;
	}

// require a certain significance and a minimum number of events
// often as well required: n_diff > 0.05 N_bck (TODO)
//        if( s > iSignificance && t * f * n_diff > iMinEvents && fData[iD].fBackground > fBackgroundEvents_min )
        if( s > iSignificance && t * f * n_diff > iMinEvents )
        {
            if( n > 0 )
            {
                f = fSourceStrength[n];
                bSuccess = true;
		if( fDebug && energy > 0. )
		{
		    cout << "\t n: " << n-1 << "\t f " << f;
		    cout << "\t significance: " << s;
		    cout << "\t min events: " << t * f * fData[iD].fSignal;
		    cout << "\t ndiff: " << t * ( f * n_diff );
		    cout << "\t non: " << t * ( f * n_diff + fData[iD].fBackground * fData[iD].fAlpha);
		    cout << "\t noff: " << t * fData[iD].fBackground;
		    cout << "\t alpha: " << fData[iD].fAlpha;
		    cout << "\t t: " << t;
		    cout << endl;
                }
            }
            break;
        }
	else
	{
// print out reasons for failure on above cut:
/*	    if( fDebug )
            {
	       if( s > iSignificance )
	       {
		 cout << "\t PASSED SIGNIFICANCE " << endl;
		 cout << "\t n: " << n-1 << "\t f " << f;
		 cout << "\t significance: " << s << " (" << iSignificance << ")";
		 cout << "\t min events: " << t * f * fData[iD].fSignal << " (" << iMinEvents << ")";
		 cout << "\t bck fraction: " << b << " (" << fBackgroundEvents_min << ")";
		 cout << "\t ndiff: " << t * ( f * n_diff );
		 cout << "\t non: " << t * ( f * n_diff + fData[iD].fBackground * fData[iD].fAlpha);
		 cout << "\t noff: " << t * fData[iD].fBackground;
		 cout << "\t alpha: " << fData[iD].fAlpha;
		 cout << "\t t: " << t;
		 cout << endl;
               }
           } */
	}
    }

    if( !bSuccess ) return -1.;

    return f;
}


bool VSensitivityCalculator::checkDataSet( unsigned int iD, string iName )
{
    if( iD >= fData.size() )
    {
        cout << "ERROR (" << iName << "): data set ID out of range: should be < " << fData.size() << endl;
        return false;
    }
    return true;
}


unsigned int VSensitivityCalculator::listDataSets()
{
    cout << "Available data sets " << fData.size() << ": " << endl;
    cout << "==================== " << endl;
    cout << "Signal rate [1/min] \t Background rate [1/min] \t Alpha \t Name " << endl;
    cout << "--------------------\t-------------------------\t-------\t------" << endl;
    for( unsigned int i = 0; i < fData.size(); i++ )
    {
        cout << fData[i].fSignal << "\t\t\t";
        cout << fData[i].fBackground << "\t\t\t";
        cout << fData[i].fAlpha << "\t\t";
        cout << fData[i].fName << endl;
    }
    cout << endl;

    return fData.size();
}


bool VSensitivityCalculator::setCurrentDataSet( unsigned int iD )
{
    if( !checkDataSet( iD, "setCurrentDataSet" ) ) return false;

    fCurrentDataSet = iD;

    return true;
}


bool VSensitivityCalculator::removeDataSet( unsigned int iD )
{
    if( !checkDataSet( iD, "removeDataSet" ) ) return false;

    fData.erase( fData.begin() + iD );

    return true;
}


/*!
     iGammaRayRate = signal rate [1/min]
     iBackGroundRate = background rate [1/min]
     iAlpha = normalization parameter
     iName  = name of data set
*/
unsigned int VSensitivityCalculator::addDataSet( double iGammaRayRate, double iBackGroundRate, double iAlpha, string iName )
{
    sSensitivityData t;

    t.fSignal = iGammaRayRate;
    t.fBackground = iBackGroundRate;
    t.fAlpha = iAlpha;
    t.fName = iName;

    fData.push_back( t );

    return fData.size() - 1;
}


/*!
    fSourceStrength will be sorted in reverse order
*/
void VSensitivityCalculator::setSourceStrengthRange_CU( double iMin, double iMax, double iStep, bool iLog )
{
    fSourceStrength.clear();

    fSourceStrength_min = iMin;
    fSourceStrength_max = iMax;
    fSourceStrength_step = iStep;

    unsigned int i_Steps = (unsigned int)( (fSourceStrength_max-fSourceStrength_min)/fSourceStrength_step ) + 1;
    for( unsigned int n = 0; n < i_Steps; n++ )
    {
        fSourceStrength.push_back( fSourceStrength_max - n * fSourceStrength_step );
    }

    if( iLog )
    {
        for( unsigned int i = 0; i < fSourceStrength.size(); i++ )
        {
            fSourceStrength[i] = TMath::Power( 10., fSourceStrength[i] );
        }
    }
}

/*
   
    set the source strength vector and sort it in reverse order

*/
void VSensitivityCalculator::setSourceStrengthVector_CU( vector< double > iF )
{
    fSourceStrength = iF;

// sort in reverse order
    sort( fSourceStrength.begin(), fSourceStrength.end() );
    reverse( fSourceStrength.begin(), fSourceStrength.end() );
}

TGraph* VSensitivityCalculator::getCrabSpectrum( bool bInt, string bUnit, bool bReset )
{
    vector< double > iTemp;
    iTemp.push_back( 1. );

    vector< TGraph* > i_Temp_Graph = getCrabSpectrum( iTemp, bInt, bUnit, bReset );
    if( i_Temp_Graph.size() == 1 ) return i_Temp_Graph[0];

    return 0;
}

/*!
     fill graphs with Crab-like spectra and different source strengths

     i_fCrabFlux:  vector of Crab fluxes in CU units [1. = 1 Crab ]
     bInt:         integrated or differential flux
     bUnit:        PFLUX, CU, ENERGY
     bReset:       recalculate the graphs (otherwise: graph is not filled if the number of graph requested is the same as CrabFlux_SourceStrength.size() )

*/
vector< TGraph* > VSensitivityCalculator::getCrabSpectrum( vector< double > i_fCrabFlux, bool bInt, string bUnit, bool bReset )
{
    if( fDebug ) cout << "VSensitivityCalculator::getCrabSpectrum " << i_fCrabFlux.size() << endl;

// check if Crab spectrum is already defined
    if( i_fCrabFlux.size() == fCrabFlux_SourceStrength.size() && !bReset ) return fCrabFlux_SourceStrength;

// temporary function describing Crab spectrum
    TF1* i_fFunCrabFlux = 0;
// temporary graph    
    TGraph *i_GraphCrabFlux = new TGraph( 100 );

// hardwired Crab spectra
// (better: use spectral from external text file)
    if( fEnergySpectrumfromLiterature == 0 )
    {
       char hname[800];

       unsigned int i_CrabSpectrum_ID = 1;              // NOTE: hardwired to 1!

// Whipple Crab spectrum (1989)
       if( i_CrabSpectrum_ID == 0 )
       {
	  double i_gamma = -2.49;
	  double i_Norm = 3.20e-11;
	  sprintf( hname, "%e*TMath::Power( x, %f)", i_Norm, i_gamma );
       }
// MAGIC curved Crab spectrum
       else if( i_CrabSpectrum_ID == 1 )
       {
	  sprintf( hname, "6.0e-10*TMath::Power( x/0.3, -2.31 - 0.26*log10( x/0.3 ) )" );
       }
       if( bUnit == "PFLUX" )       sprintf( hname, "%s", hname );
       else if( bUnit == "ENERGY" ) sprintf( hname, "%s * 1.e12 * x * x * %f", hname, fConstant_Flux_To_Ergs );
       else                         sprintf( hname, "%f", 1. );
       i_fFunCrabFlux = new TF1( "i_fFunCrabFlux" , hname, TMath::Power( 10., fEnergy_min_Log ), 10000. );
    }
// use spectrum from text file
    else
    {
       if( bUnit == "PFLUX" )
       {
           i_fFunCrabFlux = fEnergySpectrumfromLiterature->getEnergySpectrum( fEnergySpectrumfromLiterature_ID, false, TMath::Power( 10., fEnergy_min_Log ), 10000. );
       }
       else if( bUnit == "CU" )
       {
          i_fFunCrabFlux = new TF1( "i_fFunCrabFlux" , "1.", TMath::Power( 10., fEnergy_min_Log ), 10000. );
       }
       else if( bUnit == "ENERGY" )
       {

       }
    }
    if( !i_fFunCrabFlux )
    {
	cout << "Error reading Crab Nebula spectrum " << fEnergySpectrumfromLiterature << "\t" << fEnergySpectrumfromLiterature_ID << endl;
	vector< TGraph * > xx;
	return xx;
    }
/////////////////////////////////////////////////////
// calculate integral spectrum
    if( bInt )
    {
       const int np = 1000;
       double *x = new double[np];
       double *y = new double[np];
       double iE = 0.;

       i_fFunCrabFlux->CalcGaussLegendreSamplingPoints( np, x, y, 1.e-14 );
// loop over different Crab flux
       for( int p = 0; p < i_GraphCrabFlux->GetN(); p++ )
       {
// equal intervall in logE
	  iE = fEnergy_min_Log + p * (fEnergy_max_Log-fEnergy_min_Log)/i_GraphCrabFlux->GetN();
// integrate above this energy
	  i_GraphCrabFlux->SetPoint( p, iE, i_fFunCrabFlux->IntegralFast( np, x, y, TMath::Power( 10., iE ), 10000. ) );
       }
       delete x;
       delete y;
    }
/////////////////////////////////////////////////////
// calculate differental spectrum
    else
    {
        double iE = 0.;
// loop over different Crab fluxes
       for( int p = 0; p < i_GraphCrabFlux->GetN(); p++ )
       {
// equal intervall in logE
	  iE = fEnergy_min_Log + p * (fEnergy_max_Log-fEnergy_min_Log)/i_GraphCrabFlux->GetN();
// differential flux
	  i_GraphCrabFlux->SetPoint( p, iE, i_fFunCrabFlux->Eval( TMath::Power( 10., iE ) ) );
       }
    }

// now fill a vector with graph for the different flux values needed
    double xx = 0.;
    double yy = 0.;
    fCrabFlux_SourceStrength.clear();
    for( unsigned int i = 0; i < i_fCrabFlux.size(); i++ )
    {
	fCrabFlux_SourceStrength.push_back( new TGraph( i_GraphCrabFlux->GetN() ) );
	for( int p = 0; p < i_GraphCrabFlux->GetN(); p++ )
	{
	      i_GraphCrabFlux->GetPoint( p, xx, yy );
	      fCrabFlux_SourceStrength.back()->SetPoint( p, xx, yy * i_fCrabFlux[i] );
        }
    } 
    if( fDebug ) cout << "VSensitivityCalculator::getCrabSpectrum " << fCrabFlux_SourceStrength.size() << endl;

    return fCrabFlux_SourceStrength;
}


/*

    calculate and plot integral sensitivity

*/
TCanvas* VSensitivityCalculator::plotIntegralSensitivityvsEnergyFromCrabSpectrum( TCanvas *cSensitivity, string iAnasumCrabFile,
                                                                                  int iColor, string bUnit,
										  double iEnergyMin_TeV_lin, double iEnergyMax_TeV_lin )
{
   return plotSensitivityvsEnergyFromCrabSpectrum( cSensitivity, iAnasumCrabFile, iColor, bUnit, -1., iEnergyMin_TeV_lin, iEnergyMax_TeV_lin );
}


/*

    calculate and plot differential sensitivity

*/
TCanvas* VSensitivityCalculator::plotDifferentialSensitivityvsEnergyFromCrabSpectrum( TCanvas *cSensitivity, string iAnasumCrabFile,
                                                                                      int iColor, string bUnit,
										      double dE_Log10, double iEnergyMin_TeV_lin, double iEnergyMax_TeV_lin )
{
   return plotSensitivityvsEnergyFromCrabSpectrum( cSensitivity, iAnasumCrabFile, iColor, bUnit, dE_Log10, iEnergyMin_TeV_lin, iEnergyMax_TeV_lin );
}


/*

    calculate and plot sensitivities

*/
TCanvas* VSensitivityCalculator::plotSensitivityvsEnergyFromCrabSpectrum( TCanvas *cSensitivity, string iAnasumCrabFile,
                                                                          int iColor, string bUnit,
									  double dE_Log10, double iEnergyMin_TeV_lin, double iEnergyMax_TeV_lin )
{
    if( !checkUnits( bUnit ) )
    {
	cout << "VSensitivityCalculator::plotSensitivityvsEnergyFromCrabSpectrum: error while checking units: " << bUnit << endl;
	return 0;
    }
    if( dE_Log10 > 0. ) cout << "plotting differential sensitivity" << endl;
    else                cout << "plotting integral sensitivity" << endl;

// get vector with integral and differential Crab-like spectra for different flux levels
// (use Whipple spectrum)
    TGraph* i_fFunCrabFlux = 0;
    if( bUnit != "CU" && (dE_Log10 > 10.) )      i_fFunCrabFlux = getCrabSpectrum( true, bUnit, true );
    else if( bUnit != "CU" && (dE_Log10 < 10.) ) i_fFunCrabFlux = getCrabSpectrum( true, "PFLUX", true );
    else                                         i_fFunCrabFlux = getCrabSpectrum( (dE_Log10 < 10.), bUnit, true );
//    if( bUnit == "CU" ) i_fFunCrabFlux = getCrabSpectrum( true, bUnit, true );
//    else                i_fFunCrabFlux = getCrabSpectrum( true, "PFLUX", true );
/*    if( bUnit != "CU" && (dE_Log10 > 10.) )
    {
       i_fFunCrabFlux = getCrabSpectrum( true, bUnit, true );
       cout << "A" << endl;
    }
    else if( bUnit != "CU" && (dE_Log10 < 10.) )
    {
       i_fFunCrabFlux = getCrabSpectrum( true, "PFLUX", true );
       cout << "B" << endl;
    }
    else
    {
       i_fFunCrabFlux = getCrabSpectrum( (dE_Log10 < 10.), bUnit, true );
       cout << "C" << endl;
    } */
    if( !i_fFunCrabFlux )
    {
       cout << "VSensitivityCalculator::plotSensitivityvsEnergyFromCrabSpectrum: error: no reference spectrum found " << endl;
       return 0;
    }

// prepare debugging plots
    if( fPlotDebugName.size() > 0 ) prepareDebugPlots();

//////////////////////////////////////////////////////////////////////////////////////
// get differential flux vector (used for differential and integral flux calculations)
    double alpha = 1.;
    vector< VDifferentialFlux > fDifferentialFlux;
    if( iAnasumCrabFile == "MC" )
    {
	fDifferentialFlux = getDifferentialFluxVectorfromMC( dE_Log10, alpha );
    }
    else
    {
	fDifferentialFlux = getDifferentFluxVectorfromData( iAnasumCrabFile, dE_Log10, alpha );
    }
    if( fPlotDebugName.size() > 0 ) plotDebugPlotsParticleNumbers( fDifferentialFlux, alpha );

    if( fDifferentialFlux.size() == 0 )
    {
	cout << "Error: no entries in differential flux vector" << endl;
	return 0;
    }

// this is the range of fluxes which will be searched
    setSourceStrengthRange_CU( -4., 0., 4./1000., true );

    double non = 0;
    double non_error = 0.;
    double noff = 0;
    double noff_error = 0.;
    double s = 0.;
    double s_error_U = 0.;
    double s_error_L = 0.;

// create sensitivity vs energy graph
    gSensitivityvsEnergy = new TGraphAsymmErrors( 1 );
    gSensitivityvsEnergy->SetTitle( "" );
    setGraphPlottingStyle( gSensitivityvsEnergy, iColor, 4., 28, 1.1, 1001 );

/////////////////////////////////////////////////////////////////
// loop over all on and off counts and calculate sensitivity
    int z = 0;
    for( int i = fDifferentialFlux.size() - 1; i >= 0; i-- )
    {
// integral spectrum: add number of events
	if( dE_Log10 < 0. )
	{
	    non  += fDifferentialFlux[i].NOn;
	    noff += fDifferentialFlux[i].NOff;
	    non_error  += fDifferentialFlux[i].NOn_error * fDifferentialFlux[i].NOn_error;
	    noff_error += fDifferentialFlux[i].NOff_error * fDifferentialFlux[i].NOff_error;
	}
// differential spectrum: number of events per differential bin
	else
	{
	    non  = fDifferentialFlux[i].NOn;
	    noff = fDifferentialFlux[i].NOff;
	    non_error  = fDifferentialFlux[i].NOn_error * fDifferentialFlux[i].NOn_error;
	    noff_error = fDifferentialFlux[i].NOff_error * fDifferentialFlux[i].NOff_error;
	}
	non_error  = sqrt( non_error );
	noff_error = sqrt( noff_error );

// skip all zero entries
	if( TMath::Abs( non ) < 1.e-2 && TMath::Abs( noff ) < 1.e-2 ) continue;

// observe that the signal rate is defined differently for list_sensitivity() etc...
	unsigned int iD = addDataSet( non / fDifferentialFlux[i].ObsTime * 60., noff / fDifferentialFlux[i].ObsTime * 60., alpha, "" );
// get fraction of Crab flux for XX sigma etc...
	s = getSensitivity( iD, fObservationTime, fSignificance_min, fEvents_min, fBackgroundEvents_min, fDifferentialFlux[i].Energy );
// get error on sensitivity estimate
	iD = addDataSet( (non+non_error) / fDifferentialFlux[i].ObsTime * 60., (noff-noff_error) / fDifferentialFlux[i].ObsTime * 60. , alpha, "" );
	s_error_U = getSensitivity( iD, fObservationTime, fSignificance_min, fEvents_min, fBackgroundEvents_min );
	iD = addDataSet( (non-non_error) / fDifferentialFlux[i].ObsTime * 60., (noff+noff_error) / fDifferentialFlux[i].ObsTime * 60. , alpha, "" );
	s_error_L = getSensitivity( iD, fObservationTime, fSignificance_min, fEvents_min, fBackgroundEvents_min );

// fill sensitivity graphs
	double energy = TMath::Log10( fDifferentialFlux[i].EnergyWeightedMean );
        if(    i_fFunCrabFlux != 0 && s > 0.
	    && fDifferentialFlux[i].Energy > iEnergyMin_TeV_lin && fDifferentialFlux[i].Energy < iEnergyMax_TeV_lin
	    && noff > fBackgroundEvents_min )
        {
//            energy = TMath::Log10( fDifferentialFlux[i].Energy );
            energy = TMath::Log10( fDifferentialFlux[i].EnergyWeightedMean );
// integral sensitivity
            if( dE_Log10 < 0. )
            {
                gSensitivityvsEnergy->SetPoint( z, energy, i_fFunCrabFlux->Eval( energy ) * s );
                gSensitivityvsEnergy->SetPointEYhigh( z, i_fFunCrabFlux->Eval( energy ) * TMath::Abs( s - s_error_U ) );
                gSensitivityvsEnergy->SetPointEYlow( z, i_fFunCrabFlux->Eval( energy ) * TMath::Abs( s - s_error_L ) );
            }
// differential sensitivity
            else
            {
                double f1 = i_fFunCrabFlux->Eval( log10( fDifferentialFlux[i].Energy_lowEdge ) );
                double f2 = i_fFunCrabFlux->Eval( log10( fDifferentialFlux[i].Energy_upEdge ) ); 
                if( bUnit == "PFLUX" )
                {
                    gSensitivityvsEnergy->SetPoint( z, energy, ( f1 - f2 ) * s / fDifferentialFlux[i].dE );
                }
		else if( bUnit == "CU" )
		{
                    gSensitivityvsEnergy->SetPoint( z, energy, s );
		    gSensitivityvsEnergy->SetPointEYhigh( z, TMath::Abs( s - s_error_U ) );
		    gSensitivityvsEnergy->SetPointEYlow(  z, TMath::Abs( s - s_error_L ) );
                }
                else if( bUnit == "ENERGY" )
                {
                    gSensitivityvsEnergy->SetPoint( z, energy,
		                                    ( f1 - f2 ) * s / fDifferentialFlux[i].dE * 
						    fDifferentialFlux[i].EnergyWeightedMean * fDifferentialFlux[i].EnergyWeightedMean *
						    1.e12 * fConstant_Flux_To_Ergs );
    cout << "XXX " << ( f1 - f2 ) * s / fDifferentialFlux[i].dE * fDifferentialFlux[i].EnergyWeightedMean * fDifferentialFlux[i].EnergyWeightedMean *  1.e12 * fConstant_Flux_To_Ergs << "\t" << f1 << "\t" << f2 << endl;
                }
		else
		{
		   cout << "plotSensitivityvsEnergyFromCrabSpectrum error: unknown unit: " << bUnit << endl;
		   return 0;
                }
            }
            z++;
        }
// print some debugging information
	if( fDebug )
	{
	     cout << fixed << "FLUX RESULTS: " << z << "\t" << fDifferentialFlux[i].EnergyWeightedMean << " [TeV]\t";
	     cout << fDifferentialFlux[i].Energy << " [TeV]";
	     if( dE_Log10 > 0. ) cout << " dE_log10: " << fDifferentialFlux[i].dE << "\t";
	     cout << "[" << fDifferentialFlux[i].Energy_lowEdge << ", " << fDifferentialFlux[i].Energy_upEdge << "]\t";
	     cout << "[" << log10(fDifferentialFlux[i].Energy_lowEdge) << ", " << log10(fDifferentialFlux[i].Energy_upEdge) << "]\t";
	     cout << s << "(" << s_error_L << "," << s_error_U << ") [CU]";
	     cout << endl;
	     cout << "\t NON: " << non << "(" << non_error << ")";
	     cout << "\t NOFF: " << noff << "(" << noff_error << ")\t";
	     cout << fDifferentialFlux[i].ObsTime << " [s]\t" << scientific;
	     if( bUnit == "PFLUX" )       cout <<  i_fFunCrabFlux->Eval( energy ) * s << " [cm^-2s^-1] (" << energy << ")";
	     else if( bUnit == "ENERGY" ) cout <<  i_fFunCrabFlux->Eval( energy ) * s << " [erg cm^-2s^-1]";
             cout << endl;
        }
    }

//////////////////////////////////////////////////////////////////////////////////////////////////
// plot
//////////////////////////////////////////////////////////////////////////////////////////////////

// get canvas
    if( cSensitivity == 0 )
    {
        cSensitivity = plotCanvas_SensitivityvsEnergy( bUnit, (dE_Log10 < 0. ) );
	if( !cSensitivity ) return 0;
    }
    cSensitivity->cd();

// plot everything
    if(  dE_Log10 < 0. ) gSensitivityvsEnergy->Draw( "l3" );
    else                 gSensitivityvsEnergy->Draw( "lp" );

    if( fDebug ) gSensitivityvsEnergy->Print();

    cSensitivity->Update();

    return cSensitivity;
}

bool VSensitivityCalculator::printSensitivity()
{
   if( gSensitivityvsEnergy )
   {
      cout << "Contents of sensitivity graph: " << endl;
      gSensitivityvsEnergy->Print();
      return true;
   }
   
   return false;
}


/*!
    plot an empty canvas for the sensitivy vs energy graphs

    add lines corresponding to XXX% of the Crab

*/
TCanvas* VSensitivityCalculator::plotCanvas_SensitivityvsEnergy( string bUnit, bool bIntegralSensitivity )
{
     char htitle[400];
     TCanvas *iCanvas = new TCanvas( "iCanvas", "sensitivity vs energy", 10, 10, fPlot_CanvasSize_x, fPlot_CanvasSize_y );
     iCanvas->SetGridx( 0 );
     iCanvas->SetGridy( 0 );
     iCanvas->SetLeftMargin( 0.15 );

     hnull = new TH1D( "hnullSens", "", 10, fEnergy_min_Log, fEnergy_max_Log );
     hnull->SetStats( 0 );
// integral sensitivity
     if( bIntegralSensitivity )
     {
	 hnull->SetXTitle( "log_{10} energy E_{t} [TeV]" );
	 if( bUnit == "PFLUX" )
	 {
	     hnull->SetYTitle( "Integral Flux Sensitivity (E>E_{t}) [cm^{-2} s^{-1}]" );
	     hnull->SetMaximum( fPlot_flux_PFLUX_max );
	     hnull->SetMinimum( fPlot_flux_PFLUX_min );
	 }
	 else if( bUnit == "ENERGY" )
	 {
	     hnull->SetYTitle( "E_{t} x Integral Flux Sensitivity (E>E_{t}) [erg cm^{-2} s^{-1}]" );
	     hnull->SetMaximum( fPlot_flux_ENERG_max );
	     hnull->SetMinimum( fPlot_flux_ENERG_min );
	 }
	 else if( bUnit == "CU" )
	 {
	     hnull->SetYTitle( "Integral Flux Sensitivity (E>E_{t}) [C.U.]" );
	     hnull->SetMaximum( fPlot_flux_CU_max );
	     hnull->SetMinimum( fPlot_flux_CU_min );
	 }
     }
// differential sensitivity
     else
     {

	 hnull->SetXTitle( "log_{10} energy [TeV]" );
	 if( bUnit == "PFLUX" )
	 {
	     hnull->SetYTitle( "Flux Sensitivity [cm^{-2} s^{-1} TeV^{-1}]" );
	     hnull->SetMaximum( fPlot_flux_PFLUX_max );
	     hnull->SetMinimum( fPlot_flux_PFLUX_min );
	 }
	 else if( bUnit == "ENERGY" )
	 {
	     hnull->SetYTitle( "E^{2} x Flux Sensitivity [erg cm^{-2} s^{-1}]" );
	     hnull->SetMaximum( fPlot_flux_ENERG_max );
	     hnull->SetMinimum( fPlot_flux_ENERG_min );
	 }
	 else if( bUnit == "CU" )
	 {
	     hnull->SetYTitle( "Differential Flux Sensitivity  [C.U.]" );
	     hnull->SetMaximum( fPlot_flux_CU_max );
	     hnull->SetMinimum( fPlot_flux_CU_min );
	 }
     }

     plot_nullHistogram( iCanvas, hnull, true, true, 1.7, TMath::Power( 10., fEnergy_min_Log ), TMath::Power( 10., fEnergy_max_Log ) );
     iCanvas->SetLogy( 1 );

// get vector with integral or differential Crab-like spectra for different flux levels
     vector< TGraph* > i_fFunCrabFlux = getCrabSpectrum( fPlottingCrabFlux_CU, bIntegralSensitivity, bUnit, false );

     for( unsigned int i = 0; i < fPlottingCrabFlux_CU.size(); i++ )
     {
	 if( i < i_fFunCrabFlux.size() && i_fFunCrabFlux[i] )
	 {
	    i_fFunCrabFlux[i]->Draw( "l" );

	    if( bUnit != "CU" )
	    {
		if( fPlottingCrabFlux_CU[i]*100. >=1. )         sprintf( htitle, "%d%% Crab", (int)(fPlottingCrabFlux_CU[i]*100) );
		else if( fPlottingCrabFlux_CU[i]*100. > 0.09 )  sprintf( htitle, "%.1f%% Crab", fPlottingCrabFlux_CU[i]*100 );
		else if( fPlottingCrabFlux_CU[i]*100. > 0.009 ) sprintf( htitle, "%.2f%% Crab", fPlottingCrabFlux_CU[i]*100 );
		double e = 1. - 0.3 * (double)i;
		TText *iT = new TText( e, i_fFunCrabFlux[i]->Eval( e ) * 1.1, htitle );
		iT->SetTextSize( 0.3 * iT->GetTextSize() );
		iT->Draw();
	    }
         }
     }
     iCanvas->Update();

     return iCanvas;
}


/*!
    units used in data files should be encoded into the file name

    CU  = Crab units
    PFLUX = particle flux [1/cm2/s]
    ENERG = energy flux [erg/cm2/s]

    energy in TeV

   TODO: conversion of different units
*/
TCanvas* VSensitivityCalculator::plotSensitivityvsEnergyFromTextTFile( TCanvas *c, string iTextFile,
                                                                       int iColor, double iLineWidth, int iLineStyle,
								       string bUnit, bool bIntegralSensitivity )
{
// check if canvas exists, otherwise create a new one
    if( c == 0 ) 
    {
        c = plotCanvas_SensitivityvsEnergy( bUnit, bIntegralSensitivity );
	if( !c ) return 0;
    }
    c->cd();

// graph with sensitivity values: read them directly from the text file
    TGraph *g = new TGraph( gSystem->ExpandPathName( iTextFile.c_str() ) );
    cout << "reading " << iTextFile << " with " << g->GetN() << " data points." << endl;
    if( g->IsZombie() || g->GetN() < 1 )
    {
       cout << "VSensitivityCalculator::plotSensitivityvsEnergyFromTextTFile warning: ";
       cout << " no data points found" << endl;
       return c;
    }
    g->SetLineColor( iColor );
    g->SetLineWidth( (Width_t)iLineWidth );
    g->SetLineStyle( iLineStyle );

// check what units the data is in
    string i_fileUnit = "PFLUX";
    if( iTextFile.find( "ENERGY" ) < iTextFile.size() )   i_fileUnit = "ENERGY";
    else if( iTextFile.find( "CU" ) < iTextFile.size() )  i_fileUnit = "CU";

// convert energy to log10( energy )
    double i_Energy = 0.;
    double i_fluxSensitivity = 0.;
    for( int i = 0; i < g->GetN(); i++ )
    {
        g->GetPoint( i, i_Energy, i_fluxSensitivity );
// convert flux
        if( bUnit == "ENERGY" && i_fileUnit != "ENERGY" ) i_fluxSensitivity *= i_Energy * 1.e12 * fConstant_Flux_To_Ergs;
        i_Energy = log10( i_Energy );
        g->SetPoint( i, i_Energy, i_fluxSensitivity );
    }
// draw sensitivity graph
    g->Draw( "c" );

    c->Update();

    return c;
}

/*

   list the available units for flux sensitivity calculations

*/
void VSensitivityCalculator::listUnits()
{
    cout << "Available units for sensitivity plot: " << endl;
    cout << "\t CU    = Crab units" << endl;
    cout << "\t PFLUX = particle flux [1/cm2/s]" << endl;
    cout << "\t ENERG = energy flux [erg/cm2/s]" << endl;
}


void VSensitivityCalculator::setEnergyRange_Log( double iN, double iX )
{
    fEnergy_min_Log = iN;
    fEnergy_max_Log = iX;
}


void VSensitivityCalculator::setEnergyRange_Lin( double iN, double iX )
{
    fEnergy_min_Log = log10( iN );
    fEnergy_max_Log = log10( iX );
}


void VSensitivityCalculator::setSignificanceParameter( double iSignificance, double iMinEvents, double iObservationTime, double iBackgroundEvents_min )
{
    fSignificance_min = iSignificance;
    fEvents_min = iMinEvents;
    fObservationTime = iObservationTime;
    fBackgroundEvents_min = iBackgroundEvents_min;
}


void VSensitivityCalculator::setObservationTimeRange( double iObs_min, double iObs_max, int iObs_steps )
{
    fObservationTime_min = iObs_min;
    fObservationTime_max = iObs_max;
    fObservationTime_steps = iObs_steps;
}


void VSensitivityCalculator::plotObservationTimevsFluxFromTextFile( TCanvas*c, string iTextFile, int iLineColor, double iLineWidth, int iLineStyle )
{
    if( !c ) return;

    TGraph *g = new TGraph( iTextFile.c_str() );
    g->SetLineColor( iLineColor );
    g->SetLineWidth( (Width_t)iLineWidth );
    g->SetLineStyle( iLineStyle );

    cout << "reading " << iTextFile << " with " << g->GetN() << " data points" << endl;

    g->Draw( "c" );
}


TCanvas* VSensitivityCalculator::plotObservationTimevsFlux( unsigned int iD, TCanvas *c, int iLineColor, double iLineWidth )
{
    if( !checkDataSet( iD, "plotObservationTimevsFlux" ) ) return 0;

    calculateObservationTimevsFlux( iD );

    bool bNewCanvas = ( c == 0 );
    if( c == 0 )
    {
        c = new TCanvas( "cF", "flux vs time", 510, 10, 400, 400 );
        c->SetGridx( 0 );
        c->SetGridy( 0 );
        c->SetLogx( 1 );
        c->SetLogy( 1 );
        c->SetLeftMargin( 0.12 );
        c->SetRightMargin( 0.09 );
        c->Draw();
    }
    setGraphPlottingStyle( fGraphObsvsTime[iD], iLineColor, iLineWidth );

    fGraphObsvsTime[iD]->SetTitle( "" );
    if( bNewCanvas ) fGraphObsvsTime[iD]->Draw( "ac" );
    else             fGraphObsvsTime[iD]->Draw( "c" );
    fGraphObsvsTime[iD]->GetHistogram()->SetYTitle( "observation time [h]" );
    fGraphObsvsTime[iD]->GetHistogram()->SetXTitle( "flux [Crab Units]" );
    fGraphObsvsTime[iD]->GetHistogram()->GetYaxis()->SetTitleOffset( 1.3 );
    if( fDebug ) fGraphObsvsTime[iD]->Print();

    if( bNewCanvas )
    {
        for( unsigned int i = 0; i < fSourceStrength.size(); i++ )
        {
            plot_guidingLines( fSourceStrength[i], fGraphObsvsTime[iD], (fSourceStrength[i] < 0.1 ) );
        }
    }

    return c;
}


void VSensitivityCalculator::list_sensitivity( unsigned int iD )
{
    if( !checkDataSet( iD, "plotObservationTimevsFlux" ) ) return ;

    calculateObservationTimevsFlux( iD );

    cout << " Flux               time           time" << endl;
    cout << " [Crab Units]       [min]           [h]" << endl;
    cout << " ========================================" << endl;

    for( unsigned int i = 0; i < fSourceStrength.size(); i++ )
    {
        cout << " " << fSourceStrength[i] << "\t\t" << setw( 8 ) << setprecision( 3 );
        cout << fGraphObsvsTime[iD]->Eval( fSourceStrength[i] ) * 60. << "\t" << setw( 8 ) << setprecision( 3 );
        cout << fGraphObsvsTime[iD]->Eval( fSourceStrength[i] );
        cout << endl;
    }

    cout << "(requiring a significance of at least " << fSignificance_min << " sigma or " << fEvents_min;
    cout << " events, and using Li & Ma formula " << fLiAndMaEqu << ")" << endl;
}


double VSensitivityCalculator::calculateObservationTimevsFlux( unsigned int iD )
{
    if( !checkDataSet( iD, "plotObservationTimevsFlux" ) ) return 0.;

    fGraphObsvsTime[iD] = new TGraph( 100 );

    double s = 0.;
    double t = 0.;
    double x = 0.;
    double iG = fData[iD].fSignal;
    double iB = fData[iD].fBackground;
    double alpha = fData[iD].fAlpha;
    if( alpha > 0. ) iB /= alpha;
    else             return 0.;

    if( fDebug ) cout << "Rates for ID " << iD << ", " << iG << ", " << iB << ", " << alpha << endl;

    int z = 0;
    int z_max = fGraphObsvsTime[iD]->GetN();
    for( int i = 0; i < z_max; i++ )
    {
// take logarithmic steps in flux
        x = TMath::Log10( fSourceStrength_min ) + ( TMath::Log10( fSourceStrength_max ) - TMath::Log10( fSourceStrength_min ) ) / (double)fGraphObsvsTime[iD]->GetN() * (double)i;
        x = TMath::Power( 10., x );

// loop over possible observation lengths
        bool bSuccess = false;
        for( int j = 0; j < fObservationTime_steps; j++ )
        {
            t = TMath::Log10( fObservationTime_min ) + ( TMath::Log10( fObservationTime_max ) - TMath::Log10( fObservationTime_min ) ) / (double)fObservationTime_steps * (double)j;
            t = TMath::Power( 10., t ) * 60.;

            s = VStatistics::calcSignificance( iG*t*x + iB*t*alpha, iB*t, alpha, fLiAndMaEqu );

            if( s > fSignificance_min && t*x*iG > fEvents_min )
            {
                bSuccess = true;
                break;
            }
        }
        if( bSuccess )
        {
            fGraphObsvsTime[iD]->SetPoint( z, x, t / 60. );
            z++;
        }
    }

    return fGraphObsvsTime[iD]->Eval( fSignificance_min );
}


void VSensitivityCalculator::plot_guidingLines( double x, TGraph *g, bool iHours )
{
    if( !g ) return;

    double i_y = g->Eval( x );
    TLine *iL_x = new TLine( x, 0., x, i_y );
    iL_x->SetLineStyle( 2 );
    iL_x->SetLineColor( 2 );
    iL_x->Draw();
    TLine *iL_y = new TLine( 0., i_y, x, i_y );
    iL_y->SetLineStyle( 4 );
    iL_y->SetLineColor( 2 );
    iL_y->Draw();

    char hname[600];
    if( iHours ) sprintf( hname, "%d%% Crab in %d h", (int)(x*100.), (int)(i_y+0.5) );
    else         sprintf( hname, "%d%% Crab in %d min", (int)(x*100.), (int)(i_y*60.+0.5) );
    TText *iT = new TText( x*1.5, i_y, hname );
    iT->SetTextSize( iT->GetTextSize() * 0.8 );
    iT->SetTextAngle( 45. );
    iT->SetTextColor( 2 );
    iT->Draw();
}


void VSensitivityCalculator::setSourceStrengthVector_CU()
{
    fSourceStrength.clear();
    fSourceStrength.push_back( 1. );   // this is 100% Crab
    fSourceStrength.push_back( 0.30 );
    fSourceStrength.push_back( 0.10 );
    fSourceStrength.push_back( 0.05 );
    fSourceStrength.push_back( 0.03 );
    fSourceStrength.push_back( 0.01 );
}


bool VSensitivityCalculator::checkUnits( string bUnit )
{
    if( bUnit != "CU" && bUnit != "PFLUX" && bUnit != "ENERGY" ) return false;

    return true;
}


vector< VDifferentialFlux > VSensitivityCalculator::getDifferentialFluxVectorfromMC_ErrorMessage( string i_message )
{
    cout << "VSensitivityCalculator::getDifferentialFluxVectorfromMC error: " << i_message << endl;

    vector< VDifferentialFlux > a;

    return a;
}


/*
    calculate differential or integral flux from gamma and proton effective areas

*/
vector< VDifferentialFlux > VSensitivityCalculator::getDifferentialFluxVectorfromMC( double dE_Log10, double &iNorm )
{
    vector< VDifferentialFlux > a;
    iNorm = 0.;
// iterator with MC data
    char hname[800];
    map< unsigned int, VSensitivityCalculatorDataResponseFunctions* >::iterator i_MCData_iterator;

///////////////////////////////////////////////////////////////////
// check if data is complete (need gamma-ray data)
    if( fMC_Data.find( 1 ) == fMC_Data.end() ) 
    {
       return getDifferentialFluxVectorfromMC_ErrorMessage( "no gamma-ray MC data given" );
    }
    if( fMC_Data.size() < 2 )   // (need at least one background data set)
    {
       return getDifferentialFluxVectorfromMC_ErrorMessage( "MC Data vector not large enough (should be >=2)" );
    }

///////////////////////////////////////////////////////////////////
// differential flux vector (for gammas and background particles)
    vector< VDifferentialFlux > v_flux;

///////////////////////////////////////////////////////////////////
// get Crab spectrum from literature ( index [1] is gamma-ray)
    VEnergySpectrumfromLiterature i_Crab( fMC_Data[1]->fSpectralParameterFile );
    cout << "\t reading Crab Nebula spectrum with ID" <<  fMC_Data[1]->fSpectralParameterID << endl;
    if( i_Crab.isZombie() ) return a;
    i_Crab.listValues( fMC_Data[1]->fSpectralParameterID );

///////////////////////////////////////////////////////////////////
// get effective areas for gamma-rays and background
    
    for( i_MCData_iterator = fMC_Data.begin(); i_MCData_iterator != fMC_Data.end(); i_MCData_iterator++ )
    {
       if( !getMonteCarlo_EffectiveArea( (*i_MCData_iterator).second ) ) return a;
    }

///////////////////////////////////////////////////////////////////
// set up energy binning for differential flux vector

///////////////////////////////////////////////////////////////////
// binning in gamma and proton effective areas must be the same (number of bins, range, and bin width)
    for( i_MCData_iterator = fMC_Data.begin(); i_MCData_iterator != fMC_Data.end(); i_MCData_iterator++ )
    {
        if( (*i_MCData_iterator).first == 1 ) continue;

        if( fMC_Data[1]->effArea_Ebins != (*i_MCData_iterator).second->effArea_Ebins )
	{
	   return getDifferentialFluxVectorfromMC_ErrorMessage( "diffent number of bins in gamma and background effective areas" );
        }
        if( TMath::Abs( fMC_Data[1]->effArea_Emin - (*i_MCData_iterator).second->effArea_Emin ) > 0.05
	 || TMath::Abs( fMC_Data[1]->effArea_Emax - (*i_MCData_iterator).second->effArea_Emax ) > 0.05 )
	{
	   return getDifferentialFluxVectorfromMC_ErrorMessage( "different energy axis definition in gamma and background effective areas" );
        }
        if( (*i_MCData_iterator).second->energy.size() == 0 )
	{
	   sprintf( hname, "VSensitivityCalculator::getDifferentialFluxVectorfromMC error: effective area (%s) vector with length 0", (*i_MCData_iterator).second->fName.c_str() );
	   return getDifferentialFluxVectorfromMC_ErrorMessage( hname );
        }
     }

// integral sensitivity: take existing energy binning (from gammas)
    double iBinSize = (fMC_Data[1]->effArea_Emax - fMC_Data[1]->effArea_Emin ) / fMC_Data[1]->effArea_Ebins;
// offset between gamma and background effective areas 
// (offset in effective area vector, first filled value)
    map< unsigned int, int > iEnergyScaleOffset;
    for( i_MCData_iterator = fMC_Data.begin(); i_MCData_iterator != fMC_Data.end(); i_MCData_iterator++ )
    {
        if( (*i_MCData_iterator).first == 1 ) continue;
	iEnergyScaleOffset[(*i_MCData_iterator).first] = (int)( ( fMC_Data[1]->energy[0] - (*i_MCData_iterator).second->energy[0] ) / iBinSize );
    }

///////////////////////////////////////////////////////////////////
// GAMMA RAYS (signal)
///////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// INTEGRAL SENSITIVITY
////////////////////////////////////////////////////////////////
    if( dE_Log10 < 0. )
    {
        for( unsigned int i = 0; i < fMC_Data[1]->energy.size(); i++ )
        {
            VDifferentialFlux i_flux;
// energy bins in sensitivity curve
            i_flux.Energy_lowEdge = fMC_Data[1]->energy[i] - iBinSize/2.;
            i_flux.Energy_upEdge  = fMC_Data[1]->energy[i] + iBinSize/2.;
            i_flux.Energy_lowEdge_bin = i;
            i_flux.Energy_upEdge_bin = i;
// energies are on linear scale
            i_flux.Energy_lowEdge = TMath::Power( 10., i_flux.Energy_lowEdge );
            i_flux.Energy_upEdge  = TMath::Power( 10., i_flux.Energy_upEdge );

            i_flux.dE = i_flux.Energy_upEdge - i_flux.Energy_lowEdge;
// simplified (maybe should somehow be weighted by the shape of the energy spectrum)
            i_flux.Energy = (i_flux.Energy_lowEdge + i_flux.Energy_upEdge)/2.;
            i_flux.EnergyWeightedMean = i_flux.Energy;
// convert from [h] to [s]
            i_flux.ObsTime = fObservationTime * 60. * 60.;

            v_flux.push_back( i_flux );
            if( fDebug )
            {
                cout << "ENERGY: " << v_flux.size() << "\t" << i_flux.Energy_lowEdge << " - " << i_flux.Energy_upEdge;
		cout << "\t" << i_flux.Energy << "\t" << i_flux.EnergyWeightedMean;
                cout << "\t" << i_flux.dE << "\t" << i_flux.ObsTime << "\t" << fObservationTime << "  Off: ";
		map< unsigned int, int >::iterator iEnergyScaleOffset_iter;
		for( iEnergyScaleOffset_iter = iEnergyScaleOffset.begin(); iEnergyScaleOffset_iter != iEnergyScaleOffset.end(); iEnergyScaleOffset_iter++ )
		{
		   cout << (*iEnergyScaleOffset_iter).second << "\t";
                }
		cout << endl;
            }
        }
    }
////////////////////////////////////////////////////////////////
// DIFFERENTIAL SENSITIVITY
////////////////////////////////////////////////////////////////
    else
    {
// dE_Log10 must be a multiple of effective area energy bin size
        if( dE_Log10 < iBinSize )
	{
	   return getDifferentialFluxVectorfromMC_ErrorMessage( "VSensitivityCalculator::getDifferentialFluxVectorfromMC error: dE_log10 smaller than bin size of effective areas" );
        }
        if( TMath::Abs( dE_Log10 / iBinSize - (int)(dE_Log10 / iBinSize) ) > 1.e-2 )
	{
	   return getDifferentialFluxVectorfromMC_ErrorMessage( "VSensitivityCalculator::getDifferentialFluxVectorfromMC error: dE_Log10 must be a multiple of effective area energy bin size" );
        }

        double iBinEnergyMin = fMC_Data[1]->effArea_Emin;
// get minimum energy bin
        while( iBinEnergyMin < fMC_Data[1]->energy[0] - iBinSize/2. )
        {
            iBinEnergyMin += dE_Log10;
        }
        if( TMath::Abs( iBinEnergyMin - ( fMC_Data[1]->energy[0] - iBinSize/2. ) ) > 1.e-2 ) iBinEnergyMin += dE_Log10;

// now fill vector with energies and energy bins
        while( iBinEnergyMin < fMC_Data[1]->effArea_Emax )
        {
            VDifferentialFlux i_flux;

            bool i_eff_found = false;
// lower energy bin
            for( unsigned int i = 0; i < fMC_Data[1]->energy.size(); i++ )
            {
                if( TMath::Abs( fMC_Data[1]->energy[i] - iBinSize/2. - iBinEnergyMin ) < 1.e-2 )
                {
                    i_flux.Energy_lowEdge = fMC_Data[1]->energy[i] - iBinSize/2.;                    // temporary: Energy_lowEdge should be on lin scale, here it is on log scale
                    i_flux.Energy_lowEdge_bin = i;
                    i_eff_found = true;
                    break;
                }
            }
            if( i_eff_found )
            {
// upper energy bin
                if( i_flux.Energy_lowEdge_bin + int(dE_Log10/iBinSize) < fMC_Data[1]->energy.size() )
                {
                    i_flux.Energy_upEdge     = i_flux.Energy_lowEdge + dE_Log10;
                    i_flux.Energy_upEdge_bin = i_flux.Energy_lowEdge_bin + int(dE_Log10/iBinSize);
// energies are on linear scale 
                    i_flux.Energy_lowEdge = TMath::Power( 10., i_flux.Energy_lowEdge );
                    i_flux.Energy_upEdge  = TMath::Power( 10., i_flux.Energy_upEdge );

                    i_flux.dE = i_flux.Energy_upEdge - i_flux.Energy_lowEdge;
// simplified
                    i_flux.Energy = (i_flux.Energy_lowEdge + i_flux.Energy_upEdge)/2.;
                    i_flux.EnergyWeightedMean = i_flux.Energy;
// convert from [h] to [s]
                    i_flux.ObsTime = fObservationTime * 60. * 60.;

                    v_flux.push_back( i_flux );
                    if( fDebug )
                    {
                        cout << "ENERGY: " << v_flux.size() << "\t" << i_flux.Energy_lowEdge << " - " << i_flux.Energy_upEdge << "\t" << i_flux.Energy << "\t" << i_flux.EnergyWeightedMean;
                        cout << "\t" << i_flux.dE << "\t" << i_flux.ObsTime << "\t" << log10( i_flux.Energy_lowEdge ) << "\t" << log10( i_flux.Energy_upEdge ) << "\t";
			map< unsigned int, int >::iterator iEnergyScaleOffset_iter;
			for( iEnergyScaleOffset_iter = iEnergyScaleOffset.begin(); iEnergyScaleOffset_iter != iEnergyScaleOffset.end(); iEnergyScaleOffset_iter++ )
			{
			   cout << (*iEnergyScaleOffset_iter).second << "\t";
			}
			cout << endl;
                    }
                }
            }
            iBinEnergyMin += dE_Log10;
        }
    }

///////////////////////////////////////////////////////////////////
// get gamma rate [1/min] for a certain ze, az, noise, wobble offset
// for integral sensitivity set energy bins according to effective areas
//      purgeEnergies( fMC_Data[1]->energy, v_flux );
    for( unsigned int i = 0; i < v_flux.size(); i++ )
    {
        v_flux[i].NOn       = getMonteCarlo_Rate( v_flux[i].Energy_lowEdge_bin, v_flux[i].Energy_upEdge_bin, i_Crab, *fMC_Data[1], false );
	v_flux[i].NOn_error = getMonteCarlo_Rate( v_flux[i].Energy_lowEdge_bin, v_flux[i].Energy_upEdge_bin, i_Crab, *fMC_Data[1], true );
    }


///////////////////////////////////////////////////////////////////
// COSMIC RAYS (background)
///////////////////////////////////////////////////////////////////

   map< unsigned int, vector< double > > v_flux_NOff;
   map< unsigned int, vector< double > > v_flux_NOff_error;

// loop over all background files
   for( i_MCData_iterator = fMC_Data.begin(); i_MCData_iterator != fMC_Data.end(); i_MCData_iterator++ )
   {
      if( (*i_MCData_iterator).first == 1 ) continue;   // ignore gamma rays

///////////////////////////////////////////////////////////////////
// get CR spectrum from literature
      VEnergySpectrumfromLiterature i_CR( (*i_MCData_iterator).second->fSpectralParameterFile );
      cout << "\t reading CR spectrum with ID" <<  (*i_MCData_iterator).second->fSpectralParameterID << endl;
      if( i_CR.isZombie() ) return a;
      i_CR.listValues( (*i_MCData_iterator).second->fSpectralParameterID );

// get CR rate for a certain ze, az, noise, wobble offset
       for( unsigned int i = 0; i < v_flux.size(); i++ )
       {
           v_flux_NOff[(*i_MCData_iterator).first].push_back( 0. );
           v_flux_NOff_error[(*i_MCData_iterator).first].push_back( 0. );
       }

//      purgeEnergies( (*i_MCData_iterator).second->energy, v_flux );
// loop over all energy bins
       for( unsigned int i = 0; i < v_flux.size(); i++ )
       {
           if( iEnergyScaleOffset[(*i_MCData_iterator).first] + (int)v_flux[i].Energy_lowEdge_bin >= 0 && iEnergyScaleOffset[(*i_MCData_iterator).first] + (int)v_flux[i].Energy_lowEdge_bin < (int)(*i_MCData_iterator).second->energy.size() )
           {
            v_flux_NOff[(*i_MCData_iterator).first][i]       =    getMonteCarlo_Rate( v_flux[i].Energy_lowEdge_bin 
	                                                        + iEnergyScaleOffset[(*i_MCData_iterator).first], v_flux[i].Energy_upEdge_bin
								+ iEnergyScaleOffset[(*i_MCData_iterator).first], i_CR, *(*i_MCData_iterator).second );
            v_flux_NOff_error[(*i_MCData_iterator).first][i] =    getMonteCarlo_Rate( v_flux[i].Energy_lowEdge_bin
	                                                        + iEnergyScaleOffset[(*i_MCData_iterator).first], v_flux[i].Energy_upEdge_bin
								+ iEnergyScaleOffset[(*i_MCData_iterator).first], i_CR, *(*i_MCData_iterator).second, true );
           }
	   else continue;

//////////////////////////////////////////////////////////////////////////////////////////
// take care of space angle and theta2 cut normalisation
//////////////////////////////////////////////////////////////////////////////////////////

// CR spectrum is given as dN/dt/dA/dOmega (Omega = space angle)
// multiply CR rate by space angle used in CORSIKA (scattering angle)
	   v_flux_NOff[(*i_MCData_iterator).first][i]       *= (*i_MCData_iterator).second->SolidAngle_MCScatterAngle;
	   v_flux_NOff_error[(*i_MCData_iterator).first][i] *= (*i_MCData_iterator).second->SolidAngle_MCScatterAngle;
// scale direction cut CR solid angle to gamma-ray solid angle 
// (CR theta2 might be larger than gamma-ray, simply to gain statistics under the assumption of a flat angular acceptance)
           double iSolidAngle_Gamma =  fMC_Data[1]->getSolidAngle_DirectionCut( v_flux[i].Energy );
	   double iSolidAngle_Bck   = (*i_MCData_iterator).second->getSolidAngle_DirectionCut( v_flux[i].Energy );
	   if( iSolidAngle_Bck > 0. )
	   {
	       v_flux_NOff[(*i_MCData_iterator).first][i]       *= iSolidAngle_Gamma / iSolidAngle_Bck;
	       v_flux_NOff_error[(*i_MCData_iterator).first][i] *= iSolidAngle_Gamma / iSolidAngle_Bck;
	       if( fDebug )
	       {
		  cout << "SOLID ANGLE GAMMA AND BCK: " << i;
		  cout << "\t" << iSolidAngle_Gamma << "\t" << iSolidAngle_Bck << "\t" << iSolidAngle_Gamma / iSolidAngle_Bck;
		  cout << "\t" << v_flux[i].Energy;
		  cout << endl;
               }
           }
// missing helium (etc) simulations
	   v_flux_NOff[(*i_MCData_iterator).first][i]       *= (1.+fMC_BackgroundMissingParticleFraction);
	   v_flux_NOff_error[(*i_MCData_iterator).first][i] *= (1.+fMC_BackgroundMissingParticleFraction);
        }
    }

//////////////////////////////////
// now sum up all background 
//////////////////////////////////

// add background rate to signal rate (Non is gamma-ray + background rate in signal region)
   map< unsigned int, vector< double > >::iterator v_flux_NOff_iter;
   for( unsigned int i = 0; i < v_flux.size(); i++ )
   {
       v_flux[i].NOff = 0.;
       for( v_flux_NOff_iter = v_flux_NOff.begin(); v_flux_NOff_iter != v_flux_NOff.end(); v_flux_NOff_iter++ )
       {
          if( i < (*v_flux_NOff_iter).second.size() )
	  {
	     v_flux[i].NOn  += (*v_flux_NOff_iter).second[i];
	     v_flux[i].NOff += (*v_flux_NOff_iter).second[i];
          }
       }
   }
// error calculation
   for( unsigned int i = 0; i < v_flux.size(); i++ )
   {
      v_flux[i].NOn_error = v_flux[i].NOn_error*v_flux[i].NOn_error;
      v_flux[i].NOff_error = 0.;
      for( v_flux_NOff_iter = v_flux_NOff_error.begin(); v_flux_NOff_iter != v_flux_NOff_error.end(); v_flux_NOff_iter++ )
      {
          if( i < (*v_flux_NOff_iter).second.size() ) v_flux[i].NOn_error  += (*v_flux_NOff_iter).second[i] * (*v_flux_NOff_iter).second[i];
          if( i < (*v_flux_NOff_iter).second.size() ) v_flux[i].NOff_error += (*v_flux_NOff_iter).second[i] * (*v_flux_NOff_iter).second[i];
      }
      v_flux[i].NOn_error  =  sqrt( v_flux[i].NOn_error );
      v_flux[i].NOff_error =  sqrt( v_flux[i].NOff_error );
   }
   if( fPlotDebugName.size() > 0 ) plotDebugPlotsBackgroundParticleNumbers( v_flux, v_flux_NOff, v_flux_NOff_error );

// calculate mean alpha

// scale by alpha (preliminary)
   double alpha = 0.;
   double zz = 0;
   for( i_MCData_iterator = fMC_Data.begin(); i_MCData_iterator != fMC_Data.end(); i_MCData_iterator++ )
   {
      if( (*i_MCData_iterator).first == 1 ) continue;

      alpha += (*i_MCData_iterator).second->alpha;
      zz++;
   }
   if( zz > 0. ) alpha /= zz;
   else          alpha  = 1.;

   for( unsigned int i = 0; i < v_flux.size(); i++ )
   {
        if( alpha > 0. )
	{
	   v_flux[i].NOff /= alpha;
	   v_flux[i].NOff_error /= alpha;
        }
    } 

    if( fPlotDebugName.size() > 0 ) plotEffectiveArea();

////////////////////////////////////////////////////
// calculate number of on/off events
////////////////////////////////////////////////////
    double iTotG = 0.;
    double iTotB = 0.;
    if( v_flux.size() > 0 )
    {
       for( unsigned int i = v_flux.size()-1; i > 0; i-- )
       { 
	   v_flux[i].NOn  *= fObservationTime * 60.;               // [min] -> [sec]
	   v_flux[i].NOff *= fObservationTime * 60.;
	   v_flux[i].NOn_error  *= fObservationTime * 60.;
	   v_flux[i].NOff_error *= fObservationTime * 60.;
	   iTotG += (v_flux[i].NOn - v_flux[i].NOff * alpha)/(fObservationTime * 60.);
	   iTotB += v_flux[i].NOff*alpha/(fObservationTime * 60.);
	   if( fDebug )
	   {
	       cout << "NUMBER OF MC ON/OFF EVENTS: " << i << "\t";
	       cout << v_flux[i].Energy_lowEdge << " - " << v_flux[i].Energy_upEdge << " TeV,\t";
	       cout << "Non-Noff: " << v_flux[i].NOn - v_flux[i].NOff * alpha  << "\t";
	       cout << "NON: " << v_flux[i].NOn << " (" << v_flux[i].NOn_error << "), ";
	       cout << "NOFF: " << v_flux[i].NOff << "(" << v_flux[i].NOff_error << "), "  << v_flux[i].NOff * alpha << "\t" << alpha << "\t";
	       cout << "RateG: " << iTotG << " [1/min], " << iTotB << " [1/min]";
	       cout << endl;
	   }
       }
    } 

////////////////////////////////////////////////////
    iNorm = alpha;  // (norm is a return value)
    return v_flux;
}
////////////////////////////////////////////////////
////////////////////////////////////////////////////


vector< VDifferentialFlux > VSensitivityCalculator::getDifferentFluxVectorfromData( string iAnasumCrabFile, double dE_Log10, double &iNorm )
{
// read energy spectrum from file
    VEnergySpectrum e( iAnasumCrabFile );
    if( e.isZombie() )
    {
        vector< VDifferentialFlux > a;
        iNorm = 0.;
        return a;
    }

    if( dE_Log10 < 0. ) e.setEnergyBinning( 0.10 );
    else                e.setEnergyBinning( dE_Log10 );
    e.setEnergyRangeLinear( 0., 1.e10 );
    e.setEnergyThresholdDefinition( 3 );
    e.setEnergyThreshold( 1.e-10 );
    e.setSignificanceParameters( -50., -1. );
    e.combineRuns();
    e.calculateDifferentialFluxes();

    iNorm = e.getTotalNormalisationFactor();

    cout << endl << "Differential fluxes and significances per differential bin:" << endl;
    e.printDifferentialFluxes();
    cout << endl;

    return e.getDifferentialFlux();
}


/* 
   set all Monte Carlo parameters for gammas or protons

   iParticleID                 corsika particle ID: 1  = gamma, 14 = proton
   iSpectralParameterFile      read source spectrum from this file (VEnergySpectrumfromLiterature file)
   iSpectralParameterID        spectrum ID to read from iSpectralParameterFile
   iGammaEffectiveAreaFile     file with effective areas
   ze, az, woff, noise, index  parameters for effective area search (to be read from iGammaEffectiveAreaFile)
   theta2_MCSatterAngle        background scattering angle [deg2]
   alpha                       ratio between signal and background regions
*/
void VSensitivityCalculator::setMonteCarloParameters( unsigned int iParticleID, string iSpectralParameterFile, unsigned int iSpectralParameterID, string iGammaEffectiveAreaFile, double ze, int az, double woff, int noise, double index, double alpha, double iEnergy_min_log, double iEnergy_max_log )
{
    if( fMC_Data.find( iParticleID ) != fMC_Data.end() )
    {
       cout << "VSensitivityCalculator::setMCParameters: particle with ID " << iParticleID << " already in set of MC parameters" << endl;
       cout << "\t ignoring input" << endl;
       return;
    }
// create a new MC data object
    VSensitivityCalculatorDataResponseFunctions *f = new VSensitivityCalculatorDataResponseFunctions();;
    fMC_Data[iParticleID] = f;

// fill data
    f->fParticleID = iParticleID;
    f->fSpectralParameterFile = gSystem->ExpandPathName( iSpectralParameterFile.c_str() );
    f->fSpectralParameterID = iSpectralParameterID;
    f->fEffectiveAreaFile = gSystem->ExpandPathName( iGammaEffectiveAreaFile.c_str() );
    f->ze = ze;
    f->az = az;
    f->woff = woff;
    f->noise = noise;
    f->index = index;
    f->alpha = alpha;
    f->energy_min_log = iEnergy_min_log;
    f->energy_max_log = iEnergy_max_log;

    if( iParticleID == 1 )
    {
        f->fName = "gamma";
    }
    else if( iParticleID == 2 )
    {
        f->fName = "electron";
    }
    else if( iParticleID == 14 )
    {
        f->fName = "proton";
    }
    else if( iParticleID == 402 )
    {
        f->fName = "helium";
    }
    else
    {
        cout << "VSensitivityCalculator::setMCParameters: unknown particle ID: " << iParticleID  << endl;
        return;
    }
}


/*!
   calculate rate (in [1/min]) from given spectral shape and effective areas
*/
double VSensitivityCalculator::getMonteCarlo_Rate( unsigned int iE_low, unsigned int iE_up, 
                                                   VEnergySpectrumfromLiterature i_Espec, 
						   VSensitivityCalculatorDataResponseFunctions iMCPara, bool iRateError )
{
    VMonteCarloRateCalculator iMCR;

// return error on rate (from error on effective area, which is derived from MC statistics)
    if( iRateError ) return iMCR.getMonteCarloRate( iMCPara.energy, iMCPara.effArea_error, &i_Espec, iMCPara.fSpectralParameterID, iE_low, iE_up, getDebug() );

// return rate calculated from MC effective areas
    return iMCR.getMonteCarloRate( iMCPara.energy, iMCPara.effArea, &i_Espec, iMCPara.fSpectralParameterID, iE_low, iE_up, getDebug() );
}


/*!
    fill effective area vector into VSensitivityCalculatorDataResponseFunctions
*/
bool VSensitivityCalculator::getMonteCarlo_EffectiveArea( VSensitivityCalculatorDataResponseFunctions *iMCPara )
{
//////////////////////////////////////////////////////////////////////////////////////
// read effective areas from file
//////////////////////////////////////////////////////////////////////////////////////
    TFile fEff( iMCPara->fEffectiveAreaFile.c_str() );
    if( fEff.IsZombie() )
    {
        cout << "VSensitivityCalculator::getMonteCarlo_EffectiveArea: cannot find effective area file " << iMCPara->fEffectiveAreaFile.c_str() << endl;
        return false;
    }
    TTree *t = (TTree*)fEff.Get( "fEffArea" );
    if( !t )
    {
        cout << "VSensitivityCalculator::getMonteCarlo_EffectiveArea: cannot find effective area tree in " << iMCPara->fEffectiveAreaFile.c_str() << endl;
        return false;
    }
    CEffArea *c = new CEffArea( t );
    if( !c ) return false;

    cout << endl;
    cout << "=================================================================================" << endl;
    cout << "reading effective areas for " << iMCPara->fName << " from " << iMCPara->fEffectiveAreaFile.c_str() << endl;
    cout << "\t total number of effective areas in this data file: " << c->fChain->GetEntries() << endl;

    iMCPara->energy.clear();
    iMCPara->effArea.clear();

    bool bFound = false;
    for( unsigned int i = 0; i < c->fChain->GetEntries(); i++ )
    {
        c->GetEntry( i );

        if( TMath::Abs( c->index - iMCPara->index ) > 1.e-2 ) continue;

        if( c->noise != iMCPara->noise ) continue;

        if( c->az != iMCPara->az ) continue;

// ignore everything else for non-gammas (wobble offsets and zenith angles)
        if( iMCPara->fParticleID == 1 )
        {
            if( TMath::Abs( c->ze - iMCPara->ze ) > 2. ) continue;
            if( TMath::Abs( c->Woff - iMCPara->woff ) > 0.05 ) continue;
        }

        cout << "\t found effective area " << i << " with " << c->nbins << " bins" << endl;
        bFound = true;

// fill effective areas vs reconstructed energy (log10!)
        for( int n = 0; n < c->Rec_nbins; n++ )
        {
            if( c->Rec_e0[n] < iMCPara->energy_min_log || c->Rec_e0[n] > iMCPara->energy_max_log ) continue;
            iMCPara->energy.push_back( c->Rec_e0[n] );
            iMCPara->effArea.push_back( c->Rec_eff[n] );
	    iMCPara->effArea_error.push_back( c->Rec_seff_L[n] );  // ignore upper and lower errors from effective area file
        }
// get global energy binning
        if( c->hEmc )
        {
            iMCPara->effArea_Ebins = c->hEmc->GetNbinsX();
            iMCPara->effArea_Emin  = c->hEmc->GetXaxis()->GetXmin();
            iMCPara->effArea_Emax  = c->hEmc->GetXaxis()->GetXmax();
	    cout << "\t global binning for effective areas: ";
	    cout << " nbin: " << iMCPara->effArea_Ebins;
	    cout << " emin: " << iMCPara->effArea_Emin;
	    cout << " emax: " << iMCPara->effArea_Emax;
	    cout << endl;
        }
        break;
    }
    if( !bFound ) cout << "\t no effective area found!" << endl;

//////////////////////////////////////////////////////////////////////////////////////
// read MC parameters in case run header is available
// (e.g. diffuse scatter angle used in simtel simulations)
//////////////////////////////////////////////////////////////////////////////////////
    VMonteCarloRunHeader *iMCHeader = (VMonteCarloRunHeader*)fEff.Get( "MC_runheader" );
    if( iMCHeader )
    {
       cout << "reading Monte Carlo header" << endl;
// diffuse scatter angle
       if( iMCHeader->diffuse )
       {
          iMCPara->theta2_MCScatterAngle = iMCHeader->viewcone[1]*iMCHeader->viewcone[1];
	  cout << "\t setting diffuse scattering angle to " << sqrt(iMCPara->theta2_MCScatterAngle) << " [deg]" << endl;
       }
    }
    else
    {
       cout << "WARNING: no Monte Carlo header found; setting diffuse scattering angle to 4 deg!" << endl;
       iMCPara->theta2_MCScatterAngle = 4.*4.;
    }
    iMCPara->SolidAngle_MCScatterAngle = 2. * TMath::Pi() * ( 1. - cos( sqrt( iMCPara->theta2_MCScatterAngle ) * TMath::Pi() / 180. ) );

//////////////////////////////////////////////////////////////////////////////////////
// calculate solid angle from analysis cuts (or from input parameter theta2)
//////////////////////////////////////////////////////////////////////////////////////
    VGammaHadronCuts* iCuts = (VGammaHadronCuts*)fEff.Get( "anaCuts" );
    if( iCuts )
    {
// theta2 might be energy dependent
       cout << "calculating solid angle from analysis cuts (might be energy dependent)" << endl;
       iMCPara->theta2_min = iCuts->getTheta2Cut_min();       // theta2 min assumed to be energy independent
       if( iMCPara->theta2_min < 0. ) iMCPara->theta2_min = 0.;

// solid angle vs energy is stored in a TGraph
// (note that lower cut on theta2 is not a function of energy)
       iMCPara->gSolidAngle_DirectionCut_vs_EnergylgTeV = new TGraph( 1000 );

//       if( fDebug && iCuts->getTheta2Cut_TMVA_max() )
       if( iCuts->getTheta2Cut_TMVA_max() )
       {
          iCuts->getTheta2Cut_TMVA_max()->Print();
       }

       double e = 0.;
       double iSolidAngle = 0.;
       double itheta2 = 0.;
       for( int i = 0; i < iMCPara->gSolidAngle_DirectionCut_vs_EnergylgTeV->GetN(); i++ )
       {
	  e = fEnergy_min_Log + i * (fEnergy_max_Log-fEnergy_min_Log)/iMCPara->gSolidAngle_DirectionCut_vs_EnergylgTeV->GetN();

// VGammaHadronCuts::getTheta2Cut_max work with lin E
	  itheta2 = iCuts->getTheta2Cut_max( TMath::Power( 10., e ) );
	  if( itheta2 > 0. )
          {
	     iSolidAngle  = 2. * TMath::Pi() * ( 1. - cos( sqrt( itheta2 ) * TMath::Pi() / 180. ) );
	     if( iMCPara->theta2_min > 0. ) iSolidAngle -= 2. * TMath::Pi() * ( 1. - cos( sqrt( iMCPara->theta2_min ) * TMath::Pi() / 180. ) );
          }
	  else
	  {
	     cout << "Error: no theta2 cut given " << endl;
	     return false;
          }
	  iMCPara->gSolidAngle_DirectionCut_vs_EnergylgTeV->SetPoint( i, e, iSolidAngle );
       }
    }

    return true;
}


void VSensitivityCalculator::purgeEnergies( vector< double > energy, vector< VDifferentialFlux > &v_flux )
{
// lowest energy in effective areas
    unsigned int iRemove = 0;

    if( energy.size() > 0 && v_flux.size() > 1 )
    {
        for( unsigned int i = 0; i < v_flux.size()-1; i++ )
        {
            if( v_flux[i+1].Energy_lowEdge <  TMath::Power( 10., energy[0] ) ) iRemove = i;
        }
        if( iRemove > 0 ) v_flux.erase( v_flux.begin(), v_flux.begin()+iRemove );
        cout << "purgeEnergies: removing " << iRemove << " low energy bins";
        if( v_flux.size() > 0 ) cout << ", lowest energy now: " << v_flux[0].Energy_lowEdge << " [TeV]";
        cout << endl;
    }

// highest energy in effective areas
    iRemove = 0;
    if( energy.size() > 0 && v_flux.size() > 0 )
    {
        for( unsigned int i = v_flux.size() - 1; i > 0; i-- )
        {
            if( v_flux[i].Energy_lowEdge >  TMath::Power( 10., energy[energy.size()-1] ) ) iRemove = i;
        }
        if( iRemove > 0 )
        {
            v_flux.erase( v_flux.begin()+iRemove, v_flux.end() );
            cout << "purgeEnergies removing " << v_flux.size() - iRemove << " high energy bins";
        }
        if( v_flux.size() > 0 ) cout << ", highest energy now: " << v_flux[v_flux.size()-1].Energy_lowEdge << " [TeV]";
        cout << endl;
    }
}


void VSensitivityCalculator::plotSignificanceParameters( TCanvas *cSensitivity )
{
    if( !cSensitivity )
    {
        cout << "warning: no sensitivity canvas" << endl;
        return;
    }

    char hname[400];
    sprintf( hname, "%.0f hours, %.0f #sigma, >%.0f events", fObservationTime, fSignificance_min, fEvents_min );
    TLatex *iL = new TLatex( 0.17, 0.15, hname );
    iL->SetTextSize( 0.03 );
    iL->SetNDC();
    iL->Draw();
}

/*
 
    prepare three canvases for debugging plots
*/   
void VSensitivityCalculator::prepareDebugPlots()
{
    vector< string > iCanvasTitle;
    iCanvasTitle.push_back( "debug plot: number of on/off (total) " );
    iCanvasTitle.push_back( "debug plot: effective areas" );
    iCanvasTitle.push_back( "debug plot: number of on/off (per particle type) " );
    char hname[800];
    for( unsigned int i = 0; i < iCanvasTitle.size(); i++ )
    {
       sprintf( hname, "%s_%d", fPlotDebugName.c_str(), i ); 
       cPlotDebug.push_back( new TCanvas( hname, iCanvasTitle[i].c_str(), 20+i*350, 650, 300, 300 ) );

       cPlotDebug.back()->SetGridx( 0 );
       cPlotDebug.back()->SetGridy( 0 );
       cPlotDebug.back()->SetLogy( 1 );
       cPlotDebug.back()->Draw();
    }
}


void VSensitivityCalculator::plotEffectiveArea()
{
    if( cPlotDebug.size() != 3 ) return;

    if( !cPlotDebug[1] || !cPlotDebug[1]->cd() ) return;

    vector< TGraphErrors* > g;
    TLegend *iL = new TLegend( 0.6, 0.6, 0.85, 0.85 );

    int z = 0;
    map< unsigned int, VSensitivityCalculatorDataResponseFunctions* >::iterator i_MCData_iterator;
    for( i_MCData_iterator = fMC_Data.begin(); i_MCData_iterator != fMC_Data.end(); i_MCData_iterator++ )
    {
       g.push_back( new TGraphErrors( 1 ) );
       g.back()->SetMinimum( 0.1 );
       g.back()->SetMaximum( 5.e6 );
       setGraphPlottingStyle( g.back(), z+1, 1, 20+z, 2 );

       for( unsigned int i = 0; i < (*i_MCData_iterator).second->energy.size(); i++ )
       {
	   g.back()->SetPoint( i, (*i_MCData_iterator).second->energy[i], (*i_MCData_iterator).second->effArea[i] );
	   g.back()->SetPointError( i, 0., (*i_MCData_iterator).second->effArea_error[i] );
       }
       if( g.back() )
       {
	  if( z == 0 )
	  {
	     g.back()->Draw( "ap" );
	     g.back()->GetHistogram()->SetXTitle( "log_{10} energy_{Rec} [TeV]" );
	     g.back()->GetHistogram()->SetYTitle( "effective area [m^{2}]" );
	  }
	  else
	  {
	     g.back()->Draw( "p" );
	  }
	  iL->AddEntry( g.back(), (*i_MCData_iterator).second->fName.c_str(), "pl" );
       }
       z++;
    }
    if( iL ) iL->Draw();

    cPlotDebug[1]->Update();
}

/*!

    plot particle numbers

*/
void VSensitivityCalculator::plotDebugPlotsBackgroundParticleNumbers( vector< VDifferentialFlux > iDifferentialFlux,
                                                                      map< unsigned int, vector< double > > i_flux_NOff,
								      map< unsigned int, vector< double > > i_flux_NOffError )
{
   if( cPlotDebug.size() != 3 ) return;

   if( !cPlotDebug[2] || !cPlotDebug[2]->cd() ) return;

   TGraph *gNon = new TGraph( 1 );
   gNon->SetMinimum( 0.0001 );
   gNon->SetMaximum( 1.e6 );
   setGraphPlottingStyle( gNon, 1, 1, 20, 2 );
    TGraph *gNoff = new TGraph( 1 );
    setGraphPlottingStyle( gNoff, 1, 1, 24, 2 );
   unsigned int z = 0;
   for( unsigned int i = 0; i < iDifferentialFlux.size(); i++ )
   {
       if( iDifferentialFlux[i].Energy > 0. )
       {
           gNon->SetPoint( z, log10( iDifferentialFlux[i].Energy ), iDifferentialFlux[i].NOn * fObservationTime * 60. );
           gNoff->SetPoint( z, log10( iDifferentialFlux[i].Energy ), iDifferentialFlux[i].NOff * fObservationTime * 60. );
           z++;
       }
   }
   if( fDebug )
   {
      cout << "particle numbers: signal (on) region): " << endl;
      gNon->Print();
      cout << "particle numbers: signal (off) region): " << endl;
      gNoff->Print();
   }
   gNon->Draw( "ap" );
   gNon->GetHistogram()->SetXTitle( "log_{10} energy [TeV]" );
   gNon->GetHistogram()->SetYTitle( "number of non/noff" );
   gNoff->Draw( "p" );

   vector< TGraphErrors* > g;

   map< unsigned int, vector< double > >::iterator i_flux_NOff_iter;
   map< unsigned int, vector< double > >::iterator i_flux_NOffError_iter;

   z = 0;
   for( i_flux_NOff_iter = i_flux_NOff.begin(); i_flux_NOff_iter != i_flux_NOff.end(); i_flux_NOff_iter++ )
   {
       g.push_back( new TGraphErrors( 1 ) );
       g.back()->SetMinimum( 1.e-4 );
       g.back()->SetMaximum( 1.e6 );
       setGraphPlottingStyle( g.back(), z+2, 1, 21+z, 2 );
       z++;
   }

   for( unsigned int i = 0; i < iDifferentialFlux.size(); i++ )
   {
       z = 0;
       for( i_flux_NOff_iter = i_flux_NOff.begin(); i_flux_NOff_iter != i_flux_NOff.end(); i_flux_NOff_iter++ )
       {
          if( i < (*i_flux_NOff_iter).second.size() )
	  {
	     g[z]->SetPoint( i, log10( iDifferentialFlux[i].Energy ), (*i_flux_NOff_iter).second[i] * fObservationTime * 60. );
          }
	  z++;
       }
       z = 0;
       for( i_flux_NOffError_iter = i_flux_NOffError.begin(); i_flux_NOffError_iter != i_flux_NOffError.end(); i_flux_NOffError_iter++ )
       {
          if( i < (*i_flux_NOffError_iter).second.size() )
	  {
	     g[z]->SetPointError( i, 0., (*i_flux_NOffError_iter).second[i] );
          }
	  z++;
       }
   }

   for( unsigned int i = 0; i < g.size(); i++ )
   {
      g[i]->Draw( "p" );
   }

   cPlotDebug[2]->Update();
}


void VSensitivityCalculator::plotDebugPlotsParticleNumbers( vector< VDifferentialFlux > iDifferentialFlux, double alpha )
{
    if( cPlotDebug.size() != 3 ) return;

    if( !cPlotDebug[0] || !cPlotDebug[0]->cd() ) return;

    TGraph *gNon = new TGraph( 1 );
    gNon->SetMinimum( 0.0001 );
    gNon->SetMaximum( 1.e6 );
    setGraphPlottingStyle( gNon, 1, 1, 20, 2 );
    TGraph *gNoff = new TGraph( 1 );
    setGraphPlottingStyle( gNoff, 2, 1, 21, 2 );

    int z = 0;
    for( unsigned int i = 0; i < iDifferentialFlux.size(); i++ )
    {
        if( iDifferentialFlux[i].Energy > 0. )
        {
            gNon->SetPoint( z, log10( iDifferentialFlux[i].Energy ), iDifferentialFlux[i].NOn );
            gNoff->SetPoint( z, log10( iDifferentialFlux[i].Energy ), iDifferentialFlux[i].NOff * alpha );
            z++;
        }
    }
    gNon->Draw( "ap" );
    gNon->GetHistogram()->SetXTitle( "log_{10} energy [TeV]" );
    gNon->GetHistogram()->SetYTitle( "number of non/noff" );
    gNoff->Draw( "p" );

    cPlotDebug[0]->Update();

// write graphs with on and off events to disk
    if( fDebugParticleNumberFile.size() > 0 )
    {
        cout << "writing graphs with on/off events to file: " << fDebugParticleNumberFile << endl;

	TFile iF( fDebugParticleNumberFile.c_str(), "RECREATE" );
	if( iF.IsZombie() )
	{
	    cout << "VSensitivityCalculator::plotDebugPlotsParticleNumbers: error opening particle number file (write): " << endl;
	    cout << fDebugParticleNumberFile << endl;
	    return;
        }
	gNon->SetName( "gNOn" );
	gNon->Write();
	gNoff->SetName( "gNOff" );
	gNoff->Write();

	iF.Close();
     }
}

/*!
    read values for Crab energy spectra from disk
*/
bool VSensitivityCalculator::setEnergySpectrumfromLiterature( string iFile, unsigned int iID )
{
    fEnergySpectrumfromLiterature = new VEnergySpectrumfromLiterature( iFile );
    if( fEnergySpectrumfromLiterature->isZombie() ) return false;

    setEnergySpectrumfromLiterature_ID( iID );

    return true;
}

void VSensitivityCalculator::plotSensitivityLimitations( TCanvas *c, double iYValue )
{
   if( !c ) 
   {
       cout << "VSensitivityCalculator::plotSensitivityLimitations: error, canvas not found" << endl;
       return;
   }
   c->cd();

// get maximum in histogram
   if( iYValue < -100. )
   {
      if( hnull ) iYValue = 0.5 * hnull->GetMaximum();
      cout << "iYValue " << iYValue << endl;
   }

   cout << "plotSensitivityLimitations: " << fMinEventsLimited.size() << "\t" << fSignificanceLimited.size() << "\t" << fMinBackgroundEventsLimited.size() << endl;

// minimum number of events
   if( fMinEventsLimited.size() > 0 )
   {
      TGraph *g = new TGraph( 1 );
      map< int, double >::const_iterator itx;
      int z = 0;
      for( itx = fMinEventsLimited.begin(); itx != fMinEventsLimited.end(); itx++ )
      {
	 double energy = (double((*itx).first))/1.e3;
	 if( energy > 0. )
	 {
	    energy = log10( energy );
	    g->SetPoint( z, energy, iYValue );
	    cout << "MinEvents (>" << fEvents_min << "): " << z << "\t" << energy << "\t" << (*itx).second << endl;
         }
	 z++;
      }
      g->SetLineColor( 2 );
      g->SetLineWidth( 3 );
      g->Draw( "L" );
   }
// background events limited
   if( fMinBackgroundEventsLimited.size() > 0 )
   {
      TGraph *g = new TGraph( 1 );
      map< int, double >::const_iterator itx;
      int z = 0;
      for( itx = fMinBackgroundEventsLimited.begin(); itx != fMinBackgroundEventsLimited.end(); itx++ )
      {
	 double energy = (double((*itx).first))/1.e3;
	 if( energy > 0. )
	 {
	    energy = log10( energy );
	    g->SetPoint( z, energy, iYValue * 1.1 );
	    cout << "Background events (>" << fBackgroundEvents_min << "): " << z << "\t" << energy << "\t" << (*itx).second << endl;
         }
	 z++;
      }
      g->SetLineColor( 3 );
      g->SetLineWidth( 3 );
      g->Draw( "L" );
   }
// significance
   if( fSignificanceLimited.size() > 0 )
   {
      TGraph *g = new TGraph( 1 );
      map< int, double >::const_iterator itx;
      int z = 0;
      for( itx = fSignificanceLimited.begin(); itx != fSignificanceLimited.end(); itx++ )
      {
	 double energy = (double((*itx).first))/1.e3;
	 if( energy > 0. )
	 {
	    energy = log10( energy );
	    g->SetPoint( z, energy, iYValue * 0.9 );
	    cout << "Significance: (>" << fSignificance_min << " ): " << z << "\t" << energy << "\t" << (*itx).second << endl;
         }
	 z++;
      }
      g->SetLineColor( 4 );
      g->SetLineWidth( 3 );
// (don't draw this one)
//      g->Draw( "L" );
   }

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// data class for response functions for a give primary
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VSensitivityCalculatorDataResponseFunctions::VSensitivityCalculatorDataResponseFunctions()
{
    fName = "";
    fParticleID = 0;
    fSpectralParameterFile = "";
    fSpectralParameterID = 0;
    fEffectiveAreaFile = "";
    ze = 0.;
    az = 0;
    woff = 0.;
    noise = 0;
    index = 0.;
    theta2_min = 0.;
//    theta2_max = -1.;
    theta2_MCScatterAngle = 0.;
    gSolidAngle_DirectionCut_vs_EnergylgTeV = 0;
    SolidAngle_MCScatterAngle = 0.;
    alpha = 0.;
    effArea_Ebins = 0;
    effArea_Emin = 0.;
    effArea_Emax = 0.;
    energy_min_log = 0.;
    energy_max_log = 0.;
}

double VSensitivityCalculatorDataResponseFunctions::getSolidAngle_DirectionCut( double e )
{
    if( gSolidAngle_DirectionCut_vs_EnergylgTeV ) 
    {
       if( e > 0. ) return gSolidAngle_DirectionCut_vs_EnergylgTeV->Eval( log10( e ) );
    }

    return -1.;
}
