/*! \class VVirtualDataReader
    \brief wrapper classe for data reading

    Revision $Id: VVirtualDataReader.cpp,v 1.5.50.1 2011/04/21 10:03:37 gmaier Exp $

    \author Gernot Maier
*/

#include <VVirtualDataReader.h>

// VVirtualDataReader::VVirtualDataReader() throw (VFileException)
VVirtualDataReader::VVirtualDataReader()
{

}

double VVirtualDataReader::getSample_double( unsigned channel, unsigned sample, bool iNewNoiseTrace )
{
   if( has16Bit() ) return (double)getSample16Bit( channel, sample, iNewNoiseTrace );

   return (double)getSample( channel, sample, iNewNoiseTrace );
}
