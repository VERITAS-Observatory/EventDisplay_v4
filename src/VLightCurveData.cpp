/*! \class VLightCurveData
    \brief data class for light curve calculations


*/

#include "VLightCurveData.h"

VLightCurveData::VLightCurveData( string iName )
{
	fName = iName;
	fDataFileName = "";
	
	bIsZombie = false;
	
	fMJD_min = 0.;
	fMJD_max = 0.;
	
	fEnergy_min_TeV = 0.;
	fEnergy_max_TeV = -1.;
	
	fMJD_Data_min = 0.;
	fMJD_Data_max = 0.;
	fPhase_Data_min = 0.;
	fPhase_Data_max = 0.;
	fRunTime = 0.;
	fRunElevation = 0.;
	fNon = 0.;
	fNoff = 0.;
	fNoffAlpha = 0.;
	fSignificance = 0.;
	fFlux = 0.;
	fFluxErrorUp = 0.;
	fFluxErrorDown = 0.;
	fUpperFluxLimit = 0.;
	fRunFluxCI_lo_1sigma = 0.;
	fRunFluxCI_up_1sigma = 0.;
	fRunFluxCI_lo_3sigma = 0.;
	fRunFluxCI_up_3sigma = 0.;
	fFluxState = "";
}

VLightCurveData::VLightCurveData( const VLightCurveData& p )
{
	bIsZombie = p.bIsZombie;
	
	fName = p.fName;
	fDataFileName = p.fDataFileName;
	
	fRunList = p.fRunList;
	
	fMJD_min = p.fMJD_min;
	fMJD_max = p.fMJD_max;
	
	fEnergy_min_TeV = p.fEnergy_min_TeV;
	fEnergy_max_TeV = p.fEnergy_max_TeV;
	
	fMJD_Data_min = p.fMJD_Data_min;
	fMJD_Data_max = p.fMJD_Data_max;
	fPhase_Data_min = p.fPhase_Data_min;
	fPhase_Data_max = p.fPhase_Data_max;
	fRunTime = p.fRunTime;
	fRunElevation = p.fRunElevation;
	fNon = p.fNon;
	fNoff = p.fNoff;
	fNoffAlpha = p.fNoffAlpha;
	fSignificance = p.fSignificance;
	fFlux = p.fFlux;
	fFluxErrorUp = p.fFluxErrorUp;
	fFluxErrorDown = p.fFluxErrorDown;
	fUpperFluxLimit = p.fUpperFluxLimit;
	fRunFluxCI_lo_1sigma = p.fRunFluxCI_lo_1sigma;
	fRunFluxCI_up_1sigma = p.fRunFluxCI_up_1sigma;
	fRunFluxCI_lo_3sigma = p.fRunFluxCI_lo_3sigma;
	fRunFluxCI_up_3sigma = p.fRunFluxCI_up_3sigma;
	fFluxState = p.fFluxState;
	
}

void VLightCurveData::setFluxCalculationEnergyInterval( double iEMin, double iEMax )
{
	fEnergy_min_TeV = iEMin;
	fEnergy_max_TeV = iEMax;
}

/*

   calculate and fill a single flux value from a anasum result file

*/
bool VLightCurveData::fillTeVEvndispData( string iAnaSumFile, double iThresholdSignificance, double iMinEvents, double iUpperLimit,
		int iUpperlimitMethod, int iLiMaEqu, double iMinEnergy, double E0, double alpha )
{
	fDataFileName = iAnaSumFile;
	
	VFluxCalculation fFluxCalculation( fDataFileName, 1, 0, 100000, fMJD_min, fMJD_max, false );
	if( fFluxCalculation.IsZombie() )
	{
		cout << "VLightCurveData::fill error reading anasum file: " << fDataFileName << endl;
		bIsZombie = true;
		return false;
	}
	fFluxCalculation.setTimeBinnedAnalysis( false );
	fFluxCalculation.setDebug( false );
	fFluxCalculation.setSignificanceParameters( -999., -999. );
	fFluxCalculation.setSignificanceParameters( iThresholdSignificance, iMinEvents, iUpperLimit, iUpperlimitMethod, iLiMaEqu );
	if( fEnergy_max_TeV > 0. )
	{
		fFluxCalculation.setSpectralParameters( iMinEnergy, E0, alpha, fEnergy_max_TeV );
	}
	else
	{
		fFluxCalculation.setSpectralParameters( iMinEnergy, E0, alpha );
	}
	
	
	// reset min/max values
	double iMJDMin = 1.e10;
	double iMJDMax = -1.;
	for( unsigned int i = 0; i < fFluxCalculation.getMJD().size(); i++ )
	{
		if( fFluxCalculation.getRunList()[i] < 0 )
		{
			continue;
		}
		if( fFluxCalculation.getMJD()[i] - 0.5 * fFluxCalculation.getTOn()[i] / 86400. < iMJDMin )
		{
			iMJDMin = fFluxCalculation.getMJD()[i] - fFluxCalculation.getTOn()[i] / 86400.;
		}
		if( fFluxCalculation.getMJD()[i] + 0.5 * fFluxCalculation.getTOn()[i] / 86400. > iMJDMax )
		{
			iMJDMax = fFluxCalculation.getMJD()[i] + fFluxCalculation.getTOn()[i] / 86400.;
		}
	}
	fMJD_Data_min = iMJDMin;
	fMJD_Data_max = iMJDMax;
	
	fFluxCalculation.calculateIntegralFlux( fEnergy_min_TeV );
	
	fRunList  = fFluxCalculation.getRunList();
	fNon      = fFluxCalculation.getNOn( -1 );
	fNoff     = fFluxCalculation.getNOff( -1 );
	fNoffAlpha = fFluxCalculation.getAlpha( -1 );
	fRunTime  = fFluxCalculation.getRunTime( -1 );
	fRunElevation = fFluxCalculation.getRunElevation( -1 );
	fSignificance = fFluxCalculation.getSignificance( -1 );
	double iFluxError = 0;
	fFluxCalculation.getFlux( -1, fFlux, iFluxError, fUpperFluxLimit );
	setFluxError( iFluxError );
	fFluxCalculation.getFluxConfidenceInterval( -1, fRunFluxCI_lo_1sigma, fRunFluxCI_up_1sigma, true );
	fFluxCalculation.getFluxConfidenceInterval( -1, fRunFluxCI_lo_3sigma, fRunFluxCI_up_3sigma, false );
	
	if( fEnergy_max_TeV > 0. )
	{
		fFluxCalculation.calculateIntegralFlux( fEnergy_max_TeV );
		double iFlux = 0.;
		double iFluxE = 0.;
		double iFluxUL = 0.;
		fFluxCalculation.getFlux( -1, iFlux, iFluxE, iFluxUL );
		if( iFluxE > 0. )
		{
			fFlux -= iFlux;
			setFluxError( sqrt( getFluxError()*getFluxError() + iFluxE * iFluxE ) );
		}
	}
	
	return true;
}

double VLightCurveData::getFluxError()
{
	if( fFluxErrorUp > 0. && fFluxErrorDown )
	{
		return 0.5 * ( fFluxErrorUp + fFluxErrorDown );
	}
	else if( fFluxErrorUp > 0. )
	{
		return fFluxErrorUp;
	}
	else if( fFluxErrorDown > 0. )
	{
		return fFluxErrorDown;
	}
	
	return 0.;
}

void VLightCurveData::setFluxError( double iL )
{
	fFluxErrorUp = iL;
	fFluxErrorDown = iL;
}

double VLightCurveData::getMJD()
{
	return 0.5 * ( fMJD_Data_min + fMJD_Data_max );
}

double VLightCurveData::getMJDError()
{
	return 0.5 * ( fMJD_Data_max - fMJD_Data_min );
}

double VLightCurveData::getPhase()
{
	return 0.5 * ( fPhase_Data_min + fPhase_Data_max );
}

double VLightCurveData::getPhaseError()
{
	return 0.5 * ( fPhase_Data_max - fPhase_Data_min );
}
