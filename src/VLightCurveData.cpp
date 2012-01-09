/*! \class VLightCurveData  
    \brief data class for light curve calculations

    Revision $Id: VLightCurveData.cpp,v 1.1.2.2 2011/06/16 14:53:04 gmaier Exp $

*/

#include "VLightCurveData.h"

VLightCurveData::VLightCurveData( string iName )
{
   fName = iName;
   fDataFileName = "";

   fMJD_min = 0.;
   fMJD_max = 0.;

   fEnergy_min_TeV = 0.;
   fEnergy_max_TeV = -1.;

   fMJD_Data_min = 0.;
   fMJD_Data_max = 0.;
   fRunTime = 0.;
   fNon = 0.;
   fNoff = 0.;
   fNoffAlpha = 0.;
   fSignificance = 0.;
   fFlux = 0.;
   fFluxError = 0.;
   fUpperFluxLimit = 0.;
   fRunFluxCI_lo_1sigma = 0.;
   fRunFluxCI_up_1sigma = 0.;
   fRunFluxCI_lo_3sigma = 0.;
   fRunFluxCI_up_3sigma = 0.;
}

void VLightCurveData::setFluxCalculationEnergyInterval( double iEMin, double iEMax )
{
   fEnergy_min_TeV = iEMin;
   fEnergy_max_TeV = iEMax;
}

bool VLightCurveData::fillTeVEvndispData( string iAnaSumFile, double iThresholdSignificance, double iMinEvents, double iUpperLimit, int iUpperlimitMethod, int iLiMaEqu, double iMinEnergy, double E0, double alpha )
{
   fDataFileName = iAnaSumFile;

   VFluxCalculation fFluxCalculation( fDataFileName, 1, 0, 100000, fMJD_min, fMJD_max, false );
   if( fFluxCalculation.IsZombie() )
   {
      cout << "VLightCurveData::fill error reading anasum file: " << fDataFileName << endl;
      return false;
   }
   fFluxCalculation.setTimeBinnedAnalysis(false );
   fFluxCalculation.setDebug( false );
   fFluxCalculation.setSignificanceParameters( -999., -999. );
   fFluxCalculation.setSignificanceParameters( iThresholdSignificance, iMinEvents, iUpperLimit, iUpperlimitMethod, iLiMaEqu );
   fFluxCalculation.setSpectralParameters( iMinEnergy, E0, alpha );

// reset min/max values
   double iMJDMin = 1.e10;
   double iMJDMax = -1.;
   for( unsigned int i = 0; i < fFluxCalculation.getMJD().size(); i++ )
   {
      if( fFluxCalculation.getRunList()[i] < 0 ) continue;
      if( fFluxCalculation.getMJD()[i] - 0.5*fFluxCalculation.getTOn()[i]/86400. < iMJDMin )
      {
         iMJDMin = fFluxCalculation.getMJD()[i] - fFluxCalculation.getTOn()[i]/86400.;
      }
      if( fFluxCalculation.getMJD()[i] + 0.5*fFluxCalculation.getTOn()[i]/86400. > iMJDMax )
      {
         iMJDMax = fFluxCalculation.getMJD()[i] + fFluxCalculation.getTOn()[i]/86400.;
      }
   }
   fMJD_Data_min = iMJDMin;
   fMJD_Data_max = iMJDMax;

   fFluxCalculation.calculateIntegralFlux( fEnergy_min_TeV, true );

   fRunList  = fFluxCalculation.getRunList();
   fNon      = fFluxCalculation.getNOn( -1 );
   fNoff     = fFluxCalculation.getNOff( -1 );
   fNoffAlpha= fFluxCalculation.getAlpha( -1 );
   fRunTime  = fFluxCalculation.getRunTime( -1 );
   fSignificance = fFluxCalculation.getSignificance( -1 );
   fFluxCalculation.getFlux( -1, fFlux, fFluxError, fUpperFluxLimit );
   fFluxCalculation.getFluxConfidenceInterval( -1, fRunFluxCI_lo_1sigma, fRunFluxCI_up_1sigma, true );
   fFluxCalculation.getFluxConfidenceInterval( -1, fRunFluxCI_lo_3sigma, fRunFluxCI_up_3sigma, false );

   if( fEnergy_max_TeV > 0. )
   {
      fFluxCalculation.calculateIntegralFlux( fEnergy_max_TeV, true );
      double iFlux = 0.;
      double iFluxE = 0.;
      double iFluxUL = 0.;
      fFluxCalculation.getFlux( -1, iFlux, iFluxE, iFluxUL );
      if( iFluxE > 0. )
      {
         fFlux -= iFlux;
	 fFluxError = sqrt( fFluxError*fFluxError + iFluxE*iFluxE );
      }
   }   

   return true;
}

double VLightCurveData::getMJD()
{
   return 0.5*(fMJD_Data_min+fMJD_Data_max);
}

double VLightCurveData::getMJDError()
{
   return 0.5*( fMJD_Data_max-fMJD_Data_min );
}
