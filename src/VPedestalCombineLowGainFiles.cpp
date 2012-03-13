/*! \class VPedestalCombineLowGainFiles

    low gain calibration fles are taken with one half of the camera at high voltage,
    while the other half is kept at nominal voltage

    This class combines the low gain pedestals results from two runs taken this way

    In general, only combineLowGainPedestalFileForAllTelescopes(...) has to be called
    (as long as channel 0-249 are in one file and 250-499 in the other)

    \author Gernot Maier

*/

#include "VPedestalCombineLowGainFiles.h"

VPedestalCombineLowGainFiles::VPedestalCombineLowGainFiles()
{
    fDebug = false;

    fFile1 = 0;
    fFile2 = 0;

    setChannelNumberRange();
    setSummationWindowRange();
    setTelescopeID();
}

void VPedestalCombineLowGainFiles::setChannelNumberRange( unsigned int iChannel1_min, unsigned int iChannel1_max, unsigned int iChannel2_min, unsigned int iChannel2_max )
{
   fChannel1_min = iChannel1_min;
   fChannel1_max = iChannel1_max;
   fChannel2_min = iChannel2_min;
   fChannel2_max = iChannel2_max;
}

bool VPedestalCombineLowGainFiles::readLowGainPedestalFiles( string iFile1, string iFile2 )
{
    fFile1 = readLowGainHistograms( iFile1, fChannel1_min, fChannel1_max );
    if( !fFile1 ) return false;
    fFile2 = readLowGainHistograms( iFile2, fChannel2_min, fChannel2_max );
    if( !fFile2 ) return false;

    return true;
}

TFile* VPedestalCombineLowGainFiles::readLowGainHistograms( string iFile, unsigned int iChannel_min, unsigned int iChannel_max )
{
   char hname[2000];

// read root file
   sprintf( hname, "%s.root", iFile.c_str() );
   TFile *fFile = new TFile( hname );
   if( fFile->IsZombie() )
   {
      cout << "VPedestalCombineLowGainFiles::readLowGainHistograms error reading " << hname << endl;
      return fFile;
   }
   if( !fFile->cd( "distributions" ) ) return fFile;

   cout << "reading " << iFile << " (telescope ID " <<  fTelescopeID-1 << ")" << endl;

   for( unsigned int i = iChannel_min; i <= iChannel_max; i++ )
   {
      for( unsigned int s = fSumWindow_min; s <= fSumWindow_max; s++ )
      {
	 sprintf( hname, "hped_%d_%d_%d", fTelescopeID-1, s, i );
	 TH1F* h = (TH1F*)gDirectory->Get( hname );
	 if( h ) fHped.push_back( h );
	 else
	 {
	    cout << "VPedestalCombineLowGainFiles::readLowGainHistograms warning: histogram not found " << hname << endl;
         }
      }
   }

// read text file
   sprintf( hname, "%s", iFile.c_str() );
   ifstream is;
   is.open( hname, ifstream::in );
   if( !is )
   {
      cout << "VPedestalCombineLowGainFiles::readLowGainHistograms error reading " << hname << endl;
   }
   string is_line;
   unsigned int iTemp;

   while( getline( is, is_line ) )
   {
      if( is_line.size() == 0 ) continue;

      istringstream is_stream( is_line );

      is_stream >> iTemp;
      if( iTemp != fTelescopeID-1 ) continue;

      is_stream >> iTemp;
      if( iTemp >= iChannel_min && iTemp <= iChannel_max )
      {
         fPedLine.push_back( is_line );
      }
   }

   return fFile;
}

bool VPedestalCombineLowGainFiles::writeLowGainPedestalFile( string iOutFileName )
{
   char hname[2000];

// write root file
   sprintf( hname, "%s.root", iOutFileName.c_str() );
   TFile iF( hname, "RECREATE" );
   if( iF.IsZombie() )
   {
      cout << "VPedestalCombineLowGainFiles::writeLowGainPedestalFile error creating output file " << iOutFileName << endl;
      return false;
   }
   cout << "writing root file: " << hname << endl;
   iF.mkdir( "distributions" )->cd();

   for( unsigned int i = 0; i < fHped.size(); i++ )
   {
      if( fHped[i] ) fHped[i]->Write();
   }

   iF.Close();

// write text file
   sprintf( hname, "%s", iOutFileName.c_str() );
   ofstream is;
   is.open( hname, ifstream::out );
   if( !is )
   {
      cout << "VPedestalCombineLowGainFiles::writeLowGainPedestalFile error reading " << hname << endl;
   }
   cout << "writing text file: " << hname << " (" << fPedLine.size() << ")" << endl;
   for( unsigned int i = 0; i< fPedLine.size(); i++ )
   {
      is << fPedLine[i] << endl;
   }
   is.close();

   return true;
}

void VPedestalCombineLowGainFiles::reset()
{
   fPedLine.clear();
   fHped.clear();
   if( fFile1 ) delete fFile1;
   if( fFile2 ) delete fFile2;
}

bool VPedestalCombineLowGainFiles::combineLowGainPedestalFileForAllTelescopes( unsigned int iNTel, string iCalibrationDirectory, string iRun1, string iRun2, string iOutRun )
{
   char hname1[2000];
   char hname2[2000];

   for( unsigned int i = 0; i < iNTel; i++ )
   {
      reset();
      setTelescopeID( i+1 );

      sprintf( hname1, "%s/Tel_%d/%s.lped", iCalibrationDirectory.c_str(), i+1, iRun1.c_str() );
      sprintf( hname2, "%s/Tel_%d/%s.lped", iCalibrationDirectory.c_str(), i+1, iRun2.c_str() );
      readLowGainPedestalFiles( hname1, hname2 );

      sprintf( hname1, "%s/Tel_%d/%s.lped", iCalibrationDirectory.c_str(), i+1, iOutRun.c_str() );
      writeLowGainPedestalFile( hname1 );

   }
   return true;
}
