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

   fMCEnergyRange_TeV_min = 0.05;
   fMCEnergyRange_TeV_max = 50.0;
   fMCSpectralIndex       = 2.0;

   fMCCuts = false;
   fArrayxyoff_MC_min = -1.e5;
   fArrayxyoff_MC_max =  1.e5;

   fEnergyAxisBins_log10 = 60;
   fEnergyAxisMin_log10  = -2.;
   fEnergyAxisMax_log10  =  4.;

   setDebug();
}

void VEffectiveAreaCalculatorMCHistograms::setDefaultValues()
{
// define azimuth bins
    fVMinAz.clear();
    fVMaxAz.clear();
    fVMinAz.push_back( 135.0 );       fVMaxAz.push_back( -165.0 );
    fVMinAz.push_back( 150.0 );       fVMaxAz.push_back( -150.0 );
    fVMinAz.push_back( -180. );       fVMaxAz.push_back( -120. );
    for( int i = 0; i < 13; i++ )
    {
	 fVMinAz.push_back( fVMinAz.back() + 22.5 );
	 fVMaxAz.push_back( fVMaxAz.back() + 22.5 );
    }
// (no az cut)
    fVMinAz.push_back( -1.e3 );     fVMaxAz.push_back( +1.e3 );

/////////////////////////////////////////////////////////////////
// define  spectral index bins
    fVSpectralIndex.clear();
    for( unsigned int i = 0; i < 20; i++ ) fVSpectralIndex.push_back( 2.0 + (double)i * 0.1 );

/////////////////////////////////////////////////////////////////
    fEnergyAxisBins_log10 = 60;
    fEnergyAxisMin_log10  = -2.;
    fEnergyAxisMax_log10  =  4.;
}


void VEffectiveAreaCalculatorMCHistograms::print()
{
    cout << "VEffectiveAreaCalculatorMCHistograms::print(): found ";
    cout << fVSpectralIndex.size() << " spectral index bin(s), ";
    cout << fVMinAz.size() << " azimuth bin(s)" << endl;
    if( fMCCuts ) cout << "\tMC cuts: " << fArrayxyoff_MC_min << " < MCxy < " << fArrayxyoff_MC_max << endl;
    cout << "\tMC Energy range: [" << fMCEnergyRange_TeV_min << "," << fMCEnergyRange_TeV_max << "] TeV, index " << fMCSpectralIndex << endl;

    for( unsigned int i = 0; i < fVSpectralIndex.size(); i++ )
    {
       cout << "\tSpectral index: " << fVSpectralIndex[i] << endl;

       for( unsigned int j = 0; j < fVMinAz.size(); j++ )
       {
           if( getHistogram_Emc( j, i ) && getHistogram_Emc( j, i )->GetEntries() > 0 )
	   {
	      cout << "\tAzimuth bin: [" << fVMinAz[j] << ", " << fVMaxAz[j] << "]";
	      cout << "\tEntries (MC): " << getHistogram_Emc( j, i )->GetEntries();
	      if( getHistogram_EmcWeight( j, i ) && getHistogram_EmcWeight( j, i )->GetEntries() > 0 )
	      {
		 cout << "\tEntries (MCweights): ";
		 cout << getHistogram_EmcWeight( j, i )->GetEntries();
	      }
	      cout << endl;
           }
       }
    }
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

bool VEffectiveAreaCalculatorMCHistograms::fill( double i_ze, TTree *i_MCData, bool bAzimuthBins )
{
    cout << endl;
    cout << "filling MC histograms for effective area calculation for ze " << i_ze << " [deg]" << endl;
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
    if( fMCCuts )
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
    cout << "\t array lengths az: " << i_vMinAzSize << ", spectral index: " << i_vSpectralIndexSize << endl;

// entries in MC tree (must be long, chain could contain lots of events)
    Long64_t nentries = i_MCData->GetEntries();
    cout << "total number of MC events: " << nentries << endl;
    if( fMCCuts ) cout << "(apply MC cuts)" << endl;
//////////////////////////////////////////////////////////
// now loop over all MC entries
    for( Long64_t i = 0; i < nentries; i++ )
    {
// read data (subset)
        i_MCData->GetEntry( i );

// apply MC cuts
        if( fMCCuts )
	{
	   if( i_fMCxoff*i_fMCxoff + i_fMCyoff*i_fMCyoff > fArrayxyoff_MC_max ) continue;
	   if( i_fMCxoff*i_fMCxoff + i_fMCyoff*i_fMCyoff < fArrayxyoff_MC_min ) continue;
	    
        }

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

void VEffectiveAreaCalculatorMCHistograms::initializeHistograms()
{
    initializeHistograms( fVMinAz, fVMaxAz, fVSpectralIndex, fEnergyAxisBins_log10, fEnergyAxisMin_log10, fEnergyAxisMax_log10 );
}


void VEffectiveAreaCalculatorMCHistograms::initializeHistograms( vector< double > iAzMin, vector< double > iAzMax,
                                                                 vector< double > iSpectralIndex,
								 int nbins, double xmin, double xmax )
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
        fVSpectralWeight.back()->setMCParameter( fMCSpectralIndex, fMCEnergyRange_TeV_min, fMCEnergyRange_TeV_max );
    }
// backwards compatibility
    if( fVSpectralWeight.size() > 0 && fVSpectralWeight[0] ) fSpectralWeight = fVSpectralWeight[0];
}

bool VEffectiveAreaCalculatorMCHistograms::setMonteCarloEnergyRange( double iMin, double iMax, double iMCIndex )
{
   fMCEnergyRange_TeV_min = iMin;
   fMCEnergyRange_TeV_max = iMax;
   fMCSpectralIndex       = iMCIndex;

   for( unsigned int i = 0; i < fVSpectralWeight.size(); i++ )
   {
      if( fVSpectralWeight[i] ) fVSpectralWeight[i]->setMCParameter( fMCSpectralIndex, fMCEnergyRange_TeV_min, fMCEnergyRange_TeV_max );
   }
// backwards compatibility
   if( fVSpectralWeight.size() > 0 && fVSpectralWeight[0] ) fSpectralWeight = fVSpectralWeight[0];

   return true;
}

void VEffectiveAreaCalculatorMCHistograms::setCuts( double iArrayxyoff_MC_min, double iArrayxyoff_MC_max  )
{
   fArrayxyoff_MC_min = iArrayxyoff_MC_min;
   fArrayxyoff_MC_max = iArrayxyoff_MC_max;
   if( fArrayxyoff_MC_min < 0. || fArrayxyoff_MC_max < 0. ) fMCCuts = false;
   else                                                     fMCCuts = true;
}

/*

   add histograms 

*/
bool VEffectiveAreaCalculatorMCHistograms::add( const VEffectiveAreaCalculatorMCHistograms* iMChis )
{
   if( !iMChis ) return false;

// make sure that both instants are similar
   if( !checkParameters( iMChis ) )
   {
      cout << "VEffectiveAreaCalculatorMCHistograms::add() error: parameters differ" << endl;
      return false;
   } 

// add histograms
   for( unsigned int i = 0; i < hVEmc.size(); i++ )
   {
      for( unsigned int j = 0; j < hVEmc[i].size(); j++ )
      {
         if( hVEmc[i][j] && iMChis->hVEmc[i][j] ) hVEmc[i][j]->Add( iMChis->hVEmc[i][j] );
      }
   }

   for( unsigned int i = 0; i < hVEmcSWeight.size(); i++ )
   {
      for( unsigned int j = 0; j < hVEmcSWeight[i].size(); j++ )
      {
         if( hVEmcSWeight[i][j] && iMChis->hVEmcSWeight[i][j] ) hVEmcSWeight[i][j]->Add( iMChis->hVEmcSWeight[i][j] );
      }
   }

   return true;
}

bool VEffectiveAreaCalculatorMCHistograms::checkParameters( const VEffectiveAreaCalculatorMCHistograms* iMChis )
{
   if( fDebug )
   {
       cout << "VEffectiveAreaCalculatorMCHistograms::checkParameters" << endl;
   }
   if( !iMChis ) return false;

   if( iMChis->fMCCuts != fMCCuts ) return false;
   if( TMath::Abs( iMChis->fArrayxyoff_MC_min - fArrayxyoff_MC_min ) > 1.e-3 ) return false;
   if( TMath::Abs( iMChis->fArrayxyoff_MC_max - fArrayxyoff_MC_max ) > 1.e-3 ) return false;
   if( TMath::Abs( iMChis->fMCEnergyRange_TeV_min - fMCEnergyRange_TeV_min ) > 1.e-3 ) return false;
   if( TMath::Abs( iMChis->fMCEnergyRange_TeV_max - fMCEnergyRange_TeV_max ) > 1.e-3 ) return false;
   if( TMath::Abs( iMChis->fMCSpectralIndex - fMCSpectralIndex ) > 1.e-3 ) return false;
   if( iMChis->fVMinAz.size() != fVMinAz.size() ) return false;
   for( unsigned int i = 0; i < iMChis->fVMinAz.size(); i++ ) if( TMath::Abs( iMChis->fVMinAz[i] - fVMinAz[i] ) > 1.e-3 ) return false;
   if( iMChis->fVMaxAz.size() != fVMaxAz.size() ) return false;
   for( unsigned int i = 0; i < iMChis->fVMaxAz.size(); i++ ) if( TMath::Abs( iMChis->fVMaxAz[i] - fVMaxAz[i] ) > 1.e-3 ) return false;
   if( iMChis->fVSpectralIndex.size() != fVSpectralIndex.size() ) return false;
   for( unsigned int i = 0; i < iMChis->fVSpectralIndex.size(); i++ ) if( TMath::Abs( iMChis->fVSpectralIndex[i] - fVSpectralIndex[i] ) > 1.e-3 ) return false;

   if( iMChis->hVEmc.size() != hVEmc.size() ) return false;
   for( unsigned int i = 0; i < hVEmc.size(); i++ )
   {
      if( hVEmc[i].size() != iMChis->hVEmc[i].size() ) return false;
      for( unsigned int j = 0; j < hVEmc[i].size(); j++ )
      {
	 if( hVEmc[i][j] && iMChis->hVEmc[i][j] )
	 {
	    if( hVEmc[i][j]->GetNbinsX() != iMChis->hVEmc[i][j]->GetNbinsX() ) return false;
	    if( hVEmc[i][j]->GetXaxis()->GetXmin() != iMChis->hVEmc[i][j]->GetXaxis()->GetXmin() ) return false;
	    if( hVEmc[i][j]->GetXaxis()->GetXmax() != iMChis->hVEmc[i][j]->GetXaxis()->GetXmax() ) return false;
         }
      }
   }

   if( iMChis->hVEmcSWeight.size() != hVEmcSWeight.size() ) return false;
   for( unsigned int i = 0; i < hVEmcSWeight.size(); i++ )
   {
      if( hVEmcSWeight[i].size() != iMChis->hVEmcSWeight[i].size() ) return false;
      for( unsigned int j = 0; j < hVEmcSWeight[i].size(); j++ )
      {
	 if( hVEmcSWeight[i][j] && iMChis->hVEmcSWeight[i][j] )
	 {
	    if( hVEmcSWeight[i][j]->GetNbinsX() != iMChis->hVEmcSWeight[i][j]->GetNbinsX() ) return false;
	    if( hVEmcSWeight[i][j]->GetXaxis()->GetXmin() != iMChis->hVEmcSWeight[i][j]->GetXaxis()->GetXmin() ) return false;
	    if( hVEmcSWeight[i][j]->GetXaxis()->GetXmax() != iMChis->hVEmcSWeight[i][j]->GetXaxis()->GetXmax() ) return false;
         }
      }
   }

   return true;
}

bool VEffectiveAreaCalculatorMCHistograms::matchDataVectors( vector< double > iAzMin, vector< double > iAzMax, vector< double > iSpectralIndex )
{

    vector< double > iVSpectralIndex_new;
    vector< vector< TH1D* > > ihVEmc_new;
    vector< vector< TProfile* > > iVEmcSWeight_new;
    vector< TH1D* > iH;
    vector< TProfile* > iP;

// match spectral index
    for( unsigned int i = 0; i < fVSpectralIndex.size(); i++ )
    {
        for( unsigned int j = 0; j < iSpectralIndex.size(); j++ )
	{
	   if( TMath::Abs( iSpectralIndex[j] - fVSpectralIndex[i] ) < 1.e-3 )
	   {
	      iVSpectralIndex_new.push_back( fVSpectralIndex[i] );
	      ihVEmc_new.push_back( hVEmc[i] );
	      iVEmcSWeight_new.push_back( hVEmcSWeight[i] );
           }
	}
    }
    fVSpectralIndex = iVSpectralIndex_new;
    hVEmc = ihVEmc_new;
    hVEmcSWeight = iVEmcSWeight_new;

    if( iAzMin.size() != iAzMax.size() )
    {
       cout << "VEffectiveAreaCalculatorMCHistograms::matchDataVectors error: mismatch in az vector size: " << iAzMin.size() << "\t" << iAzMax.size() << endl;
       return false;
    }
// match azimuth vector
    vector< double > iVMinAz_new;
    vector< double > iVMaxAz_new;
    for( unsigned int s = 0; s < fVSpectralIndex.size(); s++ )
    {
       iVMinAz_new.clear();
       iVMaxAz_new.clear();
       vector< double > iVMinAz = fVMinAz;
       vector< double > iVMaxAz = fVMaxAz;
       vector< TH1D* > ihVEmc_new;
       vector< TProfile* > iVEmcSWeight_new;

       for( unsigned int i = 0; i < iAzMin.size(); i++ )
       {
	   for( unsigned int j = 0; j < iVMinAz.size(); j++ )
	   {
	      if( TMath::Abs( iAzMin[i] - iVMinAz[j] ) < 1.e-3 && TMath::Abs( iAzMax[i] - iVMaxAz[j] ) < 1.e-3  )
	      {
                 iVMinAz_new.push_back( iAzMin[i] );
                 iVMaxAz_new.push_back( iAzMax[i] );
		 ihVEmc_new.push_back( hVEmc[s][j] );
		 iVEmcSWeight_new.push_back( hVEmcSWeight[s][j] );
	      }
	   }
       }
       hVEmc[s] = ihVEmc_new;
       hVEmcSWeight[s] = iVEmcSWeight_new;
    }
    fVMinAz = iVMinAz_new;
    fVMaxAz = iVMaxAz_new;

    return true;
}
