/*! file VTS.getRunListFromDB read exposure from DB

    (VERITAS only)

    Revision $Id$

    \author Gareth Hughes
*/

#include "VGlobalRunParameter.h"
#include "VExposure.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

void parseOptions(int argc, char *argv[]);

string laserlist="";
string runlist="";
string startdate="2000-01-01";
string enddate="2050-01-01";
string sourcename="Crab";
double elevation=0.;
double duration=0.;
int laserruns=0;
int getRuns=0;
int timemask=0;
int verbose=0;

int main( int argc, char *argv[] )
{

    parseOptions(argc, argv);
    VExposure a;

    if( runlist != "" )
    {

      a.readRunListFromFile(runlist);
      a.readFromDBList();
      a.setSelectLaser( laserruns );
      a.setPrintVerbose( verbose );
      a.setPrintTimeMask( timemask );

      a.readRunCommentsFromDB();
      if( laserruns == 1 ) a.getLaserList();
      a.printListOfRuns();
      if( getRuns == 1 ) a.downloadRunList();

    } else if( laserlist != "" )
    {

      a.readLaserRunListFromFile(laserlist);
      a.setSelectLaser( laserruns );
      if( getRuns == 1 ) a.downloadRunList();

    } else
    {

      a.setMakeRunList( true );
      a.setTimeRange( startdate, enddate );
      a.setSourceName( sourcename );
      a.setTelMinElevation( elevation );
      a.setSelectLaser( laserruns );
      a.setMinDuration( duration );
      a.setPrintVerbose( verbose );
      a.setPrintTimeMask( timemask );

      a.readFromDB();
      a.readRunCommentsFromDB();
      if( laserruns == 1 ) a.getLaserList();
      a.printListOfRuns();
      if( getRuns == 1 ) a.downloadRunList();

    }

}

void parseOptions(int argc, char *argv[])
{
    while(1)
    {
        static struct option long_options[]=
        {
            {"help", no_argument, NULL, 'h'},
            {"runlist", required_argument, 0, 'l'},
            {"laserlist", required_argument, 0, 'm'},
            {"start", required_argument, 0, 'b'},
            {"stop", required_argument, 0, 'e'},
            {"source", required_argument, 0, 's'},
            {"elevation", required_argument, 0, 'z'},
            {"duration", required_argument, 0, 'd'},
            {"laser", no_argument, NULL, 'x'},
            {"getRuns", no_argument, NULL, 'g'},
            {"timemask", no_argument, NULL, 't'},
            {"verbose", no_argument, NULL, 'v'},
	    { 0, 0, 0, 0 }
        };

        int option_index=0;
        int c=getopt_long(argc, argv, "ho:lm:b:e:s:z:d:xgtv", long_options, &option_index);
        if( argc == 1 ) c = 'h';
        if (c==-1) break;

        switch(c)
        {
            case 0:
                if(long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;
            case 'h':
		char *ENV;
		ENV = getenv("OBS_EVNDISP_ANA_DIR");
		char readme[500];
		sprintf(readme,"cat %s/ParameterFiles/EVNDISP.getSourceInfo.runparameter",ENV);
                system( readme );
                exit( 0 );
                break;
            case 'l':
                runlist=optarg;
                break;
            case 'm':
                laserlist=optarg;
                break;
            case 'b':
                startdate=optarg;
                break;
            case 'e':
                enddate=optarg;
                break;
            case 's':
                sourcename=optarg;
                break;
            case 'z':
                elevation=atof(optarg);
                break;
            case 'd':
                duration=atof(optarg);
                break;
            case 'x':
                laserruns=1;
                break;
            case 'g':
                getRuns=1;
                break;
            case 't':
                timemask=1;
                break;
            case 'v':
                verbose=1;
                break;
            case '?':
                break;
            default:
                abort();
        }
    }
    return;
}

