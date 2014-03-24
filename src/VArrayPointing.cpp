/* \class VArrayPointing

   \brief data class for general pointing values valid for all telescopes

   e.g. reference pointing, target, e.g.



*/

#include "VArrayPointing.h"

VArrayPointing::VArrayPointing( bool bInitTree )
{
	setObservatory();
	reset();
	
	if( bInitTree )
	{
		initializePointingTree();
	}
}


void VArrayPointing::initializePointingTree()
{
	char hname[200];
	char htitle[200];
	
	sprintf( hname, "pointingData" );
	sprintf( htitle, "pointing (Array)" );
	fPointingTree = new TTree( hname, htitle );
	fPointingTree->Branch( "MJD", &fMJD, "MJD/i" );
	fPointingTree->Branch( "Time", &fTime, "Time/D" );
	fPointingTree->Branch( "TargetAzimuth", &fTargetAzimuth, "TargetAzimuth/D" );
	fPointingTree->Branch( "TargetElevation", &fTargetElevation, "TargetElevation/D" );
	fPointingTree->Branch( "TargetRAJ2000", &fTargetRAJ2000, "TargetRAJ2000/D" );
	fPointingTree->Branch( "TargetDecJ2000", &fTargetDecJ2000, "TargetDecJ2000/D" );
	fPointingTree->Branch( "TargetRA", &fTargetRA, "TargetRA/D" );
	fPointingTree->Branch( "TargetDec", &fTargetDec, "TargetDec/D" );
// TelAzimuth and TelAzimthCalculate should be the same (same for Elevation)
	fPointingTree->Branch( "TelAzimuth", &fTelAzimuth, "TelAzimuth/D" );
	fPointingTree->Branch( "TelElevation", &fTelElevation, "TelElevation/D" );
	fPointingTree->Branch( "TelAzimuthCalculated", &fTelAzimuthCalculated, "TelAzimuthCalculated/F" );
	fPointingTree->Branch( "TelElevationCalculated", &fTelElevationCalculated, "TelElevationCalculated/F" );
	fPointingTree->Branch( "TelRAJ2000", &fTelRAJ2000, "TelRAJ2000/D" );
	fPointingTree->Branch( "TelDecJ2000", &fTelDecJ2000, "TelDecJ2000/D" );
	fPointingTree->Branch( "TelRA", &fTelRA, "TelRA/D" );
	fPointingTree->Branch( "TelDec", &fTelDec, "TelDec/D" );
}


void VArrayPointing::fillPointingTree()
{
	if( fPointingTree )
	{
		fPointingTree->Fill();
	}
}

void VArrayPointing::terminate()
{
	if( fPointingTree )
	{
		fPointingTree->Write();
	}
}

