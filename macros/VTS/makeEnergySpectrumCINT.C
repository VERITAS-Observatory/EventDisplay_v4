/* \file  makeEnergySpectrum.C

   \author Bagmeet Behera (bagmeet.beheraesy.de)
   \Version 1.0 (20 February 2013)

   \brief Macro to set spectral bimnning and fit parameters to do a spectral fit on a anasum results file, e.g. <runnum>.anasum.root

   *** under development ***
   Starting point to automize the spectral reconstruction: call this macro in a script with anasum and iterate for different results.

   Notes:       
      This is the CINT version

   To Do:
      1. Provide or extend existing xxxLinkDef.h files to make a dictionary to use iterators of vector <VDifferentialFlux> to get a executable
         and  vector <VRunList>. This could eventually be incorporated to libVAnaSum.so
      2. Adapt to use a key file instaed of a long command line argument list


*/

//.L $EVNDISPSYS/lib/libVAnaSum.so;

/*
# include "TCanvas.h"  

# include "/afs/ifh.de/group/cta/scratch/bagmeet/VERITAS/EVNDISP/EVNDISP-400/trunk/inc/VEnergySpectrum.h"
# include "/afs/ifh.de/group/cta/scratch/bagmeet/VERITAS/EVNDISP/EVNDISP-400/trunk/inc/VFluxCalculation.h"
# include "/afs/ifh.de/group/cta/scratch/bagmeet/VERITAS/EVNDISP/EVNDISP-400/trunk/inc/VTargets.h"
*/




int makeEnergySpectrum(char * anasumrootfile,
		       int    fitFunc = 0,                        // 0 PowerLaw, 1 PL exp CO, 2 Broken PL, 3 Curved PL  
		       double Ebins = 0.1,                        // Bin width, (1) adjust to get N_on >=5, & N_off >= 5 to have good statistics, &
		                                                  //            (2) also check that there are no UL in between flux points
		       double gamma = -2.73,                      // make sure in the ANASUM you have used the value from the results of this function and reiterated 
		       double Ethr = 0.2,                         // only used for calculating the Integral flux
		       double Enorm = 0.4,                        // Fixing the normalization energy, check that it is > E_threshold (but within max energy with good stats)
		       double Emin = 0.15, double Emax = 30.,     // Range of Energies (TeV) for spectral fittingmaking energy-binned fluxes (OK to usually leave with defaults)
		       double EminFit = 0.1, double EmaxFit = 50.,// min and max energies for the fit
		       double Earea_fraction_threshold = 0.1,     // definine threshold as the energy where A_eff falls a fraction of the maximum A_Eff
		       double minSigmaPerBin = 1.5, double minEvtsPerBin = 5.0, // which bins to plot, according to the minimum sigma and excess-events per bin
		       double TimeBinLength_days = 1., double minSigmaLC = 2.0) // Params of light curve
{  
  double fitNormalization=-1.1, fitGamma = -1., intFlux=-1.1, intFlux2=-1.1, intFluxErr=-1.1, intFluxUL=-1.1;
  
  VEnergySpectrum  * e = new VEnergySpectrum(anasumrootfile);
  VFluxCalculation * f = new VFluxCalculation(anasumrootfile);
  
  //vector <VDifferentialFlux> diffFlux;
  //vector <VRunList> & runlist =   e->getRunList();

  int i = 0;

  e->setSpectralFitFunction(fitFunc);  // 0 Powerlaw, 1 PL exp CO, 2 BPL

  e->setSignificanceParameters( minSigmaPerBin, minEvtsPerBin );
  e->setEnergyBinning(Ebins);
  e->setPlottingSpectralWeightForBinCenter(gamma );   
  e->setSpectralFitFluxNormalisationEnergy(Enorm);     // flux normalization energy for fitting  

  e->setEnergyRangeLinear(Emin, Emax);               // range over which flux points will be plotted

  // Note: Do not over restrict the fit-range or it will make the result unstable - and correlated with binning chosen
  e->setSpectralFitRangeLin(EminFit, EmaxFit);             // range over which spectral fitting will be performed

  // The following is reccommended to get the stable and consistent results
  e->setEnergyThresholdDefinition( 2, -1., Earea_fraction_threshold);    // set threshold to the energy at which a certain fraction 
                                                                         // of the maximum of the effective area is reached ~ 10% of 
                                                                         // the max Eff Area here

  // The following is not recommended in normal cases. Bins used for the finding differential flux
  // do not get calculated properly, biasing the results.
  //e->setEnergyThresholdDefinition(3);                // needed before next stmt  
  //e->setEnergyThreshold(Ethr);                        // set threshold energy specifically (TeV)
  
  
  e->plotLifeTimevsEnergy();

  TCanvas *c2 = e->plot();
  // Plot bins and event details
  e->plotEventNumbers();

  e->printEnergyBins();

  /*
  // Get Threshold energies of runs
  i=0;
  for(vector<VRunList>::iterator it = runlist.begin(); it !=runlist.end(); ++it)
    {      
      fprintf(stderr, " Energy Threshold for run id %d, is %0.3f GeV", i+1, runlist[i].energyThreshold*1.e3); 
      ++i;
    }
  */

  // Perform the spectral fit
  e->fitEnergySpectrum();

  // Print fit results
  e->plotFitValues();

  // Get the fit function object and hence the fit params
  TF1 * fitfunc = e->getSpectralFitFunction();

  fprintf(stderr, "\n Number of free params in the fit is %d \n", fitfunc->GetNumberFreeParameters());
  
  for (i=0; i<fitfunc->GetNumberFreeParameters(); i++)
    {
      fprintf(stderr, "   param[%d] = %0.3e +- %0.3e\n", i, fitfunc->GetParameter(i), fitfunc->GetParError(i) );
      if(i==0)
	fitNormalization = fitfunc->GetParameter(i);
      else if(i==1)
	fitGamma = fitfunc->GetParameter(i);
    }
  fprintf(stderr, "\n");

  e->printEnergyThresholds();
  e->printDifferentialFluxes();

  /*
  // Get the differential flux
  i = 0;
  diffFlux = e->getDifferentialFlux();
  for(vector<VDifferentialFlux>::iterator it = diffFlux.begin(); it !=diffFlux.end(); ++it)
    //for (int it; it< diffFlux.size(); it++)
    {
      //fprintf(stderr, "just iterating \n");
      //diffFlux[i].print();
      
      fprintf(stderr, "E[%0.3f, %0.3f] : N(on/off)(%f/%f) : Flux = %0.3e +- %0.3e \n",
	      diffFlux[i].Energy_lowEdge, diffFlux[i].Energy_upEdge,
	      diffFlux[i].NOn, diffFlux[i].NOff,
	      diffFlux[i].DifferentialFlux, diffFlux[i].DifferentialFluxError);
      
      ++i;
    }
  
  // Get Threshold energies of runs
  i=0;
  for(vector<VRunList>::iterator it = runlist.begin(); it !=runlist.end(); ++it)
    //for (int it; it< diffFlux.size(); it++)
    {
      //fprintf(stderr, "just iterating \n");
      //diffFlux[i].print();
      
      fprintf(stderr, " Energy Threshold for run id %d, is %0.3f GeV", i+1, runlist[i].energyThreshold*1.e3);
      
      ++i;
    }
  */

  // Calculate Integral Fluxes
  f->setSpectralParameters( EminFit, Enorm, gamma);
  f->setTimeBinnedAnalysis(true);
  f->calculateIntegralFlux(Ethr);
  f->printResults();
  //f->plotFluxesVSMJD(); 
  f->plotFluxesVSMJDDaily(); 

  f->getFlux(-1, intFlux, intFluxErr, intFluxUL);
  
  if(intFluxUL==-99.)
    fprintf(stderr, " Integral flux = %.3e +- %.3e\n", intFlux, intFluxErr);  

  VLightCurve *iLightCurve = new VLightCurve();
  iLightCurve->initializeTeVLightCurve( anasumrootfile, TimeBinLength_days );  // e.g. TimeBineLength_days=1 for daily binning
  iLightCurve->setSignificanceParameters( minSigmaLC ); // plot ULs for poins with <2 sigma significance
  iLightCurve->fill( Ethr ); // calculate fluxes and upper flux limits for energies >0.2 GeV
  iLightCurve->plotLightCurve();   // plot the light curve
  //

  std::cout << "\nCoder's WARNING: If BPL is used - check the output of the fitter and note the Error Matrix is not from MINOS" << endl;
  std::cout << "NOTE: To try a different fit function exit ROOT and load again" << std::endl;

  
  delete e;
  delete f;
  delete iLightCurve;

  return 1;
}



/*
# ifndef __CINT__  // the following code will be invisible for the interpreter

int main(int argc, char * argv[])
{
  char * anasumrootfile;
  double Ebins = 0.1, gamma = -2.73, Ethr = 0.2, Enorm = 0.4, Emin = 0.1, Emax= 30.;
  
  if(argc<2)
    {
      fprintf(stderr, "ERROR: not enough inputs\n");
      exit(0);
    }
  
  for(int i=1; i< argc; i++)
    {

      switch (i)
	{
	case 1:
	  anasumrootfile = *(argv+1);
	  break;
	case 2:
	  Ebins = atof(*(argv+2));
	  break;
	case 3:
	  gamma = atof(*(argv+3));
	  break;
	case 4:
	  Ethr = atof(*(argv+4));
	  break;
	case 5:
	  Enorm = atof(*(argv+5));
	  break;
	case 6:
	  Emin = atof(*(argv+6));
	  break;
	case 7:
	  Emax = atof(*(argv+7));
	  break;
	default:
	  break;
	}
      
    }

  fprintf(stderr, "Input file name :%s\n", anasumrootfile);

  // Call function here
  makeEnergySpectrum(anasumrootfile, Ebins, gamma, Ethr, Enorm, Emin, Emax);

  return 0;
}
 
# endif
*/
