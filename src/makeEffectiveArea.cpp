/*! \file  makeEffectiveArea
 *  \brief get effective area from simulations
 *
 *   fill effective areas calculated from gamma-ray simulations for different zenith angles
 *
 *   input is a file list of mscw_energy output file from gamma-ray simulations
 *
 * \author
 *   Gernot Maier
 *
 *  Revision $Id: makeEffectiveArea.cpp,v 1.10.2.10.4.9.10.3.2.12.4.1.2.6.2.2.6.1.2.5.2.16.2.1.2.14 2011/04/08 13:57:28 gmaier Exp $
 */

#include "TChain.h"
#include "TChainElement.h"
#include "TFile.h"
#include "TH1D.h"
#include "TMath.h"
#include "TObjArray.h"
#include "TStopwatch.h"
#include "TTree.h"

#include "VGlobalRunParameter.h"
#include "CData.h"
#include "Ctelconfig.h"
#include "VGammaHadronCuts.h"
#include "VEffectiveAreaCalculatorMCHistograms.h"
#include "VEffectiveAreaCalculator.h"
#include "VInstrumentResponseFunction.h"
#include "VInstrumentResponseFunctionRunParameter.h"
#include "VMonteCarloRunHeader.h"
#include "VTableLookupRunParameter.h"

#include <fstream>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

VEffectiveAreaCalculatorMCHistograms* copyMCHistograms( TChain *c );

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{

    cout << endl;
    cout << "makeEffectiveArea " << VGlobalRunParameter::getEVNDISP_VERSION() << endl;
    cout << "-----------------------------" << endl;
    cout << endl;

/////////////////////////////////////////////////////////////////
// read command line parameters
    if( argc != 3 )
    {
	cout << endl;
	cout << "makeEffectiveArea <runparameter file> <output effective area file> " << endl;
	cout << endl;
	exit( 0 );
    }
    string fOutputfileName = argv[2];

/////////////////////////////////////////////////////////////////
// read run parameters from file
    VInstrumentResponseFunctionRunParameter *fRunPara = new VInstrumentResponseFunctionRunParameter();
    fRunPara->SetName( "makeEffectiveArea_runparameter" );
    if( !fRunPara->readRunParameterFromTextFile( argv[1] ) ) exit( -1 );
    fRunPara->print();

/////////////////////////////////////////////////////////////////
// open output file and write results to dist
    TFile *fOutputfile = new TFile( fOutputfileName.c_str(), "RECREATE" );
    if( fOutputfile->IsZombie() )
    {
        cout << "Error in opening output file: " << fOutputfile->GetName() << endl;
        exit( 0 );
    }

/////////////////////////////////////////////////////////////////
// gamma/hadron cuts
    VGammaHadronCuts *fCuts = new VGammaHadronCuts();
    fCuts->setNTel( fRunPara->telconfig_ntel, fRunPara->telconfig_arraycentre_X, fRunPara->telconfig_arraycentre_Y );
    if( !fCuts->readCuts( fRunPara->fCutFileName, 2 ) ) exit( -1 );
    fRunPara->fGammaHadronCutSelector = fCuts->getGammaHadronCutSelector();
    fRunPara->fDirectionCutSelector   = fCuts->getDirectionCutSelector();
    fCuts->initializeCuts( -1, fRunPara->fGammaHadronProbabilityFile );
    fCuts->printCutSummary();

/////////////////////////////////////////////////////////////////
// read MC header (might not be there, no problem; but depend on right input in runparameter file)
    VMonteCarloRunHeader *iMonteCarloHeader = fRunPara->readMCRunHeader();

/////////////////////////////////////////////////////////////////
// stopwatch to keep track of execution time
    TStopwatch fStopWatch;

/////////////////////////////////////////////////////////////////////////////
// set effective area class
    VEffectiveAreaCalculator fEffectiveAreaCalculator( fRunPara, fCuts );

/////////////////////////////////////////////////////////////////////////////
// set effective area Monte Carlo histogram class
    TFile *fMC_histoFile = 0;
    VEffectiveAreaCalculatorMCHistograms *fMC_histo = 0;

/////////////////////////////////////////////////////////////////////////////
// set angular, core, etc resolution calculation class
    vector< VInstrumentResponseFunction* > f_IRF;
    vector< string > f_IRF_Name;
    vector< string > f_IRF_Type;
    string fCuts_AngularResolutionName = "";
    if( fRunPara->fFillingMode != 3 )
    {
       f_IRF_Name.push_back( "angular_resolution" );            f_IRF_Type.push_back( "angular_resolution" );
       if( fCuts->getAngularResolutionContainmentRadius() )
       {
	  char hname[200];
	  sprintf( hname, "angular_resolution_0%dp", fCuts->getAngularResolutionContainmentRadius() );
	  fCuts_AngularResolutionName = hname;
          f_IRF_Name.push_back( fCuts_AngularResolutionName );       f_IRF_Type.push_back( "angular_resolution" );
       }
       if( fRunPara->fFillingMode != 2 )
       {
	  f_IRF_Name.push_back( "core_resolution" );               f_IRF_Type.push_back( "core_resolution" );
	  f_IRF_Name.push_back( "energy_resolution" );             f_IRF_Type.push_back( "energy_resolution" );
       }
    }
    for( unsigned int i = 0; i < f_IRF_Name.size(); i++ )
    {
        f_IRF.push_back( new VInstrumentResponseFunction() );
	f_IRF.back()->setRunParameter( fRunPara );
	if( fCuts_AngularResolutionName.size() > 0 && f_IRF_Name[i] == fCuts_AngularResolutionName )
	{
	   f_IRF.back()->setContainmentProbability( ((double)fCuts->getAngularResolutionContainmentRadius())/100. );
	   cout << "setting containment probability to " << f_IRF.back()->getContainmentProbability() << endl;
        }
	else
	{
	   f_IRF.back()->setContainmentProbability( 0.68 );
        }
        f_IRF.back()->initialize( f_IRF_Name[i], f_IRF_Type[i],
	                          fRunPara->telconfig_ntel, fRunPara->fCoreScatterRadius,
				  fRunPara->fze, fRunPara->fnoise, fRunPara->fpedvar, fRunPara->fXoff, fRunPara->fYoff );
    }


/////////////////////////////////////////////////////////////////////////////
// load data chain
    TChain *c = new TChain( "data" );
    if( !c->Add( fRunPara->fdatafile.c_str(), -1 ) )
    {
         cout << "Error while trying to add mscw data tree from file " << fRunPara->fdatafile  << endl;
         cout << "exiting..." << endl;
         exit( -1 );
    }

     CData d( c, true, 6, true );
     fCuts->setDataTree( &d );

     TH1D* hE0mc = (TH1D*)gDirectory->Get( "hE0mc" );

/////////////////////////////////////////////////////////////////////////////
// fill resolution plots
    for( unsigned int i = 0; i < f_IRF_Name.size(); i++ )
    {
        if( f_IRF[i] )
        {
           f_IRF[i]->setDataTree( &d );
           f_IRF[i]->setCuts( fCuts );
           f_IRF[i]->fill();
	   if( fCuts_AngularResolutionName.size() > 0 && f_IRF_Name[i] == fCuts_AngularResolutionName )
	   {
	     if( fCuts->getDirectionCutSelector() == 2 ) fCuts->setIRFGraph( f_IRF[i]->getAngularResolutionGraph( 0, 0 ) );
           }
        }
    }

/////////////////////////////////////////////////////////////////////////////
// calculate effective areas
     if( !fRunPara->fFillMCHistograms )
     {
// set azimuth bins and spectral index bins
// (make sure that spectral index is positive)
        fEffectiveAreaCalculator.initializeHistograms( fRunPara->fAzMin, fRunPara->fAzMax, fRunPara->fSpectralIndex );
     }

//////////////////////////////////////////////////////////////////////////////
// MC histograms
    if( fRunPara->fFillingMode != 1 && fRunPara->fFillingMode != 2 )
    {
        fStopWatch.Start();
	if( fRunPara->fMCdatafile_tree.size() == 0 && fRunPara->fMCdatafile_histo.size() == 0 )
	{
	   fMC_histo = copyMCHistograms( c );
	   if( fMC_histo )
	   {
	      fMC_histo->matchDataVectors( fRunPara->fAzMin, fRunPara->fAzMax, fRunPara->fSpectralIndex );
	      fMC_histo->print();
	   }
        }
// read MC histograms from a separate file
	else if( fRunPara->fMCdatafile_histo.size() > 0 )
	{
	   fMC_histoFile = new TFile( fRunPara->fMCdatafile_histo.c_str() );
	   if( fMC_histoFile->IsZombie() )
	   {
	       cout << "Error reading MC histograms from file " << fRunPara->fMCdatafile_histo << endl;
	       cout << "exiting..." << endl;
	       exit( -1 );
	   }
	   fMC_histo = (VEffectiveAreaCalculatorMCHistograms*)fMC_histoFile->Get( "MChistos" );
	   if( !fMC_histo )
	   {
	       cout << "Error reading MC histograms from file " << fRunPara->fMCdatafile_histo << " (no histograms)" << endl;
	       cout << "exiting..." << endl;
	       exit( -1 );
	   }
	   fMC_histo->matchDataVectors( fRunPara->fAzMin, fRunPara->fAzMax, fRunPara->fSpectralIndex );
	   fMC_histo->print();
	}
// recalulate MC spectra from MCpars tree. Very slow! 
       else if( fRunPara->fMCdatafile_tree.size() > 0 && fRunPara->fMCdatafile_tree != "0" )
       {
	   TChain *c2 = new TChain( "MCpars" );
	   if( !c2->Add( fRunPara->fMCdatafile_tree.c_str(), -1 ) )
	   {
	      cout << "Error while trying to read MC data file: " << fRunPara->fMCdatafile_tree << endl;
	      cout << "exiting..." << endl;
	      exit( -1 );
	   }
	   fMC_histo = new VEffectiveAreaCalculatorMCHistograms();
	   fMC_histo->setMonteCarloEnergyRange( fRunPara->fMCEnergy_min, fRunPara->fMCEnergy_max, TMath::Abs( fRunPara->fMCEnergy_index ) );
	   fMC_histo->initializeHistograms( fRunPara->fAzMin, fRunPara->fAzMax, fRunPara->fSpectralIndex, 
					    fRunPara->fEnergyAxisBins_log10, 
					    fEffectiveAreaCalculator.getEnergyAxis_minimum_defaultValue(), 
					    fEffectiveAreaCalculator.getEnergyAxis_maximum_defaultValue() );
	   fMC_histo->fill( fRunPara->fze, c2, fRunPara->fAzimuthBins );
	   fMC_histo->print();
	   fOutputfile->cd();
	   cout << "writing MC histograms to file " << fOutputfile->GetName() << endl;
	   fMC_histo->Write();
	}
        fStopWatch.Print();
     }  

// fill effective areas
     if( !fRunPara->fFillMCHistograms && fRunPara->fFillingMode != 1 && fRunPara->fFillingMode != 2 )
     {
	fOutputfile->cd();
        fEffectiveAreaCalculator.fill( hE0mc, &d, fMC_histo, fRunPara->fEnergyReconstructionMethod );
        fStopWatch.Print();
     }

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// write results to disk
    if( !fRunPara->fFillMCHistograms )
    {
       if( fEffectiveAreaCalculator.getTree() )
       {
          cout << "writing effective areas (" << fEffectiveAreaCalculator.getTree()->GetName() << ") to " << fOutputfile->GetName() << endl;
          fOutputfile->cd();
          fEffectiveAreaCalculator.getTree()->Write();
       }
       else
       {
          cout << "error: no effective area tree found" << endl;
       }
       if( fEffectiveAreaCalculator.getHistogramhEmc() ) fEffectiveAreaCalculator.getHistogramhEmc()->Write();
    }
    for( unsigned int i = 0; i < f_IRF_Name.size(); i++ )
    {
        if( f_IRF[i] && f_IRF[i]->getDataProduct() )
        {
           f_IRF[i]->getDataProduct()->Write();
        }
    }
// writing cuts to disk
    if( fCuts ) 
    {
       fCuts->terminate();
    } 
// writing monte carlo header to disk
    if( iMonteCarloHeader ) iMonteCarloHeader->Write();

// write run parameters to disk
    if( fRunPara ) fRunPara->Write();

    fOutputfile->Close();
    cout << "end..." << endl;
}

VEffectiveAreaCalculatorMCHistograms* copyMCHistograms( TChain *c )
{
   VEffectiveAreaCalculatorMCHistograms *iMC_his = 0;
   if( c )
   {
// loop over all files and add MC histograms
        TObjArray *fileElements = c->GetListOfFiles();
        TChainElement *chEl=0;
        TIter next(fileElements);
	unsigned int z = 0;
	while( (chEl = (TChainElement*)next()) )
	{
           TFile *ifInput = new TFile( chEl->GetTitle() );
	   if( !ifInput->IsZombie() )
	   {
	      if( z == 0 )
	      {
	         iMC_his = (VEffectiveAreaCalculatorMCHistograms*)ifInput->Get( "MChistos" );
              }
	      else
	      {
	         if( iMC_his )
		 {
		    iMC_his->add( (VEffectiveAreaCalculatorMCHistograms*)ifInput->Get( "MChistos" ) );
                 }
		 ifInput->Close();
              }
	      z++;
           }
        }
   }
   return iMC_his;
}


