//! VReadRunParameter  reader for runparameter (now from command line, later from configuration file)

#ifndef VREADRUNPARAMETER_H
#define VREADRUNPARAMETER_H

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "TFile.h"
#include "TSystem.h"
#include "TTree.h"

#include <VGlobalRunParameter.h>
#include <VDBRunInfo.h>
#include <VEvndispRunParameter.h>
#include <VTargets.h>
#include <VUtilities.h>

class VReadRunParameter
{
    private:
        bool fDebug;
        bool fPrintOutputFile;
        VEvndispRunParameter *fRunPara;                //!< data class for all run parameters
        bool fusercamera;                         //!< default camera or user camera
        void test_and_adjustParams();             //!< get run number, default camera..
        void printHelp();                         //!< print a short help
	void printShortHelp();

        bool f_boolCommandline;                   //!< reading parameters from command line
        bool f_boolConfigfile;                    //!< reading parameters from config file

        unsigned int fTelToAna;
        string       fTelToAnaString;

	bool fGetLaserRunNumber;

                                                  //!< prel pointing error
        map< unsigned int, double > f_pointingErrorX;
                                                  //!< prel pointing error
        map< unsigned int, double > f_pointingErrorY;

        void isCompiledWithDB();
        bool getRunParametersFromDST();
        void setDirectories();

    public:
        VReadRunParameter();
        ~VReadRunParameter() {}
        VEvndispRunParameter* getRunParameter()        //!< return vector with run parameters
        {
            return fRunPara;
        }
        void printTargets();                      //!< print targets
	void printStartMessage();
                                                  //!< read in all run parameters from command line
        bool readCommandline( int argc, char *arg[] );
        bool readConfigFile( string i_cFile );    //!< read run parameters from configuration file
};
#endif
