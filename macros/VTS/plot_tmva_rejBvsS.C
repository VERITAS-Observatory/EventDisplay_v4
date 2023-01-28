/*
 * Plot signal vs background effience for two files
 * from TMVA gamma/hadron separation output
 *
 * input are the complete BDT output files
 *
 * usage e.g, 
 * root -l -q -b 'plot_tmva_rejBvsS.C("V6_2012_2013a_ATM61/NTel2-Moderate/complete_BDTroot/", "V6_2021_2022w_ATM61/NTel2-Moderate/complete_BDTroot")'
 *
 *
*/

TH1D* get_histogram( string iFile, int iColor )
{
    TFile *f = new TFile( iFile.c_str() );
    if( f->IsZombie() ) return 0;
    f->cd("Method_BDT/BDT_0");
    TH1D *MVA_BDT_0_rejBvsS = (TH1D*)gDirectory->Get( "MVA_BDT_0_rejBvsS" );
    if( !MVA_BDT_0_rejBvsS) return 0;

    MVA_BDT_0_rejBvsS->SetTitle( "" );
    MVA_BDT_0_rejBvsS->SetStats( 0 );
    MVA_BDT_0_rejBvsS->SetLineWidth(2);
    MVA_BDT_0_rejBvsS->SetLineColor(iColor);
    MVA_BDT_0_rejBvsS->SetAxisRange( 0.45, 1. );

    return MVA_BDT_0_rejBvsS;
}

/*
 * expect files names to be e.g. BDT_2_1.root 
 *
 */
void plot_tmva_rejBvsS( 
        string iFileDir1,
        string iFileDir2,
        unsigned int ebin_min, unsigned int ebin_max,
        unsigned int zebin_min, unsigned int zebin_max )
{
    TCanvas *c = new TCanvas( "c", "", 100, 10, 400, 400 );
    c->SetLeftMargin( 0.12 );
    c->Draw();

    for( unsigned int e = ebin_min; e <= ebin_max; e++ )
    {
        for( unsigned int z = zebin_min; z <= zebin_max; z++ )
        {
            ostringstream iFile1;
            iFile1 << iFileDir1 << "/BDT_" << e << "_" << z << ".root";
            ostringstream iFile2;
            iFile2 << iFileDir2 << "/BDT_" << e << "_" << z << ".root";

            TH1D *h1 = get_histogram(iFile1.str(), 1 );
            TH1D *h2 = get_histogram(iFile2.str(), 2 );

            if( h1 )
            {
                h1->SetMinimum( 0. );
                h1->SetMaximum( 1.05 );
                h1->Draw();
            }
            if( h2 ) h2->Draw( "same" );

            ostringstream printName;
            printName << "BDT-" << e << "-" << z << ".pdf";
            c->Print( printName.str().c_str() );
        }
    }
 }
