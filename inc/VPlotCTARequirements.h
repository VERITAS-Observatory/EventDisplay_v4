//! VPlotCTARequirements plot CTA requirements on top of sensitivity and angular resolution plots

#ifndef VPlotCTARequirements_H
#define VPlotCTARequirements_H

#include "TArrow.h"
#include "TCanvas.h"
#include "TGraph.h"

#include "VPlotUtilities.h"

#include <iostream>

using namespace std;

class VPlotCTARequirements : public VPlotUtilities
{
   private:

   int fSetOfRequirementID;

   double fReqEnergyRange_min;
   double fReqEnergyRange_max;

   TGraph *fReqDifferentialSensitivity50h; // vs log10 energy [TeV]
   TGraph *fReqEffectiveArea;           // vs log10 energy [TeV]
   TGraph *fReqAngularResolution;       // vs log10 energy [TeV]
   TGraph *fReqEnergyResolution;        // vs log10 energy [TeV]
   TGraph *fGoalDifferentialSensitivity50h; // vs log10 energy [TeV]
   TGraph *fGoalEffectiveArea;           // vs log10 energy [TeV]
   TGraph *fGoalAngularResolution;       // vs log10 energy [TeV]
   TGraph *fGoalEnergyResolution;        // vs log10 energy [TeV]

   void plotRequirements( TGraph *g, bool iLog = false );

   public:

   VPlotCTARequirements();
  ~VPlotCTARequirements() {}

   void    plotRequirement_EnergyRange( TCanvas *c, bool iPlotGoalRequirements = false );
   TGraph* plotRequirement_AngularResolution( TCanvas *c, bool iPlotGoalRequirements = false );
   TGraph* plotRequirement_EffectiveArea( TCanvas *c, bool iPlotGoalRequirements = false );
   TGraph* plotRequirement_EnergyResolution( TCanvas *c, bool iPlotGoalRequirements = false );
   TGraph* plotRequirement_DifferentialSensitivity50h( TCanvas *c, bool iPlotGoalRequirements = false );
   void    printRequirements( int iRequirementID = 0 );
   bool    setRequirement( int iRequirementID = 0 );

};

#endif
