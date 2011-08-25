//! VImageCleaning   basic cleaning routines (used by ...)

#ifndef VIMAGECLEANING_H
#define VIMAGECLEANING_H

#include <VEvndispData.h>

using namespace std;

class VImageCleaning
{
    private:

        VEvndispData *fData;

        void fillImageBorderNeighbours();
        void removeIsolatedPixels();
	void mergeClusters(); 
	void printDataError( string iFunctionName );
	void removeSmallClusters( int ); 

    public:
        VImageCleaning( VEvndispData *iData = 0 );
        ~VImageCleaning() {}

                                                  //!< fixed tailcut cleaning (needed by VDisplay)
        void cleanImageFixed(double iimagethresh, double iborderthresh );
                                                  //!< fixed tailcut cleaning (needed by VDisplay)
        void cleanImageFixed(double iimagethresh, double iborderthresh, double brightthresh );
                                                  //!< signal to noise tailcut cleaning
        void cleanImagePedvars(double,double, bool, bool );
                                                  //!< signal to noise tailcut cleaning
        void cleanImagePedvars(double,double, double, bool, bool );
                                                  //!< time image cleaning (signal-to-noise)
	void cleanImagePedvarsWithTiming(double, double, double, double, double, int, int );
        void cleanImage_clusterCleaning( double threshold_clustersize );
                                                  // MS
        void cleanTriggerFixed( double hithresh, double lothresh );

                                                  //!< add this pixel to image
        void addImageChannel( unsigned int iChannel );
                                                  //!< remove this pixel from image
        void removeImageChannel( unsigned int iChannel );
                                                  //!< reset this pixel to standard value
        void resetImageChannel( unsigned int iChannel );
};
#endif
