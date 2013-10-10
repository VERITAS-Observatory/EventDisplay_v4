//! VAtmosphereSoundings read and analyse sounding data
// Revision $Id: VAtmosphereSoundings.h,v 1.1.2.7 2010/09/20 10:49:28 gmaier Exp $

#ifndef VAtmosphereSoundingData_H
#define VAtmosphereSoundingData_H

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
  
    void makeGraphScaledDensity(  );
    void makeGraphPressure(  );
    void makeGraphHumidity(  );
    void makeGraphTemperature(  );
    void makeGraphIndex(  );
    void makeGraphThickness(  );

    TGraph * getDensityGraph()		{ if(!fGraphScaledDensityHeight)	makeGraphScaledDensity();	return fGraphScaledDensityHeight;	}
    TGraph * getPressureGraph()		{ if(!fGraphPressureHeight)		makeGraphPressure();		return fGraphPressureHeight;		}
    TGraph * getHumidityGraph()		{ if(!fGraphHumidityHeight)		makeGraphHumidity();		return fGraphHumidityHeight;		}
    TGraph * getTemperatureGraph()	{ if(!fGraphTemperatureHeight)		makeGraphTemperature();		return fGraphTemperatureHeight;		}
    TGraph * getIndexGraph()		{ if(!fGraphIndexHeight)		makeGraphIndex();		return fGraphIndexHeight;		}
    TGraph * getThicknessGraph()	{ if(!fGraphThicknessHeight)		makeGraphThickness();		return fGraphThicknessHeight;		}

    TGraph * fGraphScaledDensityHeight;
    TGraph * fGraphPressureHeight;
    TGraph * fGraphHumidityHeight;
    TGraph * fGraphTemperatureHeight;
    TGraph * fGraphIndexHeight;
    TGraph * fGraphThicknessHeight;

};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////



#endif
