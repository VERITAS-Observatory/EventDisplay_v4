//! VTargets  data class with all astronomical targets
//  Revision $Id: VTargets.h,v 1.3.10.1.12.1.8.2.10.1 2010/03/08 08:01:35 gmaier Exp $
//
//  NOTE:    THIS CLASS SHOULD NOT BE USED ANYMORE
//

#ifndef VTARGETS_H
#define VTARGETS_H

#include "TMath.h"

#include "VASlalib.h"

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>

using namespace std;

class VTargets
{
    private:
        vector< string > fTargetCode;
        vector< string > fTargetName;
        vector< string > fDec2000;
        vector< double > fDec2000rad;
        vector< string > fRA2000;
        vector< double > fRA2000rad;

        unsigned int fIndexSelected;

        void convertCoordinates();
        double getDecrad( string );
        double getRArad( string );
        string lowerCase(string& s);

    public:

        VTargets();
        ~VTargets() {}
        string getTargetCode();
        double getTargetDecJ2000();
        string getTargetName();
        double getTargetRAJ2000();
        void printTargets();                      // print target list to stdout
        bool readTargets( string iTargetFile );   // read targets from a text file
                                                  // use this target
        bool selectTargetbyName( string iTargetName );
                                                  // use this target
        bool selectTargetbyCode( string iTargetName );
        void setTarget( string iname, double iRaJ2000, double iDecJ2000 );
};
#endif
