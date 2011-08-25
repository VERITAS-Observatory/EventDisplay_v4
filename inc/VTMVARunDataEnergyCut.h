//! VTMVARunDataEnergyCut data class for TMVA energy cuts
// Revision $Id: VTMVARunDataEnergyCut.h,v 1.1.2.1 2011/04/01 12:10:32 gmaier Exp $

#ifndef VTMVARunDataEnergyCut_H
#define VTMVARunDataEnergyCut_H

#include <iostream>

#include "TCut.h"

using namespace std;

class VTMVARunDataEnergyCut : public TNamed
{
   public:

   unsigned int fEnergyCutBin;
   double       fEnergyCut_Log10TeV_min;
   double       fEnergyCut_Log10TeV_max;
   TCut         fEnergyCut;
   unsigned int fEnergyReconstructionMethod;

   VTMVARunDataEnergyCut();
  ~VTMVARunDataEnergyCut() {}

   ClassDef(VTMVARunDataEnergyCut,1);
};

#endif

