/* \class VTablesToRead
   \brief service class for lookup tables


   \author
   Gernot Maier
*/

#include "VTablesToRead.h"

VTablesToRead::VTablesToRead( int iNTel )
{
	fNTel = ( unsigned int )iNTel;
	
	for( unsigned int i = 0; i < fNTel; i++ )
	{
		hmscwMedian.push_back( 0 );
		hmscwSigma.push_back( 0 );
		hmsclMedian.push_back( 0 );
		hmsclSigma.push_back( 0 );
		henergyERMedian.push_back( 0 );
		henergyERSigma.push_back( 0 );
		henergySRMedian.push_back( 0 );
		henergySRSigma.push_back( 0 );
	}
	
	mscw_T = new double[fNTel];
	mscl_T = new double[fNTel];
	energyER_T = new double[fNTel];
	energySR_T = new double[fNTel];
	mscw_Tsigma = new double[fNTel];
	mscl_Tsigma = new double[fNTel];
	energySR_Tsigma = new double[fNTel];
	
	reset();
	
}


void VTablesToRead::reset()
{
	mscw = -99.;
	mscl = -99.;
	energyER = -99.;
	energySR = -99.;
	energyER_Chi2 = -99.;
	energySR_Chi2 = -99.;
	energyER_dE   = -99.;
	energySR_dE   = -99.;
	
	for( unsigned int i = 0; i < fNTel; i++ )
	{
		mscw_T[i] = -99.;
		mscl_T[i] = -99.;
		energyER_T[i] = -99.;
		energySR_T[i] = -99.;
		mscw_Tsigma[i] = -99.;
		mscl_Tsigma[i] = -99.;
		energySR_T[i] = -99.;
	}
}
