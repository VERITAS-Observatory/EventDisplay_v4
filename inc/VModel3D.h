//! VModel3D class for 3D reconstruction of showers

#ifndef VMODEL3D_H
#define VMODEL3D_H

#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>

#include <valarray>
#include <vector>

#include "VEvndispData.h"

#include "VModel3DData.h"
#include "VModel3DFn.h"
#include "VModelLnL.h"

#include "VEmissionHeightCalculator.h" ///make clean fails (but works if compiled with removed and then added in), something to do with VStatistics ? ///

using namespace std;

class VModel3D : public VEvndispData
{

 private:

  VEvndispData *fData;      // data class

  VEmissionHeightCalculator *fEmissionHeightCalculator; // shower height class

  VModel3DData *fData3D;   // data3D class
  VModel3DFn *fModel3DFn;  // model3D function class
  VModelLnL *fModelLnL;    // model lnl class

  bool fInitialized3D;      // true after initialization

  /// FUNCTIONS ///
  void doFit();          // do LnL fit

  void readLnLTable();   // read lookup table for likelihood
  void getDetector();    // get telescope locations, pixel positions, etc.
  void calcPointing();   // calculate the telescope pointing
  void calcCoord();      // calculate sky unit base vectors in ground coordinate
  void calcPvector();    // calculate line of sight for each pixel
  void calcStartParameters(); // get model starting parameters from the Hillas analysis
  void setSelectedPixels();   // pixels used in Model3D analysis (if different)
  void startModel3D();   // model with start values, proceed with fit or not
  void fillMu( const vector<double>& vp ); // fill mu for each telescope
  void fillMuDisplay();  // fill mu for each telescope for display vectors
  void writeParameters3D();   // write to shower parameters and mu for display
  void calcModelDirection();   // calculate best-fit model direction

 public:
  
  VModel3D();
  ~VModel3D();

  void doModel3D();       // do Model3D analysis
  void createLnLTable();  // create lookup table for likelihood
  void fillInit3D();      // fill initialized values
};
#endif