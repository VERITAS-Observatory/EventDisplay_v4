/*!   \file makeOptimizeBoxCutsbyParameterSpaceSearch
      \brief simple cut optimization, step through the parameter space and record significances for different source strengths


      cut range and source strengths are hard coded

      using real data from e.g. Crab or Mrk 421

      \author Gernot Maier
*/

#include "VGlobalRunParameter.h"
#include "VStatistics.h"

#include "TChain.h"
#include "TFile.h"
#include "TMath.h"
#include "TTree.h"

#include <iostream>
#include <vector>

#define NORM 1

using namespace std;

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
	TH1F* hOn_NImages = new TH1F( "hNImages", "hNImages", 5, 0, 5 );
	TH1F* hOn_MSCW = new TH1F( "hMSCW", "hMSCW", 100, -2., 2. );
	TH1F* hOn_MSCL = new TH1F( "hMSCL", "hMSCL", 100, -2., 2. );
	TH1F* hOn_theta2 = new TH1F( "htheta2", "htheta2", 200, 0., 4. );
	TH1F* hOn_EChi2 = new TH1F( "hEChi2", "hEChi2", 100, 0., 100. );
	TH1F* hOn_EmissionHeightChi2 = new TH1F( "hEmissionHeightChi2", "hEmissionHeightChi2", 100, 0., 100. );
	
	TH1F* hOff_NImages = new TH1F( "hOffNImages", "hOffNImages", 5, 0, 5 );
	TH1F* hOff_MSCW = new TH1F( "hOffMSCW", "hOffMSCW", 100, -2., 2. );
	TH1F* hOff_MSCL = new TH1F( "hOffMSCL", "hOffMSCL", 100, -2., 2. );
	TH1F* hOff_theta2 = new TH1F( "hOfftheta2", "hOfftheta2", 200, 0., 4. );
	TH1F* hOff_EChi2 = new TH1F( "hOffEChi2", "hOffEChi2", 100, 0., 100. );
	TH1F* hOff_EmissionHeightChi2 = new TH1F( "hOffEmissionHeightChi2", "hOffEmissionHeightChi2", 100, 0., 100. );
	
	string fInput = argv[1];
	string fOutput = argv[2];
	
	// get normalisation factor
	double fNorm = getNorm( fInput );
	cout << "Normalisation factor (alpha) " << fNorm << endl;
	
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
	float emissionHeight = 0.;
	int NImages = 0;
	tOn->SetBranchAddress( "NImages", &NImages );
	tOn->SetBranchAddress( "MSCW", &MSCW );
	tOn->SetBranchAddress( "MSCL", &MSCL );
	tOn->SetBranchAddress( "theta2", &theta2 );
	tOn->SetBranchAddress( "EChi2", &echi2 );
	tOn->SetBranchAddress( "EmissionHeightChi2", &emissionHeight );
	tOff->SetBranchAddress( "NImages", &NImages );
	tOff->SetBranchAddress( "MSCW", &MSCW );
	tOff->SetBranchAddress( "MSCL", &MSCL );
	tOff->SetBranchAddress( "theta2", &theta2 );
	tOff->SetBranchAddress( "EChi2", &echi2 );
	tOff->SetBranchAddress( "EmissionHeightChi2", &emissionHeight );
	
	// cut values
	double mscw_step = 0.025;
	unsigned int    mscw_min_n = 1;
	double mscw_min_start = -1.2;
	unsigned int    mscw_max_n = 41;
	double mscw_max_start =  0.;
	
	double mscl_step = 0.025;
	unsigned int    mscl_min_n = 1;
	double mscl_min_start = -1.2;
	unsigned int    mscl_max_n = 41;
	double mscl_max_start =  0.;
        mscl_max_n = 1;
        mscl_max_start = 0.7;
	
	double theta2_step = 0.001;
	//    unsigned int    theta2_max_n = 41;
	//    double theat2_max_start = 0.005;
	unsigned int    theta2_max_n = 1;
	double theat2_max_start = 0.008;
	
	double emm_step = 2.;
	//unsigned int    emm_max_n = 81;
	unsigned int    emm_max_n = 1;
	double emm_max_start = 1.e12;
	
	double echi2_step = 0.1;
	//    unsigned int echi2_n = 41;
	//    double echi2_max_start = -1.;
	unsigned int echi2_n = 1;
	double echi2_max_start = 1.e99;
	
	vector< double > mscw_min;
	vector< double > mscw_max;
	vector< double > mscl_min;
	vector< double > mscl_max;
	vector< double > theta2_max;
	vector< double > echi2_max;
	vector< double > emm_max;
	
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
								
								mscw_min.push_back( imscw_min );
								mscw_max.push_back( imscw_max );
								mscl_min.push_back( imscl_min );
								mscl_max.push_back( imscl_max );
								theta2_max.push_back( itheta2_max );
								emm_max.push_back( iemm_max );
								echi2_max.push_back( iechi2_max );
							}
						}
					}
				}
			}
		}
	}
	
	
	cout << "total number of cuts: " << mscw_min.size() << endl;
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
		hOn_EChi2->Fill( echi2 );
		hOn_EmissionHeightChi2->Fill( emissionHeight );
		
		for( unsigned int c = 0; c < mscw_min.size(); c++ )
		{
		
			if( NImages < 2 )
			{
				continue;
			}
			///if( NImages < 3 ) continue;
			if( NORM )
			{
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
			}
			if( theta2 > theta2_max[c] )
			{
				continue;
			}
			if( emissionHeight > emm_max[c] ) continue;
			
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
		hOff_EChi2->Fill( echi2, fNorm );
		hOff_EmissionHeightChi2->Fill( emissionHeight, fNorm );
		
		for( unsigned int c = 0; c < mscw_min.size(); c++ )
		{
			if( NImages < 2 )
			{
				continue;
			}
			////if( NImages < 3 ) continue;
			if( NORM )
			{
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
			}
			//            if( theta2 < 0 || theta2 > theta2_max[c] ) continue;
			if( emissionHeight > emm_max[c] ) continue;
			
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
	double t_emm_max = 0.;
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
	
        t->Branch( "mscw_min", &t_mscw_min, "mscw_min/D" );
        t->Branch( "mscw_max", &t_mscw_max, "mscw_max/D" );
        t->Branch( "mscl_min", &t_mscl_min, "mscl_min/D" );
        t->Branch( "mscl_max", &t_mscl_max, "mscl_max/D" );
        t->Branch( "theta2_max", &t_theta2_max, "theta2_max/D" );
        t->Branch( "echi2_max", &t_echi2_max, "echi2_max/D" );
        t->Branch( "emm_max", &t_emm_max, "emm_max/D" );
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
	for( unsigned int c = 0; c < mscw_min.size(); c++ )
	{
		t_mscw_min = mscw_min[c];
		t_mscw_max = mscw_max[c];
		t_mscl_min = mscl_min[c];
		t_mscl_max = mscl_max[c];
		t_theta2_max = theta2_max[c];
		t_echi2_max = echi2_max[c];
		t_emm_max = emm_max[c];
		t_noff = noff[c];
		
		for( unsigned int s = 0; s < source.size(); s++ )
		{
			t_non[s] = ( non[c] - noff[c] ) * source[s] + noff[c];
			if( fNorm > 0. )
			{
				t_sig[s] = VStatistics::calcSignificance( t_non[s], noff[c], fNorm );
// calculate time needed to reach detetection significance
                                
                                double iG = t_non[s] / 1111.85;
                                double iB = noff[c] / 1111.85;
   
                               // loop over possible observation lengths
                                double sig = 0.;
                                for( int j = 0; j < fObservationTime_steps; j++ )
                                {    
                                        // log10 hours
                                        double i_t = TMath::Log10( fObservationTime_min ) + ( TMath::Log10( fObservationTime_max ) -
                                                        TMath::Log10( fObservationTime_min ) ) / ( double )fObservationTime_steps * ( double )j;
                                        // log10 hours to min
                                        i_t = TMath::Power( 10., i_t ) * 60.; 

/*                                        if( s == 0 )
                                        {
                                            cout << i_t << " min: " << iG << "\t" << iB << "\t" << i_t << "\t" << VStatistics::calcSignificance( iG * i_t, iB * i_t, fNorm ) << endl;
                                        } */
                                        sig = VStatistics::calcSignificance( iG * i_t, iB * i_t, fNorm );
                     
                                        if( sig > fSignificance_min && i_t * iG >= fEvents_min )
                                        {    
                                                t_obs5sigma[s] = i_t;
                                                break;
                                        }    

                                }

                        }
                        t->Fill();
               }
	}
	cout << "writing results to output file" << endl;
	
	hOn_NImages->Write();
	hOff_NImages->Write();
	hOn_MSCW->Write();
	hOff_MSCW->Write();
	hOn_MSCL->Write();
	hOff_MSCL->Write();
	hOn_theta2->Write();
	hOff_theta2->Write();
	hOn_EChi2->Write();
	hOff_EChi2->Write();
	hOn_EmissionHeightChi2->Write();
	hOff_EmissionHeightChi2->Write();
	t->Write();

	fO->Close();
	
	return 0;
	
}



