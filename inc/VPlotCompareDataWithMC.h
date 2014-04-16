//! VPlotCompareDataWithMC compare data with MC

#ifndef VPlotCompareDataWithMC_H
#define VPlotCompareDataWithMC_H

#include "TBox.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TLine.h"
#include "TMath.h"
#include "TStyle.h"
#include "TTree.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class VPlotCompareDataWithMC
{
	private:
	
		bool fDebug;
		bool fPlotPoster;
		string fPrintName;
		unsigned int fNTel;
		
		string  fDataFileName;
		TFile*  fDataFile;
		
		double fRelatePlotRange_min;
		double fRelatePlotRange_max;
		
		void   drawMatchingTests( TH1D* h1, TH1D* h2, double xmin = -9999., double xmax = 9999. );
		TF1*   do_theta2Fit( TH1D* h, int icolor = 1, int istyle = 1 );
		void   getScaling( TH1D* h_sims, TH1D* h_diff, double& s_sims, double& s_diff,
						   int bContents = 1, double xmin = -9999., double xmax = 9999. );
		void   getScaling( double& s_sims, double& s_diff, string his = "MSCW",
						   int bContents = 1, double xmin = -9999., double xmax = 9999. );
		void   plot_energyDependentDistributions( string iVariable, int iRebin, double x_min, double x_max, string iPlot = "SIMSDIFF" );
		void   plotLegend( TH1D* hsims, TH1D* hdiff, double x0 = 0.5 );
		TCanvas* plotRelativePlots( char* i_CanvasName, char* i_CanvasTitle, TH1D* h1, TH1D* h2, double xmin, double xmax );
		void   plotRelativePlot( TH1D* h1, TH1D* h2, double xmin = -999., double xmax = -999. );
		void   plot_singleCanvas( string iHistoName, string iCanvasTitle, double iHistoXAxisMax, string iScalingVariable = "MSCW" );
		void   setAxisTitles( TH2D* h, string iS, int iTel );
		void   setHistogramAtt( TH1D* his, int icolor, double iwidth, double isize, int imarker = 1, int irebin = 1 );
		void   setHistogramAtt( TH2D* his, double imin );
		
	public:
	
		VPlotCompareDataWithMC( string iFile = "" );
		~VPlotCompareDataWithMC() {}
		
		void help();
		void centroids();
		void core_plots( int iRebin = 4, int iScaling = 1 );
		void distance_plots();
		void emission_height( double iEmissionHeightMax = 20. );
		void msc_plots( char* offFile = 0, char* helium = 0, char* proton = 0, double xmin = -1.5, double xmax = 4., string ivar = "MSCW" );
		void msc_vs_energy_plots( int iRebin = 4, double xmin = -1.5, double xmax = 1.5 );
		void mwr_vs_energy_plots( int iRebin = 4, double xmin =  0.5, double xmax = 1.5 );
		void multiplicity_plots();
		bool openDataFile( string ifile );
		void plot( string iPrintName );
		void single_telescope( int telid = -1 );
		void single_telescope( int telid, string iPlot, bool iOneCanvas = true,
							   int iScalingMethod = 1, int i_rebin = 1 );
		void stereo_parameter( int msc_rebin = 1 );
		
		void setDebug( bool iB = false )
		{
			fDebug = iB;
		}
		void setNTel( unsigned int iN = 4 )
		{
			fNTel = iN;
		}
		void setPosterPlot( bool iB = false )
		{
			fPlotPoster = iB;
		}
		void setPrintName( string iP = "" )
		{
			fPrintName = iP;
		}
		void setRelativePlotRange( double iMin = 0.03, double iMax = 3 )
		{
			fRelatePlotRange_min = iMin;
			fRelatePlotRange_max = iMax;
		}
};

#endif
