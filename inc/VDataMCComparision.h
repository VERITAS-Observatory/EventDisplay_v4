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
#include <map>
#include <string>
#include <vector>

using namespace std;

class VDataMCComparisionHistogramData
{
      public:

      string fVarName;
      string fHistogramType;
      unsigned int fTelescopeID;                    // 0 = array variable

      TH1D*  fHis1D;
      TH2D*  fHis2D;

      VDataMCComparisionHistogramData( string iVarName = "", string iHistogramType = "", unsigned int iTelescopeID = 0 );
     ~VDataMCComparisionHistogramData() {}
      bool   initHistogram( string iXTitle, int iNbins, double ix_min, double ix_max );
      void   fill( double iV, double iWeight = 1., double iLogEnergy_TeV = -99. );  
};

class VDataMCComparision
{
	private:

                enum E_varname { ELENGTH, EWIDTH, EDIST, EALPHA, ENTUBES, ENLOWGAIN, ESIZE, ESIZE2, ESIZELG, EFRACLOW, EMAX1, EMAX2, EMAX3, ELOSS, ELOS, EASYM,
                                 ECENX, ECENY, ETGRADX, EMSCWT, EMSCLT, ETELDIST, ETHETA2, ELTHETA2, EMSCW, EMSCL, EMWR, EMLR, EXCORE, EYCORE, EEREC, ENIMAGES,
                                 EIMGSEL, EEMISSIONHEIGHT, EMVA, ESIGMAT3D, ENC3D, EDEPTH3D, ERWIDTH3D, EERRRWIDTH3D };
	
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
		
		// lists with all histograms
		TList* hisList;
		vector<TH1D* > hTel;
		vector<TH2D* > hTel2D;

                // histogram classes
                map< E_varname, vector< VDataMCComparisionHistogramData* > > fHistoSingleTel; 
                map< E_varname, VDataMCComparisionHistogramData* > fHistoArray; 

		// stereo histograms
		TH2D* hXYcore;
		TH2D* hAzYcore;
		TH2D* hYt2;
                vector<TH2D* > hcen_xy;
		vector< TH2D* > hdistR;
		
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
