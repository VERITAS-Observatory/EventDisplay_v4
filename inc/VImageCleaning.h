//! VImageCleaning   basic cleaning routines 

#ifndef VIMAGECLEANING_H
#define VIMAGECLEANING_H

#include <VEvndispData.h>
#include <TGraphErrors.h>
using namespace std;

class VImageCleaning
{
    private:

        VEvndispData *fData;

	void cleanImageWithTiming( double, double, double, double, double, int, int, bool );                // time image cleaning
        void fillImageBorderNeighbours();
        void recoverImagePixelNearDeadPixel();
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

// NN image cleaning
        bool  kInitNNImageCleaning;
        unsigned int fIPRdim;
        TObjArray* fProb4nnCurves;
        TObjArray* fProb3nnrelCurves;
        TObjArray* fProb2plus1Curves;
        TObjArray* fProb2nnCurves;
        TObjArray* fProbBoundCurves;

        int   VALIDITY[VDST_MAXCHANNELS];      //
        int   VALIDITYBOUND[VDST_MAXCHANNELS]; //
        int   VALIDITYBUF[VDST_MAXCHANNELS];   //
        unsigned int nRings;
        float CoincWinLimit;                   //ns
        double fNSBscale;
	double fSPhePulseFWHM;
	double fIntegWindow;  // ns
        float fFADCtoPhe[VDST_MAXTELTYPES];
        float fFADCsampleRate[VDST_MAXTELTYPES];

        float INTENSITY[VDST_MAXCHANNELS];     //
        float TIMES[VDST_MAXCHANNELS];         //
        float **IPR;                           // IPR[TelType][ScanDim] scan. Not used TelType==0 is filled with DT values

        int   LocMin(int n, float *ptr, float &min);
        int   LocMax(int n, float *ptr, float &max);

// main functions
        bool  BoundarySearch(int type, float thresh, TF1* fProbCurve, float refdT, int refvalidity, int idx);
        int   NNGroupSearchProbCurve(int type, TF1* fProbCurve, float PreCut);
        int   NNGroupSearchProbCurveRelaxed(int type, TF1* fProbCurve, float PreCut);
        void  ScaleCombFactors(int type, float scale);
        void  ResetCombFactors(int type);
        float ImageCleaningCharge(int type, int& ngroups);
        void  cleanNNImageFixed();
        void  cleanNNImagePedvars();
        bool  InitNNImageCleaning();
        int   getTrigSimTelType(unsigned int fTelType);
        void  DiscardTimeOutlayers(int type);
        void  DiscardIsolatedPixels(int type);
        void  FillIPR(unsigned int teltype);
        void  FillPreThresholds(TGraph* gipr, float NNthresh[5]); // defines pre-search thresholds for nn-groups (below this threshold group is not searched)
        TGraphErrors* GetIPRGraph(unsigned int teltype, float ScanWidow);
        void  CalcSliceRMS();
// MS
        void cleanTriggerFixed( double hithresh, double lothresh );

        void addImageChannel( unsigned int iChannel );                      // add this pixel to image
        void removeImageChannel( unsigned int iChannel );                   // remove this pixel from image
        void resetImageChannel( unsigned int iChannel );                    // reset this pixel to standard value

// trace correlation cleaning (AMc)
	void cleanImageTraceCorrelate( double sumThresh, double corrThresh, double pixThresh);
};
#endif
