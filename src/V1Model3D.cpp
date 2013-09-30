/*! \class V1Model3D
     \brief V1Model3D class for 3D-reconstruction of showers
            based on Lemoine-Goumard et al. 2006
            adapted by J. Grube and G. Gyuk
*/

#include "V1Model3D.h"

//////////
void* pfitModel3D;   // global pointer to object
int filter3D[10]; // TEST: filter data
//////////

V1Model3D::V1Model3D()
{
  fData3D = new V1Model3DData();
  fInitialized3D = false;

  fData3D->fBadNumber3D = -999;
  fData3D->fexNoise3D = 0.35; // HARD-WIRED
  fData3D->fDCPE = 5.5; // HARD-WIRED (need implementation)

  fData3D->fDebug3D = false; // set true for debug mode

  //fEmissionHeightCalculator = new VEmissionHeightCalculator(); // TEST height
}

V1Model3D::~V1Model3D()
{
  delete fData3D;
}

/*!
     called for every event
*/
void V1Model3D::doModel3D()
{
  //// initialize only at first call in the analysis run ////
  if( !fInitialized3D ) {
    vector<unsigned int> iNpix3D;
    iNpix3D.resize( fData->getNTel(), 0 );
    for (unsigned int i = 0; i < fData->getNTel(); i++) {
      iNpix3D[i] = getDetectorGeo()->getX( i ).size();
    }
    /// initialize data ///
    fData3D->initModel3D( fData->getNTel(), iNpix3D ); 

    //// get detector configuration ///
    for (unsigned int i = 0; i < fData->getNTel(); i++) {
      getDetector( i ); 
    }
    /// check telescopes are not in sub-arrays or convergent pointing ///
    fData3D->fGoodModel3D = checkModel3D(); 

    fInitialized3D = true;
  }

  if( !fData3D->fGoodModel3D ) {
    cout<< "WARNING: V1Model3D: not using Model3D analysis due to sub-array or convergent pointing mode" << endl;
    return;
  }
  
  /////// HARD CUTS ON 3,4 Tel events, Size cut above 400 GeV ////////
  //int recID = 0;
  //ULong64_t recBitCode = fData->getShowerParameters()->fTelIDImageSelected_bitcode[recID];
  // cout<<"Rec 8 images:" << recBitCode <<endl;
  /// exit if less than 3 number of telescopes passing recID cuts ///
//   if( recBitCode != 7 && recBitCode != 11 && recBitCode < 13 ) {
//     if( fData3D->fDebug3D ) cout<<"skipping Model"<<endl;
//     return;
//   }
 /////////////////////////////////////

  /// initialize event ///
  fData3D->initEventModel3D();   

  /// setup Model3D analysis ///
  calcPointing();        // get pointing 
  calcCoord();           // convert coordinates

  getHillasParameters(); // get Hillas parameters 

  calcEmissionHeight();  // get emission height
  calcStartParameters(); // get start parameters  

  /// calcHeightMethod2(); //TEST

  setSelectedPixels();    // pixels used in Model3D analysis

  if( fData3D->fDebug3D ) {
    cout<<"--- event "<< fData->getShowerParameters()->eventNumber <<" ---"<<endl;
    cout<<"total number of live pixels = "<< fData3D->fNtotPixel3D << endl;
    cout<<"Start: "<<fData3D->fStartSel3D<<" "<<fData3D->fStartSaz3D<<" "<<fData3D->fStartXcore3D<<" "<<fData3D->fStartYcore3D<<" "<<fData3D->fStartSmax3D<<" "<<fData3D->fStartsigmaL3D<<" "<<fData3D->fStartsigmaT3D<<" "<<fData3D->fStartNc3D <<endl; //TEST
  }

  // for each telescope and pixel 
  for (unsigned int i = 0; i < fData3D->fNTel3D; i++) {
    for( unsigned int j = 0; j < fData3D->fNpix3D[i]; j++ ) {
      calcPvector( i, j );
    }
  }

  /// select between MINUIT or GSL fit analysis ///
  //doFitMinuit3D();
  //doFitGSL3D();

  doTestModel3D(); // test output of 3D model for fixed parameters

  /// write model parameters to shower tree ///
  //writeParameters3D(); 
}

/////////////////////////////////////////////////////////////////////

void V1Model3D::doTestModel3D()
{
  // cout.precision(10);
  cout<<"VOLD Par: "<<fData3D->fStartSel3D<<" "<<fData3D->fStartSaz3D<<" "<<fData3D->fStartXcore3D<<" "<<fData3D->fStartYcore3D<<" "<<fData3D->fStartSmax3D<<" "<<fData3D->fStartsigmaL3D<<" "<<fData3D->fStartsigmaT3D<<" "<<fData3D->fStartNc3D <<endl; //TEST

  fData3D->fSel3D    =  fData3D->fStartSel3D;
  fData3D->fSaz3D    =  fData3D->fStartSaz3D; 
  fData3D->fXcore3D  =  fData3D->fStartXcore3D; 
  fData3D->fYcore3D  =  fData3D->fStartYcore3D; 
  fData3D->fSmax3D   =  fData3D->fStartSmax3D; 
  fData3D->fsigmaL3D =  fData3D->fStartsigmaL3D; 
  fData3D->fsigmaT3D =  fData3D->fStartsigmaT3D;
  fData3D->fNc3D     =  fData3D->fStartNc3D;

  double mu = 0;

  V1FitModel3D *fFitModel3D = new V1FitModel3D( fData3D );
  fFitModel3D->posFit3D(); // call member

  for( unsigned int j = 0; j < fData3D->fNpix3D[0]; j++ ) {

    mu = fFitModel3D->calcModel3D( 0, j );
    if( mu > 20 ) cout<< "VOLD: T1 ch"<< j <<" mu = "<< mu <<endl; //TEST
  }
}

void V1Model3D::doFitMinuit3D()
{
  pfitModel3D = (void *) new  V1FitModel3D( fData3D );
  ROOT::Math::Functor f(&V1FitModel3D::wrapFitMinuit3D,8); 

  // starting point    
  //// TEST cosmic-ray ////
  ///fData3D->fStartsigmaL3D    = 3000.0; // FIXED for cosmic-rays

  double variable[8] = { fData3D->fStartSel3D, fData3D->fStartSaz3D, fData3D->fStartXcore3D, fData3D->fStartYcore3D, fData3D->fStartSmax3D, fData3D->fStartsigmaL3D, fData3D->fStartsigmaT3D, fData3D->fStartNc3D };

  /// double stepsize = 0.01;  // best?
  /// double step[8] = { stepsize, stepsize, stepsize, stepsize, stepsize, stepsize, stepsize, stepsize };

  ///// TEST cosmic-ray ////
  // fData3D->fStepSel3D    = 0.0; // best 0.1
  // fData3D->fStepSaz3D    = 0.0; // best 1.0
  // fData3D->fStepXcore3D  = 0.0; // best 1.0
  // fData3D->fStepYcore3D  = 0.0; // best 1.0
  // fData3D->fStepSmax3D   = 1e4; // best 100 or 1000?
  // fData3D->fStepsigmaL3D = 1e4; // best 500 or 1000?
  // fData3D->fStepsigmaT3D = 1.0; // best 0.5 or 0.1?
  // fData3D->fStepNc3D     = 0.7; // LOG best 1.0 or 0.5?

  ///// bad? gamma-ray ////
  // fData3D->fStepSel3D    = 2.0; // def 1.0
  // fData3D->fStepSaz3D    = 7.0; // def 10.0
  // fData3D->fStepXcore3D  = 50.0; // def 10.0
  // fData3D->fStepYcore3D  = 50.0; // def 10.0
  // fData3D->fStepSmax3D   = 500.0; // def 100.0
  // fData3D->fStepsigmaL3D = 500.0; // def 100.0
  // fData3D->fStepsigmaT3D = 2.0; // def 1.0
  // fData3D->fStepNc3D     = 3.0; // def 1.0
  ///// BEST gamma-ray ////
  fData3D->fStepSel3D    = 0.1; // best 0.5
  fData3D->fStepSaz3D    = 0.1; // best 0.5
  fData3D->fStepXcore3D  = 1.0; // best 1.0
  fData3D->fStepYcore3D  = 1.0; // best 1.0
  fData3D->fStepSmax3D   = 100; // best 500
  fData3D->fStepsigmaL3D = 100; // best 500
  fData3D->fStepsigmaT3D = 1.0; // best 0.8
  fData3D->fStepNc3D     = 0.1; // LOG best 0.1

  //fData3D->fStepNc3D     = 0.8; // LOG best 1.0 or 0.5?

  ///////fData3D->fStepNc3D     = 0.1e6; // LIN best 0.5e6

  double step[8] = { fData3D->fStepSel3D, fData3D->fStepSaz3D, fData3D->fStepXcore3D, fData3D->fStepYcore3D, fData3D->fStepSmax3D, fData3D->fStepsigmaL3D, fData3D->fStepsigmaT3D, fData3D->fStepNc3D };

  ///cout<< "event "<< getShowerParameters()->eventNumber <<" starting values"<<endl;
  ///cout<<fData3D->fSel3D<<" "<<fData3D->fSaz3D<<" "<<fData3D->fXcore3D<<" "<<fData3D->fYcore3D<<" "<<fData3D->fSmax3D<<" "<<fData3D->fsigmaL3D<<" "<<fData3D->fsigmaT3D<<" "<<fData3D->fNc3D <<endl;


  ////// DON'T need first fit //////////////////////////
//
//  const char *minName  = "Minuit"; 
//  //const char *algoName = "Migrad";
//  const char *algoName = "Simplex";
//
//  ROOT::Math::Minimizer* min = ROOT::Math::Factory::CreateMinimizer(minName, algoName);
//
//  min->SetStrategy(1); // fast: 0, default: 1, slow,precise: 2
//  min->SetTolerance(0.1); // original
//
//  min->SetPrintLevel(0); // set to "0" for no printing
//  min->SetFunction(f);
//  min->SetMaxFunctionCalls(10); // ORIGINAL for Minuit/Minuit2
//
//  ////// TEST cosmic-ray (fixed) //////
//  min->SetFixedVariable(0,"Sel",    variable[0]);
//  min->SetFixedVariable(1,"Saz",    variable[1]);
//  min->SetFixedVariable(2,"Xcore",  variable[2]);
//  min->SetFixedVariable(3,"Ycore",  variable[3]);
//  min->SetLimitedVariable(4,"Smax",   variable[4], step[4], 2000, 25000);
//  min->SetFixedVariable(5,"sigmaL",  3000.);
//  min->SetLimitedVariable(6,"sigmaT", variable[6], step[6], 0, 80); 
//  min->SetLimitedVariable(7,"logNc",   variable[7], step[7], 10., 20. ); //LOG
//
//  min->Minimize(); // DO MINIMIZATION
//
//  if( fData3D->fDebug3D ) cout<<"First:  "<<fData3D->fSel3D<<" "<<fData3D->fSaz3D<<" "<<fData3D->fXcore3D<<" "<<fData3D->fYcore3D<<" "<<fData3D->fSmax3D<<" "<<fData3D->fsigmaL3D<<" "<<fData3D->fsigmaT3D<<" "<<fData3D->fNc3D <<endl; //TEST
//
//  ////// REPEAT ? ///////
// 
//  const double *xs0 = min->X();
//
//  float fit1sigmaT = xs0[6];
//
/////////////////////////////////////////////////

  ///only proceed with additional fit if 3D-width is small (gamma-like) //
  if( fData3D->fStartsigmaT3D < 25. && fData3D->fStartsigmaT3D > 5. ) {
    /// if( fit1sigmaT < 25. && fit1sigmaT > 5. && fData3D->fGoodness3D < 100. ) {
    // cout<<"GAMMA-LIKE"<<endl;

  /////// TEST Repeat //////
  ROOT::Math::Minimizer* min2 = ROOT::Math::Factory::CreateMinimizer("Minuit", "Migrad");

  min2->SetStrategy(1); // fast: 0, default: 1, slow,precise: 2
  min2->SetTolerance(0.1); // original
  min2->SetPrintLevel(0); // set to "0" for no printing
  min2->SetFunction(f);
  min2->SetMaxFunctionCalls(200); // ORIGINAL for Minuit/Minuit2
  
  ///// BEST gamma-ray (limited) //////////////////
  // min2->SetLimitedVariable(0,"Sel",    variable[0], step[0], 0, 90 );
  // min2->SetLimitedVariable(1,"Saz",    variable[1], step[1], -180, 180 );
  // min2->SetLimitedVariable(2,"Xcore",  variable[2], step[2], -1000, 1000 );
  // min2->SetLimitedVariable(3,"Ycore",  variable[3], step[3], -1000, 1000 );
  // min2->SetLimitedVariable(4,"Smax",        xs0[4], step[4], 1000, 20000);
  // min2->SetLimitedVariable(5,"sigmaL", variable[5], step[5], 100, 10000);
  // min2->SetLimitedVariable(6,"sigmaT", variable[6], step[6], 0, 80); 
  // min2->SetLimitedVariable(7,"logNc",       xs0[7], step[7], 10., 20. ); //LOG
  //////min->SetLimitedVariable(7,"Nc",   variable[7], step[7], 1e3, 1e10 ); //LIN

  //// ONLY STARTING ///////
  min2->SetLimitedVariable(0,"Sel",    variable[0], step[0], 0, 90 );
  min2->SetLimitedVariable(1,"Saz",    variable[1], step[1], -180, 180 );
  min2->SetLimitedVariable(2,"Xcore",  variable[2], step[2], -1000, 1000 );
  min2->SetLimitedVariable(3,"Ycore",  variable[3], step[3], -1000, 1000 );
  min2->SetLimitedVariable(4,"Smax",   variable[4], step[4], 1000, 20000);
  min2->SetLimitedVariable(5,"sigmaL", variable[5], step[5], 100, 10000);
  min2->SetLimitedVariable(6,"sigmaT", variable[6], step[6], 0, 80); 
  min2->SetLimitedVariable(7,"logNc",  variable[7], step[7], 10., 20. ); //LOG

  ////min2->SetVariable(0,"Sel",    xs0[0], step[0]);
  ////min2->SetVariable(1,"Saz",    xs0[1], step[1]);
  ////min2->SetVariable(2,"Xcore",  xs0[2], step[2]);
  ////min2->SetVariable(3,"Ycore",  xs0[3], step[3]);
  ////min2->SetVariable(4,"Smax",   xs0[4], step[4]);
  ////min2->SetVariable(5,"sigmaL", xs0[5], step[5]);
  ////min2->SetVariable(6,"sigmaT", xs0[6], step[6]);
  ////min2->SetVariable(7,"Nc",     xs0[7], step[7]);
  
  min2->Minimize(); // DO MINIMIZATION

  if( fData3D->fDebug3D ) cout<<"Best:  "<<fData3D->fSel3D<<" "<<fData3D->fSaz3D<<" "<<fData3D->fXcore3D<<" "<<fData3D->fYcore3D<<" "<<fData3D->fSmax3D<<" "<<fData3D->fsigmaL3D<<" "<<fData3D->fsigmaT3D<<" "<<fData3D->fNc3D <<endl; //TEST
  if( fData3D->fDebug3D ) cout<< "GAMMA-like GOODNESS: "<< fData3D->fGoodness3D <<endl;

  // NOT NEEDED? get best-fit values // 
  // const double *xs1 = min->X();
  // fData3D->fSel3D = xs1[0];
  // fData3D->fSaz3D = xs1[1];
  // fData3D->fXcore3D = xs1[2];
  // fData3D->fYcore3D = xs1[3];
  // fData3D->fSmax3D = xs1[4];
  // fData3D->fsigmaL3D = xs1[5];
  // fData3D->fsigmaT3D = xs1[6];
  // fData3D->fNc3D = xs1[7];

  ///// TEST /////
  // one instance of fitmodel3D
  //fFitModel3D->calcTotalModel3D(); // u = ln( Qmodel3D/Qmeas ) and model display

  ///cout<< "event "<< getShowerParameters()->eventNumber <<" final values"<<endl;
  //// cout<<fData3D->fSel3D<<" "<<fData3D->fSaz3D<<" "<<fData3D->fXcore3D<<" "<<fData3D->fYcore3D<<" "<<fData3D->fSmax3D<<" "<<fData3D->fsigmaL3D<<" "<<fData3D->fsigmaT3D<<" "<<fData3D->fNc3D <<endl;
  /// cout<<"uQality: "<< fData3D->fuQuality3D <<endl;
  ////cout<< "event "<< getShowerParameters()->eventNumber <<" goodness = "<< fData3D->fGoodness3D << endl;
  // cout<< "event (after): " << getShowerParameters()->eventNumber <<" "<< fData3D->fSel3D<< " " << fData3D->fSaz3D<< " " << fData3D->fXcore3D<< " " << fData3D->fYcore3D<< " " << fData3D->fSmax3D<< " " << fData3D->fsigmaL3D<< " " << fData3D->fsigmaT3D<< " " << fData3D->fNc3D <<endl;

  }
  else {
    if( fData3D->fDebug3D ) cout<<"COSMIC-like"<<endl;

    fData3D->fSel3D =   fData3D->fStartSel3D;
    fData3D->fSaz3D =   fData3D->fStartSaz3D;
    fData3D->fXcore3D = fData3D->fStartXcore3D;
    fData3D->fYcore3D = fData3D->fStartYcore3D;
    fData3D->fSmax3D =  fData3D->fStartSmax3D;
    fData3D->fsigmaL3D =fData3D->fStartsigmaL3D;
    fData3D->fsigmaT3D =fData3D->fStartsigmaT3D;
    fData3D->fNc3D =    fData3D->fStartNc3D;

  }

  return;
}

void V1Model3D::doFitGSL3D()
{

  unsigned int fNumMaxIter = 50; // TEST

  // use global pointer for V1FitModel3D object //
  pfitModel3D = (void *) new  V1FitModel3D( fData3D );

  // put the starting point into a GSL vector. 
  const size_t p = 8; // dimension of the parameter space
  gsl_vector *x;
  x = gsl_vector_alloc (p);

  gsl_vector_set (x, 0, fData3D->fStartSel3D   );    
  gsl_vector_set (x, 1, fData3D->fStartSaz3D   );    
  gsl_vector_set (x, 2, fData3D->fStartXcore3D );  
  gsl_vector_set (x, 3, fData3D->fStartYcore3D );  
  gsl_vector_set (x, 4, fData3D->fStartSmax3D  );   
  gsl_vector_set (x, 5, fData3D->fStartsigmaL3D); 
  gsl_vector_set (x, 6, fData3D->fStartsigmaT3D); 
  gsl_vector_set (x, 7, fData3D->fStartNc3D    );  

  //Prepare function minimization 
  const size_t n = fData3D->fNtotPixel3D; // total number of pixels in fit

  /// set data in structure ///
  //stData3D data;
  //data.n3D = n;

  gsl_multifit_function_fdf func;

  func.n = n; // number of pixels used
  func.p = p; // Dimension of the parameter space

  func.f = &V1FitModel3D::wrapFitGSL3D;   // function to be minimized
  func.df = &V1FitModel3D::wrapDF3D;   // likelihood_derivative
  func.fdf = &V1FitModel3D::wrapFDF3D; // GSL function

  /// func.params = (void *)param;        // not needed?

  const gsl_multifit_fdfsolver_type *T;
  gsl_multifit_fdfsolver *s;

  T = gsl_multifit_fdfsolver_lmsder;
  s = gsl_multifit_fdfsolver_alloc(T, n, p);

  gsl_multifit_fdfsolver_set(s, &func, x);
    
  /// loop over fit iterations ///
  int status=0;
  unsigned int iter = 0;
  do {
    iter++;  
    status = gsl_multifit_fdfsolver_iterate(s);

    cout<< "ITER: "<<iter<<" "<<gsl_vector_get (s->x, 0)<<endl;

    if (status) break;
    status = gsl_multifit_test_delta (s->dx, s->x, 1e-7, 1e-7);
    cout<<"MEGA FAIL, status = " <<status<<endl; 
  } while (status == GSL_CONTINUE && iter < fNumMaxIter ); /// FIX: set max number of iterations
  
  /// if the maximum number of iteration is reached return a null result ///
  if( iter == fNumMaxIter ) {  
    /// rtn.gsl_convergence_status=status;
    /// rtn.nb_iter=iter;
    gsl_multifit_fdfsolver_free(s);
    return;
  } 

// get the best-fit values
   fData3D->fSel3D    = gsl_vector_get (s->x, 0);    
   fData3D->fSaz3D    = gsl_vector_get (s->x, 1);    
   fData3D->fXcore3D  = gsl_vector_get (s->x, 2);  
   fData3D->fYcore3D  = gsl_vector_get (s->x, 3);  
   fData3D->fSmax3D   = gsl_vector_get (s->x, 4);   
   fData3D->fsigmaL3D = gsl_vector_get (s->x, 5); 
   fData3D->fsigmaT3D = gsl_vector_get (s->x, 6); 
   fData3D->fNc3D     = gsl_vector_get (s->x, 7);  

   if( fData3D->fDebug3D ) {
     cout<< "Goodness = "<< fData3D->fGoodness3D <<endl;
     cout<<"PAR:  "<<fData3D->fSel3D<<" "<<fData3D->fSaz3D<<" "<<fData3D->fXcore3D<<" "<<fData3D->fYcore3D<<" "<<fData3D->fSmax3D<<" "<<fData3D->fsigmaL3D<<" "<<fData3D->fsigmaT3D<<" "<<fData3D->fNc3D <<endl; //TEST
   }
   ////// get fit status
//  rtn.gsl_convergence_status=status;
//  rtn.event_id=d->event_id;
//  rtn.nb_iter=iter;
//
//  /*GSL take lambda to any value but only the value used in the model 
//    calculation is directly meaningfull. Here, we convert the value set 
//    by GSL to the actual value used in the model calculation. */
//  float maxlambda=tmplt->min[0]+(tmplt->nstep[0]-1)*tmplt->step[0];
//  rtn.cvrgpt.lambda=floatwrap(rtn.cvrgpt.lambda,tmplt->min[0],maxlambda);

  gsl_matrix *covar = gsl_matrix_alloc (p, p);
  gsl_multifit_covar(s->J, 0.0, covar);
    
  //Get the error for each parameter
  // float chi = gsl_blas_dnrm2(s->f);
  // float dof = n - p;
  // float c = GSL_MAX_DBL(1, chi/sqrt(dof));

//#define ERR(i) sqrt(gsl_matrix_get(covar, i, i))
//  //ERR(i) is nan when iter==1. We then change the error to FROGS_BAD_NUMBER
//  rtn.cvrgpterr.xs = c*ERR(FROGS_XS); 
//  if(!frogs_is_a_good_number(rtn.cvrgpterr.xs)) 
//    rtn.cvrgpterr.xs=FROGS_BAD_NUMBER;
//  rtn.cvrgpterr.ys = c*ERR(FROGS_YS);
//  if(!frogs_is_a_good_number(rtn.cvrgpterr.ys)) 
//    rtn.cvrgpterr.ys=FROGS_BAD_NUMBER;
//  rtn.cvrgpterr.xp = c*ERR(FROGS_XP); 
//  if(!frogs_is_a_good_number(rtn.cvrgpterr.xp)) 
//    rtn.cvrgpterr.xp=FROGS_BAD_NUMBER;
//  rtn.cvrgpterr.yp = c*ERR(FROGS_YP); 
//  if(!frogs_is_a_good_number(rtn.cvrgpterr.yp)) 
//    rtn.cvrgpterr.yp=FROGS_BAD_NUMBER;
//  rtn.cvrgpterr.log10e = c*ERR(FROGS_LOG10E); 
//  if(!frogs_is_a_good_number(rtn.cvrgpterr.log10e)) 
//    rtn.cvrgpterr.log10e=FROGS_BAD_NUMBER;  
//  rtn.cvrgpterr.lambda = c*ERR(FROGS_LAMBDA); 
//  if(!frogs_is_a_good_number(rtn.cvrgpterr.lambda)) 
//    rtn.cvrgpterr.lambda=FROGS_BAD_NUMBER;
//
//  /*Calculate the image and background goodness for the convergence point.*/ 
//  if(frogs_goodness(&rtn,d,tmplt)!=FROGS_OK) 
//    frogs_showxerror("Problem encountered in the convergence goodness calculation");

  gsl_multifit_fdfsolver_free(s);
  gsl_matrix_free(covar);

///////////////

  return;
}

///////// TEST ////////////////////////

void V1Model3D::setSelectedPixels()
{

  fData3D->fNtotPixel3D = 0;

  // for each telescope and pixel 
  for (unsigned int i = 0; i < fData3D->fNTel3D; i++) {

    initializeDataReader();
    setTelID(i);

    /// TEST ///
    // double imageThresh = 3.0;
    // double borderThresh = 1.5;
    // double nonImageThresh = 2.5;
    // double timeCutPixel = 0.5;
    // double timeCutCluster = 2.0;
    // int minNumPixelsInCluster = 3;
    // int numLoops = 3;
    // fCleaning->cleanImagePedvarsWithTiming(imageThresh, borderThresh, nonImageThresh, timeCutPixel, timeCutCluster, minNumPixelsInCluster, numLoops );

    for( unsigned int j = 0; j < fData3D->fNpix3D[i]; j++ ) {

      /// TEST use ALL non-dead pixels
      //if( fData->getDead()[j] == 0 ) {
      //fData3D->fClean3D[i][j] = true;
	// fData3D->fNtotPixel3D += 1;
      //}

      /// fill measured signal and pedvar ///
      if( fData3D->fDCPE > 0 ) {
	fData3D->fMeasuredSum3D[i][j] = fData->getData()->getSums()[j] / fData3D->fDCPE;
	fData3D->fPedvar3D[i][j] = fData->getData()->getPeds()[j] / fData3D->fDCPE;
      }

      /// same cleaning as Hillas ///
      if( fData->getData()->getImage()[j] || fData->getData()->getBorder()[j] ) {
	fData3D->fClean3D[i][j] = true;
	fData3D->fNtotPixel3D += 1;

//	//// also select neighbors of border and image pixels ////
// 	for( unsigned int k=0; k < getDetectorGeo()->getNeighbours()[j].size(); k++ ) {
// 	  unsigned int n = getDetectorGeo()->getNeighbours()[j][k];
// 	  if ( !fData->getDead(fData->getHiLo()[j])[j] && !fData->getData()->getImage()[n] && !fData->getData()->getBorder()[n] && n < fData->getData()->getImage().size() ) {
// 	    fData3D->fClean3D[i][n] = true;
// 	    fData3D->fNtotPixel3D += 1;
// 	  }
// 	}

      }
    }
  }

  ///  if( fData3D->fDebug3D ) cout<<"Npix = "<< fData3D->fNtotPixel3D <<endl;
}

// void V1Model3D::setDataGSL()
// {
// 
//   Sn3D = fData3D->fNtotPixel3D;
//   Sq3D = new double[Sn3D];
//   Sped3D = new double[Sn3D];
// 
//   size_t Icount = 0;
// 
//   for (unsigned int i = 0; i < fData3D->fNTel3D; i++) {
//     for( unsigned int j = 0; j < fData3D->fNpix3D[i]; j++ ) {
//       if( fData3D->fClean3D[i][j] ) {
// 	Sq3D[Icount] = fData3D->fMeasuredSum3D[i][j];
// 	Sped3D[Icount] = fData3D->fPedvar3D[i][j];
// 	Icount += 1;
//       }
//     }
//   }
// 
// }

////////////////////
void V1Model3D::getHillasParameters()
{
  for (unsigned int i = 0; i < fData3D->fNTel3D; i++) {

    initializeDataReader();
    setTelID(i);

    fData3D->fcen_x3D[i] = fData->getImageParameters()->cen_x;
    fData3D->fcen_y3D[i] = fData->getImageParameters()->cen_y;
    fData3D->fsize3D[i] = fData->getImageParameters()->size;
    fData3D->fcosphi3D[i] = fData->getImageParameters()->cosphi;
    fData3D->fsinphi3D[i] = fData->getImageParameters()->sinphi;
    fData3D->flength3D[i] = fData->getImageParameters()->length;
    fData3D->fwidth3D[i] = fData->getImageParameters()->width;

    /// calculate position along semi-major (length) axis 
    /// for points closest and furthest to reconstructed source position
    fData3D->flengthInX3D[i] = fData3D->fcen_x3D[i] - (fData3D->flength3D[i] * fData3D->fcosphi3D[i]);
    fData3D->flengthInY3D[i] = fData3D->fcen_y3D[i] - (fData3D->flength3D[i] * fData3D->fsinphi3D[i]);
    fData3D->flengthOutX3D[i] = fData3D->fcen_x3D[i] + (fData3D->flength3D[i] * fData3D->fcosphi3D[i]);
    fData3D->flengthOutY3D[i] = fData3D->fcen_y3D[i] + (fData3D->flength3D[i] * fData3D->fsinphi3D[i]);

    /// compare distance of closest and furthest to source location
//     if( fData3D->fsize3D[i] > 0 ) {
//       srcdistIn = imageDistance( fData3D->flengthInX3D[i], fData->getShowerParameters()->fShower_Xoffset[0], fData3D->flengthInY3D[i], fData->getShowerParameters()->fShower_Yoffset[0] );
//       srcdistOut = imageDistance( fData3D->flengthOutX3D[i], fData->getShowerParameters()->fShower_Xoffset[0], fData3D->flengthOutY3D[i], fData->getShowerParameters()->fShower_Yoffset[0] );
// 
//       cout<<"T"<<i+1<<" dist-in  = "<<srcdistIn<<endl; 
//       cout<<"T"<<i+1<<" dist-out = "<<srcdistOut<<endl; 
//     }

  }

}
////////////////////

void V1Model3D::writeParameters3D()
{

  getModelDirection(); //TEST

  /// write start parameters to file ///
  fData->getShowerParameters()->fStartSel3D = fData3D->fStartSel3D;    
  fData->getShowerParameters()->fStartSaz3D = fData3D->fStartSaz3D;    
  fData->getShowerParameters()->fStartXcore3D = fData3D->fStartXcore3D;  
  fData->getShowerParameters()->fStartYcore3D = fData3D->fStartYcore3D;  
  fData->getShowerParameters()->fStartSmax3D = fData3D->fStartSmax3D;   
  fData->getShowerParameters()->fStartsigmaL3D = fData3D->fStartsigmaL3D; 
  fData->getShowerParameters()->fStartsigmaT3D = fData3D->fStartsigmaT3D; 
  fData->getShowerParameters()->fStartNc3D = fData3D->fStartNc3D;  

  /// write best-fit parameters to file ///
  fData->getShowerParameters()->fSel3D = fData3D->fSel3D;    
  fData->getShowerParameters()->fSaz3D = fData3D->fSaz3D;    
  fData->getShowerParameters()->fXcore3D = fData3D->fXcore3D;  
  fData->getShowerParameters()->fYcore3D = fData3D->fYcore3D;  
  fData->getShowerParameters()->fSmax3D = fData3D->fSmax3D;   
  fData->getShowerParameters()->fsigmaL3D = fData3D->fsigmaL3D; 
  fData->getShowerParameters()->fsigmaT3D = fData3D->fsigmaT3D; 
  fData->getShowerParameters()->fNc3D = fData3D->fNc3D;  
  /// fit quality ///
  fData->getShowerParameters()->fGoodness3D = fData3D->fGoodness3D;
  ///fData->getShowerParameters()->fuQuality3D = fData3D->fuQuality3D;

  /// plotting purposes ///
  if( getRunParameter()->fUseDisplayModel3D ) {

    for (unsigned int i = 0; i < fData3D->fNTel3D; i++) {
      setTelID(i);
      valarray<double> a0( -5, fData3D->fNpix3D[i] );

      for( unsigned int j = 0; j < fData3D->fNpix3D[i]; j++ ) {
	a0[j] = V1FitModel3D::wrapCalcModel3D( i, j ) * fData3D->fDCPE;
      }
      fData->setModel3DMu( a0 );
    }
  }

}

//// get p-hat vector (line of sight) //////////////////////////

void V1Model3D::calcPvector( unsigned int iTel, unsigned int iPix )
{
  //// add pixel "p" vector to "T" vector in ground coordinates ////
  double tp[3]; // temp p vector

  //// convert from deg to rad ////
  tp[0] = getDetectorGeo()->getX( iTel )[iPix] * (TMath::Pi()/180.);
  tp[1] = getDetectorGeo()->getY( iTel )[iPix] * (TMath::Pi()/180.);
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

///////////////////////////////////

void V1Model3D::calcHeightMethod2() //TEST
{
  double degrad = 45. / atan( 1. );

  // everything in shower coordinates
  float coreXSC,coreYSC;
  float telXSC[4];
  float telYSC[4];
  float telZSC[4];

  float Xoff, Yoff, alpha;
  float iHeightWeightTemp = 0;
  float iHeightWeight = 0;

  float fRSC[4]; // distance between shower core and telescope 
  float fheightTel[4]; // height estimate for each telescope
  float fheight = 0;

  coreXSC = fData->getShowerParameters()->fShowerXcore_SC[0];
  coreYSC = fData->getShowerParameters()->fShowerYcore_SC[0];

  Xoff = fData->getShowerParameters()->fShower_Xoffset[0];
  Yoff = fData->getShowerParameters()->fShower_Yoffset[0];
  alpha = sqrt( Xoff*Xoff + Yoff*Yoff );

  cout<<"--- event "<< fData->getShowerParameters()->eventNumber <<" ---"<<endl;
  for (unsigned int i = 0; i < fData3D->fNTel3D; i++) {
    telXSC[i] = fData->getShowerParameters()->fTel_x_SC[i];
    telYSC[i] = fData->getShowerParameters()->fTel_y_SC[i];
    telZSC[i] = fData->getShowerParameters()->fTel_z_SC[i];

    fRSC[i] = line_point_distance( coreYSC, -1.*coreXSC, 0., fData3D->fTze3D[i], fData3D->fTaz3D[i], telYSC[i], -1.*telXSC[i], telZSC[i] );

    fheightTel[i] = fRSC[i] / tan( alpha / degrad );
    cout<<"T"<<i+1<<" R = "<< fRSC[i] <<" H = "<< fheightTel[i] <<endl;

    if( fData3D->fsize3D[i] > 0 ) {
      iHeightWeightTemp = log10( fData3D->fsize3D[i] );
      iHeightWeight += iHeightWeightTemp;
      fheight += fheightTel[i] * iHeightWeightTemp;
    }
  }

  if( iHeightWeight > 0. ) {
    fheight = fheight / iHeightWeight;
  }
  else fheight = 0;

  cout<<"aH = "<< fheight <<endl;
  cout<<"oH = "<< fData3D->fStartSmax3D <<endl;
}
///////////////

void V1Model3D::calcEmissionHeight()
{
  
  double degrad = 45. / atan( 1. );

  // reset emission heights
  // fData3D->fSmax3D = 0.;

  //////
  double TelescopeDistanceSC = 0.;
  double ImageDistance = 0.; // counter for telescope pairs
  int nTPair = 0;

  double iEmissionHeight = 0.;
  double iEmissionHeightWeight = 0.;
  double iEmissionHeightWeightTemp = 0.;
  double iEmissionHeight2 = 0.;
  double iEmissionHeightTemp = 0.;

  double iDistLengthIn = 0;
  double iDistLengthOut = 0;
  double iHeightLengthIn = 0;
  double iHeightLengthOut = 0;
  double iHeightLengthInTot = 0;
  double iHeightLengthOutTot = 0;

  /////////////////////////////////
  // loop over all telescope pairs

  for( unsigned int i = 0; i < fData->getNTel(); i++ ) {

    for( unsigned int j = i; j < fData->getNTel(); j++ ) {
	  
      // require elevation > 0. (test if telescope is present in analysis)
      if( i != j && fData3D->fsize3D[i] > 0 && fData3D->fsize3D[j] > 0 ) {
	
	////// 3D-Length //////

	// 3D-Length: get tangents of distance between inner and outer length points
	iDistLengthIn = tan( imageDistance( fData3D->flengthInX3D[i], fData3D->flengthInX3D[j], fData3D->flengthInY3D[i], fData3D->flengthInY3D[j] ) / degrad );
	iDistLengthOut = tan( imageDistance( fData3D->flengthOutX3D[i], fData3D->flengthOutX3D[j], fData3D->flengthOutY3D[i], fData3D->flengthOutY3D[j] ) / degrad );

	// Emission Height: get tangents of distance between the two image centroids 
	ImageDistance = tan( imageDistance( fData3D->fcen_x3D[i], fData3D->fcen_x3D[j], fData3D->fcen_y3D[i], fData3D->fcen_y3D[j] ) / degrad );

	if( ImageDistance > 0. ) {
	  // get distance between the two telescopes in shower coordinates
	  TelescopeDistanceSC = getTelescopeDistanceSC( i, j, fData3D->fTaz3D[i], fData3D->fTze3D[i] );

	  // 3D-Length: calculate height [m]
	  iHeightLengthIn = TelescopeDistanceSC / iDistLengthIn;
	  iHeightLengthOut = TelescopeDistanceSC / iDistLengthOut;

	  // calculate emission height [m]
	  iEmissionHeightTemp = TelescopeDistanceSC / ImageDistance;

	  // weight for pairwise emission height calculation
	  iEmissionHeightWeightTemp = 1./((1./log10( fData3D->fsize3D[i] ))+(1./log10( fData3D->fsize3D[j] )));
	  iEmissionHeightWeight    += iEmissionHeightWeightTemp;
	  iEmissionHeight          += iEmissionHeightTemp * iEmissionHeightWeightTemp;
	  iEmissionHeight2         += iEmissionHeightTemp * iEmissionHeightTemp * iEmissionHeightWeightTemp;

	  // 3D-Length total
	  iHeightLengthInTot += iHeightLengthIn * iEmissionHeightWeightTemp;
	  iHeightLengthOutTot += iHeightLengthOut * iEmissionHeightWeightTemp;

	}
      }
      nTPair++;
    }
  }
      
  if( iEmissionHeightWeight > 0. ) {
    // calculate mean emission height 
    fData3D->fStartSmax3D = iEmissionHeight / iEmissionHeightWeight;
    // calculate mean 3D-Length heights 
    iHeightLengthInTot = iHeightLengthInTot / iEmissionHeightWeight;
    iHeightLengthOutTot = iHeightLengthOutTot / iEmissionHeightWeight;
    fData3D->fStartsigmaL3D = iHeightLengthInTot - iHeightLengthOutTot;
    ///cout<<"In,Out,Length: "<< iHeightLengthInTot <<" , "<< iHeightLengthOutTot <<" , "<< TestLength <<endl;
  }
  else{
    fData3D->fSmax3D = 0.;
    fData3D->fStartsigmaL3D = 0.;
  }
  /// TEST fixed ///
  //fData3D->fStartsigmaL3D = 3000;

  /// 3DWidth estimate ///
  double iWidthWeight = 0;
  double iWidthWeightTemp = 0;
  double iWidth = 0;

  for( unsigned int i = 0; i < fData->getNTel(); i++ ) {
      // test if telescope is present in analysis
    if( fData3D->fsize3D[i] > 0 ) {
      iWidthWeightTemp = log10( fData3D->fsize3D[i] );
      iWidthWeight += iWidthWeightTemp;
      iWidth += iWidthWeightTemp * fData3D->fStartSmax3D * tan( fData3D->fwidth3D[i] / degrad );
    }
  }

  //TEST//cout<<"weight = "<< iWidthWeight <<" ,width = "<< iWidth <<endl;
  /// double scaleWidth = 1.2;

  if( iWidthWeight > 0. ) {
    fData3D->fStartsigmaT3D = iWidth / iWidthWeight;
  }
  else fData3D->fStartsigmaT3D = 0;

}

/////////////

double V1Model3D::getTelescopeDistanceSC( unsigned int iTel1, unsigned int iTel2, double az, double z )
{
  if( iTel1 >= fData3D->flocTel3D[0].size() || iTel2 >= fData3D->flocTel3D[0].size() )  return -999.;
    
  double s[3], t1[3], t2[3];

  double degrad = 45. / atan( 1. );
 
  az /= degrad;
  z  /= degrad;
 
  s[0]    = sin(z)*cos(az);
  s[1]    = sin(z)*sin(az);
  s[2]    = cos(z);
 
  t1[0] = fData3D->flocTel3D[0][iTel1];
  t1[1] = fData3D->flocTel3D[1][iTel1];
  t1[2] = fData3D->flocTel3D[2][iTel1];
 
  t2[0] = fData3D->flocTel3D[0][iTel2];
  t2[1] = fData3D->flocTel3D[1][iTel2];
  t2[2] = fData3D->flocTel3D[2][iTel2];
 
    return line_point_distance( t1[0], t1[1], t1[2], z*degrad, az*degrad, t2[0], t2[1], t2[2] );

}

/////////////

double V1Model3D::line_point_distance(double x1, double y1, double z1, double el, double az, double x, double y, double z)
{
    double a, a1, a2, a3, b;

    az = 180. - az;

    double cx = -1.*cos(el*(TMath::Pi()/180.))*cos(az*(TMath::Pi()/180.));
    double cy = -1.*cos(el*(TMath::Pi()/180.))*sin(az*(TMath::Pi()/180.));
    double cz = sin(el*(TMath::Pi()/180.));

    a1 = (y-y1)*cz - (z-z1)*cy;
    a2 = (z-z1)*cx - (x-x1)*cz;
    a3 = (x-x1)*cy - (y-y1)*cx;
    a  = a1*a1 + a2*a2 + a3*a3;
    b = cx*cx + cy*cy + cz*cz;

    if ( a<0. || b<= 0. ) return -1;

    return sqrt(a/b);
}

//////////////

double V1Model3D::imageDistance( double c1x, double c2x, double c1y, double c2y )
{
  return sqrt( (c1x-c2x)*(c1x-c2x) + (c1y-c2y)*(c1y-c2y) );
}

//////////////

void V1Model3D::getModelDirection()
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

  ////// GOOD, but not sure why (swap Y sign) ///////
  ts[1] = 0. - ts[1];

  ts[2] = 0;

  ts[0] = ts[0] / (TMath::Pi()/180.); // from radians to deg
  ts[1] = ts[1] / (TMath::Pi()/180.); // from radians to deg
  ts[2] = ts[2] / (TMath::Pi()/180.); // from radians to deg

  fData->getShowerParameters()->fXoffModel3D = ts[0];
  fData->getShowerParameters()->fYoffModel3D = ts[1];

  //cout<<"Hillas (X,Y): "<< fData->getShowerParameters()->fShower_Xoffset[0] <<", "<< fData->getShowerParameters()->fShower_Yoffset[0] <<endl;

  //cout<<"Model3D (X,Y): "<< fData->getShowerParameters()->fXoffModel3D <<", "<< fData->getShowerParameters()->fYoffModel3D << endl;

}

////////////////////////////////////

void V1Model3D::calcStartParameters()
{
  /////// Get model starting point ///////////

  /////// need height of shower max calculated //////

  /// use reconstruction method 1 as default ///

  fData3D->fStartXcore3D = fData->getShowerParameters()->fShowerXcore[0];
  fData3D->fStartYcore3D = fData->getShowerParameters()->fShowerYcore[0];

  ////// test MC ///
  //fData3D->fXcore3D = fData->getShowerParameters()->MCxcore;
  //fData3D->fYcore3D = fData->getShowerParameters()->MCycore;

  //// shower direction (sky to ground coordinates) ////
  //// multiply by base vectors to be in ground system ////
  /// fShower_XoffsetDeRot[0]; // use de-rotated?

  double ts[3]; // temp s vector

  ts[0] = fData->getShowerParameters()->fShower_Xoffset[0];
  ts[1] = fData->getShowerParameters()->fShower_Yoffset[0];
  ts[2] = 0;

  ////// test MC (not working for some reason, check VShowerParameters?) ///
  //ts[0] = fData->getShowerParameters()->MCTel_Xoff;
  //ts[1] = fData->getShowerParameters()->MCTel_Yoff;
  //ts[2] = 0;

  ////// GOOD, but not sure why (swap Y sign) ///////
  ts[1] = 0. - ts[1];

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
  fData3D->fStartSel3D = 90. - ( acos( ts[2] ) *(180./TMath::Pi()) );  // in deg

  /// convert azimuth from -180 to 180 (deg) into 0 to 360 (deg) ///
  //fData3D->fStartSaz3D = 180. + fData3D->fStartSaz3D;

  ////// simple Size to Nc lookup //////
  /// double bigSize = 0; 
  /// for (unsigned int i = 0; i < fData3D->fNTel3D; i++) {
  /// if( fData3D->fsize3D[i] > bigSize ) bigSize = fData3D->fsize3D[i];
  /// }
  /// fData3D->fStartNc3D = ffun->Eval( log10(bigSize) ); // ln(Nc)
  /// cout<<"size, ln(Nc) = "<< bigSize <<" "<< fData3D->fNc3D << endl;
  /// bigSize = 0;
  //////////////////////////////////////
  double TotSize = 0; 
  double TotTel = 0;
  for (unsigned int i = 0; i < fData3D->fNTel3D; i++) {
    if( fData3D->fsize3D[i] > 0 ) {
      TotSize += fData3D->fsize3D[i];
      TotTel += 1.;
    }
  }
  if( TotSize > 0 ) TotSize = (4.0/(float)TotTel)*TotSize; // HARD-WIRED!

  //  fData3D->fStartNc3D = 1.2e3 * TotSize * pow( cos( ZenRad ), 0.46 );
  // cout<<"TotSize: "<< TotSize <<endl;
  fData3D->fStartNc3D = 370 * TotSize * 0.97179; //Nc
  fData3D->fStartNc3D = log( fData3D->fStartNc3D ); //log(Nc)

  //// OLD ///
  //fData3D->fStartNc3D = log( ffun->Eval( TotSize ) ); // ln(Nc)
  //fData3D->fStartNc3D = ffun->Eval( TotSize ); // Nc
  ///cout<< "Old-Nc: "<< ffun->Eval( TotSize ) <<endl;

  //fData3D->fStartNc3D += 0.5e6; // TEST increasing Start 

  //fData3D->fStartNc3D = 1.e6; //fixed Start

  ///fData3D->fsigmaL3D = 3500;
  ///fData3D->fsigmaT3D = 15;

  ////fData3D->fNc3D = 15; /// balls....

  ////// FIX THIS: same starting Nc, sigmaL, sigmaT for all events 
  ///// event 57 //////
  //fData3D->fsigmaL3D = 1000;
  //fData3D->fsigmaT3D = 15;
  //fData3D->fNc3D = 1.e6;

}

////////////////////////////////////

void V1Model3D::getDetector( unsigned int iTel )
{
  /////// Get detector geometry ///////////

  /////// get telescope locations on ground ///////

  fData3D->flocTel3D[0][iTel] = getDetectorGeo()->getTelXpos()[iTel];
  fData3D->flocTel3D[1][iTel] = getDetectorGeo()->getTelYpos()[iTel];
  fData3D->flocTel3D[2][iTel] = getDetectorGeo()->getTelZpos()[iTel];

  ///// zero value in config file /////
  // fMarea3D[iTel] = getDetectorGeo()->getMirrorArea()[iTel];

  fData3D->fMarea3D[iTel] = 111.; // Hard-wired VERITAS mirror area (m^2)

  double pwid = 0.148 * (TMath::Pi()/180.); // Hard-wired VERITAS PMT diameter 

  for( unsigned int j = 0; j < fData3D->fomegapix3D[iTel].size(); j++ ) {
    ///// radius value in config file depends on collection efficiency ///
    //    //pwid = getDetectorGeo()->getTubeRadius( iTel )[j];

    fData3D->fomegapix3D[iTel][j] = TMath::Pi() * (pwid/2.)*(pwid/2.);
  }

//////////////////

}

////////////////////////////////////

void V1Model3D::filterEvents()
{
  /// 10 bins of 0.2 in log10 
  /// starting at 0.2 TeV (-0.7) upto 20 TeV (1.3)

  double energy = fData->getShowerParameters()->MCenergy;

  double binLo[10]; // lower bin log10(E)
  double binUp[10]; // upper bin log10(E)
  double bintemp = -0.7;
  for( int i=0; i<10; i++) {
    binLo[i] = bintemp;
    bintemp += 0.2;
    binUp[i] = bintemp;
  }
  if( log10(energy) < binLo[0] || log10(energy) > binUp[9] ) return;

  return;
}

////////////////////////////////////

bool V1Model3D::checkModel3D()
{

  ///// check that all valid telescopes have the same pointing within a limit /////
  ///// (fails for sub-array or convergent pointing modes) /////

  bool isgood = true;

// pointing for each telescope
  fData3D->fTze3D.resize( fData->getNTel(), 0 );
  fData3D->fTaz3D.resize( fData->getNTel(), 0 );

  for (unsigned int i = 0; i < fData->getNTel(); i++) {
    fData3D->fTze3D[i] = fData->getShowerParameters()->fTelElevation[i];
    fData3D->fTaz3D[i] = fData->getShowerParameters()->fTelAzimuth[i];
  }

  double maxToffset = 0.5; // Hard-wired maximum offset allowed between telescopes (deg)

  unsigned int Npoint = 0; // number of telescopes with non-zero pointing
  double diffTze = 0;  // diff zenith of telescopes (non-zero pointing)
  double diffTaz = 0;  // diff azimuth of telescopes (non-zero pointing)
  double BdiffTze = 0;  // biggest diff zenith of telescopes (non-zero pointing)
  double BdiffTaz = 0;  // biggest diff azimuth of telescopes (non-zero pointing)

  for (unsigned int i = 0; i < fData->getNTel(); i++) {
    for (unsigned int j = 0; j < fData->getNTel(); j++) {

      if( fData3D->fTze3D[i] != 0 && fData3D->fTaz3D[i] != 0 ) {
	Npoint += 1;

	if( fData3D->fTze3D[j] != 0 && fData3D->fTaz3D[j] != 0 ) {
	  diffTze = fabs( fData3D->fTze3D[i] - fData3D->fTze3D[j] );
	  diffTaz = fabs( fData3D->fTaz3D[i] - fData3D->fTaz3D[j] );
	  if( diffTze > BdiffTze ) BdiffTze = diffTze;
	  if( diffTaz > BdiffTaz ) BdiffTaz = diffTaz;
	}
      }
    }
  }

  if( BdiffTze > maxToffset || BdiffTaz > maxToffset ) isgood = false;
  if( Npoint == 0 ) isgood = false;

  return isgood;
}

void V1Model3D::calcPointing()
{

  // elevation and azimuth of telescopes (in ground coordinates)
  //// get mean pointing from valid telescopes ////

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

////////////////////////////////////

void V1Model3D::calcCoord()
{

  //// telescope parameters ////

  // sky unit base vectors in ground coordinate frame (base)

  /////// z //////////////
  //// same as T ////
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

  ////////

  //cout<<"xsg (norm) = ("<<fData3D->fxsg3D[0]<<", "<<fData3D->fxsg3D[1]<<", "<<fData3D->fxsg3D[2]<<")"<<endl;
  //cout<<"ysg (norm) = ("<<fData3D->fysg3D[0]<<", "<<fData3D->fysg3D[1]<<", "<<fData3D->fysg3D[2]<<")"<<endl;
  //cout<<"zsg (norm) = ("<<fData3D->fzsg3D[0]<<", "<<fData3D->fzsg3D[1]<<", "<<fData3D->fzsg3D[2]<<")"<<endl;

  ////// GOOD, but not sure why (swap Y sign) /////
  fData3D->fysg3D[0] = 0. - fData3D->fysg3D[0];
  fData3D->fysg3D[1] = 0. - fData3D->fysg3D[1];
  fData3D->fysg3D[2] = 0. - fData3D->fysg3D[2];


}

