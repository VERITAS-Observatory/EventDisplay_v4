// VSpectralWeight calculate energy dependent spectral weight

#ifndef VSpectralWeight_H
#define VSpectralWeight_H

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "TMath.h"
#include "TObject.h"

using namespace std;

class VSpectralWeight : public TObject
{
   private:

   bool fDebug;

   double fMCSpectralIndex;
   double fMCMinEnergy_TeV_Lin;
   double fMCMaxEnergy_TeV_Lin;

   double fIndex;
   double fSpectralWeightAlpha;


   public:

   VSpectralWeight();
  ~VSpectralWeight() {}

   double getSpectralWeight( double iEnergy_TeV_Lin );
   void   print();
   void   setMCParameter( double iMCSpectralIndex = 2., double iMCEnergy_min_TeV_Lin = 0.03, double iMCEnergy_max_TeV_Lin = 200. );
   void   setSpectralIndex( double iG, bool iPrint = false );

   ClassDef(VSpectralWeight,1);
};

#endif
