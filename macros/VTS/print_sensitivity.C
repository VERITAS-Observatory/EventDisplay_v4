/* 
 * calculate sensitivity from Crab runs and
 * print in latex and markdown
 *
 * root -l -q -b 'print_sensitivity.C("$VERITAS_USER_DATA_DIR/analysis/Results/v490/AP/Crab_DISP/archive/20230218_DISPwithDispDiff/anasum_V6_moderate2tel_0.5deg_IGNOREACCEPTANCE/anasum.combined.root")'
 *
*/

R__LOAD_LIBRARY( $EVNDISPSYS/lib/libVAnaSum.so );

void print_sensitivity( string anasum_file, string latex_table_title = "", double alpha = 1./6. )
{
    // get on and off counts
    TFile f( anasum_file.c_str() );
    if( f.IsZombie() )
    {
        cout << "Error reading anasum file " << anasum_file << endl;
        cout << "Exiting...";
        return;
    }
    TTree *t = (TTree*)f.Get( "total_1/stereo/tRunSummary" );
    if( !t )
    {
        cout << "Error reading run summary tree from " << anasum_file << endl;
        cout << "Exiting...";
        return;
    }
    double Rate = 0;
    double RateOff = 0.;
    double RateE = 0;
    double RateOffE = 0.;
    t->SetBranchAddress( "Rate", &Rate );
    t->SetBranchAddress( "RateOff", &RateOff );
    t->SetBranchAddress( "RateE", &RateE );
    t->SetBranchAddress( "RateOffE", &RateOffE );

    t->GetEntry( t->GetEntries()-1 );

    cout << "Rates (on/Off): " << Rate << "\t" << RateOff << endl;

    VSensitivityCalculator a;
    a.addDataSet( Rate, RateOff, alpha, "" );
    a.list_sensitivity();
    a.list_sensitivity_latex_table( 0, latex_table_title, RateE);
}
