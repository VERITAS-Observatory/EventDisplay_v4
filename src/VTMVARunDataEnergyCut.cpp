/*! \class VTMVARunDataEnergyCut
    \brief  VTMVARunDataEnergyCut data class for TMVA energy cuts


*/

#include "VTMVARunDataEnergyCut.h"

VTMVARunDataEnergyCut::VTMVARunDataEnergyCut()
{
   fEnergyCutBin = 0;
   fEnergyCut_Log10TeV_min = 0.;
   fEnergyCut_Log10TeV_max = 0;
   fEnergyCut = "";
   fEnergyReconstructionMethod = 0;
}


