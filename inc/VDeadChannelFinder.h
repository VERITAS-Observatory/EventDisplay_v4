//! VDeadChannelFinder  find problem channels (from peds, gain, etc.)

#ifndef VDeadChannelFinder_H
#define VDeadChannelFinder_H

#include <cmath>
#include <cstdlib>
#include <bitset>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class VDeadChannelFinder
{
    private:

        bool         fDebug;

        int          frunmode;
        unsigned int fTelID;
        bool         fLowGain;
        bool         fIsMC;

// dead channel definitions
        double fDEAD_ped_min;
        double fDEAD_ped_max;
        double fDEAD_pedvar_min;
        double fDEAD_pedvar_max;
        double fDEAD_peddev_min;
        double fDEAD_peddev_max;
        double fDEAD_gain_min;
        double fDEAD_gain_max;
        double fDEAD_gainvar_min;
        double fDEAD_gainvar_max;
        double fDEAD_gaindev_min;
        double fDEAD_gaindev_max;
        double fDEAD_toffset_max;
        double fDEAD_gainmult_min;
        double fDEAD_gainmult_max;

    public:

        VDeadChannelFinder( int irunmode = 0, unsigned int iTelID = 0, bool bLowGain = false, bool isMC = false );
        ~VDeadChannelFinder() {};
        void      printDeadChannelDefinition();
//     void      printChannelSummary( unsigned int, unsigned int );
        void      printSummary();
        bool      readDeadChannelFile( string iFile );
        unsigned int testGainMultiplier( unsigned int, double, double, double );
        unsigned int testGains( unsigned int, double );
        unsigned int testGainVariations( unsigned int,double );
        unsigned int testGainDev( unsigned int, double, double, bool );
        unsigned int testPedestals( unsigned int, double );
        unsigned int testPedestalVariations( unsigned int, double );
        unsigned int testPedestalVariationsMinOut( unsigned int, double, double, double );
        unsigned int testPedestalVariationsMaxOut( unsigned int, double, double, double );
        unsigned int testTimeOffsets( unsigned int, double );
};

#endif
