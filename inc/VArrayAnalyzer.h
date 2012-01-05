//! VArrayAnalyzer class for analyzing VERITAS data (full array analysis)
// Revision $Id: VArrayAnalyzer.h,v 1.18.2.2.4.3.10.1.10.1.2.3.2.2.2.1.2.5.2.3.4.1.2.2 2011/04/21 10:44:43 gmaier Exp $
#ifndef VARRAYANALYZER_H
#define VARRAYANALYZER_H

#include "TMath.h"

#include "VEvndispData.h"
#include "VDispAnalyzer.h"
#include "VGrIsuAnalyzer.h"
#include "VEffectiveAreaCalculatorMCHistograms.h"
#include "VMLPAnalyzer.h"
#include "VShowerParameters.h"
#include "VSkyCoordinatesUtilities.h"

#include <bitset>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

class VArrayAnalyzer : public VEvndispData, public VGrIsuAnalyzer
{
    private:

        bool fDebug;
        bool fInitialized;                        //!< true after initialization
//   vector< bool > fTelIDImageSelected;   //!< true, if image of telescope is used for the reconstruction
        vector< int > fSelectNtubes;              //!< number of tubes needed to use image for shower reconstruction
        vector< double > fSelectDist;             //!< maximum distance of image in camera to be included in shower reconstruction
        vector< double > fSelectAlpha;            //!< maximum alpha of image in camera to be included in shower reconstruction

        vector< VDispAnalyzer* > fDispAnalyzer;

        vector< unsigned int > frcs_img_req;      //!< minimum number of images required per reconstruction method
        double fmin_ang;                          //!< minum angle for reconstruction with two images only

        vector< double > fMeanPointingMismatch;   //!< mean pointing mismatch between eventdisplay and vbf (per telescope)
        vector< double > fNMeanPointingMismatch;

        float adjustAzimuthToRange( float az );
        void calcShowerDirection_and_Core();      //!< calculate shower core and direction
        void checkPointing();                     //!< check if pointing directions
        void prepareforCoreReconstruction( unsigned int iMeth, float xs, float ys );
        void prepareforDirectionReconstruction( unsigned int iMethIndex, unsigned int iReconstructionMethod );
        bool fillSimulationEvent();
        bool fillShowerDirection( unsigned int iMeth, float xoff, float yoff, float stds );
                                                  //!< fill shower core results into VEvndispData
        bool fillShowerCore( unsigned int iMeth, float ximp, float yimp );
        void getNumImages();                      //!< calculate number if images
        double getMeanPointingMismatch( unsigned int iTel);
        void initEvent();                         //!< reset vectors, etc. (called for each event)
        int  rcs_method_0( unsigned int );        //!< GrIsu reconstruction method 1(!)
        int  rcs_method_3( unsigned int );
        int  rcs_method_4( unsigned int );
        int  rcs_method_5( unsigned int, unsigned int );
        int  rcs_method_7( unsigned int );
        int  rcs_method_8( unsigned int );
        int  rcs_method_9( unsigned int );
        float recalculateImagePhi( double, double );
        void selectShowerImages( unsigned int );  //!< select shower images to be used in determinate of shower coordinates
                                                  //!< transform telescope positions into shower coordinates
        void transformTelescopePosition( int iTel, float ize, float iaz, bool i_MC );

        double angDist( double, double, double, double );

// temporary variables needed for array reconstruction
        vector< unsigned int > telID;
        vector< float > x;
        vector< float > y;
        vector< float > w;
        vector< float > l;
        vector< float > m;
        vector< float > phi;
        vector< float > sinphi;
        vector< float > cosphi;
        vector< float > width;
        vector< float > length;
        vector< float > asym;
        vector< float > loss;
        vector< float > dist;
        vector< float > pedvar;
	vector< float > tgrad;
        vector< float > az;
        vector< float > ze;
	vector<ULong64_t> teltype;

        vector< float > xtelnew;
        vector< float > ytelnew;
        vector< float > ztelnew;

    public:

        VArrayAnalyzer();
        ~VArrayAnalyzer();
        void doAnalysis();
        void initAnalysis();
        void initOutput();
        void terminate();
//        void initOuput();                         //!< check for output file,
        void initTree();
};
#endif
