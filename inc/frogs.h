#ifndef FROGS_H_INC
#define FROGS_H_INC

/*================================================================*\
|      _    _                                        _    _        |
|     (o)--(o)                                      (o)--(o)       |
|    /.______.\    FFFF RRRR   OOO   GGG   SSS     /.______.\      |
|    \________/    F    R   R O   O G     S        \________/      |
|  ./        \.    FFF  RRRR  O   O G  GG  SSS     ./        \.    |
| ( .        , )   F    R R   O   O G   G     S   ( .        , )   |
|  \ \_\\//_/ /    F    R  RR  OOO   GGG  SSSS     \ \_\\//_/ /    |
|   ~~  ~~  ~~                                      ~~  ~~  ~~     |
| svincent@physics.utah.edu             lebohec@physics.utah.edu   |
|                  VERSION 1.02 OCTOBER 10th 2011                  |
|  For license issues, see www.physics.utah.edu/gammaray/FROGS     |
\*================================================================*/

#include <stdio.h>
#include <string.h>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlin.h> //Levenberg-Marquardt 
#include <gsl/gsl_blas.h> //Levenberg-Marquardt linear algebra
#include <gsl/gsl_integration.h>
#include <gsl/gsl_interp.h>
//================================================================
//================================================================
/*This is the name of the file holding the list of template file names 
  and their range of applicatbility:*/
#define FROGS_TEMPLATE_LIST "./frogs_template_file_list.txt"
#define FROGS_DEG_PER_RAD 57.295779513082325  //Number of degrees in one radian
#define FROGS_FILE_NAME_MAX_LENGTH 100 //Maximum file name length
#define FROGS_MAX_ITER_NBR 100 //Maximum number of iterations
#define FROGS_OK 1
#define FROGS_NOTOK 0
#define FROGS_BAD_NUMBER -9999  //Used to signal a calculation was not possible
//#define FROGS_isNaN(x) ((x) != (x))
#define FROGS_SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr
#define FROGS_NONEG(x) 0.5*(x+fabs(x))
#define FROGS_SMALL_NUMBER 1E-85        /*Used to protect the case when the 
				    likilhood is found equal to zero which 
				    would result in a divergence. */
#define FROGS_PI 3.141592653589793      //PI
#define FROGS_TWOPI 6.283185307179586   //2.0*PI
#define FROGS_SQRTTWOPI 2.506628274631  //sqrt(2.0*PI)
#define FROGS_LNTWOPI 1.837877066409345 //log(2.0*PI)
#define FROGS_LARGE_PE_SIGNAL 50.0      /*Limit beyond which Poisson can be 
				    replaced by Gauss statistics*/
#define FROGS_NUMBER_OF_SIGMA 10.0      /*Number of standard deviations to be 
				    explored around a signal. 15 is 
				    certainly too large*/
#define FROGS_INTERP_ORDER 2    /*Interpolation order should be 
				  set to 1 (linear) or 2 (quadratic)*/
#define FROGS_NBEVENT_GDNS_CALIBR 0 /*Number of events used to build a goodness 
				  calibration file. Must be set to a positive 
				  number to activate the good ness 
				  calibration output. */
#define FROGS_NBEVENT_DISPLAY 0 /*Number of eents used to build a goodness 
				  calibration file. Must be set to a positive 
				  number to activate the good ness 
				  calibration output. */
#define FROGS_XS 0     //These tags are used to specify the parameter with 
#define FROGS_YS 1     //respect to which the derivative of the likelihood is 
#define FROGS_XP 2     //calculated. Because of GLS, the Ttags should have 
#define FROGS_YP 3     //values ranging from 0 to the number of parameter 
#define FROGS_LOG10E 4 //minus one so 5 in our case. 
#define FROGS_LAMBDA 5

#define FROGS_HITHRESH 4.5   //Higher threshold to define the picture
#define FROGS_LOTHRESH 2.5   //Lower threshold to define the picture. 
#define FROGS_PICTRAD  0.35  //Radius defining the picture
#define FROGS_NEIGHBORAD 0.16 //Pixels separated by less than that are neighbors

#ifdef __cplusplus
extern "C" {
#endif 
//================================================================
//================================================================
struct frogs_reconstruction {
  //Holds the event parameters as we would like to reconstruct them:
  float xs;  //Source position x corrdinate (degrees)
  float ys;  //Source position y corrdinate (degrees)
  float xp;  /*x coordinate of the specular impact point in a plane 
	       perpendicaula to the direction tracked*/
  float yp;  /*y coordinate of the specular impact point in a plane 
	       perpendicaula to the direction tracked*/
  float log10e; //Log10(energy/1TeV)
  float lambda; //Depth of the 1st interaction point in radiation length
};
//----------------------------------------------------------------
struct frogs_imgtmplt_in {
  //Holds all the data needed for the template analysis
  int event_id; //Event identification number
  float elevation;  //Elevation in degrees
  int nb_live_pix_total; //Number of used pixel in the array
  struct frogs_reconstruction startpt; //Starting point for the optimization
  int ntel;  //Number of telescopes
  struct frogs_telescope *scope;
  int worthy_event; //FROGS_OK if event is to be analysed FROGS_NOTOK otherwise
};
//----------------------------------------------------------------
struct frogs_telescope {
  //Holds the data for each telescope
  float xfield; //Telescope position X coordinate in meters
  float yfield; //Telescope position Y coordinate in meters
    /*Note: Z points to the tracked direction. X perpendicular to Z and 
    horizontal. X oriented to the East when the telescope are pointing south. 
    Y perpendicular to X and Z and such that (X,Y,Z) be direct*/
  int npix; //Number of pixels
  int nb_live_pix;  //Number of pixels in use
  float *xcam; //Pixel camera x position in degrees
  float *ycam; //Pixel camera y position in degrees
  /*ycam increases toward zenith. xcam increases to the west when the 
    telescopes are pointing south*/
  float *telpixarea; //telescope area (m^2) times pixel area (square degrees)
  float *q;    //Pixel signal in photo-electrons
  float *ped;  //Pixel pedestal std dev in photo-electrons
  float *exnoise;  //Pixel excess noise
  int *pixinuse;  //Pixel status FROGS_OK or FROGS_NOTOK (to be used or not used)
};

//----------------------------------------------------------------
struct frogs_imgtmplt_out {
  //Holds the results of the image template analysis
  struct frogs_reconstruction cvrgpt;  //Convergence point in parameter space
  struct frogs_reconstruction cvrgpterr; //Error on convergence point
  float goodness_img;//Telescope array image goodness
  int npix_img; //Number of pixels in images
  float goodness_bkg;//Telescope array background goodness
  int npix_bkg; //Number of pixels in background
  int nb_iter;  //Number of iterations in the optimization
  int gsl_convergence_status; //Status of the GSL convergence
  int event_id; //Event identification number
};
//----------------------------------------------------------------
struct frogs_imgtemplate {
  //Holds the template model
  float elevation; //Elevation of the template data in degrees
  float elevmin;   //Minimal elevation of the template
  float elevmax;   //Maximal elevation of the template
  int ndim;        //Number of dimension for the parameter space
  float *step;     //Table step size for each parameter
  float *min;      //Table starting value for each parameter
  int *nstep;      //Number of steps for each parameter
  int sz;          //Size of the table
  float *c;        //Cherenkov light density /m^2/deg^2
};
//----------------------------------------------------------------
struct frogs_gsl_data_wrapper {
  /*Regroups all the data to be passed under a void pointer to 
    functions invoked by GSL */
  struct frogs_imgtemplate *tmplt;  //Pointer to the current template
  struct frogs_imgtmplt_in *data;     //Pointer to the structure containing the data
};
//----------------------------------------------------------------
struct frogs_gsl_func_param {
  /*Holds data to be passed to the function integrated by GSL to estimate 
    the average log-likelihood*/
  double ped;     //Pedestal with of a given pixel
  double exnoise; //Excess noise of a given pixel
  double mu;  //Expectation valus for a given pixel
};
//================================================================
//================================================================
struct frogs_imgtmplt_out frogs_img_tmplt(struct frogs_imgtmplt_in *d);
struct frogs_imgtmplt_in frogs_convert_from_grisu(struct array_event *taevnt,
						  struct array *ta,
						  int adc_type,
						  struct array_ped *taped);
struct frogs_imgtemplate frogs_read_template_elev(float elevation);
struct frogs_imgtemplate frogs_read_template_file(char fname[FROGS_FILE_NAME_MAX_LENGTH]); 
struct frogs_imgtmplt_out frogs_likelihood_optimization(struct frogs_imgtmplt_in *d, 
					    struct frogs_imgtemplate *tmplt);
struct frogs_imgtmplt_out frogs_null_imgtmplt_out();
int frogs_likelihood(const gsl_vector *v, void *ptr, gsl_vector *f);
int frogs_likelihood_derivative(const gsl_vector *v, void *ptr, gsl_matrix *J);
int frogs_likelihood_fdf(const gsl_vector *v, void *ptr, gsl_vector *f, 
		   gsl_matrix *J);
int frogs_goodness(struct frogs_imgtmplt_out *tmplanlz,struct frogs_imgtmplt_in *d, 
	     struct frogs_imgtemplate *tmplt);

float frogs_goodness_correction(float goodness0,float ped,float mu);
double frogs_probability_density(float q,double mu,float ped,float exnoise);
double frogs_mean_pix_lkhd(double q, double mu, double ped, double exnoise);
double frogs_poisson_distribution(double mu, long int n);
double frogs_logarithm_factorial(long int n);
double frogs_integrand_for_averaging(double q, void *par);
double frogs_img_model(int pix,int tel,struct frogs_reconstruction pnt,
		       struct frogs_imgtmplt_in *d,
		       struct frogs_imgtemplate *tmplt,int *intemplate);
void frogs_showxerror(const char *msg);
int frogs_print_param_spc_point(struct frogs_imgtmplt_out output);
double frogs_linear_interpolation(float x1,float x2,double y1,
				  double y2,float x);
double frogs_quadratic_interpolation(float x1,float x2,float x3,
			      double y1,double y2,double y3,float x);
double frogs_chertemplate_lin(float lambda,float log10e,float b,
			      float x,float y,struct frogs_imgtemplate *tmplt,
			      int *intemplate);
double frogs_chertemplate_quad(float lambda,float log10e,float b,
			       float x,float y,
			struct frogs_imgtemplate *tmplt,int *intemplate);
double frogs_get_tmplt_val(int il,int iloge,int ib,int ix,int iy,
		  struct frogs_imgtemplate *tmplt);
int frogs_printfrog(void);
int frogs_print_raw_event(struct frogs_imgtmplt_in d);
int frogs_release_memory(struct frogs_imgtmplt_in *d);
int frogs_is_a_good_number(double x);
double frogs_pix_lkhd_deriv_2ndorder(int pix, int tel,
				     struct frogs_reconstruction pnt, 
				     struct frogs_reconstruction delta,
				     struct frogs_imgtmplt_in *d,
				     struct frogs_imgtemplate *tmplt,
				     int gsl_par_id);
double frogs_pix_lkhd_deriv_4thorder(int pix, int tel,
				     struct frogs_reconstruction pnt, 
				     struct frogs_reconstruction delta,
				     struct frogs_imgtmplt_in *d,
				     struct frogs_imgtemplate *tmplt,
				     int gsl_par_id);
struct frogs_reconstruction frogs_param_step(struct frogs_reconstruction pnt,
					     struct frogs_reconstruction delta,
					     int gsl_par_id,float mult);
int frogs_gdns_calibr_out(int event_id, int tel, int pix, float q,
			  float ped, float mu,double pix_goodness);
int frogs_event_display(int event_id, float q,float mu,float xtel,
			float ytel,float xpix,float ypix,int pix_in_img);
int frogs_image_or_background(int tel,int pix,struct frogs_imgtmplt_in *d);
float floatwrap(float x,float min,float max);
#ifdef __cplusplus
}
#endif 
#endif
