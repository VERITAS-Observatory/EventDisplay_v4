/*  \class VImageCleaningRunParameter

     data class for parameters for image cleaning

*/

#include "VImageCleaningRunParameter.h"

VImageCleaningRunParameter::VImageCleaningRunParameter()
{
    fTelID = 0;

    fUseFixedThresholds = false;
    fImageCleaningMethod = 0;

// default two-level threshold cleaning
    fimagethresh = 5.0;
    fborderthresh = 2.5;
    fbrightnonimagetresh = 2.5;

// time cluster cleaning
    ftimecutpixel = 0.5;   //HP
    ftimecutcluster = 2.0; //HP
    fminpixelcluster = 3;  //HP
    floops = 2;            //HP

// Trace Correlation Cleaning
    fCorrelationCleanBoardThresh = 1.0; //AMc S/N ratio of 1
    fCorrelationCleanCorrelThresh = 0.75; //AMc Sample correlation coefficient of 0.75
    fCorrelationCleanNpixThresh = 15;  //AMc Images whose number of pixels is above this value will skip correlation cleaning
}

bool VImageCleaningRunParameter::initialize()
{
   return true;
}

void VImageCleaningRunParameter::print()
{
    cout << "\t cleaning method \t \t" << getImageCleaningMethod() << " (" << getImageCleaningMethodIndex() << ")" << endl;
    cout << "\t image/border\t" << fimagethresh << "/" << fborderthresh;
    if( fUseFixedThresholds ) cout << " (fixed cleaning thresholds)" << endl;
    else                      cout << " (signal-to-noise cleaning thresholds)" << endl;
    if( getImageCleaningMethodIndex() == 1 ) 
    {
	cout << "\t Tpixel/Tcluster/nMin/nLoops \t" << ftimecutpixel << "/" << ftimecutcluster 
	     << "/" << fminpixelcluster << "/" << floops << endl;
    }
    
    if( getImageCleaningMethodIndex() == 3 )
    {
      cout << "\t Using trace correlation cleaning: " << fCorrelationCleanBoardThresh << "/";
      cout << fCorrelationCleanCorrelThresh << "/" << fCorrelationCleanNpixThresh << "\t";
      cout << "BorderThresh/CorrelationThresh/MaxPixThresh" << endl;
    }
}


string VImageCleaningRunParameter::getImageCleaningMethod()
{
   if( fImageCleaningMethod == 1 )      return "TIMECLUSTERCLEANING";
   else if( fImageCleaningMethod == 2 ) return "TIMENEXTNEIGHBOUR";
   else if( fImageCleaningMethod == 3 ) return "TWOLEVELANDCORRELATION";

   return "TWOLEVELCLEANING";
}

bool VImageCleaningRunParameter::setImageCleaningMethod( string iMethod )
{
   if( iMethod == "TWOLEVELCLEANING" )              fImageCleaningMethod = 0;
   else if( iMethod == "TIMECLUSTERCLEANING" )      fImageCleaningMethod = 1;
   else if( iMethod == "TIMENEXTNEIGHBOUR" )        fImageCleaningMethod = 2;
   else if( iMethod == "TWOLEVELANDCORRELATION" )   fImageCleaningMethod = 3;
   else return false;

   return true;
}
