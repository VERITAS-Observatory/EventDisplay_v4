//! VInstrumentResponseFunction calculate response function (e.g. angular resolution)

#ifndef VInstrumentResponseFunction_H
#define VInstrumentResponseFunction_H

#include "TGraphErrors.h"
#include "TH2D.h"
#include "TList.h"
#include "TMath.h"
#include "TTree.h"

#include <iostream>
#include <string>
#include <vector>

#include "CData.h"
#include "VGammaHadronCuts.h"
#include "VInstrumentResponseFunctionData.h"
#include "VInstrumentResponseFunctionRunParameter.h"
#include "VSpectralWeight.h"

using namespace std;

class VInstrumentResponseFunction
{
	private:
	
		bool  fDebug;
		
		string fName;
		string fType;
		
// data tree
		CData*   fData;
		
// return data tree
		TTree*    fDataProduct;
		VInstrumentResponseFunctionData* fIRFData_Tree;
		
		unsigned int fEnergyReconstructionMethod;
		
// histograms and data
		vector< vector< VInstrumentResponseFunctionData* > > fIRFData;
		
// cuts
		VGammaHadronCuts* fAnaCuts;
		
// effective area calculation
		vector< double > fVMinAz;
		vector< double > fVMaxAz;
// spectral weighting
		vector< double > fVSpectralIndex;
		VSpectralWeight* fSpectralWeight;
		
// containment probabilities
		double  fContainmentProbability;
		double  fContainmentProbabilityError;
		
		bool    defineHistograms();
		
	public:
	
		VInstrumentResponseFunction();
		~VInstrumentResponseFunction() {}
		bool   fill();
		double getContainmentProbability()
		{
			return fContainmentProbability;
		}
		TTree* getDataProduct()
		{
			return fDataProduct;
		}
		TGraphErrors* getAngularResolutionGraph( unsigned int iAzBin, unsigned int iSpectralIndexBin );
		bool   initialize( string iName, string iType, unsigned int iNTel, double iMCMaxCoreRadius, double iZe, int iNoise, double iPedvars, double iXoff, double iYoff );
		void   setEnergyReconstructionMethod( unsigned int iMethod );
		void   setCuts( VGammaHadronCuts* iCuts );
		void   setContainmentProbability( double iP = 0.68, double iPError = 0.95 )
		{
			fContainmentProbability = iP;
			fContainmentProbabilityError = iPError;
		}
		void   setDataTree( CData* iData );
		void   setHistogrambinning( int iN = 20, double iMin = -2., double iMax = 2. );
		void   setMonteCarloEnergyRange( double iMin, double iMax, double iMCIndex = 2. );
		void   setRunParameter( VInstrumentResponseFunctionRunParameter* );
};

#endif
