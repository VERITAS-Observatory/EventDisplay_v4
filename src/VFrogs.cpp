/*********************************************
 *
 * EVENDISPLAY - FROGS INTERFACE Ver. 0.0 
 *
 *********************************************/

/*! /class VFrogs
  
\brief  Called from VEventLoop. Opens templates and does minimization.

\author S Vincent, G Hughes.

*/

#include "VFrogs.h"
#include "frogs.h"

#define FROGSDEBUG 0

VFrogs::VFrogs()
{

  fFrogParameters = new VFrogParameters();
  frogsRecID = getRunParameter()->ffrogsRecID;

  fInitialized = false;

}

VFrogs::~VFrogs()
{
  
}

//================================================================
//================================================================
void VFrogs::doFrogsStuff( int eventNumber ) {
 
  int i,j;

  // only at first call in the analysis run: initialize data class, set trees
  if( !fInitialized ) {
    initAnalysis();
    fInitialized = true;
    readTableFrogs();
    fStartEnergyLoop = 0;
  }
  
  initFrogEvent();
  
  int adc=2;
  double inEnergy = getFrogsStartEnergy(eventNumber);

  // Get Anasum Event Number for this Run
  ///int AnasumNumber = getFrogsAnasumNumber(eventNumber,fRunPar->frunnumber);
 
  //Store data from the GrISU analysis to a FROGS structure
  //In this example the function frogs_convert_from_grisu takes 
  //arguments corresponding the the GrISU analysis. An equivalent 
  //function should be written for any other analysis package to 
  //make the FROGS analysis usable. 
  ////if ( inEnergy != FROGS_BAD_NUMBER && AnasumNumber != FROGS_BAD_NUMBER ) {
  if ( inEnergy != FROGS_BAD_NUMBER ) {

    struct frogs_imgtmplt_in d;
    d=frogs_convert_from_ed(eventNumber,adc,inEnergy);
  
    //Print out the data contained in the FROGS structure frogs_imgtmplt_in
    //This is useful when developing a frogs_convert_from_XXXX function

    if( FROGSDEBUG )  frogs_print_raw_event(d);
  
    //Call the FROGS analysis
    struct frogs_imgtmplt_out output;
    output=frogs_img_tmplt(&d);
  
    frogsEventID     = output.event_id;
    frogsGSLConStat  = output.gsl_convergence_status;
    frogsNB_iter     = output.nb_iter;
    frogsXS          = output.cvrgpt.xs;
    frogsXSerr       = output.cvrgpterr.xs;
    frogsYS          = output.cvrgpt.ys;
    frogsYSerr       = output.cvrgpterr.ys;
    frogsXP          = output.cvrgpt.xp;
    frogsXPerr       = output.cvrgpterr.xp;
    frogsYP          = output.cvrgpt.yp;
    frogsYPerr       = output.cvrgpterr.yp;
    frogsXPGC	     = transformShowerPosition(90. - fData->getShowerParameters()->fTelElevation[0], fData->getShowerParameters()->fTelAzimuth[0],output.cvrgpt.xp,output.cvrgpt.yp,0);
    frogsYPGC	     = transformShowerPosition(90. - fData->getShowerParameters()->fTelElevation[0], fData->getShowerParameters()->fTelAzimuth[0],output.cvrgpt.xp,output.cvrgpt.yp,1);
    frogsEnergy      = output.cvrgpt.log10e;
    frogsEnergyerr   = output.cvrgpterr.log10e;
    frogsLambda      = output.cvrgpt.lambda;
    frogsLambdaerr   = output.cvrgpterr.lambda;
    frogsGoodnessImg = output.goodness_img;
    frogsNpixImg     = output.npix_img;
    frogsGoodnessBkg = output.goodness_bkg;
    frogsNpixBkg     = output.npix_bkg;

    for( j=0; j<4; j++ ) {
      frogsTelGoodnessImg[j] = output.tel_goodnessImg[j];
      frogsTelGoodnessBkg[j] = output.tel_goodnessBkg[j];
    }

    for( j=0; j<4; j++ ) {
      for( i=0; i<500; i++ ) {
        if( j == 0 ) frogsTemplateMu0[i] = output.tmplt_tubes[j][i];
        if( j == 1 ) frogsTemplateMu1[i] = output.tmplt_tubes[j][i];
        if( j == 2 ) frogsTemplateMu2[i] = output.tmplt_tubes[j][i];
        if( j == 3 ) frogsTemplateMu3[i] = output.tmplt_tubes[j][i];
      }
    }

    frogsXPStart     = getShowerParameters()->fShowerXcore_SC[frogsRecID];
    frogsYPStart     = getShowerParameters()->fShowerYcore_SC[frogsRecID];
    frogsXPED        = getShowerParameters()->fShowerXcore[frogsRecID];
    frogsYPED        = getShowerParameters()->fShowerYcore[frogsRecID];
    frogsXSStart     = fData->getShowerParameters()->fShower_Xoffset[frogsRecID]; //TEMP GH
    //frogsXSStart     = getShowerParameters()->fShower_Xoffset[frogsRecID];
    frogsYSStart     = -1.0*fData->getShowerParameters()->fShower_Yoffset[frogsRecID];

    getFrogParameters()->frogsEventID = getFrogsEventID();
    getFrogParameters()->frogsGSLConStat = getFrogsGSLConStat();
    getFrogParameters()->frogsNB_iter = getFrogsNB_iter();
    getFrogParameters()->frogsXS = getFrogsXS();
    getFrogParameters()->frogsXSerr = getFrogsXSerr();
    getFrogParameters()->frogsYS = getFrogsYS();
    getFrogParameters()->frogsYSerr = getFrogsYSerr();
    getFrogParameters()->frogsXP = getFrogsXP();
    getFrogParameters()->frogsXPerr = getFrogsXPerr();
    getFrogParameters()->frogsYP = getFrogsYP();
    getFrogParameters()->frogsXPGC = getFrogsXPGC();
    getFrogParameters()->frogsYPGC = getFrogsYPGC();
    getFrogParameters()->frogsYPerr = getFrogsYPerr();
    getFrogParameters()->frogsEnergy = getFrogsEnergy();
    getFrogParameters()->frogsEnergyerr = getFrogsEnergyerr();
    getFrogParameters()->frogsLambda = getFrogsLambda();
    getFrogParameters()->frogsLambdaerr = getFrogsLambdaerr();
    getFrogParameters()->frogsGoodnessImg = getFrogsGoodnessImg();
    getFrogParameters()->frogsNpixImg = getFrogsNpixImg();
    getFrogParameters()->frogsGoodnessBkg = getFrogsGoodnessBkg();
    getFrogParameters()->frogsNpixBkg = getFrogsNpixBkg();

    getFrogParameters()->frogsXPStart = getFrogsXPStart();
    getFrogParameters()->frogsYPStart = getFrogsYPStart();
    getFrogParameters()->frogsXPED = getFrogsXPED();
    getFrogParameters()->frogsYPED = getFrogsYPED();
    getFrogParameters()->frogsXSStart = getFrogsXSStart();
    getFrogParameters()->frogsYSStart = getFrogsYSStart();

    getFrogParameters()->frogsTelGoodnessImg0 = getFrogsTelGoodnessImg(0);
    getFrogParameters()->frogsTelGoodnessImg1 = getFrogsTelGoodnessImg(1);
    getFrogParameters()->frogsTelGoodnessImg2 = getFrogsTelGoodnessImg(2);
    getFrogParameters()->frogsTelGoodnessImg3 = getFrogsTelGoodnessImg(3);
    getFrogParameters()->frogsTelGoodnessBkg0 = getFrogsTelGoodnessBkg(0);
    getFrogParameters()->frogsTelGoodnessBkg1 = getFrogsTelGoodnessBkg(1);
    getFrogParameters()->frogsTelGoodnessBkg2 = getFrogsTelGoodnessBkg(2);
    getFrogParameters()->frogsTelGoodnessBkg3 = getFrogsTelGoodnessBkg(3);

//    cout << "Out: " << getFrogsXS() << " " << getFrogsYS() << " " << getFrogsXP() << " " << getFrogsYP() << " " << pow(10.,(double)getFrogsEnergy()) << endl;

//    cout << endl;
//    cout << endl;

    getFrogParameters()->getTree()->Fill();

    valarray<double> a0(frogsTemplateMu0,500);
    valarray<double> a1(frogsTemplateMu1,500);
    valarray<double> a2(frogsTemplateMu2,500);
    valarray<double> a3(frogsTemplateMu3,500);
    for( j=0; j<4; j++ ) {
      setTelID(j);
      if( j == 0 ) fData->setTemplateMu( a0 );
      if( j == 1 ) fData->setTemplateMu( a1 );
      if( j == 2 ) fData->setTemplateMu( a2 );
      if( j == 3 ) fData->setTemplateMu( a3 );
    }

    //Print out the results of the image template analysis
    //The values returned by frogs_img_tmplt really are to be stored in 
    //structures used in the analysis package in which FROGS is being sed. 
    if( FROGSDEBUG ) frogs_print_param_spc_point(output);

  }
  //return FROGS_OK;
  return; 
}
//================================================================
//================================================================
void VFrogs::initFrogEvent()
{
}
//================================================================
//================================================================
void VFrogs::initAnalysis()
{
    initOutput();
    initFrogTree();
}
//================================================================
//================================================================
void VFrogs::initOutput()
{
  if( fDebug ) cout << "void VFrogs::initOuput()" << endl;
  // check if root outputfile exist
  if( fOutputfile != 0 )
    {
      printf("FROGPUT: Frogs Output File Exists\n");
      return;
    }
// otherwise create it
  if( fRunPar->foutputfileName != "-1" )
    {
      printf("FROGPUT: Frogs foutputfileName = -1 : Attempt to create file\n");
      char i_textTitle[300];
      sprintf( i_textTitle, "VERSION %d", getRunParameter()->getEVNDISP_TREE_VERSION() );
      if( getRunParameter()->fShortTree ) sprintf( i_textTitle, "%s (short tree)", i_textTitle );
      fOutputfile = new TFile( fRunPar->foutputfileName.c_str(), "RECREATE", i_textTitle );
    }
  
  if (FROGSDEBUG)
    printf("FROGPUT Are we initOutput??\n");
  
}
//================================================================
//================================================================
void VFrogs::initFrogTree()
{

  if (FROGSDEBUG)
    printf("FROGPUT Check in initTree\n");
  
  char i_text[300];
  char i_textTitle[300];
  sprintf( i_text, "frogspars" );
  // tree versioning numbers used in mscw_energy
  sprintf( i_textTitle, "FROGPUT: Frogs Parameters (VERSION %d)\n", getRunParameter()->getEVNDISP_TREE_VERSION() );
  if( getRunParameter()->fShortTree ) sprintf( i_textTitle, "%s (short tree)", i_textTitle );
  fFrogParameters->initTree( i_text, i_textTitle );
  
}
//================================================================
//================================================================
void VFrogs::readTableFrogs()
{
  
  int eventNumber;
  double Erec;
  
  string fmscwFrogsFile = getRunParameter()->ffrogsmscwfile;
  cout << "FROGS readTableFrogs " << getRunParameter()->ffrogsmscwfile.c_str() << endl;
  
  TFile *mscwFrogsFile = new TFile( fmscwFrogsFile.c_str() , "READ" );
  
  if( mscwFrogsFile->IsZombie() )
    {
      cout << "VFrogs::readTableFrogs error: File " << fmscwFrogsFile.c_str() << " does not exist!" << endl;
      exit( -1 );
    }

  TTree *mscwTreeFrogs = (TTree*)mscwFrogsFile->Get("data");
  mscwTreeFrogs->SetBranchAddress("eventNumber",&eventNumber);
  mscwTreeFrogs->SetBranchAddress("Erec",&Erec);
  
  for ( int i=0 ; i < mscwTreeFrogs->GetEntries() ; i++ ) 
    { 
      mscwTreeFrogs->GetEntry(i);
      fTableRunNumber.push_back(eventNumber);
      fTableEnergy.push_back(Erec);
    }

  mscwFrogsFile->Close();
  
  cout << "Finished Reading: " << fTableRunNumber.size() << " with size " << fTableEnergy.size() << endl;
  
}
//================================================================
///================================================================
void VFrogs::readAnasumFrogs()
{
 
  int i,j,k=0;

  int runOn; 
  int eventNumber;
 
  char dataname[256];
 
  string fAnasumFrogsFile = "/afs/ifh.de/user/g/gahughes/scratch/VERITAS/EVNDISP/EVNDISP-400/trunk/bin/Crab_20120206.root";
  cout << "FROGS readAnasumFrogs " << fAnasumFrogsFile.c_str() << endl;
  
  TFile *AnasumFrogsFile = new TFile( fAnasumFrogsFile.c_str() , "READ" );
  
  if( AnasumFrogsFile->IsZombie() )
    {
      cout << "VFrogs::readAnasumFrogs error: File " << fAnasumFrogsFile.c_str() << " does not exist!" << endl;
      exit( -1 );
    }

  TTree *tRun = (TTree*)AnasumFrogsFile->Get("total_1/stereo/tRunSummary");

  tRun->SetBranchAddress("runOn",&runOn);

  for( i=0; i<tRun->GetEntries(); i++ )
  {

    tRun->GetEntry(i);

    if( runOn > 0 )
    {

      sprintf(dataname,"run_%d/stereo/data_on",runOn);
      TTree *tData = (TTree*)AnasumFrogsFile->Get(dataname);
      tData->SetBranchAddress("eventNumber",&eventNumber);

      for( j=0; j<tData->GetEntries(); j++ )
      {
         tData->GetEntry(j);
	 fAnasumRunNumber.push_back(runOn);
	 fAnasumEventNumber.push_back(eventNumber);
         k++;
      }

    }

  }

  cout << "Finished Reading Anasum File " << i << " Runs, " << k << " events read." << endl;

  AnasumFrogsFile->Close();
  
}
//================================================================
//================================================================
double VFrogs::getFrogsStartEnergy(int eventNumber)
{
  
  int mscwTotal = fTableRunNumber.size();
  
  for( int i=fStartEnergyLoop ; i < mscwTotal ; i++ ) 
  {
    if( eventNumber == fTableRunNumber[i] )
    {
      fStartEnergyLoop = i - 1;
      return fTableEnergy[i];
    } 
    else if( eventNumber < fTableRunNumber[i] )
    {
      return FROGS_BAD_NUMBER;
    }
  }
  return FROGS_BAD_NUMBER;
  
}
//================================================================
//================================================================
int VFrogs::getFrogsAnasumNumber( int eventNumber, int runNumber )
{

  int AnasumTotal = fAnasumRunNumber.size();
  
  for( int i = 0 ; i < AnasumTotal ; i++ ) 
  {
    if( runNumber == fAnasumRunNumber[i] && eventNumber == fAnasumEventNumber[i] )
      return eventNumber;
  }

  return FROGS_BAD_NUMBER;
  
}
//================================================================
//================================================================
float VFrogs::transformTelescopePosition( int iTel, float i_ze, float i_az, int axis )
{
  // transform telescope positions from ground into shower coordinates
  float i_xrot, i_yrot, i_zrot;
  float i_xcos = 0.;
  float i_ycos = 0.;
  
  // calculate direction cosine
  i_xcos = sin(i_ze / TMath::RadToDeg() ) * sin( (i_az-180.)/TMath::RadToDeg() );
  i_ycos = sin(i_ze / TMath::RadToDeg() ) * cos( (i_az-180.)/TMath::RadToDeg() );
  
  setTelID( iTel );
  // call to GrIsu routine
  tel_impact( i_xcos, i_ycos, getDetectorGeo()->getTelXpos()[iTel], getDetectorGeo()->getTelYpos()[iTel], getDetectorGeo()->getTelZpos()[iTel], &i_xrot, &i_yrot, &i_zrot, false );
  
  if( axis == 0 ) 
    return i_xrot;
  else if( axis == 1 )
    return i_yrot;
  else
    return FROGS_BAD_NUMBER; 
}
//================================================================
//================================================================
float VFrogs::transformShowerPosition( float i_ze, float i_az, float xcore, float ycore, int axis )
{
// transform telescope positions from ground into shower coordinates
  float i_xrot, i_yrot, i_zrot;
  float i_xcos = 0.;
  float i_ycos = 0.;
 
  i_ze *= -1.;
  i_az *= -1.;
 
  // calculate direction cosine
  i_xcos = sin(i_ze / TMath::RadToDeg() ) * sin( (i_az-180.)/TMath::RadToDeg() );
  i_ycos = sin(i_ze / TMath::RadToDeg() ) * cos( (i_az-180.)/TMath::RadToDeg() );
  
  // call to GrIsu routine
  tel_impact( i_xcos, i_ycos, xcore, ycore, 0., &i_xrot, &i_yrot, &i_zrot, false );
  
  if( axis == 0 ) 
    return -i_xrot;
  else if( axis == 1 )
    return -i_yrot;
  else
    return FROGS_BAD_NUMBER; 
}
//================================================================
//================================================================
void VFrogs::terminate()
{
 
    getFrogParameters()->getTree()->Write();

}

void VFrogs::finishFrogs(TFile *f)
{

  // Open outfile again to copy frogs tree to mscw file.

  // reopen mscw file 
  string fmscwFrogsFile = getRunParameter()->ffrogsmscwfile;
  TFile *mscwFrogsFile = new TFile( fmscwFrogsFile.c_str() , "UPDATE" );
  if( mscwFrogsFile->IsZombie() )
  {
     cout << "Finish Frogs:" << endl;
     cout << "VFrogs::readTableFrogs error: File " << fmscwFrogsFile.c_str() << " does not exist!" << endl;
     exit( -1 );
  }

  // Clone tree to mscw file checking it opened
  if( f->IsZombie() )
  {
    cout << "error: finish Frogs f file problem: "  << endl;
  }
  else
  {
    ((TTree*)f->Get("frogspars"))->CloneTree()->Write();
  }

// Close the files
  mscwFrogsFile->Close();

  return;
 
}
//================================================================
//================================================================
struct frogs_imgtmplt_in VFrogs::frogs_convert_from_ed(int eventNumber, int adc_type, double inEnergy) {
  /* The frogs_convert_from_grisu function is called with 
     arguments containing all the data necessary to the image template 
     analysis as per the structures used in grisu analysis. It can serve 
     as an example for the interfacing of the template analysis with other 
     analysis packages. It returns the data necessary to the template 
     analysis in a structure that is appropriate.  */

   struct frogs_imgtmplt_in rtn;
 
   //Tracked elevation from telescope 0
   rtn.elevation = fData->getShowerParameters()->fTelElevation[0];
   rtn.event_id  = eventNumber;

  //Telescopes
  rtn.ntel=fData->getNTel(); //Number of telescopes

  rtn.scope=new struct frogs_telescope [rtn.ntel];
  rtn.nb_live_pix_total=0;//Total number or pixels in use
  for(int tel=0;tel<rtn.ntel;tel++) {
    initializeDataReader();
    setTelID(tel);
    
    //Telescope position in the coordonate system used in the reconstruction
    rtn.scope[tel].xfield = 
      transformTelescopePosition( tel, 90 - fData->getShowerParameters()->fTelElevation[0], fData->getShowerParameters()->fTelAzimuth[0], 0 );
    rtn.scope[tel].yfield = 
      transformTelescopePosition( tel, 90 - fData->getShowerParameters()->fTelElevation[0], fData->getShowerParameters()->fTelAzimuth[0], 1 );
   
    //printf("FROGS Tel Position: %d %f %f\n",tel+1,rtn.scope[tel].xfield,rtn.scope[tel].yfield);
 
    if(FROGSDEBUG)
      printf("TelSC %d | %.2f %.2f | %.2f %.2f | %.2f %.2f | %.2f %.2f | %.2f %.2f \n",tel,
	     fData->getShowerParameters()->fShowerZe[frogsRecID],
	     fData->getShowerParameters()->fShowerAz[frogsRecID],
	     getDetectorGeo()->getTelXpos()[tel]-fData->getShowerParameters()->fShowerXcore[frogsRecID],
	     getDetectorGeo()->getTelYpos()[tel]-fData->getShowerParameters()->fShowerYcore[frogsRecID],		
	     getDetectorGeo()->getTelXpos()[tel],
	     getDetectorGeo()->getTelYpos()[tel],
	     rtn.scope[tel].xfield,
	     rtn.scope[tel].yfield,
	     (180.0/3.1415)*atan2(rtn.scope[tel].yfield,rtn.scope[tel].xfield),
	     (180.0/3.1415)*atan2(getDetectorGeo()->getTelYpos()[tel],getDetectorGeo()->getTelXpos()[tel]));

    //Telescope effective collection area
    //Number of pixels 
    rtn.scope[tel].npix=fData->getDetectorGeo()->getNChannels()[tel];

    //Set the dimension of the pixel parameter arrays
    rtn.scope[tel].xcam= new float [rtn.scope[tel].npix];
    rtn.scope[tel].ycam= new float [rtn.scope[tel].npix];
    rtn.scope[tel].q= new float [rtn.scope[tel].npix];
    rtn.scope[tel].ped= new float [rtn.scope[tel].npix];
    rtn.scope[tel].exnoise= new float [rtn.scope[tel].npix];
    rtn.scope[tel].pixinuse= new int [rtn.scope[tel].npix];
    rtn.scope[tel].telpixarea= new float [rtn.scope[tel].npix];
    float foclen=1000.0*fData->getDetectorGeo()->getFocalLength()[tel]; //Focal length in mm //modified by sv

    //Initialize the number of live pixel in the telescope
    rtn.scope[tel].nb_live_pix=0;

    //Loop on the pixels
    for(int pix=0;pix<rtn.scope[tel].npix;pix++) 
    {
      //Pixel coordinates
      rtn.scope[tel].xcam[pix]=fData->getDetectorGeo()->getX()[pix];
      rtn.scope[tel].ycam[pix]=fData->getDetectorGeo()->getY()[pix];

      //Excess noise
      rtn.scope[tel].exnoise[pix]=extra_noise;
      //Pixel dead or alive
      rtn.scope[tel].pixinuse[pix]=FROGS_OK;
      if(fData->getDead()[pix]!=0)
        rtn.scope[tel].pixinuse[pix]=FROGS_NOTOK;
      //Increment the number of live pixels
      if(rtn.scope[tel].pixinuse[pix]==FROGS_OK) rtn.scope[tel].nb_live_pix++;
      //Pixel effective collecting area in square degrees
      float tmppixarea=
	fData->getDetectorGeo()->getTubeRadius_MM(tel)[pix]*FROGS_DEG_PER_RAD/foclen;//modified by sv
      tmppixarea=FROGS_PI*tmppixarea*tmppixarea;
      rtn.scope[tel].telpixarea[pix]=telarea*tmppixarea*cone_eff;

      //Initialize the pixel signal and pedestal width to zero
      rtn.scope[tel].q[pix]=0;
      rtn.scope[tel].ped[pix]=0;
      //Set them to their values in p.e. if the d.c./p.e. factor is non zero
      if(dc2pe!=0) 
      {
        rtn.scope[tel].q[pix]=fData->getData()->getSums()[pix]/dc2pe;
        rtn.scope[tel].ped[pix]=fData->getData()->getPedvars()[pix]*frogs_pedwidth_correction/dc2pe;
      }
    }
    //Total number of live pixels in the array
    rtn.nb_live_pix_total=rtn.nb_live_pix_total+rtn.scope[tel].nb_live_pix;
  } 

  //Optimization starting point todo y -> -y ??
  rtn.startpt.xs=1.0*fData->getShowerParameters()->fShower_Xoffset[frogsRecID];
  rtn.startpt.ys=-1.0*fData->getShowerParameters()->fShower_Yoffset[frogsRecID];

//MC  rtn.startpt.xs=1.0*fData->getShowerParameters()->MCTel_Xoff;
//MC  rtn.startpt.ys=-1.0*fData->getShowerParameters()->MCTel_Yoff;

  rtn.startpt.xp=fData->getShowerParameters()->fShowerXcore_SC[frogsRecID];
  rtn.startpt.yp=1.0*fData->getShowerParameters()->fShowerYcore_SC[frogsRecID];
//MC  rtn.startpt.xp=fData->getShowerParameters()->MCxcore_SC;
//MC  rtn.startpt.yp=1.0*fData->getShowerParameters()->MCycore_SC;

  if (FROGSDEBUG) {
    printf("ShowerSC %f %f\n",getShowerParameters()->fShowerXcore_SC[frogsRecID],getShowerParameters()->fShowerYcore_SC[frogsRecID]);
    printf("Shower %f %f\n",getShowerParameters()->fShowerXcore[frogsRecID],getShowerParameters()->fShowerYcore[frogsRecID]);
  }

  rtn.startpt.lambda=0.3; //We use a fixed value by lack of information. 

  rtn.startpt.log10e = inEnergy; //inEnergy from ED 
  if( rtn.startpt.log10e > 0.0 )
    rtn.startpt.log10e = log10(rtn.startpt.log10e);
  else
   rtn.startpt.log10e = FROGS_BAD_NUMBER;
//MC  rtn.startpt.log10e = log10(fData->getShowerParameters()->MCenergy); //inEnergy from MC 

//  cout << "MC " << 1.0*fData->getShowerParameters()->MCTel_Xoff << " " << -1.0*fData->getShowerParameters()->MCTel_Yoff << " " << fData->getShowerParameters()->MCxcore_SC << " " << 1.0*fData->getShowerParameters()->MCycore_SC << " " << fData->getShowerParameters()->MCenergy << endl;
//  cout << "ED " << 1.0*fData->getShowerParameters()->fShower_Xoffset[frogsRecID] << " " << -1.0*fData->getShowerParameters()->fShower_Yoffset[frogsRecID] << " " << fData->getShowerParameters()->fShowerXcore_SC[frogsRecID] << " " << 1.0*fData->getShowerParameters()->fShowerYcore_SC[frogsRecID] << " " << inEnergy << endl;
//  cout << endl;

  //Decides if the event is worth analysing. 
  rtn.worthy_event=FROGS_OK;
  //Log(0.1)=-1; Log(0.15)=-0.824; Log(0.2)=-0.699; Log(0.25)=-0.602
  //Log(0.3)=-0.523; Log(0.35)=-0.456; Log(0.4)=-0.398 
  //Energy large enough? 
  if(rtn.startpt.log10e<-0.699)   rtn.worthy_event=FROGS_NOTOK;
  //Energy small enough? 
  if(rtn.startpt.log10e>1.0)   rtn.worthy_event=FROGS_NOTOK;
  //if(rtn.startpt.log10e>1.39112)   rtn.worthy_event=FROGS_NOTOK;
  //if(rtn.startpt.log10e>2.0)   rtn.worthy_event=FROGS_NOTOK;
  //Distance of the impact point small enough? 
  if(sqrt(rtn.startpt.xp*rtn.startpt.xp+rtn.startpt.yp*rtn.startpt.xp)>350.0) 
    rtn.worthy_event=FROGS_NOTOK;
  //Count the number of telescopes with more than 300dc in their image
  int ngoodimages=0;
  for(int tel=0; tel<rtn.ntel;tel++)
  {
    setTelID(tel);
    if(fData->getImageParameters()->size>300.0) ngoodimages=ngoodimages+1;
  }
  //Require the number of telescopes with more than 300dc to be at least 3
  if (ngoodimages<3) rtn.worthy_event=FROGS_NOTOK;

  return rtn;

}
