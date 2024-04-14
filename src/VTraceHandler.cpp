/*! \class VTraceHandler
    \brief calculation of trace specific stuff like charge sums, getting the maximum, etc.


*/

#include "VTraceHandler.h"
#include "TMath.h"

VTraceHandler::VTraceHandler()
{
	fpTrace.assign( 64, 0. );
	fPed = 0.;
	fPedrms = 0.;
	fTraceAverageTime = 0.;
	fChanID = 0;
	fpTrazeSize = 0;
	fHiLo = false;
	fDynamicRange = 216;                 // 8bit FADC, switch to low-gain; used only for debugging info
	fMaxThreshold = 150;                 // used for trace max calculation in low gain
	fMC_FADCTraceStart = 0;
	fpTrazeSize = 0;
	fpulsetiming_maxPV = 0;
	fpulsetiminglevels_size = 0;
	fFindPulseTiming = false;

	fTraceIntegrationMethod = 1;
	kIPRmeasure  = false;

}

void VTraceHandler::reset()
{
	fTraceAverageTime = 0.;
	fSumWindowFirst = 0;
	fSumWindowLast  = 0;
	fHiLo = false;
}

/*
    analysis routines call this function

    (generally, this function is called)
*/
void VTraceHandler::setTrace( VVirtualDataReader* iReader, unsigned int iNSamples, double ped, double pedrms, unsigned int iChanID, unsigned int iHitID, double iHiLo )
{
	fPed = ped;
	fPedrms = pedrms;
	fChanID = iChanID;

	reset();

	if( !iReader )
	{
		cout << "VTraceHandler::setTrace( VVirtualDataReader* iReader, double ped, double pedrms, unsigned int iChanID ): no reader set" << endl;
		return;
	}

	///////////////////////////////////////
	// copy trace from raw data reader
	if( iNSamples != fpTrace.size() )
	{
		fpTrace.clear();
		for( unsigned int i = 0; i < iNSamples; i++ )
		{
			fpTrace.push_back( iReader->getSample_double( iHitID, i + fMC_FADCTraceStart, ( i == 0 ) ) );
		}
	}
	else for( unsigned int i = 0; i < iNSamples; i++ )
		{
			fpTrace[i] = iReader->getSample_double( iHitID, i + fMC_FADCTraceStart, ( i == 0 ) );
		}

	fpTrazeSize = int( fpTrace.size() );

	////////////////////////////
	// apply hi-lo gain ratio
	fHiLo = apply_lowgain( iHiLo );
}

/*
 *  used only for time jitter calibration
 *
 */
void VTraceHandler::setTrace( vector<uint16_t> pTrace, double ped, double pedrms, unsigned int iChanID, double iHiLo )
{
	fPed = ped;
	fPedrms = pedrms;
	fChanID = iChanID;
	reset();
	// copy trace
	unsigned int i_tsize = pTrace.size();
	if( i_tsize != fpTrace.size() )
	{
		fpTrace.clear();
		for( unsigned int i = 0; i < i_tsize; i++ )
		{
			fpTrace.push_back( ( double )pTrace[i] );
		}
	}
	else for( unsigned int i = 0; i < i_tsize; i++ )
		{
			fpTrace[i] = ( double )pTrace[i];
		}

	fpTrazeSize = int( fpTrace.size() );
	fHiLo = apply_lowgain( iHiLo );
}

/*
 *  used only for time jitter calibration
 *
 */
void VTraceHandler::setTrace( vector<uint8_t> pTrace, double ped, double pedrms, unsigned int iChanID, double iHiLo )
{
	fPed = ped;
	fPedrms = pedrms;
	fChanID = iChanID;
	reset();
	// copy trace
	unsigned int i_tsize = pTrace.size();
	if( i_tsize != fpTrace.size() )
	{
		fpTrace.clear();
		for( unsigned int i = 0; i < i_tsize; i++ )
		{
			fpTrace.push_back( ( double )pTrace[i] );
		}
	}
	else
	{
		for( unsigned int i = 0; i < i_tsize; i++ )
		{
			fpTrace[i] = ( double )pTrace[i];
		}
	}

	fpTrazeSize = int( fpTrace.size() );
	fHiLo = apply_lowgain( iHiLo );
}


bool VTraceHandler::apply_lowgain( double iHiLo )
{
	// hilo switch is set
	if( iHiLo > 0. )
	{
		for( int i = 0; i < fpTrazeSize; i++ )
		{
			fpTrace[i]  = ( fpTrace[i] - fPed ) * iHiLo;
			fpTrace[i] += fPed;
		}
		return true;
	}
	return false;
}

/*
 * sum up FADC trace from fFirst to fLast
 *
 */
double VTraceHandler::calculateTraceSum_fixedWindow( int fFirst, int fLast, bool iRaw )
{
	double sum = 0.;
	double tcharge = 0.;
	fTraceAverageTime = 0.;
	for( int i = fFirst; i < fLast; i++ )
	{
		// require that trace is >0.
		// (CTA MC write trace values above a certain signal only)
		if( i < fpTrazeSize && fpTrace[i] > 0. )
		{
			if( !iRaw )
			{
				sum += fpTrace[i] - fPed;
				tcharge += ( i + 0.5 ) * ( fpTrace[i] - fPed );
			}
			else
			{
				sum += fpTrace[i];
				tcharge += ( i + 0.5 ) * fpTrace[i];
			}
		}
	}
	if( TMath::IsNaN( sum ) )
	{
		sum = 0.;
	}
	if( TMath::Abs( sum ) < 1.e-10 )
	{
		sum = 0.;
		fTraceAverageTime = 0.;
	}
	else
	{
		fTraceAverageTime = tcharge / sum;
	}
	return sum;
}

double VTraceHandler::getQuickTZero( int fFirst, int fLast )
{
	return getQuickTZero( fFirst, fLast, fFirst );
}


// calculates the half-max time for this channel
double VTraceHandler::getQuickTZero( int fFirst, int fLast, int fTFirst )
{
	cout << "VTraceHandler::getQuickTZero: WARNING DO NOT USE; FUNCTION OBSOLUTE; USE VTraceHandler::getPulseTiming" << endl;
	if( fFirst < 0 )
	{
		fFirst = 0;
	}
	double tzero = -100;
	double halfmax = 0.;
	unsigned int n255 = 0;
	int maxpos = 0;
	getQuickMax( fFirst, fLast, halfmax, maxpos, n255 );
	double before = halfmax;
	double after = before;
	halfmax /= 2.;
	double pedsubbed = 0.;
	for( int i = maxpos; i >= fTFirst; i-- )
	{
		pedsubbed = fpTrace[i] - fPed;
		if( pedsubbed < halfmax )
		{
			after = pedsubbed;
			tzero = i;
			break;
		}
		before = pedsubbed;
	}
	if( ( before - after ) > 0.0 )
	{
		tzero += ( halfmax - after ) / ( before - after );
	}
	else
	{
		tzero += 0.5;
	}
	return tzero;
}

/*
 *  determines timing if L2 pulse to calculate crate timing jitter
 *
 *
*/
vector<float> VTraceHandler::getFADCTiming( int fFirst, int fLast, bool debug )
{

	if( fLast - fFirst <= 20 ) // small readout window -> don't bother with extra step
	{
		return getPulseTiming( fFirst, fLast, fFirst, fLast );
	}
	int i_start = fFirst;
	int i_stop = fLast;
	double trace_max = 0.;
	unsigned int n255 = 0;
	int maxpos = 0;
	getQuickMax( fFirst, fLast, trace_max, maxpos, n255 );

	bool have_first = false;
	bool have_second = false;

	float temp = 0;

	//cout << "VTraceHandler::getFADCTiming(): maxpos " << maxpos << ", trace_max " << trace_max  << endl;

	//find first bin above 50 dc & first bin after that where the trace goes down again
	for( int i = fFirst; i < fLast && !have_second; i++ )
	{
		if( !have_first && ( fpTrace[i] - fPed ) > 40 )
		{
			i_start = i;
			have_first = true;
		}
		if( have_first && ( fpTrace[i] - fPed ) < temp )
		{
			i_stop = i;
			have_second = true;
		}
		temp = fpTrace[i] - fPed;
	}

	if( !have_first && debug )
	{
		cout << "VTraceHandler::getFADCTiming()  Warning: coulnd't find bin with signal > 40 dc in range " << fFirst << " - " << fLast << endl;
	}
	if( i_start >= 4 )
	{
		i_start -= 4;
	}
	while( i_start < fFirst )
	{
		i_start++;
	}
	i_stop += 4;
	while( i_stop > fLast )
	{
		if( i_stop > 0 )
		{
			i_stop--;
		}
	}
	return getPulseTiming( i_start, i_stop, i_start, i_stop );
}

/*!
   calculate pulse timing
   (pulse times at different fraction of maximum)

   fFirst, fLast:   range where maximum is determined
   fTFirst, fTLast: range where timing parameters are determined

*/

vector< float >& VTraceHandler::getPulseTiming( int fFirst, int fLast, int fTFirst, int fTLast )
{
	if( fFirst < 0 )
	{
		fFirst = 0;
	}
	// reset pulse timing vector
	for( unsigned int i = 0; i < fpulsetiming.size(); i++ )
	{
		fpulsetiming[i] = 0.;
	}
	unsigned int m_pos = 0;

	// by definition are there always an odd number of values -> centre value is 1
	double i_trace = 0.;

	// get pulse maximum
	double trace_max = 0.;
	unsigned int n255 = 0;
	int maxpos = 0;
	getQuickMax( fFirst, fLast, trace_max, maxpos, n255 );
	fpulsetiming[fpulsetiming_maxPV] = ( float )maxpos + 0.5;

	// first half of the pulse
	// (loop backwards over pulse)
	bool bBreak = false;
	fFindPulseTiming = false;
	for( int i = maxpos; i >= fTFirst ; i-- )
	{
		i_trace = fpTrace[i] - fPed;
		// loop over all pulse level
		for( unsigned int m = 0; m < fpulsetiming_maxPV; m++ )
		{
			m_pos = fpulsetiming_maxPV - 1 - m;
			if( m_pos < fpulsetiminglevels_size && fpulsetiming[m_pos] < 1.e-5 )
			{
				if( i_trace < fpulsetiminglevels[m_pos] * trace_max )
				{
					fpulsetiming[m_pos] = getLinInterpol( fpulsetiminglevels[m_pos] * trace_max, i, i_trace, i + 1, fpTrace[i + 1] - fPed );
					if( m_pos == 0 )
					{
						fFindPulseTiming = true;
						bBreak = true;
					}
				}
			}
		}
		if( bBreak )
		{
			break;
		}
	}
	// second half of the pulse: contains pulse widths
	// (loop forwards over pulse)
	bBreak = false;
	if( maxpos > 0 )
	{
		for( int i = maxpos; i < fTLast; i++ )
		{
			i_trace = fpTrace[i] - fPed;
			// loop over all pulse level
			for( unsigned int m_pos = fpulsetiming_maxPV + 1; m_pos < fpulsetiminglevels_size; m_pos++ )
			{
				if( m_pos < fpulsetiminglevels_size && fpulsetiming[m_pos] < 1.e-5 )
				{
					if( i_trace < fpulsetiminglevels[m_pos] * trace_max )
					{
						fpulsetiming[m_pos] = getLinInterpol( fpulsetiminglevels[m_pos] * trace_max, i, i_trace, i - 1, fpTrace[i - 1] - fPed );
						fpulsetiming[m_pos] -= fpulsetiming[fpulsetiminglevels_size - m_pos - 1];
						if( m_pos == fpulsetiminglevels_size - 1 )
						{
							bBreak = true;
						}
					}
				}
			}
			if( bBreak )
			{
				break;
			}
		}
	}

	return fpulsetiming;
}


/*
 *
 *   return trace value and position of trace maximum
 *
 *   trace value is pedestal subtracted
 *
 */
void VTraceHandler::getQuickMax( int fFirst, int fLast, double& tmax, int& maxpos )
{
	unsigned int n255 = 0;
	getQuickMax( fFirst, fLast, tmax, maxpos, n255 );
}


void VTraceHandler::getQuickMax( int fFirst, int fLast, double& tmax, int& maxpos, unsigned int& n255 )
{
	double it = 0.;
	int nMax = ( int )( fDynamicRange * tmax );
	n255 = 0;
	// value at maximum
	tmax = -10000.;
	// position of maximum (in samples)
	maxpos = -100;
	/////////////////////////////////////////////////////
	// determine maximum position in a high gain channel
	if( !fHiLo )
	{
		if( fFirst >= 0 && fFirst < fLast && fLast <= fpTrazeSize )
		{
			for( int i = fFirst; i < fLast; i++ )
			{
				it = fpTrace[i];
				if( it > tmax )
				{
					tmax = it;
					maxpos = i;
				}
			}
			tmax -= fPed;
		}
	}
	/////////////////////////////////////////////////////
	// low gain channel
	// (needs special treatment as end of the saturated high gain pulse is
	//  occassionally at the beginning of the readout window)
	else
	{
		if( fFirst >= 0 && fFirst < fLast && fLast <= fpTrazeSize )
		{
			// start search at end of the integration window
			for( int i = fLast - 1; i >= fFirst; i-- )
			{
				it = fpTrace[i];
				if( it < fMaxThreshold && tmax < 0. )
				{
					continue;
				}
				if( it < fMaxThreshold / 2 )
				{
					break;
				}
				if( it > tmax )
				{
					tmax = it;
					maxpos = i;
				}
				// do some rough counting of saturation
				if( nMax > 0 && it > nMax )
				{
					n255++;
				}
			}
			tmax -= fPed;
		}
	}
}


double VTraceHandler::getTraceMax()
{
	double tmax = 0.;
	int maxposInt = 0;
	unsigned int n255 = 0;
	getQuickMax( 0, fpTrazeSize, tmax, maxposInt, n255 );
	return tmax;
}


double VTraceHandler::getTraceMax( unsigned int& n255, double iHiLo )
{
	double tmax = iHiLo;
	int maxposInt = 0;
	n255 = 0;
	getQuickMax( 0, fpTrazeSize, tmax, maxposInt, n255 );
	return tmax;
}


void VTraceHandler::getTraceMax( double& tmax, double& maxpos )
{
	int maxposInt = 0;
	unsigned int n255 = 0;
	getQuickMax( 0, fpTrazeSize, tmax, maxposInt, n255 );
	maxpos = ( double )maxposInt;
}


/*!
    pulsewidth is defined as width of pulse at 50% of maximum
*/

double VTraceHandler::getQuickPulseWidth( int fFirst, int fLast, double fPed )
{
	cout << "VTraceHandler::getQuickPulseWidth: WARNING DO NOT USE; FUNCTION OBSOLUTE; USE VTraceHandler::getPulseTiming" << endl;
	double it = 0.;
	double imax = 0.;
	int maxpos = 0;
	double p50_min = 0.;
	double p50_max = 0.;
	unsigned int n255 = 0;
	// first get pulse maximum
	getQuickMax( fFirst, fLast, imax, maxpos, n255 );

	if( fFirst >= 0 && fFirst < fLast && fLast <= fpTrazeSize )
	{
		for( int i = maxpos; i >= fFirst; i-- )
		{
			if( i < 0 )
			{
				continue;
			}
			if( i == fpTrazeSize )
			{
				continue;
			}
			it = fpTrace[i] - fPed;
			if( it < 0.5 * imax )
			{
				p50_min = getLinInterpol( 0.5 * imax, i, fpTrace[i] - fPed, i + 1, fpTrace[i + 1] - fPed );
				break;
			}
		}
		for( int i = maxpos; i < fLast; i++ )
		{
			if( i < 0 )
			{
				continue;
			}
			if( i == 0 )
			{
				continue;
			}
			it = fpTrace[i] - fPed;
			if( it < 0.5 * imax )
			{
				p50_max = getLinInterpol( 0.5 * imax + fPed, i - 1, fpTrace[i - 1], i, fpTrace[i] );
				break;
			}
		}
		return ( p50_max - p50_min );
	}
	else
	{
		return 0.;
	}

	return 0.;
}


double VTraceHandler::getLinInterpol( double y5, int x1, double y1, int x2, double y2 )
{
	double a = 0.;
	double b = 0.;
	if( x2 - x1 != 0 )
	{
		a = ( y2 - y1 ) / ( double )( x2 - x1 );
	}
	// shift by 0.5 to locate bin center
	if( a != 0. && x2 + 0.5 > 0 )
	{
		b = y2 - a * ( ( double )( x2 ) + 0.5 );
	}

	if( a != 0. )
	{
		return ( y5 - b ) / a;
	}

	return 0.;
}

void VTraceHandler::setPulseTimingLevels( vector< float > iP )
{
	fpulsetiminglevels = iP;
	fpulsetiminglevels_size = fpulsetiminglevels.size();;
	fpulsetiming.assign( fpulsetiminglevels_size, 0. );
	fpulsetiming_maxPV = ( fpulsetiminglevels_size - 1 ) / 2;
}

/*

    select trace integration method

    1: get trace sum between given integration range

    2: get maximum sum from sliding window method (use integration range to calculate integration window only)

*/
bool VTraceHandler::setTraceIntegrationmethod( unsigned int iT )
{
	// check method numbers
	if( iT > 5 )
	{
		return false;
	}

	fTraceIntegrationMethod = iT;

	return true;
}

/*
 *   trace integration
 *
 *   note that there are a number of different trace integration methods
 *
 */
double VTraceHandler::getTraceSum( int iSumWindowFirst, int iSumWindowLast, bool iRaw,
								   unsigned int iTraceIntegrationMethod, bool iForceWindowStart,
								   unsigned int iSlidingWindowLast )
{
	// set trace integration method
	if( iTraceIntegrationMethod < 9999 )
	{
		fTraceIntegrationMethod = iTraceIntegrationMethod;
	}

	// integrate from fFirst to fLast
	if( fTraceIntegrationMethod == 1 )
	{
		fSumWindowFirst = ( unsigned int )iSumWindowFirst;
		fSumWindowLast  = ( unsigned int )iSumWindowLast;
		return calculateTraceSum_fixedWindow( fSumWindowFirst, fSumWindowLast, iRaw );
	}
	// find maximum integral
	else if( fTraceIntegrationMethod == 2 )
	{
		if( !kIPRmeasure )
		{
			// special case: search over restricted window
			if( iForceWindowStart )
			{
				if( iSlidingWindowLast >= fpTrace.size() )
				{
					iSlidingWindowLast = fpTrace.size();
				}
				return calculateTraceSum_slidingWindow( iSumWindowFirst,
														iSlidingWindowLast,
														iSumWindowLast - iSumWindowFirst,
														iRaw );
			}
			// default: search over whole summation window
			else
			{
				return calculateTraceSum_slidingWindow( 0, fpTrace.size(), iSumWindowLast - iSumWindowFirst, iRaw );
			}
		}
		// IPR measurements from long trace file
		else
		{
			return calculateTraceSum_slidingWindow( 0.5 * fpTrace.size(),
													fpTrace.size(),
													iSumWindowLast - iSumWindowFirst,
													iRaw );
		}
	}
	// return simple the trace maximum as trace sum
	else if( fTraceIntegrationMethod == 5 )
	{
		double peakamplitude = getTraceMax();
		double result = 0;
		if( iRaw )
		{
			result = peakamplitude + fPed;
		}
		else
		{
			result = peakamplitude;
		}
		return result;
	}

	return 0.;
}


/*
 *
 * get maximum trace sum
 * (sliding window, search along trace for maximum sum)
 *
 * iSearchEnd is the last FADC bin where the integration is starting from (!)
 *
*/
double VTraceHandler::calculateTraceSum_slidingWindow( unsigned int iSearchStart,
		unsigned int iSearchEnd,
		int iIntegrationWindow,
		bool fRaw )
{
	unsigned int n = fpTrace.size();

	// zero length trace
	if( n == 0 )
	{
		fTraceAverageTime = 0.;
		return 0.;
	}
	// last bin to start search from
	if( ( n - iIntegrationWindow ) <= iSearchEnd )
	{
		iSearchEnd = n - iIntegrationWindow + 1;
	}
	double charge = 0.;
	float ped = fPed;
	if( kIPRmeasure )
	{
		ped = 0.;
	}
	////////////////////////////////////////
	// sample time and value (ped subracted)
	float muxBINS[n], FADC[n];
	for( unsigned int i = 0; i < n; i++ )
	{
		muxBINS[i] = i + 0.5;
		FADC[i] = ( float )fpTrace.at( i ) - ped;
	}

	////////////////////////////////////////
	// special case for ped calculation
	if( fRaw )
	{
		for( unsigned int i = 0; i < ( unsigned int )iIntegrationWindow; i++ )
		{
			charge += ( float )fpTrace.at( n - 1 - i );
		}
		fTraceAverageTime = muxBINS[n - 1];
		fSumWindowFirst = n - iIntegrationWindow;
		fSumWindowLast  = n;

		return FADC[1];
	}

	////////////////////////////////////////
	// sliding window
	fSumWindowFirst = 0;
	fSumWindowLast = 0;

	// first window
	float xmax = 0.;
	for( unsigned int i = iSearchStart; i < iIntegrationWindow + iSearchStart; i++ )
	{
		xmax += FADC[i];
	}
	// extract charge and slide to the right
	for( unsigned int i = iSearchStart; i < iSearchEnd; i++ )
	{
		if( charge < xmax )
		{
			charge = xmax;
			fSumWindowFirst = i;
			fSumWindowLast = i + iIntegrationWindow;
			if( fSumWindowLast > n )
			{
				fSumWindowLast = n;
			}
		}
		xmax = xmax - FADC[i] + FADC[i + iIntegrationWindow];
	}
	// arrival times (weighted average)
	float tcharge = 0.;
	for( unsigned int k = fSumWindowFirst; k < fSumWindowLast; k++ )
	{
		tcharge += muxBINS[k] * FADC[k];
	}
	if( charge != 0. )
	{
		fTraceAverageTime = tcharge / charge;
	}
	if( fTraceAverageTime < iSearchStart )
	{
		fTraceAverageTime = 0.;
	}
	if( fTraceAverageTime > ( ( int )iSearchEnd + ( int )iIntegrationWindow - 1 ) )
	{
		fTraceAverageTime = ( ( int )iSearchEnd + ( int )iIntegrationWindow - 1 );
	}

	return charge;
}
