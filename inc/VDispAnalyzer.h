//! VDispAnalyzer wrapper class for all modified disp analysis

#ifndef VDispAnalyzer_H
#define VDispAnalyzer_H

#include "TFile.h"
#include "TMath.h"

#include <iostream>
#include <string>
#include <vector>

#include "VDispTableAnalyzer.h"
#include "VMLPAnalyzer.h"
#include "VTMVADispAnalyzer.h"

using namespace std;

class VDispAnalyzer
{
    private:

    bool                bZombie;

    string              fDispMethod;

    VMLPAnalyzer       *fMLPAnalyzer;
    VDispTableAnalyzer *fDispTableAnalyzer;
    VTMVADispAnalyzer  *fTMVADispAnalyzer;

    float f_disp;
    float f_dispE;

    vector<ULong64_t> fTelescopeTypeList;

    public:

     VDispAnalyzer();
     ~VDispAnalyzer() {}

     void  calculateMeanDirection( float &xs, float &ys, vector< float > x, vector< float > y,
				   vector< float > cosphi, vector< float > sinphi, vector< float > v_disp, vector< float > v_weight );
     float evaluate( float iWidth, float iLength, float iAsymm, float iDist, float iSize, float iPedvar, float tgrad, ULong64_t iTelType,
                     float iZe, float iAz, bool b2D = true );
     float getDisp() { return f_disp; }
     float getDispE() { return f_dispE; }
     float getXcoordinate_disp( unsigned int i, float x, float cosphi ); 
     float getYcoordinate_disp( unsigned int i, float y, float sinphi );
     bool  initialize( string iFile, string iDispMethod );
     bool  isZombie() { return bZombie; }
     void  setTelescopeTypeList( vector<ULong64_t> iTelescopeTypeList );
     void  setZombie( bool iB = true ) { bZombie = iB; }
     void  terminate();
};

#endif
