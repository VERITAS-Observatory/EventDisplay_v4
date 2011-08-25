//! VDeadTime dead time calculator
//  Revision $Id: VDeadTime.h,v 1.3.8.1.16.1.14.1 2010/03/08 07:45:09 gmaier Exp $

#ifndef VDEADTIME_H
#define VDEADTIME_H

#include "TDirectory.h"
#include "TDirectoryFile.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TList.h"
#include "TMath.h"

#include <iostream>

using namespace std;

class VDeadTime
{
    private:
        bool bIsOn;

        double fDeadTimeMiss;
        double fDeadTimeMS;
        double fDeadTimeFrac;

        double fTFitMin;
        double fTFitMax;

        double ft0;
        double fRunStart;
        TH1D *hTimeDiff;
        TH1D *hTimeDiffLog;
        TH2D *hTimeDiff2D;
        TF1  *hFTimeDiff;
        TGraphErrors *hgDeadTime;
        TH1D *hNEventTime;
        TList *hisList;

    public:
        VDeadTime( bool iIsOn = true );
        ~VDeadTime() {}
        void defineHistograms();
        double fillDeadTime( double time );
        double calculateDeadTime();
        double getDeadTimeMS() { return fDeadTimeMS; }
        double getDeadTimeFraction() { return fDeadTimeFrac; }
        TList* getDeadTimeHistograms();
        void printDeadTime();
        bool readHistograms( TDirectoryFile *iDir );
        void reset();
        void writeHistograms();
};
#endif
