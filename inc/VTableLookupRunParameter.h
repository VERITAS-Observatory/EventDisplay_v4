//! VTableLookupRunParameter parameter storage class
// Revision $Id: VTableLookupRunParameter.h,v 1.1.2.2.4.1.4.2.6.1.2.5.2.7.2.1 2010/11/18 10:33:51 gmaier Exp $

#ifndef VTABLELOOKUPRUNPARAMTER_H
#define VTABLELOOKUPRUNPARAMTER_H

#include <TMath.h>
#include <TNamed.h>
#include <TSystem.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "VGlobalRunParameter.h"

using namespace std;

class VTableLookupRunParameter : public TNamed, public VGlobalRunParameter
{
    public:

	unsigned int fDebug;              // 0 = off, 1 = default debug level, 2 = detailed

        string inputfile;
        string outputfile;
        string tablefile;
        double ze;
        bool isMC;
        bool fUseMedianEnergy;
        bool fPE;                          // input size type is 'pe' (not [dc])
        int fInterpolate;
        string fInterpolateString;
        char readwrite;

	float fMinRequiredShowerPerBin;    // minimum number of showers required per table bin

	bool  fUseSelectedImagesOnly;

        double fMSCWSizecorrection;
        double fMSCLSizecorrection;
        double fEnergySizecorrection;
        string writeoption;
        bool bNoNoTrigger;
        int  bWriteReconstructedEventsOnly;
        bool bShortTree;
        bool bWriteMCPars;
        int  rec_method;
        bool point_source;
        string esysfile;
        bool fWrite1DHistograms;
        double fSpectralIndex;
        int fWobbleOffset;
        int fNoiseLevel;

	unsigned int fTableFillingCut_NImages_min;
	double       fTableFillingCut_CoreError_max;
	double       fTableFillingCut_WobbleCut_max;
        double fmaxlocaldistance;
        double fmaxdist;
        double fminsize;
        double fSelectRandom;
        int fSelectRandomSeed;
	double fMC_distance_to_cameracenter_min;
	double fMC_distance_to_cameracenter_max;

        int fNentries;
        double fMaxRunTime;

        double fDeadTimeFraction;

// parameters to be used in anasum
        double meanpedvars;                       // mean pedvar
        vector< double > pedvars;                 // mean pedvar per telescope

        string printpara;

        VTableLookupRunParameter();
        ~VTableLookupRunParameter() {}
        bool fillParameters( int argc, char *argv[] );
        void print( int iB = 0 );
        void printHelp();

        ClassDef(VTableLookupRunParameter,17);
};
#endif
