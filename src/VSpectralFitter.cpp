/*! \class VSpectralFitter
    \brief  fitter class for energy spectra

    Revision $Id: VSpectralFitter.cpp,v 1.1.2.3.10.2 2010/03/08 07:49:52 gmaier Exp $

    \author Gernot Maier
*/

#include "VSpectralFitter.h"

ClassImp(VSpectralFitter)

VSpectralFitter::VSpectralFitter( string fitname )
{
    fFitFunction = 0;
    fFitFunction_lin = 0;
    fFitFunction_CovarianceMatrix = 0;
    fFitName = fitname;

    setSpectralFitFunction();
    setSpectralFitFluxNormalisationEnergy();
    setSpectralFitRangeLin();
    setPlottingStyle();
}


/*

    fit the current function

    (note the fit options)

*/
TF1* VSpectralFitter::fit( TGraph *g, string fitname )
{
    if( !g )
    {
        cout << "VSpectralFitter::fit warning: no graph" << endl;
        return 0;
    }
    if( fitname.size() > 0 ) fFitName = fitname;

// define fit function
    defineFitFunction();

// fit
    g->Fit( fFitFunction, "0MNER" );

    TVirtualFitter *fitter = TVirtualFitter::GetFitter();
    if( fitter )
    {
       int nPars = fitter->GetNumberFreeParameters();
       TMatrixD* COV = new TMatrixD( nPars, nPars, fitter->GetCovarianceMatrix() );
       if( COV ) fFitFunction_CovarianceMatrix = COV->GetMatrixArray();
    }

    updateFitFunction_lin();

    return fFitFunction;
}

/*

   define fit function

   These are predifined and can be set with a function ID:

   fSpectralFitFunction == 0 :  power law
   fSpectralFitFunction == 1 :  power law with exponential cutoff
   fSpectralFitFunction == 2 :  broken power law
   fSpectralFitFunction == 3 :  curved power law
*/
bool VSpectralFitter::defineFitFunction()
{
    char hname[2000];
    string iFitName_lin = fFitName + "_lin";

/////////////////////////////////////////////////////////
// power law
    if( fSpectralFitFunction == 0 )
    {
        cout << "Fitfunction: power law" << endl;
        sprintf( hname, "[0] * TMath::Power( TMath::Power( 10, x ) / %f, [1] )", fSpectralFitFluxNormalisationEnergy );
        fFitFunction = new TF1( fFitName.c_str(), hname, log10( fSpectralFitEnergy_min ), log10( fSpectralFitEnergy_max ) );
        fFitFunction->SetParameter( 0, 1.e-7 );
        fFitFunction->SetParameter( 1, -2.5 );
// linear axis
	sprintf( hname, "[0] * TMath::Power( x/ %f, [1] )", fSpectralFitFluxNormalisationEnergy );
	fFitFunction_lin = new TF1( iFitName_lin.c_str(), hname, fSpectralFitEnergy_min, fSpectralFitEnergy_max );
    }
/////////////////////////////////////////////////////////
// power law with exponential cutoff
    else if( fSpectralFitFunction == 1 )
    {
        cout << "Fitfunction: power law with exponential cutoff" << endl;
        sprintf( hname, "[0] * TMath::Power( TMath::Power( 10, x ) / %f, [1] ) * TMath::Exp( -1. * TMath::Power( 10, x ) / [3] )", fSpectralFitFluxNormalisationEnergy );
        fFitFunction = new TF1( fFitName.c_str(), hname, log10( fSpectralFitEnergy_min ), log10( fSpectralFitEnergy_max ) );
        fFitFunction->SetParameter( 0, 1.e-7 );
        fFitFunction->SetParameter( 1, -2. );
        fFitFunction->SetParameter( 2, 10. );
        sprintf( hname, "[0] * TMath::Power( x  / %f, [1] ) * TMath::Exp( -1. * x / [3] )", fSpectralFitFluxNormalisationEnergy );
	fFitFunction_lin = new TF1( iFitName_lin.c_str(), hname, fSpectralFitEnergy_min, fSpectralFitEnergy_max );
    }
// broken power law fit
    else if( fSpectralFitFunction == 2 )
    {
        cout << "broken power law fit NOT YET IMPLEMENTED (feel free to do this)" << endl;
        fFitFunction = 0;
	fFitFunction_lin = 0;
        return false;
    }
//curved power law fit
    else if( fSpectralFitFunction == 3 )
    {
      cout << "curved power law fit" << endl;
      sprintf( hname, "[0] * TMath::Power( TMath::Power( 10, x ) / %f, [1]+[2]*TMath::Power( 10, x ) )", fSpectralFitFluxNormalisationEnergy );
      fFitFunction = new TF1( fFitName.c_str(), hname, log10( fSpectralFitEnergy_min), log10( fSpectralFitEnergy_max));
      fFitFunction->SetParameter( 0, 1.e-7 );
      fFitFunction->SetParameter( 1, -2. );
      fFitFunction->SetParameter( 2, -0.01 );
    }
    else
    {
        cout << "VSpectralFitter::defineFitFunction: unknown spectral fit function: " << fSpectralFitFunction << endl;
        return false;
    }

// set all parameters for the function with linear energy axis
    updateFitFunction_lin();

// set plotting style
    if( fFitFunction )
    {
        fFitFunction->SetLineStyle( fPlottingEnergySpectrumLineStyle );
        fFitFunction->SetLineColor( fPlottingEnergySpectrumLineColor );
        fFitFunction->SetLineWidth( (Width_t)fPlottingEnergySpectrumLineWidth );
    }

    return true;
}

void VSpectralFitter::updateFitFunction_lin()
{
   if( !fFitFunction || !fFitFunction_lin ) return;

   for( int i = 0; i < fFitFunction->GetNpar(); i++ )
   {
      fFitFunction_lin->SetParameter( i, fFitFunction->GetParameter( i ) );
      fFitFunction_lin->SetParError( i, fFitFunction->GetParError( i ) );
   }
}


void VSpectralFitter::print()
{
    if( !fFitFunction ) return;

    cout << endl;
    if( fSpectralFitFunction == 0 )
    {
        cout << "Results for power law fit: " << endl;
        cout << "--------------------------" << endl;
        cout << "dN/dE = I x (E/" << fSpectralFitFluxNormalisationEnergy << " TeV)^{-Gamma}" << endl;
        cout << "I = " << scientific << fFitFunction->GetParameter( 0 ) << " +- " << fFitFunction->GetParError( 0 ) << " cm^-2s^-1TeV^-1" << endl;
        cout << "Gamma = " << fixed << fFitFunction->GetParameter( 1 ) << " +- " << fFitFunction->GetParError( 1 ) << endl;
        cout << "Chi2 " << fFitFunction->GetChisquare();
        cout << ", N = " << fFitFunction->GetNDF();
        if( fFitFunction->GetNDF() > 0. ) cout << " (Chi2/N=" << fFitFunction->GetChisquare()/fFitFunction->GetNDF() << ")" << endl;
        cout << endl;
    }
}

/*

   get integral flux from fit function

   iMinEnergy_TeV: threshold for integral flux in TeV (lin axis)

*/
double VSpectralFitter::getIntegralFlux( double iMinEnergy_TeV, double iMaxEnergy_TeV )
{
   if( !fFitFunction_lin )
   {
      cout << "VSpectralFitter::getIntegralFlux(): error: no fit function" << endl;
      return -99999.;
   }

// analytical calculation for power law (fSpectralFitFunction == 0)
/*   if( fSpectralFitFunction == 0 )
   {
       if( fFitFunction_lin->GetParameter( 1 ) != -1. ) 
       {
          double iFL = -1.* fFitFunction_lin->GetParameter( 0 ) / (fFitFunction_lin->GetParameter( 1 ) + 1.) / fSpectralFitFluxNormalisationEnergy;
	  iFL *= TMath::Power( iMinEnergy_TeV / fSpectralFitFluxNormalisationEnergy, fFitFunction_lin->GetParameter( 1 ) + 1. );
	  return iFL;
       }
    }     */

    return fFitFunction_lin->Integral( iMinEnergy_TeV, iMaxEnergy_TeV, fFitFunction_lin->GetParameters(), 1.e-30 );
}

/*

   get integral flux from fit function

   iMinEnergy_TeV: threshold for integral flux in TeV (lin axis)

*/
double VSpectralFitter::getIntegralFluxError( double iMinEnergy_TeV, double iMaxEnergy_TeV )
{
   if( !fFitFunction_lin || !fFitFunction )
   {
      cout << "VSpectralFitter::getIntegralFlux(): error: no fit function" << endl;
      return -99999.;
   }

   return fFitFunction_lin->IntegralError( iMinEnergy_TeV, iMaxEnergy_TeV, fFitFunction_lin->GetParameters(), fFitFunction_CovarianceMatrix );
}

