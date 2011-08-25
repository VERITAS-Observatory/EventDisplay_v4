/*! \class VSourceGeometryFitter
    \brief analyse source geometry (position and extension)


    TODO:

    plotter
    LL function
    arcsec calculation
    look at extension

*/

#include "VSourceGeometryFitter.h"

VSourceGeometryFitter::VSourceGeometryFitter()
{
   fDebug = false;

   fAnasumDataFile = "";
   fRunNumber      = -1;
   fHisSkyMap      = 0;
   fXStart         = 0;
   fYStart         = 0;
   setFitterDefaultData();
   setFitter( "Gauss_RadialSymmetric_Chi2" );
   
}

VSourceGeometryFitter::VSourceGeometryFitter( string iAnaSumDataFile, int iRunNumber )
{
   fDebug = false;

   fAnasumDataFile = iAnaSumDataFile;
   fRunNumber      = iRunNumber;
   fHisSkyMap      = 0;
   fXStart         = 0;
   fYStart         = 0;
   if( !openFile( fAnasumDataFile, fRunNumber, 1 ) )
   {
      return;
   }

   setFitterDefaultData();
   setFitter( "Gauss_RadialSymmetric_Chi2" );
   
}

void VSourceGeometryFitter::setFitterDefaultData()
{

   fDefaultFitterData.clear();

// radial symetrical source (Gaussian shape) (Chi2)
   fDefaultFitterData.push_back( new VSourceGeometryFitterData() );
   fDefaultFitterData.back()->fFitterName = "Gauss_RadialSymmetric_Chi2";

   fDefaultFitterData.back()->fParameterName.push_back( "background" );
   fDefaultFitterData.back()->fParameterInitValue.push_back( 30. );
   fDefaultFitterData.back()->fParameterStep.push_back( 1.e-7 );
   fDefaultFitterData.back()->fParameterLowerLimit.push_back( -1.e5 );
   fDefaultFitterData.back()->fParameterUpperLimit.push_back(  1.e5 );

   fDefaultFitterData.back()->fParameterName.push_back( "constant" );
   fDefaultFitterData.back()->fParameterInitValue.push_back( 1.e5 );
   fDefaultFitterData.back()->fParameterStep.push_back( 1.e-7 );
   fDefaultFitterData.back()->fParameterLowerLimit.push_back( 0. );
   fDefaultFitterData.back()->fParameterUpperLimit.push_back( 0. );

   fDefaultFitterData.back()->fParameterName.push_back( "sigmaSource" );
   fDefaultFitterData.back()->fParameterInitValue.push_back( 0.1 );
   fDefaultFitterData.back()->fParameterStep.push_back( 1.e-7 );
   fDefaultFitterData.back()->fParameterLowerLimit.push_back( 1.e-3 );
   fDefaultFitterData.back()->fParameterUpperLimit.push_back( 1.0 );

   fDefaultFitterData.back()->fParameterName.push_back( "Xpos" );
   fDefaultFitterData.back()->fParameterInitValue.push_back( 0. );
   fDefaultFitterData.back()->fParameterStep.push_back( 1.e-7 );
   fDefaultFitterData.back()->fParameterLowerLimit.push_back( -1. );
   fDefaultFitterData.back()->fParameterUpperLimit.push_back(  1. );

   fDefaultFitterData.back()->fParameterName.push_back( "Ypos" );
   fDefaultFitterData.back()->fParameterInitValue.push_back( 0. );
   fDefaultFitterData.back()->fParameterStep.push_back( 1.e-7 );
   fDefaultFitterData.back()->fParameterLowerLimit.push_back( -1. );
   fDefaultFitterData.back()->fParameterUpperLimit.push_back(  1. );

// 2D Normal distribution (LL)
   fDefaultFitterData.push_back( new VSourceGeometryFitterData() );
   fDefaultFitterData.back()->fFitterName = "2DGauss_LL";

   fDefaultFitterData.back()->fParameterName.push_back( "rho" );
   fDefaultFitterData.back()->fParameterInitValue.push_back( 0. );
   fDefaultFitterData.back()->fParameterStep.push_back( 1.e-7 );
   fDefaultFitterData.back()->fParameterLowerLimit.push_back(  0. );
   fDefaultFitterData.back()->fParameterUpperLimit.push_back(  1. );

   fDefaultFitterData.back()->fParameterName.push_back( "Xpos" );
   fDefaultFitterData.back()->fParameterInitValue.push_back( 0. );
   fDefaultFitterData.back()->fParameterStep.push_back( 1.e-7 );
   fDefaultFitterData.back()->fParameterLowerLimit.push_back( -1. );
   fDefaultFitterData.back()->fParameterUpperLimit.push_back(  1. );

   fDefaultFitterData.back()->fParameterName.push_back( "sigmaX" );
   fDefaultFitterData.back()->fParameterInitValue.push_back( 0.1 );
   fDefaultFitterData.back()->fParameterStep.push_back( 1.e-7 );
   fDefaultFitterData.back()->fParameterLowerLimit.push_back( 0. );
   fDefaultFitterData.back()->fParameterUpperLimit.push_back( 1.e2 );

   fDefaultFitterData.back()->fParameterName.push_back( "Ypos" );
   fDefaultFitterData.back()->fParameterInitValue.push_back( 0. );
   fDefaultFitterData.back()->fParameterStep.push_back( 1.e-7 );
   fDefaultFitterData.back()->fParameterLowerLimit.push_back( -1. );
   fDefaultFitterData.back()->fParameterUpperLimit.push_back(  1. );

   fDefaultFitterData.back()->fParameterName.push_back( "sigmaY" );
   fDefaultFitterData.back()->fParameterInitValue.push_back( 0.1 );
   fDefaultFitterData.back()->fParameterStep.push_back( 1.e-7 );
   fDefaultFitterData.back()->fParameterLowerLimit.push_back( 0. );
   fDefaultFitterData.back()->fParameterUpperLimit.push_back( 1.e2 );

}

void VSourceGeometryFitter::help()
{

   cout << "list of available fit functions (select with VSourceGeometryFitter::setFitter() ): " << endl;
   for( unsigned int i = 0; i < fDefaultFitterData.size(); i++ )
   {
      cout << "\t" << fDefaultFitterData[i]->fFitterName << endl;
   }
}

TCanvas* VSourceGeometryFitter::plot( double rmax, double zmin, double zmax, string iPlotMode )
{
   if( !fHisSkyMap ) 
   {
      cout << "\t no sky map set" << endl;
      return 0;
   }
// preliminary
   double xcenter = -1*fXStart;
   double ycenter = -1*fYStart;

// style settings
   default_settings();
   setHistogramPlottingStyle( fHisSkyMap, 1.5 );

// signi
   char hname[800];
   char htitle[800];
   sprintf( hname, "c_skymap_%d", fRunNumber );
   sprintf( htitle, "sky map, run %d", fRunNumber );
   TCanvas *c_sky = new TCanvas( hname, htitle, 10, 10, 400, 400 );
   c_sky->Draw();
   c_sky->SetRightMargin( 0.14 );
   c_sky->SetLeftMargin( 0.11 );
   c_sky->SetGridx( 1 );
   c_sky->SetGridy( 1 );

   fHisSkyMap->SetTitle( "" );

   double x1 = -1.*rmax - xcenter;
   double x2 = rmax - xcenter;
   double y1 = -1.*rmax - ycenter;
   double y2 = rmax - ycenter;
   if( x1 < fHisSkyMap->GetXaxis()->GetXmin() ) x1 = fHisSkyMap->GetXaxis()->GetXmin();
   if( x2 > fHisSkyMap->GetXaxis()->GetXmax() ) x2 = fHisSkyMap->GetXaxis()->GetXmax();
   if( y1 < fHisSkyMap->GetYaxis()->GetXmin() ) y1 = fHisSkyMap->GetYaxis()->GetXmin();
   if( y2 > fHisSkyMap->GetYaxis()->GetXmax() ) y2 = fHisSkyMap->GetYaxis()->GetXmax();
   fHisSkyMap->SetAxisRange( x1, x2, "X" );
   fHisSkyMap->SetAxisRange( y1, y2, "Y" );

   if( zmin > -1000. ) fHisSkyMap->SetMinimum( zmin );
   if( zmax > -1000. ) fHisSkyMap->SetMaximum( zmax );

   fHisSkyMap->Draw( iPlotMode.data() );

// plot reconstructed source geometry
   plotSourceGeometry();

// plot fit result
   plotFitResult();

// return canvas
   return c_sky;
}

void VSourceGeometryFitter::plotFitResult()
{
   if( !fFitter || fFitter->fFitResult_Status < -10 ) return;
// check right number of parameters
   if( fFitter->fFitResult_Parameter.size() != fFitter->fParameterName.size() || fFitter->fFitResult_ParameterError.size() != fFitter->fParameterName.size() ) return;

/////////////////////////////////////////////////////////////////////////////////////////////
// symmetric Gauss fit
   if( fFitter->fFitterName == "Gauss_RadialSymmetric_Chi2" )
   {
      if( fHisSkyMap )
      {
	 char hname[800];
	 char htitle[800];
	 sprintf( hname, "c_skyFitResultmap_%d", fRunNumber );
	 sprintf( htitle, "fit result, run %d", fRunNumber );
	 TCanvas *c_skyFitResult = new TCanvas( hname, htitle, 610, 10, 400, 400 );
	 c_skyFitResult->Draw();
	 c_skyFitResult->SetRightMargin( 0.14 );
	 c_skyFitResult->SetLeftMargin( 0.11 );
	 c_skyFitResult->SetGridx( 1 );
	 c_skyFitResult->SetGridy( 1 );

	 sprintf( hname, "hFitResult_%d", fRunNumber );
	 TH1D *hFitResult = new TH1D( hname, "", 4*(int)(sqrt( fFitter->fParameterUpperLimit[3]*fFitter->fParameterUpperLimit[3]+fFitter->fParameterUpperLimit[4]*fFitter->fParameterUpperLimit[4])/0.025/0.025), 0., sqrt( fFitter->fParameterUpperLimit[3]*fFitter->fParameterUpperLimit[3]+fFitter->fParameterUpperLimit[4]*fFitter->fParameterUpperLimit[4]) );
	 hFitResult->SetXTitle( "#Theta^{2} [deg]" );
	 hFitResult->SetYTitle( "dN/d#Theta^{2}" );
	 setHistogramPlottingStyle( hFitResult, 1, 2. );

// fill theta2 histogram
      
	  double x = 0.;
	  double y = 0.;
	  double t2 = 0.;
	  int nbinsX = fHisSkyMap->GetNbinsX();
	  int nbinsY = fHisSkyMap->GetNbinsY();
	  for( int i = 1; i <= nbinsX; i++ )
	  {
	      x = fHisSkyMap->GetXaxis()->GetBinCenter( i );
	      for( int j = 1; j <= nbinsY; j++ )
	      {
		 y = fHisSkyMap->GetYaxis()->GetBinCenter( j );
// calculate theta2
		 t2 = (x-fFitter->fFitResult_Parameter[3])*(x-fFitter->fFitResult_Parameter[3]) + (y-fFitter->fFitResult_Parameter[4])*(y-fFitter->fFitResult_Parameter[4]);
// fill histogram
		 if( fHisSkyMap->GetBinContent( i, j ) > -90. ) hFitResult->Fill( t2, fHisSkyMap->GetBinContent( i, j ) );
	      }
	   }
// plot histogram
	  hFitResult->Draw();
// plot fit function
	  sprintf( hname, "fFitResult_%d", fRunNumber );
	  sprintf( htitle, "%f + %f*TMath::Exp( -1.*x/ 2. / %f / %f)", fFitter->fFitResult_Parameter[0], fFitter->fFitResult_Parameter[1], fFitter->fFitResult_Parameter[2], fFitter->fFitResult_Parameter[2] );
	  TF1 *fFitResult = new TF1( hname, htitle, hFitResult->GetXaxis()->GetXmin(), hFitResult->GetXaxis()->GetXmax() );
	  fFitResult->SetLineColor( 2 );
	  fFitResult->Draw( "same" );
       }
       
   }
}



TGraph* VSourceGeometryFitter::plotSourceGeometry( int iColor )
{
    if( !fFitter || fFitter->fFitResult_Status < -10 ) return 0;
// check right number of parameters
    if( fFitter->fFitResult_Parameter.size() != fFitter->fParameterName.size() || fFitter->fFitResult_ParameterError.size() != fFitter->fParameterName.size() ) return 0;

/////////////////////////////////////////////////////////////////////////////////////////////
// symmetric Gauss fit
    if( fFitter->fFitterName == "Gauss_RadialSymmetric_Chi2" ) 
    {
// define graph
       TGraphErrors *g = new TGraphErrors( 1 );
       setGraphPlottingStyle( g, iColor, 2., 7, 2. );

       g->SetPoint( 0, fFitter->fFitResult_Parameter[3], fFitter->fFitResult_Parameter[4] );
       g->SetPointError( 0, fFitter->fFitResult_ParameterError[3], fFitter->fFitResult_ParameterError[4] );
       g->Draw( "p" );

       if( fFitter->fFitResult_Parameter[2] > 0. )
       {
	  TEllipse *e = new TEllipse( fFitter->fFitResult_Parameter[3], fFitter->fFitResult_Parameter[4], fFitter->fFitResult_Parameter[2], fFitter->fFitResult_Parameter[2] );
	  e->SetFillStyle( 0 );
	  e->Draw();
       } 

       return g;
    }
/////////////////////////////////////////////////////////////////////////////////////////////
// 2D normal distribution
    else if( fFitter->fFitterName == "2DGauss_LL" )
    {
// define graph
       TGraphErrors *g = new TGraphErrors( 1 );
       setGraphPlottingStyle( g, iColor, 2., 7, 2. );

       g->SetPoint( 0, fFitter->fFitResult_Parameter[1], fFitter->fFitResult_Parameter[3] );
       g->SetPointError( 0, fFitter->fFitResult_ParameterError[1], fFitter->fFitResult_ParameterError[3] );
       g->Draw( "p" );

       return g;
    }

    return 0;
}

bool VSourceGeometryFitter::setFitter( string iDescription )
{
   for( unsigned int i = 0; i < fDefaultFitterData.size(); i++ )
   {
      if( fDefaultFitterData[i]->fFitterName == iDescription )
      {
         fFitter = fDefaultFitterData[i];
         return true;
      }
   }
   fFitter = 0;

   return false;
}

void VSourceGeometryFitter::fitSource( string iHisName, double xStart, double yStart, double xyRange )
{

    fXStart = xStart;
    fYStart = yStart;

    if( !fFitter )
    {
       cout << "VSourceGeometryFitter::fitSource: undefined fitter" << endl;
       return;
    }

// get sky map histogram
    fHisSkyMap = (TH2D*)getHistogram( iHisName, fRunNumber, "skyHistograms" );

    if( !fHisSkyMap )
    {
        cout << "VSourceGeometryFitter::fitSource: histogram not found: " << iHisName << endl;
        return;
    }

//////////////////////////////////////
// define minuit
//////////////////////////////////////
    TFitterMinuit *fSourceGeometryFitter_MINUIT = new TFitterMinuit();

//////////////////////////////////////
// set fit function
//////////////////////////////////////

// radial symmetric Gauss
    VFun_SourceDescription_RadialSymmetricSource_Chi2 fcn_RadialSymmetricSource_Chi2( fHisSkyMap, xStart - xyRange, xStart + xyRange, yStart - xyRange, yStart + xyRange );
    if( fFitter->fFitterName == "Gauss_RadialSymmetric_Chi2" )
    {
       fSourceGeometryFitter_MINUIT->SetMinuitFCN( &fcn_RadialSymmetricSource_Chi2 );
// update parameters
       fFitter->fParameterInitValue[3]  = xStart;
       fFitter->fParameterLowerLimit[3] = xStart - xyRange;
       fFitter->fParameterUpperLimit[3] = xStart + xyRange;
       fFitter->fParameterInitValue[4]  = yStart;
       fFitter->fParameterLowerLimit[4] = yStart - xyRange;
       fFitter->fParameterUpperLimit[4] = yStart + xyRange;
    }

// 2D normal distributions
    VFun_SourceDescription_2DNormal_LL fcn_2DNormal_LL( fHisSkyMap, xStart - xyRange, xStart + xyRange, yStart - xyRange, yStart + xyRange );
    if( fFitter->fFitterName == "2DGauss_LL" )
    {
       fSourceGeometryFitter_MINUIT->SetMinuitFCN( &fcn_2DNormal_LL );
// update parameters
       fFitter->fParameterInitValue[1]  = xStart;
       fFitter->fParameterLowerLimit[1] = xStart - xyRange;
       fFitter->fParameterUpperLimit[1] = xStart + xyRange;
       fFitter->fParameterInitValue[3]  = yStart;
       fFitter->fParameterLowerLimit[3] = yStart - xyRange;
       fFitter->fParameterUpperLimit[3] = yStart + xyRange;
    }
    fSourceGeometryFitter_MINUIT->SetPrintLevel(3);

// set parameters
    for( unsigned int i = 0; i < fFitter->fParameterName.size(); i++ )
    {
        fSourceGeometryFitter_MINUIT->SetParameter( i, fFitter->fParameterName[i].c_str(), fFitter->fParameterInitValue[i], fFitter->fParameterStep[i], fFitter->fParameterLowerLimit[i], fFitter->fParameterUpperLimit[i] );
    }


// start minimizing
// (default is kMigrad)
    fSourceGeometryFitter_MINUIT->CreateMinimizer();

    fFitter->fFitResult_Status = fSourceGeometryFitter_MINUIT->Minimize();
    cout << "Fit status " << fFitter->fFitResult_Status << endl;

// retrieve parameters
    fFitter->fFitResult_Parameter.clear();
    fFitter->fFitResult_ParameterError.clear();
    for( unsigned int i = 0; i < fFitter->fParameterName.size(); i++ )
    {
       fFitter->fFitResult_Parameter.push_back( fSourceGeometryFitter_MINUIT->GetParameter( i ) );
       fFitter->fFitResult_ParameterError.push_back( fSourceGeometryFitter_MINUIT->GetParError( i ) );
    }



// combined error calculation for 5 parameters (see Minuit manual Table 7.1)
//    fMinuit.Command( "SET ERR 6.06" );

}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
     Functions for source position and extension fitting
*/
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


/*
    radial symetrical source (Gaussian shape) (Chi2)

    fit parameters are:   source position, source extension, normalisation

    input:                TH2 histogram with on counts
    return value:         chi2, fit parameters

    units in [deg]
*/

VFun_SourceDescription_RadialSymmetricSource_Chi2::VFun_SourceDescription_RadialSymmetricSource_Chi2( TH2D *iSkymap, double i_xmin, double i_xmax, double i_ymin, double i_ymax )
{
   hSkyMap = iSkymap;
   xmin = i_xmin;
   xmax = i_xmax;
   ymin = i_ymin;
   ymax = i_ymax;
}

/*
    2D normal distribution (LL)

    fit parameters are:   source position, source extension, normalisation

    input:                TH2 histogram with on counts
    return value:         chi2, fit parameters

    units in [deg]
*/
VFun_SourceDescription_2DNormal_LL::VFun_SourceDescription_2DNormal_LL( TH2D *iSkymap, double i_xmin, double i_xmax, double i_ymin, double i_ymax )
{
   hSkyMap = iSkymap;
   xmin = i_xmin;
   xmax = i_xmax;
   ymin = i_ymin;
   ymax = i_ymax;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* \class VSourceGeometryFitterData
   \brief containts fit defaults and results

*/

VSourceGeometryFitterData::VSourceGeometryFitterData()
{
   fFitResult_Status = -99;
}


