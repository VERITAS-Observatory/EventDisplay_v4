/*! \class VLightCurve
    \brief light curves

    Revision $Id: VLightCurve.cpp,v 1.1.2.2 2011/06/16 14:53:04 gmaier Exp $

*/

#include "VLightCurve.h"

VLightCurve::VLightCurve()
{
   fDayInterval = 1.;
   fFluxCombined = 0;

   fDataType = "";                       // possible data types are TeV_anasum, TeV_ascii, XRT_ascii

   fEnergy_min_TeV = 1.;
   fEnergy_max_TeV = -99.;

   fLightCurveGraph = 0;
   fCanvasLightCurve = 0;
   fMCRandomizedPhaseogram = 0;
   fMCRandomizedPhaseogramProf = 0;

   setSignificanceParameters();
   setSpectralParameters();

   setPlottingParameter( -99., -99. );
   setLightCurveAxis();
   setPhaseFoldingValues( -99., -99., false );
}

/*

    read XRT light curve from a ASCII file

*/
bool VLightCurve::initializeXRTLightCurve( string iXRTFile, double iMJDMin, double iMJDMax )
{
   fDataType = "XRT_ascii";

// read in ascii file
   readASCIIFile( iXRTFile, iMJDMin, iMJDMax );
   if( isZombie() ) return false;

// plotting range
   if( fPlottingMJDMin < 0. ) fPlottingMJDMin = getLightCurveMJD_min() - 5.;
   if( fPlottingMJDMax < 0. ) fPlottingMJDMax = getLightCurveMJD_max() + 5.;

   return true;
}

/*

    read TeV fluxes from ASCII file

*/
bool VLightCurve::initializeTeVLightCurve( string iASCIIFile )
{
// make sure that this is really a ascii file
   if( iASCIIFile.find( ".root" ) != string::npos )
   {
       return initializeTeVLightCurve( iASCIIFile, 1. );
   }

   fDataType = "TeV_ascii";

// read in ascii file
   readASCIIFile( iASCIIFile );
   if( isZombie() ) return false;

// plotting range
   if( fPlottingMJDMin < 0. ) fPlottingMJDMin = getLightCurveMJD_min() - 5.;
   if( fPlottingMJDMax < 0. ) fPlottingMJDMax = getLightCurveMJD_max() + 5.;

   return true;
}

/*

    read fluxes etc from anasum file

*/
bool VLightCurve::initializeTeVLightCurve( string iAnaSumFile, double iDayInterval, double iMJDMin, double iMJDMax )
{
   fDataType = "TeV_anasum";

   fDayInterval = iDayInterval;
   fAnaSumFile = iAnaSumFile;

// read full file to get an overview of all available runs
   fFluxCombined = new VFluxCalculation( fAnaSumFile, 1, 0, 100000, -99., -99., false );
   if( fFluxCombined->IsZombie() )
   {
      cout << "VLightCurve::initializeLightCurve error reading anasum file" << endl;
      return false;
   }
   fFluxCombined->setTimeBinnedAnalysis(false );

/////////////////////////////////
// setup time bins

// get timing vectors (expect them to be sorted from earliest to latest)
   vector< double > fMJD    = fFluxCombined->getMJD();
   vector< double > fRunTOn = fFluxCombined->getTOn();
   if( fMJD.size() < 2 )
   {
      cout << "VLightCurve::initializeLightCurve error, time vector too small: " << fMJD.size() << endl;
      return false;
   }

// get MJD min
   if( iMJDMin < 0 ) iMJDMin = TMath::Floor( fMJD[0] );
   if( iMJDMax < 0 ) iMJDMax = TMath::Floor( fMJD[fMJD.size()-2] ) + 1;

// plotting parameters
   if( fPlottingMJDMin < 0. ) fPlottingMJDMin = iMJDMin - 5.;
   if( fPlottingMJDMax < 0. ) fPlottingMJDMax = iMJDMax + 5.;

   unsigned int iNbins = (unsigned int)( (iMJDMax-iMJDMin)/iDayInterval + 0.5);
   if( iNbins == 0 && iMJDMax > iMJDMin ) iNbins = 1;
   iMJDMax = iMJDMin + iNbins * iDayInterval;

   cout << "Defining " << iNbins << " time " << (iNbins==1 ? "interval" : "intervals") << " from " << iMJDMin << " to " << iMJDMax << endl;

// resetting light curve data vector
   fLightCurveData.clear();

// fill all light curve bins (might be a run, or XX days long)
   for( unsigned int i = 0; i < iNbins; i++ )
   {
       unsigned int z = 0;
// loop over all runs (using the MJD vector)
       for( unsigned int j = 0; j < fMJD.size() - 1; j++ )
       {
// check if there are data in this MJD interval
           if( fMJD[j] >= iMJDMin + i * iDayInterval && fMJD[j] < iMJDMin + (i+1) * iDayInterval )
	   {
// get observing epoch
	       if( z == 0 )
	       {
	          fLightCurveData.push_back( new VLightCurveData() ); 
		  fLightCurveData.back()->setMJDInterval( iMJDMin + i * iDayInterval, iMJDMin + (i+1) * iDayInterval );
               }
	       if( fLightCurveData.back() )
	       {
	          fLightCurveData.back()->fRunList.push_back( fFluxCombined->getRunList()[j] );
               }
	       z++;
	   }
       }
   }

   return true;
}

bool VLightCurve::fill( double iEMin_TeV, double iEMax_TeV, bool iPrint )
{
   fEnergy_min_TeV = iEMin_TeV;
   fEnergy_max_TeV = iEMax_TeV;

   cout << "filling data for " << fDataType << endl;

   if( fDataType == "TeV_anasum" )     return fillTeV_anasum( iPrint );
   else if( fDataType == "TeV_ascii" ) return fillTeV_ascii(  iPrint );
   else if( fDataType == "XRT_ascii" ) return fillXRT_ascii(  iPrint );

   return false;
}

bool VLightCurve::fillTeV_ascii( bool iPrint )
{
    if( iPrint ) printLightCurve();
    return true;
}

bool VLightCurve::fillXRT_ascii( bool iPrint )
{
    if( iPrint ) printLightCurve();
    
    return true;
}

bool VLightCurve::fillTeV_anasum( bool iPrint )
{
// loop over all light curve data and calculate fluxes   
   for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
   {
       if( fLightCurveData[i] )
       {
          fLightCurveData[i]->setFluxCalculationEnergyInterval( fEnergy_min_TeV, fEnergy_max_TeV );
          fLightCurveData[i]->fillTeVEvndispData( fAnaSumFile, fThresholdSignificance, fMinEvents, fUpperLimit, fUpperLimitMethod, fLiMaEqu, fMinEnergy, fE0, fAlpha );
//	  fLightCurveData[i]->fillTeVEvndispData( fAnaSumFile, fThresholdSignificance, fMinEvents, fUpperLimit, fUpperLimitMethod, fLiMaEqu );
       }
   }

   if( iPrint ) printLightCurve();

   return true;
}

/*

      plot a light curve for the given data

      TCanvas* iCanvasLightCurve      	0 for a new canvas, or pointer to an existing canvas
      string iCanvasName              	canvas name (for new canvas)
      int iPlotConfidenceInterval 	plot rates as usual (0; Non-Noff with Poisson error)
      					or
					1 sigma confidence intervalls using Rolke et al (iPlotConfidenceInterval >= 0; number is plotting color)

*/
TCanvas* VLightCurve::plotLightCurve( TCanvas* iCanvasLightCurve, string iCanvasName, int iPlotConfidenceInterval, string iPlottingOption )
{
    char hname[800];
    char htitle[800];

    TH1D *hLightCurve = 0;

// canvas
    if( !iCanvasLightCurve )
    {
       sprintf( hname, "%s", iCanvasName.c_str() );
       if( fName.size() > 0 ) sprintf( htitle, "%s", fName.c_str() );
       else                   sprintf( htitle, "light curve" );
       if( fPhase_Period_days > 0. )
       { 
          sprintf( hname, "%s_%d_%d_%d", iCanvasName.c_str(), (int)fPhase_MJD0, (int)fPhase_Period_days, (int)fPhasePlotting );
	  sprintf( htitle, "%s, T_{0}=%.1f, period=%.1f days", fName.c_str(), fPhase_MJD0, fPhase_Period_days );
       }

       fCanvasLightCurve = new TCanvas( hname, htitle, 10, 10, 600, 400 );
       fCanvasLightCurve->SetGridx( 0 );
       fCanvasLightCurve->SetGridy( 0 );

// histogram values
       double i_xmin = fPlottingMJDMin;
       double i_xmax = fPlottingMJDMax;
       sprintf( hname, "MJD" );
       sprintf( htitle, "hLightCurve" );
       if( fPhase_Period_days > 0. )
       {
	  if( fPhasePlotting )
	  {
	     i_xmin = 0.;
	     i_xmax = 1.;
	     sprintf( hname, "phase" );
          }
	  else
	  {
	     i_xmin = 0.;
	     i_xmax = fPhase_Period_days;
	     sprintf( hname, "phase folded days" );
	  }
          sprintf( htitle, "%s_%d_%d_%d", htitle, (int)fPhase_MJD0, (int)fPhase_Period_days, (int)fPhasePlotting );
       }

       hLightCurve = new TH1D( htitle, "", 100, i_xmin, i_xmax );
       hLightCurve->SetStats( 0 );
       hLightCurve->SetXTitle( hname );
       hLightCurve->GetXaxis()->CenterTitle( true );	  
       hLightCurve->SetYTitle(  getLightCurveAxisTitle().c_str() );
       hLightCurve->SetMinimum( getLightCurveAxisRange_Min() );
       hLightCurve->SetMaximum( getLightCurveAxisRange_Max() );
       hLightCurve->Draw("");
       hLightCurve->Draw("AH");

       cout << "Plotting range: " << fPlottingMJDMin << " < MJD < " << fPlottingMJDMax;
       cout << ", " << hLightCurve->GetMinimum() << " < rate < " << hLightCurve->GetMaximum() << endl;

       plot_nullHistogram( fCanvasLightCurve, hLightCurve, false, true, 1.2, fPlottingMJDMin, fPlottingMJDMax );

    }
    else
    {
       fCanvasLightCurve = iCanvasLightCurve;
       fCanvasLightCurve->cd();
       sprintf( htitle, "hLightCurve" );
       if( fPhase_Period_days > 0. ) sprintf( htitle, "%s_%d_%d_%d", htitle, (int)fPhase_MJD0, (int)fPhase_Period_days, (int)fPhasePlotting );
       hLightCurve = (TH1D*)fCanvasLightCurve->GetListOfPrimitives()->FindObject( htitle );
       if( !hLightCurve )
       {
          cout << "VLightCurve::plot: no light curve histogram found with name " << htitle << endl;
	  fCanvasLightCurve->GetListOfPrimitives()->Print();
	  return fCanvasLightCurve;
       }
    }

////////////////////////////////////////////////////////////////////////
// flux and upper flux plotting

    fLightCurveGraph = new TGraphAsymmErrors( 0 );
    setGraphPlottingStyle( (TGraph*)fLightCurveGraph );

// loop over all measured values
    unsigned int z = 0;
    for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
    {
// x-axis: MJD or phase 
       double iMJD_mean =  fLightCurveData[i]->getMJD();
       double iMJD_error = fLightCurveData[i]->getMJDError();
// phase dependent analysis
       if( fPhase_Period_days > 0. )
       {
	  iMJD_mean = getPhase( iMJD_mean );
	  if( fPhasePlotting ) iMJD_error /= fPhase_Period_days;
       }

/////////////
// plot fluxes or confidence intervals
       if( fLightCurveData[i] && fLightCurveData[i]->fFluxError >= 0. )
       {
	  if( iPlotConfidenceInterval < 0 )
	  {
	     fLightCurveGraph->SetPoint( z, iMJD_mean, fLightCurveData[i]->fFlux );
	     fLightCurveGraph->SetPointError( z, iMJD_error, iMJD_error, fLightCurveData[i]->fFluxError, fLightCurveData[i]->fFluxError );
	  }
	  else
	  {
// plot coincidence intervals
	     double iFMean = 0.5*(fLightCurveData[i]->fRunFluxCI_lo_1sigma+fLightCurveData[i]->fRunFluxCI_up_1sigma);
	     fLightCurveGraph->SetPoint( z, iMJD_mean, iFMean );
// (minimum width -> make sure that line is visible...)
	     if( iMJD_error < 0.3 && !(fPhase_Period_days > 0. ) ) iMJD_error = 0.3;
	     fLightCurveGraph->SetPointError( z, iMJD_error, iMJD_error, iFMean-fLightCurveData[i]->fRunFluxCI_lo_1sigma, fLightCurveData[i]->fRunFluxCI_up_1sigma-iFMean );
	  }
	  z++;
       }
/////////////
// plot upper flux limits
       else if( fLightCurveData[i]->fUpperFluxLimit > 0. )
       {
           TArrow *fUL = new TArrow( iMJD_mean, fLightCurveData[i]->fUpperFluxLimit, iMJD_mean, fLightCurveData[i]->fUpperFluxLimit - 0.05*hLightCurve->GetMaximum(), 0.01, "|-|>" );
	   setArrowPlottingStyle( fUL );
	   fUL->Draw(); 
       }
/////////////
// plot observational periods
       else
       {
           cout << "no plotting for flux intervall " << i << "\t" << fLightCurveData[i]->fMJD_min << "\t" << fLightCurveData[i]->fMJD_max << ":" << endl;
	   cout << "\t" << fLightCurveData[i]->fFlux << " +- " << fLightCurveData[i]->fFluxError << " , UL" << fLightCurveData[i]->fUpperFluxLimit << endl;
       }
    }
    if( fLightCurveGraph->GetN() > 0 )
    {
       if( iPlotConfidenceInterval < 0 ) fLightCurveGraph->Draw( iPlottingOption.c_str() );
       else
       {
	  fLightCurveGraph->SetFillColor( iPlotConfidenceInterval );
	  fLightCurveGraph->Draw( "2" );
       }
    }

    return fCanvasLightCurve;
}

void VLightCurve::setPlottingParameter( double iPlottingMJDMin, double iPlottingMJDMax )
{
   fPlottingMJDMin = iPlottingMJDMin;
   fPlottingMJDMax = iPlottingMJDMax;
}


void VLightCurve::setSignificanceParameters( double iSig, double iMinEvents, double iUL, int iULAlgo, int iLiAndMa )
{
    fThresholdSignificance = iSig;
    fMinEvents = iMinEvents;
    fUpperLimit = iUL;
    fLiMaEqu = iLiAndMa;
    fUpperLimitMethod = iULAlgo;
}


void VLightCurve::setSpectralParameters( double iMinEnergy, double  E0, double alpha )
{
    fMinEnergy = iMinEnergy;
    fE0 = E0;
    fAlpha = alpha;
}


bool VLightCurve::plotObservingPeriods( TCanvas* iCanvasLightCurve, string iDataFile, int iColor )
{
   if( !iCanvasLightCurve || !iCanvasLightCurve->cd() ) return false;

   iDataFile = "";
   iColor = 1;

// read text file with observing periods

   /*

       to ob added

   */

   return true;
}

void VLightCurve::setLightCurveAxis( double iYmin, double iYmax, string iAxisTitle )
{
   fRateAxisMin = iYmin;
   fRateAxisMax = iYmax;
   fRateAxisTitle = iAxisTitle;
}

   
double VLightCurve::getLightCurveAxisRange_Min()
{
   double iFluxMin = 1.e99;

   if( fRateAxisMin >= -1.e5 ) iFluxMin = fRateAxisMin;
   else
   {
       for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
       {
	  if( fLightCurveData[i] && fLightCurveData[i]->fFluxError > 0. && fLightCurveData[i]->fFlux - fLightCurveData[i]->fFluxError < iFluxMin )
	  {
	     iFluxMin = fLightCurveData[i]->fFlux - fLightCurveData[i]->fFluxError;
	  }
	  else if( fLightCurveData[i] && fLightCurveData[i]->fUpperFluxLimit > 0. && fLightCurveData[i]->fUpperFluxLimit < iFluxMin )
	  {
	     iFluxMin = fLightCurveData[i]->fUpperFluxLimit;
	  }
       }
    }

    return iFluxMin;
}

double VLightCurve::getLightCurveAxisRange_Max()
{
   double iFluxMax = -1.;

   if( fRateAxisMax >= 0. ) iFluxMax = fRateAxisMax;
   else
   {
       for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
       {
	  if( fLightCurveData[i] && fLightCurveData[i]->fFluxError > 0. && fLightCurveData[i]->fFlux + fLightCurveData[i]->fFluxError > iFluxMax )
	  {
	     iFluxMax = fLightCurveData[i]->fFlux + fLightCurveData[i]->fFluxError;
	  }
	  else if( fLightCurveData[i] && fLightCurveData[i]->fUpperFluxLimit > 0. && fLightCurveData[i]->fUpperFluxLimit > iFluxMax )
	  {
	     iFluxMax = fLightCurveData[i]->fUpperFluxLimit;
	  }
       }
   }

   return iFluxMax;
}

string VLightCurve::getLightCurveAxisTitle()
{
    char hname[1000];

    if( fRateAxisTitle == "tevRate" )
    {
       if( fEnergy_max_TeV < 0. ) sprintf( hname, "Flux (E>%.1f TeV) [cm^{-2}s^{-1}]", fEnergy_min_TeV );
       else                       sprintf( hname, "Flux ( %.1f - %.1f TeV) [cm^{-2}s^{-1}]", fEnergy_min_TeV, fEnergy_max_TeV );

       string iTemp = hname;
       fRateAxisTitle = hname;
       return iTemp;
    }


    return fRateAxisTitle;
}

/*

   show impact of error on period on phaseograms by using randomizing the period and filling a 2D histogram

*/
bool VLightCurve::fillRandomizedPhaseogram( double iMCCycles, double iPhaseError_low, double iPhaseErrorUp, string iHisName, double iHisMin_y, double iHisMax_y )
{
// 2D histogram
   if( fMCRandomizedPhaseogram == 0 )
   {
      fMCRandomizedPhaseogram = new TH2D( iHisName.c_str(), "", 100, 0., 1., 100., iHisMin_y, iHisMax_y );
      fMCRandomizedPhaseogram->SetStats( 0 );
   }
   else
   {
      fMCRandomizedPhaseogram->Reset();
   }
// profile histogram
   if( fMCRandomizedPhaseogramProf == 0 )
   {
      iHisName += "prof";
      fMCRandomizedPhaseogramProf = new TProfile( iHisName.c_str(), "", 100, 0., 1., iHisMin_y, iHisMax_y );
      fMCRandomizedPhaseogramProf->SetStats( 0 );
   }
   else
   {
      fMCRandomizedPhaseogramProf->Reset();
   }

// keep old period
   double iPhase_Period_days_backup = fPhase_Period_days;

   double x = 0.;
   double y = 0.;
   double iPhaseOrbit = 0.;
   double iPhaseOrbitError = 0.;

   for( unsigned int i = 0; i < iMCCycles; i++ )
   {
	 if( gRandom->Uniform( 1. ) < 0.5 )
	 {
	    iPhaseOrbitError = -1.*TMath::Abs( gRandom->Gaus( 0., iPhaseError_low ) );
	 }
	 else
	 {
	    iPhaseOrbitError = TMath::Abs( gRandom->Gaus( 0., iPhaseErrorUp ) );
	 }
	 iPhaseOrbit = iPhase_Period_days_backup + iPhaseOrbitError;

	 setPhaseFoldingValues( fPhase_MJD0, iPhaseOrbit, fPhasePlotting );

	 for( unsigned int j = 0; j < fLightCurveData.size(); j++ )
	 {
// second MC cycle: take errors in flux into account
	     x = getPhase( fLightCurveData[j]->getMJD() );
             for( unsigned int k = 0; k < iMCCycles; k++ )
	     {
		y = gRandom->Gaus( fLightCurveData[j]->fFlux, fLightCurveData[j]->fFluxError );

		fMCRandomizedPhaseogram->Fill( x, y );
		fMCRandomizedPhaseogramProf->Fill( x, y );
             }
	 } 
   }

// restore old phase values
   setPhaseFoldingValues( fPhase_MJD0, iPhase_Period_days_backup, fPhasePlotting );

   return true;
}
