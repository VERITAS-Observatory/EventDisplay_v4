//! VFITS read anasum file and write sky plots into FITS format
// Revision $Id: VFITS.h,v 1.1.2.3.8.2.2.1.2.1.2.1 2011/04/08 13:48:13 gmaier Exp $

#ifndef VFITS_H
#define VFITS_H

#include <ctime>
#include <iostream>
#include <fstream>
#include <math.h>
#include <sstream>
#include <stdio.h>
#include <string>
#include <stdlib.h>

#include "TDatime.h"
#include "TH2D.h"
#include "TH1D.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TTree.h"

#include "/afs/ifh.de/group//cta/VERITAS/software/FITS/cfitsio//include/fitsio.h"
#include "/afs/ifh.de/group//cta/VERITAS/software/FITS/cfitsio//include/longnam.h"
#include "CRunSummary.h"

#include "VAnalysisUtilities.h"
#include "VHistogramUtilities.h"
#include "VPlotUtilities.h"
#include "VStatistics.h"
#include "VEnergySpectrum.h" 

using namespace std;

class VFITS : public VAnalysisUtilities, public VPlotUtilities, public VHistogramUtilities
{
    private:
        fitsfile *fptr;      
 
        string fEVDversion;
        string fFile_anasum;
        string fFile_FITS;
        string fTarget_Name;

        bool fWriteOneFile; 

        CRunSummary* ctRunSum;
        float fTarget_Exposure;
        float fTarget_RAJ200;
        float fTarget_DecJ2000;

        bool printerror( int status );
        int writeTH1DFits(TH1D* h,string DiagName, string x_name, string y_name, string x_unit, string y_unit, bool iPrint);
        int writeTGraphFits(TGraph* g,string DiagName, string x_name, string y_name, string x_unit, string y_unit, bool iPrint);
        int writeVecTH1DFits(vector<pair<TH1D*,string> > vhist, string DiagName,  char *tType[],char *tUnit[],char *tForm[] , bool iPrint);
        int writeTGraphErrorsFits(TGraphErrors* g,string DiagName, string x_name, string y_name, string x_unit, string y_unit, bool iPrint);
        int createTableFitsFile(vector< vector<double> > Table ,char *ttype[] ,char *tunit[], char *tform[],string DiagName, bool iPrint);
        int createImageFitsFile(TH2D *hSkyMap , string DiagName, bool iPrint);
        bool writeFITSInfo( bool iPrint = false);
	bool writeFITSimageInfo(long naxis,long *naxes, TH2D *hSkyMap , string DiagName, bool iPrint = false);
        bool mergeColumns(fitsfile *fPtr, vector<int> hdunums, vector<vector <int> > columns, int nRows, bool iPrint);
        double getFluxIntegral(TGraphErrors* gEspec, double minE, bool iPrint);

    public:

        VFITS( string anasum_file, string fits_file , string object_name, bool iOneFile = true, bool iPrint = false);
        ~VFITS() {} 
        bool readAnasumFile( bool iPrint = false );
        bool writeCumSignificance(bool iPrint = false );
        bool writeSignificanceDistribution(bool iPrint = false );
        bool writeSignificanceSkyMap(bool iPrint= false );
        bool writeExcessSkyMap(bool iPrint =false);
        bool writeLightCurve(bool iPrint = false );
        bool writeThetaSquareDistribution(bool iPrint= false );
        bool writeEnergySpectrum(bool iPrint = false);
        bool writeFITSFile(bool iPrint = false);
};
#endif
