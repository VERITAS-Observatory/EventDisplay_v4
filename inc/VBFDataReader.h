//! VBFDataReader steering class specifically for reading VBF data
// Revision $Id: VBFDataReader.h,v 1.10.36.1.4.1.4.2 2011/02/12 20:36:33 gmaier Exp $

#ifndef VBFDATAREADER_H
#define VBFDATAREADER_H

#include "VBaseRawDataReader.h"

#include <VBankFileReader.h>
#include <VPacket.h>

#include <bitset>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class VBFDataReader : public VBaseRawDataReader
{
    protected:
        VPacket *pack;
        VBankFileReader reader;
        unsigned index;

        VArrayEvent   *ae;
        VArrayTrigger *at;

        bool fArrayTrigger;

        vector< bool > ib_temp;

    public:
        VBFDataReader( string,
            int isourcetype,
            unsigned int iNTel,
            bool iDebug );

        virtual ~VBFDataReader();

        unsigned int getNTel();

        bool hasAT()
        {
            return at!=NULL;
        }

        uint8_t getATEventType()
        {
            if (at==NULL)
            {
                return getEventType();
            }
            else
            {
                return at->getEventType().trigger;
            }
        }

        uint16_t getATGPSYear();

        VArrayEvent*      getArrayEvent() { return ae; }
        VArrayTrigger*    getArrayTrigger() { return at; }
        vector< bool >&   getLocalTrigger();
        bool              getNextEvent();
        unsigned int      getNTelLocalTrigger();
	uint16_t          getNumSamples();
        bool              hasArrayTrigger();
        bool              hasLocalTrigger( unsigned int iTel );
	void              setPerformFADCAnalysis( bool iB ) { ; }
};
#endif
