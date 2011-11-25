/*! \class VPlotWPPhysSensitivity

*/

#include "VPlotWPPhysSensitivity.h"

VPlotWPPhysSensitivity::VPlotWPPhysSensitivity()
{
   fIRF = 0;
}


void VPlotWPPhysSensitivity::addAnalysis( string iAnalysis, int iColor )
{
   fAnalysis.push_back( iAnalysis );
   fAnalysisColor.push_back( iColor );
}

void VPlotWPPhysSensitivity::addObservationTime( double iObsTime )
{
   fObservationTime_H.push_back( iObsTime );
}

void VPlotWPPhysSensitivity::addSubArray( string iArray )
{
   fSubArray.push_back( iArray );
}

bool VPlotWPPhysSensitivity::initialize()
{
   char hname[200];
   fSensitivityFile.clear();
   fPlottingColor.clear();
   fPlottingLineStyle.clear();

   for( unsigned int a = 0; a < fSubArray.size(); a++ )
   {
      for( unsigned int t = 0; t < fObservationTime_H.size(); t++ )
      {
	 for( unsigned int i = 0; i < fAnalysis.size(); i++ )
	 {
	    ostringstream iTemp;
	    if( fAnalysis[i] == "DESY" )
	    {
	       sprintf( hname, "%.1f", fObservationTime_H[t] );
	       iTemp << "DESY_20111123/DESY." << fSubArray[a] << "." << hname << ".root";
            }
	    else if( fAnalysis[i] == "ISDC" )
	    {
	       sprintf( hname, "%.1f", fObservationTime_H[t] );
	       iTemp << "ISDC/ISDC_2000m_KonradB_optimal_"  << fSubArray[a] << "_" << hname;
	       iTemp << "h_20deg_20110615.root";
            }
	    else if( fAnalysis[i] == "IFAE" )
	    {
	       if( fObservationTime_H[t] > 1. ) sprintf( hname, "%d", (int)fObservationTime_H[t] );
	       else                             sprintf( hname, "%.1f", fObservationTime_H[t] );
	       iTemp << "IFAEPerformanceBCDEINANB_Nov2011/Subarray" << fSubArray[a];
	       iTemp << "_IFAE_" << hname << "hours_20111109.root";
	    }
	    else continue;
	    fSensitivityFile.push_back( iTemp.str() );
	    fPlottingColor.push_back( fAnalysisColor[i] );
	    fPlottingLineStyle.push_back( t+1 );
         }
      }
   }

// print data sets
   for( unsigned int i = 0; i < fSensitivityFile.size(); i++ )
   {
      cout << fSensitivityFile[i] << "\t" << fPlottingColor[i] << "\t" << fPlottingLineStyle[i] << endl;
   }

   return true;
}
  
bool VPlotWPPhysSensitivity::plotIRF()
{
    fIRF = new VPlotInstrumentResponseFunction();

    fIRF->setCanvasSize( 400, 400 );
    fIRF->setPlottingAxis( "energy_Lin", "X", true, 0.01, 200 );
    fIRF->setPlottingAxis( "effarea_Lin", "X", true, 50., 5.e7 );
    fIRF->setPlottingAxis( "energyresolution_Lin", "X", false, 0., 0.7 );

    for( unsigned int i = 0; i < fSensitivityFile.size(); i++ )
    {
       fIRF->addInstrumentResponseData( fSensitivityFile[i], 20., 0.5, 0, 2.4, 200, "A_MC", 
                                        fPlottingColor[i], fPlottingLineStyle[i], 21, 0.5 );
    }

    fIRF->plotEffectiveArea();
    fIRF->plotAngularResolution();
    fIRF->plotEnergyResolution( 0.7 );


   return true;
}

bool VPlotWPPhysSensitivity::plotSensitivity()
{
   string iCrabFile = "$EVNDISPDATA/AstroData/TeV_data/EnergySpectrum_literatureValues_CrabNebula.dat";
   unsigned int iCrabID = 6;

   TCanvas *c = 0;
   for( unsigned int i = 0; i < fSensitivityFile.size(); i++ )
   {
      VSensitivityCalculator *a = new VSensitivityCalculator();
      a->setMonteCarloParametersCTA_MC( fSensitivityFile[i], iCrabFile, iCrabID );
      a->setEnergyRange_Lin( 0.01, 200. );
      a->setPlotCanvasSize( 900, 600 );
      a->setPlottingStyle( fPlottingColor[i], fPlottingLineStyle[i], 2., 1 );
      TCanvas *c_temp = a->plotDifferentialSensitivityvsEnergyFromCrabSpectrum( c, "CTA-PHYS", fPlottingColor[i], "ENERGY", 0.2, 0.01 );
      if( c_temp ) c = c_temp;
   }

   return true;
}
