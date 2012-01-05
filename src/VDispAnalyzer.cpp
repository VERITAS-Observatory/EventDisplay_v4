/* \class VDispAnalyzer


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
      else setZombie( false );
   }
   else if( fDispMethod == "DISPTABLES" )
   {
      fDispTableAnalyzer = new VDispTableAnalyzer( iFile );
      if( fDispTableAnalyzer->isZombie() )
      {
         setZombie( true );
      }
      else setZombie( false );
   }
   else if( fDispMethod == "TMVABDT" )
   {
      fTMVADispAnalyzer = new VTMVADispAnalyzer( iFile, fTelescopeTypeList );
      if( fTMVADispAnalyzer->isZombie() )
      {
         setZombie( true );
      }
      else setZombie( false );
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
   if( fMLPAnalyzer ) fMLPAnalyzer->terminate();

   if( fDispTableAnalyzer ) fDispTableAnalyzer->terminate(); 

   if( fTMVADispAnalyzer )  fTMVADispAnalyzer->terminate();
}

float VDispAnalyzer::evaluate( float iWidth, float iLength, float iAsymm, float iDist, float iSize,
                               float iPedvar, float tgrad, ULong64_t iTelType, float iZe, float iAz, bool b2D )
{
    f_disp = -99.;

    if( fMLPAnalyzer )            f_disp = fMLPAnalyzer->evaluate( iWidth, iLength, iAsymm, iSize, iDist );
    else if( fDispTableAnalyzer ) f_disp = fDispTableAnalyzer->evaluate( iWidth, iLength, iSize, iPedvar, iZe, iAz, b2D ); 
    else if( fTMVADispAnalyzer  ) f_disp = fTMVADispAnalyzer->evaluate( iWidth, iLength, iSize, iPedvar, tgrad, iZe, iAz, iAsymm, iTelType );

    return f_disp;
}

void VDispAnalyzer::calculateMeanDirection( float &xs, float &ys, vector< float > x, vector< float > y, 
                                            vector< float > cosphi, vector< float > sinphi, vector< float > v_disp, vector< float > v_weight )
{
    if( fDispTableAnalyzer )
    {
       fDispTableAnalyzer->calculateMeanDirection( xs, ys, x, y, cosphi, sinphi, v_disp, v_weight );
    }
    else
    {
       float itotweight = 0.;
       float ixs = 0.;
       float iys = 0.;

       for( unsigned int ii = 0; ii < v_weight.size(); ii++ )
       {
	   ixs += getXcoordinate_disp( ii, x[ii], cosphi[ii] ) * v_weight[ii];
	   iys += getYcoordinate_disp( ii, y[ii], sinphi[ii] ) * v_weight[ii];
	   itotweight += v_weight[ii];
       }
       if( itotweight > 0. )
       {
	   xs = ixs / itotweight;
	   ys = iys / itotweight;
       }
       else
       {
	   xs = -99999.;
	   ys = -99999.;
       }
    }
}

float VDispAnalyzer::getXcoordinate_disp( unsigned int ii, float x, float cosphi )
{
   if( fDispTableAnalyzer ) return fDispTableAnalyzer->getXcoordinate_disp( ii );

   return x - f_disp * cosphi;
}
      
float VDispAnalyzer::getYcoordinate_disp( unsigned int ii, float y, float sinphi )
{
   if( fDispTableAnalyzer ) return fDispTableAnalyzer->getYcoordinate_disp( ii );

   return y - f_disp * sinphi;
}

