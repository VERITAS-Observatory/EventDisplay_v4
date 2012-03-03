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

   fLightCurveMJD_min = 1.e99;
   fLightCurveMJD_max = -1.e99;
}

bool VLightCurveUtilities::readASCIIFile( string iFile, double iMJDMin, double iMJDMax )
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

   string is_line;

   while(  getline( is, is_line ) )
   {
       if( is_line.size() == 0 ) continue;

       istringstream is_stream( is_line );

//! no errors are catched here..
       is_stream >> iTemp1;     // second since fXRTMissionTimeStart
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
       else
       {
	  fLightCurveData.back()->fMJD_Data_min = iTemp1;
	  fLightCurveData.back()->fMJD_Data_max = iTemp2;
// TODO: check this
	  fLightCurveData.back()->setMJDInterval( fLightCurveData.back()->fMJD_Data_min, fLightCurveData.back()->fMJD_Data_max );
       }

       if( fLightCurveData.back()->fMJD_Data_min < fLightCurveMJD_min ) fLightCurveMJD_min = fLightCurveData.back()->fMJD_Data_min;
       if( fLightCurveData.back()->fMJD_Data_max > fLightCurveMJD_max ) fLightCurveMJD_max = fLightCurveData.back()->fMJD_Data_max;

       is_stream >> iTemp1;     // rate
       is_stream >> iTemp2;     // rate error

       if( iTemp2 > 0. )
       {
          fLightCurveData.back()->fFlux = iTemp1;
          fLightCurveData.back()->fFluxError = iTemp2;
       }
// error < 0 -> upper flux limit
       else if( iTemp1 > 0. && iTemp2 < 0. )
       {
          fLightCurveData.back()->fUpperFluxLimit = iTemp1;
       }

   }

   is.close();

   cout << "VLightCurve::readASCIIFile() total number of light curve data: " << fLightCurveData.size() << endl;

   return true;
}

/* 

   print a row for a typical latex table

*/
void VLightCurveUtilities::printLightCurveLaTexTableRow( double iSigmaMinFluxLimits, double iFluxMultiplicator )
{
   for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
   {
      cout << (int)fLightCurveData[i]->fMJD_Data_min << " - " << (int)fLightCurveData[i]->fMJD_Data_max << " & ";  
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
      if( fLightCurveData[i]->fFluxError > 0. && fLightCurveData[i]->fSignificance > iSigmaMinFluxLimits )
      {
         cout << setprecision(1) << fLightCurveData[i]->fFlux*iFluxMultiplicator << " $\\pm$ " << fLightCurveData[i]->fFluxError*iFluxMultiplicator;
      }
      else
      {
         cout << " $<$ " << fLightCurveData[i]->fUpperFluxLimit*iFluxMultiplicator;
      }
      cout << " \\\\";
      cout << endl;
   }
}


void VLightCurveUtilities::printLightCurve( bool bFullDetail )
{
// print light curve with many details
   if( bFullDetail )
   {
      for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
      {
	 cout << i << "\tMJD " << fLightCurveData[i]->fMJD_min << " - " << fLightCurveData[i]->fMJD_max;
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
	 cout << "\tFlux " << fLightCurveData[i]->fFlux;
	 cout << " +- " << fLightCurveData[i]->fFluxError << "\tUL " << fLightCurveData[i]->fUpperFluxLimit;
	 if( fLightCurveData[i]->fRunFluxCI_lo_1sigma >= 0. )
	 {
	    cout << "\t CI (1 sigma): " << fLightCurveData[i]->fRunFluxCI_lo_1sigma << "\t" << fLightCurveData[i]->fRunFluxCI_up_1sigma;
	    cout << "\t CI (3 sigma): " << fLightCurveData[i]->fRunFluxCI_lo_3sigma << "\t" << fLightCurveData[i]->fRunFluxCI_up_3sigma;
	 }
	 cout << endl;
	 if( fLightCurveData[i]->fName.size() > 0 ) cout << "\t (" << fLightCurveData[i]->fName << ")" << endl;
      }
   }
// print values useful for e.g. z-correlation analysis
   else
   {
      for( unsigned int i = 0; i < fLightCurveData.size(); i++ )
      {
         cout << "  "    << fLightCurveData[i]->getMJD();
	 cout << "     " << fLightCurveData[i]->fFlux;
	 cout << "     " << fLightCurveData[i]->fFluxError;
	 cout << endl;
      }
   }

}

void VLightCurveUtilities::setPhaseFoldingValues( double iZeroPhase_MJD, double iPhase_Days, bool bPlotPhase )
{
   fPhase_MJD0 = iZeroPhase_MJD;
   fPhase_Period_days = iPhase_Days;
   fPhasePlotting = bPlotPhase;
}

double VLightCurveUtilities::getPhase( double iMJD )
{
   iMJD = ( iMJD - fPhase_MJD0 ) / fPhase_Period_days;
   iMJD =   iMJD - TMath::Floor( iMJD );
   if( !fPhasePlotting ) iMJD  *= fPhase_Period_days;

   return iMJD;
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
