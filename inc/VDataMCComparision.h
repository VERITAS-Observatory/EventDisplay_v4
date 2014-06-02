//! VDataMCComparision

#include "TChain.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TList.h"
#include "TMath.h"
#include "TProfile.h"

#include "CData.h"
#include "VEvndispRunParameter.h"
#include "VGammaHadronCuts.h"
#include "VMonteCarloRunHeader.h"
#include "VSpectralWeight.h"
#include "VUtilities.h"

#include <bitset>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class VDataMCComparision
{
	private:
	
		string fName;
		int fNTel;
		
		vector< double > fTel_x;
		vector< double > fTel_y;
		vector< double > fTel_z;
		
		bool bBckData;
		
		// wobble north offset
		double fWobbleNorth;
		double fWobbleEast;
		bool   fWobbleFromDataTree;
		
		double fAzMin;
		double fAzMax;
		bool fAzRange;
		
		// spectral weighting
		VSpectralWeight* fSpectralWeight;
		
		// data tree
		CData* fData;
		
		// cuts
		VGammaHadronCuts* fCuts;
		bool fCalculateMVAValues;
		
		// list with all histograms
		TList* hisList;
		// stereo histograms
		TH1D* htheta2;
		TH1D* hltheta2;
		TH1D* hMSCW;
		TH1D* hMSCL;
		TH2D* hMSCWErec;
		TH2D* hMSCLErec;
		TH1D* hMWR;//AMc
		TH1D* hMLR;//AMc
		TH2D* hMWRErec;//AMc
		TH2D* hMLRErec;//AMc
		TH1D* hXcore;
		TH1D* hYcore;
		TH2D* hXYcore;
		TH2D* hAzYcore;
		TH2D* hYt2;
		TH1D* hErec;
		TH1D* hNimages;
		TH1D* hImgSel;
		TH1D* hEmissionHeight;
		TH1D* hMVA;
		vector< TH1D* > hR;
		vector< TH2D* > hdistR;
		TH1D* hsigmaT3D;    //JG
		TH1D* hNc3D;        //JG
		TH1D* hDepth3D;     //JG
		TH1D* hRWidth3D;    //JG
		TH1D* hErrRWidth3D; //JG
		TH2D* hsigmaT3DErec;    //JG
		TH2D* hNc3DErec;        //JG
		TH2D* hDepth3DErec;     //JG
		TH2D* hRWidth3DErec;    //JG
		TH2D* hErrRWidth3DErec; //JG

		// single telescope histograms
		vector<TH1D* > hdist;
		vector<TH1D* > hntubes;
		vector<TH1D* > hnlowgain;
		vector<TH1D* > hwidth;
		vector<TH1D* > hlength;
		vector<TH1D* > hlos;
		vector<TH1D* > hloss;
		vector<TH1D* > hsize;
		vector<TH1D* > hsize2;
		vector<TH1D* > hsizeLG;
                vector<TH1D* > hfraclow;
		vector<TH1D* > hmax1;
		vector<TH1D* > hmax2;
		vector<TH1D* > hmax3;
		vector<TH1D* > halpha;
		vector<TH1D* > hasym;
		vector<TH1D* > hcen_x;
		vector<TH1D* > hcen_y;
		vector<TH2D* > hcen_xy;
		vector<TH1D* > htgrad_x;
		vector<TH1D* > hmscwt;
		vector<TH1D* > hmsclt;
		vector<TH1D* > hr;
		vector<TH1D* > hTel;
		vector<TH2D* > hTel2D;
		
		void setEntries( TH1D* );
		void setEntries( TH2D* );
		
		void initialGammaHadronCuts();
		
	public:
	
		VDataMCComparision( string, bool, int );
		~VDataMCComparision() {}
		void defineHistograms();
		bool fillHistograms( string ifile, int iSingleTelescopeCuts );
		bool fillHistograms( string ifile, int iSingleTelescopeCuts, double iWobbleNorth, double iWobbleEast );
		void resetTelescopeCoordinates();
		void scaleHistograms( string );
		void setAzRange( double iAzMin, double iAzMax );
		bool setOnOffHistograms( VDataMCComparision*, VDataMCComparision*, double norm );
		bool setTelescopeCoordinates( double x, double y, double z = 0. );
		void setWobbleFromDataTree()
		{
			fWobbleFromDataTree = true;
		}
		bool writeHistograms( TDirectory* );
};
