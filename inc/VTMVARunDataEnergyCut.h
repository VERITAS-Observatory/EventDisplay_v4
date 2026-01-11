//! VTMVARunDataEnergyCut data class for TMVA energy cuts

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
        unsigned int fDirectionReconstructionMethod;

        VTMVARunDataEnergyCut();
        ~VTMVARunDataEnergyCut() {}

        void print();

        ClassDef( VTMVARunDataEnergyCut, 3 );
};

#endif
