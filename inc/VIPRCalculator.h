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
		vector< string > fPedFileNameC;
		bool fIPRTimeSlices;
		bool fIPRAverageTel;   // flag to make average of all telescopes IPR in case there is not enough statistics to produce IPR graphs
		bool fIPRInTimeSlices;
		int fPedPerTelescopeTypeMinCnt;
		TH1F* FillIPRHistogram( unsigned int iSummationWindow,  unsigned int i_tel );
		void definePedestalFile( std::vector<std::string> fPedFileNameCalibrator );
		TH1F* initializeIPRHistogram( unsigned int iSummationWindow, unsigned int i_tel );
		bool copyIPRTelAveraged( unsigned int iSummationWindow, ULong64_t iTelType, unsigned int i_tel );
		TH1F* calculateIPRGraphAveraged( unsigned int iSummationWindow );
	public:
		vector<vector<vector<vector<TH1F*>>>> fpedcal_histo_storage;
		bool calculateIPRGraphs( std::vector<std::string>  fPedFileNameCalibrator );
		bool calculateIPRGraphs( string iPedFileName, unsigned int iSummationWindow, ULong64_t iTelType, unsigned int i_tel );
		bool calculateIPRGraphsTimeSlices( string iPedFileName, int TS,  unsigned int iSummationWindow, ULong64_t iTelType, unsigned int i_tel );
		bool writeIPRgraphs( map<ULong64_t, vector<vector<TH1F*>>>& hped_vec, string iFile = "" );
		void fillIPRPedestalHisto( const int telID, const int NTimeSlices, const vector<vector<vector<TH1F*>>>& fpedcal_histo );
		void fillIPRPedestalHisto();
		TH1F* getIPRPedestalHisto( const int telID, const int ts, const int pixel, const int sw );
		bool clearHistos();
		vector<vector<vector<vector<TH1F*>>>> getStorageHist();
		
		VIPRCalculator();
		~VIPRCalculator() {}
		
		void initialize();
};
#endif
