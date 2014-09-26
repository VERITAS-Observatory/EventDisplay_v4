//! VTMVARunDataZenithCut data class for TMVA energy cuts

#ifndef VTMVARunDataZenithCut_H
#define VTMVARunDataZenithCut_H

#include <iostream>

#include "TCut.h"

using namespace std;

class VTMVARunDataZenithCut : public TNamed
{
	public:
	
		unsigned int fZenithCutBin;
		double       fZenithCut_min;
		double       fZenithCut_max;
		TCut         fZenithCut;
		
		VTMVARunDataZenithCut();
		~VTMVARunDataZenithCut() {}
		
		void print();
		
		ClassDef( VTMVARunDataZenithCut, 2 );
};

#endif

