///////////////////////////////
//  Take a single runnumber, and print the elevation and azimuth
//  the telescope is pointing at once per second
#include "CData.h"
#include "VSkyCoordinates.h"
#include "VDB_Connection.h"
#include "VGlobalRunParameter.h"
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TSQLServer.h>
#include <bitset>
void getDBMJDTime( string itemp, int& MJD, double& Time, bool bStrip );
using namespace std;

int main( int argc, char* argv[] )
{
	// parse argument
	if( argc != 2 )
	{
		cout << "Error, needs runnumber argument." << endl;
		cout << "For Example:" << endl;
		cout << "  $ VTS.getRun_TimeElevAzim 65765" << endl;
		exit( 1 ) ;
	}
	int runnumber = atoi( argv[1] ) ;
	if( runnumber < 9999 or runnumber > 99999 )
	{
		cout << "Error, needs valid runnumber (9999-99999) (read in " << runnumber << ")." << endl;
		exit( 1 );
	}
	//printf( "examining run %d...\n", runnumber ) ;
	
	VGlobalRunParameter* blah = new VGlobalRunParameter() ;
	cout << " VGlobal->getDBServer(): " << blah->getDBServer() << endl;
	// start connection
	//string db_name = "mysql://romulus.ucsc.edu/" ;
	//TSQLServer * f_db = TSQLServer::Connect( db_name.c_str(), "readonly" , "" );
	// fixed below to follow the party line
	string tmpdb_ver;
	tmpdb_ver = blah->getDBServer() + "/VERITAS" ;
	cout << "Using server " << tmpdb_ver << endl;
	VDB_Connection my_connection_ver( tmpdb_ver.c_str(), "readonly", "" ) ;
	if( !my_connection_ver.Get_Connection_Status() )
	{
		cout << "error connecting to db" << endl;
		return -1;
	}
	
	// get run start and finish, # of telescopes
	char c_query[1000] ;
	sprintf( c_query, "select run_id, data_start_time, data_end_time, config_mask from tblRun_Info where run_id = %d", runnumber ) ;
	//printf( "%s\n", query ) ;
	if( !my_connection_ver.make_query( c_query ) )
	{
		cout << "Error, unable to get response from db for query \"" << c_query << "\"" << endl;
		cout << "Exiting..." << endl;
		return -1;
	}
	
	//TSQLResult *db_res = f_db->Query( query );
	TSQLResult* db_res = my_connection_ver.Get_QueryResult();
	
	if( !db_res )
	{
		cout << "TSQLResult empty, exiting..." << endl ;
		exit( 1 ) ;
	}
	int fNRows = db_res->GetRowCount();
	string itemp;
	double MJDStart = 0.0, MJDEnd = 0.0 ;
	unsigned long int ImgSel = 0 ;
	for( int j = 0; j < fNRows; j++ )
	{
		TSQLRow* db_row = db_res->Next();
		if( !db_row )
		{
			break;
		}
		int iMJD = 0;
		double iTime = 0.;
		
		itemp = db_row->GetField( 1 );
		getDBMJDTime( itemp, iMJD, iTime, true );
		MJDStart = ( double )iMJD + iTime / 86400.0;
		//printf("date to MJD: %s : %f : sec %f\n", itemp.c_str(), MJDStart, (MJDStart-(int)MJDStart)*86400.0 ) ;
		
		itemp = db_row->GetField( 2 );
		getDBMJDTime( itemp, iMJD, iTime, true );
		MJDEnd = ( double )iMJD + iTime / 86400.0;
		//printf("date to MJD: %s : %f : sec %f\n", itemp.c_str(), MJDEnd, (MJDEnd-(int)MJDEnd)*86400.0 ) ;
		
		ImgSel = ( unsigned long int )db_row->GetField( 3 ) ;
	}
	
	//printf( "Run %d started at %f and ended at %f, using telescope combo %lu\n", runnumber, MJDStart, MJDEnd, ImgSel ) ;
	
	string tmpdb_vof;
	tmpdb_vof = blah->getDBServer() + "/VOFFLINE" ;
	VDB_Connection my_connection_vof( tmpdb_ver.c_str(), "readonly", "" ) ;
	if( !my_connection_vof.Get_Connection_Status() )
	{
		cout << "error connecting to db VOFFLINE" << endl;
		return -1;
	}
	
	// request pointing data
	vector <double> mjd  ;
	vector <int>    mjdDay ;
	vector <double> mjdDayFraction ;
	vector <double> mjdSecondsOfDay ;
	vector <double> ra   ;
	vector <double> decl ;
	bitset<8 * sizeof( ULong64_t )> a = ImgSel;
	int telescope = -1 ;
	for( unsigned int i_tel = 0 ; i_tel < 4 ; i_tel++ )
	{
		if( ! a.test( i_tel ) )
		{
			continue ;
		}
		sprintf( c_query, "select mjd, ra, decl from VOFFLINE.tblPointing_Monitor_Telescope%d_Calibrated_Pointing where mjd > %f and mjd < %f ;", i_tel, MJDStart, MJDEnd ) ;
		//printf( "%s\n", query) ;
		if( !my_connection_vof.make_query( c_query ) )
		{
			cout << "Error, unable to get response from db for query \"" << c_query << "\"" << endl;
			cout << "Exiting..." << endl;
			return -1;
		}
		
		//TSQLResult *db_res2 = f_db->Query( query );
		TSQLResult* db_res2 = my_connection_vof.Get_QueryResult();
		fNRows = db_res2->GetRowCount();
		//int count=0 ;
		//printf( "%d rows\n", fNRows ) ;
		double imjd, ira, idecl ;
		for( int j = 0; j < fNRows; j++ )
		{
			TSQLRow* db_row = db_res2->Next();
			if( !db_row )
			{
				break;
			}
			//if ( count<8 ) { count++ ; printf( "Row %d  :  %s  :  %s  :  %s\n", j, db_row->GetField(0), db_row->GetField(1), db_row->GetField(2) ) ; }
			
			// time
			itemp = db_row->GetField( 0 ) ;
			sscanf( itemp.c_str(), "%lf", &imjd ) ;
			mjd.push_back( imjd );
			mjdDay.push_back( ( int )imjd ) ;
			mjdDayFraction.push_back( ( double )( imjd - ( int )imjd ) ) ;
			mjdSecondsOfDay.push_back( mjdDayFraction.back() * 86400.0 ) ;
			
			// ra decl
			itemp = db_row->GetField( 1 ) ;
			sscanf( itemp.c_str(), "%lf", &ira ) ;
			ra.push_back( ira );
			itemp = db_row->GetField( 2 ) ;
			sscanf( itemp.c_str(), "%lf", &idecl ) ;
			decl.push_back( idecl );
			//if ( count<8 ) { printf("     imjd:%.8f   ra:%f, decl:%f\n", imjd, ira, idecl ) ; }
			
		}
		
		// stop after first telescope, we only need one
		if( a.test( i_tel ) && fNRows > 0 )
		{
			telescope = i_tel ;
			break ;
		}
	}
	//f_db->Close() ;
	
	// convert pointing data from ra/dec to elev/azi
	float lat = 31.675  ; // observatory coordinates
	float lon = 110.952 ;
	VSkyCoordinates* vsky = new VSkyCoordinates() ;
	vsky->supressStdoutText( true ) ;
	vsky->setObservatory( lon, lat );
	for( unsigned int i_row = 0 ; i_row < decl.size() ; i_row++ )
	{
		// target's ra and decl in degrees
		vsky->setTargetJ2000( decl[i_row] * TMath::RadToDeg() , ra[i_row] * TMath::RadToDeg() ) ;
		// day that you're looking for elev and azimuth on
		vsky->precessTarget( mjdDay[i_row], telescope ) ;
		// calculate new param
		vsky->updatePointing( mjdDay[i_row], mjdSecondsOfDay[i_row] ) ;
		printf( "%f %f %f\n", mjd[i_row], vsky->getTargetElevation(), vsky->getTargetAzimuth() ) ;
		/*
		if ( i_row < 8 )
		{
			printf( " MJD: %.8f\n", mjd[0] ) ;
			printf( " (decl,ra) (%f,%f)\n", decl[i_row]*TMath::RadToDeg(), ra[i_row]*TMath::RadToDeg() ) ;
			printf( " (elev,az) (%f,%f)\n", vsky->getTargetElevation(), vsky->getTargetAzimuth() ) ;
		}
		*/
	}
	
	
	
	return 0 ;
}


void getDBMJDTime( string itemp, int& MJD, double& Time, bool bStrip )
{
	if( itemp.size() < 16 )
	{
		MJD = 0;
		Time = 0.;
		return;
	}
	if( bStrip )
	{
		itemp.replace( itemp.find( "-" ), 1, "" );
		itemp.replace( itemp.find( "-" ), 1, "" );
		itemp.replace( itemp.find( " " ), 1, "" );
		itemp.replace( itemp.find( ":" ), 1, "" );
		itemp.replace( itemp.find( ":" ), 1, "" );
	}
	int y, m, d, h, min, s, ms, l;
	double gMJD;
	
	// get y, m, d
	y = atoi( itemp.substr( 0, 4 ).c_str() );
	m = atoi( itemp.substr( 4, 2 ).c_str() );
	d = atoi( itemp.substr( 6, 2 ).c_str() );
	h = atoi( itemp.substr( 8, 2 ).c_str() );
	min = atoi( itemp.substr( 10, 2 ).c_str() );
	s = atoi( itemp.substr( 12, 2 ).c_str() );
	if( !bStrip )
	{
		ms = atoi( itemp.substr( 14, 3 ).c_str() );
	}
	else
	{
		ms = 0;
	}
	
	// calculate MJD
	slaCldj( y, m, d, &gMJD, &l );
	MJD = ( int )gMJD;
	Time = h * 60.*60. + min * 60. + s + ms / 1.e3;
}

