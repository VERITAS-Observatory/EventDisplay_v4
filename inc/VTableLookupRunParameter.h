//! VTableLookupRunParameter parameter storage class

//   ClassDef VERSION NUMBER HAS TO BE INCREASED BY ONE AFTER ANY CHANGES HERE

#ifndef VTABLELOOKUPRUNPARAMTER_H
#define VTABLELOOKUPRUNPARAMTER_H

#include <TChain.h>
#include <TFile.h>
#include <TMath.h>
#include <TNamed.h>
#include <TSystem.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "VEvndispRunParameter.h"
#include "VEvndispReconstructionParameter.h"
#include "VGlobalRunParameter.h"

using namespace std;

class VTableLookupRunParameter : public TNamed, public VGlobalRunParameter
{
	private:
	
		bool fillInputFile_fromList( string iList );
		bool readTelescopeToAnalyze( string iFile );
		
	public:
	
		// debug levels 0 = off, 1 = default debug level, 2 = detailed
		unsigned int fDebug;
		
		// list of evndisp input files
		vector< string > inputfile;
		string outputfile;
		// name of lookup table file
		string tablefile;
		double ze;
		bool isMC;
		int fUseMedianEnergy;
		bool fPE;                          // input size type is 'pe' (not [dc])
		string fInterpolateString;
		char readwrite;
		bool fUpdateInstrumentEpoch;
		
		bool  fLimitEnergyReconstruction;
		
		float fMinRequiredShowerPerBin;    // minimum number of showers required per table bin
		
		bool  fUseSelectedImagesOnly;
		
		string writeoption;
		bool bNoNoTrigger;
		int  bWriteReconstructedEventsOnly;
		bool bShortTree;
		bool bWriteMCPars;
		int  rec_method;
		// quality cut level
		unsigned int fQualityCutLevel;
		bool fWrite1DHistograms;
		double fSpectralIndex;
		// wobble offset
		int fWobbleOffset;
		// NSB (pedvars) level
		int fNoiseLevel;
		
		vector< unsigned int > fTelToAnalyse;             // telescopes used in analysis
		
		unsigned int fTableFillingCut_NImages_min;
		double       fTableFillingCut_WobbleCut_max;
		double fMC_distance_to_cameracenter_min;
		double fMC_distance_to_cameracenter_max;
		double       fmaxdist;   // note: same for all telescope types
		double       fmaxloss;   // note: same for all telescope types
		double       fminsize;   // note: same for all telescope times
		// seed for random selection of showers before table filling
		double fSelectRandom;
		int fSelectRandomSeed;
		vector< double > fAddMC_spectral_index;
		
		Long64_t fNentries;
		double fMaxRunTime;
		// parameters to be used in anasum
		double meanpedvars;                       // mean pedvar
		vector< double > pedvars;                 // mean pedvar per telescope
		
		string printpara;
		// rerun stereo reconstruction
		bool  fRerunStereoReconstruction;
		double fRerunStereoReconstruction_minAngle;
		string fRerunStereoReconstruction_BDTFileName;
		unsigned int fRerunStereoReconstruction_BDTNImages_max;
		string fDispError_BDTFileName;
		float  fDispError_BDTWeight;
		
		// functions...
		VTableLookupRunParameter();
		~VTableLookupRunParameter() {}
		
		bool fillParameters( int argc, char* argv[] );
		void print( int iB = 0 );
		void printHelp();
		
		ClassDef( VTableLookupRunParameter, 28 );
};
#endif
