/*
 *  Compare distributions of some variables from two mscw files
 *
*/


void plot_mscw_variables( string iFile1, string iFile2, float iW1 = 1., float iW2 = 1., string iAddCut = "ErecS>0.", bool short_MC  = false )
{
    TFile* f1 = new TFile( iFile1.c_str() );
    if( f1->IsZombie() )
    {
        return;
    }
    TTree* T1 = ( TTree* )f1->Get( "data" );
    if( !T1 )
    {
        cout << "T1 tree not found" << endl;
        return;
    }
    T1->SetLineWidth( 2 );
    bool is_MC = false;
    if( T1->GetBranchStatus("MCxoff") )
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
        if( !T2 )
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

    V.push_back( "MSCW" );
    Vmin.push_back( -2. );
    Vmax.push_back( 3. );
    V.push_back( "MSCL" );
    Vmin.push_back( -2. );
    Vmax.push_back( 3. );
    V.push_back( "NImages" );
    Vmin.push_back( 0. );
    Vmax.push_back( 70. );
    if( !short_MC )
    {
        V.push_back( "EmissionHeight" );
        Vmin.push_back( 0. );
        Vmax.push_back( 40. );
        V.push_back( "log10(EChi2S)" );
        Vmin.push_back( -2. );
        Vmax.push_back( 4. );
        V.push_back( "log10(EmissionHeightChi2)" );
        Vmin.push_back( -11. );
        Vmax.push_back( 3. );
        V.push_back( "log10(SizeSecondMax)" );
        Vmin.push_back( 2. );
        Vmax.push_back( 8. );
        V.push_back( "log10(DispDiff)" );
        Vmin.push_back( -10. );
        Vmax.push_back( 3. );
        V.push_back( "dES" );
        Vmin.push_back( 0. );
        Vmax.push_back( 3. );
        V.push_back( "sqrt( Xcore*Xcore+Ycore*Ycore)" );
        Vmin.push_back( 0. );
        Vmax.push_back( 2500. );
    }
    if( is_MC )
    {
        V.push_back( "log10(MCe0)" );
        Vmin.push_back( -2. );
        Vmax.push_back( log10( 300. ) );
    }
    else
    {
        V.push_back( "log10(ErecS)" );
        Vmin.push_back( -2. );
        Vmax.push_back( log10( 300. ) );
    }
    if( short_MC )
    {
        V.push_back( "(ErecS-MCe0)/MCe0" );
        Vmin.push_back( -1. );
        Vmax.push_back( 1. );
    }

    if( is_MC )
    {
        V.push_back( "sqrt( (Xoff-MCxoff)*(Xoff-MCxoff)+(Yoff-MCyoff)*(Yoff-MCyoff))" );
    }
    else
    {
        V.push_back( "sqrt( Xoff*Xoff+Yoff*Yoff)" );
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

        if( T2 )
        {
            f2->cd();
            T2->Draw( V[i].c_str(), Vcut, "sames" );
        }
    }
}
