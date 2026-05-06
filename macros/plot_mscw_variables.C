/*
 *  Compare distributions of variables from two mscw files
 *
*/

/*
 * print RMS or containment radii to screen
 *
 */

#include <string>

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
        int nQuantiles = 2;
        double value[2];
        double prob[2] = {0.68, 0.95};
        h->GetQuantiles( nQuantiles, value, prob );
        cout << "\t 68% (95%) value: " << value[0] << "( " << value[1] << ", ";
        cout << h->GetEntries() << " entries)";
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
void plot_mscw_variables( string iFile1, string iFile2, float iW1 = 1., float iW2 = 1., string iAddCut1 = "Erec>0.", string iAddCut2 = "Erec>0.", int log_y = 1, string plot_type = "short_mc", string i_print_file = "" )
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
    cout << "Opening " << iFile1 << endl;
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
    cout << "Opening " << iFile2 << endl;

    vector< string > V;
    vector< float > Vmax;
    vector< float > Vmin;
    vector< string > Vprintout;

    V.push_back( "MSCW" );
    Vmin.push_back(-2. );
    Vmax.push_back( 5. );
    Vprintout.push_back( "RMS" );
    V.push_back( "MSCL" );
    Vmin.push_back(-2. );
    Vmax.push_back( 5. );
    Vprintout.push_back( "RMS" );
    V.push_back( "NImages" );
    Vmin.push_back( 0. );
    Vmax.push_back( 4. );
    Vprintout.push_back( "" );
    if( plot_type.find( "short_mc" ) == string::npos )
    {
        V.push_back( "EmissionHeight" );
        Vmin.push_back( 0. );
        Vmax.push_back( 40. );
        Vprintout.push_back( "" );
        V.push_back( "log10(EChi2)" );
        Vmin.push_back(-2. );
        Vmax.push_back( 4. );
        Vprintout.push_back( "" );
        V.push_back( "log10(EmissionHeightChi2)" );
        Vmin.push_back(-11. );
        Vmax.push_back( 3. );
        Vprintout.push_back( "" );
        V.push_back( "log10(SizeSecondMax)" );
        Vmin.push_back( 2. );
        Vmax.push_back( 5.5 );
        Vprintout.push_back( "" );
        V.push_back( "log10(DispDiff)" );
        Vmin.push_back(-6. );
        Vmax.push_back( 2. );
        Vprintout.push_back( "" );
        V.push_back( "sqrt( Xcore*Xcore+Ycore*Ycore)" );
        Vmin.push_back( 0. );
        Vmax.push_back( 3500. );
        Vprintout.push_back( "" );
    }
    if( is_MC )
    {
        V.push_back( "log10(MCe0)" );
        Vmin.push_back(-2. );
        Vmax.push_back( log10( 300. ) );
        Vprintout.push_back( "VALUE" );
        V.push_back( "(ErecS-MCe0)/MCe0" );
        Vmin.push_back(-1. );
        Vmax.push_back( 1. );
        Vprintout.push_back( "RMS" );
        V.push_back( "(Erec-MCe0)/MCe0" );
        Vmin.push_back(-1. );
        Vmax.push_back( 1. );
        Vprintout.push_back( "RMS" );
    }
    else
    {
        V.push_back( "log10(ErecS)" );
        Vmin.push_back(-2. );
        Vmax.push_back( log10( 300. ) );
        Vprintout.push_back( "VALUE" );
    }

    if( is_MC )
    {
        V.push_back( "sqrt( (Xoff_intersect-MCxoff)*(Xoff_intersect-MCxoff)+(Yoff_intersect-MCyoff)*(Yoff_intersect-MCyoff))" );
        Vprintout.push_back( "68p" );
        Vmin.push_back( 0. );
        Vmax.push_back( 1.5 );
        V.push_back( "sqrt( (Xoff-MCxoff)*(Xoff-MCxoff)+(Yoff-MCyoff)*(Yoff-MCyoff))" );
        Vprintout.push_back( "68p" );
        Vmin.push_back( 0. );
        Vmax.push_back( 1.5 );
    }
    else
    {
        V.push_back( "sqrt( Xoff*Xoff+Yoff*Yoff)" );
        Vprintout.push_back( "68p" );
        Vmin.push_back( 0. );
        Vmax.push_back( 1.5 );
    }


    TCanvas* c = new TCanvas( "c", "mscw variables", 0, 10, 1400, 900 );
    if( is_MC && plot_type.find( "short_mc" ) != string::npos )
    {
        c->Divide( 3, 2 );
    }
    else
    {
        c->Divide( 4, 4);
    }

    int n_bins = 100;
    for( unsigned int i = 0; i < V.size(); i++ )
    {
        TPad* p = ( TPad* )c->cd( i + 1 );
        if( p )
        {
            p->SetLogy( log_y );
            p->SetGridx( 0 );
            p->SetGridy( 0 );
        }

        char Vdraw[400];
        if( V[i] == "NImages" )
        {
            sprintf( Vdraw, "%s>>h%d(%d, %f, %f)", V[i].c_str(), i, int(Vmax[i]-Vmin[i]), Vmin[i], Vmax[i] );
        }
        else
        {
            sprintf( Vdraw, "%s>>h%d(%d, %f, %f)", V[i].c_str(), i, n_bins, Vmin[i], Vmax[i] );
        }
        char Vcut[400];
        sprintf( Vcut, "%f*(%s>%f&&%s<%f&&%s)", iW1, V[i].c_str(), Vmin[i], V[i].c_str(), Vmax[i], iAddCut1.c_str() );
        cout << "Canvas " << i + 1 << ", variable " << V[i] << " cut: " << Vcut << " draw: " << Vdraw << endl;

        f1->cd();
        T1->Draw( Vdraw, Vcut );

        print_statistics_output(( TH1F* )gPad->GetPrimitive( "htemp" ), Vprintout[i] );

        if( T2 )
        {
            if( i >= 0 && plot_type.find("table") == string::npos )
            {
                size_t pos = V[i].find("ErecS");
                if (pos != std::string::npos) {
                    V[i].replace(pos, 5, "Erec" );
                }
            }
            sprintf( Vcut, "%f*(%s>%f&&%s<%f&&%s)", iW2, V[i].c_str(), Vmin[i], V[i].c_str(), Vmax[i], iAddCut1.c_str() );

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
