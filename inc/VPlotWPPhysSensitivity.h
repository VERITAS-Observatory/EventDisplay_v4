//! VPlotWPPhysSensitivity plot CTA style sensitivities

#ifndef VPlotWPPhysSensitivity_H
#define VPlotWPPhysSensitivity_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TH1D.h"
#include "TH1F.h"
#include "TLegend.h"
#include "TLine.h"
#include "TPad.h"

#include "VHistogramUtilities.h"
#include "VMathsandFunctions.h"
#include "VCTARequirements.h"
#include "VCTASensitivityRequirements.h"
#include "VPlotInstrumentResponseFunction.h"
#include "VPlotUtilities.h"
#include "VSensitivityCalculator.h"
#include "VSiteData.h"


class VPlotWPPhysSensitivity : public VPlotUtilities
{
	private:
	
		VPlotInstrumentResponseFunction* fIRF;
		
		vector< VSiteData* > fData;
		
		double fMinEnergy_TeV;
		double fMaxEnergy_TeV;
		string fCrabSpectraFile;
		unsigned int fCrabSpectraID;
		
		int fPlotCTARequirementsID;
		bool fPlotCTARequirementGoals;
		VCTARequirements* fPlotCTARequirements;

		bool fUseIntegratedSensitivityForOffAxisPlots;
		
		// FOM variables
		double fSensitivityFOM;
		double fSensitivityFOM_error;
		
		// projected sensitvity plots
		vector< double >  fProjectionEnergy_min_logTeV;
		vector< double >  fProjectionEnergy_max_logTeV;
		map< string, vector< TGraphAsymmErrors* > > fProjectionSensitivityvsCameraOffset;
		
		void    fillProjectedSensitivityPlot( unsigned int i, TGraphAsymmErrors* g );
		void    initialProjectedSensitivityPlots();
		bool    plotLegend( TCanvas* c = 0, bool iDown = false, bool iLeft = false, bool iAddFirst = true );
		
	public:
	
		VPlotWPPhysSensitivity();
		~VPlotWPPhysSensitivity() {}
		
		bool addDataSet( VSiteData* iData );
		bool addDataSet( string iAnalysis, string iSubArray = "E", double iObservationTime_s = 180000., double iOffset_deg = 0.0,
						 string iLegend = "", int iColor = 1, int iLineStyle = 1, int iFillStyle = 3001 );
		bool addDataSets( string iDataSettxtFile, string iDirectionString );
		double getSensitivityFOM()
		{
			return fSensitivityFOM;
		}
		double getSensitivityFOM_error()
		{
			return fSensitivityFOM_error;
		}
		vector< VSiteData* > getData()
		{
			return fData;
		}
		bool plotIRF( string iPrint = "", double iEffAreaMin = 50., double iEffAreaMax = 5.e7, double iEnergyResolutionMax = 0.5, TPad *iEffAreaPad = 0, TPad *iAngResPad = 0, TPad *iEResPad = 0 );
		TCanvas* plotProjectedSensitivities( TCanvas*, double iMaxOffset, int iColor = -1 );
		bool plotSensitivity( string iPrint = "", double iMinSensitivity = 4.e-14, double iMaxSensitivity = 2.5e-10, string iUnit = "ENERGY", TPad *iSensitivityPad = 0, TPad *iSensitivityRatioPad = 0, TPad *iBckPad = 0 );
		bool plotSensitivityRatio( string iPrint, double ymin = 0.01, double ymax = 2., bool iRatoToGoal = false, TPad *iSensRatio = 0 );
		void printSensitivityFigureOfMerit( TGraphAsymmErrors* gSensitivity, double iEmin_TeV = 0.03, double iEmax_TeV = 100., string iAnalysis = "" );
		void printSensitivityFigureOfMerit( double iEmin_TeV = 0.03, double iEmax_TeV = 100. );
		void reset();
		void setCrabSpectraFile( string iFile = "$CTA_EVNDISP_AUX_DIR/AstroData/TeV_data/EnergySpectrum_literatureValues_CrabNebula.dat",
								 unsigned int iSpectraID = 6 )
		{
			fCrabSpectraFile = iFile;
			fCrabSpectraID = iSpectraID;
		}
		void setEnergyRange_Lin_TeV( double iMinEnergy_TeV = 0.01, double iMaxEnergy_TeV = 200. )
		{
			fMinEnergy_TeV = iMinEnergy_TeV;
			fMaxEnergy_TeV = iMaxEnergy_TeV;
		}
		
		bool setPlotCTARequirements( int iRequirementID = -1, bool iPlotRequirementGoals = false );
		bool setCTARequirements( int iRequirementID = -1, bool iPlotRequirementGoals = true )
		{
			return setPlotCTARequirements( iRequirementID, iPlotRequirementGoals );
		}
		void setUseIntegratedSensitivityForOffAxisPlots( bool iUseIntegratedSensitivityForOffAxisPlots = false )
		{
		       fUseIntegratedSensitivityForOffAxisPlots = iUseIntegratedSensitivityForOffAxisPlots;
		}
};

#endif
