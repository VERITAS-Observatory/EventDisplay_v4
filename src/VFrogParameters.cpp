/*! \class VFrogParameters
    \brief  VFrogParameters storage class for shower Frogs reconstruction 

    \author Gareth Hughes
*/

#include "VFrogParameters.h"

VFrogParameters::VFrogParameters()
{

//    fDebug = false;
//    if( fDebug ) cout << "VFrogParameters::VFrogParameters( int iNTel ) " << iNTel << endl;
    fTreeFrog = 0;
//    fNTel = iNTel;
    reset();

//    vector< bool > it( fNTel, 0 );
//    vector< float > i_f( fNTel, -99. );
/*
    for( unsigned int i = 0; i < fNMethods; i++ )
    {
        fTelIDImageSelected.push_back( it );
//C. Duke 19Oct06
        fTelIDImageSelected_bitcode[i]=0;
        fShower_Xoff_DISP.push_back( i_f );
        fShower_Yoff_DISP.push_back( i_f );
	fShower_Weight_DISP.push_back( i_f );
    }
*/
}


/*!
   initialise this data tree
   \param iName tree name
   \param iTitle tree title
*/
void VFrogParameters::initTree( string iName, string iTitle )
{

    printf("FROGPUT In VFrogParameters %s %s\n",iName.c_str(),iTitle.c_str());
    
    fTreeFrog = new TTree( iName.c_str(), iTitle.c_str() );
    fTreeFrog->SetMaxTreeSize(1000*Long64_t(2000000000));
//    fTreeFrog->SetAutoSave(100000000);              // autosave when 100 Mbytes written
    fTreeFrog->SetAutoSave(10);              // autosave when 100 Mbytes written

//  Branches:
    fTreeFrog->Branch("frogsEventID", &frogsEventID, "frogsEventID/I" );
    fTreeFrog->Branch("frogsGSLConStat", &frogsGSLConStat, "frogsGSLConStat/I" );
    fTreeFrog->Branch("frogsNB_iter", &frogsNB_iter, "frogsNB_iter/I" );
    fTreeFrog->Branch("frogsXS", &frogsXS, "frogsXS/F" );
    fTreeFrog->Branch("frogsXSerr", &frogsXSerr, "frogsXSerr/F" );
    fTreeFrog->Branch("frogsYS", &frogsYS, "frogsYS/F" );
    fTreeFrog->Branch("frogsYSerr", &frogsYSerr, "frogsYSerr/F" );
    fTreeFrog->Branch("frogsXP", &frogsXP, "frogsXP/F" );
    fTreeFrog->Branch("frogsXPerr", &frogsXPerr, "frogsXPerr/F" );
    fTreeFrog->Branch("frogsYP", &frogsYP, "frogsYP/F" );
    fTreeFrog->Branch("frogsYPerr", &frogsYPerr, "frogsYPerr/F" );
    fTreeFrog->Branch("frogsEnergy", &frogsEnergy, "frogsEnergy/F" );
    fTreeFrog->Branch("frogsEnergyerr", &frogsEnergyerr, "frogsEnergyerr/F" );
    fTreeFrog->Branch("frogsLambda", &frogsLambda, "frogsLambda/F" );
    fTreeFrog->Branch("frogsLambdaerr", &frogsLambdaerr, "frogsLambdaerr/F" );
    fTreeFrog->Branch("frogsGoodnessImg", &frogsGoodnessImg, "frogsGoodnessImg/F" );
    fTreeFrog->Branch("frogsNpixImg", &frogsNpixImg, "frogsNpixImg/I" );
    fTreeFrog->Branch("frogsGoodnessBkg", &frogsGoodnessBkg, "frogsGoodnessBkg/F" );
    fTreeFrog->Branch("frogsNpixBkg", &frogsNpixBkg, "frogsNpixBkg/I" );

    fTreeFrog->Branch("frogsXPStart", &frogsXPStart, "frogsXPStart/F" );
    fTreeFrog->Branch("frogsYPStart", &frogsYPStart, "frogsYPStart/F" );
    fTreeFrog->Branch("frogsXPED", &frogsXPED, "frogsXPED/F" );
    fTreeFrog->Branch("frogsYPED", &frogsYPED, "frogsYPED/F" );
    fTreeFrog->Branch("frogsXSStart", &frogsXSStart, "frogsXSStart/F" );
    fTreeFrog->Branch("frogsYSStart", &frogsYSStart, "frogsYSStart/F" );

}



void VFrogParameters::reset()
{

//  0 all the values
     frogsEventID     = 0;
     frogsGSLConStat  = 0;
     frogsNB_iter     = 0;
     frogsXS          = 0.0;
     frogsXSerr       = 0.0;
     frogsYS          = 0.0;
     frogsYSerr       = 0.0;
     frogsXP          = 0.0;
     frogsXPerr       = 0.0;
     frogsYP          = 0.0;
     frogsYPerr       = 0.0;
     frogsEnergy      = 0.0;
     frogsEnergyerr   = 0.0;
     frogsLambda      = 0.0;
     frogsLambdaerr   = 0.0;
     frogsGoodnessImg = 0.0;
     frogsNpixImg     = 0;
     frogsGoodnessBkg = 0.0;
     frogsNpixBkg     = 0;

 }


void VFrogParameters::printParameters()
{

// Output Stuff

/*
    cout << "Shower parameters: " << endl;
    cout << runNumber << "\t" << eventNumber << " (status " << eventStatus << ")\t" <<  MJD;
    cout << time << "\t" << fNTelescopes << endl;
    cout << "Trigger: " << endl;
    cout << fNTrig << "\t" << fLTrig << endl;

    cout << "Target elev,azim,dec,ra,wobblenorth,wobbleease" << endl;
    cout << fTargetElevation <<  "\t" << fTargetAzimuth << endl;
    cout <<  fTargetDec <<  "\t" << fTargetRA << endl;
    cout << fWobbleNorth <<  "\t" << fWobbleEast << endl;
*/
}


