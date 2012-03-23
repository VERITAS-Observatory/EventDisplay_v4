//! VDSTReader reader for data summary source files

#ifndef VDSTREADER_H
#define VDSTREADER_H

#include <VGlobalRunParameter.h>
#include <VDSTTree.h>
#include <VVirtualDataReader.h>

#include "TFile.h"
#include "TTree.h"

#include <bitset>
#include <iostream>
#include <valarray>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////
// MAXIMUM NUMBERS OF TELESCOPES AND CHANNELS ARE DEFINED IN EVNDISP_definition.h
///////////////////////////////////////////////////////////////////////////////////

class VDSTReader : public VVirtualDataReader
{
    private:
        bool fDebug;
        string fSourceFileName;
        TFile *fDSTfile;                          //!< dst source file
        unsigned int fDSTtreeEvent;               //!< tree event number
        VDSTTree *fDSTTree;                       //!< data tree
        bool   fMC;                               //!< source data is Monte Carlo

	bool   fPerformFADCAnalysis;              //!< look at FADC traces

        unsigned int fTelID;
        unsigned int fNTelescopes;
	uint16_t     fNumSamples;
        unsigned int fSelectedHitChannel;
        vector< unsigned int > fNChannel;
        vector< valarray< double > > fSums;
	vector< vector< valarray< double > > > fTracePulseTiming;
        vector< vector < unsigned int > > fDead;
        vector< valarray< double > > fTraceMax;
        vector< valarray< double > > fRawTraceMax;
        vector< float > fLTtime;
        vector< float > fLDTtime;
        vector< double > fTelAzimuth;
        vector< double > fTelElevation;

        vector< vector< bool > > fFullHitVec;
        vector< vector< bool > > fFullTrigVec;
        vector< vector < bool > > fHiLo;
        vector< int > fNumberofFullTrigger;

        vector< uint8_t > fDummySample;
        vector< uint16_t > fDummySample16Bit;
        vector< vector< vector< uint16_t > > > fFADCTrace;

        vector< bool > fDSTvltrig;

        bool init();                              //!< open source file and init tree

    public:
        VDSTReader( string isourcefile, bool iMC, int iNTel, int iNChannel, bool iDebug );
        ~VDSTReader() {}
        std::pair< bool, uint32_t > getChannelHitIndex( uint32_t i_channel );
        string    getDataFormat() { return "DST"; }
        unsigned int getDataFormatNum() { return 4; }
        vector<unsigned int>&       getDead() { return fDead[fTelID]; }
        uint32_t  getEventNumber() { return fDSTTree->getDSTEventNumber(); }
        uint8_t   getEventType() { return fDSTTree->getDSTEventType(); }
        uint8_t   getATEventType() { return fDSTTree->getDSTEventType(); }
        vector< bool > getFullHitVec() { return fFullHitVec[fTelID]; }
        vector< bool >         getFullTrigVec() { return fFullTrigVec[fTelID]; }
        int                    getNumberofFullTrigger() { return fNumberofFullTrigger[fTelID]; }
        uint32_t  getGPS0() { return fDSTTree->getDSTGPS0(); }
        uint32_t  getGPS1() { return fDSTTree->getDSTGPS1(); }
        uint32_t  getGPS2() { return fDSTTree->getDSTGPS2(); }
        uint32_t  getGPS3() { return fDSTTree->getDSTGPS3(); }
        uint32_t  getGPS4() { return fDSTTree->getDSTGPS4(); }
        uint16_t  getGPSYear() { return fDSTTree->getDSTGPSYear(); }
        uint16_t  getATGPSYear() { return fDSTTree->getDSTATGPSYear(); }
        uint32_t  getHitID( uint32_t );
        bool                        getHiLo(uint32_t i) { if( i < fHiLo[fTelID].size() ) return fHiLo[fTelID][i]; else return 0; }
        vector< bool >&             getLocalTrigger() { return fDSTvltrig; }
        vector< float >             getLocalTriggerTime() { return fLTtime; }
        vector< float >             getLocalDelayedTriggerTime() { return fLDTtime; }
        uint16_t  getMaxChannels() { return fNChannel[fTelID]; }
        uint16_t  getNumChannelsHit()             //!< preli
        {
            return fNChannel[fTelID];
        }
        uint16_t  getNumSamples()                 
        {
            return fNumSamples;
        }
        TTree*    getMCTree();
        int       getMC_primary()                 //!< MC primary type
        {
            return fDSTTree->getDSTMCPrimary();
        }
        float     getMC_energy()                  //!< MC primary energy
        {
            return fDSTTree->getDSTMCEnergy();
        }
        float     getMC_X()                       //!< MC x-coordinate of impact point on ground plane
        {
            return fDSTTree->getDSTMCxcore();
        }
        float     getMC_Y()                       //!< MC y-coordinate of impact point on ground plane
        {
            return fDSTTree->getDSTMCycore();
        }
        float     getMC_Xcos()                    //!< MC x direction cosine of primary in ground coordinate system
        {
            return fDSTTree->getDSTMCxcos();
        }
        float     getMC_Ycos()                    //!< MC y direction cosine of primary in ground coordinate system
        {
            return fDSTTree->getDSTMCycos();
        }
        float     getMC_Ze()                      //!< MC zenith angle of primary
        {
            return fDSTTree->getDSTMCze();
        }
        float     getMC_Az()                      //!< MC azimuth angle of primary
        {
            return fDSTTree->getDSTMCaz();
        }
        float     getMC_Xoffset()                 //!< MC x coordinate of source location in degrees
        {
            return fDSTTree->getDSTMCxoff();
        }
        float     getMC_Yoffset()                 //!< MC x coordinate of source location in degrees
        {
            return fDSTTree->getDSTMCyoff();
        }
        bool         getNextEvent();
	VMonteCarloRunHeader*         getMonteCarloHeader();
        unsigned int                  getNumTelescopes() { return fNTelescopes; }
        unsigned int                  getNTelLocalTrigger() { return fDSTTree->getDSTNLocalTrigger(); }
        unsigned int                  getNTel() { return fNTelescopes; }
        uint32_t                      getRunNumber() { return fDSTTree->getDSTRunNumber(); }
        vector< uint8_t >             getSamplesVec();
	uint8_t                       getSample( unsigned channel, unsigned sample, bool iNewNoiseTrace = true );
        vector< uint16_t >            getSamplesVec16Bit();
	uint16_t                      getSample16Bit( unsigned channel, unsigned sample, bool iNewNoiseTrace = true );
        valarray< double >&           getSums() { return fSums[fTelID]; }
        string                        getSourceFileName() { return fSourceFileName; }
        vector< double >              getTelAzimuth() { return fTelAzimuth; }
        vector< double >              getTelElevation() { return fTelElevation; }
        unsigned int                  getTelescopeID() { return fTelID; }
        valarray< double >&           getTraceMax() { return fTraceMax[fTelID]; }
        valarray< double >&           getTraceRawMax() { return fRawTraceMax[fTelID]; }
	vector< valarray< double > >& getTracePulseTiming() { return fTracePulseTiming[fTelID]; }
	bool      has16Bit()  { return true; }
	bool      hasFADCTrace() { if( fDSTTree ) return fDSTTree->getFADC(); else return false; }
        bool      hasLocalTrigger( unsigned int iTel ) { if( fDSTTree->hasLocalTrigger( iTel ) < 0 ) return false; else return true; }
        bool      isMC() { return fMC; }
        void      selectHitChan( uint32_t hit ) { fSelectedHitChannel = hit; }
	void      setNumSamples( uint16_t iS ) { fNumSamples = iS; }
	void      setPerformFADCAnalysis( bool iB ) { fPerformFADCAnalysis = iB; }
        bool      setTelescopeID( unsigned int );
                                                  //!< set trigger values
        void      setTrigger( vector<bool> iImage, vector<bool> iBorder );
        bool      wasLossyCompressed() { return false; }
};
#endif
