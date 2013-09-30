//! V1FitModel3D class is the fit for V1Model3D

#ifndef V1FITMODEL3D_H
#define V1FITMODEL3D_H

#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>

#include <valarray>
#include <vector>

#include "TF1.h" // cherenkov angle

//// GSL //////
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlin.h> //Levenberg-Marquardt 
#include <gsl/gsl_blas.h> //Levenberg-Marquardt linear algebra
#include <gsl/gsl_integration.h>
#include <gsl/gsl_interp.h>

#include "V1Model3D.h"  
#include "V1Model3DData.h" 

using namespace std;

class V1FitModel3D
{
 private:

  V1Model3DData *fData3D;

  ///////////////////

  /// TEST cherenkov angle //
  double fZenRad; // zenith angle in radians
  TF1 *ffun; // Cherenkov angle

  // PDF //
  double fGaussLimit; // model signal when PDF becomes Gaussian-like
  double fNstdPoisson; // number of standard deviations for Poisson calculation
  double fsmallNumber; // to avoid divergence when likelihood approaches zero

  ////// FUNCTIONS ///////////////
  void stepPar( int iParID, float iMult );
  double calcPDF3D( double qdata, double qmodel, double ped, double exnoise );
  double calcDerivLL3D( unsigned int iTel, unsigned int iPix, int iParID );
  double freq(double x);

  ///////////////////////

 public:

  V1FitModel3D( V1Model3DData *iData3D );
  ~V1FitModel3D();

  ////void calcTotalModel3D(); // TEST

  /////////// Model Calculation //////////
  static double wrapCalcModel3D(unsigned int iTel, unsigned int iPix );
  double calcModel3D( unsigned int iTel, unsigned int iPix );
  void posFit3D();

  /////////// Minuit //////////
  static double wrapFitMinuit3D(const double *xx );
  double funFitMinuit3D(const double *xx ); 

  /////////// GSL //////////
  static int wrapFitGSL3D( const gsl_vector *v, void *ptr, gsl_vector *f );
  static int wrapDF3D( const gsl_vector *v, void *ptr, gsl_matrix *J );
  static int wrapFDF3D( const gsl_vector *v, void *ptr, gsl_vector *f, gsl_matrix *J );
  int funFitGSL3D( const gsl_vector *v, void *ptr, gsl_vector *f );
  int funDF3D( const gsl_vector *v, void *ptr, gsl_matrix *J );
  int funFDF3D( const gsl_vector *v, void *ptr, gsl_vector *f, gsl_matrix *J );

};
#endif
