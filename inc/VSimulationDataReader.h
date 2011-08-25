//! VSimulationDataReader reads simulations packages from vbf files

#ifndef VSIMULATIONDATAREADER_H
#define VSIMULATIONDATAREADER_H

#include "TMath.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include <VArrayEvent.h>
#include <VMonteCarloRunHeader.h>
#include <VPacket.h>
#include <VRawEventData.h>
#include <VSimulationData.h>

#include "VSkyCoordinates.h"

using namespace std;

class VSimulationDataReader
{
    private:
        VSimulationData *fSimulationData;
        bool fDebug;

        uint32_t  fEventNumber;
        long      fRunNumber;
        int    fMCprimary;
        float  fMCenergy;
        float  fMCX;
        float  fMCY;
        float  fMCXcos;
        float  fMCYcos;
        float  fMCZe;
        float  fMCAz;
        float  fMCXoff;
        float  fMCYoff;
        double fTel_Elevation;
        double fTel_Azimuth;

        vector< bool > fLocalTrigger;

    public:
        VSimulationDataReader();
        ~VSimulationDataReader() {}

        vector< bool >&            getSLocalTrigger() { return fLocalTrigger; }
        uint32_t                   getSMC_eventNumber() { return fEventNumber; }
        long                       getSMC_runNumber() { return fRunNumber; }
        int                        getSMC_primary() { return fMCprimary; }
        float                      getSMC_energy() { return fMCenergy; }
        float                      getSMC_X() { return fMCX; }
        float                      getSMC_Y() { return fMCY; }
        float                      getSMC_Xcos() { return fMCXcos; }
        float                      getSMC_Ycos() { return fMCYcos; }
        float                      getSMC_Ze()    // return shower direction
        {
            return fMCZe;
        }
        float                      getSMC_Az()    // return shower direction
        {
            return fMCAz;
        }
                                                  // offset in camera coordinates [deg]
        float                      getSMC_Xoffset()
        {
            return  fMCXoff;
        }
                                                  // offset in camera coordinates [deg]
        float                      getSMC_Yoffset()
        {
            return  fMCYoff;
        }
        double                     getSMC_TelPointing_Elevation() { return fTel_Elevation; }
        double                     getSMC_TelPointing_Azimuth() { return fTel_Azimuth; }
	VMonteCarloRunHeader*      fillSimulationHeader( VPacket* packet );
        bool                       printSimulationHeader( VPacket* packet, bool bPrintCFG = false );
        bool                       setSimulationData( VRawEventData* iraweventData );
        bool                       setSimulationData( VPacket* packet );

        void                       setSimuDebugFlag() { fDebug = true; }
};
#endif
