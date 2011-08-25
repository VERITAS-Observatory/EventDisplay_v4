/*! \brief VAtmosphereSoundings read and analyse sounding data

   Revision $Id: VAtmosphereSoundings.cpp,v 1.1.2.10 2010/10/01 09:17:01 gmaier Exp $

//////////////////////////////////////////////////////////////////////////////////////

   reading ascii files with sounding data for Tucson

   Data from: http://weather.uwyo.edu/upperair/sounding.html

    VAtmosphereSoundings a;
    a.readSoundingsFromTextFile("allBalloon.txt");
    a.writeRootFile("ballonDataTucson_199501_201007.root");


//////////////////////////////////
   FIX ATMOSPHERIC THICKNESS

   \author
   Gernot Maier
*/

#include "VAtmosphereSoundings.h"

VAtmosphereSoundings::VAtmosphereSoundings()
{
    bDebug = false;
    plotAttributes_ColorChange();
    plotAttributes_PlotLegend();
    setPlottingRangeHeight();
    setPlottingRelativePlots();

    setGeographicPosition();

    fDataFile = 0;
    fDataTree = 0;

//////////////////////////////////////////////////////////////////////////////////
// hardwired search strings for reading of sounding text files (preliminary)
    fTXTSearch_DataString = "72274 TUS Tucson Observations at";
//////////////////////////////////////////////////////////////////////////////////
}

VAtmosphereSoundings::VAtmosphereSoundings( string iRootFile )
{
    bDebug = false;
    plotAttributes_ColorChange();    
    plotAttributes_PlotLegend();
    setPlottingRelativePlots();

    setPlottingPeriod();

    fDataFile = new TFile( iRootFile.c_str() );
    if( fDataFile->IsZombie() )
    {
        cout << "VAtmosphereSoundings::VAtmosphereSoundings( string ) error: data file not found: " << iRootFile << endl;
	return;
    }
    fDataTree = (TTree*)fDataFile->Get( "tSoundings" );
    if( !fDataTree )
    {
       cout << "VAtmosphereSoundings::VAtmosphereSoundings( string ) error: data tree not found in " << iRootFile << endl;
       return;
    }

    readRootFile();
}

/*

*/
bool VAtmosphereSoundings::readSoundingsFromTextFile( string iFileList )
{
    ifstream is;
    is.open( iFileList.c_str() );
    if( !is )
    {
       cout << "VAtmosphereSoundings::readSoundingsFromTextFile: error: file list not found: " << iFileList << endl;
       return false;
    }
    string is_line;
    while(  getline( is, is_line ) )
    {
       if( is_line.size() <=0 ) continue;

       fListTextFile.push_back( is_line );
    }
    is.close();

    cout << "number of files in file list: " << fListTextFile.size() << endl;

// open all files and read sounding data
    string iTemp;
    for( unsigned int i = 0; i < fListTextFile.size(); i++ )
    {
       cout << "reading " << fListTextFile[i] << endl;

       ifstream is;
       is.open( fListTextFile[i].c_str() );
       if( !is )
       {
          cout << "VAtmosphereSoundings::readSoundingsFromTextFile: error: sounding file not found: " << fListTextFile[i] << endl;
	  continue;
       }
       int z = 0;
       while(  getline( is, is_line ) )
       {
          if( is_line.size() <=0 ) continue;

///////////////////////// search for a valid date
	  if( is_line.find( fTXTSearch_DataString ) < is_line.size() )
	  {
	     istringstream is_stream( is_line.substr( is_line.find( fTXTSearch_DataString )+fTXTSearch_DataString.size(), fTXTSearch_DataString.size() ) );

             is_stream >> iTemp;
	     int iHour = atoi( iTemp.substr( 0, 2 ).c_str() );
             is_stream >> iTemp;
	     int iDay  = atoi( iTemp.c_str() );
	     is_stream >> iTemp;
	     int iMonth = getMonth( iTemp );
	     is_stream >> iTemp;
	     int iYear = atoi( iTemp.c_str() );

// get MJD
             double iMJD = 0;
	     int j = 0;
	     slaCldj ( iYear, iMonth, iDay, &iMJD, &j );
	     if( j != 0 )
	     {
	        cout << "VAtmosphereSoundings::readSoundingsFromTextFile: error: invalid data: " << is_line << endl;
		continue;
             }
// preli!
	     if( iHour == 0 ) iMJD += 0.5;

// a valid new data starts a new entry
             fData.push_back( new VAtmosphereSoundingData() );
	     fData.back()->MJD   = iMJD;
	     fData.back()->Year  = iYear;
	     fData.back()->Month = iMonth;
	     fData.back()->Day   = iDay;
	     fData.back()->Hour  = (double)iHour;
	     z = 0; 
///////////////////////// read a full entry
// expect first entry 7 lines after date 
	     while(  getline( is, is_line ) )
	     {
		if( is_line.size() <=0 ) continue;

		if( is_line.find( "Station information" ) < is_line.size() ) break;

		if( z >= 5 )
		{
		    double iT = 0.;
		    istringstream is_stream( is_line );
		    iT = -9999.;
		    if( !is_stream.eof() ) is_stream >> iT;
		    if( is_line.substr( 4, 1 ) != " " ) fData.back()->fPressure_Pa.push_back( iT*100. );
		    else                                fData.back()->fPressure_Pa.push_back( -9999. );
		    iT = -9999.;
		    if( !is_stream.eof() ) is_stream >> iT;
		    if( is_line.substr( 13, 1 ) != " " ) fData.back()->fHeight_m.push_back( iT );
		    else                                 fData.back()->fHeight_m.push_back( -9999. ); 
		    iT = -9999.;
		    if( !is_stream.eof() )
		    {
		       is_stream >> iT;
		       iT += 273.15;
		    }
		    if( is_line.substr( 18, 1 ) != " " ) fData.back()->fTemperature_K.push_back( iT );
		    else                                 fData.back()->fTemperature_K.push_back( -9999. );

		    iT = -9999.;
		    if( !is_stream.eof() )
		    {
		      is_stream >> iT;
		      iT += 273.15;
		    }
		    if( is_line.substr( 25, 1 ) != " " ) fData.back()->fDewPoint_K.push_back( iT );
		    else                                 fData.back()->fDewPoint_K.push_back( -9999. );

		    iT = -9999.;
		    if( !is_stream.eof() )
		    {
			is_stream >> iT;
			iT /= 1.e2;                // % to fraction
		    }
		    if( is_line.substr( 34, 1 ) != " " ) fData.back()->fRelativeHumidity.push_back( iT );
		    else                                 fData.back()->fRelativeHumidity.push_back( -9999. ); 
// mixing ratio
		    iT = -9999.;
		    if( !is_stream.eof() )
		    {
		      is_stream >> iT;
		    }
		    if( is_line.substr( 41, 1 ) != " " ) fData.back()->fMixingRatio_gkg.push_back( iT );
		    else                                 fData.back()->fMixingRatio_gkg.push_back( -9999. );
// wind direction		    
		    iT = -9999.;
		    if( !is_stream.eof() )
		    {
		      is_stream >> iT;
		    }
		    if( is_line.substr( 48, 1 ) != " " ) fData.back()->fWindDirection_deg.push_back( iT );
		    else                                 fData.back()->fWindDirection_deg.push_back( -9999. );
// wind speed
		    iT = -9999.;
		    if( !is_stream.eof() )
		    {
		      is_stream >> iT;
                      iT *= 0.514444;  // [knots] to [m/s]
		    }
		    if( is_line.substr( 55, 1 ) != " " ) fData.back()->fWindSpeed_ms.push_back( iT );
		    else                                 fData.back()->fWindSpeed_ms.push_back( -9999. );
		}
	     z++;
	     }
	  }
       }
    }
// fill remaining fields
    fillWaterVaporDensity();
    fillAtmosphericDensity();
    fillAtmosphericThickness();
    fillIndexofRefraction();
    fillO2();
    fillO3();

    return true; 
}

void VAtmosphereSoundings::fillO2()
{
   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      if( fData[i] )
      {
	 for( unsigned int j = 0; j < fData[i]->fPressure_Pa.size(); j++ )
	 {
	    fData[i]->fO2_cmkm.push_back( -9999. );
	 }
       }
   }
}

void VAtmosphereSoundings::fillO3()
{
   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      if( fData[i] )
      {
	 for( unsigned int j = 0; j < fData[i]->fPressure_Pa.size(); j++ )
	 {
	    fData[i]->fO3_cmkm.push_back( -9999. );
	 }
       }
   }
}



/*void VAtmosphereSoundings::fillAtmosphericThickness()
{
   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      if( fData[i] )
      {
         for( unsigned int j = 0; j < fData[i]->fPressure_Pa.size(); j++ )
	 {
	    fData[i]->fThickness_gcm2.push_back( -9999. );
         }
       }
   }
} */

void VAtmosphereSoundings::fillAtmosphericPressure()
{
   for( unsigned int i = 0; i < fData.size(); i++ ) fillAtmosphericPressure( fData[i] );
}

/*
   calculate atmospheric pressure from atmospheric thickness
*/
void VAtmosphereSoundings::fillAtmosphericPressure( VAtmosphereSoundingData* iData )
{
    if( !iData ) return;

// calculate graviational constant for the given observatory latitude and height
    double phi = fObservatoryLatitude * atan(1.)/45.;
    double g = 9.780327 * ( 1.+0.0053024*(sin( phi ) * sin( phi ) ) - 0.0000058 * sin( 2.*phi ) * sin( 2.*phi ) ) - 3.086e-6 * fObservatoryHeight_km * 1.e3; 

    double p = -9999;
    for( unsigned int j = 0; j < iData->fThickness_gcm2.size(); j++ )
    {
       if( iData->fThickness_gcm2[j] > 0. )
       {
	     p  = g * iData->fThickness_gcm2[j];
	     p *= 10.;                                     // kg/m2 -> m/cm2
       }
       else  p  = -9999.;
       if( j >= iData->fPressure_Pa.size() ) iData->fPressure_Pa.push_back( p );
       else                                  iData->fPressure_Pa[j] = p;
    }
}

void VAtmosphereSoundings::fillAtmosphericThickness()
{
   for( unsigned int i = 0; i < fData.size(); i++ ) fillAtmosphericThickness( fData[i] );
}

void VAtmosphereSoundings::fillAtmosphericThickness( VAtmosphereSoundingData* iData )
{
    if( !iData ) return;

// calculate graviational constant
    double phi = fObservatoryLatitude * atan(1.)/45.;
    double g = 9.780327 * ( 1.+0.0053024*(sin( phi ) * sin( phi ) ) - 0.0000058 * sin( 2.*phi ) * sin( 2.*phi ) ) - 3.086e-6 * fObservatoryHeight_km * 1.e3; 

    double t = -9999;
    for( unsigned int j = 0; j < iData->fPressure_Pa.size(); j++ )
    {
       if( iData->fPressure_Pa[j] > 0. )
       {
             t  = iData->fPressure_Pa[j] / g;  
	     t /= 10.;                                     // kg/m2 -> m/cm2
       }
       else  t  = -9999.;
       if( j >= iData->fThickness_gcm2.size() ) iData->fThickness_gcm2.push_back( t );
       else                                     iData->fThickness_gcm2[j] = t;
    }
}

void VAtmosphereSoundings::fillIndexofRefraction()
{
   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      if( fData[i] )
      {
         for( unsigned int j = 0; j < fData[i]->fPressure_Pa.size(); j++ )
	 {
	    fData[i]->fIndexofRefraction.push_back( -9999. );
         }
       }
   }
}

bool VAtmosphereSoundings::add_CORSIKA_Atmosphere( string iFile, string iName, int iColor, int iLineStyle )
{
    ifstream is;
    is.open( iFile.c_str() );
    if( !is )
    {
       cout << "VAtmosphereSoundings::add_CORSIKA_Atmosphere: error opening CORSIKA atmospheric file " << iFile << endl;
       return false;
    }

    fDataCORSIKAMODTRAN.push_back( new VAtmosphereSoundingData() );
    fDataCORSIKAMODTRAN.back()->Name = iName;
    fDataCORSIKAMODTRAN.back()->PlotColor = iColor;
    fDataCORSIKAMODTRAN.back()->PlotLineStyle = iLineStyle;

    string is_line;
    string iTemp;
    double d;

    while(  getline( is, is_line ) )
    {
       if( is_line.size() <= 0 ) continue;
       if( is_line.substr( 0, 1 ) == "#" ) continue;

       istringstream is_stream( is_line );

       fDataCORSIKAMODTRAN.back()->fPressure_Pa.push_back( -9999. );

       is_stream >> iTemp;
       d = atof( iTemp.c_str() );
       fDataCORSIKAMODTRAN.back()->fHeight_m.push_back( d*1.e3 );
       
       is_stream >> iTemp;
       d = atof( iTemp.c_str() );
       fDataCORSIKAMODTRAN.back()->fDensity_gcm3.push_back( d );

       is_stream >> iTemp;
       d = atof( iTemp.c_str() );
       fDataCORSIKAMODTRAN.back()->fThickness_gcm2.push_back( d );

       fDataCORSIKAMODTRAN.back()->fTemperature_K.push_back( -9999. );

       fDataCORSIKAMODTRAN.back()->fDewPoint_K.push_back( -9999. );

       fDataCORSIKAMODTRAN.back()->fRelativeHumidity.push_back( -9999. );
       fDataCORSIKAMODTRAN.back()->fVaporMassDensity_gm3.push_back( -9999. );

       is_stream >> iTemp;
       d = atof( iTemp.c_str() );
       fDataCORSIKAMODTRAN.back()->fIndexofRefraction.push_back( d+1. );

       fDataCORSIKAMODTRAN.back()->fMixingRatio_gkg.push_back( -9999. );
       fDataCORSIKAMODTRAN.back()->fWindDirection_deg.push_back( -9999. );
       fDataCORSIKAMODTRAN.back()->fWindSpeed_ms.push_back( -9999. );
       fDataCORSIKAMODTRAN.back()->fO2_cmkm.push_back( -9999. );
       fDataCORSIKAMODTRAN.back()->fO3_cmkm.push_back( -9999. );

    }
    is.close();

    fillAtmosphericPressure( fDataCORSIKAMODTRAN.back() );

    return true;
}

/*!
    read MODTRAN tp6 files


*/
bool VAtmosphereSoundings::add_MODTRAN_Atmosphere( string iFile, string iName, int iColor, int iLineStyle )
{
    ifstream is;
    is.open( iFile.c_str() );
    if( !is )
    {
       cout << "VAtmosphereSoundings::add_MODTRAN_Atmosphere: error opening MODTRAN tp6 file" << iFile << endl;
       return false;
    }
// for now: only tp6 files can be read
    if( iFile.find( "tp6" ) >= iFile.size() ) return false;

    fDataCORSIKAMODTRAN.push_back( new VAtmosphereSoundingData() );
    fDataCORSIKAMODTRAN.back()->Name = iName;
    fDataCORSIKAMODTRAN.back()->PlotColor = iColor;
    fDataCORSIKAMODTRAN.back()->PlotLineStyle = iLineStyle;

    cout << "reading MODTRAN atmosphere from " << iFile << endl;

    string is_line;
    string iTemp;

    int z = 0;

    while(  getline( is, is_line ) )
    {
       z++;
       if( is_line.size() <= 0 ) continue;
       if( is_line.substr( 0, 1 ) == "#" ) continue;
// pressure etc values from line 7 to 
       if( z > 5 && z < 42 )
       {
	  istringstream is_stream( is_line );

// counter
	  is_stream >> iTemp;
// height (km)
	  is_stream >> iTemp;
	  fDataCORSIKAMODTRAN.back()->fHeight_m.push_back( atof( iTemp.c_str() ) * 1.e3 );
// pressure (MB)
	  is_stream >> iTemp;
	  fDataCORSIKAMODTRAN.back()->fPressure_Pa.push_back( atof( iTemp.c_str() ) * 1.e2 );
// temperature (K)
	  is_stream >> iTemp;
	  fDataCORSIKAMODTRAN.back()->fTemperature_K.push_back( atof( iTemp.c_str() ) );
// density
	  fDataCORSIKAMODTRAN.back()->fDensity_gcm3.push_back( -9999. );
	  fDataCORSIKAMODTRAN.back()->fThickness_gcm2.push_back( -9999. );
	  fDataCORSIKAMODTRAN.back()->fDewPoint_K.push_back( -9999. );
	  fDataCORSIKAMODTRAN.back()->fMixingRatio_gkg.push_back( -9999. );
	  fDataCORSIKAMODTRAN.back()->fWindDirection_deg.push_back( -9999. );
	  fDataCORSIKAMODTRAN.back()->fWindSpeed_ms.push_back( -9999. );
// N2
	  is_stream >> iTemp;
// CNTMSLF
	  is_stream >> iTemp;
// CNTMFRN
	  is_stream >> iTemp;
// MOL SCAT
	  is_stream >> iTemp;
// N-1   
	  is_stream >> iTemp;
	  fDataCORSIKAMODTRAN.back()->fIndexofRefraction.push_back( atof( iTemp.c_str() ) + 1. );
// O3 (UV) 
	  is_stream >> iTemp;
	  fDataCORSIKAMODTRAN.back()->fO3_cmkm.push_back( atof( iTemp.c_str() ) );
// O2 (UV)
	  is_stream >> iTemp;
	  fDataCORSIKAMODTRAN.back()->fO2_cmkm.push_back( atof( iTemp.c_str() ) );
       }
       else if( z > 48 )
       {
	  istringstream is_stream( is_line );

	  for( unsigned int s = 0; s < 9; s++ ) is_stream >> iTemp;
// relative humidity
	  is_stream >> iTemp;
	  fDataCORSIKAMODTRAN.back()->fRelativeHumidity.push_back( atof( iTemp.c_str() ) );
       }
    }
    is.close();

    for( unsigned int i = 0; i < fDataCORSIKAMODTRAN.size(); i++ )
    {
       if( fDataCORSIKAMODTRAN[i]->fIndexofRefraction.size() != fDataCORSIKAMODTRAN[i]->fRelativeHumidity.size() )
       {   
          for( unsigned int j = 0; j < fDataCORSIKAMODTRAN[i]->fIndexofRefraction.size(); j++ )
	  {
	     fDataCORSIKAMODTRAN[i]->fRelativeHumidity.push_back( -9999. );
          }
       }
    }
    fillWaterVaporDensity( fDataCORSIKAMODTRAN.back() );
    fillAtmosphericDensity( fDataCORSIKAMODTRAN.back() );
    fillAtmosphericThickness( fDataCORSIKAMODTRAN.back() );

    return true;
}

void VAtmosphereSoundings::fillWaterVaporDensity()
{
   for( unsigned int i = 0; i < fData.size(); i++ ) fillWaterVaporDensity( fData[i] );
}

void VAtmosphereSoundings::fillWaterVaporDensity( VAtmosphereSoundingData* iData )
{
   if( !iData ) return;

   for( unsigned int i = 0; i < iData->fTemperature_K.size(); i++ )
   {
      double w = -9999.;
      if( i <= iData->fRelativeHumidity.size() )
      {
	 w = getWaterVaporDensity( iData->fTemperature_K[i], iData->fRelativeHumidity[i] );
      }

      if( i < iData->fVaporMassDensity_gm3.size() ) iData->fVaporMassDensity_gm3[i] = w;
      else                                          iData->fVaporMassDensity_gm3.push_back( w );
    }
}

void VAtmosphereSoundings::fillAtmosphericDensity()
{
   for( unsigned int i = 0; i < fData.size(); i++ ) fillAtmosphericDensity( fData[i] );
}


/*!
   calculate atmosheric density 

   after meteorology textbooks (see e.g. http://wahiduddin.net/calc/density_altitude.htm )

*/
void VAtmosphereSoundings::fillAtmosphericDensity( VAtmosphereSoundingData* iData )
{
   double R_d = 287.05;        //  Rd = gas constant for dry air, J/(kg*degK) 

   if( iData )
   {
      for( unsigned int j = 0; j < iData->fPressure_Pa.size(); j++ )
      {
	 double d = 0.; 
	 double pressure =    iData->fPressure_Pa[j];
	 double temperature = -9999;
	 if( j < iData->fTemperature_K.size() ) temperature = iData->fTemperature_K[j];
	 double relativeHumidity = -9999.;
	 if( j < iData->fRelativeHumidity.size() ) relativeHumidity = iData->fRelativeHumidity[j];
	 double dewpoint    = -9999;
// calculate dew point: if not available
	 if( j < iData->fDewPoint_K.size() && iData->fDewPoint_K[j] > -300. ) dewpoint = iData->fDewPoint_K[j];
	 else
	 {
	    dewpoint = getDewPoint( temperature, relativeHumidity, 1 );
	    if( j < iData->fDewPoint_K.size() ) iData->fDewPoint_K[j] = dewpoint;
	    else                                iData->fDewPoint_K.push_back( dewpoint );
	 }

	 if( pressure < -9998. || temperature < -0. || dewpoint < -9998. ) d = -9999.;
	 else
	 {
// dry air
            if( TMath::Abs( relativeHumidity ) < 1.e-2 )
	    {
	       d = ( pressure / R_d / temperature );
            }
// wet air
	    else
	    {
	       double pressure_vapore = getAmosphericVaporPressure( dewpoint );

	       if( pressure > 0. )
	       {
		  d  = ( pressure / R_d / temperature ) * ( 1. - 0.378 * pressure_vapore / pressure );                // kg/m3
	       }
	       else
	       {
		  d = 0.;
	       }
            }
	    d *= 1.e3 / 1.e6;  // kg/m3 -> g/cm3
	 }
	 if( j < iData->fDensity_gcm3.size() ) iData->fDensity_gcm3[j] = d;
	 else                                  iData->fDensity_gcm3.push_back( d );
      }
   }
}

/*!

   calculate water vapor mass density [g/m3] for a given humidity

   T in [K]

   from MODTRAN watvap.f
*/
double VAtmosphereSoundings::getWaterVaporDensity( double T, double RH )
{
   if( T < 0. || RH < 0. ) return -9999.;

   double AVOGAD =  TMath::Na();   // Avogadro number
   double AMWT = 18.015;           // molecular weight of water
   double B = AVOGAD / AMWT;

   double A = 273.15 / T;

   double WWMOL = RH;

   return 1.e6 * getWaterVaporMassDensity(A) * (WWMOL/100.0) / B;
}

/*!
   calculate saturated vapor mass density [g/m3] 

   accuracy: <1% for -50 to +50 [C]
   
   from MODTRAN watvap.f
*/
double VAtmosphereSoundings::getWaterVaporMassDensity( double ATEMP )
{
   double C1 = 18.9766;
   double C2 = -14.9595;
   double C3 = -2.43882;

   double AVOGAD =  TMath::Na();   // Avogadro number
   double AMWT = 18.015;           // molecular weight of water
   double B = AVOGAD / AMWT;

   return ATEMP * B * TMath::Exp( C1 + C2 * ATEMP + C3 * ATEMP*ATEMP )*1.0e-6;
}


/*!

   calculate dew point from temperature and relative humidity 

   T in [K]

   Attention: this might be in inappropriate approximation (valid only for T > 0. [C] and humidity > 1%)

*/
double VAtmosphereSoundings::getDewPoint( double T, double R, int iMethod )
{
   if( T < 1.e-2 || R < 0. ) return -9999.;

// not sure if this makes sense
   if( TMath::Abs( R ) < 1.e-3 ) return 0.;

   double d = -9999.;
   
   if( iMethod == 0 )
   {
      d = TMath::Power( R / 100., 1./8. ) * (112.+0.9* (T-273.15)) + 0.1 * (T-273.15) - 112.;
      d += 273.15;
   }
// Magnus-Tetens formula
   else if( iMethod == 1 )
   {
      double alpha = 17.27 * (T-273.15) / (237.7 + (T-273.15)) + log( R/1.e2 );
      d = 237.7*alpha/(17.27-alpha);
      d += 273.15;
   }
   else if( iMethod == 2 )
// NOAA
//  (http://www.srh.noaa.gov/images/epz/wxcalc/rhTdFromWetBulb.pdf)
   {
     double es = 6.112 * TMath::Exp( 17.67*(T-273.15)/(T-273.15+243.5) );
     double e  = R/1.e2 * es;

     d = 243.5*log( e/6.112 ) / (17.67-log( e/6.112 ) );

     d += 273.15;
   }

   return d;
}

/*!
    calculate vapor pressure from temperature 

    or

    calculate actual vapor pressure from dewpoint 

    after Wobus 
*/
double VAtmosphereSoundings::getAmosphericVaporPressure( double T )
{
   double c0 = 0.99999683;
   double c1 = -0.90826951e-2;
   double c2 = 0.78736169e-4;
   double c3 = -0.61117958e-6;
   double c4 = 0.43884187e-8;
   double c5 = -0.29883885e-10;
   double c6 = 0.21874425e-12;
   double c7 = -0.17892321e-14;
   double c8 = 0.11112018e-16;
   double c9 = -0.30994571e-19;
   double eso = 6.1078;

   T -= 273.15; // K -> deg

   double p = (c0+T*(c1+T*(c2+T*(c3+T*(c4+T*(c5+T*(c6+T*(c7+T*(c8+T*(c9))))))))));

   double Es = eso / TMath::Power( p, 8. )*1.e2;               // vapour pressure [mb->pascal]

// alternative calculation (less accurate)
//   double Es = 6.1078 * TMath::Power( 10., 7.5*T/(237.3+T) ) * 1.e2;

   return Es;
}

int VAtmosphereSoundings::getMonth( string iT )
{
   if( iT == "Jan" ) return 1;
   if( iT == "Feb" ) return 2;
   if( iT == "Mar" ) return 3;
   if( iT == "Apr" ) return 4;
   if( iT == "May" ) return 5;
   if( iT == "Jun" ) return 6;
   if( iT == "Jul" ) return 7;
   if( iT == "Aug" ) return 8;
   if( iT == "Sep" ) return 9;
   if( iT == "Oct" ) return 10;
   if( iT == "Nov" ) return 11;
   if( iT == "Dec" ) return 12;

   return 0;
}

bool VAtmosphereSoundings::writeRootFile( string iFileName )
{
   TFile iFile( iFileName.c_str(), "RECREATE" );
   if( iFile.IsZombie() )
   {
      cout << "VAtmosphereSoundings::writeRootFile error opening root file " << iFileName << endl;
      return false;
   }
   TTree *t = new TTree( "tSoundings", "soundings data" );
   t->Branch( "MJD", &MJD, "MJD/D" );
   t->Branch( "Year", &Year, "Year/I" );
   t->Branch( "Month", &Month, "Month/I" );
   t->Branch( "Day", &Day, "Day/I" );
   t->Branch( "Hour", &Hour, "Hour/D" );
   t->Branch( "nPoints", &nPoints, "nPoints/i" );
   t->Branch( "Height_m", Height_m, "Height_m[nPoints]/D" );
   t->Branch( "Pressure_Pa", Pressure_Pa, "Pressure_Pa[nPoints]/D" );
   t->Branch( "Density_gcm3", Density_gcm3, "Density_gcm3[nPoints]/D" );
   t->Branch( "Thickness_gcm2", Thickness_gcm2, "Thickness_gcm2[nPoints]/D" );
   t->Branch( "Temperature_K", Temperature_K, "Temperature_K[nPoints]/D" );
   t->Branch( "DewPoint_K", DewPoint_K, "DewPoint_K[nPoints]/D" );
   t->Branch( "RelativeHumidity", RelativeHumidity, "RelativeHumidity[nPoints]/D" );
   t->Branch( "VaporMassDensity_gm3", VaporMassDensity_gm3, "VaporMassDensity_gm3[nPoints]/D" );
   t->Branch( "MixingRatio_gkg", MixingRatio_gkg, "MixingRatio_gkg[nPoints]/D" );
   t->Branch( "WindDirection_deg", WindDirection_deg, "WindDirection_deg[nPoints]/D" );
   t->Branch( "WindSpeed_ms", WindSpeed_ms, "WindSpeed_ms[nPoints]/D" );

   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      MJD   = fData[i]->MJD;
      Year  = fData[i]->Year;
      Month = fData[i]->Month;
      Day   = fData[i]->Day;
      Hour  = fData[i]->Hour;

      nPoints = fData[i]->fPressure_Pa.size();
      if( nPoints >= VMAXNUMBEROFSOUNDINGPOINTS )
      {
	 cout << "more than " << VMAXNUMBEROFSOUNDINGPOINTS << " points..." << endl;
         nPoints = VMAXNUMBEROFSOUNDINGPOINTS;
      }
      for( unsigned int j = 0; j < nPoints; j++ )
      {
         if( j < fData[i]->fPressure_Pa.size() )       Pressure_Pa[j]      = fData[i]->fPressure_Pa[j];
	 if( j < fData[i]->fHeight_m.size() )          Height_m[j]         = fData[i]->fHeight_m[j];
	 if( j < fData[i]->fDensity_gcm3.size() )      Density_gcm3[j]     = fData[i]->fDensity_gcm3[j];
	 if( j < fData[i]->fThickness_gcm2.size() )    Thickness_gcm2[j]   = fData[i]->fThickness_gcm2[j];
	 if( j < fData[i]->fTemperature_K.size() )     Temperature_K[j]    = fData[i]->fTemperature_K[j];
	 if( j < fData[i]->fDewPoint_K.size() )        DewPoint_K[j]       = fData[i]->fDewPoint_K[j];
	 if( j < fData[i]->fRelativeHumidity.size() )  RelativeHumidity[j] = fData[i]->fRelativeHumidity[j];
	 if( j < fData[i]->fVaporMassDensity_gm3.size() ) VaporMassDensity_gm3[j] = fData[i]->fVaporMassDensity_gm3[j];
         if( j < fData[i]->fMixingRatio_gkg.size() )   MixingRatio_gkg[j]  = fData[i]->fMixingRatio_gkg[j];
         if( j < fData[i]->fWindDirection_deg.size() ) WindDirection_deg[j]= fData[i]->fWindDirection_deg[j];
	 if( j < fData[i]->fWindSpeed_ms.size() )      WindSpeed_ms[j]     = fData[i]->fWindSpeed_ms[j];
      }

      t->Fill();
   }
   t->Write();

   iFile.Close();

   return true;
}

unsigned int VAtmosphereSoundings::getHistogramIdentifier( unsigned int i )
{
    if( i < fData.size() )
    {
       if( fPlottingPeriod == "yearly" )
       {
          return fData[i]->Year;
       }
       else if( fPlottingPeriod == "monthly" )
       {
	  return fData[i]->Year * 100 + fData[i]->Month;
       }
       else if( fPlottingPeriod == "all" )
       {
          return 100;
       }
       else if( fPlottingPeriod == "monthly_all" )
       {
          return fData[i]->Month;
       }
       else if( fPlottingPeriod == "day_night" )
       {
          if( fData[i]->Hour == 0 ) return 1;
          else if( fData[i]->Hour == 12 ) return (unsigned int)fData[i]->Hour;
          return 0;
       }
       else if( fPlottingPeriod == "bimonthly_all" )
       {
          return fData[i]->Month/2;
       }
       else if( fPlottingPeriod.find( ".dat" ) < fPlottingPeriod.size() )
       {
          return checkPlottingPeriodIdentifier( i );
       }
// choose similar winter months
       else if( fPlottingPeriod == "winter" )
       {
          if( fData[i]->Month == 12 || fData[i]->Month <=3 ) return 1;
       }
// choose summer months outside of monsoon season
       else if( fPlottingPeriod == "summer" )
       {
          if( fData[i]->Month == 6 || fData[i]->Month == 9 ) return 1;
       }
    }

    return  0;
}

unsigned int VAtmosphereSoundings::checkPlottingPeriodIdentifier( unsigned int i )
{
    if( i >= fData.size() ) return 0;

    unsigned int iDate = fData[i]->Year * 10000 + fData[i]->Month * 100 + fData[i]->Day;

    map< unsigned int, vector< unsigned int> >::iterator i_iter;
    for( i_iter = fPlottingPeriodDates.begin(); i_iter != fPlottingPeriodDates.end(); i_iter++ )
    {
       for( unsigned int j = 0; j < (*i_iter).second.size(); j++ )
       {
          if( (*i_iter).second[j] == iDate )
	  {
	     return (*i_iter).first;
          }
       }
    }

   return 0;
}

void VAtmosphereSoundings::setPlottingPeriod( string iPeriod )
{
    fPlottingPeriod = iPeriod;

    if( fPlottingPeriod.find( ".dat" ) < fPlottingPeriod.size() )
    {
       readPlottingPeriodsFromTextFile( fPlottingPeriod );
    }
}

bool VAtmosphereSoundings::readPlottingPeriodsFromTextFile( string iFile )
{
   if( iFile.size() == 0 ) return false;

   fPlottingPeriodFiles.clear();
   fPlottingPeriodDates.clear();

// get file names with run dates
   ifstream is;
   is.open( iFile.c_str() );
   if( !is )
   {
      cout << "VAtmosphereSoundings::readPlottingPeriodsFromTextFile(): file with plotting periods not found " << iFile << endl;
      return false;
   }
   string is_line;
   string iTemp;
   while( getline( is, is_line ) )
   {
       if( is_line.size() <= 0 ) continue;

       istringstream is_stream( is_line );

       is_stream >> iTemp;
       unsigned int iID = (unsigned int)atoi( iTemp.c_str() );
       is_stream >> iTemp;
       fPlottingPeriodFiles[iID] = iTemp;
    }
    is.close();
    cout << "reading plotting periods from " << fPlottingPeriodFiles.size() << " files " << endl;

// get run dates from different files
    map< unsigned int, string >::iterator i_iter;
    for( i_iter = fPlottingPeriodFiles.begin(); i_iter != fPlottingPeriodFiles.end(); i_iter++ )
    {
       ifstream is;
       is.open( (*i_iter).second.c_str() );
       if( !is )
       {
          cout << "error opening file for plotting periods " << (*i_iter).first << "(" << (*i_iter).second << ")" << endl;
	  continue;
       }
       cout << "reading plotting dates for " << (*i_iter).first << "(" << (*i_iter).second << ")" << endl;
       unsigned int iDate = 0;
       while( getline( is, is_line ) )
       {
           if( is_line.size() <= 0 ) continue;

	   istringstream is_stream( is_line );

	   is_stream >> iTemp;

	   iDate = (unsigned int)atoi( iTemp.c_str() );

	   fPlottingPeriodDates[(*i_iter).first].push_back( iDate );
       }
       is.close();

       cout << "\t read " << fPlottingPeriodDates[(*i_iter).first].size() << " days " << endl;
    }

    return true;
}




void VAtmosphereSoundings::plot2DProfiles( unsigned int iYearStart, unsigned int iMonthStart, unsigned int iYearStop, unsigned int iMonthStop )
{
   plotProfiles( iYearStart, iMonthStart, iYearStop, iMonthStop, true );
}

void VAtmosphereSoundings::plotAverages( unsigned int iYearStart, unsigned int iMonthStart, unsigned int iYearStop, unsigned int iMonthStop, string iPlotOption, bool iSame )
{
   plotProfiles( iYearStart, iMonthStart, iYearStop, iMonthStop, false, iPlotOption, iSame );
}

void VAtmosphereSoundings::plotProfiles( unsigned int iYearStart, unsigned int iMonthStart, unsigned int iYearStop, unsigned int iMonthStop, bool b2D, string iPlotOption, bool bSames )
{
   vector< string > iXaxis;
   vector< string > iYaxis;
   vector< int >    iXbin;
   vector< int >    iYbin;
   vector< double > iXmin;
   vector< double > iXmax;
   vector< double > iYmin;
   vector< double > iYmax;

// temperature vs height
   iXaxis.push_back( "height [km]" );
   iYaxis.push_back( "temperature [K]" );
   iXbin.push_back( 50 );
   iYbin.push_back( 100 );
   iXmin.push_back( fPlottingHeight_min );
   iXmax.push_back( fPlottingHeight_max );
   iYmin.push_back( 0. );
   iYmax.push_back( 500. );
   if( b2D )
   {
      iYmin.back() = 150.;
      iYmax.back() = 360.;
   }

// dewpoint vs height
   iXaxis.push_back( "height [km]" );
   iYaxis.push_back( "dewpoint [K]" );
   iXbin.push_back( 50 );
   iYbin.push_back( 100 );
   iXmin.push_back( fPlottingHeight_min );
   iXmax.push_back( fPlottingHeight_max );
   iYmin.push_back( 0. );
   iYmax.push_back( 500. );
   if( b2D )
   {
      iYmin.back() = 150.;
      iYmax.back() = 360.;
   }

// pressure vs height
   iXaxis.push_back( "height [km]" );
   iYaxis.push_back( "pressure [Pa]" );
   iXbin.push_back( 50 );
   iYbin.push_back( 100 );
   iXmin.push_back( fPlottingHeight_min );
   iXmax.push_back( fPlottingHeight_max );
   iYmin.push_back( 1. );
   iYmax.push_back( 1.e6 );
   if( b2D )
   {
      iYmin.back() = 0.;
      iYmax.back() = 6.;
   }

// density vs height
   iXaxis.push_back( "height [km]" );
   iYaxis.push_back( "density * exp( height/7.739 km ) [g/cm2]" );
   if( fPlotRelativePlots ) iYaxis.back() = "density (relative plot)";
   iXbin.push_back( 50 );
   iYbin.push_back( 100 );
   iXmin.push_back( fPlottingHeight_min );
   iXmax.push_back( fPlottingHeight_max );
   iYmin.push_back( 0. );
   iYmax.push_back( 1.e6 );
   if( b2D )
   {
      iYbin.back() = 200;
      iXbin.back() = 100;
      iYmin.back() = 0.0008;
      iYmax.back() = 0.0017;
   }

// relative humidity vs height
   iXaxis.push_back( "height [km]" );
   iYaxis.push_back( "relative humidity [%]" );
   if( fPlotRelativePlots ) iYaxis.back() = "rel humidity(relative plot)";
   iXbin.push_back( 50 );
   iYbin.push_back( 100 );
   iXmin.push_back( fPlottingHeight_min );
   iXmax.push_back( fPlottingHeight_max );
   iYmin.push_back( 0. );
   iYmax.push_back( 1.e2 );
   if( b2D )
   {
      iYbin.back() = 200;
      iXbin.back() = 100;
      iYmin.back() = 0.;
      iYmax.back() = 1.e2;
   }

// thickness vs height
   iXaxis.push_back( "height [km]" );
   iYaxis.push_back( "atmospheric thickness [g/cm2]" );
   if( fPlotRelativePlots ) iYaxis.back() = "atmospheric thickness(relative plot)";
   iXbin.push_back( 50 );
   iYbin.push_back( 100 );
   iXmin.push_back( fPlottingHeight_min );
   iXmax.push_back( fPlottingHeight_max );
   iYmin.push_back( 0. );
   iYmax.push_back( 5.e4 );
   if( b2D )
   {
      iYbin.back() = 200;
      iXbin.back() = 100;
      iYmin.back() = 0.;
      iYmax.back() = 1.2e3;
   }


/////////////////////////////
// create and fill histograms
   char hname[800];
   char htitle[800];

   vector< map< unsigned int, TProfile* > > hMonthly;
   map< unsigned int, TProfile* > iTempMap;
   vector< TH2D* > hProfile2D;
   vector< TProfile* > hProfileAll;

   for( unsigned int k = 0; k < iXaxis.size(); k++ )
   {
      hMonthly.push_back( iTempMap );

      if( b2D )
      {
	 sprintf( hname, "h%d", k );
	 hProfile2D.push_back( new TH2D( hname, "", iXbin[k], iXmin[k], iXmax[k], iYbin[k], iYmin[k], iYmax[k] ) );
	 hProfile2D[k]->SetStats( 0 );
	 hProfile2D[k]->SetXTitle( iXaxis[k].c_str() );
	 hProfile2D[k]->SetYTitle( iYaxis[k].c_str() );
      }
      else hProfile2D.push_back( 0 );
      sprintf( hname, "hP%d", k );
      hProfileAll.push_back( new TProfile( hname, "", iXbin[k], iXmin[k], iXmax[k], iYmin[k], iYmax[k] ) );
      hProfileAll[k]->SetStats( 0 );
      hProfileAll[k]->SetXTitle( iXaxis[k].c_str() );
      hProfileAll[k]->SetYTitle( iYaxis[k].c_str() );
      hProfileAll[k]->SetMarkerStyle( 7 );
   }

   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      unsigned int iID = getHistogramIdentifier( i );

// ignore all 0 identifiers
      if( iID == 0 ) continue;

      for( unsigned int k = 0; k < iXaxis.size(); k++ )
      {
	 if( hMonthly[k].find( iID ) == hMonthly[k].end() )
	 {
	    sprintf( hname, "h%d%d", k, iID );
	    if( !b2D ) hMonthly[k][iID] = new TProfile( hname, "", iXbin[k], iXmin[k], iXmax[k], iYmin[k], iYmax[k] );
	    else       hMonthly[k][iID] = 0;
	    if( hMonthly[k][iID] )
            {
	       if( fBoolColorChange )
	       {
	          int iColor = hMonthly[k].size();
		  if( iColor > 9 ) iColor++;
		  hMonthly[k][iID]->SetMarkerColor( iColor );
		  hMonthly[k][iID]->SetLineColor( iColor );
	          hMonthly[k][iID]->SetMarkerStyle( 20 );
               }
	       else
	       {
	          hMonthly[k][iID]->SetMarkerStyle( 20 );
               }
	       hMonthly[k][iID]->SetStats( 0 );
	       hMonthly[k][iID]->SetXTitle( iXaxis[k].c_str() );
	       hMonthly[k][iID]->SetYTitle( iYaxis[k].c_str() );
            }
	 }
// check time range
         if( fData[i]->Year * 100 + fData[i]->Month >= (int)(iYearStart*100+iMonthStart) && fData[i]->Year*100 + fData[i]->Month <= (int)(iYearStop*100 + iMonthStop ) )
	 {
	    if( k == 0 )
	    {
	       for( unsigned int j = 0; j < fData[i]->fTemperature_K.size(); j++ )
	       {
		  if( fData[i]->fTemperature_K[j] > 0. && fData[i]->fHeight_m[j] > 0. )
		  {
		     if( hMonthly[k][iID] ) hMonthly[k][iID]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fTemperature_K[j] );
		     if( hProfile2D[k] ) hProfile2D[k]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fTemperature_K[j] );
		     if( hProfileAll[k] ) hProfileAll[k]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fTemperature_K[j] );
		  }
	       }
            }
	    else if( k == 1 )
	    {
	       for( unsigned int j = 0; j < fData[i]->fDewPoint_K.size(); j++ )
	       {
		  if( fData[i]->fDewPoint_K[j] > 0. && fData[i]->fHeight_m[j] > 0. )
		  {
		     if( hMonthly[k][iID] ) hMonthly[k][iID]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fDewPoint_K[j] );
		     if( hProfile2D[k] ) hProfile2D[k]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fDewPoint_K[j] );
		     if( hProfileAll[k] ) hProfileAll[k]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fDewPoint_K[j] );
		  }
	       }
            }
	    else if( k == 2 )
	    {
	       for( unsigned int j = 0; j < fData[i]->fPressure_Pa.size(); j++ )
	       {
		  if( fData[i]->fPressure_Pa[j] > 0. && fData[i]->fHeight_m[j] > 0 )
		  {
		     if( hMonthly[k][iID] ) hMonthly[k][iID]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fPressure_Pa[j] );
		     if( hProfile2D[k] ) hProfile2D[k]->Fill( fData[i]->fHeight_m[j]/1.e3, log10( fData[i]->fPressure_Pa[j] ) );
		     if( hProfileAll[k] ) hProfileAll[k]->Fill( fData[i]->fHeight_m[j]/1.e3, log10( fData[i]->fPressure_Pa[j] ) );
		  }
	       }
            }
	    else if( k == 3 )
	    {
	       for( unsigned int j = 0; j < fData[i]->fDensity_gcm3.size(); j++ )
	       {
		  if( fData[i]->fDensity_gcm3[j] > 0. && fData[i]->fHeight_m[j] > 0. )
		  {
		     if( !fPlotRelativePlots )
		     {
			if( hMonthly[k][iID] ) hMonthly[k][iID]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fDensity_gcm3[j] * TMath::Exp( fData[i]->fHeight_m[j]/1.e3/7.739 ) );
			if( hProfile2D[k] )  hProfile2D[k]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fDensity_gcm3[j] * TMath::Exp( fData[i]->fHeight_m[j]/1.e3/7.739 ) );
			if( hProfileAll[k] ) hProfileAll[k]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fDensity_gcm3[j] * TMath::Exp( fData[i]->fHeight_m[j]/1.e3/7.739 ) );
                     }
                     else
		     {
			if( hMonthly[k][iID] ) hMonthly[k][iID]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fDensity_gcm3[j] );
			if( hProfile2D[k] )  hProfile2D[k]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fDensity_gcm3[j] );
			if( hProfileAll[k] ) hProfileAll[k]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fDensity_gcm3[j] );
                     }
		  }
	       }
            }
	    else if( k == 4 )
	    {
	       for( unsigned int j = 0; j < fData[i]->fRelativeHumidity.size(); j++ )
	       {
		  if( fData[i]->fRelativeHumidity[j] > 0. && fData[i]->fHeight_m[j] > 0 )
		  {
		     if( hMonthly[k][iID] ) hMonthly[k][iID]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fRelativeHumidity[j] );
		     if( hProfile2D[k] ) hProfile2D[k]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fRelativeHumidity[j] );
		     if( hProfileAll[k] ) hProfileAll[k]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fRelativeHumidity[j] );
		  }
	       }
            }
	    else if( k == 5 )
	    {
	       for( unsigned int j = 0; j < fData[i]->fThickness_gcm2.size(); j++ )
	       {
		  if( fData[i]->fThickness_gcm2[j] > 0. && fData[i]->fHeight_m[j] > 0 )
		  {
		     if( hMonthly[k][iID] ) hMonthly[k][iID]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fThickness_gcm2[j] );
		     if( hProfile2D[k] ) hProfile2D[k]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fThickness_gcm2[j] );
		     if( hProfileAll[k] ) hProfileAll[k]->Fill( fData[i]->fHeight_m[j]/1.e3, fData[i]->fThickness_gcm2[j] );
		  }
	       }
            }
	 }	 
      }
   }
/////////////////////////////////////////////////////////////////////////
// plot everything

   TCanvas *cTemp = 0;

   if( !bSames )
   {
      fCanvasProfile.clear();
      fCanvas2D.clear();
   }
   
   for( unsigned int k = 0; k < hMonthly.size(); k++ )
   {
      if( !bSames )
      {
	 sprintf( hname, "c%s%s%d", iYaxis[k].c_str(), iXaxis[k].c_str(), b2D );
	 sprintf( htitle, "%s vs %s", iYaxis[k].c_str(), iXaxis[k].c_str() );
	 if( !b2D )
	 {
	    fCanvasProfile.push_back( new TCanvas( hname, htitle, 10, 10, 600, 400 ) );
	    cTemp = fCanvasProfile.back();
         }
	 else
	 {
	    fCanvas2D.push_back(  new TCanvas( hname, htitle, 10, 10, 600, 400 ) );
	    cTemp = fCanvas2D.back();
         }
	 cTemp->SetLeftMargin( 0.13 );
	 cTemp->SetGridx( 0 );
	 cTemp->SetGridy( 0 );
	 cTemp->Draw();

	 fPlottingLegend.push_back( new TLegend( 0.5, 0.15, 0.59, 0.35 ) );
      }
      else
      {
         if( !b2D && k < fCanvasProfile.size() ) cTemp = fCanvasProfile[k];
	 else if( k < fCanvas2D.size() )         cTemp = fCanvas2D[k];
	 else continue;
      }


      if( b2D )
      {
         hProfile2D[k]->Draw( "colz" );
	 hProfile2D[k]->GetYaxis()->SetTitleOffset( 1.5 );
	 if( hProfileAll[k] ) hProfileAll[k]->Draw( "same" );
	 cTemp->Update();
      }
      else
      {
	 map< unsigned int, TProfile* >::iterator iterTempMap;
	 
	 int z = 0;
	 for( iterTempMap = hMonthly[k].begin(); iterTempMap != hMonthly[k].end(); iterTempMap++ )
	 {
	    TProfile *h = (TProfile*)(*iterTempMap).second;
	    if( h )
	    {
		if( h->GetEntries() > 0 )
		{
		   if( fPlotRelativePlots ) h->Divide( hProfileAll[k] );
		   if( z == 0 && !bSames )
		   {
		      sprintf( hname, "%s", iPlotOption.c_str() );
		      h->GetYaxis()->SetTitleOffset( 1.5 );
		      if( k == 3 )      h->SetAxisRange( 0.0004, 0.0017, "Y" );       // density * XXX
		      else if( k == 0 ) h->SetAxisRange( 180., 340., "Y" );           // temperature
		      else if( k == 1 ) h->SetAxisRange( 180., 340., "Y" );           // dew point
		      else if( k == 2 ) h->SetAxisRange( 8.2, 1.5e5, "Y" );           // pressure
		      else if( k == 4 ) h->SetAxisRange( 1.e-2, 100., "Y" );          // relative humidity
		      else if( k == 5 ) h->SetAxisRange( 1.e-2, 1.2e3, "Y" );           // atmospheric thickness
		      h->Draw( hname );
		   }
		   else
		   {
		      sprintf( hname, "same %s", iPlotOption.c_str() );
		      h->Draw( hname );
		   }
		   z++;
		   cTemp->Update();
		   sprintf( hname, "%d", (*iterTempMap).first );
		   if( fPlottingLegend[k] ) fPlottingLegend[k]->AddEntry( h, hname, "pl" );
                }

	    }
	 }
      }
// add CORSIKA and user defined atmosphere plots
      if( k == 0 )
      {
         plotCORSIKA_Temperature_vs_Heigth( cTemp );
	 plotUserAtmosphere_Temperature_vs_Heigth( cTemp );
      }
      else if( k == 1 )
      {
         plotCORSIKA_DewPoint_vs_Heigth( cTemp );
	 plotUserAtmosphere_DewPoint_vs_Heigth( cTemp );
      }
      else if( k == 2 )
      {
         plotCORSIKA_Pressure_vs_Heigth( cTemp );
	 plotUserAtmosphere_Pressure_vs_Heigth( cTemp );
      }
      else if( k == 3 )
      {
         plotCORSIKA_Density_vs_Heigth( cTemp );
	 plotUserAtmosphere_Density_vs_Heigth( cTemp );
      }
      else if( k == 4 )
      {
         plotCORSIKA_RelativeHumidity_vs_Heigth( cTemp );
	 plotUserAtmosphere_RelativeHumidity_vs_Heigth( cTemp );
      }
      else if( k == 5 )
      {
         plotCORSIKA_Thickness_vs_Heigth( cTemp );
	 plotUserAtmosphere_Thickness_vs_Heigth( cTemp );
      }

      if( fPlottingLegendDraw && fPlottingLegend[k] ) fPlottingLegend[k]->Draw();
   }
}

bool VAtmosphereSoundings::readRootFile()
{
   if( !fDataTree ) return false;

   fDataTree->SetBranchAddress( "MJD", &MJD );
   fDataTree->SetBranchAddress( "Year", &Year );
   fDataTree->SetBranchAddress( "Month", &Month );
   fDataTree->SetBranchAddress( "Day", &Day );
   fDataTree->SetBranchAddress( "Hour", &Hour );
   fDataTree->SetBranchAddress( "nPoints", &nPoints );
   fDataTree->SetBranchAddress( "Height_m", Height_m );
   fDataTree->SetBranchAddress( "Pressure_Pa", Pressure_Pa );
   fDataTree->SetBranchAddress( "Density_gcm3", Density_gcm3 );
   fDataTree->SetBranchAddress( "Thickness_gcm2", Thickness_gcm2 );
   fDataTree->SetBranchAddress( "Temperature_K", Temperature_K );
   fDataTree->SetBranchAddress( "DewPoint_K", DewPoint_K );
   fDataTree->SetBranchAddress( "RelativeHumidity", RelativeHumidity );
   fDataTree->SetBranchAddress( "VaporMassDensity_gm3", VaporMassDensity_gm3 );
   fDataTree->SetBranchAddress( "MixingRatio_gkg", MixingRatio_gkg );
   fDataTree->SetBranchAddress( "WindDirection_deg", WindDirection_deg );
   fDataTree->SetBranchAddress( "WindSpeed_ms", WindSpeed_ms );

   for( unsigned int i = 0; i < fDataTree->GetEntries(); i++ )
   {
      fDataTree->GetEntry( i );

      fData.push_back( new VAtmosphereSoundingData() );

      fData[i]->MJD = MJD;
      fData[i]->Year = Year;
      fData[i]->Month = Month;
      fData[i]->Day = Day;
      fData[i]->Hour = Hour;

      for( unsigned int j = 0; j < nPoints; j++ )
      {
         fData[i]->fPressure_Pa.push_back( Pressure_Pa[j] );
	 fData[i]->fHeight_m.push_back( Height_m[j] ); 
	 fData[i]->fDensity_gcm3.push_back( Density_gcm3[j] );
	 fData[i]->fThickness_gcm2.push_back( Thickness_gcm2[j] );
	 fData[i]->fTemperature_K.push_back( Temperature_K[j] );
	 fData[i]->fDewPoint_K.push_back( DewPoint_K[j] );
	 fData[i]->fRelativeHumidity.push_back( RelativeHumidity[j] );
	 if( fData[i]->fRelativeHumidity.back() > 0. ) fData[i]->fRelativeHumidity.back() *= 1.e2;
	 fData[i]->fVaporMassDensity_gm3.push_back( VaporMassDensity_gm3[j] );
	 fData[i]->fIndexofRefraction.push_back( 0. );
	 fData[i]->fMixingRatio_gkg.push_back( MixingRatio_gkg[j] );
	 fData[i]->fWindDirection_deg.push_back( WindDirection_deg[j] );
	 fData[i]->fWindSpeed_ms.push_back( WindSpeed_ms[j] );
      }

   }

   cout << "total number of data sets available: " << fData.size() << endl;
   return true;
}

TCanvas* VAtmosphereSoundings::plotCORSIKA( TCanvas *c, int iPlotID, vector< VAtmosphereSoundingData* > iData, double iHeightMin, double iHeightMax )
{
    char hname[800];

    string iCTitle;
    string iHTitle;
    string iXTitle;
    double iYmin = 0.;
    double iYmax = 0.;

// density vs height
    if( iPlotID == 0 )
    {
       iHTitle = "CORSIKADH";
       iCTitle = "CORSIKA (density vs height)";
       iXTitle = "density * exp( height/7.739 km ) [kg/cm2]";
       iYmin = 0.0004;
       iYmax = 0.0017;
       if( iHeightMax < 40. ) iYmin = 0.0004;
    }
// index of refraction vs height
    else if( iPlotID == 1 )
    {
       iHTitle = "CORSIKANH";
       iCTitle = "CORSIKA (index of refraction vs height)";
       iXTitle = "index of refraction [n-1]";
       iYmin = 1.e-10;
       iYmax = 0.00033;
       if( iHeightMax < 7. ) iYmin = 0.15e-3;
    }
// temperature vs height
    else if( iPlotID == 2 )
    {
       iHTitle = "CORSIKATH";
       iCTitle = "CORSIKA (temperature vs height)";
       iXTitle = "temperature [K]";
       iYmin = 150;
       iYmax = 350;
    }
// ozone vs height
    else if( iPlotID == 3 )
    {
       iHTitle = "CORSIKAO3";
       iCTitle = "CORSIKA (ozone vs height)";
       iXTitle = "ozone [ATM cm/km]";
       iYmin = 1.e-8;
       iYmax = 1.e-1;
    }
// relative humidity vs height
    else if( iPlotID == 4 )
    {
       iHTitle = "CORSIKAO4";
       iCTitle = "CORSIKA (relative humidity vs height)";
       iXTitle = "relative humidity [%]";
       iYmin = 0.;
       iYmax = 100.;
    }
// dew point vs height
    else if( iPlotID == 5 )
    {
       iHTitle = "CORSIKAO5";
       iCTitle = "CORSIKA (dew point vs height)";
       iXTitle = "dew point [K]";
       iYmin = 100.;
       iYmax = 400.;
    }
// pressure vs height
    else if( iPlotID == 6 )
    {
       iHTitle = "CORSIKAO6";
       iCTitle = "CORSIKA (pressure vs height)";
       iXTitle = "pressure [Pa]";
       iYmin = 0.;
       iYmax = 1.e6;
    }
// thickness vs height
    else if( iPlotID == 7 )
    {
       iHTitle = "CORSIKAO7";
       iCTitle = "CORSIKA (thickness vs height)";
       iXTitle = "atmospheric thickness [g/cm2]";
       iYmin = 0.;
       iYmax = 1.e6;
    }


//////////////////////////////////////////////////////
    if( c ) 
    {
       c->cd();
    }
    else
    {
       sprintf( hname, "c%s", iHTitle.c_str() );
       c = new TCanvas( hname, iCTitle.c_str(), 10, 10, 600, 400 );
       c->SetGridx( 0 );
       c->SetGridy( 0 );
       sprintf( hname, "h%s", iHTitle.c_str() );
       TH1D *h = new TH1D( hname, "", 100, iHeightMin, iHeightMax );
       h->SetStats( 0 );
       h->SetMinimum( iYmin );
       h->SetMaximum( iYmax );
       h->SetXTitle( "height[km]" );
       h->SetYTitle( iXTitle.c_str() );
       h->Draw();
    }
    vector< TGraph* > g;

    TLegend *iL = 0;
    if( iData.size() == 2 ) iL = new TLegend( 0.55, 0.65, 0.85, 0.75 );
    else                    iL = new TLegend( 0.65, 0.55, 0.85, 0.75 );

    for( unsigned int i = 0; i < iData.size(); i++ )
    {
       int z = 0;
       g.push_back( new TGraph( 1 ) );
       g.back()->SetLineColor( iData[i]->PlotColor );
       g.back()->SetMarkerColor( iData[i]->PlotColor );
       g.back()->SetLineStyle( iData[i]->PlotLineStyle );
       g.back()->SetLineWidth( iData[i]->PlotLineWidth );

       for( unsigned int j = 0; j < iData[i]->fDensity_gcm3.size(); j++ )
       {
	   if( iPlotID == 0 )
	   {
	      if( iData[i]->fDensity_gcm3[j] > -90000. && iData[i]->fHeight_m[j] > -90000. )
	      {
		 g.back()->SetPoint( z, iData[i]->fHeight_m[j]/1.e3, iData[i]->fDensity_gcm3[j] * TMath::Exp( iData[i]->fHeight_m[j]/7.739/1.e3 ) );
		 z++;
	      }
           }
	   else if( iPlotID == 1 )
	   {
	      if( iData[i]->fIndexofRefraction[j] > -90000. && iData[i]->fHeight_m[j] > -90000. )
	      {
	         g.back()->SetPoint( z, iData[i]->fHeight_m[j]/1.e3, iData[i]->fIndexofRefraction[j] - 1. );
                 z++;
              }
           }
	   else if( iPlotID == 2 )
	   {
	      if( iData[i]->fTemperature_K[j] > 0. && iData[i]->fHeight_m[j] > -90000. )
	      {
	         g.back()->SetPoint( z, iData[i]->fHeight_m[j]/1.e3, iData[i]->fTemperature_K[j] );
                 z++;
              }
           }
	   else if( iPlotID == 3 )
	   {
	      if( iData[i]->fO3_cmkm[j] > 0. && iData[i]->fHeight_m[j] > -90000. )
	      {
	         g.back()->SetPoint( z, iData[i]->fHeight_m[j]/1.e3, iData[i]->fO3_cmkm[j] );
                 z++;
              }
           }
	   else if( iPlotID == 4 )
	   {
	      if( iData[i]->fRelativeHumidity[j] > 0. && iData[i]->fHeight_m[j] > -90000. )
	      {
	         g.back()->SetPoint( z, iData[i]->fHeight_m[j]/1.e3, iData[i]->fRelativeHumidity[j] );
                 z++;
              }
           }
	   else if( iPlotID == 5 )
	   {
	      if( iData[i]->fDewPoint_K[j] > 0. && iData[i]->fHeight_m[j] > -90000. )
	      {
	         g.back()->SetPoint( z, iData[i]->fHeight_m[j]/1.e3, iData[i]->fDewPoint_K[j] );
                 z++;
              }
           }
	   else if( iPlotID == 6 )
	   {
	      if( iData[i]->fPressure_Pa[j] > 0. && iData[i]->fHeight_m[j] > -90000. )
	      {
	         g.back()->SetPoint( z, iData[i]->fHeight_m[j]/1.e3, iData[i]->fPressure_Pa[j] );
                 z++;
              }
           }
	   else if( iPlotID == 7 )
	   {
	      if( iData[i]->fThickness_gcm2[j] > 0. && iData[i]->fHeight_m[j] > -90000. )
	      {
	         g.back()->SetPoint( z, iData[i]->fHeight_m[j]/1.e3, iData[i]->fThickness_gcm2[j] );
                 z++;
              }
           }
       }
       if( g.back()->GetN() > 1 )
       {
          g.back()->Draw( "l" );
	  iL->AddEntry( g.back(), iData[i]->Name.c_str(), "pl" );
       }
    }
    if( iData.size() > 0 ) iL->Draw();

    return c;
}
	       
/*
   add a new profile from soundings and MODTRAN/CORSIKA data

   (some values are hardcoded here!)

   iIndexCORSIKAMODTRAN   : index of MODTRAN data set to use
   iHeightMaxData         : height [km] to change between sounding data and model
   iName                  : profile name (currently not used)
*/
bool VAtmosphereSoundings::add_user_Atmosphere( unsigned int iIndexCORSIKAMODTRAN, double iHeightMaxData, string iName )
{
   if( iIndexCORSIKAMODTRAN >= fDataCORSIKAMODTRAN.size() )
   {
     cout << "VAtmosphereSoundings::add_user_profile: data set index out of range (MODTRAN): ";
     cout << iIndexCORSIKAMODTRAN << "\t" << fDataCORSIKAMODTRAN.size() << endl;
     return false;
   }
   if( !fDataCORSIKAMODTRAN[iIndexCORSIKAMODTRAN] )
   {
     cout << "VAtmosphereSoundings::add_user_profile: empty data set (MODTRAN): " << iIndexCORSIKAMODTRAN << endl;
     return false;
   }
//////////////////////////////////////////////////////////////////////
// create a new user profile
   fDataUserProfile.push_back( new VAtmosphereSoundingData() );
   fDataUserProfile.back()->Name  = iName;

// use height steps from Monte Carlo
   fDataUserProfile.back()->setdefaultvalues( fDataCORSIKAMODTRAN[iIndexCORSIKAMODTRAN]->fHeight_m.size() );
   for( unsigned int i = 0; i < fDataCORSIKAMODTRAN[iIndexCORSIKAMODTRAN]->fHeight_m.size(); i++ )
   {
      fDataUserProfile.back()->fHeight_m[i] = fDataCORSIKAMODTRAN[iIndexCORSIKAMODTRAN]->fHeight_m[i];
   }

// pressure 
   fDataUserProfile.back()->fPressure_Pa = getDataVectorForUserAtmosphere( iHeightMaxData, fDataCORSIKAMODTRAN[iIndexCORSIKAMODTRAN], "pressure" );
// temperature 
   fDataUserProfile.back()->fTemperature_K = getDataVectorForUserAtmosphere( iHeightMaxData, fDataCORSIKAMODTRAN[iIndexCORSIKAMODTRAN], "temperature" );
// dew point
   fDataUserProfile.back()->fDewPoint_K = getDataVectorForUserAtmosphere( iHeightMaxData, fDataCORSIKAMODTRAN[iIndexCORSIKAMODTRAN], "dewpoint" );
// relative humidity
   fDataUserProfile.back()->fRelativeHumidity = getDataVectorForUserAtmosphere( iHeightMaxData, fDataCORSIKAMODTRAN[iIndexCORSIKAMODTRAN], "relhumid" );
// dew point (from relative humidity and temperature)
/*   for( unsigned int i = 0; i < fDataUserProfile.back()->fRelativeHumidity.size(); i++ )
   {
      if( i < fDataUserProfile.back()->fDewPoint_K.size() )
      {
         fDataUserProfile.back()->fDewPoint_K[i] = getDewPoint( fDataUserProfile.back()->fTemperature_K[i], fDataUserProfile.back()->fRelativeHumidity[i], 1 );
      }
      else
      {
         fDataUserProfile.back()->fDewPoint_K.push_back( getDewPoint( fDataUserProfile.back()->fTemperature_K[i], fDataUserProfile.back()->fRelativeHumidity[i], 1 ) );
      }
   } */
// atmospheric thickness
   fillAtmosphericThickness( fDataUserProfile.back() );
// atmospheric density
   fillAtmosphericDensity( fDataUserProfile.back() );
      
   return true;
}

vector< double > VAtmosphereSoundings::getDataVectorForUserAtmosphere( double iHeightMaxData, VAtmosphereSoundingData* iDataMonteCarlo, string iType )
{
   vector< double > f;   // return vector
   vector< double > n;   // counting vector
   if( !iDataMonteCarlo ) return f;

   f.assign( iDataMonteCarlo->fHeight_m.size(), 0. );
   n.assign( iDataMonteCarlo->fHeight_m.size(), 0. );

// km -> m
   iHeightMaxData *= 1.e3;

// type IDs
   unsigned int iTypeID = 0;
   if( iType == "pressure" )         iTypeID = 1;
   else if( iType == "temperature" ) iTypeID = 2;
   else if( iType == "dewpoint" )    iTypeID = 3;
   else if( iType == "relhumid" )    iTypeID = 4;

// unknown type
   if( iTypeID == 0 ) return f;

// calculate height bins
   vector< double > iHeight_min( iDataMonteCarlo->fHeight_m.size(), 0. );
   vector< double > iHeight_max( iDataMonteCarlo->fHeight_m.size(), 0. );

   for( unsigned int i = 0; i < iDataMonteCarlo->fHeight_m.size(); i++ )
   {
      if( i == 0 ) iHeight_min[i] = iDataMonteCarlo->fHeight_m[0];
      else
      {
         iHeight_min[i] = 0.5*( iDataMonteCarlo->fHeight_m[i] + iDataMonteCarlo->fHeight_m[i-1] );
      }
      if( i == iDataMonteCarlo->fHeight_m.size()-1 ) iHeight_max[i] = iDataMonteCarlo->fHeight_m[i];
      else if( i == 0 ) iHeight_max[i]  = 0.5*( iDataMonteCarlo->fHeight_m[i] + iDataMonteCarlo->fHeight_m[i+1] );
      else
      {
         iHeight_max[i] = iDataMonteCarlo->fHeight_m[i] + (iDataMonteCarlo->fHeight_m[i]-iHeight_min[i] );
      }
   }

// loop over all data sets
   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      unsigned int iID = getHistogramIdentifier( i );

// ignore all 0 identifiers
      if( iID == 0 ) continue;

////////////////////////////////////////////////////////////////////////
// pressure
      if( iTypeID == 1 )
      {
// loop over this data set
	 for( unsigned int j = 0; j < fData[i]->fPressure_Pa.size(); j++ )
	 {
	     if( fData[i]->fPressure_Pa[j] > 0. )
	     {
// fill data value for heights < iHeightMaxData
//		if( fData[i]->fHeight_m[j] < iHeightMaxData )
		{
		   for( unsigned int k = 1; k < iDataMonteCarlo->fHeight_m.size(); k++ )
		   {
		      if( fData[i]->fHeight_m[j] > iHeight_min[k] && fData[i]->fHeight_m[j] < iHeight_max[k] )
		      {
			 f[k] += fData[i]->fPressure_Pa[j];
			 n[k] ++;
		      }
		   }
		}
              }
	  }
      }
////////////////////////////////////////////////////////////////////////
// temperature
      else if( iTypeID == 2 )
      {
// loop over this data set
	 for( unsigned int j = 0; j < fData[i]->fTemperature_K.size(); j++ )
	 {
	     if( fData[i]->fTemperature_K[j] > 0. )
	     {
// fill data value for heights < iHeightMaxData
//		if( fData[i]->fHeight_m[j] < iHeightMaxData )
		{
		   for( unsigned int k = 1; k < iDataMonteCarlo->fHeight_m.size(); k++ )
		   {
		      if( fData[i]->fHeight_m[j] > iHeight_min[k] && fData[i]->fHeight_m[j] < iHeight_max[k] )
		      {
			 f[k] += fData[i]->fTemperature_K[j];
			 n[k] ++;
		      }
		   }
		}
             }
	  }
      }
////////////////////////////////////////////////////////////////////////
// dew point
      else if( iTypeID == 3 )
      {
// loop over this data set
	 for( unsigned int j = 0; j < fData[i]->fDewPoint_K.size(); j++ )
	 {
	     if( fData[i]->fDewPoint_K[j] > 0. )
	     {
// fill data value for heights < iHeightMaxData
//		if( fData[i]->fHeight_m[j] < iHeightMaxData )
		{
		   for( unsigned int k = 1; k < iDataMonteCarlo->fHeight_m.size(); k++ )
		   {
		      if( fData[i]->fHeight_m[j] > iHeight_min[k] && fData[i]->fHeight_m[j] < iHeight_max[k] )
		      {
			 f[k] += fData[i]->fDewPoint_K[j];
			 n[k] ++;
		      }
		   }
		}
             }
	  }
      }
////////////////////////////////////////////////////////////////////////
// relative humidity
      else if( iTypeID == 4 )
      {
// loop over this data set
	 for( unsigned int j = 0; j < fData[i]->fRelativeHumidity.size(); j++ )
	 {
	     if( fData[i]->fRelativeHumidity[j] >= 0. && fData[i]->fRelativeHumidity[j] <= 100. )
	     {
// fill data value for heights < iHeightMaxData
//		if( fData[i]->fHeight_m[j] < iHeightMaxData )
		{
		   for( unsigned int k = 1; k < iDataMonteCarlo->fHeight_m.size(); k++ )
		   {
		      if( fData[i]->fHeight_m[j] > iHeight_min[k] && fData[i]->fHeight_m[j] < iHeight_max[k] )
		      {
			 f[k] += fData[i]->fRelativeHumidity[j];
			 n[k] ++;
		      }
		   }
		}
             }
	  }
      }
   }

////////////////////////////////////////////////////////////////////////
// calculate mean value
   for( unsigned int i = 0; i < f.size(); i++ )
   {
      if( i < n.size() && n[i] > 0. ) f[i] /= n[i];
      else                            f[i]  = -9999;

// fill MC values for > iHeightMaxData
      if( iDataMonteCarlo->fHeight_m[i] >= iHeightMaxData )
      {
         if( iTypeID == 1 )      f[i] = iDataMonteCarlo->fPressure_Pa[i];
	 else if( iTypeID == 2 ) f[i] = iDataMonteCarlo->fTemperature_K[i];
	 else if( iTypeID == 3 ) f[i] = iDataMonteCarlo->fDewPoint_K[i];
	 else if( iTypeID == 4 ) f[i] = iDataMonteCarlo->fRelativeHumidity[i];
      }
   }

   return f;
}

bool VAtmosphereSoundings::write_MODTRAN_UserProfile( unsigned int iIndexUserData, unsigned int defaultAtmosphericModel, bool iWriteDewPoint )
{
   if( iIndexUserData >= fDataUserProfile.size() )
   {
      cout << "VAtmosphereSoundings::write_MODTRAN_UserProfile: index out of range: " << iIndexUserData << "\t" << fDataUserProfile.size() << endl;
      return false;
   }
   if( !fDataUserProfile[iIndexUserData] )
   {
      cout << "VAtmosphereSoundings::write_MODTRAN_UserProfile: no data" << endl;
      return false;
   }

////////////////////////////////////////////////////////////
// CARD 2C:	ML, IRD1, IRD2, HMODEL
// FORMAT (315, A20)
   printf( "%5d", (int)fDataUserProfile[iIndexUserData]->fHeight_m.size() );
   printf( "%5d", 0 );
   printf( "%5d", 0 );
//   printf( "%65s\n", iProfileName.c_str() );
   cout << endl;

////////////////////////////////////////////////////////////
// CARD 2C1: ZM, P, T, (WMOL(J), J = 1, 3), (JCHAR(J), J = 1, 14), JCHARX
// FORMAT (F10.3, 5E10.3, 14A1, 1X, A1)

////////////////////////////////////////////////////////////
// pressure
   for( unsigned int i = 0; i < fDataUserProfile[iIndexUserData]->fHeight_m.size(); i++ )
   {
      printf( "%10.3f", fDataUserProfile[iIndexUserData]->fHeight_m[i] / 1.e3 );

      if( fDataUserProfile[iIndexUserData]->fPressure_Pa[i] > 0. )
      {
	 printf( "%10.3f",  fDataUserProfile[iIndexUserData]->fPressure_Pa[i]/1.e2 );  // Pa -> mbar
      }
      else                                          printf( "%10.3f", 1.e-3 );     // MODTRAN requires pressure > 0.

////////////////////////////////////////////////////////////
// temperature
      printf( "%10.3f", fDataUserProfile[iIndexUserData]->fTemperature_K[i] );

////////////////////////////////////////////////////////////
// dew point or relative humidity
      if( iWriteDewPoint )  printf( "%10.3f", fDataUserProfile[iIndexUserData]->fDewPoint_K[i] );
      else                  printf( "%10.3f", fDataUserProfile[iIndexUserData]->fRelativeHumidity[i] );   // relative humdity (in percent)

////////////////////////////////////////////////////////////
// remaining values need by MODTRAN
      printf( "%10.3f", 0. );
      printf( "%10.3f", 0. );
      if( iWriteDewPoint )  printf( "AAF" );   // pressure in [mbar]; temperature in [K]; dew point [K];
      else                   printf( "AAH" );   // pressure in [mbar]; temperature in [K]; relative humidity;
      for( unsigned int j = 0; j < 13; j++ ) cout << defaultAtmosphericModel;
      cout << endl;
   }

   return true;
}

/*
   this function is currently not used (but might be useful later)
*/
double VAtmosphereSoundings::getInterpolation( double h, VAtmosphereSoundingData* iData, string iType )
{
   double f = -9999;      // return value

   if( h < 0. || !iData ) return f;

   int iLup = -1;
   int iLlo = -1;
   for( unsigned int i = 0; i < iData->fHeight_m.size(); i++ )
   {
       if( h > iData->fHeight_m[i] )
       {
          iLup = (int)i;
	  if( i > 0 ) iLlo = (int)(i-1);
	  else        iLlo = 0;
	  break;
       }
   }
   if( iLlo < 0 || iLup < 0 ) return f;

   double yup = 0.;
   double ylo = 0.;

   if( iType == "pressure" )
   {
      if( iLup < (int)iData->fPressure_Pa.size() ) yup = iData->fPressure_Pa[iLup];
      if( iLlo < (int)iData->fPressure_Pa.size() ) ylo = iData->fPressure_Pa[iLlo];
   }
   else if( iType == "temperature" )
   {
      if( iLup < (int)iData->fTemperature_K.size() ) yup = iData->fTemperature_K[iLup];
      if( iLlo < (int)iData->fTemperature_K.size() ) ylo = iData->fTemperature_K[iLlo];
   }
   else if( iType == "dewpoint" )
   {
      if( iLup < (int)iData->fDewPoint_K.size() ) yup = iData->fDewPoint_K[iLup];
      if( iLlo < (int)iData->fDewPoint_K.size() ) ylo = iData->fDewPoint_K[iLlo];
   }
   else if( iType == "relhum" )
   {
      if( iLup < (int)iData->fRelativeHumidity.size() ) yup = iData->fRelativeHumidity[iLup];
      if( iLlo < (int)iData->fRelativeHumidity.size() ) ylo = iData->fRelativeHumidity[iLlo];
   }

// interpolate
   if( iLup > iLlo && TMath::Abs( iData->fHeight_m[iLlo]-iData->fHeight_m[iLup]) > 1.e-2 )
   {
	   f = yup + (h-iData->fHeight_m[iLup]) * (yup-ylo) / (iData->fHeight_m[iLlo]-iData->fHeight_m[iLup]);
   }
   else    f = yup;

   return f;
}

void VAtmosphereSoundings::list_datasets_CORSIKAMODTRAN()
{
   for( unsigned int i = 0; i < fDataCORSIKAMODTRAN.size(); i++ )
   {
      cout << i << "\t";
      if( fDataCORSIKAMODTRAN[i] )
      {
         cout << fDataCORSIKAMODTRAN[i]->Name << "\t";
	 cout << fDataCORSIKAMODTRAN[i]->fPressure_Pa.size() << "\t";
      }
      cout << endl;
   }
}

void VAtmosphereSoundings::list_datasets()
{
   for( unsigned int i = 0; i < fData.size(); i++ )
   {
      cout << i << "\t";
      if( fData[i] )
      {
         cout << fData[i]->Name << "\t";
	 cout << fData[i]->fPressure_Pa.size() << "\t";
      }
      cout << endl;
   }
}

bool VAtmosphereSoundings::write_CORSIKA_UserProfile( unsigned int iMODTRANIndex, unsigned int atmprofmodel, string iName )
{
   if( iMODTRANIndex >= fDataCORSIKAMODTRAN.size() )
   {
      cout << "VAtmosphereSoundings::write_CORSIKA_UserProfile: index out of range: " << iMODTRANIndex << "\t" << fDataCORSIKAMODTRAN.size() << endl;
      return false;
   }
   if( !fDataCORSIKAMODTRAN[iMODTRANIndex] )
   {
      cout << "VAtmosphereSoundings::write_CORSIKA_UserProfile: no data" << endl;
      return false;
   }

   cout << "# Atmospheric Model " << atmprofmodel << " (" << iName << ")" << endl;
   cout << "#Col. #1          #2           #3            #4" << endl;
   cout << "# Alt [km]    rho [g/cm^3] thick [g/cm^2]    n-1" << endl;

   for( unsigned int i = 0; i < fDataCORSIKAMODTRAN[iMODTRANIndex]->fHeight_m.size(); i++ )
   {
      printf( "%10.3f", fDataCORSIKAMODTRAN[iMODTRANIndex]->fHeight_m[i]/1.e3 );
      printf( "%15.5e", fDataCORSIKAMODTRAN[iMODTRANIndex]->fDensity_gcm3[i] );
      printf( "%13.5e", fDataCORSIKAMODTRAN[iMODTRANIndex]->fThickness_gcm2[i] );
      printf( "%13.5e", fDataCORSIKAMODTRAN[iMODTRANIndex]->fIndexofRefraction[i]-1. );
      cout << endl;
   }
   return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VAtmosphereSoundingData::VAtmosphereSoundingData()
{
   MJD = 0;
   Year = 0;
   Month = 0;
   Day = 0;
   Hour = 0.;
   Name = "";

   PlotColor = 1;
   PlotMarker = 20;
   PlotLineStyle = 1;
   PlotLineWidth = 2;
}

void VAtmosphereSoundingData::setdefaultvalues( unsigned int iN )
{
   fPressure_Pa.clear();
   fHeight_m.clear();
   fDensity_gcm3.clear();
   fThickness_gcm2.clear();
   fTemperature_K.clear();
   fDewPoint_K.clear();
   fRelativeHumidity.clear();
   fVaporMassDensity_gm3.clear();
   fMixingRatio_gkg.clear();
   fWindDirection_deg.clear();
   fWindSpeed_ms.clear();
   fIndexofRefraction.clear();
   fO2_cmkm.clear();
   fO3_cmkm.clear();

// set default values
   for( unsigned int i = 0; i < iN; i++ )
   {
       fPressure_Pa.push_back( -9999. );
       fHeight_m.push_back( -9999. );
       fDensity_gcm3.push_back( -9999. );
       fThickness_gcm2.push_back( -9999. );
       fTemperature_K.push_back( -9999. );
       fDewPoint_K.push_back( -9999. );
       fRelativeHumidity.push_back( -9999. );
       fVaporMassDensity_gm3.push_back( -9999. );
       fMixingRatio_gkg.push_back( -9999. );
       fWindDirection_deg.push_back( -9999. );
       fWindSpeed_ms.push_back( -9999. );
       fIndexofRefraction.push_back( -9999. );
       fO2_cmkm.push_back( -9999. );
       fO3_cmkm.push_back( -9999. );
   }
}
