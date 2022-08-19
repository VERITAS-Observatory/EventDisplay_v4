//! VDBRunInfo get run info from DB

#ifndef VDBRUNINFO_H
#define VDBRUNINFO_H

#include "VAstronometry.h"

#include <TMath.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TSQLServer.h>
#include <TSystem.h>

#include <bitset>
#include <iostream>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>
#include <bitset>

#include "VDB_Connection.h"
#include "VSkyCoordinatesUtilities.h"
#include "VSQLTextFileReader.h"

using namespace std;

class VDBRunInfo
{
	private:
	
		bool fDBStatus;
		string fDBServer;
		string fDBTextDirectory;
		
		unsigned int fNTel;
		
		int fRunNumber;
		int fDBDate;
		string fDataStartTimeSQL;
		string fDataStoppTimeSQL;
		int fDataStartTime;
		int fDataStoppTime;
		int fDataStartTimeHMS;
		int fDataStoppTimeHMS;
		double fDataStartTimeMJD;
		double fDataStoppTimeMJD;
		int fDuration;                            /// [s]
		string fTargetName;
		double fTargetDec;
		double fTargetRA;
		double fWobbleNorth;
		double fWobbleEast;
		unsigned int fConfigMask;
		unsigned int fTelToAna;
		string fRunType;
		string fObservingMode;
		string fRunStatus;
		string fWeather;
		vector< unsigned int > fLaserRunID;
		
		int get_time_ymd( string iTemp );
		double get_time_MJD( string iTemp );
		int get_time_HMS( string iTemp );
		int get_time_seconds_of_date( string iTemp );
		int get_duration( string iTemp );
		int get_duration_from_sql_string();
		string get_time_sql( string iTemp );
		double get_wobble_north( string dist, string angle );
		double get_wobble_east( string dist, string angle );
		unsigned int get_dqm_configmask( unsigned int config_mask, unsigned int ConfigMaskDQM );
		void set_laser_run( vector< unsigned int > iLaserList, vector< unsigned int > iLaserExclude, vector< unsigned int > iLaserConfigMask );
		void set_telescope_to_analyse();
		
		vector< unsigned int > readLaserRun();
		void                   readRunInfoFromDB();
		unsigned int           readRunDQM( int run_number, unsigned int config_mask );
		void                   readRunDQM();
		
		vector< unsigned int > readLaserFromDBTextFile();
		bool readRunInfoFromDBTextFile();
		void readRunDQMFromDBTextFile();
		unsigned int readRunDQMFromDBTextFile( int run_number, unsigned int config_mask );
		bool readTargetFromDBTextFile();
		
	public:
	
		VDBRunInfo( int irun, string iDBserver, unsigned int iNTel, string iDBTextDirectory = "" );
		~VDBRunInfo() {}
		
		int    getRunDate()
		{
			return fDBDate;
		}
		unsigned int getConfigMask()
		{
			return fConfigMask;
		}
		string getDataStartTimeSQL()
		{
			return fDataStartTimeSQL;
		}
		string getDataStoppTimeSQL()
		{
			return fDataStoppTimeSQL;
		}
		int    getDataStartTime()
		{
			return fDataStartTime;
		}
		int    getDataStoppTime()
		{
			return fDataStoppTime;
		}
		int    getDataStartTimeHMS()
		{
			return fDataStartTimeHMS;
		}
		int    getDataStoppTimeHMS()
		{
			return fDataStoppTimeHMS;
		}
		double getDataStartTimeMJD()
		{
			return fDataStartTimeMJD;
		}
		double getDataStoppTimeMJD()
		{
			return fDataStoppTimeMJD;
		}
		int    getDuration()
		{
			return fDuration;
		}
		vector< unsigned int > getLaserRun()
		{
			return fLaserRunID;
		}
		int    getRunNumber()
		{
			return fRunNumber;
		}
		string getTargetName()
		{
			return fTargetName;
		}
		double getTargetDec()
		{
			return fTargetDec;
		}
		double getTargetRA()
		{
			return fTargetRA;
		}
		unsigned int getTelToAna()
		{
			return fTelToAna;
		}
		string getObservingMode()
		{
			return fObservingMode;
		}
		string getRunType()
		{
			return fRunType;
		}
		string getRunStatus()
		{
			return fRunStatus;
		}
		string getWeather()
		{
			return fWeather;
		}
		double getWobbleNorth()
		{
			return fWobbleNorth;
		}
		double getWobbleEast()
		{
			return fWobbleEast;
		}
		bool   isGood()
		{
			return fDBStatus;
		}
		void   print();
};
#endif
