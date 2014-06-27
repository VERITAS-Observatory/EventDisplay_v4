//! VEvndispReconstructionParameter event cuts for array analysis

#ifndef VARRAYANALYSISCUTS_H
#define VARRAYANALYSISCUTS_H

#include "VEvndispRunParameter.h"
#include "VImageParameter.h"
#include "VStarCatalogue.h"
#include "VUtilities.h"

#include <TNamed.h>
#include <TSystem.h>

#include <bitset>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class VEvndispReconstructionParameter : public TNamed
{
	private:
	
		bool   fDebug;
		
		VEvndispRunParameter*  fRunPara;
		
		unsigned int fNTel_type;
		vector< ULong64_t > fTel_type_V;
		set< ULong64_t > fTel_type;
		
		void addNewMethod( unsigned int iMethodID );
		void reset();
		
	public:
		unsigned int fNMethods;                   // total number of methods
		
		vector< unsigned int > fRecordCounter;
		vector< unsigned int > fMethodID;
		vector< int > fNImages_min;
		vector< double > fAxesAngles_min;
		
		// [methodID][telescope type] (not telescope number!!)
		vector< vector< int > >    fLocalNtubes_min;
		vector< vector< int > >    fLocalNLowGain_max;
		vector< vector< double > > fLocalDistance_min;
		vector< vector< double > > fLocalDistance_max;
		vector< vector< double > > fSize_min;
		vector< vector< double > > fSize_max;
		vector< vector< double > > fLength_min;
		vector< vector< double > > fLength_max;
		vector< vector< double > > fWidth_min;
		vector< vector< double > > fWidth_max;
		vector< vector< double > > fLocalAlpha_min;
		vector< vector< double > > fLocalAlpha_max;
		vector< vector< double > > fAsym_min;
		vector< vector< double > > fAsym_max;
		vector< vector< double > > fLoss_max;
		vector< vector< double > > fFui_min;
		vector< vector< double > > fWidthLength_max;
		vector< vector< unsigned int > > fL2TriggerType;
		vector< vector< double > > fMCEnergy_linTeV_min;
		vector< vector< double > > fMCEnergy_linTeV_max;
		
		vector< bool > fUseEventdisplayPointing;
		vector< vector< bool > > fLocalUseImage;
		
		vector< string > fMLPFileName;
                vector< vector< double > > fMTVAZenithBin;
		vector< vector< string > > fTMVAFileName;
		vector< string > fDispFileName;
		
		vector< float >  fMODDISP_MinAngleForDisp;
		vector< float >  fMODDISP_MinAngleExpFactor;
		
		VEvndispReconstructionParameter();
		VEvndispReconstructionParameter( vector< ULong64_t > itel_type, VEvndispRunParameter* iRunPara );
		~VEvndispReconstructionParameter() {}
		
		bool   applyArrayAnalysisCuts( unsigned int iMeth, unsigned int iTel, unsigned int iTelType,
									   VImageParameter* iImageParameter, unsigned short int iLocalTriggerType,
									   VStarCatalogue* iStar = 0 );
		int    getTelescopeType_counter( ULong64_t t );
		int    getTelescopeType_counter_from_MirrorArea( ULong64_t t );
		int    getTelescopeType_counter_from_MirrorArea_and_PixelSize( ULong64_t t );
		vector <int >    getTelescopeType_counterVector( ULong64_t t );
		vector< int >    getTelescopeType_counter_from_MirrorAreaVector( ULong64_t t );
		vector< int >    getTelescopeType_counter_from_MirrorArea_and_PixelSizeVector( ULong64_t t );
		void   print_arrayAnalysisCuts();
		unsigned int read_arrayAnalysisCuts( string ifile );
		void   setDebug( bool iD = false )
		{
			fDebug = iD;
		}
		
		ClassDef( VEvndispReconstructionParameter, 19 );
};
#endif
