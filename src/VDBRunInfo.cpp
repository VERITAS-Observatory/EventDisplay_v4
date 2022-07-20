/*! \class VDBRunInfo
    \brief get run info from DB

*/

#include "VDBRunInfo.h"

//
// irun      : runnumber to pull info for
// iDBserver : database url to get runinfo from, usually from VGlobalRunParameter->getDBServer()
// iNTel     : number of telescopes to look for?
VDBRunInfo::VDBRunInfo( int irun, string iDBserver, unsigned int iNTel, string iDBTextDirectory )
{
	fRunNumber = irun;
	fDBStatus = false;
	fDBServer = iDBserver;
	fDBTextDirectory = iDBTextDirectory;
	
	fNTel = iNTel;
	fTargetName = "";
	fTargetDec = -99.;
	fTargetRA = -99.;
	fWobbleNorth = 0.;
	fWobbleEast = 0.;
	fConfigMask = 0;
	fTelToAna = 1234;
	fRunType = "";
	fObservingMode = "";
	fRunStatus = "";
	fWeather = "";
	fDataStoppTimeSQL = "";
	fDataStoppTime = 0;
	fDataStartTimeSQL = "";
	fDataStartTime = 0;
	fDataStartTimeHMS = 0;
	fDataStoppTimeHMS = 0;
	fDataStartTimeMJD = 0.;
	fDataStoppTimeMJD = 0.;
	fDuration = 0;
	
	if( fDBTextDirectory.size() == 0 )
	{
		readRunInfoFromDB();
		readRunDQM();
		readLaserRun();
	}
	else
	{
		readRunInfoFromDBTextFile();
		readTargetFromDBTextFile();
		readRunDQMFromDBTextFile();
		readLaserFromDBTextFile();
	}
}

void VDBRunInfo::readRunDQM()
{
	int config_mask_new = readRunDQM( fRunNumber, getConfigMask() );
	fConfigMask = config_mask_new;
	set_telescope_to_analyse();
}

unsigned int VDBRunInfo::readRunDQM( int run_number, unsigned int config_mask )
{
	unsigned int ConfigMaskDQM = 0;
	
	stringstream iTempS;
	iTempS << fDBServer << "/VOFFLINE";
	char c_query[1000];
	sprintf( c_query, "SELECT run_id , data_category   , status   , status_reason , tel_cut_mask , usable_duration , time_cut_mask , light_level , vpm_config_mask , authors  , comment from tblRun_Analysis_Comments where run_id=%d", run_number );
	
	VDB_Connection my_connection( iTempS.str().c_str(), "readonly", "" ) ;
	if( !my_connection.Get_Connection_Status() )
	{
		return config_mask;
	}
	if( !my_connection.make_query( c_query ) )
	{
		return config_mask;
	}
	TSQLResult* db_res = my_connection.Get_QueryResult();
	
	TSQLRow* db_row = db_res->Next();
	if( !db_row )
	{
		cout << "VDBRunInfo:readRunDQM:Info no row in VOFFLINE DB for run " << run_number << endl;
		return config_mask;
	}
	if( db_row->GetField( 4 ) )
	{
		ConfigMaskDQM = ( unsigned int )( atoi( db_row->GetField( 4 ) ) );
	}
	else
	{
		return config_mask;
	}
	
	// Check if the mask is 0
	if( ConfigMaskDQM == 0 )
	{
		return config_mask;
	}
	return get_dqm_configmask( config_mask, ConfigMaskDQM );
}

unsigned int VDBRunInfo::get_dqm_configmask( unsigned int config_mask, unsigned int ConfigMaskDQM )
{
	bitset<4> bitConfig( config_mask );
	bitset<4> bitDQM( ConfigMaskDQM );
	bitset<4> bitNDQM;
	
	for( int i = 0; i < ( int )bitDQM.size(); i++ )
	{
		bitNDQM.set( ( int )bitDQM.size() - i - 1, bitDQM.test( i ) );
	}
	
	bitNDQM = ~bitNDQM;
	bitset<4> bitNewConfig = bitConfig & bitNDQM;
	unsigned int ConfigMaskNew = 0;
	
	for( int i = 0; i < ( int )bitNewConfig.size(); i++ )
	{
		if( bitNewConfig.test( i ) )
		{
			ConfigMaskNew += ( unsigned int )pow( 2., i );
		}
	}
	return ConfigMaskNew;
}

void VDBRunInfo::readRunInfoFromDB()
{
	stringstream iTempS;
	iTempS << fDBServer << "/VERITAS";
	char c_query[1000];
	sprintf( c_query, "select * from tblRun_Info where run_id=%d", fRunNumber );
	
	VDB_Connection my_connection( iTempS.str(), "readonly", "" ) ;
	if( !my_connection.Get_Connection_Status() )
	{
		fDBStatus = false;
		return;
	}
	if( !my_connection.make_query( c_query ) )
	{
		fDBStatus = false;
		return;
	}
	TSQLResult* db_res = my_connection.Get_QueryResult();
	
	TSQLRow* db_row = db_res->Next();
	if( !db_row )
	{
		cout << "VDBRunInfo: failed reading a row from DB for run " << fRunNumber << endl;
		fDBStatus = false;
		return;
	}
	// get date
	if( db_row->GetField( 4 ) )
	{
		string iTemp = db_row->GetField( 4 );
		if( iTemp.size() > 8 )
		{
			fDBDate = get_time_ymd( iTemp );
		}
		else
		{
			fDBDate = 0;
		}
	}
	else
	{
		fDBDate = 0;
	}
	if( db_row->GetField( 6 ) )
	{
		fDataStartTimeSQL = get_time_sql( db_row->GetField( 6 ) );
		fDataStartTimeMJD = get_time_MJD( db_row->GetField( 6 ) );
		fDataStartTimeHMS = get_time_HMS( db_row->GetField( 6 ) );
		fDataStartTime = get_time_seconds_of_date( db_row->GetField( 6 ) );
	}
	else
	{
		fDataStartTime = 0;
		fDataStartTimeHMS = 0;
		fDataStartTimeSQL = "";
	}
	if( db_row->GetField( 7 ) )
	{
		fDataStoppTimeSQL = get_time_sql( db_row->GetField( 7 ) );
		fDataStoppTimeHMS = get_time_HMS( db_row->GetField( 7 ) );
		fDataStoppTimeMJD = get_time_MJD( db_row->GetField( 7 ) );
		fDataStoppTime = get_time_seconds_of_date( db_row->GetField( 7 ) );
	}
	else
	{
		fDataStoppTime = 0;
		fDataStoppTimeHMS = 0;
		fDataStoppTimeSQL = "";
	}
	
	if( db_row->GetField( 8 ) )
	{
		fDuration = get_duration( db_row->GetField( 8 ) );
	}
	else
	{
		fDuration = 0.;
	}
	if( TMath::Abs( fDuration < 1.e-4 ) )
	{
		fDuration = get_duration_from_sql_string();
	}
	
	if( db_row->GetField( 19 ) )
	{
		fTargetName = db_row->GetField( 19 );
	}
	else
	{
		fTargetName = "";
	}
	if( db_row->GetField( 1 ) )
	{
		fRunType = db_row->GetField( 1 );
	}
	if( db_row->GetField( 2 ) )
	{
		fObservingMode = db_row->GetField( 2 );
	}
	if( db_row->GetField( 3 ) )
	{
		fRunStatus = db_row->GetField( 3 );
	}
	if( db_row->GetField( 9 ) )
	{
		fWeather = db_row->GetField( 9 );
	}
	if( db_row->GetField( 17 ) && db_row->GetField( 18 ) )
	{
		fWobbleNorth = get_wobble_north( db_row->GetField( 17 ), db_row->GetField( 18 ) );
		fWobbleEast = get_wobble_east( db_row->GetField( 17 ), db_row->GetField( 18 ) );
	}
	
	// get config mask
	if( db_row->GetField( 10 ) )
	{
		fConfigMask = ( unsigned int )( atoi( db_row->GetField( 10 ) ) );
	}
	else
	{
		fConfigMask = 0;
	}
	set_telescope_to_analyse();
	
	// get source coordinates
	sprintf( c_query, "select * from tblObserving_Sources where source_id like convert( _utf8 \'%s\' using latin1)", fTargetName.c_str() );
	if( !my_connection.make_query( c_query ) )
	{
		fDBStatus = false;
		return;
	}
	db_res = my_connection.Get_QueryResult();
	
	db_row = db_res->Next();
	if( !db_row )
	{
		cout << " VDBRunInfo::readRunInfoFromDB warning :  no source name in tblObserving_Sources like the name of the target for this run ("
			 << fTargetName << ")" << endl;
	}
	else
	{
		fTargetDec = atof( db_row->GetField( 2 ) ) * 180. / TMath::Pi();
		fTargetRA = atof( db_row->GetField( 1 ) ) * 180. / TMath::Pi();
	}
	
	fDBStatus = true;
	return;
}


void VDBRunInfo::print()
{
	cout << "Run info for run " << fRunNumber << ":" << endl;
	cout << "Date: " << fDBDate << "(" << fDataStartTimeSQL << "," << fDataStoppTimeSQL << ")";
	cout << ", Duration: " << fDuration << " [s]";
	cout << ", " << fRunType << ", " << fObservingMode << ", " << fRunStatus;
	cout << ", Weather: " << fWeather << endl;
	cout << "Target: " << fTargetName;
	cout << " (ra,dec)(J2000)=(" << fTargetRA << ", " << fTargetDec << ")";
	cout << ", Wobble (N,E): " << fWobbleNorth << ", " << fWobbleEast << "), TelToAna: " << fTelToAna << endl;
	cout << "Laser/Flasher runs: ";
	for( unsigned int i = 0; i < getLaserRun().size(); i++ )
	{
		cout << "T" << i + 1 << ": " << getLaserRun()[i] << "   ";
	}
	cout << endl;
}


vector< unsigned int > VDBRunInfo::readLaserRun()
{
	stringstream iTempS;
	iTempS << fDBServer << "/VERITAS";
	char c_query[1000];
	
	sprintf( c_query, "SELECT info.run_id, grp_cmt.excluded_telescopes, info.config_mask FROM tblRun_Info AS info, tblRun_Group AS grp, tblRun_GroupComment AS grp_cmt, (SELECT group_id FROM tblRun_Group WHERE run_id=%d) AS run_grp WHERE grp_cmt.group_id = run_grp.group_id AND grp_cmt.group_type='laser' AND grp_cmt.group_id=grp.group_id AND grp.run_id=info.run_id AND (info.run_type='flasher' OR info.run_type='laser')", fRunNumber );
	
	//std::cout<<"VDBRunInfo::getLaserRun "<<std::endl;
	VDB_Connection my_connection( iTempS.str(), "readonly", "" ) ;
	if( !my_connection.Get_Connection_Status() )
	{
		fDBStatus = false;
		return fLaserRunID;
	}
	if( !my_connection.make_query( c_query ) )
	{
		fDBStatus = false;
		return fLaserRunID;
	}
	TSQLResult* db_res = my_connection.Get_QueryResult();
	
	vector< unsigned int > iLaserList;
	vector< unsigned int > iLaserConfigMask;
	vector< unsigned int > iLaserExclude;
	
	if( db_res->GetRowCount() > 0 )
	{
		while( TSQLRow* db_row = db_res->Next() )
		{
			if( !db_row )
			{
				cout << "VDBRunInfo: failed reading a row from DB for run " << fRunNumber << endl;
				fDBStatus = false;
				return fLaserRunID;
			}
			iLaserList.push_back( atoi( db_row->GetField( 0 ) ) );
			iLaserExclude.push_back( atoi( db_row->GetField( 1 ) ) );
			iLaserConfigMask.push_back( atoi( db_row->GetField( 2 ) ) );
		}
	}
	else
	{
		cout << "WARNING: VDBRunInfo::readLaserRun() no laser run found for telescope " << fNTel << " and run " << fRunNumber << endl;
	}
	
	set_laser_run( iLaserList, iLaserExclude, iLaserConfigMask );
	
	return fLaserRunID;
}

void VDBRunInfo::set_laser_run(
	vector< unsigned int > iLaserList,
	vector< unsigned int > iLaserExclude,
	vector< unsigned int > iLaserConfigMask )
{
	fLaserRunID.assign( fNTel, 0 );
	for( unsigned int t = 0; t < fNTel; t++ )
	{
		// check if this run is excluded from group
		// also check if telescope is within the config mask (taking DQM cuts into account)
		for( unsigned int i = 0; i < iLaserList.size(); i++ )
		{
			bitset< 8 > ibit( iLaserExclude[i] );
			unsigned int config_mask = 0;
			if( fDBTextDirectory.size() == 0 )
			{
				config_mask = readRunDQM( iLaserList[i], iLaserConfigMask[i] );
			}
			else
			{
				config_mask = readRunDQMFromDBTextFile( iLaserList[i], iLaserConfigMask[i] );
			}
			bitset< 8 > ibit_mask( config_mask );
			if( !ibit.test( t ) && ibit_mask.test( t ) )
			{
				fLaserRunID[t] = iLaserList[i];
			}
		}
	}
}

int VDBRunInfo::get_time_ymd( string iTemp )
{
	return atoi( iTemp.substr( 0, 4 ).c_str() ) * 10000
		   + atoi( iTemp.substr( 5, 2 ).c_str() ) * 100 + atoi( iTemp.substr( 8, 2 ).c_str() );
}

double VDBRunInfo::get_time_MJD( string iTemp )
{
	if( iTemp.size() < 9 )
	{
		return 0.;
	}
	int iTime = get_time_ymd( iTemp );
	if( iTime <= 0 )
	{
		return 0.;
	}
	double imjd = 0.;
	int j = 0;
	int iy = iTime / 10000;
	int im = ( iTime - iy * 10000 ) / 100;
	int id = iTime - iy * 10000 - im * 100;
	if( iTime > 0 )
	{
		VAstronometry::vlaCldj( iy, im, id, &imjd, &j );
	}
	return imjd;
}

int VDBRunInfo::get_time_HMS( string iTemp )
{
	if( iTemp.size() < 9 )
	{
		return 0.;
	}
	return atoi( iTemp.substr( 11, 2 ).c_str() ) * 10000
		   + atoi( iTemp.substr( 14, 2 ).c_str() ) * 100 + atoi( iTemp.substr( 17,  2 ).c_str() );
}

int VDBRunInfo::get_time_seconds_of_date( string iTemp )
{
	if( iTemp.size() < 9 )
	{
		return 0.;
	}
	return atoi( iTemp.substr( 11, 2 ).c_str() ) * 60 * 60
		   + atoi( iTemp.substr( 14, 2 ).c_str() ) * 60 + atoi( iTemp.substr( 17, 2 ).c_str() );
}

int VDBRunInfo::get_duration( string iTemp )
{
	if( iTemp.size() < 8 )
	{
		return 0.;
	}
	return atoi( iTemp.substr( 0, 1 ).c_str() ) * 3600 + atoi( iTemp.substr( 3, 4 ).c_str() ) * 60 + atoi( iTemp.substr( 6, 7 ).c_str() );
}

int VDBRunInfo::get_duration_from_sql_string()
{
	double mjd = 0.;
	double isec_start = 0.;
	double isec_stopp = 0.;
	VSkyCoordinatesUtilities::getMJD_from_SQLstring( fDataStartTimeSQL, mjd, isec_start );
	VSkyCoordinatesUtilities::getMJD_from_SQLstring( fDataStoppTimeSQL, mjd, isec_stopp );
	return ( int )( isec_stopp - isec_start );
}

string VDBRunInfo::get_time_sql( string iTemp )
{
	if( iTemp.size() < 9 )
	{
		return "";
	}
	return iTemp;
}

double VDBRunInfo::get_wobble_north( string dist, string angle )
{
	double w = atof( dist.c_str() ) * cos( atof( angle.c_str() ) * TMath::DegToRad() );
	if( TMath::Abs( w ) < 1.e-15 )
	{
		return 0.;
	}
	return w;
}

double VDBRunInfo::get_wobble_east( string dist, string angle )
{
	double w = atof( dist.c_str() ) * sin( atof( angle.c_str() ) * TMath::DegToRad() );
	if( TMath::Abs( w ) < 1.e-15 )
	{
		return 0.;
	}
	return w;
}

void VDBRunInfo::set_telescope_to_analyse()
{
	if( fConfigMask == 1 )
	{
		fTelToAna = 1;
	}
	else if( fConfigMask == 2 )
	{
		fTelToAna = 2;
	}
	else if( fConfigMask == 3 )
	{
		fTelToAna = 12;
	}
	else if( fConfigMask == 4 )
	{
		fTelToAna = 3;
	}
	else if( fConfigMask == 5 )
	{
		fTelToAna = 13;
	}
	else if( fConfigMask == 6 )
	{
		fTelToAna = 23;
	}
	else if( fConfigMask == 7 )
	{
		fTelToAna = 123;
	}
	else if( fConfigMask == 8 )
	{
		fTelToAna = 4;
	}
	else if( fConfigMask == 9 )
	{
		fTelToAna = 14;
	}
	else if( fConfigMask == 10 )
	{
		fTelToAna = 24;
	}
	else if( fConfigMask == 11 )
	{
		fTelToAna = 124;
	}
	else if( fConfigMask == 12 )
	{
		fTelToAna = 34;
	}
	else if( fConfigMask == 13 )
	{
		fTelToAna = 134;
	}
	else if( fConfigMask == 14 )
	{
		fTelToAna = 234;
	}
	else if( fConfigMask == 15 )
	{
		fTelToAna = 1234;
	}
}

bool VDBRunInfo::readRunInfoFromDBTextFile()
{
	VSQLTextFileReader a( fDBTextDirectory, fRunNumber, "runinfo" );
	if( !a.isGood() )
	{
		return false;
	}
	if( a.getValue_from_key( "db_start_time" ).size() > 0 )
	{
		fDBDate = get_time_ymd( a.getValue_from_key( "db_start_time" ) );
	}
	if( a.getValue_from_key( "data_start_time" ).size() > 0 )
	{
		fDataStartTimeSQL = get_time_sql( a.getValue_from_key( "data_start_time" ) );
		fDataStartTimeMJD = get_time_MJD( a.getValue_from_key( "data_start_time" ) );
		fDataStartTimeHMS = get_time_HMS( a.getValue_from_key( "data_start_time" ) );
		fDataStartTime = get_time_seconds_of_date( a.getValue_from_key( "data_start_time" ) );
	}
	if( a.getValue_from_key( "data_end_time" ).size() > 0 )
	{
		fDataStoppTimeSQL = get_time_sql( a.getValue_from_key( "data_end_time" ) );
		fDataStoppTimeMJD = get_time_MJD( a.getValue_from_key( "data_end_time" ) );
		fDataStoppTimeHMS = get_time_HMS( a.getValue_from_key( "data_end_time" ) );
		fDataStoppTime = get_time_seconds_of_date( a.getValue_from_key( "data_end_time" ) );
	}
	if( a.getValue_from_key( "duration" ).size() > 0 )
	{
		fDuration  = get_duration( a.getValue_from_key( "duration" ) );
	}
	if( TMath::Abs( fDuration < 1.e-4 ) )
	{
		fDuration = get_duration_from_sql_string();
	}
	if( a.getValue_from_key( "source_id" ).size() > 0 )
	{
		fTargetName = a.getValue_from_key( "source_id" );
	}
	if( a.getValue_from_key( "run_type" ).size() > 0 )
	{
		fRunType = a.getValue_from_key( "run_type" );
	}
	if( a.getValue_from_key( "observing_mode" ).size() > 0 )
	{
		fObservingMode = a.getValue_from_key( "observing_mode" );
	}
	if( a.getValue_from_key( "run_status" ).size() > 0 )
	{
		fRunStatus = a.getValue_from_key( "run_status" );
	}
	if( a.getValue_from_key( "weather" ).size() > 0 )
	{
		fWeather = a.getValue_from_key( "weather" );
	}
	if( a.getValue_from_key( "offset_distance" ).size() > 0 && a.getValue_from_key( "offset_angle" ).size() > 0 )
	{
		fWobbleNorth = get_wobble_north( a.getValue_from_key( "offset_distance" ), a.getValue_from_key( "offset_angle" ) );
		fWobbleEast = get_wobble_east( a.getValue_from_key( "offset_distance" ), a.getValue_from_key( "offset_angle" ) );
	}
	if( a.getValue_from_key( "config_mask" ).size() > 0 )
	{
		fConfigMask = ( unsigned int )( atoi( a.getValue_from_key( "config_mask" ).c_str() ) );
		set_telescope_to_analyse();
	}
	fDBStatus = true;
	return true;
}

bool VDBRunInfo::readTargetFromDBTextFile()
{
	VSQLTextFileReader a( fDBTextDirectory, fRunNumber, "target" );
	if( !a.isGood() )
	{
		return false;
	}
	string i_dec = a.getValue_from_key( "decl", "source_id", fTargetName );
	string i_ra = a.getValue_from_key( "ra", "source_id", fTargetName );
	if( i_dec.size() > 0 && i_ra.size() > 0 )
	{
		fTargetDec = atof( i_dec.c_str() ) * TMath::RadToDeg();
		fTargetRA =  atof( i_ra.c_str() ) * TMath::RadToDeg();
	}
	else
	{
		cout << "Error reading target (" << fTargetName << ") from DB text file" << endl;
		return false;
	}
	
	return true;
}

vector< unsigned int > VDBRunInfo::readLaserFromDBTextFile()
{
	vector< unsigned int > iTemp;
	VSQLTextFileReader a( fDBTextDirectory, fRunNumber, "laserrun" );
	if( !a.isGood() )
	{
		return iTemp;
	}
	vector< unsigned int > iLaserList = a.getValueVector_from_key_as_integer( "run_id" );
	vector< unsigned int > iLaserConfigMask = a.getValueVector_from_key_as_integer( "config_mask" );
	vector< unsigned int > iLaserExclude = a.getValueVector_from_key_as_integer( "excluded_telescopes" );
	
	set_laser_run( iLaserList, iLaserExclude, iLaserConfigMask );
	
	return fLaserRunID;
}

void VDBRunInfo::readRunDQMFromDBTextFile()
{
	fConfigMask = readRunDQMFromDBTextFile( fRunNumber, getConfigMask() );
	set_telescope_to_analyse();
}


unsigned int VDBRunInfo::readRunDQMFromDBTextFile( int run_number, unsigned int config_mask )
{
	unsigned int ConfigMaskDQM = 0;
	
	VSQLTextFileReader a( fDBTextDirectory, fRunNumber, "rundqm" );
	if( !a.isGood() )
	{
		return config_mask;
	}
	string i_config_mask = a.getValue_from_key( "tel_cut_mask", "run_id", to_string( run_number ) );
	if( i_config_mask.size() > 0 && i_config_mask != "NULL" )
	{
		ConfigMaskDQM = atoi( i_config_mask.c_str() );
	}
	if( ConfigMaskDQM == 0 )
	{
		return config_mask;
	}
	return get_dqm_configmask( config_mask, ConfigMaskDQM );
}
