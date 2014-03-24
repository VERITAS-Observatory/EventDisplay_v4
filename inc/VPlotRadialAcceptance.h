//! VPlotRadialAcceptance plot radial acceptance curves
//

#ifndef VPlotRadialAcceptance_H
#define VPlotRadialAcceptance_H

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TLine.h"
#include "TText.h"

#include <iostream>
#include <string>
#include <vector>

#include "VHistogramUtilities.h"
#include "VPlotUtilities.h"
#include "VUtilities.h"

using namespace std;

class VPlotRadialAcceptance : public VPlotUtilities
{
	private:
	
		bool fDebug;
		
		string fName;
		
		TFile* fAcceptanceFile;
		TH1F*  fAcceptanceHisto;
		TH1F*  fAcceptanceHistoFit;
		TF1*   fAcceptanceFunction;
		vector< TH1F* > fAcceptancePhiHisto;
		vector< TF1* >  fAcceptancePhiFitFunction;
		vector< TH1F* > fAcceptancePhiHistoDeRot;
		vector< TF1* >  fAcceptancePhiFitFunctionDeRot;
		TH1F*  hPhiDist;
		TH1F*  hPhiDistDeRot;
		
		double fAxis_x_min;
		double fAxis_x_max;
		double fAxis_y_min;
		double fAxis_y_max;
		
	public:
	
		VPlotRadialAcceptance( string iFile = "", int iAzBin = -1 );
		~VPlotRadialAcceptance() {}
		
		TF1*  getAcceptanceFunction()
		{
			return fAcceptanceFunction;
		}
		TH1F* getAcceptanceHisto()
		{
			return fAcceptanceHisto;
		}
		TH1F* getAcceptanceHistoFit()
		{
			return fAcceptanceHistoFit;
		}
		
		TCanvas* plotRadialAcceptance( TCanvas* cX = 0 );
		TCanvas* plotPhiDependentRadialAcceptances( TCanvas* cX = 0, int iIterator = 4, bool iDeRot = false );
		TCanvas* plotPhiDistributions( TCanvas* cX = 0, int iColor = 1 );
		TCanvas* plotResiduals( TCanvas* cX = 0, double i_res_min = -0.5, double i_res_max = 0.5, bool iDrawChi2 = true );
		bool     openAcceptanceFile( string iFile, unsigned int iZeBin = 0, int iAzBin = -1 );
		void     setAxisRange( double x_min = 0., double x_max = 2.5, double y_min = 0., double y_max = 1.5 );
		void     setName( string iName )
		{
			fName = iName;
		}
		
		ClassDef( VPlotRadialAcceptance, 1 );
};

#endif
