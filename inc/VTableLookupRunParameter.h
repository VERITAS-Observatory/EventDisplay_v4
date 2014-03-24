//! VTableLookupRunParameter parameter storage class

#ifndef VTABLELOOKUPRUNPARAMTER_H
#define VTABLELOOKUPRUNPARAMTER_H

#include <TChain.h>
#include <TMath.h>
#include <TNamed.h>
#include <TSystem.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "VGlobalRunParameter.h"

using namespace std;

class VTableLookupRunParameter : public TNamed, public VGlobalRunParameter
{
	private:
	
		bool fillInputFile_fromList( string iList );
		void printCTA_MC_offaxisBins();
		void setCTA_MC_offaxisBins();
		
	public:
	
		unsigned int fDebug;              // 0 = off, 1 = default debug level, 2 = detailed
		
		vector< string > inputfile;
		string outputfile;
		string tablefile;
		double ze;
		bool isMC;
		int fUseMedianEnergy;
		bool fPE;                          // input size type is 'pe' (not [dc])
		int fInterpolate;
		string fInterpolateString;
		char readwrite;
		
		bool  fLimitEnergyReconstruction;
		
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
		double fmaxdist;
		double fminsize;
		double fSelectRandom;
		int fSelectRandomSeed;
		double fMC_distance_to_cameracenter_min;
		double fMC_distance_to_cameracenter_max;
		vector< double > fCTA_MC_offaxisBin_min;
		vector< double > fCTA_MC_offaxisBin_max;
		vector< double > fAddMC_spectral_index;
		
		int fNentries;
		double fMaxRunTime;
		
		double fDeadTimeFraction;
		
// parameters to be used in anasum
		double meanpedvars;                       // mean pedvar
		vector< double > pedvars;                 // mean pedvar per telescope
		
		string printpara;
		
		VTableLookupRunParameter();
		~VTableLookupRunParameter() {}
		bool fillParameters( int argc, char* argv[] );
		void print( int iB = 0 );
		void printHelp();
		
		ClassDef( VTableLookupRunParameter, 22 );
};
#endif
