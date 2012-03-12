/*! \class VImageParameterCalculation
    \brief calculation of the Hillas image parameters

    \todo delete fParGeo before setParameters(...) ??

    dead channels for low gains and LL

    Revision $Id: VImageParameterCalculation.cpp,v 1.28.2.1.2.1.2.4.4.1.4.2.2.3.4.3.4.1.2.3.2.1.4.10.2.5 2010/10/26 00:06:15 gmaier Exp $

    \author
       Jamie Holder
Gernot Maier
*/

#include <VImageParameterCalculation.h>

VImageParameterCalculation::VImageParameterCalculation( unsigned int iShortTree, VEvndispData *iData )
{
    fDebug = false;
    fLLDebug = false;
    if( fDebug ) cout << "VImageParameterCalculation::VImageParameterCalculation()" << endl;
    fData = iData;
    fParGeo = new VImageParameter( iShortTree );
    fParLL =  new VImageParameter( iShortTree );
    fboolCalcGeo = false;
    fboolCalcTiming = false;
    fDetectorGeometry = 0;

}


VImageParameterCalculation::~VImageParameterCalculation()
{
    if( fDebug ) cout << "VImageParameterCalculation::~VImageParameterCalculation()" << endl;
    delete fParGeo;
    delete fParLL;
    delete fLLFitter;
}


/*!
   \param iVmode Minuit print mode, 1 = quit, 2 - verbose

*/
void VImageParameterCalculation::initMinuit( int iVmode )
{
    if( iVmode == 2 ) fLLDebug = true;
    fLLFitter = new TMinuit( 6 );
                                                  // no minuit printouts
    if( iVmode == 1 )      fLLFitter->Command( "SET PRINT -1" );
                                                  // minuit printouts
    else if( iVmode == 2 ) fLLFitter->Command( "SET PRINT 1" );
    fLLFitter->Command( "SET NOWA" );             // no warnings
    fLLFitter->Command( "SET ERR 0.5" );          // loglikelihood -> UP = 0.5  (solves 2*ll = chi2 != 1)
    fLLFitter->SetObjectFit( this );
    fLLFitter->SetFCN( get_LL_imageParameter_2DGauss );
}


void VImageParameterCalculation::calcTimingParameters(valarray<double> fTZeros, valarray<double> fTOffsetvars, valarray<double> fSums, vector<bool> fImage, vector<bool> fBorder, VEvndispData *iData )
{
    if( fDebug ) cout << "VImageParameterCalculation::calcTimingParameters" << endl;
// calculate timing parameters
    if (!fboolCalcGeo) return;
    if(fTZeros.size()==0)return;

    unsigned int num=fTZeros.size();
    double xpos[num], ypos[num], rpos[num], t[num];
    double ex[num], ey[num], er[num], et[num];
    int nclean=0;
    for( unsigned int i = 0; i < fTZeros.size(); i++ )
    {
        if( (fImage[i] || fBorder[i]) && fTZeros[i]>0. && !iData->getHiLo()[i] )
        {
            double xi = getDetectorGeo()->getX()[i];
            double yi = getDetectorGeo()->getY()[i];
// loop over image tubes
            double xpmt= xi - fParGeo->cen_x;
            double ypmt= yi - fParGeo->cen_y;
//position along the major axis of the image
            xpos[nclean]=xpmt*fParGeo->cosphi+
                ypmt*fParGeo->sinphi;
            ex[nclean]=0;                         // error on xpos (deg)
//position along the minor axis of the image
            ypos[nclean]=ypmt*fParGeo->cosphi-
                xpmt*fParGeo->sinphi;
            ey[nclean]=0;                         // error on ypos (deg)
//radial position from camera centre
            rpos[nclean]=sqrt(yi*yi + xi*xi );
            er[nclean]=0;                         // error on rpos (deg)
            t[nclean]=fTZeros[i];

//  timing resolution from variable laser pulse studies (run 751)
            et[nclean]=13.0*exp(-0.035*(fSums[i]+30.))+fTOffsetvars[i];
// make that the timing resolution is not too small (important for MC)
            if( et[nclean] < 5.e-2 ) et[nclean] = 0.3;
// min/max/mean times
            if( fTZeros[i]  < fParGeo->tmin ) fParGeo->tmin = fTZeros[i];
            if( fTZeros[i]  > fParGeo->tmax ) fParGeo->tmax = fTZeros[i];
            fParGeo->tmean += fTZeros[i];

            nclean+=1;
        }
    }

    TGraphErrors *xgraph = iData->getXGraph();
    if( xgraph )
    {
        xgraph->Set( nclean );

        if( nclean > 2 )
        {
// Fill the graphs for long (x) short(y) and radial (r) axis
            for (int i=0;i<nclean;i++)
            {
                xgraph->SetPoint(i,xpos[i],t[i]);
                xgraph->SetPointError(i,ex[i],et[i]);
            }
            xgraph->Fit("pol1","Q");
            TF1 *xline=xgraph->GetFunction("pol1");

            fParGeo->tint_y=-999.;
            fParGeo->tgrad_y=-999.;
            fParGeo->tint_dy=-999.;
            fParGeo->tgrad_dy=-999.;
            fParGeo->tchisq_y=-999.;

            fParGeo->tint_x=xline->GetParameter(0);
            fParGeo->tgrad_x=xline->GetParameter(1);
            fParGeo->tint_dx=xline->GetParError(0);
            fParGeo->tgrad_dx=xline->GetParError(1);
            fParGeo->tchisq_x=xline->GetChisquare();
            fParGeo->tmean = fParGeo->tmean / nclean;
            fParGeo->tint_r=-999.;
            fParGeo->tgrad_r=-999.;
            fParGeo->tint_dr=-999.;
            fParGeo->tgrad_dr=-999.;
            fParGeo->tchisq_r=-999.;
            iData->setXGraph(xgraph);
        }
    }
    else
    {
        fParGeo->tint_x=-999.;
        fParGeo->tint_y=-999.;
        fParGeo->tint_r=-999.;
        fParGeo->tgrad_x=-999.;
        fParGeo->tgrad_y=-999.;
        fParGeo->tgrad_r=-999.;
        fParGeo->tint_dx=-999.;
        fParGeo->tint_dy=-999.;
        fParGeo->tint_dr=-999.;
        fParGeo->tgrad_dx=-999.;
        fParGeo->tgrad_dy=-999.;
        fParGeo->tgrad_dr=-999.;
        fParGeo->tchisq_x=-999.;
        fParGeo->tchisq_y=-999.;
        fParGeo->tchisq_r=-999.;
        fParGeo->tmin = -999.;
        fParGeo->tmax = -999.;
        fParGeo->tmean = -999.;
    }
    fboolCalcTiming=true;
}


/*****************************************************************************
muonRingFinder
input: pointer to pixels passing cleaning
output: return the likely location of x0, y0, and R with sigmaR
****************************************************************************
*/
void VImageParameterCalculation::muonRingFinder( valarray<double> fSums, vector<bool> fImage, vector<bool> fBorder  )
{
    if( fDebug ) cout << "VImageParameterCalculation::muonRingFinder" << endl;
    if( fSums.size()==0)
    {
        fParGeo->muonX0 = 0.0;
        fParGeo->muonY0 = 0.0;
        fParGeo->muonRadius = 0.0;
        fParGeo->muonRSigma = 0.0;
        return;
    }

    if( !getDetectorGeo() )
    {
        cout << "VImageParameterCalculation::muonRingFinder error: detector geometry not defined" << endl;
        exit( 0 );
    }

    int counter = 0, safty = 0, noChangeX = 0, noChangeY = 0;
    unsigned int i;
    double x0[2] =
    {
        0.0
    }
    , y0[2] =
    {
        0.0
    }
    , rBar[2] =
    {
        0.0
    }
    , rVariance[2] =
    {
        20.0
    };
    double rTotal = 0.0, rSquaredTotal = 0.0, tmp;
    double xi, yi;

//calculate r to each point in the filtered binary image & thereby mean of r
    for( i=0; i<fSums.size(); i++ )
    {
        if( fImage[i] || fBorder[i] )
        {
            counter++;
            xi = getDetectorGeo()->getX()[i];
            yi = getDetectorGeo()->getY()[i];
            tmp = sqrt( pow( (xi - x0[0]),2) + pow( (yi - y0[0]) ,2) );
            rTotal += tmp;
            rSquaredTotal += tmp*tmp;
        }
    }

    if( counter > 10 )
    {
        rVariance[0] = (rSquaredTotal-rTotal*rTotal/counter)/counter;
        rBar[0] = rTotal/counter;
        rBar[1]=20.0;                             //so we have a starting value to enter the loop
    }
    else
    {
        fParGeo->muonX0 = 0.0;
        fParGeo->muonY0 = 0.0;
        fParGeo->muonRadius = 0.0;
        fParGeo->muonRSigma = 0.0;
        return;
    }
///****************************************************
// LOOP through until good x0, y0 coordinates are found
///****************************************************
    while( ( (noChangeX == 0 && noChangeY == 0) || safty < 20 ) && safty < 100 )
    {
        safty++;
///*********************************************
// TAKE A STEP IN THE X-DRIRECTION ?
///*********************************************
//try a different x0[1], see if sigma of r decreases
        x0[1] = x0[0]+0.1/(1+pow(safty,.3));
        y0[1] = y0[0];
        noChangeX = 0;                            //reset the end switch

//calculate r to each point in the filtered binary image
        rTotal = 0;
        rSquaredTotal = 0;
        tmp = 0;
        for( i=0; i<fSums.size(); i++ )
        {
            if( fImage[i] || fBorder[i] )
            {
                xi = getDetectorGeo()->getX()[i];
                yi = getDetectorGeo()->getY()[i];
                tmp = sqrt( pow( (xi - x0[1]),2) + pow( (yi - y0[1]) ,2) );
                rTotal += tmp;
                rSquaredTotal += tmp*tmp;
            }
        }
        rVariance[1] = (rSquaredTotal-rTotal*rTotal/counter)/counter;
        rBar[1] = rTotal/counter;

//is rVariance[1] > rVariance[0] ? then try a step in the -x direction
        if( rVariance[1] > rVariance[0] )
        {
//	x0[1] =  x0[0]-0.03/(1+pow(safty,.3));       //try a different x0[1], see if mean of r decreases
            x0[1] =  x0[0]-0.05;                  //try a different x0[1], see if mean of r decreases
            rTotal = 0;
            rSquaredTotal = 0;
            tmp = 0;
            for( i=0; i<fSums.size(); i++ )
            {
                if( fImage[i] || fBorder[i] )
                {
                    xi = getDetectorGeo()->getX()[i];
                    yi = getDetectorGeo()->getY()[i];
                    tmp = sqrt( pow( (xi - x0[1]),2) + pow( (yi - y0[1]) ,2) );
                    rTotal += tmp;
                    rSquaredTotal += tmp*tmp;
                }
            }
            rVariance[1] = (rSquaredTotal-rTotal*rTotal/counter)/counter;
            rBar[1] = rTotal/counter;

//is rVariance[1] > rVariance[0] ? then keep original coordinate
            if( rVariance[1] > rVariance[0] )
            {
                x0[1] = x0[0];
                rBar[1] = rBar[0];
                rVariance[1] = rVariance[0];
                noChangeX = 1;
            }
        }

        rBar[0]=rBar[1];
        rVariance[0]=rVariance[1];                // why was this commented out??
        x0[0]=x0[1];

///*********************************************
// TAKE A STEP IN THE Y-DRIRECTION ?
///*********************************************
//try a different x0[1], see if sigma of r decreases
        x0[1] = x0[0];
        y0[1] = y0[0]+0.1/(1+pow(safty,.3));
        noChangeY = 0;                            //reset the end switch

//calculate r to each point in the filtered binary image
        rTotal = 0;
        rSquaredTotal = 0;
        tmp = 0;
        for( i=0; i<fSums.size(); i++ )
        {
            if( fImage[i] || fBorder[i] )
            {
                xi = getDetectorGeo()->getX()[i];
                yi = getDetectorGeo()->getY()[i];
                tmp = sqrt( pow( (xi - x0[1]),2) + pow( (yi - y0[1]) ,2) );
                rTotal += tmp;
                rSquaredTotal += tmp*tmp;
            }
        }
        rVariance[1] = (rSquaredTotal-rTotal*rTotal/counter)/counter;
        rBar[1] = rTotal/counter;

//is rVariance[1] > rVariance[0] ? then try a step in the -x direction
        if( rVariance[1] > rVariance[0] )
        {
//	y0[1] =  y0[0]-0.03/(1+pow(safty,.3));       //try a different x0[1], see if mean of r decreases
            y0[1] =  y0[0]-0.05;                  //try a different x0[1], see if mean of r decreases
            rTotal = 0;
            rSquaredTotal = 0;
            tmp = 0;
            for( i=0; i<fSums.size(); i++ )
            {
                if( fImage[i] || fBorder[i] )
                {
                    xi = getDetectorGeo()->getX()[i];
                    yi = getDetectorGeo()->getY()[i];
                    tmp = sqrt( pow( (xi - x0[1]),2) + pow( (yi - y0[1]) ,2) );
                    rTotal += tmp;
                    rSquaredTotal += tmp*tmp;
                }
            }
            rVariance[1] = (rSquaredTotal-rTotal*rTotal/counter)/counter;
            rBar[1] = rTotal/counter;

//is rVariance[1] > rVariance[0] ? then keep original coordinate
            if( rVariance[1] > rVariance[0] )
            {
                y0[1] = y0[0];
                rBar[1] = rBar[0];
                rVariance[1] = rVariance[0];
                noChangeY = 1;
            }
        }

        rBar[0]=rBar[1];
        rVariance[0]=rVariance[1];
        y0[0]=y0[1];

    }

    fParGeo->muonX0 = x0[1];
    fParGeo->muonY0 = y0[1];
    fParGeo->muonRadius = rBar[1];
    fParGeo->muonRSigma = sqrt( rVariance[1] );

//cout <<"x0 "<<fParGeo->muonX0<<",y0 "<<fParGeo->muonY0<<",R "<<fParGeo->muonRadius<<",RS "<<fParGeo->muonRSigma<<" steps: "<<safty<<endl;

}

/*****************************************************************************
sizeInMuonRing

input: pointer to  origion of masking ring, radius and sigmaR
output: sum of all pixel values in the ring +-2 sigmaR
notes: this needs to correct for the fraction of tubes off in the ring region
	   this is rather tricky because the intensity depends on the ring diamter and the blurring!
*****************************************************************************/
void VImageParameterCalculation::sizeInMuonRing( valarray<double> fSums, vector<bool> fImage, vector<bool> fBorder, vector< unsigned int > fDead  )
{
  if( fDebug ) cout << "VImageParameterCalculation::sizeInMuonRing" << endl;
  if( fParGeo->muonValid == 0 )
      {
      fParGeo->muonSize = 0.0;      
      return;
      }

  if( fSums.size()==0 || fParGeo->muonRadius == 0.0 )
    {
      fParGeo->muonSize = 0.0;     
      return;
    }
  if( !getDetectorGeo() )
  {
      cout << "VImageParameterCalculation::sizeInMuonRing error: detector geometry not defined" << endl;
      exit( 0 );
  } 

  unsigned int i;
  double xi, yi, rp, size=0.0, x0, y0, radius, rsigma, si;
  int totalPixels = 0;
  int offPixels = 0;
  x0 = fParGeo->muonX0;
  y0 = fParGeo->muonY0;
  radius = fParGeo->muonRadius;
  rsigma = fParGeo->muonRSigma;

  for( i=0; i<fSums.size(); i++ )
    {
      xi = getDetectorGeo()->getX()[i];
      yi = getDetectorGeo()->getY()[i];
      rp = sqrt( pow( xi-x0 ,2) + pow( yi-y0,2) );
      
//      if( rp > radius - 1.5* rsigma && rp < radius + 1.5* rsigma  )
      if( rp > radius - 0.15 && rp < radius + 0.15  )
	{
	if( fBorder[i] || fImage[i] )
	    {
	    si=(double)fSums[i]; // charge (dc)
	    size += si;
	    totalPixels ++;
	    }
	if( fDead[i] )
	    offPixels ++;  // list of pixels turned off
	}
    }
  // Correct for fraction of tubes turned off:
  if( totalPixels > 0 )
      size *= (1.*offPixels)/(1.0*totalPixels) + 1. ;

  fParGeo->muonSize = size;
}

/*****************************************************************************
calcPixelDistribution
input: pointer to binaryPicture, origin of masking ring, radius and sigmaR
output: return 1 if there are at least 1 pixels in each octant segment of the ring
        AND at least half the pixels fall within +-1 sigmaR,
        otherwise returns 0
*****************************************************************************/
void VImageParameterCalculation::muonPixelDistribution( valarray<double> fSums, vector<bool> fImage, vector<bool> fBorder  )
{
    if( fDebug ) cout << "VImageParameterCalculation::muonPixelDistribution" << endl;
    if( fSums.size()==0 || fParGeo->muonRadius == 0.0 )
    {
        fParGeo->muonValid = 0;
        return;
    }
    if( !getDetectorGeo() )
    {
        cout << "VImageParameterCalculation::muonPixelDistribution error: detector geometry not defined" << endl;
        exit( 0 );
    }

    int totalPixels=0, inside=0, pass = 0;
    unsigned int i;
    int q[8] = {0};
    double rp, phi, xi, yi, x0, y0, radius, rsigma;

    x0 = fParGeo->muonX0;
    y0 = fParGeo->muonY0;
    radius = fParGeo->muonRadius;
    rsigma = fParGeo->muonRSigma;

    for( i=0; i<fSums.size(); i++ )
        if( fImage[i] || fBorder[i] )
    {
        totalPixels++;
        xi = getDetectorGeo()->getX()[i];
        yi = getDetectorGeo()->getY()[i];
        rp = sqrt( pow( xi-x0 ,2) + pow( yi-y0,2) );
        phi = 180.0/TMath::Pi()* atan2( yi-y0 , xi-x0 ) +180.;

        if( rp > radius - 1.5*rsigma  && rp < radius + 1.5*rsigma )
        {
            inside++;

//	    cout << "i "<<i<<" phi "<<phi<<endl;

            if( phi > 0 && phi < 45 )
                q[0]++;
            else if( phi > 45 && phi < 90 )
                q[1]++;
            else if( phi > 90 && phi < 135 )
                q[2]++;
            else if( phi > 135 && phi < 180 )
                q[3]++;
            else if( phi > 180 && phi < 225 )
                q[4]++;
            else if( phi > 225 && phi < 270 )
                q[5]++;
            else if( phi > 270 && phi < 315 )
                q[6]++;
            else if( phi > 315 && phi < 360 )
                q[7]++;

        }

    }

//cout <<""<<inside<<" total: "<<totalPixels<<" q0:"<<q[0]<<" q1:"<<q[1]<<" q2:"<<q[2]<<" q3:"<<q[3]<<" q4:"<<q[4]<<" q5:"<<q[5]<<" q6:"<<q[6]<<" q7:"<<q[7]<<endl;

    if( (1.0*inside/totalPixels > 0.7) && q[0]>1&& q[1]>1&& q[2]>1&& q[3]>1&& q[4]>1&& q[5]>1&& q[6]>1&& q[7]>1  )
        pass = 1;

    fParGeo->muonValid = pass;

}


//****************************************************************///

void VImageParameterCalculation::calcParameters( valarray<double> fSums, valarray<double> fSums2, vector<bool> fImage, vector<bool> fBorder )
{
    vector< bool > iBrightNoImage;
    iBrightNoImage.resize( fImage.size(), false );
    calcParameters( fSums, fSums2, fImage, fBorder, iBrightNoImage, iBrightNoImage, iBrightNoImage );
}


void VImageParameterCalculation::calcParameters( valarray<double> fSums, valarray<double> fSums2, vector<bool> fImage, vector<bool> fBorder, vector< bool > fBrightNoImage )
{
    vector< bool > iHiLo;
    iHiLo.resize( fImage.size(), false );
    calcParameters( fSums, fSums2, fImage, fBorder, fBrightNoImage, iHiLo, iHiLo );
}


void VImageParameterCalculation::calcTriggerParameters( vector<bool> fTrigger )
{
// MS: This function does:
// Calculate the trigger-level centroids
    double sumx_trig=0.;                          // MS
    double sumy_trig=0.;                          // MS
    double sumx2_trig=0.;                         // MS
    double sumy2_trig=0.;                         // MS
    int trig_tubes=0;

// MS: calculated trigger-level parameters
// loop over all pixels
    for( unsigned int j = 0; j < fTrigger.size(); j++ )
    {
// select trigger pixel
        if( fTrigger[j] )
        {
            trig_tubes +=1;

            double xi = getDetectorGeo()->getX()[j];
            double yi = getDetectorGeo()->getY()[j];

            sumx_trig += xi;                      // MS
            sumy_trig += yi;                      // MS
            sumx2_trig += xi*xi;                  // MS
            sumy2_trig += yi*yi;                  // MS
        }
    }
    fParGeo->trig_tubes = trig_tubes;

//  MS: store the trigger-level information
    if (fParGeo->trig_tubes!=0)
    {
                                                  // MS
        const double xmean_trig = sumx_trig / trig_tubes;
                                                  // MS
        const double ymean_trig = sumy_trig / trig_tubes;
                                                  // MS
        const double x2mean_trig = sumx2_trig / trig_tubes;
                                                  // MS
        const double y2mean_trig = sumy2_trig / trig_tubes;

        fParGeo->cen_x_trig=xmean_trig;           // MS
        fParGeo->cen_y_trig=ymean_trig;           // MS
        fParGeo->cen_x2_trig=x2mean_trig;         // MS
        fParGeo->cen_y2_trig=y2mean_trig;         // MS
    }

}


/*!
    see Fegan, D.J. J. Phys. G: Nucl. Part. Phys. 23 (1997) 1013-1060
*/
void VImageParameterCalculation::calcParameters( valarray<double> fSums, valarray<double> fSums2, 
                                                 vector<bool> fImage, vector<bool> fBorder, vector< bool > fBrightNoImage,
						 vector< bool > fHiLo, vector< bool > fImageBorderNeighbour )
{
    if( fDebug ) cout << "VImageParameterCalculation::calcParameters " << fImageBorderNeighbour.size() << endl;
    const double ZeroTolerence = 1e-8;

    if( fSums.size() == 0 )
    {
       if( fDebug ) cout << "\t VImageParameterCalculation::calcParameters: fSums.size() " << fSums.size() << endl;
       return;
    }

    if( !getDetectorGeo() )
    {
        cout << "VImageParameterCalculation::calcParameters error: detector geometry not defined" << endl;
        exit( 0 );
    }

    double sumsig=0;
    double sumsig_2 = 0.;
    double sumxsig=0;
    double sumysig=0;
    double sumx2sig=0;
    double sumy2sig=0;
    double sumxysig=0;
    double sumx3sig=0;
    double sumy3sig=0;
    double sumx2ysig=0;
    double sumxy2sig=0;
    int pntubes=0;
    int pntubesBrightNoImage = 0;
    double sumOuterRing = 0.;                     // sum signal of image in out ring
    double sumLowGain = 0.;

// calculate mean ped and pedvar
    double nPixPed = 0.;
    fParGeo->fmeanPed_Image = 0.;
    fParGeo->fmeanPedvar_Image = 0.;

    if( fData->getRunParameter()->doFADCAnalysis() && fData->getReader()->hasFADCTrace() )
    {
        for( unsigned int j = 0; j < fImageBorderNeighbour.size(); j++ )
        {
            if( fImageBorderNeighbour[j] )
            {
// mean ped and pedvar over image
                if( fData )
                {
                    if( j < fData->getPeds().size() )
                    {
                        fParGeo->fmeanPed_Image += fData->getPeds()[j];
			fParGeo->fmeanPedvar_Image += fData->getPedvars( fData->getSumWindow() )[j];
                    }
                    nPixPed++;
                }
            }
        }
        if( nPixPed > 0 )
        {
            fParGeo->fmeanPed_Image /= nPixPed;
            fParGeo->fmeanPedvar_Image /= nPixPed;
        }
        else
        {
            fParGeo->fmeanPed_Image = 0.;         // -> pead = 0 means that pedvar is meanpedvar over camera
            if( fData )
            {
                fParGeo->fmeanPedvar_Image = fData->getmeanPedvars( false, fData->getSumWindow() );
            }
            else fParGeo->fmeanPedvar_Image = 0.;
        }
    }

/////////////////////////////////////////
// calculate image parameters
/////////////////////////////////////////

// loop over all pixels
    for( unsigned int j = 0; j < fSums.size(); j++ )
    {
        if( fBrightNoImage[j] ) pntubesBrightNoImage++;

// select image or border pixel
        if( fImage[j] || fBorder[j])
        {
            pntubes +=1;

// loop over image tubes

            double xi = getDetectorGeo()->getX()[j];
            double yi = getDetectorGeo()->getY()[j];

            const double si=(double)fSums[j];     // charge (dc)
            sumsig += si;
	    sumsig_2 += (double)fSums2[j];
            if( getDetectorGeo()->getNNeighbours()[j] < getDetectorGeo()->getMaxNeighbour() ) sumOuterRing += si;
            if( fHiLo[j] ) sumLowGain += si;

            const double sixi=si*xi;
            const double siyi=si*yi;

            sumxsig += sixi;
            sumysig += siyi;

            const double sixi2=sixi*xi;
            const double siyi2=siyi*yi;
            const double sixiyi=sixi*yi;

            sumx2sig += sixi2;
            sumy2sig += siyi2;
            sumxysig += sixiyi;

            sumx3sig += sixi2*xi;
            sumy3sig += siyi2*yi;
            sumx2ysig += sixi2*yi;
            sumxy2sig += siyi2*xi;
        }
    }
    fParGeo->ntubes=pntubes;
    fParGeo->ntubesBrightNoImage = pntubesBrightNoImage;
    if( sumsig > 0. ) fParGeo->loss = sumOuterRing / sumsig;
    else              fParGeo->loss = 0.;
    if( sumLowGain > 0. ) fParGeo->fracLow = sumLowGain / sumsig;
    else                  fParGeo->fracLow = 0.;

//! clumsy, but effective, algorithm for finding 3 largest sums and pixel indices
    double i_max[3];
    i_max[0]=-1000.;
    i_max[1]=-1000.;
    i_max[2]=-1000.;
    int i_index[3];
    i_index[0]=0;
    i_index[1]=0;
    i_index[2]=0;

    for (unsigned int i=0;i<fSums.size();i++)
    {
        if (fSums[i]>i_max[0])
        {
            i_max[2]=i_max[1];
            i_max[1]=i_max[0];
            i_max[0]=fSums[i];
            i_index[2]=i_index[1];
            i_index[1]=i_index[0];
            i_index[0]=i;
        }
        else if (fSums[i]>i_max[1])
        {
            i_max[2]=i_max[1];
            i_max[1]=fSums[i];
            i_index[2]=i_index[1];
            i_index[1]=i;
        }
        else if (fSums[i]>i_max[2])
        {
            i_max[2]=fSums[i];
            i_index[2]=i;
        }
    }

    for (unsigned int i=0;i<3;i++)
    {
        fParGeo->max[i]=i_max[i];
        if (i_max[0]>0.0) fParGeo->frac[i]=i_max[i]/i_max[0];
        fParGeo->index_of_max[i]=i_index[i];
    }

    if (fParGeo->ntubes == 0)
    {
       fParGeo->reset( 1 );   // reset level 1: don't reset run numbers, event numbers, etc.
    }
    else
    {
        double xmean = 0.;
        if( sumsig > 0. ) xmean = sumxsig / sumsig;
        double ymean = 0.;
        if( sumsig > 0. ) ymean = sumysig / sumsig;

        double x2mean = 0.;
        if( sumsig > 0. ) x2mean = sumx2sig / sumsig;
        double y2mean = 0.;
        if( sumsig > 0. ) y2mean = sumy2sig / sumsig;
        double xymean = 0.;
        if( sumsig > 0. ) xymean = sumxysig / sumsig;

        const double xmean2 = xmean * xmean;
        const double ymean2 = ymean * ymean;
        const double meanxy = xmean * ymean;

        double sdevx2 = x2mean - xmean2;
        double sdevy2 = y2mean - ymean2;
        const double sdevxy = xymean - meanxy;

        const double dist    = sqrt(xmean2+ymean2);

        fParGeo->size=sumsig;
        fParGeo->size2=sumsig_2;
        fParGeo->cen_x=xmean;
        fParGeo->cen_y=ymean;
        fParGeo->dist=dist;
        if( sdevx2 < ZeroTolerence ) sdevx2 = 0.;
        fParGeo->sigmaX = sqrt( sdevx2 );
        if( sdevy2 < ZeroTolerence ) sdevy2 = 0.;
        fParGeo->sigmaY = sqrt( sdevy2 );

////////////////////////////////////////////////////////////////////////////
/////////////// directional cosines of the semi-major axis /////////////////
////////////////////////////////////////////////////////////////////////////

        const double d = sdevy2 - sdevx2;
        const double z = sqrt(d*d + 4.0*sdevxy*sdevxy);

        fParGeo->f_d = d;
        fParGeo->f_s = z;
        fParGeo->f_sdevxy = sdevxy;

        double cosphi=0;
        double sinphi=0;

        if ( fabs(sdevxy) > ZeroTolerence )       // length != width, semi-major axis
        {                                         // not along x or y
            const double ac=(d+z)*ymean + 2.0*sdevxy*xmean;
            const double bc=2.0*sdevxy*ymean - (d-z)*xmean;
            const double cc=sqrt(ac*ac+bc*bc);
            cosphi=bc/cc;
            sinphi=ac/cc;
        }
        else if ( z > ZeroTolerence )             // semi-major axis along x or y
        {                                         // and length != width
            cosphi = (sdevx2 > sdevy2) ? 1 : 0;
            sinphi = (sdevx2 > sdevy2) ? 0 : 1;
        }
        else if ( dist > 0 )                      // length = width so might as well have semi-major axis
        {                                         // be consistant with miss = dist, ie alpha = 90
            cosphi=-ymean / dist;
// There seems to be a strange FP problem with the code below..
//      sinphi= xmean / dist;
            sinphi= sqrt(1.0-cosphi*cosphi);
        }
        else
        {
            cosphi=1;
            sinphi=0;
        }

        fParGeo->cosphi=cosphi;
        fParGeo->sinphi=sinphi;

////////////////////////////////////////////////////////////////////////////
//////////////// length, width and miss - image parameters /////////////////
////////////////////////////////////////////////////////////////////////////

        double length2 = (sdevx2+sdevy2+z)/2.0;
        if ( length2 < ZeroTolerence )
        {
//if ( length2 < -(ZeroTolerence) )
//	throw Error("Length squared is less than -ZeroTolerence");

            length2=0;
        }

        double width2  = (sdevx2+sdevy2-z)/2.0;
        if ( width2 < ZeroTolerence )
        {
//if ( width2 < -(ZeroTolerence) )
//throw Error("Width squared is less than -ZeroTolerence");

            width2=0;
        }

        const double length  = sqrt(length2);
        const double width   = sqrt(width2);

#ifdef OLDMISS
        double miss2=0;
        if( z > ZeroTolerence )
        {
            const double u = 1+d/z;
            const double v = 2-u;
            miss2 = (u*xmean2 + v*ymean2)/2.0 - meanxy*(2.0*sdevxy/z);

            if ( miss2 < ZeroTolerence )
            {
//if ( miss2 < -(ZeroTolerence) )
//throw Error("Miss squared is less than -ZeroTolerence");

                miss2 = 0;
            }
        }
        else
        {
            miss2 = xmean2 + ymean2;              // ie = dist ^ 2
        }
        const double miss=sqrt(miss2);
#else
        double miss    = fabs(-sinphi*xmean + cosphi*ymean);
        if(miss > dist)miss=dist;                 // Weird rounding error
#endif

        fParGeo->length=length;
        fParGeo->width=width;
        fParGeo->miss=miss;

        fParGeo->los=length/sumsig;

///////////////////////////////////////////////////////////////////////////////////
///////// fraction of image/border pixels located under image ellipse /////////////
///////////////////////////////////////////////////////////////////////////////////

        fParGeo->fui = getFractionOfImageBorderPixelUnderImage( fImage, fBorder, xmean, ymean, width, length, cosphi, sinphi );

////////////////////////////////////////////////////////////////////////////
/////////////////////// orientation: sinalpha and alpha ////////////////////
////////////////////////////////////////////////////////////////////////////

        const double sinalpha = (dist>ZeroTolerence) ? miss/dist : 0;
//  if(sinalpha>1.0)sinalpha=1.0; // Floating point sanity check

        const double alpha=fabs(45./atan(1.) * asin(sinalpha));

        fParGeo->alpha=alpha;

////////////////////////////////////////////////////////////////////////////
//////////////////////////////// Azwidth ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////

        double azwidth;
        if(width2 > ZeroTolerence)
        {
            const double s2a=sinalpha*sinalpha;
            const double c2a=1.0-s2a;
            const double azfactor=
                1.0+((sinalpha==0)?0.0:(length2-width2)/(width2+length2*c2a/s2a));
            azwidth=width*sqrt(azfactor);
        }
        else
        {
            azwidth=length;
        }
        fParGeo->azwidth=azwidth;

////////////////////////////////////////////////////////////////////////////
//////////////////////// asymmetry major and minor /////////////////////////
////////////////////////////////////////////////////////////////////////////

        double asymmetry = 0;
        double minorasymmetry = 0;

        if(length2 > ZeroTolerence)
        {
            const double x3mean = sumx3sig / sumsig;
            const double y3mean = sumy3sig / sumsig;
            const double x2ymean = sumx2ysig / sumsig;
            const double xy2mean = sumxy2sig / sumsig;

            const double sdevx3 = x3mean - 3.0*xmean*x2mean + 2.0*xmean*xmean2;
            const double sdevy3 = y3mean - 3.0*ymean*y2mean + 2.0*ymean*ymean2;
            const double sdevx2y =
                x2ymean - 2.0*xymean*xmean + 2.0*xmean2*ymean - x2mean*ymean;
            const double sdevxy2 =
                xy2mean - 2.0*xymean*ymean + 2.0*xmean*ymean2 - xmean*y2mean;

            const double cosphi2=cosphi*cosphi;
            const double sinphi2=sinphi*sinphi;

            const double asymmetry3length3 =
                sdevx3*cosphi*cosphi2 + 3.0*sdevx2y*sinphi*cosphi2 +
                3.0*sdevxy2*cosphi*sinphi2 + sdevy3*sinphi*sinphi2;

            if ( fabs(asymmetry3length3) > ZeroTolerence )
            {
                asymmetry = pow(fabs(asymmetry3length3),0.333333333333) / length;
                if ( asymmetry3length3 < 0 )asymmetry = -asymmetry;
            }

            if(width2 > ZeroTolerence)
            {
                const double minorasymmetry3width3 =
                    -sdevx3*sinphi*sinphi2 + 3.0*sdevx2y*cosphi*sinphi2 -
                    3.0*sdevxy2*sinphi*cosphi2 + sdevy3*cosphi*cosphi2;

                if ( fabs(minorasymmetry3width3) > ZeroTolerence )
                {
                    minorasymmetry = pow(fabs(minorasymmetry3width3),
                        0.333333333333) / width;
                    if ( minorasymmetry3width3 < 0 )minorasymmetry = -minorasymmetry;
                }
            }
        }

        fParGeo->asymmetry=asymmetry;

        fParGeo->phi = atan2( fParGeo->sinphi, fParGeo->cosphi );
    }

//  cout << time << " ";
//  printf(" %f %f %f %f %f %f %f %d \n",fParGeo->cen_x,fParGeo->cen_y,fParGeo->dist,fParGeo->length,fParGeo->width,fParGeo->alpha,fParGeo->size,fParGeo->ntubes);

    fboolCalcGeo = true;

}


/*!
    The loglikelihood fit is thought to estimate signals in dead channels.

    FCN is get_LL_imageParameter_2DGauss()

    Only signals in image/border pixels are taken into account, all other are set to zero.

    Signals are estimated for dead pixels.
    If estimated signal is above image/border threshold, VEvndispData::getSums()[pixelID] is set to
    this value and the fLLEst[pixelID] is set true.
    A new value for the size is calculated.
This is the old size from the geometrical calculation  plus the estimated signals from the
dead pixels.

Fit has problems with images where all pixels are in one line (not really a 2D Gauss).
For all failed fits, the signals in the dead channels are estimated by averaging over all
neighbour pixels.

\attention

Check always in the analysis the status of the fit (FitStat).
Values lower than 3 (approximate or no error matrix) should be excluded from the analysis.
In this case there is a large probablitity that the parameters are wrong.

(GM)
*/
vector<bool> VImageParameterCalculation::calcLL( VEvndispData *iData )
{
    if( !iData )
    {
        vector< bool > a;
        cout << "VImageParameterCalculation::calcLL error: data vector is zero" << endl;
        return a;
    }
    if( fLLDebug )
    {
        cout << endl;
        cout << "=================================================================" << endl;
        cout << "Telescope " << iData->getTelID()+1 << endl;
        cout << "=================================================================" << endl;
        cout << endl;
    }

    const double ZeroTolerence = 1e-8;
// fit will fail for width < this value
    const double iFMinWidth = 0.001;

// get pixel values, all nonimage/nonborder pixels have iData->getSums()=0.
// if there are dead channels, size of these vectors is < iData->getSums().size() !!
    fll_X.clear();
    fll_Y.clear();
    fll_Sums.clear();
// will be true if sum in pixel is estimated by fit
    fLLEst.assign( iData->getSums().size(), false );
// maximum size of the camera (for fit parameter limits)
    double fdistXmin = 0.;
    double fdistXmax = 0.;
    double fdistYmin = 0.;
    double fdistYmax = 0.;
    double iPixelrad = 0.;
    for( unsigned int j = 0; j < iData->getSums().size(); j++ )
    {
        if( !iData->getDead()[j] )
        {
            double xi = getDetectorGeo()->getX()[j];
            double yi = getDetectorGeo()->getY()[j];

            if( xi > fdistXmax ) fdistXmax = xi;
            if( xi < fdistXmin ) fdistXmin = xi;
            if( yi > fdistYmax ) fdistYmax = yi;
            if( yi < fdistYmin ) fdistYmin = yi;

            fll_X.push_back( xi );
            fll_Y.push_back( yi );
            iPixelrad = getDetectorGeo()->getTubeRadius()[j];
// only sums from image/border pixels are taken in the fit
            if( iData->getImage()[j] || iData->getBorder()[j]) fll_Sums.push_back( iData->getSums()[j] );
            else                         fll_Sums.push_back( 0. );
        }
    }
    if( fLLDebug ) cout << "FLL FITTER limits " << fdistXmax << "\t" << fdistXmin << "\t" << fdistYmax << "\t" << fdistYmin << "\t" << fll_Sums.size() << endl;

// take geometrical values as start values (calculate if not already calculated)
    if( !fboolCalcGeo ) calcParameters( iData->getSums(), iData->getSums2(), iData->getImage(), iData->getBorder() );

// define fit variables
    double rho = 0.;
    double drho = 0.;
    double cen_x = 0.;
    double dcen_x = 0.;
    double sigmaX = 0.;
    double dsigmaX = 0.;
    double cen_y = 0.;
    double dcen_y = 0.;
    double sigmaY = 0.;
    double dsigmaY = 0.;
    double signal = 0.;
    double dsignal = 0.;

    sigmaX = fParGeo->sigmaX;
    sigmaY = fParGeo->sigmaY;
    cen_x = fParGeo->cen_x;
    cen_y = fParGeo->cen_y;

    if( sigmaX > ZeroTolerence && sigmaY > ZeroTolerence )
    {
        rho = tan( 2. * fParGeo->phi ) * ( sigmaX*sigmaX - sigmaY*sigmaY ) / sigmaX / sigmaY / 2.;
    }
    else rho = 0.;

// don't know if this step parameter is the best
    double step = 1.e-4;
    if( fParGeo->width < iFMinWidth ) sigmaX = sigmaY = 0.1;
    fLLFitter->Release( 0 );
    fLLFitter->Release( 1 );
    fLLFitter->Release( 3 );
    fLLFitter->DefineParameter( 0, "rho", rho, step, 0., 0. );
    if( fParGeo->sigmaX > 0. ) fLLFitter->DefineParameter( 1, "meanX", cen_x, step, cen_x - 2.*fParGeo->sigmaX, cen_x + 2.*fParGeo->sigmaX );
    else                       fLLFitter->DefineParameter( 1, "meanX", cen_x, step, fdistXmin, fdistXmax );
    fLLFitter->DefineParameter( 2, "sigmaX", sigmaX, step, 0., fdistXmax * 2. + 0.1 );
    if( fParGeo->sigmaY > 0. ) fLLFitter->DefineParameter( 3, "meanY", cen_y, step, cen_y - 2.*fParGeo->sigmaY, cen_y + 2.*fParGeo->sigmaY );
    else              fLLFitter->DefineParameter( 3, "meanY", cen_y, step, fdistYmin, fdistYmax );
    fLLFitter->DefineParameter( 4, "sigmaY", sigmaY, step, 0., fdistYmax * 2. + 0.1 );
    fLLFitter->DefineParameter( 5, "signal", signal, step, 0., 1.e6 );

    if( fLLDebug ) cout << "FLLFITTER START " << rho << "\t" << cen_x << "\t" << sigmaX << "\t" << cen_y << "\t" << sigmaY << "\t" << signal << endl;

// now do the minimization
    fLLFitter->Command( "MIGRAD" );
// don't call HESS, trouble with migrad in the error calculation means usually to not use the errors and LL results
//    fLLFitter->Command( "HESSE" );

// get fit statistics
    double edm = 0.;
    double amin = 0.;
    double errdef = 0.;
    int nvpar = 0;
    int nparx = 0;
    int nstat = 0;
    fLLFitter->mnstat( amin, edm, errdef, nvpar, nparx, nstat );
    fParLL->Fitstat = nstat;

    if( fLLDebug ) cout << "FLLFITTER " << nstat << endl;

// get fit results
    fLLFitter->GetParameter( 0, rho, drho );
    fLLFitter->GetParameter( 1, cen_x, dcen_x );
    fLLFitter->GetParameter( 2, sigmaX, dsigmaX );
    fLLFitter->GetParameter( 3, cen_y, dcen_y );
    fLLFitter->GetParameter( 4, sigmaY, dsigmaY );
    fLLFitter->GetParameter( 5, signal, dsignal );

    if( fLLDebug ) cout << "FLLFITTER FIT " << rho << "\t" << cen_x << "\t" << sigmaX << "\t" << cen_y << "\t" << sigmaY << "\t" << signal << endl;

// estimate shower size
// integrate over all bins to get fitted size
    float  iSize = fParGeo->size;
    if( fParLL->Fitstat > 2 )
    {
// assume that all pixel are of the same size
        double cen_x_recentered = 0.;
        double cen_y_recentered = 0.;
        if( iData->getDetectorGeo()->getTubeRadius().size() > 0 && iData->getDetectorGeo()->getTubeRadius()[0] > 0. )
        {
            cen_x_recentered = cen_x - iData->getDetectorGeo()->getTubeRadius()[0]*2.*(int)(cen_x/iData->getDetectorGeo()->getTubeRadius()[0]/2. );
            cen_y_recentered = cen_y - iData->getDetectorGeo()->getTubeRadius()[0]*2.*(int)(cen_y/iData->getDetectorGeo()->getTubeRadius()[0]/2. );
        }
        if( fLLDebug ) cout << "FLLFITTER RECENTERED " << cen_x_recentered << "\t" << cen_y_recentered << endl;
////////////////////////////////////////////////////////////////////
// calculate new size from fit function (not from measured charges!)
        iSize = 0.;
        for( unsigned int i = 0; i < iData->getSums().size(); i++ )
        {
            iSize +=  getFitValue( i, rho, cen_x_recentered, sigmaX, cen_y_recentered, sigmaY, signal );
        }
    }

// calculate phi, length, width
    sigmaX = fabs( sigmaX );
    sigmaY = fabs( sigmaY );
    if( sigmaX < ZeroTolerence ) sigmaX = 0.;
    if( sigmaY < ZeroTolerence ) sigmaY = 0.;

// covariance
    double sigmaXY = rho * sqrt( sigmaX*sigmaX * sigmaY*sigmaY );
    double dsx_y2 = sigmaX*sigmaX-sigmaY*sigmaY;
    double dsxxy2 = sigmaX*sigmaX*sigmaY*sigmaY;

    double d = sigmaY*sigmaY - sigmaX*sigmaX;
    double z = sqrt( d*d + 4. * sigmaXY*sigmaXY );
    fParLL->f_d = d;
    fParLL->f_s = z;
    fParLL->f_sdevxy = sigmaXY;
    double length = sqrt( 0.5 * ( sigmaX * sigmaX + sigmaY * sigmaY + z ) );
    double width = sigmaX * sigmaX + sigmaY * sigmaY - z;
    double phi = atan2( 2.*rho*sigmaX*sigmaY, dsx_y2 ) / 2.;
    if( width > ZeroTolerence ) width = sqrt( 0.5 * width );
//  fit was not successfull if width is close to zero -> take pargeo parameters
    else
    {
        if( fLLDebug ) cout << "FLLFITTER: with < 0T " << width << "\t" << ZeroTolerence << endl;
        width  = fParGeo->width;
        length = fParGeo->length;
        cen_x  = fParGeo->cen_x;
        cen_y  = fParGeo->cen_y;
        phi    = fParGeo->phi;
        iSize  = fParGeo->size;
    }
    double cosphi = cos( phi );
    double sinphi = sin( phi );

    double dist = 0.;
    dist = sqrt( cen_x*cen_x + cen_y*cen_y );
    double miss = 0.;
    miss = fabs( cen_y * cosphi - cen_x * sinphi );

    double alpha = 0.;
    if( dist != 0. ) alpha = asin( miss / dist );

    double los;
    if( iSize != 0. ) los = length / iSize;
    else              los = -1.;

// error propagation (uff...)

    double dsigmaXY2;
    dsigmaXY2  = dsxxy2 * drho * drho;
    dsigmaXY2 += rho*rho * sigmaX*sigmaX *sigmaY*sigmaY*sigmaY*sigmaY / dsxxy2 * dsigmaX*dsigmaX;
                                                  // o.k.
    dsigmaXY2 += rho*rho * sigmaX*sigmaX *sigmaX*sigmaX*sigmaY*sigmaY / dsxxy2 * dsigmaY*dsigmaY;

    double dd2;
                                                  // o.k.
    dd2 = 4.*sigmaY*sigmaY * dsigmaY*dsigmaY + 4.*sigmaX*sigmaX * dsigmaX*dsigmaX;

    double dz2;
    dz2  = d*d / (d*d + 4.*sigmaXY*sigmaXY ) * dd2;
                                                  // o.k.
    dz2 += 16.*sigmaXY*sigmaXY / (d*d + 4.*sigmaXY*sigmaXY ) * dsigmaXY2;

    double dlength;
    dlength  = 4.*sigmaX*sigmaX * dsigmaX*dsigmaX;
    dlength += 4.*sigmaY*sigmaY * dsigmaY*dsigmaY;
    dlength += 1./2. * dz2;
    dlength *= 1./8. / (sigmaX*sigmaX + sigmaY*sigmaY + z );
    if( dlength > 0. ) dlength = sqrt( dlength ); // o.k.
    else               dlength = 0.;

    double dwidth;
    dwidth  = 4.*sigmaX*sigmaX * dsigmaX*dsigmaX;
    dwidth += 4.*sigmaY*sigmaY * dsigmaY*dsigmaY;
    dwidth += dz2;
    dwidth *= 1./8. / (sigmaX*sigmaX + sigmaY*sigmaY - z );
    if( dwidth > 0. ) dwidth  = sqrt( dwidth );   // o.k.
    else              dwidth = 0.;

    double dphi;
    if( dsx_y2 == 0. ) dphi = 1000.;
    else
    {
        dphi  = 4.* dsxxy2 / dsx_y2 / dsx_y2 * drho * drho;
                                                  // o.k.
        dphi += (2.*rho*sigmaY*dsx_y2-4.*rho*sigmaY*sigmaX*sigmaX)*(2.*rho*sigmaY*dsx_y2-4.*rho*sigmaY*sigmaX*sigmaX)/dsx_y2/dsx_y2/dsx_y2/dsx_y2*dsigmaX*dsigmaX;
                                                  // o.k.
        dphi += (2.*rho*sigmaX*dsx_y2+4.*rho*sigmaY*sigmaY*sigmaX)*(2.*rho*sigmaX*dsx_y2+4.*rho*sigmaY*sigmaY*sigmaX)/dsx_y2/dsx_y2/dsx_y2/dsx_y2*dsigmaY*dsigmaY;
        dphi *= 1. / ( 1. + 2.*rho*sigmaX*sigmaY/dsx_y2 * 2.*rho*sigmaX*sigmaY/dsx_y2 );
        dphi *= 1. / ( 1. + 2.*rho*sigmaX*sigmaY/dsx_y2 * 2.*rho*sigmaX*sigmaY/dsx_y2 );
        dphi  = sqrt( dphi );
        dphi  = redang( dphi, M_PI/2. );          // o.k.
    }

    double ddist = 0.;
    if( dist != 0. ) ddist = cen_x*cen_x /dist/dist * dcen_x*dcen_x + cen_y*cen_y /dist/dist * dcen_y*dcen_y;
    ddist = sqrt( ddist );                        // o.k.
    double dmiss = 0.;
    dmiss  = cosphi*cosphi * dcen_x*dcen_x + sinphi*sinphi * dcen_y*dcen_y;
    dmiss += dphi*dphi*( sinphi*sinphi * cen_x*cen_x + cosphi*cosphi * cen_y*cen_y );
    dmiss = sqrt( dmiss );                        // o.k.

    double dalpha = 0.;
    if( dist != 0. )
    {
        dalpha  = dmiss*dmiss / dist/dist + miss*miss / dist/dist/dist/dist * ddist*dist;
                                                  // o.k.
        dalpha *= 1. / sqrt( 1. - miss*miss / dist/dist );
    }
    else dalpha = 0.;
    dalpha = redang( sqrt( dalpha ), 2. * M_PI );
    if( dalpha > 360. ) fParLL->Fitstat = -1;     //  floating point problem

// filling of VImageParameter 

    fParLL->cen_x = cen_x;
    fParLL->cen_y = cen_y;
    fParLL->length = length;
    fParLL->width = width;
    fParLL->size = iSize;
    fParLL->dist = dist;
    fParLL->azwidth = -1.;                        // !!!!!!!!!!!!!! to tired for calculation
    fParLL->alpha = alpha * 45. / atan( 1.);
    fParLL->los = los;
    fParLL->miss = miss;
    fParLL->phi = phi;
    fParLL->cosphi = cos( fParLL->phi );
    fParLL->sinphi = sin( fParLL->phi );
    fParLL->fui = getFractionOfImageBorderPixelUnderImage( iData->getImage(), iData->getBorder(), cen_x, cen_y, width, length, fParLL->cosphi, fParLL->sinphi );
    fParLL->dcen_x = dcen_x;
    fParLL->dcen_y = dcen_y;
    fParLL->dlength = dlength;
    fParLL->dwidth = dwidth;
    fParLL->ddist = ddist;
    fParLL->dmiss = dmiss;
    fParLL->dphi = dphi;
    fParLL->dalpha = dalpha * 45. / atan( 1. );
    fParLL->dazwidth = 0.;
// all the fit parameters
    fParLL->Fitmin = amin;
    fParLL->Fitedm = edm;
    fParLL->ntRec = 0;
    fParLL->rho = rho;
    fParLL->sigmaX = sigmaX;
    fParLL->sigmaY = sigmaY;
    fParLL->signal = signal;
    fParLL->drho = drho;
    fParLL->dsigmaX = dsigmaX;
    fParLL->dsigmaY = dsigmaY;
    fParLL->dsignal = dsignal;

    fParLL->ntubes = fParGeo->ntubes;
    fParLL->bad = fParGeo->bad;

    return fLLEst;
}


double VImageParameterCalculation::getFitValue( unsigned int iChannel, double rho, double meanX, double sigmaX, double meanY, double sigmaY, double signal )
{
    double f = 0;
// get channel coordinates
    double x, y;
    x = getDetectorGeo()->getX()[iChannel];
    y = getDetectorGeo()->getY()[iChannel];
// calculate 2D-gauss
    f  = ( x - meanX ) * ( x - meanX ) / sigmaX / sigmaX;
    f += ( y - meanY ) * ( y - meanY ) / sigmaY / sigmaY;
    f += -2. * rho * ( x - meanX ) / sigmaX * ( y - meanY ) / sigmaY;
    f *= -1. / 2. / ( 1. - rho * rho );
    f  = exp( f );
    f *= 1. / 2. / TMath::Pi() / sigmaX / sigmaY / sqrt( 1. - rho * rho );
    f *= signal;

    return f;
}


/*!
 */
double VImageParameterCalculation::getMeanSignal( int i, VEvndispData *iData )
{
// otherwise take mean value from surrounding pixels
    double iLLsums = 0.;
    int iNllsums;
    iNllsums = 0;
    for( unsigned int j = 0; j < fDetectorGeometry->getNeighbours()[i].size(); j++ )
    {
        unsigned int k = fDetectorGeometry->getNeighbours()[i][j];
        iLLsums += iData->getSums()[k];
        iNllsums++;
    }
    if( iNllsums > 0 ) iLLsums /= iNllsums;
    else iLLsums = 0.;
    return iLLsums;
}


/*!
   reduce angles to values in [0.,iMax]
*/
double VImageParameterCalculation::redang( double iangle, double iMax )
{
    if( iMax == 0. ) return 0.;
    if( iangle >= 0 )
    {
        iangle = iangle - int( iangle / iMax ) * iMax;
    }
    else
    {
        iangle = iMax + iangle + int( iangle / iMax ) * iMax;
    }

    return iangle;
}


/*!
    calculate the fraction of image/border pixels which are located under the image ellipse.

    For a perfect image this is 1, for many cosmic rays this is < 1
*/
double VImageParameterCalculation::getFractionOfImageBorderPixelUnderImage( vector< bool > fImage, vector< bool > fBorder, double cen_x, double cen_y, double width, double length, double cosphi, double sinphi )
{
    float i_ImageCoverFactor = 2.;
    if( fData ) i_ImageCoverFactor = fData->getRunParameter()->fImageAnalysisFUIFactor;
    if( i_ImageCoverFactor <= 0. )
    {
        cout << "VImageParameterCalculation::getFractionOfImageBorderPixelUnderImage error: fui factor <= 0, using 2" << endl;
        i_ImageCoverFactor = 2.;
    }
    float i_ImageCoverNPixel = 0;
    float i_ImageCoverNPixelImageBorder = 0;

    if( length > 1.e-2 && width > 1.e-2 )
    {
// loop over all image and check if they are close to the image ellipse
        for( unsigned int i = 0; i < fImage.size(); i++ )
        {
// pixel coordinates rotated into frame of image ellipse
            double xi =     cosphi * (getDetectorGeo()->getX()[i]-cen_x) + sinphi * (getDetectorGeo()->getY()[i]-cen_y);
            double yi = -1.*sinphi * (getDetectorGeo()->getX()[i]-cen_x) + cosphi * (getDetectorGeo()->getY()[i]-cen_y);

// check if these pixels are inside the image ellipse
            if( xi*xi/length/length/i_ImageCoverFactor/i_ImageCoverFactor + yi*yi/width/width/i_ImageCoverFactor/i_ImageCoverFactor < 1. )
            {
                i_ImageCoverNPixel++;
                if( fImage[i] || fBorder[i]) i_ImageCoverNPixelImageBorder++;
            }
        }
        if( fDebug )
        {
            cout << "Fraction of pixel covered: " << i_ImageCoverNPixel << "\t" << i_ImageCoverNPixelImageBorder << "\t";
            if( i_ImageCoverNPixel > 0. ) cout << i_ImageCoverNPixelImageBorder/i_ImageCoverNPixel;
            else                                     cout << 0.;
            cout << endl;
        }
    }
    else return 1.;

    if( i_ImageCoverNPixel > 0. ) return i_ImageCoverNPixelImageBorder/i_ImageCoverNPixel;

    return 0.;
}


/*! \fn get_LL_imageParameter_2DGauss( Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag)
    \brief function for the loglikelihood calculation of image parameters in minuit (2dim-Gaus)

   gives same parameters as geometrical analysis, but with errors

   log likelihood: see Blobel p.195

   The image is described by a 2d-Gaussian. The signal in each tube is therefore:
   \f[
           S(x,y)=\frac{C}{2\pi\sigma_{x}\sigma_{y}\sqrt{1-\rho^{2}}}
              \exp\left\{-\frac{1}{2(1-\rho^{2})}
\left[\left(\frac{x-c_{x}}{\sigma_{x}}\right)^{2}
-2\rho\left(\frac{x-c_{x}}{\sigma_{x}}\right)\left(\frac{y-c_{y}}{\sigma_{y}}\right)
+\left(\frac{y-c_{y}}{\sigma_{y}}\right)^{2}\right]\right\}

\f]

Assume Poissonian (ignoring NSB) for measuring amplitude:

\f[
P(n_{i}; S ) = \frac{S^{n}}{n!}\exp^{-S}
\f]

This gives the extented log likelihood estimator:

\f[
LL = - \sum (n_{i}\ln S_{i} - S_{i} - n_{i}\ln n_{i} + n_{i})
\f]

Errors are calculated from this estimator by

\f[
2 LL \approx \chi^{2}
\f]

*/
void get_LL_imageParameter_2DGauss( Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag)
{
    double LL = 0.;
    double sum = 0.;

    double x = 0.;
    double y = 0.;
    double n = 0.;                                // measured sum in channel i
//    double iVar = 0.;

    double rho_1 = -1. / 2. / ( 1. - par[0] * par[0] );
    double rho_s =  1. / 2. / M_PI / par[2] / par[4] / sqrt( 1. - par[0] * par[0] ) * par[5];

    VImageParameterCalculation *iImageCalculation = (VImageParameterCalculation*)fLLFitter->GetObjectFit();

    if( par[0] * par[0] < 1. && par[2] > 0. && par[4] > 0. )
    {
        unsigned int nSums = iImageCalculation->getSums().size();
        for( unsigned int i = 0; i < nSums; i++ )
        {
            n = iImageCalculation->getSums()[i];
            if( n > -999. )
            {
                x = iImageCalculation->getX()[i];
                y = iImageCalculation->getY()[i];
                sum  = ( x - par[1] ) * ( x - par[1] ) / par[2] / par[2];
                sum += ( y - par[3] ) * ( y - par[3] ) / par[4] / par[4];
                sum += -2. * par[0] * ( x - par[1] ) / par[2] * ( y - par[3] ) / par[4];
                sum  = rho_s * exp( sum * rho_1 );

// assume Poisson fluctuations (neglecting background noise)
                if( n != 0. && sum != 0. ) LL += n * log(sum) - sum - n * log(n) + n;
                else                       LL += -1. * sum;
// Gauss with background noise
//             iVar = sum+pedvars*pedvars;
//             iVar = sum;
//             LL += -0.5*log(2. * TMath::Pi()) - log(sqrt(iVar)) - (n-sum)*(n-sum)/2./iVar;
            }
        }
    }
    f = -1. * LL;
}
