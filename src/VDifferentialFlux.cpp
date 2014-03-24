/*! \class VDifferentialFlux
    \brief data and converter class for differential flux values


    \author Gernot Maier
*/

#include "VDifferentialFlux.h"

VDifferentialFlux::VDifferentialFlux()
{
// constants
	constant_TeVtoHz = TMath::C() * 1.e12 /  1.239841875e-6;
	
	MJD_min = 0.;
	MJD_max = 0.;
	Energy = 0.;
	Energy_lowEdge = 0.;
	Energy_upEdge = 0.;
	Energy_lowEdge_bin = 0;
	Energy_upEdge_bin = 0;
	Energy_Hz = 0.;
	EnergyWeightedMean = 0.;
	dE = 0.;
	DifferentialFlux = 0.;
	DifferentialFluxError = 0.;
	DifferentialFluxError_low = 0.;
	DifferentialFluxError_up = 0.;
	DifferentialFlux_vFv = 0.;
	DifferentialFluxError_vFv = 0.;
	DifferentialFluxError_low_vFv = 0.;
	DifferentialFluxError_up_vFv = 0.;
	ObsTime = 0.;
	NOn = 0.;
	NOn_error = 0.;
	NOff = 0.;
	NOff_error = 0.;
	NOff_alpha = 1.;
	Significance = 0.;
}


/*
    print all data

    bSED = true: print vF_v fluxes to be used in VSpectralEnergyDistribution
*/
void VDifferentialFlux::print( bool bSED )
{

	if( !bSED )
	{
		cout << "E: " << setprecision( 2 ) << setw( 4 ) << Energy << " [TeV]";
		cout << " (dE = (" << Energy_lowEdge << "-" << Energy_upEdge << ")TeV";
		cout << " = " << dE << " TeV)";
		if( DifferentialFluxError > 0. )
		{
			cout << scientific << setprecision( 2 ) <<  "\tdiff flux: " << DifferentialFlux << " +- " << DifferentialFluxError;
		}
		else
		{
			cout << scientific << setprecision( 2 ) << "\tUL: " << DifferentialFlux;
		}
		cout << " [1/cm^2/s/TeV]" << endl;
		cout << setw( 7 ) << fixed << setprecision( 1 ) << "NOn: " << NOn << "+-" << NOn_error;
		cout << setw( 7 ) << fixed << setprecision( 1 ) << "\tNOff: " << NOff << "+-" << NOff_error;
		cout << setprecision( 2 ) << " (alpha=" << NOff_alpha << ")";
		cout << setw( 7 ) << fixed << setprecision( 1 ) << "\tSign.: " << Significance << " sigma";
		cout << setw( 7 ) << fixed << setprecision( 1 ) << "\tObs.Time: " << ObsTime << "[s]";
		cout << endl;
	}
	else
	{
		if( Energy_Hz > 0. )
		{
			cout << scientific;
			cout << setprecision( 8 ) << MJD_min << "\t" << MJD_max << "\t";
			cout << setprecision( 3 ) << Energy_Hz << "\t";
			cout << DifferentialFlux_vFv << "\t";
			cout << DifferentialFluxError_vFv;
			cout << endl;
		}
	}
}


/*
    print all data but without texts, so that the output can be copied
    for printing externally. e.g. in gnuplot

    bSED = true: print vF_v fluxes to be used in VSpectralEnergyDistribution
*/
void VDifferentialFlux::printClean( bool bSED )
{
	if( !bSED )
	{
		cout <<  setprecision( 3 ) << setw( 7 ) << Energy;
		cout << "    " << Energy_lowEdge << "    " << Energy_upEdge ;
		cout << "    " << dE ;
		if( DifferentialFluxError > 0. )
		{
			cout << scientific << setprecision( 3 ) <<  "\t " << DifferentialFlux << "   " << DifferentialFluxError;
		}
		else
		{
			cout << scientific << setprecision( 3 ) << "\t " << DifferentialFlux <<  "   0.0  ";
		}
		cout << "   ";
		cout << setw( 7 ) << fixed << setprecision( 1 ) << "\t " << NOn << " " << NOn_error;
		cout << setw( 7 ) << fixed << setprecision( 1 ) << "   " << NOff << " " << NOff_error;
		cout << setprecision( 2 ) << "     " << NOff_alpha ;
		cout << setw( 7 ) << fixed << setprecision( 1 ) << "    " << Significance ;
		cout << setw( 7 ) << fixed << setprecision( 1 ) << "    " << ObsTime ;
		cout << endl;
	}
	else
	{
		if( Energy_Hz > 0. )
		{
			cout << scientific;
			cout << setprecision( 8 ) << MJD_min << "\t" << MJD_max << "\t";
			cout << setprecision( 3 ) << Energy_Hz << "\t";
			cout << DifferentialFlux_vFv << "\t";
			cout << DifferentialFluxError_vFv;
			cout << endl;
		}
	}
}


/*
    calculate energy in Hz, vF_v, etc...
*/
void VDifferentialFlux::fillEvent( double iMJD_min, double iMJD_max )
{
	MJD_min = iMJD_min;
	MJD_max = iMJD_max;
	
	Energy_Hz = convertEnergy_TeV_to_Hz( Energy );
	
	DifferentialFlux_vFv      = convertPhotonFlux_to_Ergs( Energy, DifferentialFlux );
	DifferentialFluxError_vFv = convertPhotonFlux_to_Ergs( Energy, DifferentialFluxError );
}


/*
   convert energies from TeV to Hz

   linear energy
*/
double VDifferentialFlux::convertEnergy_TeV_to_Hz( double e )
{
	return e * constant_TeVtoHz;
}

/*
   convert energies from keV to Hz

   linear energy
*/
double VDifferentialFlux::convertEnergy_keV_to_Hz( double e )
{
	return e * 1.e-9 * constant_TeVtoHz;
}


/*
    convert fluxes in 1/cm2/s to ergs/cm2/s

    e    :    energy in TeV
    f    :    flux per energy bin in 1/cm2/s
*/
double VDifferentialFlux::convertPhotonFlux_to_Ergs( double e, double f, bool bLin )
{
	if( bLin )
	{
		e *= 1.e12;
	}
	else
	{
		e = TMath::Power( 10., e ) * 1.e12;
	}
// eV / cm2 / s
	f *= e * e / 1.e12;
	
// eV -> J
	f *= TMath::Qe();
// J -> ergs
	f /= 1.e-7;
	
	return f;
}

/*

   calculate nuFnu from an integral flux point

   energy in eV

   assume power laws dN/dE = c * E^{gamma}

   return value in [erg/cm2/s]

*/
double VDifferentialFlux::nuFnu( double F, double gamma, double e1, double e2, double e3 )
{
	if( e1 >= e2 )
	{
		cout << "invalid energy interval " << e1 << ", " << e2 << endl;
		return -99.;
	}
	
// calculate constant
	double c = 0.;
	if( gamma != -1. )
	{
		c = F * ( gamma + 1. ) * TMath::Power( e3, gamma ) / ( TMath::Power( e2, gamma + 1. ) - TMath::Power( e1, gamma + 1. ) );
	}
// (not correct)
	else
	{
		c = F * ( log( e2 ) - log( e1 ) );
	}
	
// calculate nuFu
	double nF = 0.;
	if( e3 > 0. )
	{
		nF = c * TMath::Power( e3 / e3, gamma ) * e3 * e3;
	}
// Following A.Tramacere (Fermi Saas Fee analysis session; 2010)
// http://www.isdc.unige.ch/sf2010/fermi
	else
	{
		nF = c * TMath::Power( sqrt( e1 * e2 ) / e3, gamma + 2. );
	}
	
// from eV to ergs
	nF *= TMath::Qe();
	nF /= 1.e-7;
	
	return nF;
}
