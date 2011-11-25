/*! \file VInstrumentResponseFunctionReader

    data class for effective area plotting

    \author Gernot Maier
*/

#include "VInstrumentResponseFunctionReader.h"

VInstrumentResponseFunctionReader::VInstrumentResponseFunctionReader()
{
    fIsZombie = true;
    fDebug = false;

    fFile = "";
    fA_MC = "A_MC";
    fZe = 0.;
    fWoff = 0.;
    fAzbin = 0;
    fIndex = 0.;
    fNoise = 0;
    fPlotOption = "pl";
    fColor = 1;
    fLineStyle = 1;
    fMarkerStyle = 20;
    fLegend = "";

    gEffArea_MC = 0;
    gEffArea_MC_Ratio = 0;
    gEffArea_Rec = 0;
    gEffArea_Rec_Ratio = 0;

    hEmc = 0;
    hEcut = 0;
    hEcut_rec = 0;
    hEcutUW = 0;
    hEcut_recUW = 0;
    hEsys = 0;
    hERecMatrix = 0;
    hEsysMCRelative2D = 0;
    gEnergyResolution = 0;
    gEnergySystematic_Mean = 0;
    gEnergySystematic_Median = 0;
    gAngularResolution = 0;
    gAngularResolution80 = 0;

    initializeIRFData();
}

bool VInstrumentResponseFunctionReader::initializeIRFData()
{
// read angular and core resolution
    fIRF_TreeNames.push_back( "t_angular_resolution" );
    fIRF_TreeNames.push_back( "t_angular_resolution_080p" );
    fIRF_TreeNames.push_back( "t_core_resolution" );
    fIRF_TreeNames.push_back( "t_energy_resolution" );

    for( unsigned int i = 0; i < fIRF_TreeNames.size(); i++ )
    {
      vector< TH2D* > iTemp;
      fIRF_Data.push_back( 0 );
    }

    return true;
}

bool VInstrumentResponseFunctionReader::fillData( string iDataLine, int iDataID )
{
    if( iDataLine.size() == 0 )
    {
       fIsZombie = true;
       return false;
    }
    string temp;

    istringstream is_stream( iDataLine );
    is_stream >> temp;
    if( temp != "*" ) return false;
    is_stream >> temp;
// check set number
    if( atoi( temp.c_str() ) != iDataID ) return false;
// read this line
    is_stream >> fFile;
    is_stream >> temp;
    fZe = atof( temp.c_str() );
    is_stream >> temp;
    fAzbin = atoi( temp.c_str() );
    is_stream >> temp;
    fWoff = atof( temp.c_str() );
    is_stream >> temp;
    fIndex = atof( temp.c_str() );
    is_stream >> temp;
    fNoise = atoi( temp.c_str() );
    is_stream >> fA_MC;

// plotting options
    is_stream >> fPlotOption;
    is_stream >> temp;
    fPlottingColor = atoi( temp.c_str() );
    is_stream >> temp;
    fPlottingLineStyle = atoi( temp.c_str() );
    is_stream >> temp;
    fPlottingMarkerStyle = atoi( temp.c_str() );
    fLegend = is_stream.str();
    fLegend = is_stream.str().substr( is_stream.tellg(), is_stream.str().size() );

    return fillData();
}

bool VInstrumentResponseFunctionReader::fillData( string iFile, double iZe, double iWoff, int iAzBin, double iIndex, int iNoise, string iA_MC )
{
    fFile = iFile;
    fZe = iZe;
    fWoff = iWoff;
    fAzbin = iAzBin;
    fIndex = iIndex;
    fNoise = iNoise;
    fA_MC = iA_MC;

    return fillData();
}

bool VInstrumentResponseFunctionReader::fillData()
{
// read in all the necessary data from the effective area tree

    if( !getDataFromFile() ) 
    {
       fIsZombie = true;
       return false;
    }

// calculate ratio of cut - efficiencies
    if( !calculateCutEfficiencies() ) return false;

    fIsZombie = false;

    return true;
}

/*

    read response functions from CTA file

    see http://www.cta-observatory.org/ctawpcwiki/index.php/WP_MC for documentation

*/
bool VInstrumentResponseFunctionReader::getDataFromCTAFile()
{
    bool bLinX = false;  // energy axis for effective areas

// gamma-ray effective area
    TH1F *h = (TH1F*)gDirectory->Get( "EffectiveArea" );
    if( !h )
    {
       h = (TH1F*)gDirectory->Get( "harea_gamma" );
       if( !h ) return false;
       bLinX = true;
    }
    
    cout << "reading CTA-style file" << endl;

    gEffArea_MC = new TGraphAsymmErrors( 1 );
    setGraphPlottingStyle( gEffArea_MC );
    fillResolutionGraphfromHistogram( h, gEffArea_MC, false, bLinX );

///////////////////////////////////////////////////////////////
// name and axis units are not consistent in the CTA files!!!
///////////////////////////////////////////////////////////////

// energy resolution
    gEnergyResolution = new TGraphErrors( 1 );
// try Paris style file
    h = (TH1F*)gDirectory->Get( "ERes" );
    if( !h )
    {
       h = (TH1F*)gDirectory->Get( "EnResol_RMS" );
    }
    fillResolutionGraphfromHistogram( h, gEnergyResolution, true );
    setGraphPlottingStyle( gEnergyResolution );

// angular resolution
    gAngularResolution = new TGraphErrors( 1 );
    h = (TH1F*)gDirectory->Get( "AngRes" );
// try Paris style file
    if( !h )
    {
       h = (TH1F*)gDirectory->Get( "AngResolution68" );
// arcmin -> deg
       if( h ) 
       {
          h->Scale( 1./60. );
       }
    }
    fillResolutionGraphfromHistogram( h, gAngularResolution, true );   // ignore errors in resolution graph
    setGraphPlottingStyle( gAngularResolution );

    gAngularResolution80 = new TGraphErrors( 1 );
    h = (TH1F*)gDirectory->Get( "AngRes80" );
    if( !h )
    {
       h = (TH1F*)gDirectory->Get( "AngResolution80" );
// arcmin -> deg
       if( h ) 
       {
          h->Scale( 1./60. );
       }
    }
    fillResolutionGraphfromHistogram( h, gAngularResolution80, true );   // ignore errors in resolution graph
    setGraphPlottingStyle( gAngularResolution80 );

    return true;
}

bool VInstrumentResponseFunctionReader::fillResolutionGraphfromHistogram( TH1F *h, TGraphErrors *g, bool bIgnoreErrors )
{
    if( !h || !g ) return false;

    unsigned int z = 0;
    for( int i = 1; i <= h->GetNbinsX(); i++ )
    {
        if( h->GetBinContent( i ) > 0. )
	{
	   g->SetPoint( z, h->GetXaxis()->GetBinCenter( i ), h->GetBinContent( i ) );
	   if( bIgnoreErrors ) g->SetPointError( z, 0., 0. );
	   else                g->SetPointError( z, 0., h->GetBinError( i ) );
           z++;
        }
    }
    return true;
}


bool VInstrumentResponseFunctionReader::fillResolutionGraphfromHistogram( TH1F *h, TGraphAsymmErrors *g, bool bIgnoreErrors, bool bLinXaxis )
{
    if( !h || !g ) return false;

    unsigned int z = 0;
    for( int i = 1; i <= h->GetNbinsX(); i++ )
    {
        if( h->GetBinContent( i ) > 0. )
	{
	   if( !bLinXaxis ) g->SetPoint( z, h->GetXaxis()->GetBinCenter( i ), h->GetBinContent( i ) );
	   else
	   {
	      if( h->GetXaxis()->GetBinCenter( i ) > 0. )
	      {
	         g->SetPoint( z, TMath::Log10( h->GetXaxis()->GetBinCenter( i ) ), h->GetBinContent( i ) );
              }
	      else continue;
           }
	   if( bIgnoreErrors )
	   {
	      g->SetPointEYlow( z, 0. );
	      g->SetPointEYhigh( z, 0 );
           }
	   else
	   {
	      g->SetPointEYlow( z, h->GetBinError( i ) );
	      g->SetPointEYhigh( z, h->GetBinError( i ) );
           }
           z++;
        }
    }
    return true;
}

/*!
 
    read IRF from a root file

    might be a

    - evndisp IRF file (produced with makeEffectiveArea)
    - CTA WP MC response file

*/
bool VInstrumentResponseFunctionReader::getDataFromFile()
{
    if( fDebug ) cout << "VInstrumentResponseFunctionReader::getDataFromFile" << endl;

    TFile *iFile = new TFile( fFile.c_str() );
    if( iFile->IsZombie() ) return false;

///////////////////////////////////////////////////////////////////////////////////////////////////////
// read effective areas
// 
    TTree *t = (TTree*)iFile->Get( "fEffArea" );
// not effective area tree - is this a CTA file?
    if( !t )
    {
// try to read CTA file
       if( !getDataFromCTAFile() )
       {
	  cout << "Error: effective area tree not found" << endl;
	  return false;
       }
       else return true;
    }

// read IRF from a EVNDISP response file 
    CEffArea *c = new CEffArea( t );

    bool bFound = false;
    for( int j = 0; j < c->fChain->GetEntries(); j++ ) 
    {
       c->GetEntry( j );

       if( fDebug ) cout << "VInstrumentResponseFunctionReader::getDataFromFile: reading event " << j << endl;

// ignore all values if there is only one entry in this tree
       if( c->fChain->GetEntries() > 1 )
       {
// azimuth
	   if( fDebug ) cout << "\t AZ: tree entry " << j << "\t az " << c->az << "\t az bin " << fAzbin << endl;
	   if( c->az != fAzbin ) continue;
// spectral index
	   if( fDebug ) cout << "\t Index: " << j << "\t" << c->index << "\t " << fIndex << endl;
	   if( TMath::Abs( c->index - fIndex ) > 0.05 ) continue;
// wobble offset
	   if( fDebug ) cout << "\t Woff: " << j << "\t" << c->Woff<< "\t" << fWoff << endl;
	   if( TMath::Abs( c->Woff - fWoff ) > 0.05 ) continue;
// noise level
	   if( fDebug ) cout << "\t Noise: " << j << "\t" << c->noise<< "\t" << fNoise << endl;
	   if( c->noise != fNoise ) continue;
// zenith angle
	   if( fDebug ) cout << "\t Ze: " << j << "\t" << c->ze << "\t" << fZe << endl;
	   if( TMath::Abs( c->ze - fZe ) > 3. ) continue;
       }
       cout << "\t FOUND EFFECTIVE AREA (entry " << j << ")" << endl;
       bFound = true;

// get effective areas
       if( c->gEffAreaMC )
       {
          gEffArea_MC  = (TGraphAsymmErrors*)c->gEffAreaMC->Clone();
	  setGraphPlottingStyle( gEffArea_MC );
       }
       else if( c->nbins > 0 )
       {
          gEffArea_MC  = new TGraphAsymmErrors( c->nbins );
	  for( int k = 0; k < c->nbins; k++ )
	  {
	     gEffArea_MC->SetPoint( k, c->e0[k], c->eff[k] );
	     gEffArea_MC->SetPointEYlow( k, c->seff_L[k] );
	     gEffArea_MC->SetPointEYhigh( k, c->seff_U[k] );
          }
	  setGraphPlottingStyle( gEffArea_MC );
       }
       else gEffArea_MC = 0;
       if( c->gEffAreaRec )
       {
          gEffArea_Rec = (TGraphAsymmErrors*)c->gEffAreaRec->Clone();
	  setGraphPlottingStyle( gEffArea_Rec );
       }
       else if( c->Rec_nbins > 0 )
       {
           gEffArea_Rec  = new TGraphAsymmErrors( c->Rec_nbins );
	   for( int k = 0; k < c->Rec_nbins; k++ )
	   {
	     gEffArea_Rec->SetPoint( k, c->Rec_e0[k], c->Rec_eff[k] );
	     gEffArea_Rec->SetPointEYlow( k, c->Rec_seff_L[k] );
	     gEffArea_Rec->SetPointEYhigh( k, c->Rec_seff_U[k] );
           }
	   setGraphPlottingStyle( gEffArea_Rec );
       }
       else                 gEffArea_Rec = 0;
// get energy spectra
       if( c->hEmc )
       {
           hEmc = (TH1D*)c->hEmc->Clone();
	   setHistogramPlottingStyle( hEmc );
       }
       if( c->hEcut )
       {
           hEcut = (TH1D*)c->hEcut->Clone();
	   setHistogramPlottingStyle( hEcut );
       }
       if( c->hEcutRec )
       {
           hEcut_rec = (TH1D*)c->hEcutRec->Clone();
	   setHistogramPlottingStyle( hEcut_rec );
	   hEcut_rec->SetMarkerStyle( hEcut->GetMarkerStyle()+4 );
       }
       if( c->hEcutUW )
       {
           hEcutUW = (TH1D*)c->hEcutUW->Clone();
	   setHistogramPlottingStyle( hEcutUW );
       }
       if( c->hEcutRecUW )
       {
           hEcut_recUW = (TH1D*)c->hEcutRecUW->Clone();
	   setHistogramPlottingStyle( hEcut_recUW );
	   hEcut_recUW->SetMarkerStyle( hEcutUW->GetMarkerStyle()+4 );
       }
// get energy reconstruction matrix
       hERecMatrix = (TH2D*)c->hEmcCut;
// get error in energy reconstruction
       hEsys = (TH2D*)c->hEsys2D;
// erec/emc
       hEsysMCRelative2D = (TH2D*)c->hEsysMCRelative2D;
// get energy resolution
//       getEnergyResolutionPlot( (TProfile*)c->hEsysMCRelative );
       getEnergyResolutionPlot( (TProfile*)c->hEsysRec );
       setGraphPlottingStyle( gEnergyResolution );
// get energy bias
       gEnergySystematic_Mean = get_Profile_from_TH2D( (TH2D*)c->hEsys2D, 0, "mean", 1, -10. );
       setGraphPlottingStyle( gEnergySystematic_Mean );
       gEnergySystematic_Median = get_Profile_from_TH2D( (TH2D*)c->hEsys2D, 0, "median", 1, -10. );
       setGraphPlottingStyle( gEnergySystematic_Median ); 
// get cut efficiencies
       if( c->hhEcutTrigger )              hCutEfficiency.push_back( (TH1D*)c->hhEcutTrigger->Clone() );
       else                                hCutEfficiency.push_back( 0 );
       if( c->hhEcutFiducialArea )         hCutEfficiency.push_back( (TH1D*)c->hhEcutFiducialArea->Clone() );
       else                                hCutEfficiency.push_back( 0 );
       if( c->hhEcutStereoQuality )        hCutEfficiency.push_back( (TH1D*)c->hhEcutStereoQuality->Clone() );
       else                                hCutEfficiency.push_back( 0 );
       if( c->hhEcutTelType )              hCutEfficiency.push_back( (TH1D*)c->hhEcutTelType->Clone() );
       else                                hCutEfficiency.push_back( 0 );
       if( c->hhEcutDirection )            hCutEfficiency.push_back( (TH1D*)c->hhEcutDirection->Clone() );
       else                                hCutEfficiency.push_back( 0 );
       if( c->hhEcutGammaHadron )          hCutEfficiency.push_back( (TH1D*)c->hhEcutGammaHadron->Clone() );
       else                                hCutEfficiency.push_back( 0 );
       if( c->hhEcutEnergyReconstruction ) hCutEfficiency.push_back( (TH1D*)c->hhEcutEnergyReconstruction->Clone() );
       else                                hCutEfficiency.push_back( 0 );
       for( unsigned int i = 0; i < hCutEfficiency.size(); i++ ) setHistogramPlottingStyle( hCutEfficiency[i], i+1, 1., 1.5, fPlottingMarkerStyle );

       break;
    }

//////////////////////////////////////////////////////////////
// read resolution files
   
    for( unsigned int i = 0; i < fIRF_TreeNames.size(); i++ )
    {
       cout << "reading IRF " << fIRF_TreeNames[i].c_str() << endl;
       TTree *t_a = (TTree*)iFile->Get( fIRF_TreeNames[i].c_str() );
       fIRF_Data[i] = getIRFFromFile( t_a, i );
       if( !fIRF_Data[i] ) cout << " ...not found" << endl;
       else
       {
          fIRF_Data[i]->setPlottingStyle( getPlottingColor(), getPlottingLineWidth(), getPlottingMarkerStyle(), getPlottingMarkerSize(), getPlottingFillStyle(), getPlottingLineStyle() );
          cout << " ...found" << endl;
	  bFound = true;
       }
    }

    if( !bFound ) return false;

    return true;
}
    


VInstrumentResponseFunctionData* VInstrumentResponseFunctionReader::getIRFFromFile( TTree* t, unsigned int iIRFIndex )
{
    if( !t ) return 0;

    VInstrumentResponseFunctionData *c = new VInstrumentResponseFunctionData();
    TBranch *br = t->GetBranch( "IRF" );
    br->SetAddress( &c );

    for( int j = 0; j < t->GetEntries(); j++ ) 
    {
       t->GetEntry( j );

       if( fDebug ) cout << "VInstrumentResponseFunctionReader::getDataFromFile (angular resolution): reading event " << j << endl;

// check that there is data for this tree entry
       if( !c ) continue;

// ignore all values if there is only one entry in this tree
       if( t->GetEntries() > 1 )
       {
// azimuth
	   if( fDebug ) cout << "AZ: " << j << "\t" << c->fAz_bin << "\t" << fAzbin << endl;
	   if( c->fAz_bin != fAzbin ) continue;
// spectral index
           if( fDebug ) cout << "Index: " << j << "\t" << c->fSpectralIndex << "\t" << fIndex << endl;
	   if( TMath::Abs( c->fSpectralIndex - fIndex ) > 0.05 ) continue;
// wobble offset
           if( fDebug ) cout << "Woff: " << j << "\t" << c->fWobble << "\t" << fWoff << endl;
	   if( TMath::Abs( c->fWobble - fWoff ) > 0.05 ) continue;
// noise level
	   if( fDebug ) cout << "Noise: " << j << "\t" << c->fNoise << "\t" << fNoise << endl;
	   if( c->fNoise != fNoise ) continue;
// zenith angle
	   if( fDebug ) cout << "Ze: " << j << "\t" << c->fZe << "\t" << fZe << endl;
	   if( TMath::Abs( c->fZe - fZe ) > 3. ) continue; 
       }
       if( c && c->fResolutionGraph.size() > 0 )
       {
          for( unsigned int r = 0; r < c->fResolutionGraph.size(); r++ )
	  {
	     if( c->fResolutionGraph[r] ) setGraphPlottingStyle( c->fResolutionGraph[r] );
          }
       }

       return (VInstrumentResponseFunctionData*)c->Clone();

    }

    return 0;
}


void VInstrumentResponseFunctionReader::getEnergyResolutionPlot( TProfile *iP, int i_rebin, double iMinEnergy )
{
    if( !iP )
    {
       gEnergyResolution = 0;
       return;
    }

    iP->Rebin( i_rebin );

    gEnergyResolution = new TGraphErrors( 1 );
    gEnergyResolution->SetMarkerStyle( 20 );
    gEnergyResolution->SetMarkerSize( 2 );
    gEnergyResolution->SetLineWidth( 2 );
    gEnergyResolution->SetTitle( "" );
    gEnergyResolution->SetName( "" );
    setGraphPlottingStyle( gEnergyResolution );

    string iErrorOption = iP->GetErrorOption();

    int zz = 0;
    for( int b = 1; b <= iP->GetNbinsX(); b++ )
    {
        if( iP->GetBinEntries( b ) > 3. )
        {
            if( iP->GetXaxis()->GetBinCenter( b ) < iMinEnergy ) continue;
            if( iErrorOption == "s" )
            {
                gEnergyResolution->SetPoint( zz, iP->GetXaxis()->GetBinCenter( b ), iP->GetBinError(b) );
                if( iP->GetBinEntries(b) > 0. )
		{
		    gEnergyResolution->SetPointError( zz, 0., iP->GetBinError(b)/sqrt(iP->GetBinEntries(b)-1. ) );
                }
                else                            gEnergyResolution->SetPointError( zz, 0., 0. );
            }
            else
            {
                gEnergyResolution->SetPoint( zz, iP->GetXaxis()->GetBinCenter( b ), iP->GetBinError(b)*sqrt(iP->GetBinEntries(b)-1.) );
                gEnergyResolution->SetPointError( zz, 0., iP->GetBinError(b) );
            }
            zz++;
        }
    }
    return;
}


bool VInstrumentResponseFunctionReader::calculateCutEfficiencies()
{
    char hname[200];
// create histograms
    for( unsigned int i = 0; i < hCutEfficiency.size(); i++ )
    {
       if( hCutEfficiency[i] )
       {
          sprintf( hname, "%s_R", hCutEfficiency[i]->GetName() );
	  hCutEfficiencyRelativePlots.push_back( (TH1D*)hCutEfficiency[i]->Clone( hname ) );
	  hCutEfficiencyRelativePlots.back()->Reset();
       }
       else hCutEfficiencyRelativePlots.push_back( 0 );
    }
// calculate relative plots
    for( unsigned int i = 0; i < hCutEfficiencyRelativePlots.size(); i++ )
    {
       if( hCutEfficiencyRelativePlots[i] ) hCutEfficiencyRelativePlots[i]->Divide( hCutEfficiency[i], hCutEfficiency[0] );
    }       

    return true;
}

bool VInstrumentResponseFunctionReader::calculateEffectiveAreaRatios( TGraphAsymmErrors *g0 )
{
    if( !g0 ) return false;

    gEffArea_MC_Ratio  = calculateEffectiveAreaRatios( g0, gEffArea_MC );
    gEffArea_Rec_Ratio = calculateEffectiveAreaRatios( g0, gEffArea_Rec );

    return true;
}

TGraphAsymmErrors* VInstrumentResponseFunctionReader::calculateEffectiveAreaRatios( TGraphAsymmErrors *g0, TGraphAsymmErrors *g1 )
{
    if( !g0 || !g1 ) return 0;

    TGraphAsymmErrors *g = new TGraphAsymmErrors( 1 );

    double e = 0.;
    int z = 0;

    double x0 = 0.;
    double y0 = 0.;
    double y0_U = 0.;
    double y0_L = 0.;

    double x1 = 0.;
    double y1 = 0.;
    double y1_U = 0.;
    double y1_L = 0.;

    for( int f = 0; f < g1->GetN(); f++ )
    {
	g1->GetPoint( f, x1, y1 );
	y1_U = g1->GetErrorYhigh( f );
	y1_L = g1->GetErrorYlow( f );

	for( int k = 0; k < g0->GetN(); k++ )
	{
//	   g0->GetPoint( k, x0, y0 );
           x0 = x1;
	   y0 = g0->Eval( x0 );
	   y0_U = g0->GetErrorYhigh( k );
	   y0_L = g0->GetErrorYlow( k );

	   if( y0 > 0. )
	   {
	      if( TMath::Abs( x0 - x1 ) < 0.001 )
	      {
	          g->SetPoint( z, x0, y1/y0 );

		  e = y1_U*y1_U/y0/y0 + y1*y1/y0/y0/y0/y0 * y0_U*y0_U;
		  g->SetPointEYhigh( z, sqrt( e ) );
// (Preli)
                  g->SetPointEYhigh( z, 0. );

		  e = y1_L*y1_L/y0/y0 + y1*y1/y0/y0/y0/y0 * y0_L*y0_L;
		  g->SetPointEYlow( z, sqrt( e ) );
// (Preli)
                  g->SetPointEYlow( z, 0. );

		  z++;
               }
            }
        }
    }
    setGraphPlottingStyle( g );

    return g;
}

bool VInstrumentResponseFunctionReader::fillResolutionHistogram( TH1F *h, string iContainmentRadius, string iResolutionTreeName )
{
    if( !h ) return false;

    if( iContainmentRadius != "68" ) iResolutionTreeName += "_0" + iContainmentRadius + "p";

    for( unsigned int j = 0; j < fIRF_TreeNames.size(); j++ )
    {
	if( fIRF_TreeNames[j] == iResolutionTreeName )
	{
	     if( j < fIRF_Data.size() && fIRF_Data[j] )
	     {
// try to get EVNDISP resolution graph
		 TGraphErrors *g = fIRF_Data[j]->fResolutionGraph[VInstrumentResponseFunctionData::E_DIFF];
		 if( iResolutionTreeName == "t_energy_resolution" )
		 {
		    g = gEnergyResolution;
                 }
		 if( g )
		 {
		    double x = 0.;
		    double y = 0.;
		    for( int i = 0; i < g->GetN(); i++ )
		    {
		       g->GetPoint( i, x, y );
		       if( y > 0. ) h->SetBinContent( h->FindBin( x ), y ); 
                    }
		}
            }
       }
   }

   return true;
}

bool VInstrumentResponseFunctionReader::fillEffectiveAreasHistograms( TH1F *hEffRec, string iContainmentRadius, TH1F *hEff_MC )
{
    if( !hEffRec ) return false;

    if( iContainmentRadius.size() > 0 )
    {
       cout << "VInstrumentResponseFunctionReader::fillEffectiveAreasHistograms() warning: " << endl;
       cout << "\t assuming that effective areas are calculated using a 80% containment on direction" << endl;
    }

    if( gEffArea_Rec )
    {
	double x = 0.;
	double y = 0.;
	for( int i = 0; i < gEffArea_Rec->GetN(); i++ )
	{
	   gEffArea_Rec->GetPoint( i, x, y );
	   if( y > 0. )
	   {
	      hEffRec->SetBinContent( hEffRec->FindBin( x ), y ); 
	      hEffRec->SetBinError( hEffRec->FindBin( x ), 0.5*(gEffArea_Rec->GetErrorYlow(i)+gEffArea_Rec->GetErrorYhigh(i)) );
           }	      
	}
    }
    else 
    {
       cout << "VInstrumentResponseFunctionReader::fillEffectiveAreasHistograms() warning: " << endl;
       cout << "\t no effective are graph found" << endl;
       return false;
    }
    if( gEffArea_MC && hEff_MC )
    {
	double x = 0.;
	double y = 0.;
	for( int i = 0; i < gEffArea_MC->GetN(); i++ )
	{
	   gEffArea_MC->GetPoint( i, x, y );
	   if( y > 0. )
	   {
	      hEff_MC->SetBinContent( hEff_MC->FindBin( x ), y ); 
	      hEff_MC->SetBinError( hEff_MC->FindBin( x ), 0.5*(gEffArea_MC->GetErrorYlow(i)+gEffArea_MC->GetErrorYhigh(i)) );
           }	      
        }
    }
    
    return true;
}

