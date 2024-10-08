//! VImageParameterCalculation   calculation of Hillas Parameters

#ifndef VImageParameterCalculation_H
#define VImageParameterCalculation_H

#include <cmath>
#include <iostream>
#include <valarray>
#include <vector>

#include "VDetectorGeometry.h"
#include "VEvndispData.h"
#include "VHoughTransform.h"
#include "VImageParameter.h"

#include "TError.h"
#include "TMath.h"
#include "TMinuit.h"
#include "TObject.h"
#include <TH2.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TF1.h>

//Include the GSL elliptical integral header file if GSL is installed
//This is for the impact parameter corrected muon size calculation
#ifndef NOGSL
#include "gsl/gsl_sf_ellint.h"
#endif

using namespace std;

// global functions and pointers, but how to define them nonglobal without handstands?
extern void get_LL_imageParameter_2DGauss( Int_t&, Double_t*, Double_t&, Double_t*, Int_t );
extern TMinuit* fLLFitter;

class VImageParameterCalculation : public TObject
{
    private:

        bool fDebug;
        bool fLLDebug;

        VDetectorGeometry* fDetectorGeometry;     //< the detector geometry
        VImageParameter* fParGeo;                 //!< image parameters (geo.)
        bool fboolCalcGeo;                        //!< switch to show if there was a geometrical analysis
        bool fboolCalcTiming;                     //!< switch to show if there was a timing analysis
        VImageParameter* fParLL;                      //!< image parameters (log likelihood)

        VEvndispData* fData;

        vector<double> fll_X;                     //!< data vector for minuit function (x-coordinate of pmt)
        vector<double> fll_Y;                     //!< data vector for minuit function (y-coordinate of pmt)
        vector<double> fll_Sums;                  //!< data vector for minuit function
        vector<double> fll_Pedvars;               //!< data vector for minuit function
        vector<bool> fLLEst;                      //!< true if channel has an estimated sum from the LL fit

        double getFractionOfImageBorderPixelUnderImage( double, double, double, double, double, double );
        double redang( double angle, double maxI );  //!< reduce angle to interval [0.,maxI]
        void   setImageBorderPixelPosition( VImageParameter* iPar );

        // Hough transform
        VHoughTransform* fHoughTransform;

    public:

        VImageParameterCalculation( unsigned int iShortTree = 0, VEvndispData* iData = 0 );
        ~VImageParameterCalculation();
        vector<bool> calcLL( bool );                                                               //!< calculate image parameters (log like)
        void muonRingFinder();                                                                 //!< fit a single ring to the image to look for muons
        void sizeInMuonRing();                                                                 //! calculate the brightness of the muon ring
        void muonPixelDistribution();                                                          //!< determine the distribution of pixels in the image

        //Impact parameter correction factor for size
        float correctSizeInMuonRing();

        //Hough transform
        void houghInitialization(); 				//Initialize the Hough transform class
        void houghMuonPixelDistribution();          //!< determine the distribution of pixels in the image

        void calcTriggerParameters( vector<bool> fTrigger );                                   //!< MS: calculate trigger-level image parameters
        void calcParameters();                                                                 //!< calculate image parameters (geo.)
        void calcTimingParameters();
        void initMinuit( int );                   //!< initialize minuit
        vector<double>& getPedvars()              //!< return data vector for minuit function
        {
            return fll_Pedvars;
        }
        vector<double>& getLLSums()                 //!< return data vector for minuit function
        {
            return fll_Sums;
        }
        vector<double>& getLLX()                    //!< return data vector for minuit function
        {
            return fll_X;
        }
        vector<double>& getLLY()                    //!< return data vector for minuit function
        {
            return fll_Y;
        }
        VImageParameter* getParameters()              //!< get image parameters
        {
            return fParGeo;
        }
        VImageParameter* getLLParameters()            //!< get image parameters from loglikelihood
        {
            return fParLL;
        }
        bool getboolCalcGeo()                     //!< get image parameters calculated flag
        {
            return fboolCalcGeo;
        }
        bool getboolCalcTiming()                  //!< get timing parameters calculated flag
        {
            return fboolCalcTiming;
        }
        VDetectorGeometry* getDetectorGeo()       //!< get the detector geometry
        {
            return fDetectorGeometry;
        }
        VDetectorGeometry* getDetectorGeometry()  //!< get the detector geometry
        {
            return fDetectorGeometry;
        }
        //!< fill image/border list (optional)
        void fillImageBorderPixelTree();
        //!< return value of 2d-gaus at channel iChannel
        double getFitValue( unsigned int iChannel, double, double, double, double, double, double );
        //!< set the detector geometry
        void setDebug( bool iB = true )
        {
            fDebug = iB;
        }
        void setDetectorGeometry( VDetectorGeometry* iDet )
        {
            fDetectorGeometry = iDet;
        }
        void setParameters( VImageParameter* iP )     //!< set pointer to parameter class
        {
            fParGeo = iP;
        }
        void setParametersLogL( VImageParameter* iP ) //!< set pointer to parameter class
        {
            fParLL = iP;
        }
};
#endif
