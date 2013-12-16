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
    fTraceAverageTime = 0.;
    fChanID = 0;
    fpTrazeSize = 0;
    fHiLo = false;
    fDynamicRange = 216;                 // 8bit FADC, switch to low-gain
    fMaxThreshold = 200;
    fMC_FADCTraceStart = 0;
    fMaxSumSearchStart =30;
    fpTrazeSize = 0;
    fpulsetiming_maxPV = 0;
    fpulsetiminglevels_size = 0;

    fTraceIntegrationMethod = 1;

    // Signal Extractors (Maxim)
    SaturLimit=10000; //FIXME discuss with Gernot
    fSliceRMS[1]=9.; fSliceRMS[2]=11.; fSliceRMS[3]=5.; fSliceRMS[4]=4.7; // FIXME: TEMP: should be extracted from traces automatically
    oversampling=4;
    WinToAverage=2*oversampling;
    MaxNumPulses=15;
    PoleZeroFlash=0.96;   // for FlashCam ~10% undershoot  pole zero cancelation after oversampled digital signal differentiation
    PoleZeroDragNec=0.84; // for fwhm=3.16ns
    ProcToAmplFlash=3.;
    ProcToAmplDragNec=5.;

}

void VTraceHandler::reset()
{
    fSum = 0.;
    fTraceAverageTime = 0.;
    fSumWindowFirst = 0;
    fSumWindowLast  = 0;
    fHiLo = false;
}

/*
    analysis routines call this function
*/
void VTraceHandler::setTrace( VVirtualDataReader* iReader, unsigned int iNSamples, double ped, double pedrms, unsigned int iChanID, unsigned int iHitID, double iHiLo )
{
    fPed=ped;
    fPedrms = pedrms;
    fChanID = iChanID;

    reset();

    if( !iReader )
    {
        cout << "VTraceHandler::setTrace( VVirtualDataReader* iReader, double ped, double pedrms, unsigned int iChanID ): no reader set" << endl;
        return;
    }

// copy trace
    if( iNSamples != fpTrace.size() )
    {
        fpTrace.clear();
        for( unsigned int i = 0; i < iNSamples; i++ ) fpTrace.push_back( iReader->getSample_double( iHitID, i+fMC_FADCTraceStart, (i==0) ) );
    }
    else for( unsigned int i = 0; i < iNSamples; i++ ) fpTrace[i] = iReader->getSample_double( iHitID, i+fMC_FADCTraceStart, (i==0) );

    fpTrazeSize = int(fpTrace.size());
    if( iHiLo > 0. )
    {
        apply_lowgain( iHiLo );
        fHiLo = true;
    }
    else fHiLo = false;
}

void VTraceHandler::setTrace( vector<uint16_t> pTrace, double ped, double pedrms, unsigned int iChanID, double iHiLo )
{
    fPed=ped;
    fPedrms = pedrms;
    fChanID = iChanID;
    reset();
// copy trace
    unsigned int i_tsize = pTrace.size();
    if( i_tsize != fpTrace.size() )
    {
        fpTrace.clear();
        for( unsigned int i = 0; i < i_tsize; i++ ) fpTrace.push_back( (double)pTrace[i] );
    }
    else for( unsigned int i = 0; i < i_tsize; i++ ) fpTrace[i] = (double)pTrace[i];

    fpTrazeSize = int(fpTrace.size());
    fHiLo = apply_lowgain( iHiLo );
}

void VTraceHandler::setTrace( vector<uint8_t> pTrace, double ped, double pedrms, unsigned int iChanID, double iHiLo )
{
    fPed=ped;
    fPedrms = pedrms;
    fChanID = iChanID;
    reset();
// copy trace
    unsigned int i_tsize = pTrace.size();
    if( i_tsize != fpTrace.size() )
    {
        fpTrace.clear();
        for( unsigned int i = 0; i < i_tsize; i++ ) fpTrace.push_back( (double)pTrace[i] );
    }
    else for( unsigned int i = 0; i < i_tsize; i++ ) fpTrace[i] = (double)pTrace[i];

    fpTrazeSize = int(fpTrace.size());
    fHiLo = apply_lowgain( iHiLo );
}


bool VTraceHandler::apply_lowgain( double iHiLo )
{
// hilo switch is set
    if( iHiLo > 0. )
    {
       for( int i = 0; i < fpTrazeSize; i++ )
       {
	   fpTrace[i]  = (fpTrace[i]-fPed)*iHiLo;
	   fpTrace[i] += fPed;
       }
       return true;
    }
    return false;
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
    for( int i = fFirst; i < fLast; i++ )
    {
// require that trace is >0.
// (CTA MC write trace values above a certain signal only)
        if( i<fpTrazeSize && fpTrace[i] > 0. )
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

vector<float> VTraceHandler::getFADCTiming(int fFirst, int fLast, bool debug) {

	if(fLast-fFirst<=20) {// small readout window -> don't bother with extra step
		return getPulseTiming( fFirst,fLast, fFirst, fLast );
	}
	int i_start = fFirst;
	int i_stop = fLast;
	double trace_max = 0.;
	unsigned int n255 = 0;
	int maxpos = 0;
	getQuickMax( fFirst, fLast, trace_max, maxpos, n255 );
	
	bool have_first = false;
	bool have_second= false;

	float temp=0;

	//cout << "VTraceHandler::getFADCTiming(): maxpos " << maxpos << ", trace_max " << trace_max  << endl; 

	//find first bin above 50 dc & first bin after that where the trace goes down again
	for(int i=fFirst; i<fLast && !have_second; i++) {
		if ( !have_first && ( fpTrace[i] - fPed ) > 40 ) {
			i_start=i;
			have_first=true;
			//cout << "VTraceHandler::getFADCTiming(): First " << i_start << endl;
		}
		if ( have_first &&  ( fpTrace[i]-fPed ) < temp )  {
			i_stop=i;
			have_second = true;
			//cout << "VTraceHandler::getFADCTiming(): Stop  " << i_stop << endl;
		}
		temp=fpTrace[i]-fPed;
		//cout << "VTraceHandler::getFADCTiming(): i " << i << ", Temp  " << temp << endl;
	}

	if(!have_first && debug) {
		cout << "VTraceHandler::getFADCTiming()  Warning: coulnd't find bin with signal > 40 dc in range " << fFirst << " - " << fLast << endl;
	}
	i_start-=4;
	while (i_start < fFirst ) i_start++;
	i_stop+=4;
	while (i_stop > fLast ) i_stop--;
	return getPulseTiming( i_start, i_stop, i_start, i_stop );
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
    unsigned int n255 = 0;
    getQuickMax( fFirst, fLast, tmax, maxpos, n255 );
}


void VTraceHandler::getQuickMax(int fFirst, int fLast, double &tmax, int &maxpos, unsigned int &n255 )
{
    double it = 0.;
    int nMax = (int)(fDynamicRange*tmax);
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
		if( nMax > 0 && it > nMax ) n255++;
            }
            tmax -= fPed;
            fMax = tmax;
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


double VTraceHandler::getTraceMax( unsigned int &n255, double iHiLo )
{
    double tmax = iHiLo;
    int maxposInt = 0;
    n255 = 0;
    getQuickMax( 0, fpTrazeSize, tmax, maxposInt, n255 );
    return tmax;
}


void VTraceHandler::getTraceMax( double &tmax, double &maxpos )
{
    int maxposInt = 0;
    unsigned int n255 = 0;
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

/*

    select trace integration method

    0: get trace sum between given integration range


    1: get maximum sum from sliding window method (use integration range to calculate integration window only)

*/
bool VTraceHandler::setTraceIntegrationmethod( unsigned int iT )
{
// check method numbers
   if( iT > 4 ) return false;

   fTraceIntegrationMethod = iT;

   return true;
}

double VTraceHandler::getTraceSum( int fFirst, int fLast, bool fRaw ) 
{
    if( fTraceIntegrationMethod == 1 )
    {
       fSumWindowFirst = fFirst;
       fSumWindowLast  = fLast;
       return getQuickSum( fFirst, fLast, fRaw );
    }
    else if( fTraceIntegrationMethod == 2 )
    {
        //return getQuickMaximumSum( 0, 0.5*fpTrace.size(),fLast - fFirst, fRaw );
        return getQuickMaximumSum( 0, fpTrace.size(),fLast - fFirst, fRaw );
    }
    else if( fTraceIntegrationMethod == 3 )  // extraction with oversampling (less time clustering (due to limited sampling) even for short windows )
    {
        //return getMaxSumWithOverSampling(0,0.5*fpTrace.size(),0, fLast-fFirst, fRaw);
        return getMaxSumWithOverSampling(0,fpTrace.size(),0, fLast-fFirst, fRaw);
    }
    else if( fTraceIntegrationMethod == 4 )  // TEMP: IPR calculation from long traces for extractors 2/3
    {
        return getQuickMaximumSum( 0.5*fpTrace.size(), fpTrace.size(),fLast - fFirst, fRaw );            // extractor 2
        //return getMaxSumWithOverSampling( 0.5*fpTrace.size(), fpTrace.size(),0,fLast - fFirst, fRaw );   //extractor 3
    }

    return 0.;
}


//****************************************************************************************
// Various signal extractors (Maxim)
//****************************************************************************************
double VTraceHandler::getQuickMaximumSum(unsigned int iSearchStart, unsigned int iSearchEnd, int iIntegrationWindow, bool fRaw )
{
    fRaw = false;
    unsigned int n=fpTrace.size();
    int saturflag=0;
    int max = 0;
    unsigned int window=iIntegrationWindow;
    unsigned int windowcalib=2*iIntegrationWindow;
    unsigned int SearchEnd=iSearchEnd;
    if((n-window)<=SearchEnd) SearchEnd=n-window;
    int lolimit=0; int lolimitcal=0;
    int uplimit=0; int uplimitcal=0;
    //float SaturLimit=fDynamicRange*10.;//FIXME (need to be discussed with Gernot)
    float tcharge=0, tcharge2=0;
    float arrtime=0, arrtime2=0;
    double charge=0, charge2=0, ampl=0.;

    float muxBINS[n], FADC[n];
    for(unsigned int i=1;i<=n;i++) {muxBINS[i-1]=i-0.5; FADC[i-1]=(float)fpTrace.at(i-1)-fPed;}

    charge=0, charge2=0;
    tcharge=0, tcharge2=0;
    //maxbin   = LocMax(ampl);
    //if(ampl>=SaturLimit) saturflag=1;
    if(iIntegrationWindow==1&&iSearchStart==1) // special case for slice RMS calculation
    {
        charge=FADC[1]; arrtime=muxBINS[1];
        fSum = charge;
        fTraceAverageTime = arrtime;
        fSumWindowFirst = lolimit;
        fSumWindowLast  = uplimit;

        return charge;
    }


    if  (n <= 0) return -1;
    float xmax =0, xmax2=0;
    for (unsigned int i = iSearchStart; i < int(window)+ iSearchStart; i++)   {xmax+=FADC[i]; }
    for (unsigned int i = iSearchStart; i < int(windowcalib)+ iSearchStart; i++){xmax2+=FADC[i];}
    // extract charge for small window **********************************
    for (unsigned int i = iSearchStart; i < SearchEnd; i++){
        if(charge<xmax){
            charge=xmax; max=i;
            uplimit=max+int(window); if(uplimit>int(n)) uplimit=n;
            lolimit=max;        if(lolimit<0)  lolimit=0; }
        xmax=xmax-FADC[i]+FADC[i+window];
    }
    // extract charge for big window ************************************
    uplimitcal=int(uplimit)+(int(windowcalib)-int(window))/2; if(uplimitcal>int(SearchEnd)+(int)window-2) uplimitcal=(int)SearchEnd+(int)window-1;
    lolimitcal=int(lolimit)-(int(windowcalib)-int(window))/2; if(lolimitcal<int(iSearchStart))  lolimitcal=(int)iSearchStart;
    // arrival times *****************************************************
    for(int k=lolimit; k<uplimit;k++)       { tcharge+=muxBINS[k]*FADC[k];                   }
    for(int k=lolimitcal; k<uplimitcal;k++) { tcharge2+=muxBINS[k]*FADC[k]; charge2+=FADC[k];}
    // extract saturated charge (integrate everything to the end) ************************************
    if(saturflag){
        tcharge=0; charge=0; charge2=0;
        uplimitcal=(int)SearchEnd+(int)window-1;
        lolimitcal=lolimit-(int(windowcalib)-int(window))/2; if(lolimitcal<int(iSearchStart))  lolimitcal=iSearchStart;
        for(int k=lolimitcal; k<uplimitcal;k++)  { tcharge+=muxBINS[k]*FADC[k]; charge2+=FADC[k];}
    }

    if(charge!=0) arrtime=tcharge/charge;
    if(charge2!=0)arrtime2=tcharge2/charge2;
    if(arrtime<iSearchStart)   arrtime=0;  if(arrtime>((int)SearchEnd+(int)window-1))  arrtime=((int)SearchEnd+(int)window-1);
    if(arrtime2<iSearchStart)  arrtime2=0; if(arrtime2>((int)SearchEnd+(int)window-1)) arrtime2=((int)SearchEnd+(int)window-1);
    ampl-=fPed;

    fSum = charge;
    fTraceAverageTime = arrtime;
    fSumWindowFirst = lolimit;
    fSumWindowLast  = uplimit;

    return charge;
}

double VTraceHandler::getMaxSumAutoWindow(float AmplThresh, unsigned int iSearchStart, unsigned int iSearchEnd, int iIntegrationWindow, bool fRaw )
{
    fRaw = false;
    unsigned int n=fpTrace.size();
    int saturflag=0;
    int max = 0;
    unsigned int window=iIntegrationWindow;
    unsigned int windowcalib=2*iIntegrationWindow;
    unsigned int SearchEnd=iSearchEnd;
    if((n-window)<=SearchEnd) SearchEnd=n-window;
    int lolimit=0; int lolimitcal=0;
    int uplimit=0; int uplimitcal=0;
    //float SaturLimit=fDynamicRange*10.;//FIXME (need to be discussed with Gernot)
    float tcharge=0, tcharge2=0;
    float arrtime=0, arrtime2=0;
    double charge=0, charge2=0, ampl=0.;

    float muxBINS[n], FADC[n];
    for(unsigned int i=1;i<=n;i++) {muxBINS[i-1]=i-0.5; FADC[i-1]=(float)fpTrace.at(i-1)-fPed;}

    charge=0, charge2=0;
    tcharge=0, tcharge2=0;
    //maxbin   = LocMax(ampl);
    //if(ampl>=SaturLimit) saturflag=1;

    if  (n <= 0) return -1;
    float xmax =0, xmax2=0;
    for (unsigned int i = iSearchStart; i < int(window)+ iSearchStart; i++)   {xmax+=FADC[i]; }
    for (unsigned int i = iSearchStart; i < int(windowcalib)+ iSearchStart; i++){xmax2+=FADC[i];}
    // extract charge for small window **********************************
    for (unsigned int i = iSearchStart; i < SearchEnd; i++){
        if(charge<xmax){
            charge=xmax; max=i;
            uplimit=max+int(window); if(uplimit>int(n)) uplimit=n-1;
            lolimit=max;        if(lolimit<0)  lolimit=1; }
        xmax=xmax-FADC[i]+FADC[i+window];
    }
    // extract charge for big window ************************************
    uplimitcal=int(uplimit)+(int(windowcalib)-int(window))/2; if(uplimitcal>int(SearchEnd)+(int)window-2) uplimitcal=(int)SearchEnd+(int)window-1;
    lolimitcal=int(lolimit)-(int(windowcalib)-int(window))/2; if(lolimitcal<int(iSearchStart))  lolimitcal=(int)iSearchStart;
    // arrival times *****************************************************
    for(int k=lolimit; k<uplimit;k++)       { tcharge+=muxBINS[k]*FADC[k];                   }
    for(int k=lolimitcal; k<uplimitcal;k++) { tcharge2+=muxBINS[k]*FADC[k]; charge2+=FADC[k];}
    // extract saturated charge (integrate everything to the end) ************************************
    if(saturflag){
        tcharge=0; charge=0; charge2=0;
        uplimitcal=(int)SearchEnd+(int)window-1;
        lolimitcal=lolimit-(int(windowcalib)-int(window))/2; if(lolimitcal<int(iSearchStart))  lolimitcal=iSearchStart;
        for(int k=lolimitcal; k<uplimitcal;k++)  { tcharge+=muxBINS[k]*FADC[k]; charge2+=FADC[k];}
    }

    if(charge!=0) arrtime=tcharge/charge;
    if(charge2!=0)arrtime2=tcharge2/charge2;
    if(arrtime<iSearchStart)   arrtime=0;  if(arrtime>((int)SearchEnd+(int)window-1))  arrtime=((int)SearchEnd+(int)window-1);
    if(arrtime2<iSearchStart)  arrtime2=0; if(arrtime2>((int)SearchEnd+(int)window-1)) arrtime2=((int)SearchEnd+(int)window-1);
    ampl-=fPed;

    // extract charge for automatic integration window (extends window )
    int intwin=0;
    int Start=lolimit;
    int Stop=uplimit;

    float chargeAutoWin=0.;

    for(int i=lolimit;i>int(iSearchStart);i--){
        if(FADC[i]>=AmplThresh){
            intwin++;
            //chargeAutoWin+=FADC[i];
            Start=i;
        }
        else {break;}
    }
    for(int i=uplimit;i<int(SearchEnd);i++){
        if(FADC[i]>=AmplThresh){
            intwin++;
            //chargeAutoWin+=FADC[i];
            Stop=i;
        }
        else {break;}
    }
    //*/

    for(int k=Start; k<Stop;k++) { chargeAutoWin+=FADC[k];  }

    fSum = charge;
    //fSum = chargeAutoWin;
    fTraceAverageTime = arrtime;
    //fSumWindowFirst = lolimit;
    //fSumWindowLast  = uplimit;
    fSumWindowFirst = Start;
    fSumWindowLast  = Stop;

    std::cout<<" Charge:"<<charge<<" arrtime:"<<arrtime<<" ChargeAuto:"<<chargeAutoWin<<" autowin:"<<intwin<<" start/stop:"<<Start<<"/"<<Stop<<endl;
    return charge;
    //return chargeAutoWin;
}

double VTraceHandler::getMaxSumWithOverSampling(unsigned int iSearchStart, unsigned int iSearchEnd, unsigned int ElecConcept, int iIntegrationWindow, bool fRaw)
{
    fRaw = false;
    unsigned int n=fpTrace.size();
    int saturflag=0;
    int max = 0;
    float ProcToAmpl=1.;
    unsigned int integwin=iIntegrationWindow*oversampling;
    unsigned int bigintegwin=2*iIntegrationWindow*oversampling;

    int lolimit=0; int lolimitcal=0;
    int uplimit=0; int uplimitcal=0;
    //float SaturLimit=fDynamicRange*10.;//FIXME (need to be discussed with Gernot)
    float tcharge=0., tcharge2=0.;
    float arrtime=0., arrtime2=0.;
    double charge=0., charge2=0., ampl=0.;

    //float muxBINS[n], FADC[n];
    int ElecResBoost=1;
    float polezero=PoleZeroFlash;
    if(ElecConcept==0) {ElecResBoost=5;polezero=PoleZeroDragNec; ProcToAmpl=ProcToAmplDragNec;}
    if(ElecConcept==1) {ProcToAmpl=ProcToAmplFlash;}
    //for(int i=1;i<=n;i++) {muxBINS[i-1]=i-0.5; FADC[i-1]=(float)ptr[i-1]-ped;}

    //overampled array
    unsigned int nfine=n*oversampling;
    unsigned int SearchStart=iSearchStart*oversampling;
    unsigned int SearchEnd=iSearchEnd*oversampling;
    if((nfine-integwin)<=SearchEnd) SearchEnd=(nfine-integwin);
    float muxBINSfine[nfine], FADCfine[nfine]; // ovesampled fadc slices;
    float FADCfineBuf[nfine]; // ovesampled fadc slices;

    // filling oversampled array
    for(unsigned int i=1;i<=n;i++) {
        unsigned int start=oversampling*i-oversampling;
        for(unsigned int j=start;j<start+oversampling;j++)
            {
                muxBINSfine[j]=j+0.5;
                FADCfine[j]=float(ElecResBoost)*ProcToAmpl*((float)fpTrace.at(i-1)-fPed);
            }
    }
    for(unsigned int j=0;j<nfine;j++){
        float val=0;
        int cnt=0;
        for(unsigned int m=0;m<WinToAverage;m++){
            val+=FADCfine[j+m];
            unsigned int end=j+m;
            if(end>=nfine) break;
            cnt++;
        }
        val/=float(cnt);
        FADCfineBuf[j]=val;
    }

    // process oversampled array ************************
    //short* ProcessedFADC=new short[nfine];
    float* ProcessedFADC=new float[nfine];
    //differentiate with pole-zero cancellation
    for(unsigned int j=0;j<nfine;j++){ProcessedFADC[j]=float(int(-polezero*FADCfineBuf[j]+FADCfineBuf[j+1])  ); }
    ProcessedFADC[nfine-1]=ProcessedFADC[nfine-2];
    //rolling average
    for(unsigned int j=0;j<nfine;j++){
        ProcessedFADC[j]=(ProcessedFADC[j]+ProcessedFADC[j+1])/2;
        //std::cout<<"roll. ave j:"<<" val:"<<ProcessedFADC[j]<<std::endl;
    }
    ProcessedFADC[nfine-1]=ProcessedFADC[nfine-2];
    //again rolling average
    for(unsigned int j=0;j<nfine;j++){
        ProcessedFADC[j]=(ProcessedFADC[j]+ProcessedFADC[j+1])/2;
        //std::cout<<"roll. ave j:"<<j<<" val:"<<ProcessedFADC[j]<<" ptr:"<<ptr[j]<<std::endl;
    }
    ProcessedFADC[nfine-1]=ProcessedFADC[nfine-2];

    //******************************************************************************************************************
    if(fPed<=0.00001) //if input value for pedestal is around zero, but positive - this function extracts pedestal charge
    {
        charge=0.,charge2=0.;
        for(unsigned int k=0; k<integwin;k++)    {charge+=ProcessedFADC[nfine-1-k];}
        for(unsigned int k=0; k<bigintegwin;k++) {charge2+=ProcessedFADC[nfine-1-k];}
        //for(int k=0; k<integwin;k++)    {charge+=FADC[k];}
        //for(int k=0; k<bigintegwin;k++) {charge2+=FADC[k];}
        arrtime=-1;
        ampl=-100;
    }
    if(fPed>0.00001)
    {
        charge=0, charge2=0;
        tcharge=0, tcharge2=0;
        //maxbin   = LocMax(n,ptr,ampl);
        //ampl=LocMaxf(nfine,FADCfine);
        //if(ampl>=SaturLimit) saturflag=1;

        //if  (n <= 0) return -1;
        float xmax =0, xmax2=0;
        //for (Int_t i = 0; i < int(integwin); i++)   {xmax+=ProcessedFADC[i]; }
        //for (Int_t i = 0; i < int(bigintegwin); i++){xmax2+=ProcessedFADC[i];}
        for (unsigned int i = SearchStart; i < int(integwin)+ SearchStart; i++)   {xmax+=ProcessedFADC[i]; }
        for (unsigned int i = SearchStart; i < int(bigintegwin)+ SearchStart; i++){xmax2+=ProcessedFADC[i];}
        // extract charge for small window **********************************
//        for (unsigned int i = 0; i < nfine - int(integwin); i++){
        for (unsigned int i = SearchStart; i < SearchEnd; i++){
            if(charge<xmax){
                charge=xmax; max=i;
                uplimit=max+int(integwin); if(uplimit>int(nfine)) uplimit=nfine;
                lolimit=max;        if(lolimit<0)  lolimit=0; }
            xmax=xmax-ProcessedFADC[i]+ProcessedFADC[i+integwin];
            //tcharge=tcharge-muxBINS[i]*FADC[i] + muxBINS[i+integwin]*FADC[i+integwin];
        }
        // extract charge for big window ************************************
        uplimitcal=uplimit+((int(bigintegwin)-int(integwin))/2); if(uplimitcal>int(SearchEnd)+(int)integwin-2) uplimitcal=int(SearchEnd)+(int)integwin-1;
        lolimitcal=lolimit-((int(bigintegwin)-int(integwin))/2); if(lolimitcal<int(SearchStart))  lolimitcal=int(SearchStart);
        // arrival times *****************************************************
        for(int k=lolimit; k<uplimit;k++)       { tcharge+=muxBINSfine[k]*ProcessedFADC[k];                            }
        for(int k=lolimitcal; k<uplimitcal;k++) { tcharge2+=muxBINSfine[k]*ProcessedFADC[k]; charge2+=ProcessedFADC[k];}
        // extract saturated charge (integrate everything to the end) ************************************
        if(saturflag){
            tcharge=0; charge=0; charge2=0;
            uplimitcal=(int)SearchEnd+(int)integwin-1;
            lolimitcal=lolimit-((bigintegwin-integwin)/2); if(lolimitcal<int(SearchStart))  lolimitcal=int(SearchStart);
            for(int k=lolimitcal; k<uplimitcal;k++)  { tcharge+=muxBINSfine[k]*ProcessedFADC[k]; charge2+=ProcessedFADC[k];}
        }

        if(charge!=0.) arrtime=tcharge/charge/float(oversampling);
        if(charge2!=0.)arrtime2=tcharge2/charge2/float(oversampling);
        //arrtime2= FindPulseWidth(0.5*(ampl-fPed),FADC);
        if(arrtime<0)   arrtime=0;  if(arrtime>(int(SearchEnd)+(int)integwin-2)/float(oversampling))  arrtime=(int(SearchEnd)+(int)integwin-1)/float(oversampling);
        if(arrtime2<0)  arrtime2=0; if(arrtime2>(int(SearchEnd)+(int)integwin-2)/float(oversampling)) arrtime2=(int(SearchEnd)+(int)integwin-1)/float(oversampling);
        ampl-=fPed;
    }
    // alternative: time of maximum amplitude (should be better for small signals, distorted by NSB)
    /*
    float tmaxampl=0.;
    float chargeampl=0.;
    int tmaxindex=0;
    ampl=ProcessedFADC[lolimit];
    for(int i = lolimit; i <uplimit; i++){
        if (ampl <=ProcessedFADC[i]){
            ampl = ProcessedFADC[i];
            tmaxindex=i;
        }
    }
    if(tmaxindex<=lolimit) tmaxindex=lolimit+0.5*oversampling;
    if(tmaxindex>=uplimit) tmaxindex=uplimit-0.5*oversampling;
    for(int i = tmaxindex-0.5*oversampling; i <=tmaxindex+0.5*oversampling; i++)
    {
         chargeampl+=ProcessedFADC[i];
         tmaxampl+=muxBINSfine[i]*ProcessedFADC[i];
    }
    if(charge!=0.) {tmaxampl=tmaxampl/chargeampl/float(oversampling);}
    else  tmaxampl=-1.;
    */
    //normalization (for convenient calibration):***************
    //if(ElecConcept==0)
    {
       //charge/=((float)ElecResBoost*ProcToAmpl*(float)oversampling);
       //charge2/=((float)ElecResBoost*ProcToAmpl*(float)oversampling);
       charge/=((float)ElecResBoost*(float)oversampling);
       charge2/=((float)ElecResBoost*(float)oversampling);
    }

    delete[] ProcessedFADC;
    //return ProcessedFADC;
    //return saturflag;
    fSum = charge;
    fTraceAverageTime = arrtime;
    //fTraceAverageTime = tmaxampl;
    fSumWindowFirst = lolimit/oversampling;
    fSumWindowLast  = uplimit/oversampling;
    //std::cout<<" Charge:"<<charge<<" arrtime:"<<arrtime<<"[slices]"<<" intwin:"<<" f/l bin:"<<fSumWindowFirst<<"/"<<fSumWindowLast<<std::endl;

    return charge;
}

double VTraceHandler::getMaximumSums(float AmplThresh, int *integwindows, float *charges, float *arrtimes, bool fRaw)
{
    int   saturflag=-1;
    fRaw = false;
    unsigned int n=fpTrace.size();
    float muxBINS[n], FADC[n];
    int   Start[n], Stop[n];
    int IntWindows[n];
    float Charges[n], Times[n];
    for(unsigned int i=1;i<=n;i++) {muxBINS[i-1]=float(i)-0.5; FADC[i-1]=(float)fpTrace.at(i-1)-fPed;}
    //for(int i=0;i<n;i++)  {IntWindows[i]=0; Charges[i]=0;Times[i]=0;}
    // Define integration windows ******************************************
    //std::cout<<"Thresh:"<<AmplThresh<<" FADC/slice"<<std::endl;
    unsigned int pulsecnt=0;
    int intwin=0;
    for(unsigned int i=1;i<n-1;i++)
    {
        if(FADC[i]>=AmplThresh){intwin++;}
        if(FADC[i]>=SaturLimit){saturflag=i;}
        if(intwin>0 && FADC[i]<=AmplThresh)
        {
            pulsecnt++;
            Stop[pulsecnt]=i;
            Start[pulsecnt]=i-intwin;
            Charges[pulsecnt]=0.;
            Times[pulsecnt]=0.;
            if(intwin>=3)
            {
                for(int m=Start[pulsecnt];m<Stop[pulsecnt];m++){ Charges[pulsecnt]+=FADC[m];}
                for(int m=Start[pulsecnt];m<Stop[pulsecnt];m++){ Times[pulsecnt]+=muxBINS[m]*FADC[m];}
                //for(int m=Start[pulsecnt];m<Start[pulsecnt]+4;m++){ Charges[pulsecnt]+=FADC[m];}
                //for(int m=Start[pulsecnt];m<Start[pulsecnt]+4;m++){ Times[pulsecnt]+=muxBINS[m]*FADC[m];}
            }
            if(intwin<3){
                for(int m=Start[pulsecnt]-1;m<=Start[pulsecnt]+1;m++){ Charges[pulsecnt]+=FADC[m];}
                for(int m=Start[pulsecnt]-1;m<=Start[pulsecnt]+1;m++){ Times[pulsecnt]+=muxBINS[m]*FADC[m];}
                intwin=3;
                Start[pulsecnt]=Start[pulsecnt]-1;
                Stop[pulsecnt]=Stop[pulsecnt]+1;
            }

            if(Charges[pulsecnt]>0.) Times[pulsecnt]/=Charges[pulsecnt];

            //std::cout<<pulsecnt<<" win:"<<intwin<<" Start:"<<Start[pulsecnt]<<" Stop:"<<Stop[pulsecnt]<<" charge:"<<Charges[pulsecnt]<<" "<<Times[pulsecnt]<<std::endl;
            IntWindows[pulsecnt]=intwin;
            intwin=0;
            if(pulsecnt>=MaxNumPulses) break;
        }
    }
    IntWindows[0]=pulsecnt;
    Charges[0]=pulsecnt;
    Times[0]=pulsecnt;
    integwindows[0]=pulsecnt;
    charges[0]=pulsecnt;
    arrtimes[0]=pulsecnt;
    for(unsigned int i=0;i<=pulsecnt;i++)
    {
        integwindows[i+1]=IntWindows[i+1];
        charges[i+1]=Charges[i+1];
        arrtimes[i+1]=Times[i+1];
    }

    // TEMP: getMaximum pulse:
    float MaxCharge=charges[1];
    float MaxChargeTime=arrtimes[1];
    int MaxChargeIndex=1;
    if(pulsecnt>=1){
        for  (unsigned int i = 1; i <=pulsecnt; i++){
            if (MaxCharge <=charges[i]){
                MaxCharge = charges[i];
                MaxChargeTime = arrtimes[i];
                MaxChargeIndex=i;
            }
        }
        std::cout<<pulsecnt<<" maxcharge:"<<MaxCharge<<" arrtime:"<<MaxChargeTime<<" index:"<<MaxChargeIndex<<"  start/stop: "
            <<Start[MaxChargeIndex]<<"/"<<Stop[MaxChargeIndex]<<std::endl;
    }
    if(pulsecnt<1)
    {
        MaxCharge=0;
        MaxChargeTime=-1;
        MaxChargeIndex=1;
        charges[MaxChargeIndex]=0;
    }
    // sorting pulses *******************************************************
    /*
    float signal[pulsecnt];
    int  index[pulsecnt];
    for(int i=0;i<pulsecnt;i++) {signal[i]=Charges[i+1];}
    SortArray(pulsecnt,signal,index,true);
    for(int i=0;i<pulsecnt;i++)
    {
        integwindows[i+1]=IntWindows[index[i]+1];
        charges[i+1]=Charges[index[i]+1];
        arrtimes[i+1]=Times[index[i]+1];
        std::cout<<"  win:"<<integwindows[i+1]<<" charge:"<<charges[i+1]<<" arrtime:"<<arrtimes[i+1]<<std::endl;
    }
    //*/

    fSum = charges[MaxChargeIndex];
    fTraceAverageTime = arrtimes[MaxChargeIndex];
    fSumWindowFirst = Start[MaxChargeIndex];
    fSumWindowLast  = Stop[MaxChargeIndex];

    return charges[MaxChargeIndex];
}


/*
double VTraceHandler::getQuickMaximumSum(unsigned int iSearchStart, int iIntegrationWindow, bool fRaw )
{
    fRaw = false;
    unsigned int n=fpTrace.size();
    int saturflag=0;
    int max = 0;
    int window=iIntegrationWindow;
    int windowcalib=2*iIntegrationWindow;
    unsigned int lolimit=0; int lolimitcal=0;
    unsigned int uplimit=0; int uplimitcal=0;
    //float SaturLimit=fDynamicRange*10.;//FIXME (need to be discussed with Gernot)
    float tcharge=0, tcharge2=0;
    float arrtime=0, arrtime2=0;
    double charge=0, charge2=0, ampl=0.;

    float muxBINS[n], FADC[n];
    for(unsigned int i=1;i<=n;i++) {muxBINS[i-1]=i-0.5; FADC[i-1]=(float)fpTrace.at(i-1)-fPed;}

    charge=0, charge2=0;
    tcharge=0, tcharge2=0;
    //maxbin   = LocMax(ampl);
    //if(ampl>=SaturLimit) saturflag=1;

    if  (n <= 0) return -1;
    float xmax =0, xmax2=0;
    for (unsigned int i = iSearchStart; i < int(window)+ iSearchStart; i++)   {xmax+=FADC[i]; }
    for (unsigned int i = iSearchStart; i < int(windowcalib)+ iSearchStart; i++){xmax2+=FADC[i];}
    // extract charge for small window **********************************
    for (unsigned int i = iSearchStart; i < n - int(window); i++){
        if(charge<xmax){
            charge=xmax; max=i;
            uplimit=max+window; if(uplimit>n) uplimit=n;
            lolimit=max;        if(lolimit<0)  lolimit=0; }
        xmax=xmax-FADC[i]+FADC[i+window];
    }
    // extract charge for big window ************************************
    uplimitcal=int(uplimit)+(windowcalib-window)/2; if(uplimitcal>int(n)) uplimitcal=(int)n;
    lolimitcal=int(lolimit)-(windowcalib-window)/2; if(lolimitcal<int(iSearchStart))  lolimitcal=(int)iSearchStart;
    // arrival times *****************************************************
    for(int k=lolimit; k<uplimit;k++)       { tcharge+=muxBINS[k]*FADC[k];                   }
    for(int k=lolimitcal; k<uplimitcal;k++) { tcharge2+=muxBINS[k]*FADC[k]; charge2+=FADC[k];}
    // extract saturated charge (integrate everything to the end) ************************************
    if(saturflag){
        tcharge=0; charge=0; charge2=0;
        uplimitcal=n;
        lolimitcal=lolimit-(windowcalib-window)/2; if(lolimitcal<iSearchStart)  lolimitcal=iSearchStart;
        for(int k=lolimitcal; k<uplimitcal;k++)  { tcharge+=muxBINS[k]*FADC[k]; charge2+=FADC[k];}
    }

    if(charge!=0) arrtime=tcharge/charge;
    if(charge2!=0)arrtime2=tcharge2/charge2;
    if(arrtime<iSearchStart)   arrtime=0;  if(arrtime>n)  arrtime=n;
    if(arrtime2<iSearchStart)  arrtime2=0; if(arrtime2>n) arrtime2=n;
    ampl-=fPed;

    fSum = charge;
    fTraceAverageTime = arrtime;
    fSumWindowFirst = lolimit;
    fSumWindowLast  = uplimit;

    return charge;
}

/*
double VTraceHandler::getQuickMaximumSum(unsigned int iSearchStart, int iIntegrationWindow, bool fRaw )
{
    //std::cout<<"fTraceIntegrationMethod:"<<fTraceIntegrationMethod<<"  IntWindow: "<<iIntegrationWindow<<std::endl;
    fRaw = false;
    int n=fpTrace.size();
    int saturflag=0;
    int max = 0;
    int window=iIntegrationWindow;
    int windowcalib=2*iIntegrationWindow;
    int lolimit=0, lolimitcal=0;
    int uplimit=0, uplimitcal=0;
    //float SaturLimit=fDynamicRange*10.;//FIXME (need to be discussed with Gernot)
    float tcharge=0, tcharge2=0;
    float arrtime=0, arrtime2=0;
    double charge=0, charge2=0, ampl=0.;

    float muxBINS[n], FADC[n];
    for(int i=1;i<=n;i++) {muxBINS[i-1]=i-0.5; FADC[i-1]=(float)fpTrace.at(i-1)-fPed;}

    charge=0, charge2=0;
    tcharge=0, tcharge2=0;
    //maxbin   = LocMax(ampl);
    //if(ampl>=SaturLimit) saturflag=1;

    if  (n <= 0) return -1;
    float xmax =0, xmax2=0;
    for (Int_t i = 0; i < int(window); i++)   {xmax+=FADC[i]; }
    for (Int_t i = 0; i < int(windowcalib); i++){xmax2+=FADC[i];}
    // extract charge for small window **********************************
    for (Int_t i = 0; i < n - int(window); i++){
        if(charge<xmax){
            charge=xmax; max=i;
            uplimit=max+window; if(uplimit>n) uplimit=n;
            lolimit=max;        if(lolimit<0)  lolimit=0; }
        xmax=xmax-FADC[i]+FADC[i+window];
    }
    // extract charge for big window ************************************
    uplimitcal=uplimit+(windowcalib-window)/2; if(uplimitcal>n) uplimitcal=n;
    lolimitcal=lolimit-(windowcalib-window)/2; if(lolimitcal<0)  lolimitcal=0;
    // arrival times *****************************************************
    for(int k=lolimit; k<uplimit;k++)       { tcharge+=muxBINS[k]*FADC[k];                   }
    for(int k=lolimitcal; k<uplimitcal;k++) { tcharge2+=muxBINS[k]*FADC[k]; charge2+=FADC[k];}
    // extract saturated charge (integrate everything to the end) ************************************
    if(saturflag){
        tcharge=0; charge=0; charge2=0;
        uplimitcal=n;
        lolimitcal=lolimit-(windowcalib-window)/2; if(lolimitcal<0)  lolimitcal=0;
        for(int k=lolimitcal; k<uplimitcal;k++)  { tcharge+=muxBINS[k]*FADC[k]; charge2+=FADC[k];}
    }

    if(charge!=0) arrtime=tcharge/charge;
    if(charge2!=0)arrtime2=tcharge2/charge2;
    if(arrtime<0)   arrtime=0;  if(arrtime>n)  arrtime=n;
    if(arrtime2<0)  arrtime2=0; if(arrtime2>n) arrtime2=n;
    ampl-=fPed;

    fSum = charge;
    fTraceAverageTime = arrtime;
    fSumWindowFirst = lolimit;
    fSumWindowLast  = uplimit;

    return charge;
}
*/
