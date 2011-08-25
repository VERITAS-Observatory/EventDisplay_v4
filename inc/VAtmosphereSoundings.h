//! VAtmosphereSoundings read and analyse sounding data
// Revision $Id: VAtmosphereSoundings.h,v 1.1.2.7 2010/09/20 10:49:28 gmaier Exp $

#ifndef VAtmosphereSoundings_H
#define VAtmosphereSoundings_H

#include "VASlalib.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH2D.h"
#include "TLegend.h"
#include "TMath.h"
#include "TProfile.h"
#include "TTree.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#define VMAXNUMBEROFSOUNDINGPOINTS 1000

using namespace std;

class VAtmosphereSoundingData
{
   public:

    double MJD;
    int Year;
    int Month;
    int Day;
    double Hour;
    string Name;
    vector< double > fPressure_Pa;
    vector< double > fHeight_m;
    vector< double > fDensity_gcm3;
    vector< double > fThickness_gcm2;
    vector< double > fTemperature_K;
    vector< double > fDewPoint_K;
    vector< double > fRelativeHumidity;
    vector< double > fVaporMassDensity_gm3;
    vector< double > fMixingRatio_gkg;
    vector< double > fWindDirection_deg;
    vector< double > fWindSpeed_ms;
    vector< double > fIndexofRefraction;
    vector< double > fO2_cmkm;
    vector< double > fO3_cmkm;
    int PlotColor;
    int PlotMarker;
    int PlotLineStyle;
    Width_t PlotLineWidth;

    VAtmosphereSoundingData();
   ~VAtmosphereSoundingData() {}
    void setdefaultvalues( unsigned int iN );
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

class VAtmosphereSoundings
{
   private:
     
    bool             bDebug;

    vector< string > fListTextFile;

// tree writing and reading
    TFile *fDataFile;
    TTree *fDataTree;

     double MJD;
     int Year;
     int Month;
     int Day;
     double Hour;
     unsigned int nPoints;
     double Pressure_Pa[VMAXNUMBEROFSOUNDINGPOINTS];
     double Height_m[VMAXNUMBEROFSOUNDINGPOINTS];
     double Density_gcm3[VMAXNUMBEROFSOUNDINGPOINTS];
     double Thickness_gcm2[VMAXNUMBEROFSOUNDINGPOINTS];
     double Temperature_K[VMAXNUMBEROFSOUNDINGPOINTS];
     double DewPoint_K[VMAXNUMBEROFSOUNDINGPOINTS];
     double RelativeHumidity[VMAXNUMBEROFSOUNDINGPOINTS];
     double VaporMassDensity_gm3[VMAXNUMBEROFSOUNDINGPOINTS];
     double MixingRatio_gkg[VMAXNUMBEROFSOUNDINGPOINTS];
     double WindDirection_deg[VMAXNUMBEROFSOUNDINGPOINTS];
     double WindSpeed_ms[VMAXNUMBEROFSOUNDINGPOINTS];

    string           fTXTSearch_DataString;

// data containers
    vector< VAtmosphereSoundingData* > fData;                         // profile from soundings data
    vector< VAtmosphereSoundingData* > fDataCORSIKAMODTRAN;           // CORSIKA/MODTRAN profile
    vector< VAtmosphereSoundingData* > fDataUserProfile;              // user created profile

// periods to calculate average over
    map< unsigned int, string > fPlottingPeriodFiles;
    map< unsigned int, vector< unsigned int > > fPlottingPeriodDates;

// plotting
    bool   fPlotRelativePlots;
    bool   fBoolColorChange;
    vector< TLegend* > fPlottingLegend;
    bool   fPlottingLegendDraw;
    vector< TCanvas* > fCanvasProfile;
    vector< TCanvas* > fCanvas2D;
    string fPlottingPeriod;
    double fPlottingHeight_min;
    double fPlottingHeight_max;

// observatory
    double fObservatoryLatitude;
    double fObservatoryHeight_km;

    unsigned int checkPlottingPeriodIdentifier( unsigned int );
    void   fillAtmosphericDensity();
    void   fillAtmosphericDensity( VAtmosphereSoundingData* );
    void   fillAtmosphericThickness();
    void   fillAtmosphericThickness( VAtmosphereSoundingData* );
    void   fillAtmosphericPressure();
    void   fillAtmosphericPressure( VAtmosphereSoundingData* );
    void   fillIndexofRefraction();
    void   fillO2();
    void   fillO3();
    void   fillWaterVaporDensity();
    void   fillWaterVaporDensity( VAtmosphereSoundingData* );
    double getInterpolation( double h, VAtmosphereSoundingData* iData, string iType );
    int    getMonth( string );
    unsigned int getHistogramIdentifier( unsigned int );
    vector< double > getDataVectorForUserAtmosphere( double iHeightMaxData, VAtmosphereSoundingData* iDataMonteCarlo, string iType );
    double getWaterVaporDensity( double T, double RH );
    double getWaterVaporMassDensity( double ATEMP );
    TCanvas* plotCORSIKA( TCanvas *c, int iPlotID, vector< VAtmosphereSoundingData* > iData, double iHeightMin = 0., double iHeightMax = 120. );
    void   plotProfiles( unsigned int iYearStart, unsigned int iMonthStart, unsigned int iYearStop, unsigned int iMonthStop, bool b2D = false, string iPlotOption = "", bool bSames = false );
    bool   readPlottingPeriodsFromTextFile( string );
    bool   readRootFile();

   public:

    VAtmosphereSoundings();
    VAtmosphereSoundings( string iRootFile );
   ~VAtmosphereSoundings() {}
    bool     add_CORSIKA_Atmosphere( string iFile, string iName = "", int iColor = 2, int iLineStyle = 1 );
    bool     add_MODTRAN_Atmosphere( string iFile, string iName = "", int iColor = 2, int iLineStyle = 1 );
    bool     add_user_Atmosphere( unsigned int iIndexCORSIKAMODTRAN, double iHeightMaxData, string iName = "" );
    bool     readSoundingsFromTextFile( string iFileList );
    double   getAmosphericVaporPressure( double T );
    double   getDewPoint( double temperature, double relativeHumidity, int iMethod = 0 );
    void     list_datasets();
    void     list_datasets_CORSIKAMODTRAN();

    TCanvas* plotCORSIKA_Density_vs_Heigth( TCanvas *c = 0, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 0, fDataCORSIKAMODTRAN, iHeightMin, iHeightMax ); }
    TCanvas* plotCORSIKA_DewPoint_vs_Heigth( TCanvas *c = 0, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 5, fDataCORSIKAMODTRAN, iHeightMin, iHeightMax ); }
    TCanvas* plotCORSIKA_IndexofRefraction_vs_Heigth( TCanvas *c = 0, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 1, fDataCORSIKAMODTRAN, iHeightMin, iHeightMax ); }
    TCanvas* plotCORSIKA_Ozone_vs_Heigth( TCanvas *c = 0, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 3, fDataCORSIKAMODTRAN, iHeightMin, iHeightMax ); } 
    TCanvas* plotCORSIKA_Pressure_vs_Heigth( TCanvas *c = 0, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 6, fDataCORSIKAMODTRAN, iHeightMin, iHeightMax ); }
    TCanvas* plotCORSIKA_RelativeHumidity_vs_Heigth( TCanvas *c, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 4, fDataCORSIKAMODTRAN, iHeightMin, iHeightMax ); }
    TCanvas* plotCORSIKA_Temperature_vs_Heigth( TCanvas *c = 0, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 2, fDataCORSIKAMODTRAN, iHeightMin, iHeightMax ); }
    TCanvas* plotCORSIKA_Thickness_vs_Heigth( TCanvas *c = 0, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 7, fDataCORSIKAMODTRAN, iHeightMin, iHeightMax ); }

    TCanvas* plotUserAtmosphere_Density_vs_Heigth( TCanvas *c = 0, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 0, fDataUserProfile, iHeightMin, iHeightMax ); }
    TCanvas* plotUserAtmosphere_DewPoint_vs_Heigth( TCanvas *c = 0, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 5, fDataUserProfile, iHeightMin, iHeightMax ); }
    TCanvas* plotUserAtmosphere_IndexofRefraction_vs_Heigth( TCanvas *c = 0, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 1, fDataUserProfile, iHeightMin, iHeightMax ); }
    TCanvas* plotUserAtmosphere_Ozone_vs_Heigth( TCanvas *c = 0, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 3, fDataUserProfile, iHeightMin, iHeightMax ); }
    TCanvas* plotUserAtmosphere_Pressure_vs_Heigth( TCanvas *c = 0, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 6, fDataUserProfile, iHeightMin, iHeightMax ); }
    TCanvas* plotUserAtmosphere_RelativeHumidity_vs_Heigth( TCanvas *c, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 4, fDataUserProfile, iHeightMin, iHeightMax ); }
    TCanvas* plotUserAtmosphere_Temperature_vs_Heigth( TCanvas *c = 0, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 2, fDataUserProfile, iHeightMin, iHeightMax ); }
    TCanvas* plotUserAtmosphere_Thickness_vs_Heigth( TCanvas *c = 0, double iHeightMin = 0., double iHeightMax = 120. ) { return plotCORSIKA( c, 7, fDataUserProfile, iHeightMin, iHeightMax ); }

    void     plot2DProfiles( unsigned int iYearStart = 1980, unsigned int iMonthStart = 1, unsigned int iYearStop = 2020, unsigned int iMonthStop = 12 );
    void     plotAverages( unsigned int iYearStart = 1980, unsigned int iMonthStart = 1, unsigned int iYearStop = 2020, unsigned int iMonthStop = 12, string iPlotOption = "", bool iSames = false );
    void     plotAttributes_ColorChange( bool iB = true ) { fBoolColorChange = iB; }
    void     plotAttributes_PlotLegend( bool iB = true ) { fPlottingLegendDraw = iB; }
    void     setGeographicPosition( double iLatitude = 31.675, double iObsHeight_km = 1.27 ) { fObservatoryLatitude = iLatitude; fObservatoryHeight_km = iObsHeight_km; }
    void     setPlottingPeriod( string iPeriod = "monthly" );
    void     setPlottingRangeHeight( double iHeightMin = 0., double iHeightMax = 30. ) { fPlottingHeight_min = iHeightMin; fPlottingHeight_max = iHeightMax; } // in [km]
    void     setPlottingRelativePlots( bool iB = false ) { fPlotRelativePlots = iB; }
    bool     write_CORSIKA_UserProfile( unsigned int iMODTRANIndex, unsigned int atmprofmodel, string iName = "user profile" );
    bool     write_MODTRAN_UserProfile( unsigned int iIndexUserData, unsigned int defaultModel = 6, bool iWriteDewPoint = false );
    bool     writeRootFile( string iFile );
};

#endif
