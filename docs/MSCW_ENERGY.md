
mscw_energy: calculation of mean scaled width and length, and energy with lookup tables
---------------------------------------------------------------------------------------

Input data must be in form of evndisp result files (ROOT files produced by evndisp)
Output is
   i)  for filling table mode: a lookup table
   ii) for analyzing mode:     a ROOT file with MSCW/MSCL/Energy, etc per event ( .mscw.root )

--------------------------------------------

command line parameters:

general parameters:
	 -arrayrecid=0/1/... 	 choose eventdisplay array reconstruction record (default 0)
	 -inputfile FILE 	 evndisp root file as input (wildcards possible use, -inputfile "YOUR_DATA_DIRECTORY/*.root")
	 -inputfilelist FILELIST list of evndisp root file as input (wildcards possible use, -inputfile "YOUR_DATA_DIRECTORY/*.root")
	 			 (simple text file)

table filling:

	 -filltables=1           flag to indicate that tables should be filled
	 -tablefile FILE 	 root file with all the tables (existing files are updated)
	 -ze=FLOAT       	 zenith angle of simulations
	 -woff=FLOAT      	 wobble offset of simulations [deg]
	 -noise=NINT     	 mean pedestal variance x 100 (integer value; used to set the right directory structure)

table reading:

	 -outputfile FILE 	 name of output file
	 -tablefile FILE 	 file with lookup tables
	                 	 (see file "Where-to-find-tables" to get existing lookup tables)

For additional optional options, see further below.

--------------------------------------------

Lookup table files:

	lookup table files are expected to be in
	$VERITAS_EVNDISP_AUX_DIR/Tables/

--------------------------------------------

EXAMPLES:

 i) using a eventdisplay output file called 33072.root and a table file called table_ID00.root

      mscw_energy -tablefile table_ID00.root -inputfile 33072.root

      this produces an output file called 33072.mscw.root with mscw, mscl, and energy assign for each event

  ii) using a eventdisplay output file called 33072.root for array reconstruction ID 8 and table file table_ID8.root

      mscw_energy -tablefile table_ID8root -inputfile 33072.root -arrayrecid=8

      this produces an output file called 33072.mscw.root with mscw, mscl, and energy assign for each event

 iii) using a bunch of simulations files (say runnumbers 521000 to 5210010)
     (with size cut, but no distance cut applied in eventdisplay array analysis):

      mscw_energy -tablefile table.root -inputfile "52100[0-1].root" -outputfile simu.mscw.root

==========================================================================================

Additional options for table filling:

	 -debug=UINT             debug output level (0=default, 1=low, 2=high)
         -updateEpoch=0/1        re-read instrument epoch from VERITAS.Epochs.runparameter and update runparameters
	 -minshowerperbin=INT    minimum number of showers per bin required for analysis (default=5)
	 -write1DHistograms 	 write 1D-histograms for median determination to disk (default off)
	 -selectRandom=[0,1] 	 selected events randomly (give probability)
	 -selectRandomSeed=INT 	 set seed for random select (default=17)
	 -mindistancetocameracenter=FLOAT  minimum distance of events from camera center (MC distance, default = -1.e10)
	 -maxdistancetocameracenter=FLOAT  maximum distance of events from camera center (MC distance, default =  1.e10)
	 -minImages=INT          minimum number of images required per event (comparator geq, default=2)
         (general corrections, optional)
         -sizecorrect=FLOAT,FLOAT,FLOAT,...      apply correction to the size in the mscw/mscl/energy table [experimental]
         (individual corrections, optional, overwrites general corrections)
         -sizemscwcorrect=FLOAT,FLOAT,FLOAT,...  apply correction to the size in the mscw table, per telescope  [experimental]
         -sizemsclcorrect=FLOAT,FLOAT,FLOAT,...  apply correction to the size in the mscl table     [experimental]
         -sizeenergycorrect=FLOAT,FLOAT,FLOAT,...  apply correction to the size in the energy table [experimental]

Additional options for table reading:

	 -use_evndisp_selected_images=0/1  use images selected by evndisp file reconstruction (default: 1)
	 -noNoTrigger 		 don't fill events without array trigger into output tree [RECOMMENDED VALUE FOR MC]
	 -writeReconstructedEventsOnly	 write only reconstructed events to output tree   [RECOMMENDED VALUE FOR MC]
	 -shorttree 		 write only a short version of the output tree to disk (switch of -noshorttree)

	 -maxnevents=INT         maximum number of events to read from eventdisplay file (default=all)
	 -maxruntime=FLOAT       maximum amount of time in this run to analyse in [s]
	 -nomctree               do not copy MC tree to mscw output file

print run parameters for an existing mscw file

	 -printrunparameters FILE

### Exploratory Monte Carlo image smearing

In lookup-table reading mode, `-imagesmearing=seed,fraction,size,width,length,centroid,asym,tgrad`
perturbs image parameters before stereo image selection and reconstruction. It requires
`-redo_stereo_reconstruction`, Monte Carlo evndisp input, and existing nominal
lookup tables. For example:

```sh
mscw_energy -tablefile nominal.root -inputfile gamma.root -outputfile gamma_smeared.root \
  -redo_stereo_reconstruction -imagesmearing=42,1,0.18,0.02,0.04,0.005,0.2,0.5
```

The example strengths are exploratory, not a detector calibration. The fraction is the
probability of perturbing an event, shared by its images. Size receives an independent
Gaussian perturbation in **log10(size)** with the specified standard deviation. Width,
length and each centroid coordinate receive additive Gaussian perturbations in degrees;
asymmetry and timing gradient use their native units. Positive width and length are
floored at 1e-6 degrees. Centroid distance is recalculated from the smeared coordinates.
Images must have a finite positive size and a readable `tpars` entry. By default only
images selected by evndisp are read; use the existing `-use_evndisp_selected_images=0`
option if the stereo rerun should consider all available images. Existing output selection
still determines which events are written.

The seed, fraction, strengths and algorithm version are stored in `TLRunParameter`.
Version 1 uses event/run/telescope keyed random draws; these are reproducible for the
same identifiers and seed, but do not reproduce NumPy's particular random numbers.
The output image branches contain the values used by the reconstruction. Do not also
activate Eventdisplay-ML image smearing on these files unless a second perturbation is
intended. The smearing model uses independent Gaussian perturbations and needs validation
against conditional data/MC distributions before production use.
