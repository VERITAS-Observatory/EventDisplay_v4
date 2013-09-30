/*! \class V1FitModel3D

     V1FitModel3D class is the fit for V1Model3D
     based on Lemoine-Goumard et al. 2006
     adapted by J. Grube and G. Gyuk
*/

#include "V1FitModel3D.h"

////////////

extern void *pfitModel3D;   // global pointer to object

///////////

V1FitModel3D::V1FitModel3D( V1Model3DData *iData3D )
{
  fData3D = iData3D;

  //// TEST cherenkov angle ///
  ffun = new TF1("ffun", "[0] + [1]*x + [2]*x*x + [3]*x*x*x", 0, 30);
  double p0 =      1.41111;
  double p1 =   -0.0805553;
  double p2 =   0.00165614;
  double p3 = -1.18822e-05;
  ffun->SetParameters( p0, p1, p2, p3 );
  ////////////////

  ///cout<<"Test: "<< fData3D->flocTel3D[0][0] << endl;
}

V1FitModel3D::~V1FitModel3D()
{
}

//// wrapper functions to allow a callback to the member function ////
//// explicitly casts global pfitModel3D to a pointer to V1FitModel3D ////

//// calcModel ////
double V1FitModel3D::wrapCalcModel3D( unsigned int iTel, unsigned int iPix )
{
  // explicitly cast global variable <pfitModel3D> to a pointer to V1FitModel3D
  V1FitModel3D *mySelf = (V1FitModel3D*) pfitModel3D;
  mySelf->posFit3D(); // call member
  return mySelf->calcModel3D( iTel, iPix );    // call member
}

//// Minuit ////
double V1FitModel3D::wrapFitMinuit3D( const double *xx )
{
  // explicitly cast global variable <pfitModel3D> to a pointer to V1FitModel3D
  V1FitModel3D *mySelf = (V1FitModel3D*) pfitModel3D;
  return mySelf->funFitMinuit3D( xx );    // call member
}

double V1FitModel3D::funFitMinuit3D(const double *xx ) 
{

  fData3D->fSel3D = xx[0];
  fData3D->fSaz3D = xx[1];
  fData3D->fXcore3D = xx[2];
  fData3D->fYcore3D = xx[3];
  fData3D->fSmax3D = xx[4];
  fData3D->fsigmaL3D = xx[5];
  fData3D->fsigmaT3D = xx[6];
  fData3D->fNc3D = xx[7];

  fZenRad = (90. - fData3D->fSel3D) * (TMath::Pi()/180.); // in radians

  //////cout<< "DUDE "<< cos( fZenRad ) << endl;
  /// cout<<fData3D->fSel3D<<" "<<fData3D->fSaz3D<<" "<<fData3D->fXcore3D<<" "<<fData3D->fYcore3D<<" "<<fData3D->fSmax3D<<" "<<fData3D->fsigmaL3D<<" "<<fData3D->fsigmaT3D<<" "<<fData3D->fNc3D <<endl;

  //// calculate "s" and "xB" vectors ////
  posFit3D(); 

  //// const double LL = calcLLFit3D();

  ////////////
  double qdata;
  double qmodel;
  double ped;

  double pdf = 0;
  double LL = 0;

  double Lexpect = 0; 
  double pixDiff = 0;
  int npixel = 0;

  for (unsigned int i = 0; i < fData3D->fNTel3D; i++) {
    for( unsigned int j = 0; j < fData3D->fNpix3D[i]; j++ ) {

      //// skip non-selected pixels //// 
      if( fData3D->fClean3D[i][j] ) {

	/// get measured data ///
	qdata = fData3D->fMeasuredSum3D[i][j]; 
	ped = fData3D->fPedvar3D[i][j];
	/////////////////////////
	/// get model signal ///
	qmodel = calcModel3D( i, j );
	///// FIX: do check for bad data /////
	/// calculate pdf ///
	pdf = calcPDF3D( qdata, qmodel, ped, fData3D->fexNoise3D );

	LL = LL + ( -2.* log( pdf ));
 
	//// pixel goodness of fit ////
	//// Gaussian approximation (not doing numerical integration) ////
	Lexpect = 1. + log(2.*TMath::Pi()) + log( (ped*ped) + qmodel * ( 1. + (fData3D->fexNoise3D*fData3D->fexNoise3D) ) );
	
	pixDiff += (-2. * log( pdf )) - Lexpect;
	
	npixel += 1;
      }
    }
  }

  //// Total Goodness of Fit ////
  /// 8 fit parameters ///
  double sqNDF = sqrt( 2. * ( (double)npixel - 8. ) );

  //// updates fGoodness3D in V1Model3D ////
  fData3D->fGoodness3D = pixDiff / sqNDF;

  /// cout<< "pixDiff = "<< pixDiff <<", Npixel = " << (double)npixel - 8. <<", sqNDF = "<< sqNDF << endl;

  ///////////

  return LL;
}

//// GSL ////

int V1FitModel3D::wrapFitGSL3D( const gsl_vector *v, void *ptr, gsl_vector *f )
{
  V1FitModel3D *mySelf = (V1FitModel3D*) pfitModel3D;
  return mySelf->funFitGSL3D( v, ptr, f );    // call member
}
int V1FitModel3D::wrapDF3D( const gsl_vector *v, void *ptr, gsl_matrix *J )
{
  V1FitModel3D *mySelf = (V1FitModel3D*) pfitModel3D;
  return mySelf->funDF3D( v, ptr, J );    // call member
}
int V1FitModel3D::wrapFDF3D( const gsl_vector *v, void *ptr, gsl_vector *f, gsl_matrix *J )
{
  V1FitModel3D *mySelf = (V1FitModel3D*) pfitModel3D;
  return mySelf->funFDF3D( v, ptr, f, J );    // call member
}

//////////////////////

int V1FitModel3D::funFitGSL3D( const gsl_vector *v, void *ptr, gsl_vector *f ) 
{

  fData3D->fSel3D    = gsl_vector_get( v, 0 );
  fData3D->fSaz3D    = gsl_vector_get( v, 1 );
  fData3D->fXcore3D  = gsl_vector_get( v, 2 );
  fData3D->fYcore3D  = gsl_vector_get( v, 3 );
  fData3D->fSmax3D   = gsl_vector_get( v, 4 );
  fData3D->fsigmaL3D = gsl_vector_get( v, 5 );
  fData3D->fsigmaT3D = gsl_vector_get( v, 6 );
  fData3D->fNc3D     = gsl_vector_get( v, 7 );

  fZenRad = (90. - fData3D->fSel3D) * (TMath::Pi()/180.); // in radians

  //// calculate "s" and "xB" vectors ////
  posFit3D(); 

  /////////////////////////////////////////
  ////// calculation below //////////////
  double qdata;
  double qmodel;
  double ped;

  double pdf;
  double LL;

  double Lexpect = 0; 
  double pixDiff = 0;
  int npixel = 0;

  for (unsigned int i = 0; i < fData3D->fNTel3D; i++) {
    for( unsigned int j = 0; j < fData3D->fNpix3D[i]; j++ ) {
      
      //// skip non-selected pixels //// 
      if( fData3D->fClean3D[i][j] ) {

	qdata = fData3D->fMeasuredSum3D[i][j]; // get measured data
	ped = fData3D->fPedvar3D[i][j]; // get pedvar
	qmodel = calcModel3D( i, j ); // get model signal

	///// FIX: do check for bad data /////
	/// calculate pdf ///
	pdf = calcPDF3D( qdata, qmodel, ped, fData3D->fexNoise3D );

	LL = -2.0 * log( pdf );

	/// if( fData3D->fDebug3D ) cout<<"Tel, Pix: pdf "<< i+1<<","<<j+1<<": "<< pdf << endl;

	/// return pixel LL value to gsl ///
	gsl_vector_set ( f, npixel, LL ); 
 
	//// pixel goodness of fit ////
	//// Gaussian approximation (not doing numerical integration) ////
	Lexpect = 1. + log(2.*TMath::Pi()) + log( (ped*ped) + qmodel * ( 1. + (fData3D->fexNoise3D*fData3D->fexNoise3D) ) );
	
	pixDiff += (-2. * log( pdf )) - Lexpect;
	
	npixel += 1;
      }
    }
  }

////// Total Goodness of Fit ////
  /// 8 fit parameters ///
  double sqNDF = sqrt( 2. * ( (double)npixel - 8. ) );

  //// updates fGoodness3D in V1Model3D ////
  fData3D->fGoodness3D = pixDiff / sqNDF;

//////////////

  return GSL_SUCCESS;
}

int V1FitModel3D::funDF3D( const gsl_vector *v, void *ptr, gsl_matrix *J )
{

  fData3D->fSel3D    = gsl_vector_get( v, 0 );
  fData3D->fSaz3D    = gsl_vector_get( v, 1 );
  fData3D->fXcore3D  = gsl_vector_get( v, 2 );
  fData3D->fYcore3D  = gsl_vector_get( v, 3 );
  fData3D->fSmax3D   = gsl_vector_get( v, 4 );
  fData3D->fsigmaL3D = gsl_vector_get( v, 5 );
  fData3D->fsigmaT3D = gsl_vector_get( v, 6 );
  fData3D->fNc3D     = gsl_vector_get( v, 7 );

  //// TEST: step sizes ////
  // fData3D->fStepSel3D    = 5.; // frogs
  // fData3D->fStepSaz3D    = 20.; // frogs
  // fData3D->fStepXcore3D  = 50.0; // frogs
  // fData3D->fStepYcore3D  = 50.0; // frogs
  // fData3D->fStepSmax3D   = 5000.0; // unknown?
  // fData3D->fStepsigmaL3D = 5000.0; // unknown?
  // fData3D->fStepsigmaT3D = 5.0; // unknown?
  // fData3D->fStepNc3D     = 2.0; // unknown?

  double stepfrac = 0.1;

  fData3D->fStepSel3D    = fabs( fData3D->fStartSel3D )* (stepfrac*0.01); // best 0.1
  fData3D->fStepSaz3D    = fabs( fData3D->fStartSaz3D )* stepfrac; // best 1.0
  fData3D->fStepXcore3D  = fabs( fData3D->fStartXcore3D )* stepfrac; // best 1.0
  fData3D->fStepYcore3D  = fabs( fData3D->fStartYcore3D )* stepfrac; // best 1.0
  fData3D->fStepSmax3D   = fabs( fData3D->fStartSmax3D )* stepfrac; // best 100 or 1000?
  fData3D->fStepsigmaL3D = fabs( fData3D->fStartsigmaL3D )* stepfrac; // best 500 or 1000?
  fData3D->fStepsigmaT3D = fabs( fData3D->fStartsigmaT3D )* stepfrac; // best 0.5 or 0.1?
  fData3D->fStepNc3D     = fabs( fData3D->fStartNc3D )* (stepfrac*0.1); // LOG best 1.0 or 0.5?

  ///// calculation /////
  double derivative = 0;
  int npixel = 0;

  for (unsigned int i = 0; i < fData3D->fNTel3D; i++) {
    for( unsigned int j = 0; j < fData3D->fNpix3D[i]; j++ ) {
      
      //// skip non-selected pixels //// 
      if( fData3D->fClean3D[i][j] ) {

	// derivative with respect to Sel3D
	derivative = calcDerivLL3D( i, j, 0 );
	gsl_matrix_set( J, npixel, 0, derivative );
	// derivative with respect to Saz3D
	derivative = calcDerivLL3D( i, j, 1 );
	gsl_matrix_set( J, npixel, 1, derivative );
	// derivative with respect to Xcore3D
	derivative = calcDerivLL3D( i, j, 2 );
	gsl_matrix_set( J, npixel, 2, derivative );
	// derivative with respect to Ycore3D
	derivative = calcDerivLL3D( i, j, 3 );
	gsl_matrix_set( J, npixel, 3, derivative );
	// derivative with respect to Smax3D
	derivative = calcDerivLL3D( i, j, 4 );
	gsl_matrix_set( J, npixel, 4, derivative );
	// derivative with respect to sigmaL3D
	derivative = calcDerivLL3D( i, j, 5 );
	gsl_matrix_set( J, npixel, 5, derivative );
	// derivative with respect to sigmaT3D
	derivative = calcDerivLL3D( i, j, 6 );
	gsl_matrix_set( J, npixel, 6, derivative );
	// derivative with respect to Nc3D
	derivative = calcDerivLL3D( i, j, 7 );
	gsl_matrix_set( J, npixel, 7, derivative );

	npixel += 1;
      }
    }
  }

  return GSL_SUCCESS;
}
  
int V1FitModel3D::funFDF3D( const gsl_vector *v, void *ptr, gsl_vector *f, gsl_matrix *J )
{
  funFitGSL3D( v, ptr, f ); 
  funDF3D( v, ptr, J );

  return GSL_SUCCESS;
}

///////////

void V1FitModel3D::posFit3D()
{
  //// correct for azimuth below 0 above 360 (deg) ////
  //if( fData3D->fSaz3D < 360. ) fData3D->fSaz3D = 360. + fData3D->fSaz3D;
  //if( fData3D->fSaz3D > 360. ) fData3D->fSaz3D = 360. - fData3D->fSaz3D;
  //// convert back to -180 to 180 (deg) from 0 to 360 (deg) ////
  //fData3D->fSaz3D = fData3D->fSaz3D - 180.;

  ///////////// get "s" vector ////////////////////

  double tSzen = 0; // temp S zenith
  double tSaz = 0; // temp S azimuth
  tSzen = (90. - fData3D->fSel3D) * (TMath::Pi()/180.); // in radians
  tSaz = fData3D->fSaz3D * (TMath::Pi()/180.); // in radians

  //// from spherical to cartesian (ground coordinates) ////
  fData3D->fs3D[0] = sin( tSzen ) * cos( tSaz );
  fData3D->fs3D[1] = sin( tSzen ) * sin( tSaz );
  fData3D->fs3D[2] = cos( tSzen );

  ///cout<< "s: ("<< fData3D->fs3D[0] <<", "<< fData3D->fs3D[1] <<", "<< fData3D->fs3D[2] <<")"<<endl;

  //// calculate xB using height of shower max and core location ////
  // apply magnitude (shower max) to get shower max point
  fData3D->fxBo3D[0] = fData3D->fSmax3D * fData3D->fs3D[0];
  fData3D->fxBo3D[1] = fData3D->fSmax3D * fData3D->fs3D[1];
  fData3D->fxBo3D[2] = fData3D->fSmax3D * fData3D->fs3D[2];

  // move shower to land at core (on ground) relative to (0,0,0)
  fData3D->fxBo3D[0] = fData3D->fXcore3D + fData3D->fxBo3D[0];
  fData3D->fxBo3D[1] = fData3D->fYcore3D + fData3D->fxBo3D[1];
  fData3D->fxBo3D[2] = 0. + fData3D->fxBo3D[2];

  ///cout<<"xB(0,0,0): ("<< fData3D->fxBo3D[0] <<", "<< fData3D->fxBo3D[1] <<", "<< fData3D->fxBo3D[2] <<")"<<endl;
  //cout<<"-------------"<<endl;
  
  ///// telescope xB /////
  for (unsigned int i = 0; i < fData3D->fNdim3D; i++) {
    for (unsigned int j = 0; j < fData3D->fNTel3D; j++) {
       fData3D->fxB3D[i][j] = fData3D->fxBo3D[i] - fData3D->flocTel3D[i][j];
    }
  }

}


///////////////

double V1FitModel3D::calcPDF3D( double qdata, double qmodel, double ped, double exnoise )
{

  //// method adapted from frogs (U of Utah) ////
  double rtn = 0; // return pdf

  ////// TEST Lemoine 2006:  ped = 0.4, exnoise = 0 /////
  // exnoise = 0;
  // ped = 0.4;
  // fGaussLimit = 1e10; // model signal when PDF becomes Gaussian-like
  // fNstdPoisson = 3.0; // number of standard deviations for Poisson calculation
  // fsmallNumber = 1.e-25; // to avoid divergence when likelihood approaches zero


  ///// FROGS ped correction /////
  /// double correction = -0.0598*ped + 0.8257;
  /// ped = ped * correction;

  //// ped = ped*1.1; TEST

  ///// FROGS //////////////////////////////////////////////////////////////
  fGaussLimit = 25.; // model signal when PDF becomes Gaussian-like
  fNstdPoisson = 5.0; // number of standard deviations for Poisson calculation
  fsmallNumber = 1.e-25; // to avoid divergence when likelihood approaches zero

  //// TEST FROGS ////
  // exnoise = 0.4;
  // ped = 0.4;
  // fGaussLimit = 15; // model signal when PDF becomes Gaussian-like
  //fNstdPoisson = 3.0; // number of standard deviations for Poisson calculatio

  //// case when qmodel = 0 ////
  if( qmodel == 0.0 ) return exp( -qdata*qdata / (2.0*ped*ped) ) / ( sqrt(2.*TMath::Pi())*ped );

  //// case for Gaussian-like distribution instead of a Poisson ////
  //// (when qmodel is larger than fGaussLimit) /// 
  if( qmodel > fGaussLimit ) {
    double term1 = 1. / sqrt( 2*TMath::Pi() * ( (ped*ped) + (qmodel*(1. + (exnoise*exnoise) )) ) );
    double term2 = exp( -((qdata - qmodel)*(qdata - qmodel)) / (2.*( (ped*ped) + (qmodel*(1. + (exnoise*exnoise) ))) ) );
    double pdfG = term1 * term2;
    return pdfG; 
  }

  //// case for a Poisson distribution (intermediate values of qmodel) ////
  if( qmodel <= fGaussLimit ) {

    float stdev = sqrt( qmodel*(1 + exnoise*exnoise) + ped*ped );

    int Nmin=(int)(floor( qmodel - fNstdPoisson*stdev ));
    if(Nmin<0) Nmin=0;
    int Nmax=(int)(floor( qmodel + fNstdPoisson*stdev ));
    if( Nmax<Nmin ) Nmax = Nmin; //Although it does not seem possible

    double poisson1 = 0; // Poisson factor
    double poisson2 = 0; // Poisson factor
    double logFact = 0;  // log of factorial

    // tabulated value for n<11
    int a[11]={1,1,2,6,24,120,720,5040,40320,362880,3628800};

    //Sum over the possible number of photons contributing to the signal
    for(int n=Nmin; n<=Nmax; n++) {

      // P(qmodel,n)=qmodel^n*exp(-qmodel)/n! but we go logarithmic to avoid numerical problems
      
      //Compute the log of n!
      if(n<0) logFact = 0;
      else if(n<11) logFact = (double)log( a[n] );
     // What follows is Srinivasa Ramanujan approximation of n!
     // For n=10 it returns log(3628797.9)
     // For n=11 it returns log(39916782) instead of log(3991680)
     // 0.57236494 is 0.5*log(pi)
      else logFact =n*log(n)-n+log(n*(1.0+4.0*n*(1.0+2.0*n)))/6.0+0.57236494;

      if( qmodel > 0.0 ) poisson1 = exp( n*log(qmodel) - qmodel - logFact );
      else if( n==0 && qmodel==0 ) poisson1 = 1.0;
      else poisson1 = 0.0;

      poisson2 = ped*ped+n*exnoise*exnoise; //standard deviation
      poisson1 = poisson1 / sqrt( 2.*TMath::Pi()*poisson2 ); //Gauss distrib. norm factor
      rtn = rtn + poisson1*exp( -(qdata-n)*(qdata-n) / (2.0*poisson2)); //Gauss distribution with Poisson probability weight
    }
 
  }

  // if the probability density function numerically drops to zero, 
  // the penalty payed for that pixel diverges, 
  // so it's capped to retain sensitivity to parameter changes from other pixels
  if( rtn < fsmallNumber ) return fsmallNumber;

  return rtn;
}

//////

double V1FitModel3D::calcDerivLL3D( unsigned int iTel, unsigned int iPix, int iParID )
{
  /// derivative calculated as df/dx=[f(x+dx)-f(x-dx)]/(2*dx) ///

  double rtn;
  double qdata;
  double qmodel;
  double ped;
  double pdf;

  /// get measured data ///
  qdata = fData3D->fMeasuredSum3D[iTel][iPix]; 
  ped = fData3D->fPedvar3D[iTel][iPix];

  /// step forward ///  
  stepPar( iParID, 1.0 );
  posFit3D();
  qmodel = calcModel3D( iTel, iPix );
  pdf = calcPDF3D( qdata, qmodel, ped, fData3D->fexNoise3D );
  double pix_plus = -2.0 * log( pdf );
  stepPar( iParID, -1.0 ); // reset

  /// step backward ///  
  stepPar( iParID, -1.0 ); 
  posFit3D();
  qmodel = calcModel3D( iTel, iPix );
  pdf = calcPDF3D( qdata, qmodel, ped, fData3D->fexNoise3D );
  double pix_minus = -2.0 * log( pdf );
  stepPar( iParID, 1.0 ); // reset

  /// evaluate the derivative ///
  float deltaPar = 0;
  if( iParID == 0 ) deltaPar = fData3D->fStepSel3D;
  if( iParID == 1 ) deltaPar = fData3D->fStepSaz3D;
  if( iParID == 2 ) deltaPar = fData3D->fStepXcore3D;
  if( iParID == 3 ) deltaPar = fData3D->fStepYcore3D;
  if( iParID == 4 ) deltaPar = fData3D->fStepSmax3D;
  if( iParID == 5 ) deltaPar = fData3D->fStepsigmaL3D;
  if( iParID == 6 ) deltaPar = fData3D->fStepsigmaT3D;
  if( iParID == 7 ) deltaPar = fData3D->fStepNc3D;

  rtn = ( pix_plus - pix_minus ) / ( 2.0 * deltaPar );

  /// don't FIX el and az ///
  // if( iParID == 0 ) rtn = 0;
  // if( iParID == 1 ) rtn = 0;
  // if( rtn < fsmallNumber ) return fsmallNumber;

  if( fData3D->fMeasuredSum3D[iTel][iPix] > 9.0 ) cout<<"Der("<<iTel<<","<<iPix<<","<<iParID<<") plus =  "<< pix_plus <<", minus = "<< pix_minus <<", der = "<< rtn <<endl;

  return rtn;
}

void V1FitModel3D::stepPar( int iParID, float iMult )
{
  if( iParID == 0 ) fData3D->fSel3D    = fData3D->fSel3D    + ( iMult * fData3D->fStepSel3D    );
  if( iParID == 1 ) fData3D->fSaz3D    = fData3D->fSaz3D    + ( iMult * fData3D->fStepSaz3D    );
  if( iParID == 2 ) fData3D->fXcore3D  = fData3D->fXcore3D  + ( iMult * fData3D->fStepXcore3D  );
  if( iParID == 3 ) fData3D->fYcore3D  = fData3D->fYcore3D  + ( iMult * fData3D->fStepYcore3D  );
  if( iParID == 4 ) fData3D->fSmax3D   = fData3D->fSmax3D   + ( iMult * fData3D->fStepSmax3D   );
  if( iParID == 5 ) fData3D->fsigmaL3D = fData3D->fsigmaL3D + ( iMult * fData3D->fStepsigmaL3D );
  if( iParID == 6 ) fData3D->fsigmaT3D = fData3D->fsigmaT3D + ( iMult * fData3D->fStepsigmaT3D );
  if( iParID == 7 ) fData3D->fNc3D     = fData3D->fNc3D     + ( iMult * fData3D->fStepNc3D     );
}

//////////////
// void V1FitModel3D::calcTotalModel3D()
// {
//    cout<<"TOTAL"<<endl;
//    cout<<fData3D->fSel3D<<" "<<fData3D->fSaz3D<<" "<<fData3D->fXcore3D<<" "<<fData3D->fYcore3D<<" "<<fData3D->fSmax3D<<" "<<fData3D->fsigmaL3D<<" "<<fData3D->fsigmaT3D<<" "<<fData3D->fNc3D <<endl;
//  
//    double tempmodel = 0;
//    double totmodel = 0;
//    double totmeasured = 0;
//  
//    /// unsigned int notdead = 0; //TEST
//  
//    for (unsigned int i = 0; i < fData3D->fNTel3D; i++) {
//      for( unsigned int j = 0; j < fData3D->fNpix3D[i]; j++ ) {
//  
//        //// skip non-dead pixels (low and high gain) //// 
//        //if( fDead[j] ) continue;
//        //if( fHiLo[j] && fLowGainDead[j] ) continue;
//  
//        tempmodel = calcModel3D( i, j );
//  
//        //totmodel += tempmodel;
//        //totmeasured += ( fSums[j] / fData3D->fDCPE ) ;
//  
//        if( getRunParameter()->fUseDisplayModel3D ) {
//  	/// if( fData3D->fClean3D )
//  	//// convert into dc from pe ////
//        }
//        ///notdead += 1;
//      }
//  
//      ///cout<<"T"<<i+1<<" p"<<notdead<<": model, meas, u: "<<totmodel<<" "<<totmeasured<<" "<<u<<endl;
//      ///notdead = 0;
//      ///totmodel = 0;
//      ///totmeasured = 0;
//    }
//  
//    /////double u = log( totmodel / totmeasured );
//    ////cout<<"model, meas, u: "<<totmodel<<" "<<totmeasured<<" "<<u<<endl;
//  
//    ///fData3D->fuQuality3D = log( totmodel / totmeasured );
//  
// }
// 
//////////////

double V1FitModel3D::calcModel3D( unsigned int iTel, unsigned int iPix )
{
  double tSzen = (90. - fData3D->fSel3D) * (TMath::Pi()/180.); // in radians

////////////////////////////////////////////////////////////////
//// Lemoine-Goumard 2006, equation 5 //////////////////////////

  //// get Bs, Bp, u, epsilon, deltaB, sigmaU, sigmaD ///////

  //cout<< "fData3D->fpX3D, fData3D->fpY3D, fData3D->fpZ3D: " << fData3D->fpX3D[iTel][iPix] <<", "<< fData3D->fpY3D[iTel][iPix] <<", "<< fData3D->fpZ3D[iTel][iPix] <<endl;

  /////if(i>300 && i<310 && k>1) cout<<"tel "<<k+1<<", pix "<<i+1<<", xBx = "<<fData3D->fxB3D[0][k] <<", xBy = "<<fData3D->fxB3D[1][k] <<", xBz = "<<fData3D->fxB3D[2][k] << endl;

  double Bs = fData3D->dot3D( fData3D->fxB3D[0][iTel], fData3D->fxB3D[1][iTel], fData3D->fxB3D[2][iTel], fData3D->fs3D[0], fData3D->fs3D[1], fData3D->fs3D[2] );
  double Bp = fData3D->dot3D( fData3D->fxB3D[0][iTel], fData3D->fxB3D[1][iTel], fData3D->fxB3D[2][iTel], fData3D->fpX3D[iTel][iPix], fData3D->fpY3D[iTel][iPix], fData3D->fpZ3D[iTel][iPix] );

  double u = fData3D->dot3D( fData3D->fs3D[0], fData3D->fs3D[1], fData3D->fs3D[2], fData3D->fpX3D[iTel][iPix], fData3D->fpY3D[iTel][iPix], fData3D->fpZ3D[iTel][iPix] );

  //cout<<"Bs, Bp, u: "<<Bs<<", "<<Bp<<", "<<u<<endl;

  double epsilon = acos( u ); // in radians

  ///////fepsilon3D[iTel][iPix] = acos( u ); // in radians

  ///Note: deltaB gives nan error (negative in sqrt) ///
  // use sqare of deltaB //

  double deltaBsq = ( (fData3D->fxB3D[0][iTel]*fData3D->fxB3D[0][iTel]) + (fData3D->fxB3D[1][iTel]*fData3D->fxB3D[1][iTel]) + (fData3D->fxB3D[2][iTel]*fData3D->fxB3D[2][iTel]) ) - (Bp*Bp); 

  double sigmaU = sqrt( (fData3D->fsigmaT3D *fData3D->fsigmaT3D *u*u) + (fData3D->fsigmaL3D *fData3D->fsigmaL3D *(1 - (u*u) ) ) );

  double sigmaD = sqrt( (fData3D->fsigmaL3D*fData3D->fsigmaL3D) - (fData3D->fsigmaT3D*fData3D->fsigmaT3D) );

  ////cout<<"(tel="<<k+1<<", pix="<<i+1<<") xB*xB, Bp*Bp, deltaBsq: "<< (fData3D->fxB3D[0][iTel]*fData3D->fxB3D[0][iTel]) + (fData3D->fxB3D[1][iTel]*fData3D->fxB3D[1][iTel]) + (fData3D->fxB3D[2][k]*fData3D->fxB3D[2][k]) <<", "<< Bp*Bp <<", "<< deltaBsq << endl;

  ///cout<<"epsilon, deltaB, sigmaU, sigmaD: "<< epsilon <<", "<< deltaBsq <<", "<< sigmaU <<", "<< sigmaD << endl;

   //// Eq.5 (first term) ///////

  double Nc = exp( fData3D->fNc3D ); // get Nc from ln(Nc)
  //double Nc = fData3D->fNc3D; // Nc

  double Cx = -( ( (fData3D->fsigmaL3D*fData3D->fsigmaL3D *Bp) - (sigmaD*sigmaD *u *Bs) ) / (sigmaU * fData3D->fsigmaT3D * fData3D->fsigmaL3D ) );

  double C = 1. - freq( Cx );

  ////double Eq5t1 = (fData3D->fNc3D * C) / (2.*TMath::Pi() * sigmaU * fData3D->fsigmaT3D); 

  double Eq5t1 = (Nc * C) / (2.*TMath::Pi() * sigmaU * fData3D->fsigmaT3D); 

  //// if(i>300 && i<310 && k>1) cout<<"(tel="<<k+1<<", pix="<<i+1<<") C, Eq5t1: "<< C <<", "<< Eq5t1 << endl;

  //// Eq.5 (second term) ///////

  double Eq5t2p1 = deltaBsq/(fData3D->fsigmaT3D*fData3D->fsigmaT3D);
  double Eq5t2p2 = (sigmaD*sigmaD)/(fData3D->fsigmaT3D*fData3D->fsigmaT3D * sigmaU*sigmaU);
  double Eq5t2p3 = ((u*Bp) - Bs)*((u*Bp) - Bs);

  double Eq5t2 = Eq5t2p1 - (Eq5t2p2 * Eq5t2p3);

  ////if(i>300 && i<310 && k>1) cout<<"(tel="<<k+1<<", pix="<<i+1<<") Eq5t2p1, Eq5t2p2, Eq5t2p3: "<< Eq5t2p1 <<", "<< Eq5t2p2 <<", "<< Eq5t2p3 << endl;

  ////  Eq.5 ///////

  double Eq5 = Eq5t1 * exp( -0.5 * Eq5t2 );

  //cout<<"Eq5t1, Eq5t2, Eq5: "<< Eq5t1 <<", "<< Eq5t2 <<", "<< Eq5 <<endl;

  //////////////////////////////////////////
  //// I(epsilon) probability of Cherenkov photon per unit solid angle ////

  /////  (FIX THIS) /////
  //double nZ = 1.00009; // index of refraction of air (fixed for altitude z = 10 km)
  //double eta = acos( 1./ nZ ); // (radians) maximal Cherenkov angle at altitude of emission

  /// Lemoine ////
  double eta = 0.015 * sqrt( cos( tSzen ) );

  //// TEST using shower height ////
  ///double eta = ffun->Eval( fData3D->fSmax3D / 1000. ); 
  ///eta = eta * (TMath::Pi()/180.); // in radians
  /////////////

  double Ie = 0; // I(epsilon)

  double K = 1./(9*TMath::Pi()*(eta*eta)); // normalization

  if( epsilon < eta ) Ie = K; 

  else if( epsilon > eta ) Ie = K * (eta/epsilon) * exp( -((epsilon - eta)/(4*eta)) );
 

  ///double tIe = Ie( iTel, iPix );

  ////////////////////////////////////////////

  double qterm1 = fData3D->fMarea3D[iTel] * fData3D->fomegapix3D[iTel][iPix] * fData3D->fcosptheta3D[iTel][iPix];

  ///cout<<"area, omega, cosptheta: "<<fData3D->fMarea3D[iTel]<<", "<<fData3D->fomegapix3D[iTel][iPix]<<", "<<fData3D->fcosptheta3D[iTel][iPix] <<endl;

  /////fSignalModel3D[iTel][iPix] = qterm1 * tIe * Eq5;

  double qout = qterm1 * Ie * Eq5;

  /// cout<< "pix3D: "<< qout << endl;

  return qout; //Original
  ////// return Eq5; //TEST ONLY Eq5
}

//// standard normal cumulative distribution function (CDF) ////

double V1FitModel3D::freq(double x)
{
  // Handbook of Mathematical Functions by Abramowitz and Stegun //

  // constants
  double a1 =  0.254829592;
  double a2 = -0.284496736;
  double a3 =  1.421413741;
  double a4 = -1.453152027;
  double a5 =  1.061405429;
  double p  =  0.3275911;

  // Save the sign of x
  int sign = 1;
  if (x < 0) sign = -1;

  x = fabs(x)/sqrt(2.0);
  
  // A&S formula 7.1.26
  double t = 1.0/(1.0 + p*x);
  double y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);

  return 0.5*(1.0 + sign*y);
}

