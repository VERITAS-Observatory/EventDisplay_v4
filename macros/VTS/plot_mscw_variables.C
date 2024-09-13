/*
 *  Compare distributions of variables from two mscw files
 *
*/

/*
 * print RMS or containment radii to screen
 *
 */
void print_statistics_output( TH1F* h, string print_out )
{
    if(!h || print_out.size() == 0 )
    {
        return;
    }
    if( print_out == "RMS" )
    {
        cout << "\t " << print_out << ": " << h->GetRMS();
        cout << " (" << h->GetEntries() << " entries)";
        cout <<  endl;
    }
    else if( print_out == "68p" )
    {
        int nQuantiles = 1;
        double value[1];
        double prob[1] = {0.68};
        h->GetQuantiles( nQuantiles, value, prob );
        cout << "\t 68% value: " << value[0];
        cout << " (" << h->GetEntries() << " entries)";
        cout << endl;
    }
    else if( print_out == "VALUE" )
    {
        cout << "\t Value at 10 (30) TeV: ";
        cout << h->GetBinContent( h->FindBin( log10( 10 ) ) );
        cout << " (" << h->GetBinContent( h->FindBin( log10( 30. ) ) ) << ") ";
        cout << " (" << h->GetEntries() << " entries)";
        cout << endl;
    }
}


/*
 * plot_type:
 * - short_mc: expecting MC input; six plots only
 * - (anything else): plot large number of variables
 * - string includes 'geo': use intersection method for angular reconstruction
*/
void plot_mscw_variables( string iFile1, string iFile2, float iW1 = 1., float iW2 = 1., string iAddCut = "ErecS>0.", string plot_type = "short_mc", string i_print_file = "" )
{
    TFile* f1 = new TFile( iFile1.c_str() );
    if( f1->IsZombie() )
    {
        return;
    }
    TTree* T1 = ( TTree* )f1->Get( "data" );
    if(!T1 )
    {
        cout << "T1 tree not found" << endl;
        return;
    }
    T1->SetLineWidth( 2 );
    bool is_MC = false;
    if( T1->GetBranchStatus( "MCxoff" ) )
    {
        is_MC = true;
        cout << "MC type files detected" << endl;
    }

    TFile* f2 = new TFile( iFile2.c_str() );
    TTree *T2 = 0;
    if( f2->IsZombie() )
    {
        return;
    }
    else
    {
        T2 = ( TTree* )f2->Get( "data" );
        if(!T2 )
        {
            cout << "T2 tree not found" << endl;
            T2 = 0;
        }
        else
        {
            T2->SetLineColor( 2 );
            T2->SetLineWidth( 2 );
        }
    }

    vector< string > V;
    vector< float > Vmax;
    vector< float > Vmin;
    vector< string > Vprintout;

    V.push_back( "MSCW" );
    Vmin.push_back(-2. );
    Vmax.push_back( 3. );
    Vprintout.push_back( "RMS" );
    V.push_back( "MSCL" );
    Vmin.push_back(-2. );
    Vmax.push_back( 3. );
    Vprintout.push_back( "RMS" );
    V.push_back( "NImages" );
    Vmin.push_back( 0. );
    Vmax.push_back( 70. );
    Vprintout.push_back( "" );
    if( plot_type.find( "short_mc" ) == string::npos )
    {
        V.push_back( "EmissionHeight" );
        Vmin.push_back( 0. );
        Vmax.push_back( 40. );
        Vprintout.push_back( "" );
        V.push_back( "log10(EChi2S)" );
        Vmin.push_back(-2. );
        Vmax.push_back( 4. );
        Vprintout.push_back( "" );
        V.push_back( "log10(EmissionHeightChi2)" );
        Vmin.push_back(-11. );
        Vmax.push_back( 3. );
        Vprintout.push_back( "" );
        V.push_back( "log10(SizeSecondMax)" );
        Vmin.push_back( 2. );
        Vmax.push_back( 8. );
        Vprintout.push_back( "" );
        V.push_back( "log10(DispDiff)" );
        Vmin.push_back(-10. );
        Vmax.push_back( 3. );
        Vprintout.push_back( "" );
        V.push_back( "dES" );
        Vmin.push_back( 0. );
        Vmax.push_back( 3. );
        Vprintout.push_back( "" );
        V.push_back( "sqrt( Xcore*Xcore+Ycore*Ycore)" );
        Vmin.push_back( 0. );
        Vmax.push_back( 2500. );
        Vprintout.push_back( "" );
    }
    if( is_MC )
    {
        V.push_back( "log10(MCe0)" );
        Vmin.push_back(-2. );
        Vmax.push_back( log10( 300. ) );
        Vprintout.push_back( "VALUE" );
    }
    else
    {
        V.push_back( "log10(ErecS)" );
        Vmin.push_back(-2. );
        Vmax.push_back( log10( 300. ) );
        Vprintout.push_back( "VALUE" );
    }
    if( plot_type.find( "short_mc" ) != string::npos )
    {
        V.push_back( "(ErecS-MCe0)/MCe0" );
        Vmin.push_back(-1. );
        Vmax.push_back( 1. );
        Vprintout.push_back( "RMS" );
    }

    if( is_MC )
    {
        if( plot_type.find( "geo" ) != string::npos )
        {
            V.push_back( "sqrt( (Xoff_intersect-MCxoff)*(Xoff_intersect-MCxoff)+(Yoff_intersect-MCyoff)*(Yoff_intersect-MCyoff))" );
        }
        else
        {
            V.push_back( "sqrt( (Xoff-MCxoff)*(Xoff-MCxoff)+(Yoff-MCyoff)*(Yoff-MCyoff))" );
        }
        Vprintout.push_back( "68p" );
    }
    else
    {
        V.push_back( "sqrt( Xoff*Xoff+Yoff*Yoff)" );
        Vprintout.push_back( "68p" );
    }
    Vmin.push_back( 0. );
    Vmax.push_back( 1.5 );


    TCanvas* c = new TCanvas( "c", "mscw variables", 0, 100, 1600, 1100 );
    if( is_MC )
    {
        c->Divide( 3, 2 );
    }
    else
    {
        c->Divide( 4, 3 );
    }

    for( unsigned int i = 0; i < V.size(); i++ )
    {
        TPad* p = ( TPad* )c->cd( i + 1 );
        if( p )
        {
            p->SetLogy( 1 );
            p->SetGridx( 0 );
            p->SetGridy( 0 );
        }

        char Vcut[400];
        sprintf( Vcut, "MSCW>-2.&&MSCW<3.&&dES>0.&&%s>%f&&%s<%f&&%s", V[i].c_str(), Vmin[i], V[i].c_str(), Vmax[i], iAddCut.c_str() );
        cout << "Canvas " << i + 1 << ", variable " << V[i] << " cut: " << Vcut << endl;

        f1->cd();
        T1->Draw( V[i].c_str(), Vcut );

        print_statistics_output(( TH1F* )gPad->GetPrimitive( "htemp" ), Vprintout[i] );

        if( T2 )
        {
            TList* primitives = gPad->GetListOfPrimitives();
            f2->cd();
            T2->Draw( V[i].c_str(), Vcut, "sames" );
            print_statistics_output(( TH1F* )primitives->At( primitives->GetSize() - 1 ), Vprintout[i] );
        }
    }

    if( i_print_file.size() > 0 )
    {
        c->Print( i_print_file.c_str() );
    }
}
