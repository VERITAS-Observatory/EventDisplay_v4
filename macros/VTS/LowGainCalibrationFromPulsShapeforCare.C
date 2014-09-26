/*  VLowGainCalibrationFromPulsShape
    macro to calculate high/low gain multiplier from pulse shapes for high and low gain

    input:
    pulse shape in ascii format

    output:
    LOWGAINMULTIPLIER_SUM line as required in calibrationlist.LowGain.dat

*/


#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "TCanvas.h"
#include "TGraph.h"

class VLowGainCalibrationFromPulsShape
{
	private:
	
		double fFADCSampling;                         // sampling time in [ns]
		
		TGraph* fHighGainPulse;
		TGraph* fLowGainPulse;
		TGraph* fLGRatio_window;
		
		double fWindowStartRelT0_sample;
		double fHighGainWindowLength_sample;
		double fLowGainWindowStart_sample;
		double fLowGainWindowStop_sample;
		
		
		double  integrate( TGraph* g, double iWindowStartRelT0_sample, double iWindowLength_sample );
		TGraph* readPulseShape( string iFile, bool iGrisuFormat, double iSampling );
		
		
	public:
		VLowGainCalibrationFromPulsShape();
		~VLowGainCalibrationFromPulsShape() {}
		void calculateChargeFraction();
		bool readPulseShapes( string iHighGainPulse, string iLowGainPulse, bool iGrisuFormat = false, double iSampling = 2.0 );
		void plot();
		void setTraceIntegrationParameters( double iStartRelT0_sample, int iHighGainWindowLength_sample, int iLowGainWindowStart_sample, int iLowGainWindowStop_sample );
		void test();
};

/////////////////////////////

VLowGainCalibrationFromPulsShape::VLowGainCalibrationFromPulsShape()
{
	fFADCSampling = 2.;
	fHighGainPulse = 0;
	fLowGainPulse = 0;
	fLGRatio_window = 0;
	
	fWindowStartRelT0_sample = 1;
	fHighGainWindowLength_sample = 10;
	fLowGainWindowStart_sample = 4;
	fLowGainWindowStop_sample = 18;
}

void VLowGainCalibrationFromPulsShape::test()
{
	readPulseShapes( "VERITASHighGainPulseShapeUpgradePMTFromFADC.txt", "VERITASLowGainPulseShapesUpgradePMTFromFADC.txt" );
	calculateChargeFraction();
	plot();
}

void VLowGainCalibrationFromPulsShape::setTraceIntegrationParameters( double iStartRelT0_sample, int iHighGainWindowLength_sample, int iLowGainWindowStart_sample, int iLowGainWindowStop_sample )
{
	fWindowStartRelT0_sample = iStartRelT0_sample;
	fHighGainWindowLength_sample = iHighGainWindowLength_sample;
	fLowGainWindowStart_sample = iLowGainWindowStart_sample;
	fLowGainWindowStop_sample = iLowGainWindowStop_sample;
}

TGraph* VLowGainCalibrationFromPulsShape::readPulseShape( string iFile, bool iGrisuFormat, double iSampling )
{
	ifstream is;
	is.open( iFile.c_str(), ifstream::in );
	if( !is )
	{
		return 0;
	}
	
	TGraph* g = new TGraph( 1 );
	
	string iTemp = "";
	string is_line = "";
	double x = 0.;
	double y = 0.;
	int z = 0;
	double iSampleOffset = 0.;
	while( getline( is, is_line ) )
	{
		if( is_line.find( "*" ) != string::npos )
		{
			break;
		}
		istringstream is_stream( is_line );
		if( !iGrisuFormat )
		{
			if( !is_stream.eof() )
			{
				is_stream >> x;
			}
		}
		else
		{
			x = iSampleOffset;
		}
		if( !is_stream.eof() )
		{
			is_stream >> y;
		}
		if( x > 1000. )
		{
			continue;
		}
		g->SetPoint( z, x, y );
		z++;
		iSampleOffset += iSampling;
	}
	return g;
}

bool VLowGainCalibrationFromPulsShape::readPulseShapes( string iHighGainPulse, string iLowGainPulse, bool iGrisuFormat, double iSampling )
{
	fHighGainPulse = readPulseShape( iHighGainPulse, iGrisuFormat, iSampling );
	if( !fHighGainPulse || fHighGainPulse->GetN() < 1 )
	{
		cout << "error reading high gain pulse shape" << endl;
		return false;
	}
	fHighGainPulse->SetTitle( "" );
	fLowGainPulse = readPulseShape( iLowGainPulse, iGrisuFormat, iSampling );
	if( !fLowGainPulse || fLowGainPulse->GetN() < 1 )
	{
		cout << "error reading low gain pulse shape" << endl;
		return false;
	}
	fLowGainPulse->SetLineColor( 2 );
	fLowGainPulse->SetTitle( "" );
	
	return true;
}

void VLowGainCalibrationFromPulsShape::plot()
{
	TCanvas* c = new TCanvas( "cP", "pulse shapes" );
	c->Draw();
	
	if( !fHighGainPulse )
	{
		return;
	}
	
	fHighGainPulse->Draw( "al" );
	fLowGainPulse->Draw( "l" );
	
	if( fLGRatio_window && fLGRatio_window->GetN() > 0 )
	{
		TCanvas* d = new TCanvas( "cLGRatio", "LG ratio" );
		d->Draw();
		
		fLGRatio_window->Draw( "al" );
	}
}

void VLowGainCalibrationFromPulsShape::calculateChargeFraction()
{
	cout << "Sample length: " << fFADCSampling << endl;
	cout << "Window start relative T0: " << fWindowStartRelT0_sample << endl;
	cout << "High gain window length: " << fHighGainWindowLength_sample << endl;
	cout << "Low gain window length: " << fLowGainWindowStart_sample << " to " << fLowGainWindowStop_sample << endl;
	cout << endl;
	if( !fHighGainPulse || !fLowGainPulse )
	{
		return;
	}
	
	// from CARE simulations
	// CARE uses a hi/lo gain ratio on the amplitudes that describes the characteristics of the system where the lo gain is linear.
	// * FADCLOHIGHGAINRATIO 0 0.099
	double i_LG_norm = i_LG_norm = 0.099;
	cout << "Normalization: " << i_LG_norm << endl;
	
	double iHighGainIntegral = integrate( fHighGainPulse, fWindowStartRelT0_sample, fHighGainWindowLength_sample );
	fLGRatio_window = new TGraph( 1 );
	int z = 0;
	vector< double > iLGRatio_V;
	for( int i = fLowGainWindowStart_sample; i <= fLowGainWindowStop_sample; i++ )
	{
		double iLowGainIntegral = integrate( fLowGainPulse, fWindowStartRelT0_sample, i )  * i_LG_norm;
		cout << "**** low/high gain ratio ***" << endl;
		cout << "Window length " << i << ": " << iHighGainIntegral << "\t" << iLowGainIntegral;
		if( iLowGainIntegral > 0. && iHighGainIntegral > 0. )
		{
			cout << " ratio: " << iHighGainIntegral / iLowGainIntegral;
			cout << " LG : " << iHighGainIntegral / iLowGainIntegral;
			fLGRatio_window->SetPoint( z, i, iHighGainIntegral / iLowGainIntegral );
			iLGRatio_V.push_back( iHighGainIntegral / iLowGainIntegral );
			z++;
		}
		cout << endl;
		cout << "*******" << endl;
	}
	
	for( unsigned int i = 0; i < iLGRatio_V.size(); i++ )
	{
		cout << "    " << iLGRatio_V[i];
	}
	cout << endl;
	
}

double VLowGainCalibrationFromPulsShape::integrate( TGraph* g, double iWindowStartRelT0_sample, double iWindowLength_sample )
{
	if( !g )
	{
		return -999.;
	}
	
	double x = 0.;
	double y = 0.;
	double ymax_half = 0.;
	for( int i = 0; i < g->GetN(); i++ )
	{
		g->GetPoint( i, x, y );
		if( y < ymax_half )
		{
			ymax_half = y;
		}
	}
	ymax_half *= 0.5;
	double x_t0 = 0.;
	double x_start = 0.;
	double x_stop = 0.;
	
	for( int i = 0; i < g->GetN(); i++ )
	{
		g->GetPoint( i, x, y );
		if( y < ymax_half )
		{
			x_t0 = x;
			break;
		}
	}
	x_start = x_t0 - iWindowStartRelT0_sample * fFADCSampling;
	x_stop  = x_start + iWindowLength_sample * fFADCSampling;
	
	double i_sum = 0.;
	for( int i = 0; i < iWindowLength_sample; i++ )
	{
		i_sum += TMath::Abs( g->Eval( x_start + i * fFADCSampling ) );
	}
	cout << "TO, xstart, stop, window length, sum " << x_t0 << "\t" << x_start << "\t" << x_stop << "\t" << iWindowLength_sample << "\t" << i_sum << endl;
	
	return i_sum;
}
