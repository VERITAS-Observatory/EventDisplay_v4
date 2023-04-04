#ifndef VIPRCALCULATOR_H
#define VIPRCALCULATOR_H

#include <VImageBaseAnalyzer.h>
#include <VDB_CalibrationInfo.h>
#include <VSQLTextFileReader.h>

#include "TClonesArray.h"
#include "TFile.h"
#include "TH1F.h"
#include "TLeaf.h"
#include "TMath.h"
#include "TProfile.h"
#include "TSystem.h"
#include "TTree.h"

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class VIPRCalculator : public VImageBaseAnalyzer
{

	private:
		VEvndispData*      fData;
		vector< string > fPedFile;
		bool fIPRTimeSlices;
		bool fIPRAverageTel;   // flag to make average of all telescopes IPR in case there is not enough statistics to produce IPR graphs
		bool fIPRInTimeSlices;
		int fPedPerTelescopeTypeMinCnt;
		TH1F* FillIPRHistogram( unsigned int iSummationWindow,  unsigned int i_tel);
		TGraphErrors* updateIPRGraph(TH1F *hIPR, unsigned int i_tel, int iSummationWindow);
		float convertRate(unsigned int i_tel);
		float getTsearch();
		void definePedestalFile( std::vector<std::string> fPedFileNameC );
		TFile* initializePedestalFile( int i_tel );
		TH1F* initializeIPRHistogram( unsigned int iSummationWindow, unsigned int i_tel);
		bool calculateIPRGraphsTimeSlices( const  int TimeSlice, int iSummationWindow, const int itel);
		bool calculateIPRGraphsNoTimeSlices( unsigned int iSummationWindow, unsigned int i_tel );
		bool copyIPRInitialized( unsigned int iSummationWindow, unsigned int i_tel );
		TH1F* calculateIPRGraphAveragedNoTimeSlices( unsigned int iSummationWindow, unsigned int i_tel );
	public:
		
		vector<vector<vector<vector<TH1F*>>>> fpedcal_histo_storage;
		TH1F* getIPRPedestalHisto(const int telID, const int ts, const int pixel, const int sw);
		vector<vector<vector<vector<TH1F*>>>> getStorageHist();
		void fillIPRPedestalHisto();
		void fillIPRPedestalHisto(const int telID, const vector<vector<vector<TH1F*>>>& fpedcal_histo );
		bool calculateIPRGraphs( std::vector<std::string>  fPedFileNameC);
		bool writeIPRgraphs( map<ULong64_t, vector<vector<TH1F*>>> &hped_vec, string iFile = "" );
		void checkHistEmpty(const int telID, const int ts, const int pixel, const int sw);
	
		VIPRCalculator();
                ~VIPRCalculator() {}

		void initialize();
};
#endif 
