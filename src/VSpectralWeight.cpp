/*! \file   getEnergyWeighting
    \brief calculate energy dependent spectral weight

    general assumption: MC is simulated according to a power law between to energies

    use power laws only (but can easily be expanded for arbitrary spectral shapes)

*/

#include "VSpectralWeight.h"

VSpectralWeight::VSpectralWeight()
{
    fDebug = false;

    fIndex = 2.;
    fSpectralWeightAlpha = 1.;

    setMCParameter();
}

void VSpectralWeight::setMCParameter( double iMCSpectralIndex, double iMCEnergy_min_TeV_Lin, double iMCEnergy_max_TeV_Lin )
{
   fMCSpectralIndex = iMCSpectralIndex;
   fMCMinEnergy_TeV_Lin = iMCEnergy_min_TeV_Lin;
   fMCMaxEnergy_TeV_Lin = iMCEnergy_max_TeV_Lin;
}

/*
    weights will be always > 1
*/
void VSpectralWeight::setSpectralIndex( double iG, bool iPrint )
{
    fIndex = iG;

    if( iPrint ) cout << "weighting events to spectral index of " << fIndex << endl;

    if( fabs( fIndex - fMCSpectralIndex ) < 0.02 ) fSpectralWeightAlpha = 1.;
    else if( fIndex > fMCSpectralIndex )
    {
       fSpectralWeightAlpha = TMath::Power( fMCMaxEnergy_TeV_Lin, -1.*fMCSpectralIndex ) / TMath::Power( fMCMaxEnergy_TeV_Lin, -1.*fIndex );
    }
    else if( fIndex < fMCSpectralIndex )
    {
       fSpectralWeightAlpha = TMath::Power( fMCMinEnergy_TeV_Lin, -1.*fMCSpectralIndex ) / TMath::Power( fMCMinEnergy_TeV_Lin, -1.*fIndex );
    }
}


double VSpectralWeight::getSpectralWeight( double iE )
{
    if( fabs( fIndex - fMCSpectralIndex ) < 0.01 ) return 1.;
    else 
    {
       return fSpectralWeightAlpha * TMath::Power( iE, -1.*fIndex + fMCSpectralIndex );
    }
    return 1.;
}    

void VSpectralWeight::print()
{ 
   cout << "VSpectralWeight: "; 
   cout << "expect input (MC) energy spectra with index " << fMCSpectralIndex;
   cout << " in energy range [" << fMCMinEnergy_TeV_Lin << ", " << fMCMaxEnergy_TeV_Lin << "] TeV";
   cout << endl;
}
