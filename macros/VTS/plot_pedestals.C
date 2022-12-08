/*
 * compare pedestal mean and median values vs run number
 * compare pedestal var and 68% containment values vs run number
 *
 * reads data from pedestal files
 *
 */

#include "sstream"
#include "vector"

vector< unsigned int > get_run_list( string iRunlist )
{
	vector< unsigned int > i_run_list;
	ifstream is;
	is.open( iRunlist.c_str(), ifstream::in );
	if( !is )
	{
		cout << "Error reading run list from " << iRunlist << endl;
		return i_run_list;
	}
	string is_line;
	while( getline( is, is_line ) )
	{
		if( is_line.size() > 0 )
		{
			i_run_list.push_back( atoi( is_line.c_str() ) );
		}
	}
	is.close();
	return i_run_list;
}


TCanvas* plot_vs_run_number( TGraph* iG1, TGraph* iG2, string iTitle, string iCanvasName, double imin, double imax )
{
	TCanvas* c = new TCanvas( iCanvasName.c_str(), "", 100, 100, 800, 600 );
	c->Draw();
	
	iG1->SetTitle( "" );
	iG2->SetTitle( "" );
	iG1->SetMarkerStyle( 7 );
	iG2->SetMarkerStyle( 7 );
	iG2->SetMarkerColor( 2 );
	
	iG1->SetMinimum( imin );
	iG1->SetMaximum( imax );
	iG1->Draw( "ap" );
	iG1->GetHistogram()->SetXTitle( "run number" );
	iG1->GetHistogram()->SetYTitle( iTitle.c_str() );
	iG2->Draw( "p" );
	
	return c;
}

/*
 * divide runs into n_run_bins equally large bins
 *
 */
vector< int > define_epochs( int n_run_bins, bool i_min, int run_min, int run_max )
{
	vector< int > i_run;
	for( int i = 0; i < n_run_bins; i++ )
	{
		if( i_min )
		{
			i_run.push_back( run_min + i * ( run_max - run_min ) / ( float )n_run_bins );
		}
		else
		{
			i_run.push_back( run_min + ( i + 1 ) * ( run_max - run_min ) / ( float )n_run_bins );
		}
	}
	return i_run;
}


/*
 * plot pedestal mean and median vs run number
 * plot pedestal var and 68 containment vs run number
 *
 * expects a simple run list
 */
void plot_pedestals( int iTelID = 1, int iChannelID = 1, int iSummationWindowIndex = 5, string iRunlist = "", string iPedestalDirectory = "" )
{
	vector< unsigned int > run_list = get_run_list( iRunlist );
	
	vector< int > run_epoch_min = define_epochs( 4, true, 63373, 102655 );
	vector< int > run_epoch_max = define_epochs( 4, false, 63373, 102655 );
	
	char hname[200];
	char htitle[200];
	vector< TH1D* > ped_ratio_per_epoch;
	vector< TH1D* > pedvar_ratio_per_epoch;
	for( unsigned int i = 0; i < run_epoch_min.size(); i++ )
	{
		cout << run_epoch_min[i] << "\t" << run_epoch_max[i] << endl;
		
		sprintf( hname, "hped_ratio_per_epoch_%d", i );
		sprintf( htitle, "ped (Tel %d), runs %d - %d", iTelID, run_epoch_min[i], run_epoch_max[i] );
		ped_ratio_per_epoch.push_back( new TH1D( hname, htitle, 100, 0.5, 1.5 ) );
		ped_ratio_per_epoch.back()->SetLineWidth( 2 );
		ped_ratio_per_epoch.back()->SetXTitle( "ped median/mean" );
		sprintf( hname, "hpedvar_ratio_per_epoch_%d", i );
		sprintf( htitle, "pedvar (Tel %d), runs %d - %d", iTelID, run_epoch_min[i], run_epoch_max[i] );
		pedvar_ratio_per_epoch.push_back( new TH1D( hname, htitle, 100, 0.5, 1.5 ) );
		pedvar_ratio_per_epoch.back()->SetLineColor( 2 );
		pedvar_ratio_per_epoch.back()->SetLineWidth( 2 );
		pedvar_ratio_per_epoch.back()->SetXTitle( "pedvars variance/(68p containment)" );
	}
	
	TGraph* fG_ped_mean = new TGraph( 1 );
	TGraph* fG_ped_median = new TGraph( 1 );
	TGraph* fG_ped_vars = new TGraph( 1 );
	TGraph* fG_ped_vars68 = new TGraph( 1 );
	TGraph* fG_ped_ratio = new TGraph( 1 );
	TGraph* fG_ped_vars_ratio = new TGraph( 1 );
	int ip = 0;
	int ip_r = 0;
	
	for( unsigned int i = 0; i < run_list.size(); i++ )
	{
		cout << "Run " << i << "\t" << run_list[i] << endl;
		
		TFile* i_f = new TFile( iPedestalDirectory + "/Tel_" + iTelID + "/" + run_list[i] + ".ped.root" );
		if( i_f->IsZombie() )
		{
			cout << "Error reading " << i_f->GetName() << endl;
			continue;
		}
		cout << "Reading " << i_f->GetName() << endl;
		
		stringstream i_t_name;
		i_t_name << "tPeds_" << iTelID;
		TTree* i_t = ( TTree* )i_f->Get( i_t_name.str().c_str() );
		if( !i_t )
		{
			cout << "Error reading tree " << i_t_name.str() << endl;
			continue;
		}
		
		unsigned int channel;
		float pedmean;
		float pedmedian;
		float pedvars[100];
		float pedvars68[100];
		i_t->SetBranchAddress( "channel", &channel );
		i_t->SetBranchAddress( "pedmean", &pedmean );
		i_t->SetBranchAddress( "pedmedian", &pedmedian );
		i_t->SetBranchAddress( "pedvars", pedvars );
		i_t->SetBranchAddress( "pedvars68", pedvars68 );
		
		for( int e = 0; e < i_t->GetEntries(); e++ )
		{
			i_t->GetEntry( e );
			if( iChannelID == channel )
			{
				fG_ped_mean->SetPoint( ip, run_list[i], pedmean );
				fG_ped_median->SetPoint( ip, run_list[i], pedmedian );
				fG_ped_vars->SetPoint( ip, run_list[i], pedvars[iSummationWindowIndex] );
				fG_ped_vars68->SetPoint( ip, run_list[i], pedvars68[iSummationWindowIndex] );
				if( pedmean > 0. && pedvars[iSummationWindowIndex] > 0. )
				{
					fG_ped_vars_ratio->SetPoint( ip_r, run_list[i], pedvars68[iSummationWindowIndex] / pedvars[iSummationWindowIndex] );
					fG_ped_ratio->SetPoint( ip_r, run_list[i], pedmedian / pedmean );
					ip_r++;
				}
				ip++;
			}
			if( pedmean > 0. && pedvars[iSummationWindowIndex] > 0. )
			{
				for( unsigned int r = 0; r < run_epoch_min.size(); r++ )
				{
					if( run_list[i] >= run_epoch_min[r] && run_list[i] < run_epoch_max[r] )
					{
						ped_ratio_per_epoch[r]->Fill( pedmedian / pedmean );
						pedvar_ratio_per_epoch[r]->Fill( pedvars68[iSummationWindowIndex] / pedvars[iSummationWindowIndex] );
					}
				}
			}
		}
		i_f->Close();
	}
	TCanvas* c = 0;
	sprintf( hname, "c_ped_%d_%d.pdf", iTelID, iChannelID );
	c = plot_vs_run_number( fG_ped_mean, fG_ped_median, "ped mean, median", "ped_med", 12., 18. );
	c->Print( hname );
	sprintf( hname, "c_ped_vars_%d_%d.pdf", iTelID, iChannelID );
	c = plot_vs_run_number( fG_ped_vars, fG_ped_vars68, "ped vars, vars68", "ped_vars", 1., 17. );
	c->Print( hname );
	sprintf( hname, "c_ped_ratio_%d_%d.pdf", iTelID, iChannelID );
	c = plot_vs_run_number( fG_ped_ratio, fG_ped_vars_ratio, "ratio med/mean", "ped_ratio", 0., 1.4 );
	c->Print( hname );
	
	TCanvas* cAvPed = new TCanvas( "cAvPed", "peds ratios", 10, 10, 800, 800 );
	cAvPed->Divide( 2, 2 );
	TCanvas* cAvPedvars = new TCanvas( "cAvPedvars", "pedvars ratios", 10, 10, 800, 800 );
	cAvPedvars->Divide( 2, 2 );
	for( unsigned int r = 0; r < run_epoch_min.size(); r++ )
	{
		cAvPed->cd( r + 1 );
		ped_ratio_per_epoch[r]->Draw();
		cAvPedvars->cd( r + 1 );
		pedvar_ratio_per_epoch[r]->Draw();
	}
	sprintf( hname, "ped_ratio_tel%d.pdf", iTelID );
	cAvPed->Print( hname );
	sprintf( hname, "pedvar_ratio_tel%d.pdf", iTelID );
	cAvPedvars->Print( hname );
	
}
