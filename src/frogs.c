/*================================================================*\
|      _    _                                        _    _        |
|     (o)--(o)                                      (o)--(o)       |
|    /.______.\    FFFF RRRR   OOO   GGG   SSS     /.______.\      |
|    \________/    F    R   R O   O G     S        \________/      |
|  ./        \.    FFF  RRRR  O   O G  GG  SSS     ./        \.    |
| ( .        , )   F    R R   O   O G   G     S   ( .        , )   |
|  \ \_\\//_/ /    F    R  RR  OOO   GGG  SSSS     \ \_\\//_/ /    |
|   ~~  ~~  ~~                                      ~~  ~~  ~~     |
| svincent@physics.utah.edu               lebohec@physics.utah.edu |
|                  VERSION 1.02 OCTOBER 10th 2011                  |
|  For license issues, see www.physics.utah.edu/gammaray/FROGS     |
\*================================================================*/

#include "frogs.h"

#define FROGS_TEST 0
//================================================================
//================================================================
int frogs_print_raw_event(struct frogs_imgtmplt_in d) {
  /*This function prints the data stored for the current even in the 
    frogs_imgtmplt_in structure. It is useful when developing a 
    frogs_convert_from_XXXX function*/
#define OUTUNIT stdout
  fprintf(OUTUNIT,"-------------------------------------------------\n");
  fprintf(OUTUNIT,"EVENT %d\n",d.event_id);
  fprintf(OUTUNIT,"Elevation: %f degrees\n",d.elevation);
  fprintf(OUTUNIT,"Number of telescopes: %d\n",d.ntel);
  fprintf(OUTUNIT,"Total number of live pixels: %d\n",d.nb_live_pix_total);
  for(int tel=0;tel<d.ntel;tel++) {
    fprintf(OUTUNIT,"****************************\n");
    fprintf(OUTUNIT,"*****   Telescope %d   *****\n",tel+1);
    fprintf(OUTUNIT,"****************************\n");
    fprintf(OUTUNIT,"Position X=%fm Y=%fm\n",
	    d.scope[tel].xfield,d.scope[tel].yfield);
    fprintf(OUTUNIT,"Number of pixels:%d\n",d.scope[tel].npix);
    fprintf(OUTUNIT,"Number of live pixels:%d\n",d.scope[tel].nb_live_pix);
  }
  fprintf(OUTUNIT,".................................................\n");
  fprintf(OUTUNIT,"            STARTING POINT:\n");
  fprintf(OUTUNIT,"  xs=%f deg. ys=%f deg. \n",d.startpt.xs,d.startpt.ys);
  fprintf(OUTUNIT,"  xp=%f m. yp=%f m. \n",d.startpt.xp,d.startpt.yp);
  fprintf(OUTUNIT,"  Log10(E/1TeV)=%f \n",d.startpt.log10e);
  fprintf(OUTUNIT,"  Lambda=%f Xo\n",d.startpt.lambda);
  
  fprintf(OUTUNIT,"-------------------------------------------------\n");
  if(d.worthy_event==FROGS_OK)   fprintf(OUTUNIT,"  GOOD EVENT\n");
  else   fprintf(OUTUNIT,"  BAD EVENT\n");

  fprintf(OUTUNIT,"-------------------------------------------------\n");
  return FROGS_OK;
}
//================================================================
//================================================================
struct frogs_imgtmplt_out frogs_img_tmplt(struct frogs_imgtmplt_in *d) {
  /* This function performs the image template analysis. It returns a 
     structure of type frogs_imgtmplt_out containing all the useful 
     information  regarding the convergence of the likelihood 
     optimization. All the data from the telescopes for the event 
     being  analyzed are in the structure frogs_imgtmplt_in d 
     received as an argument.*/
  
  struct frogs_imgtmplt_out rtn;
  /* Event selection used to avoid processing bad or poor events 
    worthy_event is calculated in frogs_convert_from_grisu (or 
    equivalent for other analysis packages)*/
  if(d->worthy_event==FROGS_NOTOK) {
    rtn=frogs_null_imgtmplt_out();
    rtn.event_id=d->event_id;
    rtn.nb_iter=FROGS_BAD_NUMBER;
    rtn.gsl_convergence_status=FROGS_BAD_NUMBER;
    //Release memory used in the data structure
    frogs_release_memory(d);
    return rtn;
  }

  static struct calibration_file calib;
  static struct frogs_probability_array prob_array;

  /*This check if a template image needs to be read and reads it if necessary*/
  static struct frogs_imgtemplate tmplt;
  static int firstcall=1;
  //On the first call set the template elevation to zero
  if(firstcall) {
    tmplt.elevmin=0;tmplt.elevmax=0;firstcall=0;
    fill_prob_density( &prob_array ); 
  }
  //If needed read the template file according to elevation
  if(d->elevation>tmplt.elevmax || d->elevation<tmplt.elevmin) {
    tmplt=frogs_read_template_elev(d->elevation);
  }


  //Optimize the likelihood
  rtn=frogs_likelihood_optimization(d,&tmplt,&calib,&prob_array);

  //Release memory used in the data structure
  frogs_release_memory(d);

  return rtn;
}
//================================================================
//================================================================
struct frogs_imgtmplt_out frogs_img_tmplt_old(struct frogs_imgtmplt_in *d) {
  /* This function performs the image template analysis. It returns a 
     structure of type frogs_imgtmplt_out containing all the useful 
     information  regarding the convergence of the likelihood 
     optimization. All the data from the telescopes for the event 
     being  analyzed are in the structure frogs_imgtmplt_in d 
     received as an argument.*/

  struct frogs_imgtmplt_out rtn;
  /* Event selection used to avoid processing bad or poor events 
    worthy_event is calculated in frogs_convert_from_grisu (or 
    equivalent for other analysis packages)*/
  if(d->worthy_event==FROGS_NOTOK) {
    rtn=frogs_null_imgtmplt_out();
    rtn.event_id=d->event_id;
    rtn.nb_iter=FROGS_BAD_NUMBER;
    rtn.gsl_convergence_status=FROGS_BAD_NUMBER;
    //Release memory used in the data structure
    frogs_release_memory(d);
    return rtn;
  }

  /*This check if a template image needs to be read and reads it if necessary*/
  static struct frogs_imgtemplate tmplt;
  static int firstcall=1;
  //On the first call set the template elevation to zero
  if(firstcall) {tmplt.elevation=0;firstcall=0;}
  //Read the template file according to elevation
  if(fabs(d->elevation-tmplt.elevation)>5.0) {
    if(tmplt.elevation!=70 || d->elevation<65) {
      /*This weird logic is due to the fact that as of now, everything 
        above 65 degree elevation is analysed with templates at 70 deg. 
        elevation. Change it if you need. */
      tmplt=frogs_read_template_elev(d->elevation);
    }
  }

  //Optimize the likelihood
//  rtn=frogs_likelihood_optimization(d,&tmplt);

  //Release memory used in the data structure
  frogs_release_memory(d);

  return rtn;
}
//================================================================
//================================================================
int frogs_release_memory(struct frogs_imgtmplt_in *d) {
  /*This function is used to release the memory taken up by each event. */
  for(int tel=0;tel<d->ntel;tel++) {
    delete d->scope[tel].xcam;
    delete d->scope[tel].ycam;
    delete d->scope[tel].q;
    delete d->scope[tel].ped;
    delete d->scope[tel].exnoise;
    delete d->scope[tel].pixinuse;
    delete d->scope[tel].telpixarea;
  }
  delete d->scope;
  return 0; 
}
//================================================================
//================================================================
struct frogs_imgtmplt_out frogs_null_imgtmplt_out() {
  //Used to set the image template analysis output to some null values 
  //when the analysis failed. 
  struct frogs_imgtmplt_out rtn;
  rtn.nb_iter=FROGS_BAD_NUMBER;
  rtn.goodness_img=FROGS_BAD_NUMBER;
  rtn.npix_img=FROGS_BAD_NUMBER;
  rtn.goodness_bkg=FROGS_BAD_NUMBER;
  rtn.npix_bkg=FROGS_BAD_NUMBER;
  rtn.cvrgpt.xs=FROGS_BAD_NUMBER;
  rtn.cvrgpt.ys=FROGS_BAD_NUMBER;
  rtn.cvrgpt.xp=FROGS_BAD_NUMBER;
  rtn.cvrgpt.yp=FROGS_BAD_NUMBER;
  rtn.cvrgpt.log10e=FROGS_BAD_NUMBER;
  rtn.cvrgpt.lambda=FROGS_BAD_NUMBER;
  rtn.cvrgpterr.xs=FROGS_BAD_NUMBER;
  rtn.cvrgpterr.ys=FROGS_BAD_NUMBER;
  rtn.cvrgpterr.xp=FROGS_BAD_NUMBER;
  rtn.cvrgpterr.yp=FROGS_BAD_NUMBER;
  rtn.cvrgpterr.log10e=FROGS_BAD_NUMBER;
  rtn.cvrgpterr.lambda=FROGS_BAD_NUMBER;
  rtn.tel_goodnessImg[0] = FROGS_BAD_NUMBER;
  rtn.tel_goodnessImg[1] = FROGS_BAD_NUMBER;
  rtn.tel_goodnessImg[2] = FROGS_BAD_NUMBER;
  rtn.tel_goodnessImg[3] = FROGS_BAD_NUMBER;
  rtn.tel_goodnessBkg[0] = FROGS_BAD_NUMBER;
  rtn.tel_goodnessBkg[1] = FROGS_BAD_NUMBER;
  rtn.tel_goodnessBkg[2] = FROGS_BAD_NUMBER;
  rtn.tel_goodnessBkg[3] = FROGS_BAD_NUMBER;
  return rtn;
}
//================================================================
//================================================================
struct frogs_imgtmplt_out
frogs_likelihood_optimization(struct frogs_imgtmplt_in *d, 
			      struct frogs_imgtemplate *tmplt,
			      struct calibration_file *calib, 
			      struct frogs_probability_array *prob_array) {
  /* This function optimizes the likelihood function on the event parameter 
     space. It returns all the relevant information regarding the convergence 
     in a structure frogs_imgtmplt_out. d is a pointer to a structure 
     containing all the data from the telescopes. tmplt is a pointer to a 
     structure containing the image template data*/
  struct frogs_imgtmplt_out rtn;
 
  //Gets the starting point in a GSL vector. 
  const size_t p = tmplt->ndim+1;    //Dimension of the parameter space
  gsl_vector *x;
  x = gsl_vector_alloc (p);
  gsl_vector_set (x, FROGS_XS, d->startpt.xs);
  gsl_vector_set (x, FROGS_YS, d->startpt.ys);
  gsl_vector_set (x, FROGS_XP, d->startpt.xp);
  gsl_vector_set (x, FROGS_YP, d->startpt.yp);
  gsl_vector_set (x, FROGS_LOG10E, d->startpt.log10e);
  gsl_vector_set (x, FROGS_LAMBDA, d->startpt.lambda);

  //Prepare function minimization 
  const size_t n = d->nb_live_pix_total;  //Number of pixels used
  gsl_multifit_function_fdf func;
  func.n = n;  //number of pixels used
  func.p = p; ////Dimension of the parameter space
  func.f = &frogs_likelihood; //frogs_likelihood: function to be minimized
  func.df = &frogs_likelihood_derivative; //frogs_likelihood_derivative
  func.fdf = &frogs_likelihood_fdf;  //Werid function GSL wants to see
  //Construct a structure with all the data to be passed to the image model
  struct  frogs_gsl_data_wrapper data;
  data.tmplt=tmplt;  //template data
  data.data=d;       //telescope data
  data.probarray = prob_array;
  func.params = (void *)&data; /*This will be passed to the functions 
  				frogs_likelihood and 
  				frogs_likelihood_derivative 
  				as a pointer to a void*/
  const gsl_multifit_fdfsolver_type *T;
  gsl_multifit_fdfsolver *s;
  T = gsl_multifit_fdfsolver_lmsder;
  s = gsl_multifit_fdfsolver_alloc(T, n, p);
  gsl_multifit_fdfsolver_set(s, &func, x);
    
 //Loop on iterations
  int status=0;
  unsigned int iter = 0;
  do {
    iter++;
    status = gsl_multifit_fdfsolver_iterate(s);
    if (status) break;
    status = gsl_multifit_test_delta (s->dx, s->x, 1e-7, 1e-7);
  } while (status == GSL_CONTINUE && iter < FROGS_MAX_ITER_NBR);
  
  //If the maximum number of iteration was reached, we return a null result.  
  if(iter == FROGS_MAX_ITER_NBR) {  
    rtn=frogs_null_imgtmplt_out();
    rtn.gsl_convergence_status=status;
    rtn.event_id=d->event_id;
    rtn.nb_iter=iter;
    gsl_multifit_fdfsolver_free(s);
    return rtn;
  } 
  rtn.gsl_convergence_status=status;
  rtn.event_id=d->event_id;
  rtn.nb_iter=iter;
  //Gathers the parameter space convergence point coordinates. 
  rtn.cvrgpt.xs=gsl_vector_get (s->x, FROGS_XS);
  rtn.cvrgpt.ys=gsl_vector_get (s->x, FROGS_YS);
  rtn.cvrgpt.xp=gsl_vector_get (s->x, FROGS_XP);
  rtn.cvrgpt.yp=gsl_vector_get (s->x, FROGS_YP);
  rtn.cvrgpt.log10e=gsl_vector_get (s->x, FROGS_LOG10E);
  rtn.cvrgpt.lambda=gsl_vector_get (s->x, FROGS_LAMBDA);

  /*GSL take lambda to any value but only the value used in the model 
    calculation is directly meaningfull. Here, we convert the value set 
    by GSL to the actual value used in the model calculation. */
  float maxlambda=tmplt->min[0]+(tmplt->nstep[0]-1)*tmplt->step[0];
  rtn.cvrgpt.lambda=floatwrap(rtn.cvrgpt.lambda,tmplt->min[0],maxlambda);

  gsl_matrix *covar = gsl_matrix_alloc (p, p);
  gsl_multifit_covar(s->J, 0.0, covar);
  
  
  //Get the error for each parameter
  float chi = gsl_blas_dnrm2(s->f);
  float dof = n - p;
  float c = GSL_MAX_DBL(1, chi/sqrt(dof));
#define ERR(i) sqrt(gsl_matrix_get(covar, i, i))
  //ERR(i) is nan when iter==1. We then change the error to FROGS_BAD_NUMBER
  rtn.cvrgpterr.xs = c*ERR(FROGS_XS); 
  if(!frogs_is_a_good_number(rtn.cvrgpterr.xs)) 
    rtn.cvrgpterr.xs=FROGS_BAD_NUMBER;
  rtn.cvrgpterr.ys = c*ERR(FROGS_YS);
  if(!frogs_is_a_good_number(rtn.cvrgpterr.ys)) 
    rtn.cvrgpterr.ys=FROGS_BAD_NUMBER;
  rtn.cvrgpterr.xp = c*ERR(FROGS_XP); 
  if(!frogs_is_a_good_number(rtn.cvrgpterr.xp)) 
    rtn.cvrgpterr.xp=FROGS_BAD_NUMBER;
  rtn.cvrgpterr.yp = c*ERR(FROGS_YP); 
  if(!frogs_is_a_good_number(rtn.cvrgpterr.yp)) 
    rtn.cvrgpterr.yp=FROGS_BAD_NUMBER;
  rtn.cvrgpterr.log10e = c*ERR(FROGS_LOG10E); 
  if(!frogs_is_a_good_number(rtn.cvrgpterr.log10e)) 
    rtn.cvrgpterr.log10e=FROGS_BAD_NUMBER;  
  rtn.cvrgpterr.lambda = c*ERR(FROGS_LAMBDA); 
  if(!frogs_is_a_good_number(rtn.cvrgpterr.lambda)) 
    rtn.cvrgpterr.lambda=FROGS_BAD_NUMBER;

  /*Calculate the image and background goodness for the convergence point.*/ 
  if(frogs_goodness(&rtn,d,tmplt,calib,prob_array)!=FROGS_OK) 
    frogs_showxerror("Problem encountered in the convergence goodness calculation");
  gsl_multifit_fdfsolver_free(s);
  gsl_matrix_free(covar);

  return rtn;
}
//================================================================
//================================================================
int frogs_goodness(struct frogs_imgtmplt_out *tmplanlz,
		   struct frogs_imgtmplt_in *d, 
		   struct frogs_imgtemplate *tmplt,
		   struct calibration_file *calib,
		   struct frogs_probability_array *prob_array) {
  /* Calculates the image and background goodness whose values and associated 
     number of pixels are passed back in tmplanlz which also contains the 
     values of the event physical parameters for which the goodness is 
     calculated. d is a pointer to a structure holding all the data from the 
     telescopes while tmplt is a pointer to a structure holding the image 
     template data */
 
  int telnpix[4] = {0}; // pixel counter for single camera 
 
  /*Initialize goodnesses and the number of pixels for both image 
    and background regions*/
  tmplanlz->goodness_img=0;
  tmplanlz->npix_img=0;
  tmplanlz->goodness_bkg=0;
  tmplanlz->npix_bkg=0;

// GH: Goodness of fit per telescope for image and background
  tmplanlz->tel_goodnessImg[0] = 0.;
  tmplanlz->tel_goodnessImg[1] = 0.;
  tmplanlz->tel_goodnessImg[2] = 0.;
  tmplanlz->tel_goodnessImg[3] = 0.;
  tmplanlz->tel_goodnessBkg[0] = 0.;
  tmplanlz->tel_goodnessBkg[1] = 0.;
  tmplanlz->tel_goodnessBkg[2] = 0.;
  tmplanlz->tel_goodnessBkg[3] = 0.;

  for(int tel=0;tel<d->ntel;tel++) {
    telnpix[tel] = 0;
    for(int pix=0;pix<d->scope[tel].npix;pix++) {
      if(d->scope[tel].pixinuse[pix]==FROGS_OK) {
	//Here call image model and calculate expected signal
	int pix_in_template;//FROGS_OK in image, FROGS_NOTOK in background
	double mu=frogs_img_model(pix,tel,tmplanlz->cvrgpt,d,
				  tmplt,&pix_in_template);

// GH: eventdisplay OUTPUT to see templates in the display gui
	tmplanlz->tmplt_tubes[tel][pix] = mu;

	if(mu!=FROGS_BAD_NUMBER) { 
	  double pd;

// GH: probabilityArray: lookup table to get probability densities
// Saves factor of 2x in computing
	  if( mu > 1.e-18 && mu < FROGS_LARGE_PE_SIGNAL )
            pd=probabilityArray(prob_array,d->scope[tel].q[pix],mu,d->scope[tel].ped[pix]);
	  else
	    pd=frogs_probability_density(d->scope[tel].q[pix],mu,
                                               d->scope[tel].ped[pix],
                                               d->scope[tel].exnoise[pix]);

	  double mean_lkhd=frogs_mean_pix_lkhd(d->scope[tel].q[pix],mu,
					       d->scope[tel].ped[pix],
					       d->scope[tel].exnoise[pix],
					       prob_array);

	  double pix_goodness=-2.0*log(pd)-mean_lkhd;

// SV: Not sure this is still useful needs to be tested
	  if( pd <= 10.0*FROGS_SMALL_NUMBER )
	  {
	    if( mu < 1.0e-18 )
	    {
		pix_goodness  = d->scope[tel].q[pix]*d->scope[tel].q[pix];
		pix_goodness /= d->scope[tel].ped[pix]*d->scope[tel].ped[pix];
		pix_goodness -= 1.0;
            } else
	    {
		pix_goodness  = d->scope[tel].q[pix] - mu;
		pix_goodness *= d->scope[tel].q[pix] - mu;
		pix_goodness /= d->scope[tel].ped[pix]*d->scope[tel].ped[pix] + mu*(1.0 + d->scope[tel].exnoise[pix]*d->scope[tel].exnoise[pix]);
		pix_goodness -= 1.0;
	    }
	  }

	  //If requested we produce a calibration output
	  if(FROGS_NBEVENT_GDNS_CALIBR>0) 
	    frogs_gdns_calibr_out(d->event_id, tel, pix, d->scope[tel].q[pix],
				  d->scope[tel].ped[pix],mu,pix_goodness,tmplanlz->cvrgpt.log10e,tmplanlz->cvrgpt.xp,tmplanlz->cvrgpt.yp);

	  /*Apply the single pixel goodness correction according to the 
	    pixel pedestal width and the model value mu*/

	  /*Decides if the pixel should be counted in the image 
	    or background region*/

	  int pix_in_img=frogs_image_or_background(tel,pix,d);

	  //If requested, we produce a display of the event
	  if(FROGS_NBEVENT_DISPLAY>0) 
	    frogs_event_display(d->event_id, d->scope[tel].q[pix],mu,
				d->scope[tel].xfield,d->scope[tel].yfield,
				d->scope[tel].xcam[pix],
				d->scope[tel].ycam[pix],pix_in_img);	  

	  //If the pixel is in the image region
	  if(pix_in_img==FROGS_OK) {
	    tmplanlz->goodness_img=tmplanlz->goodness_img+pix_goodness;
	    tmplanlz->npix_img++;
	    tmplanlz->tel_goodnessImg[tel] += pix_goodness;
	    telnpix[tel]++;
 	  }
	  //If the pixel is in the background region
	  if(pix_in_img==FROGS_NOTOK) {
	    tmplanlz->goodness_bkg=tmplanlz->goodness_bkg+pix_goodness;
	    tmplanlz->npix_bkg++;
    	    tmplanlz->tel_goodnessBkg[tel] += pix_goodness;
	  }
	}//End of background/image region test
      } //End of test on pixel viability


    }//End of pixel loop

// SV: Image and background goodness computed for each telescope
// Sum over all telescope should equal the total Goodness.
    if(d->nb_live_pix_total>tmplt->ndim+1) 
      tmplanlz->tel_goodnessImg[tel] /= sqrt(2.0*(d->nb_live_pix_total-(tmplt->ndim+1)));
    else
      tmplanlz->tel_goodnessImg[tel] = FROGS_BAD_NUMBER;
    if(d->nb_live_pix_total>tmplt->ndim+1) 
      tmplanlz->tel_goodnessBkg[tel] /= sqrt(2.0*(d->nb_live_pix_total-(tmplt->ndim+1)));
    else
      tmplanlz->tel_goodnessBkg[tel] = FROGS_BAD_NUMBER;

 }//End of telescope loop

  for( int itel=0; itel<d->ntel; itel++ )
  {
    if( telnpix[itel] == 0 || fabs(tmplanlz->tel_goodnessImg[itel]) < 1.e-7 )  tmplanlz->tel_goodnessImg[itel] = FROGS_BAD_NUMBER;
    if( telnpix[itel] == 0 || fabs(tmplanlz->tel_goodnessBkg[itel]) < 1.e-7 )  tmplanlz->tel_goodnessBkg[itel] = FROGS_BAD_NUMBER;
  }

  //Finilize the background goodness calculation (*** See note)
  if(d->nb_live_pix_total>tmplt->ndim+1) 
    tmplanlz->goodness_bkg=tmplanlz->goodness_bkg/
      //sqrt(2.0*(tmplanlz->npix_bkg-(tmplt->ndim+1))); 
      sqrt(2.0*(d->nb_live_pix_total-(tmplt->ndim+1))); 
  else
    tmplanlz->goodness_bkg=FROGS_BAD_NUMBER;
  //Finilize the image goodness calculation (*** See note)
  if(d->nb_live_pix_total>tmplt->ndim+1) 
    tmplanlz->goodness_img=tmplanlz->goodness_img/
      //sqrt(2.0*(tmplanlz->npix_img-(tmplt->ndim+1)));
      sqrt(2.0*(d->nb_live_pix_total-(tmplt->ndim+1)));
  else
    tmplanlz->goodness_img=FROGS_BAD_NUMBER;

  /* ***note on the goodness. The goodness is the difference between the 
     log-likelihood and its average value divided by the square root of two 
     times the number of degrees of freedom. Here we calculate a goodness for 
     the image and for the background and we subtract the number of optimized 
     parameter to both. This seems odd. It probably does not matter much when 
     the number of pixels in the image and in the background are large 
     compared to the number of optimized parameters.*/
    
  //printf("Energy %d %f\n",d->event_id,tmplanlz->cvrgpt.log10e);

  return FROGS_OK;
}
//================================================================
//================================================================
int frogs_image_or_background(int tel,int pix,struct frogs_imgtmplt_in *d){
  /*Returns FROGS_OK if the pixel is identified to be part of the image 
    region and returns FROGS_NOTOK is the pixel is to be counted in the 
    bacground region. 
    The image is defined by the pixels surviving a two threshold standard 
    cleaning algorithm. If a pixel is less than FROGS_PICTRAD from a pixel 
    passing the cleaning, it is in the image region. Otherwise it is 
    in the bacground region. 
    If a pixel signal exceeds FROGS_HITHRESH passes the cleaning.
    If a pixel signal exceeds FROGS_LOTHRESH it passes the cleaning if 
    it has a direct neighbor whose signal exceeds FROGS_HITHRESH. The 
    thesholds FROGS_HITHRESH and FROGS_LOTHRESH are expressed in units 
    of pedestal standard deviations. 
  */

  //If the pixel tested is not a pixel in use, returns FROGS_BAD_NUMBER
  if(d->scope[tel].pixinuse[pix]==FROGS_NOTOK) return FROGS_BAD_NUMBER;

  /*If the signal in the pixel is above the higher threshold, 
  the pixel is part of the image*/
  if(d->scope[tel].q[pix]>FROGS_HITHRESH*d->scope[tel].ped[pix]) 
    return FROGS_OK;
  
  //Get the square of the radius defining the picture region
  float d2max=FROGS_PICTRAD*FROGS_PICTRAD;

  //Loop over the pixels
  for(int p=0;p<d->scope[tel].npix;p++) {
    //Only for pixels in use
    if(d->scope[tel].pixinuse[p]==FROGS_OK) {
      //Distance to the tested pixel
      float d2=(d->scope[tel].xcam[pix]-d->scope[tel].xcam[p])*
	(d->scope[tel].xcam[pix]-d->scope[tel].xcam[p])+
	(d->scope[tel].ycam[pix]-d->scope[tel].ycam[p])*
	(d->scope[tel].ycam[pix]-d->scope[tel].ycam[p]);
      //If the distance is less than the picture definition radius
      if(d2<d2max) {
	/*If the pixel has a signal exceeding the higher threshold, the 
	  tested pixel is counted in the image. */
	if(d->scope[tel].q[p]>FROGS_HITHRESH*d->scope[tel].ped[p]) 
	  return FROGS_OK;
	/*If the pixel has a signal exceeding the lower threshold but not 
	  the higher, it needs to have a direct neighbor exceeding the 
	  higher threshold for the tested pixel to be part of the picture 
	  region*/
	if(d->scope[tel].q[p]>FROGS_LOTHRESH*d->scope[tel].ped[p]) {
	  //Square of the maximal distance for pixels to be neighbors
	  float dnb2max=FROGS_NEIGHBORAD*FROGS_NEIGHBORAD;
	  //Loop over the pixels
	  for(int pnb=0;pnb<d->scope[tel].npix;pnb++) {
	    //Only for pixels in use
	    if(d->scope[tel].pixinuse[pnb]==FROGS_OK) {
	      float dnb2=(d->scope[tel].xcam[p]-d->scope[tel].xcam[pnb])*
		(d->scope[tel].xcam[p]-d->scope[tel].xcam[pnb])+
		(d->scope[tel].ycam[p]-d->scope[tel].ycam[pnb])*
		(d->scope[tel].ycam[p]-d->scope[tel].ycam[pnb]);	    
	      //if pixels p and pnb are neighbors
	      if(dnb2<dnb2max)
		/*if the neighbor of p has a signal in excess of the high 
		  threshold the tested pixel is counted in the picture. */
		if(d->scope[tel].q[pnb]>FROGS_HITHRESH*d->scope[tel].ped[pnb])
		  return FROGS_OK;
	    } //end of the pixinuse test in the search for neighbors of pixel p
	  } //End of the loop searching for neighbors of pixel p
	} //End of the test for LOTHRESH
      }//End of test on the distance
    }//End of the test of pixel being in use
  }//End of the main loop on pixels
  return FROGS_NOTOK;
}
//================================================================
//================================================================
int frogs_gdns_calibr_out(int event_id, int tel, int pix, float q,
			  float ped, float mu,double pix_goodness,double energy,double xp,double yp){
  /*This funtion is used to print out information used to establish the 
    calibration correction to be applied to individual pixel goodness 
    values. On the first time it is called, it opens a file named 
    frogs_goodness_calibration.frogs and starts counting events, printing 
    calibration data for each pixel. When the number of events exceeds 
    FROGS_NBEVENT_GDNS_CALIBR it closes the file and stops the program 
    This function should be called from the function frogs_goodness*/

  static int last_event_id=FROGS_NOTOK; //Stores the last event id. 
  static int first_time=FROGS_OK;       //Check for first time call
  static int nbrevt_calib=0;       //Counter of events used for calibration
  static FILE *calib;              //File pointer
  //On the first call to this function
  if(first_time==FROGS_OK) {
    first_time=FROGS_NOTOK;
    //Open the file
    //calib=fopen("frogs_goodness_calibration.frogs","w");
    calib=fopen("frogs_goodness_calibration_dummy.frogs","w");
    if(calib==NULL) 
      frogs_showxerror("Failed opening the file frogs_goodness_calibration.frogs for writing");
  }

  //If the event id is different from the one at the last call of this function
  if(event_id!=last_event_id) {
    //Update the last event id for next time
    last_event_id=event_id;
    //Count one more event used for the calibration
    nbrevt_calib++;
  }

  //If we collected enough events we ned to stop
  if(nbrevt_calib>FROGS_NBEVENT_GDNS_CALIBR) {
    //Close the file
    fclose(calib);
    //Stop the execussion
    frogs_showxerror("Done writing the calibration file data in frogs_goodness_calibration.frogs");

    return FROGS_OK;
  }

  //If we get here it means we have an open file and we need to print the data
  fprintf(calib,"%d %d %d %f %f %f %g %f %f %f\n",event_id,tel,pix,ped,q,mu,
	  pix_goodness,energy,xp,yp);
  return FROGS_OK;
}
//================================================================
//================================================================
int frogs_event_display(int event_id, float q,float mu,float xtel,
			float ytel,float xpix,float ypix,int pix_in_img) {
  /* This funtion is used to print out a kumac script to be executed from 
     a PAWX11 session to obtain a display of the events and their template 
     fit. On the first time it is called, it opens a file named 
     frogs_display.kumac and starts counting events, printing drawing 
     script commands for each pixel. When the number of events exceeds 
     FROGS_NBEVENT_DISPLAY it closes the file and stops the program
     This function should be called from the function frogs_goodness*/
  
  static int last_event_id=FROGS_NOTOK; //Stores the last event id. 
  static int first_time=FROGS_OK;  //Check for first time call
  static int nbrevt_display=0;       //Counter of events used for calibration
  static FILE *display;
  
  //On the first call to this function
  if(first_time==FROGS_OK) {
    first_time=FROGS_NOTOK;
    //Open the file
    display=fopen("frogs_display.kumac","w");
    if(display==NULL) 
      frogs_showxerror("Failed opening the file frogs_goodness_calibration.frogs for writing");
  }

  //If we collected enough events we need to stop
  if(nbrevt_display>FROGS_NBEVENT_DISPLAY) {
    //Close the file
    fclose(display);
    //Stop the execussion
    frogs_showxerror("Done writing the event display file  frogs_display.kumac");
    return FROGS_OK;
  }

  //If the event id is different from the one at the last call of this function
  if(event_id!=last_event_id) {
    if(last_event_id==FROGS_NOTOK) fprintf(display,"opt ntic\n");
    else fprintf(display,"wait\n");
    //Update the last event id for next time
    last_event_id=event_id;
    //Count one more event used for the display
    nbrevt_display++;
    fprintf(display,"HISTOGRAM/CREATE/TITLE_GLOBAL 'Event %d'\n",event_id);
    fprintf(display,"nul -150 150 -150 150 ab\n");
  }

  float scale=15.0; //Angular scale meters/degrees
  float thresh=6;  //Smallest number of photons to be displayed
  float maxlight=50; //Maximal signal
  float maxrad=0.075; //Maximal pixel radius in degrees
  //If we get here it means we have an open file and we need to draw a pixel
  if(q>thresh || mu>thresh){
    float radius; 
    radius=(q-thresh)/maxlight; if(radius>1) radius=1;
    if (radius>0) {
      radius=maxrad*sqrt(radius);
      fprintf(display,"set plci 2\n");
      fprintf(display,"arc %f %f %f\n",xtel+xpix*scale,ytel+ypix*scale,
	      radius*scale);
    }
    radius=(mu-thresh)/maxlight; if(radius>1) radius=1;
    if (radius>0) {
      radius=maxrad*sqrt(radius);
      fprintf(display,"set plci 3\n");
      fprintf(display,"arc %f %f %f\n",xtel+xpix*scale,ytel+ypix*scale,
	      radius*scale);
    }
  }
  if(pix_in_img==FROGS_OK) {
      fprintf(display,"set plci 1\n");
      fprintf(display,"arc %f %f %f\n",xtel+xpix*scale,ytel+ypix*scale,
	      maxrad*scale);
  } else {
      fprintf(display,"set plci 1\n");
      fprintf(display,"arc %f %f %f\n",xtel+xpix*scale,ytel+ypix*scale,
	      maxrad*scale*0.1);
    
  }
  return FROGS_OK;
}
//================================================================
//================================================================
int frogs_is_a_good_number(double x) {
  //Returns 0 is x is a NaN of a Inf and returns 1 otherwise. 
  if(isnan(x)) return 0;
  if(isinf(x)) return 0;
  return 1;
}
//================================================================
//================================================================
float frogs_goodness_correction(float goodness0,float ped,float mu) {
  /* Applies correction to the individual pixel goodness in order to
     compensate for its sensitivity to the pedestal width. The function
     returns the single pixel corrected goodness value
     goodness0 = uncorrected goodness
     ped = pedestal width */
  
  /***** Tucson's correction ********/
  /*if(ped<=1.0) return goodness0+0.6046;
  if(ped>1.0 && ped<=1.5) return goodness0+0.3748;
  if(ped>1.5 && ped<=2.0) return goodness0+0.3860;
  if(ped>2.0 && ped<=2.5) return goodness0+0.4126;
  if(ped>2.5 && ped<=3.0) return goodness0+0.4397;
  return goodness0+0.5502;*/
  /***** Tucson's correction ********/

  /***** simulation correction ******/
/*
  float correction=0.;
  if(mu>0.0) {
    goodness0 = goodness0 + 1; //goodness=goodness+1
    if(mu<=40.0)
      correction = 0.1065*mu+1.0;
    if(mu>40.0 && mu<=130)
      correction = 0.0894*mu+0.998;
    if(mu>130.0 && mu<=200)
      correction = 0.1481*mu-9.563;
    if(mu>200)
      correction = 0.1286*mu-8.781;
    goodness0 = goodness0/correction - 1;
  }
*/
  /***** simulation correction ******/
  /******* very new correction 06.12.2011 **************/
  if( mu > 0.0 ) {
    float correction=pow((mu+1.0),-0.48643);
    return (goodness0 + 1.0)*correction - 1.0;
  }
  /******* new correction **************/

  return goodness0;

}
//================================================================
//================================================================
double frogs_probability_density(float q, double mu, float ped, 
				 float exnoise) {
  double rtn = 0.0;
  /* This function returns the probability density of obtaining a signal q 
     when the average of the expected signal is mu
     q = actual signal in the pixel
     mu = expectation value of the signal in the pixel
     ped = pedestal width for that pixel 
     exnoise = exess noise for that pixel */
  
  //Case mu=0
  if( mu < 1.0e-18 ) rtn=exp(-q*q/(2.0*ped*ped))/(FROGS_SQRTTWOPI*ped);
  
  //Case where mu is large enough the poisson distribution is gauss-like
  if(mu>=FROGS_LARGE_PE_SIGNAL) {
    double dummy1=ped*ped+mu*(1.0+exnoise*exnoise);//Variance
    double dummy2=exp(-(q-mu)*(q-mu)*0.5/dummy1);  //Gauss probability density
    rtn=dummy2/sqrt(FROGS_TWOPI*dummy1);           //Normalization
  }

  //Detailed calculation for intermediate values of mu
  if(mu>=1.0e-18 && mu<FROGS_LARGE_PE_SIGNAL) {
    float stdev=sqrt(mu*(1+exnoise*exnoise)+ped*ped);
    int Nmin=(int)(floor(mu-FROGS_NUMBER_OF_SIGMA*stdev));
    if(Nmin<0) Nmin=0;
    int Nmax=(int)(floor(mu+FROGS_NUMBER_OF_SIGMA*stdev));
    if(Nmax<Nmin ) Nmax=Nmin; //Although it does not seem possible
    //Sum over the possible number of photons contributing to the signal
    for(int n=Nmin; n<=Nmax; n++) {
      //Poisson factor
      double dummy1=frogs_poisson_distribution(mu,n);
      double dummy2=ped*ped+n*exnoise*exnoise; //standard deviation
      dummy1=dummy1/sqrt(FROGS_TWOPI*dummy2);//Gauss distrib. normaliz. factor
      rtn=rtn+dummy1*exp(-(q-n)*(q-n)/(2.0*dummy2));/*Gauss distribution with 
						      Poisson probability 
						      weight*/
    }
  }

  /* If the probability density numerically drops to zero, the penalty 
     payed for that pixel diverges so we cap it so as to retain sensitivity 
     to parameter changes from other pixels. */
  if(rtn<FROGS_SMALL_NUMBER) rtn=FROGS_SMALL_NUMBER; 
  return rtn;
}
//================================================================
//================================================================
double frogs_poisson_distribution(double mu, long int n) {
  //Compute the Poisson probability to get n for an average mu
  double rtn;
  if(mu>0.0) {
    /*P(mu,n)=mu^n*exp(-mu)/n! but we go logarithmic to avoid 
      numerical problems.*/
    rtn=n*log(mu)-mu-frogs_logarithm_factorial(n);
    rtn=exp(rtn);
  }
  else {//that is if mu==0
    if(n==0) rtn = 1.0;
    else rtn = 0.0;
  }
  return rtn;
}
//================================================================
//================================================================
double frogs_logarithm_factorial(long int n) {
  //Compute the log of n!
  
  //Returns 0 when n<0
  if(n<0) return 0;
  
  //Returns tabulated value for n<11 
  if(n<11) {
    int a[11]={1,1,2,6,24,120,720,5040,40320,362880,3628800};
    return (double) log(a[n]);
  }
  else {
    //What follows is Srinivasa Ramanujan approximation of n!
    //For n=10 it returns log(3628797.9)
    //For n=11 it returns log(39916782) instead of log(3991680)
    return n*log(n)-n+log(n*(1.0+4.0*n*(1.0+2.0*n)))/6.0+0.57236494;
    //0.57236494 is 0.5*log(pi)
  }
}
//================================================================
//================================================================
//double frogs_mean_pix_lkhd(float q,double mu,float ped,float exnoise) {
double frogs_mean_pix_lkhd(double q,double mu, double ped,double exnoise, struct frogs_probability_array *prob_array) {
  double rtn=0;
  /* Computes and returns the average of the log-likelihood for the 
     considered pixel for which:  
     q = actual signal in the pixel
     mu = expectation value of the signal in the pixel
     ped = pedestal width for that pixel 
     exnoise = exess noise for that pixel */
  
  //Case mu=0
  if(mu < 1.0e-18) rtn=1.0+FROGS_LNTWOPI+2.0*log(ped);
  
  //Case where mu is large enough the poisson distribution is gauss-like
  if(mu>=FROGS_LARGE_PE_SIGNAL) {
    double dummy=ped*ped+ mu*(1.0+exnoise*exnoise);
    rtn=1.0+FROGS_LNTWOPI+log(dummy);
  }
  
  //Detailed calculation for intermediate values of mu
  if(mu>=1.0e-18 && mu<FROGS_LARGE_PE_SIGNAL) {
    //We use GSL for the integration calculating the average
    gsl_integration_workspace * w = gsl_integration_workspace_alloc(3000);
    double error;
    gsl_function F;
    F.function = &frogs_integrand_for_averaging; //Integrand function
    struct frogs_gsl_func_param par; //Parameters to be passed to the function
    par.ped = ped;
    par.exnoise = exnoise;
    par.mu = mu;
    par.probarray = prob_array;

    F.params = (void *)&par;
    
    /* The integral must run over the domain where the probability density has 
       the largest values. For the signal photons when the expectation value 
       is mu, the variance is mu and this must be combined with the variance 
       associated with the pedestal width. The variances are considered to 
       add. */
    double stdev = sqrt(mu*(1+exnoise*exnoise)+ped*ped);
    double x_min = mu-FROGS_NUMBER_OF_SIGMA*stdev;
    double x_max = mu+FROGS_NUMBER_OF_SIGMA*stdev;
    //Proceed with the integration. Parameter values unclear
    gsl_integration_qag(&F, x_min, x_max, 1E-7, 1E-7, 3000,3, w, &rtn, &error);
    gsl_integration_workspace_free (w);
  }
  return rtn;
}
//================================================================
//================================================================
double frogs_integrand_for_averaging(double q, void *par) {
  /* This function calculates,-2*ln(P(q|mu)) * P(q|mu) where P(q|mu) 
    is the probability density to get a signal q from a pixel for which 
    the expectation value is mu. This is used in the calculation of the 
    likelihood average.*/
  struct frogs_gsl_func_param *p = (struct frogs_gsl_func_param *)par;
  
  double proba_density;
  proba_density=frogs_probability_density(q, p->mu,p->ped, p->exnoise);

  double loglikelihood=-2.0*log( proba_density );
  
  if(!frogs_is_a_good_number(loglikelihood * proba_density)) 
    frogs_showxerror("NaN resulted from calculations in frogs_integrand_for_averaging"); 
  //Returns the log-likelihood multiplied by the probability it is achieved
  return loglikelihood * proba_density; 
}
//================================================================
//================================================================
struct frogs_imgtemplate frogs_read_template_elev(float elevation) {
  /* This function reads the file whose name is specified by the variable 
     FROGS_TEMPLATE_LIST, searching for the first one matching the elevation 
     provided as an argument. The file should have one line for each template 
     file to be considered. Each line contains in that order: 
     1) The smallest elevation for which the template can be used
     2) The largest elevation for which the template can be used
     3) The file name for that template 
  */

  char *EVN;
  char FROGS_TEMPLATE_LIST_PATH[500];

  EVN  = getenv("EVNDISPSYS");
  sprintf(FROGS_TEMPLATE_LIST_PATH,"%s/bin/%s",EVN,FROGS_TEMPLATE_LIST);

  //Open the template files list file
  FILE *fu; //file pointer
  if((fu = fopen(FROGS_TEMPLATE_LIST_PATH, "r")) == NULL ) {
    printf("%s\n",FROGS_TEMPLATE_LIST_PATH);
    frogs_showxerror("Failed opening the template files list file");
  }

  /*Read the file until the end is encountered unless a matching 
    elevation range is found */
  float minel, maxel; //Min and max elevation to use the listed files
  char fname[500];
  struct frogs_imgtemplate rtn; //Variable to hold template data
  while(fscanf(fu,"%f%f%s",&minel,&maxel,fname)!=EOF) {
    if(elevation>=minel && elevation<=maxel) {
      fclose(fu);//Closes the template filename list 
      //fprintf(stdout,"%f %f %s\n",minel,maxel,fname);
      rtn=frogs_read_template_file(fname);
      rtn.elevmin=minel;
      rtn.elevmax=maxel;
      return rtn;
    }
  }
  fclose(fu);//Closes the template filename list 
  fprintf(stderr,"Elevation %f, check file %s\n",elevation,FROGS_TEMPLATE_LIST_PATH);
  frogs_showxerror("FROGS could not find a matching template file");
  return rtn;
}
//================================================================
//================================================================
struct frogs_imgtemplate frogs_read_template_elev_old(float elevation) {
  /* Read the template in a file for which the elevation provided in 
     argument is a good match. The template data is returned. */
  struct frogs_imgtemplate rtn;
  //Above 65 deg we use the 70 deg template for now
  if(elevation>=65.0) {
//    char template_file_name[FROGS_FILE_NAME_MAX_LENGTH];
    char template_file_name[256];
    strcpy(template_file_name,"$OBS_EVNDISP_ANA_DIR/Templates/elev70X0.0-4.0s0.5r085.tmplt"); 
    rtn=frogs_read_template_file(template_file_name);
    //The template file elevation is not stored in the file and we set it here.
    rtn.elevation=70.0;
    return rtn;
  } 
  /*If the elevation matches that corresponding to a template file, we 
    read that file*/
  if(fabs(elevation-60.0)<=5.0) {
    char template_file_name[FROGS_FILE_NAME_MAX_LENGTH];
    strcpy(template_file_name,"$OBS_EVNDISP_ANA_DIR/Templates/elev60X0.0-4.0s0.5r085.tmplt"); 
    rtn=frogs_read_template_file(template_file_name);
    //The template file elevation is not stored in the file and we set it here.
    rtn.elevation=60.0;
    return rtn;
  } 
  if(fabs(elevation-50.0)<=5.0) {
    char template_file_name[FROGS_FILE_NAME_MAX_LENGTH];
    strcpy(template_file_name,"$OBS_EVNDISP_ANA_DIR/Templates/elev50X0.0-4.0s0.5r085.tmplt"); 
    rtn=frogs_read_template_file(template_file_name);
    //The template file elevation is not stored in the file and we set it here.
    rtn.elevation=50.0;
    return rtn;
  } 
 
  //If we arrive here, that means no appropriate template file was found
  fprintf(stderr,"Elevation=%f\n",elevation);
  frogs_showxerror("Could not find an appropriate template file");
  
  return rtn;
}
//================================================================
//================================================================
struct frogs_imgtemplate 
frogs_read_template_file(
			 char fname[FROGS_FILE_NAME_MAX_LENGTH]) {
  /* Read the image template data in a file whose name is received 
    as an argument.*/

  FILE *fu; //file pointer  
  //open file 
  if((fu = fopen(fname, "r")) == NULL ) {
    frogs_showxerror("Failed opening the template file");
  }


  frogs_printfrog();
  fprintf(stderr,"Reading template file %s\n",fname);

  struct frogs_imgtemplate rtn;
  //Read the number of dimensions
  fscanf(fu,"%d",&(rtn.ndim));
  //Read the step sizes for each parameter
  rtn.step=new float[rtn.ndim];
  for(int i=0;i<rtn.ndim;i++) fscanf(fu,"%f",&(rtn.step[i]));
  //Read the lowest value for each parameter
  rtn.min=new float[rtn.ndim];
  for(int i=0;i<rtn.ndim;i++) fscanf(fu,"%f",&(rtn.min[i]));
  //Read the number of steps for each parameter
  rtn.nstep=new int[rtn.ndim];
  for(int i=0;i<rtn.ndim;i++) fscanf(fu,"%d",&(rtn.nstep[i]));

  //Calculate the number of points in the template data table
  rtn.sz=1;
  for(int i=0;i<rtn.ndim;i++) rtn.sz=rtn.sz*rtn.nstep[i];
  //Read the Cherenkov light densities
  rtn.c=new float [rtn.sz];
  for(int i=0;i<rtn.sz;i++) {
    if(i%(int)floor(0.1*rtn.sz)==0) 
      fprintf(stderr,"%d/100 reading template\n",(int)floor(i*100.0/rtn.sz));
    fscanf(fu,"%f",&(rtn.c[i]));
  }  
  fclose(fu);
  fprintf(stderr,"Done reading template file %s\n",fname);

  return rtn;
}
//================================================================
//================================================================
void frogs_showxerror(const char *msg) {
  /*Print the error message received as an argument and stops the analysis*/
  fprintf(stderr,"%s\n",msg);
  fprintf(stderr,"Terminating now\n");
  exit(0);
}
//================================================================
//================================================================
int frogs_likelihood(const gsl_vector *v, void *ptr, gsl_vector *f) {
  /* Calculates the likelihood for each pixel and store the result in the gsl 
     vector f. The event physical parameters for which the likelihood is 
     calculated are provided in the gsl vector v. All the data from the 
     telescope and from the template are available through the pointer to 
     void ptr*/

  struct frogs_gsl_data_wrapper *dwrap=(struct frogs_gsl_data_wrapper *)ptr;

  //Here is the parameter space point
  struct frogs_reconstruction pnt;
  pnt.xs=gsl_vector_get(v,FROGS_XS);
  pnt.ys=gsl_vector_get(v,FROGS_YS);
  pnt.xp=gsl_vector_get(v,FROGS_XP);
  pnt.yp=gsl_vector_get(v,FROGS_YP);
  pnt.log10e=gsl_vector_get(v,FROGS_LOG10E);
  pnt.lambda=gsl_vector_get(v,FROGS_LAMBDA);

  //This is where the model is called to calculate the expected 
  //value for all pixels in the original version
  int gsl_pix_id=0; //This counter is used as a pixel identified for gsl
  for(int tel=0;tel<dwrap->data->ntel;tel++) {
    for(int pix=0;pix<dwrap->data->scope[tel].npix;pix++) {
      if(dwrap->data->scope[tel].pixinuse[pix]==FROGS_OK) {
	int pix_in_template;//FROGS_OK in image, FROGS_NOTOK in background
	double mu=frogs_img_model(pix,tel,pnt,dwrap->data,dwrap->tmplt,
				  &pix_in_template); 
	if(mu!=FROGS_BAD_NUMBER) {
	  double pd;

          if( mu > 1.e-18 && mu < FROGS_LARGE_PE_SIGNAL )
	    pd=probabilityArray(dwrap->probarray,
			  dwrap->data->scope[tel].q[pix],
			  mu,
			  dwrap->data->scope[tel].ped[pix]);
	  else
	    pd=frogs_probability_density(dwrap->data->scope[tel].q[pix],
				         mu,
				         dwrap->data->scope[tel].ped[pix],
				         dwrap->data->scope[tel].exnoise[pix]);


	  double pix_lkhd=-2.0*log(pd);
	  gsl_vector_set (f,gsl_pix_id,pix_lkhd);
	  gsl_pix_id++;
		  
	}
      }//End of test on pixel viability
    }//End of pixel loop
  }//End of telescope loop
    
  return GSL_SUCCESS;
}
//================================================================
//================================================================
int frogs_likelihood_derivative(const gsl_vector *v, void *ptr, gsl_matrix *J) {
  /* Calculates the likelihood derivative for each pixel and with respect 
    to each event physical parameter and store the result in the gsl 
    matrix J. The event physical parameter for which the likelihood is 
    calculated are provided in the gsl vector v. All the data from the 
    telescope and from the template are available through the pointer to 
    void ptr*/

  struct frogs_gsl_data_wrapper *dwrap=(struct frogs_gsl_data_wrapper *)ptr;

  //Here is the parameter space point
  struct frogs_reconstruction pnt;
  pnt.xs=gsl_vector_get(v,FROGS_XS);
  pnt.ys=gsl_vector_get(v,FROGS_YS);
  pnt.xp=gsl_vector_get(v,FROGS_XP);
  pnt.yp=gsl_vector_get(v,FROGS_YP);
  pnt.log10e=gsl_vector_get(v,FROGS_LOG10E);
  pnt.lambda=gsl_vector_get(v,FROGS_LAMBDA);

  /*This is used to store the finite difference step magnitudes used in 
    the estimation of the derivatives. Eventually their values could be 
    coming in the frogs_gsl_data_wrapper structure*/
  struct frogs_reconstruction delta;

  delta.xs=0.07;
  delta.ys=0.07;
  delta.xp=5.0;
  delta.yp=5.0;
  delta.log10e=0.03;
  delta.lambda=0.3;

  int gsl_pix_id=0; //This counter is used as a pixel identified for gsl
  for(int tel=0;tel<dwrap->data->ntel;tel++) {
    for(int pix=0;pix<dwrap->data->scope[tel].npix;pix++) {
      if(dwrap->data->scope[tel].pixinuse[pix]==FROGS_OK) {
	double derivative;
	//Derivative with respect to xs
	derivative=frogs_pix_lkhd_deriv_2ndorder(pix,tel,pnt,delta,dwrap->data,
						 dwrap->tmplt,FROGS_XS,dwrap->probarray);
	/*derivative=frogs_pix_lkhd_deriv_4thorder(pix,tel,pnt,delta,
	  dwrap->data,dwrap->tmplt,FROGS_XS);*/
	gsl_matrix_set(J,gsl_pix_id,FROGS_XS,derivative);

	//Derivative with respect to ys
	derivative=frogs_pix_lkhd_deriv_2ndorder(pix,tel,pnt,delta,dwrap->data,
						 dwrap->tmplt,FROGS_YS,dwrap->probarray);
	/*derivative=frogs_pix_lkhd_deriv_4thorder(pix,tel,pnt,delta,
	  dwrap->data,dwrap->tmplt,FROGS_YS);*/
	gsl_matrix_set(J,gsl_pix_id,FROGS_YS,derivative);

	//Derivative with respect to xp
	derivative=frogs_pix_lkhd_deriv_2ndorder(pix,tel,pnt,delta,dwrap->data,
						 dwrap->tmplt,FROGS_XP,dwrap->probarray);
	/*derivative=frogs_pix_lkhd_deriv_4thorder(pix,tel,pnt,delta,
	dwrap->data,dwrap->tmplt,FROGS_XP);*/
	gsl_matrix_set(J,gsl_pix_id,FROGS_XP,derivative);

	//Derivative with respect to yp
	derivative=frogs_pix_lkhd_deriv_2ndorder(pix,tel,pnt,delta,dwrap->data,
						 dwrap->tmplt,FROGS_YP,dwrap->probarray);
	/*derivative=frogs_pix_lkhd_deriv_4thorder(pix,tel,pnt,delta,
	dwrap->data,dwrap->tmplt,FROGS_YP);*/
	gsl_matrix_set(J,gsl_pix_id,FROGS_YP,derivative);

	//Derivative with respect to log10e
	derivative=frogs_pix_lkhd_deriv_2ndorder(pix,tel,pnt,delta,dwrap->data,
						 dwrap->tmplt,FROGS_LOG10E,dwrap->probarray);
	/*derivative=frogs_pix_lkhd_deriv_4thorder(pix,tel,pnt,delta,
	dwrap->data,dwrap->tmplt,FROGS_LOG10E);*/
	gsl_matrix_set(J,gsl_pix_id,FROGS_LOG10E,derivative);

	//Derivative with respect to lambda
	derivative=frogs_pix_lkhd_deriv_2ndorder(pix,tel,pnt,delta,dwrap->data,
						 dwrap->tmplt,FROGS_LAMBDA,dwrap->probarray);
	/*derivative=frogs_pix_lkhd_deriv_4thorder(pix,tel,pnt,delta,
	  dwrap->data, dwrap->tmplt,FROGS_LAMBDA); */
	gsl_matrix_set(J,gsl_pix_id,FROGS_LAMBDA,derivative);
	
	gsl_pix_id++;
      }//End of test on pixel viability
    }//End of pixel loop
  }//End of telescope loop

  return GSL_SUCCESS;
}
//================================================================
//================================================================
double frogs_pix_lkhd_deriv_2ndorder(int pix, int tel,
				     struct frogs_reconstruction pnt, 
				     struct frogs_reconstruction delta,
				     struct frogs_imgtmplt_in *d,
				     struct frogs_imgtemplate *tmplt,
				     int gsl_par_id,
				     struct frogs_probability_array *prob_array ) {
  /*Calculates the derivative of the pixel likelihood for the pixel specified 
    by tel and pix and with respect to the parameter specified by gsl_par_id. 
    This function calculate the derivative as df/dx=[f(x+dx)-f(x-dx)]/(2*dx)*/

  double rtn;
  double mu;
  double pd;
  int pix_in_template;
  //Step forward 
  mu=frogs_img_model(pix,tel,frogs_param_step(pnt,delta,gsl_par_id,1.0),
		     d,tmplt,&pix_in_template);
  if(mu==FROGS_BAD_NUMBER) 
    frogs_showxerror("frogs_img_model() invoked for an invalid pixel");

  if( mu > 1.e-18 && mu < FROGS_LARGE_PE_SIGNAL )
    pd=probabilityArray(prob_array,d->scope[tel].q[pix],mu,
                  d->scope[tel].ped[pix]);
  else
    pd=frogs_probability_density(d->scope[tel].q[pix],mu,
			         d->scope[tel].ped[pix],
			         d->scope[tel].exnoise[pix]);


  double pix_lkhd_plus=-2.0*log(pd);
  //Step backward
  mu=frogs_img_model(pix,tel,frogs_param_step(pnt,delta,gsl_par_id,-1.0),
		     d,tmplt,&pix_in_template);
  if(mu==FROGS_BAD_NUMBER) 
    frogs_showxerror("frogs_img_model() invoked for an invalid pixel");

  if( mu > 1.e-18 && mu < FROGS_LARGE_PE_SIGNAL )
    pd=probabilityArray(prob_array,d->scope[tel].q[pix],mu,
                  d->scope[tel].ped[pix]);
  else
    pd=frogs_probability_density(d->scope[tel].q[pix],mu,
			         d->scope[tel].ped[pix],
			         d->scope[tel].exnoise[pix]);

  double pix_lkhd_minus=-2.0*log(pd);

  //Evaluate the derivative
  float delta_param=0;
  if(gsl_par_id==FROGS_XS) delta_param=delta.xs;
  if(gsl_par_id==FROGS_YS) delta_param=delta.ys;
  if(gsl_par_id==FROGS_XP) delta_param=delta.xp;
  if(gsl_par_id==FROGS_YP) delta_param=delta.yp;
  if(gsl_par_id==FROGS_LOG10E) delta_param=delta.log10e;
  if(gsl_par_id==FROGS_LAMBDA) delta_param=delta.lambda;
  if(delta_param==0) 
    frogs_showxerror("Bad parameter identifier in pix_lkhd_deriv_2ndorder");  
  rtn=(pix_lkhd_plus-pix_lkhd_minus)/(2.0*delta_param);

  return rtn;
}
//================================================================
//================================================================
double frogs_pix_lkhd_deriv_4thorder(int pix, int tel,
				     struct frogs_reconstruction pnt, 
				     struct frogs_reconstruction delta,
				     struct frogs_imgtmplt_in *d,
				     struct frogs_imgtemplate *tmplt,
				     int gsl_par_id) {
  /*Calculates the derivative of the pixel likelihood for the pixel specified 
    by tel and pix and with respect to the parameter specified by gsl_par_id. 
    This function calculate the derivative as 
    df/dx=[-f(x+2*dx)+8f(x+dx)-8f(x-dx)+f(x-2dx)]/(12*dx)*/

  double rtn;
  double mu;
  double pd;
  int pix_in_template;
  //Step forward 
  mu=frogs_img_model(pix,tel,frogs_param_step(pnt,delta,gsl_par_id,1.0),
		     d,tmplt,&pix_in_template);
  if(mu==FROGS_BAD_NUMBER) 
    frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
  pd=frogs_probability_density(d->scope[tel].q[pix],mu,
			       d->scope[tel].ped[pix],
			       d->scope[tel].exnoise[pix]);
  double pix_lkhd_plus=-2.0*log(pd);
  //Step backward
  mu=frogs_img_model(pix,tel,frogs_param_step(pnt,delta,gsl_par_id,-1.0),
		     d,tmplt,&pix_in_template);
  if(mu==FROGS_BAD_NUMBER) 
    frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
  pd=frogs_probability_density(d->scope[tel].q[pix],mu,
			       d->scope[tel].ped[pix],
			       d->scope[tel].exnoise[pix]);
  double pix_lkhd_minus=-2.0*log(pd);
  //two Step forward 
  mu=frogs_img_model(pix,tel,frogs_param_step(pnt,delta,gsl_par_id,2.0),
		     d,tmplt,&pix_in_template);
  if(mu==FROGS_BAD_NUMBER) 
    frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
  pd=frogs_probability_density(d->scope[tel].q[pix],mu,
			       d->scope[tel].ped[pix],
			       d->scope[tel].exnoise[pix]);
  double pix_lkhd_plus_plus=-2.0*log(pd);
  //two Step backward
  mu=frogs_img_model(pix,tel,frogs_param_step(pnt,delta,gsl_par_id,-2.0),
		     d,tmplt,&pix_in_template);
  if(mu==FROGS_BAD_NUMBER) 
    frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
  pd=frogs_probability_density(d->scope[tel].q[pix],mu,
			       d->scope[tel].ped[pix],
			       d->scope[tel].exnoise[pix]);
  double pix_lkhd_minus_minus=-2.0*log(pd);

  //Evaluate the derivative
  float delta_param=0;
  if(gsl_par_id==FROGS_XS) delta_param=delta.xs;
  if(gsl_par_id==FROGS_YS) delta_param=delta.ys;
  if(gsl_par_id==FROGS_XP) delta_param=delta.xp;
  if(gsl_par_id==FROGS_YP) delta_param=delta.yp;
  if(gsl_par_id==FROGS_LOG10E) delta_param=delta.log10e;
  if(gsl_par_id==FROGS_LAMBDA) delta_param=delta.lambda;
  if(delta_param==0) 
    frogs_showxerror("Bad parameter identifier in pix_lkhd_deriv_2ndorder");  
  rtn=(-pix_lkhd_plus_plus+8*pix_lkhd_plus-8*pix_lkhd_minus+pix_lkhd_minus_minus)/(12*delta_param);
  
  return rtn;
}

//================================================================
//================================================================
struct frogs_reconstruction frogs_param_step(struct frogs_reconstruction pnt,
					     struct frogs_reconstruction delta,
					     int gsl_par_id,float mult) {
  /*This function returns a point of the parameter space equal to pnt with 
    a change applied to one of the parameters as specified by gsl_par_id 
    with an amplitude equal to delta and multiplied by mult. It is used in 
    the function frogs_pix_lkhd_deriv calculating the likelihood derivaive.*/
  struct frogs_reconstruction rtn;
  rtn=pnt;
  if(gsl_par_id==FROGS_XS) rtn.xs=rtn.xs+mult*delta.xs;
  if(gsl_par_id==FROGS_YS) rtn.ys=rtn.ys+mult*delta.ys;
  if(gsl_par_id==FROGS_XP) rtn.xp=rtn.xp+mult*delta.xp;
  if(gsl_par_id==FROGS_YP) rtn.yp=rtn.yp+mult*delta.yp;
  if(gsl_par_id==FROGS_LOG10E) rtn.log10e=rtn.log10e+mult*delta.log10e;
  if(gsl_par_id==FROGS_LAMBDA) rtn.lambda=rtn.lambda+mult*delta.lambda;
  
  return rtn;
}
//================================================================
//================================================================
int frogs_likelihood_fdf(const gsl_vector *v, void *ptr, gsl_vector *f, 
		   gsl_matrix *J) {
  /*It is not clear what this is good for but GSL needs to see that 
    for the Levenberg-Marquardt optimisation*/
  frogs_likelihood(v, ptr, f);
  frogs_likelihood_derivative(v, ptr, J);
  return GSL_SUCCESS;
}
//================================================================
//================================================================
double frogs_img_model(int pix,int tel,struct frogs_reconstruction pnt,
		struct frogs_imgtmplt_in *d,struct frogs_imgtemplate *tmplt,
		       int *intemplate) {
  /* This function calculates and returns the expected signal in 
     photoelectrons in pixel pix in telescope tel for the event physical 
     parameters specified in pnt and using the telescope properties in d 
     and the image template table tmplt. If the pixel is outside the region 
     covered by the template, the value *intemplate is set to FROGS_NOTOK 
     while otherwise it is set to FROGS_OK. If the pixel is invalid both the 
     returned value and intemplate are set to  FROGS_BAD_NUMBER */
  
  //If the pixel is not active return FROGS_BAD_NUMBER
  if(d->scope[tel].pixinuse[pix]!=FROGS_OK) {
    *intemplate=FROGS_BAD_NUMBER;
    return FROGS_BAD_NUMBER;
  }
  
  //Angle between the x direction and the shower image axis
  float phi=atan2(pnt.yp-d->scope[tel].yfield,pnt.xp-d->scope[tel].xfield);
  phi=phi+FROGS_PI; //This is for real data only. We'll have to understand that
  float cphi=cos(phi);
  float sphi=sin(phi);
  //Impact parameter to the telescope
  float timp=sqrt((pnt.yp-d->scope[tel].yfield)*
		  (pnt.yp-d->scope[tel].yfield)+
		  (pnt.xp-d->scope[tel].xfield)*
		  (pnt.xp-d->scope[tel].xfield));
  //Subtract the source coordinate from the pixel coordinate
  float xrs=d->scope[tel].xcam[pix]-pnt.xs;
  float yrs=-(d->scope[tel].ycam[pix]-pnt.ys);
  //Apply a rotation to move to the template coordinate system
  float tmpltxpix=xrs*cphi+yrs*sphi; 
  float tmpltypix=-xrs*sphi+yrs*cphi;

  /*The optimization has a tendency to drag the value of lambda 
    outside the range covered by the template table. In order to avoid 
    the effects of this on the model, when the value is out of range 
    we use the lambda value of the range that is the closest.*/
  float maxlambda=tmplt->min[0]+(tmplt->nstep[0]-1)*tmplt->step[0];
  pnt.lambda=floatwrap(pnt.lambda,tmplt->min[0],maxlambda);
  
  //Here we get the pixel value from the template table
  double rtn;
  if(FROGS_INTERP_ORDER==1) rtn=frogs_chertemplate_lin(pnt.lambda,pnt.log10e,
						       timp,tmpltxpix,
						       tmpltypix,tmplt,
						       intemplate);
  if(FROGS_INTERP_ORDER==2) rtn=frogs_chertemplate_quad(pnt.lambda,pnt.log10e,
							timp,tmpltxpix,
							tmpltypix,tmplt,
							intemplate);
  
  /* The image template data is in photo-electrons per square meter and per 
     square degree. We muliply this density by the telescope area and by the 
     pixel area. */
  rtn=rtn*d->scope[tel].telpixarea[pix];
  return rtn;
}
//================================================================
//================================================================
double frogs_chertemplate_lin(float lambda,float log10e,float b,float x,
			     float y,struct frogs_imgtemplate *tmplt,
			     int *intemplate) {
  /*This function return an evaluation of the Cherenkov ight density for 
    the given values of lambda the depth of the first interaction point, 
    log10e = log10(E/TeV), b the impact parameter to the telescope, x and 
    y the longitudinal and transverse coordinate with respect to the source 
    and the direction of development of the shower image. The evaluation is 
    obtained by linear interpolation in lambda,log10e and b in that order. 
    When the parameters fall outside the range covered by the template table, 
    the value is linearly extrapolated. 
    For x and y the closest table values are simply used.   
  */
  /* Note:in the template data table, the index to parameter 
    correspondance is as follows
    0 --- lambda = 1st interaction depth in interaction lengths
    1 --- log10e = log10(E/1TeV)
    2 --- b      = impact parameter to the considered telescope
    3 --- x      = x coordinate of the pixel (see note)
    4 --- y      = y coordinate of the pixel (see note)
    This has nothing to do with the order in which the parameters are 
    entered in GSL for the likelihood optimization. 
    (x and y measured in degrees. x along the image major axis from 
    the source and increasing with the age of the shower. y in a 
    perpendicular direction. )*/

  // index for x we will not interpolate
  int ix=(int)floor((x-tmplt->min[3])/tmplt->step[3]);
  if(ix<0||ix>=tmplt->nstep[3]) {*intemplate=FROGS_NOTOK;return 0.0;} 
 
  // index for y we will not interpolate
  int iy=(int)floor((fabs(y)-tmplt->min[4])/tmplt->step[4]);
  if(iy<0||iy>=tmplt->nstep[4]) {*intemplate=FROGS_NOTOK;return 0.0;} 

  //If we get here the pixel is within the area covered by the templates
  *intemplate=FROGS_OK;

  /*For lambda we will proceed to a linear interpolation. We need two 
    bracketing indices*/
  int il1=(int)floor((lambda-tmplt->min[0])/tmplt->step[0]);
  int il2=il1+1;
  if(il1<0) {il1=0;il2=1;} 
  if(il2>=tmplt->nstep[0]) {il2=tmplt->nstep[0]-1;il1=il2-1;}
  float l1=tmplt->min[0]+il1*tmplt->step[0];
  float l2=tmplt->min[0]+il2*tmplt->step[0];

  //For the energy as well we will use a linear interpolation. 
  int iloge1=(int)floor((log10e-tmplt->min[1])/tmplt->step[1]);
  int iloge2=iloge1+1;
  if(iloge1<0) {iloge1=0;iloge2=1;} 
  if(iloge2>=tmplt->nstep[1]) {iloge2=tmplt->nstep[1]-1;iloge1=iloge2-1;}
  float loge1=tmplt->min[1]+iloge1*tmplt->step[1];
  float loge2=tmplt->min[1]+iloge2*tmplt->step[1];

  //For the impact parameter we will use a linear interpolation as well for now
  int ib1=(int)floor((b-tmplt->min[2])/tmplt->step[2]);
  int ib2=ib1+1;
  if(ib1<0) {ib1=0;ib2=1;} 
  if(ib2>=tmplt->nstep[2]) {ib2=tmplt->nstep[2]-1;ib1=ib2-1;}
  float b1=tmplt->min[2]+ib1*tmplt->step[2];
  float b2=tmplt->min[2]+ib2*tmplt->step[2];

  //Get the model values at the vertices
  double mu111=frogs_get_tmplt_val(il1,iloge1,ib1,ix,iy,tmplt);
  double mu112=frogs_get_tmplt_val(il1,iloge1,ib2,ix,iy,tmplt);
  double mu121=frogs_get_tmplt_val(il1,iloge2,ib1,ix,iy,tmplt);
  double mu122=frogs_get_tmplt_val(il1,iloge2,ib2,ix,iy,tmplt);
  double mu211=frogs_get_tmplt_val(il2,iloge1,ib1,ix,iy,tmplt);
  double mu212=frogs_get_tmplt_val(il2,iloge1,ib2,ix,iy,tmplt);
  double mu221=frogs_get_tmplt_val(il2,iloge2,ib1,ix,iy,tmplt);
  double mu222=frogs_get_tmplt_val(il2,iloge2,ib2,ix,iy,tmplt);

  //Interpolation in lambda first
  double mu011=frogs_linear_interpolation(l1,l2,mu111,mu211,lambda);
  mu011=FROGS_NONEG(mu011);
  double mu012=frogs_linear_interpolation(l1,l2,mu112,mu212,lambda);
  mu012=FROGS_NONEG(mu012);
  double mu021=frogs_linear_interpolation(l1,l2,mu121,mu221,lambda);
  mu021=FROGS_NONEG(mu021);
  double mu022=frogs_linear_interpolation(l1,l2,mu122,mu222,lambda);
  mu022=FROGS_NONEG(mu022);

  //Interpolation in log(E)
  double mu001=frogs_linear_interpolation(loge1,loge2,mu011,mu021,log10e);
  mu001=FROGS_NONEG(mu001);
  double mu002=frogs_linear_interpolation(loge1,loge2,mu012,mu022,log10e);
  mu002=FROGS_NONEG(mu002);

  //Interpolation in b
  double mu000=frogs_linear_interpolation(b1,b2,mu001,mu002,b);
  mu000=FROGS_NONEG(mu000);
  
  return mu000;
}
//================================================================
//================================================================
double frogs_chertemplate_quad(float lambda,float log10e,float b,float x,
			       float y,struct frogs_imgtemplate *tmplt,
			       int *intemplate) {
  /*This function return an evaluation of the Cherenkov ight density for 
    the given values of lambda the depth of the first interaction point, 
    log10e = log10(E/TeV), b the impact parameter to the telescope, x and 
    y the longitudinal and transverse coordinate with respect to the source 
    and the direction of development of the shower image. The evaluation is 
    obtained by quadratic interpolation in lambda,log10e and b in that order. 
    When the parameters fall outside the range covered by the template table, 
    the value is quadratically extrapolated. 
    For x and y the closest table values are simply used.   
  */
  /* Note:in the template data table, the index to parameter 
    correspondance is as follows
    0 --- lambda = 1st interaction depth in intaraction lengths
    1 --- log10e = log10(E/1TeV)
    2 --- b      = impact parameter to the considered telescope
    3 --- x      = x coordinate of the pixel (see note)
    4 --- y      = y coordinate of the pixel (see note)
    (note: x and y measured in degrees. x along the image major axis from 
    the source and increasing with the age of the shower. y in a 
    perpendicular direction. )*/

  // index for x we will not interpolate
  int ix=(int)floor((x-tmplt->min[3])/tmplt->step[3]);
  //int ix=(int)floor(((x-0.30)-tmplt->min[3])/tmplt->step[3]); //TEMP GH
  //int ix=(int)floor(((x-0.44)-tmplt->min[3])/tmplt->step[3]); //TEMP GH
  if(ix<0||ix>=tmplt->nstep[3]) {*intemplate=FROGS_NOTOK;return 0.0;} 
 
  // index for y we will not interpolate
  int iy=(int)floor((fabs(y)-tmplt->min[4])/tmplt->step[4]);
  if(iy<0||iy>=tmplt->nstep[4]) {*intemplate=FROGS_NOTOK;return 0.0;} 

  //If we get here the pixel is within the area covered by the templates
  *intemplate=FROGS_OK;

  float dummy; //Temporary variable
  /*For lambda we will proceed to a quadratic interpolation. We need three 
    bracketing indices*/
  int il1,il2,il3;
  dummy=(lambda-tmplt->min[0])/tmplt->step[0];
  il1=(int)floor(dummy); il2=il1+1;il3=il2+1;
  if(il1<0) {il1=0;il2=1;il3=2;} 
  if(il3>=tmplt->nstep[0]) {il3=tmplt->nstep[0]-1;il2=il3-1;il1=il2-1;}
  float l1=tmplt->min[0]+il1*tmplt->step[0];
  float l2=tmplt->min[0]+il2*tmplt->step[0];
  float l3=tmplt->min[0]+il3*tmplt->step[0];

  //For the energy as well we will use a quadratic interpolation as well. 
  int iloge1,iloge2,iloge3;
  dummy=(log10e-tmplt->min[1])/tmplt->step[1];
  iloge1=(int)floor(dummy); iloge2=iloge1+1; iloge3=iloge2+1;
  if(iloge1<0) {iloge1=0;iloge2=1;iloge3=2;} 
  if(iloge3>=tmplt->nstep[1]) {iloge3=tmplt->nstep[1]-1;iloge2=iloge3-1;iloge1=iloge2-1;}
  float loge1=tmplt->min[1]+iloge1*tmplt->step[1];
  float loge2=tmplt->min[1]+iloge2*tmplt->step[1];
  float loge3=tmplt->min[1]+iloge3*tmplt->step[1];

  //For the impact parameter we will use a quadratic interpolation as well
  int ib1,ib2,ib3;
  dummy=(b-tmplt->min[2])/tmplt->step[2];
  ib1=(int)floor(dummy);ib2=ib1+1; ib3=ib2+1;
  if(ib1<0) {ib1=0;ib2=1;ib3=2;} 
  if(ib3>=tmplt->nstep[2]) {ib3=tmplt->nstep[2]-1;ib2=ib3-1;ib1=ib2-1;}
  float b1=tmplt->min[2]+ib1*tmplt->step[2];
  float b2=tmplt->min[2]+ib2*tmplt->step[2];
  float b3=tmplt->min[2]+ib3*tmplt->step[2];

  //Get the model values at the vertices of a 3x3x3 cube
  double mu111=frogs_get_tmplt_val(il1,iloge1,ib1,ix,iy,tmplt);
  double mu112=frogs_get_tmplt_val(il1,iloge1,ib2,ix,iy,tmplt);
  double mu113=frogs_get_tmplt_val(il1,iloge1,ib3,ix,iy,tmplt);
  double mu121=frogs_get_tmplt_val(il1,iloge2,ib1,ix,iy,tmplt);
  double mu122=frogs_get_tmplt_val(il1,iloge2,ib2,ix,iy,tmplt);
  double mu123=frogs_get_tmplt_val(il1,iloge2,ib3,ix,iy,tmplt);
  double mu131=frogs_get_tmplt_val(il1,iloge3,ib1,ix,iy,tmplt);
  double mu132=frogs_get_tmplt_val(il1,iloge3,ib2,ix,iy,tmplt);
  double mu133=frogs_get_tmplt_val(il1,iloge3,ib3,ix,iy,tmplt);
  double mu211=frogs_get_tmplt_val(il2,iloge1,ib1,ix,iy,tmplt);
  double mu212=frogs_get_tmplt_val(il2,iloge1,ib2,ix,iy,tmplt);
  double mu213=frogs_get_tmplt_val(il2,iloge1,ib3,ix,iy,tmplt);
  double mu221=frogs_get_tmplt_val(il2,iloge2,ib1,ix,iy,tmplt);
  double mu222=frogs_get_tmplt_val(il2,iloge2,ib2,ix,iy,tmplt);
  double mu223=frogs_get_tmplt_val(il2,iloge2,ib3,ix,iy,tmplt);
  double mu231=frogs_get_tmplt_val(il2,iloge3,ib1,ix,iy,tmplt);
  double mu232=frogs_get_tmplt_val(il2,iloge3,ib2,ix,iy,tmplt);
  double mu233=frogs_get_tmplt_val(il2,iloge3,ib3,ix,iy,tmplt);
  double mu311=frogs_get_tmplt_val(il3,iloge1,ib1,ix,iy,tmplt);
  double mu312=frogs_get_tmplt_val(il3,iloge1,ib2,ix,iy,tmplt);
  double mu313=frogs_get_tmplt_val(il3,iloge1,ib3,ix,iy,tmplt);
  double mu321=frogs_get_tmplt_val(il3,iloge2,ib1,ix,iy,tmplt);
  double mu322=frogs_get_tmplt_val(il3,iloge2,ib2,ix,iy,tmplt);
  double mu323=frogs_get_tmplt_val(il3,iloge2,ib3,ix,iy,tmplt);
  double mu331=frogs_get_tmplt_val(il3,iloge3,ib1,ix,iy,tmplt);
  double mu332=frogs_get_tmplt_val(il3,iloge3,ib2,ix,iy,tmplt);
  double mu333=frogs_get_tmplt_val(il3,iloge3,ib3,ix,iy,tmplt);

  //Interpolation in lambda first
  double mu011=frogs_quadratic_interpolation(l1,l2,l3,mu111,mu211,mu311,lambda);
  mu011=FROGS_NONEG(mu011);
  double mu012=frogs_quadratic_interpolation(l1,l2,l3,mu112,mu212,mu312,lambda);
  mu012=FROGS_NONEG(mu012);
  double mu013=frogs_quadratic_interpolation(l1,l2,l3,mu113,mu213,mu313,lambda);
  mu013=FROGS_NONEG(mu013);
  double mu021=frogs_quadratic_interpolation(l1,l2,l3,mu121,mu221,mu321,lambda);
  mu021=FROGS_NONEG(mu021);
  double mu022=frogs_quadratic_interpolation(l1,l2,l3,mu122,mu222,mu322,lambda);
  mu022=FROGS_NONEG(mu022);
  double mu023=frogs_quadratic_interpolation(l1,l2,l3,mu123,mu223,mu323,lambda);
  mu023=FROGS_NONEG(mu023);
  double mu031=frogs_quadratic_interpolation(l1,l2,l3,mu131,mu231,mu331,lambda);
  mu031=FROGS_NONEG(mu031);
  double mu032=frogs_quadratic_interpolation(l1,l2,l3,mu132,mu232,mu332,lambda);
  mu032=FROGS_NONEG(mu032);
  double mu033=frogs_quadratic_interpolation(l1,l2,l3,mu133,mu233,mu333,lambda);
  mu033=FROGS_NONEG(mu033);

  //Interpolation in log(E)
  double mu001=frogs_quadratic_interpolation(loge1,loge2,loge3,mu011,mu021,
					    mu031,log10e);
  mu001=FROGS_NONEG(mu001);
  double mu002=frogs_quadratic_interpolation(loge1,loge2,loge3,mu012,mu022,
					    mu032,log10e);
  mu002=FROGS_NONEG(mu002);
  double mu003=frogs_quadratic_interpolation(loge1,loge2,loge3,mu013,mu023,
					    mu033,log10e);
  mu003=FROGS_NONEG(mu003);

  //Interpolation in b
  double mu000=frogs_quadratic_interpolation(b1,b2,b3,mu001,mu002,mu003,b);
  mu000=FROGS_NONEG(mu000);

  return mu000; 
}
//================================================================
//================================================================
double frogs_get_tmplt_val(int il,int iloge,int ib,int ix,int iy,
		  struct frogs_imgtemplate *tmplt) {
  /*The template table is stored in a one dimensional table. This 
    function is used to return the template table content for the given 
    indices for each of the entry parameters: il,iloge, ib, ix and iy*/
  /* Note:in the template data table, the index to parameter 
    correspondance is as follows
    0 --- lambda = 1st interaction depth in intaraction lengths
    1 --- log10e = log10(E/1TeV)
    2 --- b      = impact parameter to the considered telescope
    3 --- x      = x coordinate of the pixel (see note)
    4 --- y      = y coordinate of the pixel (see note)*/

  int index; 
  index=(((il*tmplt->nstep[1]+iloge)*tmplt->nstep[2]+ib)*
	 tmplt->nstep[3]+ix)*tmplt->nstep[4]+iy;
  if(index<0 || index>=tmplt->sz) 
    frogs_showxerror("Index out of range in frogs_get_tmplt_val");
  double rtn=tmplt->c[index];
  return rtn;
}
//================================================================
//================================================================
int frogs_print_param_spc_point(struct frogs_imgtmplt_out output){
  /*This function can be used to print out the results or the current 
    status of the image template analysis. */
  fprintf(stderr,"----------------------------------------------\n");
  fprintf(stderr,"Event ID number:%d\n",output.event_id);
  fprintf(stderr,"GSL convergence status:%d\n",output.gsl_convergence_status);
  fprintf(stderr,"Number of iterations:%d\n",output.nb_iter);
  fprintf(stderr,"xs=%f +/- %f degrees\n",output.cvrgpt.xs,
	  output.cvrgpterr.xs);
  fprintf(stderr,"ys=%f +/- %f degrees\n",output.cvrgpt.ys,
	  output.cvrgpterr.ys);
  fprintf(stderr,"xp=%f +/- %f m\n",output.cvrgpt.xp,output.cvrgpterr.xp);
  fprintf(stderr,"yp=%f +/- %f m\n",output.cvrgpt.yp,output.cvrgpterr.yp);
  fprintf(stderr,"log10(E/TeV)=%f +/- %f\n",output.cvrgpt.log10e,
	  output.cvrgpterr.log10e);
  fprintf(stderr,"lambda=%f +/- %f g/cm2\n",output.cvrgpt.lambda,
	  output.cvrgpterr.lambda);
  fprintf(stderr,"..............................................\n");
  fprintf(stderr,"Image goodness Gi=%f for %d d.o.f.\n",output.goodness_img,
	  output.npix_img);
  fprintf(stderr,"Gkgnd goodness Gb=%f for %d d.o.f.\n",output.goodness_bkg,
	  output.npix_bkg);
  fprintf(stderr,"----------------------------------------------\n");

  return FROGS_OK;
}
//================================================================
//================================================================
double frogs_linear_interpolation(float x1, float x2, double y1, double y2, 
				 float x) {
  /*Returns the value in x of the first degree Lagrange polynomial 
    constructed on points (x1,y1) and (x2,y2)*/
  double rtn;
  if(x1==x2) frogs_showxerror("Error in two point linear lagrange interpolation");
  rtn=y1*(x-x2)/(x1-x2)+y2*(x-x1)/(x2-x1);
  return rtn;
}
//================================================================
//================================================================
double frogs_quadratic_interpolation(float x1, float x2, float x3, double y1, 
			      double y2, double y3, float x) {
  /*Returns the value in x of the second degree Lagrange polynomial 
    constructed on points (x1,y1), (x2,y2) and (x3,y3)*/
  double rtn;
  if(x1==x2 || x1==x3 || x2==x3) 
    frogs_showxerror("Error in three point quadratic Lagrange interpolation");
  rtn=y1*(x-x2)*(x-x3)/((x1-x2)*(x1-x3))
     +y2*(x-x1)*(x-x3)/((x2-x1)*(x2-x3))
     +y3*(x-x1)*(x-x2)/((x3-x1)*(x3-x2));
  return rtn;
}
//================================================================
//================================================================
float floatwrap(float x,float min,float max) {
  /*This function returns a value between min and max. It is periodic 
    in x of periode 2*(max-min). For x in [min,max] the returned value 
    is x. For x in [max,2*max-min], the return value is 2*max-x. 
    This function is used on parameters controled by the GSL optimization 
    to maintain the parameter within the range for which we have models 
    without introducing any discontinuity
     
  rtn ^ 
    o |         o           o
  o   o       o | o       o   o       o
      | o   o   |   o   o       o   o
      |   o     |     o           o
      | . |     |     | 
    0 .---|-----|-----|---------------------> x
     0   min   max   2*max-min
*/

  float rtn;
  if(min>max) frogs_showxerror("In floatwrap, min>max");
  float range=max-min;
  float dumx=x-min;
  rtn=min+range-fabs(dumx-range*(2*floor(0.5*dumx/range)+1));
  return rtn;
}
//================================================================
//================================================================
int frogs_printfrog() {
  /*This function clearly is the most important one of the project. */
  fprintf(stderr," ---------------------------------------------------------------- \n");
  fprintf(stderr,"|      _    _                                        _    _      |\n");
  fprintf(stderr,"|     (o)--(o)                                      (o)--(o)     |\n");
  fprintf(stderr,"|    /.______.\\    FFFF RRRR   OOO   GGG   SSS     /.______.\\    |\n");
  fprintf(stderr,"|    \\________/    F    R   R O   O G     S        \\________/    |\n");
  fprintf(stderr,"|  ./        \\.    FFF  RRRR  O   O G  GG  SSS     ./        \\.  |\n");
  fprintf(stderr,"| ( .        , )   F    R R   O   O G   G     S   ( .        , ) |\n");
  fprintf(stderr,"|  \\ \\_\\\\//_/ /    F    R  RR  OOO   GGG  SSSS     \\ \\_\\\\//_/ /  |\n");
  fprintf(stderr,"|   ~~  ~~  ~~                                      ~~  ~~  ~~   |\n");
  fprintf(stderr,"| svincent@physics.utah.edu             lebohec@physics.utah.edu |\n");
  fprintf(stderr,"|                 VERSION 1.02 OCTOBER 10th 2011                 |\n");
  fprintf(stderr,"|  For license issues, see www.physics.utah.edu/gammaray/FROGS   |\n");
  fprintf(stderr," ---------------------------------------------------------------- \n");
  return FROGS_OK;
}
//================================================================
//================================================================
void fill_prob_density( struct frogs_probability_array *parray )
{

/*
 * Function fills probability density table to speed up calculations
 * Table is filled once at the beginning of the analysis.
 */

 fprintf(stderr,"\nFROGS: Filling Probability Density Table ......... ");

 int i,j,k;
 double q,mu,ped;

 for( i=0; i<BIN1; i++ )
 {

   q = ((float)i*(RANGE1-MIN1)/BIN1 + MIN1);

   for( j=0; j<BIN2; j++ )
   {

     mu = ((float)j*(RANGE2-MIN2)/BIN2 + MIN2);

     //if( fabs(q-mu) < 100. )
     if( 1 )
     {

       for( k=0; k<BIN3; k++ )
       {

         ped = (float)k*(RANGE3-MIN3)/BIN3 + MIN3;
         parray->prob_density_table[i][j][k] = frogs_probability_density(q, mu, ped, 0.35);

       }
     } 
   }
 }

 fprintf(stderr," completed reading %d x %d x %d entries.\n\n",BIN1,BIN2,BIN3);

 return;

}

double probabilityArray( struct frogs_probability_array *prob_array, double q, double mu, double ped )
{

/*
 * Reads the  probability table and extrapolates in 3 dimensions to find correct probability density.
 */

 float ii,jj,kk;
 int imin,jmin,kmin;
 int imax,jmax,kmax;

 ii = BIN1*(q-MIN1)/(RANGE1-MIN1);
 jj = BIN2*(mu-MIN2)/(RANGE2-MIN2);
 kk = BIN3*(ped-MIN3)/(RANGE3-MIN3);


 imin = (int)ii;
 jmin = (int)jj;
 kmin = (int)kk;

 imax = imin+1;
 jmax = jmin+1;
 kmax = kmin+1;

 if( imin < 0 || kmin < 0 || jmin < 0 || imax >= BIN1-1 || jmax >= BIN2-1 || kmax >= BIN3-1 )
 {
   return frogs_probability_density(q,mu,ped,0.35);
 }

 
 float qmin = imin*(RANGE1-MIN1)/BIN1 + MIN1;
 float mumin = jmin*(RANGE2-MIN2)/BIN2 + MIN2;
 float pedmin = kmin*(RANGE3-MIN3)/BIN3 + MIN3;

 float qmax = imax*(RANGE1-MIN1)/BIN1 + MIN1;
 float mumax = jmax*(RANGE2-MIN2)/BIN2 + MIN2;
 float pedmax = kmax*(RANGE3-MIN3)/BIN3 + MIN3;

 float qdel = (q-qmin)/(qmax - qmin);
 float mudel = (mu-mumin)/(mumax - mumin);
 float peddel = (ped-pedmin)/(pedmax - pedmin);

 double f000 = prob_array->prob_density_table[imin][jmin][kmin];
 double f001 = prob_array->prob_density_table[imin][jmin][kmax];
 double f010 = prob_array->prob_density_table[imin][jmax][kmin];
 double f011 = prob_array->prob_density_table[imin][jmax][kmax];
 double f100 = prob_array->prob_density_table[imax][jmin][kmin];
 double f101 = prob_array->prob_density_table[imax][jmin][kmax];
 double f110 = prob_array->prob_density_table[imax][jmax][kmin];
 double f111 = prob_array->prob_density_table[imax][jmax][kmax];


 double c0 = f000;
 double c1 = f100-f000;
 double c2 = f010-f000;
 double c3 = f001-f000;
 double c4 = f110-f010-f100+f000;
 double c5 = f011-f001-f010+f000;
 double c6 = f101-f001-f100+f000;
 double c7 = f111-f011-f101-f110+f100+f001+f010-f000;

 double p = c0;
 p += c1*qdel;
 p += c2*mudel;
 p += c3*peddel;
 p += c4*qdel*mudel;
 p += c5*mudel*peddel;
 p += c6*peddel*qdel;
 p += c7*qdel*mudel*peddel;

 return p;

}
