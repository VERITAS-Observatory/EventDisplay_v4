/*! class VHiLoTools

    tools for doing HiLo analysis


*/

# include "VHiLoTools.h"
//ClassImp(VHiLoTools);


#define LOVMAX 600.
#define VMAX 2000.
#define BINS 200

#define GMIN 0.01
#define GMAX 10.

#define GEMIN 0.
#define GEMAX 0.2

VHiLoTools::VHiLoTools()
{

	fDebug = false;
	
}

VHiLoTools::~VHiLoTools()
{
	fDebug = false;
}


void VHiLoTools::help()
{

	cout << "VHiLoTools:  Tools for analysing and looking at HiLo runs" << endl << endl;
	
//cout << "void makeRootFileFromDSTFile( int tel, int iSumWindow, int ifile1, int ifile2, string dataDir )" << endl << endl;

	cout << "void getMeanRMS( string ifile, int tel, int sumwindow, bool bPlot)" << endl << endl;
	cout << "Used to get mean and RMS of HiLo run" << endl << endl;
	
	
	cout << "void mergeHiLoFiles( string infile1, string infile2, int SW, int TEL )" << endl << endl;
	cout << "merge files made with different sum windows" << endl << endl;
	
}

void VHiLoTools::makeRootFileFromDSTFile( int tel, int iSumWindow, int ifile1, int ifile2, string dataDir )
{

	char infile1[500];
	char infile2[500];
	char infile3[500];
	char infile4[500];
	
	char treeName[500];
	char foutfile[500];
	
	vector<double> out;
	int chanID;
	double gainMult, gainMultE, chi2;
	int dof;
	
	sprintf( infile1, "%s/%d/%d.DST.root", dataDir.c_str(), iSumWindow, ifile1 );
	sprintf( infile2, "%s/%d/%d.DST.root", dataDir.c_str(), iSumWindow, ifile2 );
	sprintf( infile3, "%s/18/%d.DST.root", dataDir.c_str(), ifile1 );
	sprintf( infile4, "%s/18/%d.DST.root", dataDir.c_str(), ifile2 );
	
	
	sprintf( treeName, "lowGainMultiplier_%d_%d", tel + 1, iSumWindow );
	sprintf( foutfile, "%d_%d_SW%d_T%d.root", ifile1, ifile2, iSumWindow, tel );
	
	TFile* outfile = new TFile();
	if( !gSystem->AccessPathName( foutfile ) )
	{
		outfile->Open( foutfile, "UPDATE" );
	}
	else
	{
		outfile->Open( foutfile, "RECREATE" );
	}
	TTree* outtree = new TTree( treeName, "outtree" );
	
	
	outtree->Branch( "chanID", &chanID, "chanID/I" );
	outtree->Branch( "gainMult", &gainMult, "gainMult/D" );
	outtree->Branch( "gainMultE", &gainMultE, "gainMultE/D" );
	outtree->Branch( "chi2", &chi2, "chi2/D" );
	outtree->Branch( "dof", &dof, "dof/I" );
	
	TFile* dstfile1 = new TFile( infile1 );
	TFile* dstfile2 = new TFile( infile2 );
	TFile* dstfile3 = new TFile( infile3 );
	TFile* dstfile4 = new TFile( infile4 );
	
	int lower  = 0;
	int higher = 250;
	for( int i = lower; i < higher; i++ )
	{
		out = lowGainCalib( dstfile1, dstfile3, tel, i, false );
		gainMult  = out[0];
		gainMultE = out[1];
		chi2      = out[2];
		dof       = ( int )out[3];
		chanID    = i;
		if( gainMult > 10. )
		{
			gainMultE = 0.;
		}
		outtree->Fill();
	}
	
	lower  = 250;
	higher = 499;
	for( int i = lower; i < higher; i++ )
	{
		out = lowGainCalib( dstfile2, dstfile4, tel, i, false );
		gainMult  = out[0];
		gainMultE = out[1];
		chi2      = out[2];
		dof       = ( int )out[3];
		chanID    = i;
		if( gainMult > 10. )
		{
			gainMultE = 0.;
		}
		outtree->Fill();
	}
	
	dstfile1->Close();
	dstfile2->Close();
	
	outfile->Write();
	outfile->Close();
	
	return;
	
}


vector<double> VHiLoTools::lowGainCalib( TFile* dstfile, TFile* dstfile18, int tel, int channel, bool bPlot )
{

	vector<double> out;
	
	bool bDebug = false;
	
	char hname[500];
	
	sprintf( hname, "Sum Tel %d Chan %d", tel + 1, channel + 1 );
	
	//gStyle->SetOptStat(1111111);
	//gStyle->SetOptFit(1111111);
	//gStyle->SetOptStat(0);
	
	int loNUM;
	int hiNUM;
	double loAverage;
	double loAverage2;
	double hiAverage;
	double hiAverage2;
	
	unsigned int eventNumber;
	float sum[4][2900];
	UShort_t sumfirst[4][2900];
	UShort_t HiLo[4][2900];
	UShort_t sumwindow[4][2900];
	
	unsigned int eventNumber18;
	float sum18[4][2900];
	UShort_t HiLo18[4][2900];
	UShort_t sumwindow18[4][2900];
	
	TTree* dsttree = ( TTree* )dstfile->Get( "dst" );
	TTree* dsttree18 = ( TTree* )dstfile18->Get( "dst" );
	
	dsttree->SetBranchAddress( "sumfirst", &sumfirst );
	dsttree->SetBranchAddress( "sum", &sum );
	dsttree->SetBranchAddress( "HiLo", &HiLo );
	dsttree->SetBranchAddress( "sumwindow", &sumwindow );
	dsttree->SetBranchAddress( "eventNumber", &eventNumber );
	
	dsttree18->SetBranchAddress( "sum", &sum18 );
	dsttree18->SetBranchAddress( "HiLo", &HiLo18 );
	dsttree18->SetBranchAddress( "sumwindow", &sumwindow18 );
	dsttree18->SetBranchAddress( "eventNumber", &eventNumber18 );
	
	TH2F* hscatLow = new TH2F( "hscatLow", "hscatLow", BINS, 0., LOVMAX, BINS, 0., VMAX );
	TH2F* hscatHi = new TH2F( "hscatHi", "hscatHi", BINS, 0., LOVMAX, BINS, 0., VMAX );
	TH2F* hscatSum = new TH2F( "hscatSum", hname, BINS, 0., LOVMAX, BINS, 0., VMAX );
	TH1F* hlinLow = new TH1F( "hlinLow", "hlinLow", BINS, 0., LOVMAX );
	TH1F* hlinHi = new TH1F( "hlinHi", "hlinHi", BINS, 0., LOVMAX );
	TH1F* hlinSum = new TH1F( "hlinSum", "hlinSum", BINS, 0., LOVMAX );
	
	hscatSum->GetXaxis()->SetTitle( "Average Integrated Charge Low Voltage [dc]" );
	hscatSum->GetYaxis()->SetTitle( "[dc]     " );
	
	int lowLOW;
	int lowHI;
	
	if( channel >= 250. )
	{
		lowLOW = 0;
		lowHI = 250;
	}
	else
	{
		lowLOW = 250;
		lowHI = 499;
	}
	
	for( int i = 0; i < 1000; i++ )
	{
	
		dsttree->GetEntry( i );
		dsttree18->GetEntry( i );
		
		if( eventNumber != eventNumber18 )
		{
			cout << "HELP!: eventNumbers not matching... " << i;
		}
		
// do zeroing
		loNUM = 0;
		hiNUM = 0;
		loAverage = 0.;
		loAverage2 = 0.;
		hiAverage = 0.;
		hiAverage2 = 0.;
		
//   Hi Gain Stuff (Low Voltage half of Camera)
		for( int chan = lowLOW; chan < lowHI; chan++ )
		{
			if( HiLo18[tel][chan] == 0 )
			{
				if( sum18[tel][chan] > 0. )
				{
					loAverage += sum18[tel][chan];
					loAverage2 += sum18[tel][chan] * sum18[tel][chan];
					loNUM++;
				}
			}
		}
		loAverage /= loNUM;
		
//    Low Gain Stuff (Normal Voltage half of Camera)
		if( 1 )
		{
		
			if( sum[tel][channel] > 0. )
			{
				hiAverage += sum[tel][channel];
				hiNUM++;
				
				if( loAverage > 10. )
				{
				
					if( HiLo[tel][channel] == 1 )
					{
						hscatSum->Fill( loAverage, sum[tel][channel] / 6. );
					}
					else
					{
						hscatSum->Fill( loAverage, sum[tel][channel] );
					}
					
					hlinSum->Fill( loAverage );
					if( HiLo[tel][channel] == 1 )
					{
						hscatLow->Fill( loAverage, sum[tel][channel] / 6. );
					}
					if( HiLo[tel][channel] == 1 )
					{
						hlinLow->Fill( loAverage );
					}
					if( HiLo[tel][channel] == 0 )
					{
						hscatHi->Fill( loAverage, sum[tel][channel] );
					}
					if( HiLo[tel][channel] == 0 )
					{
						hlinHi->Fill( loAverage );
					}
					
				}
			}
			
		}
		
	}
	
	double yfpos[20];
	double eyfpos[20];
	double xfpos[20];
	double exfpos[20];
	int    nfpos = 0;
	
	double ypos[20];
	double eypos[20];
	double xpos[20];
	double expos[20];
	int    npos;
	
	
	// Hi Gain
	if( bPlot )
	{
		TCanvas* c1 = new TCanvas( "c1", "c1", 0, 0, 500, 500 );
		hscatHi->Draw( "zcol" );
		c1->Update();
	}
	npos = lowGainVector( hscatHi, hlinHi, xpos, ypos, expos, eypos );
	if( npos == 0 )
	{
		cout << tel << " " << channel << " No Fit High " << endl;
		gDirectory->GetList()->Delete();
		out.push_back( 0. );
		out.push_back( 0. );
		return out;
	}
	TGraphErrors* gHi = new TGraphErrors( npos, xpos, ypos, expos, eypos );
	gHi->SetMarkerColor( 2 );
	gHi->SetMarkerStyle( 20 );
	gHi->SetMarkerSize( 1.5 );
	if( bPlot )
	{
		gHi->Draw( "P" );
	}
	
	int TOP;
	if( npos == 1 )
	{
		TOP = npos;
	}
	else
	{
		TOP = npos - 1;
	}
	
	///LATER RUNS if( TOP > 3 ) TOP = 3;
	
	for( int i = 0; i < TOP; i++ )
	{
		xfpos[i]  = xpos[i];
		exfpos[i] = expos[i];
		yfpos[i]  = ypos[i];
		eyfpos[i] = eypos[i];
		nfpos++;
	}
	
	double breakpoint = -999.;
	breakpoint = xpos[TOP - 1];
	
// Low Gain
	if( bPlot )
	{
		TCanvas* c2 = new TCanvas( "c2", "c2", 510, 0, 500, 500 );
		hscatLow->Draw( "zcol" );
		c2->Update();
	}
	npos = lowGainVector( hscatLow, hlinLow, xpos, ypos, expos, eypos );
	if( npos == 0 )
	{
		cout << tel << " " << channel << " No Fit Low " << endl;
		gDirectory->GetList()->Delete();
		out.push_back( 0. );
		out.push_back( 0. );
		return out;
	}
	TGraphErrors* gLow = new TGraphErrors( npos, xpos, ypos, expos, eypos );
	gLow->SetMarkerColor( 2 );
	gLow->SetMarkerStyle( 20 );
	gLow->SetMarkerSize( 1.5 );
	if( bPlot )
	{
		gLow->Draw( "P" );
	}
	
	int itemp = nfpos - 1;
	for( int i = 1; i < npos; i++ )
	{
		xfpos[i + itemp]  = xpos[i];
		exfpos[i + itemp] = expos[i];
		yfpos[i + itemp]  = ypos[i];
		eyfpos[i + itemp] = eypos[i];
		nfpos++;
	}
	
	if( bDebug )
	{
		cout << "**********************************" << endl;
		for( int i = 0; i < nfpos; i++ )
		{
			cout << xfpos[i] << " ";
			cout << exfpos[i] << " ";
			cout << yfpos[i] << " ";
			cout << eyfpos[i] << endl;;
		}
		cout << "**********************************" << endl;
	}
	
	breakpoint += xpos[1];
	breakpoint /= 2.0;
	
	
	// Total Fit
	TF1* splitFit = new TF1( "splitFit", splitFitFunc, 10., LOVMAX, 3 );
	double pars[3] = { 1., 10., breakpoint };
	//double pars[2] = { 1., 10.,};
	splitFit->SetParameters( pars );
	splitFit->SetParNames( "Shift", "Slope", "Break" );
	//splitFit->SetParNames("Shift","Slope");
	
	splitFit->FixParameter( 2, breakpoint );
	
	if( bPlot )
	{
		TCanvas* c3 = new TCanvas( "c3", "c3", 1020, 0, 500, 500 );
		hscatSum->Draw( "zcol" );
		c3->Update();
	}
	npos = lowGainVector( hscatSum, hlinSum, xpos, ypos, expos, eypos );
	TGraphErrors* gSum = new TGraphErrors( nfpos, xfpos, yfpos, exfpos, eyfpos );
	gSum->SetMarkerColor( 2 );
	gSum->SetMarkerStyle( 20 );
	gSum->SetMarkerSize( 1.5 );
	if( bPlot )
	{
		gSum->Draw( "P" );
		gSum->Fit( "splitFit", "Q" );
	}
	else
	{
		gSum->Fit( "splitFit", "N0Q" );
	}
	
	cout << tel << " " << channel << " ";
	cout << splitFit->GetParameter( 0 ) << " " << splitFit->GetParError( 0 ) << " ";
	cout << splitFit->GetParameter( 1 ) << " " << splitFit->GetParError( 1 ) << " ";
	cout << splitFit->GetParameter( 2 ) << " " << splitFit->GetParError( 2 ) << " ";
	if( splitFit->GetNDF() > 0 )
	{
		cout << "Chi2/NoF " << splitFit->GetChisquare() << " / " << splitFit->GetNDF() << " = " << splitFit->GetChisquare() / splitFit->GetNDF() << endl;
	}
	else
	{
		cout << "Chi2/NoF " << splitFit->GetChisquare() << " / " << splitFit->GetNDF() << "0. = 0."  << endl;
	}
	
	if( !bPlot )
	{
		gDirectory->GetList()->Delete();
	}
	
	out.push_back( splitFit->GetParameter( 0 ) );
	out.push_back( splitFit->GetParError( 0 ) );
	out.push_back( splitFit->GetChisquare() );
	out.push_back( splitFit->GetNDF() );
	
	if( splitFit->GetNDF() > 0 )
		if( splitFit->GetChisquare() / splitFit->GetNDF() > 10. )
		{
			out[1] = 0.;
		}
		
	if( splitFit->GetNDF() == 0 )
	{
		out[1] = 0.;
	}
	
	
	return out;
	
}

Double_t splitFitFunc( Double_t* x, Double_t* par )
{

	Double_t result;
	
	if( x[0] < par[2] )
	{
		result = par[0] * par[1] * x[0] / 1.0;
	}
	else
	{
		result = par[1] * x[0] / 1.0;
	}
	
	return result;
	
}

int VHiLoTools::lowGainVector( TH2F* hscat, TH1F* hlin, double xpos[10], double ypos[10], double expos[10], double eypos[10] )
{

	bool bDebug = false;
	
	int npos = 0;
	int n_x;
	int n_y;
	double uav_x;
	double uav_y;
	double av_x;
	double av_y;
	double w_x;
	double w_y;
	double av_x2;
	double av_y2;
	
	vector<int> vLimit;
	vLimit.push_back( 0 );
	
	for( int i = 0; i < hlin->GetNbinsX() - 1; i++ )
	{
		if( TMath::Abs( hlin->GetBinContent( i ) ) < 0.001 && hlin->GetBinContent( i + 1 ) > 0. )
		{
			vLimit.push_back( i );
		}
	}
	
	vLimit.push_back( hlin->GetNbinsX() );
	
	for( int i = 0; i < ( int )vLimit.size() - 1; i++ )
	{
		n_x = 0;
		n_y = 0;
		uav_x = 0.;
		uav_y = 0.;
		av_x = 0.;
		av_y = 0.;
		av_x2 = 0.;
		av_y2 = 0.;
		w_x = 0.;
		w_y = 0.;
		
		for( int j = 0; j < hscat->GetNbinsX() - 1; j++ )
		{
			if( j > vLimit[i] && j <= vLimit[i + 1] )
			{
				for( int k = 0; k < hscat->GetNbinsY() - 1; k++ )
				{
				
					if( hscat->GetBinContent( j, k ) != 0 )
					{
						uav_x  += hscat->GetXaxis()->GetBinCenter( j );
						uav_y  += hscat->GetYaxis()->GetBinCenter( k );
						av_x  += hscat->GetXaxis()->GetBinCenter( j ) * hscat->GetBinContent( j, k );
						av_y  += hscat->GetYaxis()->GetBinCenter( k ) * hscat->GetBinContent( j, k );
						w_x   += hscat->GetBinContent( j, k );
						w_y   += hscat->GetBinContent( j, k );
						av_x2 += hscat->GetXaxis()->GetBinCenter( j ) * hscat->GetXaxis()->GetBinCenter( j ) * hscat->GetBinContent( j, k );
						av_y2 += hscat->GetYaxis()->GetBinCenter( k ) * hscat->GetYaxis()->GetBinCenter( k ) * hscat->GetBinContent( j, k );
						n_x++;
						n_y++;
					}
					
				}
			}
		}
		
		double rms_x = 0.;
		double rms_y = 0.;
		
		if( n_x > 0 )
		{
			rms_x = sqrt( av_x2 / w_x - ( av_x / w_x ) * ( av_x / w_x ) );
			rms_x /= sqrt( w_x );
			av_x /= w_x;
		}
		else
		{
			av_x = 0.;
			rms_x = 0.;
		}
		
		
		if( n_y > 0 )
		{
			rms_y = sqrt( av_y2 / w_y - ( av_y / w_y ) * ( av_y / w_y ) );
			rms_y /= sqrt( w_y );
			av_y /= w_y;
		}
		else
		{
			av_y = 0.;
			rms_y = 0.;
		}
		
		if( i > 0 )
		{
		
			if( bDebug )
			{
				cout << i << " ";
				cout << av_x << " ";
				cout << rms_x << " ";
				cout << av_y << " ";
				cout << rms_y << endl;
			}
			
			//if( av_x > 1. && rms_x > 0. && rms_y > 0. )
			if( av_x > 1. && rms_y > 0. )
			{
				xpos[npos] = av_x;
				ypos[npos] = av_y;
				expos[npos] = rms_x;
				eypos[npos] = rms_y;
				npos++;
			}
			
		}
		
	}
	
	return npos;
	
}

void VHiLoTools::mergeHiLoFiles( string infile1, string infile2, int SW, int TEL )
{

	TFile* f1 = new TFile( infile1.c_str(), "UPDATE" );
	if( !f1 )
	{
		cout << "No file called: " << infile1.c_str() << endl;
		exit( -1 );
	}
	TFile* f2 = new TFile( infile2.c_str(), "UPDATE" );
	if( !f2 )
		if( !f2 )
		{
			cout << "No file called: " << infile2.c_str() << endl;
			exit( -1 );
		}
		
	char treestring[500];
	
	sprintf( treestring, "lowGainMultiplier_%d_%d", TEL + 1, SW );
	TTree* t = ( TTree* )f1->Get( treestring );
	if( !t )
	{
		cout << "Error: no tree called: " << treestring << endl;
		exit( -1 );
	}
	
	f2->cd();
	
	if( t )
	{
		t->CloneTree()->Write();
	}
	else
	{
		exit( -1 );
	}
	
	f1->Close();
	f2->Close();
	
	exit( -1 );
	
}

void VHiLoTools::getMeanRMS( string ifile, int tel, int sumwindow, bool bPlot )
{

	TFile* infile = new TFile( ifile.c_str() );
	
	char treename[500];
	sprintf( treename, "lowGainMultiplier_%d_%d", tel, sumwindow );
	
	TTree* t = ( TTree* )infile->Get( treename );
	
	int chanID;
	double gainMult;
	double gainMultE;
	int LOST = 0;
	double chi2;
	int dof;
	
	t->SetBranchAddress( "chanID", &chanID );
	t->SetBranchAddress( "gainMult", &gainMult );
	t->SetBranchAddress( "gainMultE", &gainMultE );
	t->SetBranchAddress( "chi2", &chi2 );
	t->SetBranchAddress( "dof", &dof );
	
	TH1F* hgain = new TH1F( "hgain", "Gain Muliplier", 100, GMIN, GMAX );
	TH1F* hgainE = new TH1F( "hgainE", "Error in Multiplier", 100, GEMIN, GEMAX );
	TH2F* h2 = new TH2F( "h2", "Error vs Gain", 100, GMIN, GMAX, 100, GEMIN, GEMAX );
	TH2F* h3 = new TH2F( "h3", "h3", 100, GEMIN, GEMAX, 8, 0., 8. );
	TH1F* hchi2 = new TH1F( "hchi2", "#chi^{2}", 100, 0., 100. );
	TH1F* hchi2d = new TH1F( "hchi2d", "#chi2^{2} / DoF ", 100, 0., 10. );
	
	for( int i = 0; i < t->GetEntries(); i++ )
	{
	
		t->GetEntry( i );
		
		if( dof > 0 )
		{
			hgain->Fill( gainMult );
			hgainE->Fill( gainMultE );
			h2->Fill( gainMult, gainMultE );
			h3->Fill( gainMultE, dof );
			hchi2->Fill( chi2 );
			hchi2d->Fill( chi2 / dof );
		}
		
		if( ( gainMult < GMIN || gainMult > GMAX ) || ( gainMultE < GEMIN || gainMultE > GEMAX ) )
		{
			LOST++;
		}
		
	}
	
	if( bPlot )
	{
		TCanvas* c1 = new TCanvas( "c1", "c1", 0, 0, 500, 500 );
		hgain->Draw();
		c1->Update();
		TCanvas* c2 = new TCanvas( "c2", "c2", 510, 0, 500, 500 );
		hgainE->Draw();
		c2->Update();
		TCanvas* c3 = new TCanvas( "c3", "c3", 1020, 0, 500, 500 );
		h2->Draw( "zcol" );
		c3->Update();
		TCanvas* c4 = new TCanvas( "c4", "c4", 510, 510, 500, 500 );
		hchi2->Draw();
		c4->Update();
		TCanvas* c5 = new TCanvas( "c5", "c5", 1020, 510, 500, 500 );
		hchi2d->Draw();
		c5->Update();
		TCanvas* c6 = new TCanvas( "c6", "c6", 1020, 510, 500, 500 );
		h3->Draw( "zcol" );
		c6->Update();
	}
	else
	{
		exit( -1 );
	}
	
	
}

