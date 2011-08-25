/*! \class VTraceHandler
    \brief calculation of trace specific stuff like charge sums, getting the maximum, etc.

    base class for all varitations of trace handlers, see VFitTraceHandler for an example

*/

#include "VTraceHandler.h"

VTraceHandler::VTraceHandler()
{
    fpTrace.assign( 64, 0. );
    fPed = 0.;
    fPedrms = 0.;
    fMax = 0.;
    fSum = 0.;
    fChanID = 0;
    fpTrazeSize = 0;
    fHiLo = false;
    fDynamicRange = 255;
    fMaxThreshold = 200;
    fMC_FADCTraceStart = 0;

    fpTrazeSize = 0;
    fpulsetiming_maxPV = 0;
    fpulsetiminglevels_size = 0;
}


void VTraceHandler::setTrace( VVirtualDataReader* iReader, unsigned int iNSamples, double ped, double pedrms, unsigned int iChanID )
{
    setTrace( iReader, iNSamples, ped, pedrms, iChanID, -1. );
}

/*
    analysis routines call this function
*/
void VTraceHandler::setTrace( VVirtualDataReader* iReader, unsigned int iNSamples, double ped, double pedrms, unsigned int iChanID, double iHiLo )
{
    fPed=ped;
    fPedrms = pedrms;
    fChanID = iChanID;

    if( !iReader )
    {
        cout << "VTraceHandler::setTrace( VVirtualDataReader* iReader, double ped, double pedrms, unsigned int iChanID ): no reader set" << endl;
        return;
    }

// copy trace
    if( iNSamples != fpTrace.size() )
    {
        fpTrace.clear();
        for( unsigned int i = 0; i < iNSamples; i++ ) fpTrace.push_back( (double)iReader->getSample( iChanID, i+fMC_FADCTraceStart, (i==0) ) );
    }
    else for( unsigned int i = 0; i < iNSamples; i++ ) fpTrace[i] = (double)iReader->getSample( iChanID, i+fMC_FADCTraceStart, (i==0) );

    fpTrazeSize = int(fpTrace.size());
    if( iHiLo > 0. )
    {
        apply_lowgain( iHiLo );
        fHiLo = true;
    }
    else fHiLo = false;
}


void VTraceHandler::setTrace(vector<uint8_t> pTrace, double ped, double pedrms, unsigned int iChanID )
{
    fPed=ped;
    fPedrms = pedrms;
    fChanID = iChanID;
// copy trace
    unsigned int i_tsize = pTrace.size();
    if( i_tsize != fpTrace.size() )
    {
        fpTrace.clear();
        for( unsigned int i = 0; i < i_tsize; i++ ) fpTrace.push_back( (double)pTrace[i] );
    }
    else for( unsigned int i = 0; i < i_tsize; i++ ) fpTrace[i] = (double)pTrace[i];

    fpTrazeSize = int(fpTrace.size());
}


void VTraceHandler::setTrace(vector<uint8_t> pTrace, double ped, double pedrms, unsigned int iChanID, double iHiLo )
{
    fPed=ped;
    fPedrms = pedrms;
    fChanID = iChanID;
// copy trace
    unsigned int i_tsize = pTrace.size();
    if( i_tsize != fpTrace.size() )
    {
        fpTrace.clear();
        for( unsigned int i = 0; i < i_tsize; i++ ) fpTrace.push_back( (double)pTrace[i] );
    }
    else for( unsigned int i = 0; i < i_tsize; i++ ) fpTrace[i] = (double)pTrace[i];

    fpTrazeSize = int(fpTrace.size());
    if( iHiLo > 0. )
    {
        apply_lowgain( iHiLo );
        fHiLo = true;
    }
    else fHiLo = false;
}


void VTraceHandler::apply_lowgain( double iHiLo )
{
// hilo switch is set
    for( int i = 0; i < fpTrazeSize; i++ )
    {
// this assumes same pedestal values for low gain (which is only an approximation)
        fpTrace[i]  = (fpTrace[i]-fPed)*iHiLo;
        fpTrace[i] += fPed;
    }
}


void VTraceHandler::calcQuickPed(int fFirst, int fLast)
{
// calculates the pedestal for this channel
    double pedsum=0;
    int count=0;

    for (int i=fFirst; i<fLast; i++)
    {
        if (i<fpTrazeSize)
        {
            count++;
            pedsum+=fpTrace[i];
        }
    }

    if (count>0)
    {
        pedsum=pedsum/(double)count;
    }
    else
    {
        pedsum=0;
    }
//cout << pedsum << " " << count <<endl;
    fPed=pedsum;
}


double VTraceHandler::getQuickSum(int fFirst, int fLast, bool fRaw)
{
    double sum=0.;
    for (int i=fFirst; i<fLast; i++)
    {
        if (i<fpTrazeSize)
        {
            if(!fRaw) sum+= fpTrace[i]-fPed;
            if(fRaw)  sum+= fpTrace[i];
        }
    }
    if( isnan( sum ) ) sum = 0.;
    if( abs( sum ) < 1.e-10 ) sum = 0.;
    fSum=sum;
    return sum;
}


double VTraceHandler::getQuickTZero(int fFirst, int fLast )
{
    return getQuickTZero( fFirst, fLast, fFirst );
}


// calculates the half-max time for this channel
double VTraceHandler::getQuickTZero(int fFirst, int fLast, int fTFirst )
{
    cout << "VTraceHandler::getQuickTZero: WARNING DO NOT USE; FUNCTION OBSOLUTE; USE VTraceHandler::getPulseTiming" << endl;
    if( fFirst < 0 ) fFirst = 0;
    double tzero=-100;
    double halfmax = 0.;
    unsigned int n255 = 0;
    int maxpos = 0;
    getQuickMax(fFirst,fLast,halfmax,maxpos,n255);
    double before = halfmax;
    double after = before;
    halfmax /= 2.;
    double pedsubbed = 0.;
    for (int i=maxpos;i>=fTFirst;i--)
    {
        pedsubbed= fpTrace[i]-fPed;
        if( pedsubbed < halfmax )
        {
            after=pedsubbed;
            tzero=i;
            break;
        }
        before=pedsubbed;
    }
    if( (before-after) > 0.0 ) tzero += (halfmax-after)/(before-after);
    else                       tzero += 0.5;
    return tzero;
}

/*!
   calculate pulse timing 
   (pulse times at different fraction of maximum)

   fFirst, fLast:   range where maximum is determined
   fTFirst, fTLast: range where timing parameters are determined

  (this function still needs some optimization (GM))

*/

vector< float >& VTraceHandler::getPulseTiming( int fFirst, int fLast, int fTFirst, int fTLast )
{
   if( fFirst < 0 ) fFirst = 0;
// reset pulse timing vector
   for( unsigned int i = 0; i < fpulsetiming.size(); i++ ) fpulsetiming[i] = 0.;
   unsigned int m_pos = 0;

// by definition are there always an odd number of values -> centre values is 1
   double i_trace = 0.;

// get pulse maximum
   double trace_max = 0.;
   unsigned int n255 = 0;
   int maxpos = 0;
   getQuickMax( fFirst, fLast, trace_max, maxpos, n255 );
   fpulsetiming[fpulsetiming_maxPV] = (float)maxpos+0.5;

// first half of the pulse
// (loop backwards over pulse)
   bool bBreak = false;
   for( int i = maxpos; i >= fTFirst ; i-- )
   {
       i_trace = fpTrace[i] - fPed;
// loop over all pulse level
       for( unsigned int m = 0; m < fpulsetiming_maxPV; m++ )
       {
          m_pos = fpulsetiming_maxPV-1-m;
	  if( m_pos < fpulsetiminglevels_size && fpulsetiming[m_pos] < 1.e-5 )
	  {
	     if( i_trace < fpulsetiminglevels[m_pos] * trace_max )
	     {
		fpulsetiming[m_pos] = getLinInterpol( fpulsetiminglevels[m_pos] * trace_max, i, i_trace, i+1, fpTrace[i+1] - fPed );
		if( m_pos == 0 ) bBreak = true;
             }
	  } 
       }
       if( bBreak ) break;
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
	  for( unsigned int m_pos = fpulsetiming_maxPV+1; m_pos < fpulsetiminglevels_size; m_pos++ )
	  {
	     if( m_pos < fpulsetiminglevels_size && fpulsetiming[m_pos] < 1.e-5 )
	     {
		if( i_trace < fpulsetiminglevels[m_pos] * trace_max )
		{
    		   fpulsetiming[m_pos] = getLinInterpol( fpulsetiminglevels[m_pos] * trace_max, i, i_trace, i-1, fpTrace[i-1] - fPed );
		   fpulsetiming[m_pos] -= fpulsetiming[fpulsetiminglevels_size - m_pos - 1];
		   if( m_pos == fpulsetiminglevels_size-1 )
		   {
		      bBreak = true;
                   }
		}
	     }
	  }
	  if( bBreak ) break;
      } 
   }

   return fpulsetiming;
}


void VTraceHandler::getQuickMax(int fFirst, int fLast, double &tmax, int &maxpos )
{
    unsigned int n255;
    getQuickMax( fFirst, fLast, tmax, maxpos, n255 );
}


void VTraceHandler::getQuickMax(int fFirst, int fLast, double &tmax, int &maxpos, unsigned int &n255 )
{
    double it;
    n255 = 0;
    tmax=-10000.;
    maxpos=-100;
// high gain channel
    if( !fHiLo )
    {
        if( fFirst >= 0 && fFirst < fLast && fLast <= fpTrazeSize )
        {
            for (int i=fFirst; i<fLast; i++)
            {
                it = fpTrace[i];
                if( it > tmax )
                {
                    tmax=it;
                    maxpos=i;
                }
// this is as hardcoded as it can be
                if( it > 254.9 ) n255++;
            }
            tmax -= fPed;
            fMax = tmax;
        }
    }
// low gain channel
    else
    {
        if( fFirst >= 0 && fFirst < fLast && fLast <= fpTrazeSize )
        {
            for (int i=fLast-1; i>=fFirst; i--)
            {
                it = fpTrace[i];
                if( it < fMaxThreshold && tmax < 0. ) continue;
                if( it < fMaxThreshold / 2 ) break;
                if( it > tmax )
                {
                    tmax=it;
                    maxpos=i;
                }
            }
            tmax -= fPed;
            fMax = tmax;
        }
    }
}


double VTraceHandler::getTraceMax()
{
    double tmax;
    int maxposInt;
    unsigned int n255;
    getQuickMax( 0, fpTrazeSize, tmax, maxposInt, n255 );
    return tmax;
}


double VTraceHandler::getTraceMax( unsigned int &n255 )
{
    double tmax;
    int maxposInt;
    n255 = 0;
    getQuickMax( 0, fpTrazeSize, tmax, maxposInt, n255 );
    return tmax;
}


void VTraceHandler::getTraceMax( double &tmax, double &maxpos )
{
    int maxposInt;
    unsigned int n255;
    getQuickMax( 0, fpTrazeSize, tmax, maxposInt, n255 );
    maxpos = (double)maxposInt;
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
            if( i < 0 ) continue;
            if( i == fpTrazeSize ) continue;
            it = fpTrace[i] - fPed;
            if( it < 0.5*imax )
            {
                p50_min = getLinInterpol( 0.5*imax, i, fpTrace[i] - fPed, i+1, fpTrace[i+1] - fPed );
                break;
            }
        }
        for( int i = maxpos; i < fLast; i++ )
        {
            if( i < 0 ) continue;
            if( i == 0 ) continue;
            it = fpTrace[i] - fPed;
            if( it < 0.5*imax )
            {
                p50_max = getLinInterpol( 0.5*imax + fPed, i-1, fpTrace[i-1] , i, fpTrace[i]  );
                break;
            }
        }
        return (p50_max - p50_min);
    }
    else return 0.;

    return 0.;
}


double VTraceHandler::getLinInterpol( double y5, int x1, double y1, int x2, double y2 )
{
    double a = 0.;
    double b = 0.;
    if( x2-x1 != 0 ) a = (y2-y1)/(double)(x2-x1);
// shift by 0.5 to locate bin center
    if( a != 0. && x2+0.5 > 0 ) b = y2 - a * ((double)(x2)+0.5);

    if( a != 0. ) return (y5-b)/a;

    return 0.;
}

void VTraceHandler::setPulseTimingLevels( vector< float > iP )
{
   fpulsetiminglevels = iP;
   fpulsetiminglevels_size = fpulsetiminglevels.size();;
   fpulsetiming.assign( fpulsetiminglevels_size, 0. );
   fpulsetiming_maxPV = (fpulsetiminglevels_size-1)/2;
}
