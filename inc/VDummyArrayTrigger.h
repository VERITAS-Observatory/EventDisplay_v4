//! VDummyArrayTrigger dummy replacement for VArrayTrigger (for compilation without vbf)
// Revision $Id: VDummyArrayTrigger.h,v 1.1.2.1 2011/04/21 10:03:37 gmaier Exp $

#ifndef VDummyArrayTrigger_H
#define VDummyArrayTrigger_H

#include <stdint.h>

class VDummyArrayTrigger
{
    public:

    uint32_t getEventNumber() { return 0; }
    uint8_t  getNumSubarrayTelescopes() { return 0; }
    uint32_t getSubarrayTelescopeId( unsigned index ) { return 0; } 
    float    getAltitude( unsigned index ) { return 0.; }
    float    getAzimuth( unsigned index ) { return 0.; }

    VDummyArrayTrigger() {}
   ~VDummyArrayTrigger() {}
};
#endif
