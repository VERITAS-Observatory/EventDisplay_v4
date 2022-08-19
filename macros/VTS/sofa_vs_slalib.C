/*
 * test routines for event-by-event comparision between
 * sofa and slalib reconstructed events
 *
 *
 */

class dset
{
	public:
	
		string Name;
		
		double Energy;
		double Xderot;
		double Yderot;
		double Xoff;
		double Yoff;
		double MSCW;
		double MSCL;
		double XCore;
		double YCore;
		int IsGamma;
		
		dset( string iname = "" );
		~dset() {}
		void Print( int eventnumber, string iFirstPrint = "\t", bool bFullPrinting = false );
};

dset::dset( string iname )
{
	Name = iname;
	Energy = 0.;
	Xderot = 0.;
	Yderot = 0.;
	Xoff = 0.;
	Yoff = 0.;
	XCore = 0.;
	YCore = 0.;
	MSCW = 0.;
	MSCL = 0.;
	int IsGamma;
}

void dset::Print( int eventnumber, string iFirstPrint, bool bFullPrinting )
{
	cout << iFirstPrint;
	cout << " Event " << eventnumber;
	cout << ", energy " << Energy;
	cout << ", Xderot " << Xderot;
	cout << ", Yderot " << Yderot;
	cout << ", Wderot " << sqrt( Xderot * Xderot + Yderot * Yderot );
	if( bFullPrinting )
	{
		cout << endl;
		cout << "\t\t";
	}
	cout << ", Xoff " << Xoff;
	cout << ", Yoff " << Yoff;
	cout << ", Woff " << sqrt( Xoff * Xoff + Yoff * Yoff );
	if( bFullPrinting )
	{
		cout << ", MSCW " << MSCW;
		cout << ", MSCW " << MSCL;
		cout << ", XCore " << XCore;
		cout << ", YCore " << YCore;
	}
	cout << endl;
}


map< int, dset* > read_tree( int runnumber, string iFile, string iname = "" )
{
	map< int, dset* > d;
	
	TFile* f = new TFile( iFile.c_str() );
	if( f->IsZombie() )
	{
		return d;
	}
	char hname[200];
	sprintf( hname, "run_%d/stereo", runnumber );
	f->cd( hname );
	TTree* t = ( TTree* )gDirectory->Get( "DL3EventTree" );
	if( !t )
	{
		return d;
	}
	
	int en;
	double e;
	double xd;
	double yd;
	double x;
	double y;
	double xc = 0.;
	double yc = 0.;
	double mscw = 0.;
	double mscl = 0.;
	int gh = 1;
	t->SetBranchAddress( "eventNumber", &en );
	t->SetBranchAddress( "Energy", &e );
	t->SetBranchAddress( "Xderot", &xd );
	t->SetBranchAddress( "Yderot", &yd );
	t->SetBranchAddress( "Xoff", &x );
	t->SetBranchAddress( "Yoff", &y );
	if( t->GetBranchStatus( "XCore" ) )
	{
		t->SetBranchAddress( "XCore", &xc );
	}
	if( t->GetBranchStatus( "YCore" ) )
	{
		t->SetBranchAddress( "YCore", &yc );
	}
	if( t->GetBranchStatus( "MSCW" ) )
	{
		t->SetBranchAddress( "MSCW", &mscw );
	}
	if( t->GetBranchStatus( "MSCL" ) )
	{
		t->SetBranchAddress( "MSCL", &mscl );
	}
	if( t->GetBranchStatus( "IsGamma" ) )
	{
		t->SetBranchAddress( "IsGamma", &gh );
	}
	
	for( int i = 0; i < t->GetEntries(); i++ )
	{
		t->GetEntry( i );
		
		dset* b = new dset( iname );
		b->Energy = e;
		b->Xderot = xd;
		b->Yderot = yd;
		b->Xoff = x;
		b->Yoff = y;
		b->XCore = xc;
		b->YCore = yc;
		b->MSCW = mscw;
		b->MSCL = mscl;
		b->IsGamma = gh;
		
		d[en] = b;
	}
	return d;
}

/*
 * print events which are in map1 but not in map2
 *
 * bPrintIsGamma: check IsGamma in map1
 *
*/

unsigned int print_map_difference(
	map< int, dset* > map1,
	map< int, dset* > map2,
	bool bPrint,
	bool bPrintIsGamma )
{
	unsigned int z = 0;
	for( map< int, dset* >::iterator it = map1.begin(); it != map1.end(); ++it )
	{
		if( bPrintIsGamma && it->second->IsGamma == 0 )
		{
			continue;
		}
		
		if( map2.find( it->first ) == map2.end() )
		{
			if( bPrint )
			{
				it->second->Print( it->first, "\t event missing: " );
			}
			z++;
		}
		else if( bPrintIsGamma && map2[it->first]->IsGamma == 0 )
		{
			if( bPrint )
			{
				it->second->Print( it->first, "\t " + it->second->Name, true );
				map2[it->first]->Print( it->first, "\t " + map2[it->first]->Name, true );
			}
			z++;
		}
	}
	return z;
}

void print_event_differences(
	map< int, dset* > map1,
	map< int, dset* > map2,
	bool b_derot )
{
	float d_diff_mean = 0.;
	float d_diff_mean_N = 0.;
	float d_diff_max = -1.;
	float d_diff_min = 1.e5;
	float d_diff = 0.;
	
	for( map< int, dset* >::iterator it = map1.begin(); it != map1.end(); ++it )
	{
		if( map2.find( it->first ) != map2.end() )
		{
			if( b_derot )
			{
				d_diff = sqrt(
							 ( it->second->Xderot - map2[it->first]->Xderot ) * ( it->second->Xderot - map2[it->first]->Xderot )
							 + ( it->second->Yderot - map2[it->first]->Yderot ) * ( it->second->Yderot - map2[it->first]->Yderot ) );
			}
			else
			{
				d_diff = sqrt(
							 ( it->second->Xoff - map2[it->first]->Xoff ) * ( it->second->Xoff - map2[it->first]->Xoff )
							 + ( it->second->Yoff - map2[it->first]->Yoff ) * ( it->second->Yoff - map2[it->first]->Yoff ) );
			}
			d_diff_mean += d_diff;
			d_diff_mean_N++;
			if( d_diff > d_diff_max )
			{
				d_diff_max = d_diff;
			}
			if( d_diff < d_diff_min )
			{
				d_diff_min = d_diff;
			}
		}
	}
	if( b_derot )
	{
		cout << "X/Yderot diff: mean ";
	}
	else
	{
		cout << "X/Yoff diff: mean ";
	}
	cout << d_diff_mean / d_diff_mean_N;
	cout << ", max " << d_diff_max;
	cout << ", min " << d_diff_min << endl;
}


void sofa_vs_slalib(
	int runnumber = 64080, bool bPrintEvents = false, bool bPrintIsGamma = false,
	string sofafile = "$VERITAS_USER_DATA_DIR/analysis/Results/v490/TS/anasum_soft/64080.anasum.root",
	string slalibfile = "$VERITAS_USER_DATA_DIR/analysis/Results/v490/TSsla/using_TS_Tables/anasum_soft/64080.anasum.root" )
{
	map< int, dset* > sofa = read_tree( runnumber, sofafile, "sofa" );
	map< int, dset* > slal = read_tree( runnumber, slalibfile, "slalib" );
	
	cout << "sofa entries: " << sofa.size() << endl;
	cout << "slalib entries: " << slal.size() << endl;
	
	/////////////////
	cout << "Events which are in sofa but not in slalib set (or not classified as gamma): " << endl;
	int sofa_not_in_slalib = print_map_difference( sofa, slal, bPrintEvents, bPrintIsGamma );
	cout << "found " << sofa_not_in_slalib << " missing events in slalib (out of ";
	cout << sofa.size() << ", " << slal.size() << ")" << endl;
	cout << endl;
	
	cout << "Events which are in slalib but not in sofa set (or not classified as gamma): " << endl;
	int slalib_not_in_sofa = print_map_difference( slal, sofa, bPrintEvents, bPrintIsGamma );
	cout << "found " << slalib_not_in_sofa << " missing events in sofa (out of ";
	cout << slal.size() << ", " << sofa.size() << ")" << endl;
	cout << endl;
	
	print_event_differences( sofa, slal, true );
	print_event_differences( sofa, slal, false );
}
