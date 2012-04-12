/*  \class VImageCleaningRunParameter


*/

#include "VImageCleaningRunParameter.h"

VImageCleaningRunParameter::VImageCleaningRunParameter()
{
    fTelID = 0;

    fUseFixedThresholds = false;
    fImageCleaningMethod = 0;

    fimagethresh = 5.0;
    fborderthresh = 2.5;
    fbrightnonimagetresh = 2.5;

// time cluster cleaning
    ftimecutpixel = 0.5;   //HP
    ftimecutcluster = 2.0; //HP
    fminpixelcluster = 3;  //HP
    floops = 2;            //HP
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
    else                      cout << " (signal-to-noise cleaning thesholds)" << endl;
    if( getImageCleaningMethodIndex() == 1 ) 
    {
	cout << "\t Tpixel/Tcluster/nMin/nLoops \t" << ftimecutpixel << "/" << ftimecutcluster 
	     << "/" << fminpixelcluster << "/" << floops << endl;
    }
}



string VImageCleaningRunParameter::getImageCleaningMethod()
{
   if( fImageCleaningMethod == 1 )      return "TIMECLUSTERCLEANING";
   else if( fImageCleaningMethod == 2 ) return "TIMENEXTNEIGHBOUR";

   return "TWOLEVELCLEANING";
}

bool VImageCleaningRunParameter::setImageCleaningMethod( string iMethod )
{
   if( iMethod == "TWOLEVELCLEANING" )         fImageCleaningMethod = 0;
   else if( iMethod == "TIMECLUSTERCLEANING" ) fImageCleaningMethod = 1;
   else if( iMethod == "TIMENEXTNEIGHBOUR" )   fImageCleaningMethod = 2;
   else return false;

   return true;
}
