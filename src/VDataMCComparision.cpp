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
   
   c = 0;

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

   fAzRange = false;
   fAzMin = 0.;
   fAzMax = 0.;

   if( fNTel == 2 )
   {
      fTel_x.push_back( 0. );    fTel_y.push_back( 0. );     fTel_z.push_back( 0. );
      fTel_x.push_back( 81. );   fTel_y.push_back( -27. );   fTel_z.push_back( 0. );
   }
   else
   {
      fTel_x.assign( fNTel, 0. );
      fTel_y.assign( fNTel, 0. );
      fTel_z.assign( fNTel, 0. );
   }

   fWobbleNorth = 0.;
   fWobbleEast  = 0.;
   fWobbleFromDataTree = false;
   fAstroSource = 0;

   defineHistograms();
}

bool VDataMCComparision::setTelescopeCoordinates( unsigned int itelid, double x, double y, double z )
{
   if( (int)itelid < fNTel && itelid < fTel_x.size() && itelid < fTel_y.size() && itelid < fTel_z.size() )
   {
       cout << "\t setting telescope coordinates " << fNTel << "\t" << itelid << "\t" << x << "\t" << y << "\t" << z << endl;
       fTel_x[itelid] = x;  
       fTel_y[itelid] = y;  
       fTel_z[itelid] = z;  
       return true;
   }
   cout << "error while setting telescope coordinates " << fNTel << "\t" << itelid << "\t" << fTel_x.size() << "\t" << fTel_y.size() << "\t" << fTel_z.size() << endl; 
   return false;
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
   hMSCW = new TH1D( hname, "", 100, -5., vmax );
   hMSCW->SetXTitle( "mean scaled width [deg]" );
   hMSCW->Sumw2();
   hisList->Add( hMSCW );

   if( bBckData ) vmax = 20.;
   else           vmax = 10.;
   sprintf( hname, "hMSCL_%s", fName.c_str() ); 
   hMSCL = new TH1D( hname, "", 100, -5., vmax );
   hMSCL->SetXTitle( "mean scaled length [deg]" );
   hMSCL->Sumw2();
   hisList->Add( hMSCL );

   if( bBckData ) vmax = 20.;
   else           vmax = 10.;
   sprintf( hname, "hMSCWErec_%s", fName.c_str() );
   hMSCWErec = new TH2D( hname, "",  6, -1., 1., 100, -5., vmax );
   hMSCWErec->SetYTitle( "mean scaled width [deg]" );
   hMSCWErec->SetXTitle( "log_{10} energy_{rec} [TeV]" );
   hMSCWErec->Sumw2();
   hisList->Add( hMSCWErec );

   if( bBckData ) vmax = 20.;
   else           vmax = 10.;
   sprintf( hname, "hMSCLErec_%s", fName.c_str() );
   hMSCLErec = new TH2D( hname, "",  6, -1., 1., 100, -5., vmax );
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

      sprintf( hname, "hntubesBNI_%d_%s", i, fName.c_str() );
      hntubesBNI.push_back( new TH1D( hname, "", 100, 0., 200.0) );
      hntubesBNI.back()->SetXTitle( "ntubes" );
      hntubesBNI.back()->Sumw2();
      hTel.push_back( hntubesBNI.back() );
      hisList->Add( hntubesBNI.back() );


      sprintf( hname, "hnsat_%d_%s", i, fName.c_str() );
      hnsat.push_back( new TH1D( hname, "", 100, 0., 100.0) );
      hnsat.back()->SetXTitle( "nsat" );
      hnsat.back()->Sumw2();
      hTel.push_back( hnsat.back() );
      hisList->Add( hnsat.back() );

      sprintf( hname, "hsize_%d_%s", i, fName.c_str() );
      hsize.push_back( new TH1D( hname, "", 80, 2., 5.0) );
      hsize.back()->SetXTitle( "log_{10} size [d.c.]" );
      hsize.back()->Sumw2();
      hTel.push_back( hsize.back() );
      hisList->Add( hsize.back() );

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
   c = new CData( iC, fName == "SIMS" );

   int nentries =  c->fChain->GetEntries();
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
       cout << " Theta2 cut only (<" << theta2_cut << " deg2)" << endl;
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
   double i_UTC = 0.;
   double i_xderot = 0.;
   double i_yderot = 0.;

   int iOldRun = 0;

   double weight = 1.;

   int fInput = 0;
   if( fName == "SIMS" ) fInput = 0;
   else if( fName == "ON" ) fInput = 1;
   else if( fName == "OFF" ) fInput = 2;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// now loop over all events
   for( int i = 0; i < nentries; i++ )
   {
      c->GetEntry( i );

// define astro object (each time a new run starts)
      if( iOldRun != c->runNumber && fWobbleFromDataTree )
      {
         fWobbleNorth = c->WobbleN;
	 fWobbleEast = c->WobbleE;

	 cout << "\t =======================" << endl;
         cout << "\t now at run " << c->runNumber << " (" << c->eventNumber << ")" << endl;

         setTarget( fTargetName, fWobbleNorth, fWobbleEast, c->MJD );

         iOldRun = c->runNumber;
      }

// use only events with full array
      if( c->NImages < fNImages_min ) continue;

// apply az cut to MC (to choose similar AZ range as in Crab runs)
// (STD)     if( fInput == 0 && ( c->MCaz < 100.  || c->MCaz > 140. ) ) continue;

/////////////////////////////////////////////////
// quality cuts
      if( c->EChi2 < 0 ) continue;
      if( c->MSCW < -50. ) continue;
      if( c->MSCL < -50. ) continue;
      for( int j = 0; j < fNTel; j++ )
      {
	 if( c->MSCWT[0] < -50. ) continue;
	 if( c->MSCLT[0] < -50. ) continue;
      }
      if( sqrt( c->Xcore*c->Xcore + c->Ycore*c->Ycore ) > fCoreMax_QC ) continue;

// AZ cut to check azimuth dependence of core distance distributions
      if( fAzRange && (c->Az < fAzMin || c->Az > fAzMax ) ) continue; 

      theta2 = c->theta2;
// get correct theta2 for wobble runs
      if( fInput != 0 && fAstroSource && (fWobbleNorth != 0. || fWobbleEast != 0 ) )
      {
	 i_UTC = VSkyCoordinatesUtilities::getUTC(c->MJD,c->Time);
	 fAstroSource->derotateCoords(i_UTC,c->Xoff,-1.*c->Yoff,i_xderot,i_yderot);
	 i_yderot *= -1.;
	 if( fInput == 2 )
	 {
	     theta2 = (i_yderot-fWobbleNorth)*(i_yderot-fWobbleNorth) + (i_xderot-fWobbleEast)*(i_xderot-fWobbleEast);
         }
	 else
	 {
	     theta2 = (i_yderot+fWobbleNorth)*(i_yderot+fWobbleNorth) + (i_xderot+fWobbleEast)*(i_xderot+fWobbleEast);
         }
      }
      else if( fInput == 0 )
      {
         theta2 = (c->Yoff-c->MCyoff)*(c->Yoff-c->MCyoff) + (c->Xoff-c->MCxoff)*(c->Xoff-c->MCxoff);
      }
      else continue;

// this is only filled for simulations
      if( fInput == 0 && c->MCe0 > 0 )
      {
         hErec->Fill( log10( c->MCe0 ) );
	 if( fSpectralWeight ) weight = fSpectralWeight->getSpectralWeight( c->MCe0 );
	 else                  weight = 1.;
      }

// fill histograms with all cuts applied
//
     
// quality cuts & theta2 cut only
      if( fSingleTelescopeCuts < -1 )
      {
         if( theta2 <= theta2_min || theta2 > theta2_cut ) continue;
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
	     if( c->alpha[fSingleTelescopeCuts-1] > alpha_max ) continue;
         }
	 if( c->dist[fSingleTelescopeCuts-1] < dist_min || c->dist[fSingleTelescopeCuts-1] > dist_max ) continue;
	 if( c->size[fSingleTelescopeCuts-1] && c->length[fSingleTelescopeCuts-1]/c->size[fSingleTelescopeCuts-1] > los_max ) continue;
	 if( c->length[fSingleTelescopeCuts-1] < length_min ||  c->length[fSingleTelescopeCuts-1] > length_max ) continue;
	 if( c->width[fSingleTelescopeCuts-1] < width_min || c->width[fSingleTelescopeCuts-1] > width_max ) continue;    
      }

      if( fSingleTelescopeCuts != -1 || (theta2 >= 0. && c->MSCW < msw_max && c->MSCW > msw_min && c->MSCL > msl_min && c->MSCL < msl_max ) )
      {
	 htheta2->Fill( theta2, weight );
	 hltheta2->Fill( log10(theta2), weight );
	 hYt2->Fill( log10( theta2 ), c->Ycore, weight );
	 hNimages->Fill( c->NImages, weight );
	 hImgSel->Fill( c->ImgSel, weight );
	 if( c->EmissionHeight > 0. ) hEmissionHeight->Fill( c->EmissionHeight, weight );
      }
      if( fSingleTelescopeCuts != -1 || (theta2 >= theta2_min && theta2 < theta2_cut &&  c->MSCL < msl_max && c->MSCL > msl_min ) )
      {
	 hMSCW->Fill( c->MSCW, weight );
	 if( c->Erec > 0. ) hMSCWErec->Fill( log10( c->Erec ), c->MSCW, weight );
	 for( int j = 0; j < fNTel; j++ ) if( c->MSCWT[j] > 0. ) hmscwt[j]->Fill( c->width[j] / c->MSCWT[j], weight );
      }
      if( fSingleTelescopeCuts != -1 || (theta2 >= theta2_min && theta2 < theta2_cut  &&  c->MSCW < msw_max && c->MSCW > msw_min ) )
      {
	 hMSCL->Fill( c->MSCL, weight );
	 for( int j = 0; j < fNTel; j++ ) if( c->MSCLT[j] > 0. ) hmsclt[j]->Fill( c->length[j] / c->MSCLT[j], weight );
	 if( c->Erec > 0. ) hMSCLErec->Fill( log10( c->Erec ), c->MSCL, weight );
      }
       
////////////////////////////////////////////////////////////////////////////////////////////////
// stereo parameters
      if( fSingleTelescopeCuts != -1 ||
          ( theta2 >= theta2_min && theta2 < theta2_cut  && c->MSCW < msw_max && c->MSCW > msw_min && c->MSCL < msl_max && c->MSCL > msl_min ) )
      {
	 hXcore->Fill( c->Xcore, weight );
	 if( fInput == 0 )
	 {
	    hYcore->Fill( c->Ycore , weight );
	    hXYcore->Fill( c->Xcore, c->Ycore, weight );
	    hAzYcore->Fill( c->Az, c->Ycore , weight );
         }
	 else
	 {
	    hYcore->Fill( c->Ycore , weight );
	    hXYcore->Fill( c->Xcore, c->Ycore, weight );
	    hAzYcore->Fill( c->Az, c->Ycore , weight );
         }


///////////////////////////////////////////////////////////////
// core position relative to each telescope
	 for( int j = 0; j < fNTel; j++ )
	 {
	    if( c->ntubes[j] > ntubes_min )
	    {
	       rdist1 = VUtilities::line_point_distance( c->Ycore, -1.*c->Xcore, 0., c->Ze, c->Az, fTel_y[j], -1.*fTel_x[j], fTel_z[j] );
	       hR[j]->Fill( rdist1, weight );
	       hdistR[j]->Fill( rdist1, sqrt( c->cen_x[j]*c->cen_x[j]+c->cen_y[j]*c->cen_y[j]), weight );
            }
	 }

///////////////////////////////////////////////////////////////
// single telescope distributions
	 for( int j = 0; j < fNTel; j++ )
	 {
	    if( c->ntubes[j] > ntubes_min )
	    {
	       hntubes[j]->Fill( c->ntubes[j], weight );
	       hntubesBNI[j]->Fill( c->ntubesBNI[j], weight );
	       if( c->nsat[j] > 0 ) hnsat[j]->Fill( (double)c->nsat[j], weight );
	       hdist[j]->Fill( c->dist[j], weight );
	       if( c->size[j] > 0. ) hsize[j]->Fill( log10( c->size[j] ), weight );
	       if( c->max1[j] > 0. ) hmax1[j]->Fill( log10( c->max1[j] ), weight );
	       if( c->max2[j] > 0. ) hmax2[j]->Fill( log10( c->max2[j] ), weight );
	       if( c->max3[j] > 0. ) hmax3[j]->Fill( log10( c->max3[j] ), weight );
	       hwidth[j]->Fill( c->width[j], weight );
	       hlength[j]->Fill( c->length[j], weight );
	       halpha[j]->Fill( c->alpha[j], weight );
	       if( c->size[j] > 0. ) hlos[j]->Fill( c->length[j] / c->size[j] * 1000., weight );
	       hloss[j]->Fill( c->loss[j], weight );
	       hasym[j]->Fill( c->asym[j], weight );
               if( fInput == 0 )
	       {
		  hcen_x[j]->Fill( c->cen_x[j], weight );
		  hcen_y[j]->Fill( -1.*c->cen_y[j], weight );
               }
	       else
	       {
		  hcen_x[j]->Fill( c->cen_x[j], weight );
		  hcen_y[j]->Fill( c->cen_y[j], weight );
               }
	       hcen_xy[j]->Fill( c->cen_x[j], c->cen_y[j], weight );
	       htgrad_x[j]->Fill( c->tgrad_x[j], weight );
	       hr[j]->Fill( c->R[j], weight );
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

void VDataMCComparision::setTarget( string iTarget, double iWobbleNorth, double iWobbleEast, double iMJD  )
{
   double degrad = 45./atan(1.);

   fWobbleNorth = iWobbleNorth;
   fWobbleEast = iWobbleEast;

   fTargetName = iTarget;

   if( fName == "SIMS" ) return;

   VTargets fTarget;
   double i_ra=0;
   double i_dec=0;
   if( fTarget.selectTargetbyName( iTarget ) )
   {
      i_dec = fTarget.getTargetDec();  // in [rad]
      i_ra  = fTarget.getTargetRA();
   }
   else
   {
      exit( 0 );
   }
   if( iWobbleNorth < -90. ) return;

/////////////////////////////////////////////////////
// precess source coordinates
   VSkyCoordinatesUtilities::precessTarget( iMJD, i_ra, i_dec);

// get wobble offsets in ra,dec
   double i_decDiff =  0.;
   double i_raDiff = 0.;
   if( fWobbleNorth != 0. || fWobbleEast != 0. )
   {
      VSkyCoordinatesUtilities::getWobbleOffsets( fWobbleNorth, fWobbleEast, i_dec*degrad, i_ra*degrad, i_decDiff, i_raDiff ); 
   }

   if( fAstroSource ) delete fAstroSource;
// (GM): old v355 contructor
//   fAstroSource = new VSkyCoordinates( i_ra + i_raDiff/degrad, i_dec + i_decDiff/degrad, i_ra, i_dec  );
   fAstroSource = new VSkyCoordinates();
   fAstroSource->setTelDec_deg( i_dec + i_decDiff/degrad );
   fAstroSource->setTelRA_deg( i_ra + i_raDiff / degrad );

   cout << "\t new source/telescope coordinates: " << (i_ra + i_raDiff/degrad)*degrad << "\t" << (i_dec + i_decDiff/degrad)*degrad;
   cout << "\t" << i_ra*degrad << "\t" << i_dec*degrad << endl;
   cout << "\t (MJD " << iMJD << ", N" << fWobbleNorth << ", E" << fWobbleEast << ", " << i_raDiff << "," << i_decDiff << ")" << endl;
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

