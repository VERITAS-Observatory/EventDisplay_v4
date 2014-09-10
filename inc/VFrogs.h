#ifndef VFROGS_H_INC
#define VFROGS_H_INC

#include "TDirectory.h"
#include "TError.h"
#include "TFile.h"
#include "TMath.h"
#include "TSystem.h"

#include "frogs.h"

#include "VEvndispData.h"
#include "VShowerParameters.h"
#include "VFrogsParameters.h"
#include <VEvndispRunParameter.h>
#include <VDetectorGeometry.h>
#include <VGrIsuAnalyzer.h>

#include <fstream>
#include <iostream>
#include <math.h>
#include <set>
#include <string>
#include <vector>
#include <valarray>

using namespace std;

class VFrogs : public VEvndispData, public VGrIsuAnalyzer
{
	public:
		VFrogs();
		~VFrogs();
		
		// vectors for readTableFrogs mscw runNumber and Erec
		vector<int>    fTableEventNumber;
		vector<double> fTableEnergy;
		
		vector<int>    fAnasumRunNumber;
		vector<int>    fAnasumEventNumber;
		
		void  doFrogsStuff( int );                      //!< do the actual analysis (called for each event)
		int   getFrogsEventID()
		{
			return frogsEventID;
		}
		int   getFrogsGSLConStat()
		{
			return frogsGSLConStat;
		}
		int   getFrogsNB_iter()
		{
			return frogsNB_iter;
		}
		int   getFrogsNImages()
		{
			return frogsNImages;
		}
		float getFrogsXS()
		{
			return frogsXS;
		}
		float getFrogsXSerr()
		{
			return frogsXSerr;
		}
		float getFrogsYS()
		{
			return frogsYS;
		}
		float getFrogsYSerr()
		{
			return frogsYSerr;
		}
		float getFrogsXP()
		{
			return frogsXP;
		}
		float getFrogsXPerr()
		{
			return frogsXPerr;
		}
		float getFrogsYP()
		{
			return frogsYP;
		}
		float getFrogsYPerr()
		{
			return frogsYPerr;
		}
		float getFrogsXPGC()
		{
			return frogsXPGC;
		}
		float getFrogsYPGC()
		{
			return frogsYPGC;
		}
		float getFrogsEnergy()
		{
			return frogsEnergy;
		}
		float getFrogsEnergyerr()
		{
			return frogsEnergyerr;
		}
		float getFrogsLambda()
		{
			return frogsLambda;
		}
		float getFrogsLambdaerr()
		{
			return frogsLambdaerr;
		}
		float getFrogsGoodnessImg()
		{
			return frogsGoodnessImg;
		}
		int   getFrogsNpixImg()
		{
			return frogsNpixImg;
		}
		float getFrogsGoodnessBkg()
		{
			return frogsGoodnessBkg;
		}
		int   getFrogsNpixBkg()
		{
			return frogsNpixBkg;
		}
		float getFrogsXPStart()
		{
			return frogsXPStart;
		}
		float getFrogsYPStart()
		{
			return frogsYPStart;
		}
		float getFrogsXPED()
		{
			return frogsXPED;
		}
		float getFrogsYPED()
		{
			return frogsYPED;
		}
		float getFrogsXSStart()
		{
			return frogsXSStart;
		}
		float getFrogsYSStart()
		{
			return frogsYSStart;
		}
		float getFrogsTelGoodnessImg( int i )
		{
			return frogsTelGoodnessImg[i];
		}
		float getFrogsTelGoodnessBkg( int i )
		{
			return frogsTelGoodnessBkg[i];
		}
		
		void initAnalysis();
		void initFrogsTree();
		void initOutput();
		void initFrogsEvent();
		void reset();
		void terminate();
		float transformTelescopePosition( int iTel, float i_ze, float i_az, int axis );
		float transformShowerPosition( float i_ze, float i_az, float xcore, float ycore, int axis );
		float transformPosition( float i_ze, float i_az, float x, float y, float z, int axis, bool bInv );
		void readTableFrogs();
		double getFrogsStartEnergy( int eventNumber );
		int getFrogsAnasumNumber( int eventNumber, int runNumber );
		void finishFrogs( TFile* f );
		
		TFile* mscwFrogsFile;
		TFile* AnasumFrogsFile;
		
		
	private:
		struct 		     frogs_imgtmplt_in frogs_convert_from_ed( int eventNumber, int adc_type, double inEnergy );
		VEvndispData*         fData;                    //!< pointer to data class
		//VShowerParameters    *fShowerParam;
		//VEvndispRunParameter *fRunPara;                //!< data class for all run parameters
		//VVirtualDataReader   *fReader;
		
		int frogsRecID;
		string templatelistname;
		
		int   frogsEventID;
		int   frogsGSLConStat;
		int   frogsNB_iter;
		int   frogsNImages;
		float frogsXS;
		float frogsXSerr;
		float frogsYS;
		float frogsYSerr;
		float frogsXP;
		float frogsXPerr;
		float frogsYP;
		float frogsYPerr;
		float frogsXPGC;
		float frogsYPGC;
		float frogsEnergy;
		float frogsEnergyerr;
		float frogsLambda;
		float frogsLambdaerr;
		float frogsGoodnessImg;
		int   frogsNpixImg;
		float frogsGoodnessBkg;
		int   frogsNpixBkg;
		
		float frogsXPStart;
		float frogsYPStart;
		float frogsXPED;
		float frogsYPED;
		float frogsXSStart;
		float frogsYSStart;
		
		bool  fInitialized;                        //!< true after initialization
		int   fStartEnergyLoop;
		
		double frogsTemplateMu0[500];
		double frogsTemplateMu1[500];
		double frogsTemplateMu2[500];
		double frogsTemplateMu3[500];
		
		float frogsTelGoodnessImg[4];
		float frogsTelGoodnessBkg[4];
		
};
#endif
