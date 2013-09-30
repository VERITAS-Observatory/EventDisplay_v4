//! V1Model3DData data class for V1Model3D and V1FitModel3D

#ifndef V1MODEL3DDATA_H
#define V1MODEL3DDATA_H

#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>

#include <valarray>
#include <vector>

using namespace std;

class V1Model3DData
{

 private:

 public:

  ///////////////// DATA ////////////////////

  bool fDebug3D; // set for debug mode

  bool fGoodModel3D; // if detector config is OK for Model3D analysis
  bool fGoodEvent3D; // if event passes cuts for Model3D analysis

  int fBadNumber3D;  // bad number 

  unsigned int fNdim3D; // dimensional space (x,y,z)

  double fexNoise3D; // HARD-WIRED 
  double fDCPE;  // HARD-WIRED (need implementation)

  unsigned int fNTel3D; // total number of telescopes
  vector<unsigned int> fNpix3D; // total number of pixels for each telescope

  unsigned int fNtotPixel3D; // total number of selected pixels in full array

//// telescope configuration parameters ////
  // for each telescope
  vector< vector< double > > flocTel3D; // location of telescope on ground
  vector<double> fMarea3D;              // mirror area (m^2) 
  // for each telescope and pixel
  vector< vector< double > > fomegapix3D; // solid angle for each telescope and pixel

//// pointing for each telescope (sub-array pointing not implemented) ////
  vector<double> fTze3D; // zenith
  vector<double> fTaz3D; // azimuth
  vector<double> fT3D;   // "T" vector of telescope pointing 
  // for each telescope and pixel
  vector< vector< double > > fcosptheta3D;   // cos of angle between pixel center and telescope axis
  vector< vector< double > > fpX3D; // "p" vector (X in ground coordinates)
  vector< vector< double > > fpY3D; // "p" vector (Y in ground coordinates)
  vector< vector< double > > fpZ3D; // "p" vector (Z in ground coordinates)

//// general coordinate frame conversion ////
  // for three dimensional space
  vector<double> fxg3D;   // X ground coordinate (unit base vectors)
  vector<double> fyg3D;   // Y ground coordinate (unit base vectors)
  vector<double> fzg3D;   // Z ground coordinate (unit base vectors)
  // for each telescope
  vector<double> fxsg3D;  // X sky unit base vector in ground coordinate frame 
  vector<double> fysg3D;  // Y sky unit base vector in ground coordinate frame 
  vector<double> fzsg3D;  // Z sky unit base vector in ground coordinate frame 

//// pixel signals (sums), cleaning, and pedvars ////
  // for each telescope and pixel
  vector< vector< bool > > fClean3D;         // pixels passing cleaning
  vector< vector< float > > fMeasuredSum3D; // measured signal
  vector< vector< float > > fPedvar3D;      // measured pedvars
  //  vector< vector< bool > > fDead3D;          // dead pixels

//// Hillas parameters needed for emission height calculation ////
  vector<float> fcen_x3D;
  vector<float> fcen_y3D;
  vector<float> fsize3D;
  vector<float> fcosphi3D;
  vector<float> fsinphi3D;
  vector<float> flength3D;
  vector<float> fwidth3D;

//// points in camera for length and width ////
  vector<float> flengthInX3D;  // point closest to source position
  vector<float> flengthInY3D;  // point closest to source position
  vector<float> flengthOutX3D;  // point furthest to source position
  vector<float> flengthOutY3D;  // point furthest to source position

//// model start parameters ////
  float fStartSel3D;      // model: elevation of shower direction (deg)
  float fStartSaz3D;      // model: azimuth of shower direction (deg)
  float fStartXcore3D;    // model: X shower core in ground coordinates
  float fStartYcore3D;    // model: Y shower core in ground coordinates
  float fStartSmax3D;     // model: height of shower maximum along the shower axis
  float fStartsigmaL3D;   // model: longitudinal (3D-length)
  float fStartsigmaT3D;   // model: transverse (3D-width)
  float fStartNc3D;       // model: number of Cherenkov photons in shower 
 
//// model fit parameters ////
  float fSel3D;      // model: elevation of shower direction (deg)
  float fSaz3D;      // model: azimuth of shower direction (deg)
  float fXcore3D;    // model: X shower core in ground coordinates
  float fYcore3D;    // model: Y shower core in ground coordinates
  float fSmax3D;     // model: height of shower maximum along the shower axis
  float fsigmaL3D;   // model: longitudinal (3D-length)
  float fsigmaT3D;   // model: transverse (3D-width)
  float fNc3D;       // model: number of Cherenkov photons in shower 

//// model step size for fit parameters ////
  float fStepSel3D;    // model: elevation of shower direction (deg)
  float fStepSaz3D;    // model: azimuth of shower direction (deg)
  float fStepXcore3D;  // model: X shower core in ground coordinates
  float fStepYcore3D;  // model: Y shower core in ground coordinates
  float fStepSmax3D;   // model: height of shower maximum along the shower axis
  float fStepsigmaL3D; // model: longitudinal (3D-length)
  float fStepsigmaT3D; // model: transverse (3D-width)
  float fStepNc3D;     // model: number of Cherenkov photons in shower 

//// derived shower direction from model ////
  double fXoffModel3D; // model shower direction
  double fYoffModel3D; // model shower direction

//// geometry for model fit ////
  vector<double> fs3D;   // "s" unit vector along the shower axis   
  vector<double> fxBo3D; // "xB" vector of telescope optical center and shower barycenter B at (0,0,0) ground
  vector< vector< double > > fxB3D; // "xB" vector of telescope optical center and shower barycenter B for each telescope
  // NOT NEEDED? angle between the shower axis and photon direction //
  /// vector< vector< double > > fepsilon3D; // for each telescope and pixel

//// goodness-of-fit ////
  double fGoodness3D; // goodness-of-fit
  double fuQuality3D;

//// FUNCTIONS ////

//// initialize ////
  void initModel3D( unsigned int iNTel3D, vector<unsigned int> &iNpix3D ); 
  void initEventModel3D(); // reset (called for each event)
  void getDetector( unsigned int iTel ); // get telescope locations, pixel positions, etc.

//// vector geometry ////
  void norm3D(double &ax, double &ay, double &az); // normalize the vector
  void cross3D(double ax, double ay, double az, double bx, double by, double bz, double &cx, double &cy, double &cz);  // compute the cross product
  double dot3D(double ax, double ay, double az, double bx, double by, double bz);  // compute the dot product


  V1Model3DData();
  ~V1Model3DData();

};
#endif
