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

   setSignificanceParameters();
   setSpectralParameters();

   setPlottingParameter( -99., -99. );
   setLightCurveAxis();
   setPhaseFoldingValues( -99., -99., false );
}

bool VLightCurve::initializeXRTLightCurve( string iXRTFile, double iMJDMin, double iMJDMax, double iMJDStart )
{
   fDataType = "XRT_ascii";

// resetting light curve data vector
   fFluxInterval.clear();

// read in ascii file
   ifstream is( iXRTFile.c_str() );
   if( !is )
   {
      cout << "VLightCurve::initializeXRTLightCurve error reading XRT file: " << iXRTFile << endl;
      return false;
   }
   double iTemp1 = 0.;
   double iTemp2 = 0.;

   double iMJDDataMin = 1.e99;
   double iMJDDataMax = -1.e99;

   string is_line;

   while(  getline( is, is_line ) )
   {
       if( is_line.size() == 0 ) continue;

       istringstream is_stream( is_line );

//! no errors are catched here..
       is_stream >> iTemp1;     // second since iMJDStart
       is_stream >> iTemp2;     // error [s]

       iTemp1  = iMJDStart + iTemp1/(24.0*60.0*60.0);
       iTemp2 /= (24.0*60.0*60.0);

       if( iMJDMin > 0. && iTemp1 - iTemp2 < iMJDMin ) continue;
       if( iMJDMax > 0. && iTemp1 + iTemp2 > iMJDMax ) continue;

       fFluxInterval.push_back( new VLightCurveData() ); 
       fFluxInterval.back()->fMJD_Data_min = iTemp1 - iTemp2;
       fFluxInterval.back()->fMJD_Data_max = iTemp1 + iTemp2;

       if( fFluxInterval.back()->fMJD_Data_min < iMJDDataMin ) iMJDDataMin = fFluxInterval.back()->fMJD_Data_min;
       if( fFluxInterval.back()->fMJD_Data_max > iMJDDataMax ) iMJDDataMax = fFluxInterval.back()->fMJD_Data_max;

       is_stream >> iTemp1;     // rate
       is_stream >> iTemp2;     // rate error

       fFluxInterval.back()->fFlux = iTemp1;
       fFluxInterval.back()->fFluxError = iTemp2;
   }

// plotting range
   if( fPlottingMJDMin < 0. ) fPlottingMJDMin = iMJDDataMin - 5.;
   if( fPlottingMJDMax < 0. ) fPlottingMJDMax = iMJDDataMax + 5.;

   is.close();

   cout << "VLightCurve::initializeXRTLightCurve total number of light curve data: " << fFluxInterval.size() << endl;

   return true;
}

bool VLightCurve::initializeTeVLightCurve( string iASCIIFile )
{
// make sure that this is really a ascii file
   if( iASCIIFile.find( ".root" ) != string::npos )
   {
       return initializeTeVLightCurve( iASCIIFile, 1. );
   }

   fDataType = "TeV_ascii";

// resetting light curve data vector
   fFluxInterval.clear();

// read in ascii file
   ifstream is( iASCIIFile.c_str() );
   if( !is )
   {
      cout << "VLightCurve::initializeTeVLightCurve error reading TeV ascii file: " << iASCIIFile << endl;
      return false;
   }
   double iTemp1 = 0.;
   double iTemp2 = 0.;

   double iMJDMin = 1.e99;
   double iMJDMax = -1.e99;

   string is_line;

   while(  getline( is, is_line ) )
   {
       if( is_line.size() == 0 ) continue;

       istringstream is_stream( is_line );

       fFluxInterval.push_back( new VLightCurveData() ); 

       is_stream >> iTemp1;     // MJD min
       is_stream >> iTemp2;     // MJD max

       fFluxInterval.back()->fMJD_Data_min = iTemp1;
       fFluxInterval.back()->fMJD_Data_max = iTemp2;
       fFluxInterval.back()->setMJDInterval( fFluxInterval.back()->fMJD_Data_min, fFluxInterval.back()->fMJD_Data_max );

       if( fFluxInterval.back()->fMJD_Data_min < iMJDMin ) iMJDMin = fFluxInterval.back()->fMJD_Data_min;
       if( fFluxInterval.back()->fMJD_Data_max > iMJDMax ) iMJDMax = fFluxInterval.back()->fMJD_Data_max;

       is_stream >> iTemp1;     // flux
       is_stream >> iTemp2;     // flux error

       if( iTemp2 > 0. )
       {
          fFluxInterval.back()->fFlux = iTemp1;
          fFluxInterval.back()->fFluxError = iTemp2;
       }
       else if( iTemp1 > 0. && iTemp2 < 0. )
       {
          fFluxInterval.back()->fUpperFluxLimit = iTemp1;
       }

       if( !is_stream.eof() ) 
       {
          is_stream >> fFluxInterval.back()->fName;
       }
   }

// plotting range
   if( fPlottingMJDMin < 0. ) fPlottingMJDMin = iMJDMin - 5.;
   if( fPlottingMJDMax < 0. ) fPlottingMJDMax = iMJDMax + 5.;

   is.close();

   return true;
}

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
   fFluxInterval.clear();

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
	          fFluxInterval.push_back( new VLightCurveData() ); 
		  fFluxInterval.back()->setMJDInterval( iMJDMin + i * iDayInterval, iMJDMin + (i+1) * iDayInterval );
               }
	       if( fFluxInterval.back() )
	       {
	          fFluxInterval.back()->fRunList.push_back( fFluxCombined->getRunList()[j] );
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
   for( unsigned int i = 0; i < fFluxInterval.size(); i++ )
   {
       if( fFluxInterval[i] )
       {
          fFluxInterval[i]->setFluxCalculationEnergyInterval( fEnergy_min_TeV, fEnergy_max_TeV );
          fFluxInterval[i]->fillTeVEvndispData( fAnaSumFile, fThresholdSignificance, fMinEvents, fUpperLimit, fUpperLimitMethod, fLiMaEqu, fMinEnergy, fE0, fAlpha );
//	  fFluxInterval[i]->fillTeVEvndispData( fAnaSumFile, fThresholdSignificance, fMinEvents, fUpperLimit, fUpperLimitMethod, fLiMaEqu );
       }
   }

   if( iPrint ) printLightCurve();

   return true;
}

void VLightCurve::setPhaseFoldingValues( double iZeroPhase_MJD, double iPhase_Days, bool bPlotPhase )
{
   fPhase_MJD0 = iZeroPhase_MJD;
   fPhase_Period_days = iPhase_Days;
   fPhasePlotting = bPlotPhase;
}

double VLightCurve::getPhase( double iMJD )
{
   iMJD = ( iMJD - fPhase_MJD0 ) / fPhase_Period_days;
   iMJD =   iMJD - TMath::Floor( iMJD );
   if( !fPhasePlotting ) iMJD  *= fPhase_Period_days;

   return iMJD;
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
    for( unsigned int i = 0; i < fFluxInterval.size(); i++ )
    {
// x-axis: MJD or phase 
       double iMJD_mean =  fFluxInterval[i]->getMJD();
       double iMJD_error = fFluxInterval[i]->getMJDError();
// phase dependent analysis
       if( fPhase_Period_days > 0. )
       {
	  iMJD_mean = getPhase( iMJD_mean );
	  if( fPhasePlotting ) iMJD_error /= fPhase_Period_days;
       }

/////////////
// plot fluxes or confidence intervals
       if( fFluxInterval[i] && fFluxInterval[i]->fFluxError >= 0. )
       {
	  if( iPlotConfidenceInterval < 0 )
	  {
	     fLightCurveGraph->SetPoint( z, iMJD_mean, fFluxInterval[i]->fFlux );
	     fLightCurveGraph->SetPointError( z, iMJD_error, iMJD_error, fFluxInterval[i]->fFluxError, fFluxInterval[i]->fFluxError );
	  }
	  else
	  {
// plot coincidence intervals
	     double iFMean = 0.5*(fFluxInterval[i]->fRunFluxCI_lo_1sigma+fFluxInterval[i]->fRunFluxCI_up_1sigma);
	     fLightCurveGraph->SetPoint( z, iMJD_mean, iFMean );
// (minimum width -> make sure that line is visible...)
	     if( iMJD_error < 0.3 && !(fPhase_Period_days > 0. ) ) iMJD_error = 0.3;
	     fLightCurveGraph->SetPointError( z, iMJD_error, iMJD_error, iFMean-fFluxInterval[i]->fRunFluxCI_lo_1sigma, fFluxInterval[i]->fRunFluxCI_up_1sigma-iFMean );
	  }
	  z++;
       }
/////////////
// plot upper flux limits
       else if( fFluxInterval[i]->fUpperFluxLimit > 0. )
       {
           TArrow *fUL = new TArrow( iMJD_mean, fFluxInterval[i]->fUpperFluxLimit, iMJD_mean, fFluxInterval[i]->fUpperFluxLimit - 0.05*hLightCurve->GetMaximum(), 0.01, "|-|>" );
	   setArrowPlottingStyle( fUL );
	   fUL->Draw(); 
       }
/////////////
// plot observational periods
       else
       {
           cout << "no plotting for flux intervall " << i << "\t" << fFluxInterval[i]->fMJD_min << "\t" << fFluxInterval[i]->fMJD_max << ":" << endl;
	   cout << "\t" << fFluxInterval[i]->fFlux << " +- " << fFluxInterval[i]->fFluxError << " , UL" << fFluxInterval[i]->fUpperFluxLimit << endl;
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
       for( unsigned int i = 0; i < fFluxInterval.size(); i++ )
       {
	  if( fFluxInterval[i] && fFluxInterval[i]->fFluxError > 0. && fFluxInterval[i]->fFlux - fFluxInterval[i]->fFluxError < iFluxMin )
	  {
	     iFluxMin = fFluxInterval[i]->fFlux - fFluxInterval[i]->fFluxError;
	  }
	  else if( fFluxInterval[i] && fFluxInterval[i]->fUpperFluxLimit > 0. && fFluxInterval[i]->fUpperFluxLimit < iFluxMin )
	  {
	     iFluxMin = fFluxInterval[i]->fUpperFluxLimit;
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
       for( unsigned int i = 0; i < fFluxInterval.size(); i++ )
       {
	  if( fFluxInterval[i] && fFluxInterval[i]->fFluxError > 0. && fFluxInterval[i]->fFlux + fFluxInterval[i]->fFluxError > iFluxMax )
	  {
	     iFluxMax = fFluxInterval[i]->fFlux + fFluxInterval[i]->fFluxError;
	  }
	  else if( fFluxInterval[i] && fFluxInterval[i]->fUpperFluxLimit > 0. && fFluxInterval[i]->fUpperFluxLimit > iFluxMax )
	  {
	     iFluxMax = fFluxInterval[i]->fUpperFluxLimit;
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
       if( fEnergy_max_TeV < 0. ) sprintf( hname, "F(E>%.1f TeV) [cm^{-2}s^{-1}]", fEnergy_min_TeV );
       else                       sprintf( hname, "F( %.1f - %.1f TeV) [cm^{-2}s^{-1}]", fEnergy_min_TeV, fEnergy_max_TeV );

       string iTemp = hname;
       fRateAxisTitle = hname;
       return iTemp;
    }


    return fRateAxisTitle;
}

void VLightCurve::printLightCurve( bool bFullDetail )
{
// print light curve with many details
   if( bFullDetail )
   {
      for( unsigned int i = 0; i < fFluxInterval.size(); i++ )
      {
	 cout << i << "\tMJD " << fFluxInterval[i]->fMJD_min << " - " << fFluxInterval[i]->fMJD_max;
	 cout << " (" << fFluxInterval[i]->getMJD() << " +- " << fFluxInterval[i]->getMJDError() << ")";
	 if( fPhase_Period_days > 0. )
	 {
	     double iMJD_mean = fFluxInterval[i]->getMJD();
	     cout << ", Phase " << getPhase( iMJD_mean );
	 }
	 if( fFluxInterval[i]->fRunList.size() > 0 ) cout << " # runs: " << fFluxInterval[i]->fRunList.size();
	 cout << endl;
	 if( fFluxInterval[i]->fNon >= 0. ) cout << "\tNon " << fFluxInterval[i]->fNon << "\tNoff " << fFluxInterval[i]->fNoff;
	 cout << "\t Significance: " << fFluxInterval[i]->fSignificance;
	 cout << "\t Tot Time [h]: " << fFluxInterval[i]->fRunTime/3600.;
	 cout << endl;
	 cout << "\tFlux " << fFluxInterval[i]->fFlux;
	 cout << " +- " << fFluxInterval[i]->fFluxError << "\tUL " << fFluxInterval[i]->fUpperFluxLimit;
	 if( fFluxInterval[i]->fRunFluxCI_lo_1sigma >= 0. )
	 {
	    cout << "\t CI (1 sigma): " << fFluxInterval[i]->fRunFluxCI_lo_1sigma << "\t" << fFluxInterval[i]->fRunFluxCI_up_1sigma;
	    cout << "\t CI (3 sigma): " << fFluxInterval[i]->fRunFluxCI_lo_3sigma << "\t" << fFluxInterval[i]->fRunFluxCI_up_3sigma;
	 }
	 cout << endl;
	 if( fFluxInterval[i]->fName.size() > 0 ) cout << "\t (" << fFluxInterval[i]->fName << ")" << endl;
      }
   }
// print values useful for e.g. z-correlation analysis
   else
   {
      for( unsigned int i = 0; i < fFluxInterval.size(); i++ )
      {
         cout << "  "    << fFluxInterval[i]->getMJD();
	 cout << "     " << fFluxInterval[i]->fFlux;
	 cout << "     " << fFluxInterval[i]->fFluxError;
	 cout << endl;
      }
   }

}

vector< VLightCurveData* > VLightCurve::getLightCurveData()
{
   return fFluxInterval;
}
