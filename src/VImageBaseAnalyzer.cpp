/*! \class VImageBaseAnalyzer

    basic routines for VImageAnalyzer, VCalibrator, VDST

 */

#include "VImageBaseAnalyzer.h"

bool VImageBaseAnalyzer::setSpecialChannels()
{
    if( getDebugFlag() ) cout << "VImageBaseAnalyzer::setSpecialChannels " << getEventNumber() << endl;


// set masked (FADC trig) channels
    if( getFADCstopTrig().size() > 0 )
    {
       for ( unsigned int i = 0; i < fReader->getMaxChannels(); i++ )
       {
	   if ( i < fReader->getNumChannelsHit() )
	   {
	       unsigned int i_channelHitID = i;
	       try
	       {
		   i_channelHitID = fReader->getHitID(i);
	       }
	       catch(...)
	       {
		   cout << "VImageBaseAnalyzer::setSpecialChannels(), index out of range (fReader->getHitID) " << i << "(Telescope " << getTelID()+1 << ", event " << getEventNumber() << ")" << endl;
		   continue;
	       }
	       for( unsigned int t = 0; t < getFADCstopTrig().size(); t++ )
	       {
		   if( i_channelHitID == getFADCstopTrig()[t] && i < getMasked().size() )
		   {
		       if(      t == 0 ) getMasked()[i] = 1;
		       else if( t == 1 ) getMasked()[i] = 2;
		       else if( t == 2 ) getMasked()[i] = 3;
		       else if( t == 3 ) getMasked()[i] = 4;
		   }
	       }
	   }
       }
    }

    if (fReader->isMC()) fRunPar->fL2TimeCorrect=false;

///////////////////////////////////////////////////////////////////////////////////
// set channel status
//
// Note: dead channel settings overwrite all pre-existing settings
    for( unsigned int i = 0; i < getChannelStatus().size(); i++ )
    {
       if( getAnaData()->getSpecialChannel() )
       {
          if( getChannelStatus()[i] == 1 )
	  {
              getChannelStatus()[i] = getAnaData()->getSpecialChannel()->getChannelStatus( i );
          }
       }
    }
///////////////////////////////////////////////////////////////////////////////////

    return true;
}

/*

    calculate integrated charges for all pixels

   (to be used for calibration)

*/
void VImageBaseAnalyzer::calcSums(int iFirst, int iLast, bool iMakingPeds, bool iLowGainOnly )
{
    if( getDebugFlag() ) cout << "VImageBaseAnalyzer::calcSums() " << iFirst << "\t" << iLast << endl;

// for DST source file, ignore everything and just get the sums and tzeros
    if( fReader->getDataFormatNum() == 4 || fReader->getDataFormatNum() == 6 )
    {
        setSums( fReader->getSums() );
        return;
    }

// check integration range
    if( iFirst < 0 )
    {
        if( getDebugFlag() ) cout << "void VImageBaseAnalyzer::calcSums warning: set summation start to 0 " << endl;
        iFirst = 0;
    }
    if( iLast > (int)getNSamples() )
    {
        if( getDebugFlag() ) cout << "void VImageBaseAnalyzer::calcSums warning: set summation last to " << getNSamples() << endl;
        iLast = (int)getNSamples();
    }

// calculates the sum over a sample range for all channels
    setSums( 0. );
    unsigned int nhits = fReader->getNumChannelsHit();
// exclude photodiode from this
    if( nhits > getDead(false).size() ) nhits = getDead(false).size();

    for (unsigned int i = 0; i < nhits; i++)
    {
        unsigned int i_channelHitID = 0;
        try
        {
            i_channelHitID = fReader->getHitID(i);
// for low gain pedestal calibration: ignore high gain channels
	    if( iLowGainOnly && i_channelHitID < getHiLo().size() && !getHiLo()[i_channelHitID] ) continue;

            if( i_channelHitID < getHiLo().size() && i_channelHitID < getDead(getHiLo()[i_channelHitID]).size() && !getDead(getHiLo()[i_channelHitID])[i_channelHitID] )
            {
                fReader->selectHitChan(i);
                if( iMakingPeds )
		{
		   fTraceHandler->setTrace( fReader, getNSamples(), getPeds(getHiLo()[i_channelHitID] )[i_channelHitID], 
		                                     getPedrms(getHiLo()[i_channelHitID])[i_channelHitID], i_channelHitID, i, 0. );
                }
                else
		{
		    fTraceHandler->setTrace( fReader, getNSamples(),getPeds(getHiLo()[i_channelHitID] )[i_channelHitID],
		                                                    getPedrms(getHiLo()[i_channelHitID])[i_channelHitID],
								    i_channelHitID, i, getLowGainMultiplier()[i_channelHitID]*getHiLo()[i_channelHitID] );
                }
                fTraceHandler->setTraceIntegrationmethod( getTraceIntegrationMethod() );
                setSums( i_channelHitID, fTraceHandler->getTraceSum(iFirst, iLast,iMakingPeds) );
            }
        }
        catch(...)
        {
            if( getDebugLevel() == 0 )
            {
                cout << "VImageBaseAnalyzer::calcSums(), index out of range (fReader->getHitID) " << i << ", i_channelHitID " << i_channelHitID << endl; 
		cout << "\t nhits: " << nhits << endl;
		cout << "\t (Telescope " << getTelID()+1 << ", event " << getEventNumber() << ")" << endl;
                setDebugLevel( 1 );
            }
            continue;
        }
    }
}


/* 

   calculate trace timing parameters

   (to be used for calibration)

*/
void VImageBaseAnalyzer::calcTZeros(int fFirst, int fLast)
{
    if( getDebugFlag() ) cout << "VImageBaseAnalyzer::calcTZeros() " << fFirst << "\t" << fLast << endl;
// for DST source file, ignore everything and just get the sums and tzeros
    if( fReader->getDataFormatNum() == 4 || fReader->getDataFormatNum() == 6 )
    {
        setPulseTiming( fReader->getTracePulseTiming(), true );
        setPulseTiming( fReader->getTracePulseTiming(), false );
        return;
    }
// check integration range
    if( fFirst < 0 )
    {
        if( getDebugFlag() ) cout << "void VImageBaseAnalyzer::calcTZeros warning: set start to 0 " << endl;
        fFirst = 0;
    }
    if( fLast > (int)getNSamples() )
    {
        if( getDebugFlag() ) cout << "void VImageBaseAnalyzer::calcTZeros warning: set last to " << getNSamples() << endl;
        fLast = (int)getNSamples();
    }
// calculates the rising edge time for all channels
    setPulseTiming( 0., true );

    unsigned int nhits=fReader->getNumChannelsHit();
// exclude photodiode from this
    if( nhits > getDead(false).size() ) nhits = getDead(false).size();

    for (unsigned int i=0; i<nhits; i++)
    {
        unsigned int i_channelHitID=0;
        try
        {
            i_channelHitID = fReader->getHitID(i);
            if( i_channelHitID < getHiLo().size() && i_channelHitID < getDead(getHiLo()[i_channelHitID]).size() && !getDead(getHiLo()[i_channelHitID])[i_channelHitID])
            {
                if( getDebugFlag() ) cout << "\t VImageBaseAnalyzer::calcTZeros() channel ID: " << i_channelHitID << endl;
                fReader->selectHitChan(i);
                fTraceHandler->setTrace(fReader, getNSamples(), getPeds(getHiLo()[i_channelHitID] )[i_channelHitID],
		                        getPedrms(getHiLo()[i_channelHitID])[i_channelHitID], i_channelHitID, i, 
					getLowGainMultiplier()[i_channelHitID]*getHiLo()[i_channelHitID] );
		
                fTraceHandler->setTraceIntegrationmethod( getTraceIntegrationMethod() );
		setPulseTiming( i_channelHitID, fTraceHandler->getPulseTiming( fFirst, fLast, 0, getNSamples() ), true );
            }
        }
        catch(...)
        {
            cout << "VImageBaseAnalyzer::calcTZeros(), index out of range (fReader->getHitID) " << i;
	    cout << "(Telescope " << getTelID()+1 << ", event " << getEventNumber() << ")" << endl;
            continue;
        }
    }
}


void VImageBaseAnalyzer::FADCStopCorrect()
{
    if( fDebug ) cout << "VImageBaseAnalyzer::FADCStopCorrect()" << endl;

// don't do this if there is no L2 trigger
    if( !getReader()->hasLocalTrigger( getTelID() ) ) return;

// do the FADC stop correction only if all channels are set
    if( getFADCstopTrig().size() == 0 ) return;
    for( unsigned int t = 0; t < getFADCstopTrig().size(); t++ )
    {
       if( getFADCstopTrig()[t] < 0 ) return;
    }
    unsigned int i_channelHitID = 0;
    setFADCStopOffsets( 0. );

// special treatment of channels with i_channelHitID beyond camera channels (e.g. 499)
    unsigned int iPedFADCTrigChan = 1000000;
    for( unsigned int t = 0; t < getFADCstopTrig().size(); t++ )
    {
       if( getFADCstopTrig()[t] >= 0 && getFADCstopTrig()[t] < getNChannels() ) iPedFADCTrigChan = getFADCstopTrig()[t];
    }

///////////////////////////////////////////////////////////////////////////////////////
// now get channel jitter

    for( unsigned int t = 0; t < getFADCstopTrig().size(); t++ )
    {
// no FADC stop trig L2 channel set
       if( getFADCstopTrig()[t] < 0 ) continue;

//  calculate TZero for first crate trigger signal
       pair< bool, uint32_t > i_hitIndexPair = fReader->getChannelHitIndex( (unsigned int)getFADCstopTrig()[t] );
       if( !i_hitIndexPair.first ) continue;
       unsigned int i = (unsigned int)i_hitIndexPair.second;
       double crateTZero = 0.;
       double offset = 0.;
       try
       {
	   if( i < fReader->getMaxChannels() && !getReader()->isZeroSuppressed( i ) )
	   {
	       i_channelHitID = fReader->getHitID(i);
	       fReader->selectHitChan((uint32_t)i);
	       if( i_channelHitID < getPeds().size() )
	       {
		   fTraceHandler->setTrace( fReader, getNSamples(),getPeds()[i_channelHitID], getPedrms()[i_channelHitID], i_channelHitID, i, 0. );
	       }
// take pedestal from another FADC trig channel
	       else if( iPedFADCTrigChan < getPeds().size() && !getReader()->isZeroSuppressed( iPedFADCTrigChan ) ) 
	       {
		   fTraceHandler->setTrace( fReader, getNSamples(),getPeds()[iPedFADCTrigChan], getPedrms()[iPedFADCTrigChan], i_channelHitID, i, 0. );
		   i_channelHitID = fReader->getHitID( i );
	       }
	       else continue;
               fTraceHandler->setTraceIntegrationmethod( getTraceIntegrationMethod() );
// calculate t0
	       if( fTraceHandler->getTraceSum( 0, getNSamples(), false ) > 300 )
	       {
     	           crateTZero=fTraceHandler->getPulseTiming(0,getNSamples(), 0, getNSamples() )[getRunParameter()->fpulsetiming_tzero_index];
		   if( i_channelHitID < getHiLo().size() && getHiLo()[i_channelHitID] ) crateTZero = 0.;
	       }
	       getFADCstopTZero()[t] = crateTZero;
// calculate offsets
               if( t > 0 )
	       {
	          if( getFADCstopTZero()[0] > 0. ) offset = getFADCstopTZero()[0] - crateTZero;
		  else                             offset = 0.;
                  unsigned int iC_start = 0;
		  unsigned int iC_stop =  0;
// crate 2
		  if( t == 1 )
		  {
		     iC_start = 130;
		     iC_stop =  250;
                  }
// crate 3
		  else if( t == 2 )
		  {
		     iC_start = 250;
		     iC_stop =  380;
                  }
// crate 4
		  else if( t == 3 )
		  {
		     iC_start = 380;
		     iC_stop =  getTZeros().size();
                  }
// apply offset
		  for( unsigned int j= iC_start; j < iC_stop; j++ )
		  {
		     if( j != i ) setPulseTimingCorrection( j, offset );
		     getFADCStopOffsets()[j] = offset;
                  } 
               } 
	   }
       }
       catch(...)
       {
	   if( getDebugLevel() == 0 )
	   {
	       cout << "VImageBaseAnalyzer::FADCStopCorrect(), index out of range warning (fReader->getHitID) ";
	       cout << "channel " << i << ", hit ID " << i_channelHitID << endl;
	       cout << "zero suppression " << getReader()->isZeroSuppressed(i) << endl;
	       cout << " (Telescope " << getTelID()+1 << ", event " << getEventNumber() << ")";
	       cout << endl;
	       setDebugLevel( 1 );
	   }
       }
   }
}


/*

   this is doing the same as calcTZerosSums, calcTZeros (makes program about 15% faster that way)

   used in VDisplay for event movies, not in general analysis

*/
void VImageBaseAnalyzer::calcTCorrectedSums(int iFirst, int iLast)
{
// for DST source file, ignore everything and just get the sums and tzeros
    if( fReader->getDataFormatNum() == 4 || fReader->getDataFormatNum() == 6 )
    {
        setSums( fReader->getSums() );
	setPulseTiming( fReader->getTracePulseTiming(), true );
	setPulseTiming( fReader->getTracePulseTiming(), false );
        setTraceMax( fReader->getTraceMax() );
        setTraceRawMax( fReader->getTraceRawMax() );
        return;
    }

// calculates the sum over a sample range, corrected for time offsets, for all channels
    setSums( 0. );
    unsigned int nhits = fReader->getNumChannelsHit();
// exclude photodiode from this
    if( nhits > getDead(false).size() ) nhits = getDead(false).size();

    for (unsigned int i = 0; i < nhits; i++)
    {

        unsigned int i_channelHitID = 0;
        try
        {
            i_channelHitID = fReader->getHitID(i);
            if( i_channelHitID < getHiLo().size() && !getDead(getHiLo()[i_channelHitID])[i_channelHitID])
            {
                fReader->selectHitChan(i);
                fTraceHandler->setTrace(fReader, getNSamples(),getPeds(getHiLo()[i_channelHitID] )[i_channelHitID],
		                        getPedrms(getHiLo()[i_channelHitID])[i_channelHitID], i_channelHitID, i, 
				 	getLowGainMultiplier()[i_channelHitID]*getHiLo()[i_channelHitID] );
 
		fTraceHandler->setTraceIntegrationmethod( getTraceIntegrationMethod() );
                int offset=0;
                if( !getRunParameter()->fFixWindowStart )
                {
                    if (getTOffsets()[i_channelHitID]>0) offset=(int)getTOffsets()[i_channelHitID];
                    if (getTOffsets()[i_channelHitID]<0) offset=(int)(getTOffsets()[i_channelHitID]-1.);
                }
                int corrfirst=iFirst+offset;
                if (corrfirst<0) corrfirst=0;
                if (corrfirst > (int)getNSamples()) corrfirst = (int)getNSamples();
                int corrlast=iLast+offset;
                if (corrlast<0) corrlast=0;
                if (corrlast > (int)getNSamples()) corrlast = (int)getNSamples();

                setTCorrectedSumFirst( i_channelHitID, corrfirst );
                setTCorrectedSumLast( i_channelHitID, corrlast );
                setCurrentSummationWindow( i_channelHitID, corrfirst, corrlast );
                setSums( i_channelHitID, fTraceHandler->getTraceSum( corrfirst, corrlast, fRaw ) );
            }
        }
        catch(...)
        {
            cout << "VImageBaseAnalyzer::calcTCorrectedSums(), index out of range (fReader->getHitID) " << i << "(Telescope " << getTelID()+1 << ", event " << getEventNumber() << ")" << endl;
            continue;
        }
    }
}


/*

     calculate sums and timing parameters of FADC traces


     this function is called from VAnalyzer::doAnalysis()

*/
void VImageBaseAnalyzer::calcTZerosSums( int iFirstSum, int iLastSum, unsigned int iTraceIntegrationMethod )
{
    if( getDebugFlag() ) cout << "VImageBaseAnalyzer::calcTZerosSums() \t" << iFirstSum << "\t" << iLastSum << endl;

/////////////////////////////////////////////////////////////////////////////////
// DST source file, 
// ignore everything and just get the sums and tzeros from data trees
    if( !fReader->hasFADCTrace() || !getRunParameter()->doFADCAnalysis() )
    {
        if( getDebugFlag() ) cout << "VImageBaseAnalyzer::calcTZerosSums() reading sums from DST file" << endl;
        setSums( fReader->getSums() );
        setSums2( fReader->getSums() );
	setPulseTiming( fReader->getTracePulseTiming(), true );
	setPulseTiming( fReader->getTracePulseTiming(), false );
        setTraceMax( fReader->getTraceMax() );
        setTraceRawMax( fReader->getTraceRawMax() );
	setTraceAverageTime( 0. );
        return;
    }

/////////////////////////////////////////////////////////////////////////////////
// FAD trace analysis

// reset the data vectors
    if( getTraceFit() > -1 )
    {
        setTraceRiseTime( 0. );
        setTraceFallTime( 0. );
        setTraceChi2( 0. );
    }
    setPulseTiming( 0., true );    // corrected   times
    setPulseTiming( 0., false );   // uncorrected times
    setTraceN255( 0 );
    setTraceAverageTime( 0. );
    setSums( 0. );
    setSums2( 0. );

//////////////////////////////////////////////////////
// calculates the sum over a sample range, corrected for time offsets, for all channels
//////////////////////////////////////////////////////
    unsigned int nhits = fReader->getNumChannelsHit();
// exclude photodiode from this
    if( nhits > getDead(false).size() ) nhits = getDead(false).size();
    unsigned int ndead_size = getDead().size();

    double i_tempTraceMax = 0;
    unsigned int i_tempN255 = 0;
    int corrfirst = 0;
    int corrlast = 0;

// loop over all channels
    for (unsigned int i=0; i<nhits; i++)
    {
        unsigned int i_channelHitID=0;
        try
        {
            i_channelHitID = fReader->getHitID(i);
	    pair< bool, uint32_t > a = fReader->getChannelHitIndex( i_channelHitID );
        }
        catch(...)
        {
            if( getDebugLevel() == 0 )
            {
                cout << "VImageBaseAnalyzer::calcTZerosSums, index out of range " << i;
		cout << "(Telescope " << getTelID()+1 << ", event " << getEventNumber() << ")" << endl;
                setDebugLevel( 1 );
            }
            continue;
        }
        if( i_channelHitID >= ndead_size ) continue;
// calculate tzero and sums for good channels only
        if(!getDead(getHiLo()[i_channelHitID])[i_channelHitID])
        {
// initialize trace handler
            fTraceHandler->setTrace( fReader, getNSamples(), 
	                             getPeds(getHiLo()[i_channelHitID] )[i_channelHitID], getPedrms(getHiLo()[i_channelHitID])[i_channelHitID], 
				     i_channelHitID, i, getLowGainMultiplier()[i_channelHitID]*getHiLo()[i_channelHitID] );
            fTraceHandler->setTraceIntegrationmethod( iTraceIntegrationMethod );
// integration range
// get time offsets (from laser data)
            int offset=0;
            if( !getRunParameter()->fFixWindowStart )
            {
                if (getTOffsets()[i_channelHitID]>0) offset=(int)getTOffsets()[i_channelHitID];
                if (getTOffsets()[i_channelHitID]<0) offset=(int)getTOffsets()[i_channelHitID]-1;
            }
// start of summation window
            corrfirst = iFirstSum + offset;
            if (corrfirst < 0) corrfirst = 0;
            if (corrfirst > (int)getNSamples()) corrfirst=(int)getNSamples();
            setTCorrectedSumFirst( i_channelHitID, corrfirst );
// end of summation window
            corrlast=iLastSum+offset;
            if (corrlast<0) corrlast=0;
            if (corrlast>(int)getNSamples()) corrlast=(int)getNSamples();
            setTCorrectedSumLast( i_channelHitID, corrlast );
// set current summation window (might change from pixel to pixel and event to event
            setCurrentSummationWindow( i_channelHitID, corrfirst, corrlast );
// calculate timing parameters (raw and corrected; tzero correction happens later)
	    setPulseTiming( i_channelHitID, fTraceHandler->getPulseTiming( corrfirst, corrlast, 0, getNSamples() ), true );
// shift the summation window if necessary
            if( getSumWindowShift() != 0 && !getRunParameter()->fFixWindowStart )
            {
                corrfirst = iFirstSum + (int)getTOffsets()[i_channelHitID] + getSumWindowShift();
                if( corrfirst < 0 ) corrfirst = 0;
                corrlast = corrfirst + (iLastSum - iFirstSum);
                setTCorrectedSumFirst( i_channelHitID, corrfirst );
                setTCorrectedSumLast( i_channelHitID, corrlast );
            }
	    setSums( i_channelHitID, fTraceHandler->getTraceSum(corrfirst, corrlast, fRaw ) );
	    setTCorrectedSumFirst( i_channelHitID, fTraceHandler->getTraceIntegrationFirst() );
	    setTCorrectedSumLast( i_channelHitID, fTraceHandler->getTraceIntegrationLast() );
	    setTraceAverageTime( i_channelHitID, fTraceHandler->getTraceAverageTime() );
            i_tempTraceMax = fTraceHandler->getTraceMax( i_tempN255, getLowGainMultiplier()[i_channelHitID] );
            setTraceMax( i_channelHitID, i_tempTraceMax );
            setTraceRawMax( i_channelHitID, i_tempTraceMax + getPeds(getHiLo()[i_channelHitID] )[i_channelHitID] );
            setTraceN255( i_channelHitID, i_tempN255 );
            if( getFillMeanTraces() )
	    {
	       setTrace( i_channelHitID, fTraceHandler->getTrace(), getHiLo()[i_channelHitID], getPeds(getHiLo()[i_channelHitID] )[i_channelHitID] );
            }
// the following parameters will be recalculated in the second pass
            if( !getRunParameter()->fDoublePass )
	    {
// don't fill pulse timing in first pass of double pass (saves time)
	       if( getFillPulseSum() ) getAnaData()->fillPulseSum( i_channelHitID, getSums()[i_channelHitID], getHiLo()[i_channelHitID] );
	       setSums2( i_channelHitID, fTraceHandler->getTraceSum( corrfirst, corrfirst+(int)getSumWindow_2(), fRaw ) );
            }

// get results for trace fitting
// rise and fall time are 10/90% values
            if( getTraceFit() > -1 )
            {
                setTraceChi2( i_channelHitID, fTraceHandler->getTraceChi2() );
                setTraceFallTime( i_channelHitID, fTraceHandler->getTraceFallTime( getPeds(getHiLo()[i_channelHitID] )[i_channelHitID], 0.9, 0.1 ) );
                setTraceRiseTime( i_channelHitID, fTraceHandler->getTraceRiseTime( getPeds(getHiLo()[i_channelHitID] )[i_channelHitID], 0.1, 0.9 ) );
                setTraceFallTimeParameter( i_channelHitID, fTraceHandler->getTraceFallTime() );
                setTraceRiseTimeParameter( i_channelHitID, fTraceHandler->getTraceRiseTime() );
                setTraceNorm( i_channelHitID, fTraceHandler->getTraceNorm() );
            }
        }
// calculate size of FADC stop channel
        else
        {
            for( unsigned int c = 0; c < getFADCstopTrig().size(); c++ )
            {
                if( i_channelHitID == getFADCstopTrig()[c] )
                {
                    fReader->selectHitChan((uint32_t)i);
		    if( fReader->has16Bit() )
		    {
		        fTraceHandler->setTrace(fReader->getSamplesVec16Bit(), getPeds(getHiLo()[i_channelHitID])[i_channelHitID],
			                        getPedrms(getHiLo()[i_channelHitID])[i_channelHitID], i_channelHitID,
						getLowGainMultiplier()[i_channelHitID]*getHiLo()[i_channelHitID] );
                    }
		    else
		    {
		        fTraceHandler->setTrace(fReader->getSamplesVec(), getPeds(getHiLo()[i_channelHitID])[i_channelHitID],
			                        getPedrms(getHiLo()[i_channelHitID])[i_channelHitID], i_channelHitID,
						getLowGainMultiplier()[i_channelHitID]*getHiLo()[i_channelHitID] );
                    }
		    fTraceHandler->setTraceIntegrationmethod( iTraceIntegrationMethod );
                    getFADCstopSums()[c] = fTraceHandler->getTraceSum(0, getNSamples(), fRaw );
                }
            }
        }
    }
// fill tzero vector with uncorrected times
    setPulseTiming( getPulseTiming( true ), false );
}

/*!
    apply relative gain corretion

*/
void VImageBaseAnalyzer::gainCorrect()
{
    if( fDebug ) cout << "void VImageBaseAnalyzer::gainCorrect()" << endl;

// do not gain correct if traces where not calculated by evndisp (using e.g. sim_telarray analysis results)
    if( !getRunParameter()->doFADCAnalysis() ) return;

// loop over all channels
    unsigned int nc = getSums().size();
    for( unsigned int i = 0; i < nc; i++ )
    {
// apply gain correction for HIGHQE channels
        double iHIGHQE = getHIGHQE_gainfactor( i );
        if( iHIGHQE <= 0. ) iHIGHQE = 1.;
// correct gains
        if ( getGains()[i] > 0  && !getDead(getHiLo()[i])[i] ) setSums( i, getSums()[i] / (getGains()[i] * iHIGHQE) );
        else setSums( i, 0. );
        if ( getGains()[i] > 0  && !getDead(getHiLo()[i])[i] ) setSums2( i, getSums2()[i] / (getGains()[i] * iHIGHQE) );
        else setSums2( i, 0. );
    }
}


/*!
  added maximum time offset (GM)

  dead channel coding
  - outside pedestal range (1)
  - small absolute pedvars (2)
  - small relative pedvars (3)
  - large relative pedvars (4)
  - outside gain range (5)
  - small gain vars (6)
  - large gain vars (7)
  - large time offset (8)
  - FADC stop signal (9)
  - masked (10)
  - user set (11)
  - MC dead (12)
  - gain multiplier (13)

*/
void VImageBaseAnalyzer::findDeadChans( bool iLowGain, bool iFirst )
{
    if( fDebug ) cout << "VImageBaseAnalyzer::findDeadChans( bool iLowGain )" << endl;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// do not set anything dead for PE-mode
    if( getRunParameter()->fsourcetype == 6 ) return;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// DST: read dead channel list from DST file
    if( getRunParameter()->fsourcetype == 7 || getRunParameter()->fsourcetype == 4 )
    {
        for ( unsigned int i = 0; i < getNChannels(); i++ )
        {
            if( getReader()->getDead()[i] > 0 ) setDead( i, TMath::Nint( log( (float)getReader()->getDead()[i] )/log(2.) ), iLowGain );
            else                                setDead( i, 0, iLowGain );
        }
// getFullAnaVec()[i]: -1: dead channel, 0: channel does not exist, 1 channel o.k.
        if( getNChannels() >= getDetectorGeometry()->getAnaPixel().size() )
        {
            for( unsigned int i = 0; i < getNChannels(); i++ )
            {
                if( getDetectorGeometry()->getAnaPixel()[i] < 1 ) setDead( i, 12, iLowGain );
            }
        }
        return;
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// for plotting ignore dead channels (no pedestal information available)
    if( getRunParameter()->fPlotRaw ) return;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// find dead channels from calibration and event data

// check if dead channel configuration is time dependent
    if( !iFirst && !usePedestalsInTimeSlices( iLowGain ) ) return;

// reset dead channel vector
    if( iFirst ) setDead( false, iLowGain );

// get mean and rms of pedvar
    double i_meanPedVar = 0.;
    double i_meanPedVarRMS = 0.;
    if( usePedestalsInTimeSlices( iLowGain ) )
    {
        getmeanPedvars( i_meanPedVar, i_meanPedVarRMS, iLowGain, getSumWindow() );
    }
    else
    {
        i_meanPedVar = getmeanPedvars( iLowGain, getSumWindow() );
        i_meanPedVarRMS = getmeanRMSPedvars( iLowGain, getSumWindow() );
    }

    for ( unsigned int i = 0; i < getNChannels(); i++ )
    {
// FADC stop channels (don't set any other reasons for channels to be dead)
        if( iFirst )
        {
            if( getMasked()[i] )
            {
                setDead( i, 9, iLowGain );
                continue;
            }
// TEMP! quickfix for vbf simulation files
            for( unsigned int t = 0; t < getFADCstopTrig().size(); t++ )
            {
                if( getFADCstopTrig()[t] == i )
                {
                    setDead( i, 9, iLowGain );
                    continue;
                }
            }
        }

// calibration data (pedestals in time slices)
        if( usePedestalsInTimeSlices( iLowGain ) )
        {
// reset bits for ped settings
            setDead( i, 1, iLowGain, false, true );
            setDead( i, 2, iLowGain, false, true );
            setDead( i, 3, iLowGain, false, true );
            setDead( i, 4, iLowGain, false, true );

// check values
            setDead( i, getDeadChannelFinder( iLowGain && getLowGainPedestals() )->testPedestals( i, getPeds( iLowGain )[i] ), iLowGain );
            setDead( i, getDeadChannelFinder( iLowGain && getLowGainPedestals() )->testPedestalVariations( i, getPedvars( iLowGain, getSumWindow() )[i] ), iLowGain );
            setDead( i, getDeadChannelFinder( iLowGain && getLowGainPedestals() )->testPedestalVariationsMinOut( i, getPedvars( iLowGain, getSumWindow() )[i], i_meanPedVar, i_meanPedVarRMS ), iLowGain );
            setDead( i, getDeadChannelFinder( iLowGain && getLowGainPedestals() )->testPedestalVariationsMaxOut( i, getPedvars( iLowGain, getSumWindow() )[i], i_meanPedVar, i_meanPedVarRMS ), iLowGain );
        }
// same data for whole run
        else if( iFirst )
        {
            setDead( i, getDeadChannelFinder( iLowGain && getLowGainPedestals() )->testPedestals( i, getPeds( iLowGain )[i] ), iLowGain );
            setDead( i, getDeadChannelFinder( iLowGain && getLowGainPedestals() )->testPedestalVariations( i, getPedvars( iLowGain )[i] ), iLowGain );
            setDead( i, getDeadChannelFinder( iLowGain && getLowGainPedestals() )->testPedestalVariationsMinOut( i, getPedvars( iLowGain )[i], i_meanPedVar, i_meanPedVarRMS ), iLowGain );
            setDead( i, getDeadChannelFinder( iLowGain && getLowGainPedestals() )->testPedestalVariationsMaxOut( i, getPedvars( iLowGain )[i], i_meanPedVar, i_meanPedVarRMS ), iLowGain );
        }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// time independent values
// gain/toff/low gain values are not time dependent
        if( iFirst )
        {
            if( (!iLowGain && getRunParameter()->fGainFileNumber[getTelID()] > 0) || (iLowGain && getRunParameter()->fGainLowGainFileNumber[getTelID()] > 0 ) )
            {
                setDead( i, getDeadChannelFinder( iLowGain && getLowGainGains() )->testGains( i, getGains( iLowGain )[i] ), iLowGain );
                setDead( i, getDeadChannelFinder( iLowGain && getLowGainGains() )->testGainVariations( i, getGainvars( iLowGain && getLowGainGains() )[i] ), iLowGain );
                setDead( i, getDeadChannelFinder( iLowGain && getLowGainGains() )->testGainDev( i, getGains( iLowGain )[i], getGainvars( iLowGain && getLowGainGains() )[i] , getGains_DefaultValue( iLowGain )[i] ), iLowGain );
            }
            if( (!iLowGain && getRunParameter()->fTOffFileNumber[getTelID()] > 0) || (iLowGain && getRunParameter()->fTOffLowGainFileNumber[getTelID()] > 0 ) )
            {
                setDead( i, getDeadChannelFinder( iLowGain && getLowGainTOff() )->testTimeOffsets( i, getTOffsets( iLowGain )[i] ), iLowGain );
            }
// test low gain multiplier
// (this actually do not set anything dead, it sets the mean value low gain multiplier for these channels)
            if( iLowGain )
            {
                if( getDeadChannelFinder( iLowGain )->testGainMultiplier( i, getLowGainMultiplier()[i], getMeanLowGainMultiplier(), getRMSLowGainMultiplier() ) )
                {
                    getLowGainMultiplier()[i] = getMeanLowGainMultiplier();
                    getLowGainMultiplierError()[i] = 0.;
                }
                if( fRunPar->fDoublePass )
                {
                    if( getDeadChannelFinder( iLowGain )->testGainMultiplier( i, getLowGainMultiplier( true )[i], getMeanLowGainMultiplier( true ), getRMSLowGainMultiplier( true ) ) )
                    {
                        getLowGainMultiplier( true )[i] = getMeanLowGainMultiplier( true );
                        getLowGainMultiplierError( true )[i] = 0.;
                    }
                }
            }

// test pixel status (from pix file in calibration directory)
            if( getChannelStatus()[i] <= 0 ) setDead( i, 11, iLowGain );
        }

/////////////////////////////////////////////////////
// set channels dead from .cfg file
        if( iFirst && fReader->isMC() )
        {
// getFullAnaVec()[i]: -1: dead channel, 0: channel does not exist, 1 channel o.k.
            if( fReader->getDataFormatNum() == 1 && getNChannels() >= fReader->getFullAnaVec().size() )
            {
                if( fReader->getFullAnaVec()[i] == 0 ) setDead( i, 12, iLowGain );
                if( !fRunPar->fMCnoDead && fReader->getFullAnaVec()[i] == -1 ) setDead( i, 12, iLowGain );
            }
            else if( getNChannels() >= getDetectorGeometry()->getAnaPixel().size() )
            {
                if( getDetectorGeometry()->getAnaPixel()[i] < 1 ) setDead( i, 12, iLowGain );
            }
        }
    }
/////////////////////////////////////////////////////

// check number of dead channels, print warning for more than 30 dead channels
    unsigned int n_dead = 0;
    for( unsigned int i = 0; i < getDead( iLowGain ).size(); i++ )
    {
       if( getDead( iLowGain )[i] )
       {
          n_dead++;
       }
    }
    setNDead( n_dead, iLowGain );

    if( iFirst && getNDead( iLowGain ) > 30 )
    {
        cout << "WARNING: number of dead";
        if( !iLowGain ) cout << " high gain ";
        else            cout << " low gain ";
        cout << "channels on telescope " << getTelID()+1 << " exceeds 30: " << n_dead << endl;

// do not allow to run eventdisplay with all channels dead in the low channels one of the telescopes
        if( iLowGain && getNDead( iLowGain ) == getNChannels() )
        {
            cout << "Error: Number of dead channel is comparable to total number of channels" << endl;
            cout << "Exiting..." << endl;
            exit( 0 );
        }
    }
// set random dead channels in camera (unless it's a GrISU run, in which case
// setting dead here might be double-dipping.  Require fMCNdead > 4 because
// there will be 4 dead pixels at minimum in a real camera due to the L2
// triggers used to correct timing jitter on the FADC crates.
    if( iFirst && fRunPar->fMCNdead > 4 && fRunPar->fsourcetype != 4 && fRunPar->fsourcetype != 6 )
    {
        int iN = 0;
        unsigned int iPix = 0;
        int iNstep = 0;
        cout << "Telescope " << getTelID()+1 << ": setting randomly channels dead (seed " << getAnaData()->getRandomDeadChannelSeed() << ")" << endl;
        do
        {
            iPix = getAnaData()->getRandomDeadChannel();
// check if this one is alread dead
            if( !(getDead( iLowGain ).at( iPix )) )
            {
                setDead( iPix, 10, iLowGain );
                iN++;
            }
            iNstep++;
// avoid too large loop
            if( iNstep > 10000 )
            {
                cout << "VImageBaseAnalyzer::findDeadChans: too many dead "
                    << "channels, break setting of random dead "
                    << "channels at " << iN << endl;
                break;
            }
        } while ( iN < fRunPar->fMCNdead-4 );
    }
}


void VImageBaseAnalyzer::timingCorrect()
{
// apply timing correction
    unsigned int nc = getTZeros().size();
    if( nc == getTOffsets().size() )
    {
       for( unsigned int i = 0; i < nc; i++ )
       {
	  if( !getDead()[i] ) setPulseTimingCorrection( i, -1. * getTOffsets()[i] );
       }
    }
}


int VImageBaseAnalyzer::fillHiLo()
{
// reset all channels
    setHiLo( false );
    int z = 0;
    unsigned int chID = 0;
    unsigned int nchannel = getReader()->getNumChannelsHit();
    unsigned int nhilo = getHiLo().size();

    for( unsigned int i = 0; i < nchannel; i++ )
    {
        if( getReader()->getHiLo(i) != 0 )
        {
            getReader()->selectHitChan(i);
            try
            {
                chID = fReader->getHitID(i);
            }
            catch(...)
            {
                if( getDebugLevel() == 0 )
                {
                    cout << "VImageBaseAnalyzer::fillHiLo(), index out of range (fReader->getHitID) " << i;
		    cout << "(Telescope " << getTelID()+1 << ", event " << getEventNumber() << ")" << endl;
// flag this event
                    getAnalysisTelescopeEventStatus()[getTelID()] = 1;
                    setDebugLevel( 1 );
                }
                continue;
            }
            if( chID >= nhilo ) continue;
            setHiLo( chID, true );
            if( !getDead( true )[chID] && !(fReader->getMaxChannels() == 500 && chID == 499 ) ) z++;
        }
    }
    return z;
}

int VImageBaseAnalyzer::fillSaturatedChannels()
{
   int z = 0;
   for( unsigned int i = 0; i < getTraceN255().size(); i++ )
   {
      if( getTraceN255()[i] > 0 ) z++;
   }
   return z;
}


TTree* VImageBaseAnalyzer::makeDeadChannelTree()
{
    char hname[200];
    char htitle[200];
    sprintf( hname, "tchannel" );
    sprintf( htitle, "channel state (Telescope %d)", getTelID()+1 );
    TTree *itc = new TTree( hname, htitle );
    int ipix;
    int istat;
    int istatLow;
    itc->Branch( "channel", &ipix, "channel/I" );
    itc->Branch( "state", &istat, "state/I" );
    itc->Branch( "stateLow", &istatLow, "stateLow/I" );

    for( unsigned int i = 0; i < getDead().size(); i++ )
    {
        ipix = (int)i;
        istat = (int)getDead()[i];
        if( i < getDead( true ).size() ) istatLow = (int)getDead(true)[i];
        itc->Fill();
    }

    return itc;
}


/*
   get the maximum value of the photo diode trace
   (used in dst only)
*/
float VImageBaseAnalyzer::getPhotoDiodeMax()
{
    const double i_photoDiodePed = 17.8;

    if( fReader->getMaxChannels() >= 499 )
    {
        fReader->selectHitChan(499);
// hard coded pedestal -> this should not change during a run
        if( fReader->has16Bit() ) fTraceHandler->setTrace( fReader->getSamplesVec16Bit(), i_photoDiodePed, 1., 499 );
	else                      fTraceHandler->setTrace( fReader->getSamplesVec(), i_photoDiodePed, 1., 499 );
	fTraceHandler->setTraceIntegrationmethod( getTraceIntegrationMethod() );
        if( !getReader()->getHiLo( 499 ) ) return     fTraceHandler->getTraceMax();
        else                               return -1.*fTraceHandler->getTraceMax();
    }
    return 0.;
}


/*
   get the charge value of the photo diode trace
   (used in dst only)
*/
float VImageBaseAnalyzer::getPhotoDiodeSum()
{
    const int i_traceSumStart = 7;
    const double i_photoDiodePed = 17.8;

    if( fReader->getMaxChannels() >= 499 )
    {
        fReader->selectHitChan(499);
// hard coded pedestal -> this should not change during a run
        if( fReader->has16Bit() ) fTraceHandler->setTrace( fReader->getSamplesVec16Bit(), i_photoDiodePed, 1., 499 );
	else                      fTraceHandler->setTrace( fReader->getSamplesVec(), i_photoDiodePed, 1., 499 );
	fTraceHandler->setTraceIntegrationmethod( getTraceIntegrationMethod() );
        if( !getReader()->getHiLo( 499 ) ) return       fTraceHandler->getTraceSum( i_traceSumStart, i_traceSumStart + getSumWindow(), false );
        else                                            return -1. * fTraceHandler->getTraceSum( i_traceSumStart, i_traceSumStart + getSumWindow(), false );
    }
    return 0.;
}


/*

   FADC integration: second pass for double pass method

   use time gradient of first image to determine window placement here

*/
void VImageBaseAnalyzer::calcSecondTZerosSums()
{
    if( fDebug ) cout << "VImageBaseAnalyzer::calcSecondTZerosSums()" << endl;

// get number of channels
    unsigned int nhits = fReader->getNumChannelsHit();
// exclude photodiode from this
    if( nhits > getDead(false).size() ) nhits = getDead(false).size();

// set integration window
    unsigned int iSumWindow = getSumWindowSmall();
    setTCorrectedSumFirst( getSumFirst() );
// set dynamic integration window
// (depending on the measured integrated charge in first pass)
    for( unsigned int i = 0; i < nhits; i++ )
    {
        unsigned int i_channelHitID = 0;
        try
        {
            i_channelHitID = fReader->getHitID(i);
            if( i_channelHitID < getHiLo().size() && !getDead(getHiLo()[i_channelHitID])[i_channelHitID] )
            {
		if( getRunParameter()->fDynamicIntegrationWindow ) iSumWindow = getDynamicSummationWindow( i_channelHitID );
                int corrlast = getSumFirst() + iSumWindow;
                if( corrlast>(int)getNSamples() ) corrlast = (int)getNSamples();
		setTCorrectedSumLast( i_channelHitID, corrlast );
		setCurrentSummationWindow( i_channelHitID, getSumFirst(), corrlast );
            }
        }
        catch(...)
        {
            if( getDebugLevel() == 0 )
            {
                cout << "VAnalyzer::calcSecondTZerosSums(), dynamical summation window, index out of range (fReader->getHitID) ";
		cout << i << "(Telescope " << getTelID()+1 << ", event " << getEventNumber() << ")" << endl;
                setDebugLevel( 0 );
            }
            continue;
        }
    }

// reinitialize arrays
    setSums( 0. );
    setSums2( 0. );
    setImage( false );
    setBorder( false );
    setBrightNonImage( false );
    setImageBorderNeighbour( false );

    float xpmt = 0.;
    float ypmt = 0.;
    float xpos = 0;
    float xtime = 0.;
    int corrfirst = 0;

    for(unsigned int i = 0; i < nhits; i++ )
    {
        unsigned int i_channelHitID = 0;
        try
        {
            i_channelHitID = fReader->getHitID(i);
            if( i_channelHitID < getHiLo().size() && !getDead(getHiLo()[i_channelHitID])[i_channelHitID] )
            {
// trace handler
                fTraceHandler->setTrace( fReader, getNSamples(),getPeds(getHiLo()[i_channelHitID])[i_channelHitID], 
		                                  getPedrms(getHiLo()[i_channelHitID])[i_channelHitID], i_channelHitID, i,
						  getLowGainMultiplier( true )[i_channelHitID]*getHiLo()[i_channelHitID] );
                fTraceHandler->setTraceIntegrationmethod( getTraceIntegrationMethod() );

// position of the PMT in camera coordinates
                xpmt = getDetectorGeo()->getX()[i_channelHitID] - getImageParameters()->cen_x;
                ypmt = getDetectorGeo()->getY()[i_channelHitID] - getImageParameters()->cen_y;
// position along the major axis of the image
                xpos = xpmt*getImageParameters()->cosphi + ypmt*getImageParameters()->sinphi;
// fit time along the major axis of the image
                if (abs(getImageParameters()->tgrad_x)<200) xtime = getImageParameters()->tgrad_x*xpos+getImageParameters()->tint_x;
                else xtime = getSumFirst();

///////////////////////////////////
// calculate start of integration

// fit times are corrected for TOffsets and FADCStopOffsets.
// undo this to get integration window in FADC trace.
                corrfirst = (int)(xtime + getTOffsets()[i_channelHitID] - getFADCStopOffsets()[i_channelHitID]);
// (GM) low gain channel have different time -> use tzero
                if( getHiLo()[i_channelHitID] || getTraceMax()[i_channelHitID] > getRunParameter()->fSumWindowStartAtT0Min )
                {
// get new tzero for sumwindow starting at corrfirst to the end of the window
		    float iT0 = fTraceHandler->getPulseTiming( corrfirst, getNSamples(), 0, getNSamples() )[getRunParameter()->fpulsetiming_tzero_index];
                    if( corrfirst - iT0 < getDBSumWindowMaxTimedifference() )
                    {
                        corrfirst = TMath::Nint( iT0 ) + getSumWindowShift();
                    }
                }
// integration start might be before sample 0 -> set to sample 0
                if (corrfirst < 0 )
                {
                    unsigned int isw = getSumWindowSmall();
                    if( -1*corrfirst > (int)isw ) isw = 0;
                    else                          isw += corrfirst;
                    corrfirst = 0;
                    setCurrentSummationWindow( i_channelHitID, corrfirst, isw );
                }
   	        else if( corrfirst == 0 )
		{
// important for low energies: pixels are cleaned away in the first pass, but bright with the shorter summation window
// below a very preliminary fix; should be treated properly
		    if( getImageParameters()->tgrad_x < 1.e-2 && getImageParameters()->tint_x < 1.e-2 )
		    {
		        corrfirst = getSumFirst() + 3;
		    }
                }  
// integration start is at beyond last sample: set to last sample
                if (corrfirst> (int)getNSamples()) corrfirst=(int)getNSamples();
// set start of integration window
                setTCorrectedSumFirst( i_channelHitID, corrfirst );

///////////////////////////////////
// calculate end of integration window
                int corrlast = corrfirst + getCurrentSumWindow()[i_channelHitID];
                if (corrlast>(int)getNSamples()) corrlast=(int)getNSamples();
                setTCorrectedSumLast( i_channelHitID, corrlast );
// set current summation window
                setCurrentSummationWindow( i_channelHitID, corrfirst, corrlast );

///////////////////////////////////
// calculate pulse sum
                setSums( i_channelHitID, fTraceHandler->getTraceSum(corrfirst, corrlast, fRaw ) );
                if( getFillPulseSum() ) getAnaData()->fillPulseSum( i_channelHitID, getSums()[i_channelHitID], getHiLo()[i_channelHitID] );
// calculate sum for second summation window
                setSums2( i_channelHitID, fTraceHandler->getTraceSum(corrfirst, corrfirst+(int)getSumWindow_2(), fRaw ) );
            }
        }
        catch(...)
        {
            if( getDebugLevel() == 0 )
            {
                cout << "VAnalyzer::calcSecondTZerosSums(), index out of range (fReader->getHitID) ";
		cout << i << "(Telescope " << getTelID()+1 << ", event " << getEventNumber() << ")" << endl;
                setDebugLevel( 0 );
            }
            continue;
        }
    }
}

/*

    determine length of integration window depending on pulse size of first integration pass

    TODOTODOTODO  not implemented yet TODOTODOTODOTODO

*/
unsigned int VImageBaseAnalyzer::getDynamicSummationWindow( unsigned int i_channelHitID )
{
// low gain: return always largest value
   if( getHiLo()[i_channelHitID] ) 
   {
      return 12;
   }
// for integrated pulses below a certain threshold: return smallest dynamical window
   if( getSums()[i_channelHitID] < 0 ) return getSumWindowSmall();

// calculate integration window

   return getSumWindowSmall();
}
