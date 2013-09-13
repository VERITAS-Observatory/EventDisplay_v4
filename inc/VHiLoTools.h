//! VHiLoTools  plot anasum results

#ifndef VHiLoTools_h
#define VHiLoTools_h

#include "TArrow.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TMath.h"
#include "TRandom.h"
#include "TTree.h"
#include "TEllipse.h"
#include "TMarker.h"

#include "TSystem.h"
#include "TH1.h"
#include "TH2F.h"
#include "TF1.h"
#include "TLegend.h"
#include "TCanvas.h"


#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

Double_t splitFitFunc(Double_t *x, Double_t *par);

class VHiLoTools
{

   private:

   bool fDebug;

   public:

   VHiLoTools(); 
   virtual ~VHiLoTools();
  
   void help();
   void makeRootFileFromDSTFile( int tel, int iSumWindow, int ifile1, int ifile2, string dataDir );
   vector<double> lowGainCalib( TFile *dstfile, TFile *dstfile18, int tel, int channel, bool bPlot );
   int lowGainVector( TH2F *hscat, TH1F *hlin, double xpos[10], double ypos[10], double expos[10], double eypos[10] );
   void mergeHiLoFiles( string infile1, string infile2, int SW, int TEL );
   void getMeanRMS( string ifile, int tel, int sumwindow, bool bPlot);

   //ClassDef(VHiLoTools,1);

};

#endif
