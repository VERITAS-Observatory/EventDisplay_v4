//! VFrogParameters storage class for Frogs data
#ifndef VFROGPARAMETERS_H
#define VFROGPARAMETERS_H

#include "VGlobalRunParameter.h"
#include "TTree.h"

#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>

using namespace std;

class VFrogParameters
{
    private:
        bool fDebug;
        TTree *fTreeFrog;                           //!< output tree
        unsigned int fNTel;                       //!< number of telescopes

    public:

//        VFrogParameters( int iNTel = 4, unsigned int iShortTree = 0 );
        VFrogParameters();
        ~VFrogParameters();

        void fill() { if( fTreeFrog ) fTreeFrog->Fill(); }
        TTree *getTree() { return fTreeFrog; }
        void initTree( string, string );
        void printParameters();                   //!< write tree parameters to standard output
        void reset();                             //!< reset all tree variable to standard values

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
	float frogsXSStart;
	float frogsYSStart;

};
#endif
