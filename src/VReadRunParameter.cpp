/*! \class VReadRunParameter

  evndisp -help gives an overview of all parameters

  \attention
  camera type (<> run 592) hard coded in adjustParams() (this is valid for the Prototype)

  \todo
   - tests, if run parameters make sense
   - reading a configuration file (for more than one telescope...)

\date
16/08/2004

Revision $Id: VReadRunParameter.cpp,v 1.56.2.5.2.2.2.12.10.10.2.10.4.7.2.3.2.10.2.6.4.3.2.11.2.19.2.13.2.5 2011/04/06 11:57:57 gmaier Exp $

\author
Gernot Maier

*/

#include <VReadRunParameter.h>

VReadRunParameter::VReadRunParameter()
{
    fDebug = false;
    if( fDebug ) cout << "VReadRunParameter::VReadRunParameter()" << endl;
    fRunPara = new VEvndispRunParameter();
    fRunPara->printGlobalRunParameter();
    fusercamera = false;
    f_boolCommandline = false;
    f_boolConfigfile = false;

    fTelToAna = 0;
    fTelToAnaString = "";

    fPrintOutputFile = false;
    fGetLaserRunNumber = false;
}


/*!
   \attention
      reading from the command line assumes same parameters for all telescopes (preliminary)
*/
bool VReadRunParameter::readCommandline( int argc, char *argv[] )
{
    if( fDebug ) cout << "VReadRunParameter::readCommandline()" << endl;
    f_boolCommandline = true;
    int i = 1;
// no command line parameters
    if( argc == 1 )
    {
        printHelp();
        return false;
    }
// read all command line parameters
    while( i++ < argc )
    {
        string iTemp  = argv[i-1];
        string iTemp1 = argv[i-1];                // this is to get the camera name correctly
        string iTemp2 = "";
        if( i < argc )
        {
            iTemp2 = argv[i];
        }
        iTemp = VUtilities::lowerCase( iTemp );
// print help text
        if( iTemp.find( "help" ) < iTemp.size() )
        {
            printHelp();
            return false;
        }
// print target list
        if( iTemp.rfind( "printtarget" ) < iTemp.size() )
        {
            printTargets();
            exit( 0 );
        }
        if( iTemp.find( "debug" ) < iTemp.size() )
        {
            fRunPara->fDebug = true;
            fDebug = true;
        }
        else if( iTemp.find( "photodiode" ) < iTemp.size() )
        {
            fRunPara->fShowPhotoDiode = true;
        }
// camera
        else if( iTemp.find( "camera" ) < iTemp.size() && !(iTemp.find( "cameradirectory" ) < iTemp.size() )
	    && !(iTemp.find( "cameracoordinatetransformx" ) < iTemp.size() )  && !(iTemp.find( "cameracoordinatetransformy" ) < iTemp.size() )  )
        {
// reading iTemp1, to get upper/lower cases right
            fRunPara->fcamera[0] = iTemp1.substr( iTemp1.rfind( "=" )+1, iTemp1.size() );
            fusercamera = true;
        }
	else if( iTemp.find( "vbfnsamples" ) < iTemp.size() )
	{
	    fRunPara->fUseVBFSampleLength = true;
        }
// number of telescopes (>0)
        else if( iTemp.find( "ntel" ) < iTemp.size() )
        {
            fRunPara->fNTelescopes = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
            if( fRunPara->fNTelescopes <= 0 )
            {
                cout << "can only work with more than one telescope" << endl;
                return false;
            }
        }
// display mode
        else if( iTemp.find( "disp" ) < iTemp.size() )
        {
            fRunPara->fdisplaymode = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// muon mode
        else if( iTemp.find( "muon" ) < iTemp.size() )
        {
            fRunPara->fmuonmode = 1;
        }
// Frogs mode
        else if( iTemp.find( "frogs" ) < iTemp.size() )
        {
            fRunPara->ffrogsmode = 1;
	    if( iTemp2.size() > 0 )
	    {
	      fRunPara->ffrogsmscwfile = iTemp2;
              i++;
            }
            else fRunPara->ffrogsmscwfile = "";
        }
// source file
        else if( iTemp.find( "sourcefi" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                fRunPara->fsourcefile = iTemp2;
                i++;
            }
            else    fRunPara->fsourcefile = "";
        }
// pedestal file for grisu simulations
        else if( iTemp.find( "pedestalfile" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                fRunPara->fsimu_pedestalfile = iTemp2;
                i++;
            }
        }
// user name
        else if( iTemp.find( "user" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                fRunPara->fEventDisplayUser = iTemp2;
                i++;
            }
        }
// get laser run number (will exit after printing this)
        else if( iTemp.find( "getlaserrunnumber" ) < iTemp.size() )
	{
	   fGetLaserRunNumber = true;
        }
// file with dead channel definition
        else if( iTemp.find( "deadchannelfile" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                fRunPara->fDeadChannelFile = iTemp2;
                i++;
            }
        }
// noise level of external grisu pedestal file
        else if( iTemp.find( "pedestalnoiselevel" ) < iTemp.size() )
        {
            fRunPara->fsimu_noiselevel = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "pedestaldefaultpedestal" ) < iTemp.size() )
        {
            fRunPara->fsimu_pedestalfile_DefaultPed = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// calibration file
        else if( iTemp.find( "calibrationfi" ) < iTemp.size() && !(iTemp.find( "lowgain" ) < iTemp.size() )  )
        {
            if( iTemp2.size() > 0 )
            {
                fRunPara->fcalibrationfile = iTemp2;
                i++;
            }
            else  fRunPara->fcalibrationfile = "";
        }
	else if( iTemp.find( "lowgaincalibrationfile" ) < iTemp.size() )
	{
	   if( iTemp2.size() > 0 )
	   {
	      fRunPara->fLowGainCalibrationFile = iTemp2;
	      i++;
           }
	   else fRunPara->fLowGainCalibrationFile = "";
        }
        else if( iTemp.find( "arraycuts" ) < iTemp.size() || iTemp.find( "recopara" ) < iTemp.size() || iTemp.find( "reconstructionparameter" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                fRunPara->farrayanalysiscutfile = iTemp2;
                i++;
            }
            else  fRunPara->farrayanalysiscutfile = "";
        }
// dst output file
        else if( iTemp.find( "dstfile" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                fRunPara->fdstfile = iTemp2;
                i++;
            }
            else    fRunPara->fdstfile = "dstfile.root";
        }
// write all pixel to dst file
        else if( iTemp.find( "dstallpixel" ) < iTemp.size() )
        {
            fRunPara->fdstwriteallpixel  = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// minimal number of tubes for dst event
        else if( iTemp.find( "dstntubes" ) < iTemp.size() )
        {
            fRunPara->fdstminntubes = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// calculate pedestal variations on a short time scale for tracking tests
        else if( iTemp.find( "pedestalsintimeslices" ) < iTemp.size() && !(iTemp.find( "pedestalsintimeslicessumwindow" ) < iTemp.size() ) 
	    && !(iTemp.find( "pedestalsintimeslicessumfirst" ) < iTemp.size() )
	    && !( iTemp.find( "usepedestalsintimeslices" ) < iTemp.size() )
	    && !(iTemp.find( "usepedestalsintimesliceslowgain" ) < iTemp.size() ) )
        {
            fRunPara->fPedestalsInTimeSlices = true;
            fRunPara->fUsePedestalsInTimeSlices = true;
            fRunPara->fLowGainUsePedestalsInTimeSlices = true;
        }
        else if( iTemp.find( "usepedestalsintimeslices" ) < iTemp.size() && !(iTemp.find( "usepedestalsintimesliceslowgain" ) < iTemp.size() ) )
        {
            fRunPara->fUsePedestalsInTimeSlices = bool( atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() ) );
        }
        else if( iTemp.find( "usepedestalsintimesliceslowgain" ) < iTemp.size() )
        {
            fRunPara->fLowGainUsePedestalsInTimeSlices = bool( atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() ) );
        }
// parameter for pedestal variations on a short time scale for tracking tests
        else if( iTemp.find( "pedestalslengthoftimeslice" ) < iTemp.size() )
        {
            fRunPara->fPedestalsLengthOfTimeSlice = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
            fRunPara->fUsePedestalsInTimeSlices = true;
            fRunPara->fLowGainUsePedestalsInTimeSlices = true;
        }
        else if( iTemp.find( "pedestalsintimeslicessumwindow" ) < iTemp.size() && iTemp != "pedestalsintimeslices" )
        {
            fRunPara->fPedestalsInTimeSlicesSumWindow = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "pedestalsintimeslicessumfirst" ) < iTemp.size() && iTemp != "pedestalsintimeslices" )
        {
            fRunPara->fPedestalsInTimeSlicesSumFirst = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// fast plotting without pedestals
        else if( iTemp.find( "plotraw" ) < iTemp.size() )
        {
            fRunPara->fPlotRaw = true;
	    fRunPara->fuseDB = false;
	    fRunPara->fDBTracking = false;
	    fRunPara->fTargetName = "laser";
	    fRunPara->fdisplaymode = 1;
        }
        else if( iTemp.find( "plotmethod" ) < iTemp.size() )
        {
            fRunPara->fPlotAllInOneMethod = (unsigned int)atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "writeallmc" ) < iTemp.size() )
        {
            fRunPara->fWriteTriggerOnly = false;
        }
        else if( iTemp.find( "writenomctree" ) < iTemp.size() )
        {
            fRunPara->fwriteMCtree = 0;
        }
	else if( iTemp.find( "fillmchisto" ) < iTemp.size() )
	{
	    fRunPara->fFillMCHistos = true;
        }
// use db for run infos
        else if( iTemp.find( "usedbinfo" ) < iTemp.size() && !(iTemp.find( "donotusedbinfo" ) < iTemp.size()) )
        {
            fRunPara->fuseDB = true;
            isCompiledWithDB();
        }
        else if( iTemp.find( "donotusedbinfo" ) < iTemp.size() )
        {
            fRunPara->fuseDB = false;
        }
        else if( iTemp.find( "usefixedthresholds" ) < iTemp.size() )
        {
            fRunPara->fUseFixedThresholds = true;
        }
        else if( iTemp.find( "usesignalnoisethresholds" ) < iTemp.size() )
        {
            fRunPara->fUseFixedThresholds = false;
        }
        else if( iTemp.find( "usetimecleaning" ) < iTemp.size() )  // HP
        {
            fRunPara->fUseTimeCleaning = true;
        }
// ignore configuration file versions
        else if( iTemp.find( "ignorecfgversions" ) < iTemp.size() )
        {
            fRunPara->fIgnoreCFGversions = true;
        }
// print analysis progress
        else if( iTemp.find( "printanalysisprogress" ) < iTemp.size() || iTemp.find( "pap" ) < iTemp.size() )
        {
            fRunPara->fPrintAnalysisProgress = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// nice plots for talks/papers
        else if( iTemp.find( "plotpaper" ) < iTemp.size() )
        {
            fRunPara->fPlotPaper = true;
        }
// set run number from command line (default: get it from source file name)
        else if( iTemp.rfind( "runnum" ) < iTemp.size() )
        {
            fRunPara->frunnumber = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.rfind( "gaincorrection" ) < iTemp.size() )
        {
            fRunPara->fGainCorrection[0] = atof(  iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// which telescope should be analyzed
        else if( iTemp.rfind( "teltoana" ) < iTemp.size() )
        {
            fTelToAnaString = iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() );
// comma separated list
            if( fTelToAnaString.find( "," ) < fTelToAnaString.size() ) fTelToAna = 0;
            else                                                       fTelToAna=(unsigned int)(atoi( fTelToAnaString.c_str() ) );
        }
// elevation
        else if( iTemp.rfind( "elevat" ) < iTemp.size() )
        {
            fRunPara->felevation= atof(iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// azimuth
        else if( iTemp.rfind( "azimuth" ) < iTemp.size() )
        {
            fRunPara->fazimuth = atof(iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// target declination
        else if( iTemp.rfind( "declination" ) < iTemp.size() )
        {
            fRunPara->fTargetDec = atof(iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// target ra
        else if( iTemp.rfind( "rightascension" ) < iTemp.size() )
        {
            fRunPara->fTargetRA = atof(iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// target declination offset
        else if( iTemp.rfind( "decoffset" ) < iTemp.size() )
        {
            fRunPara->fTargetDecOffset = atof(iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// target ra offset
        else if( iTemp.rfind( "raoffset" ) < iTemp.size() )
        {
            fRunPara->fTargetRAOffset = atof(iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// target name
        else if( iTemp.rfind( "target" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                fRunPara->fTargetName = iTemp2;
                i++;
            }
            else  fRunPara->fTargetName = "";
        }
// wobble offset NORTH
        else if( iTemp.rfind( "wobblenorth" ) < iTemp.size() )
        {
            fRunPara->fWobbleNorth = atof(iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// wobble offset EAST
        else if( iTemp.rfind( "wobbleeast" ) < iTemp.size() )
        {
            fRunPara->fWobbleEast = atof(iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// pointing check
        else if( iTemp.rfind( "checkpointing" ) < iTemp.size() )
        {
            fRunPara->fCheckPointing  = atof(iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// pointing error x
        else if( iTemp.rfind( "-pointingerrorx" ) < iTemp.size() )
        {
            unsigned int iTelID = 0;
            double ix = 0.;
            iTelID = (unsigned int) atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.rfind( ":" ) ).c_str() ) - 1;
            ix = atof( iTemp.substr( iTemp.rfind( ":" )+1, iTemp.size() ).c_str() );
            f_pointingErrorX[iTelID] = ix;
            fRunPara->fDBTracking = false;
        }
// pointing error y
        else if( iTemp.rfind( "-pointingerrory" ) < iTemp.size() )
        {
            unsigned int iTelID = 0;
            double ix = 0.;
            iTelID = (unsigned int) atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.rfind( ":" ) ).c_str() ) - 1;
            ix = atof( iTemp.substr( iTemp.rfind( ":" )+1, iTemp.size() ).c_str() );
            f_pointingErrorY[iTelID] = ix;
            fRunPara->fDBTracking = false;
        }
        else if( iTemp.rfind( "-usedbtracking" ) < iTemp.size() )
        {
            fRunPara->fDBTracking = true;
            isCompiledWithDB();
        }
        else if( iTemp.rfind( "-usenodbtracking" ) < iTemp.size() )
        {
            fRunPara->fDBTracking = false;
        }
        else if( iTemp.rfind( "-usedbvpm" ) < iTemp.size() ) 
	{
            fRunPara->fDBVPM = true;
            fRunPara->fDBTracking = true;
        }
        else if( iTemp.rfind( "-usenodbvpm" ) < iTemp.size() ) 
	{
            fRunPara->fDBVPM = false;
            fRunPara->fDBTracking = true;
        }
	else if( iTemp.rfind( "-usedbrotations" ) < iTemp.size() )
	{
	    fRunPara->fDBCameraRotationMeasurements = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.rfind( "-usetcorrectionfrom" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                fRunPara->fDBTrackingCorrections = iTemp2;
                fRunPara->fDBTracking = true;
                i++;
            }
            else
            {
                fRunPara->fDBTrackingCorrections = "";
                cout << "no date for T-Point correction given" << endl;
                return false;
            }
        }
        else if( iTemp.rfind( "-pointingmonitortxt" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                fRunPara->fPMTextFileDirectory = iTemp2;
                fRunPara->fDBTracking = true;
                i++;
            }
            else
            {
                fRunPara->fPMTextFileDirectory = "";
                cout << "no pointing monitor text file directory give" << endl;
                return false;
            }
        }
// min laser charge
        else if( iTemp.rfind( "lasermin" ) < iTemp.size() )
        {
            fRunPara->fLaserSumMin = (double)atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
// run mode
        else if( iTemp.rfind( "mod" ) < iTemp.size() )
        {
            fRunPara->frunmode = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
            if( fRunPara->frunmode < 0 || fRunPara->frunmode > 6 )
            {
                cout << "invalid rundmode (0=analysis,1=pedestal,2=gains/toffsets,3=trace library,4=dst output,5=gains/toffsets low gain,6=low gain pedestals)" << endl;
                return false;
            }
        }
// number of events
        else if( iTemp.rfind( "neve" ) < iTemp.size() )
        {
            fRunPara->fnevents = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
            if( fRunPara->fnevents <= 0 ) fRunPara->fnevents = -10000;
        }
// source type
        else if( iTemp.find( "type" ) < iTemp.size() )
        {
            fRunPara->fsourcetype = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
            if( fRunPara->fsourcetype < 0 || fRunPara->fsourcetype > 7 )
            {
                cout << "unknown sourcetype " << fRunPara->fsourcetype << endl;
                return false;
            }
// MC grisu file
            if( fRunPara->fsourcetype == 1 || fRunPara->fsourcetype == 2 
	     || fRunPara->fsourcetype == 5 || fRunPara->fsourcetype == 6 
	     || fRunPara->fsourcetype == 7 ) 
	     {
	        fRunPara->fIsMC = 1;
             }
        }
// fill some diagnostic histograms
        else if( iTemp.find( "fill" ) < iTemp.size() )
        {
            cout << "Warning: Parameter -fillhistos obsolete" << endl;
//          fRunPara->ffillhistos = true;
        }
        else if( iTemp.find( "image" ) < iTemp.size() )
        {
            fRunPara->fimagethresh[0] = atof( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "border" ) < iTemp.size() )
        {
            fRunPara->fborderthresh[0] = atof( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "brightnonimage" ) < iTemp.size() )
        {
            fRunPara->fbrightnonimagetresh[0] = atof( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
	else if( iTemp.find( "timecutpixel" ) < iTemp.size() )  //HP
	{
	    fRunPara->ftimecutpixel[0] = atof( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
	}
	else if( iTemp.find( "timecutcluster" ) < iTemp.size() )  //HP
	{
	    fRunPara->ftimecutcluster[0] = atof( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
	}
	else if( iTemp.find( "minpixelcluster" ) < iTemp.size() )  //HP
	{
	    fRunPara->fminpixelcluster[0] = atoi( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
	}
	else if( iTemp.find( "loops" ) < iTemp.size() )  //HP
	{
	    fRunPara->floops[0] = atoi( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
	}
        else if( iTemp.find( "sumfirst" ) < iTemp.size() || iTemp.find( "sumstart" ) < iTemp.size() )
        {
            fRunPara->fsumfirst[0] = atoi( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "sumwindow" ) < iTemp.size() && !(iTemp.find( "sumwindow_doublepass" ) < iTemp.size()) )
        {
            fRunPara->fsumwindow[0] = atoi( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "sumwindow_doublepass" ) < iTemp.size() )
        {
            fRunPara->fsumwindowsmall[0] = atoi( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "tracewindowshift" ) < iTemp.size() )
        {
            fRunPara->fTraceWindowShift[0] = atoi( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
	else if( iTemp.find( "mc_fadctracestart" ) < iTemp.size() )
	{
	    fRunPara->fMC_FADCTraceStart = (unsigned int)atoi( iTemp.substr(iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
	}
	else if( iTemp.find( "traceanalysis" ) < iTemp.size() )
	{
	    fRunPara->fperformFADCAnalysis = (bool)atoi( iTemp.substr(iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "tracedefinesmallpulse" ) < iTemp.size() )
        {
            fRunPara->fSumWindowStartAtT0Min = atoi( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "cameracoordinatetransformx" ) < iTemp.size() )
        {
            fRunPara->fCameraCoordinateTransformX = atof( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "cameracoordinatetransformy" ) < iTemp.size() )
        {
            fRunPara->fCameraCoordinateTransformY = atof( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "loop" ) < iTemp.size() )
        {
            fRunPara->floopmode = true;
        }
        else if( iTemp.find( "printoutputfile" ) < iTemp.size() )
        {
            fPrintOutputFile = true;
        }
	else if( iTemp.find( "outputdirectory" ) < iTemp.size() )
	{
	   if( iTemp2.size() > 0 )
	   {
	      fRunPara->setDirectory_EVNDISPOutput( iTemp2 );
	      i++;
           }
        }
// analysis output file
        else if( iTemp.find( "output" ) < iTemp.size() && iTemp != "outputdirectory" && iTemp != "printoutputfile" )
        {
            if( iTemp2.size() > 0 )
            {
                fRunPara->foutputfileName = iTemp2;
                i++;
            }
            else                    fRunPara->foutputfileName = "";
        }
        else if( iTemp.find( "high" ) < iTemp.size() )
        {
            fRunPara->fh = 833;
            fRunPara->fw = 1250;
            fRunPara->fh = (unsigned int)(833*1.5);
            fRunPara->fw = (unsigned int)(1250*1.5);
        }
        else if( iTemp.find( "telenoff" ) < iTemp.size() )
        {
            fRunPara->ftelescopeNOffset = atoi( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "sampleoff" ) < iTemp.size() )
        {
            fRunPara->fsampleoffset = atoi( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "logl" ) < iTemp.size() && !( iTemp.find( "loglminloss" ) < iTemp.size() ) )
        {
            fRunPara->fImageLL = atoi( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "loglminloss" ) < iTemp.size() )
        {
            fRunPara->fLogLikelihoodLoss_min = atof( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "fuifactor" ) < iTemp.size() )
        {
            fRunPara->fImageAnalysisFUIFactor  = atof( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "doublepass" ) < iTemp.size() && !(iTemp.find( "nodoublepass" ) < iTemp.size()) )
        {
            fRunPara->fDoublePass = true;
        }
        else if( iTemp.find( "nodoublepass" ) < iTemp.size() )
        {
            fRunPara->fDoublePass = false;
        }
        else if( iTemp.find( "removeisolatedpixel" ) < iTemp.size() )
        {
            fRunPara->fRemoveIsolatedPixel = atoi( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
            i++;
        }
        else if( iTemp.find( "fixwindowstart" ) < iTemp.size() && !(iTemp.find( "nofixwindowstart" ) < iTemp.size()) )
        {
            fRunPara->fFixWindowStart = true;
        }
        else if( iTemp.find( "nofixwindowstart" ) < iTemp.size() )
        {
            fRunPara->fFixWindowStart = false;
        }
        else if( iTemp.find( "smoothdead" ) < iTemp.size() )
        {
            fRunPara->fSmoothDead = true;
        }
        else if( iTemp.find( "usepeds" ) < iTemp.size() && !(iTemp.find( "donotusepeds" ) < iTemp.size() ) )
        {
            fRunPara->fUsePedEvents = true;
        }
	else if( iTemp.find( "donotusepeds" ) < iTemp.size() )
	{
            fRunPara->fUsePedEvents = false;
        }
        else if( iTemp.find( "l2timecorrect" ) < iTemp.size() )
        {
            fRunPara->fL2TimeCorrect = atoi( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "l2setspecialchannels" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                fRunPara->fsetSpecialChannels = iTemp2;
                if( fRunPara->fsetSpecialChannels == "nofile" ) fRunPara->fsetSpecialChannels = "";
                i++;
            }
        }
        else if( iTemp.find( "tracelib" ) < iTemp.size() )
        {
            if( iTemp2.size() > 0 )
            {
                fRunPara->ftracefile = iTemp2;
                i++;
            }
            else  fRunPara->ftracefile = "";
        }
        else if( iTemp.find( "tracefit" ) < iTemp.size() )
        {
            fRunPara->ftracefit = atof( iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "fitfun" ) < iTemp.size() )
        {
            fRunPara->ftracefitfunction = iTemp.substr( iTemp.rfind( "=" )+1,iTemp.size() );
        }
        else if( iTemp.find( "ignoredead" ) < iTemp.size() )
        {
            fRunPara->fMCnoDead = true;
        }
        else if( iTemp.find( "ndeadchannel" ) < iTemp.size() )
        {
            fRunPara->fMCNdead = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "seedndead" ) < iTemp.size() )
        {
            fRunPara->fMCNdeadSeed = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "ndeadboard" ) < iTemp.size() )
        {
            fRunPara->fMCNdeadboard = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "writepulses" ) < iTemp.size() )
        {
            fRunPara->fwritepulses = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "fadcscale" ) < iTemp.size() )
        {
            fRunPara->fMCScale = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "pedestalseed" ) < iTemp.size() )
        {
            fRunPara->fgrisuseed = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "ismc" ) < iTemp.size() )
        {
            fRunPara->fIsMC = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( iTemp.find( "shorttree" ) < iTemp.size() )
        {
            fRunPara->fShortTree = 1;
        }
                                                  // MS
        else if( iTemp.find( "pwmethod" ) < iTemp.size() )
        {
            fRunPara->fPWmethod = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
                                                  // MS
        else if( iTemp.find( "pwcleanneighbors" ) < iTemp.size() )
        {
            fRunPara->fPWcleanNeighbors = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
                                                  // MS
        else if( iTemp.find( "pwcleanthreshold" ) < iTemp.size() )
        {
            fRunPara->fPWcleanThreshold = atof( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
                                                  // MS
        else if( iTemp.find( "pwlimit" ) < iTemp.size() )
        {
            fRunPara->fPWlimit = atoi( iTemp.substr( iTemp.rfind( "=" )+1, iTemp.size() ).c_str() );
        }
        else if( i > 1 )
        {
            cout << "unknown run parameter: " << iTemp << endl;
            cout << "exiting..." << endl;
            exit( -1 );
        }
    }

// test and adjust some parameters
    test_and_adjustParams();

    return true;
}


/*!
   \attention
      using different cameras for prototype data for runs < 592 and > 592 as default (no user camera)
*/
void VReadRunParameter::test_and_adjustParams()
{
    if( fDebug ) cout << "VReadRunParameter::test_and_adjustParams()" << endl;

// test if this is a DST file
    getRunParametersFromDST();
// set pulse timing for DST case
    if( fRunPara->frunmode == 4 )
    {
       fRunPara->fpulsetiminglevels.clear();
       fRunPara->fpulsetiminglevels.push_back( 0.2 );
       fRunPara->fpulsetiminglevels.push_back( 0.5 );
       fRunPara->fpulsetiminglevels.push_back( 0.8 );
       fRunPara->fpulsetiminglevels.push_back( 1.0 );
       unsigned int i_fps = fRunPara->fpulsetiminglevels.size();
       if( i_fps > 1 )
       {
	  for( unsigned int i = 0; i < i_fps - 1; i++ )
	  {
	       fRunPara->fpulsetiminglevels.push_back( fRunPara->fpulsetiminglevels[i_fps-i-2] );
	  }
       } 
// calculate tzero index and width index
       fRunPara->setPulseZeroIndex();
    }

   fRunPara->setSystemParameters();

/////////////////////////////////////////////////////////////////
// MC adjustments
    if( fRunPara->fIsMC > 0 )
    {
        fRunPara->fDBTracking = false;
        fRunPara->fuseDB = false;
	fRunPara->fL2TimeCorrect = false;
	fRunPara->fDBCameraRotationMeasurements = false;
    }

// CTA/AGIS adjustments
    if( fRunPara->getObservatory().find( "cta" ) != string::npos || fRunPara->getObservatory().find( "CTA" ) != string::npos || fRunPara->getObservatory().find( "agis" ) != string::npos || fRunPara->getObservatory().find( "AGIS" ) != string::npos )
    {
       fRunPara->fsetSpecialChannels = "";
       fRunPara->fDeadChannelFile = "";
    }

////////////////////////////////////////////////////
// get runnumbers from file name (if source file is given)
    if( fRunPara->frunnumber <= 0 )
    {
        if( fRunPara->fsourcefile.find( "pure" ) < fRunPara->fsourcefile.size() )
        {
            fRunPara->frunnumber = atoi( fRunPara->fsourcefile.substr( fRunPara->fsourcefile.rfind( "/" ) + 1, fRunPara->fsourcefile.rfind( "." ) ).c_str() );
        }
// MC (grisu)
        else if ( fRunPara->fsourcefile.find( "grisu" ) < fRunPara->fsourcefile.size() )
        {
            fRunPara->frunnumber = atoi( fRunPara->fsourcefile.substr( fRunPara->fsourcefile.rfind( "/" ) + 4, 6 ).c_str() );
        }
// vbf
        else if ( fRunPara->fsourcefile.find( "vbf" ) < fRunPara->fsourcefile.size() || fRunPara->fsourcefile.find( "cvbf" ) < fRunPara->fsourcefile.size() )
        {
            fRunPara->frunnumber = atoi( fRunPara->fsourcefile.substr( fRunPara->fsourcefile.rfind( "/" ) + 1, fRunPara->fsourcefile.rfind( "." ) ).c_str() );
            if (fRunPara->fsourcetype==0) fRunPara->fsourcetype=3;
        }
// dst
        else if ( fRunPara->fsourcefile.find( "dst" ) < fRunPara->fsourcefile.size() )
        {
            fRunPara->frunnumber = atoi( fRunPara->fsourcefile.substr( fRunPara->fsourcefile.rfind( "/" ) + 4, 6 ).c_str() );
        }
        else
        {
            fRunPara->frunnumber = atoi( fRunPara->fsourcefile.substr( fRunPara->fsourcefile.rfind( "/" ) + 1, fRunPara->fsourcefile.size() ).c_str() );
        }
    }
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// settings for pedestal and gain calculations
    if( fRunPara->frunmode == 1 || fRunPara->frunmode == 2 || fRunPara->frunmode == 5 || fRunPara->frunmode == 6  )
    { 
        fRunPara->fcalibrationfile = "";
        fRunPara->fDBTracking = false;
//        fRunPara->fuseDB = false;
	fRunPara->fcalibrationrun = true;
    }
    if( fRunPara->frunmode != 1 && fRunPara->frunmode != 6 ) fRunPara->fPedestalsInTimeSlices = false;
// low gain pedestals: ignore sample length in cfg file
    if( fRunPara->frunmode == 6 )
    {
       fRunPara->fUseVBFSampleLength = true;
       fRunPara->fPedestalsInTimeSlices = false;
       fRunPara->fUsePedestalsInTimeSlices = false;
       fRunPara->fLowGainUsePedestalsInTimeSlices = false;
    }
// use fixed image/border thresholds for the case of no pedvars in the DSTs
    if( fRunPara->fCalibrationDataType == 0 )
    {
            fRunPara->fUseFixedThresholds = true;
    }

/////////////////////////////////////////////////////////////////
// set vector sizes for calibration numbers
    for( unsigned int i = 0; i < fRunPara->fNTelescopes; i++ )
    {
        if( i >= fRunPara->fGainFileNumber.size() ) fRunPara->fGainFileNumber.push_back( 0 );
        if( i >= fRunPara->fTOffFileNumber.size() ) fRunPara->fTOffFileNumber.push_back( 0 );
        if( i >= fRunPara->fPedFileNumber.size() ) fRunPara->fPedFileNumber.push_back( fRunPara->frunnumber );   // default: take peds from same run
	else                                       fRunPara->fPedFileNumber[i] = fRunPara->frunnumber;
        if( !fRunPara->fPlotRaw )
	{
// writing low gain pedestals
	   if( fRunPara->frunmode == 6 ) 
	   {
	      if( i >= fRunPara->fPedLowGainFileNumber.size() ) fRunPara->fPedLowGainFileNumber.push_back( fRunPara->frunnumber );
	      else                                              fRunPara->fPedLowGainFileNumber[i] = fRunPara->frunnumber;
           }
// reading low gain pedestals
	   else
	   {
//	      if( i >= fRunPara->fPedLowGainFileNumber.size() ) fRunPara->fPedLowGainFileNumber.push_back( 36862 );
//	      else                                              fRunPara->fPedLowGainFileNumber[i] = 36862;
	      if( i >= fRunPara->fPedLowGainFileNumber.size() ) fRunPara->fPedLowGainFileNumber.push_back( 0 );
	      else                                              fRunPara->fPedLowGainFileNumber[i] = 0;
           }
        }
	if( i >= fRunPara->fLowGainMultiplierFileNumber.size() ) fRunPara->fLowGainMultiplierFileNumber.push_back( 0 );
        if( i >= fRunPara->fGainLowGainFileNumber.size() ) fRunPara->fGainLowGainFileNumber.push_back( 0 );
        if( i >= fRunPara->fTOffLowGainFileNumber.size() ) fRunPara->fTOffLowGainFileNumber.push_back( 0 );
        if( i >= fRunPara->fPixFileNumber.size() ) fRunPara->fPixFileNumber.push_back( 0 );
        if( i >= fRunPara->fPadFileNumber.size() ) fRunPara->fPadFileNumber.push_back( 0 );
    }
// gain and toff calculation: set output run numbers
    if( fRunPara->frunmode == 2 || fRunPara->frunmode == 5 )
    {
       for( unsigned int i = 0; i < fRunPara->fNTelescopes; i++ )
       {
           fRunPara->fGainFileNumber[i] = fRunPara->frunnumber;
	   fRunPara->fGainLowGainFileNumber[i] = fRunPara->frunnumber;
	   fRunPara->fTOffFileNumber[i] = fRunPara->frunnumber;
	   fRunPara->fTOffLowGainFileNumber[i] = fRunPara->frunnumber;
       }
     }
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// read run info from database
    if( fRunPara->fuseDB )
    {
        VDBRunInfo i_DBinfo( fRunPara->frunnumber, fRunPara->getDBServer(), fRunPara->fNTelescopes );
        if( i_DBinfo.isGood() )
        {
            fRunPara->fTargetName = i_DBinfo.getTargetName();
            fRunPara->fTargetDec = i_DBinfo.getTargetDec();
            fRunPara->fTargetRA = i_DBinfo.getTargetRA();
            fRunPara->fWobbleNorth = i_DBinfo.getWobbleNorth();
            fRunPara->fWobbleEast = i_DBinfo.getWobbleEast();
	    fRunPara->fDBRunType = i_DBinfo.getRunType();
	    fRunPara->fDBRunStartTimeSQL = i_DBinfo.getDataStartTimeSQL();
	    fRunPara->fDBRunStoppTimeSQL = i_DBinfo.getDataStoppTimeSQL();
            if( fTelToAna == 0 ) fTelToAna = i_DBinfo.getTelToAna();
// get source file from run number (if run number is given and no sourcefile)
	    if( fRunPara->fsourcefile.size() < 1 )
	    {
	       char iname[5000];
// set raw data directories
	       if( fRunPara->getDirectory_VBFRawData().size() > 0 )
	       {
		  sprintf( iname, "%s/d%d/%d.cvbf", fRunPara->getDirectory_VBFRawData().c_str(), i_DBinfo.getRunDate(), fRunPara->frunnumber );
	       }
	       else
	       {
		  sprintf( iname, "data/d%d/%d.cvbf", i_DBinfo.getRunDate(), fRunPara->frunnumber );
		  cout << iname << endl;
	       }
	       fRunPara->fsourcefile = iname;
            }

// get laser runs
            if( fRunPara->frunmode != 2 && fRunPara->frunmode != 5 )
	    {
	       vector< unsigned int > iL = i_DBinfo.getLaserRun();
	       if( iL.size() != fRunPara->fNTelescopes )
	       {
		  cout << "VReadRunParameter::test_and_adjustParams() error: list of laser file has wrong length " << iL.size() << "\t" << fRunPara->fNTelescopes << endl;
		  exit( -1 );
	       }
	       else
	       {
		  for( unsigned int i = 0; i < iL.size(); i++ )
		  {
		     fRunPara->fGainFileNumber[i] = (int)iL[i];
		     fRunPara->fTOffFileNumber[i] = (int)iL[i];
		  }
	       }
	       if( fGetLaserRunNumber )
	       {
		 if( fRunPara->fTelToAnalyze.size() == 1 ) 
		 {
//		    if( fRunPara->fTelToAnalyze[0] < iL.size() ) cout << iL[fRunPara->fTelToAnalyze[0]] << endl;
// GH edit
		    if( fRunPara->fTelToAnalyze[0] < iL.size() ) cout << iL[fTelToAna-1] << endl;
		 }
		 else
		 {
		    cout << "only one telescope allowed for getlaserrunnumber" << endl;
		 }
		 exit( 0 );
	      }	      
            }

            i_DBinfo.print();
        }
        else
        {
            cout << endl;
            cout << "FATAL ERROR: cannot connect to VERITAS database" << endl;
            cout << "exiting..." << endl;
            exit( 0 );
        }
    }

/////////////////////////////////////////////////////////////////
// check if sourcefile is given
    if( fRunPara->fsourcefile.size() < 1 )
    {
        cout << "error: no sourcefile (test failed in VReadRunParameter)" << endl;
	cout << "possible reasons:" << endl;
	cout << "\t - source file does not exist" << endl;
	cout << "\t - eventdisplay was compiled without database support and couldn't find corresponding data directory" << endl;
	cout << "exit...." << endl;
        exit( 0 );
    }

    if ( fRunPara->fsourcefile.find( "vbf" ) < fRunPara->fsourcefile.size() && fRunPara->fsourcetype==0 )
    {
        fRunPara->fsourcetype=3;
    }

// bz2 and gz reading does not work for vbf files; dont allow
    if ( fRunPara->fsourcefile.find( "vbf" ) < fRunPara->fsourcefile.size() )
    {
       if( fRunPara->fsourcefile.find( ".gz" ) < fRunPara->fsourcefile.size() || fRunPara->fsourcefile.find( ".bz2" ) < fRunPara->fsourcefile.size() )
       {
          cout << "error: cannot read gzipped or bzipped files" << endl;
	  exit( 0 );
       }
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// can't apply T-point corrections to pointing monitor data
    if( fRunPara->fDBTrackingCorrections.size() > 0 && fRunPara->fPMTextFileDirectory.size() > 0 )
    {
        cout << "error: can't apply T-point corrections to pointing monitor data" << endl;
        cout << "exiting..." << endl;
        exit( -1 );
    }

// switch of display in calibration and dst mode
    if( fRunPara->fdisplaymode == true && fRunPara->frunmode != 0 )
    {
        fRunPara->fdisplaymode = 0;
    }
    if( fRunPara->fdisplaymode ) fRunPara->fWriteTriggerOnly = false;

// fRunPara->fImageLL: values between 0-2
    if( fRunPara->fImageLL < 0 && fRunPara->fImageLL > 2 )
    {
        cout << "warning: logl parameter out of range, setting it to 1" << endl;
        fRunPara->fImageLL = 1;
    }
// check fadc trace scaling
    if( fRunPara->fMCScale <= 0. )
    {
        cout << "errors: fadcscale <= 0" << endl;
        exit( -1 );
    }


// set target name for calibration runs
    if( fRunPara->frunmode == 1 && fRunPara->fTargetName == "NONAME" ) fRunPara->fTargetName = "wy";
    if( fRunPara->frunmode == 6 && fRunPara->fTargetName == "NONAME" ) fRunPara->fTargetName = "wy";
    if( fRunPara->frunmode == 2 && fRunPara->fTargetName == "NONAME" ) fRunPara->fTargetName = "laser";
    if( fRunPara->frunmode == 5 && fRunPara->fTargetName == "NONAME" ) fRunPara->fTargetName = "laser";

// remove .cam if present
    for( unsigned int i = 0; i < fRunPara->fcamera.size(); i++ )
    {
        if( fRunPara->fcamera[i].find( ".cam" ) < fRunPara->fcamera[i].size() )
        {
            fRunPara->fcamera[i] = fRunPara->fcamera[i].substr( 0, fRunPara->fcamera[i].find( ".cam" )-1 );
            cout << "adjusted camera name: " << fRunPara->fcamera[i] << endl;
        }
    }

// set default camera configurations
    if( !fusercamera )
    {
       if( fRunPara->frunnumber > 46642 ) fRunPara->fcamera[0] = "veritasBC4N_090916_Autumn2009-4.1.5_EVNDISP.cfg";  // new array
       else                               fRunPara->fcamera[0] = "veritasBC4_090723_Autumn2007-4.1.5_EVNDISP.cfg";   // old array
    }

// set camera file name to dstfile for dst reading
    if( fRunPara->fsourcetype == 4 || fRunPara->fsourcetype == 7 )
    {
        for( unsigned int t = 0; t < fRunPara->fcamera.size(); t++ ) fRunPara->fcamera[t] = "dstfile";
    }

    setDirectories();

// for command line case, all parameters are the same for all telescopes
// !preli!
// this will go with a configuration file or connection to a database
    if( f_boolCommandline )
    {
// start at 1, parameters for first telescope are filled by default
        for( unsigned int i = 1; i < fRunPara->fNTelescopes; i++ )
        {
            fRunPara->fcamera.push_back( fRunPara->fcamera[0] );
            fRunPara->fimagethresh.push_back( fRunPara->fimagethresh[0] );
            fRunPara->fGainCorrection.push_back( fRunPara->fGainCorrection[0] );
            fRunPara->fborderthresh.push_back( fRunPara->fborderthresh[0] );
            fRunPara->fbrightnonimagetresh.push_back( fRunPara->fbrightnonimagetresh[0] );
            fRunPara->ftimecutpixel.push_back( fRunPara->ftimecutpixel[0] );
            fRunPara->ftimecutcluster.push_back( fRunPara->ftimecutcluster[0] );
            fRunPara->fminpixelcluster.push_back( fRunPara->fminpixelcluster[0] );
            fRunPara->floops.push_back( fRunPara->floops[0] );
            fRunPara->fsumwindow.push_back( fRunPara->fsumwindow[0] );
            fRunPara->fsumwindowsmall.push_back( fRunPara->fsumwindowsmall[0] );
            fRunPara->fsumfirst.push_back( fRunPara->fsumfirst[0] );
            fRunPara->fTraceWindowShift.push_back( fRunPara->fTraceWindowShift[0] );
            fRunPara->fDBSumWindowMaxTimedifference.push_back( fRunPara->fDBSumWindowMaxTimedifference[0] );
        }
    }

// set pointing errors
    fRunPara->fPointingErrorX.clear();
    fRunPara->fPointingErrorY.clear();
    for( unsigned int i = 0; i < fRunPara->fNTelescopes; i++ )
    {
        if( f_pointingErrorX.find( i ) != f_pointingErrorX.end() ) fRunPara->fPointingErrorX.push_back( f_pointingErrorX[i] );
        else                                                       fRunPara->fPointingErrorX.push_back( 0. );
        if( f_pointingErrorY.find( i ) != f_pointingErrorY.end() ) fRunPara->fPointingErrorY.push_back( f_pointingErrorY[i] );
        else                                                       fRunPara->fPointingErrorY.push_back( 0. );
    }

// set telescope to analyse
//   1,2,3 = Telescope 1,2,3
//   12    = Telescope 1,2
//   23    = Telescope 2,3
//
    fRunPara->fTelToAnalyze.clear();
    if( fTelToAna != 0 && fRunPara->fNTelescopes < 10 && fTelToAna > 10 )
    {
        unsigned int iM = 10;
        for( int i = 0; i < 10; i++ )
        {
            unsigned int tt = (fTelToAna%(iM))/(iM/10);
            if( tt == 0 ) break;
            fRunPara->fTelToAnalyze.push_back( tt-1 );
            iM *= 10;
        }
        sort( fRunPara->fTelToAnalyze.begin(), fRunPara->fTelToAnalyze.end() );
    }
    else if( fTelToAna != 0 && fTelToAna < 10 )
    {
        fRunPara->fTelToAnalyze.push_back( fTelToAna-1 );
    }
    else if( fTelToAna == 0 && fTelToAnaString.size() > 0 )
    {
        int pos_prev = -1;
        size_t pos_curr = 0;
        vector< int > comma_positions;
        comma_positions.push_back(-1);
        while( (pos_curr = fTelToAnaString.find(",", pos_prev + 1)) < fTelToAnaString.size() )
        {
            comma_positions.push_back(pos_curr);
            pos_prev = pos_curr;
        }
        for( size_t i = 0; i < comma_positions.size(); i++ )
        {
            string tmpstr = fTelToAnaString.substr( comma_positions[i]+1, (i == comma_positions.size() - 1) ? fTelToAnaString.size() : comma_positions[i + 1] - comma_positions[i] - 1);
            fRunPara->fTelToAnalyze.push_back( atoi(tmpstr.c_str()) - 1 );
        }
        sort( fRunPara->fTelToAnalyze.begin(), fRunPara->fTelToAnalyze.end() );
    }
// nothing set: analyse all telescope
    else
    {
        for( unsigned int i = 0; i < fRunPara->fNTelescopes; i++ ) fRunPara->fTelToAnalyze.push_back( i );
    }

// check if it is possible to analyze the requested telescope
    if( fRunPara->fTelToAnalyze.size() > fRunPara->fNTelescopes )
    {
        cout << "error: can't analyze this telescope ";
        cout << fRunPara->fTelToAnalyze.size() << "\t";
        cout << "(total number of telescopes: " << fRunPara->fNTelescopes << ")" << endl;
    }

    for( unsigned int i = 0; i < fRunPara->fTelToAnalyze.size(); i++ )
    {
        if( fRunPara->fTelToAnalyze[i] > fRunPara->fNTelescopes )
        {
            cout << "telescope number to analyze out of range " << fRunPara->fTelToAnalyze[i] << "\t" << fRunPara->fNTelescopes << endl;
            exit( -1 );
        }
    }

// gain/toff only per telescope
    if( ( fRunPara->fTelToAnalyze.size() > 1 || fRunPara->fTelToAnalyze.size() == 0 ) && (fRunPara->frunmode == 2 || fRunPara->frunmode == 5 ) )
    {
        cout << fRunPara->fTelToAnalyze.size() << "\t" << fRunPara->frunmode << endl;
        cout << "gain/toff calibration only possible for one telescope, use teltoana command line parameter" << endl;
        exit( -1 );
    }

// double pass and fixed window start is incompatible
    if( fRunPara->fDoublePass && fRunPara->fFixWindowStart )
    {
        cout << "doublepass and fixed position of integration window is incompatible!" << endl;
        cout << "...exiting" << endl;
        exit( -1 );
    }
// set trace window shift to zero for fixed window start
    if( fRunPara->fFixWindowStart )
    {
        for( unsigned int t = 0; t < fRunPara->fTraceWindowShift.size(); t++ ) fRunPara->fTraceWindowShift[t] = 0;
    }

    if( fRunPara->frunmode == 1 || fRunPara->frunmode == 6 )
    {
        if( fRunPara->fsumwindow.size() > 0 ) fRunPara->fPedestalsInTimeSlicesSumWindow = fRunPara->fsumwindow[0];
    }

/*   if( fRunPara->fUsePedEvents && fRunPara->fsourcetype != 3 )
   {
      fRunPara->fUsePedEvents = false;
      cout << "Warning: pedestal events only work with the VBF file reader" << endl;
      cout << "         Check your file is vbf and specify sourcetype=3" << endl;
   } */

// MS: throws an error if its a simulation file and it was asked to calculate the PW parameters from the CFD hits, since CFDs don't exist in the simulation record
    if( fRunPara->fIsMC > 0 && ( fRunPara->fPWmethod == 0 || fRunPara->fPWmethod == 1) )
    {
        cout <<" GrISU simulations don't have true CFD hits! PWmethod="<<fRunPara->fPWmethod<<" can only be used with data with CFD hits."<<endl;
        cout <<" Switching to FADC signals cleaned with PWcleanThreshold!!  "<<endl<<endl;
    }

    if( fRunPara->fPWmethod > 3 )
    {
        cout << "Incorrect method for calculating binary-image trigger map: "<<fRunPara->fPWmethod<<endl;
        exit( -1 );
    }
    if( fRunPara->fPWcleanNeighbors < 0 )
    {
        cout << "Incorrect number of nearest neighbors required for cleaning: "<<fRunPara->fPWcleanNeighbors << endl;
        exit( -1 );
    }
    if( fRunPara->fPWlimit < 0 )
    {
        cout << "Can't send less than zero pixels to moment-generating function: "<<fRunPara->fPWlimit << endl;
        exit( -1 );
    }

    if( fPrintOutputFile )
    {
        cout << fRunPara->foutputfileName << endl;
        exit( 0 );
    }

    printStartMessage();
}


void VReadRunParameter::printHelp()
{
    printStartMessage();
    if( fDebug ) cout << "VReadRunParameter::printHelp()" << endl;
    cout << endl;
    cout << "Command line options:" << endl;
    cout << "=====================" << endl;
    cout << endl;
    cout << "(observe: default values are good for a standard analysis)" << endl;
    cout << endl;
    cout << "General:" << endl;
    cout << "--------" << endl;
    cout << "\t -runmode=0-6 \t\t\t\t 0=analysis (default)" << endl;
    cout << "\t              \t\t\t\t 1=pedestal calculation (high gain channels)" << endl;
    cout << "\t              \t\t\t\t 2=gain/toffset calculation (high gain channels)" << endl;
    cout << "\t              \t\t\t\t 3=trace library" << endl;
    cout << "\t              \t\t\t\t 4=write dstfile" << endl;
    cout << "\t              \t\t\t\t 5=gain/toffset calculation (low gain channels)" << endl;
    cout << "\t              \t\t\t\t 6=pedestal calculation (low gain channels)" << endl;
    cout << "\t -sourcefile FILENAME \t\t\t full path + filename" << endl;
    cout << "\t -sourcetype=0-7 \t\t\t source data file format" << endl;
    cout << "\t              \t\t\t\t 0=rawdata" << endl;
    cout << "\t              \t\t\t\t 1=GrIsu Monte Carlo (ascii file)" << endl;
    cout << "\t              \t\t\t\t 2=MC in vbf/cvbf format" << endl;
    cout << "\t              \t\t\t\t 3=rawdata in vbf/cvbf format (default)" << endl;
    cout << "\t              \t\t\t\t 4=DST (data)" << endl;
    cout << "\t              \t\t\t\t 5=multiple GrISu MC files" << endl;
    cout << "\t              \t\t\t\t 6=PE file" << endl;
    cout << "\t              \t\t\t\t 7=DST (MC) file" << endl;
    cout << "\t -runnumber=INT \t\t\t set runnumber (default: get run number from sourcefile name)" << endl;
    cout << "\t -useDBinfo\t\t\t\t get run info (target, wobble offsets, etc.) from database" << endl;
    cout << "\t\t\t\t\t\t (attention, this might overwrite some of the given command line parameters, default: on, switch of with -donotusedbinfo )" << endl;
    cout << "\t -nevents=NEVENTS \t\t\t loop over NEVENTS events in display=0 mode (<0 = no limit) (default=" << fRunPara->fnevents << ")" << endl;
    cout << "\t -reconstructionparameter FILENAME \t file with reconstruction parameters (e.g., array analysis cuts)" << endl;
    cout << endl;

    cout << "Output:" << endl;
    cout << "-------" << endl;
    cout << "\t -output FILE.root \t\t\t file with analysis results (FILE=-1 means no output) (default=RUNNUMBER.root)" << endl;
    cout << "\t -writeallMC \t\t\t\t write all events to showerpars and tpars trees (default: off)" << endl;
    cout << "\t -writenoMCTree \t\t\t do not write MC event tree to output file (default: " << fRunPara->fwriteMCtree << ")" << endl;
    cout << endl;

    cout << "Detector definition:" << endl;
    cout << "--------------------" << endl;
    cout << "\t -ntelescopes=INT \t\t\t number of telescopes (any integer, default 4)" << endl;
    cout << "\t -teltoana=INT \t\t\t\t analyze only these telescopes" << endl;
    cout << "\t\t\t\t\t\t (Telescope 1=1,..., Telescopes 2 and 3 = 23, Telescopes 1,2,4 = 124, or 1,2,10,25; default 1234, )" << endl;
    cout << "\t -camera=CAMERA \t\t\t set detector geometry file (default=veritasBC4_080117_Autumn2007-4.1.2_EVNDISP.cfg)" << endl;
    cout << "\t -vbfnsamples \t\t\t\t use number of FADC samples from VBF file (default=" << fRunPara->fUseVBFSampleLength << ")" << endl;
    cout << endl;

    cout << "Calibration:" << endl;
    cout << "------------" << endl;
    cout << "\t -calibrationfile FILENAME \t\t file with names of pedestal/gain/toffset/pixel status files (assume path $EVNDATA/calibration/)" << endl;
    cout << "\t -lowgaincalibrationfile FILENAME \t file with names for pedestals and high/low gain multiplier files (assume path $EVNDATA/calibration/)" << endl;
    cout << "\t -gaincorrection=FLOAT \t\t\t apply correction to gains (default=1)" << endl;
    cout << "\t -usepeds \t\t\t\t use only true pedestal events (event type=2; use -donotusepeds to switch it off)" << endl;
    cout << "\t -lasermin=INT \t\t\t\t minimal total charge sum for a event to be a laser event (default=" << fRunPara->fLaserSumMin << ")" << endl;
    cout << "\t -l2setspecialchannels FILENAME \t set special channels for l2 feed into FADC crates (default=specialChannel.dat)" << endl;
    cout << "\t -l2timecorrect=0/1 \t\t\t apply FADC stop time corrections based on L2 pulses (default=true)" << endl;
    cout << "\t -usePedestalsInTimeSlices=0/1 \t\t use time dependent pedestals (high gain channels) (default = on(1))" << endl;
    cout << "\t -usePedestalsInTimeSlicesLowGain=0/1 \t use time dependent pedestals (low gain channels) (default = off(0))" << endl;
    cout << "\t -PedestalsInTimeSlices \t\t calculate pedestals on short time scale (default=false)" << endl;
    cout << "\t -PedestalsLengthOfTimeSlice=FLOAT \t length of time slices for pedestal variations (default=180s)" << endl;
    cout << "\t -PedestalsInTimeSlicesSumWindow=INT \t length of sum window for pedestal variations (default=4)" << endl;
    cout << "\t -PedestalsInTimeSlicesSumFirst=INT \t start of sum window for pedestal variations (default=0)" << endl;
    cout << "\t -deadchannelfile FILE \t\t\t read this file with dead channel definitions (default=deadChannelDefinition.dat)" << endl;
    cout << endl;

    cout << "Pointing: " << endl;
    cout << "---------" << endl;
    cout << "\t -elevation \t\t\t\t telescope elevation (preli)" << endl;
    cout << "\t -azimuth \t\t\t\t telescope azimuth (preli)" << endl;
    cout << "\t -target TARGET \t\t\t telescope is pointed to this target (use -print targets to print available targets)" << endl;
    cout << "\t -printtargets \t\t\t\t print available targets" << endl;
    cout << "\t -declination=DEC \t\t\t target is at this declination (J2000)" << endl;
    cout << "\t -rightascension=RA \t\t\t target is at this right ascension (J2000)" << endl;
    cout << "\t -decoffset=DEC \t\t\t offset added to target declination (degrees)" << endl;
    cout << "\t -raoffset=RA \t\t\t\t offset added to target right ascension (degrees - e.g. +/- 7.5 for 30 minutes OFF source)" << endl;
    cout << "\t -wobblenorth=FLOAT \t\t\t wobble offset NORTH (degrees)" << endl;
    cout << "\t -wobbleeast=FLOAT \t\t\t wobble offset EAST (degrees)" << endl;
    cout << "\t -checkpointing=FLOAT \t\t\t abort of difference between calculated pointing direction and vbf pointing direction is larger than this value" << endl;
    cout << "\t\t\t\t\t\t ([deg], default=" << fRunPara->fCheckPointing << " deg)"  << endl;
    cout << "\t -pointingErrorX=INT:FLOAT \t\t take pointing error in array reconstruction into account" << endl;
    cout << "\t\t\t\t\t\t (camera x-direction [deg], default 0, usage: for telescope 1 do for example: -pointingErrorX=1:0.05)" << endl;
    cout << "\t\t\t\t\t\t (NOTE: experts only; option disables reading of pointing values from the DB)" << endl;
    cout << "\t -pointingErrorY=INT:FLOAT \t\t\t take pointing error in array reconstruction into account" << endl;
    cout << "\t\t\t\t\t\t (camera y-direction [deg], default 0, usage: for telescope 1 do for example: -pointingErrorY=1:0.05)" << endl;
    cout << "\t\t\t\t\t\t (NOTE: experts only; option disables reading of pointing values from the DB)" << endl;
    cout << "\t -useDBtracking \t\t\t use database to calculate pointing errors (default: on, switch off with -usenodbtracking )" << endl;
    cout << "\t -useTCorrectionfrom SQL-DATE \t\t use pointing calculated with T-point correction valid for this data (default: not applied, example: -useTCorrectionfrom \"2007-10-10\"" << endl;
    cout << "\t -pointingmonitortxt DIRECTORY \t\t find pointing monitor text files in this directory (default: not applied, expect filename as pointing_VPM.37195.t1.dat)" << endl;
    cout << "\t -usedbvpm \t\t\t\t use calibrated pointing monitor data from DB (usenodbvpm to switch it off)" << endl;
// (obsolete)  cout << "\t -fillhistos \t\t fill diagnostic histograms (default=" << fRunPara->ffillhistos << ")" << endl;
    cout << "\t -dstfile FILENAME \t\t\t name of dst output file (root file, default: dstfile.root)" << endl;
    cout << "\t -dstallpixel=INT \t\t\t write data from all pixels to dst files (0: write image/border pixel only; default: 1)" << endl;
    cout << endl;

    cout << "FADC pulse integration:" << endl;
    cout << "-----------------------" << endl;
    cout << "\t -sumfirst=INT \t\t\t\t start of summation window (default=" << fRunPara->fsumfirst[0] << ")" << endl;
    cout << "\t -sumwindow=INT \t\t\t length of summation window (default=" << fRunPara->fsumwindow[0] << ")" << endl;
    cout << "\t -fixwindowstart \t\t\t fix the start of the integration windwo to -sumfirst=INT (do not move according to pulse position, incompatible with doublepass, default: off)" << endl;
    cout << "\t -tracefit=FLOAT \t\t\t fit FADC traces (-1.=off(default),0=fit all PMTs, >0: fit PMTs with peak value > tracefit x pedestal rms" << endl;
    cout << "\t -fitfunction=FIFU \t\t\t fit function to fit FADC traces (ev or grisu)" << endl;
    cout << "\t -MC_FADCTraceStart=INT \t\t MC FADC trace start (observe: nsamples+MC_FADCTraceStart<n_simulated_samples; default=0)" << endl;
    cout << "  Double pass options:" << endl;
    cout << "\t -doublepass \t\t\t\t optimize sum window parameters (default: on, use -nodoublepass to switch off)" << endl;
    cout << "\t -tracewindowshift=INT \t\t\t shift the summation window by value (in doublepass: low gain channels only, default: -1 (0 for -fixwindowstart))" << endl;
    cout << "\t -tracedefinesmallpulse=INT \t\t use double pass window placement for pulses with maximum smaller than this value (default: 15 d.c.)" << endl;
    cout << "\t -sumwindow_doublepass=INT \t\t length of summation window for second pass in double pass method (default=" << fRunPara->fsumwindowsmall[0] << ")" << endl;
    cout << endl;

    cout << "Image cleaning and calculation:" << endl;
    cout << "-------------------------------" << endl;
    cout << "\t -useSignalNoiseThresholds \t\t use use multiples of pedestal variations (default: on)" << endl;
    cout << "\t -useFixedThresholds \t\t\t use fixed image/border thresholds (default: off)" << endl;
    cout << "\t -imagethresh=FLOAT \t\t\t image threshold (default=" << fRunPara->fimagethresh[0] << ", units depend on cleaning method)" << endl;
    cout << "\t -borderthresh=FLOAT \t\t\t border threshold (default=" << fRunPara->fborderthresh[0] << ", units depend on cleaning method)" << endl;
    cout << "\t -brightnonimage=FLOAT \t\t\t bright non image threshold (default=" << fRunPara->fbrightnonimagetresh[0] << ")" << endl;
    cout << "\t -useTimeCleaning \t\t\t use advanced time cleaning (default: off)" << endl; //HP
    cout << "\t\t -timecutpixel=FLOAT \t\t\t time cut between pixels (default=" << fRunPara->ftimecutpixel[0] << ")" << endl; //HP
    cout << "\t\t -timecutcluster=FLOAT \t\t\t time cut between cluster (default=" << fRunPara->ftimecutcluster[0] << ") " << endl; //HP
    cout << "\t\t -minpixelcluster=INT \t\t\t minimum number of pixels in cluster (default=" << fRunPara->fminpixelcluster[0] << ")" << endl; //HP
    cout << "\t\t -loops=INT \t\t\t\t number of loops for border pixel finding (default=" << fRunPara->floops[0] << ")" << endl; //HP
    cout << "\t -smoothdead \t\t\t\t smooth over dead pixels" << endl;
    cout << "\t -logl=0/1/2 \t\t\t\t perform loglikelihood image parameterisation 0=off,1=on,2=on with minuit output (default=off)" << endl;
    cout << "\t -loglminloss=FLOAT \t\t\t perform loglikelihood image parameterisation for images with loss > values (default=off=1.)" << endl;
    cout << "\t -fuifactor=FLOAT \t\t\t fraction of image/border pixel under image ellipse fact (default=" << fRunPara->fImageAnalysisFUIFactor << ")" << endl;
    cout << "\t -padrun=RUN \t\t\t\t padding run number (use with care)" << endl;
    cout << endl;                                 // MS

    cout << "Parallaxwidth calculation"<<endl;    // MS
    cout << "--------------------------" << endl;
                                                  // MS
    cout << "\t -PWmethod=-1/0/1/2, \t\t\t default = "<<fRunPara->fPWmethod<<endl;
                                                  // MS
    cout << "\t\t\t\t\t\t -1 = no trigger-map parameters calculated"<<endl;
                                                  // MS
    cout << "\t\t\t\t\t\t 0  = cleaned CFD hits, requiring center pixels to have at least PWcleanNeighbors"<<endl;
                                                  // MS
    cout << "\t\t\t\t\t\t 1  = cleaned CFD hits, center + nearest neighbor pixels are accepted in the map"<<endl;
                                                  // MS
    cout << "\t\t\t\t\t\t 2  = simulations: FADC signals integrated and cleaned with fixed PWcleanThreshold, requiring PWcleanNeighbors nearest neighbors"<<endl;
                                                  // MS
    cout << "\t\t\t\t\t\t 3  = simulations: FADC signals integrated and cleaned with fixed PWcleanThreshold, requiring PWcleanNeighbors nearest neighbors," << endl;
    cout << "\t\t\t\t                 nearest-neighbor pixels are also accepted in the trigger map"<<endl;
                                                  // MS
    cout << "\t -PWcleanNeighbors=0/1/2/3..." "\t\t sets the number of neighbors required for a pixel to survive cleaning, default = "<<fRunPara->fPWcleanNeighbors<<endl;
    cout << "\t -PWcleanThreshold=FLOAT \t\t software cleaning threshold to use for method 3, default = "<<fRunPara->fPWcleanThreshold<<endl;
    cout << "\t -PWlimit=0...INT, \t\t\t limits the number of pixels PER trigger-sector transmitted to the moment-generating function. Setting of 0 transmits all pixels. Default: "<<fRunPara->fPWlimit <<endl;
    cout << endl;

    cout << "Display options:" << endl;
    cout << "----------------" << endl;
    cout << "\t -display=0/1 \t\t\t\t show eventdisplay (default=" << fRunPara->fdisplaymode << ")" << endl;
    cout << "\t -loop \t\t\t\t\t infinite loop (default=" << fRunPara->floopmode << ")" << endl;
    cout << "\t -highres \t\t\t\t large display" << endl;
    cout << "\t -photodiode \t\t\t\t show photodiode" << endl;
    cout << "\t -plotraw \t\t\t\t fast plotting without pedestals/gains/toffsets" << endl;
    cout << "\t -plotpaper \t\t\t\t clean plots for talks/papers (no dead channels, no small text, ..., default=false)" << endl;
    cout << "\t -plotmethod=INT \t\t\t results of this array reconstrutions are shown in 'all in one' display (default=0)" << endl;
    cout << endl;

    cout << "Simulations: " << endl;
    cout << "-------------" << endl;
    cout << "\t -isMC=0/1/2 \t\t\t\t source data is MC (not/write all MC events/only triggered MC events, default=0)" << endl;
    cout << "\t -fillmchisto \t\t\t\t fill MC histograms (default " << fRunPara->fFillMCHistos << ")" << endl;
    cout << "\t -teleNoff=INT \t\t\t\t offset in telescope counting (default: first tel. = 1; for grisu MC raw files only)" << endl;
    cout << "\t -sampleoff=INT \t\t\t offset in FADC sample reading (default: 0)" << endl;
    cout << "\t -tracelib FILE.root \t\t\t trace library file for MC background creation" << endl;
    cout << "\t -ignoredead \t\t\t\t ignore dead channel labeling in camera configuration files (default=false)" << endl;
    cout << "\t -ndeadchannel=INT \t\t\t number of pixels to set randomly dead (default=0)" << endl;
    cout << "\t -seedndead=INT \t\t\t seed for setting pixels randomly dead (default=0)" << endl;
    cout << "\t -ndeadboard=INT \t\t\t number of boards to set randomly dead (default=0)" << endl;
    cout << "\t -fadcscale=float \t\t\t scale factor for traces (default=1.)" << endl;
    cout << "\t -pedestalseed=int \t\t\t seed for pedestal calculation (default=0)" << endl;
    cout << "\t -pedestalfile FILENAME \t\t use pedestals from P lines in this pedestal file (default: Off)" << endl;
    cout << "\t -pedestalnoiselevel=int \t\t GrIsu noise level used for external pedestal file (default=250)" << endl;
    cout << "\t -pedestalDefaultPedestal=float \t GrIsu default pedestals used in simulations (default=15.)" << endl;
    cout << endl;
    cout << "Others:" << endl;
    cout << "-------" << endl;
    cout << "\t -debug \t\t\t\t print lots of debug output" << endl;
    cout << "\t -muon \t\t\t\t\t search for muon rings" << endl;
    cout << "\t -frogs \t\t\t\t use frogs template analysis" << endl;
    cout << "\t -user USERNAME \t\t\t username" << endl;
    cout << endl;
    cout << endl;
    cout << "for installation see file INSTALL" << endl;
    cout << endl;
    cout << "please report any problems and bugs to Gernot Maier or Jamie Holder" << endl;
    cout << endl;
}


bool VReadRunParameter::readConfigFile( string iCfile )
{
    return false;
}


void VReadRunParameter::printTargets()
{
    VTargets iT;
    iT.printTargets();
}


void VReadRunParameter::setDirectories()
{
    char i_text[600];

// outputfilename
    if( fRunPara->foutputfileName != "-1" && fRunPara->foutputfileName.size() == 0 )
    {
        sprintf( i_text, "%s/%d.root", fRunPara->getDirectory_EVNDISPOutput().c_str(), fRunPara->frunnumber );
        fRunPara->foutputfileName = i_text;
// check if output directory exists, otherwise create it
        string i_worDir;
        i_worDir = gSystem->WorkingDirectory();
        if( !gSystem->cd( fRunPara->getDirectory_EVNDISPOutput().c_str() ) )
        {
            if( gSystem->mkdir( fRunPara->getDirectory_EVNDISPOutput().c_str() ) != 0 )
            {
                cout << "VReadRunParameter::test_and_adjustParams() error: unable to create output directory: " << endl;
		cout <<  fRunPara->getDirectory_EVNDISPOutput() << endl;
                exit( -1 );
            }
            else
            {
                cout << "VReadRunParameter::test_and_adjustParams() info: created output directory" << endl;
            }
        }
        gSystem->cd( i_worDir.c_str() );
    }

// check if calibration directories exist

    if( fRunPara->frunmode == 1 || fRunPara->frunmode == 2 || fRunPara->frunmode == 5 || fRunPara->frunmode == 6 )
    {
        string i_worDir = gSystem->WorkingDirectory();
        for( unsigned int i = 0; i < fRunPara->fNTelescopes; i++ )
        {
            sprintf( i_text, "%s/Calibration/Tel_%d/", fRunPara->getDirectory_EVNDISPAnaData().c_str(), i+1 );
            gSystem->cd( i_worDir.c_str() );
            if( !gSystem->cd( i_text ) )
            {
                cout << "\t creating calibration directory for Telescope " << i+1 << " : " << i_text << endl;
                if( gSystem->mkdir( i_text, kTRUE ) != 0 )
                {
                    cout << "VReadRunParameter::test_and_adjustParams() error: unable to create calibration directory for Telescope ";
		    cout << i+1 << ": " << endl;
		    cout << i_text << endl;
                    exit( -1 );
                }
            }
        }
        gSystem->cd( i_worDir.c_str() );
    }
}


void VReadRunParameter::isCompiledWithDB()
{
#ifndef RUNWITHDB
    cout << endl;
    cout << "#########################################################################################" << endl;
    cout << "ERROR: eventdisplay is not compiled with mysql support" << endl;
    cout << "       unable to use database" << endl;
    cout << "       do not use command line parameters for database usage" << endl;
    cout << endl;
    cout << "       uncomment line 'DBFLAG=-DRUNWITHDB' in Makefile to compile eventdisplay with mysql support" << endl;
    cout << "#########################################################################################" << endl;
    exit( 0 );
#endif
}


/*!

   open DST file and read run number and number of telescopes
*/
bool VReadRunParameter::getRunParametersFromDST()
{
// check if suffix is .root (required for dst files)
    if( fRunPara->fsourcefile.size() < 6 || fRunPara->fsourcefile.substr( fRunPara->fsourcefile.size()-5, 5 ) != ".root" )
    {
        return false;
    }
    if( fDebug ) cout << "VReadRunParameter::getRunParametersFromDST() " << endl;

// open dst file
    TFile iF( fRunPara->fsourcefile.c_str() );
    if( iF.IsZombie() )
    {
        cout << "VReadRunParameter::getRunParametersFromDST error opening DST file: " << fRunPara->fsourcefile << endl;
        exit( -1 );
    }
// check if this is a MC or data DST
    if( (TTree*)iF.Get( "mc" ) )
    {
        fRunPara->fsourcetype = 7;
        fRunPara->fIsMC = 1;
    }
    else
    {
        fRunPara->fsourcetype = 4;
        fRunPara->fIsMC = 0;
    }
    fRunPara->fuseDB = false;
    fRunPara->fDBTracking = false;
//    fRunPara->fDoublePass = false;
    VEvndispRunParameter *iV = (VEvndispRunParameter*)iF.Get( "runparameterDST" );
    if( !iV) iV = (VEvndispRunParameter*)iF.Get( "runparameterV2" );
    if( iV )
    {
       fRunPara->fTargetName = iV->fTargetName;
       if( fRunPara->fTargetName == "NOSOURCE" ) fRunPara->fTargetName = "laser";
       fRunPara->frunnumber  = iV->frunnumber;
       fRunPara->fNTelescopes = iV->fNTelescopes;
       fRunPara->fIsMC = iV->fIsMC;
       if( fRunPara->fIsMC ) fRunPara->fsourcetype = 7;
       else                  fRunPara->fsourcetype = 4;
       fRunPara->getObservatory() = iV->getObservatory();
       fRunPara->fCalibrationDataType = iV->fCalibrationDataType;
       fRunPara->fFADCChargeUnit = iV->fFADCChargeUnit;
       fRunPara->fpulsetiminglevels = iV->fpulsetiminglevels;
       fRunPara->setPulseZeroIndex();
    }
// no run parameters found
    else
    {
// get dst tree and read run number and number of telescopes
       TTree *t = (TTree*)iF.Get( "dst" );
       if( !t )
       {
	   cout << "VReadRunParameter::getRunParametersFromDST error: DST tree not found" << endl;
	   exit( -1 );
       }
       unsigned int iR = 0;
       unsigned int iNTel = 0;
       t->SetBranchAddress( "runNumber", &iR );
       t->SetBranchAddress( "ntel", &iNTel );

       if( t->GetEntries() > 0 )
       {
	   t->GetEntry( 0 );
	   fRunPara->frunnumber = iR;
	   fRunPara->fNTelescopes = iNTel;
       }
       else return false;
    }

    return true;
}

void VReadRunParameter::printStartMessage()
{
    cout << endl;
    cout << "\t---------------------------------------------------" << endl;
    cout << "\t|                                                 | " << endl;
    cout << "\t|        IACT event analysis and display          |" << endl;
    cout << "\t|                                                 | " << endl;
    cout << "\t|     \t\t Version " << fRunPara->getEVNDISP_VERSION() << "                   |" << endl;
    cout << "\t|     \t\t SVN " << fRunPara->getSVN_VERSION() << "             |" << endl;
    cout << "\t|                                                 | " << endl;
    cout << "\t---------------------------------------------------" << endl;
}
