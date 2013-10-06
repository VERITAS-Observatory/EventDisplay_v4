//! VInstrumentResponseFunctionRunParameter run parameters for response function calculator (effective areas)

#ifndef VInstrumentResponseFunctionRunParameter_H
#define VInstrumentResponseFunctionRunParameter_H

#include "Ctelconfig.h"
#include "VMonteCarloRunHeader.h"
#include "VTableLookupRunParameter.h"

#include <fstream>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "TChain.h"
#include "TNamed.h"

using namespace std;

class VInstrumentResponseFunctionRunParameter : public TNamed
{
   private:

   bool            readRunParameters( string ifilename );

   public:

   unsigned int    fFillingMode;              // filling mode

   string          fCutFileName;
   int             fGammaHadronCutSelector;
   int             fDirectionCutSelector;

   unsigned int    fEnergyReconstructionMethod;
   unsigned int    fEnergyAxisBins_log10;
   bool            fIgnoreEnergyReconstructionQuality;
   unsigned int    fNSpectralIndex;
   double          fSpectralIndexMin;
   double          fSpectralIndexStep;
   vector< double > fSpectralIndex;
   double          fMCEnergy_min;
   double          fMCEnergy_max;
   double          fMCEnergy_index;
   bool            fFillMCHistograms;

   string          fCoreScatterMode;
   double          fCoreScatterRadius;

   double          fViewcone_min;
   double          fViewcone_max;

   bool            fAzimuthBins;
   bool            fIsotropicArrivalDirections;
   float           fIgnoreFractionOfEvents;

   bool            fTelescopeTypeCuts;

   string          fdatafile;
   string          fMCdatafile_tree;
   string          fMCdatafile_histo;
   string          fGammaHadronProbabilityFile;

   double          fze;
   int             fnoise;
   double          fpedvar;
   double          fXoff;
   double          fYoff;
   vector< double > fAzMin;
   vector< double > fAzMax;

   unsigned int    telconfig_ntel;
   double          telconfig_arraycentre_X;
   double          telconfig_arraycentre_Y;
   double          telconfig_arraymax;

   string          fCREnergySpectrumFile;
   unsigned int    fCREnergySpectrumID;


   VInstrumentResponseFunctionRunParameter();
  ~VInstrumentResponseFunctionRunParameter() {}

   void                  print();
   VMonteCarloRunHeader* readMCRunHeader();
   bool                  readRunParameterFromTextFile( string iFile );
   bool                  testRunparameters();

   ClassDef( VInstrumentResponseFunctionRunParameter, 6 );
};

#endif
