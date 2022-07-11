/*
 * compare IRFs written as histograms or arrays
 *
 * used to test replacement of TH2F by arrays
 *
 * requires to run combineEffectiveAreas with option DL3test
*/

TH2F* plot_h2f( TFile *f,
               string iHistogramName,
               int iEntry )
{
    if( !f ) return 0;
    TTree *t = (TTree*)f->Get( "fEffArea" );
    if( !t ) return 0;
    TH2F *h2f = 0;
    char hname[200];
    sprintf( hname, "%s", iHistogramName.c_str() );
    t->SetBranchAddress( hname, &h2f );
    t->GetEntry( iEntry );

    if( h2f )
    {
        TCanvas *ch2f = new TCanvas( ("ch2f"+iHistogramName).c_str(), "", 10, 10, 600, 600 );
        ch2f->Draw();
        h2f->SetTitle( (iHistogramName + " (H2F)").c_str() );
        h2f->Draw( "colz" );
    }
    return h2f;
}

TH2F* plot_array( TFile *f,
                 string iHistogramName,
                 int iEntry )
{
    if( !f ) return 0;
    TTree *t = (TTree*)f->Get( "fEffAreaH2F" );
    if( !t ) return 0;

    int nbinsx = 0;
    float x_max = 0.;
    float x_min = 0.;
    int nbinsy = 0;
    float y_max = 0.;
    float y_min = 0.;
    int binsxy = 0;
    float value[10000];

    char hname[200];
    sprintf( hname, "%s_binsx", iHistogramName.c_str() );
    t->SetBranchAddress( hname, &nbinsx );
    sprintf( hname, "%s_minx", iHistogramName.c_str() );
    t->SetBranchAddress( hname, &x_min );
    sprintf( hname, "%s_maxx", iHistogramName.c_str() );
    t->SetBranchAddress( hname, &x_max );
    sprintf( hname, "%s_binsy", iHistogramName.c_str() );
    t->SetBranchAddress( hname, &nbinsy );
    sprintf( hname, "%s_miny", iHistogramName.c_str() );
    t->SetBranchAddress( hname, &y_min );
    sprintf( hname, "%s_maxy", iHistogramName.c_str() );
    t->SetBranchAddress( hname, &y_max );

    sprintf( hname, "%s_binsxy", iHistogramName.c_str() );
    t->SetBranchAddress( hname, &binsxy );
    sprintf( hname, "%s_value", iHistogramName.c_str() );
    t->SetBranchAddress( hname, &value );

    t->GetEntry( iEntry );

    cout << "XXX " << nbinsx << "\t" << x_min << "\t" << x_max << endl;
    cout << "YYY " << nbinsy << "\t" << y_min << "\t" << y_max << endl;

    TH2F *hA = new TH2F( ("array"+iHistogramName).c_str(), "",
                         nbinsx, x_min, x_max,
                         nbinsy, y_min, y_max );
    for( unsigned int i = 0; i < binsxy; i++ )
    {
        int nx = i % nbinsx;
        int ny = (i - nx) / nbinsx;
        hA->SetBinContent( nx+1, ny+1, value[i] );
    }

    if( hA )
    {
        TCanvas *arrayf = new TCanvas( ("arrayf"+iHistogramName).c_str(), "", 710, 10, 600, 600 );
        arrayf->Draw();
        hA->SetTitle( (iHistogramName + " (from array)").c_str() );
        hA->DrawCopy( "colz" );
    }

    return hA;
}

void printEntry( string iname, int iEntry, double ze, int az, double Woff, double pedvar, double index )
{
    cout << iname << ": ";
    cout << " entry: "<< iEntry;
    cout << " ze: " << ze;
    cout << " az: " << az;
    cout << " Woff: " << Woff;
    cout << " pedvar: " << pedvar;
    if( index > 0. ) cout << " index: " << index;
    cout << endl;
}

TGraph* get_effAreaGraph( TFile *f,
                          string tree_name,
                          string var_name,
                          int iEntry )
{
    if( !f ) return 0;
    TTree *t = (TTree*)f->Get( tree_name.c_str() );
    if( !t ) return 0;

    int fH2F_treecounter_offset = 20;
    int count_max_az_bins = 17;

    TGraph *g = new TGraph( 1 );
    if( tree_name == "fEffArea" )
    {
        Int_t nbins;
        Double_t e0[1000];
        Double_t eff[1000];
        t->SetBranchAddress( "nbins", &nbins );
        t->SetBranchAddress( "e0", e0 );
        t->SetBranchAddress( var_name.c_str(), eff );
        t->GetEntry( iEntry );
        g->SetMarkerStyle( 20 );
        for( int i = 0; i < nbins; i++ )
        {
            g->SetPoint( i, e0[i], eff[i] );
        }
    }
    else
    {
        int iEntryH2F = count_max_az_bins * (iEntry / (fH2F_treecounter_offset*count_max_az_bins))
            + iEntry % count_max_az_bins;
        cout << "Reading entryH2F " << iEntryH2F << endl;
        UShort_t nbins;
        Float_t e0[1000];
        Float_t eff[1000];
        t->SetBranchAddress( "nbins", &nbins );
        t->SetBranchAddress( "e0", e0 );
        t->SetBranchAddress( var_name.c_str(), eff );
        t->GetEntry( iEntryH2F );
        g->SetMarkerStyle( 24 );
        for( int i = 0; i < nbins; i++ )
        {
            g->SetPoint( i, e0[i], eff[i] );
        }
    }
    return g;
}

void test_reducedIRFFiles( string iIRFFile,
                           string iHistogramName = "hEsysMCRelative2D",
                           int iEntry = 0 )
{
     TFile *f = new TFile( iIRFFile.c_str() );
     if( f->IsZombie() )
     {
        return 0;
     }
    int fH2F_treecounter_offset = 20;
    int count_max_az_bins = 17;

     TH2F* h1 = plot_h2f( f, iHistogramName, iEntry );

     TH2F* h2 = plot_array( f, iHistogramName, 
             count_max_az_bins * (iEntry / (fH2F_treecounter_offset*count_max_az_bins)) + iEntry % count_max_az_bins );

     if( h1 && h2 )
     {
         TCanvas *c = new TCanvas( ("comp"+iHistogramName).c_str(), "", 80, 10, 600, 600 );
         c->Draw();
         TH2F *h3 = (TH2F*)h1->Clone( ("hc"+iHistogramName).c_str() );
         h3->Divide( h2 );
         h3->SetMinimum( 0.5 );
         h3->SetMaximum( 2. );
         h3->SetStats( 0 );
         h3->SetTitle( (iHistogramName + " (ratio)" ).c_str() );
         h3->DrawCopy( "colz" );
     }
}

void test_effectiveAreas( string iIRFFile,
                          string iVariable = "effNoTh2",
                          int iEntry = 0 )
{
     TFile *f = new TFile( iIRFFile.c_str() );
     if( f->IsZombie() )
     {
        return 0;
     }

     TGraph *h1_eff = get_effAreaGraph( f, "fEffArea", iVariable, iEntry );
     TGraph *h2_eff = get_effAreaGraph( f, "fEffAreaH2F", iVariable, iEntry );
     if( h1_eff && h2_eff )
     {
         TCanvas *d = new TCanvas( "c_effNoTh2", "", 80, 600, 600, 600 );
         d->Draw();
         d->cd();
         h1_eff->Draw( "ap" );
         h2_eff->SetMarkerColor(2);
         h2_eff->Draw( "p" );
    }
}

void sync_effectiveAreaTrees( string iIRFFile, int zmax = 100 )
{

    TFile *f = new TFile( iIRFFile.c_str() );
    if( f->IsZombie() )
    {
        return 0;
    }
    TTree *fEffArea = (TTree*)f->Get( "fEffArea" );
    if( !fEffArea ) return;
    Double_t ze = 0.;
    Int_t az = 0;
    Double_t Woff = 0.;
    Double_t pedvar = 0.;
    Double_t index = 0.;
    fEffArea->SetBranchAddress( "ze", &ze );
    fEffArea->SetBranchAddress( "az", &az );
    fEffArea->SetBranchAddress( "Woff", &Woff );
    fEffArea->SetBranchAddress( "pedvar", &pedvar );
    fEffArea->SetBranchAddress( "index", &index );

    TTree *fEffAreaH2F = (TTree*)f->Get( "fEffAreaH2F" );
    if( !fEffAreaH2F ) return;
    Float_t ze2 = 0.;
    UShort_t az2 = 0;
    Float_t Woff2 = 0.;
    Float_t pedvar2 = 0.;
    fEffAreaH2F->SetBranchAddress( "ze", &ze2 );
    fEffAreaH2F->SetBranchAddress( "az", &az2 );
    fEffAreaH2F->SetBranchAddress( "Woff", &Woff2 );
    fEffAreaH2F->SetBranchAddress( "pedvar", &pedvar2 );

    int fH2F_treecounter_offset = fEffArea->GetEntries() / fEffAreaH2F->GetEntries();
    cout << "Tree counter offset: " << fH2F_treecounter_offset << endl;
    int count_max_az_bins = 0;
    for( unsigned int i = 0; i < fEffArea->GetEntries(); i++ )
    {
        fEffArea->GetEntry( i );
        if( az > count_max_az_bins ) count_max_az_bins = az;
        if( az < count_max_az_bins ) break;
    }
    count_max_az_bins++;
    cout << "Maximum number of az bin: " << count_max_az_bins << endl;

    int z = 0;
    for( unsigned int i = 0; i < fEffArea->GetEntries(); i++ )
    {
        fEffArea->GetEntry( i );
        printEntry( "EffArea", i, ze, az, Woff, pedvar, index );

        for( unsigned int j = 0; j < fEffAreaH2F->GetEntries(); j++ )
        {
            fEffAreaH2F->GetEntry( j );

            if( TMath::Abs( ze - ze2 ) > 1.e-2 ) continue;
            if( TMath::Abs( Woff - Woff2 ) > 1.e-2 ) continue;
            if( TMath::Abs( pedvar - pedvar2 ) > 1.e-2 ) continue;
            if( az != az2 ) continue;
            printEntry( "EffAreaH2", j, ze2, az2, Woff2, pedvar2, 0.);
            cout << "H2 entry should be: ";
            cout <<  count_max_az_bins * (i / (fH2F_treecounter_offset*count_max_az_bins)) + i % count_max_az_bins << endl;
            cout << "===========================================" << endl;
        }
        z++;
        if( z > zmax ) break;
     }
}
