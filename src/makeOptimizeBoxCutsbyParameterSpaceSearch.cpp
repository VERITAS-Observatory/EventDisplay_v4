/*!   \file makeOptimizeBoxCutsbyParameterSpaceSearch
      \brief simple cut optimization, step through the parameter space and record significances for different source strengths

      this is a brute force approach - not very well thought through

      cut range and source strengths are hard coded

      use data from a strong gamma-ray source e.g. Crab or Mrk 421

      \author Gernot Maier
*/

#include "VGlobalRunParameter.h"
#include "VStatistics.h"

#include "TChain.h"
#include "TFile.h"
#include "TKey.h"
#include "TMath.h"
#include "TTree.h"

#include <iostream>
#include <vector>

using namespace std;

double getObservingTime( string ifile )
{
	TFile* fIn = new TFile( ifile.c_str() );
	if( fIn->IsZombie() )
	{
		exit( -1 );
	}
	
	// get normalization from run summary tree
	fIn->cd( "total_1/stereo" );
	TTree* t = ( TTree* )gDirectory->Get( "tRunSummary" );
	
	if( t )
	{
		int iRun = 0;
		double iTime = 0.;
		t->SetBranchAddress( "runOn", &iRun );
		t->SetBranchAddress( "tOn", &iTime );
		for( int i = 0; i < t->GetEntries(); i++ )
		{
			t->GetEntry( i );
			
			if( iRun == -1 )
			{
				fIn->Close();
				// return observing time in [min]
				return iTime/60.;
			}
		}
	}
	
	fIn->Close();
	
	return 0.;
	
}

double getNorm( string ifile )
{
	TFile* fIn = new TFile( ifile.c_str() );
	if( fIn->IsZombie() )
	{
		exit( -1 );
	}
	
	// get normalization from run summary tree
	fIn->cd( "total_1/stereo" );
	TTree* t = ( TTree* )gDirectory->Get( "tRunSummary" );
	
	if( t )
	{
		int iRun;
		double iOffNorm;
		t->SetBranchAddress( "runOn", &iRun );
		t->SetBranchAddress( "OffNorm", &iOffNorm );
		for( int i = 0; i < t->GetEntries(); i++ )
		{
			t->GetEntry( i );
			
			if( iRun == -1 )
			{
				fIn->Close();
				return iOffNorm;
			}
		}
	}
	
	fIn->Close();
	
	return 0.;
	
}

TChain* getTreeWithSelectedEvents( string ifile, bool iOn = true )
{
	char dname[800];
	char hname[800];
	if( iOn )
	{
		sprintf( dname, "data_on" );
	}
	else
	{
		sprintf( dname, "data_off" );
	}
	TTree* t = 0;
	TChain* c = new TChain( dname );
	
	TFile* fIn = new TFile( ifile.c_str() );
	if( fIn->IsZombie() )
	{
		return 0;
	}
	
	// get some numbers from the run summary tree
	fIn->cd( "total_1/stereo" );
	t = ( TTree* )gDirectory->Get( "tRunSummary" );
	if( t )
	{
		int iRun;
		t->SetBranchAddress( "runOn", &iRun );
		for( int i = 0; i < t->GetEntries(); i++ )
		{
			t->GetEntry( i );
			
			if( iRun != -1 )
			{
				sprintf( hname, "%s/run_%d/stereo/%s", ifile.c_str(), iRun, dname );
				c->Add( hname );
			}
		}
	}
	return c;
}


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{


        cout << endl;
	cout << "makeOptimizeBoxCutsbyParameterSpaceSearch (" << VGlobalRunParameter::getEVNDISP_VERSION() << ")" << endl;
	cout << "----------------------------" << endl;
	
	if( argc != 3 )
	{
		cout << "makeOptimizeBoxCutsbyParameterSpaceSearch <anasum file> <output file>" << endl;
		cout << endl;
		cout << "(some parameter and parameter ranges are hard-coded)" << endl;
		exit( 0 );
	}
	
	// Histograms
	TList *hOn = new TList();
	TH1F* hOn_NImages = new TH1F( "hOnNImages", "hOnNImages", 5, 0, 5 );
	hOn->Add( hOn_NImages );
	TH1F* hOn_MSCW = new TH1F( "hOnMSCW", "hOnMSCW", 200, -2., 4. );
	hOn->Add( hOn_MSCW );
	TH1F* hOn_MSCL = new TH1F( "hOnMSCL", "hOnMSCL", 200, -2., 4. );
	hOn->Add( hOn_MSCL );
	TH1F* hOn_theta2 = new TH1F( "htheta2", "hOntheta2", 200, 0., 4. );
	hOn->Add( hOn_theta2 );
	TH1F* hOn_EChi2 = new TH1F( "hOnEChi2", "hOnEChi2", 50, -3., 2. );
	hOn->Add( hOn_EChi2 );
	TH1F* hOn_SizeSecondMax = new TH1F( "hOnSizeSecondMax", "hOnSizeSecondMax", 50, 2., 7. );
	hOn->Add( hOn_SizeSecondMax );
	TH1F* hOn_EmissionHeight = new TH1F( "hOnEmissionHeight", "hOnEmissionHeight", 50, 0., 50. );
	hOn->Add( hOn_EmissionHeight );
	TH1F* hOn_EmissionHeightChi2 = new TH1F( "hOnEmissionHeightChi2", "hOnEmissionHeightChi2", 100, -3., 3. );
	hOn->Add( hOn_EmissionHeightChi2 );
	
	TList *hOff = new TList();
	TH1F* hOff_NImages = new TH1F( "hOffNImages", "hOffNImages", 5, 0, 5 );
	hOff->Add( hOff_NImages );
	TH1F* hOff_MSCW = new TH1F( "hOffMSCW", "hOffMSCW", 200, -2., 4. );
	hOff->Add( hOff_MSCW );
	TH1F* hOff_MSCL = new TH1F( "hOffMSCL", "hOffMSCL", 200, -2., 4. );
	hOff->Add( hOff_MSCL );
	TH1F* hOff_theta2 = new TH1F( "hOfftheta2", "hOfftheta2", 200, 0., 4. );
	hOff->Add( hOff_theta2 );
	TH1F* hOff_EChi2 = new TH1F( "hOffEChi2", "hOffEChi2", 50, -3., 2. );
	hOff->Add( hOff_EChi2 );
	TH1F* hOff_SizeSecondMax = new TH1F( "hOffSizeSecondMax", "hOffSizeSecondMax", 50, 2., 7. );
	hOff->Add( hOff_SizeSecondMax );
	TH1F* hOff_EmissionHeight = new TH1F( "hOffEmissionHeight", "hOffEmissionHeight", 50, 0., 50. );
	hOff->Add( hOff_EmissionHeight );
	TH1F* hOff_EmissionHeightChi2 = new TH1F( "hOffEmissionHeightChi2", "hOffEmissionHeightChi2", 100, -3., 3. );
	hOff->Add( hOff_EmissionHeightChi2 );
	TIter next( hOff );
	TH1F *h = 0;
	while( ( h = ( TH1F* )next() ) )
	{
	   h->SetLineColor(2);
        }
	
	string fInput = argv[1];
	string fOutput = argv[2];
	
	// get normalisation factor
	double fNorm = getNorm( fInput );
	cout << "Normalisation factor (alpha) " << fNorm << endl;
	double fObservingTime_min = getObservingTime( fInput );
	cout << "Total observing time: " << fObservingTime_min << " [min]" << endl;
	if( fObservingTime_min < 1.e-5 )
	{
	   exit( 0 );
        }
	
	// get input chain
	TChain* tOn = ( TChain* )getTreeWithSelectedEvents( fInput, true );
	TChain* tOff = ( TChain* )getTreeWithSelectedEvents( fInput, false );
	if( !tOn || !tOff )
	{
		cout << "error reading data trees from " << fInput << endl;
		exit( 0 );
	}
	
	double MSCW = 0.;
	double MSCL = 0.;
	double theta2 = 0.;
	double echi2 = 0.;
	double size2ndmax = 0;
	float emissionHeight = 0.;
	float emissionHeightChi2 = 0.;
	int NImages = 0;
	tOn->SetBranchAddress( "NImages", &NImages );
	tOn->SetBranchAddress( "MSCW", &MSCW );
	tOn->SetBranchAddress( "MSCL", &MSCL );
	tOn->SetBranchAddress( "theta2", &theta2 );
	tOn->SetBranchAddress( "EChi2S", &echi2 );
	tOn->SetBranchAddress( "SizeSecondMax", &size2ndmax );
	tOn->SetBranchAddress( "EmissionHeight", &emissionHeight );
	tOn->SetBranchAddress( "EmissionHeightChi2", &emissionHeightChi2 );
	tOff->SetBranchAddress( "NImages", &NImages );
	tOff->SetBranchAddress( "MSCW", &MSCW );
	tOff->SetBranchAddress( "MSCL", &MSCL );
	tOff->SetBranchAddress( "theta2", &theta2 );
	tOff->SetBranchAddress( "EChi2S", &echi2 );
	tOff->SetBranchAddress( "SizeSecondMax", &size2ndmax );
	tOff->SetBranchAddress( "EmissionHeight", &emissionHeight );
	tOff->SetBranchAddress( "EmissionHeightChi2", &emissionHeightChi2 );
	
	// cut values

	// mscw
	double mscw_step = 0.05;
	unsigned int    mscw_min_n = 1;
	double mscw_min_start = -1.2;
	unsigned int    mscw_max_n = 20;
	double mscw_max_start =  0.;
	
	// mscl
	double mscl_step = 0.05;
	unsigned int    mscl_min_n = 1;
	double mscl_min_start = -1.2;
	unsigned int    mscl_max_n = 20;
	double mscl_max_start =  0.;
        mscl_max_n = 1;
        mscl_max_start = 0.7;

	// size2ndmax
	double size_step = 50.;
	unsigned int size_n = 20;
	double size_start = 100.;
	size_n = 1;
	
	// theta2
	double theta2_step = 0.001;
	unsigned int    theta2_max_n = 1;
	double theat2_max_start = 0.008;
	
	// emmission height
	double emm_step = 0.5;
	unsigned int    emm_max_n = 20.;
	double emm_max_start = 10.;
//	emm_max_n = 1;
//	emm_max_start = 1.e99;
	unsigned int    emm_min_n = 20;
	double emm_min_start = 0.;
//	emm_min_n = 1;
	
	// emmission height Chi2 (log scale)
	double emmChi2_step = 0.05;
	double emmChi2_n = 30;
	double emmChi2_start = -1.0;
	emmChi2_start = 10.;
	emmChi2_n = 1;
	
	// echi2
	double echi2_step = 0.1;
	unsigned int echi2_n = 15;
	double echi2_max_start = -1.;
	echi2_n = 1;
	echi2_max_start = 1.e99;
	
	vector< double > mscw_min;
	vector< double > mscw_max;
	vector< double > mscl_min;
	vector< double > mscl_max;
	vector< double > theta2_max;
	vector< double > size2ndmax_min;
	vector< double > echi2_max;
	vector< double > emm_min;
	vector< double > emm_max;
	vector< double > emmChi2_max;
	
	// results
	vector< double > source;                      
	vector< double > non;
	vector< double > noff;
	vector< vector< double > > sig;
	
        // source strength
	source.push_back( 1. );
	source.push_back( 0.1 );
	source.push_back( 0.05 );
	source.push_back( 0.03 );
	source.push_back( 0.01 );
	
	vector< double > itemp( source.size(), 0. );
	
	for( unsigned int i = 0; i < mscw_min_n; i++ )
	{
		double imscw_min = mscw_min_start + mscw_step * i;
		
		for( unsigned int j = 0; j < mscw_max_n; j++ )
		{
			double imscw_max = mscw_max_start + mscw_step * j;
			
			for( unsigned int k = 0; k < mscl_min_n; k++ )
			{
				double imscl_min = mscl_min_start + mscl_step * k;
				
				for( unsigned int l = 0; l < mscl_max_n; l++ )
				{
					double imscl_max = mscl_max_start + mscl_step * l;
					
					for( unsigned int m = 0; m < theta2_max_n; m++ )
					{
						double itheta2_max = theat2_max_start + theta2_step * m;
						
						for( unsigned int n = 0; n < emm_max_n; n++ )
						{
							double iemm_max = emm_max_start + emm_step * n;
							
							for( unsigned int o = 0; o < echi2_n; o++ )
							{
							       double iechi2_max = echi2_max_start + echi2_step * o;

							       for( unsigned int p = 0; p < emm_min_n; p++ )
							       {
								       double iemm_min = emm_min_start + emm_step * n;

								       for( unsigned int q = 0; q < size_n; q++ )
								       {
								           double isize_min = size_start + size_step * q;

									   for( unsigned int r = 0; r < emmChi2_n; r++ )
									   {
									         double iemmChi2_min = emmChi2_start + emmChi2_step * r;
								
										 mscw_min.push_back( imscw_min );
										 mscw_max.push_back( imscw_max );
										 mscl_min.push_back( imscl_min );
										 mscl_max.push_back( imscl_max );
										 theta2_max.push_back( itheta2_max );
										 size2ndmax_min.push_back( isize_min );
										 emm_min.push_back( iemm_min );
										 emm_max.push_back( iemm_max );
										 echi2_max.push_back( iechi2_max );
										 emmChi2_max.push_back( iemmChi2_min );
                                                                            }
                                                                       }
                                                               }
							}
						}
					}
				}
			}
		}
	}
	
	
	cout << "total number of cuts: " << mscw_min.size() << endl;
	cout << "MSCW " << mscw_min_n << ", " << mscw_max_n << ", MSCL " << mscl_min_n << ", " << mscl_max_n << ", Theta2 " << theta2_max_n << ", ";
	cout << "EHeight " << emm_min_n << ", " << emm_max_n << ", EChi2 " << echi2_n << ", Size " << size_n << endl;
	non.assign( mscw_min.size(), 0. );
	noff.assign( mscw_min.size(), 0. );
	sig.assign( mscw_min.size(), itemp );
	
	cout << "loop over on tree  (" << tOn->GetEntries() << ")" << endl;
	for( int i = 0; i < tOn->GetEntries(); i++ )
	{
		tOn->GetEntry( i );
		
		hOn_NImages->Fill( NImages );
		hOn_MSCW->Fill( MSCW );
		hOn_MSCL->Fill( MSCL );
		hOn_theta2->Fill( theta2 );
		if( echi2 > 0. ) hOn_EChi2->Fill( log10( echi2 ) );
		if( size2ndmax > 0. ) hOn_SizeSecondMax->Fill( log10( size2ndmax ) );
		hOn_EmissionHeight->Fill( emissionHeight );
		if( emissionHeightChi2 > 0 ) hOn_EmissionHeightChi2->Fill( log10( emissionHeightChi2 ) );
		
		for( unsigned int c = 0; c < mscw_min.size(); c++ )
		{
		
			if( NImages < 2 )
			{
				continue;
			}
			  if( MSCW < mscw_min[c] )
			  {
				  continue;
			  }
			  if( MSCW > mscw_max[c] )
			  {
				  continue;
			  }
			  if( MSCL < mscl_min[c] )
			  {
				  continue;
			  }
			  if( MSCL > mscl_max[c] )
			  {
				  continue;
			  }
			  if( echi2 > echi2_max[c] )
			  {
				  continue;
			  }
			 if( theta2 > theta2_max[c] )
			 {
				continue;
			 }
			 if( emissionHeight > emm_max[c] ) continue;
			 if( emissionHeight < emm_min[c] ) continue;
		         if( emissionHeightChi2 > 0. && log10( emissionHeightChi2 ) > emmChi2_max[c] ) continue;
		         if( size2ndmax < size2ndmax_min[c] ) continue;
			
			non[c]++;
		}
	}
	cout << "loop over off tree (" << tOff->GetEntries() << ")" << endl;
	for( int i = 0; i < tOff->GetEntries(); i++ )
	{
		tOff->GetEntry( i );
		
		hOff_NImages->Fill( NImages, fNorm );
		hOff_MSCW->Fill( MSCW, fNorm );
		hOff_MSCL->Fill( MSCL, fNorm );
		hOff_theta2->Fill( theta2, fNorm );
		if( echi2 > 0. ) hOff_EChi2->Fill( log10( echi2 ), fNorm );
		if( size2ndmax > 0. ) hOff_SizeSecondMax->Fill( log10( size2ndmax ), fNorm );
		hOff_EmissionHeight->Fill( emissionHeight, fNorm );
		if( emissionHeightChi2 > 0 ) hOff_EmissionHeightChi2->Fill( log10( emissionHeightChi2 ), fNorm );
		
		for( unsigned int c = 0; c < mscw_min.size(); c++ )
		{
		       if( NImages < 2 )
	  	       {
				continue;
		       }
		       if( MSCW < mscw_min[c] )
		       {
			       continue;
		       }
		       if( MSCW > mscw_max[c] )
		       {
			       continue;
		       }
		       if( MSCL < mscl_min[c] )
		       {
			       continue;
		       }
		       if( MSCL > mscl_max[c] )
		       {
			       continue;
		       }
		       if( echi2 > echi2_max[c] )
		       {
			       continue;
		       }
		       if( emissionHeight > emm_max[c] )
		       {
			      continue;
                       }
		       if( emissionHeight < emm_min[c] ) continue;
		       if( emissionHeightChi2 > 0. && log10( emissionHeightChi2 ) > emmChi2_max[c] ) continue;
		       if( size2ndmax < size2ndmax_min[c] ) continue;
		       noff[c]++;
		}
	}
	
	
	// output tree
	TFile* fO = new TFile( fOutput.c_str(), "RECREATE" );
	
	TTree *t = new TTree( "topt", "cut optimization" );
        t->SetMarkerStyle(7);
	double t_mscw_min = 0.;
	double t_mscw_max = 0.;
	double t_mscl_min = 0.;
	double t_mscl_max = 0.;
	double t_theta2_max = 0.;
	double t_size2ndmax_min = 0.;
	double t_emm_min = 0.;
	double t_emm_max = 0.;
	double t_emmChi2_max = 0.;
	double t_echi2_max = 0.;
	unsigned int nsources = source.size();
        if( nsources >= 10000 ) 
        {
             cout << "source strength vector too large" << endl;
             exit(0);
        }
        double t_sourceStrength[10000];
	double t_non[10000];
	double t_noff = 0.;
	double t_sig[10000];
        double t_obs5sigma[10000];
        for( int i = 0; i < 10000; i++ )
        {
            t_sourceStrength[i] = 0.;
            t_non[i] = 0.;
            t_sig[i] = 0.;
            t_obs5sigma[i] = 0.;
        }
	
        t->Branch( "MSCW_min", &t_mscw_min, "MSCW_min/D" );
        t->Branch( "MSCW_max", &t_mscw_max, "MSCW_max/D" );
        t->Branch( "MSCL_min", &t_mscl_min, "MSCL_min/D" );
        t->Branch( "MSCL_max", &t_mscl_max, "MSCL_max/D" );
        t->Branch( "theta2_max", &t_theta2_max, "theta2_max/D" );
        t->Branch( "EChi2_max", &t_echi2_max, "EChi2_max/D" );
	t->Branch( "SizeSecondMax_min", &t_size2ndmax_min, "SizeSecondMax_min/D" );
        t->Branch( "EmissionHeight_min", &t_emm_min, "EmissionHeight_min/D" );
        t->Branch( "EmissionHeight_max", &t_emm_max, "EmissionHeight_max/D" );
	t->Branch( "EmissionHeightChi2_max", &t_emmChi2_max, "EmissionHeightChi2_max/D" );
        t->Branch( "nsource", &nsources, "nsource/i" );
        t->Branch( "CU", t_sourceStrength, "CU[nsource]/D" );
        t->Branch( "non", t_non, "non[nsource]/D" );
        t->Branch( "noff", &t_noff, "noff/D" );
        t->Branch( "sig", t_sig, "sig[nsource]/D" );
        t->Branch( "obs5sigma", t_obs5sigma, "obs5sigma[nsource]/D" );

        for( unsigned int i = 0; i < source.size(); i++ ) t_sourceStrength[i] = source[i];

        // units are hours
        int fObservationTime_steps = 1000;
        double fObservationTime_min = 0.5e-3; 
        double fObservationTime_max = 5.e4;
        double fSignificance_min = 5.;
        double fEvents_min = 0.;
        
	cout << endl;
	cout << "calculating significances" << endl;
	cout << "=========================" << endl;
	for( unsigned int c = 0; c < mscw_min.size(); c++ )
	{
		t_mscw_min = mscw_min[c];
		t_mscw_max = mscw_max[c];
		t_mscl_min = mscl_min[c];
		t_mscl_max = mscl_max[c];
		t_theta2_max = theta2_max[c];
		t_echi2_max = echi2_max[c];
		t_emm_min = emm_min[c];
		t_emm_max = emm_max[c];
		t_emmChi2_max = emmChi2_max[c];
		t_size2ndmax_min = size2ndmax_min[c];
		t_noff = noff[c];

                // require a minimum number of on and off events
		if( noff[c] < 10 || non[c] < 10 ) continue;
		
		for( unsigned int s = 0; s < source.size(); s++ )
		{
			t_non[s] = ( non[c] - noff[c] * fNorm ) * source[s] + noff[c] * fNorm;
			if( fNorm > 0. )
			{
				t_sig[s] = VStatistics::calcSignificance( t_non[s], noff[c], fNorm );
// calculate time needed to reach detetection significance
                                
                                double iG = t_non[s] / fObservingTime_min;
                                double iB = noff[c] / fObservingTime_min;
   
                               // loop over possible observation lengths
                                double sig = 0.;
                                for( int j = 0; j < fObservationTime_steps; j++ )
                                {    
                                        // log10 hours
                                        double i_t = TMath::Log10( fObservationTime_min ) + ( TMath::Log10( fObservationTime_max ) -
                                                        TMath::Log10( fObservationTime_min ) ) / ( double )fObservationTime_steps * ( double )j;
                                        // log10 hours to min
                                        i_t = TMath::Power( 10., i_t ) * 60.; 

                                        sig = VStatistics::calcSignificance( iG * i_t, iB * i_t, fNorm );
                     
                                        if( sig > fSignificance_min && i_t * iG >= fEvents_min )
                                        {    
                                                t_obs5sigma[s] = i_t / 60.;
                                                break;
                                        }    
                                }

                        }
               }
	       t->Fill();
	}
	cout << "writing results to output file: " << fO->GetName() << endl;

	hOn->Write();
	hOff->Write();
	t->Write();
	fO->Close();
	
	return 0;
	
}



