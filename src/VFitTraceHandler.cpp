/*! \class VFitTraceHandler
    \brief calculation of trace specific stuff with a fit function

     Fit function is VFitTraceHandler_tracefunction()

    \todo
       - sum window should be adjusted according to the pulse width,
         this requires a lot of changes in VAnalyser, VGrIsuReader and
         storage of pedestal variances in VCalibrationData
- make it faster (just now, it's a factor 90 slower than simple
trace summing)
*/

#include "VFitTraceHandler.h"

/*! \fn VFitTraceHandler_tracefunction(double *x, double *p)
    \brief trace fitting function

    For \f$x < \bar x\f$:

    \f[
       f = A * \exp(-0.5*(x-\bar x)^2/\sigma^2) + pedestal
    \f]

    else

\f[
f = A * \exp(-0.5*x^2/(\sigma^2+\alpha \cdot (x-\bar x) ) + pedestal
\f]

*/
double VFitTraceHandler_tracefunction(double *x, double *p)
{
    double A = p[0];
    double xbar = p[1];
    double sig = p[2];
    double alp = p[3];
    double ped = p[4];
    double xd = x[0] - xbar;
    double f = 0.;

    if (x[0] < xbar)
    {
        f = A * exp(-0.5 * xd*xd/sig/sig) + ped;
    }
    else
    {
        f = A * exp(-0.5 * xd*xd / (sig*sig + (alp*xd))) + ped;
    }
    return f;
}


/*! \fn VFitTraceHandler_tracefunction_Grisu(double *x, double *p)
    \brief trace fitting function from Grisu

*/
double VFitTraceHandler_tracefunction_Grisu(double *x, double *par )
{
    double t = x[0];
    double rize = par[0];
    double fall = par[1];
    double rctim = par[2];
    double tstart = par[3];
    double norm = par[4];
    double ped = par[5];

    double rtn;
    double alpha;                                 /*Power index in the pulse shape definition*/
    double wid = 0.;                              /*Photo electron pulse width*/
    double renorm;                                /*Photo electron pulse normalisation factor*/

    t -= tstart;
/*In case it is earlier than the p.e. arrival we return 0*/
    if (t<0.) return ped;

/*The first component is the single p.e.pulse*/
    if(t<(fall+rize))
    {
        wid=fall+rize;
        alpha=wid/rize-1.0;
        renorm=pow(wid,alpha+2)/((alpha+1)*(alpha+2));
        rtn=t*pow(wid-t,alpha)/renorm;
        rtn *= norm;
        rtn += ped;
        return rtn;
    }
/*The second component corresponds to the AC coupling over-shoot that
  guaranties the full signal integral to be null*/
    if(rctim>0.0) rtn=-exp(-(t-wid)/rctim)/rctim;
    else rtn=0.0;

    rtn *= norm;
    rtn += ped;
    return rtn;
}


/*!
    Fit function is VFitTraceHandler_tracefunction()

    \attention

    assume 64 samples per trace, if more, change fMaxSamples.
    Less doesn't matter.
*/
VFitTraceHandler::VFitTraceHandler( string iFit )
{
    fMinuitPrint = false;
    fMaxSamples = 64;
    fpTrace.reserve( fMaxSamples );
    fpTrazeSize = 0;

    fFitFunction = iFit;
    if( !setFitFunction( fFitFunction ) )
    {
        cout << "VFitTraceHandler::VFitTraceHandler: failed defining fit functions, exiting..." << endl;
        exit( -1 );
    }
    fH1TraceData = new TH1D( "fH1TraceHandlerData", "", (int)fMaxSamples, 0., fMaxSamples );
// plotting histogram
    fH1Trace = new TH1D( "fH1TraceHandler", "", fF1Trace->GetNpx(), 0., fMaxSamples );
// diagnostic histograms (chi2, fitstat)
    fHchi2 = new TH1D( "fHchi2", "Tracefit: chi2", 100, 0., 10. );
    fHchi2->SetXTitle( "chi2" );
    fHfitstat = new TH1D( "fHfitstat", "Tracefit: status of error matrix", 4, 0., 4. );
    fHfitstat->SetXTitle( "status of error matrix" );
    fHxbar = new TH2D( "fHFitxbar", "Tracefit: xbar vs. tube number", 499, 0., 499., 128, 0., 46. );
    fHxbar->SetXTitle( "tube number" );
    fHxbar->SetYTitle( "xbar" );
    fHsigma = new TH2D( "fHFitsigma", "Tracefit: #sigma vs. tube number", 499, 0., 499., 100, 0., 10. );
    fHsigma->SetXTitle( "tube number" );
    fHsigma->SetYTitle( "#sigma" );
    fHalpha = new TH2D( "fHFitalpha", "Tracefit: #alpha vs. tube number", 499, 0., 499., 100, 0., 10. );
    fHalpha->SetXTitle( "tube number" );
    fHalpha->SetYTitle( "#alpha" );

    fChi2 = 0.;
    fRT = 0.;
    fFT = 0.;
    fTraceNorm = 0.;

    fMC_FADCTraceStart = 0;
}


VFitTraceHandler::~VFitTraceHandler()
{
    delete fF1Trace;
    delete fF1TraceGr;
    delete fH1Trace;
    delete fH1TraceData;
    delete fHxbar;
    delete fHsigma;
    delete fHalpha;
}


void VFitTraceHandler::setTrace( VVirtualDataReader* iReader, unsigned int iNSamples, double ped, double pedrms, unsigned int chanID )
{
    setTrace( iReader, iNSamples, ped, pedrms, chanID );
}


void VFitTraceHandler::setTrace( VVirtualDataReader* iReader, unsigned int iNSamples, double ped, double pedrms, unsigned int iChanID, double iHiLo )
{
    fPed = ped;
    fPedrms = pedrms;

    if( !iReader )
    {
        cout << "VFitTraceHandler::setTrace: no reader set" << endl;
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

    fitTrace( iChanID );

}


void VFitTraceHandler::setTrace(vector<uint8_t> pTrace, double ped, double pedrms, unsigned int chanID )
{
    setTrace( pTrace, ped, pedrms, chanID, false );
}


/*!
     set the trace values and fit the peak

*/
void VFitTraceHandler::setTrace(vector<uint8_t> pTrace, double ped, double pedrms, unsigned int chanID, double iHiLo )
{
    fPed = ped;
    fPedrms = pedrms;

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

    fitTrace( chanID );
}


void VFitTraceHandler::fitTrace( unsigned int chanID )
{

    fFitted = false;
    fChi2 = 0.;
    fRT = 0.;
    fFT = 0.;
    fTraceNorm = 0.;

    fH1TraceData->Reset();
    fH1Trace->SetLineStyle( 1 );
// check if histogram size is still correct
    if( (int)fpTrace.size() != (int)fH1TraceData->GetNbinsX() ) fH1TraceData->SetBins( (int)fpTrace.size(), 0.,(double)fpTrace.size() );
// fill histogram
    for( unsigned int i = 0; i < fpTrace.size(); i++ )
    {
        fH1TraceData->SetBinContent( i+1, -1. * fpTrace[i] );
// errors are signal+rms of pedestal
        fH1TraceData->SetBinError( i+1, sqrt(fabs(fpTrace[i]-fPed) + fPedrms*fPedrms)/2. );
    }

// find start parameters
    double ipeak = 0.;
    int ipeakpos = 0;
    getQuickMax( 0, fpTrace.size(), ipeak, ipeakpos );
    int ipeakStartX = 0;
    ipeakStartX = (int)getQuickTZero( 0, fpTrace.size() );

// fit only if peak value of trace is above threshold
    if( ipeak > fFitThresh * fPedrms )
    {
        if( fFitFunction == "ev" )
        {
// fit in samples
            fF1Trace->SetParameters( -1.*ipeak, ipeakpos, 0.6, 1.6, -1.*fPed );
// fit in ns
//	 fF1Trace->SetParameters( -1.*ipeak, ipeakpos*2, 1.2, 3.0, -1.*fPed );
            fF1Trace->SetParNames("Constant","Mean","Sigma","Alpha","Pedestal");
            fF1Trace->FixParameter( 4, -1.*fPed );
// constant always negative
            fF1Trace->SetParLimits( 0, -5.e5, 0. );
// fit in samples
            fF1Trace->SetParLimits( 2, 0.01, 20. );
// fit in ns
//	 fF1Trace->SetParLimits( 2, 0.1, 20. );
            fF1Trace->SetParLimits( 3, 0., 20. );
        }
        else if( fFitFunction == "grisu" )
        {
// fit in samples
            fF1Trace->SetParameters( 2.4, 8.0, 0., ipeakpos, -1.*ipeak,  -1.*fPed );
// fit in ns
//         fF1Trace->SetParameters( 2.4, 8.0, 0., 2.*ipeakpos, -1.*ipeak,  -1.*fPed );
            fF1Trace->SetParNames( "RT", "FT", "RC", "T0", "Constant", "Pedestal" );
            fF1Trace->FixParameter( 2, 0. );
            fF1Trace->SetParLimits( 0, 0., 20. );
            fF1Trace->SetParLimits( 1, 0., 2000. );
        }

// now fit everything
        if( !fMinuitPrint )
        {
//         fH1TraceData->Fit( fF1Trace, "0Q" );
            fH1TraceData->Fit( fF1Trace, "E" );
        }
        else                fH1TraceData->Fit( fF1Trace, "E" );
// pulse maximum
        fTraceMax =  fF1Trace->GetMinimum( (double)0., (double)fMaxSamples );
        fTraceMaxX = fF1Trace->GetMinimumX( (double)0., (double)fMaxSamples );
// status of the error matrix
        double edm;
        double amin;
        double errdef;
        int nvpar;
        int nparx;
        gMinuit->mnstat( amin, edm, errdef, nvpar, nparx, fnstat );
        if( fnstat < 3 ) fH1Trace->SetLineStyle( 2 );
        fHfitstat->Fill( fnstat );
// chi2
        fChi2 = fF1Trace->GetChisquare() / (fH1TraceData->GetNbinsX() - fF1Trace->GetNumberFreeParameters() );
        fHchi2->Fill( fChi2 );
// parameters
        if( fnstat > 0 )
        {
            fHxbar->Fill( (double)chanID, fF1Trace->GetParameter( 1 ) );
            fHsigma->Fill( (double)chanID, fF1Trace->GetParameter( 2 ) );
            fHalpha->Fill( (double)chanID, fF1Trace->GetParameter( 3 ) );
            if( fFitFunction == "ev" )
            {
                fRT = fF1Trace->GetParameter( 2 );
                fFT = fF1Trace->GetParameter( 3 );
                fTraceNorm = fF1Trace->GetParameter( 0 );
            }
            else if( fFitFunction == "grisu" )
            {
                fRT = fF1Trace->GetParameter( 0 );
                fFT = fF1Trace->GetParameter( 1 ) ;
                fTraceNorm = fF1Trace->GetParameter( 4 );
            }
        }
        fFitted = true;
    }
    else
    {
        fFitted = false;
        fChi2 = 0.;
        fFT = 0.;
        fRT = 0.;
        fTraceNorm = 0.;
    }
}


/*!
    for successful fits return integral, otherwise quicksum

*/
double VFitTraceHandler::getTraceSum( int iFirst, int iLast, bool iRaw )
{
    double isum = 0.;
    if( fnstat > 0 && fFitted )
    {
        isum = -1. * fF1Trace->Integral( iFirst, iLast );
        if( !iRaw ) isum -= fPed * ( iLast - iFirst );
    }
    else isum = getQuickSum( iFirst, iLast, iRaw );

    return isum;
}


void VFitTraceHandler::getTraceMax( int iFirst, int iLast, double &max, int &maxpos )
{
    if( fFitted )
    {
        max = -1.*fF1Trace->GetMinimum( (double)iFirst, (double)iLast ) - fPed;
        maxpos = (int)fF1Trace->GetMinimumX( (double)iFirst, (double)iLast );
    }
    else getQuickMax( iFirst, iLast, max, maxpos );
}


void VFitTraceHandler::getTraceMax( double &max, double &maxpos )
{
    max = -1.*fTraceMax;
    max -= fPed;
    maxpos = fTraceMaxX;
}


double VFitTraceHandler::getTraceTZero( int iFirst, int iLast )
{
    double imax = 0.;
    int maxpos = 0;
    if( !fFitted ) return getQuickTZero( iFirst, iLast );
    getTraceMax( iFirst, iLast, imax, maxpos );
    return fF1Trace->GetX( -1.* (imax/2+fPed), 0., (double)maxpos );
}


TH1D *VFitTraceHandler::getFitHis()
{
    if( !fFitted ) return 0;
    double ieval;
    for( int i = 1; i <= fH1Trace->GetNbinsX(); i++ )
    {
        ieval = fF1Trace->Eval( fH1Trace->GetBinCenter( i ) );
        if( !finite( (double)ieval ) ) ieval = fF1Trace->Eval( fH1Trace->GetBinCenter( i - 1 ) );
        fH1Trace->SetBinContent( i, ieval );
    }
    return fH1Trace;
// preli! Want to see the error bars
//   return fH1TraceData;
}


void VFitTraceHandler::terminate()
{
    if( gDirectory )
    {
        fHchi2->Write();
        fHfitstat->Write();
        fHxbar->Write();
        fHsigma->Write();
        fHalpha->Write();
    }
}


bool VFitTraceHandler::setFitFunction( string iFunc )
{
    if( iFunc != "ev" && iFunc != "grisu" )
    {
        return false;
    }

    if( iFunc == "ev" )
    {
        fF1Trace = new TF1( "fF1tracehandler", VFitTraceHandler_tracefunction, 0., fMaxSamples, 5 );
        fF1Trace->SetLineColor( 1 );
    }
    else if( iFunc == "grisu" )
    {
        fF1Trace = new TF1( "fF1tracehandler", VFitTraceHandler_tracefunction_Grisu, 0., fMaxSamples, 6 );
        fF1Trace->SetLineColor( 2 );
    }
    fF1Trace->SetTitle( iFunc.c_str() );
    fF1Trace->SetLineWidth( 2 );
    fF1Trace->SetNpx( 500 );
    return true;
}


double VFitTraceHandler::getTraceWidth( int fFirst, int fLast, double fPed )
{
    if( !fF1Trace ) return -1.;

    double iMax = fTraceMax;
    iMax *= -1.;
    iMax = fPed + 0.5*(iMax-fPed);
    iMax *= -1.;

    return ( fF1Trace->GetX( iMax, fTraceMaxX, (double)fMaxSamples) - fF1Trace->GetX( iMax, 0., fTraceMaxX));
}


double VFitTraceHandler::getTraceRiseTime( double fPed, double ystart, double ystop )
{
    if( !fF1Trace ) return -1.;
    if( ystart < 0. || ystart > 1. ) return -1.;
    if( ystop < 0. || ystop > 1. ) return -1.;
    double t1 = 0.;
    double t2 = 0.;

    double iMax = fTraceMax;
    t1 = fF1Trace->GetX(  -1.*(fPed + ystart*(-1.*iMax-fPed)), 0., fTraceMaxX );
    t2 = fF1Trace->GetX(  -1.*(fPed + ystop*(-1.*iMax-fPed)), 0., fTraceMaxX );

    return t2-t1;
}


double VFitTraceHandler::getTraceFallTime( double fPed, double ystart, double ystop )
{
    if( !fF1Trace ) return -1.;
    if( ystart < 0. || ystart > 1. ) return -1.;
    if( ystop < 0. || ystop > 1. ) return -1.;
    double t1 = 0.;
    double t2 = 0.;

    double iMax = fTraceMax;
    t1 = fF1Trace->GetX(  -1.*(fPed + ystart*(-1.*iMax-fPed)), fTraceMaxX, (double)fMaxSamples );
    t2 = fF1Trace->GetX(  -1.*(fPed + ystop*(-1.*iMax-fPed)), fTraceMaxX, (double)fMaxSamples );

    return t2-t1;
}
