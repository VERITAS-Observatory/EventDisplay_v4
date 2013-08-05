/*! \file   plot_CTA
    \brief  CTA related plotting macros

*/



#include <fstream>
#include <iostream>
#include <string>
#include <vector>

vector< string > getListofArrrays( string iArrayFile )
{
   vector< string > SubArray;
   ifstream is;
   is.open( iArrayFile.c_str(), ifstream::in );
   if( !is ) return;

   string is_line;
   while( getline( is, is_line ) ) 
   {
      SubArray.push_back( is_line );
   }

   return SubArray;
}

/*
 
   plot a single telescope

*/

void drawTelescope( double x, double y, string iText, double iMarkerMult, int iMarkerColor )
{
    TGraph *iG = new TGraph( 1 );
    iG->SetMarkerStyle( 20 );
    iG->SetMarkerSize( 1.5 * iMarkerMult );
    iG->SetMarkerColor( iMarkerColor );

    iG->SetPoint( 0, x, y );

    iG->Draw( "p" );

    if( iText.size() > 0 )
    {
       TText *iT = new TText( x, y, iText.c_str() );
       iT->SetTextSize( 0.014 );
       iT->Draw();
    }
}



/*
     plot array layout

     INPUT:

     ifile        eventdisplay or mscw root file
     iname        name of array (e.g. J)
     iMarkerMult  increase marker size by this factor
     xmax/ymax    maximum extension of array to plot (in [m])

     OUTPUT:

     plots

*/
TCanvas* plot_array( char *ifile, char *iname = 0, double iMarkerMult = 1., double xmax = 1450., double ymax = 1450., bool drawTelescopeNumbers = true )
{
    TFile *f1 = new TFile( ifile );
    if( f1->IsZombie() ) return 0;

    TTree *t = (TTree*)f1->Get( "telconfig");
    if( !t ) return 0;

    cout << "Array " << iname << ": telconfig tree found with " << t->GetEntries() << " telescopes" << endl;

    TCanvas *c = new TCanvas( "c2", "array layout", 10, 10, 600, 600 );
    c->SetGridx( 0 );
    c->SetGridy( 0 );
    c->SetRightMargin( 0.05 );
    c->SetLeftMargin( 0.13 );
    c->Draw();

    TH2D *hnull = new TH2D( "hnull", "", 100, -1.*xmax, xmax, 100, -1.*ymax, ymax );
    hnull->SetStats( 0 );
    hnull->SetXTitle( "x [m]" );
    hnull->SetYTitle( "y [m]" );
    hnull->GetYaxis()->SetTitleOffset( 1.6 );

    hnull->Draw();

    float iTelX = 0.;
    float iTelY = 0.;
    ULong64_t iTelType = 0;
    int iTelID = 0;
    unsigned int iTelIDHA = 0;

    t->SetBranchAddress( "TelID", &iTelID );
    t->SetBranchAddress( "TelType", &iTelType );
    t->SetBranchAddress( "TelID_hyperArray", &iTelIDHA );
    t->SetBranchAddress( "TelX", &iTelX );
    t->SetBranchAddress( "TelY", &iTelY );

    int iMarkerColor = 0;
    char hname[200];

    unsigned int iLST = 0;
    unsigned int iMST = 0;
    unsigned int iDCSST = 0;
    unsigned int iSCSST = 0;

    for( int i = 0; i < t->GetEntries(); i++ )
    {
       t->GetEntry( i );

// LSTs (1)
       if( iTelType == 138704810 || iTelType == 141305009 || iTelType == 141305109 )
       {
          iMarkerColor = 2;
	  iLST++;
       }
// standard MSTs (2)
       else if( iTelType == 10007818 || iTelType == 10408418 || iTelType == 10008118 )
       {
          iMarkerColor = 1;
	  iMST++;
       }
// large pixel MSTs (4)
       else if( iTelType == 10009725 )  iMarkerColor = 6;
// standard SSTs (3)
       else if( iTelType == 3709725 || iTelType == 3709425 || iTelType == 3710125 )
       {
          iMarkerColor = 3;
	  iDCSST++;
       }
// 7m telescopes (5, prod1) or SCT (prod2)
       else if( iTelType == 7309930  || iTelType == 201509515 )
       {
          iMarkerColor = 4;
	  iSCSST++;
       }
       else
       {
          cout << "unknown telescope type: " << iTelType << endl;
	  continue;
       }

       if( drawTelescopeNumbers ) sprintf( hname, "%d, %d", iTelID, iTelIDHA );
       else                       sprintf( hname, "" );
       drawTelescope( iTelX, iTelY, hname, iMarkerMult, iMarkerColor );


    }
    cout << "# of LSTs: " << iLST << endl;
    cout << "# of MSTs: " << iMST << endl;
    cout << "# of DCSSTs: " << iDCSST << endl;
    cout << "# of SCSSTs: " << iSCSST << endl;
// draw array name
    if( iname )
    {
       TText *it = new TText( -1.*0.9*xmax, 0.75*ymax, iname );
       it->SetTextSize( it->GetTextSize() * 1.5 );
       it->Draw();
    }

    return c;
}

/*

    plot all array layout from a list of runs

    (read telconfig trees from evndisp output files)

*/
void plot_allArrayLayouts( string iArrayListFile, string iFileName, double iMarkerMult = 1., double xmax = 1450., double ymax = 1450., string iDataSet = "prod2-Aar-North" )
{
   vector< string > iVArray = getListofArrrays( iArrayListFile );

   char hname[2000];

   for( unsigned int i = 0; i < iVArray.size(); i++ )
   {
      sprintf( hname, "$CTA_USER_DATA_DIR/analysis/AnalysisData/%s/%s/gamma_onSource/%s", iDataSet.c_str(), iVArray[i].c_str(), iFileName.c_str() );
      cout << hname << endl;

      TCanvas *cC = plot_array( hname, iVArray[i].c_str(), iMarkerMult, xmax, ymax );
      if( cC )
      {
         sprintf( hname, "ArrayLayout-%s.eps", iVArray[i].c_str() );
	 cC->Print( hname );
      }
   }
}
