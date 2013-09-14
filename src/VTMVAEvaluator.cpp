/*! \class VTMVAEvaluator
    \brief use a TMVA weight file for energy dependent gamma/hadron separation

    \author Gernot Maier
*/		

#include "VTMVAEvaluator.h"

VTMVAEvaluator::VTMVAEvaluator()
{
    fIsZombie = false;

    setDebug();

    fData = 0;
    fTMVAEvaluatorResults = 0;

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
   setPlotEfficiencyPlotsPerEnergy();
   setSensitivityOptimizationParameters();
   setSensitivityOptimizationFixedSignalEfficiency();
   setTMVAOptimizationEnergyStepSize();
   setTMVAMethod();
// default: don't expect that the theta2 cut is performed here   
   setTMVAThetaCutVariable( false );
   setTMVAErrorFraction();
   setTMVAAngularContainmentRadiusMax();
   fTMVA_EvaluationResult = -99.;
   fTMVACutValueNoVec = -99.;
}

/*

    get list of training variables

*/
vector< string > VTMVAEvaluator::getTrainingVariables( string iXMLFile, vector< bool >& iSpectator )
{
   vector< string > iVar;

   cout << "reading list of variables from TMVA XML file: " << iXMLFile << endl;

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
//////////////////////////////
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

//////////////////////////////
// reset all vectors
   fBoxCutValue_theta2.clear();
   fBoxCutValue_min.clear();
   fBoxCutValue_max.clear();
   fBoxCutValue_Name.clear();
   fEnergyCut_Log10TeV_min.clear();
   fEnergyCut_Log10TeV_max.clear();
   fSpectralWeightedMeanEnergy_Log10TeV.clear();
   fSignalEfficiency.clear();
   fSourceStrengthAtOptimum_CU.clear();
   fAngularContainmentRadius.clear();
   fAngularContainmentFraction.clear();
   vector< unsigned int > iFileNumber;

//////////////////////////////
// number of energy bins
   unsigned int iNbin = iWeightFileIndex_max - iWeightFileIndex_min + 1;

   cout << "VTMVAEvaluator::initializeWeightFiles: reading energies from TMVA root files " << endl; 

/////////////////////////////////////////////////////////////////////////////////////////////
// read energy binning from root files and check that all neccessary objects are in the file
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
	     cout << "VTMVAEvaluator::initializeWeightFiles: warning: problem while initializing energies from TMVA root file ";
	     cout << iFullFileName.str() << endl;
	     cout << "(this might be not a problem if the sensitive energy range of the given array is relatively small)" << endl;
	     continue;
          }
       }
       if( !iEnergyData )
       {
	  cout << "VTMVAEvaluator::initializeWeightFiles: warning: problem while reading energies from TMVA root file ";
	  cout << iFullFileName.str() << endl;
	  iFileNumber.push_back( i );
	  fIsZombie = true;
	  return false;
       }
// initialize one value per energy bin
       double e = iEnergyData->fEnergyCut_Log10TeV_min;
       do
       {
	  iFileNumber.push_back( i );
	  fEnergyCut_Log10TeV_min.push_back( e );
	  fEnergyCut_Log10TeV_max.push_back( e + fTMVAOptimizationStepsize );
          fSpectralWeightedMeanEnergy_Log10TeV.push_back( 
	            VMathsandFunctions::getSpectralWeightedMeanEnergy( fEnergyCut_Log10TeV_min.back(), 
		                                                       fEnergyCut_Log10TeV_max.back(), 
								       fSpectralIndexForEnergyWeighting ) );
	  fEnergyReconstructionMethod.push_back( iEnergyData->fEnergyReconstructionMethod );
// get requested signal efficiency for this energy bin
	  fSignalEfficiency.push_back( getSignalEfficiency( iWeightFileIndex_min+i, 
							    iEnergyData->fEnergyCut_Log10TeV_min, 
							    iEnergyData->fEnergyCut_Log10TeV_max ) );
	  fBackgroundEfficiency.push_back( -99. );
	  fTMVACutValue.push_back( getTMVACutValue( iWeightFileIndex_min+i,
						    iEnergyData->fEnergyCut_Log10TeV_min, 
						    iEnergyData->fEnergyCut_Log10TeV_max ) );
	  fTMVAOptimumCutValueFound.push_back( false );
	  fSourceStrengthAtOptimum_CU.push_back( 0. );
	  fAngularContainmentRadius.push_back( -99. );
	  fAngularContainmentFraction.push_back( -99. );
	  sprintf( hname, "MVA%d", i );
	  fTMVAMethodTag.push_back( hname );
	  e += fTMVAOptimizationStepsize;
       } while( e < iEnergyData->fEnergyCut_Log10TeV_max );
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
      cout << "\t(energy reconstruction method " << fEnergyReconstructionMethod[i];
      cout << ", file number " << iFileNumber[i] << ")"<< endl;
   }
   cout << endl;

//////////////////////////////////////////////////////////////////////////////////////
// create and initialize TMVA readers
// loop over all  energy bins: open one weight (XML) file per energy bin
   unsigned int z = 0;
   for( unsigned int b = 0; b < iFileNumber.size(); b++ )
   {
      fTMVAReader.push_back( new TMVA::Reader() );

//////////////////////////////////////////
// set TMVA cut value 
// (optimization later)

// fixed signal efficiency
      if( fTMVACutValueNoVec < -1. && fSignalEfficiencyNoVec > 0. )
      {
	 fTMVACutValue[b] = -99.;
	 double iDummy = -99.;
         getValuesFromEfficiencyHistograms( fTMVACutValue[b], fSignalEfficiency[b], iDummy, iFileNumber[b], iWeightFileName );
      }
// fixed TMVA cut value
      else if( fTMVACutValueNoVec > -1. )
      {
	 double iDummy = -99.;
	 fSignalEfficiency[b] = -99.;
         getValuesFromEfficiencyHistograms( fTMVACutValue[b], fSignalEfficiency[b], iDummy, iFileNumber[b], iWeightFileName );
      }
      fTMVAOptimumCutValueFound[b] = false;

// weight file for this energy bin
      sprintf( hname, "MVA%d", iFileNumber[b] );
      ostringstream iFullFileName;
      iFullFileName << iWeightFileName << iWeightFileIndex_min+iFileNumber[b];
      iFullFileName << "_" << fTMVAMethodName << "_" << fTMVAMethodCounter << ".weights.xml";
      if( fDebug ) cout << "reading TMVA XML weight file: " << iFullFileName << endl;

// get list of training variables
      vector< bool > iVariableIsASpectator;
      vector< string > iTrainingVariables = getTrainingVariables( iFullFileName.str(), iVariableIsASpectator );

// note that the following list of variables must be the same as during training
      for( unsigned int t = 0; t < iTrainingVariables.size(); t++ )
      {
         if( iTrainingVariables[t] == "MSCW" && !iVariableIsASpectator[t] ) 
	 {
	     fTMVAReader.back()->AddVariable( "MSCW", &fMSCW );
         }
         else if( iTrainingVariables[t] == "MSCL" && !iVariableIsASpectator[t] )
	 {
	    fTMVAReader.back()->AddVariable( "MSCL", &fMSCL );
         }
	 else if( iTrainingVariables[t] == "EmissionHeight" && !iVariableIsASpectator[t] )
	 {
	    fTMVAReader.back()->AddVariable( "EmissionHeight", &fEmissionHeight );
         }
	 else if( iTrainingVariables[t] == "log10(EmissionHeightChi2)" && !iVariableIsASpectator[t] )
	 {
	    fTMVAReader.back()->AddVariable( "log10(EmissionHeightChi2)", &fEmissionHeightChi2_log10 );
         }
	 else if( iTrainingVariables[t] == "NImages" && !iVariableIsASpectator[t] ) 
	 {
	    fTMVAReader.back()->AddVariable( "NImages", &fNImages );
         }
	 else if( iTrainingVariables[t] == "dE" && !iVariableIsASpectator[t] ) 
	 {
	    fTMVAReader.back()->AddVariable( "dE", &fdE );
         }
	 else if( iTrainingVariables[t] == "EChi2" && !iVariableIsASpectator[t] )
	 {
	    fTMVAReader.back()->AddVariable( "EChi2", &fEChi2 );
         }
	 else if( iTrainingVariables[t] == "log10(EChi2)" && !iVariableIsASpectator[t] ) 
	 {
	    fTMVAReader.back()->AddVariable( "log10(EChi2)", &fEChi2_log10 );
         }
	 else if( iTrainingVariables[t] == "dES" && !iVariableIsASpectator[t] ) 
	 {
	    fTMVAReader.back()->AddVariable( "dES", &fdES );
         }
	 else if( iTrainingVariables[t] == "log10(SizeSecondMax)" && !iVariableIsASpectator[t] ) 
	 {
	    fTMVAReader.back()->AddVariable( "log10(SizeSecondMax)", &fSizeSecondMax_log10 );
         }
	 else if( iTrainingVariables[t] == "EChi2S" && !iVariableIsASpectator[t] )
	 {
	    fTMVAReader.back()->AddVariable( "EChi2S", &fEChi2S );
         }
	 else if( iTrainingVariables[t] == "log10(EChi2S)" && !iVariableIsASpectator[t] )   
	 {
	    fTMVAReader.back()->AddVariable( "log10(EChi2S)", &fEChi2S_log10 );
         }
	 else if( iTrainingVariables[t] == "(Xoff*Xoff+Yoff*Yoff)" && !iVariableIsASpectator[t] ) 
	 {
	    fTMVAReader.back()->AddVariable( "(Xoff*Xoff+Yoff*Yoff)", &fTheta2 ); 
	    setTMVAThetaCutVariable( true );
         }
	 else if( iTrainingVariables[t] == "sqrt(Xcore*Xcore+Ycore*Ycore)" && !iVariableIsASpectator[t] )
	 {
	    fTMVAReader.back()->AddVariable( "sqrt(Xcore*Xcore+Ycore*Ycore)", &fCoreDist );
         }
// Note: assume not more then 3 different telescope types
	 else if( iTrainingVariables[t] == "NImages_Ttype[0]" && !iVariableIsASpectator[t] )
	 {
	    fTMVAReader.back()->AddVariable( "NImages_Ttype[0]", &fImages_Ttype[0] );
         }
	 else if( iTrainingVariables[t] == "NImages_Ttype[1]" && !iVariableIsASpectator[t] )
	 {
	    fTMVAReader.back()->AddVariable( "NImages_Ttype[1]", &fImages_Ttype[1] );
         }
	 else if( iTrainingVariables[t] == "NImages_Ttype[2]" && !iVariableIsASpectator[t] )
	 {
	    fTMVAReader.back()->AddVariable( "NImages_Ttype[2]", &fImages_Ttype[2] );
         }
	 else if( iVariableIsASpectator[t] )
	 {
	    fTMVAReader.back()->AddSpectator( iTrainingVariables[t].c_str(), &fDummy );
         }
      }
      cout << "Following " << iTrainingVariables.size() << " variables have been found and are used for TMVA separation: " << endl;
      for( unsigned int t = 0; t < iTrainingVariables.size(); t++ ) 
      {
	 cout << "\t" << iTrainingVariables[t];
	 if( iVariableIsASpectator[t] ) cout << " (spectator)";
	 cout << endl;
      }
	    
      if( !fTMVAReader.back()->BookMVA( hname, iFullFileName.str().c_str() ) )
      {
	  cout << "VTMVAEvaluator::initializeWeightFiles: error while initializing TMVA reader from weight file ";
	  cout << iFullFileName.str() << endl;
	  fIsZombie = true;
          return false;
      }
/////////////////////////////////////////////////////////
// get optimal signal efficiency (from maximum signal/noise ratio)
/////////////////////////////////////////////////////////

      if( fParticleNumberFileName.size() > 0 )
      {
	  ostringstream iFullFileNameRoot;
          iFullFileNameRoot << iWeightFileName << iWeightFileIndex_min+iFileNumber[b] << ".root";

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

// sanity checks
   if( fTMVAReader.size() != fEnergyCut_Log10TeV_min.size() ||
       fTMVAReader.size() != fEnergyCut_Log10TeV_max.size() ||
       fTMVAReader.size() != fEnergyReconstructionMethod.size() )
   {
      cout << "VTMVAEvaluator::initializeWeightFiles: error while initializing TMVA reader (energy vector sizes) ";
      cout << fTMVAReader.size() << "\t" << fEnergyCut_Log10TeV_min.size() << "\t";
      cout << fEnergyCut_Log10TeV_max.size() << "\t" << fEnergyReconstructionMethod.size() << endl;
   }

// print some info to screen
   cout << "VTMVAEvaluator: Initialized " << fTMVAReader.size() << " MVA readers " << endl;

   fillTMVAEvaluatorResults();

   return true;
}

void VTMVAEvaluator::fillTMVAEvaluatorResults()
{
   if( !fTMVAEvaluatorResults )
   {
       fTMVAEvaluatorResults = new VTMVAEvaluatorResults;
       fTMVAEvaluatorResults->SetName( "TMVAEvaluatorResults" );
   }

   if( fTMVAEvaluatorResults )
   {
       fTMVAEvaluatorResults->fEnergyCut_Log10TeV_min = fEnergyCut_Log10TeV_min;
       fTMVAEvaluatorResults->fEnergyCut_Log10TeV_max = fEnergyCut_Log10TeV_max;
       fTMVAEvaluatorResults->fSpectralWeightedMeanEnergy_Log10TeV = fSpectralWeightedMeanEnergy_Log10TeV;
       fTMVAEvaluatorResults->fSignalEfficiency = fSignalEfficiency;
       fTMVAEvaluatorResults->fBackgroundEfficiency = fBackgroundEfficiency;
       fTMVAEvaluatorResults->fTMVAOptimumCutValueFound = fTMVAOptimumCutValueFound;
       fTMVAEvaluatorResults->fSourceStrengthAtOptimum_CU = fSourceStrengthAtOptimum_CU;
       fTMVAEvaluatorResults->fTMVACutValue = fTMVACutValue;
       fTMVAEvaluatorResults->fAngularContainmentRadius = fAngularContainmentRadius;
       fTMVAEvaluatorResults->fAngularContainmentFraction = fAngularContainmentFraction;
   }
}

TH1F *VTMVAEvaluator::getEfficiencyHistogram( string iName, TFile *iF )
{
   if( !iF ) return 0;

   char hname[800];
// the following naming might have to be adjusted for other methods than BDTs or Boxcuts
   if( fTMVAMethodName == "BOXCUTS" )
   {
      sprintf( hname, "Method_Cuts/%s_%d/MVA_%s_%d_%s", fTMVAMethodName.c_str(), 
							fTMVAMethodCounter, fTMVAMethodName.c_str(), fTMVAMethodCounter,
						        iName.c_str() );
   }
   else
   {
      sprintf( hname, "Method_%s/%s_%d/MVA_%s_%d_%s", fTMVAMethodName.c_str(), fTMVAMethodName.c_str(), 
						      fTMVAMethodCounter, fTMVAMethodName.c_str(), fTMVAMethodCounter,
						      iName.c_str() );
   }

// read signal efficiency histogram
    TH1F *eff = (TH1F*)iF->Get( hname );
    if( !eff )
    {
        sprintf( hname, "Method_%s/%d/MVA_%d_%s", fTMVAMethodName.c_str(), fTMVAMethodCounter, fTMVAMethodCounter,
	                                          iName.c_str() );
	eff = (TH1F*)iF->Get( hname );
    }
    if( !eff )
    {
        cout << "VTMVAEvaluator::getEfficiencyHistogram() error finding efficiency histogram " << iName;
	cout << " from " << iF->GetName() << endl;
	return 0;
    }
    return eff;
}

/* 

   get TMVA cut values 
   (e.g. signal efficiency for a given MVA cut or
         MVA cut for a given signal efficiency

*/

bool VTMVAEvaluator::getValuesFromEfficiencyHistograms( double& iMVACut, double& iSignalEfficiency, double& iBackgroundEfficiency,
                                                       unsigned int iBin, string iWeightFileName )
{
// make sure that default values are set
    if( iMVACut > -1. )                   iSignalEfficiency = iBackgroundEfficiency = -99.;
    else if( iSignalEfficiency > 0. )     iMVACut = iBackgroundEfficiency = -99.;
    else if( iBackgroundEfficiency > 0. ) iMVACut = iSignalEfficiency = -99.;
    
// check file name for consistency
    if( iWeightFileName.size() == 0 ) return false;

    ostringstream iFullFileName;
    iFullFileName << iWeightFileName << iBin  << ".root";
    TFile iTMVAFile( iFullFileName.str().c_str() );
    if( iTMVAFile.IsZombie() )
    {
       cout << "VTMVAEvaluator::getValuesFromEfficiencyHistograms() error reading TMVA root file: " << iFullFileName.str() << endl;
       return false;
    }
    TH1F *effS = getEfficiencyHistogram( "effS", &iTMVAFile );
    TH1F *effB = getEfficiencyHistogram( "effB", &iTMVAFile );
    if( !effS || !effB ) return false;

    cout << "VTMVAEvaluator::getValuesFromEfficiencyHistograms: evaluating " << iTMVAFile.GetName() << endl;

// get MVA cut for a given signal efficiency
    if( iSignalEfficiency > 0. )
    {
       iMVACut = effS->GetBinCenter( effS->FindLastBinAbove( iSignalEfficiency ) );
       iBackgroundEfficiency = effB->GetBinContent( effB->GetXaxis()->FindBin( iMVACut ) );

       cout << "TMVA CUT VALUE FOR SIGNAL EFFICIENCY " << iSignalEfficiency << ": " << iMVACut;
       cout << " (bin " << effS->FindLastBinAbove( iSignalEfficiency ) << ")" << endl;
    }
// get signal efficiency from histogram
    else if( iMVACut > -1. )
    {
       iSignalEfficiency = effS->GetBinContent( effS->GetXaxis()->FindBin( iMVACut ) );
       iBackgroundEfficiency = effB->GetBinContent( effB->GetXaxis()->FindBin( iMVACut ) );

       cout << "Signal efficiency for TMVA cut value " << iMVACut << ": " << iSignalEfficiency;
       cout << " (bin " << effS->GetXaxis()->FindBin( iMVACut ) << ")" << endl;
    }
// get MVA cut for a given background efficiency
    else if( iBackgroundEfficiency > 0. )
    {
       iMVACut = effB->GetBinCenter( effB->FindLastBinAbove( iBackgroundEfficiency ) );
       iSignalEfficiency = effS->GetBinContent( effS->GetXaxis()->FindBin( iMVACut ) );

       cout << "TMVA CUT VALUE FOR SIGNAL EFFICIENCY " << iBackgroundEfficiency << ": " << iMVACut;
       cout << " (bin " << effB->FindLastBinAbove( iBackgroundEfficiency ) << ")" << endl;
    } 

    iTMVAFile.Close();

    return true;
}

/*!

    evaluate this event and return passed/not passed

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
////////////////////////////
// box cuts
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
	 else
	 {
	     cout << "VTMVAEvaluator::evaluate: error: energy bin out of range: " << endl;
	     cout << "\t " << iEnergybin << "\t" << fSignalEfficiency.size() << endl;
	     if( iEnergybin < fSignalEfficiency.size() ) cout << "\t signal efficiency: " << fSignalEfficiency[iEnergybin] << endl;
	     cout << "exiting..." << endl;
	     exit( -1 );
         }
      }
////////////////////////////
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
   double       i_Diff_Energy = 1.e10;           // difference between energy of current event and mean bin energy
   double       iMeanEnergy = 0.;
   double       iMeanEnergy_min = 1.e10;

// find energy bin
   double iErec = 0.;
   for( unsigned int i = 0; i < fEnergyCut_Log10TeV_min.size(); i++ )
   {
// choose energy reconstruction method
      if(      fEnergyReconstructionMethod[i] == 0 && fData->Erec  > 0. ) iErec = log10( fData->Erec );
      else if( fEnergyReconstructionMethod[i] == 1 && fData->ErecS > 0. ) iErec = log10( fData->ErecS );
      else iErec = -1.e99;

// mean energy of this energy bin (possibly spectral weighted)
      iMeanEnergy = VMathsandFunctions::getMeanEnergyInBin( 2, fEnergyCut_Log10TeV_min[i], fEnergyCut_Log10TeV_max[i],
							    fSpectralIndexForEnergyWeighting );

// check which energy bin is closest
      if( TMath::Abs( iMeanEnergy - iErec ) < i_Diff_Energy ) 
      {
         i_Diff_Energy = TMath::Abs( iMeanEnergy - iErec );
	 iEnergyBin = i;
	 iMeanEnergy_min = iMeanEnergy;
      }
   }
   if( fDebug )
   {
      cout << "VTMVAEvaluator::getSpectralWeightedEnergyBin() ";
      cout << "energy bin: " << iEnergyBin;
      cout << " [" << fEnergyCut_Log10TeV_min[iEnergyBin] << ", " << fEnergyCut_Log10TeV_max[iEnergyBin];
      cout << "], mean energy " << iMeanEnergy_min;
      cout << ", log10 energy " << iErec << "\t" << i_Diff_Energy ;
      cout << "\t" << fSpectralIndexForEnergyWeighting;
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
double VTMVAEvaluator::getOptimalTheta2Cut( double iEnergy_log10TeV )
{
    if( fEnergyCut_Log10TeV_min.size() != fAngularContainmentRadius.size() )
    {
       cout << "VTMVAEvaluator::getOptimalTheta2Cu terror: theta2 and energy vector dimensions inconsistent: ";
       cout << fEnergyCut_Log10TeV_min.size() << "\t" << fAngularContainmentRadius.size() << endl;
       return -99.;
    }

// for very small energies: return smallest value
    if( fEnergyCut_Log10TeV_min.size() > 0 && iEnergy_log10TeV < fEnergyCut_Log10TeV_min[0] )
    {
       return fAngularContainmentRadius[0]*fAngularContainmentRadius[0];
    }

// for very high energies: return largest value
    if( fEnergyCut_Log10TeV_min.size() > 0 && iEnergy_log10TeV > fEnergyCut_Log10TeV_max[fEnergyCut_Log10TeV_max.size()-1] )
    {
       return fAngularContainmentRadius[fAngularContainmentRadius.size()-1]*fAngularContainmentRadius[fAngularContainmentRadius.size()-1];
    }

// find the theta2 cut for the corresponding energy
    for( unsigned int i = 0; i < fEnergyCut_Log10TeV_min.size(); i++ )
    {
       if( iEnergy_log10TeV > fEnergyCut_Log10TeV_min[i] && iEnergy_log10TeV < fEnergyCut_Log10TeV_max[i] ) return fAngularContainmentRadius[i]*fAngularContainmentRadius[i];
    }

    return 0.;
}

/*
   return a graph with all the theta2 cuts

   (is a memory leak...)

*/
TGraph* VTMVAEvaluator::getOptimalTheta2Cut_Graph()
{
// check consistency
   if( fEnergyCut_Log10TeV_min.size() != fAngularContainmentRadius.size() )
   {
       cout << "VTMVAEvaluator::getOptimalTheta2Cut_Graph: theta2 and energy vector dimensions inconsistent: ";
       cout << fEnergyCut_Log10TeV_min.size() << "\t" << fAngularContainmentRadius.size() << endl;
       return 0;
   }

// fill the graph - energy is at the spectral weighted energy
   double iMeanEnergy = 0.;

   TGraph *g = new TGraph( 1 );
   for( unsigned int i = 0; i < fEnergyCut_Log10TeV_min.size(); i++ )
   {
      
      iMeanEnergy = VMathsandFunctions::getSpectralWeightedMeanEnergy( fEnergyCut_Log10TeV_min[i], fEnergyCut_Log10TeV_max[i],
                                                                       fSpectralIndexForEnergyWeighting );

      g->SetPoint( i, iMeanEnergy, fAngularContainmentRadius[i]*fAngularContainmentRadius[i] );
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

void VTMVAEvaluator::plotSignalAndBackgroundEfficiencies( bool iLogY, double iYmin, double iMVA_min, double iMVA_max )
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
   TGraphAsymmErrors *igSignalOpt = new TGraphAsymmErrors( 1 );
   TGraphAsymmErrors *igBck = 0;
   TGraphAsymmErrors *igBckOpt = 0;
   if( fBackgroundEfficiency.size() > 0 )
   {
      igBck = new TGraphAsymmErrors( 1 );
      igBckOpt = new TGraphAsymmErrors( 1 );
   }
   TGraphAsymmErrors *igCVa = 0;
   TGraphAsymmErrors *igCVaOpt = 0;
   if( fTMVACutValue.size() > 0 )
   {
      igCVa    = new TGraphAsymmErrors( 1 );
      igCVaOpt = new TGraphAsymmErrors( 1 );
   }
   unsigned int z_opt = 0;
   unsigned int z_noOpt = 0;

   double iMinBck = 1.;

   for( unsigned int i = 0; i < fSignalEfficiency.size(); i++ )
   {
      double iEnergy = VMathsandFunctions::getSpectralWeightedMeanEnergy( fEnergyCut_Log10TeV_min[i], fEnergyCut_Log10TeV_max[i],
                                                                          fSpectralIndexForEnergyWeighting );
      if( i < fTMVAOptimumCutValueFound.size() && i < fTMVACutValue.size()
       && fSignalEfficiency[i] > 0. && fBackgroundEfficiency[i] > 0. )
      {
          if( fTMVAOptimumCutValueFound[i] )
	  {
	      igSignal->SetPoint( z_opt, iEnergy, fSignalEfficiency[i] );
	      igSignal->SetPointEXlow( z_opt, iEnergy - fEnergyCut_Log10TeV_min[i] );
	      igSignal->SetPointEXhigh( z_opt, fEnergyCut_Log10TeV_max[i] - iEnergy );
	      if( igBck && i < fBackgroundEfficiency.size() )
	      {
	         igBck->SetPoint( z_opt, iEnergy, fBackgroundEfficiency[i] );
	         igBck->SetPointEXlow( z_opt, iEnergy - fEnergyCut_Log10TeV_min[i] );
	         igBck->SetPointEXhigh( z_opt, fEnergyCut_Log10TeV_max[i] - iEnergy );
	      }
	      if( igCVa )
	      {
		  igCVa->SetPoint( z_opt, iEnergy, fTMVACutValue[i] );
		  igCVa->SetPointEXlow( z_opt, iEnergy - fEnergyCut_Log10TeV_min[i] );
		  igCVa->SetPointEXhigh( z_opt, fEnergyCut_Log10TeV_max[i] - iEnergy );
              }
	      z_opt++;
           }
	   else if( fTMVACutValue[i] > -90. )
	   {
	      igSignalOpt->SetPoint( z_noOpt, iEnergy, fSignalEfficiency[i] );
	      igSignalOpt->SetPointEXlow( z_noOpt, iEnergy - fEnergyCut_Log10TeV_min[i] );
	      igSignalOpt->SetPointEXhigh( z_noOpt, fEnergyCut_Log10TeV_max[i] - iEnergy );
	      if( igBckOpt && i < fBackgroundEfficiency.size() )
	      {
	         igBckOpt->SetPoint( z_noOpt, iEnergy, fBackgroundEfficiency[i] );
	         igBckOpt->SetPointEXlow( z_noOpt, iEnergy - fEnergyCut_Log10TeV_min[i] );
	         igBckOpt->SetPointEXhigh( z_noOpt, fEnergyCut_Log10TeV_max[i] - iEnergy );
	      }
	      if( igCVaOpt && i < fTMVACutValue.size() )
	      {
		 igCVaOpt->SetPoint( z_noOpt, iEnergy, fTMVACutValue[i] );
		 igCVaOpt->SetPointEXlow( z_noOpt, iEnergy - fEnergyCut_Log10TeV_min[i] );
		 igCVaOpt->SetPointEXhigh( z_noOpt, fEnergyCut_Log10TeV_max[i] - iEnergy );
              }
	      z_noOpt++;
	   }
      }
      if( fBackgroundEfficiency[i] < iMinBck ) iMinBck = fBackgroundEfficiency[i];
   }

// plot everything
   TCanvas *iCanvas = new TCanvas( "cSignalAndBackgroundEfficiencies", "signal and background efficiencies", 10, 10, 400, 400 );
   iCanvas->SetGridx( 0 );
   iCanvas->SetGridy( 0 );
   iCanvas->SetLeftMargin( 0.13 );
   if( iLogY ) iCanvas->SetLogy();
   else        iCanvas->SetLogy( 0 );
   iCanvas->Draw();

   TH1D *hnull = new TH1D( "hnullcSignalAndBackgroundEfficiencies", "", 100, fEnergyCut_Log10TeV_min[0], fEnergyCut_Log10TeV_max[fEnergyCut_Log10TeV_max.size()-1] );
   hnull->SetStats( 0 );
   hnull->SetXTitle( "energy [TeV]" );
   hnull->SetYTitle( "signal/background efficiency" );
   hnull->SetMinimum( iYmin );
   hnull->SetMaximum( 1. );
   plot_nullHistogram( iCanvas, hnull, false, false, 1.5, fEnergyCut_Log10TeV_min[0], fEnergyCut_Log10TeV_max[fEnergyCut_Log10TeV_max.size()-1] );

   setGraphPlottingStyle( igSignal, 1, 1., 20 );
   setGraphPlottingStyle( igSignalOpt, 1, 1., 24 );
   if( igBck ) setGraphPlottingStyle( igBck, 2, 1., 21 );
   if( igBckOpt > 0 ) setGraphPlottingStyle( igBckOpt, 2, 1., 25 );

   igSignal->Draw( "pl" );
   if( z_noOpt > 0 ) igSignalOpt->Draw( "pl" );
   if( igBck ) igBck->Draw( "pl" );
   if( igBckOpt && z_noOpt > 0 ) igBckOpt->Draw( "pl" );

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
      hnull->SetMinimum( iMVA_min );
      hnull->SetMaximum( iMVA_max );
      plot_nullHistogram( iCanvas, hnull, false, false, 1.3, fEnergyCut_Log10TeV_min[0], 
                          fEnergyCut_Log10TeV_max[fEnergyCut_Log10TeV_max.size()-1] );

      setGraphPlottingStyle( igCVa, 1, 1., 20 );
      igCVa->Draw( "p" );
      if( igCVaOpt && z_noOpt > 0 )
      {
	 setGraphPlottingStyle( igCVaOpt, 1, 1., 24 );
	 igCVaOpt->Draw( "p" );
      }
   }

}

void VTMVAEvaluator::setSignalEfficiency( double iSignalEfficiency )
{
   fSignalEfficiencyMap[9999] = iSignalEfficiency;

   fSignalEfficiencyNoVec = iSignalEfficiency;
}

void VTMVAEvaluator::setSignalEfficiency( map< unsigned int, double > iSignalEfficiencyMap )
{
   fSignalEfficiencyMap = iSignalEfficiencyMap;

// set to value >0: indicates that signal efficiency had been set from outsite
   fSignalEfficiencyNoVec = 1.;
}

void VTMVAEvaluator::setTMVACutValue( map< unsigned int, double > iMVA )
{
   fTMVACutValueMap = iMVA;

   fTMVACutValueNoVec = 1.;
}

void VTMVAEvaluator::setTMVACutValue( double iE )
{
   fTMVACutValueNoVec = iE;
}

void VTMVAEvaluator::printSourceStrength_CU()
{
   for( unsigned int i = 0; i < fSourceStrengthAtOptimum_CU.size(); i++ )
   {
      if( i < fEnergyCut_Log10TeV_min.size() && i < fEnergyCut_Log10TeV_max.size() )
      {
         cout << "E [" << showpoint << setprecision( 3 ) << fEnergyCut_Log10TeV_min[i] << "," << fEnergyCut_Log10TeV_max[i] << "] TeV";
	 cout << " (bin " << i << "):\t ";
      }
      cout << fSourceStrengthAtOptimum_CU[i] << " CU " << endl;
   }
}

void VTMVAEvaluator::printAngularContainmentRadius()
{
   if( fAngularContainmentRadius.size() == 0 ) return;

   cout << "VTMVAEvaluator: energy dependent optimal containment radius cut: " << endl;
   for( unsigned int i = 0; i < fAngularContainmentRadius.size(); i++ )
   {
      if( i < fEnergyCut_Log10TeV_min.size() && i < fEnergyCut_Log10TeV_max.size() )
      {
         cout << "E [" << showpoint << setprecision( 3 ) << fEnergyCut_Log10TeV_min[i] << "," << fEnergyCut_Log10TeV_max[i] << "] TeV";
	 cout << " (bin " << i << "):\t ";
      }
      cout << fAngularContainmentRadius[i] << " [deg], ";
      if( i < fAngularContainmentFraction.size() ) cout << fAngularContainmentFraction[i] << "%";
      cout << endl;
   }
   cout << noshowpoint << endl;
}
       
void VTMVAEvaluator::printSignalEfficiency()
{
   cout << "VTMVAEvaluator: energy dependent signal (background) efficiency: " << endl;
   for( unsigned int i = 0; i < fSignalEfficiency.size(); i++ )
   {
      if( i < fEnergyCut_Log10TeV_min.size() && i < fEnergyCut_Log10TeV_max.size() )
      {
         cout << "E [" << showpoint << setprecision( 3 ) << fEnergyCut_Log10TeV_min[i] << "," << fEnergyCut_Log10TeV_max[i] << "] TeV";
	 cout << " (bin " << i << "):\t ";
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
      if( i < fTMVAOptimumCutValueFound.size() && fParticleNumberFileName.size() > 0
       && i < fSourceStrengthAtOptimum_CU.size() )
      { 
         if( fTMVAOptimumCutValueFound[i] )
	 {
	    cout << " (optimum reached for " << fSourceStrengthAtOptimum_CU[i] << " CU)";
         }
	 else
	 {
	    cout << " (no optimum reached (" << fSourceStrengthAtOptimum_CU[i] << " CU)";
         }
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

// need particle number file for optimization 
// (contains signal and background rate vs energy)
   if( fParticleNumberFileName.size() == 0 ) return false;

   printSensitivityOptimizationParameters();

//////////////////////////////////////////////////////
// read file with  NOn and Noff graphs
// (created from effective areas with quality cuts applied only,
   TFile iPN( fParticleNumberFileName.c_str() );
   if( iPN.IsZombie() )
   {
       cout << "VTVMAEvaluator::getOptimalSignalEfficiency error:" << endl;
       cout << " cannot read particle number file " << fParticleNumberFileName << endl;
       cout << " (energy bin " << iEnergyBin << ")" << endl;
       return false;
   }
   cout << "TVMAEvaluator::getOptimalSignalEfficiency reading: " << fParticleNumberFileName << endl;
// get the NOn (signal + background) and NOff (background) graphs
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
   TH2D *iHAngContainment = (TH2D*)iPN.Get( "AngResCumulative" );
   if( iHAngContainment ) cout << "TVMAEvaluator::getOptimalSignalEfficiency found angular containment histogram" << endl;
   else                   cout << "TVMAEvaluator::getOptimalSignalEfficiency no angular containment histogram found" << endl;
//////////////////////////////////////////////////////
// get mean energy of the considered bins
   double iMeanEnergy_TeV = -99.;
   double iSpectralWeightedMeanEnergy_TeV = -99.;
   if( iEnergyBin < fEnergyCut_Log10TeV_min.size() && iEnergyBin < fEnergyCut_Log10TeV_max.size() )
   {
      iMeanEnergy_TeV = 0.5*( TMath::Power( 10., fEnergyCut_Log10TeV_min[iEnergyBin] )
                            + TMath::Power( 10., fEnergyCut_Log10TeV_max[iEnergyBin] ));
      iMeanEnergy_TeV = TMath::Log10( iMeanEnergy_TeV ); // log10 energy
// get spectral weighed mean
      iSpectralWeightedMeanEnergy_TeV = VMathsandFunctions::getMeanEnergyInBin( 2, fEnergyCut_Log10TeV_min[iEnergyBin],
                                                                                   fEnergyCut_Log10TeV_max[iEnergyBin],
										   fSpectralIndexForEnergyWeighting );
// make sure that energy is not lower or higher then minimum/maximum bins in the rate graphs
      double x = 0.;
      double y = 0.;
      for( int ii = 0; ii < i_on->GetN(); ii++ )
      {
	 i_on->GetPoint( ii, x, y );
	 if( y > 0. && ( x+i_on->GetErrorXhigh( ii ) <= fEnergyCut_Log10TeV_max[iEnergyBin]
	              || x <= fEnergyCut_Log10TeV_max[iEnergyBin] ) )
	 {
	    if( iSpectralWeightedMeanEnergy_TeV < x )
	    {
		if( x +i_on->GetErrorXhigh( ii ) <= fEnergyCut_Log10TeV_max[iEnergyBin] ) 
		{
		   iSpectralWeightedMeanEnergy_TeV = x + 0.97*i_on->GetErrorXhigh( ii );
                }
		else
		{
		   iSpectralWeightedMeanEnergy_TeV = x;
                }
	    }
	    break;
         }
      }
      i_on->GetPoint( i_on->GetN()-1, x, y );
      if( iSpectralWeightedMeanEnergy_TeV > x ) iSpectralWeightedMeanEnergy_TeV = TMath::Log10( TMath::Power( 10., x ) * 0.8 );
      fSpectralWeightedMeanEnergy_Log10TeV[iEnergyBin] = iSpectralWeightedMeanEnergy_TeV;
   }
   else
   {
      cout << "VTVMAEvaluator::getOptimalSignalEfficiency error:" << endl;
      cout << " invalid energy range ";
      cout << iEnergyBin << "\t" << fEnergyCut_Log10TeV_min.size() << endl;
      return false;
   } 
//////////////////////////////////////////////////////
// get number of events (after quality cuts) at this energy 
   double Non = 0.;
   double Nof = 0.;
   double Ndif = 0.;

   Non = i_on->Eval( fSpectralWeightedMeanEnergy_Log10TeV[iEnergyBin] ) * fOptimizationObservingTime_h * 60.;
   Nof = i_of->Eval( fSpectralWeightedMeanEnergy_Log10TeV[iEnergyBin] ) * fOptimizationObservingTime_h * 60.;
   if( Nof < 0. ) Nof = 0.;
   Ndif= Non - Nof;

   cout << "VTVMAEvaluator::getOptimalSignalEfficiency event numbers: ";
   cout << " non = " << Non;
   cout << " noff = " << Nof;
   cout << " ndif = " << Ndif << " (1 CU)" << endl;
   cout << "VTVMAEvaluator::getOptimalSignalEfficiency event numbers: ";
   cout << " (energy bin " << iEnergyBin << ", " << TMath::Power( 10., iMeanEnergy_TeV ) << " [TeV],";
   cout << " weighted mean energy " << TMath::Power( 10., fSpectralWeightedMeanEnergy_Log10TeV[iEnergyBin] ) << " [TeV] )";
   cout << endl;
       
///////////////////////////////////////////////////////////////////
// get signal and background efficiency histograms from TMVA files

   TFile iTMVAFile( iTMVARootFile.c_str() );
   if( iTMVAFile.IsZombie() )
   {
      cout << "VTVMAEvaluator::getOptimalSignalEfficiency error:" << endl;
      cout << " cannot read TMVA file " << iTMVARootFile << endl;
      cout << " (energy bin " << iEnergyBin << ")" << endl;
      return false;
   }
// get signal and background efficiency histograms
   TH1F *effS = getEfficiencyHistogram( "effS", &iTMVAFile );
   TH1F *effB = getEfficiencyHistogram( "effB", &iTMVAFile );
   if( !effS || !effB )
   {
      cout << "VTVMAEvaluator::getOptimalSignalEfficiency error:" << endl;
      cout << " cannot find signal and/or background efficiency histogram(s)" << endl;
      cout << effS << "\t" << effB << endl;
      return false;
   }
// evaluate errors on determination of background cut efficiency and remove bins with large errors
   if( fTMVAMethodName != "BOXCUTS" )
   {
      char hname[800];
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
   
   cout << "VTVMAEvaluator::getOptimalSignalEfficiency() optimization parameters: ";
   cout << "maximum signal efficiency is " << fOptmizationFixedSignalEfficiency << endl;
   cout << " (alpha: " << fOptimizationBackgroundAlpha << ")" << endl;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// optimization starts here
//////////////////////////////////////////////////////////////////////////
   double i_Signal_to_sqrtNoise = 0.;
   double i_AngularContainmentRadius = 0.;
   double i_AngularContainmentFraction = 0.;

   double i_TMVACutValue_AtMaximum = -99.;
   double i_SourceStrength_atMaximum = 0.;
   double i_SignalEfficiency_AtMaximum = -99.;
   double i_BackgroundEfficiency_AtMaximum = -99.;
   double i_Signal_to_sqrtNoise_atMaximum = 0.;
   double i_AngularContainmentRadiusAtMaximum = 0.;
   double i_AngularContainmentFractionAtMaximum = 0.;

   TGraph *iGSignal_to_sqrtNoise = 0;
   TGraph *iGSignalEvents        = 0;
   TGraph *iGBackgroundEvents    = 0;
   TGraph *iGSignal_to_sqrtNoise_Smooth = 0;
   TGraph *iGOpt_AngularContainmentRadius = 0;
   TGraph *iGOpt_AngularContainmentFraction = 0;

//////////////////////////////////////////////////////
// loop over different source strengths (in Crab Units)
// (hardwired: start at 0.001 CU to 30 CU)
   unsigned int iSourceStrengthStepSizeN = (unsigned int)((log10( 30. ) - log10( 0.001 )) / 0.005);
   for( unsigned int s = 0; s < iSourceStrengthStepSizeN; s++ )
   {
       double iSourceStrength = log10( 0.001 ) + s * 0.005;
       iSourceStrength = TMath::Power( 10., iSourceStrength );

// source events
       Ndif = (Non - Nof) * iSourceStrength;

// first quick pass to see if there is a change of reaching the required fOptmizationSourceSignificance
// (needed to speed up the calculation)
// (ignore any detail, no optimization of angular cut)
      bool bPassed = false;
      for( int i = 1; i < effS->GetNbinsX(); i++ )
      {
	 if( effB->GetBinContent( i ) > 0. && Nof > 0. )
	 {
	    if( fOptimizationBackgroundAlpha > 0. )
	    {
	       i_Signal_to_sqrtNoise = VStatistics::calcSignificance( effS->GetBinContent( i ) * Ndif + effB->GetBinContent( i ) * Nof,
								      effB->GetBinContent( i ) * Nof / fOptimizationBackgroundAlpha,
								      fOptimizationBackgroundAlpha );
// check significance criteria
	       if( i_Signal_to_sqrtNoise > fOptmizationSourceSignificance )
	       {
	           bPassed = true;
		   break;
               }
	    }
	    else break;
          }
      }
// no change to pass significance criteria -> continue to next energy bin
      if( !bPassed ) continue;

//////////////////////////////////////////////////////
// now loop over signal and background efficiency levels
      i_Signal_to_sqrtNoise = 0.;
      i_AngularContainmentRadius = 0.;
      i_AngularContainmentFraction = 0.;

      i_TMVACutValue_AtMaximum = -99.;
      i_SourceStrength_atMaximum = 0.;
      i_SignalEfficiency_AtMaximum = -99.;
      i_BackgroundEfficiency_AtMaximum = -99.;
      i_Signal_to_sqrtNoise_atMaximum = 0.;
      i_AngularContainmentRadiusAtMaximum = 0.;
      i_AngularContainmentFractionAtMaximum = 0.;

      iGSignal_to_sqrtNoise = new TGraph( 1 );
      iGSignalEvents        = new TGraph( 1 );
      iGBackgroundEvents    = new TGraph( 1 );
      iGOpt_AngularContainmentRadius = new TGraph( 1 );
      iGOpt_AngularContainmentFraction = new TGraph( 1 );

      int z = 0;
      int z_SB = 0;
// loop over all signal efficiency bins
      for( int i = 1; i < effS->GetNbinsX(); i++ )
      {
	 if( effB->GetBinContent( i ) > 0. && Nof > 0. )
	 {
	    if( fOptimizationBackgroundAlpha > 0. )
	    {
	       if( iHAngContainment )
	       {
	           getOptimalAngularContainmentRadius( effS->GetBinContent( i ), effB->GetBinContent( i ), Ndif, Nof, 
		                                       iHAngContainment, fSpectralWeightedMeanEnergy_Log10TeV[iEnergyBin],
						       i_Signal_to_sqrtNoise, i_AngularContainmentRadius, i_AngularContainmentFraction );
               }
	       else
	       {
		  i_Signal_to_sqrtNoise = VStatistics::calcSignificance( effS->GetBinContent( i ) * Ndif + effB->GetBinContent( i ) * Nof,
									 effB->GetBinContent( i ) * Nof / fOptimizationBackgroundAlpha,
									 fOptimizationBackgroundAlpha );
               }
	    }
	    else
	    {
	       i_Signal_to_sqrtNoise = 0.;
	       i_AngularContainmentRadius = 0.;
	       i_AngularContainmentFraction = fTMVAngularContainmentRadiusMax;
            }
	    if( fDebug )
	    {
	       cout << "___________________________________________________________" << endl;
	       cout << i << "\t" << Non << "\t" << effS->GetBinContent( i )  << "\t";
	       cout << Nof << "\t" << effB->GetBinContent( i ) << "\t";
	       cout << Ndif << endl;
	       cout << "\t" << effS->GetBinContent( i ) * Ndif;
	       cout << "\t" << effS->GetBinContent( i ) * Ndif + effB->GetBinContent( i ) * Nof;
	       cout << "\t" << effS->GetBinContent( i ) * Non + effB->GetBinContent( i ) * Nof;
	       cout << "\t" << effB->GetBinContent( i ) * Nof << endl;
	    }
	    if( effS->GetBinContent( i ) * Ndif > 0. )
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
		  cout << "\t number of background events lower than ";
		  cout << fOptmizationMinBackGroundEvents << ": setting signal/sqrt(noise) to 0; bin " << i << endl;
	       }
	       i_Signal_to_sqrtNoise = 0.;
	    }
// add results to a graph
	    if( iGSignal_to_sqrtNoise && i_Signal_to_sqrtNoise > 1.e-2 )
	    {
	       iGSignal_to_sqrtNoise->SetPoint( z, effS->GetBinCenter( i ), i_Signal_to_sqrtNoise );
	       iGOpt_AngularContainmentRadius->SetPoint( z, effS->GetBinCenter( i ), i_AngularContainmentRadius );
	       iGOpt_AngularContainmentFraction->SetPoint( z, effS->GetBinCenter( i ), i_AngularContainmentFraction );
	       if( fDebug )
	       {
	           cout << "\t SET " << z << "\t" << effS->GetBinCenter( i ) << "\t" << i_Signal_to_sqrtNoise << "\t";
		   cout << i_AngularContainmentRadius << "\t" << i_AngularContainmentFraction << endl;
               }
	       z++;
	    }
	    if( fDebug )
	    {
	       cout << "\t z " << z << "\t" << i_Signal_to_sqrtNoise << endl;
	       cout << "___________________________________________________________" << endl;
	    }
	 }
      } // END loop over all signal efficiency bins
// fill a histogram from these values, smooth it, and determine position of maximum significance
      double i_xmax = -99.;
      if( iGSignal_to_sqrtNoise )
      {
	 TGraphSmooth *iGSmooth = new TGraphSmooth("s");
	 iGSignal_to_sqrtNoise_Smooth = iGSmooth->SmoothKern( iGSignal_to_sqrtNoise, "normal", 0.05, 100 );
// get maximum values
	 double x = 0.;
	 double y = 0.;
	 double i_ymax = -99.;
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
	 i_SourceStrength_atMaximum       = iSourceStrength;
	 i_AngularContainmentRadiusAtMaximum = iGOpt_AngularContainmentRadius->Eval( i_xmax );
	 i_AngularContainmentFractionAtMaximum = iGOpt_AngularContainmentFraction->Eval( i_xmax );

// make sure that signal efficency is > 0 (and 1 for the case that there is no maximum found)
	 if( i_Signal_to_sqrtNoise_atMaximum < 1.e-9 )
	 {
	    cout << "VTMVAEvaluator::optimizeSensitivity: signal/sqrt(bck) too small at maximum, try previous bin" << endl;
// first try: take previous bin:
	    if( iEnergyBin > 0 )
	    {
	       i_SignalEfficiency_AtMaximum     = fSignalEfficiency[iEnergyBin-1];
	       i_TMVACutValue_AtMaximum         = fTMVACutValue[iEnergyBin-1];
	       i_BackgroundEfficiency_AtMaximum = fBackgroundEfficiency[iEnergyBin-1];
	       i_AngularContainmentRadiusAtMaximum = fAngularContainmentRadius[iEnergyBin-1];
	       i_AngularContainmentFractionAtMaximum = fAngularContainmentFraction[iEnergyBin-1];
	    }
	    else
	    {
	       i_SignalEfficiency_AtMaximum     = effS->GetBinContent( effS->GetMaximumBin() );
	       i_TMVACutValue_AtMaximum         = effS->GetXaxis()->GetBinCenter( effS->GetMaximumBin() );
	       i_BackgroundEfficiency_AtMaximum = effB->GetBinContent( effS->GetMaximumBin() );
	       i_AngularContainmentRadiusAtMaximum = iGOpt_AngularContainmentRadius->Eval( i_TMVACutValue_AtMaximum );
	       i_AngularContainmentFractionAtMaximum = iGOpt_AngularContainmentFraction->Eval( i_TMVACutValue_AtMaximum );
	    }
	 } 
      }
// check if value if really at the optimum or if information is missing from background efficiency curve
// (check if maximum was find in the last bin or if next bin content is zero)
      if( ( effB->FindBin( i_xmax ) + 1  < effB->GetNbinsX() && effB->GetBinContent( effB->FindBin( i_xmax ) + 1 ) < 1.e-10 )
	 || ( effB->FindBin( i_xmax ) == effB->GetNbinsX() ) )
      {
          if( fDebug )
	  {
	     cout << "VTMVAEvaluator::optimizeSensitivity: no optimum found" << endl;
	     cout << "\t sampling of background cut efficiency not sufficient" << endl;
	     if( effB->FindBin( i_xmax ) + 1  < effB->GetNbinsX() )
	     {
		cout << "\t bin " << effB->FindBin( i_xmax ) << "\t" << " bin content ";
		cout << effB->GetBinContent( effB->FindBin( i_xmax ) + 1 ) << endl;
	     }
          }
// now check slope of sqrtNoise curve (if close to constant -> maximum reached)
          if( iGSignal_to_sqrtNoise_Smooth->Eval( i_TMVACutValue_AtMaximum ) > 0. 
	   && iGSignal_to_sqrtNoise_Smooth->Eval( i_TMVACutValue_AtMaximum-0.02 ) /
	      iGSignal_to_sqrtNoise_Smooth->Eval( i_TMVACutValue_AtMaximum ) > 0.98 )
          {
	     cout << "VTMVAEvaluator::optimizeSensitivity: recovered energy bin ";
	     cout << iGSignal_to_sqrtNoise_Smooth->Eval( i_TMVACutValue_AtMaximum-0.02 ) /
	             iGSignal_to_sqrtNoise_Smooth->Eval( i_TMVACutValue_AtMaximum );
             cout << " (" << iEnergyBin << ")" << endl;
	     if( iEnergyBin < fTMVAOptimumCutValueFound.size() ) fTMVAOptimumCutValueFound[iEnergyBin] = true;
	  }
	  else if( iEnergyBin < fTMVAOptimumCutValueFound.size() ) fTMVAOptimumCutValueFound[iEnergyBin] = false;
      }
      else
      {
	 if( iEnergyBin < fTMVAOptimumCutValueFound.size() ) fTMVAOptimumCutValueFound[iEnergyBin] = true;
      }

// check detection criteria
      if(  i_Signal_to_sqrtNoise_atMaximum > fOptmizationSourceSignificance
        && Ndif < fOptmizationMinSignalEvents ) 
	{
	    cout << "\t passed significance but not signal events criterium";
	    cout << " (" << iSourceStrength << " CU): ";
	    cout << "sig " << i_Signal_to_sqrtNoise_atMaximum;
	    cout << ", Ndif " << Ndif << endl;
        }
      if(  i_Signal_to_sqrtNoise_atMaximum > fOptmizationSourceSignificance
        && Ndif > fOptmizationMinSignalEvents ) break;

// delete graphs
// (not in last step, keep them there for plotting)
       if( s != iSourceStrengthStepSizeN - 1 )
       {
	  if( iGSignal_to_sqrtNoise ) delete iGSignal_to_sqrtNoise;
	  if( iGSignalEvents ) delete iGSignalEvents;
	  if( iGBackgroundEvents ) delete iGBackgroundEvents;
	  if( iGSignal_to_sqrtNoise_Smooth ) delete iGSignal_to_sqrtNoise_Smooth;
	  if( iGOpt_AngularContainmentRadius ) delete iGOpt_AngularContainmentRadius;
	  if( iGOpt_AngularContainmentFraction ) delete iGOpt_AngularContainmentFraction;
       }
   } // end of loop over source strength
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// check if signal efficiency is above allowed value
   if( i_SignalEfficiency_AtMaximum > fOptmizationFixedSignalEfficiency )
   {
       if( fOptmizationFixedSignalEfficiency > 0.99 )
       {
	  i_TMVACutValue_AtMaximum         = effS->GetBinCenter( effS->GetNbinsX()-1 );
	  i_BackgroundEfficiency_AtMaximum = effB->GetBinContent( effS->GetNbinsX()-1 );
       }
       else
       {
	  for( int i = 1; i < effS->GetNbinsX(); i++ )
	  {
	     if( effS->GetBinContent( i ) < fOptmizationFixedSignalEfficiency )
	     {
		i_TMVACutValue_AtMaximum         = effS->GetBinCenter( i );
		i_BackgroundEfficiency_AtMaximum = effB->GetBinContent( i );
		i_AngularContainmentRadiusAtMaximum = iGOpt_AngularContainmentRadius->Eval( i_TMVACutValue_AtMaximum );
		i_AngularContainmentFractionAtMaximum = iGOpt_AngularContainmentFraction->Eval( i_TMVACutValue_AtMaximum );
		break;
	     }
	  }
       }
       i_SignalEfficiency_AtMaximum = fOptmizationFixedSignalEfficiency;
       cout << "VTMVAEvaluator::optimizeSensitivity: setting signal efficiency to ";
       cout << fOptmizationFixedSignalEfficiency << endl;
   } 
   else
   {
      cout << "VTMVAEvaluator::optimizeSensitivity: signal efficiency at maximum (";
      cout << i_SourceStrength_atMaximum << " CU) is ";
      cout << i_SignalEfficiency_AtMaximum << " with a significance of " << i_Signal_to_sqrtNoise_atMaximum << endl;
      cout << "\t Ndiff = " << Ndif << endl;
   }
   cout << "\t MVA parameter: " << i_TMVACutValue_AtMaximum;
   cout << ", background efficiency: " << i_BackgroundEfficiency_AtMaximum << endl;
   cout << "\t angular containment is " << i_AngularContainmentFractionAtMaximum*100. << "%, radius ";
   cout << i_AngularContainmentRadiusAtMaximum << " [deg]" << endl; 
////////////////////////////////////////////////////////////////

// get mean energy for this bin
   double iMeanEnergyAfterCuts = getMeanEnergyAfterCut( &iTMVAFile, i_TMVACutValue_AtMaximum, fEnergyCut_Log10TeV_min[iEnergyBin],
                                                        fEnergyCut_Log10TeV_max[iEnergyBin],
                                                        fTMVAMethodName, fEnergyReconstructionMethod[iEnergyBin] );
   cout << "Mean energy after cuts [TeV]: " << iMeanEnergyAfterCuts << endl;

// fill results into data vectors
   if( iEnergyBin < fSignalEfficiency.size() )     fSignalEfficiency[iEnergyBin]     = i_SignalEfficiency_AtMaximum;
   if( iEnergyBin < fBackgroundEfficiency.size() ) fBackgroundEfficiency[iEnergyBin] = i_BackgroundEfficiency_AtMaximum;
   if( iEnergyBin < fTMVACutValue.size() )         fTMVACutValue[iEnergyBin]         = i_TMVACutValue_AtMaximum;
   if( iEnergyBin < fSourceStrengthAtOptimum_CU.size() ) fSourceStrengthAtOptimum_CU[iEnergyBin] = i_SourceStrength_atMaximum;
   if( iEnergyBin < fSpectralWeightedMeanEnergy_Log10TeV.size() && iMeanEnergyAfterCuts > 0. ) 
   {
      fSpectralWeightedMeanEnergy_Log10TeV[iEnergyBin] = log10( iMeanEnergyAfterCuts );
   }
   if( iEnergyBin < fAngularContainmentRadius.size() )   fAngularContainmentRadius[iEnergyBin] = i_AngularContainmentRadiusAtMaximum;
   if( iEnergyBin < fAngularContainmentFraction.size() ) fAngularContainmentFraction[iEnergyBin] = i_AngularContainmentFractionAtMaximum;

// plot optimziation procedure and event numbers
   if( bPlotEfficiencyPlotsPerEnergy )
   {
      plotEfficiencyPlotsPerEnergy( iEnergyBin, iGSignal_to_sqrtNoise, iGSignal_to_sqrtNoise_Smooth,
                                    effS, effB,
				    fEnergyCut_Log10TeV_min[iEnergyBin], fEnergyCut_Log10TeV_max[iEnergyBin],
				    iGSignalEvents, iGBackgroundEvents );
   }

   return true;
}

void VTMVAEvaluator::getOptimalAngularContainmentRadius( double effS, double effB, double Ndif, double Nof, 
                                                         TH2D* iHAngContainment, double iEnergy_log10_TeV,
                                                         double &i_Signal_to_sqrtNoise, double &i_AngularContainmentRadius,
							 double &i_AngularContainmentFraction )
{
    i_AngularContainmentFraction = 0.;
    i_Signal_to_sqrtNoise = -99.;
    i_AngularContainmentRadius = -99.;
    if( !iHAngContainment || fOptimizationBackgroundAlpha <= 0 )
    {
       return;
    }
    double iEnergyBin = iHAngContainment->GetXaxis()->FindBin( iEnergy_log10_TeV );
    double iR_Max = iHAngContainment->GetBinContent( iEnergyBin, iHAngContainment->GetYaxis()->FindBin( fTMVAngularContainmentRadiusMax ) );

// find containment radius giving maximum significance
    double iC = 0.;
    double iR = 0.;
    double iOn = 0.;
    double iOff = 0.;
    double iSigma = 0.;
    for( int i = 1; i < iHAngContainment->GetNbinsY(); i++ )
    {
        iC = iHAngContainment->GetYaxis()->GetBinLowEdge( i );
	iR = iHAngContainment->GetBinContent( iEnergyBin, i );

// gamma point source
	iOn  = effS * Ndif * iC / fTMVAngularContainmentRadiusMax;
	iOn += effB * Nof * iR * iR / iR_Max / iR_Max;
	iOff = effB * Nof / fOptimizationBackgroundAlpha * iR * iR / iR_Max / iR_Max;

	iSigma = VStatistics::calcSignificance( iOn, iOff, fOptimizationBackgroundAlpha );

	if( iSigma >= i_Signal_to_sqrtNoise ) 
	{
	   i_Signal_to_sqrtNoise = iSigma;
	   i_AngularContainmentRadius = iR;
	   i_AngularContainmentFraction = iC;
        }
    }
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
      sprintf( htitle, "efficiency plots (bin %d, %.2f < log10(E) < %.2f)", iBin, iEnergy_Log10TeV_min, iEnergy_Log10TeV_max );
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
      sprintf( htitle, "signal / sqrt( noise ) (bin %d, %.2f < log10(E) < %.2f)", iBin, iEnergy_Log10TeV_min, iEnergy_Log10TeV_max );
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
      sprintf( htitle, "event numbers (bin %d, %.2f < log10(E) < %.2f)", iBin, iEnergy_Log10TeV_min, iEnergy_Log10TeV_max );
      TCanvas *iCanvas = new TCanvas( hname, htitle, 850, 10+iBin*30, 400, 400 );
      iCanvas->SetLeftMargin( 0.13 );
      iCanvas->SetGridx( 0 );
      iCanvas->SetGridy( 0 );

      sprintf( hname, "hBC_%d", iBin );
      TH1D *hnull = new TH1D( hname, "", 100, -1., 1. );
      hnull->SetXTitle( "cut value" );
      hnull->SetYTitle( "number of events" );
      hnull->SetMinimum( 1.e-3 );
      double x = 0.;
      double y = 0.;
      double y_max = 0.;
      for( int i = 0; i < iGBackgroundEvents->GetN(); i++ )
      {
          iGBackgroundEvents->GetPoint( i, x, y );
	  if( y > y_max ) y_max = y;
      }
      for( int i = 0; i < iGSignalEvents->GetN(); i++ )
      {
          iGSignalEvents->GetPoint( i, x, y );
	  if( y > y_max ) y_max = y;
      }
      hnull->SetMaximum( y_max * 1.5 );
      hnull->SetStats( 0 );
      hnull->GetYaxis()->SetTitleOffset( 1.5 );
      hnull->DrawCopy();

      setGraphPlottingStyle( iGBackgroundEvents, 1, 1., 20 );
      iGBackgroundEvents->Draw( "pl" );

      if( iGSignalEvents ) 
      {
         setGraphPlottingStyle( iGSignalEvents, 2, 2. );
         iGSignalEvents->Draw( "pl" );
      }

      if( iBin < fTMVACutValue.size() )
      {
         TLine *iL = new TLine( fTMVACutValue[iBin], hnull->GetMinimum(),
	                        fTMVACutValue[iBin], hnull->GetMaximum() );
	 iL->SetLineStyle( 2 );
	 iL->Draw();
         TLine *iLMinBack = new TLine( hnull->GetXaxis()->GetXmin(), fOptmizationMinBackGroundEvents,
	                               hnull->GetXaxis()->GetXmax(), fOptmizationMinBackGroundEvents );
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

void VTMVAEvaluator::setSensitivityOptimizationFixedSignalEfficiency( double iOptmizationFixedSignalEfficiency )
{
   fOptmizationFixedSignalEfficiency = iOptmizationFixedSignalEfficiency;
}

double VTMVAEvaluator::getValueFromMap( map< unsigned int, double > iDataMap, double iDefaultData,
                                        unsigned int iEnergyBin, 
                                        double iE_min_log10, double iE_max_log10, string iVariable )
{
   if( iDataMap.size() == 0 ) return iDefaultData;

   map< unsigned int, double >::iterator iIter;

   for( iIter = iDataMap.begin(); iIter != iDataMap.end(); iIter++ )
   {
// data does not depend on energy
       if( iIter->first > 9998 ) return iIter->second;

// data signal efficiency
       if( iIter->first == iEnergyBin )
       {
	  cout << "VTMVAEvaluator::getValueFromMap (" << iVariable << "): ";
	  cout << "E [" << iE_min_log10 << ", " << iE_max_log10 << "], bin ";
	  cout << iEnergyBin << ": ";
	  cout << iIter->first << "\t" << iIter->second << endl;
          return iIter->second;
       }
   }

   cout << "VTMVAEvaluator::getSignalEfficiency: warning, couldn't find a data value (" << iVariable << ") for energy bin ";
   cout << iEnergyBin << ", E=[ " << iE_min_log10 << ", " << iE_max_log10 << "] " << endl;

   return -1.;
}

double VTMVAEvaluator::getSignalEfficiency( unsigned int iEnergyBin, double iE_min_log10, double iE_max_log10 )
{
   return getValueFromMap( fSignalEfficiencyMap, fSignalEfficiencyNoVec, iEnergyBin, iE_min_log10, iE_max_log10, "SignalEfficiency" );
}

double VTMVAEvaluator::getTMVACutValue( unsigned int iEnergyBin, double iE_min_log10, double iE_max_log10 )
{
   return getValueFromMap( fTMVACutValueMap, fTMVACutValueNoVec, iEnergyBin, iE_min_log10, iE_max_log10, "MVA_CUT" );
}
    
   

void VTMVAEvaluator::printSensitivityOptimizationParameters()
{
     cout << "VTMAEvaluator: MVA cut parameter is optimized for: " << endl;
     cout << "\t" << fOptimizationObservingTime_h << " hours of observing time" << endl;
     cout << "\t" << fOptmizationSourceSignificance << " minimum significance" << endl;
     cout << "\t" << fOptmizationMinSignalEvents << " minimum number of on events" << endl;
     cout << "\t" << fOptimizationBackgroundAlpha << " signal to background area ratio" << endl;
}    

double VTMVAEvaluator::getMeanEnergyAfterCut( TFile *f, double iCut, double iEmin, double iEmax,
                                              string iMethodName, unsigned int iEnergyReconstructionMethod )
{
   if( !f ) return -99.;
   iEmin = TMath::Power( 10., iEmin );
   iEmax = TMath::Power( 10., iEmax );
   TTree *t = (TTree*)f->Get( "TrainTree" );
   if( !t )
   {
       cout << "VTMVAEvaluator::getMeanEnergyAfterCut(): test tree not found in " << f->GetName() << endl;
       return -99.;
   }
   float iErec = 0.;
   float iMVA = 0.;
   int classID;
   if( iEnergyReconstructionMethod == 0 )      t->SetBranchAddress( "Erec", &iErec );
   else if( iEnergyReconstructionMethod == 1 ) t->SetBranchAddress( "ErecS", &iErec );                                      
   ostringstream iCutName;
   iCutName << iMethodName << "_0";
   t->SetBranchAddress( iCutName.str().c_str(), &iMVA );
   t->SetBranchAddress( "classID", &classID );

   float n = 0.;
   float m = 0.;
   for( int i = 0; i < t->GetEntries(); i++ )
   {
       t->GetEntry( i );

       if( classID == 0 && iErec > 0. && iErec > iEmin && iErec < iEmax && iMVA > iCut )
       {
          m += iErec;
	  n++;
       }
    }
    if( n > 0. ) return m/n;

    return -99.;
}

