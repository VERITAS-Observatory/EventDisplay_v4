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

#include "VAtmosphereSoundingData.h"


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

   fGraphScaledDensityHeight=0;
   fGraphPressureHeight=0;
   fGraphHumidityHeight=0;
   fGraphTemperatureHeight=0;
   fGraphIndexHeight=0;
   fGraphThicknessHeight=0;
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

	if( fGraphScaledDensityHeight!=0) fGraphScaledDensityHeight->Delete();

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




void VAtmosphereSoundingData::makeGraphScaledDensity(  ) {
	if(fGraphScaledDensityHeight != 0 ) fGraphScaledDensityHeight->Delete();

	fGraphScaledDensityHeight = new TGraph(0);
	for(unsigned int i=0; i<fHeight_m.size(); i++) {
		if(fDensity_gcm3[i] > 0) {
			fGraphScaledDensityHeight->SetPoint(fGraphScaledDensityHeight->GetN(), fHeight_m[i]/1000.0, fDensity_gcm3[i] * TMath::Exp( fHeight_m[i]/7739.0) );
		}
	}
	
	fGraphScaledDensityHeight->SetMarkerStyle(PlotMarker);
	fGraphScaledDensityHeight->SetMarkerColor(PlotColor);
	fGraphScaledDensityHeight->SetLineColor(PlotColor);
	fGraphScaledDensityHeight->SetLineStyle(PlotLineStyle);
	fGraphScaledDensityHeight->SetLineWidth(PlotLineWidth);	
	fGraphScaledDensityHeight->SetTitle("Scaled density;height [km]; density*exp(h/7.739km) [g/cm^{3}]");
	TString temp="density_" + Name;
	fGraphScaledDensityHeight->SetName(temp.Data());
}


void VAtmosphereSoundingData::makeGraphPressure(  ) {
	if(fGraphPressureHeight != 0 ) fGraphPressureHeight->Delete();

	fGraphPressureHeight = new TGraph(0);
	for(unsigned int i=0; i<fHeight_m.size(); i++) {
		if(fPressure_Pa[i] > 0) {
			fGraphPressureHeight->SetPoint(fGraphPressureHeight->GetN(), fHeight_m[i]/1000.0, fPressure_Pa[i] );
		}
	}
	
	fGraphPressureHeight->SetMarkerStyle(PlotMarker);
	fGraphPressureHeight->SetMarkerColor(PlotColor);
	fGraphPressureHeight->SetLineColor(PlotColor);
	fGraphPressureHeight->SetLineStyle(PlotLineStyle);
	fGraphPressureHeight->SetLineWidth(PlotLineWidth);
	fGraphPressureHeight->SetTitle("Pressure;height [km]; pressure [Pa]");
	TString temp="pressure_" + Name;
	fGraphPressureHeight->SetName(temp.Data() );	
}


void VAtmosphereSoundingData::makeGraphTemperature(  ) {
	if(fGraphTemperatureHeight) fGraphTemperatureHeight->Delete();

	fGraphTemperatureHeight = new TGraph(0);
	for(unsigned int i=0; i<fHeight_m.size(); i++) {
		if(fTemperature_K[i] > 0) {
			fGraphTemperatureHeight->SetPoint(fGraphTemperatureHeight->GetN(), fHeight_m[i]/1000.0, fTemperature_K[i] );
		}
	}
	
	fGraphTemperatureHeight->SetMarkerStyle(PlotMarker);
	fGraphTemperatureHeight->SetMarkerColor(PlotColor);
	fGraphTemperatureHeight->SetLineColor(PlotColor);
	fGraphTemperatureHeight->SetLineStyle(PlotLineStyle);
	fGraphTemperatureHeight->SetLineWidth(PlotLineWidth);
	fGraphTemperatureHeight->SetTitle("Temperature;height [km]; temperature [K]");
	TString temp="temperature_" + Name;
	fGraphTemperatureHeight->SetName(temp.Data() );
}


void VAtmosphereSoundingData::makeGraphHumidity(  ) {
	if(fGraphHumidityHeight != 0 ) fGraphHumidityHeight->Delete();

	fGraphHumidityHeight = new TGraph(0);
	for(unsigned int i=0; i<fHeight_m.size(); i++) {
		if(fRelativeHumidity[i] > 0) {
			fGraphHumidityHeight->SetPoint(fGraphHumidityHeight->GetN(), fHeight_m[i]/1000.0, fRelativeHumidity[i] );
		}
	}
	
	fGraphHumidityHeight->SetMarkerStyle(PlotMarker);
	fGraphHumidityHeight->SetMarkerColor(PlotColor);
	fGraphHumidityHeight->SetLineColor(PlotColor);
	fGraphHumidityHeight->SetLineStyle(PlotLineStyle);
	fGraphHumidityHeight->SetLineWidth(PlotLineWidth);
	fGraphHumidityHeight->SetTitle("Humidity;height [km]; rel. humidity [\%]");
	TString temp="relHum_" + Name;
	fGraphHumidityHeight->SetName(temp.Data() );
}

void VAtmosphereSoundingData::makeGraphThickness(  ) {
	if(fGraphThicknessHeight != 0 ) fGraphThicknessHeight->Delete();

	fGraphThicknessHeight = new TGraph(0);
	for(unsigned int i=0; i<fHeight_m.size(); i++) {
		if(fThickness_gcm2[i] > 0) {
			fGraphThicknessHeight->SetPoint(fGraphThicknessHeight->GetN(), fHeight_m[i]/1000.0, fThickness_gcm2[i] );
		}
	}
	
	fGraphThicknessHeight->SetMarkerStyle(PlotMarker);
	fGraphThicknessHeight->SetMarkerColor(PlotColor);
	fGraphThicknessHeight->SetLineColor(PlotColor);
	fGraphThicknessHeight->SetLineStyle(PlotLineStyle);
	fGraphThicknessHeight->SetLineWidth(PlotLineWidth);
	fGraphThicknessHeight->SetTitle("Thickness;height [km]; thickness [g/cm^{2}]");	
	TString temp="thickness_" + Name;
	fGraphThicknessHeight->SetName(temp.Data() );
}

void VAtmosphereSoundingData::makeGraphIndex(  ) {
	if(fGraphIndexHeight != 0 ) fGraphIndexHeight->Delete();

	fGraphIndexHeight = new TGraph(0);
	for(unsigned int i=0; i<fHeight_m.size(); i++) {
		if(fIndexofRefraction[i] > 0) {
			fGraphIndexHeight->SetPoint(fGraphIndexHeight->GetN(), fHeight_m[i]/1000.0, fIndexofRefraction[i]-1 );
		}
	}
	
	fGraphIndexHeight->SetMarkerStyle(PlotMarker);
	fGraphIndexHeight->SetMarkerColor(PlotColor);
	fGraphIndexHeight->SetLineColor(PlotColor);
	fGraphIndexHeight->SetLineStyle(PlotLineStyle);
	fGraphIndexHeight->SetLineWidth(PlotLineWidth);
	fGraphIndexHeight->SetTitle("Index of Refraction;height [km]; index of refraction -1 ]");
	TString temp="n-1_" + Name;
	fGraphIndexHeight->SetName(temp.Data() );
}

void VAtmosphereSoundingData::setColor( int color ) {
	PlotColor=color;

	if( fGraphIndexHeight ) { fGraphIndexHeight->SetLineColor(PlotColor); }
	if( fGraphThicknessHeight ) { fGraphThicknessHeight->SetLineColor(PlotColor); }
	if( fGraphHumidityHeight ) { fGraphHumidityHeight->SetLineColor(PlotColor); }
	if( fGraphTemperatureHeight ) { fGraphTemperatureHeight->SetLineColor(PlotColor); }
	if( fGraphPressureHeight ) { fGraphPressureHeight->SetLineColor(PlotColor); }
	if( fGraphScaledDensityHeight ) { fGraphScaledDensityHeight->SetLineColor(PlotColor); }
}
