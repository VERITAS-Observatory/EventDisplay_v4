//! VCTARequirements plot CTA requirements on top of sensitivity and angular resolution plots

#ifndef VCTARequirements_H
#define VCTARequirements_H

#include "TArrow.h"
#include "TCanvas.h"
#include "TGraph.h"

#include "VCTASensitivityRequirements.h"
#include "VPlotUtilities.h"

#include <iostream>

using namespace std;

class VCTARequirements : public VPlotUtilities
{
   private:

   int fSetOfRequirementID;

   double fReqEnergyRange_min;
   double fReqEnergyRange_max;

   TGraph *fReqDifferentialSensitivity; // vs log10 energy [TeV]
   TGraph *fReqEffectiveArea;           // vs log10 energy [TeV]
   TGraph *fReqAngularResolution;       // vs log10 energy [TeV]
   TGraph *fReqEnergyResolution;        // vs log10 energy [TeV]
   TGraph *fGoalDifferentialSensitivity; // vs log10 energy [TeV]
   TGraph *fGoalEffectiveArea;           // vs log10 energy [TeV]
   TGraph *fGoalAngularResolution;       // vs log10 energy [TeV]
   TGraph *fGoalEnergyResolution;        // vs log10 energy [TeV]

   void plotRequirements( TGraph *g, bool iLog = false, bool iLine = false );

   public:

   VCTARequirements();
  ~VCTARequirements() {}

   double  getFOVRequirement( double E_lin_TeV );
   TGraph* getGoalDifferentialSensitivity() { return fGoalDifferentialSensitivity; }
   TGraph* getRequiredDifferentalSensitivity() { return fReqDifferentialSensitivity; }
   void    listRequirementSets();
   void    plotRequirement_EnergyRange( TCanvas *c, bool iPlotGoalRequirements = false );
   TGraph* plotRequirement_AngularResolution( TCanvas *c, bool iPlotGoalRequirements = false );
   TGraph* plotRequirement_EffectiveArea( TCanvas *c, bool iPlotGoalRequirements = false );
   TGraph* plotRequirement_EnergyResolution( TCanvas *c, bool iPlotGoalRequirements = false );
   TGraph* plotRequirement_DifferentialSensitivity( TCanvas *c, bool iPlotGoalRequirements = false, string iUnit = "ENERGY" );
   void    printRequirements( int iRequirementID = 0 );
   bool    setRequirement( int iRequirementID = 0 );

};

#endif

