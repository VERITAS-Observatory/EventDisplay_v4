/*  \class VLightCurveUtilities
    \brief class handling basic light curve functions

*/

#include "VLightCurveUtilities.h"

VLightCurveUtilities::VLightCurveUtilities()
{

   fIsZombie = false;
   fASCIIFormSecondColumnIsObservingInterval = false;

   resetLightCurveData();

   setXRTTimeSettings( false );
   setPhaseFoldingValues();

}

/*

   reset light curve data vector

   (TODO: this is a memory leak, fix it)
*/
void VLightCurveUtilities::resetLightCurveData()
{
   fLightCurveData.clear();

   fLightCurveMJD_min =  1.e99;
   fLightCurveMJD_max = -1.e99;
}

bool VLightCurveUtilities::writeASCIIFile( string iFile )
{
    return writeASCIIFile( iFile, fLightCurveData );
}

bool VLightCurveUtilities::writeASCIIFile( string iFile, vector< VLightCurveData* > iV )
{
   ofstream is( iFile.c_str() );
   if( !is )
   {
      cout << "VLightCurveUtilities::writeASCIIFile() error writing light curve to " << iFile << endl;
      return false;
   }
   for( unsigned int i = 0; i < iV.size(); i++ )
   {
      if( iV[i] )
      {
         is << iV[i]->getMJD() << "\t" << iV[i]->getMJDError() << "\t";
	 is << iV[i]->fFlux << "\t" << iV[i]->getFluxError();
	 is << endl;
       }
   }
   is.close();

   return true;
}

bool VLightCurveUtilities::readASCIIFile( string iFile, double iMJDMin, double iMJDMax, double iFluxMultiplier )
{
   resetLightCurveData();

// read in ascii file
   ifstream is( iFile.c_str() );
   if( !is )
   {
      cout << "VLightCurveUtilities::readASCIIFile(): error reading " << iFile << endl;
      fIsZombie = true;
      return false;
   }
   cout << "VLightCurveUtilities::readASCIIFile(): reading " << iFile << endl;

   double iTemp1 = 0.;
   double iTemp2 = 0.;
   double iTemp3 = 0.;

   string is_line;

   while(  getline( is, is_line ) )
   {
       if( is_line.size() == 0 ) continue;

       istringstream is_stream( is_line );

//! no errors are catched here..
       is_stream >> iTemp1;     // second since fXRTMissionTimeStart or MJD (depends on fXRTTimeSettings)
       is_stream >> iTemp2;     // error [s]

// times are given in XRT mission sec.
       if( fXRTTimeSettings )
       {
	  iTemp1  = fXRTMissionTimeStart + iTemp1/(24.0*60.0*60.0);
	  iTemp2 /= (24.0*60.0*60.0);
       }

       if( iMJDMin > 0. && iTemp1 - iTemp2 < iMJDMin ) continue;
       if( iMJDMax > 0. && iTemp1 + iTemp2 > iMJDMax ) continue;

       fLightCurveData.push_back( new VLightCurveData() ); 
// second column is observing intervall
       if( fASCIIFormSecondColumnIsObservingInterval )
       {
	  fLightCurveData.back()->fMJD_Data_min = iTemp1 - iTemp2;
	  fLightCurveData.back()->fMJD_Data_max = iTemp1 + iTemp2;
       }
// second column is MJD max
       else
       {
	  fLightCurveData.back()->fMJD_Data_min = iTemp1;
	  if( iTemp2 > 0. && iTemp2 > iTemp1 ) fLightCurveData.back()->fMJD_Data_max = iTemp2;
	  else                                 fLightCurveData.back()->fMJD_Data_max = iTemp1;
// TODO: check this
	  fLightCurveData.back()->setMJDInterval( fLightCurveData.back()->fMJD_Data_min, fLightCurveData.back()->fMJD_Data_max );
       }

       if( fLightCurveData.back()->fMJD_Data_min < fLightCurveMJD_min )
       {
          fLightCurveMJD_min = fLightCurveData.back()->fMJD_Data_min;
       }
       if( fLightCurveData.back()->fMJD_Data_max > 0. && fLightCurveData.back()->fMJD_Data_max > fLightCurveMJD_max ) 
       {
          fLightCurveMJD_max = fLightCurveData.back()->fMJD_Data_max;
       }
       else if( fLightCurveData.back()->fMJD_Data_min > fLightCurveMJD_max )
       {
          fLightCurveMJD_max = fLightCurveData.back()->fMJD_Data_min;
       }

       is_stream >> iTemp1;     // rate or flux
       is_stream >> iTemp2;     // rate or flux error
       if( !is_stream ) is_stream >> iTemp3;
       else  iTemp3 = iTemp2;

       iTemp1 *= iFluxMultiplier;
       iTemp2 *= iFluxMultiplier;
       iTemp3 *= iFluxMultiplier;

       if( iTemp2 > 0. && iTemp3 > 0. )
       {
          fLightCurveData.back()->fFlux = iTemp1;
          fLightCurveData.back()->fFluxErrorDown = iTemp3;
          fLightCurveData.back()->fFluxErrorUp   = iTemp2;
       }
// error < 0 -> upper flux limit
       else if( iTemp1 > 0. && iTemp2 < 0. )
       {
          fLightCurveData.back()->fUpperFluxLimit = iTemp1;
          fLightCurveData.back()->fFlux = -99.;
          fLightCurveData.back()->setFluxError( -99. );
       }

   }
   is.close();

// update phase folding values
   updatePhaseFoldingValues();

   cout << "VLightCurve::readASCIIFile() total number of light curve data: " << fLightCurveData.size() << endl;
   if( fLightCurveData.size() > 0 ) cout << "\t(MJD range: " << fLightCurveMJD_min << "," << fLightCurveMJD_max << ")" << endl;

   return true;
}

/* 

   print a row for a typical latex table

*/
void VLightCurveUtilities::printLightCurveLaTexTableRow( double iSigmaMinFluxLimits, double iFluxMultiplicator, bool iPrintPhaseValues )
{
   for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
   {
      cout << (int)fLightCurveData[i]->fMJD_Data_min << " - " << (int)fLightCurveData[i]->fMJD_Data_max << " & ";  
      if( fPhase_Period_days > 0. && iPrintPhaseValues )
      {
          cout << setprecision( 2 ) << getPhase( fLightCurveData[i]->getMJD() ) << " & ";
      }
      cout << "VERITAS & ";
// observing time in minutes
      cout << (int)(fLightCurveData[i]->fRunTime/60.) << " & ";
// mean elevation
      cout << setprecision(1) << fixed << fLightCurveData[i]->fRunElevation << " & ";
// on and off events
      cout << (int)fLightCurveData[i]->fNon  << " & ";
      cout << (int)fLightCurveData[i]->fNoff << " & ";
// alpha
      cout << setprecision(2) << fixed << fLightCurveData[i]->fNoffAlpha << " & ";
// significance
      cout << setprecision(1) << fLightCurveData[i]->fSignificance << " & ";
// flux (with error) or upper flux limit)
      if( iSigmaMinFluxLimits != 1 ) cout << fixed;
      else                           cout << scientific;
      if( fLightCurveData[i]->getFluxError() > 0. && fLightCurveData[i]->fSignificance > iSigmaMinFluxLimits )
      {
         cout << setprecision(1) << fLightCurveData[i]->fFlux*iFluxMultiplicator << " $\\pm$ ";
	 cout << fLightCurveData[i]->getFluxError()*iFluxMultiplicator;
      }
      else
      {
         cout << " $<$ " << fLightCurveData[i]->fUpperFluxLimit*iFluxMultiplicator;
      }
      cout << " \\\\";
      cout << endl;
   }
}

void VLightCurveUtilities::printLightCurveWiki( double iMinEnergy_TeV )
{
    cout << "{| border=\"1\" cellspacing=\"0\" cellpadding=\"5\" align=\"center\"" << endl;
    cout << "!MJD" << endl;
    if( fPhase_Period_days > 0. ) cout << "!Phase" << endl;
    cout << "!Observation Time [min]" << endl;
    cout << "!Significance <math>\\sigma</math>" << endl;
    cout << "!Non" << endl;
    cout << "!Noff" << endl;
    cout << "!Alpha" << endl;
    cout << "!Flux (>" << setprecision(2) << iMinEnergy_TeV << " TeV) [cm^-2 s^-1]" << endl;

    for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
    {
      cout << "|- align=\"center\"" << endl;
      cout << "| " << fixed << setprecision( 1 ) << fLightCurveData[i]->getMJD() << endl;
      if( fPhase_Period_days > 0. ) cout << "| " << setprecision( 2 ) << getPhase( fLightCurveData[i]->getMJD() ) << endl;
      cout << "| " << setprecision( 1 ) << fLightCurveData[i]->fRunTime / 60. << endl;
      cout << "| " << setprecision( 1 ) << fLightCurveData[i]->fSignificance << endl;
      cout << "| " << (int)fLightCurveData[i]->fNon << endl;
      cout << "| " << (int)fLightCurveData[i]->fNoff << endl;
      cout << "| " << setprecision(2) << fLightCurveData[i]->fNoffAlpha << endl;
      if( fLightCurveData[i]->getFluxError() >  0. )
      {
         cout << "| " << setprecision( 1 ) << scientific << fLightCurveData[i]->fFlux << "+-" << fLightCurveData[i]->getFluxError() << endl;
      }
      else
      {
        cout << "| <" << setprecision( 1 ) << scientific << fLightCurveData[i]->fUpperFluxLimit << endl;
      }
    }

    cout << "|}" << fixed << endl;


}

void VLightCurveUtilities::printLightCurveDCF()
{
   for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
   {
       cout << "DCF\t";
       cout << fLightCurveData[i]->getMJD() << "\t";
       cout << scientific << setprecision( 4 ) << fLightCurveData[i]->fFlux << "\t";
       cout << fLightCurveData[i]->getFluxError();
       cout << fixed << endl;
   }
}

void VLightCurveUtilities::printLightCurve( bool bFullDetail )
{
// print light curve with many details
   if( bFullDetail )
   {
      for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
      {
	 cout << i << fixed << setprecision( 4 ) << "\tMJD " << fLightCurveData[i]->fMJD_min << " - " << fLightCurveData[i]->fMJD_max;
	 cout << " (" << fLightCurveData[i]->getMJD() << " +- " << fLightCurveData[i]->getMJDError() << ")";
	 if( fPhase_Period_days > 0. )
	 {
	     double iMJD_mean = fLightCurveData[i]->getMJD();
	     cout << ", Phase " << getPhase( iMJD_mean );
	 }
	 if( fLightCurveData[i]->fRunList.size() > 0 ) cout << " # runs: " << fLightCurveData[i]->fRunList.size();
	 cout << endl;
	 if( fLightCurveData[i]->fNon >= 0. ) cout << "\tNon " << fLightCurveData[i]->fNon << "\tNoff " << fLightCurveData[i]->fNoff;
	 cout << "\t Significance: " << fLightCurveData[i]->fSignificance;
	 cout << "\t Tot Time [h]: " << fLightCurveData[i]->fRunTime/3600.;
	 cout << endl;
	 cout << "\tFlux " << scientific << setprecision( 4 ) << fLightCurveData[i]->fFlux;
	 cout << " +- " << fLightCurveData[i]->getFluxError() << "\tUL " << fLightCurveData[i]->fUpperFluxLimit;
	 if( fLightCurveData[i]->fRunFluxCI_lo_1sigma >= 0. )
	 {
	    cout << "\t CI (1 sigma): " << fLightCurveData[i]->fRunFluxCI_lo_1sigma << "\t" << fLightCurveData[i]->fRunFluxCI_up_1sigma;
	    cout << "\t CI (3 sigma): " << fLightCurveData[i]->fRunFluxCI_lo_3sigma << "\t" << fLightCurveData[i]->fRunFluxCI_up_3sigma;
	 }
	 cout << fixed << endl;
	 if( fLightCurveData[i]->fName.size() > 0 ) cout << "\t (" << fLightCurveData[i]->fName << ")" << endl;
      }
   }
// print values useful for e.g. z-correlation analysis
   else
   {
      for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
      {
	 cout << "Light-curve point: ";
         cout << "  "    << fixed << setprecision( 2 ) << fLightCurveData[i]->fMJD_min << " - " << fLightCurveData[i]->fMJD_max;
	 if( fPhase_Period_days > 0. )
	 {
	     double iMJD_mean = fLightCurveData[i]->getMJD();
	     cout << " (phase " << getPhase( iMJD_mean ) << ")";
	 }
	 cout << "     " << scientific << fLightCurveData[i]->fFlux;
	 cout << "     " << scientific << fLightCurveData[i]->getFluxError();
	 cout << fixed << endl;
      }
   }

}

void VLightCurveUtilities::setPhaseFoldingValues( double iZeroPhase_MJD, double iPhase_Days, 
                                                  double iPhaseError_low_Days, double iPhaseError_up_Days,
						  bool bPlotPhase )
{
   fPhase_MJD0 = iZeroPhase_MJD;
   fPhase_Period_days = iPhase_Days;
   fPhaseError_low_fPhase_Period_days = iPhaseError_low_Days;
   fPhaseError_up_fPhase_Period_days = iPhaseError_up_Days;
   fPhasePlotting = bPlotPhase;

   updatePhaseFoldingValues();
}

double VLightCurveUtilities::getPhase( double iMJD )
{
   if( fPhase_Period_days > 0. )
   {
      iMJD = ( iMJD - fPhase_MJD0 ) / fPhase_Period_days;
      iMJD =   iMJD - TMath::Floor( iMJD );
      if( !fPhasePlotting ) iMJD  *= fPhase_Period_days;
   }
   else iMJD = -99.;

   return iMJD;
}

/*
   calculate errors on orbital phase
*/
double VLightCurveUtilities::getPhaseError( double iMJD )
{
   double iError = sqrt(fPhaseError_up_fPhase_Period_days*fPhaseError_up_fPhase_Period_days
                      + fPhaseError_low_fPhase_Period_days*fPhaseError_low_fPhase_Period_days);

   double iP = 0.;

   if( fPhase_Period_days > 0. )
   {
      iP  = ( iMJD - fPhase_MJD0 )*( iMJD - fPhase_MJD0 )/fPhase_Period_days/fPhase_Period_days/fPhase_Period_days/fPhase_Period_days;
      iP *= iError*iError;
   }
   else return -99.;

   return sqrt( iP );
}

double VLightCurveUtilities::getFlux_Mean()
{
   double iMean = 0.;
   double iNN = 0.;
   for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
   {
      if( fLightCurveData[i] )
      {
          iMean += fLightCurveData[i]->fFlux;
	  iNN++;
      }
   }
   if( iNN > 0. ) return iMean / iNN;

   return -1.e99;
}

double VLightCurveUtilities::getFluxError_Mean()
{
   double iMean = 0.;
   double iNN = 0.;
   for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
   {
      if( fLightCurveData[i] )
      {
          iMean += fLightCurveData[i]->getFluxError();
	  iNN++;
      }
   }
   if( iNN > 0. ) return iMean / iNN;

   return -1.e99;
}

double VLightCurveUtilities::getFlux_Variance()
{
   double Sx = 0.;
   double Sxx = 0.;
   double iNN = 0.;

   for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
   {
      if( fLightCurveData[i] )
      {
         Sx  += fLightCurveData[i]->fFlux;
	 Sxx += fLightCurveData[i]->fFlux*fLightCurveData[i]->fFlux;
	 iNN++;
      }
   }

   if( iNN > 1. ) return (1./(iNN-1.) * (Sxx - 1./iNN * Sx * Sx ) );

   return 0.;
}

double VLightCurveUtilities::getFlux_Max()
{
   double iF_max = -1.e90;

   for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
   {
      if( fLightCurveData[i] )
      {
          if( fLightCurveData[i]->fFlux > iF_max ) iF_max = fLightCurveData[i]->fFlux;
      }
   }

   return iF_max;
}

double VLightCurveUtilities::getFlux_Min()
{
   double iF_Min = 1.e90;

   for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
   {
      if( fLightCurveData[i] )
      {
          if( fLightCurveData[i]->fFlux < iF_Min ) iF_Min = fLightCurveData[i]->fFlux;
      }
   }

   return iF_Min;
}

bool VLightCurveUtilities::updatePhaseFoldingValues()
{
   if( fPhase_MJD0 < 0 || fPhase_Period_days < 0 ) return false;

   for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
   {
      if( fLightCurveData[i] )
      {
          fLightCurveData[i]->fPhase_Data_min = getPhase( fLightCurveData[i]->fMJD_Data_min );
          fLightCurveData[i]->fPhase_Data_max = getPhase( fLightCurveData[i]->fMJD_Data_max );
      }
   }
   return true;
}
   
double VLightCurveUtilities::getMeanObservationInterval()
{
   double iM = 0.;
   double iN = 0.;

   for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
   {
      if( fLightCurveData[i] )
      {
          iM += fLightCurveData[i]->getMJDError();
	  iN++;
      }
   }

   if( iN > 0. ) return iM / iN;

   return 0.;
}

/*

   produce a dummy light curve according the given function

*/
bool VLightCurveUtilities::writeASCIIFile( string iFile, TF1 *f1, unsigned int iNPoints, double iMJD_min, double iMJD_max, double iFluxMeanError, bool bClear )
{
     if( !f1 ) return false;

     if( bClear )
     {
        fLightCurveData.clear();

	for( unsigned int i = 0; i < iNPoints; i++ )
	{
	    fLightCurveData.push_back( new VLightCurveData() );
	    fLightCurveData.back()->fMJD_Data_min = iMJD_min + i * (iMJD_max-iMJD_min)/((double)iNPoints);
	    fLightCurveData.back()->fMJD_Data_max = fLightCurveData.back()->fMJD_Data_min;
	    fLightCurveData.back()->fFlux = f1->Eval( fLightCurveData.back()->fMJD_Data_min );
	    fLightCurveData.back()->setFluxError( TMath::Abs( gRandom->Gaus( 0., iFluxMeanError ) ) );
	 }
      }
      else
      {
         for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
	 {
	    if( fLightCurveData[i] )
	    {
	       fLightCurveData[i]->fFlux = f1->Eval( fLightCurveData[i]->getMJD() );
            }
         }
      }
      cout << "VLightCurveUtilities::writeASCIIFile: total number of data points: " << fLightCurveData.size() << endl;
      return writeASCIIFile( iFile );
}
