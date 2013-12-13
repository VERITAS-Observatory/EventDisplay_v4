/*! \class VModel3D
     \brief VModel3D class for 3D-reconstruction of showers
            based on Lemoine-Goumard et al. 2006
            adapted by J. Grube and G. Gyuk
*/

#include "VModel3D.h"

VModel3D::VModel3D()
{
  fData3D = new VModel3DData();
  fModel3DParameters = new VModel3DParameters();
  fEmissionHeightCalculator = new VEmissionHeightCalculator(); 
  fModel3DFn = new VModel3DFn();
  fModelLnL = new VModelLnL( fData3D, fModel3DFn );

  fInitialized3D = false;
  fData3D->fexNoise3D = 0.4; // HARD-WIRED
  fData3D->fDCPE = 5.62; // HARD-WIRED (need implementation)
  fData3D->fDebug3D = false; // set true for debug mode
}

VModel3D::~VModel3D()
{
  delete fData3D;
  delete fModel3DFn;
}

void VModel3D::doModel3D()
{
  //// initialized only at first call in the analysis run ////
  if( !fInitialized3D ) {
    initOutput(); //TEST
    initModel3DTree(); // initialize the output tree
    vector<unsigned int> iNpix3D;
    iNpix3D.resize( fData->getNTel(), 0 );
    for (unsigned int i = 0; i < fData->getNTel(); i++) {
      iNpix3D[i] = fData->getDetectorGeo()->getX( i ).size();
    }
    fData3D->initModel3D( fData->getNTel(), iNpix3D ); // initialize data
    readLnLTable();    // read LnL lookup table                      
    getDetector();     // get detector configuration
    fInitialized3D = true;
  }
  // return if triggered only events are written to output
  ///if( !fReader->hasArrayTrigger() ) return;
  /// if( getRunParameter()->fWriteTriggerOnly && !fReader->hasArrayTrigger() ) return;

  fData3D->initEventModel3D();  /// initialize event
  if( fData3D->fDebug3D ) cout<<"--- event "<< fData->getShowerParameters()->eventNumber <<" ---"<<endl; 
  /// reconstruction quality check ///
  if( fData->getShowerParameters()->fShowerNumImages[0] < 2 ) {
    if( fData3D->fDebug3D ) cout<<"doModel3D: NImages < 2"<<endl;
    writeParameters3D();
    return;
  }
  /// setup Model3D ///
  calcPointing();         // setup the pointing 
  calcCoord();            // convert coordinates
  calcPvector();          // calc pixel line of sights
  calcStartParameters();  // calc Model3D start parameters 
  if( ! fData3D->fGoodEvent3D ) {
    if( fData3D->fDebug3D ) cout<<"doModel3D: bad start parameters"<<endl;
    writeParameters3D();
    return;
  }
  /// run Model3D analysis ///
  setSelectedPixels();    // select pixels used in Model3D fit
  startModel3D();         // model with start values, proceed with fit or not
  if( ! fData3D->fGoodEvent3D ) {
    if( fData3D->fDebug3D ) cout<<"doModel3D: bad initial GOF"<<endl;
    writeParameters3D();
    return;
  }
  doFit();
  writeParameters3D();
  return;
}

/////////////////////////////////////////////////////////////////////

void VModel3D::initOutput()
{
  // check if root outputfile exist
  if( fOutputfile != 0 ) return;

  if( fRunPar->foutputfileName != "-1" ) {
      printf("Model3D: attempt to create file\n");
      char i_textTitle[300];
      sprintf( i_textTitle, "VERSION %d", getRunParameter()->getEVNDISP_TREE_VERSION() );
      fOutputfile = new TFile( fRunPar->foutputfileName.c_str(), "RECREATE", i_textTitle );
    }
}

void VModel3D::initModel3DTree()
{
  char i_text[300];
  char i_textTitle[300];
  sprintf( i_text, "model3Dpars" );
  // tree versioning numbers used in mscw_energy
  sprintf( i_textTitle, "Model3D Parameters (VERSION %d)\n", getRunParameter()->getEVNDISP_TREE_VERSION() );
  fModel3DParameters->initTree( i_text, i_textTitle );
}

void VModel3D::terminate()
{
  getModel3DParameters()->getTree()->Write();
}

void VModel3D::createLnLTable()
{
  double pixel_var = 1.0; //need to create tables for many noise levels?
  double pixel_nsb = 0.;
  fModelLnL->createLnLTable( pixel_var, pixel_nsb );
}

void VModel3D::readLnLTable()
{
  string LnLTableFile = fData->getRunParameter()->fLnLTableFile;
  fModelLnL->readLnLTable( LnLTableFile );
}

void VModel3D::getDetector()
{
  /// get telescope locations on ground ///
  for (unsigned int iTel = 0; iTel < fData->getNTel(); iTel++) {

    fData3D->flocTel3D[0][iTel] = fData->getDetectorGeo()->getTelXpos()[iTel];
    fData3D->flocTel3D[1][iTel] = fData->getDetectorGeo()->getTelYpos()[iTel];
    fData3D->flocTel3D[2][iTel] = fData->getDetectorGeo()->getTelZpos()[iTel];

    // fMarea3D[iTel] = getDetectorGeo()->getMirrorArea()[iTel];
    fData3D->fMarea3D[iTel] = 111.; // FIX: Hard-wired VERITAS mirror area (m^2)
    //// use full pixel diameter, or effective diameter?? ////
    double pwid = 0.148 * (TMath::Pi()/180.); // Hard-wired VERITAS PMT diameter 
    ///double pwid = 0.131 * (TMath::Pi()/180.); // Hard-wired VERITAS PMT diameter 

    for( unsigned int j = 0; j < fData3D->fomegapix3D[iTel].size(); j++ ) {
      ///// radius value in config file depends on collection efficiency ///
      //    //pwid = getDetectorGeo()->getTubeRadius( iTel )[j];
      ///// FIX !!!!!
      fData3D->fomegapix3D[iTel][j] = TMath::Pi() * (pwid/2.)*(pwid/2.);
    }
  }

  //// for shower max estimation from Hillas reconstruction ///
  const unsigned int ntel = fData->getNTel();
  double TelX[ntel];
  double TelY[ntel];
  double TelZ[ntel];
  for (unsigned int iTel = 0; iTel < ntel; iTel++) {
    TelX[iTel] = fData3D->flocTel3D[0][iTel];
    TelY[iTel] = fData3D->flocTel3D[1][iTel];
    TelZ[iTel] = fData3D->flocTel3D[2][iTel];
  }
  fEmissionHeightCalculator->setTelescopePositions( ntel, TelX, TelY, TelZ );

}

void VModel3D::calcPointing()
{
  /// elevation and azimuth of telescopes (in ground coordinates) ///
  /// get mean pointing from valid telescopes ////

  for (unsigned int i = 0; i < fData->getNTel(); i++) {
    fData3D->fTze3D[i] = fData->getShowerParameters()->fTelElevation[i];
    fData3D->fTaz3D[i] = fData->getShowerParameters()->fTelAzimuth[i];
  }

  unsigned int Npoint = 0; // number of telescopes with non-zero pointing
  double mTze = 0;  // mean zenith of telescopes (non-zero pointing)
  double mTaz = 0;  // mean azimuth of telescopes (non-zero pointing)

  for (unsigned int i = 0; i < fData->getNTel(); i++) {
    if( fData3D->fTze3D[i] != 0 && fData3D->fTaz3D[i] != 0 ) {
      Npoint += 1;
      mTze = mTze + fData3D->fTze3D[i]; 
      mTaz = mTaz + fData3D->fTaz3D[i]; 
    }
  }

  if( Npoint > 0 ) mTze = mTze / (double)Npoint;
  if( Npoint > 0 ) mTaz = mTaz / (double)Npoint;

  mTze = ( 90. - mTze ) * (TMath::Pi()/180.); // zenith angle in radians
  mTaz = mTaz * (TMath::Pi()/180.); // az in radians

  //// from spherical to cartesian (ground coordinates) ////
  fData3D->fT3D[0] = sin( mTze ) * sin( mTaz );
  fData3D->fT3D[1] = sin( mTze ) * cos( mTaz );
  fData3D->fT3D[2] = cos( mTze );
}

void VModel3D::calcCoord()
{
  /// telescope parameters ///

  /// sky unit base vectors in ground coordinate frame (base) ///
  /////// z (Same as T ) //////////////
  fData3D->fzsg3D[0] = fData3D->fT3D[0];
  fData3D->fzsg3D[1] = fData3D->fT3D[1];
  fData3D->fzsg3D[2] = fData3D->fT3D[2];
  //// should already be normalized ////
  fData3D->norm3D( fData3D->fzsg3D[0], fData3D->fzsg3D[1], fData3D->fzsg3D[2] ); 

  /////// x /////////////
  fData3D->cross3D( fData3D->fzg3D[0], fData3D->fzg3D[1], fData3D->fzg3D[2], fData3D->fT3D[0], fData3D->fT3D[1], fData3D->fT3D[2], fData3D->fxsg3D[0], fData3D->fxsg3D[1], fData3D->fxsg3D[2] );
  fData3D->norm3D( fData3D->fxsg3D[0], fData3D->fxsg3D[1], fData3D->fxsg3D[2] );

  /////// y /////////////
  fData3D->cross3D( fData3D->fzsg3D[0], fData3D->fzsg3D[1], fData3D->fzsg3D[2], fData3D->fxsg3D[0], fData3D->fxsg3D[1], fData3D->fxsg3D[2], fData3D->fysg3D[0], fData3D->fysg3D[1], fData3D->fysg3D[2] ); 
  fData3D->norm3D( fData3D->fysg3D[0], fData3D->fysg3D[1], fData3D->fysg3D[2] );
  ////// swap Y sign /////
  fData3D->fysg3D[0] = 0. - fData3D->fysg3D[0];
  fData3D->fysg3D[1] = 0. - fData3D->fysg3D[1];
  fData3D->fysg3D[2] = 0. - fData3D->fysg3D[2];

}

void VModel3D::calcPvector()
{
  //// get p-hat vector (line of sight) for each telescope and pixel /////
  for (unsigned int iTel = 0; iTel < fData3D->fNTel3D; iTel++) {
    for( unsigned int iPix = 0; iPix < fData3D->fNpix3D[iTel]; iPix++ ) {

      //// add pixel "p" vector to "T" vector in ground coordinates ////
      double tp[3]; // temp p vector
      //// convert from deg to rad ////
      tp[0] = fData->getDetectorGeo()->getX( iTel )[iPix] * (TMath::Pi()/180.);
      tp[1] = fData->getDetectorGeo()->getY( iTel )[iPix] * (TMath::Pi()/180.);
      tp[2] = 0;
      //// multiply by base vectors to be in ground system ////
      double tpx[3];
      double tpy[3];
      double tpz[3];
      tpx[0] = tp[0] * fData3D->fxsg3D[0];
      tpx[1] = tp[0] * fData3D->fxsg3D[1];
      tpx[2] = tp[0] * fData3D->fxsg3D[2];
      tpy[0] = tp[1] * fData3D->fysg3D[0];
      tpy[1] = tp[1] * fData3D->fysg3D[1];
      tpy[2] = tp[1] * fData3D->fysg3D[2];
      tpz[0] = tp[2] * fData3D->fzsg3D[0];
      tpz[1] = tp[2] * fData3D->fzsg3D[1];
      tpz[2] = tp[2] * fData3D->fzsg3D[2];
      
      //// add to the telescope vector ////
      fData3D->fpX3D[iTel][iPix] = fData3D->fT3D[0] + tpx[0] + tpy[0] + tpz[0];
      fData3D->fpY3D[iTel][iPix] = fData3D->fT3D[1] + tpx[1] + tpy[1] + tpz[1];
      fData3D->fpZ3D[iTel][iPix] = fData3D->fT3D[2] + tpx[2] + tpy[2] + tpz[2];

      fData3D->norm3D( fData3D->fpX3D[iTel][iPix], fData3D->fpY3D[iTel][iPix], fData3D->fpZ3D[iTel][iPix] );

      //// angle between pixel center and telescope axis (in ground frame) ////
      fData3D->fcosptheta3D[iTel][iPix] = fData3D->dot3D( fData3D->fpX3D[iTel][iPix], fData3D->fpY3D[iTel][iPix], fData3D->fpZ3D[iTel][iPix], fData3D->fT3D[0], fData3D->fT3D[1], fData3D->fT3D[2] );

    }
  }
}

void VModel3D::calcStartParameters()
{
  /////// Get model starting point ///////////
  /////// use reconstruction method 0 as default ////
  fData3D->fStartXcore3D = fData->getShowerParameters()->fShowerXcore[0];
  fData3D->fStartYcore3D = fData->getShowerParameters()->fShowerYcore[0];

  double corecut = 9999;
  if( fabs(fData3D->fStartXcore3D) >= corecut || fabs(fData3D->fStartYcore3D) >= corecut ) fData3D->fGoodEvent3D = false;

  //// shower direction (sky to ground coordinates) ////
  //// multiply by base vectors to be in ground system ////
  double ts[3]; // temp s vector
  ts[0] = fData->getShowerParameters()->fShower_Xoffset[0];
  ts[1] = fData->getShowerParameters()->fShower_Yoffset[0];
  ts[2] = 0;
  if( ts[0] == 0 && ts[1] == 0 ) fData3D->fGoodEvent3D = false;
  ts[1] = 0. - ts[1];   // swap Y sign
  ts[0] = ts[0] * (TMath::Pi()/180.); // in radians
  ts[1] = ts[1] * (TMath::Pi()/180.); // in radians
  ts[2] = ts[2] * (TMath::Pi()/180.); // in radians
  double tsx[3];
  double tsy[3];
  double tsz[3];
  tsx[0] = ts[0] * fData3D->fxsg3D[0];
  tsx[1] = ts[0] * fData3D->fxsg3D[1];
  tsx[2] = ts[0] * fData3D->fxsg3D[2];
  tsy[0] = ts[1] * fData3D->fysg3D[0];
  tsy[1] = ts[1] * fData3D->fysg3D[1];
  tsy[2] = ts[1] * fData3D->fysg3D[2];
  tsz[0] = ts[2] * fData3D->fzsg3D[0];
  tsz[1] = ts[2] * fData3D->fzsg3D[1];
  tsz[2] = ts[2] * fData3D->fzsg3D[2];
  //// add to the telescope vector ////
  ts[0] = fData3D->fT3D[0] + tsx[0] + tsy[0] + tsz[0];
  ts[1] = fData3D->fT3D[1] + tsx[1] + tsy[1] + tsz[1];
  ts[2] = fData3D->fT3D[2] + tsx[2] + tsy[2] + tsz[2];
  //// normalize s ////
  fData3D->norm3D( ts[0], ts[1], ts[2] );
  //// get shower spherical parameters (ground coordinates) ////
  fData3D->fStartSaz3D = atan2( ts[1], ts[0] ) * (180./TMath::Pi()); // in deg
  fData3D->fStartSel3D = 90. - ( acos( ts[2] ) *(180./TMath::Pi()) );// in deg
  //// calculated shower max ////
  const unsigned int ntel = fData->getNTel();
  double cen_x[ntel];
  double cen_y[ntel];
  double dist[ntel];
  double size[ntel];
  double rzero[ntel];
  double telAz[ntel];
  double telEl[ntel];
  double width[ntel];  // for 3D width estimate
  for (unsigned int iTel = 0; iTel < ntel; iTel++) {
    initializeDataReader(); // inherited from VEvndispData
    setTelID(iTel);         // inherited from VEvndispData
    cen_x[iTel] = fData->getImageParameters()->cen_x;
    cen_y[iTel] = fData->getImageParameters()->cen_y;
    dist[iTel] = fData->getImageParameters()->dist;
    size[iTel] = fData->getImageParameters()->size;
    rzero[iTel] = 0;        // blank for height calculation
    telAz[iTel] = fData->getShowerParameters()->fTelAzimuth[iTel];
    telEl[iTel] = fData->getShowerParameters()->fTelElevation[iTel];
    width[iTel] = fData->getImageParameters()->width;
  }
  fEmissionHeightCalculator->getEmissionHeight( cen_x, cen_y, dist, size, rzero, telAz, telEl );
  fData3D->fStartSmax3D = fEmissionHeightCalculator->getMeanEmissionHeight();

  fData3D->fStartsigmaL3D = 3.;  // 3DLength: use fixed starting value of 3 km ///

  /// 3DWidth estimate ///
  double iWidthWeight = 0;
  double iWidthWeightTemp = 0;
  double iWidth = 0;
  for( unsigned int i = 0; i < ntel; i++ ) {
    if( size[i] > 0 ) {
      iWidthWeightTemp = log10( size[i] );
      iWidthWeight += iWidthWeightTemp;
      iWidth += iWidthWeightTemp * fData3D->fStartSmax3D * tan( width[i] / (45./atan(1.)) );;
    }
  }
  if( iWidthWeight > 0. ) fData3D->fStartsigmaT3D = iWidth / iWidthWeight;
  else fData3D->fStartsigmaT3D = 0;
  fData3D->fStartsigmaT3D *= 1000.; // from km to m

  //// log(Nc) estimate: use rough scaling
  double TotSize = 0; 
  unsigned int TotTel = 0;
  for (unsigned int i = 0; i < fData3D->fNTel3D; i++) {
    if( size[i] > 0 ) {
      TotSize += size[i];
      TotTel += 1;
    }
  }
  if( TotSize > 0 ) TotSize = ((double)fData3D->fNTel3D/(double)TotTel)*TotSize;
  //  fData3D->fStartNc3D = 1.2e3 * TotSize * pow( cos( ZenRad ), 0.46 );
  fData3D->fStartNc3D = 370 * TotSize * 0.97179; //Nc
  fData3D->fStartNc3D = log( fData3D->fStartNc3D ); //log(Nc)

  //// sanity cuts on starting values ////
  if( fData3D->fStartSmax3D <= 0 || fData3D->fStartSmax3D > 100. || fData3D->fStartsigmaL3D <= 0 || fData3D->fStartsigmaL3D > 100 || fData3D->fStartsigmaT3D <= 0 || fData3D->fStartsigmaT3D > 100 || fData3D->fStartNc3D <= 0 || fData3D->fStartNc3D > 100. ) fData3D->fGoodEvent3D = false;

  if( fData3D->fDebug3D ) cout<<"Model3D Start: "<< fData3D->fStartSel3D<<" "<<fData3D->fStartSaz3D<<" "<<fData3D->fStartXcore3D<<" "<<fData3D->fStartYcore3D<<" "<<fData3D->fStartSmax3D<<" "<<fData3D->fStartsigmaL3D<<" "<<fData3D->fStartsigmaT3D<<" "<<fData3D->fStartNc3D<<endl;
}

void VModel3D::setSelectedPixels()
{
  unsigned int totPix = 0;
  unsigned int telPix = 0;

  for (unsigned int iTel = 0; iTel < fData3D->fNTel3D; iTel++) {
    initializeDataReader(); // inherited from VEvndispData
    setTelID(iTel);         // inherited from VEvndispData
    
    for( unsigned int iPix = 0; iPix < fData3D->fNpix3D[iTel]; iPix++ ) {
      //// below inherited from VEvndispData ////
      if( getImage()[iPix] || getBorder()[iPix] || getImageBorderNeighbour()[iPix] ) { 
fData3D->fClean3D[iTel][iPix] = true;
telPix += 1;
fData3D->fMeasuredSum3D[iTel][iPix] = getData()->getSums()[iPix] / fData3D->fDCPE; 
fData3D->fPedvar3D[iTel][iPix] = getData()->getPedvars( fData->getCurrentSumWindow()[iPix] )[iPix] / fData3D->fDCPE;
      }
    }
    fData3D->fNDFTel3D[iTel] = telPix;
    totPix += telPix;
    telPix = 0;
  }
  fData3D->fNDF3D = totPix;

  if( fData3D->fDebug3D ) cout<<"Model3D: total selected pixels: "<< fData3D->fNDF3D <<endl;
}

void VModel3D::startModel3D()
{
  double lnl = 0; 
  vector<double> beta;
  vector< vector< double > > alpha;
  vector<double> vp(8); // Model3D uses 8 parameters
  vp[0] = fData3D->fStartSel3D;
  vp[1] = fData3D->fStartSaz3D;
  vp[2] = fData3D->fStartXcore3D;
  vp[3] = fData3D->fStartYcore3D;
  vp[4] = fData3D->fStartSmax3D;
  vp[5] = fData3D->fStartsigmaL3D;
  vp[6] = fData3D->fStartsigmaT3D;
  vp[7] = fData3D->fStartNc3D;

  fModelLnL->val(vp, lnl, beta, alpha);
  fData3D->fStartGOF3D = fData3D->fGOF3D;
  if( fData3D->fDebug3D ) cout<<"Model3D Start GOF: "<<fData3D->fStartGOF3D<<endl;

  double maxGOF = 100; // FIXED quality cut for GOF
  if( fData3D->fStartGOF3D > maxGOF ) fData3D->fGoodEvent3D = false;
}

void VModel3D::fillMu( const vector<double>& vp )
{
  /// for display ///
  fModel3DFn->setModel3DFn( fData3D->flocTel3D, vp );
  const unsigned int nTel = fData3D->fNTel3D;
  vector<double> dmuda(8);
  for(unsigned int iTel = 0; iTel < nTel; iTel++) {
    for(unsigned int iPix = 0; iPix < fData3D->fNpix3D[iTel]; iPix++) {
      double mu = 0;
      fModel3DFn->calcModel3DFn( iTel, iPix, fData3D->fpX3D[iTel][iPix], fData3D->fpY3D[iTel][iPix], fData3D->fpZ3D[iTel][iPix], mu, dmuda );
      double qterm1 = fData3D->fMarea3D[iTel] * fData3D->fomegapix3D[iTel][iPix] * fData3D->fcosptheta3D[iTel][iPix];
      mu = mu * qterm1; 
      fData3D->fMu3D[iTel][iPix] = mu;
      fData3D->fMuTel3D[iTel] += mu;
    }
  }
}

void VModel3D::fillMuDisplay()
{
  /// for display ///
  for (unsigned int iTel = 0; iTel < fData3D->fNTel3D; iTel++) {
    initializeDataReader(); // inherited from VEvndispData
    setTelID(iTel); // inherited from VEvndispData
    valarray<double> a0( fData3D->fNpix3D[iTel] );
    vector<bool> clean( fData3D->fNpix3D[iTel] );
    for( unsigned int iPix = 0; iPix < fData3D->fNpix3D[iTel]; iPix++ ) {
      a0[iPix] = fData3D->fMu3D[iTel][iPix] * fData3D->fDCPE;
      clean[iPix] = fData3D->fClean3D[iTel][iPix];
    }
    fData->setModel3DMu( a0 );
    fData->setModel3DClean( clean );
  }
}

void VModel3D::doFit()
{
  //// likelihood minimization ////
 
  VMinimizerFactory *mf = VMinimizerFactory::getInstance();

  vector<double> beta;
  vector< vector< double > > alpha;

  vector<double> vp(8); // Model3D uses 8 parameters
  vp[0] = fData3D->fStartSel3D;
  vp[1] = fData3D->fStartSaz3D;
  vp[2] = fData3D->fStartXcore3D;
  vp[3] = fData3D->fStartYcore3D;
  vp[4] = fData3D->fStartSmax3D;
  vp[5] = fData3D->fStartsigmaL3D;
  vp[6] = fData3D->fStartsigmaT3D;
  vp[7] = fData3D->fStartNc3D;

  VMinimizer* minimizer = mf->getMinimizer( *fModelLnL, vp );

  minimizer->setLoBound(0,vp[0]-10.);   minimizer->setHiBound(0,vp[0]+10.);
  minimizer->setLoBound(1,vp[1]-10.);   minimizer->setHiBound(1,vp[1]+10.);
  minimizer->setLoBound(2,vp[2]-100.);  minimizer->setHiBound(2,vp[2]+100.);
  minimizer->setLoBound(3,vp[3]-100.);  minimizer->setHiBound(3,vp[3]+100.);
  minimizer->setLoBound(4,0.5) ;       minimizer->setHiBound(4,50.);
  minimizer->setLoBound(5,0.5);        minimizer->setHiBound(5,50.);
  minimizer->setLoBound(6,1.);         minimizer->setHiBound(6,100.);
  minimizer->setLoBound(7,1.);         minimizer->setHiBound(7,50.);

  minimizer->minimize(); // minimize!!

  vector<double> param = minimizer->getParam();
  /// azimuth from -180 to 180 ///
  if( param[1] < -180. ) param[1] += 360.;
  if( param[1] >  180. ) param[1] -= 360.;

  vector<double> err = minimizer->getError();
  fData3D->fConverged3D = minimizer->getConverged();

  //// for final GOF ////////////
  double lnl = 0; 
  fModelLnL->val(param, lnl, beta, alpha);

  fData3D->fSel3D    = param[0];
  fData3D->fSaz3D    = param[1];
  fData3D->fXcore3D  = param[2];
  fData3D->fYcore3D  = param[3];
  fData3D->fSmax3D   = param[4];
  fData3D->fsigmaL3D = param[5];
  fData3D->fsigmaT3D = param[6];
  fData3D->fNc3D     = param[7];

  fData3D->fErrorSel3D    = err[0];
  fData3D->fErrorSaz3D    = err[1];
  fData3D->fErrorXcore3D  = err[2];
  fData3D->fErrorYcore3D  = err[3];
  fData3D->fErrorSmax3D   = err[4];
  fData3D->fErrorsigmaL3D = err[5];
  fData3D->fErrorsigmaT3D = err[6];
  fData3D->fErrorNc3D     = err[7];

  //////////////////////
  /// slant depth 
  /// Fundamentals of Neutrino Physics and Astrophysics
  /// by Giunti and Kim (page 396)
  double h0 = 6.4;       // scale height of atmosphere (km)
  double X0 = 1300; // atomsperic depth at sea level 
  double Ds;         // slant depth of shower maximum
  double Szen, hmax;
  //// reduced width: atmospheric density vs altitude 
  double Po = 101325; // sea level standard pressure (Pa)
  double To = 288.15; // sea level standard temperature (K)
  double g = 9.80665; // gravitational constant (m s^-2)
  double L = 6.5;     // temperature lapse rate (deg K/km)
  double R = 8.31447; // gas constant (J / mol*deg K) 
  double M = 28.9644; // molecular weight of dry air (gm/mol)
  double UP = (g*M)/(R*L);
  double T, P, rho;
  double Dwidth, rWidth, ErWidth;
  /// slant depth ///
  Szen = (90. - fData3D->fSel3D) * (TMath::Pi()/180.);
  hmax = fData3D->fSmax3D * cos(Szen);
  Ds = X0 * exp( -hmax / h0 );
  /// reduced 3D Width ///
  T = To - (L*hmax);
  P = Po * pow( 1. - (L*hmax/To), UP );
  rho = (P*M)/(R*T*1000); // kg m^-3
  Dwidth = fData3D->fsigmaT3D * 100. * rho * 0.001;
  ErWidth = fData3D->fErrorsigmaT3D * 100. * rho * 0.001;
  if(Ds > 0) {
    rWidth = Dwidth / Ds; 
    ErWidth = ErWidth / Ds; 
  } else {
    rWidth = 0;
    ErWidth = 0;
  }
  fData3D->fDepth3D = Ds;
  fData3D->fRWidth3D = rWidth *1000.;    // e-3
  fData3D->fErrRWidth3D = ErWidth *1000.; // e-3  

  ///////////////////////
  if( fData->getRunParameter()->fUseDisplayModel3D ) { //for display
    fillMu( param ); 
    fillMuDisplay();
  }
  if( fData3D->fDebug3D ) cout<<"Model3D Fit:   "<< fData3D->fSel3D<<" "<<fData3D->fSaz3D<<" "<<fData3D->fXcore3D<<" "<<fData3D->fYcore3D<<" "<<fData3D->fSmax3D<<" "<<fData3D->fsigmaL3D<<" "<<fData3D->fsigmaT3D<<" "<<fData3D->fNc3D<<endl;
  if( fData3D->fDebug3D ) cout<<"Model3D Fit   GOF: "<<fData3D->fGOF3D<<endl;

  return;
}

void VModel3D::fillInit3D()
{
  if( !fInitialized3D ) {
    vector<unsigned int> iNpix3D;
    iNpix3D.resize( fData->getNTel(), 0 );
    for (unsigned int i = 0; i < fData->getNTel(); i++) {
      iNpix3D[i] = fData->getDetectorGeo()->getX( i ).size();
    }
    fData3D->initModel3D( fData->getNTel(), iNpix3D ); // initialize data
  }
  fData3D->initEventModel3D(); 
  writeParameters3D();
  //for display
  if( fData->getRunParameter()->fUseDisplayModel3D ) fillMuDisplay();
}

void VModel3D::writeParameters3D()
{
  fData->getModel3DParameters()->eventNumber = fData->getShowerParameters()->eventNumber;
  /// write start parameters to file ///
  fData->getModel3DParameters()->fStartSel3D = fData3D->fStartSel3D;    
  fData->getModel3DParameters()->fStartSaz3D = fData3D->fStartSaz3D;    
  fData->getModel3DParameters()->fStartXcore3D = fData3D->fStartXcore3D;  
  fData->getModel3DParameters()->fStartYcore3D = fData3D->fStartYcore3D;  
  fData->getModel3DParameters()->fStartSmax3D = fData3D->fStartSmax3D;   
  fData->getModel3DParameters()->fStartsigmaL3D = fData3D->fStartsigmaL3D; 
  fData->getModel3DParameters()->fStartsigmaT3D = fData3D->fStartsigmaT3D; 
  fData->getModel3DParameters()->fStartNc3D = fData3D->fStartNc3D;  
  /// write best-fit parameters to file ///
  fData->getModel3DParameters()->fSel3D = fData3D->fSel3D;    
  fData->getModel3DParameters()->fSaz3D = fData3D->fSaz3D;    
  fData->getModel3DParameters()->fXcore3D = fData3D->fXcore3D;  
  fData->getModel3DParameters()->fYcore3D = fData3D->fYcore3D;  
  fData->getModel3DParameters()->fSmax3D = fData3D->fSmax3D;   
  fData->getModel3DParameters()->fsigmaL3D = fData3D->fsigmaL3D; 
  fData->getModel3DParameters()->fsigmaT3D = fData3D->fsigmaT3D; 
  fData->getModel3DParameters()->fNc3D = fData3D->fNc3D;  
  /// fit quality ///
  fData->getModel3DParameters()->fStartGoodness3D = fData3D->fStartGOF3D;
  fData->getModel3DParameters()->fGoodness3D = fData3D->fGOF3D;
  fData->getModel3DParameters()->fConverged3D = fData3D->fConverged3D;
  /// slant depth and reduced width ///
  fData->getModel3DParameters()->fDepth3D = fData3D->fDepth3D;
  fData->getModel3DParameters()->fRWidth3D = fData3D->fRWidth3D;
  fData->getModel3DParameters()->fErrRWidth3D = fData3D->fErrRWidth3D;
  /// write errors in fit parameters to file ///
  fData->getModel3DParameters()->fErrorSel3D = fData3D->fErrorSel3D;    
  fData->getModel3DParameters()->fErrorSaz3D = fData3D->fErrorSaz3D;    
  fData->getModel3DParameters()->fErrorXcore3D = fData3D->fErrorXcore3D;  
  fData->getModel3DParameters()->fErrorYcore3D = fData3D->fErrorYcore3D;  
  fData->getModel3DParameters()->fErrorSmax3D = fData3D->fErrorSmax3D;   
  fData->getModel3DParameters()->fErrorsigmaL3D = fData3D->fErrorsigmaL3D; 
  fData->getModel3DParameters()->fErrorsigmaT3D = fData3D->fErrorsigmaT3D; 
  fData->getModel3DParameters()->fErrorNc3D = fData3D->fErrorNc3D;  
  /// model direction ///
  if( fData3D->fConverged3D ) calcModelDirection();
  fData->getModel3DParameters()->fXoffModel3D = fData3D->fXoffModel3D;
  fData->getModel3DParameters()->fYoffModel3D = fData3D->fYoffModel3D;
  /// write to file ///
  fData->getModel3DParameters()->getTree()->Fill();
}

void VModel3D::calcModelDirection()
{
  // convert Model3D shower azimuth and elevation to sky direction //
  double tSzen = 0; // temp S zenith
  double tSaz = 0; // temp S azimuth
  tSzen = (90. - fData3D->fSel3D) * (TMath::Pi()/180.); // in radians
  tSaz = fData3D->fSaz3D * (TMath::Pi()/180.); // in radians

  double tsi[3]; // temp input s vector

  //// from spherical to cartesian (ground coordinates) ////
  tsi[0] = sin( tSzen ) * cos( tSaz );
  tsi[1] = sin( tSzen ) * sin( tSaz );
  tsi[2] = cos( tSzen );

  double ts[3]; // temp output s vector

  ts[0] =  ( -fData3D->fysg3D[1] * (tsi[0] - fData3D->fT3D[0]) + fData3D->fysg3D[0] * (tsi[1] - fData3D->fT3D[1]) ) / ( -(fData3D->fysg3D[1] * fData3D->fxsg3D[0]) + (fData3D->fysg3D[0] * fData3D->fxsg3D[1]) );

  ts[1] = ( (tsi[0] - fData3D->fT3D[0]) - (ts[0] * fData3D->fxsg3D[0]) ) / fData3D->fysg3D[0];
  ts[1] = 0. - ts[1];   ////// (swap Y sign) ///////
  ts[2] = 0;

  ts[0] = ts[0] / (TMath::Pi()/180.); // from radians to deg
  ts[1] = ts[1] / (TMath::Pi()/180.); // from radians to deg
  ts[2] = ts[2] / (TMath::Pi()/180.); // from radians to deg

  fData3D->fXoffModel3D = ts[0];
  fData3D->fYoffModel3D = ts[1];
  fData->getModel3DParameters()->fXoffModel3D = fData3D->fXoffModel3D;
  fData->getModel3DParameters()->fYoffModel3D = fData3D->fYoffModel3D;
}
