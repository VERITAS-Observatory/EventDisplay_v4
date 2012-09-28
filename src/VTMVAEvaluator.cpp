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
   fEmissionHeightChi2_log10 = 0.;
   fEChi2 = 0.;
   fEChi2_log10 = 0.;
   fdE = 0.;
   fEChi2S = 0.;
   fEChi2S_log10 = 0.;
   fdES = 0.;
   fSizeSecondMax_log10 = 0;
   fTheta2 = 0.;
   fCoreDist = 0.;
   fDummy = 0.;
   for( int i = 0; i < VDST_MAXTELESCOPES; i++ ) fImages_Ttype[i] = 0.;

   fTMVAMethodName_BOXCUTS = false;
   setTMVACutValue();
   setSignalEfficiency();
   setIgnoreTheta2Cut();
   setSpectralIndexForEnergyWeighting();
   setParticleNumberFile();
   setPlotEffiencyPlotsPerEnergy();
   setSensitivityOptimizationParameters();
   setTMVAMethod();
// default: don't expect that the theta2 cut is performed here   
   setTMVAThetaCutVariable( false );
   setTMVAErrorFraction();
   fTMVA_EvaluationResult = -99.;
}

/*

    get list of training variables

*/
vector< string > VTMVAEvaluator::getTrainingVariables( string iXMLFile, vector< bool >& iSpectator )
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
	 iVar.push_back( is_line.substr( is_line.find( "Expression=\"" ) + 12, is_line.find( "Label=" ) - 
					 is_line.find( "Expression=\"" ) - 14 ) );
          if( is_line.find( "SpecIndex" ) != string::npos ) iSpectator.push_back( true );
	  else                                              iSpectator.push_back( false );
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
      cout << "VTMVAEvaluator::initializeWeightFiles: min energy bin larger than maximum: ";
      cout << iWeightFileIndex_min << " > " << iWeightFileIndex_max << endl;
      fIsZombie = true;
      return false;
   }
   char hname[800];

// reset all vectors
   fBoxCutValue_theta2.clear();
   fBoxCutValue_min.clear();
   fBoxCutValue_max.clear();
   fBoxCutValue_Name.clear();
   fEnergyCut_Log10TeV_min.clear();
   fEnergyCut_Log10TeV_max.clear();
   vector< unsigned int > iFileNumber;

// number of energy bins
   unsigned int iNbin = iWeightFileIndex_max - iWeightFileIndex_min + 1;

   cout << "VTMVAEvaluator::initializeWeightFiles: reading energies from TMVA root files " << endl; 

//////////////////////////////////////////////////////////////////////////////////////
// read energy binning from root files and check that all neccessary object are in the file
   unsigned int iMinMissingBin = 0;
   for( unsigned int i = 0; i < iNbin; i++ )
   {
       ostringstream iFullFileName;
       iFullFileName << iWeightFileName << iWeightFileIndex_min+i << ".root";
       TFile iF( iFullFileName.str().c_str() );
       bool bGoodRun = true;
       VTMVARunDataEnergyCut *iEnergyData = 0;
       if( iF.IsZombie() ) bGoodRun = false;
       else
       {
          iEnergyData = (VTMVARunDataEnergyCut*)iF.Get( "fDataEnergyCut" );
	  if( !iEnergyData ) bGoodRun = false;
// signal efficiency
	  sprintf( hname, "Method_%s/%s_%d/MVA_%s_%d_effS", fTMVAMethodName.c_str(), fTMVAMethodName.c_str(),
							    fTMVAMethodCounter, fTMVAMethodName.c_str(), fTMVAMethodCounter );
	  if( !iF.Get( hname ) )
	  {
	     sprintf( hname, "Method_%s/%d/MVA_%d_effS", fTMVAMethodName.c_str(), fTMVAMethodCounter, fTMVAMethodCounter );
          }
          if( !iF.Get( hname ) ) bGoodRun = false;
       }
       if( !bGoodRun )
       {
// allow that first files are missing (this happens when there are no training events in the first energy bins)
          if( i == iMinMissingBin )
	  {
	      cout << "VTMVAEvaluator::initializeWeightFiles() warning: TMVA root file not found or incomplete file (" << i << ") " << endl;
	      cout << iFullFileName.str() << endl;
	      cout << "  assume this is a low-energy empty bin (bin number " << i << ";";
	      cout << " number of missing bins: " << iMinMissingBin+1 << ")" << endl;
	      iMinMissingBin++;
	      continue;
          }
	  else if( i == iWeightFileIndex_max )
	  {
	      cout << "VTMVAEvaluator::initializeWeightFiles() warning: TMVA root file not found " << iFullFileName.str() << endl;
	      cout << "  assume this is a high-energy empty bin (bin number " << i << ")" << endl;
	      iNbin--;
	      iWeightFileIndex_max--;
	      continue;
          }
          else
	  {
	     cout << "VTMVAEvaluator::initializeWeightFiles: warning: problem while initializing energies from TMVA root file " << iFullFileName.str() << endl;
	     cout << "(this might be not a problem if the sensitive energy range of the given array is relatively small)" << endl;
	     continue;
          }
       }
       iFileNumber.push_back( i );
       if( !iEnergyData )
       {
	  cout << "VTMVAEvaluator::initializeWeightFiles: warning: problem while reading energies from TMVA root file " << iFullFileName.str() << endl;
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
   if( iFileNumber.size() == 0 )
   {
      fIsZombie = true;
      return false;
   }

   cout << "VTMVAEvaluator: energy binning: " << endl;
   for( unsigned int i = 0; i < fEnergyCut_Log10TeV_min.size(); i++ )
   {
      cout << "\t" << i << "\t" << fEnergyCut_Log10TeV_min[i] << "\t" << fEnergyCut_Log10TeV_max[i];
      cout << "\t(energy reconstruction method " << fEnergyReconstructionMethod[i] << ")" << endl;
   }
   cout << endl;

//////////////////////////////////////////////////////////////////////////////////////
// create and initialize TMVA readers
// loop over all  energy bins: open one weight (XML) file per energy bin
   unsigned int z = 0;
   for( unsigned int b = 0; b < iFileNumber.size(); b++ )
   {
      unsigned int i = iFileNumber[b];

      fTMVAReader.push_back( new TMVA::Reader() );

// make sure that signal efficiency is set correctly
      if( z >= fSignalEfficiency.size() )
      {
          fSignalEfficiency.push_back( fSignalEfficiencyNoVec );
	  fBackgroundEfficiency.push_back( -99. );
      }
// make sure that MVA cut value is set correctly
      if( z >= fTMVACutValue.size() )
      {
	 if( fTMVACutValueNoVec < 0. && fSignalEfficiencyNoVec > 0. )
	 {
	     fTMVACutValue.push_back( getTMVACutValueFromSignalEfficiency( fSignalEfficiency.back(), i, iWeightFileName ) );
         }
	 else
	 {
	    fTMVACutValue.push_back( fTMVACutValueNoVec );
         }
      }

// weight file for this energy bin
      sprintf( hname, "MVA%d", i );
      ostringstream iFullFileName;
      iFullFileName << iWeightFileName << iWeightFileIndex_min+i;
      iFullFileName << "_" << fTMVAMethodName << "_" << fTMVAMethodCounter << ".weights.xml";
      if( fDebug ) cout << "reading TMVA XML weight file: " << iFullFileName << endl;

// get list of training variables
      vector< bool > iVariableIsASpectator;
      vector< string > iTrainingVariables = getTrainingVariables( iFullFileName.str(), iVariableIsASpectator );

// note that the following list of variables must be the same as during training
      for( unsigned int t = 0; t < iTrainingVariables.size(); t++ )
      {
         if( iTrainingVariables[t] == "MSCW" ) 
	 {
	     fTMVAReader.back()->AddVariable( "MSCW", &fMSCW );
         }
         else if( iTrainingVariables[t] == "MSCL" )
	 {
	    fTMVAReader.back()->AddVariable( "MSCL", &fMSCL );
         }
	 else if( iTrainingVariables[t] == "EmissionHeight" )
	 {
	    fTMVAReader.back()->AddVariable( "EmissionHeight", &fEmissionHeight );
         }
	 else if( iTrainingVariables[t] == "log10(EmissionHeightChi2)" )
	 {
	    fTMVAReader.back()->AddVariable( "log10(EmissionHeightChi2)", &fEmissionHeightChi2_log10 );
         }
	 else if( iTrainingVariables[t] == "NImages" && !iVariableIsASpectator[t] ) 
	 {
	    fTMVAReader.back()->AddVariable( "NImages", &fNImages );
         }
	 else if( iTrainingVariables[t] == "dE" ) 
	 {
	    fTMVAReader.back()->AddVariable( "dE", &fdE );
         }
	 else if( iTrainingVariables[t] == "EChi2" )
	 {
	    fTMVAReader.back()->AddVariable( "EChi2", &fEChi2 );
         }
	 else if( iTrainingVariables[t] == "log10(EChi2)" ) 
	 {
	    fTMVAReader.back()->AddVariable( "log10(EChi2)", &fEChi2_log10 );
         }
	 else if( iTrainingVariables[t] == "dES" ) 
	 {
	    fTMVAReader.back()->AddVariable( "dES", &fdES );
         }
	 else if( iTrainingVariables[t] == "log10(SizeSecondMax)" ) 
	 {
	    fTMVAReader.back()->AddVariable( "log10(SizeSecondMax)", &fSizeSecondMax_log10 );
         }
	 else if( iTrainingVariables[t] == "EChi2S" )
	 {
	    fTMVAReader.back()->AddVariable( "EChi2S", &fEChi2S );
         }
	 else if( iTrainingVariables[t] == "log10(EChi2S)" )   
	 {
	    fTMVAReader.back()->AddVariable( "log10(EChi2S)", &fEChi2S_log10 );
         }
	 else if( iTrainingVariables[t] == "(Xoff*Xoff+Yoff*Yoff)" ) 
	 {
	    fTMVAReader.back()->AddVariable( "(Xoff*Xoff+Yoff*Yoff)", &fTheta2 ); 
	    setTMVAThetaCutVariable( true );
         }
	 else if( iTrainingVariables[t] == "sqrt(Xcore*Xcore+Ycore*Ycore)" )
	 {
	    fTMVAReader.back()->AddVariable( "sqrt(Xcore*Xcore+Ycore*Ycore)", &fCoreDist );
         }
// Note: assume not more then 3 different telescope types
	 else if( iTrainingVariables[t] == "NImages_Ttype[0]" )
	 {
	    fTMVAReader.back()->AddVariable( "NImages_Ttype[0]", &fImages_Ttype[0] );
         }
	 else if( iTrainingVariables[t] == "NImages_Ttype[1]" )
	 {
	    fTMVAReader.back()->AddVariable( "NImages_Ttype[1]", &fImages_Ttype[1] );
         }
	 else if( iTrainingVariables[t] == "NImages_Ttype[2]" )
	 {
	    fTMVAReader.back()->AddVariable( "NImages_Ttype[2]", &fImages_Ttype[2] );
         }
	 else if( iVariableIsASpectator[t] )
	 {
	    fTMVAReader.back()->AddSpectator( iTrainingVariables[t].c_str(), &fDummy );
         }
      }
      cout << "Following variables have been found and are used for TMVA separation: " << endl;
      for( unsigned int t = 0; t < iTrainingVariables.size(); t++ ) 
      {
	 cout << "\t" << iTrainingVariables[t];
	 if( iVariableIsASpectator[t] ) cout << " (spectator)";
	 cout << endl;
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

      if( fOptmizationSourceStrengthCrabUnits > 0. && fParticleNumberFileName.size() > 0 )
      {
	  ostringstream iFullFileNameRoot;
          iFullFileNameRoot << iWeightFileName << iWeightFileIndex_min+i << ".root";

          if( !optimizeSensitivity( z, iFullFileNameRoot.str() ) )
	  {
	     cout << "VTMVAEvaluator::initializeWeightFiles: error while calculating optimized sensitivity" << endl;
	     return false;
          }
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
	    i_mcuts->GetCuts( fSignalEfficiency[z], i_cuts_min, i_cuts_max );
	    if( z < fSignalEfficiency.size() )
	    {
	       cout << "Box cuts for a signal efficiency at " << fSignalEfficiency[z];
            }
	    if( z < fBackgroundEfficiency.size() && fBackgroundEfficiency[z] > 0. )
	    {
	       cout << " (background efficiency: " << fBackgroundEfficiency[z] << ")";
            }
	    cout << ":" << endl;
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
      z++;
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
   cout << "VTMVAEvaluator: Initialized " << fTMVAReader.size() << " MVA readers " << endl;

   return true;
}

/* 

   get TMVA cut value for a given signal efficiency

*/
double VTMVAEvaluator::getTMVACutValueFromSignalEfficiency( double iSignalEfficiency, unsigned int iBin, string iWeightFileName )
{
    if( iWeightFileName.size() == 0 ) return -99.;

    ostringstream iFullFileName;
    iFullFileName << iWeightFileName << iBin  << ".root";
    TFile iTMVAFile( iFullFileName.str().c_str() );
    if( iTMVAFile.IsZombie() )
    {
       cout << "VTMVAEvaluator::getTMVACutValueFromSignalEfficiency() error reading TMVA root file: " << iFullFileName.str() << endl;
       return -9999.;
    }
    char hname[800];
    sprintf( hname, "Method_%s/%s_%d/MVA_%s_%d_effS", fTMVAMethodName.c_str(), fTMVAMethodName.c_str(),
                                                      fTMVAMethodCounter, fTMVAMethodName.c_str(), fTMVAMethodCounter );
// read signal efficiency histogram
    TH1F *effS = (TH1F*)iTMVAFile.Get( hname );
    if( !effS )
    {
        sprintf( hname, "Method_%s/%d/MVA_%d_effS", fTMVAMethodName.c_str(), fTMVAMethodCounter, fTMVAMethodCounter );
	effS = (TH1F*)iTMVAFile.Get( hname );
    }
    if( !effS )
    {
        cout << "VTMVAEvaluator::getTMVACutValueFromSignalEfficiency() error finding signal efficiency from " << iFullFileName.str() << endl;
	return -9999.;
    }
    cout << "VTMVAEvaluator::getTMVACutValueFromSignalEfficiency: evaluating " << iTMVAFile.GetName() << endl;
    cout << "\t method: " << hname << endl;

// get signal efficiency from histogram
    double iT = effS->GetBinCenter( effS->FindLastBinAbove( iSignalEfficiency ) );

    cout << "TMVA CUT VALUE FOR SIGNAL EFFICIENCY " << iSignalEfficiency << ": " << iT;
    cout << " (bin " << effS->FindLastBinAbove( iSignalEfficiency ) << ")" << endl;

    iTMVAFile.Close();

    return iT;
}

/*!

    evaluate this event and return a probability

*/

bool VTMVAEvaluator::evaluate()
{
   if( fDebug ) cout << "VTMVAEvaluator::evaluate (" << fData << ")" << endl;
// copy event data
   if( fData )
   {
       fNImages        = (float)fData->NImages;
       fMSCW           = fData->MSCW;
       fMSCL           = fData->MSCL;
       fMWR            = fData->MWR;
       fMLR            = fData->MLR;
       fEmissionHeight = fData->EmissionHeight;
       if( fData->EmissionHeightChi2 > 0. ) fEmissionHeightChi2_log10 = TMath::Log10( fData->EmissionHeightChi2 );
       else                                 fEmissionHeightChi2_log10 = -10.;   // !!! not clear what the best value is
       fEChi2          = fData->EChi2;
       if( fEChi2 > 0. ) fEChi2_log10 = TMath::Log10( fEChi2 );
       else              fEChi2_log10 = 0.;    // !!! not clear what the best value is
       fdE             = fData->dE;
       fEChi2S         = fData->EChi2S;
       if( fEChi2S > 0. ) fEChi2S_log10 = TMath::Log10( fEChi2S );
       else               fEChi2S_log10 = 0.;    // !!! not clear what the best value is
       fdES            = fData->dES;
       fSizeSecondMax_log10 = fData->SizeSecondMax;
       if( fSizeSecondMax_log10 > 0. ) fSizeSecondMax_log10 = TMath::Log10( fSizeSecondMax_log10 );
       else                      fSizeSecondMax_log10 = 0.;  // !!! not clear what the best value is
       if( fTMVAIgnoreTheta2Cut ) fTheta2 = 1.e-30;
       else                       fTheta2 = fData->Xoff*fData->Xoff + fData->Yoff*fData->Yoff;
       fCoreDist = sqrt( fData->Xcore*fData->Xcore+fData->Ycore*fData->Ycore );
       if( fData->NTtype > 0 ) fImages_Ttype[0] = (float)fData->NImages_Ttype[0];
       if( fData->NTtype > 1 ) fImages_Ttype[1] = (float)fData->NImages_Ttype[1];
       if( fData->NTtype > 2 ) fImages_Ttype[2] = (float)fData->NImages_Ttype[2];
   }
   else return false;

   unsigned int iEnergybin = getSpectralWeightedEnergyBin();
   fTMVA_EvaluationResult = -99.;

   if( iEnergybin < fTMVAReader.size() && fTMVAReader[iEnergybin] )
   {
      if( isBoxCuts() )
      {
	 if( iEnergybin < fSignalEfficiency.size() && fSignalEfficiency[iEnergybin] > 0. )
	 {
       	    if( fDebug )
	    {
	       cout << "VTMVAEvaluator::evaluate: energy bin " << iEnergybin;
	       cout << ", MVA Method Tag " << fTMVAMethodTag[iEnergybin];
	       cout << ", Signal Efficiency " << fSignalEfficiency[iEnergybin];
	       cout << endl;
	    }
	    fTMVA_EvaluationResult = (double)fTMVAReader[iEnergybin]->EvaluateMVA( fTMVAMethodTag[iEnergybin], fSignalEfficiency[iEnergybin] );
            return (bool)fTMVAReader[iEnergybin]->EvaluateMVA( fTMVAMethodTag[iEnergybin], fSignalEfficiency[iEnergybin] );
         }
	 else if( fSignalEfficiencyNoVec > 0. )
	 {
	    fTMVA_EvaluationResult = (double)fTMVAReader[iEnergybin]->EvaluateMVA( fTMVAMethodTag[iEnergybin], fSignalEfficiencyNoVec );
	    return (bool)fTMVAReader[iEnergybin]->EvaluateMVA( fTMVAMethodTag[iEnergybin], fSignalEfficiencyNoVec );
         }
      }
// all but box cuts (e.g. BDT, NN, etc)
      else
      {
	 if( iEnergybin < fTMVACutValue.size() && fTMVACutValue[iEnergybin] > -90. )
	 {
       	    if( fDebug )
	    {
	       cout << "VTMVAEvaluator::evaluate: energy bin " << iEnergybin;
	       cout << ", MVA Method Tag " << fTMVAMethodTag[iEnergybin];
	       cout << ", MVA Cut value " << fTMVACutValue[iEnergybin];
	       cout << endl;
	    }
	    fTMVA_EvaluationResult = fTMVAReader[iEnergybin]->EvaluateMVA( fTMVAMethodTag[iEnergybin] );
	    if( fTMVA_EvaluationResult < fTMVACutValue[iEnergybin] )
	    {
	       return false;
            }
	    else
	    {
	       return true;
            }
         }
	 else
	 {
	    fTMVA_EvaluationResult = fTMVAReader[iEnergybin]->EvaluateMVA( fTMVAMethodTag[iEnergybin] );
	    if( fTMVA_EvaluationResult < fTMVACutValueNoVec )
	    {
	       return false;
            }
	    else
	    {
	       return true;
            }
         }
      }
   }

   return false;
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
      if(      fEnergyReconstructionMethod[i] == 0 && fData->Erec  > 0. ) iErec = log10( fData->Erec );
      else if( fEnergyReconstructionMethod[i] == 1 && fData->ErecS > 0. ) iErec = log10( fData->ErecS );
      else if( fEnergyReconstructionMethod[i] == 2 && fData->ErecS > 0. ) iErec = log10( fData->ErecS );
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
   if( fDebug )
   {
      cout << "VTMVAEvaluator::getSpectralWeightedEnergyBin() ";
      cout << "energy bin: " << iEnergyBin;
      cout << ", log10 energy " << iErec;
      cout << endl;
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
// ignore when TMVA has no theta2 cut
    if( !getTMVAThetaCutVariable() ) return -1.;

    if( fEnergyCut_Log10TeV_min.size() != fBoxCutValue_theta2.size() )
    {
       cout << "VTMVAEvaluator::getBoxCut_Theta2 error: theta2 and energy vector dimensions inconsistent: ";
       cout << fEnergyCut_Log10TeV_min.size() << "\t" << fBoxCutValue_theta2.size() << endl;
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
// ignore when TMVA has no theta2 cut
    if( !getTMVAThetaCutVariable() ) return 0;

// check consistency
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

void VTMVAEvaluator::plotSignalAndBackgroundEfficiencies( bool iLogY, double iYmin )
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
   TGraphAsymmErrors *igCVa = 0;
   if( fTMVACutValue.size() > 0 )         igCVa = new TGraphAsymmErrors( 1 );

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
      if( igCVa && i < fTMVACutValue.size() )
      {
         igCVa->SetPoint(  i, iEnergy, fTMVACutValue[i] );
	 igCVa->SetPointEXlow( i, iEnergy - fEnergyCut_Log10TeV_min[i] );
	 igCVa->SetPointEXhigh( i, fEnergyCut_Log10TeV_max[i] - iEnergy );
      }

      if( fSignalEfficiency[i] <= 0. || fBackgroundEfficiency[i] <= 0. ) bNullEntry = true;

      if( fBackgroundEfficiency[i] < iMinBck ) iMinBck = fBackgroundEfficiency[i];
   }

// plot everything
   TCanvas *iCanvas = new TCanvas( "cSignalAndBackgroundEfficiencies", "signal and background efficiencies", 10, 10, 400, 400 );
   iCanvas->SetGridx( 0 );
   iCanvas->SetGridy( 0 );
   iCanvas->SetLeftMargin( 0.13 );
   if( !bNullEntry && iLogY ) iCanvas->SetLogy();
   else if( !iLogY )          iCanvas->SetLogy( 0 );
   iCanvas->Draw();

   TH1D *hnull = new TH1D( "hnullcSignalAndBackgroundEfficiencies", "", 100, fEnergyCut_Log10TeV_min[0], fEnergyCut_Log10TeV_max[fEnergyCut_Log10TeV_max.size()-1] );
   hnull->SetStats( 0 );
   hnull->SetXTitle( "energy [TeV]" );
   hnull->SetYTitle( "signal/background efficiency" );
   hnull->SetMinimum( iYmin );
   hnull->SetMaximum( 1. );
   plot_nullHistogram( iCanvas, hnull, false, false, 1.5, fEnergyCut_Log10TeV_min[0], fEnergyCut_Log10TeV_max[fEnergyCut_Log10TeV_max.size()-1] );

   setGraphPlottingStyle( igSignal, 1, 1., 20 );
   if( igBck ) setGraphPlottingStyle( igBck, 2, 1., 21 );

   igSignal->Draw( "pl" );
   if( igBck ) igBck->Draw( "pl" );

// plot MVA cut value
   if( igCVa )
   {
      TCanvas *iCVACanvas = new TCanvas( "iCVACanvas", "MVA cut value", 500, 10, 400, 400 );
      iCVACanvas->SetGridx( 0 );
      iCVACanvas->SetGridy( 0 );

      TH1D *hnull = new TH1D( "hnullcMVACuts", "", 100, fEnergyCut_Log10TeV_min[0], fEnergyCut_Log10TeV_max[fEnergyCut_Log10TeV_max.size()-1] );
      hnull->SetStats( 0 );
      hnull->SetXTitle( "energy [TeV]" );
      hnull->SetYTitle( "MVA cut variable" );
      double iMinMax = 1.e99;
      for( unsigned int i = 0; i < fTMVACutValue.size(); i++ ) if( fTMVACutValue[i] < iMinMax ) iMinMax = fTMVACutValue[i];
      if( iMinMax > 0. ) iMinMax *= 0.5;
      else               iMinMax *= 1.5;
      hnull->SetMinimum( iMinMax );
      for( unsigned int i = 0; i < fTMVACutValue.size(); i++ ) if( fTMVACutValue[i] > iMinMax ) iMinMax = fTMVACutValue[i];
      if( iMinMax > 0. ) iMinMax *= 1.5;
      else               iMinMax *= 0.5;
      hnull->SetMaximum( iMinMax );
      plot_nullHistogram( iCanvas, hnull, false, false, 1.3, fEnergyCut_Log10TeV_min[0], fEnergyCut_Log10TeV_max[fEnergyCut_Log10TeV_max.size()-1] );

      setGraphPlottingStyle( igCVa, 1, 1., 20 );

      igCVa->Draw( "pl" );
   }

}

void VTMVAEvaluator::setSignalEfficiency( double iE )
{
   for( unsigned int i = 0; i < fSignalEfficiency.size(); i++ )
   {
       fSignalEfficiency[i] = iE;
   }

   fSignalEfficiencyNoVec = iE;
}

void VTMVAEvaluator::setTMVACutValue( double iE )
{
   for( unsigned int i = 0; i < fTMVACutValue.size(); i++ )
   {
       fTMVACutValue[i] = iE;
   }

   fTMVACutValueNoVec = iE;
}

void VTMVAEvaluator::printSignalEfficiency()
{
   cout << "VTMVAEvaluator: energy dependent signal (background) efficiency: " << endl;
   for( unsigned int i = 0; i < fSignalEfficiency.size(); i++ )
   {
      if( i < fEnergyCut_Log10TeV_min.size() && i < fEnergyCut_Log10TeV_max.size() )
      {
         cout << "E [" << showpoint << setprecision( 3 ) << fEnergyCut_Log10TeV_min[i] << "," << fEnergyCut_Log10TeV_max[i] << "] TeV :\t ";
      }
      cout << fSignalEfficiency[i];
      if( i < fBackgroundEfficiency.size() && fBackgroundEfficiency[i] > 0. )
      {
         cout << "\t(" << fBackgroundEfficiency[i] << ")";
      }
      if( i < fTMVACutValue.size() )
      {
         cout << "\t MVACut: " << fTMVACutValue[i];
      }
      cout << endl;
   }
   cout << noshowpoint << endl;
}

/*

    calculate the optimal signal to noise ratio for a given particle number spectrum

*/
bool VTMVAEvaluator::optimizeSensitivity( unsigned int iEnergyBin, string iTMVARootFile )
{
   if( fParticleNumberFileName.size() == 0 ) return false;

//////////////////////////////////////////////////////
// read file with  NOn and Noff graphs
// (created from effective areas with quality cuts applied only,
//  use macro $ENVDISPSYS/macros/sensitivity.C (based on VSensitivity) for this)
   TFile iPN( fParticleNumberFileName.c_str() );
   if( iPN.IsZombie() )
   {
       cout << "VTVMAEvaluator::getOptimalSignalEfficiency error:" << endl;
       cout << " cannot read particle number file " << fParticleNumberFileName << endl;
       cout << " (energy bin " << iEnergyBin << ")" << endl;
       return false;
   }
   cout << "TVMAEvaluator::getOptimalSignalEfficiency reading: " << fParticleNumberFileName << endl;
// get the NOn and Noff graphs
   TGraph *i_on = (TGraph*)iPN.Get( "gNOn" );
   if( !i_on ) i_on = (TGraph*)iPN.Get( "gSignalRate" );
   TGraph *i_of = (TGraph*)iPN.Get( "gNOff" );
   if( !i_of ) i_of = (TGraph*)iPN.Get( "gBGRate" );
   if( !i_on || !i_of )
   {
       cout << "VTVMAEvaluator::getOptimalSignalEfficiency error:" << endl;
       cout << " cannot read graphs from particle number file " << endl;
       cout << i_on << "\t" << i_of << endl;
       return false;
   }
//////////////////////////////////////////////////////
// get mean energy of the considered bins
   double iMeanEnergy_TeV = -99.;
   if( iEnergyBin < fEnergyCut_Log10TeV_min.size() && iEnergyBin < fEnergyCut_Log10TeV_max.size() )
   {
      iMeanEnergy_TeV = 0.5*( TMath::Power( 10., fEnergyCut_Log10TeV_min[iEnergyBin] ) + TMath::Power( 10., fEnergyCut_Log10TeV_max[iEnergyBin] ));
      iMeanEnergy_TeV = TMath::Log10( iMeanEnergy_TeV ); // log10 energy
// make sure that energy is not lower or higher then minimum/maximum bins in graphs
      double x = 0.;
      double y = 0.;
      i_on->GetPoint( 0, x, y );
      if( iMeanEnergy_TeV < x ) iMeanEnergy_TeV = TMath::Log10( TMath::Power( 10., x ) * 1.2 );
      i_on->GetPoint( i_on->GetN()-1, x, y );
      if( iMeanEnergy_TeV > x ) iMeanEnergy_TeV = TMath::Log10( TMath::Power( 10., x ) * 0.8 );
   }
   else
   {
      cout << "VTVMAEvaluator::getOptimalSignalEfficiency error:" << endl;
      cout << " invalid energy range ";
      cout << iEnergyBin << "\t" << fEnergyCut_Log10TeV_min.size() << endl;
      return false;
   } 
//////////////////////////////////////////////////////
// get number of events at this energy 
   double Non = 0.;
   double Nof = 0.;
   double Ndif = 0.;

// Note that observation time is irrelevant for the determination of the optimal MVA cut value
   Non = i_on->Eval( iMeanEnergy_TeV ) * fOptimizationObservingTime_h * 60.;
   Nof = i_of->Eval( iMeanEnergy_TeV ) * fOptimizationObservingTime_h * 60.;
   if( Nof < 0. ) Nof = 0.;
   Ndif= Non - Nof;

   cout << "VTVMAEvaluator::getOptimalSignalEfficiency event numbers: ";
   cout << " non = " << Non;
   cout << " noff = " << Nof;
   cout << " ndif = " << Ndif << " (" << Ndif*fOptmizationSourceStrengthCrabUnits << ")";
   cout << " (energy bin " << iEnergyBin << "," << TMath::Power( 10., iMeanEnergy_TeV ) << " [TeV] )";
   cout << endl;
// apply source strength
   Ndif *= fOptmizationSourceStrengthCrabUnits;
       
//////////////////////////////////////////////////////
// get signal and background efficiency histograms

   TFile iTMVAFile( iTMVARootFile.c_str() );
   if( iTMVAFile.IsZombie() )
   {
      cout << "VTVMAEvaluator::getOptimalSignalEfficiency error:" << endl;
      cout << " cannot read TMVA file " << iTMVARootFile << endl;
      cout << " (energy bin " << iEnergyBin << ")" << endl;
      return false;
   }
   char hname[200];
   if( fTMVAMethodName == "BOXCUTS" )
   {
      sprintf( hname, "Method_Cuts/%s_%d/MVA_%s_%d_effS", fTMVAMethodName.c_str(), 
							  fTMVAMethodCounter, fTMVAMethodName.c_str(), fTMVAMethodCounter );
   }
   else
   {
      sprintf( hname, "Method_%s/%s_%d/MVA_%s_%d_effS", fTMVAMethodName.c_str(), fTMVAMethodName.c_str(), 
							fTMVAMethodCounter, fTMVAMethodName.c_str(), fTMVAMethodCounter );
   }
   TH1F *effS = (TH1F*)iTMVAFile.Get( hname );
   if( !effS )
   {
      sprintf( hname, "Method_%s/%d/MVA_%d_effS", fTMVAMethodName.c_str(), fTMVAMethodCounter, fTMVAMethodCounter );
      effS = (TH1F*)iTMVAFile.Get( hname );
   }
   if( fTMVAMethodName == "BOXCUTS" )
   {
      sprintf( hname, "Method_Cuts/%s_%d/MVA_%s_%d_effB", fTMVAMethodName.c_str(), 
							  fTMVAMethodCounter, fTMVAMethodName.c_str(), fTMVAMethodCounter );
   }
   else
   {
      sprintf( hname, "Method_%s/%s_%d/MVA_%s_%d_effB", fTMVAMethodName.c_str(), fTMVAMethodName.c_str(), 
							fTMVAMethodCounter, fTMVAMethodName.c_str(), fTMVAMethodCounter );
   }
   TH1F *effB = (TH1F*)iTMVAFile.Get( hname );
   if( !effB )
   {
      sprintf( hname, "Method_%s/%d/MVA_%d_effB", fTMVAMethodName.c_str(), fTMVAMethodCounter , fTMVAMethodCounter );
      effB = (TH1F*)iTMVAFile.Get( hname );
   }
   if( !effS || !effB )
   {
      cout << "VTVMAEvaluator::getOptimalSignalEfficiency error:" << endl;
      cout << " cannot find signal and/or background efficiency histogram(s)" << endl;
      cout << effS << "\t" << effB << endl;
      cout << hname << endl;
      return false;
   }
// evaluate errors on determination of box cut efficiency and remove bins with large errors
   if( fTMVAMethodName != "BOXCUTS" )
   {
      sprintf( hname, "Method_%s/%s_%d/MVA_%s_%d_B", fTMVAMethodName.c_str(), fTMVAMethodName.c_str(), 
							fTMVAMethodCounter, fTMVAMethodName.c_str(), fTMVAMethodCounter );
      TH1F *effB_counts = (TH1F*)iTMVAFile.Get( hname );
      if( effB_counts )
      {
	 double iMaxMVACutValue = -1.;
         for( int i = effB_counts->GetNbinsX()-1; i > 0; i-- )
	 {
	    if( effB_counts->GetBinContent( i ) > 0. )
	    {
	       if( effB_counts->GetBinError( i ) / effB_counts->GetBinContent( i ) > fTMVAErrorFraction_min )
	       {
		  iMaxMVACutValue = effB_counts->GetBinCenter( i );
               }
	       else break;
            }
	 }
	 if( iMaxMVACutValue > 0. )
	 {
	    cout << "VTVMAEvaluator::getOptimalSignalEfficiency() removing low significance bins from background efficiency curve (";
	    cout << fTMVAErrorFraction_min << ", " << iMaxMVACutValue << ")" << endl;
	    for( int i = 1; i <= effB->GetNbinsX(); i++ )
	    {
		if( effB->GetBinCenter( i ) > iMaxMVACutValue ) effB->SetBinContent( i, 0. );
	    }
         }
      }
   }
   
   cout << "TMVA Evaluator optimization: ";
   cout << fOptmizationSourceStrengthCrabUnits << " CU signal source; ";
   cout << "at least " << fOptmizationMinBackGroundEvents << " background events required";
   cout << " (alpha: " << fOptimizationBackgroundAlpha << ")" << endl;

//////////////////////////////////////////////////////
// now loop over signal and background efficiency levels
   double i_SignalEfficiency_AtMaximum = -99.;
   double i_TMVACutValue_AtMaximum = -99.;
   double i_BackgroundEfficiency_AtMaximum = -99.;
   double i_Signal_to_sqrtNoise = 0.;
   double i_Signal_to_sqrtNoise_atMaximum = 0.;

   TGraph *iGSignal_to_sqrtNoise = new TGraph( 1 );
   TGraph *iGSignalEvents        = new TGraph( 1 );
   TGraph *iGBackgroundEvents    = new TGraph( 1 );

   int z = 0;
   int z_SB = 0;
   for( int i = 1; i < effS->GetNbinsX(); i++ )
   {
      if( effB->GetBinContent( i ) > 0. && Nof > 0. )
      {
	 if( fOptimizationBackgroundAlpha > 0. )
	 {
	    i_Signal_to_sqrtNoise = VStatistics::calcSignificance( effS->GetBinContent( i ) * Ndif + effB->GetBinContent( i ) * Nof,
								   effB->GetBinContent( i ) * Nof / fOptimizationBackgroundAlpha,
								   fOptimizationBackgroundAlpha );
         }
	 else i_Signal_to_sqrtNoise = 0.;
	 if( fDebug )
	 {
	    cout << "___________________________________________________________" << endl;
	    cout << i << "\t" << Non << "\t" << effS->GetBinContent( i )  << "\t";
	    cout << Nof << "\t" << effB->GetBinContent( i ) << "\t";
	    cout << Ndif << endl;
	    cout << "\t" << effS->GetBinContent( i ) * Ndif * fOptmizationSourceStrengthCrabUnits;
	    cout << "\t" << effS->GetBinContent( i ) * Ndif * fOptmizationSourceStrengthCrabUnits + effB->GetBinContent( i ) * Nof;
	    cout << "\t" << effS->GetBinContent( i ) * Non + effB->GetBinContent( i ) * Nof << "\t" << effB->GetBinContent( i ) * Nof << endl;
         }
	 if( effS->GetBinCenter( i ) > 0. && effS->GetBinContent( i ) * Ndif > 0. )
	 {
	    iGSignalEvents->SetPoint( z_SB, effS->GetBinCenter( i ),  effS->GetBinContent( i ) * Ndif );
	    iGBackgroundEvents->SetPoint( z_SB, effS->GetBinCenter( i ), effB->GetBinContent( i ) * Nof );
	    z_SB++;
         }
// check that a minimum number of off events is available
         if( effB->GetBinContent( i ) * Nof < fOptmizationMinBackGroundEvents )
	 {
	    if( fDebug ) 
	    {
	       cout << "\t number of background events lower ";
	       cout << fOptmizationMinBackGroundEvents << ": setting signal/sqrt(noise) to 0; bin " << i << endl;
            }
	    i_Signal_to_sqrtNoise = 0.;
         }
	 if( iGSignal_to_sqrtNoise && i_Signal_to_sqrtNoise > 1.e-2 )
	 {
	    iGSignal_to_sqrtNoise->SetPoint( z, effS->GetBinCenter( i ), i_Signal_to_sqrtNoise );
	    if( fDebug ) cout << "\t SET " << z << "\t" << effS->GetBinCenter( i ) << "\t" << i_Signal_to_sqrtNoise << endl;
	    z++;
         }
	 if( fDebug )
	 {
	    cout << "\t z " << z << "\t" << i_Signal_to_sqrtNoise << endl;
	    cout << "___________________________________________________________" << endl;
         }
      }
   }
// fill a histogram from these values and smooth it
   TGraph *iGSignal_to_sqrtNoise_Smooth = 0;
   if( iGSignal_to_sqrtNoise )
   {
      TGraphSmooth *iGSmooth = new TGraphSmooth("s");
      iGSignal_to_sqrtNoise_Smooth = iGSmooth->SmoothKern( iGSignal_to_sqrtNoise, "normal", 0.05, 100 );
// get maximum values
      double x = 0.;
      double y = 0.;
      double i_ymax = -99.;
      double i_xmax = -99.;
      for( int i = 0; i < iGSignal_to_sqrtNoise_Smooth->GetN(); i++ )
      {
         iGSignal_to_sqrtNoise_Smooth->GetPoint( i, x, y );
	 if( y > i_ymax )
	 {
	    i_ymax = y;
	    i_xmax = x;
         }
      }
      i_SignalEfficiency_AtMaximum     = effS->GetBinContent( effS->FindBin( i_xmax ) );
      i_BackgroundEfficiency_AtMaximum = effB->GetBinContent( effB->FindBin( i_xmax ) );
      i_TMVACutValue_AtMaximum         = i_xmax;
      i_Signal_to_sqrtNoise_atMaximum  = i_ymax;
          
// make sure that signal efficency is > 0 (and 1 for the case that there is no maximum found)
      if( i_Signal_to_sqrtNoise_atMaximum < 1.e-3 )
      {
// first try: take previous bin:
         if( iEnergyBin > 0 )
	 {
	    i_SignalEfficiency_AtMaximum     = fSignalEfficiency[iEnergyBin-1];
	    i_TMVACutValue_AtMaximum         = fTMVACutValue[iEnergyBin-1];
	    i_BackgroundEfficiency_AtMaximum = fBackgroundEfficiency[iEnergyBin-1];
         }
	 else
	 {
	    i_SignalEfficiency_AtMaximum     = effS->GetBinContent( effS->GetMaximumBin() );
	    i_TMVACutValue_AtMaximum         = effS->GetBinCenter( effS->GetMaximumBin() );
	    i_BackgroundEfficiency_AtMaximum = effB->GetBinContent( effS->GetMaximumBin() );
         }
      } 
   }
   cout << "VTMVAEvaluator::optimizeSensitivity: signal efficiency at maximum is ";
   cout << i_SignalEfficiency_AtMaximum << " with a significance of " << i_Signal_to_sqrtNoise_atMaximum << endl;

   if( iEnergyBin < fSignalEfficiency.size() )     fSignalEfficiency[iEnergyBin]     = i_SignalEfficiency_AtMaximum;
   if( iEnergyBin < fBackgroundEfficiency.size() ) fBackgroundEfficiency[iEnergyBin] = i_BackgroundEfficiency_AtMaximum;
   if( iEnergyBin < fTMVACutValue.size() )         fTMVACutValue[iEnergyBin]         = i_TMVACutValue_AtMaximum;

   if( bPlotEfficiencyPlotsPerEnergy )
   {
      plotEfficiencyPlotsPerEnergy( iEnergyBin, iGSignal_to_sqrtNoise, iGSignal_to_sqrtNoise_Smooth,
                                    effS, effB,
				    fEnergyCut_Log10TeV_min[iEnergyBin], fEnergyCut_Log10TeV_max[iEnergyBin],
				    iGSignalEvents, iGBackgroundEvents );
   }

   return true;
}


void VTMVAEvaluator::plotEfficiencyPlotsPerEnergy( unsigned int iBin, TGraph* iGSignal_to_sqrtNoise, 
                                                   TGraph* iGSignal_to_sqrtNoise_Smooth, TH1F* hEffS, TH1F* hEffB, 
						   double iEnergy_Log10TeV_min, double iEnergy_Log10TeV_max,
						   TGraph* iGSignalEvents, TGraph* iGBackgroundEvents )
{
   char hname[800];
   char htitle[800];

// signal and noise plot
   if( hEffS )
   {
      sprintf( hname, "cEfficiencyPlotPerEnergy_%d", iBin );
      sprintf( htitle, "efficiency plots (bin %d, %.1f < log10(E) < %.1f)", iBin, iEnergy_Log10TeV_min, iEnergy_Log10TeV_max );
      TCanvas *iCanvas = new TCanvas( hname, htitle, 10, 10+iBin*30, 400, 400 );
      iCanvas->SetGridx( 0 );
      iCanvas->SetGridy( 0 );
      iCanvas->SetLeftMargin( 0.13 );
      iCanvas->Draw();

      hEffS->SetStats( 0 );
      hEffS->SetTitle( "" );
      hEffS->SetLineWidth( 3 );
      hEffS->GetYaxis()->SetTitleOffset( 1.5 );
      hEffS->SetXTitle( "cut value" );
      hEffS->SetYTitle( "signal/background efficiency" );
      hEffS->DrawCopy();

      if( hEffB )
      {
	 hEffB->SetStats( 0 );
	 hEffB->SetTitle( "" );
	 hEffB->SetLineColor( 2 );
	 hEffB->SetLineWidth( 3 );
         hEffB->DrawCopy( "same" );
      }

      if( iBin < fTMVACutValue.size() )
      {
         TLine *iL = new TLine( fTMVACutValue[iBin], hEffS->GetMinimum(), fTMVACutValue[iBin], hEffS->GetMaximum() );
	 iL->SetLineStyle( 2 );
	 iL->SetLineWidth( 3 );
	 iL->Draw();
      }
   }

// signal to noise
   if( iGSignal_to_sqrtNoise )
   {
      sprintf( hname, "cSignalToSqrtNoise_%d", iBin );
      sprintf( htitle, "signal / sqrt( noise ) (bin %d, %.1f < log10(E) < %.1f)", iBin, iEnergy_Log10TeV_min, iEnergy_Log10TeV_max );
      TCanvas *iCanvas = new TCanvas( hname, htitle, 425, 10+iBin*30, 400, 400 );
      iCanvas->SetLeftMargin( 0.13 );
      iCanvas->SetGridx( 0 );
      iCanvas->SetGridy( 0 );

      iGSignal_to_sqrtNoise->SetTitle( "" );
      setGraphPlottingStyle( iGSignal_to_sqrtNoise, 1, 1., 20 );

      iGSignal_to_sqrtNoise->Draw( "apl" );
      iGSignal_to_sqrtNoise->GetHistogram()->GetYaxis()->SetTitleOffset( 1.5 );
      iGSignal_to_sqrtNoise->GetHistogram()->SetXTitle( "cut value" );
      iGSignal_to_sqrtNoise->GetHistogram()->SetYTitle( "significance" );

      if( iGSignal_to_sqrtNoise_Smooth ) 
      {
         setGraphPlottingStyle( iGSignal_to_sqrtNoise_Smooth, 2, 2. );
         iGSignal_to_sqrtNoise_Smooth->Draw( "pl" );
      }

      if( iBin < fTMVACutValue.size() )
      {
         TLine *iL = new TLine( fTMVACutValue[iBin], iGSignal_to_sqrtNoise->GetHistogram()->GetMinimum(),
	                        fTMVACutValue[iBin], iGSignal_to_sqrtNoise->GetHistogram()->GetMaximum() );
	 iL->SetLineStyle( 2 );
	 iL->Draw();
      }
   }

// signal and background events numbers
   if( iGBackgroundEvents )
   {
      sprintf( hname, "cEventNumbers_%d", iBin );
      sprintf( htitle, "event numbers (bin %d, %.1f < log10(E) < %.1f)", iBin, iEnergy_Log10TeV_min, iEnergy_Log10TeV_max );
      TCanvas *iCanvas = new TCanvas( hname, htitle, 850, 10+iBin*30, 400, 400 );
      iCanvas->SetLeftMargin( 0.13 );
      iCanvas->SetGridx( 0 );
      iCanvas->SetGridy( 0 );

      iGBackgroundEvents->SetTitle( "" );
      setGraphPlottingStyle( iGBackgroundEvents, 1, 1., 20 );

      iGBackgroundEvents->Draw( "apl" );
      iGBackgroundEvents->GetHistogram()->GetYaxis()->SetTitleOffset( 1.5 );
      iGBackgroundEvents->GetHistogram()->SetXTitle( "cut value" );
      iGBackgroundEvents->GetHistogram()->SetYTitle( "number of events" );

      if( iGSignalEvents ) 
      {
         setGraphPlottingStyle( iGSignalEvents, 2, 2. );
         iGSignalEvents->Draw( "pl" );
      }

      if( iBin < fTMVACutValue.size() )
      {
         TLine *iL = new TLine( fTMVACutValue[iBin], iGBackgroundEvents->GetHistogram()->GetMinimum(),
	                        fTMVACutValue[iBin], iGBackgroundEvents->GetHistogram()->GetMaximum() );
	 iL->SetLineStyle( 2 );
	 iL->Draw();
         TLine *iLMinBack = new TLine( iGBackgroundEvents->GetHistogram()->GetXaxis()->GetXmin(), fOptmizationMinBackGroundEvents,
	                               iGBackgroundEvents->GetHistogram()->GetXaxis()->GetXmax(), fOptmizationMinBackGroundEvents );
	 iLMinBack->SetLineStyle( 2 );
	 iLMinBack->Draw();
      }
   }
}

void VTMVAEvaluator::setTMVAMethod( string iMethodName, unsigned int iMethodCounter )
{
   fTMVAMethodName = iMethodName; 
   if( fTMVAMethodName == "BOXCUTS" ) fTMVAMethodName_BOXCUTS = true;
   fTMVAMethodCounter = iMethodCounter;
}
   
