////////////////////////////////////////////////
//
// To Convert an anasum.root file into the
// ctool's fits formatted event list
//
// Written by Nathan Kelley-Hoskins
// Jan 2014
//
// For more info, do
//    $ writeCTAEventListFromAnasum -h
//
//
#include "stdint.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bitset>

#include "TChain.h"
#include "TChainElement.h"
#include "TFile.h"
#include "TTree.h"
#include "TKey.h"
#include "TDirectory.h"

#include "CData.h"
#include "VSkyCoordinatesUtilities.h"
#include "VASlalib.h"
#include "VStereoAnalysis.h"
#include "VAnaSumRunParameter.h"
#include "VDBRunInfo.h"
#include "VPointingDB.h"
#include "VTimeMask.h"

#include "FITSRecord.h"
#include "fitsio.h"

using namespace std;

inline bool file_exists( const std::string& name ) ;
std::string get_env_var( std::string const& key ) ;
void getHumanDateTimes( VEvndispRunParameter* evrunpar, char dateobs[], char timeobs[],
						char dateend[], char timeend[] ) ;
void getRunTimes( VEvndispRunParameter* evrunpar, double met_zero, double& tstart,
				  double& tstop, double& telapse, double& startMJD, double& stopMJD ) ;

void telmask_clear( bool telmask[] ) ;
void telmask_set( bool telmask[], int tel, bool state ) ;
bool telmask_get( bool telmask[], int tel ) ;
bool readImgSel( int  ImgSel,    int tel ) ;
string telmask_to_string( bool telmask[] ) ;
void calc_Avg_RADec_from_Pointing( TChain* chainPointData, double& ra, double& dec ) ;
void calc_Avg_AltAz_from_Pointing( TChain* ch, double& alt, double& az ) ;
void calcGTIsFromTimeMask( VTimeMask* vtm, const double tInterval, const double maxSeconds, const double startMJD, const double stopMJD, vector<double>& gti_beg, vector<double>& gti_end ) ;
const unsigned int NTEL = 4 ; // HARDCODE : number of telescopes for this program to work with

int main( int argc, char* argv[] )
{

	int c ;
	bool helptext = false ;
	char INPfname[200]     = "empty" ;
	char INPoutname[200]   = "empty" ;
	string INPfnameStr ;
	string INPoutnameStr ;
	bool overwriteFlag = false ;
	bool maxEventsFlag = false ;
	bool rawEventsFlag = false ;
	unsigned int maxEvents = -1 ;
	while( ( c = getopt( argc, argv, "i:o:n:hfe" ) ) != -1 )
	{
		switch( c )
		{
			case 'i':
				sprintf( INPfname, "%s", optarg ) ;
				INPfnameStr = INPfname ;
				break ;
			case 'o':
				sprintf( INPoutname, "%s", optarg ) ;
				INPoutnameStr = INPoutname ;
				break ;
			case 'h':
				helptext = true ;
				break ;
			case 'n':
				maxEventsFlag = true ;
				maxEvents = atoi( optarg ) ;
				break ;
			case 'f':
				overwriteFlag = true ;
				break ;
			case 'e':
				rawEventsFlag = true ;
				break ;
			default:
				//cout << "unknown option '" << c << "', aborting!" << endl;
				printf( "unknown option '%c', exiting!\n", c ) ;
				exit( 1 ) ;
				break ;
		}
	}
	
	char examplecmd[150] = "" ;
	sprintf( examplecmd, "$ %s -i 45538.anasum.root -o mywrittenfile.fits", argv[0] ) ;
	
	if( argc < 5 || helptext )
	{
		cout << endl << argv[0] << endl;
		cout << "   For converting eventdisplay anasum files to the 'ctools fits' format." << endl;
		cout << "   Must specify at least the input and output filenames." << endl << endl ;
		printf( "   example: \n" );
		printf( "      %s\n\n", examplecmd ) ;
		cout << "   Input Arguments are:" << endl;
		cout << "      -i <fname> : input anasum root file to convert ***" << endl ;
		cout << "      -o <fname> : output fits filename" << endl ;
		cout << "      -h         : prints help text" << endl << endl;;
		cout << "   Optional:" << endl;
		cout << "      -m <int>   : only load first <int> ttrees from anasum root file, for debugging."    << endl ;
		cout << "      -n <int>   : only read at most <int> events total, for debugging."                  << endl ;
		cout << "      -f         : clobber, will forcably overwrite existing output fits file."           << endl ;
		cout << "                   otherwise error out if file already exists."                           << endl ;
		cout << "      -e         : also print each event to stdout, for debugging purposes"               << endl ;
		cout << endl;
		cout << "   *** Note: input anasum file must have been created with the '* WRITEEVENTTREEFORCTOOLS 1'" << endl;
		cout << "        option in ANASUM.runparameter, which is not on by default.  The output anasum" << endl;
		cout << "        file will then have the needed extra information in it." << endl;
		cout << endl;
		return 1 ;
	}
	
	//cout << "NTEL                      : " << NTEL << endl;
	cout << "input anasum file         : " << INPfnameStr.substr( INPfnameStr.find_last_of( "\\/" ) + 1, -1 ) << endl;
	cout << "output fits filename      : " << INPoutnameStr.substr( INPoutnameStr.find_last_of( "\\/" ) + 1, -1 ) << endl;
	cout << "overwrite output fits file: " ;
	if( overwriteFlag )
	{
		cout << "yes" << endl;
	}
	else
	{
		cout << "no"  << endl;
	}
	if( maxEventsFlag )
	{
		cout << "events to read in: " << maxEvents  << endl;
	}
	
	// ANASUM File Checks
	ifstream INPinputfile( INPfname ) ;
	if( strcmp( INPfname,    "empty" ) == 0 )
	{
		cout << "Need to specify input file with -m" << endl;
		printf( "   ex: %s\n", examplecmd ) ;
		return 1 ;
	}
	else if( ! INPinputfile.good() )
	{
		cout << "Error, could not read input anasum file '" << INPfname << "', exiting." << endl;
		return 1 ;
	}
	INPinputfile.close();
	
	TFile* myfile = new TFile( INPfname ) ;
	if( myfile->IsZombie() || myfile->TestBit( TFile::kRecovered ) )
	{
		cout << "Error, anasum file " << INPfname << " didn't close correctly or is corrupt.  Exiting..." << endl;
		return 1 ;
	}
	
	// Output name check
	if( strcmp( INPoutname,    "empty" ) == 0 )
	{
		cout << "Need to specify output fits filename with -o" << endl;
		printf( "   ex: %s\n", examplecmd ) ;
		return 1 ;
	}
	
	// Fits wont overwrite an existing file for some reason,
	// this section deletes 'INPoutname'
	if( file_exists( INPoutname ) )
	{
		if( overwriteFlag )
		{
			//cout << "Force Overwrite flag -f detected," << endl;
			//cout << "   ...deleting file " << INPoutname << " ..." << endl;
			if( remove( INPoutname ) != 0 )
			{
				perror( "Error deleting file" );
				return 1 ;
			} // else puts( "File successfully deleted" );
		}
		else
		{
			cout << "Error, file " << INPoutname << " already exists.  Use the '-f' option to overwrite this existing file." << endl;
			return 1 ;
		}
	}
	
	char infile[500];
	sprintf( infile, "%s", INPfname );
	char outfile[500] ;
	sprintf( outfile, "%s", INPoutname ) ;
	
	TFile* anasumfile = new TFile( infile );
	// see root.cern.ch/drupal/content/how-read-objects-file
	// and ftp://root.cern.ch/root/doc/ROOTUsersGuideHTML/ch11s02.html
	// for more explaination of the below TIter and TKey algorithm
	TIter next( anasumfile->GetListOfKeys() );
	TKey* key ;
	vector <string> dirList ;
	while( ( key = ( TKey* )next() ) )
	{
		//printf( "key: %s points to an object of class: %s at %d\n", key->GetName(), key->GetClassName(), key->GetSeekKey() );
		if( strcmp( key->GetClassName(), "TDirectoryFile" ) != 0 )
		{
			//cout << "  No TDirectoryFile, skipping..." << endl;
			continue ;
		}
		
		// God help me, but there is no regex libs in the C++ standard.
		// So now this only works if the directory name contains 'run_' ,
		// rather than matches the robust '^run_\d{5,6}$' regex string.
		// I'm going to cry.
		string nam( key->GetName() ) ;
		if( nam.find( "run_" ) == std::string::npos )
		{
			//cout << "  Did not find run_ !" << endl;
			continue ;
		}
		
		dirList.push_back( nam ) ;
		break ; // only want first run, assuming this is an anasum-parallel root file, and not a merged one.
		//cout << "  added " << key->GetName() << " to the list of ROOT dir's to add, now " << dirList.size() << " total dirs"  << endl;
	}
	
	if( dirList.size() != 1 )
	{
		cout << "Error, The variable 'dirList' is only supposed to have 1 run in it!, exiting!" << endl;
		exit( 1 ) ;
	}
	
	cout << endl;
	char treeName[100] ;
	TChain* chainEventList = new TChain() ;
	TChain* chainTelConfig = new TChain() ;
	TChain* chainPointData = new TChain() ;
	for( std::vector<string>::iterator it = dirList.begin(); it != dirList.end(); ++it )
	{
		//std::cout << " " << *it;
		
		// load event list in TTree format
		sprintf( treeName, "%s/%s/stereo/TreeWithEventsForCtools", infile, ( *it ).c_str() ) ;
		chainEventList->Add( treeName ) ;
		cout << "Loaded TTree " << treeName << " from TFile " << infile << "..." << endl;
		
		// load telescope config TTree 'telconfig'
		sprintf( treeName, "%s/%s/stereo/telconfig", infile, ( *it ).c_str() ) ;
		cout << "Loaded TTree " << treeName << " from TFile " << infile << "..." << endl;
		chainTelConfig->Add( treeName ) ;
		
		// load TTree of pointingDataReduced containing pointing info for the run
		sprintf( treeName, "%s/%s/stereo/pointingDataReduced", infile, ( *it ).c_str() ) ;
		cout << "Loaded TTree " << treeName << " from TFile " << infile << "..." << endl;
		chainPointData->Add( treeName ) ;
		
	}
	
	// FITS records' template file
	string EVNDISPSYS = get_env_var( "EVNDISPSYS" ) ;
	//cout << "EVNDISPSYS:" << EVNDISPSYS << endl;
	char evlTemplateChar[200] = "" ;
	//sprintf( evlTemplateChar, "%s/templates/evl/1.0.0/EventList.tpl", EVNDISPSYS.c_str() ) ;
	sprintf( evlTemplateChar, "%s/templates/EventList.tpl", EVNDISPSYS.c_str() ) ;
	string evlTemplate( evlTemplateChar ) ;
	
	// Test if template exists?
	if( ! file_exists( evlTemplate ) )
	{
		cout << "Error, template " << evlTemplate << " doesn't exist, exiting..." << endl;
		return 1 ;
	}
	
	FITSRecord recEVENTS( INPoutname, evlTemplate, "EVENTS" );
	FITSRecord recTELARRAY( INPoutname, evlTemplate, "TELARRAY" );
	FITSRecord recGTI( INPoutname, evlTemplate, "GTI" );
	recEVENTS.setVerbose( 0 ); // Dont show us adding every single column mapping.
	recTELARRAY.setVerbose( 0 ); // Dont show us adding every single column mapping.
	recGTI.setVerbose( 0 );    // Dont show us adding every single column mapping.
	
	// get crude runid from the TDirectory name
	int runid = 0 ;
	sscanf( dirList[0].c_str(), "run_%d", &runid ) ;
	
	////////////////////////////////////////////////////////////////////////////////////
	// setup the TFile
	char objName[200]   = "" ;
	sprintf( objName, "run_%d/stereo/cdatatree", runid ) ;
	TFile* obsInfoFile = new TFile( infile, "READ" ) ;
	if( !obsInfoFile )
	{
		cout << "Error, Unable to read file " << infile << " , exiting..." << endl;
		return 1 ;
	}
	else
	{
		cout << "Loaded TFile from " << infile << "..." << endl;
	}
	
	////////////////////////////////////////////////////////////////////////////////////
	// setup the CData's TTree
	sprintf( objName, "run_%d/stereo/cdatatree", runid ) ;
	TTree* cdataTree = ( TTree* )obsInfoFile->Get( objName ) ;
	if( !cdataTree )
	{
		cout << "Error, Unable to load TTree " << objName << " from file " << infile << " , exiting..." << endl;
		return 1 ;
	}
	else
	{
		cout << "Loaded TTree " << objName << "..." << endl;
	}
	CData* cdata     = new CData( cdataTree ) ;
	if( !cdata )
	{
		cout << "Error, Unable to load CData object from the CData TTree, exiting..."  << endl;
		return 1 ;
	}
	else
	{
		cout << "Loaded CData object..." << endl;
	}
	cdata->GetEntry( 1 ) ;
	int runiddumb = cdata->runNumber ;
	if( runid != runiddumb )
	{
		cout << "Error, runid:" << runid << " and runiddumb:" << runiddumb << " are not equal, is something wrong with " << infile << " ? " << endl;
		return 1 ;
	}
	
	/*
	sprintf( objName, "run_%d/stereo/VASRPDC", runid ) ;
	VAnaSumRunParameterDataClass * varpdc = (VAnaSumRunParameterDataClass*)obsInfoFile->Get( objName ) ;
	if ( varpdc ) {
	    cout << "VASRPDC:" << varpdc << endl;
	    cout << "VASRPDC->fRunOn:" << varpdc->fRunOn << endl;
	} else {
	    cout << "VAnaSumRunParameterDataClass failed to load... exiting!" << endl;
	    return 1 ;
	}
	*/
	
	//////////////////////////////////////////////////////////////////////////////////
	// get VAnaSumRunParameter object from file
	VAnaSumRunParameter* anasumRunPar = 0 ;
	sprintf( objName, "run_%d/stereo/VAnaSumRunParameter", runid ) ;
	anasumRunPar = ( VAnaSumRunParameter* )obsInfoFile->Get( objName ) ;
	if( !anasumRunPar )
	{
		cout << "Error, Unable to load VAnaSumRunParameter object " << objName << " from file " << infile << " , exiting..." << endl;
		return 1 ;
	}
	else
	{
		cout << "Loaded VAnaSumRunParameter object '" << objName << "' ..." << endl;
	}
	//cout << "anasumRunPar->fEnergySpectrumBinSize: " << anasumRunPar->fEnergySpectrumBinSize << endl;
	//cout << "anasumRunPar->fRunList[0]: " << anasumRunPar->fRunList[0] << endl;
	//cout << "anasumRunPar->fRunList.size(): "     << anasumRunPar->fRunList.size() << endl;
	//cout << "anasumRunPar->fScalarDeadTimeFrac: " << anasumRunPar->fScalarDeadTimeFrac << endl;
	double deadtimefrac = anasumRunPar->fScalarDeadTimeFrac ;
	
	char progname[100] = "" ;
	int  svnrev        = 0  ;
	sscanf( anasumRunPar->getSVN_VERSION().c_str(), "$Revision: %d $", &svnrev ) ;
	sprintf( progname, "Event Display %s, svn rev:%d", anasumRunPar->getEVNDISP_VERSION().c_str(), svnrev ) ;
	
	/////////////////////////////////////////////////////////////////////////////////////
	// get VEvndispRunParameter object from file
	VEvndispRunParameter* evndispRunPar = 0 ;
	sprintf( objName, "run_%d/stereo/runparameterV2", runid ) ;
	evndispRunPar = ( VEvndispRunParameter* )obsInfoFile->Get( objName ) ;
	if( !evndispRunPar )
	{
		cout << "Error, Unable to load VEvndispRunParameter object " << objName << " from file " << infile << " , exiting..." << endl;
		return 1 ;
	}
	else
	{
		cout << "Loaded VEvndispRunParameter object '" << objName << "' ..." << endl;
	}
	
	// run start/stop date strings, for human reading
	char dateobs[30] = "null" ;
	char timeobs[30] = "null" ;
	char dateend[30] = "null" ;
	char timeend[30] = "null" ;
	getHumanDateTimes( evndispRunPar, dateobs, timeobs, dateend, timeend ) ;
	
	// Establish Reference MJD (0.0 in MET time)
	// This is defined by:
	//   "The beginning of the UTC day of the earliest run in the database"
	//
	// Earliest Run: 30576 , 2006-05-01, UTC 08:28
	int    mjdrefi = 0   ; // day in MJD when MET is 0.0  (integer part only)
	double mjdreff = 0.0 ; // decimal part of mjdrefi
	int MJDREF_YEAR = 2006 ;
	int MJDREF_MONT =    5 ;
	int MJDREF_DAYY =    1 ;
	printf( "Calculating Mission Elapsed Time(MET) starting at the beginning of UTC day %04d-%02d-%02d...\n", MJDREF_YEAR, MJDREF_MONT, MJDREF_DAYY ) ;
	mjdrefi = VSkyCoordinatesUtilities::getMJD( MJDREF_YEAR, MJDREF_MONT, MJDREF_DAYY ) ;
	
	// MET start, stop, and duration of run (seconds)
	double tstart   = 0.0 ;
	double tstop    = 0.0 ;
	double telapse  = 0.0 ;
	double met_zero = mjdrefi + mjdreff ; // MJD(days) - met_zero = MET (days)
	double startMJD = 0.0 ;
	double stopMJD  = 0.0 ;
	getRunTimes( evndispRunPar, met_zero, tstart, tstop, telapse, startMJD, stopMJD ) ;
	//cout << "  telapse     :" << telapse << endl;
	//cout << "  deadtimefrac:" << deadtimefrac << endl;
	double livetime = telapse * ( 1.0 - deadtimefrac ) ;
	//cout << "  livetime    :" << livetime << endl;
	
	// calculate average RA, Dec, Alt, Az
	// just by a straight average
	// veritas always tracks one RA/Dec coordinate
	double ra_pnt  = 0.0 ;
	double dec_pnt = 0.0 ;
	double alt_pnt = 0.0 ;
	double az_pnt  = 0.0 ;
	calc_Avg_RADec_from_Pointing( chainPointData, ra_pnt,  dec_pnt ) ;
	calc_Avg_AltAz_from_Pointing( chainPointData, alt_pnt, az_pnt ) ;
	cout << "  ra_pnt :" << ra_pnt  << endl;
	cout << "  dec_pnt:" << dec_pnt << endl;
	//cout << "  alt_pnt:" << alt_pnt << endl;
	//cout << "  az_pnt :" << az_pnt  << endl;
	
	// load time mask, get good time intervals
	//cout << endl;
	//cout << "Attempting to load VTimeMask" << endl;
	string timemaskfile = "/afs/ifh.de/user/n/nkelhos/scratch/ConvertVeritasDataToCTOOLs/testzone/ANASUM.timemask.dat" ;
	//string timemaskfile = "" ;
	//cout << "  Using time mask file: " << timemaskfile << endl;
	VTimeMask* vtm = 0 ;
	int vtmmode = 2 ;
	if( vtmmode == 1 )
	{
		vtm = new VTimeMask( runid, startMJD, stopMJD, timemaskfile ) ;
		sprintf( objName, "/run_%d/stereo/timeMask", runid ) ;
		myfile->cd( objName ) ;
		TDirectory* tdir = myfile->CurrentDirectory();
		//cout << "  pwd:" << tdir->GetPath() << endl;
		vtm->readObjects( tdir ) ;
		vtm->setMask( runid, startMJD, stopMJD, timemaskfile ) ;
	}
	else if( vtmmode == 2 )
	{
		//cout << "   ...from streamed object" << endl;
		sprintf( objName, "run_%d/stereo/vtimemask", runid ) ;
		vtm = ( VTimeMask* )obsInfoFile->Get( objName ) ;
		vtm->setMask() ;
	}
	else
	{
		cout << "Error, vtmmode=" << vtmmode << endl;
		return 1 ;
	}
	
	if( ! vtm )
	{
		cout << "Error, Unable to load VTimeMask directory " << objName << " from file " << infile << " , exiting..." << endl;
		return 1 ;
	}
	else
	{
		cout << "Loaded VTimeMask directory " << objName << "..." << endl;
	}
	
	//cout << "  vtm->getMaskStatus():" << vtm->getMaskStatus() << "   (true=" << true << ", false=" << false << ")" << endl;
	
	// loop over entire run, display time mask
	//printf( "  startMJD:%11.5f\n", startMJD ) ;
	//printf( "  stopMJD :%11.5f\n", stopMJD  ) ;
	//printf( "  mask_file:%s\n",    vtm->getMaskFileName().c_str() ) ;
	//printf( "  mask.size():%d\n",  vtm->getMask().size()          ) ;
	//cout << "  mask.size():" << vtm->getMask().size() << " seconds (or bits)" << endl;
	/*
	double currMJD = 0.0 ;
	bool   stat    = false ;
	for ( int i_dur=0 ; i_dur<10000 ; i_dur++ ) {
	    currMJD = startMJD + (i_dur/(24.*60.*60.)) ;
	    if ( currMJD > stopMJD ) break ;
	    if ( i_dur % 60 == 0 ) printf( "\n  currMJD:%11.5f : sec %4d : ", currMJD, i_dur ) ;
	    stat = false ;
	    stat = vtm->checkAgainstMask( currMJD ) ;
	    cout << stat ;
	}
	*/
	cout << endl;
	//vtm->printMask() ;
	//cout << endl;
	cout << "displayMask()" ;
	vtm->displayMask( cout ) ;
	cout << endl;
	
	// sample the time mask every 'tInterval' seconds
	// to get the Good Time Intervals (hereafter: GTIs)
	double tInterval = 0.0001 ;
	
	// break out of search loop automatically after 'maxSeconds' seconds
	double maxSeconds = 3000.0 ;
	
	// vectors for storing the times of the
	// beginning and end of each GTI
	vector<double> gti_Beginning ;
	vector<double> gti_Ending ;
	
	// find GTI edges
	calcGTIsFromTimeMask( vtm, tInterval, maxSeconds, startMJD, stopMJD, gti_Beginning, gti_Ending ) ;
	cout << "  calcGTIsFromTimeMask():" << endl;
	cout.precision( 4 ) ;
	cout << "    startMJD:" << startMJD << endl;
	cout << "    stopMJD :" << stopMJD  << endl;
	if( gti_Beginning.size() > 0 )
	{
		printf( "    Edges:  Begin    - Ending\n" );
		for( unsigned int i = 0 ; i < gti_Beginning.size() ; i++ )
		{
			printf( "           %9.4f - %9.4f \n", ( gti_Beginning[i] - startMJD ) * 24 * 60 * 60, ( gti_Ending[i] - startMJD ) * 24 * 60 * 60 ) ;
		}
	}
	
	// EVENTS table headers
	recEVENTS.writeHeader( "CONV_RA" , 9999.9 ) ;
	recEVENTS.writeHeader( "CONV_DEC", 9999.9 ) ;
	recEVENTS.writeHeader( "CREATOR" , string( progname ) ) ;
	recEVENTS.writeHeader( "DATE_END", string( dateend ) ) ;        // human readable run-end   date string
	recEVENTS.writeHeader( "DATE_OBS", string( dateobs ) ) ;        // human readable run-start date string
	recEVENTS.writeHeader( "DEC_OBJ" , evndispRunPar->fTargetDec ) ;  // Dec of the target object in view
	recEVENTS.writeHeader( "EQUINOX" , 2000.0 ) ;                   // which JXXXX RA/Dec epoch to use for all RA/Dec in here
	//recEVENTS.writeHeader( "EXTNAME" , string( "events" ) ) ;
	recEVENTS.writeHeader( "EXTNAME" , string( "EVENTS" ) ) ;
	recEVENTS.writeHeader( "OBJECT"  , evndispRunPar->fTargetName ) ;
	recEVENTS.writeHeader( "OBS_ID"  , runid ) ;
	recEVENTS.writeHeader( "RA_OBJ"  , evndispRunPar->fTargetRA ) ;
	recEVENTS.writeHeader( "RADECSYS", string( "fk5" ) ) ;          // something related to the precession-rotation standard
	recEVENTS.writeHeader( "TELESCOP", string( "VERITAS" ) ) ;
	recEVENTS.writeHeader( "TIME_END", string( timeend ) ) ;
	recEVENTS.writeHeader( "TIME_OBS", string( timeobs ) ) ;
	recEVENTS.writeHeader( "TIMEREF" , string( "local" ) ) ;
	recEVENTS.writeHeader( "TIMESYS" , string( "TT" ) ) ;
	recEVENTS.writeHeader( "TIMEUNIT", string( "days" ) ) ;
	recEVENTS.writeHeader( "MJDREFI" , mjdrefi ) ;
	recEVENTS.writeHeader( "MJDREFF" , mjdreff ) ;
	recEVENTS.writeHeader( "TSTART"  , tstart ) ;
	recEVENTS.writeHeader( "TSTOP"   , tstop ) ;
	recEVENTS.writeHeader( "TELAPSE" , telapse ) ;
	recEVENTS.writeHeader( "ONTIME"  , telapse ) ;
	recEVENTS.writeHeader( "RA_PNT"  , ra_pnt ) ;                   // average pointing RA
	recEVENTS.writeHeader( "DEC_PNT" , dec_pnt ) ;                  // averate pointing Dec
	recEVENTS.writeHeader( "ALT_PNT" , alt_pnt ) ;
	recEVENTS.writeHeader( "AZ_PNT"  , az_pnt ) ;
	recEVENTS.writeHeader( "DEADC"   , deadtimefrac ) ;
	recEVENTS.writeHeader( "LIVETIME", livetime ) ;
	/*
	*/
	
	//recEVENTS.writeHeader( "CON_DEP" , 9999.9                   ) ; // ignore, not defined in template
	//recEVENTS.writeHeader( "OBSERVER", string("null")           ) ; // ignore, not defined in template
	//recEVENTS.writeHeader( "PNTMODE" , string("track")          ) ; // ignore, not defined in template
	//recEVENTS.writeHeader( "TIMEDEL" , 1.0                      ) ; // ignore, not defined in template
	
	// VERITAS Event Parameters (start with caps?)
	int    RunNumber        =     0   ;
	int    EventNumber      =     0   ;
	int    NImages          =     0   ;
	int    MJD              =     0   ;
	double Time             =     0.0 ;
	double RA               =     0.0 ;
	double DEC              =     0.0 ;
	double XGroundCore      =     0.0 ;
	double YGroundCore      =     0.0 ;
	double Energy_Erec      =     0.0 ;
	double Energy_Erec_Err  =     0.0 ;
	double Energy_ErecS     =     0.0 ;
	double Energy_ErecS_Err =     0.0 ;
	double Az               =     0.0 ;
	double El               =     0.0 ;
	double MSCW             = -9999.0 ;
	double MSCL             = -9999.0 ;
	double EmissionHeight   = -9999.0 ;
	double Xoff             =     0.0 ;
	double Yoff             =     0.0 ;
	int    ImgSel           =     0   ;
	chainEventList->SetBranchAddress( "runNumber"     , &RunNumber ) ;
	chainEventList->SetBranchAddress( "eventNumber"   , &EventNumber ) ;
	chainEventList->SetBranchAddress( "dayMJD"        , &MJD ) ;
	chainEventList->SetBranchAddress( "timeOfDay"     , &Time ) ;
	chainEventList->SetBranchAddress( "NImages"       , &NImages ) ;
	chainEventList->SetBranchAddress( "RA"            , &RA ) ;
	chainEventList->SetBranchAddress( "DEC"           , &DEC ) ;
	chainEventList->SetBranchAddress( "XGroundCore"   , &XGroundCore ) ;
	chainEventList->SetBranchAddress( "YGroundCore"   , &YGroundCore ) ;
	chainEventList->SetBranchAddress( "Energy"        , &Energy_Erec ) ;
	chainEventList->SetBranchAddress( "EnergyS"       , &Energy_ErecS ) ;
	chainEventList->SetBranchAddress( "Energy_Err"    , &Energy_Erec_Err ) ;
	chainEventList->SetBranchAddress( "EnergyS_Err"   , &Energy_ErecS_Err ) ;
	chainEventList->SetBranchAddress( "Az"            , &Az ) ;
	chainEventList->SetBranchAddress( "El"            , &El ) ;
	chainEventList->SetBranchAddress( "MSCW"          , &MSCW ) ;
	chainEventList->SetBranchAddress( "MSCL"          , &MSCL ) ;
	chainEventList->SetBranchAddress( "EmissionHeight", &EmissionHeight ) ;
	chainEventList->SetBranchAddress( "Xoff"          , &Xoff ) ;
	chainEventList->SetBranchAddress( "Yoff"          , &Yoff ) ;
	chainEventList->SetBranchAddress( "ImgSel"        , &ImgSel ) ;
	
	//double Xderot      = 0.0 ;
	//double Yderot      = 0.0 ;
	//double timeOfDay   = 0.0 ;
	//int    dayMJD      = 0   ;
	//double MSCW        = 0.0 ;
	//double MSCL        = 0.0 ;
	//chainEventList->SetBranchAddress( "Xderot"      , &Xderot      ) ;
	//chainEventList->SetBranchAddress( "Yderot"      , &Yderot      ) ;
	//chainEventList->SetBranchAddress( "timeOfDay"   , &timeOfDay   ) ;
	//chainEventList->SetBranchAddress( "dayMJD"      , &dayMJD      ) ;
	//chainEventList->SetBranchAddress( "MSCW"        , &MSCW        ) ;
	//chainEventList->SetBranchAddress( "MSCL"        , &MSCL        ) ;
	//chainEventList->SetBranchAddress( ""    , &    ) ;
	
	// CTOOLs Event Parameters
	long  int obs_id      = 0   ;
	long  int event_id    = 0   ;
	double    event_time  = 0.0 ;
	short int multip      = 0   ;
	float     ra          = 0.0 ;
	float     dec         = 0.0 ;
	float     dir_err     = 0.0 ;
	float     corex       = 0.0 ;
	float     corey       = 0.0 ;
	float     cor_err     = 0.0 ;
	float     energy      = 0.0 ;
	float     energyerr   = 0.0 ;
	float     az          = 0.0 ;
	float     alt         = 0.0 ;
	float     hil_msw     = 0.0 ;
	float     hil_msl     = 0.0 ;
	float     hil_msw_err = 0.0 ;
	float     hil_msl_err = 0.0 ;
	float     xmax        = 0.0 ;
	float     xmax_err    = 0.0 ;
	bool*     telmask     = new bool[NTEL];
	float     detx        = 0.0 ;
	float     dety        = 0.0 ;
	float     shwidth     = 0.0 ;
	float     shlength    = 0.0 ;
	recEVENTS.mapColumnToVar( "OBS_ID"     , obs_id ) ;
	recEVENTS.mapColumnToVar( "EVENT_ID"   , event_id ) ;
	recEVENTS.mapColumnToVar( "TIME"       , event_time ) ;
	recEVENTS.mapColumnToVar( "MULTIP"     , multip ) ;
	recEVENTS.mapColumnToVar( "RA"         , ra ) ;
	recEVENTS.mapColumnToVar( "DEC"        , dec ) ;
	recEVENTS.mapColumnToVar( "DIR_ERR"    , dir_err ) ;
	recEVENTS.mapColumnToVar( "COREX"      , corex ) ;
	recEVENTS.mapColumnToVar( "COREY"      , corey ) ;
	//recEVENTS.mapColumnToVar( "COR_ERR"    , cor_err   ) ;
	recEVENTS.mapColumnToVar( "ENERGY"     , energy ) ;
	recEVENTS.mapColumnToVar( "ENERGY_ERR" , energyerr ) ;
	recEVENTS.mapColumnToVar( "AZ"         , az ) ;
	recEVENTS.mapColumnToVar( "ALT"        , alt ) ;
	recEVENTS.mapColumnToVar( "HIL_MSW"    , hil_msw ) ;
	recEVENTS.mapColumnToVar( "HIL_MSW_ERR", hil_msw_err ) ;
	recEVENTS.mapColumnToVar( "HIL_MSL"    , hil_msl ) ;
	recEVENTS.mapColumnToVar( "HIL_MSL_ERR", hil_msl_err ) ;
	recEVENTS.mapColumnToVar( "XMAX"       , xmax ) ;
	recEVENTS.mapColumnToVar( "XMAX_ERR"   , xmax_err ) ;
	recEVENTS.mapColumnToVar( "TELMASK"    , telmask ) ;
	recEVENTS.mapColumnToVar( "DETX"       , detx ) ;
	recEVENTS.mapColumnToVar( "DETY"       , dety ) ;
	//recEVENTS.mapColumnToVar( "SHWIDTH"    , shwidth     ) ;
	//recEVENTS.mapColumnToVar( "SHLENGTH"   , shlength    ) ;
	
	// Transcribe events to FITSRecord EVENTS Table
	unsigned int totalEvents = chainEventList->GetEntries() ;
	//if ( maxEventsFlag && maxEvents < totalEvents ) totalEvents = maxEvents ;
	
	//cout << endl;
	//cout << "Looping over " << totalEvents << " Events..." << endl;
	
	//printf( "%6s %7s %6s %6s %6s %6s %6s %6s\n", "ObsID", "EventID", "RA  ", "DEC ", "Xoff", "Yoff", "MSCW", "MSCL") ;
	//printf( "HEAD %6s %8s %4s %17s %6s %6s %6s %6s %5s %12s\n", "OBS_ID", "EVENT_ID", "MASK", "TIME(MET)", "RA  ", "DEC ", "AZ", "EL", "MJD", "Time") ;
	for( unsigned int i = 0; i <= totalEvents - 1 ; i++ )
	{
		//if ( i > 20 ) break ;
		chainEventList->GetEntry( i );
		// variable names in all lowercase are CTOOLs
		// variable names with any capital letters are VERITAS variables
		obs_id      = RunNumber      ;
		event_id    = EventNumber    ;
		multip      = NImages        ;
		ra          = RA             ;
		dec         = DEC            ;
		dir_err     = -9999.9        ;
		corex       = XGroundCore    ;
		corey       = YGroundCore    ;
		cor_err     = -9999.9        ;
		energy      = Energy_ErecS   ;
		energyerr   = -9999.9        ; // Energy_ErecS_Err ?
		az          = Az             ;
		alt         = El             ;
		hil_msw     = MSCW           ;
		hil_msw_err = -9999.9        ;
		hil_msl     = MSCL           ;
		hil_msl_err = -9999.9        ;
		xmax        = -9999.9        ;
		xmax_err    = -9999.9        ;
		detx        = Xoff           ;
		dety        = Yoff           ;
		shwidth     = -9999.9        ;
		shlength    = -9999.9        ;
			
		if ( rawEventsFlag ) {
			printf( "EVENT %d %d %f %f\n", obs_id, event_id, ra, dec ) ;
		}
		
		// set the telmask
		telmask_clear( telmask ) ;
		for( unsigned int j_tel = 1 ; j_tel <= NTEL ; j_tel++ )
		{
			telmask_set( telmask, j_tel, readImgSel( ImgSel, j_tel ) ) ;
		}
		
		// convert MJD and Time to UTC
		//event_time  = VSkyCoordinatesUtilities::getUTC( MJD, Time ) ; // using MJD epoch (time in seconds since MJD=0.0)
		//event_time  = (MJD*86400.0) + Time - met_zero ;
		event_time  = ( ( MJD - met_zero ) * 86400.0 ) + Time ;
		
		//
		recEVENTS.write() ;
		//printf( "DATA %6d %8d %4s %17.7f %6.1f %6.1f %6.2f %6.2f %5d %12f\n", (int)obs_id, (int)event_id, telmask_to_string(telmask).c_str(), event_time, ra, dec, Az, El, MJD, Time) ;
		//printf( "%3d: Event %6d to %s\n", i, EventNumber, outfile ) ;
		//if ( i==0 ) recEVENTS.writeHeader( "OBS_ID", RunNumber ) ;
	}
	printf( "RunNumber %d, %d events written.\n", RunNumber, totalEvents ) ;
	
	
	///////////////////////////////////
	// -TELARRAY Table
	recTELARRAY.writeHeader( "TELESCOP", string( "VERITAS" ) ) ;
	recTELARRAY.writeHeader( "ARRAY"   , string( "null" ) ) ;
	/*
	recTELARRAY.writeHeader( "OBS_ID"  ,  ) ;
	recTELARRAY.writeHeader( "GEOLAT"  ,  ) ;
	recTELARRAY.writeHeader( "GEOLON"  ,  ) ;
	recTELARRAY.writeHeader( "ALTITUDE",  ) ;
	*/
	short int telid    = 0   ;
	//long long int telclass = 0   ;
	long long int subclass = 0   ;
	double posx  = 0.0 ;
	double posy  = 0.0 ;
	double posz  = 0.0 ;
	double mirror_area = 0.0 ;
	double cam_area    = 0.0 ;
	double focal_length = 0.0 ;
	double fieldofview  = 0.0 ;
	recTELARRAY.mapColumnToVar( "TELID"    , telid );
	//recTELARRAY.mapColumnToVar( "TELCLASS" , telclass     );
	recTELARRAY.mapColumnToVar( "SUBCLASS" , subclass );
	recTELARRAY.mapColumnToVar( "POSX"     , posx );
	recTELARRAY.mapColumnToVar( "POSY"     , posy );
	recTELARRAY.mapColumnToVar( "POSZ"     , posz );
	recTELARRAY.mapColumnToVar( "MIRAREA"  , mirror_area );
	recTELARRAY.mapColumnToVar( "CAMAREA"  , cam_area );
	recTELARRAY.mapColumnToVar( "FOCLEN"   , focal_length );
	recTELARRAY.mapColumnToVar( "FOV"      , fieldofview );
	//recTELARRAY.mapColumnToVar( "N_PIX"    , npix         );
	//recTELARRAY.mapColumnToVar( "PIX_SIZE" , pix_size     );
	//recTELARRAY.mapColumnToVar( "PIX_SEP"  , pix_sep      );
	
	totalEvents = chainTelConfig->GetEntries() ;
	for( unsigned int i = 0 ; i <= totalEvents - 1 ; i++ )
	{
		if( i > 10 )
		{
			break ;
		}
		chainTelConfig->GetEntry( i );
		telid        = 0 ;
		//telclass     = 0 ;
		subclass     = 0 ;
		posx         = 0.0 ;
		posy         = 0.0 ;
		posz         = 0.0 ;
		mirror_area  = 0.0 ;
		cam_area     = 0.0 ;
		focal_length = 0.0 ;
		fieldofview  = 0.0 ;
		recTELARRAY.write() ;
	}
	
	///////////////////////////////////
	// -GTI Table
	// Good Time Interval
	// in VERITAS, this is the INVERSE table of Time Cuts
	//   VERITAS Time Cuts: list of times that are *UNUSABLE*
	//   GTI: list of times that are *USABLE*
	//   start = when to start the cut time period, from db
	//   stop  = when to stop the cut time period, from db
	//recGTI.writeHeader( "" , ) ;
	recGTI.writeHeader( "MJDREFI" , mjdrefi ) ;
	recGTI.writeHeader( "MJDREFF" , mjdreff ) ;
	recGTI.writeHeader( "TSTART"  , tstart ) ;
	recGTI.writeHeader( "TSTOP"   , tstop ) ;
	recGTI.writeHeader( "TIMESYS" , string( "TT" ) ) ;
	recGTI.writeHeader( "TIMEREF" , string( "local" ) ) ;
	recGTI.writeHeader( "TIMEUNIT", string( "days" ) ) ;
	float gti_start  = 0.0 ;
	float gti_stop   = 0.0 ;
	recGTI.mapColumnToVar( "START" , gti_start );
	recGTI.mapColumnToVar( "STOP"  , gti_stop );
	for( unsigned int i = 0 ; i < gti_Beginning.size() ; i++ )
	{
		gti_start = gti_Beginning[i] ;
		gti_stop  = gti_Ending[i]    ;
		recGTI.write() ;
		printf( " writing GTI: %9.4f to %9.4f \n", gti_Beginning[i], gti_Ending[i] ) ;
	}
	
	// cleanup
	recEVENTS.finishWriting();
	recTELARRAY.finishWriting();
	recGTI.finishWriting();
	anasumfile->Close();
	
	// check output file
	ifstream output_file( INPoutnameStr.c_str() ) ;
	if( ! output_file.good() )
	{
		cout << "Warning, output file " << INPoutnameStr << " may not have been completed." << endl;
		return 1 ;
	}
	else
	{
		cout << "Conversion from " << INPfnameStr.substr( INPfnameStr.find_last_of( "\\/" ) + 1, -1 ) ;
		cout << " to " << INPoutnameStr.substr( INPoutnameStr.find_last_of( "\\/" ) + 1, -1 ) ;
		cout << " complete." << endl;
		return 0 ;
	}
	
	return 1; // if we get to this return something went wrong
}

void getHumanDateTimes( VEvndispRunParameter* evrunpar, char dateobs[], char timeobs[], char dateend[], char timeend[] )
{
	bool debug = false ;
	if( debug )
	{
		cout << "getHumanDateTimes()" << endl;
	}
	if( debug )
	{
		cout << "   startSQL:" << evrunpar->fDBRunStartTimeSQL << endl ;
	}
	if( debug )
	{
		cout << "   stopSQL :" << evrunpar->fDBRunStoppTimeSQL << endl ;
	}
	sscanf( evrunpar->fDBRunStartTimeSQL.c_str(), "%s %s", dateobs, timeobs ) ;
	sscanf( evrunpar->fDBRunStoppTimeSQL.c_str(), "%s %s", dateend, timeend ) ;
	if( debug )
	{
		cout << "   dateobs:|" << dateobs << "|" << endl ;
	}
	if( debug )
	{
		cout << "   timeobs:|" << timeobs << "|" << endl ;
	}
	if( debug )
	{
		cout << "   dateend:|" << dateend << "|" << endl ;
	}
	if( debug )
	{
		cout << "   timeend:|" << timeend << "|" << endl ;
	}
	
}

// met_zero : MJD time (in days) when MET is 0.0
void getRunTimes( VEvndispRunParameter* evrunpar, double met_zero, double& tstart, double& tstop, double& telapse, double& startMJD, double& stopMJD )
{
	bool debug = false ;
	if( debug )
	{
		cout << "getRunTimes()" << endl;
	}
	int sYe = 0 ;
	int sMo = 0 ;
	int sDa = 0 ;
	int sHo = 0 ;
	int sMi = 0 ;
	int sSe = 0 ;
	
	// run start
	if( debug )
	{
		cout << "fDBStart    :" << evrunpar->fDBRunStartTimeSQL.c_str() << endl;
	}
	sscanf( evrunpar->fDBRunStartTimeSQL.c_str(), "%4d-%2d-%2d %2d:%2d:%2d", &sYe, &sMo, &sDa, &sHo, &sMi, &sSe ) ;
	if( debug )
	{
		printf( "start string: %4d %2d %2d - %2d %2d %2d\n", sYe, sMo, sDa, sHo, sMi, sSe ) ;
	}
	int sMJDint = VSkyCoordinatesUtilities::getMJD( sYe, sMo, sDa ) ;
	startMJD = ( ( double )sMJDint ) + ( sHo / 24.0 ) + ( sMi / ( 24.*60. ) ) + ( sSe / ( 24.*60.*60. ) ) ;
	if( debug )
	{
		printf( "startMJD    : %10.4f\n", startMJD ) ;
	}
	
	// run end
	if( debug )
	{
		cout << "fDBStop     :" << evrunpar->fDBRunStoppTimeSQL.c_str() << endl;
	}
	sscanf( evrunpar->fDBRunStoppTimeSQL.c_str(), "%4d-%2d-%2d %2d:%2d:%2d", &sYe, &sMo, &sDa, &sHo, &sMi, &sSe ) ;
	if( debug )
	{
		printf( "stop string : %4d %2d %2d - %2d %2d %2d\n", sYe, sMo, sDa, sHo, sMi, sSe ) ;
	}
	sMJDint = VSkyCoordinatesUtilities::getMJD( sYe, sMo, sDa ) ;
	stopMJD = ( ( double )sMJDint ) + ( sHo / 24.0 ) + ( sMi / ( 24.*60. ) ) + ( sSe / ( 24.*60.*60. ) ) ;
	if( debug )
	{
		printf( "stopMJD     : %10.4f\n", stopMJD ) ;
	}
	
	// final times
	double met_corrected_start = ( startMJD - met_zero ) * 24 * 60 * 60 ;
	double met_corrected_stop  = ( stopMJD  - met_zero ) * 24 * 60 * 60 ;
	double met_duration        = met_corrected_stop - met_corrected_start ;
	if( debug )
	{
		cout << endl;
	}
	if( debug )
	{
		printf( "  met_corrected_start:%14.2f (s)\n", met_corrected_start ) ;
	}
	if( debug )
	{
		printf( "  met_corrected_stop :%14.2f (s)\n", met_corrected_stop ) ;
	}
	if( debug )
	{
		printf( "  met_duration       :%14.2f (s)\n", met_duration ) ;
	}
	tstart  = met_corrected_start ;
	tstop   = met_corrected_stop  ;
	telapse = met_duration       ;
}

// http://stackoverflow.com/questions/631664/accessing-environment-variables-in-c
std::string get_env_var( std::string const& key )
{
	char* val;
	val = getenv( key.c_str() );
	std::string retval = "";
	if( val != NULL )
	{
		retval = val;
	}
	return retval;
}

// fast way to check if file exists
// http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
inline bool file_exists( const std::string& name )
{
	struct stat buffer;
	return ( stat( name.c_str(), &buffer ) == 0 );
}

// sets telmask to all off
void telmask_clear( bool telmask[] )
{
	//cout << "   telmask_clear(): Clearing telmask to 0000" << endl;
	for( unsigned int i = 0 ; i < NTEL ; i++ )
	{
		//printf( "  testing index %3d : %d\n", i, telmask[i] ) ;
		telmask[i] = false ;
		//printf( "                now : %d\n", telmask[i] ) ;
	}
}

// tel: veritas telescope number, T1234 = 1, 2, 3, or 4
void telmask_set( bool telmask[], int tel, bool state )
{
	//cout << "telmask_set(): Setting T" << tel << " to state " << state << "..." << endl;
	if( state )
	{
		telmask[tel - 1] = true  ;
	}
	else
	{
		telmask[tel - 1] = false ;
	}
}

// get the state of one telescope from telmask
// tel: veritas telescope number, T1234 = 1, 2, 3, or 4
bool telmask_get( bool telmask[], int tel )
{
	bool out ;
	out = ( bool )telmask[tel - 1] ;
	//printf("telmask_get(): T%d : %d\n", tel, out ) ;
	return out ;
}

// convert the telmask to a string for humans to read
string telmask_to_string( bool telmask[] )
{
	char telchar[NTEL + 1] ;
	for( unsigned int i = 0 ; i < NTEL ; i++ )
	{
		if( telmask[i] )
		{
			telchar[i] = '1' ;
		}
		else
		{
			telchar[i] = '0' ;
		}
	}
	telchar[NTEL] = '\0' ; // to terminate the char array, because I can't count.
	string outstring = telchar ;
	//cout << "   telmask_to_string():" << outstring << endl;
	return outstring ;
}

// tel: veritas telescope number, T1234 = 1, 2, 3, or 4
bool readImgSel( int ImgSel, int tel )
{
	bitset<8 * sizeof( ULong64_t )> imgsel = ImgSel;
	bool out = imgsel.test( tel - 1 ) ;
	//printf( "   readImgSel( %2d , %d ): %d\n", ImgSel, tel, out ) ;
	return out ;
}

void calc_Avg_RADec_from_Pointing( TChain* ch, double& ra, double& dec )
{

	bool debug = false ;
	if( debug )
	{
		cout << "calc_Avg_RADec_from_Pointing()" << endl;
	}
	//int    readMJD = 0   ;
	//double readTi  = 0.0 ;  // time in seconds since beginning of day 'readMJD'
	double readRA  = 0.0 ;
	double readDE  = 0.0 ;
	//ch->SetBranchAddress( "MJD"         , &readMJD ) ;
	//ch->SetBranchAddress( "Time"        , &readTi  ) ;
	ch->SetBranchAddress( "TelRAJ2000"  , &readRA ) ;
	ch->SetBranchAddress( "TelDecJ2000" , &readDE ) ;
	unsigned int npts = ch->GetEntries() ;
	if( debug )
	{
		cout << "  Looping over " << npts << " rows of pointing info." << endl;
	}
	double meanRA = 0.0 ;
	double meanDE = 0.0 ;
	unsigned int i = 0 ;
	for( i = 0; i <= npts - 1 ; i++ )
	{
		ch->GetEntry( i );
		if( debug )
		{
			printf( "  i:%2d  ra:%11.9f  dec:%11.9f\n", i, readRA, readDE ) ;
		}
		meanRA += readRA ;
		meanDE += readDE ;
	}
	ra  = meanRA / i ;
	dec = meanDE / i ;
	
	// convert from radians to degrees
	ra  *= TMath::RadToDeg() ;
	dec *= TMath::RadToDeg() ;
	
	// ResetBranchAddresses() of the chain we just used.
	// because if we dont, it will silently fail,
	// and will rear its ugly head later when you try to do new
	// SetBranchAddress.  God Damn You ROOT.
	ch->ResetBranchAddresses() ;
	if( debug )
	{
		cout << "  ra :" << ra  << endl;
	}
	if( debug )
	{
		cout << "  dec:" << dec << endl;
	}
	if( debug )
	{
		cout << "  calc_Avg_RADec_from_Pointing() done." << endl;
	}
}

// for a given time mask 'vtm', find beginning and end of all time-regions which pass cuts (GTI's).
// will search for at most 'maxSecond' seconds from the beginning of the run
// will check for the edge of a cut-passing time region every 'tInterval' seconds
// will return a vector of the beginning and ends of the GTIs
// the returned GTI times will be correct to within +-(0.5*tInterval)
void calcGTIsFromTimeMask( VTimeMask* vtm, const double tInterval, const double maxSeconds, const double startMJD, const double stopMJD, vector<double>& gti_beg, vector<double>& gti_end )
{

	// time we are currently checking
	double searchTime = 0.0 ;
	double IsearchTime = 0.0 ;
	
	// last iteration's
	bool searchStatus  = false ; // false= time does not pass cuts
	bool lastStatus    = false ; // true = time does pass cuts
	bool IsearchStatus = false ;
	bool IlastStatus   = false ;
	
	// maximum number of loops to try
	int maxIter = ( int )( maxSeconds / tInterval ) ;
	
	// flag to exit both loops
	bool breakFlag = false ;
	
	// Loop over entire run duration, searching for GTIs
	for( int i = 0 ; i < maxIter ; i++ )
	{
	
		// instant in time we're checking
		searchTime = startMJD + ( ( i * tInterval ) / ( 24.*60.*60. ) ) ;
		
		// if we're at the end of the run, then break out of the loop
		if( searchTime > stopMJD )
		{
			break ;
		}
		
		// see if this point in time passes the cuts
		searchStatus = vtm->checkAgainstMask( searchTime ) ;
		
		// if we go from a does-not-pass-cuts instant to a does-pass-cuts instant
		// then its the start of a GTI
		// and we should start searching forwards for the end, aka
		// when we go from a does-pass-cuts instant to a does-not-pass-cuts instant
		if( searchStatus && !lastStatus )
		{
		
			// reset interior loop variables
			IsearchTime   = 0 ;
			IsearchStatus = false ;
			IlastStatus   = false ;
			for( int j = 0 ; j < maxIter ; j++ )
			{
				IsearchTime = searchTime + ( ( j * tInterval ) / ( 24.*60.*60. ) ) ;
				
				// if we're at the end of the run, then break out of the loop
				if( IsearchTime > stopMJD )
				{
					breakFlag = true ;
					break ;
				}
				
				// see if this point in time passes the cut mask
				IsearchStatus = vtm->checkAgainstMask( IsearchTime ) ;
				
				// if we go from a does-pass-cuts instant to a does-not-pass-cuts instant
				// OR, if we get to the end of the run
				// then we have reached the end of this GTI
				if( ( !IsearchStatus && IlastStatus ) || IsearchTime > stopMJD )
				{
				
					// congrats, you've found a complete GTI
					gti_beg.push_back( searchTime ) ;  // -((0.5*tInterval)/(24.*60.*60.)) ) ;
					gti_end.push_back( IsearchTime ) ; // -((0.5*tInterval)/(24.*60.*60.)) ) ;
					
					// continue searching for a next GTI where the old one left off
					i = i + j ;
					
					// if we're at the end of the run, jump out of both loops
					// break out of this loop
					break ;
				}
				IlastStatus = IsearchStatus ;
			}
			
			// if the inner loop set 'breakFlag', we should exit the outer loop too
			if( breakFlag )
			{
				break ;
			}
		}
		
		lastStatus = searchStatus ;
	}
	
	if( gti_beg.size() != gti_end.size() )
	{
		cout << "Warning, gti_beg and gti_end aren't the same size." << endl;
		cout << "Problem finding/saving GTI edge?" << endl;
	}
	if( gti_beg.size() == 0 || gti_end.size() == 0 )
	{
		//cout << "Warning, gti_beg/gti_end is size 0." << endl;
		cout << "No GTI's found, assuming entire run is usable..." << endl;
	}
}


// Find the 'median time of the run',
// then return that time's alt/az
// because right now, ctools only accepts 1 alt/az
// per Observation (run)
void calc_Avg_AltAz_from_Pointing( TChain* ch, double& alt, double& az )
{
	bool debug = false ;
	if( debug )
	{
		cout << "calc_Avg_AltAz_from_Pointing()" << endl;
	}
	unsigned int MJD = 0   ;
	double TIM = 0.0 ;  // time in seconds since beginning of day 'readMJD'
	double EL  = 0.0 ;
	double AZ  = 0.0 ;
	ch->SetBranchAddress( "MJD"         , &MJD ) ;
	ch->SetBranchAddress( "Time"        , &TIM ) ;
	ch->SetBranchAddress( "TelAzimuth"  , &AZ ) ;
	ch->SetBranchAddress( "TelElevation", &EL ) ;
	int npts = ch->GetEntries() ;
	int i    = 0 ;
	
	// Find Avg Azimuth and Avg Elevation
	double totAZ = 0.0 ;
	double totEL = 0.0 ;
	for( i = 0 ; i <= npts - 1 ; i++ )
	{
		ch->GetEntry( i ) ;
		totAZ += AZ ;
		totEL += EL ;
	}
	az  = totAZ / npts ;
	alt = totEL / npts ;
	
	/*
	THE FOLLOWING IS NOT USED
	FINDS THE ALT/AZIM AT THE MEDIAN TIME OF THE RUN
	MAY SOMETIMES PICK LESS-THAN-USEFUL ALT/AZIM
	
	// Finding The 'middle' time/az/el
	// aka, find the median time in the run,
	// and grab that time's az/el
	
	// the row times in pointingDataReduced are from
	// the whole seconds since the beginning of the
	// run, so they might be as much as 0.999
	// seconds off from the beginning of the run, so
	// we're just going to get the
	// Floor(numberOfRows/2)'th row and call it good
	// for now.
	int igrab = (int)(npts/2) ;
	if (debug) cout << "  npts:" << npts  << endl;
	if (debug) cout << "  grab:" << igrab << endl;
	ch->GetEntry(igrab);
	az  = AZ ;
	alt = EL ;
	if (debug) printf( "  az :%8.4f\n", az  ) ;
	if (debug) printf( "  alt:%8.4f\n", alt ) ;
	if (debug) cout << "  MJD:" << MJD << endl ;
	if (debug) cout << "  TIM:" << TIM << endl ;
	
	// display middle 10 rows, for debugging
	if (debug) {
	    int debugmin = ((int)(npts/2)) - 4 ;
	    int debugmax = ((int)(npts/2)) + 4 ;
	    int i        = 0                   ;
	    string mark = "blah" ;
	    for( i=debugmin; i<=debugmax ; i++ ) {
	        if ( i == igrab ) mark = ">" ;
	        else              mark = " " ;
	        ch->GetEntry(i);
	        printf( " %si:%3d  MJD:%5d  Time:%7.4f  Az:%7.3f  El:%7.4f\n",
	                mark.c_str(), i, MJD, TIM, AZ, EL ) ;
	    }
	}
	*/
	
	// ResetBranchAddresses() of the chain we just used.
	// Because if we dont, it will silently fail,
	// and will rear its ugly head later when you try to do new
	// SetBranchAddress.  God Damn You, ROOT.
	ch->ResetBranchAddresses() ;
	if( debug )
	{
		cout << "  calc_Avg_AltAz_from_Pointing() done." << endl;
	}
}
