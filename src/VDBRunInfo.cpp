/*! \class VDBRunInfo
    \brief get run info from DB

    Revision $Id: VDBRunInfo.cpp,v 1.1.2.5.12.2.4.1.4.3.10.3.2.1.2.1 2011/04/06 11:57:57 gmaier Exp $

    \author Gernot Maier
*/

#include "VDBRunInfo.h"

VDBRunInfo::VDBRunInfo( int irun, string iDBserver, unsigned int iNTel )
{
    fRunNumber = irun;
    fDBStatus = false;

    fTargetName = "";;
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

    readRunInfoFromDB( iDBserver );
    getLaserRun( iDBserver, fRunNumber, iNTel );
}


void VDBRunInfo::readRunInfoFromDB( string iDBserver )
{
    stringstream iTempS;
    iTempS << iDBserver << "/VERITAS";
    TSQLServer *f_db = TSQLServer::Connect( iTempS.str().c_str(), "readonly", "" );
    if( !f_db )
    {
        cout << "VDBRunInfo: failed to connect to database server" << endl;
        cout << "\t server: " <<  iDBserver << endl;
        fDBStatus = false;
        return;
    }
    char c_query[1000];

    sprintf( c_query, "select * from tblRun_Info where run_id=%d", fRunNumber );

    TSQLResult *db_res = f_db->Query( c_query );
    if( !db_res )
    {
        fDBStatus = false;
        return;
    }

    TSQLRow *db_row = db_res->Next();
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
        if( iTemp.size() > 8 ) fDBDate = atoi( iTemp.substr( 0, 4 ).c_str() ) * 10000 + atoi( iTemp.substr( 5, 2 ).c_str() ) * 100 + atoi( iTemp.substr( 8, 2 ).c_str() );
        else fDBDate = 0;
    }
    else fDBDate = 0;
    if( db_row->GetField( 6 ) )
    {
        string iTemp = db_row->GetField( 6 );
        if( iTemp.size() > 8 )
        {
	    fDataStartTimeSQL = iTemp;
            fDataStartTime = atoi( iTemp.substr( 0, 4 ).c_str() ) * 10000 + atoi( iTemp.substr( 5, 2 ).c_str() ) * 100 + atoi( iTemp.substr( 8, 2 ).c_str() );
            fDataStartTimeHMS = atoi( iTemp.substr( 11, 2 ).c_str() ) * 10000 + atoi( iTemp.substr( 14, 2 ).c_str() ) * 100 + atoi( iTemp.substr( 17, 2 ).c_str() );
        }
        else
	{
	   fDataStartTime = 0;
	   fDataStartTimeHMS = 0;
	   fDataStartTimeSQL = "";
        }
    }
    else
    {
       fDataStartTime = 0;
       fDataStartTimeHMS = 0;
       fDataStartTimeSQL = "";
    }
    if( db_row->GetField( 7 ) )
    {
        string iTemp = db_row->GetField( 7 );
        if( iTemp.size() > 8 )
        {
	    fDataStoppTimeSQL = iTemp;
            fDataStoppTime = atoi( iTemp.substr( 0, 4 ).c_str() ) * 10000 + atoi( iTemp.substr( 5, 2 ).c_str() ) * 100 + atoi( iTemp.substr( 8, 2 ).c_str() );
            fDataStoppTimeHMS = atoi( iTemp.substr( 11, 2 ).c_str() ) * 10000 + atoi( iTemp.substr( 14, 2 ).c_str() ) * 100 + atoi( iTemp.substr( 17, 2 ).c_str() );
        }
        else
	{
	    fDataStoppTime = 0;
	    fDataStoppTimeHMS = 0;
	    fDataStoppTimeSQL = "";
        }
    }
    else
    {
       fDataStoppTime = 0;
       fDataStoppTimeHMS = 0;
       fDataStoppTimeSQL = "";
    }

    if( db_row->GetField( 8 ) )
    {
        string iTemp = db_row->GetField( 8 );
        if( iTemp.size() > 7 ) fDuration = atoi( iTemp.substr( 0, 1 ).c_str() ) * 3600 + atoi( iTemp.substr( 3, 4 ).c_str() ) * 60 + atoi( iTemp.substr( 6, 7 ).c_str() );
        else fDuration = 0;
    }
    else fDuration = 0;

    if( db_row->GetField( 19 ) ) fTargetName = db_row->GetField( 19 );
    else                         fTargetName = "";

    double imjd;
    int j;
    int iy = fDataStartTime/10000;
    int im = (fDataStartTime-(fDataStartTime/10000)*10000)/100;
    int id = (fDataStartTime-(fDataStartTime/100)*100);
    if( fDataStartTime > 0 ) slaCldj( iy, im, id, &imjd, &j );
    else imjd = 0.;
    fDataStartTimeMJD = imjd;
    iy = fDataStoppTime/10000;
    im = (fDataStoppTime-(fDataStoppTime/10000)*10000)/100;
    id = (fDataStoppTime-(fDataStoppTime/100)*100);
    if( fDataStoppTime > 0 ) slaCldj( iy, im, id, &imjd, &j );
    else imjd = 0.;
    fDataStoppTimeMJD  = imjd;

    if( db_row->GetField( 1 ) ) fRunType = db_row->GetField( 1 );
    if( db_row->GetField( 2 ) ) fObservingMode = db_row->GetField( 2 );
    if( db_row->GetField( 3 ) ) fRunStatus = db_row->GetField( 3 );
    if( db_row->GetField( 9 ) ) fWeather = db_row->GetField( 9 );

    float dist = 0.;
    if( db_row->GetField( 17 ) ) dist = atof( db_row->GetField( 17 ) );
    float angl = 0.;
    if( db_row->GetField( 18 ) ) angl = atof( db_row->GetField( 18 ) );

    if( fabs( angl ) < 0.1  )
    {
        fWobbleNorth = dist;
        fWobbleEast = 0.;
    }
    else if( fabs( angl - 90. ) < 0.1 )
    {
        fWobbleNorth = 0.;
        fWobbleEast = dist;
    }
    else if( fabs( angl - 180. ) < 0.1 )
    {
        fWobbleNorth = -1.*dist;
        fWobbleEast = 0.;
    }
    else if( fabs( angl - 270. ) < 0.1 )
    {
        fWobbleNorth = 0.;
        fWobbleEast = -1.*dist;
    }
    if( fRunNumber == 50308 )
    {
       fWobbleNorth = 0.;
       fWobbleEast = -0.5;
    }
// get config mask
    if( db_row->GetField( 10 ) ) fConfigMask = (unsigned int)(atoi( db_row->GetField( 10 ) ) );
    else fConfigMask = 0;
    if( fConfigMask == 1 ) fTelToAna = 1;
    else if( fConfigMask == 2 ) fTelToAna = 2;
    else if( fConfigMask == 3 ) fTelToAna = 12;
    else if( fConfigMask == 4 ) fTelToAna = 3;
    else if( fConfigMask == 5 ) fTelToAna = 13;
    else if( fConfigMask == 6 ) fTelToAna = 23;
    else if( fConfigMask == 7 ) fTelToAna = 123;
    else if( fConfigMask == 8 ) fTelToAna = 4;
    else if( fConfigMask == 9 ) fTelToAna = 14;
    else if( fConfigMask == 10 ) fTelToAna = 24;
    else if( fConfigMask == 11 ) fTelToAna = 124;
    else if( fConfigMask == 12 ) fTelToAna = 34;
    else if( fConfigMask == 13 ) fTelToAna = 134;
    else if( fConfigMask == 14 ) fTelToAna = 234;
    else if( fConfigMask == 15 ) fTelToAna = 1234;

// get source coordinates
    sprintf( c_query, "select * from tblObserving_Sources where source_id like convert( _utf8 \'%s\' using latin1)", fTargetName.c_str() );
    db_res = f_db->Query( c_query );
    if( !db_res )
    {
        fDBStatus = false;
        return;
    }
    db_row = db_res->Next();
    if( !db_row )
    {
        cout <<" VDBRunInfo::readRunInfoFromDB warning :  no source name in tblObserving_Sources like the name of the target for this run ("
            << fTargetName <<")"<< endl;
    }
    else
    {
        fTargetDec = atof( db_row->GetField( 2 ) ) * 180./TMath::Pi();
        fTargetRA = atof( db_row->GetField( 1 ) ) * 180./TMath::Pi();
    }
    f_db->Close();

    fDBStatus = true;
    return;
}


void VDBRunInfo::print()
{
    cout << "Reading run info from database for run " << fRunNumber << ":" << endl;
    cout << "Date: " << fDBDate;
    cout << ", Duration: " << fDuration << " [s]";
    cout << ", " << fRunType << ", " << fObservingMode << ", " << fRunStatus;
    cout << ", Weather: " << fWeather << endl;
    cout << "Target: " << fTargetName;
    cout << " (ra,dec)(J2000)=(" << fTargetRA << ", " << fTargetDec << ")";
    cout << ", Wobble (N,E): " << fWobbleNorth << ", " << fWobbleEast << "), TelToAna: " << fTelToAna << endl;
    cout << "Laser/Flasher runs: ";
    for( unsigned int i = 0; i < getLaserRun().size(); i++ ) cout << "T" << i+1 << ": " << getLaserRun()[i] << "   ";
    cout << endl;
}

vector< unsigned int > VDBRunInfo::getLaserRun( string iDBserver, unsigned int iRunNumber, unsigned int iNTel )
{
    stringstream iTempS;
    iTempS << iDBserver << "/VERITAS";
    TSQLServer *f_db = TSQLServer::Connect( iTempS.str().c_str(), "readonly", "" );
    if( !f_db )
    {
        cout << "VDBRunInfo: failed to connect to database server" << endl;
        cout << "\t server: " <<  iDBserver << endl;
        fDBStatus = false;
        return fLaserRunID;
    }
    char c_query[1000];

    sprintf( c_query, "SELECT info.run_id, grp_cmt.excluded_telescopes FROM tblRun_Info AS info, tblRun_Group AS grp, tblRun_GroupComment AS grp_cmt, (SELECT group_id FROM tblRun_Group WHERE run_id=%d) AS run_grp WHERE grp_cmt.group_id = run_grp.group_id AND grp_cmt.group_type='laser' AND grp_cmt.group_id=grp.group_id AND grp.run_id=info.run_id AND (info.run_type='flasher' OR info.run_type='laser')", iRunNumber );

    TSQLResult *db_res = f_db->Query( c_query );
    if( !db_res )
    {
        fDBStatus = false;
        return fLaserRunID;
    }
    vector< unsigned int > iLaserList;
    vector< unsigned int > iLaserExclude;
    if( db_res->GetRowCount() > 0 )
    {
       while( TSQLRow *db_row = db_res->Next() )
       {
	  if( !db_row )
	  {
	      cout << "VDBRunInfo: failed reading a row from DB for run " << fRunNumber << endl;
	      fDBStatus = false;
	      return fLaserRunID;
	  }
	  iLaserList.push_back( atoi( db_row->GetField( 0 ) ) );
	  iLaserExclude.push_back( atoi( db_row->GetField( 1 ) ) );
       }

    }
    else
    {
       cout << "WARNING: VDBRunInfo::getLaserRun() no laser run found for telescope " << iNTel << " and run " << iRunNumber << endl;
    }
    f_db->Close();

    fLaserRunID.assign( iNTel, 0 );
    for( unsigned int t = 0; t < iNTel; t++ )
    {
// check if this run is excluded from group
       for( unsigned int i = 0; i < iLaserList.size(); i++ )
       {
          bitset< 8 > ibit( iLaserExclude[i] ); 
          if( !ibit.test( t ) ) fLaserRunID[t] = iLaserList[i];
       }
    }

    return fLaserRunID;
}
