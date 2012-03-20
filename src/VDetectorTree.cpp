/*! \class VDetectorTree
    \brief tree with basic telescope parameters (positions, ...)

    Revision $Id: VDetectorTree.cpp,v 1.2.30.1.10.12.2.1 2010/12/17 09:58:34 gmaier Exp $

    \author Gernot Maier
*/

#include "VDetectorTree.h"

VDetectorTree::VDetectorTree()
{
    fDebug = false;
    if( fDebug ) cout << "VDetectorTree::VDetectorTree()" << endl;

    fTreeDet = 0;

    fNTel = 0;
    fTelxpos = 0.;
    fTelypos = 0.;
    fTelzpos = 0.;

}


VDetectorTree::~VDetectorTree()
{

}


bool VDetectorTree::fillDetectorTree( VDetectorGeometry* iDet )
{
    if( fDebug ) cout << "VDetectorTree::fill" << endl;

    if( iDet == 0 ) return false;

// define tree
    int fTelID = 0;
    unsigned int fTelID_hyperArray = 0;
    ULong64_t fTelType = 1;
    float fFocalLength = 0.;
    float fFOV = 0.;
    float fCameraScaleFactor = 0.;
    float fCameraCentreOffset = 0.;
    float fCameraRotation = 0.;
    unsigned int nPixel = 0;
    unsigned int nSamples = 0;
    unsigned int nGains = 2;
    float fHiLoScale = 0.;
    int   fHiLoThreshold = 0;
    float fHiLoOffset = 0.;
    const unsigned int fMaxPixel = 50000;
    float fXTubeMM[fMaxPixel];
    float fYTubeMM[fMaxPixel];
    float fRTubeMM[fMaxPixel];
    float fXTubeDeg[fMaxPixel];
    float fYTubeDeg[fMaxPixel];
    float fRTubeDeg[fMaxPixel];
    float fMirrorArea = 0.;
    int   fNMirrors = 0;

    if( fTreeDet == 0 )
    {
        fTreeDet = new TTree( "telconfig", "detector configuration" );

        fTreeDet->Branch( "NTel", &fNTel, "NTel/i" );
	fTreeDet->Branch( "TelID", &fTelID, "TelID/I" );
        fTreeDet->Branch( "TelType", &fTelType, "TelType/l" );
	fTreeDet->Branch( "TelID_hyperArray", &fTelID_hyperArray, "TelID_hyperArray/i" );
        fTreeDet->Branch( "TelX", &fTelxpos, "TelX/F" );
        fTreeDet->Branch( "TelY", &fTelypos, "TelY/F" );
        fTreeDet->Branch( "TelZ", &fTelzpos, "TelZ/F" );
	fTreeDet->Branch( "NMirrors", &fNMirrors, "NMirrors/I" );
	fTreeDet->Branch( "MirrorArea", &fMirrorArea, "MirrorArea/F" );
	fTreeDet->Branch( "FOV", &fFOV, "FOV/F" );
        fTreeDet->Branch( "FocalLength", &fFocalLength, "FocalLength/F" );
        fTreeDet->Branch( "CameraScaleFactor", &fCameraScaleFactor, "CameraScaleFactor/F" );
        fTreeDet->Branch( "CameraCentreOffset", &fCameraCentreOffset, "CameraCentreOffset/F" );
        fTreeDet->Branch( "CameraRotation", &fCameraRotation, "CameraRotation/F" );
        fTreeDet->Branch( "NPixel", &nPixel, "NPixel/i" );
        fTreeDet->Branch( "NSamples", &nSamples, "NSamples/i" );
	fTreeDet->Branch( "NGains", &nGains, "NGains/i" );
	fTreeDet->Branch( "HiLoScale", &fHiLoScale, "HiLoScale/F" );
	fTreeDet->Branch( "HiLoThreshold", &fHiLoThreshold, "HiLoThreshold/I" );
	fTreeDet->Branch( "HiLoOffset", &fHiLoOffset, "HiLoOffset/F" );
        fTreeDet->Branch( "XTubeMM", fXTubeMM, "XTubeMM[NPixel]/F" );
        fTreeDet->Branch( "YTubeMM", fYTubeMM, "YTubeMM[NPixel]/F" );
        fTreeDet->Branch( "RTubeMM", fRTubeMM, "RTubeMM[NPixel]/F" );
        fTreeDet->Branch( "XTubeDeg", fXTubeDeg, "XTubeDeg[NPixel]/F" );
        fTreeDet->Branch( "YTubeDeg", fYTubeDeg, "YTubeDeg[NPixel]/F" );
        fTreeDet->Branch( "RTubeDeg", fRTubeDeg, "RTubeDeg[NPixel]/F" );
    }

// fill the tree
    if( iDet != 0 )
    {
        for( unsigned int i = 0; i < iDet->getNTel(); i++ )
        {
            fNTel = iDet->getNTel();
	    if( iDet->getTelID_matrix().find( i ) != iDet->getTelID_matrix().end() )
	    {
	       fTelID_hyperArray = iDet->getTelID_matrix()[i];
	       fTelID = i;
            }
	    else
	    {
	       fTelID_hyperArray = i;
	       fTelID = i;
            }
            fTelxpos = iDet->getTelXpos()[i];
            fTelypos = iDet->getTelYpos()[i];
            fTelzpos = iDet->getTelZpos()[i];
            fFocalLength = iDet->getFocalLength()[i];
	    fFOV = iDet->getFieldofView()[i];
	    if( i < iDet->getNMirrors().size() )   fMirrorArea = (int)iDet->getNMirrors()[i];
	    else                                   fMirrorArea = 0;
	    if( i < iDet->getMirrorArea().size() ) fNMirrors = (int)iDet->getMirrorArea()[i];
	    else                                   fNMirrors = 0;
            fCameraScaleFactor = iDet->getCameraScaleFactor()[i];
            fCameraCentreOffset = iDet->getCameraCentreOffset()[i];
            fCameraRotation = iDet->getCameraRotation()[i];
            fTelType = iDet->getTelType()[i];
            nPixel = iDet->getNChannels( i );
            nSamples = iDet->getNSamples( i );
	    fHiLoScale = iDet->getLowGainMultiplier()[i];
	    fHiLoThreshold = iDet->getLowGainThreshold()[i];
	    fHiLoOffset = 0.;
            if( nPixel < fMaxPixel )
            {
                for( unsigned int p = 0; p < nPixel; p++ )
                {
                    if( p < iDet->getX( i ).size() ) fXTubeDeg[p] = iDet->getX( i )[p];
                    if( p < iDet->getY( i ).size() ) fYTubeDeg[p] = iDet->getY( i )[p];
                    if( p < iDet->getTubeRadius( i ).size() ) fRTubeDeg[p] = iDet->getTubeRadius( i )[p];
                    if( p < iDet->getX_MM( i ).size() ) fXTubeMM[p] = iDet->getX_MM( i )[p];
                    if( p < iDet->getY_MM( i ).size() ) fYTubeMM[p] = iDet->getY_MM( i )[p];
                    if( p < iDet->getTubeRadius( i ).size() ) fRTubeMM[p] = iDet->getTubeRadius_MM( i )[p];
                }
            }

            fTreeDet->Fill();
        }
    }
    return true;
}


bool VDetectorTree::readDetectorTree( VDetectorGeometry *iDet, TTree *iTree )
{
    if( !iDet || !iTree ) return false;

    cout << "Filling detector tree: " << iTree->GetName() << endl;

// define tree
    float fFocalLength = 0.;
    float fCameraScaleFactor = 1.;
    float fCameraCentreOffset = 0.;
    float fCameraRotation = 0.;
    float fFOV= 0.;
    unsigned int nPixel = 0;
    unsigned int nSamples = 0;
    unsigned int nGains = 0;
    float fHiLoScale = 0.;
    int   fHiLoThreshold = 0;
    float fHiLoOffset = 0.;
    const unsigned int fMaxPixel = 50000;
    float fXTubeMM[fMaxPixel];
    float fYTubeMM[fMaxPixel];
    float fRTubeMM[fMaxPixel];
    float fXTubeDeg[fMaxPixel];
    float fYTubeDeg[fMaxPixel];
    float fRTubeDeg[fMaxPixel];
    int fTubeOFF[fMaxPixel];
    for( unsigned int i = 0; i < fMaxPixel; i++ ) fTubeOFF[i] = 0;
    ULong64_t fTelType = 1;
    int fTelID;
    int fNMirrors = 0;
    float fMirrorArea = 0.;

    iTree->SetBranchAddress( "NTel", &fNTel );
    iTree->SetBranchAddress( "TelID", &fTelID );
    iTree->SetBranchAddress( "TelType", &fTelType );
    iTree->SetBranchAddress( "TelX", &fTelxpos );
    iTree->SetBranchAddress( "TelY", &fTelypos );
    iTree->SetBranchAddress( "TelZ", &fTelzpos );
    iTree->SetBranchAddress( "FocalLength", &fFocalLength );
    if( iTree->GetBranchStatus( "FOV" ) ) iTree->SetBranchAddress( "FOV", &fFOV );
    iTree->SetBranchAddress( "CameraScaleFactor", &fCameraScaleFactor );
    iTree->SetBranchAddress( "CameraCentreOffset", &fCameraCentreOffset );
    iTree->SetBranchAddress( "CameraRotation", &fCameraRotation );
    iTree->SetBranchAddress( "NPixel", &nPixel );
    iTree->SetBranchAddress( "NSamples", &nSamples );
    if( iTree->GetBranchStatus( "NGains" ) ) iTree->SetBranchAddress( "NGains", &nGains );
    if( iTree->GetBranchStatus( "HiLoScale" ) ) iTree->SetBranchAddress( "HiLoScale", &fHiLoScale );
    if( iTree->GetBranchStatus( "HiLoThreshold" ) ) iTree->SetBranchAddress( "HiLoThreshold", &fHiLoThreshold );
    if( iTree->GetBranchStatus( "HiLoOffset" ) ) iTree->SetBranchAddress( "HiLoOffset", &fHiLoOffset );
    iTree->SetBranchAddress( "XTubeMM", fXTubeMM );
    iTree->SetBranchAddress( "YTubeMM", fYTubeMM );
    iTree->SetBranchAddress( "RTubeMM", fRTubeMM );
    iTree->SetBranchAddress( "XTubeDeg", fXTubeDeg );
    iTree->SetBranchAddress( "YTubeDeg", fYTubeDeg );
    iTree->SetBranchAddress( "RTubeDeg", fRTubeDeg );
    if( iTree->GetBranchStatus( "TubeOFF" ) )    iTree->SetBranchAddress( "TubeOFF", fTubeOFF );
    if( iTree->GetBranchStatus( "NMirrors" ) )   iTree->SetBranchAddress( "NMirrors", &fNMirrors );
    if( iTree->GetBranchStatus( "MirrorArea" ) ) iTree->SetBranchAddress( "MirrorArea", &fMirrorArea );

    vector< unsigned int > i_npix;
    for( int i = 0; i < iTree->GetEntries(); i++ )
    {
        iTree->GetEntry( i );

        i_npix.push_back( nPixel );
    }
    iDet->addDataVector( fNTel, i_npix );

    map< unsigned int, unsigned int > i_telID_matrix;

    for( int i = 0; i < iTree->GetEntries(); i++ )
    {
        iTree->GetEntry( i );

	i_telID_matrix[(unsigned int)i] = (unsigned int)fTelID;

        iDet->getTelType()[i] = fTelType;

        iDet->getTelXpos()[i] = fTelxpos;
        iDet->getTelYpos()[i] = fTelypos;
        iDet->getTelZpos()[i] = fTelzpos;

        iDet->getFocalLength()[i] = fFocalLength;
        iDet->getFieldofView()[i] = fFOV;

        iDet->getCameraScaleFactor()[i] = fCameraScaleFactor;
        iDet->getCameraCentreOffset()[i] = fCameraCentreOffset;
        iDet->getCameraRotation()[i] = fCameraRotation;

        iDet->getNChannels()[i] = nPixel;
	iDet->setNSamples( i, nSamples, true );
	iDet->setLowGainMultiplier( i, fHiLoScale );
	iDet->setLowGainThreshold( i, (unsigned int)fHiLoThreshold );

	iDet->getNMirrors()[i] = (unsigned int)fNMirrors;
	iDet->getMirrorArea()[i] = fMirrorArea;

        for( unsigned int p = 0; p < nPixel; p++ )
        {
// change camera coordinate system to VERITAS one
// (GM) (-y,-x is as well ok, check this with offset MC)
            if( p < iDet->getX( i ).size() ) iDet->getX( i )[p] = fYTubeDeg[p];
            if( p < iDet->getY( i ).size() ) iDet->getY( i )[p] = fXTubeDeg[p];
            if( p < iDet->getX( i ).size() ) iDet->getXUnrotated( i )[p] = fYTubeDeg[p];
            if( p < iDet->getY( i ).size() ) iDet->getYUnrotated( i )[p] = fXTubeDeg[p];
            if( p < iDet->getTubeRadius( i ).size() ) iDet->getTubeRadius( i )[p] = fRTubeDeg[p];
            if( p < iDet->getX_MM( i ).size() ) iDet->getX_MM( i )[p] = fYTubeMM[p];
            if( p < iDet->getY_MM( i ).size() ) iDet->getY_MM( i )[p] = fXTubeMM[p];
            if( p < iDet->getTubeRadius_MM( i ).size() ) iDet->getTubeRadius_MM( i )[p] = fRTubeMM[p];
            if( p < iDet->getAnaPixel( i ).size() )
            {
                if( fTubeOFF[p] != 0 ) iDet->getAnaPixel( i )[p] = -1*TMath::Abs( fTubeOFF[p] );
                else                   iDet->getAnaPixel( i )[p] = 1;
            }
        }
    }
    iDet->setTelID_matrix( i_telID_matrix );

    iDet->stretchAndMoveCamera();
    iDet->rotateCamera();

    iDet->makeNeighbourList();

    cout << "..done (" << fNTel << ")" << endl;

    return true;
}
