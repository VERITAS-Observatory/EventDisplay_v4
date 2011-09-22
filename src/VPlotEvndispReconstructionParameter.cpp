/*! \file VPlotEvndispReconstructionParameter
    \brief plot eventdisplay reconstruction parameters

    ***** CTA *****

    plot events lossed due to image quality cuts applied before the stereo reconstruction

    Note: not all cuts are implemented yet

    \Author Gernot Maier
*/

#include "VPlotEvndispReconstructionParameter.h"

VPlotEvndispReconstructionParameter::VPlotEvndispReconstructionParameter()
{
   setDebug( false );

   fDataChain = 0;
   fDataFileName = "";
   fDetectorGeometry = 0;
   fDataFile = 0;
   fPlotCanvas = 0;
   fDataTpars = 0;
   fDataShowerPars = 0;
   fEvndispReconstructionParameter = 0;
}

bool VPlotEvndispReconstructionParameter::initialize( string iEventdisplayFileName, 
                                                      int iNEnergyBins, double iEnergy_TeV_log_min, double iEnergy_TeV_log_max )
{
   fDataFileName = iEventdisplayFileName;

   fDataChain = new TChain( "showerpars" );
   fDataChain->Add( fDataFileName.c_str() );
   if( !fDataChain )
   {
      cout << "VPlotEvndispReconstructionParameter::initialize() error:";
      cout << "file/chain " << fDataFileName << " not found" << endl;
      return false;
   }
   if( fDataChain->GetFile() )
   {
      fDataFile = new TFile( fDataChain->GetFile()->GetName() );
   }
   if( !fDataFile || fDataFile->IsZombie() )
   {
      cout << "VPlotEvndispReconstructionParameter::initialize() error:";
      cout << "file " << fDataFileName << " not found" << endl;
      return false;
   }

// read in EvndispReconstructionParameter
   fEvndispReconstructionParameter = (VEvndispReconstructionParameter*)fDataFile->Get( "EvndispReconstructionParameter" );
   if( !fEvndispReconstructionParameter )
   {
      cout << "VPlotEvndispReconstructionParameter::initialize() error:";
      cout << " no reconstruction parameters found" << endl;
      return false;
   }

// get telescope type
   TTree *i_telconfig = (TTree*)fDataFile->Get( "telconfig" );
   if( !i_telconfig )
   {
      cout << "VPlotEvndispReconstructionParameter::initialize() error:";
      cout << " no telescope configuration found" << endl;
      return false;
   }
   fDetectorGeometry = new VDetectorGeometry( i_telconfig->GetEntries(), false );
   VDetectorTree i_DetectorTree;
   i_DetectorTree.readDetectorTree( fDetectorGeometry, i_telconfig ); 

   fEvndispReconstructionParameterName.push_back( "noCut" );
   fEvndispReconstructionParameterName.push_back( "Size_min" );
   fEvndispReconstructionParameterName.push_back( "Ntubes_min" );
   fEvndispReconstructionParameterName.push_back( "Loss_max" );
   fEvndispReconstructionParameterName.push_back( "Fui_min" );
   fEvndispReconstructionParameterName.push_back( "Saturated_max" );
/*   fEvndispReconstructionParameterName.push_back( "length_min" );
   fEvndispReconstructionParameterName.push_back( "length_max" );
   fEvndispReconstructionParameterName.push_back( "width_min" );
   fEvndispReconstructionParameterName.push_back( "width_max" );
   fEvndispReconstructionParameterName.push_back( "dist_min" );
   fEvndispReconstructionParameterName.push_back( "dist_max" );
   fEvndispReconstructionParameterName.push_back( "widthlength_max" ); */

   char hname[200];
   for( unsigned int i = 0; i < fEvndispReconstructionParameterName.size(); i++ )
   {
      sprintf( hname, "h_%s", fEvndispReconstructionParameterName[i].c_str() );
      fEvndispReconstructionParameterHisto.push_back( new TH1D( hname, "", iNEnergyBins, iEnergy_TeV_log_min, iEnergy_TeV_log_max ) );
      fEvndispReconstructionParameterHisto.back()->SetXTitle( "log_{10} energy [TeV]" );
      fEvndispReconstructionParameterHisto.back()->SetStats( 0 );
      fEvndispReconstructionParameterHisto.back()->SetLineWidth( 2 );
      if( i != 0 ) fEvndispReconstructionParameterHisto.back()->SetLineStyle( 2 );
      fEvndispReconstructionParameterHisto.back()->SetLineColor( i+1 );


      sprintf( hname, "hInt_%s", fEvndispReconstructionParameterName[i].c_str() );
      fEvndispReconstructionParameterHistoInt.push_back( new TH1D( hname, "", iNEnergyBins, iEnergy_TeV_log_min, iEnergy_TeV_log_max ) );
      fEvndispReconstructionParameterHistoInt.back()->SetXTitle( "log_{10} energy [TeV]" );
      fEvndispReconstructionParameterHistoInt.back()->SetStats( 0 );
      fEvndispReconstructionParameterHistoInt.back()->SetLineWidth( 2 );
      fEvndispReconstructionParameterHistoInt.back()->SetLineColor( i+1 );
   }
   

   return true;
}

void VPlotEvndispReconstructionParameter::reset()
{
   for( unsigned int i = 0; i < fEvndispReconstructionParameterHisto.size(); i++ )
   {
      fEvndispReconstructionParameterHisto[i]->Reset();
      fEvndispReconstructionParameterHistoInt[i]->Reset();
   }
}

void VPlotEvndispReconstructionParameter::plot( unsigned int iMethod, unsigned int iTelescope )
{
// reset all histograms
   reset();

// fill all histograms
   fill( iMethod, iTelescope );

// plot everything
   char hname[200];
   char htitle[200];
   sprintf( hname, "c_PERP_%d_%d", iMethod, iTelescope );
   sprintf( htitle, "eventdisplay reconstruction parameters (method %d, telescope %d)", iMethod, iTelescope );
   fPlotCanvas = new TCanvas( hname, htitle, 10, 10, 600, 600 );
   fPlotCanvas->Draw();

   unsigned int z = 0;
   for(  unsigned int i = 0; i < fEvndispReconstructionParameterHisto.size(); i++ )
   {
      if( fEvndispReconstructionParameterHisto[i] && fEvndispReconstructionParameterHisto[i]->GetEntries() > 0 )
      {
	 if( z == 0 )
	 {
	    fEvndispReconstructionParameterHisto[i]->DrawCopy();
	    z++;
         }
	 else
	 {
	   fEvndispReconstructionParameterHisto[i]->DrawCopy( "same" );
         }
	 cout << fEvndispReconstructionParameterName[i];
	 cout << "(color " << fEvndispReconstructionParameterHisto[i]->GetLineColor() << ")" << endl;
	 cout << "\t entries: " << fEvndispReconstructionParameterHisto[i]->GetEntries() << endl;
      }
   }
   for( unsigned int i = 0; i < fEvndispReconstructionParameterHistoInt.size(); i++ )
   {
     if( z > 0 )
     {
        fEvndispReconstructionParameterHistoInt[i]->DrawCopy( "same" );
     }
   }
}

bool VPlotEvndispReconstructionParameter::fill( unsigned int iMethod, unsigned int iTelescope )
{
   cout << "filling histograms for method " << iMethod << " and telescope " << iTelescope+1 << endl;
   if( !fDataFile )
   {
      cout << "VPlotEvndispReconstructionParameter::fill() error: no file found" << endl;
      return false;
   }
// check method
   if( fEvndispReconstructionParameter && iMethod >= fEvndispReconstructionParameter->fNMethods )
   {
      cout << "VPlotEvndispReconstructionParameter::fill() error: method out of range " << endl;
      cout << iMethod << "\t" << fEvndispReconstructionParameter->fNMethods << endl;
      return false;
   }

// check telescope type
    int iTelType = -1;
    if( iTelescope < fDetectorGeometry->getTelType().size() )
    {
       iTelType = fEvndispReconstructionParameter->getTelescopeType_counter( fDetectorGeometry->getTelType()[iTelescope] );
    }
    if( iTelType < 0 )
    {
       cout << "VPlotEvndispReconstructionParameter::fill() error: telescope number not found" << endl;
       return false;
    }
    cout << "telescope type " << iTelType << endl;

// shower parameter
   TChain *i_showerpars = new TChain( "showerpars" );
   i_showerpars->Add( fDataFileName.c_str() );
   fDataShowerPars = new Cshowerpars( i_showerpars );

// telescope (image parameters)
   char hname[200];
   sprintf( hname, "%s/Tel_%d/tpars", fDataFileName.c_str(), iTelescope+1 );
   TChain *i_tpars = new TChain( "tpars" );
   i_tpars->Add( hname );
   fDataTpars = new CtparsShort( i_tpars, true, 8, true );

// loop over all entries and fill the histograms
   cout << "total number of entries : " << fDataShowerPars->fChain->GetEntries() << endl;

   for( int i = 0; i < fDataShowerPars->fChain->GetEntries(); i++ )
   {
      fDataShowerPars->GetEntry( i );
      fDataTpars->GetEntry ( i );

// no cuts
      if( fEvndispReconstructionParameterHisto[0] && fDataTpars->ntubes > 0 )
      {
         fEvndispReconstructionParameterHisto[0]->Fill( log10( fDataShowerPars->MCe0 ) );
      }
      else continue;

// size cut
      if( fEvndispReconstructionParameterHisto[1] && fEvndispReconstructionParameterHistoInt[1] )
      {
         if( fDataTpars->size < fEvndispReconstructionParameter->fSize_min[iMethod][iTelType] )  
	 {
	    fEvndispReconstructionParameterHisto[1]->Fill( log10( fDataShowerPars->MCe0 ) );
	    continue;
         }
	 else
	 {
	    fEvndispReconstructionParameterHistoInt[1]->Fill( log10( fDataShowerPars->MCe0 ) );
         }
      }
// ntubes cut
      if( fEvndispReconstructionParameterHisto[2] && fEvndispReconstructionParameterHistoInt[2] )
      {
         if( fDataTpars->ntubes <= fEvndispReconstructionParameter->fLocalNtubes_min[iMethod][iTelType] )  
	 {
	    fEvndispReconstructionParameterHisto[2]->Fill( log10( fDataShowerPars->MCe0 ) );
	    continue;
         }
	 else
	 {
	    fEvndispReconstructionParameterHistoInt[2]->Fill( log10( fDataShowerPars->MCe0 ) );
	 }   
      }
// number of saturated events
      if( fEvndispReconstructionParameterHisto[3] && fEvndispReconstructionParameterHistoInt[3] )
      {
         if( fDataTpars->nsat > fEvndispReconstructionParameter->fLocalNLowGain_max[iMethod][iTelType] )  
	 {
	    fEvndispReconstructionParameterHisto[3]->Fill( log10( fDataShowerPars->MCe0 ) );
	    continue;
         }
	 else
	 {
	    fEvndispReconstructionParameterHistoInt[3]->Fill( log10( fDataShowerPars->MCe0 ) );
         }
      }
// loss cut
      if( fEvndispReconstructionParameterHisto[4] && fEvndispReconstructionParameterHistoInt[4] )
      {
         if( fDataTpars->loss > fEvndispReconstructionParameter->fLoss_max[iMethod][iTelType] )  
	 {
	    fEvndispReconstructionParameterHisto[4]->Fill( log10( fDataShowerPars->MCe0 ) );
	    continue;
         }
	 else
	 {
	    fEvndispReconstructionParameterHistoInt[4]->Fill( log10( fDataShowerPars->MCe0 ) );
         }
      }
// fui cut
      if( fEvndispReconstructionParameterHisto[5] && fEvndispReconstructionParameterHistoInt[5] )
      {
         if( fDataTpars->fui < fEvndispReconstructionParameter->fFui_min[iMethod][iTelType] )  
	 {
	    fEvndispReconstructionParameterHisto[5]->Fill( log10( fDataShowerPars->MCe0 ) );
	    continue;
         }
	 else
	 {
	    fEvndispReconstructionParameterHistoInt[5]->Fill( log10( fDataShowerPars->MCe0 ) );
         }
      }
     
  
   } 

   return true;
}
