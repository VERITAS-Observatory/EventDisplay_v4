//! VImageBaseAnalyzer   basic image analyzer routine (used by VImageAnalyzer and VCalibrator)

#ifndef VIMAGEBASEANALYZER_H
#define VIMAGEBASEANALYZER_H

#include <VEvndispData.h>

#include "TTree.h"

#include <valarray>
#include <vector>

using namespace std;

class VImageBaseAnalyzer : public VEvndispData
{
    protected:
        vector<bool> fCalibrated;                 //!  true = calibration is done
        bool fRaw;

        void calcSecondTZerosSums();
        void calcSums(int ,int, bool);
        void calcTZeros(int ,int);
        void calcTZerosSums(int, int, unsigned int );
	unsigned int getDynamicSummationWindow( unsigned int chanID );
        void FADCStopCorrect();
        bool setSpecialChannels();
        void timingCorrect();
        TTree* makeDeadChannelTree();

    public:
        VImageBaseAnalyzer() {}
       ~VImageBaseAnalyzer() {}

        void  calcTCorrectedSums(int ,int);
        int   fillHiLo();                          //!< fill hi/low gain vector
	int   fillSaturatedChannels();
        void  findDeadChans( bool iLowGain = false, bool iFirst = true );
        void  gainCorrect();
        float getPhotoDiodeMax();
        float getPhotoDiodeSum();
};
#endif
