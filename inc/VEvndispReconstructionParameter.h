//! VEvndispReconstructionParameter event cuts for array analysis
// Revision $Id: VEvndispReconstructionParameter.h,v 1.5.2.1.4.1.4.1.4.2.10.1.4.1.4.4.2.7.4.2 2011/04/11 07:56:26 prokoph Exp $

#ifndef VARRAYANALYSISCUTS_H
#define VARRAYANALYSISCUTS_H

#include "VImageParameter.h"
#include "VUtilities.h"

#include <TNamed.h>
#include <TSystem.h>

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

    double fDefault_imagethresh;
    double fDefault_borderthresh;
    double fDefault_brightimagethresh;

    void addNewMethod( unsigned int iMethodID );
    void reset();

    public:
        unsigned int fNMethods;                   // total number of methods
        unsigned int fNTel_type;
	set< ULong64_t > fTel_type;

        vector< unsigned int > fRecordCounter;
        vector< unsigned int > fMethodID;
        vector< int > fNImages_min;
        vector< double > fAxesAngles_min;
// [methodID][telescope type] (not telescope number!!)
        vector< vector< double > > fimagethresh;
	vector< vector< double > > fborderthresh;
	vector< vector< double > > fbrightnonimagetresh;

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
        vector< bool > fUseEventdisplayPointing;
// C. Duke: 20oct06  added vector to store select image results
        vector< vector< bool > > fLocalUseImage;

        vector< string > fMLPFileName;
	vector< string > fTMVAFileName;
        vector< string > fDispFileName;

	vector< float >  fMODDISP_MinAngleForDisp;
	vector< float >  fMODDISP_MinAngleExpFactor;

        VEvndispReconstructionParameter();
        VEvndispReconstructionParameter( vector< ULong64_t > itel_type );
        ~VEvndispReconstructionParameter() {}

	bool   applyArrayAnalysisCuts( unsigned int iMeth, unsigned int iTel, VImageParameter* iImageParameter );
	double getImageThreshold( ULong64_t i_tel_type );
	double getBorderThreshold( ULong64_t i_tel_type );
	double getBrightNonImageThreshold( ULong64_t i_tel_type );
        int    getTelescopeType_counter( ULong64_t t );
	int    getTelescopeType_counter_from_MirrorArea( ULong64_t t );
	int    getTelescopeType_counter_from_MirrorArea_and_PixelSize( ULong64_t t );
        void   print_arrayAnalysisCuts();
        unsigned int read_arrayAnalysisCuts( string ifile );
	void   setDebug( bool iD = false ) { fDebug = iD; }
	void   setDefaultThresholds( double imagethresh, double borderthresh, double brightimagethresh );

        ClassDef(VEvndispReconstructionParameter,12);
};
#endif
