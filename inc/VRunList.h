//! VRunList data class
// Revision $Id: VRunList.h,v 1.1.2.7.8.1.2.2.2.1 2010/11/15 10:34:06 gmaier Exp $

#ifndef VRUNLIST_H
#define VRUNLIST_H

#include "TObject.h"

#include <iomanip>
#include <iostream>

using namespace std;

class VRunList : public TObject
{
    public:

        int runnumber;
        double MJD;
        double tOn;                               // [s]
        double deadTimeFraction;
        double NOn;
        double NOff;
        double OffNorm;
        double elevationOn;
        double elevationOff;
	double TargetRAJ2000;
	double TargetDecJ2000;
        double pedvarsOn;
        double alpha;
        double energyThreshold;                   //  TeV (lin)
	double phase;                             //  e.g. orbital phase

        VRunList();
        ~VRunList() {}
        void print();
	void print( bool csv );
        void reset();

        ClassDef (VRunList, 4 );
};
#endif
