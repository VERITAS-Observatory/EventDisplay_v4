/*! \class VTMVARunDataZenithCut
    \brief  VTMVARunDataZenithCut data class for TMVA energy cuts


*/

#include "VTMVARunDataZenithCut.h"

VTMVARunDataZenithCut::VTMVARunDataZenithCut()
{
	fZenithCutBin = 0;
	fZenithCut_min = 0.;
	fZenithCut_max = 0;
	fZenithCut = "";
}

void VTMVARunDataZenithCut::print()
{
	cout << "zenith bin " << fZenithCutBin;
	cout << ":  [" << fZenithCut_min;
	cout << ", " << fZenithCut_max << "] deg";
	cout << "\t cuts: " << fZenithCut.GetTitle() << endl;
}


