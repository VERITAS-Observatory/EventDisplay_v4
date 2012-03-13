//! VPedestalCombineLowGainFiles  combine two low gain pedestal files into a single file

#ifndef VPedestalCombineLowGainFiles_H
#define VPedestalCombineLowGainFiles_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "TH1F.h"
#include "TFile.h"
#include "TList.h"
#include "TROOT.h"

using namespace std;

class VPedestalCombineLowGainFiles
{
   private:

   bool   fDebug;

   unsigned int fChannel1_min;
   unsigned int fChannel1_max;
   TFile*       fFile1;

   unsigned int fChannel2_min;
   unsigned int fChannel2_max;
   TFile*       fFile2;

   unsigned int fTelescopeID;
   unsigned int fSumWindow_min;
   unsigned int fSumWindow_max;

   vector< TH1F* > fHped;
   vector< string > fPedLine;

   TFile* readLowGainHistograms( string iFile, unsigned int iChannel_min, unsigned int iChannel_max );
   void   reset();

   public:

   VPedestalCombineLowGainFiles();
  ~VPedestalCombineLowGainFiles() {}

   bool readLowGainPedestalFiles( string iFile1, string iFile2 );
   void setChannelNumberRange( unsigned int iChannel1_min = 0, unsigned int iChannel1_max = 249, unsigned int iChannel2_min = 250, unsigned int iChannel2_max = 499 );
   void setSummationWindowRange( unsigned int iSumWindow_min = 1, unsigned int iSumWindow_max = 20 ) { fSumWindow_min = iSumWindow_min; fSumWindow_max = iSumWindow_max; }
   void setTelescopeID( unsigned int iTelID = 1 ) { fTelescopeID = iTelID; }
   bool combineLowGainPedestalFileForAllTelescopes( unsigned int iNTel, string iCalibrationDirectory, string iRun1, string iRun2, string iOutRun );
   bool writeLowGainPedestalFile( string iOutFileName );
};

#endif
