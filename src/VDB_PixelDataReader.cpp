/* \file  VDB_PixelDataReader
 * \brief reading pixel data from DB
 *
 */

#include "VDB_PixelDataReader.h"

VDB_PixelDataReader::VDB_PixelDataReader( vector< unsigned int > nPixel_per_telescope )
{
    setDebug( true );

//////////////////////////////////////////////////////////////////
// coding of different data vectors:
//  ID = 0: L1 rates
//  ID = 1: HV
//  ID = 3: currents
//  note: this is propagated into the data vector fPixelData
    fPixelDataType.push_back( "L1_Rates" );
    fPixelDataType.push_back( "HV" );
    fPixelDataType.push_back( "Currents" );


// define data vectors and histograms
    char htitle[800];
    char hname[800];
    for( unsigned int t = 0; t < fPixelDataType.size(); t++ )
    {
        vector< vector< VDB_PixelData* > > iD;
        vector< TH1F* > i_his;
        for( unsigned int i = 0; i < nPixel_per_telescope.size(); i++ )
        {
// histograms
           sprintf( hname, "h_%s_tel%d", fPixelDataType[t].c_str(), i+1 );
           sprintf( htitle, "%s (tel %d)", fPixelDataType[t].c_str(), i+1 );
           i_his.push_back( new TH1F( hname, htitle, 200, 0., 1. ) );
           i_his.back()->SetXTitle( fPixelDataType[t].c_str() );
// data
           vector< VDB_PixelData* > iP;
           for( unsigned int j = 0; j < nPixel_per_telescope[i]; j++ )
           {
              iP.push_back( new VDB_PixelData( fPixelDataType[t] ) );
// preliminary: set sampling interval to 60s
              iP.back()->fTimeBinWidth_s = 60.;
           }
           iD.push_back( iP );
        }
        fPixelData.push_back( iD );
        fPixelData_histogram.push_back( i_his );
    }
//////////////////////////////////////////////////////////////////
}

void VDB_PixelDataReader::print( string iDataType, unsigned int iTelID, unsigned int iPixel )
{
   for( unsigned int i = 0; i < fPixelDataType.size(); i++ )
   {
       if( fPixelDataType[i] == iDataType )
       {
           if( iTelID < fPixelData[i].size() && iPixel < fPixelData[i][iTelID].size() )
           {
              fPixelData[i][iTelID][iPixel]->print();
           }
           else
           {
              cout << "VDB_PixelDataReader::print() error: invalid telescope ID/pixel ID (";
              cout << fPixelDataType[i] << ", " << iTelID << ","<< iPixel << ")" << endl;
              return;
           }
           return;
        }
   }
}

void VDB_PixelDataReader::print()
{
   for( unsigned int i = 0; i < fPixelDataType.size(); i++ )
   {
       cout << "VDB_PixelDataReader database: " << fPixelDataType[i] << ":\t";
       cout << "# tel=" << fPixelData[i].size();
       if( fPixelData[i].size() > 0 )
       {
          cout << ", # pixel = " << fPixelData[i][0].size();
          if( fPixelData[i][0].size() > 0 )
          {
             cout << ", # time stamps " << fPixelData[i][0][0]->fMJD.size() << endl;
          }
       }
   }
}

void VDB_PixelDataReader::fillDataRow( unsigned int iDataType, string iTimeStamp, int iTel, int iPixel, float iData )
{
    if( iDataType < fPixelData.size() )
    {
        if( (unsigned int)iTel < fPixelData[iDataType].size() && (unsigned int)iPixel < fPixelData[iDataType][iTel].size() )
        {
            double i_mjd = 0;
            double i_sec_of_day = 0;
            VSkyCoordinatesUtilities::getMJD_from_SQLstring( iTimeStamp, i_mjd, i_sec_of_day );
            fPixelData[iDataType][iTel][iPixel]->fMJD.push_back( i_mjd );
            fPixelData[iDataType][iTel][iPixel]->fsec_of_day.push_back( i_sec_of_day );
            fPixelData[iDataType][iTel][iPixel]->fData.push_back( iData );
            if( fDebug )
            {
                cout << "VDB_PixelDataReader::fillDataRow fill data type " << iDataType << ", time ";
                cout << iTimeStamp << ", T" << iTel+1 << ", pixel " << iPixel << ": " << iData << endl;
            }
        }
    }
}

bool VDB_PixelDataReader::readFromDB( string iDBserver, unsigned int runNumber )
{
   if( fDebug )
   {
       cout << "VDB_PixelDataReader::readFromDB: " << runNumber << endl;
   }

   stringstream iTempS;
   iTempS << iDBserver << "/VERITAS";
   char c_query[1000];
   sprintf( c_query, "select timestamp, telescope_id, pixel_id, rate from tblL1_TriggerInfo, tblRun_Info where timestamp > tblRun_Info.data_start_time AND timestamp <  tblRun_Info.data_end_time AND tblRun_Info.run_id=%d", runNumber );

   VDB_Connection my_connection( iTempS.str(), "readonly", "" ) ; 
   if( !my_connection.Get_Connection_Status() )
   {
	fDBStatus = false;
	return false;
    }
    if(!my_connection.make_query(c_query) )
    {
      	fDBStatus = false;
	return false;
    }  
    TSQLResult *db_res = my_connection.Get_QueryResult();

    if( db_res && db_res->GetRowCount() > 0 )
    {
       while( TSQLRow *db_row = db_res->Next() )
       {
           if( !db_row )
           {
               cout << "VDB_PixelDataReader::readFromDB: failed reading a DB row" << endl;
               fDBStatus = false;
               return false;
           }
           fillDataRow( 0, db_row->GetField( 0 ), atoi( db_row->GetField( 1 )), atoi( db_row->GetField( 2 )), atof( db_row->GetField( 3 )) );
       }
    }

    print();

   return true;
}

bool VDB_PixelDataReader::writeDataTree( unsigned int iTel )
{

    char htitle[200];
    char hname[200];
    float mjd = 0.;
    float time = 0.;
    float value[VDST_MAXCHANNELS];
    unsigned int nPixel = 0;
    for( unsigned int i = 0; i < fPixelData.size(); i++ )
    {
       if( iTel >= fPixelData[i].size() ) 
       {
           cout << "VDB_PixelDataReader::writeDataTree error: invalid data telescope number: ";
           cout << iTel << ", allowed is up to " << fPixelData[i].size() << endl;
           return false;
       }
       sprintf( htitle, "DB pixel data for %s (Tel %d)", fPixelDataType[i].c_str(), iTel+1 );
       sprintf( hname, "dbpixeldata_%s", fPixelDataType[i].c_str() );
       TTree iTree( hname, htitle );
       iTree.Branch( "mjd", &mjd, "mjd/F" );
       iTree.Branch( "sec_of_day", &time, "sec_of_day/F" );
       iTree.Branch( "npixel", &nPixel, "npixel/i" );
       sprintf( hname, "%s", fPixelDataType[i].c_str() );
       sprintf( htitle, "%s[npixel]", fPixelDataType[i].c_str() );
       iTree.Branch( hname, value, htitle );

       if( fPixelData[i][iTel].size() > 0 )
       {
           nPixel = fPixelData[i][iTel].size();
// loop over all MJDs (assuming the same for all pixel)
           for( unsigned int t = 0; t < fPixelData[i][iTel][0]->fMJD.size(); t++ )
           {
               mjd  = fPixelData[i][iTel][0]->fMJD[t];
               time = fPixelData[i][iTel][0]->fsec_of_day[t];
               for( unsigned int p = 0; p < fPixelData[i][iTel].size(); p++ )
               {
                  if( t < fPixelData[i][iTel][p]->fMJD.size() )
                  {
                      value[p] = fPixelData[i][iTel][p]->fData[t];
                  }
                  else
                  {
                      value[p] = -99.;
                  }
               }
               iTree.Fill();
           }
       }

       iTree.Write();
   }
   return true;
}

/*
 * return distribution of data values
 *
 */
TH1F* VDB_PixelDataReader::getDataHistogram( unsigned int iDataType, unsigned int iTel, int iMJD, float iTime )
{
// fills fDummyReturnVector with data
    getDataVector( iDataType, iTel, iMJD, iTime );
    if( fDummyReturnVector.size() == 0 ) return 0;

// get min/maximum
    float v_min = *std::min_element( fDummyReturnVector.begin(), fDummyReturnVector.end() );
    float v_max = *std::max_element( fDummyReturnVector.begin(), fDummyReturnVector.end() );
// make sure that min/max give a valid histogram
    if( v_min < 0. ) v_min = 0.;
    if( TMath::Abs( v_max - v_min ) < 1.e-2 ) v_max = v_min +1.;
    if( fPixelData_histogram[iDataType][iTel] )
    {
        fPixelData_histogram[iDataType][iTel]->Reset();
        fPixelData_histogram[iDataType][iTel]->SetBins( fPixelData_histogram[iDataType][iTel]->GetNbinsX(), 0., v_max );
        for( unsigned int i = 0; i < fDummyReturnVector.size(); i++ )
        {
           fPixelData_histogram[iDataType][iTel]->Fill( fDummyReturnVector[i] );
        }
        return fPixelData_histogram[iDataType][iTel];
    }

    return 0;
}

/*
 * get a single value for a given telescope, channel, and time
 *
 * this function is very inefficient, don't call it in a loop!
 */
float VDB_PixelDataReader::getValue( unsigned int iDataType, unsigned int iTel, unsigned int iChannel, int iMJD, float iTime )
{
// fills fDummyReturnVector with data
    getDataVector( iDataType, iTel, iMJD, iTime );
    if( fDummyReturnVector.size() == 0 ) return 0;

    if( iChannel < fDummyReturnVector.size() ) return fDummyReturnVector[iChannel];

    return 0.;
}


/*
 * return a vector of size npixel
 */
vector< float > VDB_PixelDataReader::getDataVector( unsigned int iDataType, unsigned int iTel, int iMJD, float iTime )
{
    fDummyReturnVector.clear();

// is this a good data type ID?
    if( iDataType < fPixelData.size() )
    {
// check telescope number
        if( iTel < fPixelData[iDataType].size() )
        {
// reset return vector
            fDummyReturnVector.assign( fPixelData[iDataType][iTel].size(), 0. );
// loop over all pixel
            for( unsigned int i = 0; i < fPixelData[iDataType][iTel].size(); i++ )
            {
// loop over all pixel times to get correct time
                if( fPixelData[iDataType][iTel][i]->fMJD.size() > 1 )
                {
                    for( unsigned int t = 0; t < fPixelData[iDataType][iTel][i]->fMJD.size(); t++ )
                    {
// this should never happen
                       if( TMath::Abs( fPixelData[iDataType][iTel][i]->fMJD[t] - iMJD ) > 1.e-1 ) continue;
// first measurement is sometimes a bit late: look in a wide search window
                       double iTimeBinWidth = fPixelData[iDataType][iTel][i]->fTimeBinWidth_s;
                       if( t == 0 ) iTimeBinWidth *= 4.;
// check time and fill return vector
                       if( iTime >= fPixelData[iDataType][iTel][i]->fsec_of_day[t] - iTimeBinWidth
                         && iTime < fPixelData[iDataType][iTel][i]->fsec_of_day[t] )
                       {
                           fDummyReturnVector[i] = fPixelData[iDataType][iTel][i]->fData[t];
// first and/or last time bin is sometimes not filled (run start/end and L1 rate interval star mismatch)
// use in this case the L1 rates from second bin
                           if( fDummyReturnVector[i] < 1.e-2 && fPixelData[iDataType][iTel][i]->fData.size() > 1 )
                           {
                               if( t == 0 )
                               {
                                  fDummyReturnVector[i] = fPixelData[iDataType][iTel][i]->fData[t+1];
                               }
                               else if( t == fPixelData[iDataType][iTel][i]->fMJD.size()-1 )
                               {
                                  fDummyReturnVector[i] = fPixelData[iDataType][iTel][i]->fData[t-1];
                               }
                           }
                           break;
                       }
                    }
                }
            }
        }
    }
    return fDummyReturnVector;
}

/*
 * return list of channels with data outside of the given range
 *
 */
vector< unsigned int > VDB_PixelDataReader::getDeadChannelList( unsigned int iDataType, unsigned int iTel, int iMJD, float iTime, float i_min, float i_max )
{
    vector< unsigned int > i_channelList;
// fills fDummyReturnVector with data
    getDataVector( iDataType, iTel, iMJD, iTime );
    if( fDummyReturnVector.size() == 0 ) return i_channelList;

    for( unsigned int i = 0; i < fDummyReturnVector.size(); i++ )
    {
       if( fDummyReturnVector[i] < i_min )      i_channelList.push_back( i );
       else if( fDummyReturnVector[i] > i_max ) i_channelList.push_back( i );
    }

    return i_channelList;
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

VDB_PixelData::VDB_PixelData( string iDataType )
{
   fDataType = iDataType;
   fTimeBinWidth_s = 60.;
}

void VDB_PixelData::print()
{
   for( unsigned int i = 0; i < fMJD.size(); i++ )
   {
       cout << "\t Pixel: " << i+1;
       cout << ", MJD " << fMJD[i];
       cout << ", sec of day " << fsec_of_day[i];
       cout << ", " << fDataType << ": " << fData[i];
       cout << endl;
   }
}
