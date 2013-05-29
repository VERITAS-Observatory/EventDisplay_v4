//! VDBRunInfo get run info from DB
// Revision $Id: VDBRunInfo.h,v 1.1.2.5.12.1.18.2.4.1 2011/04/06 11:57:57 gmaier Exp $

#ifndef VDBRUNINFO_H
#define VDBRUNINFO_H

#include "VASlalib.h"

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

using namespace std;

class VDBRunInfo
{
    private:

        bool fDBStatus;

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
        unsigned int fConfigMaskDQM;
        unsigned int fConfigMaskNew;
        unsigned int fTelToAna;
        string fRunType;
        string fObservingMode;
        string fRunStatus;
        string fWeather;
	vector< unsigned int > fLaserRunID;

	TSQLServer*            connectToSQLServer( string iDBserver );
	vector< unsigned int > getLaserRun( string iDBserver, unsigned int iRunNumber, unsigned int iNTel );
        void                   readRunInfoFromDB( string );
        void                   readRunDQM( string );

    public:

        VDBRunInfo( int irun, string iDBserver, unsigned int iNTel );
        ~VDBRunInfo() {}
        int    getRunDate() { return fDBDate; }
        unsigned int getConfigMask() { return fConfigMask; }
        string getDataStartTimeSQL() { return fDataStartTimeSQL; }
        string getDataStoppTimeSQL() { return fDataStoppTimeSQL; }
        int    getDataStartTime() { return fDataStartTime; }
        int    getDataStoppTime() { return fDataStoppTime; }
        int    getDataStartTimeHMS() { return fDataStartTimeHMS; }
        int    getDataStoppTimeHMS() { return fDataStoppTimeHMS; }
        double getDataStartTimeMJD() { return fDataStartTimeMJD; }
        double getDataStoppTimeMJD() { return fDataStoppTimeMJD; }
        int    getDuration() { return fDuration; }
	vector< unsigned int > getLaserRun() { return fLaserRunID; }
        int    getRunNumber() { return fRunNumber; }
        string getTargetName() { return fTargetName; }
        double getTargetDec() { return fTargetDec; }
        double getTargetRA() { return fTargetRA; }
        unsigned int getTelToAna() { return fTelToAna; }
        string getObservingMode() { return fObservingMode; }
        string getRunType() { return fRunType; }
        string getRunStatus() { return fRunStatus; }
        string getWeather() { return fWeather; }
        double getWobbleNorth() { return fWobbleNorth; }
        double getWobbleEast() { return fWobbleEast; }
        bool   isGood() { return fDBStatus; }
        void   print();
};
#endif
