/* \class VDispAnalyzer


     calculate displacement as function of width, length, size, ...

     call different methods to get disp (lookup tables, neural network, boosted decision trees)

*/

#include "VDispAnalyzer.h"

VDispAnalyzer::VDispAnalyzer()
{
	fMLPAnalyzer = 0;
	fDispTableAnalyzer = 0;
	fTMVADispAnalyzer = 0;
	
	fDispMethod = "NOMETHODDEFINED";
	
	f_disp = -999.;
	f_dispE = -999.;
}

void VDispAnalyzer::setTelescopeTypeList( vector<ULong64_t> iTelescopeTypeList )
{
	fTelescopeTypeList = iTelescopeTypeList;
}

bool VDispAnalyzer::initialize( string iFile, string iDispMethod )
{
	fDispMethod = iDispMethod;
	
	if( fDispMethod == "MLP" )
	{
		fMLPAnalyzer = new VMLPAnalyzer( iFile );
		if( fMLPAnalyzer->isZombie() )
		{
			setZombie( true );
		}
		else
		{
			setZombie( false );
		}
	}
	else if( fDispMethod == "DISPTABLES" )
	{
		fDispTableAnalyzer = new VDispTableAnalyzer( iFile );
		if( fDispTableAnalyzer->isZombie() )
		{
			setZombie( true );
		}
		else
		{
			setZombie( false );
		}
	}
	else if( fDispMethod == "TMVABDT" )
	{
		fTMVADispAnalyzer = new VTMVADispAnalyzer( iFile, fTelescopeTypeList );
		if( fTMVADispAnalyzer->isZombie() )
		{
			setZombie( true );
		}
		else
		{
			setZombie( false );
		}
	}
	else
	{
		cout << "VDispAnalyzer::initialize ERROR: unknown disp method: " << fDispMethod << endl;
		return false;
	}
	
	if( isZombie() )
	{
		cout << "VDispAnalyzer::initialize ERROR initializing method " << fDispMethod << endl;
		cout << "\t no input file: " << iFile << endl;
		return false;
	}
	
	return true;
}

void VDispAnalyzer::terminate()
{
	if( fMLPAnalyzer )
	{
		fMLPAnalyzer->terminate();
	}
	
	if( fDispTableAnalyzer )
	{
		fDispTableAnalyzer->terminate();
	}
	
	if( fTMVADispAnalyzer )
	{
		fTMVADispAnalyzer->terminate();
	}
}

float VDispAnalyzer::evaluate( float iWidth, float iLength, float iAsymm, float iDist, float iSize,
							   float iPedvar, float iTGrad, float iLoss, float icen_x, float icen_y,
							   float xoff_4, float yoff_4, ULong64_t iTelType, float iZe, float iAz, bool b2D )
{
	f_disp = -99.;
	
	if( fMLPAnalyzer )
	{
		f_disp = fMLPAnalyzer->evaluate( iWidth, iLength, iAsymm, iSize, iDist );
	}
	else if( fDispTableAnalyzer )
	{
		f_disp = fDispTableAnalyzer->evaluate( iWidth, iLength, iSize, iPedvar, iZe, iAz, b2D );
	}
	else if( fTMVADispAnalyzer )
	{
		f_disp = fTMVADispAnalyzer->evaluate( iWidth, iLength, iSize, iAsymm, iLoss, iTGrad, icen_x, icen_y, xoff_4, yoff_4, iTelType, iZe, iAz );
	}
	return f_disp;
	
}

void VDispAnalyzer::calculateMeanDirection( float& xs, float& ys, vector< float > x, vector< float > y, vector< float > cosphi, vector< float > sinphi, vector< float > v_disp, vector< float > v_weight, float& dispdiff )
{
	if( fDispTableAnalyzer )
	{
	  fDispTableAnalyzer->calculateMeanDirection( xs, ys, x, y, cosphi, sinphi, v_disp, v_weight );
	}
	else
	{
	
		float itotweight = 0.;
		float x1 = 0.;
		float x2 = 0.;
		float x3 = 0.;
		float x4 = 0.;
		float x5 = 0.;
		float x6 = 0.;
		float y1 = 0.;
		float y2 = 0.;
		float y3 = 0.;
		float y4 = 0.;
		float y5 = 0.;
		float y6 = 0.;
		float xa = 0.;
		float xb = 0.;
		float xc = 0.;
		float xd = 0.;
		float ya = 0.;
		float yb = 0.;
		float yc = 0.;
		float yd = 0.;
		float ixs = 0.;
		float iys = 0.;
		
		for( unsigned int ii = 0; ii < v_weight.size() ; ii++ )
		{
			itotweight += v_weight[ii] * v_weight[ii];
			
			if( ii == 0 )
			
			{
				x1 = x[ii] - v_disp[ii] * cosphi[ii];
				x2 = x[ii] + v_disp[ii] * cosphi[ii];
				
				y1 = y[ii] - v_disp[ii] * sinphi[ii];
				y2 = y[ii] + v_disp[ii] * sinphi[ii];
			}
			
			if( ii == 1 )
			{
			
				x3 = x[ii] - v_disp[ii] * cosphi[ii];
				x4 = x[ii] + v_disp[ii] * cosphi[ii];
				
				y3 = y[ii] - v_disp[ii] * sinphi[ii];
				y4 = y[ii] + v_disp[ii] * sinphi[ii];
				
				if( ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) && ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) && ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) )
				{
					xa = x1;
					xb = x3;
					ya = y1;
					yb = y3;
				}
				else if( ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) && ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) && ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) )
				{
					xa = x1;
					xb = x4;
					ya = y1;
					yb = y4;
				}
				else if( ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) && ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) && ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) )
				{
					xa = x2;
					xb = x3;
					ya = y2;
					yb = y3;
				}
				else if( ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) && ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) && ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) )
				{
					xa = x2;
					xb = x4;
					ya = y2;
					yb = y4;
				}
				
				ixs = ( xa * v_weight[0] * v_weight[0] + xb * v_weight[1] * v_weight[1] ) / ( v_weight[0] * v_weight[0] + v_weight[1] * v_weight[1] );
				iys = ( ya * v_weight[0] * v_weight[0] + yb * v_weight[1] * v_weight[1] ) / ( v_weight[0] * v_weight[0] + v_weight[1] * v_weight[1] );
				
			}
			
			if( ii == 2 )
			{
			
				x5 = x[ii] - v_disp[ii] * cosphi[ii];
				x6 = x[ii] + v_disp[ii] * cosphi[ii];
				
				y5 = y[ii] - v_disp[ii] * sinphi[ii];
				y6 = y[ii] + v_disp[ii] * sinphi[ii];
				
				if( ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) && ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) && ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) && ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) && ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) && ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) && ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) )
				{
					xa = x1;
					xb = x3;
					xc = x5;
					ya = y1;
					yb = y3;
					yc = y5;
				}
				else if( ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) && ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) && ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) && ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) && ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) && ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) && ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) )
				{
					xa = x2;
					xb = x3;
					xc = x5;
					ya = y2;
					yb = y3;
					yc = y5;
				}
				else if( ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) && ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) && ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) && ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) && ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) && ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) && ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) )
				{
					xa = x1;
					xb = x4;
					xc = x5;
					ya = y1;
					yb = y4;
					yc = y5;
				}
				else if( ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) && ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) && ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) && ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) && ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) && ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) && ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) )
				{
					xa = x2;
					xb = x4;
					xc = x5;
					ya = y2;
					yb = y4;
					yc = y5;
				}
				else if( ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) && ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) && ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) && ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) && ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) && ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) && ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) )
				{
					xa = x1;
					xb = x3;
					xc = x6;
					ya = y1;
					yb = y3;
					yc = y6;
				}
				else if( ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) && ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) && ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) && ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) && ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) && ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) && ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) )
				{
					xa = x2;
					xb = x3;
					xc = x6;
					ya = y2;
					yb = y3;
					yc = y6;
				}
				else if( ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) && ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) && ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) && ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) && ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) && ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) && ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) )
				{
					xa = x1;
					xb = x4;
					xc = x6;
					ya = y1;
					yb = y4;
					yc = y6;
				}
				else if( ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) && ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) && ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) < ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x5 ) * ( x2 - x5 ) + ( y2 - y5 ) * ( y2 - y5 ) + ( x4 - x5 ) * ( x4 - x5 ) + ( y4 - y5 ) * ( y4 - y5 ) && ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) && ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) < ( x2 - x3 ) * ( x2 - x3 ) + ( y2 - y3 ) * ( y2 - y3 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x3 - x6 ) * ( x3 - x6 ) + ( y3 - y6 ) * ( y3 - y6 ) && ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) < ( x1 - x4 ) * ( x1 - x4 ) + ( y1 - y4 ) * ( y1 - y4 ) + ( x1 - x6 ) * ( x1 - x6 ) + ( y1 - y6 ) * ( y1 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) && ( x2 - x4 ) * ( x2 - x4 ) + ( y2 - y4 ) * ( y2 - y4 ) + ( x2 - x6 ) * ( x2 - x6 ) + ( y2 - y6 ) * ( y2 - y6 ) + ( x4 - x6 ) * ( x4 - x6 ) + ( y4 - y6 ) * ( y4 - y6 ) < ( x1 - x3 ) * ( x1 - x3 ) + ( y1 - y3 ) * ( y1 - y3 ) + ( x1 - x5 ) * ( x1 - x5 ) + ( y1 - y5 ) * ( y1 - y5 ) + ( x3 - x5 ) * ( x3 - x5 ) + ( y3 - y5 ) * ( y3 - y5 ) )
				{
					xa = x2;
					xb = x4;
					xc = x6;
					ya = y2;
					yb = y4;
					yc = y6;
				}				
				ixs = ( xa * v_weight[0] * v_weight[0] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xa - xc ) * ( xa - xc ) + ( ya - yc ) * ( ya - yc ) ) + xb * v_weight[1] * v_weight[1] / ( ( xb - xa ) * ( xb - xa ) + ( yb - ya ) * ( yb - ya ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) ) + xc * v_weight[2] * v_weight[2] / ( ( xc - xa ) * ( xc - xa ) + ( yc - ya ) * ( yc - ya ) + ( xc - xb ) * ( xc - xb ) + ( yc - yb ) * ( yc - yb ) ) ) / ( v_weight[0] * v_weight[0] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xa - xc ) * ( xa - xc ) + ( ya - yc ) * ( ya - yc ) ) + v_weight[1] * v_weight[1] / ( ( xb - xa ) * ( xb - xa ) + ( yb - ya ) * ( yb - ya ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) ) + v_weight[2] * v_weight[2] / ( ( xc - xa ) * ( xc - xa ) + ( yc - ya ) * ( yc - ya ) + ( xc - xb ) * ( xc - xb ) + ( yc - yb ) * ( yc - yb ) ) );
				iys = ( ya * v_weight[0] * v_weight[0] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xa - xc ) * ( xa - xc ) + ( ya - yc ) * ( ya - yc ) ) + yb * v_weight[1] * v_weight[1] / ( ( xb - xa ) * ( xb - xa ) + ( yb - ya ) * ( yb - ya ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) ) + yc * v_weight[2] * v_weight[2] / ( ( xc - xa ) * ( xc - xa ) + ( yc - ya ) * ( yc - ya ) + ( xc - xb ) * ( xc - xb ) + ( yc - yb ) * ( yc - yb ) ) ) / ( v_weight[0] * v_weight[0] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xa - xc ) * ( xa - xc ) + ( ya - yc ) * ( ya - yc ) ) + v_weight[1] * v_weight[1] / ( ( xb - xa ) * ( xb - xa ) + ( yb - ya ) * ( yb - ya ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) ) + v_weight[2] * v_weight[2] / ( ( xc - xa ) * ( xc - xa ) + ( yc - ya ) * ( yc - ya ) + ( xc - xb ) * ( xc - xb ) + ( yc - yb ) * ( yc - yb ) ) );

	
			}
			if( ii == 3 )
			{
			
				x3 = x[ii] - v_disp[ii] * cosphi[ii];
				x4 = x[ii] + v_disp[ii] * cosphi[ii];
				
				y3 = y[ii] - v_disp[ii] * sinphi[ii];
				y4 = y[ii] + v_disp[ii] * sinphi[ii];
				
				
				if( ( ixs - x3 ) * ( ixs - x3 ) + ( iys - y3 ) * ( iys - y3 ) < ( ixs - x4 ) * ( ixs - x4 ) + ( iys - y4 ) * ( iys - y4 ) )
				{
					xd = x3;
					yd = y3;
				}
				else
				{
					xd = x4;
					yd = y4;
				}
			}
			
			
		}
		
		if( v_weight.size() == 2 ) // 2 images
		{
			xs = ixs;
			ys = iys;
			dispdiff = ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb );
		}
		
		else if( v_weight.size() == 3 ) // 3 images
		{
			xs = ( xa * v_weight[0] * v_weight[0] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xa - xc ) * ( xa - xc ) + ( ya - yc ) * ( ya - yc ) ) + xb * v_weight[1] * v_weight[1] / ( ( xb - xa ) * ( xb - xa ) + ( yb - ya ) * ( yb - ya ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) ) + xc * v_weight[2] * v_weight[2] / ( ( xc - xa ) * ( xc - xa ) + ( yc - ya ) * ( yc - ya ) + ( xc - xb ) * ( xc - xb ) + ( yc - yb ) * ( yc - yb ) ) ) / ( v_weight[0] * v_weight[0] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xa - xc ) * ( xa - xc ) + ( ya - yc ) * ( ya - yc ) ) + v_weight[1] * v_weight[1] / ( ( xb - xa ) * ( xb - xa ) + ( yb - ya ) * ( yb - ya ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) ) + v_weight[2] * v_weight[2] / ( ( xc - xa ) * ( xc - xa ) + ( yc - ya ) * ( yc - ya ) + ( xc - xb ) * ( xc - xb ) + ( yc - yb ) * ( yc - yb ) ) );
			ys = ( ya * v_weight[0] * v_weight[0] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xa - xc ) * ( xa - xc ) + ( ya - yc ) * ( ya - yc ) ) + yb * v_weight[1] * v_weight[1] / ( ( xb - xa ) * ( xb - xa ) + ( yb - ya ) * ( yb - ya ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) ) + yc * v_weight[2] * v_weight[2] / ( ( xc - xa ) * ( xc - xa ) + ( yc - ya ) * ( yc - ya ) + ( xc - xb ) * ( xc - xb ) + ( yc - yb ) * ( yc - yb ) ) ) / ( v_weight[0] * v_weight[0] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xa - xc ) * ( xa - xc ) + ( ya - yc ) * ( ya - yc ) ) + v_weight[1] * v_weight[1] / ( ( xb - xa ) * ( xb - xa ) + ( yb - ya ) * ( yb - ya ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) ) + v_weight[2] * v_weight[2] / ( ( xc - xa ) * ( xc - xa ) + ( yc - ya ) * ( yc - ya ) + ( xc - xb ) * ( xc - xb ) + ( yc - yb ) * ( yc - yb ) ) );

			dispdiff = ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xa - xc ) * ( xa - xc ) + ( ya - yc ) * ( ya - yc ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc );
		}
		
		else if( v_weight.size() == 4 ) // 4 images
		{
			xs = ( xa * v_weight[0] * v_weight[0] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xa - xc ) * ( xa - xc ) + ( ya - yc ) * ( ya - yc ) + ( xa - xd ) * ( xa - xd ) + ( ya - yd ) * ( ya - yd ) ) + xb * v_weight[1] * v_weight[1] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) + ( xb - xd ) * ( xb - xd ) + ( yb - yd ) * ( yb - yd ) ) + xc * v_weight[2] * v_weight[2] / ( ( xc - xa ) * ( xc - xa ) + ( yc - ya ) * ( yc - ya ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) + ( xc - xd ) * ( xc - xd ) + ( yc - yd ) * ( yc - yd ) ) + xd * v_weight[3] * v_weight[3] / ( ( xa - xd ) * ( xa - xd ) + ( ya - yd ) * ( ya - yd ) + ( xb - xd ) * ( xb - xd ) + ( yb - yd ) * ( yb - yd ) + ( xc - xd ) * ( xc - xd ) + ( yc - yd ) * ( yc - yd ) ) ) / ( v_weight[0] * v_weight[0] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xa - xc ) * ( xa - xc ) + ( ya - yc ) * ( ya - yc ) + ( xa - xd ) * ( xa - xd ) + ( ya - yd ) * ( ya - yd ) ) + v_weight[1] * v_weight[1] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) + ( xb - xd ) * ( xb - xd ) + ( yb - yd ) * ( yb - yd ) ) + v_weight[2] * v_weight[2] / ( ( xc - xa ) * ( xc - xa ) + ( yc - ya ) * ( yc - ya ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) + ( xc - xd ) * ( xc - xd ) + ( yc - yd ) * ( yc - yd ) ) + v_weight[3] * v_weight[3] / ( ( xa - xd ) * ( xa - xd ) + ( ya - yd ) * ( ya - yd ) + ( xb - xd ) * ( xb - xd ) + ( yb - yd ) * ( yb - yd ) + ( xc - xd ) * ( xc - xd ) + ( yc - yd ) * ( yc - yd ) ) );
			ys = ( ya * v_weight[0] * v_weight[0] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xa - xc ) * ( xa - xc ) + ( ya - yc ) * ( ya - yc ) + ( xa - xd ) * ( xa - xd ) + ( ya - yd ) * ( ya - yd ) ) + yb * v_weight[1] * v_weight[1] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) + ( xb - xd ) * ( xb - xd ) + ( yb - yd ) * ( yb - yd ) ) + yc * v_weight[2] * v_weight[2] / ( ( xc - xa ) * ( xc - xa ) + ( yc - ya ) * ( yc - ya ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) + ( xc - xd ) * ( xc - xd ) + ( yc - yd ) * ( yc - yd ) ) + yd * v_weight[3] * v_weight[3] / ( ( xa - xd ) * ( xa - xd ) + ( ya - yd ) * ( ya - yd ) + ( xb - xd ) * ( xb - xd ) + ( yb - yd ) * ( yb - yd ) + ( xc - xd ) * ( xc - xd ) + ( yc - yd ) * ( yc - yd ) ) ) / ( v_weight[0] * v_weight[0] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xa - xc ) * ( xa - xc ) + ( ya - yc ) * ( ya - yc ) + ( xa - xd ) * ( xa - xd ) + ( ya - yd ) * ( ya - yd ) ) + v_weight[1] * v_weight[1] / ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) + ( xb - xd ) * ( xb - xd ) + ( yb - yd ) * ( yb - yd ) ) + v_weight[2] * v_weight[2] / ( ( xc - xa ) * ( xc - xa ) + ( yc - ya ) * ( yc - ya ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) + ( xc - xd ) * ( xc - xd ) + ( yc - yd ) * ( yc - yd ) ) + v_weight[3] * v_weight[3] / ( ( xa - xd ) * ( xa - xd ) + ( ya - yd ) * ( ya - yd ) + ( xb - xd ) * ( xb - xd ) + ( yb - yd ) * ( yb - yd ) + ( xc - xd ) * ( xc - xd ) + ( yc - yd ) * ( yc - yd ) ) );

			dispdiff = ( ( xa - xb ) * ( xa - xb ) + ( ya - yb ) * ( ya - yb ) + ( xa - xc ) * ( xa - xc ) + ( ya - yc ) * ( ya - yc ) + ( xb - xc ) * ( xb - xc ) + ( yb - yc ) * ( yb - yc ) + ( xa - xd ) * ( xa - xd ) + ( ya - yd ) * ( ya - yd ) + ( xb - xd ) * ( xb - xd ) + ( yb - yd ) * ( yb - yd ) + ( xc - xd ) * ( xc - xd ) + ( yc - yd ) * ( yc - yd ) );
		}
		
		else
		{
			xs = -99999.;
			ys = -99999.;
			dispdiff = -9999.;
		}
		
	}
}

float VDispAnalyzer::getXcoordinate_disp( unsigned int ii, float x, float cosphi )
{
	if( fDispTableAnalyzer )
	{
		return fDispTableAnalyzer->getXcoordinate_disp( ii );
	}
	
	else
	{
		return x - f_disp * cosphi;
	}
	
}

float VDispAnalyzer::getYcoordinate_disp( unsigned int ii, float y, float sinphi )
{
	if( fDispTableAnalyzer )
	{
		return fDispTableAnalyzer->getYcoordinate_disp( ii );
	}
	
	else
	{
		return y - f_disp * sinphi;
	}
}

