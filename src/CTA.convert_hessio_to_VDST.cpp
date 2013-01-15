/** @file CTA.convert_hessio_to_VDST
 *  @short a program to convert sim_telarray files (hessio) to EVNDISP DST format
 *
 *
 *  
 *  As a skeleton for programs reading H.E.S.S. data in eventio format,
 *  this program reads the whole range of hessio item types into a
 *  single tree of data structures but normally does nothing with the data.
 *
 *  It can be instructed, though, to create nice camera images similar to
 *  those generated in sim_hessarray.
 *
 *
@verbatim
Syntax: CTA.convert_hessio_to_VDST [ options ] [ - | input_fname ... ]
Options:
   --max-events n  (Skip remaining data after so many triggered events.)
@endverbatim
 *
 *  @author  Konrad Bernloehr
 *  Modified by Gernot Maier (DESY) (EVNDISPLAY related code)
 *
 *  Revision $Id: CTA.convert_hessio_to_VDST.cc,v 1.1.2.1.2.2.2.2 2011/02/11 22:51:03 gmaier Exp $
 */


#include "io_basic.h"     /* This file includes others as required. */
#include "history.h"
#include "io_hess.h"
#include "fileopen.h"

#include <bitset>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <string>
#include <vector>

#include <TFile.h>
#include <TMath.h>
#include <TTree.h>
#include <TStopwatch.h>

#include "VGlobalRunParameter.h"
#include "VEvndispRunParameter.h"
#include "VDSTTree.h"
#include "VMonteCarloRunHeader.h"
#include "VSkyCoordinatesUtilities.h"

///////////////////////////////////////////////////////
// maximum number of pixels for the current array configuration
unsigned int fGlobalMaxNumberofPixels = 0;
// maximum number of FADC samples for the current array configuration
unsigned int fGlobalMaxNumberofSamples = 0;
// maximum number of telescopes in previous event
unsigned int fGlobalNTelPreviousEvent = 0;
// set if this event does not pass the minimum number of telescope condition
bool fGlobalTriggerReset = false;
///////////////////////////////////////////////////////

/*!
      hard wired timing configuration:

      1     	1
      0.2	2
      0.5	2
      0.8	2
      0.5	4
      0.2	4
      -40	5
*/

vector< float > getPulseTimingLevels()
{
   vector< float > t;
   t.push_back( 0.2 );
   t.push_back( 0.5 );
   t.push_back( 1.0 );
   t.push_back( 0.5 );
   t.push_back( 0.2 );

   return t;
}


unsigned int getTimingLevelIndex( unsigned int i )
{
    if( i == 0 )      return 2;        // 100%
    else if( i == 1 ) return 0;        //  20%
    else if( i == 2 ) return 1;        //  50%
    else if( i == 4 ) return 3;        //  50%
    else if( i == 5 ) return 4;        //  20%

    return 9999;
}


/** The factor needed to transform from mean p.e. units to units of the single-p.e. peak:
    Depends on the collection efficiency, the asymmetry of the single p.e. amplitude 
    distribution and the electronic noise added to the signals. */
#define CALIB_SCALE 0.92

/*
 *  FLAG_AMP_TMP       0: Use normal integrated amplitude.
 *                     1: Use integration around global peak position from
 *                        pulse shape analysis. May include all pixels or only selected.
 *                     2: Use integration around local peak position from
 *                        pulse shape analysis. Return 0 for pixels without
 *                        a fairly significant peak.
 */
#define FLAG_AMP_TMP 0

/* ---------------------- calibrate_pixel_amplitude ----------------------- */

/** Calibrate a single pixel amplitude, for cameras with two gains per pixel.
 *
 * @return Pixel amplitude in peak p.e. units.
 */

double calibrate_pixel_amplitude(AllHessData *hsdata, int itel, int ipix, int dummy, bool& iLowGain );

double calibrate_pixel_amplitude(AllHessData *hsdata, int itel, int ipix, int dummy, bool& iLowGain )
{
   int i = ipix, npix, significant, hg_known, lg_known;
   double npe, sig_hg, npe_hg, sig_lg, npe_lg;
   AdcData *raw;

   if ( hsdata == NULL || itel < 0 || itel >= H_MAX_TEL )
      return 0.;
   npix = hsdata->camera_set[itel].num_pixels;
   if ( ipix < 0 || ipix >= npix )
      return 0.;
   raw = hsdata->event.teldata[itel].raw;
   if ( raw == NULL )
      return 0.;
   if ( ! raw->known )
      return 0.;

 
   significant = hsdata->event.teldata[itel].raw->significant[i];

   hg_known = hsdata->event.teldata[itel].raw->adc_known[HI_GAIN][i];
   sig_hg = hg_known ? (hsdata->event.teldata[itel].raw->adc_sum[HI_GAIN][i] -
        hsdata->tel_moni[itel].pedestal[HI_GAIN][i]) : 0.;
   npe_hg = sig_hg * hsdata->tel_lascal[itel].calib[HI_GAIN][i];

   lg_known = hsdata->event.teldata[itel].raw->adc_known[LO_GAIN][i];
   sig_lg = lg_known ? (hsdata->event.teldata[itel].raw->adc_sum[LO_GAIN][i] -
        hsdata->tel_moni[itel].pedestal[LO_GAIN][i]) : 0.;
   npe_lg = sig_lg * hsdata->tel_lascal[itel].calib[LO_GAIN][i];

   iLowGain = false;
   if ( !significant ) 
      npe = 0.;
   else if ( hg_known && sig_hg < 1000000 && sig_hg > -1000 )
      npe = npe_hg;
   else
   {
      npe = npe_lg;
      iLowGain = true;
   }

   /* npe is in units of 'mean photo-electrons'. */
   /* We convert to experimentalist's */
   /* 'peak photo-electrons' now. */
   return CALIB_SCALE * npe;
}

#include <signal.h>

void stop_signal_function (int isig);

static int interrupted;

/* ---------------------- stop_signal_function -------------------- */
/**
 *  Stop the program gracefully when it catches an INT or TERM signal.
 *
 *  @param isig  Signal number.
 *
 *  @return (none)
 */

void stop_signal_function (int isig)
{
   if ( isig >= 0 )
   {
      fprintf(stderr,"Received signal %d\n",isig);
   }
   if ( !interrupted )
      fprintf(stderr,
      "Program stop signal. Stand by until current data block is finished.\n");

   interrupted = 1;
   
   signal(SIGINT,SIG_DFL);
   signal(SIGTERM,SIG_DFL);
}

/** Show program syntax */

static void syntax (char *program);

static void syntax (char *program)
{
   printf("Syntax: %s [ options ] [ - | input_fname ... ]\n",program);
   printf("Options:\n");
   printf("   -v               (More verbose output)\n");
   printf("   -q               (Much more quiet output)\n");
   printf("   -s               (Show data explained)\n");
   printf("   -S               (Show data explained, including raw data)\n");
   printf("   --history (-h)   (Show contents of history data block)\n");
   printf("   -i               (Ignore unknown data block types)\n");
   printf("   --max-events n   (Skip remaining data after so many triggered events.)\n");
   printf("   -a subarray file (list of telescopes to read with FOV.)\n" );
   printf("   -o dst filename  (name of dst output file)\n" );
   printf("   -f on=1/off=0    (write FADC samples to DST file;default=0)\n" );
   printf("   -r on=1/off=0    (apply camera plate scaling for DC telescopes; default=1)\n" );
   printf("   -d <nbits dyn.>  (dynamic range of readout (e.g. 12 for 12 bit. Switch to low gain)\n" );      

   exit(1);
}

using namespace std;

bool DST_fillMCRunheader( VMonteCarloRunHeader *f, AllHessData *hsdata )
{
// values from CORSIKA
   f->shower_prog_id = hsdata->mc_run_header.shower_prog_id;
   f->shower_prog_vers = hsdata->mc_run_header.shower_prog_vers;
   f->detector_prog_id = hsdata->mc_run_header.detector_prog_id;
   f->detector_prog_vers = (unsigned int)hsdata->mc_run_header.detector_prog_vers;
// TODO: add simulation dates from shower and detector simulation
// (need a compiler flag for backwards compatibility)
   f->obsheight = hsdata->mc_run_header.obsheight;
   f->num_showers = hsdata->mc_run_header.num_showers;
   f->num_use = hsdata->mc_run_header.num_use;
   f->core_pos_mode = hsdata->mc_run_header.core_pos_mode;
   f->core_range[0] = hsdata->mc_run_header.core_range[0];
   f->core_range[1] = hsdata->mc_run_header.core_range[1];
   f->az_range[0] = hsdata->mc_run_header.az_range[0];
   f->az_range[1] = hsdata->mc_run_header.az_range[1];
   f->alt_range[0] = hsdata->mc_run_header.alt_range[0];
   f->alt_range[1] = hsdata->mc_run_header.alt_range[1];
   f->diffuse = hsdata->mc_run_header.diffuse;
   f->viewcone[0] = hsdata->mc_run_header.viewcone[0];
   f->viewcone[1] = hsdata->mc_run_header.viewcone[1];
   f->E_range[0] = hsdata->mc_run_header.E_range[0];
   f->E_range[1] = hsdata->mc_run_header.E_range[1];
   f->spectral_index = hsdata->mc_run_header.spectral_index;
   f->B_total = hsdata->mc_run_header.B_total;
   f->B_inclination = hsdata->mc_run_header.B_inclination;
   f->B_declination = hsdata->mc_run_header.B_declination;
   f->injection_height = hsdata->mc_run_header.injection_height;
   f->fixed_int_depth = hsdata->mc_run_header.fixed_int_depth;
   f->atmosphere = hsdata->mc_run_header.atmosphere;
   f->corsika_iact_options = hsdata->mc_run_header.corsika_iact_options;
   f->corsika_low_E_model = hsdata->mc_run_header.corsika_low_E_model;
   f->corsika_high_E_model = hsdata->mc_run_header.corsika_high_E_model;
   f->corsika_bunchsize = hsdata->mc_run_header.corsika_bunchsize;
   f->corsika_wlen_min = hsdata->mc_run_header.corsika_wlen_min;
   f->corsika_wlen_max = hsdata->mc_run_header.corsika_wlen_max;
   f->corsika_low_E_detail = hsdata->mc_run_header.corsika_low_E_detail;
   f->corsika_high_E_detail = hsdata->mc_run_header.corsika_high_E_detail;

   return true;
}


bool DST_fillMCEvent( VDSTTree *fData, AllHessData *hsdata )
{
// MC
   fData->fDSTeventnumber = hsdata->mc_event.event;
   fData->fDSTrunnumber = hsdata->run_header.run;
   fData->fDSTprimary = hsdata->mc_shower.primary_id;
   fData->fDSTenergy = hsdata->mc_shower.energy;
   fData->fDSTaz = hsdata->mc_shower.azimuth * 45./atan( 1. );
   fData->fDSTze = 90. - hsdata->mc_shower.altitude * 45./atan( 1. );
// Observe: transform to VERITAS coordinate system
// x: East, y: North
   fData->fDSTxcore = -1.* hsdata->mc_event.ycore;
   fData->fDSTycore = hsdata->mc_event.xcore;
/////////////////////////////////////////////////////////////////////////////
// calculate offset in camera coordinates from telescope and MC direction
// (Note: assume that all telescope point into the same direction)
   double i_tel_el = hsdata->run_header.direction[1]*TMath::RadToDeg();
   double i_tel_az = hsdata->run_header.direction[0]*TMath::RadToDeg();
   float i_x = 0.;
   float i_y = 0.;
   float i_z = 0.;
   VSkyCoordinatesUtilities::getDifferenceInCameraCoordinates( 90.-i_tel_el, i_tel_az, fData->fDSTze, fData->fDSTaz, i_x, i_y, i_z );
   fData->fDSTTel_xoff = i_x;
   fData->fDSTTel_yoff = i_y;
/////////////////////////////////////////////////////////////////////////////

   if( fData->fMCtree ) fData->fMCtree->Fill();

   return true;
}

bool DST_fillEvent( VDSTTree *fData, AllHessData *hsdata, map< unsigned int, float > telescope_list, bool iWriteFADC, unsigned int iDynRange )
{
   if( !fData || !hsdata ) return false;

// set dynamic range for low gain flip
   float i_maxDynamicRange = 1.e99;
   if( iDynRange > 0 ) i_maxDynamicRange = TMath::Power( 2., (float)iDynRange ) *0.95;

// reset all arrays
   fData->resetDataVectors( 99999, fData->fDSTntel, fGlobalNTelPreviousEvent,
                            fGlobalMaxNumberofPixels,                            
			    getPulseTimingLevels().size(),
			    fGlobalMaxNumberofSamples,
			    fGlobalTriggerReset, true );

/////////////////////////////////////////////////////////////////////////////////////
// event data
   fData->fDSTeventnumber = hsdata->mc_event.event;
   fData->fDSTrunnumber = hsdata->run_header.run;
   fData->fDSTeventtype = 0;
   fData->fDSTgpsyear = 2010;

// ntel doesn't change from event to event, but needed by VDST
   fData->fDSTntel = (unsigned short int)hsdata->run_header.ntel;
   if( telescope_list.size() == 0 )
   {
      for( unsigned int i = 0; i < fData->fDSTntel; i++ )
      {
         telescope_list[i+1] = 0.;
      }
   }
   fData->fDSTntel = telescope_list.size();


////////////////////////////////////////////////
// trigger data
   fData->fDSTNTrig = hsdata->event.central.num_teltrg;
   unsigned int i_ntel_trig = 0;
   bitset<8*sizeof(unsigned long) > i_localTrigger;
   for( unsigned int t = 0; t < (unsigned int)hsdata->event.central.num_teltrg; t++ )
   {
      if( telescope_list.find( hsdata->event.central.teltrg_list[t] ) != telescope_list.end() )
      {
	 if( hsdata->event.central.teltrg_list[t] < (int)i_localTrigger.size() )
	 {
	    i_localTrigger.set( hsdata->event.central.teltrg_list[t]-1, true );
	 }
	 if( t < (unsigned int)hsdata->run_header.ntel )
	 {
	    fData->fDSTLTrig_list[i_ntel_trig] = hsdata->event.central.teltrg_list[t];
	    fData->fDSTLTtime[i_ntel_trig] = hsdata->event.central.teltrg_time[t];
// read L2 trigger type
#ifdef CTA_PROD2
// PROD2: bit1: majority
//        bit2: analog sum
//        bit3: digital sum
	    fData->fDSTL2TrigType[i_ntel_trig] = (unsigned short int)hsdata->event.central.teltrg_type_mask[t];
#else
            fData->fDSTL2TrigType[i_ntel_trig] = 0;
#endif
	 }
         i_ntel_trig++;
      }
   }
   fData->fDSTLTrig = i_localTrigger.to_ulong();
   fData->fDSTNTrig = i_ntel_trig;
// check array trigger condition again
   if( fData->fDSTNTrig < (unsigned int)hsdata->run_header.min_tel_trig )
   {
// set trigger reset variable (needed for efficient resetting of DST arrays)
      fGlobalTriggerReset = true;
      return true;
   }
   fGlobalTriggerReset = false;

/////////////////////////////////////////////////////////////////////////////////////
// tracking data
   unsigned int z = 0;
   for( unsigned short int i = 0; i < (unsigned short int)hsdata->run_header.ntel; i++ )
   {
      if( telescope_list.find( hsdata->event.trackdata[i].tel_id ) != telescope_list.end() )
      {
	 fData->fDSTpointAzimuth[z] = hsdata->event.trackdata[i].azimuth_raw * 45./atan(1.);
	 fData->fDSTpointElevation[z] = hsdata->event.trackdata[i].altitude_raw * 45./atan(1.);
	 z++;
      }
   }

/////////////////////////////////////////////////////////////////////////////////////
// event data
   fData->fDSTntel_data = (unsigned short int)hsdata->event.central.num_teldata;
   unsigned short int i_ntel_data = 0;
// list of telescopes with data
   for( unsigned short int i = 0; i < fData->fDSTntel_data; i++ )
   {
      if( telescope_list.find( hsdata->event.central.teldata_list[i] ) != telescope_list.end() )
      {
         fData->fDSTtel_data[i_ntel_data] = (unsigned int)hsdata->event.central.teldata_list[i];
	 unsigned int telID = fData->fDSTtel_data[i_ntel_data] - 1;
// newer hessio version needs this statement
#if CTA_SC>1
	 if( hsdata->event.teldata[telID].known == 0 ) continue;
#endif

////////////////////////////////////////////////
// get pixel (QADC) data
         fData->fDSTTelescopeZeroSupression[i_ntel_data] = (unsigned short int)hsdata->event.teldata[telID].raw->zero_sup_mode;
	 fData->fDSTnumSamples[i_ntel_data] = (unsigned short int)hsdata->event.teldata[telID].raw->num_samples;
	 if( iWriteFADC && fData->fDSTnumSamples[i_ntel_data] == 0 )
	 {
	    cout << "Error in DST_fillEvent: sample length in hessio file is null" << endl;
	    cout << "exiting..." << endl;
	    exit( 0 );
         }
// set maximum number of FADC samples (needed for efficient resetting of DST arrays)
	 if( fData->fDSTnumSamples[i_ntel_data] > fGlobalMaxNumberofSamples )
	 {
	    fGlobalMaxNumberofSamples = fData->fDSTnumSamples[i_ntel_data];
         }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// find time integration type required for DST
// (not sure if this really has to be done for every event and all telescopes)
	 for( int t = 0; t < hsdata->event.teldata[telID].pixtm->num_types; t++ )
	 {
	     if( t < (int)fData->getDSTpulsetiminglevelsN() )
	     {
	         if( getTimingLevelIndex( t ) < fData->getDSTpulsetiminglevelsN() )
		 {
		    fData->fDSTpulsetiminglevels[i_ntel_data][getTimingLevelIndex(t)] = hsdata->event.teldata[telID].pixtm->time_level[t];
                 }
             }
	 }
	 if( hsdata->camera_set[telID].num_pixels < VDST_MAXCHANNELS )
	 {
// set maximum number of pixels (needed for efficient resetting of DST arrays)
	    bool iLowGain = false;
	    if( (unsigned int)hsdata->camera_set[telID].num_pixels > fGlobalMaxNumberofPixels )
	    {
	       fGlobalMaxNumberofPixels = (unsigned int)hsdata->camera_set[telID].num_pixels;
            }
// loop over all pixel
	    for( int p = 0; p < hsdata->camera_set[telID].num_pixels; p++ )
	    {
		fData->fDSTChan[i_ntel_data][p] =  0;
		fData->fDSTdead[i_ntel_data][p] = !(hsdata->event.teldata[telID].raw->adc_known[HI_GAIN][p]);
		fData->fDSTRecord[i_ntel_data][p] = hsdata->event.teldata[telID].raw->significant[p];
		fData->fDSTsums[i_ntel_data][p] = (float)(TMath::Nint( calibrate_pixel_amplitude( hsdata, telID, p, FLAG_AMP_TMP, iLowGain ) * 100. ))/100.;
// set low gain switch
		fData->fDSTHiLo[i_ntel_data][p] = iLowGain;

// fill FADC trace
		unsigned int iTraceIsZero = 1;
   	        if( iWriteFADC )
		{
		   iTraceIsZero = 0;
		   iLowGain = false;
		   if( hsdata->event.teldata[telID].raw->adc_known[HI_GAIN][p] == 1 )
		   {
		      if( hsdata->event.teldata[telID].raw->adc_sample && hsdata->event.teldata[telID].raw->adc_sample[HI_GAIN] )
		      {
			 for( int t = 0; t < hsdata->event.teldata[telID].raw->num_samples; t++ )
			 {
			    iTraceIsZero += hsdata->event.teldata[telID].raw->adc_sample[HI_GAIN][p][t];
			    fData->fDSTtrace[i_ntel_data][t][p] = hsdata->event.teldata[telID].raw->adc_sample[HI_GAIN][p][t];
			    if( fData->fDSTtrace[i_ntel_data][t][p] > i_maxDynamicRange ) iLowGain = true;
			 }
		      }
                   }
// preli: no low-gain filled

		   if( iLowGain && hsdata->event.teldata[telID].raw->adc_known[LO_GAIN][p] == 1 )
		   {
		      if( hsdata->event.teldata[telID].raw->adc_sample && hsdata->event.teldata[telID].raw->adc_sample[LO_GAIN] )
		      {
			 for( int t = 0; t < hsdata->event.teldata[telID].raw->num_samples; t++ )
			 {
			    iTraceIsZero += hsdata->event.teldata[telID].raw->adc_sample[LO_GAIN][p][t];
			    fData->fDSTtrace[i_ntel_data][t][p] = hsdata->event.teldata[telID].raw->adc_sample[LO_GAIN][p][t];
			    fData->fDSTHiLo[i_ntel_data][p] = true;
			 }
		      }
                   } 
		} 
// check zero suppression (for FADC case only)
		if( iTraceIsZero > 0 )   fData->fDSTZeroSuppressed[i_ntel_data][p] = 0;
		else  
		{
		   fData->fDSTZeroSuppressed[i_ntel_data][p] = 1;
                }

// fill timing information
		for( int t = 0; t < hsdata->event.teldata[telID].pixtm->num_types; t++ )
		{
		   if( t < (int)fData->getDSTpulsetiminglevelsN() && getTimingLevelIndex( t ) < fData->getDSTpulsetiminglevelsN() )
		   {
		      fData->fDSTpulsetiming[i_ntel_data][getTimingLevelIndex(t)][p] = hsdata->event.teldata[telID].pixtm->timval[p][t];
                   }
                }
		fData->fDSTMax[i_ntel_data][p] = (short)(hsdata->event.teldata[telID].pixtm->pulse_sum_loc[HI_GAIN][p]);
		if( FLAG_AMP_TMP > 0 )
		{
		   fData->fDSTsumwindow[i_ntel_data][p] = hsdata->event.teldata[telID].pixtm->after_peak;
		   fData->fDSTsumwindow[i_ntel_data][p] -= hsdata->event.teldata[telID].pixtm->before_peak;
		}
		else fData->fDSTsumwindow[i_ntel_data][p] = hsdata->pixel_set[telID].sum_bins;
	    }
	 }
	 else
	 {
	    cout << "ERROR: number of pixels too high: " << telID + 1 << "\t";
	    cout << hsdata->camera_set[telID].num_pixels << "\t" << VDST_MAXCHANNELS << endl;
	    exit( 0 );
	 }

// get local trigger data (this is not corrected for clipping!)
	 unsigned int i_nL1trig = 0;
	 for( int p = 0; p < hsdata->event.teldata[telID].trigger_pixels.pixels; p++ )
	 {
	    if( hsdata->event.teldata[telID].trigger_pixels.pixel_list[p] < VDST_MAXCHANNELS )
	    {
	       fData->fDSTL1trig[i_ntel_data][hsdata->event.teldata[telID].trigger_pixels.pixel_list[p]] = 1;
	       i_nL1trig++;
            }
	 }
	 fData->fDSTnL1trig[i_ntel_data] = i_nL1trig;
         i_ntel_data++;
      }
   }
   fGlobalNTelPreviousEvent = i_ntel_data;
   fData->fDSTntel_data = i_ntel_data;

// MC
   fData->fDSTprimary = hsdata->mc_shower.primary_id;
   fData->fDSTenergy = hsdata->mc_shower.energy;
   fData->fDSTaz = hsdata->mc_shower.azimuth * 45./atan( 1. );
   fData->fDSTze = 90.-hsdata->mc_shower.altitude * 45./atan( 1. );
// Observe: transform to VERITAS coordinate system
// x: East, y: North
   fData->fDSTxcore = -1.*hsdata->mc_event.ycore;
   fData->fDSTycore = hsdata->mc_event.xcore;
/////////////////////////////////////////////////////////////////////////////
// calculate offset in camera coordinates from telescope and MC direction
// (OBS: this assumes that all telescopes point into the same direction)
   double i_tel_el = fData->fDSTpointElevation[0];
   double i_tel_az = fData->fDSTpointAzimuth[0];
   float i_x = 0.;
   float i_y = 0.;
   float i_z = 0.;
   VSkyCoordinatesUtilities::getDifferenceInCameraCoordinates( 90.-i_tel_el, i_tel_az, fData->fDSTze, fData->fDSTaz, i_x, i_y, i_z );
   fData->fDSTTel_xoff = i_x;
   fData->fDSTTel_yoff = i_y;
/////////////////////////////////////////////////////////////////////////////

   if( fData->fDST_tree ) fData->fDST_tree->Fill();

   return true;
}

/* 
   fill pedestals, etc into a tree
*/
TTree* DST_fillCalibrationTree( AllHessData *hsdata, map< unsigned int, float > telescope_list )
{
   if( !hsdata ) return 0;

   int fTelID;
   unsigned int nPixel = 0;
   const unsigned int fMaxPixel = 50000;
   float fPed_high[fMaxPixel];
   float fPedvar_high[fMaxPixel];
   float fPed_low[fMaxPixel];
   float fPedvar_low[fMaxPixel];
   float fConv_high[fMaxPixel];
   float fConv_low[fMaxPixel];

   TTree *t = new TTree( "calibration", "calibration data" );

   t->Branch( "TelID", &fTelID, "TelID/I" );
   t->Branch( "NPixel", &nPixel, "NPixel/i" );
   t->Branch( "ped_high", fPed_high, "ped_high[NPixel]/F" );
   t->Branch( "pedvar_high", fPedvar_high, "pedvar_high[NPixel]/F" );
   t->Branch( "ped_low", fPed_low, "ped_low[NPixel]/F" );
   t->Branch( "pedvar_low", fPedvar_low, "pedvar_low[NPixel]/F" );
   t->Branch( "conv_high", fConv_high, "conv_high[NPixel]/F" );
   t->Branch( "conv_low", fConv_low, "conv_low[NPixel]/F" );

   for( int itel = 0; itel <  hsdata->run_header.ntel; itel++ )
   {
     fTelID = hsdata->tel_moni[itel].tel_id;
     if( telescope_list.size() == 0 || telescope_list.find( fTelID ) != telescope_list.end() )
     {
       nPixel = (unsigned int)hsdata->tel_moni[itel].num_pixels;
       if( fMaxPixel < nPixel )
       {
          cout << "DST_fillCalibrationTree error: number of pixels (" << nPixel << ") exeeds allowed range (" << fMaxPixel << ")" << endl;
	  cout << "\t adjust arrays..." << endl;
	  exit( -1 );
       }
       for( unsigned int p = 0; p < nPixel; p++ )
       {
	  if( hsdata->tel_moni[itel].num_ped_slices > 0. )
	  {
	     fPed_high[p] = hsdata->tel_moni[itel].pedestal[HI_GAIN][p] / (double)(hsdata->tel_moni[itel].num_ped_slices);
             fPed_low[p] = hsdata->tel_moni[itel].pedestal[LO_GAIN][p] / (double)(hsdata->tel_moni[itel].num_ped_slices);
          }
	  else
	  {
	     fPed_high[p] = 0.;
	     fPed_low[p] = 0.;
          }
          fPedvar_high[p] = hsdata->tel_moni[itel].noise[HI_GAIN][p];
 	  fPedvar_low[p] = hsdata->tel_moni[itel].noise[LO_GAIN][p];
          fConv_high[p] = hsdata->tel_lascal[itel].calib[HI_GAIN][p] * CALIB_SCALE;
          fConv_low[p] = hsdata->tel_lascal[itel].calib[LO_GAIN][p] * CALIB_SCALE;
       }

       t->Fill();
     }
   }

   return t;
}

TTree* DST_fill_detectorTree( AllHessData *hsdata, map< unsigned int, float > telescope_list, bool iApplyCameraScaling )
{
   if( !hsdata ) return 0;

// HARDCODED: all large telescopes are parabolic
   double fParabolic_mirrorArea = 400.;
   int    fSC_number_of_mirrors = 2;
   cout << "\t Info:" << endl;
   cout << "\t assume that all telescopes with mirror area larger than " << fParabolic_mirrorArea;
   cout << " m^2 are of parabolic type" << endl;
   cout << "\t assume that all telescopes with " << fSC_number_of_mirrors;
   cout << " mirrors are Schwarzschild-Couder telescopes" << endl;

// define tree
   int fTelID = 0;
   unsigned int fNTel = 0;
   float fTelxpos = 0.;
   float fTelypos = 0.;
   float fTelzpos = 0.;
   float fFocalLength = 0.;
   float fCameraScaleFactor = 1.;
   float fCameraCentreOffset = 0.;
   float fCameraRotation = 0.;
   unsigned int nPixel = 0;
   unsigned int nPixel_active = 0;
   unsigned int nSamples = 0;
   float Sample_time_slice = 0.;
   unsigned int nGains;
   float fHiLoScale = 0.;
   int   fHiLoThreshold = 0;
   float fHiLoOffset = 0.;
   const unsigned int fMaxPixel = 50000;
   float fXTubeMM[fMaxPixel];
   float fYTubeMM[fMaxPixel];
   float fRTubeMM[fMaxPixel];
   float fXTubeDeg[fMaxPixel];
   float fYTubeDeg[fMaxPixel];
   float fRTubeDeg[fMaxPixel];
   int nDisabled= 0;
   int fTubeDisabled[fMaxPixel];
   float fMirrorArea = 0.;
   int fNMirrors = 0;
   float fFOV = 0.;
   ULong64_t fTelescope_type = 0;
   for( unsigned int i = 0; i < fMaxPixel; i++ )
   {
      fXTubeMM[i] = 0.;
      fYTubeMM[i] = 0.;
      fRTubeMM[i] = 0.;
      fXTubeDeg[i] = 0.;
      fYTubeDeg[i] = 0.;
      fRTubeDeg[i] = 0.;
      fTubeDisabled[i] = 0;
   }

   TTree *fTreeDet = new TTree( "telconfig", "detector configuration" );

   fTreeDet->Branch( "NTel", &fNTel, "NTel/i" );
   fTreeDet->Branch( "TelID", &fTelID, "TelID/I" );
   fTreeDet->Branch( "TelType", &fTelescope_type, "TelType/l" );
   fTreeDet->Branch( "TelX", &fTelxpos, "TelX/F" );
   fTreeDet->Branch( "TelY", &fTelypos, "TelY/F" );
   fTreeDet->Branch( "TelZ", &fTelzpos, "TelZ/F" );
   fTreeDet->Branch( "FocalLength", &fFocalLength, "FocalLength/F" );
   fTreeDet->Branch( "FOV", &fFOV, "FOV/F" );
   fTreeDet->Branch( "CameraScaleFactor", &fCameraScaleFactor, "CameraScaleFactor/F" );
   fTreeDet->Branch( "CameraCentreOffset", &fCameraCentreOffset, "CameraCentreOffset/F" );
   fTreeDet->Branch( "CameraRotation", &fCameraRotation, "CameraRotation/F" );
   fTreeDet->Branch( "NPixel", &nPixel, "NPixel/i" );
   fTreeDet->Branch( "NPixel_active", &nPixel_active, "NPixel_active/i" );
   fTreeDet->Branch( "NSamples", &nSamples, "NSamples/i" );
   fTreeDet->Branch( "Sample_time_slice", &Sample_time_slice, "Sample_time_slice/F" );
   fTreeDet->Branch( "NGains", &nGains, "NGains/i" );
   fTreeDet->Branch( "HiLoScale", &fHiLoScale, "HiLoScale/F" );
   fTreeDet->Branch( "HiLoThreshold", &fHiLoThreshold, "HiLoThreshold/I" );
   fTreeDet->Branch( "HiLoOffset", &fHiLoOffset, "HiLoOffset/F" );
   fTreeDet->Branch( "XTubeMM", fXTubeMM, "XTubeMM[NPixel]/F" );
   fTreeDet->Branch( "YTubeMM", fYTubeMM, "YTubeMM[NPixel]/F" );
   fTreeDet->Branch( "RTubeMM", fRTubeMM, "RTubeMM[NPixel]/F" );
   fTreeDet->Branch( "XTubeDeg", fXTubeDeg, "XTubeDeg[NPixel]/F" );
   fTreeDet->Branch( "YTubeDeg", fYTubeDeg, "YTubeDeg[NPixel]/F" );
   fTreeDet->Branch( "RTubeDeg", fRTubeDeg, "RTubeDeg[NPixel]/F" );
   fTreeDet->Branch( "NTubesOFF", &nDisabled, "NTubesOFF/I" );
   fTreeDet->Branch( "TubeOFF", fTubeDisabled, "TubeOFF[NPixel]/I" );
   fTreeDet->Branch( "NMirrors", &fNMirrors, "NMirrors/I" );
   fTreeDet->Branch( "MirrorArea", &fMirrorArea, "MirrorArea/F" );

// fill the tree
   for( int itel = 0; itel <  hsdata->run_header.ntel; itel++ )
   {
     fTelID = hsdata->run_header.tel_id[itel];
     if( telescope_list.size() == 0 || telescope_list.find( fTelID ) != telescope_list.end() )
     {
        fNTel++;
     }
   }
   for( int itel = 0; itel <  hsdata->run_header.ntel; itel++ )
   {
     fTelID = hsdata->run_header.tel_id[itel];
     if( telescope_list.size() == 0 || telescope_list.find( fTelID ) != telescope_list.end() )
     {
// Observe: transform to VERITAS coordinate system
// x: East, y: North
	fTelxpos = -1.*hsdata->run_header.tel_pos[itel][1];
	fTelypos = hsdata->run_header.tel_pos[itel][0];
	fTelzpos = hsdata->run_header.tel_pos[itel][2];
	fFocalLength = hsdata->camera_set[itel].flen;
	fCameraScaleFactor = 1.;
	fCameraCentreOffset = 0.;
#ifdef CTA_PROD2
	fCameraRotation = -1.*hsdata->camera_set[itel].cam_rot * TMath::RadToDeg();  // TEMP: check orientation 
#else
        fCameraRotation = 0.;
#endif
	fNMirrors = hsdata->camera_set[itel].num_mirrors;
	fMirrorArea = hsdata->camera_set[itel].mirror_area;

	nPixel = hsdata->camera_set[itel].num_pixels;
	nDisabled = hsdata->pixel_disabled[itel].num_HV_disabled;
	nSamples = hsdata->event.teldata[itel].raw->num_samples;
	Sample_time_slice = hsdata->pixel_set[itel].time_slice;
	nGains = hsdata->event.teldata[itel].raw->num_gains;
	fHiLoScale = hsdata->event.teldata[itel].raw->scale_hg8;
	fHiLoThreshold = hsdata->event.teldata[itel].raw->threshold;
	fHiLoOffset = hsdata->event.teldata[itel].raw->offset_hg8;

	nPixel_active = 0;
	float maxPix_dist = 0.;
	float pix_size = 0.;
	if( nPixel < fMaxPixel )
	{
	   for( unsigned int p = 0; p < nPixel; p++ )
	   {
	       fXTubeMM[p] = hsdata->camera_set[itel].xpix[p]*1.e3;
	       fYTubeMM[p] = hsdata->camera_set[itel].ypix[p]*1.e3;
// use as size the radius of the active area of the tube
	       fRTubeMM[p] = sqrt( hsdata->camera_set[itel].area[p]/TMath::Pi() ) * 1.e3;
	       if( p == 0 ) pix_size = atan2((double)hsdata->camera_set[itel].size[p], (double)fFocalLength ) * TMath::RadToDeg();

   // mm -> deg
	       fXTubeDeg[p] = atan2( (double)fXTubeMM[p]/1000., (double)fFocalLength ) * TMath::RadToDeg();
	       fYTubeDeg[p] = atan2( (double)fYTubeMM[p]/1000., (double)fFocalLength ) * TMath::RadToDeg();
               fRTubeDeg[p] = atan2( (double)fRTubeMM[p]/1000., (double)fFocalLength ) * TMath::RadToDeg();

	       fTubeDisabled[p] = hsdata->pixel_disabled[itel].HV_disabled[p];

	       float x2 = fXTubeDeg[p]*fXTubeDeg[p];
	       float y2 = fYTubeDeg[p]*fYTubeDeg[p];
	       if( sqrt( x2 + y2 ) > maxPix_dist ) 
	       {
	           maxPix_dist = sqrt( x2 + y2 ) * 2.;
               }
// disable pixels which are too far out
	       if( telescope_list.size() != 0 && TMath::Abs( telescope_list[fTelID] ) > 1.e-2 )
	       {
		  if( sqrt( x2 + y2 ) - fRTubeDeg[p] > telescope_list[fTelID]*0.5 )
		  {
		     fTubeDisabled[p] = 2;
		     nDisabled++;
		  }
               }
	       if( fTubeDisabled[p] == 0 ) nPixel_active++;
            }
	}
	if( telescope_list.size() == 0 || TMath::Abs( telescope_list[fTelID] ) < 1.e5 )
	{
	   fFOV = maxPix_dist;
        }
	else if( maxPix_dist < telescope_list[fTelID] )
	{
	   fFOV = maxPix_dist;
        }
	else  fFOV = telescope_list[fTelID];

	fTelescope_type  = TMath::Nint(pix_size*100.);
	fTelescope_type += TMath::Nint(fFOV*10.)*100;
	fTelescope_type += TMath::Nint(fMirrorArea)*100*10*100;
// all large telescopes are parabolic, all others are Davies-Cotton (hardwired)
        if( fMirrorArea > fParabolic_mirrorArea )
	{
	    fTelescope_type += 100000000;
        }
// Schwarzschild-Couder: check number of mirrors
// (assumption is that SC telescope has 2 mirrors only)
	else if( fNMirrors == fSC_number_of_mirrors )
	{
	   fTelescope_type += 200000000;
        }
	else
	{
// from raytracing by G.Hughes and optics note by S.Fegan and V.Vassiliev (2009)
	   if( iApplyCameraScaling )
	   {
// assume that CTA MST is of intermediate-1.2 design
              if( TMath::Abs( fMirrorArea - 100. ) < 5. )
	      {
// G.Hughes (2011/10/21): 2.79+-0.06 %
	         fCameraScaleFactor = 1.-0.0279;
              }
// eq 1 from optics note by S.Fegan and V.Vassiliev (2009)
	      else
	      {
// need f/D; unknown?
//                 fCameraScaleFactor = 1.-1./(TMath::Power(2.,5.)*f_D*f_D) + 1./(TMath::Power(2.,8.) * TMath::Power(f_D,4.) );
//
	         fCameraScaleFactor = 1.;
	      }
           }
	}

        fTreeDet->Fill();
     }
  }

  return fTreeDet;
}

/* -------------------- main program ---------------------- */
/** 
 *  @short Main program 
 *
 *  Main program function of read_hess.c program.
 */

int main(int argc, char **argv)
{
// stop watch 
   TStopwatch fStopWatch;
   fStopWatch.Start();

   IO_BUFFER *iobuf = NULL;
   IO_ITEM_HEADER item_header;
   const char *input_fname = NULL;
   int itel, rc = 0;
   int tel_id;
   int verbose = 0, ignore = 0, quiet = 0;
   double wsum_all = 0.;
   int ntel_trg = 0, min_tel_trg = 0;
   int nev = 0, ntrg = 0;
   char *program = argv[0];
   int showdata = 0, showhistory = 0;
   size_t events = 0, max_events = 0;
   int iarg;

   fGlobalNTelPreviousEvent = 0;
   fGlobalMaxNumberofPixels = 0;
   fGlobalMaxNumberofSamples = 0;
   fGlobalTriggerReset = false;

   string config_file = "";             // file with list of telescopes
   string dst_file = "dst.root";        // output dst file
   bool   fWriteFADC = false;           // fill FADC traces into converter
   unsigned int fDynamicRange = 0;      // dynamic range (for decision of high/low gain)
   bool   fApplyCameraScaling = true;   // apply camera plate scaling according for DC telescopes
   
   static AllHessData *hsdata;

   cout << endl;
   cout << "CTA.convert_hessio_to_VDST: A program to convert hessio data to EVNDISP DST files";
   cout << " (" << VGlobalRunParameter::fEVNDISP_VERSION << ")" << endl;
   cout << " (based on a skeleton program distributed with the hessio package)" << endl;
   cout << "=====================================================================" << endl;
   cout << "(SVN " << VGlobalRunParameter::fEVNDISP_SVNREVISION << ")" << endl;
   
   /* Show command line on output */
   if ( getenv("SHOWCOMMAND") != NULL )
   {
      for (iarg=0; iarg<argc; iarg++)
         printf("%s ",argv[iarg]);
      printf("\n");
   }

   /* Catch INTerrupt and TERMinate signals to stop program */
   signal(SIGINT,stop_signal_function);
   signal(SIGTERM,stop_signal_function);

   if ( argc < 2 )
   {
      syntax(program);
   }
   interrupted = 0;

   /* Check assumed limits with the ones compiled into the library. */
   H_CHECK_MAX();

   if ( (iobuf = allocate_io_buffer(1000000L)) == NULL )
   {
      cout << "Cannot allocate I/O buffer" << endl;
      exit(1);
   }
   iobuf->max_length = 100000000L;


   /* Command line options */
   while ( argc > 1 )
   {
      if ( strcmp(argv[1],"-i") == 0 )
      {
       	 ignore = 1;
	 argc--;
	 argv++;
	 continue;
      }
      else if ( strcmp(argv[1],"-v") == 0 )
      {
       	 verbose = 1;
         quiet = 0;
	 argc--;
	 argv++;
	 continue;
      }
      else if ( strcmp(argv[1],"-q") == 0 )
      {
       	 quiet = 1;
         verbose = 0;
	 argc--;
	 argv++;
	 continue;
      }
      else if ( strcmp(argv[1],"-h") == 0 || strcmp(argv[1],"--history") == 0 )
      {
       	 showhistory = 1;
	 argc--;
	 argv++;
	 continue;
      }
      else if ( strcmp(argv[1],"-s") == 0 )
      {
       	 showdata = 1;
	 argc--;
	 argv++;
	 continue;
      }
      else if ( strcmp(argv[1],"-S") == 0 )
      {
       	 showdata = 1;
         putenv((char *)"PRINT_VERBOSE=1");
	 argc--;
	 argv++;
	 continue;
      }
      else if ( strcmp(argv[1],"--max-events") == 0 && argc > 2 )
      {
       	 max_events = atol(argv[2]);
	 argc -= 2;
	 argv += 2;
	 continue;
      }
      else if (strcmp(argv[1],"-a") == 0 )
      {
	 config_file = argv[2];
	 argc -= 2;
	 argv += 2;
	 continue;
      }
      else if (strcmp(argv[1],"-o") == 0 )
      {
	 dst_file = argv[2];
	 argc -= 2;
	 argv += 2;
	 continue;
      }
      else if (strcmp(argv[1],"-f") == 0 )
      {
         fWriteFADC = atoi( argv[2] );
	 argc -= 2;
	 argv += 2;
	 continue;
      }
      else if (strcmp(argv[1],"-r") == 0 )
      {
         fApplyCameraScaling = atoi( argv[2] );
	 argc -= 2;
	 argv += 2;
	 continue;
      }
      else if (strcmp(argv[1],"-d") == 0 )
      {
         fDynamicRange = (unsigned int)atoi( argv[2] );
	 argc -= 2;
	 argv += 2;
	 continue;
      }
      else if ( strcmp(argv[1],"--help") == 0 )
      {
        printf("\nc_DST: A program to convert hessio data to EVNDISP DST files.\n\n");
        syntax(program);
      }
      else if ( argv[1][0] == '-' && argv[1][1] != '\0' )
      {
        printf("Syntax error at '%s'\n", argv[1]);
        syntax(program);
      }
      else
        break;
   }
   
   if ( verbose && !quiet )
      showhistory = 1;

///////////////////////////////////////////////////////////////////
   cout << endl << "NOTE: FIXED TIMING LEVELS!!" << endl << endl;
///////////////////////////////////////////////////////////////////
// open DST file
   TFile *fDSTfile = new TFile( dst_file.c_str(), "RECREATE" );
   if( fDSTfile->IsZombie() )
   {
       cout << "Error while opening DST output file: " << fDSTfile->GetName() << endl;
       exit( -1 );
   }
   cout << "DST tree will be written to " << dst_file << endl;
   if( fWriteFADC ) cout << "Writing FADC samples to DST file" << endl;
   else             cout << "No FADC output" << endl;
   if( fDynamicRange > 0 ) cout << "Assume " << fDynamicRange << " bit dynamic range" << endl;
   if( fApplyCameraScaling ) cout << "Apply camera plate scaling for DC (intermediate) telescopes" << endl;

// new DST tree
   VDSTTree *fDST = new VDSTTree();
   fDST->setMC();
   map< unsigned int, float > fTelescope_list = fDST->readArrayConfig( config_file );
   if( fTelescope_list.size() < 1 )
   {
      cout << "error reading array configuration file" << endl;
      exit( -1 );
   }
   fDST->setFADC( fWriteFADC );
   fDST->initDSTTree( false );
   fDST->initMCTree();

// MC run header
   VMonteCarloRunHeader *fMC_header = new VMonteCarloRunHeader();
   fMC_header->SetName( "MC_runheader" );
    
   /* Now go over rest of the command line */
   while ( argc > 1 || input_fname != NULL )
   {
    if ( interrupted )
      break;
    if ( argc > 1 )
    {
      if ( argv[1][0] == '-' && argv[1][1] != '\0' )
         syntax(program);
      else
      {
	 input_fname = argv[1];
	 argc--;
	 argv++;
      }
    }
    if ( strcmp(input_fname ,"-") == 0 )
      iobuf->input_file = stdin;
    else if ( (iobuf->input_file = fileopen(input_fname,READ_BINARY)) == NULL )
    {
      perror(input_fname);
      cout << "Cannot open input file." << endl;
      break;
    }

    fflush(stdout);
    fprintf(stderr,"%s\n",input_fname);
    string f_inputfilename= "";
    if( input_fname ) f_inputfilename = input_fname;
    printf("\nInput file '%s' has been opened.\n",input_fname);
    input_fname = NULL;

    for (;;) /* Loop over all data in the input file */
    {
      if ( interrupted )
         break;

      /* Find and read the next block of data. */
      /* In case of problems with the data, just give up. */
      if ( find_io_block(iobuf,&item_header) != 0 )
         break;
      if ( max_events > 0 && events >= max_events )
      {
         if ( iobuf->input_file != stdin )
            break;
         if ( skip_io_block(iobuf,&item_header) != 0 )
            break;
         continue;
      }
      if ( read_io_block(iobuf,&item_header) != 0 )
         break;

      if ( hsdata == NULL && 
           item_header.type > IO_TYPE_HESS_RUNHEADER &&
           item_header.type < IO_TYPE_HESS_RUNHEADER + 200)
      {
         fprintf(stderr,"Trying to read event data before run header.\n");
         fprintf(stderr,"Skipping this data block.\n");
         continue;
      }

//////////////////////////////////////////
// check header types
      switch ( (int) item_header.type )
      {
         /* =================================================== */
         case IO_TYPE_HESS_RUNHEADER:
            /* Summary of a preceding run in the same file ? */
            if ( nev > 0 ) printf("%d of %d events triggered.\n", ntrg, nev);
            nev = ntrg = 0;
            wsum_all = 0.;
            /* Structures might be allocated from previous run */
            if ( hsdata != NULL )
            {
               /* Free memory allocated inside ... */
               for (itel=0; itel<hsdata->run_header.ntel; itel++)
               {
                  if ( hsdata->event.teldata[itel].raw != NULL )
		  {
                     free(hsdata->event.teldata[itel].raw);
		     hsdata->event.teldata[itel].raw = NULL;
		  }
                  if ( hsdata->event.teldata[itel].pixtm != NULL )
		  {
                     free(hsdata->event.teldata[itel].pixtm);
		     hsdata->event.teldata[itel].pixtm = NULL;
		  }
                  if ( hsdata->event.teldata[itel].img != NULL )
		  {
                     free(hsdata->event.teldata[itel].img);
		     hsdata->event.teldata[itel].img = NULL;
		  }
               }
               /* Free main structure */
               free(hsdata);
               hsdata = NULL;
               
               /* Perhaps some cleaning needed in ROOT as well ... */
               
            }
            hsdata = (AllHessData *) calloc(1,sizeof(AllHessData));
            if ( (rc = read_hess_runheader(iobuf,&hsdata->run_header)) < 0 )
            {
	       cout << "Reading run header failed." << endl;
               exit(1);
            }
            if ( !quiet )
               printf("Reading simulated data for %d telescope(s)\n",hsdata->run_header.ntel);
	    if ( verbose || rc != 0 )
               printf("read_hess_runheader(), rc = %d\n",rc);
            if ( showdata ) print_hess_runheader(iobuf);

            for (itel=0; itel<hsdata->run_header.ntel; itel++)
            {
               tel_id = hsdata->run_header.tel_id[itel];
               hsdata->camera_set[itel].tel_id = tel_id;
               hsdata->camera_org[itel].tel_id = tel_id;
               hsdata->pixel_set[itel].tel_id = tel_id;
               hsdata->pixel_disabled[itel].tel_id = tel_id;
               hsdata->cam_soft_set[itel].tel_id = tel_id;
               hsdata->tracking_set[itel].tel_id = tel_id;
               hsdata->point_cor[itel].tel_id = tel_id;
               hsdata->event.num_tel = hsdata->run_header.ntel;
               hsdata->event.teldata[itel].tel_id = tel_id;
               hsdata->event.trackdata[itel].tel_id = tel_id;
               if ( (hsdata->event.teldata[itel].raw = 
                      (AdcData *) calloc(1,sizeof(AdcData))) == NULL )
               {
		   cout << "Not enough memory" << endl;
                  exit(1);
               }
               hsdata->event.teldata[itel].raw->tel_id = tel_id;
               if ( (hsdata->event.teldata[itel].pixtm =
                     (PixelTiming *) calloc(1,sizeof(PixelTiming))) == NULL )
               {
		  cout << "Not enough memory" << endl;
                  exit(1);
               }
               hsdata->event.teldata[itel].pixtm->tel_id = tel_id;
               if ( (hsdata->event.teldata[itel].img = 
                      (ImgData *) calloc(2,sizeof(ImgData))) == NULL )
               {
		  cout << "Not enough memory" << endl;
                  exit(1);
               }
               hsdata->event.teldata[itel].max_image_sets = 2;
               hsdata->event.teldata[itel].img[0].tel_id = tel_id;
               hsdata->event.teldata[itel].img[1].tel_id = tel_id;
               hsdata->tel_moni[itel].tel_id = tel_id;
               hsdata->tel_lascal[itel].tel_id = tel_id;
            }
            break;

         /* =================================================== */
         case IO_TYPE_HESS_MCRUNHEADER:
            rc = read_hess_mcrunheader(iobuf,&hsdata->mc_run_header);

	    if ( verbose || rc != 0 )
               printf("read_hess_mcrunheader(), rc = %d\n",rc);
//            if ( showdata )
               print_hess_mcrunheader(iobuf);

// fill EVNDISP DST run header
	    DST_fillMCRunheader( fMC_header, hsdata );
            break; 

         /* =================================================== */
	 case IO_TYPE_MC_INPUTCFG:
            {
               struct linked_string corsika_inputs;
               corsika_inputs.text = NULL;
               corsika_inputs.next = NULL;
	       read_input_lines(iobuf,&corsika_inputs);
	       if ( corsika_inputs.text != NULL )
	       {
	          struct linked_string *xl = NULL, *xln = NULL;
                  if ( ! quiet )
	             printf("\nCORSIKA was run with the following input lines:\n");
	          for (xl = &corsika_inputs; xl!=NULL; xl=xln)
                  {
                     if ( ! quiet )
		        printf("   %s\n",xl->text);
		     free(xl->text);
		     xl->text = NULL;
		     xln = xl->next;
		     xl->next = NULL;
		     if ( xl != &corsika_inputs )
		        free(xl);
                  }
               }
            }
            break;

         /* =================================================== */
         case 70: /* How sim_hessarray was run and how it was configured. */
            if ( showhistory )
               list_history(iobuf,NULL);
            break;

         /* =================================================== */
         case IO_TYPE_HESS_CAMSETTINGS:
            tel_id = item_header.ident; // Telescope ID is in the header
            if ( (itel = find_tel_idx(tel_id)) < 0 )
            {
               char msg[256];
               snprintf(msg,sizeof(msg)-1,
                  "Camera settings for unknown telescope %d.", tel_id);
                cout << msg << endl;
               exit(1);
            }
            rc = read_hess_camsettings(iobuf,&hsdata->camera_set[itel]);
	    if ( verbose || rc != 0 )
               printf("read_hess_camsettings(), rc = %d\n",rc);
            break;

         /* =================================================== */
         case IO_TYPE_HESS_CAMORGAN:
            tel_id = item_header.ident; // Telescope ID is in the header
            if ( (itel = find_tel_idx(tel_id)) < 0 )
            {
               char msg[256];
               snprintf(msg,sizeof(msg)-1,
                  "Camera organisation for unknown telescope %d.", tel_id);
                cout << msg << endl;
               exit(1);
            }
            rc = read_hess_camorgan(iobuf,&hsdata->camera_org[itel]);
	    if ( verbose || rc != 0 )
               printf("read_hess_camorgan(), rc = %d\n",rc);
            if ( showdata )
               print_hess_camorgan(iobuf);
            break;

         /* =================================================== */
         case IO_TYPE_HESS_PIXELSET:
            tel_id = item_header.ident; // Telescope ID is in the header
            if ( (itel = find_tel_idx(tel_id)) < 0 )
            {
               char msg[256];
               snprintf(msg,sizeof(msg)-1,
                  "Pixel settings for unknown telescope %d.", tel_id);
	       cout << msg << endl;
               exit(1);
            }
            rc = read_hess_pixelset(iobuf,&hsdata->pixel_set[itel]);
	    if ( verbose || rc != 0 )
               printf("read_hess_pixelset(), rc = %d\n",rc);
            if ( showdata )
               print_hess_pixelset(iobuf);
            break;

         /* =================================================== */
         case IO_TYPE_HESS_PIXELDISABLE:
            tel_id = item_header.ident; // Telescope ID is in the header
            if ( (itel = find_tel_idx(tel_id)) < 0 )
            {
               char msg[256];
               snprintf(msg,sizeof(msg)-1,
                  "Pixel disable block for unknown telescope %d.", tel_id);
		cout << msg << endl;
               exit(1);
            }
            rc = read_hess_pixeldis(iobuf,&hsdata->pixel_disabled[itel]);
	    if ( verbose || rc != 0 )
               printf("read_hess_pixeldis(), rc = %d\n",rc);
            break;

         /* =================================================== */
         case IO_TYPE_HESS_CAMSOFTSET:
            tel_id = item_header.ident; // Telescope ID is in the header
            if ( (itel = find_tel_idx(tel_id)) < 0 )
            {
               char msg[256];
               snprintf(msg,sizeof(msg)-1,
                  "Camera software settings for unknown telescope %d.", tel_id);
		cout << msg << endl;
               exit(1);
            }
            rc = read_hess_camsoftset(iobuf,&hsdata->cam_soft_set[itel]);
	    if ( verbose || rc != 0 )
               printf("read_hess_camsoftset(), rc = %d\n",rc);
            break;

         /* =================================================== */
         case IO_TYPE_HESS_POINTINGCOR:
            tel_id = item_header.ident; // Telescope ID is in the header
            if ( (itel = find_tel_idx(tel_id)) < 0 )
            {
               char msg[256];
               snprintf(msg,sizeof(msg)-1,
                  "Pointing correction for unknown telescope %d.", tel_id);
	       cout << msg << endl;
               exit(1);
            }
            rc = read_hess_pointingcor(iobuf,&hsdata->point_cor[itel]);
	    if ( verbose || rc != 0 )
               printf("read_hess_pointingco(), rc = %d\n",rc);
            break;

         /* =================================================== */
         case IO_TYPE_HESS_TRACKSET:
            tel_id = item_header.ident; // Telescope ID is in the header
            if ( (itel = find_tel_idx(tel_id)) < 0 )
            {
               char msg[256];
               snprintf(msg,sizeof(msg)-1,
                  "Tracking settings for unknown telescope %d.", tel_id);
		cout << msg << endl;
               exit(1);
            }
            rc = read_hess_trackset(iobuf,&hsdata->tracking_set[itel]);
	    if ( verbose || rc != 0 )
               printf("read_hess_trackset(), rc = %d\n",rc);
            break;

         /* =================================================== */
         case IO_TYPE_HESS_EVENT:
            rc = read_hess_event(iobuf,&hsdata->event,-1);
	    if ( verbose || rc != 0 )
               printf("read_hess_event(), rc = %d\n",rc);
            events++;
            if ( showdata )
               print_hess_event(iobuf);
            /* Count number of telescopes (still) present in data and triggered */
            ntel_trg = 0;
            for (itel=0; itel<hsdata->run_header.ntel; itel++)
               if ( hsdata->event.teldata[itel].known )
               {
                  /* If non-triggered telescopes record data (like HEGRA),
                     we may have to check the central trigger bit as well,
                     but ignore this for now. */
                  ntel_trg++;
               }
	    if ( hsdata->event.shower.known )
	       hsdata->event.shower.num_trg = ntel_trg;
            if ( ntel_trg < min_tel_trg )
               continue;
            ntrg++;

// fill EVNDISP DST event
	    DST_fillEvent( fDST, hsdata, fTelescope_list, fWriteFADC, fDynamicRange );

            break;

         /* =================================================== */
         case IO_TYPE_HESS_CALIBEVENT:
         {
            int type = -1;
            rc = read_hess_calib_event(iobuf,&hsdata->event,-1,&type);
	    if ( verbose || rc != 0 )
               printf("read_hess_calib_event(), rc = %d, type=%d\n",rc,type);
         }
            break;

         /* =================================================== */
         case IO_TYPE_HESS_MC_SHOWER:
            rc = read_hess_mc_shower(iobuf,&hsdata->mc_shower);
	    if ( verbose || rc != 0 )
               printf("read_hess_mc_shower(), rc = %d\n",rc);
            if ( showdata )
               print_hess_mc_shower(iobuf);
            break;

         /* =================================================== */
         case IO_TYPE_HESS_MC_EVENT:
            rc = read_hess_mc_event(iobuf,&hsdata->mc_event);
	    if ( verbose || rc != 0 )
               printf("read_hess_mc_event(), rc = %d\n",rc);
            if ( showdata ) print_hess_mc_event(iobuf);

            DST_fillMCEvent( fDST, hsdata );

            break;

         /* =================================================== */
         case IO_TYPE_MC_TELARRAY:
            if ( hsdata && hsdata->run_header.ntel > 0 )
            {
               rc = read_hess_mc_phot(iobuf,&hsdata->mc_event);
	       if ( verbose || rc != 0 )
                  printf("read_hess_mc_phot(), rc = %d\n",rc);
            }
            break;

         /* =================================================== */
         case IO_TYPE_HESS_MC_PE_SUM:
            rc = read_hess_mc_pe_sum(iobuf,&hsdata->mc_event.mc_pesum);
	    if ( verbose || rc != 0 )
               printf("read_hess_mc_pe_sum(), rc = %d\n",rc);
            if ( showdata )
               print_hess_mc_pe_sum(iobuf);
            break;

         /* =================================================== */
         case IO_TYPE_HESS_TEL_MONI:
            // Telescope ID among others in the header
            tel_id = (item_header.ident & 0xff) | 
                     ((item_header.ident & 0x3f000000) >> 16); 
            if ( (itel = find_tel_idx(tel_id)) < 0 )
            {
               char msg[256];
               snprintf(msg,sizeof(msg)-1,
                  "Telescope monitor block for unknown telescope %d.", tel_id);
	       cout << msg << endl;
               exit(1);
            }
            rc = read_hess_tel_monitor(iobuf,&hsdata->tel_moni[itel]);
	    if ( verbose || rc != 0 )
               printf("read_hess_tel_monitor(), rc = %d\n",rc);
            if ( showdata )
               print_hess_tel_monitor(iobuf);
            break;

         /* =================================================== */
         case IO_TYPE_HESS_LASCAL:
            tel_id = item_header.ident; // Telescope ID is in the header
            if ( (itel = find_tel_idx(tel_id)) < 0 )
            {
               char msg[256];
               snprintf(msg,sizeof(msg)-1,
                  "Laser/LED calibration for unknown telescope %d.", tel_id);
		cout << msg << endl;
               exit(1);
            }
            rc = read_hess_laser_calib(iobuf,&hsdata->tel_lascal[itel]);
	    if ( verbose || rc != 0 )
               printf("read_hess_laser_calib(), rc = %d\n",rc);
            if ( showdata )
               print_hess_laser_calib(iobuf);
            break;

         /* =================================================== */
         case IO_TYPE_HESS_RUNSTAT:
            rc = read_hess_run_stat(iobuf,&hsdata->run_stat);
	    if ( verbose || rc != 0 )
               printf("read_hess_run_stat(), rc = %d\n",rc);
            if ( showdata )
               print_hess_run_stat(iobuf);
            break;

         /* =================================================== */
         case IO_TYPE_HESS_MC_RUNSTAT:
            rc = read_hess_mc_run_stat(iobuf,&hsdata->mc_run_stat);
	    if ( verbose || rc != 0 )
               printf("read_hess_mc_run_stat(), rc = %d\n",rc);
            if ( showdata )
               print_hess_mc_run_stat(iobuf);
            break;

         default:
            if ( !ignore )
               fprintf(stderr,"Ignoring data block type %ld\n",item_header.type);
      }
    }
    cout << "writing DST, MC and detector trees" << endl;
    if( fDST && fDST->getDSTTree() )
    {
        fDST->getDSTTree()->Write();
	cout << "\t (writing " << fDST->getDSTTree()->GetEntries() << " events)" << endl;
    }
    if( fDST && fDST->getMCTree()  )
    {
       cout << "\t (writing " << fDST->getMCTree()->GetEntries() << " MC events)" << endl;
       fDST->getMCTree()->Write();
    }
// writing detector tree
    TTree *i_detTree = DST_fill_detectorTree( hsdata, fTelescope_list, fApplyCameraScaling );
    if( i_detTree ) i_detTree->Write();
// writing Monte Carlo header
    if( fMC_header )
    {
       fMC_header->Write();
       fMC_header->print();
    }
// writing calibration data
    TTree *i_calibTree = DST_fillCalibrationTree( hsdata, fTelescope_list );
    if( i_calibTree ) i_calibTree->Write();
///////////////////////////////////////////////////
// writing run parameters to dst file
    VEvndispRunParameter *fRunPara = new VEvndispRunParameter();
    fRunPara->SetName( "runparameterDST" );
    fRunPara->setSystemParameters();
    fRunPara->fEventDisplayUser = "CTA-DST";
    fRunPara->frunnumber = fDST->getDSTRunNumber();
    fRunPara->getObservatory() = "CTA";
    fRunPara->fsourcetype = 7;                                  // 7 is DST - MC
    fRunPara->fsourcefile = f_inputfilename;
    fRunPara->fIsMC = true;
    fRunPara->fCalibrationDataType = 0;                         // no pedvars available
    fRunPara->fFADCChargeUnit = "PE";
    fRunPara->fNTelescopes = fDST->getDSTNTel();
    if( fRunPara->fNTelescopes == 0 && i_detTree ) fRunPara->fNTelescopes = (unsigned int)i_detTree->GetEntries();
    fRunPara->fpulsetiminglevels = getPulseTimingLevels();
    fRunPara->setPulseZeroIndex();
    fRunPara->Write();
///////////////////////////////////////////////////

    if( fDSTfile ) fDSTfile->Close();

    if ( iobuf->input_file != NULL && iobuf->input_file != stdin ) fileclose(iobuf->input_file);
    iobuf->input_file = NULL;
    reset_io_block(iobuf);

    if ( nev > 0 ) printf("%d of %d events triggered\n", ntrg, nev);

    if ( hsdata != NULL ) hsdata->run_header.run = 0;
   }

   fStopWatch.Stop();
   fStopWatch.Print();

   cout << "exit..." << endl;

   return 0;
}

