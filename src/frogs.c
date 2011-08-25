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
|                    VERSION FEBRUARY 21st 2011                    |
\*================================================================*/

/*
//The FROGS analysis needs the folowing definitions.
//It also need some less common definition files for GSL. Those are 
//included in the frogs.h file
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
*/

/*
//In the GrISU context, FROGS needs the following definition files. 
#include "./anal_definition.h"  //GrISU/Analysis/
#include "./showerror.h" //in GrISU/CommonTools
#include "./read_array.h"  //in GrISU/Config/Read/
*/

#define FROGS_TEST 0
#include "frogs.h"
//================================================================
//================================================================
/*
int main(void) {
  //This main function should be commented out. It is provided to 
  //give an example of how to call the FROGS analysis in the GrISU 
  //context. 

  //If the main function is uncommented, the FROGS analysis can be compiled 
  //with:
  //INCLUDE=-I/usr/local/include
  //GSLIB=-L/usr/local/lib -lgsl -lgslcblas
  //imgtmpl: frogs.c frogs.h Makefile
  //	 g++ frogs.c  -O -w $(INCLUDE)  $(GSLIB) -o frogs -lm -Wall
  //in a make file. There will be a segfault at execution time because of 
  //the GrISU specific structure will not contain any meaningful data. 

  struct array_event taevnt;
  struct array ta;
  int adc=2;
  struct array_ped taped;  

  //Store data from the GrISU analysis to a FROGS structure
  //In this example the function frogs_convert_from_grisu takes 
  //arguments corresponding the the GrISU analysis. An equivalent 
  //function should be written for any other analysis package to 
  //make the FROGS analysis usable. 
  struct frogs_imgtmplt_in d;
  d=frogs_convert_from_grisu(&taevnt,&ta,adc,&taped);

  //Print out the data contained in the FROGS structure frogs_imgtmplt_in
  //This is useful when developing a frogs_convert_from_XXXX function
  frogs_print_raw_event(d);

  //Call the FROGS analysis
  struct frogs_imgtmplt_out output;
  output=frogs_img_tmplt(d);
  
  //Print out the results of the image template analysis
  //The values returned by frogs_img_tmplt really are to be stored in 
  //structures used in the analysis package in which FROGS is being sed. 
  frogs_print_param_spc_point(output);
  
  return FROGS_OK;
}
*/
//================================================================
//================================================================
struct frogs_imgtmplt_in frogs_convert_from_grisu(struct array_event *taevnt, 
						  struct array *ta,
						  int adc_type,
						  struct array_ped *taped) {
  /* The frogs_convert_from_grisu function is called with 
     arguments containing all the data necessary to the image template 
     analysis as per the structures used in grisu analysis. It can serve 
     as an example for the interfacing of the template analysis with other 
     analysis packages. It returns the data necessary to the template 
     analysis in a structure that is appropriate.  */
  
  struct frogs_imgtmplt_in rtn;

  //SLB: need to move the event worthyness decision to here. 

  //Tracked elevation from telescope 0
  rtn.elevation=taevnt->teltrack[0].elev*FROGS_DEG_PER_RAD; 
  rtn.event_id=taevnt->event_id;

  //Telescopes
  rtn.ntel=ta->nbr_tel; //Number of telescopes
  rtn.scope=new struct frogs_telescope [rtn.ntel];
  rtn.nb_live_pix_total=0;//Total number or pixels in use
  for(int tel=0;tel<rtn.ntel;tel++) {
    //Telescope position in the coordonate system used in the reconstruction
    rtn.scope[tel].xfield=ta->scope[tel].xfieldrot;
    rtn.scope[tel].yfield=ta->scope[tel].yfieldrot;
    //Telescope effective collection area
    float telarea=94; //telescope area could be from the configuration file
    //Number of pixels 
    rtn.scope[tel].npix=ta->scope[tel].camera.nbr_pmt;
    //Set the dimension of the pixel parameter arrays
    rtn.scope[tel].xcam= new float [rtn.scope[tel].npix];
    rtn.scope[tel].ycam= new float [rtn.scope[tel].npix];
    rtn.scope[tel].q= new float [rtn.scope[tel].npix];
    rtn.scope[tel].ped= new float [rtn.scope[tel].npix];
    rtn.scope[tel].exnoise= new float [rtn.scope[tel].npix];
    rtn.scope[tel].pixinuse= new int [rtn.scope[tel].npix];
    rtn.scope[tel].telpixarea= new float [rtn.scope[tel].npix];
    float foclen=1000*ta->scope[tel].mirror.foclength; //Focal length in mm
    //Initialize the number of live pixel in the telescope
    rtn.scope[tel].nb_live_pix=0;
    //Loop on the pixels
    for(int pix=0;pix<rtn.scope[tel].npix;pix++) {
      //Pixel coordinates
      rtn.scope[tel].xcam[pix]=ta->scope[tel].camera.pmt[pix].x*
	FROGS_DEG_PER_RAD/foclen;;
      rtn.scope[tel].ycam[pix]=ta->scope[tel].camera.pmt[pix].y*
	FROGS_DEG_PER_RAD/foclen;;
      //Excess noise
      rtn.scope[tel].exnoise[pix]=ta->scope[tel].camera.pmt[pix].ampfluc;
      //Pixel dead or alive
      rtn.scope[tel].pixinuse[pix]=FROGS_OK;
      if(ta->scope[tel].camera.pmt[pix].inanalyz==0)
	rtn.scope[tel].pixinuse[pix]=FROGS_NOTOK;
      //Increment the numbe rof live pixels
      if(rtn.scope[tel].pixinuse[pix]==FROGS_OK) rtn.scope[tel].nb_live_pix++;
      //Pixel effective collecting area in square degrees
      float tmppixarea=ta->scope[tel].camera.pmt[pix].radius*
	FROGS_DEG_PER_RAD/foclen;
      tmppixarea=FROGS_PI*tmppixarea*tmppixarea;
      rtn.scope[tel].telpixarea[pix]=telarea*tmppixarea*
	ta->scope[tel].camera.pmt[pix].coll_eff;
      float dc2pe=5.5;  //This is the number of d.c. in one p.e.
      //Initialize the pixel signal and pedestal width to zero
      rtn.scope[tel].q[pix]=0;
      rtn.scope[tel].ped[pix]=0;
      //Set them to their values in p.e. if the d.c./p.e. factor is non zero
      if(dc2pe!=0) {
	rtn.scope[tel].q[pix]=taevnt->img[tel][adc_type].nbpe[pix]/dc2pe;
	rtn.scope[tel].ped[pix]=taped->nbpedev[adc_type][tel][pix]/dc2pe;
      }
    }
    //Total number of live pixels in the array
    rtn.nb_live_pix_total=rtn.nb_live_pix_total+rtn.scope[tel].nb_live_pix;
  }

  //Optimization starting point
  rtn.startpt.xs=asin(taevnt->imgmodel.geodirx)*FROGS_DEG_PER_RAD;
  rtn.startpt.ys=asin(taevnt->imgmodel.geodiry)*FROGS_DEG_PER_RAD;
  rtn.startpt.xp=taevnt->imgmodel.geoptx;
  rtn.startpt.yp=taevnt->imgmodel.geopty;
  rtn.startpt.lambda=0.3; //We use a fixed value by lack of information. 

  //SLB the energy starting point evaluation should be improved. 
  //Energy starting point
  rtn.startpt.log10e=0;
  float cosz = sin(taevnt->teltrack[0].elev);
  if(rtn.ntel!=4) 
    showxerror("Estimation of starting energy value requires 4 telescopes");
  float tel_correct[rtn.ntel];
  tel_correct[0]=0.97;
  tel_correct[1]=0.95;
  tel_correct[2]=1.03;
  tel_correct[3]=1.00;  
  float Etelsum = 0;
  int nEtel=0;
  for(int tel=0; tel<ta->nbr_tel;tel++) {
    // impact parameter
    float dimp =sqrt((rtn.scope[tel].xfield-rtn.startpt.xp)*
		     (rtn.scope[tel].xfield-rtn.startpt.xp)+
		     (rtn.scope[tel].yfield-rtn.startpt.yp)*
		     (rtn.scope[tel].yfield-rtn.startpt.yp));
    // corrected sum
    float corrected_sum = taevnt->img[tel][adc_type].sum/tel_correct[tel];
    // small image correction in datareader
    if(taevnt->img[tel][adc_type].ntub<15) 
      corrected_sum *= 1+(15-taevnt->img[tel][adc_type].ntub)*0.015;  
    // summing all energies from each tel. This is Pierre's weird expression
    //We should try something simpler and cleaner. 
    Etelsum += 0.63/(cosz*cosz*cosz)*
      pow(corrected_sum/exp(0.02*cosz*cosz*(120./cosz-dimp)/
			    (1+exp(1+0.032*(120.0/cosz-dimp)))),0.86);
    nEtel++;
  }
  if(Etelsum>0)
    rtn.startpt.log10e=log10( (Etelsum/nEtel)/1000 );
  else
    rtn.startpt.log10e=FROGS_BAD_NUMBER;

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
  for(int tel=0; tel<ta->nbr_tel;tel++) 
    if(taevnt->img[tel][adc_type].sum>300.0) ngoodimages=ngoodimages+1;
  //Require the number of telescopes with more than 300dc to be at least 3
  if (ngoodimages<3) rtn.worthy_event=FROGS_NOTOK;
  return rtn;
}
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
    for(int pix=0;pix<d.scope[tel].npix;pix++) {

      fprintf(OUTUNIT,
	      "Pixel %3d %1d x=%5.2f y=%5.2f q=%f ped=%f xn=%f col=%f\n", 
	      pix+1,d.scope[tel].pixinuse[pix],d.scope[tel].xcam[pix],
	      d.scope[tel].ycam[pix],d.scope[tel].q[pix],
	      d.scope[tel].ped[pix],d.scope[tel].exnoise[pix],
	      d.scope[tel].telpixarea[pix]);
    }
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
  rtn=frogs_likelihood_optimization(d,&tmplt);

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
  return rtn;
}
//================================================================
//================================================================
struct frogs_imgtmplt_out
frogs_likelihood_optimization(struct frogs_imgtmplt_in *d, 
			      struct frogs_imgtemplate *tmplt) {
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
  if(frogs_goodness(&rtn,d,tmplt)!=FROGS_OK) 
    frogs_showxerror("Problem encountered in the convergence goodness calculation");
  gsl_multifit_fdfsolver_free(s);
  gsl_matrix_free(covar);
  
  return rtn;
}
//================================================================
//================================================================
int frogs_goodness(struct frogs_imgtmplt_out *tmplanlz,
		   struct frogs_imgtmplt_in *d, 
		   struct frogs_imgtemplate *tmplt) {
  /* Calculates the image and background goodness whose values and associated 
     number of pixels are passed back in tmplanlz which also contains the 
     values of the event physical parameters for which the goodness is 
     calculated. d is a pointer to a structure holding all the data from the 
     telescopes while tmplt is a pointer to a structure holding the image 
     template data */
  
  /*Initialize goodnesses and the number of pixels for both image 
    and background regions*/
  tmplanlz->goodness_img=0;
  tmplanlz->npix_img=0;
  tmplanlz->goodness_bkg=0;
  tmplanlz->npix_bkg=0;
  for(int tel=0;tel<d->ntel;tel++) {
    for(int pix=0;pix<d->scope[tel].npix;pix++) {
      if(d->scope[tel].pixinuse[pix]==FROGS_OK) {
	//Here call image model and calculate expected signal
	int pix_in_template;//FROGS_OK in image, FROGS_NOTOK in background
	double mu=frogs_img_model(pix,tel,tmplanlz->cvrgpt,d,
				  tmplt,&pix_in_template);
	if(mu!=FROGS_BAD_NUMBER) { 
	  double pd=frogs_probability_density(d->scope[tel].q[pix],mu,
					      d->scope[tel].ped[pix],
					      d->scope[tel].exnoise[pix]);
	  double mean_lkhd=frogs_mean_pix_lkhd(d->scope[tel].q[pix],mu,
					       d->scope[tel].ped[pix],
					       d->scope[tel].exnoise[pix]);
	  double pix_goodness=-2.0*log(pd)-mean_lkhd;
	  //If requested we produce a calibration output
	  if(FROGS_NBEVENT_GDNS_CALIBR>0) 
	    frogs_gdns_calibr_out(d->event_id, tel, pix, d->scope[tel].q[pix],
				  d->scope[tel].ped[pix],mu,pix_goodness);

	  /*Apply the single pixel goodness correction according to the 
	    pixel pedestal width*/
	  pix_goodness=frogs_goodness_correction(pix_goodness,
						 d->scope[tel].ped[pix]);  

	  /*Decides if the pixel should be counted in the image 
	    or background region*/
	   int pix_in_img=frogs_image_or_background(tel,pix,d);
	  //int pix_in_img=pix_in_template;


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
	  }
	  //If the pixel is in the background region
	  if(pix_in_img==FROGS_NOTOK) {
	    tmplanlz->goodness_bkg=tmplanlz->goodness_bkg+pix_goodness;
	    tmplanlz->npix_bkg++;
	  }
	}//End of background/image region test
      } //End of test on pixel viability
    }//End of pixel loop
  }//End of telescope loop

  //Finilize the background goodness calculation (*** See note)
  if(tmplanlz->npix_bkg>tmplt->ndim+1) 
    tmplanlz->goodness_bkg=tmplanlz->goodness_bkg/
      sqrt(2.0*(tmplanlz->npix_bkg-(tmplt->ndim+1))); 
  else
    tmplanlz->goodness_bkg=FROGS_BAD_NUMBER;
  //Finilize the image goodness calculation (*** See note)
  if(tmplanlz->npix_img>tmplt->ndim+1) 
    tmplanlz->goodness_img=tmplanlz->goodness_img/
      sqrt(2.0*(tmplanlz->npix_img-(tmplt->ndim+1)));
  else
    tmplanlz->goodness_img=FROGS_BAD_NUMBER;

  /* ***note on the goodness. The goodness is the difference between the 
     log-likelihood and its average value divided by the square root of two 
     times the number of degrees of freedom. Here we calculate a goodness for 
     the image and for the background and we subtract the number of optimized 
     parameter to both. This seems odd. It probably does not matter much when 
     the number of pixels in the image and in the background are large 
     compared to the number of optimized parameters.*/
    

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
			  float ped, float mu,double pix_goodness){
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
    calib=fopen("frogs_goodness_calibration.frogs","w");
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
  fprintf(calib,"%d %d %d %f %f %f %g\n",event_id,tel,pix,ped,q,mu,
	  pix_goodness);
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
float frogs_goodness_correction(float goodness0,float ped) {
  /* Applies correction to the individual pixel goodness in order to
     compensate for its sensitivity to the pedestal width. The function
     returns the single pixel corrected goodness value
     goodness0 = uncorrected goodness
     ped = pedestal width */
  if(ped<=1.0) return goodness0+0.6046;
  if(ped>1.0 && ped<=1.5) return goodness0+0.3748;
  if(ped>1.5 && ped<=2.0) return goodness0+0.3860;
  if(ped>2.0 && ped<=2.5) return goodness0+0.4126;
  if(ped>2.5 && ped<=3.0) return goodness0+0.4397;
  return goodness0+0.5502;
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
  if(mu==0.0) rtn=exp(-q*q/(2.0*ped*ped))/(FROGS_SQRTTWOPI*ped);
  
  //Case where mu is large enough the poisson distribution is gauss-like
  if(mu>=FROGS_LARGE_PE_SIGNAL) {
    double dummy1=ped*ped+mu*(1.0+exnoise*exnoise);//Variance
    double dummy2=exp(-(q-mu)*(q-mu)*0.5/dummy1);  //Gauss probability density
    rtn=dummy2/sqrt(FROGS_TWOPI*dummy1);           //Normalization
  }

  //Detailed calculation for intermediate values of mu
  if(mu>0 && mu<FROGS_LARGE_PE_SIGNAL) {
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
double frogs_mean_pix_lkhd(double q,double mu, double ped,double exnoise) {
  double rtn=0;
  /* Computes and returns the average of the log-likelihood for the 
     considered pixel for which:  
     q = actual signal in the pixel
     mu = expectation value of the signal in the pixel
     ped = pedestal width for that pixel 
     exnoise = exess noise for that pixel */
  
  //Case mu=0
  if(mu==0.0) rtn=1.0+FROGS_LNTWOPI+2.0*log(ped);
  
  //Case where mu is large enough the poisson distribution is gauss-like
  if(mu>=FROGS_LARGE_PE_SIGNAL) {
    double dummy=ped*ped+ mu*(1.0+exnoise*exnoise);
    rtn=1.0+FROGS_LNTWOPI+log(dummy);
  }
  
  //Detailed calculation for intermediate values of mu
  if(mu>0.0 && mu<FROGS_LARGE_PE_SIGNAL) {
    //We use GSL for the integration calculating the average
    gsl_integration_workspace * w = gsl_integration_workspace_alloc(3000);
    double error;
    gsl_function F;
    F.function = &frogs_integrand_for_averaging; //Integrand function
    struct frogs_gsl_func_param par; //Parameters to be passed to the function
    par.ped = ped;
    par.exnoise = exnoise;
    par.mu = mu;
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
  
  double proba_density=frogs_probability_density(q, p->mu,p->ped, p->exnoise);
  double loglikelihood=-2.0*log( proba_density );
  
  if(!frogs_is_a_good_number(loglikelihood * proba_density)) 
    frogs_showxerror("NaN resulted from calculations in frogs_integrand_for_averaging"); 
  //Returns the log-likelihood multiplied by the probability it is achieved
  return loglikelihood * proba_density; 
}
//================================================================
//================================================================
struct frogs_imgtemplate frogs_read_template_elev(float elevation) {
  /* Read the template in a file for which the elevation provided in 
     argument is a good match. The template data is returned. */
  struct frogs_imgtemplate rtn;
  //Above 65 deg we use the 70 deg template for now
  if(elevation>=65.0) {
    char template_file_name[FROGS_FILE_NAME_MAX_LENGTH];
    strcpy(template_file_name,"./Templates/SimulatedModels/elev70X0.0-4.0s0.5r085.tmplt"); 
    rtn=frogs_read_template_file(template_file_name);
    //The template file elevation is not stored in the file and we set it here.
    rtn.elevation=70.0;
    return rtn;
  } 
  /*If the elevation matches that corresponding to a template file, we 
    read that file*/
  if(fabs(elevation-60.0)<=5.0) {
    char template_file_name[FROGS_FILE_NAME_MAX_LENGTH];
    strcpy(template_file_name,"./Templates/SimulatedModels/elev60X0.0-4.0s0.5r085.tmplt"); 
    rtn=frogs_read_template_file(template_file_name);
    //The template file elevation is not stored in the file and we set it here.
    rtn.elevation=60.0;
    return rtn;
  } 
  if(fabs(elevation-50.0)<=5.0) {
    char template_file_name[FROGS_FILE_NAME_MAX_LENGTH];
    strcpy(template_file_name,"./Templates/SimulatedModels/elev50X0.0-4.0s0.5r085.tmplt"); 
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
	  double pd=frogs_probability_density(dwrap->data->scope[tel].q[pix],
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
  delta.xs=0.01;
  delta.ys=0.01;
  delta.xp=2.0;
  delta.yp=2.0;
  delta.log10e=0.02;
  delta.lambda=0.2;
  
  int gsl_pix_id=0; //This counter is used as a pixel identified for gsl
  for(int tel=0;tel<dwrap->data->ntel;tel++) {
    for(int pix=0;pix<dwrap->data->scope[tel].npix;pix++) {
      if(dwrap->data->scope[tel].pixinuse[pix]==FROGS_OK) {
	double derivative;
	//Derivative with respect to xs
	derivative=frogs_pix_lkhd_deriv_2ndorder(pix,tel,pnt,delta,dwrap->data,
				  dwrap->tmplt,FROGS_XS);
	gsl_matrix_set(J,gsl_pix_id,FROGS_XS,derivative);
	//Derivative with respect to ys
	derivative=frogs_pix_lkhd_deriv_2ndorder(pix,tel,pnt,delta,dwrap->data,
				  dwrap->tmplt,FROGS_YS);
	gsl_matrix_set(J,gsl_pix_id,FROGS_YS,derivative);
	//Derivative with respect to xp
	derivative=frogs_pix_lkhd_deriv_2ndorder(pix,tel,pnt,delta,dwrap->data,
				  dwrap->tmplt,FROGS_XP);
	gsl_matrix_set(J,gsl_pix_id,FROGS_XP,derivative);
	//Derivative with respect to yp
	derivative=frogs_pix_lkhd_deriv_2ndorder(pix,tel,pnt,delta,dwrap->data,
				  dwrap->tmplt,FROGS_YP);
	gsl_matrix_set(J,gsl_pix_id,FROGS_YP,derivative);
	//Derivative with respect to log10e
	derivative=frogs_pix_lkhd_deriv_2ndorder(pix,tel,pnt,delta,dwrap->data,
				  dwrap->tmplt,FROGS_LOG10E);
	gsl_matrix_set(J,gsl_pix_id,FROGS_LOG10E,derivative);
	//Derivative with respect to lambda
	derivative=frogs_pix_lkhd_deriv_2ndorder(pix,tel,pnt,delta,dwrap->data,
				  dwrap->tmplt,FROGS_LAMBDA);
	gsl_matrix_set(J,gsl_pix_id,FROGS_LAMBDA,derivative);

	gsl_pix_id++;
      }
    }
  }

  return GSL_SUCCESS;
}
//================================================================
//================================================================
double frogs_pix_lkhd_deriv_2ndorder(int pix, int tel,
				     struct frogs_reconstruction pnt, 
				     struct frogs_reconstruction delta,
				     struct frogs_imgtmplt_in *d,
				     struct frogs_imgtemplate *tmplt,
				     int gsl_par_id) {
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

  //Evaluate the derivative
  float delta_param=0;
  if(gsl_par_id==FROGS_XS) delta_param=delta.xs;
  if(gsl_par_id==FROGS_YS) delta_param=delta.ys;
  if(gsl_par_id==FROGS_XP) delta_param=delta.xp;
  if(gsl_par_id==FROGS_YP) delta_param=delta.yp;
  if(gsl_par_id==FROGS_LOG10E) delta_param=delta.log10e;
  if(gsl_par_id==FROGS_LAMBDA) delta_param=delta.lambda;
  if(delta_param==0) 
    frogs_showxerror("Bad parameter identifier in pix_lkhd_deriv");  
  rtn=(pix_lkhd_plus-pix_lkhd_minus)/(2.0*delta_param);
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
int frogs_likelihood_derivative_old(const gsl_vector *v, void *ptr, 
				    gsl_matrix *J) {
  //SLB: is it time I delete this function? 
  /* Calculates the likelihood derivative for each pixel and with respect 
    to each event physical parameter and store the result in the gsl 
    matrix J. The event physical parameter for which the likelihood is 
    calculated are provided in the gsl vector v. All the data from the 
    telescope and from the template are available through the pointer to 
    void ptr */

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
  delta.xs=0.01;
  delta.ys=0.01;
  delta.xp=2.0;
  delta.yp=2.0;
  delta.log10e=0.02;
  delta.lambda=0.2;
  
  int gsl_pix_id=0; //This counter is used as a pixel identified for gsl
  for(int tel=0;tel<dwrap->data->ntel;tel++) {
    for(int pix=0;pix<dwrap->data->scope[tel].npix;pix++) {
      if(dwrap->data->scope[tel].pixinuse[pix]==FROGS_OK) {
	double mu;
	double pd;
	double pix_lkhd_plus,pix_lkhd_minus,derivative;
	int pix_in_template;//FROGS_OK in image, FROGS_NOTOK in background
	//Derivative with respect to xs
	pnt.xs=pnt.xs+delta.xs;	//One step forward
	mu=frogs_img_model(pix,tel,pnt,dwrap->data,dwrap->tmplt,&pix_in_template);
	if(mu==FROGS_BAD_NUMBER) 
	  frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
	pd=frogs_probability_density(dwrap->data->scope[tel].q[pix],mu,
				     dwrap->data->scope[tel].ped[pix],
				     dwrap->data->scope[tel].exnoise[pix]);
	pix_lkhd_plus=-2.0*log(pd);
	pnt.xs=pnt.xs-2.0*delta.xs; //One step back	
	mu=frogs_img_model(pix,tel,pnt,dwrap->data,dwrap->tmplt,&pix_in_template);
	if(mu==FROGS_BAD_NUMBER) 
	  frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
	pd=frogs_probability_density(dwrap->data->scope[tel].q[pix],mu,
				     dwrap->data->scope[tel].ped[pix],
				     dwrap->data->scope[tel].exnoise[pix]);
	pix_lkhd_minus=-2.0*log(pd);
	derivative=(pix_lkhd_plus-pix_lkhd_minus)/(2.0*delta.xs);
	gsl_matrix_set(J,gsl_pix_id,FROGS_XS,derivative);
	pnt.xs=pnt.xs+delta.xs;	//Get back to where we are
	
	//Derivative with respect to ys
	pnt.ys=pnt.ys+delta.ys;	//One step forward
	mu=frogs_img_model(pix,tel,pnt,dwrap->data,dwrap->tmplt,&pix_in_template);
	if(mu==FROGS_BAD_NUMBER) 
	  frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
	pd=frogs_probability_density(dwrap->data->scope[tel].q[pix],mu,
				     dwrap->data->scope[tel].ped[pix],
				     dwrap->data->scope[tel].exnoise[pix]);
	pix_lkhd_plus=-2.0*log(pd);
	pnt.ys=pnt.ys-2.0*delta.ys; //One step back
	mu=frogs_img_model(pix,tel,pnt,dwrap->data,dwrap->tmplt,&pix_in_template);
	if(mu==FROGS_BAD_NUMBER) 
	  frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
	pd=frogs_probability_density(dwrap->data->scope[tel].q[pix],mu,
				     dwrap->data->scope[tel].ped[pix],
				     dwrap->data->scope[tel].exnoise[pix]);
	pix_lkhd_minus=-2.0*log(pd);
	derivative=(pix_lkhd_plus-pix_lkhd_minus)/(2.0*delta.ys);
	gsl_matrix_set(J,gsl_pix_id,FROGS_YS,derivative);
	pnt.ys=pnt.ys+delta.ys;	//Get back to where we are
	
	//Derivative with respect to xp
	pnt.xp=pnt.xp+delta.xp;	//One step forward
	mu=frogs_img_model(pix,tel,pnt,dwrap->data,dwrap->tmplt,&pix_in_template);
	if(mu==FROGS_BAD_NUMBER) 
	  frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
	pd=frogs_probability_density(dwrap->data->scope[tel].q[pix],mu,
				     dwrap->data->scope[tel].ped[pix],
				     dwrap->data->scope[tel].exnoise[pix]);
	pix_lkhd_plus=-2.0*log(pd);
	pnt.xp=pnt.xp-2.0*delta.xp; //One step back
	mu=frogs_img_model(pix,tel,pnt,dwrap->data,dwrap->tmplt,&pix_in_template);
	if(mu==FROGS_BAD_NUMBER) 
	  frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
	pd=frogs_probability_density(dwrap->data->scope[tel].q[pix],mu,
				     dwrap->data->scope[tel].ped[pix],
				     dwrap->data->scope[tel].exnoise[pix]);
	pix_lkhd_minus=-2.0*log(pd);
	derivative=(pix_lkhd_plus-pix_lkhd_minus)/(2.0*delta.xp);
	gsl_matrix_set(J,gsl_pix_id,FROGS_XP,derivative);
	pnt.xp=pnt.xp+delta.xp;	//Get back to where we are
	
	//Derivative with respect to yp
	pnt.yp=pnt.yp+delta.yp;	//One step forward
	mu=frogs_img_model(pix,tel,pnt,dwrap->data,dwrap->tmplt,&pix_in_template);
	if(mu==FROGS_BAD_NUMBER) 
	  frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
	pd=frogs_probability_density(dwrap->data->scope[tel].q[pix],mu,
				     dwrap->data->scope[tel].ped[pix],
				     dwrap->data->scope[tel].exnoise[pix]);
	pix_lkhd_plus=-2.0*log(pd);
	pnt.yp=pnt.yp-2.0*delta.yp; //One step back
	mu=frogs_img_model(pix,tel,pnt,dwrap->data,dwrap->tmplt,&pix_in_template);
	if(mu==FROGS_BAD_NUMBER) 
	  frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
	pd=frogs_probability_density(dwrap->data->scope[tel].q[pix],mu,
				     dwrap->data->scope[tel].ped[pix],
				     dwrap->data->scope[tel].exnoise[pix]);
	pix_lkhd_minus=-2.0*log(pd);
	derivative=(pix_lkhd_plus-pix_lkhd_minus)/(2.0*delta.yp);
	gsl_matrix_set(J,gsl_pix_id,FROGS_YP,derivative);
	pnt.yp=pnt.yp+delta.yp;	//Get back to where we are
	
	//Derivative with respect to log10e
	pnt.log10e=pnt.log10e+delta.log10e;	//One step forward
	mu=frogs_img_model(pix,tel,pnt,dwrap->data,dwrap->tmplt,&pix_in_template);
	if(mu==FROGS_BAD_NUMBER) 
	  frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
	pd=frogs_probability_density(dwrap->data->scope[tel].q[pix],mu,
				     dwrap->data->scope[tel].ped[pix],
				     dwrap->data->scope[tel].exnoise[pix]);
	pix_lkhd_plus=-2.0*log(pd);
	pnt.log10e=pnt.log10e-2.0*delta.log10e; //One step back
	mu=frogs_img_model(pix,tel,pnt,dwrap->data,dwrap->tmplt,&pix_in_template);
	if(mu==FROGS_BAD_NUMBER) 
	  frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
	pd=frogs_probability_density(dwrap->data->scope[tel].q[pix],mu,
				     dwrap->data->scope[tel].ped[pix],
				     dwrap->data->scope[tel].exnoise[pix]);
	pix_lkhd_minus=-2.0*log(pd);
	derivative=(pix_lkhd_plus-pix_lkhd_minus)/(2.0*delta.log10e);
	gsl_matrix_set(J,gsl_pix_id,FROGS_LOG10E,derivative);
	pnt.log10e=pnt.log10e+delta.log10e;	//Get back to where we are
	
	//Derivative with respect to lambda
	pnt.lambda=pnt.lambda+delta.lambda;	//One step forward
	mu=frogs_img_model(pix,tel,pnt,dwrap->data,dwrap->tmplt,&pix_in_template);
	if(mu==FROGS_BAD_NUMBER) 
	  frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
	pd=frogs_probability_density(dwrap->data->scope[tel].q[pix],mu,
				     dwrap->data->scope[tel].ped[pix],
				     dwrap->data->scope[tel].exnoise[pix]);
	pix_lkhd_plus=-2.0*log(pd);
	pnt.lambda=pnt.lambda-2.0*delta.lambda; //One step back
	mu=frogs_img_model(pix,tel,pnt,dwrap->data,dwrap->tmplt,&pix_in_template);
	if(mu==FROGS_BAD_NUMBER) 
	  frogs_showxerror("frogs_img_model() invoked for an invalid pixel");
	pd=frogs_probability_density(dwrap->data->scope[tel].q[pix],mu,
				     dwrap->data->scope[tel].ped[pix],
				     dwrap->data->scope[tel].exnoise[pix]);
	pix_lkhd_minus=-2.0*log(pd);
	derivative=(pix_lkhd_plus-pix_lkhd_minus)/(2.0*delta.lambda);
	gsl_matrix_set(J,gsl_pix_id,FROGS_LAMBDA,derivative);
	pnt.lambda=pnt.lambda+delta.lambda;	//Get back to where we are
	
	gsl_pix_id++;
      }
    }
  }
  
  return GSL_SUCCESS;
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
  float yrs=d->scope[tel].ycam[pix]-pnt.ys;
  //Apply a rotation to move to the template coordinate system
  float tmpltxpix=xrs*cphi+yrs*sphi; 
  float tmpltypix=-xrs*sphi+yrs*cphi;

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
  il1=(int)floor(dummy); il2=il1+1;
  if(fabs(dummy-il1)>fabs(dummy-il2)) il3=il2+1; 
  else {il2=il1;il1=il2-1;il3=il2+1;}
  if(il1<0) {il1=0;il2=1;il3=2;} 
  if(il3>=tmplt->nstep[0]) {il3=tmplt->nstep[0]-1;il2=il3-1;il1=il2-1;}
  float l1=tmplt->min[0]+il1*tmplt->step[0];
  float l2=tmplt->min[0]+il2*tmplt->step[0];
  float l3=tmplt->min[0]+il3*tmplt->step[0];

  //For the energy as well we will use a quadratic interpolation as well. 
  int iloge1,iloge2,iloge3;
  dummy=(log10e-tmplt->min[1])/tmplt->step[1];
  iloge1=(int)floor(dummy); iloge2=iloge1+1;
  if(fabs(dummy-iloge1)>fabs(dummy-iloge2)) iloge3=iloge2+1; 
  else {iloge2=iloge1;iloge1=iloge2-1;iloge3=iloge2+1;}
  if(iloge1<0) {iloge1=0;iloge2=1;iloge3=2;} 
  if(iloge3>=tmplt->nstep[1]) {iloge3=tmplt->nstep[1]-1;iloge2=iloge3-1;iloge1=iloge2-1;}
  float loge1=tmplt->min[1]+iloge1*tmplt->step[1];
  float loge2=tmplt->min[1]+iloge2*tmplt->step[1];
  float loge3=tmplt->min[1]+iloge3*tmplt->step[1];

  //For the impact parameter we will use a quadratic interpolation as well
  int ib1,ib2,ib3;
  dummy=(b-tmplt->min[2])/tmplt->step[2];
  ib1=(int)floor(dummy);ib2=ib1+1;
  if(fabs(dummy-ib1)>fabs(dummy-ib2)) ib3=ib2+1; 
  else {ib2=ib1;ib1=ib2-1;ib3=ib2+1;}
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
  fprintf(stderr,"xs=%f +/- %f degrees\n",output.cvrgpt.lambda,
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
  fprintf(stderr," ---------------------------------------------------------------- \n");
  return FROGS_OK;
}
//================================================================
//================================================================
