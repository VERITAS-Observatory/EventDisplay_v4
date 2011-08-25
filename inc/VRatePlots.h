//! VRatePlots   class for significance, rates, etc. graphs
//  Revision $Id: VRatePlots.h,v 1.3.2.1.26.1.10.1 2010/03/08 07:45:10 gmaier Exp $

#ifndef VRatePlots_H
#define VRatePlots_H

#include "TDirectory.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TList.h"

#include "VAnaSumRunParameter.h"

#include <iostream>
#include <map>
#include <vector>

using namespace std;

class VRatePlots
{
    private:

        vector< int > fRuns;
        map< int, double > fTime;

        TList *hList;
        TList *hListTime;
        TList *hListRun;

        TGraph *hSig_run;
        TGraph *hSigMax_run;
        TGraphErrors *hOn_run;
        TGraphErrors *hOff_run;
        TGraphErrors *hRate_run;

        TGraph *hSig_time;
        TGraph *hSigMax_time;
        TGraphErrors *hOn_time;
        TGraphErrors *hOff_time;
        TGraphErrors *hRate_time;

        void defineRunGraphs( int );
        void defineTimeGraphs( int );

    public:

        VRatePlots();
        VRatePlots( VAnaSumRunParameter*, map< int, double > );
        ~VRatePlots() {}
        void fill( int irun, double iMJD, double isig, double isigmax, double ion, double ioff, double irate );
        void fill( int irun, double isig, double isigmax, double ion, double ioff, double irate );
        void fill(  vector< double > itime, vector< double > ion, vector< double > ioff, vector< double > isig, vector< double > irate );
        void write();
        void write( string );
};
#endif
