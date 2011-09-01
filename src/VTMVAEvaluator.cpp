/*! \class VTMVAEvaluator
    \brief use a TMVA weight file for energy dependent gamma/hadron separation

    Revision $Id: VTMVAEvaluator.cpp,v 1.1.2.4 2011/04/11 12:37:21 gmaier Exp $

    \author Gernot Maier
*/		

#include "VTMVAEvaluator.h"

VTMVAEvaluator::VTMVAEvaluator()
{
    fIsZombie = false;

    setDebug();

    fData = 0;

    reset();
}

void VTMVAEvaluator::reset()
{
   fNImages = 0.;
   fMSCW = 0.;
   fMSCL = 0.;
   fMWR = 0.;
   fMLR = 0.;
   fEmissionHeight = 0.;
   fEChi2 = 0.;
   fdE = 0.;
   fTheta2 = 0.;

   setSignalEfficiency();
   setIgnoreTheta2Cut();
   setSpectralIndexForEnergyWeighting();
   setParticleNumberFile();
   setPlotEffiencyPlotsPerEnergy();
   setSensitivityOptimizationParameters();
   setTMVAFileParameters();
}

/*

    get list of training variables

*/
vector< string > VTMVAEvaluator::getTrainingVariables( string iXMLFile )
{
   vector< string > iVar;

// open TMVA XML file 
// NOTE: extreme dependendence on the structure of the TMVA XML file
   ifstream is;
   is.open( iXMLFile.c_str(), ifstream::in);
   if( !is )
   {
      cout << "VTMVAEvaluator::getTrainingVariable error: cannot open TMVA weight file: " << iXMLFile << endl;
      return iVar;
   }
   string is_line;
   string iTemp;

   int nVar = 0;

   while( getline( is, is_line ) )
   {

// number of variables
      if( is_line.find( "NVar=\"" ) != string::npos  )
      {
	 nVar = atoi( is_line.substr( is_line.find( "NVar=\"" ) + 6, is_line.size() - is_line.find( "\">" ) - 1  ).c_str() );
	 if( fDebug ) cout << "\t reading TMVA XML file: number of variables is " << nVar << endl;
      }
// AGAIN, NOTE: extreme dependendence on the structure of the TMVA XML file
      if( is_line.find( "Expression=\"" ) != string::npos )
      {
         iVar.push_back( is_line.substr( is_line.find( "Expression=\"" ) + 12, is_line.find( "Label=" ) - is_line.find( "Expression=\"" ) - 14 ) );
	 if( fDebug ) cout << "\t reading TMVA XML file: new variable: " << iVar.back() << endl;
      }
   }
   is.close();

   return iVar;
}

/*
    initialize TMVA readers
*/
bool VTMVAEvaluator::initializeWeightFiles( string iWeightFileName, unsigned int iWeightFileIndex_min, unsigned int iWeightFileIndex_max )
{
// sanity checks
   if( iWeightFileName.size() == 0 )
   {
       cout << "VTMVAEvaluator::initializeWeightFiles error: no file name" << endl;
       fIsZombie = true;
       return false;
   }
   if( iWeightFileIndex_min > iWeightFileIndex_max )
   {
      cout << "VTMVAEvaluator::initializeWeightFiles: min energy bin larger than maximum: " << iWeightFileIndex_min << " > " << iWeightFileIndex_max << endl;
      fIsZombie = true;
      return false;
   }
   char hname[600];

// reset all vectors
   fBoxCutValue_theta2.clear();
   fBoxCutValue_min.clear();
   fBoxCutValue_max.clear();
   fBoxCutValue_Name.clear();
   fEnergyCut_Log10TeV_min.clear();
   fEnergyCut_Log10TeV_max.clear();

// number of energy bins
   unsigned int iNbin = iWeightFileIndex_max - iWeightFileIndex_min + 1;

   cout << "VTMVAEvaluator::initializeWeightFiles: reading energies from file " << endl;

//////////////////////////////////////////////////////////////////////////////////////
// read energy binning from root files
   for( unsigned int i = 0; i < iNbin; i++ )
   {
       ostringstream iFullFileName;
       iFullFileName << iWeightFileName << iWeightFileIndex_min+i << ".root";
       TFile iF( iFullFileName.str().c_str() );
       if( iF.IsZombie() )
       {
	  cout << "VTMVAEvaluator::initializeWeightFiles: error while initializing energies from TMVA root file " << iFullFileName.str() << endl;
	  fIsZombie = true;
	  return false;
       }
       VTMVARunDataEnergyCut *iEnergyData = (VTMVARunDataEnergyCut*)iF.Get( "fDataEnergyCut" );
       if( !iEnergyData )
       {
	  cout << "VTMVAEvaluator::initializeWeightFiles: error while reading energies from TMVA root file " << iFullFileName.str() << endl;
	  fIsZombie = true;
	  return false;
       }
       fEnergyCut_Log10TeV_min.push_back( iEnergyData->fEnergyCut_Log10TeV_min );
       fEnergyCut_Log10TeV_max.push_back( iEnergyData->fEnergyCut_Log10TeV_max );
       fEnergyReconstructionMethod.push_back( iEnergyData->fEnergyReconstructionMethod );
       sprintf( hname, "MVA%d", i );
       fTMVAMethodTag.push_back( hname );
       iF.Close();
   }

//////////////////////////////////////////////////////////////////////////////////////
// create and initialize TMVA readers
// loop over all  energy bins: open one weight file per energy bin
   for( unsigned int i = 0; i < iNbin; i++ )
   {
      fTMVAReader.push_back( new TMVA::Reader() );

// make sure that signal efficiency is set correctly
      if( i >= fSignalEfficiency.size() )
      {
          fSignalEfficiency.push_back( fSignalEfficiencyNoVec );
	  fBackgroundEfficiency.push_back( -99. );
      }

// weight file for this energy bin
      sprintf( hname, "MVA%d", i );
      ostringstream iFullFileName;
      iFullFileName << iWeightFileName << iWeightFileIndex_min+i << ".weights.xml";
      if( fDebug ) cout << "reading TMVA XML weight file: " << iFullFileName << endl;

// get list of training variables
      vector< string > iTrainingVariables = getTrainingVariables( iFullFileName.str() );

// note that the following list of variables must be the same as during training
      for( unsigned int t = 0; t < iTrainingVariables.size(); t++ )
      {
         if( iTrainingVariables[t] == "MSCW" )           fTMVAReader.back()->AddVariable( "MSCW", &fMSCW );
         if( iTrainingVariables[t] == "MSCL" )           fTMVAReader.back()->AddVariable( "MSCL", &fMSCL );
	 if( iTrainingVariables[t] == "EmissionHeight" ) fTMVAReader.back()->AddVariable( "EmissionHeight", &fEmissionHeight );
	 if( iTrainingVariables[t] == "NImages" )        fTMVAReader.back()->AddVariable( "NImages", &fNImages );
	 if( iTrainingVariables[t] == "dE" )             fTMVAReader.back()->AddVariable( "dE", &fdE );
	 if( iTrainingVariables[t] == "EChi2" )          fTMVAReader.back()->AddVariable( "EChi2", &fEChi2 );
	 if( iTrainingVariables[t] == "(Xoff*Xoff+Yoff*Yoff)" ) fTMVAReader.back()->AddVariable( "(Xoff*Xoff+Yoff*Yoff)", &fTheta2 ); 
      }
      if( fDebug )
      {
         cout << "Following variables have been found: " << endl;
	 for( unsigned int t = 0; t < iTrainingVariables.size(); t++ ) cout << "\t" << iTrainingVariables[t] << endl;
      }
	    

      if( !fTMVAReader.back()->BookMVA( hname, iFullFileName.str().c_str() ) )
      {
	  cout << "VTMVAEvaluator::initializeWeightFiles: error while initializing TMVA reader from weight file " << iFullFileName.str() << endl;
	  fIsZombie = true;
          return false;
      }
/////////////////////////////////////////////////////////
// get optimal signal efficiency (from maximum signal/noise ratio)
/////////////////////////////////////////////////////////

      if( fParticleNumberFileName.size() > 0 )
      {
	  ostringstream iFullFileNameRoot;
          iFullFileNameRoot << iWeightFileName << iWeightFileIndex_min+i << ".root";

// note (TODO): assume that this is the first method written to the TMVA root file
          optimizeSensitivity( i, iFullFileNameRoot.str() );
      }

/////////////////////////////////////////////////////////
// if necessary: read and print box cut variables:

      TMVA::MethodCuts *i_mcuts = (TMVA::MethodCuts*)fTMVAReader.back()->FindCutsMVA( hname );
      if( i_mcuts )
      {
// check if the current TMVA method is of type kCuts
// (TMVA::Types::kCuts)
         if( i_mcuts->GetMethodType() == TMVA::Types::kCuts )
	 {
	    vector< Double_t > i_cuts_min;
	    vector< Double_t > i_cuts_max;
	    vector< string >   i_cuts_name;
	    i_mcuts->GetCuts( fSignalEfficiency[i], i_cuts_min, i_cuts_max );
	    cout << "Printing box cuts for a signal efficiency of " << fSignalEfficiency[i];
	    if( i < fBackgroundEfficiency.size() && fBackgroundEfficiency[i] > 0. ) cout << "(background efficiency: " << fBackgroundEfficiency[i] << ")";
	    cout << " :" << endl;
	    for( unsigned int v = 0; v < i_cuts_min.size(); v++ )
	    {
		cout << "\t" << i_cuts_min[v] << " < " << i_mcuts->GetInputVar( v ) << " < " << i_cuts_max[v] << endl;
		i_cuts_name.push_back( (string)i_mcuts->GetInputVar( v ) );
	    }
// fill global cut vectors
            fBoxCutValue_min.push_back( i_cuts_min );
            fBoxCutValue_max.push_back( i_cuts_max );
            fBoxCutValue_Name.push_back( i_cuts_name );
// get theta2 cut values
	    for( unsigned int t = 0; t < iTrainingVariables.size(); t++ )
	    {
   	      if( iTrainingVariables[t] == "(Xoff*Xoff+Yoff*Yoff)" )
	      {
	         if( t < i_cuts_max.size() ) fBoxCutValue_theta2.push_back( i_cuts_max[t] );
              }
            }
         }
      }
   }


// sanity check
   if( fTMVAReader.size() != fEnergyCut_Log10TeV_min.size() ||
       fTMVAReader.size() != fEnergyCut_Log10TeV_max.size() ||
       fTMVAReader.size() != fEnergyReconstructionMethod.size() )
   {
      cout << "VTMVAEvaluator::initializeWeightFiles: error while initilizing TMVA reader (energy vector sizes) ";
      cout << fTMVAReader.size() << "\t" << fEnergyCut_Log10TeV_min.size() << "\t";
      cout << fEnergyCut_Log10TeV_max.size() << "\t" << fEnergyReconstructionMethod.size() << endl;
   }

// print some info to screen
   cout << "Initialized " << fTMVAReader.size() << " MVA readers " << endl;
   cout << "\t energy binning: " << endl;
   for( unsigned int i = 0; i < fTMVAReader.size(); i++ )
   {
      cout << "energy bin " << i << "\t" << fEnergyCut_Log10TeV_min[i] << "\t" << fEnergyCut_Log10TeV_max[i];
      cout << " (reconstruction method " << fEnergyReconstructionMethod[i] << ")" << endl;
   }

   return true;
}

/*!

    evaluate this event and return a probability

*/

double VTMVAEvaluator::evaluate( double iSignalEfficiency, double iProbabilityThreshold )
{
// copy event data
   if( fData )
   {
       fNImages        = (float)fData->NImages;
       fMSCW           = fData->MSCW;
       fMSCL           = fData->MSCL;
       fMWR            = fData->MWR;
       fMLR            = fData->MLR;
       fEmissionHeight = fData->EmissionHeight;
       fEChi2          = fData->EChi2;
       fdE             = fData->dE;
       if( fTMVAIgnoreTheta2Cut ) fTheta2 = 1.e-30;
       else                       fTheta2         = fData->Xoff*fData->Xoff + fData->Yoff*fData->Yoff;
   }
   else return -1.;

   unsigned int iEnergybin = getSpectralWeightedEnergyBin();

   if( iEnergybin < fTMVAReader.size() && fTMVAReader[iEnergybin] )
   {
      if( iSignalEfficiency > 0. && iProbabilityThreshold > 0. ) return fTMVAReader[iEnergybin]->GetProba( fTMVAMethodTag[iEnergybin], iSignalEfficiency );
      else                                                       return fTMVAReader[iEnergybin]->EvaluateMVA( fTMVAMethodTag[iEnergybin], iSignalEfficiency );
   }

   return 0.;
}

/*

   get spectral weighted energy bin for the current energy (read from fData)

*/
unsigned int VTMVAEvaluator::getSpectralWeightedEnergyBin()
{
   unsigned int iEnergyBin = 0;
   double       i_Diff_Energy = 1.30;           // difference between energy of current event and mean bin energy
   double       iMeanEnergy = 0.;

// find energy bin
   double iErec = 0.;
   for( unsigned int i = 0; i < fEnergyCut_Log10TeV_min.size(); i++ )
   {
// choose energy reconstruction method
      if(      fEnergyReconstructionMethod[i] == 0 && fData->Erec )  iErec = log10( fData->Erec );
      else if( fEnergyReconstructionMethod[i] == 0 && fData->ErecS ) iErec = log10( fData->ErecS );
      else iErec = -1.e99;

// mean energy of this energy bin (possibly spectral weighted)
      iMeanEnergy = VMathsandFunctions::getSpectralWeightedMeanEnergy( fEnergyCut_Log10TeV_min[i], fEnergyCut_Log10TeV_max[i],
                                                                       fSpectralIndexForEnergyWeighting );

// check which energy bin is closest
      if( TMath::Abs( iMeanEnergy - iErec ) < i_Diff_Energy ) 
      {
         i_Diff_Energy = TMath::Abs( iMeanEnergy - iErec );
	 iEnergyBin = i;
      }
   }

   return iEnergyBin;
}

bool VTMVAEvaluator::initializeDataStrutures( CData* iC )
{
   fData = iC;

   if( !fData )
   {
      fIsZombie = true;
      return false;
   }

   return true;
}

/*

   get energy dependent theta2 cut

*/
double VTMVAEvaluator::getBoxCut_Theta2( double iEnergy_log10TeV )
{
    if( fEnergyCut_Log10TeV_min.size() != fBoxCutValue_theta2.size() )
    {
       cout << "VTMVAEvaluator::getBoxCut_Theta2 error: theta2 and energy vector dimensions inconsistent: " << fEnergyCut_Log10TeV_min.size() << "\t" << fBoxCutValue_theta2.size() << endl;
       exit( -1 );
    }

// for very small energies: return smallest value
    if( fEnergyCut_Log10TeV_min.size() > 0 && iEnergy_log10TeV < fEnergyCut_Log10TeV_min[0] )
    {
       return fBoxCutValue_theta2[0];
    }

// for very high energies: return largest value
    if( fEnergyCut_Log10TeV_min.size() > 0 && iEnergy_log10TeV > fEnergyCut_Log10TeV_max[fEnergyCut_Log10TeV_max.size()-1] )
    {
       return fBoxCutValue_theta2[fBoxCutValue_theta2.size()-1];
    }

// find the theta2 cut for the corresponding energy
    for( unsigned int i = 0; i < fEnergyCut_Log10TeV_min.size(); i++ )
    {
       if( iEnergy_log10TeV > fEnergyCut_Log10TeV_min[i] && iEnergy_log10TeV < fEnergyCut_Log10TeV_max[i] ) return fBoxCutValue_theta2[i];
    }

    return 0.;
}

/*
   return a graph with all the box theta2 cuts

   (is a memory leak...)

*/
TGraph* VTMVAEvaluator::getBoxCut_Theta2_Graph()
{
   if( fEnergyCut_Log10TeV_min.size() != fBoxCutValue_theta2.size() )
   {
       cout << "VTMVAEvaluator::getBoxCut_Theta2_Graph: theta2 and energy vector dimensions inconsistent: ";
       cout << fEnergyCut_Log10TeV_min.size() << "\t" << fBoxCutValue_theta2.size() << endl;
       return 0;
   }

// fill the graph - energy is at the spectral weighted energy
   double iMeanEnergy = 0.;

   TGraph *g = new TGraph( 1 );
   for( unsigned int i = 0; i < fEnergyCut_Log10TeV_min.size(); i++ )
   {
      
      iMeanEnergy = VMathsandFunctions::getSpectralWeightedMeanEnergy( fEnergyCut_Log10TeV_min[i], fEnergyCut_Log10TeV_max[i],
                                                                       fSpectralIndexForEnergyWeighting );

      g->SetPoint( i, iMeanEnergy, fBoxCutValue_theta2[i] );
   }

   return g;
}

/*
   return a graph with all the box theta(!no ^2!) cuts

   (is a memory leak...)

*/
TGraph* VTMVAEvaluator::getBoxCut_Theta_Graph()
{
   TGraph *g = getBoxCut_Theta2_Graph();
   if( !g ) return 0;

   double x = 0.;
   double y = 0.;
   for( int i = 0; i < g->GetN(); i++ )
   {
      g->GetPoint( i, x, y );
      if( y > 0. ) y = sqrt( y );
      g->SetPoint( i, x, y );
   }
   return g;
}

void VTMVAEvaluator::plotBoxCuts()
{
    if( fBoxCutValue_Name.size() != fBoxCutValue_min.size() || fBoxCutValue_Name.size() != fBoxCutValue_max.size() )
    {
       cout << "VTMVAEvaluator::plotBoxCuts() error: inconsistent vector lengths" << endl;
       cout << "\t" << fBoxCutValue_Name.size() << "\t" << fBoxCutValue_min.size() << "\t" << fBoxCutValue_max.size() << endl;
       exit( -1 );
    }
    if( fBoxCutValue_Name.size() == 0 ) return;

    char hname[800];
    char htitle[800];

    TCanvas *c = 0;
    TGraphAsymmErrors *gN = 0;
    TGraphAsymmErrors *gX = 0;
    unsigned int z = 0;
    unsigned int pN = 0;
    unsigned int pX = 0;
    double iMeanEnergy = 0.;

// loop over all variables
    for( unsigned int i = 0; i < fBoxCutValue_Name[0].size(); i++ )
    {
// plot minimum cut value
	z = 0;
        gN = new TGraphAsymmErrors( 1 );
	for( unsigned int j = 0; j < fEnergyCut_Log10TeV_min.size(); j++ )
        {
   	    if( fBoxCutValue_min[j][i] > -1.e10 )
	    {
	       iMeanEnergy = VMathsandFunctions::getSpectralWeightedMeanEnergy( fEnergyCut_Log10TeV_min[j], fEnergyCut_Log10TeV_max[j],
	                                                                        fSpectralIndexForEnergyWeighting );
	       gN->SetPoint( z, iMeanEnergy, fBoxCutValue_min[j][i] );
	       gN->SetPointEXhigh( z, fEnergyCut_Log10TeV_max[j] - iMeanEnergy );
	       gN->SetPointEXlow( z, iMeanEnergy - fEnergyCut_Log10TeV_min[j] );
	       z++;
            }
        }
// plot only if there are points
        if( z > 0 && fBoxCutValue_Name[0].size() > 0 )
	{
	   sprintf( hname, "cTMVABoxCutMin_%d", i );
	   sprintf( htitle, "TMVA Box cut (min): %s", fBoxCutValue_Name[0][i].c_str() );
	   c = new TCanvas( hname, htitle, 100, 10+pN*200, 400, 400 );
	   c->SetGridx( 0 );
	   c->SetGridy( 0 );
	   c->SetLeftMargin( 0.15 );
	   c->Draw();
	   pN++;

	   setGraphPlottingStyle( gN, 1, 2., 20, 1. );

	   gN->Draw( "alp" );
	   if( gN->GetHistogram() )
	   {
	      gN->GetHistogram()->SetXTitle( "log_{10} energy [TeV]" );
	      sprintf( hname, "%s min cut value", fBoxCutValue_Name[0][i].c_str() );
	      gN->GetHistogram()->SetYTitle( hname );
	      gN->GetHistogram()->GetYaxis()->SetTitleOffset( 1.8 );
           }
        }
// plot maximum cut value
	z = 0;
        gX = new TGraphAsymmErrors( 1 );
	for( unsigned int j = 0; j < fEnergyCut_Log10TeV_min.size(); j++ )
        {
   	    if( fBoxCutValue_max[j][i] < 1.e10 )
	    {
	       iMeanEnergy = VMathsandFunctions::getSpectralWeightedMeanEnergy( fEnergyCut_Log10TeV_min[j], fEnergyCut_Log10TeV_max[j],
	                                                                        fSpectralIndexForEnergyWeighting );
	       gX->SetPoint( z, iMeanEnergy, fBoxCutValue_max[j][i] );
	       gX->SetPointEXhigh( z, fEnergyCut_Log10TeV_max[j] - iMeanEnergy );
	       gX->SetPointEXlow( z, iMeanEnergy - fEnergyCut_Log10TeV_min[j] );
	       z++;
            }
        }
// plot only if there are points
        if( z > 0 && fBoxCutValue_Name[0].size() > 0 )
	{
	   sprintf( hname, "cTMVABoxCutMax_%d", i );
	   sprintf( htitle, "TMVA Box cut (max): %s", fBoxCutValue_Name[0][i].c_str() );
	   c = new TCanvas( hname, htitle, 500, 10+pX*200, 400, 400 );
	   c->SetGridx( 0 );
	   c->SetGridy( 0 );
	   c->SetLeftMargin( 0.15 );
	   c->Draw();
	   pX++;

	   setGraphPlottingStyle( gX, 1, 2., 20, 1. );

	   gX->Draw( "alp" );
	   if( gX->GetHistogram() )
	   {
	      gX->GetHistogram()->SetXTitle( "log_{10} energy [TeV]" );
	      sprintf( hname, "%s max cut value", fBoxCutValue_Name[0][i].c_str() );
	      gX->GetHistogram()->SetYTitle( hname );
	      gX->GetHistogram()->GetYaxis()->SetTitleOffset( 1.8 );
           }
        }
     }
}

void VTMVAEvaluator::plotSignalAndBackgroundEfficiencies()
{
   if( fSignalEfficiency.size() == 0 )
   {
      cout << "TMVAEvaluator::plotSignalAndBackgroundEfficiencies error: signal efficiency vector with size 0" << endl;
      return;
   }
   if( fEnergyCut_Log10TeV_min.size() != fSignalEfficiency.size() )
   {
      cout << "TMVAEvaluator::plotSignalAndBackgroundEfficiencies error: signal efficiency of invalid size" << endl;
      cout << fEnergyCut_Log10TeV_min.size() << "\t" << fSignalEfficiency.size() << endl;
      return;
   }
   if( fEnergyCut_Log10TeV_max.size() != fSignalEfficiency.size() )
   {
      cout << "TMVAEvaluator::plotSignalAndBackgroundEfficiencies error: signal efficiency of invalid size" << endl;
      cout << fEnergyCut_Log10TeV_max.size() << "\t" << fSignalEfficiency.size() << endl;
      return;
   }

// fill graphs
   TGraphAsymmErrors *igSignal = new TGraphAsymmErrors( 1 );
   TGraphAsymmErrors *igBck = 0;
   if( fBackgroundEfficiency.size() > 0 ) igBck = new TGraphAsymmErrors( 1 );

   bool bNullEntry = false;
   double iMinBck = 1.;

   for( unsigned int i = 0; i < fSignalEfficiency.size(); i++ )
   {
      double iEnergy = log10( 0.5*( TMath::Power( 10., fEnergyCut_Log10TeV_min[i] ) + TMath::Power( 10., fEnergyCut_Log10TeV_max[i] ) ) );
      igSignal->SetPoint( i, iEnergy, fSignalEfficiency[i] );
      igSignal->SetPointEXlow( i, iEnergy - fEnergyCut_Log10TeV_min[i] );
      igSignal->SetPointEXhigh( i, fEnergyCut_Log10TeV_max[i] - iEnergy );
      if( igBck && i < fBackgroundEfficiency.size() )
      {
         igBck->SetPoint( i, iEnergy, fBackgroundEfficiency[i] );
	 igBck->SetPointEXlow( i, iEnergy - fEnergyCut_Log10TeV_min[i] );
	 igBck->SetPointEXhigh( i, fEnergyCut_Log10TeV_max[i] - iEnergy );
      }

      if( fSignalEfficiency[i] <= 0. || fBackgroundEfficiency[i] <= 0. ) bNullEntry = true;

      if( fBackgroundEfficiency[i] < iMinBck ) iMinBck = fBackgroundEfficiency[i];
   }

// plot everything
   TCanvas *iCanvas = new TCanvas( "cSignalAndBackgroundEfficiencies", "signal and background efficiencies", 10, 10, 400, 400 );
   iCanvas->SetGridx( 0 );
   iCanvas->SetGridy( 0 );
   if( !bNullEntry ) iCanvas->SetLogy();
   iCanvas->Draw();

   TH1D *hnull = new TH1D( "hnullcSignalAndBackgroundEfficiencies", "", 100, fEnergyCut_Log10TeV_min[0], fEnergyCut_Log10TeV_max[fEnergyCut_Log10TeV_max.size()-1] );
   hnull->SetStats( 0 );
   hnull->SetXTitle( "energy [TeV]" );
   hnull->SetYTitle( "signal/background efficiency" );
   hnull->SetMinimum( iMinBck*0.5 );
   hnull->SetMaximum( 1. );
   plot_nullHistogram( iCanvas, hnull, false, false, 1.3, fEnergyCut_Log10TeV_min[0], fEnergyCut_Log10TeV_max[fEnergyCut_Log10TeV_max.size()-1] );

   setGraphPlottingStyle( igSignal, 1, 1., 20 );
   if( igBck ) setGraphPlottingStyle( igBck, 2, 1., 21 );

   igSignal->Draw( "pl" );
   if( igBck ) igBck->Draw( "pl" );
}

void VTMVAEvaluator::setSignalEfficiency( double iE )
{
   for( unsigned int i = 0; i < fSignalEfficiency.size(); i++ )
   {
       fSignalEfficiency[i] = iE;
   }

   fSignalEfficiencyNoVec = iE;
}

/*

    calculate the optimal signal to noise ratio for a given particle number spectrum

*/
double VTMVAEvaluator::optimizeSensitivity( unsigned int iEnergyBin, string iTMVARootFile )
{
   if( fParticleNumberFileName.size() == 0 ) return -99.;

//////////////////////////////////////////////////////
// read file with  NOn and Noff graphs
// (created from effective areas with quality cuts applied only, use VSensitivity for this)
   TFile iPN( fParticleNumberFileName.c_str() );
   if( iPN.IsZombie() )
   {
       cout << "TVMAEvaluator::getOptimalSignalEfficiency error:" << endl;
       cout << " cannot read particle number file " << fParticleNumberFileName << endl;
       cout << " (energy bin " << iEnergyBin << ")" << endl;
       return -99.;
   }
   cout << "TVMAEvaluator::getOptimalSignalEfficiency reading: " << fParticleNumberFileName << endl;
// get the NOn and Noff graphs
   TGraph *i_on = (TGraph*)iPN.Get( "gNOn" );
   TGraph *i_of = (TGraph*)iPN.Get( "gNOff" );
   if( !i_on || !i_of )
   {
       cout << "TVMAEvaluator::getOptimalSignalEfficiency error:" << endl;
       cout << " cannot read graphs from particle number file " << endl;
       cout << i_on << "\t" << i_of << endl;
       return -99.;
   }
//////////////////////////////////////////////////////
// get mean energy of the considered bins
   double iMeanEnergy_TeV = -99.;
   if( iEnergyBin < fEnergyCut_Log10TeV_min.size() && iEnergyBin < fEnergyCut_Log10TeV_max.size() )
   {
      iMeanEnergy_TeV = 0.5*( TMath::Power( 10., fEnergyCut_Log10TeV_min[iEnergyBin] ) + TMath::Power( 10., fEnergyCut_Log10TeV_max[iEnergyBin] ));
      iMeanEnergy_TeV = TMath::Log10( iMeanEnergy_TeV ); // log10 energy
   }
   else
   {
      cout << "TVMAEvaluator::getOptimalSignalEfficiency error:" << endl;
      cout << " invalid energy range ";
      cout << iEnergyBin << "\t" << fEnergyCut_Log10TeV_min.size() << endl;
      return -99.;
   }
//////////////////////////////////////////////////////
// get number of events in this energy bin 
// (note: this should take binning into account!)
   double Non = i_on->Eval( iMeanEnergy_TeV ) * fOptmizationSourceStrengthCrabUnits;
   double Nof = i_of->Eval( iMeanEnergy_TeV );
   cout << "TVMAEvaluator::getOptimalSignalEfficiency event numbers: ";
   cout << " non = " << Non;
   cout << " noff = " << Nof;
   cout << endl;
       
//////////////////////////////////////////////////////
// get signal and background efficiency histograms

   TFile iTMVAFile( iTMVARootFile.c_str() );
   if( iTMVAFile.IsZombie() )
   {
      cout << "TVMAEvaluator::getOptimalSignalEfficiency error:" << endl;
      cout << " cannot read TMVA file " << iTMVARootFile << endl;
      cout << " (energy bin " << iEnergyBin << ")" << endl;
      return -99.;
   }
   char hname[200];
   sprintf( hname, "%s/%d/MVA_%d_effS", fTMVAMethodName.c_str(), fTMVAMethodCounter, fTMVAMethodCounter );
   TH1F *effS = (TH1F*)iTMVAFile.Get( hname );
   sprintf( hname, "%s/%d/MVA_%d_effB", fTMVAMethodName.c_str(), fTMVAMethodCounter , fTMVAMethodCounter );
   TH1F *effB = (TH1F*)iTMVAFile.Get( hname );
   if( !effS || !effB )
   {
      cout << "TVMAEvaluator::getOptimalSignalEfficiency error:" << endl;
      cout << " cannot find signal and/or background efficiency histogram(s)" << endl;
      cout << effS << "\t" << effB << endl;
      cout << hname << endl;
      return -99.;
   }

//////////////////////////////////////////////////////
// now loop over signal and background efficiency levels
   double i_SignalEfficiency_AtMaximum = -99.;
   double i_BackgroundEfficiency_AtMaximum = -99.;
   double i_Signal_to_sqrtNoise = 0.;
   double i_Signal_to_sqrtNoise_atMaximum = 0.;

   TGraph *iGSignal_to_sqrtNoise = new TGraph( 1 );

   int z = 0;
   for( int i = 1; i < effS->GetNbinsX(); i++ )
   {
      if( effB->GetBinContent( i ) > 0. && Nof > 0. )
      {
         i_Signal_to_sqrtNoise = VStatistics::calcSignificance( effS->GetBinContent( i ) * Non + effB->GetBinContent( i ) * Nof, effB->GetBinContent( i ) * Nof, 1. );
	 if( iGSignal_to_sqrtNoise )
	 {
	    iGSignal_to_sqrtNoise->SetPoint( z, effS->GetBinCenter( i ), i_Signal_to_sqrtNoise );
	    z++;
         }
      }
   }
// fill a histogram from these values and smooth it
   TH1F *iHSignal_to_sqrtNoise = 0;
   if( iGSignal_to_sqrtNoise )
   {
      sprintf( hname, "hSignalToSqrtNoise_%d", iEnergyBin );
      iHSignal_to_sqrtNoise = new TH1F( hname, "", effS->GetNbinsX(), effS->GetXaxis()->GetXmin(), effS->GetXaxis()->GetXmax() );
      for( int i = 1; i <= iHSignal_to_sqrtNoise->GetNbinsX(); i++ )
      {
         iHSignal_to_sqrtNoise->SetBinContent( i, iGSignal_to_sqrtNoise->Eval( iHSignal_to_sqrtNoise->GetBinCenter( i ) ) );
      }
      iHSignal_to_sqrtNoise->Smooth( 10 );
// get maximum values
      i_Signal_to_sqrtNoise_atMaximum = iHSignal_to_sqrtNoise->GetMaximum();
      i_SignalEfficiency_AtMaximum = effS->GetBinCenter( iHSignal_to_sqrtNoise->GetMaximumBin() );
      i_BackgroundEfficiency_AtMaximum = effB->GetBinContent( iHSignal_to_sqrtNoise->GetMaximumBin() );
   }
   cout << "===================================================" << endl;
   cout << i_SignalEfficiency_AtMaximum << " with a significance of " << i_Signal_to_sqrtNoise_atMaximum << endl;
   cout << "===================================================" << endl;

   if( iEnergyBin < fSignalEfficiency.size() )     fSignalEfficiency[iEnergyBin]     = i_SignalEfficiency_AtMaximum;
   if( iEnergyBin < fBackgroundEfficiency.size() ) fBackgroundEfficiency[iEnergyBin] = i_BackgroundEfficiency_AtMaximum;

   if( bPlotEfficiencyPlotsPerEnergy )
   {
      plotEfficiencyPlotsPerEnergy( iEnergyBin, iGSignal_to_sqrtNoise, iHSignal_to_sqrtNoise, effS, effB, fEnergyCut_Log10TeV_min[iEnergyBin], fEnergyCut_Log10TeV_max[iEnergyBin] );
   }

   return i_SignalEfficiency_AtMaximum;
}


void VTMVAEvaluator::plotEfficiencyPlotsPerEnergy( unsigned int iBin, TGraph* iGSignal_to_sqrtNoise, TH1F* iHSignal_to_sqrtNoise, TH1F* hEffS, TH1F* hEffB, double iEnergy_Log10TeV_min, double iEnergy_Log10TeV_max )
{
   char hname[800];
   char htitle[800];

// signal and noise plot
   if( hEffS )
   {
      sprintf( hname, "cEfficiencyPlotPerEnergy_%d", iBin );
      sprintf( htitle, "efficiency plots (bin %d, %.1f < log10(E) < %.1f)", iBin, iEnergy_Log10TeV_min, iEnergy_Log10TeV_max );
      TCanvas *iCanvas = new TCanvas( hname, htitle, 10, 10+iBin*30, 600, 600 );
      iCanvas->SetGridx( 0 );
      iCanvas->SetGridy( 0 );
      iCanvas->Draw();

      hEffS->SetStats( 0 );
      hEffS->SetTitle( "" );

      hEffS->DrawCopy();

      if( hEffB )
      {
	 hEffB->SetStats( 0 );
	 hEffB->SetTitle( "" );
	 hEffB->SetLineColor( 2 );
         hEffB->DrawCopy( "same" );
      }
   }

// signal to noise
   if( iGSignal_to_sqrtNoise )
   {
      sprintf( hname, "cSignalToSqrtNoise_%d", iBin );
      sprintf( htitle, "signal / sqrt( noise ) (bin %d, %.1f < log10(E) < %.1f)", iBin, iEnergy_Log10TeV_min, iEnergy_Log10TeV_max );
      TCanvas *iCanvas = new TCanvas( hname, htitle, 610, 10+iBin*30, 600, 600 );
      iCanvas->SetGridx( 0 );
      iCanvas->SetGridy( 0 );

      iGSignal_to_sqrtNoise->SetTitle( "" );
      setGraphPlottingStyle( iGSignal_to_sqrtNoise, 1., 1., 20 );

      iGSignal_to_sqrtNoise->Draw( "apl" );
      iGSignal_to_sqrtNoise->GetHistogram()->SetXTitle( "signal efficiency" );
      iGSignal_to_sqrtNoise->GetHistogram()->SetYTitle( "significance" );

      if( iHSignal_to_sqrtNoise ) 
      {
         setHistogramPlottingStyle( iHSignal_to_sqrtNoise, 2, 2. );
         iHSignal_to_sqrtNoise->DrawCopy("same");
      }
   }
}
