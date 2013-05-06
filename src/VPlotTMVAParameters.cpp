/*  \class VPlotTMVAParameters

*/

#include "VPlotTMVAParameters.h"

VPlotTMVAParameters::VPlotTMVAParameters()
{
    fDataDirectory = "";
}

void VPlotTMVAParameters::plot()
{
   char hname[2000];
   char htitle[2000];

   for( unsigned int i = 0; i < hSignalEfficiency.size(); i++ )
   {
       sprintf( hname, "cTMVA_S_BC_%d", i );
       sprintf( htitle, "signal/background efficiency distribution (energy bin %d)", i );
       TCanvas *c = new TCanvas( hname, htitle, 100+i*20, 100+i*20, 400, 400 );
       c->SetGridx( 0 );
       c->SetGridy( 0 );

       if( hSignalEfficiency[i] )
       {
          hSignalEfficiency[i]->Draw();
	  cout << "Signal efficiency in energy bin " << i << ": ";
	  cout << hSignalEfficiency[i]->GetMean() << " +- " << hSignalEfficiency[i]->GetRMS() << endl;
       }
       if( i < hBackgroundEfficiency.size() && hBackgroundEfficiency[i] && hBackgroundEfficiency[i]->GetEntries() > 0 )
       {
          hBackgroundEfficiency[i]->Draw( "same" );
	  cout << "Background efficiency in energy bin " << i << ": ";
	  cout << hBackgroundEfficiency[i]->GetMean() << " +- " << hBackgroundEfficiency[i]->GetRMS() << endl;
       }
   }
}

bool VPlotTMVAParameters::initializeHistograms( unsigned int iWeightFileIndex_min, unsigned int iWeightFileIndex_max )
{
   char hname[2000];

   for( unsigned int i = iWeightFileIndex_min; i < iWeightFileIndex_max; i++ )
   {
       sprintf( hname, "hSignalEfficiency_%d", i );
       hSignalEfficiency.push_back( new TH1D( hname, "", 100, 0., 1. ) );
       hSignalEfficiency.back()->SetXTitle( "efficiency" );
       hSignalEfficiency.back()->SetLineWidth( 2 );

       sprintf( hname, "hBackgroundEfficiency_%d", i );
       hBackgroundEfficiency.push_back( new TH1D( hname, "", 100, 0., 1. ) );
       hBackgroundEfficiency.back()->SetXTitle( "efficiency" );
       hBackgroundEfficiency.back()->SetLineWidth( 2 );
       hBackgroundEfficiency.back()->SetLineColor( 2 );
   }

   return true;
}

void VPlotTMVAParameters::initializeWeightFiles( string iDirectory, string iTMVADirectory, 
                                                 unsigned int iWeightFileIndex_min, unsigned int iWeightFileIndex_max )
{
    if( !initializeHistograms( iWeightFileIndex_min, iWeightFileIndex_max ) )
    {
        cout << "VPlotTMVAParameters::initializeWeightFiles error initializing histograms" << endl;
        return;
    }

// loop over all sub arrays, get efficiency from optimization and fill them into histograms
    char hname[2000];
    for( unsigned int i = 0; i < fSubArrays.size(); i++ )
    {
       VTMVAEvaluator a;
       sprintf( hname, "%s/ParticleNumbers.%s.00.root", fDataDirectory.c_str(), fSubArrays[i].c_str() ); 
       a.setParticleNumberFile( hname );
       sprintf( hname, "%s/%s/%s", iDirectory.c_str(), fSubArrays[i].c_str(), iTMVADirectory.c_str() );
       a.initializeWeightFiles( hname, iWeightFileIndex_min, iWeightFileIndex_max );

       for( unsigned int j = 0; j < a.getOptimumCutValueFound().size(); j++ )
       {
           if( a.getOptimumCutValueFound()[j] )
	   {
	      if( j < a.getSignalEfficiency().size() && j < hSignalEfficiency.size() && hSignalEfficiency[j] )
	      {
	          hSignalEfficiency[j]->Fill( a.getSignalEfficiency()[j] );
              }
	      if( j < a.getBackgroundEfficiency().size() && j < hBackgroundEfficiency.size() && hBackgroundEfficiency[j] )
	      {
	          hBackgroundEfficiency[j]->Fill( a.getBackgroundEfficiency()[j] );
              }
           }
        }
     } 
}

bool VPlotTMVAParameters::setSubArrays( string iFileTxt )
{
    fSubArrays.clear();

    ifstream is; 
    is.open( iFileTxt.c_str(), ifstream::in );
    if( !is ) return false;

    string is_line;
    while( getline( is, is_line ) ) 
    {   
      fSubArrays.push_back( is_line );
    }   

    if( fSubArrays.size() == 0 ) 
    {   
        cout << "VPlotTMVAParameters::setSubArrays: no arrays found" << endl;
        cout << "\t " << iFileTxt << endl;
        return false;
    }   

    cout << "found " << fSubArrays.size() << " sub arrays" << endl;

    return true;
}

