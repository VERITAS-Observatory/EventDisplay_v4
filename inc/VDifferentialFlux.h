//! VDifferentialFlux data and converter class for differential flux values
// Revision $Id: VDifferentialFlux.h,v 1.1.2.4.2.1.4.1.2.1.2.3 2010/08/27 09:55:19 gmaier Exp $

#ifndef VDifferentialFlux_H
#define VDifferentialFlux_H

#include "TMath.h"
#include "TObject.h"

#include <iomanip>
#include <iostream>

using namespace std;

class VDifferentialFlux : public TObject
{
    private:
        double constant_TeVtoHz;

    public:

        double MJD_min;                           // time range of observations
        double MJD_max;                           // time range of observations
        double Energy;                            // [TeV]
        double Energy_lowEdge;                    // lower bound of energy bin [TeV]
        double Energy_upEdge;                     // upper bound of energy bin [TeV]
        unsigned int Energy_lowEdge_bin;
        unsigned int Energy_upEdge_bin;
        double Energy_Hz;                         // [Hz]
        double EnergyWeightedMean;                // [TeV]
        double dE;                                // size of energy bin (in TeV)
        double DifferentialFlux;                  // [1/cm2/s/TeV]
        double DifferentialFluxError;             // [1/cm2/s/TeV]
        double DifferentialFluxError_up;             // [1/cm2/s/TeV]
        double DifferentialFluxError_low;             // [1/cm2/s/TeV]
        double DifferentialFlux_vFv;              // vF_v [ergs/cm2/s]
        double DifferentialFluxError_vFv;         // error in vF_v [ergs/cm2/s]
        double DifferentialFluxError_up_vFv;         // error in vF_v [ergs/cm2/s]
        double DifferentialFluxError_low_vFv;         // error in vF_v [ergs/cm2/s]
        double ObsTime;                           // observation time [s]
        double NOn;
	double NOn_error;
        double NOff;
	double NOff_error;
	double NOff_alpha;
        double Significance;

        VDifferentialFlux();
        ~VDifferentialFlux() {}
        void fillEvent( double iMinMJD = 0., double iMaxMJD = 1.e14 );  // calculate vFv etc., fill MJDs, etc.
        void print( bool bSED = false );

        double convertEnergy_keV_to_Hz( double energy_keV );
        double convertEnergy_TeV_to_Hz( double energy_TeV );
        double convertPhotonFlux_to_Ergs( double energy_TeV, double flux_photons_per_cm2_s, bool bLin = true );
	double nuFnu( double F, double gamma, double e1, double e2, double e3 = -9999. );

        ClassDef(VDifferentialFlux,6);
};
#endif
