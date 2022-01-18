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

TGraph* get_effAreaGraph( TFile *f,
                          string tree_name,
                          string var_name,
                          int iEntry )
{
    if( !f ) return 0;
    TTree *t = (TTree*)f->Get( tree_name.c_str() );
    if( !t ) return 0;

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
        UShort_t nbins;
        Float_t e0[1000];
        Float_t eff[1000];
        t->SetBranchAddress( "nbins", &nbins );
        t->SetBranchAddress( "e0", e0 );
        t->SetBranchAddress( var_name.c_str(), eff );
        t->GetEntry( iEntry );
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

     TH2F* h1 = plot_h2f( f, iHistogramName, iEntry );

     TH2F* h2 = plot_array( f, iHistogramName, iEntry );

     if( h1 && h2 )
     {
         TCanvas *c = new TCanvas( ("comp"+iHistogramName).c_str(), "", 1380, 10, 600, 600 );
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
         TCanvas *d = new TCanvas( "c_effNoTh2", "", 1380, 600, 600, 600 );
         d->Draw();
         d->cd();
         h1_eff->Draw( "ap" );
         h2_eff->SetMarkerColor(2);
         h2_eff->Draw( "p" );
    }
}
