//! VImageCleaning   basic cleaning routines (used by ...)

#ifndef VIMAGECLEANING_H
#define VIMAGECLEANING_H

#include <VEvndispData.h>

using namespace std;

class VImageCleaning
{
    private:

        VEvndispData *fData;

	void cleanImageWithTiming( double, double, double, double, double, int, int, bool );                // time image cleaning
        void fillImageBorderNeighbours();
        void removeIsolatedPixels();
	void mergeClusters(); 
	void printDataError( string iFunctionName );
	void removeSmallClusters( int ); 

    public:

        VImageCleaning( VEvndispData *iData = 0 );
       ~VImageCleaning() {}

// tailcut cleaning
        void cleanImageFixed(   double iimagethresh, double iborderthresh, double brightthresh = -999. ); 
        void cleanImagePedvars( double hithresh, double lothresh, double brightthresh );                 

// time cluster cleaning
	void cleanImageFixedWithTiming( double, double, double, double, double, int, int );             
	void cleanImagePedvarsWithTiming(double, double, double, double, double, int, int );           

// 
        void cleanImageFixedMaxim();
        void cleanImagePedvarsMaxim();

// obsolete?
        void cleanImage_clusterCleaning( double threshold_clustersize );

// MS
        void cleanTriggerFixed( double hithresh, double lothresh );

        void addImageChannel( unsigned int iChannel );                      // add this pixel to image
        void removeImageChannel( unsigned int iChannel );                   // remove this pixel from image
        void resetImageChannel( unsigned int iChannel );                    // reset this pixel to standard value
};
#endif
