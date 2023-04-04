# EVENTDISPLAY - IACT event analysis and display

calibrate and parameterize images, event reconstruction, stereo analysis

----------------------------------------------

For installation see INSTALL.md

For a detailed description of the command line parameters see
[docs/EVNDISP.commandline.md](docs/EVNDISP.commandline.md)

----------------------------------------------

Required options for VERITAS [VTS] analysis are:

	-runnumber=<RUNNUMBER>         [number of the run to analyse]
	   or
        -sourcefile <source file name> [name of the vbf file to analyse]

	see $EVNDISPSYS/scripts/VTS for scripts for calibration, analysis and
	efficient submission of a large number of jobs to a computing cluster

Option to display events:

	-display=<0=off/1=on>          [open the display to look at events]

----------------------------------------------

Required auxiliary files:

   global run parameters (e.g. observatory location and links to database)
   $VERITAS_EVNDISP_AUX_DIR/Parameterfiles/EVNDISP.global.runparameter

   analysis and reconstruction parameter file, see e.g.
   $VERITAS_EVNDISP_AUX_DIR/Parameterfiles/EVNDISP.reconstruction.runparameter

Optional auxiliary files:

   [VTS] list of special channels (e.g. L2 channels, channels to remove from the analysis)
   $VERITAS_EVNDISP_AUX_DIR/Parameterfiles/EVNDISP.specialchannels.dat

   [VTS] list of criteria to find malfunctioning channels
   $VERITAS_EVNDISP_AUX_DIR/Parameterfiles/EVNDISP.validchannels.dat

----------------------------------------------

Calibration data:

   [VTS] Calibration files (relative gains and time offsets, pedestals) are located in
   $VERITAS_EVNDISP_AUX_DIR/Calibration/Tel_NN

   [VTS] Run numbers for low-gain calibration:
   $VERITAS_EVNDISP_AUX_DIR/Calibration/calibrationlist.LowGain.dat

   [VTS] The following file is ONLY needed in case non-standard flasher files are used
   (or no connection to the VERITAS DB could be established):
   $VERITAS_EVNDISP_AUX_DIR/Calibration/calibrationlist.dat

----------------------------------------------

Detector configuration:

   [VTS] Detector configuration files (with telescope positions, pixel positions, etc):
   $VERITAS_EVNDISP_AUX_DIR/DetectorGeometry/*.cfg
