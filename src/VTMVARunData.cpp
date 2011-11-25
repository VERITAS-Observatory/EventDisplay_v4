/*! \class VTMVARunData
    \brief run parameter data class for TMVA optimization

	Revision $Id: VTMVARunData.cpp,v 1.1.2.5 2011/04/11 16:09:05 gmaier Exp $

    \author Gernot Maier
*/

#include "VTMVARunData.h"

VTMVARunData::VTMVARunData()
{
   setDebug();

   fName = "noname";

   fOutputDirectoryName = "";
   fOutputFileName = "";

   fQualityCuts = "";
   fMCxyoffCut = "";
   fPrepareTrainingOptions = "SplitMode=random:!V";

   fSignalWeight = 1.;
   fBackgroundWeight = 1.;
   fMinSignalEvents = 50;
   fMinBackgroundEvents = 0;
}

/*!

    open data files and make data trees available

*/
bool VTMVARunData::openDataFiles()
{
   if( fDebug ) cout << "VTMVARunData::openDataFiles()" << endl;
// open signal trees
   fSignalFile.clear();
   fSignalTree.clear();

   for( unsigned int i = 0; i < fSignalFileName.size(); i++ )
   {
      fSignalFile.push_back( new TFile( fSignalFileName[i].c_str() ) );
      if( fSignalFile.back()->IsZombie() )
      {
         cout << "VTMVARunData::openDataFiles() error reading signal file: " << fSignalFileName[i] << endl;
         cout << "aborting..." << endl;
         return false;
      }
      fSignalTree.push_back( (TTree*)fSignalFile.back()->Get( "data" ) );
      if( !fSignalTree.back() )
      {
         cout << "VTMVARunData::openDataFiles() error: no data tree in signal file: " << fSignalFileName[i] << endl;
         cout << "aborting..." << endl;
         return false;
      }
   }
// open background trees
   fBackgroundFile.clear();
   fBackgroundTree.clear();
   for( unsigned int i = 0; i < fBackgroundFileName.size(); i++ )
   {
      fBackgroundFile.push_back( new TFile( fBackgroundFileName[i].c_str() ) );
      if( fBackgroundFile.back()->IsZombie() )
      {
         cout << "VTMVARunData::openDataFiles() error reading background file: " << fBackgroundFileName[i] << endl;
         cout << "aborting..." << endl;
         return false;
      }
      fBackgroundTree.push_back( (TTree*)fBackgroundFile.back()->Get( "data" ) );
      if( !fBackgroundTree.back() )
      {
         cout << "VTMVARunData::openDataFiles() error: no data tree in background file: " << fBackgroundFileName[i] << endl;
         cout << "aborting..." << endl;
         return false;
      }
   }

   if( fDebug ) cout << "VTMVARunData::openDataFiles() open output files " << endl;

///////////////////////////////////////////////////////////////////
// check how many events there are in signal and background trees (after cuts)

// loop over all energy bins
   TEntryList *i_SignalList = 0;
   TEntryList *i_BackgroundList = 0;
   bool iEnoughEvents = true;
   for( unsigned int i = 0; i < fEnergyCutData.size(); i++ )
   {
       for( unsigned int j = 0; j < fSignalTree.size(); j++ )
       {
          if( fSignalTree[j] )
          {
             fSignalTree[j]->Draw(">>+signalList", fQualityCuts && fMCxyoffCut && fEnergyCutData[i]->fEnergyCut, "entrylist");
             i_SignalList =(TEntryList*)gDirectory->Get( "signalList" );
          }
       }
       if( i_SignalList )
       {
           cout << "number of signal in energy bin " << i << "\t" << i_SignalList->GetN() << "\t required > " << fMinSignalEvents << endl;
	   cout << "  (cuts are " << fQualityCuts << "&&" << fMCxyoffCut;
	   cout << "&&" << fEnergyCutData[i]->fEnergyCut << ")" << endl;
           if( i_SignalList->GetN() < fMinSignalEvents ) iEnoughEvents = false;
           i_SignalList->Reset();
       }
       for( unsigned int j = 0; j < fBackgroundTree.size(); j++ )
       {
          if( fBackgroundTree[j] )
          {
             fBackgroundTree[j]->Draw(">>+BackgroundList", fQualityCuts && fMCxyoffCut && fEnergyCutData[i]->fEnergyCut, "entrylist");
             i_BackgroundList =(TEntryList*)gDirectory->Get( "BackgroundList" );
          }
       }
       if( i_BackgroundList )
       {
          cout << "number of background in energy bin " << i << "\t" << i_BackgroundList->GetN();
	  cout << "\t required > " << fMinBackgroundEvents << endl;
	  cout << "  (cuts are " << fQualityCuts << "&&" << fMCxyoffCut;
	  cout << "&&" << fEnergyCutData[i]->fEnergyCut << ")" << endl;
          if( i_BackgroundList->GetN() < fMinBackgroundEvents ) iEnoughEvents = false;
          i_BackgroundList->Reset();
       }
    }
    if( !iEnoughEvents )
    {
       cout << endl;
       cout << "ERROR: not enough signal or/and background events" << endl;
       cout << "please adjust energy intervals " << endl;
       cout << "exiting..." << endl;
       cout << endl;
       exit( 0 );
    }


///////////////////////////////////////////////////////////////////
// open output file
   if( fOutputFileName.size() > 0 && fOutputDirectoryName.size() > 0 )
   {
      for( unsigned int i = 0; i < fEnergyCutData.size(); i++ )
      {
         stringstream iTempS;
         gSystem->mkdir( fOutputDirectoryName.c_str() );
         if( fEnergyCutData.size() > 1 ) iTempS << fOutputDirectoryName << "/" << fOutputFileName << "_" << i << ".root";   // append a _# at the file name
	 else                            iTempS << fOutputDirectoryName << "/" << fOutputFileName << ".root";
         fOutputFile.push_back( new TFile( iTempS.str().c_str(), "RECREATE" ) );
         if( fOutputFile.back()->IsZombie() )
         {
            cout << "VTMVARunData::openDataFiles() error creating output file " << fOutputFile.back()->GetName() << endl;
            cout << "aborting..." << endl;
            return false;
         }
         if( fEnergyCutData[i] ) fEnergyCutData[i]->Write();
      }
   }

   if( fDebug ) cout << "VTMVARunData::openDataFiles() END" << endl;

   return true;
}

/*!
    print run information to screen
*/
void VTMVARunData::print()
{
    cout << "TMVA box cut configuration (" << fName << ")" << endl;
    cout << "=================================" << endl;
    cout << endl;
    cout << "MVA Methods and options: " << endl;
    for( unsigned int i = 0; i < fMVAMethod.size(); i++ )
    {
       cout << "METHOD: " << fMVAMethod[i];
       if( i < fMVAMethod_Options.size() ) cout << "  Options: " << fMVAMethod_Options[i];
       cout << endl;
    }
    cout << "list of variables: " << endl;
    for( unsigned int i = 0; i < fTrainingVariable.size(); i++ )
    {
       cout << "\t" << fTrainingVariable[i];
       if( i < fTrainingVariableType.size() )         cout << "\t\t" << fTrainingVariableType[i];
       if( i < fTrainingVariable_CutRangeMin.size() ) cout << "\t" << fTrainingVariable_CutRangeMin[i];
       if( i < fTrainingVariable_CutRangeMax.size() ) cout << "\t" << fTrainingVariable_CutRangeMax[i];
       if( i < fTrainingVariable_VarProp.size() )     cout << "\t" << fTrainingVariable_VarProp[i];
       cout << endl;
    }
    cout << endl;
    cout << "pre-training selection cuts: " << fQualityCuts << endl;
    cout << "cut on MC arrival directions: " << fMCxyoffCut << endl;
    cout << endl;
    cout << "prepare traing options: " << fPrepareTrainingOptions << endl;
    cout << "energy bins (" << fEnergyCutData.size() << ")";
    for( unsigned int i = 0; i < fEnergyCutData.size(); i++ ) cout << fEnergyCutData[i]->fEnergyCut_Log10TeV_min << ",";
    cout << endl;
// all bins should use same energy reconstruction method
    if( fEnergyCutData.size() > 0 && fEnergyCutData[0] )
    {
       cout << "energy reconstruction method " << fEnergyCutData[0]->fEnergyReconstructionMethod << endl;
    }
    cout << "signal data file(s): " << endl;
    for( unsigned int i = 0; i < fSignalFileName.size(); i++ ) cout << "\t" << fSignalFileName[i] << endl;
    cout << "background data file(s): " << endl;
    for( unsigned int i = 0; i < fBackgroundFileName.size(); i++ ) cout << "\t" << fBackgroundFileName[i] << endl;
    cout << "output file: " << fOutputFileName << " (" << fOutputDirectoryName << ")" << endl;
    cout << endl;
    cout << endl;
}    

/*!

    read ascii configuration file

*/
bool VTMVARunData::readConfigurationFile( char *iC )
{
   cout << "reading TMVA optimizer configuration from " << iC << endl;

   ifstream is;
   is.open( iC, ifstream::in);
   if( !is )
   {
      cout << "VTMVARunData::readConfigurationFile error configuration file not found: " << iC << endl;
      return false;
   }

   string is_line;
   string temp;
   fMVAMethod.clear();
   fMVAMethod_Options.clear();

   while( getline( is, is_line ) )
   {
      if(  is_line.size() > 0 )
      {
         istringstream is_stream( is_line );
         if( is_stream.eof() ) continue;

         is_stream >> temp;
         if( temp != "*" ) continue;
         if( is_stream.eof() ) continue;

          is_stream >> temp;
///////////////////////////////////////////////////////////////////////////////////////////
// MVA method and options
///////////////////////////////////////////////////////////////////////////////////////////
        if( temp == "MVA_METHOD" )
	{
	   if( !is_stream.eof() )
	   {
	      is_stream >> temp;
	      fMVAMethod.push_back( temp );
           }
	   if( !is_stream.eof() )
	   {
	      is_stream >> temp;
	      fMVAMethod_Options.push_back( temp );
           }
	   else
	   {
	      fMVAMethod_Options.push_back( "" );
           }
	}
// Box cuts: kept for backwards compatibility
         if( temp == "OPTIMIZATION_METHOD" )
         {
            if( !is_stream.eof() ) 
            {
               is_stream >> temp;
	       fMVAMethod.push_back( "BOXCUTS" );
	       fMVAMethod_Options.push_back( temp );
            }
            else
            {
               cout << "VTMVARunData::readConfigurationFile error while reading input for variable OPTIMIZATION_METHOD" << endl;
               return false;
            }
         }
///////////////////////////////////////////////////////////////////////////////////////////
// training variables
         if( temp == "VARIABLE" )
         {
            if( !is_stream.eof() ) 
            {
               char iV = 'F';
               if( !is_stream.eof() ) is_stream >> iV;
               fTrainingVariableType.push_back( iV );
               float iR = -1.;
               if( !is_stream.eof() ) is_stream >> iR;
               fTrainingVariable_CutRangeMin.push_back( iR );
               iR = -1.;
               if( !is_stream.eof() ) is_stream >> iR;
               fTrainingVariable_CutRangeMax.push_back( iR );
               temp = "NotEnforced";
               if( !is_stream.eof() ) is_stream >> temp;
               fTrainingVariable_VarProp.push_back( temp );
               fTrainingVariable.push_back( is_stream.str().substr( is_stream.tellg(), is_stream.str().size() ) );
            }
            else
            {
               cout << "VTMVARunData::readConfigurationFile error while reading input for variable VARIABLE" << endl;
               return false;
            }
         }
// preselection cut
         if( temp == "SELECTION_CUTS" )
         {
            if( !is_stream.eof() ) 
            {
               fQualityCuts = is_stream.str().substr( is_stream.tellg(), is_stream.str().size() ).c_str();
            }
            else
            {
               cout << "VTMVARunData::readConfigurationFile error while reading input for variable SELECTION_CUTS" << endl;
               return false;
            }
         } 
// MC arrival direction cut
         if( temp == "MCXYOFF" )
         {
            if( !is_stream.eof() ) 
            {
               fMCxyoffCut = is_stream.str().substr( is_stream.tellg(), is_stream.str().size() ).c_str();
            }
            else
            {
               cout << "VTMVARunData::readConfigurationFile error while reading input for variable MCXYOFF" << endl;
               return false;
            }
         } 
    
// prepare training options
         if( temp == "PREPARE_TRAINING_OPTIONS" )
	 {
	     if( !is_stream.eof() )
	     {
	        fPrepareTrainingOptions = is_stream.str().substr( is_stream.tellg(), is_stream.str().size() ).c_str();
		fPrepareTrainingOptions = VUtilities::removeSpaces( fPrepareTrainingOptions );
// remove all spaces
             }
	     else
	     {
	        cout << "VTMVARunData::readConfigurationFile error while reading input for variable PREPARE_TRAINING_OPTIONS" << endl;
		return false;
             }
         }
// signal weight
         if( temp == "SIGNALWEIGHT" )
         {
            if( !is_stream.eof() ) 
            {
               is_stream >> fSignalWeight;
            }
            else
            {
               cout << "VTMVARunData::readConfigurationFile error while reading input for variable SIGNALWEIGHT" << endl;
               return false;
            }
         }
// signal files
         if( temp == "SIGNALFILE" )
         {
            if( !is_stream.eof() ) 
            {
               is_stream >> temp;
               fSignalFileName.push_back( temp );
            }
            else
            {
               cout << "VTMVARunData::readConfigurationFile error while reading input for variable SIGNALFILE" << endl;
               return false;
            }
         }
// background weight
         if( temp == "BACKGROUNDWEIGHT" )
         {
            if( !is_stream.eof() ) 
            {
               is_stream >> fBackgroundWeight;
            }
            else
            {
               cout << "VTMVARunData::readConfigurationFile error while reading input for variable BACKGROUNDWEIGHT" << endl;
               return false;
            }
         }
// background files
         if( temp == "BACKGROUNDFILE" )
         {
            if( !is_stream.eof() ) 
            {
               is_stream >> temp;
               fBackgroundFileName.push_back( temp );
            }
            else
            {
               cout << "VTMVARunData::readConfigurationFile error while reading input for variable BACKGROUNDFILE" << endl;
               return false;
            }
         }
// output file
         if( temp == "OUTPUTFILE" )
         {
            if( !is_stream.eof() ) is_stream >> fOutputDirectoryName;
            if( !is_stream.eof() ) is_stream >> fOutputFileName;
            else
            {
               cout << "VTMVARunData::readConfigurationFile error while reading input for variable OUTPUTFILE" << endl;
               return false;
            }
         }
// energy bins
         if( temp == "ENERGYBINS" )
         {
            vector< double > iEnergyCut_Log10TeV_min;
            vector< double > iEnergyCut_Log10TeV_max;
            vector< TCut > iEnergyCut;

            unsigned int iEMethod;

            if( !is_stream.eof() ) is_stream >> iEMethod;

// read in energy bin
            while( !is_stream.eof() )
            {
                double iT = 0.;
                is_stream >> iT;
		iEnergyCut_Log10TeV_min.push_back( iT );
            }
// sort
            sort( iEnergyCut_Log10TeV_min.begin(), iEnergyCut_Log10TeV_min.end() );
// check sanity
            if( iEnergyCut_Log10TeV_min.size() < 2 )
            {
               cout << cout << "VTMVARunData::readConfigurationFile error: need at least two energy bins " << iEnergyCut_Log10TeV_min.size() << endl;
            }
// fill maximum bins
            for( unsigned int i = 1; i < iEnergyCut_Log10TeV_min.size(); i++ )
            {
               iEnergyCut_Log10TeV_max.push_back( iEnergyCut_Log10TeV_min[i] );
            }
// remove last minimum
            iEnergyCut_Log10TeV_min.pop_back();
// fill cuts
            for( unsigned int i = 0; i < iEnergyCut_Log10TeV_min.size(); i++ )
            {
                ostringstream iCut;
                if( iEMethod == 0 )
		{
		   iCut << "Erec>0.&&"  << iEnergyCut_Log10TeV_min[i]  <<  "<log10(Erec)&&log10(Erec)<" << iEnergyCut_Log10TeV_max[i];
                }
                else
		{
		   iCut << "ErecS>0.&&" <<  iEnergyCut_Log10TeV_min[i] <<  "<log10(ErecS)&&log10(ErecS)<" << iEnergyCut_Log10TeV_max[i];
                }
                iEnergyCut.push_back( iCut.str().c_str() );
            }
// filling everything into the energy data structure
            fEnergyCutData.clear();
            for( unsigned int i = 0; i < iEnergyCut_Log10TeV_min.size(); i++ )
            {
               fEnergyCutData.push_back( new VTMVARunDataEnergyCut() );
               fEnergyCutData.back()->SetName( "fDataEnergyCut" );
               fEnergyCutData.back()->fEnergyCutBin = 0;
               fEnergyCutData.back()->fEnergyCut_Log10TeV_min = iEnergyCut_Log10TeV_min[i];
               fEnergyCutData.back()->fEnergyCut_Log10TeV_max = iEnergyCut_Log10TeV_max[i];
               fEnergyCutData.back()->fEnergyCut = iEnergyCut[i];
               fEnergyCutData.back()->fEnergyReconstructionMethod = iEMethod;
            }
         }
// minimum number of events
         if( temp == "MINEVENTS" )
         {
            if( !is_stream.eof() ) is_stream >> fMinSignalEvents;
            if( !is_stream.eof() ) is_stream >> fMinBackgroundEvents;
            if( fMinBackgroundEvents == 0 || fMinSignalEvents == 0 )
            {
               cout << "VTMVARunData::readConfigurationFile error: minimum number of events should be > 0" << endl;
               return false;
            }
         }
      }
   }

   return true;
}

