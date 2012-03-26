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
        void cleanImage_clusterCleaning( double threshold_clustersize );

// NN image cleaning
        bool  kInitNNImageCleaning;
        
        TF1*  fProbCurve4nn;
        TF1*  fProbCurve3nnrel;
        TF1*  fProbCurve2plus1;
        TF1*  fProbCurve2nn;
        TF1*  fProbCurveBound;
        int   VALIDITY[VDST_MAXCHANNELS];      //
        int   VALIDITYBOUND[VDST_MAXCHANNELS]; //
        int   VALIDITYBUF[VDST_MAXCHANNELS];   //
        unsigned int nRings;
        float CoincWinLimit;                   //ns
        float INTENSITY[VDST_MAXCHANNELS];     //
        float TIMES[VDST_MAXCHANNELS];         //

        int   LocMin(int n, float *ptr, float &min);
        int   LocMax(int n, float *ptr, float &max);

        // main functions
        bool  BoundarySearch(int type, float thresh, TF1* fProbCurve, float refdT, int refvalidity, int idx);
        int   NNGroupSearchProbCurve(int type, TF1* fProbCurve, float PreCut);
        int   NNGroupSearchProbCurveRelaxed(int type, TF1* fProbCurve, float PreCut);
        float ImageCleaningCharge(int type, float NSBscale, int& ngroups);
        void  cleanNNImageFixed();
        void  cleanNNImagePedvars();
        bool  InitNNImageCleaning();


// MS
        void cleanTriggerFixed( double hithresh, double lothresh );

        void addImageChannel( unsigned int iChannel );                      // add this pixel to image
        void removeImageChannel( unsigned int iChannel );                   // remove this pixel from image
        void resetImageChannel( unsigned int iChannel );                    // reset this pixel to standard value
};
#endif
