//! VImageParameterCalculation   calculation of Hillas Parameters

#ifndef VImageParameterCalculation_H
#define VImageParameterCalculation_H

#include <cmath>
#include <iostream>
#include <valarray>
#include <vector>

#include <VDetectorGeometry.h>
#include <VImageParameter.h>
#include <VEvndispData.h>

#include "TError.h"
#include "TMath.h"
#include "TMinuit.h"
#include "TObject.h"
#include <TH2.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TF1.h>

using namespace std;

// global functions and pointers, but how to define them nonglobal without handstands?
extern void get_LL_imageParameter_2DGauss( Int_t&, Double_t*, Double_t&, Double_t*, Int_t );
extern TMinuit *fLLFitter;

class VImageParameterCalculation : public TObject
{
    private:

        bool fDebug;
        bool fLLDebug;

        VDetectorGeometry *fDetectorGeometry;     //< the detector geometry
        VImageParameter *fParGeo;                     //!< image parameters (geo.)
        bool fboolCalcGeo;                        //!< switch to show if there was a geometrical analysis
        bool fboolCalcTiming;                     //!< switch to show if there was a timing analysis
        VImageParameter *fParLL;                      //!< image parameters (log lik.)

        VEvndispData *fData;

        vector<double> fll_X;                     //!< data vector for minuit function (x-coordinate of pmt)
        vector<double> fll_Y;                     //!< data vector for minuit function (y-coordinate of pmt)
        vector<double> fll_Sums;                  //!< data vector for minuit function
        vector<double> fll_Pedvars;               //!< data vector for minuit function
        vector<bool> fLLEst;                      //!< true if channel has an estimated sum from the LL fit

        double getFractionOfImageBorderPixelUnderImage( vector<bool>, vector<bool>, double, double, double, double, double, double );
        double getMeanSignal( int, VEvndispData* );      //!< calculate signal in a channel from the sum in the surrounding channels
                                                  //!< reduce angle to intervall [0.,maxI]
        double redang( double angle, double maxI );

    public:

        VImageParameterCalculation( unsigned int iShortTree = 0, VEvndispData *iData = 0 );
        ~VImageParameterCalculation();
        vector<bool> calcLL( VEvndispData* iData );      //!< calculate image parameters (log like)
                                                  //!< fit a single ring to the image to look for muons
        void muonRingFinder( valarray<double>, vector<bool>, vector<bool> );
                                                  //! calculate the brightness of the muon ring
        void sizeInMuonRing( valarray<double>, vector<bool>, vector<bool>, vector< unsigned int > );
                                                  //!< determine the distribution of pixels in the image
        void muonPixelDistribution( valarray<double>, vector<bool>, vector<bool> );
                                                  //!< MS: calculate trigger-level image parameters
        void calcTriggerParameters( vector<bool> fTrigger );
                                                  //!< calculate image parameters (geo.)
        void calcParameters( valarray<double>, vector<bool>, vector<bool> );
                                                  //!< calculate image parameters (geo.)
        void calcParameters( valarray<double>, vector<bool>, vector<bool>, vector< bool > );
                                                  //!< calculate image parameters (geo.)
        void calcParameters( valarray<double>, vector<bool>, vector<bool>, vector< bool >, vector< bool >, vector< bool > );
        void calcTimingParameters( valarray<double>, valarray<double>, valarray<double>, vector<bool>, vector<bool>, VEvndispData* iData);
        void initMinuit( int );                   //!< initialize minuit
        vector<double>& getPedvars()              //!< return data vector for minuit function
        {
            return fll_Pedvars;
        }
        vector<double>& getSums()                 //!< return data vector for minuit function
        {
            return fll_Sums;
        }
        vector<double>& getX()                    //!< return data vector for minuit function
        {
            return fll_X;
        }
        vector<double>& getY()                    //!< return data vector for minuit function
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
                                                  //!< return value of 2d-gaus at channel iChannel
        double getFitValue( unsigned int iChannel, double, double, double, double, double, double );
                                                  //!< set the detector geometry
	void setDebug( bool iB = true ) { fDebug = iB; }
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
