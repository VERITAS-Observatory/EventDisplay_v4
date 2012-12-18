/*! \class VCameraRead
    \brief reading of telescope/camera geometry file (.cam and GrIsu .cfg files)

    all values in these class are static and should not change during a run

    observe that for EVNDISP version > 3 only .cfg are supported

Revision $Id: VCameraRead.cpp,v 1.18.2.1.4.2.4.1.4.3.10.7.2.8.2.1.2.1.4.7.2.2.2.2 2011/04/06 11:57:56 gmaier Exp $

\author Gernot Maier
*/

#include "VCameraRead.h"

VCameraRead::VCameraRead()
{
    fDebug = false;
    fNTel = 0;
    fTelID = 0;
// configuration file type
    fCFGtype = 0;
// pixel type
    fPixelType = 1;
// maximal number of neighbours is 6 (for circular pixel type)
    fMaxNeighbour = 6;
// default directory for cfg files
    fConfigDir = "../data/detector_geometry/";
// default pedestal
    fDefPed = 20.;
    fFADCRange = 256;
// default number of patches
    fNPatches = 91;
// default GrIsu version
    fGrIsuVersion = 500;
// low gain multiplier
    fLowGainIsSet = false;
// coordinate transformers
    setCoordinateTransformer( 1., 1. );
// default source type
    fsourcetype = 3;
}


/*!
     if telescope ID is out of range, ID is set to zero (good idea??)
*/
bool VCameraRead::setTelID( unsigned int iTel )
{
    if( iTel < fNTel ) fTelID = iTel;
    else
    {
        fTelID = 0;
        cout << " VCameraRead::setTelID() error: telescope ID out of range: " << iTel << "\t" << fTelID << "\t" << fNTel << endl;
//      cout << "                                setting ID to zero" << endl;
        cout << "   ...exiting" << endl;
        exit( -1 );
        return false;
    }
    return true;
}


bool VCameraRead::initialize( unsigned int iNtel, vector< string > iCamera )
{
    if( fDebug ) cout << "VCameraRead::initialize " << iNtel << endl;
    if( iNtel > iCamera.size() )
    {
        cout << "VCameraRead::initialize error: number of telescopes larger than camera vector ";
        cout << iNtel << "\t" << iCamera.size() << endl;
        return false;
    }
    fNTel = iNtel;
    resetTelVectors();
    fCameraName = iCamera;
// get number of channels
    for( unsigned int i = 0; i < fNTel; i++ )
    {
        string iCameraFile = fConfigDir + iCamera[i] + ".cam";
        ifstream inFileStream( iCameraFile.c_str() );
        if( !inFileStream )
        {
            cout << "VCameraRead::initialize camera geometry file not found: " << iCamera[i] << endl;
	    cout << iCameraFile << endl;
            return false;
        }
        string i_Line;
        unsigned int zaehler = 0;
        while( getline( inFileStream, i_Line ) )
        {
            zaehler++;
        }
        fCNChannels[i] = zaehler;
        inFileStream.close();
    }
    resetCamVectors();
    return true;
}


/*!
     initialization necessary for .cam files

*/
bool VCameraRead::initialize( unsigned int i_Ntel, unsigned int i_Nchannel )
{
    if( fDebug ) cout << "VCameraRead::initialize " << i_Ntel << endl;
    fNTel = i_Ntel;
    resetTelVectors();
    for( unsigned int i = 0; i < fNTel; i++ )
    {
        fCNChannels[i] = i_Nchannel;
        fCNSamples[i] = 128;                       // actual sample size is set later in VImageBaseAnalyzer (from reader)
    }
    resetCamVectors();
    return true;
}


bool VCameraRead::initialize( unsigned int i_Ntel, vector< unsigned int > i_Nchannel )
{
    if( fDebug ) cout << "VCameraRead::initialize " << i_Ntel << endl;
    fNTel = i_Ntel;
    resetTelVectors();
    for( unsigned int i = 0; i < fNTel; i++ )
    {
        fCNChannels[i] = i_Nchannel[i];
        fCNSamples[i] = 128;                       // actual sample size is set later in VImageBaseAnalyzer (from reader)
    }
    resetCamVectors( false );
    return true;
}


bool VCameraRead::readCameraFile( string iCameraFile, unsigned int i_Nchannel )
{
    if( fDebug ) cout << "VCameraRead::readCameraFile " << iCameraFile << "\t" << i_Nchannel << endl;
    initialize( 1, i_Nchannel );
    return readCameraFile( iCameraFile );
}


/*! read in camera geometry from .cam files

     \param iCameraFile file name for camera geometry file

     \attention
       - fine tuned to layout of .cam files
       - exit(-1) if camera geometry file not found

     \return true if reading succesful
*/
bool VCameraRead::readCameraFile( string iCameraFile )
{
    if( fDebug ) cout << "VCameraRead::readCameraFile " << iCameraFile << endl;
    iCameraFile.insert( 0, fConfigDir );
    iCameraFile.append( ".cam" );
    std::ifstream inFileStream( iCameraFile.c_str() );
    if( !inFileStream )
    {
        std::cout << "VCameraRead::readCameraFile() error: camera geometry file not found: " << iCameraFile << std::endl;
        exit( -1 );
        return false;
    }

    if( fCNChannels[fTelID] == 0 )
    {
        cout << "VCameraRead::readCameraFile() error: VCameraRead not initialized" << endl;
        return false;
    }

    string i_char;
    int i_ch;
    int i_mix;

    std::vector<int> i_temp;

    std::string i_Line;
    unsigned int zaehler = 0;
    while( getline( inFileStream, i_Line ) )
    {
        if( zaehler >= fCNChannels[fTelID] )
        {
            cout << "VCameraRead::readCameraFile() error: number of channels invalid" << endl;
            return false;
        }
        if( i_Line.size() > 0 )
        {
            std::istringstream is_stream( i_Line );
            is_stream >> i_char;
            is_stream >> i_ch;
            is_stream >> i_char;
            is_stream >> fXTube[fTelID][i_ch];
            is_stream >> i_char;
            is_stream >> fYTube[fTelID][i_ch];
            is_stream >> i_char;
            is_stream >> fRTube[fTelID][i_ch];
// reading neighbours
            is_stream >> i_char;
            unsigned int j = 0;
            while( !is_stream.eof() && i_char.substr( 0,1 ) == "N" )
            {
                if( j < fNeighbour[fTelID][i_ch].size() ) is_stream >> fNeighbour[fTelID][i_ch][j];
                is_stream >> i_char;
                j++;
            }
// maybe there is some information about triggers and dead channels
            if( !is_stream.eof() && i_char.substr( 0,4 ) == "TRIG"  )
            {
                is_stream >> fTrigTube[fTelID][i_ch];
            }
            if( !is_stream.eof() )
            {
                is_stream >> i_char;
                if( i_char.substr( 0, 3 ) == "ANA" )
                {
                    is_stream >> fAnaTube[fTelID][i_ch];
                }
            }
// get convertion from MC tube numbering to real data tube numbering
            if( !is_stream.eof() )
            {
                is_stream >> i_char;
                if( i_char.substr( 0, 3 ) == "MIX" )
                {
                    is_stream >> i_mix;
                    fXim.push_back( (unsigned int)i_mix);
                }
            }
            zaehler++;
        }
    }

    if( fXim.size() > 0 )
    {
// revert mixing vector
        unsigned int i_max = 0;
// ?? (GM)
        for( unsigned int i = 0; i < fXim.size(); i++ ) if( fXim[i] > i_max ) i_max = fXim[i];
        if( i_max > fXim.size() )
        {
            cout << "error in mixing vector size: " << i_max << "\t" << fXim.size() << endl;
            exit( -1 );
        }
        fMix.assign( fXim.size(), 0 );
        for( unsigned int i = 0; i < fXim.size(); i++ ) fMix[fXim[i]] = i;
// preliminary!!!
        fMix[0] = 0;
//   for( unsigned int i = 0; i < fXim.size(); i++ ) cout << i << "\t" << fXim[i] << "\t" << fMix[i] << endl;
    }

    inFileStream.close();

    return true;
}


/*!

    - use .cam file for trace library option (preliminary)

    \par iFile name of .cfg GrIsu detector configuration file

*/
bool VCameraRead::readGrisucfg( string iFile, unsigned int iNTel  )
{
    if( fDebug ) cout << "VCameraRead::readGrisucfg " << iFile << endl;
    fNTel = iNTel;

    iFile.insert( 0, fConfigDir );
    std::ifstream inFileStream( iFile.c_str() );
    if( !inFileStream )
    {
        cout << "VCameraRead::readGrisucfg() error: grisu cfg file not found: " << iFile << endl;
        exit( -1 );
        return false;
    }

    string iline = "";
    string i_char = "";
    unsigned int i_telID = 0;
    unsigned int i_telID_SIMU = 0;
    unsigned int i_chan = 0;
    unsigned int i_NN = 0;
    unsigned int i_NTelcfg = 0;

    while( getline( inFileStream, iline ) )
    {
// '*' in line
        if( iline.substr( 0, 1 ) != "*" ) continue;
        istringstream i_stream( iline );
// GrIsu version (4.0.0 = 400, 4.1.1 = 411)
        if( iline.find( "VERSN" ) < iline.size() )
        {
            if( iline.find( "." ) < iline.size() ) fGrIsuVersion = atoi( iline.substr( iline.find( "VERSN" )+5, iline.find( "." ) ).c_str() ) * 100;
            if( iline.rfind( "." ) < iline.size() ) fGrIsuVersion += atoi( iline.substr( iline.find( "." )+1, iline.rfind( "." )-iline.find( "." )-1 ).c_str() ) * 10;
            if( iline.rfind( "." ) < iline.size() ) fGrIsuVersion += atoi( iline.substr( iline.rfind( "." )+1, iline.size() ).c_str() );
	    cout << endl;
            cout << "reading detector configuration file (GrIsu version " << fGrIsuVersion << ")" << endl;
        }
// telescope file type
        if( iline.find( "CFGTYPE" ) < iline.size() )
        {
            i_stream >> i_char; i_stream >> i_char;
            i_stream >> fCFGtype;
            cout << "\t (file type " << fCFGtype << ")" << endl;
        }
// nubmer of telescopes
        if( iline.find( "NBRTL" ) < iline.size() )
        {
            i_stream >> i_char; i_stream >> i_char;
            i_stream >> i_NTelcfg;
            if( i_NTelcfg < fNTel ) fNTel = i_NTelcfg;
            if( fDebug ) cout << "VCameraRead: fNTel = " << fNTel << endl;
            resetTelVectors();
        }
// telescope IDs
// (used if telescope number in simulation is different then in eventdisplay or cfg file)
        else if( iline.find( "TELID" ) < iline.size() )
        {
            i_stream >> i_char;
            i_stream >> i_char;
            i_stream >> i_telID;
            if( i_telID > 0 ) i_telID -= 1;
            i_stream >> i_telID_SIMU;
            if( i_telID_SIMU > 0 ) i_telID_SIMU -= 1;
            fTelIDGrisu[i_telID] = i_telID_SIMU;
        }
// telescope positions
        else if( iline.find( "TLLOC" ) < iline.size() )
        {
            i_stream >> i_char; i_stream >> i_char;
            i_stream >> i_telID;
            if( i_telID > 0 ) i_telID -= 1;
            if( i_telID < fNTel )
            {
                i_stream >> fTelXpos[i_telID];
                i_stream >> fTelYpos[i_telID];
                i_stream >> fTelZpos[i_telID];
            }
            if( fDebug ) cout << "VCameraRead telescope positions: " << fTelXpos[i_telID] << "\t" << fTelYpos[i_telID] << "\t" << fTelZpos[i_telID] << " (id=" << i_telID << ")" << endl;
        }
// FADC
// (there is only one FADC record for all telescopes)
        else if( iline.find( "FADCS" ) < iline.size() )
        {
            i_stream >> i_char; i_stream >> i_char;
            i_stream >> i_char; i_stream >> i_char;
            i_stream >> fDefPed;
            i_stream >> fFADCRange;
//////////////////////////////////////////////
// IMPORTANT: IGNORING SAMPLE SETTINGS FROM CFG FILE HERE!!!! (GM)
            i_stream >> fCNSamples[0];
	    if( fsourcetype != 1 && fsourcetype != 5 ) fCNSamples[0] = 128;
            for( unsigned int i = 0; i < fNTel; i++ )  fCNSamples[i] = fCNSamples[0];
//////////////////////////////////////////////
// hi/lo gains
            if( fGrIsuVersion >= 411 )
            {
                i_stream >> i_char; i_stream >> i_char;
                if( !i_stream.eof() )
                {
                    i_stream >> fLowGainMultiplier[0];
                    for( unsigned int i = 0; i < fNTel; i++ ) fLowGainMultiplier[i] = fLowGainMultiplier[0];
                    i_stream >> fLowGainActivator[0];
                    for( unsigned int i = 0; i < fNTel; i++ ) fLowGainActivator[i] = fLowGainActivator[0];
                    fLowGainIsSet = true;
                }
            }
        }
// low gain multiplier (not a grisu line)
        else if( iline.find( "LOWMULT" ) < iline.size() )
        {
            i_stream >> i_char; i_stream >> i_char;
            i_stream >> i_telID;
            if( i_telID > 0 ) i_telID -= 1;
            if( i_telID < fNTel ) i_stream >> fLowGainMultiplier[i_telID];
            fLowGainIsSet = true;
        }
// mirror design
        else if( iline.find( "MIROR" ) < iline.size() )
        {
            i_stream >> i_char; i_stream >> i_char;
            i_stream >> i_telID;
            if( i_telID > 0 ) i_telID -= 1;
            if( i_telID < fNTel )
            {
                i_stream >> fTelRad[i_telID];
                i_stream >> fMirFocalLength[i_telID];
            }
            if( fDebug ) cout << "VCameraRead mirrors: " << i_telID+1 << " " << fTelRad[i_telID] << " " << fMirFocalLength[i_telID] << endl;
        }
// number of pixels
        else if( iline.find( "CAMRA" ) < iline.size() )
        {
            i_stream >> i_char; i_stream >> i_char;
            i_stream >> i_telID;
            if( i_telID > 0 ) i_telID -= 1;
            if( i_telID < fNTel )
            {
                i_stream >> fCNChannels[i_telID];
                fTelID = i_telID;
            }
            if( fDebug ) cout << "VCameraRead: total number of channels: " << fCNChannels[i_telID] << " (" << i_telID+1 << ")" << endl;
            if( fCFGtype == 1 ) for( unsigned int t = 1; t < fCNChannels.size(); t++ ) fCNChannels[t] = fCNChannels[0];
        }
// camera rotation (not a original grisu line)
        else if( iline.find( "CAROT" ) < iline.size() )
        {
            i_stream >> i_char; i_stream >> i_char;
            i_stream >> i_telID;
            if( i_telID > 0 ) i_telID -= 1;
            if( i_telID < fNTel )
            {
                i_stream >> fCameraRotation[i_telID];
                fTelID = i_telID;
            }
        }
// place scale change ( scale factor, offsets, camera rotation)
        else if( iline.find( "CAMPLATE" ) < iline.size() )
        {
            i_stream >> i_char; i_stream >> i_char;
            i_stream >> i_telID;
            if( i_telID > 0 ) i_telID -= 1;
            if( i_telID < fNTel )
            {
                i_stream >> fCameraScaleFactor[i_telID];
                i_stream >> fCameraCentreOffset[i_telID];
                i_stream >> fCameraRotation[i_telID];
            }
        }
// tube stuff
        else if( iline.find( "PMPIX" ) < iline.size() )
        {
            if( fXTubeMM.size() == 0 ) resetCamVectors();
            i_stream >> i_char; i_stream >> i_char;
            i_stream >> i_telID;
            if( i_telID > 0 ) i_telID -= 1;
            i_stream >> i_chan;
            if( i_chan > 0 ) i_chan -= 1;
            if( fGrIsuVersion >= 400 )
            {
                i_stream >> i_char;
                i_stream >> i_char;
            }
            if( i_telID < fNTel && i_chan < fCNChannels[i_telID] )
            {
                i_stream >> fXTubeMM[i_telID][i_chan];
                i_stream >> fYTubeMM[i_telID][i_chan];
                i_stream >> fRTubeMM[i_telID][i_chan];
                i_stream >> i_char; i_stream >> i_char;
                i_stream >> i_char; i_stream >> i_char;
                i_stream >> i_char; i_stream >> i_char;
                i_stream >> fTrigTube[i_telID][i_chan];
                i_stream >> fAnaTube[i_telID][i_chan];
                i_stream >> fTOff[i_telID][i_chan];
                i_stream >> fGain[i_telID][i_chan];
            }
        }
// neighbour list
        else if( iline.find( "NGHBR" ) < iline.size() )
        {
            i_stream >> i_char; i_stream >> i_char;
            vector< unsigned int > i_pixN;
            i_stream >> i_telID;
            if( i_telID > 0 ) i_telID -= 1;
            i_stream >> i_chan;
            if( i_chan > 0 ) i_chan -= 1;
            i_stream >> i_NN;
            if( i_telID < fNTel  && i_chan < fCNChannels[i_telID] ) fNNeighbour[i_telID][i_chan] = i_NN;
            if( i_NN > fMaxNeighbour )
            {
                cout << "VCameraRead::readGrisucfg() error: maximal number of neighbours wrong " << (int)i_NN << "\t" << fMaxNeighbour << endl;
                continue;
            }
            if( i_telID < fNTel && i_chan < fCNChannels[i_telID] )
            {
                for( unsigned int j = 0; j < fMaxNeighbour; j++ )
                {
                    if( !i_stream.eof() )
                    {
                        i_stream >> fNeighbour[i_telID][i_chan][j];
// grisu starts at 1 with counting, evndisp at 0
                        fNeighbour[i_telID][i_chan][j] -= 1;
                    }
                    else fNeighbour[i_telID][i_chan][j] = -1;
                }
            }
        }
// pattern trigger
        else if( iline.find( "PSTON" ) < iline.size() )
        {
            i_stream >> i_char; i_stream >> i_char;
            i_stream >> i_char;
            i_stream >> fNPatches;
        }
        else if( iline.find( "PATCH" ) < iline.size() )
        {
            i_stream >> i_char; i_stream >> i_char;
            i_stream >> i_char; i_stream >> i_char;
            vector< int > i_tPatch( 19, 0 );
            for( int i = 0; i < 19; i++ ) i_stream >> i_tPatch[i];
            fPatch.push_back( i_tPatch );
        }
        else if( iline.find( "PIXGB" ) < iline.size() )
        {
            if( fXTubeMM.size() == 0 ) resetCamVectors();
            i_stream >> i_char; i_stream >> i_char;
            i_stream >> i_char;
            fPixelType = (unsigned int)atoi( i_char.c_str() );
// square pixel allow 8 neighbours
            if( fPixelType == 3 )
            {
                fMaxNeighbour = 8;
            }
            i_stream >> i_char;
            i_stream >> i_char;
            float r = atof( i_char.c_str() );
            for( unsigned int c = 0; c < fCNChannels[i_telID]; c++ ) fRTubeMM[i_telID][c] = r;
        }
        else if( iline.find( "PIXFI" ) < iline.size() )
        {
            i_stream >> i_char; i_stream >> i_char;
            i_stream >> i_char;
            readPixelFile( i_char );
        }

    }
    if( fCFGtype == 1 ) fillTelescopeVectors();

// convert from mm to deg
    convertMMtoDeg();

// stretch and move camera
    stretchAndMoveCamera();

// rotate the camera
    rotateCamera();

// clean neighbour lists
    cleanNeighbourList();

    if( fDebug ) cout << "END: VCameraRead::readGrisucfg " << iFile << endl;

    return true;
}


void VCameraRead::cleanNeighbourList()
{
    for( unsigned int i = 0; i < fNeighbour.size(); i++ )
    {
        for( unsigned int j = 0; j < fNeighbour[i].size(); j++ )
        {
            if( fNNeighbour[i][j] != fNeighbour[i][j].size() )
            {
                fNeighbour[i][j].erase( fNeighbour[i][j].begin() + fNNeighbour[i][j], fNeighbour[i][j].end() );
            }
        }
    }
}


/*
    read external pixel file
*/
void VCameraRead::readPixelFile( string iFile )
{
    if( fDebug ) cout << "VCameraRead::readPixelFile " << iFile << endl;
    ifstream is;
    is.open( iFile.c_str(), ifstream::in);
    if( !is )
    {
        cout << "VCameraRead error: cannot find file with pixel information: " << iFile << endl;
        cout << "...exiting" << endl;
        exit( -1 );
    }
    string is_line;
    string itemp;
    unsigned int i_chan = 0;
    unsigned int i_telID = 0;
    unsigned int i_NN = 0;

    while( getline( is, is_line ) )
    {
        if( is_line.substr( 0, 1 ) != "*" ) continue;

        istringstream is_stream( is_line );
        is_stream >> itemp;
        is_stream >> itemp;
        if( itemp == "PIXLC" )
        {
            is_stream >> i_chan;
            i_chan -= 1;
            if( i_telID < fNTel  && i_chan < fCNChannels[i_telID] )
            {
                is_stream >> fXTubeMM[i_telID][i_chan];
                is_stream >> fYTubeMM[i_telID][i_chan];
                is_stream >> i_NN;
                if( i_telID < fNTel  && i_chan < fCNChannels[i_telID] ) fNNeighbour[i_telID][i_chan] = i_NN;
                if( i_NN > fMaxNeighbour )
                {
                    cout << "VCameraRead::readGrisucfg() error: maximal number of neighbours wrong " << (int)i_NN << "\t" << fMaxNeighbour << endl;
                    continue;
                }
                for( unsigned int j = 0; j < fMaxNeighbour; j++ )
                {
                    if( !is_stream.eof() )
                    {
                        if( j < fNeighbour[i_telID][i_chan].size() )
                        {
                            is_stream >> fNeighbour[i_telID][i_chan][j];
// grisu starts at 1 with counting, evndisp at 0
                            fNeighbour[i_telID][i_chan][j] -= 1;
                        }
                        else
                        {
                            int a = 0;
                            is_stream >> a;
                            fNeighbour[i_telID][i_chan].push_back( a - 1 );
                        }
                    }
                    else
                    {
                        if( j < fNeighbour[i_telID][i_chan].size() ) fNeighbour[i_telID][i_chan][j] = -1;
                        else                                         fNeighbour[i_telID][i_chan].push_back( -1 );
                    }
                }
            }
        }
    }
}


void VCameraRead::fillTelescopeVectors()
{
    if( fDebug ) cout << "VCameraRead::fillTelescopeVectors()" << endl;
    for( unsigned int t = 1; t < fTelRad.size(); t++ ) fTelRad[t] = fTelRad[0];
    for( unsigned int t = 1; t < fMirFocalLength.size(); t++ ) fMirFocalLength[t] = fMirFocalLength[0];
    for( unsigned int t = 1; t < fNMirrors.size(); t++ ) fNMirrors[t] = fNMirrors[0];
    for( unsigned int t = 1; t < fMirrorArea.size(); t++ ) fMirrorArea[t] = fMirrorArea[0];
    for( unsigned int t = 1; t < fCameraFieldofView.size(); t++ ) fCameraFieldofView[t] = fCameraFieldofView[0];
    for( unsigned int t = 1; t < fCNChannels.size(); t++ ) fCNChannels[t] = fCNChannels[0];
    for( unsigned int t = 1; t < fCameraScaleFactor.size(); t++ ) fCameraScaleFactor[t] = fCameraScaleFactor[0];
    for( unsigned int t = 1; t < fCameraCentreOffset.size(); t++ ) fCameraCentreOffset[t] = fCameraCentreOffset[0];
    for( unsigned int t = 1; t < fCameraRotation.size(); t++ ) fCameraRotation[t] = fCameraRotation[0];
    for( unsigned int t = 1; t < fXTubeMM.size(); t++ )
    {
        for( unsigned int i = 0; i < fXTubeMM[t].size(); i++ )
        {
            if( i < fXTubeMM[0].size() )  fXTubeMM[t][i] = fXTubeMM[0][i];
            if( i < fYTubeMM[0].size() )  fYTubeMM[t][i] = fYTubeMM[0][i];
            if( i < fRTubeMM[0].size() )  fRTubeMM[t][i] = fRTubeMM[0][i];
            if( i < fTrigTube[0].size() ) fTrigTube[t][i] = fTrigTube[0][i];
            if( i < fAnaTube[0].size() )  fAnaTube[t][i] = fAnaTube[0][i];
            if( i < fTOff[0].size() )     fTOff[t][i] = fTOff[0][i];
            if( i < fGain[0].size() )     fGain[t][i] = fGain[0][i];
        }
    }
    for( unsigned int t = 1; t < fNNeighbour.size(); t++ )
    {
        for( unsigned int i = 0; i < fNNeighbour[t].size(); i++ )
        {
            if( i < fNNeighbour[0].size() ) fNNeighbour[t][i] = fNNeighbour[0][i];
            for( unsigned int j = 0; j < fNeighbour[0][i].size(); j++ )
            {
                if( j < fNeighbour[t][i].size() ) fNeighbour[t][i][j] = fNeighbour[0][i][j];
                else                              fNeighbour[t][i].push_back( fNeighbour[0][i][j] );
            }
        }
    }
}

//void VCameraRead::print( bool bDetailed ){};
void VCameraRead::print( bool bDetailed )
{
  if( fDebug ) cout << "VCameraRead::print()" << endl;
    cout << "Number of telescopes: " << fNTel << endl;
    cout << "telescope positions: " << endl;
    for( unsigned int i = 0; i < fTelXpos.size(); i++ )
    {
        cout << "Telescope " << i+1 << " (type " << fTelType[i] << "): ";
        cout << "X: " << fTelXpos[i] << " [m], ";
        cout << "Y: " << fTelYpos[i] << " [m], ";
        cout << "Z: " << fTelZpos[i] << " [m], ";
        cout << "R: " << fTelRad[i] << " [m], ";
        cout << "Focal length: " << fMirFocalLength[i] << " [m], ";
        cout << "FOV: " << fCameraFieldofView[i] << " [deg], ";
        cout << "NChannel: " << fCNChannels[i];
// number of samples are determined with the first event, possible not known yet
//        cout << "NSamples: " << fCNSamples[i] << endl;
        if( fXTube.size() > 0 )
        {
	    if( bDetailed )
	    {
               cout << "\t Tube geometry: " << fXTube[i].size() << "\t" << fYTube[i].size() << "\t" << fRTube[i].size() << endl;
	       for( unsigned int j = 0; j < fXTube[i].size(); j++ )
	       {
		   cout << "\t\t" << i << "\t" << j << "\t";
		   cout << setw(6) << setprecision( 4 ) << fXTube[i][j] << "\t";
		   cout << setw(6) << setprecision( 4 ) << fYTube[i][j] << "\t";
		   cout << setw(6) << setprecision( 4 )<< fRTube[i][j] << "\t";
		   cout << fTrigTube[i][j] << "\t" << fAnaTube[i][j] << "\t";
		   cout << fTOff[i][j] << "\t" << fGain[i][j] << "\t";
		   cout << "N ";
		   cout << "(" << fNNeighbour[i][j] << "," << fNeighbour[i][j].size() << ") ";
		   for( unsigned int k = 0; k < fNeighbour[i][j].size(); k++ ) cout << fNeighbour[i][j][k] << "\t";
		   cout << endl;
	       }
	   }
	   else cout << endl;
       }
    }
}


void VCameraRead::convertMMtoDeg()
{
    if( fDebug ) cout << "VCameraRead::convertMMtoDeg" << endl;
    for( unsigned int i = 0; i < fNTel; i++ )
    {
        for( unsigned int j = 0; j < fXTube[i].size(); j++ )
        {
// transform coordinates
            fXTubeMM[i][j] *= fCoordinateTransformerX;
            fYTubeMM[i][j] *= fCoordinateTransformerY;

            fXTube[i][j] = atan2( (double)fXTubeMM[i][j] / 1000., (double)fMirFocalLength[i] ) * 45. / atan( 1. );
            fYTube[i][j] = atan2( (double)fYTubeMM[i][j] / 1000., (double)fMirFocalLength[i] ) * 45. / atan( 1. );
            fRTube[i][j] = atan2( (double)fRTubeMM[i][j] / 1000., (double)fMirFocalLength[i] ) * 45. / atan( 1. );
        }
    }
}


/*!
 *  stretch and move camera
 */
void VCameraRead::stretchAndMoveCamera()
{
    if( fDebug ) cout << "VCameraRead::stretchAndMoveCamera" << endl;

    cout << "camera plate scaled by";
// stretch
    for( unsigned int i = 0; i < fNTel; i++ )
    {
        cout << " T" << i+1 << ": " << fCameraScaleFactor[i];
        for( unsigned int j = 0; j < fXTube[i].size(); j++ )
        {
            fXTube[i][j] *= fCameraScaleFactor[i];
            fXTube[i][j] += fCameraCentreOffset[i];
            fYTube[i][j] *= fCameraScaleFactor[i];
            fYTube[i][j] += fCameraCentreOffset[i];
        }
    }
    cout << endl;
}


/*!
 *  rotate camera counterclockwise
 */
void VCameraRead::rotateCamera()
{
    if( fDebug ) cout << "VCameraRead::rotateCamera " << endl;
    double degrad = 45. / atan( 1. );

    for( unsigned int i = 0; i < fNTel; i++ )
    {
        double iR = fCameraRotation[i] / degrad;
	if( i < fXTube.size() && i < fYTube.size() && i < fRotXTube.size() && i < fRotYTube.size() )
	{
	   for( unsigned int j = 0; j < fXTube[i].size(); j++ )
	   {
	       if( j < fRotXTube[i].size() && j < fRotYTube[i].size() && j < fYTube[i].size() )
	       {
		  fRotXTube[i][j] = fXTube[i][j] * cos( iR ) - fYTube[i][j] * sin( iR );
		  fRotYTube[i][j] = fXTube[i][j] * sin( iR ) + fYTube[i][j] * cos( iR );
               }
	       else
	       {
		   cout << "VCameraRead::rotateCamera() error: invalid vector sizes (expeced " << i << "," << j << "): ";
		   cout << fXTube[i].size() << "\t" << fYTube[i].size() << "\t" << fRotXTube[i].size() << "\t" << fRotYTube[i].size() << endl;
		   cout << "exiting..." << endl;
		   exit( -1 );
	       }
	   }
        }
	else
	{
	   cout << "VCameraRead::rotateCamera() error: invalid vector sizes (expeced " << i << "): ";
	   cout << fXTube.size() << "\t" << fYTube.size() << "\t" << fRotXTube.size() << "\t" << fRotYTube.size() << endl;
	   cout << "exiting..." << endl;
	   exit( -1 );
        }
    }
}


/*!
 */
void VCameraRead::resetTelVectors()
{
    fCameraName.assign( fNTel, "camera" );
    fCameraScaleFactor.assign( fNTel, 1. );
    fCameraCentreOffset.assign( fNTel, 0. );
    fCameraRotation.assign( fNTel, 0. );
    fTelIDGrisu.clear();
    fTelType.clear();
//////////////////////////////////////////////////////////////////////////
// each telescope is different
    for( unsigned int i = 0; i < fNTel; i++ ) fTelType.push_back( i+1 );
// set fTelType to same value for similar telescopes
//////////////////////////////////////////////////////////////////////////
    fTelXpos.assign( fNTel, 0. );
    fTelYpos.assign( fNTel, 0. );
    fTelZpos.assign( fNTel, 0. );
    fTelRad.assign( fNTel, 7. );
    fCNChannels.assign( fNTel, 0 );
    fCNSamples.assign( fNTel, 128 );               // actual sample size is set later in VImageBaseAnalyzer
    fMirFocalLength.assign( fNTel, 12. );
    fNMirrors.assign( fNTel, 0 );
    fMirrorArea.assign( fNTel, 0. );
    fCameraFieldofView.assign( fNTel, 3.5 );
    fLowGainMultiplier.assign( fNTel, 6.0 );
    fLowGainActivator.assign( fNTel, 255 );
// set default values for array of four telescopes
//  later this values are overwritten by the values from the .cfg file
    if( fNTel == 4 )
    {
        fTelXpos[0] = 0.;   fTelXpos[1] = -69.282;  fTelXpos[2] = 69.282;  fTelXpos[3] =   0.;
        fTelYpos[0] = 0.;   fTelYpos[1] =  40.000;  fTelYpos[2] = 40.000;  fTelYpos[3] = -80.;
        fTelZpos[0] = 0.;   fTelZpos[1] =   0.000;  fTelZpos[2] =  0.000;  fTelZpos[3] =   0.;
    }
}


/*!
   2d vectors [telescope][pixel]
*/
void VCameraRead::resetCamVectors( bool bMaxN )
{
    vector< float > i_tel;
    vector< int > i_pix;
    vector< unsigned int > i_pixN;
    vector< vector< int > > i_N;
    for( unsigned int i = 0; i < fNTel; i++ )
    {
        i_tel.assign( fCNChannels[i], 0. );
        fXTube.push_back( i_tel );
        fYTube.push_back( i_tel );
        fRTube.push_back( i_tel );
        fRotXTube.push_back( i_tel );
        fRotYTube.push_back( i_tel );
        fXTubeMM.push_back( i_tel );
        fYTubeMM.push_back( i_tel );
        fRTubeMM.push_back( i_tel );
        fTOff.push_back( i_tel );
        i_tel.assign( fCNChannels[i], 1. );
        fGain.push_back( i_tel );
        i_pix.assign( fCNChannels[i], 1 );
        i_pixN.assign( fCNChannels[i], 1 );
        fTrigTube.push_back( i_pix );
        fAnaTube.push_back( i_pix );
    }
    resetNeighbourLists( bMaxN );
}


void VCameraRead::resetNeighbourLists( bool bMaxN )
{
    if( fDebug ) cout << "VCameraRead::resetNeighbourLists() " << fMaxNeighbour << endl;
    vector< int > i_pix;
    vector< unsigned int > i_pixN;
    vector< vector< int > > i_N;
    fNeighbour.clear();
    fNNeighbour.clear();
    for( unsigned int i = 0; i < fNTel; i++ )
    {
        if( bMaxN ) i_pix.assign( fMaxNeighbour, -1 );
        i_N.assign( fCNChannels[i], i_pix );
        if( bMaxN ) i_pixN.assign( fCNChannels[i], 1 );
        else        i_pixN.assign( fCNChannels[i], 0 );
        fNeighbour.push_back( i_N );
        fNNeighbour.push_back( i_pixN );
    }
}


bool VCameraRead::makeNeighbourList()
{
    for( unsigned int i = 0; i < fNTel; i++ )
    {
// check if all tubes have the same size
        double i_TubeRadius_0 = 0.;

        if( getTubeRadius_MM(i).size() > 0 )
        {
            i_TubeRadius_0 = getTubeRadius_MM( i )[0];
            for( unsigned int j = 0; j < getTubeRadius_MM( i ).size(); j++ )
            {
                if( TMath::Abs( getTubeRadius_MM( i )[j] - i_TubeRadius_0 ) > 1.e-2 )
                {
                    cout << "VCameraRead::makeNeighbourList error: found tubes with different sizes in camera " << i+1 << endl;
                    return false;
                }
            }
        }
        else continue;
// get minimum distance between tubes
        double iTubeDistance_min = 1.e5;
        for( unsigned int j = 0; j < getTubeRadius_MM( i ).size(); j++ )
        {
            for( unsigned int k = 0; k < j; k++ )
            {
                double itemp = sqrt( (getX_MM(i)[j]-getX_MM(i)[k])*(getX_MM(i)[j]-getX_MM(i)[k]) + (getY_MM(i)[j]-getY_MM(i)[k])*(getY_MM(i)[j]-getY_MM(i)[k]) );
                if( itemp < iTubeDistance_min ) iTubeDistance_min = itemp;
            }
        }
        iTubeDistance_min *= 0.5;

// now find all tubes which less then 2*iTubeDistance_min*sqrt(2) away (should work for grid)
// ignore all channels with
        for( unsigned int j = 0; j < getTubeRadius_MM( i ).size(); j++ )
        {
            for( unsigned int k = 0; k < j; k++ )
            {
                double itemp = sqrt( (getX_MM(i)[j]-getX_MM(i)[k])*(getX_MM(i)[j]-getX_MM(i)[k]) + (getY_MM(i)[j]-getY_MM(i)[k])*(getY_MM(i)[j]-getY_MM( i)[k]) );
                if( itemp < 2.1*sqrt(2.)*iTubeDistance_min )
                {
                    if( getAnaPixel( i )[k] > 0 )
                    {
                        fNNeighbour[i][j]++;
                        fNeighbour[i][j].push_back( k );
                        if( fNeighbour[i][j].size() > fMaxNeighbour ) fMaxNeighbour = fNeighbour[i][j].size();
                    }
                    if( getAnaPixel( i )[j] > 0 )
                    {
                        fNNeighbour[i][k]++;
                        fNeighbour[i][k].push_back( j );
                        if( fNeighbour[i][k].size() > fMaxNeighbour ) fMaxNeighbour = fNeighbour[i][j].size();
                    }
                }
            }
        }
    }
    return true;
}


float VCameraRead::getOuterEdgeDistance( unsigned int i )
{
    if( i > fXTube[fTelID].size()-1 )
    {
        cout << "VCameraRead::getOuterEdgeDistance(): channel index out of range, " << i << "\t" << fXTube[fTelID].size() << endl;
        return 0.;
    }

    double iDist = sqrt( fXTube[fTelID][i] * fXTube[fTelID][i] + fYTube[fTelID][i] * fYTube[fTelID][i] );
    iDist += fRTube[fTelID][i];

    return iDist;
}


bool VCameraRead::setLowGainMultiplier( unsigned int iTel, double ival )
{
    if( iTel < fLowGainMultiplier.size() ) fLowGainMultiplier[iTel] = ival;
    else
    {
        cout << "VCameraRead::setLowGainMultiplier: invalid low gain multiplier, set value to 1" << endl;
        return false;
    }
    return true;
}

bool VCameraRead::setLowGainThreshold( unsigned int iTel, unsigned int ival )
{
    if( iTel < fLowGainActivator.size() ) fLowGainActivator[iTel] = ival;
    else
    {
        cout << "VCameraRead::setLowGainThreshold: invalid low gain threshold, set value to 0" << endl;
        return false;
    }
    return true;
}


vector<ULong64_t> VCameraRead::getTelType_list()
{
   vector<ULong64_t> t;

   set< ULong64_t > s;

   for( unsigned int i = 0; i < fTelType.size(); i++ ) s.insert( fTelType[i] );

   set< ULong64_t >::iterator it_s;
   for( it_s = s.begin(); it_s != s.end(); it_s++ ) t.push_back( *it_s );

   return t;
}

/*!

     read varios detector parameters from the DB

*/
bool VCameraRead::readDetectorGeometryFromDB( string iDBStartTime, bool iReadRotationsFromDB )
{
  if(fDebug) cout << "VCameraRead::readDetectorGeometryFromDB" << endl;
    cout << "VCameraRead::readDetectorGeometryFromDB for " << iDBStartTime;
    if( iReadRotationsFromDB ) cout << " (read rotations)";
    cout << endl;

    if( iDBStartTime.size() < 8 )
    {
       cout << "VCameraRead::readDetectorGeometryFromDB error: no valid SQL data for getting DB detector geometry: " << iDBStartTime << endl;
       return false;
    }

// read camera rotations from DB
    if( iReadRotationsFromDB )
    {
       stringstream iTempS;
       iTempS << getDBServer() << "/VOFFLINE";
       TSQLServer *i_DB = TSQLServer::Connect( iTempS.str().c_str(), "readonly", "" );
       if( !i_DB )
       {
	   cout << "VDBRunInfo: failed to connect to database server" << endl;
	   cout << "\t server: " <<  iTempS.str() << endl;
	   return false;
       }
       char c_query[800];
       sprintf( c_query, "select telescope_id, version, pmt_rotation from tblPointing_Monitor_Camera_Parameters where start_date <= \"%s\" AND end_date > \"%s\" ", iDBStartTime.substr( 0, 10 ).c_str(), iDBStartTime.substr( 0, 10 ).c_str() );

       TSQLResult *db_res = i_DB->Query( c_query );
       if( !db_res ) return false;

       int iNRows = db_res->GetRowCount();
       vector< int > iVersion( fCameraRotation.size(), -99 );
       for( int j = 0; j < iNRows; j++ )
       {
          TSQLRow *db_row = db_res->Next();
	  if( !db_row ) continue;

	  int itelID = -99;
	  double iRot = -99.;
	  if( db_row->GetField( 0 ) )
	  {
	      itelID = atoi( db_row->GetField( 0 ) );
	  }
// check entry version
	  if( itelID >= 0 && itelID < (int)iVersion.size() )
	  {
              if( db_row->GetField( 1 ) && atoi( db_row->GetField( 1 ) ) > iVersion[itelID] )
	      {
		if( db_row->GetField( 2 ) )
		{
		    iRot = atof( db_row->GetField( 2 ) ) * TMath::RadToDeg();
		    if( itelID >=0 && itelID < (int)fCameraRotation.size() )
		    {
		       fCameraRotation[itelID] = -1.* iRot;
		    }
		}
		iVersion[itelID] = atoi( db_row->GetField( 1 ) );
              }
           }
       }
       i_DB->Close();
    }

    cout << "\t (rotations from DB [deg]: ";
    for( unsigned int i = 0; i < fCameraRotation.size(); i++ ) cout << " T" << i+1 << ": " << fCameraRotation[i];
    cout << ")" << endl;

// rotate the camera
    rotateCamera();
    if( fDebug ) cout << "rotateCamera() Finished" << endl;
    return true;
}
