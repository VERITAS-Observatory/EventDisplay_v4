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
#include "VFrogParameters.h"
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

//#include <gsl/gsl_vector.h>
//#include <gsl/gsl_multifit_nlin.h> //Levenberg-Marquardt 
//#include <gsl/gsl_blas.h> //Levenberg-Marquardt linear algebra
//#include <gsl/gsl_integration.h>
//#include <gsl/gsl_interp.h>



using namespace std;

class VFrogs : public VEvndispData, public VGrIsuAnalyzer
{
    public:
        VFrogs();
        ~VFrogs();

// vectors for readTableFrogs mscw runNumber and Erec
	vector<int>    fTableRunNumber;
	vector<double> fTableEnergy;

	vector<int>    fAnasumRunNumber;
	vector<int>    fAnasumEventNumber;

        void  doFrogsStuff(int);                        //!< do the actual analysis (called for each event)
        int   getFrogsEventID();
	int   getFrogsGSLConStat();
	int   getFrogsNB_iter();
	float getFrogsXS();
	float getFrogsXSerr();
	float getFrogsYS();
	float getFrogsYSerr();
	float getFrogsXP();
	float getFrogsXPerr();
	float getFrogsYP();
	float getFrogsYPerr();
	float getFrogsEnergy();
	float getFrogsEnergyerr();
	float getFrogsLambda();
	float getFrogsLambdaerr();
	float getFrogsGoodnessImg();
	int   getFrogsNpixImg();
	float getFrogsGoodnessBkg();
	int   getFrogsNpixBkg();

	float getFrogsXPStart();
	float getFrogsYPStart();
	float getFrogsXPED();
	float getFrogsYPED();
	float getFrogsXSStart();
	float getFrogsYSStart();
	float getFrogsTelGoodnessImg( int i );
	float getFrogsTelGoodnessBkg( int i );

        void initAnalysis();
        void initFrogTree();
        void initOutput();
        void initFrogEvent();
	void terminate();
	float transformTelescopePosition( int iTel, float i_ze, float i_az, int axis );
	void readTableFrogs();
	void readAnasumFrogs();
	double getFrogsStartEnergy(int eventNumber);
	int getFrogsAnasumNumber(int eventNumber, int runNumber);
        void finishFrogs(TFile *f);
        //void finishFrogs();

	TFile *mscwFrogsFile;
	TFile *AnasumFrogsFile;


    private:
	struct 		     frogs_imgtmplt_in frogs_convert_from_ed(int eventNumber, int adc_type, double inEnergy); 
        VEvndispData         *fData;                    //!< pointer to data class
	VShowerParameters    *fShowerParam;
//	VFrogParameters      *fFrogParameters;
        VEvndispRunParameter *fRunPara;                //!< data class for all run parameters
//	VDetectorGeometry    *fDetector;
//        VBFDataReader        *fVBFReader;
        VVirtualDataReader   *fReader;

	int frogsRecID;

	int   frogsEventID;
        int   frogsGSLConStat; 
        int   frogsNB_iter;
        float frogsXS;
        float frogsXSerr;
        float frogsYS;
        float frogsYSerr;
        float frogsXP;
        float frogsXPerr;
        float frogsYP;
        float frogsYPerr;
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
