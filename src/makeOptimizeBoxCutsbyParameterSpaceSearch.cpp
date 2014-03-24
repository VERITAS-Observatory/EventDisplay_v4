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


	//    TTree *t = 0;
	
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
	double fNorm;
	fNorm = getNorm( fInput );
	cout << "Normalisation factor: " << fNorm << endl;
	
	
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
	//    unsigned int    mscw_max_n = 41;
	//    double mscw_max_start =  0.;
	unsigned int    mscw_max_n = 1;
	//    double mscw_max_start =  0.35;
	double mscw_max_start =  5.0;
	
	double mscl_step = 0.025;
	unsigned int    mscl_min_n = 1;
	double mscl_min_start = -1.2;
	//   unsigned int    mscl_max_n = 41;
	//    double mscl_max_start =  0.;
	unsigned int    mscl_max_n = 1;
	//    double mscl_max_start =  0.7;
	double mscl_max_start =  5.0;
	
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
	vector< double > source;                      // source strength
	vector< double > non;
	vector< double > noff;
	vector< vector< double > > sig;
	
	source.push_back( 1. );
	source.push_back( 0.1 );
	source.push_back( 0.05 );
	source.push_back( 0.03 );
	source.push_back( 0.01 );
	const unsigned int nsources = 5;
	
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
			//            if( emissionHeight > emm_max[c] ) continue;
			
			non[c]++;
		}
	}
	cout << "loop over off tree (" << tOff->GetEntries() << ")" << endl;
	for( int i = 0; i < tOff->GetEntries(); i++ )
	{
		tOff->GetEntry( i );
		
		hOff_NImages->Fill( NImages );
		hOff_MSCW->Fill( MSCW );
		hOff_MSCL->Fill( MSCL );
		hOff_theta2->Fill( theta2 );
		hOff_EChi2->Fill( echi2 );
		hOff_EmissionHeightChi2->Fill( emissionHeight );
		
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
			//            if( emissionHeight > emm_max[c] ) continue;
			
			noff[c]++;
		}
	}
	
	
	// output tree
	
	TFile* fO = new TFile( fOutput.c_str(), "RECREATE" );
	
	//    TTree *t = new TTree( "topt", "cut optimization" );
	double t_mscw_min;
	double t_mscw_max;
	double t_mscl_min;
	double t_mscl_max;
	double t_theta2_max;
	double t_emm_max;
	double t_echi2_max;
	double t_non[nsources];
	double t_noff;
	double t_sig[nsources];
	
	/*
	    t->Branch( "mscw_min", &t_mscw_min, "mscw_min/D" );
	    t->Branch( "mscw_max", &t_mscw_max, "mscw_max/D" );
	    t->Branch( "mscl_min", &t_mscl_min, "mscl_min/D" );
	    t->Branch( "mscl_max", &t_mscl_max, "mscl_max/D" );
	    t->Branch( "theta2_max", &t_theta2_max, "theta2_max/D" );
	    t->Branch( "echi2_max", &t_echi2_max, "echi2_max/D" );
	    t->Branch( "emm_max", &t_emm_max, "emm_max/D" );
	    t->Branch( "frog_max", &t_frog_max, "frog_max/D" );
	    t->Branch( "non", t_non, "non[5]/D" );
	    t->Branch( "noff", &t_noff, "noff/D" );
	    t->Branch( "sig", t_sig, "sig[5]/D" );
	*/
	
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
			}
			
			if( s == 0 )
			{
				cout << "Source " << s << endl;
				cout << "t_mscw_min " << t_mscw_min << endl;
				cout << "t_mscw_max " << t_mscw_max << endl;
				cout << "t_mscl_min " << t_mscl_min << endl;
				cout << "t_mscl_max " << t_mscl_max << endl;
				cout << "t_theta2_max " << t_theta2_max << endl;
				cout << "t_echi2_max " << t_echi2_max << endl;
				cout << "t_emm_max " << t_emm_max << endl;
				cout << "On " << t_non[s] << endl;
				cout << "Off " << t_noff << endl;
				cout << "Sig " << t_sig[s] << endl;
				cout << endl;
				
			}
			
		}
		//        t->Fill();
	}
	//    cout << "writing results to output file" << endl;
	
	//    TCanvas *cNImages = new TCanvas("cNImages","cNImages",0,0,500,500);
	//    hOn_NImages->Draw();
	//    hOff_NImages->Draw("same");
	//    cNImages->Update();
	//    TCanvas *cMSCW = new TCanvas("cMSCW","cMSCW",0,0,500,500);
	//    hOn_MSCW->Draw();
	//    hOff_MSCW->Draw("same");
	//    cMSCW->Update();
	//    TCanvas *cMSCL = new TCanvas("cMSCL","cMSCL",0,0,500,500);
	//    hOn_MSCL->Draw();
	//    hOff_MSCL->Draw("same");
	//    cMSCL->Update();
	//    TCanvas *ctheta2 = new TCanvas("ctheta2","ctheta2",0,0,500,500);
	//    hOn_theta2->Draw();
	//    hOff_theta2->Draw("same");
	//    ctheta2->Update();
	//    TCanvas *cEChi2 = new TCanvas("cEChi2","cEChi2",0,0,500,500);
	//    hOn_EChi2->Draw();
	//    hOff_EChi2->Draw("same");
	//    cEChi2->Update();
	//    TCanvas *cEmissionHeightChi2 = new TCanvas("cEmissionHeightChi2","cEmissionHeightChi2",0,0,500,500);
	//    hOn_EmissionHeightChi2->Draw();
	//    hOff_EmissionHeightChi2->Draw("same");
	//    cEmissionHeightChi2->Update();
	
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
	
	
	//    t->Write();
	fO->Close();
	
	return 0;
	
}



