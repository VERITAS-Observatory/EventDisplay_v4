//! VGlobalRunParameter global definitions for eventdisplay package
// Revision $Id$

#ifndef VGlobalRunParameter_H
#define VGlobalRunParameter_H

#include <TChain.h>
#include <TSystem.h>
#include <TTree.h>
#include <TROOT.h>

#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>

// HARDWIRED MAXIMUM NUMBER OF TELESCOPES AND CHANNELS, etc.
#define VDST_MAXTELESCOPES  100    // maximum number of telescopes
#define VDST_MAXCHANNELS  12000   // maximum number of channels per telescopes
#define VDST_MAXSUMWINDOW   64    // maximum number of summation windows (=maximum number of samples per FADC trace)
#define VDST_PEDTIMESLICES 5000    // maximum number of time slices for pedestal calculation
#define VDST_MAXRECMETHODS  100    // maximum number of arrayreconstruction method
#define VDST_MAXTIMINGLEVELS 10    // maximum number of timing levels

using namespace std;

class VGlobalRunParameter
{
   private:

   static bool      bReadRunParameter;
   static bool      bDebug;                                         // print debug output

   static string       fObservatory;
   static double       fObservatory_Latitude_deg;
   static double       fObservatory_Longitude_deg;
   static double       fObservatory_Height_m;


// OUTPUT TREE VERSION
// 
// changes from 5 to 6: LTrig  now ULong64_t
//                      ImgSel now ULong64_t
// changes from 6 to 7: introduced list of selected telescopes
// 
// changes from 7 to 8: add MC primary to showerpars
   static unsigned int fEVNDISP_TREE_VERSION;

   static string    fDBServer;                                         // database location

// DIRECTORIES
   static string fEVNDISPAnaDataDirectory;          // directory where all data (calibration, detectorgeometry, ...) is expected and written to (output file)
   static string fVBFRawDataDirectory;              // directory with VERITAS vbf data (vbf files)
   static string fEVNDISPOutputDirectory;                  // output- and result files are written into this directory

   public:

   static string       fEVNDISP_VERSION;                             // EVNDISPLAY VERSION

   VGlobalRunParameter();
   virtual ~VGlobalRunParameter();

   string       getDBServer() const             { return fDBServer; }
   static string getDirectory_EVNDISPAnaData()   { return fEVNDISPAnaDataDirectory; }
   string       getDirectory_EVNDISPCalibrationData() { return fEVNDISPAnaDataDirectory + "/Calibration/"; }
   string       getDirectory_EVNDISPDetectorGeometry() { return fEVNDISPAnaDataDirectory + "/DetectorGeometry/"; }
   string       getDirectory_EVNDISPParameterFiles() { return fEVNDISPAnaDataDirectory + "/ParameterFiles/"; }
   string       getDirectory_VBFRawData()       { return fVBFRawDataDirectory; }
   string       getDirectory_EVNDISPOutput()    { return fEVNDISPOutputDirectory; }
   static string getEVNDISP_VERSION() { return fEVNDISP_VERSION; }
   static float  getEVNDISP_VERSION_FL() { return atof( fEVNDISP_VERSION.substr( 2, 5 ).c_str() ); }
   static unsigned int getEVNDISP_VERSION_UI() { return (unsigned int)( 100.*getEVNDISP_VERSION_FL()); }
   static unsigned int getEVNDISP_TREE_VERSION() { return fEVNDISP_TREE_VERSION; }
   static unsigned int getEVNDISP_TREE_VERSION( TTree* );
   static bool         getEVNDISP_TREE_isShort( TTree* );
   string       getObservatory()                { return fObservatory; }
   double       getObservatory_Height_m()       { return fObservatory_Height_m; }
   double       getObservatory_Latitude_deg()    { return fObservatory_Latitude_deg; }
   double       getObservatory_Longitude_deg()    { return fObservatory_Longitude_deg; }
   void         printGlobalRunParameter();
   bool         readRunparameterFile( string iFile );
   bool         setDirectories();
   void         setDirectory_EVNDISPOutput( string iDir ) { fEVNDISPOutputDirectory = iDir; }
   bool         update( TChain *ic );

   ClassDef(VGlobalRunParameter,6);
};

#endif
