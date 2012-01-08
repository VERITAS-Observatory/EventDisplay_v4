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
}

VFrogs::~VFrogs()
{
  
}

//================================================================
//================================================================
void VFrogs::doFrogsStuff( int eventNumber ) {
  
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
 
  //Store data from the GrISU analysis to a FROGS structure
  //In this example the function frogs_convert_from_grisu takes 
  //arguments corresponding the the GrISU analysis. An equivalent 
  //function should be written for any other analysis package to 
  //make the FROGS analysis usable. 
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
  frogsEnergy      = output.cvrgpt.log10e;
  frogsEnergyerr   = output.cvrgpterr.log10e;
  frogsLambda      = output.cvrgpt.lambda;
  frogsLambdaerr   = output.cvrgpterr.lambda;
  frogsGoodnessImg = output.goodness_img;
  frogsNpixImg     = output.npix_img;
  frogsGoodnessBkg = output.goodness_bkg;
  frogsNpixBkg     = output.npix_bkg;

  frogsXPStart     = getShowerParameters()->fShowerXcore_SC[5];
  frogsYPStart     = getShowerParameters()->fShowerYcore_SC[5];
  frogsXPED        = getShowerParameters()->fShowerXcore[5];
  frogsYPED        = getShowerParameters()->fShowerYcore[5];
  frogsXSStart     = fData->getShowerParameters()->fShower_Xoffset[5];
  frogsYSStart     = -1.0*fData->getShowerParameters()->fShower_Yoffset[5];


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


  getFrogParameters()->getTree()->Fill();

  //Print out the results of the image template analysis
  //The values returned by frogs_img_tmplt really are to be stored in 
  //structures used in the analysis package in which FROGS is being sed. 
  if( FROGSDEBUG ) frogs_print_param_spc_point(output);
   }
  //return FROGS_OK;
  return; 
}
int VFrogs::getFrogsEventID()
{
  return frogsEventID;
}
int VFrogs::getFrogsGSLConStat()
{
  return frogsGSLConStat;
}
int VFrogs::getFrogsNB_iter()
{
  return frogsNB_iter;
}
float VFrogs::getFrogsXS()
{
  return frogsXS;
}
float VFrogs::getFrogsXSerr()
{
  return frogsXSerr;
}
float VFrogs::getFrogsYS()
{
  return frogsYS;
}
float VFrogs::getFrogsYSerr()
{
  return frogsYSerr;
}
float VFrogs::getFrogsXP()
{
  return frogsXP;
}
float VFrogs::getFrogsXPerr()
{
  return frogsXPerr;
}
float VFrogs::getFrogsYP()
{
  return frogsYP;
}
float VFrogs::getFrogsYPerr()
{
  return frogsYPerr;
}
float VFrogs::getFrogsEnergy()
{
  return frogsEnergy;
}
float VFrogs::getFrogsEnergyerr()
{
  return frogsEnergyerr;
}
float VFrogs::getFrogsLambda()
{
  return frogsLambda;
}
float VFrogs::getFrogsLambdaerr()
{
  return frogsLambdaerr;
}
float VFrogs::getFrogsGoodnessImg()
{
  return frogsGoodnessImg;
}
int VFrogs::getFrogsNpixImg()
{
  return frogsNpixImg;
}
float VFrogs::getFrogsGoodnessBkg()
{
  return frogsGoodnessBkg;
}
int VFrogs::getFrogsNpixBkg()
{
  return frogsNpixBkg;
}
float VFrogs::getFrogsXPStart()
{
  return frogsXPStart;
}
float VFrogs::getFrogsYPStart()
{
  return frogsYPStart;
}

float VFrogs::getFrogsXPED()
{
  return frogsXPED;
}
float VFrogs::getFrogsYPED()
{
  return frogsYPED;
}

float VFrogs::getFrogsXSStart()
{
  return frogsXSStart;
}
float VFrogs::getFrogsYSStart()
{
  return frogsYSStart;
}
//================================================================
//================================================================
void VFrogs::initFrogEvent()
{
  //    if( fDebug ) cout << "void VArrayAnalyzer::initEvent()" << endl;
  
  //    setAnalysisArrayEventStatus( 0 );

  // reset shower parameters
  //    getShowerParameters()->reset( getNTel() );
  
  // get basic infos for this event
}
//================================================================
//================================================================
void VFrogs::initAnalysis()
{

//    todo Need a debug statement here.
//    if( fDebug ) cout << "VFrogs::initAnalysis()" << endl;

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
  
  //   todo Need Debug Statement here.
  //     if( fDebug ) cout << "void VArrayAnalyzer::initTrees() " <<  endl;
  
  //    fOutputfile->cd();
  // now book the tree (for MC with additional MC block)
  char i_text[300];
  char i_textTitle[300];
  sprintf( i_text, "frogspars" );
  // tree versioning numbers used in mscw_energy
  sprintf( i_textTitle, "FROGPUT: Frogs Parameters (VERSION %d)\n", getRunParameter()->getEVNDISP_TREE_VERSION() );
  if( getRunParameter()->fShortTree ) sprintf( i_textTitle, "%s (short tree)", i_textTitle );
  // Is the next line needed?
  //sprintf( i_textTitle, "%s (short tree)", i_textTitle );
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
  mscwTreeFrogs->SetBranchAddress("ErecS",&Erec);
  
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
float VFrogs::transformTelescopePosition( int iTel, float i_ze, float i_az, int axis )
{
// transform telescope positions from ground into shower coordinates
  float i_xrot, i_yrot, i_zrot;
  float i_xcos = 0.;
  float i_ycos = 0.;
  
  // calculate direction cosinii
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
void VFrogs::terminate()
{
 
    getFrogParameters()->getTree()->Write();
    //fOutputfile->Flush();

}

void VFrogs::finishFrogs(TFile *f)
//void VFrogs::finishFrogs()
{

  // Open outfile again to copy frogs tree to mscw file.
  //TFile f( fRunPar->foutputfileName.c_str(), "READ" );

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
  //f.Close();

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
    
    if(FROGSDEBUG)
      printf("TelSC %d | %.2f %.2f | %.2f %.2f | %.2f %.2f | %.2f %.2f | %.2f %.2f \n",tel,
	     fData->getShowerParameters()->fShowerZe[5],
	     fData->getShowerParameters()->fShowerAz[5],
	     getDetectorGeo()->getTelXpos()[tel]-fData->getShowerParameters()->fShowerXcore[5],
	     getDetectorGeo()->getTelYpos()[tel]-fData->getShowerParameters()->fShowerYcore[5],		
	     getDetectorGeo()->getTelXpos()[tel],
	     getDetectorGeo()->getTelYpos()[tel],
	     rtn.scope[tel].xfield,
	     rtn.scope[tel].yfield,
	     (180.0/3.1415)*atan2(rtn.scope[tel].yfield,rtn.scope[tel].xfield),
	     (180.0/3.1415)*atan2(getDetectorGeo()->getTelYpos()[tel],getDetectorGeo()->getTelXpos()[tel]));

    //Telescope effective collection area
    float telarea=94.0; //telescope area could be from the configuration file
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
    float foclen=1000*fData->getDetectorGeo()->getFocalLength()[tel]; //Focal length in mm //modified by sv

    //Initialize the number of live pixel in the telescope
    rtn.scope[tel].nb_live_pix=0;

    //Loop on the pixels
    for(int pix=0;pix<rtn.scope[tel].npix;pix++) 
    {
      //Pixel coordinates
      rtn.scope[tel].xcam[pix]=fData->getDetectorGeo()->getX()[pix];
      rtn.scope[tel].ycam[pix]=fData->getDetectorGeo()->getY()[pix];

      //Excess noise
      rtn.scope[tel].exnoise[pix]=0.35;
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
      rtn.scope[tel].telpixarea[pix]=telarea*tmppixarea*
//    todo this is not 1 !
//     GH WHAT IS THE PMT COLLECTION EFF??
        0.81;

      float dc2pe=5.5;  //This is the number of d.c. in one p.e.
      //Initialize the pixel signal and pedestal width to zero
      rtn.scope[tel].q[pix]=0;
      rtn.scope[tel].ped[pix]=0;
      //Set them to their values in p.e. if the d.c./p.e. factor is non zero
      if(dc2pe!=0) 
      {
        rtn.scope[tel].q[pix]=fData->getData()->getSums()[pix]/dc2pe;
        //rtn.scope[tel].ped[pix]=fData->getData()->getPeds()[pix]/dc2pe;
        rtn.scope[tel].ped[pix]=fData->getData()->getPedvarsSmall()[pix]*frogs_pedwidth_correction/dc2pe;
      }
    }
    //Total number of live pixels in the array
    rtn.nb_live_pix_total=rtn.nb_live_pix_total+rtn.scope[tel].nb_live_pix;
  } 

  //Optimization starting point todo y -> -y ??
  rtn.startpt.xs=fData->getShowerParameters()->fShower_Xoffset[5];
  rtn.startpt.ys=-1.0*fData->getShowerParameters()->fShower_Yoffset[5];

  rtn.startpt.xp=fData->getShowerParameters()->fShowerXcore_SC[5];
  rtn.startpt.yp=fData->getShowerParameters()->fShowerYcore_SC[5];
//  rtn.startpt.xp=fData->getShowerParameters()->fShowerXcore[5];
//  rtn.startpt.yp=fData->getShowerParameters()->fShowerYcore[5];
  if (FROGSDEBUG) {
    printf("ShowerSC %f %f\n",getShowerParameters()->fShowerXcore_SC[5],getShowerParameters()->fShowerYcore_SC[5]);
    printf("Shower %f %f\n",getShowerParameters()->fShowerXcore[5],getShowerParameters()->fShowerYcore[5]);
  }

  rtn.startpt.lambda=0.3; //We use a fixed value by lack of information. 

  rtn.startpt.log10e = inEnergy; //inEnergy from ED 
  if( rtn.startpt.log10e > 0.0 )
    rtn.startpt.log10e = log10(rtn.startpt.log10e);
  else
    rtn.startpt.log10e = FROGS_BAD_NUMBER;

  //Decides if the event is worth analysing. 
  rtn.worthy_event=FROGS_OK;
  //Log(0.1)=-1; Log(0.15)=-0.824; Log(0.2)=-0.699; Log(0.25)=-0.602
  //Log(0.3)=-0.523; Log(0.35)=-0.456; Log(0.4)=-0.398 
  //Energy large enough? 
  if(rtn.startpt.log10e<-0.699)   rtn.worthy_event=FROGS_NOTOK;
  //Energy small enough? 
  if(rtn.startpt.log10e>1.0)   rtn.worthy_event=FROGS_NOTOK;
  //Distance of the impact point small enough? 
  if(sqrt(rtn.startpt.xp*rtn.startpt.xp+rtn.startpt.yp*rtn.startpt.xp)>350.0) 
    rtn.worthy_event=FROGS_NOTOK;
  //Count the number of telescopes with more than 300dc in their image
  int ngoodimages=0;
  for(int tel=0; tel<rtn.ntel;tel++) 
    if(fData->getImageParameters()->size>300.0) ngoodimages=ngoodimages+1;
  //Require the number of telescopes with more than 300dc to be at least 3
  if (ngoodimages<3) rtn.worthy_event=FROGS_NOTOK;

  return rtn;

}

