//! VImageAnalyzer    class for analyzing VERITAS data (single telescope analysis)
// Revision $Id: VImageAnalyzer.h,v 1.30.8.2.12.1.4.2.10.1.4.2 2010/03/08 08:01:32 gmaier Exp $
#ifndef VImageAnalyzer_H
#define VImageAnalyzer_H

#include "TFile.h"
#include "TMath.h"
#include "TObject.h"
#include "TTree.h"
#include "TROOT.h"

#include <VImageBaseAnalyzer.h>
#include <VImageParameterCalculation.h>
#include <VSkyCoordinates.h>
#include <VImageCleaning.h>

#include <iostream>
#include <fstream>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <valarray>
#include <vector>

using namespace std;

class VImageAnalyzer : public VImageBaseAnalyzer
{
    private:
        bool fDebug;
	VImageCleaning* fVImageCleaning;                            //!< image cleaning
        VImageParameterCalculation* fVImageParameterCalculation;    //!< image calculation

        bool fInit;

// time since run start
        double fTimeSinceRunStart;                //!< time since run start
        double fTimeRunStart;                     //!< time of first event in run

// temporary vectors for dead pixel smoothing
        vector< unsigned int > savedDead;
        vector< unsigned int > savedDeadLow;
        valarray< double > savedGains;
        valarray< double > savedGainsLow;

        void fillOutputTree();                    //!< fill tree with image parameterisation results
        bool initEvent();                         //! reset image calculation for next event
        void initTrees();                         //!  intitalize output tree
        void printTrace( int i_channel );         //!< print trace information for one channel (debugging)
        void setAnaDir( unsigned int iTel );      //!< set directories in root output file
        void setNTrigger();
        void smoothDeadTubes();                   //!< reduce the effect of dead tubes

    public:
        VImageAnalyzer();
       ~VImageAnalyzer();

        void doAnalysis();                        //!< do the actual analysis (called for each event)
	VImageCleaning*  getImageCleaner() { return fVImageCleaning; }   //! return pointer to image cleaner
        void initAnalysis();                      //!< set the data vectors, read the calibration data (called once at the beginning of the analysis)
        void initOutput();                        //!< open outputfile
        void shutdown();                          //!< close outputfile
        void terminate();                         //!< write results to disk
};
#endif
