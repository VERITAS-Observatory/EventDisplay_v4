/*! \CLASS veFFECTIVEaREAcALCULATORmchISTOGRAMS
    \brief filling, reading, writing of MC histograms for effective area calculation

    Revision $Id: VEffectiveAreaCalculatorMCHistograms.cpp,v 1.1.2.4.4.2 2011/03/31 14:51:25 gmaier Exp $

    \author
    Gernot Maier
*/

#include "VEffectiveAreaCalculatorMCHistograms.h"

VEffectiveAreaCalculatorMCHistograms::VEffectiveAreaCalculatorMCHistograms()
{
// default name
   SetName( "MChistos" );

   fSpectralWeight = 0;
   fCuts = 0;

}

void VEffectiveAreaCalculatorMCHistograms::print()
{
    cout << "VEffectiveAreaCalculatorMCHistograms::print(): found ";
    cout <<  fVSpectralIndex.size() << " spectral index bin(s), ";
    cout << fVMinAz.size() << " azimuth bin(s)" << endl;

    for( unsigned int i = 0; i < fVSpectralIndex.size(); i++ )
    {
       cout << "Spectral index: " << fVSpectralIndex[i] << endl;

       for( unsigned int j = 0; j < fVMinAz.size(); j++ )
       {
           cout << "\t Azimuth bin: [" << fVMinAz[j] << ", " << fVMaxAz[j] << "]";
	   if( getHistogram_Emc( j, i ) ) cout << "\tEntries (MC): " << getHistogram_Emc( j, i )->GetEntries();
	   if( getHistogram_EmcWeight( j, i ) )
	   {
	      cout << "\tEntries (MCweights): ";
	      cout << getHistogram_EmcWeight( j, i )->GetEntries();
           }
	   cout << endl;
       }
    }
}

bool VEffectiveAreaCalculatorMCHistograms::add()
{
   return false;
}

TH1D* VEffectiveAreaCalculatorMCHistograms::getHistogram_Emc( unsigned int iAz, unsigned int iIndex )
{
   if( iIndex < hVEmc.size() )
   {
      if( iAz < hVEmc[iIndex].size() ) return hVEmc[iIndex][iAz];
   }

   return 0;
}

TProfile* VEffectiveAreaCalculatorMCHistograms::getHistogram_EmcWeight( unsigned int iAz, unsigned int iIndex )
{
   if( iIndex < hVEmcSWeight.size() )
   {
      if( iAz < hVEmcSWeight[iIndex].size() ) return hVEmcSWeight[iIndex][iAz];
   }

   return 0;
}

bool VEffectiveAreaCalculatorMCHistograms::readFromEffectiveAreaTree( string iFile )
{
   iFile = "nofile";
   return false;
}

bool VEffectiveAreaCalculatorMCHistograms::readFromEffectiveAreaFile( string iFile )
{
   iFile = "nofile";
   return false;
}

bool VEffectiveAreaCalculatorMCHistograms::fill( double i_ze, TChain *i_MCData, bool bAzimuthBins )
{
    cout << endl;
    cout << "filling MC histograms for effective area calculation" << endl;
    cout << "=========================================================================================" << endl;
    if( fVSpectralWeight.size() > 0 && fVSpectralWeight[0] ) fVSpectralWeight[0]->print();
    cout << "=========================================================================================" << endl;
    cout << endl;

    if( !i_MCData )
    {
       cout << "VEffectiveAreaCalculatorMCHistograms::fill error: no MC data chain" << endl;
       return false;
    }

    float i_fMCE0 = 0.;
    float i_fMCAz = 0.;
    float i_fMCxoff = 0.;
    float i_fMCyoff = 0.;
    i_MCData->SetBranchAddress( "MCe0", &i_fMCE0 );
    if( bAzimuthBins ) i_MCData->SetBranchAddress( "MCaz", &i_fMCAz );
    if( fCuts && fCuts->isMCCuts() )
    {
       i_MCData->SetBranchAddress( "MCxoff", &i_fMCxoff );
       i_MCData->SetBranchAddress( "MCyoff", &i_fMCyoff );
    }

// spectral weight
    double i_weight = 1.;
// MC energy (log10)
    double eMC = 0.;

// array lengths
    unsigned int i_vMinAzSize        = fVMinAz.size();
    unsigned int i_vSpectralIndexSize = fVSpectralWeight.size();

// entries in MC tree (must be long, chain could contain lots of events)
    Long64_t nentries = i_MCData->GetEntries();
    cout << "total number of MC events: " << nentries << endl;
    if( fCuts && fCuts->isMCCuts() ) cout << "(apply MC cuts)" << endl;
//////////////////////////////////////////////////////////
// now loop over all MC entries
    for( Long64_t i = 0; i < nentries; i++ )
    {
// read data (subset)
        i_MCData->GetEntry( i );

// apply MC cuts
        if( !(fCuts && fCuts->applyMCXYoffCut( i_fMCxoff, i_fMCyoff, false ) ) ) continue;

// log of MC energy
        eMC = log10( i_fMCE0 );

// fill the MC histogram for all az bins
	for( unsigned int i_az = 0; i_az < i_vMinAzSize; i_az++ )
	{
// check which azimuth bin we are
	   if( bAzimuthBins && i_ze > 3. )
	   {
// confine MC az to -180., 180.
	      if( i_fMCAz > 180. ) i_fMCAz -= 360.;
// expect bin like [135,-135]
	      if( fVMinAz[i_az] > fVMaxAz[i_az] )
	      {
		     if( i_fMCAz < fVMinAz[i_az] && i_fMCAz > fVMaxAz[i_az] ) continue;
	      }
// expect bin like [-135,-45.]
	      else
	      {
		  if( i_fMCAz < fVMinAz[i_az] || i_fMCAz > fVMaxAz[i_az] ) continue;
	      }
	   }
// loop over all spectral index
	   for( unsigned int s = 0; s < i_vSpectralIndexSize; s++ )
	   {
// weight by spectral index
              i_weight = fVSpectralWeight[s]->getSpectralWeight( i_fMCE0 );

// fill MC histograms
	      if( hVEmc[s][i_az] )        hVEmc[s][i_az]->Fill(        eMC, i_weight );
	      if( hVEmcSWeight[s][i_az] ) hVEmcSWeight[s][i_az]->Fill( eMC, i_weight );
           }
        }
   } // end of loop over all MC entries

   return true;
}


void VEffectiveAreaCalculatorMCHistograms::initializeHistograms( vector< double > iAzMin, vector< double > iAzMax, vector< double > iSpectralIndex, int nbins, double xmin, double xmax )
{
    fVMinAz = iAzMin;
    fVMaxAz = iAzMax;
    fVSpectralIndex = iSpectralIndex;

    char hname[400];

    vector< TH1D* > iT_TH1D;
    vector< TProfile* > iT_TProfile;

    for( unsigned int i = 0; i < fVSpectralIndex.size(); i++ )
    {
        iT_TProfile.clear();

// histograms for effective area calculation
        iT_TH1D.clear();
        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVVEmc_%d_%d", i, j );
	    iT_TH1D.push_back( new TH1D( hname, "", nbins, xmin, xmax ) );
	    iT_TH1D.back()->SetXTitle( "energy_{MC} [TeV]" );
	    iT_TH1D.back()->SetYTitle( "entries" );
	    iT_TH1D.back()->Sumw2();
        }
        hVEmc.push_back( iT_TH1D );

        for( unsigned int j = 0; j < fVMinAz.size(); j++ )
        {
            sprintf( hname, "hVVEmcSWeight_%d_%d", i, j );
	    iT_TProfile.push_back( new TProfile( hname, "", nbins, xmin, xmax, 0., 1.e12 ) );
	    iT_TProfile.back()->SetXTitle( "energy_{MC} [TeV]" );
	    iT_TProfile.back()->SetYTitle( "spectral weight" );
        }
        hVEmcSWeight.push_back( iT_TProfile );
    }

// set spectral weight vector
    for( unsigned int s = 0; s < fVSpectralIndex.size(); s++ )
    {
        fVSpectralWeight.push_back( new VSpectralWeight() );
// weight by spectral index
        fVSpectralWeight.back()->setSpectralIndex( fVSpectralIndex[s] );
        fVSpectralWeight.back()->setMCParameter( 2.0, 0.05, 50. );
    }
// backwards compatibility
    if( fVSpectralWeight.size() > 0 && fVSpectralWeight[0] ) fSpectralWeight = fVSpectralWeight[0];
}

bool VEffectiveAreaCalculatorMCHistograms::setMonteCarloEnergyRange( double iMin, double iMax, double iMCIndex )
{
   for( unsigned int i = 0; i < fVSpectralWeight.size(); i++ )
   {
      if( fVSpectralWeight[i] ) fVSpectralWeight[i]->setMCParameter( iMCIndex, iMin, iMax );
   }
// backwards compatibility
   if( fVSpectralWeight.size() > 0 && fVSpectralWeight[0] ) fSpectralWeight = fVSpectralWeight[0];

   return true;
}
