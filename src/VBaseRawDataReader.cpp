/*! \class VBaseRawDataReader

    contains common elements for VRawDataReader and VBFDataReader

    Revision $Id: VBaseRawDataReader.cpp,v 1.16.8.2.12.2.8.5.2.1.4.6.2.1.2.2.2.2.2.2 2011/02/12 20:36:33 gmaier Exp $

    \author
      Gernot Maier
      Filip Pizlo

*/

#include <VRawDataReader.h>

VBaseRawDataReader::VBaseRawDataReader( string sourcefile, int isourcetype, unsigned int iNTel, bool iDebug )
{
    fDebug = iDebug;
    if( fDebug ) cout << "VBaseRawDataReader::VBaseRawDataReader" << endl;
    if( fDebug ) setSimuDebugFlag();
    fDataFormat = "rawdata";
    fDataFormatNum = 0;
    fEventNumber = 0;
    fNTel = iNTel;
    fTelID = 0;
    fMonteCarloHeader = 0;
    fNoiseFileReader = 0;
    if( isourcetype == 2 )
    {
        fDataFormat = "MCvbf";
        fDataFormatNum = 2;
    }
    else if( isourcetype == 3 )
    {
        fDataFormat = "Rawvbf";
        fDataFormatNum = 3;
    }
    fSourceFileName = sourcefile;
    for( unsigned int i = 0; i < fNTel; i++ )
    {
        fEvent.push_back( 0 );
    }
}


void VBaseRawDataReader::setDebug( bool iDebug )
{
    fDebug = iDebug;
    if( fDebug ) setSimuDebugFlag();
}


bool VBaseRawDataReader::isMC()
{
    if( fDataFormat == "MCvbf" ) return true;

    return false;
}


VBaseRawDataReader::~VBaseRawDataReader()
{
    if( fDebug ) cout << "VBaseRawDataReader::~VBaseRawDataReader()" << endl;
/*   delete fEvent; */
}


/*!
    1 = rawdata
    3 = MCvbf
*/
unsigned int VBaseRawDataReader::getDataFormatNum()
{
    return fDataFormatNum;
}


bool VBaseRawDataReader::setTelescopeID( unsigned int iTelID )
{
    if( iTelID < fNTel ) fTelID = iTelID;
    else
    {
        fTelID = 0;
        return false;
    }
    return true;
}


uint32_t VBaseRawDataReader::getHitID(uint32_t i)
{
    if( !fEvent[fTelID] ) return 0;
    uint32_t t_hitid = 0;
    try
    {
        t_hitid = fEvent[fTelID]->getHitID( i );
    }
    catch(VIndexOutOfBoundsException &ex)
    {
        cout << "VRawDataReader::getHitID error " << ex << endl;
        return 0;
    }
    return t_hitid;
}


uint16_t VBaseRawDataReader::getMaxChannels()
{
    if( !fEvent[fTelID] ) return 499;
    if( fTelID < fEvent.size() )
    {
        if( fEvent[fTelID] )
        {
            return fEvent[fTelID]->getMaxChannels();
        }
    }
    else cout << "VBaseRawDataReader::getMaxChannels(): problem with event size " << fTelID << "\t" << fEvent.size() << endl;
    return 499;
}


uint16_t VBaseRawDataReader::getNumSamples()
{
    if( !fEvent[fTelID] ) return 128;
    if( fTelID < fEvent.size() )
    {
        if( fEvent[fTelID] )
        {
            return fEvent[fTelID]->getNumSamples();
        }
    }
    return 128;
} 


uint8_t VBaseRawDataReader::getNewEventType( unsigned int itelID )
{
    if( !fEvent[itelID] ) return 0;

    if( itelID < fEvent.size() ) return fEvent[itelID]->getEventType().getBestNewStyleCode();

    return 0;
}


uint8_t VBaseRawDataReader::getEventType()
{
    if( fEvent[fTelID] ) return fEvent[fTelID]->getEventType().getBestOldStyleCode();

    return 0;
}


uint8_t VBaseRawDataReader::getNewEventType()
{
    if( fEvent[fTelID] ) return fEvent[fTelID]->getEventType().getBestNewStyleCode();

    return 0;
}


int VBaseRawDataReader::getNumberofFullTrigger()
{
    int z = 0;
    if( fEvent[fTelID] )
    {
        unsigned int i_maxchannel = getMaxChannels();
        for (unsigned i=0; i < i_maxchannel;i++ )
        {
            if( fEvent[fTelID]->getTriggerBit(i) ) z++;
        }
    }
    return z;
}


uint16_t VBaseRawDataReader::getGPSYear()
{
    if( fEvent[fTelID] )
    {
        if( getRunNumber() < 33242 || getRunNumber() > 33253 )
        {
            return fEvent[fTelID]->getGPSYear();
        }
        else
        {
            return fEvent[fTelID]->getGPSYear()+1;
        }
    }
    return 50;
}


bool VBaseRawDataReader::wasLossyCompressed()
{
#ifdef VBF_027
    if( fEvent[fTelID] ) return fEvent[fTelID]->wasLossyCompressed();
    else                 return false;
#endif

    return false;
}


bool VBaseRawDataReader::getHiLo(uint32_t i)
{
    if( fEvent[fTelID] )
    {
        try
        {
            return fEvent[fTelID]->getHiLo(i);
        }
        catch( VException &e )
        {
            cout << "EXCEPTION " << i << endl;
            return false;
        }
    }

    return false;
}


bool VBaseRawDataReader::initTraceNoiseGenerator( unsigned int iType, string iT, VDetectorGeometry* iD, vector<int> iSW, bool iDebug, int iseed, double iDefaultPed, vector<double> iFADCCorrect )
{
    if( fDebug ) cout << "VBaseRawDataReader::initTraceNoiseGenerator " << endl;
    fNoiseFileReader = new VNoiseFileReader( iType, iT );

// preliminary: use value from Telescope 1 for all telescopes
    double iCorrection = 1.;
    if( iFADCCorrect.size() > 0 && iFADCCorrect[0] > 0. )
    {
        iCorrection = 1.;
        if( TMath::Abs( iCorrection - 1. ) > 0.01 && iCorrection > 0. )
        {
            cout << "init trace noise generator: use gain correction from telescope 1 for all telescope (" << 1./iCorrection << ")" << endl;
        }
    }
    bool iB = fNoiseFileReader->init( iD, iD->getNumTelescopes(), iSW, iDebug, iseed, iCorrection );
    fNoiseFileReader->setDefaultGrisuPed( iDefaultPed );

    return iB;
}


uint8_t VBaseRawDataReader::getSample( unsigned channel, unsigned sample, bool iNewNoiseTrace )
{
    if( !fNoiseFileReader )
    {
        if( fEvent[fTelID] )
        {
            return fEvent[fTelID]->getSample( channel, sample );
        }
    }
    else
    {
        if( fEvent[fTelID] )
        {
            return fEvent[fTelID]->getSample( channel, sample ) + fNoiseFileReader->getNoiseSample( fTelID, channel, sample, iNewNoiseTrace );
        }
    }
    return 0;
}


std::vector< uint8_t > VBaseRawDataReader::getSamplesVec()
{
// standard way
    if( !fNoiseFileReader )
    {
        if( fEvent[fTelID] )
        {
            return fEvent[fTelID]->getSamplesVec();
        }
        else
        {
            return fDummyUint8V;
        }
    }
    else
    {
        if( fEvent[fTelID] )
        {
            std::vector< uint8_t > i_temp = fEvent[fTelID]->getSamplesVec();
            std::vector< uint8_t > i_pedV = fNoiseFileReader->getNoiseVec( fTelID, fHitID );
            if( i_temp.size() == i_pedV.size() )
            {
                for( unsigned int i = 0; i < i_temp.size(); i++ )
                {
                    i_temp[i] += i_pedV[i];
                }
            }
            return i_temp;
        }
    }
    return fDummyUint8V;
}


void VBaseRawDataReader::selectHitChan( uint32_t i)
{
    fHitID = i;

    if( fEvent[fTelID] ) return fEvent[fTelID]->selectHitChan( i );

    return;
}


void VBaseRawDataReader::setSumWindow( unsigned int iTelID, int isw )
{
    if( fNoiseFileReader ) fNoiseFileReader->setSumWindow( iTelID, isw );
}


valarray<double>& VBaseRawDataReader::getPeds()
{
    if( fNoiseFileReader ) return fNoiseFileReader->getPeds();

    return v;
}


valarray<double>& VBaseRawDataReader::getPedvars()
{
    if( fNoiseFileReader ) return fNoiseFileReader->getPedvars();

    return v;
}


vector< valarray<double> >& VBaseRawDataReader::getPedvarsAllSumWindows()
{
    if( fNoiseFileReader ) return fNoiseFileReader->getPedvarsAllSumWindows();

    return vv;
}


valarray<double>& VBaseRawDataReader::getPedRMS()
{
    if( fNoiseFileReader ) return fNoiseFileReader->getPedRMS();

    return v;
}
