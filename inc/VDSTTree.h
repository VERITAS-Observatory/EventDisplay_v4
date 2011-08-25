//! VDSTTree   data summarizer class (stores pixel sums and times in a tree)
#ifndef VDSTTree_H
#define VDSTTree_H

#include "TMath.h"
#include "TTree.h"

#include <bitset>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

#include <VGlobalRunParameter.h>

///////////////////////////////////////////////////////////////////////////////////
// MAXIMUM NUMBERS OF TELESCOPES AND CHANNELS ARE DEFINED IN EVNDISP_definition.h
///////////////////////////////////////////////////////////////////////////////////

using namespace std;

class VDSTTree
{
    public:

        TTree *fDST_tree;
        TTree *fDST_conf;
        TTree *fMCtree;

                                                  // [telID] = FOV
        map< unsigned int, float > fDST_list_of_telescopes;
//      map< unsigned int, unsigned int > fTelescope_type_list;  // [telID] = telescope type (=
                                                  // [#] = telID
        vector< unsigned int > fDST_vlist_of_telescopes;

        bool fMC;
        bool fFullTree;

        unsigned int fDSTnchannel[VDST_MAXTELESCOPES];

        unsigned int fDSTrunnumber;
        unsigned int fDSTeventnumber;
        unsigned int fDSTeventtype;
        unsigned int fDSTgps0;
        unsigned int fDSTgps1;
        unsigned int fDSTgps2;
        unsigned int fDSTgps3;
        unsigned int fDSTgps4;
        unsigned int fDSTgpsyear;
        unsigned int fDSTATgpsyear;
// triggered telescopes
        unsigned int fDSTLTrig;
        unsigned int fDSTNTrig;
        unsigned int fDSTLTrig_list[VDST_MAXTELESCOPES];
// maximum number of telescopes is VDST_MAXTELESCOPES
// maximum number of channels per camera VDST_MAXCHANNELS
        unsigned int   fDSTntel;
        unsigned int   fDSTntel_data;
        unsigned int   fDSTtel_data[VDST_MAXTELESCOPES];
        float fDSTpointAzimuth[VDST_MAXTELESCOPES];
        float fDSTpointElevation[VDST_MAXTELESCOPES];

        unsigned short int   fDSTChan[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
        float fDSTsums[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
        unsigned short int fDSTdead[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
        unsigned short int fDSTsumwindow[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
        unsigned short int fDSTsumfirst[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
                                                  // (float) x 10.
        float fDSTt0[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
        short fDSTMax[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
// assume that all pulse timing levels are the same for all channels in a telescope
	float fDSTpulsetiminglevels[VDST_MAXTELESCOPES][VDST_MAXTIMINGLEVELS];
	float fDSTpulsetiming[VDST_MAXTELESCOPES][VDST_MAXTIMINGLEVELS][VDST_MAXCHANNELS];
        short int fDSTRawMax[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
        float fDSTLTtime[VDST_MAXTELESCOPES];
        float fDSTLDTtime[VDST_MAXTELESCOPES];
        unsigned short int fDSTHiLo[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
        unsigned short int fDSTN255[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
        unsigned short int fDSTnL1trig[VDST_MAXTELESCOPES];
        unsigned short int fDSTL1trig[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
// photodiode
        float fDSTPDMax[VDST_MAXTELESCOPES];
        float fDSTPDSum[VDST_MAXTELESCOPES];
// fit parameter
        float fDSTChi2[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
                                                  //!< rise time 10-90%
        float fDSTRT[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
                                                  //!< fall time 10-90%
        float fDSTFT[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
                                                  //!< rise time parameter from fit
        float fDSTRTpar[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
                                                  //!< fall time parameter from fit
        float fDSTFTpar[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
                                                  //< trace width                
        float fDSTTraceWidth[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
                                                  //!< trace norm
        float fDSTTraceNorm[VDST_MAXTELESCOPES][VDST_MAXCHANNELS];
// MC parameters
        unsigned short int fDSTprimary;
        float fDSTenergy;
        float fDSTxcore;
        float fDSTycore;
        float fDSTaz;
        float fDSTze;
        float fDSTTel_xoff;
        float fDSTTel_yoff;

        VDSTTree();
        ~VDSTTree() {}
        map< unsigned int, float> getArrayConfig() { return fDST_list_of_telescopes; }
        TTree *getDSTTree() { return fDST_tree; }
        TTree *getMCTree()  { return fMCtree; }
        bool isMC() { return fMC; }
        bool initDSTTree( bool iFullTree = false, bool iPhotoDiode = false, bool iTraceFit = false );
        bool initDSTTree( TTree *t, TTree *c );
        bool initMCTree();
        map< unsigned int, float> readArrayConfig( string );
        map< unsigned int, unsigned int > readTelescopeTypeList( string );
        void resetDataVectors( unsigned int iCH = 0 );
        void setMC( bool iMC = true ) { fMC = iMC; }

// getters for all variables
        uint32_t     getDSTRunNumber() { return fDSTrunnumber; }
        uint32_t     getDSTEventNumber() { return fDSTeventnumber; }
        uint8_t      getDSTEventType() { return fDSTeventtype; }
        uint32_t     getDSTGPS0() { return fDSTgps0; }
        uint32_t     getDSTGPS1() { return fDSTgps1; }
        uint32_t     getDSTGPS2() { return fDSTgps2; }
        uint32_t     getDSTGPS3() { return fDSTgps3; }
        uint32_t     getDSTGPS4() { return fDSTgps4; }
        uint16_t     getDSTGPSYear() { return fDSTgpsyear; }
        uint16_t     getDSTATGPSYear() { return fDSTATgpsyear; }
        unsigned int getDSTNumTelescopes() { return fDSTntel; }
        unsigned int getDSTNTel() { return fDSTntel; }
        unsigned int getDSTNChannels( unsigned int iTel ) { if( iTel < getDSTNTel() ) return fDSTnchannel[iTel]; else return 0; }
        float        getDSTTelAzimuth( unsigned int iTel ) { if( iTel < getDSTNTel() ) return fDSTpointAzimuth[iTel]; else return 0.; }
        float        getDSTTelElevation( unsigned int iTel ) { if( iTel < getDSTNTel() ) return fDSTpointElevation[iTel]; else return 0.; }
        unsigned int getDSTNLocalTrigger() { return fDSTNTrig; }
        bool         getDSTLocalTrigger( int iTelID );
        float        getDSTLocalTriggerTime( int iTelID );
        float        getDSTLocalDelayedTriggerTime( int iTelID );

        double       getDSTSums( int iTelID, int iChannelID );
        double       getDSTMax( int iTelID, int iChannelID );
        double       getDSTRawMax( int iTelID, int iChannelID );
        double       getDSTWidth( int iTelID, int iChannelID );
        double       getDSTTZeros( int iTelID, int iChannelID );
	unsigned int getDSTpulsetiminglevelsN();
	double       getDSTpulsetiming( int iTelID, int iChannelID, int iTimingLevelN );
        unsigned int getDSTDead( int iTelID, int iChannelID );
        UShort_t     getDSTHiLo( int iTelID, int iChannelID );
        unsigned int getNTrigL1( unsigned int iTelID ) { if( iTelID < getDSTNTel() ) return fDSTnL1trig[iTelID]; else return 0; }
        unsigned int getTrigL1( int iTelID, int iChannelID );

        unsigned short int getDSTMCPrimary() { return  fDSTprimary; }
        float        getDSTMCEnergy()             // [TeV]
        {
            return fDSTenergy;
        }
// VERITAS coordinate system: x -> east, y-> north
        float        getDSTMCxcore()              // [m]
        {
            return fDSTxcore;
        }
        float        getDSTMCycore()              // [m]
        {
            return fDSTycore;
        }
        float        getDSTMCaz();                // [deg]
        float        getDSTMCze()                 // [deg]
        {
            return fDSTze;
        }
        float        getDSTMCxcos() { return TMath::Cos( fDSTze*atan(1.)/45. ) * TMath::Cos( fDSTaz*atan(1.)/45. ); }
        float        getDSTMCycos() { return TMath::Cos( fDSTze*atan(1.)/45. ) * TMath::Sin( fDSTaz*atan(1.)/45. ); }
        float        getDSTMCxoff()               // offset of source from camera centre (in camera coordinates) [deg]
        {
            return fDSTTel_xoff;
        }
        float        getDSTMCyoff()               // offset of source from camera centre (in camera coordinates) [deg]
        {
            return fDSTTel_yoff;
        }

        int          hasLocalTrigger( int iTelID );
        int          hasData( int iTelID );

};
#endif
