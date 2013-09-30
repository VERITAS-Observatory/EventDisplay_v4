//! V1Model3D class for simple 3D-reconstruction of showers

#ifndef V1MODEL3D_H
#define V1MODEL3D_H

#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>

#include <valarray>
#include <vector>

#include "TF1.h" // Nc vs Size

//// GSL ////
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlin.h> //Levenberg-Marquardt 
#include <gsl/gsl_blas.h> //Levenberg-Marquardt linear algebra
#include <gsl/gsl_integration.h>
#include <gsl/gsl_interp.h>

//// Minuit ////
#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"

#include "VEvndispData.h"
#include <VGrIsuAnalyzer.h>
#include "V1FitModel3D.h"  
#include "V1Model3DData.h" 

/// TEST external cleaning and height ///
// #include <VImageCleaning.h>
// #include "VEmissionHeightCalculator.h"

using namespace std;

class V1Model3D : public VEvndispData, public VGrIsuAnalyzer
{
 private:

  bool fInitialized3D;              // true after initialization

  VEvndispData *fData;              // pointer to data class
  V1Model3DData *fData3D;            // pointer to Model3DData class

  // VImageCleaning* fCleaning; // TEST cleaning
  // VEmissionHeightCalculator *fEmissionHeightCalculator; // TEST height

  TF1 *ffun; // Nc vs Size

  /// GSL ///
  // int Sn3D;
  // double *Sq3D;
  // double *Sped3D;

//// FUNCTIONS ////
//// initialize ////
  void getDetector( unsigned int iTel ); // get telescope locations, pixel positions, etc.

//// calculate the telescope pointing and set up the coordinate systems ////
  void calcPointing(); // calculate the telescope pointing
  void calcCoord();    // calculate sky unit base vectors in ground coordinate

//// calculate the P unit vectors (line of sight of each pixel) ////
  void calcPvector( unsigned int iTel, unsigned int iPix );

//// get the model starting parameters from the Hillas analysis ////
  void calcStartParameters(); // get the model starting parameters from the Hillas analysis
  void calcEmissionHeight(); // calculate the emission height of the shower

  void calcHeightMethod2(); // TEST calculate the emission height of the shower

  double getTelescopeDistanceSC( unsigned int iTel1, unsigned int iTel2, double az, double z ); // used in the emission height calculation
  double imageDistance( double c1x, double c2x, double c1y, double c2y ); // used in the emission height calculation
  double line_point_distance(double x1, double y1, double z1, double el, double az, double x, double y, double z); // used in the emission height calculation

  void setSelectedPixels(); //// TEST

  // void setDataGSL(); //GSL

  void getHillasParameters();

//// output of model analysis ////
  void getModelDirection(); // get the model direction in the sky frame
  void writeParameters3D(); // write the model parameters to the output root file 
  void filterEvents(); // for testing

  void doTestModel3D(); //TEST

    public:

        V1Model3D();
        ~V1Model3D();

        void doModel3D();
	void doFitGSL3D();
	void doFitMinuit3D();
	bool checkModel3D();
};

// // NEW for GSL //
// struct stData3D
// {
//   size_t n3D;
//   double *q3D;
//   double *ped3D;
// };

#endif
