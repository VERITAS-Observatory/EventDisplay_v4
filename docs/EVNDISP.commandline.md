
evndisp command line options:
=============================

(observe: default values are in general good for a standard analysis; if not complain!)

Typical use cases:
------------------

Plot VTS data:
./bin/evndisp -display=1 -runnumber=<run number> \
              -calibrationdirectory <point to directory with calibration values>
              (for larger display, add the option -highres)

Quick plotting of VTS data, ignoring all calibration data:
./bin/evndisp -plotraw -nocalibnoproblem -display=1 -sourcefile <your vbf data file>

For analysis, use the ANALYSIS.evndisp.sh script. 
To run the calibration steps only, use the ANALYSIS.evndisp.sh with calibration option set to 5.

General:
--------
     -runmode=0-7                0=analysis (default)
                                 1=pedestal calculation (high gain channels)
                                 2=relative gain/toffset calculation (high gain channels)
                                 3=trace library production
                                 4=write dstfile
                                 5=relative gain/toffset calculation (low gain channels)
                                 6=pedestal calculation (low gain channels)
                                 7=average pulse arrival (tzero) calculation
     -sourcefile FILENAME        full path + filename
     -sourcetype=0-7             source data file format
                                 0=rawdata (VTS eventbuilder type)
                                 1=GrIsu Monte Carlo (ascii file)
                                 2=MC in vbf/cvbf format
                                 3=rawdata in vbf/cvbf format (default)
                                 4=DST (data)
                                 5=multiple GrISu MC files
                                 6=PE file
                                 7=DST (MC) file
     -runnumber=INT              set runnumber (default: get run number from sourcefile name)
     -useDBinfo                  get run info (target, wobble offsets, etc.) from database
                                 (attention, this might overwrite some of the given command line parameters, 
                                 default: on, switch of with -donotusedbinfo )
     -nevents=NEVENTS            loop over NEVENTS events in display=0 mode (<0 = no limit) (default=-10000)
     -firstevent=EVENTNUMBER     start analysis at event EVENTNUMBER (default=-10000)
     -timecutMin=TIME_MIN        start analysis at minute TIME_MIN
     -timecutMax=TIME_MAX        stop analysis at minute TIME_MAX
     -reconstructionparameter FILENAME   file with reconstruction parameters (e.g., array analysis cuts)
     -epochfile FILENAME         file with definitions of epochs (e.g. VERITAS.Epochs.runparameter)
     -epoch STRING               set epoch (e.g. V5) for current run
     -atmosphereid=INT           set ID for atmosphere (corsika value)

Output:
-------
     -output FILE.root           file with analysis results (FILE=-1 means no output) (default=RUNNUMBER.root)
     -writeallMC                 write all events, even those without array trigger, to showerpars and
                                 tpars trees (MC only, default: off)
     -writenoMCTree              do not write MC event tree to output file (MC only, default: 1)
     -printdeadpixelinfo         print list of the telescope, gain, and channel number of all disabled 
                                 channels to <runnumber>.evndisp.log 
                                 each line will contain the word DEADCHAN for easy grep-ability, 
                                 and the reason(s) the channel was disabled
     -writeextracalibtree        In gain calculating mode: Write additional tree into gain.root file 
                                 containing channel charge, tzero, and monitor charge for all flasher events. 

Detector definition:
--------------------
     -ntelescopes=INT            number of telescopes (any integer, default 4)
     -teltoana=INT               analyze only these telescopes (default 1234)
                                 (Telescope 1=1,..., Telescopes 2 and 3 = 23, Telescopes 1,2,4 = 124, or 1,2,10,25)
     -camera=CAMERA              set detector geometry file (default=veritasBC4_080117_Autumn2007-4.1.2_EVNDISP.cfg)
     -vbfnsamples                use number of FADC samples from VBF file (default=0)


Calibration:
------------
     -readcalibdb                read relative gains and time offsets from offline DB (switch off with -nocalibnoproblem)
     -calibrationfile FILENAME   file with names of pedestal/gain/toffset/pixel status files (assume path $EVNDATA/calibration/)
     -lowgaincalibrationfile FILENAME    file with names for pedestals and high/low gain multiplier files 
                                             (assume path $EVNDATA/calibration/)
     -gaincorrection=FLOAT       apply correction to gains (default=1)
     -usepeds                    use only true pedestal events (event type=2; use -donotusepeds to switch it off)
     -lasermin=INT               minimal total charge sum for a event to be a laser event (default=50000)
     -l2setspecialchannels FILENAME      set special channels for l2 feed into FADC crates (default=specialChannel.dat)
     -l2timecorrect=0/1          apply FADC stop time corrections based on L2 pulses (default=true)
     -Pedestalsumwindow=INT      length of sum window for pedestal variations (default=20)
     -Pedestalssumfirst=INT      start of sum window for pedestal variations (default=0)
     -usePedestalsInTimeSlices=0/1       use time dependent pedestals (high gain channels) (default = on(1))
     -usePedestalsInTimeSlicesLowGain=0/1    use time dependent pedestals (low gain channels) (default = off(0))
     -PedestalsInTimeSlices      calculate pedestals on short time scale (default=false)
     -PedestalsLengthOfTimeSlice=FLOAT   length of time slices for pedestal variations (default=180s)
     -deadchannelfile FILE       read this file with dead channel definitions (default=deadChannelDefinition.dat)

Pointing: 
---------
     -elevation                  telescope elevation
     -azimuth                    telescope azimuth 
     -target TARGET              telescope is pointed to this target (use -print targets to print available targets)
     -printtargets               print available targets
     -declination=DEC            target is at this declination (J2000)
     -rightascension=RA          target is at this right ascension (J2000)
     -decoffset=DEC              offset added to target declination (degrees)
     -raoffset=RA                offset added to target right ascension (degrees - e.g. +/- 7.5 for 30 minutes OFF source)
     -wobblenorth=FLOAT          wobble offset NORTH (degrees) (this does not overwrite the DB value)
     -wobbleeast=FLOAT           wobble offset EAST (degrees) (this does not overwrite the DB value)
     -overwriteDB_wobblenorth=FLOAT      wobble offset NORTH (degrees) (this does overwrite the DB value)
     -overwriteDB_wobbleeast=FLOAT       wobble offset EAST (degrees) (this does overwrite the DB value)
     -checkpointing=FLOAT        abort of difference between calculated pointing direction and vbf pointing direction 
                                 is larger than this value ([deg], default=999 deg)
     -pointingErrorX=INT:FLOAT   take pointing error in array reconstruction into account
                                 (camera x-direction [deg], default 0, 
                                 usage: for telescope 1 do for example: -pointingErrorX=1:0.05)
                                 (NOTE: experts only; option disables reading of pointing values from the DB)
     -pointingErrorY=INT:FLOAT   take pointing error in array reconstruction into account
                                 (camera y-direction [deg], default 0, 
                                 usage: for telescope 1 do for example: -pointingErrorY=1:0.05)
                                 (NOTE: experts only; option disables reading of pointing values from the DB)
     -useDBtracking              use database to calculate pointing errors (default: on, switch off with -usenodbtracking )
     -useTCorrectionfrom SQL-DATE        use pointing calculated with T-point correction valid for this data
                                         (default: not applied, example: -useTCorrectionfrom "2007-10-10"
     -pointingmonitortxt DIRECTORY       find pointing monitor text files in this directory 
                                         (default: not applied, expect filename as pointing_VPM.37195.t1.dat)
     -usedbvpm                   use calibrated pointing monitor data from DB (usenodbvpm to switch it off)
     -dstfile FILENAME           name of dst output file (root file, default: dstfile.root)
     -dstallpixel=INT            write data from all pixels to dst files (0: write image/border pixel only; default: 1)

Image calculation:
------------------
     -smoothdead                 smooth over dead pixels
     -logl=0/1/2                 perform loglikelihood image parametrization 0=off,1=on,2=on with minuit output (default=off)
     -fuifactor=FLOAT            fraction of image/border pixel under image ellipse fact (default=2)

Display options:
----------------
     -display=0/1                show eventdisplay (default=0)
     -loop                       infinite loop (default=0)
     -highres                    large display
     -hdhres                     larger display
     -photodiode                 show photodiode
     -plotraw                    fast plotting without pedestals/gains/toffsets
     -plotpaper                  clean plots for talks/papers (no dead channels, no small text, ..., default=false)
     -plotmethod=INT             results of this array reconstruction are shown in 'all in one' display (default=0)
     -starcatalogue Hipparcos_MAG8_1997.dat  plot stars into the display
     -starbrightness=float       plot stars brighter than the given B magnitude

Simulations: 
-------------
     -isMC=0/1/2                 source data is MC (not/write all MC events/only triggered MC events, default=0)
     -fillmchisto                fill MC histograms (default 1)
     -teleNoff=INT               offset in telescope counting (default: first tel. = 1; for grisu MC raw files only)
     -sampleoff=INT              offset in FADC sample reading (default: 0)
     -tracelib FILE.root         trace library file for MC background creation
     -ignoredead                 ignore dead channel labeling in camera configuration files (default=false)
     -ndeadchannel=INT           number of pixels to set randomly dead (default=0)
     -seedndead=INT              seed for setting pixels randomly dead (default=0)
     -ndeadboard=INT             number of boards to set randomly dead (default=0)
     -fadcscale=float            scale factor for traces (default=1.)
     -pedestalseed=int           seed for pedestal calculation (default=0)
     -pedestalfile FILENAME      use pedestals from P lines in this pedestal file (default: Off)
     -pedestalnoiselevel=int     noise level used for external pedestal file (default=250)
     -pedestalDefaultPedestal=float      default pedestals used in simulations (default=15.)
     -lowgainpedestallevel=float pedestal level to be used for low-gain channels in MC (default=off)
     -printGrisuHeader=0/1/2     0 (default)=nothing,   
                                 1=Reproduce full grisu simulation header including detector config file (from VBF header).
                                 2=Print config file name that was used for detector simulation, if available.

Muon analysis:
--------------
     -muon                       search for muon rings
     -hough                      search for muon rings with Hough method

Others:
-------
     -debug                      print lots of debug output
     -user USERNAME              username

For installation see file INSTALL.md
