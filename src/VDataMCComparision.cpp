 /* \class VDataMCComparision
 *
 *   
 *   \author Gernot Maier
 */

#include "VDataMCComparision.h"


VDataMCComparision::VDataMCComparision( string iname, bool iBackgroundData, int intel )
{
   bBckData = iBackgroundData;    // use histogram binning for background data
   fNTel = intel;
   fName = iname;
   if( fName != "ON" && fName != "OFF" && fName != "SIMS" && fName != "DIFF" )
   {
      cout << "error: unknown data type: " << fName << endl;
      cout << " allowed data types: ON, OFF, SIMS, DIFF" << endl;
      cout << "...exiting" << endl;
      exit( -1 );
   }
   
   fData = 0;
   fCuts = 0;
   fCalculateMVAValues = false;

// specral weighting (might be read from MC run header)
   fSpectralWeight = new VSpectralWeight();
   fSpectralWeight->setMCParameter( 2.0, 0.05, 20. );
// index MC events are weighted to
   fSpectralWeight->setSpectralIndex( 2.5 );

// setting all variables
   hisList = 0;
   htheta2 = 0;
   hltheta2 = 0;
   hMSCW = 0;
   hMSCL = 0;
   hMSCWErec = 0;
   hMSCLErec = 0;
   hXcore = 0;
   hYcore = 0;
   hXYcore = 0;
   hAzYcore = 0;
   hYt2 = 0;
   hErec = 0;
   hNimages = 0;
   hImgSel = 0;
   hEmissionHeight = 0;
   hMVA = 0;

   fAzRange = false;
   fAzMin = 0.;
   fAzMax = 0.;

   fWobbleNorth = 0.;
   fWobbleEast  = 0.;
   fWobbleFromDataTree = false;

   defineHistograms();
}

/*

   needed only for the calculation of MVA value (not a default)

*/
void VDataMCComparision::initialGammaHadronCuts()
{
   fCuts = new VGammaHadronCuts();
   fCuts->initialize();
   fCuts->resetCutValues();
// HARDWIRED CUT FILE
   if( !fCuts->readCuts( "$VERITAS_EVNDISP_AUX_DIR/GammaHadronCutFiles/ANASUM.GammaHadron.TMVA.BDT.T2Fixed.dat" ) )
   {
      cout << "exiting..." << endl;
      exit( -1 );
   }
   fCuts->initializeCuts();
   fCuts->setDataTree( fData );
   fCuts->printCutSummary();
}


void VDataMCComparision::resetTelescopeCoordinates()
{
   fTel_x.clear();
   fTel_y.clear();
   fTel_z.clear();
}

bool VDataMCComparision::setTelescopeCoordinates( double x, double y, double z )
{
   fTel_x.push_back( x );
   fTel_y.push_back( y );
   fTel_z.push_back( z );
   cout << "\t setting telescope coordinates for telescope " << fTel_x.size()+1 << "\t" << x << "\t" << y << "\t" << z << endl;
   return true;
}

void VDataMCComparision::defineHistograms()
{
   hisList = new TList();
   char hname[200];

   double vmax = 1.;

   if( bBckData ) vmax = 20.;
   else           vmax = 0.3;
   sprintf( hname, "htheta2_%s", fName.c_str() );
   htheta2 = new TH1D( hname, "", 100, 0., vmax );
   htheta2->SetXTitle( "#theta^{2} [deg^{2}]" );
   htheta2->Sumw2();
   hisList->Add( htheta2 );

   sprintf( hname, "hltheta2_%s", fName.c_str() );
   hltheta2 = new TH1D( hname, "", 30, -5., 2. );
   hltheta2->SetXTitle( "log_{10} #theta^{2} [deg^{2}]" );
   hltheta2->Sumw2();
   hisList->Add( hltheta2 );

   if( bBckData ) vmax = 20.;
   else           vmax = 10.;
   sprintf( hname, "hMSCW_%s", fName.c_str() );
   hMSCW = new TH1D( hname, "", 500, -5., vmax );
   hMSCW->SetXTitle( "mean scaled width [deg]" );
   hMSCW->Sumw2();
   hisList->Add( hMSCW );

   if( bBckData ) vmax = 20.;
   else           vmax = 10.;
   sprintf( hname, "hMSCL_%s", fName.c_str() ); 
   hMSCL = new TH1D( hname, "", 500, -5., vmax );
   hMSCL->SetXTitle( "mean scaled length [deg]" );
   hMSCL->Sumw2();
   hisList->Add( hMSCL );

   if( bBckData ) vmax = 20.;
   else           vmax = 10.;
   sprintf( hname, "hMSCWErec_%s", fName.c_str() );
   hMSCWErec = new TH2D( hname, "",  6, -1., 1., 500, -5., vmax );
   hMSCWErec->SetYTitle( "mean scaled width [deg]" );
   hMSCWErec->SetXTitle( "log_{10} energy_{rec} [TeV]" );
   hMSCWErec->Sumw2();
   hisList->Add( hMSCWErec );

   if( bBckData ) vmax = 20.;
   else           vmax = 10.;
   sprintf( hname, "hMSCLErec_%s", fName.c_str() );
   hMSCLErec = new TH2D( hname, "",  6, -1., 1., 500, -5., vmax );
   hMSCLErec->SetYTitle( "mean scaled length [deg]" );
   hMSCLErec->SetXTitle( "log_{10} energy_{rec} [TeV]" );
   hMSCLErec->Sumw2();
   hisList->Add( hMSCLErec );

   sprintf( hname, "hXcore_%s", fName.c_str() );
   hXcore = new TH1D( hname, "", 200, -250., 250. );
   hXcore->SetXTitle( "core position x [m]" );
   hXcore->Sumw2();
   hisList->Add( hXcore );

   sprintf( hname, "hYcore_%s", fName.c_str() );
   hYcore = new TH1D( hname, "", 200, -250., 250. );
   hYcore->SetXTitle( "core position Y [m]" );
   hYcore->Sumw2();
   hisList->Add( hYcore );

   sprintf( hname, "hXYcore_%s", fName.c_str() );
   hXYcore = new TH2D( hname, "", 75, -250., 250., 75, -250., 250. );
   hXYcore->SetXTitle( "core position X [m]" );
   hXYcore->SetYTitle( "core position Y [m]" );
   hXYcore->Sumw2();
   hisList->Add( hXYcore );

   sprintf( hname, "hAzYcore_%s", fName.c_str() );
   hAzYcore = new TH2D( hname, "", 360, -180., 180., 300, -250., 250. );
   hAzYcore->SetXTitle( "core position X [m]" );
   hAzYcore->SetYTitle( "core position Y [m]" );
   hAzYcore->Sumw2();
   hisList->Add( hAzYcore );

   sprintf( hname, "hYt2_%s", fName.c_str() );
   hYt2 = new TH2D( hname, "", 100,-6., 0., 800, -200., 200. );
   hYt2->SetXTitle( "log_{10} #Theta^{2}" );
   hYt2->SetYTitle( "core position Y [m]" );
   hYt2->Sumw2();
   hisList->Add( hYt2 );
   
   sprintf( hname, "hNimages_%s", fName.c_str() );
   hNimages = new TH1D( hname, "", 5, 0., 5. );
   hNimages->SetXTitle( "number of images" );
   hNimages->Sumw2();
   hisList->Add( hNimages );

   sprintf( hname, "hImgSel_%s", fName.c_str() );
   hImgSel = new TH1D( hname, "", 16, 0., 16. );
   hImgSel->SetXTitle( "telescope combinations" );
   hImgSel->Sumw2();
   hisList->Add( hImgSel );

   sprintf( hname, "hEmissionHeight_%s", fName.c_str() );
   hEmissionHeight = new TH1D( hname, "", 100, 0., 200. );
   hEmissionHeight->SetXTitle( "estimated height of maximum emission [km]" );
   hEmissionHeight->Sumw2();
   hisList->Add( hEmissionHeight );

   sprintf( hname, "hMVA_%s", fName.c_str() );
   hMVA = new TH1D( hname, "", 100, -1., 1. );
   hMVA->SetXTitle( "MVA variable" );
   hMVA->Sumw2();
   hisList->Add( hMVA );

// this histogram is only filled for simulations
   sprintf( hname, "hErec_%s", fName.c_str() );
   hErec = new TH1D( hname, "", 50, -2., 2. );
   hErec->SetXTitle( "log_{10} energy_{MC}" );
   hErec->Sumw2();
   hisList->Add( hErec );

   for( int i = 0; i < fNTel; i++ )
   {
      sprintf( hname, "hR%d_%s", i+1, fName.c_str() );
      hR.push_back( new TH1D( hname, "", 25, 0., 300. ) );
      sprintf( hname, "distance to T%d [m]", i+1 );
      hR.back()->SetXTitle( hname );
      hisList->Add( hR.back() );

      sprintf( hname, "hdistR%d_%s", i+1, fName.c_str() );
      hdistR.push_back( new TH2D( hname, "", 20, 0., 300., 20, 0., 2. ) );
      sprintf( hname, "distance to T%d [m]", i+1 );
      hdistR.back()->SetXTitle( hname );
      hdistR.back()->SetYTitle( "local distance [deg]" );
      hisList->Add( hdistR.back() );
      hTel2D.push_back( hdistR.back() );
   }

// telescope numbering starts at 1!
   for( int i = 1; i <= fNTel; i++ )
   {
      if( bBckData ) vmax = 1.;
      else           vmax = 0.5;
      sprintf( hname, "hlength_%d_%s", i, fName.c_str() );
      hlength.push_back( new TH1D( hname, "", 80, 0., vmax) );
      hlength.back()->SetXTitle( "length [deg]" );
      hlength.back()->Sumw2();
      hTel.push_back( hlength.back() );
      hisList->Add( hlength.back() );

      if( bBckData ) vmax = 1.;
      else           vmax = 0.25;
      sprintf( hname, "hwidth_%d_%s", i, fName.c_str() );
      hwidth.push_back( new TH1D( hname, "", 100, 0., vmax) );
      hwidth.back()->SetXTitle( "width [deg]" );
      hwidth.back()->Sumw2();
      hTel.push_back( hwidth.back() );
      hisList->Add( hwidth.back() );

      sprintf( hname, "hdist_%d_%s", i, fName.c_str() );
      hdist.push_back( new TH1D( hname, "", 100, 0., 2.0) );
      hdist.back()->SetXTitle( "dist [deg]" );
      hdist.back()->Sumw2();
      hTel.push_back( hdist.back() );
      hisList->Add( hdist.back() );

      if( bBckData ) vmax = 90.;
      else           vmax = 25.;
      sprintf( hname, "halpha_%d_%s", i, fName.c_str() );
      halpha.push_back( new TH1D( hname, "", 100, 0., vmax) );
      halpha.back()->SetXTitle( "alpha [deg]" );
      halpha.back()->Sumw2();
      hTel.push_back( halpha.back() );
      hisList->Add( halpha.back() );

      sprintf( hname, "hntubes_%d_%s", i, fName.c_str() );
      hntubes.push_back( new TH1D( hname, "", 100, 0., 200.0) );
      hntubes.back()->SetXTitle( "ntubes" );
      hntubes.back()->Sumw2();
      hTel.push_back( hntubes.back() );
      hisList->Add( hntubes.back() );

      sprintf( hname, "hnlowgain_%d_%s", i, fName.c_str() );
      hnlowgain.push_back( new TH1D( hname, "", 100, 0., 100.0) );
      hnlowgain.back()->SetXTitle( "nlowgain" );
      hnlowgain.back()->Sumw2();
      hTel.push_back( hnlowgain.back() );
      hisList->Add( hnlowgain.back() );

      sprintf( hname, "hsize_%d_%s", i, fName.c_str() );
      hsize.push_back( new TH1D( hname, "", 80, 2., 5.0) );
      hsize.back()->SetXTitle( "log_{10} size [d.c.]" );
      hsize.back()->Sumw2();
      hTel.push_back( hsize.back() );
      hisList->Add( hsize.back() );

      sprintf( hname, "hsize2_%d_%s", i, fName.c_str() );
      hsize2.push_back( new TH1D( hname, "", 80, 2., 5.0) );
      hsize2.back()->SetXTitle( "log_{10} size2 [d.c.]" );
      hsize2.back()->Sumw2();
      hTel.push_back( hsize2.back() );
      hisList->Add( hsize2.back() );

      sprintf( hname, "hmax1_%d_%s", i, fName.c_str() );
      hmax1.push_back( new TH1D( hname, "", 80, 1., 4.0) );
      hmax1.back()->SetXTitle( "log_{10} size_{max1} [d.c.]" );
      hmax1.back()->Sumw2();
      hTel.push_back( hmax1.back() );
      hisList->Add( hmax1.back() );

      sprintf( hname, "hmax2_%d_%s", i, fName.c_str() );
      hmax2.push_back( new TH1D( hname, "", 80, 1., 4.0) );
      hmax2.back()->SetXTitle( "log_{10} size_{max2} [d.c.]" );
      hmax2.back()->Sumw2();
      hTel.push_back( hmax2.back() );
      hisList->Add( hmax2.back() );

      sprintf( hname, "hmax3_%d_%s", i, fName.c_str() );
      hmax3.push_back( new TH1D( hname, "", 80, 1., 4.0) );
      hmax3.back()->SetXTitle( "log_{10} size_{max3} [d.c.]" );
      hmax3.back()->GetXaxis()->SetTitleOffset( 1.2 );
      hmax3.back()->Sumw2();
      hTel.push_back( hmax3.back() );
      hisList->Add( hmax3.back() );

      sprintf( hname, "hloss_%d_%s", i, fName.c_str() );
      hloss.push_back( new TH1D( hname, "", 80, 0., 1. ) );
      hloss.back()->SetXTitle( "loss" );
      hloss.back()->Sumw2();
      hTel.push_back( hloss.back() );
      hisList->Add( hloss.back() );

      sprintf( hname, "hlos_%d_%s", i, fName.c_str() );
      hlos.push_back( new TH1D( hname, "", 40, 0., 1. ) );
      hlos.back()->SetXTitle( "length/size [deg] x 1000" );
      hlos.back()->Sumw2();
      hTel.push_back( hlos.back() );
      hisList->Add( hlos.back() );

      sprintf( hname, "hasym_%d_%s", i, fName.c_str() );
      hasym.push_back( new TH1D( hname, "", 100, -2., 2. ) );
      hasym.back()->SetXTitle( "asymmetry" );
      hasym.back()->Sumw2();
      hTel.push_back( hasym.back() );
      hisList->Add( hasym.back() );

      sprintf( hname, "hcen_xy%d_%s", i, fName.c_str() );
      hcen_xy.push_back( new TH2D( hname, "", 100, -2., 2., 100, -2., 2. ) );
      hcen_xy.back()->SetXTitle( "image centroid (x) [deg]" );
      hcen_xy.back()->SetYTitle( "image centroid (y) [deg]" );
      hTel2D.push_back( hcen_xy.back() );
      hisList->Add( hcen_xy.back() );


      sprintf( hname, "hcen_x_%d_%s", i, fName.c_str() );
      hcen_x.push_back( new TH1D( hname, "", 100, -2., 2. ) );
      hcen_x.back()->SetXTitle( "image centroid (x) [deg]" );
      hcen_x.back()->Sumw2();
      hTel.push_back( hcen_x.back() );
      hisList->Add( hcen_x.back() );

      sprintf( hname, "hcen_y_%d_%s", i, fName.c_str() );
      hcen_y.push_back( new TH1D( hname, "", 100, -2., 2. ) );
      hcen_y.back()->SetXTitle( "image centroid (y) [deg]" );
      hcen_y.back()->Sumw2();
      hTel.push_back( hcen_y.back() );
      hisList->Add( hcen_y.back() );

      sprintf( hname, "htgrad_x_%d_%s", i, fName.c_str() );
      htgrad_x.push_back( new TH1D( hname, "", 100, -30., 30. ) );
      htgrad_x.back()->SetXTitle( "time gradient (x)" );
      htgrad_x.back()->Sumw2();
      hTel.push_back( htgrad_x.back() );
      hisList->Add( htgrad_x.back() );

      sprintf( hname, "hmscwt_%d_%s", i, fName.c_str() );
      hmscwt.push_back( new TH1D( hname, "", 100, 0., 2. ) );
      hmscwt.back()->SetXTitle( "expected width" );
      hmscwt.back()->Sumw2();
      hTel.push_back( hmscwt.back() );
      hisList->Add( hmscwt.back() );

      sprintf( hname, "hmsclt_%d_%s", i, fName.c_str() );
      hmsclt.push_back( new TH1D( hname, "", 100, 0., 2. ) );
      hmsclt.back()->SetXTitle( "expected length" );
      hmsclt.back()->Sumw2();
      hTel.push_back( hmsclt.back() );
      hisList->Add( hmsclt.back() );

      sprintf( hname, "hr_%d_%s", i, fName.c_str() );
      hr.push_back( new TH1D( hname, "", 100, 0., 300. ) );
      hr.back()->SetXTitle( "distance telescope - core [m]" );
      hr.back()->Sumw2();
      hTel.push_back( hr.back() );
      hisList->Add( hr.back() );
   }
   TIter next(hisList);
   while(TH1* h = (TH1*)next() )
   {
      if( fName == "OFF" && h ) h->SetLineColor( 2 );
   }

}

/*
 * norm applies on off runs (mult.)
*/

bool VDataMCComparision::setOnOffHistograms( VDataMCComparision *on, VDataMCComparision *off, double norm )
{
   if( !on || !off ) 
   {
      cout << "on or off not defined" << endl;
      return false;
   }
   norm *= -1.;

   htheta2->Add( on->htheta2, off->htheta2, 1., norm );
   setEntries( htheta2 );
   hltheta2->Add( on->hltheta2, off->hltheta2, 1., norm );
   setEntries( hltheta2 );
   hMSCW->Add( on->hMSCW, off->hMSCW, 1., norm );
   setEntries( hMSCW );
   hMSCL->Add( on->hMSCL, off->hMSCL, 1., norm );
   setEntries( hMSCL );
   hMSCWErec->Add( on->hMSCWErec, off->hMSCWErec, 1., norm );
   setEntries( hMSCWErec );
   hMSCLErec->Add( on->hMSCLErec, off->hMSCLErec, 1., norm );
   setEntries( hMSCLErec );
   hXcore->Add( on->hXcore, off->hXcore, 1., norm );
   setEntries( hXcore );
   hYcore->Add( on->hYcore, off->hYcore, 1., norm );
   setEntries( hYcore );
   hXYcore->Add( on->hXYcore, off->hXYcore, 1., norm );
   hAzYcore->Add( on->hAzYcore, off->hAzYcore, 1., norm );
   hYt2->Add( on->hYt2, off->hYt2, 1., norm );
   for( unsigned int j = 0; j < hR.size(); j++ ) hR[j]->Add( on->hR[j], off->hR[j], 1., norm );
   hNimages->Add( on->hNimages, off->hNimages, 1., norm );
   hImgSel->Add( on->hImgSel, off->hImgSel, 1., norm );
   hEmissionHeight->Add( on->hEmissionHeight, off->hEmissionHeight, 1., norm );
   hMVA->Add( on->hMVA, off->hMVA, 1., norm );

// 1D histograms
   for( unsigned int j = 0; j < hTel.size(); j++ ) hTel[j]->Add( on->hTel[j], off->hTel[j], 1., norm );
// 2d histograms
   for( unsigned int j = 0; j < hTel2D.size(); j++ ) hTel2D[j]->Add( on->hTel2D[j], off->hTel2D[j], 1., norm );

   return true;
}

void VDataMCComparision::setEntries( TH1D * iH )
{
   double ie = 0.;
   for( int i = 1; i <= iH->GetNbinsX(); i++ ) if( iH->GetBinContent( i ) > 0. ) ie += iH->GetBinContent( i );

   iH->SetEntries( ie );
}

void VDataMCComparision::setEntries( TH2D * iH )
{
   double ie = 0.;
   for( int i = 1; i <= iH->GetNbinsX(); i++ )
   {
       for( int j = 1; j <= iH->GetNbinsY(); j++ )
       {
           if( iH->GetBinContent( i, j ) > 0. ) ie += iH->GetBinContent( i, j );
       }
   }
   iH->SetEntries( ie );
}

bool VDataMCComparision::fillHistograms( string ifile, int iSingleTelescopeCuts, double iWN, double iWE )
{
   fWobbleNorth = iWN;
   fWobbleEast  = iWE;

   fWobbleFromDataTree = false;

   return fillHistograms( ifile, iSingleTelescopeCuts );
}

bool VDataMCComparision::fillHistograms( string ifile, int iSingleTelescopeCuts )
{
// quality cuts
   double fCoreMax_QC = 350.;       // cut on core distance
   int    fNImages_min = 3;         // minimum number of images per event
   fNImages_min = 2;
// stereo cuts
   double theta2_cut = 0.035;
//   if( fNTel > 2 ) theta2_cut = 0.025;
   if( iSingleTelescopeCuts > 0 || iSingleTelescopeCuts == -2 ) theta2_cut = 0.2;
   if( iSingleTelescopeCuts == -3 ) theta2_cut = 0.035;
   double theta2_min = 0.;
   double msw_max = 0.5;
   double msw_min = -1.2;
   double msl_max = 0.5;
   double msl_min = -1.2; 
   double size2ndmax_min = 0.;

// single telescope cuts
    int    ntubes_min = 4;
    double alpha_max = 8.;
    double length_min = 0.12;
    double length_max = 0.38;
    double width_max = 0.12;
    double width_min = 0.05;
    double los_max = 0.0003;
    double dist_min = -10.;
    double dist_max = 1.e10;

   TChain *iC = new TChain( "data" );
   if( !iC->Add( ifile.c_str() ) )
   {
       cout << "error while reading data chain" << endl;
       cout << "exiting..." << endl;
       exit( -1 );
   }
   if( iC->GetFile()->Get("data") )
   {
      iC->SetTitle( iC->GetFile()->Get( "data" )->GetTitle() );

// get MC data
      VMonteCarloRunHeader *iMC_H = (VMonteCarloRunHeader*)iC->GetFile()->Get( "MC_runheader" );
      if( iMC_H )
      {
	 if( fSpectralWeight ) 
	 {
	    fSpectralWeight->setMCParameter( -1.*iMC_H->spectral_index, iMC_H->E_range[0], iMC_H->E_range[1] );
	    fSpectralWeight->print();
         }
      }
   }

// set this false for stereo cuts
   int fSingleTelescopeCuts = iSingleTelescopeCuts;

   if( fName == "SIMS" ) cout << "\t reading simulations..." << endl;
   fData = new CData( iC, fName == "SIMS" );

   int nentries =  fData->fChain->GetEntries();
   cout << "\t entries: " << nentries << " (" << fNTel << " telescopes)" << endl;
   cout << "\t quality cuts: " << endl;
   cout << "\t\t maximum core distance [m]: " << fCoreMax_QC << endl;
   cout << "\t\t minimum number of images per event: " << fNImages_min << endl;
   cout << "\t cuts: ";
   if( fSingleTelescopeCuts == -1 )
   {
       cout << " stereo cuts (hardwired)" << endl;
       cout << "\t " << msw_min << " < MSCW < " << msw_max << ", " << msl_min << " < MSCL < " << msl_max;
       cout << ", theta2 < " << theta2_cut << " deg2" << endl;
   }
   else if( fSingleTelescopeCuts == -2 ) cout << "NO CUTS (quality cuts only)" << endl;
   else if( fSingleTelescopeCuts == -3 )
   {
       cout << " Theta2 cut (<" << theta2_cut << " deg2),  ";
       cout << " Size2ndMax cut (<" << size2ndmax_min << ")" << endl;
   }
   else
   {
      cout << " single telescope cuts for Telescope (hardwired): " << fSingleTelescopeCuts << endl;
      cout << "\t ntubes >" << ntubes_min << endl;
      cout << "\t alpha < " << alpha_max << endl;
      cout << "\t " << length_min << " < length < " << length_max << endl;
      cout << "\t " << width_min << " < width < " << width_max << endl;
      cout << "\t los < " << los_max << endl;
      cout << "\t " << dist_min << " < dist < " << dist_max << endl;
   }

   double rdist1 = 0.;
   double theta2 = 0.;

   int iOldRun = 0;

   double weight = 1.;

   int fInput = 0;
   if( fName == "SIMS" ) fInput = 0;
   else if( fName == "ON" ) fInput = 1;
   else if( fName == "OFF" ) fInput = 2;

////////////////////////////////////////////////////////////
// VGammaHadronCuts is needed for calulate of MVA values
   if( fCalculateMVAValues ) initialGammaHadronCuts();

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// now loop over all events
   for( int i = 0; i < nentries; i++ )
   {
      fData->GetEntry( i );

// define astro object (each time a new run starts)
      if( iOldRun != fData->runNumber && fWobbleFromDataTree )
      {
         fWobbleNorth = fData->WobbleN;
	 fWobbleEast = fData->WobbleE;

         cout << "\t now at run " << fData->runNumber << " (" << fData->eventNumber << "):";
	 cout << " N " << fData->WobbleN << ", E " << fData->WobbleE << endl;

         iOldRun = fData->runNumber;
      }

// nimage cut
      if( fData->NImages < fNImages_min ) continue;

// apply az cut to MC (to choose similar AZ range as in Crab runs)
     if( fInput == 0 && ( fData->MCaz < 105.  || fData->MCaz > 230. ) ) continue;

/////////////////////////////////////////////////
// quality cuts
      if( fData->EChi2 < 0 ) continue;
      if( fData->MSCW < -50. ) continue;
      if( fData->MSCL < -50. ) continue;
      if( sqrt( fData->Xcore*fData->Xcore + fData->Ycore*fData->Ycore ) > fCoreMax_QC ) continue;

// AZ cut to check azimuth dependence of core distance distributions
      if( fAzRange && (fData->Az < fAzMin || fData->Az > fAzMax ) ) continue; 

      theta2 = fData->theta2;
// get correct theta2 for wobble runs
// (off data)
      if( fInput == 2 )
      {
	  theta2 = (fData->Yoff_derot-fWobbleNorth)*(fData->Yoff_derot-fWobbleNorth) + (fData->Xoff_derot-fWobbleEast)*(fData->Xoff_derot-fWobbleEast);
      }
// MC data
      else if( fInput == 0 )
      {
         theta2 = (fData->Yoff-fData->MCyoff)*(fData->Yoff-fData->MCyoff) + (fData->Xoff-fData->MCxoff)*(fData->Xoff-fData->MCxoff);
      }
// on data
      else
      {
	  theta2 = (fData->Yoff_derot+fWobbleNorth)*(fData->Yoff_derot+fWobbleNorth) + (fData->Xoff_derot+fWobbleEast)*(fData->Xoff_derot+fWobbleEast);
      }

// this is only filled for simulations
      if( fInput == 0 && fData->MCe0 > 0 )
      {
         hErec->Fill( log10( fData->MCe0 ) );
	 if( fSpectralWeight ) weight = fSpectralWeight->getSpectralWeight( fData->MCe0 );
	 else                  weight = 1.;
      }
      else weight = 1.;

// fill histograms with all cuts applied
//
     
// quality cuts & theta2 cut only
      if( fSingleTelescopeCuts < -1 )
      {
         if( theta2 <= theta2_min || theta2 > theta2_cut ) continue;
	 if( fData->SizeSecondMax < size2ndmax_min ) continue;
      }

/////////////////////////////////////////////////////////
//   ---    apply single telescope cuts  ----
/////////////////////////////////////////////////////////
//   apply single telescope cuts to the named telescope
      if( fSingleTelescopeCuts > 0 )
      {
         if( fWobbleNorth != 0. || fWobbleEast != 0. )
	 {
	     if( theta2 <= theta2_min || theta2 > theta2_cut ) continue;
         }
	 else
	 {
	     if( fData->alpha[fSingleTelescopeCuts-1] > alpha_max ) continue;
         }
	 if( fData->dist[fSingleTelescopeCuts-1] < dist_min || fData->dist[fSingleTelescopeCuts-1] > dist_max ) continue;
	 if( fData->size[fSingleTelescopeCuts-1] && fData->length[fSingleTelescopeCuts-1]/fData->size[fSingleTelescopeCuts-1] > los_max ) continue;
	 if( fData->length[fSingleTelescopeCuts-1] < length_min ||  fData->length[fSingleTelescopeCuts-1] > length_max ) continue;
	 if( fData->width[fSingleTelescopeCuts-1] < width_min || fData->width[fSingleTelescopeCuts-1] > width_max ) continue;    
      }

      if( fSingleTelescopeCuts != -1 || (theta2 >= 0. && fData->MSCW < msw_max && fData->MSCW > msw_min && fData->MSCL > msl_min && fData->MSCL < msl_max ) )
      {
	 htheta2->Fill( theta2, weight );
	 hltheta2->Fill( log10(theta2), weight );
	 hYt2->Fill( log10( theta2 ), fData->Ycore, weight );
      }
      if( fSingleTelescopeCuts != -1 || (theta2 >= theta2_min && theta2 < theta2_cut &&  fData->MSCL < msl_max && fData->MSCL > msl_min ) )
      {
	 hMSCW->Fill( fData->MSCW, weight );
	 if( fData->Erec > 0. ) hMSCWErec->Fill( log10( fData->Erec ), fData->MSCW, weight );
	 for( int j = 0; j < fNTel; j++ ) if( fData->MSCWT[j] > 0. ) hmscwt[j]->Fill( fData->width[j] / fData->MSCWT[j], weight );
      }
      if( fSingleTelescopeCuts != -1 || (theta2 >= theta2_min && theta2 < theta2_cut  &&  fData->MSCW < msw_max && fData->MSCW > msw_min ) )
      {
	 hMSCL->Fill( fData->MSCL, weight );
	 for( int j = 0; j < fNTel; j++ ) if( fData->MSCLT[j] > 0. ) hmsclt[j]->Fill( fData->length[j] / fData->MSCLT[j], weight );
	 if( fData->Erec > 0. ) hMSCLErec->Fill( log10( fData->Erec ), fData->MSCL, weight );
	 if( fData->EmissionHeight > 0. ) hEmissionHeight->Fill( fData->EmissionHeight, weight );
	 hNimages->Fill( fData->NImages, weight );
	 hImgSel->Fill( fData->ImgSel, weight );
	 if( fCuts )
	 {
	    fCuts->newEvent();
	    fCuts->applyTMVACut( i );
	    if( fCuts->getTMVA_EvaluationResult() > -90. ) hMVA->Fill( fCuts->getTMVA_EvaluationResult(), weight );
         }
      }
       
////////////////////////////////////////////////////////////////////////////////////////////////
// stereo parameters
      if( fSingleTelescopeCuts != -1 ||
          ( theta2 >= theta2_min && theta2 < theta2_cut  && fData->MSCW < msw_max && fData->MSCW > msw_min && fData->MSCL < msl_max && fData->MSCL > msl_min ) )
      {
	 hXcore->Fill( fData->Xcore, weight );
	 if( fInput == 0 )
	 {
	    hYcore->Fill( fData->Ycore , weight );
	    hXYcore->Fill( fData->Xcore, fData->Ycore, weight );
	    hAzYcore->Fill( fData->Az, fData->Ycore , weight );
         }
	 else
	 {
	    hYcore->Fill( fData->Ycore , weight );
	    hXYcore->Fill( fData->Xcore, fData->Ycore, weight );
	    hAzYcore->Fill( fData->Az, fData->Ycore , weight );
         }


///////////////////////////////////////////////////////////////
// core position relative to each telescope
	 for( int j = 0; j < fNTel; j++ )
	 {
	    if( fData->ntubes[j] > ntubes_min )
	    {
	       rdist1 = VUtilities::line_point_distance( fData->Ycore, -1.*fData->Xcore, 0., fData->Ze, fData->Az, fTel_y[j], -1.*fTel_x[j], fTel_z[j] );
	       hR[j]->Fill( rdist1, weight );
	       hdistR[j]->Fill( rdist1, sqrt( fData->cen_x[j]*fData->cen_x[j]+fData->cen_y[j]*fData->cen_y[j]), weight );
            }
	 }

///////////////////////////////////////////////////////////////
// single telescope distributions
	 for( int j = 0; j < fNTel; j++ )
	 {
	    if( fData->ntubes[j] > ntubes_min )
	    {
	       hntubes[j]->Fill( fData->ntubes[j], weight );
	       if( fData->nlowgain[j] > 0 ) hnlowgain[j]->Fill( (double)fData->nlowgain[j], weight );
	       hdist[j]->Fill( fData->dist[j], weight );
	       if( fData->size[j] > 0. ) hsize[j]->Fill( log10( fData->size[j] ), weight );
	       if( fData->size[j] > 0. ) hsize2[j]->Fill( log10( fData->size2[j] ), weight );
	       if( fData->max1[j] > 0. ) hmax1[j]->Fill( log10( fData->max1[j] ), weight );
	       if( fData->max2[j] > 0. ) hmax2[j]->Fill( log10( fData->max2[j] ), weight );
	       if( fData->max3[j] > 0. ) hmax3[j]->Fill( log10( fData->max3[j] ), weight );
	       hwidth[j]->Fill( fData->width[j], weight );
	       hlength[j]->Fill( fData->length[j], weight );
	       halpha[j]->Fill( fData->alpha[j], weight );
	       if( fData->size[j] > 0. ) hlos[j]->Fill( fData->length[j] / fData->size[j] * 1000., weight );
	       hloss[j]->Fill( fData->loss[j], weight );
	       hasym[j]->Fill( fData->asym[j], weight );
               if( fInput == 0 )
	       {
		  hcen_x[j]->Fill( fData->cen_x[j], weight );
		  hcen_y[j]->Fill( -1.*fData->cen_y[j], weight );
               }
	       else
	       {
		  hcen_x[j]->Fill( fData->cen_x[j], weight );
		  hcen_y[j]->Fill( fData->cen_y[j], weight );
               }
	       hcen_xy[j]->Fill( fData->cen_x[j], fData->cen_y[j], weight );
	       htgrad_x[j]->Fill( fData->tgrad_x[j], weight );
	       hr[j]->Fill( fData->R[j], weight );
	    }
         }
      }
   }
   return true;
}

void VDataMCComparision::scaleHistograms( string ifile )
{
   TFile f( ifile.c_str() );
   if( f.IsZombie() )
   {
      cout << "histogram scaling failed, no input file: " << ifile << endl;
      return;
   }
   TH1D *hEmc = (TH1D*)f.Get("hE0mc" );
   if( !hEmc )
   {
      cout << "histogram scaling failed, no mc histogram" << endl;
      return;
   }
   double iEntries = (double)hEmc->GetEntries();

   TIter next(hisList);
   while(TH1* h = (TH1*)next() )
   {
      if( iEntries > 0 ) h->Scale( 1./iEntries );
   }
}

bool VDataMCComparision::writeHistograms( TDirectory *iOut )
{
   iOut->cd();
   cout << "\t writing results (" << fName << ") to " << iOut->GetName() << endl;

    TIter next(hisList);
    while(TObject* h = next() )
    {
       h->Write();
    }

   return true;
}

void VDataMCComparision::setAzRange( double iMin, double iMax )
{
   fAzMin = 0.;
   fAzMax = 0.;
   fAzRange = false;

   if( TMath::Abs( iMin ) > 5. && TMath::Abs( iMax ) > 5. )
   {
      fAzMin = iMin;
      fAzMax = iMax;
      fAzRange = true;
   }
}

