// VWPPhysSensitivityFile write write CTA WP Phys sensitivity files

#ifndef VWPPhysSensitivityFile_H
#define VWPPhysSensitivityFile_H

#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TList.h"

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "VASlalib.h"
#include "VInstrumentResponseFunctionReader.h"
#include "VSensitivityCalculator.h"

using namespace std;

class VWPPhysSensitivityFile
{
	private:
	
		bool         fDebug;
		
		TFile*       fOutFile;
		string       fDataFile_gamma_onSource;
		string       fDataFile_gamma_cone;
		string       fDataFile_proton;
		string       fDataFile_proton_onSource;
		string       fDataFile_electron;
		string       fDataFile_electron_onSource;
		
		string       fSubArray;
		string       fObservatory;
		double       fObservingTime_h;
		
		string       fCrabSpectrumFile;
		unsigned int fCrabSpectrumID;
		string       fCosmicRaySpectrumFile;
		unsigned int fProtonSpectrumID;
		unsigned int fElectronSpectrumID;
		
		unsigned int fOffsetCounter;
		
		vector< TH1* > hisList;
		vector< TH1* > hisListToDisk;
		vector< TH1* > hisListToDiskDebug;
		
		TH1F* fSensitivity;
		vector< TH1F* > fSensitivityLimits;
		TH1F* fSensitivityCU;
		vector< TH1F* > fSensitivityCULimits;
		TH1F* fIntSensitivity;
		//vector< TH1F* > fIntSensitivityLimits;
		TH1F* fIntSensitivityCU;
		//vector< TH1F* > fIntSensitivityCULimits;
		TH1F* fBGRate;
		TH1F* fBGRateSqDeg;
		TH1F* fProtRate;
		TH1F* fProtRateSqDeg;
		TH1F* fElecRate;
		TH1F* fElecRateSqDeg;
		TH1F* fEffArea;
		TH1F* fEffAreaMC;
		TH1F* fEffArea80;
		TH1F* fAngRes68;
		TH1F* fAngRes80;
		TH1F* fEres;
		TH1F* fEbias;
		
	public:
	
		VWPPhysSensitivityFile();
		~VWPPhysSensitivityFile() {}
		
		bool fillIRFHistograms( string iEffectiveAreaFile, double iZe = 20., double iWoff = 0. );
		bool fillHistograms1D( string iDataDirectory, bool iFill1D = true );
		bool fillHistograms2D( vector< double > iWobble_min, vector< double > iWobble_max );
		bool fillSensitivityHistograms( string iDataDirectory, bool iFill1D = true );
		bool initializeHistograms( int iEnergyXaxisNbins = 20, double iEnergyXaxis_min = -1.8, double iEnergyXaxis_max = 2.2,
								   int iEnergyTrue2DXaxisNbins = 500, double iEnergyTrue2DXaxis_min = -1.8, double iEnergyTrue2DXaxis_max = 2.2,
								   int iEnergyRec2DXaxisNbins = 400, double iEnergyRec2DXaxis_min = -2.3, double iEnergyRec2DXaxis_max = 2.7,
								   unsigned int iOffsetCounter = 9999 );
		bool initializeOutputFile( string iOutputFile );
		unsigned int isVTS();
		void setCrabSpectrum( string iCrabSpectrum, unsigned int iID = 5 )
		{
			fCrabSpectrumFile = iCrabSpectrum;
			fCrabSpectrumID = iID;
		}
		void setCosmicRaySpectrum( string iCRSpectrum, unsigned iPID = 0, unsigned iEID = 2 )
		{
			fCosmicRaySpectrumFile = iCRSpectrum;
			fProtonSpectrumID = iPID;
			fElectronSpectrumID = iEID;
		}
		void setDebug( bool iB = false )
		{
			fDebug = iB;
		}
		void setObservationTime( double iO_h = 50. )
		{
			fObservingTime_h = iO_h;
		}
		void setObservatory( string iO = "CTA" )
		{
			fObservatory = iO;
		}
		void setDataFiles( string iA = "E", int iRecID = 0 );
		bool terminate();
		
};

#endif

